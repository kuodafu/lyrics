#include "lyric_exe_header.h"
#include <string>
#include <random>
#include <thread>

#include <read_file.h>
#include "../kuodafu_lyric.h"
#include <control/WndBase.h>
#include "lyric_wnd.h"
#include <WaitObject.h>

#include "bass.h"

#pragma comment(lib, "bass.lib")

// 全局变量:
static HINSTANCE hInst;                                // 当前实例
static HSTREAM m_hStream;      // 音乐播放句柄
static HWND m_hLyricWindow;    // 歌词窗口句柄

// 此代码模块中包含的函数的前向声明:
BOOL                InitInstance(HINSTANCE, int);
bool init_dpi();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    init_dpi();
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

    std::random_device rd;
    std::mt19937 gen(rd());

    do {
        std::wstring filePath = directory + L"\\" + findFileData.cFileName;
        //filePath = LR"(T:\tool盘\KuGou\Lyric\周杰伦 - 半岛铁盒-dd89bdf19d27a9874e774e6faf655206-13604931-00000000.krc)";


        std::string data;
        read_file(filePath.c_str(), data);
        auto p = lyric_parse((LPBYTE)data.c_str(), (int)data.size());
        LYRIC_CALC_STRUCT arg{};

        // 定义随机数分布范围
        std::uniform_int_distribution<> dis(0, 400000);
        int r = dis(gen);
        lyric_calc(p, r, &arg);
        lyric_destroy(p);

    } while (FindNextFileW(hFind, &findFileData) != 0);


    FindClose(hFind);
}

HWND hStatic[3];

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
        BASS_Init(-1, 44100, 0, hWnd, NULL);

        //LPCWSTR file = LR"(I:\音乐\The Tech Thieves - Fake.mp3)";
        //LPCWSTR file = LR"(I:\音乐\音乐\低调组合 - 终点起点.mp3)";
        //LPCWSTR file = LR"(I:\音乐\音乐\低调组合 - 夜空中最亮的星.mp3)";
        LPCWSTR file = LR"(I:\音乐\陈奕迅 - 淘汰.mp3)";
        //LPCWSTR file = LR"(I:\音乐\周杰伦\10-2006-依然范特西\周杰伦 - 本草纲目.mp3)";
        m_hStream = BASS_StreamCreateFile(FALSE, file, 0, 0, BASS_SAMPLE_FLOAT);
        if (m_hStream)
        {
            BASS_ChannelPlay(m_hStream, FALSE);
            BASS_ChannelSetPosition(m_hStream, BASS_ChannelSeconds2Bytes(m_hStream, 40.), BASS_POS_BYTE);
        }
        SetTimer(hWnd, 100, 1000, 0);
        SetTimer(hWnd, 200, 10, 0);

        //std::thread _th([hWnd]()
        //{
        //    while (IsWindow(hWnd))
        //    {
        //        SendMessageW(hWnd, WM_TIMER, 200, 0);
        //        Sleep(10);
        //    }
        //});
        //_th.detach();
        // 枚举  目录下所有 krc文件
        //EnumerateKRCFiles(LR"(T:\tool盘\KuGou\Lyric)");
        //LPCWSTR krc1 = LR"(T:\tool盘\KuGou\Lyric\The Tech Thieves - Fake-cf6d70385ebd673a9f423ed466bd200d-125671670-00000000.krc)";
        //LPCWSTR krc1 = LR"(T:\tool盘\KuGou\Lyric\低调组合 - 终点起点-6257a78a9df6fb551f2324dbc08b4cf7-121741063-00000000.krc)";
        //LPCWSTR krc1 = LR"(T:\移动机械硬盘\E源码备份\易语言备份\1自己写的源码\实用工具\播放器\lrc\低调组合 - 夜空中最亮的星.krc)";
        LPCWSTR krc1 = LR"(T:\tool盘\KuGou\Lyric\陈奕迅 - 淘汰-ea514c1f8eaee9f24dcd1f26575bac4f-135600792-00000000.krc)";
        //LPCWSTR krc1 = LR"(I:\Kugou\Lyric\周杰伦 - 本草纲目-50f657c1d53e3acb1381ef97e5cfabd2-108581021-00000000.krc)";
        std::string data;
        read_file(krc1, data);
        m_hLyricWindow = lyric_wnd_create();
        MoveWindow(m_hLyricWindow, 400, 1100, 1700, 200, true);
        lyric_wnd_load_krc(m_hLyricWindow, data.c_str(), (int)data.size());
        break;
    }
    case WM_TIMER:
    {
        if (wParam == 100)
        {
            wchar_t text[10];
            swprintf_s(text, L"%.2f", BASS_GetCPU());
            SetWindowTextW(hWnd, text);
            break;
        }
        else if (wParam == 200)
        {
            double d_pos = BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetPosition(m_hStream, BASS_POS_BYTE)) * 1000.;
            int pos = (int)d_pos;
            lyric_wnd_update(m_hLyricWindow, pos);
            break;
        }
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
    case WM_CTLCOLOR:
    case WM_CTLCOLORMSGBOX:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSCROLLBAR:
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        HWND hWndChild = (HWND)lParam;
        if (hWndChild == hStatic[1] || hWndChild == hStatic[2])
        {
            if (hWndChild == hStatic[2])
                SetTextColor(hdc, RGB(255, 0, 0));
            SetBkColor(hdc, RGB(255, 255, 255));
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)GetStockObject(LTGRAY_BRUSH);
        }

        return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


