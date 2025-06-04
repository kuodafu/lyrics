#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <kuodafu_lyric.h>
#include "../charset_stl.h"

#define LYRIC_NAMESPACE lyric
#define LYRIC_NAMESPACE_BEGIN namespace LYRIC_NAMESPACE {
#define LYRIC_NAMESPACE_END }

LYRIC_NAMESPACE_BEGIN

// 一个字的结构, 记录这个字的开始时间, 持续时间, 歌词内容
struct INSIDE_LYRIC_WORD
{
    int     start;      // 开始时间是基于这一行时间的开始
    int     duration;   // 这个字持续的时间
    int     t3;         // 暂时不知道这个值是干嘛的, 看了一下, 好像都是0
    float   width;      // 这一个字占用的文本宽度, 为0就是没有计算
    float   left;       // 前面所有字的宽度之和, 这个字的左边距
    float   top;        // 前面所有字的高度之和, 这个字的顶边距, 纵向歌词使用
    float   height;     // 这一个字占用的文本高度
    LPCWSTR text;       // 歌词内容, 这里的指针是指向歌词对象里的 krc 字符串里的字
    int     size;       // 这个字的字符数
    INSIDE_LYRIC_WORD()
    {
        start = duration = t3 = size = 0;
        width = height = 0.0f;
        top = 0.f;
        text = nullptr;
        left = 0;
    }
};

template<typename T, typename R>inline bool __query(T l, R r)
{
    return ((R)l & r) == r;
}

using INSIDE_LYRIC_WORDS = std::vector<INSIDE_LYRIC_WORD>;

#ifndef MAXINT
#define MAXINT ((INT)(((UINT)~((UINT)0)) >> 1))
#endif

// 一行歌词的结构, 记录这一行的开始时间, 持续时间, 歌词内容, 以及每一个字的结构
struct INSIDE_LYRIC_LINE
{
    int                 start;      // 这一行的开始时间
    int                 duration;   // 这一行持续的时间
    int                 interval;   // 距离下一行的间隔时间, 单位是毫秒, MAXINT表示是最后一行
    float               width;      // 这一行歌词占用的文本宽度
    float               height;     // 这一行歌词占用的文本高度, 纵向歌词使用
    int                 size;       // 这一整行的歌词内容的字符数, 不包括结束符, 不能使用 text.size(), 音译歌词会存放3倍长度的文本
    std::wstring        text;       // 这一整行的歌词内容
    INSIDE_LYRIC_WORDS  words;      // 每一个字的结构

    INSIDE_LYRIC_LINE()
    {
        start = duration = 0;
        width = 0;
        height = 0;
        interval = 0;
        size = 0;
    }
    INSIDE_LYRIC_LINE(INSIDE_LYRIC_LINE&& obj) noexcept
    {
        start       = obj.start;
        duration    = obj.duration;
        interval    = obj.interval;
        width       = obj.width;
        height      = obj.height;
        size        = obj.size;
        text        = std::move(obj.text);
        words       = std::move(obj.words);
    }
    ~INSIDE_LYRIC_LINE()
    {

    }
};
using INSIDE_LYRIC_LINDS = std::vector<INSIDE_LYRIC_LINE>;

// 翻译歌词的结构
struct INSIDE_LYRIC_TRANSLATE
{
    int language;   // 暂时不知道是什么, 看了一下, 好像都是0
    int type;       // 0: 翻译, 1: 音译
    std::vector<std::wstring> lines;    // 翻译/音译歌词的每一行, 翻译/音译是没有逐字的, 都是一行一行记录的
    INSIDE_LYRIC_TRANSLATE()
    {
        language = type = 0;
    }
};
using INSIDE_LYRIC_LANGUAGE = std::vector<INSIDE_LYRIC_TRANSLATE>;

