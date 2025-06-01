#include "lyric_typedef.h"
#include <zlib.h>
#include <cjson/cJSON.h>

#include "../charset_stl.h"
#include "../base64.h"

#ifdef _DEBUG
#   define DEBUG_SHOW_TIME 0
#endif



LYRIC_NAMESPACE_BEGIN

bool _lrc_decode(const void* pData, int nSize, std::wstring& krc);
void _lrc_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPWSTR pEnd);
void _lrc_parse_tget_translate(PINSIDE_LYRIC_INFO pLyric, LPCWSTR language);

// ��pDataת��UTF-16������ַ���, �����Ƿ�ת���ɹ�, ������ݵı���������δ֪��, �򷵻�false
bool _lrc_to_utf16_le(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, std::wstring& ret);

// ͨ����־λ��ȡpDataָ����ı�����, ���жϱ���ת��UTF16�ַ���, ����pData�Ƿ�ָ���ı�
// �������false, �Ǿͱ�׼pData����ָ���ı�����, ����ָ��ʵ������, ��Ҫ���ݱ�־λ������
bool _lrc_parse_get_lyric_text(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, std::wstring& ret);

// ͨ����־λ��ȡ�������
bool _lrc_parse_get_lyric_data(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, std::wstring& ret);

LYRIC_NAMESPACE_END


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// ���������ǹ�����ȥ�Ľӿ�

HLYRIC LYRICCALL lyric_parse(const void* pData, int nSize, LYRIC_PARSE_TYPE nType)
{
    if (!pData || nSize <= 0)
        return nullptr;
    using namespace LYRIC_NAMESPACE;
    auto pLyric = new INSIDE_LYRIC_INFO;
    if (false)
    {
        pLyric->krc.assign((LPCWSTR)pData, nSize);
    }
    else
    {
        if (!_lrc_decode(pData, nSize, pLyric->krc))
        {
            delete pLyric;
            return nullptr;
        }
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
                _lrc_parse_text(pLyric, pStart - 1, pEnd);
                break;  // ֱ������ѭ����, ��������ı���ʱ���һֱ��������β
            }
#undef _cmp
        }
    }

    if (language)
        _lrc_parse_tget_translate(pLyric, language);

    if (pLyric->lines.size() > 300)
        __debugbreak();
    return (HLYRIC)pLyric;
}

void LYRICCALL lyric_destroy(HLYRIC pData)
{
    using namespace LYRIC_NAMESPACE;
    auto pLyric = (PINSIDE_LYRIC_INFO)pData;
    delete pLyric;
}

LPWSTR LYRICCALL lyric_decode2(const void* pData, int nSize)
{
    const BYTE zh[] = { 64, 71, 97, 119, 94, 50, 116, 71, 81, 54, 49, 45, 206, 210, 110, 105 };
    std::string lyric((LPCSTR)pData + 4, nSize - 4);
    for (int i = 0; i < nSize - 4; i++)
        lyric[i] ^= zh[i % 16];

    uLongf destLen = (nSize - 4) * 10;
    std::string u8(destLen, 0);

    int err = uncompress((Bytef*)&u8[0], &destLen, (const Bytef*)lyric.data(), (uLongf)lyric.size());
    if (err != Z_OK)
        return nullptr;
    u8.resize(destLen);
    auto krc = charset_stl::U2W(u8);
    auto p = new wchar_t[krc.size() + 1];
    wcsncpy_s(p, krc.size() + 1, krc.c_str(), krc.size());
    return p;
}

// �����Ľӿڵ��������
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////




LYRIC_NAMESPACE_BEGIN


