#include "lyric_wnd_header.h"
#include <control/CControlDraw.h>
#include <windowsx.h>

#pragma comment(lib, "dxguid.lib")

using namespace NAMESPACE_D2D;

#include <control/Ctips.h>
NAMESPACE_LYRIC_WND_BEGIN


static HCURSOR m_hCursorArrow;
static HCURSOR m_hCursorHand;

#define TIMER_ID_LEAVE 1000

void lyric_wnd_draw(LYRIC_WND_INFU& wnd_info, const RECT& rcWindow, LYRIC_CALC_STRUCT& arg);
LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnTimer(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnHitTest(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnMouseMove(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnLbuttonDown(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT lyric_wnd_OnCaptureChanged(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam);


template<typename _Ty>
inline void SafeDelete(_Ty*& p)
{
    if (p)
        delete p;
    p = nullptr;
}

bool LYRIC_WND_DX::re_create(LYRIC_WND_INFU* pWndInfo)
{
    LYRIC_WND_INFU& wnd_info = *pWndInfo;
    auto& clrNormal = wnd_info.clrNormal;
    auto& clrLight = wnd_info.clrLight;
    RECT rc;
    GetClientRect(wnd_info.hWnd, &rc);
    const int cxClient = rc.right - rc.left;
    const int cyClient = rc.bottom - rc.top;

    if (!hFont)
        re_create_font(pWndInfo);

    SafeDelete(hCanvas);
    re_create_brush(pWndInfo, true);
    re_create_brush(pWndInfo, false);
    re_create_border(pWndInfo);
    re_create_image(pWndInfo);

    SafeDelete(hbrLine);
    hbrLine = new CD2DBrush(MAKEARGB(100, 255, 255, 255));

    hCanvas = new CD2DRender(cxClient, cyClient);


    return true;
}

bool LYRIC_WND_DX::re_create_brush(LYRIC_WND_INFU* pWndInfo, bool isLight)
{
    DWORD* pClr;
    int size;
    LYRIC_WND_INFU& wnd_info = *pWndInfo;
    NAMESPACE_D2D::CD2DBrush_LinearGradient** ppBrush;
    if (isLight)
    {
        SafeDelete(hbrLight);
        pClr = &wnd_info.clrLight[0];
        size = (int)wnd_info.clrLight.size();
        ppBrush = &hbrLight;
    }
    else
    {
        SafeDelete(hbrNormal);
        pClr = &wnd_info.clrNormal[0];
        size = (int)wnd_info.clrNormal.size();
        ppBrush = &hbrNormal;
    }
    
    POINT_F pt1 = { 0.f, 0.f };
    POINT_F pt2 = { 1.f, .1f };
    *ppBrush = new CD2DBrush_LinearGradient(pt1, pt2, pClr, size);
    return *ppBrush != nullptr;
}

bool LYRIC_WND_DX::re_create_border(LYRIC_WND_INFU* pWndInfo)
{
    SafeDelete(hbrBorder);
    hbrBorder = new CD2DBrush(pWndInfo->clrBorder);
    return hbrBorder != nullptr;
}

bool LYRIC_WND_DX::re_create_font(LYRIC_WND_INFU* pWndInfo)
{
    SafeDelete(hFont);

    LOGFONT lf = pWndInfo->lf;
    lf.lfHeight = -MulDiv(lf.lfHeight, pWndInfo->scale, 72);
    hFont = new CD2DFont(&lf);

    float width = 0;
    hCanvas->calc_text(hFont, L"福仔", 2, DT_SINGLELINE, 0, 0, 0, &width, &pWndInfo->nLineHeight, 0);
    return hFont != nullptr;
}

bool LYRIC_WND_DX::re_create_image(LYRIC_WND_INFU* pWndInfo)
{
    SafeDelete(image);
    lyric_wnd_load_image(*pWndInfo);
    return false;
}

bool LYRIC_WND_DX::destroy(bool isDestroyFont)
{
    // 除了字体, 剩下的都是设备相关的
    if (isDestroyFont)
        SafeDelete(hFont);

    SafeDelete(hCanvas);
    SafeDelete(hbrBorder);
    SafeDelete(hbrLine);
    SafeDelete(hbrNormal);
    SafeDelete(hbrLight);
    SafeDelete(image);
    return true;
}


void LYRIC_WND_INFU::set_def_arg(const LYRIC_WND_ARG* arg)
{
    // 有值就根据传递进来的值设置, 没有值就设置默认值
    LYRIC_WND_ARG def = { 0 };
    lyric_wnd_get_default_arg(&def);
    if (!arg)
        arg = &def;
    
    if (arg->pClrNormal)
        clrNormal.assign(&arg->pClrNormal[0], &arg->pClrNormal[arg->nClrNormal]);
    if (arg->pClrLight)
        clrLight.assign(&arg->pClrLight[0], &arg->pClrLight[arg->nClrLight]);

    clrBorder = arg->clrBorder;
    dx.clrBack = arg->clrBackground;

    LPCWSTR pszFontName = arg->pszFontName;
    if (!pszFontName || !*pszFontName)
        pszFontName = L"微软雅黑";

    wcscpy_s(lf.lfFaceName, pszFontName);
    lf.lfWeight = FW_BOLD;
    lf.lfHeight = arg->nFontSize;


}


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
    wc.cbWndExtra       = 0;
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

    HWND hWnd = CreateWindowExW(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
                                wc.lpszClassName, L"www.kuodafu.com",
                                WS_POPUP | WS_VISIBLE,
                                0, 0, 1, 1,
                                NULL, NULL, wc.hInstance, NULL);


    if (!hWnd)
        return nullptr;

    return hWnd;
}

// 绘画的时候没有创建对象, 那就需要创建默认对象
void lyric_wnd_default_object(LYRIC_WND_INFU& wnd_info)
{
    wnd_info.dx.re_create(&wnd_info);
}

bool lyric_wnd_invalidate(LYRIC_WND_INFU& wnd_info)
{
    if (!wnd_info.hLyric)
        return false;
    if (!wnd_info.dx.hCanvas)
        lyric_wnd_default_object(wnd_info);
    if (!wnd_info.dx.hCanvas)
        return false;

    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    RECT rcWindow;
    GetWindowRect(wnd_info.hWnd, &rcWindow);
    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;

    int bmpWidth = 0, bmpHeight = 0;
    hCanvas.getsize(&bmpWidth, &bmpHeight);

    bool isresize = false;
    if (cxClient != bmpWidth || cyClient != bmpHeight)
        wnd_info.change_wnd = true, isresize = true;


    LYRIC_CALC_STRUCT arg{};
    // 这个函数百万次调用也就100多毫秒不到200, release在100毫秒以内
    // 完全支撑得起100帧的刷新率
    lyric_calc(wnd_info.hLyric, wnd_info.nCurrentTimeMS + wnd_info.nTimeOffset, &arg);

    // 是否需要绘画歌词文本, 行数和上次记录的不一样, 或者宽度不一样, 那就为真, 需要重画
    const bool isDrawString = arg.indexLine != wnd_info.prevIndexLine || arg.nWidthWord != wnd_info.prevWidth;
    const bool isDraw = isDrawString || wnd_info.change;

    // 这里需要判断一下是否需要更新, 如果按钮没变化, 不是强制更新, 并且歌词文本也没变, 那就不需要重画
    if (!isDraw)
        return true;

    wnd_info.change = 0;    // 把所有改变的标志位清零
    wnd_info.prevIndexLine = arg.indexLine;
    wnd_info.prevWidth = arg.nWidthWord;

#ifdef _DEBUG
    if (!arg.word.pText || !*arg.word.pText)
        __debugbreak();
#endif

    if (isresize)
        hCanvas.resize(cxClient, cyClient);

    hCanvas->BeginDraw();
    if (wnd_info.isFillBack && !wnd_info.isLock)
        hCanvas->Clear(ARGB_D2D(wnd_info.dx.clrBack));
    else
        hCanvas->Clear();
    

    auto oldAntialiasMode = hCanvas->GetAntialiasMode();
    auto oldTextAntialiasMode = hCanvas->GetTextAntialiasMode();
    hCanvas->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    hCanvas->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    if (!wnd_info.isLock)   // 没锁定的时候才绘画按钮
        lyric_wnd_draw_button(wnd_info, rcWindow, arg);
    
    // 如果有重画操作, 歌词文本肯定要绘画
    lyric_wnd_draw(wnd_info, rcWindow, arg);

    ID2D1GdiInteropRenderTarget* pGdiInterop = hCanvas;
    HDC hdcD2D = 0;
    pGdiInterop->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hdcD2D);
    UpdateLayered(wnd_info.hWnd, cxClient, cyClient, hdcD2D);
    pGdiInterop->ReleaseDC(0);

    hCanvas->SetAntialiasMode(oldAntialiasMode);
    hCanvas->SetTextAntialiasMode(oldTextAntialiasMode);

    HRESULT hr = hCanvas->EndDraw();
    if (FAILED(hr))
    {
        // 这里需要清除对象, 然后在绘画前重新创建, 设备无效了
        wnd_info.dx.destroy(false);  // 销毁, 然后绘画前会判断有没有创建
    }
    return SUCCEEDED(hr);
}

// 绘画相关的内容
void lyric_wnd_draw(LYRIC_WND_INFU& wnd_info, const RECT& rcWindow, LYRIC_CALC_STRUCT& arg)
{
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    LYRIC_LINE_STRUCT& line = arg.line;
    LYRIC_WORD_STRUCT& word = arg.word;
    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;
    HRESULT hr = 0;

    D2D1_RECT_F rcText;
    D2D1_RECT_F rcText2;
    float height = (float)wnd_info.nLineHeight + wnd_info.scale(10);
    float top = (float)(wnd_info.nLineTop1);
    float left = (float)wnd_info.scale(16);
    auto& d2dInfo = d2d_get_info();
    CD2DFont& font = *wnd_info.dx.hFont;
    IDWriteTextFormat* dxFormat = font;


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
    D2D1_RECT_F rcBack(0.F, 0.F, (float)(cxClient), (float)(cyClient));
    ID2D1LinearGradientBrush* hbrNormal = *wnd_info.dx.hbrNormal;
    ID2D1LinearGradientBrush* hbrLight = *wnd_info.dx.hbrLight;
    ID2D1SolidColorBrush* hbrBorder = *wnd_info.dx.hbrBorder;

    // TODO 这里得把文本缓存起来, 等行改变的时候再重画缓存

    CComPtr<ID2D1PathGeometry> textGeometry = nullptr;
    hr = d2dInfo.pFactory->CreatePathGeometry(&textGeometry);
    if (FAILED(hr))
        return;

    float calcWidth = 0.f, calcHeight = 0.f;
    CComPtr<IDWriteTextLayout> pTextLayout = 0;
    hr = d2dInfo.pDWriteFactory->CreateTextLayout(line.pText,
                                                  (UINT32)line.nLength, dxFormat,
                                                  //rcText.right - rcText.left, rcText.bottom - rcText.top,
                                                  rcBack.right - rcBack.left, rcText.bottom - rcText.top,
                                                  &pTextLayout);
    if (FAILED(hr))
        return;

    lyric_wnd_geometry_add_string(hCanvas, textGeometry, pTextLayout);
    lyric_wnd_draw_geometry(hCanvas, textGeometry, hbrNormal, hbrLight, hbrBorder, rcText, rcText2, arg, dxFormat);

    rcText.top = wnd_info.nLineTop2;
    rcText.bottom = rcText2.bottom - rcText2.top;
    rcText2 = rcText;
    rcText2.right = rcText2.left + (float)word.nLeft + arg.nWidthWord;
    lyric_wnd_draw_geometry(hCanvas, textGeometry, hbrNormal, hbrLight, hbrBorder, rcText, rcText2, arg, dxFormat);

}


LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto pWndInfo = (LYRIC_WND_INFU*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    if (!pWndInfo)
        return DefWindowProcW(hWnd, message, wParam, lParam);

    switch (message)
    {
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
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
        delete pWndInfo;
        return 0;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* pMMI = (MINMAXINFO*)lParam;
        pMMI->ptMinTrackSize.x = pWndInfo->nMinWidth;
        pMMI->ptMinTrackSize.y = pWndInfo->nMinHeight;
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
    const int borderWidth = wnd_info.scale(6);

    // 获取鼠标屏幕坐标
    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // 转换为窗口客户区坐标
    RECT rcWindow;
    GetWindowRect(wnd_info.hWnd, &rcWindow);


    // 检测鼠标是否在窗口边缘
    if (pt.x <= rcWindow.left + borderWidth && pt.y <= rcWindow.top + borderWidth)
        return HTTOPLEFT;      // 左上角
    if (pt.x >= rcWindow.right - borderWidth && pt.y <= rcWindow.top + borderWidth)
        return HTTOPRIGHT;     // 右上角
    if (pt.x <= rcWindow.left + borderWidth && pt.y >= rcWindow.bottom - borderWidth)
        return HTBOTTOMLEFT;    // 左下角
    if (pt.x >= rcWindow.right - borderWidth && pt.y >= rcWindow.bottom - borderWidth)
        return HTBOTTOMRIGHT;   // 右下角
    if (pt.x <= rcWindow.left + borderWidth)
        return HTLEFT;         // 左边缘
    if (pt.x >= rcWindow.right - borderWidth)
        return HTRIGHT;        // 右边缘
    if (pt.y <= rcWindow.top + borderWidth)
        return HTTOP;          // 上边缘
    if (pt.y >= rcWindow.bottom - borderWidth)
        return HTBOTTOM;       // 下边缘

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


NAMESPACE_LYRIC_WND_END


