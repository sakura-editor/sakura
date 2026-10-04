/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "env/CDataProfile.h"

#include "env/CShareData_IO.h"
#include "env/DLLSHAREDATA.h"
#include "uiparts/CMenuDrawer.h"

#include <Shlwapi.h>

#include <cstdlib>
#include <fstream>

#include "util/file.h"
#include "view/colors/EColorIndexType.h"

#include "env/ShareDataTestSuite.hpp"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

extern template
void ShareData_IO_KeyHelpArr<KeyHelpInfo(&)[MAX_KEYHELP_FILE]>(
	CDataProfile&			cProfile,
	std::wstring_view		sectionName,	//!< [in] セクション名
	KeyHelpInfo				(&KeyHelpArr)[MAX_KEYHELP_FILE],
	int&					nKeyHelpNum
);

extern template
void ShareData_IO_OutlineDockRect<CommonSetting_OutLine>(
	CDataProfile&			cProfile,
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		entryKey,		//!< [in] エントリ名
	CommonSetting_OutLine&	tEntryValue		//!< [in,out] エントリ値
);

extern template
void ShareData_IO_VertLineIdx<CKetaXInt(&)[MAX_VERTLINES]>(
	CDataProfile&		cProfile,
	std::wstring_view	sectionName,	//!< [in] セクション名
	CKetaXInt			(&nVertLineIdx)[MAX_VERTLINES]
);

/*!
 * @brief 内部バッファが溢れたら拡張する
 */
TEST(CProfile, ReadProfile_ExpandLineBuffer)
{
	std::filesystem::path iniPath{ "test.ini" };

	std::error_code ec;
	std::filesystem::remove(iniPath, ec);

	// ファイル出力ストリームをバイナリモードで開く
	std::ofstream os(iniPath, std::ios::binary);

	std::vector<std::string> lines{
		"; test"s,
		"[test]"s,
		"test=1"s,
	};

	constexpr auto& prefix = "too_long_item=";
	auto tooLongLine = std::format("{}{:x<4083}", prefix, 'x');	// 4096文字を超える行を作る
	lines.emplace_back(tooLongLine);

	// 各行を書き込む
	for (const auto& line : lines) {
		if (!line.empty()) {
			os.write(LPCSTR(std::data(line)), std::size(line));
		}
		os << '\r';
	}

	os.close();

	CDataProfile cProfile;
	cProfile.ReadProfile(iniPath);

	bool value = false;
	EXPECT_TRUE(cProfile.IOProfileData(L"test", L"test", value));
	EXPECT_THAT(value, IsTrue());

	std::filesystem::remove(iniPath, ec);
}

/*!
 * @brief 設定ファイルの改行コードがCR
 */
TEST(CProfile, ReadProfile_LineTerminatorCr)
{
	std::filesystem::path iniPath{ "test.ini" };

	std::error_code ec;
	std::filesystem::remove(iniPath, ec);

	// ファイル出力ストリームをバイナリモードで開く
	std::ofstream os(iniPath, std::ios::binary);

	const std::array lines{
		u8"; test"sv,
		u8"[test]"sv,
		u8"test=1"sv,
	};

	// 各行を書き込む
	for (const auto& line : lines) {
		if (!line.empty()) {
			os.write(LPCSTR(std::data(line)), std::size(line));
		}
		os << '\r';
	}

	os.close();

	CDataProfile cProfile;
	cProfile.ReadProfile(iniPath);

	bool value = false;
	EXPECT_TRUE(cProfile.IOProfileData(L"test", L"test", value));
	EXPECT_THAT(value, IsTrue());

	std::filesystem::remove(iniPath, ec);
}

/*!
 * @brief WriteProfileは指定されたパスに含まれるサブディレクトリを作成する
 */
TEST( CProfile, WriteProfileMakesSubDirectories )
{
	// サブディレクトリを含むパスを作成する
	WCHAR szIniName[_MAX_PATH]{ 0 };
	::_wfullpath( szIniName, L"test1\\test2\\test.ini", int(std::size(szIniName)) );

	// プロファイルを書き出す
	CProfile cProfile;
	cProfile.WriteProfile( szIniName, L"WriteProfileのテスト" );

	ASSERT_TRUE( fexist( szIniName ) );

	WCHAR* p;

	std::error_code ec;

	// ファイルを削除
	std::filesystem::remove( szIniName, ec );

	// フォルダーを削除
	p = ::PathFindFileNameW( szIniName );
	p[0] = L'\0';
	std::filesystem::remove( szIniName, ec );

	// フォルダーを削除
	p = ::PathFindFileNameW( szIniName );
	p[0] = L'\0';
	std::filesystem::remove( szIniName, ec );
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NoSection)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	// 初期状態は空なのでセクションが見つからない
	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"存在しないセクション名", L"szTest", value));
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NewSection)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"szTest", value));

	// セクションを追加
	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	// 追加されたセクションを取得
	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NoEntry)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"szTest", value));

	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());

	// 該当するキーがないのでエントリを取得できない
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"nTest", value));
}

/*!
 * @brief GetProfileDataのテスト
 */
