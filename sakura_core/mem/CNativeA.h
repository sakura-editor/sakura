/*! @file */
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include "CNative.h"

class CNativeA final : public CNative
{
  public:
    CNativeA() noexcept;
    CNativeA(const CNativeA &rhs);
    CNativeA(CNativeA &&other) noexcept;
    CNativeA(const char *szData, size_t cchData);
    CNativeA(const char *szData);

    //ネイティブ設定
    void SetString(const char *pszData); //!< バッファの内容を置き換える
    void SetString(const char *pData, int nDataLen); //!< バッファの内容を置き換える。nDataLenは文字単位。
    void SetNativeData(const CNativeA &pcNative); //!< バッファの内容を置き換える
    void AppendString(const char *pszData); //!< バッファの最後にデータを追加する
    void AppendString(const char *pszData, int nLength); //!< バッファの最後にデータを追加する。nLengthは文字単位。
    void AppendStringF(const char *pszData, ...); //!< バッファの最後にデータを追加する (フォーマット機能付き)
    void AppendNativeData(const CNativeA &pcNative); //!< バッファの最後にデータを追加する
    void AllocStringBuffer(int nDataLen); //!< (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。

    //ネイティブ取得
    int GetStringLength() const;
    char operator[](int nIndex) const; //!< 任意位置の文字取得。nIndexは文字単位。
    const char *GetStringPtr() const
    {
        return reinterpret_cast<const char *>(GetRawPtr());
    }
    char *GetStringPtr()
    {
        return reinterpret_cast<char *>(GetRawPtr());
    }

    //演算子
    CNativeA &operator=(const CNativeA &rhs)
    {
        CNative::operator=(rhs);
        return *this;
    }
    CNativeA &operator=(CNativeA &&rhs) noexcept
    {
        CNative::operator=(std::forward<CNativeA>(rhs));
        return *this;
    }
    const CNativeA &operator=(char);
    const CNativeA &operator+=(char);

    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
    //                           変換                              //
    // -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

    //ネイティブ変換
    void Replace_j(const char *pszFrom, const char *pszTo); //!< 文字列置換（日本語考慮版）
};
