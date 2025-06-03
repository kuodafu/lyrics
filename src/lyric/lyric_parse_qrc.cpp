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

    // ���� QrcHeadInfo ��ǩ������
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
    // ���� LyricInfo ��ǩ������
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
            // xml ��ʼ��ǩ, �ҵ���һ����ǩ
            LPCWSTR name = _qrc_get_name(pStart, pEnd);
            if (*name == L'?' || _wcsicmp(name, L"QrcInfos") == 0)
            {
                pStart = wcschr(pStart + 1, L'<');
                continue;   // QrcInfos ����xmlͷ ������, ������һ����ǩ
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
                // �����ǩ�Ǻ�������, ����, ��Ҫ��ȡ�������Ϣ, Ȼ�����
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

    // ��Ҫ����һ���ڴ�, �����޸�ԭ�ڴ�����
    std::string buffer(reinterpret_cast<LPCSTR>(pData), nSize);
    auto pStart = reinterpret_cast<uint8_t*>(&buffer[0]);
    auto pEnd = pStart + nSize;

    // 1. ����, ���ú���޸�pStartָ����ڴ�����
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


    // 2. ���� 3DES ��������Կ
    uint8_t schedule[3][16][6] = { 0 };
    DESHelper::TripleDESKeySetup(QQKey, schedule, DESHelper::DECRYPT);

    // 3. 8�ֽ�Ϊһ��, ������
    for (size_t i = 0; i < srcSize; i += 8)
    {
        uint8_t temp[8] = { 0 };
        DESHelper::TripleDESCrypt(&src[i], temp, schedule);

        for (int j = 0; j < 8; ++j)
            data[i + j] = temp[j];
    }

    // 4. zlib ��ѹ, ��ѹ�ɹ�����qrc�ļ�����, UTF8����
    std::string u8;
    if (!zlib_decompress(&data[0], data.size(), u8))
        return false;

    // 5. UTF8 ת Unicode, ���ؽ��
    *ppLyricText = charset_stl::Utf8ToUnicode(u8.c_str(), u8.size());
    return true;
}

bool _qrc_parse_lyric(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPCWSTR pEnd)
{
    // ��pStartָ����һ������, �������е�ʱ���ѻ��иĳ�\0
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

            // ÿһ�еĿ�ʼ����[, ���������ž��������п�ʼ, ������һ��
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
                // ��������, ˵���Ǹ����, ������������
                return __qrc_parse_text(pLyric, pStart - 1, pEnd);
                break;  // ֱ������ѭ����, ��������ı���ʱ���һֱ��������β
            }
#undef _cmp
        }
        if (ch == L'\"')
            break;
    }
    return false;
}

// ���������, "[��ʼʱ��,����ʱ��]��(��ʼʱ��,����ʱ��)" ������������
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

    // ����( �����žͷ���
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
        // �����ָ�ʽ, ȫ���������ʽ����, Ȼ�󱣴浽����
        // [123,456]��(1,2)��(1,2)\r\n
        if (*pStart == L'[')
            pStart++;


        // ��һ�еĸ������
        INSIDE_LYRIC_LINE& lines = pLyric->lines.emplace_back();

        lines.start = pfn_get_num();
        lines.duration = pfn_get_num();
        lines.interval = MAXINT;    // ÿ�ζ�������������һ��

        // ���￪ʼ���� ��(1,2)\r\n ���ָ�ʽ��, ������һ��, ��Ҫѭ������
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

        // ��Ա������1, ��Ҫ��ȡ��һ����Ա, Ȼ����㵱ǰ��Ա����һ����Ա�ļ��ʱ��
        // ��ǰ��Ա�������һ��, ��һ����Ա�ǵ����ڶ���
        size_t lines_size = pLyric->lines.size();
        if (lines_size > 1)
        {
            INSIDE_LYRIC_LINE& back = pLyric->lines[lines_size - 2];
            // ��� = ��ǰ�еĿ�ʼʱ�� ��ȥ ��һ�еĽ���ʱ��
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

    pStart++;   // ���￪ʼ�����ź����������, Ҳ��ֵ�Ŀ�ʼ
    LPCWSTR ret = pStart;
    while (pStart < pEnd)
    {
        wchar_t& ch = *pStart++;
        if (ch == L'\"')
        {
            // ���ݽ���
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

