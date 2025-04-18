#include "lyric_wnd_header.h"
using namespace NAMESPACE_D2D;


NAMESPACE_LYRIC_WND_BEGIN
// 通过id获取结构地址, 失败返回空指针
LYRIC_WND_BUTTON_INFO* lrc_click_get_item(LYRIC_WND_INFU& wnd_info, int id);


void lrc_click_wrong(LYRIC_WND_INFU& wnd_info, int id);     // 歌词不对事件, 应该是要弹出个窗口搜索歌词
bool lrc_click_translate(LYRIC_WND_INFU& wnd_info, int id); // 翻译相关按钮点击事件
void lrc_click_vmode(LYRIC_WND_INFU& wnd_info, int id);     // 切换到竖屏模式
void lrc_click_hmode(LYRIC_WND_INFU& wnd_info, int id);     // 切换到横屏模式
void lrc_click_makelrc(LYRIC_WND_INFU& wnd_info, int id);   // 弹出制作歌词窗口
void lrc_click_font(LYRIC_WND_INFU& wnd_info, int id);      // 字体放大缩小
void lrc_click_lrc_ms(LYRIC_WND_INFU& wnd_info, int id);    // 歌词延后/提前
void lrc_click_lock_un(LYRIC_WND_INFU& wnd_info, int id);   // 锁定/解锁
void lrc_click_setting(LYRIC_WND_INFU& wnd_info, int id);   // 弹出设置窗口
void lrc_click_close(LYRIC_WND_INFU& wnd_info, int id);     // 关闭窗口
void lrc_click_lrc_color(LYRIC_WND_INFU& wnd_info, int id); // 设置字体颜色
void lrc_click_menu(LYRIC_WND_INFU& wnd_info, int id);      // 菜单按钮
bool lrc_click_play(LYRIC_WND_INFU& wnd_info, int id);      // 播放按钮相关事件, 播放, 暂停, 上一首, 下一首, 这里只能处理切换播放/暂停按钮样式

void lyric_wnd_button_click(LYRIC_WND_INFU& wnd_info)
{
    auto& item = wnd_info.button.rcBtn[wnd_info.button.indexDown];
    int id = item.id;

    int r = 0;
    if (wnd_info.pfnCommand)
        r = wnd_info.pfnCommand(wnd_info.hWnd, id, wnd_info.lParam);
    if (r != 0)
        return; // 返回值不是0, 表示要拦截事件, 不处理

    lyric_wnd_call_event(wnd_info, id);
    //wchar_t buf[100];
    //swprintf_s(buf, L"按钮%d被按下了, id = %d\n", wnd_info.button.indexDown, id);
    //OutputDebugStringW(buf);
}

