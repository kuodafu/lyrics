#include "lyric_exe.h"
#include <winsock2.h>
#include <mutex>
#include "../charset_stl.h"
#include <kuodafu_lyric_wnd.h>

#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Ws2_32.lib")

#ifdef _WIN64
#   ifdef _DEBUG
#       pragma comment(lib, "output/x64/lyric_desktop_libD.lib")
#   else
#       pragma comment(lib, "output/x64/lyric_desktop_lib.lib")
#   endif
#else
#   ifdef _DEBUG
#       pragma comment(lib, "output/x86/lyric_desktop_libD.lib")
#   else
#       pragma comment(lib, "output/x86/lyric_desktop_lib.lib")
#   endif
#endif


HINSTANCE   g_hInst;
HWND        g_hWnd;         // 消息窗口, 处理一些消息, 还有把一些特定事件转到窗口线程执行
HWND        g_hWndReceive;  // 消息通知窗口, 窗口有效的时候有消息会通知到这里

static std::mutex m_mtx_message;
static HWND m_hLyricWindow;

HWND lrc_exe_create_msg_window();
void ws_OnMessage(cJSON* data, LPCSTR type);
int CALLBACK OnLyricCommand(HWND hWindowLyric, int id, LPARAM lParam);



// 获取命令行, 根据命令行来决定开启哪些服务, 没有窗口, 全都是通过交互来控制
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);
    g_hInst = hInstance;

    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0)
    {
        wchar_t buf[100];
        swprintf_s(buf, L"WSAStartup 调用失败, 无法初始化ws, 错误码: %d", wsaResult);
        MessageBoxW(0, buf, L"WebSokect初始化失败", MB_ICONERROR);
        return 0;
    }
    lyric_wnd_init();

    LYRIC_WND_ARG arg{};
    lyric_wnd_get_default_arg(&arg);
    arg.rcWindow = { 300, 800, 1000, 1000 };
    //arg.pszFontName = L"黑体";
    m_hLyricWindow = lyric_wnd_create(&arg, OnLyricCommand, 0);

    if (lrc_exe_create_msg_window() && lrc_exe_init_cmdline())
    {
        MSG msg;
        while (GetMessageW(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    WSACleanup();
    lyric_wnd_uninit();
    return 0;
}


HWND lrc_exe_create_msg_window()
{
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MessageWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = g_hInst;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = L"www.kuodafu.com";
    wcex.hIconSm = wcex.hIcon;

    if (!RegisterClassExW(&wcex))
        return NULL;

    // 创建一个消息-only 窗口（不在任务栏，不显示，不有可见父窗口）
    return CreateWindowExW(0,
                           wcex.lpszClassName,
                           NULL,
                           0,
                           0, 0, 0, 0,
                           HWND_MESSAGE, // HWND_MESSAGE 表示创建消息-only 窗口
                           NULL,
                           wcex.hInstance,
                           NULL);
}

void ws_OnMessageReceived(LPCSTR message, size_t len, bool binary)
{
    if (binary)
    {

        return;
    }

    cJSON* json = cJSON_Parse(message);
    LPCSTR type = cJSON_GetStringValue(cJSON_GetObjectItem(json, "type"));
    cJSON* data = cJSON_GetObjectItem(json, "data");
    if (type && *type && data)
        ws_OnMessage(data, type);
    cJSON_Delete(json);
}

void ws_OnMessage(cJSON* data, LPCSTR type)
{
    // 歌词这个不是线程安全, 得加锁处理
    std::lock_guard<std::mutex> lock(m_mtx_message);

#define _cmp(_s) (_stricmp(type, _s) == 0)
    if (_cmp("lyrics"))
    {
        double currentTime = cJSON_GetNumberValue(cJSON_GetObjectItem(data, "currentTime"));
        LPCSTR lyricsData = cJSON_GetStringValue(cJSON_GetObjectItem(data, "lyricsData"));
        auto w = charset_stl::U2W(lyricsData);
        lyric_wnd_load_krc(m_hLyricWindow, w.c_str(), (int)w.size(), true);
        lyric_wnd_update(m_hLyricWindow, (int)(currentTime * 1000.));

        //SYSTEMTIME st;
        //GetLocalTime(&st);
        //char buf[260];
        //sprintf_s(buf, "[%02d:%02d:%02d.%03d] currentTime = %f\n",
        //          st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
        //          currentTime);
        //OutputDebugStringA(buf);

        return;
    }

    if (_cmp("playerState"))
    {
        bool isPlaying = cJSON_IsTrue(cJSON_GetObjectItem(data, "isPlaying"));
        if (isPlaying)
            lyric_wnd_call_event(m_hLyricWindow, LYRIC_WND_BUTTON_ID_PLAY);
        else
            lyric_wnd_call_event(m_hLyricWindow, LYRIC_WND_BUTTON_ID_PAUSE);

        return;
    }
#undef _cmp
}

static bool IsCheck(HWND hWnd)
{
    return SendMessageW(hWnd, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

static bool SetCheck(HWND hWnd, bool isCheck)
{
    return SendMessageW(hWnd, BM_SETCHECK, isCheck ? BST_CHECKED : BST_UNCHECKED, 0);
}
int CALLBACK OnLyricCommand(HWND hWindowLyric, int id, LPARAM lParam)
{
    switch (id)
    {
    case LYRIC_WND_BUTTON_ID_LOCK:
        break;
    case LYRIC_WND_BUTTON_ID_UNLOCK:
        break;
    case LYRIC_WND_BUTTON_ID_SHOW:
        break;
    case LYRIC_WND_BUTTON_ID_CLOSE:
        break;
    case LYRIC_WND_BUTTON_ID_NEXT:
    case LYRIC_WND_BUTTON_ID_PREV:
    case LYRIC_WND_BUTTON_ID_PLAY:
    case LYRIC_WND_BUTTON_ID_PAUSE:
    {
        //if (!g_ws_connected)
        //    break;
        LPCSTR command = "";
        switch (id)
        {
        case LYRIC_WND_BUTTON_ID_NEXT:
            command = "next";
            break;
        case LYRIC_WND_BUTTON_ID_PREV:
            command = "prev";
            break;
        case LYRIC_WND_BUTTON_ID_PLAY:
        case LYRIC_WND_BUTTON_ID_PAUSE:
            command = "toggle";
            break;
        default:
            break;
        }

        cJSON* json = cJSON_CreateObject();
        cJSON_AddStringToObject(json, "type", "control");
        cJSON* data = cJSON_AddObjectToObject(json, "data");
        cJSON_AddStringToObject(data, "command", command);
        char* str = cJSON_PrintUnformatted(json);

        //TODO 把消息发送出去
        std::string str_msg = str;

        lrc_exe_ws_client_send(str_msg);
        lrc_exe_ws_server_send(str_msg);
        if (g_hWndReceive)
        {
            // 给窗口通知消息
            //SendMessageW(g_hWndReceive, LM_XXX, 0, 0);
        }

        cJSON_Delete(json);
        cJSON_free(str);
        break;
    }
    default:
        break;
    }
    return 0;
}

