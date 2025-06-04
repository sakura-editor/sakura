/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CNATIVE_D712C69D_7FA3_49CE_B63A_49273441B558_H_
#define SAKURA_CNATIVE_D712C69D_7FA3_49CE_B63A_49273441B558_H_
#pragma once

#include "mem/CMemory.h"

#include "util/string_ex.h"

/*!
 * 文字列バッファ管理クラス
 *
 * ヒープメモリにバッファ領域を確保する
 * ※CMemoryをprotect継承することにより、あまり自由にCMemoryを使えないようにしておく
 *
 * @author kobake
 * @date 2007/11/06 kobake 新規作成
 */
template<typename C>
class CNative : protected CMemory {
private:
	using Me = CNative<C>;
	using Base = CMemory;

public:
	using char_type = C;
	using string_type = std::basic_string<char_type>;
	using string_view_type = std::basic_string_view<char_type>;

	CNative() noexcept = default;

	CNative(
		_In_reads_(cchData)
		const char_type* pData,
		size_t cchData)
	{
		SetString(pData, cchData);
	}

	explicit CNative(string_view_type rhs) { SetString(rhs.data(), rhs.length()); }

	/* implicit */ CNative(_In_opt_z_ const char_type* pszData) { SetString(pszData); }
	/* implicit */ CNative(const string_type& rhs) { SetString(rhs.data(), rhs.length()); }

	CMemory*       _GetMemory()       { return static_cast<CMemory*>(this); }       //<! CMemoryのポインタを取得（旧コード互換用）
	const CMemory* _GetMemory() const { return static_cast<const CMemory*>(this); } //<! CMemoryのポインタを取得（旧コード互換用）

	void _SetStringLength(size_t nLength) { _SetRawLength(nLength * sizeof(char_type)); }

	/*!
	 * (重要：nDataLenは文字単位) バッファサイズの調整。必要に応じて拡大する。
	 */
	void AllocStringBuffer(size_t nDataLen) {
		AllocBuffer((nDataLen + 1) * sizeof(char_type));
	}

	void AppendString( _In_reads_( nDataLen ) const char_type* pData, size_t nDataLen ) { Base::AppendRawData( pData, nDataLen * sizeof( char_type ) ); }	//!< バッファの最後にデータを追加する。nLengthは文字単位。

	//! バッファの最後にデータを追加する
	void AppendString(_In_z_ const char_type* pszData) {
		if (pszData) {
			AppendString(pszData, auto_strlen(pszData));
		}
	}

	void AppendStringF(_In_z_ _Printf_format_string_ const char_type* pszFormat, ...)          //!< バッファの最後にデータを追加する (フォーマット機能付き)
	{
		// auto_vscprintf に NULL を渡してはならないので除外する
		if (!pszFormat || !*pszFormat) {
			throw std::invalid_argument("format can't be empty");
		}

		va_list v;
		va_start(v, pszFormat);

		// 整形によって追加される文字数をカウント
		const auto additional = auto_vscprintf(pszFormat, v);

		// 現在の文字列長を取得
		const auto currentLength = length();

		// 現在の文字数 + 追加文字数が収まるようにバッファを拡張する
		AllocStringBuffer(currentLength + additional);

		// 追加処理の実体はCRTに委譲。この関数は無効な書式を与えると即死する。
		auto_vsprintf_s(data() + currentLength, additional + 1, pszFormat, v);

		va_end(v);

		// 文字列終端を再設定する
		_SetStringLength(currentLength + additional);
	}

	const char_type* c_str() const noexcept { return data(); }

	void Clear() { _SetRawLength(0); } //!< 空っぽにする

	char_type*       data()        noexcept { return get<char_type>(); }
	const char_type* data()  const noexcept { return get<char_type>(); }

	bool    empty() const noexcept { return length() == 0; } //!< 空っぽかどうか

	char_type*       GetStringPtr()       { return data(); }
	const char_type* GetStringPtr() const { return data(); }

	int     GetStringLength() const { return Length(); }

	size_t  length() const noexcept { return Base::length<char_type>(); }
	int     Length() const noexcept { return int(length()); }

	//! バッファの内容を置き換える。nDataLenは文字単位。
	void SetString(
		_In_reads_(nDataLen)
		const char_type* pData,
		size_t nDataLen
	)
	{
		Base::SetRawData(pData, nDataLen * sizeof(char_type));
	}

	//! バッファの内容を置き換える
	void SetString(_In_opt_z_ const char_type* pszData) {
		if (!pszData) {
			Base::Reset();
		} else {
			SetString(pszData, auto_strlen(pszData));
		}
	}

	//! メモリ再確保を行わずに格納できる最大文字数を求める
	size_t size() const noexcept { return Base::capacity() / sizeof(char_type); }

	Me& operator = (string_view_type rhs) { SetString(rhs.data(), rhs.length()); return *this; }
	Me& operator = (_In_opt_z_ const char_type* rhs) { SetString(rhs); return *this; }
	Me& operator = (const string_type& rhs) { SetString(rhs.data(), rhs.length()); return *this; }
	Me& operator = (const char_type ch) { SetString(&ch, 1); return *this; }

	Me& operator += (string_view_type rhs) { AppendString(rhs.data(), rhs.length()); return *this; }
	Me& operator += (_In_z_ const char_type* rhs) { AppendString(rhs); return *this; }
	Me& operator += (const string_type& rhs) { AppendString(rhs.data(), rhs.length()); return *this; }
	Me& operator += (const char_type ch) { AppendString(&ch, 1); return *this; }

	explicit operator string_view_type() const noexcept { return string_view_type(data(), length()); }
	explicit operator string_type() const noexcept { return string_type(data(), length()); }
};

#endif /* SAKURA_CNATIVE_D712C69D_7FA3_49CE_B63A_49273441B558_H_ */
