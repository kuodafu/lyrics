#pragma once

#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
DECLARE_HANDLE(HLYRIC);

#define LYRICCALL __stdcall


struct LYRIC_LINE_STRUCT
{
    const wchar_t*  pText;      // 行歌词文本
    const wchar_t*  pTranslate1;// 行歌词翻译文本
    const wchar_t*  pTranslate2;// 行歌词音译文本
    int             nLength;    // 字符数
    int             nTranslate1;// 字符数
    int             nTranslate2;// 字符数

    int             nStart;     // 行歌词开始时间, 单位是毫秒
    int             nEnd;       // 行歌词结束时间, 单位是毫秒
    int             nWordCount; // 这一行歌词的字数, 英文是单词数, 这里的字不是字符也不是字节, 是根据歌词内容决定的
    
    int             nWidth;     // 这一行文本占用的宽度, 单位是像素, 没有设置计算文本回调时值为0
};

struct LYRIC_WORD_STRUCT
{
    const wchar_t*  pText;      // 字歌词文本
    int             nLength;    // 字符数
    int             nStart;     // 字的开始时间, 单位是毫秒, 这个开始时间是相对歌词行的
    int             nEnd;       // 字的结束时间, 单位是毫秒

    int             nLeft;      // 这个字在这一行歌词里的左边位置, 如果没有设置计算文本回调, 下面这几个值会返回0
    int             nWidth;     // 字占用的宽度
    int             nHeight;    // 字占用的高度, 一般来说整行都是一样的高度, 目前只处理这种

};


// 计算指定时间在那一行歌词的哪一个字上
struct LYRIC_CALC_STRUCT
{
    int                 indexLine;  // 当前时间在整体歌词的那一行上, 从0开始
    int                 indexWord;  // 当前时间在这一行歌词的哪一个字上, 从0开始
    int                 nWidthWord; // 传递这个时间字索引高亮占用的宽度, 用来确定高亮位置
    LYRIC_LINE_STRUCT   line;       // 歌词行信息
    LYRIC_WORD_STRUCT   word;       // 歌词字信息

};


// 整个歌词的基础信息
struct LYRIC_INFO_STRUCT
{
    const wchar_t* id;
    const wchar_t* ar;
    const wchar_t* ti;
    const wchar_t* by;
    const wchar_t* hash;
    const wchar_t* al;
    const wchar_t* sign;
    const wchar_t* qq;
    const wchar_t* total;
    const wchar_t* offset;

    int             nArrayStrBuffer;    // 输入时表示 pArrayBuffer 的成员数, 输出时表示写入 pArrayBuffer 的成员数
    const wchar_t** pArrayStrBuffer;    // 数组缓冲区, 为0则不写入, 每一行歌词的文本

    int                 nArrayLineInfoBuffer;   // 输入时表示 pArrayLineInfoBuffer 的成员数, 输出时表示写入 pArrayLineInfoBuffer 的成员数
    LYRIC_LINE_STRUCT*  pArrayLineInfoBuffer;   // 数组缓冲区, 为0则不写入, 每一行歌词的信息, 包括歌词文本, 开始时间, 结束时间, 字数

};

#define _def_struct(_name) typedef _name *P##_name, *LP##_name; typedef const _name *PC##_name, *LPC##_name

_def_struct(LYRIC_CALC_STRUCT);
_def_struct(LYRIC_LINE_STRUCT);
_def_struct(LYRIC_WORD_STRUCT);
_def_struct(LYRIC_INFO_STRUCT);

#undef _def_struct

typedef int (LYRICCALL* LYRIC_PARSE_CALCTEXT)(void* pUserData, LPCWSTR pText, int nTextLen, int* pRetHeight);

/// <summary>
/// 解析歌词, 返回krc解密后的数据, 返回的指针需要调用 lyric_destroy 销毁句柄
/// </summary>
/// <param name="pData">输入, 需要解密的歌词数据</param>
/// <param name="nSize">输入, 歌词数据的长度</param>
/// <param name="pfnCalcText">可以为空指针, 这个是个回调函数, 用来计算文字的占用宽度, 根据文字宽度确定高亮位置</param>
/// <param name="pUserData">传递到 pfnCalcText 里的用户数据</param>
/// <returns>返回解密后的数据, 不使用时需要调用 lyric_destroy 销毁句柄</returns>
HLYRIC LYRICCALL lyric_parse(const void* pData, int nSize);

/// <summary>
/// 销毁 lyric_parse() 返回的歌词句柄
/// </summary>
/// <param name="hLyric">lyric_parse() 返回的歌词句柄</param>
/// <returns>无返回值</returns>
void LYRICCALL lyric_destroy(HLYRIC hLyric);

/// <summary>
/// 释放文本指针, 函数如果有写需要释放的, 就需要调用这个函数释放, 没写需要释放就不需要调用
/// </summary>
/// <param name="pStr">要释放的文本指针</param>
/// <returns>无返回值</returns>
void LYRICCALL lyric_free(void* pStr);

/// <summary>
/// 计算歌词文字的宽度, 调用 lyric_calc() 函数后会调用这里传递的回调函数来计算歌词占用宽度
/// 每次计算都是计算这一行所有字的宽度, 用来确定歌词高亮位置
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="pfnCalcText">可以为空指针, 这个是个回调函数, 用来计算文字的占用宽度, 根据文字宽度确定高亮位置</param>
/// <param name="pUserData">传递到 pfnCalcText 里的用户数据</param>
/// <returns>返回是否处理成功</returns>
bool LYRICCALL lyric_calc_text_width(HLYRIC hLyric, LYRIC_PARSE_CALCTEXT pfnCalcText, void* pUserData);

