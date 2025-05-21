#ifndef _GETMONITORRECT_H_
#define _GETMONITORRECT_H_
#include <windows.h>


// ��ȡ������Ļ��ɵľ���, �������Ļλ�õ�������, ��߶����п����Ǹ���
// ������Ļ����, ʧ�ܷ��� 0
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
    // ʹ�� EnumDisplayMonitors ��������������ʾ��
    EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL
    {
        MONITORINFO monitorInfo = { 0 };
        monitorInfo.cbSize = sizeof(MONITORINFO);

        MONITOR_ARG* arg = (MONITOR_ARG*)dwData;

        // ��ȡ��ʾ����Ϣ
        if (GetMonitorInfoW(hMonitor, &monitorInfo))
        {
            RECT* prc = arg->prc;
            arg->count++;
            // �ϲ�����
            prc->left   = min(prc->left  , lprcMonitor->left);
            prc->top    = min(prc->top   , lprcMonitor->top);
            prc->right  = max(prc->right , lprcMonitor->right);
            prc->bottom = max(prc->bottom, lprcMonitor->bottom);
        }
        return TRUE; // ����ö����һ����ʾ��
    }, (LPARAM)&arg);
    return arg.count;
}

// ��ȡ������Ļ�ľ���, ����ÿ����Ļ�ľ���
// pArr = ���ս�������黺����, Ϊ0�򷵻�����������Ա��
// size = ���黺������С, Ϊ0�򷵻�����������Ա��
// ���ؿ���������ĳ�Ա��
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
    // ʹ�� EnumDisplayMonitors ��������������ʾ��
    EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL
    {
        MONITORINFO monitorInfo = { 0 };
        monitorInfo.cbSize = sizeof(MONITORINFO);

        MONITOR_ARG* arg = (MONITOR_ARG*)dwData;

        // ��ȡ��ʾ����Ϣ
        if (GetMonitorInfoW(hMonitor, &monitorInfo))
            arg->arr[arg->index++] = *lprcMonitor;
        
        return arg->index < arg->count;
    }, (LPARAM)&arg);
    return arg.index;
}

// ��ȡ������Ļ�ľ���, ����ÿ����Ļ�ľ���
// pArr = ���ս�������黺����, Ϊ0�򷵻�����������Ա��
// size = ���黺������С, Ϊ0�򷵻�����������Ա��
// ���ؿ���������ĳ�Ա��
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
    // ʹ�� EnumDisplayMonitors ��������������ʾ��
    EnumDisplayMonitors(NULL, NULL, [](HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) -> BOOL
    {
        MONITORINFO monitorInfo = { 0 };
        monitorInfo.cbSize = sizeof(MONITORINFO);

        MONITOR_ARG* arg = (MONITOR_ARG*)dwData;

        // ��ȡ��ʾ����Ϣ
        if (GetMonitorInfoW(hMonitor, &monitorInfo))
            arg->arr[arg->index++] = monitorInfo;

        return arg->index < arg->count;
    }, (LPARAM)&arg);
    return arg.index;
}

// ��ȡ������Ļ�ľ���, ����ÿ����Ļ�ľ���
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
