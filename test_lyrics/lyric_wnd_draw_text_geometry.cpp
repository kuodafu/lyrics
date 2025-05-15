// �滭����ı�, ·���ķ�ʽ�滭

#include "lyric_wnd_function.h"

using namespace NAMESPACE_D2D;

constexpr float strokeWidth = 2.0f; // ����ı���߿��

NAMESPACE_LYRIC_WND_BEGIN

class CustomTextRenderer : public IDWriteTextRenderer
{
private:
    ULONG m_refCount;
    ID2D1DeviceContext* m_pRenderTarget;
    ID2D1PathGeometry* m_pPathGeometry;

public:
    CustomTextRenderer(
        ID2D1DeviceContext* pRenderTarget,
        ID2D1PathGeometry* pPathGeometry
    ) : m_refCount(1),
        m_pRenderTarget(pRenderTarget),
        m_pPathGeometry(pPathGeometry)
    {
        SafeAddref(m_pRenderTarget);
        SafeAddref(m_pPathGeometry);
    }

    ~CustomTextRenderer()
    {
        SafeRelease(m_pRenderTarget);
        SafeRelease(m_pPathGeometry);
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
        SafeAddref(m_pRenderTarget);
        SafeAddref(m_pPathGeometry);
        return InterlockedIncrement(&m_refCount);
    }

    IFACEMETHOD_(ULONG, Release)() override
    {
        ULONG newCount = InterlockedDecrement(&m_refCount);
        SafeRelease(m_pRenderTarget);
        SafeRelease(m_pPathGeometry);
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

        // ������ת��Ϊ����ͼ��
        CComPtr<ID2D1GeometrySink> pSink = nullptr;
        hr = m_pPathGeometry->Open(&pSink);
        if (FAILED(hr)) return hr;

        hr = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            glyphRun->bidiLevel % 2,
            pSink
        );
        if (FAILED(hr))
            return hr;
        pSink->Close();

        //D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
        //    1.0f, 0.0f,
        //    0.0f, 1.0f,
        //    baselineOriginX, baselineOriginY
        //);


        // ������ͼ����ӵ�������ͼ����
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
        //// ʵ���»��߻��ƣ���ѡ��

        //CComPtr<ID2D1RectangleGeometry> pGeometry = nullptr;
        //D2D1_RECT_F rect = { 0.f, underline->offset, underline->width, underline->offset + underline->thickness };
        //HRESULT hr = m_pD2DFactory->CreateRectangleGeometry(rect, &pGeometry);
        //if (FAILED(hr))
        //    return hr;

        //D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
        //    1.0f, 0.0f,
        //    0.0f, 1.0f,
        //    baselineOriginX, baselineOriginY
        //);
        //CComPtr<ID2D1TransformedGeometry> transformedGeometry = nullptr;
        //hr = m_pD2DFactory->CreateTransformedGeometry(pGeometry, matrix, &transformedGeometry);
        //if (FAILED(hr))
        //    return hr;

        //if (m_pFillBrush)
        //    m_pRenderTarget->FillGeometry(transformedGeometry, m_pFillBrush);
        //if (m_pDrawBrush)
        //    m_pRenderTarget->DrawGeometry(transformedGeometry, m_pDrawBrush, m_strokeStyle);

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
        //// ʵ��ɾ���߻��ƣ���ѡ��
        //CComPtr<ID2D1RectangleGeometry> pGeometry = nullptr;
        //D2D1_RECT_F rect = { 0.f, strikethrough->offset, strikethrough->width, strikethrough->offset + strikethrough->thickness };
        //HRESULT hr = m_pD2DFactory->CreateRectangleGeometry(rect, &pGeometry);
        //if (FAILED(hr))
        //    return hr;

        //D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
        //    1.0f, 0.0f,
        //    0.0f, 1.0f,
        //    baselineOriginX, baselineOriginY
        //);


        //CComPtr<ID2D1TransformedGeometry> transformedGeometry = nullptr;
        //hr = m_pD2DFactory->CreateTransformedGeometry(pGeometry, matrix, &transformedGeometry);
        //if (FAILED(hr))
        //    return hr;

