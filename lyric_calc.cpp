/*
*   本文件主要是处理计算歌词位置
*   传递一个时间, 通过时间计算出歌词的位置
*   然后返回一个百分比, 调用方可以在接收到这个百分比后进行处理
*   这里的百分比只是一个描述, 实际上比例是自己设置的
*   按100份处理的话, 1/100可能会占用好几十个像素
*   为了让歌词滚动能更顺畅, 肯定会用千分比或者万分比
*/
#include "lyric_typedef.h"
#include <algorithm>

LYRIC_NAMESPACE_BEGIN
int lyric_find_line(PINSIDE_LYRIC_INFO pLyric, int time);

// 根据时间查找是在第几个字里面
int lyric_find_word(INSIDE_LYRIC_LINE& line, int time);




LYRIC_NAMESPACE_END

// 设置计算歌词回调函数, 记录回调前先清除所有记录的字宽度
bool LYRICCALL lyric_calc_text_width(HLYRIC hLyric, LYRIC_PARSE_CALCTEXT pfnCalcText, void* pUserData)
{
    using namespace LYRIC_NAMESPACE;
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;

    if (!pLyric)
        return false;

    pLyric->pfnCalcText = pfnCalcText;
    pLyric->pUserData = pUserData;

    // 清除所有记录的字宽度, 每一行每一个字都清零
    for (auto& line : pLyric->lines)
        line.width = 0;

    return false;
}



bool LYRICCALL lyric_calc(HLYRIC hLyric, int time, LYRIC_CALC_STRUCT* pRet)
{
    using namespace LYRIC_NAMESPACE;
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;

    if (!pRet || !pLyric)
        return false;

    int size = (int)pLyric->lines.size();
    int index = lyric_find_line(pLyric, time);
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
    int index_word = lyric_find_word(line, time - line.start);
    auto& word = line.words[index_word];

    if (line.width == 0 && pLyric->pfnCalcText)
    {
        // 这一行没有计算字宽度, 这里计算一下
        int left = 0;
        for (auto& item_word : line.words)
        {
            item_word.width = pLyric->pfnCalcText(pLyric->pUserData, item_word.text, item_word.size, &item_word.height);
            item_word.left = left;
            left += item_word.width;
        }
        line.width = left;
    }

    pRet->indexLine     = pLyric->index;
    pRet->indexWord     = index_word;

    lyric_get_line(hLyric, pRet->indexLine, &pRet->line);
    lyric_get_word(hLyric, pRet->indexLine, pRet->indexWord, &pRet->word);

    if (word.width > 0)
    {
        // 下来这里是字的高亮宽度, 先计算时间百分比, 然后乘以字的宽度
        int nStartTime = 0;
        int nEndTime = word.duration ? word.duration : 1;
        int nNowTime = time - line.start - word.start;

        int bili = word.width;
        int percentage = (nNowTime * bili + nEndTime / 2) / nEndTime;
        int result = (word.width * percentage + bili / 2) / bili;
        if (result > bili)
            result = bili;

        pRet->nWidthWord += result;

    }

    return true;
}




LYRIC_NAMESPACE_BEGIN


// 二分查找, 传递起始位置和结束位置, 循环查找
template<typename _Pr>
static int binarySearchGreaterThan(int l, int r, _Pr _Pred)
{
    r--;
    while (l <= r)
    {
        int m = l + (r - l) / 2;
        int cmp = _Pred(m);
        if (cmp == 0)
            return m;   // 两个成员相等, 找到了, 直接返回索引

        // 如果中间元素大于目标值, 则在右半部分查找
        if (cmp > 0)
            l = m + 1;  // 在右半部分查找
        else
            r = m - 1;  // 在左半部分查找
    }
    return -1;
}

