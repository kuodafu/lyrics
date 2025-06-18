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
#include <assist/assist.h>
#include <charset_stl.h>
#include <cJSON/cJSON.h>
#include "bass.h"
#include <d2d/Color.h>

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

#define ID_LOCK 5000
#define ID_SHOW 5001

#define ID_UPDATE_MP3 5010
#define ID_UPDATE_LRC 5011

// 全局变量:
static HINSTANCE hInst;         // 当前实例
static HSTREAM m_hStream;       // 音乐播放句柄
static HWND m_hLyricWindow;     // 歌词窗口句柄
static HWND m_hWnd;             // 主窗口句柄
static HWND m_list;             // 歌曲列表
static HWND m_listlrc;          // 歌词列表
static HWND m_edit_mp3;         // 音乐目录
static HWND m_edit_lrc;         // 歌词目录
static HWND m_static_state;     // 显示一些操作状态

struct LIST_DATA_LRC
{
    std::wstring szIndex;
    std::wstring szName;
    std::wstring szPath;
};
struct LIST_DAT
{
    int index{ 0 };  // 序号, 排序用

    std::wstring szIndex;
    std::wstring szName;
    std::wstring szPath;
    std::vector<LIST_DATA_LRC> lrc_arr;  // 歌词文件数组, 为0的时候去搜索, 不为0就是已经有了
};
static std::vector<LIST_DAT> m_data;
static std::vector<LIST_DATA_LRC>* m_datalrc;


BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK OnLyricCommand(HWND hWindowLyric, int id, LPARAM lParam);
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnNotify_mp3_list(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnNotify_lrc_list(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void find_krc(const std::wstring& name, std::vector<LIST_DATA_LRC>& arr);
void EnumerateMP3Files(const std::wstring& directory);


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




_NODISCARD inline UINT _hash_str(const char* _First) noexcept
{
    UINT _FNV_offset_basis = 2166136261U;
    constexpr UINT _FNV_prime = 16777619U;
    while (*_First)
    {
        _FNV_offset_basis ^= static_cast<UINT>(*_First++);
        _FNV_offset_basis *= _FNV_prime;
    }

    return _FNV_offset_basis;
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

    auto author = (LPCSTR)u8"kuodafu QQ: 121007124, group: 20752843";
    auto xxx = _hash_str(author);

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



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        m_hWnd = hWnd;
        BASS_Init(-1, 44100, 0, hWnd, nullptr);
        UINT dpi = GetDpiForWindow(hWnd);
        auto scale = [dpi](int x) -> int
            {
                return MulDiv(x, dpi, 96);
            };

        static HFONT hFont;
        if (!hFont)
        {
            int height = scale(-14);
            LOGFONTW lf = { 0 };
            SystemParametersInfoW(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, 0);
            lf.lfHeight = height;
            lf.lfItalic = 0;
            lf.lfUnderline = 0;
            lf.lfStrikeOut = 0;
            lf.lfCharSet = GB2312_CHARSET;
            wcscpy_s(lf.lfFaceName, L"微软雅黑");
            hFont = CreateFontIndirectW(&lf);
        }


        const int _8 = scale(8);
        const int _24 = scale(24);
        const int _32 = scale(32);
        const int _80 = scale(80);
        const int _100 = scale(100);
        const int _150 = scale(150);
        const int _800 = scale(800);

        int next_left = 0, next_top = 0;

        auto pfn_create_edit = [&](int left, int top, int id, LPCWSTR name)
            {
                HWND hStatic = CreateWindowExW(0, WC_STATICW, name, WS_CHILD | WS_VISIBLE,
                                               left, top, _80, _24, hWnd, 0, hInst, nullptr);
                SendMessageW(hStatic, WM_SETFONT, (WPARAM)hFont, TRUE);
                HWND hChild = CreateWindowExW(0x00000200, WC_EDITW, nullptr, 0x54010000,
                                              left + _80, top, _800, _24, hWnd, 0, hInst, nullptr);
                SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

                HWND hBtn = CreateWindowExW(0, WC_BUTTONW, L"更新目录", 0x54010000,
                                            left + _80 + _800, top, _80, _24, hWnd, (HMENU)(ULONG_PTR)id, hInst, nullptr);
                SendMessageW(hBtn, WM_SETFONT, (WPARAM)hFont, TRUE);

                next_left = left + _80 + _800 + _80;
                next_top = top + _24;
                return hChild;
            };

        m_edit_mp3 = pfn_create_edit(_8, _8, ID_UPDATE_MP3, L"歌曲目录:");
        m_edit_lrc = pfn_create_edit(_8, _32 + _8, ID_UPDATE_LRC, L"歌词目录:");

        SetWindowTextW(m_edit_mp3, L"I:\\音乐\\");
        SetWindowTextW(m_edit_lrc, L"J:\\cahce\\kugou\\Lyric\\");

        auto pfn_create = [&](int left, int top, int width, int height, int id, LPCWSTR name)
            {
                HWND hChild = CreateWindowExW(0, L"BUTTON", name, WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,
                                              left, top, width, height, hWnd, (HMENU)(LONG_PTR)id, hInst, nullptr);
                SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);

                next_left = left + width;
                next_top = top + height;
                return hChild;
            };

        int btn_top = next_top + _8;

        pfn_create(_8, btn_top, _100, _32, ID_LOCK, L"锁定歌词");
        pfn_create(next_left + _8, btn_top, _100, _32, ID_SHOW, L"歌词可视");

        m_static_state = CreateWindowExW(0, WC_STATICW, L"这里把上一首/下一首按钮的功能改成了快进快退", WS_CHILD | WS_VISIBLE,
                                         next_left + _8, btn_top, _800, _24, hWnd, 0, hInst, nullptr);
        SendMessageW(m_static_state, WM_SETFONT, (WPARAM)hFont, TRUE);

        auto pfn_create_list = [&](int left, int top, int width, int height)
            {
                HWND hChild = CreateWindowExW(0x00000200, WC_LISTVIEWW, L"", 0x5600104D,
                                              left, top, width, height, hWnd, 0, hInst, nullptr);
                SendMessageW(hChild, WM_SETFONT, (WPARAM)hFont, TRUE);
                ListView_SetExtendedListViewStyle(hChild, 0x00010423);
                next_left = left + width;
                next_top = top + height;
                return hChild;
            };
        auto pfn_insert_column = [](HWND hList, int iSubItem, LPCWSTR text, int width) -> int
            {
                if (!text)
                    text = L"";
                LVCOLUMNW data = { 0 };
                data.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH;
                data.fmt = 0;
                data.iSubItem = iSubItem;
                data.cx = width;
                data.pszText = (LPWSTR)text;
                data.cchTextMax = (int)wcslen(text);
                data.iImage = 0;
                return ListView_InsertColumn(hList, iSubItem, &data);
            };

        int list_top = next_top + _8;

        m_list = pfn_create_list(_8, list_top, scale(1000), scale(600));
        pfn_insert_column(m_list, 0, L"序号", scale(50));
        pfn_insert_column(m_list, 1, L"歌曲", scale(500));
        pfn_insert_column(m_list, 2, L"路径", scale(420));

        m_listlrc = pfn_create_list(next_left + _8, list_top, scale(400), scale(600));
        pfn_insert_column(m_listlrc, 0, L"序号", scale(50));
        pfn_insert_column(m_listlrc, 1, L"歌词", scale(530));

        // 调用事件, 刷新歌曲目录
        OnCommand(hWnd, ID_UPDATE_MP3, 0);

        char* json_config = nullptr;
        {
            // 横向窗口一些默认值, 有高度用高度
            // 左边: (屏幕宽度 - 窗口宽度) / 2
            // 顶边: 屏幕底边 - 窗口高度 - 100
            // 宽度: 800
            // 高度: 高度不允许设置, 内部根据字体大小自动调整

            cJSON* json = cJSON_CreateObject();
            auto author = (LPCSTR)u8"kuodafu QQ: 121007124, group: 20752843";
            cJSON_AddStringToObject(json, "author", author);    // 1600659896
            cJSON_AddNumberToObject(json, "refreshRate", 144);
            cJSON_AddBoolToObject(json, "bVertical", false);
            cJSON_AddBoolToObject(json, "bSingleLine", false);
            cJSON_AddBoolToObject(json, "bSelfy", false);
            cJSON_AddBoolToObject(json, "bSelyy", false);


            cJSON_AddStringToObject(json, "szDefText", (LPCSTR)u8"没有歌词时的默认文本, 可以做广告之类的");
            cJSON_AddNumberToObject(json, "padding_text", 5);
            cJSON_AddNumberToObject(json, "padding_wnd", 8);
            cJSON_AddNumberToObject(json, "strokeWidth", 2.2);
            cJSON_AddNumberToObject(json, "strokeWidth_div", 0);
            cJSON_AddBoolToObject(json, "fillBeforeDraw", false);
            cJSON_AddNumberToObject(json, "nLineSpace", 2);

            // 歌词模式, 对齐方式
            cJSON* lyric_mode = cJSON_AddObjectToObject(json, "lyric_mode");
            cJSON* line1 = cJSON_AddObjectToObject(lyric_mode, "line1");
            cJSON* line2 = cJSON_AddObjectToObject(lyric_mode, "line2");
            cJSON_AddNumberToObject(line1, "align", 0);
            cJSON_AddNumberToObject(line2, "align", 2);

            cJSON_AddStringToObject(json, "szFontName", (LPCSTR)u8"微软雅黑");
            cJSON_AddNumberToObject(json, "nFontSize", 24);
            cJSON_AddNumberToObject(json, "lfWeight", 700);

            cJSON* rect_v = cJSON_AddObjectToObject(json, "rect_v");
            cJSON* rect_h = cJSON_AddObjectToObject(json, "rect_h");
            cJSON_AddNumberToObject(rect_v, "left", 1700);
            cJSON_AddNumberToObject(rect_v, "top", 100);
            cJSON_AddNumberToObject(rect_v, "right", 2000);
            cJSON_AddNumberToObject(rect_v, "bottom", 800);
            cJSON_AddNumberToObject(rect_h, "left", 300);
            cJSON_AddNumberToObject(rect_h, "top", 800);
            cJSON_AddNumberToObject(rect_h, "right", 1000);
            cJSON_AddNumberToObject(rect_h, "bottom", 1000);

            cJSON* clrNormal = cJSON_AddArrayToObject(json, "clrNormal");
            cJSON* clrNormal_GradientStop = cJSON_AddArrayToObject(json, "clrNormal_GradientStop");
            cJSON* clrLight = cJSON_AddArrayToObject(json, "clrLight");
            cJSON* clrLight_GradientStop = cJSON_AddArrayToObject(json, "clrLight_GradientStop");
            cJSON_AddItemToArray(clrNormal, cJSON_CreateNumber(MAKEARGB(255, 0, 109, 178)));
            cJSON_AddItemToArray(clrNormal, cJSON_CreateNumber(MAKEARGB(255, 3, 189, 241)));
            cJSON_AddItemToArray(clrNormal, cJSON_CreateNumber(MAKEARGB(255, 3, 202, 252)));

            cJSON_AddItemToArray(clrLight, cJSON_CreateNumber(MAKEARGB(255, 255,255,255)));
            cJSON_AddItemToArray(clrLight, cJSON_CreateNumber(MAKEARGB(255, 130,247,253)));
            cJSON_AddItemToArray(clrLight, cJSON_CreateNumber(MAKEARGB(255, 3, 233, 252)));

            cJSON_AddItemToArray(clrNormal_GradientStop, cJSON_CreateNumber(0));
            cJSON_AddItemToArray(clrNormal_GradientStop, cJSON_CreateNumber(0.5));
            cJSON_AddItemToArray(clrNormal_GradientStop, cJSON_CreateNumber(1));

            cJSON_AddItemToArray(clrLight_GradientStop, cJSON_CreateNumber(0));
            cJSON_AddItemToArray(clrLight_GradientStop, cJSON_CreateNumber(0.5));
            cJSON_AddItemToArray(clrLight_GradientStop, cJSON_CreateNumber(1));

            cJSON_AddNumberToObject(json, "clrBorderNormal", MAKEARGB(255, 33, 33, 33));
            cJSON_AddNumberToObject(json, "clrBorderLight", MAKEARGB(255, 33, 33, 33));
            cJSON_AddNumberToObject(json, "clrWndBack", MAKEARGB(100, 0, 0, 0));
            cJSON_AddNumberToObject(json, "clrWndBorder", MAKEARGB(200, 0, 0, 0));
            cJSON_AddNumberToObject(json, "clrLine", MAKEARGB(100, 255, 255, 255));



            cJSON* debug = cJSON_AddObjectToObject(json, "debug");
            //cJSON_AddNumberToObject(debug, "clrTextBackNormal", MAKEARGB(200, 0, 255, 0));
            //cJSON_AddNumberToObject(debug, "clrTextBackLight", MAKEARGB(200, 255, 0, 0));
            cJSON_AddNumberToObject(debug, "clrTextBackNormal", 0);
            cJSON_AddNumberToObject(debug, "clrTextBackLight", 0);
            cJSON_AddBoolToObject(debug, "alwaysFillBack", 0);
            cJSON_AddBoolToObject(debug, "alwaysDraw", 0);
            cJSON_AddBoolToObject(debug, "alwaysCache", 0);
            cJSON_AddBoolToObject(debug, "alwaysCache1", 0);

            json_config = cJSON_PrintUnformatted(json);
            cJSON_Delete(json);

            //_str base = wstr::base64_encode(json_config);
            //SetClipboard(base.c_str());
        }


        m_hLyricWindow = lyric_desktop_create(json_config, OnLyricCommand, 0);
        if (json_config)
            cJSON_free(json_config);


        // 更新bass cpu占用率
        SetTimer(hWnd, 100, 1000, [](HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
                 {
                     wchar_t text[64];
                     swprintf_s(text, L"bass CPU 占用 %.2f%%", BASS_GetCPU());
                     SetWindowTextW(hWnd, text);
                 });

        // 更新播放进度
        SetTimer(hWnd, 200, 10, [](HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
                 {
                     if (!m_hStream)
                         return;

                     double pos = BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetPosition(m_hStream, BASS_POS_BYTE)) * 1000.;
                     lyric_desktop_update(m_hLyricWindow, static_cast<int>(pos));
                 });


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
        RECT rc;
        PAINTSTRUCT ps;
        GetClientRect(hWnd, &rc);
        HDC hdc = BeginPaint(hWnd, &ps);

        FillRect(hdc, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
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
        if (pnmh->hwndFrom == m_list)
            return OnNotify_mp3_list(hWnd, message, wParam, lParam);
        if (pnmh->hwndFrom == m_listlrc)
            return OnNotify_lrc_list(hWnd, message, wParam, lParam);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK OnNotify_lrc_list(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto pnmh = (LPNMHDR)lParam;
    switch (pnmh->code)
    {
    case NM_DBLCLK:
    {
        auto pItem = (LPNMITEMACTIVATE)lParam;
        int index = pItem->iItem;
        if (!m_datalrc || index < 0 || index >= (int)m_datalrc->size())
            break;
        auto& item = m_datalrc->at(index);
        int nType = LYRIC_PARSE_TYPE_KRC | LYRIC_PARSE_TYPE_UTF16 | LYRIC_PARSE_TYPE_PATH;
        lyric_desktop_load_lyric(m_hLyricWindow, item.szPath.c_str(), -1, static_cast<LYRIC_PARSE_TYPE>(nType));
        break;
    }
    case LVN_GETDISPINFOW:
    {
        auto pDispInfo = (LPNMLVDISPINFOW)lParam;
        int index = pDispInfo->item.iItem;
        if (!m_datalrc || index < 0 || index >= (int)m_datalrc->size())
            break;
        const auto& item = m_datalrc->at(index);
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

LRESULT CALLBACK OnNotify_mp3_list(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto pnmh = (LPNMHDR)lParam;
    switch (pnmh->code)
    {
    case NM_DBLCLK:
    {
        auto pItem = (LPNMITEMACTIVATE)lParam;
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
        LPCWSTR pData = nullptr;
        if (size > 0)
            pData = m_datalrc->front().szPath.c_str();

        ListView_SetItemCountEx(m_listlrc, (int)size, LVSICF_NOSCROLL);

        int nType = LYRIC_PARSE_TYPE_KRC | LYRIC_PARSE_TYPE_UTF16 | LYRIC_PARSE_TYPE_PATH;
        lyric_desktop_load_lyric(m_hLyricWindow, pData, -1, static_cast<LYRIC_PARSE_TYPE>(nType));
        lyric_desktop_call_event(m_hLyricWindow, LYRIC_DESKTOP_BUTTON_ID_PLAY);
        break;
    }
    case LVN_GETDISPINFOW:
    {
        auto pDispInfo = (LPNMLVDISPINFOW)lParam;
        int index = pDispInfo->item.iItem;
        if (index < 0 || index >= (int)m_data.size())
            break;
        const auto& item = m_data.at(index);
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
        ListView_SetItemCountEx(m_list, (int)m_data.size(), LVSICF_NOSCROLL);
        break;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


// “关于”框的消息处理程序。
bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND hChild = (HWND)lParam;
    const int id = LOWORD(wParam);
    const int code = HIWORD(wParam);
    const bool isCheck = SendMessageW(hChild, BM_GETCHECK, 0, 0) == BST_CHECKED;
    switch (id)
    {
    case ID_LOCK:
        lyric_desktop_call_event(m_hLyricWindow, isCheck ? LYRIC_DESKTOP_BUTTON_ID_LOCK : LYRIC_DESKTOP_BUTTON_ID_UNLOCK);
        break;
    case ID_SHOW:
        lyric_desktop_call_event(m_hLyricWindow, isCheck ? LYRIC_DESKTOP_BUTTON_ID_SHOW : LYRIC_DESKTOP_BUTTON_ID_CLOSE);
        break;
    case ID_UPDATE_MP3:
    {
        int len = GetWindowTextLengthW(m_edit_mp3);
        std::wstring dir(len, 0);
        GetWindowTextW(m_edit_mp3, &dir[0], len + 1);

        m_data.clear();
        EnumerateMP3Files(dir);
        ListView_SetItemCountEx(m_list, (int)m_data.size(), LVSICF_NOSCROLL);
        break;
    }
    case ID_UPDATE_LRC:
    {
        // 更新歌词就把列表里的歌词数组删了, 等使用的时候再根据名字重新查找
        for (auto& item : m_data)
        {
            item.lrc_arr.clear();
        }
        break;
    }
    case IDOK:
    {
        break;
    }
    default:
        return false;
    }
    return true;
}

static void set_state(LPCWSTR fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::wstring text;
    text.resize(1024);
    vswprintf_s(&text[0], text.size(), fmt, args);
    va_end(args);
    SetWindowTextW(m_static_state, text.c_str());
}

static bool SetCheck(HWND hWnd, bool isCheck)
{
    return SendMessageW(hWnd, BM_SETCHECK, isCheck ? BST_CHECKED : BST_UNCHECKED, 0);
}

int CALLBACK OnLyricCommand(HWND hWindowLyric, int id, LPARAM lParam)
{
    switch (id)
    {
    case LYRIC_DESKTOP_BUTTON_ID_LOCK:
    {
        SetCheck(GetDlgItem(m_hWnd, ID_LOCK), true);
        set_state(L"桌面歌词按下了 锁定按钮, ID: %d", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_UNLOCK:
    {
        SetCheck(GetDlgItem(m_hWnd, ID_LOCK), false);
        set_state(L"桌面歌词按下了 解锁按钮, ID: %d", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_SHOW:
    {
        SetCheck(GetDlgItem(m_hWnd, ID_SHOW), true);
        set_state(L"桌面歌词按下了 显示歌词按钮, ID: %d", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_CLOSE:
    {
        SetCheck(GetDlgItem(m_hWnd, ID_SHOW), false);
        set_state(L"桌面歌词按下了 关闭歌词按钮, ID: %d", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_NEXT:
    case LYRIC_DESKTOP_BUTTON_ID_PREV:
    {
        // 这里的上一首下一首的功能我给改成快进快退了....
        double d_pos = BASS_ChannelBytes2Seconds(m_hStream, BASS_ChannelGetPosition(m_hStream, BASS_POS_BYTE));
        double new_pos = id == LYRIC_DESKTOP_BUTTON_ID_NEXT ? 10. : -10.;
        BASS_ChannelSetPosition(m_hStream, BASS_ChannelSeconds2Bytes(m_hStream, d_pos + new_pos), BASS_POS_BYTE);
        LPCWSTR btn = id == LYRIC_DESKTOP_BUTTON_ID_NEXT ? L"下一首" : L"上一首";
        set_state(L"桌面歌词按下了 %s按钮, ID: %d, 功能已经改为快进快退了", btn, id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_PLAY:
    {
        BASS_ChannelPlay(m_hStream, FALSE);
        set_state(L"桌面歌词按下了 播放按钮, ID: %d, 这里可以判断当前是否可用播放, 如果不能播放, 返回非0值可以阻止按钮改变", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_PAUSE:
    {
        BASS_ChannelPause(m_hStream);
        set_state(L"桌面歌词按下了 暂停按钮, ID: %d, 如果暂停失败, 可以返回非0值, 返回非0值可以阻止按钮改变", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY:
    {
        set_state(L"桌面歌词按下了 音译按钮, ID: %d, 这里是要歌词改成音译模式", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY_SEL:
    {
        set_state(L"桌面歌词按下了 音译按钮_选中模式, ID: %d, 这里是要歌词从 音译模式 改成 普通模式", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY:
    {
        set_state(L"桌面歌词按下了 翻译按钮, ID: %d, 这里是要歌词改成翻译模式", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY_SEL:
    {
        set_state(L"桌面歌词按下了 翻译按钮_选中模式, ID: %d, 这里是要歌词从 翻译模式 改成 普通模式", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_LRCWRONG:
    case LYRIC_DESKTOP_BUTTON_ID_LRCWRONG_V:
    {
        set_state(L"桌面歌词按下了 歌词不对按钮, ID: %d, 这里可以弹出一个列表, 让用户选择其他歌词, 可以从网络获取, 或者从本地, 反正就是让用户选新的歌词文件", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_VERTICAL:
    {
        set_state(L"桌面歌词按下了 竖屏按钮, ID: %d, 改成竖屏模式", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_HORIZONTAL:
    {
        set_state(L"桌面歌词按下了 横屏按钮, ID: %d, 改成横屏模式", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_MAKELRC:
    case LYRIC_DESKTOP_BUTTON_ID_MAKELRC_V:
    {
        set_state(L"桌面歌词按下了 制作歌词按钮, ID: %d, 这个自己发挥了, 感觉没必要做制作歌词, 已经有成品的话, 这里可以对接", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_FONT_DOWN:
    {
        set_state(L"桌面歌词按下了 字体减小按钮, ID: %d, 每次减2", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_FONT_UP:
    {
        set_state(L"桌面歌词按下了 字体增加按钮, ID: %d, 每次加2", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_BEHIND:
    {
        set_state(L"桌面歌词按下了 歌词延后按钮, ID: %d, 每次延后0.5秒", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_AHEAD:
    {
        set_state(L"桌面歌词按下了 歌词提前按钮, ID: %d, 每次提前0.5秒", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_SETTING:
    {
        set_state(L"桌面歌词按下了 设置按钮, ID: %d, 这里自行发挥, 看是弹出设置窗口还是怎样", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_LRCCOLOR:
    {
        set_state(L"桌面歌词按下了 歌词配色按钮, ID: %d, 酷狗这个按钮是让用户选择歌词字体配色, 可以弹窗口让用户设置颜色, 然后设置桌面歌词配置", id);
        break;
    }
    case LYRIC_DESKTOP_BUTTON_ID_MENU:
    {
        set_state(L"桌面歌词按下了 菜单按钮, ID: %d, 自行发挥了.... 应该是要弹个菜单什么的", id);
        break;
    }
    default:
        break;
    }
    return 0;
}


void find_krc(const std::wstring& name, std::vector<LIST_DATA_LRC>& arr)
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
            if (wcscmp(findFileData.cFileName, L"..") != 0 && wcscmp(findFileData.cFileName, L".") != 0)
                EnumerateMP3Files(filePath);
        }
        else if (filePath.ends_with(L"mp3"))
        {
            // 有文件, 加载到列表里
            auto& item = m_data.emplace_back();

            item.index = (int)m_data.size();
            item.szIndex = std::to_wstring(item.index);
            item.szName = findFileData.cFileName;
            item.szPath = std::move(filePath);
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);


    FindClose(hFind);
}

