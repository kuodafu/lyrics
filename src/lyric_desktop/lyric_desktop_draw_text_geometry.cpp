// 绘画歌词文本, 路径的方式绘画

#include "lyric_desktop_function.h"
#include <d2d/CCustomTextRenderer.h>
#include <atlbase.h>


using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN

struct GlyphGeometryInfo
{
    wchar_t ch = 0;         // 处理的字符
    bool vertical = false;  // 是否要旋转
    float width = 0.0f;     // 这个字符的宽度
    float height = 0.0f;    // 这个字符的高度

    D2D1_MATRIX_3X2_F transform{};
    D2D1_RECT_F bounds{};
    CComPtr<ID2D1TransformedGeometry> geometry; // 这个字符的几何图形
};


// 绘画到缓存位图里, 两个位图, 一个绘画高亮, 一个绘画普通
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
    pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);  // 关闭抗锯齿
    lyric_wnd_draw_text_geometry_draw_cache(wnd_info, draw_info, nDrawLineIndex);
    pRenderTarget->SetAntialiasMode(oldMode);  // 恢复默认
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

    //pszText = L"Think About It (想一想) - Avril Lavigne (艾薇儿·拉维尼)";
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
    const bool is_vertical = wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL);
    std::vector<GlyphGeometryInfo> m_glyphGeometries;

    // 调用后每个文字的路径信息都记录到数组里
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


    // 绘画左边顶边偏移的位置, 不从0开始, 画阴影部分会小于0, 这里偏移一些像素
    const float _offset = wnd_info.config.padding_text;
    //const float _offset = 0;

    auto pfn_draw_text = [&](ID2D1LinearGradientBrush* hbrFill, ID2D1Bitmap*& pBitmap) -> void
    {
        // 没有文本, 不需要绘画, 只需要把位图清空
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

        // 走到这就是有文本, 或者需要创建位图
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

        // 弄个新的渲染目标, 把数据绘画到这个目标上, 然后取出位图保存
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
            // 绘画结束, 取出位图返回
            //TODO 这里应该创建一个位图, 然后画到这个位图里面
            // 这个位图的尺寸就是计算出来的边界矩形的大小
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
            // 竖屏模式, 渐变区域从右到左
            float start_top_left = (bounds.bottom - bounds.top) / 2;
            POINT_F startPoint = { bounds.right, start_top_left };
            POINT_F endPoint = { bounds.left, start_top_left };
            pFillBrush->SetStartPoint(startPoint);
            pFillBrush->SetEndPoint(endPoint);
        }
        else
        {
            // 横屏模式, 渐变区域从上到下
            float start_top_left = (bounds.right - bounds.left) / 2;
            POINT_F startPoint = { start_top_left, bounds.top };
            POINT_F endPoint = { start_top_left, bounds.bottom };
            pFillBrush->SetStartPoint(startPoint);
            pFillBrush->SetEndPoint(endPoint);
        }
        // 平移到路径那个起始位置

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
        // 1. 创建一个临时 path geometry 用来写入字形路径
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
                draw_info.text_height += item.width;    // 旋转的字按宽度累加
            else
                draw_info.text_height += item.height;
        }
        else
        {
            if (draw_info.text_height < item.height)
                draw_info.text_height = item.height;

            draw_info.text_width += item.width;
        }
        // 构建字符变换矩阵
        if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL))
        {
            // 竖屏, 需要判断字符是否是字母, 
            // 如果是字母, 则需要将字母的高度加到 offset_y, 否则只需要加上字母的宽度
            if (is_alpha)
            {

                // 用 rotated baseline 的顶部对齐
                D2D1::Matrix3x2F rotate = D2D1::Matrix3x2F::Rotation(90.0f);

                // 修正中心点平移，额外减去 ascent
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
