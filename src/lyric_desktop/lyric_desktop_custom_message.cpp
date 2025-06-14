#include "lyric_desktop_function.h"

NAMESPACE_LYRIC_DESKTOP_BEGIN

class LyricMapFile
{
    HANDLE hMapFile;
    LPVOID pData;
public:
    LyricMapFile(LPCWSTR pszMapName) : hMapFile(nullptr), pData(nullptr)
    {
        hMapFile = OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, pszMapName);
        if (!hMapFile)
            return;
    }
    ~LyricMapFile()
    {
        if (pData)
            UnmapViewOfFile(pData);
        if (hMapFile)
            CloseHandle(hMapFile);
    }
    // 返回的指针没有写的权限
    const void* read()
    {
        if (!hMapFile)
            return nullptr;
        if (pData)
            return pData;
        pData = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
        return pData;
    }
    bool write(const void* pBuf, size_t nSize) const
    {
        PVOID ptr = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0);
        if (!ptr)
            return false;
        memcpy(ptr, pBuf, nSize);
        UnmapViewOfFile(ptr);
        return true;
    }
};


bool lyric_wnd_proc_custom_message(PLYRIC_DESKTOP_INFO pWndInfo, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& ret)
{
    return false;
    switch (message)
    {
    case LD_LOADLYRIC:
    {
        LyricMapFile map(MAPFILE_NAME_LOADLYRIC);
        LPCVOID pData = map.read();
        if (!pData)
            break;
        auto nSize = static_cast<int>(wParam);
        auto nType = static_cast<LYRIC_PARSE_TYPE>(lParam);
        ret = _lyric_desktop_load_lyric(pWndInfo, pData, nSize, nType);
        break;
    }
    case LD_UPDATE:
    {
        pWndInfo->nCurrentTimeMS = static_cast<int>(wParam);
        ret = 1;
        break;
    }
    case LD_GETCONFIG:
    {
        LyricMapFile map(MAPFILE_NAME_GETCONFIG);
        LPCVOID pData = map.read();
        if (!pData)
            break;
        char* pConfig = pWndInfo->config.to_json(pWndInfo);
        if (!pConfig)
            break;
        size_t nSize = strlen(pConfig);
        ret = static_cast<LRESULT>(nSize);
        map.write(pConfig, nSize + 1);
        free(pConfig);
        break;
    }
    case LD_SETCONFIG:
    {
        LyricMapFile map(MAPFILE_NAME_SETCONFIG);
        LPCVOID pData = map.read();
        if (!pData)
            break;
        auto pConfig = reinterpret_cast<LPCSTR>(map.read());
        ret = pWndInfo->config.parse(pConfig, pWndInfo);
        break;
    }
    case LD_CALLEVENT:
        ret = lyric_wnd_call_evt(*pWndInfo, static_cast<int>(wParam));
        break;
    case LD_SETBUTTONSTATE:
        ret = lyric_wnd_set_btn_state(*pWndInfo, static_cast<int>(wParam), static_cast<LYRIC_DESKTOP_BUTTON_STATE>(lParam));
        break;
    case LD_GETBUTTONSTATE:
        ret = lyric_wnd_get_btn_state(*pWndInfo, static_cast<int>(wParam));
        break;
    default:
        return false;
    }
    return true;
}

NAMESPACE_LYRIC_DESKTOP_END




