#pragma once
#include "lyric_wnd_header.h"
#include "_lyric_desktop_draw_header.h"


NAMESPACE_LYRIC_DESKTOP_BEGIN

extern KUODAFU_NAMESPACE::D2DInterface* g_d2d_interface;

float _lyric_wnd_load_krc_calc_text(PLYRIC_DESKTOP_INFO pWndInfo, IDWriteTextLayout* pTextLayout, float* pHeight);

// ���ø�ʴ������ݵ�����
void lyric_wnd_set_data(HWND hWnd, PLYRIC_DESKTOP_INFO pWndInfo);
// �Ӵ��ڻ�ȡ��ʴ�������
PLYRIC_DESKTOP_INFO lyric_wnd_get_data(HWND hWnd);

// ��ʼ�����洰��
bool _ld_init();
bool _ld_uninit();

// ������ʴ���, �������Ƿֲ㴰��, ��֧��͸����, ʧ�ܷ��ؿ�ָ��
PLYRIC_DESKTOP_INFO _ld_create_layered_window(const LYRIC_DESKTOP_ARG* arg);

// ����������һ���߾��ȶ�ʱ��, ����ˢ�¸����ʾ
void _ld_start_high_precision_timer(PLYRIC_DESKTOP_INFO pWndInfo);

void lyric_wnd_default_object(LYRIC_DESKTOP_INFO& wnd_info);


// ��ʴ����ϵİ�ť�����
void lyric_wnd_button_click(LYRIC_DESKTOP_INFO& wnd_info);

// ����ָ���¼�
bool lyric_wnd_call_evt(LYRIC_DESKTOP_INFO& wnd_info, int id);

bool lyric_wnd_set_btn_state(LYRIC_DESKTOP_INFO& wnd_info, int id, LYRIC_DESKTOP_BUTTON_STATE state);
LYRIC_DESKTOP_BUTTON_STATE lyric_wnd_get_btn_state(LYRIC_DESKTOP_INFO& wnd_info, int id);

// ����ƶ�����ť��, ��ʾ��ʾ��Ϣ
void lyric_wnd_button_hover(LYRIC_DESKTOP_INFO& wnd_info);
// ����뿪��ť, ������ʾ��Ϣ
void lyric_wnd_button_leave(LYRIC_DESKTOP_INFO& wnd_info);


// �������÷���/���밴ť״̬
int lyric_wnd_set_state_translate(LYRIC_DESKTOP_INFO& wnd_info, int language);


LPBYTE _lrc_dwsktop_get_shadow_image(size_t& size);


NAMESPACE_LYRIC_DESKTOP_END