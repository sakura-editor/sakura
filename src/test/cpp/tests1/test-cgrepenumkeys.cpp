/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "grep/CGrepEnumKeys.h"

/*!
	@brief 解析結果を std::wstring の配列に変換する

	VGrepEnumKeys の要素型に依存せず比較できるようにするためのヘルパー。
*/
static std::vector<std::wstring> ToStrings(const VGrepEnumKeys& keys)
{
	return std::vector<std::wstring>(keys.cbegin(), keys.cend());
}

/*!
	@brief 区切り文字(空白・セミコロン・カンマ)でパターンが分割されること
*/
TEST(CGrepEnumKeys, SplitPattern_Delimiters)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L"*.cpp *.h;*.txt,*.md");
	EXPECT_EQ(std::vector<std::wstring>({ L"*.cpp", L"*.h", L"*.txt", L"*.md" }), patterns);
}

/*!
	@brief 引用符が取り除かれ、引用符内の区切り文字が無視されること
*/
TEST(CGrepEnumKeys, SplitPattern_DoubleQuotes)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L"\"a b;c.txt\";*.h");
	EXPECT_EQ(std::vector<std::wstring>({ L"a b;c.txt", L"*.h" }), patterns);
}

/*!
	@brief 空文字列を渡すと空の配列が返ること
*/
TEST(CGrepEnumKeys, SplitPattern_Empty)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L"");
	EXPECT_TRUE(patterns.empty());
}

/*!
	@brief 区切り文字だけを渡すと空の配列が返ること
*/
TEST(CGrepEnumKeys, SplitPattern_DelimitersOnly)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L" ;,");
	EXPECT_TRUE(patterns.empty());
}

/*!
	@brief 引用符が閉じられていない場合、以降が1つのトークンとして扱われること

	my_strtok() は引用符の中では区切り文字を無視するため、閉じ忘れると末尾まで
	1トークンになる。現在の挙動を追認するテスト。
*/
TEST(CGrepEnumKeys, SplitPattern_UnclosedDoubleQuote)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L"\"*.cpp;*.h");
	EXPECT_EQ(std::vector<std::wstring>({ L"*.cpp;*.h" }), patterns);
}

/*!
	@brief 検索パターン省略時に既定のワイルドカードが補われること
*/
TEST(CGrepEnumKeys, SetFileKeys_DefaultWildcard)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L""));
	EXPECT_EQ(std::vector<std::wstring>({ L"*.*" }), ToStrings(keys.m_vecSearchFileKeys));
	EXPECT_EQ(std::vector<std::wstring>({ L"*.*" }), ToStrings(keys.m_vecSearchFolderKeys));
}

/*!
	@brief 接頭辞(なし・!・#)でパターンが振り分けられること
*/
TEST(CGrepEnumKeys, SetFileKeys_Classify)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"*.cpp !*.bak #obj"));
	EXPECT_EQ(std::vector<std::wstring>({ L"*.cpp" }), ToStrings(keys.m_vecSearchFileKeys));
	EXPECT_EQ(std::vector<std::wstring>({ L"*.bak" }), ToStrings(keys.m_vecExceptFileKeys));
	EXPECT_EQ(std::vector<std::wstring>({ L"obj" }), ToStrings(keys.m_vecExceptFolderKeys));
	EXPECT_EQ(std::vector<std::wstring>({ L"*.*" }), ToStrings(keys.m_vecSearchFolderKeys));
}

/*!
	@brief 除外指定の絶対パスが専用の配列に振り分けられること
*/
TEST(CGrepEnumKeys, SetFileKeys_AbsoluteExcept)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"*.cpp !C:\\work\\*.bak #C:\\work\\obj"));
	EXPECT_EQ(std::vector<std::wstring>({ L"C:\\work\\*.bak" }), ToStrings(keys.m_vecExceptAbsFileKeys));
	EXPECT_EQ(std::vector<std::wstring>({ L"C:\\work\\obj" }), ToStrings(keys.m_vecExceptAbsFolderKeys));
	EXPECT_TRUE(keys.m_vecExceptFileKeys.empty());
	EXPECT_TRUE(keys.m_vecExceptFolderKeys.empty());
}

