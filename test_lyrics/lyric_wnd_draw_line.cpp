#include "lyric_wnd_function.h"
#include <control/CControlDraw.h>

using namespace NAMESPACE_D2D;

NAMESPACE_LYRIC_WND_BEGIN

// �滭˫�и��
void lyric_wnd_draw_double_row(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

// �滭���и��
void lyric_wnd_draw_single_row(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

// �滭����/������, ����϶���˫��
void lyric_wnd_draw_translate(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow);

void lyric_wnd_draw_line(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& line_info, int nDrawLineIndex);

// �ѻ�����ı��滭����
void lyric_wnd_draw_cache_text(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);



// �滭����ı��ĺ���, ������������ı��滭����
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
        lyric_wnd_draw_double_row(wnd_info, arg, rcWindow); // Ĭ��˫�и��, ǰ��������������������


    // ������ػ�����, ����ı��϶�Ҫ�滭
    lyric_wnd_draw_line(wnd_info, wnd_info.line1, 1);
    lyric_wnd_draw_line(wnd_info, wnd_info.line2, 2);

}

// �滭˫�и��
void lyric_wnd_draw_double_row(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
    float nLightWidth = arg.word.nLeft + arg.nWidthWord;
    float nLightHeight = arg.word.nTop + arg.nHeightWord;

    // �Ƿ��л���һ��, ��ǰλ�ó�����ʵ�30%����л�����һ��
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
    pLine1->nLightWidth = nLightWidth;      // ��ǰ�еĸ���λ��
    pLine1->nLightHeight = nLightHeight;    // ��ǰ�еĸ���λ��

    if (wnd_info.has_mode(LYRIC_MODE::VERTICAL))
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

void lyric_wnd_draw_single_row(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
}

void lyric_wnd_draw_translate(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg, const RECT& rcWindow)
{
    float nLightWidth = arg.word.nLeft + arg.nWidthWord;

    // ԭ�ı��Ͱ�����λ�û滭, ����/������Ҫ�������Ȼ�����ø���λ��


    LYRIC_WND_DRAWTEXT_INFO* pLine1 = &wnd_info.line1, * pLine2 = &wnd_info.line2;

    pLine1->line = arg.line;    // ���ж�ָ��ͬһ���ı���, ���ж�״̬ѡ����ͨ/����/������
    pLine2->line = arg.line;
    pLine1->nLightWidth = nLightWidth;  // ��ǰ�еĸ���λ��
    pLine2->nLightWidth = 0;            // ���ڻ滭������ǰ�������λ��

}

void lyric_wnd_draw_line(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& line_info, int nDrawLineIndex)
{
    //TODO ������������ж�ѡ�����ַ�ʽ�滭����ı�
    decltype(lyric_wnd_draw_text_geometry)* pfn_create_cache_bitmap = nullptr;
    if (1)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_geometry;
    else if (2)
        pfn_create_cache_bitmap = lyric_wnd_draw_text_glow;

    auto& cache = line_info.cache;
    auto& line = line_info.line;
    // ���ж������Ƿ�ı�, ����ı�, ��Ҫ���»���
    if (wnd_info.change_trans
        || cache.preIndex != line_info.index
        || cache.preText != line.pText
        || cache.preLength != line.nLength
        || wnd_info.change_font || wnd_info.change_hbr  // ����/��ˢ�ı�, ��Ҫ���´�������
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

void lyric_wnd_draw_cache_text(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex)
{
    // �ӻ�����������ó�������Ŀ����
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
        const int shadowRadius = (int)wnd_info.shadowRadius;    // ��Ӱ������ɢ�뾶

        // �����ʿ�ȳ������, �Ǿ�Ҫ�ڸ�ʸ�����ʾ������һ���ʱ��������λ��
        const int cxScreen = rcWindow.right - rcWindow.left;        // ������������ڵĿ��
        const int cxBound = cxScreen - shadowRadius * 2;            // ����Ǵ��ڿ�����ʾ�Ŀ��
        const float lrcWidth = ((float)cxBound) - _offset * 2;      // ����Ǹ���ı��������, ���ڿ�ȼ�ȥ������Ӱ, ��ȥ��ʻ滭����ƫ��
        const float _p60 = lrcWidth * 0.6f;
        const D2D1_RECT_F& rcText = draw_info.rcText;

        // ��С����߾���
        const float min_left = cxBound - draw_info.line.nWidth - rcText.left + _offset / 2;


        // ��λͼ�����λ���ó����滭, ����������λͼ����
        D2D1_SIZE_F si = pBitmap->GetSize();
        D2D1_RECT_F rcSrc = { 0.f, 0.f, si.width, si.height };

        float text_left = rcText.left;
        if (si.width > cxBound)
        {
            // ��ʿ�ȳ������ڿ��, �жϸ���λ���Ƿ���ڴ���һ��, �������, �͵������λ��
            // ��Ҫ�����ұ�����ʾ����
            if (draw_info.nLightWidth > _p60)
            {
                // ����λ�� ��ȥ ��Ȱٷֱ�, �õ��ٷֱ� �� ����λ�õľ���
                // ���λ����Ҫ��ȥ�������
                text_left -= (draw_info.nLightWidth - _p60);
                text_left = max(min_left, text_left);   // �趨һ����Сֵ, С�������Сֵ����Ϊ��Сֵ
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

        // ����и�������, �Ǿ����òü���
        if (draw_info.nLightWidth > 0.f)
        {
            D2D1_RECT_F rcRgn = rcDst;
            float light_offset = 0.f;
            if (draw_info.nLightWidth == draw_info.text_width)
                light_offset += _offset;
            float light_right = rcRgn.left + draw_info.nLightWidth + _offset + light_offset;
            if (isLigth)
            {
                // �����ı�, �����ұ߲�����ʾ, �ұ���ʾ��Χ���Ǹ����Ŀ��
                rcRgn.right = light_right;
            }
            else
            {
                // ��ͨ�ı�, ������߲�����ʾ
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

    //TODO ����ǻ滭������, ������Ҫ���㷭���ʵĸ���λ��, Ȼ�����õ� draw_info.nLightWidth
    if (wnd_info.has_mode(LYRIC_MODE::EXISTTRANS))
    {
        if (wnd_info.has_mode(LYRIC_MODE::TRANSLATION1) || wnd_info.has_mode(LYRIC_MODE::TRANSLATION2))
        {
            // �������λ��, ��ͨ����һ�еĸ���λ�ü���
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

    // ����滭λ��
    lyric_wnd_draw_calc_text_rect(wnd_info, draw_info, nDrawLineIndex);

    pfn_draw_bitmap(draw_info.cache.pBitmapNormal);
    if (draw_info.nLightWidth > 0)  // �и����Ż滭��������
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
            align = 1;  // ����Ƿ���/����, �Ǿ�ǿ�ƾ��ж���
    }
    const bool is_vertical = wnd_info.has_mode(LYRIC_MODE::VERTICAL);
    if (is_vertical)
    {
        float top = 0;
        float left = (float)(nDrawLineIndex == 1 ? wnd_info.nLineTop1 : wnd_info.nLineTop2);
        float width = (float)wnd_info.get_lyric_line_height();

        if (align == 1)
        {
            // ���ж���
            int nTemp = (cyClient - nLeft * 2 - (int)draw_info.text_height) / 2;
            top = (float)max(nLeft, nTemp);
        }
        else if (align == 2)
        {
            // �Ҷ���
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
            // ���ж���
            int nTemp = (cxClient - nLeft * 2 - (int)draw_info.text_width) / 2;
            left = (float)max(nLeft, nTemp);
        }
        else if (align == 2)
        {
            // �Ҷ���
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