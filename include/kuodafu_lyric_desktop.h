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


#define MAPFILE_NAME_LOADLYRIC L"kuodafu\\lyric_desktop\\loadlyric"
#define MAPFILE_NAME_GETCONFIG L"kuodafu\\lyric_desktop\\getconfig"
#define MAPFILE_NAME_SETCONFIG L"kuodafu\\lyric_desktop\\setconfig"

#define LD_FIRST 0x1000

 // 加载歌词消息, 返回是否加载成功
 // wParam: 数据尺寸
 // lParam: 解析类型
 // 备注: 收到此消息会打开 MAPFILE_NAME_LOADLYRIC 文件映射, 然后读取数据, 数据尺寸就是 wParam执行的尺寸
#define LD_LOADLYRIC        (LD_FIRST + 1)

// 更新播放时间消息, 返回是否成功
// wParam: 播放时间, 单位是毫秒
// lParam: 未使用
// 备注: 收到此消息会更新播放时间, 在下一次刷新时会将歌词高亮位置调整到这个备份时间
#define LD_UPDATE           (LD_FIRST + 2)

// 获取配置信息消息, 返回读取到的配置项数据尺寸, 单位是字节
// wParam: 未使用
// lParam: 未使用
// 备注: 收到此消息会打开 MAPFILE_NAME_GETCONFIG 文件映射, 然后读取配置数据
#define LD_GETCONFIG        (LD_FIRST + 3)

// 设置配置信息消息, 返回影响几个配置项
// wParam: 配置数据尺寸
// lParam: 未使用
// 备注: 收到此消息会打开 MAPFILE_NAME_SETCONFIG 文件映射, 然后解析配置数据
#define LD_SETCONFIG        (LD_FIRST + 4)

// 调用按钮事件消息, 返回是否成功
// wParam: 按钮id, id参考 LYRIC_DESKTOP_BUTTON_ID 枚举类型
// lParam: 未使用
// 备注: 收到此消息会调用按钮事件
// 注意: 窗口上显示的是什么按钮, 就使用对应的按钮id, 播放和暂停, 音译/翻译, 选中和没选中都是不同的id
#define LD_CALLEVENT        (LD_FIRST + 5)

// 设置按钮状态消息, 返回是否成功
// wParam: 按钮id, id参考 LYRIC_DESKTOP_BUTTON_ID 枚举类型
// lParam: 状态, 参考 LYRIC_DESKTOP_BUTTON_STATE 枚举类型
// 备注: 收到此消息会设置按钮状态
// 注意: 窗口上显示的是什么按钮, 就使用对应的按钮id, 播放和暂停, 音译/翻译, 选中和没选中都是不同的id
#define LD_SETBUTTONSTATE   (LD_FIRST + 6)

// 获取按钮状态消息, 返回按钮状态
// wParam: 按钮id, id参考 LYRIC_DESKTOP_BUTTON_ID 枚举类型
// lParam: 未使用
// 备注: 收到此消息会获取按钮状态, 返回按钮状态, 参考 LYRIC_DESKTOP_BUTTON_STATE 枚举类型
// 注意: 窗口上显示的是什么按钮, 就使用对应的按钮id, 播放和暂停, 音译/翻译, 选中和没选中都是不同的id
#define LD_GETBUTTONSTATE   (LD_FIRST + 7)



// 歌词窗口按钮ID, 目前就这几个默认按钮
enum LYRIC_DESKTOP_BUTTON_ID
{
    LYRIC_DESKTOP_BUTTON_ID_FIRST           = 1001, // 第一个按钮的索引

    LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY     = 1001, // 音译按钮
    LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY_SEL = 1002, // 音译按钮, 选中模式
    LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY     = 1003, // 翻译按钮
    LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY_SEL = 1004, // 翻译按钮, 选中模式
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

    LYRIC_DESKTOP_BUTTON_ID_MAKELRC_V       = 1024, // 制作歌词, 纵向按钮, 这个没弄对..... 目前也用不上
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
/// 释放歌词窗口返回的指针
/// </summary>
/// <returns></returns>
void LYRICCALL lyric_desktop_free(void* ptr);

/// <summary>
/// 创建一个歌词窗口, 这个窗口是分层窗口, 主要显示歌词用
/// </summary>
/// <param name="arg">创建歌词窗口的参数, 为空则使用默认值, 字体, 窗口位置, 颜色等信息</param>
/// <param name="pfnCommand">按钮被点击回调函数</param>
/// <param name="lParam">传递到 pfnCommand() 函数里的参数</param>
/// <returns>返回窗口句柄</returns>
HWND LYRICCALL lyric_desktop_create(const char* arg, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam);

/// <summary>
/// 歌词窗口加载歌词数据, 歌词是什么类型在 nType 参数里指定
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() 返回的窗口句柄</param>
/// <param name="pData">输入, 执行要解析的歌词数据, 是指向文件还是数据根据nType决定, 如果传递的文本有BOM, 则忽略编码标志位, 使用BOM的编码方式</param>
/// <param name="nSize">输入, pData 的长度, 不管传递什么数据, 单位都是字节</param>
/// <param name="nType">解析类型, 见 LYRIC_PARSE_TYPE 定义</param>
/// <returns>返回是否加载成功</returns>
bool LYRICCALL lyric_desktop_load_lyric(HWND hWindowLyric, LPCVOID pData, int nSize, LYRIC_PARSE_TYPE nType);

/// <summary>
/// 更新播放时间, 歌词显示是根据这个时间来显示的
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="nCurrentTimeMS">要更新的时间, 单位是毫秒</param>
/// <returns>返回是否更新成功</returns>
bool LYRICCALL lyric_desktop_update(HWND hWindowLyric, int nCurrentTimeMS);


/// <summary>
/// 获取歌词窗口的配置信息, 不使用时需要调用 lyric_desktop_free() 释放
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="arg">参考返回歌词窗口的配置信息, 返回的内容不可修改</param>
/// <returns>返回配置json信息</returns>
char* LYRICCALL lyric_desktop_get_config(HWND hWindowLyric);

/// <summary>
/// 设置歌词配置, 设置后会重新创建配置对应的对象
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="argJson">配置的json字符串</param>
/// <returns>返回影响了多少个配置, 失败返回0</returns>
int LYRICCALL lyric_desktop_set_config(HWND hWindowLyric, const char* argJson);

/// <summary>
/// 调用歌词窗口的事件
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">触发的事件ID, 也是按钮id, 触发事件相当于这个按钮被点击</param>
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


