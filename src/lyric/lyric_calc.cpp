/*
*   ���ļ���Ҫ�Ǵ��������λ��
*   ����һ��ʱ��, ͨ��ʱ��������ʵ�λ��
*   Ȼ�󷵻�һ���ٷֱ�, ���÷������ڽ��յ�����ٷֱȺ���д���
*   ����İٷֱ�ֻ��һ������, ʵ���ϱ������Լ����õ�
*   ��100�ݴ���Ļ�, 1/100���ܻ�ռ�úü�ʮ������
*   Ϊ���ø�ʹ����ܸ�˳��, �϶�����ǧ�ֱȻ�����ֱ�
*/
#include "lyric_typedef.h"
#include <algorithm>


LYRIC_NAMESPACE_BEGIN


// ���ֲ���, ������ʼλ�úͽ���λ��, ѭ������
template<typename _Pr>
static int binarySearchGreaterThan(int l, int r, _Pr _Pred)
{
    r--;
    while (l <= r)
    {
        int m = l + (r - l) / 2;
        int cmp = _Pred(m);
        if (cmp == 0)
            return m;   // ������Ա���, �ҵ���, ֱ�ӷ�������

        // ����м�Ԫ�ش���Ŀ��ֵ, �����Ұ벿�ֲ���
        if (cmp > 0)
            l = m + 1;  // ���Ұ벿�ֲ���
        else
            r = m - 1;  // ����벿�ֲ���
    }
    return -1;
}

template<typename _Ty>
static int _cmp_function(_Ty& arr, int size, int m, int time)
{
    // ��� time �� line��Χ��, �Ǿͷ���0, ��ʾ�ҵ���
        // ��� time ���� line�Ľ���ʱ��, �Ǿͷ��� ������, ��ʾҪ���ұ߲���
        // ��� time С�� line�Ŀ�ʼʱ��, �Ǿͷ��� ������, ��ʾҪ����߲���
    auto& item = arr[m];
    if (time >= item.start && time < (item.start + item.duration))
        return 0;   // time���� line��Χ��, �ҵ���

    if (time >= (item.start + item.duration))
    {
        // ���ﻹ����һ�����, �Ǿ���ʱ���Ǵ��ڵ�ǰ��, ����С����һ��, �����������м�
        // �������ֱ�ӷ���0
        // �����õ���һ����Ա, �ж�ʱ���Ƿ�С�ڿ�ʼʱ��
        if (m + 1 < size)
        {
            auto& next = arr[m + 1];
            if (time < next.start)
                return 0;
        }
        return 1;   // time���� line�Ľ���ʱ��, Ҫ���ұ߲���
    }

    // ���С����һ�еĿ�ʼʱ��, �ǻ���һ������Ǵ�����һ�еĽ���ʱ��
    // ����Ҳ���������м�, Ҳ��Ҫ����һ��
    //if (m - 1 >= 0 && m - 1 < size)
    //{
    //    auto& prev = arr[m - 1];
    //    if (time > prev.start + prev.duration)
    //        return 0;
    //}
    
    return -1;  // timeС�� line�Ŀ�ʼʱ��, Ҫ����߲���
}

/// <summary>
/// ����ʱ���Ӧ���к�, �����к�����
/// </summary>
/// <param name="pLyric"></param>
/// <param name="time"></param>
/// <returns>����ֵ��3��, -1��ʾʱ��С�ڵ�һ��, ����size��ʾʱ��������һ��, >-1 < size ��ʾ�ҵ���λ��</returns>
int _lrc_find_line(PINSIDE_LYRIC_INFO pLyric, int time)
{
    // ���ж��ǲ��Ǳȿ�ʼС, Ȼ�����ж��ǲ��ǱȽ�����, �ǵĻ��ͷ���
    int size = (int)pLyric->lines.size();
    if (size == 0)
        return -1;

    auto& begin = pLyric->lines.front();
    auto& end = pLyric->lines.back();
    if (time < begin.start)
        return -1;  // С�ڵ�һ��, ����-1

    if (time >= end.start + end.duration)
        return size;    // �������һ��, �������һ���±�+1

    int cmp = 0;    // Ҫ���Ҳ�����ʱ��, �������ֵ�жϷ���-1, ���Ƿ���size

    auto pfn_cmp = [time, &pLyric, &cmp, size](int mid)
    {
        cmp = _cmp_function(pLyric->lines, size, mid, time);
        return cmp;
    };

    int index = -1;
    int left = 0, right = size;
    // �ϴ���������, ���Դ����������ʼ����
    if (pLyric->index >= 0 && pLyric->index < size)
    {
        if (pfn_cmp(pLyric->index) == 0)
            return pLyric->index;   // �ڵ�ǰ��¼�����ҵ���

        if (cmp > 0)    // ����һ��û�ҵ�, �Ǿ�������������ʱ�����λ�ú��ұ�λ��
            left = pLyric->index;
        else
            right = pLyric->index;

        // �����ǰ���Ҳ���, ����Ŀ��λ�����ұ�, �Ǿͳ�����һ��, ���о�������������
        // ��Ϊ�󲿷�����������е�����, ������;����λ��
        // ��Ȼ���������������ܿ첻��
        if (cmp > 0 && pLyric->index < size - 1)
        {
            if (pfn_cmp(left) == 0)
                return left;
            
            // ���˵ڶ���, �����Ҳ���, �Ǿͼ�������������Χ
            if (cmp > 0)
                left++;   // �����ķ�Χ���� index+1 ��λ��, �Ǿ���Ҫ�����ұ�, �������Ϊ index+2
            else
                right--;
        }
        if (left >= size || right > size)
            return cmp > 0 ? size : -1;

    }

    if (index == -1)    // ��������û��������, �Ǿ��ö��ֲ�����������
        index = binarySearchGreaterThan(left, right, pfn_cmp);

    if (index == -1)
        __debugbreak(); // �ߵ���ľ���bug, ǰ���Ѿ�������С�ڵ�һ�кʹ������һ��, ���ﻹ��-1, �Ǿ�����������û������
    return index;
}

int _lrc_find_word(INSIDE_LYRIC_LINE& line, int time)
{
    // ���ж��ǲ��Ǳȿ�ʼС, Ȼ�����ж��ǲ��ǱȽ�����, �ǵĻ��ͷ���
    int size = (int)line.words.size();
    if (size == 0)
        return 0;

    auto& begin = line.words.front();
    auto& end = line.words.back();

    if (time < begin.start)
        return 0;  // С�ڵ�һ��, ����0

    if (time >= end.start + end.duration)
        return size - 1;    // �������һ����, �������һ���ֵ��±�

    int left = 0, right = (int)line.words.size();
    int index = binarySearchGreaterThan(left, right, [time, size, &line](int mid)
    {
        return _cmp_function(line.words, size, mid, time);
    });
    if (index == -1)
    {
        __debugbreak(); // bug, ���ߵ������index������Ϊ-1
        index = 0;
    }
    return index;
}


LYRIC_NAMESPACE_END