TEST(CProfile, GetProfileData_NewEntry)
{
	CProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"szTest", value));

	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"szTest", value));
	ASSERT_STREQ(L"value", value.data());

	ASSERT_FALSE(cProfile.GetProfileData(L"Test", L"nTest", value));

	cProfile.SetProfileData(L"Test", L"nTest", L"109");

	ASSERT_TRUE(cProfile.GetProfileData(L"Test", L"nTest", value));
	ASSERT_STREQ(L"109", value.data());
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_int)
{
	int value = 0;
	ASSERT_TRUE(profile_data::TryParse(L"109", value));
	ASSERT_EQ(109, value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_EQ(109, value);

	ASSERT_FALSE(profile_data::TryParse(L"not a number", value));
	ASSERT_EQ(109, value);

	ASSERT_TRUE(profile_data::TryParse(L"888", value));
	ASSERT_EQ(888, value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_CLayoutInt)
{
	CLayoutInt value{ 0 };
	ASSERT_TRUE(profile_data::TryParse(L"109", value));
	ASSERT_EQ(109, value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_EQ(109, value);

	ASSERT_FALSE(profile_data::TryParse(L"not a number", value));
	ASSERT_EQ(109, value);

	ASSERT_TRUE(profile_data::TryParse(L"888", value));
	ASSERT_EQ(888, value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_enum)
{
	enum ETest { NONE, RED, GREEN, BLUE, };
	ETest value = NONE;
	ASSERT_TRUE(profile_data::TryParse(L"1", value));
	ASSERT_EQ(RED, value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_EQ(RED, value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_bool)
{
	bool value = false;
	ASSERT_TRUE(profile_data::TryParse(L"1", value));
	ASSERT_TRUE(value);

	ASSERT_FALSE(profile_data::TryParse(L"", value));
	ASSERT_TRUE(value);

	ASSERT_FALSE(profile_data::TryParse(L"false", value));
	ASSERT_TRUE(value);

	ASSERT_TRUE(profile_data::TryParse(L"0", value));
	ASSERT_FALSE(value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_WCHAR)
{
	WCHAR value = L'\0';
	ASSERT_TRUE(profile_data::TryParse(L"|", value));
	ASSERT_EQ(L'|', value);

	ASSERT_FALSE(profile_data::TryParse(L"\U0001F51E", value));
	ASSERT_EQ(L'|', value);

	ASSERT_TRUE(profile_data::TryParse(L"", value));
	ASSERT_EQ(L'\0', value);
}

/*!
 * @brief TryParseのテスト
 */
TEST(profile_data, TryParse_KEYCODE)
{
	KEYCODE value = '\0';
	ASSERT_TRUE(profile_data::TryParse(L"W", value));
	ASSERT_EQ('W', value);

	ASSERT_FALSE(profile_data::TryParse(L"漢", value));
	ASSERT_EQ('W', value);

	ASSERT_TRUE(profile_data::TryParse(L"", value));
	ASSERT_EQ('\0', value);
}

/*!
 * @brief ToString(KEYCODE)のテスト
 */
TEST(profile_data, ToString_KEYCODE)
{
	std::wstring str;
	KEYCODE code;

	code = 0x80;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"", str.c_str());

	code = -1;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"", str.c_str());

	code = 0;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"", str.c_str());

	code = 1;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"\x01", str.c_str());

	code = 0x61;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"a", str.c_str());

	code = 0x7f;
	str = profile_data::ToString(code);
	ASSERT_STREQ(L"\x7f", str.c_str());
}

/*!
 * @brief IOProfileDataのテスト
 */
TEST(CDataProfile, IOProfileData)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	std::wstring value;
	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"szTest", value), IsFalse());

	cProfile.SetProfileData(L"Test", L"szTest", L"value");

	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"szTest", value), IsTrue());
	EXPECT_THAT(value, StrEq(L"value"));

	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"nTest", value), IsFalse());

	cProfile.SetProfileData(L"Test", L"nTest", L"109");

	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"nTest", value), IsTrue());
	EXPECT_THAT(value, StrEq(L"109"));

	int nValue = 0;
	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"nTest", nValue), IsTrue());
	EXPECT_THAT(nValue, 109);

	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"szTest", nValue), IsFalse());
	EXPECT_THAT(nValue, 109);
}

/*!
 * @brief IOProfileDataのテスト
 */
TEST(CDataProfile, IOProfileData_ColorInfo)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	ColorInfo value{};

	// 設定項目がないと読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"clrTest", value));

	// 値は変更されない
	EXPECT_THAT(value.m_bDisp, IsFalse());

	// 不適切な値を設定（足りない）
	cProfile.SetProfileData(L"Test", L"clrTest", L"1,1,ffffff,000000");

	// 不完全な設定項目は読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"clrTest", value));

	// 値を設定
	cProfile.SetProfileData(L"Test", L"clrTest", L"1,1,ffffff,000000,1");

	// 読める
	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"clrTest", value), IsTrue());

	// 値は変更される
	EXPECT_THAT(value.m_bDisp, IsTrue());
	EXPECT_THAT(value.m_sFontAttr.m_bBoldFont, IsTrue());
	EXPECT_THAT(value.m_sColorAttr.m_cTEXT, Eq(0xFFFFFF));
	EXPECT_THAT(value.m_sColorAttr.m_cBACK, Eq(0));
	EXPECT_THAT(value.m_sFontAttr.m_bUnderLine, IsTrue());
}

/*!
 * @brief IOProfileDataのテスト
 */
TEST(CDataProfile, IOProfileData_EditInfo)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	EditInfo value{};

	// 設定項目がなくても読める
	EXPECT_TRUE(cProfile.IOProfileData(L"Test", L"eiTest", value));
}

/*!
 * @brief IOProfileDataのテスト
 */
