/*
    这个文件主要显示歌词的窗口
    一个透明窗口, 显示一行/两行歌词, 歌词顶部有一些按钮操作
*/

#include "lyric_wnd.h"
#include "lyric_exe_header.h"
#include <string>
#include <unordered_map>
#include <control/CControlDraw.h>
#include <CScale.h>
#include "../kuodafu_lyric.h"
#include "CD2DRender.h"

#define NAMESPACE_LYRIC_WND_BEGIN namespace lyric_wnd{
#define NAMESPACE_LYRIC_WND_END }

NAMESPACE_LYRIC_WND_BEGIN

#define TIMERID_UPDATE_LYRIC 1000   // 时钟ID, 更新歌词
using namespace d2d;

typedef struct LYRIC_WND_INFU
{
    HWND        hWnd;           // 歌词窗口句柄
    CD2DRender*    hCanvas;        // D2D绘画句柄

    CD2DFont*   hFont;          // 绘画歌词的字体
    CD2DBrush*  hbrBack;        // 鼠标移动上来之后显示的歌词背景画刷
    CD2DBrush_LinearGradient*  hbrNormal;      // 普通歌词画刷
    CD2DBrush_LinearGradient*  hbrLight;       // 高亮歌词画刷
    HLYRIC      hLyric;         // 歌词句柄
    int         prevIndexLine;  // 上一次绘画的歌词行号
    int         prevWidth;      // 上一次绘画的歌词宽度
    int         nLineHeight;    // 一行歌词的高度

    PFN_LYRIC_WND_COMMAND   pfnCommand; // 歌词窗口上的按钮被点击回调函数
    LPARAM                  lParam;     // 歌词窗口上的按钮被点击回调函数的参数
    CScale      scale;          // 缩放比例
    LYRIC_WND_INFU()
    {
        hWnd = nullptr;
        hFont = nullptr;
        hCanvas = nullptr;
        hbrBack = nullptr;
        hbrNormal = nullptr;
        hbrLight = nullptr;
        hLyric = nullptr;
        prevIndexLine = -1;
        prevWidth = 0;
        pfnCommand = nullptr;
        lParam = 0;
        nLineHeight = 0;
    }
    ~LYRIC_WND_INFU()
    {
        delete hCanvas;
        delete hFont;
        delete hbrBack;
        delete hbrNormal;
        delete hbrLight;
        lyric_destroy(hLyric);

    }
}*PLYRIC_WND_INFU;


HWND lyric_create_layered_window(const LYRIC_WND_ARG* arg);
void lyric_wnd_default_object(LYRIC_WND_INFU& wnd_info);
void lyric_wnd_draw(LYRIC_WND_INFU& wnd_info, int pos, const RECT& rcWindow, LYRIC_CALC_STRUCT& arg);
LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


NAMESPACE_LYRIC_WND_END



HWND lyric_wnd_create(const LYRIC_WND_ARG* arg, PFN_LYRIC_WND_COMMAND pfnCommand, LPARAM lParam)
{
    using namespace lyric_wnd;

    HWND hWnd = lyric_create_layered_window(arg);
    if (!hWnd)
        return nullptr;

    LYRIC_WND_INFU* pWndInfo = new LYRIC_WND_INFU;
    LYRIC_WND_INFU& wnd_info = *pWndInfo;
    SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)pWndInfo);

    RECT rc;
    GetClientRect(hWnd, &rc);
    const int cxClient = rc.right - rc.left;
    const int cyClient = rc.bottom - rc.top;

    wnd_info.hWnd = hWnd;
    wnd_info.scale.SetDpi(wnd_info.hWnd); // 记录当前窗口的dpi
    wnd_info.hCanvas = new CD2DRender(cxClient, cyClient);

    return hWnd;
}

