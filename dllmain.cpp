// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <string>
#include <vector>
#include "kuodafu_lyric.h"
#include <zlib.h>

#include "charset_stl.h"
#include "base64.h"

extern "C"
{
#include <cjson/cJSON.h>
}


bool lyric_decode(const void* pData, int nSize, std::wstring& krc);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

/// <summary>
/// 解码krc数据, 返回是否解密成功, krc文件就是一段歌词文本压缩后, 然后异或一段字节
/// </summary>
/// <param name="pData">krc数据地址</param>
/// <param name="nSize">krc数据尺寸</param>
/// <param name="krc">参考返回解密后的krc数据</param>
/// <returns>返回是否成功</returns>
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

// 一个字的结构, 记录这个字的开始时间, 持续时间, 歌词内容
struct LYRIC_WORD_STRUCT
{
    int     start;      // 开始时间是基于这一行时间的开始
    int     sustain;    // 这个字持续的时间
    int     t3;         // 暂时不知道这个值是干嘛的, 看了一下, 好像都是0
    LPCWSTR text;       // 歌词内容, 这里的指针是指向歌词对象里的 krc 字符串里的字
    LYRIC_WORD_STRUCT()
    {
        start = sustain = t3 = 0;
        text = nullptr;
    }
};
using LYRIC_WORDS = std::vector<LYRIC_WORD_STRUCT>;

// 一行歌词的结构, 记录这一行的开始时间, 持续时间, 歌词内容, 以及每一个字的结构
struct LYRIC_LINE_STRUCT
{
    int             start;      // 开始时间是基于这一行时间的开始
    int             sustain;    // 这个字持续的时间
    std::wstring    text;       // 这一整行的歌词内容
    LYRIC_WORDS     words;      // 每一个字的结构

    LYRIC_LINE_STRUCT()
    {
        start = sustain = 0;
    }
};
using LYRIC_LINDS = std::vector<LYRIC_LINE_STRUCT>;

// 翻译歌词的结构
struct LYRIC_TRANSLATE_STRUCT
{
    int language;   // 暂时不知道是什么, 看了一下, 好像都是0
    int type;       // 0: 翻译, 1: 音译
    std::vector<std::wstring> lines;    // 翻译/音译歌词的每一行, 翻译/音译是没有逐字的, 都是一行一行记录的
    LYRIC_TRANSLATE_STRUCT()
    {
        language = type = 0;
    }
};
using LYRIC_LANGUAGE = std::vector<LYRIC_TRANSLATE_STRUCT>;

typedef struct LYRIC_STRUCT
{
    std::wstring krc;       // 解密后的krc数据, 解析后会把很多杂字符改成0
    LYRIC_LINDS lines;      // 每一行的结构
    LYRIC_LANGUAGE language;// 翻译/音译的结构
    LPCWSTR id;
    LPCWSTR ar;
    LPCWSTR ti;
    LPCWSTR by;
    LPCWSTR hash;
    LPCWSTR al;
    LPCWSTR sign;
    LPCWSTR qq;
    LPCWSTR total;
    LPCWSTR offset;
    LYRIC_STRUCT()
    {
        id = ar = ti = by = hash = al = sign = qq = total = offset = nullptr;
    }
}*PLYRIC_STRUCT;

/// <summary>
/// 解析歌词行, "[开始时间,结束时间]<开始时间,结束时间,0>字" 解析这种内容
/// </summary>
/// <param name="pData"></param>
/// <param name="nSize"></param>
/// <returns></returns>
void lyric_parse_text(PLYRIC_STRUCT pLyric, LPWSTR pStart, LPWSTR pEnd)
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
        LYRIC_LINE_STRUCT& lines = pLyric->lines.emplace_back();

        lines.start = pfn_get_num();
        lines.sustain = pfn_get_num();

        // 这里开始就是 <0,1,2>字\r\n 这种格式了, 数量不一定, 需要循环解析
        while (pStart < pEnd && *pStart == L'<')
        {
            *pStart++ = 0;
            LYRIC_WORD_STRUCT& words = lines.words.emplace_back();
            words.start = pfn_get_num();
            words.sustain = pfn_get_num();
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
            lines.text.append(words.text, pStart - words.text);
        }

        while (*pStart == L'\r' || *pStart == L'\n')
            *pStart++ = 0;
    }
}

/// <summary>
/// 获取翻译的歌词, 翻译和音译都在这里统一处理
/// </summary>
/// <param name="pLyric"></param>
/// <param name="pStart"></param>
/// <param name="pEnd"></param>
void lyric_parse_tget_translate(PLYRIC_STRUCT pLyric, LPCWSTR language)
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

        LYRIC_TRANSLATE_STRUCT& translate = pLyric->language[i];

        translate.language = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "language"));
        translate.type = (int)cJSON_GetNumberValue(cJSON_GetObjectItem(item, "type"));
        if (translate.language)
            __debugbreak();
        cJSON* lyricContent = cJSON_GetObjectItem(item, "lyricContent");
        if (lyricContent)
        {
            int lyricContent_count = cJSON_GetArraySize(lyricContent);
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
    
    cJSON_Delete(json);

}


/// <summary>
/// 解析歌词, 返回krc解密后的数据, 返回的指针需要调用 lyric_free 释放
/// </summary>
/// <param name="pData">输入, 需要解密的歌词数据</param>
/// <param name="nSize">输入, 歌词数据的长度</param>
/// <returns>返回解密后的数据, 不使用时需要调用 lyric_free 释放</returns>
void* LYRICCALL lyric_parse(const void* pData, int nSize)
{
    auto pLyric = new LYRIC_STRUCT;
    if (!lyric_decode(pData, nSize, pLyric->krc))
    {
        delete pLyric;
        return nullptr;
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
            else if(ch == L'[')
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
            if (_cmp(L"id"))            pLyric->id       = pfn_warp(cmp_size);
            else if (_cmp(L"ar"))       pLyric->ar       = pfn_warp(cmp_size);
            else if (_cmp(L"ti"))       pLyric->ti       = pfn_warp(cmp_size);
            else if (_cmp(L"by"))       pLyric->by       = pfn_warp(cmp_size);
            else if (_cmp(L"hash"))     pLyric->hash     = pfn_warp(cmp_size);
            else if (_cmp(L"al"))       pLyric->al       = pfn_warp(cmp_size);
            else if (_cmp(L"sign"))     pLyric->sign     = pfn_warp(cmp_size);
            else if (_cmp(L"qq"))       pLyric->qq       = pfn_warp(cmp_size);
            else if (_cmp(L"total"))    pLyric->total    = pfn_warp(cmp_size);
            else if (_cmp(L"offset"))   pLyric->offset   = pfn_warp(cmp_size);
            else if (_cmp(L"language"))
            {
                language = pfn_warp(cmp_size);
            }
            else if (isdigit(*pStart))
            {
                // 遇到数字, 说明是歌词行, 另外解析歌词行
                lyric_parse_text(pLyric, pStart - 1, pEnd);
                break;  // 直接跳出循环了, 解析歌词文本的时候会一直解析到结尾
            }
#undef _cmp
        }
    }

    if (language)
        lyric_parse_tget_translate(pLyric, language);

    return pLyric;
}

/// <summary>
/// 释放插件返回的内存地址
/// </summary>
/// <param name="pData">插件返回的内存地址</param>
/// <returns>无返回值</returns>
void LYRICCALL lyric_free(void* pData)
{
    auto pLyric = (PLYRIC_STRUCT)pData;
    delete pLyric;
}

