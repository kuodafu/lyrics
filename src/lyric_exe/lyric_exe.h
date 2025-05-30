#pragma once
// // 包含 SDKDDKVer.h 可定义可用的最高版本的 Windows 平台。
// 如果希望为之前的 Windows 平台构建应用程序，在包含 SDKDDKVer.h 之前请先包含 WinSDKVer.h 并
// 将 _WIN32_WINNT 宏设置为想要支持的平台。
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
#include <vector>
#include <unordered_map>
#include <functional>
#include <cJSON/cJSON.h>

extern HINSTANCE    g_hInst;
extern HWND         g_hWnd;         // 消息窗口, 处理一些消息, 还有把一些特定事件转到窗口线程执行
extern HWND         g_hWndReceive;  // 消息通知窗口, 窗口有效的时候有消息会通知到这里

LRESULT CALLBACK MessageWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
using PFN_ON_MESSAGE_RECEIVED = void(*)(LPCSTR message, size_t len, bool binary);

// ws的消息回调函数, 客户端/服务端处理都一样, 都是等消息, 无非是谁给谁发而已
void ws_OnMessageReceived(LPCSTR message, size_t len, bool binary);


// 初始化命令行, 根据命令行来决定开启哪些服务, 没有窗口, 全都是通过交互来控制
bool lrc_exe_init_cmdline();

// 创建ws客户端, 然后连接到指定的服务器, 有消息会通过回调函数来处理, 断开后会自动重连
bool lrc_exe_ws_client(const std::wstring& server_ip, PFN_ON_MESSAGE_RECEIVED on_message_received);
// 关闭ws客户端
bool lrc_exe_ws_client_close();
bool lrc_exe_ws_client_send(const std::string& text);

// 创建ws服务端, 等待客户端连接, 有消息会通过回调函数来处理
bool lrc_exe_ws_server(const std::wstring& ip_or_port, PFN_ON_MESSAGE_RECEIVED on_message_received);
// 关闭ws服务端
bool lrc_exe_ws_server_close();
bool lrc_exe_ws_server_send(const std::string& text);


