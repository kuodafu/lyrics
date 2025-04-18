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
// 歌词窗口dx相关的对象
struct LYRIC_WND_DX
{
    NAMESPACE_D2D::CD2DRender*  hCanvas;        // D2D绘画句柄
    NAMESPACE_D2D::CD2DFont*    hFont;          // 绘画歌词的字体, 这个是设备无关字体, 设备失效不需要重新创建

    NAMESPACE_D2D::CD2DImage*   image;          // 歌词窗口按钮需要的图片
    NAMESPACE_D2D::CD2DBrush*   hbrBorder;      // 绘画歌词文本的边框画刷
    NAMESPACE_D2D::CD2DBrush*   hbrLine;        // 歌词按钮分隔部分的线条画刷
    NAMESPACE_D2D::CD2DBrush_LinearGradient* hbrNormal;      // 普通歌词画刷
    NAMESPACE_D2D::CD2DBrush_LinearGradient* hbrLight;       // 高亮歌词画刷

    DWORD       clrBack;        // 鼠标移动上来之后显示的歌词ARGB背景颜色


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

    // 重新创建所有对象
    bool re_create(LYRIC_WND_INFU* pWndInfo);
    
    // 重新创建画刷对象, 普通画刷/高亮画刷, 外部重新设置颜色的时候调用
    bool re_create_brush(LYRIC_WND_INFU* pWndInfo, bool isLight);

    // 重新创建边框画刷
    bool re_create_border(LYRIC_WND_INFU* pWndInfo);

    // 重新创建字体对象, 外部重新设置字体的时候调用
    bool re_create_font(LYRIC_WND_INFU* pWndInfo);

    // 重新创建图片资源对象
    bool re_create_image(LYRIC_WND_INFU* pWndInfo);

    // 销毁所有设备相关的对象, 字体是设备无关对象, 可以选择是否销毁
    bool destroy(bool isDestroyFont);
};

// 歌词窗口图片的矩形
struct LYRIC_WND_IMAGE
{
    RECT rcNormal;
    RECT rcLight;
    RECT rcDown;
    RECT rcDisable;
};

// 按钮信息, 包括按钮的位置, id等信息
struct LYRIC_WND_BUTTON_INFO
{
    int     id;         // 按钮的ID, 通过这个id找到从哪个位置把图片拿出来绘画
    int     index;      // 按钮索引, 从1开始, 表示显示的第几个按钮, 和xml里的顺序对应
    int     state;      // 按钮状态
    RECT    rc;         // 按钮实际的位置, 单位是像素, 判断鼠标移动到这个位置就在按钮上
    RECT*   prcSrc;     // 按钮的源矩形, 从大图片上的哪个位置拿出来绘画
};
struct LYRIC_WND_BUTTON
{
    std::vector<LYRIC_WND_BUTTON_INFO>  rcBtn;  // 按钮实际绘画的位置, id 等信息
    std::vector<LYRIC_WND_IMAGE>        rcSrc;  // 源矩形, 从大图片上的哪个位置拿出来绘画

    int     index{-1};    // 按钮索引, 当前鼠标移动到了哪个索引上, 这个索引就是 rcBtn 的下标
    int     indexDown{-1};// 按下索引
    int     width{};      // 所有按钮的宽度
    int     maxHeight{};  // 最大按钮的高度

};



// 歌词窗口 USERDATA 里存放的是这个结构
typedef struct LYRIC_WND_INFU
{
    HWND        hWnd;           // 歌词窗口句柄
    HWND        hTips;          // 提示窗口句柄
    HLYRIC      hLyric;         // 歌词句柄
    int         prevIndexLine;  // 上一次绘画的歌词行号
    int         prevWidth;      // 上一次绘画的歌词宽度
    float       nLineHeight;    // 一行歌词的高度
    int         nCurrentTimeMS; // 当前歌词播放时间
    int         nTimeOffset;    // 时间偏移, 计算歌词位置的时候加上这个偏移
    int         nMinWidth;      // 歌词窗口最小宽度, 所有按钮的宽度 加上一些边距
    int         nMinHeight;     // 歌词窗口最小高度
    int         nLineTop1;      // 第一行歌词的顶部位置
    int         nLineTop2;      // 第二行歌词的顶部位置
    union
    {
        struct
        {
            USHORT  isFillBack : 1; // 是否填充背景
            USHORT  isLock : 1;     // 歌词是否已经锁定
        };
        USHORT  status; // 状态都放这里, 方便管理
    };
    union
    {
        struct
        {
            USHORT  change_wnd : 1; // 窗口是否有改变
            USHORT  change_btn : 1; // 按钮部分是否有改变, 热点改变/按下改变等
        };    // 有窗口部分有改变的放这里, 里面的值为真的时候会往下走进行重画
        USHORT  change;
    };


    LYRIC_WND_BUTTON        button;
    PFN_LYRIC_WND_COMMAND   pfnCommand; // 歌词窗口上的按钮被点击回调函数
    LPARAM                  lParam;     // 歌词窗口上的按钮被点击回调函数的参数
    LOGFONTW                lf{};       // 字体信息
    LYRIC_WND_DX            dx;         // dx相关的对象
    std::vector<DWORD>      clrNormal;  // 普通歌词画刷颜色组
    std::vector<DWORD>      clrLight;   // 高亮歌词画刷颜色组
    DWORD                   clrBorder;  // 歌词文本边框颜色
    CScale                  scale;      // 缩放比例
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

// 让歌词窗口失效, 然后重画
// isUpdate = 为true的时候始终重画, 为false的时候会判断是否需要绘画
bool lyric_wnd_invalidate(LYRIC_WND_INFU& wnd_info);

// 加载程序资源里的图片, 然后把各个坐标都记录好
bool lyric_wnd_load_image(LYRIC_WND_INFU& wnd_info);

// 绘画歌词窗口需要的按钮
void lyric_wnd_draw_button(LYRIC_WND_INFU& wnd_info, const RECT& rcWindow, LYRIC_CALC_STRUCT& arg);

// 歌词窗口上的按钮被点击
void lyric_wnd_button_click(LYRIC_WND_INFU& wnd_info);

// 调用指定事件
bool lyric_wnd_call_event(LYRIC_WND_INFU& wnd_info, int id);

bool lyric_wnd_set_btn_state(LYRIC_WND_INFU& wnd_info, int id, LYRIC_WND_BUTTON_STATE state);
LYRIC_WND_BUTTON_STATE lyric_wnd_get_btn_state(LYRIC_WND_INFU& wnd_info, int id);

// 鼠标移动到按钮上, 显示提示信息
void lyric_wnd_button_hover(LYRIC_WND_INFU& wnd_info);
// 鼠标离开按钮, 隐藏提示信息
void lyric_wnd_button_leave(LYRIC_WND_INFU& wnd_info);

// 计算需要绘画的按钮的总宽度, 计算好宽度后可以让所有按钮居中
int lyric_wnd_calc_button(LYRIC_WND_INFU& wnd_info, int& maxHeight, int offset);



NAMESPACE_LYRIC_WND_END