typedef struct INSIDE_LYRIC_INFO
{
    LPWSTR                  krc;        // 解密后的krc数据, 解析后会把很多杂字符改成0
    INSIDE_LYRIC_LINDS      lines;      // 每一行的结构
    INSIDE_LYRIC_LINDS      lines_yy;   // 音译, 每一行的结构, 音译一般是和歌词字数一样, 音译也做逐字的
    INSIDE_LYRIC_LINDS      lines_fy;   // 翻译, 每一行的结构, 翻译一般就是一行一行记录的
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
    LYRIC_PARSE_CALCTEXT pfnCalcText;
    void*   pUserData;
    int     language;       // 当前歌词存在哪些翻译, LYRIC_LANGUAGE_TYPE 枚举值
    int     lyric_type;     // 当前解析的歌词类型, LYRIC_PARSE_TYPE 里低4位的值
    int     index;          // 歌词高亮索引, 当前是在第几行, 搜索时限搜索这个, 不是在这个索引才去搜索数组
    int     nTimeOffset;    // 时间偏移, 计算歌词位置的时候加上这个偏移
    INSIDE_LYRIC_INFO()
    {
        id = ar = ti = by = hash = al = sign = qq = total = offset = nullptr;
        krc = nullptr;
        index = -1;
        lyric_type = 0;
        pfnCalcText = nullptr;
        pUserData = nullptr;
        nTimeOffset = 0;
        language = 0;
    }
    ~INSIDE_LYRIC_INFO()
    {
        if (krc)
            charset_stl::charset_free(krc);
    }
}*PINSIDE_LYRIC_INFO;

// 读入文件数据, 返回读取的尺寸
int read_file(LPCWSTR file, std::string& ret);

// zlib解压, 返回是否解压成功
bool zlib_decompress(const void* compressedData, size_t compressedSize, std::string& output);


// 通过标志位获取pData指向的文本数据, 会判断编码转成UTF16字符串, 返回pData是否指向文本
// 如果返回false, 那就标准pData不是指向文本数据, 而是指向实际数据, 需要根据标志位来解密
bool _lrc_parse_get_lyric_text(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, wchar_t** ret);

// 根据时间查找当前时间在第几行歌词里
// 如果时间超过某一行的结束, 但是小于下一行的开始, 那返回的是某一行
// 比如 时间大于 第5行的结束时间, 小于第6行的开始时间, 那返回的是第5行
int _lrc_find_line(PINSIDE_LYRIC_INFO pLyric, int time);

// 根据时间查找是在第几个字里面
// 如果时间大于 第5个字的结束时间, 小于第6个字的开始时间, 那返回的是第5个字
int _lrc_find_word(INSIDE_LYRIC_LINE& line, int time);

// 把加密的酷狗歌词数据解密成明文
bool _lrc_decrypt_krc(const void* pData, size_t nSize, wchar_t** ppLyricText);
// 把加密的QQ音乐歌词数据解密成明文
bool _lrc_decrypt_qrc(const void* pData, size_t nSize, wchar_t** ppLyricText);
// 解密lrc, 一般lrc没加密, 这里直接拷贝一份返回
bool _lrc_decrypt_lrc(const void* pData, size_t nSize, wchar_t** ppLyricText);

// 解析酷狗歌词, 返回是否解析成功, 解析的结果都保存到pLyric里
bool _lrc_parse_krc(PINSIDE_LYRIC_INFO pLyric);
// 解析QQ音乐歌词, 返回是否解析成功, 解析的结果都保存到pLyric里
bool _lrc_parse_qrc(PINSIDE_LYRIC_INFO pLyric);
// 解析lrc歌词, 返回是否解析成功, 解析的结果都保存到pLyric里
bool _lrc_parse_lrc(PINSIDE_LYRIC_INFO pLyric);

// 根据type确定歌词是否有翻译/音译, 1=翻译, 2=音译
int _lrc_get_language(PINSIDE_LYRIC_INFO pLyric, int type);

// 最后一个字加入时间间隔显示, 调试用
void _lrc_dbg_append_interval_time(PINSIDE_LYRIC_INFO pLyric, size_t back_index);


LYRIC_NAMESPACE_END

