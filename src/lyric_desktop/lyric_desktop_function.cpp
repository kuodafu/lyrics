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

    // win10才支持的设置dpi方式
    auto pfnSetProcessDpiAwareness = (pfn_SetProcessDpiAwareness)GetProcAddress(Shcore, "SetProcessDpiAwareness");
    auto pfnSetProcessDpiAwarenessContext = (pfn_SetProcessDpiAwarenessContext)GetProcAddress(Shcore, "SetProcessDpiAwarenessContext");
    auto pfnSetThreadDpiAwarenessContext = (pfn_SetThreadDpiAwarenessContext)GetProcAddress(hUser32, "SetThreadDpiAwarenessContext");
    if (pfnSetProcessDpiAwareness)
    {
        enum PROCESS_DPI_AWARENESS
        {
            PROCESS_DPI_UNAWARE = 0,    // DPI 不知道。 此应用不会缩放 DPI 更改，并且始终假定其比例系数为 100% (96 DPI) 。 系统将在任何其他 DPI 设置上自动缩放它
            PROCESS_SYSTEg_dpi_AWARE = 1,    // 统 DPI 感知。 此应用不会缩放 DPI 更改。 它将查询 DPI 一次，并在应用的生存期内使用该值。 如果 DPI 发生更改，应用将不会调整为新的 DPI 值。 当 DPI 与系统值发生更改时，系统会自动纵向扩展或缩减它。
            PROCESS_PER_MONITOR_DPI_AWARE = 2     // 按监视器 DPI 感知。 此应用在创建 DPI 时检查 DPI，并在 DPI 发生更改时调整比例系数。 系统不会自动缩放这些应用程序
        };

        // 感知多个屏幕的dpi
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
        // win10以下的设置dpi方式, 不能感知多个屏幕
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
    wc.cbWndExtra       = sizeof(PVOID);    // 窗口多存放一个PVOID指针
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

// 计算给定刷新率对应的时钟周期（毫秒，向下取整）
inline static int GetFrameIntervalMs(int refreshRate)
{
    if (refreshRate <= 0.0)
        return 16; // 默认60Hz对应的周期

    // 1000ms / Hz = 每帧所需时间（毫秒）
    int interval = 1000 / refreshRate;
    return interval;
}
void _ld_start_high_precision_timer(PLYRIC_DESKTOP_INFO pWndInfo)
{
    pWndInfo->Addref();
    // 不使用时钟, 直接创建个线程, 如果高精度时钟创建失败, 就使用Sleep代替
    std::thread([](PLYRIC_DESKTOP_INFO pWndInfo)
                {
                    HANDLE hTimer = CreateWaitableTimerW(NULL, FALSE, NULL);
                    HWND hWnd = pWndInfo->hWnd;
                    while (IsWindow(hWnd))
                    {
                        int interval = GetFrameIntervalMs(pWndInfo->config.refreshRate);
                        LARGE_INTEGER liDueTime{};
                        liDueTime.QuadPart = -static_cast<LONGLONG>(interval * 10000LL);  // 相对时间（单位100ns）
                        bool isSleep = true;

                        if (hTimer && SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, FALSE))
                        {
                            if (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0)
                                isSleep = false;
                        }
                        if (isSleep)
                            Sleep(interval);    // 高精度定时器处理失败, 这里使用Sleep代替
                        // 需要使用发送, 让他处理完毕后才进行下一次循环
                        SendMessageW(hWnd, WM_USER, 121007124, 20752843);
                    }

                    if (hTimer)
                        CloseHandle(hTimer);
                    
                    pWndInfo->Release();
                }, pWndInfo).detach();


    //// 使用 SetTimer(精度大约 10~15ms)
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

    //TODO: 这里初始化歌词的默认配置, 如果传递进来有配置, 就按传递进来的配置处理, 否则就按默认配置处理

    auto pWndInfo = new LYRIC_DESKTOP_INFO;
    lyric_wnd_set_data(hWnd, pWndInfo);

    pWndInfo->init(hWnd, arg, pfnCommand, lParam);

    // 创建一个提示窗口, 用来显示提示信息
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
    // 这里是创建, 所以需要缩放一下, 保存的配置也是按100%缩放保存的
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


    // 创建高精度定时器, 需要10毫秒以内的精度, 时钟最低只能10ms, 不满足要求
    //TODO 要是在意效率的话应该是有需要重画的时候投递消息然后再处理
    // 不过处理起来会麻烦一点, 干脆就内部一直处理重画吧
    _ld_start_high_precision_timer(pWndInfo);

    MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, pWndInfo->nMinHeight, TRUE);

    return pWndInfo;
}

