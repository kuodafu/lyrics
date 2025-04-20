#include <control/WndControl6_0.h>

#include "lyric_exe_header.h"
#include <string>
#include <random>
#include <thread>

#include <read_file.h>
#include "../kuodafu_lyric.h"
#include <control/WndBase.h>
#include <control/CListView.h>
#include "lyric_wnd.h"
#include <WaitObject.h>

#include "bass.h"

#pragma comment(lib, "bass.lib")

// 全局变量:
static HINSTANCE hInst;                                // 当前实例
static HSTREAM m_hStream;      // 音乐播放句柄
static HWND m_hLyricWindow;    // 歌词窗口句柄
static HWND m_hWnd;            // 主窗口句柄
static CListView m_list;

// 此代码模块中包含的函数的前向声明:
BOOL                InitInstance(HINSTANCE, int);
bool init_dpi();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
int CALLBACK OnLyricCommand(HWND hWindowLyric, int id, LPARAM lParam);

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

void EnumerateMP3Files(const std::wstring& directory)
{
    std::wstring searchPath = directory + L"\\*.*";
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return;
    }

    int i = 1;
    do
    {
        std::wstring filePath = directory + L"\\" + findFileData.cFileName;
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (findFileData.cFileName[0] != L'.')
                EnumerateMP3Files(filePath);
        }
        else if (filePath.ends_with(L"mp3"))
        {
            wchar_t buf[50];
            swprintf_s(buf, L"%d", i++);
            // 有文件, 加载到列表里
            int index = m_list.InsertItem(-1, buf);
            m_list.SetItemText(index, 1, findFileData.cFileName);
            m_list.SetItemText(index, 2, filePath.c_str());

        }
    } while (FindNextFileW(hFind, &findFileData) != 0);


    FindClose(hFind);
}

