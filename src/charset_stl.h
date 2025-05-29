#pragma once

#define _CHARSETSTL_BEGIN namespace charset_stl {
#define _CHARSETSTL_END   }
_CHARSETSTL_BEGIN

// Unicode转ansi, 返回的结果需要调用 delete[] 释放
inline char* UnicodeToAnsi(const wchar_t* const unicode, size_t len = 0, int cp = 936)
{
    if (!unicode)
        return 0;
    if (len <= 0)
        len = wcslen(unicode);
    if (len <= 0)
        return 0;
    int convertResult = WideCharToMultiByte(cp, 0, unicode, (int)len, 0, 0, 0, 0);
    if (convertResult <= 0)
        return 0;

    int size = convertResult + 10;
    char* szStr = new char[size];
    memset(szStr, 0, size);
    WideCharToMultiByte(cp, 0, unicode, (int)len, szStr, size, 0, 0);
    return szStr;
}

// ansi转Unicode, 返回的结果需要调用 delete[] 释放
inline wchar_t* AnsiToUnicode(const char* const ansi, size_t len = 0, int cp = 936)
{
    if (!ansi)
        return 0;
    if (len <= 0)
        len = strlen(ansi);
    if (len <= 0)
        return 0;

    int convertResult = MultiByteToWideChar(cp, 0, ansi, (int)len, NULL, 0);
    if (convertResult <= 0)
        return 0;

    int size = convertResult + 10;;
    wchar_t* wzStr = new wchar_t[size];
    memset(wzStr, 0, size * sizeof(wchar_t));
    MultiByteToWideChar(cp, 0, ansi, (int)len, wzStr, size);
    return wzStr;
}

// unicode转UTF8, 返回的结果需要调用 delete[] 释放
inline char* UnicodeToUtf8(const wchar_t* const str, size_t len = 0)
{
    return UnicodeToAnsi(str, len, CP_UTF8);
}

// UTF8转unicode, 返回的结果需要调用 delete[] 释放
inline wchar_t* Utf8ToUnicode(const char* const str, size_t len = 0)
{
    return AnsiToUnicode(str, len, CP_UTF8);
}

// UTF8转ansi, 返回的结果需要调用 delete[] 释放
inline char* Utf8ToAnsi(const char* const utf8, size_t len = 0)
{
    // 先把utf8文本转成unicode, 在把unicode转成ansi
    if (!utf8)return 0;
    wchar_t* unicode = Utf8ToUnicode(utf8, len);
    if (!unicode) return 0;

    char* ansi = UnicodeToAnsi(unicode, wcslen(unicode));
    delete[] unicode;
    return ansi;
}

// ansi转UTF8, 返回的结果需要调用 delete[] 释放
inline char* AnsiToUtf8(const char* const ansi, size_t len = 0)
{
    // 先把ansi文本转成unicode, 在把unicode转成utf8
    if (!ansi)return 0;
    wchar_t* unicode = AnsiToUnicode(ansi, len);
    if (!unicode) return 0;

    char* utf8 = UnicodeToAnsi(unicode, wcslen(unicode), CP_UTF8);
    delete[] unicode;
    return utf8;
}



// Unicode转ansi
inline std::string (W2A)(const wchar_t* const unicode, size_t len = 0, int cp = 936)
{
    std::string result;
    if (!unicode)
        return result;
    if (len <= 0)
        len = wcslen(unicode);
    if (len <= 0)
        return result;

    int convertResult = WideCharToMultiByte(cp, 0, unicode, (int)len, 0, 0, 0, 0);
    if (convertResult <= 0)
        return result;

    result.resize(static_cast<size_t>(convertResult) + 10);

    int ret_len = WideCharToMultiByte(cp, 0, unicode, (int)len, &result[0], (int)result.size(), 0, 0);
    result.resize(ret_len);
    return result;
}
inline std::string(W2A)(const std::wstring& str, int cp = 936)
{
    return (W2A)(str.c_str(), str.size(), cp);
}
// ansi转Unicode
inline std::wstring (A2W)(const char* const ansi, size_t len = 0, int cp = 936)
{
    std::wstring result;
    if (!ansi)
        return result;
    if (len <= 0)
        len = strlen(ansi);
    if (len <= 0)
        return result;

    int convertResult = MultiByteToWideChar(cp, 0, ansi, (int)len, NULL, 0);
    if (convertResult <= 0)
        return result;

    result.resize(static_cast<size_t>(convertResult) + 10);

    int ret_len = MultiByteToWideChar(cp, 0, ansi, (int)len, &result[0], (int)result.size());
    result.resize(ret_len);
    return result;
}
inline std::wstring(A2W)(const std::string& str, int cp = 936)
{
    return (A2W)(str.c_str(), str.size(), cp);
}

// unicode转UTF8
inline std::string W2U(const wchar_t* const str, size_t len = 0)
{
    return (W2A)(str, len, CP_UTF8);
}

// unicode转UTF8
inline std::string W2U(const std::wstring& str)
{
    return (W2A)(str.c_str(), str.size(), CP_UTF8);
}

// UTF8转unicode
inline std::wstring U2W(const char* const str, size_t len = 0)
{
    return (A2W)(str, len, CP_UTF8);
}
// UTF8转unicode
inline std::wstring U2W(const std::string& str)
{
    return (A2W)(str.c_str(), str.size(), CP_UTF8);
}

// UTF8转ansi
inline std::string U2A(const char* const utf8, size_t len = 0)
{
    // 先把utf8文本转成unicode, 在把unicode转成ansi
    if (!utf8)
        return std::string();
    std::wstring unicode = U2W(utf8, len);
    if (unicode.empty())
        return std::string();
    return (W2A)(unicode.c_str(), unicode.length());
}

// UTF8转ansi
inline std::string U2A(const std::string& str)
{
    return (U2A)(str.c_str(), str.size());
}

// ansi转UTF8
inline std::string A2U(const char* const ansi, size_t len = 0)
{
    // 先把ansi文本转成unicode, 在把unicode转成utf8
    if (!ansi)
        return std::string();
    std::wstring unicode = (A2W)(ansi, len);
    if (unicode.empty())
        return std::string();
    return (W2A)(unicode.c_str(), unicode.length(), CP_UTF8);
}

// ansi转UTF8
inline std::string A2U(const std::string& str)
{
    return (A2U)(str.c_str(), str.size());
}


_CHARSETSTL_END