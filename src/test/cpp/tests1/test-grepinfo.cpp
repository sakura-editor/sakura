/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <tchar.h>
#include <Windows.h>

#include "basis/GrepInfo.h"
#include "_main/CCommandLine.h"
#include "dlg/CDlgGrepReplace.h"
#include "grep/CGrepEnumKeys.h"
#include "env/ShareDataTestSuite.hpp"

/*!
 * 同型との等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しい
 * @retval false 等しくない
 */
bool operator == (const GrepInfo& lhs, const GrepInfo& rhs) noexcept {
	if (&lhs == &rhs) return true;
	return lhs.cmGrepKey == rhs.cmGrepKey
		&& lhs.cmGrepRep == rhs.cmGrepRep
		&& lhs.cmGrepFile == rhs.cmGrepFile
		&& lhs.cmGrepFolder == rhs.cmGrepFolder
		&& lhs.sGrepSearchOption == rhs.sGrepSearchOption
		&& lhs.bGrepCurFolder == rhs.bGrepCurFolder
		&& lhs.bGrepStdout == rhs.bGrepStdout
		&& lhs.bGrepHeader == rhs.bGrepHeader
		&& lhs.bGrepSubFolder == rhs.bGrepSubFolder
		&& lhs.nGrepCharSet == rhs.nGrepCharSet
		&& lhs.nGrepOutputStyle == rhs.nGrepOutputStyle
		&& lhs.nGrepOutputLineType == rhs.nGrepOutputLineType
		&& lhs.bGrepOutputFileOnly == rhs.bGrepOutputFileOnly
		&& lhs.bGrepOutputBaseFolder == rhs.bGrepOutputBaseFolder
		&& lhs.bGrepSeparateFolder == rhs.bGrepSeparateFolder
		&& lhs.bGrepReplace == rhs.bGrepReplace
		&& lhs.bGrepPaste == rhs.bGrepPaste
		&& lhs.bGrepBackup == rhs.bGrepBackup
		&& lhs.bGrepExceptFileRegexp == rhs.bGrepExceptFileRegexp;
}

/*!
 * 同型との否定の等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しくない
 * @retval false 等しい
 */
bool operator != (const GrepInfo& lhs, const GrepInfo& rhs) noexcept
{
	return !(lhs == rhs);
}

/*!
 * @brief 等価比較演算子のテスト
 *  初期値同士の等価比較を行う
 */
TEST(GrepInfo, operatorEqualSame)
{
	GrepInfo value, other;
	ASSERT_EQ(value, other);
}

/*!
 * @brief 等価比較演算子のテスト
 *  自分自身との等価比較を行う
 */
TEST(GrepInfo, operatorEqualBySelf)
{
	GrepInfo value;
	ASSERT_EQ(value, value);
}

/*!
 * @brief 否定の等価比較演算子のテスト
 *  メンバの値を変えて、等価比較を行う
 *
 *  合格条件：メンバの値が1つでも違ったら不一致を検出できること。
 */
