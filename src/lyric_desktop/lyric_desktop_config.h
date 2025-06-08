#pragma once
#include "lyric_desktop_typedef.h"

NAMESPACE_LYRIC_DESKTOP_BEGIN


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
    bool                bVertical;      // 是否竖屏模式
    bool                bSingleLine;    // 是否单行显示
    bool                bSelfy;         // 翻译按钮是否被选中, 和音译按钮互斥, 两个都为true时, 显示翻译
    bool                bSelyy;         // 音译按钮是否被选中, 和翻译按钮互斥

    float               padding_text_;  // 歌词4个边的间距, 原始值, 下面这两个成员的缩放后的值
    float               padding_wnd_;   // 窗口4个边的间距, 原始值, 下面这两个成员的缩放后的值
    float               padding_text;   // 歌词4个边的间距, 这个边就是预留给发光/阴影 超出的范围
    float               padding_wnd;    // 窗口4个边的间距, 这个范围留空, 不让内容绘画到窗口边上
    float               strokeWidth;    // 文本边框的宽度
    float               strokeWidth_div;// 通过字体大小除以这个值来得到边框宽度, 有这个值优先使用
    bool                fillBeforeDraw; // 是否先填充后描边, 这个是绘画歌词文本那处理
    int                 nLineSpace;     // 行距, 单位是像素

    int                 line1_align;    // 第一行歌词文本对齐方式
    int                 line2_align;    // 第二行歌词文本对齐方式

    std::wstring        szDefText;      // 没有歌词时的默认文本, 可以做广告之类的
    std::wstring        szFontName;     // 字体名称
    int                 nFontSize;      // 字体尺寸, 这个值是没有缩放, 创建字体的时候会缩放
    int                 lfWeight;       // 400=正常, 700=粗体

    RECT                rect_h;         // 横屏模式下的窗口位置
    RECT                rect_v;         // 竖屏模式下的窗口位置

    std::vector<DWORD>  clrNormal;      // 普通歌词画刷颜色组
    std::vector<DWORD>  clrLight;       // 高亮歌词画刷颜色组

    DWORD               clrBorderNormal;// 歌词普通文本边框颜色
    DWORD               clrBorderLight; // 歌词高亮文本边框颜色
    DWORD               clrWndBack;     // 鼠标移动上来之后显示的歌词ARGB背景颜色
    DWORD               clrWndBorder;   // 鼠标移动上来之后显示的歌词ARGB边框颜色
    DWORD               clrLine;        // 鼠标移动上来之后显示的歌词ARGB边框颜色

    std::vector<float>  clrNormal_GradientStop; // 普通歌词画刷颜色组
    std::vector<float>  clrLight_GradientStop;  // 普通歌词画刷颜色组

    LYRIC_DESKTOP_CONFIG_DEBUG debug;   // 调试配置信息

    // 设置默认配置, 这个结构初始化的时候没有给成员赋值, 调用这个方法后才赋值
    // 一般是在窗口创建之后调用, 然后通过DPI来计算缩放
    void init();

    // 从json里解析配置信息
    int parse(const char* pszJson, LYRIC_DESKTOP_INFO* pWndInfo);

    // 把配置信息转成json文本
    char* to_json(LYRIC_DESKTOP_INFO* pWndInfo) const;

}*PLYRIC_DESKTOP_CONFIG;

NAMESPACE_LYRIC_DESKTOP_END

