/*
* 这个文件是桌面歌词导出的接口, 所有接口都写在这里
* 不是接口函数都不在这里写
*/

#include "lyric_desktop_function.h"
#include <CommCtrl.h>
#include <atlbase.h>

using namespace NAMESPACE_LYRIC_DESKTOP;
using namespace KUODAFU_NAMESPACE;

/// <summary>
/// 初始化桌面歌词, 会初始化D2D, 开启DPI缩放, 注册窗口类等
/// </summary>
/// <returns>返回是否初始化成功</returns>
bool LYRICCALL lyric_desktop_init()
{
    return _lyric_dwsktop_init();
}

/// <summary>
/// 取消初始化, 卸载D2D, 释放各种资源
/// </summary>
/// <returns>返回是否初始化成功</returns>
bool LYRICCALL lyric_desktop_uninit()
{
    return _lyric_dwsktop_uninit();
}

void LYRICCALL lyric_desktop_free(void* ptr)
{
    ::free(ptr);
}


/// <summary>
/// 创建一个歌词窗口, 这个窗口是分层窗口, 主要显示歌词用
/// </summary>
/// <param name="arg">创建歌词窗口的参数, 字体, 窗口位置, 颜色等信息</param>
/// <param name="pfnCommand">按钮被点击回调函数</param>
/// <param name="lParam">传递到 pfnCommand() 函数里的参数</param>
/// <returns>返回窗口句柄</returns>
HWND LYRICCALL lyric_desktop_create(const char* arg, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _ld_create_layered_window(arg, pfnCommand, lParam);
    if (!pWndInfo)
        return nullptr;

    return pWndInfo->hWnd;
}

/// <summary>
/// 歌词窗口加载歌词数据, 歌词是什么类型在 nType 参数里指定
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() 返回的窗口句柄</param>
/// <param name="pData">输入, 执行要解析的歌词数据, 是指向文件还是数据根据nType决定, 如果传递的文本有BOM, 则忽略编码标志位, 使用BOM的编码方式</param>
/// <param name="nSize">输入, pData 的长度, 不管传递什么数据, 单位都是字节</param>
/// <param name="nType">解析类型, 见 LYRIC_PARSE_TYPE 定义</param>
/// <returns>返回是否加载成功</returns>
bool LYRICCALL lyric_desktop_load_lyric(HWND hWindowLyric, LPCVOID pData, int nSize, LYRIC_PARSE_TYPE nType)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _lyric_desktop_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    return _lyric_desktop_load_lyric(pWndInfo, pData, nSize, nType);
}

/// <summary>
/// 更新播放时间, 歌词显示是根据这个时间来显示的
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="nCurrentTimeMS">要更新的时间, 单位是毫秒</param>
/// <returns>返回是否更新成功</returns>
bool LYRICCALL lyric_desktop_update(HWND hWindowLyric, int nCurrentTimeMS)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _lyric_desktop_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    wnd_info.nCurrentTimeMS = nCurrentTimeMS;
    //InvalidateRect(hWindowLyric, 0, 0);   // 使用这个方式会卡, 不知道啥情况, 直接调用重画吧
    //lyric_wnd_invalidate(wnd_info);
    pWndInfo->update();
    return true;
}

/// <summary>
/// 获取歌词窗口的配置信息, 不使用时需要调用 lyric_desktop_free() 释放
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="arg">参考返回歌词窗口的配置信息, 返回的内容不可修改</param>
/// <returns>返回配置json信息</returns>
char* LYRICCALL lyric_desktop_get_config(HWND hWindowLyric)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _lyric_desktop_get_data(hWindowLyric);
    if (!pWndInfo)
        return nullptr;
    return pWndInfo->config.to_json(pWndInfo);
}

/// <summary>
/// 设置歌词配置, 设置后会重新创建配置对应的对象
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="argJson">配置的json字符串</param>
/// <returns>返回影响了多少个配置, 失败返回0</returns>
int LYRICCALL lyric_desktop_set_config(HWND hWindowLyric, const char* argJson)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _lyric_desktop_get_data(hWindowLyric);
    if (!pWndInfo)
        return 0;
    return pWndInfo->config.parse(argJson, pWndInfo);
}

/// <summary>
/// 调用歌词窗口的事件
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">触发的事件ID</param>
/// <returns>返回是否调用成功</returns>
bool LYRICCALL lyric_desktop_call_event(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _lyric_desktop_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    return lyric_desktop::lyric_wnd_call_evt(*pWndInfo, id);
}

/// <summary>
/// 设置按钮状态
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">按钮ID</param>
/// <param name="state">要设置的状态</param>
/// <returns>返回是否调用成功</returns>
bool LYRICCALL lyric_desktop_set_button_state(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id, LYRIC_DESKTOP_BUTTON_STATE state)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _lyric_desktop_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    return lyric_desktop::lyric_wnd_set_btn_state(*pWndInfo, id, state);
}

/// <summary>
/// 获取按钮状态
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">按钮ID</param>
/// <returns>返回按钮状态</returns>
LYRIC_DESKTOP_BUTTON_STATE LYRICCALL lyric_desktop_get_button_state(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _lyric_desktop_get_data(hWindowLyric);
    if (!pWndInfo)
        return LYRIC_DESKTOP_BUTTON_STATE_ERROR;
    return lyric_desktop::lyric_wnd_get_btn_state(*pWndInfo, id);
}

