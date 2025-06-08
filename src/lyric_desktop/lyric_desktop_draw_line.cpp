#include "lyric_desktop_function.h"

using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN

// �滭˫�и��
void lyric_wnd_draw_double_row(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

// �滭���и��
void lyric_wnd_draw_single_row(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

// �滭����/������, ����϶���˫��
void lyric_wnd_draw_translate(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

void lyric_wnd_draw_line(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& line_info, int nDrawLineIndex);

// �ѻ�����ı��滭����
void lyric_wnd_draw_cache_text(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);



// �滭����ı��ĺ���, ������������ı��滭����
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
        lyric_wnd_draw_double_row(wnd_info, arg, rcWindow); // Ĭ��˫�и��, ǰ��������������������


    // ������ػ�����, ����ı��϶�Ҫ�滭
    lyric_wnd_draw_line(wnd_info, wnd_info.line1, 1);
    lyric_wnd_draw_line(wnd_info, wnd_info.line2, 2);

}

// �滭˫�и��
void lyric_wnd_draw_double_row(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
    float nLightWidth = arg.word.nLeft + arg.nWidthWord;
    float nLightHeight = arg.word.nTop + arg.nHeightWord;

    // �Ƿ��л���һ��, ��ǰλ�ó�����ʵ�30%����л�����һ��
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
    pLine1->nLightWidth = nLightWidth;      // ��ǰ�еĸ���λ��
    pLine1->nLightHeight = nLightHeight;    // ��ǰ�еĸ���λ��

    if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL))
    {
        // �����Ҫ����, �Ǿͻ�ȡ��һ�и����Ϣ���浽 pLine2, pLine2 ����Ϊ0
        if (isSwitchLine_V || arg.indexLine == 0)
        {
            if (arg.indexLine + 1 < arg.nLineCount)
                lyric_get_line(wnd_info.hLyric, arg.indexLine + 1, &pLine2->line);
            pLine2->nLightHeight = (arg.indexLine + 1 == arg.nLineCount) ? pLine2->line.nHeight : 0.0f;
        }
        else if (!isSwitchLine_V)
        {
            // �����л���һ��, ��һ�еĸ�ʸ�����100%, ��һ�и���ǵ�ǰ������һ��
            lyric_get_line(wnd_info.hLyric, arg.indexLine - 1, &pLine2->line);
            pLine2->nLightHeight = pLine2->line.nHeight;
        }
    }
    else
    {
        // �����Ҫ����, �Ǿͻ�ȡ��һ�и����Ϣ���浽 pLine2, pLine2 ����Ϊ0
        if (isSwitchLine_H || arg.indexLine == 0)
        {
            if (arg.indexLine + 1 < arg.nLineCount)
                lyric_get_line(wnd_info.hLyric, arg.indexLine + 1, &pLine2->line);
            pLine2->nLightWidth = (arg.indexLine + 1 == arg.nLineCount) ? pLine2->line.nWidth : 0.0f;
        }
        else if (!isSwitchLine_H)
        {
            // �����л���һ��, ��һ�еĸ�ʸ�����100%, ��һ�и���ǵ�ǰ������һ��
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

    // ԭ�ı��Ͱ�����λ�û滭, ����/������Ҫ�������Ȼ�����ø���λ��


    LYRIC_DESKTOP_DRAWTEXT_INFO* pLine1 = &wnd_info.line1, * pLine2 = &wnd_info.line2;

    pLine1->line = arg.line;    // ���ж�ָ��ͬһ���ı���, ���ж�״̬ѡ����ͨ/����/������
    pLine2->line = arg.line;
    pLine1->nLightWidth = nLightWidth;      // ��ǰ�еĸ���λ��
    pLine1->nLightHeight = nLightHeight;    // ��ǰ�еĸ���λ��
    pLine2->nLightWidth = 0;                // ���ڻ滭������ǰ�������λ��
    pLine2->nLightHeight = 0;               // ���ڻ滭������ǰ�������λ��

}

void lyric_wnd_draw_line(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& line_info, int nDrawLineIndex)
{
    //TODO ������������ж�ѡ�����ַ�ʽ�滭����ı�
    decltype(lyric_wnd_draw_text_geometry)* pfn_create_cache_bitmap = nullptr;
    if (1)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_geometry;
    else if (2)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_glow;

    auto& cache = line_info.cache;
    auto& line = line_info.line;
    // �����ж��ܽ�, �иı�ı�־, �����ı����ݲ�ͬ, �Ǿ����´�������
    if (wnd_info.config.debug.alwaysCache   // ǿ�ƻ���
        || wnd_info.change_text             // ���ı��ı��־, ��Ҫ���´�������
        || cache.preLength != line.nLength  // �ı����Ȳ�ͬ, ��Ҫ���´�������
        || (cache.preText && wcscmp(cache.preText, line.pText) != 0)   // �ı����ݲ�ͬ
        )
    {
        // �ϴμ�¼��ֵ����β�һ����, �ı��ı���, ���»滭, Ȼ���¼��λͼ��

        // ��¼�滭���кú��ı�
        cache.preIndex = line_info.index;
        cache.preText = line.pText;
        cache.preLength = line.nLength;

        // ��������λͼ
        pfn_create_cache_bitmap(wnd_info, line_info, nDrawLineIndex);

    }

    lyric_wnd_draw_cache_text(wnd_info, line_info, nDrawLineIndex);

}


void lyric_wnd_draw_cache_text(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, int nDrawLineIndex)
{
    // �ӻ�����������ó�������Ŀ����
    const bool is_vertical = wnd_info.has_mode(LYRIC_DESKTOP_MODE::VERTICAL);

    //TODO ����ǻ滭������, ������Ҫ���㷭���ʵĸ���λ��, Ȼ�����õ� draw_info.nLightWidth
    if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::EXISTTRANS))
    {
        if (wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_FY) || wnd_info.has_mode(LYRIC_DESKTOP_MODE::TRANSLATION_YY))
        {
            // �������λ��, ��ͨ����һ�еĸ���λ�ü���
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

    // ����滭λ��
    lyric_wnd_draw_calc_text_rect(wnd_info, draw_info, nDrawLineIndex);

    if (is_vertical)
    {
        // �滭��ͨ�ı�, �����ֱ�ӿ�����ʾ, ��������ʾ��Χ
        if (draw_info.nLightHeight < draw_info.text_height)
            lyric_wnd_draw_cache_text_v(wnd_info, draw_info, draw_info.cache.pBitmapNormal);

        // �и����Ż滭��������
        if (draw_info.nLightHeight > 0)
            lyric_wnd_draw_cache_text_v(wnd_info, draw_info, draw_info.cache.pBitmapLight);
    }
    else
    {
        // �滭��ͨ�ı�, �����ֱ�ӿ�����ʾ, ��������ʾ��Χ
        if (draw_info.nLightWidth < draw_info.text_width)
            lyric_wnd_draw_cache_text_h(wnd_info, draw_info, draw_info.cache.pBitmapNormal);

        // �и����Ż滭��������
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
            align = 1;  // ����Ƿ���/����, �Ǿ�ǿ�ƾ��ж���
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
            // ���ж���
            int nTemp = (cyClient - (int)text_height) / 2;
            top = (float)max(offset_top, nTemp);
        }
        else if (align == 2)
        {
            // �Ҷ���
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
            // ���ж���
            int nTemp = (cxClient - (int)text_width) / 2;
            left = (float)max(offset_left, nTemp);
        }
        else if (align == 2)
        {
            // �Ҷ���
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