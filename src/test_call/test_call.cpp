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
#include <kuodafu_lyric_desktop.h>
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
static CListView m_listlrc;
struct LIST_DATA_LRC
{
    std::wstring szIndex;
    std::wstring szName;
    std::wstring szPath;
};
struct LIST_DAT
{
    int index{0};  // 序号, 排序用

    std::wstring szIndex;
    std::wstring szName;
    std::wstring szPath;
    std::vector<LIST_DATA_LRC> lrc_arr;  // 歌词文件数组, 为0的时候去搜索, 不为0就是已经有了
};
static std::vector<LIST_DAT> m_data;
static std::vector<LIST_DATA_LRC>* m_datalrc;

// 此代码模块中包含的函数的前向声明:
BOOL                InitInstance(HINSTANCE, int);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    OnNotify_lrc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    OnNotify_data(HWND, UINT, WPARAM, LPARAM);
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
int CALLBACK OnLyricCommand(HWND hWindowLyric, int id, LPARAM lParam);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR    lpCmdLine,
                      _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    lyric_desktop_init();

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
    lyric_desktop_uninit();
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

    do
    {
        std::wstring filePath = directory + L"\\" + findFileData.cFileName;
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (findFileData.cFileName[0] != L'.' && findFileData.cFileName[0] != L'京')
                EnumerateMP3Files(filePath);
        }
        else if (filePath.ends_with(L"mp3"))
        {
            // 有文件, 加载到列表里
            m_data.emplace_back();
            auto& item = m_data.back();
            item.index = (int)m_data.size();
            item.szIndex = std::to_wstring(item.index);
            item.szName = findFileData.cFileName;
            item.szPath = std::move(filePath);
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);


    FindClose(hFind);
}

