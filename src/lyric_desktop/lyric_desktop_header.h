#pragma once
#include <d2d/d2d_interface.h>
#include <CScale.h>
#include <kuodafu_lyric.h>
#include <atlbase.h>
#include "lyric_desktop_config.h"
#include "lyric_desktop_typedef.h"

NAMESPACE_LYRIC_DESKTOP_BEGIN

// 歌词窗口dx相关的对象
struct LYRIC_DESKTOP_DX
{
    ID2D1GdiInteropRenderTarget*  pGDIInterop;              // 
    KUODAFU_NAMESPACE::D2DRender* pRender;                  // D2D绘画句柄
    KUODAFU_NAMESPACE::D2DFont*   hFont;                    // 绘画歌词的字体, 这个是设备无关字体, 设备失效不需要重新创建

    KUODAFU_NAMESPACE::D2DImage* image_button;              // 歌词窗口按钮需要的图片
    KUODAFU_NAMESPACE::D2DImage* image_shadow;              // 阴影图片
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrBorderNormal;      // 绘画普通歌词文本的边框画刷
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrBorderLight;       // 绘画高亮歌词文本的边框画刷
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrWndBorder;         // 歌词窗口的边框画刷
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrWndBack;           // 歌词窗口的背景画刷
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrLine;              // 歌词按钮分隔部分的线条画刷
    KUODAFU_NAMESPACE::D2DLinearGradientBrush* hbrNormal;   // 普通歌词画刷
    KUODAFU_NAMESPACE::D2DLinearGradientBrush* hbrLight;    // 高亮歌词画刷

    ID2D1Bitmap* pBitmapBack;   // 缓存位图, 窗口背景, 尺寸改变的时候需要重新创建


    LYRIC_DESKTOP_DX()
    {
        pGDIInterop = nullptr;
        hFont = nullptr;
        pRender = nullptr;
        image_button = nullptr;
        image_shadow = nullptr;
        hbrBorderNormal = nullptr;
        hbrBorderLight = nullptr;
        hbrWndBorder = nullptr;
        hbrWndBack = nullptr;
        hbrLine = nullptr;
        hbrNormal = nullptr;
        hbrLight = nullptr;
        pBitmapBack = nullptr;
    }

    ~LYRIC_DESKTOP_DX()
    {
        destroy(true);
    }

    // 配置已经初始化了, 根据配置创建dx对象
    void init(LYRIC_DESKTOP_INFO* pWndInfo);

    // 重新创建所有对象
    bool re_create(LYRIC_DESKTOP_INFO* pWndInfo);

    // 重新创建画刷对象, 普通画刷/高亮画刷, 外部重新设置颜色的时候调用
    bool re_create_brush(LYRIC_DESKTOP_INFO* pWndInfo, bool isLight);

    // 重新创建边框画刷
    bool re_create_brush(KUODAFU_NAMESPACE::D2DSolidBrush*& hbr, DWORD argb);

    // 重新创建字体对象, 外部重新设置字体的时候调用
    bool re_create_font(LYRIC_DESKTOP_INFO* pWndInfo);

    // 重新创建图片资源对象
    bool re_create_image(LYRIC_DESKTOP_INFO* pWndInfo);

    // 销毁所有设备相关的对象, 字体是设备无关对象, 可以选择是否销毁
    bool destroy(bool isDestroyFont);
};

// 歌词窗口图片的矩形
struct LYRIC_DESKTOP_IMAGE
{
    RECT rcNormal;
    RECT rcLight;
    RECT rcDown;
    RECT rcDisable;
};

// 按钮信息, 包括按钮的位置, id等信息
struct LYRIC_DESKTOP_BUTTON_INFO
{
    int     id;         // 按钮的ID, 通过这个id找到从哪个位置把图片拿出来绘画
    int     index;      // 按钮索引, 从1开始, 表示显示的第几个按钮, 和xml里的顺序对应
    int     state;      // 按钮状态
    RECT    rc;         // 按钮实际的位置, 单位是像素, 判断鼠标移动到这个位置就在按钮上
    RECT*   prcSrc;     // 按钮的源矩形, 从大图片上的哪个位置拿出来绘画
};
struct LYRIC_DESKTOP_BUTTON
{
    std::vector<LYRIC_DESKTOP_BUTTON_INFO>  rcBtn;  // 按钮实际绘画的位置, id 等信息
    std::vector<LYRIC_DESKTOP_IMAGE>        rcSrc;  // 源矩形, 从大图片上的哪个位置拿出来绘画

