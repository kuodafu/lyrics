#pragma once

#define CHARSET_NAME_BEGIN namespace charset_stl{
#define CHARSET_NAME_END }

CHARSET_NAME_BEGIN

// �Ƿ����������ı�����, Ϊ���ʱ���û�����ĳ��ȱ�ʵ���ı����ֵҲ����ȷת��, ���ǻ�����һ�μ����ı����ȵĺ���
constexpr const bool __calc_input_text_length = false;

// �ͷŷ��ص�ָ��
template<typename T>
inline void charset_free(T* ptr)
{
    ::free(ptr);
}

// �����ڴ�, ���صĽ����Ҫ���� charset_free() �ͷ�, ����ʧ���׳� std::bad_alloc �쳣
template<typename T>
inline T* charset_alloc(size_t size)
{
    T* p = reinterpret_cast<T*>(::malloc(sizeof(T) * size));
    if (!p)
        throw std::bad_alloc();
    return p;
}

// Unicodeתansi, ���صĽ����Ҫ���� charset_free() �ͷ�
inline char* UnicodeToAnsi(const wchar_t* const pStr, size_t len = -1, int cp = 936)
{
    if (!pStr)
        return 0;

    size_t str_len = (__calc_input_text_length || len < 0 || len > 0x7fffffff) ? wcslen(pStr) : len;
    size_t min_len = min(str_len, len);

    int convertResult = WideCharToMultiByte(cp, 0, pStr, static_cast<int>(min_len), 0, 0, 0, 0);
    if (convertResult <= 0)
        return 0;

    int size = convertResult + 10;
    char* ptr = charset_alloc<char>(size);
    int r_len = WideCharToMultiByte(cp, 0, pStr, static_cast<int>(min_len), ptr, size, 0, 0);
    ptr[r_len] = L'\0';
    return ptr;
}

// ansiתUnicode, ���صĽ����Ҫ���� charset_free() �ͷ�
inline wchar_t* AnsiToUnicode(const char* const pStr, size_t len = -1, int cp = 936)
{
    if (!pStr)
        return 0;

    size_t str_len = (__calc_input_text_length || len < 0 || len > 0x7fffffff) ? strlen(pStr) : len;
    size_t min_len = min(str_len, len);

    int convertResult = MultiByteToWideChar(cp, 0, pStr, static_cast<int>(min_len), NULL, 0);
    if (convertResult <= 0)
        return 0;

    int size = convertResult + 10;
    wchar_t* ptr = charset_alloc<wchar_t>(size);
    int r_len = MultiByteToWideChar(cp, 0, pStr, static_cast<int>(min_len), ptr, size);
    ptr[r_len] = L'\0';
    return ptr;
}

// unicodeתUTF8, ���صĽ����Ҫ���� charset_free() �ͷ�
inline char* UnicodeToUtf8(const wchar_t* const pStr, size_t len = -1)
{
    return UnicodeToAnsi(pStr, len, CP_UTF8);
}

// UTF8תunicode, ���صĽ����Ҫ���� charset_free() �ͷ�
inline wchar_t* Utf8ToUnicode(const char* const pStr, size_t len = -1)
{
    return AnsiToUnicode(pStr, len, CP_UTF8);
}

// UTF8תansi, ���صĽ����Ҫ���� charset_free() �ͷ�
inline char* Utf8ToAnsi(const char* const pStr, size_t len = -1)
{
    // �Ȱ�utf8�ı�ת��unicode, �ڰ�unicodeת��ansi
    if (!pStr)
        return 0;
    wchar_t* unicode = Utf8ToUnicode(pStr, len);
    if (!unicode)
        return 0;

    char* ansi = UnicodeToAnsi(unicode);
    charset_free(unicode);
    return ansi;
}

