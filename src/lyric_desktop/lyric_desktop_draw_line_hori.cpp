#include "lyric_wnd_function.h"

using namespace KUODAFU_NAMESPACE;

NAMESPACE_LYRIC_DESKTOP_BEGIN

// �滭��������ı�
void lyric_wnd_draw_cache_text_h(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, ID2D1Bitmap* pBitmap)
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
    const int shadowRadius = (int)wnd_info.shadowRadius;    // ��Ӱ������ɢ�뾶

    // �����ʿ�ȳ������, �Ǿ�Ҫ�ڸ�ʸ�����ʾ������һ���ʱ��������λ��
    const int cxScreen = rcWindow.right - rcWindow.left;        // ������������ڵĿ��
    const int cxBound = cxScreen - shadowRadius * 2;            // ����Ǵ��ڿ�����ʾ�Ŀ��
    const float lrcWidth = ((float)cxBound) - _offset * 2;      // ����Ǹ���ı��������, ���ڿ�ȼ�ȥ������Ӱ, ��ȥ��ʻ滭����ƫ��
    const float _p60 = lrcWidth * 0.6f;
    const D2D1_RECT_F& rcText = draw_info.rcText;

    // ��С����߾���
    const float min_left = cxBound - draw_info.text_width - rcText.left + _offset / 2;


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
    rcDst.top = rcText.top;
    rcDst.right = rcDst.left + si.width;
    rcDst.bottom = rcDst.top + si.height;
    bool isLigth = draw_info.cache.pBitmapLight == pBitmap;

    bool isClip = false;

    if (draw_info.nLightWidth > 0.f && draw_info.nLightWidth < draw_info.text_width)
    {
        D2D1_RECT_F rcRgn = rcDst;
        float light_right = rcRgn.left + draw_info.nLightWidth + _offset;
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
    
    pRenderTarget->DrawBitmap(pBitmap, rcDst, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rcSrc);
    if (isClip)
        pRenderTarget->PopAxisAlignedClip();

    pRenderTarget->PopAxisAlignedClip();
}


NAMESPACE_LYRIC_DESKTOP_END
