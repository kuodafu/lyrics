#pragma once
#include "lyric_desktop_typedef.h"

NAMESPACE_LYRIC_DESKTOP_BEGIN

// ����λ����Ϣ, ����������ʹ�õ�λ�ò�һ��
struct LYRIC_DESKTOP_POS : RECT
{
    int width;
    int height;
};

// ������Ϣ, ����ʹ��, �������߹ر�һЩ����, ����鿴Ч��
typedef struct LYRIC_DESKTOP_CONFIG_DEBUG
{
    DWORD       clrTextBackNormal;  // ��ͨ����ı�������ɫ
    DWORD       clrTextBackLight;   // ��������ı�������ɫ

    bool        alwaysFillBack;     // ʼ����䱳��
    bool        alwaysDraw;         // ʼ�ջ滭, ��ʹ�������û�б仯
    bool        alwaysCache;        // ʼ�մ�������, ��ʹ�������û�б仯
    bool        alwaysCache1;       // ʼ�մ�������, ��ʹ�������û�б仯

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

// ������Ϣ, �����õ�������, �������õ�, ��д������, ��ʱ������һ��json, ���ⲿ����
typedef struct LYRIC_DESKTOP_CONFIG
{
    int                 refreshRate;    // ˢ����, ˢ�¸�ʵ�Ƶ��, ������ˢ������ 30, 60, 75, 90, 100, 120, 144, 165, 240
    LPCWSTR             pszDefText;     // û�и��ʱ��Ĭ���ı�, ���������֮���
    int                 nDefText;       // Ĭ���ı�����
    float               padding_text_;  // ���4���ߵļ��, ԭʼֵ, ������������Ա�����ź��ֵ
    float               padding_wnd_;   // ����4���ߵļ��, ԭʼֵ, ������������Ա�����ź��ֵ
    float               padding_text;   // ���4���ߵļ��, ����߾���Ԥ��������/��Ӱ �����ķ�Χ
    float               padding_wnd;    // ����4���ߵļ��, �����Χ����, �������ݻ滭�����ڱ���
    LYRIC_MODE          mode;           // �����ʾģʽ, LYRIC_MODE ö������

    std::wstring        pszFontName;    // ��������
    int                 nFontSize;      // ����ߴ�, ���ֵ��û������, ���������ʱ�������
    int                 lfWeight;       // 400=����, 700=����
    int                 nLineSpace;     // �о�, ��λ������

    LYRIC_DESKTOP_POS   pos_h;          // ����ģʽ�µĴ���λ��
    LYRIC_DESKTOP_POS   pos_v;          // ����ģʽ�µĴ���λ��

    std::vector<DWORD>  clrNormal;      // ��ͨ��ʻ�ˢ��ɫ��
    std::vector<DWORD>  clrLight;       // ������ʻ�ˢ��ɫ��
    DWORD               clrBorder;      // ����ı��߿���ɫ
    DWORD               clrWndBack;     // ����ƶ�����֮����ʾ�ĸ��ARGB������ɫ
    DWORD               clrWndBorder;   // ����ƶ�����֮����ʾ�ĸ��ARGB�߿���ɫ

    LYRIC_DESKTOP_CONFIG_DEBUG debug;   // ����������Ϣ
    LYRIC_DESKTOP_CONFIG() { default_config(); }
    void default_config();
}*PLYRIC_DESKTOP_CONFIG;

NAMESPACE_LYRIC_DESKTOP_END

