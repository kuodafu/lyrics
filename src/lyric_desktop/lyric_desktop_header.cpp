#include "lyric_wnd_function.h"
#include <d2d/CCustomTextRenderer.h>
#include "GetMonitorRect.h"
#include <atlbase.h>

using namespace KUODAFU_NAMESPACE;


NAMESPACE_LYRIC_DESKTOP_BEGIN


void LYRIC_DESKTOP_INFO::set_def_arg(const LYRIC_DESKTOP_ARG* arg)
{
    // 有值就根据传递进来的值设置, 没有值就设置默认值
    LYRIC_DESKTOP_ARG def = { 0 };
    lyric_desktop_get_default_arg(&def);
    if (!arg)
        arg = &def;

    if (arg->pClrNormal)
        clrNormal.assign(&arg->pClrNormal[0], &arg->pClrNormal[arg->nClrNormal]);
    if (arg->pClrLight)
        clrLight.assign(&arg->pClrLight[0], &arg->pClrLight[arg->nClrLight]);

    clrBorder = arg->clrBorder;
    dx.clrWndBorder = arg->clrWndBorder;
    dx.clrBack = arg->clrWndBack;

    LPCWSTR pszFontName = arg->pszFontName;
    if (!pszFontName || !*pszFontName)
        pszFontName = L"微软雅黑";

    wcscpy_s(lf.lfFaceName, pszFontName);
    lf.lfWeight = FW_BOLD;
    lf.lfHeight = arg->nFontSize;


}

void LYRIC_DESKTOP_INFO::dpi_change(HWND hWnd)
{
    scale = hWnd;
    const int _10 = scale(10);
    if (dx.hFont)
        dx.re_create_font(this);
    padding_text = (float)(_10 / 2);
    shadowRadius = (float)10;
    padding_wnd = (float)scale(8);
    change_btn = true;
    change_text = true;
    change_wnd = true;
}

void LYRIC_DESKTOP_INFO::get_monitor()
{
    int len = GetMonitorRects(rcMonitors);
    if (len == 0)
    {
        rcMonitor = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
        rcMonitors = { rcMonitor };
        return;
    }

    GetMonitorRect(&rcMonitor);
    rcMonitor = {};
    for (const RECT& rc : rcMonitors)
    {
        rcMonitor.left   = min(rcMonitor.left  , rc.left);
        rcMonitor.top    = min(rcMonitor.top   , rc.top);
        rcMonitor.right  = max(rcMonitor.right , rc.right);
        rcMonitor.bottom = max(rcMonitor.bottom, rc.bottom);
    }

}

float LYRIC_DESKTOP_INFO::get_lyric_line_height() const
{
    if (has_mode(LYRIC_MODE::VERTICAL))
    {
        // 竖屏, 取宽度加上边距
        return word_width + padding_text * 2;
    }
    return word_height + padding_text * 2;
}

float LYRIC_DESKTOP_INFO::get_lyric_line_width(float vl) const
{
    if (has_mode(LYRIC_MODE::VERTICAL))
        return (vl ? vl : nLineDefHeight) + padding_text * 2;
    
    return (vl ? vl : nLineDefWidth) + padding_text * 2;
}


LYRIC_DESKTOP_CACHE_OBJ::LYRIC_DESKTOP_CACHE_OBJ()
{
    preIndex = -1;
    preText = nullptr;
    preLength = 0;
    pBitmapNormal = nullptr;
    pBitmapLight = nullptr;
    rcBounds = { 0 };
}

LYRIC_DESKTOP_CACHE_OBJ::~LYRIC_DESKTOP_CACHE_OBJ()
{
    SafeRelease(pBitmapNormal);
    SafeRelease(pBitmapLight);
}

LYRIC_DESKTOP_INFO::LYRIC_DESKTOP_INFO()
{
    const int clear_size = offsetof(LYRIC_DESKTOP_INFO, line1);
    memset(this, 0, clear_size);
    prevIndexLine = -1;
    mode = LYRIC_MODE::DOUBLE_ROW;
    pszDefText = L"该歌曲暂时没有歌词";
    nDefText = 9;

    pCritSec = new CRITICAL_SECTION;
    InitializeCriticalSection(pCritSec);

    dx.clrBack = MAKEARGB(100, 0, 0, 0);
    clrBorder = MAKEARGB(255, 33, 33, 33);

    pfnCommand = nullptr;
    lParam = 0;
    nAddref = 1;
}

// 设置歌词窗口数据到窗口
void lyric_wnd_set_data(HWND hWnd, PLYRIC_DESKTOP_INFO pWndInfo)
{
    SetWindowLongPtrW(hWnd, 0, (LONG_PTR)pWndInfo);
}

// 从窗口获取歌词窗口数据
PLYRIC_DESKTOP_INFO lyric_wnd_get_data(HWND hWnd)
{
    return (PLYRIC_DESKTOP_INFO)GetWindowLongPtrW(hWnd, 0);
}

bool isLatinCharacter(wchar_t ch) 
{
    return
        // Printable ASCII characters: 0x21 ('!') to 0x7E ('~')
        (ch >= 0x20 && ch <= 0x7E) ||

        (ch == L'（' || ch == L'）'
         || ch == L'「' || ch == L'」'
         || ch == L'【' || ch == L'】'
         || ch == L'《' || ch == L'》'
         || ch == L'〈' || ch == L'〉'
         || ch == L'『' || ch == L'』'
         || ch == L'〔' || ch == L'〕'
         || ch == L'〖' || ch == L'〗'
         || ch == L'‘' || ch == L'’'
         || ch == L'“' || ch == L'”'
         || ch == L'《' || ch == L'》'
         || ch == L'〈' || ch == L'〉'
         || ch == L'『' || ch == L'』'
         ) ||

        // Latin-1 Supplement
        (ch >= 0x00C0 && ch <= 0x00D6) || // 
        (ch >= 0x00D8 && ch <= 0x00F6) || // 
        (ch >= 0x00F8 && ch <= 0x00FF) || // 

        // Latin Extended-A
        (ch >= 0x0100 && ch <= 0x017F) ||

        // Latin Extended-B
        (ch >= 0x0180 && ch <= 0x024F);
}

int lyric_wnd_set_state_translate(LYRIC_DESKTOP_INFO& wnd_info, int language)
{
    LYRIC_DESKTOP_BUTTON_STATE b1 = __query(language, 1) ? LYRIC_DESKTOP_BUTTON_STATE_NORMAL : LYRIC_DESKTOP_BUTTON_STATE_DISABLE;
    LYRIC_DESKTOP_BUTTON_STATE b2 = __query(language, 2) ? LYRIC_DESKTOP_BUTTON_STATE_NORMAL : LYRIC_DESKTOP_BUTTON_STATE_DISABLE;
    lyric_wnd_set_btn_state(wnd_info, LYRIC_DESKTOP_BUTTON_ID_TRANSLATE1, b1);
    lyric_wnd_set_btn_state(wnd_info, LYRIC_DESKTOP_BUTTON_ID_TRANSLATE2, b2);
    lyric_wnd_set_btn_state(wnd_info, LYRIC_DESKTOP_BUTTON_ID_TRANSLATE1_SEL, b1);
    lyric_wnd_set_btn_state(wnd_info, LYRIC_DESKTOP_BUTTON_ID_TRANSLATE2_SEL, b2);
    return 0;
}

NAMESPACE_LYRIC_DESKTOP_END