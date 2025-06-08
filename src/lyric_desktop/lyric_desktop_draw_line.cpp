#include "lyric_desktop_function.h"

using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN

// 绘画双行歌词
void lyric_wnd_draw_double_row(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

// 绘画单行歌词
void lyric_wnd_draw_single_row(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

// 绘画翻译/音译歌词, 这里肯定是双行
void lyric_wnd_draw_translate(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

void lyric_wnd_draw_line(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& line_info, int nDrawLineIndex);

// 把缓存的文本绘画出来
void lyric_wnd_draw_cache_text(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);



// 绘画歌词文本的函数, 在这个函数把文本绘画出来
void lyric_wnd_draw_lyric(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg)
{
    const RECT& rcWindow = wnd_info.rcWindow;

    if (!wnd_info.hLyric)
        arg.line.pText = wnd_info.config.szDefText.c_str(), arg.line.nLength = (int)wnd_info.config.szDefText.size();

    if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::EXISTTRANS) && (wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_FY) || wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_YY)))
        lyric_wnd_draw_translate(wnd_info, arg, rcWindow);
    else if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::SINGLE_ROW))
        lyric_wnd_draw_single_row(wnd_info, arg, rcWindow);
    else
        lyric_wnd_draw_double_row(wnd_info, arg, rcWindow); // 默认双行歌词, 前面条件不成立就走这里


    // 如果有重画操作, 歌词文本肯定要绘画
    lyric_wnd_draw_line(wnd_info, wnd_info.line1, 1);
    lyric_wnd_draw_line(wnd_info, wnd_info.line2, 2);

}

// 绘画双行歌词
void lyric_wnd_draw_double_row(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
    float nLightWidth = arg.word.nLeft + arg.nWidthWord;
    float nLightHeight = arg.word.nTop + arg.nHeightWord;

    // 是否切换下一行, 当前位置超过歌词的30%后就切换到下一行
    bool isSwitchLine_H = nLightWidth > arg.line.nWidth * 0.3;
    bool isSwitchLine_V = nLightHeight > arg.line.nHeight * 0.3;

    LYRIC_DESKTOP_DRAWTEXT_INFO* pLine1 = nullptr, * pLine2 = nullptr;
    if (arg.indexLine % 2 == 1)
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

    if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL))
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

void lyric_wnd_draw_single_row(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
}

void lyric_wnd_draw_translate(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
    float nLightWidth = arg.word.nLeft + arg.nWidthWord;
    float nLightHeight = arg.word.nTop + arg.nHeightWord;

    // 原文本就按高亮位置绘画, 翻译/音译需要计算比例然后设置高亮位置


    LYRIC_DESKTOP_DRAWTEXT_INFO* pLine1 = &wnd_info.line1, * pLine2 = &wnd_info.line2;

    pLine1->line = arg.line;    // 两行都指向同一个文本行, 会判断状态选择普通/翻译/音译歌词
    pLine2->line = arg.line;
    pLine1->nLightWidth = nLightWidth;      // 当前行的高亮位置
    pLine1->nLightHeight = nLightHeight;    // 当前行的高亮位置
    pLine2->nLightWidth = 0;                // 会在绘画到窗口前计算高亮位置
    pLine2->nLightHeight = 0;               // 会在绘画到窗口前计算高亮位置

}

