#include "lyric_wnd_function.h"
#include <CommCtrl.h>


NAMESPACE_LYRIC_WND_BEGIN

// 设置提示信息
void lyric_wnd_set_tips(LYRIC_WND_INFO& wnd_info, LPCWSTR pszTips);

// 隐藏提示信息
void lyric_wnd_hide_tips(LYRIC_WND_INFO& wnd_info);



// 鼠标悬停在按钮上, 处理一下提示信息
void lyric_wnd_button_hover(LYRIC_WND_INFO& wnd_info)
{
    auto& item = wnd_info.button.rcBtn[wnd_info.button.index];
    LPCWSTR pszTips = nullptr;
    int id = item.id;
    wchar_t buffer[260] = { 0 };

    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_TRANSLATE2:    // 音译按钮
        pszTips = L"点击切换到音译模式";
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL:// 音译按钮, 选中模式
        pszTips = L"点击关闭音译模式";
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE1:    // 翻译按钮
        pszTips = L"点击切换到翻译模式";
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL:// 翻译按钮, 选中模式
        pszTips = L"点击关闭翻译模式";
        break;
    case LYRIC_WND_BUTTON_ID_LRCWRONG:      // 歌词不对
    case LYRIC_WND_BUTTON_ID_LRCWRONG_V:    // 歌词不对, 纵向的按钮图标
        pszTips = L"歌词不对, 打开歌词搜索";
        break;
    case LYRIC_WND_BUTTON_ID_VERTICAL:      // 竖屏按钮
        pszTips = L"切换到竖屏模式";
        break;
    case LYRIC_WND_BUTTON_ID_MAKELRC:       // 制作歌词
        pszTips = L"制作歌词";
        break;
    case LYRIC_WND_BUTTON_ID_FONT_DOWN:     // 字体减小
    case LYRIC_WND_BUTTON_ID_FONT_UP:       // 字体增加
    {
        LPCWSTR fmt = id == LYRIC_WND_BUTTON_ID_FONT_DOWN
            ? L"缩小桌面歌词字体, 当前字体大小: %d"
            : L"放大桌面歌词字体, 当前字体大小: %d";
        swprintf_s(buffer, fmt, wnd_info.lf.lfHeight);
        pszTips = buffer;
        break;
    }
    case LYRIC_WND_BUTTON_ID_BEHIND:        // 歌词延后
    case LYRIC_WND_BUTTON_ID_AHEAD:         // 歌词提前
    {
        if (wnd_info.nTimeOffset == 0)
        {
            // 值是0, 表示没有延时歌词
            pszTips = id == LYRIC_WND_BUTTON_ID_BEHIND
                ? L"歌词延后0.5秒, 当前没有延后"
                : L"歌词提前0.5秒, 当前没有提前";
        }
        else if (wnd_info.nTimeOffset > 0)
        {
            // 延后是负数, 提前是正数
            // 大于0, 那就是歌词提前了
            LPCWSTR fmt = id == LYRIC_WND_BUTTON_ID_BEHIND
                ? L"歌词延后, 当前提前了 %.1f 秒"
                : L"歌词提前, 当前提前了 %.1f 秒";
            double f = (double)wnd_info.nTimeOffset / 1000.0f;
            swprintf_s(buffer, fmt, f);
            pszTips = buffer;
        }
        else
        {
            // 延后是负数, 提前是正数
            // 小于0, 那就是歌词延后了
            LPCWSTR fmt = id == LYRIC_WND_BUTTON_ID_BEHIND
                ? L"歌词延后, 当前延后了 %.1f 秒"
                : L"歌词提前, 当前延后了 %.1f 秒";
            double f = (double)(-wnd_info.nTimeOffset) / 1000.0f;
            swprintf_s(buffer, fmt, f);
            pszTips = buffer;
        }
        break;
    }
    case LYRIC_WND_BUTTON_ID_LOCK:          // 锁定按钮
        pszTips = L"锁定桌面歌词";
        break;
    case LYRIC_WND_BUTTON_ID_SETTING:       // 设置按钮
        pszTips = L"桌面歌词设置";
        break;
    case LYRIC_WND_BUTTON_ID_UNLOCK:        // 解锁按钮
        pszTips = L"解锁桌面歌词";
        break;
    case LYRIC_WND_BUTTON_ID_CLOSE:         // 关闭按钮
        pszTips = L"关闭桌面歌词";
        break;
    case LYRIC_WND_BUTTON_ID_LRCCOLOR:      // 设置字体颜色, 田字的按钮图标
        pszTips = L"设置字体颜色";
        break;
    case LYRIC_WND_BUTTON_ID_MENU:          // 菜单按钮
        pszTips = L"菜单";
        break;
    case LYRIC_WND_BUTTON_ID_HORIZONTAL:    // 横屏按钮
        pszTips = L"横屏模式";
        break;
    case LYRIC_WND_BUTTON_ID_PLAY:          // 播放, 回调函数返回0后会变成暂停按钮
        pszTips = L"播放";
        break;
    case LYRIC_WND_BUTTON_ID_PAUSE:         // 暂停, 回调函数返回0后会变成播放按钮
        pszTips = L"暂停";
        break;
    case LYRIC_WND_BUTTON_ID_PREV:          // 上一首
        pszTips = L"上一首";
        break;
    case LYRIC_WND_BUTTON_ID_NEXT:          // 下一首
        pszTips = L"下一首";
        break;
    default:
        break;
    }
    lyric_wnd_set_tips(wnd_info, pszTips);

}

void lyric_wnd_button_leave(LYRIC_WND_INFO& wnd_info)
{
    lyric_wnd_hide_tips(wnd_info);
}


void lyric_wnd_set_tips(LYRIC_WND_INFO& wnd_info, LPCWSTR pszTips)
{
    TTTOOLINFOW ti = { 0 };
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_IDISHWND;
    ti.hwnd = wnd_info.hWnd;
    ti.uId = (UINT_PTR)wnd_info.hWnd;
    ti.lpszText = (LPWSTR)pszTips;
    auto ret = SendMessageW(wnd_info.hTips, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);

}

void lyric_wnd_hide_tips(LYRIC_WND_INFO& wnd_info)
{
    TTTOOLINFOW ti = { 0 };
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_IDISHWND;
    ti.hwnd = wnd_info.hWnd;
    ti.uId = (UINT_PTR)wnd_info.hWnd;

    SendMessageW(wnd_info.hTips, TTM_POP, 0, 0);
    ti.lpszText = (LPWSTR)L"";
    SendMessageW(wnd_info.hTips, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
}


NAMESPACE_LYRIC_WND_END

