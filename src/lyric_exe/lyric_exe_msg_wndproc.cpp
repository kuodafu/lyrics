#include "lyric_exe.h"

// 窗口过程函数
LRESULT CALLBACK MessageWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        g_hWnd = hWnd;
        break;
    case WM_USER + 1:
        MessageBoxW(NULL, _T("收到消息 WM_USER+1"), _T("提示"), MB_OK);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}