bool lyric_wnd_load_krc(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen)
{
    using namespace lyric_wnd;
    auto pWndInfo = (LYRIC_WND_INFU*)GetWindowLongPtrW(hWindowLyric, GWLP_USERDATA);
    if (!pWndInfo)
        return false;
    LYRIC_WND_INFU& wnd_info = *pWndInfo;
    HLYRIC hLyric = lyric_parse(pKrcData, nKrcDataLen);
    if (!hLyric)
        return false;
    wnd_info.hLyric = hLyric;
    lyric_calc_text_width(hLyric, [](void* pUserData, LPCWSTR pText, int nTextLen, int* pRetHeight) -> int
    {
        LYRIC_WND_INFU* pWndInfo = (LYRIC_WND_INFU*)pUserData;
        CD2DRender& hCanvas = *pWndInfo->hCanvas;
        if (!pWndInfo->hFont)
            lyric_wnd_default_object(*pWndInfo);
        float width = 0, height = 0;
        hCanvas.calc_text(pWndInfo->hFont, pText, nTextLen, DT_SINGLELINE, 0, 0, 0, &width, &height, 0);
        //_canvas_calctextsize(hCanvas, pWndInfo->hFont, pText, nTextLen, DT_SINGLELINE, 0, 0, &width, &height);
        *pRetHeight = (int)height;
        return (int)width;
    }, pWndInfo);
    return true;
}

bool lyric_wnd_update(HWND hWindowLyric, int nCurrentTimeMS)
{
    using namespace lyric_wnd;
    auto pWndInfo = (LYRIC_WND_INFU*)GetWindowLongPtrW(hWindowLyric, GWLP_USERDATA);
    if (!pWndInfo)
        return false;

    LYRIC_WND_INFU& wnd_info = *pWndInfo;
    if (!wnd_info.hLyric)
        return false;

    LYRIC_CALC_STRUCT arg{};
    // 这个函数百万次调用也就100多毫秒不到200, release在100毫秒以内
    // 完全支撑得起100帧的刷新率
    lyric_calc(wnd_info.hLyric, nCurrentTimeMS, &arg);
    if (arg.indexLine == wnd_info.prevIndexLine && arg.nWidthWord == wnd_info.prevWidth)
        return true;    // 上次绘画的是这一行的这个位置, 这一次还是这个, 那就不需要绘画了, 直接返回

    wnd_info.prevIndexLine = arg.indexLine;
    wnd_info.prevWidth = arg.nWidthWord;

#ifdef _DEBUG
    if (!arg.word.pText || !*arg.word.pText)
        __debugbreak();
#endif

    if (!wnd_info.hFont)
        lyric_wnd_default_object(wnd_info);

    CD2DRender& hCanvas = *pWndInfo->hCanvas;
    RECT rc;
    GetWindowRect(wnd_info.hWnd, &rc);
    const int cxClient = rc.right - rc.left;
    const int cyClient = rc.bottom - rc.top;

    //if ((int)hCanvas->GetWidth() != cxClient || (int)hCanvas->GetHeight() != cyClient)
    //    hCanvas->Resize(cxClient, cyClient);


    hCanvas->BeginDraw();
    hCanvas->Clear();

    lyric_wnd_draw(wnd_info, nCurrentTimeMS, rc, arg);
    ID2D1GdiInteropRenderTarget* pGdiInterop = hCanvas;
    HDC hdcD2D = 0;
    pGdiInterop->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hdcD2D);
    UpdateLayered(wnd_info.hWnd, cxClient, cyClient, hdcD2D);
    pGdiInterop->ReleaseDC(0);
    hCanvas->EndDraw();

    return true;
}

bool lyric_wnd_set_color(HWND hWindowLyric, bool isLight, DWORD* pClr, int nCount)
{




    return false;
}


NAMESPACE_LYRIC_WND_BEGIN

HWND lyric_create_layered_window(const LYRIC_WND_ARG* arg)
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

    const RECT& rc = arg->rcWindow;
    MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);

    return hWnd;
}