static bool DecompressZlib(const void* compressedData, size_t compressedSize, std::string& output)
{
    size_t bufferSize = compressedSize * 4;

    z_stream strm{};
    strm.next_in = const_cast<Bytef*>(static_cast<const Bytef*>(compressedData));
    strm.avail_in = static_cast<uInt>(compressedSize);
    strm.total_out = 0;

    if (inflateInit(&strm) != Z_OK)
        return false;

    output.resize(bufferSize);

    int ret;
    do
    {
        if (strm.total_out >= output.size())
        {
            output.resize(output.size() * 2); // ��̬����
        }

        strm.next_out = reinterpret_cast<Bytef*>(&output[0]) + strm.total_out;
        strm.avail_out = static_cast<uInt>(output.size() - strm.total_out);

        ret = inflate(&strm, Z_NO_FLUSH);
        if (ret == Z_STREAM_ERROR || ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
        {
            output.clear();
            inflateEnd(&strm);
            return false;
        }

    } while (ret != Z_STREAM_END);

    output.resize(strm.total_out); // ȥ����������
    inflateEnd(&strm);
    return true;
}


/// <summary>
/// ����krc����, �����Ƿ���ܳɹ�, krc�ļ�����һ�θ���ı�ѹ����, Ȼ�����һ���ֽ�
/// </summary>
/// <param name="pData">krc���ݵ�ַ</param>
/// <param name="nSize">krc���ݳߴ�</param>
/// <param name="krc">�ο����ؽ��ܺ��krc����</param>
/// <returns>�����Ƿ�ɹ�</returns>
bool _lrc_decode(const void* pData, int nSize, std::wstring& krc)
{
    krc.clear();
    const BYTE zh[] = { 64, 71, 97, 119, 94, 50, 116, 71, 81, 54, 49, 45, 206, 210, 110, 105 };
    std::string lyric((LPCSTR)pData + 4, nSize - 4);
    for (int i = 0; i < nSize - 4; i++)
        lyric[i] ^= zh[i % 16];


    std::string u8;
    if(!DecompressZlib(lyric.c_str(), lyric.size(), u8))
        return false;

    krc = charset_stl::U2W(u8);
    return true;
}



// ����״̬�°�ʱ�������뵽���β���������
#if DEBUG_SHOW_TIME
static void _dbg_append_interval_time(PINSIDE_LYRIC_INFO pLyric, size_t back_index)
{
    wchar_t num[50] = { 0 };
    size_t lines_size = pLyric->lines.size();
    if (lines_size > 1)
    {
        INSIDE_LYRIC_LINE& back = pLyric->lines[back_index];

        int len = swprintf_s(num, L"{%d}", back.interval);
        INSIDE_LYRIC_WORD& words = back.words.emplace_back();
        auto& words_prev = back.words[back.words.size() - 2];
        words.start = words_prev.start + words_prev.duration;
        words.duration = 10;
        words.t3 = 0;

        words.text = back.words.front().text - len - 1;
        words.size = len;
        wcscpy_s((LPWSTR)words.text, (size_t)len + 1, num);

        back.text.append(num, len);
    }
}
#else
#define _dbg_append_interval_time(x,r)
#endif

/// <summary>
/// ���������, "[��ʼʱ��,����ʱ��]<��ʼʱ��,����ʱ��,0>��" ������������
/// </summary>
/// <param name="pData"></param>
/// <param name="nSize"></param>
/// <returns></returns>
void _lrc_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPWSTR pEnd)
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
        lines.interval = MAXINT;    // ÿ�ζ�������������һ��

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
            _dbg_append_interval_time(pLyric, lines_size - 2);

        }

    }

    _dbg_append_interval_time(pLyric, pLyric->lines.size() - 1);
}

/// <summary>
/// ��ȡ����ĸ��, ��������붼������ͳһ����
/// </summary>
/// <param name="pLyric"></param>
/// <param name="pStart"></param>
/// <param name="pEnd"></param>
void _lrc_parse_tget_translate(PINSIDE_LYRIC_INFO pLyric, LPCWSTR language)
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

