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
#include <string>
#include "charset/CCodeBase.h"

TEST(CCodeBase, MIMEHeaderDecode)
{
	CMemory m;

	// Base64 JIS
	std::string source1("From: =?iso-2022-jp?B?GyRCJTUlLyVpGyhC?=");
	EXPECT_TRUE(CCodeBase::MIMEHeaderDecode(source1.c_str(), source1.length(), &m, CODE_JIS));
	EXPECT_STREQ(static_cast<char*>(m.GetRawPtr()), "From: $B%5%/%i(B");

	// Base64 UTF-8
	std::string source2("From: =?utf-8?B?44K144Kv44Op?=");
	EXPECT_TRUE(CCodeBase::MIMEHeaderDecode(source2.c_str(), source2.length(), &m, CODE_UTF8));
	EXPECT_STREQ(static_cast<char*>(m.GetRawPtr()), "From: \xe3\x82\xb5\xe3\x82\xaf\xe3\x83\xa9");

	// QP UTF-8
	std::string source3("From: =?utf-8?Q?=E3=82=B5=E3=82=AF=E3=83=A9!?=");
	EXPECT_TRUE(CCodeBase::MIMEHeaderDecode(source3.c_str(), source3.length(), &m, CODE_UTF8));
	EXPECT_STREQ(static_cast<char*>(m.GetRawPtr()), "From: \xe3\x82\xb5\xe3\x82\xaf\xe3\x83\xa9!");

	// 引数の文字コードとヘッダー内の文字コードが異なる場合は変換しない
	EXPECT_TRUE(CCodeBase::MIMEHeaderDecode(source1.c_str(), source1.length(), &m, CODE_UTF8));
	EXPECT_STREQ(static_cast<char*>(m.GetRawPtr()), source1.c_str());

	// 対応していない文字コードなら変換しない
	std::string source4("From: =?utf-7?B?+MLUwrzDp-");
	EXPECT_TRUE(CCodeBase::MIMEHeaderDecode(source4.c_str(), source4.length(), &m, CODE_UTF7));
	EXPECT_STREQ(static_cast<char*>(m.GetRawPtr()), source4.c_str());

	// 謎の符号化方式が指定されていたら何もしない
	std::string source5("From: =?iso-2022-jp?X?GyRCJTUlLyVpGyhC?=");
	EXPECT_TRUE(CCodeBase::MIMEHeaderDecode(source5.c_str(), source5.length(), &m, CODE_JIS));
	EXPECT_STREQ(static_cast<char*>(m.GetRawPtr()), source5.c_str());

	// 末尾の ?= がなければ変換しない
	std::string source6("From: =?iso-2022-jp?B?GyRCJTUlLyVpGyhC");
	EXPECT_TRUE(CCodeBase::MIMEHeaderDecode(source6.c_str(), source6.length(), &m, CODE_JIS));
	EXPECT_STREQ(static_cast<char*>(m.GetRawPtr()), source6.c_str());
}
