#pragma once
#include <kuodafu_lyric_desktop.h>
#include <d2d/CD2DRender.h>
#include <d2d/CD2DFont.h>
#include <d2d/CD2DBrush.h>
#include <d2d/CD2DImage.h>
#include <CScale.h>
#include <vector>
#include <string>
#include <mutex>
#include <kuodafu_lyric.h>

#define NAMESPACE_LYRIC_DESKTOP lyric_desktop
#define NAMESPACE_LYRIC_DESKTOP_BEGIN namespace NAMESPACE_LYRIC_DESKTOP{
#define NAMESPACE_LYRIC_DESKTOP_END }

NAMESPACE_LYRIC_DESKTOP_BEGIN


struct LYRIC_DESKTOP_INFO;
// ��ʴ���dx��صĶ���
struct LYRIC_DESKTOP_DX
{
    ID2D1GdiInteropRenderTarget* pGDIInterop;
    KUODAFU_NAMESPACE::D2DRender* hCanvas;         // D2D�滭���
    KUODAFU_NAMESPACE::CD2DFont* hFont;             // �滭��ʵ�����, ������豸�޹�����, �豸ʧЧ����Ҫ���´���

    KUODAFU_NAMESPACE::D2DImage* image;        // ��ʴ��ڰ�ť��Ҫ��ͼƬ
    KUODAFU_NAMESPACE::D2DImage* image_shadow; // ��ӰͼƬ
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrBorder;        // �滭����ı��ı߿�ˢ
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrWndBorder;     // ��ʴ��ڵı߿�ˢ
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrWndBack;       // ��ʴ��ڵı�����ˢ
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrLine;          // ��ʰ�ť�ָ����ֵ�������ˢ
    KUODAFU_NAMESPACE::D2DLinearGradientBrush* hbrNormal;      // ��ͨ��ʻ�ˢ
    KUODAFU_NAMESPACE::D2DLinearGradientBrush* hbrLight;       // ������ʻ�ˢ

    DWORD       clrBack;        // ����ƶ�����֮����ʾ�ĸ��ARGB������ɫ
    DWORD       clrWndBorder;   // ����ƶ�����֮����ʾ�ĸ��ARGB�߿���ɫ

    ID2D1Bitmap* pBitmapBack;   // ����λͼ, ���ڱ���, �ߴ�ı��ʱ����Ҫ���´���


    LYRIC_DESKTOP_DX()
    {
        pGDIInterop = nullptr;
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

    ~LYRIC_DESKTOP_DX()
    {
        destroy(true);
    }

    // ���´������ж���
    bool re_create(LYRIC_DESKTOP_INFO* pWndInfo);

    // ���´�����ˢ����, ��ͨ��ˢ/������ˢ, �ⲿ����������ɫ��ʱ�����
    bool re_create_brush(LYRIC_DESKTOP_INFO* pWndInfo, bool isLight);

    // ���´����߿�ˢ
    bool re_create_border(LYRIC_DESKTOP_INFO* pWndInfo);

    // ���´����������, �ⲿ�������������ʱ�����
    bool re_create_font(LYRIC_DESKTOP_INFO* pWndInfo);

    // ���´���ͼƬ��Դ����
    bool re_create_image(LYRIC_DESKTOP_INFO* pWndInfo);

    // ���������豸��صĶ���, �������豸�޹ض���, ����ѡ���Ƿ�����
    bool destroy(bool isDestroyFont);
};

// ��ʴ���ͼƬ�ľ���
struct LYRIC_DESKTOP_IMAGE
{
    RECT rcNormal;
    RECT rcLight;
    RECT rcDown;
    RECT rcDisable;
};

// ��ť��Ϣ, ������ť��λ��, id����Ϣ
struct LYRIC_DESKTOP_BUTTON_INFO
{
    int     id;         // ��ť��ID, ͨ�����id�ҵ����ĸ�λ�ð�ͼƬ�ó����滭
    int     index;      // ��ť����, ��1��ʼ, ��ʾ��ʾ�ĵڼ�����ť, ��xml���˳���Ӧ
    int     state;      // ��ť״̬
    RECT    rc;         // ��ťʵ�ʵ�λ��, ��λ������, �ж�����ƶ������λ�þ��ڰ�ť��
    RECT* prcSrc;     // ��ť��Դ����, �Ӵ�ͼƬ�ϵ��ĸ�λ���ó����滭
};
struct LYRIC_DESKTOP_BUTTON
{
    std::vector<LYRIC_DESKTOP_BUTTON_INFO>  rcBtn;  // ��ťʵ�ʻ滭��λ��, id ����Ϣ
    std::vector<LYRIC_DESKTOP_IMAGE>        rcSrc;  // Դ����, �Ӵ�ͼƬ�ϵ��ĸ�λ���ó����滭

