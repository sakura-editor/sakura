/*! @file */
/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "parse/CWordParse.h"

#include <cstring>
#include <iostream>
#include <string_view>
#include "charset/charcode.h"
#include "mem/CNativeW.h"

std::ostream& operator<<(std::ostream& os, ECharKind kind)
{
	static const char* s[] = {"CK_NUL", "CK_TAB", "CK_CR", "CK_LF", "CK_CTR", "CK_SPACE",
	   	"CK_CSYM", "CK_KATA", "CK_LATIN", "CK_UDEF", "CK_ETC", "CK_ZEN_SPACE",
	   	"CK_ZEN_NOBASU", "CK_ZEN_DAKU", "CK_ZEN_CSYM", "CK_ZEN_KIGO", "CK_HIRA",
	   	"CK_ZEN_KATA", "CK_GREEK", "CK_ZEN_ROS", "CK_ZEN_SKIGO", "CK_ZEN_ETC"};
	return os << s[kind];
}

namespace parse {

TEST(IsURL, HttpProtocol)
{
	const auto result = IsURL(LTEXT("http://example.com"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(18)));
}

TEST(IsURL, HttpsProtocol)
{
	const auto result = IsURL(LTEXT("https://example.com"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(19)));
}

TEST(IsURL, FtpProtocol)
{
	const auto result = IsURL(LTEXT("ftp://ftp.example.com/path"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(26)));
}

TEST(IsURL, FileProtocol)
{
	const auto result = IsURL(LTEXT("file:///path/to/file.txt"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(24)));
}

TEST(IsURL, TtpProtocol)
{
	const auto result = IsURL(LTEXT("ttp://example.com"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(17)));
}

TEST(IsURL, TpProtocol)
{
	const auto result = IsURL(LTEXT("tp://example.com"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(16)));
}

TEST(IsURL, MailtoProtocol)
{
	const auto result = IsURL(LTEXT("mailto:test@example.com"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(23)));	// 7 (mailto:) + 16 (user@example.com) = 23
}

TEST(IsURL, MailtoProtocolWithBadAddress)
{
	const auto result = IsURL(LTEXT("mailto:admin@localhost"));
	EXPECT_FALSE(result);
}

TEST(IsURL, MailAddressOnly)
{
	const auto result = IsURL(LTEXT("user@example.com"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(16)));
}

TEST(IsURL, UrlWithPath)
{
	const auto result = IsURL(LTEXT("https://example.com/path/to/page.html"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(37)));
}

TEST(IsURL, UrlWithQuery)
{
	const auto result = IsURL(LTEXT("http://example.com?key=value&other=data"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(39)));
}

TEST(IsURL, UrlWithFragment)
{
	const auto result = IsURL(LTEXT("http://example.com/page#section"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(31)));
}

TEST(IsURL, UrlWithPort)
{
	const auto result = IsURL(LTEXT("http://example.com:8080/path"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(28)));
}

TEST(IsURL, UrlInText)
{
	sakura::wstring_view text = LTEXT("Check this: http://example.com for more info");
	const auto result = IsURL(text, 12);
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(18)));
}

TEST(IsURL, NotUrl_NoProtocol)
{
	const auto result = IsURL(LTEXT("example.com"));
	EXPECT_FALSE(result);
}

TEST(IsURL, NotUrl_JustText)
{
	const auto result = IsURL(LTEXT("hello world"));
	EXPECT_FALSE(result);
}

TEST(IsURL, NotUrl_IncompleteProtocol)
{
	const auto result = IsURL(LTEXT("http://"));
	EXPECT_FALSE(result);
}

TEST(IsURL, NotUrl_InvalidMailAddress)
{
	const auto result = IsURL(LTEXT("@example.com"));
	EXPECT_FALSE(result);
}

TEST(IsURL, MailAddressWithSymbols)
{
	const auto result = IsURL(LTEXT("user.name+tag@example.co.jp"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(27)));
}

TEST(IsURL, UrlStopsAtSpace)
{
	const auto result = IsURL(LTEXT("http://example.com more text"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(18)));
}

