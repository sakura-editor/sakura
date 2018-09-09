#include <gtest/gtest.h>

#include <cstdio>

#define NOMINMAX
#include <tchar.h>
#include <Windows.h>

// 独自定義型 ACHAR を解決するために参照が必要
#include "basis/primitive.h"

// テスト対象関数があるヘッダファイル
#include "parse/CWordParse.h"

// テスト対象関数(コピペで取込)
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
#pragma region ver20180909

// 指定された文字列がメールアドレス前半部分の要件を満たすか判定する
inline static bool IsMailAddressLocalPart(
	_In_z_ const wchar_t* pszStart,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszAtmark
) noexcept;

// 指定された文字列がメールアドレス後半部分の要件を満たすか判定する
inline static bool IsMailAddressDomain(
	_In_z_ const wchar_t* pszAtmark,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszEndOfMailBox
) noexcept;

/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す
	@date 2016.04.27 記号類を許可
	@date 2018.09.09 RFC準拠
*/
BOOL IsMailAddress( const wchar_t* pszBuf, int nBufLen, int* pnAddressLenfth )
{
	// RFC5321による mailbox の最大文字数
	const ptrdiff_t MAX_MAILBOX = 255; //255オクテット

	// バカ避け
	if (nBufLen < 1) return FALSE;

	// メールアドレスには必ず＠が含まれる
	const wchar_t* pszAtmark;

	// メールアドレス前半部分(＠の手前)をチェックする
	if (!IsMailAddressLocalPart(pszBuf, pszBuf + nBufLen, &pszAtmark)) {
		return FALSE;
	}
	assert(L'@' == *pszAtmark);

	// メールアドレスの終了位置を受け取るポインタを宣言する
	const wchar_t* pszEndOfMailBox;

	// メールアドレス後半部分(＠の後ろ)をチェックする
	if (!IsMailAddressDomain(pszAtmark, pszBuf + nBufLen, &pszEndOfMailBox))
	{
		return FALSE;
	}

	// 全体の長さが制限を超えていないかチェックする
	if (MAX_MAILBOX < pszEndOfMailBox - pszBuf)
	{
		return FALSE; // 文字数オーバー
	}

	if (pnAddressLenfth != nullptr)
	{
		*pnAddressLenfth = pszEndOfMailBox - pszBuf;
	}
	return TRUE;
}

/*!
 * 指定された文字列がメールアドレス前半部分の要件を満たすか判定する
 *
 * 高速化のため単純化した条件でチェックしている
 * 参照する標準は RFC5321
 * @see http://srgia.com/docs/rfc5321j.html
 */
inline static bool IsMailAddressLocalPart(
	_In_z_ const wchar_t* pszStart,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszAtmark
) noexcept
{
	// RFC5321による local-part の最大文字数
	const ptrdiff_t MAX_LOCAL_PART = 64; //64オクテット

	// 関数仕様
	assert(pszStart != pszEnd); // 長さ0の文字列をチェックしてはならない
	assert(pszStart < pszEnd); // 開始位置と終了位置は逆転してはならない

	// 出力値を初期化する
	*ppszAtmark = nullptr;

	// 文字列が二重引用符で始まっているかチェックして結果を保存
	const bool quoted = (L'"' == *pszStart);

	// ループ中にスキャンする文字位置を設定する
	auto pszScan = pszStart + (quoted ? 1 : 0);

	// スキャン位置が終端に達するまでループ
	while (pszScan < pszEnd)
	{
		switch (*pszScan)
		{
		case L'@':
			if (pszStart == pszScan)
			{
				return false; // local-partは1文字以上なのでNG
			}
			if (quoted)
			{
				return false; // 二重引用符で始まる場合、終端にも二重引用符が必要なのでNG
			}
			*ppszAtmark = pszScan;
			return true; // ここが正常終了
		case L'\\': // エスケープ記号
			if (pszScan + 1 == pszEnd || pszScan[1] < L'\x20' || L'\x7E' < pszScan[1])
			{
				return false;
			}
			pszScan++; // エスケープ記号の分1文字進める
			break;
		case L'"': // 二重引用符
			if (quoted && pszScan + 1 < pszEnd && L'@' == pszScan[1])
			{
				*ppszAtmark = &pszScan[1];
				return true; // ここは準正常終了。正常終了とはあえて区別しない。
			}
			return false; // 末尾以外に現れるエスケープされてない二重引用符は不正
		}
		pszScan++;
		if (MAX_LOCAL_PART < pszScan - pszStart)
		{
			return false; // 文字数オーバー
		}
	}
	return false;
}