    int     index{ -1 };    // ��ť����, ��ǰ����ƶ������ĸ�������, ����������� rcBtn ���±�
    int     indexDown{ -1 };// ��������
    RECT    rc;             // ��ťʵ�ʻ滭��λ��, ��λ������, �ж�����ƶ������λ�þ��ڰ�ť��
    int     maxWidth{};     // ���ť�Ŀ��
    int     maxHeight{};    // ���ť�ĸ߶�

};

// �������, һ����������λͼ, һ������ͨ���, һ���Ǹ������, ��������, ����ֱ���趨�ü����Ϳ�����
// ������߷���ģʽ�������ж��и�������
// ��һ������ͨ��� + һ���и�����ʻ滭������λͼ��, ֻ�и�ʸı�Ż����»滭�ڶ���
struct LYRIC_DESKTOP_CACHE_OBJ
{
    int     preIndex;   // �ϴλ滭���к�����
    LPCWSTR preText;    // �ϴλ滭���ı���ַ, �кź��ı���һ���Ǿ��ǲ���Ҫ���´�������
    int     preLength;  // �ϴλ滭�ı��ĳ���

    D2D1_RECT_F rcBounds;       // ʵ�ʻ滭������
    ID2D1Bitmap* pBitmapNormal; // ����λͼ, ��ͨ����ı�, һ�λ���, ����ֱ���趨�ü����Ϳ�����
    ID2D1Bitmap* pBitmapLight;  // ����λͼ, ��������ı�

    LYRIC_DESKTOP_CACHE_OBJ();
    ~LYRIC_DESKTOP_CACHE_OBJ();
};

class CCriticalSection
{
public:
    CCriticalSection(LPCRITICAL_SECTION cs) : m_cs(cs)
    {
        Lock();
    }
    CCriticalSection(LPCRITICAL_SECTION cs, const std::try_to_lock_t&) : m_cs(cs)
    {
        Lock();
    }
    CCriticalSection(LPCRITICAL_SECTION cs, const std::adopt_lock_t&) : m_cs(cs)
    {
    }
    ~CCriticalSection()
    {
        Unlock();
    }
    bool TryLock()
    {
        is_unlock = TryEnterCriticalSection(m_cs);
        return is_unlock;
    }
    void Lock()
    {
        EnterCriticalSection(m_cs);
        is_unlock = true;
    }
    void Unlock()
    {
        if (is_unlock)
            LeaveCriticalSection(m_cs);
        m_cs = nullptr;
    }
private:
    LPCRITICAL_SECTION m_cs{};
    bool is_unlock{};
};

// ��¼�滭�ı���Ҫ������, ·��, ��Ӱ��ʽ����ʹ������ṹ, һ�ж�Ӧһ���ṹ
struct LYRIC_DESKTOP_DRAWTEXT_INFO
{
    LYRIC_LINE_STRUCT       line;   // �������Ϣ
    LYRIC_DESKTOP_CACHE_OBJ cache;  // �������ָ��

    int         index;              // ����к�, ��ǰ�滭���к�

    int         align;              // ����ģʽ, 0=�����, 1=���ж���, 2=�Ҷ���
    D2D1_RECT_F rcText;             // ����ı��滭��λ��, ���λ���Ǹ��ݶ���ģʽ�����

    float text_width;               // ����ı����, �ڻ滭ʱ�����, ���з��������, ������Ҫ����
    float text_height;              // ����ı��߶�, �ڻ滭ʱ�����, ����ÿ���ֵ�ʱ��ֻ��������ͨ���

    float nLightWidth;              // ��ʸ���λ��, ����0�Ļ�����Ҫ�滭��������
    float nLightHeight;             // ��ʸ���λ��, ����0�Ļ�����Ҫ�滭��������

