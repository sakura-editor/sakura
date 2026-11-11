/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "grep/CGrepEnumKeys.h"
#include "grep/CGrepEnumFiles.h"
#include "grep/CGrepEnumFilterFiles.h"
#include "grep/GrepTestSuite.hpp"	// grep_test::TempFolder

#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

/*
	除外ファイルの正規表現のテスト

	CGrepEnumKeys: 除外ファイル(正規表現)のキーの振り分けと照合関数
	CGrepEnumFilterFiles: 照合関数による列挙の除外
	CGrepExceptFileRegexps: 正規表現の照合器

	コマンドライン・ダイアログからの実行のテストは、Grep の実行基盤(GrepTestSuite)を使うので
	test-grep-cmdline.cpp・test-grep-dialog.cpp に置く。

	注意: 引用符を含む生文字列はマクロの引数に書かない(MSVC の従来のプリプロセッサーで C2017 やマクロ引数の分割が起きる)。
	変数に入れてからマクロに渡す。区切りの「,」を含む文字列も同様に変数に入れる。
*/

using ::testing::ElementsAre;	// pch.h に using が無いのでここで宣言する

namespace {

using grep_test::TempFolder;

//! 列挙結果の名前を昇順(符号単位)で返す
std::vector<std::wstring> Names(const CGrepEnumFileBase& items)
{
	std::vector<std::wstring> names;
	for (int i = 0; i < items.GetCount(); ++i) {
		names.emplace_back(items.GetFileName(i));
	}
	std::ranges::sort(names);
	return names;
}

} // namespace

// ---------------------------------------------------------------------------
// キーの振り分け(CGrepEnumKeys)
// ---------------------------------------------------------------------------

/*!
	@brief 正規表現モードでは除外ファイル(!)が正規表現の配列に振り分けられること
*/
TEST(CGrepEnumKeysExceptRegex, Classify)
{
	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(LR"(*.cpp;!^[^.]+$;!\.bak$;#obj)", true), Eq(0));

	EXPECT_THAT(keys.m_vecSearchFileKeys, ElementsAre(L"*.cpp"));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, ElementsAre(L"^[^.]+$", LR"(\.bak$)"));
	EXPECT_THAT(keys.m_vecExceptFileKeys, IsEmpty());
	EXPECT_THAT(keys.m_vecExceptAbsFileKeys, IsEmpty());
	EXPECT_THAT(keys.m_vecExceptFolderKeys, ElementsAre(L"obj"));
}

/*!
	@brief 正規表現モードではパスとしての検査をしないこと
*/
TEST(CGrepEnumKeysExceptRegex, SkipsPathValidation)
{
	CGrepEnumKeys keys;
	// ワイルドカードとして扱うと、フォルダー部分に * があるのでエラー
	EXPECT_THAT(keys.SetFileKeys(LR"(!.*\.txt$)"), Eq(1));
	// 正規表現として扱うときはエラーにならない。絶対パスに見えるものも正規表現として扱う
	EXPECT_THAT(keys.SetFileKeys(LR"(!.*\.txt$;!C:\a$)", true), Eq(0));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, ElementsAre(LR"(.*\.txt$)", LR"(C:\a$)"));
	EXPECT_THAT(keys.m_vecExceptAbsFileKeys, IsEmpty());
}

/*!
	@brief 空の正規表現は入れないこと(すべてのファイルが除外されるのを防ぐ)
*/
TEST(CGrepEnumKeysExceptRegex, IgnoresEmpty)
{
	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(L"*.cpp;!", true), Eq(0));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, IsEmpty());
}

/*!
	@brief SetFileKeys() を呼び直すと、正規表現と照合関数がクリアされること
*/
TEST(CGrepEnumKeysExceptRegex, ClearsOnReentry)
{
	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(L"!^a$", true), Eq(0));
	keys.m_fnIsExceptFilePath = [](std::wstring_view) { return true; };

	ASSERT_THAT(keys.SetFileKeys(L"*.cpp"), Eq(0));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, IsEmpty());
	EXPECT_THAT(keys.IsExceptFilePath(L"a"), IsFalse());
}

/*!
	@brief 除外ファイルの一覧(結果の先頭に表示するもの)に正規表現も含まれること
*/
TEST(CGrepEnumKeysExceptRegex, GetExcludeFilesIncludesRegex)
{
	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(L"!^a$;!^b$", true), Eq(0));
	EXPECT_THAT(keys.GetExcludeFiles(), ElementsAre(L"^a$", L"^b$"));
}

/*!
	@brief 照合関数が未設定なら一致しない扱い、設定されていればその結果になること
*/
TEST(CGrepEnumKeysExceptRegex, IsExceptFilePath)
{
	const std::wstring readme = LR"(C:\work\README)";
	const std::wstring text = LR"(C:\work\a.txt)";

	CGrepEnumKeys keys;
	EXPECT_THAT(keys.IsExceptFilePath(readme), IsFalse());

	keys.m_fnIsExceptFilePath = [&readme](std::wstring_view filePath) { return filePath == readme; };
	EXPECT_THAT(keys.IsExceptFilePath(readme), IsTrue());
	EXPECT_THAT(keys.IsExceptFilePath(text), IsFalse());
}

/*!
	@brief 正規表現モードでなければ、除外ファイルは従来どおりワイルドカードとして振り分けられること
*/
TEST(CGrepEnumKeysExceptRegex, OffKeepsWildcard)
{
	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(L"!*.bak"), Eq(0));
	EXPECT_THAT(keys.m_vecExceptFileKeys, ElementsAre(L"*.bak"));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, IsEmpty());
}

