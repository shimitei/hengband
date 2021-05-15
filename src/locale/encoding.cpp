/*!
 * @file encoding.h
 * @brief 文字列のエンコーディング
 */

#include "locale/encoding.h"

#ifdef WINDOWS
#include <windows.h>
#endif

#if defined(SJIS) && defined(WINDOWS)
utf16_string to_utf16(const wchar_t *s)
{
    if (s == NULL)
        return utf16_string::empty();

    utf16_string result(wcslen(s) + 1);
    wcscpy(result.raw_ptr(), s);
    return result;
}

utf16_string to_utf16(const char *s)
{
    utf16_string result(0);
    int size = ::MultiByteToWideChar(932, 0, s, -1, NULL, 0);
    if (size > 0) {
        result.set_size(size + 1);
        if (::MultiByteToWideChar(932, 0, s, -1, result.raw_ptr(), result.get_size()) == 0) {
            // fail
        }
    }

    return result;
}

utf16_string to_utf16(system_string &ss)
{
    return to_utf16(ss.c_ptr());
}

system_string to_system(const char *s)
{
    if (s == NULL)
        return system_string::empty();

    system_string result(strlen(s) + 1);
    strcpy(result.raw_ptr(), s);
    return result;
}

system_string to_system(utf16_string &u16str)
{
    system_string result(0);
    int size = ::WideCharToMultiByte(932, 0, u16str.raw_ptr(), -1, NULL, 0, NULL, NULL);
    if (size > 0) {
        result.set_size(size + 1);
        if (::WideCharToMultiByte(932, 0, u16str.raw_ptr(), -1, result.raw_ptr(), result.get_size(), NULL, NULL) == 0) {
            // fail
        }
    }

    return result;
}
#endif
