#include "lyric_wnd_function.h"
#include <control/CControlDraw.h>

using namespace NAMESPACE_D2D;

NAMESPACE_LYRIC_WND_BEGIN

// 绘画双行歌词
void lyric_wnd_draw_double_row(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

// 绘画单行歌词
void lyric_wnd_draw_single_row(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

// 绘画翻译/音译歌词, 这里肯定是双行
void lyric_wnd_draw_translate(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

void lyric_wnd_draw_line(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& line_info, int nDrawLineIndex);

// 把缓存的文本绘画出来
void lyric_wnd_draw_cache_text(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);



// 绘画歌词文本的函数, 在这个函数把文本绘画出来
void lyric_wnd_draw_lyric(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg)
{
    const RECT& rcWindow = wnd_info.rcWindow;

    if (!wnd_info.hLyric)
        arg.line.pText = wnd_info.pszDefText, arg.line.nLength = wnd_info.nDefText;

    if (wnd_info.has_mode(LYRIC_MODE::EXISTTRANS) && (wnd_info.has_mode(LYRIC_MODE::TRANSLATION1) || wnd_info.has_mode(LYRIC_MODE::TRANSLATION2)))
        lyric_wnd_draw_translate(wnd_info, arg, rcWindow);
    else if (wnd_info.has_mode(LYRIC_MODE::SINGLE_ROW))
        lyric_wnd_draw_single_row(wnd_info, arg, rcWindow);
    else
        lyric_wnd_draw_double_row(wnd_info, arg, rcWindow); // 默认双行歌词, 前面条件不成立就走这里


    // 如果有重画操作, 歌词文本肯定要绘画
    lyric_wnd_draw_line(wnd_info, wnd_info.line1, 1);
    lyric_wnd_draw_line(wnd_info, wnd_info.line2, 2);

}

// 绘画双行歌词
void lyric_wnd_draw_double_row(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
    float nLightWidth = arg.word.nLeft + arg.nWidthWord;
    float nLightHeight = arg.word.nTop + arg.nHeightWord;

    // 是否切换下一行, 当前位置超过歌词的30%后就切换到下一行
    bool isSwitchLine_H = nLightWidth > arg.line.nWidth * 0.3;
    bool isSwitchLine_V = nLightHeight > arg.line.nHeight * 0.3;

    LYRIC_WND_DRAWTEXT_INFO* pLine1 = nullptr, * pLine2 = nullptr;
    if (arg.indexLine % 2 == 0)
    {
        pLine1 = &wnd_info.line1;
        pLine2 = &wnd_info.line2;

        pLine1->index = arg.indexLine;
        pLine2->index = wnd_info.line2.cache.preIndex;
        if (pLine2->index == -1)
            pLine2->index = pLine1->index + 1;
    }
    else
    {
        pLine1 = &wnd_info.line2;
        pLine2 = &wnd_info.line1;

        pLine1->index = wnd_info.line1.cache.preIndex;
        pLine2->index = arg.indexLine;
        if (pLine1->index == -1)
            pLine1->index = pLine2->index + 1;
    }

    pLine1->line = arg.line;
    pLine1->nLightWidth = nLightWidth;      // 当前行的高亮位置
    pLine1->nLightHeight = nLightHeight;    // 当前行的高亮位置

    if (wnd_info.has_mode(LYRIC_MODE::VERTICAL))
    {
        // 如果需要换行, 那就获取下一行歌词信息保存到 pLine2, pLine2 高亮为0
        if (isSwitchLine_V || arg.indexLine == 0)
        {
            if (arg.indexLine + 1 < arg.nLineCount)
                lyric_get_line(wnd_info.hLyric, arg.indexLine + 1, &pLine2->line);
            pLine2->nLightHeight = (arg.indexLine + 1 == arg.nLineCount) ? pLine2->line.nHeight : 0.0f;
        }
        else if (!isSwitchLine_V)
        {
            // 还不切换下一行, 另一行的歌词高亮是100%, 另一行歌词是当前行是上一行
            lyric_get_line(wnd_info.hLyric, arg.indexLine - 1, &pLine2->line);
            pLine2->nLightHeight = pLine2->line.nHeight;
        }
    }
    else
    {
        // 如果需要换行, 那就获取下一行歌词信息保存到 pLine2, pLine2 高亮为0
        if (isSwitchLine_H || arg.indexLine == 0)
        {
            if (arg.indexLine + 1 < arg.nLineCount)
                lyric_get_line(wnd_info.hLyric, arg.indexLine + 1, &pLine2->line);
            pLine2->nLightWidth = (arg.indexLine + 1 == arg.nLineCount) ? pLine2->line.nWidth : 0.0f;
        }
        else if (!isSwitchLine_H)
        {
            // 还不切换下一行, 另一行的歌词高亮是100%, 另一行歌词是当前行是上一行
            lyric_get_line(wnd_info.hLyric, arg.indexLine - 1, &pLine2->line);
            pLine2->nLightWidth = pLine2->line.nWidth;
        }
    }

}

void lyric_wnd_draw_single_row(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
}

void lyric_wnd_draw_translate(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
    float nLightWidth = arg.word.nLeft + arg.nWidthWord;

    // 原文本就按高亮位置绘画, 翻译/音译需要计算比例然后设置高亮位置


    LYRIC_WND_DRAWTEXT_INFO* pLine1 = &wnd_info.line1, * pLine2 = &wnd_info.line2;

    pLine1->line = arg.line;    // 两行都指向同一个文本行, 会判断状态选择普通/翻译/音译歌词
    pLine2->line = arg.line;
    pLine1->nLightWidth = nLightWidth;  // 当前行的高亮位置
    pLine2->nLightWidth = 0;            // 会在绘画到窗口前计算高亮位置

}

void lyric_wnd_draw_line(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& line_info, int nDrawLineIndex)
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
    if (wnd_info.change_trans
        || cache.preIndex != line_info.index
        || cache.preText != line.pText
        || cache.preLength != line.nLength
        || wnd_info.change_font || wnd_info.change_hbr  // 字体/画刷改变, 需要重新创建缓存
        )
    {
        // 上次记录的值和这次不一样了, 文本改变了, 重新绘画, 然后记录到位图里

        // 记录绘画的行好和文本
        cache.preIndex = line_info.index;
        cache.preText = line.pText;
        cache.preLength = line.nLength;

        // 创建缓存位图
        pfn_create_cache_bitmap(wnd_info, line_info, nDrawLineIndex);

    }

    lyric_wnd_draw_cache_text(wnd_info, line_info, nDrawLineIndex);

}

void lyric_wnd_draw_cache_text(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex)
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
        const bool is_vertical = wnd_info.has_mode(LYRIC_MODE::VERTICAL);

        LYRIC_LINE_STRUCT& line = draw_info.line;
        const int _10 = wnd_info.scale(10);

        RECT& rcWindow = wnd_info.rcWindow;

        const float _offset = wnd_info.padding;
        //const float _offset = (draw_info.layout_text_max_height - wnd_info.nLineHeight) / 2;
        const int shadowRadius = (int)wnd_info.shadowRadius;    // 阴影向外扩散半径

        // 如果歌词宽度超过这个, 那就要在歌词高亮显示到窗口一半的时候调整左边位置
        const int cxScreen = rcWindow.right - rcWindow.left;        // 这个是整个窗口的宽度
        const int cxBound = cxScreen - shadowRadius * 2;            // 这个是窗口可以显示的宽度
        const float lrcWidth = ((float)cxBound) - _offset * 2;      // 这个是歌词文本的最大宽度, 窗口宽度减去左右阴影, 减去歌词绘画左右偏移
        const float _p60 = lrcWidth * 0.6f;
        const D2D1_RECT_F& rcText = draw_info.rcText;

        // 最小的左边距离
        const float min_left = cxBound - draw_info.line.nWidth - rcText.left + _offset / 2;


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

        if (is_vertical)
        {
            if (nDrawLineIndex == 1)
                text_left = (float)wnd_info.nLineTop1;
            else
                text_left = (float)wnd_info.nLineTop2;
        }

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
            if (draw_info.nLightWidth == draw_info.text_width)
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

    //TODO 如果是绘画翻译歌词, 这里需要计算翻译歌词的高亮位置, 然后设置到 draw_info.nLightWidth
    if (wnd_info.has_mode(LYRIC_MODE::EXISTTRANS))
    {
        if (wnd_info.has_mode(LYRIC_MODE::TRANSLATION1) || wnd_info.has_mode(LYRIC_MODE::TRANSLATION2))
        {
            // 计算高亮位置, 是通过第一行的高亮位置计算
            auto& line1 = wnd_info.line1;
            const bool is_vertical = wnd_info.has_mode(LYRIC_MODE::VERTICAL);
            if (is_vertical)
            {
                draw_info.nLightHeight = line1.nLightHeight / line1.line.nHeight * draw_info.text_height;
            }
            else
            {
                draw_info.nLightWidth = line1.nLightWidth / line1.line.nWidth * draw_info.text_width;
            }
        }
    }

    // 计算绘画位置
    lyric_wnd_draw_calc_text_rect(wnd_info, draw_info, nDrawLineIndex);

    pfn_draw_bitmap(draw_info.cache.pBitmapNormal);
    if (draw_info.nLightWidth > 0)  // 有高亮才绘画高亮部分
        pfn_draw_bitmap(draw_info.cache.pBitmapLight);
}


void lyric_wnd_draw_calc_text_rect(LYRIC_WND_INFO& wnd_info,
                                  LYRIC_WND_DRAWTEXT_INFO& draw_info,
                                  int nDrawLineIndex)
{
    const RECT& rcWindow = wnd_info.rcWindow;

    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;

    const int nLeft = wnd_info.scale(16);


    int align = draw_info.align;
    if (wnd_info.has_mode(LYRIC_MODE::EXISTTRANS))
    {
        if (wnd_info.has_mode(LYRIC_MODE::TRANSLATION1) || wnd_info.has_mode(LYRIC_MODE::TRANSLATION2))
            align = 1;  // 如果是翻译/音译, 那就强制居中对齐
    }
    const bool is_vertical = wnd_info.has_mode(LYRIC_MODE::VERTICAL);
    if (is_vertical)
    {
        float top = 0;
        float left = (float)(nDrawLineIndex == 1 ? wnd_info.nLineTop1 : wnd_info.nLineTop2);
        float width = (float)wnd_info.get_lyric_line_height();

        if (align == 1)
        {
            // 居中对齐
            int nTemp = (cyClient - nLeft * 2 - (int)draw_info.text_height) / 2;
            top = (float)max(nLeft, nTemp);
        }
        else if (align == 2)
        {
            // 右对齐
            int nTemp = cyClient - nLeft * 2 - (int)draw_info.text_height;
            top = (float)max(nLeft, nTemp);
        }
        else
        {
            top = (float)nLeft;
        }

        draw_info.rcText = { left, top, left + width, top + draw_info.text_height };
    }
    else
    {
        float top = (float)(nDrawLineIndex == 1 ? wnd_info.nLineTop1 : wnd_info.nLineTop2);
        float left = 0.f;
        float height = (float)wnd_info.get_lyric_line_height();

        if (align == 1)
        {
            // 居中对齐
            int nTemp = (cxClient - nLeft * 2 - (int)draw_info.text_width) / 2;
            left = (float)max(nLeft, nTemp);
        }
        else if (align == 2)
        {
            // 右对齐
            int nTemp = cxClient - nLeft * 2 - (int)draw_info.text_width;
            left = (float)max(nLeft, nTemp);
        }
        else
        {
            left = (float)nLeft;
        }

        draw_info.rcText = { left, top, left + draw_info.text_width, top + height };
    }
}

NAMESPACE_LYRIC_WND_END