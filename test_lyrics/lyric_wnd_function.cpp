#include "lyric_wnd_function.h"
#include <windowsx.h>
#include "dwrite_1.h"
#include <dwmapi.h>

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace NAMESPACE_D2D;

NAMESPACE_LYRIC_WND_BEGIN


static HCURSOR m_hCursorArrow;
static HCURSOR m_hCursorHand;

#define TIMER_ID_LEAVE 1000



LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnTimer(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnHitTest(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnMouseMove(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnLbuttonDown(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnCaptureChanged(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);



HWND lyric_create_layered_window(const LYRIC_WND_ARG* arg)
{
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
    wc.lpszClassName    = L"www.kuodafu.com lyric_window";
    wc.hIconSm          = wc.hIcon;



#ifdef _DEBUG
    const bool isDebug = true;
#else
    const bool isDebug = false;
#endif
    static ATOM atom = RegisterClassExW(&wc);
    static bool init_d2d = d2d::d2d_init(isDebug);

    HWND hWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
                                wc.lpszClassName, L"",
                                WS_POPUP | WS_VISIBLE,// | WS_BORDER,
                                0, 0, 1, 1,
                                NULL, NULL, wc.hInstance, NULL);


    if (!hWnd)
        return nullptr;

    //DWMNCRENDERINGPOLICY pv = DWMNCRP_ENABLED;
    //HRESULT hr = DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &pv, sizeof(pv));

    return hWnd;
}

// 绘画的时候没有创建对象, 那就需要创建默认对象
void lyric_wnd_default_object(LYRIC_WND_INFU& wnd_info)
{
    wnd_info.dx.re_create(&wnd_info);
}


bool lyric_wnd_invalidate(LYRIC_WND_INFU& wnd_info)
{
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
    // 这个函数百万次调用也就100多毫秒不到200, release在100毫秒以内
    // 完全支撑得起100帧的刷新率
    lyric_calc(wnd_info.hLyric, wnd_info.nCurrentTimeMS, &arg);

    // 是否需要绘画歌词文本, 行数和上次记录的不一样, 或者宽度不一样, 那就为真, 需要重画
    const bool isDrawString = arg.indexLine != wnd_info.prevIndexLine || arg.nWidthWord != wnd_info.prevWidth;
    const bool isDraw = isDrawString || wnd_info.change;

    // 这里需要判断一下是否需要更新, 如果按钮没变化, 不是强制更新, 并且歌词文本也没变, 那就不需要重画
    if (!isDraw)
        return true;

    HRESULT hr = lyric_wnd_OnPaint(wnd_info, isresize, arg);

    wnd_info.change = 0;    // 把所有改变的标志位清零
    wnd_info.prevIndexLine = arg.indexLine;
    wnd_info.prevWidth = arg.nWidthWord;
    return SUCCEEDED(hr);
}


LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PLYRIC_WND_INFU pWndInfo = lyric_wnd_get_data(hWnd);
    if (!pWndInfo)
        return DefWindowProcW(hWnd, message, wParam, lParam);

    //return DefWindowProcW(hWnd, message, wParam, lParam);

    switch (message)
    {
    //case WM_CREATE:
    //{
    //    DWMNCRENDERINGPOLICY pv = DWMNCRP_ENABLED;
    //    HRESULT hr = DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &pv, sizeof(pv));
    //    return 0;
    //}
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
    case WM_SIZE:
        //InvalidateRect(hWnd, 0, 0);
        return 0;
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
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMMI = (MINMAXINFO*)lParam;
        pMMI->ptMinTrackSize.x = pWndInfo->nMinWidth;
        pMMI->ptMinTrackSize.y = pWndInfo->nMinHeight;  // 高度不让改变, 调整字体会自动改变
        pMMI->ptMaxTrackSize.y = pWndInfo->nMinHeight;
        return 0;
    }
    case WM_NCHITTEST:
        return lyric_wnd_OnHitTest(*pWndInfo, message, wParam, lParam);
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT lyric_wnd_OnTimer(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
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

LRESULT lyric_wnd_OnHitTest(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
{
    // 定义边缘检测的宽度
    const int borderWidth = wnd_info.scale(12) + (int)wnd_info.shadowRadius;

    // 获取鼠标屏幕坐标
    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // 转换为窗口客户区坐标
    RECT rcWindow;
    GetWindowRect(wnd_info.hWnd, &rcWindow);

    // 只检测左右两边, 高度不让调整
    if (pt.x <= rcWindow.left + borderWidth)
        return HTLEFT;         // 左边缘
    if (pt.x >= rcWindow.right - borderWidth)
        return HTRIGHT;        // 右边缘

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

static int lyric_wnd_pt2index(LYRIC_WND_INFU& wnd_info, const POINT& pt)
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
LRESULT lyric_wnd_OnMouseMove(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
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
        if (__query(wnd_info.button.rcBtn[index].state, LYRIC_WND_BUTTON_STATE_DISABLE))
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

LRESULT lyric_wnd_OnLbuttonDown(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
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
LRESULT lyric_wnd_OnCaptureChanged(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
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

IDWriteTextLayout* lyric_wnd_create_text_layout(LPCWSTR str, int len, IDWriteTextFormat* dxFormat, float layoutWidth, float layoutHeight)
{
    if (!str)
        return nullptr;
    IDWriteTextLayout* pDWriteTextLayout = 0;
    auto& d2dInfo = d2d_get_info();
    HRESULT hr = d2dInfo.pDWriteFactory->CreateTextLayout(str, (UINT32)len, dxFormat,
                                                          layoutWidth, layoutHeight, &pDWriteTextLayout);
    if (FAILED(hr))
        return nullptr;

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

    return pDWriteTextLayout;
}

NAMESPACE_LYRIC_WND_END




