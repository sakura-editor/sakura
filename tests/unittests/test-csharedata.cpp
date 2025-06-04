/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

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
#include "pch.h"
#include "env/CShareData.h"

#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"
#include "_main/CNormalProcess.h"

#include "config/system_constants.h"
#include "String_define.h"

#include "util/window.h"
#include "CDataProfile.h" // StringBufferW
#include "uiparts/CMenuDrawer.h"

UINT GetPrivateProfileIntW(
	_In_ LPCWSTR lpAppName,
	_In_ LPCWSTR lpKeyName,
	_In_ INT nDefault,
	std::optional<std::filesystem::path> iniPath = std::nullopt
);

std::wstring GetPrivateProfileStringW(
	_In_opt_ LPCWSTR lpAppName,
	_In_opt_ LPCWSTR lpKeyName,
	_In_opt_ LPCWSTR lpDefault,
	_In_     DWORD nSize,
	std::optional<std::filesystem::path> iniPath = std::nullopt
);

struct SMenuItem {
	int m_nLevel;
	EFunctionCode m_eFuncCode;
	wchar_t m_chAccessKey = '\0';

	SMenuItem(
		int nLevel,
		int nFuncCode,
		KEYCODE accessKey = '\0'
	)
		: m_nLevel( nLevel )
		, m_eFuncCode(EFunctionCode(nFuncCode))
		, m_chAccessKey(accessKey)
	{
	}

	SMenuItem(
		int nFuncCode,
		KEYCODE accessKey = '\0'
	)
		: SMenuItem(0, nFuncCode, accessKey)
	{
	}

	auto GetType() const noexcept
	{
		// セパレーター
		if (1 == m_eFuncCode) return T_SEPARATOR;

		// 特殊メニュー
		if (29001 <= m_eFuncCode && m_eFuncCode <= 29006) return T_SPECIAL;

		// 最上位ポップアップメニュー
		if (34052 <= m_eFuncCode && m_eFuncCode <= 34059) return T_NODE;

		// ポップアップメニュー
		const std::set<int> popupFuncCodes{
			32805,
			34000,
			34005,
			34006,
			34007,
			34008,
			34009,
			34010,
			34011,
			34012,
			34013,
			34022,
			34023,
			34044,
			34047,
			34048,
			34051,
			34060,
		};
		if (const auto found = popupFuncCodes.find(m_eFuncCode); found != popupFuncCodes.cend()) return T_NODE;

		return T_LEAF;
	}
};

using SFuncCodeArray = std::array<EFunctionCode, 8>;

struct KEYDATAINIT {
	short				m_nKeyCode;			//!< Key Code (0 for non-keybord button)
	union {
		const WCHAR*	m_pszKeyName;		//!< Key Name (for display)
		UINT			m_nKeyNameId;		//!< String Resource Id ( 0x0000 - 0xFFFF )
	};
	SFuncCodeArray		m_nFuncCodeArr;		//!< Key Function Number
};

bool operator==( const SEncodingConfig& lhs, const SEncodingConfig& rhs ) {
	return lhs.m_bPriorCesu8 == rhs.m_bPriorCesu8 &&
		lhs.m_eDefaultCodetype == rhs.m_eDefaultCodetype &&
		lhs.m_eDefaultEoltype == rhs.m_eDefaultEoltype &&
		lhs.m_bDefaultBom == rhs.m_bDefaultBom;
}

bool operator==(const LOGFONT& lhs, const LOGFONT& rhs) {
	return lhs.lfHeight == rhs.lfHeight &&
		   lhs.lfWidth == rhs.lfWidth &&
		   lhs.lfEscapement == rhs.lfEscapement &&
		   lhs.lfOrientation == rhs.lfOrientation &&
		   lhs.lfWeight == rhs.lfWeight &&
		   lhs.lfItalic == rhs.lfItalic &&
		   lhs.lfUnderline == rhs.lfUnderline &&
		   lhs.lfStrikeOut == rhs.lfStrikeOut &&
		   lhs.lfCharSet == rhs.lfCharSet &&
		   lhs.lfOutPrecision == rhs.lfOutPrecision &&
		   lhs.lfClipPrecision == rhs.lfClipPrecision &&
		   lhs.lfQuality == rhs.lfQuality &&
		   lhs.lfPitchAndFamily == rhs.lfPitchAndFamily &&
		   0 == wcscmp(lhs.lfFaceName, rhs.lfFaceName);
}

bool operator==( const RECT& lhs, const RECT& rhs ) {
	return lhs.left == rhs.left &&
		lhs.top == rhs.top &&
		lhs.right == rhs.right &&
		lhs.bottom == rhs.bottom;
}

MATCHER_P(EqSTypeConfig, expected, "Checks if STypeConfig is equal to the expected value") {
    const STypeConfig& actual = arg;

    // プロパティごとに等価評価を行う
	EXPECT_THAT(actual.m_nIdx, expected.m_nIdx);
	EXPECT_THAT(actual.m_id, expected.m_id);
	EXPECT_THAT(actual.m_szTypeName, StrEq(expected.m_szTypeName));
	EXPECT_THAT(actual.m_szTypeExts, StrEq(expected.m_szTypeExts));
	EXPECT_THAT(actual.m_nTextWrapMethod, expected.m_nTextWrapMethod);
	EXPECT_THAT(actual.m_nMaxLineKetas, expected.m_nMaxLineKetas);
	EXPECT_THAT(actual.m_nColumnSpace, expected.m_nColumnSpace);
	EXPECT_THAT(actual.m_nLineSpace, expected.m_nLineSpace);
	EXPECT_THAT(actual.m_nTabSpace, expected.m_nTabSpace);
	EXPECT_THAT(actual.m_bTabArrow, expected.m_bTabArrow);
	EXPECT_THAT(actual.m_szTabViewString, StrEq(expected.m_szTabViewString));
	EXPECT_THAT(actual.m_bInsSpace, expected.m_bInsSpace);
	EXPECT_THAT(actual.m_nTsvMode, expected.m_nTsvMode);
	for (size_t i = 0; i < std::size(actual.m_nKeyWordSetIdx); ++i) {
		EXPECT_THAT(actual.m_nKeyWordSetIdx[i], expected.m_nKeyWordSetIdx[i]) << L"Unexpected value at index " << i;
	}
	for (size_t i = 0; i < COMMENT_DELIMITER_NUM; ++i) {
		EXPECT_THAT(actual.m_cLineComment.getLineComment(i),    StrEq(expected.m_cLineComment.getLineComment(i)));
		EXPECT_THAT(actual.m_cLineComment.getLineCommentPos(i), expected.m_cLineComment.getLineCommentPos(i));
	}
	for (size_t i = 0; i < std::size(actual.m_cBlockComments ); ++i) {
		EXPECT_THAT(actual.m_cBlockComments[i].getBlockCommentFrom(), StrEq(expected.m_cBlockComments[i].getBlockCommentFrom())) << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_cBlockComments[i].getBlockFromLen(),     expected.m_cBlockComments[i].getBlockFromLen())            << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_cBlockComments[i].getBlockCommentTo(),   StrEq(expected.m_cBlockComments[i].getBlockCommentTo()))   << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_cBlockComments[i].getBlockToLen(),       expected.m_cBlockComments[i].getBlockToLen())              << L"Unexpected value at index " << i;
	}
	EXPECT_THAT(actual.m_nStringType, expected.m_nStringType);
	EXPECT_THAT(actual.m_bStringLineOnly, expected.m_bStringLineOnly);
	EXPECT_THAT(actual.m_bStringEndLine, expected.m_bStringEndLine);
	EXPECT_THAT(actual.m_nHeredocType, expected.m_nHeredocType);
	EXPECT_THAT(actual.m_szIndentChars, StrEq(expected.m_szIndentChars));
	EXPECT_THAT(actual.m_nColorInfoArrNum, expected.m_nColorInfoArrNum);
	for (size_t i = 0; i < std::size(actual.m_ColorInfoArr); ++i) {
		EXPECT_THAT(actual.m_ColorInfoArr[i].m_bDisp,                  expected.m_ColorInfoArr[i].m_bDisp)                  << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_ColorInfoArr[i].m_sFontAttr.m_bBoldFont,  expected.m_ColorInfoArr[i].m_sFontAttr.m_bBoldFont)  << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_ColorInfoArr[i].m_sFontAttr.m_bUnderLine, expected.m_ColorInfoArr[i].m_sFontAttr.m_bUnderLine) << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_ColorInfoArr[i].m_sColorAttr.m_cTEXT,     expected.m_ColorInfoArr[i].m_sColorAttr.m_cTEXT)     << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_ColorInfoArr[i].m_sColorAttr.m_cBACK,     expected.m_ColorInfoArr[i].m_sColorAttr.m_cBACK)     << L"Unexpected value at index " << i;
	}
	EXPECT_THAT(actual.m_szBackImgPath, StrEq(expected.m_szBackImgPath.c_str()));
 	EXPECT_THAT(actual.m_backImgPos, expected.m_backImgPos);
	EXPECT_THAT(actual.m_backImgRepeatX, expected.m_backImgRepeatX);
	EXPECT_THAT(actual.m_backImgRepeatY, expected.m_backImgRepeatY);
	EXPECT_THAT(actual.m_backImgScrollX, expected.m_backImgScrollX);
	EXPECT_THAT(actual.m_backImgScrollY, expected.m_backImgScrollY);
	EXPECT_THAT(actual.m_backImgPosOffset.x, expected.m_backImgPosOffset.x);
	EXPECT_THAT(actual.m_backImgPosOffset.y, expected.m_backImgPosOffset.y);
	EXPECT_THAT(actual.m_backImgOpacity, expected.m_backImgOpacity);
	EXPECT_THAT(actual.m_bLineNumIsCRLF, expected.m_bLineNumIsCRLF);
	EXPECT_THAT(actual.m_nLineTermType, expected.m_nLineTermType);
	EXPECT_THAT(actual.m_cLineTermChar, expected.m_cLineTermChar);
	for (size_t i = 0; i < std::size(actual.m_nVertLineIdx); ++i) {
		EXPECT_THAT(actual.m_nVertLineIdx[i], expected.m_nVertLineIdx[i]) << L"Unexpected value at index " << i;
	}
	EXPECT_THAT(actual.m_nNoteLineOffset, expected.m_nNoteLineOffset);
	EXPECT_THAT(actual.m_bWordWrap, expected.m_bWordWrap);
	EXPECT_THAT(actual.m_bKinsokuHead, expected.m_bKinsokuHead);
	EXPECT_THAT(actual.m_bKinsokuTail, expected.m_bKinsokuTail);
	EXPECT_THAT(actual.m_bKinsokuRet, expected.m_bKinsokuRet);
	EXPECT_THAT(actual.m_bKinsokuKuto, expected.m_bKinsokuKuto);
	EXPECT_THAT(actual.m_bKinsokuHide, expected.m_bKinsokuHide);
	EXPECT_THAT(actual.m_szKinsokuHead, StrEq(expected.m_szKinsokuHead));
	EXPECT_THAT(actual.m_szKinsokuTail, StrEq(expected.m_szKinsokuTail));
	EXPECT_THAT(actual.m_szKinsokuKuto, StrEq(expected.m_szKinsokuKuto));
	EXPECT_THAT(actual.m_nCurrentPrintSetting, expected.m_nCurrentPrintSetting);
	EXPECT_THAT(actual.m_bOutlineDockDisp, expected.m_bOutlineDockDisp);
	EXPECT_THAT(actual.m_eOutlineDockSide, expected.m_eOutlineDockSide);
	EXPECT_THAT(actual.m_cxOutlineDockLeft, expected.m_cxOutlineDockLeft);
	EXPECT_THAT(actual.m_cyOutlineDockTop, expected.m_cyOutlineDockTop);
	EXPECT_THAT(actual.m_cxOutlineDockRight, expected.m_cxOutlineDockRight);
	EXPECT_THAT(actual.m_cyOutlineDockBottom, expected.m_cyOutlineDockBottom);
	EXPECT_THAT(actual.m_nDockOutline, expected.m_nDockOutline);
	EXPECT_THAT(actual.m_eDefaultOutline, expected.m_eDefaultOutline);
	EXPECT_THAT(actual.m_szOutlineRuleFilename, StrEq(expected.m_szOutlineRuleFilename.c_str()));
	EXPECT_THAT(actual.m_nOutlineSortCol, expected.m_nOutlineSortCol);
	EXPECT_THAT(actual.m_bOutlineSortDesc, expected.m_bOutlineSortDesc);
	EXPECT_THAT(actual.m_nOutlineSortType, expected.m_nOutlineSortType);
	EXPECT_THAT(actual.m_sFileTree.m_bProject,     expected.m_sFileTree.m_bProject);
	EXPECT_THAT(actual.m_sFileTree.m_szProjectIni, StrEq(expected.m_sFileTree.m_szProjectIni.c_str()));
	EXPECT_THAT(actual.m_sFileTree.m_nItemCount,   expected.m_sFileTree.m_nItemCount);
	for (size_t i = 0; i < std::size(actual.m_sFileTree.m_aItems); ++i) {
		EXPECT_THAT(actual.m_sFileTree.m_aItems[i].m_eFileTreeItemType, expected.m_sFileTree.m_aItems[i].m_eFileTreeItemType)           << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_sFileTree.m_aItems[i].m_szTargetPath,      StrEq(expected.m_sFileTree.m_aItems[i].m_szTargetPath.c_str())) << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_sFileTree.m_aItems[i].m_szLabelName,       StrEq(expected.m_sFileTree.m_aItems[i].m_szLabelName.c_str()))  << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_sFileTree.m_aItems[i].m_nDepth,            expected.m_sFileTree.m_aItems[i].m_nDepth)                      << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_sFileTree.m_aItems[i].m_szTargetFile,      StrEq(expected.m_sFileTree.m_aItems[i].m_szTargetFile.c_str())) << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_sFileTree.m_aItems[i].m_bIgnoreHidden,     expected.m_sFileTree.m_aItems[i].m_bIgnoreHidden)               << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_sFileTree.m_aItems[i].m_bIgnoreReadOnly,   expected.m_sFileTree.m_aItems[i].m_bIgnoreReadOnly)             << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_sFileTree.m_aItems[i].m_bIgnoreSystem,     expected.m_sFileTree.m_aItems[i].m_bIgnoreSystem)               << L"Unexpected value at index " << i;
	}
	EXPECT_THAT(actual.m_eSmartIndent, expected.m_eSmartIndent);
	EXPECT_THAT(actual.m_bIndentCppStringIgnore, expected.m_bIndentCppStringIgnore);
	EXPECT_THAT(actual.m_bIndentCppCommentIgnore, expected.m_bIndentCppCommentIgnore);
	EXPECT_THAT(actual.m_bIndentCppUndoSep, expected.m_bIndentCppUndoSep);
	EXPECT_THAT(actual.m_bDetectIndentationStyleOnFileLoad, expected.m_bDetectIndentationStyleOnFileLoad);
	EXPECT_THAT(actual.m_nImeState, expected.m_nImeState);
	EXPECT_THAT(actual.m_szHokanFile, StrEq(expected.m_szHokanFile.c_str()));
	EXPECT_THAT(actual.m_nHokanType, expected.m_nHokanType);
	EXPECT_THAT(actual.m_bUseHokanByFile, expected.m_bUseHokanByFile);
	EXPECT_THAT(actual.m_bUseHokanByKeyword, expected.m_bUseHokanByKeyword);
	EXPECT_THAT(actual.m_bHokanLoHiCase, expected.m_bHokanLoHiCase);
	EXPECT_THAT(actual.m_szExtHelp, StrEq(expected.m_szExtHelp.c_str()));
	EXPECT_THAT(actual.m_szExtHtmlHelp, StrEq(expected.m_szExtHtmlHelp.c_str()));
	EXPECT_THAT(actual.m_bHtmlHelpIsSingle, expected.m_bHtmlHelpIsSingle);
	EXPECT_THAT(actual.m_bChkEnterAtEnd, expected.m_bChkEnterAtEnd);
	EXPECT_THAT(actual.m_encoding, expected.m_encoding);
	EXPECT_THAT(actual.m_bUseRegexKeyword, expected.m_bUseRegexKeyword);
	EXPECT_THAT(actual.m_nRegexKeyMagicNumber, expected.m_nRegexKeyMagicNumber);
	for (size_t i = 0; i < std::size(actual.m_RegexKeywordArr); ++i) {
		EXPECT_THAT(actual.m_RegexKeywordArr[i].m_nColorIndex, expected.m_RegexKeywordArr[i].m_nColorIndex) << L"Unexpected value at index " << i;
	}
	EXPECT_THAT(actual.m_RegexKeywordList, StrEq(expected.m_RegexKeywordList));
	EXPECT_THAT(actual.m_bUseKeyWordHelp, expected.m_bUseKeyWordHelp);
	EXPECT_THAT(actual.m_nKeyHelpNum, expected.m_nKeyHelpNum);
	for (size_t i = 0; i < std::size(actual.m_KeyHelpArr); ++i) {
		EXPECT_THAT(actual.m_KeyHelpArr[i].m_bUse,    expected.m_KeyHelpArr[i].m_bUse)                  << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_KeyHelpArr[i].m_szAbout, StrEq(expected.m_KeyHelpArr[i].m_szAbout))        << L"Unexpected value at index " << i;
		EXPECT_THAT(actual.m_KeyHelpArr[i].m_szPath,  StrEq(expected.m_KeyHelpArr[i].m_szPath.c_str())) << L"Unexpected value at index " << i;
	}
	EXPECT_THAT(actual.m_bUseKeyHelpAllSearch, expected.m_bUseKeyHelpAllSearch);
	EXPECT_THAT(actual.m_bUseKeyHelpKeyDisp, expected.m_bUseKeyHelpKeyDisp);
	EXPECT_THAT(actual.m_bUseKeyHelpPrefix, expected.m_bUseKeyHelpPrefix);
	EXPECT_THAT(actual.m_eKeyHelpRMenuShowType, expected.m_eKeyHelpRMenuShowType);
	EXPECT_THAT(actual.m_bAutoIndent, expected.m_bAutoIndent);
	EXPECT_THAT(actual.m_bAutoIndent_ZENSPACE, expected.m_bAutoIndent_ZENSPACE);
	EXPECT_THAT(actual.m_bRTrimPrevLine, expected.m_bRTrimPrevLine);
	EXPECT_THAT(actual.m_nIndentLayout, expected.m_nIndentLayout);
	EXPECT_THAT(actual.m_bUseDocumentIcon, expected.m_bUseDocumentIcon);
	EXPECT_THAT(actual.m_bUseTypeFont, expected.m_bUseTypeFont);
	EXPECT_THAT(actual.m_lf, expected.m_lf);
	EXPECT_THAT(actual.m_nPointSize, expected.m_nPointSize);
	EXPECT_THAT(actual.m_nLineNumWidth, expected.m_nLineNumWidth);

    // 常に true を返す
    return true;
}

