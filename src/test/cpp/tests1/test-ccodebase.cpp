/*
	Copyright (C) 2021-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "charset/CCodeFactory.h"

#include "window/CMainStatusBar.h"

namespace convert {

std::string ToUtf16LeBytes(std::string_view ascii)
{
	std::string dest;
	dest.reserve(ascii.size() * 2);
	std::ranges::for_each(ascii, [&dest](char8_t c) {
		dest += LOBYTE(c);
		dest += '\0';
	});
	return dest;
}

std::string ToUtf16LeBytes(std::wstring_view wide)
{
	std::string dest;
	dest.reserve(wide.size() * 2);
	std::ranges::for_each(wide, [&dest](uchar16_t c) {
		dest += LOBYTE(c);
		dest += HIBYTE(c);
	});
	return dest;
}

std::string ToUtf16BeBytes(std::string_view ascii)
{
	std::string dest;
	dest.reserve(ascii.size() * 2);
	std::ranges::for_each(ascii, [&dest](char8_t c) {
		dest += '\0';
		dest += LOBYTE(c);
	});
	return dest;
}

std::string ToUtf16BeBytes(std::wstring_view wide)
{
	std::string dest;
	dest.reserve(wide.size() * 2);
	std::ranges::for_each(wide, [&dest](uchar16_t c) {
		dest += HIBYTE(c);
		dest += LOBYTE(c);
	});
	return dest;
}

std::string ToUtf32LeBytes(std::string_view ascii)
{
	std::string dest;
	dest.reserve(ascii.size() * 4);
	std::ranges::for_each(ascii, [&dest](uchar32_t c) {
		dest += LOBYTE(LOWORD(c));
		dest += HIBYTE(LOWORD(c));
		dest += LOBYTE(HIWORD(c));
		dest += HIBYTE(HIWORD(c));
	});
	return dest;
}

std::string ToUtf32LeBytes(std::wstring_view wide)
{
	std::string dest;
	dest.reserve(wide.size() * 2);
	std::ranges::for_each(wide, [&dest](uchar32_t c) {
		dest += LOBYTE(LOWORD(c));
		dest += HIBYTE(LOWORD(c));
		dest += LOBYTE(HIWORD(c));
		dest += HIBYTE(HIWORD(c));
	});
	return dest;
}

std::string ToUtf32BeBytes(std::string_view ascii)
{
	std::string dest;
	dest.reserve(ascii.size() * 4);
	std::ranges::for_each(ascii, [&dest](uchar32_t c) {
		dest += HIBYTE(HIWORD(c));
		dest += LOBYTE(HIWORD(c));
		dest += HIBYTE(LOWORD(c));
		dest += LOBYTE(LOWORD(c));
	});
	return dest;
}

std::string ToUtf32BeBytes(std::wstring_view wide)
{
	std::string dest;
	dest.reserve(wide.size() * 2);
	std::ranges::for_each(wide, [&dest](uchar32_t c) {
		dest += HIBYTE(HIWORD(c));
		dest += LOBYTE(HIWORD(c));
		dest += HIBYTE(LOWORD(c));
		dest += LOBYTE(LOWORD(c));
	});
	return dest;
}

/*!
 * @brief MIMEヘッダーデコードテストのパラメーター
 *
 * @param eCodeType 文字コードセット種別
 * @param input デコードする文字列
 * @param optExpected デコードされたバイト列の期待値
 */
using MIMEHeaderDecodeTestParam = std::tuple<ECodeType, std::string_view, std::optional<std::string>>;

//! MIMEヘッダーデコードテスト
struct MIMEHeaderDecodeTest : public ::testing::TestWithParam<MIMEHeaderDecodeTestParam> {};

TEST_P(MIMEHeaderDecodeTest, DoDecode)
{
	const auto  eCodeType   = std::get<0>(GetParam());
	const auto  input       = std::get<1>(GetParam());
	const auto& optExpected = std::get<2>(GetParam());

	CMemory m;
	const auto result = CCodeBase::MIMEHeaderDecode(std::data(input), std::size(input), &m, eCodeType);

	EXPECT_THAT((bool)result, optExpected.has_value());

	if (optExpected.has_value()) {
		const std::string_view decoded{ LPCSTR(m.GetRawPtr()), size_t(m.GetRawLength()) };
		EXPECT_THAT(decoded, StrEq(*optExpected));
	}
}

