// �滭����ı�, ·���ķ�ʽ�滭

#include "lyric_desktop_function.h"
#include <d2d/CCustomTextRenderer.h>
#include <atlbase.h>


using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN

struct GlyphGeometryInfo
{
    wchar_t ch = 0;         // ������ַ�
    bool vertical = false;  // �Ƿ�Ҫ��ת
    float width = 0.0f;     // ����ַ��Ŀ��
    float height = 0.0f;    // ����ַ��ĸ߶�

    D2D1_MATRIX_3X2_F transform{};
    D2D1_RECT_F bounds{};
    CComPtr<ID2D1TransformedGeometry> geometry; // ����ַ��ļ���ͼ��
};


// �滭������λͼ��, ����λͼ, һ���滭����, һ���滭��ͨ
void lyric_wnd_draw_text_geometry_draw_cache(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);
void DrawGlyphGeometriesWithDebugText(
    ID2D1RenderTarget* pRenderTarget,
    LYRIC_DESKTOP_INFO& wnd_info,
    LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info,
    std::vector<GlyphGeometryInfo>& m_glyphGeometries,
    ID2D1LinearGradientBrush* pFillBrush,
    ID2D1Brush* pStrokeBrush,
    CCustomTextRenderer& render,
    float strokeWidth,
    float offset_left
);

void lyric_wnd_draw_geometry_DrawGlyphRun(LYRIC_DESKTOP_INFO& wnd_info,
                                          LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info,
                                          std::vector<GlyphGeometryInfo>& m_glyphGeometries,
                                          void* clientDrawingContext,
                                          FLOAT baselineOriginX,
                                          FLOAT baselineOriginY,
                                          DWRITE_MEASURING_MODE measuringMode,
                                          DWRITE_GLYPH_RUN const* glyphRun,
                                          DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                                          IUnknown* clientDrawingEffect);




void lyric_wnd_draw_text_geometry(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, int nDrawLineIndex)
{
    D2DRender& pRender = *wnd_info.dx.pRender;
    ID2D1DeviceContext* pRenderTarget = pRender.GetD2DContext();

    D2D1_ANTIALIAS_MODE oldMode = pRenderTarget->GetAntialiasMode();
    pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);  // �رտ����
    lyric_wnd_draw_text_geometry_draw_cache(wnd_info, draw_info, nDrawLineIndex);
    pRenderTarget->SetAntialiasMode(oldMode);  // �ָ�Ĭ��
}



