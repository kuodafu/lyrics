#include "hp_ws.h"
#include "HPSocket/HPSocket4C.h"
#include <exception>

#define _USE_4C 1

#ifdef _WIN64
#   ifdef _DEBUG
#       ifdef _USE_4C
#           ifdef UNICODE
#               pragma comment(lib, "hp/x64/HPSocket4C_UD.lib")
#           else
#               pragma comment(lib, "hp/x64/HPSocket4C_D.lib")
#           endif
#       else
#           ifdef UNICODE
#               pragma comment(lib, "hp/x64/HPSocket_UD.lib")
#           else
#               pragma comment(lib, "hp/x64/HPSocket_D.lib")
#           endif
#       endif
#   else
#       ifdef _USE_4C
#           ifdef UNICODE
#               pragma comment(lib, "hp/x64/HPSocket4C_U.lib")
#           else
#               pragma comment(lib, "hp/x64/HPSocket4C.lib")
#           endif
#       else
#           ifdef UNICODE
#               pragma comment(lib, "hp/x64/HPSocket_U.lib")
#           else
#               pragma comment(lib, "hp/x64/HPSocket.lib")
#           endif
#       endif
#   endif
#else
#   ifdef _DEBUG
#       ifdef _USE_4C
#           ifdef UNICODE
#               pragma comment(lib, "hp/x86/HPSocket4C_UD.lib")
#           else
#               pragma comment(lib, "hp/x86/HPSocket4C_D.lib")
#           endif
#       else
#           ifdef UNICODE
#               pragma comment(lib, "hp/x86/HPSocket_UD.lib")
#           else
#               pragma comment(lib, "hp/x86/HPSocket_D.lib")
#           endif
#       endif
#   else
#       ifdef _USE_4C
#           ifdef UNICODE
#               pragma comment(lib, "hp/x86/HPSocket4C_U.lib")
#           else
#               pragma comment(lib, "hp/x86/HPSocket4C.lib")
#           endif
#       else
#           ifdef UNICODE
#               pragma comment(lib, "hp/x86/HPSocket_U.lib")
#           else
#               pragma comment(lib, "hp/x86/HPSocket.lib")
#           endif
#       endif
#   endif
#endif