// 绘画的时候没有创建对象, 那就需要创建默认对象
void lyric_wnd_default_object(LYRIC_WND_INFU& wnd_info)
{
    RECT rc;
    GetClientRect(wnd_info.hWnd, &rc);
    wnd_info.hFont = new CD2DFont(L"微软雅黑", wnd_info.scale(-30), FONTSTYLE::FontStyleBold);
    float width = 0, height = 0;
    wnd_info.hCanvas->calc_text(wnd_info.hFont, L"福仔", 2, DT_SINGLELINE, 0, 0, 0, &width, &height, 0);
    //_canvas_calctextsize(wnd_info.hCanvas, wnd_info.hFont, L"福仔", 2, DT_SINGLELINE, 0, 0, &width, &height);
    wnd_info.nLineHeight = (int)height;

    DWORD clrNormal[] =
    {
        MAKEARGB(255, 0,52,138),
        MAKEARGB(255, 0,128,192),
        MAKEARGB(255, 3,202,252),
    };

    DWORD clrLight[] =
    {
        MAKEARGB(255, 130,247,253),
        MAKEARGB(255, 255,255,255),
        MAKEARGB(255, 3, 233, 252),
    };


    POINT_F pt1 = { 0.f, 0.f };
    POINT_F pt2 = { 1.f, .1f };

    float r[] =
    {
        0.8f,0.9f,1.0f
    };

    wnd_info.hbrNormal  = new CD2DBrush_LinearGradient(pt1, pt2, clrNormal, ARRAYSIZE(clrNormal), 0, r, 0);
    wnd_info.hbrLight   = new CD2DBrush_LinearGradient(pt1, pt2, clrLight, ARRAYSIZE(clrLight), 0, r, 0);
    wnd_info.hbrBack    = new CD2DBrush(MAKEARGB(100, 0, 0, 0));


}

class CustomTextRenderer : public IDWriteTextRenderer
{
public:
    CustomTextRenderer(
        ID2D1Factory* pD2DFactory,
        ID2D1DeviceContext* pRenderTarget,
        ID2D1Brush* pFillBrush,
        ID2D1Brush* pDrawBrush,
        ID2D1GeometrySink* pGeometrySink,
        ID2D1PathGeometry* pPathGeometry,
        float strokeStyle
    ) : m_refCount(1),
        m_pD2DFactory(pD2DFactory),
        m_pRenderTarget(pRenderTarget),
        m_pFillBrush(pFillBrush),
        m_pDrawBrush(pDrawBrush),
        m_pGeometrySink(pGeometrySink),
        m_pPathGeometry(pPathGeometry),
        m_strokeStyle(strokeStyle)
    {
        SafeAddref(m_pD2DFactory);
        SafeAddref(m_pRenderTarget);
        SafeAddref(m_pFillBrush);
        SafeAddref(m_pDrawBrush);
    }

    ~CustomTextRenderer()
    {
        SafeRelease(m_pD2DFactory);
        SafeRelease(m_pRenderTarget);
        SafeRelease(m_pFillBrush);
        SafeRelease(m_pDrawBrush);
    }

