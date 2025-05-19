// 绘画歌词文本, 路径的方式绘画

#include "lyric_wnd_function.h"

using namespace NAMESPACE_D2D;

constexpr float strokeWidth = 2.0f; // 歌词文本描边宽度

NAMESPACE_LYRIC_WND_BEGIN

class CustomTextRenderer : public IDWriteTextRenderer
{
private:
    ULONG m_refCount;
    ID2D1DeviceContext* m_pRenderTarget;
    ID2D1PathGeometry* m_pPathGeometry;
    bool m_is_vertical;
    float m_text_height;
public:
    CustomTextRenderer(
        ID2D1DeviceContext* pRenderTarget,
        ID2D1PathGeometry* pPathGeometry,
        bool is_vertical,   // 是否是竖排歌词
        float text_height   // 文本高度
    ) : m_refCount(1),
        m_pRenderTarget(pRenderTarget),
        m_pPathGeometry(pPathGeometry),
        m_is_vertical(is_vertical),
        m_text_height(text_height)
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

    inline bool isLatinCharacter(wchar_t ch)
    {
        return
            // Printable ASCII characters: 0x21 ('!') to 0x7E ('~')
            (ch >= 0x20 && ch <= 0x7E) ||

            // Latin-1 Supplement
            (ch >= 0x00C0 && ch <= 0x00D6) || // 
            (ch >= 0x00D8 && ch <= 0x00F6) || // 
            (ch >= 0x00F8 && ch <= 0x00FF) || // 

            // Latin Extended-A
            (ch >= 0x0100 && ch <= 0x017F) ||

            // Latin Extended-B
            (ch >= 0x0180 && ch <= 0x024F);
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
        auto& d2dInfo = d2d_get_info();

        CComPtr<ID2D1GeometrySink> finalSink;
        hr = m_pPathGeometry->Open(&finalSink);
        if (FAILED(hr))
            return hr;
        float offset_x = 0.f, offset_y = 0.f;
        for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
        {
            // 1. 创建一个临时 path geometry 用来写入字形路径
            CComPtr<ID2D1PathGeometry> tempGeometry;
            hr = d2dInfo.pFactory->CreatePathGeometry(&tempGeometry);
            if (FAILED(hr))
                break;

            CComPtr<ID2D1GeometrySink> pSink;
            hr = tempGeometry->Open(&pSink);
            if (FAILED(hr))
                break;

#define _get_value(_v) ((_v) ? (&_v[i]) : nullptr)
            DWRITE_GLYPH_RUN oneGlyph = *glyphRun;
            oneGlyph.glyphCount = 1;
            oneGlyph.glyphIndices  = _get_value(glyphRun->glyphIndices);
            oneGlyph.glyphAdvances = _get_value(glyphRun->glyphAdvances);
            oneGlyph.glyphOffsets = _get_value(glyphRun->glyphOffsets);
#undef _get_value

            IDWriteFontFace* pFontFace = glyphRun->fontFace;
            if (!pFontFace || !oneGlyph.glyphIndices)
                break;


            wchar_t ch = glyphRunDescription && glyphRunDescription->string ? glyphRunDescription->string[i] : L'\0';
            bool is_alpha = isLatinCharacter(ch);
            // 当前字符的 advance 宽度
            float glyphAdvance = (glyphRun->glyphAdvances ? glyphRun->glyphAdvances[i] : 0.0f);


            // 获取字形轮廓并写入几何体
            hr = pFontFace->GetGlyphRunOutline(
                oneGlyph.fontEmSize,
                oneGlyph.glyphIndices,
                oneGlyph.glyphAdvances,
                oneGlyph.glyphOffsets,
                oneGlyph.glyphCount, // 一个字
                oneGlyph.isSideways,
                FALSE, // rtl
                pSink
            );
            pSink->Close();

            if (FAILED(hr))
                break;

            // 获取字体度量
            DWRITE_FONT_METRICS metrics;
            glyphRun->fontFace->GetMetrics(&metrics);
            float designUnitsPerEm = (float)metrics.designUnitsPerEm;

            float ascent = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;

            D2D1_RECT_F bounds2 = {};
            tempGeometry->GetBounds(nullptr, &bounds2);
            // 构建字符变换矩阵
            D2D1::Matrix3x2F transform;
            if (m_is_vertical)
            {
                // 竖屏, 需要判断字符是否是字母, 
                // 如果是字母, 则需要将字母的高度加到 offset_y, 否则只需要加上字母的宽度
                if (is_alpha)
                {
                    //// 对字母字符进行逆时针旋转 90 度，再平移到 offset 位置
                    //D2D1::Matrix3x2F rotate = D2D1::Matrix3x2F::Rotation(90.0f);
                    //D2D1::Matrix3x2F translate = D2D1::Matrix3x2F::Translation(
                    //    offset_x, // 修正中心偏移
                    //    offset_y
                    //);
                    //transform = rotate * translate;
                    ////transform = D2D1::Matrix3x2F::Translation(offset_x, offset_y);

                    // 字母偏移中心点，先旋转 90 度，再向下移动 ascent 修正
                    float ascent = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;

                    // 用 rotated baseline 的顶部对齐
                    D2D1::Matrix3x2F rotate = D2D1::Matrix3x2F::Rotation(90.0f);

                    // 修正中心点平移，额外减去 ascent
                    D2D1::Matrix3x2F translate = D2D1::Matrix3x2F::Translation(
                        offset_x + 10,
                        offset_y - ascent
                    );

                    transform = rotate * translate;
                    //transform = translate * rotate;

                    offset_y += glyphAdvance; // 继续向下排
                }
                else
                {
                    transform = D2D1::Matrix3x2F::Translation(offset_x, offset_y);
                }
            }
            else
            {
                transform = D2D1::Matrix3x2F::Translation(offset_x, offset_y);
                offset_x += glyphAdvance;
            }

            CComPtr<ID2D1TransformedGeometry> transformed;
            hr = d2dInfo.pFactory->CreateTransformedGeometry(
                tempGeometry,
                &transform,
                &transformed
            );



            if(m_is_vertical && !is_alpha)
            {
                DWRITE_FONT_METRICS metrics;
                glyphRun->fontFace->GetMetrics(&metrics);

                // 将设计单位转换为实际像素：fontEmSize 是字形渲染大小（如 32.0f）
                float designUnitsPerEm = (float)metrics.designUnitsPerEm;

                float ascent = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;
                float descent = glyphRun->fontEmSize * metrics.descent / designUnitsPerEm;
                float lineGap = glyphRun->fontEmSize * metrics.lineGap / designUnitsPerEm;

                // 实际推荐行高：
                float lineSpacing = ascent + descent + lineGap;

                // 如果你希望只考虑绘制高度：
                float visualHeight = ascent + descent;


                D2D1_RECT_F bounds = {};
                tempGeometry->GetBounds(nullptr, &bounds);
                float text_height = bounds.bottom - bounds.top;
                offset_y += ascent;
            }


            // 5. 把变换后的 geometry 写入目标 PathGeometry

            //hr = transformed->Outline(nullptr, finalSink);
            hr = transformed->Simplify(
                D2D1_GEOMETRY_SIMPLIFICATION_OPTION_CUBICS_AND_LINES,
                nullptr, // world transform
                finalSink
            );

            if (FAILED(hr))
                break;
        }
        finalSink->Close();

        return hr;

        //// 将字形转换为几何图形
        //CComPtr<ID2D1GeometrySink> pSink = nullptr;
        //hr = m_pPathGeometry->Open(&pSink);
        //if (FAILED(hr)) return hr;
        //
        //hr = glyphRun->fontFace->GetGlyphRunOutline(
        //    glyphRun->fontEmSize,
        //    glyphRun->glyphIndices,
        //    glyphRun->glyphAdvances,
        //    glyphRun->glyphOffsets,
        //    glyphRun->glyphCount,
        //    glyphRun->isSideways,
        //    glyphRun->bidiLevel % 2,
        //    pSink
        //);
        //if (FAILED(hr))
        //    return hr;
        //pSink->Close();
        //return hr;
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

// 绘画到缓存位图里, 两个位图, 一个绘画高亮, 一个绘画普通
void lyric_wnd_draw_text_geometry_draw_cache(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);


void lyric_wnd_draw_text_geometry(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex)
{
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    ID2D1DeviceContext* pRenderTarget = hCanvas;

    D2D1_ANTIALIAS_MODE oldMode = pRenderTarget->GetAntialiasMode();
    pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);  // 关闭抗锯齿
    lyric_wnd_draw_text_geometry_draw_cache(wnd_info, draw_info, nDrawLineIndex);
    pRenderTarget->SetAntialiasMode(oldMode);  // 恢复默认
}

void lyric_wnd_draw_text_geometry_draw_cache(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex)
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



