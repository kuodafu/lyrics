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

template<typename T, typename R>inline bool __query(T l, R r)
{
    return ((R)l & r) == r;
}

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
    int                 size;       // ��һ���еĸ�����ݵ��ַ���, ������������, ����ʹ�� text.size(), �����ʻ���3�����ȵ��ı�
    std::wstring        text;       // ��һ���еĸ������
    INSIDE_LYRIC_WORDS  words;      // ÿһ���ֵĽṹ

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
    LPWSTR                  krc;        // ���ܺ��krc����, �������Ѻܶ����ַ��ĳ�0
    INSIDE_LYRIC_LINDS      lines;      // ÿһ�еĽṹ
    INSIDE_LYRIC_LINDS      lines_yy;   // ����, ÿһ�еĽṹ, ����һ���Ǻ͸������һ��, ����Ҳ�����ֵ�
    INSIDE_LYRIC_LINDS      lines_fy;   // ����, ÿһ�еĽṹ, ����һ�����һ��һ�м�¼��
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
    int     language;       // ��ǰ��ʴ�����Щ����, LYRIC_LANGUAGE_TYPE ö��ֵ
    int     lyric_type;     // ��ǰ�����ĸ������, LYRIC_PARSE_TYPE ���4λ��ֵ
    int     index;          // ��ʸ�������, ��ǰ���ڵڼ���, ����ʱ���������, ���������������ȥ��������
    int     nTimeOffset;    // ʱ��ƫ��, ������λ�õ�ʱ��������ƫ��
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

// �����ļ�����, ���ض�ȡ�ĳߴ�
int read_file(LPCWSTR file, std::string& ret);

// zlib��ѹ, �����Ƿ��ѹ�ɹ�
bool zlib_decompress(const void* compressedData, size_t compressedSize, std::string& output);


// ͨ����־λ��ȡpDataָ����ı�����, ���жϱ���ת��UTF16�ַ���, ����pData�Ƿ�ָ���ı�
// �������false, �Ǿͱ�׼pData����ָ���ı�����, ����ָ��ʵ������, ��Ҫ���ݱ�־λ������
bool _lrc_parse_get_lyric_text(const void* pData, size_t nSize, LYRIC_PARSE_TYPE nType, wchar_t** ret);

// ����ʱ����ҵ�ǰʱ���ڵڼ��и����
// ���ʱ�䳬��ĳһ�еĽ���, ����С����һ�еĿ�ʼ, �Ƿ��ص���ĳһ��
// ���� ʱ����� ��5�еĽ���ʱ��, С�ڵ�6�еĿ�ʼʱ��, �Ƿ��ص��ǵ�5��
int _lrc_find_line(PINSIDE_LYRIC_INFO pLyric, int time);

// ����ʱ��������ڵڼ���������
// ���ʱ����� ��5���ֵĽ���ʱ��, С�ڵ�6���ֵĿ�ʼʱ��, �Ƿ��ص��ǵ�5����
int _lrc_find_word(INSIDE_LYRIC_LINE& line, int time);

// �Ѽ��ܵĿṷ������ݽ��ܳ�����
bool _lrc_decrypt_krc(const void* pData, size_t nSize, wchar_t** ppLyricText);
// �Ѽ��ܵ�QQ���ָ�����ݽ��ܳ�����
bool _lrc_decrypt_qrc(const void* pData, size_t nSize, wchar_t** ppLyricText);
// ����lrc, һ��lrcû����, ����ֱ�ӿ���һ�ݷ���
bool _lrc_decrypt_lrc(const void* pData, size_t nSize, wchar_t** ppLyricText);

// �����ṷ���, �����Ƿ�����ɹ�, �����Ľ�������浽pLyric��
bool _lrc_parse_krc(PINSIDE_LYRIC_INFO pLyric);
// ����QQ���ָ��, �����Ƿ�����ɹ�, �����Ľ�������浽pLyric��
bool _lrc_parse_qrc(PINSIDE_LYRIC_INFO pLyric);
// ����lrc���, �����Ƿ�����ɹ�, �����Ľ�������浽pLyric��
bool _lrc_parse_lrc(PINSIDE_LYRIC_INFO pLyric);

// ����typeȷ������Ƿ��з���/����, 1=����, 2=����
int _lrc_get_language(PINSIDE_LYRIC_INFO pLyric, int type);

// ���һ���ּ���ʱ������ʾ, ������
void _lrc_dbg_append_interval_time(PINSIDE_LYRIC_INFO pLyric, size_t back_index);


LYRIC_NAMESPACE_END

