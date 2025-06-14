#pragma once
// // ���� SDKDDKVer.h �ɶ�����õ���߰汾�� Windows ƽ̨��
// ���ϣ��Ϊ֮ǰ�� Windows ƽ̨����Ӧ�ó����ڰ��� SDKDDKVer.h ֮ǰ���Ȱ��� WinSDKVer.h ��
// �� _WIN32_WINNT ������Ϊ��Ҫ֧�ֵ�ƽ̨��
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // �� Windows ͷ�ļ����ų�����ʹ�õ�����
// Windows ͷ�ļ�
#include <windows.h>
// C ����ʱͷ�ļ�
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
extern HWND         g_hWnd;         // ��Ϣ����, ����һЩ��Ϣ, ���а�һЩ�ض��¼�ת�������߳�ִ��
extern HWND         g_hWndReceive;  // ��Ϣ֪ͨ����, ������Ч��ʱ������Ϣ��֪ͨ������
extern UINT         g_nReceiveMsg;  // ֪ͨ����Ϣֵ, û�еĻ���Ĭ��Ϊ WM_USER + 1
extern std::string  g_json_config;  // ������Ϣ����

using PFN_WS_SEND_MESSAGE = bool(*)(const std::string& text);

LRESULT CALLBACK MessageWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
using PFN_ON_MESSAGE_RECEIVED = void(*)(LPCSTR message, size_t len, bool binary, PFN_WS_SEND_MESSAGE pfnSend);

// ws����Ϣ�ص�����, �ͻ���/����˴���һ��, ���ǵ���Ϣ, �޷���˭��˭������
void ws_OnMessageReceived(LPCSTR message, size_t len, bool binary, PFN_WS_SEND_MESSAGE pfnSend);


// ��ʼ��������, ����������������������Щ����, û�д���, ȫ����ͨ������������
bool lrc_exe_init_cmdline();

bool lrc_exe_init_cmdline_ws();

// ����ws�ͻ���, Ȼ�����ӵ�ָ���ķ�����, ����Ϣ��ͨ���ص�����������, �Ͽ�����Զ�����
bool lrc_exe_ws_client(const std::wstring& server_ip, PFN_ON_MESSAGE_RECEIVED on_message_received);
// ����һ��ʱ�����������ӷ�����
void lrc_exe_ws_client_try_connect(HWND hWnd);
// �ر�ws�ͻ���
bool lrc_exe_ws_client_close();
bool lrc_exe_ws_client_send(const std::string& text);

// ����ws�����, �ȴ��ͻ�������, ����Ϣ��ͨ���ص�����������
bool lrc_exe_ws_server(const std::wstring& ip_or_port, PFN_ON_MESSAGE_RECEIVED on_message_received);
// �ر�ws�����
bool lrc_exe_ws_server_close();
bool lrc_exe_ws_server_send(const std::string& text);