// ansiתUTF8, ���صĽ����Ҫ���� charset_free() �ͷ�
inline char* AnsiToUtf8(const char* const pStr, size_t len = -1)
{
    // �Ȱ�ansi�ı�ת��unicode, �ڰ�unicodeת��utf8
    if (!pStr)
        return 0;
    wchar_t* unicode = AnsiToUnicode(pStr, len);
    if (!unicode)
        return 0;

    char* utf8 = UnicodeToAnsi(unicode, -1, CP_UTF8);
    charset_free(unicode);
    return utf8;
}


// Unicodeתansi
inline std::string (W2A)(const wchar_t* const pStr, size_t len = -1, int cp = 936)
{
    std::string result;
    if (!pStr)
        return result;

    size_t str_len = (__calc_input_text_length || len < 0 || len > 0x7fffffff) ? wcslen(pStr) : len;
    size_t min_len = min(str_len, len);

    int convertResult = WideCharToMultiByte(cp, 0, pStr, static_cast<int>(min_len), 0, 0, 0, 0);
    if (convertResult <= 0)
        return result;

    result.resize(static_cast<size_t>(convertResult) + 10);

    int ret_len = WideCharToMultiByte(cp, 0, pStr, static_cast<int>(min_len), &result[0], static_cast<int>(result.size()), 0, 0);
    result.resize(ret_len);
    return result;
}
inline std::string(W2A)(const std::wstring& str, int cp = 936)
{
    return (W2A)(str.c_str(), str.size(), cp);
}
// ansiתUnicode
inline std::wstring (A2W)(const char* const pStr, size_t len = -1, int cp = 936)
{
    std::wstring result;
    if (!pStr)
        return result;

    size_t str_len = (__calc_input_text_length || len < 0 || len > 0x7fffffff) ? strlen(pStr) : len;
    size_t min_len = min(str_len, len);

    int convertResult = MultiByteToWideChar(cp, 0, pStr, static_cast<int>(min_len), NULL, 0);
    if (convertResult <= 0)
        return result;

    result.resize(static_cast<size_t>(convertResult) + 10);

    int ret_len = MultiByteToWideChar(cp, 0, pStr, static_cast<int>(min_len), &result[0], static_cast<int>(result.size()));
    result.resize(ret_len);
    return result;
}
inline std::wstring(A2W)(const std::string& str, int cp = 936)
{
    return (A2W)(str.c_str(), str.size(), cp);
}

// unicodeתUTF8
inline std::string (W2U)(const wchar_t* const str, size_t len = -1)
{
    return (W2A)(str, len, CP_UTF8);
}

// unicodeתUTF8
inline std::string (W2U)(const std::wstring& str)
{
    return (W2A)(str.c_str(), str.size(), CP_UTF8);
}

// UTF8תunicode
inline std::wstring (U2W)(const char* const str, size_t len = -1)
{
    return (A2W)(str, len, CP_UTF8);
}
// UTF8תunicode
inline std::wstring (U2W)(const std::string& str)
{
    return (A2W)(str.c_str(), str.size(), CP_UTF8);
}

// UTF8תansi
inline std::string (U2A)(const char* const utf8, size_t len = -1)
{
    // �Ȱ�utf8�ı�ת��unicode, �ڰ�unicodeת��ansi
    if (!utf8)
        return std::string();
    std::wstring unicode = (U2W)(utf8, len);
    if (unicode.empty())
        return std::string();
    return (W2A)(unicode.c_str(), unicode.length());
}

// UTF8תansi
inline std::string (U2A)(const std::string& str)
{
    return (U2A)(str.c_str(), str.size());
}

// ansiתUTF8
inline std::string (A2U)(const char* const ansi, size_t len = -1)
{
    // �Ȱ�ansi�ı�ת��unicode, �ڰ�unicodeת��utf8
    if (!ansi)
        return std::string();
    std::wstring unicode = (A2W)(ansi, len);
    if (unicode.empty())
        return std::string();
    return (W2A)(unicode.c_str(), unicode.length(), CP_UTF8);
}

// ansiתUTF8
inline std::string (A2U)(const std::string& str)
{
    return (A2U)(str.c_str(), str.size());
}

#define CHARSET_TYPE_GBK        0
#define CHARSET_TYPE_UTF8       1
#define CHARSET_TYPE_UTF16_LE   2
#define CHARSET_TYPE_UTF16_BE   3