/*!
	@brief 同じ正規表現は 1 つにまとめられること
*/
TEST(CGrepEnumKeysExceptRegex, Unique)
{
	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(L"!^a$;!^a$", true), Eq(0));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, ElementsAre(L"^a$"));
}

/*!
	@brief 区切り文字(空白・;・,)を含む正規表現は引用符で囲めば分割されないこと
*/
TEST(CGrepEnumKeysExceptRegex, QuotedDelimiters)
{
	const std::wstring pattern = LR"(!"^a b$";!"\d{2,4}";!"a;b")";	// 引用符を含むのでマクロの外で作る
	const std::wstring digits = LR"(\d{2,4})";

	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(pattern.c_str(), true), Eq(0));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, ElementsAre(L"^a b$", digits, L"a;b"));
}

/*!
	@brief 引用符で囲まない「,」では分割されること(引用符が必要なことの確認)
*/
TEST(CGrepEnumKeysExceptRegex, UnquotedCommaSplits)
{
	const std::wstring pattern = LR"(!\d{2,4})";	// 「,」を含むのでマクロの外で作る

	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(pattern.c_str(), true), Eq(0));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, ElementsAre(LR"(\d{2)"));
	EXPECT_THAT(keys.m_vecSearchFileKeys, ElementsAre(L"4}"));
}

/*!
	@brief 正規表現モードでも、除外フォルダーと検索対象の検査は従来どおりであること
*/
TEST(CGrepEnumKeysExceptRegex, OtherRulesUnchanged)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(LR"(#a*\b)", true), Eq(1));
	EXPECT_THAT(keys.SetFileKeys(LR"(sub*\a.txt)", true), Eq(1));
	EXPECT_THAT(keys.SetFileKeys(LR"(C:\x\*.txt)", true), Eq(2));
}

/*!
	@brief 先頭の「!」を 1 つだけ取り除き、残りはそのまま正規表現になること
*/
TEST(CGrepEnumKeysExceptRegex, PrefixCharsInPattern)
{
	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(L"!!x;!#y", true), Eq(0));
	EXPECT_THAT(keys.m_vecExceptFileRegexKeys, ElementsAre(L"!x", L"#y"));
	EXPECT_THAT(keys.m_vecExceptFolderKeys, IsEmpty());
}

/*!
	@brief 除外ファイルしか指定しないとき、検索対象は既定の「*.*」になること
*/
TEST(CGrepEnumKeysExceptRegex, OnlyExcludes)
{
	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(L"!^a$", true), Eq(0));
	EXPECT_THAT(keys.m_vecSearchFileKeys, ElementsAre(L"*.*"));
}

// ---------------------------------------------------------------------------
// 照合関数のフック(CGrepEnumFilterFiles)
// ---------------------------------------------------------------------------

/*!
	@brief 照合関数に一致したファイルが列挙から除かれること。照合関数が無ければすべて列挙されること
*/
TEST(CGrepEnumFilterFilesExceptRegex, ExceptByFilePathMatcher)
{
	TempFolder folder;
	for (const auto name : { L"a.txt", L"b.log", L"README" }) {
		folder.AddFile(name, "x");
	}

	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(L"*", true), Eq(0));
	{
		CGrepEnumFilterFiles files;
		CGrepEnumFiles absExcept;
		files.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
		EXPECT_THAT(Names(files), ElementsAre(L"README", L"a.txt", L"b.log"));
	}

	const std::wstring readme = (folder.Path() / L"README").wstring();
	keys.m_fnIsExceptFilePath = [&readme](std::wstring_view filePath) { return filePath == readme; };
	{
		CGrepEnumFilterFiles files;
		CGrepEnumFiles absExcept;
		files.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
		EXPECT_THAT(Names(files), ElementsAre(L"a.txt", L"b.log"));
	}
}

/*!
	@brief 照合関数にはフルパス(キーのフォルダー部分を含む)が渡されること
*/
TEST(CGrepEnumFilterFilesExceptRegex, MatcherGetsFullPath)
{
	TempFolder folder;
	folder.AddFile(LR"(sub\x.txt)", "x");

	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(LR"(sub\*.txt)", true), Eq(0));
	std::vector<std::wstring> calledPaths;
	keys.m_fnIsExceptFilePath = [&calledPaths](std::wstring_view filePath) {
		calledPaths.emplace_back(filePath);
		return true;
	};

	CGrepEnumFilterFiles files;
	CGrepEnumFiles absExcept;
	files.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
	const std::wstring expected = (folder.Path() / LR"(sub\x.txt)").wstring();
	EXPECT_THAT(calledPaths, ElementsAre(expected));
	EXPECT_THAT(files.GetCount(), Eq(0));
}

/*!
	@brief フォルダーには照合関数が呼ばれないこと
*/
TEST(CGrepEnumFilterFilesExceptRegex, MatcherNotCalledForFolders)
{
	TempFolder folder;
	folder.AddFolder(L"d");
	folder.AddFile(L"f", "x");

	CGrepEnumKeys keys;
	ASSERT_THAT(keys.SetFileKeys(L"*", true), Eq(0));
	std::vector<std::wstring> calledPaths;
	keys.m_fnIsExceptFilePath = [&calledPaths](std::wstring_view filePath) {
		calledPaths.emplace_back(filePath);
		return false;
	};

	CGrepEnumFilterFiles files;
	CGrepEnumFiles absExcept;
	files.Enumerates(folder.Path().c_str(), keys, CGrepEnumOptions(), absExcept);
	const std::wstring expected = (folder.Path() / L"f").wstring();
	EXPECT_THAT(calledPaths, ElementsAre(expected));
}
