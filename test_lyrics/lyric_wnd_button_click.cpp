#include "lyric_wnd_header.h"
using namespace NAMESPACE_D2D;


NAMESPACE_LYRIC_WND_BEGIN
// ͨ��id��ȡ�ṹ��ַ, ʧ�ܷ��ؿ�ָ��
LYRIC_WND_BUTTON_INFO* lrc_click_get_item(LYRIC_WND_INFU& wnd_info, int id);


void lrc_click_wrong(LYRIC_WND_INFU& wnd_info, int id);     // ��ʲ����¼�, Ӧ����Ҫ�����������������
bool lrc_click_translate(LYRIC_WND_INFU& wnd_info, int id); // ������ذ�ť����¼�
void lrc_click_vmode(LYRIC_WND_INFU& wnd_info, int id);     // �л�������ģʽ
void lrc_click_hmode(LYRIC_WND_INFU& wnd_info, int id);     // �л�������ģʽ
void lrc_click_makelrc(LYRIC_WND_INFU& wnd_info, int id);   // ����������ʴ���
void lrc_click_font(LYRIC_WND_INFU& wnd_info, int id);      // ����Ŵ���С
void lrc_click_lrc_ms(LYRIC_WND_INFU& wnd_info, int id);    // ����Ӻ�/��ǰ
void lrc_click_lock_un(LYRIC_WND_INFU& wnd_info, int id);   // ����/����
void lrc_click_setting(LYRIC_WND_INFU& wnd_info, int id);   // �������ô���
void lrc_click_close(LYRIC_WND_INFU& wnd_info, int id);     // �رմ���
void lrc_click_lrc_color(LYRIC_WND_INFU& wnd_info, int id); // ����������ɫ
void lrc_click_menu(LYRIC_WND_INFU& wnd_info, int id);      // �˵���ť
bool lrc_click_play(LYRIC_WND_INFU& wnd_info, int id);      // ���Ű�ť����¼�, ����, ��ͣ, ��һ��, ��һ��, ����ֻ�ܴ����л�����/��ͣ��ť��ʽ

void lyric_wnd_button_click(LYRIC_WND_INFU& wnd_info)
{
    auto& item = wnd_info.button.rcBtn[wnd_info.button.indexDown];
    int id = item.id;

    int r = 0;
    if (wnd_info.pfnCommand)
        r = wnd_info.pfnCommand(wnd_info.hWnd, id, wnd_info.lParam);
    if (r != 0)
        return; // ����ֵ����0, ��ʾҪ�����¼�, ������

    lyric_wnd_call_event(wnd_info, id);
    //wchar_t buf[100];
    //swprintf_s(buf, L"��ť%d��������, id = %d\n", wnd_info.button.indexDown, id);
    //OutputDebugStringW(buf);
}