TEST(CDataProfile, IOProfileData_KeyHelpInfo)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	KeyHelpInfo value{};
	value.m_bUse	= false;
	value.m_szAbout	= L"about";
	value.m_szPath	= L"path";

	// 設定項目がないと読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"keyHelp", value));

	// 値は変更されない
	EXPECT_THAT(value.m_bUse, IsFalse());
	EXPECT_THAT(value.m_szAbout, StrEq(L"about"));
	EXPECT_THAT(value.m_szPath, StrEq(L"path"));

	// 不適切な値を設定
	cProfile.SetProfileData(L"Test", L"keyHelp", std::format(L"{},{},{}", L"true", L"ABOUT", L"PATH"));

	// 値が不適切なので読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"keyHelp", value));

	// 値は変更されない
	EXPECT_THAT(value.m_bUse, IsFalse());
	EXPECT_THAT(value.m_szAbout, StrEq(L"about"));
	EXPECT_THAT(value.m_szPath, StrEq(L"path"));

	// 不適切な値を設定
	cProfile.SetProfileData(L"Test", L"keyHelp", std::format(L"{},{},{}", 1, std::wstring(50, L'a'), L"PATH"));

	// 値が不適切なので読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"keyHelp", value));

	// 値は変更されない
	EXPECT_THAT(value.m_bUse, IsFalse());
	EXPECT_THAT(value.m_szAbout, StrEq(L"about"));
	EXPECT_THAT(value.m_szPath, StrEq(L"path"));

	// 不適切な値を設定
	cProfile.SetProfileData(L"Test", L"keyHelp", std::format(L"{},{},{}", 1, std::wstring(50 - 1, L'a'), std::wstring(_MAX_PATH, L'b')));

	// 値が不適切なので読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"keyHelp", value));

	// 値は変更されない
	EXPECT_THAT(value.m_bUse, IsFalse());
	EXPECT_THAT(value.m_szAbout, StrEq(L"about"));
	EXPECT_THAT(value.m_szPath, StrEq(L"path"));

	// 適切（？）な値を設定
	cProfile.SetProfileData(L"Test", L"keyHelp", std::format(L"{},{},{}", 1, std::wstring(50 - 1, L'a'), std::wstring(_MAX_PATH - 1, L'b')));

	// 読める
	EXPECT_TRUE(cProfile.IOProfileData(L"Test", L"keyHelp", value));

	// 値は変更される
	EXPECT_THAT(value.m_bUse, IsTrue());
	EXPECT_THAT(value.m_szAbout, StrEq(std::wstring(50 - 1, L'a')));
	EXPECT_THAT(value.m_szPath, StrEq(std::wstring(_MAX_PATH - 1, L'b')));

	// 書き込みモード
	cProfile.SetWritingMode();

	// パス未設定なら書き込み失敗とする仕様の確認
	value.m_szPath = L"";
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"keyHelp", value));

	// 出力フォーマットの確認
	value.m_bUse	= true;
	value.m_szAbout	= L"ABOUT";
	value.m_szPath	= L"PATH";

	EXPECT_TRUE(cProfile.IOProfileData(L"Test", L"keyHelp", value));

	std::wstring written;
	cProfile.GetProfileData(L"Test", L"keyHelp", written);
	EXPECT_THAT(written, StrEq(L"1,ABOUT,PATH"));
}

/*!
 * @brief IOProfileDataのテスト
 */
TEST(CDataProfile, IOProfileData_MacroRec)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	MacroRec value{};
	value.m_szName[0]			= L'\0';
	value.m_szFile[0]			= L'\0';
	value.m_bReloadWhenExecute	= false;

	// 設定項目がないと読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"[000]", value));

	// 値は変更されない
	EXPECT_THAT(value.m_szName, StrEq(L""));
	EXPECT_THAT(value.m_szFile, StrEq(L""));
	EXPECT_THAT(value.m_bReloadWhenExecute, IsFalse());

	// 不適切な値を設定
	cProfile.SetProfileData(L"Test", L"Name[000]", std::wstring(MACRONAME_MAX, L'a'));

	// 値が不適切なので読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"[000]", value));

	// 値は変更されない
	EXPECT_THAT(value.m_szName, StrEq(L""));
	EXPECT_THAT(value.m_szFile, StrEq(L""));
	EXPECT_THAT(value.m_bReloadWhenExecute, IsFalse());

	// 不適切な値を設定
	cProfile.SetProfileData(L"Test", L"Name[000]", std::wstring(MACRONAME_MAX - 1, L'a'));
	cProfile.SetProfileData(L"Test", L"File[000]", std::wstring(_MAX_PATH + 1, L'b'));

	// 値が不適切なので読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"[000]", value));

	// 値は変更されない
	EXPECT_THAT(value.m_szName, StrEq(L""));
	EXPECT_THAT(value.m_szFile, StrEq(L""));
	EXPECT_THAT(value.m_bReloadWhenExecute, IsFalse());

	// 適切（？）な値を設定
	cProfile.SetProfileData(L"Test", L"Name[000]", std::wstring(MACRONAME_MAX - 1, L'a'));
	cProfile.SetProfileData(L"Test", L"File[000]", std::wstring(_MAX_PATH, L'b'));
	cProfile.SetProfileData(L"Test", L"ReloadWhenExecute[000]", L"1");

	// 読める
	EXPECT_TRUE(cProfile.IOProfileData(L"Test", L"[000]", value));

	// 値は変更される
	EXPECT_THAT(value.m_szName, StrEq(std::wstring(MACRONAME_MAX - 1, L'a')));
	EXPECT_THAT(value.m_szFile, StrEq(std::wstring(_MAX_PATH, L'b')));
	EXPECT_THAT(value.m_bReloadWhenExecute, IsTrue());
}

