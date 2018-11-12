#include <gtest/gtest.h>

#define NOMINMAX
#include <tchar.h>
#include <Windows.h>

// テスト対象関数のヘッダファイル
//#include "util/string_ex.h" //依存関係が多いのでテスト対象の関数定義のみ抜き出し
BOOL IsMailAddress(const wchar_t* pszBuf, int nBufLen, int* pnAddressLenfth);


// 変更前実装の定義
// ※関数定義は IsMailAddress_20160427.cpp を参照。
BOOL IsMailAddress_20160427(const wchar_t* pszBuf, int nBufLen, int* pnAddressLenfth);


//////////////////////////////////////////////////////////////////////
// テストマクロ

// 新旧動作比較用マクロ1(TRUE, FALSE向け)
// ASSERT_SAME: 旧実装と新実装で動作が変わらないことを期待
#define ASSERT_SAME(expected, szTarget, cchTarget, pchMatchedLen) \
		EXPECT_##expected(_OLD_IMPL(szTarget, cchTarget, pchMatchedLen)); \
		ASSERT_##expected(_NEW_IMPL(szTarget, cchTarget, pchMatchedLen));

// 新旧動作比較用マクロ2(TRUE, FALSE向け)
// ASSERT_CHANGE: 旧実装と新実装で動作が変わることを期待
#define ASSERT_CHANGE(expected, szTarget, cchTarget, pchMatchedLen) \
		EXPECT_NE(expected, _OLD_IMPL(szTarget, cchTarget, pchMatchedLen)); \
		ASSERT_##expected(_NEW_IMPL(szTarget, cchTarget, pchMatchedLen));

// テスト対象の新旧関数をマクロに当てる
#define _OLD_IMPL IsMailAddress_20160427
#define _NEW_IMPL IsMailAddress


//////////////////////////////////////////////////////////////////////
// テストコード

TEST(testIsMailAddress, CheckBlank)
{
	wchar_t szTest[] = L""; //空文字
	ASSERT_SAME(FALSE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckExample)
{
	wchar_t szTest[] = L"test@example.com"; //標準的なサンプルメールアドレス
	ASSERT_SAME(TRUE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckExampleCoJp)
{
	wchar_t szTest[] = L"test@example.co.jp"; //標準的なサンプルメールアドレス
	ASSERT_SAME(TRUE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckTrailingSpace)
{
	wchar_t szTest[] = L"test@example.co.jp "; //標準的なサンプルメールアドレス
	int mailboxLength;
	ASSERT_SAME(TRUE, szTest, _countof(szTest) - 1, &mailboxLength);
	ASSERT_EQ(_countof(szTest) - 2, mailboxLength);
}

TEST(testIsMailAddress, CheckPunctuation)
{
	wchar_t szTest[] = L"test!#$%&'*+-/=?^_`{|}~@example.com"; //記号類を含む
	ASSERT_SAME(TRUE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckMaxLocalPart)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::swprintf_s(szTest, _countof(szTest), L"%s%s%s%s@example.com", szSeed, szSeed, szSeed, szSeed); //4個繋げて64文字にする
	ASSERT_SAME(TRUE, szTest, ::wcslen(szTest), NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckExceedMaxLocalPart)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::swprintf_s(szTest, _countof(szTest), L"%s%s%s%s0@example.com", szSeed, szSeed, szSeed, szSeed); //4個繋げて64文字 + 1
	ASSERT_CHANGE(FALSE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckMaxMailbox)
{
	wchar_t szTest[256];
	wchar_t szSeed64[64 + 1];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::swprintf_s(szSeed64, _countof(szSeed64), L"%s%s%s%s", szSeed, szSeed, szSeed, szSeed); //4個繋げて64文字にする
	::swprintf_s(szTest, _countof(szTest), L"%s@%.63s.%.63s.%.58s.com", szSeed64, szSeed64, szSeed64, szSeed64); //最大255文字のチェック
	int mailboxLength;
	ASSERT_SAME(TRUE, szTest, _countof(szTest) - 1, &mailboxLength);
	ASSERT_EQ(255, mailboxLength);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckMaxExceedMailbox)
{
	wchar_t szTest[256 + 1];
	wchar_t szSeed64[64 + 1];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::swprintf_s(szSeed64, _countof(szSeed64), L"%s%s%s%s", szSeed, szSeed, szSeed, szSeed); //4個繋げて64文字にする
	::swprintf_s(szTest, _countof(szTest), L"%s@%.63s.%.63s.%.58s0.com", szSeed64, szSeed64, szSeed64, szSeed64); //最大255文字オーバーのチェック
	ASSERT_CHANGE(FALSE, szTest, _countof(szTest) - 1, NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckTooLongDomain)
{
	wchar_t szTest[256];
	wchar_t szSeed64[64 + 1];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::swprintf_s(szSeed64, _countof(szSeed64), L"%s%s%s%s", szSeed, szSeed, szSeed, szSeed); //4個繋げて64文字にする
	::swprintf_s(szTest, _countof(szTest), L"%s@%s.com", szSeed64, szSeed64); //63文字を超えるドメイン
	ASSERT_CHANGE(FALSE, szTest, ::wcslen(szTest), NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckTooShortDomain)
{
	wchar_t szTest[] = L"yajim@my.me"; //ドメイン部は3文字以上
	ASSERT_CHANGE(FALSE, szTest, _countof(szTest) - 1, NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckTooShortCCTLD)
{
	wchar_t szTest[] = L"test@test.c.bak"; //CCTLD部は2文字以上
	ASSERT_CHANGE(FALSE, szTest, _countof(szTest) - 1, NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckDomainIncludesUnderScore)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::swprintf(szTest, _countof(szTest), L"%s@test_domain.com", szSeed); //_を含むドメイン
	ASSERT_CHANGE(FALSE, szTest, ::wcslen(szTest), NULL);
}

TEST(testIsMailAddress, CheckDomainIncludesSingleHyphen)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::swprintf_s(szTest, _countof(szTest), L"%s@test-domain.com", szSeed); //途中に-を含むドメイン
	ASSERT_SAME(TRUE, szTest, ::wcslen(szTest), NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckDomainIncludesDoubleHyphen)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::swprintf_s(szTest, _countof(szTest), L"%s@test--domain.com", szSeed); //途中に-を含むドメイン
	ASSERT_CHANGE(FALSE, szTest, ::wcslen(szTest), NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckQuotedLocalPart)
{
	wchar_t szTest[] = L"\"test\\@c\"@test.com";
	ASSERT_CHANGE(TRUE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckBadQuotedLocalPart)
{
	wchar_t szTest[] = L"\"test@test.com";
	ASSERT_SAME(FALSE, szTest, _countof(szTest) - 1, NULL);
}

// レビューコメントにより追試。動作変えていない部分だが、誤って動作が変わっていた。
TEST(testIsMailAddress, CheckAwithAtmark)
{
	wchar_t szTest[] = L"a@";
	ASSERT_SAME(FALSE, szTest, _countof(szTest) - 1, NULL);
}

//////////////////////////////////////////////////////////////////////
// テストマクロの後始末

#undef _OLD_IMPL
#undef _NEW_IMPL

#undef ASSERT_SAME
#undef ASSERT_CHANGE
