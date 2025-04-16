#pragma once
#include <windows.h>

// ��ʴ��ڰ�ťID, Ŀǰ���⼸��Ĭ�ϰ�ť
enum LYRIC_WND_BUTTON_ID
{
    LYRIC_WND_BUTTON_ID_MENU        = 1000, // �˵���ť
    LYRIC_WND_BUTTON_ID_PREV        = 1001, // ��һ��
    LYRIC_WND_BUTTON_ID_NEXT        = 1002, // ��һ��
    LYRIC_WND_BUTTON_ID_PLAY        = 1003, // ����, �ص���������0�������ͣ��ť
    LYRIC_WND_BUTTON_ID_PAUSE       = 1004, // ��ͣ, �ص���������0����ɲ��Ű�ť
    LYRIC_WND_BUTTON_ID_FONT_UP     = 1005, // ��������
    LYRIC_WND_BUTTON_ID_FONT_DOWN   = 1006, // �����С
    LYRIC_WND_BUTTON_ID_SETTING     = 1007, // ���ð�ť
    LYRIC_WND_BUTTON_ID_AHEAD       = 1008, // �����ǰ
    LYRIC_WND_BUTTON_ID_BEHIND      = 1009, // ����Ӻ�
    LYRIC_WND_BUTTON_ID_SEARCH      = 1010, // ��ʲ���, һ���������������
    LYRIC_WND_BUTTON_ID_TRANSLATE1  = 1011, // ���밴ť
    LYRIC_WND_BUTTON_ID_TRANSLATE2  = 1012, // ���밴ť
    LYRIC_WND_BUTTON_ID_LOCK        = 1013, // ������ť
    LYRIC_WND_BUTTON_ID_CLOSE       = 1014, // �رհ�ť

};

typedef struct LYRIC_WND_ARG
{
    RECT        rcWindow;       // Ҫ���õĴ���λ�úʹ�С
    COLORREF    clrBackground;  // ������ɫ
    int         nFontSize;      // �����С
    LPCWSTR     pwszFontName;   // ��������

}*PLYRIC_WND_ARG;

// ��ʴ��ڰ�ť������¼�, ����0��ʾ�¼�����, ���ط�0��ʾ�����¼�
typedef int (CALLBACK* PFN_LYRIC_WND_COMMAND)(HWND hWindowLyric, int id, LPARAM lParam);

/// <summary>
/// ����һ����ʴ���, ��������Ƿֲ㴰��, ��Ҫ��ʾ�����
/// </summary>
/// <param name="arg">������ʴ��ڵĲ���, ����, ����λ��, ��ɫ����Ϣ</param>
/// <param name="pfnCommand">��ť������ص�����</param>
/// <param name="lParam">���ݵ� pfnCommand() ������Ĳ���</param>
/// <returns>���ش��ھ��</returns>
HWND lyric_wnd_create(const LYRIC_WND_ARG* arg, PFN_LYRIC_WND_COMMAND pfnCommand, LPARAM lParam);

/// <summary>
/// ��ʴ��ڼ���krc���, krc�ǿṷ��ʸ�ʽ, ���غ���Ե��ø��º�����ʾ
/// Ŀǰ��ʱֻ֧��krc, ����ܽ���������/qq���õ�ר�ø�ʸ�ʽ, ����������
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() ���صĴ��ھ��</param>
/// <param name="pKrcData">krc�ļ�����ָ��</param>
/// <param name="nKrcDataLen">krc�ļ����ݳߴ�</param>
/// <returns>�����Ƿ���سɹ�</returns>
bool lyric_wnd_load_krc(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen);

/// <summary>
/// ���¸����ʾ, ����ٶȻ�����, ����ε���Ҳ�ͼ��ٺ���, 200֡�����ȶ���ʾ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="nCurrentTimeMS">Ҫ���µ�����, ��λ�Ǻ���</param>
/// <returns>�����Ƿ���³ɹ�</returns>
bool lyric_wnd_update(HWND hWindowLyric, int nCurrentTimeMS);

/// <summary>
/// ��������ø���ı���ɫ, Ŀǰֻ֧����ͨ��ɫ�͸�����ɫ
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="isLight">���õ��Ƿ��Ǹ����ĸ��</param>
/// <param name="pClr">��ɫ����, ARGB��ɫֵ</param>
/// <param name="nCount">��ɫ�����һ��</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool lyric_wnd_set_color(HWND hWindowLyric, bool isLight, DWORD* pClr, int nCount);

/// <summary>
/// ���ø���ı�����, ��������ͨ��ʹ��ͬһ������, ����ʹ�ò�ͬ��ɫ����
/// </summary>
/// <param name="hWindowLyric">��ʴ��ھ��</param>
/// <param name="pszName">��������</param>
/// <param name="nSize">����ߴ�</param>
/// <param name="isBold">�Ƿ�Ӵ�</param>
/// <param name="isItalic">�Ƿ�б��</param>
/// <returns>�����Ƿ����óɹ�</returns>
bool lyric_wnd_set_font(HWND hWindowLyric, LPCWSTR pszName, int nSize, bool isBold, bool isItalic);


