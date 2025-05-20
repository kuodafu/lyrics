#include "CCustomTextRenderer.h"
#include <atlbase.h>

CCustomTextRenderer::CCustomTextRenderer()
    : m_pfnDrawGlyphRun(nullptr)
    , m_refCount(1)
    , m_width(0)
    , m_height(0)
{
}


CCustomTextRenderer::CCustomTextRenderer(std::function<PFN_DRAW_GLYPH_RUN> pfn)
    : m_pfnDrawGlyphRun(pfn)
    , m_refCount(1)
    , m_width(0)
    , m_height(0)
{
}

CCustomTextRenderer::~CCustomTextRenderer()
{
}

HRESULT CCustomTextRenderer::QueryInterface(REFIID riid, void** ppvObject)
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

ULONG CCustomTextRenderer::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}
ULONG CCustomTextRenderer::Release()
{
    ULONG newCount = InterlockedDecrement(&m_refCount);
    return newCount;
}

HRESULT  CCustomTextRenderer::IsPixelSnappingDisabled(
    void* clientDrawingContext,
    BOOL* isDisabled
)
{
    *isDisabled = FALSE;
    return S_OK;
}

HRESULT  CCustomTextRenderer::GetCurrentTransform(
    void* clientDrawingContext,
    DWRITE_MATRIX* transform
)
{
    transform->m11 = 1.0f; transform->m12 = 0.0f;
    transform->m21 = 0.0f; transform->m22 = 1.0f;
    transform->dx = 0.0f;  transform->dy = 0.0f;
    return S_OK;
}

HRESULT  CCustomTextRenderer::GetPixelsPerDip(
    void* clientDrawingContext,
    FLOAT* pixelsPerDip
)
{
    *pixelsPerDip = 1.0f;
    return S_OK;
}


HRESULT  CCustomTextRenderer::DrawGlyphRun(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    DWRITE_GLYPH_RUN const* glyphRun,
    DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
    IUnknown* clientDrawingEffect
)
{
    if (m_pfnDrawGlyphRun)
    {
        m_pfnDrawGlyphRun(clientDrawingContext, baselineOriginX, baselineOriginY, measuringMode, glyphRun, glyphRunDescription, clientDrawingEffect);
        return S_OK;
    }

    calc_text(glyphRun, glyphRunDescription);
    return S_OK;
}

HRESULT  CCustomTextRenderer::DrawUnderline(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_UNDERLINE const* underline,
    IUnknown* clientDrawingEffect
)
{
    // 实现下划线绘制（可选）
    return S_OK;
}

HRESULT  CCustomTextRenderer::DrawStrikethrough(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_STRIKETHROUGH const* strikethrough,
    IUnknown* clientDrawingEffect
)
{
    // 实现删除线绘制（可选）

    return S_OK;
}

HRESULT  CCustomTextRenderer::DrawInlineObject(
    void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
)
{
    // 实现内联对象绘制（可选）
    return E_NOTIMPL;
}

bool CCustomTextRenderer::calc_text(DWRITE_GLYPH_RUN const* glyphRun, DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription)
{
    for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
    {
        DWRITE_FONT_METRICS metrics{};
        glyphRun->fontFace->GetMetrics(&metrics);

        float designUnitsPerEm = (float)metrics.designUnitsPerEm;
        float height = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;
        float width = (glyphRun->glyphAdvances ? glyphRun->glyphAdvances[i] : 0.0f);

        if (m_height < height)
            m_height = height;

        m_width += width;
    }

    return true;
}