/*!
 * @brief IOProfileDataのテスト
 */
TEST(CDataProfile, IOProfileData_PluginRec)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	PluginRec value{};
	value.m_szId[0] = L'\0';
	value.m_szName[0] = L'\0';
	value.m_state = EPluginState::PLS_NONE;
	value.m_nCmdNum = 0;

	// 設定項目がないと読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"pluginTest", value));

	// 値は変更されない
	EXPECT_THAT(value.m_nCmdNum, 0);

	// 不適切な値を設定（サイズオーバー1）
	cProfile.SetProfileData(L"Test", L"pluginTest.Name", std::wstring(size_t(MAX_PLUGIN_ID), L'a'));

	// 不完全な設定項目は読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"pluginTest", value));

	cProfile.SetProfileData(L"Test", L"pluginTest.Name", std::wstring(size_t(MAX_PLUGIN_ID) - 1, L'a'));

	// 不適切な値を設定（サイズオーバー2）
	cProfile.SetProfileData(L"Test", L"pluginTest.Id", std::wstring(size_t(MAX_PLUGIN_NAME), L'b'));

	// 不完全な設定項目は読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"pluginTest", value));

	cProfile.SetProfileData(L"Test", L"pluginTest.Id", std::wstring(size_t(MAX_PLUGIN_NAME) - 1, L'b'));

	// 不適切な値を設定（型誤り）
	cProfile.SetProfileData(L"Test", L"pluginTest.CmdNum", L"なし");

	// 不完全な設定項目は読めない
	EXPECT_FALSE(cProfile.IOProfileData(L"Test", L"pluginTest", value));

	// 値を設定
	cProfile.SetProfileData(L"Test", L"pluginTest.CmdNum", L"1");

	// 読める
	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"pluginTest", value), IsTrue());

	// 値は変更される
	EXPECT_THAT(value.m_szName, StartsWith(L"aaa"));
	EXPECT_THAT(value.m_szId, StartsWith(L"bbb"));
	EXPECT_THAT(value.m_nCmdNum, 1);
}

/*!
 * @brief IOProfileDataのテスト
 */
TEST(CDataProfile, IOProfileData_RECT)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	RECT value = { 1, 2, 3, 4 };

	// 設定項目がないと読めない
	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"rcTest", value), IsFalse());

	// 値は変更されない
	EXPECT_THAT(value.left, 1);
	EXPECT_THAT(value.top, 2);
	EXPECT_THAT(value.right, 3);
	EXPECT_THAT(value.bottom, 4);

	// 値を設定
	cProfile.SetProfileData(L"Test", L"rcTest", L"4,3,2,1");

	// 読める
	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"rcTest", value), IsTrue());

	// 値は変更される
	EXPECT_THAT(value.left, 4);
	EXPECT_THAT(value.top, 3);
	EXPECT_THAT(value.right, 2);
	EXPECT_THAT(value.bottom, 1);

	// 値を設定（足りない）
	cProfile.SetProfileData(L"Test", L"rcTest", L"109,108,107");

	// 不完全な設定項目は読めない
	EXPECT_THAT(cProfile.IOProfileData(L"Test", L"rcTest", value), IsFalse());

	// 値は変更されない
	EXPECT_THAT(value.left, 4);
	EXPECT_THAT(value.top, 3);
	EXPECT_THAT(value.right, 2);
	EXPECT_THAT(value.bottom, 1);
}

/*!
 * @brief ShareData_IO_2のテスト
 *
 * INIファイルのバージョンが読めなかった場合、バックアップファイルを作成する仕様の確認
 */
TEST(ShareData_IO, ShareData_IO_2)
{
	constexpr std::array UTF8_BOM = { '\xEF', '\xBB', '\xBF' };

	const auto iniPath = GetIniFileName();
	const auto bakPath = std::filesystem::path{ iniPath.native() + L".bak" };

	// ファイル出力ストリームをバイナリモードで開く
	std::ofstream fos{ iniPath };

	// UTF-8 BOMを出力
	fos.write(UTF8_BOM.data(), UTF8_BOM.size());

	// 各行を書き込む
	fos << "[Other]" << std::endl;
	fos << "szVersion=1,2,3" << std::endl;

	fos.close();

	// 共有データを生成して初期化する
	env::ShareDataTestSuite::SetUpShareData();

	// テスト対象を呼び出す
	CShareData_IO::LoadShareData();

	EXPECT_THAT(fexist(bakPath), IsTrue());

	// 共有データを破棄する
	env::ShareDataTestSuite::TearDownShareData();

	std::error_code ec;
	std::filesystem::remove(bakPath, ec);
	std::filesystem::remove(iniPath, ec);
}

/*!
 * @brief ShareData_IO_KeyHelpArrのテスト
 */