    LYRIC_DESKTOP_DRAWTEXT_INFO()
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
        nLightHeight = 0.f;
        text_width = 0.f;
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

// ����λ����Ϣ, ����������ʹ�õ�λ�ò�һ��
struct LYRIC_DESKTOP_POS : RECT
{
    int width;
    int height;
};

// ������Ϣ, ����ʹ��, �������߹ر�һЩ����, ����鿴Ч��
typedef struct LYRIC_DESKTOP_CONFIG_DEBUG
{
    DWORD       clrTextBackNormal;   // ��ͨ����ı�������ɫ
    DWORD       clrTextBackLight;    // ��������ı�������ɫ

}*PLYRIC_DESKTOP_CONFIG_DEBUG;

// ������Ϣ, �����õ�������, �������õ�, ��д������, ��ʱ������һ��json, ���ⲿ����
typedef struct LYRIC_DESKTOP_CONFIG
{
    int         refreshRate;    // ˢ����, ˢ�¸�ʵ�Ƶ��, ������ˢ������ 30, 60, 75, 90, 100, 120, 144, 165, 240

    LYRIC_DESKTOP_CONFIG_DEBUG debug;   // ����������Ϣ
}*PLYRIC_DESKTOP_CONFIG;

// ��ʴ��� USERDATA ���ŵ�������ṹ
typedef struct LYRIC_DESKTOP_INFO
{
    long        nAddref;        // ���ü���
    HWND        hWnd;           // ��ʴ��ھ��
    HWND        hTips;          // ��ʾ���ھ��
    HLYRIC      hLyric;         // ��ʾ��
    int         prevIndexLine;  // ��һ�λ滭�ĸ���к�
    float       prevWidth;      // ��һ�λ滭�ĸ�ʿ��
    float       prevHeight;     // ��һ�λ滭�ĸ�ʿ��
    float       word_width;     // һ�����ֵĿ��, ����ʹ��
    float       word_height;    // һ�����ֵĸ߶�
    float       nLineDefWidth;  // û�и��ʱ��ʵ�Ĭ�Ͽ��
    float       nLineDefHeight; // û�и��ʱ��ʵ�Ĭ�ϸ߶�, ����ʹ��
    LPCWSTR     pszDefText;     // û�и��ʱ��Ĭ���ı�
    int         nDefText;       // Ĭ���ı�����
    int         nCurrentTimeMS; // ��ǰ��ʲ���ʱ��
    int         nTimeOffset;    // ʱ��ƫ��, ��ʾ��ʾʱʹ��
    int         nMinWidth;      // ��ʴ�����С���, ���а�ť�Ŀ�� ����һЩ�߾�
    int         nMinHeight;     // ��ʴ�����С�߶�
    int         nLineTop1;      // ��һ�и�ʵĶ���λ��
    int         nLineTop2;      // �ڶ��и�ʵĶ���λ��
    RECT        rcWindow;       // ��ʴ��ڵ�λ��, �������ڶ��ǿͻ���, �����¼������Ļλ��, �滭ʱ��λ��, ����֤�ǵ�ǰ���ڵ�λ��
    RECT        rcMonitor;      // ������ʾ���ϲ���ľ���
    
    float       padding_text;   // ���4���ߵļ��, ����߾���Ԥ��������/��Ӱ �����ķ�Χ
    float       padding_wnd;    // ����4���ߵļ��, �����Χ����, �������ݻ滭�����ڱ���

    float       shadowRadius;   // ��Ӱ�뾶
    LYRIC_MODE  mode;           // �����ʾģʽ

    LYRIC_DESKTOP_POS pos_h;    // ����ģʽ�µĴ���λ��
    LYRIC_DESKTOP_POS pos_v;    // ����ģʽ�µĴ���λ��

    LPCRITICAL_SECTION pCritSec;// ��ʼ��ص��ٽ���, ��ֹ���߳��ͷ��˸��, Ȼ�󴰿��߳�ȥ��ѯ
    
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
            USHORT  change_text : 1;    // �����иı�, ��Ҫ�ڻ滭����ı���ʱ�����´�������
        };    // �д��ڲ����иı�ķ�����, �����ֵΪ���ʱ��������߽����ػ�
        USHORT  change;
    };

    LYRIC_DESKTOP_CONFIG        config;     // ��ʵ�������Ϣ, �������ö�������

    LYRIC_DESKTOP_DRAWTEXT_INFO line1;      // ��һ�и����Ϣ, �����������, �������Ϣ, ����ı��滭λ��, ��ʸ���λ�õ�
    LYRIC_DESKTOP_DRAWTEXT_INFO line2;      // �ڶ��и����Ϣ

    LYRIC_DESKTOP_BUTTON        button;
    PFN_LYRIC_DESKTOP_COMMAND   pfnCommand; // ��ʴ����ϵİ�ť������ص�����
    LPARAM                      lParam;     // ��ʴ����ϵİ�ť������ص������Ĳ���
    LOGFONTW                    lf{};       // ������Ϣ
    LYRIC_DESKTOP_DX            dx;         // dx��صĶ���
    std::vector<DWORD>          clrNormal;  // ��ͨ��ʻ�ˢ��ɫ��
    std::vector<DWORD>          clrLight;   // ������ʻ�ˢ��ɫ��
    DWORD                       clrBorder;  // ����ı��߿���ɫ
    CScale                      scale;      // ���ű���
    std::vector<RECT>           rcMonitors; // ������ʾ���ľ���, ��¼ÿ����Ļ��λ��, ���ƴ����ƶ���Χ
    LYRIC_DESKTOP_INFO();
    int Addref()
    {
        return InterlockedIncrement(&nAddref);
    }
    int Release()
    {
        int nRet = InterlockedDecrement(&nAddref);
        if (nRet == 0)
        {
            lyric_destroy(hLyric);
            DestroyWindow(hTips);
            DeleteCriticalSection(pCritSec);
            delete pCritSec;
            delete this;
        }
        return nRet;
    }

    void set_def_arg(const LYRIC_DESKTOP_ARG* arg);

    // DPI�ı�ʱ����, �����¼�������, λ��ƫ�Ƶ���Ϣ
    void dpi_change(HWND hWnd);

    void get_monitor();

    // �жϻ滭��ʻ����Ŀ��/�߶�, �������ص��ǿ��, �������ص��Ǹ߶�
    float get_lyric_line_height() const;
    // ���ݸ���ı����/�߶�, ���ػ�����Ҫ�Ŀ��, 
    float get_lyric_line_width(float vl) const;


}*PLYRIC_DESKTOP_INFO;


NAMESPACE_LYRIC_DESKTOP_END

