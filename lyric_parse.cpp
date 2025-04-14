#include "lyric_typedef.h"
#include <zlib.h>

#include "charset_stl.h"
#include "base64.h"

extern "C"
{
#include <cjson/cJSON.h>
}

LYRIC_NAMESPACE_BEGIN

bool lyric_decode(const void* pData, int nSize, std::wstring& krc);
void lyric_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPWSTR pEnd);
void lyric_parse_tget_translate(PINSIDE_LYRIC_INFO pLyric, LPCWSTR language);


LYRIC_NAMESPACE_END


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// ���������ǹ�����ȥ�Ľӿ�

HLYRIC LYRICCALL lyric_parse(const void* pData, int nSize)
{
    if (!pData || nSize <= 0)
        return nullptr;
    using namespace LYRIC_NAMESPACE;
    auto pLyric = new INSIDE_LYRIC_INFO;
    if (!lyric_decode(pData, nSize, pLyric->krc))
    {
        delete pLyric;
        return nullptr;
    }

    LPWSTR pStart = &pLyric->krc[0];
    LPWSTR pEnd = pStart + pLyric->krc.size();
    LPCWSTR language = nullptr;

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
        wchar_t& ch = *pStart++;
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
            else if (_cmp(L"language"))
            {
                language = pfn_warp(cmp_size);
            }
            else if (isdigit(*pStart))
            {
                // ��������, ˵���Ǹ����, ������������
                lyric_parse_text(pLyric, pStart - 1, pEnd);
                break;  // ֱ������ѭ����, ��������ı���ʱ���һֱ��������β
            }
#undef _cmp
        }
    }

    if (language)
        lyric_parse_tget_translate(pLyric, language);

    return (HLYRIC)pLyric;
}

void LYRICCALL lyric_destroy(HLYRIC pData)
{
    using namespace LYRIC_NAMESPACE;
    auto pLyric = (PINSIDE_LYRIC_INFO)pData;
    delete pLyric;
}


// �����Ľӿڵ��������
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




LYRIC_NAMESPACE_BEGIN



/// <summary>
/// ����krc����, �����Ƿ���ܳɹ�, krc�ļ�����һ�θ���ı�ѹ����, Ȼ�����һ���ֽ�
/// </summary>
/// <param name="pData">krc���ݵ�ַ</param>
/// <param name="nSize">krc���ݳߴ�</param>
/// <param name="krc">�ο����ؽ��ܺ��krc����</param>
/// <returns>�����Ƿ�ɹ�</returns>
bool lyric_decode(const void* pData, int nSize, std::wstring& krc)
{
    krc.clear();
    const BYTE zh[] = { 64, 71, 97, 119, 94, 50, 116, 71, 81, 54, 49, 45, 206, 210, 110, 105 };
    std::string lyric((LPCSTR)pData + 4, nSize - 4);
    for (int i = 0; i < nSize - 4; i++)
        lyric[i] ^= zh[i % 16];

    uLongf destLen = (nSize - 4) * 10;
    std::string u8(destLen, 0);

    int err = uncompress((Bytef*)&u8[0], &destLen, (const Bytef*)lyric.data(), (uLongf)lyric.size());
    if (err != Z_OK)
        return false;
    u8.resize(destLen);
    krc = charset_stl::U2W(u8);
    return true;
}

