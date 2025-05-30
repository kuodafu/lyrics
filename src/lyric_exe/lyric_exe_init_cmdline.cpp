#include "lyric_exe.h"
#include <shellapi.h>

#pragma comment(lib, "shell32.lib")

// 获取命令行参数, 解析后记录到map里
bool lrc_exe_get_cmdline(std::unordered_map<std::wstring, std::wstring>& cmdLine);


bool lrc_exe_init_cmdline()
{
    std::unordered_map<std::wstring, std::wstring> cmdLine;
    auto pfn_err = []()
    {
        std::wstring text(1024, 0);
        text.assign(L"命令行参数错误, 无法继续执行\r\n");
        text.append(L"当前命令行: ").append(GetCommandLineW()).append(L"\r\n");
        text.append(L"请使用 --ws_client=服务器地址\r\n");
        text.append(L"或者用 --ws_server=\"监听地址 或者 端口\"\r\n");
        text.append(L"或者用 --hwnd=窗口句柄\r\n");
        text.append(L"程序必须有一个端能跟本进程通讯\r\n");
        text.append(L"更详细的信息请查看SDK\r\n");
        MessageBoxW(0, text.c_str(), L"命令行参数错误", MB_ICONERROR);
        return false;
    };
    if (!lrc_exe_get_cmdline(cmdLine))
        return pfn_err();   // 没有正确的命令行参数, 返回

    bool bRet = false;

#define _cmp(_s) (_wcsicmp(pName, (_s)) == 0)
    for (auto& it : cmdLine)
    {
        const std::wstring& name = it.first;
        const std::wstring& value = it.second;
        LPCWSTR pName = name.c_str();
        LPCWSTR pValue = value.c_str();
        if (_cmp(L"ws_client"))
        {
            // 连接ws服务器, value就是服务器地址
            // 连接失败不管, 会创建个时钟一直尝试连接
            lrc_exe_ws_client(value, ws_OnMessageReceived);
            bRet = true;
        }
        else if (_cmp(L"ws_server"))
        {
            // 开启ws服务器, value就是服务器端口
            if (!lrc_exe_ws_server(value, ws_OnMessageReceived))
            {
                wchar_t szMsg[1024];
                swprintf_s(szMsg, L"开启ws服务器失败, 不能交互, 无法继续执行\r\nws服务器地址: %s", pValue);
                MessageBoxW(0, szMsg, L"开启ws服务器失败", MB_ICONERROR);
                return false;   // 服务端开启失败就不能往下执行了, 没法提供服务
            }
            bRet = true;
        }
        else if (_cmp(L"hwnd"))
        {
            // 通过窗口消息接收通知, 可以和ws共存
            if (pValue[0] == L'0' && (pValue[1] == L'x' || pValue[1] == L'X'))
                g_hWndReceive = (HWND)std::stoull(pValue + 2, nullptr, 16);
            else
                g_hWndReceive = (HWND)std::stoull(value, nullptr, 10);
            bRet = true;
        }

    }
#undef _cmp
    if (!bRet)
        return pfn_err();   // 没有正确的命令行参数, 返回
    return bRet;
}


bool lrc_exe_get_cmdline(std::unordered_map<std::wstring, std::wstring>& cmdLine)
{
    int nArgs = 0;
    LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
    for (int i = 1; i < nArgs; i++)
    {
        LPCWSTR pStart = szArglist[i];
        while (*pStart && *pStart == L'\"')
            pStart++;   // 跳过开始的引号

        if (pStart[0] == pStart[1] && pStart[0] == '-')
        {
            pStart += 2;
            if (!*pStart)
                continue;   // 空参数, 忽略

            // -- 后面跟着命令行名字, 名字后面是等号, 等号后面是值, 没有等号就是没有值

            LPCWSTR name = pStart;
            LPCWSTR nameEnd = name;
            LPCWSTR value = nullptr;
            while (*nameEnd && *nameEnd != '=')
                nameEnd++;

            value = nameEnd;
            while (*value && (*value == '=' || *value == '\"'))
                value++;    // 跳过等号和引号
            size_t value_size = wcslen(value);

            while (value[value_size - 1] == '\"')
                value_size--;   // 跳过最后的引号

            cmdLine.insert(std::make_pair(std::wstring(name, nameEnd - name), std::wstring(value, value_size)));

        }
    }

    LocalFree(szArglist);
    return !cmdLine.empty();
}


