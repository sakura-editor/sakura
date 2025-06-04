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
#include "convert/CDecode.h"

#include "testing/GuiAwareTestSuite.hpp"

#include "convert/CDecode_Base64Decode.h"
#include "convert/CDecode_UuDecode.h"

#include "mem/CNativeA.h"

namespace convert {

/*!
 * @brief デコードテストのためのフィクスチャクラス
 * @tparam T デコーダーの型
 * @tparam BaseTestSuiteType テストスイートの基底クラス
 */
template<typename T, typename BaseTestSuiteType>
struct TConvertTest : public testing::TGuiAware<BaseTestSuiteType> {
	using Base = testing::TGuiAware<BaseTestSuiteType>;

	T decoder;
	std::string dst;

	TConvertTest() = default;

	bool DoDecode(std::wstring_view source, std::string& destination)
	{
		CNativeW s(source);
		CNativeA d;
		const auto ret = decoder.DoDecode(s, d._GetMemory());
		if (ret) {
			destination = std::string_view(d);
		}
		return ret;
	}

	bool CallDecode(std::wstring_view source, std::string& destination)
	{
		// 最初にゴミを入れておく
		destination = "ゴミデータ";

		CNativeW s( source );
		CNativeA d(destination);
		const auto ret = decoder.CallDecode(s, d._GetMemory());

		// デコード成否に関わらず、データを書き戻す
		destination = std::string_view(d);

		return ret;
	}
};

//! デコードテストのためのテストパラメータ型
using DecoderTestParamType = std::tuple<std::wstring_view, std::string_view>;

//! デコードテストのためのフィクスチャクラス
using Base64Decoder = TConvertTest<CDecode_Base64Decode, ::testing::Test>;

//! デコードテストのためのフィクスチャクラス
using Base64DecoderP = TConvertTest<CDecode_Base64Decode, ::testing::TestWithParam<DecoderTestParamType>>;

TEST_P(Base64DecoderP, DoDecode001)
{
	const auto source = std::get<0>(GetParam());
	const auto expected = std::get<1>(GetParam());
	EXPECT_TRUE(DoDecode(source, dst));
	EXPECT_THAT(dst, StrEq(expected));
}

TEST_F(Base64Decoder, DoDecode002)
{
	// 空白は無視する
	EXPECT_TRUE(DoDecode(L"c2Fr \t dQ==", dst));
	EXPECT_THAT(dst, StrEq("saku"));
}

TEST_F(Base64Decoder, DoDecode101)
{
	// 異常な文字があったら変換を中止する
	EXPECT_FALSE(DoDecode(L"c2Fr?dQ==", dst));
}

TEST_F(Base64Decoder, CallDecode001)
{
	EXPECT_TRUE(CallDecode(L"c2FrdXJh", dst));
	EXPECT_THAT(dst, StrEq("sakura"));
}

TEST_F(Base64Decoder, CallDecode101)
{
	EXPECT_MSGBOX(CallDecode(L"c2Fr?dQ==", dst), GSTR_APPNAME, L"変換でエラーが発生しました");
	EXPECT_THAT(dst, IsEmpty());
}

/*!
 * @brief パラメータテストをインスタンス化する
 *  各変換機能の正常系をチェックするパターンで実体化させる
 */
INSTANTIATE_TEST_SUITE_P(Base64Decoder
	, Base64DecoderP
	, ::testing::Values(
		DecoderTestParamType{ L"",         "" },
		DecoderTestParamType{ L"cw==",     "s" },
		DecoderTestParamType{ L"c2E=",     "sa" },
		DecoderTestParamType{ L"c2Fr",     "sak" },
		DecoderTestParamType{ L"c2FrdQ==", "saku" },
		DecoderTestParamType{ L"c2FrdXI=", "sakur" },
		DecoderTestParamType{ L"c2FrdXJh", "sakura" }
	)
);

//! デコードテストのためのフィクスチャクラス
using UuDecoder  = TConvertTest<CDecode_UuDecode, ::testing::Test>;

//! デコードテストのためのフィクスチャクラス
using UuDecoderP = TConvertTest<CDecode_UuDecode, ::testing::TestWithParam<DecoderTestParamType>>;

TEST_P(UuDecoderP, DoDecode001)
{
	const auto source = std::get<0>(GetParam());
	const auto expected = std::get<1>(GetParam());
	EXPECT_TRUE(DoDecode(source, dst));
	EXPECT_THAT(dst, StrEq(expected));

	SFilePath fileName;
	decoder.CopyFilename(fileName);
	EXPECT_THAT(fileName, StrEq(L"test"));
}

TEST_F(UuDecoder, DoDecode002)
{
	// ヘッダーおよびフッターの先頭と末尾の空白は無視する
	EXPECT_TRUE(DoDecode(L"\tbegin 666 test \r\n!<P  \r\n \r\n\tend \r\n", dst));
	EXPECT_THAT(dst, StrEq("s"));
}

TEST_F(UuDecoder, DoDecode101)
{
	// 入力文字列が空の場合
	EXPECT_FALSE(DoDecode(L"", dst));
}

TEST_F(UuDecoder, DoDecode102)
{
	// 文字列が begin で始まっていない場合
	EXPECT_FALSE(DoDecode(L"benign 666 test\r\n!<P  \r\n \r\nend\r\n", dst));
}

TEST_F(UuDecoder, DoDecode103)
{
	// 文字列が begin で始まっていない場合
	EXPECT_FALSE(DoDecode(L"bigin 666 test\r\n!<P  \r\n \r\nend\r\n", dst));
}

TEST_F(UuDecoder, DoDecode104)
{
	// パーミッション設定が異常である場合
	EXPECT_FALSE(DoDecode(L"begin 66 test\r\n!<P  \r\n \r\nend\r\n", dst));
}

TEST_F(UuDecoder, DoDecode105)
{
	// パーミッション設定が異常である場合
	EXPECT_FALSE(DoDecode(L"begin 888 test\r\n!<P  \r\n \r\nend\r\n", dst));
}

TEST_F(UuDecoder, DoDecode106)
{
	// ファイル名が指定されていない場合
	EXPECT_FALSE(DoDecode(L"begin 666\r\n!<P  \r\n \r\nend\r\n", dst));
}

TEST_F(UuDecoder, DoDecode107)
{
	// ファイル名の長さが_MAX_PATHを超える場合
	std::wstring tooLongFileName(_MAX_PATH, L'a');
	EXPECT_FALSE(DoDecode(strprintf(L"begin 666 %s\r\n!<P  \r\n \r\nend\r\n", tooLongFileName.c_str()), dst));
}

TEST_F(UuDecoder, DoDecode108)
{
	// 改行コードがCRLFではない場合
	EXPECT_FALSE(DoDecode(L"begin 666 test\n!<P  \n \nend\n", dst));
}

TEST_F(UuDecoder, DoDecode109)
{
	// 妥当なヘッダー行の後にEOSが現れた場合
	EXPECT_FALSE(DoDecode(L"begin 666 test", dst));
}

TEST_F(UuDecoder, DoDecode110)
{
	// ボディに空行が含まれる場合
	EXPECT_FALSE(DoDecode(L"begin 666 test\r\n\r\n", dst));
}

TEST_F(UuDecoder, DoDecode111)
{
	// 文字列が end で終わっていない場合
	EXPECT_FALSE(DoDecode(L"begin 666 test\r\n!<P  \r\n \r\nen", dst));
}

TEST_F(UuDecoder, DoDecode112)
{
	// 文字列が end で終わっていない場合
	EXPECT_FALSE(DoDecode(L"begin 666 test\r\n!<P  \r\n \r\nned\r\n", dst));
}

TEST_F(UuDecoder, DoDecode113)
{
	// 文字列が end で終わっていない場合
	// end の後ろに空白以外の文字がある場合
	EXPECT_FALSE(DoDecode(L"begin 666 test\r\n!<P  \r\n \r\nendo\r\n", dst));
}

TEST_F(UuDecoder, CallDecode001)
{
	EXPECT_TRUE(CallDecode(L"begin 666 test\r\n&<V%K=7)A\r\n \r\nend\r\n", dst));
	EXPECT_THAT(dst, StrEq("sakura"));

	SFilePath fileName;
	decoder.CopyFilename(fileName);
	EXPECT_THAT(fileName, StrEq(L"test"));
}

TEST_F(UuDecoder, CallDecode101)
{
	EXPECT_MSGBOX(CallDecode(L"", dst), GSTR_APPNAME, L"変換でエラーが発生しました");
	EXPECT_THAT(dst, IsEmpty());
}

/*!
 * @brief パラメータテストをインスタンス化する
 *  各変換機能の正常系をチェックするパターンで実体化させる
 */
INSTANTIATE_TEST_SUITE_P(UuDecoder
	, UuDecoderP
	, ::testing::Values(
		DecoderTestParamType{ L"begin 666 test\r\n \r\nend\r\n",              "" },
		DecoderTestParamType{ L"begin 666 test\r\n!<P  \r\n \r\nend\r\n",     "s" },
		DecoderTestParamType{ L"begin 666 test\r\n\"<V$ \r\n \r\nend\r\n",    "sa" },
		DecoderTestParamType{ L"begin 666 test\r\n#<V%K\r\n \r\nend\r\n",     "sak" },
		DecoderTestParamType{ L"begin 666 test\r\n$<V%K=0  \r\n \r\nend\r\n", "saku" },
		DecoderTestParamType{ L"begin 666 test\r\n%<V%K=7( \r\n \r\nend\r\n", "sakur" },
		DecoderTestParamType{ L"begin 666 test\r\n&<V%K=7)A\r\n \r\nend\r\n", "sakura" }
	)
);

} // namespace convert
