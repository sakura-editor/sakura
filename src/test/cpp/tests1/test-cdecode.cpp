/*
	Copyright (C) 2021-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "convert/CDecode.h"

#include "convert/CDecode_Base64Decode.h"
#include "convert/CDecode_UuDecode.h"

namespace convert {

/*!
 * デコーダーテストのパラメーター
 *
 * デコーダーはワイド文字列っぽいものを受け取り、バイト列を出力する。
 * 入力するワイド文字列によってはデコードが失敗する。
 * デコードの正否と期待値を同時に指定できるよう std::optional を使う。
 */
using DecoderTestParam = std::tuple<CNativeW, std::optional<std::string>>;

// Base64デコーダーのテスト
struct Base64DecodeTest : public ::testing::TestWithParam<DecoderTestParam> {
	CDecode_Base64Decode decoder;
};

TEST_P(Base64DecodeTest, DoDecode)
{
	const auto& input = std::get<0>(GetParam());
	const auto& expected = std::get<1>(GetParam());
	CMemory m;
	const auto result = decoder.DoDecode(input, &m);
	EXPECT_THAT((bool)result, expected.has_value());
	if (expected.has_value()) {
		EXPECT_THAT(LPCSTR(m.GetRawPtr()), StrEq(*expected));
	}
}

INSTANTIATE_TEST_SUITE_P(
	Base64Cases,
	Base64DecodeTest,
	::testing::Values(
		DecoderTestParam{ L"",             "" },
		DecoderTestParam{ L"cw==",         "s" },
		DecoderTestParam{ L"c2E=",         "sa" },
		DecoderTestParam{ L"c2Fr",         "sak" },
		DecoderTestParam{ L"c2FrdQ==",     "saku" },
		DecoderTestParam{ L"c2FrdXI=",     "sakur" },
		DecoderTestParam{ L"c2FrdXJh",     "sakura" },
		DecoderTestParam{ L"c2Fr \t dQ==", "saku" },		// 空白は無視する
		DecoderTestParam{ L"c2Fr?dQ==",    std::nullopt }	// 異常な文字があったら変換を中止する
	));

// UUデコーダーのテスト
struct UuDecodeTest : public ::testing::TestWithParam<DecoderTestParam> {
	CDecode_UuDecode decoder;
};

TEST_P(UuDecodeTest, DoDecode)
{
	const auto& input = std::get<0>( GetParam() );
	const auto& expected = std::get<1>(GetParam());
	CMemory m;
	const auto result = decoder.DoDecode(input, &m);
	EXPECT_THAT((bool)result, expected.has_value());
	if (expected.has_value()) {
		EXPECT_THAT(LPCSTR(m.GetRawPtr()), StrEq(*expected));
		// 正常に解釈できた場合、ファイル名もチェック。
		// （ファイル名は固定でtestを指定し、評価を簡略化している）
		SFilePath fileName;
		decoder.CopyFilename(static_cast<std::span<WCHAR, _MAX_PATH>>(fileName));
		EXPECT_THAT(fileName, StrEq(L"test"));
	}
}

INSTANTIATE_TEST_SUITE_P(
	UuDecodeCases,
	UuDecodeTest,
	::testing::Values(
		DecoderTestParam{ L"begin 666 test\r\n \r\nend\r\n",                "" },
		DecoderTestParam{ L"begin 666 test\r\n!<P  \r\n \r\nend\r\n",       "s" },
		DecoderTestParam{ L"begin 666 test\r\n\"<V$ \r\n \r\nend\r\n",      "sa" },
		DecoderTestParam{ L"begin 666 test\r\n#<V%K \r\n \r\nend\r\n",      "sak" },
		DecoderTestParam{ L"begin 666 test\r\n$<V%K=0 \r\n`\r\nend\r\n",    "saku" },
		DecoderTestParam{ L"begin 666 test\r\n%<V%K=7(\r\n~\r\nend\r\n",    "sakur" },
		DecoderTestParam{ L"begin 666 test\r\n&<V%K=7)A\r\n \r\nend\r\n",   "sakura" },
		DecoderTestParam{ L"  begin 666 test\r\n&<V%K=7)A\r\n \r\nend\r\n", "sakura" },		// 先頭の空白はスキップする
		DecoderTestParam{ L"",                                              std::nullopt },	// 入力文字列が空の場合
		DecoderTestParam{ L"benign 666 test\r\n!<P  \r\n \r\nend\r\n",      std::nullopt },	// 文字列が begin で始まっていない場合
		DecoderTestParam{ L"bigin 666 test\r\n!<P  \r\n \r\nend\r\n",       std::nullopt },	// 文字列が begin で始まっていない場合
		DecoderTestParam{ L"biginn 666 test\r\n!<P  \r\n \r\nend\r\n",      std::nullopt },	// 文字列が begin で始まっていない場合
		DecoderTestParam{ L"begin 66 test\r\n!<P  \r\n \r\nend\r\n",        std::nullopt },	// パーミッション設定が異常である場合（パーミッションは8進数3桁なので、2桁はNG。）
		DecoderTestParam{ L"begin 888 test\r\n!<P  \r\n \r\nend\r\n",       std::nullopt },	// パーミッション設定が異常である場合（パーミッションは8進数なので、8はNG。）
		DecoderTestParam{ L"begin 666\r\n!<P  \r\n \r\nend\r\n",            std::nullopt },	// ファイル名が指定されていない場合
		DecoderTestParam{ L"begin 666 test\n!<P  \n \nend\n",               std::nullopt },	// 改行コードがCRLFではない場合
		DecoderTestParam{ L"begin 666 test",                                std::nullopt },	// 妥当なヘッダー行の後にEOSが現れた場合
		DecoderTestParam{ L"begin 666 test\r\n!<P  \r\n\r\nend\r\n",        std::nullopt }, // 本文データに1文字未満の行がある場合
		DecoderTestParam{ L"begin 666 test\r\n!<P  \r\n \r\nen",            std::nullopt },	// 文字列が end で終わっていない場合
		DecoderTestParam{ L"begin 666 test\r\n!<P  \r\n \r\nned\r\n",       std::nullopt },	// 文字列が end で終わっていない場合
		DecoderTestParam{ L"begin 666 test\r\n!<P  \r\n \r\nendo\r\n",      std::nullopt },	// end の後ろに空白以外の文字がある場合

		DecoderTestParam{ CNativeW(std::format(L"begin 666 {:a<260}\r\n!<P  \r\n \r\nend\r\n", L'a').c_str()), std::nullopt }	// ファイル名の長さが_MAX_PATHを超える場合
	));

} // namespace convert