/*!
	@brief UNCパスの除外指定が絶対パスとして振り分けられること
*/
TEST(CGrepEnumKeys, SetFileKeys_UncPathExcept)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"*.cpp !\\\\server\\share\\*.bak"));
	EXPECT_EQ(std::vector<std::wstring>({ L"\\\\server\\share\\*.bak" }), ToStrings(keys.m_vecExceptAbsFileKeys));
	EXPECT_TRUE(keys.m_vecExceptFileKeys.empty());
}

/*!
	@brief 同じパターンを重複して登録しないこと
*/
TEST(CGrepEnumKeys, SetFileKeys_Unique)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"*.cpp *.cpp"));
	EXPECT_EQ(std::vector<std::wstring>({ L"*.cpp" }), ToStrings(keys.m_vecSearchFileKeys));
}

/*!
	@brief フォルダー部分のワイルドカードがエラーになること
*/
TEST(CGrepEnumKeys, SetFileKeys_WildcardInFolderIsError)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(1, keys.SetFileKeys(L"*\\file.exe"));
}

/*!
	@brief フォルダー部分の '?' ワイルドカードがエラーになること
*/
TEST(CGrepEnumKeys, SetFileKeys_QuestionMarkInFolderIsError)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(1, keys.SetFileKeys(L"?\\file.exe"));
}

/*!
	@brief 検索対象への絶対パス指定がエラーになること
*/
TEST(CGrepEnumKeys, SetFileKeys_AbsoluteSearchIsError)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(2, keys.SetFileKeys(L"C:\\work\\*.cpp"));
}

/*!
	@brief 再呼び出し時に絶対パスの除外指定が持ち越されないこと

	ClearItems() が絶対パス用の2配列をクリアしていなかった不具合の回帰テスト。
*/
TEST(CGrepEnumKeys, SetFileKeys_ClearsAbsoluteExceptOnReentry)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"*.cpp !C:\\work\\*.bak #C:\\work\\obj"));
	ASSERT_EQ(1u, keys.m_vecExceptAbsFileKeys.size());
	ASSERT_EQ(1u, keys.m_vecExceptAbsFolderKeys.size());

	EXPECT_EQ(0, keys.SetFileKeys(L"*.cpp"));
	EXPECT_TRUE(keys.m_vecExceptAbsFileKeys.empty());
	EXPECT_TRUE(keys.m_vecExceptAbsFolderKeys.empty());
}

/*!
	@brief 除外リストが相対パス・絶対パスの順に連結されること
*/
TEST(CGrepEnumKeys, GetExcludeFilesAndFolders)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"*.cpp !*.bak !C:\\work\\*.tmp #obj #C:\\work\\bin"));
	EXPECT_EQ(std::vector<std::wstring>({ L"*.bak", L"C:\\work\\*.tmp" }), ToStrings(keys.GetExcludeFiles()));
	EXPECT_EQ(std::vector<std::wstring>({ L"obj", L"C:\\work\\bin" }), ToStrings(keys.GetExcludeFolders()));
}

/*!
	@brief 除外パターンの追加が既存の解析結果に積まれること
*/
TEST(CGrepEnumKeys, AddExceptFileAndFolder)
{
	CGrepEnumKeys keys;
	EXPECT_EQ(0, keys.SetFileKeys(L"*.cpp !*.bak"));
	EXPECT_EQ(0, keys.AddExceptFile(L"*.tmp;C:\\work\\*.log"));
	EXPECT_EQ(0, keys.AddExceptFolder(L"obj"));
	EXPECT_EQ(std::vector<std::wstring>({ L"*.bak", L"*.tmp" }), ToStrings(keys.m_vecExceptFileKeys));
	EXPECT_EQ(std::vector<std::wstring>({ L"C:\\work\\*.log" }), ToStrings(keys.m_vecExceptAbsFileKeys));
	EXPECT_EQ(std::vector<std::wstring>({ L"obj" }), ToStrings(keys.m_vecExceptFolderKeys));
}
