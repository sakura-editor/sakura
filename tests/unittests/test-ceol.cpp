/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "CEol.h"

/*!
	CEolのテスト
 */
TEST(CEol, CEol)
{
	// 初期値は none 
	CEol cEol;
	EXPECT_EQ(EEolType::none, cEol.GetType());
	EXPECT_EQ(EEolType::none, (EEolType)cEol);

	// 代入したら変更できる
	cEol = EEolType::line_feed;
	EXPECT_EQ(EEolType::line_feed, cEol.GetType());

	// コピーの確認
	CEol cCopied = cEol;
	EXPECT_EQ(cEol.GetType(), cCopied.GetType());

	// EEolTypeは変な値でも格納できる
	EEolType eBadValue = static_cast<EEolType>(100);
	EXPECT_EQ(100, static_cast<int>(eBadValue));

	// CEolは変な値を格納できない（入れようとした場合CRLFになる）
	cEol = eBadValue;
	EXPECT_EQ(EEolType::cr_and_lf, cEol.GetType());
}

/*!
	CEolのテスト
 */
TEST(CEol, typeNone)
{
	CEol cEol(EEolType::none);

	EXPECT_EQ(EEolType::none, cEol.GetType());
	EXPECT_EQ(0, cEol.GetLen());
	EXPECT_STREQ(L"改行無", cEol.GetName());
	EXPECT_STREQ(L"", cEol.GetValue2());
	EXPECT_FALSE(cEol.IsValid());
}

/*!
	CEolのテスト
 */
TEST(CEol, typeCrlf)
{
	CEol cEol(EEolType::cr_and_lf);

	EXPECT_EQ(EEolType::cr_and_lf, cEol.GetType());
	EXPECT_EQ(2, cEol.GetLen());
	EXPECT_STREQ(L"CRLF", cEol.GetName());
	EXPECT_STREQ(L"\r\n", cEol.GetValue2());
	EXPECT_TRUE(cEol.IsValid());
}

/*!
	CEolのテスト
 */
TEST(CEol, typeLf)
{
	CEol cEol(EEolType::line_feed);

	EXPECT_EQ(EEolType::line_feed, cEol.GetType());
	EXPECT_EQ(1, cEol.GetLen());
	EXPECT_STREQ(L"LF", cEol.GetName());
	EXPECT_STREQ(L"\n", cEol.GetValue2());
	EXPECT_TRUE(cEol.IsValid());
}

/*!
	CEolのテスト
 */
TEST(CEol, typeCr)
{
	CEol cEol(EEolType::carriage_return);

	EXPECT_EQ(EEolType::carriage_return, cEol.GetType());
	EXPECT_EQ(1, cEol.GetLen());
	EXPECT_STREQ(L"CR", cEol.GetName());
	EXPECT_STREQ(L"\r", cEol.GetValue2());
	EXPECT_TRUE(cEol.IsValid());
}

/*!
	CEolのテスト
 */
TEST(CEol, typeNel)
{
	CEol cEol(EEolType::next_line);

	EXPECT_EQ(EEolType::next_line, cEol.GetType());
	EXPECT_EQ(1, cEol.GetLen());
	EXPECT_STREQ(L"NEL", cEol.GetName());
	EXPECT_STREQ(L"\x85", cEol.GetValue2());
	EXPECT_TRUE(cEol.IsValid());
}

/*!
	CEolのテスト
 */
TEST(CEol, typeLs)
{
	CEol cEol(EEolType::line_separator);

	EXPECT_EQ(EEolType::line_separator, cEol.GetType());
	EXPECT_EQ(1, cEol.GetLen());
	EXPECT_STREQ(L"LS", cEol.GetName());
	EXPECT_STREQ(L"\u2028", cEol.GetValue2());
	EXPECT_TRUE(cEol.IsValid());
}

/*!
	CEolのテスト
 */
TEST(CEol, typePs)
{
	CEol cEol(EEolType::paragraph_separator);

	EXPECT_EQ(EEolType::paragraph_separator, cEol.GetType());
	EXPECT_EQ(1, cEol.GetLen());
	EXPECT_STREQ(L"PS", cEol.GetName());
	EXPECT_STREQ(L"\u2029", cEol.GetValue2());
	EXPECT_TRUE(cEol.IsValid());
}
