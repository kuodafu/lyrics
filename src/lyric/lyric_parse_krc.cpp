#include "lyric_typedef.h"
#include <cjson/cJSON.h>

#include "../charset_stl.h"
#include "../base64.h"

LYRIC_NAMESPACE_BEGIN

void __krc_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPWSTR pEnd);
void __krc_get_translate(PINSIDE_LYRIC_INFO pLyric, LPCWSTR language);


// �����ṷ��krc, ����ǰ���������Ѿ����浽 pLyric->krc ��
// ���ﲻ���ж�, ����ǰ��Ҫ�ж�
bool _lrc_parse_krc(PINSIDE_LYRIC_INFO pLyric)
{
    size_t nSize = wcslen(pLyric->krc);
    LPWSTR pStart = &pLyric->krc[0];
    LPWSTR pEnd = pStart + nSize;
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
                __krc_parse_text(pLyric, pStart - 1, pEnd);
                break;  // ֱ������ѭ����, ��������ı���ʱ���һֱ��������β
            }
#undef _cmp
        }
    }

    if (language)
        __krc_get_translate(pLyric, language);

#ifdef _DEBUG
    if (pLyric->lines.size() > 300)
        __debugbreak();
#endif
    return (HLYRIC)pLyric;
}

bool _lrc_decrypt_krc(const void* pData, size_t nSize, wchar_t** ppLyricText)
{
    *ppLyricText = nullptr;
    const BYTE zh[] = { 64, 71, 97, 119, 94, 50, 116, 71, 81, 54, 49, 45, 206, 210, 110, 105 };
    std::string lyric((LPCSTR)pData + 4, nSize - 4);
    for (size_t i = 0; i < nSize - 4; i++)
        lyric[i] ^= zh[i % 16];


    std::string u8;
    if(!zlib_decompress(lyric.c_str(), lyric.size(), u8))
        return false;

    *ppLyricText = charset_stl::Utf8ToUnicode(u8.c_str(), u8.size());
    return true;
}


// ���������, "[��ʼʱ��,����ʱ��]<��ʼʱ��,����ʱ��,0>��" ������������
void __krc_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPWSTR pEnd)
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
        while (pStart < pEnd && (*pStart == L',' || *pStart == L']' || *pStart == L'>'))
            pStart++;
        return _wtol(num);
    };

    pLyric->lines.reserve(30);

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
        lines.interval = MAXINT;    // ÿ�ζ�������������һ��
        int duration = 0;
        lines.size = 0;

        // ���￪ʼ���� <0,1,2>��\r\n ���ָ�ʽ��, ������һ��, ��Ҫѭ������
        while (pStart < pEnd && *pStart == L'<')
        {
            *pStart++ = 0;
            INSIDE_LYRIC_WORD& words = lines.words.emplace_back();
            words.start = pfn_get_num();
            words.duration = pfn_get_num();
            duration += words.duration;
            words.t3 = pfn_get_num();
            if (words.t3 != 0)
                __debugbreak();
            words.text = pStart;
            LPWSTR apos_pos = nullptr;
            while (pStart < pEnd)
            {
                wchar_t ch = *pStart;
                if (ch == L'<' || ch == L'\r' || ch == L'\n' || ch == L'\0')
                    break;
                if (ch == L'&')
                {
                    // ��&��ͷ, �ȽϽ�����5���ַ��ǲ���&apos;
                    if (_wcsnicmp(pStart + 1, L"apos;", 5) == 0)
                    {
                        apos_pos = pStart;
                        pStart += 5;
                    }
                }
                pStart++;
            }
            words.size = (int)(pStart - words.text);
            if (apos_pos)
            {
                LPCWSTR replace_end = pStart;
                size_t replace_len = (size_t)(apos_pos - words.text);  // �ҵ�λ��ǰ���м����ַ�
                // ������ &apos; ����, Ҫ�滻�ɵ�����
                LPWSTR p1 = (LPWSTR)apos_pos;
                LPWSTR p2 = p1 + 6;

                *p1++ = L'\'';  // ���ɵ�����
                replace_len++;

                // ��p2һֱ������p1����
                while (p2 < replace_end)
                    *p1++ = *p2++, replace_len++;  // ������ַ���������ȥ, ����+1
                *p1 = 0;
                words.size = (int)replace_len;
            }

            lines.size += words.size;
        }

        // ����ѭ���ȼ����ܳ���, Ȼ��һ���Է���ÿռ�, �������
        lines.text.reserve(lines.size);
        for (auto& words : lines.words)
            lines.text.append(words.text, words.size);
        
        while (*pStart == L'\r' || *pStart == L'\n')
            *pStart++ = 0;

        if (duration < lines.duration)
            lines.duration = duration;

        // ��Ա������1, ��Ҫ��ȡ��һ����Ա, Ȼ����㵱ǰ��Ա����һ����Ա�ļ��ʱ��
        // ��ǰ��Ա�������һ��, ��һ����Ա�ǵ����ڶ���
        size_t lines_size = pLyric->lines.size();
        if (lines_size > 1)
        {
            INSIDE_LYRIC_LINE& back = pLyric->lines[lines_size - 2];
            // ��� = ��ǰ�еĿ�ʼʱ�� ��ȥ ��һ�еĽ���ʱ��
            back.interval = lines.start - (back.start + back.duration);
            _lrc_dbg_append_interval_time(pLyric, lines_size - 2);
        }

    }

    _lrc_dbg_append_interval_time(pLyric, pLyric->lines.size() - 1);
}