TEST(GrepInfo, operatorNotEqual)
{
	GrepInfo value, other;

	value.cmGrepKey = L"ぐれっぷ";
	ASSERT_NE(value, other);
	value.cmGrepKey = other.cmGrepKey;

	value.cmGrepRep = L"ちかん";
	ASSERT_NE(value, other);
	value.cmGrepRep = other.cmGrepRep;

	value.cmGrepFile = L"#.git;*.*";
	ASSERT_NE(value, other);
	value.cmGrepFile = other.cmGrepFile;

	value.cmGrepFolder = L"C:\\work\\sakura";
	ASSERT_NE(value, other);
	value.cmGrepFolder = other.cmGrepFolder;

	value.sGrepSearchOption.bRegularExp = true;
	ASSERT_NE(value, other);
	value.sGrepSearchOption = other.sGrepSearchOption;

	value.sGrepSearchOption.bLoHiCase = true;
	ASSERT_NE(value, other);
	value.sGrepSearchOption = other.sGrepSearchOption;

	value.sGrepSearchOption.bWordOnly = true;
	ASSERT_NE(value, other);
	value.sGrepSearchOption = other.sGrepSearchOption;

	value.bGrepCurFolder = true;
	ASSERT_NE(value, other);
	value.bGrepCurFolder = other.bGrepCurFolder;

	value.bGrepStdout = true;
	ASSERT_NE(value, other);
	value.bGrepStdout = other.bGrepStdout;

	value.bGrepHeader = false;
	ASSERT_NE(value, other);
	value.bGrepHeader = other.bGrepHeader;

	value.bGrepSubFolder = true;
	ASSERT_NE(value, other);
	value.bGrepSubFolder = other.bGrepSubFolder;

	value.nGrepCharSet = CODE_EUC;
	ASSERT_NE(value, other);
	value.nGrepCharSet = other.nGrepCharSet;

	value.nGrepOutputStyle = 2;
	ASSERT_NE(value, other);
	value.nGrepOutputStyle = other.nGrepOutputStyle;

	value.nGrepOutputLineType = 2;
	ASSERT_NE(value, other);
	value.nGrepOutputLineType = other.nGrepOutputLineType;

	value.bGrepOutputFileOnly = true;
	ASSERT_NE(value, other);
	value.bGrepOutputFileOnly = other.bGrepOutputFileOnly;

	value.bGrepOutputBaseFolder = true;
	ASSERT_NE(value, other);
	value.bGrepOutputBaseFolder = other.bGrepOutputBaseFolder;

	value.bGrepSeparateFolder = true;
	ASSERT_NE(value, other);
	value.bGrepSeparateFolder = other.bGrepSeparateFolder;

	value.bGrepReplace = true;
	ASSERT_NE(value, other);
	value.bGrepReplace = other.bGrepReplace;

	value.bGrepPaste = true;
	ASSERT_NE(value, other);
	value.bGrepPaste = other.bGrepPaste;

	value.bGrepBackup = true;
	ASSERT_NE(value, other);
	value.bGrepBackup = other.bGrepBackup;

	value.bGrepExceptFileRegexp = true;
	ASSERT_NE(value, other);
	value.bGrepExceptFileRegexp = other.bGrepExceptFileRegexp;
}

/*!
 * @brief 等価比較演算子のテスト
 *  期待結果EQ,期待結果NEでは判定できない、逆条件のテストを行う
 */
TEST(GrepInfo, operatorEqualAndNotEqual)
{
	// 初期値同士の比較(等価になる)
	GrepInfo v1, v2;

	EXPECT_TRUE(v1 == v2);
	EXPECT_FALSE(v1 != v2);

	// 初期値と値を変えた値の比較(不一致になる)
	v2.bGrepBackup = true;
	EXPECT_FALSE(v1 == v2);
	EXPECT_TRUE(v1 != v2);
}

/*!
 * @brief GrepInfo::Normalized() のテスト
 *  補正対象以外のメンバがそのまま引き継がれること
 */
TEST(GrepInfo, Normalized_KeepsOtherMembers)
{
	GrepInfo gi;
	gi.bGrepSubFolder = true;
	gi.bGrepStdout = true;
	gi.bGrepHeader = false;
	gi.nGrepCharSet = CODE_EUC;
	gi.nGrepOutputLineType = 1;
	gi.nGrepOutputStyle = 3;
	gi.bGrepOutputFileOnly = true;
	gi.bGrepOutputBaseFolder = true;
	gi.bGrepSeparateFolder = true;
	gi.bGrepReplace = false;
	gi.bGrepPaste = true;
	gi.bGrepBackup = true;

	const GrepInfo normalized = gi.Normalized();

	EXPECT_TRUE(normalized.bGrepSubFolder);
	EXPECT_TRUE(normalized.bGrepStdout);
	EXPECT_FALSE(normalized.bGrepHeader);
	EXPECT_EQ(CODE_EUC, normalized.nGrepCharSet);
	EXPECT_EQ(1, normalized.nGrepOutputLineType);
	EXPECT_EQ(3, normalized.nGrepOutputStyle);
	EXPECT_TRUE(normalized.bGrepOutputFileOnly);
	EXPECT_TRUE(normalized.bGrepOutputBaseFolder);
	EXPECT_TRUE(normalized.bGrepSeparateFolder);
	EXPECT_FALSE(normalized.bGrepReplace);
	EXPECT_TRUE(normalized.bGrepPaste);
	EXPECT_TRUE(normalized.bGrepBackup);
}