namespace __charsel_impl
{
    // ��ȡָ���ڵ��ַ���, ����ָ���ڵ��ַ�����������, �ο�����ָ���ı����ݵ�ָ����ֽ���
    // �����BOMͷ������BOMͷ, ������BOMͷ������
    inline int __charset_get_strcharset(const void* pInput, size_t nInput, int charset_type, const void*& pStrRet, size_t& nStrRet)
    {
        auto pStart = reinterpret_cast<const BYTE*>(pInput);
        size_t nSize = nInput;
        if (nSize >= 2 && nSize < MAXINT)
        {
            if (pStart[0] == 0xff && pStart[1] == 0xfe)
            {
                charset_type = CHARSET_TYPE_UTF16_LE;
                pStart += 2;
                nSize -= 2;
            }
            else if (pStart[0] == 0xfe && pStart[1] == 0xff)
            {
                charset_type = CHARSET_TYPE_UTF16_BE;
                pStart += 2;
                nSize -= 2;
            }
            else if (nSize >= 3 && pStart[0] == 0xEF && pStart[1] == 0xBB && pStart[2] == 0xBF)
            {
                charset_type = CHARSET_TYPE_UTF8;
                pStart += 3;
                nSize -= 3;
            }
        }
        
        pStrRet = pStart;
        nStrRet = nSize;
        return charset_type;
    }

    // ��ȡ�����ַ������ֽ���, ���ݴ��ݽ����ĳ��Ȼ�ȡ, �������Խ��, �ͻ�ȡ�ı�����
    template<typename _Ty>
    inline size_t __charset_get_input_size(const _Ty* pInput, size_t& nInput)
    {
        if (nInput > MAXINT || nInput < 0)
        {
            auto ptr = pInput;
            nInput = 0;
            while (ptr && *ptr)
            {
                ptr++;
                nInput++;
            }
            nInput *= sizeof(_Ty);
        }
        return nInput;
    }

    // ֱ�����ڴ��ַ�ϰѴ�С��תһ��, ����ѭ����ÿ���ֽڵ�λ�ý���һ��, ת��LE���BE, BE���LE
    inline bool __charset_le_be_convert(wchar_t* pInput, size_t nInput)
    {
        if (!pInput)
            return false;
        auto ptr = reinterpret_cast<BYTE*>(pInput);
        for (size_t i = 0; i < nInput; i += 2)
        {
            BYTE t = ptr[i];    // ��С��ת��һ��
            ptr[i] = ptr[i + 1];
            ptr[i + 1] = t;
        }
        return true;
    }

    inline bool __charset_copy_str(const char* pInput, size_t nInput, char** pOutput, size_t& nOutput, bool isConvertBe)
    {
        auto pText = charset_alloc<char>(nInput + sizeof(char));
        memcpy(pText, pInput, nInput);
        memset(pText + nInput, 0, sizeof(char));  // ���Ͻ�����־��
        nOutput = nInput;
        *pOutput = pText;
        return nOutput != 0;
    }
    inline bool __charset_copy_str(const char* pInput, size_t nInput, std::string* pOutput, size_t& nOutput, bool isConvertBe)
    {
        pOutput->assign(pInput, nInput);
        nOutput = nInput;
        return nOutput != 0;
    }
    inline bool __charset_copy_str(const wchar_t* pInput, size_t nInput, wchar_t** pOutput, size_t& nOutput, bool isConvertBe)
    {
        auto pText = charset_alloc<char>(nInput + sizeof(wchar_t));
        memcpy(pText, pInput, nInput);
        memset(pText + nInput, 0, sizeof(wchar_t)); // ���Ͻ�����־��
        nOutput = nInput;
        *pOutput = reinterpret_cast<wchar_t*>(pText);
        if (isConvertBe)
            __charset_le_be_convert(*pOutput, nOutput);
        return nOutput != 0;
    }
    inline bool __charset_copy_str(const wchar_t* pInput, size_t nInput, std::wstring* pOutput, size_t& nOutput, bool isConvertBe)
    {
        pOutput->assign(pInput, nInput / sizeof(wchar_t));
        nOutput = nInput;
        if (isConvertBe)
            __charset_le_be_convert(&((*pOutput)[0]), nOutput);
        return nOutput != 0;
    }

    
    inline size_t __charset_get_str_size(const char* vl)        { return vl ? strlen(vl) * sizeof(char) : 0; }
    inline size_t __charset_get_str_size(const wchar_t* vl)     { return vl ? wcslen(vl) * sizeof(wchar_t) : 0; }
    inline size_t __charset_get_str_size(const std::string& vl) { return vl.size() * sizeof(char); }
    inline size_t __charset_get_str_size(const std::wstring& vl){ return vl.size() * sizeof(wchar_t); }

