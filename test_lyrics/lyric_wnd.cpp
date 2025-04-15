/*
    ����ļ���Ҫ��ʾ��ʵĴ���
    һ��͸������, ��ʾһ��/���и��, ��ʶ�����һЩ��ť����
*/

#include "lyric_wnd.h"
#include <d2d/d2d.h>
#include "lyric_exe_header.h"
#include <string>
#include <unordered_map>
#include <control/CControlDraw.h>
#include <CScale.h>
#include "../kuodafu_lyric.h"

#define NAMESPACE_LYRIC_WND_BEGIN namespace lyric_wnd{
#define NAMESPACE_LYRIC_WND_END }

NAMESPACE_LYRIC_WND_BEGIN

#define TIMERID_UPDATE_LYRIC 1000   // ʱ��ID, ���¸��

typedef struct LYRIC_WND_INFU
{
    HWND        hWnd;           // ��ʴ��ھ��
    LPEX_FONT   hFont;          // �滭��ʵ�����
    LPCANVAS    hCanvas;        // D2D�滭���
    LPEX_BRUSH  hbrBack;        // ����ƶ�����֮����ʾ�ĸ�ʱ�����ˢ
    LPEX_BRUSH  hbrNormal;      // ��ͨ��ʻ�ˢ
    LPEX_BRUSH  hbrHighlight;   // ������ʻ�ˢ
    HLYRIC      hLyric;         // ��ʾ��
    int         prevIndexLine;  // ��һ�λ滭�ĸ���к�
    int         prevWidth;      // ��һ�λ滭�ĸ�ʿ��
    CScale      scale;          // ���ű���
    LYRIC_WND_INFU()
    {
        hWnd = nullptr;
        hFont = nullptr;
        hCanvas = nullptr;
        hbrBack = nullptr;
        hbrNormal = nullptr;
        hbrHighlight = nullptr;
        hLyric = nullptr;
        prevIndexLine = -1;
        prevWidth = 0;
    }
}*PLYRIC_WND_INFU;

static std::unordered_map<HWND, LYRIC_WND_INFU> m_map;

HWND lyric_create_layered_window();
void lyric_wnd_draw(LYRIC_WND_INFU& wnd_info, int pos, const RECT& rcWindow, LYRIC_CALC_STRUCT& arg);
LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


NAMESPACE_LYRIC_WND_END



HWND lyric_wnd_create()
{
    using namespace lyric_wnd;

    HWND hWnd = lyric_create_layered_window();
    if (!hWnd)
        return nullptr;

    LYRIC_WND_INFU& wnd_info = m_map[hWnd];

    wnd_info.hWnd = hWnd;
    wnd_info.scale.SetDpi(wnd_info.hWnd); // ��¼��ǰ���ڵ�dpi

    wnd_info.hFont          = _font_create(L"΢���ź�", wnd_info.scale(-30));
    wnd_info.hCanvas        = _canvas_create(hWnd, 0, 0);
    wnd_info.hbrBack        = _brush_create(MAKEARGB(100, 0, 0, 0));
    wnd_info.hbrNormal      = _brush_create(MAKEARGB(255, 0, 0, 0));
    wnd_info.hbrHighlight   = _brush_create(MAKEARGB(255, 255, 0, 0));

    float width = 0, height = 0;
    _canvas_calctextsize(wnd_info.hCanvas, wnd_info.hFont, L"����", 2, DT_SINGLELINE, 0, 0, &width, &height);
    MoveWindow(hWnd, 0, 0, (int)1000, (int)(height * 2) + wnd_info.scale(20), TRUE);

    return hWnd;
}

bool lyric_wnd_load_krc(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen)
{
    using namespace lyric_wnd;

    auto it = m_map.find(hWindowLyric);
    if (it == m_map.end())
        return false;
    LYRIC_WND_INFU& wnd_info = it->second;
    HLYRIC hLyric = lyric_parse(pKrcData, nKrcDataLen);
    if (!hLyric)
        return false;
    wnd_info.hLyric = hLyric;
    lyric_calc_text_width(hLyric, [](void* pUserData, LPCWSTR pText, int nTextLen, int* pRetHeight) -> int
    {
        LYRIC_WND_INFU* pWndInfo = (LYRIC_WND_INFU*)pUserData;
        LPCANVAS hCanvas = pWndInfo->hCanvas;
        float width = 0, height = 0;
        _canvas_calctextsize(hCanvas, pWndInfo->hFont, pText, nTextLen, DT_SINGLELINE, 0, 0, &width, &height);
        *pRetHeight = (int)height;
        return (int)width;
    }, &wnd_info);
    return true;
}

