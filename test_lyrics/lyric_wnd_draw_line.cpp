#include "lyric_wnd_function.h"
#include <control/CControlDraw.h>

using namespace NAMESPACE_D2D;

NAMESPACE_LYRIC_WND_BEGIN

// �ѻ�����ı��滭����
void lyric_wnd_draw_cache_text(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info);

void lyric_wnd_draw_line(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& line_info, int nIndexLine)
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
    if (cache.preIndex != nIndexLine
        || cache.preText != line.pText
        || cache.preLength != line.nLength
        || wnd_info.change_font || wnd_info.change_hbr  // ����/��ˢ�ı�, ��Ҫ���´�������
        )
    {
        // �ϴμ�¼��ֵ����β�һ����, �ı��ı���, ���»滭, Ȼ���¼��λͼ��

        // ��¼�滭���кú��ı�
        cache.preIndex = nIndexLine;
        cache.preText = line.pText;
        cache.preLength = line.nLength;

        // ��������λͼ
        pfn_create_cache_bitmap(wnd_info, line_info);

    }

    lyric_wnd_draw_cache_text(wnd_info, line_info);

}

void lyric_wnd_draw_cache_text(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info)
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

        LYRIC_LINE_STRUCT& line = draw_info.line;
        const int _10 = wnd_info.scale(10);

        RECT& rcWindow = wnd_info.rcWindow;

        const float _offset = (draw_info.layout_text_max_height - wnd_info.nLineHeight) / 2;
        const int shadowRadius = (int)wnd_info.shadowRadius;    // ��Ӱ������ɢ�뾶
        const float shadowRadiusF = wnd_info.shadowRadius;      // ��Ӱ������ɢ�뾶

        // �����ʿ�ȳ������, �Ǿ�Ҫ�ڸ�ʸ�����ʾ������һ���ʱ��������λ��
        const int cxScreen = rcWindow.right - rcWindow.left;        // ������������ڵĿ��
        const int cxBound = cxScreen - shadowRadius * 2;            // ����Ǵ��ڿ�����ʾ�Ŀ��
        const float lrcWidth = ((float)cxBound) - _offset * 2;      // ����Ǹ���ı��������, ���ڿ�ȼ�ȥ������Ӱ, ��ȥ��ʻ滭����ƫ��
        const float _p60 = lrcWidth * 0.6f;
        const D2D1_RECT_F& rcText = draw_info.rcText;

        // ��С����߾���
        const float min_left = cxBound - line.nWidth - rcText.left + _offset / 2;


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
            if (draw_info.nLightWidth == line.nWidth)
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

    pfn_draw_bitmap(draw_info.cache.pBitmapNormal);
    if (draw_info.nLightWidth > 0)  // �и����Ż滭��������
        pfn_draw_bitmap(draw_info.cache.pBitmapLight);
}


NAMESPACE_LYRIC_WND_END