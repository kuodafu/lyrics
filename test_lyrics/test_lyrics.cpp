#include "test_lyrics.h"
#include <string>

#include <read_file.h>
#include "../kuodafu_lyric.h"

// 全局变量:
HINSTANCE hInst;                                // 当前实例

// 此代码模块中包含的函数的前向声明:
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}




//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 将实例句柄存储在全局变量中
    WNDCLASSEXW wcex;
    LPCWSTR szWindowClass = L"歌词测试";
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = wcex.hIcon;

    RegisterClassExW(&wcex);
    HWND hWnd = CreateWindowW(wcex.lpszClassName, L"测试调用歌词", WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void EnumerateKRCFiles(const std::wstring& directory) {
    std::wstring searchPath = directory + L"\\*.krc";
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        std::wstring filePath = directory + L"\\" + findFileData.cFileName;
        //filePath = LR"(T:\tool盘\KuGou\Lyric\周杰伦 - 半岛铁盒-dd89bdf19d27a9874e774e6faf655206-13604931-00000000.krc)";

        std::string data;
        read_file(filePath.c_str(), data);
        auto p = lyric_parse((LPBYTE)data.c_str(), (int)data.size());
        lyric_free(p);

    } while (FindNextFileW(hFind, &findFileData) != 0);


    FindClose(hFind);
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        // 枚举  目录下所有 krc文件
        
        EnumerateKRCFiles(LR"(T:\tool盘\KuGou\Lyric)");

        //std::string data;
        //read_file(LR"(T:\tool盘\KuGou\Lyric\Sarah Connor、Natural - Just One Last Dance-2c835702040e3e914030829298405d36-136422962-00000000.krc)", data);
        //lyric_parse((LPBYTE)data.c_str(), (int)data.size());
        break;
    }
    case WM_COMMAND:
    {
        if (OnCommand(hWnd, wParam, lParam))
            return 0;
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: 在此处添加使用 hdc 的任何绘图代码...
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    const int id = LOWORD(wParam);
    const int code = HIWORD(wParam);
    switch (id)
    {
    case 0:
        break;
    default:
        return false;
    }
    return true;
}
