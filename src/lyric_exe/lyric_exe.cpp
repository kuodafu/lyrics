#include "lyric_exe.h"
#include <winsock2.h>
#include <mutex>
#include <charset_stl.h>
#include <kuodafu_lyric_desktop.h>

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
UINT        g_nReceiveMsg;  // 通知的消息值, 没有的话就默认为 WM_USER + 1
std::string g_json_config;  // 配置信息数据

static std::mutex m_mtx_message;
static HWND m_hLyricWindow;
static bool m_notify_click = true;

HWND lrc_exe_create_msg_window();
void ws_OnMessage(cJSON* params, LPCSTR type, LPCSTR id, PFN_WS_SEND_MESSAGE pfnSend);
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

    lyric_desktop_init();
    lrc_exe_init_cmdline();

    // 命令行初始化成功, 开始创建窗口
    m_hLyricWindow = lyric_desktop_create(g_json_config.c_str(), OnLyricCommand, 0);

    if (m_hLyricWindow && lrc_exe_create_msg_window())
    {
        // 创建窗口后再初始化websocket
        if (lrc_exe_init_cmdline_ws())
        {
            lrc_exe_ws_client_try_connect(g_hWnd);
            MSG msg;
            while (GetMessageW(&msg, nullptr, 0, 0))
            {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
        }
    }
    

    lyric_desktop_uninit();
    WSACleanup();
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

void ws_OnMessageReceived(LPCSTR message, size_t len, bool binary, PFN_WS_SEND_MESSAGE pfnSend)
{
    if (binary)
    {

        return;
    }

    cJSON* json = cJSON_Parse(message);
    LPCSTR id = cJSON_GetStringValue(cJSON_GetObjectItem(json, "id"));
    LPCSTR type = cJSON_GetStringValue(cJSON_GetObjectItem(json, "method"));
    cJSON* params = cJSON_GetObjectItem(json, "params");
    if (type && *type && params)
        ws_OnMessage(params, type, id, pfnSend);
    cJSON_Delete(json);
}

template<typename _Ty>
inline void _ws_ret_data(LPCSTR id, _Ty data, PFN_WS_SEND_MESSAGE pfnSend)
{
    if (!id || !*id || !pfnSend)
        return;

    cJSON* ret_json = cJSON_CreateObject();
    cJSON_AddStringToObject(ret_json, "id", id);
    cJSON* result = cJSON_AddObjectToObject(ret_json, "result");

    if constexpr (std::is_same_v<_Ty, LPCSTR> || std::is_same_v<_Ty, LPSTR>)
    {
        cJSON_AddStringToObject(result, "result", data);
    }
    else if constexpr (std::is_same_v<_Ty, int>)
    {
        cJSON_AddNumberToObject(result, "result", data);
    }
    else if constexpr (std::is_same_v<_Ty, bool>)
    {
        cJSON_AddBoolToObject(result, "result", data);
    }
    else
        throw std::exception("not support type");

    char* str_msg = cJSON_PrintUnformatted(ret_json);
    pfnSend(str_msg);
    cJSON_Delete(ret_json);
    cJSON_free(str_msg);
}

void ws_OnMessage(cJSON* params, LPCSTR type, LPCSTR id, PFN_WS_SEND_MESSAGE pfnSend)
{
    // 歌词这个不是线程安全, 得加锁处理
    std::lock_guard<std::mutex> lock(m_mtx_message);
#define _cmp(_s) (_stricmp(type, _s) == 0)

    if (_cmp("lyric_desktop_exit"))
    {
        DestroyWindow(m_hLyricWindow);
        DestroyWindow(g_hWnd);
        return;
    }
    if (_cmp("lyric_desktop_load_lyric"))
    {
        LPCSTR data = cJSON_GetStringValue(cJSON_GetObjectItem(params, "data"));
        LPCSTR file = cJSON_GetStringValue(cJSON_GetObjectItem(params, "file"));
        LPCSTR lyric = cJSON_GetStringValue(cJSON_GetObjectItem(params, "lyric"));
        if (!lyric || !*lyric) lyric = "krc";
        bool bRet = false;
        int nType = LYRIC_PARSE_TYPE_KRC;
        if (_stricmp(lyric, "qrc") == 0)
            nType = LYRIC_PARSE_TYPE_QRC;
        else if (_stricmp(lyric, "lrc") == 0)
            nType = LYRIC_PARSE_TYPE_LRC;

        if (data && *data)
        {
            nType |= LYRIC_PARSE_TYPE_UTF8 | LYRIC_PARSE_TYPE_DECRYPT;
            bRet = lyric_desktop_load_lyric(m_hLyricWindow, data, -1, static_cast<LYRIC_PARSE_TYPE>(nType));
        }
        else if(file && *file)
        {
            nType |= LYRIC_PARSE_TYPE_UTF8 | LYRIC_PARSE_TYPE_PATH;
            bRet = lyric_desktop_load_lyric(m_hLyricWindow, file, -1, static_cast<LYRIC_PARSE_TYPE>(nType));
        }

        _ws_ret_data(id, bRet, pfnSend);
        return;
    }

    if (_cmp("lyric_desktop_update"))
    {
        double time = cJSON_GetNumberValue(cJSON_GetObjectItem(params, "time"));
        bool bRet = lyric_desktop_update(m_hLyricWindow, static_cast<int>(time));
        _ws_ret_data(id, bRet, pfnSend);
        return;
    }

    if (_cmp("lyric_desktop_get_config"))
    {
        char* str_config = lyric_desktop_get_config(m_hLyricWindow);
        _ws_ret_data(id, str_config, pfnSend);
        lyric_desktop_free(str_config);
        return;
    }
    if (_cmp("lyric_desktop_set_config"))
    {
        LPCSTR str_config = cJSON_GetStringValue(cJSON_GetObjectItem(params, "config"));
        int nRet = lyric_desktop_set_config(m_hLyricWindow, str_config);
        _ws_ret_data(id, nRet, pfnSend);
        return;
    }
    if (_cmp("lyric_desktop_call_event"))
    {
        double btn_id = cJSON_GetNumberValue(cJSON_GetObjectItem(params, "id"));
        bool bRet = lyric_desktop_call_event(m_hLyricWindow, static_cast<LYRIC_DESKTOP_BUTTON_ID>(btn_id));
        _ws_ret_data(id, bRet, pfnSend);
        return;
    }
    if (_cmp("lyric_desktop_set_button_state"))
    {
        double btn_id = cJSON_GetNumberValue(cJSON_GetObjectItem(params, "id"));
        double state = cJSON_GetNumberValue(cJSON_GetObjectItem(params, "state"));
        bool bRet = lyric_desktop_set_button_state(m_hLyricWindow,
                                                   static_cast<LYRIC_DESKTOP_BUTTON_ID>(btn_id),
                                                   static_cast<LYRIC_DESKTOP_BUTTON_STATE>(state));
        _ws_ret_data(id, bRet, pfnSend);
        return;
    }
    if (_cmp("lyric_desktop_get_button_state"))
    {
        double btn_id = cJSON_GetNumberValue(cJSON_GetObjectItem(params, "id"));
        auto state = lyric_desktop_get_button_state(m_hLyricWindow,
                                                    static_cast<LYRIC_DESKTOP_BUTTON_ID>(btn_id));
        _ws_ret_data(id, static_cast<int>(state), pfnSend);
        return;
    }
    if (_cmp("lyric_desktop_disable"))
    {
        m_notify_click = false;
        return;
    }
    if (_cmp("lyric_desktop_enable"))
    {
        m_notify_click = true;
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
    LPCSTR command = "";
    switch (id)
    {
    case LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY:
        command = "yy";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_TRANSLATEYY_SEL:
        command = "yy_sel";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY:
        command = "fy";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_TRANSLATEFY_SEL:
        command = "fy_sel";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_LRCWRONG:
        command = "lrcwrong";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_VERTICAL:
        command = "vertical";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_MAKELRC:
        command = "makelrc";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_FONT_DOWN:
        command = "font_down";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_FONT_UP:
        command = "font_up";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_BEHIND:
        command = "behind";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_AHEAD:
        command = "ahead";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_SETTING:
        command = "setting";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_LRCCOLOR:
        command = "lrccolor";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_MENU:
        command = "menu";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_LRCWRONG_V:
        command = "lrcwrong_v";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_HORIZONTAL:
        command = "horizontal";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_MAKELRC_V:
        command = "makelrc_v";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_LOCK:
        command = "lock";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_UNLOCK:
        command = "unlock";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_SHOW:
        command = "show";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_CLOSE:
        command = "close";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_NEXT:
        command = "next";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_PREV:
        command = "prev";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_PLAY:
        command = "play";
        break;
    case LYRIC_DESKTOP_BUTTON_ID_PAUSE:
        command = "pause";
        break;
    default:
        return 0;
    }


    // 优先判断窗口消息, 如果没有再处理ws消息
    if (IsWindow(g_hWndReceive))
    {
        // 给窗口通知消息
        UINT msg = g_nReceiveMsg ? g_nReceiveMsg : WM_USER + 1;
        int r = (int)SendMessageW(g_hWndReceive, msg, id, 0);
        return r;
    }

    if (!m_notify_click)
        return 0;

    cJSON* json = cJSON_CreateObject();
    cJSON_AddStringToObject(json, "method", "button-click");
    cJSON* data = cJSON_AddObjectToObject(json, "params");
    cJSON_AddNumberToObject(data, "id", id);
    cJSON_AddStringToObject(data, "id_str", command);
    char* str = cJSON_PrintUnformatted(json);

    std::string str_msg = str;
    cJSON_Delete(json);
    cJSON_free(str);

    // 把消息发送出去, 这里不是同步的, 所以不管返回值, 默认都是允许
    if (!lrc_exe_ws_client_send(str_msg))
        lrc_exe_ws_server_send(str_msg);

    return 0;
}

