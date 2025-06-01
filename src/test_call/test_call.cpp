#include <control/WndControl6_0.h>

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
// C 运行时头文件
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <random>
#include <thread>
#include <mutex>

#include <read_file.h>
#include <kuodafu_lyric.h>
#include <control/WndBase.h>
#include <control/CListView.h>
#include <kuodafu_lyric_wnd.h>
#include <WaitObject.h>
#include "../src/charset_stl.h"
#include <cJSON/cJSON.h>
#include "bass.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Bcrypt.lib")

#ifdef _WIN64
#   pragma comment(lib, "lib/x64/bass.lib")
#else
#   pragma comment(lib, "lib/x86/bass.lib")
#endif

// 是否使用静态库
#define USING_LIB 0

#if USING_LIB
#   ifdef _WIN64
#      ifdef _DEBUG
#          pragma comment(lib, "output/x64/lyric_desktop_libD.lib")
#      else
#          pragma comment(lib, "output/x64/lyric_desktop_lib.lib")
#      endif
#   else
#      ifdef _DEBUG
#          pragma comment(lib, "output/x86/lyric_desktop_libD.lib")
#      else
#          pragma comment(lib, "output/x86/lyric_desktop_lib.lib")
#      endif
#   endif
#else
#   ifdef _WIN64
#      ifdef _DEBUG
#          pragma comment(lib, "output/x64/lyric_desktopD.lib")
#      else
#          pragma comment(lib, "output/x64/lyric_desktop.lib")
#      endif
#   else
#      ifdef _DEBUG
#          pragma comment(lib, "output/x86/lyric_desktopD.lib")
#      else
#          pragma comment(lib, "output/x86/lyric_desktop.lib")
#      endif
#   endif
#endif


// 全局变量:
static HINSTANCE hInst;                                // 当前实例
static HSTREAM m_hStream;      // 音乐播放句柄
static HWND m_hLyricWindow;    // 歌词窗口句柄
static HWND m_hWnd;            // 主窗口句柄
static CListView m_list;

// 此代码模块中包含的函数的前向声明:
BOOL                InitInstance(HINSTANCE, int);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
int CALLBACK OnLyricCommand(HWND hWindowLyric, int id, LPARAM lParam);

