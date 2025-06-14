/*
* ����ļ��ǹ�����ȥ�ĺ���
* 
*/
#include "lyric_typedef.h"
using namespace LYRIC_NAMESPACE;

/// <summary>
/// �������, ����krc���ܺ������, ���ص�ָ����Ҫ���� lyric_destroy ���پ��
/// </summary>
/// <param name="pData">����, ִ��Ҫ�����ĸ������, ��ָ���ļ��������ݸ���nType����, ������ݵ��ı���BOM, ����Ա����־λ, ʹ��BOM�ı��뷽ʽ</param>
/// <param name="nSize">����, pData �ĳ���, ���ܴ���ʲô����, ��λ�����ֽ�</param>
/// <param name="nType">��������, �� LYRIC_PARSE_TYPE ����</param>
/// <returns>���ؽ��ܺ������, ��ʹ��ʱ��Ҫ���� lyric_destroy ���پ��</returns>
HLYRIC LYRICCALL lyric_parse(const void* pData, int nSize, LYRIC_PARSE_TYPE nType)
{
    if (!pData)
        return nullptr;
    wchar_t* lyric_text = lyric_decrypt(pData, nSize, nType);
    if (!lyric_text)
        return nullptr; // �����ı�ʧ��, ���ؿ�ָ��

    // �ߵ�������ǽ��ܳɹ���, ���ݸ������ȥ����
    auto pLyric = new INSIDE_LYRIC_INFO;
    bool bParse = false;
    pLyric->krc = lyric_text;   // ����Ҫ����, Ȼ������ĺ������������ı�
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
        return nullptr;   // ��֧�ֵĸ������
    }
    if (!bParse)
    {
        delete pLyric;
        return nullptr;   // ����ʧ��, ���ؿ�ָ��
    }
    return (HLYRIC)pLyric;
}

/// <summary>
/// ���ܸ��, ���ؽ��ܺ����������, ���ص�ָ����Ҫ���� lyric_free �ͷ�
/// </summary>
/// <param name="pData">����, ִ��Ҫ�����ĸ������, ��ָ���ļ��������ݸ���nType����, ������ݵ��ı���BOM, ����Ա����־λ, ʹ��BOM�ı��뷽ʽ</param>
/// <param name="nSize">����, pData �ĳ���, ���ܴ���ʲô����, ��λ�����ֽ�</param>
/// <param name="nType">��������, �� LYRIC_PARSE_TYPE ����</param>
/// <returns>���ؽ��ܺ����������, ��ʹ��ʱ��Ҫ���� lyric_free �ͷ�</returns>
wchar_t* LYRICCALL lyric_decrypt(const void* pData, int nSize, LYRIC_PARSE_TYPE nType)
{
    wchar_t* buffer_text = nullptr;
    std::string lyric_data;
    if (_lrc_parse_get_lyric_text(pData, nSize, nType, &buffer_text))
    {
        // ת�����ı�, �ж���·����������, ·���Ļ���Ҫ����
        if (__query(nType, LYRIC_PARSE_TYPE_PATH))
        {
            read_file(buffer_text, lyric_data);
            pData = lyric_data.c_str();     // ��Ҫ����, ��ŵ��������������ִ�н���
            nSize = (int)lyric_data.size();
        }
        else
        {
            return buffer_text; // �����ļ�·��, �Ǿ�����������, ֱ�ӷ���
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
        return nullptr;   // ��֧�ֵĸ������
    }

    return buffer_text;
}

/// <summary>
/// ���� lyric_parse() ���صĸ�ʾ��
/// </summary>
/// <param name="hLyric">lyric_parse() ���صĸ�ʾ��</param>
/// <returns>�޷���ֵ</returns>
void LYRICCALL lyric_destroy(HLYRIC pData)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)pData;
    delete pLyric;
}

/// <summary>
/// �ͷ��ı�ָ��, ���������д��Ҫ�ͷŵ�, ����Ҫ������������ͷ�, ûд��Ҫ�ͷžͲ���Ҫ����
/// </summary>
/// <param name="pStr">Ҫ�ͷŵ��ı�ָ��</param>
/// <returns>�޷���ֵ</returns>
void LYRICCALL lyric_free(void* pStr)
{
    return free(pStr);
}