    LPCWSTR pszText = line.pText;
    UINT32 nLength = (UINT32)line.nLength;
    if (nDrawLineIndex == 2 && wnd_info.has_mode(LYRIC_MODE::EXISTTRANS))
    {
        if (wnd_info.has_mode(LYRIC_MODE::TRANSLATION1))
            pszText = line.pTranslate1, nLength = (UINT32)line.nTranslate1;
        else if (wnd_info.has_mode(LYRIC_MODE::TRANSLATION2))
            pszText = line.pTranslate2, nLength = (UINT32)line.nTranslate2;
    }

    draw_info.text_wtdth = 0.f;     // 每次进来都先清零, 需要重新计算
    draw_info.text_height = 0.f;
    if (nLength && pszText && *pszText)
    {
        hr = d2dInfo.pFactory->CreatePathGeometry(&pTextGeometry);
        if (FAILED(hr))
            return;

        //dxFormat->SetReadingDirection(DWRITE_READING_DIRECTION_BOTTOM_TO_TOP);
        DWRITE_LINE_SPACING_METHOD a1;
        float lineSpacing, baseline;
        dxFormat->GetLineSpacing(&a1, &lineSpacing, &baseline);

        pTextLayout = lyric_wnd_create_text_layout(pszText, (UINT32)nLength, dxFormat, 10000, 20000);

        if (!pTextLayout)
            return;

        // 获取第一行的度量
        DWRITE_LINE_METRICS lineMetrics;
        UINT32 lineCount;
        pTextLayout->GetLineMetrics(&lineMetrics, 1, &lineCount);
        // 获取文本范围内的实际字形度量
        DWRITE_HIT_TEST_METRICS hitTestMetrics;
        float x = 0, y = 0;
        pTextLayout->HitTestTextPosition(0, FALSE, &x, &y, &hitTestMetrics);


        DWRITE_TEXT_METRICS metrics = { 0 };
        pTextLayout->GetMetrics(&metrics);
        draw_info.text_wtdth = metrics.widthIncludingTrailingWhitespace;
        draw_info.text_height = metrics.height;

        lyric_wnd_draw_calc_text_rect(wnd_info, draw_info, nDrawLineIndex);
    }