typedef struct WS_STRUCT
{
    bool                    m_bWebSocket;           // 是否是ws协议
    bool                    m_isServer;             // 是否是服务端
    HANDLE                  hEvent;                 // 升级到ws协议后触发, 没升级就是连接失败
    HP_HttpClientListener   m_HttpClientListener;   
    HP_HttpClient           m_HttpClient;
    WS_STRUCT() : m_bWebSocket(false), m_isServer(false), hEvent(0), m_HttpClientListener(0), m_HttpClient(0)
    {
        hEvent = CreateEventW(0, 0, 0, 0);
        if (!hEvent)
            throw std::exception("CreateEventW 创建失败");
        m_HttpClientListener = ::Create_HP_HttpClientListener();
        if (!m_HttpClientListener)
            throw std::exception("HttpClientListener 创建失败");

        // 设置 HTTP 监听器回调函数
        ::HP_Set_FN_HttpClient_OnConnect            (m_HttpClientListener, OnConnect);
        ::HP_Set_FN_HttpClient_OnHandShake          (m_HttpClientListener, OnHandShake);
        ::HP_Set_FN_HttpClient_OnSend               (m_HttpClientListener, OnSend);
        ::HP_Set_FN_HttpClient_OnReceive            (m_HttpClientListener, OnReceive);
        ::HP_Set_FN_HttpClient_OnClose              (m_HttpClientListener, OnClose);

        ::HP_Set_FN_HttpClient_OnMessageBegin       (m_HttpClientListener, OnMessageBegin);
        ::HP_Set_FN_HttpClient_OnStatusLine         (m_HttpClientListener, OnStatusLine);
        ::HP_Set_FN_HttpClient_OnHeader             (m_HttpClientListener, OnHeader);
        ::HP_Set_FN_HttpClient_OnHeadersComplete    (m_HttpClientListener, OnHeadersComplete);
        ::HP_Set_FN_HttpClient_OnBody               (m_HttpClientListener, OnBody);
        ::HP_Set_FN_HttpClient_OnChunkHeader        (m_HttpClientListener, OnChunkHeader);
        ::HP_Set_FN_HttpClient_OnChunkComplete      (m_HttpClientListener, OnChunkComplete);
        ::HP_Set_FN_HttpClient_OnMessageComplete    (m_HttpClientListener, OnMessageComplete);
        ::HP_Set_FN_HttpClient_OnUpgrade            (m_HttpClientListener, OnUpgrade);
        ::HP_Set_FN_HttpClient_OnParseError         (m_HttpClientListener, OnParseError);

        ::HP_Set_FN_HttpClient_OnWSMessageHeader    (m_HttpClientListener, OnWSMessageHeader);
        ::HP_Set_FN_HttpClient_OnWSMessageBody      (m_HttpClientListener, OnWSMessageBody);
        ::HP_Set_FN_HttpClient_OnWSMessageComplete  (m_HttpClientListener, OnWSMessageComplete);

        m_HttpClient = ::Create_HP_HttpClient(m_HttpClientListener);

        if (!m_HttpClient)
            throw std::exception("HttpClient 创建失败");

        ::HP_HttpClient_SetUseCookie(m_HttpClient, TRUE);

    }
    ~WS_STRUCT()
    {
        if (hEvent)
            CloseHandle(hEvent);

        HP_Client_Stop(m_HttpClient);

        Destroy_HP_TcpClientListener(m_HttpClientListener);
        Destroy_HP_TcpClient(m_HttpClient);

        hEvent = 0;
        m_HttpClient = 0;
        m_HttpClientListener = 0;
    }


    static EnHandleResult CALLBACK OnConnect(HP_Client pSender, CONNID dwConnID)
    {
        TCHAR szAddress[100];
        int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
        USHORT usPort;

        ::HP_Client_GetLocalAddress(pSender, szAddress, &iAddressLen, &usPort);

        return HR_OK;
    }

    static EnHandleResult CALLBACK OnHandShake(HP_Client pSender, CONNID dwConnID)
    {
        // 在这里升级ws协议
        HP_THeader header[] =
        {
            { "Upgrade"                 , "websocket"                                   },
            { "Connection"              , "Upgrade"                                     },
            { "Sec-WebSocket-Key"       , "x3JJHMbDL1EzLkh9GBhXDw=="                    },
            { "Sec-WebSocket-Version"   , "13"                                          },
            { "Sec-WebSocket-Protocol"  , "json"                                        },
            { "Sec-WebSocket-Extensions", "permessage-deflate; client_max_window_bits"  },
        };

        HP_HttpClient_SendRequest(pSender, "GET", "/", header, ARRAYSIZE(header), 0, 0);
        return HR_OK;
    }

    static EnHandleResult CALLBACK OnSend(HP_Client pSender, CONNID dwConnID, const BYTE* pData, int iLength)
    {
        return HR_OK;
    }

    static EnHandleResult CALLBACK OnReceive(HP_Client pSender, CONNID dwConnID, const BYTE* pData, int iLength)
    {
        return HR_OK;
    }

    static EnHandleResult CALLBACK OnClose(HP_Client pSender, CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
    {
        PHTTP_STRUCT pObj = (PHTTP_STRUCT)HP_Client_GetExtra(pSender);
        if (pObj)
        {
            pObj->m_bWebSocket = false;
            delete pObj;
        }
        return HR_OK;
    }

    // ------------------------------------------------------------------------------------------------------------- //

    static EnHttpParseResult CALLBACK OnMessageBegin(HP_HttpClient pSender, CONNID dwConnID)
    {
        return HPR_OK;
    }

    static EnHttpParseResult CALLBACK OnStatusLine(HP_HttpClient pSender, CONNID dwConnID, USHORT usStatusCode, LPCSTR lpszDesc)
    {
        return HPR_OK;
    }

    static EnHttpParseResult CALLBACK OnHeader(HP_HttpClient pSender, CONNID dwConnID, LPCSTR lpszName, LPCSTR lpszValue)
    {
        return HPR_OK;
    }

    static EnHttpParseResult CALLBACK OnHeadersComplete(HP_HttpClient pSender, CONNID dwConnID)
    {
        return HPR_OK;
    }

    static EnHttpParseResult CALLBACK OnBody(HP_HttpClient pSender, CONNID dwConnID, const BYTE* pData, int iLength)
    {
        PHTTP_STRUCT pObj = (PHTTP_STRUCT)HP_Client_GetExtra(pSender);
        if (pObj)
        {

        }
        return HPR_OK;
    }

    static EnHttpParseResult CALLBACK OnChunkHeader(HP_HttpClient pSender, CONNID dwConnID, int iLength)
    {
        return HPR_OK;
    }

    static EnHttpParseResult CALLBACK OnChunkComplete(HP_HttpClient pSender, CONNID dwConnID)
    {

        return HPR_OK;
    }

    static EnHttpParseResult CALLBACK OnMessageComplete(HP_HttpClient pSender, CONNID dwConnID)
    {
        PHTTP_STRUCT pObj = (PHTTP_STRUCT)HP_Client_GetExtra(pSender);
        if (pObj)
        {

        }
        return HPR_OK;
    }

    static EnHttpParseResult CALLBACK OnUpgrade(HP_HttpClient pSender, CONNID dwConnID, EnHttpUpgradeType enUpgradeType)
    {
        PHTTP_STRUCT pObj = (PHTTP_STRUCT)HP_Client_GetExtra(pSender);
        if (pObj && enUpgradeType == HUT_WEB_SOCKET)
        {
            pObj->m_bWebSocket = true;
            SetEvent(pObj->hEvent);
        }

        return HPR_OK;
    }

    static EnHttpParseResult CALLBACK OnParseError(HP_HttpClient pSender, CONNID dwConnID, int iErrorCode, LPCSTR lpszErrorDesc)
    {
        return HPR_OK;
    }

    // ------------------------------------------------------------------------------------------------------------- //

    static En_HP_HandleResult CALLBACK OnWSMessageHeader(HP_HttpClient pSender, CONNID dwConnID, BOOL bFinal, BYTE iReserved, BYTE iOperationCode, const BYTE lpszMask[4], ULONGLONG ullBodyLen)
    {
        return HR_OK;
    }

    static En_HP_HandleResult CALLBACK OnWSMessageBody(HP_HttpClient pSender, CONNID dwConnID, const BYTE* pData, int iLength)
    {
        PHTTP_STRUCT pObj = (PHTTP_STRUCT)HP_Client_GetExtra(pSender);
        if (pObj)
        {

        }
        return HR_OK;
    }

    static En_HP_HandleResult CALLBACK OnWSMessageComplete(HP_HttpClient pSender, CONNID dwConnID)
    {
        //TODO 接收到了所有数据
        BYTE iOperationCode;
        BOOL isOk = ::HP_HttpClient_GetWSMessageState(pSender, nullptr, nullptr, &iOperationCode, nullptr, nullptr, nullptr);

        if (isOk && iOperationCode == 0x8)
            return HR_ERROR;

        PHTTP_STRUCT pObj = (PHTTP_STRUCT)HP_Client_GetExtra(pSender);
        if (pObj)
        {

        }

        return HR_OK;
    }

}*PHTTP_STRUCT;




HP_HANDLE ws_server_start()
{
    try
    {
        PHTTP_STRUCT pObj = new WS_STRUCT;
        pObj->m_isServer = true;

        // 创建 HTTP Server
        HP_HttpServerListener listener = pObj->m_HttpClientListener;
        HP_HttpServer server = ::Create_HP_HttpServer(listener);
        if (!server)
            throw std::exception("HttpServer 创建失败");

        // 设置回调（共享 listener）
        ::HP_Set_FN_HttpServer_OnUpgrade(listener, [](HP_HttpServer pSender, CONNID dwConnID, EnHttpUpgradeType enUpgradeType) -> EnHttpParseResult
        {
            if (enUpgradeType == HUT_WEB_SOCKET)
            {
                PHTTP_STRUCT pObj = (PHTTP_STRUCT)::HP_Server_GetExtra(pSender, dwConnID);
                if (pObj)
                {
                    pObj->m_bWebSocket = true;
                    SetEvent(pObj->hEvent);
                }
            }
            return HPR_OK;
        });

        ::HP_Server_SetExtra(server, 0, pObj);  // 绑定额外数据
        ::HP_HttpServer_SetUseCookie(server, TRUE);

        // 绑定监听地址
        if (!::HP_Server_Start(server, pszBindAddress, port))
        {
            delete pObj;
            return 0;
        }

        // 启动成功
        return (HP_HANDLE)pObj;
    }
    catch (std::exception& e)
    {
        (e);
    }

    return 0;
}

bool ws_send_server(HP_HANDLE hws, CONNID connID, const void* ptr, int size)
{
    PHTTP_STRUCT pObj = (PHTTP_STRUCT)hws;
    BYTE mask[] = { 0, 0, 0, 0 }; // 服务端不需要 mask
    return HP_HttpServer_SendWSMessage((HP_HttpServer)pObj->m_HttpClient, connID, TRUE, 0, 2, (LPBYTE)ptr, size, 0);
}

HP_HANDLE ws_client_connect(const wchar_t* const pszAddress, unsigned short usPort)
{
    // 创建监听器对象
    try
    {
        PHTTP_STRUCT pObj = new WS_STRUCT;
        pObj->m_isServer = false;
        HP_Client_SetExtra(pObj->m_HttpClient, pObj);
        if (!::HP_Client_Start(pObj->m_HttpClient, pszAddress, usPort, false))
        {
            delete pObj;
            return 0;
        }
        DWORD ret = WaitForSingleObject(pObj->hEvent, 5000);
        if (ret == WAIT_OBJECT_0)
            return (HP_HANDLE)pObj;
        delete pObj;
        return 0;
    }
    catch (std::exception& e)
    {
        (e);
    }

    return 0;
}

bool ws_send(HP_HANDLE hws, const void* ptr, int size)
{
    PHTTP_STRUCT pObj = (PHTTP_STRUCT)hws;
    BYTE mask[] = { 1, 2, 3, 4 };
    return HP_HttpClient_SendWSMessage(pObj->m_HttpClient, true, 0, 2, mask, (LPBYTE)ptr, size, 0);
}
bool ws_send(HP_HANDLE hws, const char* str)
{
    if (!str)
        return false;
    PHTTP_STRUCT pObj = (PHTTP_STRUCT)hws;
    BYTE mask[] = { 1, 2, 3, 4 };
    int size = (int)strlen(str);
    return HP_HttpClient_SendWSMessage(pObj->m_HttpClient, true, 0, 1, mask, (LPBYTE)str, size, 0);
}


