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
// 歌词窗口dx相关的对象
struct LYRIC_DESKTOP_DX
{
    ID2D1GdiInteropRenderTarget* pGDIInterop;
    KUODAFU_NAMESPACE::D2DRender* hCanvas;         // D2D绘画句柄
    KUODAFU_NAMESPACE::CD2DFont* hFont;             // 绘画歌词的字体, 这个是设备无关字体, 设备失效不需要重新创建

    KUODAFU_NAMESPACE::D2DImage* image;        // 歌词窗口按钮需要的图片
    KUODAFU_NAMESPACE::D2DImage* image_shadow; // 阴影图片
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrBorder;        // 绘画歌词文本的边框画刷
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrWndBorder;     // 歌词窗口的边框画刷
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrWndBack;       // 歌词窗口的背景画刷
    KUODAFU_NAMESPACE::D2DSolidBrush* hbrLine;          // 歌词按钮分隔部分的线条画刷
    KUODAFU_NAMESPACE::D2DLinearGradientBrush* hbrNormal;      // 普通歌词画刷
    KUODAFU_NAMESPACE::D2DLinearGradientBrush* hbrLight;       // 高亮歌词画刷

    DWORD       clrBack;        // 鼠标移动上来之后显示的歌词ARGB背景颜色
    DWORD       clrWndBorder;   // 鼠标移动上来之后显示的歌词ARGB边框颜色

    ID2D1Bitmap* pBitmapBack;   // 缓存位图, 窗口背景, 尺寸改变的时候需要重新创建


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

    // 重新创建所有对象
    bool re_create(LYRIC_DESKTOP_INFO* pWndInfo);

    // 重新创建画刷对象, 普通画刷/高亮画刷, 外部重新设置颜色的时候调用
    bool re_create_brush(LYRIC_DESKTOP_INFO* pWndInfo, bool isLight);

    // 重新创建边框画刷
    bool re_create_border(LYRIC_DESKTOP_INFO* pWndInfo);

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
    RECT* prcSrc;     // 按钮的源矩形, 从大图片上的哪个位置拿出来绘画
};
struct LYRIC_DESKTOP_BUTTON
{
    std::vector<LYRIC_DESKTOP_BUTTON_INFO>  rcBtn;  // 按钮实际绘画的位置, id 等信息
    std::vector<LYRIC_DESKTOP_IMAGE>        rcSrc;  // 源矩形, 从大图片上的哪个位置拿出来绘画

    int     index{ -1 };    // 按钮索引, 当前鼠标移动到了哪个索引上, 这个索引就是 rcBtn 的下标
    int     indexDown{ -1 };// 按下索引
    RECT    rc;             // 按钮实际绘画的位置, 单位是像素, 判断鼠标移动到这个位置就在按钮上
    int     maxWidth{};     // 最大按钮的宽度
    int     maxHeight{};    // 最大按钮的高度

};

// 缓存对象, 一行两个缓存位图, 一个是普通歌词, 一个是高亮歌词, 缓存起来, 后面直接设定裁剪区就可以了
// 音译或者翻译模式下是两行都有高亮部分
// 把一整行普通歌词 + 一整行高亮歌词绘画到缓存位图上, 只有歌词改变才会重新绘画第二次
struct LYRIC_DESKTOP_CACHE_OBJ
{
    int     preIndex;   // 上次绘画的行号索引
    LPCWSTR preText;    // 上次绘画的文本地址, 行号和文本都一样那就是不需要重新创建对象
    int     preLength;  // 上次绘画文本的长度

    D2D1_RECT_F rcBounds;       // 实际绘画的区域
    ID2D1Bitmap* pBitmapNormal; // 缓存位图, 普通歌词文本, 一次画好, 后面直接设定裁剪区就可以了
    ID2D1Bitmap* pBitmapLight;  // 缓存位图, 高亮歌词文本

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
    DOUBLE_ROW      = 0x0000,   // 双行歌词
    TRANSLATION1    = 0x0001,   // 翻译
    TRANSLATION2    = 0x0002,   // 音译
    SINGLE_ROW      = 0x0004,   // 单行显示

    VERTICAL        = 0x10000,  // 竖屏模式
    EXISTTRANS      = 0x20000,  // 存在翻译, 存在这个就判断歌词对齐模式, 否则默认双行

};

