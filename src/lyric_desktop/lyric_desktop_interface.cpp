/*
* 这个文件是桌面歌词导出的接口, 所有接口都写在这里
* 不是接口函数都不在这里写
*/

#include "lyric_wnd_function.h"
#include <CommCtrl.h>

using namespace NAMESPACE_LYRIC_DESKTOP;
using namespace NAMESPACE_D2D;

/// <summary>
/// 初始化桌面歌词, 会初始化D2D, 开启DPI缩放, 注册窗口类等
/// </summary>
/// <returns>返回是否初始化成功</returns>
bool LYRICCALL lyric_desktop_init()
{
    return _ld_init();
}

/// <summary>
/// 取消初始化, 卸载D2D, 释放各种资源
/// </summary>
/// <returns>返回是否初始化成功</returns>
bool LYRICCALL lyric_desktop_uninit()
{
    return _ld_uninit();
}

/// <summary>
/// 获取创建窗口的默认参数
/// </summary>
/// <param name="arg">接收默认参数的结构</param>
void LYRICCALL lyric_desktop_get_default_arg(LYRIC_DESKTOP_ARG* arg)
{
    //static DWORD clrNormal[] =
    //{
    //    MAKEARGB(255, 0,52,138),
    //    MAKEARGB(255, 0,128,192),
    //    MAKEARGB(255, 3,202,252),
    //};

    static DWORD clrNormal[] =
    {
        MAKEARGB(255, 0,109,178),
        MAKEARGB(255, 3,189,241),
        MAKEARGB(255, 3,202,252),
    };

    static DWORD clrLight[] =
    {
        MAKEARGB(255, 255,255,255),
        MAKEARGB(255, 130,247,253),
        MAKEARGB(255, 3, 233, 252),
    };

    arg->clrWndBack = MAKEARGB(100, 0, 0, 0);
    arg->clrWndBorder = MAKEARGB(200, 0, 0, 0);
    arg->nFontSize = 24;
    arg->pszFontName = L"微软雅黑";
    arg->clrBorder = MAKEARGB(255, 33, 33, 33);

    arg->pClrNormal = clrNormal;
    arg->nClrNormal = _countof(clrNormal);

    arg->pClrLight = clrLight;
    arg->nClrLight = _countof(clrLight);

    RECT rc;
    GetWindowRect(GetDesktopWindow(), &rc);
    const int cxScreen = rc.right - rc.left;
    const int cyScreen = rc.bottom - rc.top;

    const int width = 900;
    const int height = 200;
    arg->rcWindow.left = (cxScreen - width) / 2;
    arg->rcWindow.top = (cyScreen - height - 100);
    arg->rcWindow.right = arg->rcWindow.left + width;
    arg->rcWindow.bottom = arg->rcWindow.top + height;

}

/// <summary>
/// 创建一个歌词窗口, 这个窗口是分层窗口, 主要显示歌词用
/// </summary>
/// <param name="arg">创建歌词窗口的参数, 字体, 窗口位置, 颜色等信息</param>
/// <param name="pfnCommand">按钮被点击回调函数</param>
/// <param name="lParam">传递到 pfnCommand() 函数里的参数</param>
/// <returns>返回窗口句柄</returns>
HWND LYRICCALL lyric_desktop_create(const LYRIC_DESKTOP_ARG* arg, PFN_LYRIC_DESKTOP_COMMAND pfnCommand, LPARAM lParam)
{
    PLYRIC_DESKTOP_INFO pWndInfo = _ld_create_layered_window(arg);
    if (!pWndInfo)
        return nullptr;
    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    HWND hWnd = wnd_info.hWnd;

    RECT rcDesk;
    GetWindowRect(GetDesktopWindow(), &rcDesk);
    const int cxScreen = rcDesk.right - rcDesk.left;
    const int cyScreen = rcDesk.bottom - rcDesk.top;

    wnd_info.hWnd = hWnd;
    wnd_info.pfnCommand = pfnCommand;
    wnd_info.lParam = lParam;
    wnd_info.line1.align = 0;
    wnd_info.line2.align = 2;
    wnd_info.mode = LYRIC_MODE::DOUBLE_ROW;

    // 位置相关的放一起处理
    wnd_info.dpi_change(hWnd);
    wnd_info.get_monitor();

    RECT rc = arg->rcWindow;
    wnd_info.scale(rc);

    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;

    if (rc.top > cyScreen - wnd_info.scale(250))
        rc.top = cyScreen - wnd_info.scale(250), rc.bottom = rc.top + height;
    if (rc.left < rcDesk.left)
        rc.left = (cxScreen - width) / 2, rc.right = rc.left + width;

    MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);




    wnd_info.set_def_arg(arg);
    lyric_wnd_default_object(wnd_info);

    //TODO 这里创建高精度定时器, 根据刷新率来调整刷新间隔


    _ld_start_high_precision_timer(pWndInfo);

    if (wnd_info.nMinHeight)
        MoveWindow(hWnd, rc.left, rc.top, rc.right - rc.left, wnd_info.nMinHeight, TRUE);

    return hWnd;
}

