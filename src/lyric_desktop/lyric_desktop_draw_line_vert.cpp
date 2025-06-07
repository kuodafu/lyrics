#include "lyric_wnd_function.h"

using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN


// 绘画竖屏歌词文本
void lyric_wnd_draw_cache_text_v(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, ID2D1Bitmap* pBitmap)
{
    if (!pBitmap)
        return;

    D2DRender& hCanvas = *wnd_info.dx.hCanvas;
    ID2D1LinearGradientBrush* hbrNormal = wnd_info.dx.hbrNormal->GetNative();
    ID2D1LinearGradientBrush* hbrLight = wnd_info.dx.hbrLight->GetNative();
    ID2D1DeviceContext* pRenderTarget = hCanvas.GetD2DContext();

    LYRIC_LINE_STRUCT& line = draw_info.line;

    RECT& rcWindow = wnd_info.rcWindow;

    const float _offset = wnd_info.padding_text;
    //const float _offset = (draw_info.layout_text_max_height - wnd_info.nLineHeight) / 2;
    const int shadowRadius = (int)wnd_info.shadowRadius;    // 阴影向外扩散半径

    // 如果歌词宽度超过这个, 那就要在歌词高亮显示到窗口一半的时候调整顶边位置
    const int cyScreen = rcWindow.bottom - rcWindow.top;        // 这个是整个窗口的高度
    const int cyBound = cyScreen - shadowRadius * 2;            // 这个是窗口可以显示的高度
    const float lrcHeight = ((float)cyBound) - _offset * 2;     // 这个是歌词文本的最大高度, 窗口高度减去左右阴影, 减去歌词绘画左右偏移
    const float _p60 = lrcHeight * 0.6f;
    const D2D1_RECT_F& rcText = draw_info.rcText;

    // 最小的顶边距离
    const float min_left = cyBound - draw_info.text_height - rcText.top + _offset / 2;


    // 从位图的这个位置拿出来绘画, 就是拿整个位图数据
    D2D1_SIZE_F si = pBitmap->GetSize();
    D2D1_RECT_F rcSrc = { 0.f, 0.f, si.width, si.height };

    float text_top = rcText.top;
    if (si.height > cyBound)
    {
        // 歌词宽度超过窗口宽度, 判断高亮位置是否大于窗口一半, 如果大于, 就调整左边位置
        // 还要计算右边能显示多少
        if (draw_info.nLightHeight > _p60)
        {
            // 高亮位置 减去 高度百分比, 得到百分比 到 高亮位置的距离
            // 左边位置需要减去这个距离
            text_top -= (draw_info.nLightHeight - _p60);
            text_top = max(min_left, text_top);   // 设定一个最小值, 小于这个最小值就设为最小值
        }
    }

    D2D1_RECT_F rcBound = { wnd_info.shadowRadius, wnd_info.shadowRadius, 0, 0 };
    rcBound.right = ((float)(rcWindow.right - rcWindow.left)) - wnd_info.shadowRadius;
    rcBound.bottom = ((float)(rcWindow.bottom - rcWindow.top)) - wnd_info.shadowRadius;
    pRenderTarget->PushAxisAlignedClip(rcBound, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    D2D1_RECT_F rcDst = { 0 };
    rcDst.left = rcText.left;
    rcDst.top = text_top;
    rcDst.right = rcDst.left + si.width;
    rcDst.bottom = rcDst.top + si.height;
    bool isLigth = draw_info.cache.pBitmapLight == pBitmap;

    bool isClip = false;

    if (draw_info.nLightHeight > 0.f && draw_info.nLightHeight < draw_info.text_height)
    {
        D2D1_RECT_F rcRgn = rcDst;
        float light_bottom = rcRgn.top + draw_info.nLightHeight + _offset;
        if (isLigth)
        {
            // 高亮文本, 限制右边不能显示, 右边显示范围就是高亮的宽度
            rcRgn.bottom = light_bottom;
        }
        else
        {
            // 普通文本, 限制左边不能显示
            rcRgn.top = light_bottom;
        }
        isClip = true;
        pRenderTarget->PushAxisAlignedClip(rcRgn, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
    }

    pRenderTarget->DrawBitmap(pBitmap, rcDst, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rcSrc);
    if (isClip)
        pRenderTarget->PopAxisAlignedClip();

    pRenderTarget->PopAxisAlignedClip();

}

NAMESPACE_LYRIC_DESKTOP_END