    template<typename _Ty, typename _Fn_w, typename _Fn_a>
    struct CHARSET_CONVERT_ARG
    {
        const void* pInput;
        size_t      nInput;
        int         input_charset;
        int         output_charset;
        _Ty*        pOutput;
        _Fn_w       fn_w;
        _Fn_a       fn_a;
        CHARSET_CONVERT_ARG(const void* pInput, size_t nInput, int input_charset,
                            int output_charset, _Ty* pOutput,
                            _Fn_w fn_w_a, _Fn_a fn_a_w)
            : pInput(pInput), nInput(nInput), input_charset(input_charset)
            , output_charset(output_charset), pOutput(pOutput)
            , fn_w(fn_w), fn_a(fn_a)
        {

        }
    };

    template<typename _Arg>
    inline bool __charset_to_ansi(const void* pInput, size_t nInput, int input_charset, _Arg&& arg)
    {
        size_t nOutput = 0;
        switch (input_charset)
        {
        case CHARSET_TYPE_GBK:
            return __charset_copy_str(reinterpret_cast<const char*>(pInput), nInput, arg.pOutput, nOutput, false);
        case CHARSET_TYPE_UTF8:
            return arg.fn_a(reinterpret_cast<const char*>(pInput), nInput, arg.pOutput);
        case CHARSET_TYPE_UTF16_LE:
            return arg.fn_w(reinterpret_cast<const wchar_t*>(pInput), nInput, arg.pOutput);
        case CHARSET_TYPE_UTF16_BE:
        {
            std::wstring pLe;
            __charset_copy_str(reinterpret_cast<const wchar_t*>(pInput), nInput, &pLe, nOutput, true);
            return arg.fn_w(pLe.c_str(), nOutput, arg.pOutput);
        }
        default:
            break;
        }
        return false;
    }

