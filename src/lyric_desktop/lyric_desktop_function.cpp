#include "lyric_desktop_function.h"
#include <windowsx.h>
#include "dwrite_1.h"
#include <dwmapi.h>
#include "GetMonitorRect.h"
#include <d2d/CCustomTextRenderer.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dxguid.lib")

#define _LIB

#ifdef _LIB
#   ifdef _WIN64
#      ifdef _DEBUG
#          pragma comment(lib, "output/x64/lyric_libD.lib")
#      else
#          pragma comment(lib, "output/x64/lyric_lib.lib")
#      endif
#   else
#      ifdef _DEBUG
#          pragma comment(lib, "output/x86/lyric_libD.lib")
#      else
#          pragma comment(lib, "output/x86/lyric_lib.lib")
#      endif
#   endif
#else
#   ifdef _WIN64
#      ifdef _DEBUG
#          pragma comment(lib, "output/x64/lyricD.lib")
#      else
#          pragma comment(lib, "output/x64/lyric.lib")
#      endif
#   else
#      ifdef _DEBUG
#          pragma comment(lib, "output/x86/lyricD.lib")
#      else
#          pragma comment(lib, "output/x86/lyric.lib")
#      endif
#   endif
#endif



using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN


static HCURSOR m_hCursorArrow;
static HCURSOR m_hCursorHand;
D2DInterface* g_d2d_interface;


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

bool _lyric_dwsktop_init()
{
#ifdef _DEBUG
    const bool isDebug = true;
#else
    const bool isDebug = false;
#endif
    if (!g_d2d_interface)
        g_d2d_interface = d2d_init(false, isDebug);

    if (!m_hCursorArrow)
    {
        //m_hCursorArrow = LoadCursorW(0, IDC_ARROW);
        m_hCursorArrow = LoadCursorW(0, IDC_SIZEALL);
        m_hCursorHand = LoadCursorW(0, IDC_HAND);
    }

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
    return g_d2d_interface != nullptr;
}

bool _lyric_dwsktop_uninit()
{
    UnregisterClassW(pszClassName, GetModuleHandleW(nullptr));
    return d2d_uninit(g_d2d_interface);
}

// �������ˢ���ʶ�Ӧ��ʱ�����ڣ����룬����ȡ����
inline static int GetFrameIntervalMs(int refreshRate)
{
    if (refreshRate <= 0.0)
        return 16; // Ĭ��60Hz��Ӧ������

    // 1000ms / Hz = ÿ֡����ʱ�䣨���룩
    int interval = 1000 / refreshRate;
    return interval;
}
void _ld_start_high_precision_timer(PLYRIC_DESKTOP_INFO pWndInfo)
{
    pWndInfo->Addref();
    // ��ʹ��ʱ��, ֱ�Ӵ������߳�, ����߾���ʱ�Ӵ���ʧ��, ��ʹ��Sleep����
    std::thread([](PLYRIC_DESKTOP_INFO pWndInfo)
                {
                    HANDLE hTimer = CreateWaitableTimerW(NULL, FALSE, NULL);
                    HWND hWnd = pWndInfo->hWnd;
                    while (IsWindow(hWnd))
                    {
                        int interval = GetFrameIntervalMs(pWndInfo->config.refreshRate);
                        LARGE_INTEGER liDueTime{};
                        liDueTime.QuadPart = -static_cast<LONGLONG>(interval * 10000LL);  // ���ʱ�䣨��λ100ns��
                        bool isSleep = true;

                        if (hTimer && SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, FALSE))
                        {
                            if (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0)
                                isSleep = false;
                        }
                        if (isSleep)
                            Sleep(interval);    // �߾��ȶ�ʱ������ʧ��, ����ʹ��Sleep����
                        // ��Ҫʹ�÷���, ����������Ϻ�Ž�����һ��ѭ��
                        SendMessageW(hWnd, WM_USER, 121007124, 20752843);
                    }

                    if (hTimer)
                        CloseHandle(hTimer);
                    
                    pWndInfo->Release();
                }, pWndInfo).detach();


    //// ʹ�� SetTimer(���ȴ�Լ 10~15ms)
    //SetTimer(hWnd, fallbackTimerId, 10, [](HWND hWnd, UINT, UINT_PTR, DWORD)
    //         {
    //             PostMessageW(hWnd, WM_USER, 121007124, 20752843);
    //         });
}


