#include "lyric_wnd_function.h"
#include <d2d/CCustomTextRenderer.h>
#include "GetMonitorRect.h"
#include <atlbase.h>

using namespace KUODAFU_NAMESPACE;


NAMESPACE_LYRIC_DESKTOP_BEGIN


bool LYRIC_DESKTOP_DX::re_create(LYRIC_DESKTOP_INFO* pWndInfo)
{
    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    auto& clrNormal = wnd_info.clrNormal;
    auto& clrLight = wnd_info.clrLight;
    RECT rc;
    GetClientRect(wnd_info.hWnd, &rc);
    const int cxClient = rc.right - rc.left;
    const int cyClient = rc.bottom - rc.top;

    SafeRelease(hCanvas);
    hCanvas = g_d2d_interface->CreateRender(cxClient, cyClient, true);
    hCanvas->GetD2DContext()->QueryInterface(&pGDIInterop);

    SafeRelease(image_shadow);

    size_t imgSize = 0;
    LPBYTE pImageData = _lrc_dwsktop_get_shadow_image(imgSize);


    hCanvas->CreateImage(pImageData, imgSize, &image_shadow);

    if (!hFont)
        re_create_font(pWndInfo);

    re_create_brush(pWndInfo, true);
    re_create_brush(pWndInfo, false);
    re_create_border(pWndInfo);
    re_create_image(pWndInfo);

    SafeRelease(hbrWndBorder);
    SafeRelease(hbrWndBack);
    SafeRelease(hbrLine);

    hCanvas->CreateSolidBrush(clrWndBorder, &hbrWndBorder);
    hCanvas->CreateSolidBrush(clrBack, &hbrWndBack);
    hCanvas->CreateSolidBrush(MAKEARGB(100, 255, 255, 255), &hbrLine);

    return true;
}

bool LYRIC_DESKTOP_DX::re_create_brush(LYRIC_DESKTOP_INFO* pWndInfo, bool isLight)
{
    DWORD* pClr;
    int size;
    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    D2DLinearGradientBrush** ppBrush;
    if (isLight)
    {
        SafeRelease(hbrLight);
        pClr = &wnd_info.clrLight[0];
        size = (int)wnd_info.clrLight.size();
        ppBrush = &hbrLight;
    }
    else
    {
        SafeRelease(hbrNormal);
        pClr = &wnd_info.clrNormal[0];
        size = (int)wnd_info.clrNormal.size();
        ppBrush = &hbrNormal;
    }

    POINT_F pt1 = { 0.f, 0.f };
    POINT_F pt2 = { 1.f, .1f };
    D2DBRUSH_CREATE_STRUCT arg{};
    arg.color = pClr;
    arg.colorCount = size;
    HRESULT hr = hCanvas->CreateLinearGradientBrush(&pt1, &pt2, &arg, ppBrush);
    return SUCCEEDED(hr);
}

bool LYRIC_DESKTOP_DX::re_create_border(LYRIC_DESKTOP_INFO* pWndInfo)
{
    SafeRelease(hbrBorder);
    HRESULT hr = hCanvas->CreateSolidBrush(pWndInfo->clrBorder, &hbrBorder);
    return SUCCEEDED(hr);
}

bool LYRIC_DESKTOP_DX::re_create_font(LYRIC_DESKTOP_INFO* pWndInfo)
{
    SafeRelease(hFont);

    LOGFONT lf = pWndInfo->lf;
    lf.lfHeight = -MulDiv(lf.lfHeight, pWndInfo->scale, 72);
    hFont = new CD2DFont(hCanvas, &lf);

    CComPtr<IDWriteTextLayout> pTextLayout;
    lyric_wnd_create_text_layout(pWndInfo->pszDefText, pWndInfo->nDefText, *hFont, 0, 0, &pTextLayout);
    if (pTextLayout)
    {
        float width = 0.f, height = 0.f;
        pWndInfo->word_width = 0;
        pWndInfo->word_height = 0;
        pWndInfo->nLineDefWidth = 0;
        pWndInfo->nLineDefHeight = 0;

        CCustomTextRenderer renderer([&](void* clientDrawingContext,
                                         FLOAT baselineOriginX,
                                         FLOAT baselineOriginY,
                                         DWRITE_MEASURING_MODE measuringMode,
                                         DWRITE_GLYPH_RUN const* glyphRun,
                                         DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
                                         IUnknown* clientDrawingEffect)
                                     {
                                         // 获取字体度量
                                         for (UINT32 i = 0; i < glyphRun->glyphCount; ++i)
                                         {
                                             DWRITE_FONT_METRICS metrics{};
                                             glyphRun->fontFace->GetMetrics(&metrics);

                                             float designUnitsPerEm = (float)metrics.designUnitsPerEm;
                                             float _height = glyphRun->fontEmSize * metrics.ascent / designUnitsPerEm;
                                             float _width = (glyphRun->glyphAdvances ? glyphRun->glyphAdvances[i] : 0.0f);

                                             if (pWndInfo->word_width < _width)
                                                 pWndInfo->word_width = _width;      // 竖屏用宽度
                                             if (pWndInfo->word_height < _height)
                                                 pWndInfo->word_height = _height;    // 横屏用高度

                                             wchar_t ch = glyphRunDescription && glyphRunDescription->string ? glyphRunDescription->string[i] : L'\0';

                                             pWndInfo->nLineDefWidth += _width;      // 记录总宽度, 横屏用
                                             pWndInfo->nLineDefHeight += _height;    // 记录总高度, 竖屏用

                                         }
                                         return S_OK;
                                     });
        pTextLayout->Draw(nullptr, &renderer, 0, 0);

    }

    return hFont != nullptr;
}

bool LYRIC_DESKTOP_DX::re_create_image(LYRIC_DESKTOP_INFO* pWndInfo)
{
    SafeRelease(image);
    lyric_wnd_load_image(*pWndInfo);
    return false;
}

bool LYRIC_DESKTOP_DX::destroy(bool isDestroyFont)
{
    // 除了字体, 剩下的都是设备相关的
    if (isDestroyFont)
        SafeRelease(hFont);

    SafeRelease(pBitmapBack);

    SafeRelease(hbrBorder);
    SafeRelease(hbrWndBorder);
    SafeRelease(hbrWndBack);
    SafeRelease(hbrLine);
    SafeRelease(hbrNormal);
    SafeRelease(hbrLight);
    SafeRelease(image);
    SafeRelease(image_shadow);
    SafeRelease(pGDIInterop);
    SafeRelease(hCanvas);
    return true;
}


NAMESPACE_LYRIC_DESKTOP_END