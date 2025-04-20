#include "lyric_wnd_function.h"
#include <control/CControlDraw.h>
#include <windowsx.h>
#include "dwrite_1.h"

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

void lyric_wnd_get_draw_text_info(LYRIC_WND_INFU& wnd_info,
                                  LYRIC_WND_DRAWTEXT_INFO& draw_info,
                                  const RECT& rcWindow, int nLine,
                                  LYRIC_LINE_STRUCT& line, int nLightWidth)
{
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    auto& d2dInfo = d2d_get_info();
    CD2DFont& font = *wnd_info.dx.hFont;

    draw_info.hbrNormal = *wnd_info.dx.hbrNormal;
    draw_info.hbrLight = *wnd_info.dx.hbrLight;
    draw_info.hbrBorder = *wnd_info.dx.hbrBorder;
    draw_info.dxFormat = font;
    draw_info.pRenderTarget = hCanvas;


    draw_info.line = &line;
    draw_info.cache = nLine == 1 ? &wnd_info.cache1 : &wnd_info.cache2;

    draw_info.cxClient = rcWindow.right - rcWindow.left;
    draw_info.cyClient = rcWindow.bottom - rcWindow.top;
    draw_info.rcBack = { 0.F, 0.F, (float)(draw_info.cxClient), (float)(draw_info.cyClient) };

    const int nLeft = wnd_info.scale(16);
    float height = (float)wnd_info.nLineHeight + wnd_info.scale(10);
    float top = (float)(nLine == 1 ? wnd_info.nLineTop1 : wnd_info.nLineTop2);
    float left = (float)nLeft;

    draw_info.nLightWidth = (float)(nLightWidth);
    draw_info.rcText = { left, top, left + (float)line.nWidth, top + height };
    draw_info.layout_text_max_width = (float)(draw_info.cxClient - nLeft * 2);
    draw_info.layout_text_max_height = (float)(draw_info.rcText.bottom - draw_info.rcText.top);
    
}

bool lyric_wnd_invalidate(LYRIC_WND_INFU& wnd_info)
{
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

    LYRIC_LINE_STRUCT line2{L"", L"", L"", 0};  // 下一行的信息, 一行画高亮, 另一行画下一行歌词
    if (arg.indexLine + 1 < arg.nLineCount)
        lyric_get_line(wnd_info.hLyric, arg.indexLine + 1, &line2);

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
        lyric_wnd_draw_button(wnd_info, rcWindow);
    
    // 如果有重画操作, 歌词文本肯定要绘画

    LYRIC_WND_DRAWTEXT_INFO draw_text_info[2] = { 0 };
    lyric_wnd_get_draw_text_info(wnd_info, draw_text_info[0], rcWindow, 1, arg.line, arg.word.nLeft + arg.nWidthWord);
    lyric_wnd_get_draw_text_info(wnd_info, draw_text_info[1], rcWindow, 2, line2, 0);
    decltype(lyric_wnd_draw_text_geometry)* pfn_create_cache_bitmap = nullptr;

    if (!wnd_info.hLyric)
        arg.line.pText = wnd_info.pszDefText, arg.line.nLength = wnd_info.nDefText;
    //TODO 这里可以做个判定选择哪种方式绘画歌词文本
    if (1)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_geometry;
    else if (2)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_glow;

    for (auto& draw_info : draw_text_info)
    {
        if (!draw_info.line || !draw_info.cache)
            throw;

        LYRIC_LINE_STRUCT& _line = *draw_info.line;     // 歌词行信息
        LYRIC_WND_CACHE_OBJ& cache = *draw_info.cache;  // 缓存对象

        if (cache.preIndex != arg.indexLine
            || cache.preText != arg.line.pText
            || cache.preLength != arg.line.nLength
            || wnd_info.change_font || wnd_info.change_hbr  // 字体/画刷改变, 需要重新创建缓存
            )
        {
            // 上次记录的值和这次不一样了, 文本改变了, 重新绘画, 然后记录到位图里

            // 记录绘画的行好和文本
            draw_info.cache->preIndex = arg.indexLine;
            draw_info.cache->preText = arg.line.pText;
            draw_info.cache->preLength = arg.line.nLength;

            // 创建缓存位图
            pfn_create_cache_bitmap(wnd_info, draw_info);

        }

        lyric_wnd_draw_cache_text(wnd_info, draw_info);
        //break;
    }

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

    wnd_info.change = 0;    // 把所有改变的标志位清零
    wnd_info.prevIndexLine = arg.indexLine;
    wnd_info.prevWidth = arg.nWidthWord;
    return SUCCEEDED(hr);
}

void lyric_wnd_draw_cache_text(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info)
{
    LYRIC_LINE_STRUCT& line = *draw_info.line;
    ID2D1DeviceContext* pRenderTarget = draw_info.pRenderTarget;
    const int _10 = wnd_info.scale(10);

    // 从缓存里把数据拿出来画到目标上
    const float _offset = (draw_info.layout_text_max_height - wnd_info.nLineHeight) / 2;
    auto pfn_draw_bitmap = [&](ID2D1Bitmap* pBitmap)
    {
        if (!pBitmap)
            return;
        // 从位图的这个位置拿出来绘画, 就是拿整个位图数据
        D2D1_SIZE_F si = pBitmap->GetSize();
        D2D1_RECT_F rcSrc = { 0.f, 0.f, si.width, si.height };

        const D2D1_RECT_F& rcText = draw_info.rcText;
        D2D1_RECT_F rcDst = { 0 };
        rcDst.left = rcText.left;
        rcDst.top = rcText.top - _10;
        rcDst.right = rcDst.left + si.width;
        rcDst.bottom = rcDst.top + si.height;
        bool isLigth = draw_info.cache->pBitmapLight == pBitmap;

        D2D1_RECT_F rcRgn = rcDst;
        if (line.nWidth > 0)
        {
            float light_offset = 0.f;
            if (draw_info.nLightWidth == line.nWidth)
                light_offset += _offset;
            float light_right = rcRgn.left + draw_info.nLightWidth + _offset + light_offset;
            if (isLigth)
            {
                // 高亮文本, 限制右边不能显示, 右边显示范围就是高亮的宽度
                rcRgn.right = light_right;
            }
            else
            {
                // 普通文本, 限制左边不能显示
                rcRgn.left = light_right;
            }
        }


        //if (!isLigth)
        //{
        //    CD2DBrush hbrBak(MAKEARGB(180, 255, 0, 0));
        //    pRenderTarget->FillRectangle(rcDst, hbrBak);
        //}

        pRenderTarget->PushAxisAlignedClip(rcRgn, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        pRenderTarget->DrawBitmap(pBitmap, rcDst, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rcSrc);
        pRenderTarget->PopAxisAlignedClip();
    };

    pfn_draw_bitmap(draw_info.cache->pBitmapNormal);
    if (line.nWidth > 0)
        pfn_draw_bitmap(draw_info.cache->pBitmapLight);
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
    const int borderWidth = wnd_info.scale(12);

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




