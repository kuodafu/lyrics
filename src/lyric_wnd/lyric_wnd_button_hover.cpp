#include "lyric_wnd_function.h"
#include <CommCtrl.h>


NAMESPACE_LYRIC_WND_BEGIN

// ������ʾ��Ϣ
void lyric_wnd_set_tips(LYRIC_WND_INFO& wnd_info, LPCWSTR pszTips);

// ������ʾ��Ϣ
void lyric_wnd_hide_tips(LYRIC_WND_INFO& wnd_info);



// �����ͣ�ڰ�ť��, ����һ����ʾ��Ϣ
void lyric_wnd_button_hover(LYRIC_WND_INFO& wnd_info)
{
    auto& item = wnd_info.button.rcBtn[wnd_info.button.index];
    LPCWSTR pszTips = nullptr;
    int id = item.id;
    wchar_t buffer[260] = { 0 };

    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_TRANSLATE2:    // ���밴ť
        pszTips = L"����л�������ģʽ";
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL:// ���밴ť, ѡ��ģʽ
        pszTips = L"����ر�����ģʽ";
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE1:    // ���밴ť
        pszTips = L"����л�������ģʽ";
        break;
    case LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL:// ���밴ť, ѡ��ģʽ
        pszTips = L"����رշ���ģʽ";
        break;
    case LYRIC_WND_BUTTON_ID_LRCWRONG:      // ��ʲ���
    case LYRIC_WND_BUTTON_ID_LRCWRONG_V:    // ��ʲ���, ����İ�ťͼ��
        pszTips = L"��ʲ���, �򿪸������";
        break;
    case LYRIC_WND_BUTTON_ID_VERTICAL:      // ������ť
        pszTips = L"�л�������ģʽ";
        break;
    case LYRIC_WND_BUTTON_ID_MAKELRC:       // �������
        pszTips = L"�������";
        break;
    case LYRIC_WND_BUTTON_ID_FONT_DOWN:     // �����С
    case LYRIC_WND_BUTTON_ID_FONT_UP:       // ��������
    {
        LPCWSTR fmt = id == LYRIC_WND_BUTTON_ID_FONT_DOWN
            ? L"��С����������, ��ǰ�����С: %d"
            : L"�Ŵ�����������, ��ǰ�����С: %d";
        swprintf_s(buffer, fmt, wnd_info.lf.lfHeight);
        pszTips = buffer;
        break;
    }
    case LYRIC_WND_BUTTON_ID_BEHIND:        // ����Ӻ�
    case LYRIC_WND_BUTTON_ID_AHEAD:         // �����ǰ
    {
        if (wnd_info.nTimeOffset == 0)
        {
            // ֵ��0, ��ʾû����ʱ���
            pszTips = id == LYRIC_WND_BUTTON_ID_BEHIND
                ? L"����Ӻ�0.5��, ��ǰû���Ӻ�"
                : L"�����ǰ0.5��, ��ǰû����ǰ";
        }
        else if (wnd_info.nTimeOffset > 0)
        {
            // �Ӻ��Ǹ���, ��ǰ������
            // ����0, �Ǿ��Ǹ����ǰ��
            LPCWSTR fmt = id == LYRIC_WND_BUTTON_ID_BEHIND
                ? L"����Ӻ�, ��ǰ��ǰ�� %.1f ��"
                : L"�����ǰ, ��ǰ��ǰ�� %.1f ��";
            double f = (double)wnd_info.nTimeOffset / 1000.0f;
            swprintf_s(buffer, fmt, f);
            pszTips = buffer;
        }
        else
        {
            // �Ӻ��Ǹ���, ��ǰ������
            // С��0, �Ǿ��Ǹ���Ӻ���
            LPCWSTR fmt = id == LYRIC_WND_BUTTON_ID_BEHIND
                ? L"����Ӻ�, ��ǰ�Ӻ��� %.1f ��"
                : L"�����ǰ, ��ǰ�Ӻ��� %.1f ��";
            double f = (double)(-wnd_info.nTimeOffset) / 1000.0f;
            swprintf_s(buffer, fmt, f);
            pszTips = buffer;
        }
        break;
    }
    case LYRIC_WND_BUTTON_ID_LOCK:          // ������ť
        pszTips = L"����������";
        break;
    case LYRIC_WND_BUTTON_ID_SETTING:       // ���ð�ť
        pszTips = L"����������";
        break;
    case LYRIC_WND_BUTTON_ID_UNLOCK:        // ������ť
        pszTips = L"����������";
        break;
    case LYRIC_WND_BUTTON_ID_CLOSE:         // �رհ�ť
        pszTips = L"�ر�������";
        break;
    case LYRIC_WND_BUTTON_ID_LRCCOLOR:      // ����������ɫ, ���ֵİ�ťͼ��
        pszTips = L"����������ɫ";
        break;
    case LYRIC_WND_BUTTON_ID_MENU:          // �˵���ť
        pszTips = L"�˵�";
        break;
    case LYRIC_WND_BUTTON_ID_HORIZONTAL:    // ������ť
        pszTips = L"����ģʽ";
        break;
    case LYRIC_WND_BUTTON_ID_PLAY:          // ����, �ص���������0�������ͣ��ť
        pszTips = L"����";
        break;
    case LYRIC_WND_BUTTON_ID_PAUSE:         // ��ͣ, �ص���������0����ɲ��Ű�ť
        pszTips = L"��ͣ";
        break;
    case LYRIC_WND_BUTTON_ID_PREV:          // ��һ��
        pszTips = L"��һ��";
        break;
    case LYRIC_WND_BUTTON_ID_NEXT:          // ��һ��
        pszTips = L"��һ��";
        break;
    default:
        break;
    }
    lyric_wnd_set_tips(wnd_info, pszTips);

}

void lyric_wnd_button_leave(LYRIC_WND_INFO& wnd_info)
{
    lyric_wnd_hide_tips(wnd_info);
}


void lyric_wnd_set_tips(LYRIC_WND_INFO& wnd_info, LPCWSTR pszTips)
{
    TTTOOLINFOW ti = { 0 };
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_IDISHWND;
    ti.hwnd = wnd_info.hWnd;
    ti.uId = (UINT_PTR)wnd_info.hWnd;
    ti.lpszText = (LPWSTR)pszTips;
    auto ret = SendMessageW(wnd_info.hTips, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);

}

void lyric_wnd_hide_tips(LYRIC_WND_INFO& wnd_info)
{
    TTTOOLINFOW ti = { 0 };
    ti.cbSize = sizeof(ti);
    ti.uFlags = TTF_IDISHWND;
    ti.hwnd = wnd_info.hWnd;
    ti.uId = (UINT_PTR)wnd_info.hWnd;

    SendMessageW(wnd_info.hTips, TTM_POP, 0, 0);
    ti.lpszText = (LPWSTR)L"";
    SendMessageW(wnd_info.hTips, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);
}


NAMESPACE_LYRIC_WND_END

