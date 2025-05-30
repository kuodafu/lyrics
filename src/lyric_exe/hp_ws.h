#pragma once
#ifndef HP_WS_H
#define HP_WS_H
#include <string>

typedef struct HPWS__ { int unused; } *HP_HANDLE;

HP_HANDLE ws_server_start();

// 连接ws服务器
HP_HANDLE ws_client_connect(const wchar_t* const pszAddress, unsigned short usPort);
bool ws_send(HP_HANDLE hws, const void* ptr, int size);
bool ws_send(HP_HANDLE hws, const char* str);


#endif