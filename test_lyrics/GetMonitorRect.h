#ifndef _GETMONITORRECT_H_
#define _GETMONITORRECT_H_
#include <windows.h>


// 获取所有屏幕组成的矩形, 会根据屏幕位置调整坐标, 左边顶边有可能是负数
// 返回屏幕数量, 失败返回 0
inline int GetMonitorRect(RECT* prc)
{
    if (!prc)
        return FALSE;
    memset(prc, 0, sizeof(RECT));
    struct MONITOR_ARG
    {
        RECT* prc;
        int count;
    };
    MONITOR_ARG arg = { prc, 0 };
    // 使用 EnumDisplayMonitors 函数遍历所有显示器
    EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL
    {
        MONITORINFO monitorInfo = { 0 };
        monitorInfo.cbSize = sizeof(MONITORINFO);

        MONITOR_ARG* arg = (MONITOR_ARG*)dwData;

        // 获取显示器信息
        if (GetMonitorInfoW(hMonitor, &monitorInfo))
        {
            RECT* prc = arg->prc;
            arg->count++;
            // 合并矩形
            prc->left   = min(prc->left  , lprcMonitor->left);
            prc->top    = min(prc->top   , lprcMonitor->top);
            prc->right  = max(prc->right , lprcMonitor->right);
            prc->bottom = max(prc->bottom, lprcMonitor->bottom);
        }
        return TRUE; // 继续枚举下一个显示器
    }, (LPARAM)&arg);
    return arg.count;
}

// 获取所有屏幕的矩形, 返回每个屏幕的矩形
// pArr = 接收结果的数组缓冲区, 为0则返回所需的数组成员数
// size = 数组缓冲区大小, 为0则返回所需的数组成员数
// 返回拷贝到数组的成员数
inline int GetMonitorRects(RECT* pArr, int size)
{
    if(!pArr && size <= 0)
        return GetSystemMetrics(SM_CMONITORS);

    struct MONITOR_ARG
    {
        RECT* arr;
        int count;
        int index;
    };

    MONITOR_ARG arg = { pArr, size, 0 };
    // 使用 EnumDisplayMonitors 函数遍历所有显示器
    EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL
    {
        MONITORINFO monitorInfo = { 0 };
        monitorInfo.cbSize = sizeof(MONITORINFO);

        MONITOR_ARG* arg = (MONITOR_ARG*)dwData;

        // 获取显示器信息
        if (GetMonitorInfoW(hMonitor, &monitorInfo))
            arg->arr[arg->index++] = *lprcMonitor;
        
        return arg->index < arg->count;
    }, (LPARAM)&arg);
    return arg.index;
}

// 获取所有屏幕的矩形, 返回每个屏幕的矩形
// pArr = 接收结果的数组缓冲区, 为0则返回所需的数组成员数
// size = 数组缓冲区大小, 为0则返回所需的数组成员数
// 返回拷贝到数组的成员数
inline int GetMonitorRects(MONITORINFO* pArr, int size)
{
    if (!pArr && size <= 0)
        return GetSystemMetrics(SM_CMONITORS);

    struct MONITOR_ARG
    {
        MONITORINFO* arr;
        int count;
        int index;
    };

    MONITOR_ARG arg = { pArr, size, 0 };
    // 使用 EnumDisplayMonitors 函数遍历所有显示器
    EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL
    {
        MONITORINFO monitorInfo = { 0 };
        monitorInfo.cbSize = sizeof(MONITORINFO);

        MONITOR_ARG* arg = (MONITOR_ARG*)dwData;

        // 获取显示器信息
        if (GetMonitorInfoW(hMonitor, &monitorInfo))
            arg->arr[arg->index++] = monitorInfo;

        return arg->index < arg->count;
    }, (LPARAM)&arg);
    return arg.index;
}

// 获取所有屏幕的矩形, 返回每个屏幕的矩形
template<typename _Ty>
inline int GetMonitorRects(_Ty& rc)
{
    int count = GetSystemMetrics(SM_CMONITORS);
    if (count == 0)
    {
        rc.clear();
        return 0;
    }
    rc.resize(count);
    return GetMonitorRects(&rc[0], count);
}

#endif  // _GETMONITORRECT_H_