TEST(ShareData_IO, ShareData_IO_KeyHelpArr)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	KeyHelpInfo KeyHelpArr[MAX_KEYHELP_FILE]{};
	int nKeyHelpNum = 0;

	cProfile.SetProfileData(L"Test", L"KDct[00]", L"0,about,path");
	cProfile.SetProfileData(L"Test", L"KDct[01]", L"1,ABOUT,PATH");

	ShareData_IO_KeyHelpArr(cProfile, L"Test", KeyHelpArr, nKeyHelpNum);

	EXPECT_THAT(KeyHelpArr[0].m_bUse, IsFalse());
	EXPECT_THAT(KeyHelpArr[0].m_szAbout, StrEq(L"about"));
	EXPECT_THAT(KeyHelpArr[0].m_szPath, StrEq(L"path"));
	EXPECT_THAT(KeyHelpArr[1].m_bUse, IsTrue());
	EXPECT_THAT(KeyHelpArr[1].m_szAbout, StrEq(L"ABOUT"));
	EXPECT_THAT(KeyHelpArr[1].m_szPath, StrEq(L"PATH"));
	EXPECT_THAT(nKeyHelpNum, 2);

	// 旧バージョンサポート
	cProfile.SetProfileData(L"Test", L"szKeyWordHelpFile", L"szKeyWordHelpFile");
	ShareData_IO_KeyHelpArr(cProfile, L"Test", KeyHelpArr, nKeyHelpNum);
	EXPECT_THAT(KeyHelpArr[0].m_szPath, StrEq(L"szKeyWordHelpFile"));
	EXPECT_THAT(nKeyHelpNum, 1);
}

/*!
 * @brief ShareData_IO_LogFontのテスト
 */
TEST(ShareData_IO, ShareData_IO_LogFont)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	LOGFONT lf{};
	INT nPointSize = 0;

	// 設定項目がないと読めない
	ShareData_IO_LogFont(cProfile, L"Test", L"lf", lf, nPointSize);

	EXPECT_THAT(nPointSize, 0);
	EXPECT_THAT(lf.lfHeight, 0);

	// 値を設定
	cProfile.SetProfileData(L"Test", L"nPointSize", L"12");

	// 複合項目なので、やっぱり読めない
	EXPECT_FALSE(ShareData_IO_LogFont(cProfile, L"Test", L"lf", lf, nPointSize));

	EXPECT_THAT(nPointSize, 12);
	EXPECT_THAT(lf.lfHeight, 0);

	// 不適切な値を設定（足りない）
	cProfile.SetProfileData(L"Test", L"lf", L"1,2,3,4,5,6,7,8,9,10,11,12");

	// 項目が足りないので読めない
	EXPECT_FALSE(ShareData_IO_LogFont(cProfile, L"Test", L"lf", lf, nPointSize));

	// 値を設定
	cProfile.SetProfileData(L"Test", L"lf", L"1,2,3,4,5,6,7,8,9,10,11,12,13");

	// 複合項目なので、やっぱり読めない
	EXPECT_FALSE(ShareData_IO_LogFont(cProfile, L"Test", L"lf", lf, nPointSize));

	EXPECT_THAT(nPointSize, 12);
	EXPECT_THAT(lf.lfHeight, 0);
	EXPECT_THAT(lf.lfFaceName[0], L'\0');

	// 値を設定
	cProfile.SetProfileData(L"Test", L"lfFaceName", L"font name");

	// 読める
	EXPECT_TRUE(ShareData_IO_LogFont(cProfile, L"Test", L"lf", lf, nPointSize));

	EXPECT_THAT(nPointSize,				12);
	EXPECT_THAT(lf.lfHeight,			-2);	// 補正が入るので入力値と異なる
	EXPECT_THAT(lf.lfWidth,				0x2);
	EXPECT_THAT(lf.lfEscapement,		0x3);
	EXPECT_THAT(lf.lfOrientation,		0x4);
	EXPECT_THAT(lf.lfWeight,			0x5);
	EXPECT_THAT(lf.lfItalic,			0x6);
	EXPECT_THAT(lf.lfUnderline,			0x7);
	EXPECT_THAT(lf.lfStrikeOut,			0x8);
	EXPECT_THAT(lf.lfCharSet,			0x9);
	EXPECT_THAT(lf.lfOutPrecision,		0xA);
	EXPECT_THAT(lf.lfClipPrecision,		0xB);
	EXPECT_THAT(lf.lfQuality,			0xC);
	EXPECT_THAT(lf.lfPitchAndFamily,	0xD);
	EXPECT_THAT(lf.lfFaceName,			StrEq(L"font name"));
}

/*!
 * @brief ShareData_IO_OutlineDockRectのテスト
 */
TEST(ShareData_IO, ShareData_IO_OutlineDockRect)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	CommonSetting_OutLine value = {};
	value.m_cxOutlineDockLeft	= 1;
	value.m_cyOutlineDockTop	= 2;
	value.m_cxOutlineDockRight	= 3;
	value.m_cyOutlineDockBottom	= 4;

	// 設定項目がないと読めない
	ShareData_IO_OutlineDockRect(cProfile, L"Test", L"xyOutlineDock", value);

	// 値は変更されない
	EXPECT_THAT(value.m_cxOutlineDockLeft, 1);
	EXPECT_THAT(value.m_cyOutlineDockTop, 2);
	EXPECT_THAT(value.m_cxOutlineDockRight, 3);
	EXPECT_THAT(value.m_cyOutlineDockBottom, 4);

	// 値を設定
	cProfile.SetProfileData(L"Test", L"xyOutlineDock", L"4,3,2,1");

	// 読める
	ShareData_IO_OutlineDockRect(cProfile, L"Test", L"xyOutlineDock", value);

	// 値は変更される
	EXPECT_THAT(value.m_cxOutlineDockLeft, 4);
	EXPECT_THAT(value.m_cyOutlineDockTop, 3);
	EXPECT_THAT(value.m_cxOutlineDockRight, 2);
	EXPECT_THAT(value.m_cyOutlineDockBottom, 1);

	// 値を設定（足りない）
	cProfile.SetProfileData(L"Test", L"xyOutlineDock", L"109,108,107");

	// 不完全な設定項目は読めない
	ShareData_IO_OutlineDockRect(cProfile, L"Test", L"xyOutlineDock", value);

	// 値は変更されない
	EXPECT_THAT(value.m_cxOutlineDockLeft, 4);
	EXPECT_THAT(value.m_cyOutlineDockTop, 3);
	EXPECT_THAT(value.m_cxOutlineDockRight, 2);
	EXPECT_THAT(value.m_cyOutlineDockBottom, 1);
}

