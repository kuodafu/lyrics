#include "lyric_typedef.h"
#include "crypt_de.h"

LYRIC_NAMESPACE_BEGIN
bool _qrc_parse_lyric(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPCWSTR pEnd);
bool __qrc_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPCWSTR pEnd);
LPCWSTR _qrc_get_name(LPWSTR& pStart, LPCWSTR pEnd);
LPCWSTR _qrc_get_value(LPWSTR& pStart, LPCWSTR pEnd);

bool _lrc_parse_qrc(PINSIDE_LYRIC_INFO pLyric)
{
    size_t nSize = wcslen(pLyric->krc);
    LPWSTR pStart = &pLyric->krc[0];
    LPWSTR pEnd = pStart + nSize;

    LPCWSTR SaveTime = nullptr;
    LPCWSTR Version = nullptr;
    LPCWSTR LyricCount = nullptr;
    LPCWSTR LyricType = nullptr;
    LPCWSTR LyricContent = nullptr;

    // 处理 QrcHeadInfo 标签的内容
    auto pfn_QrcHeadInfo = [&]()
        {
            LPCWSTR name = _qrc_get_name(pStart, pEnd);
            if (_wcsicmp(name, L"SaveTime") == 0)
            {
                SaveTime = _qrc_get_value(pStart, pEnd);
                name = _qrc_get_name(pStart, pEnd);
                if (_wcsicmp(name, L"Version") == 0)
                    Version = _qrc_get_value(pStart, pEnd);
            }
            pStart = wcschr(pStart + 1, L'<');
        };
    // 处理 LyricInfo 标签的内容
    auto pfn_LyricInfo = [&]()
        {
            LPCWSTR name = _qrc_get_name(pStart, pEnd);
            if (_wcsicmp(name, L"LyricCount") == 0)
                LyricCount = _qrc_get_value(pStart, pEnd);
            pStart = wcschr(pStart + 1, L'<');
        };

    while (pStart < pEnd)
    {
        wchar_t ch = *pStart++;
        if (ch == L'<')
        {
            // xml 开始标签, 找到下一个标签
            LPCWSTR name = _qrc_get_name(pStart, pEnd);
            if (*name == L'?' || _wcsicmp(name, L"QrcInfos") == 0)
            {
                pStart = wcschr(pStart + 1, L'<');
                continue;   // QrcInfos 或者xml头 不处理, 跳到下一个标签
            }
            if (_wcsicmp(name, L"QrcHeadInfo") == 0)
            {
                pfn_QrcHeadInfo();
                continue;
            }
            if (_wcsicmp(name, L"LyricInfo") == 0)
            {
                pfn_LyricInfo();
                continue;
            }
            if (_wcsnicmp(name, L"Lyric_", 6) == 0)
            {
                // 这个标签是核心内容, 解析, 需要获取出歌词信息, 然后解析
                LPCWSTR name = _qrc_get_name(pStart, pEnd);
                if (_wcsicmp(name, L"LyricType") == 0)
                {
                    LyricType = _qrc_get_value(pStart, pEnd);
                    name = _qrc_get_name(pStart, pEnd);
                }

                if (_wcsicmp(name, L"LyricContent") == 0)
                {
                    while (pStart < pEnd && *pStart != L'\"')
                        ++pStart;
                    pStart++;
                    return _qrc_parse_lyric(pLyric, pStart, pEnd);
                }
#ifdef _DEBUG
                else
                    __debugbreak();
#endif

                continue;
            }

            continue;
        }

    }


    return false;
}

bool _lrc_decrypt_qrc(const void* pData, size_t nSize, wchar_t** ppLyricText)
{
    *ppLyricText = nullptr;
    if (nSize < 11)
        return false;

    // 需要拷贝一份内存, 不能修改原内存数据
    std::string buffer(reinterpret_cast<LPCSTR>(pData), nSize);
    auto pStart = reinterpret_cast<uint8_t*>(&buffer[0]);
    auto pEnd = pStart + nSize;

    // 1. 解密, 调用后会修改pStart指向的内存数据
    DESHelper::qmc1_decrypt(pStart, nSize);

    uint8_t* src = pStart + 11;
    size_t srcSize = nSize - 11;
    std::vector<uint8_t> data(srcSize);

    const uint8_t QQKey[] = "!@#)(*$%123ZXC!@!@#)(NHL";
    //const uint8_t QQKey[] = {
    //    '!', '@', '#', ')', '(', '*', '$', '%',
    //    '1', '2', '3', 'Z', 'X', 'C', '!', '@',
    //    '!', '@', '#', ')', '(', 'N', 'H', 'L'
    //};


    // 2. 生成 3DES 解密子密钥
    uint8_t schedule[3][16][6] = { 0 };
    DESHelper::TripleDESKeySetup(QQKey, schedule, DESHelper::DECRYPT);

    // 3. 8字节为一块, 逐块解密
    for (size_t i = 0; i < srcSize; i += 8)
    {
        uint8_t temp[8] = { 0 };
        DESHelper::TripleDESCrypt(&src[i], temp, schedule);

        for (int j = 0; j < 8; ++j)
            data[i + j] = temp[j];
    }

    // 4. zlib 解压, 解压成功就是qrc文件内容, UTF8编码
    std::string u8;
    if (!zlib_decompress(&data[0], data.size(), u8))
        return false;

    // 5. UTF8 转 Unicode, 返回结果
    *ppLyricText = charset_stl::Utf8ToUnicode(u8.c_str(), u8.size());
    return true;
}