/*!
 * @brief GrepInfo の既定値と Normalized() で、除外ファイルの正規表現の指定が保たれること
 */
TEST(GrepInfo, ExceptFileRegexp_DefaultAndNormalized)
{
	GrepInfo gi;
	EXPECT_THAT(gi.bGrepExceptFileRegexp, IsFalse());

	gi.bGrepExceptFileRegexp = true;
	gi.bGrepReplace = true;
	gi.nGrepOutputLineType = 2;
	const GrepInfo normalized = gi.Normalized();
	EXPECT_THAT(normalized.bGrepExceptFileRegexp, IsTrue());
}

/*!
 * @brief GrepInfo::Normalized() のテスト
 *  Grep置換では「一致しなかった行を出力」が行単位出力に落ちること
 */
TEST(GrepInfo, Normalized_ReplaceDisablesNoHitLine)
{
	GrepInfo gi;
	gi.bGrepReplace = true;
	gi.nGrepOutputLineType = 2;	// 否ヒット行を出力

	const GrepInfo normalized = gi.Normalized();

	EXPECT_TRUE(normalized.bGrepReplace);
	EXPECT_EQ(1, normalized.nGrepOutputLineType);	// 行単位に落ちる
}

/*!
 * @brief GrepInfo::Normalized() のテスト
 *  Grep置換でなければ「一致しなかった行を出力」がそのまま残ること
 */
TEST(GrepInfo, Normalized_KeepsNoHitLineWhenNotReplace)
{
	GrepInfo gi;
	gi.bGrepReplace = false;
	gi.nGrepOutputLineType = 2;

	const GrepInfo normalized = gi.Normalized();

	EXPECT_FALSE(normalized.bGrepReplace);
	EXPECT_EQ(2, normalized.nGrepOutputLineType);
}

/*!
 * @brief GrepInfo::Normalized() のテスト
 *  元のオブジェクトが変更されないこと
 */
TEST(GrepInfo, Normalized_DoesNotModifySelf)
{
	GrepInfo gi;
	gi.bGrepReplace = true;
	gi.nGrepOutputLineType = 2;

	const GrepInfo normalized = gi.Normalized();

	EXPECT_EQ(2, gi.nGrepOutputLineType);
	EXPECT_EQ(1, normalized.nGrepOutputLineType);
}

/*!
 * @brief GrepInfo::MakeCommandLine() のテスト
 *  既定値では、出力形式だけが -GOPT に付き、-GREPR は付かない
 */
TEST(GrepInfo, MakeCommandLine_Default)
{
	GrepInfo gi;
	gi.cmGrepKey.SetString(L"key");
	gi.cmGrepFile.SetString(L"*.txt");
	gi.cmGrepFolder.SetString(LR"(C:\work)");
	const std::wstring expected = LR"(-GREPMODE -GKEY="key" -GFILE="*.txt" -GFOLDER="C:\work" -GCODE=0 -GOPT=1)";	// 引用符を含むのでマクロの外で作る(C2017 の回避)
	EXPECT_THAT(gi.MakeCommandLine(), StrEq(expected));
}

/*!
 * @brief GrepInfo::MakeCommandLine() のテスト
 *  すべての項目を既定値以外にしたコマンドラインを解析すると、元と同じになる(引用符を含む値も)
 */