bool lyric_wnd_call_event(LYRIC_WND_INFU& wnd_info, int id)
{
    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_TRANSLATE2:    // 音译按钮
    case LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL:// 音译按钮, 选中模式
    case LYRIC_WND_BUTTON_ID_TRANSLATE1:    // 翻译按钮
    case LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL:// 翻译按钮, 选中模式
        return lrc_click_translate(wnd_info, id);

    case LYRIC_WND_BUTTON_ID_LRCWRONG:      // 歌词不对
    case LYRIC_WND_BUTTON_ID_LRCWRONG_V:    // 歌词不对, 纵向的按钮图标
        lrc_click_wrong(wnd_info, id);  // 不管横向纵向处理是完全一样的
        break;
    case LYRIC_WND_BUTTON_ID_VERTICAL:      // 竖屏按钮
        lrc_click_vmode(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_HORIZONTAL:    // 横屏按钮
        lrc_click_hmode(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_MAKELRC:       // 制作歌词
        lrc_click_makelrc(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_FONT_DOWN:     // 字体减小
    case LYRIC_WND_BUTTON_ID_FONT_UP:       // 字体增加
        lrc_click_font(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_BEHIND:        // 歌词延后
    case LYRIC_WND_BUTTON_ID_AHEAD:         // 歌词提前
        lrc_click_lrc_ms(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_LOCK:          // 锁定按钮
    case LYRIC_WND_BUTTON_ID_UNLOCK:        // 解锁按钮
        lrc_click_lock_un(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_SETTING:       // 设置按钮
        lrc_click_setting(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_CLOSE:         // 关闭按钮
    case LYRIC_WND_BUTTON_ID_SHOW:          // 显示歌词, 这个没有按钮, 纯外部触发
        lrc_click_close(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_LRCCOLOR:      // 设置字体颜色, 田字的按钮图标
        lrc_click_lrc_color(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_MENU:          // 菜单按钮
        lrc_click_menu(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_PLAY:          // 播放, 回调函数返回0后会变成暂停按钮
    case LYRIC_WND_BUTTON_ID_PAUSE:         // 暂停, 回调函数返回0后会变成播放按钮
    case LYRIC_WND_BUTTON_ID_PREV:          // 上一首
    case LYRIC_WND_BUTTON_ID_NEXT:          // 下一首
        lrc_click_play(wnd_info, id);
        break;
    default:
        return false;
    }
    return true;
}

bool lyric_wnd_set_btn_state(LYRIC_WND_INFU& wnd_info, int id, LYRIC_WND_BUTTON_STATE state)
{
    auto* pItem = lrc_click_get_item(wnd_info, id);
    if (!pItem)
        return false;
    auto& item = *pItem;
    item.state = state;
    return true;
}

LYRIC_WND_BUTTON_STATE lyric_wnd_get_btn_state(LYRIC_WND_INFU& wnd_info, int id)
{
    auto* pItem = lrc_click_get_item(wnd_info, id);
    if (!pItem)
        return LYRIC_WND_BUTTON_STATE_ERROR;
    auto& item = *pItem;
    return (LYRIC_WND_BUTTON_STATE)item.state;
}


LYRIC_WND_BUTTON_INFO* lrc_click_get_item(LYRIC_WND_INFU& wnd_info, int id)
{
    // 去枚举数组找到对应的按钮
    for (auto& item : wnd_info.button.rcBtn)
    {
        if (item.id == id)
            return &item;
    }
    return nullptr;
}

void lrc_click_wrong(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"这里需要弹出一个搜索歌词的窗口, 应该由外部去实现", L"提示", MB_OK);
}

bool lrc_click_translate(LYRIC_WND_INFU& wnd_info, int id)
{
    LYRIC_WND_BUTTON_INFO* pItem = lrc_click_get_item(wnd_info, id);
    if (pItem == nullptr)
        return false;

    auto& item = *pItem;

    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_TRANSLATE2:    // 音译按钮
        item.id = LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL;
        // 需要切换到 上面显示歌词文本, 下面显示音译文本的模式, 两个都居中对齐
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL:// 音译按钮, 选中模式
        item.id = LYRIC_WND_BUTTON_ID_TRANSLATE2;
        // 切换回原来的歌词模式
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE1:    // 翻译按钮
        item.id = LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL;
        // 需要切换到 上面显示歌词文本, 下面显示翻译文本的模式, 两个都居中对齐
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL:// 翻译按钮, 选中模式
        item.id = LYRIC_WND_BUTTON_ID_TRANSLATE1;
        // 切换回原来的歌词模式
        break;
    default:
        return false;
    }
    return true;
}

void lrc_click_vmode(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"切换到竖屏模式", L"提示", MB_OK);
}

void lrc_click_hmode(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"切换到横屏模式", L"提示", MB_OK);
}

void lrc_click_makelrc(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"弹出制作歌词窗口", L"提示", MB_OK);

}

void lrc_click_font(LYRIC_WND_INFU& wnd_info, int id)
{
    // 调整字体尺寸, 还要根据字体尺寸然后调整窗口尺寸, 然后重新创建字体
    int size = id == LYRIC_WND_BUTTON_ID_FONT_DOWN ? -2 : 2;

    wnd_info.lf.lfHeight += size;
    wnd_info.dx.re_create_font(&wnd_info);      // 调整尺寸后重新创建
    lyric_re_calc_text_width(wnd_info.hLyric);  // 重新计算歌词宽度
    wnd_info.change_btn = true;                 // 标记按钮需要重新绘画
}

void lrc_click_lrc_ms(LYRIC_WND_INFU& wnd_info, int id)
{
    wnd_info.nTimeOffset += (id == LYRIC_WND_BUTTON_ID_BEHIND ? -500 : 500);
    wnd_info.change_btn = true;   // 标记按钮需要重新绘画
}

void lrc_click_lock_un(LYRIC_WND_INFU& wnd_info, int id)
{
    DWORD styleEx = (DWORD)GetWindowLongPtrW(wnd_info.hWnd, GWL_EXSTYLE);
    wnd_info.isLock = (id == LYRIC_WND_BUTTON_ID_LOCK);
    if (wnd_info.isLock)
        SetWindowLongPtrW(wnd_info.hWnd, GWL_EXSTYLE, styleEx | WS_EX_TRANSPARENT);
    else
        SetWindowLongPtrW(wnd_info.hWnd, GWL_EXSTYLE, styleEx & ~WS_EX_TRANSPARENT);
    wnd_info.change_btn = true; // 标记按钮需要重新绘画
}

void lrc_click_setting(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"弹出设置窗口", L"提示", MB_OK);
}

void lrc_click_close(LYRIC_WND_INFU& wnd_info, int id)
{
    if (id == LYRIC_WND_BUTTON_ID_SHOW)
        SetWindowPos(wnd_info.hWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    else
        SetWindowPos(wnd_info.hWnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
}

void lrc_click_lrc_color(LYRIC_WND_INFU& wnd_info, int id)
{
    // 这里酷狗使用的是点击后弹出选择配色的菜单, 选择后歌词颜色就变了
    // 内部定义几个配色, 点击按钮后切换到对应的颜色, 需要设置画刷颜色
}

void lrc_click_menu(LYRIC_WND_INFU& wnd_info, int id)
{
    // 目前暂时没有使用

}

bool lrc_click_play(LYRIC_WND_INFU& wnd_info, int id)
{
    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_PLAY:          // 播放, 回调函数返回0后会变成暂停按钮
    case LYRIC_WND_BUTTON_ID_PAUSE:         // 暂停, 回调函数返回0后会变成播放按钮
    {
        auto* pItem = lrc_click_get_item(wnd_info, id);
        if (!pItem)
            return false;
        auto& item = *pItem;

        // 播放, 这个内部处理不了, 这里只能换个id
        if (item.id == LYRIC_WND_BUTTON_ID_PLAY)
            item.id = LYRIC_WND_BUTTON_ID_PAUSE;    // 是备份事件就把按钮换成暂停
        else
            item.id = LYRIC_WND_BUTTON_ID_PLAY;     // 是暂停事件就把按钮换成播放

        wnd_info.change_btn = true; // 标记按钮需要重新绘画
        break;
    }
    case LYRIC_WND_BUTTON_ID_PREV:          // 上一首
    case LYRIC_WND_BUTTON_ID_NEXT:          // 下一首
        break;
    default:
        return false;
    }
    return true;
}

NAMESPACE_LYRIC_WND_END
