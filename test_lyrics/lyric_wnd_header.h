#pragma once
#include "lyric_wnd.h"
#include "CD2DRender.h"
#include "CD2DFont.h"
#include "CD2DBrush.h"
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
    NAMESPACE_D2D::CD2DRender* hCanvas;         // D2D�滭���
    NAMESPACE_D2D::CD2DFont* hFont;             // �滭��ʵ�����, ������豸�޹�����, �豸ʧЧ����Ҫ���´���

    NAMESPACE_D2D::CD2DImage* image;            // ��ʴ��ڰ�ť��Ҫ��ͼƬ
    NAMESPACE_D2D::CD2DImage* image_shadow;     // ��ӰͼƬ
    NAMESPACE_D2D::CD2DBrush* hbrBorder;        // �滭����ı��ı߿�ˢ
    NAMESPACE_D2D::CD2DBrush* hbrWndBorder;     // ��ʴ��ڵı߿�ˢ
    NAMESPACE_D2D::CD2DBrush* hbrWndBack;       // ��ʴ��ڵı�����ˢ
    NAMESPACE_D2D::CD2DBrush* hbrLine;          // ��ʰ�ť�ָ����ֵ�������ˢ
    NAMESPACE_D2D::CD2DBrush_LinearGradient* hbrNormal;      // ��ͨ��ʻ�ˢ
    NAMESPACE_D2D::CD2DBrush_LinearGradient* hbrLight;       // ������ʻ�ˢ

    DWORD       clrBack;        // ����ƶ�����֮����ʾ�ĸ��ARGB������ɫ
    DWORD       clrWndBorder;   // ����ƶ�����֮����ʾ�ĸ��ARGB�߿���ɫ

    ID2D1Bitmap* pBitmapBack;   // ����λͼ, ���ڱ���, �ߴ�ı��ʱ����Ҫ���´���


    LYRIC_WND_DX()
    {
        hFont = nullptr;
        hCanvas = nullptr;
        image = nullptr;
        image_shadow = nullptr;
        hbrBorder = nullptr;
        hbrWndBorder = nullptr;
        hbrWndBack = nullptr;
        hbrLine = nullptr;
        hbrNormal = nullptr;
        hbrLight = nullptr;
        pBitmapBack = nullptr;
        clrBack = 0;
        clrWndBorder = 0;
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
    RECT* prcSrc;     // ��ť��Դ����, �Ӵ�ͼƬ�ϵ��ĸ�λ���ó����滭
};
struct LYRIC_WND_BUTTON
{
    std::vector<LYRIC_WND_BUTTON_INFO>  rcBtn;  // ��ťʵ�ʻ滭��λ��, id ����Ϣ
    std::vector<LYRIC_WND_IMAGE>        rcSrc;  // Դ����, �Ӵ�ͼƬ�ϵ��ĸ�λ���ó����滭

    int     index{ -1 };    // ��ť����, ��ǰ����ƶ������ĸ�������, ����������� rcBtn ���±�
    int     indexDown{ -1 };// ��������
    int     width{};        // ���а�ť�Ŀ��
    int     height{};       // ���а�ť�ĸ߶�
    int     maxWidth{};     // ���ť�Ŀ��
    int     maxHeight{};    // ���ť�ĸ߶�

};

// �������, һ����������λͼ, һ������ͨ���, һ���Ǹ������, ��������, ����ֱ���趨�ü����Ϳ�����
// ������߷���ģʽ�������ж��и�������
// ��һ������ͨ��� + һ���и�����ʻ滭������λͼ��, ֻ�и�ʸı�Ż����»滭�ڶ���
struct LYRIC_WND_CACHE_OBJ
{
    int     preIndex;   // �ϴλ滭���к�����
    LPCWSTR preText;    // �ϴλ滭���ı���ַ, �кź��ı���һ���Ǿ��ǲ���Ҫ���´�������
    int     preLength;  // �ϴλ滭�ı��ĳ���

    D2D1_RECT_F rcBounds;       // ʵ�ʻ滭������
    ID2D1Bitmap* pBitmapNormal; // ����λͼ, ��ͨ����ı�, һ�λ���, ����ֱ���趨�ü����Ϳ�����
    ID2D1Bitmap* pBitmapLight;  // ����λͼ, ��������ı�

    LYRIC_WND_CACHE_OBJ();
    ~LYRIC_WND_CACHE_OBJ();
};

// ��¼�滭�ı���Ҫ������, ·��, ��Ӱ��ʽ����ʹ������ṹ, һ�ж�Ӧһ���ṹ
struct LYRIC_WND_DRAWTEXT_INFO
{
    LYRIC_LINE_STRUCT   line;       // �������Ϣ
    LYRIC_WND_CACHE_OBJ cache;      // �������ָ��

    int         index;              // ����к�, ��ǰ�滭���к�

    int         align;              // ����ģʽ, 0=�����, 1=���ж���, 2=�Ҷ���
    D2D1_RECT_F rcText;             // ����ı��滭��λ��, ���λ���Ǹ��ݶ���ģʽ�����

    float text_wtdth;               // ����ı����, �ڻ滭ʱ�����
    float text_height;              // ����ı��߶�, �ڻ滭ʱ�����

    float nLightWidth;              // ��ʸ���λ��, ����0�Ļ�����Ҫ�滭��������

    LYRIC_WND_DRAWTEXT_INFO()
    {
        align = 0;
        clear();
    }

    inline void clear()
    {
        line = {};
        index = -1;
        rcText = {0};
        nLightWidth = 0.f;
        text_wtdth = 0.f;
        text_height = 0.f;

        cache.preIndex = -1;
        cache.preText = nullptr;
        cache.preLength = 0;
        cache.rcBounds = { 0 };
    }
};

enum class LYRIC_MODE : unsigned int
{
    DOUBLE_ROW      = 0x0000,   // ˫�и��
    TRANSLATION1    = 0x0001,   // ����
    TRANSLATION2    = 0x0002,   // ����
    SINGLE_ROW      = 0x0004,   // ������ʾ

    VERTICAL        = 0x10000,  // ����ģʽ
    EXISTTRANS      = 0x20000,  // ���ڷ���, ����������жϸ�ʶ���ģʽ, ����Ĭ��˫��

};


// ��ʴ��� USERDATA ���ŵ�������ṹ
typedef struct LYRIC_WND_INFU
{
    HWND        hWnd;           // ��ʴ��ھ��
    HWND        hTips;          // ��ʾ���ھ��
    HLYRIC      hLyric;         // ��ʾ��
    int         prevIndexLine;  // ��һ�λ滭�ĸ���к�
    float       prevWidth;      // ��һ�λ滭�ĸ�ʿ��
    float       nLineHeight;    // һ�и�ʵĸ߶�
    float       nLineDefWidth;  // û�и��ʱ��ʵ�Ĭ�Ͽ��
    LPCWSTR     pszDefText;     // û�и��ʱ��Ĭ���ı�
    int         nDefText;       // Ĭ���ı�����
    int         nCurrentTimeMS; // ��ǰ��ʲ���ʱ��
    int         nTimeOffset;    // ʱ��ƫ��, ��ʾ��ʾʱʹ��
    int         nMinWidth;      // ��ʴ�����С���, ���а�ť�Ŀ�� ����һЩ�߾�
    int         nMinHeight;     // ��ʴ�����С�߶�
    int         nLineTop1;      // ��һ�и�ʵĶ���λ��
    int         nLineTop2;      // �ڶ��и�ʵĶ���λ��
    RECT        rcWindow;       // ��ʴ��ڵ�λ��, �������ڶ��ǿͻ���, �����¼������Ļλ��, �滭ʱ��λ��, ����֤�ǵ�ǰ���ڵ�λ��
    
    float       shadowRadius;   // ��Ӱ�뾶
    LYRIC_MODE  mode;           // �����ʾģʽ
    
    bool has_mode(LYRIC_MODE flag) const
    {
        using T = std::underlying_type_t<LYRIC_MODE>;
        return (static_cast<T>(mode) & static_cast<T>(flag)) != 0;
    }
    void add_mode(LYRIC_MODE flag)
    {
        using T = std::underlying_type_t<LYRIC_MODE>;
        mode = static_cast<LYRIC_MODE>(static_cast<T>(mode) | static_cast<T>(flag));
    }
    void del_mode(LYRIC_MODE flag)
    {
        using T = std::underlying_type_t<LYRIC_MODE>;
        mode = static_cast<LYRIC_MODE>(static_cast<T>(mode) & ~static_cast<T>(flag));
    }

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
            USHORT  change_wnd : 1;     // �����Ƿ��иı�
            USHORT  change_btn : 1;     // ��ť�����Ƿ��иı�, �ȵ�ı�/���¸ı��
            USHORT  change_font : 1;    // �����иı�, ��Ҫ�ڻ滭����ı���ʱ�����´�������
            USHORT  change_hbr : 1;     // ��ˢ�иı�, ��Ҫ�ڻ滭����ı���ʱ�����´�������
            USHORT  change_trans : 1;   // ���벿��, ��Ҫ�ڻ滭����ı���ʱ�����´�������
        };    // �д��ڲ����иı�ķ�����, �����ֵΪ���ʱ��������߽����ػ�
        USHORT  change;
    };

    LYRIC_WND_DRAWTEXT_INFO line1;      // ��һ�и����Ϣ, �����������, �������Ϣ, ����ı��滭λ��, ��ʸ���λ�õ�
    LYRIC_WND_DRAWTEXT_INFO line2;      // �ڶ��и����Ϣ

    LYRIC_WND_BUTTON        button;
    PFN_LYRIC_WND_COMMAND   pfnCommand; // ��ʴ����ϵİ�ť������ص�����
    LPARAM                  lParam;     // ��ʴ����ϵİ�ť������ص������Ĳ���
    LOGFONTW                lf{};       // ������Ϣ
    LYRIC_WND_DX            dx;         // dx��صĶ���
    std::vector<DWORD>      clrNormal;  // ��ͨ��ʻ�ˢ��ɫ��
    std::vector<DWORD>      clrLight;   // ������ʻ�ˢ��ɫ��
    DWORD                   clrBorder;  // ����ı��߿���ɫ
    CScale                  scale;      // ���ű���
    LYRIC_WND_INFU();

    ~LYRIC_WND_INFU()
    {
        lyric_destroy(hLyric);
        DestroyWindow(hTips);
    }

    void set_def_arg(const LYRIC_WND_ARG* arg);

}*PLYRIC_WND_INFU;


NAMESPACE_LYRIC_WND_END

