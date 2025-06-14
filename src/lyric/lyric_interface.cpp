/*
* 这个文件是公开出去的函数
* 
*/
#include "lyric_typedef.h"
using namespace LYRIC_NAMESPACE;

/// <summary>
/// 解析歌词, 返回krc解密后的数据, 返回的指针需要调用 lyric_destroy 销毁句柄
/// </summary>
/// <param name="pData">输入, 执行要解析的歌词数据, 是指向文件还是数据根据nType决定, 如果传递的文本有BOM, 则忽略编码标志位, 使用BOM的编码方式</param>
/// <param name="nSize">输入, pData 的长度, 不管传递什么数据, 单位都是字节</param>
/// <param name="nType">解析类型, 见 LYRIC_PARSE_TYPE 定义</param>
/// <returns>返回解密后的数据, 不使用时需要调用 lyric_destroy 销毁句柄</returns>
HLYRIC LYRICCALL lyric_parse(const void* pData, int nSize, LYRIC_PARSE_TYPE nType)
{
    if (!pData)
        return nullptr;
    wchar_t* lyric_text = lyric_decrypt(pData, nSize, nType);
    if (!lyric_text)
        return nullptr; // 解密文本失败, 返回空指针

    // 走到这里就是解密成功了, 根据歌词类型去解析
    auto pLyric = new INSIDE_LYRIC_INFO;
    bool bParse = false;
    pLyric->krc = lyric_text;   // 必须要保存, 然后下面的函数会解析这个文本
    pLyric->lyric_type = nType & 0x0f;

    switch (pLyric->lyric_type)
    {
    case LYRIC_PARSE_TYPE_KRC:
        bParse = _lrc_parse_krc(pLyric);
        break;
    case LYRIC_PARSE_TYPE_QRC:
        bParse = _lrc_parse_qrc(pLyric);
        break;
    case LYRIC_PARSE_TYPE_LRC:
        bParse = _lrc_parse_lrc(pLyric);
        break;
    default:
        delete pLyric;
        return nullptr;   // 不支持的歌词类型
    }
    if (!bParse)
    {
        delete pLyric;
        return nullptr;   // 解析失败, 返回空指针
    }
    return (HLYRIC)pLyric;
}

/// <summary>
/// 解密歌词, 返回解密后的明文数据, 返回的指针需要调用 lyric_free 释放
/// </summary>
/// <param name="pData">输入, 执行要解析的歌词数据, 是指向文件还是数据根据nType决定, 如果传递的文本有BOM, 则忽略编码标志位, 使用BOM的编码方式</param>
/// <param name="nSize">输入, pData 的长度, 不管传递什么数据, 单位都是字节</param>
/// <param name="nType">解析类型, 见 LYRIC_PARSE_TYPE 定义</param>
/// <returns>返回解密后的明文数据, 不使用时需要调用 lyric_free 释放</returns>
wchar_t* LYRICCALL lyric_decrypt(const void* pData, int nSize, LYRIC_PARSE_TYPE nType)
{
    wchar_t* buffer_text = nullptr;
    std::string lyric_data;
    if (_lrc_parse_get_lyric_text(pData, nSize, nType, &buffer_text))
    {
        // 转成了文本, 判断是路径还是数据, 路径的话需要读入
        if (__query(nType, LYRIC_PARSE_TYPE_PATH))
        {
            read_file(buffer_text, lyric_data);
            pData = lyric_data.c_str();     // 需要解密, 存放到这个变量里往下执行解密
            nSize = (int)lyric_data.size();
        }
        else
        {
            return buffer_text; // 不是文件路径, 那就是明文数据, 直接返回
        }
    }
    if (nSize < 4 || nSize > MAXINT)
        return nullptr;
    int lyric_type = (nType & 0x0f);
    bool bRet = false;
    switch (lyric_type)
    {
    case LYRIC_PARSE_TYPE_KRC:
        bRet = _lrc_decrypt_krc(pData, nSize, &buffer_text);
        break;
    case LYRIC_PARSE_TYPE_QRC:
        bRet = _lrc_decrypt_qrc(pData, nSize, &buffer_text);
        break;
    case LYRIC_PARSE_TYPE_LRC:
        bRet = _lrc_decrypt_lrc(pData, nSize, &buffer_text);
        break;
    default:
        return nullptr;   // 不支持的歌词类型
    }

    return buffer_text;
}

