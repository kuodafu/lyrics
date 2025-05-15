#include "lyric_wnd_function.h"
#include <control/CControlDraw.h>

using namespace NAMESPACE_D2D;

NAMESPACE_LYRIC_WND_BEGIN

HRESULT DrawShadowRect(
    LYRIC_WND_INFU& wnd_info,
    const RECT_F& rect,
    D2D1_COLOR_F shadowColor,
    ID2D1Bitmap** ppBitmapRet
);
void lyric_wnd_fill_background(LYRIC_WND_INFU& wnd_info);


void lyric_wnd_get_draw_text_info(LYRIC_WND_INFU& wnd_info,
                                  LYRIC_WND_DRAWTEXT_INFO& draw_info,
                                  const RECT& rcWindow, int nLine,
                                  float nLightWidth);


HRESULT lyric_wnd_OnPaint(LYRIC_WND_INFU& wnd_info, bool isresize, LYRIC_CALC_STRUCT& arg)
{
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    RECT& rcWindow = wnd_info.rcWindow;
    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;


    if (isresize)
        hCanvas.resize(cxClient, cyClient);

    hCanvas->BeginDraw();

    lyric_wnd_fill_background(wnd_info);



    auto oldAntialiasMode = hCanvas->GetAntialiasMode();
    auto oldTextAntialiasMode = hCanvas->GetTextAntialiasMode();
    hCanvas->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    hCanvas->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);

    if (!wnd_info.isLock)   // 没锁定的时候才绘画按钮
        lyric_wnd_draw_button(wnd_info, rcWindow);

    float nLightWidth = arg.word.nLeft + arg.nWidthWord;
    int nIndexLine1 = -1, nIndexLine2 = -1;


    // 是否切换下一行, 当前位置超过歌词的30%后就切换到下一行
    bool isSwitchLine = nLightWidth > arg.line.nWidth * 0.3;

    LYRIC_WND_DRAWTEXT_INFO* pLine1 = nullptr, * pLine2 = nullptr;
    if (arg.indexLine % 2 == 0)
    {
        pLine1 = &wnd_info.line1;
        pLine2 = &wnd_info.line2;

        nIndexLine1 = arg.indexLine;
        nIndexLine2 = wnd_info.line2.cache.preIndex;
        if (nIndexLine2 == -1)
            nIndexLine2 = nIndexLine1 + 1;
    }
    else
    {
        pLine1 = &wnd_info.line2;
        pLine2 = &wnd_info.line1;

        nIndexLine1 = wnd_info.line1.cache.preIndex;
        nIndexLine2 = arg.indexLine;
        if (nIndexLine1 == -1)
            nIndexLine1 = nIndexLine2 + 1;
    }

    pLine1->line = arg.line;
    pLine1->nLightWidth = nLightWidth;  // 当前行的高亮位置


    // 如果需要换行, 那就获取下一行歌词信息保存到 pLine2, pLine2 高亮为0
    if (isSwitchLine || arg.indexLine == 0)
    {
        if (arg.indexLine + 1 < arg.nLineCount)
            lyric_get_line(wnd_info.hLyric, arg.indexLine + 1, &pLine2->line);
        pLine2->nLightWidth = 0;
    }
    else if (!isSwitchLine)
    {
        // 还不切换下一行, 另一行的歌词高亮是100%, 另一行歌词是当前行是上一行
        lyric_get_line(wnd_info.hLyric, arg.indexLine - 1, &pLine2->line);
        pLine2->nLightWidth = pLine2->line.nWidth;
    }

    lyric_wnd_get_draw_text_info(wnd_info, wnd_info.line1, rcWindow, 1, wnd_info.line1.nLightWidth);
    lyric_wnd_get_draw_text_info(wnd_info, wnd_info.line2, rcWindow, 2, wnd_info.line2.nLightWidth);

    // 如果有重画操作, 歌词文本肯定要绘画
    lyric_wnd_draw_line(wnd_info, wnd_info.line1, nIndexLine1);
    lyric_wnd_draw_line(wnd_info, wnd_info.line2, nIndexLine2);


    if (!wnd_info.hLyric)
        arg.line.pText = wnd_info.pszDefText, arg.line.nLength = wnd_info.nDefText;

    ID2D1GdiInteropRenderTarget* pGdiInterop = hCanvas;
    HDC hdcD2D = nullptr;
    pGdiInterop->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hdcD2D);
    UpdateLayered(wnd_info.hWnd, cxClient, cyClient, hdcD2D);
    pGdiInterop->ReleaseDC(0);

    hCanvas->SetAntialiasMode(oldAntialiasMode);
    hCanvas->SetTextAntialiasMode(oldTextAntialiasMode);

    HRESULT hr = hCanvas->EndDraw();
    if (FAILED(hr))
    {
        // 这里需要清除对象, 然后在绘画前重新创建, 设备无效了
        wnd_info.dx.destroy(false);  // 销毁, 然后绘画前会判断有没有创建
    }

    return hr;
}