    // IUnknown methods
    IFACEMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override
    {
        if (__uuidof(IDWriteTextRenderer) == riid)
        {
            *ppvObject = static_cast<IDWriteTextRenderer*>(this);
        }
        else if (__uuidof(IDWritePixelSnapping) == riid)
        {
            *ppvObject = static_cast<IDWritePixelSnapping*>(this);
        }
        else if (__uuidof(IUnknown) == riid)
        {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else
        {
            *ppvObject = nullptr;
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    IFACEMETHOD_(ULONG, AddRef)() override
    {
        return InterlockedIncrement(&m_refCount);
    }

    IFACEMETHOD_(ULONG, Release)() override
    {
        ULONG newCount = InterlockedDecrement(&m_refCount);
        SafeRelease(m_pD2DFactory);
        SafeRelease(m_pRenderTarget);
        SafeRelease(m_pFillBrush);
        SafeRelease(m_pDrawBrush);
        return newCount;
    }

    // IDWritePixelSnapping methods
    IFACEMETHOD(IsPixelSnappingDisabled)(
        void* clientDrawingContext,
        BOOL* isDisabled
        ) override
    {
        *isDisabled = FALSE;
        return S_OK;
    }

    IFACEMETHOD(GetCurrentTransform)(
        void* clientDrawingContext,
        DWRITE_MATRIX* transform
        ) override
    {
        m_pRenderTarget->GetTransform((D2D1_MATRIX_3X2_F*)transform);
        //transform->m11 = 1.0f; transform->m12 = 0.0f;
        //transform->m21 = 0.0f; transform->m22 = 1.0f;
        //transform->dx = 0.0f;  transform->dy = 0.0f;
        return S_OK;
    }

    IFACEMETHOD(GetPixelsPerDip)(
        void* clientDrawingContext,
        FLOAT* pixelsPerDip
        ) override
    {
        float dpix, dpiy;
        m_pRenderTarget->GetDpi(&dpix, &dpiy);
        *pixelsPerDip = dpix / 96.f;
        return S_OK;
    }

    // IDWriteTextRenderer methods
    IFACEMETHOD(DrawGlyphRun)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect
        ) override
    {
        HRESULT hr = S_OK;

        //// 将字形转换为几何图形
        //CComPtr<ID2D1PathGeometry> pPathGeometry = nullptr;
        //hr = m_pD2DFactory->CreatePathGeometry(&pPathGeometry);
        //if (FAILED(hr)) return hr;

        //CComPtr<ID2D1GeometrySink> pSink = nullptr;
        //hr = pPathGeometry->Open(&pSink);
        //if (FAILED(hr)) return hr;
        

        hr = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            glyphRun->bidiLevel != 0,
            m_pGeometrySink
        );
        //pSink->Close();
        if (FAILED(hr))
            return hr;

        D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
            1.0f, 0.0f,
            0.0f, 1.0f,
            baselineOriginX, baselineOriginY
        );


        // 将几何图形添加到主几何图形中
        //pPathGeometry->Stream(m_pGeometrySink);

        //CComPtr<ID2D1TransformedGeometry> transformedGeometry;
        //hr = m_pD2DFactory->CreateTransformedGeometry(m_pPathGeometry, matrix, &transformedGeometry);
        //if (FAILED(hr))
        //    return hr;

        //if (m_pFillBrush)
        //    m_pRenderTarget->FillGeometry(transformedGeometry, m_pFillBrush);
        //if (m_pDrawBrush)
        //    m_pRenderTarget->DrawGeometry(transformedGeometry, m_pDrawBrush, m_strokeStyle);

        return hr;
    }

    IFACEMETHOD(DrawUnderline)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect
        ) override
    {
        // 实现下划线绘制（可选）

        CComPtr<ID2D1RectangleGeometry> pGeometry = nullptr;
        D2D1_RECT_F rect = { 0.f, underline->offset, underline->width, underline->offset + underline->thickness };
        HRESULT hr = m_pD2DFactory->CreateRectangleGeometry(rect, &pGeometry);
        if (FAILED(hr))
            return hr;

        D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
            1.0f, 0.0f,
            0.0f, 1.0f,
            baselineOriginX, baselineOriginY
        );
        CComPtr<ID2D1TransformedGeometry> transformedGeometry = nullptr;
        hr = m_pD2DFactory->CreateTransformedGeometry(pGeometry, matrix, &transformedGeometry);
        if (FAILED(hr))
            return hr;

        if (m_pFillBrush)
            m_pRenderTarget->FillGeometry(transformedGeometry, m_pFillBrush);
        if (m_pDrawBrush)
            m_pRenderTarget->DrawGeometry(transformedGeometry, m_pDrawBrush, m_strokeStyle);

        return S_OK;
    }

    IFACEMETHOD(DrawStrikethrough)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect
        ) override
    {
        // 实现删除线绘制（可选）
        CComPtr<ID2D1RectangleGeometry> pGeometry = nullptr;
        D2D1_RECT_F rect = { 0.f, strikethrough->offset, strikethrough->width, strikethrough->offset + strikethrough->thickness };
        HRESULT hr = m_pD2DFactory->CreateRectangleGeometry(rect, &pGeometry);
        if (FAILED(hr))
            return hr;

        D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
            1.0f, 0.0f,
            0.0f, 1.0f,
            baselineOriginX, baselineOriginY
        );


        CComPtr<ID2D1TransformedGeometry> transformedGeometry = nullptr;
        hr = m_pD2DFactory->CreateTransformedGeometry(pGeometry, matrix, &transformedGeometry);
        if (FAILED(hr))
            return hr;

        if (m_pFillBrush)
            m_pRenderTarget->FillGeometry(transformedGeometry, m_pFillBrush);
        if (m_pDrawBrush)
            m_pRenderTarget->DrawGeometry(transformedGeometry, m_pDrawBrush, m_strokeStyle);

        return S_OK;
    }

    IFACEMETHOD(DrawInlineObject)(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
        ) override
    {
        // 实现内联对象绘制（可选）
        return E_NOTIMPL;
    }