TEST(IsURL, UrlStopsAtParenthesis)
{
	const auto result = IsURL(LTEXT("(http://example.com)"), 1);
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(18)));
}

TEST(IsURL, GopherProtocol)
{
	const auto result = IsURL(LTEXT("gopher://gopher.example.com"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(27)));
}

TEST(IsURL, NewsProtocol)
{
	const auto result = IsURL(LTEXT("news:comp.lang.c++"));
	EXPECT_TRUE(result);
	EXPECT_THAT(result.matched, ::testing::SizeIs(Eq(18)));
}

ECharKind WhatKindOfChar(WCHAR ch)
{
	return CWordParse::WhatKindOfChar(&ch, 1, 0);
}

void ExpectEqualForEachChars(ECharKind kind, sakura::wstring_view s)
{
	for (WCHAR ch : s) {
		EXPECT_EQ(kind, WhatKindOfChar(ch));
	}
}

TEST(WhatKindOfChar, Null)
{
	EXPECT_EQ(CK_NULL, CWordParse::WhatKindOfChar(LTEXT(""), 0, 0));
}

TEST(WhatKindOfChar, AsciiChars)
{
	ExpectEqualForEachChars(CK_CTRL, LTEXT("\x01\x02\x03\x04\x05\x06\x07\x08\x0b\x0c\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f"));
	EXPECT_EQ(CK_TAB, WhatKindOfChar(LCHAR('\t')));
	EXPECT_EQ(CK_LF, WhatKindOfChar(LCHAR('\n')));
	EXPECT_EQ(CK_CR, WhatKindOfChar(LCHAR('\r')));
	EXPECT_EQ(CK_SPACE, WhatKindOfChar(LCHAR(' ')));
	ExpectEqualForEachChars(CK_ETC, LTEXT("!\"%&'()*+,-./:;<=>?[]^`{|}~"));
	ExpectEqualForEachChars(CK_UDEF, LTEXT("#$@\\"));
	ExpectEqualForEachChars(CK_CSYM, LTEXT("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz"));
}

TEST(WhatKindOfChar, HankakuKana)
{
	ExpectEqualForEachChars(CK_KATA, LTEXT("｡｢｣､･ｦｧｨｩｪｫｬｭｮｯｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ"));
}

TEST(WhatKindOfChar, Latin1)
{
	ExpectEqualForEachChars(CK_LATIN, LTEXT("ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽžſ"));
}

TEST(WhatKindOfChar, ZenkakuSpace)
{
	EXPECT_EQ(CK_ZEN_SPACE, WhatKindOfChar(LCHAR('　')));
}

TEST(WhatKindOfChar, Nobasu)
{
	EXPECT_EQ(CK_ZEN_NOBASU, WhatKindOfChar(LCHAR('ー')));
}

TEST(WhatKindOfChar, Dakuten)
{
	EXPECT_EQ(CK_ZEN_DAKU, WhatKindOfChar(LCHAR('゛')));
	EXPECT_EQ(CK_ZEN_DAKU, WhatKindOfChar(LCHAR('゜')));
}

TEST(WhatKindOfChar, ZenkakuSymbols)
{
	ExpectEqualForEachChars(CK_ZEN_CSYM, LTEXT("＿０１２３４５６７８９ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｔｓｔｕｖｗｘｙｚ"));
}

TEST(WhatKindOfChar, ZenkakuKigou)
{
	ExpectEqualForEachChars(CK_ZEN_KIGO, LTEXT("、。，．・：；？！´｀¨＾￣〃―‐／＼～∥｜…‥‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭♪†‡¶◯"));
}

TEST(WhatKindOfChar, Hiragana)
{
	ExpectEqualForEachChars(CK_HIRA, LTEXT("ぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんゔゕゖゝゞ"));
}

TEST(WhatKindOfChar, Katakana)
{
	ExpectEqualForEachChars(CK_ZEN_KATA, LTEXT("ァアィイゥウェエォオカガキギクグケゲコゴサザシジスズセゼソゾタダチヂッツヅテデトドナニヌネノハバパヒビピフブプヘベペホボポマミムメモャヤュユョヨラリルレロヮワヰヱヲンヴヵヶヷヸヹヺヽヾ"));
}

