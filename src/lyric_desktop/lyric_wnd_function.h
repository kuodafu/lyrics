#pragma once
#include "lyric_wnd_header.h"
#include "_lyric_desktop_draw_header.h"


NAMESPACE_LYRIC_DESKTOP_BEGIN

extern KUODAFU_NAMESPACE::D2DInterface* g_d2d_interface;

float _lyric_wnd_load_krc_calc_text(PLYRIC_DESKTOP_INFO pWndInfo, IDWriteTextLayout* pTextLayout, float* pHeight);

// 设置歌词窗口数据到窗口
void lyric_wnd_set_data(HWND hWnd, PLYRIC_DESKTOP_INFO pWndInfo);
// 从窗口获取歌词窗口数据
PLYRIC_DESKTOP_INFO lyric_wnd_get_data(HWND hWnd);

// 初始化桌面窗口
bool _ld_init();
bool _ld_uninit();

// 创建歌词窗口, 创建的是分层窗口, 是支持透明的, 失败返回空指针
PLYRIC_DESKTOP_INFO _ld_create_layered_window(const LYRIC_DESKTOP_ARG* arg);

// 给窗口增加一个高精度定时器, 用于刷新歌词显示
void _ld_start_high_precision_timer(PLYRIC_DESKTOP_INFO pWndInfo);

void lyric_wnd_default_object(LYRIC_DESKTOP_INFO& wnd_info);


// 歌词窗口上的按钮被点击
void lyric_wnd_button_click(LYRIC_DESKTOP_INFO& wnd_info);

// 调用指定事件
bool lyric_wnd_call_evt(LYRIC_DESKTOP_INFO& wnd_info, int id);

bool lyric_wnd_set_btn_state(LYRIC_DESKTOP_INFO& wnd_info, int id, LYRIC_DESKTOP_BUTTON_STATE state);
LYRIC_DESKTOP_BUTTON_STATE lyric_wnd_get_btn_state(LYRIC_DESKTOP_INFO& wnd_info, int id);

// 鼠标移动到按钮上, 显示提示信息
void lyric_wnd_button_hover(LYRIC_DESKTOP_INFO& wnd_info);
// 鼠标离开按钮, 隐藏提示信息
void lyric_wnd_button_leave(LYRIC_DESKTOP_INFO& wnd_info);


// 重新设置翻译/音译按钮状态
int lyric_wnd_set_state_translate(LYRIC_DESKTOP_INFO& wnd_info, int language);


LPBYTE _lrc_dwsktop_get_shadow_image(size_t& size);


NAMESPACE_LYRIC_DESKTOP_END