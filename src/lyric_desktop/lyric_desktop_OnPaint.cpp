#include "lyric_wnd_function.h"
#include <atlbase.h>

using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN

HRESULT DrawShadowRect(
    LYRIC_DESKTOP_INFO& wnd_info,
    const RECT_F& rect,
    D2D1_COLOR_F shadowColor,
    ID2D1Bitmap** ppBitmapRet
);
void lyric_wnd_fill_background(LYRIC_DESKTOP_INFO& wnd_info);



bool _canvas_drawimagegridPadding(D2DRender* d2dRender, D2DImage* img,
                                  float dstLeft, float dstTop, float dstRight, float dstBottom,
                                  float srcLeft, float srcTop, float srcRight, float srcBottom,
                                  float gridPaddingLeft, float gridPaddingTop, float gridPaddingRight, float gridPaddingBottom,
                                  BYTE alpha);
int UpdateLayered(HWND hWnd, int width, int height, HDC srcDC, PPOINT ppt = 0, int alpha = 255);


HRESULT lyric_wnd_OnPaint(LYRIC_DESKTOP_INFO& wnd_info, bool isresize, LYRIC_CALC_STRUCT& arg)
{
    D2DRender& hCanvas = *wnd_info.dx.hCanvas;
    ID2D1DeviceContext* pRenderTarget = hCanvas.GetD2DContext();
    RECT& rcWindow = wnd_info.rcWindow;
    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;

    //wnd_info.isFillBack = true;

    if (isresize)
        hCanvas.Resize(cxClient, cyClient);

    pRenderTarget->BeginDraw();

    lyric_wnd_fill_background(wnd_info);


    //auto oldAntialiasMode = hCanvas->GetAntialiasMode();
    //auto oldTextAntialiasMode = hCanvas->GetTextAntialiasMode();
    //hCanvas->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    //hCanvas->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    if (!wnd_info.isLock)   // 没锁定的时候才绘画按钮
        lyric_wnd_draw_button(wnd_info);

    lyric_wnd_draw_lyric(wnd_info, arg);

    HRESULT hr_gdi = E_INVALIDARG;
    HDC hdcD2D = nullptr;

    hr_gdi = wnd_info.dx.pGDIInterop->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hdcD2D);
    UpdateLayered(wnd_info.hWnd, cxClient, cyClient, hdcD2D);
    wnd_info.dx.pGDIInterop->ReleaseDC(nullptr);
    hdcD2D = nullptr;

    //hCanvas->SetAntialiasMode(oldAntialiasMode);
    //hCanvas->SetTextAntialiasMode(oldTextAntialiasMode);

    HRESULT hr = pRenderTarget->EndDraw();
    if (FAILED(hr))
    {
        // 这里需要清除对象, 然后在绘画前重新创建, 设备无效了
        wnd_info.dx.destroy(false);  // 销毁, 然后绘画前会判断有没有创建
    }

    if (FAILED(hr_gdi))
    {
        hdcD2D = hCanvas.GetDC();
        UpdateLayered(wnd_info.hWnd, cxClient, cyClient, hdcD2D);
        hCanvas.ReleaseDC(hdcD2D);
    }
    return hr;
}

HRESULT DrawShadowRect2(
    LYRIC_DESKTOP_INFO& wnd_info,
    RECT_F& rect,
    D2D1_COLOR_F shadowColor
)
{
    D2DRender& hCanvas = *wnd_info.dx.hCanvas;
    ID2D1DeviceContext* pRenderTarget = hCanvas.GetD2DContext();
    HRESULT hr = S_OK;

    //// 取消抗锯齿
    //auto oldAntialiasMode = hCanvas->GetAntialiasMode();
    //auto oldTextAntialiasMode = hCanvas->GetTextAntialiasMode();
    //pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
    //pRenderTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);


    // 把图片四个边画出来
    _canvas_drawimagegridPadding(wnd_info.dx.hCanvas, wnd_info.dx.image_shadow,
                                 0, 0, rect.width(), rect.height(), // 画到哪个位置上
                                 0, 0, 40, 40,                      // 从图片的哪个位置
                                 16, 16, 20, 20,                    // 九宫区域
                                 255);

    RECT_F rc;
    rc.left = rect.left + 10.f;
    rc.top = rect.top + 10.f;
    rc.right = rect.right - 10.f;
    rc.bottom = rect.bottom - 10.f;
    //CD2DBrush brush(hCanvas, MAKEARGB(180,255,0,0));
    //pRenderTarget->FillRectangle(rc, brush);
    pRenderTarget->FillRectangle(rc, wnd_info.dx.hbrWndBack->GetBrush());
    //pRenderTarget->DrawRectangle(rc, *wnd_info.dx.hbrWndBorder);


    //hCanvas->SetAntialiasMode(oldAntialiasMode);
    //hCanvas->SetTextAntialiasMode(oldTextAntialiasMode);

    return S_OK;
}

