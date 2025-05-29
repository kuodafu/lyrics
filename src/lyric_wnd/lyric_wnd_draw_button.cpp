#include "lyric_wnd_function.h"

using namespace NAMESPACE_D2D;



NAMESPACE_LYRIC_WND_BEGIN

void lyric_wnd_draw_button(LYRIC_WND_INFO& wnd_info)
{
    if (!wnd_info.isFillBack)
        return;
    CD2DRender& hCanvas = *wnd_info.dx.hCanvas;
    ID2D1DeviceContext* pRenderTarget = hCanvas;

    lyric_wnd_calc_btn_pos(wnd_info);
    lyric_wnd_calc_wnd_pos(wnd_info, false);

    D2D1_ANTIALIAS_MODE oldMode = pRenderTarget->GetAntialiasMode();
    pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

    HRESULT hr = 0;
    ID2D1Bitmap1* image = wnd_info.dx.image->GetBitmap(*wnd_info.dx.hCanvas, &hr);

    const RECT& rcBtn = wnd_info.button.rc;
    for (LYRIC_WND_BUTTON_INFO& item : wnd_info.button.rcBtn)
    {
        if (item.id == 0)
        {
            // 需要画一个分割线
            ID2D1SolidColorBrush* pBrush = *wnd_info.dx.hbrLine;
            auto ppt = (POINT*)&item.rc;
            D2D1_POINT_2F pt1{ (float)ppt[0].x, (float)ppt[0].y };
            D2D1_POINT_2F pt2{ (float)ppt[1].x, (float)ppt[1].y };
            pRenderTarget->DrawLine(pt1, pt2, pBrush, 2.0f);
            continue;
        }
        RECT_F rcDst = item.rc, rcSrc = *item.prcSrc;
        hCanvas->DrawBitmap(image, rcDst, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, rcSrc);
    }

    pRenderTarget->SetAntialiasMode(oldMode);

}


// 获取鼠标所在的屏幕位置, 返回屏幕矩形
static void _lyric_wnd_calc_wnd_pos_get_monitor_info(RECT& rc, POINT& pt)
{
    MONITORINFO mi = { sizeof(MONITORINFO) };
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    if (!hMonitor || !GetMonitorInfoW(hMonitor, &mi))
    {
        rc = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
        return;
    }
    rc = mi.rcMonitor;
}