private:
    ULONG m_refCount;
    ID2D1Factory* m_pD2DFactory;
    ID2D1DeviceContext* m_pRenderTarget;
    ID2D1Brush* m_pFillBrush;
    ID2D1Brush* m_pDrawBrush;
    ID2D1GeometrySink* m_pGeometrySink;
    ID2D1PathGeometry* m_pPathGeometry;
    float m_strokeStyle;
};

// 绘画相关的内容
void lyric_wnd_draw(LYRIC_WND_INFU& wnd_info, int pos, const RECT& rcWindow, LYRIC_CALC_STRUCT& arg)
{
    CD2DRender& hCanvas = *wnd_info.hCanvas;
    LYRIC_LINE_STRUCT& line = arg.line;
    LYRIC_WORD_STRUCT& word = arg.word;
    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;
    HRESULT hr = 0;

    //static int prev_width, prev_index;
    //if (prev_width == arg.nLineWidth && prev_index == arg.indexLine)
    //    return;
    //prev_width = arg.nLineWidth;
    //prev_index = arg.indexLine;

    D2D1_RECT_F rcText;
    D2D1_RECT_F rcText2;
    float top = (float)(cyClient - wnd_info.nLineHeight - wnd_info.scale(10));
    float left = (float)wnd_info.scale(16);
    float height = (float)wnd_info.nLineHeight;
    const int fmt = DT_LEFT | DT_TOP | DT_NOPREFIX | DT_SINGLELINE;
    auto& d2dInfo = d2d_get_info();
    CD2DFont& font = *wnd_info.hFont;


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
    ID2D1LinearGradientBrush* hbrNormal = *wnd_info.hbrNormal;
    ID2D1LinearGradientBrush* hbrLight = *wnd_info.hbrLight;
    ID2D1SolidColorBrush* hbrBack = *wnd_info.hbrBack;
    hCanvas->FillRectangle(rcBack, hbrBack);


    CComPtr<ID2D1PathGeometry> textGeometry = nullptr;
    CComPtr<ID2D1GeometrySink> sink = nullptr;
    hr = d2dInfo.pFactory->CreatePathGeometry(&textGeometry);
    textGeometry->Open(&sink);

    CD2DBrush hbr_red(MAKEARGB(255, 0, 0, 0));
    CustomTextRenderer rand(d2dInfo.pFactory, hCanvas, 0, hbr_red, sink, textGeometry, 1.f);

    float calcWidth = 0.f, calcHeight = 0.f;
    IDWriteTextLayout* pTextLayout = 0;
    IDWriteInlineObject* pWriteInlineObject = font;
    IDWriteTextFormat* dxFormat = font;
    dxFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    dxFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    hr = d2dInfo.pDWriteFactory->CreateTextLayout(line.pText,
                                                  (UINT32)line.nLength, dxFormat,
                                                  //rcText.right - rcText.left, rcText.bottom - rcText.top,
                                                  rcBack.right - rcBack.left, rcBack.bottom - rcBack.top,
                                                  &pTextLayout);

    D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
        1.0f, 0.0f,
        0.0f, 1.0f,
        0.f, 0.f
    );
    hCanvas->SetTransform(matrix);
    pTextLayout->Draw(0, &rand, 50.f, 50.f);

    sink->Close();


    //hr = pTextLayout->Draw(NULL, &renderer, 100.0f, 100.0f);

    IDWriteInlineObject* _fmt = *wnd_info.hFont;
    //pTextLayout->Draw(nullptr, _fmt, 100.0f, 100.0f);
    D2D1_RECT_F bounds;
    textGeometry->GetBounds(matrix, &bounds);


    //D2D1_POINT_2F startPoint = { (float)(cxClient / 2), rcText.top - bounds.top };
    //D2D1_POINT_2F endPoint = { (float)(cxClient / 2), rcText.bottom - bounds.top };

    float start_top_left = (bounds.right - bounds.left) / 2;
    D2D1_POINT_2F startPoint = { start_top_left, bounds.top };
    D2D1_POINT_2F endPoint = { start_top_left, bounds.bottom };
    hbrNormal->SetStartPoint(startPoint);
    hbrNormal->SetEndPoint(endPoint);
    //hCanvas->FillRectangle(&rcText2, hbrNormal);



    D2D1_MATRIX_3X2_F oldTransform;
    hCanvas->GetTransform(&oldTransform);

    // 6. **应用新变换（平移，使顶部对齐到 y=0）**
    float translateTop = -bounds.top + rcText.top;
    D2D1_MATRIX_3X2_F newTransform = D2D1::Matrix3x2F::Translation(rcText.left, translateTop);
    hCanvas->SetTransform(&newTransform);

    //D2D1_RECT_F rcRgn = { rcText2.right, rcText2.top, rcText.right, rcText2.bottom };
    D2D1_RECT_F rcRgn = { bounds.left + rcText2.right, bounds.top, bounds.right, bounds.bottom };
    hCanvas->PushAxisAlignedClip(rcRgn, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);


    hCanvas->DrawGeometry(textGeometry, hbr_red, 1.f, nullptr);
    hCanvas->FillGeometry(textGeometry, hbrNormal);

    hCanvas->PopAxisAlignedClip();

    rcRgn = { bounds.left, bounds.top, rcText2.right, bounds.bottom };
    hCanvas->PushAxisAlignedClip(rcRgn, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);


    D2D1_POINT_2F pt = { rcText.left, rcText.top };
    //hCanvas->DrawTextLayout(pt, pTextLayout, hbrNormal, D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);

    hbrLight->SetStartPoint(startPoint);
    hbrLight->SetEndPoint(endPoint);
    hCanvas->DrawGeometry(textGeometry, hbr_red, 1.f, nullptr);
    hCanvas->FillGeometry(textGeometry, hbrLight);

    //_brush_setstartpoint(wnd_info.hbrLight, (float)(cxClient / 2), rcText.top);
    //_brush_setendpoint(wnd_info.hbrLight, (float)(cxClient / 2), (float)(rcText.bottom));
    //hCanvas->drawtext(wnd_info.hFont, wnd_info.hbrLight, line.pText, line.nLength, fmt, &rcText2);

    hCanvas->SetTransform(&oldTransform);
    hCanvas->PopAxisAlignedClip();

    pTextLayout->Release();

}

LRESULT CALLBACK lyric_wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto pWndInfo = (LYRIC_WND_INFU*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
    if (!pWndInfo)
        return DefWindowProcW(hWnd, message, wParam, lParam);

    switch (message)
    {
    case WM_MOVE:
        break;
    case WM_LBUTTONDOWN:
    case WM_NCLBUTTONDOWN:
    {
        return DefWindowProcW(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
    }
    case WM_DESTROY:
    {
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
        delete pWndInfo;
        return 0;
    }
    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}


NAMESPACE_LYRIC_WND_END


