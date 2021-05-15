#pragma once
/*!
 * @file encoding.h
 * @brief 文字列のエンコーディングヘッダ
 */

#include "system/h-type.h"

#include <vector>

/*!
 * 文字列クラス
 * @details
 * char配列文字列と置き換えて使用するためのvectorバッファ実装の文字列クラス。
 */
template <typename T>
class vector_string {
public:
    static vector_string<T> empty()
    {
        return vector_string<T>(0);
    }
    /*!
     * コンストラクタ.
     * @param sz バッファサイズ
     */
    vector_string(uint sz)
    {
        set_size(sz);
    }

    vector_string()
    {
        set_size(1024);
    }

    vector_string(vector_string<T> &&src) noexcept
    {
        this->buf = std::move(src.buf);
    }

    ~vector_string() = default;
    vector_string(const vector_string<T> &vs)
    {
        this->copy_from(vs);
    }
    vector_string<T> &operator=(const vector_string<T> &vs)
    {
        this->copy_from(vs);
        return *this;
    }

    void copy_from(const vector_string<T> &vs)
    {
        if (this == &vs)
            return;
        if (this->buf.size() < vs.buf.size())
            this->buf.resize(vs.buf.size());
        std::copy(vs.buf.begin(), vs.buf.end(), this->buf.begin());
    }

    /*!
     * バッファへのポインタを参照する
     * @return バッファへのポインタ
     */
    const T *c_ptr()
    {
        return &buf[0];
    }

    /*!
     * バッファへのポインタを取得する
     * @return バッファへのポインタ
     */
    T *raw_ptr()
    {
        return &buf[0];
    }

    /*!
     * 新しいバッファサイズを設定する
     * @param sz 新しいバッファサイズ
     */
    void set_size(uint sz)
    {
        buf.resize(sz);
    }

    /*!
     * バッファサイズを取得する
     * @return バッファサイズ
     */
    uint get_size()
    {
        return buf.size();
    }

protected:
    std::vector<T> buf;
};

/*!
 * 内部コード文字列.
 * @details
 * Windowsでは文字コードはShift_JIS。
 */
using system_string = vector_string<char>;
/*!
 * システムロケール文字列.
 * @details
 * 文字コードがシステムロケールの文字列。
 */
using ansi_string = vector_string<char>;
using utf8_string = vector_string<char>;
using utf16_string = vector_string<wchar_t>;

#if defined(SJIS) && defined(WINDOWS)
utf16_string to_utf16(const wchar_t *s);
utf16_string to_utf16(const char *s);
utf16_string to_utf16(system_string &ss);
system_string to_system(const char *s);
system_string to_system(utf16_string &u16str);
#endif