/*!
 * @brief ShareData_IO_Printのテスト
 */
TEST(ShareData_IO, ShareData_IO_Print)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	cProfile.SetProfileData(L"Print", L"PS[00].szHF[0]", L"&f");
	cProfile.SetProfileData(L"Print", L"PS[00].szFTF[0]", L"&C- &P -");

	std::vector<PRINTSETTING> printSettings{ static_cast<size_t>(MAX_PRINTSETTINGARR) };

	ShareData_IO_Print(cProfile, printSettings);

	EXPECT_THAT(printSettings[0].m_szHeaderForm[0], StrEq(L"$f"));
	EXPECT_THAT(printSettings[0].m_szFooterForm[0], StrEq(L""));
	EXPECT_THAT(printSettings[0].m_szFooterForm[1], StrEq(L"- $p -"));
}

/*!
 * @brief ShareData_IO_PrintIntsのテスト
 */
TEST(ShareData_IO, ShareData_IO_PrintInts)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	PRINTSETTING printSetting{};

	// テスト用に初期値を入れる
	printSetting.m_nPrintFontWidth			= 0;
	printSetting.m_nPrintFontHeight			= 0;
	printSetting.m_nPrintDansuu				= 0;
	printSetting.m_nPrintDanSpace			= 0;
	printSetting.m_nPrintLineSpacing		= 0;
	printSetting.m_nPrintMarginTY			= 0;
	printSetting.m_nPrintMarginBY			= 0;
	printSetting.m_nPrintMarginLX			= 0;
	printSetting.m_nPrintMarginRX			= 0;
	printSetting.m_nPrintPaperOrientation	= 0;
	printSetting.m_nPrintPaperSize			= 0;
	printSetting.m_bPrintWordWrap			= false;
	printSetting.m_bPrintLineNumber			= true;
	printSetting.m_bHeaderUse[0]			= FALSE;
	printSetting.m_bHeaderUse[1]			= TRUE;
	printSetting.m_bHeaderUse[2]			= FALSE;
	printSetting.m_bFooterUse[0]			= TRUE;
	printSetting.m_bFooterUse[1]			= FALSE;
	printSetting.m_bFooterUse[2]			= TRUE;

	// 設定項目がないと読めない
	EXPECT_FALSE(ShareData_IO_PrintInts(cProfile, L"Test", L"PS[00].nInts", printSetting));

	EXPECT_THAT(printSetting.m_nPrintFontWidth,			0);
	EXPECT_THAT(printSetting.m_nPrintFontHeight,		0);
	EXPECT_THAT(printSetting.m_nPrintDansuu,			0);
	EXPECT_THAT(printSetting.m_nPrintDanSpace,			0);
	EXPECT_THAT(printSetting.m_nPrintLineSpacing,		0);
	EXPECT_THAT(printSetting.m_nPrintMarginTY,			0);
	EXPECT_THAT(printSetting.m_nPrintMarginBY,			0);
	EXPECT_THAT(printSetting.m_nPrintMarginLX,			0);
	EXPECT_THAT(printSetting.m_nPrintMarginRX,			0);
	EXPECT_THAT(printSetting.m_nPrintPaperOrientation,	0);
	EXPECT_THAT(printSetting.m_nPrintPaperSize,			0);
	EXPECT_THAT(printSetting.m_bPrintWordWrap,			IsFalse());
	EXPECT_THAT(printSetting.m_bPrintLineNumber,		IsTrue());
	EXPECT_THAT(printSetting.m_bHeaderUse[0],			IsFalse());
	EXPECT_THAT(printSetting.m_bHeaderUse[1],			IsTrue());
	EXPECT_THAT(printSetting.m_bHeaderUse[2],			IsFalse());
	EXPECT_THAT(printSetting.m_bFooterUse[0],			IsTrue());
	EXPECT_THAT(printSetting.m_bFooterUse[1],			IsFalse());
	EXPECT_THAT(printSetting.m_bFooterUse[2],			IsTrue());

	// 値を設定(18個指定で失敗させる)
	cProfile.SetProfileData(L"Test", L"PS[00].nInts", L"1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18");

	// 項目足りないので、読めない
	EXPECT_FALSE(ShareData_IO_PrintInts(cProfile, L"Test", L"PS[00].nInts", printSetting));

	EXPECT_THAT(printSetting.m_nPrintFontWidth,			0);
	EXPECT_THAT(printSetting.m_nPrintFontHeight,		0);
	EXPECT_THAT(printSetting.m_nPrintDansuu,			0);
	EXPECT_THAT(printSetting.m_nPrintDanSpace,			0);
	EXPECT_THAT(printSetting.m_nPrintLineSpacing,		0);
	EXPECT_THAT(printSetting.m_nPrintMarginTY,			0);
	EXPECT_THAT(printSetting.m_nPrintMarginBY,			0);
	EXPECT_THAT(printSetting.m_nPrintMarginLX,			0);
	EXPECT_THAT(printSetting.m_nPrintMarginRX,			0);
	EXPECT_THAT(printSetting.m_nPrintPaperOrientation,	0);
	EXPECT_THAT(printSetting.m_nPrintPaperSize,			0);
	EXPECT_THAT(printSetting.m_bPrintWordWrap,			IsFalse());
	EXPECT_THAT(printSetting.m_bPrintLineNumber,		IsTrue());
	EXPECT_THAT(printSetting.m_bHeaderUse[0],			IsFalse());
	EXPECT_THAT(printSetting.m_bHeaderUse[1],			IsTrue());
	EXPECT_THAT(printSetting.m_bHeaderUse[2],			IsFalse());
	EXPECT_THAT(printSetting.m_bFooterUse[0],			IsTrue());
	EXPECT_THAT(printSetting.m_bFooterUse[1],			IsFalse());
	EXPECT_THAT(printSetting.m_bFooterUse[2],			IsTrue());

	// 値を設定
	cProfile.SetProfileData(L"Test", L"PS[00].nInts", L"1,2,3,4,5,6,7,8,9,10,11,1,0,1,0,1,0,1,0");

	// 値が揃ったので読める
	EXPECT_TRUE(ShareData_IO_PrintInts(cProfile, L"Test", L"PS[00].nInts", printSetting));

	EXPECT_THAT(printSetting.m_nPrintFontWidth,			0x1);
	EXPECT_THAT(printSetting.m_nPrintFontHeight,		0x2);
	EXPECT_THAT(printSetting.m_nPrintDansuu,			0x3);
	EXPECT_THAT(printSetting.m_nPrintDanSpace,			0x4);
	EXPECT_THAT(printSetting.m_nPrintLineSpacing,		0x5);
	EXPECT_THAT(printSetting.m_nPrintMarginTY,			0x6);
	EXPECT_THAT(printSetting.m_nPrintMarginBY,			0x7);
	EXPECT_THAT(printSetting.m_nPrintMarginLX,			0x8);
	EXPECT_THAT(printSetting.m_nPrintMarginRX,			0x9);
	EXPECT_THAT(printSetting.m_nPrintPaperOrientation,	0xA);
	EXPECT_THAT(printSetting.m_nPrintPaperSize,			0xB);
	EXPECT_THAT(printSetting.m_bPrintWordWrap,			IsTrue());
	EXPECT_THAT(printSetting.m_bPrintLineNumber,		IsFalse());
	EXPECT_THAT(printSetting.m_bHeaderUse[0],			IsTrue());
	EXPECT_THAT(printSetting.m_bHeaderUse[1],			IsFalse());
	EXPECT_THAT(printSetting.m_bHeaderUse[2],			IsTrue());
	EXPECT_THAT(printSetting.m_bFooterUse[0],			IsFalse());
	EXPECT_THAT(printSetting.m_bFooterUse[1],			IsTrue());
	EXPECT_THAT(printSetting.m_bFooterUse[2],			IsFalse());
}