#include <assist/assist.h>


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    void* out = nullptr;
    size_t out_size;
    charset_stl::__charsel_impl::__CHARSET_CONVERT cptr;

    auto pfn_strlen_a = [](const std::string& x)
    {
        return x.size();
    };

    //auto pOutput = cptr.copy_gbk("sda", 3, out_size);
    //pOutput = cptr.gbk_to("sda", 3, charset_stl::AnsiToUtf8, out_size);
    //pOutput = cptr.utf8_to("sda", 3, charset_stl::AnsiToUtf8, out_size);
    //auto xx1 = cptr.utf16le_to(L"sda", 6, charset_stl::UnicodeToAnsi, out_size);


    std::string xx232;
    //charset_stl::ptr2gbk(L"123456", 12, CHARSET_TYPE_UTF16_LE, xx232);

    //charset_stl::__charsel_impl::__charset_convert_str_ptr("123我是", 7, CHARSET_TYPE_GBK, CHARSET_TYPE_UTF16_BE, &out, out_size);
    //charset_stl::__charsel_impl::__charset_convert_str(u8"123我是", -1, CHARSET_TYPE_UTF8, out, out_size, CHARSET_TYPE_GBK);
    //charset_stl::__charsel_impl::__charset_convert_str(L"123我是", -1, CHARSET_TYPE_UTF16_LE, out, out_size, CHARSET_TYPE_UTF16_LE);
    LPCWSTR xxasd = (LPCWSTR)out;
    auto xx = GetClipboard();


    auto xxx = charset_stl::UnicodeToUtf8(xx.c_str());
    auto xxx2 = charset_stl::Utf8ToUnicode(xxx);



    lyric_wnd_init();

    // 执行应用程序初始化:
    if (InitInstance(hInstance, nCmdShow))
    {
        MSG msg;

        // 主消息循环:
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    BASS_Free();
    lyric_wnd_uninit();
    return 0;
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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = wcex.hIcon;

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
        for (wchar_t& ch : findFileData.cFileName)
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
        auto p = lyric_parse((LPBYTE)data.c_str(), (int)data.size(), LYRIC_PARSE_TYPE_KRCDATA);
        LYRIC_CALC_STRUCT arg{};

        int a = lyric_get_language(p);
        if (a == 3)
        {
            OutputDebugStringW(L"这个歌词有双翻译: ");
            OutputDebugStringW(filePath.c_str());
            OutputDebugStringW(L"\n");
        }

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

        pfn_create(IDOK, L"测试内存泄漏");

        m_list.create(hWnd, left + 150, 10, 1000, 800, 0x00000200, 0x56000045, 0x00010423, 0);
        m_list.SetFont(CWndBase::CreateFontW(L"微软雅黑", -18));
        m_list.InsertColumn(0, L"序号", 50);
        m_list.InsertColumn(1, L"歌曲", 500);
        m_list.InsertColumn(2, L"路径", 420);
        EnumerateMP3Files(LR"(I:\音乐\)");

        std::vector<LPCWSTR> files =
        {
            LR"(I:\音乐\The Tech Thieves - Fake.mp3)",
            LR"(I:\音乐\音乐\低调组合 - 终点起点.mp3)",
            LR"(I:\音乐\音乐\低调组合 - 夜空中最亮的星.mp3)",
            LR"(I:\音乐\陈奕迅 - 淘汰.mp3)",
            LR"(I:\音乐\周杰伦\10-2006-依然范特西\周杰伦 - 本草纲目.mp3)",
            LR"(I:\音乐\音乐\王菲 - 但愿人长久.mp3)",
            LR"(I:\音乐\音乐\Beyoncé - Halo.mp3)",
            LR"(I:\音乐\音乐\和田光司 - Butter-Fly - tri.mp3)"
        };


        m_hStream = BASS_StreamCreateFile(FALSE, files.back(), 0, 0, BASS_SAMPLE_FLOAT);
        if (m_hStream)
        {
            BASS_ChannelPlay(m_hStream, FALSE);
            BASS_ChannelSetPosition(m_hStream, BASS_ChannelSeconds2Bytes(m_hStream, 10.), BASS_POS_BYTE);
        }
        SetTimer(hWnd, 100, 1000, 0);
        if (1)
        {
            SetTimer(hWnd, 200, 10, 0);
        }
        else
        {
            std::thread _th([hWnd]()
            {
                while (IsWindow(hWnd))
                {
                    SendMessageW(hWnd, WM_TIMER, 200, 0);
                    Sleep(1);
                }
            });
            _th.detach();
        }
        // 枚举  目录下所有 krc文件
        //EnumerateKRCFiles(LR"(J:\cahce\kugou\Lyric)");
        std::vector<LPCWSTR> krcs =
        {
            LR"(J:\cahce\kugou\Lyric\The Tech Thieves - Fake-cf6d70385ebd673a9f423ed466bd200d-125671670-00000000.krc)",
            LR"(J:\cahce\kugou\Lyric\低调组合 - 终点起点-6257a78a9df6fb551f2324dbc08b4cf7-121741063-00000000.krc)",
            LR"(T:\移动机械硬盘\E源码备份\易语言备份\1自己写的源码\实用工具\播放器\lrc\低调组合 - 夜空中最亮的星.krc)",
            LR"(J:\cahce\kugou\Lyric\陈奕迅 - 淘汰-ea514c1f8eaee9f24dcd1f26575bac4f-135600792-00000000.krc)",
            LR"(J:\cahce\kugou\Lyric\周杰伦 - 本草纲目-50f657c1d53e3acb1381ef97e5cfabd2-108581021-00000000.krc)",
            LR"(J:\cahce\kugou\Lyric\王菲 - 但愿人长久-001e34650e0104930a54d570cb43f994-38649663-00000000.krc)",
            LR"(J:\cahce\kugou\Lyric\和田光司 - Butter-Fly - tri-073ce978dbff1f0ecf82304a0097369b-202443788-00000000.krc)",
        };
        std::string data;
        read_file(krcs.back(), data);
        LYRIC_WND_ARG arg{};
        lyric_wnd_get_default_arg(&arg);
        arg.rcWindow = { 300, 800, 1000, 1000 };
        //arg.pszFontName = L"黑体";
        m_hLyricWindow = lyric_wnd_create(&arg, OnLyricCommand, 0);
        lyric_wnd_load_krc(m_hLyricWindow, data.c_str(), (int)data.size(), false);
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

            lyric_wnd_load_krc(m_hLyricWindow, data.c_str(), (int)data.size(), false);
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


static bool IsCheck(HWND hWnd)
{
    return SendMessageW(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

static bool SetCheck(HWND hWnd, bool isCheck)
{
    return SendMessageW(hWnd, BM_SETCHECK, isCheck ? BST_CHECKED : BST_UNCHECKED, 0);
}

LPCWSTR psz_krc = LR"_KRC_([ti:It's Not Right (&friends Remake) (Radio Edit)]
[ar:Gianni Romano/Emanuele Esposito/&Friends/Helen Tesfazghi]
[al:It's Not Right (&friends Remake) (Radio Edit)]
[by:]
[offset:0]
[language:eyJjb250ZW50IjogW3sibHlyaWNDb250ZW50IjogW1siVE1FXHU0ZWFiXHU2NzA5XHU2NzJjXHU3ZmZiXHU4YmQxXHU0ZjVjXHU1NGMxXHU3Njg0XHU4NDU3XHU0ZjVjXHU2NzQzIl0sIFsiIl0sIFsiIl0sIFsiXHU4ZmQ5XHU2NjJmXHU0ZTBkXHU1YmY5XHU3Njg0IFx1NGY0Nlx1NmNhMVx1NTE3M1x1N2NmYiJdLCBbIlx1OGZkOVx1NjYyZlx1NGUwZFx1NWJmOVx1NzY4NCBcdTRmNDZcdTZjYTFcdTUxNzNcdTdjZmIiXSwgWyJcdTY1ZTBcdThiYmFcdTU5ODJcdTRmNTVcdTYyMTFcdTkwZmRcdTg5ODFcdTYyMTBcdTUyOWYiXSwgWyJcdThmZDlcdTY4MzdcdTRlMGRcdTViZjkiXSwgWyJcdThmZDlcdTY4MzdcdTRlMGRcdTViZjkiXSwgWyJcdThmZDlcdTY2MmZcdTRlMGRcdTViZjlcdTc2ODQgXHU0ZjQ2XHU2Y2ExXHU1MTczXHU3Y2ZiIl0sIFsiXHU4ZmQ5XHU2NjJmXHU0ZTBkXHU1YmY5XHU3Njg0IFx1NGY0Nlx1NmNhMVx1NTE3M1x1N2NmYiJdLCBbIlx1OGZkOVx1NjYyZlx1NGUwZFx1NWJmOVx1NzY4NCBcdTRmNDZcdTZjYTFcdTUxNzNcdTdjZmIiXSwgWyJcdTY1ZTBcdThiYmFcdTU5ODJcdTRmNTVcdTYyMTFcdTkwZmRcdTg5ODFcdTYyMTBcdTUyOWYiXSwgWyJcdThmZDlcdTY2MmZcdTRlMGRcdTViZjlcdTc2ODQgXHU0ZjQ2XHU2Y2ExXHU1MTczXHU3Y2ZiIl0sIFsiXHU2NWUwXHU4YmJhXHU1OTgyXHU0ZjU1XHU2MjExXHU5MGZkXHU4OTgxXHU2MjEwXHU1MjlmIl0sIFsiXHU2NTM2XHU2MmZlXHU0ZjYwXHU3Njg0XHU4ODRjXHU2NzRlXHU3OWJiXHU1ZjAwXHU1NDI3Il0sIFsiXHU0ZjExXHU2MGYzXHU1MThkXHU2NzY1XHU2MjdlXHU2MjExIl0sIFsiXHU4ZmQ5XHU2NjJmXHU0ZTBkXHU1YmY5XHU3Njg0IFx1NGY0Nlx1NmNhMVx1NTE3M1x1N2NmYiJdLCBbIlx1NjVlMFx1OGJiYVx1NTk4Mlx1NGY1NVx1NjIxMVx1OTBmZFx1ODk4MVx1NjIxMFx1NTI5ZiJdLCBbIlx1NTE3M1x1OTVlOFx1NTQwZVx1OGJmN1x1NjI4YVx1OTRhNVx1NTMxOVx1NzU1OVx1NGUwYiJdLCBbIlx1NjIxMVx1NWI4MVx1NjEzZlx1NWI2NFx1NzJlY1x1NGU1Zlx1NGUwZFx1NjEzZlx1OTZiZVx1OGZjNyJdLCBbIlx1OGZkOVx1NjYyZlx1NGUwZFx1NWJmOVx1NzY4NCBcdTRmNDZcdTZjYTFcdTUxNzNcdTdjZmIiXSwgWyJcdTY1ZTBcdThiYmFcdTU5ODJcdTRmNTVcdTYyMTFcdTkwZmRcdTg5ODFcdTYyMTBcdTUyOWYiXSwgWyJcdTY1MzZcdTYyZmVcdTRmNjBcdTc2ODRcdTg4NGNcdTY3NGVcdTc5YmJcdTVmMDBcdTU0MjciXSwgWyJcdTRmMTFcdTYwZjNcdTUxOGRcdTY3NjVcdTYyN2VcdTYyMTEiXSwgWyJcdThmZDlcdTY2MmZcdTRlMGRcdTViZjlcdTc2ODQgXHU0ZjQ2XHU2Y2ExXHU1MTczXHU3Y2ZiIl0sIFsiXHU2NWUwXHU4YmJhXHU1OTgyXHU0ZjU1XHU2MjExXHU5MGZkXHU4OTgxXHU2MjEwXHU1MjlmIl0sIFsiXHU1MTczXHU5NWU4XHU1NDBlXHU4YmY3XHU2MjhhXHU5NGE1XHU1MzE5XHU3NTU5XHU0ZTBiIl0sIFsiXHU2MjExXHU1YjgxXHU2MTNmXHU1YjY0XHU3MmVjXHU0ZTVmXHU0ZTBkXHU2MTNmXHU5NmJlXHU4ZmM3Il0sIFsiXHU4ZmQ5XHU2NjJmXHU0ZTBkXHU1YmY5XHU3Njg0IFx1NGY0Nlx1NmNhMVx1NTE3M1x1N2NmYiJdLCBbIlx1OGZkOVx1NjYyZlx1NGUwZFx1NWJmOVx1NzY4NCBcdTRmNDZcdTZjYTFcdTUxNzNcdTdjZmIiXSwgWyJcdTY1ZTBcdThiYmFcdTU5ODJcdTRmNTVcdTYyMTFcdTkwZmRcdTg5ODFcdTYyMTBcdTUyOWYiXSwgWyJcdThmZDlcdTY4MzdcdTRlMGRcdTViZjkiXV0sICJ0eXBlIjogMSwgImxhbmd1YWdlIjogMH1dLCAiY29udGVudFYyIjogW10sICJ2ZXJzaW9uIjogMX0=]
[0,501]<0,35,0>It's <36,35,0>Not <72,35,0>Right (&<108,35,0>friends <143,35,0>Remake) (<179,35,0>Radio <215,35,0>Edit) - <251,35,0>Gianni <287,35,0>Romano/<323,35,0>Emanuele <359,35,0>Esposito/&<394,35,0>Friends/<430,35,0>Helen <466,35,0>Tesfazghi
[502,501]<0,35,0>Lyrics <36,35,0>by：<72,35,0>Fred <108,35,0>Jerkins <143,35,0>Iii/<179,35,0>Isaac <215,35,0>Phillips/<251,35,0>Lashawn <287,35,0>Ameen <323,35,0>Daniels/<359,35,0>Toni <394,35,0>Estes/<430,35,0>Rodney <466,35,0>Jerkins
[1004,501]<0,35,0>Composed <36,35,0>by：<72,35,0>Fred <108,35,0>Jerkins <143,35,0>Iii/<179,35,0>Isaac <215,35,0>Phillips/<251,35,0>Lashawn <287,35,0>Ameen <323,35,0>Daniels/<359,35,0>Toni <394,35,0>Estes/<430,35,0>Rodney <466,35,0>Jerkins
[1506,9096]<0,207,0>It's <207,385,0>not <592,480,0>right <7463,185,0>but <7648,327,0>it's <7975,1121,0>okay
[16921,5503]<0,272,0>It's <272,472,0>not <744,610,0>right <3658,211,0>but <3869,310,0>it's <4179,1324,0>okay
[24601,5279]<0,367,0>I'm <367,408,0>gonna <775,481,0>make <1256,527,0>it <1783,1549,0>anyway <3720,1559,0>anyway
[32616,1377]<0,191,0>It's <191,383,0>not <574,803,0>right
[48239,1301]<0,207,0>It's <207,498,0>not <705,596,0>right
[63771,9378]<0,231,0>It's <231,472,0>not <703,737,0>right <7679,176,0>but <7855,232,0>it's <8087,1291,0>okay
[79430,9146]<0,227,0>It's <227,474,0>not <701,674,0>right <7565,196,0>but <7761,377,0>it's <8138,1008,0>okay
[95017,5353]<0,263,0>It's <263,472,0>not <735,504,0>right <3728,208,0>but <3936,174,0>it's <4110,1243,0>okay
[102656,5316]<0,336,0>I'm <336,466,0>gonna <802,511,0>make <1313,500,0>it <1813,1547,0>anyway <3760,1556,0>anyway
[110629,3159]<0,214,0>It's <214,440,0>not <654,546,0>right <1703,176,0>but <1879,249,0>it's <2128,1031,0>okay
[114229,3448]<0,430,0>I'm <430,472,0>gonna <902,451,0>make <1353,518,0>it <1871,1577,0>anyway
[118084,3566]<0,439,0>Pack <439,536,0>your <975,661,0>bags <2007,401,0>up <2408,536,0>and <2944,622,0>leave
[122027,3768]<0,407,0>Don't <407,490,0>you <897,375,0>dare <1272,168,0>come <1440,447,0>running <1887,464,0>back <2351,485,0>to <2836,932,0>me
[126146,3381]<0,293,0>It's <293,458,0>not <751,583,0>right <1702,239,0>but <1941,346,0>it's <2287,1094,0>okay
[129815,3656]<0,498,0>I'm <498,449,0>gonna <947,486,0>make <1433,518,0>it <1951,1705,0>anyway
[133767,3451]<0,201,0>Close <201,290,0>the <491,265,0>door <756,670,0>behind <1426,490,0>you <1916,456,0>leave <2372,473,0>your <2845,606,0>key
[137602,4212]<0,138,0>I'd <138,373,0>rather <511,356,0>be <867,561,0>alone <1428,591,0>than <2019,1659,0>unhappy <3678,534,0>yeah
[141981,3017]<0,152,0>It's <152,400,0>not <552,545,0>right <1480,232,0>but <1712,320,0>it's <2032,985,0>okay
[145397,3554]<0,480,0>I'm <480,512,0>gonna <992,448,0>make <1440,457,0>it <1897,1657,0>anyway
[149317,3689]<0,466,0>Pack <466,534,0>your <1000,608,0>bags <1952,505,0>up <2457,463,0>and <2920,769,0>leave
[153252,3689]<0,443,0>Don't <443,511,0>you <954,312,0>dare <1266,168,0>come <1434,439,0>running <1873,490,0>back <2363,567,0>to <2930,759,0>me
[157517,3241]<0,209,0>It's <209,452,0>not <661,535,0>right <1513,273,0>but <1786,351,0>it's <2137,1104,0>okay
[161017,3646]<0,463,0>I'm <463,496,0>gonna <959,405,0>make <1364,627,0>it <1991,1655,0>anyway
[164951,3507]<0,317,0>Close <317,204,0>the <521,390,0>door <911,489,0>behind <1400,533,0>you <1933,473,0>leave <2406,586,0>your <2992,515,0>key
[168728,4230]<0,165,0>I'd <165,424,0>rather <589,449,0>be <1038,495,0>alone <1533,544,0>than <2077,1769,0>unhappy <3846,384,0>yeah
[172958,9409]<0,304,0>It's <304,519,0>not <823,599,0>right <7742,185,0>but <7927,319,0>it's <8246,1163,0>okay
[188726,5251]<0,212,0>It's <212,444,0>not <656,599,0>right <3751,162,0>but <3913,182,0>it's <4095,1156,0>okay
[196310,5401]<0,369,0>I'm <369,489,0>gonna <858,447,0>make <1305,479,0>it <1784,1521,0>anyway <3690,1711,0>anyway
[204279,1432]<0,263,0>It's <263,512,0>not <775,657,0>right)_KRC_";


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
    case IDOK:
    {
        break;
    }
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


