#pragma once
#include "lyric_wnd_header.h"


NAMESPACE_LYRIC_WND_BEGIN
// 设置歌词窗口数据到窗口
void lyric_wnd_set_data(HWND hWnd, PLYRIC_WND_INFU pWndInfo);
// 从窗口获取歌词窗口数据
PLYRIC_WND_INFU lyric_wnd_get_data(HWND hWnd);

// 用路径的方式绘画歌词文本
void lyric_wnd_draw_text_geometry(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_text_info, int nDrawLineIndex);
// 绘画发光字体的歌词文本
void lyric_wnd_draw_text_glow(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);

// 重画函数, 处理绘画的函数
HRESULT lyric_wnd_OnPaint(LYRIC_WND_INFU& wnd_info, bool isresize, LYRIC_CALC_STRUCT& arg);

// 绘画歌词文本的函数, 在这个函数把文本绘画出来
// 绘画翻译/音译/横屏/竖屏/单行/双行/都是在这里处理的
void lyric_wnd_draw_lyric(LYRIC_WND_INFU& wnd_info, LYRIC_CALC_STRUCT& arg);

// 计算歌词绘画的位置, 需要先计算歌词宽度, 然后根据宽度计算歌词位置
// 会根据当前是否是翻译/音译来计算位置, 如果选择了翻译/音译, 但是没有翻译/音译的歌词, 会设置为双行
// 每次绘画都需要计算一次, 缓存的时候绘画一次, 显示出来也需要计算, 不然中途修改窗口尺寸会导致位置不对
void lyric_wnd_draw_calc_text_rect(LYRIC_WND_INFU& wnd_info,
                                   LYRIC_WND_DRAWTEXT_INFO& draw_info,
                                   int nDrawLineIndex);


HWND lyric_create_layered_window(const LYRIC_WND_ARG* arg);
void lyric_wnd_default_object(LYRIC_WND_INFU& wnd_info);

// 让歌词窗口失效, 然后重画
// isUpdate = 为true的时候始终重画, 为false的时候会判断是否需要绘画
bool lyric_wnd_invalidate(LYRIC_WND_INFU& wnd_info);

// 加载程序资源里的图片, 然后把各个坐标都记录好
bool lyric_wnd_load_image(LYRIC_WND_INFU& wnd_info);

// 不用重新加载图片, 直接使用已经加载的图片, 然后重新设置按钮id
// 如果图片没有加载, 会先加载图片
bool lyric_wnd_load_image_recalc(LYRIC_WND_INFU& wnd_info);

// 绘画歌词窗口需要的按钮
void lyric_wnd_draw_button(LYRIC_WND_INFU& wnd_info, const RECT& rcWindow);

// 歌词窗口上的按钮被点击
void lyric_wnd_button_click(LYRIC_WND_INFU& wnd_info);

// 调用指定事件
bool lyric_wnd_call_event(LYRIC_WND_INFU& wnd_info, int id);

bool lyric_wnd_set_btn_state(LYRIC_WND_INFU& wnd_info, int id, LYRIC_WND_BUTTON_STATE state);
LYRIC_WND_BUTTON_STATE lyric_wnd_get_btn_state(LYRIC_WND_INFU& wnd_info, int id);

// 鼠标移动到按钮上, 显示提示信息
void lyric_wnd_button_hover(LYRIC_WND_INFU& wnd_info);
// 鼠标离开按钮, 隐藏提示信息
void lyric_wnd_button_leave(LYRIC_WND_INFU& wnd_info);

// 计算需要绘画的按钮的总宽度, 计算好宽度后可以让所有按钮居中
int lyric_wnd_calc_button(LYRIC_WND_INFU& wnd_info, int& maxWidth, int& maxHeight, int offset);

// 计算歌词窗口的高度, 整个窗口的高度, 调用前必须已经初始化了默认按钮, 还有字体
// 高度是用按钮的高度 + 歌词占用一行高度 * 2 + 边距 得到的
void lyric_wnd_calc_height(LYRIC_WND_INFU& wnd_info);

// 创建文本布局, 计算文本, 还有绘画文本时使用, 统一设置文本间距
// 这里统一都是不换行的文本布局, 左/顶对齐
IDWriteTextLayout* lyric_wnd_create_text_layout(LPCWSTR str, int len, IDWriteTextFormat* dxFormat, float layoutWidth, float layoutHeight);

NAMESPACE_D2D::CD2DImage* __shadow_image(NAMESPACE_D2D::CD2DRender& d2dRender);


NAMESPACE_LYRIC_WND_END