static void find_krc(const std::wstring& name, std::vector<LIST_DATA_LRC>& arr)
{
    std::wstring directory = LR"(J:\cahce\kugou\Lyric\)";
    std::wstring searchPath = directory + L"*" + name + LR"(*.krc)";
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        for (wchar_t& ch : findFileData.cFileName)
            ch = towlower(ch);

        if (wcsstr(findFileData.cFileName, name.c_str()))
        {
            arr.emplace_back();
            auto& item = arr.back();
            item.szIndex = std::to_wstring(arr.size());
            item.szName = findFileData.cFileName;
            item.szPath = directory + findFileData.cFileName;
        }
        
    } while (FindNextFileW(hFind, &findFileData) != 0);


    FindClose(hFind);
    return;
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
        auto p = lyric_parse((LPBYTE)data.c_str(), (int)data.size(), LYRIC_PARSE_TYPE_KRC);
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

        m_list.create(hWnd, left + 150, 10, 1000, 800, 0x00000200, 0x56001045, 0x00010423, 0);
        m_list.SetFont(CWndBase::CreateFontW(L"微软雅黑", -18));
        m_list.InsertColumn(0, L"序号", 50);
        m_list.InsertColumn(1, L"歌曲", 500);
        m_list.InsertColumn(2, L"路径", 420);

        m_listlrc.create(hWnd, left + 1150, 10, 600, 800, 0x00000200, 0x56001045, 0x00010423, 0);
        m_listlrc.SetFont(CWndBase::CreateFontW(L"微软雅黑", -18));
        m_listlrc.InsertColumn(0, L"序号", 50);
        m_listlrc.InsertColumn(1, L"歌词", 530);
        EnumerateMP3Files(LR"(I:\音乐\)");

        m_list.SetItemCount((int)m_data.size());
        std::vector<LPCWSTR> files =
        {
            LR"(I:\音乐\The Tech Thieves - Fake.mp3)",
            LR"(I:\音乐\音乐\低调组合 - 终点起点.mp3)",
            LR"(I:\音乐\音乐\低调组合 - 夜空中最亮的星.mp3)",
            LR"(I:\音乐\陈奕迅 - 淘汰.mp3)",
            LR"(I:\音乐\周杰伦\10-2006-依然范特西\周杰伦 - 本草纲目.mp3)",
            LR"(I:\音乐\音乐\王菲 - 但愿人长久.mp3)",
            LR"(I:\音乐\音乐\Beyoncé - Halo.mp3)",
            LR"(I:\音乐\音乐\和田光司 - Butter-Fly - tri.mp3)",
            LR"(I:\音乐\周杰伦\05-2003-叶惠美\周杰伦 - 晴天.mp3)",
            LR"(I:\音乐\煌めく瞬間に捕われて (Cinema Version) (Bonus Track).mp3)",
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
        std::vector<LPCWSTR> qrc =
        {
            LR"(T:\tool\QQMusic\download\cache\QQMusicLyricNew\ - 煌めく瞬間に捕われて (Cinema Version) (Bonus Track) - 149 - _qmRoma.qrc)",

        };
        std::vector<LPCWSTR> lrc =
        {
            LR"(I:\cpp_public\lyrics\res\晴天-MusicEnc.lrc)",

        };
        std::string data;
        read_file(krcs.back(), data);
        LYRIC_DESKTOP_ARG arg{};
        lyric_desktop_get_default_arg(&arg);
        arg.rcWindow = { 300, 800, 1000, 1000 };
        //arg.pszFontName = L"黑体";
        m_hLyricWindow = lyric_desktop_create(&arg, OnLyricCommand, 0);
        lyric_desktop_load_lyric(m_hLyricWindow, data.c_str(), (int)data.size(), LYRIC_PARSE_TYPE_KRC);
        int nType = LYRIC_PARSE_TYPE_QRC | LYRIC_PARSE_TYPE_UTF16 | LYRIC_PARSE_TYPE_PATH;
        lyric_desktop_load_lyric(m_hLyricWindow, qrc.back(), -1, static_cast<LYRIC_PARSE_TYPE>(nType));
        lyric_desktop_call_event(m_hLyricWindow, LYRIC_DESKTOP_BUTTON_ID_PLAY);


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
            lyric_desktop_update(m_hLyricWindow, pos);
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
        if (pnmh->hwndFrom == m_list.m_hWnd)
            return OnNotify_data(hWnd, message, wParam, lParam);
        if (pnmh->hwndFrom == m_listlrc.m_hWnd)
            return OnNotify_lrc(hWnd, message, wParam, lParam);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK OnNotify_lrc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;
    switch (pnmh->code)
    {
    case NM_DBLCLK:
    {
        LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE)lParam;
        int index = pItem->iItem;
        if (!m_datalrc || index < 0 || index >= (int)m_datalrc->size())
            break;
        auto& item = m_datalrc->at(index);
        std::string data;
        read_file(item.szPath.c_str(), data);
        lyric_desktop_load_lyric(m_hLyricWindow, data.c_str(), (int)data.size(), LYRIC_PARSE_TYPE_KRC);
        break;
    }
    case LVN_GETDISPINFOW:
    {
        NMLVDISPINFOW* pDispInfo = (NMLVDISPINFOW*)lParam;
        int index = pDispInfo->item.iItem;
        if (!m_datalrc || index < 0 || index >= (int)m_datalrc->size())
            break;
        auto& item = m_datalrc->at(index);
        switch (pDispInfo->item.iSubItem)
        {
        case 0:
            pDispInfo->item.pszText = (LPWSTR)(item.szIndex.c_str());
            break;
        case 1:
            pDispInfo->item.pszText = (LPWSTR)item.szName.c_str();
            break;
        case 2:
            pDispInfo->item.pszText = (LPWSTR)item.szPath.c_str();
            break;
        }
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK OnNotify_data(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPNMHDR pnmh = (LPNMHDR)lParam;
    switch (pnmh->code)
    {
    case NM_DBLCLK:
    {
        LPNMITEMACTIVATE pItem = (LPNMITEMACTIVATE)lParam;
        int index = pItem->iItem;
        auto& item = m_data[index];
        
        const std::wstring& filePath = item.szPath;
        m_datalrc = &item.lrc_arr;

        BASS_StreamFree(m_hStream);
        m_hStream = BASS_StreamCreateFile(FALSE, filePath.c_str(), 0, 0, BASS_SAMPLE_FLOAT);
        BASS_ChannelPlay(m_hStream, FALSE);

        if (m_datalrc->empty())
        {
            std::wstring find_name = item.szName;

            find_name.erase(find_name.rfind(L'.'));
            for (wchar_t& ch : find_name)
                ch = towlower(ch);
            LPWSTR name = &find_name[0];

            find_krc(find_name, *m_datalrc);
        }

        size_t size = m_datalrc->size();
        std::string data;
        if (size > 0)
            read_file(m_datalrc->front().szPath.c_str(), data);

        m_listlrc.SetItemCount((int)size);
        m_listlrc.InvalidateRect();

        lyric_desktop_load_lyric(m_hLyricWindow, data.c_str(), (int)data.size(), LYRIC_PARSE_TYPE_KRC);
        lyric_desktop_call_event(m_hLyricWindow, LYRIC_DESKTOP_BUTTON_ID_PLAY);
        break;
    }
    case LVN_GETDISPINFOW:
    {
        NMLVDISPINFOW* pDispInfo = (NMLVDISPINFOW*)lParam;
        int index = pDispInfo->item.iItem;
        if (index < 0 || index >= (int)m_data.size())
            break;
        auto& item = m_data.at(index);
        switch (pDispInfo->item.iSubItem)
        {
        case 0:
            pDispInfo->item.pszText = (LPWSTR)item.szIndex.c_str();
            break;
        case 1:
            pDispInfo->item.pszText = (LPWSTR)item.szName.c_str();
            break;
        case 2:
            pDispInfo->item.pszText = (LPWSTR)item.szPath.c_str();
            break;
        }
        break;
    }
    case LVN_COLUMNCLICK:
    {
        LPNMLISTVIEW pNMLV = (LPNMLISTVIEW)lParam;
        int iSubItem = pNMLV->iSubItem;
        static bool bAscending = true;
        std::sort(m_data.begin(), m_data.end(), [iSubItem](const LIST_DAT& a, const LIST_DAT& b)
                  {
                      if (bAscending)
                      {
                          // 升序, 从小到大
                          switch (iSubItem)
                          {
                          case 0:
                              return a.index < b.index;
                          case 1:
                              return a.szName < b.szName;
                          case 2:
                              return a.szPath < b.szPath;
                          default:
                              __debugbreak();
                          }
                      }
                      else
                      {
                          // 降序, 从大到小
                          switch (iSubItem)
                          {
                          case 0:
                              return a.index > b.index;
                          case 1:
                              return a.szName > b.szName;
                          case 2:
                              return a.szPath > b.szPath;
                          default:
                              __debugbreak();
                          }
                      }

                      return false;
                  });
        bAscending = !bAscending;
        m_list.SetItemCount((int)m_data.size());
        m_list.InvalidateRect();
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
        lyric_desktop_call_event(m_hLyricWindow, isCheck ? LYRIC_DESKTOP_BUTTON_ID_LOCK : LYRIC_DESKTOP_BUTTON_ID_UNLOCK);
        break;
    case ID_SHOW:
        lyric_desktop_call_event(m_hLyricWindow, isCheck ? LYRIC_DESKTOP_BUTTON_ID_SHOW : LYRIC_DESKTOP_BUTTON_ID_CLOSE);
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
    case LYRIC_DESKTOP_BUTTON_ID_LOCK:
        SetCheck(GetDlgItem(m_hWnd, ID_LOCK), true);
        break;
    case LYRIC_DESKTOP_BUTTON_ID_UNLOCK:
        SetCheck(GetDlgItem(m_hWnd, ID_LOCK), false);
        break;
    case LYRIC_DESKTOP_BUTTON_ID_SHOW:
        SetCheck(GetDlgItem(m_hWnd, ID_SHOW), true);
        break;
    case LYRIC_DESKTOP_BUTTON_ID_CLOSE:
        SetCheck(GetDlgItem(m_hWnd, ID_SHOW), false);
        break;
    case LYRIC_DESKTOP_BUTTON_ID_NEXT:
    case LYRIC_DESKTOP_BUTTON_ID_PREV:
    {
        double d_pos = BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetPosition(m_hStream, BASS_POS_BYTE));
        double new_pos = id == LYRIC_DESKTOP_BUTTON_ID_NEXT ? 10. : -10.;
        BASS_ChannelSetPosition(m_hStream, BASS_ChannelSeconds2Bytes(m_hStream, d_pos + new_pos), BASS_POS_BYTE);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_PLAY:
    {
        BASS_ChannelPlay(m_hStream, FALSE);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_PAUSE:
    {
        BASS_ChannelPause(m_hStream);
        break;
    }
    default:
        break;
    }
    return 0;
}


