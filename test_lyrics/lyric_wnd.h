#pragma once
#include <windows.h>

// 歌词窗口按钮ID, 目前就这几个默认按钮
enum LYRIC_WND_BUTTON_ID
{
    LYRIC_WND_BUTTON_ID_MENU        = 1000, // 菜单按钮
    LYRIC_WND_BUTTON_ID_PREV        = 1001, // 上一首
    LYRIC_WND_BUTTON_ID_NEXT        = 1002, // 下一首
    LYRIC_WND_BUTTON_ID_PLAY        = 1003, // 播放, 回调函数返回0后会变成暂停按钮
    LYRIC_WND_BUTTON_ID_PAUSE       = 1004, // 暂停, 回调函数返回0后会变成播放按钮
    LYRIC_WND_BUTTON_ID_FONT_UP     = 1005, // 字体增加
    LYRIC_WND_BUTTON_ID_FONT_DOWN   = 1006, // 字体减小
    LYRIC_WND_BUTTON_ID_SETTING     = 1007, // 设置按钮
    LYRIC_WND_BUTTON_ID_AHEAD       = 1008, // 歌词提前
    LYRIC_WND_BUTTON_ID_BEHIND      = 1009, // 歌词延后
    LYRIC_WND_BUTTON_ID_SEARCH      = 1010, // 歌词不对, 一般点击后是搜索歌词
    LYRIC_WND_BUTTON_ID_TRANSLATE1  = 1011, // 翻译按钮
    LYRIC_WND_BUTTON_ID_TRANSLATE2  = 1012, // 音译按钮
    LYRIC_WND_BUTTON_ID_LOCK        = 1013, // 锁定按钮
    LYRIC_WND_BUTTON_ID_CLOSE       = 1014, // 关闭按钮

};

typedef struct LYRIC_WND_ARG
{
    RECT        rcWindow;       // 要设置的窗口位置和大小
    COLORREF    clrBackground;  // 背景颜色
    int         nFontSize;      // 字体大小
    LPCWSTR     pwszFontName;   // 字体名称

}*PLYRIC_WND_ARG;

// 歌词窗口按钮被点击事件, 返回0表示事件放行, 返回非0表示拦截事件
typedef int (CALLBACK* PFN_LYRIC_WND_COMMAND)(HWND hWindowLyric, int id, LPARAM lParam);

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


