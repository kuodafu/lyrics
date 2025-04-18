#include "lyric_wnd_header.h"

using namespace NAMESPACE_D2D;



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

        // 将字形转换为几何图形
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
            glyphRun->bidiLevel != 0,
            pSink
        );
        pSink->Close();
        if (FAILED(hr))
            return hr;

        //D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
        //    1.0f, 0.0f,
        //    0.0f, 1.0f,
        //    baselineOriginX, baselineOriginY
        //);


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
        //// 实现下划线绘制（可选）

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
        //// 实现删除线绘制（可选）
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
        // 实现内联对象绘制（可选）
        return E_NOTIMPL;
    }


};

// 将文本加入到路几何图形径中
bool lyric_wnd_geometry_add_string(ID2D1DeviceContext* pRenderTarget, ID2D1PathGeometry* pPathGeometry, IDWriteTextLayout* pTextLayout)
{
    CustomTextRenderer rand(pRenderTarget, pPathGeometry);

    D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
        1.0f, 0.0f,
        0.0f, 1.0f,
        0.f, 0.f
    );
    pRenderTarget->SetTransform(matrix);
    pTextLayout->Draw(0, &rand, 0.f, 0.f);

    return true;
}

bool lyric_wnd_draw_glow_text(ID2D1DeviceContext* pRenderTarget, IDWriteTextFormat* pTextFormat, LYRIC_CALC_STRUCT& arg,
                              ID2D1LinearGradientBrush* hbrNormal, const D2D1_RECT_F& rcText);