void lyric_wnd_draw_text_geometry_draw_cache(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, int nDrawLineIndex)
{
    D2DRender& pRender = *wnd_info.dx.pRender;
    ID2D1LinearGradientBrush* hbrNormal = wnd_info.dx.hbrNormal->GetNative();
    ID2D1LinearGradientBrush* hbrLight = wnd_info.dx.hbrLight->GetNative();
    ID2D1SolidColorBrush* hbrBorderNormal = wnd_info.dx.hbrBorderNormal->GetNative();
    ID2D1SolidColorBrush* hbrBorderLight = wnd_info.dx.hbrBorderLight->GetNative();
    IDWriteTextFormat* dxFormat = wnd_info.dx.hFont->GetDWTextFormat();
    ID2D1DeviceContext* pRenderTarget = pRender.GetD2DContext();
    ID2D1Factory1* pFactory = pRender.GetD2DInterface()->GetD2DFactory();

    LYRIC_LINE_STRUCT& line = draw_info.line;

    CComPtr<ID2D1PathGeometry> pTextGeometry;
    CComPtr<IDWriteTextLayout> pTextLayout;
    HRESULT hr = S_OK;


    draw_info.text_width = 0;
    draw_info.text_height = 0;

    LPCWSTR pszText = line.pText;
    UINT32 nLength = (UINT32)line.nLength;
    if (nDrawLineIndex == 2 && wnd_info.has_mode(LYRIC_DESKTOP_MODE::EXISTTRANS))
    {
        if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_FY))
            pszText = line.pTranslate1, nLength = (UINT32)line.nTranslate1;
        else if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_YY))
            pszText = line.pTranslate2, nLength = (UINT32)line.nTranslate2;
    }

    //pszText = L"Think About It (��һ��) - Avril Lavigne (��ޱ������ά��)";
    //pszText = L"Take it with no regrets and that is what I'll do";
    //pszText = L"'Cause I can't be bought";
    //nLength = (UINT32)wcslen(pszText);

    if (nLength && pszText && *pszText)
    {
        hr = pFactory->CreatePathGeometry(&pTextGeometry);
        if (FAILED(hr))
            return;

        //dxFormat->SetReadingDirection(DWRITE_READING_DIRECTION_BOTTOM_TO_TOP);
        lyric_wnd_create_text_layout(pszText, (UINT32)nLength, dxFormat, 0, 0, &pTextLayout);

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
    const bool is_vertical = wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL);
    std::vector<GlyphGeometryInfo> m_glyphGeometries;

    // ���ú�ÿ�����ֵ�·����Ϣ����¼��������
    CCustomTextRenderer renderer([&](void* clientDrawingContext,
                                     FLOAT baselineOriginX,
                                     FLOAT baselineOriginY,
                                     DWRITE_MEASURING_MODE measuringMode,
                                     DWRITE_GLYPH_RUN const* glyphRun,
                                     DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                                     IUnknown* clientDrawingEffect
                                     )
                                 {
                                     lyric_wnd_draw_geometry_DrawGlyphRun(wnd_info, draw_info, m_glyphGeometries, clientDrawingContext,
                                                                          baselineOriginX, baselineOriginY,
                                                                          measuringMode,
                                                                          glyphRun, glyphRunDescription, clientDrawingEffect);
                                     return S_OK;
                                 });
    if (pTextLayout)
    {
        pTextLayout->Draw(nullptr, &renderer, 0, 0);
        lyric_wnd_draw_calc_text_rect(wnd_info, draw_info, nDrawLineIndex);
    }


    // �滭��߶���ƫ�Ƶ�λ��, ����0��ʼ, ����Ӱ���ֻ�С��0, ����ƫ��һЩ����
    const float _offset = wnd_info.config.padding_text;
    //const float _offset = 0;

    auto pfn_draw_text = [&](ID2D1LinearGradientBrush* hbrFill, ID2D1Bitmap*& pBitmap) -> void
    {
        // û���ı�, ����Ҫ�滭, ֻ��Ҫ��λͼ���
        if (!pTextLayout && pBitmap)
        {
            ID2D1Image* pOldImage = nullptr;
            pRenderTarget->GetTarget(&pOldImage);
            pRenderTarget->SetTarget(pBitmap);
            pRenderTarget->Clear(nullptr);
            pRenderTarget->SetTarget(pOldImage);
            SafeRelease(pOldImage);
            return;
        }

        // �ߵ���������ı�, ������Ҫ����λͼ
        SafeRelease(pBitmap);

        float width, height;
        if (is_vertical)
        {
            width = wnd_info.get_lyric_line_height();
            height = wnd_info.get_lyric_line_width(draw_info.text_height);
        }
        else
        {
            height = wnd_info.get_lyric_line_height();
            width = wnd_info.get_lyric_line_width(draw_info.text_width);
        }

        // Ū���µ���ȾĿ��, �����ݻ滭�����Ŀ����, Ȼ��ȡ��λͼ����
        ID2D1BitmapRenderTarget* pRender = nullptr;

        D2D1_SIZE_F size = D2D1::SizeF(width, height);
        hr = pRenderTarget->CreateCompatibleRenderTarget(size, &pRender);
        if (FAILED(hr))
            return;

        const bool isLight = (hbrFill == hbrLight);
        pRender->BeginDraw();
        pRender->Clear();
        if (wnd_info.config.debug.clrTextBackLight || wnd_info.config.debug.clrTextBackNormal)
        {
            if (isLight)
                pRender->Clear(ARGB_D2D(wnd_info.config.debug.clrTextBackLight));
            else
                pRender->Clear(ARGB_D2D(wnd_info.config.debug.clrTextBackNormal));
        }

        auto hbrBorder = isLight ? hbrBorderLight : hbrBorderNormal;

        if (pTextLayout)
        {
            float strokeWidth = wnd_info.config.strokeWidth;
            if (wnd_info.config.strokeWidth_div > 0.f)
            {
                UINT dpi = wnd_info.scale;
                int nFontSize = -MulDiv(wnd_info.config.nFontSize, dpi, 72);
                if (nFontSize < 0) nFontSize *= -1;

                strokeWidth = (float)nFontSize / wnd_info.config.strokeWidth_div;
            }
            DrawGlyphGeometriesWithDebugText(pRender,
                                             wnd_info,
                                             draw_info,
                                             m_glyphGeometries,
                                             hbrFill, hbrBorder,
                                             renderer, strokeWidth, _offset);
        }

        hr = pRender->EndDraw();
        if (SUCCEEDED(hr))
        {
            // �滭����, ȡ��λͼ����
            //TODO ����Ӧ�ô���һ��λͼ, Ȼ�󻭵����λͼ����
            // ���λͼ�ĳߴ���Ǽ�������ı߽���εĴ�С
            hr = pRender->GetBitmap(&pBitmap);
        }
        SafeRelease(pRender);
        return;
    };


    pfn_draw_text(hbrNormal, draw_info.cache.pBitmapNormal);
    pfn_draw_text(hbrLight, draw_info.cache.pBitmapLight);

}


