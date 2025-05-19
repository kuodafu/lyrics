#pragma once
#include "lyric_wnd_header.h"


NAMESPACE_LYRIC_WND_BEGIN
// ���ø�ʴ������ݵ�����
void lyric_wnd_set_data(HWND hWnd, PLYRIC_WND_INFU pWndInfo);
// �Ӵ��ڻ�ȡ��ʴ�������
PLYRIC_WND_INFU lyric_wnd_get_data(HWND hWnd);

// ��·���ķ�ʽ�滭����ı�
void lyric_wnd_draw_text_geometry(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_text_info, int nDrawLineIndex);
// �滭��������ĸ���ı�
void lyric_wnd_draw_text_glow(LYRIC_WND_INFU& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);

// �ػ�����, ����滭�ĺ���
HRESULT lyric_wnd_OnPaint(LYRIC_WND_INFU& wnd_info, bool isresize, LYRIC_CALC_STRUCT& arg);

// �滭����ı��ĺ���, ������������ı��滭����
// �滭����/����/����/����/����/˫��/���������ﴦ���
void lyric_wnd_draw_lyric(LYRIC_WND_INFU& wnd_info, LYRIC_CALC_STRUCT& arg);

// �����ʻ滭��λ��, ��Ҫ�ȼ����ʿ��, Ȼ����ݿ�ȼ�����λ��
// ����ݵ�ǰ�Ƿ��Ƿ���/����������λ��, ���ѡ���˷���/����, ����û�з���/����ĸ��, ������Ϊ˫��
// ÿ�λ滭����Ҫ����һ��, �����ʱ��滭һ��, ��ʾ����Ҳ��Ҫ����, ��Ȼ��;�޸Ĵ��ڳߴ�ᵼ��λ�ò���
void lyric_wnd_draw_calc_text_rect(LYRIC_WND_INFU& wnd_info,
                                   LYRIC_WND_DRAWTEXT_INFO& draw_info,
                                   int nDrawLineIndex);


HWND lyric_create_layered_window(const LYRIC_WND_ARG* arg);
void lyric_wnd_default_object(LYRIC_WND_INFU& wnd_info);

// �ø�ʴ���ʧЧ, Ȼ���ػ�
// isUpdate = Ϊtrue��ʱ��ʼ���ػ�, Ϊfalse��ʱ����ж��Ƿ���Ҫ�滭
bool lyric_wnd_invalidate(LYRIC_WND_INFU& wnd_info);

// ���س�����Դ���ͼƬ, Ȼ��Ѹ������궼��¼��
bool lyric_wnd_load_image(LYRIC_WND_INFU& wnd_info);

// �������¼���ͼƬ, ֱ��ʹ���Ѿ����ص�ͼƬ, Ȼ���������ð�ťid
// ���ͼƬû�м���, ���ȼ���ͼƬ
bool lyric_wnd_load_image_recalc(LYRIC_WND_INFU& wnd_info);

// �滭��ʴ�����Ҫ�İ�ť
void lyric_wnd_draw_button(LYRIC_WND_INFU& wnd_info, const RECT& rcWindow);

// ��ʴ����ϵİ�ť�����
void lyric_wnd_button_click(LYRIC_WND_INFU& wnd_info);

// ����ָ���¼�
bool lyric_wnd_call_event(LYRIC_WND_INFU& wnd_info, int id);

bool lyric_wnd_set_btn_state(LYRIC_WND_INFU& wnd_info, int id, LYRIC_WND_BUTTON_STATE state);
LYRIC_WND_BUTTON_STATE lyric_wnd_get_btn_state(LYRIC_WND_INFU& wnd_info, int id);

// ����ƶ�����ť��, ��ʾ��ʾ��Ϣ
void lyric_wnd_button_hover(LYRIC_WND_INFU& wnd_info);
// ����뿪��ť, ������ʾ��Ϣ
void lyric_wnd_button_leave(LYRIC_WND_INFU& wnd_info);

// ������Ҫ�滭�İ�ť���ܿ��, ����ÿ�Ⱥ���������а�ť����
int lyric_wnd_calc_button(LYRIC_WND_INFU& wnd_info, int& maxWidth, int& maxHeight, int offset);

// �����ʴ��ڵĸ߶�, �������ڵĸ߶�, ����ǰ�����Ѿ���ʼ����Ĭ�ϰ�ť, ��������
// �߶����ð�ť�ĸ߶� + ���ռ��һ�и߶� * 2 + �߾� �õ���
void lyric_wnd_calc_height(LYRIC_WND_INFU& wnd_info);

// �����ı�����, �����ı�, ���л滭�ı�ʱʹ��, ͳһ�����ı����
// ����ͳһ���ǲ����е��ı�����, ��/������
IDWriteTextLayout* lyric_wnd_create_text_layout(LPCWSTR str, int len, IDWriteTextFormat* dxFormat, float layoutWidth, float layoutHeight);

NAMESPACE_D2D::CD2DImage* __shadow_image(NAMESPACE_D2D::CD2DRender& d2dRender);


NAMESPACE_LYRIC_WND_END