TEST(WhatKindOfChar, Greek)
{
	ExpectEqualForEachChars(CK_GREEK, LTEXT("ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡ΢ΣΤΥΦΧΨΩΪΫάέήίΰαβγδεζηθικλμνξοπρςστυφχψω"));
}

TEST(WhatKindOfChar, Cyrillic)
{
	ExpectEqualForEachChars(CK_ZEN_ROS, LTEXT("ЀЁЂЃЄЅІЇЈЉЊЋЌЍЎЏАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюяѐёђѓєѕіїјљњћќѝўџѠѡѢѣѤѥѦѧѨѩѪѫѬѭѮѯѰѱѲѳѴѵѶѷѸѹѺѻѼѽѾѿҀҁ҂҃҄҅҆҇҈҉ҊҋҌҍҎҏҐґҒғҔҕҖҗҘҙҚқҜҝҞҟҠҡҢңҤҥҦҧҨҩҪҫҬҭҮүҰұҲҳҴҵҶҷҸҹҺһҼҽҾҿӀӁӂӃӄӅӆӇӈӉӊӋӌӍӎӏӐӑӒӓӔӕӖӗӘәӚӛӜӝӞӟӠӡӢӣӤӥӦӧӨөӪӫӬӭӮӯӰӱӲӳӴӵӶӷӸӹӺӻӼӽӾӿԀԁԂԃԄԅԆԇԈԉԊԋԌԍԎԏԐԑԒԓԔԕԖԗԘԙԚԛԜԝԞԟԠԡԢԣԤԥԦԧԨԩԪԫԬԭԮԯ"));
	ExpectEqualForEachChars(CK_ZEN_ROS, LTEXT("\x2de0\x2de1\x2de2\x2de3\x2de4\x2de5\x2de6\x2de7\x2de8\x2de9\x2dea\x2deb\x2dec\x2ded\x2dee\x2def\x2df0\x2df1\x2df2\x2df3\x2df4\x2df5\x2df6\x2df7\x2df8\x2df9\x2dfa\x2dfb\x2dfc\x2dfd\x2dfe\x2dff"));
	ExpectEqualForEachChars(CK_ZEN_ROS, LTEXT("ꙀꙁꙂꙃꙄꙅꙆꙇꙈꙉꙊꙋꙌꙍꙎꙏꙐꙑꙒꙓꙔꙕꙖꙗꙘꙙꙚꙛꙜꙝꙞꙟꙠꙡꙢꙣꙤꙥꙦꙧꙨꙩꙪꙫꙬꙭꙮ꙯꙰꙱꙲꙳ꙴꙵꙶꙷꙸꙹꙺꙻ꙼꙽꙾ꙿꚀꚁꚂꚃꚄꚅꚆꚇꚈꚉꚊꚋꚌꚍꚎꚏꚐꚑꚒꚓꚔꚕꚖꚗꚘꚙꚚꚛꚜꚝꚞꚟ"));
}

TEST(WhatKindOfChar, BoxDrawing)
{
	ExpectEqualForEachChars(CK_ZEN_SKIGO, LTEXT("─━│┃┄┅┆┇┈┉┊┋┌┍┎┏┐┑┒┓└┕┖┗┘┙┚┛├┝┞┟┠┡┢┣┤┥┦┧┨┩┪┫┬┭┮┯┰┱┲┳┴┵┶┷┸┹┺┻┼┽┾┿╀╁╂╃╄╅╆╇╈╉╊╋╌╍╎╏═║╒╓╔╕╖╗╘╙╚╛╜╝╞╟╠╡╢╣╤╥╦╧╨╩╪╫╬╭╮╯╰╱╲╳╴╵╶╷╸╹╺╻╼╽╾╿"));
}

TEST(WhatKindOfChar, SurrogatePairs)
{
//	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfChar(L"🌸", 2, 0));
//	EXPECT_EQ(CK_ZEN_ETC, CWordParse::WhatKindOfChar(L"𠮷", 2, 0));
	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfChar(LTEXT("\xd83c\xdf38"), 2, 0));
	EXPECT_EQ(CK_ZEN_ETC, CWordParse::WhatKindOfChar(LTEXT("\xd842\xdfb7"), 2, 0));
}

