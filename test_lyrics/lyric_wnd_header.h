#pragma once
#include "lyric_wnd.h"
#include "CD2DRender.h"
#include "CD2DImage.h"
#include <CScale.h>
#include <vector>
#include <string>
#include "../kuodafu_lyric.h"

#define NAMESPACE_LYRIC_WND lyric_wnd
#define NAMESPACE_LYRIC_WND_BEGIN namespace NAMESPACE_LYRIC_WND{
#define NAMESPACE_LYRIC_WND_END }




NAMESPACE_LYRIC_WND_BEGIN



struct LYRIC_WND_INFU;
// ��ʴ���dx��صĶ���
struct LYRIC_WND_DX
{
    NAMESPACE_D2D::CD2DRender*  hCanvas;        // D2D�滭���
    NAMESPACE_D2D::CD2DFont*    hFont;          // �滭��ʵ�����, ������豸�޹�����, �豸ʧЧ����Ҫ���´���

    NAMESPACE_D2D::CD2DImage*   image;          // ��ʴ��ڰ�ť��Ҫ��ͼƬ
    NAMESPACE_D2D::CD2DBrush*   hbrBorder;      // �滭����ı��ı߿�ˢ
    NAMESPACE_D2D::CD2DBrush*   hbrLine;        // ��ʰ�ť�ָ����ֵ�������ˢ
    NAMESPACE_D2D::CD2DBrush_LinearGradient* hbrNormal;      // ��ͨ��ʻ�ˢ
    NAMESPACE_D2D::CD2DBrush_LinearGradient* hbrLight;       // ������ʻ�ˢ

    DWORD       clrBack;        // ����ƶ�����֮����ʾ�ĸ��ARGB������ɫ


    LYRIC_WND_DX()
    {
        hFont = nullptr;
        hCanvas = nullptr;
        image = nullptr;
        hbrBorder = nullptr;
        hbrLine = nullptr;
        hbrNormal = nullptr;
        hbrLight = nullptr;
        clrBack = 0;
    }

    ~LYRIC_WND_DX()
    {
        destroy(true);
    }

    // ���´������ж���
    bool re_create(LYRIC_WND_INFU* pWndInfo);
    
    // ���´�����ˢ����, ��ͨ��ˢ/������ˢ, �ⲿ����������ɫ��ʱ�����
    bool re_create_brush(LYRIC_WND_INFU* pWndInfo, bool isLight);

    // ���´����߿�ˢ
    bool re_create_border(LYRIC_WND_INFU* pWndInfo);

    // ���´����������, �ⲿ�������������ʱ�����
    bool re_create_font(LYRIC_WND_INFU* pWndInfo);

    // ���´���ͼƬ��Դ����
    bool re_create_image(LYRIC_WND_INFU* pWndInfo);

    // ���������豸��صĶ���, �������豸�޹ض���, ����ѡ���Ƿ�����
    bool destroy(bool isDestroyFont);
};

// ��ʴ���ͼƬ�ľ���
struct LYRIC_WND_IMAGE
{
    RECT rcNormal;
    RECT rcLight;
    RECT rcDown;
    RECT rcDisable;
};

// ��ť��Ϣ, ������ť��λ��, id����Ϣ
struct LYRIC_WND_BUTTON_INFO
{
    int     id;         // ��ť��ID, ͨ�����id�ҵ����ĸ�λ�ð�ͼƬ�ó����滭
    int     index;      // ��ť����, ��1��ʼ, ��ʾ��ʾ�ĵڼ�����ť, ��xml���˳���Ӧ
    int     state;      // ��ť״̬
    RECT    rc;         // ��ťʵ�ʵ�λ��, ��λ������, �ж�����ƶ������λ�þ��ڰ�ť��
    RECT*   prcSrc;     // ��ť��Դ����, �Ӵ�ͼƬ�ϵ��ĸ�λ���ó����滭
};
struct LYRIC_WND_BUTTON
{
    std::vector<LYRIC_WND_BUTTON_INFO>  rcBtn;  // ��ťʵ�ʻ滭��λ��, id ����Ϣ
    std::vector<LYRIC_WND_IMAGE>        rcSrc;  // Դ����, �Ӵ�ͼƬ�ϵ��ĸ�λ���ó����滭

    int     index{-1};    // ��ť����, ��ǰ����ƶ������ĸ�������, ����������� rcBtn ���±�
    int     indexDown{-1};// ��������
    int     width{};      // ���а�ť�Ŀ��
    int     maxHeight{};  // ���ť�ĸ߶�

};



