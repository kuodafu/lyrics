#pragma once
#include <windows.h>
#include <mutex>
#include <vector>
#include <string>
#include "lyric_desktop_namespace.h"

NAMESPACE_LYRIC_DESKTOP_BEGIN

class CCriticalSection
{
public:
    CCriticalSection(LPCRITICAL_SECTION cs) : m_cs(cs)
    {
        Lock();
    }
    CCriticalSection(LPCRITICAL_SECTION cs, const std::try_to_lock_t&) : m_cs(cs)
    {
        Lock();
    }
    CCriticalSection(LPCRITICAL_SECTION cs, const std::adopt_lock_t&) : m_cs(cs)
    {
    }
    ~CCriticalSection()
    {
        Unlock();
    }
    bool TryLock()
    {
        is_unlock = TryEnterCriticalSection(m_cs);
        return is_unlock;
    }
    void Lock()
    {
        EnterCriticalSection(m_cs);
        is_unlock = true;
    }
    void Unlock()
    {
        if (is_unlock)
            LeaveCriticalSection(m_cs);
        m_cs = nullptr;
    }
private:
    LPCRITICAL_SECTION m_cs{};
    bool is_unlock{};
};

enum class LYRIC_MODE : unsigned int
{
    DOUBLE_ROW      = 0x0000,   // 双行歌词
    TRANSLATION1    = 0x0001,   // 翻译
    TRANSLATION2    = 0x0002,   // 音译
    SINGLE_ROW      = 0x0004,   // 单行显示

    VERTICAL        = 0x10000,  // 竖屏模式
    EXISTTRANS      = 0x20000,  // 存在翻译, 存在这个就判断歌词对齐模式, 否则默认双行

};


NAMESPACE_LYRIC_DESKTOP_END

