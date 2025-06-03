#pragma once
#include <windows.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <functional>

using PFN_DRAW_GLYPH_RUN = void(void* clientDrawingContext,
                                FLOAT baselineOriginX,
                                FLOAT baselineOriginY,
                                DWRITE_MEASURING_MODE measuringMode,
                                DWRITE_GLYPH_RUN const* glyphRun,
                                DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                                IUnknown* clientDrawingEffect
                                );


class CCustomTextRenderer : public IDWriteTextRenderer
{
    ULONG m_refCount;   // 引用计数
    float m_width;      // 文本宽度
    float m_height;     // 文本高度
    std::function<PFN_DRAW_GLYPH_RUN> m_pfnDrawGlyphRun;
public:
    CCustomTextRenderer();

    CCustomTextRenderer(std::function<PFN_DRAW_GLYPH_RUN> pfn);

    ~CCustomTextRenderer();

    float get_width() const { return m_width; }
    float get_height() const { return m_height; }

    IFACEMETHOD(QueryInterface)(REFIID riid, void** ppvObject) override;

    IFACEMETHOD_(ULONG, AddRef)() override;
    IFACEMETHOD_(ULONG, Release)() override;
    IFACEMETHOD(IsPixelSnappingDisabled)(
        void* clientDrawingContext,
        BOOL* isDisabled
        ) override;

    IFACEMETHOD(GetCurrentTransform)(
        void* clientDrawingContext,
        DWRITE_MATRIX* transform
        ) override;

    IFACEMETHOD(GetPixelsPerDip)(
        void* clientDrawingContext,
        FLOAT* pixelsPerDip
        ) override;

    IFACEMETHOD(DrawGlyphRun)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect
        ) override;

    IFACEMETHOD(DrawUnderline)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect
        ) override;

    IFACEMETHOD(DrawStrikethrough)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect
        ) override;

    IFACEMETHOD(DrawInlineObject)(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
        ) override;

private:
    bool calc_text(DWRITE_GLYPH_RUN const* glyphRun, DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription);

public:

    template<typename T>static void SafeRelease(T*& pObj)
    {
        if (pObj)
            pObj->Release();
        pObj = nullptr;
    }
    template<typename T>static void SafeAddref(T* pObj)
    {
        if (pObj)
            pObj->AddRef();
    }

};