TEST(WhatKindOfChar, IVS)
{
//	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfChar(L"葛󠄀", 3, 0));
	EXPECT_EQ(CK_ZEN_ETC, CWordParse::WhatKindOfChar(LTEXT("\U0000845B\U000E0100")/*葛󠄀*/, 3, 0));
}

TEST(WhatKindOfTwoChars, ReturnsSameKindIfTwoKindsAreIdentical)
{
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars(CK_HIRA, CK_HIRA));
	EXPECT_EQ(CK_LATIN, CWordParse::WhatKindOfTwoChars(CK_LATIN, CK_LATIN));
	EXPECT_EQ(CK_UDEF, CWordParse::WhatKindOfTwoChars(CK_UDEF, CK_UDEF));
	EXPECT_EQ(CK_CTRL, CWordParse::WhatKindOfTwoChars(CK_CTRL, CK_CTRL));
}

TEST(WhatKindOfTwoChars, MergesZenkakuNobasuIntoKanas)
{
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars(CK_ZEN_NOBASU, CK_HIRA));
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars(CK_HIRA, CK_ZEN_NOBASU));
	EXPECT_EQ(CK_ZEN_KATA, CWordParse::WhatKindOfTwoChars(CK_ZEN_NOBASU, CK_ZEN_KATA));
	EXPECT_EQ(CK_ZEN_KATA, CWordParse::WhatKindOfTwoChars(CK_ZEN_KATA, CK_ZEN_NOBASU));
}

TEST(WhatKindOfTwoChars, MergesZenkakuDakutenIntoKanas)
{
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars(CK_ZEN_DAKU, CK_HIRA));
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars(CK_HIRA, CK_ZEN_DAKU));
	EXPECT_EQ(CK_ZEN_KATA, CWordParse::WhatKindOfTwoChars(CK_ZEN_DAKU, CK_ZEN_KATA));
	EXPECT_EQ(CK_ZEN_KATA, CWordParse::WhatKindOfTwoChars(CK_ZEN_KATA, CK_ZEN_DAKU));
}

TEST(WhatKindOfTwoChars, MergesNobasuAndDakutenTogether)
{
	EXPECT_EQ(CK_ZEN_NOBASU, CWordParse::WhatKindOfTwoChars(CK_ZEN_DAKU, CK_ZEN_NOBASU));
	EXPECT_EQ(CK_ZEN_DAKU, CWordParse::WhatKindOfTwoChars(CK_ZEN_NOBASU, CK_ZEN_DAKU));
}

TEST(WhatKindOfTwoChars, TreatsLatinAsAlphanumeric)
{
	EXPECT_EQ(CK_CSYM, CWordParse::WhatKindOfTwoChars(CK_CSYM, CK_LATIN));
	EXPECT_EQ(CK_CSYM, CWordParse::WhatKindOfTwoChars(CK_LATIN, CK_CSYM));
}

TEST(WhatKindOfTwoChars, TreatsUserDefinedAsEtc)
{
	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfTwoChars(CK_ETC, CK_UDEF));
	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfTwoChars(CK_UDEF, CK_ETC));
}

TEST(WhatKindOfTwoChars, TreatsControlCharsAsEtc)
{
	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfTwoChars(CK_ETC, CK_CTRL));
	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfTwoChars(CK_CTRL, CK_ETC));
}

TEST(WhatKindOfTwoChars, ReturnsNullOnIncompatibleKinds)
{
	EXPECT_EQ(CK_NULL, CWordParse::WhatKindOfTwoChars(CK_HIRA, CK_LATIN));
}

TEST(WhatKindOfTwoChars4KW, ReturnsSameKindIfTwoKindsAreIdentical)
{
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars4KW(CK_HIRA, CK_HIRA));
	EXPECT_EQ(CK_LATIN, CWordParse::WhatKindOfTwoChars4KW(CK_LATIN, CK_LATIN));
	EXPECT_EQ(CK_UDEF, CWordParse::WhatKindOfTwoChars4KW(CK_UDEF, CK_UDEF));
	EXPECT_EQ(CK_CTRL, CWordParse::WhatKindOfTwoChars4KW(CK_CTRL, CK_CTRL));
}