/// <summary>
/// 歌词窗口加载歌词数据, 歌词是什么类型在 nType 参数里指定
/// </summary>
/// <param name="hWindowLyric">lyric_wnd_create() 返回的窗口句柄</param>
/// <param name="pKrcData">krc文件数据指针</param>
/// <param name="nKrcDataLen">krc文件数据尺寸</param>
/// <param name="nType">解析类型, 见 LYRIC_PARSE_TYPE 定义</param>
/// <returns>返回是否加载成功</returns>
bool LYRICCALL lyric_desktop_load_lyric(HWND hWindowLyric, LPCVOID pKrcData, int nKrcDataLen, LYRIC_PARSE_TYPE nType)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;
    CCriticalSection cs(pWndInfo->pCritSec);    // 上锁, 防止这里不是窗口线程调用, 窗口线程会不停的访问歌词结构

    lyric_destroy(pWndInfo->hLyric);
    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    wnd_info.hLyric = lyric_parse(pKrcData, nKrcDataLen, nType);
    pWndInfo->nTimeOffset = lyric_behind_ahead(wnd_info.hLyric, 0);
    pWndInfo->line1.clear();
    pWndInfo->line2.clear();

    int language = lyric_get_language(wnd_info.hLyric);
    lyric_wnd_set_state_translate(*pWndInfo, language);

    if (language)
        wnd_info.add_mode(LYRIC_MODE::EXISTTRANS);
    else
        wnd_info.del_mode(LYRIC_MODE::EXISTTRANS);

    if (!wnd_info.hLyric)
        return false;
    auto& d2dInfo = d2d_get_info();

    // 必须得用小数记, 不然歌词字多了会相差出很多个像素
    lyric_calc_text(wnd_info.hLyric, [](void* pUserData, LPCWSTR pText, int nTextLen, float* pRetHeight) -> float
                          {
                              LYRIC_DESKTOP_INFO* pWndInfo = (LYRIC_DESKTOP_INFO*)pUserData;
                              CD2DRender& hCanvas = *pWndInfo->dx.hCanvas;
                              if (!pWndInfo->dx.hFont)
                                  lyric_wnd_default_object(*pWndInfo);

                              CComPtr<IDWriteTextLayout> pTextLayout;
                              lyric_wnd_create_text_layout(pText, nTextLen, *pWndInfo->dx.hFont, 0, 0, &pTextLayout);
                              float width = _lyric_wnd_load_krc_calc_text(pWndInfo, pTextLayout, pRetHeight);
                              return width;
                          }, pWndInfo);

    return true;
}

/// <summary>
/// 更新播放时间, 歌词显示是根据这个时间来显示的
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="nCurrentTimeMS">要更新的时间, 单位是毫秒</param>
/// <returns>返回是否更新成功</returns>
bool LYRICCALL lyric_desktop_update(HWND hWindowLyric, int nCurrentTimeMS)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    LYRIC_DESKTOP_INFO& wnd_info = *pWndInfo;
    wnd_info.nCurrentTimeMS = nCurrentTimeMS;
    //InvalidateRect(hWindowLyric, 0, 0);   // 使用这个方式会卡, 不知道啥情况, 直接调用重画吧
    //lyric_wnd_invalidate(wnd_info);
    return true;
}