void DrawGlyphGeometriesWithDebugText(
    ID2D1RenderTarget* pRenderTarget,
    LYRIC_DESKTOP_INFO& wnd_info,
    LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info,
    std::vector<GlyphGeometryInfo>& m_glyphGeometries,
    ID2D1LinearGradientBrush* pFillBrush,
    ID2D1Brush* pStrokeBrush,
    CCustomTextRenderer& render,
    float strokeWidth,
    float offset_left
)
{
    D2D1::Matrix3x2F matrix;
    pRenderTarget->GetTransform(&matrix);
    HRESULT hr = S_OK;
    bool is_vertical = wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL);

    D2D1_RECT_F max_bound_top{};
    D2D1_RECT_F max_bound_left{};
    float min_left = 0;
    float min_top = 0;
    float max_right = 0;
    float max_bottom = 0;

    for (GlyphGeometryInfo& info : m_glyphGeometries)
    {
        if (!info.geometry)
            continue;
        auto& geometry = info.geometry;

        hr = geometry->GetBounds(nullptr, &info.bounds);

        if (std::isfinite(info.bounds.left))
        {
            if (min_left > info.bounds.left)
                min_left = info.bounds.left;
            if (min_top > info.bounds.top)
                min_top = info.bounds.top;
            if (max_right < info.bounds.right)
                max_right = info.bounds.right;
            if (max_bottom < info.bounds.bottom)
                max_bottom = info.bounds.bottom;

            if (max_bound_top.top > info.bounds.top)
                max_bound_top = info.bounds;
            if (max_bound_left.left > info.bounds.left)
                max_bound_left = info.bounds;
        }

    }

    //max_bound_top = m_glyphGeometries.front().bounds;
    //max_bound_left = m_glyphGeometries.front().bounds;
    //max_width = max_bound_left.right - max_bound_left.left;

    float max_width = max_right - min_left;
    float max_height = max_bottom - min_top;

    float line_height = wnd_info.get_lyric_line_height();
    float offset_y = (line_height - (max_height)) / 2;
    //float offset_x = (line_height - (max_bound_top.right - max_bound_top.left)) / 2 + 2;
    float offset_x = (line_height - (max_width)) / 2;


    D2D1_POINT_2F offset = {};
    for (const GlyphGeometryInfo& info : m_glyphGeometries)
    {
        if (!info.geometry)
            continue;
        auto& geometry = info.geometry;

        const D2D1_RECT_F& bounds = info.bounds;

        if (is_vertical)
        {
            // ����ģʽ, ����������ҵ���
            float start_top_left = (bounds.bottom - bounds.top) / 2;
            POINT_F startPoint = { bounds.right, start_top_left };
            POINT_F endPoint = { bounds.left, start_top_left };
            pFillBrush->SetStartPoint(startPoint);
            pFillBrush->SetEndPoint(endPoint);
        }
        else
        {
            // ����ģʽ, ����������ϵ���
            float start_top_left = (bounds.right - bounds.left) / 2;
            POINT_F startPoint = { start_top_left, bounds.top };
            POINT_F endPoint = { start_top_left, bounds.bottom };
            pFillBrush->SetStartPoint(startPoint);
            pFillBrush->SetEndPoint(endPoint);
        }
        // ƽ�Ƶ�·���Ǹ���ʼλ��

        D2D1_MATRIX_3X2_F newTransform;
        if (is_vertical)
        {
            float x = 0;
            if (info.vertical)
                x =  0;
            else
                x = (line_height - (bounds.right - bounds.left)) / 2;
            //x = (line_height - (bounds.right - bounds.left)) / 2;
            const float y = info.vertical ? info.width : info.height;
            float _offset_y = (y - (bounds.bottom - bounds.top)) / 2;

            D2D1_POINT_2F translatePoint = { x, -bounds.top + offset.y + offset_left + _offset_y };
            newTransform = D2D1::Matrix3x2F::Translation(translatePoint.x, translatePoint.y);
            offset.y += y;
        }
        else
        {
            D2D1_POINT_2F translatePoint = { offset.x + offset_left, -max_bound_top.top + offset.y + offset_y };
            newTransform = D2D1::Matrix3x2F::Translation(translatePoint.x, translatePoint.y);
            offset.x += info.width;
        }

        pRenderTarget->SetTransform(&newTransform);
        if (wnd_info.config.fillBeforeDraw)
        {
            pRenderTarget->FillGeometry(geometry, pFillBrush);
            pRenderTarget->DrawGeometry(geometry, pStrokeBrush, strokeWidth);
        }
        else
        {
            pRenderTarget->DrawGeometry(geometry, pStrokeBrush, strokeWidth);
            pRenderTarget->FillGeometry(geometry, pFillBrush);
        }

        pRenderTarget->SetTransform(matrix);
        //break;
    }
}

