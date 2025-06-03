#include "lyric_wnd_function.h"
#include <windowsx.h>
#include "dwrite_1.h"
#include <dwmapi.h>
#include "GetMonitorRect.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace NAMESPACE_D2D;

NAMESPACE_LYRIC_DESKTOP_BEGIN


static HCURSOR m_hCursorArrow;
static HCURSOR m_hCursorHand;

#define TIMER_ID_LEAVE 1000



LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnTimer(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnHitTest(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnMouseMove(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnLbuttonDown(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnCaptureChanged(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);


static bool init_dpi()
{
    typedef HRESULT(WINAPI* pfn_SetProcessDpiAwareness)(int value);
    typedef HRESULT(WINAPI* pfn_SetProcessDpiAwarenessContext)(DPI_AWARENESS_CONTEXT value);
    typedef DPI_AWARENESS_CONTEXT(WINAPI* pfn_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT dpiContext);


    HMODULE Shcore = LoadLibraryW(L"Shcore.dll");
    HMODULE hUser32 = LoadLibraryW(L"user32.dll");
    if (!Shcore || !hUser32)
        return false;

    // win10��֧�ֵ�����dpi��ʽ
    auto pfnSetProcessDpiAwareness = (pfn_SetProcessDpiAwareness)GetProcAddress(Shcore, "SetProcessDpiAwareness");
    auto pfnSetProcessDpiAwarenessContext = (pfn_SetProcessDpiAwarenessContext)GetProcAddress(Shcore, "SetProcessDpiAwarenessContext");
    auto pfnSetThreadDpiAwarenessContext = (pfn_SetThreadDpiAwarenessContext)GetProcAddress(hUser32, "SetThreadDpiAwarenessContext");
    if (pfnSetProcessDpiAwareness)
    {
        enum PROCESS_DPI_AWARENESS
        {
            PROCESS_DPI_UNAWARE = 0,    // DPI ��֪���� ��Ӧ�ò������� DPI ���ģ�����ʼ�ռٶ������ϵ��Ϊ 100% (96 DPI) �� ϵͳ�����κ����� DPI �������Զ�������
            PROCESS_SYSTEg_dpi_AWARE = 1,    // ͳ DPI ��֪�� ��Ӧ�ò������� DPI ���ġ� ������ѯ DPI һ�Σ�����Ӧ�õ���������ʹ�ø�ֵ�� ��� DPI �������ģ�Ӧ�ý��������Ϊ�µ� DPI ֵ�� �� DPI ��ϵͳֵ��������ʱ��ϵͳ���Զ�������չ����������
            PROCESS_PER_MONITOR_DPI_AWARE = 2     // �������� DPI ��֪�� ��Ӧ���ڴ��� DPI ʱ��� DPI������ DPI ��������ʱ��������ϵ���� ϵͳ�����Զ�������ЩӦ�ó���
        };

        // ��֪�����Ļ��dpi
        HRESULT hr = 0;

        if (pfnSetProcessDpiAwarenessContext)
        {
            hr = pfnSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
        else
        {
            hr = pfnSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
            if (pfnSetThreadDpiAwarenessContext)
                pfnSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }


        if (FAILED(hr))
        {
#ifdef _DEBUG
            __debugbreak();
#endif
            return false;
        }
    }
    else
    {
        // win10���µ�����dpi��ʽ, ���ܸ�֪�����Ļ
        BOOL bRet = SetProcessDPIAware();
    }

    return true;
}

constexpr LPCWSTR pszClassName = L"www.kuodafu.com lyric_desktop_window";
bool _ld_init()
{
#ifdef _DEBUG
    const bool isDebug = true;
#else
    const bool isDebug = false;
#endif
    static bool init_d2d = d2d::d2d_init(isDebug);

    WNDCLASSEX wc;
    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.style            = 0;
    wc.lpfnWndProc      = lyric_wnd_proc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = sizeof(PVOID);    // ���ڶ���һ��PVOIDָ��
    wc.hInstance        = GetModuleHandleW(0);
    wc.hIcon            = LoadIconW(NULL, IDI_APPLICATION);
    wc.hCursor          = m_hCursorArrow;
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = pszClassName;
    wc.hIconSm          = wc.hIcon;

    static ATOM atom = RegisterClassExW(&wc);
    init_dpi();
    return init_d2d;
}

bool _ld_uninit()
{
    UnregisterClassW(pszClassName, GetModuleHandleW(0));
    return d2d_uninit();
}

HWND lyric_create_layered_window(const LYRIC_DESKTOP_ARG* arg)
{
    if (!m_hCursorArrow)
    {
        //m_hCursorArrow = LoadCursorW(0, IDC_ARROW);
        m_hCursorArrow = LoadCursorW(0, IDC_SIZEALL);
        m_hCursorHand = LoadCursorW(0, IDC_HAND);
    }


    HWND hWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                                pszClassName, L"",
                                WS_POPUP | WS_VISIBLE,// | WS_BORDER,
                                0, 0, 1, 1,
                                NULL, NULL, GetModuleHandleW(0), NULL);


    if (!hWnd)
        return nullptr;

    SetTimer(hWnd, 2000, 10, [](HWND hWnd, UINT message, UINT_PTR id, DWORD t)
    {
        PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWnd);
        lyric_wnd_invalidate(*pWndInfo);
    });

    //DWMNCRENDERINGPOLICY pv = DWMNCRP_ENABLED;
    //HRESULT hr = DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &pv, sizeof(pv));

    return hWnd;
}

// �滭��ʱ��û�д�������, �Ǿ���Ҫ����Ĭ�϶���
void lyric_wnd_default_object(LYRIC_DESKTOP_INFO& wnd_info)
{
    wnd_info.dx.re_create(&wnd_info);
}


bool lyric_wnd_invalidate(LYRIC_DESKTOP_INFO& wnd_info)
{
    // ����, ���Խ���, �ܽ����˵��û�����޸ĸ����Ϣ, ����ִ��
    CCriticalSection cs(wnd_info.pCritSec, std::adopt_lock_t());
    if (!cs.TryLock())
        return false;   // ����ʧ�ܾͲ���������

    //static bool asdas;
    //if (asdas)
    //    return true;
    //asdas = true;


    if (!wnd_info.dx.hCanvas)
        lyric_wnd_default_object(wnd_info);
    if (!wnd_info.dx.hCanvas)
        return false;

    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    RECT& rcWindow = wnd_info.rcWindow;
    GetWindowRect(wnd_info.hWnd, &rcWindow);
    //GetClientRect(wnd_info.hWnd, &rcWindow);
    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;

    int bmpWidth = 0, bmpHeight = 0;
    hCanvas.getsize(&bmpWidth, &bmpHeight);

    bool isresize = false;
    if (cxClient != bmpWidth || cyClient != bmpHeight)
        wnd_info.change_wnd = true, isresize = true;


    LYRIC_CALC_STRUCT arg = {0};
    // �����������ε���Ҳ��100����벻��200, release��100��������
    // ��ȫ֧�ŵ���100֡��ˢ����

    //wnd_info.nCurrentTimeMS = 38048;
    lyric_calc(wnd_info.hLyric, wnd_info.nCurrentTimeMS, &arg);

    // �Ƿ���Ҫ�滭����ı�, �������ϴμ�¼�Ĳ�һ��, ���߿�Ȳ�һ��, �Ǿ�Ϊ��, ��Ҫ�ػ�
    bool bLight = false;
    if (wnd_info.has_mode(LYRIC_MODE::VERTICAL))
        bLight = arg.nHeightWord != wnd_info.prevHeight;
    else
        bLight = arg.nWidthWord != wnd_info.prevWidth;
    const bool isDrawString = arg.indexLine != wnd_info.prevIndexLine || bLight;
    const bool isDraw = isDrawString || wnd_info.change;
    

    // ������Ҫ�ж�һ���Ƿ���Ҫ����, �����ťû�仯, ����ǿ�Ƹ���, ���Ҹ���ı�Ҳû��, �ǾͲ���Ҫ�ػ�
    if (!isDraw)
        return true;

    HRESULT hr = lyric_wnd_OnPaint(wnd_info, isresize, arg);

    wnd_info.change = 0;    // �����иı�ı�־λ����
    wnd_info.prevIndexLine = arg.indexLine;
    wnd_info.prevWidth = arg.nWidthWord;
    wnd_info.prevHeight = arg.nHeightWord;
    return SUCCEEDED(hr);
}


LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWnd);
    if (!pWndInfo)
        return DefWindowProcW(hWnd, message, wParam, lParam);

    //return DefWindowProcW(hWnd, message, wParam, lParam);

    switch (message)
    {
    //case WM_CREATE:
    //{
    //    //DWMNCRENDERINGPOLICY pv = DWMNCRP_ENABLED;
    //    //HRESULT hr = DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &pv, sizeof(pv));
    //    return 0;
    //}
    case WM_DPICHANGED:
    {
        pWndInfo->dpi_change(hWnd);
        break;
    }
    case WM_MOUSEMOVE:
    {
        LRESULT ret = lyric_wnd_OnMouseMove(*pWndInfo, message, wParam, lParam);
        // ����ƶ���������, �ж���û�л滭����, û�еĻ��ͻ滭һ��
        if (!pWndInfo->isFillBack)
        {
            pWndInfo->isFillBack = true;
            pWndInfo->change_wnd = true;
            SetTimer(hWnd, TIMER_ID_LEAVE, 50, 0);
        }
        return ret;
    }
    case WM_LBUTTONUP:
        ReleaseCapture();
        return 0;
    case WM_LBUTTONDOWN:
        return lyric_wnd_OnLbuttonDown(*pWndInfo, message, wParam, lParam);
    case WM_CAPTURECHANGED:
        return lyric_wnd_OnCaptureChanged(*pWndInfo, message, wParam, lParam);
    case WM_MOVE:
    case WM_SIZE:
    {
        LYRIC_DESKTOP_POS* pos = pWndInfo->has_mode(LYRIC_MODE::VERTICAL)
            ? &pWndInfo->pos_v
            : &pWndInfo->pos_h;

        if (message == WM_MOVE)
        {
            pos->left = GET_X_LPARAM(lParam);
            pos->top = GET_Y_LPARAM(lParam);
        }
        else
        {
            pos->width = LOWORD(lParam);
            pos->height = HIWORD(lParam);
        }
        pos->right = pos->left + pos->width;
        pos->bottom = pos->top + pos->height;
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        lyric_wnd_invalidate(*pWndInfo);
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_TIMER:
        return lyric_wnd_OnTimer(*pWndInfo, message, wParam, lParam);
    case WM_DESTROY:
    {
        lyric_wnd_set_data(hWnd, 0);
        delete pWndInfo;
        return 0;
    }
    //case WM_NCCALCSIZE:
    //{
    //    if (wParam)
    //    {
    //        NCCALCSIZE_PARAMS* pParams = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
    //        pParams->rgrc[0].top += 1;
    //        pParams->rgrc[0].left += 1;
    //        pParams->rgrc[0].right -= 1;
    //        pParams->rgrc[0].bottom -= 1;
    //    }
    //    return 0;
    //}
    case WM_DISPLAYCHANGE:
    {
        pWndInfo->get_monitor();
        break;
    }
    case WM_WINDOWPOSCHANGING:
    {
        // ���ƴ��ڲ��ܳ�����Ļ��Χ
        auto pPos = (WINDOWPOS*)lParam;
        if ((pPos->flags & SWP_NOMOVE) == SWP_NOMOVE)
            break;  // �в��ƶ��ı�־, ������

        LYRIC_DESKTOP_POS* pos = pWndInfo->has_mode(LYRIC_MODE::VERTICAL)
            ? &pWndInfo->pos_v
            : &pWndInfo->pos_h;
        const RECT& rcMonitorAll = pWndInfo->rcMonitor;

        //POINT pt;
        //GetCursorPos(&pt);
        //const RECT* rcMonitor = nullptr;
        //for (const RECT& rc : pWndInfo->rcMonitors)
        //{
        //    if (PtInRect(&rc, pt))
        //    {
        //        rcMonitor = &rc;
        //        break;
        //    }
        //}
        //if (!rcMonitor)
        //    rcMonitor = &rcMonitorAll;


        if (pPos->x < rcMonitorAll.left)
            pPos->x = rcMonitorAll.left;   // ���ƴ�����಻�ܳ�����߽�
        else if (pPos->x + pos->width > rcMonitorAll.right)
            pPos->x = rcMonitorAll.right - pos->width; // ���ƴ����Ҳ಻�ܳ����ұ߽�

        if (pPos->y < rcMonitorAll.top)
            pPos->y = rcMonitorAll.top;    // ���ƴ��ڶ������ܳ����ϱ߽�
        else if (pPos->y + pos->height > rcMonitorAll.bottom)
            pPos->y = rcMonitorAll.bottom - pos->height;   // ���ƴ��ڵײ����ܳ����±߽�


        break;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMMI = (MINMAXINFO*)lParam;
        const bool is_vertical = pWndInfo->has_mode(LYRIC_MODE::VERTICAL);
        RECT rcWindow;
        GetWindowRect(hWnd, &rcWindow);
        const int cxClient = rcWindow.right - rcWindow.left;
        const int cyClient = rcWindow.bottom - rcWindow.top;

        if (is_vertical)
        {
            pMMI->ptMinTrackSize.y = pWndInfo->nMinHeight;
            pMMI->ptMinTrackSize.x = pWndInfo->nMinWidth;   // ��Ȳ��øı�, ����������Զ��ı�
            pMMI->ptMaxTrackSize.x = pWndInfo->nMinWidth;

        }
        else
        {
            pMMI->ptMinTrackSize.x = pWndInfo->nMinWidth;
            pMMI->ptMinTrackSize.y = pWndInfo->nMinHeight;  // �߶Ȳ��øı�, ����������Զ��ı�
            pMMI->ptMaxTrackSize.y = pWndInfo->nMinHeight;
        }

        return 0;
    }
    case WM_NCHITTEST:
        return lyric_wnd_OnHitTest(*pWndInfo, message, wParam, lParam);
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT lyric_wnd_OnTimer(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND hWnd = wnd_info.hWnd;
    switch (wParam)
    {
    case TIMER_ID_LEAVE:
    {
        HWND capWindow = GetCapture();
        if (capWindow == wnd_info.hWnd)
            break;  // ��ǰ�ǲ���״̬, ������
        RECT rc;
        POINT pt;
        GetWindowRect(hWnd, &rc);
        GetCursorPos(&pt);
        if (!PtInRect(&rc, pt))
        {
            wnd_info.isFillBack = false;
            wnd_info.change_wnd = true;
            KillTimer(hWnd, TIMER_ID_LEAVE);
            lyric_wnd_button_leave(wnd_info);
        }
        break;
    }
    default:
        return DefWindowProcW(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
    }
    return 0;
}

LRESULT lyric_wnd_OnHitTest(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
{
    // �����Ե���Ŀ��
    const int borderWidth = wnd_info.scale(12) + (int)wnd_info.shadowRadius;

    // ��ȡ�����Ļ����
    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // ת��Ϊ���ڿͻ�������
    RECT rcWindow;
    GetWindowRect(wnd_info.hWnd, &rcWindow);
    
    const bool is_vertical = wnd_info.has_mode(LYRIC_MODE::VERTICAL);
    if (is_vertical)
    {
        // ����, ֻ�����������, ��Ȳ��õ���
        if (pt.y <= rcWindow.top + borderWidth)
            return HTTOP;          // �ϱ�Ե
        if (pt.y >= rcWindow.bottom - borderWidth)
            return HTBOTTOM;       // �±�Ե
    }
    else
    {
        // ֻ�����������, �߶Ȳ��õ���
        if (pt.x <= rcWindow.left + borderWidth)
            return HTLEFT;         // ���Ե
        if (pt.x >= rcWindow.right - borderWidth)
            return HTRIGHT;        // �ұ�Ե
    }

    //if (PtInRect(&wnd_info.button.rc, pt))
    //    return HTCLIENT;    // ����ڰ�ť��Χ��, ���ؿͻ�������

    //// �������Ƿ��ڴ��ڱ�Ե
    //if (pt.x <= rcWindow.left + borderWidth && pt.y <= rcWindow.top + borderWidth)
    //    return HTTOPLEFT;      // ���Ͻ�
    //if (pt.x >= rcWindow.right - borderWidth && pt.y <= rcWindow.top + borderWidth)
    //    return HTTOPRIGHT;     // ���Ͻ�
    //if (pt.x <= rcWindow.left + borderWidth && pt.y >= rcWindow.bottom - borderWidth)
    //    return HTBOTTOMLEFT;    // ���½�
    //if (pt.x >= rcWindow.right - borderWidth && pt.y >= rcWindow.bottom - borderWidth)
    //    return HTBOTTOMRIGHT;   // ���½�
    //if (pt.x <= rcWindow.left + borderWidth)
    //    return HTLEFT;         // ���Ե
    //if (pt.x >= rcWindow.right - borderWidth)
    //    return HTRIGHT;        // �ұ�Ե
    //if (pt.y <= rcWindow.top + borderWidth)
    //    return HTTOP;          // �ϱ�Ե
    //if (pt.y >= rcWindow.bottom - borderWidth)
    //    return HTBOTTOM;       // �±�Ե

    // Ĭ�Ϸ��ؿͻ���
    return HTCLIENT;
}

static int lyric_wnd_pt2index(LYRIC_DESKTOP_INFO& wnd_info, const POINT& pt)
{
    int index = -1;
    for (auto& item : wnd_info.button.rcBtn)
    {
        index++;
        if (PtInRect(&item.rc, pt))
            return index;
    }
    return -1;
}
// ����ƶ�, ���ж��ǲ����ڰ�ť��
LRESULT lyric_wnd_OnMouseMove(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    int index = lyric_wnd_pt2index(wnd_info, pt);

    if (index != -1)
    {
        // ����ڰ�ť��, �ж���һ���ǲ����������ť��
        // ���ߵ�ǰ�а����˰�ť, Ȼ���ƶ������
        if (index == wnd_info.button.index || wnd_info.button.indexDown != -1)
            return 0;   // ����һ�ν���İ�ť��, ������

        // ��Ҫ�жϰ�ť�Ƿ��ǽ�ֹ״̬, �ǵĻ�������
        if (__query(wnd_info.button.rcBtn[index].state, LYRIC_DESKTOP_BUTTON_STATE_DISABLE))
        {

            return 0;   // ��ǰ��ť�ǽ�ֹ״̬, ������
        }
        wnd_info.change_btn = true;
        wnd_info.button.index = index;
        SetClassLongPtrW(wnd_info.hWnd, GCLP_HCURSOR, (LONG_PTR)m_hCursorHand);
        lyric_wnd_button_hover(wnd_info);

    }
    else
    {
        // ��겻�ڰ�ť��, ��״̬ȥ��
        if (wnd_info.button.index != -1)
        {
            int index = wnd_info.button.index;
            wnd_info.change_btn = true;
            wnd_info.button.index = -1;
            SetClassLongPtrW(wnd_info.hWnd, GCLP_HCURSOR, (LONG_PTR)m_hCursorArrow);
            lyric_wnd_button_leave(wnd_info);
        }

    }

    return 0;
}

LRESULT lyric_wnd_OnLbuttonDown(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (wnd_info.button.index != -1)
    {
        // ����ڰ�ť�а�����, ���ﲶ��һ�����, Ȼ������ſ���ʱ��Ŵ����¼�
        SetCapture(wnd_info.hWnd);
        wnd_info.button.indexDown = wnd_info.button.index;
        wnd_info.change_btn = true;
        return 0;
    }
    return DefWindowProcW(wnd_info.hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
}
LRESULT lyric_wnd_OnCaptureChanged(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (wnd_info.button.indexDown != -1)
    {
        // ֮ǰ�а��¹�, �����жϷſ���λ�û��ǲ�����ԭ���İ�ť��
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(wnd_info.hWnd, &pt);
        int index = lyric_wnd_pt2index(wnd_info, pt);

        if (index == wnd_info.button.indexDown)
        {
            // �ſ���λ����ԭ���İ�ť��, �����¼�
            lyric_wnd_button_click(wnd_info);
        }
        else
        {
            // �ſ���ʱ����ԭ���İ�ť��, ȡ������
        }
        wnd_info.button.indexDown = -1;
        // �ſ���ʱ����Ҫ�ػ�һ��, ԭ���ǻ滭���µ�״̬, ���ڰѰ�����������֮��ȥ�ػ���
        wnd_info.change_btn = true;
    }
    return 0;
}

bool lyric_wnd_create_text_layout(LPCWSTR str, int len, IDWriteTextFormat* dxFormat, float layoutWidth, float layoutHeight, IDWriteTextLayout** ppTextLayout)
{
    if (!str)
        return false;
    IDWriteTextLayout* pDWriteTextLayout = 0;
    auto& d2dInfo = d2d_get_info();
    HRESULT hr = d2dInfo.pDWriteFactory->CreateTextLayout(str, (UINT32)len, dxFormat,
                                                          layoutWidth, layoutHeight, &pDWriteTextLayout);
    if (FAILED(hr))
        return false;

    pDWriteTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);           // ������
    pDWriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);         // �����
    pDWriteTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);  // ������
    //pDWriteTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    //// ��ѯ IDWriteTextLayout1 �ӿ�
    //CComPtr<IDWriteTextLayout1> pTextLayout1 = nullptr;
    //hr = pDWriteTextLayout->QueryInterface(IID_PPV_ARGS(&pTextLayout1));
    //// ����������
    //if (SUCCEEDED(hr) && pTextLayout1)
    //{
    //    DWRITE_TEXT_RANGE range = { 0, (UINT32)len };
    //    hr = pTextLayout1->SetCharacterSpacing(
    //        1.0f,  // ǰ����� (�ַ�ǰ��ӵĿռ�)
    //        1.0f,  // β���� (�ַ�����ӵĿռ�)
    //        1.0f,  // ��Сǰ����� (�ַ�����С�ܿ��)
    //        range);
    //}
    *ppTextLayout = pDWriteTextLayout;
    return true;
}

NAMESPACE_LYRIC_DESKTOP_END




