#include "lyric_wnd_function.h"
#include <d2d/CCustomTextRenderer.h>
#include "GetMonitorRect.h"
#include <atlbase.h>

using namespace KUODAFU_NAMESPACE;


NAMESPACE_LYRIC_DESKTOP_BEGIN


bool LYRIC_DESKTOP_DX::re_create(LYRIC_DESKTOP_INFO* pWndInfo)
{
    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    auto& clrNormal = wnd_info.config.clrNormal;
    auto& clrLight = wnd_info.config.clrLight;
    RECT rc;
    GetClientRect(wnd_info.hWnd, &rc);
    const int cxClient = rc.right - rc.left;
    const int cyClient = rc.bottom - rc.top;

    SafeRelease(pGDIInterop);
    SafeRelease(pRender);
    g_d2d_interface->CreateD2DRender(cxClient, cyClient, true, &pRender);
    pRender->GetD2DContext()->QueryInterface(&pGDIInterop);

    SafeRelease(image_shadow);

    size_t imgSize = 0;
    LPBYTE pImageData = _lrc_dwsktop_get_shadow_image(imgSize);


    pRender->CreateImage(pImageData, imgSize, &image_shadow);

    if (!hFont)
        re_create_font(pWndInfo);

    re_create_brush(pWndInfo, true);
    re_create_brush(pWndInfo, false);
    re_create_border(pWndInfo);
    re_create_image(pWndInfo);

    SafeRelease(hbrWndBorder);
    SafeRelease(hbrWndBack);
    SafeRelease(hbrLine);

    pRender->CreateSolidBrush(pWndInfo->config.clrWndBorder, &hbrWndBorder);
    pRender->CreateSolidBrush(pWndInfo->config.clrWndBack, &hbrWndBack);
    pRender->CreateSolidBrush(MAKEARGB(100, 255, 255, 255), &hbrLine);

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
        pClr = &wnd_info.config.clrLight[0];
        size = (int)wnd_info.config.clrLight.size();
        ppBrush = &hbrLight;
    }
    else
    {
        SafeRelease(hbrNormal);
        pClr = &wnd_info.config.clrNormal[0];
        size = (int)wnd_info.config.clrNormal.size();
        ppBrush = &hbrNormal;
    }

    POINT_F pt1 = { 0.f, 0.f };
    POINT_F pt2 = { 1.f, .1f };
    D2DBRUSH_CREATE_STRUCT arg{};
    arg.color = pClr;
    arg.colorCount = size;
    HRESULT hr = pRender->CreateLinearGradientBrush(&pt1, &pt2, &arg, ppBrush);
    return SUCCEEDED(hr);
}

bool LYRIC_DESKTOP_DX::re_create_border(LYRIC_DESKTOP_INFO* pWndInfo)
{
    SafeRelease(hbrBorder);
    HRESULT hr = pRender->CreateSolidBrush(pWndInfo->config.clrBorder, &hbrBorder);
    return SUCCEEDED(hr);
}

bool LYRIC_DESKTOP_DX::re_create_font(LYRIC_DESKTOP_INFO* pWndInfo)
{
    SafeRelease(hFont);

    // 不会多线程进来, 所以放心使用/修改静态变量
    static LOGFONT lf;
    if (lf.lfHeight == 0)
        SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(LOGFONTW), &lf, 0);
    
    lf.lfHeight = -MulDiv(pWndInfo->config.nFontSize, pWndInfo->scale, 72);
    lf.lfWeight = pWndInfo->config.lfWeight;
    wcscpy_s(lf.lfFaceName, pWndInfo->config.pszFontName.c_str());
    g_d2d_interface->CreateD2DFont(&lf, &hFont);

    CComPtr<IDWriteTextLayout> pTextLayout;
    lyric_wnd_create_text_layout(pWndInfo->config.pszDefText, pWndInfo->config.nDefText, hFont->GetDWTextFormat(), 0, 0, &pTextLayout);
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
    SafeRelease(pRender);
    return true;
}


NAMESPACE_LYRIC_DESKTOP_END