TEST(GrepInfo, MakeCommandLine_RoundTrip)
{
	GrepInfo gi;
	gi.cmGrepKey.SetString(LR"(a"b)");
	gi.cmGrepRep.SetString(LR"(c"d)");
	gi.cmGrepFile.SetString(LR"(*.txt;!"x,y.txt")");
	gi.cmGrepFolder.SetString(LR"(C:\a b)");
	gi.sGrepSearchOption.bLoHiCase = true;
	gi.sGrepSearchOption.bRegularExp = true;
	gi.sGrepSearchOption.bWordOnly = true;
	gi.bGrepCurFolder = true;
	gi.bGrepStdout = true;
	gi.bGrepHeader = false;
	gi.bGrepSubFolder = true;
	gi.nGrepCharSet = CODE_UTF8;
	gi.nGrepOutputStyle = 3;
	gi.nGrepOutputLineType = 2;
	gi.bGrepOutputFileOnly = true;
	gi.bGrepOutputBaseFolder = true;
	gi.bGrepSeparateFolder = true;
	gi.bGrepReplace = true;
	gi.bGrepPaste = true;
	gi.bGrepBackup = true;
	gi.bGrepExceptFileRegexp = true;

	CCommandLine cCommandLine;
	cCommandLine.ParseCommandLine(gi.MakeCommandLine().c_str(), false);
	EXPECT_THAT(cCommandLine.GetGrepInfoRef() == gi, IsTrue());
}

/*!
 * @brief GrepInfo::MakeCommandLine() のテスト
 *  結果出力(該当部分・該当行・否ヒット行)と結果出力形式(1〜3)の組み合わせが、解析で元に戻る
 */
TEST(GrepInfo, MakeCommandLine_OutputRoundTrip)
{
	for (const int lineType : { 0, 1, 2 }) {
		for (const int style : { 1, 2, 3 }) {
			GrepInfo gi;
			gi.cmGrepKey.SetString(L"key");
			gi.cmGrepFile.SetString(L"*.txt");
			gi.cmGrepFolder.SetString(LR"(C:\work)");
			gi.nGrepOutputLineType = lineType;
			gi.nGrepOutputStyle = style;

			CCommandLine cCommandLine;
			cCommandLine.ParseCommandLine(gi.MakeCommandLine().c_str(), false);
			EXPECT_THAT(cCommandLine.GetGrepInfoRef() == gi, IsTrue()) << "lineType=" << lineType << " style=" << style;
		}
	}
}

/*!
 * @brief GrepInfo::MakeCommandLineOptions() のテスト
 *  結果出力形式が範囲外なら付けない(解析側の既定値 1 になる)
 */
TEST(GrepInfo, MakeCommandLineOptions_OutputStyleOutOfRange)
{
	GrepInfo gi;
	gi.nGrepOutputStyle = 0;
	EXPECT_THAT(gi.MakeCommandLineOptions(), IsEmpty());
}

/*!
 * CDlgGrep を構築するテストのためのフィクスチャクラス
 *
 * CDlgGrep のコンストラクタはウィンドウを作らずメンバを初期化するだけだが、
 * 基底の CDialog が共有データの取得を検証するため、先に用意しておく必要がある。
 */
class CDlgGrepTest : public ::testing::Test {
protected:
	static void SetUpTestSuite() { env::ShareDataTestSuite::SetUpShareData(); }
	static void TearDownTestSuite() { env::ShareDataTestSuite::TearDownShareData(); }
};

/*!
 * @brief CDlgGrep::MakeGrepInfo() のテスト
 *  ダイアログの設定内容が GrepInfo の対応するメンバへ移されること
 */
