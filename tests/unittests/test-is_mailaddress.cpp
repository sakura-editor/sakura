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


// テストマクロが他と被らないようにpushする
#pragma push_macro("TEST20180909")

// ローカルテストモードの定義
// ↓をコメントインすると現新比較テストを実行できます。
//#define REGRESSION_TEST

// モードによってテストマクロの定義内容を変える
#ifdef REGRESSION_TEST
	// 新旧動作比較用マクロ(TRUE, FALSE向け)
	#define TEST20180909(pattern, buf, len, plen) \
		EXPECT_##pattern(IsMailAddress_20160427(buf, len, plen)); \
		ASSERT_##pattern(IsMailAddress(buf, len, plen));
#else
	// 新実装検証用マクロ(TRUE, FALSE向け)
	#define TEST20180909(pattern, buf, len, plen) \
		ASSERT_##pattern(IsMailAddress(buf, len, plen));
#endif

//////////////////////////////////////////////////////////////////////

TEST(testIsMailAddress, CheckBlank)
{
	wchar_t szTest[] = L""; //空文字
	TEST20180909(FALSE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckExample)
{
	wchar_t szTest[] = L"test@example.com"; //標準的なサンプルメールアドレス
	TEST20180909(TRUE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckExampleCoJp)
{
	wchar_t szTest[] = L"test@example.co.jp"; //標準的なサンプルメールアドレス
	TEST20180909(TRUE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckTrailingSpace)
{
	wchar_t szTest[] = L"test@example.co.jp "; //標準的なサンプルメールアドレス
	int mailboxLength;
	TEST20180909(TRUE, szTest, _countof(szTest) - 1, &mailboxLength);
	ASSERT_EQ(_countof(szTest) - 2, mailboxLength);
}

TEST(testIsMailAddress, CheckPunctuation)
{
	wchar_t szTest[] = L"test!#$%&'*+-/=?^_`{|}~@example.com"; //記号類を含む
	TEST20180909(TRUE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckMaxLocalPart)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::_swprintf_p(szTest, _countof(szTest), L"%1$s%1$s%1$s%1$s@example.com", szSeed); //4個繋げて64文字にする
	TEST20180909(TRUE, szTest, ::wcslen(szTest), NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckExceedMaxLocalPart)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::_swprintf_p(szTest, _countof(szTest), L"%1$s%1$s%1$s%1$s0@example.com", szSeed); //4個繋げて64文字 + 1
	TEST20180909(FALSE, szTest, _countof(szTest) - 1, NULL);
}

TEST(testIsMailAddress, CheckMaxMailbox)
{
	wchar_t szTest[256];
	wchar_t szSeed64[64 + 1];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::_swprintf_p(szSeed64, _countof(szSeed64), L"%1$s%1$s%1$s%1$s", szSeed); //4個繋げて64文字にする
	::_swprintf_p(szTest, _countof(szTest), L"%1$s@%1$.63s.%1$.63s.%1$.58s.com", szSeed64); //最大255文字のチェック
	int mailboxLength;
	TEST20180909(TRUE, szTest, _countof(szTest) - 1, &mailboxLength);
	ASSERT_EQ(255, mailboxLength);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckMaxExceedMailbox)
{
	wchar_t szTest[256 + 1];
	wchar_t szSeed64[64 + 1];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::_swprintf_p(szSeed64, _countof(szSeed64), L"%1$s%1$s%1$s%1$s", szSeed); //4個繋げて64文字にする
	::_swprintf_p(szTest, _countof(szTest), L"%1$s@%1$.63s.%1$.63s.%1$.58s0.com", szSeed64); //最大255文字オーバーのチェック
	TEST20180909(FALSE, szTest, _countof(szTest) - 1, NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckTooLongDomain)
{
	wchar_t szTest[256];
	wchar_t szSeed64[64 + 1];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::_swprintf_p(szSeed64, _countof(szSeed64), L"%1$s%1$s%1$s%1$s", szSeed); //4個繋げて64文字にする
	::_swprintf_p(szTest, _countof(szTest), L"%1$s@%1$s.com", szSeed64); //63文字を超えるドメイン
	TEST20180909(FALSE, szTest, ::wcslen(szTest), NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckTooShortDomain)
{
	wchar_t szTest[] = L"yajim@my.me"; //ドメイン部は3文字以上
	TEST20180909(FALSE, szTest, _countof(szTest) - 1, NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckTooShortCCTLD)
{
	wchar_t szTest[] = L"test@test.c.bak"; //CCTLD部は2文字以上
	TEST20180909(FALSE, szTest, _countof(szTest) - 1, NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckDomainIncludesUnderScore)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::_swprintf_p(szTest, _countof(szTest), L"%1$s@test_domain.com", szSeed); //_を含むドメイン
	TEST20180909(FALSE, szTest, ::wcslen(szTest), NULL);
}

TEST(testIsMailAddress, CheckDomainIncludesSingleHyphen)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::_swprintf_p(szTest, _countof(szTest), L"%1$s@test-domain.com", szSeed); //途中に-を含むドメイン
	TEST20180909(TRUE, szTest, ::wcslen(szTest), NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckDomainIncludesDoubleHyphen)
{
	wchar_t szTest[256];
	wchar_t szSeed[] = L"0123456789ABCDEF"; // 16文字の素片
	::_swprintf_p(szTest, _countof(szTest), L"%1$s@test--domain.com", szSeed); //途中に-を含むドメイン
	TEST20180909(FALSE, szTest, ::wcslen(szTest), NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckQuotedLocalPart)
{
	wchar_t szTest[] = L"\"test\\@c\"@test.com";
	TEST20180909(TRUE, szTest, _countof(szTest) - 1, NULL);
}

// 動作変更あり。新実装では条件を厳しくして高速化している
TEST(testIsMailAddress, CheckBadQuotedLocalPart)
{
	wchar_t szTest[] = L"\"test@test.com";
	TEST20180909(FALSE, szTest, _countof(szTest) - 1, NULL);
}

// レビューコメントにより追試
TEST(testIsMailAddress, CheckAwithAtmark)
{
	wchar_t szTest[] = L"a@";
	TEST20180909(FALSE, szTest, _countof(szTest) - 1, NULL);
}


#pragma pop_macro("TEST20180909")

