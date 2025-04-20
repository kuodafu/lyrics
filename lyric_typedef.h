#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "kuodafu_lyric.h"

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
    int     width;      // 这一个字占用的文本宽度, 为0就是没有计算
    int     left;       // 前面所有字的宽度之和, 这个字的左边距
    int     height;     // 这一个字占用的文本高度, 目前没什么用, 先记录着, 后续做什么个性歌词的时候会用到
    LPCWSTR text;       // 歌词内容, 这里的指针是指向歌词对象里的 krc 字符串里的字
    int     size;       // 这个字的字符数
    INSIDE_LYRIC_WORD()
    {
        start = duration = t3 = width = size = height = 0;
        text = nullptr;
        left = 0;
    }
};
using INSIDE_LYRIC_WORDS = std::vector<INSIDE_LYRIC_WORD>;

// 一行歌词的结构, 记录这一行的开始时间, 持续时间, 歌词内容, 以及每一个字的结构
struct INSIDE_LYRIC_LINE
{
    int                 start;      // 开始时间是基于这一行时间的开始
    int                 duration;   // 这个字持续的时间
    int                 width;      // 这一行歌词占用的文本宽度
    std::wstring        text;       // 这一整行的歌词内容
    INSIDE_LYRIC_WORDS  words;      // 每一个字的结构

    INSIDE_LYRIC_LINE()
    {
        start = duration = 0;
        width = 0;
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
    std::wstring            krc;        // 解密后的krc数据, 解析后会把很多杂字符改成0
    INSIDE_LYRIC_LINDS      lines;      // 每一行的结构
    INSIDE_LYRIC_LANGUAGE   language;   // 翻译/音译的结构
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
    int     index;          // 歌词高亮索引, 当前是在第几行, 搜索时限搜索这个, 不是在这个索引才去搜索数组
    int     nTimeOffset;    // 时间偏移, 计算歌词位置的时候加上这个偏移
    INSIDE_LYRIC_INFO()
    {
        id = ar = ti = by = hash = al = sign = qq = total = offset = nullptr;
        index = -1;
        pfnCalcText = nullptr;
        pUserData = nullptr;
        nTimeOffset = 0;
    }
}*PINSIDE_LYRIC_INFO;

LYRIC_NAMESPACE_END

