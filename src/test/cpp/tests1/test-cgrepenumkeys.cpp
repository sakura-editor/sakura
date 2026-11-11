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
	EXPECT_THAT(patterns, Eq(std::vector<std::wstring>({ L"*.cpp", L"*.h", L"*.txt", L"*.md" })));
}

/*!
	@brief 引用符が取り除かれ、引用符内の区切り文字が無視されること
*/
TEST(CGrepEnumKeys, SplitPattern_DoubleQuotes)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L"\"a b;c.txt\";*.h");
	EXPECT_THAT(patterns, Eq(std::vector<std::wstring>({ L"a b;c.txt", L"*.h" })));
}

/*!
	@brief 空文字列を渡すと空の配列が返ること
*/
TEST(CGrepEnumKeys, SplitPattern_Empty)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L"");
	EXPECT_THAT(patterns.empty(), IsTrue());
}

/*!
	@brief 区切り文字だけを渡すと空の配列が返ること
*/
TEST(CGrepEnumKeys, SplitPattern_DelimitersOnly)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L" ;,");
	EXPECT_THAT(patterns.empty(), IsTrue());
}

/*!
	@brief 引用符が閉じられていない場合、以降が1つのトークンとして扱われること

	引用符の中では区切り文字を無視するため、閉じ忘れると末尾まで
	1トークンになる。現在の挙動を追認するテスト。
*/
TEST(CGrepEnumKeys, SplitPattern_UnclosedDoubleQuote)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L"\"*.cpp;*.h");
	EXPECT_THAT(patterns, Eq(std::vector<std::wstring>({ L"*.cpp;*.h" })));
}

/*!
	@brief トークンの途中の引用符でも内外が切り替わること

	a"b c"d は1つの要素になり、引用符を取り除くと ab cd になる。
*/
TEST(CGrepEnumKeys, SplitPattern_QuoteInsideToken)
{
	const auto patterns = CGrepEnumKeys::SplitPattern(L"a\"b c\"d;*.h");
	EXPECT_THAT(patterns, Eq(std::vector<std::wstring>({ L"ab cd", L"*.h" })));
}

/*!
	@brief 引用符を残したまま区切り文字で分割されること
*/
TEST(CGrepEnumKeys, SplitPatternKeepQuotes_KeepsQuotes)
{
	const auto patterns = CGrepEnumKeys::SplitPatternKeepQuotes(L"\"a,b.txt\" *.h;a\"b c\"d,\"x y\"");
	EXPECT_THAT(patterns, Eq(std::vector<std::wstring_view>({ L"\"a,b.txt\"", L"*.h", L"a\"b c\"d", L"\"x y\"" })));
}

/*!
	@brief 空文字列・区切り文字だけの場合に空の配列が返ること
*/
TEST(CGrepEnumKeys, SplitPatternKeepQuotes_Empty)
{
	EXPECT_THAT(CGrepEnumKeys::SplitPatternKeepQuotes(L"").empty(), IsTrue());
	EXPECT_THAT(CGrepEnumKeys::SplitPatternKeepQuotes(L" ;, ;").empty(), IsTrue());
}

/*!
	@brief 引用符の閉じ忘れを判定できること
*/
TEST(CGrepEnumKeys, HasUnclosedQuote)
{
	EXPECT_THAT(CGrepEnumKeys::HasUnclosedQuote(L""), IsFalse());
	EXPECT_THAT(CGrepEnumKeys::HasUnclosedQuote(L"*.cpp;*.h"), IsFalse());
	EXPECT_THAT(CGrepEnumKeys::HasUnclosedQuote(L"\"a,b.txt\";\"x y\""), IsFalse());
	EXPECT_THAT(CGrepEnumKeys::HasUnclosedQuote(L"\"a,b.txt"), IsTrue());
	EXPECT_THAT(CGrepEnumKeys::HasUnclosedQuote(L"\"a\" \"b"), IsTrue());
}

/*!
	@brief 検索パターン省略時に既定のワイルドカードが補われること
*/
TEST(CGrepEnumKeys, SetFileKeys_DefaultWildcard)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L""), Eq(0));
	EXPECT_THAT(ToStrings(keys.m_vecSearchFileKeys), Eq(std::vector<std::wstring>({ L"*.*" })));
	EXPECT_THAT(ToStrings(keys.m_vecSearchFolderKeys), Eq(std::vector<std::wstring>({ L"*.*" })));
}

/*!
	@brief 接頭辞(なし・!・#)でパターンが振り分けられること
*/
TEST(CGrepEnumKeys, SetFileKeys_Classify)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"*.cpp !*.bak #obj"), Eq(0));
	EXPECT_THAT(ToStrings(keys.m_vecSearchFileKeys), Eq(std::vector<std::wstring>({ L"*.cpp" })));
	EXPECT_THAT(ToStrings(keys.m_vecExceptFileKeys), Eq(std::vector<std::wstring>({ L"*.bak" })));
	EXPECT_THAT(ToStrings(keys.m_vecExceptFolderKeys), Eq(std::vector<std::wstring>({ L"obj" })));
	EXPECT_THAT(ToStrings(keys.m_vecSearchFolderKeys), Eq(std::vector<std::wstring>({ L"*.*" })));
}