    int     index{ -1 };    // 按钮索引, 当前鼠标移动到了哪个索引上, 这个索引就是 rcBtn 的下标
    int     indexDown{ -1 };// 按下索引
    RECT    rc{};           // 按钮实际绘画的位置, 单位是像素, 判断鼠标移动到这个位置就在按钮上
    int     maxWidth{};     // 最大按钮的宽度
    int     maxHeight{};    // 最大按钮的高度

};

// 缓存对象, 一行两个缓存位图, 一个是普通歌词, 一个是高亮歌词, 缓存起来, 后面直接设定裁剪区就可以了
// 音译或者翻译模式下是两行都有高亮部分
// 把一整行普通歌词 + 一整行高亮歌词绘画到缓存位图上, 只有歌词改变才会重新绘画第二次
struct LYRIC_DESKTOP_CACHE_OBJ
{
    int     preIndex;               // 上次绘画的行号索引
    LPCWSTR preText;                // 上次绘画的文本地址, 行号和文本都一样那就是不需要重新创建对象
    int     preLength;              // 上次绘画文本的长度

    D2D1_RECT_F rcBounds;           // 实际绘画的区域
    ID2D1Bitmap* pBitmapNormal;     // 缓存位图, 普通歌词文本, 一次画好, 后面直接设定裁剪区就可以了
    ID2D1Bitmap* pBitmapLight;      // 缓存位图, 高亮歌词文本

    void init()
    {
        pBitmapNormal = nullptr;
        pBitmapLight = nullptr;
        clear();
    }
    void clear()
    {
        preIndex = -1;
        preText = nullptr;
        preLength = 0;
        rcBounds = { 0 };

    }

    ~LYRIC_DESKTOP_CACHE_OBJ()
    {
        KUODAFU_NAMESPACE::SafeRelease(pBitmapNormal);
        KUODAFU_NAMESPACE::SafeRelease(pBitmapLight);
    }
};

// 记录绘画文本需要的数据, 路径, 阴影方式都是使用这个结构, 一行对应一个结构
struct LYRIC_DESKTOP_DRAWTEXT_INFO
{
    LYRIC_LINE_STRUCT       line;   // 歌词行信息
    LYRIC_DESKTOP_CACHE_OBJ cache;  // 缓存对象指针

    int         index;              // 歌词行号, 当前绘画的行号

    int         align;              // 对齐模式, 0=左对齐, 1=居中对齐, 2=右对齐
    D2D1_RECT_F rcText;             // 歌词文本绘画的位置, 这个位置是根据对齐模式计算的

    float text_width;               // 歌词文本宽度, 在绘画时会计算, 会有翻译和音译, 所以需要计算
    float text_height;              // 歌词文本高度, 在绘画时会计算, 计算每个字的时候只计算了普通歌词

    float nLightWidth;              // 歌词高亮位置, 大于0的话就是要绘画高亮区域
    float nLightHeight;             // 歌词高亮位置, 大于0的话就是要绘画高亮区域

    // 初始化歌词行信息, 传递这一行的对齐模式
    // 后续的绘画都是根据这个对齐模式计算文本位置
    void init(int algin)
    {
        this->align = algin;
        cache.init();
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
        cache.clear();
    }
};


