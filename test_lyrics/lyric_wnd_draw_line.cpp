#include "lyric_wnd_function.h"
#include <control/CControlDraw.h>

using namespace NAMESPACE_D2D;

NAMESPACE_LYRIC_WND_BEGIN

// 把缓存的文本绘画出来
void lyric_wnd_draw_cache_text(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info);

void lyric_wnd_draw_line(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& line_info, int nIndexLine)
{
    //TODO 这里可以做个判定选择哪种方式绘画歌词文本
    decltype(lyric_wnd_draw_text_geometry)* pfn_create_cache_bitmap = nullptr;
    if (1)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_geometry;
    else if (2)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_glow;

    auto& cache = line_info.cache;
    auto& line = line_info.line;
    // 先判断内容是否改变, 如果改变, 需要重新缓存
    if (cache.preIndex != nIndexLine
        || cache.preText != line.pText
        || cache.preLength != line.nLength
        || wnd_info.change_font || wnd_info.change_hbr  // 字体/画刷改变, 需要重新创建缓存
        )
    {
        // 上次记录的值和这次不一样了, 文本改变了, 重新绘画, 然后记录到位图里

        // 记录绘画的行好和文本
        cache.preIndex = nIndexLine;
        cache.preText = line.pText;
        cache.preLength = line.nLength;

        // 创建缓存位图
        pfn_create_cache_bitmap(wnd_info, line_info);

    }

    lyric_wnd_draw_cache_text(wnd_info, line_info);

}

void lyric_wnd_draw_cache_text(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info)
{
    // 从缓存里把数据拿出来画到目标上
    auto pfn_draw_bitmap = [&](ID2D1Bitmap* pBitmap)
    {
        if (!pBitmap)
            return;

        CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
        CD2DFont& font = *wnd_info.dx.hFont;
        ID2D1LinearGradientBrush* hbrNormal = *wnd_info.dx.hbrNormal;
        ID2D1LinearGradientBrush* hbrLight = *wnd_info.dx.hbrLight;
        ID2D1SolidColorBrush* hbrBorder = *wnd_info.dx.hbrBorder;
        IDWriteTextFormat* dxFormat = font;
        ID2D1DeviceContext* pRenderTarget = hCanvas;

        LYRIC_LINE_STRUCT& line = draw_info.line;
        const int _10 = wnd_info.scale(10);

        RECT& rcWindow = wnd_info.rcWindow;

        const float _offset = (draw_info.layout_text_max_height - wnd_info.nLineHeight) / 2;
        const int shadowRadius = (int)wnd_info.shadowRadius;    // 阴影向外扩散半径
        const float shadowRadiusF = wnd_info.shadowRadius;      // 阴影向外扩散半径

        // 如果歌词宽度超过这个, 那就要在歌词高亮显示到窗口一半的时候调整左边位置
        const int cxScreen = rcWindow.right - rcWindow.left;        // 这个是整个窗口的宽度
        const int cxBound = cxScreen - shadowRadius * 2;            // 这个是窗口可以显示的宽度
        const float lrcWidth = ((float)cxBound) - _offset * 2;      // 这个是歌词文本的最大宽度, 窗口宽度减去左右阴影, 减去歌词绘画左右偏移
        const float _p60 = lrcWidth * 0.6f;
        const D2D1_RECT_F& rcText = draw_info.rcText;

        // 最小的左边距离
        const float min_left = cxBound - line.nWidth - rcText.left + _offset / 2;


        // 从位图的这个位置拿出来绘画, 就是拿整个位图数据
        D2D1_SIZE_F si = pBitmap->GetSize();
        D2D1_RECT_F rcSrc = { 0.f, 0.f, si.width, si.height };

        float text_left = rcText.left;
        if (si.width > cxBound)
        {
            // 歌词宽度超过窗口宽度, 判断高亮位置是否大于窗口一半, 如果大于, 就调整左边位置
            // 还要计算右边能显示多少
            if (draw_info.nLightWidth > _p60)
            {
                // 高亮位置 减去 宽度百分比, 得到百分比 到 高亮位置的距离
                // 左边位置需要减去这个距离
                text_left -= (draw_info.nLightWidth - _p60);
                text_left = max(min_left, text_left);   // 设定一个最小值, 小于这个最小值就设为最小值
            }
        }

        D2D1_RECT_F rcBound = { wnd_info.shadowRadius, wnd_info.shadowRadius, 0, 0 };
        rcBound.right = ((float)(rcWindow.right - rcWindow.left)) - wnd_info.shadowRadius;
        rcBound.bottom = ((float)(rcWindow.bottom - rcWindow.top)) - wnd_info.shadowRadius;
        pRenderTarget->PushAxisAlignedClip(rcBound, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);


        D2D1_RECT_F rcDst = { 0 };
        rcDst.left = text_left;
        rcDst.top = rcText.top - _10;
        rcDst.right = rcDst.left + si.width;
        rcDst.bottom = rcDst.top + si.height;
        bool isLigth = draw_info.cache.pBitmapLight == pBitmap;

        bool isClip = false;

        // 如果有高亮部分, 那就设置裁剪区
        if (draw_info.nLightWidth > 0.f)
        {
            D2D1_RECT_F rcRgn = rcDst;
            float light_offset = 0.f;
            if (draw_info.nLightWidth == line.nWidth)
                light_offset += _offset;
            float light_right = rcRgn.left + draw_info.nLightWidth + _offset + light_offset;
            if (isLigth)
            {
                // 高亮文本, 限制右边不能显示, 右边显示范围就是高亮的宽度
                rcRgn.right = light_right;
            }
            else
            {
                // 普通文本, 限制左边不能显示
                rcRgn.left = light_right;
            }
            isClip = true;
            pRenderTarget->PushAxisAlignedClip(rcRgn, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }


        //if (!isLigth)
        //{
        //    CD2DBrush hbrBak(MAKEARGB(180, 255, 0, 0));
        //    pRenderTarget->FillRectangle(rcDst, hbrBak);
        //}

        pRenderTarget->DrawBitmap(pBitmap, rcDst, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rcSrc);
        if (isClip)
            pRenderTarget->PopAxisAlignedClip();

        pRenderTarget->PopAxisAlignedClip();
    };

    pfn_draw_bitmap(draw_info.cache.pBitmapNormal);
    if (draw_info.nLightWidth > 0)  // 有高亮才绘画高亮部分
        pfn_draw_bitmap(draw_info.cache.pBitmapLight);
}


NAMESPACE_LYRIC_WND_END