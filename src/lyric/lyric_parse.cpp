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


// 通过标志位获取pData指向的文本数据, 会判断编码转成UTF16字符串, 返回pData是否指向文本
// 如果返回false, 那就标准pData不是指向文本数据, 而是指向实际数据, 需要根据标志位来解密
bool _lrc_parse_get_lyric_text(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, std::wstring& ret);

// 通过标志位获取歌词数据
bool _lrc_parse_get_lyric_data(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, std::wstring& ret);

template<typename T>
static int read_file(LPCWSTR file, T& ret)
{
    const DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_ATTRIBUTE_SYSTEM;
    HANDLE hFile = CreateFileW(file, FILE_READ_DATA, dwShareMode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    LARGE_INTEGER file_size = { 0 };
    file_size.LowPart = GetFileSize(hFile, (DWORD*)&file_size.HighPart);
    ret.resize((size_t)file_size.QuadPart);
    LPBYTE pBuffer = (LPBYTE)&ret[0];
    LPBYTE pEnd = pBuffer + file_size.QuadPart;
    DWORD dwBytesRead = 0;

    const int block_size = (int)file_size.QuadPart;
    SetFilePointer(hFile, 0, 0, FILE_BEGIN);

    if (ReadFile(hFile, pBuffer, block_size, &dwBytesRead, NULL))
    {
        //if (dwBytesRead < (DWORD)block_size)
        //    break;
    }
    CloseHandle(hFile);
    return (int)ret.size();
}

LYRIC_NAMESPACE_END


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// 下面这里是公开出去的接口

HLYRIC LYRICCALL lyric_parse(const void* pData, int nSize, LYRIC_PARSE_TYPE nType)
{
    if (!pData || nSize <= 0)
        return nullptr;
    using namespace LYRIC_NAMESPACE;
    auto pLyric = new INSIDE_LYRIC_INFO;
    std::wstring buffer_text;
    std::string lyric_data;
    if (_lrc_parse_get_lyric_text(pData, nSize, nType, buffer_text))
    {
        // 转成了文本, 判断是路径还是数据, 路径的话需要读入
        if (__query(nType, LYRIC_PARSE_TYPE_PATH))
        {
            read_file(buffer_text.c_str(), lyric_data);
            pData = lyric_data.c_str();     // 需要解密, 存放到这个变量里往下执行解密
            nSize = (int)lyric_data.size();
        }
        else
        {
            // 不是文件路径, 那就是明文数据, 存放到结构里, 然后等解析
            pLyric->krc.swap(buffer_text);
        }
    }


    if (pLyric->krc.empty())
    {
        //TODO 判断是什么歌词类型, 然后调用专门的解析函数
        if (!_lrc_decode(pData, nSize, pLyric->krc))
        {
            delete pLyric;
            return nullptr;
        }
    }

    LPWSTR pStart = &pLyric->krc[0];
    LPWSTR pEnd = pStart + pLyric->krc.size();
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
            else if (_cmp(L"language"))
            {
                language = pfn_warp(cmp_size);
            }
            else if (isdigit(*pStart))
            {
                // 遇到数字, 说明是歌词行, 另外解析歌词行
                _lrc_parse_text(pLyric, pStart - 1, pEnd);
                break;  // 直接跳出循环了, 解析歌词文本的时候会一直解析到结尾
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

// 公开的接口到这里结束
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
            output.resize(output.size() * 2); // 动态扩容
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

    output.resize(strm.total_out); // 去除多余容量
    inflateEnd(&strm);
    return true;
}


/// <summary>
/// 解码krc数据, 返回是否解密成功, krc文件就是一段歌词文本压缩后, 然后异或一段字节
/// </summary>
/// <param name="pData">krc数据地址</param>
/// <param name="nSize">krc数据尺寸</param>
/// <param name="krc">参考返回解密后的krc数据</param>
/// <returns>返回是否成功</returns>
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



// 调试状态下把时间间隔加入到歌词尾部方便调试
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
/// 解析歌词行, "[开始时间,结束时间]<开始时间,结束时间,0>字" 解析这种内容
/// </summary>
/// <param name="pData"></param>
/// <param name="nSize"></param>
/// <returns></returns>
void _lrc_parse_text(PINSIDE_LYRIC_INFO pLyric, LPWSTR pStart, LPWSTR pEnd)
{
    // 把pStart指向下一行数据, 遇到换行的时候会把换行改成\0
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
        // 是这种格式, 全部按这个格式解析, 然后保存到数组
        // [123,456]<0,1,2>字<0,1,3>字\r\n
        if (*pStart == L'[')
            pStart++;

        // 这一行的歌词数据
        INSIDE_LYRIC_LINE& lines = pLyric->lines.emplace_back();

        lines.start = pfn_get_num();
        lines.duration = pfn_get_num();
        lines.interval = MAXINT;    // 每次都假设这个是最后一行

        // 这里开始就是 <0,1,2>字\r\n 这种格式了, 数量不一定, 需要循环解析
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
                    // 是&开头, 比较接下来5个字符是不是&apos;
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
                size_t replace_len = (size_t)(apos_pos - words.text);  // 找到位置前面有几个字符
                // 这里是 &apos; 符号, 要替换成单引号
                LPWSTR p1 = (LPWSTR)apos_pos;
                LPWSTR p2 = p1 + 6;

                *p1++ = L'\'';  // 换成单引号
                replace_len++;

                // 把p2一直拷贝到p1里面
                while (p2 < replace_end)
                    *p1++ = *p2++, replace_len++;  // 后面的字符都拷贝过去, 数量+1
                *p1 = 0;
                words.size = (int)replace_len;
            }

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
            _dbg_append_interval_time(pLyric, lines_size - 2);

        }

    }

    _dbg_append_interval_time(pLyric, pLyric->lines.size() - 1);
}

/// <summary>
/// 获取翻译的歌词, 翻译和音译都在这里统一处理
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

bool _lrc_parse_get_lyric_text(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, std::wstring& ret)
{
    ret.clear();
    const int   lrc_type    = (nType & 0x0f);  // 低4位是歌词类型, 表示KRC还是QRC还是LRC
    const bool  is_decrypt  = __query(nType, LYRIC_PARSE_TYPE_DECRYPT);
    const bool is_path      = __query(nType, LYRIC_PARSE_TYPE_PATH);

    const int src_charset = 
          __query(nType, LYRIC_PARSE_TYPE_UTF16BE) ? CHARSET_TYPE_UTF16_BE
        : __query(nType, LYRIC_PARSE_TYPE_UTF8)    ? CHARSET_TYPE_UTF8
        : __query(nType, LYRIC_PARSE_TYPE_GBK)     ? CHARSET_TYPE_GBK
        : CHARSET_TYPE_UTF16_LE;

    if (is_decrypt || is_path)  // 已经解密, 或者是路径, 需要获取文本
        return charset_stl::ptr2utf16(pData, nSize, src_charset, ret);

    return false;
}


LYRIC_NAMESPACE_END
