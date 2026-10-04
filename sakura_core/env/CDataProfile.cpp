/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "env/CDataProfile.h"

#include "env/CShareData_IO.h"
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
			L"%u,%u,%06x,%06x,%u",
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
 * EditInfo型（編集情報）向けの特殊化。
 */
template <>
bool CDataProfile::IOProfileData<EditInfo>(
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		keyPrefix,		//!< [in] エントリ名
	EditInfo&				ei				//!< [in,out] エントリ値
)
{
	IOProfileData(sectionName, std::format(L"{}.nViewTopLine",	keyPrefix), ei.m_nViewTopLine);
	IOProfileData(sectionName, std::format(L"{}.nViewLeftCol",	keyPrefix), ei.m_nViewLeftCol);
	IOProfileData(sectionName, std::format(L"{}.nX",			keyPrefix), ei.m_ptCursor.x);
	IOProfileData(sectionName, std::format(L"{}.nY",			keyPrefix), ei.m_ptCursor.y);
	IOProfileData(sectionName, std::format(L"{}.nCharCode",		keyPrefix), ei.m_nCharCode);
	IOProfileData(sectionName, std::format(L"{}.szPath",		keyPrefix), ei.m_szPath);

	if (!IOProfileData(sectionName, std::format(L"{}.szMark2",	keyPrefix), ei.m_szMarkLines) &&
		IsReadingMode())
	{
		IOProfileData(sectionName, std::format(L"{}.szMark", keyPrefix), ei.m_szMarkLines);
	}

	IOProfileData(sectionName, std::format(L"{}.nType", keyPrefix), ei.m_nTypeId );

	return true;	// 返却値に意味はない
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
 * MacroRec型（マクロ設定データ）向けの特殊化。
 */
template<>
bool CDataProfile::IOProfileData<MacroRec>(
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		keySuffix,		//!< [in] エントリ名
	MacroRec&				macroRec		//!< [in,out] エントリ値
)
{
	decltype(macroRec.m_szName) szName{};
	decltype(macroRec.m_szFile) szFile{};

	if (!IOProfileData(sectionName, strprintf(L"Name%s", keySuffix), szName))
	{
		return false;
	}

	if (!IOProfileData(sectionName, strprintf(L"File%s", keySuffix), szFile))
	{
		return false;
	}

	IOProfileData(sectionName, strprintf(L"ReloadWhenExecute%s", keySuffix), macroRec.m_bReloadWhenExecute);

	::wcscpy_s(macroRec.m_szName, szName);
	::wcscpy_s(macroRec.m_szFile, szFile);

	return true;
}

/*!
 * @brief 設定値の入出力を行う。
 *
 * PRINTSETTING型（印刷設定データ）向けの特殊化。
 */
template<>
bool CDataProfile::IOProfileData<PRINTSETTING>(
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		keyPrefix,		//!< [in] エントリ名
	PRINTSETTING&			printSetting	//!< [in,out] エントリ値
)
{
	ShareData_IO_PrintInts(*this, sectionName, strprintf(L"%s.nInts", keyPrefix), printSetting);

	IOProfileData(sectionName, strprintf(L"%s.szSName", keyPrefix), printSetting.m_szPrintSettingName);
	IOProfileData(sectionName, strprintf(L"%s.szFF",    keyPrefix), printSetting.m_szPrintFontFaceHan);
	IOProfileData(sectionName, strprintf(L"%s.szFFZ",   keyPrefix), printSetting.m_szPrintFontFaceZen);

	// ヘッダー/フッター
	for (int j = 0; j < 3; ++j) {
		IOProfileData(sectionName, strprintf(L"%s.szHF[%d]" , keyPrefix, j), printSetting.m_szHeaderForm[j]);
		IOProfileData(sectionName, strprintf(L"%s.szFTF[%d]", keyPrefix, j), printSetting.m_szFooterForm[j]);
	}

	// ヘッダー/フッター フォント設定
	ShareData_IO_LogFont(
		*this,
		sectionName,
		strprintf(L"%s.lfHeader",			keyPrefix),
		printSetting.m_lfHeader,
		printSetting.m_nHeaderPointSize,
		strprintf(L"%s.nHeaderPointSize",	keyPrefix),
		strprintf(L"%s.lfHeaderFaceName",	keyPrefix)
	);

	ShareData_IO_LogFont(
		*this,
		sectionName,
		strprintf(L"%s.lfFooter",			keyPrefix),
		printSetting.m_lfFooter,
		printSetting.m_nFooterPointSize,
		strprintf(L"%s.nFooterPointSize",	keyPrefix),
		strprintf(L"%s.lfFooterFaceName",	keyPrefix)
	);

	IOProfileData(sectionName, strprintf(L"%s.szDriver", keyPrefix, keyPrefix), printSetting.m_mdmDevMode.m_szPrinterDriverName);
	IOProfileData(sectionName, strprintf(L"%s.szDevice", keyPrefix, keyPrefix), printSetting.m_mdmDevMode.m_szPrinterDeviceName);
	IOProfileData(sectionName, strprintf(L"%s.szOutput", keyPrefix, keyPrefix), printSetting.m_mdmDevMode.m_szPrinterOutputName);

	//禁則
	IOProfileData(sectionName, strprintf(L"%s.bKinsokuHead", keyPrefix, keyPrefix), printSetting.m_bPrintKinsokuHead);
	IOProfileData(sectionName, strprintf(L"%s.bKinsokuTail", keyPrefix, keyPrefix), printSetting.m_bPrintKinsokuTail);
	IOProfileData(sectionName, strprintf(L"%s.bKinsokuRet",  keyPrefix, keyPrefix), printSetting.m_bPrintKinsokuRet);
	IOProfileData(sectionName, strprintf(L"%s.bKinsokuKuto", keyPrefix, keyPrefix), printSetting.m_bPrintKinsokuKuto);

	//カラー印刷
	IOProfileData(sectionName, strprintf(L"%s.bColorPrint", keyPrefix), printSetting.m_bColorPrint);

	return true;	// 返却値に意味はない
}

/*!
 * @brief 設定値の入出力を行う。
 *
 * PluginRec型（プラグイン設定データ）向けの特殊化。
 */
template<>
bool CDataProfile::IOProfileData<PluginRec>(
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		keyPrefix,		//!< [in] エントリ名
	PluginRec&				pluginRec		//!< [in,out] エントリ値
)
{
	decltype(pluginRec.m_szName) szName{};
	decltype(pluginRec.m_szId) szId{};

	if (!IOProfileData(sectionName, std::format(L"{}.Name", keyPrefix), szName))
	{
		return false;
	}

	if (!IOProfileData(sectionName, std::format(L"{}.Id", keyPrefix), szId))
	{
		return false;
	}

	if (!IOProfileData(sectionName, std::format(L"{}.CmdNum", keyPrefix), pluginRec.m_nCmdNum))
	{
		return false;
	}

	::wcscpy_s(pluginRec.m_szName, szName);
	::wcscpy_s(pluginRec.m_szId, szId);

	return true;
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