/// <summary>
/// 销毁 lyric_parse() 返回的歌词句柄
/// </summary>
/// <param name="hLyric">lyric_parse() 返回的歌词句柄</param>
/// <returns>无返回值</returns>
void LYRICCALL lyric_destroy(HLYRIC pData)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)pData;
    delete pLyric;
}

/// <summary>
/// 释放文本指针, 函数如果有写需要释放的, 就需要调用这个函数释放, 没写需要释放就不需要调用
/// </summary>
/// <param name="pStr">要释放的文本指针</param>
/// <returns>无返回值</returns>
void LYRICCALL lyric_free(void* pStr)
{
    return free(pStr);
}


/// <summary>
/// 计算歌词文字的宽度, 调用 lyric_calc() 函数后会调用这里传递的回调函数来计算歌词占用宽度
/// 每次计算都是计算这一行所有字的宽度, 用来确定歌词高亮位置
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="pfnCalcText">可以为空指针, 这个是个回调函数, 用来计算文字的占用宽度, 根据文字宽度确定高亮位置</param>
/// <param name="pUserData">传递到 pfnCalcText 里的用户数据</param>
/// <returns>返回是否处理成功</returns>
bool LYRICCALL lyric_calc_text(HLYRIC hLyric, LYRIC_PARSE_CALCTEXT pfnCalcText, void* pUserData)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;

    if (!pLyric)
        return false;

    pLyric->pfnCalcText = pfnCalcText;
    pLyric->pUserData = pUserData;
    return lyric_re_calc_text(hLyric);
}

/// <summary>
/// 重新计算歌词文字的宽度, 调用这个函数会重新计算歌词占用宽度, 用来确定歌词高亮位置, 一般是字体被改变时调用
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <returns>返回是否处理成功</returns>
bool LYRICCALL lyric_re_calc_text(HLYRIC hLyric)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;

    if (!pLyric)
        return false;

    // 清除所有记录的字宽度, 每一行都清零, 计算歌词位置的时候值为0会重新计算
    for (auto& line : pLyric->lines)
        line.width = 0;

    return true;
}

/// <summary>
/// 计算指定时间是在歌词的哪一行哪一个字上
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="time">要查询的时间, 单位是毫秒</param>
/// <param name="pRet">参考返回的数据, 返回歌词行文本, 歌词字文本, 行索引, 字索引</param>
/// <returns>返回是否获取成功</returns>
bool LYRICCALL lyric_calc(HLYRIC hLyric, int time, LYRIC_CALC_STRUCT* pRet)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;

    if (!pRet || !pLyric)
        return false;

    time += pLyric->nTimeOffset;
    int size = (int)pLyric->lines.size();
    if (!size)
        return false;

    int index = _lrc_find_line(pLyric, time);
    if (index > size || index < 0)
    {
        // 时间在第一行开始之前, 这里应该把高亮位置指向0, 当前位置指向第一行
        pLyric->index = 0;
    }
    else if (index == size)
    {
        // 时间在最后一行结束之后, 这里应该把高亮位置指向最后一行, 当前位置指向最后一行
        pLyric->index = size - 1;
    }
    else
    {
        // 指定时间指向了某一行歌词, 找到是第几个字
        pLyric->index = index;
    }

    auto& line = pLyric->lines[pLyric->index];

    // 找到是第几个字
    int index_word = _lrc_find_word(line, time - line.start);

    pRet->indexLine     = pLyric->index;
    pRet->indexWord     = index_word;
    pRet->nLineCount    = size;

    lyric_get_line(hLyric, pRet->indexLine, &pRet->line);
    lyric_get_word(hLyric, pRet->indexLine, pRet->indexWord, &pRet->word);

    if (!line.words.empty())
    {
        auto& word = line.words[index_word];
        if (word.width > 0 && word.duration > 0)
        {
            // 下来这里是字的高亮宽度, 先计算时间百分比, 然后乘以字的宽度
            int nStartTime = 0;
            float nEndTime = (float)(word.duration);
            float nNowTime = (float)(time - line.start - word.start);

            float bili = word.width;
            float percentage = (nNowTime * bili + nEndTime / 2) / nEndTime;
            float result = (word.width * percentage + bili / 2) / bili;
            if (result > bili)
                result = bili;

            pRet->nWidthWord = result;

            bili = word.height;
            percentage = (nNowTime * bili + nEndTime / 2) / nEndTime;
            result = (word.height * percentage + bili / 2) / bili;
            if (result > bili)
                result = bili;
            pRet->nHeightWord = result;


        }
    }
    return true;
}

