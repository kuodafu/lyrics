#include "lyric_wnd_function.h"
#include <tstr.h>

using namespace NAMESPACE_D2D;



NAMESPACE_LYRIC_WND_BEGIN

void lyric_wnd_draw_button(LYRIC_WND_INFU& wnd_info, const RECT& rcWindow)
{
    if (!wnd_info.isFillBack)
        return;
    const int _10 = wnd_info.scale(10);
    const int _20 = _10 * 2;
    const int _50 = _10 * 5;

    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    ID2D1DeviceContext* pRenderTarget = hCanvas;
    const int offset = _10, offset_top = (int)wnd_info.shadowRadius, line_height = _10 * 2;
    const UINT dpi = wnd_info.scale;

    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;

    wnd_info.button.width = lyric_wnd_calc_button(wnd_info, wnd_info.button.maxWidth, wnd_info.button.maxHeight, offset);
    lyric_wnd_calc_height(wnd_info);


    const bool is_vertical = wnd_info.has_mode(LYRIC_MODE::VERTICAL);
    int left = (cxClient - wnd_info.button.width) / 2;
    int top = offset_top;

    if (is_vertical)
    {
        // 竖屏
        left = offset_top;
        top = (cyClient - wnd_info.button.height) / 2;
    }

    D2D1_ANTIALIAS_MODE oldMode = pRenderTarget->GetAntialiasMode();
    pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    HRESULT hr = 0;
    ID2D1Bitmap1* image = wnd_info.dx.image->GetBitmap(*wnd_info.dx.hCanvas, &hr);
    const float bl = (float)((double)dpi / 96.0);

    for (LYRIC_WND_BUTTON_INFO& item : wnd_info.button.rcBtn)
    {
        if (item.id == 0)
        {
            // 需要画一个分割线
            ID2D1SolidColorBrush* pBrush = *wnd_info.dx.hbrLine;
            if (is_vertical)
            {
                // 竖屏
                D2D1_POINT_2F pt1{ (float)(left), (float)(top + item.rc.top) };
                D2D1_POINT_2F pt2{ pt1 };
                pt2.x += (float)(line_height);
                pRenderTarget->DrawLine(pt1, pt2, pBrush, 2.0f);
            }
            else
            {
                D2D1_POINT_2F pt1{ (float)(left + item.rc.left), (float)(top + (wnd_info.button.maxHeight - line_height) / 2) };
                D2D1_POINT_2F pt2{ pt1 };
                pt2.y += (float)(line_height);
                pRenderTarget->DrawLine(pt1, pt2, pBrush, 2.0f);
            }
            continue;
        }
        RECT_F rcDst, rcSrc = *item.prcSrc;
        float width = rcSrc.width();
        float height = rcSrc.height();
        if (item.id != LYRIC_WND_BUTTON_ID_PLAY && item.id != LYRIC_WND_BUTTON_ID_PAUSE)
            height *= bl, width *= bl;

        if (is_vertical)
        {
            // 竖屏
            rcDst.left = (float)(left);
            rcDst.top = (float)(top + item.rc.top);

        }
        else
        {
            // 横屏
            rcDst.left = (float)(left + item.rc.left);
            rcDst.top = (wnd_info.button.maxHeight - height) / 2 + (float)top;

        }


        rcDst.right = rcDst.left + width;
        rcDst.bottom = rcDst.top + height;
        item.rc = rcDst;    // 记录绘画的位置, 鼠标移动的时候判断在这个位置内就是在按钮内了
        hCanvas->DrawBitmap(image, rcDst, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rcSrc);
    }

    pRenderTarget->SetAntialiasMode(oldMode);

}


NAMESPACE_LYRIC_WND_END