// 歌词窗口 USERDATA 里存放的是这个结构
typedef struct LYRIC_DESKTOP_INFO
{
    long        nAddref;        // 引用计数
    HWND        hWnd;           // 歌词窗口句柄
    HWND        hTips;          // 提示窗口句柄
    HLYRIC      hLyric;         // 歌词句柄
    int         prevIndexLine;  // 上一次绘画的歌词行号
    float       prevWidth;      // 上一次绘画的歌词宽度
    float       prevHeight;     // 上一次绘画的歌词宽度
    float       word_width;     // 一个汉字的宽度, 竖屏使用
    float       word_height;    // 一个汉字的高度
    float       nLineDefWidth;  // 没有歌词时歌词的默认宽度
    float       nLineDefHeight; // 没有歌词时歌词的默认高度, 竖屏使用
    int         nCurrentTimeMS; // 当前歌词播放时间
    int         nTimeOffset;    // 时间偏移, 显示提示时使用
    int         nMinWidth;      // 歌词窗口最小宽度, 所有按钮的宽度 加上一些边距
    int         nMinHeight;     // 歌词窗口最小高度
    int         nLineTop1;      // 第一行歌词的顶部位置
    int         nLineTop2;      // 第二行歌词的顶部位置
    RECT        rcWindow;       // 歌词窗口的位置, 整个窗口都是客户区, 这里记录的是屏幕位置, 绘画时的位置, 不保证是当前窗口的位置
    RECT        rcMonitor;      // 所有显示器合并后的矩形
    
    float       shadowRadius;   // 阴影半径
    LYRIC_DESKTOP_MODE  mode;   // 歌词显示模式, LYRIC_DESKTOP_MODE 枚举类型

    LPCRITICAL_SECTION pCritSec;// 歌词加载的临界区, 防止有线程释放了歌词, 然后窗口线程去查询
    
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
            USHORT  change_wnd : 1;     // 窗口是否有改变
            USHORT  change_btn : 1;     // 按钮部分是否有改变, 热点改变/按下改变等
            USHORT  change_text : 1;    // 字体有改变, 需要在绘画歌词文本的时候重新创建缓存
        };    // 有窗口部分有改变的放这里, 里面的值为真的时候会往下走进行重画
        USHORT  change;
    };


    LYRIC_DESKTOP_DRAWTEXT_INFO line1;      // 第一行歌词信息, 包括缓存对象, 歌词行信息, 歌词文本绘画位置, 歌词高亮位置等
    LYRIC_DESKTOP_DRAWTEXT_INFO line2;      // 第二行歌词信息

    LYRIC_DESKTOP_CONFIG        config;     // 歌词的配置信息, 所有配置都在这里

    LYRIC_DESKTOP_BUTTON        button;
    PFN_LYRIC_DESKTOP_COMMAND   pfnCommand; // 歌词窗口上的按钮被点击回调函数
    LPARAM                      lParam;     // 歌词窗口上的按钮被点击回调函数的参数
    LYRIC_DESKTOP_DX            dx;         // dx相关的对象
    CScale                      scale;      // 缩放比例
    std::vector<RECT>           rcMonitors; // 所有显示器的矩形, 记录每个屏幕的位置, 限制窗口移动范围

    // 初始化结构, 初始化配置, 初始化DX, 后续所有操作都是从这个结构进行的
    // hWnd = 显示桌面歌词的窗口句柄
    // argJson = 桌面歌词配置json字符串
    // pfnCommand = 歌词窗口上的按钮被点击回调函数
    // lParam = 歌词窗口上的按钮被点击回调函数的参数
    void init(HWND hWnd, const char* argJson, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam);

    
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

    bool has_mode(LYRIC_DESKTOP_MODE flag) const
    {
        using T = std::underlying_type_t<LYRIC_DESKTOP_MODE>;
        return (static_cast<T>(mode) & static_cast<T>(flag)) != 0;
    }
    void add_mode(LYRIC_DESKTOP_MODE flag)
    {
        using T = std::underlying_type_t<LYRIC_DESKTOP_MODE>;
        mode = static_cast<LYRIC_DESKTOP_MODE>(static_cast<T>(mode) | static_cast<T>(flag));
    }
    void del_mode(LYRIC_DESKTOP_MODE flag)
    {
        using T = std::underlying_type_t<LYRIC_DESKTOP_MODE>;
        mode = static_cast<LYRIC_DESKTOP_MODE>(static_cast<T>(mode) & ~static_cast<T>(flag));
    }

    // DPI改变时调用, 会重新计算字体, 位置偏移等信息
    void dpi_change(HWND hWnd);

    void get_monitor();

    // 判断绘画歌词画布的宽度/高度, 竖屏返回的是宽度, 横屏返回的是高度
    float get_lyric_line_height() const;
    // 传递歌词文本宽度/高度, 返回画布需要的宽度, 
    float get_lyric_line_width(float vl) const;


}*PLYRIC_DESKTOP_INFO;


NAMESPACE_LYRIC_DESKTOP_END