/*!
	@brief 除外指定の絶対パスが専用の配列に振り分けられること
*/
TEST(CGrepEnumKeys, SetFileKeys_AbsoluteExcept)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"*.cpp !C:\\work\\*.bak #C:\\work\\obj"), Eq(0));
	EXPECT_THAT(ToStrings(keys.m_vecExceptAbsFileKeys), Eq(std::vector<std::wstring>({ L"C:\\work\\*.bak" })));
	EXPECT_THAT(ToStrings(keys.m_vecExceptAbsFolderKeys), Eq(std::vector<std::wstring>({ L"C:\\work\\obj" })));
	EXPECT_THAT(keys.m_vecExceptFileKeys.empty(), IsTrue());
	EXPECT_THAT(keys.m_vecExceptFolderKeys.empty(), IsTrue());
}

/*!
	@brief UNCパスの除外指定が絶対パスとして振り分けられること
*/
TEST(CGrepEnumKeys, SetFileKeys_UncPathExcept)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"*.cpp !\\\\server\\share\\*.bak"), Eq(0));
	EXPECT_THAT(ToStrings(keys.m_vecExceptAbsFileKeys), Eq(std::vector<std::wstring>({ L"\\\\server\\share\\*.bak" })));
	EXPECT_THAT(keys.m_vecExceptFileKeys.empty(), IsTrue());
}

/*!
	@brief 同じパターンを重複して登録しないこと
*/
TEST(CGrepEnumKeys, SetFileKeys_Unique)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"*.cpp *.cpp"), Eq(0));
	EXPECT_THAT(ToStrings(keys.m_vecSearchFileKeys), Eq(std::vector<std::wstring>({ L"*.cpp" })));
}

/*!
	@brief フォルダー部分のワイルドカードがエラーになること
*/
TEST(CGrepEnumKeys, SetFileKeys_WildcardInFolderIsError)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"*\\file.exe"), Eq(1));
}

/*!
	@brief フォルダー部分の '?' ワイルドカードがエラーになること
*/
TEST(CGrepEnumKeys, SetFileKeys_QuestionMarkInFolderIsError)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"?\\file.exe"), Eq(1));
}

/*!
	@brief 検索対象への絶対パス指定がエラーになること
*/
TEST(CGrepEnumKeys, SetFileKeys_AbsoluteSearchIsError)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"C:\\work\\*.cpp"), Eq(2));
}

/*!
	@brief 再呼び出し時に絶対パスの除外指定が持ち越されないこと

	ClearItems() が絶対パス用の2配列をクリアしていなかった不具合の回帰テスト。
*/
TEST(CGrepEnumKeys, SetFileKeys_ClearsAbsoluteExceptOnReentry)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"*.cpp !C:\\work\\*.bak #C:\\work\\obj"), Eq(0));
	ASSERT_THAT(keys.m_vecExceptAbsFileKeys.size(), Eq(1u));
	ASSERT_THAT(keys.m_vecExceptAbsFolderKeys.size(), Eq(1u));

	EXPECT_THAT(keys.SetFileKeys(L"*.cpp"), Eq(0));
	EXPECT_THAT(keys.m_vecExceptAbsFileKeys.empty(), IsTrue());
	EXPECT_THAT(keys.m_vecExceptAbsFolderKeys.empty(), IsTrue());
}

/*!
	@brief 除外リストが相対パス・絶対パスの順に連結されること
*/
TEST(CGrepEnumKeys, GetExcludeFilesAndFolders)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"*.cpp !*.bak !C:\\work\\*.tmp #obj #C:\\work\\bin"), Eq(0));
	EXPECT_THAT(ToStrings(keys.GetExcludeFiles()), Eq(std::vector<std::wstring>({ L"*.bak", L"C:\\work\\*.tmp" })));
	EXPECT_THAT(ToStrings(keys.GetExcludeFolders()), Eq(std::vector<std::wstring>({ L"obj", L"C:\\work\\bin" })));
}

/*!
	@brief 除外パターンの追加が既存の解析結果に積まれること
*/
TEST(CGrepEnumKeys, AddExceptFileAndFolder)
{
	CGrepEnumKeys keys;
	EXPECT_THAT(keys.SetFileKeys(L"*.cpp !*.bak"), Eq(0));
	EXPECT_THAT(keys.AddExceptFile(L"*.tmp;C:\\work\\*.log"), Eq(0));
	EXPECT_THAT(keys.AddExceptFolder(L"obj"), Eq(0));
	EXPECT_THAT(ToStrings(keys.m_vecExceptFileKeys), Eq(std::vector<std::wstring>({ L"*.bak", L"*.tmp" })));
	EXPECT_THAT(ToStrings(keys.m_vecExceptAbsFileKeys), Eq(std::vector<std::wstring>({ L"C:\\work\\*.log" })));
	EXPECT_THAT(ToStrings(keys.m_vecExceptFolderKeys), Eq(std::vector<std::wstring>({ L"obj" })));
}