    //D2D1_STROKE_STYLE_PROPERTIES1 strokeProps = D2D1::StrokeStyleProperties1(
    //    D2D1_CAP_STYLE_FLAT,
    //    D2D1_CAP_STYLE_FLAT,
    //    D2D1_CAP_STYLE_FLAT,
    //    D2D1_LINE_JOIN_MITER,
    //    10.0f,  // miterLimit
    //    D2D1_STROKE_TRANSFORM_TYPE_FIXED,  // 关键：防止笔触向外扩展
    //    D2D1_DASH_STYLE_SOLID
    //);
    //d2dInfo.pFactory->CreateStrokeStyle1(
    //    &strokeProps,
    //    nullptr,
    //    0,
    //    &pStrokeStyle
    //);

    // 1. 创建笔触样式（防止笔触向外扩展）
    D2D1_STROKE_STYLE_PROPERTIES strokeProps = D2D1::StrokeStyleProperties(
        D2D1_CAP_STYLE_ROUND,   // 线帽：平头（避免额外延伸）
        D2D1_CAP_STYLE_ROUND,
        D2D1_CAP_STYLE_ROUND,
        D2D1_LINE_JOIN_MITER,   // 连接方式：斜接（避免圆角延伸）
        10.f,                   // 斜接限制
        D2D1_DASH_STYLE_SOLID   // 实线
    );

    CComPtr<ID2D1StrokeStyle> pStrokeStyle = nullptr;
    //d2dInfo.pFactory->CreateStrokeStyle(&strokeProps, nullptr, 0, &pStrokeStyle);
    const bool is_vertical = wnd_info.has_mode(LYRIC_MODE::VERTICAL);
    CustomTextRenderer rand(pRenderTarget, pTextGeometry, is_vertical, draw_info.text_height);  // 获取文本路径信息


    // 绘画左边顶边偏移的位置, 不从0开始, 画阴影部分会小于0, 这里偏移一些像素
    //const float _offset = (float)wnd_info.scale(10);
    const float _offset = 0;

