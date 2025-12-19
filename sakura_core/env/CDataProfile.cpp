/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CDataProfile.h"

#include <stdexcept>

/*!
	コンストラクタ
 */
StringBufferW::StringBufferW(WCHAR* pData, size_t maxCount)
	: pszData_(pData)
	, cchDataSize_(maxCount)
{
	if (pszData_ == nullptr) {
		throw std::invalid_argument("data can't be nullptr");
	}

	if (cchDataSize_ == 0) {
		throw std::invalid_argument("count can't be zero");
	}
}

/*!
	代入演算子
 */
StringBufferW& StringBufferW::operator = (std::wstring_view rhs)
{
	::wcsncpy_s(pszData_, cchDataSize_, rhs.data(), _TRUNCATE);

	return *this;
}

namespace profile_data {

	//! 設定値を文字列に変換する
	[[nodiscard]] std::wstring ToString(const StringBufferW& value)
	{
		return std::wstring(value.c_str());
	}

}; // end of namespace profile_data

/*!
 * 独自定義バッファ参照型(StringBufferW)の入出力(右辺値参照バージョン)
 *
 * @retval true	設定値を正しく読み書きできた
 * @retval false 設定値を読み込めなかった
 */
bool CDataProfile::IOProfileData(
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		entryKey,		//!< [in] エントリ名
	StringBufferW&&			refEntryValue	//!< [in,out] エントリ値
)
{
	// 右辺値参照引数(=左辺値)を使って入出力テンプレートを呼び出す
	auto entryValue(std::move(refEntryValue));
	return IOProfileData(sectionName, entryKey, entryValue);
}
