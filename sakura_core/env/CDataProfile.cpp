/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "env/CDataProfile.h"

/*!
 * @brief 設定値の入出力を行う。
 *
 * RECT型（矩形データ）向けの特殊化。
 */
template <>
bool CDataProfile::IOProfileData<RECT>(
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		entryKey,		//!< [in] エントリ名
	RECT&					rcEntryValue	//!< [in,out] エントリ値
)
{
	// 取得・設定は文字列を介して行う
	std::wstring buffer{};

	// 書き込みモード
	if (IsWritingMode()) {
		strprintf(
			buffer,
			L"%d,%d,%d,%d",
			rcEntryValue.left,
			rcEntryValue.top,
			rcEntryValue.right,
			rcEntryValue.bottom
		);
	}

	// 文字列を介して読み書きする
	const auto ret = IOProfileData(sectionName, entryKey, buffer);
	if (!ret) {
		return false;	// 読み込み失敗（書き込みは失敗しない）
	}

	// 読み込みモード
	if (IsReadingMode()) {
		std::array<int, 4> ints{};
		if (4 != ::swscanf_s(
			buffer.c_str(),
			L"%d,%d,%d,%d",
			&ints[0],
			&ints[1],
			&ints[2],
			&ints[3]
		))
		{
			return false;	// 4個揃わなければ失敗とする
		}

		rcEntryValue.left	= ints[0];
		rcEntryValue.top	= ints[1];
		rcEntryValue.right	= ints[2];
		rcEntryValue.bottom	= ints[3];
	}

	return ret;
}