/// <summary>
/// ���������, "[��ʼʱ��,����ʱ��]<��ʼʱ��,����ʱ��,0>��" ������������
/// </summary>
/// <param name="pData"></param>
/// <param name="nSize"></param>
/// <returns></returns>
void lyric_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPWSTR pEnd)
{
    // ��pStartָ����һ������, �������е�ʱ���ѻ��иĳ�\0
    auto pfn_warp = [&pStart, &pEnd](size_t offset)
    {
        pStart += offset;
        auto pRet = pStart;
        while (pStart < pEnd)
        {
            wchar_t& ch = *pStart++;
            if (ch == L'\r' || ch == L'\n')
                ch = 0;
            else
                break;
        }
        return pRet;
    };

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
        while (pStart < pEnd && (*pStart == L',' || *pStart == L']' || *pStart == L'>'))
            pStart++;
        return _wtol(num);
    };


    while (pStart < pEnd)
    {
        // �����ָ�ʽ, ȫ���������ʽ����, Ȼ�󱣴浽����
        // [123,456]<0,1,2>��<0,1,3>��\r\n
        if (*pStart == L'[')
            pStart++;

        // ��һ�еĸ������
        INSIDE_LYRIC_LINE& lines = pLyric->lines.emplace_back();

        lines.start = pfn_get_num();
        lines.duration = pfn_get_num();

        // ���￪ʼ���� <0,1,2>��\r\n ���ָ�ʽ��, ������һ��, ��Ҫѭ������
        while (pStart < pEnd && *pStart == L'<')
        {
            *pStart++ = 0;
            INSIDE_LYRIC_WORD& words = lines.words.emplace_back();
            words.start = pfn_get_num();
            words.duration = pfn_get_num();
            words.t3 = pfn_get_num();
            if (words.t3 != 0)
                __debugbreak();
            words.text = pStart;
            while (pStart < pEnd)
            {
                wchar_t ch = *pStart;
                if (ch == L'<' || ch == L'\r' || ch == L'\n' || ch == L'\0')
                    break;
                pStart++;
            }
            words.size = (int)(pStart - words.text);
            lines.text.append(words.text, pStart - words.text);
        }

        while (*pStart == L'\r' || *pStart == L'\n')
            *pStart++ = 0;
    }
}

/// <summary>
/// ��ȡ����ĸ��, ��������붼������ͳһ����
/// </summary>
/// <param name="pLyric"></param>
/// <param name="pStart"></param>
/// <param name="pEnd"></param>
void lyric_parse_tget_translate(PINSIDE_LYRIC_INFO pLyric, LPCWSTR language)
{
    auto languageA = charset_stl::W2A(language);
    std::string base_de_data(languageA.size(), 0);

    size_t olen = 0;
    mbedtls_base64_decode((unsigned char*)base_de_data.data(), base_de_data.size(),
                          &olen,
                          (const unsigned char*)languageA.c_str(), languageA.size());
    base_de_data.resize(olen);

    cJSON* json = olen > 0 ? cJSON_Parse(base_de_data.c_str()) : nullptr;
    if (!json)
        return;

    cJSON* content = cJSON_GetObjectItem(json, "content");
    if (!content)
    {
        cJSON_Delete(json);
        return;
    }

    std::string str(50, 0);
    int count = cJSON_GetArraySize(content);

    pLyric->language.resize(count);
    for (int i = 0; i < count; i++)
    {
        cJSON* item = cJSON_GetArrayItem(content, i);

        INSIDE_LYRIC_TRANSLATE& translate = pLyric->language[i];

        translate.language = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "language"));
        translate.type = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "type"));
        if (translate.language)
            __debugbreak();
        cJSON* lyricContent = cJSON_GetObjectItem(item, "lyricContent");
        if (lyricContent)
        {
            int lyricContent_count = cJSON_GetArraySize(lyricContent);
            if ((int)pLyric->lines.size() == lyricContent_count)
            {
                translate.lines.resize(lyricContent_count);
                for (int j = 0; j < lyricContent_count; j++)
                {
                    cJSON* line = cJSON_GetArrayItem(lyricContent, j);
                    int line_count = cJSON_GetArraySize(line);
                    str.clear();
                    for (int k = 0; k < line_count; k++)
                    {
                        cJSON* text = cJSON_GetArrayItem(line, k);
                        str.append(cJSON_GetStringValue(text));
                    }

                    translate.lines[j] = charset_stl::U2W(str.c_str(), str.size());
                }
            }
        }
    }

    cJSON_Delete(json);
}


LYRIC_NAMESPACE_END