// ��ʴ��� USERDATA ���ŵ�������ṹ
typedef struct LYRIC_WND_INFU
{
    HWND        hWnd;           // ��ʴ��ھ��
    HWND        hTips;          // ��ʾ���ھ��
    HLYRIC      hLyric;         // ��ʾ��
    int         prevIndexLine;  // ��һ�λ滭�ĸ���к�
    int         prevWidth;      // ��һ�λ滭�ĸ�ʿ��
    float       nLineHeight;    // һ�и�ʵĸ߶�
    int         nCurrentTimeMS; // ��ǰ��ʲ���ʱ��
    int         nTimeOffset;    // ʱ��ƫ��, ������λ�õ�ʱ��������ƫ��
    int         nMinWidth;      // ��ʴ�����С���, ���а�ť�Ŀ�� ����һЩ�߾�
    int         nMinHeight;     // ��ʴ�����С�߶�
    int         nLineTop1;      // ��һ�и�ʵĶ���λ��
    int         nLineTop2;      // �ڶ��и�ʵĶ���λ��
    union
    {
        struct
        {
            USHORT  isFillBack : 1; // �Ƿ���䱳��
            USHORT  isLock : 1;     // ����Ƿ��Ѿ�����
        };
        USHORT  status; // ״̬��������, �������
    };
    union
    {
        struct
        {
            USHORT  change_wnd : 1; // �����Ƿ��иı�
            USHORT  change_btn : 1; // ��ť�����Ƿ��иı�, �ȵ�ı�/���¸ı��
        };    // �д��ڲ����иı�ķ�����, �����ֵΪ���ʱ��������߽����ػ�
        USHORT  change;
    };


    LYRIC_WND_BUTTON        button;
    PFN_LYRIC_WND_COMMAND   pfnCommand; // ��ʴ����ϵİ�ť������ص�����
    LPARAM                  lParam;     // ��ʴ����ϵİ�ť������ص������Ĳ���
    LOGFONTW                lf{};       // ������Ϣ
    LYRIC_WND_DX            dx;         // dx��صĶ���
    std::vector<DWORD>      clrNormal;  // ��ͨ��ʻ�ˢ��ɫ��
    std::vector<DWORD>      clrLight;   // ������ʻ�ˢ��ɫ��
    DWORD                   clrBorder;  // ����ı��߿���ɫ
    CScale                  scale;      // ���ű���
    LYRIC_WND_INFU()
    {
        hWnd = nullptr;
        hTips = nullptr;
        hLyric = nullptr;
        prevIndexLine = -1;
        prevWidth = 0;
        nLineHeight = 0;
        nCurrentTimeMS = 0;
        nTimeOffset = 0;
        isFillBack = 0;
        status = 0;
        change = 0;
        nMinWidth = 0;
        nMinHeight = 0;
        nLineTop1 = 0;
        nLineTop2 = 0;

        dx.clrBack = MAKEARGB(100, 0, 0, 0);
        clrBorder = MAKEARGB(255, 33, 33, 33);

        pfnCommand = nullptr;
        lParam = 0;
    }

    ~LYRIC_WND_INFU()
    {
        lyric_destroy(hLyric);
        DestroyWindow(hTips);
    }

    void set_def_arg(const LYRIC_WND_ARG* arg);

}*PLYRIC_WND_INFU;


bool lyric_wnd_geometry_add_string(ID2D1DeviceContext* pRenderTarget, ID2D1PathGeometry* pPathGeometry, IDWriteTextLayout* pTextLayout);

bool lyric_wnd_draw_geometry(ID2D1DeviceContext* pRenderTarget, ID2D1PathGeometry* pPathGeometry,
                             ID2D1LinearGradientBrush* hbrNormal, ID2D1LinearGradientBrush* hbrLight,
                             ID2D1SolidColorBrush* hbrDraw,
                             const D2D1_RECT_F& rcText, const D2D1_RECT_F& rcText2,
                             LYRIC_CALC_STRUCT& arg, IDWriteTextFormat* dxFormat);

HWND lyric_create_layered_window(const LYRIC_WND_ARG* arg);
void lyric_wnd_default_object(LYRIC_WND_INFU& wnd_info);

// �ø�ʴ���ʧЧ, Ȼ���ػ�
// isUpdate = Ϊtrue��ʱ��ʼ���ػ�, Ϊfalse��ʱ����ж��Ƿ���Ҫ�滭
bool lyric_wnd_invalidate(LYRIC_WND_INFU& wnd_info);

// ���س�����Դ���ͼƬ, Ȼ��Ѹ������궼��¼��
bool lyric_wnd_load_image(LYRIC_WND_INFU& wnd_info);

// �滭��ʴ�����Ҫ�İ�ť
void lyric_wnd_draw_button(LYRIC_WND_INFU& wnd_info, const RECT& rcWindow, LYRIC_CALC_STRUCT& arg);

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
int lyric_wnd_calc_button(LYRIC_WND_INFU& wnd_info, int& maxHeight, int offset);



NAMESPACE_LYRIC_WND_END