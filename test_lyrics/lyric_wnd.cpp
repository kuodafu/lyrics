/*
    这个文件主要显示歌词的窗口
    一个透明窗口, 显示一行/两行歌词, 歌词顶部有一些按钮操作
*/

#include "lyric_wnd.h"
#include "lyric_exe_header.h"
#include <string>
#include <unordered_map>
#include "lyric_wnd_function.h"
#include <CommCtrl.h>
#include "CCustomTextRenderer.h"


using namespace NAMESPACE_D2D;

#define TIMERID_UPDATE_LYRIC 1000   // 时钟ID, 更新歌词


void lyric_wnd_get_default_arg(LYRIC_WND_ARG* arg)
{
    //static DWORD clrNormal[] =
    //{
    //    MAKEARGB(255, 0,52,138),
    //    MAKEARGB(255, 0,128,192),
    //    MAKEARGB(255, 3,202,252),
    //};

    static DWORD clrNormal[] =
    {
        MAKEARGB(255, 0,109,178),
        MAKEARGB(255, 3,189,241),
        MAKEARGB(255, 3,202,252),
    };

    static DWORD clrLight[] =
    {
        MAKEARGB(255, 255,255,255),
        MAKEARGB(255, 130,247,253),
        MAKEARGB(255, 3, 233, 252),
    };

    arg->clrWndBack = MAKEARGB(100, 0, 0, 0);
    arg->clrWndBorder = MAKEARGB(200, 0, 0, 0);
    arg->nFontSize = 24;
    arg->pszFontName = L"微软雅黑";
    arg->clrBorder = MAKEARGB(255, 33, 33, 33);

    arg->pClrNormal = clrNormal;
    arg->nClrNormal = _countof(clrNormal);

    arg->pClrLight = clrLight;
    arg->nClrLight = _countof(clrLight);

    RECT rc;
    GetWindowRect(GetDesktopWindow(), &rc);
    const int cxScreen = rc.right - rc.left;
    const int cyScreen = rc.bottom - rc.top;

    const int width = 900;
    const int height = 200;
    arg->rcWindow.left = (cxScreen - width) / 2;
    arg->rcWindow.top = (cyScreen - height - 100);
    arg->rcWindow.right = arg->rcWindow.left + width;
    arg->rcWindow.bottom = arg->rcWindow.top + height;

}

HWND lyric_wnd_create(const LYRIC_WND_ARG* arg, PFN_LYRIC_WND_COMMAND pfnCommand, LPARAM lParam)
{
    using namespace lyric_wnd;

    HWND hWnd = lyric_create_layered_window(arg);
    if (!hWnd)
        return nullptr;

    LYRIC_WND_INFO* pWndInfo = new LYRIC_WND_INFO;
    LYRIC_WND_INFO& wnd_info = *pWndInfo;
    lyric_wnd_set_data(hWnd, pWndInfo);


    RECT rcDesk;
    GetWindowRect(GetDesktopWindow(), &rcDesk);
    const int cxScreen = rcDesk.right - rcDesk.left;
    const int cyScreen = rcDesk.bottom - rcDesk.top;

    wnd_info.hWnd = hWnd;
    wnd_info.pfnCommand = pfnCommand;
    wnd_info.lParam = lParam;
    wnd_info.line1.align = 0;
    wnd_info.line2.align = 2;
    wnd_info.mode = LYRIC_MODE::DOUBLE_ROW;

    // 位置相关的放一起处理
    wnd_info.dpi_change(hWnd);
    wnd_info.get_monitor();

    RECT rc = arg->rcWindow;
    wnd_info.scale(rc);

    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;

    if(rc.top > cyScreen - wnd_info.scale(250))
        rc.top = cyScreen - wnd_info.scale(250), rc.bottom = rc.top + height;
    if(rc.left < rcDesk.left)
        rc.left = (cxScreen - width) / 2, rc.right = rc.left + width;

    MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

    wnd_info.hTips = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
                                     TOOLTIPS_CLASSW,
                                     nullptr,
                                     TTS_ALWAYSTIP,
                                     0, 0, 0, 0, 0, 0, 0, 0);

    SendMessageW(wnd_info.hTips, TTM_SETMAXTIPWIDTH, 0, 500);
    SendMessageW(wnd_info.hTips, TTM_SETDELAYTIME, TTDT_AUTOPOP, 0x7fff);
    TTTOOLINFOW ti = { 0 };
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.lpszText = (LPWSTR)L"";
    ti.hwnd = wnd_info.hWnd;
    ti.uId = (UINT_PTR)wnd_info.hWnd;
    auto isInsertSuccess = SendMessageW(wnd_info.hTips, TTM_ADDTOOLW, 0, (LPARAM)&ti);


    wnd_info.set_def_arg(arg);
    lyric_wnd_default_object(wnd_info);

    if (wnd_info.nMinHeight)
        MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, wnd_info.nMinHeight, TRUE);
    
    return hWnd;
}