bool lyric_wnd_update(HWND hWindowLyric, int nCurrentTimeMS)
{
    using namespace lyric_wnd;

    auto it = m_map.find(hWindowLyric);
    if (it == m_map.end())
        return false;
    LYRIC_WND_INFU& wnd_info = it->second;
    if (!wnd_info.hLyric)
        return false;

    LYRIC_CALC_STRUCT arg{};
    // �����������ε���Ҳ��100����벻��200, release��100��������
    // ��ȫ֧�ŵ���100֡��ˢ����
    lyric_calc(wnd_info.hLyric, nCurrentTimeMS, &arg);
    if (arg.indexLine == wnd_info.prevIndexLine && arg.nWidthWord == wnd_info.prevWidth)
        return true;    // �ϴλ滭������һ�е����λ��, ��һ�λ������, �ǾͲ���Ҫ�滭��, ֱ�ӷ���

    wnd_info.prevIndexLine = arg.indexLine;
    wnd_info.prevWidth = arg.nWidthWord;

#ifdef _DEBUG
    if (!arg.word.pText || !*arg.word.pText)
        __debugbreak();
#endif


    LPCANVAS hCanvas = wnd_info.hCanvas;
    RECT rc;
    GetWindowRect(wnd_info.hWnd, &rc);
    const int cxClient = rc.right - rc.left;
    const int cyClient = rc.bottom - rc.top;

    if ((int)hCanvas->GetWidth() != cxClient || (int)hCanvas->GetHeight() != cyClient)
        hCanvas->Resize(cxClient, cyClient);

    hCanvas->BeginDraw();
    hCanvas->clear(0);

    lyric_wnd_draw(wnd_info, nCurrentTimeMS, rc, arg);
    HDC hdcD2D = hCanvas->GetDC();
    UpdateLayered(wnd_info.hWnd, cxClient, cyClient, hdcD2D);
    hCanvas->ReleaseDC();
    hCanvas->EndDraw();

    return true;
}


NAMESPACE_LYRIC_WND_BEGIN

HWND lyric_create_layered_window()
{
    WNDCLASSEX wc;
    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.style            = 0;
    wc.lpfnWndProc      = lyric_wnd_proc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = GetModuleHandleW(0);
    wc.hIcon            = LoadIconW(NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = L"www.kuodafu.com lyric_window";
    wc.hIconSm          = wc.hIcon;

    static ATOM atom = RegisterClassExW(&wc);
    static bool init_d2d = _canvas_init(true, false);

    HWND hWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                                wc.lpszClassName, L"www.kuodafu.com",
                                WS_POPUP | WS_VISIBLE,
                                0, 0, 1, 1,
                                NULL, NULL, wc.hInstance, NULL);

    return hWnd;
}




// �滭��ص�����
void lyric_wnd_draw(LYRIC_WND_INFU& wnd_info, int pos, const RECT& rcWindow, LYRIC_CALC_STRUCT& arg)
{
    LPCANVAS hCanvas = wnd_info.hCanvas;
    LYRIC_LINE_STRUCT& line = arg.line;
    LYRIC_WORD_STRUCT& word = arg.word;


    //static int prev_width, prev_index;
    //if (prev_width == arg.nLineWidth && prev_index == arg.indexLine)
    //    return;
    //prev_width = arg.nLineWidth;
    //prev_index = arg.indexLine;

    RECT_F rcText;
    RECT_F rcText2;
    float top = 100.f;
    float left = 10.f;
    float height = 100.f;

    if (line.nWidth)
    {
        int width = word.nLeft + arg.nWidthWord;
        rcText = { left, top, left + (float)line.nWidth, top + height };
        rcText2 = { left, top, left + (float)width, top + height };
    }
    else
    {
        rcText = { left, top, left + (float)line.nWidth, top + height };
        rcText2 = rcText;
    }
    RECT_F rcBack(0.F, 0.F, (float)(rcWindow.right - rcWindow.left), (float)(rcWindow.bottom - rcWindow.top));


    hCanvas->FillRectangle(wnd_info.hbrBack, &rcBack);

    const int fmt = DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE;
    hCanvas->SetClip(rcText2.right, rcText2.top, rcText.right, rcText2.bottom);
    hCanvas->drawtext(wnd_info.hFont, wnd_info.hbrNormal, line.pText, line.nLength, fmt, &rcText);
    hCanvas->ResetClip();

    hCanvas->drawtext(wnd_info.hFont, wnd_info.hbrHighlight, line.pText, line.nLength, fmt, &rcText2);

}

LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //LYRIC_WND_INFU& wnd_info = m_map[hWnd];

    switch (message)
    {
    case WM_MOVE:
        break;
    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN:
    {
        return DefWindowProcW(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
    }
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}


NAMESPACE_LYRIC_WND_END


