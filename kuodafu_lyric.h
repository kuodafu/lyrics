#pragma once

#define DECLARE_HANDLE(name) struct name##__{int unused;}; typedef struct name##__ *name
DECLARE_HANDLE(HLYRIC);

#define LYRICCALL __stdcall


struct LYRIC_LINE_STRUCT
{
    const wchar_t*  pText;      // �и���ı�
    const wchar_t*  pTranslate1;// �и�ʷ����ı�
    const wchar_t*  pTranslate2;// �и�������ı�
    int             nLength;    // �ַ���
    int             nTranslate1;// �ַ���
    int             nTranslate2;// �ַ���

    int             nStart;     // �и�ʿ�ʼʱ��, ��λ�Ǻ���
    int             nEnd;       // �и�ʽ���ʱ��, ��λ�Ǻ���
    int             nWordCount; // ��һ�и�ʵ�����, Ӣ���ǵ�����, ������ֲ����ַ�Ҳ�����ֽ�, �Ǹ��ݸ�����ݾ�����
    
    int             nWidth;     // ��һ���ı�ռ�õĿ��, ��λ������, û�����ü����ı��ص�ʱֵΪ0
};

struct LYRIC_WORD_STRUCT
{
    const wchar_t*  pText;      // �ָ���ı�
    int             nLength;    // �ַ���
    int             nStart;     // �ֵĿ�ʼʱ��, ��λ�Ǻ���, �����ʼʱ������Ը���е�
    int             nEnd;       // �ֵĽ���ʱ��, ��λ�Ǻ���

    int             nLeft;      // ���������һ�и��������λ��, ���û�����ü����ı��ص�, �����⼸��ֵ�᷵��0
    int             nWidth;     // ��ռ�õĿ��
    int             nHeight;    // ��ռ�õĸ߶�, һ����˵���ж���һ���ĸ߶�, Ŀǰֻ��������

};


// ����ָ��ʱ������һ�и�ʵ���һ������
struct LYRIC_CALC_STRUCT
{
    int                 indexLine;  // ��ǰʱ���������ʵ���һ����, ��0��ʼ
    int                 indexWord;  // ��ǰʱ������һ�и�ʵ���һ������, ��0��ʼ
    int                 nWidthWord; // �������ʱ������������ռ�õĿ��, ����ȷ������λ��
    LYRIC_LINE_STRUCT   line;       // �������Ϣ
    LYRIC_WORD_STRUCT   word;       // �������Ϣ

};


// ������ʵĻ�����Ϣ
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

    int             nArrayStrBuffer;    // ����ʱ��ʾ pArrayBuffer �ĳ�Ա��, ���ʱ��ʾд�� pArrayBuffer �ĳ�Ա��
    const wchar_t** pArrayStrBuffer;    // ���黺����, Ϊ0��д��, ÿһ�и�ʵ��ı�

    int                 nArrayLineInfoBuffer;   // ����ʱ��ʾ pArrayLineInfoBuffer �ĳ�Ա��, ���ʱ��ʾд�� pArrayLineInfoBuffer �ĳ�Ա��
    LYRIC_LINE_STRUCT*  pArrayLineInfoBuffer;   // ���黺����, Ϊ0��д��, ÿһ�и�ʵ���Ϣ, ��������ı�, ��ʼʱ��, ����ʱ��, ����

};

#define _def_struct(_name) typedef _name *P##_name, *LP##_name; typedef const _name *PC##_name, *LPC##_name

_def_struct(LYRIC_CALC_STRUCT);
_def_struct(LYRIC_LINE_STRUCT);
_def_struct(LYRIC_WORD_STRUCT);
_def_struct(LYRIC_INFO_STRUCT);

#undef _def_struct

typedef int (LYRICCALL* LYRIC_PARSE_CALCTEXT)(void* pUserData, LPCWSTR pText, int nTextLen, int* pRetHeight);