bool lyric_wnd_call_event(LYRIC_WND_INFU& wnd_info, int id)
{
    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_TRANSLATE2:    // ���밴ť
    case LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL:// ���밴ť, ѡ��ģʽ
    case LYRIC_WND_BUTTON_ID_TRANSLATE1:    // ���밴ť
    case LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL:// ���밴ť, ѡ��ģʽ
        return lrc_click_translate(wnd_info, id);

    case LYRIC_WND_BUTTON_ID_LRCWRONG:      // ��ʲ���
    case LYRIC_WND_BUTTON_ID_LRCWRONG_V:    // ��ʲ���, ����İ�ťͼ��
        lrc_click_wrong(wnd_info, id);  // ���ܺ�������������ȫһ����
        break;
    case LYRIC_WND_BUTTON_ID_VERTICAL:      // ������ť
        lrc_click_vmode(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_HORIZONTAL:    // ������ť
        lrc_click_hmode(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_MAKELRC:       // �������
        lrc_click_makelrc(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_FONT_DOWN:     // �����С
    case LYRIC_WND_BUTTON_ID_FONT_UP:       // ��������
        lrc_click_font(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_BEHIND:        // ����Ӻ�
    case LYRIC_WND_BUTTON_ID_AHEAD:         // �����ǰ
        lrc_click_lrc_ms(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_LOCK:          // ������ť
    case LYRIC_WND_BUTTON_ID_UNLOCK:        // ������ť
        lrc_click_lock_un(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_SETTING:       // ���ð�ť
        lrc_click_setting(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_CLOSE:         // �رհ�ť
    case LYRIC_WND_BUTTON_ID_SHOW:          // ��ʾ���, ���û�а�ť, ���ⲿ����
        lrc_click_close(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_LRCCOLOR:      // ����������ɫ, ���ֵİ�ťͼ��
        lrc_click_lrc_color(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_MENU:          // �˵���ť
        lrc_click_menu(wnd_info, id);
        break;
    case LYRIC_WND_BUTTON_ID_PLAY:          // ����, �ص���������0�������ͣ��ť
    case LYRIC_WND_BUTTON_ID_PAUSE:         // ��ͣ, �ص���������0����ɲ��Ű�ť
    case LYRIC_WND_BUTTON_ID_PREV:          // ��һ��
    case LYRIC_WND_BUTTON_ID_NEXT:          // ��һ��
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
    // ȥö�������ҵ���Ӧ�İ�ť
    for (auto& item : wnd_info.button.rcBtn)
    {
        if (item.id == id)
            return &item;
    }
    return nullptr;
}

void lrc_click_wrong(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"������Ҫ����һ��������ʵĴ���, Ӧ�����ⲿȥʵ��", L"��ʾ", MB_OK);
}

bool lrc_click_translate(LYRIC_WND_INFU& wnd_info, int id)
{
    LYRIC_WND_BUTTON_INFO* pItem = lrc_click_get_item(wnd_info, id);
    if (pItem == nullptr)
        return false;

    auto& item = *pItem;

    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_TRANSLATE2:    // ���밴ť
        item.id = LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL;
        // ��Ҫ�л��� ������ʾ����ı�, ������ʾ�����ı���ģʽ, ���������ж���
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL:// ���밴ť, ѡ��ģʽ
        item.id = LYRIC_WND_BUTTON_ID_TRANSLATE2;
        // �л���ԭ���ĸ��ģʽ
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE1:    // ���밴ť
        item.id = LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL;
        // ��Ҫ�л��� ������ʾ����ı�, ������ʾ�����ı���ģʽ, ���������ж���
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL:// ���밴ť, ѡ��ģʽ
        item.id = LYRIC_WND_BUTTON_ID_TRANSLATE1;
        // �л���ԭ���ĸ��ģʽ
        break;
    default:
        return false;
    }
    return true;
}

void lrc_click_vmode(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"�л�������ģʽ", L"��ʾ", MB_OK);
}

void lrc_click_hmode(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"�л�������ģʽ", L"��ʾ", MB_OK);
}

void lrc_click_makelrc(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"����������ʴ���", L"��ʾ", MB_OK);

}

void lrc_click_font(LYRIC_WND_INFU& wnd_info, int id)
{
    // ��������ߴ�, ��Ҫ��������ߴ�Ȼ��������ڳߴ�, Ȼ�����´�������
    int size = id == LYRIC_WND_BUTTON_ID_FONT_DOWN ? -2 : 2;

    wnd_info.lf.lfHeight += size;
    wnd_info.dx.re_create_font(&wnd_info);      // �����ߴ�����´���
    lyric_re_calc_text_width(wnd_info.hLyric);  // ���¼����ʿ��
    wnd_info.change_btn = true;                 // ��ǰ�ť��Ҫ���»滭
}

void lrc_click_lrc_ms(LYRIC_WND_INFU& wnd_info, int id)
{
    wnd_info.nTimeOffset += (id == LYRIC_WND_BUTTON_ID_BEHIND ? -500 : 500);
    wnd_info.change_btn = true;   // ��ǰ�ť��Ҫ���»滭
}

void lrc_click_lock_un(LYRIC_WND_INFU& wnd_info, int id)
{
    DWORD styleEx = (DWORD)GetWindowLongPtrW(wnd_info.hWnd, GWL_EXSTYLE);
    wnd_info.isLock = (id == LYRIC_WND_BUTTON_ID_LOCK);
    if (wnd_info.isLock)
        SetWindowLongPtrW(wnd_info.hWnd, GWL_EXSTYLE, styleEx | WS_EX_TRANSPARENT);
    else
        SetWindowLongPtrW(wnd_info.hWnd, GWL_EXSTYLE, styleEx & ~WS_EX_TRANSPARENT);
    wnd_info.change_btn = true; // ��ǰ�ť��Ҫ���»滭
}

void lrc_click_setting(LYRIC_WND_INFU& wnd_info, int id)
{
    MessageBoxW(wnd_info.hWnd, L"�������ô���", L"��ʾ", MB_OK);
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
    // ����ṷʹ�õ��ǵ���󵯳�ѡ����ɫ�Ĳ˵�, ѡ�������ɫ�ͱ���
    // �ڲ����弸����ɫ, �����ť���л�����Ӧ����ɫ, ��Ҫ���û�ˢ��ɫ
}

void lrc_click_menu(LYRIC_WND_INFU& wnd_info, int id)
{
    // Ŀǰ��ʱû��ʹ��

}

bool lrc_click_play(LYRIC_WND_INFU& wnd_info, int id)
{
    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_PLAY:          // ����, �ص���������0�������ͣ��ť
    case LYRIC_WND_BUTTON_ID_PAUSE:         // ��ͣ, �ص���������0����ɲ��Ű�ť
    {
        auto* pItem = lrc_click_get_item(wnd_info, id);
        if (!pItem)
            return false;
        auto& item = *pItem;

        // ����, ����ڲ�������, ����ֻ�ܻ���id
        if (item.id == LYRIC_WND_BUTTON_ID_PLAY)
            item.id = LYRIC_WND_BUTTON_ID_PAUSE;    // �Ǳ����¼��ͰѰ�ť������ͣ
        else
            item.id = LYRIC_WND_BUTTON_ID_PLAY;     // ����ͣ�¼��ͰѰ�ť���ɲ���

        wnd_info.change_btn = true; // ��ǰ�ť��Ҫ���»滭
        break;
    }
    case LYRIC_WND_BUTTON_ID_PREV:          // ��һ��
    case LYRIC_WND_BUTTON_ID_NEXT:          // ��һ��
        break;
    default:
        return false;
    }
    return true;
}

NAMESPACE_LYRIC_WND_END
