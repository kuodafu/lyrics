#pragma once
#include <windows.h>

// 歌词窗口按钮ID, 目前就这几个默认按钮
enum LYRIC_WND_BUTTON_ID
{
    LYRIC_WND_BUTTON_ID_FIRST           = 1001, // 第一个按钮的索引

    LYRIC_WND_BUTTON_ID_TRANSLATE2      = 1001, // 音译按钮
    LYRIC_WND_BUTTON_ID_TRANSLATE2_SEL  = 1002, // 音译按钮, 选中模式
    LYRIC_WND_BUTTON_ID_TRANSLATE1      = 1003, // 翻译按钮
    LYRIC_WND_BUTTON_ID_TRANSLATE1_SEL  = 1004, // 翻译按钮, 选中模式
    LYRIC_WND_BUTTON_ID_LRCWRONG        = 1005, // 歌词不对
    LYRIC_WND_BUTTON_ID_VERTICAL        = 1006, // 竖屏按钮
    LYRIC_WND_BUTTON_ID_MAKELRC         = 1007, // 制作歌词

    LYRIC_WND_BUTTON_ID_FONT_DOWN       = 1008, // 字体减小
    LYRIC_WND_BUTTON_ID_FONT_UP         = 1009, // 字体增加
    LYRIC_WND_BUTTON_ID_BEHIND          = 1010, // 歌词延后
    LYRIC_WND_BUTTON_ID_AHEAD           = 1011, // 歌词提前
    LYRIC_WND_BUTTON_ID_LOCK            = 1012, // 锁定按钮
    LYRIC_WND_BUTTON_ID_SETTING         = 1013, // 设置按钮
    LYRIC_WND_BUTTON_ID_UNLOCK          = 1014, // 解锁按钮
    LYRIC_WND_BUTTON_ID_CLOSE           = 1015, // 关闭按钮
    LYRIC_WND_BUTTON_ID_LRCCOLOR        = 1016, // 设置字体颜色, 田字的按钮图标
    LYRIC_WND_BUTTON_ID_MENU            = 1017, // 菜单按钮

    LYRIC_WND_BUTTON_ID_LRCWRONG_V      = 1018, // 歌词不对, 纵向的按钮图标
    LYRIC_WND_BUTTON_ID_HORIZONTAL      = 1019, // 横屏按钮
    LYRIC_WND_BUTTON_ID_PLAY            = 1020, // 播放, 回调函数返回0后会变成暂停按钮
    LYRIC_WND_BUTTON_ID_PAUSE           = 1021, // 暂停, 回调函数返回0后会变成播放按钮
    LYRIC_WND_BUTTON_ID_PREV            = 1022, // 上一首
    LYRIC_WND_BUTTON_ID_NEXT            = 1023, // 下一首


    LYRIC_WND_BUTTON_ID_COUNT = LYRIC_WND_BUTTON_ID_NEXT - LYRIC_WND_BUTTON_ID_FIRST + 1,  // 按钮数量

    LYRIC_WND_BUTTON_ID_MAKELRCV        = 1024, // 制作歌词, 纵向按钮, 这个没弄对..... 目前也用不上
    LYRIC_WND_BUTTON_ID_SHOW            = 1025, // 显示歌词, 没这个按钮, 但是有这个事件, 可以让外部调用, 内部没这个按钮, 内部不会主动触发



};

enum LYRIC_WND_BUTTON_STATE
{
    LYRIC_WND_BUTTON_STATE_NORMAL   = 0,    // 正常状态
    LYRIC_WND_BUTTON_STATE_HOVER    = 1,    // 鼠标悬停
    LYRIC_WND_BUTTON_STATE_PUSHED   = 2,    // 鼠标按下
    LYRIC_WND_BUTTON_STATE_DISABLE = 4,    // 禁用状态

    LYRIC_WND_BUTTON_STATE_ERROR    = -1,   // 获取失败
};

typedef struct LYRIC_WND_ARG
{
    RECT        rcWindow;       // 要设置的窗口位置和大小
    DWORD       clrBackground;  // 背景颜色
    int         nFontSize;      // 字体大小
    LPCWSTR     pszFontName;    // 字体名称
    DWORD       clrBorder;      // 边框颜色

    DWORD*      pClrNormal;     // 普通歌词颜色数组
    int         nClrNormal;     // 普通歌词颜色数组长度

    DWORD*      pClrLight;      // 高亮歌词颜色数组
    int         nClrLight;      // 高亮歌词颜色数组长度

}*PLYRIC_WND_ARG;