/// <summary>
/// �������, ����krc���ܺ������, ���ص�ָ����Ҫ���� lyric_destroy ���پ��
/// </summary>
/// <param name="pData">����, ��Ҫ���ܵĸ������</param>
/// <param name="nSize">����, ������ݵĳ���</param>
/// <param name="pfnCalcText">����Ϊ��ָ��, ����Ǹ��ص�����, �����������ֵ�ռ�ÿ��, �������ֿ��ȷ������λ��</param>
/// <param name="pUserData">���ݵ� pfnCalcText ����û�����</param>
/// <returns>���ؽ��ܺ������, ��ʹ��ʱ��Ҫ���� lyric_destroy ���پ��</returns>
HLYRIC LYRICCALL lyric_parse(const void* pData, int nSize);

/// <summary>
/// ���� lyric_parse() ���صĸ�ʾ��
/// </summary>
/// <param name="hLyric">lyric_parse() ���صĸ�ʾ��</param>
/// <returns>�޷���ֵ</returns>
void LYRICCALL lyric_destroy(HLYRIC hLyric);

/// <summary>
/// �ͷ��ı�ָ��, ���������д��Ҫ�ͷŵ�, ����Ҫ������������ͷ�, ûд��Ҫ�ͷžͲ���Ҫ����
/// </summary>
/// <param name="pStr">Ҫ�ͷŵ��ı�ָ��</param>
/// <returns>�޷���ֵ</returns>
void LYRICCALL lyric_free(void* pStr);

/// <summary>
/// ���������ֵĿ��, ���� lyric_calc() �������������ﴫ�ݵĻص�������������ռ�ÿ��
/// ÿ�μ��㶼�Ǽ�����һ�������ֵĿ��, ����ȷ����ʸ���λ��
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="pfnCalcText">����Ϊ��ָ��, ����Ǹ��ص�����, �����������ֵ�ռ�ÿ��, �������ֿ��ȷ������λ��</param>
/// <param name="pUserData">���ݵ� pfnCalcText ����û�����</param>
/// <returns>�����Ƿ���ɹ�</returns>
bool LYRICCALL lyric_calc_text_width(HLYRIC hLyric, LYRIC_PARSE_CALCTEXT pfnCalcText, void* pUserData);

/// <summary>
/// ���¼��������ֵĿ��, ����������������¼�����ռ�ÿ��, ����ȷ����ʸ���λ��, ���屻�ı�ʱ����
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <returns>�����Ƿ���ɹ�</returns>
bool LYRICCALL lyric_re_calc_text_width(HLYRIC hLyric);


/// <summary>
/// ����ָ��ʱ�����ڸ�ʵ���һ����һ������
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="time">Ҫ��ѯ��ʱ��, ��λ�Ǻ���</param>
/// <param name="pRet">�ο����ص�����, ���ظ�����ı�, ������ı�, ������, ������</param>
/// <returns>�����Ƿ��ȡ�ɹ�</returns>
bool LYRICCALL lyric_calc(HLYRIC hLyric, int time, LYRIC_CALC_STRUCT* pRet);

/// <summary>
/// ��ȡ������¼�ĸ��ֻ�����Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="pRet">���շ������ݵĽṹָ��, �����Ҫ���ظ�����ı���������Ϣ, ��Ҫ���仺����, Ȼ�󴫵ݽ���</param>
/// <returns>�����Ƿ��ȡ�ɹ�</returns>
bool LYRICCALL lyric_get_info(HLYRIC hLyric, PLYRIC_INFO_STRUCT pRet);


/// <summary>
/// ��ȡ�������
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <returns>��������, û�з���0</returns>
int LYRICCALL lyric_get_line_count(HLYRIC hLyric);

/// <summary>
/// ��ȡĳһ�и������, Ӣ�ĸ���ǵ�����
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <returns>��������, û�з���0</returns>
int LYRICCALL lyric_get_word_count(HLYRIC hLyric, int indexLine);

/// <summary>
/// ��ȡ�������Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="pRet">���صĸ������Ϣ</param>
/// <returns>�����Ƿ��ȡ�ɹ�</returns>
bool LYRICCALL lyric_get_line(HLYRIC hLyric, int indexLine, PLYRIC_LINE_STRUCT pRet);