/*!
 * @brief ShareData_IO_Pluginのテスト
 */
TEST(ShareData_IO, ShareData_IO_Plugin)
{
	CDataProfile cProfile;
	cProfile.SetWritingMode();

	CommonSetting_Plugin sPlugin{};
	::wcscpy_s(sPlugin.m_PluginTable[0].m_szName, L"test name");
	::wcscpy_s(sPlugin.m_PluginTable[0].m_szId, L"test id");
	sPlugin.m_PluginTable[0].m_state = EPluginState::PLS_DELETED;
	sPlugin.m_PluginTable[0].m_nCmdNum = 2;

	// 共有データを生成して初期化する
	env::ShareDataTestSuite::SetUpShareData();

	auto pcMenuDrawer = std::make_unique<CMenuDrawer>();

	// 削除機能の確認
	ShareData_IO_Plugin(cProfile, pcMenuDrawer.get(), sPlugin);

	EXPECT_THAT(sPlugin.m_PluginTable[0].m_szName, StrEq(L""));
	EXPECT_THAT(sPlugin.m_PluginTable[0].m_szId, StrEq(L""));

	pcMenuDrawer = nullptr;

	// 共有データを破棄する
	env::ShareDataTestSuite::TearDownShareData();
}

/*!
 * @brief ShareData_IO_RegexKeywordのテスト
 */
TEST(ShareData_IO, ShareData_IO_RegexKeyword)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	const auto pType = std::make_unique<STypeConfig>();
	auto& type = *pType;

	// テスト用に初期値を入れる
	type.m_bUseRegexKeyword		= true;

	// 設定項目がないと読めない
	ShareData_IO_RegexKeyword(cProfile, L"Test", type);

	// 値は変更されない
	EXPECT_THAT(type.m_bUseRegexKeyword, IsTrue());

	// 値を設定
	cProfile.SetProfileData(L"Test", L"bUseRegexKeyword", L"0");
	cProfile.SetProfileData(L"Test", L"RxKey[000]", std::format(L"{},{}", int(COLORIDX_LAST), L"patten"));
	cProfile.SetProfileData(L"Test", L"RxKey[001]", std::format(L"{},{}", std::wstring(20, L'c'), L"patten"));
	cProfile.SetProfileData(L"Test", L"RxKey[002]", std::format(L"{},{}", L"TXT", L"patten"));

	// 設定項目があれば読める
	ShareData_IO_RegexKeyword(cProfile, L"Test", type);

	// 値は反映される
	EXPECT_THAT(type.m_bUseRegexKeyword, IsFalse());
	EXPECT_THAT(type.m_RegexKeywordArr[0].m_nColorIndex, Eq<int>(COLORIDX_REGEX1));
	EXPECT_THAT(type.m_RegexKeywordArr[1].m_nColorIndex, Eq<int>(COLORIDX_REGEX1));
	EXPECT_THAT(type.m_RegexKeywordArr[2].m_nColorIndex, Eq<int>(COLORIDX_TEXT));
}

