#include "lyric_typedef.h"
#include <zlib.h>
#include <cjson/cJSON.h>

#include "../base64.h"

#ifdef _DEBUG
#   define DEBUG_SHOW_TIME 0
#endif


LYRIC_NAMESPACE_BEGIN

int read_file(LPCWSTR file, std::string& ret)
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
    if (ReadFile(hFile, pBuffer, block_size, &dwBytesRead, NULL))
    {
        //if (dwBytesRead < (DWORD)block_size)
        //    break;
    }
    CloseHandle(hFile);
    return (int)ret.size();
}

bool zlib_decompress(const void* compressedData, size_t compressedSize, std::string& output)
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


bool _lrc_parse_get_lyric_text(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, wchar_t** ret)
{
    *ret = nullptr;
    const int  lrc_type     = (nType & 0x0f);  // 低4位是歌词类型, 表示KRC还是QRC还是LRC
    const bool is_decrypt   = __query(nType, LYRIC_PARSE_TYPE_DECRYPT);
    const bool is_path      = __query(nType, LYRIC_PARSE_TYPE_PATH);

    const int src_charset = 
          __query(nType, LYRIC_PARSE_TYPE_UTF16BE) ? CHARSET_TYPE_UTF16_BE
        : __query(nType, LYRIC_PARSE_TYPE_UTF8)    ? CHARSET_TYPE_UTF8
        : __query(nType, LYRIC_PARSE_TYPE_GBK)     ? CHARSET_TYPE_GBK
        : CHARSET_TYPE_UTF16_LE;

    if (is_decrypt || is_path)  // 已经解密, 或者是路径, 需要获取文本
        return charset_stl::ptr2utf16(pData, nSize, src_charset, *ret);

    return false;
}

int _lrc_get_language(PINSIDE_LYRIC_INFO pLyric, int type)
{
    if (pLyric->lyric_type == LYRIC_PARSE_TYPE_KRC)
    {
        if (type == 0)
            return 2;
        if (type == 1)
            return 1;
    }
    else if (pLyric->lyric_type == LYRIC_PARSE_TYPE_QRC)
    {
        // QQ音乐有翻译, 这里需要判断
    }

    // 其他类型如果有翻译/音译, 在这里加上判断

    return 0;
}

void _lrc_dbg_append_interval_time(PINSIDE_LYRIC_INFO pLyric, size_t back_index)
{
    return;
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


LYRIC_NAMESPACE_END