// 歌词窗口按钮被点击事件, 返回0表示事件放行, 返回非0表示拦截事件
typedef int (CALLBACK* PFN_LYRIC_WND_COMMAND)(HWND hWindowLyric, int id, LPARAM lParam);

/// <summary>
/// 获取创建窗口的默认参数
/// </summary>
/// <param name="arg">接收默认参数的结构</param>
void lyric_wnd_get_default_arg(LYRIC_WND_ARG* arg);

/// <summary>
/// 创建一个歌词窗口, 这个窗口是分层窗口, 主要显示歌词用
/// </summary>
/// <param name="arg">创建歌词窗口的参数, 字体, 窗口位置, 颜色等信息</param>
/// <param name="pfnCommand">按钮被点击回调函数</param>
/// <param name="lParam">传递到 pfnCommand() 函数里的参数</param>
/// <returns>返回窗口句柄</returns>
HWND lyric_wnd_create(const LYRIC_WND_ARG* arg, PFN_LYRIC_WND_COMMAND pfnCommand, LPARAM lParam);

/// <summary>
/// 歌词窗口加载krc歌词, krc是酷狗歌词格式, 加载后可以调用更新函数显示
/// 目前暂时只支持krc, 如果能解密网易云/qq引用的专用歌词格式, 后续再增加
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() 返回的窗口句柄</param>
/// <param name="pKrcData">krc文件数据指针</param>
/// <param name="nKrcDataLen">krc文件数据尺寸</param>
/// <returns>返回是否加载成功</returns>
bool lyric_wnd_load_krc(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen);

/// <summary>
/// 更新歌词显示, 这个速度还可以, 百万次调用也就几百毫秒, 200帧都能稳定显示
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="nCurrentTimeMS">要更新的世界, 单位是毫秒</param>
/// <returns>返回是否更新成功</returns>
bool lyric_wnd_update(HWND hWindowLyric, int nCurrentTimeMS);

/// <summary>
/// 给歌词设置歌词文本颜色, 目前只支持普通颜色和高亮颜色
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="isLight">设置的是否是高亮的歌词</param>
/// <param name="pClr">颜色数组, ARGB颜色值</param>
/// <param name="nCount">颜色数组差一点</param>
/// <returns>返回是否设置成功</returns>
bool lyric_wnd_set_color(HWND hWindowLyric, bool isLight, DWORD* pClr, int nCount);

/// <summary>
/// 设置歌词文本字体, 高亮个普通都使用同一个字体, 就是使用不同颜色而已
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="pszName">字体名字</param>
/// <param name="nSize">字体尺寸</param>
/// <param name="isBold">是否加粗</param>
/// <param name="isItalic">是否斜体</param>
/// <returns>返回是否设置成功</returns>
bool lyric_wnd_set_font(HWND hWindowLyric, LPCWSTR pszName, int nSize, bool isBold, bool isItalic);

/// <summary>
/// 设置歌词窗口背景色, ARGB颜色值
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="clr">背景颜色ARGB颜色值</param>
/// <returns>返回是否设置成功</returns>
bool lyric_wnd_set_clr_back(HWND hWindowLyric, DWORD clr);

/// <summary>
/// 设置歌词窗口歌词文本边框颜色, ARGB颜色值
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="clr">ARGB颜色值</param>
/// <returns>返回是否设置成功</returns>
bool lyric_wnd_set_clr_border(HWND hWindowLyric, DWORD clr);

/// <summary>
/// 获取歌词窗口的配置信息, 应该需要把这个保存到某个地方, 等创建的时候传递进来还原
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="arg">参考返回歌词窗口的配置信息, 返回的内容不可修改</param>
/// <returns></returns>
bool lyric_wnd_get_config(HWND hWindowLyric, LYRIC_WND_ARG* arg);

/// <summary>
/// 调用歌词窗口的事件
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">触发的事件ID</param>
/// <returns>返回是否调用成功</returns>
bool lyric_wnd_call_event(HWND hWindowLyric, LYRIC_WND_BUTTON_ID id);

/// <summary>
/// 设置按钮状态
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">按钮ID</param>
/// <param name="state">要设置的状态</param>
/// <returns>返回是否调用成功</returns>
bool lyric_wnd_set_button_state(HWND hWindowLyric, LYRIC_WND_BUTTON_ID id, LYRIC_WND_BUTTON_STATE state);

/// <summary>
/// 获取按钮状态
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">按钮ID</param>
/// <returns>返回按钮状态</returns>
LYRIC_WND_BUTTON_STATE lyric_wnd_get_button_state(HWND hWindowLyric, LYRIC_WND_BUTTON_ID id);


