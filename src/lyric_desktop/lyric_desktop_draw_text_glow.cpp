#include "lyric_wnd_function.h"

using namespace KUODAFU_NAMESPACE;



NAMESPACE_LYRIC_DESKTOP_BEGIN

struct GLOW_ARG
{
    LYRIC_DESKTOP_INFO* pWndInfo;
    LYRIC_DESKTOP_DRAWTEXT_INFO* pDrawInfo;

    ID2D1DeviceContext* pRenderTarget;
    IDWriteTextLayout* pTextLayout;

    float offset;
};

// �滭��������, Ȼ�󱣴浽λͼ��, ��������, �´�ֱ�Ӵӻ�����ȡ�����滭
bool glow_create_cache(GLOW_ARG& glow_arg);
ID2D1Bitmap* glow_create_text_bitmap(GLOW_ARG& glow_arg, ID2D1LinearGradientBrush* hbrFill);
void glow_draw_effetc(GLOW_ARG& glow_arg, ID2D1LinearGradientBrush* hbrFill, ID2D1Bitmap* pBitmap);


void lyric_wnd_draw_text_glow(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, int nDrawLineIndex)
{
    GLOW_ARG glow_arg = { 0 };
    glow_arg.pWndInfo = &wnd_info;
    glow_arg.pDrawInfo = &draw_info;

    //glow_create_cache(glow_arg);

}
//
//// �滭�����ı�����
//bool glow_create_cache(GLOW_ARG& glow_arg)
//{
//    HRESULT hr = S_OK;
//    LYRIC_WND_INFO& wnd_info = *glow_arg.pWndInfo;
//    LYRIC_WND_DRAWTEXT_INFO& draw_info = *glow_arg.pDrawInfo;
//    LYRIC_LINE_STRUCT& line = draw_info.line;
//    IDWriteTextFormat* pTextFormat = *wnd_info.dx.hFont;
//
//    CD2DFont& font = *wnd_info.dx.hFont;
//    ID2D1LinearGradientBrush* hbrNormal = *wnd_info.dx.hbrNormal;
//    ID2D1LinearGradientBrush* hbrLight = *wnd_info.dx.hbrLight;
//    ID2D1SolidColorBrush* hbrBorder = *wnd_info.dx.hbrBorder;
//    IDWriteTextFormat* dxFormat = font;
//
//    auto& d2dInfo = d2d_get_info();
//
//    CComPtr<ID2D1DeviceContext> pRenderTarget;  // ����������ھ��ͷ�
//    CComPtr<IDWriteTextLayout> pTextLayout;     // ����������ھ��ͷ�
//
//    glow_arg.offset = (draw_info.layout_text_max_height - wnd_info.nLineHeight) / 2;
//
//    // �����ı�����, �滭�ı��ͻ滭�������
//    //hr = d2dInfo.pDWriteFactory->CreateTextLayout(
//    //    line.pText, (UINT32)line.nLength, draw_info.dxFormat,
//    //    draw_info.layout_text_max_width, draw_info.layout_text_max_height,
//    //    &pTextLayout
//    //);
//    pTextLayout = lyric_wnd_create_text_layout(line.pText, (UINT32)line.nLength, dxFormat,
//                                               draw_info.layout_text_max_width, draw_info.layout_text_max_height);
//    if (!pTextLayout)
//        return false;
//    hr = d2dInfo.pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pRenderTarget);
//    glow_arg.pRenderTarget = pRenderTarget;
//    glow_arg.pTextLayout = pTextLayout;
//
//    auto pfn_create_bitmap = [&](ID2D1LinearGradientBrush* hbrFill) -> ID2D1Bitmap1*
//    {
//        ID2D1Bitmap1* pBitmapRet = nullptr;
//        D2D1_BITMAP_PROPERTIES1 d2dbp = {};
//        d2dbp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
//        d2dbp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
//        d2dbp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
//        d2dbp.dpiX = 96;
//        d2dbp.dpiY = 96;
//
//        D2D1_SIZE_U size = { (UINT)draw_info.layout_text_max_width, (UINT)draw_info.layout_text_max_height };
//        hr = pRenderTarget->CreateBitmap(size, 0, 0, &d2dbp, &pBitmapRet);
//        if (FAILED(hr))
//            return 0;
//
//        float start_top_left = (draw_info.layout_text_max_width) / 2;
//        POINT_F startPoint = { start_top_left, glow_arg.offset };
//        POINT_F endPoint = { start_top_left, glow_arg.offset + draw_info.layout_text_max_height };
//        hbrFill->SetStartPoint(startPoint);
//        hbrFill->SetEndPoint(endPoint);
//
//
//        CComPtr<ID2D1Image> oldBitmap;
//        pRenderTarget->GetTarget(&oldBitmap);
//        pRenderTarget->SetTarget(pBitmapRet);
//        ID2D1Bitmap* pBitmap = glow_create_text_bitmap(glow_arg, hbrFill);
//        if (!pBitmap)
//        {
//            SafeRelease(pBitmapRet);
//            return nullptr;
//        }
//
//        pRenderTarget->BeginDraw();
//        pRenderTarget->Clear();
//        glow_draw_effetc(glow_arg, hbrFill, pBitmap);
//        pRenderTarget->EndDraw();
//        pRenderTarget->SetTarget(oldBitmap);
//        SafeRelease(pBitmap);
//        return pBitmapRet;
//    };
//
//    SafeRelease(draw_info.cache.pBitmapNormal);
//    SafeRelease(draw_info.cache.pBitmapLight);
//    draw_info.cache.pBitmapNormal = pfn_create_bitmap(hbrNormal);
//    draw_info.cache.pBitmapLight = pfn_create_bitmap(hbrLight);
//
//    return true;
//}
//
//ID2D1Bitmap* glow_create_text_bitmap(GLOW_ARG& glow_arg, ID2D1LinearGradientBrush* hbrFill)
//{
//    LYRIC_WND_INFO& wnd_info = *glow_arg.pWndInfo;
//    LYRIC_WND_DRAWTEXT_INFO& draw_info = *glow_arg.pDrawInfo;
//    ID2D1DeviceContext* pRenderTarget = glow_arg.pRenderTarget;
//
//
//    // Ū���µ���ȾĿ��, �����ݻ滭�����Ŀ����, Ȼ��ȡ��λͼ����
//    CComPtr<ID2D1BitmapRenderTarget> pRender = nullptr;
//    D2D1_SIZE_F size = D2D1::SizeF(draw_info.layout_text_max_width, draw_info.layout_text_max_height);
//    HRESULT hr = pRenderTarget->CreateCompatibleRenderTarget(size, &pRender);
//    if (FAILED(hr))
//        return nullptr;
//
//    pRender->BeginDraw();
//    pRender->Clear();
//
//    //TODO ��Ҫ���ý���λ��
//    D2D1_POINT_2F pt = { glow_arg.offset, glow_arg.offset };
//    pRender->DrawTextLayout(pt, glow_arg.pTextLayout, hbrFill, D2D1_DRAW_TEXT_OPTIONS_CLIP);
//
//    hr = pRender->EndDraw();
//    if (FAILED(hr))
//        return nullptr;
//
//    // �滭����, ȡ��λͼ����
//    ID2D1Bitmap* pBitmap = nullptr;
//    hr = pRender->GetBitmap(&pBitmap);
//    if (SUCCEEDED(hr))
//        return pBitmap;
//    return nullptr;
//}
//
//void glow_draw_effetc(GLOW_ARG& glow_arg, ID2D1LinearGradientBrush* hbrFill, ID2D1Bitmap* pBitmap)
//{
//    LYRIC_WND_INFO& wnd_info = *glow_arg.pWndInfo;
//    LYRIC_WND_DRAWTEXT_INFO& draw_info = *glow_arg.pDrawInfo;
//    LYRIC_LINE_STRUCT& line = draw_info.line;
//    auto& d2dInfo = d2d_get_info();
//    HRESULT hr = S_OK;
//    ID2D1DeviceContext* pRenderTarget = glow_arg.pRenderTarget;
//
//    D2D1_COLOR_F g_GlowColor = D2D1::ColorF(0.0f, 0.7f, 0.4f, 1.0f);
//    CComPtr<ID2D1Effect> pBlurEffect = nullptr;
//    hr = pRenderTarget->CreateEffect(CLSID_D2D1GaussianBlur, &pBlurEffect);
//    if (FAILED(hr))
//        return;
//
//    // ������ɫ����Ч��(����������ɫ)
//    CComPtr<ID2D1Effect> pColorMatrixEffect = nullptr;
//    hr = pRenderTarget->CreateEffect(CLSID_D2D1ColorMatrix, &pColorMatrixEffect);
//    if (FAILED(hr))
//        return;
//
//    // ���㷢������߽�
//    draw_info.cache.rcBounds = { 0 };
//    D2D1_RECT_F textBounds = { 0.f, 0.f, (float)line.nWidth, wnd_info.nLineHeight };
//
//
//    // 1. ���ƶ��ط����(ģ�����)
//    for (int i = 3; i >= 1; i--)
//    {
//        float radius = 3.0f * i;
//        float opacity = 0.8f / i;
//
//        // ����ģ��Ч��
//
//        pBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, radius);
//        pBlurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, D2D1_GAUSSIANBLUR_OPTIMIZATION_QUALITY);
//
//        // ��������
//        pBlurEffect->SetInput(0, pBitmap);
//
//
//        D2D1_MATRIX_5X4_F matrix = {
//            g_GlowColor.r, 0, 0, 0,  // R
//            0, g_GlowColor.g, 0, 0,   // G
//            0, 0, g_GlowColor.b, 0,   // B
//            0, 0, 0, opacity,         // Alpha
//            0, 0, 0, 0               // ƫ��
//        };
//        pColorMatrixEffect->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
//        pColorMatrixEffect->SetInputEffect(0, pBlurEffect);
//
//        if (draw_info.cache.rcBounds.left == 0)
//        {
//            draw_info.cache.rcBounds.left = textBounds.left - radius; // ��չ��߽�
//            draw_info.cache.rcBounds.top = textBounds.top - radius;   // ��չ�ϱ߽�
//            draw_info.cache.rcBounds.right = textBounds.right + radius; // ��չ�ұ߽�
//            draw_info.cache.rcBounds.bottom = textBounds.bottom + radius; // ��չ�±߽�
//        }
//
//        // ���Ʒ����
//        D2D1_POINT_2F glowPosition = { 0, 0 };
//        pRenderTarget->DrawImage(pColorMatrixEffect, glowPosition);
//
//    }
//
//    // �滭�������ı�, ����滭λ�þ��Ǽ����ʸ�����λ��
//    D2D1_POINT_2F pt = { glow_arg.offset, glow_arg.offset };
//    pRenderTarget->DrawTextLayout(pt, glow_arg.pTextLayout, hbrFill, D2D1_DRAW_TEXT_OPTIONS_CLIP);
//
//    //CD2DBrush hbrBak(MAKEARGB(180, 255, 0, 0));
//    //pRenderTarget->FillRectangle(glowBounds, hbrBak);
//
//}

NAMESPACE_LYRIC_DESKTOP_END