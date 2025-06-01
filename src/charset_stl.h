#pragma once
#include <stdexcept>

namespace charset_stl
{
    // �Ƿ����������ı�����, Ϊ���ʱ���û�����ĳ��ȱ�ʵ���ı����ֵҲ����ȷת��, ���ǻ�����һ�μ����ı����ȵĺ���
    constexpr const bool __calc_input_text_length = false;

    // �ͷŷ��ص�ָ��
    template<typename T>
    inline void charset_free(T* ptr)
    {
        free(ptr);
    }

    // �����ڴ�, ���صĽ����Ҫ���� charset_free() �ͷ�, ����ʧ���׳� std::bad_alloc �쳣
    template<typename T>
    inline T* charset_alloc(size_t size)
    {
        T* p = reinterpret_cast<T*>(malloc(sizeof(T) * size));
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
            if (nSize >= 2)
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
                
            }
            else if (nSize >= 3 && pStart[0] == 0xEF && pStart[1] == 0xBB && pStart[2] == 0xBF)
            {
                charset_type = CHARSET_TYPE_UTF8;
                pStart += 3;
                nSize -= 3;
            }
            pStrRet = pStart;
            nStrRet = nSize;
            return charset_type;
        }

        // �ж��Ƿ��� char
        template<typename T>
        constexpr bool is_char_type_v = std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>;

        // �ж��Ƿ��� wchar_t
        template<typename T>
        constexpr bool is_wchar_type_v = std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, wchar_t>;

        // �ж��Ƿ��� std::string
        template<typename T>
        constexpr bool is_string_type_v = std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, std::string>;

        // �ж��Ƿ��� std::wstring
        template<typename T>
        constexpr bool is_wstring_type_v = std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, std::wstring>;

        template<typename _Fn_ptr, typename _Fn_stl, typename _Ty>
        struct __CHARSET_CONVERT_ARG
        {
            _Fn_ptr     pfn_ptr;
            _Fn_stl     pfn_stl;
            const _Ty*  pInput;
            size_t      nInput;

            int         out_charset_type;

            __CHARSET_CONVERT_ARG(_Fn_ptr pfn_ptr, _Fn_stl pfn_stl, const _Ty* pInput, size_t nInput, int out_charset_type)
                : pfn_ptr(pfn_ptr)
                , pfn_stl(pfn_stl)
                , pInput(pInput)
                , nInput(nInput)
                , out_charset_type(out_charset_type)
            {

            }

            template<typename _Ty>
            static size_t get_size(const _Ty& vl)
            {
                if constexpr (is_char_type_v<_Ty>)
                {
                    return vl ? strlen(vl) * sizeof(char) : 0;
                }
                else if constexpr (is_wchar_type_v<_Ty>)
                {
                    return vl ? wcslen(vl) * sizeof(wchar_t) : 0;
                }
                else if constexpr (is_string_type_v<_Ty>)
                {
                    return vl.size() * sizeof(char);
                }
                else if constexpr (is_wstring_type_v<_Ty>)
                {
                    return vl.size() * sizeof(wchar_t);
                }
                else
                    throw std::runtime_error("invalid type");
            }

            auto operator()(size_t& nOutput)
            {
                if constexpr (is_char_type_v<_Ty> || is_wchar_type_v<_Ty>)
                {
                    auto pRet = pfn_ptr(pInput, nInput);
                    nOutput = get_size(pRet);
                    return pRet;
                }
                if constexpr (is_string_type_v<_Ty> || is_wstring_type_v<_Ty>)
                {
                    auto pRet = pfn_stl(pInput, nInput);
                    nOutput = get_size(pRet);
                    return pRet;
                }
                throw std::runtime_error("invalid type");
            }
        };
        struct __CHARSET_CONVERT
        {
            template<typename _Ty>
            static size_t get_size(const _Ty& vl)
            {
                if constexpr (is_char_type_v<_Ty>)
                {
                    return vl ? strlen(vl) * sizeof(char) : 0;
                }
                else if constexpr (is_wchar_type_v<_Ty>)
                {
                    return vl ? wcslen(vl) * sizeof(wchar_t) : 0;
                }
                else if constexpr (is_string_type_v<_Ty>)
                {
                    return vl.size() * sizeof(char);
                }
                else if constexpr (is_wstring_type_v<_Ty>)
                {
                    return vl.size() * sizeof(wchar_t);
                }
                else
                    throw std::runtime_error("invalid type");
            }
            template<typename _Fn, typename _Ty>
            auto ret_value(const void* pInput, size_t nInput, _Fn pfn, size_t& nOutput)
            {
                auto pRet = pfn(reinterpret_cast<const _Ty*>(pInput), nInput);
                nOutput = get_size(pRet);
                return pRet;
            }

            template<typename _Fn, typename _Ty>
            auto ret_empty(const void* pInput, size_t nInput, _Fn pfn)
            {
                using ret_type = decltype(pfn(reinterpret_cast<const _Ty*>(pInput), nInput));
                return ret_type();
            }

            // gbkתָ����ʽ, תʲô������pfn����
            template<typename _Fn>
            auto gbk_to(const void* pInput, size_t nInput, _Fn pfn, size_t& nOutput)
                -> decltype(auto)
            {
                return ret_value<_Fn, char>(pInput, nInput, pfn, nOutput);
            }
            // gbkתָ����ʽ, תʲô������pfn����
            template<typename _Ty>
            auto gbk_to(_Ty&& arg, size_t& nOutput)
                -> decltype(auto)
            {
                auto r = arg(nOutput);
                return r;
            }

            // utf8תָ����ʽ, תʲô������pfn����
            template<typename _Fn>
            auto utf8_to(const void* pInput, size_t nInput, _Fn pfn, size_t& nOutput)
                -> decltype(auto)
            {
                return ret_value<_Fn, char>(pInput, nInput, pfn, nOutput);
            }

            // utf16-leתָ����ʽ, תʲô������pfn����
            template<typename _Fn>
            auto utf16le_to(const void* pInput, size_t nInput, _Fn pfn, size_t& nOutput)
                -> decltype(auto)
            {
                auto pText = reinterpret_cast<const wchar_t*>(pInput);
                get_input_size(pText, nInput);

                if (nInput > MAXINT || nInput % 2 != 0 || nInput == 0)
                    return ret_empty<_Fn, wchar_t>(pInput, nInput, pfn);  // �ֽ�������

                size_t len = nInput / 2;
                size_t str_len = wcslen(pText);
                size_t min_len = min(str_len, len);

                return ret_value<_Fn, wchar_t>(pText, min_len, pfn, nOutput);
            }
            // utf16-beתָ����ʽ, תʲô������pfn����
            template<typename _Fn>
            auto utf16be_to(const void* pInput, size_t nInput, _Fn pfn, size_t& nOutput)
                -> decltype(auto)
            {
                auto pText = reinterpret_cast<const wchar_t*>(pInput);
                get_input_size(pText, nInput);

                if (nInput > MAXINT || nInput % 2 != 0)
                    return ret_empty<_Fn, wchar_t>(pInput, nInput, pfn);  // �ֽ�������

                // ��ת��le
                auto pLe = utf16le2be_copy(pInput, nInput);
                size_t len = nInput / 2;
                size_t str_len = wcslen(pText);
                size_t min_len = min(str_len, len);

                auto pRet = ret_value<_Fn, wchar_t>(pLe, min_len, pfn, nOutput);
                charset_free(pLe);
                return pRet;
            }


        public:
            // ��ȡ�����ַ������ֽ���, ���ݴ��ݽ����ĳ��Ȼ�ȡ, �������Խ��, �ͻ�ȡ�ı�����
            template<typename _Ty>
            static size_t get_input_size(const _Ty* pInput, size_t& nInput)
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
            static char* copy_gbk(const void* pInput, size_t nInput, size_t& nOutput)
            {
                auto pText = reinterpret_cast<const char*>(pInput);
                get_input_size(pText, nInput);
                auto pOutput = charset_alloc<char>(nInput + 1);
                memcpy(pOutput, pInput, nInput);
                memset(reinterpret_cast<LPBYTE>(pOutput) + nInput, 0, 1);    // ���Ͻ�����־��
                nOutput = nInput;
                return pOutput;
            }
            static char* copy_utf8(const void* pInput, size_t nInput, size_t& nOutput)
            {
                return copy_gbk(pInput, nInput, nOutput);   // UTF8 �� GBK ����ʽ��һ����
            }
            static wchar_t* copy_utf16le(const void* pInput, size_t nInput, size_t& nOutput)
            {
                auto pText = reinterpret_cast<const wchar_t*>(pInput);
                get_input_size(pText, nInput);
                auto pOutput = charset_alloc<char>(nInput + 1);
                const auto cbSize = sizeof(wchar_t);
                memcpy(pOutput, pInput, nInput * cbSize);
                memset(reinterpret_cast<LPBYTE>(pOutput) + nInput * cbSize, 0, cbSize); // ���Ͻ�����־��
                nOutput = nInput;
                return reinterpret_cast<wchar_t*>(pOutput);
            }
            static wchar_t* copy_utf16be(const void* pInput, size_t nInput, size_t& nOutput)
            {
                auto pOutput = copy_utf16le(pInput, nInput, nOutput);
                le_be_convert(pOutput, nOutput);
                return pOutput;
            }

            // ��UTF16-LEת��BE, ����һ���µ��ڴ���
            static wchar_t* utf16le2be_copy(const void* pInput, size_t nInput)
            {
                if (!pInput)
                    return nullptr;
                get_input_size(reinterpret_cast<const wchar_t*>(pInput), nInput);

                auto pLe = charset_alloc<BYTE>(nInput + 2);
                memcpy(pLe, pInput, nInput);
                memset(pLe + nInput, 0, sizeof(wchar_t));
                le_be_convert(pLe, nInput);
                return reinterpret_cast<wchar_t*>(pLe);
            }

            // ֱ�����ڴ��ַ�ϰѴ�С��תһ��, ����ѭ����ÿ���ֽڵ�λ�ý���һ��, ת��LE���BE, BE���LE
            static bool le_be_convert(void* pInput, size_t nInput)
            {
                if (!pInput)
                    return false;
                get_input_size(reinterpret_cast<const wchar_t*>(pInput), nInput);

                auto pText = reinterpret_cast<wchar_t*>(pInput);
                auto ptr = reinterpret_cast<BYTE*>(pInput);
                for (size_t i = 0; i < nInput; i += 2)
                {
                    BYTE t = ptr[i];    // ��С��ת��һ��
                    ptr[i] = ptr[i + 1];
                    ptr[i + 1] = t;
                }
                return true;
            }


            // ��UTF16-BEת��LE, ����һ���µ��ڴ���
            static wchar_t* utf16be2le_copy(const void* pInput, size_t nInput)
            {
                if (!pInput)
                    return nullptr;
                get_input_size(reinterpret_cast<const wchar_t*>(pInput), nInput);

                auto pLe = charset_alloc<BYTE>(nInput + 2);
                memcpy(pLe, pInput, nInput);
                memset(pLe + nInput, 0, sizeof(wchar_t));
                le_be_convert(pLe, nInput);
                return reinterpret_cast<wchar_t*>(pLe);
            }


        };
        



        // ת���ַ���, ������ַ�����������, ������ַ�����������
        // �����Ƿ�ת���ɹ�, ����true��ʱ��, pOutput ��Ҫ���� charset_free() �ͷ�
        template<typename _Ty>
        inline bool __charset_convert_str_ptr(const void* pInput, size_t nInput, int src_charset_type, int dst_charset_type, _Ty* pOutput, size_t& nOutput)
        {
            nOutput = 0;
            const void* pStr = pOutput;
            size_t nSize = 0;
            if constexpr (is_char_type_v<_Ty> || is_wchar_type_v<_Ty>)
                src_charset_type = __charset_get_strcharset(pInput, nInput, src_charset_type, pStr, nSize);
            __CHARSET_CONVERT convert;

            // ͳһ���ó���������, �����ڲ��Ͳ���Ҫ�жϴ��ݵ���������������3��������
            auto pfn_a2w = [](const char* const pStr, size_t len) { return AnsiToUnicode(pStr, len); };
            auto pfn_a2u = [](const char* const pStr, size_t len) { return AnsiToUtf8(pStr, len); };
            auto pfn_u2w = [](const char* const pStr, size_t len) { return Utf8ToUnicode(pStr, len); };
            auto pfn_u2a = [](const char* const pStr, size_t len) { return Utf8ToAnsi(pStr, len); };
            auto pfn_w2a = [](const wchar_t* const pStr, size_t len) { return UnicodeToAnsi(pStr, len); };
            auto pfn_w2u = [](const wchar_t* const pStr, size_t len) { return UnicodeToUtf8(pStr, len); };

            auto pfn_a2w_stl = [](const char* const pStr, size_t len) { return (A2W)(pStr, len); };
            auto pfn_a2u_stl = [](const char* const pStr, size_t len) { return (A2U)(pStr, len); };
            auto pfn_u2w_stl = [](const char* const pStr, size_t len) { return (U2W)(pStr, len); };
            auto pfn_u2a_stl = [](const char* const pStr, size_t len) { return (U2A)(pStr, len); };
            auto pfn_w2a_stl = [](const wchar_t* const pStr, size_t len) { return (W2A)(pStr, len); };
            auto pfn_w2u_stl = [](const wchar_t* const pStr, size_t len) { return (W2U)(pStr, len); };

            auto pfn_call = [&](auto pfn_ptr, auto pfn_stl)
            {
                __CHARSET_CONVERT_ARG arg(pfn_ptr, pfn_stl, reinterpret_cast<const _Ty*>(pStr), nSize, dst_charset_type);
                *pOutput = convert.gbk_to(std::move(arg), nOutput);
            };
#define __CONVERT_CALL(_s) *pOutput = convert.gbk_to(__CHARSET_CONVERT_ARG(pfn_##_s, pfn_##_s##_stl, reinterpret_cast<const _Ty*>(pStr), nInput, nSize, dst_charset_type), nOutput);
            switch (src_charset_type)
            {
            case CHARSET_TYPE_GBK:
            {
                // ������ַ�����GBK, �����������ת��ʲô���ı���
                switch (dst_charset_type)
                {
                case CHARSET_TYPE_GBK:
                    *pOutput = convert.copy_gbk(pInput, nInput, nOutput);
                    break;
                case CHARSET_TYPE_UTF8:
                    //pfn_call(pfn_a2u, pfn_a2u_stl);
                    __CONVERT_CALL(a2u);
                    //*pOutput = convert.gbk_to(pStr, nSize, pfn_a2u, nOutput);
                    break;
                case CHARSET_TYPE_UTF16_LE:
                    __CONVERT_CALL(a2w);
                    //pfn_call(pfn_a2w, pfn_a2w_stl);
                    //*pOutput = convert.gbk_to(pStr, nSize, pfn_a2w, nOutput);
                    break;
                case CHARSET_TYPE_UTF16_BE:
                    //pfn_call(pfn_a2w, pfn_a2w_stl);
                    //*pOutput = convert.gbk_to(pStr, nSize, pfn_a2w, nOutput);
                    //__CHARSET_CONVERT::le_be_convert(*pOutput, nOutput);
                    break;
                default:
                    return false;
                }
                break;
            }
            //case CHARSET_TYPE_UTF8:
            //{
            //    // ������ַ�����UTF8, �����������ת��ʲô���ı���
            //    switch (dst_charset_type)
            //    {
            //    case CHARSET_TYPE_GBK:
            //        *pOutput = convert.utf8_to(pStr, nSize, pfn_u2a, nOutput);
            //        break;
            //    case CHARSET_TYPE_UTF8:
            //        *pOutput = convert.copy_utf8(pStr, nSize, nOutput);
            //        break;
            //    case CHARSET_TYPE_UTF16_LE:
            //        *pOutput = convert.utf8_to(pStr, nSize, pfn_u2w, nOutput);
            //        break;
            //    case CHARSET_TYPE_UTF16_BE:
            //        *pOutput = convert.utf8_to(pStr, nSize, pfn_u2w, nOutput);
            //        __CHARSET_CONVERT::le_be_convert(*pOutput, nOutput);
            //        break;
            //    default:
            //        return false;
            //    }
            //    break;
            //}
            //case CHARSET_TYPE_UTF16_LE:
            //{
            //    // ������ַ�����UTF16-LE, �����������ת��ʲô���ı���
            //    switch (dst_charset_type)
            //    {
            //    case CHARSET_TYPE_GBK:
            //        *pOutput = convert.utf16le_to(pStr, nSize, pfn_w2a, nOutput);
            //        break;
            //    case CHARSET_TYPE_UTF8:
            //        *pOutput = convert.utf16le_to(pStr, nSize, pfn_w2u, nOutput);
            //        break;
            //    case CHARSET_TYPE_UTF16_LE:
            //        *pOutput = convert.copy_utf16le(pStr, nSize, nOutput);
            //        break;
            //    case CHARSET_TYPE_UTF16_BE:
            //        *pOutput = convert.copy_utf16le(pStr, nSize, nOutput);
            //        __CHARSET_CONVERT::le_be_convert(*pOutput, nOutput);
            //        break;
            //    default:
            //        return false;
            //    }
            //    break;
            //}
            //case CHARSET_TYPE_UTF16_BE:
            //{
            //    // ������ַ�����UTF16-BE, �����������ת��ʲô���ı���
            //    switch (dst_charset_type)
            //    {
            //    case CHARSET_TYPE_GBK:
            //        *pOutput = convert.utf16be_to(pStr, nSize, pfn_w2a, nOutput);
            //        break;
            //    case CHARSET_TYPE_UTF8:
            //        *pOutput = convert.utf16be_to(pStr, nSize, pfn_w2u, nOutput);
            //        break;
            //    case CHARSET_TYPE_UTF16_LE:
            //        *pOutput = convert.copy_utf16be(pStr, nSize, nOutput);
            //        __CHARSET_CONVERT::le_be_convert(*pOutput, nOutput);
            //        break;
            //    case CHARSET_TYPE_UTF16_BE:
            //        *pOutput = convert.copy_utf16be(pStr, nSize, nOutput);
            //        break;
            //    default:
            //        return false;
            //    }
            //    break;
            //}
            default:
                return false;;
            }
            return true;
        }
