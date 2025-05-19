#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include "kuodafu_lyric.h"

#define LYRIC_NAMESPACE lyric
#define LYRIC_NAMESPACE_BEGIN namespace LYRIC_NAMESPACE {
#define LYRIC_NAMESPACE_END }

LYRIC_NAMESPACE_BEGIN

// һ���ֵĽṹ, ��¼����ֵĿ�ʼʱ��, ����ʱ��, �������
struct INSIDE_LYRIC_WORD
{
    int     start;      // ��ʼʱ���ǻ�����һ��ʱ��Ŀ�ʼ
    int     duration;   // ����ֳ�����ʱ��
    int     t3;         // ��ʱ��֪�����ֵ�Ǹ����, ����һ��, ������0
    float   width;      // ��һ����ռ�õ��ı����, Ϊ0����û�м���
    float   left;       // ǰ�������ֵĿ��֮��, ����ֵ���߾�
    float   top;        // ǰ�������ֵĸ߶�֮��, ����ֵĶ��߾�, ������ʹ��
    float   height;     // ��һ����ռ�õ��ı��߶�
    LPCWSTR text;       // �������, �����ָ����ָ���ʶ������ krc �ַ��������
    int     size;       // ����ֵ��ַ���
    INSIDE_LYRIC_WORD()
    {
        start = duration = t3 = size = 0;
        width = height = 0.0f;
        top = 0.f;
        text = nullptr;
        left = 0;
    }
};
using INSIDE_LYRIC_WORDS = std::vector<INSIDE_LYRIC_WORD>;

#ifndef MAXINT
#define MAXINT ((INT)(((UINT)~((UINT)0)) >> 1))
#endif

// һ�и�ʵĽṹ, ��¼��һ�еĿ�ʼʱ��, ����ʱ��, �������, �Լ�ÿһ���ֵĽṹ
struct INSIDE_LYRIC_LINE
{
    int                 start;      // ��һ�еĿ�ʼʱ��
    int                 duration;   // ��һ�г�����ʱ��
    int                 interval;   // ������һ�еļ��ʱ��, ��λ�Ǻ���, MAXINT��ʾ�����һ��
    float               width;      // ��һ�и��ռ�õ��ı����
    float               height;     // ��һ�и��ռ�õ��ı��߶�, ������ʹ��
    std::wstring        text;       // ��һ���еĸ������
    INSIDE_LYRIC_WORDS  words;      // ÿһ���ֵĽṹ

    INSIDE_LYRIC_LINE()
    {
        start = duration = 0;
        width = 0;
        height = 0;
        interval = 0;
    }
};
using INSIDE_LYRIC_LINDS = std::vector<INSIDE_LYRIC_LINE>;

// �����ʵĽṹ
struct INSIDE_LYRIC_TRANSLATE
{
    int language;   // ��ʱ��֪����ʲô, ����һ��, ������0
    int type;       // 0: ����, 1: ����
    std::vector<std::wstring> lines;    // ����/�����ʵ�ÿһ��, ����/������û�����ֵ�, ����һ��һ�м�¼��
    INSIDE_LYRIC_TRANSLATE()
    {
        language = type = 0;
    }
};
using INSIDE_LYRIC_LANGUAGE = std::vector<INSIDE_LYRIC_TRANSLATE>;

typedef struct INSIDE_LYRIC_INFO
{
    std::wstring            krc;        // ���ܺ��krc����, �������Ѻܶ����ַ��ĳ�0
    INSIDE_LYRIC_LINDS      lines;      // ÿһ�еĽṹ
    INSIDE_LYRIC_LANGUAGE   language;   // ����/����Ľṹ
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
    int     index;          // ��ʸ�������, ��ǰ���ڵڼ���, ����ʱ���������, ���������������ȥ��������
    int     nTimeOffset;    // ʱ��ƫ��, ������λ�õ�ʱ��������ƫ��
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