    template<typename _Arg>
    inline bool __charset_to_utf8(const void* pInput, size_t nInput, int input_charset, _Arg&& arg)
    {
        size_t nOutput = 0;
        switch (input_charset)
        {
        case CHARSET_TYPE_GBK:
            return arg.fn_a(reinterpret_cast<const char*>(pInput), nInput, arg.pOutput);
        case CHARSET_TYPE_UTF8:
            return __charset_copy_str(reinterpret_cast<const char*>(pInput), nInput, arg.pOutput, nOutput, false);
        case CHARSET_TYPE_UTF16_LE:
            return arg.fn_w(reinterpret_cast<const wchar_t*>(pInput), nInput, arg.pOutput);
        case CHARSET_TYPE_UTF16_BE:
        {
            std::wstring pLe;
            __charset_copy_str(reinterpret_cast<const wchar_t*>(pInput), nInput, &pLe, nOutput, true);
            return arg.fn_w(pLe.c_str(), nOutput, arg.pOutput);
        }
        default:
            break;
        }
        return false;
    }
    template<typename _Arg>
    inline bool __charset_to_utf16le(const void* pInput, size_t nInput, int input_charset, _Arg&& arg)
    {
        size_t nOutput = 0;
        switch (input_charset)
        {
        case CHARSET_TYPE_GBK:
            return arg.fn_a(reinterpret_cast<const char*>(pInput), nInput, arg.pOutput);
            break;
        case CHARSET_TYPE_UTF8:
            return arg.fn_w(reinterpret_cast<const char*>(pInput), nInput, arg.pOutput);
        case CHARSET_TYPE_UTF16_LE: // ����������һ��, ֱ�ӿ���
            return __charset_copy_str(reinterpret_cast<const wchar_t*>(pInput), nInput, arg.pOutput, nOutput, false);
        case CHARSET_TYPE_UTF16_BE: // �������BE, ֱ�ӿ���Ȼ���С��ת��һ��
            return __charset_copy_str(reinterpret_cast<const wchar_t*>(pInput), nInput, arg.pOutput, nOutput, true);
        default:
            break;
        }
        return false;
    }
    template<typename _Arg>
    inline bool __charset_to_utf16be(const void* pInput, size_t nInput, int input_charset, _Arg&& arg)
    {
        size_t nOutput = 0;
        switch (input_charset)
        {
        case CHARSET_TYPE_GBK:
            return arg.fn_a(reinterpret_cast<const char*>(pInput), nInput, arg.pOutput);
        case CHARSET_TYPE_UTF8:
            return arg.fn_w(reinterpret_cast<const char*>(pInput), nInput, arg.pOutput);
        case CHARSET_TYPE_UTF16_LE: // �������LE, ֱ�ӿ���Ȼ���С��ת��һ��
            return __charset_copy_str(reinterpret_cast<const wchar_t*>(pInput), nInput, arg.pOutput, nOutput, true);
        case CHARSET_TYPE_UTF16_BE: // ����������һ��, ֱ�ӿ���
            return __charset_copy_str(reinterpret_cast<const wchar_t*>(pInput), nInput, arg.pOutput, nOutput, false);
        default:
            break;
        }
        return false;
    }


    // ת���ַ���, ������ַ�����������, ������ַ�����������
    // �����Ƿ�ת���ɹ�, ����true��ʱ��, pOutput ��Ҫ���� charset_free() �ͷ�
    template<typename _Arg, typename _Fn>
    inline bool __charset_convert_str_ptr(_Arg&& arg, _Fn fn)
    {
        const void* pStr = nullptr;
        size_t nSize = 0;
        // �Ȼ�ȡ�������ݵı���, Ȼ��������ַָ���ı�����
        // ��BOMͷ�Ͱ�BOMͷ�ı�����, û���а����ݵ����������
        int input_charset = __charset_get_strcharset(arg.pInput, arg.nInput, arg.input_charset, pStr, nSize);

        switch (input_charset)
        {
        case CHARSET_TYPE_GBK:      // ������ַ�����GBK, �����������ת��ʲô���ı���
            __charset_get_input_size(reinterpret_cast<const char*>(pStr), nSize);
            break;
        case CHARSET_TYPE_UTF8:     // ������ַ�����UTF8, �����������ת��ʲô���ı���
            __charset_get_input_size(reinterpret_cast<const char*>(pStr), nSize);
            break;
        case CHARSET_TYPE_UTF16_LE: // ������ַ�����UTF16-LE, �����������ת��ʲô���ı���
            __charset_get_input_size(reinterpret_cast<const wchar_t*>(pStr), nSize);
            break;
        case CHARSET_TYPE_UTF16_BE: // ������ַ�����UTF16-BE, �����������ת��ʲô���ı���
            __charset_get_input_size(reinterpret_cast<const wchar_t*>(pStr), nSize);
            break;
        default:
            return false;
        }

        return fn(pStr, nSize, input_charset, std::move(arg));
    }

}