namespace sakura_ini {

/*!
 * @brief APIラッパー関数のテスト
 */
TEST(apiwrap, test)
{
	const auto iniPath = GetIniFileName();;

	// 設定を書き込む
	WritePrivateProfileStringW(L"Settings", L"UserRootFolder", L"2", iniPath.c_str());
	WritePrivateProfileStringW(L"Settings", L"UserSubFolder", L"", iniPath.c_str());

	EXPECT_THAT(GetPrivateProfileIntW(L"Settings", L"MultiUser", 1, iniPath), 1);
	EXPECT_THAT(GetPrivateProfileIntW(L"Settings", L"UserRootFolder", 0, iniPath), 2);
	EXPECT_THAT(GetPrivateProfileStringW(L"Settings", L"UserSubFolder", L"sakura", _MAX_DIR, iniPath), StrEq(L"sakura"));
	EXPECT_THAT(GetPrivateProfileStringW(L"Settings", L"UserRootFolder", L"sakura", _MAX_DIR, iniPath), StrEq(L"2"));

	if (std::filesystem::exists(iniPath)) {
		std::filesystem::remove(iniPath);
	}
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(CShareData, BuildPrivateIniFileName_RoamingAppData)
{
	const auto& szProfileName = L"profile1";
	const auto isMultiUserSettings = true;
	const auto userRootFolder = 0;
	const auto& userSubFolder = L"sakura";

	// exe基準のiniファイルパスを得る
	const auto iniPath = GetExeFileName().replace_extension(L".ini");

	// 設定ファイルフォルダー
	auto iniFolder = iniPath;
	iniFolder.remove_filename();

	// iniファイル名を得る
	const auto filename = iniPath.filename();

	// 期待値を取得する
	std::wstring expected(2048, L'\0');
	EXPECT_TRUE(ExpandEnvironmentStringsW(LR"(%USERPROFILE%\AppData\Roaming\sakura\profile1\)", expected.data(), DWORD(expected.capacity())));
	expected.assign(expected.data());
	expected += iniPath.filename();

	// テスト実施
	EXPECT_THAT(CShareData::BuildPrivateIniFileName(iniFolder, isMultiUserSettings, userRootFolder, userSubFolder, szProfileName, filename), StrEq(expected));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(CShareData, BuildPrivateIniFileName_Desktop)
{
	const auto& szProfileName = L"";
	const auto isMultiUserSettings = true;
	const auto userRootFolder = 3;
	const auto& userSubFolder = L"sakura";

	// exe基準のiniファイルパスを得る
	const auto iniPath = GetExeFileName().replace_extension(L".ini");

	// 設定ファイルフォルダー
	auto iniFolder = iniPath;
	iniFolder.remove_filename();

	// iniファイル名を得る
	const auto filename = iniPath.filename();

	// 期待値を取得する
	std::wstring expected(2048, L'\0');
	EXPECT_TRUE(ExpandEnvironmentStrings(LR"(%USERPROFILE%\Desktop\sakura\)", expected.data(), (DWORD)expected.capacity()));
	expected.assign(expected.data());
	expected += iniPath.filename();

	// テスト実施
	EXPECT_THAT(CShareData::BuildPrivateIniFileName(iniFolder, isMultiUserSettings, userRootFolder, userSubFolder, szProfileName, filename), StrEq(expected));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(CShareData, BuildPrivateIniFileName_Profile)
{
	const auto& szProfileName = L"";
	const auto isMultiUserSettings = true;
	const auto userRootFolder = 1;
	const auto& userSubFolder = L"sakura";

	// exe基準のiniファイルパスを得る
	const auto iniPath = GetExeFileName().replace_extension(L".ini");

	// 設定ファイルフォルダー
	auto iniFolder = iniPath;
	iniFolder.remove_filename();

	// iniファイル名を得る
	const auto filename = iniPath.filename();

	// 期待値を取得する
	std::wstring expected(2048, L'\0');
	EXPECT_TRUE(ExpandEnvironmentStrings(LR"(%USERPROFILE%\sakura\)", expected.data(), (DWORD)expected.capacity()));
	expected.assign(expected.data());
	expected += iniPath.filename();

	// テスト実施
	EXPECT_THAT(CShareData::BuildPrivateIniFileName(iniFolder, isMultiUserSettings, userRootFolder, userSubFolder, szProfileName, filename), StrEq(expected));
}

/*!
 * @brief iniファイルパスの取得
 */
TEST(CShareData, BuildPrivateIniFileName_Document)
{
	const auto& szProfileName = L"";
	const auto isMultiUserSettings = true;
	const auto userRootFolder = 2;
	const auto& userSubFolder = L"sakura";

	// exe基準のiniファイルパスを得る
	const auto iniPath = GetExeFileName().replace_extension(L".ini");

	// 設定ファイルフォルダー
	auto iniFolder = iniPath;
	iniFolder.remove_filename();

	// iniファイル名を得る
	const auto filename = iniPath.filename();

	// 期待値を取得する
	std::wstring expected(2048, L'\0');
	EXPECT_TRUE(ExpandEnvironmentStrings(LR"(%USERPROFILE%\Documents\sakura\)", expected.data(), (DWORD)expected.capacity()));
	expected.assign(expected.data());
	expected += iniPath.filename();

	// テスト実施
	EXPECT_THAT(CShareData::BuildPrivateIniFileName(iniFolder, isMultiUserSettings, userRootFolder, userSubFolder, szProfileName, filename), StrEq(expected));
}

} // namespace sakura_ini

namespace share_data {

// [全般]タブ
MATCHER(IsInitializedCommonSettingGeneral, "Checks if CommonSetting_General is properly initialized") {
    const CommonSetting_General& sGeneral = arg;

	EXPECT_THAT(sGeneral.m_nMRUArrNum_MAX, 15);
	EXPECT_THAT(sGeneral.m_nOPENFOLDERArrNum_MAX, 15);
	EXPECT_THAT(sGeneral.m_nCaretType, 0);
	EXPECT_THAT(sGeneral.m_bIsINSMode, IsTrue());
	EXPECT_THAT(sGeneral.m_bIsFreeCursorMode, IsFalse());
	EXPECT_THAT(sGeneral.m_bStopsBothEndsWhenSearchWord, IsFalse());
	EXPECT_THAT(sGeneral.m_bStopsBothEndsWhenSearchParagraph, IsFalse());
	EXPECT_THAT(sGeneral.m_bCloseAllConfirm, IsFalse());
	EXPECT_THAT(sGeneral.m_bExitConfirm, IsFalse());
	EXPECT_THAT(sGeneral.m_nRepeatedScrollLineNum, CLayoutInt(3));
	EXPECT_THAT(sGeneral.m_nRepeatedMoveCaretNum, 2);
	EXPECT_THAT(sGeneral.m_nRepeatedScroll_Smooth, IsFalse());
	EXPECT_THAT(sGeneral.m_nPageScrollByWheel, 0);
	EXPECT_THAT(sGeneral.m_nHorizontalScrollByWheel, 0);
	EXPECT_THAT(sGeneral.m_bUseTaskTray, IsTrue());
	EXPECT_THAT(sGeneral.m_bStayTaskTray, IsFalse());
	EXPECT_THAT(sGeneral.m_wTrayMenuHotKeyCode, L'Z');
	EXPECT_THAT(sGeneral.m_wTrayMenuHotKeyMods, HOTKEYF_ALT | HOTKEYF_CONTROL);
	EXPECT_THAT(sGeneral.m_bDispExitingDialog, IsFalse());
	EXPECT_THAT(sGeneral.m_bNoCaretMoveByActivation, IsFalse());

	return true;
}

// [ウィンドウ]タブ
MATCHER(IsInitializedCommonSettingWindow, "Checks if CommonSetting_Window is properly initialized") {
    const CommonSetting_Window& sWindow = arg;

	EXPECT_THAT(sWindow.m_bDispTOOLBAR, IsTrue());
	EXPECT_THAT(sWindow.m_bDispSTATUSBAR, IsTrue());
	EXPECT_THAT(sWindow.m_bDispFUNCKEYWND, IsFalse());
	EXPECT_THAT(sWindow.m_bDispMiniMap, IsFalse());
	EXPECT_THAT(sWindow.m_nFUNCKEYWND_Place, 1);
	EXPECT_THAT(sWindow.m_nFUNCKEYWND_GroupNum, 4);
	EXPECT_THAT(sWindow.m_nMiniMapFontSize, -2);
	EXPECT_THAT(sWindow.m_nMiniMapQuality, NONANTIALIASED_QUALITY);
	EXPECT_THAT(sWindow.m_nMiniMapWidth, 150);
	EXPECT_THAT(sWindow.m_bSplitterWndHScroll, IsTrue());
	EXPECT_THAT(sWindow.m_bSplitterWndVScroll, IsTrue());
	EXPECT_THAT(sWindow.m_eSaveWindowSize, WINSIZEMODE_SAVE);
	EXPECT_THAT(sWindow.m_nWinSizeType, SIZE_RESTORED);
	EXPECT_THAT(sWindow.m_nWinSizeCX, CW_USEDEFAULT);
	EXPECT_THAT(sWindow.m_nWinSizeCY, 0);
	EXPECT_THAT(sWindow.m_bScrollBarHorz, IsTrue());
	EXPECT_THAT(sWindow.m_eSaveWindowPos, WINSIZEMODE_DEF);
	EXPECT_THAT(sWindow.m_nWinPosX, CW_USEDEFAULT);
	EXPECT_THAT(sWindow.m_nWinPosY, 0);
	EXPECT_THAT(sWindow.m_nRulerHeight, 13);
	EXPECT_THAT(sWindow.m_nRulerBottomSpace, 0);
	EXPECT_THAT(sWindow.m_nLineNumRightSpace, 0);
	EXPECT_THAT(sWindow.m_nVertLineOffset, -1);
	EXPECT_THAT(sWindow.m_bUseCompatibleBMP, IsTrue());
	EXPECT_THAT(sWindow.m_bMenuIcon, IsTrue());

	EXPECT_THAT(sWindow.m_szWindowCaptionActive, StrEq(
		L"${w?$h$:アウトプット$:${I?$f$n$:$N$n$}$}${U?(更新)$} -"
		L" $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>"
	));

	EXPECT_THAT(sWindow.m_szWindowCaptionInactive, StrEq(
		L"${w?$h$:アウトプット$:$f$n$}${U?(更新)$} -"
		L" $A $V ${R?(ビューモード)$:(上書き禁止)$}${M?  【キーマクロの記録中】$} $<profile>"
	));

	return true;
}

// [タブバー]タブ
MATCHER_P2(IsInitializedCommonSettingTabBar, lfIconTitle, nIconPointSize, "Checks if CommonSetting_TabBar is properly initialized") {
    const CommonSetting_TabBar& sTabBar = arg;

	EXPECT_THAT(sTabBar.m_bDispTabWnd, IsFalse());
	EXPECT_THAT(sTabBar.m_bDispTabWndMultiWin, IsFalse());

	EXPECT_THAT(sTabBar.m_szTabWndCaption, StrEq(
		L"${w?【Grep】$h$:【アウトプット】$:$f$n$}${U?(更新)$}${R?(ビューモード)$:(上書き禁止)$}${M?【キーマクロの記録中】$}"
	));

	EXPECT_THAT(sTabBar.m_bSameTabWidth, IsFalse());
	EXPECT_THAT(sTabBar.m_bDispTabIcon, IsFalse());
	EXPECT_THAT(sTabBar.m_bDispTabClose, DISPTABCLOSE_NO);
	EXPECT_THAT(sTabBar.m_bSortTabList, IsTrue());
	EXPECT_THAT(sTabBar.m_bTab_RetainEmptyWin, IsTrue());
	EXPECT_THAT(sTabBar.m_bTab_CloseOneWin, IsFalse());
	EXPECT_THAT(sTabBar.m_bTab_ListFull, IsFalse());
	EXPECT_THAT(sTabBar.m_bChgWndByWheel, IsFalse());
	EXPECT_THAT(sTabBar.m_bNewWindow, IsFalse());
	EXPECT_THAT(sTabBar.m_bTabMultiLine, IsFalse());
	EXPECT_THAT(sTabBar.m_eTabPosition, TabPosition_Top);

	EXPECT_THAT(sTabBar.m_lf, lfIconTitle);
	EXPECT_THAT(sTabBar.m_nPointSize, nIconPointSize);
	EXPECT_THAT(sTabBar.m_nTabMaxWidth, 200);
	EXPECT_THAT(sTabBar.m_nTabMinWidth, 60);
	EXPECT_THAT(sTabBar.m_nTabMinWidthOnMulti, 100);

	return true;
}

// [編集]タブ
MATCHER(IsInitializedCommonSettingEdit, "Checks if CommonSetting_Edit is properly initialized") {
    const CommonSetting_Edit& sEdit = arg;

	EXPECT_THAT(sEdit.m_bAddCRLFWhenCopy, IsFalse());

	EXPECT_THAT(sEdit.m_bUseOLE_DragDrop, IsTrue());
	EXPECT_THAT(sEdit.m_bUseOLE_DropSource, IsTrue());
	EXPECT_THAT(sEdit.m_bSelectClickedURL, IsTrue());
	EXPECT_THAT(sEdit.m_bCopyAndDisablSelection, IsFalse());
	EXPECT_THAT(sEdit.m_bEnableNoSelectCopy, IsTrue());
	EXPECT_THAT(sEdit.m_bEnableLineModePaste, IsTrue());
	EXPECT_THAT(sEdit.m_bConvertEOLPaste, IsFalse());
	EXPECT_THAT(sEdit.m_bEnableExtEol, IsFalse());
	EXPECT_THAT(sEdit.m_bBoxSelectLock, IsTrue());
	EXPECT_THAT(sEdit.m_bVistaStyleFileDialog, IsTrue());
	EXPECT_THAT(sEdit.m_bNotOverWriteCRLF, IsTrue());
	EXPECT_THAT(sEdit.m_bOverWriteFixMode, IsFalse());

	EXPECT_THAT(sEdit.m_bOverWriteBoxDelete, IsFalse());
	EXPECT_THAT(sEdit.m_eOpenDialogDir, OPENDIALOGDIR_CUR);
	EXPECT_THAT(sEdit.m_OpenDialogSelDir, StrEq(LR"(%Personal%\)"));
	EXPECT_THAT(sEdit.m_bAutoColumnPaste, IsTrue());

	return true;
}

// [ファイル]タブ
MATCHER(IsInitializedCommonSettingFile, "Checks if CommonSetting_File is properly initialized") {
    const CommonSetting_File&sFile = arg;

	EXPECT_THAT(sFile.m_nFileShareMode, SHAREMODE_NOT_EXCLUSIVE);
	EXPECT_THAT(sFile.m_bCheckFileTimeStamp, IsTrue());
	EXPECT_THAT(sFile.m_nAutoloadDelay, 0);
	EXPECT_THAT(sFile.m_bUneditableIfUnwritable, IsTrue());

	EXPECT_THAT(sFile.m_bEnableUnmodifiedOverwrite, IsFalse());

	EXPECT_THAT(sFile.m_bNoFilterSaveNew, IsTrue());
	EXPECT_THAT(sFile.m_bNoFilterSaveFile, IsTrue());

	EXPECT_THAT(sFile.m_bDropFileAndClose, IsFalse());
	EXPECT_THAT(sFile.m_nDropFileNumMax, 8);
	EXPECT_THAT(sFile.m_bRestoreCurPosition, IsTrue());
	EXPECT_THAT(sFile.m_bRestoreBookmarks, IsTrue());
	EXPECT_THAT(sFile.m_bAutoMIMEdecode, IsFalse());
	EXPECT_THAT(sFile.m_bQueryIfCodeChange, IsTrue());
	EXPECT_THAT(sFile.m_bAlertIfFileNotExist, IsFalse());
	EXPECT_THAT(sFile.m_bAlertIfLargeFile, IsFalse());
	EXPECT_THAT(sFile.m_nAlertFileSize, 10);

	return true;
}

// [バックアップ]タブ
MATCHER(IsInitializedCommonSettingBackup, "Checks if CommonSetting_Backup is properly initialized") {
    const CommonSetting_Backup& sBackup = arg;

	EXPECT_THAT(sBackup.m_bBackUp, IsFalse());
	EXPECT_THAT(sBackup.m_bBackUpDialog, IsTrue());
	EXPECT_THAT(sBackup.m_bBackUpFolder, IsFalse());
	EXPECT_THAT(sBackup.m_szBackUpFolder, StrEq(L""));
	EXPECT_THAT(sBackup.m_nBackUpType, 2);
	EXPECT_THAT(sBackup.m_nBackUpType_Opt1, BKUP_YEAR | BKUP_MONTH | BKUP_DAY);
	EXPECT_THAT(sBackup.m_nBackUpType_Opt2, ('b' << 16 ) + 10);
	EXPECT_THAT(sBackup.m_nBackUpType_Opt3, 5);
	EXPECT_THAT(sBackup.m_nBackUpType_Opt4, 0);
	EXPECT_THAT(sBackup.m_nBackUpType_Opt5, 0);
	EXPECT_THAT(sBackup.m_nBackUpType_Opt6, 0);
	EXPECT_THAT(sBackup.m_bBackUpDustBox, IsFalse());
	EXPECT_THAT(sBackup.m_bBackUpPathAdvanced, IsFalse());
	EXPECT_THAT(sBackup.m_szBackUpPathAdvanced, StrEq(L""));

	return true;
}

// [書式]タブ
MATCHER(IsInitializedCommonSettingFormat, "Checks if CommonSetting_Format is properly initialized") {
	const CommonSetting_Format& sFormat = arg;

	EXPECT_THAT(sFormat.m_szMidashiKigou, StrEq(L"１２３４５６７８９０（(［[「『【■□▲△▼▽◆◇○◎●§・※☆★第①②③④⑤⑥⑦⑧⑨⑩⑪⑫⑬⑭⑮⑯⑰⑱⑲⑳ⅠⅡⅢⅣⅤⅥⅦⅧⅨⅩ一二三四五六七八九十壱弐参伍"));
	EXPECT_THAT(sFormat.m_szInyouKigou, StrEq(L"> "));
	EXPECT_THAT(sFormat.m_nDateFormatType, 0);
	EXPECT_THAT(sFormat.m_szDateFormat, StrEq(L"yyyy\'年\'M\'月\'d\'日(\'dddd\')\'"));
	EXPECT_THAT(sFormat.m_nTimeFormatType, 0);
	EXPECT_THAT(sFormat.m_szTimeFormat, StrEq(L"tthh\'時\'mm\'分\'ss\'秒\'"));

	return true;
}

// [検索]タブ
MATCHER(IsInitializedCommonSettingSearch, "Checks if CommonSetting_Search is properly initialized") {
	const CommonSetting_Search& sSearch = arg;

	EXPECT_THAT(sSearch.m_sSearchOption.bRegularExp, IsFalse());
	EXPECT_THAT(sSearch.m_sSearchOption.bLoHiCase, IsFalse());
	EXPECT_THAT(sSearch.m_sSearchOption.bWordOnly, IsFalse());

	EXPECT_THAT(sSearch.m_bConsecutiveAll, 0);
	EXPECT_THAT(sSearch.m_bSelectedArea, IsFalse());
	EXPECT_THAT(sSearch.m_bNOTIFYNOTFOUND, IsTrue());

	EXPECT_THAT(sSearch.m_bGrepSubFolder, IsTrue());
	EXPECT_THAT(sSearch.m_nGrepOutputLineType, 1);
	EXPECT_THAT(sSearch.m_nGrepOutputStyle, 1);
	EXPECT_THAT(sSearch.m_bGrepOutputFileOnly, IsFalse());
	EXPECT_THAT(sSearch.m_bGrepOutputBaseFolder, IsFalse());
	EXPECT_THAT(sSearch.m_bGrepSeparateFolder, IsFalse());
	EXPECT_THAT(sSearch.m_bGrepBackup, IsTrue());
	EXPECT_THAT(sSearch.m_bGrepDefaultFolder, IsFalse());
	EXPECT_THAT(sSearch.m_nGrepCharSet, CODE_AUTODETECT);
	EXPECT_THAT(sSearch.m_bGrepRealTimeView, IsFalse());
	EXPECT_THAT(sSearch.m_bCaretTextForSearch, IsTrue());
	EXPECT_THAT(sSearch.m_bInheritKeyOtherView, IsTrue());
	EXPECT_THAT(sSearch.m_szRegexpLib, StrEq(L""));
	EXPECT_THAT(sSearch.m_bGTJW_RETURN, IsTrue());
	EXPECT_THAT(sSearch.m_bGTJW_LDBLCLK, IsTrue());

	EXPECT_THAT(sSearch.m_bGrepExitConfirm, IsFalse());

	EXPECT_THAT(sSearch.m_bAutoCloseDlgFind, IsTrue());
	EXPECT_THAT(sSearch.m_bSearchAll, IsFalse());
	EXPECT_THAT(sSearch.m_bAutoCloseDlgReplace, IsTrue());

	EXPECT_THAT(sSearch.m_nTagJumpMode, 1);
	EXPECT_THAT(sSearch.m_nTagJumpModeKeyword, 3);

	return true;
}

// [キー割り当て]タブ
MATCHER(IsInitializedCommonSettingKeyBind, "Checks if CommonSetting_KeyBind is properly initialized") {
    const CommonSetting_KeyBind& sKeyBind = arg;

#pragma push_macro("KEY_NAME")

#define KEY_NAME(name)	(LPCWSTR)STR_KEY_BIND_##name

	constexpr auto _SQL_RUN	= F_PLSQL_COMPILE_ON_SQLPLUS;
	constexpr auto _COPYWITHLINENUM = F_COPYLINESWITHLINENUMBER;

	const std::array<KEYDATAINIT, 100> KeyDataInit = {{

		/* マウスボタン */
		//keycode,	keyname,					なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 0x0100,	{ KEY_NAME(DBL_CLICK) },	{ F_SELECTWORD,		F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD,		F_SELECTWORD,			F_SELECTWORD }, },
		{ 0x0101,	{ KEY_NAME(R_CLICK) },		{ F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_0,					F_0,				F_0,					F_0 }, },
		{ 0x0102,	{ KEY_NAME(MID_CLICK) },	{ F_AUTOSCROLL,		F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x0103,	{ KEY_NAME(LSD_CLICK) },	{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x0104,	{ KEY_NAME(RSD_CLICK) },	{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x0105,	{ KEY_NAME(TRI_CLICK) },	{ F_SELECTLINE,		F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE,		F_SELECTLINE,			F_SELECTLINE }, },
		{ 0x0106,	{ KEY_NAME(QUA_CLICK) },	{ F_SELECTALL,		F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL,		F_SELECTALL,			F_SELECTALL }, },
		{ 0x0107,	{ KEY_NAME(WHEEL_UP) },		{ F_WHEELUP,		F_WHEELUP,			F_SETFONTSIZEUP,		F_WHEELUP,			F_WHEELUP,				F_WHEELUP,			F_WHEELUP,				F_WHEELUP }, },
		{ 0x0108,	{ KEY_NAME(WHEEL_DOWN) },	{ F_WHEELDOWN,		F_WHEELDOWN,		F_SETFONTSIZEDOWN,		F_WHEELDOWN,		F_WHEELDOWN,			F_WHEELDOWN,		F_WHEELDOWN,			F_WHEELDOWN }, },
		{ 0x0109,	{ KEY_NAME(WHEEL_LEFT) },	{ F_WHEELLEFT,		F_WHEELLEFT,		F_WHEELLEFT,			F_WHEELLEFT,		F_WHEELLEFT,			F_WHEELLEFT,		F_WHEELLEFT,			F_WHEELLEFT }, },
		{ 0x010A,	{ KEY_NAME(WHEEL_RIGHT) },	{ F_WHEELRIGHT,		F_WHEELRIGHT,		F_WHEELRIGHT,			F_WHEELRIGHT,		F_WHEELRIGHT,			F_WHEELRIGHT,		F_WHEELRIGHT,			F_WHEELRIGHT }, },

		/* ファンクションキー */
		//keycode,	keyname,					なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_F1,	{ L"F1" },					{ F_EXTHTMLHELP,	F_MENU_ALLFUNC,		F_EXTHELP1,				F_ABOUT,			F_HELP_CONTENTS,		F_HELP_SEARCH,		F_0,					F_0 }, },
		{ VK_F2,	{ L"F2" },					{ F_BOOKMARK_NEXT,	F_BOOKMARK_PREV,	F_BOOKMARK_SET,			F_BOOKMARK_RESET,	F_BOOKMARK_VIEW,		F_0,				F_0,					F_0 }, },
		{ VK_F3,	{ L"F3" },					{ F_SEARCH_NEXT,	F_SEARCH_PREV,		F_SEARCH_CLEARMARK,		F_JUMP_SRCHSTARTPOS,F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F4,	{ L"F4" },					{ F_SPLIT_V,		F_SPLIT_H,			F_0,					F_FILECLOSE_OPEN,	F_0,					F_EXITALLEDITORS,	F_EXITALL,				F_0 }, },
		{ VK_F5,	{ L"F5" },					{ F_REDRAW,			F_0,				F_EXECMD_DIALOG,		F_0,				F_UUDECODE,				F_0,				F_TABTOSPACE,			F_SPACETOTAB }, },
		{ VK_F6,	{ L"F6" },					{ F_BEGIN_SEL,		F_BEGIN_BOX,		F_TOLOWER,				F_0,				F_BASE64DECODE,			F_0,				F_0,					F_0 }, },
		{ VK_F7,	{ L"F7" },					{ F_CUT,			F_0,				F_TOUPPER,				F_0,				F_CODECNV_UTF72SJIS,	F_CODECNV_SJIS2UTF7,F_FILE_REOPEN_UTF7,		F_0 }, },
		{ VK_F8,	{ L"F8" },					{ F_COPY,			F_COPY_CRLF,		F_TOHANKAKU,			F_0,				F_CODECNV_UTF82SJIS,	F_CODECNV_SJIS2UTF8,F_FILE_REOPEN_UTF8,		F_0 }, },
		{ VK_F9,	{ L"F9" },					{ F_PASTE,			F_PASTEBOX,			F_TOZENKAKUKATA,		F_0,				F_CODECNV_UNICODE2SJIS,	F_0,				F_FILE_REOPEN_UNICODE,	F_0 }, },
		{ VK_F10,	{ L"F10" },					{ _SQL_RUN,			F_DUPLICATELINE,	F_TOZENKAKUHIRA,		F_0,				F_CODECNV_EUC2SJIS,		F_CODECNV_SJIS2EUC,	F_FILE_REOPEN_EUC,		F_0 }, },
		{ VK_F11,	{ L"F11" },					{ F_OUTLINE,		F_ACTIVATE_SQLPLUS,	F_HANKATATOZENKATA,		F_0,				F_CODECNV_EMAIL,		F_CODECNV_SJIS2JIS,	F_FILE_REOPEN_JIS,		F_0 }, },
		{ VK_F12,	{ L"F12" },					{ F_TAGJUMP,		F_TAGJUMPBACK,		F_HANKATATOZENHIRA,		F_0,				F_CODECNV_AUTO2SJIS,	F_0,				F_FILE_REOPEN_SJIS,		F_0 }, },
		{ VK_F13,	{ L"F13" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F14,	{ L"F14" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F15,	{ L"F15" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F16,	{ L"F16" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F17,	{ L"F17" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F18,	{ L"F18" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F19,	{ L"F19" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F20,	{ L"F20" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F21,	{ L"F21" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F22,	{ L"F22" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F23,	{ L"F23" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_F24,	{ L"F24" },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },

		/* 特殊キー */
		//keycode,	keyname,					なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ VK_TAB,	{ L"Tab" },					{ F_INDENT_TAB,		F_UNINDENT_TAB,		F_NEXTWINDOW,			F_PREVWINDOW,		F_0,					F_0,				F_0,					F_0 }, },
		{ VK_RETURN,{ L"Enter" },				{ F_0,				F_0,				F_COMPARE,				F_0,				F_PROPERTY_FILE,		F_0,				F_0,					F_0 }, },
		{ VK_ESCAPE,{ L"Esc" },					{ F_CANCEL_MODE,	F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_BACK,	{ L"BkSp" },				{ F_DELETE_BACK,	F_0,				F_WordDeleteToStart,	F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_INSERT,{ L"Ins" },					{ F_CHGMOD_INS,		F_PASTE,			F_COPY,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_DELETE,{ L"Del" },					{ F_DELETE,			F_CUT,				F_WordDeleteToEnd,		F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_HOME,	{ L"Home" },				{ F_GOLINETOP,		F_GOLINETOP_SEL,	F_GOFILETOP,			F_GOFILETOP_SEL,	F_GOLINETOP_BOX,		F_0,				F_GOFILETOP_BOX,		F_0 }, },
		{ VK_END,	{ L"End(Help)" },			{ F_GOLINEEND,		F_GOLINEEND_SEL,	F_GOFILEEND,			F_GOFILEEND_SEL,	F_GOLINEEND_BOX,		F_0,				F_GOFILEEND_BOX,		F_0 }, },
		{ VK_LEFT,	{ L"←" },					{ F_LEFT,			F_LEFT_SEL,			F_WORDLEFT,				F_WORDLEFT_SEL,		F_LEFT_BOX,				F_0,				F_WORDLEFT_BOX,			F_0 }, },
		{ VK_UP,	{ L"↑" },					{ F_UP,				F_UP_SEL,			F_WndScrollDown,		F_UP2_SEL,			F_UP_BOX,				F_0,				F_UP2_BOX,				F_MAXIMIZE_V },}, 
		{ VK_RIGHT,	{ L"→" },					{ F_RIGHT,			F_RIGHT_SEL,		F_WORDRIGHT,			F_WORDRIGHT_SEL,	F_RIGHT_BOX,			F_0,				F_WORDRIGHT_BOX,		F_MAXIMIZE_H },}, 
		{ VK_DOWN,	{ L"↓" },					{ F_DOWN,			F_DOWN_SEL,			F_WndScrollUp,			F_DOWN2_SEL,		F_DOWN_BOX,				F_0,				F_DOWN2_BOX,			F_MINIMIZE_ALL },}, 
		{ VK_NEXT,	{ L"PgDn(RollUp)" },		{ F_1PageDown,		F_1PageDown_Sel,	F_HalfPageDown,			F_HalfPageDown_Sel,	F_1PageDown_BOX,		F_0,				F_HalfPageDown_BOX,		F_0 }, },
		{ VK_PRIOR,	{ L"PgUp(RollDn)" },		{ F_1PageUp,		F_1PageUp_Sel,		F_HalfPageUp,			F_HalfPageUp_Sel,	F_1PageUp_BOX,			F_0,				F_HalfPageUp_BOX,		F_0 }, },
		{ VK_SPACE,	{ L"Space" },				{ F_INDENT_SPACE,	F_UNINDENT_SPACE,	F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 }, },

		/* 数字 */
		//keycode,	keyname,					なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ '0',		{ L"0" },					{ F_0,				F_0,				F_0,					F_0,				F_CUSTMENU_10,			F_CUSTMENU_20,		F_0,					F_0 }, },
		{ '1',		{ L"1" },					{ F_0,				F_0,				F_SHOWTOOLBAR,			F_CUSTMENU_21,		F_CUSTMENU_1,			F_CUSTMENU_11,		F_0,					F_0 }, },
		{ '2',		{ L"2" },					{ F_0,				F_0,				F_SHOWFUNCKEY,			F_CUSTMENU_22,		F_CUSTMENU_2,			F_CUSTMENU_12,		F_0,					F_0 }, },
		{ '3',		{ L"3" },					{ F_0,				F_0,				F_SHOWSTATUSBAR,		F_CUSTMENU_23,		F_CUSTMENU_3,			F_CUSTMENU_13,		F_0,					F_0 }, },
		{ '4',		{ L"4" },					{ F_0,				F_0,				F_TYPE_LIST,			F_CUSTMENU_24,		F_CUSTMENU_4,			F_CUSTMENU_14,		F_0,					F_0 }, },
		{ '5',		{ L"5" },					{ F_0,				F_0,				F_OPTION_TYPE,			F_0,				F_CUSTMENU_5,			F_CUSTMENU_15,		F_0,					F_0 }, },
		{ '6',		{ L"6" },					{ F_0,				F_0,				F_OPTION,				F_0,				F_CUSTMENU_6,			F_CUSTMENU_16,		F_0,					F_0 }, },
		{ '7',		{ L"7" },					{ F_0,				F_0,				F_FONT,					F_0,				F_CUSTMENU_7,			F_CUSTMENU_17,		F_0,					F_0 }, },
		{ '8',		{ L"8" },					{ F_0,				F_0,				F_0,					F_0,				F_CUSTMENU_8,			F_CUSTMENU_18,		F_0,					F_0 }, },
		{ '9',		{ L"9" },					{ F_0,				F_0,				F_0,					F_0,				F_CUSTMENU_9,			F_CUSTMENU_19,		F_0,					F_0 }, },

		/* アルファベット */
		//keycode,	keyname,					なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 'A',		{ L"A" },					{ F_0,				F_0,				F_SELECTALL,			F_0,				F_SORT_ASC,				F_0,				F_0,					F_0 }, },
		{ 'B',		{ L"B" },					{ F_0,				F_0,				F_BROWSE,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'C',		{ L"C" },					{ F_0,				F_0,				F_COPY,					F_OPEN_HfromtoC,	F_0,					F_0,				F_0,					F_0 }, },
		{ 'D',		{ L"D" },					{ F_0,				F_0,				F_WordCut,				F_WordDelete,		F_SORT_DESC,			F_0,				F_0,					F_0 }, },
		{ 'E',		{ L"E" },					{ F_0,				F_0,				F_CUT_LINE,				F_DELETE_LINE,		F_0,					F_0,				F_CASCADE,				F_0 }, },
		{ 'F',		{ L"F" },					{ F_0,				F_0,				F_SEARCH_DIALOG,		F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'G',		{ L"G" },					{ F_0,				F_0,				F_GREP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'H',		{ L"H" },					{ F_0,				F_0,				F_CURLINECENTER,		F_OPEN_HfromtoC,	F_0,					F_0,				F_TILE_V,				F_0 }, },
		{ 'I',		{ L"I" },					{ F_0,				F_0,				F_DUPLICATELINE,		F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'J',		{ L"J" },					{ F_0,				F_0,				F_JUMP_DIALOG,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'K',		{ L"K" },					{ F_0,				F_0,				F_LineCutToEnd,			F_LineDeleteToEnd,	F_0,					F_0,				F_0,					F_0 }, },
		{ 'L',		{ L"L" },					{ F_0,				F_0,				F_LOADKEYMACRO,			F_EXECKEYMACRO,		F_LTRIM,				F_0,				F_TOLOWER,				F_TOUPPER }, },
		{ 'M',		{ L"M" },					{ F_0,				F_0,				F_SAVEKEYMACRO,			F_RECKEYMACRO,		F_MERGE,				F_0,				F_0,					F_0 }, },
		{ 'N',		{ L"N" },					{ F_0,				F_0,				F_FILENEW,				F_0,				F_JUMPHIST_NEXT,		F_0,				F_0,					F_0 }, },
		{ 'O',		{ L"O" },					{ F_0,				F_0,				F_FILEOPEN,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'P',		{ L"P" },					{ F_0,				F_0,				F_PRINT,				F_PRINT_PREVIEW,	F_JUMPHIST_PREV,		F_0,				F_PRINT_PAGESETUP,		F_0 }, },
		{ 'Q',		{ L"Q" },					{ F_0,				F_0,				F_CREATEKEYBINDLIST,	F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'R',		{ L"R" },					{ F_0,				F_0,				F_REPLACE_DIALOG,		F_0,				F_RTRIM,				F_0,				F_0,					F_0 }, },
		{ 'S',		{ L"S" },					{ F_0,				F_0,				F_FILESAVE,				F_FILESAVEAS_DIALOG,F_0,					F_0,				F_TMPWRAPSETTING,		F_0 }, },
		{ 'T',		{ L"T" },					{ F_0,				F_0,				F_TAGJUMP,				F_TAGJUMPBACK,		F_0,					F_0,				F_TILE_H,				F_0 }, },
		{ 'U',		{ L"U" },					{ F_0,				F_0,				F_LineCutToStart,		F_LineDeleteToStart,F_0,					F_0,				F_WRAPWINDOWWIDTH,		F_0 }, },
		{ 'V',		{ L"V" },					{ F_0,				F_0,				F_PASTE,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'W',		{ L"W" },					{ F_0,				F_0,				F_SELECTWORD,			F_0,				F_0,					F_0,				F_TMPWRAPWINDOW,		F_0 }, },
		{ 'X',		{ L"X" },					{ F_0,				F_0,				F_CUT,					F_0,				F_0,					F_0,				F_TMPWRAPNOWRAP,		F_0 }, },
		{ 'Y',		{ L"Y" },					{ F_0,				F_0,				F_REDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 'Z',		{ L"Z" },					{ F_0,				F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },

		/* 記号 */
		//keycode,	keyname,					なし,				Shitf+,				Ctrl+,					Shift+Ctrl+,		Alt+,					Shit+Alt+,			Ctrl+Alt+,				Shift+Ctrl+Alt+
		{ 0x00bd,	{ L"-" },					{ F_0,				F_0,				F_COPYFNAME,			F_SPLIT_V,			F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00de,	{ KEY_NAME(HAT_ENG_QT) },	{ F_0,				F_0,				F_COPYTAG,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00dc,	{ L"\\" },					{ F_0,				F_0,				F_COPYPATH,				F_SPLIT_H,			F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00c0,	{ KEY_NAME(AT_ENG_BQ) },	{ F_0,				F_0,				F_COPYLINES,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00db,	{ L"[" },					{ F_0,				F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00bb,	{ L";" },					{ F_0,				F_0,				F_0,					F_SPLIT_VH,			F_INS_DATE,				F_0,				F_0,					F_0 }, },
		{ 0x00ba,	{ L":" },					{ F_0,				F_0,				_COPYWITHLINENUM,		F_0,				F_INS_TIME,				F_0,				F_0,					F_0 }, },
		{ 0x00dd,	{ L"]" },					{ F_0,				F_0,				F_BRACKETPAIR,			F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00bc,	{ L"," },					{ F_0,				F_0,				F_0,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00be,	{ L"." },					{ F_0,				F_0,				F_COPYLINESASPASSAGE,	F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00bf,	{ L"/" },					{ F_0,				F_0,				F_HOKAN,				F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00e2,	{ L"_" },					{ F_0,				F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ 0x00df,	{ L"_(PC-98)" },			{ F_0,				F_0,				F_UNDO,					F_0,				F_0,					F_0,				F_0,					F_0 }, },
		{ VK_APPS,	{ KEY_NAME(APLI) },			{ F_MENU_RBUTTON,	F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON,		F_MENU_RBUTTON,			F_MENU_RBUTTON }, }
	}};

#pragma pop_macro("KEY_NAME")

	EXPECT_THAT(sKeyBind.m_nKeyNameArrNum, std::size(KeyDataInit));

	//InitKeyAssign
	for (size_t i = 0; i < std::size(KeyDataInit); ++i) {
		const auto& keydata = sKeyBind.m_pKeyNameArr[i];
		const auto& initData = KeyDataInit[i];
		EXPECT_THAT(keydata.m_nKeyCode, initData.m_nKeyCode);
		if (!IS_INTRESOURCE(initData.m_nKeyNameId)) {
			EXPECT_THAT(keydata.m_szKeyName, StrEq(initData.m_pszKeyName));
		}
		for (size_t j = 0; j < std::size(initData.m_nFuncCodeArr); ++j) {
			EXPECT_THAT(keydata.m_nFuncCodeArr[j], initData.m_nFuncCodeArr[j]);
		}
	}

	for (size_t i = 0; i < std::size(KeyDataInit); ++i) {
		EXPECT_THAT(sKeyBind.m_VKeyToKeyNameArr[KeyDataInit[i].m_nKeyCode], BYTE(i));
	}

	return true;
}

// [カスタムメニュー]タブ
MATCHER(IsInitializedCommonSettingCustomMenu, "Checks if CommonSetting_CustomMenu is properly initialized") {
    const CommonSetting_CustomMenu& sCustomMenu = arg;

	constexpr int EDIT_RMENU = 0;
	constexpr int CUST_MENU = 1;
	constexpr int TAB_RMENU = CUSTMENU_INDEX_FOR_TABWND;

	const std::array<SMenuItem, 30> editRMenu = {{
		{ F_UNDO, 'U' },
		{ F_REDO, 'R' },
		{ F_0 },
		{ F_CUT, 'T' },
		{ F_COPY, 'C' },
		{ F_PASTE, 'P' },
		{ F_DELETE, 'D' },
		{ F_0 },
		{ F_COPY_CRLF, 'L' },
		{ F_COPY_ADDCRLF, 'H' },
		{ F_PASTEBOX, 'X' },
		{ F_0 },
		{ F_SELECTALL, 'A' },
		{ F_0 },
		{ F_TAGJUMP, 'G' },
		{ F_TAGJUMPBACK, 'B' },
		{ F_0 },
		{ F_COPYLINES, '@' },
		{ F_COPYLINESASPASSAGE, '.' },
		{ F_0 },
		{ F_COPYFNAME, 'F' },
		{ F_COPYPATH, '\\' },
		{ F_COPYDIRPATH, 'O' },
		{ F_0 },
		{ F_OPEN_FOLDER_IN_EXPLORER, 'E' },
		{ F_OPEN_COMMAND_PROMPT, 'W' },
		{ F_OPEN_COMMAND_PROMPT_AS_ADMIN, 'w' },
		{ F_OPEN_POWERSHELL, 'P' },
		{ F_OPEN_POWERSHELL_AS_ADMIN, 'p' },
		{ F_PROPERTY_FILE, 'F' },
	}};

	const std::array<SMenuItem, 7> custMenu = {{
		{ F_FILEOPEN, 'O' },
		{ F_FILESAVE, 'S' },
		{ F_NEXTWINDOW, 'N' },
		{ F_TOLOWER, 'L' },
		{ F_TOUPPER, 'U' },
		{ F_0 },
		{ F_WINCLOSE, 'C' },
	}};

	const std::array<SMenuItem, 27> tabRMenu = {{
		{ F_FILESAVE, 'S' },
		{ F_FILESAVEAS_DIALOG, 'A' },
		{ F_FILECLOSE, 'R' },
		{ F_FILECLOSE_OPEN, 'L' },
		{ F_WINCLOSE, 'C' },
		{ F_FILE_REOPEN, 'W' },
		{ F_0 },
		{ F_COPYFNAME, 'F' },
		{ F_COPYPATH },
		{ F_COPYDIRPATH },
		{ F_0 },
		{ F_OPEN_FOLDER_IN_EXPLORER, 'F' },
		{ F_OPEN_COMMAND_PROMPT, 'W' },
		{ F_OPEN_COMMAND_PROMPT_AS_ADMIN, 'w' },
		{ F_OPEN_POWERSHELL, 'P' },
		{ F_OPEN_POWERSHELL_AS_ADMIN, 'p' },
		{ F_0 },
		{ F_GROUPCLOSE, 'G' },
		{ F_TAB_CLOSEOTHER, 'O' },
		{ F_TAB_CLOSELEFT, 'H' },
		{ F_TAB_CLOSERIGHT, 'M' },
		{ F_0 },
		{ F_TAB_MOVERIGHT, '0' },
		{ F_TAB_MOVELEFT, '1' },
		{ F_TAB_SEPARATE, 'E' },
		{ F_TAB_JOINTNEXT, 'X' },
		{ F_TAB_JOINTPREV, 'V' },
	}};

	for (int i = 0; i < MAX_CUSTOM_MENU; ++i) {
		EXPECT_THAT(sCustomMenu.m_szCustMenuNameArr[i], StrEq(L""));
		EXPECT_THAT(sCustomMenu.m_bCustMenuPopupArr[i], IsTrue());
	}

	/* 右クリックメニュー */
	EXPECT_THAT(sCustomMenu.m_nCustMenuItemNumArr[EDIT_RMENU], std::size(editRMenu));
	for (size_t i = 0; i < std::size(editRMenu); ++i) {
		EXPECT_THAT(sCustomMenu.m_nCustMenuItemFuncArr[EDIT_RMENU][i], editRMenu[i].m_eFuncCode);
		EXPECT_THAT(sCustomMenu.m_nCustMenuItemKeyArr [EDIT_RMENU][i], editRMenu[i].m_chAccessKey);
	}

	/* カスタムメニュー１ */
	EXPECT_THAT(sCustomMenu.m_nCustMenuItemNumArr[CUST_MENU], std::size(custMenu));
	for (size_t i = 0; i < std::size(custMenu); ++i) {
		EXPECT_THAT(sCustomMenu.m_nCustMenuItemFuncArr[CUST_MENU][i], custMenu[i].m_eFuncCode);
		EXPECT_THAT(sCustomMenu.m_nCustMenuItemKeyArr [CUST_MENU][i], custMenu[i].m_chAccessKey);
	}

	/* タブメニュー */
	EXPECT_THAT(sCustomMenu.m_nCustMenuItemNumArr[TAB_RMENU], std::size(tabRMenu));
	for (size_t i = 0; i < std::size(tabRMenu); ++i) {
		EXPECT_THAT(sCustomMenu.m_nCustMenuItemFuncArr[TAB_RMENU][i], tabRMenu[i].m_eFuncCode);
		EXPECT_THAT(sCustomMenu.m_nCustMenuItemKeyArr [TAB_RMENU][i], tabRMenu[i].m_chAccessKey);
	}

	return true;
}

// [ツールバー]タブ
MATCHER(IsInitializedCommonSettingToolBar, "Checks if CommonSetting_ToolBar is properly initialized") {
	const CommonSetting_ToolBar& sToolBar = arg;

	constexpr std::array<int, 25> DEFAULT_TOOL_FUNCS = {
		F_FILENEW,				//新規作成
		F_FILEOPEN_DROPDOWN,	//ファイルを開く(DropDown)
		F_FILESAVE,				//上書き保存
		F_FILESAVEAS_DIALOG,	//名前を付けて保存
		F_SEPARATOR,

		F_UNDO,					//元に戻す(Undo)
		F_REDO,					//やり直し(Redo)
		F_SEPARATOR,

		F_JUMPHIST_PREV,		//移動履歴: 前へ
		F_JUMPHIST_NEXT,		//移動履歴: 次へ
		F_SEPARATOR,

		F_SEARCH_DIALOG,		//検索
		F_SEARCH_NEXT,			//次を検索
		F_SEARCH_PREV,			//前を検索
		F_REPLACE_DIALOG,		//置換
		F_SEARCH_CLEARMARK,		//検索マークのクリア
		F_GREP_DIALOG,			//Grep
		F_SEPARATOR,

		F_OUTLINE,				//アウトライン解析
		F_SEPARATOR,

		F_TYPE_LIST,			//タイプ別設定一覧
		F_OPTION_TYPE,			//タイプ別設定
		F_OPTION,				//共通設定
		F_SEPARATOR,

		F_MENU_ALLFUNC,			//コマンド一覧
	};

	CMenuDrawer cMenuDrawer;
	EXPECT_THAT(sToolBar.m_nToolBarButtonNum, std::size(DEFAULT_TOOL_FUNCS));
	for (size_t i = 0; i < std::size(DEFAULT_TOOL_FUNCS); ++i) {
		int buttonId = 0;
		if (const auto funcCode = DEFAULT_TOOL_FUNCS[i]; F_SEPARATOR != funcCode) {
			buttonId = cMenuDrawer.FindToolbarNoFromCommandId(funcCode);
		}
		EXPECT_THAT(sToolBar.m_nToolBarButtonIdxArr[i], buttonId);
	}
	EXPECT_THAT(sToolBar.m_bToolBarIsFlat, IsFalse());

	return true;
}

// [強調キーワード]タブ
MATCHER(IsInitializedCommonSettingSpecialKeyword, "Checks if CommonSetting_SpecialKeyword is properly initialized") {
	const CommonSetting_SpecialKeyword& sSpecialKeyword = arg;

	EXPECT_THAT(sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, 0);

	//InitKeyword( m_pShareData );

	EXPECT_THAT(sSpecialKeyword.m_szKeyWordSetDir, StrEq(""));	//未使用。削除可？

	return true;
}

// [支援]タブ
MATCHER_P2(IsInitializedCommonSettingHelper, lfIconTitle, nIconPointSize, "Checks if CommonSetting_Helper is properly initialized") {
    const CommonSetting_Helper& sHelper = arg;

	EXPECT_THAT(sHelper.m_lf, lfIconTitle);
	EXPECT_THAT(sHelper.m_nPointSize, nIconPointSize);

	EXPECT_THAT(sHelper.m_szExtHelp, StrEq(L""));
	EXPECT_THAT(sHelper.m_szExtHtmlHelp, StrEq(L""));
		
	EXPECT_THAT(sHelper.m_szMigemoDll, StrEq(L""));
	EXPECT_THAT(sHelper.m_szMigemoDict, StrEq(L""));

	EXPECT_THAT(sHelper.m_bHtmlHelpIsSingle, IsTrue());

	EXPECT_THAT(sHelper.m_bHokanKey_RETURN, IsTrue());
	EXPECT_THAT(sHelper.m_bHokanKey_TAB, IsFalse());
	EXPECT_THAT(sHelper.m_bHokanKey_RIGHT, IsTrue());

	return true;
}

// [アウトライン]タブ
MATCHER(IsInitializedCommonSettingOutLine, "Checks if CommonSetting_OutLine is properly initialized") {
	const CommonSetting_OutLine& sOutline = arg;

	EXPECT_THAT(sOutline.m_nOutlineDockSet, 0);
	EXPECT_THAT(sOutline.m_bOutlineDockSync, IsTrue());
	EXPECT_THAT(sOutline.m_bOutlineDockDisp, IsFalse());
	EXPECT_THAT(sOutline.m_eOutlineDockSide, DOCKSIDE_FLOAT);
	EXPECT_THAT(sOutline.m_cxOutlineDockLeft, 0);
	EXPECT_THAT(sOutline.m_cyOutlineDockTop, 0);
	EXPECT_THAT(sOutline.m_cxOutlineDockRight, 0);
	EXPECT_THAT(sOutline.m_cyOutlineDockBottom, 0);
	EXPECT_THAT(sOutline.m_nDockOutline, OUTLINE_TEXT);
	EXPECT_THAT(sOutline.m_bAutoCloseDlgFuncList, IsFalse());
	EXPECT_THAT(sOutline.m_bMarkUpBlankLineEnable, IsFalse());
	EXPECT_THAT(sOutline.m_bFunclistSetFocusOnJump, IsFalse());

	EXPECT_THAT(sOutline.m_sFileTree.m_bProject, IsTrue());
	EXPECT_THAT(sOutline.m_sFileTree.m_nItemCount, 1);
	EXPECT_THAT(sOutline.m_sFileTree.m_aItems[0].m_eFileTreeItemType, EFileTreeItemType_Grep );
	EXPECT_THAT(sOutline.m_sFileTree.m_aItems[0].m_szTargetPath, StrEq(L"."));
	EXPECT_THAT(sOutline.m_sFileTree.m_aItems[0].m_szLabelName, StrEq(L""));
	EXPECT_THAT(sOutline.m_sFileTree.m_aItems[0].m_nDepth, 0);
	EXPECT_THAT(sOutline.m_sFileTree.m_aItems[0].m_szTargetFile, StrEq(L"*.*"));
	EXPECT_THAT(sOutline.m_sFileTree.m_aItems[0].m_bIgnoreHidden, IsTrue());
	EXPECT_THAT(sOutline.m_sFileTree.m_aItems[0].m_bIgnoreReadOnly, IsFalse());
	EXPECT_THAT(sOutline.m_sFileTree.m_aItems[0].m_bIgnoreSystem, IsFalse());

	for (size_t i = 1; i < std::size(sOutline.m_sFileTree.m_aItems); ++i) {
		const auto& item = sOutline.m_sFileTree.m_aItems[i];
		EXPECT_THAT(item.m_eFileTreeItemType, EFileTreeItemType_Grep );
		EXPECT_THAT(item.m_szTargetPath, StrEq(L""));
		EXPECT_THAT(item.m_szLabelName, StrEq(L""));
		EXPECT_THAT(item.m_nDepth, 0);
		EXPECT_THAT(item.m_szTargetFile, StrEq(L""));
		EXPECT_THAT(item.m_bIgnoreHidden, IsTrue());
		EXPECT_THAT(item.m_bIgnoreReadOnly, IsFalse());
		EXPECT_THAT(item.m_bIgnoreSystem, IsFalse());
	}

	EXPECT_THAT(sOutline.m_sFileTreeDefIniName, StrEq(L"_sakurafiletree.ini"));

	return true;
}

// [ファイル内容比較]タブ
MATCHER(IsInitializedCommonSettingCompare, "Checks if CommonSetting_Compare is properly initialized") {
    const CommonSetting_Compare& sCompare = arg;

	EXPECT_THAT(sCompare.m_bCompareAndTileHorz, IsTrue());

	return true;
}

// [ビュー]タブ
MATCHER_P(IsInitializedCommonSettingView, lf, "Checks if CommonSetting_View is properly initialized") {
    const CommonSetting_View& sView = arg;

	EXPECT_THAT(sView.m_lf, lf);
	EXPECT_THAT(sView.m_nPointSize, 0);
	EXPECT_THAT(sView.m_bFontIs_FIXED_PITCH, IsTrue());

	return true;
}

// [マクロ]タブ
MATCHER_P(IsInitializedCommonSettingMacro, iniFolder, "Checks if CommonSetting_Macro is properly initialized") {
    const CommonSetting_Macro& sMacro = arg;

	EXPECT_THAT(sMacro.m_szKeyMacroFileName, StrEq(L""));

	EXPECT_THAT(sMacro.m_szMACROFOLDER, StrEq(iniFolder));

	EXPECT_THAT(sMacro.m_nMacroOnOpened, -1);
	EXPECT_THAT(sMacro.m_nMacroOnTypeChanged, -1);
	EXPECT_THAT(sMacro.m_nMacroOnSave, -1);
	EXPECT_THAT(sMacro.m_nMacroCancelTimer, 10);

	return true;
}

// [ファイル名表示]タブ
MATCHER(IsInitializedCommonSettingFileName, "Checks if CommonSetting_FileName is properly initialized") {
    const CommonSetting_FileName& sFileName = arg;

	EXPECT_THAT(sFileName.m_bTransformShortPath, IsTrue());
	EXPECT_THAT(sFileName.m_nTransformShortMaxWidth, 100);

	const std::array<std::pair<std::wstring, std::wstring>, 7> expectedPairs = {{
		{ LR"(%DeskTop%\)",           LR"(デスクトップ\)" },
		{ LR"(%Personal%\)",          LR"(マイドキュメント\)" },
		{ LR"(%Cache%\Content.IE5\)", LR"(IEキャッシュ\)" },
		{ LR"(%TEMP%\)",              LR"(TEMP\)" },
		{ LR"(%Common DeskTop%\)",    LR"(共有デスクトップ\)" },
		{ LR"(%Common Documents%\)",  LR"(共有ドキュメント\)" },
		{ LR"(%AppData%\)",           LR"(アプリデータ\)" }
	}};

	EXPECT_THAT(sFileName.m_nTransformFileNameArrNum, expectedPairs.size());

	for (size_t i = 0; i < expectedPairs.size(); ++i) {
		EXPECT_THAT(sFileName.m_szTransformFileNameFrom[i], StrEq(expectedPairs[i].first));
		EXPECT_THAT(sFileName.m_szTransformFileNameTo[i], StrEq(expectedPairs[i].second));
	}

	for (int i = expectedPairs.size(); i < MAX_TRANSFORM_FILENAME; ++i ){
		EXPECT_THAT(sFileName.m_szTransformFileNameFrom[i], StrEq(L""));
		EXPECT_THAT(sFileName.m_szTransformFileNameTo[i], StrEq(L""));
	}

	return true;
}

// [その他]タブ
MATCHER(IsInitializedCommonSettingOthers, "Checks if CommonSetting_Others is properly initialized") {
    const CommonSetting_Others& sOthers = arg;

	EXPECT_THAT(sOthers.m_rcOpenDialog,       RECT{});
	EXPECT_THAT(sOthers.m_rcCompareDialog,    RECT{});
	EXPECT_THAT(sOthers.m_rcDiffDialog,       RECT{});
	EXPECT_THAT(sOthers.m_rcFavoriteDialog,   RECT{});
	EXPECT_THAT(sOthers.m_rcTagJumpDialog,    RECT{});
	EXPECT_THAT(sOthers.m_rcWindowListDialog, RECT{});

	EXPECT_THAT(sOthers.m_bIniReadOnly, IsFalse());

	return true;
}

// [ステータスバー]タブ
MATCHER(IsInitializedCommonSettingStatusbar, "Checks if CommonSetting_Statusbar is properly initialized") {
    const CommonSetting_Statusbar& sStatusbar = arg;

	EXPECT_THAT(sStatusbar.m_bDispUniInSjis, IsFalse());
	EXPECT_THAT(sStatusbar.m_bDispUniInJis, IsFalse());
	EXPECT_THAT(sStatusbar.m_bDispUniInEuc, IsFalse());
	EXPECT_THAT(sStatusbar.m_bDispUtf8Codepoint, IsTrue());
	EXPECT_THAT(sStatusbar.m_bDispSPCodepoint, IsTrue());
	EXPECT_THAT(sStatusbar.m_bDispSelCountByByte, IsFalse());
	EXPECT_THAT(sStatusbar.m_bDispColByChar, IsFalse());

	return true;
}

// [プラグイン]タブ
MATCHER(IsInitializedCommonSettingPlugin, "Checks if CommonSetting_Plugin is properly initialized") {
    const CommonSetting_Plugin& sPlugin = arg;

	EXPECT_THAT(sPlugin.m_bEnablePlugin, IsFalse());

	for (int nPlugin = 0; nPlugin < MAX_PLUGIN; ++nPlugin) {
		EXPECT_THAT(sPlugin.m_PluginTable[nPlugin].m_szName, StrEq(L""));
		EXPECT_THAT(sPlugin.m_PluginTable[nPlugin].m_szId, StrEq(L""));
		EXPECT_THAT(sPlugin.m_PluginTable[nPlugin].m_state, PLS_NONE);
	}

	return true;
}

// [メインメニュー]タブ
MATCHER(IsInitializedCommonSettingMainMenu, "Checks if CommonSetting_MainMenu is properly initialized") {
    const CommonSetting_MainMenu& sMainMenu = arg;

	const std::array<SMenuItem, 336> mainMenuTable = {{
		{ 0, 34052, 'F' },
		{ 1, 30101, 'N' },
		{ 1, 30110, 'M' },
		{ 1, 30102, 'O' },
		{ 1, 30103, 'S' },
		{ 1, 30104, 'A' },
		{ 1, 30120, 'Z' },
		{ 1, 1 },
		{ 1, 30109, 'E' },
		{ 1, 31320, 'C' },
		{ 1, 30105, 'R' },
		{ 1, 30107, 'L' },
		{ 1, 34005, 'W' },
		{ 2, 30119, 'W' },
		{ 2, 1 },
		{ 2, 30111, 'S' },
		{ 2, 30112, 'J' },
		{ 2, 30113, 'E' },
		{ 2, 30122, 'L' },
		{ 2, 30114, 'U' },
		{ 2, 30117, 'N' },
		{ 2, 30115, '8' },
		{ 2, 30118, 'C' },
		{ 2, 30116, '7' },
		{ 1, 1 },
		{ 1, 30150, 'P' },
		{ 1, 30151, 'V' },
		{ 1, 30152, 'U' },
		{ 1, 1 },
		{ 1, 30190, 'T' },
		{ 1, 30180, 'B' },
		{ 1, 1 },
		{ 1, 34006, 'F' },
		{ 2, 29002 },
		{ 1, 34007, 'D' },
		{ 2, 29003 },
		{ 1, 1 },
		{ 1, 31380, 'G' },
		{ 1, 30194, 'Q' },
		{ 1, 30195, 'X' },
		{ 0, 34053, 'E' },
		{ 1, 30210, 'U' },
		{ 1, 30211, 'R' },
		{ 1, 1 },
		{ 1, 30601, 'T' },
		{ 1, 30602, 'C' },
		{ 1, 30604, 'P' },
		{ 1, 30221, 'D' },
		{ 1, 30401, 'A' },
		{ 1, 1 },
		{ 1, 30285, 'R' },
		{ 1, 1 },
		{ 1, 30603, 'L' },
		{ 1, 30608, 'H' },
		{ 1, 30605, 'X' },
		{ 1, 30222, 'B' },
		{ 1, 1 },
		{ 1, 34008, 'I' },
		{ 2, 30790, 'D' },
		{ 2, 30791, 'T' },
		{ 2, 30792, 'C' },
		{ 2, 30794, 'F' },
		{ 2, 30795, 'O' },
		{ 1, 34012, 'V' },
		{ 2, 30230, 'L' },
		{ 2, 30231, 'R' },
		{ 2, 1 },
		{ 2, 30400, 'W' },
		{ 2, 30232, 'T' },
		{ 2, 30233, 'D' },
		{ 2, 1 },
		{ 2, 30240, 'U' },
		{ 2, 30241, 'K' },
		{ 2, 1 },
		{ 2, 30242, 'H' },
		{ 2, 30243, 'E' },
		{ 2, 1 },
		{ 2, 30244, 'X' },
		{ 2, 30245, 'Y' },
		{ 2, 1 },
		{ 2, 30250, '2' },
		{ 2, 1 },
		{ 2, 30260, 'A' },
		{ 2, 30261, 'B' },
		{ 2, 30262, 'S' },
		{ 2, 30263, 'P' },
		{ 2, 1 },
		{ 2, 30610, '@' },
		{ 2, 30611, '.' },
		{ 2, 30612, ':' },
		{ 2, 30613, 'C' },
		{ 2, 30614, 'F' },
		{ 2, 1 },
		{ 2, 30622, '-' },
		{ 2, 30620, '\\' },
		{ 2, 30621, '^' },
		{ 1, 34010, 'O' },
		{ 2, 30315, 'Q' },
		{ 2, 30316, 'K' },
		{ 2, 30320, 'L' },
		{ 2, 30321, 'R' },
		{ 2, 30383, 'A' },
		{ 2, 30382, 'Z' },
		{ 2, 30332, 'H' },
		{ 2, 30333, 'E' },
		{ 2, 1 },
		{ 2, 30342, 'U' },
		{ 2, 30343, 'D' },
		{ 2, 30350, 'T' },
		{ 2, 30351, 'B' },
		{ 2, 1 },
		{ 2, 30360, 'C' },
		{ 2, 1 },
		{ 2, 30920, 'J' },
		{ 2, 30909, 'I' },
		{ 2, 1 },
		{ 2, 30370, 'P' },
		{ 2, 30371, 'N' },
		{ 2, 30372, 'S' },
		{ 2, 1 },
		{ 2, 30988 },
		{ 2, 30989 },
		{ 2, 30393 },
		{ 2, 30394 },
		{ 1, 34011, 'S' },
		{ 2, 30400, 'W' },
		{ 2, 30401, 'A' },
		{ 2, 30410, 'S' },
		{ 2, 1 },
		{ 2, 30415, 'Q' },
		{ 2, 30416, 'K' },
		{ 2, 30420, 'L' },
		{ 2, 30421, 'R' },
		{ 2, 30483, '2' },
		{ 2, 30482, '8' },
		{ 2, 30432, 'H' },
		{ 2, 30433, 'T' },
		{ 2, 1 },
		{ 2, 30442, 'U' },
		{ 2, 30443, 'D' },
		{ 2, 30450, '1' },
		{ 2, 30451, '9' },
		{ 2, 1 },
		{ 2, 30484 },
		{ 2, 30485 },
		{ 1, 34051, 'F' },
		{ 2, 30510, 'S' },
		{ 2, 1 },
		{ 2, 30515, 'Q' },
		{ 2, 30516, 'K' },
		{ 2, 30520, 'L' },
		{ 2, 30521, 'R' },
		{ 2, 30530, 'A' },
		{ 2, 30532, 'H' },
		{ 2, 30533, 'T' },
		{ 2, 1 },
		{ 2, 30542, 'U' },
		{ 2, 30543, 'D' },
		{ 2, 30550, '1' },
		{ 2, 30551, '9' },
		{ 1, 34009, 'K' },
		{ 2, 30280, 'L' },
		{ 2, 30281, 'R' },
		{ 2, 1 },
		{ 2, 30282, 'A' },
		{ 2, 30283, 'D' },
		{ 2, 1 },
		{ 2, 30284, 'U' },
		{ 0, 34054, 'C' },
		{ 1, 30800, 'L' },
		{ 1, 30801, 'U' },
		{ 1, 1 },
		{ 1, 30810, 'F' },
		{ 1, 30811, 'Z' },
		{ 1, 30812, 'N' },
		{ 1, 30816, 'A' },
		{ 1, 30815, 'M' },
		{ 1, 30817, 'J' },
		{ 1, 30813, 'K' },
		{ 1, 30814, 'H' },
		{ 1, 1 },
		{ 1, 30830, 'S' },
		{ 1, 30831, 'T' },
		{ 1, 34013, 'C' },
		{ 2, 30850, 'A' },
		{ 2, 30851, 'M' },
		{ 2, 30852, 'W' },
		{ 2, 30853, 'U' },
		{ 2, 30856, 'N' },
		{ 2, 30854, 'T' },
		{ 2, 30855, 'F' },
		{ 2, 1 },
		{ 2, 30860, 'J' },
		{ 2, 30861, 'E' },
		{ 2, 30862, '8' },
		{ 2, 30863, '7' },
		{ 2, 1 },
		{ 2, 30870, 'B' },
		{ 2, 30880, 'D' },
		{ 0, 34055, 'S' },
		{ 1, 30901, 'F' },
		{ 1, 30902, 'N' },
		{ 1, 30903, 'P' },
		{ 1, 30904, 'R' },
		{ 1, 30905, 'C' },
		{ 1, 30909, 'I' },
		{ 1, 34048, 'S' },
		{ 2, 30981, 'F' },
		{ 2, 30982, 'B' },
		{ 2, 30983, 'R' },
		{ 2, 30984, 'X' },
		{ 2, 30985, 'M' },
		{ 2, 30986, 'N' },
		{ 1, 1 },
		{ 1, 34047, 'M' },
		{ 2, 30970, 'S' },
		{ 2, 30971, 'A' },
		{ 2, 30972, 'Z' },
		{ 2, 30973, 'X' },
		{ 2, 30974, 'V' },
		{ 1, 30910, 'G' },
		{ 1, 30912 },
		{ 1, 30920, 'J' },
		{ 1, 30930, 'L' },
		{ 1, 30990, 'E' },
		{ 1, 30940, 'T' },
		{ 1, 30941, 'B' },
		{ 1, 30943 },
		{ 1, 30944 },
		{ 1, 30946 },
		{ 1, 30162, 'C' },
		{ 1, 1 },
		{ 1, 30950, '@' },
		{ 1, 30976, 'D' },
		{ 1, 30978 },
		{ 1, 30979 },
		{ 1, 30980 },
		{ 1, 1 },
		{ 1, 30960, '[' },
		{ 0, 34056, 'T' },
		{ 1, 31250, 'R' },
		{ 1, 31251, 'M' },
		{ 1, 31252, 'A' },
		{ 1, 31253, 'D' },
		{ 1, 34022, 'B' },
		{ 2, 29005 },
		{ 1, 31254, 'E' },
		{ 1, 1 },
		{ 1, 31270, 'X' },
		{ 1, 30170, 'P' },
		{ 1, 30171, 'S' },
		{ 1, 1 },
		{ 1, 31430, '/' },
		{ 1, 1 },
		{ 1, 29006 },
		{ 1, 1 },
		{ 1, 34023, 'U' },
		{ 2, 29004 },
		{ 0, 34057, 'O' },
		{ 1, 31100, 'T' },
		{ 1, 31101, 'K' },
		{ 1, 31103, 'M' },
		{ 1, 31102, 'S' },
		{ 1, 31104, 'N' },
		{ 1, 1 },
		{ 1, 31110, 'L' },
		{ 1, 31111, 'Y' },
		{ 1, 31112, 'C' },
		{ 1, 31120, 'F' },
		{ 1, 31113, 'O' },
		{ 1, 1 },
		{ 1, 32805, 'X' },
		{ 2, 31141, 'X' },
		{ 2, 31142, 'S' },
		{ 2, 31143, 'W' },
		{ 1, 31140, 'W' },
		{ 1, 31144, 'B' },
		{ 1, 1 },
		{ 1, 31001, 'I' },
		{ 1, 30185, 'R' },
		{ 1, 31431, 'H' },
		{ 1, 31010, 'A' },
		{ 1, 34044, 'E' },
		{ 2, 31081, 'C' },
		{ 2, 31082, 'L' },
		{ 2, 31083, 'R' },
		{ 0, 34058, 'W' },
		{ 1, 31310, '-' },
		{ 1, 31311, 'I' },
		{ 1, 31312, 'S' },
		{ 1, 1 },
		{ 1, 31320, 'C' },
		{ 1, 31321, 'Q' },
		{ 1, 31388, 'O' },
		{ 1, 31340, 'N' },
		{ 1, 31341, 'P' },
		{ 1, 31336, 'W' },
		{ 1, 1 },
		{ 1, 31330, 'E' },
		{ 1, 31331, 'H' },
		{ 1, 31332, 'T' },
		{ 1, 31334, 'F' },
		{ 1, 34000, 'B' },
		{ 2, 31333, 'B' },
		{ 2, 31380, 'G' },
		{ 2, 31389, 'H' },
		{ 2, 31390, 'M' },
		{ 2, 31381, 'N' },
		{ 2, 31382, 'P' },
		{ 2, 31383, 'R' },
		{ 2, 31384, 'L' },
		{ 2, 31385, 'E' },
		{ 2, 31386, 'X' },
		{ 2, 31387, 'V' },
		{ 1, 1 },
		{ 1, 31350, 'V' },
		{ 1, 31352, 'Y' },
		{ 1, 31351, 'M' },
		{ 1, 1 },
		{ 1, 31360, 'R' },
		{ 1, 1 },
		{ 1, 31370, 'U' },
		{ 1, 31337, 'D' },
		{ 1, 1 },
		{ 1, 34060, 'A' },
		{ 2, 29001 },
		{ 0, 34059, 'H' },
		{ 1, 31440, 'O' },
		{ 1, 31441, 'S' },
		{ 1, 1 },
		{ 1, 31445, 'M' },
		{ 1, 30630, 'Q' },
		{ 1, 31450, 'E' },
		{ 1, 31451, 'H' },
		{ 1, 1 },
		{ 1, 31455, 'A' },
	}};

	EXPECT_THAT(sMainMenu.m_nVersion, 0);
	EXPECT_THAT(sMainMenu.m_nMainMenuNum, std::size(mainMenuTable));
	for (size_t i = 0; i < std::size(mainMenuTable); ++i) {
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_nLevel,  mainMenuTable[i].m_nLevel);
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_nType,   mainMenuTable[i].GetType());
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_nFunc,   mainMenuTable[i].m_eFuncCode);
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_sKey[0], mainMenuTable[i].m_chAccessKey);
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_sName,   StrEq(L""));
	}
	for (size_t i = std::size(mainMenuTable); i < std::size(sMainMenu.m_cMainMenuTbl); ++i) {
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_nLevel,  0);
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_nType,   T_NODE);
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_nFunc,   F_0);
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_sKey[0], '\0');
		EXPECT_THAT(sMainMenu.m_cMainMenuTbl[i].m_sName,   StrEq(L""));
	}

	const std::array<int, 8> mainMenuTopIdxs = {{
		0,
		40,
		168,
		199,
		239,
		258,
		286,
		326,
	}};

	for (size_t i = 0; i < std::size(mainMenuTopIdxs); ++i) {
		EXPECT_THAT(sMainMenu.m_nMenuTopIdx[i], mainMenuTopIdxs[i]) << L"Unexpected value at index " << i;
	}
	for (size_t i = std::size(mainMenuTopIdxs); i < std::size(sMainMenu.m_nMenuTopIdx); ++i) {
		EXPECT_THAT(sMainMenu.m_nMenuTopIdx[i], -1);
	}

	EXPECT_THAT(sMainMenu.m_bMainMenuKeyParentheses, IsTrue());

	return true;
}

// CommonSetting型のマッチャー
MATCHER_P(IsInitializedCommonSetting, iniFolder, "Checks if CommonSetting is properly initialized") {
    const CommonSetting& commonSetting = arg;

	LOGFONT lf{};
	lf.lfHeight			= DpiPointsToPixels(-10);
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;

	StringBufferW(lf.lfFaceName) = L"ＭＳ ゴシック";

	LOGFONT lfIconTitle{};
	::SystemParametersInfoW(
		SPI_GETICONTITLELOGFONT,
		sizeof(LOGFONT),
		&lfIconTitle,
		0
	);
	INT nIconPointSize = lfIconTitle.lfHeight >= 0 ? lfIconTitle.lfHeight : DpiPixelsToPoints(-lfIconTitle.lfHeight, 10);

	// CommonSettingのプロパティを検証
    return true
		&& ExplainMatchResult(IsInitializedCommonSettingGeneral(), commonSetting.m_sGeneral, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingWindow(),  commonSetting.m_sWindow, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingTabBar(lfIconTitle, nIconPointSize),  commonSetting.m_sTabBar, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingEdit(),  commonSetting.m_sEdit, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingFile(),  commonSetting.m_sFile, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingBackup(),  commonSetting.m_sBackup, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingFormat(),  commonSetting.m_sFormat, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingSearch(),  commonSetting.m_sSearch, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingKeyBind(),  commonSetting.m_sKeyBind, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingCustomMenu(),  commonSetting.m_sCustomMenu, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingToolBar(),  commonSetting.m_sToolBar, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingSpecialKeyword(),  commonSetting.m_sSpecialKeyword, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingHelper(lfIconTitle, nIconPointSize),  commonSetting.m_sHelper, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingOutLine(),  commonSetting.m_sOutline, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingCompare(),  commonSetting.m_sCompare, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingView(lf),  commonSetting.m_sView, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingMacro(iniFolder),  commonSetting.m_sMacro, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingFileName(),  commonSetting.m_sFileName, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingOthers(),  commonSetting.m_sOthers, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingStatusbar(),  commonSetting.m_sStatusbar, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingPlugin(),  commonSetting.m_sPlugin, result_listener)
		&& ExplainMatchResult(IsInitializedCommonSettingMainMenu(),  commonSetting.m_sMainMenu, result_listener)
		&& true;
}

// 独自マッチャー: DLLSHAREDATAの初期化を確認
MATCHER_P4(IsInitializedShareData, pszProfileName, isMultiUserSettings, userRootFolder, userSubFolder, "Checks if DLLSHAREDATA is properly initialized") {
    const DLLSHAREDATA& shareData = arg;

	// exe基準のiniファイルパスを得る
	const auto iniPath = GetExeFileName().replace_extension(L".ini");

	// 設定ファイルフォルダー
	auto iniFolder = iniPath;
	iniFolder.remove_filename();

	// iniファイル名を得る
	const auto filename = iniPath.filename();

	// マルチユーザー用のiniファイルパスを組み立てる
	auto privateIniPath = CShareData::BuildPrivateIniFileName(iniFolder, isMultiUserSettings, userRootFolder, userSubFolder, pszProfileName, filename);

	EXPECT_THAT(shareData.m_vStructureVersion, N_SHAREDATA_VERSION);
	EXPECT_THAT(shareData.m_nSize, sizeof(DLLSHAREDATA));

	EXPECT_THAT(shareData.m_sVersion.m_dwProductVersionMS, MAKELONG(VER_B, VER_A));
	EXPECT_THAT(shareData.m_sVersion.m_dwProductVersionLS, MAKELONG(VER_D, VER_C));

	// SShare_WorkBuffer m_sWorkBuffer (初期化漏れ)

	EXPECT_THAT(shareData.m_sFlags.m_bEditWndChanging, IsFalse());
	EXPECT_THAT(shareData.m_sFlags.m_bRecordingKeyMacro, IsFalse());
	EXPECT_THAT(shareData.m_sFlags.m_hwndRecordingKeyMacro, nullptr);

	EXPECT_THAT(shareData.m_sNodes.m_nSequences, 0);
	EXPECT_THAT(shareData.m_sNodes.m_nNonameSequences, 0);
	EXPECT_THAT(shareData.m_sNodes.m_nGroupSequences, 0);
	EXPECT_THAT(shareData.m_sNodes.m_nEditArrNum, 0);

	EXPECT_THAT(shareData.m_sHandles.m_hwndTray, nullptr);
	EXPECT_THAT(shareData.m_sHandles.m_hwndDebug, nullptr);

	EXPECT_THAT(shareData.m_sCharWidth.m_lfFaceName.data(), StrEq(L""));
	EXPECT_THAT(shareData.m_sCharWidth.m_lfFaceName2.data(), StrEq(L""));
	for (int i = 0; i < std::size(shareData.m_sCharWidth.m_nCharPxWidthCache); ++i) {
		EXPECT_THAT(shareData.m_sCharWidth.m_nCharPxWidthCache[i], 0) << L"Unexpected value at index " << i;
	}
	EXPECT_THAT(shareData.m_sCharWidth.m_nCharWidthCacheTest, 0);

	for (int i = 0; i < std::size(shareData.m_dwCustColors); ++i) {
		EXPECT_THAT(shareData.m_dwCustColors[i], RGB(255, 255, 255)) << L"Unexpected value at index " << i;
	}

	EXPECT_THAT(shareData.m_szPrivateIniFile, StrEq(privateIniPath));
	EXPECT_THAT(shareData.m_szIniFile, StrEq(iniPath));

	for (int i = 0; i < std::size(shareData.m_PlugCmdIcon); ++i) {
		EXPECT_THAT(shareData.m_PlugCmdIcon[i], 0) << L"Unexpected value at index " << i;
	}

	EXPECT_THAT(shareData.m_maxTBNum, 0);

	//InitTypeConfigs
	const std::array<std::unique_ptr<CType>, 17> table = {{
		std::make_unique<CType_Basis>(),
		std::make_unique<CType_Text>(),
		std::make_unique<CType_Cpp>(),
		std::make_unique<CType_Html>(),
		std::make_unique<CType_Sql>(),
		std::make_unique<CType_Cobol>(),
		std::make_unique<CType_Java>(),
		std::make_unique<CType_Asm>(),
		std::make_unique<CType_Awk>(),
		std::make_unique<CType_Dos>(),
		std::make_unique<CType_Pascal>(),
		std::make_unique<CType_Tex>(),
		std::make_unique<CType_Perl>(),
		std::make_unique<CType_Python>(),
		std::make_unique<CType_Vb>(),
		std::make_unique<CType_Rich>(),
		std::make_unique<CType_Ini>()
	}};

	EXPECT_THAT(shareData.m_nTypesCount, std::size(table));

	if (size_t i = 0; true) {
		auto type = std::make_unique<STypeConfig>();
		table[i]->InitTypeConfig(i, *type);
		EXPECT_THAT(shareData.m_TypeMini[i].m_id, type->m_id);
		EXPECT_THAT(shareData.m_TypeMini[i].m_szTypeName, StrEq(type->m_szTypeName));
		EXPECT_THAT(shareData.m_TypeMini[i].m_szTypeExts, StrEq(type->m_szTypeExts));
		EXPECT_THAT(shareData.m_TypeMini[i].m_encoding, type->m_encoding);

		EXPECT_THAT(shareData.m_TypeBasis.m_nIdx, type->m_nIdx);
		EXPECT_THAT(shareData.m_TypeBasis, EqSTypeConfig(*type));
	}

	for (size_t i = 1; i < std::size(table); ++i) {
		auto type = std::make_unique<STypeConfig>();
		table[i]->InitTypeConfig(i, *type);
		EXPECT_THAT(shareData.m_TypeMini[i].m_id, type->m_id);
		EXPECT_THAT(shareData.m_TypeMini[i].m_szTypeName, StrEq(type->m_szTypeName));
		EXPECT_THAT(shareData.m_TypeMini[i].m_szTypeExts, StrEq(type->m_szTypeExts));
		EXPECT_THAT(shareData.m_TypeMini[i].m_encoding, type->m_encoding);
	}

	for (int i = 0; i < std::size(shareData.m_PrintSettingArr); ++i) {
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_szPrintSettingName, StrEq(strprintf(L"印刷設定 %d", i + 1))) << L"Unexpected value at index " << i;
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_szPrintFontFaceHan, StrEq(L"ＭＳ 明朝"));
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_szPrintFontFaceZen, StrEq(L"ＭＳ 明朝"));
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bColorPrint, IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintFontWidth, 12);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintFontHeight, shareData.m_PrintSettingArr[i].m_nPrintFontWidth * 2);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintDansuu, 1);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintDanSpace, 70);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bPrintWordWrap, IsTrue());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bPrintKinsokuHead, IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bPrintKinsokuTail, IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bPrintKinsokuRet, IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bPrintKinsokuKuto, IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bPrintLineNumber, IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintLineSpacing, 30);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintMarginTY, 100);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintMarginBY, 200);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintMarginLX, 200);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintMarginRX, 100);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintPaperOrientation, DMORIENT_PORTRAIT);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_nPrintPaperSize, DMPAPER_A4);
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bHeaderUse[0], IsTrue());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bHeaderUse[1], IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bHeaderUse[2], IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_szHeaderForm[0], StrEq(L"$f"));
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_szHeaderForm[1], StrEq(L""));
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_szHeaderForm[2], StrEq(L""));
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bFooterUse[0], IsTrue());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bFooterUse[1], IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_bFooterUse[2], IsFalse());
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_szFooterForm[0], StrEq(L""));
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_szFooterForm[1], StrEq(L"- $p -"));
		EXPECT_THAT(shareData.m_PrintSettingArr[i].m_szFooterForm[2], StrEq(L""));
	}

	EXPECT_THAT(shareData.m_nLockCount, 0);

	EXPECT_THAT(shareData.m_sSearchKeywords.m_aSearchKeys.size(), 0);
	EXPECT_THAT(shareData.m_sSearchKeywords.m_aReplaceKeys.size(), 0);
	EXPECT_THAT(shareData.m_sSearchKeywords.m_aGrepFiles.size(), 1);
	EXPECT_THAT(shareData.m_sSearchKeywords.m_aGrepFiles[0], StrEq(L"*.*"));
	EXPECT_THAT(shareData.m_sSearchKeywords.m_aGrepFolders.size(), 0);

	EXPECT_THAT(shareData.m_sTagJump.m_TagJumpNum, 0);
	EXPECT_THAT(shareData.m_sTagJump.m_TagJumpTop, 0);
	EXPECT_THAT(shareData.m_sTagJump.m_aTagJumpKeywords.size(), 0);
	EXPECT_THAT(shareData.m_sTagJump.m_bTagJumpICase, IsFalse());
	EXPECT_THAT(shareData.m_sTagJump.m_bTagJumpPartialMatch, IsFalse());

	EXPECT_THAT(shareData.m_sHistory.m_aExceptMRU.size(), 0);
	EXPECT_THAT(shareData.m_sHistory.m_szIMPORTFOLDER, StrEq(iniFolder));
	EXPECT_THAT(shareData.m_sHistory.m_aCommands.size(), 0);
	EXPECT_THAT(shareData.m_sHistory.m_aCurDirs.size(), 0);

	EXPECT_THAT(shareData.m_nExecFlgOpt, 1);
	EXPECT_THAT(shareData.m_nDiffFlgOpt, 0);
	EXPECT_THAT(shareData.m_szTagsCmdLine, StrEq(L""));
	EXPECT_THAT(shareData.m_nTagsOpt, 0);
	EXPECT_THAT(shareData.m_bLineNumIsCRLF_ForJump, IsTrue());

	// 期待するプロパティの値を検証
	return shareData.m_vStructureVersion == N_SHAREDATA_VERSION
		&& shareData.m_nSize == sizeof(DLLSHAREDATA)
		&& ExplainMatchResult(IsInitializedCommonSetting(iniFolder), shareData.m_Common, result_listener)
		&& true;
}

/*!
 * @brief CShareDataのテスト
 */
TEST(CShareData, init001)
{
	// 共有メモリのインスタンスを生成する
	auto shareData = std::make_unique<CShareData>();

	// 共有メモリのインスタンスを破棄する
	shareData.reset();
}

/*!
 * @brief CShareDataのテスト
 */
TEST(CShareData, InitShareData001)
{
	const auto& pszProfileName = L"";
	const auto isMultiUserSettings = false;
	const auto userRootFolder = 0;
	const auto& userSubFolder = L"sakura";

	// 共有メモリのインスタンスを生成する
	const auto pShareData = std::make_unique<CShareData>();

	// 共有メモリのインスタンスを初期化する
	EXPECT_TRUE(pShareData->InitShareData());

	const auto& shareData = ::GetDllShareData();
	EXPECT_THAT(shareData, IsInitializedShareData(pszProfileName, isMultiUserSettings, userRootFolder, userSubFolder));
}

/*!
 * @brief CShareDataのテスト
 */
TEST(CShareData, ConvertLangValues)
{
	const auto& profileName = L"";
	const auto isMultiUserSettings = false;
	const auto userRootFolder = 0;
	const auto& userSubFolder = L"sakura";

	// 共有メモリのインスタンスを生成する
	const auto pShareData = std::make_unique<CShareData>();

	// 共有メモリのインスタンスを初期化する
	ASSERT_TRUE(pShareData->InitShareData());

	const auto& shareData = ::GetDllShareData();
	EXPECT_THAT(shareData, IsInitializedShareData(profileName, isMultiUserSettings, userRootFolder, userSubFolder));

	// 言語切り替えのテストを実施する
	std::vector<std::wstring> values;
	pShareData->ConvertLangValues(values, true);
	CSelectLang::ChangeLang(L"sakura_lang_en_US.dll");
	pShareData->ConvertLangValues(values, false);
	pShareData->RefreshString();
}

} // namespace share_data