static float _lyric_wnd_load_krc_calc_text(lyric_wnd::PLYRIC_WND_INFO pWndInfo, IDWriteTextLayout* pTextLayout, float* pHeight)
{
    using namespace lyric_wnd;

    *pHeight = 0;

    if (!pTextLayout)
        return 0.f;
    const bool is_vertical = pWndInfo->has_mode(LYRIC_MODE::VERTICAL);
    if (!is_vertical)
    {
        // 不是纵向, 直接计算然后返回
        CCustomTextRenderer renderer;
        pTextLayout->Draw(0, &renderer, 0, 0);
        *pHeight = renderer.get_height();
        return renderer.get_width();
    }
    
    // 走到这里就是纵向, 需要额外计算, 因为纵向的英文数字 这些字符会旋转, 需要特别计算
    // 只需要计算高度, 宽度是所有字符里最宽的那个, 高度是累加, 有旋转的加宽度, 没旋转的加高度

    float width = 0.f, height = 0.f;

    CCustomTextRenderer renderer([&](void* clientDrawingContext,
                                        FLOAT baselineOriginX,
                                        FLOAT baselineOriginY,
                                        DWRITE_MEASURING_MODE measuringMode,
                                        DWRITE_GLYPH_RUN const* glyphRun,
                                        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                                        IUnknown* clientDrawingEffect)
    {
        // 获取字体度量
        for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
        {
            DWRITE_FONT_METRICS metrics{};
            glyphRun->fontFace->GetMetrics(&metrics);

            float designUnitsPerEm = (float)metrics.designUnitsPerEm;
            float _height = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;
            float _width = (glyphRun->glyphAdvances ? glyphRun->glyphAdvances[i] : 0.0f);

            if (width < _width)
                width = _width; // 宽度只记录最大的宽度, 竖屏歌词宽度是固定的

            wchar_t ch = glyphRunDescription && glyphRunDescription->string ? glyphRunDescription->string[i] : L'\0';
            bool is_alpha = isLatinCharacter(ch);
            if (is_alpha)
                height += _width;   // 需要旋转的字符就加上宽度, 
            else
                height += _height;  // 非旋转的字符就加上高度
        }
    });
    pTextLayout->Draw(0, &renderer, 0, 0);

    *pHeight = height;
    return width;
}


bool lyric_wnd_load_krc(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen)
{
    using namespace lyric_wnd;
    PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    lyric_destroy(pWndInfo->hLyric);
    LYRIC_WND_INFO& wnd_info = *pWndInfo;
    wnd_info.hLyric = lyric_parse(pKrcData, nKrcDataLen);
    pWndInfo->nTimeOffset = lyric_behind_ahead(wnd_info.hLyric, 0);
    pWndInfo->line1.clear();
    pWndInfo->line2.clear();

    int language = lyric_get_language(wnd_info.hLyric);
    lyric_wnd_set_state_translate(*pWndInfo, language);

    if (language)
        wnd_info.add_mode(LYRIC_MODE::EXISTTRANS);
    else
        wnd_info.del_mode(LYRIC_MODE::EXISTTRANS);

    if (!wnd_info.hLyric)
        return false;
    auto& d2dInfo = d2d_get_info();

    // 必须得用小数记, 不然歌词字多了会相差出很多个像素
    lyric_calc_text_width(wnd_info.hLyric, [](void* pUserData, LPCWSTR pText, int nTextLen, float* pRetHeight) -> float
    {
        LYRIC_WND_INFO* pWndInfo = (LYRIC_WND_INFO*)pUserData;
        CD2DRender& hCanvas = *pWndInfo->dx.hCanvas;
        if (!pWndInfo->dx.hFont)
            lyric_wnd_default_object(*pWndInfo);

        IDWriteTextLayout* pTextLayout = lyric_wnd_create_text_layout(pText, nTextLen, *pWndInfo->dx.hFont, 0, 0);
        float width = _lyric_wnd_load_krc_calc_text(pWndInfo, pTextLayout, pRetHeight);
        SafeRelease(pTextLayout);
        return width;
    }, pWndInfo);

    return true;
}

