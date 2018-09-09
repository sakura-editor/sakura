#include <gtest/gtest.h>

#include <cstdio>

#define NOMINMAX
#include <tchar.h>
#include <Windows.h>

// リンク依存関係を取り込まないためにダミーのマクロ定義をしておく
//#include "debug/Debug2.cpp"
#define debug_output(str, ...)
#define debug_exit2(file, line, exp)


// 独自シンボル USE_STRICT_INT を解決するために参照が必要
#include "config/build_config.h"

// 独自定義型 ACHAR を解決するために参照が必要
#include "basis/primitive.h"

// 独自定義型 CLogicInt を解決するために参照が必要
// これを先にインクルードしておく必要がある
#include "basis/SakuraBasis.h"

// テスト対象関数が元々あったヘッダファイル
#include "parse/CWordParse.h"

// テスト対象関数があるcppファイルを埋め込みで取り込む
// 他ファイルで同じファイルを取り込んではいけない
#include "util/string_ex.cpp"

// テストマクロが他と被らないようにpushする
#pragma push_macro("TEST20180909")

// このテストファイルローカルのテストモード切替フラグ
//   0 で新旧比較モード
//   1 で新の検証モード
#if 0
// 新旧動作比較用マクロ(TRUE, FALSE向け)
#define TEST20180909(pattern, buf, len, plen) \
	EXPECT_##pattern(IsMailAddress_20160427(buf, len, plen)); \
	ASSERT_##pattern(IsMailAddress(buf, len, plen));


// テスト対象の旧関数(コピペで埋め込み)
#pragma region ver20160427

/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す
	@date 2016.04.27 記号類を許可
*/
BOOL IsMailAddress_20160427( const wchar_t* pszBuf, int nBufLen, int* pnAddressLenfth )
{
	int		j;
	int		nDotCount;
	int		nBgn;


	j = 0;
	if( (pszBuf[j] >= L'a' && pszBuf[j] <= L'z')
	 || (pszBuf[j] >= L'A' && pszBuf[j] <= L'Z')
	 || (pszBuf[j] >= L'0' && pszBuf[j] <= L'9')
	 || NULL != wcschr(L"!#$%&'*+-/=?^_`{|}~", pszBuf[j])
	){
		j++;
	}else{
		return FALSE;
	}
	while( j < nBufLen - 2 &&
		(
		(pszBuf[j] >= L'a' && pszBuf[j] <= L'z')
	 || (pszBuf[j] >= L'A' && pszBuf[j] <= L'Z')
	 || (pszBuf[j] >= L'0' && pszBuf[j] <= L'9')
	 || (pszBuf[j] == L'.')
	 || NULL != wcschr(L"!#$%&'*+-/=?^_`{|}~", pszBuf[j])
		)
	){
		j++;
	}
	if( j == 0 || j >= nBufLen - 2  ){
		return FALSE;
	}
	if( L'@' != pszBuf[j] ){
		return FALSE;
	}
//	nAtPos = j;
	j++;
	nDotCount = 0;
//	nAlphaCount = 0;


	for (;;) {
		nBgn = j;
		while( j < nBufLen &&
			(
			(pszBuf[j] >= L'a' && pszBuf[j] <= L'z')
		 || (pszBuf[j] >= L'A' && pszBuf[j] <= L'Z')
		 || (pszBuf[j] >= L'0' && pszBuf[j] <= L'9')
		 || (pszBuf[j] == L'-')
		 || (pszBuf[j] == L'_')
			)
		){
			j++;
		}
		if( 0 == j - nBgn ){
			return FALSE;
		}
		if( L'.' != pszBuf[j] ){
			if( 0 == nDotCount ){
				return FALSE;
			}else{
				break;
			}
		}else{
			nDotCount++;
			j++;
		}
	}
	if( NULL != pnAddressLenfth ){
		*pnAddressLenfth = j;
	}
	return TRUE;
}

#pragma endregion ver20160427 definition


#else
// 新実装検証用マクロ(TRUE, FALSE向け)
#define TEST20180909(pattern, buf, len, plen) \
	ASSERT_##pattern(IsMailAddress(buf, len, plen));
#endif

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


#pragma pop_macro("TEST20180909")