PLYRIC_DESKTOP_INFO _ld_create_layered_window(const char* arg, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam)
{
    HWND hWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                                pszClassName, L"",
                                WS_POPUP | WS_VISIBLE,// | WS_BORDER,
                                0, 0, 1, 1,
                                nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);

    if (!hWnd)
        return nullptr;

    //TODO: �����ʼ����ʵ�Ĭ������, ������ݽ���������, �Ͱ����ݽ��������ô���, ����Ͱ�Ĭ�����ô���

    auto pWndInfo = new LYRIC_DESKTOP_INFO;
    lyric_wnd_set_data(hWnd, pWndInfo);

    pWndInfo->init(hWnd, arg, pfnCommand, lParam);

    // ����һ����ʾ����, ������ʾ��ʾ��Ϣ
    pWndInfo->hTips = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
                                      TOOLTIPS_CLASSW,
                                      nullptr,
                                      TTS_ALWAYSTIP,
                                      0, 0, 0, 0, 0, 0, 0, 0);

    SendMessageW(pWndInfo->hTips, TTM_SETMAXTIPWIDTH, 0, 500);
    SendMessageW(pWndInfo->hTips, TTM_SETDELAYTIME, TTDT_AUTOPOP, 0x7fff);
    TTTOOLINFOW ti = { 0 };
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
    ti.lpszText = (LPWSTR)L"";
    ti.hwnd = pWndInfo->hWnd;
    ti.uId = (UINT_PTR)pWndInfo->hWnd;
    auto isInsertSuccess = SendMessageW(pWndInfo->hTips, TTM_ADDTOOLW, 0, (LPARAM)&ti);


    RECT rc = pWndInfo->has_mode(LYRIC_DESKTOP_MODE::VERTICAL) ? pWndInfo->config.rect_v : pWndInfo->config.rect_h;
    // �����Ǵ���, ������Ҫ����һ��, ���������Ҳ�ǰ�100%���ű����
    pWndInfo->scale(rc);

    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;

    const RECT& rcDesk = pWndInfo->rcMonitor;
    const int cxScreen = rcDesk.right - rcDesk.left;
    const int cyScreen = rcDesk.bottom - rcDesk.top;

    if (rc.top > cyScreen - pWndInfo->scale(250))
        rc.top = cyScreen - pWndInfo->scale(250), rc.bottom = rc.top + height;
    if (rc.left < rcDesk.left)
        rc.left = (cxScreen - width) / 2, rc.right = rc.left + width;


    // �����߾��ȶ�ʱ��, ��Ҫ10�������ڵľ���, ʱ�����ֻ��10ms, ������Ҫ��
    //TODO Ҫ������Ч�ʵĻ�Ӧ��������Ҫ�ػ���ʱ��Ͷ����ϢȻ���ٴ���
    // ���������������鷳һ��, �ɴ���ڲ�һֱ�����ػ���
    _ld_start_high_precision_timer(pWndInfo);

    MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, pWndInfo->nMinHeight, TRUE);

    return pWndInfo;
}

bool _lyric_desktop_load_lyric(PLYRIC_DESKTOP_INFO pWndInfo, LPCVOID pData, int nSize, LYRIC_PARSE_TYPE nType)
{
    CCriticalSection cs(pWndInfo->pCritSec);    // ����, ��ֹ���ﲻ�Ǵ����̵߳���, �����̻߳᲻ͣ�ķ��ʸ�ʽṹ

    lyric_destroy(pWndInfo->hLyric);
    pWndInfo->hLyric = nullptr;

    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;

    wnd_info.hLyric = lyric_parse(pData, nSize, nType);
    pWndInfo->nTimeOffset = lyric_behind_ahead(wnd_info.hLyric, 0);
    pWndInfo->line1.clear();
    pWndInfo->line2.clear();

    int language = lyric_get_language(wnd_info.hLyric);
    lyric_wnd_set_state_translate(*pWndInfo, language);

    if (language)
        wnd_info.add_mode(LYRIC_DESKTOP_MODE::EXISTTRANS);
    else
        wnd_info.del_mode(LYRIC_DESKTOP_MODE::EXISTTRANS);

    if (!wnd_info.hLyric)
        return false;

    // �������С����, ��Ȼ����ֶ��˻������ܶ������
    lyric_calc_text(wnd_info.hLyric, [](void* pUserData, LPCWSTR pText, int nTextLen, float* pRetHeight) -> float
                    {
                        auto pWndInfo = (LYRIC_DESKTOP_INFO*)pUserData;
                        D2DRender& pRender = *pWndInfo->dx.pRender;
                        // û�д�������ʹ���һ��, �������豸�޹ض���, ����Ҫ����Ⱦ�����ﴴ��
                        if (!pWndInfo->dx.hFont)
                            pWndInfo->dx.re_create_font(pWndInfo);

                        CComPtr<IDWriteTextLayout> pTextLayout;
                        lyric_wnd_create_text_layout(pText, nTextLen, pWndInfo->dx.hFont->GetDWTextFormat(), 0, 0, &pTextLayout);
                        float width = _lyric_wnd_load_krc_calc_text(pWndInfo, pTextLayout, pRetHeight);
                        return width;
                    }, pWndInfo);
    return true;
}


