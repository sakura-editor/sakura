/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include <tchar.h>
#include <Windows.h>

#include "basis/GrepInfo.h"
#include "agent/CGrepAgent.h"
#include "dlg/CDlgGrepReplace.h"
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
		&& lhs.bGrepBackup == rhs.bGrepBackup;
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
 * @brief SGrepOption::FromGrepInfo() のテスト
 *  GrepInfo の各メンバが SGrepOption の対応するメンバへ移されること
 */
TEST(SGrepOption, FromGrepInfo_CopiesMembers)
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

	const SGrepOption option = SGrepOption::FromGrepInfo(gi);

	EXPECT_TRUE(option.bGrepSubFolder);
	EXPECT_TRUE(option.bGrepStdout);
	EXPECT_FALSE(option.bGrepHeader);
	EXPECT_EQ(CODE_EUC, option.nGrepCharSet);
	EXPECT_EQ(1, option.nGrepOutputLineType);
	EXPECT_EQ(3, option.nGrepOutputStyle);
	EXPECT_TRUE(option.bGrepOutputFileOnly);
	EXPECT_TRUE(option.bGrepOutputBaseFolder);
	EXPECT_TRUE(option.bGrepSeparateFolder);
	EXPECT_FALSE(option.bGrepReplace);
	EXPECT_TRUE(option.bGrepPaste);
	EXPECT_TRUE(option.bGrepBackup);
}

/*!
 * @brief SGrepOption::FromGrepInfo() のテスト
 *  Grep置換では「一致しなかった行を出力」が行単位出力に落ちること
 */
TEST(SGrepOption, FromGrepInfo_ReplaceDisablesNoHitLine)
{
	GrepInfo gi;
	gi.bGrepReplace = true;
	gi.nGrepOutputLineType = 2;	// 否ヒット行を出力

	const SGrepOption option = SGrepOption::FromGrepInfo(gi);

	EXPECT_TRUE(option.bGrepReplace);
	EXPECT_EQ(1, option.nGrepOutputLineType);	// 行単位に落ちる
}

/*!
 * @brief SGrepOption::FromGrepInfo() のテスト
 *  Grep置換でなければ「一致しなかった行を出力」がそのまま残ること
 */
TEST(SGrepOption, FromGrepInfo_KeepsNoHitLineWhenNotReplace)
{
	GrepInfo gi;
	gi.bGrepReplace = false;
	gi.nGrepOutputLineType = 2;

	const SGrepOption option = SGrepOption::FromGrepInfo(gi);

	EXPECT_FALSE(option.bGrepReplace);
	EXPECT_EQ(2, option.nGrepOutputLineType);
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