#undef __CONVERT_CALL


    }


    // ��ָ���ڵ�����ת��GBK�ַ���
    // ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
    // size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
    // charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
    inline bool ptr2gbk(const void* ptr, size_t size, int charset_type, std::string& ret)
    {
        size_t nOutput = 0;
        __charsel_impl::__charset_convert_str_ptr(ptr, size, charset_type, CHARSET_TYPE_GBK, &ret, nOutput);
        return {};
    }
    // ��ָ���ڵ�����ת��GBK�ַ���
    // ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
    // size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
    // charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
    inline bool ptr2gbk(const void* ptr, size_t size, int charset_type, char*& ret)
    {

        return {};
    }

    // ��ָ���ڵ�����ת��UTF8�ַ���
    // ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
    // size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
    // charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
    inline bool ptr2utf8(const void* ptr, size_t size, int charset_type, std::string& ret)
    {

        return {};
    }
    // ��ָ���ڵ�����ת��UTF8�ַ���
    // ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
    // size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
    // charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
    inline bool ptr2utf8_ptr(const void* ptr, size_t size, int charset_type, char*& ret)
    {

        return {};
    }

    // ��ָ���ڵ�����ת��UTF16-LE�ַ���
    // ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
    // size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
    // charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
    inline bool ptr2utf16(const void* ptr, size_t size, int charset_type, std::wstring& ret)
    {

        return {};
    }
    // ��ָ���ڵ�����ת��UTF16-LE�ַ���
    // ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
    // size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
    // charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
    inline bool ptr2utf16(const void* ptr, size_t size, int charset_type, wchar_t*& ret)
    {

        return {};
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

        return {};
    }
    // ��ָ���ڵ�����ת��UTF16-BE�ַ���
    // ptr = Ҫת��������, Ҫת��UTF8���ı���ָ��, ���û�н�����־, ��size����ָ������
    // size = ptr���ֽ���, ���ptr��\0��β, ��size����Ϊ-1
    // charset_type = ��ʾptr�����ݵ��ַ�������, ���ptrû��BOM�Ͱ�����������ת��, 0: GBK, 1: UTF8, 2: UTF16_LE, 3: UTF16_BE
    inline bool ptr2utf16_ptrbe(const void* ptr, size_t size, int charset_type, wchar_t*& ret)
    {

        return {};
    }

}