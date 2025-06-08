#pragma once
#include "lyric_desktop_typedef.h"

NAMESPACE_LYRIC_DESKTOP_BEGIN


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
    bool                bVertical;      // �Ƿ�����ģʽ
    bool                bSingleLine;    // �Ƿ�����ʾ
    bool                bSelfy;         // ���밴ť�Ƿ�ѡ��, �����밴ť����, ������Ϊtrueʱ, ��ʾ����
    bool                bSelyy;         // ���밴ť�Ƿ�ѡ��, �ͷ��밴ť����

    float               padding_text_;  // ���4���ߵļ��, ԭʼֵ, ������������Ա�����ź��ֵ
    float               padding_wnd_;   // ����4���ߵļ��, ԭʼֵ, ������������Ա�����ź��ֵ
    float               padding_text;   // ���4���ߵļ��, ����߾���Ԥ��������/��Ӱ �����ķ�Χ
    float               padding_wnd;    // ����4���ߵļ��, �����Χ����, �������ݻ滭�����ڱ���
    float               strokeWidth;    // �ı��߿�Ŀ��
    float               strokeWidth_div;// ͨ�������С�������ֵ���õ��߿���, �����ֵ����ʹ��
    bool                fillBeforeDraw; // �Ƿ����������, ����ǻ滭����ı��Ǵ���
    int                 nLineSpace;     // �о�, ��λ������

    int                 line1_align;    // ��һ�и���ı����뷽ʽ
    int                 line2_align;    // �ڶ��и���ı����뷽ʽ

    std::wstring        szDefText;      // û�и��ʱ��Ĭ���ı�, ���������֮���
    std::wstring        szFontName;     // ��������
    int                 nFontSize;      // ����ߴ�, ���ֵ��û������, ���������ʱ�������
    int                 lfWeight;       // 400=����, 700=����

    RECT                rect_h;         // ����ģʽ�µĴ���λ��
    RECT                rect_v;         // ����ģʽ�µĴ���λ��

    std::vector<DWORD>  clrNormal;      // ��ͨ��ʻ�ˢ��ɫ��
    std::vector<DWORD>  clrLight;       // ������ʻ�ˢ��ɫ��

    DWORD               clrBorderNormal;// �����ͨ�ı��߿���ɫ
    DWORD               clrBorderLight; // ��ʸ����ı��߿���ɫ
    DWORD               clrWndBack;     // ����ƶ�����֮����ʾ�ĸ��ARGB������ɫ
    DWORD               clrWndBorder;   // ����ƶ�����֮����ʾ�ĸ��ARGB�߿���ɫ
    DWORD               clrLine;        // ����ƶ�����֮����ʾ�ĸ��ARGB�߿���ɫ

    std::vector<float>  clrNormal_GradientStop; // ��ͨ��ʻ�ˢ��ɫ��
    std::vector<float>  clrLight_GradientStop;  // ��ͨ��ʻ�ˢ��ɫ��

    LYRIC_DESKTOP_CONFIG_DEBUG debug;   // ����������Ϣ

    // ����Ĭ������, ����ṹ��ʼ����ʱ��û�и���Ա��ֵ, �������������Ÿ�ֵ
    // һ�����ڴ��ڴ���֮�����, Ȼ��ͨ��DPI����������
    void init();

    // ��json�����������Ϣ
    int parse(const char* pszJson, LYRIC_DESKTOP_INFO* pWndInfo);

    // ��������Ϣת��json�ı�
    char* to_json(LYRIC_DESKTOP_INFO* pWndInfo) const;

}*PLYRIC_DESKTOP_CONFIG;

NAMESPACE_LYRIC_DESKTOP_END

