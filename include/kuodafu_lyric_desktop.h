/*
 * Copyright (c) 2025 扩大福 (121007124@qq.com)
 *
 * Author      : 扩大福
 * Website     : https://www.kuodafu.com
 * Project     : https://github.com/kuodafu/lyrics
 * QQ Group    : 121007124, 20752843
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


 /*
 * 本文件是桌面歌词导出的函数接口.
 */

#pragma once
#include <windows.h>
#include "kuodafu_lyric.h"


// 歌词窗口按钮ID, 目前就这几个默认按钮
enum LYRIC_DESKTOP_BUTTON_ID
{
    LYRIC_DESKTOP_BUTTON_ID_FIRST           = 1001, // 第一个按钮的索引

    LYRIC_DESKTOP_BUTTON_ID_TRANSLATE2      = 1001, // 音译按钮
    LYRIC_DESKTOP_BUTTON_ID_TRANSLATE2_SEL  = 1002, // 音译按钮, 选中模式
    LYRIC_DESKTOP_BUTTON_ID_TRANSLATE1      = 1003, // 翻译按钮
    LYRIC_DESKTOP_BUTTON_ID_TRANSLATE1_SEL  = 1004, // 翻译按钮, 选中模式
    LYRIC_DESKTOP_BUTTON_ID_LRCWRONG        = 1005, // 歌词不对
    LYRIC_DESKTOP_BUTTON_ID_VERTICAL        = 1006, // 竖屏按钮
    LYRIC_DESKTOP_BUTTON_ID_MAKELRC         = 1007, // 制作歌词

    LYRIC_DESKTOP_BUTTON_ID_FONT_DOWN       = 1008, // 字体减小
    LYRIC_DESKTOP_BUTTON_ID_FONT_UP         = 1009, // 字体增加
    LYRIC_DESKTOP_BUTTON_ID_BEHIND          = 1010, // 歌词延后
    LYRIC_DESKTOP_BUTTON_ID_AHEAD           = 1011, // 歌词提前
    LYRIC_DESKTOP_BUTTON_ID_LOCK            = 1012, // 锁定按钮
    LYRIC_DESKTOP_BUTTON_ID_SETTING         = 1013, // 设置按钮
    LYRIC_DESKTOP_BUTTON_ID_UNLOCK          = 1014, // 解锁按钮
    LYRIC_DESKTOP_BUTTON_ID_CLOSE           = 1015, // 关闭按钮
    LYRIC_DESKTOP_BUTTON_ID_LRCCOLOR        = 1016, // 设置字体颜色, 田字的按钮图标
    LYRIC_DESKTOP_BUTTON_ID_MENU            = 1017, // 菜单按钮

    LYRIC_DESKTOP_BUTTON_ID_LRCWRONG_V      = 1018, // 歌词不对, 纵向的按钮图标
    LYRIC_DESKTOP_BUTTON_ID_HORIZONTAL      = 1019, // 横屏按钮
    LYRIC_DESKTOP_BUTTON_ID_PLAY            = 1020, // 播放, 回调函数返回0后会变成暂停按钮
    LYRIC_DESKTOP_BUTTON_ID_PAUSE           = 1021, // 暂停, 回调函数返回0后会变成播放按钮
    LYRIC_DESKTOP_BUTTON_ID_PREV            = 1022, // 上一首
    LYRIC_DESKTOP_BUTTON_ID_NEXT            = 1023, // 下一首


    LYRIC_DESKTOP_BUTTON_ID_COUNT = LYRIC_DESKTOP_BUTTON_ID_NEXT - LYRIC_DESKTOP_BUTTON_ID_FIRST + 1,  // 按钮数量

    LYRIC_DESKTOP_BUTTON_ID_MAKELRCV        = 1024, // 制作歌词, 纵向按钮, 这个没弄对..... 目前也用不上
    LYRIC_DESKTOP_BUTTON_ID_SHOW            = 1025, // 显示歌词, 没这个按钮, 但是有这个事件, 可以让外部调用, 内部没这个按钮, 内部不会主动触发



};

enum LYRIC_DESKTOP_BUTTON_STATE
{
    LYRIC_DESKTOP_BUTTON_STATE_NORMAL   = 0,    // 正常状态
    LYRIC_DESKTOP_BUTTON_STATE_HOVER    = 1,    // 鼠标悬停
    LYRIC_DESKTOP_BUTTON_STATE_PUSHED   = 2,    // 鼠标按下
    LYRIC_DESKTOP_BUTTON_STATE_DISABLE  = 4,    // 禁用状态

    LYRIC_DESKTOP_BUTTON_STATE_ERROR    = -1,   // 获取失败
};

typedef struct LYRIC_DESKTOP_ARG
{
    RECT        rcWindow;       // 要设置的窗口位置和大小
    DWORD       clrWndBack;     // 背景颜色
    DWORD       clrWndBorder;   // 窗口背景边框颜色, 为0则不显示边框
    int         nFontSize;      // 字体大小
    LPCWSTR     pszFontName;    // 字体名称
    DWORD       clrBorder;      // 边框颜色

    DWORD*      pClrNormal;     // 普通歌词颜色数组
    int         nClrNormal;     // 普通歌词颜色数组长度

    DWORD*      pClrLight;      // 高亮歌词颜色数组
    int         nClrLight;      // 高亮歌词颜色数组长度

}*PLYRIC_DESKTOP_ARG;

