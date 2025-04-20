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

// �滭��ʱ��û�д�������, �Ǿ���Ҫ����Ĭ�϶���
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
    // �����������ε���Ҳ��100����벻��200, release��100��������
    // ��ȫ֧�ŵ���100֡��ˢ����
    lyric_calc(wnd_info.hLyric, wnd_info.nCurrentTimeMS, &arg);

    // �Ƿ���Ҫ�滭����ı�, �������ϴμ�¼�Ĳ�һ��, ���߿�Ȳ�һ��, �Ǿ�Ϊ��, ��Ҫ�ػ�
    const bool isDrawString = arg.indexLine != wnd_info.prevIndexLine || arg.nWidthWord != wnd_info.prevWidth;
    const bool isDraw = isDrawString || wnd_info.change;

    // ������Ҫ�ж�һ���Ƿ���Ҫ����, �����ťû�仯, ����ǿ�Ƹ���, ���Ҹ���ı�Ҳû��, �ǾͲ���Ҫ�ػ�
    if (!isDraw)
        return true;

    LYRIC_LINE_STRUCT line2{L"", L"", L"", 0};  // ��һ�е���Ϣ, һ�л�����, ��һ�л���һ�и��
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

    if (!wnd_info.isLock)   // û������ʱ��Ż滭��ť
        lyric_wnd_draw_button(wnd_info, rcWindow);
    
    // ������ػ�����, ����ı��϶�Ҫ�滭

    LYRIC_WND_DRAWTEXT_INFO draw_text_info[2] = { 0 };
    lyric_wnd_get_draw_text_info(wnd_info, draw_text_info[0], rcWindow, 1, arg.line, arg.word.nLeft + arg.nWidthWord);
    lyric_wnd_get_draw_text_info(wnd_info, draw_text_info[1], rcWindow, 2, line2, 0);
    decltype(lyric_wnd_draw_text_geometry)* pfn_create_cache_bitmap = nullptr;

    if (!wnd_info.hLyric)
        arg.line.pText = wnd_info.pszDefText, arg.line.nLength = wnd_info.nDefText;
    //TODO ������������ж�ѡ�����ַ�ʽ�滭����ı�
    if (1)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_geometry;
    else if (2)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_glow;

    for (auto& draw_info : draw_text_info)
    {
        if (!draw_info.line || !draw_info.cache)
            throw;

        LYRIC_LINE_STRUCT& _line = *draw_info.line;     // �������Ϣ
        LYRIC_WND_CACHE_OBJ& cache = *draw_info.cache;  // �������

        if (cache.preIndex != arg.indexLine
            || cache.preText != arg.line.pText
            || cache.preLength != arg.line.nLength
            || wnd_info.change_font || wnd_info.change_hbr  // ����/��ˢ�ı�, ��Ҫ���´�������
            )
        {
            // �ϴμ�¼��ֵ����β�һ����, �ı��ı���, ���»滭, Ȼ���¼��λͼ��

            // ��¼�滭���кú��ı�
            draw_info.cache->preIndex = arg.indexLine;
            draw_info.cache->preText = arg.line.pText;
            draw_info.cache->preLength = arg.line.nLength;

            // ��������λͼ
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
        // ������Ҫ�������, Ȼ���ڻ滭ǰ���´���, �豸��Ч��
        wnd_info.dx.destroy(false);  // ����, Ȼ��滭ǰ���ж���û�д���
    }

    wnd_info.change = 0;    // �����иı�ı�־λ����
    wnd_info.prevIndexLine = arg.indexLine;
    wnd_info.prevWidth = arg.nWidthWord;
    return SUCCEEDED(hr);
}

void lyric_wnd_draw_cache_text(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info)
{
    LYRIC_LINE_STRUCT& line = *draw_info.line;
    ID2D1DeviceContext* pRenderTarget = draw_info.pRenderTarget;
    const int _10 = wnd_info.scale(10);

    // �ӻ�����������ó�������Ŀ����
    const float _offset = (draw_info.layout_text_max_height - wnd_info.nLineHeight) / 2;
    auto pfn_draw_bitmap = [&](ID2D1Bitmap* pBitmap)
    {
        if (!pBitmap)
            return;
        // ��λͼ�����λ���ó����滭, ����������λͼ����
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
                // �����ı�, �����ұ߲�����ʾ, �ұ���ʾ��Χ���Ǹ����Ŀ��
                rcRgn.right = light_right;
            }
            else
            {
                // ��ͨ�ı�, ������߲�����ʾ
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
        pMMI->ptMinTrackSize.y = pWndInfo->nMinHeight;  // �߶Ȳ��øı�, ����������Զ��ı�
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

LRESULT lyric_wnd_OnHitTest(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
{
    // �����Ե���Ŀ��
    const int borderWidth = wnd_info.scale(12);

    // ��ȡ�����Ļ����
    POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

    // ת��Ϊ���ڿͻ�������
    RECT rcWindow;
    GetWindowRect(wnd_info.hWnd, &rcWindow);

    // ֻ�����������, �߶Ȳ��õ���
    if (pt.x <= rcWindow.left + borderWidth)
        return HTLEFT;         // ���Ե
    if (pt.x >= rcWindow.right - borderWidth)
        return HTRIGHT;        // �ұ�Ե

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
// ����ƶ�, ���ж��ǲ����ڰ�ť��
LRESULT lyric_wnd_OnMouseMove(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
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
        if (__query(wnd_info.button.rcBtn[index].state, LYRIC_WND_BUTTON_STATE_DISABLE))
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

LRESULT lyric_wnd_OnLbuttonDown(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
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
LRESULT lyric_wnd_OnCaptureChanged(LYRIC_WND_INFU& wnd_info, UINT message, WPARAM wParam, LPARAM lParam)
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

    return pDWriteTextLayout;
}

NAMESPACE_LYRIC_WND_END