bool _lrc_to_utf16_le(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, std::wstring& ret)
{
    const int charset_type = (nType & 0xf000);
    auto pStart = reinterpret_cast<const BYTE*>(pData);
    LPCWSTR pText = nullptr;

    // ��pText����UTF16���뵽ret��, �����nSize��str_len���ж�ʵ�ʵ��ı�����
    // ��ֹ���ݵ�nSize������������־, ����pData����ָ��\0��β���ַ���
    auto pfn_ret_utf16 = [&ret, pText, nSize](size_t str_len) -> bool
    {
        if (nSize > MAXINT || nSize % 2 != 0)
            return false;   // �ߴ糬��, ���߲���ż��, ������false, UTF16�϶���ż��

        size_t len = nSize / 2;
        if (len != str_len)
        {
            // ���ݽ������ֽ����ͼ�����ַ�����һ��, ˵�������ǽ�����־������, �����ر���һ��
            // ȡ��С�������, ����ı�û�н�����־��wcslen() ���ܻ�ȡ����ĵط�, ���³��ȱ䳤
            // ������һ�����, ���Ǵ��ݽ�����nSize��ʵ���ı���, ����ǰ�ʵ���ı����������
            len = min(len, str_len);
        }

        ret.assign(pText, len);
        return true;
    };

    // ���ж�BOMͷ, ���û�оͰ������ʽ��ת��
    if (nSize >= 2)
    {
        if (pStart[0] == 0xff && pStart[1] == 0xfe)
        {        // UTF16LE
            pText = reinterpret_cast<const wchar_t*>(pStart + 2);   // ����BOMͷ 
            nSize -= 2; // �ֽ�����2
            return pfn_ret_utf16(wcslen(pText));
        }

        if (pStart[0] == 0xfe && pStart[1] == 0xff)
        {
            // UTF16BE, �ֽڵ���һ��, Ȼ����ת��UTF16LE
            pStart += 2;    // ����BOMͷ 
            nSize -= 2; // �ֽ�����2
            if (nSize > MAXINT || nSize % 2 != 0)
                return false;   // �ֽ�������

            size_t str_len = 0;
            size_t len = nSize / 2;
            ret.resize(len);    // Ԥ�ȷ���ռ�

            // ѭ�����������ֽڵ�λ��
            for (size_t i = 0; i < nSize; i += 2)
            {
                wchar_t ch = MAKEWORD(pStart[i + 1], pStart[i]);
                if (ch == 0)
                    break;  // ����������־, �˳�ѭ��
                ret[i / 2] = ch;
                str_len++;
            }
            if (str_len != len)
                ret.resize(str_len);
            return true;
        }
    }    // ���ж�BOMͷ, ���û�оͰ������ʽ��ת��
    if (nSize >= 2 && pStart[0] == 0xff && pStart[1] == 0xfe)
    {
        // UTF16LE
        pText = reinterpret_cast<const wchar_t*>(pStart + 2);   // ����BOMͷ 
        nSize -= 2; // �ֽ�����2
        return pfn_ret_utf16(wcslen(pText));
    }

    // �ж��ǲ���UTFBOMͷ
    if (nSize >= 3 && pStart[0] == 0xEF && pStart[1] == 0xBB && pStart[2] == 0xBF)
    {
        LPCSTR pStr = reinterpret_cast<const char*>(pStart + 3);   // ����BOMͷ 
        nSize -= 3; // �ֽ�����3
        size_t str_len = strlen(pStr);
        size_t len = min(nSize, str_len);   // ���������־��û�д��ݵ�����

        ret = charset_stl::U2W(pStr, len);
        return true;    // ת��ֱ�ӷ���
    }

    switch (charset_type)
    {
    case LYRIC_PARSE_TYPE_UTF16  :  // pData��UTF16���������, �����Ĭ��ֵ
    //case LYRIC_PARSE_TYPE_UTF16LE:  // pData��UTF16LE���������, Ĭ�ϵ�UTF16���������ʽ
    {
        ret.assign((LPCWSTR)pData, nSize);
        return true;
    }
    case LYRIC_PARSE_TYPE_UTF16BE:  // pData��UTF16BE���������
    case LYRIC_PARSE_TYPE_UTF8   :  // pData��UTF8���������
    case LYRIC_PARSE_TYPE_GBK    :  // pData��GBK���������
    default:
        break;
    }
    return false;
}

bool _lrc_parse_get_lyric_text(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, std::wstring& ret)
{
    ret.clear();
    const int   lrc_type        = (nType & 0xff);  // ��8λ�Ǹ������, ��ʾKRC����QRC����LRC
    const bool  is_decrypt      = __query(nType, LYRIC_PARSE_TYPE_DECRYPT);
    switch (lrc_type)
    {
    case LYRIC_PARSE_TYPE_KRCDATA:  // pData��KRC�ļ�����, �����Ĭ��ֵ
    {
        if (!is_decrypt)
            return false;   // ������, ��δ����, �����ı�, ֱ�ӷ���false

        // ������, ���Ѿ�����, �Ǿ����ı�����, ���ݱ���ת����UTF16-LE����
        std::wstring krc;
    }
    case LYRIC_PARSE_TYPE_KRCFILE:  // pData��KRC�ļ�·��
    case LYRIC_PARSE_TYPE_QRCDATA:  // pData��QRC�ļ�����
    case LYRIC_PARSE_TYPE_QRCFILE:  // pData��QRC�ļ�·��
    default:
        break;
    }

    return false;
}


LYRIC_NAMESPACE_END
