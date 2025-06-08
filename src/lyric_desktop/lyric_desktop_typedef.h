#pragma once
#include <windows.h>
#include <mutex>
#include <vector>
#include <string>
#include "lyric_desktop_namespace.h"
#include <kuodafu_lyric_desktop.h>

NAMESPACE_LYRIC_DESKTOP_BEGIN

struct LYRIC_DESKTOP_INFO;


// ��ʴ��ڵ�ģʽ, Ŀǰ���⼸��ģʽ
enum class LYRIC_DESKTOP_MODE : unsigned int
{
    DOUBLE_ROW      = 0x0000,   // ˫�и��
    TRANSLATION_FY  = 0x0001,   // ����
    TRANSLATION_YY  = 0x0002,   // ����
    SINGLE_ROW      = 0x0004,   // ������ʾ

    VERTICAL        = 0x10000,  // ����ģʽ
    EXISTTRANS      = 0x20000,  // ���ڷ���, ����������жϸ�ʶ���ģʽ, ����Ĭ��˫��

};

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



NAMESPACE_LYRIC_DESKTOP_END

