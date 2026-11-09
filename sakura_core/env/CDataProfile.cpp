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
	sakura::wstring_view		sectionName,	//!< [in] セクション名
	sakura::wstring_view		entryKey,		//!< [in] エントリ名
	ColorInfo&				colorInfo		//!< [in,out] エントリ値
)
{
	// 入出力形式
	//   C[XXX]=ON/OFF,IsBold,TextColor,BackColor,HasUnderLine

	// 取得・設定は文字列を介して行う
	sakura::wstring buffer{};

	// 書き込みモード
	if (IsWritingMode()) {
		// 色設定データを書式化する
		strprintf(
			buffer,
			LTEXT("%d,%d,%06x,%06x,%d"),
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
			LTEXT("%u,%u,%06x,%06x,%u"),
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
	sakura::wstring_view		sectionName,	//!< [in] セクション名
	sakura::wstring_view		keyPrefix,		//!< [in] エントリ名
	EditInfo&				ei				//!< [in,out] エントリ値
)
{
	IOProfileData(sectionName, std::format(LTEXT("{}.nViewTopLine"),	keyPrefix), ei.m_nViewTopLine);
	IOProfileData(sectionName, std::format(LTEXT("{}.nViewLeftCol"),	keyPrefix), ei.m_nViewLeftCol);
	IOProfileData(sectionName, std::format(LTEXT("{}.nX"),			keyPrefix), ei.m_ptCursor.x);
	IOProfileData(sectionName, std::format(LTEXT("{}.nY"),			keyPrefix), ei.m_ptCursor.y);
	IOProfileData(sectionName, std::format(LTEXT("{}.nCharCode"),		keyPrefix), ei.m_nCharCode);
	IOProfileData(sectionName, std::format(LTEXT("{}.szPath"),		keyPrefix), ei.m_szPath);

	if (!IOProfileData(sectionName, std::format(LTEXT("{}.szMark2"),	keyPrefix), ei.m_szMarkLines) &&
		IsReadingMode())
	{
		IOProfileData(sectionName, std::format(LTEXT("{}.szMark"), keyPrefix), ei.m_szMarkLines);
	}

	IOProfileData(sectionName, std::format(LTEXT("{}.nType"), keyPrefix), ei.m_nTypeId );

	return true;	// 返却値に意味はない
}

/*!
 * @brief 設定値の入出力を行う。
 *
 * KeyHelpInfo型（辞書データ）向けの特殊化。
 */
template<>
bool CDataProfile::IOProfileData<KeyHelpInfo>(
	sakura::wstring_view		sectionName,	//!< [in] セクション名
	sakura::wstring_view		entryKey,		//!< [in] エントリ名
	KeyHelpInfo&			keyHelpInfo		//!< [in,out] エントリ値
)
{
	// 入出力形式
	//   KDct[99]=ON/OFF,DictAbout,KeyHelpPath

	// 取得・設定は文字列を介して行う
	sakura::wstring buffer{};

	// 書き込みモード
	if (IsWritingMode()) {
		if (keyHelpInfo.m_szPath.empty()) {
			return false;	// パスが空なら保存しない
		}

		// 辞書データを書式化する
		strprintf(
			buffer,
			LTEXT("%d,%s,%s"),
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
			LTEXT("%d,%[^,],%[^\n]"),
			&ints[0],
			szAbout.data(), unsigned(std::size(szAbout)),
			szPath.data(), unsigned(std::size(szPath))
		))
		{
			// szAbout部分が空の場合も拾う
			if (2 == ::swscanf_s(
				buffer.c_str(),
				LTEXT("%d,,%[^\n]"),
				&ints[0],
				szPath.data(), unsigned(std::size(szPath))
			))
			{
				szAbout = LTEXT("");
			}
			else {
				// 1つ目の値が不正（数値でない）
				// 2つ目の値が不正（文字数超過）
				// 3つ目の値が不正（文字数超過、カンマが足りない）
				return false;
			}
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
	sakura::wstring_view		sectionName,	//!< [in] セクション名
	sakura::wstring_view		keySuffix,		//!< [in] エントリ名
	MacroRec&				macroRec		//!< [in,out] エントリ値
)
{
	decltype(macroRec.m_szName) szName{};
	decltype(macroRec.m_szFile) szFile{};

	// 書き込みモード
	if (IsWritingMode()) {
		::wcscpy_s(szName, macroRec.m_szName);
		::wcscpy_s(szFile, macroRec.m_szFile);
	}

	if (!IOProfileData(sectionName, strprintf(LTEXT("Name%s"), keySuffix), szName))
	{
		return false;
	}

	if (!IOProfileData(sectionName, strprintf(LTEXT("File%s"), keySuffix), szFile))
	{
		return false;
	}

	IOProfileData(sectionName, strprintf(LTEXT("ReloadWhenExecute%s"), keySuffix), macroRec.m_bReloadWhenExecute);

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
	sakura::wstring_view		sectionName,	//!< [in] セクション名
	sakura::wstring_view		keyPrefix,		//!< [in] エントリ名
	PRINTSETTING&			printSetting	//!< [in,out] エントリ値
)
{
	ShareData_IO_PrintInts(*this, sectionName, strprintf(LTEXT("%s.nInts"), keyPrefix), printSetting);

	IOProfileData(sectionName, strprintf(LTEXT("%s.szSName"), keyPrefix), printSetting.m_szPrintSettingName);
	IOProfileData(sectionName, strprintf(LTEXT("%s.szFF"),    keyPrefix), printSetting.m_szPrintFontFaceHan);
	IOProfileData(sectionName, strprintf(LTEXT("%s.szFFZ"),   keyPrefix), printSetting.m_szPrintFontFaceZen);

	// ヘッダー/フッター
	for (int j = 0; j < 3; ++j) {
		IOProfileData(sectionName, strprintf(LTEXT("%s.szHF[%d]") , keyPrefix, j), printSetting.m_szHeaderForm[j]);
		IOProfileData(sectionName, strprintf(LTEXT("%s.szFTF[%d]"), keyPrefix, j), printSetting.m_szFooterForm[j]);
	}

	// ヘッダー/フッター フォント設定
	ShareData_IO_LogFont(
		*this,
		sectionName,
		strprintf(LTEXT("%s.lfHeader"),			keyPrefix),
		printSetting.m_lfHeader,
		printSetting.m_nHeaderPointSize,
		strprintf(LTEXT("%s.nHeaderPointSize"),	keyPrefix),
		strprintf(LTEXT("%s.lfHeaderFaceName"),	keyPrefix)
	);

	ShareData_IO_LogFont(
		*this,
		sectionName,
		strprintf(LTEXT("%s.lfFooter"),			keyPrefix),
		printSetting.m_lfFooter,
		printSetting.m_nFooterPointSize,
		strprintf(LTEXT("%s.nFooterPointSize"),	keyPrefix),
		strprintf(LTEXT("%s.lfFooterFaceName"),	keyPrefix)
	);

	IOProfileData(sectionName, strprintf(LTEXT("%s.szDriver"), keyPrefix, keyPrefix), printSetting.m_mdmDevMode.m_szPrinterDriverName);
	IOProfileData(sectionName, strprintf(LTEXT("%s.szDevice"), keyPrefix, keyPrefix), printSetting.m_mdmDevMode.m_szPrinterDeviceName);
	IOProfileData(sectionName, strprintf(LTEXT("%s.szOutput"), keyPrefix, keyPrefix), printSetting.m_mdmDevMode.m_szPrinterOutputName);

	//禁則
	IOProfileData(sectionName, strprintf(LTEXT("%s.bKinsokuHead"), keyPrefix, keyPrefix), printSetting.m_bPrintKinsokuHead);
	IOProfileData(sectionName, strprintf(LTEXT("%s.bKinsokuTail"), keyPrefix, keyPrefix), printSetting.m_bPrintKinsokuTail);
	IOProfileData(sectionName, strprintf(LTEXT("%s.bKinsokuRet"),  keyPrefix, keyPrefix), printSetting.m_bPrintKinsokuRet);
	IOProfileData(sectionName, strprintf(LTEXT("%s.bKinsokuKuto"), keyPrefix, keyPrefix), printSetting.m_bPrintKinsokuKuto);

	//カラー印刷
	IOProfileData(sectionName, strprintf(LTEXT("%s.bColorPrint"), keyPrefix), printSetting.m_bColorPrint);

	return true;	// 返却値に意味はない
}

/*!
 * @brief 設定値の入出力を行う。
 *
 * PluginRec型（プラグイン設定データ）向けの特殊化。
 */
template<>
bool CDataProfile::IOProfileData<PluginRec>(
	sakura::wstring_view		sectionName,	//!< [in] セクション名
	sakura::wstring_view		keyPrefix,		//!< [in] エントリ名
	PluginRec&				pluginRec		//!< [in,out] エントリ値
)
{
	decltype(pluginRec.m_szName) szName{};
	decltype(pluginRec.m_szId) szId{};

	// 書き込みモード
	if (IsWritingMode()) {
		::wcscpy_s(szName, pluginRec.m_szName);
		::wcscpy_s(szId, pluginRec.m_szId);
	}

	if (!IOProfileData(sectionName, std::format(LTEXT("{}.Name"), keyPrefix), szName))
	{
		return false;
	}

	if (!IOProfileData(sectionName, std::format(LTEXT("{}.Id"), keyPrefix), szId))
	{
		return false;
	}

	if (!IOProfileData(sectionName, std::format(LTEXT("{}.CmdNum"), keyPrefix), pluginRec.m_nCmdNum))
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
	sakura::wstring_view		sectionName,	//!< [in] セクション名
	sakura::wstring_view		entryKey,		//!< [in] エントリ名
	RECT&					rcEntryValue	//!< [in,out] エントリ値
)
{
	// 取得・設定は文字列を介して行う
	sakura::wstring buffer{};

	// 書き込みモード
	if (IsWritingMode()) {
		strprintf(
			buffer,
			LTEXT("%d,%d,%d,%d"),
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
			LTEXT("%d,%d,%d,%d"),
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
	sakura::wstring_view		sectionName,	//!< [in] セクション名
	sakura::wstring_view		keyPrefix,		//!< [in] エントリ名
	SFileTreeItem&			fileTreeItem		//!< [in,out] エントリ値
)
{
	IOProfileData(sectionName, std::format(LTEXT("{}.eItemType"), keyPrefix), fileTreeItem.m_eFileTreeItemType);

	if (IsReadingMode() ||
		fileTreeItem.m_eFileTreeItemType == EFileTreeItemType_Grep ||
		fileTreeItem.m_eFileTreeItemType == EFileTreeItemType_File)
	{
		IOProfileData(sectionName, std::format(LTEXT("{}.szTargetPath"), keyPrefix), fileTreeItem.m_szTargetPath);
	}

	if (IsReadingMode() ||
		fileTreeItem.m_eFileTreeItemType == EFileTreeItemType_Folder ||
		!fileTreeItem.m_szLabelName.empty())
	{
		IOProfileData(sectionName, std::format(LTEXT("{}.szLabelName"), keyPrefix), fileTreeItem.m_szLabelName);
	}

	IOProfileData(sectionName, std::format(LTEXT("{}.nDepth"), keyPrefix), fileTreeItem.m_nDepth);

	if (IsReadingMode() ||
		fileTreeItem.m_eFileTreeItemType == EFileTreeItemType_Grep)
	{
		IOProfileData(sectionName, std::format(LTEXT("{}.szTargetFile"),   keyPrefix), fileTreeItem.m_szTargetFile);
		IOProfileData(sectionName, std::format(LTEXT("{}.bIgnoreHidden"),  keyPrefix), fileTreeItem.m_bIgnoreHidden);
		IOProfileData(sectionName, std::format(LTEXT("{}.bIgnoreReadOny"), keyPrefix), fileTreeItem.m_bIgnoreReadOnly);
		IOProfileData(sectionName, std::format(LTEXT("{}.bIgnoreSystem"),  keyPrefix), fileTreeItem.m_bIgnoreSystem);
	}

	return true;	// 返却値に意味はない
}
