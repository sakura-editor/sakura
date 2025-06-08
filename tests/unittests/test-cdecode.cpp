/*
	Copyright (C) 2021-2022, Sakura Editor Organization

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

#include "pch.h"
#include <cstdlib>
#include <string>
#include "convert/CDecode.h"
#include "convert/CDecode_Base64Decode.h"
#include "convert/CDecode_UuDecode.h"

struct DecoderTestCase {
	const wchar_t* input;
	const char* output;
};

TEST(CDecode, Base64)
{
	const DecoderTestCase testCases[] = {
		{L"", ""},
		{L"cw==", "s"},
		{L"c2E=", "sa"},
		{L"c2Fr", "sak"},
		{L"c2FrdQ==", "saku"},
		{L"c2FrdXI=", "sakur"},
		{L"c2FrdXJh", "sakura"}
	};
	CNativeW s;
	CMemory m;
	for (const auto& testCase : testCases) {
		s.SetString(testCase.input);
		EXPECT_TRUE(CDecode_Base64Decode().DoDecode(s, &m));
		EXPECT_STREQ(reinterpret_cast<char*>(m.GetRawPtr()), testCase.output);
	}

	// 空白は無視する
	s.SetString(L"c2Fr \t dQ==");
	EXPECT_TRUE(CDecode_Base64Decode().DoDecode(s, &m));
	EXPECT_STREQ(reinterpret_cast<char*>(m.GetRawPtr()), "saku");

	// 異常な文字があったら変換を中止する
	s.SetString(L"c2Fr?dQ==");
	EXPECT_FALSE(CDecode_Base64Decode().DoDecode(s, &m));
}

TEST(CDecode, uuencode)
{
	const DecoderTestCase testCases[] = {
		{L"begin 666 test\r\n \r\nend\r\n", ""},
		{L"begin 666 test\r\n!<P  \r\n \r\nend\r\n", "s"},
		{L"begin 666 test\r\n\"<V$ \r\n \r\nend\r\n", "sa"},
		{L"begin 666 test\r\n#<V%K\r\n \r\nend\r\n", "sak"},
		{L"begin 666 test\r\n$<V%K=0  \r\n \r\nend\r\n", "saku"},
		{L"begin 666 test\r\n%<V%K=7( \r\n \r\nend\r\n", "sakur"},
		{L"begin 666 test\r\n&<V%K=7)A\r\n \r\nend\r\n", "sakura"}
	};
	CNativeW s;
	CMemory m;
	for (const auto& testCase : testCases) {
		s.SetString(testCase.input);

		CDecode_UuDecode decoder;
		EXPECT_TRUE(decoder.DoDecode(s, &m));
		EXPECT_STREQ(reinterpret_cast<char*>(m.GetRawPtr()), testCase.output);

		wchar_t fileName[_MAX_PATH];
		decoder.CopyFilename(fileName);
		EXPECT_STREQ(fileName, L"test");
	}

	// ヘッダーおよびフッターの先頭と末尾の空白は無視する
	s.SetString(L"\tbegin 666 test \r\n!<P  \r\n \r\n\tend \r\n");
	EXPECT_TRUE(CDecode_UuDecode().DoDecode(s, &m));
	EXPECT_STREQ(reinterpret_cast<char*>(m.GetRawPtr()), "s");

	// 入力文字列が空の場合
	s.SetString(L"");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));

	// 文字列が begin で始まっていない場合
	s.SetString(L"benign 666 test\r\n!<P  \r\n \r\nend\r\n");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));
	s.SetString(L"bigin 666 test\r\n!<P  \r\n \r\nend\r\n");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));

	// パーミッション設定が異常である場合
	s.SetString(L"begin 66 test\r\n!<P  \r\n \r\nend\r\n");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));
	s.SetString(L"begin 888 test\r\n!<P  \r\n \r\nend\r\n");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));

	// ファイル名が指定されていない場合
	s.SetString(L"begin 666\r\n!<P  \r\n \r\nend\r\n");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));

	// ファイル名の長さが_MAX_PATHを超える場合
	std::wstring buffer;
	for (int i = 0; i <= _MAX_PATH; ++i)
		buffer.push_back(L'a');
	s.SetString(L"begin 666 ");
	s.AppendString(buffer.c_str());
	s.AppendString(L"\r\n!<P  \r\n \r\nend\r\n");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));

	// 改行コードがCRLFではない場合
	s.SetString(L"begin 666 test\n!<P  \n \nend\n");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));

	// 妥当なヘッダー行の後にEOSが現れた場合
	s.SetString(L"begin 666 test");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));

	// 文字列が end で終わっていない場合
	s.SetString(L"begin 666 test\r\n!<P  \r\n \r\nen");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));
	s.SetString(L"begin 666 test\r\n!<P  \r\n \r\nned\r\n");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));

	// end の後ろに空白以外の文字がある場合
	s.SetString(L"begin 666 test\r\n!<P  \r\n \r\nendo\r\n");
	EXPECT_FALSE(CDecode_UuDecode().DoDecode(s, &m));
}