TEST(WhatKindOfTwoChars4KW, MergesZenkakuNobasuIntoKanas)
{
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars4KW(CK_ZEN_NOBASU, CK_HIRA));
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars4KW(CK_HIRA, CK_ZEN_NOBASU));
	EXPECT_EQ(CK_ZEN_KATA, CWordParse::WhatKindOfTwoChars4KW(CK_ZEN_NOBASU, CK_ZEN_KATA));
	EXPECT_EQ(CK_ZEN_KATA, CWordParse::WhatKindOfTwoChars4KW(CK_ZEN_KATA, CK_ZEN_NOBASU));
}

TEST(WhatKindOfTwoChars4KW, MergesZenkakuDakutenIntoKanas)
{
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars4KW(CK_ZEN_DAKU, CK_HIRA));
	EXPECT_EQ(CK_HIRA, CWordParse::WhatKindOfTwoChars4KW(CK_HIRA, CK_ZEN_DAKU));
	EXPECT_EQ(CK_ZEN_KATA, CWordParse::WhatKindOfTwoChars4KW(CK_ZEN_DAKU, CK_ZEN_KATA));
	EXPECT_EQ(CK_ZEN_KATA, CWordParse::WhatKindOfTwoChars4KW(CK_ZEN_KATA, CK_ZEN_DAKU));
}

TEST(WhatKindOfTwoChars4KW, MergesNobasuAndDakutenTogether)
{
	EXPECT_EQ(CK_ZEN_NOBASU, CWordParse::WhatKindOfTwoChars4KW(CK_ZEN_DAKU, CK_ZEN_NOBASU));
	EXPECT_EQ(CK_ZEN_DAKU, CWordParse::WhatKindOfTwoChars4KW(CK_ZEN_NOBASU, CK_ZEN_DAKU));
}

TEST(WhatKindOfTwoChars4KW, TreatsLatinAsAlphanumeric)
{
	EXPECT_EQ(CK_CSYM, CWordParse::WhatKindOfTwoChars4KW(CK_CSYM, CK_LATIN));
	EXPECT_EQ(CK_CSYM, CWordParse::WhatKindOfTwoChars4KW(CK_LATIN, CK_CSYM));
}

TEST(WhatKindOfTwoChars4KW, TreatsUserDefinedAsAlphanumeric)
{
	EXPECT_EQ(CK_CSYM, CWordParse::WhatKindOfTwoChars4KW(CK_CSYM, CK_UDEF));
	EXPECT_EQ(CK_CSYM, CWordParse::WhatKindOfTwoChars4KW(CK_UDEF, CK_CSYM));
}

TEST(WhatKindOfTwoChars4KW, LeavesControlCharsAsIs)
{
	EXPECT_EQ(CK_NULL, CWordParse::WhatKindOfTwoChars4KW(CK_ETC, CK_CTRL));
	EXPECT_EQ(CK_NULL, CWordParse::WhatKindOfTwoChars4KW(CK_CTRL, CK_ETC));
}

TEST(WhatKindOfTwoChars4KW, ReturnsNullOnIncompatibleKinds)
{
	EXPECT_EQ(CK_NULL, CWordParse::WhatKindOfTwoChars4KW(CK_HIRA, CK_LATIN));
}

TEST(SearchPrevWordPosition, ReturnsFalseWhenIndexIsZero)
{
	EXPECT_FALSE(CWordParse::SearchPrevWordPosition(LTEXT(""),
				CLogicInt(0), CLogicInt(0), nullptr, FALSE));
}

TEST(SearchPrevWordPosition, RespectsBooleanArgument)
{
	CLogicInt columnNew;
	bool result;
	result = CWordParse::SearchPrevWordPosition(LTEXT(" sakura !"),
			CLogicInt(9), CLogicInt(8), &columnNew, FALSE);
	EXPECT_TRUE(result);
	EXPECT_EQ(1, columnNew);

	result = CWordParse::SearchPrevWordPosition(LTEXT(" sakura !"),
			CLogicInt(9), CLogicInt(8), &columnNew, TRUE);
	EXPECT_TRUE(result);
	EXPECT_EQ(7, columnNew);
}