bool _qrc_parse_lyric(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPCWSTR pEnd)
{
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
        wchar_t ch = *pStart++;
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
                return __qrc_parse_text(pLyric, pStart - 1, pEnd);
                break;  // 直接跳出循环了, 解析歌词文本的时候会一直解析到结尾
            }
#undef _cmp
        }
        if (ch == L'\"')
            break;
    }
    return false;
}

// 解析歌词行, "[开始时间,结束时间]字(开始时间,结束时间)" 解析这种内容
bool __qrc_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPCWSTR pEnd)
{
    wchar_t num[50] = { 0 };
    auto pfn_get_num = [&pStart, &pEnd, &num]() -> int
        {
            while (*pStart == L' ')
                pStart++;

            int i = 0;
#define isnumber(_n) ( (_n) >= L'0' && (_n) <= L'9' || (_n) == L'-' )
            while (isnumber(*pStart))
                num[i++] = *pStart++, num[i] = 0;
#undef isnumber
            while (pStart < pEnd && (*pStart == L',' || *pStart == L']' || *pStart == L')'))
                pStart++;
            return _wtol(num);
        };

    // 遇到( 左括号就返回
    auto pfn_get_word = [&](int& size)
        {
            LPCWSTR ret = pStart;
            size = 0;
            while (pStart < pEnd && *pStart != L'(')
                ++pStart, ++size;
            *pStart++ = L'\0';
            return ret;
        };

    while (pStart < pEnd)
    {
        if (*pStart == L'\"')
            return true;
        // 是这种格式, 全部按这个格式解析, 然后保存到数组
        // [123,456]字(1,2)字(1,2)\r\n
        if (*pStart == L'[')
            pStart++;


        // 这一行的歌词数据
        INSIDE_LYRIC_LINE& lines = pLyric->lines.emplace_back();

        lines.start = pfn_get_num();
        lines.duration = pfn_get_num();
        lines.interval = MAXINT;    // 每次都假设这个是最后一行

        // 这里开始就是 字(1,2)\r\n 这种格式了, 数量不一定, 需要循环解析
        while (pStart < pEnd && *pStart != L'\r' && *pStart != L'\n')
        {
            LPWSTR pWordStart = pStart;
            INSIDE_LYRIC_WORD& words = lines.words.emplace_back();
            words.text = pfn_get_word(words.size);
            words.start = pfn_get_num() - lines.start;
            words.duration = pfn_get_num();

            lines.text.append(words.text, words.size);
        }

        while (*pStart == L'\r' || *pStart == L'\n')
            *pStart++ = 0;

        // 成员数大于1, 需要获取上一个成员, 然后计算当前成员和上一个成员的间隔时间
        // 当前成员就是最后一个, 上一个成员是倒数第二个
        size_t lines_size = pLyric->lines.size();
        if (lines_size > 1)
        {
            INSIDE_LYRIC_LINE& back = pLyric->lines[lines_size - 2];
            // 间隔 = 当前行的开始时间 减去 上一行的结束时间
            back.interval = lines.start - (back.start + back.duration);
            //_dbg_append_interval_time(pLyric, lines_size - 2);

        }

    }

    return false;
}

LPCWSTR _qrc_get_name(LPWSTR& pStart, LPCWSTR pEnd)
{
    LPCWSTR ret = pStart;
    while (pStart < pEnd && *pStart != L' ' && *pStart != L'>' && *pStart != L'=')
        ++pStart;
    if (pStart < pEnd)
        *pStart++ = L'\0';
    return ret;
}

LPCWSTR _qrc_get_value(LPWSTR& pStart, LPCWSTR pEnd)
{
    while (pStart < pEnd && *pStart != L'\"')
        ++pStart;

    pStart++;   // 这里开始是引号后面的内容了, 也是值的开始
    LPCWSTR ret = pStart;
    while (pStart < pEnd)
    {
        wchar_t& ch = *pStart++;
        if (ch == L'\"')
        {
            // 内容结束
            ch = L'\0';
            while (pStart < pEnd && *pStart == L' ')
                ++pStart;
            return ret;
        }
    }
#ifdef _DEBUG
    __debugbreak();
#endif
    return nullptr;
}



LYRIC_NAMESPACE_END