// 窗口位置信息, 横屏和竖屏使用的位置不一样
struct LYRIC_DESKTOP_POS : RECT
{
    int width;
    int height;
};

// 配置信息, 调试使用, 开启或者关闭一些功能, 方便查看效果
typedef struct LYRIC_DESKTOP_CONFIG_DEBUG
{
    DWORD       clrTextBackNormal;   // 普通歌词文本背景颜色
    DWORD       clrTextBackLight;    // 高亮歌词文本背景颜色

}*PLYRIC_DESKTOP_CONFIG_DEBUG;

// 配置信息, 所有用到的配置, 可以设置的, 都写在这里, 到时候生成一个json, 让外部保存
typedef struct LYRIC_DESKTOP_CONFIG
{
    int         refreshRate;    // 刷新率, 刷新歌词的频率, 主流的刷新率是 30, 60, 75, 90, 100, 120, 144, 165, 240

    LYRIC_DESKTOP_CONFIG_DEBUG debug;   // 调试配置信息
}*PLYRIC_DESKTOP_CONFIG;

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
    LPCWSTR     pszDefText;     // 没有歌词时的默认文本
    int         nDefText;       // 默认文本长度
    int         nCurrentTimeMS; // 当前歌词播放时间
    int         nTimeOffset;    // 时间偏移, 显示提示时使用
    int         nMinWidth;      // 歌词窗口最小宽度, 所有按钮的宽度 加上一些边距
    int         nMinHeight;     // 歌词窗口最小高度
    int         nLineTop1;      // 第一行歌词的顶部位置
    int         nLineTop2;      // 第二行歌词的顶部位置
    RECT        rcWindow;       // 歌词窗口的位置, 整个窗口都是客户区, 这里记录的是屏幕位置, 绘画时的位置, 不保证是当前窗口的位置
    RECT        rcMonitor;      // 所有显示器合并后的矩形
    
    float       padding_text;   // 歌词4个边的间距, 这个边就是预留给发光/阴影 超出的范围
    float       padding_wnd;    // 窗口4个边的间距, 这个范围留空, 不让内容绘画到窗口边上

    float       shadowRadius;   // 阴影半径
    LYRIC_MODE  mode;           // 歌词显示模式

    LYRIC_DESKTOP_POS pos_h;    // 横屏模式下的窗口位置
    LYRIC_DESKTOP_POS pos_v;    // 竖屏模式下的窗口位置

    LPCRITICAL_SECTION pCritSec;// 歌词加载的临界区, 防止有线程释放了歌词, 然后窗口线程去查询
    
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

    LYRIC_DESKTOP_CONFIG        config;     // 歌词的配置信息, 所有配置都在这里

    LYRIC_DESKTOP_DRAWTEXT_INFO line1;      // 第一行歌词信息, 包括缓存对象, 歌词行信息, 歌词文本绘画位置, 歌词高亮位置等
    LYRIC_DESKTOP_DRAWTEXT_INFO line2;      // 第二行歌词信息

    LYRIC_DESKTOP_BUTTON        button;
    PFN_LYRIC_DESKTOP_COMMAND   pfnCommand; // 歌词窗口上的按钮被点击回调函数
    LPARAM                      lParam;     // 歌词窗口上的按钮被点击回调函数的参数
    LOGFONTW                    lf{};       // 字体信息
    LYRIC_DESKTOP_DX            dx;         // dx相关的对象
    std::vector<DWORD>          clrNormal;  // 普通歌词画刷颜色组
    std::vector<DWORD>          clrLight;   // 高亮歌词画刷颜色组
    DWORD                       clrBorder;  // 歌词文本边框颜色
    CScale                      scale;      // 缩放比例
    std::vector<RECT>           rcMonitors; // 所有显示器的矩形, 记录每个屏幕的位置, 限制窗口移动范围
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

    // DPI改变时调用, 会重新计算字体, 位置偏移等信息
    void dpi_change(HWND hWnd);

    void get_monitor();

    // 判断绘画歌词画布的宽度/高度, 竖屏返回的是宽度, 横屏返回的是高度
    float get_lyric_line_height() const;
    // 传递歌词文本宽度/高度, 返回画布需要的宽度, 
    float get_lyric_line_width(float vl) const;


}*PLYRIC_DESKTOP_INFO;


NAMESPACE_LYRIC_DESKTOP_END