/// <summary>
/// 重新计算歌词文字的宽度, 调用这个函数会重新计算歌词占用宽度, 用来确定歌词高亮位置, 字体被改变时调用
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <returns>返回是否处理成功</returns>
bool LYRICCALL lyric_re_calc_text_width(HLYRIC hLyric);


/// <summary>
/// 计算指定时间是在歌词的哪一行哪一个字上
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="time">要查询的时间, 单位是毫秒</param>
/// <param name="pRet">参考返回的数据, 返回歌词行文本, 歌词字文本, 行索引, 字索引</param>
/// <returns>返回是否获取成功</returns>
bool LYRICCALL lyric_calc(HLYRIC hLyric, int time, LYRIC_CALC_STRUCT* pRet);

/// <summary>
/// 获取歌词里记录的各种基础信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="pRet">接收返回数据的结构指针, 如果需要返回歌词行文本或者行信息, 需要分配缓冲区, 然后传递进来</param>
/// <returns>返回是否获取成功</returns>
bool LYRICCALL lyric_get_info(HLYRIC hLyric, PLYRIC_INFO_STRUCT pRet);


/// <summary>
/// 获取歌词行数
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <returns>返回行数, 没有返回0</returns>
int LYRICCALL lyric_get_line_count(HLYRIC hLyric);

/// <summary>
/// 获取某一行歌词字数, 英文歌词是单词数
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <returns>返回字数, 没有返回0</returns>
int LYRICCALL lyric_get_word_count(HLYRIC hLyric, int indexLine);

/// <summary>
/// 获取歌词行信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="pRet">返回的歌词行信息</param>
/// <returns>返回是否获取成功</returns>
bool LYRICCALL lyric_get_line(HLYRIC hLyric, int indexLine, PLYRIC_LINE_STRUCT pRet);

/// <summary>
/// 获取某一行歌词里指定字的信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="indexWord">歌词字索引</param>
/// <param name="pRet">返回的歌词字信息</param>
/// <returns>返回是否获取成功</returns>
bool LYRICCALL lyric_get_word(HLYRIC hLyric, int indexLine, int indexWord, PLYRIC_WORD_STRUCT pRet);

/// <summary>
/// 获取所有歌词行信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="pArrayBuffer">接收歌词信息的数组缓冲区, 需要获取字数可以调用 lyric_get_line_count() 获取</param>
/// <param name="nBufferCount">表示 pArrayBuffer 数组有多少个成员</param>
/// <returns>返回写入缓冲区的成员数</returns>
int LYRICCALL lyric_get_all_line(HLYRIC hLyric, PLYRIC_LINE_STRUCT pArrayBuffer, int nBufferCount);

/// <summary>
/// 获取某一行歌词里所有字的信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="pArrayBuffer">接收字信息的数组缓冲区, 需要获取字数可以调用 lyric_get_word_count() 获取</param>
/// <param name="nBufferCount">表示 pArrayBuffer 数组有多少个成员</param>
/// <returns>返回写入缓冲区的成员数</returns>
int LYRICCALL lyric_get_all_word(HLYRIC hLyric, int indexLine, PLYRIC_WORD_STRUCT pArrayBuffer, int nBufferCount);

/// <summary>
/// 获取歌词行文本
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <returns>返回歌词文本, 失败或者没有则返回空文本指针</returns>
const wchar_t* LYRICCALL lyric_get_line_str(HLYRIC hLyric, int indexLine);

/// <summary>
/// 获取某一行歌词里指定字的文本
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="indexWord">歌词字索引</param>
/// <returns>返回字文本, 失败或者没有则返回空文本指针</returns>
const wchar_t* LYRICCALL lyric_get_word_str(HLYRIC hLyric, int indexLine, int indexWord);

/// <summary>
/// 获取某一行歌词所有字, 需要在外部分配足够的空间存放歌词文本数组
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="pArrayBuffer">接收字的数组缓冲区, 需要获取字数可以调用 lyric_get_word_count() 获取</param>
/// <param name="nBufferCount">表示 pArrayBuffer 数组有多少个成员</param>
/// <returns>返回写入数组的成员数</returns>
int LYRICCALL lyric_get_word_all_str(HLYRIC hLyric, int indexLine, const wchar_t** pArrayBuffer, int nBufferCount);

/// <summary>
/// 获取所有歌词行文本, 需要在外部分配足够的空间存放歌词文本数组
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="pArrayBuffer">接收歌词的数组缓冲区, 需要获取歌词行数可以调用 lyric_get_line_count() 获取</param>
/// <param name="nBufferCount">表示 pArrayBuffer 数组有多少个成员</param>
/// <returns>返回写入数组的成员数</returns>
int LYRICCALL lyric_get_line_all_str(HLYRIC hLyric, const wchar_t** pArrayBuffer, int nBufferCount);

/// <summary>
/// 把歌词转换成lrc格式, lrc格式的歌词是没有逐字的, 都是一行一行的, 返回的指针需要调用 lyric_free() 释放
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <returns>返回歌词文本指针, 失败返回0, 不使用时需要调用 lyric_free() 释放</returns>
wchar_t* LYRICCALL lyric_to_lrc(HLYRIC hLyric, int indexLine, int indexWord);

/// <summary>
/// 获取歌词语言信息, 翻译歌词使用
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <returns>返回歌词语言, 0=没有翻译, 1=翻译, 2=音译, 3=翻译+音译, 其他待定</returns>
int LYRICCALL lyric_get_language(HLYRIC hLyric);