std::wstring find_krc(LPCWSTR name)
{
    std::wstring directory = LR"(J:\cahce\kugou\Lyric\)";
    std::wstring searchPath = directory + L"*" + name + LR"(*.krc)";
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
        return {};
    
    do
    {
        for(wchar_t& ch : findFileData.cFileName)
            ch = towlower(ch);

        if (wcsstr(findFileData.cFileName, name))
        {
            FindClose(hFind);
            return directory + findFileData.cFileName;
        }

    } while (FindNextFileW(hFind, &findFileData) != 0);


    FindClose(hFind);
    return {};
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


#define ID_LOCK 5000
#define ID_SHOW 5001

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        m_hWnd = hWnd;
        BASS_Init(-1, 44100, 0, hWnd, NULL);
        int left = 100;
        int top = 20;
        auto pfn_create = [&](int id, LPCWSTR name)
        {
            HWND hChild = CreateWindowExW(0, L"BUTTON", name, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                          left, top, 100, 24, hWnd, (HMENU)(LONG_PTR)id, hInst, 0);
            SendMessageW(hChild, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
            top += 28;
            return hChild;
        };

        pfn_create(ID_LOCK, L"锁定歌词");
        pfn_create(ID_SHOW, L"歌词可视");

        m_list.create(hWnd, left + 150, 10, 1000, 800, 0x00000200, 0x56000045, 0x00010423, 0);
        m_list.SetFont(CWndBase::CreateFontW(L"微软雅黑", -18));
        m_list.InsertColumn(0, L"序号", 50);
        m_list.InsertColumn(1, L"歌曲", 500);
        m_list.InsertColumn(2, L"路径", 420);
        EnumerateMP3Files(LR"(I:\音乐\)");

        //LPCWSTR file = LR"(I:\音乐\The Tech Thieves - Fake.mp3)";
        //LPCWSTR file = LR"(I:\音乐\音乐\低调组合 - 终点起点.mp3)";
        //LPCWSTR file = LR"(I:\音乐\音乐\低调组合 - 夜空中最亮的星.mp3)";
        //LPCWSTR file = LR"(I:\音乐\陈奕迅 - 淘汰.mp3)";
        //LPCWSTR file = LR"(I:\音乐\周杰伦\10-2006-依然范特西\周杰伦 - 本草纲目.mp3)";
        LPCWSTR file = LR"(I:\音乐\音乐\王菲 - 但愿人长久.mp3)";
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
        //EnumerateKRCFiles(LR"(J:\cahce\kugou\Lyric)");
        //LPCWSTR krc1 = LR"(J:\cahce\kugou\Lyric\The Tech Thieves - Fake-cf6d70385ebd673a9f423ed466bd200d-125671670-00000000.krc)";
        //LPCWSTR krc1 = LR"(J:\cahce\kugou\Lyric\低调组合 - 终点起点-6257a78a9df6fb551f2324dbc08b4cf7-121741063-00000000.krc)";
        //LPCWSTR krc1 = LR"(T:\移动机械硬盘\E源码备份\易语言备份\1自己写的源码\实用工具\播放器\lrc\低调组合 - 夜空中最亮的星.krc)";
        //LPCWSTR krc1 = LR"(J:\cahce\kugou\Lyric\陈奕迅 - 淘汰-ea514c1f8eaee9f24dcd1f26575bac4f-135600792-00000000.krc)";
        //LPCWSTR krc1 = LR"(J:\cahce\kugou\Lyric\周杰伦 - 本草纲目-50f657c1d53e3acb1381ef97e5cfabd2-108581021-00000000.krc)";
        LPCWSTR krc1 = LR"(J:\cahce\kugou\Lyric\王菲 - 但愿人长久-001e34650e0104930a54d570cb43f994-38649663-00000000.krc)";
        std::string data;
        read_file(krc1, data);
        LYRIC_WND_ARG arg{};
        lyric_wnd_get_default_arg(&arg);
        arg.rcWindow = { 300, 800, 1500, 1000 };
        //arg.pszFontName = L"黑体";
        m_hLyricWindow = lyric_wnd_create(&arg, OnLyricCommand, 0);
        lyric_wnd_load_krc(m_hLyricWindow, data.c_str(), (int)data.size());
        lyric_wnd_call_event(m_hLyricWindow, LYRIC_WND_BUTTON_ID_PLAY);
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
        DestroyWindow(m_hLyricWindow);
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
        return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    case WM_NOTIFY:
    {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->code == NM_DBLCLK)
        {
            LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE)lParam;
            int index = pItem->iItem;
            std::wstring filePath = m_list.GetItemText(index, 2);

            BASS_StreamFree(m_hStream);
            m_hStream = BASS_StreamCreateFile(FALSE, filePath.c_str(), 0, 0, BASS_SAMPLE_FLOAT);
            BASS_ChannelPlay(m_hStream, FALSE);

            filePath.erase(filePath.rfind(L'.'));
            size_t pos = filePath.rfind(L'\\');
            LPWSTR name = &filePath[0] + pos + 1;
            pos = 0;
            while (name[pos])
            {
                name[pos] = towlower(name[pos]);
                pos++;
            }

            std::wstring krc_name = find_krc(name);
            std::string data;
            if (!krc_name.empty())
                read_file(krc_name.c_str(), data);

            lyric_wnd_load_krc(m_hLyricWindow, data.c_str(), (int)data.size());
            lyric_wnd_call_event(m_hLyricWindow, LYRIC_WND_BUTTON_ID_PLAY);
            break;
        }
        break;
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

static bool IsCheck(HWND hWnd)
{
    return SendMessageW(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

static bool SetCheck(HWND hWnd, bool isCheck)
{
    return SendMessageW(hWnd, BM_SETCHECK, isCheck ? BST_CHECKED : BST_UNCHECKED, 0);
}

// “关于”框的消息处理程序。
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND hChild = (HWND)lParam;
    const int id = LOWORD(wParam);
    const int code = HIWORD(wParam);
    const bool isCheck = IsCheck(hChild);
    switch (id)
    {
    case ID_LOCK:
        lyric_wnd_call_event(m_hLyricWindow, isCheck ? LYRIC_WND_BUTTON_ID_LOCK : LYRIC_WND_BUTTON_ID_UNLOCK);
        break;
    case ID_SHOW:
        lyric_wnd_call_event(m_hLyricWindow, isCheck ? LYRIC_WND_BUTTON_ID_SHOW : LYRIC_WND_BUTTON_ID_CLOSE);
        break;
    default:
        return false;
    }
    return true;
}

int CALLBACK OnLyricCommand(HWND hWindowLyric, int id, LPARAM lParam)
{
    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_LOCK:
        SetCheck(GetDlgItem(m_hWnd, ID_LOCK), true);
        break;
    case LYRIC_WND_BUTTON_ID_UNLOCK:
        SetCheck(GetDlgItem(m_hWnd, ID_LOCK), false);
        break;
    case LYRIC_WND_BUTTON_ID_SHOW:
        SetCheck(GetDlgItem(m_hWnd, ID_SHOW), true);
        break;
    case LYRIC_WND_BUTTON_ID_CLOSE:
        SetCheck(GetDlgItem(m_hWnd, ID_SHOW), false);
        break;
    case LYRIC_WND_BUTTON_ID_NEXT:
    case LYRIC_WND_BUTTON_ID_PREV:
    {
        double d_pos = BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetPosition(m_hStream, BASS_POS_BYTE));
        double new_pos = id == LYRIC_WND_BUTTON_ID_NEXT ? 10. : -10.;
        BASS_ChannelSetPosition(m_hStream, BASS_ChannelSeconds2Bytes(m_hStream, d_pos + new_pos), BASS_POS_BYTE);
        break;
    }
    case LYRIC_WND_BUTTON_ID_PLAY:
    {
        BASS_ChannelPlay(m_hStream, FALSE);
        break;
    }
    case LYRIC_WND_BUTTON_ID_PAUSE:
    {
        BASS_ChannelPause(m_hStream);
        break;
    }
    default:
        break;
    }
    return 0;
}
