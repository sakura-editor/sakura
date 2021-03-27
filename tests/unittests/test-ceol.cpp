/*
	Copyright (C) 2021, Sakura Editor Organization

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
#include "CEol.h"

/*!
	CEolのテスト
 */
TEST(CEol, CEol)
{
	// 初期値は none 
	CEol cEol;
	EXPECT_EQ(EOL_NONE, cEol.GetType());
	EXPECT_EQ(EOL_NONE, (EEolType)cEol);

	// 代入したら変更できる
	cEol = EEolType::EOL_LF;
	EXPECT_EQ(EEolType::EOL_LF, cEol.GetType());

	// コピーの確認
	CEol cCopied = cEol;
	EXPECT_EQ(cEol.GetType(), cCopied.GetType());

	// EEolTypeは変な値でも格納できる
	EEolType eBadValue = static_cast<EEolType>(100);
	EXPECT_EQ(100, static_cast<int>(eBadValue));

	// CEolは変な値を格納できない（入れようとした場合CRLFになる）
	cEol = eBadValue;
	EXPECT_EQ(EEolType::EOL_CRLF, cEol.GetType());
}

/*!
	CEolのテスト
 */
TEST(CEol, typeNone)
{
	CEol cEol(EOL_NONE);

	EXPECT_EQ(EOL_NONE, cEol.GetType());
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
	CEol cEol(EOL_CRLF);

	EXPECT_EQ(EOL_CRLF, cEol.GetType());
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
	CEol cEol(EOL_LF);

	EXPECT_EQ(EOL_LF, cEol.GetType());
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
	CEol cEol(EOL_CR);

	EXPECT_EQ(EOL_CR, cEol.GetType());
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
	CEol cEol(EOL_NEL);

	EXPECT_EQ(EOL_NEL, cEol.GetType());
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
	CEol cEol(EOL_LS);

	EXPECT_EQ(EOL_LS, cEol.GetType());
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
	CEol cEol(EOL_PS);

	EXPECT_EQ(EOL_PS, cEol.GetType());
	EXPECT_EQ(1, cEol.GetLen());
	EXPECT_STREQ(L"PS", cEol.GetName());
	EXPECT_STREQ(L"\u2029", cEol.GetValue2());
	EXPECT_TRUE(cEol.IsValid());
}