bool init_dpi()
{

    typedef HRESULT(WINAPI* pfn_SetProcessDpiAwareness)(int value);
    typedef HRESULT(WINAPI* pfn_SetProcessDpiAwarenessContext)(DPI_AWARENESS_CONTEXT value);
    typedef DPI_AWARENESS_CONTEXT(WINAPI* pfn_SetThreadDpiAwarenessContext)(DPI_AWARENESS_CONTEXT dpiContext);


    HMODULE Shcore = LoadLibraryW(L"Shcore.dll");
    HMODULE hUser32 = LoadLibraryW(L"user32.dll");
    if (!Shcore || !hUser32)
        return false;

    // win10才支持的设置dpi方式
    auto pfnSetProcessDpiAwareness = (pfn_SetProcessDpiAwareness)GetProcAddress(Shcore, "SetProcessDpiAwareness");
    auto pfnSetProcessDpiAwarenessContext = (pfn_SetProcessDpiAwarenessContext)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
    auto pfnSetThreadDpiAwarenessContext = (pfn_SetThreadDpiAwarenessContext)GetProcAddress(hUser32, "SetThreadDpiAwarenessContext");
    if (pfnSetProcessDpiAwareness)
    {
        enum PROCESS_DPI_AWARENESS {
            PROCESS_DPI_UNAWARE = 0,    // DPI 不知道。 此应用不会缩放 DPI 更改，并且始终假定其比例系数为 100% (96 DPI) 。 系统将在任何其他 DPI 设置上自动缩放它
            PROCESS_SYSTEg_dpi_AWARE = 1,    // 统 DPI 感知。 此应用不会缩放 DPI 更改。 它将查询 DPI 一次，并在应用的生存期内使用该值。 如果 DPI 发生更改，应用将不会调整为新的 DPI 值。 当 DPI 与系统值发生更改时，系统会自动纵向扩展或缩减它。
            PROCESS_PER_MONITOR_DPI_AWARE = 2     // 按监视器 DPI 感知。 此应用在创建 DPI 时检查 DPI，并在 DPI 发生更改时调整比例系数。 系统不会自动缩放这些应用程序
        };

        // 感知多个屏幕的dpi
        HRESULT hr = 0;

        if (pfnSetProcessDpiAwarenessContext)
        {
            hr = pfnSetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }
        else
        {
            hr = pfnSetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
            if (pfnSetThreadDpiAwarenessContext)
                pfnSetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        }


        if (FAILED(hr))
        {
            //wstr::dbg(L"开启DPI感知失败, 错误码 = 0x%08X\n", GetLastError());
#ifdef _DEBUG
            __debugbreak();
#endif
            return false;
        }
    }
    else
    {
        // win10以下的设置dpi方式, 不能感知多个屏幕
        BOOL bRet = SetProcessDPIAware();
        //if (!bRet)
        //{
        //    wstr::dbg(L"开启DPI感知失败, 错误码 = 0x%08X\n", GetLastError());
        //    return 0;
        //}
    }

    return true;
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