    auto pfn_draw_text = [&](ID2D1LinearGradientBrush* hbrFill, ID2D1Bitmap*& pBitmap) -> void
    {
        // 弄个新的渲染目标, 把数据绘画到这个目标上, 然后取出位图保存
        CComPtr<ID2D1BitmapRenderTarget> pRender = nullptr;

        // 没有文本, 不需要绘画, 只需要把位图清空
        if (!pTextLayout && pBitmap)
        {
            CComPtr<ID2D1Image> pOldImage;
            pRenderTarget->GetTarget(&pOldImage);
            pRenderTarget->SetTarget(pBitmap);
            pRenderTarget->Clear(0);
            pRenderTarget->SetTarget(pOldImage);
            return;
        }

        // 走到这就是有文本, 或者需要创建位图
        SafeRelease(pBitmap);

        // 需要左右各多出一部分显示阴影
        float width = (draw_info.text_wtdth ? draw_info.text_wtdth : wnd_info.nLineDefWidth) + _offset * 2;
        float height = draw_info.text_height;
        if (is_vertical)
        {
            width = draw_info.text_height;
            height = (draw_info.line.nHeight ? draw_info.line.nHeight : wnd_info.nLineDefWidth) + _offset * 2;
        }

        D2D1_SIZE_F size = D2D1::SizeF(width, height);
        hr = pRenderTarget->CreateCompatibleRenderTarget(size, &pRender);
        if (FAILED(hr))
            return;

        pRender->BeginDraw();
        pRender->Clear();

        if (hbrFill == hbrNormal)
            pRender->Clear(ARGB_D2D(MAKEARGB(200, 255, 0, 0)));
        else
            pRender->Clear(ARGB_D2D(MAKEARGB(200, 0, 255, 0)));

        if (pTextLayout)
        {
            D2D1::Matrix3x2F matrix = D2D1::Matrix3x2F(
                1.0f, 0.0f,
                0.0f, 1.0f,
                0.f, 0.f
            );

            hr = pTextLayout->Draw(0, &rand, 0.f, 0.f);

            D2D1_RECT_F bounds = { 0 };
            hr = pTextGeometry->GetBounds(matrix, &bounds);

            float start_top_left = (bounds.right - bounds.left) / 2;
            POINT_F startPoint = { start_top_left, bounds.top };
            POINT_F endPoint = { start_top_left, bounds.bottom };
            hbrFill->SetStartPoint(startPoint);
            hbrFill->SetEndPoint(endPoint);

            // 平移到路径那个起始位置
            float translateTop = -bounds.top + _offset;


            D2D1_MATRIX_3X2_F newTransform = D2D1::Matrix3x2F::Translation(_offset, translateTop);

            hr = pTextGeometry->GetWidenedBounds(strokeWidth, pStrokeStyle, newTransform, &draw_info.cache.rcBounds);

            CD2DBrush* _hbr_back;
            if (hbrFill == hbrNormal)
                _hbr_back = new CD2DBrush(hCanvas, MAKEARGB(200, 255, 0, 0));
            else
                _hbr_back = new CD2DBrush(hCanvas, MAKEARGB(200, 0, 255, 0));

            pRender->FillRectangle(draw_info.cache.rcBounds, *_hbr_back);
            delete _hbr_back;

            pRender->SetTransform(&newTransform);

            //CD2DBrush _hbrBak(hCanvas, hbrFill == hbrLight ? MAKEARGB(255, 255, 255, 255) : MAKEARGB(255, 0, 0, 0));
            //CD2DBrush _hbrBak2(hCanvas, hbrFill == hbrNormal ? MAKEARGB(255, 255, 255, 255) : MAKEARGB(255, 0, 0, 0));
            //pRender->DrawGeometry(pTextGeometry, _hbrBak2, 1, pStrokeStyle);
            //pRender->DrawGeometry(pTextGeometry, _hbrBak, strokeWidth, pStrokeStyle);
            //pRender->FillGeometry(pTextGeometry, _hbrBak2);  // 调用方指定填充的画刷



            pRender->DrawGeometry(pTextGeometry, hbrBorder, strokeWidth, pStrokeStyle);
            pRender->FillGeometry(pTextGeometry, hbrFill);  // 调用方指定填充的画刷

 


            pRender->SetTransform(&matrix);

            //CD2DBrush hbrBak(MAKEARGB(180, 255, 0, 0));
            //pRender->FillRectangle(draw_info.cache->rcBounds, hbrBak);
        }

        hr = pRender->EndDraw();
        if (FAILED(hr))
            return;

        // 绘画结束, 取出位图返回
        //TODO 这里应该创建一个位图, 然后画到这个位图里面
        // 这个位图的尺寸就是计算出来的边界矩形的大小

        hr = pRender->GetBitmap(&pBitmap);
        return;
    };


    pfn_draw_text(hbrNormal, draw_info.cache.pBitmapNormal);
    pfn_draw_text(hbrLight, draw_info.cache.pBitmapLight);

}

NAMESPACE_LYRIC_WND_END