/*!
 * 指定された文字列がメールアドレス後半部分の要件を満たすか判定する
 */
inline static bool IsMailAddressDomain(
	_In_z_ const wchar_t* pszAtmark,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszEndOfMailBox
) noexcept
{
	// ccTLDの最小文字数
	const ptrdiff_t MIN_TLD = 2;

	// ドメインの最小文字数
	const ptrdiff_t MIN_DOMAIN = 3;

	// ドメインの最大文字数
	const ptrdiff_t MAX_DOMAIN = 63;

	// 関数仕様
	assert(pszAtmark + 1 < pszEnd); // @位置と終了位置は逆転してはならない
	assert(L'@' == *pszAtmark); // @位置にある文字は@でなければならない

	// 出力値を初期化する
	*ppszEndOfMailBox = nullptr;

	// ループ中にスキャンする文字位置を設定する
	auto pszScan = pszAtmark + 1;

	auto dotCount = 0;
	auto domainLength = 0;
	auto prevHyphen = false;

	// スキャン位置が終端に達するまでループ
	while (pszScan < pszEnd)
	{
		switch (*pszScan)
		{
		case L'.': // ドット記号
			if (dotCount == 0 && domainLength < MIN_DOMAIN)
			{
				return false; // ドメイン名の最小文字数は3なのでNG
			}
			if (0 < dotCount && domainLength < MIN_TLD)
			{
				// これはco.jpなどを正しく認識させるために必要。
				return false; // ドットで区切られる部分の最小文字数は2なのでNG
			}
			if (prevHyphen)
			{
				return false; // ハイフンに続くドットはNG
			}
			dotCount++;
			domainLength = 0;
			prevHyphen = false;
			break;
		case L'-': // ハイフン記号
			if (domainLength == 0)
			{
				return false; // ドットに続くハイフンはNG
			}
			if (prevHyphen)
			{
				return false; // 連続するハイフンはNG
			}
			domainLength++;
			prevHyphen = true;
			break;
		default:
			if (dotCount == 0)
			{
				return false; // ドメイン部には一つ以上のドット記号が必要なのでNG
			}
			if (domainLength == 0)
			{
				return false; // ドットで終わるドメインはNG
			}
			if (prevHyphen)
			{
				return false; // ハイフンで終わるドメインはNG
			}
			*ppszEndOfMailBox = pszScan;
			return true; // ここも正常終了
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
		case L'G':
		case L'H':
		case L'I':
		case L'J':
		case L'K':
		case L'L':
		case L'M':
		case L'N':
		case L'O':
		case L'P':
		case L'Q':
		case L'R':
		case L'S':
		case L'T':
		case L'U':
		case L'V':
		case L'W':
		case L'X':
		case L'Y':
		case L'Z':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'g':
		case L'h':
		case L'i':
		case L'j':
		case L'k':
		case L'l':
		case L'm':
		case L'n':
		case L'o':
		case L'p':
		case L'q':
		case L'r':
		case L's':
		case L't':
		case L'u':
		case L'v':
		case L'w':
		case L'x':
		case L'y':
		case L'z':
			domainLength++;
			prevHyphen = false;
			break;
		}
		pszScan++;
		if (pszScan == pszEnd)
		{
			*ppszEndOfMailBox = pszScan;
			return true; // ここが正常終了
		}
		if (MAX_DOMAIN < domainLength)
		{
			return false; // 文字数オーバー
		}
	}
	return false;
}

#pragma endregion ver20180909 definition


// テストマクロが他と被らないようにpushする
#pragma push_macro("TEST20180909")

// このテストファイルローカルのテストモード切替フラグ
#if 0
// 新旧動作比較用マクロ(TRUE, FALSE向け)
#define TEST20180909(pattern, buf, len, plen) \
	EXPECT_##pattern(IsMailAddress_20160427(buf, len, plen)); \
	ASSERT_##pattern(IsMailAddress(buf, len, plen));

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