/// <summary>
/// ���������ֵĿ��, ���� lyric_calc() �������������ﴫ�ݵĻص�������������ռ�ÿ��
/// ÿ�μ��㶼�Ǽ�����һ�������ֵĿ��, ����ȷ����ʸ���λ��
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="pfnCalcText">����Ϊ��ָ��, ����Ǹ��ص�����, �����������ֵ�ռ�ÿ��, �������ֿ��ȷ������λ��</param>
/// <param name="pUserData">���ݵ� pfnCalcText ����û�����</param>
/// <returns>�����Ƿ���ɹ�</returns>
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
/// ���¼��������ֵĿ��, ����������������¼�����ռ�ÿ��, ����ȷ����ʸ���λ��, һ�������屻�ı�ʱ����
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <returns>�����Ƿ���ɹ�</returns>
bool LYRICCALL lyric_re_calc_text(HLYRIC hLyric)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;

    if (!pLyric)
        return false;

    // ������м�¼���ֿ��, ÿһ�ж�����, ������λ�õ�ʱ��ֵΪ0�����¼���
    for (auto& line : pLyric->lines)
        line.width = 0;

    return true;
}

/// <summary>
/// ����ָ��ʱ�����ڸ�ʵ���һ����һ������
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="time">Ҫ��ѯ��ʱ��, ��λ�Ǻ���</param>
/// <param name="pRet">�ο����ص�����, ���ظ�����ı�, ������ı�, ������, ������</param>
/// <returns>�����Ƿ��ȡ�ɹ�</returns>
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
        // ʱ���ڵ�һ�п�ʼ֮ǰ, ����Ӧ�ðѸ���λ��ָ��0, ��ǰλ��ָ���һ��
        pLyric->index = 0;
    }
    else if (index == size)
    {
        // ʱ�������һ�н���֮��, ����Ӧ�ðѸ���λ��ָ�����һ��, ��ǰλ��ָ�����һ��
        pLyric->index = size - 1;
    }
    else
    {
        // ָ��ʱ��ָ����ĳһ�и��, �ҵ��ǵڼ�����
        pLyric->index = index;
    }

    auto& line = pLyric->lines[pLyric->index];

    // �ҵ��ǵڼ�����
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
            // �����������ֵĸ������, �ȼ���ʱ��ٷֱ�, Ȼ������ֵĿ��
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
/// �����ǰ/�Ӻ�, ��������ǰ, �������Ӻ�
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="nTime">Ҫ��ǰ�����Ӻ��ʱ��, ��������ǰ, �������Ӻ�, ��λ�Ǻ���</param>
/// <returns>�������ú���ʱ�ĺ�����, �����������0��Ϊ��ȡ</returns>
int LYRICCALL lyric_behind_ahead(HLYRIC hLyric, int nTime)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;

    if (!pLyric)
        return 0;

    pLyric->nTimeOffset += nTime;
    return pLyric->nTimeOffset;
}