/// <summary>
/// 给歌词设置歌词文本颜色, 目前只支持普通颜色和高亮颜色
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="isLight">设置的是否是高亮的歌词</param>
/// <param name="pClr">颜色数组, ARGB颜色值</param>
/// <param name="nCount">颜色数组差一点</param>
/// <returns>返回是否设置成功</returns>
bool LYRICCALL lyric_desktop_set_color(HWND hWindowLyric, bool isLight, DWORD* pClr, int nCount)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    if (isLight)
    {
        pWndInfo->clrLight.assign(&pClr[0], &pClr[nCount]);
    }
    else
    {
        pWndInfo->clrNormal.assign(&pClr[0], &pClr[nCount]);
    }

    return pWndInfo->dx.re_create_brush(pWndInfo, isLight);
}

/// <summary>
/// 设置歌词文本字体, 高亮个普通都使用同一个字体, 就是使用不同颜色而已
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="pszName">字体名字</param>
/// <param name="nSize">字体尺寸</param>
/// <param name="isBold">是否加粗</param>
/// <param name="isItalic">是否斜体</param>
/// <returns>返回是否设置成功</returns>
bool LYRICCALL lyric_desktop_set_font(HWND hWindowLyric, LPCWSTR pszName, int nSize, bool isBold, bool isItalic)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    if (!pszName || !*pszName)
        pszName = L"微软雅黑";
    if (nSize == 0)
        nSize = 24;

    LOGFONTW& lf = pWndInfo->lf;
    wcscpy_s(lf.lfFaceName, pszName);
    lf.lfHeight = nSize;
    lf.lfWeight = isBold ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = isItalic ? TRUE : FALSE;

    return pWndInfo->dx.re_create_font(pWndInfo);
}

/// <summary>
/// 设置歌词窗口背景色, ARGB颜色值
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="clr">背景颜色ARGB颜色值</param>
/// <returns>返回是否设置成功</returns>
bool LYRICCALL lyric_desktop_set_clr_back(HWND hWindowLyric, DWORD clr)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    pWndInfo->dx.clrBack = clr;
    return true;
}

/// <summary>
/// 设置歌词窗口歌词文本边框颜色, ARGB颜色值
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="clr">ARGB颜色值</param>
/// <returns>返回是否设置成功</returns>
bool LYRICCALL lyric_desktop_set_clr_border(HWND hWindowLyric, DWORD clr)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    pWndInfo->clrBorder = clr;
    return true;
}

/// <summary>
/// 获取歌词窗口的配置信息, 应该需要把这个保存到某个地方, 等创建的时候传递进来还原
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="arg">参考返回歌词窗口的配置信息, 返回的内容不可修改</param>
/// <returns></returns>
bool LYRICCALL lyric_desktop_get_config(HWND hWindowLyric, LYRIC_DESKTOP_ARG* arg)
{
    if (!arg || !hWindowLyric)
        return false;

    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return false;

    GetWindowRect(hWindowLyric, &arg->rcWindow);
    pWndInfo->scale.unscale(arg->rcWindow); // 保存到配置里的是要保存未缩放的坐标

    arg->clrWndBack = pWndInfo->dx.clrBack;
    arg->nFontSize = pWndInfo->lf.lfHeight;
    arg->pszFontName = pWndInfo->lf.lfFaceName;
    arg->clrBorder = pWndInfo->clrBorder;
    arg->nClrNormal = (int)pWndInfo->clrLight.size();
    arg->pClrNormal = arg->nClrNormal > 0 ? &pWndInfo->clrLight[0] : nullptr;
    arg->nClrLight = (int)pWndInfo->clrNormal.size();
    arg->pClrLight = arg->nClrLight > 0 ? &pWndInfo->clrNormal[0] : nullptr;

    return true;
}

/// <summary>
/// 调用歌词窗口的事件
/// </summary>
/// <param name="hWindowLyric">歌词窗口句柄</param>
/// <param name="id">触发的事件ID</param>
/// <returns>返回是否调用成功</returns>
bool LYRICCALL lyric_desktop_call_event(HWND hWindowLyric, LYRIC_DESKTOP_BUTTON_ID id)
{
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
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
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
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
    PLYRIC_DESKTOP_INFO pWndInfo = lyric_wnd_get_data(hWindowLyric);
    if (!pWndInfo)
        return LYRIC_DESKTOP_BUTTON_STATE_ERROR;
    return lyric_desktop::lyric_wnd_get_btn_state(*pWndInfo, id);
}

