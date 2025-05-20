#include "lyric_typedef.h"
using namespace LYRIC_NAMESPACE;

// 内部使用, 根据解析酷狗歌词的type确定歌词是否有翻译/音译, 1=翻译, 2=音译
int _lyric_get_type_kg(int type);

void LYRICCALL lyric_free(void* pStr)
{
    return free(pStr);
}

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
                    if(nArrayStrBuffer < pRet->nArrayStrBuffer)
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


int LYRICCALL lyric_get_line_count(HLYRIC hLyric)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric)
        return (int)pLyric->lines.size();
    return 0;
}

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

bool LYRICCALL lyric_get_line(HLYRIC hLyric, int indexLine, PLYRIC_LINE_STRUCT pRet)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric && pRet)
    {
        memset(pRet, 0, sizeof(*pRet));
        pRet->pText = L"";
        pRet->pTranslate1 = L"";
        pRet->pTranslate2 = L"";

        std::vector<std::wstring>* pTranslate1 = nullptr, *pTranslate2 = nullptr;
        size_t language_size = pLyric->language.size();
        if (language_size > 2)
            __debugbreak(); // 一般就一个翻译, 一个音译, 暂时没见过超过两个的, 超过的话断下查看一些

        pRet->nType = lyric_get_language(hLyric);
        if (language_size > 0)
        {
            for (size_t i = 0; i < language_size; i++)
            {
                auto& item = pLyric->language[i];
                if (!item.lines.empty())
                {
                    // type解析出来 0=音译, 1=翻译
                    // 记录到变量里0=没有, 1=翻译, 2=音译, 3=两者都有
                    if (item.type == 0)
                        pTranslate2 = &item.lines;
                    else if (item.type == 1)
                        pTranslate1 = &item.lines;
                }
            }
        }
        if (indexLine >= 0 && indexLine < (int)pLyric->lines.size())
        {
            auto& line = pLyric->lines[indexLine];

            if (line.width == 0 && pLyric->pfnCalcText)
            {
                // 这一行没有计算字宽度, 这里计算一下
                float left = 0.f, top = 0.f;
                for (auto& word : line.words)
                {
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
            pRet->nLength = (int)line.text.size();

            if (pTranslate1)
                pRet->pTranslate1 = (*pTranslate1)[indexLine].c_str(), pRet->nTranslate1 = (int)(*pTranslate1)[indexLine].size();
            if (pTranslate2)
                pRet->pTranslate2 = (*pTranslate2)[indexLine].c_str(), pRet->nTranslate2 = (int)(*pTranslate2)[indexLine].size();;

            pRet->nInterval     = line.interval;
            pRet->nStart        = line.start;
            pRet->nEnd          = line.start + line.duration;
            pRet->nWordCount    = (int)line.words.size();
            pRet->nWidth        = line.width;
            pRet->nHeight       = line.height;

            return true;
        }
    }
    return false;
}

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
                pRet->pText     = word.text;
                pRet->nLength   = word.size;
                pRet->nStart    = word.start;
                pRet->nEnd      = word.start + word.duration;

                pRet->nLeft     = word.left;
                pRet->nTop      = word.top;
                pRet->nWidth    = word.width;
                pRet->nHeight   = word.height;
                return true;
            }
        }
    }
    return false;
}

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
        swprintf_s(szTime, L"[%02d:%02d.%02d]", m, s, ms);
        lrc.append(szTime).append(line.text).append(L"\r\n");
    }

    if (!lrc.empty())
    {
        size_t len = lrc.size() + 1;
        auto ret = (wchar_t*)malloc(len * 2);
        memcpy(ret, lrc.c_str(), len * 2);
        return ret;
    }

    return nullptr;
}

int LYRICCALL lyric_get_language(HLYRIC hLyric)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    int ret = 0;
    if (pLyric)
    {
        for (auto& item : pLyric->language)
        {
            if (!item.lines.empty())
                ret |= _lyric_get_type_kg(item.type);
        }
    }
    return ret;
}

int _lyric_get_type_kg(int type)
{
    if (type == 0)
        return 2;
    if (type == 1)
        return 1;
    return 0;
}