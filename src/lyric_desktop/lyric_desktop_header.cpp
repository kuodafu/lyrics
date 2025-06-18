#include "lyric_desktop_function.h"
#include <d2d/CCustomTextRenderer.h>
#include "GetMonitorRect.h"
#include <atlbase.h>

using namespace KUODAFU_NAMESPACE;


NAMESPACE_LYRIC_DESKTOP_BEGIN


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void LYRIC_DESKTOP_INFO::init(HWND hWnd, const char* argJson, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam)
{
    this->pfnCommand    = pfnCommand;
    this->lParam        = lParam;
    this->nAddref       = 1;
    this->hWnd          = hWnd;
    this->scale         = hWnd;

    //////////////////////////////////////////////////////////////////////////
    // ����ѽṹ�ĳ�Ա��ʼ��һ��
    this->hTips             = nullptr;
    this->hLyric            = nullptr;
    this->prevIndexLine     = -1;
    this->prevWidth         = 0;
    this->prevHeight        = 0;
    this->word_width        = 0;
    this->word_height       = 0;
    this->nLineDefWidth     = 0;
    this->nLineDefHeight    = 0;  
    this->nCurrentTimeMS    = 0;  
    this->nTimeOffset       = 0;
    this->nMinWidth         = 0;
    this->nMinHeight        = 0;
    this->nLineTop1         = 0;
    this->nLineTop2         = 0;
    this->rcWindow          = {};
    this->rcMonitor         = {};
    this->shadowRadius      = 10.f; // �̶���, ������ӰͼƬ����10������
    this->mode              = LYRIC_DESKTOP_MODE::DOUBLE_ROW;

    // ���֤, ���ظ������Ҫ����, ��ʾ��ʵ�ʱ��Ҳ��Ҫ����
    // һ���߳��ڼ���һ���߳�����ʾ������²����������
    pCritSec = new CRITICAL_SECTION;
    InitializeCriticalSection(pCritSec);

    this->status = 0;
    this->change = 0;
    
    get_monitor();  // ��ȡ������Ļ����
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // ��ʼ������, Ȼ��������ô���dx����
    config.init();    // �����ȳ�ʼ��Ĭ������
    config.parse(argJson, this);

    //////////////////////////////////////////////////////////////////////////

    // �������ó�ʼ������, ��������������ݳ�ʼ��һЩ����
    if (config.bVertical)
        add_mode(LYRIC_DESKTOP_MODE::VERTICAL);
    if (config.bSingleLine)
        add_mode(LYRIC_DESKTOP_MODE::SINGLE_ROW);
    if (config.bSelfy)  // ʹ�õ�ʱ���ж������ֵ��ʹ�÷���
        add_mode(LYRIC_DESKTOP_MODE::TRANSLATION_FY);
    if (config.bSelyy)
        add_mode(LYRIC_DESKTOP_MODE::TRANSLATION_YY);

    //TODO ������Ҫ��������ģʽ, Ȼ���������ö���ģʽ
    line1.init(config.line1_align);
    line2.init(config.line2_align);



}

void LYRIC_DESKTOP_INFO::dpi_change(HWND hWnd)
{
    scale = hWnd;
    const int _10 = scale(10);
    if (dx.hFont)
        dx.re_create_font(this);

    config.padding_text = (float)(scale((int)config.padding_text_));
    config.padding_wnd = (float)scale((int)config.padding_wnd_);
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
    if (has_mode(LYRIC_DESKTOP_MODE::VERTICAL))
    {
        // ����, ȡ��ȼ��ϱ߾�
        return word_width + config.padding_text * 2;
    }
    return word_height + config.padding_text * 2;
}

float LYRIC_DESKTOP_INFO::get_lyric_line_width(float vl) const
{
    if (has_mode(LYRIC_DESKTOP_MODE::VERTICAL))
        return (vl ? vl : nLineDefHeight) + config.padding_text * 2;
    
    return (vl ? vl : nLineDefWidth) + config.padding_text * 2;
}



void LYRIC_DESKTOP_INFO::update(bool isSend)
{
    // ���Ƹ���Ƶ��, ����Ƶ��̫��Ҫ����һ��
    // ���200֡, Ӧ������ȫ�㹻����, 60֡��ʱ����Ѿ���������
    if (updateTime.end() < 5)
        return;

    updateTime.start();
    PostMessageW(hWnd, WM_USER, 121007124, 20752843);

    return;
    if (isSend)
        SendMessageW(hWnd, WM_USER, 121007124, 20752843);
    else
        PostMessageW(hWnd, WM_USER, 121007124, 20752843);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// ���ø�ʴ������ݵ�����
void lyric_wnd_set_data(HWND hWnd, PLYRIC_DESKTOP_INFO pWndInfo)
{
    SetWindowLongPtrW(hWnd, 0, (LONG_PTR)pWndInfo);
}

// �Ӵ��ڻ�ȡ��ʴ�������
PLYRIC_DESKTOP_INFO _lyric_desktop_get_data(HWND hWnd)
{
    return (PLYRIC_DESKTOP_INFO)GetWindowLongPtrW(hWnd, 0);
}

bool isLatinCharacter(wchar_t ch) 
{
    return
        // Printable ASCII characters: 0x21 ('!') to 0x7E ('~')
        (ch >= 0x20 && ch <= 0x7E) ||

        (ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
         || ch == L'��' || ch == L'��'
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
    lyric_wnd_set_btn_state(wnd_info, LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY, b1);
    lyric_wnd_set_btn_state(wnd_info, LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY, b2);
    lyric_wnd_set_btn_state(wnd_info, LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY_SEL, b1);
    lyric_wnd_set_btn_state(wnd_info, LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY_SEL, b2);
    return 0;
}

NAMESPACE_LYRIC_DESKTOP_END