HRESULT DrawShadowRect(
    LYRIC_DESKTOP_INFO& wnd_info,
    const RECT_F& rect,
    D2D1_COLOR_F shadowColor,
    ID2D1Bitmap** ppBitmapRet
)
{
    if (*ppBitmapRet)
        SafeRelease(*ppBitmapRet);
    D2DRender& hCanvas = *wnd_info.dx.hCanvas;
    ID2D1DeviceContext* pRenderTarget = hCanvas.GetD2DContext();
    HRESULT hr = S_OK;

    const float shadowRadius = wnd_info.shadowRadius; // 阴影向外扩散半径

    // 阴影区域
    D2D1_RECT_F shadowRect = {
        rect.left + shadowRadius,
        rect.top + shadowRadius,
        rect.right,
        rect.bottom
    };

    // 创建一个临时目标位图，用于绘制阴影形状
    D2D1_SIZE_F bitmapSize = {
        shadowRect.right - shadowRect.left,
        shadowRect.bottom - shadowRect.top
    };

    CComPtr<ID2D1Bitmap1> pBitmapRet = nullptr;
    D2D1_BITMAP_PROPERTIES1 d2dbp = {};
    d2dbp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    d2dbp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    d2dbp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
    d2dbp.dpiX = 96;
    d2dbp.dpiY = 96;

    D2D1_SIZE_U size = { (UINT32)rect.width(), (UINT32)rect.height() };
    hr = pRenderTarget->CreateBitmap(size, nullptr, 0, &d2dbp, &pBitmapRet);
    if (FAILED(hr))
        return hr;

    // 创建临时设备上下文用于绘制阴影形状
    CComPtr<ID2D1BitmapRenderTarget> shadowDC = nullptr;
    hr = pRenderTarget->CreateCompatibleRenderTarget(bitmapSize, &shadowDC);
    if (FAILED(hr))
        return hr;

    shadowDC->BeginDraw();
    shadowDC->Clear(D2D1::ColorF(0, 0)); // 全透明背景

    // 创建填充笔刷
    CComPtr<ID2D1SolidColorBrush> shadowBrush;
    hr = shadowDC->CreateSolidColorBrush(shadowColor, &shadowBrush);
    if (FAILED(hr) || !shadowBrush)
        return hr;

    // 在中心绘制实心矩形（即原始的目标矩形减去偏移）
    D2D1_RECT_F offsetRect = {
        shadowRadius,
        shadowRadius,
        shadowRadius + rect.width(),
        shadowRadius + rect.height()
    };

    shadowDC->FillRectangle(offsetRect, shadowBrush);
    //shadowDC->DrawRectangle(offsetRect, shadowBrush, 2.0f);

    hr = shadowDC->EndDraw();
    if (FAILED(hr))
        return hr;

    CComPtr<ID2D1Bitmap> shadowBitmap = nullptr;
    hr = shadowDC->GetBitmap(&shadowBitmap);

    // 创建高斯模糊效果
    CComPtr<ID2D1Effect> blurEffect;
    hr = pRenderTarget->CreateEffect(CLSID_D2D1Shadow, &blurEffect);
    if (FAILED(hr) || !blurEffect)
        return hr;

    blurEffect->SetInput(0, shadowBitmap);
    blurEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_OPTIMIZATION, shadowRadius); // 模糊半径

    CComPtr<ID2D1Image> pOldTarget = nullptr;
    pRenderTarget->GetTarget(&pOldTarget);
    pRenderTarget->SetTarget(pBitmapRet);
    pRenderTarget->Clear();
    // 绘制阴影到主设备上下文
    D2D1_POINT_2F destPoint = { rect.left, rect.top };
    pRenderTarget->DrawImage(blurEffect, destPoint);
    //pRenderTarget->DrawImage(shadowBitmap, destPoint);

    RECT_F rc;
    rc.left = rect.left + shadowRadius;
    rc.top = rect.top + shadowRadius;
    rc.right = rect.right - shadowRadius;
    rc.bottom = rect.bottom - shadowRadius;

    //pRenderTarget->PushAxisAlignedClip(rc, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    //pRenderTarget->Clear();
    //pRenderTarget->PopAxisAlignedClip();
    //pRenderTarget->FillRectangle(rc, *wnd_info.dx.hbrWndBack);
    //pRenderTarget->DrawRectangle(rc, *wnd_info.dx.hbrWndBorder, 1.0f);

    pRenderTarget->SetTarget(pOldTarget);

    *ppBitmapRet = pBitmapRet;
    pBitmapRet.Detach();
    return S_OK;
}