/// <summary>
/// 歌词提前/延后, 正数是提前, 负数是延后
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="nTime">要提前或者延后的时间, 正数是提前, 负数是延后, 单位是毫秒</param>
/// <returns>返回设置后延时的毫秒数, 如果参数传递0则为获取</returns>
int LYRICCALL lyric_behind_ahead(HLYRIC hLyric, int nTime)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;

    if (!pLyric)
        return 0;

    pLyric->nTimeOffset += nTime;
    return pLyric->nTimeOffset;
}

/// <summary>
/// 获取歌词里记录的各种基础信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="pRet">接收返回数据的结构指针, 如果需要返回歌词行文本或者行信息, 需要分配缓冲区, 然后传递进来</param>
/// <returns>返回是否获取成功</returns>
bool LYRICCALL lyric_get_info(HLYRIC hLyric, PLYRIC_INFO_STRUCT pRet)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric && pRet)
    {
        pRet->id = pLyric->id;
        pRet->ar = pLyric->ar;
        pRet->ti = pLyric->ti;
        pRet->by = pLyric->by;
        pRet->hash = pLyric->hash;
        pRet->al = pLyric->al;
        pRet->sign = pLyric->sign;
        pRet->qq = pLyric->qq;
        pRet->total = pLyric->total;
        pRet->offset = pLyric->offset;

        // 有需要获取歌词行的才循环获取
        if (pRet->nArrayStrBuffer || pRet->nArrayLineInfoBuffer)
        {
            int nCount = (int)pLyric->lines.size();
            int nArrayStrBuffer = 0, nArrayLineInfoBuffer = 0;
            for (int i = 0; i < nCount; i++)
            {
                auto& line = pLyric->lines[i];
                if (pRet->nArrayStrBuffer && pRet->pArrayStrBuffer)
                {
                    // 需要获取文本行, 写入缓冲区
                    if (nArrayStrBuffer < pRet->nArrayStrBuffer)
                        pRet->pArrayStrBuffer[nArrayStrBuffer++] = line.text.c_str();
                }

                if (pRet->nArrayLineInfoBuffer && pRet->pArrayLineInfoBuffer)
                {
                    // 需要获取行信息, 写入缓冲区
                    if (nArrayLineInfoBuffer < pRet->nArrayLineInfoBuffer)
                    {
                        auto& lineInfo = pRet->pArrayLineInfoBuffer[nArrayLineInfoBuffer];
                        lineInfo.pText = line.text.c_str();
                        lineInfo.nStart = line.start;
                        lineInfo.nEnd = line.start + line.duration;
                        lineInfo.nWordCount = (int)line.words.size();
                    }
                    nArrayLineInfoBuffer++;
                }
                pRet->nArrayStrBuffer = nArrayStrBuffer;
                pRet->nArrayLineInfoBuffer = nArrayLineInfoBuffer;
            }
        }
        return true;
    }
    return false;
}