// ��ָ���ڵ�����ת��GBK�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2gbk(const void* ptr, size_t size, int charset_type, std::string& ret)
{
    using namespace __charsel_impl;
    CHARSET_CONVERT_ARG arg(ptr, size, charset_type, CHARSET_TYPE_GBK, &ret,
                        [](const wchar_t* pInput, size_t nInput, std::string* pOutput)// _Fn_w
                        {
                            *pOutput = (W2A)(pInput, nInput / sizeof(wchar_t));
                            return !pOutput->empty();
                        },
                        [](const char* pInput, size_t nInput, std::string* pOutput)   // _Fn_a
                        {
                            *pOutput = (U2A)(pInput, nInput / sizeof(char));
                            return !pOutput->empty();
                        });
    return __charset_convert_str_ptr(std::move(arg), __charset_to_ansi<decltype(arg)>);

}
// ��ָ���ڵ�����ת��GBK�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2gbk(const void* ptr, size_t size, int charset_type, char*& ret)
{
    using namespace __charsel_impl;
    CHARSET_CONVERT_ARG arg(ptr, size, charset_type, CHARSET_TYPE_GBK, &ret,
                        [](const wchar_t* pInput, size_t nInput, char** pOutput)// _Fn_w
                        {
                            *pOutput = (UnicodeToAnsi)(pInput, nInput / sizeof(wchar_t));
                            return *pOutput != nullptr;
                        },
                        [](const char* pInput, size_t nInput, char** pOutput)   // _Fn_a
                        {
                            *pOutput = (Utf8ToAnsi)(pInput, nInput / sizeof(char));
                            return *pOutput != nullptr;
                        });
    return __charset_convert_str_ptr(std::move(arg), __charset_to_ansi<decltype(arg)>);
}

// ��ָ���ڵ�����ת��UTF8�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2utf8(const void* ptr, size_t size, int charset_type, std::string& ret)
{
    using namespace __charsel_impl;
    CHARSET_CONVERT_ARG arg(ptr, size, charset_type, CHARSET_TYPE_UTF8, &ret,
                            [](const wchar_t* pInput, size_t nInput, std::string* pOutput)// _Fn_w
                            {
                                *pOutput = (W2U)(pInput, nInput / sizeof(wchar_t));
                                return !pOutput->empty();
                            },
                            [](const char* pInput, size_t nInput, std::string* pOutput)   // _Fn_a
                            {
                                *pOutput = (A2U)(pInput, nInput / sizeof(char));
                                return !pOutput->empty();
                            }
    );

    return __charset_convert_str_ptr(std::move(arg), __charset_to_utf8<decltype(arg)>);
}
// ��ָ���ڵ�����ת��UTF8�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2utf8(const void* ptr, size_t size, int charset_type, char*& ret)
{
    using namespace __charsel_impl;
    CHARSET_CONVERT_ARG arg(ptr, size, charset_type, CHARSET_TYPE_UTF8, &ret,
                            [](const wchar_t* pInput, size_t nInput, char** pOutput)// _Fn_w
                            {
                                *pOutput = (UnicodeToUtf8)(pInput, nInput / sizeof(wchar_t));
                                return *pOutput != nullptr;
                            },
                            [](const char* pInput, size_t nInput, char** pOutput)   // _Fn_a
                            {
                                *pOutput = (AnsiToUtf8)(pInput, nInput / sizeof(char));
                                return *pOutput != nullptr;
                            }
    );

    return __charset_convert_str_ptr(std::move(arg), __charset_to_utf8<decltype(arg)>);
}