void lyric_wnd_draw_geometry_DrawGlyphRun(LYRIC_DESKTOP_INFO& wnd_info,
                                          LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info,
                                          std::vector<GlyphGeometryInfo>& m_glyphGeometries,
                                          void* clientDrawingContext,
                                          FLOAT baselineOriginX,
                                          FLOAT baselineOriginY,
                                          DWRITE_MEASURING_MODE measuringMode,
                                          DWRITE_GLYPH_RUN const* glyphRun,
                                          DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                                          IUnknown* clientDrawingEffect)
{

    ID2D1Factory1* pFactory = wnd_info.dx.pRender->GetD2DInterface()->GetD2DFactory();

    const bool is_vertical = wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL);

    //draw_info.text_wtdth = 0;
    //draw_info.text_height = 0;

    HRESULT hr = S_OK;
    for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
    {
        // 1. ����һ����ʱ path geometry ����д������·��
        CComPtr<ID2D1PathGeometry1> tempGeometry;
        hr = pFactory->CreatePathGeometry(&tempGeometry);
        if (FAILED(hr))
            break;

        CComPtr<ID2D1GeometrySink> pSink;
        hr = tempGeometry->Open(&pSink);
        if (FAILED(hr))
            break;

        GlyphGeometryInfo item;

#define _get_value(_v) ((_v) ? (&_v[i]) : nullptr)
        DWRITE_GLYPH_RUN oneGlyph = *glyphRun;
        oneGlyph.glyphCount = 1;
        oneGlyph.glyphIndices = _get_value(glyphRun->glyphIndices);
        oneGlyph.glyphAdvances = _get_value(glyphRun->glyphAdvances);
        oneGlyph.glyphOffsets = _get_value(glyphRun->glyphOffsets);
#undef _get_value

        IDWriteFontFace* pFontFace = glyphRun->fontFace;
        if (!pFontFace || !oneGlyph.glyphIndices)
            break;

        // ��ȡ����������д�뼸����
        hr = pFontFace->GetGlyphRunOutline(
            oneGlyph.fontEmSize,
            oneGlyph.glyphIndices,
            oneGlyph.glyphAdvances,
            oneGlyph.glyphOffsets,
            oneGlyph.glyphCount, // һ����
            oneGlyph.isSideways,
            FALSE, // rtl
            pSink
        );
        pSink->Close();

        if (FAILED(hr))
            break;

        // ��ȡ�������
        DWRITE_FONT_METRICS metrics{};
        glyphRun->fontFace->GetMetrics(&metrics);
        item.ch = glyphRunDescription && glyphRunDescription->string ? glyphRunDescription->string[i] : L'\0';
        bool is_alpha = isLatinCharacter(item.ch);

        float designUnitsPerEm = (float)metrics.designUnitsPerEm;
        item.height = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;
        item.width = (glyphRun->glyphAdvances ? glyphRun->glyphAdvances[i] : 0.0f);

        if (is_vertical)
        {
            if (draw_info.text_width < item.width)
                draw_info.text_width = item.width;
            if (is_alpha)
                draw_info.text_height += item.width;    // ��ת���ְ�����ۼ�
            else
                draw_info.text_height += item.height;
        }
        else
        {
            if (draw_info.text_height < item.height)
                draw_info.text_height = item.height;

            draw_info.text_width += item.width;
        }
        // �����ַ��任����
        if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL))
        {
            // ����, ��Ҫ�ж��ַ��Ƿ�����ĸ, 
            // �������ĸ, ����Ҫ����ĸ�ĸ߶ȼӵ� offset_y, ����ֻ��Ҫ������ĸ�Ŀ��
            if (is_alpha)
            {

                // �� rotated baseline �Ķ�������
                D2D1::Matrix3x2F rotate = D2D1::Matrix3x2F::Rotation(90.0f);

                // �������ĵ�ƽ�ƣ������ȥ ascent
                D2D1::Matrix3x2F translate = D2D1::Matrix3x2F::Translation(
                    item.height / 4,
                    0
                );

                item.transform = rotate * translate;
                item.vertical = true;
            }
            else
            {
                item.transform = D2D1::Matrix3x2F::Translation(0, 0);
            }
        }
        else
        {
            item.transform = D2D1::Matrix3x2F::Translation(0, 0);
        }

        CComPtr<ID2D1TransformedGeometry> transformed;
        hr = pFactory->CreateTransformedGeometry(
            tempGeometry,
            &item.transform,
            &transformed
        );

        if (FAILED(hr))
            break;

        item.geometry = transformed;
        m_glyphGeometries.push_back(item);

    }

    return;
}

NAMESPACE_LYRIC_DESKTOP_END