bool _lyric_desktop_load_lyric(PLYRIC_DESKTOP_INFO pWndInfo, LPCVOID pData, int nSize, LYRIC_PARSE_TYPE nType)
{
    CCriticalSection cs(pWndInfo->pCritSec);    // 上锁, 防止这里不是窗口线程调用, 窗口线程会不停的访问歌词结构

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

    // 必须得用小数记, 不然歌词字多了会相差出很多个像素
    lyric_calc_text(wnd_info.hLyric, [](void* pUserData, LPCWSTR pText, int nTextLen, float* pRetHeight) -> float
                    {
                        auto pWndInfo = (LYRIC_DESKTOP_INFO*)pUserData;
                        D2DRender& pRender = *pWndInfo->dx.pRender;
                        // 没有创建字体就创建一个, 字体是设备无关对象, 不需要从渲染对象里创建
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
    // 上锁, 尝试进入, 能进入就说明没有在修改歌词信息, 往下执行
    CCriticalSection cs(wnd_info.pCritSec, std::adopt_lock_t());
    if (!cs.TryLock())
        return false;   // 进入失败就不继续处理

    // 初始化DX对象, 如果已经初始化会直接返回
    // 因为绘画结束有可能会返回设备失效错误码, 然后会销毁所有DX对象
    // 所以进入绘画前先初始化一下
    wnd_info.dx.init(&wnd_info);

    if (!wnd_info.dx.pRender)
        return false;   // 初始化失败就不能继续处理

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
    // 这个函数百万次调用也就100多毫秒不到200, release在100毫秒以内
    // 完全支撑得起100帧的刷新率

    //wnd_info.nCurrentTimeMS = 38048;
    lyric_calc(wnd_info.hLyric, wnd_info.nCurrentTimeMS, &arg);

    // 是否需要绘画歌词文本, 行数和上次记录的不一样, 或者宽度不一样, 那就为真, 需要重画
    bool bLight = false;
    if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL))
        bLight = arg.nHeightWord != wnd_info.prevHeight;
    else
        bLight = arg.nWidthWord != wnd_info.prevWidth;
    const bool isDrawString = arg.indexLine != wnd_info.prevIndexLine || bLight;
    const bool isDraw = isDrawString || wnd_info.change || wnd_info.config.debug.alwaysCache;

    // 这里需要判断一下是否需要更新, 如果按钮没变化, 不是强制更新, 并且歌词文本也没变, 那就不需要重画
    if (!isDraw)
        return true;

    HRESULT hr = lyric_wnd_OnPaint(wnd_info, isresize, arg);

    wnd_info.change = 0;    // 把所有改变的标志位清零
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
        // 鼠标移动进窗口上, 判断有没有绘画背景, 没有的话就绘画一下
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

        // 直接获取, 省事, 一行代码搞定
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
        // 限制窗口不能超出屏幕范围
        auto wnd_pos = (WINDOWPOS*)lParam;
        if ((wnd_pos->flags & SWP_NOMOVE) == SWP_NOMOVE)
            break;  // 有不移动的标志, 不处理

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
            wnd_pos->x = rcMonitorAll.left;   // 限制窗口左侧不能超出左边界
        else if (wnd_pos->x + width > rcMonitorAll.right)
            wnd_pos->x = rcMonitorAll.right - width;  // 限制窗口右侧不能超出右边界

        if (wnd_pos->y < rcMonitorAll.top)
            wnd_pos->y = rcMonitorAll.top;    // 限制窗口顶部不能超出上边界
        else if (wnd_pos->y + height > rcMonitorAll.bottom)
            wnd_pos->y = rcMonitorAll.bottom - height;   // 限制窗口底部不能超出下边界

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
            pMMI->ptMinTrackSize.x = pWndInfo->nMinWidth;   // 宽度不让改变, 调整字体会自动改变
            pMMI->ptMaxTrackSize.x = pWndInfo->nMinWidth;

        }
        else
        {
            pMMI->ptMinTrackSize.x = pWndInfo->nMinWidth;
            pMMI->ptMinTrackSize.y = pWndInfo->nMinHeight;  // 高度不让改变, 调整字体会自动改变
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
            break;  // 当前是捕获状态, 不处理
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
    // 定义边缘检测的宽度
    const int borderWidth = wnd_info.scale(12) + (int)wnd_info.shadowRadius;

    // 获取鼠标屏幕坐标
    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // 转换为窗口客户区坐标
    RECT rcWindow;
    GetWindowRect(wnd_info.hWnd, &rcWindow);
    
    const bool is_vertical = wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL);
    if (is_vertical)
    {
        // 纵向, 只检测上下两边, 宽度不让调整
        if (pt.y <= rcWindow.top + borderWidth)
            return HTTOP;          // 上边缘
        if (pt.y >= rcWindow.bottom - borderWidth)
            return HTBOTTOM;       // 下边缘
    }
    else
    {
        // 只检测左右两边, 高度不让调整
        if (pt.x <= rcWindow.left + borderWidth)
            return HTLEFT;         // 左边缘
        if (pt.x >= rcWindow.right - borderWidth)
            return HTRIGHT;        // 右边缘
    }

    //if (PtInRect(&wnd_info.button.rc, pt))
    //    return HTCLIENT;    // 鼠标在按钮范围内, 返回客户区坐标

    //// 检测鼠标是否在窗口边缘
    //if (pt.x <= rcWindow.left + borderWidth && pt.y <= rcWindow.top + borderWidth)
    //    return HTTOPLEFT;      // 左上角
    //if (pt.x >= rcWindow.right - borderWidth && pt.y <= rcWindow.top + borderWidth)
    //    return HTTOPRIGHT;     // 右上角
    //if (pt.x <= rcWindow.left + borderWidth && pt.y >= rcWindow.bottom - borderWidth)
    //    return HTBOTTOMLEFT;    // 左下角
    //if (pt.x >= rcWindow.right - borderWidth && pt.y >= rcWindow.bottom - borderWidth)
    //    return HTBOTTOMRIGHT;   // 右下角
    //if (pt.x <= rcWindow.left + borderWidth)
    //    return HTLEFT;         // 左边缘
    //if (pt.x >= rcWindow.right - borderWidth)
    //    return HTRIGHT;        // 右边缘
    //if (pt.y <= rcWindow.top + borderWidth)
    //    return HTTOP;          // 上边缘
    //if (pt.y >= rcWindow.bottom - borderWidth)
    //    return HTBOTTOM;       // 下边缘

    // 默认返回客户区
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
// 鼠标移动, 先判断是不是在按钮上
LRESULT lyric_wnd_OnMouseMove(LYRIC_DESKTOP_INFO& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
    int index = lyric_wnd_pt2index(wnd_info, pt);

    if (index != -1)
    {
        // 鼠标在按钮上, 判断上一次是不是在这个按钮上
        // 或者当前有按下了按钮, 然后移动了鼠标
        if (index == wnd_info.button.index || wnd_info.button.indexDown != -1)
            return 0;   // 在上一次进入的按钮中, 不处理

        // 需要判断按钮是否是禁止状态, 是的话不处理
        if (__query(wnd_info.button.rcBtn[index].state, LYRIC_DESKTOP_BUTTON_STATE_DISABLE))
        {

            return 0;   // 当前按钮是禁止状态, 不处理
        }
        wnd_info.change_btn = true;
        wnd_info.button.index = index;
        SetClassLongPtrW(wnd_info.hWnd, GCLP_HCURSOR, (LONG_PTR)m_hCursorHand);
        lyric_wnd_button_hover(wnd_info);

    }
    else
    {
        // 鼠标不在按钮上, 把状态去除
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
        // 鼠标在按钮中按下了, 这里捕获一下鼠标, 然后等鼠标放开的时候才处理事件
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
        // 之前有按下过, 这里判断放开的位置还是不是在原来的按钮上
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(wnd_info.hWnd, &pt);
        int index = lyric_wnd_pt2index(wnd_info, pt);

        if (index == wnd_info.button.indexDown)
        {
            // 放开的位置在原来的按钮上, 处理事件
            lyric_wnd_button_click(wnd_info);
        }
        else
        {
            // 放开的时候不在原来的按钮上, 取消处理
        }
        wnd_info.button.indexDown = -1;
        // 放开的时候都需要重绘一下, 原来是绘画按下的状态, 现在把按下索引改了之后去重画他
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

    pDWriteTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);           // 不换行
    pDWriteTextLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);         // 左对齐
    pDWriteTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);  // 顶对齐
    //pDWriteTextLayout->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    //// 查询 IDWriteTextLayout1 接口
    //CComPtr<IDWriteTextLayout1> pTextLayout1 = nullptr;
    //hr = pDWriteTextLayout->QueryInterface(IID_PPV_ARGS(&pTextLayout1));
    //// 设置字体间距
    //if (SUCCEEDED(hr) && pTextLayout1)
    //{
    //    DWRITE_TEXT_RANGE range = { 0, (UINT32)len };
    //    hr = pTextLayout1->SetCharacterSpacing(
    //        1.0f,  // 前导间距 (字符前添加的空间)
    //        1.0f,  // 尾随间距 (字符后添加的空间)
    //        1.0f,  // 最小前进宽度 (字符的最小总宽度)
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
        // 不是纵向, 直接计算然后返回
        CCustomTextRenderer renderer;
        pTextLayout->Draw(nullptr, &renderer, 0, 0);
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
                                             height += _width;   // 需要旋转的字符就加上宽度
                                         else
                                             height += _height;  // 非旋转的字符就加上高度
                                     }

                                     return S_OK;
                                 }
    );
    pTextLayout->Draw(nullptr, &renderer, 0, 0);

    *pHeight = height;
    return width;
}


NAMESPACE_LYRIC_DESKTOP_END