void lyric_wnd_calc_wnd_pos(LYRIC_WND_INFO& wnd_info, bool isMoveWindow)
{
    const bool is_vertical = wnd_info.has_mode(LYRIC_MODE::VERTICAL);
    const auto padding_wnd = (int)wnd_info.padding_wnd;
    const auto padding_text = (int)wnd_info.padding_text;
    const auto shadowRadius = (int)wnd_info.shadowRadius;

    const RECT& rcButtom = wnd_info.button.rc;
    if (is_vertical)
    {
        const int btn_height = rcButtom.bottom - rcButtom.top;
        const int text_width = (int)(wnd_info.word_width + padding_text * 2);
        wnd_info.nLineTop1 = wnd_info.button.rc.left + wnd_info.button.maxWidth + padding_wnd;
        wnd_info.nLineTop2 = wnd_info.nLineTop1 + text_width + 2;
        wnd_info.nMinWidth = wnd_info.nLineTop2 + text_width + padding_wnd + shadowRadius;
        wnd_info.nMinHeight = btn_height + padding_wnd * 2 + shadowRadius * 2;

        if (isMoveWindow)
        {
            auto& pos = wnd_info.pos_v;
            POINT pt = { 0 };
            GetCursorPos(&pt);

            if (pos.height == 0 || pos.width == 0)
            {
                // 做一个默认位置, 取当前鼠标所在的屏幕, 然后计算位置, 显示到窗口右边
                RECT rcScreen;
                _lyric_wnd_calc_wnd_pos_get_monitor_info(rcScreen, pt);

                pos.width = wnd_info.nMinWidth;
                pos.height = wnd_info.scale(800);
                pos.left = rcScreen.right - wnd_info.nMinWidth - (int)wnd_info.padding_wnd;
                pos.top = rcScreen.top + (rcScreen.bottom - rcScreen.top - pos.height) / 2;
                pos.right = pos.left + pos.width;
                pos.bottom = pos.top + pos.height;
            }
            else
            {
                pos.width = wnd_info.nMinWidth;
                pos.right = pos.left + pos.width;
            }

            if (!PtInRect(&pos, pt))
                wnd_info.isFillBack = false;    // 移动后鼠标不在窗口内, 不填充背景

            lyric_wnd_invalidate(wnd_info);
            MoveWindow(wnd_info.hWnd, pos.left, pos.top, pos.width, pos.height, TRUE);

        }
        return;
    }

    const int btn_width = rcButtom.right - rcButtom.left;
    const int text_height = (int)(wnd_info.word_height + padding_text * 2);
    wnd_info.nLineTop1 = wnd_info.button.rc.top + wnd_info.button.maxHeight + padding_wnd;
    wnd_info.nLineTop2 = wnd_info.nLineTop1 + text_height + 2;
    wnd_info.nMinWidth = btn_width + padding_wnd * 2 + shadowRadius * 2;
    wnd_info.nMinHeight = wnd_info.nLineTop2 + text_height + padding_wnd + shadowRadius;

    if (isMoveWindow)
    {
        auto& pos = wnd_info.pos_h;
        POINT pt = { 0 };
        GetCursorPos(&pt);

        if (pos.height == 0 || pos.width == 0)
        {
            // 做一个默认位置, 取当前鼠标所在的屏幕, 然后计算位置, 显示到窗口右边
            RECT rcScreen;
            _lyric_wnd_calc_wnd_pos_get_monitor_info(rcScreen, pt);

            pos.width = wnd_info.scale(800);
            pos.height = wnd_info.nMinHeight;
            pos.left = (rcScreen.right - rcScreen.left - pos.width) / 2;
            pos.top = rcScreen.bottom - wnd_info.nMinHeight - (int)wnd_info.padding_wnd - 100;
            pos.right = pos.left + pos.width;
            pos.bottom = pos.top + pos.height;
        }
        else
        {
            pos.height = wnd_info.nMinHeight;
            pos.bottom = pos.left + pos.height;
        }

        if (!PtInRect(&pos, pt))
            wnd_info.isFillBack = false;    // 移动后鼠标不在窗口内, 不填充背景

        lyric_wnd_invalidate(wnd_info);
        MoveWindow(wnd_info.hWnd, pos.left, pos.top, pos.width, pos.height, TRUE);
    }
}

