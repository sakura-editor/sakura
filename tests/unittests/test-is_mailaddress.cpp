/*! @file */
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include <gtest/gtest.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif /* #ifndef NOMINMAX */

#include <tchar.h>
#include <wchar.h>
#include <assert.h>
#include <string>
#include <Windows.h>
#include "parse/CWordParse.h"

// テスト対象関数のヘッダファイル
//#include "util/string_ex.h" //依存関係が多いのでテスト対象の関数定義のみ抜き出し
BOOL IsMailAddress(const wchar_t *pszBuf, int nBufLen, int *pnAddressLength);

//////////////////////////////////////////////////////////////////////
// テストマクロ

// 新動作 = PR #421 導入によって変更されるはずだった IsMailAddress()
// 旧動作 = PR #421 導入前、revert 後の IsMailAddress()
//
// PR #421 導入によって IsMailAddress() の単体テストが実装されたが、
// PR #421 の revert によって消された単体テストを復活する。
// 将来 IsMailAddress() の仕様を変更する場合はこの単体テストを必要に応じて修正すればよい。

// 新旧動作比較用マクロ1(TRUE, FALSE向け)
// ASSERT_SAME: 旧実装と新実装で動作が変わらないことを期待
#define ASSERT_SAME(expected, szTarget, cchTarget, pchMatchedLen) \
    EXPECT_##expected(IsMailAddress(szTarget, cchTarget, pchMatchedLen))

// 新旧動作比較用マクロ2(TRUE, FALSE向け)
// ASSERT_CHANGE: 旧実装と新実装で動作が変わることを期待
#define ASSERT_CHANGE(expected, szTarget, cchTarget, pchMatchedLen) \
    EXPECT_NE(expected, IsMailAddress(szTarget, cchTarget, pchMatchedLen))

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

TEST(testIsMailAddress, OffsetParameter)
{
    /*
	   Prepare test cases.

	   ３つの offset値(-1, 0, 1)と、メールアドレスに見える２つの文字列
	   (Buffer+1=メールアドレスの先頭, Buffer+2=メールアドレスの途中)
	   の組み合わせにより定義する。
	*/
    const wchar_t *const Buffer    = L" test@example.com";
    const wchar_t *const BufferEnd = Buffer + wcslen(Buffer);
    const struct
    {
        bool expected;
        const wchar_t *address; // to be tested by IsMailAddress.
        int offset; // passed to IsMailAddress as 2nd param.
        const wchar_t *buffer() const
        { // passed to IsMailAddress as 1st param.
            return this->address - this->offset;
        }
    } testCases[] = {
        {true, Buffer + 1, 0}, // true is OK. Buffer+1 is a mail address.
        {true, Buffer + 1, 1}, // true is OK. Buffer+1 is a mail address.
        {true, Buffer + 1, -1}, // true is OK. Buffer+1 is a mail address.
        {true, Buffer + 2, 0}, // Limitation: Non positive offset prevents IsMailAddress from looking behind of a possible head of a mail address.
        {false, Buffer + 2, 1}, // false is OK. Buffer+2 is not a head of a mail adderss.
        {true, Buffer + 2, -1} // Limitation: Non positive offset prevents IsMailAddress from looking behind of a possible head of a mail address.
    };
    for (auto &aCase : testCases)
    {
        assert(Buffer <= aCase.buffer());
    }

    /*
	   Apply IsMailAddress to the cases.
	*/
    for (auto &aCase : testCases)
    {
        EXPECT_EQ(
            aCase.expected,
            bool(IsMailAddress(aCase.buffer(), aCase.offset, BufferEnd - aCase.buffer(), NULL)))
            << "1st param of IsMailAddress: pszBuf is \"" << aCase.buffer() << "\"\n"
            << "2nd param of IsMailAddress: offset is " << aCase.offset;
    }
}