bool lyric_wnd_invalidate(LYRIC_DESKTOP_INFO& wnd_info)
{
    // ����, ���Խ���, �ܽ����˵��û�����޸ĸ����Ϣ, ����ִ��
    CCriticalSection cs(wnd_info.pCritSec, std::adopt_lock_t());
    if (!cs.TryLock())
        return false;   // ����ʧ�ܾͲ���������

    // ��ʼ��DX����, ����Ѿ���ʼ����ֱ�ӷ���
    // ��Ϊ�滭�����п��ܻ᷵���豸ʧЧ������, Ȼ�����������DX����
    // ���Խ���滭ǰ�ȳ�ʼ��һ��
    wnd_info.dx.init(&wnd_info);

    if (!wnd_info.dx.pRender)
        return false;   // ��ʼ��ʧ�ܾͲ��ܼ�������

    D2DRender& pRender = *wnd_info.dx.pRender;
    RECT& rcWindow = wnd_info.rcWindow;
    GetWindowRect(wnd_info.hWnd, &rcWindow);
    //GetClientRect(wnd_info.hWnd, &rcWindow);
    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;

    int bmpWidth = 0, bmpHeight = 0;
    pRender.GetSize(&bmpWidth, &bmpHeight);

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
    if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL))
        bLight = arg.nHeightWord != wnd_info.prevHeight;
    else
        bLight = arg.nWidthWord != wnd_info.prevWidth;
    const bool isDrawString = arg.indexLine != wnd_info.prevIndexLine || bLight;
    const bool isDraw = isDrawString || wnd_info.change || wnd_info.config.debug.alwaysCache;

    // ������Ҫ�ж�һ���Ƿ���Ҫ����, �����ťû�仯, ����ǿ�Ƹ���, ���Ҹ���ı�Ҳû��, �ǾͲ���Ҫ�ػ�
    if (!isDraw)
        return true;

    HRESULT hr = lyric_wnd_OnPaint(wnd_info, isresize, arg);

    wnd_info.change = 0;    // �����иı�ı�־λ����
    wnd_info.prevIndexLine  = arg.indexLine;
    wnd_info.prevWidth      = arg.nWidthWord;
    wnd_info.prevHeight     = arg.nHeightWord;
    return SUCCEEDED(hr);
}

LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _lyric_desktop_get_data(hWnd);
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
    case WM_USER:
    {
        if (wParam == 121007124 && lParam == 20752843)
        {
            PLYRIC_DESKTOP_INFO pWndInfo = _lyric_desktop_get_data(hWnd);
            lyric_wnd_invalidate(*pWndInfo);
            return 0;
        }
        break;
    }
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
        RECT* rect = pWndInfo->has_mode(LYRIC_DESKTOP_MODE::VERTICAL)
            ? &pWndInfo->config.rect_v
            : &pWndInfo->config.rect_h;

        // ֱ�ӻ�ȡ, ʡ��, һ�д���㶨
        GetWindowRect(hWnd, rect);

        //if (message == WM_MOVE)
        //{
        //    const int width = rect->right - rect->left;
        //    const int height = rect->bottom - rect->top;
        //    rect->left = GET_X_LPARAM(lParam);
        //    rect->top = GET_Y_LPARAM(lParam);
        //    rect->right = rect->left + width;
        //    rect->bottom = rect->top + height;
        //}
        //else
        //{
        //    rect->right = rect->left + LOWORD(lParam);
        //    rect->bottom = rect->top + HIWORD(lParam);
        //}

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
        pWndInfo->Release();
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
        auto wnd_pos = (WINDOWPOS*)lParam;
        if ((wnd_pos->flags & SWP_NOMOVE) == SWP_NOMOVE)
            break;  // �в��ƶ��ı�־, ������

        RECT* rect = pWndInfo->has_mode(LYRIC_DESKTOP_MODE::VERTICAL)
            ? &pWndInfo->config.rect_v
            : &pWndInfo->config.rect_h;
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

        const int width = rect->right - rect->left;
        const int height = rect->bottom - rect->top;

        if (wnd_pos->x < rcMonitorAll.left)
            wnd_pos->x = rcMonitorAll.left;   // ���ƴ�����಻�ܳ�����߽�
        else if (wnd_pos->x + width > rcMonitorAll.right)
            wnd_pos->x = rcMonitorAll.right - width;  // ���ƴ����Ҳ಻�ܳ����ұ߽�

        if (wnd_pos->y < rcMonitorAll.top)
            wnd_pos->y = rcMonitorAll.top;    // ���ƴ��ڶ������ܳ����ϱ߽�
        else if (wnd_pos->y + height > rcMonitorAll.bottom)
            wnd_pos->y = rcMonitorAll.bottom - height;   // ���ƴ��ڵײ����ܳ����±߽�

        //wnd_pos->cx = pos->width;
        //wnd_pos->cy = pos->height;

        break;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMMI = (MINMAXINFO*)lParam;
        const bool is_vertical = pWndInfo->has_mode(LYRIC_DESKTOP_MODE::VERTICAL);
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
    {
        LRESULT ret = 0;
        if (lyric_wnd_proc_custom_message(pWndInfo, message, wParam, lParam, ret))
            return ret;
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
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
    
    const bool is_vertical = wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL);
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
    IDWriteFactory* pDWriteFactory = g_d2d_interface->GetD2DWriteFactory();
    HRESULT hr = pDWriteFactory->CreateTextLayout(str, (UINT32)len, dxFormat,
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


float _lyric_wnd_load_krc_calc_text(PLYRIC_DESKTOP_INFO pWndInfo, IDWriteTextLayout* pTextLayout, float* pHeight)
{
    *pHeight = 0;

    if (!pTextLayout)
        return 0.f;
    const bool is_vertical = pWndInfo->has_mode(LYRIC_DESKTOP_MODE::VERTICAL);
    if (!is_vertical)
    {
        // ��������, ֱ�Ӽ���Ȼ�󷵻�
        CCustomTextRenderer renderer;
        pTextLayout->Draw(nullptr, &renderer, 0, 0);
        *pHeight = renderer.get_height();
        return renderer.get_width();
    }

    // �ߵ������������, ��Ҫ�������, ��Ϊ�����Ӣ������ ��Щ�ַ�����ת, ��Ҫ�ر����
    // ֻ��Ҫ����߶�, ����������ַ��������Ǹ�, �߶����ۼ�, ����ת�ļӿ��, û��ת�ļӸ߶�

    float width = 0.f, height = 0.f;

    CCustomTextRenderer renderer([&](void* clientDrawingContext,
                                     FLOAT baselineOriginX,
                                     FLOAT baselineOriginY,
                                     DWRITE_MEASURING_MODE measuringMode,
                                     DWRITE_GLYPH_RUN const* glyphRun,
                                     DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                                     IUnknown* clientDrawingEffect)
                                 {
                                     // ��ȡ�������
                                     for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
                                     {
                                         DWRITE_FONT_METRICS metrics{};
                                         glyphRun->fontFace->GetMetrics(&metrics);

                                         float designUnitsPerEm = (float)metrics.designUnitsPerEm;
                                         float _height = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;
                                         float _width = (glyphRun->glyphAdvances ? glyphRun->glyphAdvances[i] : 0.0f);

                                         if (width < _width)
                                             width = _width; // ���ֻ��¼���Ŀ��, ������ʿ���ǹ̶���

                                         wchar_t ch = glyphRunDescription && glyphRunDescription->string ? glyphRunDescription->string[i] : L'\0';
                                         bool is_alpha = isLatinCharacter(ch);
                                         if (is_alpha)
                                             height += _width;   // ��Ҫ��ת���ַ��ͼ��Ͽ��
                                         else
                                             height += _height;  // ����ת���ַ��ͼ��ϸ߶�
                                     }

                                     return S_OK;
                                 }
    );
    pTextLayout->Draw(nullptr, &renderer, 0, 0);

    *pHeight = height;
    return width;
}


NAMESPACE_LYRIC_DESKTOP_END




