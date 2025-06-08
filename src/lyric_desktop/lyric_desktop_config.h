#pragma once
#include "lyric_desktop_typedef.h"

NAMESPACE_LYRIC_DESKTOP_BEGIN

// 窗口位置信息, 横屏和竖屏使用的位置不一样
struct LYRIC_DESKTOP_POS : RECT
{
    int width;
    int height;
};

// 配置信息, 调试使用, 开启或者关闭一些功能, 方便查看效果
typedef struct LYRIC_DESKTOP_CONFIG_DEBUG
{
    DWORD       clrTextBackNormal;  // 普通歌词文本背景颜色
    DWORD       clrTextBackLight;   // 高亮歌词文本背景颜色

    bool        alwaysFillBack;     // 始终填充背景
    bool        alwaysDraw;         // 始终绘画, 即使歌词内容没有变化
    bool        alwaysCache;        // 始终创建缓存, 即使歌词内容没有变化
    bool        alwaysCache1;       // 始终创建缓存, 即使歌词内容没有变化

    LYRIC_DESKTOP_CONFIG_DEBUG()
    {
        clrTextBackNormal = 0;
        clrTextBackLight = 0;
        alwaysFillBack = false;
        alwaysDraw = false;
        alwaysCache = false;
        alwaysCache1 = false;
    }

}*PLYRIC_DESKTOP_CONFIG_DEBUG;

// 配置信息, 所有用到的配置, 可以设置的, 都写在这里, 到时候生成一个json, 让外部保存
typedef struct LYRIC_DESKTOP_CONFIG
{
    int                 refreshRate;    // 刷新率, 刷新歌词的频率, 主流的刷新率是 30, 60, 75, 90, 100, 120, 144, 165, 240
    LPCWSTR             pszDefText;     // 没有歌词时的默认文本, 可以做广告之类的
    int                 nDefText;       // 默认文本长度
    float               padding_text_;  // 歌词4个边的间距, 原始值, 下面这两个成员的缩放后的值
    float               padding_wnd_;   // 窗口4个边的间距, 原始值, 下面这两个成员的缩放后的值
    float               padding_text;   // 歌词4个边的间距, 这个边就是预留给发光/阴影 超出的范围
    float               padding_wnd;    // 窗口4个边的间距, 这个范围留空, 不让内容绘画到窗口边上
    LYRIC_MODE          mode;           // 歌词显示模式, LYRIC_MODE 枚举类型

    std::wstring        pszFontName;    // 字体名称
    int                 nFontSize;      // 字体尺寸, 这个值是没有缩放, 创建字体的时候会缩放
    int                 lfWeight;       // 400=正常, 700=粗体
    int                 nLineSpace;     // 行距, 单位是像素

    LYRIC_DESKTOP_POS   pos_h;          // 横屏模式下的窗口位置
    LYRIC_DESKTOP_POS   pos_v;          // 竖屏模式下的窗口位置

    std::vector<DWORD>  clrNormal;      // 普通歌词画刷颜色组
    std::vector<DWORD>  clrLight;       // 高亮歌词画刷颜色组
    DWORD               clrBorder;      // 歌词文本边框颜色
    DWORD               clrWndBack;     // 鼠标移动上来之后显示的歌词ARGB背景颜色
    DWORD               clrWndBorder;   // 鼠标移动上来之后显示的歌词ARGB边框颜色

    LYRIC_DESKTOP_CONFIG_DEBUG debug;   // 调试配置信息
    LYRIC_DESKTOP_CONFIG() { default_config(); }
    void default_config();
}*PLYRIC_DESKTOP_CONFIG;

NAMESPACE_LYRIC_DESKTOP_END

