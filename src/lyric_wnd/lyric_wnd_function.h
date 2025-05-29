#pragma once
#include "lyric_wnd_header.h"


NAMESPACE_LYRIC_WND_BEGIN
// ���ø�ʴ������ݵ�����
void lyric_wnd_set_data(HWND hWnd, PLYRIC_WND_INFO pWndInfo);
// �Ӵ��ڻ�ȡ��ʴ�������
PLYRIC_WND_INFO lyric_wnd_get_data(HWND hWnd);

// ��·���ķ�ʽ�滭����ı�
void lyric_wnd_draw_text_geometry(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_text_info, int nDrawLineIndex);
// �滭��������ĸ���ı�
void lyric_wnd_draw_text_glow(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, int nDrawLineIndex);

// �ػ�����, ����滭�ĺ���
HRESULT lyric_wnd_OnPaint(LYRIC_WND_INFO& wnd_info, bool isresize, LYRIC_CALC_STRUCT& arg);

// �滭����ı��ĺ���, ������������ı��滭����
// �滭����/����/����/����/����/˫��/���������ﴦ���
void lyric_wnd_draw_lyric(LYRIC_WND_INFO& wnd_info, LYRIC_CALC_STRUCT& arg);

// �����ʻ滭��λ��, ��Ҫ�ȼ����ʿ��, Ȼ����ݿ�ȼ�����λ��
// ����ݵ�ǰ�Ƿ��Ƿ���/����������λ��, ���ѡ���˷���/����, ����û�з���/����ĸ��, ������Ϊ˫��
// ÿ�λ滭����Ҫ����һ��, �����ʱ��滭һ��, ��ʾ����Ҳ��Ҫ����, ��Ȼ��;�޸Ĵ��ڳߴ�ᵼ��λ�ò���
void lyric_wnd_draw_calc_text_rect(LYRIC_WND_INFO& wnd_info,
                                   LYRIC_WND_DRAWTEXT_INFO& draw_info,
                                   int nDrawLineIndex);

// �滭�������
void lyric_wnd_draw_cache_text_v(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, ID2D1Bitmap* pBitmap);
// �滭�������
void lyric_wnd_draw_cache_text_h(LYRIC_WND_INFO& wnd_info, LYRIC_WND_DRAWTEXT_INFO& draw_info, ID2D1Bitmap* pBitmap);



HWND lyric_create_layered_window(const LYRIC_WND_ARG* arg);
void lyric_wnd_default_object(LYRIC_WND_INFO& wnd_info);

// �ø�ʴ���ʧЧ, Ȼ���ػ�
// isUpdate = Ϊtrue��ʱ��ʼ���ػ�, Ϊfalse��ʱ����ж��Ƿ���Ҫ�滭
bool lyric_wnd_invalidate(LYRIC_WND_INFO& wnd_info);

// ���س�����Դ���ͼƬ, Ȼ��Ѹ������궼��¼��
bool lyric_wnd_load_image(LYRIC_WND_INFO& wnd_info);

// �������¼���ͼƬ, ֱ��ʹ���Ѿ����ص�ͼƬ, Ȼ���������ð�ťid
// ���ͼƬû�м���, ���ȼ���ͼƬ
bool lyric_wnd_load_image_recalc(LYRIC_WND_INFO& wnd_info);

// �滭��ʴ�����Ҫ�İ�ť
void lyric_wnd_draw_button(LYRIC_WND_INFO& wnd_info);

// ��ʴ����ϵİ�ť�����
void lyric_wnd_button_click(LYRIC_WND_INFO& wnd_info);

// ����ָ���¼�
bool lyric_wnd_call_event(LYRIC_WND_INFO& wnd_info, int id);

bool lyric_wnd_set_btn_state(LYRIC_WND_INFO& wnd_info, int id, LYRIC_WND_BUTTON_STATE state);
LYRIC_WND_BUTTON_STATE lyric_wnd_get_btn_state(LYRIC_WND_INFO& wnd_info, int id);

// ����ƶ�����ť��, ��ʾ��ʾ��Ϣ
void lyric_wnd_button_hover(LYRIC_WND_INFO& wnd_info);
// ����뿪��ť, ������ʾ��Ϣ
void lyric_wnd_button_leave(LYRIC_WND_INFO& wnd_info);

// ���㰴ť�滭��Ҫ��λ��
void lyric_wnd_calc_btn_pos(LYRIC_WND_INFO& wnd_info);

// �������÷���/���밴ť״̬
int lyric_wnd_set_state_translate(LYRIC_WND_INFO& wnd_info, int language);

// �����ʴ��ڵĸ߶�, �������ڵĸ߶�, ����ǰ�����Ѿ���ʼ����Ĭ�ϰ�ť, ��������
// �߶����ð�ť�ĸ߶� + ���ռ��һ�и߶� * 2 + �߾� �õ���
void lyric_wnd_calc_wnd_pos(LYRIC_WND_INFO& wnd_info, bool isMoveWindow);

// �����ı�����, �����ı�, ���л滭�ı�ʱʹ��, ͳһ�����ı����
// ����ͳһ���ǲ����е��ı�����, ��/������
bool lyric_wnd_create_text_layout(LPCWSTR str, int len, IDWriteTextFormat* dxFormat, float layoutWidth, float layoutHeight, IDWriteTextLayout** ppTextLayout);

NAMESPACE_D2D::CD2DImage* __shadow_image(NAMESPACE_D2D::CD2DRender& d2dRender);

bool isLatinCharacter(wchar_t ch);

NAMESPACE_LYRIC_WND_END