HRESULT DrawShadowRect(
    LYRIC_WND_INFU& wnd_info,
    const RECT_F& rect,
    D2D1_COLOR_F shadowColor,
    ID2D1Bitmap** ppBitmapRet
)
{
    if (*ppBitmapRet)
        SafeRelease(*ppBitmapRet);
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    ID2D1DeviceContext* pRenderTarget = hCanvas;
    HRESULT hr = S_OK;

    auto& d2dInfo = d2d_get_info();
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

    pRenderTarget->PushAxisAlignedClip(rc, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    pRenderTarget->Clear();
    pRenderTarget->PopAxisAlignedClip();
    pRenderTarget->FillRectangle(rc, *wnd_info.dx.hbrWndBack);
    //pRenderTarget->DrawRectangle(rc, *wnd_info.dx.hbrWndBorder, 1.0f);

    pRenderTarget->SetTarget(pOldTarget);

    *ppBitmapRet = pBitmapRet;
    pBitmapRet.Detach();
    return S_OK;
}


// 填充背景, 只填充背景颜色和边框
void lyric_wnd_fill_background(LYRIC_WND_INFU& wnd_info)
{
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
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

    ID2D1DeviceContext* pRenderTarget = hCanvas;

    hCanvas->Clear();   // 不管什么操作, 都需要先清除画布
    if (!wnd_info.isFillBack || wnd_info.isLock)
        return; // 不填充背景, 或者锁定, 就返回
    RECT_F rc(0, 0, (float)cxClient, (float)cyClient);

    // wnd_info.dx.pBitmapBack 为空, 或者 wnd_info.dx.pBitmapBack的尺寸和窗口尺寸不同
    // 那就需要创建缓存位图
    if (isCreate)
    {
        SafeRelease(wnd_info.dx.pBitmapBack);
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
        hCanvas->Clear(ARGB_D2D(wnd_info.dx.clrBack));
    }

}






void lyric_wnd_get_draw_text_info(LYRIC_WND_INFU& wnd_info,
                                  LYRIC_WND_DRAWTEXT_INFO& draw_info,
                                  const RECT& rcWindow, int nLine,
                                  float nLightWidth)
{
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    auto& d2dInfo = d2d_get_info();
    CD2DFont& font = *wnd_info.dx.hFont;


    auto& line = draw_info.line;
    //draw_info.cache = nLine == 1 ? &wnd_info.cache1 : &wnd_info.cache2;

    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;

    const int nLeft = wnd_info.scale(16);
    float height = (float)wnd_info.nLineHeight + wnd_info.scale(10);
    float top = (float)(nLine == 1 ? wnd_info.nLineTop1 : wnd_info.nLineTop2);
    float left = (float)nLeft;  // 默认左对齐
    if (draw_info.align == 1)
    {
        // 居中对齐
        int nTemp = (cxClient - nLeft * 2 - (int)line.nWidth) / 2;
        left = (float)max(nLeft, nTemp);
    }
    else if (draw_info.align == 2)
    {
        // 右对齐
        int nTemp = cxClient - nLeft * 2 - (int)line.nWidth;
        left = (float)max(nLeft, nTemp);
    }

    draw_info.nLightWidth = nLightWidth;
    draw_info.rcText = { left, top, left + line.nWidth, top + height };
    draw_info.layout_text_max_width = (float)(cxClient - nLeft * 2);
    draw_info.layout_text_max_height = (float)(draw_info.rcText.bottom - draw_info.rcText.top);

}



NAMESPACE_LYRIC_WND_END