INSTANTIATE_TEST_SUITE_P(
	MIMEHeaderCases,
	MIMEHeaderDecodeTest,
	::testing::Values(
		MIMEHeaderDecodeTestParam{ CODE_JIS,  "From: =?iso-2022-jp?B?GyRCJTUlLyVpGyhC?=",       "From: $B%5%/%i(B" },							// Base64 JIS
		MIMEHeaderDecodeTestParam{ CODE_UTF8, "From: =?utf-8?B?44K144Kv44Op?=",                 "From: \xe3\x82\xb5\xe3\x82\xaf\xe3\x83\xa9" },		// Base64 UTF-8
		MIMEHeaderDecodeTestParam{ CODE_UTF8, "From: =?utf-8?Q?=E3=82=B5=E3=82=AF=E3=83=A9!?=", "From: \xe3\x82\xb5\xe3\x82\xaf\xe3\x83\xa9!" },	// Quoted Printable UTF-8
		MIMEHeaderDecodeTestParam{ CODE_UTF8, "From: =?iso-2022-jp?B?GyRCJTUlLyVpGyhC?=",       "From: =?iso-2022-jp?B?GyRCJTUlLyVpGyhC?=" },		// 引数の文字コードとヘッダー内の文字コードが異なる場合は変換しない
		MIMEHeaderDecodeTestParam{ CODE_UTF7, "From: =?utf-7?B?+MLUwrzDp-",                     "From: =?utf-7?B?+MLUwrzDp-" },						// 対応していない文字コードなら変換しない
		MIMEHeaderDecodeTestParam{ CODE_UTF8, "From: =?iso-2022-jp?X?GyRCJTUlLyVpGyhC?=",       "From: =?iso-2022-jp?X?GyRCJTUlLyVpGyhC?=" },		// 謎の符号化方式が指定されていたら何もしない
		MIMEHeaderDecodeTestParam{ CODE_JIS,  "From: =?iso-2022-jp?B?GyRCJTUlLyVpGyhC",         "From: =?iso-2022-jp?B?GyRCJTUlLyVpGyhC" }			// 末尾の ?= がなければ変換しない
	));

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeSJis)
{
	const auto eCodeType = CODE_SJIS;
	auto pCodeBase = CCodeFactory::CreateCodeBase( eCodeType );

	// 7bit ASCII範囲（等価変換）
	constexpr const auto& mbsAscii = "\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr const auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	auto result1 = CCodeFactory::LoadFromCode(eCodeType, mbsAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(mbsAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// かな漢字の変換（Shift-JIS仕様）
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";
	constexpr const auto& mbsKanaKanji = "\xB6\xC5\x82\xA9\x82\xC8\x83\x4A\x83\x69\x8A\xBF\x8E\x9A";

	auto result2 = CCodeFactory::LoadFromCode(eCodeType, mbsKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq(mbsKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());

	// Unicodeから変換できない文字（Shift-JIS仕様）
	// 1. SJIS⇒Unicode変換ができても、元に戻せない文字は変換失敗と看做す。
	//    該当するのは NEC選定IBM拡張文字 と呼ばれる約400字。
	// 2. 先行バイトが範囲外
	//    (ch1 >= 0x81 && ch1 <= 0x9F) ||
	//    (ch1 >= 0xE0 && ch1 <= 0xFC)
	// 3. 後続バイトが範囲外
	//    ch2 >= 0x40 &&  ch2 != 0xFC &&
	//    ch2 <= 0x7F
	constexpr const auto& mbsCantConvSJis =
		"\x87\x40\xED\x40\xFA\x40"					// "①纊ⅰ" NEC拡張、NEC選定IBM拡張、IBM拡張
		"\x80\x40\xFD\x40\xFE\x40\xFF\x40"			// 第1バイト不正
		"\x81\x0A\x81\x7F\x81\xFD\x81\xFE\x81\xFF"	// 第2バイト不正
		;
	constexpr const auto& wcsCantConvSJis =
		L"①\xDCED\xDC40ⅰ"
		L"\xDC80@\xDCFD@\xDCFE@\xDCFF@"
		L"\xDC81\n\xDC81\x7F\xDC81\xDCFD\xDC81\xDCFE\xDC81\xDCFF"
		;

	auto result3 = CCodeFactory::LoadFromCode(eCodeType, mbsCantConvSJis);
	EXPECT_THAT(result3.destination, StrEq(wcsCantConvSJis));
	EXPECT_THAT(result3, IsTrue());	//👈 仕様バグ。変換できないので true が返るべき。

	// Unicodeから変換できない文字（Shift-JIS仕様）
	constexpr const auto& wcsOGuy = L"森鷗外";
	constexpr const auto& mbsOGuy = "\x90\x58\x3F\x8A\x4F"; //森?外

	const auto cresult4 = CCodeFactory::ConvertToCode(eCodeType, wcsOGuy);
	EXPECT_THAT(cresult4.destination, StrEq(mbsOGuy));
	EXPECT_THAT(cresult4, IsFalse());
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeJis)
{
	const auto eCodeType = CODE_JIS;

	// 7bit ASCII範囲（ISO-2022-JP仕様）
	constexpr const auto& mbsAscii = "\x1\x2\x3\x4\x5\x6\a\b\t\n\v\f\r\xE\xF\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr const auto& wcsAscii = L"\x1\x2\x3\x4\x5\x6\a\b\t\n\v\f\r\xE\xF\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\xDC1B\xDC1C\xDC1D\xDC1E\xDC1F\xDC20!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	// 不具合1: 不正なエスケープシーケンスをエラーバイナリに格納してない
	// 不具合2: C0領域の文字 DEL(0x7F) を取り込めてない
	auto result1 = CCodeFactory::LoadFromCode(eCodeType, mbsAscii);
	EXPECT_THAT(result1.destination, StrEq(L"\x1\x2\x3\x4\x5\x6\a\b\t\n\v\f\r\xE\xF\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A???!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~?"));
	EXPECT_THAT(result1, IsFalse());	// 👈 不正なエスケープシーケンスを検出してるので戻り値は false で正しい。

	result1.destination = wcsAscii;

	// 不具合3: エラーバイナリを復元してない
	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq("\x1\x2\x3\x4\x5\x6\a\b\t\n\v\f\r\xE\xF\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A??????!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F"));
	EXPECT_THAT(cresult1, IsFalse());	// 👈 エラーバイナリの復元はエラーではないので true を返すべき。

	// かな漢字の変換（ISO-2022-JP仕様）
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";
	constexpr const auto& mbsKanaKanji = "\x1B(I6E\x1B$B$+$J%+%J4A;z\x1B(B";

	auto result2 = CCodeFactory::LoadFromCode(eCodeType, mbsKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq(mbsKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());

	// JIS範囲外（ありえない値を使う。Windows拡張があるため境界テスト不可。）
	EXPECT_THAT(CCodeFactory::LoadFromCode(eCodeType, "\x1B$B\xFF\xFF\x1B(B").result, RESULT_LOSESOME);

	// 不正なエスケープシーケンス（JIS X 0212には非対応。）
	EXPECT_THAT(CCodeFactory::LoadFromCode(eCodeType, "\x1B(D33\x1B(B").result, RESULT_LOSESOME);

	// Shift-Jisに変換できない文字
	EXPECT_THAT(CCodeFactory::ConvertToCode(eCodeType, L"\u9DD7").result, RESULT_LOSESOME);	// 森鴎外の鷗
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeEucJp)
{
	const auto eCodeType = CODE_EUC;
	auto pCodeBase = CCodeFactory::CreateCodeBase( eCodeType );

	// 7bit ASCII範囲（等価変換）
	constexpr const auto& mbsAscii = "\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr const auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	auto result1 = CCodeFactory::LoadFromCode(eCodeType, mbsAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(mbsAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// かな漢字の変換（EUC-JP仕様）
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";
	constexpr const auto& mbsKanaKanji = "\x8E\xB6\x8E\xC5\xA4\xAB\xA4\xCA\xA5\xAB\xA5\xCA\xB4\xC1\xBB\xFA";

	auto result2 = CCodeFactory::LoadFromCode(eCodeType, mbsKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq(mbsKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());

	// Unicodeから変換できない文字（EUC-JP仕様）
	// （保留）
	constexpr const auto& mbsCantConvEucJp =
		""	// 第1バイト不正
		""	// 第2バイト不正
		;
	constexpr const auto& wcsCantConvEucJp =
		L""
		L""
		;

	auto result3 = CCodeFactory::LoadFromCode(eCodeType, mbsCantConvEucJp);
	//ASSERT_THAT(result3.destination, StrEq(wcsCantConvEucJp));
	//ASSERT_THAT(result3.losesome, IsFalse());

	// Unicodeから変換できない文字（EUC-JP仕様）
	constexpr const auto& wcsOGuy = L"森鷗外";
	constexpr const auto& mbsOGuy = "\xBF\xB9\x3F\xB3\xB0"; //森?外

	// 本来のEUC-JPは「森鷗外」を正確に表現できるため、不具合と考えられる。
	//constexpr const auto& wcsOGuy = L"森鷗外";
	//constexpr const auto& mbsOGuy = "\xBF\xB9\x8F\xEC\x3F\xB3\xB0";

	const auto cresult4 = CCodeFactory::ConvertToCode(eCodeType, wcsOGuy);
	EXPECT_THAT(cresult4.destination, StrEq(mbsOGuy));
	EXPECT_THAT(cresult4, IsFalse());
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeLatin1)
{
	const auto eCodeType = CODE_LATIN1;

	// 7bit ASCII範囲（等価変換）
	constexpr const auto& mbsAscii = "\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr const auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	auto result1 = CCodeFactory::LoadFromCode(eCodeType, mbsAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(mbsAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// Latin1はかな漢字変換非サポートなので、0x80以上の変換できる文字をチェックする
	// 符号位置 81, 8D, 8F, 90, および 9D は未使用だが、そのまま出力される。
	constexpr const auto& wcsLatin1ExtChars =
		L"€\x81‚ƒ„…†‡ˆ‰Š‹Œ\x8DŽ\x8F"
		L"\x90‘’“”•–—˜™š›œ\x9DžŸ"
		L"\xA0¡¢£¤¥¦§¨©ª«¬\xAD®¯"
		L"°±²³´µ¶·¸¹º»¼½¾¿"
		L"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ"
		L"ÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß"
		L"àáâãäåæçèéêëìíîï"
		L"ðñòóôõö÷øùúûüýþÿ"
		;
	constexpr const auto& mbsLatin1ExtChars =
		"\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8A\x8B\x8C\x8D\x8E\x8F"
		"\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9A\x9B\x9C\x9D\x9E\x9F"
		"\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA\xAB\xAC\xAD\xAE\xAF"
		"\xB0\xB1\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9\xBA\xBB\xBC\xBD\xBE\xBF"
		"\xC0\xC1\xC2\xC3\xC4\xC5\xC6\xC7\xC8\xC9\xCA\xCB\xCC\xCD\xCE\xCF"
		"\xD0\xD1\xD2\xD3\xD4\xD5\xD6\xD7\xD8\xD9\xDA\xDB\xDC\xDD\xDE\xDF"
		"\xE0\xE1\xE2\xE3\xE4\xE5\xE6\xE7\xE8\xE9\xEA\xEB\xEC\xED\xEE\xEF"
		"\xF0\xF1\xF2\xF3\xF4\xF5\xF6\xF7\xF8\xF9\xFA\xFB\xFC\xFD\xFE\xFF"
		;

	auto result2 = CCodeFactory::LoadFromCode(eCodeType, mbsLatin1ExtChars);
	EXPECT_THAT(result2.destination, StrEq(wcsLatin1ExtChars));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq(mbsLatin1ExtChars));
	EXPECT_THAT(cresult2, IsTrue());

	// Unicodeに変換できない文字はない（Latin1仕様）

	// Unicodeから変換できない文字（Latin1仕様）
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";
	constexpr const auto& mbsKanaKanji = "????????";

	const auto cresult4 = CCodeFactory::ConvertToCode(eCodeType, wcsKanaKanji);
	EXPECT_THAT(cresult4.destination, StrEq(mbsKanaKanji));
	EXPECT_THAT(cresult4, IsFalse());
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeUtf8)
{
	const auto eCodeType = CODE_UTF8;
	auto pCodeBase = CCodeFactory::CreateCodeBase( eCodeType );

	// 7bit ASCII範囲（等価変換）
	constexpr const auto& mbsAscii = "\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr const auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	auto result1 = CCodeFactory::LoadFromCode(eCodeType, mbsAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(mbsAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// かな漢字の変換（UTF-8仕様）
	constexpr const auto& mbsKanaKanji = u8"ｶﾅかなカナ漢字";
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";

	auto result2 = CCodeFactory::LoadFromCode(eCodeType, (LPCSTR)mbsKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq((LPCSTR)mbsKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeUtf8_OracleImplementation)
{
	const auto eCodeType = CODE_CESU8;
	auto pCodeBase = CCodeFactory::CreateCodeBase( eCodeType );

	// 7bit ASCII範囲（等価変換）
	constexpr const auto& mbsAscii = "\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr const auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	auto result1 = CCodeFactory::LoadFromCode(eCodeType, mbsAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(mbsAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// かな漢字の変換（UTF-8仕様）
	constexpr const auto& mbsKanaKanji = u8"ｶﾅかなカナ漢字";
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";

	auto result2 = CCodeFactory::LoadFromCode(eCodeType, (LPCSTR)mbsKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq((LPCSTR)mbsKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeUtf7)
{
	const auto eCodeType = CODE_UTF7;

	// 7bit ASCII範囲（UTF-7仕様）
	constexpr const auto& mbsAscii = "+AAEAAgADAAQABQAGAAcACA-\t\n+AAsADA-\r+AA4ADwAQABEAEgATABQAFQAWABcAGAAZABoAGwAcAB0AHgAf- +ACEAIgAjACQAJQAm-'()+ACoAKw-,-./0123456789:+ADsAPAA9AD4-?+AEA-ABCDEFGHIJKLMNOPQRSTUVWXYZ+AFsAXABdAF4AXwBg-abcdefghijklmnopqrstuvwxyz+AHsAfAB9AH4Afw-";
	constexpr const auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	auto result1 = CCodeFactory::LoadFromCode(eCodeType, mbsAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(mbsAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// かな漢字の変換（UTF-7仕様）
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";
	constexpr const auto& mbsKanaKanji = "+/3b/hTBLMGowqzDKbyJbVw-";

	auto result2 = CCodeFactory::LoadFromCode(eCodeType, mbsKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq(mbsKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());

	// UTF-7仕様
	constexpr const auto& wcsPlusPlus = L"C++";
	constexpr const auto& mbsPlusPlus = "C+-+-";

	auto result5 = CCodeFactory::LoadFromCode(eCodeType, mbsPlusPlus);
	EXPECT_THAT(result5.destination, StrEq(wcsPlusPlus));
	EXPECT_THAT(result5, IsTrue());

	auto cresult5 = CCodeFactory::ConvertToCode(eCodeType, result5.destination);
	EXPECT_THAT(cresult5.destination, StrEq(mbsPlusPlus));
	EXPECT_THAT(cresult5, IsTrue());
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeUtf16Le)
{
	const auto eCodeType = CODE_UTF16LE;

	// 7bit ASCII範囲（等価変換）
	constexpr auto& mbsAscii = "\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	const auto bytesAscii = ToUtf16LeBytes(mbsAscii);
	auto result1 = CCodeFactory::LoadFromCode(eCodeType, bytesAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(bytesAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// かな漢字の変換（UTF-16LE仕様）
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";

	const auto bytesKanaKanji = ToUtf16LeBytes(wcsKanaKanji);
	auto result2 = CCodeFactory::LoadFromCode(eCodeType, bytesKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq(bytesKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeUtf16Be)
{
	const auto eCodeType = CODE_UTF16BE;

	// 7bit ASCII範囲（等価変換）
	constexpr auto& mbsAscii = "\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	const auto bytesAscii = ToUtf16BeBytes(mbsAscii);
	auto result1 = CCodeFactory::LoadFromCode(eCodeType, bytesAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(bytesAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// かな漢字の変換（UTF-16BE仕様）
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";

	const auto bytesKanaKanji = ToUtf16BeBytes(wcsKanaKanji);
	auto result2 = CCodeFactory::LoadFromCode(eCodeType, bytesKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq(bytesKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeUtf32Le)
{
	const auto eCodeType = CODE_UTF32LE;

	// 7bit ASCII範囲（等価変換）
	constexpr auto& mbsAscii = "\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	const auto bytesAscii = ToUtf32LeBytes(mbsAscii);
	auto result1 = CCodeFactory::LoadFromCode(eCodeType, bytesAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(bytesAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// かな漢字の変換（UTF-32LE仕様）
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";

	const auto bytesKanaKanji = ToUtf32LeBytes(wcsKanaKanji);
	auto result2 = CCodeFactory::LoadFromCode(eCodeType, bytesKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq(bytesKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());
}

/*!
 * @brief 文字コード変換のテスト
 */
TEST(CCodeBase, codeUtf32Be)
{
	const auto eCodeType = CODE_UTF32BE;

	// 7bit ASCII範囲（等価変換）
	constexpr auto& mbsAscii = "\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";
	constexpr auto& wcsAscii = L"\x01\x02\x03\x04\x05\x06\a\b\t\n\v\f\r\x0E\x0F\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1A\x1B\x1C\x1D\x1E\x1F !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\x7F";

	const auto bytesAscii = ToUtf32BeBytes(mbsAscii);
	auto result1 = CCodeFactory::LoadFromCode(eCodeType, bytesAscii);
	EXPECT_THAT(result1.destination, StrEq(wcsAscii));
	EXPECT_THAT(result1, IsTrue());

	auto cresult1 = CCodeFactory::ConvertToCode(eCodeType, result1.destination);
	EXPECT_THAT(cresult1.destination, StrEq(bytesAscii));
	EXPECT_THAT(cresult1, IsTrue());

	// かな漢字の変換（UTF-32BE仕様）
	constexpr const auto& wcsKanaKanji = L"ｶﾅかなカナ漢字";

	const auto bytesKanaKanji = ToUtf32BeBytes(wcsKanaKanji);
	auto result2 = CCodeFactory::LoadFromCode(eCodeType, bytesKanaKanji);
	EXPECT_THAT(result2.destination, StrEq(wcsKanaKanji));
	EXPECT_THAT(result2, IsTrue());

	auto cresult2 = CCodeFactory::ConvertToCode(eCodeType, result2.destination);
	EXPECT_THAT(cresult2.destination, StrEq(bytesKanaKanji));
	EXPECT_THAT(cresult2, IsTrue());
}

/*!
 * @brief GetBomテストのパラメーター
 *
 * @param eCodeType 文字コードセット種別
 * @param optExpected 期待されるBOMのバイナリ表現
 */
using GetBomTestParam = std::tuple<ECodeType, std::optional<std::string>>;

//! GetBomテストのためのフィクスチャクラス
class GetBomTest : public ::testing::TestWithParam<GetBomTestParam> {};

/*!
 * @brief GetBomのテスト
 */
TEST_P(GetBomTest, test) {
	const auto  eCodeType   = std::get<0>(GetParam());
	const auto& optExpected = std::get<1>(GetParam());

	const auto pCodeBase = CCodeFactory::CreateCodeBase(eCodeType);

	CMemory m;
	pCodeBase->GetBom(&m);

	if (optExpected.has_value()) {
		const std::string_view bom{ LPCSTR(m.GetRawPtr()), size_t(m.GetRawLength()) };
		EXPECT_THAT(bom, StrEq(*optExpected));
	} else {
		EXPECT_THAT(LPCSTR(m.GetRawPtr()), IsNull());
	}
}

/*!
 * @brief パラメータテストをインスタンス化する
 */
INSTANTIATE_TEST_SUITE_P(GetBomCases
	, GetBomTest
	, ::testing::Values(
		GetBomTestParam{ CODE_SJIS,		{} },			// 非Unicodeなので実施する意味はない
		GetBomTestParam{ CODE_JIS,		{} },			// 非Unicodeなので実施する意味はない
		GetBomTestParam{ CODE_EUC,		{} },			// 非Unicodeなので実施する意味はない
		GetBomTestParam{ CODE_LATIN1,	{} },			// 非Unicodeなので実施する意味はない
		GetBomTestParam{ CODE_UTF7,		"+/v8-" },		// 対象外なので実施する意味はない
		GetBomTestParam{ CODE_UTF16LE,	"\xFF\xFE" },
		GetBomTestParam{ CODE_UTF16BE,	"\xFE\xFF" },
		GetBomTestParam{ CODE_UTF32LE,	std::string{ "\xFF\xFE\0\0", 4 } },
		GetBomTestParam{ CODE_UTF32BE,	std::string{ "\0\0\xFE\xFF", 4 } },
		GetBomTestParam{ CODE_UTF8,		"\xEF\xBB\xBF" },
		GetBomTestParam{ CODE_CESU8,	"\xEF\xBB\xBF" }
	)
);

/*!
 * @brief GetEolテストのパラメーター
 *
 * @param eCodeType 文字コードセット種別
 * @param eEolType 行終端子種別
 * @param optExpected 期待される行終端子のバイナリ表現
 */
using GetEolTestParam = std::tuple<ECodeType, EEolType, std::optional<std::string>>;

//! GetEolテストのためのフィクスチャクラス
class GetEolTest : public ::testing::TestWithParam<GetEolTestParam> {};

/*!
 * @brief GetEolのテスト
 */
TEST_P(GetEolTest, test)
{
	const auto  eCodeType   = std::get<0>(GetParam());
	const auto  eEolType    = std::get<1>(GetParam());
	const auto& optExpected = std::get<2>(GetParam());

	const auto pCodeBase = CCodeFactory::CreateCodeBase(eCodeType);

	CMemory m;
	pCodeBase->GetEol(&m, eEolType);

	if (optExpected.has_value()) {
		const std::string_view eol{ LPCSTR(m.GetRawPtr()), size_t(m.GetRawLength()) };
		EXPECT_THAT(eol, StrEq(*optExpected));
	} else {
		EXPECT_THAT(LPCSTR(m.GetRawPtr()), IsNull());
	}
}

/*!
 * @brief パラメータテストをインスタンス化する
 */
INSTANTIATE_TEST_SUITE_P(GetEolCases
	, GetEolTest
	, ::testing::Values(
		GetEolTestParam{ CODE_SJIS,    EEolType::none,                {}     },
		GetEolTestParam{ CODE_SJIS,    EEolType::cr_and_lf,           "\r\n" },
		GetEolTestParam{ CODE_SJIS,    EEolType::line_feed,           "\n"   },
		GetEolTestParam{ CODE_SJIS,    EEolType::carriage_return,     "\r"   },
		GetEolTestParam{ CODE_SJIS,    EEolType::next_line,           {}     },
		GetEolTestParam{ CODE_SJIS,    EEolType::line_separator,      {}     },
		GetEolTestParam{ CODE_SJIS,    EEolType::paragraph_separator, {}     },

		GetEolTestParam{ CODE_JIS,     EEolType::none,                {}     },
		GetEolTestParam{ CODE_JIS,     EEolType::cr_and_lf,           "\r\n" },
		GetEolTestParam{ CODE_JIS,     EEolType::line_feed,           "\n"   },
		GetEolTestParam{ CODE_JIS,     EEolType::carriage_return,     "\r"   },
		GetEolTestParam{ CODE_JIS,     EEolType::next_line,           {}     },
		GetEolTestParam{ CODE_JIS,     EEolType::line_separator,      {}     },
		GetEolTestParam{ CODE_JIS,     EEolType::paragraph_separator, {}     },

		GetEolTestParam{ CODE_EUC,     EEolType::none,                {}     },
		GetEolTestParam{ CODE_EUC,     EEolType::cr_and_lf,           "\r\n" },
		GetEolTestParam{ CODE_EUC,     EEolType::line_feed,           "\n"   },
		GetEolTestParam{ CODE_EUC,     EEolType::carriage_return,     "\r"   },
		GetEolTestParam{ CODE_EUC,     EEolType::next_line,           {}     },
		GetEolTestParam{ CODE_EUC,     EEolType::line_separator,      {}     },
		GetEolTestParam{ CODE_EUC,     EEolType::paragraph_separator, {}     },

		GetEolTestParam{ CODE_LATIN1,  EEolType::none,                {}     },
		GetEolTestParam{ CODE_LATIN1,  EEolType::cr_and_lf,           "\r\n" },
		GetEolTestParam{ CODE_LATIN1,  EEolType::line_feed,           "\n"   },
		GetEolTestParam{ CODE_LATIN1,  EEolType::carriage_return,     "\r"   },
		GetEolTestParam{ CODE_LATIN1,  EEolType::next_line,           {}     },
		GetEolTestParam{ CODE_LATIN1,  EEolType::line_separator,      {}     },
		GetEolTestParam{ CODE_LATIN1,  EEolType::paragraph_separator, {}     },

		GetEolTestParam{ CODE_UTF7,    EEolType::none,                {}      },
		GetEolTestParam{ CODE_UTF7,    EEolType::cr_and_lf,           "\r\n"  },
		GetEolTestParam{ CODE_UTF7,    EEolType::line_feed,           "\n"    },
		GetEolTestParam{ CODE_UTF7,    EEolType::carriage_return,     "\r"    },
		GetEolTestParam{ CODE_UTF7,    EEolType::next_line,           "+AIU-" },	// UTF-7 には Set B 文字をBase64エンコードする仕様があるので、このテストに意味はない。
		GetEolTestParam{ CODE_UTF7,    EEolType::line_separator,      "+ICg-" },	// UTF-7 には Set B 文字をBase64エンコードする仕様があるので、このテストに意味はない。
		GetEolTestParam{ CODE_UTF7,    EEolType::paragraph_separator, "+ICk-" },	// UTF-7 には Set B 文字をBase64エンコードする仕様があるので、このテストに意味はない。

		GetEolTestParam{ CODE_UTF16LE, EEolType::none,                {}                                },
		GetEolTestParam{ CODE_UTF16LE, EEolType::cr_and_lf,           std::string_view{ "\r\0\n\0", 4 } },
		GetEolTestParam{ CODE_UTF16LE, EEolType::line_feed,           std::string_view{ "\n\0", 2 }     },
		GetEolTestParam{ CODE_UTF16LE, EEolType::carriage_return,     std::string_view{ "\r\0", 2 }     },
		GetEolTestParam{ CODE_UTF16LE, EEolType::next_line,           std::string_view{ "\x85\0", 2 }   },
		GetEolTestParam{ CODE_UTF16LE, EEolType::line_separator,      std::string_view{ "\x28\x20", 2 } },
		GetEolTestParam{ CODE_UTF16LE, EEolType::paragraph_separator, std::string_view{ "\x29\x20", 2 } },

		GetEolTestParam{ CODE_UTF16BE, EEolType::none,                {}                                },
		GetEolTestParam{ CODE_UTF16BE, EEolType::cr_and_lf,           std::string_view{ "\0\r\0\n", 4 } },
		GetEolTestParam{ CODE_UTF16BE, EEolType::line_feed,           std::string_view{ "\0\n", 2 }     },
		GetEolTestParam{ CODE_UTF16BE, EEolType::carriage_return,     std::string_view{ "\0\r", 2 }     },
		GetEolTestParam{ CODE_UTF16BE, EEolType::next_line,           std::string_view{ "\0\x85", 2 }   },
		GetEolTestParam{ CODE_UTF16BE, EEolType::line_separator,      std::string_view{ "\x20\x28", 2 } },
		GetEolTestParam{ CODE_UTF16BE, EEolType::paragraph_separator, std::string_view{ "\x20\x29", 2 } },

		GetEolTestParam{ CODE_UTF32LE, EEolType::none,                {}                                        },
		GetEolTestParam{ CODE_UTF32LE, EEolType::cr_and_lf,           std::string_view{ "\r\0\0\0\n\0\0\0", 8 } },
		GetEolTestParam{ CODE_UTF32LE, EEolType::line_feed,           std::string_view{ "\n\0\0\0", 4 }         },
		GetEolTestParam{ CODE_UTF32LE, EEolType::carriage_return,     std::string_view{ "\r\0\0\0", 4 }         },
		GetEolTestParam{ CODE_UTF32LE, EEolType::next_line,           std::string_view{ "\x85\0\0\0", 4 }       },
		GetEolTestParam{ CODE_UTF32LE, EEolType::line_separator,      std::string_view{ "\x28\x20\0\0", 4 }     },
		GetEolTestParam{ CODE_UTF32LE, EEolType::paragraph_separator, std::string_view{ "\x29\x20\0\0", 4 }     },

		GetEolTestParam{ CODE_UTF32BE, EEolType::none,                {}                                        },
		GetEolTestParam{ CODE_UTF32BE, EEolType::cr_and_lf,           std::string_view{ "\0\0\0\r\0\0\0\n", 8 } },
		GetEolTestParam{ CODE_UTF32BE, EEolType::line_feed,           std::string_view{ "\0\0\0\n", 4 }         },
		GetEolTestParam{ CODE_UTF32BE, EEolType::carriage_return,     std::string_view{ "\0\0\0\r", 4 }         },
		GetEolTestParam{ CODE_UTF32BE, EEolType::next_line,           std::string_view{ "\0\0\0\x85", 4 }       },
		GetEolTestParam{ CODE_UTF32BE, EEolType::line_separator,      std::string_view{ "\0\0\x20\x28", 4 }     },
		GetEolTestParam{ CODE_UTF32BE, EEolType::paragraph_separator, std::string_view{ "\0\0\x20\x29", 4 }     },

		GetEolTestParam{ CODE_UTF8,    EEolType::none,                {}             },
		GetEolTestParam{ CODE_UTF8,    EEolType::cr_and_lf,           "\r\n"         },
		GetEolTestParam{ CODE_UTF8,    EEolType::line_feed,           "\n"           },
		GetEolTestParam{ CODE_UTF8,    EEolType::carriage_return,     "\r"           },
		GetEolTestParam{ CODE_UTF8,    EEolType::next_line,           "\xC2\x85"     },
		GetEolTestParam{ CODE_UTF8,    EEolType::line_separator,      "\xE2\x80\xA8" },
		GetEolTestParam{ CODE_UTF8,    EEolType::paragraph_separator, "\xE2\x80\xA9" },

		GetEolTestParam{ CODE_CESU8,   EEolType::none,                {}             },
		GetEolTestParam{ CODE_CESU8,   EEolType::cr_and_lf,           "\r\n"         },
		GetEolTestParam{ CODE_CESU8,   EEolType::line_feed,           "\n"           },
		GetEolTestParam{ CODE_CESU8,   EEolType::carriage_return,     "\r"           },
		GetEolTestParam{ CODE_CESU8,   EEolType::next_line,           "\xC2\x85"     },
		GetEolTestParam{ CODE_CESU8,   EEolType::line_separator,      "\xE2\x80\xA8" },
		GetEolTestParam{ CODE_CESU8,   EEolType::paragraph_separator, "\xE2\x80\xA9" }
	)
);

} // namespace  convert

namespace window {

//! 表示設定種別
enum class ESettingType : int8_t {
	Default,
	DispCodepoint
};

//!googletestにESettingTypeを出力させる
void PrintTo(ESettingType eSettingType, std::ostream* os)
{
	switch (eSettingType) {
	case ESettingType::Default:       *os << "Default"; break;
	case ESettingType::DispCodepoint: *os << "DispCodepoint"; break;

	default:
		// 未知の値は数値で出す
		*os << std::format("ESettingType({})", static_cast<uint16_t>(eSettingType));
		break;
	}
}

/*!
 * @brief 表示用16進変換テストのパラメーター
 *
 * @param eSettingType 表示設定種別
 * @param eCodeType 文字コードセット種別
 * @param caretChars キャレット位置の文字列
 * @param expected 期待される表示用16進変換の結果
 */
using UnicoceToHexTestParam = std::tuple<ESettingType, ECodeType, std::wstring, std::wstring>;

//! 表示用16進変換テストのためのフィクスチャクラス
struct UnicodeToHexTest : public ::testing::TestWithParam<UnicoceToHexTestParam> {
	// Unicodeコードポイントを表示する設定
	const CommonSetting_Statusbar sStatusbar0{
		false,	// m_bDispUniInSjis
		false,	// m_bDispUniInJis
		false,	// m_bDispUniInEuc
		false,	// m_bDispUtf8Codepoint
		false	// m_bDispSPCodepoint
	};

	// Unicodeコードポイントを表示する設定
	const CommonSetting_Statusbar sStatusbar1{
		true,	// m_bDispUniInSjis
		true,	// m_bDispUniInJis
		true,	// m_bDispUniInEuc
		true,	// m_bDispUtf8Codepoint
		true	// m_bDispSPCodepoint
	};
};

/*!
 * @brief UnicodeToHexのテスト
 */
TEST_P(UnicodeToHexTest, test)
{
	const auto  eSettingType = std::get<0>(GetParam());
	const auto  eCodeType    = std::get<1>(GetParam());
	const auto& caretChars   = std::get<2>(GetParam());
	const auto& expected     = std::get<3>(GetParam());

	CommonSetting_Statusbar sStatusbar{ eSettingType == ESettingType::Default ? sStatusbar0 : sStatusbar1 };

	EXPECT_THAT(CMainStatusBar::UnicodeToHex(eCodeType, caretChars, sStatusbar), StrEq(expected));
}

/*!
 * @brief パラメータテストをインスタンス化する
 */
INSTANTIATE_TEST_SUITE_P(UnicodeToHexCases
	, UnicodeToHexTest
	, ::testing::Values(
		// ASCII文字「J」
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_SJIS,    L"J", L"4A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_JIS,     L"J", L"4A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_EUC,     L"J", L"4A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16LE, L"J", L"U+004A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16BE, L"J", L"U+004A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32LE, L"J", L"U+004A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32BE, L"J", L"U+004A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF8,    L"J", L"4A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF7,    L"J", L"U+004A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_CESU8,   L"J", L"4A" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_LATIN1,  L"J", L"4a" },		// Latin1だけ英字が小文字。（おそらく仕様。）

		// 空文字
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_SJIS,    L"", L"00" },			// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_JIS,     L"", L"00" },			// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_EUC,     L"", L"00" },			// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16LE, L"", L"U+0000" },		// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16BE, L"", L"U+0000" },		// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32LE, L"", L"U+0000" },		// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32BE, L"", L"U+0000" },		// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF8,    L"", L"00" },			// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF7,    L"", L"U+0000" },		// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_CESU8,   L"", L"00" },			// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_LATIN1,  L"", L"00" },			// 👈バグ。サイズゼロなので終端NULにアクセスしてはならない。

		// コードページでサポートされない文字「鷗」（SJISで定義されていない）
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_SJIS,    L"鷗", L"U+9DD7" },

		// コードページでサポートされない文字「鷗」（JIS X 0212には非対応）
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_JIS,     L"鷗", L"U+9DD7" },

		// コードページでサポートされない文字「鷗」（補助漢字には非対応）
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_EUC,     L"鷗", L"U+9DD7" },

		// エラーバイナリ
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_SJIS,    L"\xDCEF", L"?EF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_JIS,     L"\xDCEF", L"U+DCEF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_EUC,     L"\xDCEF", L"?EF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16LE, L"\xDCEF", L"U+DCEF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16BE, L"\xDCEF", L"U+DCEF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32LE, L"\xDCEF", L"U+DCEF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32BE, L"\xDCEF", L"U+DCEF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF8,    L"\xDCEF", L"?EF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF7,    L"\xDCEF", L"U+DCEF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_CESU8,   L"\xDCEF", L"?EF" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_LATIN1,  L"\xDCEF", L"?ef" },		// Latin1だけ英字が小文字。（おそらく仕様。）

		// 日本語 ひらがな「あ」
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_SJIS,    L"あ", L"82A0" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_JIS,     L"あ", L"2422" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_EUC,     L"あ", L"A4A2" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16LE, L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16BE, L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32LE, L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32BE, L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF8,    L"あ", L"E38182" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF7,    L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_CESU8,   L"あ", L"E38182" },
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_LATIN1,  L"あ", L"U+3042" },

		// 日本語 ひらがな「あ」（コードポイント表示なら文字セットがサポートしない文字でも統一仕様）
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_SJIS,    L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_JIS,     L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_EUC,     L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF16LE, L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF16BE, L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF32LE, L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF32BE, L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF8,    L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF7,    L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_CESU8,   L"あ", L"U+3042" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_LATIN1,  L"あ", L"U+3042" },

		// サロゲートペア：カラー絵文字「男性のシンボル」
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_SJIS,    L"\U0001F6B9", L"D83DDEB9" },		// 👈バグ。コードページがサポートしない文字なので、コードポイントで表示すべき。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_JIS,     L"\U0001F6B9", L"D83DDEB9" },		// 👈バグ。コードページがサポートしない文字なので、コードポイントで表示すべき。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_EUC,     L"\U0001F6B9", L"D83DDEB9" },		// 👈バグ。コードページがサポートしない文字なので、コードポイントで表示すべき。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16LE, L"\U0001F6B9", L"D83DDEB9" },		// 2文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16BE, L"\U0001F6B9", L"D83DDEB9" },		// 2文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32LE, L"\U0001F6B9", L"D83DDEB9" },		// 2文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32BE, L"\U0001F6B9", L"D83DDEB9" },		// 2文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF8,    L"\U0001F6B9", L"F09F9AB9" },		// 2文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF7,    L"\U0001F6B9", L"D83DDEB9" },		// 2文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_CESU8,   L"\U0001F6B9", L"EDA0BDEDBAB9" },	// 2文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_LATIN1,  L"\U0001F6B9", L"D83DDEB9" },		// 👈バグ。コードページがサポートしない文字なので、コードポイントで表示すべき。

		// サロゲートペア：コードポイント表示なら文字セットがサポートしない文字でも統一仕様
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_SJIS,    L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_JIS,     L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_EUC,     L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF16LE, L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF16BE, L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF32LE, L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF32BE, L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF8,    L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF7,    L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_CESU8,   L"\U0001F6B9", L"U+1F6B9" },
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_LATIN1,  L"\U0001F6B9", L"U+1F6B9" },

		// 結合文字「ぽ」（平仮名「ほ」＋結合文字半濁点）
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_SJIS,    L"ぽ", L"82D9" },		// 👈バグ。結合文字をサポートする文字セットなのに1文字分しか見えていない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_JIS,     L"ぽ", L"245B" },		// 👈バグ。結合文字をサポートする文字セットなのに1文字分しか見えていない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_EUC,     L"ぽ", L"A4DB" },		// 👈バグ。結合文字をサポートする文字セットなのに1文字分しか見えていない。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16LE, L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16BE, L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32LE, L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32BE, L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF8,    L"ぽ", L"E381BB" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF7,    L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_CESU8,   L"ぽ", L"E381BB" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_LATIN1,  L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。

		// コードポイント表示： 結合文字「ぽ」（平仮名「ほ」＋結合文字半濁点）
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_SJIS,    L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_JIS,     L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_EUC,     L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF16LE, L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF16BE, L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF32LE, L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF32BE, L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF8,    L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF7,    L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_CESU8,   L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_LATIN1,  L"ぽ", L"U+307B" },	// 👈バグ。結合文字が捨てられている。

		// IVS(Ideographic Variation Sequence) 「葛󠄀」（葛󠄀城市の葛󠄀、下がヒ）
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_SJIS,    L"\x845B\U000E0100", L"8A8B" },				// 👈バグ。IVSをサポートしない文字セットなのに正常っぽく見えている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_JIS,     L"\x845B\U000E0100", L"336B" },				// 👈バグ。IVSをサポートしない文字セットなのに正常っぽく見えている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_EUC,     L"\x845B\U000E0100", L"B3EB" },				// 👈バグ。IVSをサポートしない文字セットなのに正常っぽく見えている。
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16LE, L"\x845B\U000E0100", L"845B, DB40DD00" },		// 3文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF16BE, L"\x845B\U000E0100", L"845B, DB40DD00" },		// 3文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32LE, L"\x845B\U000E0100", L"845B, DB40DD00" },		// 3文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF32BE, L"\x845B\U000E0100", L"845B, DB40DD00" },		// 3文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF8,    L"\x845B\U000E0100", L"E8919BF3A08480" },		// 2文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_UTF7,    L"\x845B\U000E0100", L"845B, DB40DD00" },		// 3文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_CESU8,   L"\x845B\U000E0100", L"E8919BEDAD80EDB480" },	// 3文字分だが、区切りが分かりづらい
		UnicoceToHexTestParam{ ESettingType::Default,       CODE_LATIN1,  L"\x845B\U000E0100", L"845B, DB40DD00" },		// 3文字分だが、区切りが分かりづらい

		// コードポイント表示： IVS(Ideographic Variation Sequence) 「葛󠄀」（葛󠄀城市の葛󠄀、下がヒ）
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_SJIS,    L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_JIS,     L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_EUC,     L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF16LE, L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF16BE, L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF32LE, L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF32BE, L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF8,    L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_UTF7,    L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_CESU8,   L"\x845B\U000E0100", L"845B, U+E0100" },	// 👈バグ。1文字目がコードポイントであることを示せていない。
		UnicoceToHexTestParam{ ESettingType::DispCodepoint, CODE_LATIN1,  L"\x845B\U000E0100", L"845B, U+E0100" }	// 👈バグ。1文字目がコードポイントであることを示せていない。
	)
);

} // namespace  window