        //if (m_pFillBrush)
        //    m_pRenderTarget->FillGeometry(transformedGeometry, m_pFillBrush);
        //if (m_pDrawBrush)
        //    m_pRenderTarget->DrawGeometry(transformedGeometry, m_pDrawBrush, m_strokeStyle);

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
        // ʵ������������ƣ���ѡ��
        return E_NOTIMPL;
    }


};

// �滭������λͼ��, ����λͼ, һ���滭����, һ���滭��ͨ
void lyric_wnd_draw_text_geometry_draw_cache(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info);


void lyric_wnd_draw_text_geometry(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info)
{
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    ID2D1DeviceContext* pRenderTarget = hCanvas;

    D2D1_ANTIALIAS_MODE oldMode = pRenderTarget->GetAntialiasMode();
    pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);  // �رտ����
    lyric_wnd_draw_text_geometry_draw_cache(wnd_info, draw_info);
    pRenderTarget->SetAntialiasMode(oldMode);  // �ָ�Ĭ��
}

void lyric_wnd_draw_text_geometry_draw_cache(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info)
{
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    CD2DFont& font = *wnd_info.dx.hFont;
    ID2D1LinearGradientBrush* hbrNormal = *wnd_info.dx.hbrNormal;
    ID2D1LinearGradientBrush* hbrLight = *wnd_info.dx.hbrLight;
    ID2D1SolidColorBrush* hbrBorder = *wnd_info.dx.hbrBorder;
    IDWriteTextFormat* dxFormat = font;
    ID2D1DeviceContext* pRenderTarget = hCanvas;

    auto& d2dInfo = d2d_get_info();

    LYRIC_LINE_STRUCT& line = draw_info.line;

    CComPtr<ID2D1PathGeometry> pTextGeometry;
    CComPtr<IDWriteTextLayout> pTextLayout;
    HRESULT hr = S_OK;

    if (draw_info.line.nLength && draw_info.line.pText && *draw_info.line.pText)
    {
        hr = d2dInfo.pFactory->CreatePathGeometry(&pTextGeometry);
        if (FAILED(hr))
            return;

        pTextLayout = lyric_wnd_create_text_layout(line.pText, (UINT32)line.nLength, dxFormat,
                                                   draw_info.layout_text_max_width, draw_info.layout_text_max_height);

        if (!pTextLayout)
            return;
    }


    //D2D1_STROKE_STYLE_PROPERTIES1 strokeProps = D2D1::StrokeStyleProperties1(
    //    D2D1_CAP_STYLE_FLAT,
    //    D2D1_CAP_STYLE_FLAT,
    //    D2D1_CAP_STYLE_FLAT,
    //    D2D1_LINE_JOIN_MITER,
    //    10.0f,  // miterLimit
    //    D2D1_STROKE_TRANSFORM_TYPE_FIXED,  // �ؼ�����ֹ�ʴ�������չ
    //    D2D1_DASH_STYLE_SOLID
    //);
    //d2dInfo.pFactory->CreateStrokeStyle1(
    //    &strokeProps,
    //    nullptr,
    //    0,
    //    &pStrokeStyle
    //);

    // 1. �����ʴ���ʽ����ֹ�ʴ�������չ��
    D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties(
        D2D1_CAP_STYLE_ROUND,   // ��ñ��ƽͷ������������죩
        D2D1_CAP_STYLE_ROUND,
        D2D1_CAP_STYLE_ROUND,
        D2D1_LINE_JOIN_MITER,   // ���ӷ�ʽ��б�ӣ�����Բ�����죩
        10.f,                   // б������
        D2D1_DASH_STYLE_SOLID   // ʵ��
    );

    CComPtr<ID2D1StrokeStyle> pStrokeStyle = nullptr;
    //d2dInfo.pFactory->CreateStrokeStyle(&strokeProps, nullptr, 0, &pStrokeStyle);

    CustomTextRenderer rand(pRenderTarget, pTextGeometry);  // ��ȡ�ı�·����Ϣ

    // �滭��߶���ƫ�Ƶ�λ��, ����0��ʼ, ����Ӱ���ֻ�С��0, ����ƫ��һЩ����
    const float _offset = (draw_info.layout_text_max_height - wnd_info.nLineHeight) / 2;

    auto pfn_draw_text = [&](ID2D1LinearGradientBrush* hbrFill) -> ID2D1Bitmap*
    {
        // Ū���µ���ȾĿ��, �����ݻ滭�����Ŀ����, Ȼ��ȡ��λͼ����
        CComPtr<ID2D1BitmapRenderTarget> pRender = nullptr;
        // ��Ҫ���Ҹ����һ������ʾ��Ӱ
        float width = (line.nWidth ? line.nWidth : wnd_info.nLineDefWidth) + _offset * 2;

        //width = draw_info.layout_text_max_width;
        //D2D1_SIZE_F size = D2D1::SizeF(draw_info.layout_text_max_width, draw_info.layout_text_max_height);
        D2D1_SIZE_F size = D2D1::SizeF(width, draw_info.layout_text_max_height);
        hr = pRenderTarget->CreateCompatibleRenderTarget(size, &pRender);
        if (FAILED(hr))
            return nullptr;

        pRender->BeginDraw();
        //if (hbrFill == draw_info.hbrNormal)
        //    pRender->Clear(ARGB_D2D(MAKEARGB(200, 255, 0, 0)));
        //else
        //    pRender->Clear(ARGB_D2D(MAKEARGB(200, 0, 255, 0)));

        pRender->Clear();

        if (pTextLayout)
        {
            D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
                1.0f, 0.0f,
                0.0f, 1.0f,
                0.f, 0.f
            );

            pTextLayout->Draw(0, &rand, 0.f, 0.f);

            D2D1_RECT_F bounds = { 0 };
            hr = pTextGeometry->GetBounds(matrix, &bounds);

            float start_top_left = (bounds.right - bounds.left) / 2;
            POINT_F startPoint = { start_top_left, bounds.top };
            POINT_F endPoint = { start_top_left, bounds.bottom };
            hbrFill->SetStartPoint(startPoint);
            hbrFill->SetEndPoint(endPoint);

            // ƽ�Ƶ�·���Ǹ���ʼλ��
            float translateTop = -bounds.top + _offset;

            D2D1_MATRIX_3X2_F newTransform = D2D1::Matrix3x2F::Translation(_offset, translateTop);
            pRender->SetTransform(&newTransform);

            //CD2DBrush _hbrBak(hbrFill == hbrLight ? MAKEARGB(255, 255, 255, 255) : MAKEARGB(255, 0, 0, 0));
            //CD2DBrush _hbrBak2(hbrFill == hbrNormal ? MAKEARGB(255, 255, 255, 255) : MAKEARGB(255, 0, 0, 0));
            //pRender->DrawGeometry(pTextGeometry, _hbrBak2, 1, pStrokeStyle);
            //pRender->DrawGeometry(pTextGeometry, _hbrBak, strokeWidth, pStrokeStyle);
            //pRender->FillGeometry(pTextGeometry, _hbrBak2);  // ���÷�ָ�����Ļ�ˢ

            pRender->DrawGeometry(pTextGeometry, hbrBorder, strokeWidth, pStrokeStyle);
            pRender->FillGeometry(pTextGeometry, hbrFill);  // ���÷�ָ�����Ļ�ˢ

            hr = pTextGeometry->GetWidenedBounds(strokeWidth, pStrokeStyle, newTransform, &draw_info.cache.rcBounds);

            pRender->SetTransform(&matrix);

            //CD2DBrush hbrBak(MAKEARGB(180, 255, 0, 0));
            //pRender->FillRectangle(draw_info.cache->rcBounds, hbrBak);
        }

        hr = pRender->EndDraw();
        if (FAILED(hr))
            return nullptr;

        // �滭����, ȡ��λͼ����
        //TODO ����Ӧ�ô���һ��λͼ, Ȼ�󻭵����λͼ����
        // ���λͼ�ĳߴ���Ǽ�������ı߽���εĴ�С

        ID2D1Bitmap* pBitmap = nullptr;
        hr = pRender->GetBitmap(&pBitmap);
        if (SUCCEEDED(hr))
            return pBitmap;
        return nullptr;
    };


    SafeRelease(draw_info.cache.pBitmapNormal);
    SafeRelease(draw_info.cache.pBitmapLight);
    draw_info.cache.pBitmapNormal = pfn_draw_text(hbrNormal);
    draw_info.cache.pBitmapLight = pfn_draw_text(hbrLight);

}

NAMESPACE_LYRIC_WND_END