/// <summary>
/// 获取歌词行数
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <returns>返回行数, 没有返回0</returns>
int LYRICCALL lyric_get_line_count(HLYRIC hLyric)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric)
        return (int)pLyric->lines.size();
    return 0;
}

/// <summary>
/// 获取某一行歌词字数, 英文歌词是单词数
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <returns>返回字数, 没有返回0</returns>
int LYRICCALL lyric_get_word_count(HLYRIC hLyric, int indexLine)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric)
    {
        if (indexLine >= 0 && indexLine < (int)pLyric->lines.size())
            return (int)pLyric->lines[indexLine].words.size();
    }
    return 0;
}

/// <summary>
/// 获取歌词行信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="pRet">返回的歌词行信息</param>
/// <returns>返回是否获取成功</returns>
bool LYRICCALL lyric_get_line(HLYRIC hLyric, int indexLine, PLYRIC_LINE_STRUCT pRet)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric && pRet)
    {
        memset(pRet, 0, sizeof(*pRet));
        pRet->pText = L"";
        pRet->pTranslate1 = L"";
        pRet->pTranslate2 = L"";

        std::vector<std::wstring>* pTranslate1 = nullptr, * pTranslate2 = nullptr;
        pRet->nType = lyric_get_language(hLyric);

        if (indexLine >= 0 && indexLine < (int)pLyric->lines.size())
        {
            auto& line = pLyric->lines[indexLine];
            auto* line_fy = (__query(pRet->nType, LYRIC_LANGUAGE_TYPE_FY) && !pLyric->lines_fy.empty()) ? &pLyric->lines_fy[indexLine] : nullptr;
            auto* line_yy = (__query(pRet->nType, LYRIC_LANGUAGE_TYPE_YY) && !pLyric->lines_yy.empty()) ? &pLyric->lines_yy[indexLine] : nullptr;

            if (line.width == 0 && pLyric->pfnCalcText)
            {
                // 这一行没有计算字宽度, 这里计算一下
                float left = 0.f, top = 0.f;
                for (auto& word : line.words)
                {
                    if (word.size > 0)
                        word.width = pLyric->pfnCalcText(pLyric->pUserData, word.text, word.size, &word.height);
                    word.left = left;
                    word.top = top;
                    left += word.width;
                    top += word.height;
                }
                line.width = left;
                line.height = top;
            }


            pRet->pText = line.text.c_str();
            pRet->nLength = (int)line.size;

            if (line_fy)
                pRet->pTranslate1 = line_fy->text.c_str(), pRet->nTranslate1 = line_fy->size;
            if (line_yy)
                pRet->pTranslate2 = line_yy->text.c_str(), pRet->nTranslate2 = line_yy->size;

            pRet->nInterval = line.interval;
            pRet->nStart = line.start;
            pRet->nEnd = line.start + line.duration;
            pRet->nWordCount = (int)line.words.size();
            pRet->nWidth = line.width;
            pRet->nHeight = line.height;

            return true;
        }
    }
    return false;
}

/// <summary>
/// 获取某一行歌词里指定字的信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="indexWord">歌词字索引</param>
/// <param name="pRet">返回的歌词字信息</param>
/// <returns>返回是否获取成功</returns>
bool LYRICCALL lyric_get_word(HLYRIC hLyric, int indexLine, int indexWord, PLYRIC_WORD_STRUCT pRet)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric && pRet)
    {
        memset(pRet, 0, sizeof(*pRet));
        pRet->pText = L"";
        if (indexLine >= 0 && indexLine < (int)pLyric->lines.size())
        {
            auto& line = pLyric->lines[indexLine];
            if (indexWord >= 0 && indexWord < (int)line.words.size())
            {
                auto& word = line.words[indexWord];
                pRet->pText = word.text;
                pRet->nLength = word.size;
                pRet->nStart = word.start;
                pRet->nEnd = word.start + word.duration;

                pRet->nLeft = word.left;
                pRet->nTop = word.top;
                pRet->nWidth = word.width;
                pRet->nHeight = word.height;
                return true;
            }
        }
    }
    return false;
}