TEST_F(CDlgGrepTest, MakeGrepInfo_CopiesDialogSettings)
{
	CDlgGrep dlg;
	dlg.m_strText = L"ぐれっぷ";
	dlg.m_szFile = L"*.cpp";
	dlg.m_szFolder = L"C:\\work\\sakura";
	dlg.m_bSubFolder = TRUE;
	dlg.m_sSearchOption.bRegularExp = true;
	dlg.m_nGrepCharSet = CODE_EUC;
	dlg.m_nGrepOutputStyle = 2;
	dlg.m_nGrepOutputLineType = 2;
	dlg.m_bGrepOutputFileOnly = true;
	dlg.m_bGrepOutputBaseFolder = true;
	dlg.m_bGrepSeparateFolder = true;

	const GrepInfo gi = dlg.MakeGrepInfo();

	EXPECT_STREQ(L"ぐれっぷ", gi.cmGrepKey.GetStringPtr());
	EXPECT_STREQ(L"*.cpp", gi.cmGrepFile.GetStringPtr());
	EXPECT_STREQ(L"C:\\work\\sakura", gi.cmGrepFolder.GetStringPtr());
	EXPECT_TRUE(gi.sGrepSearchOption.bRegularExp);
	EXPECT_TRUE(gi.bGrepSubFolder);
	EXPECT_EQ(CODE_EUC, gi.nGrepCharSet);
	EXPECT_EQ(2, gi.nGrepOutputStyle);
	EXPECT_EQ(2, gi.nGrepOutputLineType);
	EXPECT_TRUE(gi.bGrepOutputFileOnly);
	EXPECT_TRUE(gi.bGrepOutputBaseFolder);
	EXPECT_TRUE(gi.bGrepSeparateFolder);

	// Grep置換ではないので置換系は落ちている
	EXPECT_FALSE(gi.bGrepReplace);
	EXPECT_FALSE(gi.bGrepPaste);
	EXPECT_FALSE(gi.bGrepBackup);

	// 従来 Command_GREP が直接渡していた既定値
	EXPECT_FALSE(gi.bGrepCurFolder);
	EXPECT_FALSE(gi.bGrepStdout);
	EXPECT_TRUE(gi.bGrepHeader);
}

/*!
 * @brief CDlgGrep::MakeGrepInfo() のテスト
 *  除外ファイル・除外フォルダーがファイルパターンに pack されること
 */
TEST_F(CDlgGrepTest, MakeGrepInfo_PacksExcludePatterns)
{
	CDlgGrep dlg;
	dlg.m_szFile = L"*.cpp";
	dlg.m_szExcludeFile = L"*.bak";
	dlg.m_szExcludeFolder = L"obj";

	const GrepInfo gi = dlg.MakeGrepInfo();

	// 除外フォルダー(#)が先、除外ファイル(!)が後ろに連結される
	EXPECT_STREQ(L"*.cpp;#obj;!*.bak", gi.cmGrepFile.GetStringPtr());
}

/*!
 * @brief CDlgGrep::MakeGrepInfo() のテスト
 *  引用符で囲んだカンマ入りの除外パターンが分割されずに pack されること (#2677)
 */
TEST_F(CDlgGrepTest, MakeGrepInfo_PacksExcludePatternsWithComma)
{
	CDlgGrep dlg;
	dlg.m_szFile = L"*.*";
	dlg.m_szExcludeFile = L"\"a,b.txt\"";
	dlg.m_szExcludeFolder = L"\"obj,old\"";

	const GrepInfo gi = dlg.MakeGrepInfo();

	// 引用符で囲み直さず、接頭辞の後ろにそのまま連結される
	EXPECT_STREQ(L"*.*;#\"obj,old\";!\"a,b.txt\"", gi.cmGrepFile.GetStringPtr());

	// Grep 実行側で解析すると、除外パターンが1要素のまま復元される
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(gi.cmGrepFile.GetStringPtr()));
	EXPECT_EQ(std::vector<std::wstring>({ L"*.*" }), std::vector<std::wstring>(keys.m_vecSearchFileKeys.cbegin(), keys.m_vecSearchFileKeys.cend()));
	EXPECT_EQ(std::vector<std::wstring>({ L"a,b.txt" }), std::vector<std::wstring>(keys.m_vecExceptFileKeys.cbegin(), keys.m_vecExceptFileKeys.cend()));
	EXPECT_EQ(std::vector<std::wstring>({ L"obj,old" }), std::vector<std::wstring>(keys.m_vecExceptFolderKeys.cbegin(), keys.m_vecExceptFolderKeys.cend()));
}