/// <summary>
/// ��ȡ������¼�ĸ��ֻ�����Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="pRet">���շ������ݵĽṹָ��, �����Ҫ���ظ�����ı���������Ϣ, ��Ҫ���仺����, Ȼ�󴫵ݽ���</param>
/// <returns>�����Ƿ��ȡ�ɹ�</returns>
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

        // ����Ҫ��ȡ����еĲ�ѭ����ȡ
        if (pRet->nArrayStrBuffer || pRet->nArrayLineInfoBuffer)
        {
            int nCount = (int)pLyric->lines.size();
            int nArrayStrBuffer = 0, nArrayLineInfoBuffer = 0;
            for (int i = 0; i < nCount; i++)
            {
                auto& line = pLyric->lines[i];
                if (pRet->nArrayStrBuffer && pRet->pArrayStrBuffer)
                {
                    // ��Ҫ��ȡ�ı���, д�뻺����
                    if (nArrayStrBuffer < pRet->nArrayStrBuffer)
                        pRet->pArrayStrBuffer[nArrayStrBuffer++] = line.text.c_str();
                }

                if (pRet->nArrayLineInfoBuffer && pRet->pArrayLineInfoBuffer)
                {
                    // ��Ҫ��ȡ����Ϣ, д�뻺����
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
/// ��ȡ�������
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <returns>��������, û�з���0</returns>
int LYRICCALL lyric_get_line_count(HLYRIC hLyric)
{
    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    if (pLyric)
        return (int)pLyric->lines.size();
    return 0;
}

/// <summary>
/// ��ȡĳһ�и������, Ӣ�ĸ���ǵ�����
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <returns>��������, û�з���0</returns>
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
/// ��ȡ�������Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="pRet">���صĸ������Ϣ</param>
/// <returns>�����Ƿ��ȡ�ɹ�</returns>
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
                // ��һ��û�м����ֿ��, �������һ��
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
/// ��ȡĳһ�и����ָ���ֵ���Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="indexWord">���������</param>
/// <param name="pRet">���صĸ������Ϣ</param>
/// <returns>�����Ƿ��ȡ�ɹ�</returns>
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
/// ��ȡ���и������Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="pArrayBuffer">���ո����Ϣ�����黺����, ��Ҫ��ȡ�������Ե��� lyric_get_line_count() ��ȡ</param>
/// <param name="nBufferCount">��ʾ pArrayBuffer �����ж��ٸ���Ա</param>
/// <returns>����д�뻺�����ĳ�Ա��</returns>
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
/// ��ȡĳһ�и���������ֵ���Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="pArrayBuffer">��������Ϣ�����黺����, ��Ҫ��ȡ�������Ե��� lyric_get_word_count() ��ȡ</param>
/// <param name="nBufferCount">��ʾ pArrayBuffer �����ж��ٸ���Ա</param>
/// <returns>����д�뻺�����ĳ�Ա��</returns>
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
/// ��ȡ������ı�
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <returns>���ظ���ı�, ʧ�ܻ���û���򷵻ؿ��ı�ָ��</returns>
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
/// ��ȡĳһ�и����ָ���ֵ��ı�
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="indexWord">���������</param>
/// <returns>�������ı�, ʧ�ܻ���û���򷵻ؿ��ı�ָ��</returns>
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
/// ��ȡĳһ�и��������, ��Ҫ���ⲿ�����㹻�Ŀռ��Ÿ���ı�����
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="pArrayBuffer">�����ֵ����黺����, ��Ҫ��ȡ�������Ե��� lyric_get_word_count() ��ȡ</param>
/// <param name="nBufferCount">��ʾ pArrayBuffer �����ж��ٸ���Ա</param>
/// <returns>����д������ĳ�Ա��</returns>
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
/// ��ȡ���и�����ı�, ��Ҫ���ⲿ�����㹻�Ŀռ��Ÿ���ı�����
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="pArrayBuffer">���ո�ʵ����黺����, ��Ҫ��ȡ����������Ե��� lyric_get_line_count() ��ȡ</param>
/// <param name="nBufferCount">��ʾ pArrayBuffer �����ж��ٸ���Ա</param>
/// <returns>����д������ĳ�Ա��</returns>
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
/// �Ѹ��ת����lrc��ʽ, lrc��ʽ�ĸ����û�����ֵ�, ����һ��һ�е�, ���ص�ָ����Ҫ���� lyric_free() �ͷ�
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <returns>���ظ���ı�ָ��, ʧ�ܷ���0, ��ʹ��ʱ��Ҫ���� lyric_free() �ͷ�</returns>
wchar_t* LYRICCALL lyric_to_lrc(HLYRIC hLyric)
{
    if (!hLyric)
        return nullptr;

    /*
    *
        [al:ר����]
        [ar:������]
        [au:�������-������]
        [by:��LRC�ļ��Ĵ�����]
        [offset:+/- ʱ�䲹��ֵ���Ժ���Ϊ��λ����ֵ��ʾ�ӿ죬��ֵ��ʾ�Ӻ�]
        [re:������LRC�ļ��Ĳ�������༭��]
        [ti:���(����)�ı���]
        [ve:����İ汾]
    *
    */

    auto pLyric = (PINSIDE_LYRIC_INFO)hLyric;
    std::wstring lrc;
    lrc.reserve(pLyric->lines.size() * 30); // ����ÿ����30���ַ�

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
/// ��ȡ���������Ϣ, ������ʹ��
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <returns>���ظ������, 0=û�з���, 1=����, 2=����, 3=����+����, ��������</returns>
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