// ��ָ���ڵ�����ת��UTF16-LE�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2utf16(const void* ptr, size_t size, int charset_type, std::wstring& ret)
{
    using namespace __charsel_impl;
    CHARSET_CONVERT_ARG arg(ptr, size, charset_type, CHARSET_TYPE_UTF16_LE, &ret,
                            [](const char* pInput, size_t nInput, std::wstring* pOutput)// _Fn_w
                            {
                                *pOutput = (U2W)(pInput, nInput / sizeof(char));
                                return !pOutput->empty();
                            },
                            [](const char* pInput, size_t nInput, std::wstring* pOutput)   // _Fn_a
                            {
                                *pOutput = (A2W)(pInput, nInput / sizeof(char));
                                return !pOutput->empty();
                            }
    );
    return __charset_convert_str_ptr(std::move(arg), __charset_to_utf16le<decltype(arg)>);
}
// ��ָ���ڵ�����ת��UTF16-LE�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2utf16(const void* ptr, size_t size, int charset_type, wchar_t*& ret)
{
    using namespace __charsel_impl;
    CHARSET_CONVERT_ARG arg(ptr, size, charset_type, CHARSET_TYPE_UTF16_LE, &ret,
                            [](const char* pInput, size_t nInput, wchar_t** pOutput)// _Fn_w
                            {
                                *pOutput = (Utf8ToUnicode)(pInput, nInput / sizeof(char));
                                return *pOutput != nullptr;
                            },
                            [](const char* pInput, size_t nInput, wchar_t** pOutput)   // _Fn_a
                            {
                                *pOutput = (AnsiToUnicode)(pInput, nInput / sizeof(char));
                                return *pOutput != nullptr;
                            }
    );
    return __charset_convert_str_ptr(std::move(arg), __charset_to_utf16le<decltype(arg)>);
}

// ��ָ���ڵ�����ת��UTF16-LE�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2utf16le(const void* ptr, size_t size, int charset_type, std::wstring& ret)
{
    return ptr2utf16(ptr, size, charset_type, ret);
}
// ��ָ���ڵ�����ת��UTF16-LE�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2utf16le(const void* ptr, size_t size, int charset_type, wchar_t*& ret)
{
    return ptr2utf16(ptr, size, charset_type, ret);
}

// ��ָ���ڵ�����ת��UTF16-BE�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2utf16be(const void* ptr, size_t size, int charset_type, std::wstring& ret)
{
    using namespace __charsel_impl;
    CHARSET_CONVERT_ARG arg(ptr, size, charset_type, CHARSET_TYPE_UTF16_BE, &ret,
                            [](const char* pInput, size_t nInput, std::wstring* pOutput)// _Fn_w
                            {
                                std::wstring& ret = *pOutput;
                                ret = (U2W)(pInput, nInput / sizeof(char));
                                bool isEmpty = ret.empty();
                                if (!isEmpty)
                                    __charset_le_be_convert(&ret[0], __charset_get_str_size(ret));
                                return !isEmpty;
                            },
                            [](const char* pInput, size_t nInput, std::wstring* pOutput)   // _Fn_a
                            {
                                std::wstring& ret = *pOutput;
                                ret = (A2W)(pInput, nInput / sizeof(char));
                                bool isEmpty = ret.empty();
                                if (!isEmpty)
                                    __charset_le_be_convert(&ret[0], __charset_get_str_size(ret));
                                return !isEmpty;
                            }
    );
    return __charset_convert_str_ptr(std::move(arg), __charset_to_utf16be<decltype(arg)>);
}
// ��ָ���ڵ�����ת��UTF16-BE�ַ���
// ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
// size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
// charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
inline bool ptr2utf16be(const void* ptr, size_t size, int charset_type, wchar_t*& ret)
{
    using namespace __charsel_impl;
    CHARSET_CONVERT_ARG arg(ptr, size, charset_type, CHARSET_TYPE_UTF16_BE, &ret,
                            [](const char* pInput, size_t nInput, wchar_t** pOutput)// _Fn_a
                            {
                                wchar_t*& ret = *pOutput;
                                ret = (Utf8ToUnicode)(pInput, nInput / sizeof(char));
                                if (ret)
                                    __charset_le_be_convert(ret, __charset_get_str_size(ret));
                                return ret != nullptr;
                            },
                            [](const char* pInput, size_t nInput, wchar_t** pOutput)   // _Fn_u
                            {
                                wchar_t*& ret = *pOutput;
                                ret = (AnsiToUnicode)(pInput, nInput / sizeof(char));
                                if (ret)
                                    __charset_le_be_convert(ret, __charset_get_str_size(ret));
                                return ret != nullptr;
                            }
    );
    return __charset_convert_str_ptr(std::move(arg), __charset_to_utf16be<decltype(arg)>);
}

CHARSET_NAME_END

#undef CHARSET_NAME_BEGIN
#undef CHARSET_NAME_END