// 歌词窗口按钮被点击事件, 返回0表示事件放行, 返回非0表示拦截事件
typedef int (CALLBACK* PFN_LYRIC_DESKTOP_COMMAND)(HWND hWindowLyric, int id, LPARAM lParam);

/// <summary>
/// 初始化桌面歌词, 会初始化D2D, 开启DPI缩放, 注册窗口类等
/// </summary>
/// <returns>返回是否初始化成功</returns>
bool LYRICCALL lyric_desktop_init();

/// <summary>
/// 取消初始化, 卸载D2D, 释放各种资源
/// </summary>
/// <returns>返回是否初始化成功</returns>
bool LYRICCALL lyric_desktop_uninit();

/// <summary>
/// 获取创建窗口的默认参数
/// </summary>
/// <param name="arg">接收默认参数的结构</param>
void LYRICCALL lyric_desktop_get_default_arg(LYRIC_DESKTOP_ARG* arg);

/// <summary>
/// 创建一个歌词窗口, 这个窗口是分层窗口, 主要显示歌词用
/// </summary>
/// <param name="arg">创建歌词窗口的参数, 字体, 窗口位置, 颜色等信息</param>
/// <param name="pfnCommand">按钮被点击回调函数</param>
/// <param name="lParam">传递到 pfnCommand() 函数里的参数</param>
/// <returns>返回窗口句柄</returns>
HWND LYRICCALL lyric_desktop_create(const LYRIC_DESKTOP_ARG* arg, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam);

/// <summary>
/// 歌词窗口加载歌词数据, 歌词是什么类型在 nType 参数里指定
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() 返回的窗口句柄</param>
/// <param name="pKrcData">krc文件数据指针</param>
/// <param name="nKrcDataLen">krc文件数据尺寸</param>
/// <param name="nType">解析类型, 见 LYRIC_PARSE_TYPE 定义</param>
/// <returns>返回是否加载成功</returns>
bool LYRICCALL lyric_desktop_load_lyric(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen, LYRIC_PARSE_TYPE nType);

/// <summary>
/// 更新播放时间, 歌词显示是根据这个时间来显示的
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="nCurrentTimeMS">要更新的时间, 单位是毫秒</param>
/// <returns>返回是否更新成功</returns>
bool LYRICCALL lyric_desktop_update(HWND hWindowLyric, int nCurrentTimeMS);

/// <summary>
/// 给歌词设置歌词文本颜色, 目前只支持普通颜色和高亮颜色
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="isLight">设置的是否是高亮的歌词</param>
/// <param name="pClr">颜色数组, ARGB颜色值</param>
/// <param name="nCount">颜色数组差一点</param>
/// <returns>返回是否设置成功</returns>
bool LYRICCALL lyric_desktop_set_color(HWND hWindowLyric, bool isLight, DWORD* pClr, int nCount);

/// <summary>
/// 设置歌词文本字体, 高亮个普通都使用同一个字体, 就是使用不同颜色而已
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="pszName">字体名字</param>
/// <param name="nSize">字体尺寸</param>
/// <param name="isBold">是否加粗</param>
/// <param name="isItalic">是否斜体</param>
/// <returns>返回是否设置成功</returns>
bool LYRICCALL lyric_desktop_set_font(HWND hWindowLyric, LPCWSTR pszName, int nSize, bool isBold, bool isItalic);

/// <summary>
/// 设置歌词窗口背景色, ARGB颜色值
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="clr">背景颜色ARGB颜色值</param>
/// <returns>返回是否设置成功</returns>
bool LYRICCALL lyric_desktop_set_clr_back(HWND hWindowLyric, DWORD clr);

/// <summary>
/// 设置歌词窗口歌词文本边框颜色, ARGB颜色值
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="clr">ARGB颜色值</param>
/// <returns>返回是否设置成功</returns>
bool LYRICCALL lyric_desktop_set_clr_border(HWND hWindowLyric, DWORD clr);

/// <summary>
/// 获取歌词窗口的配置信息, 应该需要把这个保存到某个地方, 等创建的时候传递进来还原
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="arg">参考返回歌词窗口的配置信息, 返回的内容不可修改</param>
/// <returns></returns>
bool LYRICCALL lyric_desktop_get_config(HWND hWindowLyric, LYRIC_DESKTOP_ARG* arg);

/// <summary>
/// 调用歌词窗口的事件
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">触发的事件ID</param>
/// <returns>返回是否调用成功</returns>
bool LYRICCALL lyric_desktop_call_event(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id);

/// <summary>
/// 设置按钮状态
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">按钮ID</param>
/// <param name="state">要设置的状态</param>
/// <returns>返回是否调用成功</returns>
bool LYRICCALL lyric_desktop_set_button_state(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id, LYRIC_DESKTOP_BUTTON_STATE state);

/// <summary>
/// 获取按钮状态
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">按钮ID</param>
/// <returns>返回按钮状态</returns>
LYRIC_DESKTOP_BUTTON_STATE LYRICCALL lyric_desktop_get_button_state(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id);


