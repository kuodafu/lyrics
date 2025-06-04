#include "lyric_typedef.h"


LYRIC_NAMESPACE_BEGIN
void __lrc_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPWSTR pEnd);
bool is_valid_utf8(const char* data, size_t len);

bool _lrc_parse_lrc(PINSIDE_LYRIC_INFO pLyric)
{
    size_t nSize = wcslen(pLyric->krc);
    LPWSTR pStart = &pLyric->krc[0];
    LPWSTR pEnd = pStart + nSize;
    LPCWSTR language = nullptr;

    // 把pStart指向下一行数据, 遇到换行的时候会把换行改成\0
    auto pfn_warp = [&pStart, &pEnd](size_t offset)
        {
            pStart += offset;
            auto pRet = pStart;
            while (pStart < pEnd)
            {
                wchar_t& ch = *pStart;
                if (ch == L'\r' || ch == L'\n' || ch == L']' || ch == L':')
                    ch = 0;
                else if (ch == L'[')
                    break;
                pStart++;
            }
            return pRet;
        };


    while (pStart < pEnd)
    {
        wchar_t& ch = *pStart++;
        if (ch == L'[')
        {
#define _cmp(_s) (_wcsnicmp(pStart, _s L":", (cmp_size = (ARRAYSIZE(_s)))) == 0)

            // 每一行的开始都是[, 遇到左方括号就是数据行开始, 解析这一行
            size_t cmp_size = 0;
            if (_cmp(L"id"))            pLyric->id = pfn_warp(cmp_size);
            else if (_cmp(L"ar"))       pLyric->ar = pfn_warp(cmp_size);
            else if (_cmp(L"ti"))       pLyric->ti = pfn_warp(cmp_size);
            else if (_cmp(L"by"))       pLyric->by = pfn_warp(cmp_size);
            else if (_cmp(L"hash"))     pLyric->hash = pfn_warp(cmp_size);
            else if (_cmp(L"al"))       pLyric->al = pfn_warp(cmp_size);
            else if (_cmp(L"sign"))     pLyric->sign = pfn_warp(cmp_size);
            else if (_cmp(L"qq"))       pLyric->qq = pfn_warp(cmp_size);
            else if (_cmp(L"total"))    pLyric->total = pfn_warp(cmp_size);
            else if (_cmp(L"offset"))   pLyric->offset = pfn_warp(cmp_size);

            else if (isdigit(*pStart))
            {
                // 遇到数字, 说明是歌词行, 另外解析歌词行
                __lrc_parse_text(pLyric, pStart - 1, pEnd);
                return true;  // 直接返回了, 解析歌词文本的时候会一直解析到结尾
            }
#undef _cmp
        }
    }

    return false;
}

bool _lrc_decrypt_lrc(const void* pData, size_t nSize, wchar_t** ppLyricText)
{
    auto pText = reinterpret_cast<LPCSTR>(pData);
    if (is_valid_utf8(pText, nSize))
        *ppLyricText = charset_stl::Utf8ToUnicode(pText, nSize);
    else
        *ppLyricText = charset_stl::AnsiToUnicode(pText, nSize);
    return true;
}

void __lrc_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPWSTR pEnd)
{
    // 把pStart指向下一行数据, 遇到换行的时候会把换行改成\0
    auto pfn_warp = [&pStart, &pEnd](int& len)
        {
            auto pRet = pStart;
            len = 0;
            while (pStart < pEnd)
            {
                wchar_t& ch = *pStart++;
                if (ch == L'\r' || ch == L'\n')
                {
                    ch = 0;
                    while (*pStart == L'\r' || *pStart == L'\n')
                        ++pStart;
                    break;
                }
                len++;
            }
            return pRet;
        };


    while (pStart < pEnd)
    {
        // 是这种格式, 全部按这个格式解析, 然后保存到数组
        // [01:23.45]歌词内容\r\n

        INSIDE_LYRIC_LINE& lines = pLyric->lines.emplace_back();
        int m = 0, s = 0, ms = 0;
        int fmt_count = swscanf_s(pStart, L"[%d:%d.%d]", &m, &s, &ms);
        while (pStart < pEnd && *pStart != L']')
            pStart++;
        pStart++;
        LPCWSTR pTextWord = pStart;
        lines.start = m * 60 * 1000 + s * 1000 + ms * 10;
        lines.duration = 0;
        lines.interval = MAXINT;

        LPCWSTR pText = pfn_warp(lines.size);
        lines.text.assign(pText, lines.size);

        // 加入一个字, 不然现有的代码会出错
        INSIDE_LYRIC_WORD& words = lines.words.emplace_back();
        words.start = 0;
        words.duration = 0;
        words.text = pTextWord;
        words.size = (int)lines.size;

        size_t lines_size = pLyric->lines.size();
        if (lines_size > 1)
        {
            INSIDE_LYRIC_LINE& back = pLyric->lines[lines_size - 2];
            back.duration = lines.start - back.start - 300;
            back.interval = 0;
            back.words.front().duration = back.duration;
        }

    }

}

bool is_valid_utf8(const char* data, size_t len)
{
    size_t i = 0;
    while (i < len)
    {
        unsigned char c = data[i];
        if (c <= 0x7F)
        {
            // ASCII
            i += 1;
        }
        else if ((c >> 5) == 0x6 && i + 1 < len)
        {
            // 2-byte UTF-8
            if ((data[i + 1] & 0xC0) != 0x80)
                return false;
            i += 2;
        }
        else if ((c >> 4) == 0xE && i + 2 < len)
        {
            // 3-byte UTF-8
            if ((data[i + 1] & 0xC0) != 0x80 || (data[i + 2] & 0xC0) != 0x80)
                return false;
            i += 3;
        }
        else if ((c >> 3) == 0x1E && i + 3 < len)
        {
            // 4-byte UTF-8
            if ((data[i + 1] & 0xC0) != 0x80 ||
                (data[i + 2] & 0xC0) != 0x80 ||
                (data[i + 3] & 0xC0) != 0x80)
                return false;
            i += 4;
        }
        else
        {
            return false;
        }
    }
    return true;
}

LYRIC_NAMESPACE_END