/*!
 * @brief ShareData_IO_TypeIntsのテスト
 */
TEST(ShareData_IO, ShareData_IO_TypeInts)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	const auto pType = std::make_unique<STypeConfig>();
	auto& type = *pType;

	// テスト用に初期値を入れる
	type.m_nIdx					= 0;
	type.m_nMaxLineKetas		= 0;
	type.m_nColumnSpace			= 0;
	type.m_nTabSpace			= 0;
	type.m_nKeyWordSetIdx[0]	= 0;
	type.m_nKeyWordSetIdx[1]	= 0;
	type.m_nStringType			= 0;
	type.m_bLineNumIsCRLF		= false;
	type.m_nLineTermType		= 0;
	type.m_bWordWrap			= false;
	type.m_nCurrentPrintSetting	= 0;
	type.m_nTsvMode				= 0;

	// 設定項目がないと読めない
	ShareData_IO_TypeInts(cProfile, L"Test", type);

	EXPECT_THAT(type.m_nIdx,					0);
	EXPECT_THAT(type.m_nMaxLineKetas,			0);
	EXPECT_THAT(type.m_nColumnSpace,			0);
	EXPECT_THAT(type.m_nTabSpace,				0);
	EXPECT_THAT(type.m_nKeyWordSetIdx[0],		0);
	EXPECT_THAT(type.m_nKeyWordSetIdx[1],		0);
	EXPECT_THAT(type.m_nStringType,				0);
	EXPECT_THAT(type.m_bLineNumIsCRLF,			IsFalse());
	EXPECT_THAT(type.m_nLineTermType,			0);
	EXPECT_THAT(type.m_bWordWrap,				IsFalse());
	EXPECT_THAT(type.m_nCurrentPrintSetting,	0);
	EXPECT_THAT(type.m_nTsvMode,				0);

	// 値を設定(11個指定で失敗させる)
	cProfile.SetProfileData(L"Test", L"nInts", L"1,2,3,4,5,6,7,1,9,1,11");

	// 項目足りないので、読めない
	ShareData_IO_TypeInts(cProfile, L"Test", type);

	EXPECT_THAT(type.m_nIdx,					0);
	EXPECT_THAT(type.m_nMaxLineKetas,			0);
	EXPECT_THAT(type.m_nColumnSpace,			0);
	EXPECT_THAT(type.m_nTabSpace,				0);
	EXPECT_THAT(type.m_nKeyWordSetIdx[0],		0);
	EXPECT_THAT(type.m_nKeyWordSetIdx[1],		0);
	EXPECT_THAT(type.m_nStringType,				0);
	EXPECT_THAT(type.m_bLineNumIsCRLF,			IsFalse());
	EXPECT_THAT(type.m_nLineTermType,			0);
	EXPECT_THAT(type.m_bWordWrap,				IsFalse());
	EXPECT_THAT(type.m_nCurrentPrintSetting,	0);
	EXPECT_THAT(type.m_nTsvMode,				0);

	// 値を設定
	cProfile.SetProfileData(L"Test", L"nInts", L"1,2,3,4,5,6,7,1,9,1,11,12");

	// 値が揃ったので読める
	ShareData_IO_TypeInts(cProfile, L"Test", type);

	EXPECT_THAT(type.m_nIdx,					1);
	EXPECT_THAT(type.m_nMaxLineKetas,			10);
	EXPECT_THAT(type.m_nColumnSpace,			3);
	EXPECT_THAT(type.m_nTabSpace,				4);
	EXPECT_THAT(type.m_nKeyWordSetIdx[0],		5);
	EXPECT_THAT(type.m_nKeyWordSetIdx[1],		6);
	EXPECT_THAT(type.m_nStringType,				7);
	EXPECT_THAT(type.m_bLineNumIsCRLF,			IsTrue());
	EXPECT_THAT(type.m_nLineTermType,			9);
	EXPECT_THAT(type.m_bWordWrap,				IsTrue());
	EXPECT_THAT(type.m_nCurrentPrintSetting,	11);
	EXPECT_THAT(type.m_nTsvMode,				12);
}

/*!
 * @brief ShareData_IO_VertLineIdxのテスト
 */
TEST(ShareData_IO, ShareData_IO_VertLineIdx)
{
	CDataProfile cProfile;
	cProfile.SetReadingMode();

	CKetaXInt nVertLineIdx[MAX_VERTLINES]{};

	// 設定項目がないと読めない
	ShareData_IO_VertLineIdx(cProfile, L"Test", nVertLineIdx);

	// 値を設定
	cProfile.SetProfileData(L"Test", L"nVertLineIdx1", L"20");

	ShareData_IO_VertLineIdx(cProfile, L"Test", nVertLineIdx);

	EXPECT_THAT(nVertLineIdx[0], 20);
}