void lyric_wnd_draw_line(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& line_info, int nDrawLineIndex)
{
    //TODO 这里可以做个判定选择哪种方式绘画歌词文本
    decltype(lyric_wnd_draw_text_geometry)* pfn_create_cache_bitmap = nullptr;
    if (1)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_geometry;
    else if (2)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_glow;

    auto& cache = line_info.cache;
    auto& line = line_info.line;
    // 下面判断总结, 有改变的标志, 或者文本内容不同, 那就重新创建缓存
    if (wnd_info.config.debug.alwaysCache   // 强制缓存
        || wnd_info.change_text             // 有文本改变标志, 需要重新创建缓存
        || cache.preLength != line.nLength  // 文本长度不同, 需要重新创建缓存
        || (cache.preText && wcscmp(cache.preText, line.pText) != 0)   // 文本内容不同
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


void lyric_wnd_draw_cache_text(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, int nDrawLineIndex)
{
    // 从缓存里把数据拿出来画到目标上
    const bool is_vertical = wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL);

    //TODO 如果是绘画翻译歌词, 这里需要计算翻译歌词的高亮位置, 然后设置到 draw_info.nLightWidth
    if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::EXISTTRANS))
    {
        if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_FY) || wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_YY))
        {
            // 计算高亮位置, 是通过第一行的高亮位置计算
            auto& line1 = wnd_info.line1;
            if (is_vertical)
            {
                draw_info.nLightHeight = line1.nLightHeight / line1.text_height * draw_info.text_height;
            }
            else
            {
                draw_info.nLightWidth = line1.nLightWidth / line1.line.nWidth * draw_info.text_width;
            }
        }
    }

    // 计算绘画位置
    lyric_wnd_draw_calc_text_rect(wnd_info, draw_info, nDrawLineIndex);

    if (is_vertical)
    {
        // 绘画普通文本, 这个是直接拷贝显示, 不限制显示范围
        if (draw_info.nLightHeight < draw_info.text_height)
            lyric_wnd_draw_cache_text_v(wnd_info, draw_info, draw_info.cache.pBitmapNormal);

        // 有高亮才绘画高亮部分
        if (draw_info.nLightHeight > 0)
            lyric_wnd_draw_cache_text_v(wnd_info, draw_info, draw_info.cache.pBitmapLight);
    }
    else
    {
        // 绘画普通文本, 这个是直接拷贝显示, 不限制显示范围
        if (draw_info.nLightWidth < draw_info.text_width)
            lyric_wnd_draw_cache_text_h(wnd_info, draw_info, draw_info.cache.pBitmapNormal);

        // 有高亮才绘画高亮部分
        if (draw_info.nLightWidth > 0)
            lyric_wnd_draw_cache_text_h(wnd_info, draw_info, draw_info.cache.pBitmapLight);
    }

}


void lyric_wnd_draw_calc_text_rect(LYRIC_DESKTOP_INFO& wnd_info,
                                  LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info,
                                  int nDrawLineIndex)
{
    const RECT& rcWindow = wnd_info.rcWindow;

    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;



    int align = draw_info.align;
    if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::EXISTTRANS))
    {
        if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_FY) || wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_YY))
            align = 1;  // 如果是翻译/音译, 那就强制居中对齐
    }
    const bool is_vertical = wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL);
    if (is_vertical)
    {
        const auto offset_top = (int)(wnd_info.shadowRadius + wnd_info.config.padding_wnd);
        const auto left = (float)(nDrawLineIndex == 1 ? wnd_info.nLineTop1 : wnd_info.nLineTop2);
        const auto width = (float)wnd_info.get_lyric_line_height();
        const auto text_height = (float)wnd_info.get_lyric_line_width(draw_info.text_height);
        float top = 0;

        if (align == 1)
        {
            // 居中对齐
            int nTemp = (cyClient - (int)text_height) / 2;
            top = (float)max(offset_top, nTemp);
        }
        else if (align == 2)
        {
            // 右对齐
            int nTemp = cyClient - offset_top * 2 - (int)text_height;
            top = (float)max(offset_top, nTemp);
        }
        else
        {
            top = (float)offset_top;
        }

        draw_info.rcText = { left, top, left + width, top + text_height };
    }
    else
    {
        const auto offset_left = (int)(wnd_info.shadowRadius + wnd_info.config.padding_wnd);
        const auto top = (float)(nDrawLineIndex == 1 ? wnd_info.nLineTop1 : wnd_info.nLineTop2);
        const auto height = (float)wnd_info.get_lyric_line_height();
        const auto text_width = (float)wnd_info.get_lyric_line_width(draw_info.text_width);
        float left = 0.f;

        if (align == 1)
        {
            // 居中对齐
            int nTemp = (cxClient - (int)text_width) / 2;
            left = (float)max(offset_left, nTemp);
        }
        else if (align == 2)
        {
            // 右对齐
            int nTemp = cxClient - (int)(wnd_info.config.padding_wnd + wnd_info.shadowRadius + text_width);
            left = (float)max(offset_left, nTemp);
        }
        else
        {
            left = (float)offset_left;
        }

        draw_info.rcText = { left, top, left + text_width, top + height };
    }
}

NAMESPACE_LYRIC_DESKTOP_END