/// <summary>
/// 获取所有歌词行信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="pArrayBuffer">接收歌词信息的数组缓冲区, 需要获取字数可以调用 lyric_get_line_count() 获取</param>
/// <param name="nBufferCount">表示 pArrayBuffer 数组有多少个成员</param>
/// <returns>返回写入缓冲区的成员数</returns>
int LYRICCALL lyric_get_all_line(HLYRIC hLyric, PLYRIC_LINE_STRUCT pArrayBuffer, int nBufferCount)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric && pArrayBuffer && nBufferCount > 0)
    {
        int nCount = (int)pLyric->lines.size();
        if (nCount > nBufferCount)
            nCount = nBufferCount;
        for (int i = 0; i < nCount; i++)
        {
            auto& line = pLyric->lines[i];
            pArrayBuffer[i].pText = line.text.c_str();
            pArrayBuffer[i].nStart = line.start;
            pArrayBuffer[i].nEnd = line.start + line.duration;
            pArrayBuffer[i].nWordCount = (int)line.words.size();
        }
        return nCount;
    }
    return 0;
}

/// <summary>
/// 获取某一行歌词里所有字的信息
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="pArrayBuffer">接收字信息的数组缓冲区, 需要获取字数可以调用 lyric_get_word_count() 获取</param>
/// <param name="nBufferCount">表示 pArrayBuffer 数组有多少个成员</param>
/// <returns>返回写入缓冲区的成员数</returns>
int LYRICCALL lyric_get_all_word(HLYRIC hLyric, int indexLine, PLYRIC_WORD_STRUCT pArrayBuffer, int nBufferCount)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric && pArrayBuffer && nBufferCount > 0)
    {
        if (indexLine >= 0 && indexLine < (int)pLyric->lines.size())
        {
            auto& line = pLyric->lines[indexLine];
            int nCount = (int)line.words.size();
            if (nCount > nBufferCount)
                nCount = nBufferCount;
            for (int i = 0; i < nCount; i++)
            {
                auto& word = line.words[i];
                pArrayBuffer[i].pText = word.text;
                pArrayBuffer[i].nStart = word.start;
                pArrayBuffer[i].nEnd = word.start + word.duration;
            }
            return nCount;
        }
    }
    return 0;
}

/// <summary>
/// 获取歌词行文本
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <returns>返回歌词文本, 失败或者没有则返回空文本指针</returns>
const wchar_t* LYRICCALL lyric_get_line_str(HLYRIC hLyric, int indexLine)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric)
    {
        if (indexLine >= 0 && indexLine < (int)pLyric->lines.size())
            return pLyric->lines[indexLine].text.c_str();
    }
    return L"";
}

/// <summary>
/// 获取某一行歌词里指定字的文本
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="indexWord">歌词字索引</param>
/// <returns>返回字文本, 失败或者没有则返回空文本指针</returns>
const wchar_t* LYRICCALL lyric_get_word_str(HLYRIC hLyric, int indexLine, int indexWord)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric)
    {
        if (indexLine >= 0 && indexLine < (int)pLyric->lines.size())
        {
            auto& line = pLyric->lines[indexLine];
            if (indexWord >= 0 && indexWord < (int)line.words.size())
                return line.words[indexWord].text;
        }
    }
    return L"";
}

/// <summary>
/// 获取某一行歌词所有字, 需要在外部分配足够的空间存放歌词文本数组
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="indexLine">歌词行索引</param>
/// <param name="pArrayBuffer">接收字的数组缓冲区, 需要获取字数可以调用 lyric_get_word_count() 获取</param>
/// <param name="nBufferCount">表示 pArrayBuffer 数组有多少个成员</param>
/// <returns>返回写入数组的成员数</returns>
int LYRICCALL lyric_get_word_all_str(HLYRIC hLyric, int indexLine, const wchar_t** pArrayBuffer, int nBufferCount)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric && pArrayBuffer && nBufferCount > 0)
    {
        if (indexLine >= 0 && indexLine < (int)pLyric->lines.size())
        {
            auto& line = pLyric->lines[indexLine];
            int nCount = (int)line.words.size();
            if (nCount > nBufferCount)
                nCount = nBufferCount;
            for (int i = 0; i < nCount; i++)
                pArrayBuffer[i] = line.words[i].text;
            return nCount;
        }
    }
    return 0;
}