/// <summary>
/// ��ȡĳһ�и����ָ���ֵ���Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="indexWord">���������</param>
/// <param name="pRet">���صĸ������Ϣ</param>
/// <returns>�����Ƿ��ȡ�ɹ�</returns>
bool LYRICCALL lyric_get_word(HLYRIC hLyric, int indexLine, int indexWord, PLYRIC_WORD_STRUCT pRet);

/// <summary>
/// ��ȡ���и������Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="pArrayBuffer">���ո����Ϣ�����黺����, ��Ҫ��ȡ�������Ե��� lyric_get_line_count() ��ȡ</param>
/// <param name="nBufferCount">��ʾ pArrayBuffer �����ж��ٸ���Ա</param>
/// <returns>����д�뻺�����ĳ�Ա��</returns>
int LYRICCALL lyric_get_all_line(HLYRIC hLyric, PLYRIC_LINE_STRUCT pArrayBuffer, int nBufferCount);

/// <summary>
/// ��ȡĳһ�и���������ֵ���Ϣ
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="pArrayBuffer">��������Ϣ�����黺����, ��Ҫ��ȡ�������Ե��� lyric_get_word_count() ��ȡ</param>
/// <param name="nBufferCount">��ʾ pArrayBuffer �����ж��ٸ���Ա</param>
/// <returns>����д�뻺�����ĳ�Ա��</returns>
int LYRICCALL lyric_get_all_word(HLYRIC hLyric, int indexLine, PLYRIC_WORD_STRUCT pArrayBuffer, int nBufferCount);

/// <summary>
/// ��ȡ������ı�
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <returns>���ظ���ı�, ʧ�ܻ���û���򷵻ؿ��ı�ָ��</returns>
const wchar_t* LYRICCALL lyric_get_line_str(HLYRIC hLyric, int indexLine);

/// <summary>
/// ��ȡĳһ�и����ָ���ֵ��ı�
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="indexWord">���������</param>
/// <returns>�������ı�, ʧ�ܻ���û���򷵻ؿ��ı�ָ��</returns>
const wchar_t* LYRICCALL lyric_get_word_str(HLYRIC hLyric, int indexLine, int indexWord);

/// <summary>
/// ��ȡĳһ�и��������, ��Ҫ���ⲿ�����㹻�Ŀռ��Ÿ���ı�����
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="indexLine">���������</param>
/// <param name="pArrayBuffer">�����ֵ����黺����, ��Ҫ��ȡ�������Ե��� lyric_get_word_count() ��ȡ</param>
/// <param name="nBufferCount">��ʾ pArrayBuffer �����ж��ٸ���Ա</param>
/// <returns>����д������ĳ�Ա��</returns>
int LYRICCALL lyric_get_word_all_str(HLYRIC hLyric, int indexLine, const wchar_t** pArrayBuffer, int nBufferCount);

/// <summary>
/// ��ȡ���и�����ı�, ��Ҫ���ⲿ�����㹻�Ŀռ��Ÿ���ı�����
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <param name="pArrayBuffer">���ո�ʵ����黺����, ��Ҫ��ȡ����������Ե��� lyric_get_line_count() ��ȡ</param>
/// <param name="nBufferCount">��ʾ pArrayBuffer �����ж��ٸ���Ա</param>
/// <returns>����д������ĳ�Ա��</returns>
int LYRICCALL lyric_get_line_all_str(HLYRIC hLyric, const wchar_t** pArrayBuffer, int nBufferCount);

/// <summary>
/// �Ѹ��ת����lrc��ʽ, lrc��ʽ�ĸ����û�����ֵ�, ����һ��һ�е�, ���ص�ָ����Ҫ���� lyric_free() �ͷ�
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <returns>���ظ���ı�ָ��, ʧ�ܷ���0, ��ʹ��ʱ��Ҫ���� lyric_free() �ͷ�</returns>
wchar_t* LYRICCALL lyric_to_lrc(HLYRIC hLyric, int indexLine, int indexWord);

/// <summary>
/// ��ȡ���������Ϣ, ������ʹ��
/// </summary>
/// <param name="hLyric">��ʾ��</param>
/// <returns>���ظ������, 0=û�з���, 1=����, 2=����, 3=����+����, ��������</returns>
int LYRICCALL lyric_get_language(HLYRIC hLyric);