bool lyric_wnd_draw_geometry(ID2D1DeviceContext* pRenderTarget, ID2D1PathGeometry* pPathGeometry,
                             ID2D1LinearGradientBrush* hbrNormal, ID2D1LinearGradientBrush* hbrLight,
                             ID2D1SolidColorBrush* hbrDraw,
                             const D2D1_RECT_F& rcText, const D2D1_RECT_F& rcText2,
                             LYRIC_CALC_STRUCT& arg, IDWriteTextFormat* dxFormat)
{
    D2D1_MATRIX_3X2_F oldTransform;
    pRenderTarget->GetTransform(&oldTransform);
    float strokeWidth = 2.2f;
    D2D1_RECT_F bounds;
    HRESULT hr = pPathGeometry->GetBounds(oldTransform, &bounds);

    float start_top_left = (bounds.right - bounds.left) / 2;
    POINT_F startPoint = { start_top_left, bounds.top };
    POINT_F endPoint = { start_top_left, bounds.bottom };
    hbrNormal->SetStartPoint(startPoint);
    hbrNormal->SetEndPoint(endPoint);

    float translateTop = -bounds.top + rcText.top;
    D2D1_MATRIX_3X2_F newTransform = D2D1::Matrix3x2F::Translation(rcText.left, translateTop);
    pRenderTarget->SetTransform(&newTransform);

    const float _offset = 20.f;

    // 设置剪切区, 普通歌词左边的位置只能在高亮部分后面显示
    D2D1_RECT_F rcRgn = { rcText2.right, bounds.top - _offset, bounds.right + _offset, bounds.bottom + _offset };
    pRenderTarget->PushAxisAlignedClip(rcRgn, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    auto& d2dInfo = d2d_get_info();

    //CComPtr<ID2D1StrokeStyle> strokeStyle;
    //D2D1_STROKE_STYLE_PROPERTIES strokeStyleProperties =
    //{
    //    D2D1_CAP_STYLE_SQUARE,
    //    D2D1_CAP_STYLE_SQUARE,
    //    D2D1_CAP_STYLE_SQUARE,
    //    D2D1_LINE_JOIN_MITER,
    //    strokeWidth,
    //    D2D1_DASH_STYLE_SOLID,
    //    0.0f
    //};
    //d2dInfo.pFactory->CreateStrokeStyle(strokeStyleProperties, 0, 0, &strokeStyle);

    //D2D1_RECT_F rcTest = rcRgn;
    //rcTest.left = rcText2.left;
    //lyric_wnd_draw_glow_text(pRenderTarget, dxFormat, arg, hbrNormal, rcTest);
    pRenderTarget->DrawGeometry(pPathGeometry, hbrDraw, strokeWidth, nullptr);
    pRenderTarget->FillGeometry(pPathGeometry, hbrNormal);

    // 还原普通歌词剪切区
    pRenderTarget->PopAxisAlignedClip();

    // 高亮部分的右边只能显示到高亮的部分
    rcRgn = { bounds.left - rcText2.left, bounds.top - _offset, rcText2.right, bounds.bottom + _offset };
    pRenderTarget->PushAxisAlignedClip(rcRgn, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    hbrLight->SetStartPoint(startPoint);
    hbrLight->SetEndPoint(endPoint);
    pRenderTarget->DrawGeometry(pPathGeometry, hbrDraw, strokeWidth, nullptr);
    pRenderTarget->FillGeometry(pPathGeometry, hbrLight);

    // 还原高亮歌词剪切区
    pRenderTarget->PopAxisAlignedClip();

    pRenderTarget->SetTransform(&oldTransform);

    return true;
}

// 绘画发光文本
bool lyric_wnd_draw_glow_text(ID2D1DeviceContext* pRenderTarget, IDWriteTextFormat* pTextFormat, LYRIC_CALC_STRUCT& arg,
                              ID2D1LinearGradientBrush* hbrNormal, const D2D1_RECT_F& rcText)
{
    HRESULT hr = S_OK;

    // 计算文本尺寸
    D2D1_SIZE_F size = D2D1::SizeF(rcText.right - rcText.left, rcText.bottom - rcText.top);

    // 创建兼容的位图渲染目标
    CComPtr<ID2D1BitmapRenderTarget> pTextBitmapRT = nullptr;
    hr = pRenderTarget->CreateCompatibleRenderTarget(size, &pTextBitmapRT);
    if (FAILED(hr))
        return false;

    // 绘制文本到位图
    pTextBitmapRT->BeginDraw();
    pTextBitmapRT->Clear(D2D1::ColorF(0, 0, 0, 0)); // 透明背景
    pTextBitmapRT->DrawText(
        arg.line.pText, arg.line.nLength, pTextFormat,
        D2D1::RectF(0, 0, size.width, size.height),
        hbrNormal);
    pTextBitmapRT->EndDraw();

    // 获取位图
    ID2D1Bitmap* pCachedTextBitmap = nullptr;
    hr = pTextBitmapRT->GetBitmap(&pCachedTextBitmap);

    D2D1_COLOR_F g_GlowColor = D2D1::ColorF(0.0f, 0.8f, 1.0f, 1.0f);

    // 1. 绘制多重发光层(模拟光晕)
    for (int i = 8; i >= 1; i--) {
        float radius = 3.0f * i;
        float opacity = 0.8f / i;

        // 创建模糊效果
        ID2D1Effect* pBlurEffect = nullptr;
        pRenderTarget->CreateEffect(CLSID_D2D1GaussianBlur, &pBlurEffect);
        pBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, radius);
        pBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_GAUSSIANBLUR_OPTIMIZATION_QUALITY);

        // 设置输入
        pBlurEffect->SetInput(0, pCachedTextBitmap);

        // 创建颜色矩阵效果(调整发光颜色)
        ID2D1Effect* pColorMatrixEffect = nullptr;
        pRenderTarget->CreateEffect(CLSID_D2D1ColorMatrix, &pColorMatrixEffect);

        D2D1_MATRIX_5X4_F matrix = {
            g_GlowColor.r, 0, 0, 0,  // R
            0, g_GlowColor.g, 0, 0,   // G
            0, 0, g_GlowColor.b, 0,   // B
            0, 0, 0, opacity,         // Alpha
            0, 0, 0, 0               // 偏移
        };
        pColorMatrixEffect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
        pColorMatrixEffect->SetInputEffect(0, pBlurEffect);

        // 绘制发光层
        pRenderTarget->DrawImage(pColorMatrixEffect, D2D1::Point2F(rcText.left, rcText.top));

        SafeRelease(pBlurEffect);
        SafeRelease(pColorMatrixEffect);
    }

    pRenderTarget->DrawText(arg.line.pText, arg.line.nLength, pTextFormat, rcText, hbrNormal);

    return true;
}

NAMESPACE_LYRIC_WND_END