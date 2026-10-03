/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "env/CDataProfile.h"

#include "env/CShareData_IO.h"

#include <Shlwapi.h>

#include <cstdlib>
#include <fstream>

#include "util/file.h"

using namespace std::literals::string_literals;
using namespace std::literals::string_view_literals;

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