// 填充背景, 只填充背景颜色和边框
void lyric_wnd_fill_background(LYRIC_DESKTOP_INFO& wnd_info)
{
    D2DRender& hCanvas = *wnd_info.dx.hCanvas;
    RECT& rcWindow = wnd_info.rcWindow;
    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;
    HRESULT hr = S_OK;
    bool isCreate = !wnd_info.dx.pBitmapBack;
    if (wnd_info.dx.pBitmapBack)
    {
        D2D1_SIZE_F si = wnd_info.dx.pBitmapBack->GetSize();
        if ((int)si.width != cxClient || (int)si.height != cyClient)
            isCreate = true;    // 尺寸不同, 需要创建
    }

    ID2D1DeviceContext* pRenderTarget = hCanvas.GetD2DContext();

    pRenderTarget->Clear();   // 不管什么操作, 都需要先清除画布
    if (!wnd_info.isFillBack || wnd_info.isLock)
        return; // 不填充背景, 或者锁定, 就返回
    RECT_F rc(0, 0, (float)cxClient, (float)cyClient);

    DrawShadowRect2(wnd_info, rc, D2D1::ColorF(D2D1::ColorF::Black, 1.f));
    return;

    // wnd_info.dx.pBitmapBack 为空, 或者 wnd_info.dx.pBitmapBack的尺寸和窗口尺寸不同
    // 那就需要创建缓存位图
    if (isCreate)
    {
        hr = DrawShadowRect(wnd_info,
                            rc,
                            D2D1::ColorF(D2D1::ColorF::Black, 1.f),
                            &wnd_info.dx.pBitmapBack
        );
    }

    // 判断缓存位图是否创建成功, 如果成功, 就把缓存位图画到窗口上
    if (wnd_info.dx.pBitmapBack)
    {
        // 从位图的这个位置拿出来绘画, 就是拿整个位图数据
        D2D1_SIZE_F si = wnd_info.dx.pBitmapBack->GetSize();
        D2D1_RECT_F rcSrc = { 0.f, 0.f, si.width, si.height };

        D2D1_RECT_F rcDst = { 0 };
        rcDst.left = 0;
        rcDst.top = 0;
        rcDst.right = rcDst.left + si.width;
        rcDst.bottom = rcDst.top + si.height;

        pRenderTarget->DrawBitmap(wnd_info.dx.pBitmapBack, rcDst, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rcSrc);
    }
    else
    {
        // 走到这就是创建失败了, 就用默认颜色填充
        pRenderTarget->Clear(ARGB_D2D(wnd_info.dx.clrBack));
    }

}



bool _canvas_DrawImage(ID2D1DeviceContext* pRenderTarget, ID2D1Bitmap1* image,
               float dstLeft, float dstTop, float dstRight, float dstBottom,
               float srcLeft, float srcTop, float srcRight, float srcBottom, BYTE alpha)
{
    if (!image || !pRenderTarget) return false;
    HRESULT hr = S_OK;

    D2D1_RECT_F rcDst{}, rcSrc{};
    rcDst.left = dstLeft;
    rcDst.top = dstTop;
    rcDst.right = dstRight;
    rcDst.bottom = dstBottom;
    if (srcRight == 0)srcRight = dstRight - dstLeft;
    if (srcBottom == 0)srcBottom = dstBottom - dstTop;
    rcSrc.left = srcLeft;
    rcSrc.top = srcTop;
    rcSrc.right = srcRight;
    rcSrc.bottom = srcBottom;

    pRenderTarget->DrawBitmap(
        image,
        &rcDst,
        static_cast<float>(alpha) / 255.0f,
        D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        &rcSrc
    );
    return true;

}