/*!
 * @brief CDlgGrep::MakeGrepInfo() のテスト
 *  空白入りの除外パターンは、引用符が接頭辞の後ろに付いた形で pack されること
 */
TEST_F(CDlgGrepTest, MakeGrepInfo_PacksExcludePatternsWithSpace)
{
	CDlgGrep dlg;
	dlg.m_szFile = L"*.cpp";
	dlg.m_szExcludeFile = L"\"a b.txt\"";
	dlg.m_szExcludeFolder = L"\"x y\"";

	const GrepInfo gi = dlg.MakeGrepInfo();

	EXPECT_STREQ(L"*.cpp;#\"x y\";!\"a b.txt\"", gi.cmGrepFile.GetStringPtr());
}

/*!
 * @brief CDlgGrepReplace::MakeGrepInfo() のテスト
 *  基底の内容に置換固有の項目が足されること
 */
TEST_F(CDlgGrepTest, MakeGrepInfo_ReplaceDialogAddsReplaceFields)
{
	CDlgGrepReplace dlg;
	dlg.m_strText = L"before";
	dlg.m_strText2 = L"after";
	dlg.m_szFile = L"*.cpp";
	dlg.m_szFolder = L"C:\\work";
	dlg.m_bPaste = true;
	dlg.m_bBackup = true;

	const GrepInfo gi = dlg.MakeGrepInfo();

	EXPECT_STREQ(L"before", gi.cmGrepKey.GetStringPtr());
	EXPECT_STREQ(L"after", gi.cmGrepRep.GetStringPtr());
	EXPECT_STREQ(L"*.cpp", gi.cmGrepFile.GetStringPtr());
	EXPECT_TRUE(gi.bGrepReplace);
	EXPECT_TRUE(gi.bGrepPaste);
	EXPECT_TRUE(gi.bGrepBackup);
}

/*!
 * @brief CDlgGrep::MakeGrepInfo() のテスト
 *  除外ファイルの正規表現の指定が GrepInfo に写されること
 */
TEST_F(CDlgGrepTest, MakeGrepInfo_CopiesExceptFileRegexp)
{
	CDlgGrep dlg;
	EXPECT_THAT(dlg.MakeGrepInfo().bGrepExceptFileRegexp, IsFalse());

	dlg.m_bGrepExceptFileRegexp = true;
	EXPECT_THAT(dlg.MakeGrepInfo().bGrepExceptFileRegexp, IsTrue());
}

/*!
 * @brief CDlgGrep::MakeGrepInfo() のテスト
 *  正規表現の除外ファイル(区切り文字を含むものは引用符付き)が、Grep 実行時の解析で正規表現として振り分けられること
 */
TEST_F(CDlgGrepTest, MakeGrepInfo_ExceptFileRegexpRoundTrip)
{
	CDlgGrep dlg;
	dlg.m_szFile = L"*.cpp";
	dlg.m_szExcludeFile = LR"("\d{2,4}" ^a$)";
	dlg.m_bGrepExceptFileRegexp = true;

	const GrepInfo gi = dlg.MakeGrepInfo();
	const std::wstring expectedFile = LR"(*.cpp;!"\d{2,4}";!^a$)";	// 引用符を含むのでマクロの外で作る(C2017 の回避)
	EXPECT_THAT(gi.cmGrepFile.GetStringPtr(), StrEq(expectedFile));

	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(gi.cmGrepFile.GetStringPtr(), gi.bGrepExceptFileRegexp), Eq(0));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, (VGrepEnumKeys{ LR"(\d{2,4})", L"^a$" }));
	EXPECT_THAT(keys.m_vecExceptFileKeys, IsEmpty());
}

/*!
 * @brief 共有データの既定値では、除外ファイルの正規表現はオフであること
 */
TEST_F(CDlgGrepTest, ShareDataDefault_ExceptFileRegexpIsOff)
{
	EXPECT_THAT(GetDllShareData().m_Common.m_sSearch.m_bGrepExceptFileRegexp, IsFalse());
}