/// <summary>
/// 获取所有歌词行文本, 需要在外部分配足够的空间存放歌词文本数组
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <param name="pArrayBuffer">接收歌词的数组缓冲区, 需要获取歌词行数可以调用 lyric_get_line_count() 获取</param>
/// <param name="nBufferCount">表示 pArrayBuffer 数组有多少个成员</param>
/// <returns>返回写入数组的成员数</returns>
int LYRICCALL lyric_get_line_all_str(HLYRIC hLyric, const wchar_t** pArrayBuffer, int nBufferCount)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric && pArrayBuffer && nBufferCount > 0)
    {
        int nCount = (int)pLyric->lines.size();
        if (nCount > nBufferCount)
            nCount = nBufferCount;
        for (int i = 0; i < nCount; i++)
            pArrayBuffer[i] = pLyric->lines[i].text.c_str();
        return nCount;
    }
    return 0;
}

/// <summary>
/// 把歌词转换成lrc格式, lrc格式的歌词是没有逐字的, 都是一行一行的, 返回的指针需要调用 lyric_free() 释放
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <returns>返回歌词文本指针, 失败返回0, 不使用时需要调用 lyric_free() 释放</returns>
wchar_t* LYRICCALL lyric_to_lrc(HLYRIC hLyric)
{
    if (!hLyric)
        return nullptr;

    /*
    *
        [al:专辑名]
        [ar:歌手名]
        [au:歌词作者-作曲家]
        [by:此LRC文件的创建者]
        [offset:+/- 时间补偿值，以毫秒为单位，正值表示加快，负值表示延后]
        [re:创建此LRC文件的播放器或编辑器]
        [ti:歌词(歌曲)的标题]
        [ve:程序的版本]
    *
    */

    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    std::wstring lrc;
    lrc.reserve(pLyric->lines.size() * 30); // 假设每行有30个字符

    lrc.append(L"[al:").append(pLyric->al).append(L"]\r\n");
    lrc.append(L"[ar:").append(pLyric->ar).append(L"]\r\n");
    lrc.append(L"[by:").append(pLyric->by).append(L"]\r\n");
    lrc.append(L"[ti:").append(pLyric->ti).append(L"]\r\n");
    lrc.append(L"[offset:").append(pLyric->offset).append(L"]\r\n");

    wchar_t szTime[40] = { 0 };

    for (auto& line : pLyric->lines)
    {
        int s = line.start / 1000;
        int m = s / 60;
        int ms = line.start % 1000 / 10;
        swprintf_s(szTime, L"[%02d:%02d.%02d]", m, s % 60, ms);
        lrc.append(szTime).append(line.text).append(L"\r\n");
    }

    if (!lrc.empty())
    {
        size_t len = lrc.size() + 1;
        auto ret = (wchar_t*)malloc(len * sizeof(wchar_t));
        if (ret)
            memcpy(ret, lrc.c_str(), len * sizeof(wchar_t));
        return ret;
    }

    return nullptr;
}

/// <summary>
/// 获取歌词语言信息, 翻译歌词使用
/// </summary>
/// <param name="hLyric">歌词句柄</param>
/// <returns>返回歌词语言, 0=没有翻译, 1=翻译, 2=音译, 3=翻译+音译, 其他待定</returns>
int LYRICCALL lyric_get_language(HLYRIC hLyric)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    int ret = 0;
    if (pLyric)
    {
        ret = pLyric->language;
    }
    return ret;
}