void lyric_wnd_calc_btn_pos(LYRIC_WND_INFO& wnd_info)
{
    const RECT& rcWindow = wnd_info.rcWindow;
    const int cxClient = rcWindow.right - rcWindow.left;
    const int cyClient = rcWindow.bottom - rcWindow.top;

    int height = 0;
    int width = 0;
    const auto padding  = (int)wnd_info.padding_wnd / 2;
    const auto padding2 = padding / 2;
    const auto line_height = (int)wnd_info.padding_wnd * 2;

    wnd_info.button.maxHeight = 50; // 最大宽高固定死
    wnd_info.button.maxWidth = 50;

    const auto dpi = wnd_info.scale.GetDpi();
    const bool is_vertical  = wnd_info.has_mode(LYRIC_MODE::VERTICAL);
    const auto shadowRadius = (int)wnd_info.shadowRadius;

    int left = 0;
    int top = 0;
    int index = -1;
    for (LYRIC_WND_BUTTON_INFO& item : wnd_info.button.rcBtn)
    {
        index++;
        item.prcSrc = nullptr;
        if (item.id == 0)
        {
            if (is_vertical)
            {
                auto ppt = (POINT*)&item.rc;
                ppt[0].x = shadowRadius + (wnd_info.button.maxWidth - line_height) / 2;
                ppt[0].y = top + padding2;

                ppt[1].x = ppt[0].x + line_height;
                ppt[1].y = ppt[0].y;
            }
            else
            {
                auto ppt = (POINT*)&item.rc;
                ppt[0].x = left + padding2;
                ppt[0].y = shadowRadius + (wnd_info.button.maxHeight - line_height) / 2;

                ppt[1].x = ppt[0].x;
                ppt[1].y = ppt[0].y + line_height;
            }
            left += padding;
            top += padding;
            width += padding;
            height += padding;
        }
        else
        {
            auto& item_src = wnd_info.button.rcSrc[item.id - LYRIC_WND_BUTTON_ID_FIRST];
            if (wnd_info.button.indexDown == index)
            {
                // 当前按钮是按下状态
                item.prcSrc = &item_src.rcDown;
            }
            else if (wnd_info.button.index == index)
            {
                // 当前按钮是热点状态
                item.prcSrc = &item_src.rcLight;
            }
            else if (__query(item.state, LYRIC_WND_BUTTON_STATE_DISABLE))
            {
                // 当前按钮是禁止状态
                item.prcSrc = &item_src.rcDisable;
            }
            else
            {
                item.prcSrc = &item_src.rcNormal;   // 剩下的就是禁止状态
            }
            const RECT& rcSrc = *item.prcSrc;
            int btn_width = rcSrc.right - rcSrc.left;
            int btn_height = rcSrc.bottom - rcSrc.top;

            if (btn_width < wnd_info.button.maxWidth || btn_height < wnd_info.button.maxHeight)
                btn_height = btn_height * dpi / 96, btn_width = btn_width * dpi / 96;   // 缩放按钮

            //if (wnd_info.button.maxHeight < btn_height && btn_height == btn_width)
            //    wnd_info.button.maxHeight = btn_height;
            //if (wnd_info.button.maxWidth < btn_width && btn_height == btn_width)
            //    wnd_info.button.maxWidth = btn_width;



            if (is_vertical)
            {
                item.rc.left = shadowRadius + (wnd_info.button.maxWidth - btn_width) / 2;
                item.rc.top = top;
            }
            else
            {
                item.rc.left = left;
                item.rc.top = shadowRadius + (wnd_info.button.maxHeight - btn_height) / 2;
            }
            item.rc.right = item.rc.left + btn_width;
            item.rc.bottom = item.rc.top + btn_height;

            left += btn_width + padding;
            top += btn_height + padding;
            width += btn_width + padding;
            height += btn_height + padding;

        }

    }



    if (is_vertical)
    {
        wnd_info.button.rc.left     = (int)wnd_info.shadowRadius;
        wnd_info.button.rc.top      = (cyClient - height) / 2;
        wnd_info.button.rc.right    = wnd_info.button.rc.left + wnd_info.button.maxWidth;
        wnd_info.button.rc.bottom   = wnd_info.button.rc.top + height;
    }
    else
    {
        wnd_info.button.rc.left = (cxClient - width) / 2;
        wnd_info.button.rc.top = (int)wnd_info.shadowRadius;
        wnd_info.button.rc.right = wnd_info.button.rc.left + width;
        wnd_info.button.rc.bottom = wnd_info.button.rc.top + wnd_info.button.maxHeight;
    }

    for (LYRIC_WND_BUTTON_INFO& item : wnd_info.button.rcBtn)
    {
        if (item.id == 0)
        {
            auto ppt = (POINT*)&item.rc;
            if (is_vertical)
                ppt[0].y += wnd_info.button.rc.top, ppt[1].y += wnd_info.button.rc.top;
            else
                ppt[0].x += wnd_info.button.rc.left, ppt[1].x += wnd_info.button.rc.left;
            continue;
        }

        if (is_vertical)
        {
            item.rc.top += wnd_info.button.rc.top;
            item.rc.bottom += wnd_info.button.rc.top;
        }
        else
        {
            item.rc.left += wnd_info.button.rc.left;
            item.rc.right += wnd_info.button.rc.left;
        }
    }

}


NAMESPACE_LYRIC_WND_END