TEST(testIsMailAddress, OffsetParameter2)
{
    const wchar_t *const Text             = L"   test@example.com   ";
    const wchar_t *const Address          = Text + 3; // Address is "test@example.com"
    const wchar_t *const PseudoAddress    = Text + 6; // PseudoAddress is "t@example.c", shortest form recognized by IsMailAddress.
    const wchar_t *const PseudoAddressEnd = Text + 17;
    const wchar_t *const AddressEnd       = Text + 19;
    const wchar_t *const TextEnd          = Text + 22;

    struct Result
    {
        bool is_address;
        int length;
    };
    const Result FalseResult = {false, 0};
    auto ExpectedResult      = [=](const wchar_t *p1, const wchar_t *p2, const wchar_t *p3) -> Result {
        /*
		   p2 と p3 が以下の条件を外れたら、TRUE 判定の可能性はゼロ。
		   * Address <= p2 <= PseudoAddress
		   * PseudoAddressEnd <= p3
		*/
        if (p2 < Address || PseudoAddress < p2)
        {
            return FalseResult;
        }
        if (p3 < PseudoAddressEnd)
        {
            return FalseResult;
        }

        if (p2 == Address)
        {
            if (AddressEnd <= p3)
            {
                return Result{true, static_cast<int>(AddressEnd - Address)}; // 文句なしの TRUE 判定。
            }
            else
            {
                return Result{true, static_cast<int>(p3 - Address)}; // アドレスの終端が切り詰められているが、IsMailAddress には知る由がない。ゆえに問題なし。
            }
        }
        else if (p2 <= p1)
        {
            if (AddressEnd <= p3)
            {
                return Result{true, static_cast<int>(AddressEnd - p2)}; // アドレスの先端が切り詰められているが、IsMailAddress には知る由がない。ゆえに問題なし。
            }
            else
            {
                return Result{true, static_cast<int>(p3 - p2)}; // アドレスの先端と終端が切り詰められているが、IsMailAddress には知る由がない。ゆえに問題なし。
            }
        }
        else
        {
            return FalseResult; // アドレスの先端が切り詰められ、IsMailAddress が境界判定によりそれを検知した。文句なしの FALSE 判定。
        }
    };
    auto IsEqualResult = [](const Result &expected, const Result &actual) -> testing::AssertionResult {
        if (expected.is_address != actual.is_address)
        {
            return testing::AssertionFailure() << "IsMailAddress returned " << (actual.is_address ? "TRUE" : "FALSE") << " but expected " << (expected.is_address ? "TRUE" : "FALSE") << ".";
        }
        if (expected.length != actual.length)
        {
            return testing::AssertionFailure() << "IsMailAddress returned the address length " << (actual.length) << " but expected " << (expected.length) << ".";
        }
        return testing::AssertionSuccess();
    };

    /*
	   Text...TextEnd の範囲の文字配列に対して、IsMailAddress の３つの
	   引数(pszBuf, offset, nBufLen)がとりうるすべての値を総当たりで試す。
	*/
    for (const wchar_t *p1 = Text; p1 != TextEnd; ++p1) // p1 is a pointer to buffer.
        for (const wchar_t *p2 = Text; p2 != TextEnd; ++p2) // p2 is a pointer to address.
            for (const wchar_t *p3 = Text; p3 != TextEnd; ++p3)
            { // p3 is a pointer to the end of buffer.
                Result actual     = {false, 0};
                actual.is_address = IsMailAddress(p1, p2 - p1, p3 - p1, &(actual.length));

                EXPECT_TRUE(IsEqualResult(ExpectedResult(p1, p2, p3), actual))
                    << "1st param of IsMailAddress: pszBuf is \"" << (p1 <= p3 ? std::wstring(p1, p3) : L"") << "\"\n"
                    << "2nd param of IsMailAddress: offset is " << (p2 - p1) << "\n"
                    << "pszBuf + offset is \"" << (p2 <= p3 ? std::wstring(p2, p3) : L"") << "\"";
            }
}

//////////////////////////////////////////////////////////////////////
// テストマクロの後始末

#undef ASSERT_SAME
#undef ASSERT_CHANGE