bool lyric_wnd_update(HWND hWindowLyric, int nCurrentTimeMS)
{
    using namespace lyric_wnd;
    PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    LYRIC_WND_INFO& wnd_info = *pWndInfo;
    wnd_info.nCurrentTimeMS = nCurrentTimeMS;
    //InvalidateRect(hWindowLyric, 0, 0);   // 使用这个方式会卡, 不知道啥情况, 直接调用重画吧
    lyric_wnd_invalidate(wnd_info);
    return true;
}

bool lyric_wnd_set_color(HWND hWindowLyric, bool isLight, DWORD* pClr, int nCount)
{
    using namespace lyric_wnd;
        PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    if (isLight)
    {
        pWndInfo->clrLight.assign(&pClr[0], &pClr[nCount]);
    }
    else
    {
        pWndInfo->clrNormal.assign(&pClr[0], &pClr[nCount]);
    }

    return pWndInfo->dx.re_create_brush(pWndInfo, isLight);
}

bool lyric_wnd_set_font(HWND hWindowLyric, LPCWSTR pszName, int nSize, bool isBold, bool isItalic)
{
    using namespace lyric_wnd;
        PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    if (!pszName || !*pszName)
        pszName = L"微软雅黑";
    if (nSize == 0)
        nSize = 24;

    LOGFONTW& lf = pWndInfo->lf;
    wcscpy_s(lf.lfFaceName, pszName);
    lf.lfHeight = nSize;
    lf.lfWeight = isBold ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = isItalic ? TRUE : FALSE;

    return pWndInfo->dx.re_create_font(pWndInfo);
}

bool lyric_wnd_set_clr_back(HWND hWindowLyric, DWORD clr)
{
    using namespace lyric_wnd;
        PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    pWndInfo->dx.clrBack = clr;
    return true;
}

bool lyric_wnd_set_clr_border(HWND hWindowLyric, DWORD clr)
{
    using namespace lyric_wnd;
        PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    pWndInfo->clrBorder = clr;
    return true;
}

bool lyric_wnd_get_config(HWND hWindowLyric, LYRIC_WND_ARG* arg)
{
    if (!arg || !hWindowLyric)
        return false;

    using namespace lyric_wnd;
        PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    GetWindowRect(hWindowLyric, &arg->rcWindow);
    pWndInfo->scale.unscale(arg->rcWindow); // 保存到配置里的是要保存未缩放的坐标

    arg->clrWndBack  = pWndInfo->dx.clrBack;
    arg->nFontSize      = pWndInfo->lf.lfHeight;
    arg->pszFontName    = pWndInfo->lf.lfFaceName;
    arg->clrBorder      = pWndInfo->clrBorder;
    arg->nClrNormal     = (int)pWndInfo->clrLight.size();
    arg->pClrNormal     = arg->nClrNormal > 0 ? &pWndInfo->clrLight[0] : nullptr;
    arg->nClrLight      = (int)pWndInfo->clrNormal.size();
    arg->pClrLight      = arg->nClrLight > 0 ? &pWndInfo->clrNormal[0] : nullptr;

    return true;
}

bool lyric_wnd_call_event(HWND hWindowLyric, LYRIC_WND_BUTTON_ID id)
{
    using namespace lyric_wnd;
        PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    return lyric_wnd::lyric_wnd_call_event(*pWndInfo, id);
}

bool lyric_wnd_set_button_state(HWND hWindowLyric, LYRIC_WND_BUTTON_ID id, LYRIC_WND_BUTTON_STATE state)
{
    using namespace lyric_wnd;
        PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    return lyric_wnd::lyric_wnd_set_btn_state(*pWndInfo, id, state);
}

LYRIC_WND_BUTTON_STATE lyric_wnd_get_button_state(HWND hWindowLyric, LYRIC_WND_BUTTON_ID id)
{
    using namespace lyric_wnd;
        PLYRIC_WND_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return LYRIC_WND_BUTTON_STATE_ERROR;
    return lyric_wnd::lyric_wnd_get_btn_state(*pWndInfo, id);
}