TEST(SearchNextWordPosition, ReturnsFalseWhenIndexIsAtEndOfString)
{
	EXPECT_FALSE(CWordParse::SearchNextWordPosition(LTEXT(""),
				CLogicInt(0), CLogicInt(0), nullptr, FALSE));
}

TEST(SearchNextWordPosition, StopsAtIncompatibleKinds)
{
	CLogicInt columnNew;
	EXPECT_TRUE(CWordParse::SearchNextWordPosition(LTEXT("sakura!"),
				CLogicInt(7), CLogicInt(0), &columnNew, FALSE));
	EXPECT_EQ(6, columnNew);
}

TEST(SearchNextWordPosition, RespectsBooleanArgument)
{
	CLogicInt columnNew;
	bool result;
	result = CWordParse::SearchNextWordPosition(LTEXT("sakura editor"),
			CLogicInt(13), CLogicInt(0), &columnNew, FALSE);
	EXPECT_TRUE(result);
	EXPECT_EQ(7, columnNew);

	result = CWordParse::SearchNextWordPosition(LTEXT("sakura editor"),
			CLogicInt(13), CLogicInt(0), &columnNew, TRUE);
	EXPECT_TRUE(result);
	EXPECT_EQ(6, columnNew);
}

TEST(SearchNextWordPosition4KW, ReturnsFalseWhenIndexIsAtEndOfString)
{
	EXPECT_FALSE(CWordParse::SearchNextWordPosition4KW(LTEXT(""),
				CLogicInt(0), CLogicInt(0), nullptr, FALSE));
}

TEST(SearchNextWordPosition4KW, StopsAtIncompatibleKinds)
{
	CLogicInt columnNew;
	EXPECT_TRUE(CWordParse::SearchNextWordPosition4KW(LTEXT("@sakura!"),
				CLogicInt(8), CLogicInt(0), &columnNew, FALSE));
	EXPECT_EQ(7, columnNew);
}

TEST(WhereCurrentWord_2, ReturnsFalseIfIndexIsAtNewLineOrEOS)
{
	CLogicInt from, to;
	EXPECT_FALSE(CWordParse::WhereCurrentWord_2(LTEXT("sakura"), CLogicInt(6),
			CLogicInt(6), false, &from, &to, nullptr, nullptr));
	EXPECT_EQ(6, from);
	EXPECT_EQ(6, to);
	EXPECT_FALSE(CWordParse::WhereCurrentWord_2(LTEXT("sakura\n"), CLogicInt(7),
			CLogicInt(6), false, &from, &to, nullptr, nullptr));
	EXPECT_EQ(6, from);
	EXPECT_EQ(6, to);
}

TEST(WhereCurrentWord_2, RespectsExtEolFlag)
{
	CLogicInt from, to;
	EXPECT_FALSE(CWordParse::WhereCurrentWord_2(LTEXT("sakura\x85"), CLogicInt(7),
			CLogicInt(6), true, &from, &to, nullptr, nullptr));
	EXPECT_EQ(6, from);
	EXPECT_EQ(6, to);
	EXPECT_TRUE(CWordParse::WhereCurrentWord_2(LTEXT("sakura\x85"), CLogicInt(7),
			CLogicInt(6), false, &from, &to, nullptr, nullptr));
	EXPECT_EQ(6, from);
	EXPECT_EQ(7, to);
}

TEST(WhereCurrentWord_2, ReturnsCMemoryIfSpecified)
{
	CLogicInt from, to;
	CNativeW word, wordLeft;
	bool result = CWordParse::WhereCurrentWord_2(LTEXT("sakura editor"),
		   CLogicInt(13), CLogicInt(10), false, &from, &to, &word, &wordLeft);
	EXPECT_TRUE(result);
	EXPECT_EQ(7, from);
	EXPECT_EQ(13, to);
	EXPECT_EQ(word, LTEXT("editor"));
	EXPECT_EQ(wordLeft, LTEXT("edi"));
}

} // namespace parse
