#pragma once
#include "lyric_desktop_header.h"


NAMESPACE_LYRIC_DESKTOP_BEGIN


// 用路径的方式绘画歌词文本
void lyric_wnd_draw_text_geometry(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_text_info, int nDrawLineIndex);
// 绘画发光字体的歌词文本
void lyric_wnd_draw_text_glow(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);

// 重画函数, 处理绘画的函数
HRESULT lyric_wnd_OnPaint(LYRIC_DESKTOP_INFO& wnd_info, bool isresize, LYRIC_CALC_STRUCT& arg);

// 绘画歌词文本的函数, 在这个函数把文本绘画出来
// 绘画翻译/音译/横屏/竖屏/单行/双行/都是在这里处理的
void lyric_wnd_draw_lyric(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_CALC_STRUCT& arg);

// 计算歌词绘画的位置, 需要先计算歌词宽度, 然后根据宽度计算歌词位置
// 会根据当前是否是翻译/音译来计算位置, 如果选择了翻译/音译, 但是没有翻译/音译的歌词, 会设置为双行
// 每次绘画都需要计算一次, 缓存的时候绘画一次, 显示出来也需要计算, 不然中途修改窗口尺寸会导致位置不对
void lyric_wnd_draw_calc_text_rect(LYRIC_DESKTOP_INFO& wnd_info,
                                   LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info,
                                   int nDrawLineIndex);

// 绘画竖屏歌词
void lyric_wnd_draw_cache_text_v(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, ID2D1Bitmap* pBitmap);
// 绘画横屏歌词
void lyric_wnd_draw_cache_text_h(LYRIC_DESKTOP_INFO& wnd_info, LYRIC_DESKTOP_DRAWTEXT_INFO& draw_info, ID2D1Bitmap* pBitmap);


// 让歌词窗口失效, 然后重画
// isUpdate = 为true的时候始终重画, 为false的时候会判断是否需要绘画
bool lyric_wnd_invalidate(LYRIC_DESKTOP_INFO& wnd_info);

// 加载程序资源里的图片, 然后把各个坐标都记录好
bool lyric_wnd_load_image(LYRIC_DESKTOP_INFO& wnd_info);

// 不用重新加载图片, 直接使用已经加载的图片, 然后重新设置按钮id
// 如果图片没有加载, 会先加载图片
bool lyric_wnd_load_image_recalc(LYRIC_DESKTOP_INFO& wnd_info);

// 绘画歌词窗口需要的按钮
void lyric_wnd_draw_button(LYRIC_DESKTOP_INFO& wnd_info);

// 计算按钮绘画需要的位置
void lyric_wnd_calc_btn_pos(LYRIC_DESKTOP_INFO& wnd_info);


// 计算歌词窗口的高度, 整个窗口的高度, 调用前必须已经初始化了默认按钮, 还有字体
// 高度是用按钮的高度 + 歌词占用一行高度 * 2 + 边距 得到的
void lyric_wnd_calc_wnd_pos(LYRIC_DESKTOP_INFO& wnd_info, bool isMoveWindow);

// 创建文本布局, 计算文本, 还有绘画文本时使用, 统一设置文本间距
// 这里统一都是不换行的文本布局, 左/顶对齐
bool lyric_wnd_create_text_layout(LPCWSTR str, int len, IDWriteTextFormat* dxFormat, float layoutWidth, float layoutHeight, IDWriteTextLayout** ppTextLayout);

bool isLatinCharacter(wchar_t ch);

NAMESPACE_LYRIC_DESKTOP_END