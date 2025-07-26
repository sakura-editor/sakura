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
 * 文字列バッファ管理クラステンプレート
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

	/*!
	 * デフォルトコンストラクタ
	 *
	 * メモリを確保せずに構築。
	 */
	CNative() noexcept = default;

	/*!
	 * 文字列ポインタとサイズを指定して構築。
	 */
	CNative(
		_In_reads_(cchData)
		const char_type*	pData,	//!< [in] 文字列ポインタ
		size_t				cchData	//!< [in] 文字数
	)
	{
		// CMemory派生クラスにはメンバー追加禁止
		static_assert(sizeof(Me) == sizeof(CMemory));

		SetString(pData, cchData);
	}

	//! 文字列参照を指定して構築。
	explicit CNative(string_view_type rhs) { SetString(rhs.data(), rhs.length()); }

	/*!
	 * 文字列ポインタを指定して構築。
	 *
	 * 以下のような記述をできるようにexplicitは付けない。
	 * CNativeW buf = L"value";
	 */
	/* implicit */ CNative(_In_opt_z_ const char_type* pszData) { SetString(pszData); }

	/*!
	 * 文字列を指定して構築。
	 *
	 * 以下のような記述をできるようにexplicitは付けない。
	 * CNativeW buf = L"value"s;
	 * 
	 * if (std::wsmatch m; std::regex_match(buf, m, std::wregex(LR"(^(val).*$)"))) {
	 * 	CNativeW matched = m[1];
	 * }
	 */
	/* implicit */ CNative(const string_type& rhs) { SetString(rhs.data(), rhs.length()); }

	CMemory*       _GetMemory()       { return static_cast<CMemory*>(this); }       //<! CMemoryのポインタを取得（旧コード互換用、使用すべきでない）
	const CMemory* _GetMemory() const { return static_cast<const CMemory*>(this); } //<! CMemoryのポインタを取得（旧コード互換用、使用すべきでない）

	/*!
	 * 文字列サイズを変更する。
	 *
	 * 指定した位置にNUL終端を書き込み、文字列サイズを変更する。
	 * このメソッドはバッファサイズを変更しない。
	 */
	void _SetStringLength(
		size_t newLength
	)
	{
		_SetRawLength(newLength * sizeof(char_type));
	}

	/*!
	 * バッファサイズの調整。
	 *
	 * 要求サイズに満たない場合バッファを拡張する。
	 */
	void AllocStringBuffer(
		size_t required	//!< [in] 要求文字数
	)
	{
		AllocBuffer((required + 1) * sizeof(char_type));
	}

	//! バッファの最後にデータを追加する。
	void AppendString(
		_In_reads_(cchData)
		const char_type*	pData,	//!< [in] 文字列ポインタ
		size_t				cchData	//!< [in] 文字数
	)
	{
		Base::AppendRawData(pData, cchData * sizeof(char_type));
	}

	//! バッファの最後にデータを追加する。
	void AppendString(_In_z_ const char_type* pszData) {
		if (pszData) {
			AppendString(pszData, auto_strlen(pszData));
		}
	}

	/*!
	 * バッファの最後に書式化したデータを追加する。
	 *
	 * 旧コードとの互換性のため残しておく。
	 */
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

		// バッファが有効な場合のみ、フォーマットを行う
		if (data()) {
			// 追加処理の実体はCRTに委譲。この関数は無効な書式を与えると即死する。
			auto_vsprintf_s(data() + currentLength, additional + 1, pszFormat, v);
		}

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

	//! バッファの内容を置き換える。
	void SetString(
		_In_reads_(cchData)
		const char_type*	pData,	//!< [in] 文字列ポインタ
		size_t				cchData	//!< [in] 文字数
	)
	{
		Base::SetRawData(pData, cchData * sizeof(char_type));
	}

	//! バッファの内容を置き換える。
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