template<typename _Ty>
static int _cmp_function(_Ty& arr, int size, int m, int time)
{
    // 如果 time 在 line范围内, 那就返回0, 表示找到了
        // 如果 time 大于 line的结束时间, 那就返回 正整数, 表示要在右边查找
        // 如果 time 小于 line的开始时间, 那就返回 负整数, 表示要在左边查找
    auto& item = arr[m];
    if (time >= item.start && time < (item.start + item.duration))
        return 0;   // time是在 line范围内, 找到了

    if (time >= (item.start + item.duration))
    {
        // 这里还有另一种情况, 那就是时间是大于当前行, 但是小于下一行, 就是在两行中间
        // 这种情况直接返回0
        // 这里拿到下一个成员, 判断时间是否小于开始时间
        if (m + 1 < size)
        {
            auto& next = arr[m + 1];
            if (time < next.start)
                return 0;
        }
        return 1;   // time大于 line的结束时间, 要在右边查找
    }

    // 如果小于这一行的开始时间, 那还有一种情况是大于上一行的结束时间
    // 这种也是在两行中间, 也需要处理一下
    //if (m - 1 >= 0 && m - 1 < size)
    //{
    //    auto& prev = arr[m - 1];
    //    if (time > prev.start + prev.duration)
    //        return 0;
    //}
    
    return -1;  // time小于 line的开始时间, 要在左边查找
}

/// <summary>
/// 查找时间对应的行号, 返回行号索引
/// </summary>
/// <param name="pLyric"></param>
/// <param name="time"></param>
/// <returns>返回值有3种, -1表示时间小于第一行, 返回size表示时间大于最后一行, >-1 < size 表示找到的位置</returns>
int lyric_find_line(PINSIDE_LYRIC_INFO pLyric, int time)
{
    // 先判断是不是比开始小, 然后再判断是不是比结束大, 是的话就返回
    int size = (int)pLyric->lines.size();
    if (size == 0)
        return -1;

    auto& begin = pLyric->lines.front();
    auto& end = pLyric->lines.back();
    if (time < begin.start)
        return -1;  // 小于第一行, 返回-1

    if (time >= end.start + end.duration)
        return size;    // 大于最后一行, 返回最后一行下标+1

    int cmp = 0;    // 要是找不到的时候, 根据这个值判断返回-1, 还是返回size

    auto pfn_cmp = [time, &pLyric, &cmp, size](int mid)
    {
        cmp = _cmp_function(pLyric->lines, size, mid, time);
        return cmp;
    };

    int index = -1;
    int left = 0, right = size;
    // 上次有搜索过, 尝试从这个索引开始查找
    if (pLyric->index >= 0 && pLyric->index < size)
    {
        if (pfn_cmp(pLyric->index) == 0)
            return pLyric->index;   // 在当前记录的行找到了

        if (cmp > 0)    // 找了一次没找到, 那就设置搜索数组时的左边位置和右边位置
            left = pLyric->index + 1;
        else
            right = pLyric->index - 1;

        // 如果当前行找不到, 并且目标位置在右边, 那就尝试下一行, 不行就搜索整个数组
        // 因为大部分情况下是逐行递增的, 除非中途调整位置
        // 不然多做这两步操作能快不少
        if (cmp > 0 && pLyric->index < size - 1)
        {
            if (pfn_cmp(left) == 0)
                return left;
            
            // 找了第二次, 还是找不到, 那就继续设置搜索范围
            if (cmp > 0)
                left++;   // 搜索的范围大于 index+1 的位置, 那就是要搜索右边, 左边设置为 index+2
            else
                right--;
        }
        if (left >= size || right > size)
            return cmp > 0 ? size : -1;

    }

    if (index == -1)    // 上面两步没有搜索到, 那就用二分查找整个数组
        index = binarySearchGreaterThan(left, right, pfn_cmp);

    if (index == -1)
        __debugbreak(); // 走到这的就是bug, 前面已经处理了小于第一行和大于最后一行, 这里还是-1, 那就是搜索数组没搜索到
    return index;
}

int lyric_find_word(INSIDE_LYRIC_LINE& line, int time)
{
    // 先判断是不是比开始小, 然后再判断是不是比结束大, 是的话就返回
    int size = (int)line.words.size();
    if (size == 0)
        return 0;

    auto& begin = line.words.front();
    auto& end = line.words.back();

    if (time < begin.start)
        return 0;  // 小于第一行, 返回0

    if (time >= end.start + end.duration)
        return size - 1;    // 大于最后一个字, 返回最后一个字的下标

    int left = 0, right = (int)line.words.size();
    int index = binarySearchGreaterThan(left, right, [time, size, &line](int mid)
    {
        return _cmp_function(line.words, size, mid, time);
    });
    if (index == -1)
    {
        __debugbreak(); // bug, 能走到这里的index不可能为-1
        index = 0;
    }
    return index;
}


LYRIC_NAMESPACE_END