bool _canvas_DrawImage(ID2D1DeviceContext* pRenderTarget, ID2D1Bitmap1* image,
                       float dstLeft, float dstTop, float dstRight, float dstBottom,
                       float srcLeft, float srcTop, float srcRight, float srcBottom,
                       float gridPaddingLeft, float gridPaddingTop, float gridPaddingRight, float gridPaddingBottom,
                       BYTE alpha)
{
    if (!image || alpha == 0)
        return false;
    float pl = gridPaddingLeft;
    float pt = gridPaddingTop;
    float pr = gridPaddingRight;
    float pb = gridPaddingBottom;


    // 右-中间
    bool ret = _canvas_DrawImage(pRenderTarget, image,
                                 dstRight - pr, dstTop + pt, dstRight, dstBottom - pb,
                                 srcRight - gridPaddingRight, srcTop + gridPaddingTop, srcRight, srcBottom - gridPaddingBottom,
                                 alpha);


    // 右下
    ret = _canvas_DrawImage(pRenderTarget, image,
                            dstRight - pr, dstBottom - pb, dstRight, dstBottom, srcRight - gridPaddingRight,
                            srcBottom - gridPaddingBottom, srcRight, srcBottom,
                            alpha);

    // 底-中间
    ret = _canvas_DrawImage(pRenderTarget, image,
                            dstLeft + pl, dstBottom - pb, dstRight - pr, dstBottom,
                            srcLeft + gridPaddingLeft, srcBottom - gridPaddingBottom, srcRight - gridPaddingRight, srcBottom,
                            alpha);

    // 左下
    ret = _canvas_DrawImage(pRenderTarget, image,
                            dstLeft, dstBottom - pb, dstLeft + pl, dstBottom,
                            srcLeft, srcBottom - gridPaddingBottom, srcLeft + gridPaddingLeft, srcBottom,
                            alpha);
    // 左-中间
    ret = _canvas_DrawImage(pRenderTarget, image,
                            dstLeft, dstTop + pt, dstLeft + pl, dstBottom - pb,
                            srcLeft, srcTop + gridPaddingTop, srcLeft + gridPaddingLeft, srcBottom - gridPaddingBottom,
                            alpha);

    // 左上
    ret = _canvas_DrawImage(pRenderTarget, image,
                            dstLeft, dstTop, dstLeft + pl, dstTop + pt,
                            srcLeft, srcTop, srcLeft + gridPaddingLeft, srcTop + gridPaddingTop,
                            alpha);

    // 右上
    ret = _canvas_DrawImage(pRenderTarget, image,
                            dstRight - pr, dstTop, dstRight, dstTop + pt,
                            srcRight - gridPaddingRight, srcTop, srcRight, srcTop + gridPaddingTop,
                            alpha);

    // 顶-中间
    ret = _canvas_DrawImage(pRenderTarget, image,
                            dstLeft + pl, dstTop, dstRight - pr, dstTop + pt,
                            srcLeft + gridPaddingLeft, srcTop, srcRight - gridPaddingRight, srcTop + gridPaddingTop,
                            alpha);
    return ret;
}

// 修改位图颜色
// crNew = 新的颜色
// lpBits = 位图数据
// nCount = 像素数, 宽 * 高
inline static void  PixelFix(COLORREF crNew, LPBYTE lpBits, int nCount)
{
    BYTE sR = ARGB_GETR(crNew);
    BYTE sG = ARGB_GETG(crNew);
    BYTE sB = ARGB_GETB(crNew);

    DWORD* cr = (DWORD*)lpBits;     // 转成4字节一个像素点来处理
    for (int i = 0; i < nCount; ++i)
    {
        if (cr[i] == 0) continue;   // 等于0 表示是透明, 透明的不处理
        LPBYTE p = (LPBYTE)(&cr[i]);
        p[2] = sR * p[3] / 255;
        p[1] = sG * p[3] / 255;
        p[0] = sB * p[3] / 255;

        //cr[i] = MAKEARGB(255, 255, 0, 0);
    }
}

bool _canvas_drawimagegridPadding(D2DRender* d2dRender, D2DImage* img,
                                  float dstLeft, float dstTop, float dstRight, float dstBottom,
                                  float srcLeft, float srcTop, float srcRight, float srcBottom,
                                  float gridPaddingLeft, float gridPaddingTop, float gridPaddingRight, float gridPaddingBottom,
                                  BYTE alpha)
{
    ID2D1DeviceContext* pRenderTarget = d2dRender->GetD2DContext();
    if (!pRenderTarget || alpha == 0)
        return false;

    CComPtr<ID2D1Bitmap1> image;
    img->GetBitmap(0, &image, nullptr);
    bool ret = _canvas_DrawImage(pRenderTarget, image,
                                 dstLeft, dstTop, dstRight, dstBottom,
                                 srcLeft, srcTop, srcRight, srcBottom,
                                 gridPaddingLeft, gridPaddingTop, gridPaddingRight, gridPaddingBottom,
                                 alpha);

    return ret;
}


// 更新分层窗口, 返回错误码, 0=成功
// hWnd = 要重画的窗口
// width = 窗口宽度
// height = 窗口高度
// srcDC = 从哪个dc拷贝到窗口上
// ppt = 窗口新的位置
// alpha = 窗口透明度
int UpdateLayered(HWND hWnd, int width, int height, HDC srcDC, PPOINT ppt, int alpha)
{
    int ret = 0;
    HDC hdc = ::GetDC(hWnd);
    SIZE size = { width, height };
    POINT pt = { 0 };
    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.AlphaFormat = AC_SRC_ALPHA;
    blend.SourceConstantAlpha = (BYTE)alpha;
    BOOL b = UpdateLayeredWindow(hWnd, hdc, ppt, &size, srcDC, &pt, 0, &blend, ULW_ALPHA);
    if (!b)
        ret = GetLastError();
    ReleaseDC(hWnd, hdc);
    return ret;
}

NAMESPACE_LYRIC_DESKTOP_END