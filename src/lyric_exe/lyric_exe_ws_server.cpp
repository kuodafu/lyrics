#include "lyric_exe.h"
#include "ixwebsocket/IXWebSocketServer.h"
#include <charset_stl.h>


#ifdef _WIN64
#   ifdef _DEBUG
#       pragma comment(lib, "lib/x64/ixwebsocketD.lib")
#   else
#       pragma comment(lib, "lib/x64/ixwebsocket.lib")
#   endif
#else
#   ifdef _DEBUG
#       pragma comment(lib, "lib/x86/ixwebsocketD.lib")
#   else
#       pragma comment(lib, "lib/x86/ixwebsocket.lib")
#   endif
#endif


static PFN_ON_MESSAGE_RECEIVED s_on_message_received;
static std::shared_ptr<ix::WebSocketServer> ws_srv;
static bool s_ws_start;

// 保存所有活跃连接
static std::set<std::shared_ptr<ix::WebSocket>> g_clients;
static std::mutex g_clients_mutex;


bool lrc_exe_ws_server_start(const std::wstring& ip_or_port);


bool lrc_exe_ws_server(const std::wstring& ip_or_port, PFN_ON_MESSAGE_RECEIVED on_message_received)
{
    if (!on_message_received)
        return false;
    s_on_message_received = on_message_received;
    // 这里启动ws服务端
    return lrc_exe_ws_server_start(ip_or_port);
}

bool lrc_exe_ws_server_close()
{
    // 关闭服务器
    ws_srv->stop();
    ws_srv.reset();
    DestroyWindow(g_hWnd);  // 销毁消息窗口, 退出消息循环
    return true;
}

// 广播函数
static void broadcast_message(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(g_clients_mutex);
    for (auto& client : g_clients)
    {
        if (client->getReadyState() == ix::ReadyState::Open)
        {
            client->send(msg);
        }
    }
}

bool lrc_exe_ws_server_send(const std::string& text)
{
    if (!s_ws_start)
        return false;
    broadcast_message(text);
    return true;
}

bool lrc_exe_ws_server_start(const std::wstring& ip_or_port)
{
    // 创建 WebSocket 服务端对象
    int port = 0;
    std::string ip_a;
    // 是w 开头, 那就是ws, 就是一个地址
    if (ip_or_port.front() == L'w' || ip_or_port.front() == L'W')
    {
        size_t pos = ip_or_port.rfind(L':');
        if (pos == std::string::npos)
            return false;

        LPCWSTR pPort = ip_or_port.c_str() + pos + 1;
        if (*pPort == L'/')
            return false;   // 冒号后面是斜杠, 那就是前面的 ws://xxx 这里, 处理失败

        int port = _wtoi(pPort);
        ip_a = charset_stl::W2A(ip_or_port.c_str(), pos);
    }
    else
    {
        // 走到这里就是只传递了端口, 记录端口
        port = _wtoi(ip_or_port.c_str());
    }

    if (ip_a.empty())
        ws_srv = std::make_shared<ix::WebSocketServer>(port);
    else
        ws_srv = std::make_shared<ix::WebSocketServer>(port, ip_a);

    
    // 注册连接回调
    ws_srv->setOnConnectionCallback([](std::weak_ptr<ix::WebSocket> webSocket, std::shared_ptr<ix::ConnectionState> connectionState)
    {
        auto ws = webSocket.lock();
        if (!ws)
            return;

        // 添加到连接集合
        {
            std::lock_guard<std::mutex> lock(g_clients_mutex);
            g_clients.insert(ws);
        }

        // 设置消息回调
        ws->setOnMessageCallback([webSocket](const ix::WebSocketMessagePtr& msg)
        {
            auto ws = webSocket.lock();
            if (msg->type == ix::WebSocketMessageType::Message)
            {
                s_on_message_received(msg->str.c_str(), msg->str.size(), msg->binary, lrc_exe_ws_server_send);
            }
            else if (msg->type == ix::WebSocketMessageType::Open)
            {

            }
            else if (msg->type == ix::WebSocketMessageType::Close)
            {
                std::lock_guard<std::mutex> lock(g_clients_mutex);
                g_clients.erase(ws);
            }
        });
    });

    // 启动服务器
    char dbg[260];
    auto res = ws_srv->listen();
    if (!res.first)
    {
        sprintf_s(dbg, 260, "ws服务器启动失败, 错误信息: %s\n", res.second.c_str());
        OutputDebugStringA(dbg);
        return false;
    }
    // 开始服务
    ws_srv->start();
    s_ws_start = true;
    return true;
}
