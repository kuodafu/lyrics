/*
* 这个文件主要是连接指定的ws服务器, 然后等待接收消息
*/
#include "lyric_exe.h"
#include "ixwebsocket/IXWebSocketServer.h"
#include <charset_stl.h>

static PFN_ON_MESSAGE_RECEIVED s_on_message_received;
static std::string s_server_ip;
static bool s_ws_connected = false;
static ix::WebSocket s_ws;

// 连接ws服务器, 初始化的时候记录服务器地址, 然后记录起来, 后续都是连接这个地址
bool lrc_exe_ws_client_connect();

// 创建ws客户端, 然后连接到指定的服务器, 有消息会通过回调函数来处理, 断开后会自动重连
bool lrc_exe_ws_client(const std::wstring& server_ip, PFN_ON_MESSAGE_RECEIVED on_message_received)
{
    if (!on_message_received)
        return false;

    s_server_ip = charset_stl::W2A(server_ip);
    s_on_message_received = on_message_received;

    return lrc_exe_ws_client_connect();
}
void lrc_exe_ws_client_try_connect(HWND hWnd)
{
    if (s_server_ip.empty())
        return;

    SetTimer(hWnd, 300, 10000, [](HWND hWnd, UINT message, UINT_PTR id, DWORD t)
             {
                 if (s_ws_connected)
                     return;
                 lrc_exe_ws_client_connect();
             });
    return;
}
// 关闭ws客户端
bool lrc_exe_ws_client_close()
{
    s_ws.close();
    return true;
}

bool lrc_exe_ws_client_send(const std::string& text)
{
    if (!s_ws_connected)
        return false;
    auto s = s_ws.sendText(text);
    return s.success;
}

bool lrc_exe_ws_client_connect()
{
    s_ws.setUrl(s_server_ip);

    // 设置回调
    s_ws.setOnMessageCallback([](const ix::WebSocketMessagePtr& msg)
    {
        if (msg->type == ix::WebSocketMessageType::Open)
        {
            s_ws_connected = true;
        }
        else if (msg->type == ix::WebSocketMessageType::Message)
        {
            s_on_message_received(msg->str.c_str(), msg->str.size(), msg->binary, lrc_exe_ws_server_send);
        }
        else if (msg->type == ix::WebSocketMessageType::Close)
        {
            std::wstring w = charset_stl::U2W(msg->closeInfo.reason);
            wchar_t buffer[1024];
            swprintf_s(buffer, L"[close] Connection closed. Code: %d, Reason: %s\n", msg->closeInfo.code, w.c_str());
            OutputDebugStringW(buffer);
            s_ws_connected = false;
        }
        else if (msg->type == ix::WebSocketMessageType::Error)
        {
            std::wstring w = charset_stl::U2W(msg->errorInfo.reason);
            OutputDebugStringW(L"[error] ");
            OutputDebugStringW(w.c_str());
            OutputDebugStringW(L"\n");
            //__debugbreak();
        }

    });

    // 启动连接
    s_ws.start();
    return false;
}
