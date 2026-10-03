/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "env/CDataProfile.h"

#include "env/DLLSHAREDATA.h"

/*!
 * @brief 設定値の入出力を行う。
 *
 * ColorInfo型（色設定データ）向けの特殊化。
 */
template<>
bool CDataProfile::IOProfileData<ColorInfo>(
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		entryKey,		//!< [in] エントリ名
	ColorInfo&				colorInfo		//!< [in,out] エントリ値
)
{
	// 入出力形式
	//   C[XXX]=ON/OFF,IsBold,TextColor,BackColor,HasUnderLine

	// 取得・設定は文字列を介して行う
	std::wstring buffer{};

	// 書き込みモード
	if (IsWritingMode()) {
		// 色設定データを書式化する
		strprintf(
			buffer,
			L"%d,%d,%06x,%06x,%d",
			colorInfo.m_bDisp ? 1 : 0,
			colorInfo.m_sFontAttr.m_bBoldFont ? 1 : 0,
			colorInfo.m_sColorAttr.m_cTEXT,
			colorInfo.m_sColorAttr.m_cBACK,
			colorInfo.m_sFontAttr.m_bUnderLine ? 1 : 0
		);
	}

	// 文字列を介して読み書きする
	const auto ret = IOProfileData(sectionName, entryKey, buffer);
	if (!ret) {
		return false;	// 読み込み失敗（書き込みは失敗しない）
	}

	// 読み込みモード
	if (IsReadingMode()) {
		// 文字列から色設定データを構築する
		std::array<unsigned, 5> ints{};
		if (5 != ::swscanf_s(
			buffer.c_str(),
			L"%d,%d,%06x,%06x,%d",
			&ints[0],
			&ints[1],
			&ints[2],
			&ints[3],
			&ints[4]
		))
		{
			return false;
		}

		colorInfo.m_bDisp					= ints[0] != 0;
		colorInfo.m_sFontAttr.m_bBoldFont	= ints[1] != 0;
		colorInfo.m_sColorAttr.m_cTEXT		= ints[2];
		colorInfo.m_sColorAttr.m_cBACK		= ints[3];
		colorInfo.m_sFontAttr.m_bUnderLine	= ints[4] != 0;
	}

	return ret;
}

/*!
 * @brief 設定値の入出力を行う。
 *
 * KeyHelpInfo型（辞書データ）向けの特殊化。
 */
template<>
bool CDataProfile::IOProfileData<KeyHelpInfo>(
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		entryKey,		//!< [in] エントリ名
	KeyHelpInfo&			keyHelpInfo		//!< [in,out] エントリ値
)
{
	// 入出力形式
	//   KDct[99]=ON/OFF,DictAbout,KeyHelpPath

	// 取得・設定は文字列を介して行う
	std::wstring buffer{};

	// 書き込みモード
	if (IsWritingMode()) {
		if (keyHelpInfo.m_szPath.empty()) {
			return false;	// パスが空なら保存しない
		}

		// 辞書データを書式化する
		strprintf(
			buffer,
			L"%d,%s,%s",
			keyHelpInfo.m_bUse ? 1 : 0,
			keyHelpInfo.m_szAbout,
			keyHelpInfo.m_szPath
		);
	}

	// 文字列を介して読み書きする
	const auto ret = IOProfileData(sectionName, entryKey, buffer);
	if (!ret) {
		return false;	// 読み込み失敗（書き込みは失敗しない）
	}

	// 読み込みモード
	if (IsReadingMode()) {
		// 文字列から辞書データを構築する
		std::array<int, 1> ints{};
		auto szAbout = keyHelpInfo.m_szAbout;
		auto szPath = keyHelpInfo.m_szPath;
		if (3 != ::swscanf_s(
			buffer.c_str(),
			L"%hhu,%[^,],%[^\n]",
			&ints[0],
			szAbout.data(), unsigned(std::size(szAbout)),
			szPath.data(), unsigned(std::size(szPath))
		))
		{
			// 1つ目の値が不正（数値でない）
			// 2つ目の値が不正（文字数超過）
			// 3つ目の値が不正（文字数超過、カンマが足りない）
			return false;
		}

		// 構築した値をコピー代入して呼出元に返す
		keyHelpInfo.m_bUse		= ints[0] != 0;
		keyHelpInfo.m_szAbout	= szAbout;
		keyHelpInfo.m_szPath	= szPath;
	}

	return ret;
}

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

/*!
 * @brief 設定値の入出力を行う。
 *
 * SFileTreeItem型（ツリー項目データ）向けの特殊化。
 */
template <>
bool CDataProfile::IOProfileData<SFileTreeItem>(
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		keyPrefix,		//!< [in] エントリ名
	SFileTreeItem&			fileTreeItem		//!< [in,out] エントリ値
)
{
	IOProfileData(sectionName, std::format(L"{}.eItemType", keyPrefix), fileTreeItem.m_eFileTreeItemType);

	if (IsReadingMode() ||
		fileTreeItem.m_eFileTreeItemType == EFileTreeItemType_Grep ||
		fileTreeItem.m_eFileTreeItemType == EFileTreeItemType_File)
	{
		IOProfileData(sectionName, std::format(L"{}.szTargetPath", keyPrefix), fileTreeItem.m_szTargetPath);
	}

	if (IsReadingMode() ||
		fileTreeItem.m_eFileTreeItemType == EFileTreeItemType_Folder ||
		!fileTreeItem.m_szLabelName.empty())
	{
		IOProfileData(sectionName, std::format(L"{}.szLabelName", keyPrefix), fileTreeItem.m_szLabelName);
	}

	IOProfileData(sectionName, std::format(L"{}.nDepth", keyPrefix), fileTreeItem.m_nDepth);

	if (IsReadingMode() ||
		fileTreeItem.m_eFileTreeItemType == EFileTreeItemType_Grep)
	{
		IOProfileData(sectionName, std::format(L"{}.szTargetFile",   keyPrefix), fileTreeItem.m_szTargetFile);
		IOProfileData(sectionName, std::format(L"{}.bIgnoreHidden",  keyPrefix), fileTreeItem.m_bIgnoreHidden);
		IOProfileData(sectionName, std::format(L"{}.bIgnoreReadOny", keyPrefix), fileTreeItem.m_bIgnoreReadOnly);
		IOProfileData(sectionName, std::format(L"{}.bIgnoreSystem",  keyPrefix), fileTreeItem.m_bIgnoreSystem);
	}

	return true;	// 返却値に意味はない
}