void __krc_get_translate_yy(INSIDE_LYRIC_LINE& line, INSIDE_LYRIC_LINE& line_yy, cJSON* json_line, int line_count)
{
    line_yy.start    = line.start;
    line_yy.duration = line.duration;
    line_yy.interval = line.interval;
    line_yy.width    = line.width;
    line_yy.height   = line.height;
    line_yy.size     = line.size;
    line_yy.words.resize(line_count);

    // ����Ҫ�� line_fy.text ����3�����ȵĿռ�
    // Ȼ���������ָ�붼��ָ�����text���ڴ�

    std::vector<std::wstring> arr;
    arr.reserve(line_count);

    size_t len = 0;
    for (int i = 0; i < line_count; i++)
    {
        LPCSTR pText = cJSON_GetStringValue(cJSON_GetArrayItem(json_line, i));
        arr.push_back(charset_stl::U2W(pText));
        len += arr.back().size();
    }
    
    // ��������, һ�����ԭʼ�ı�, һ�����ÿ���ֵ��ı�, �ټ���ÿ���ֵ��м���
    size_t str_reserve = len * 2 + line_count + 1;
    line_yy.text.reserve(str_reserve);
    line_yy.size = (int)len; // ��¼��һ�е��ַ���, ��������������Ϣ
    line_yy.text.clear();

    LPWSTR pText = &line_yy.text[0];
    LPWSTR pStart = pText + line_yy.size + 1;  // ����ֵ���ʼλ��, ����ԭʼ�ı�������ʶ������
    LPWSTR pEnd = pText + str_reserve;


    for (int i = 0; i < line_count; i++)
    {
        auto& word = line.words[i];
        auto& word_yy = line_yy.words[i];
        auto& str = arr[i];

        word_yy.duration = word.duration;
        word_yy.start = word.start;
        word_yy.t3 = word.t3;
        word_yy.width = 0;
        word_yy.left = 0;
        word_yy.top = 0;
        word_yy.height = 0;
        word_yy.text = pStart;
        word_yy.size = (int)str.size();

        wmemcpy(pText, str.c_str(), str.size());
        pText += str.size();
        *pText = L'\0'; // ���Ͻ�����, ÿ�ζ���Ϊ�����һ����, ��ʵ����Ҳ����, resize() �õ����ڴ��Ѿ���0��

        wmemcpy(pStart, str.c_str(), str.size());
        pStart += str.size();
        *pStart++ = L'\0';  // ���Ͻ�����, �����������ָ���
        



#ifdef _DEBUG
        if (pStart > pEnd || (i + 1 == line_count && pStart != pEnd))
            __debugbreak(); // ����ĳ����Ǹոպõ�, �������, �Ǿ���������
#endif

    }

}
// ��ȡ����ĸ��, ��������붼������ͳһ����
void __krc_get_translate(PINSIDE_LYRIC_INFO pLyric, LPCWSTR language)
{
    //return;
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

    for (int i = 0; i < count; i++)
    {
        cJSON* item = cJSON_GetArrayItem(content, i);

        // type 0 ������, 1 �Ƿ���
        // language ��ʱ��֪����ʲô, �����˺ܶ���, ����0
        int language = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "language"));
        int type = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "type"));

        INSIDE_LYRIC_LINDS* pLines_fy = type == 0 ? &pLyric->lines_yy : &pLyric->lines_fy;

        pLyric->language |= (type == 0 ? LYRIC_LANGUAGE_TYPE_YY : LYRIC_LANGUAGE_TYPE_FY);

#ifdef _DEBUG
        if (language)
            __debugbreak();
#endif

        cJSON* lyricContent = cJSON_GetObjectItem(item, "lyricContent");
        if (lyricContent)
        {
            int lyricContent_count = cJSON_GetArraySize(lyricContent);
            if ((int)pLyric->lines.size() == lyricContent_count)
            {
                auto& lines_fy = *pLines_fy;
                lines_fy.resize(lyricContent_count);
                for (int j = 0; j < lyricContent_count; j++)
                {
                    INSIDE_LYRIC_LINE& line = pLyric->lines[j];
                    INSIDE_LYRIC_LINE& line_fy = lines_fy[j];

                    cJSON* json_line = cJSON_GetArrayItem(lyricContent, j);
                    int line_count = cJSON_GetArraySize(json_line);
                    // ����������͸�ʵ�����һ���ʹ������벿��, ����һ������һ���ֶ�Ӧ�����һ����
                    if (type == 0 && line_count == (int)line.words.size())
                    {
                        __krc_get_translate_yy(line, line_fy, json_line, line_count);
                        continue;   // ��������, �����´���
                    }

#ifdef _DEBUG
                    if (type == 0)
                        __debugbreak(); // ���봦��ʧ��, ���µ���
#endif
                    // �����ֱ��ƴ���ı�, Ȼ�󱣴浽 lines_fy ����
                    str.clear();
                    for (int k = 0; k < line_count; k++)
                    {
                        cJSON* text = cJSON_GetArrayItem(json_line, k);
                        str.append(cJSON_GetStringValue(text));
                    }

                    line_fy.text = charset_stl::U2W(str.c_str(), str.size());
                    line_fy.size = (int)line_fy.text.size();
                }
            }
        }
    }

    cJSON_Delete(json);
}



LYRIC_NAMESPACE_END
