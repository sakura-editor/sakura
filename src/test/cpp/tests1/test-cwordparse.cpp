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

ECharKind WhatKindOfChar(wchar_t ch)
{
	return CWordParse::WhatKindOfChar(&ch, 1, 0);
}

void ExpectEqualForEachChars(ECharKind kind, std::wstring_view s)
{
	for (wchar_t ch : s) {
		EXPECT_EQ(kind, WhatKindOfChar(ch));
	}
}

TEST(WhatKindOfChar, Null)
{
	EXPECT_EQ(CK_NULL, CWordParse::WhatKindOfChar(L"", 0, 0));
}

TEST(WhatKindOfChar, AsciiChars)
{
	ExpectEqualForEachChars(CK_CTRL, L"\x01\x02\x03\x04\x05\x06\x07\x08\x0b\x0c\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f");
	EXPECT_EQ(CK_TAB, WhatKindOfChar(L'\t'));
	EXPECT_EQ(CK_LF, WhatKindOfChar(L'\n'));
	EXPECT_EQ(CK_CR, WhatKindOfChar(L'\r'));
	EXPECT_EQ(CK_SPACE, WhatKindOfChar(L' '));
	ExpectEqualForEachChars(CK_ETC, L"!\"%&'()*+,-./:;<=>?[]^`{|}~");
	ExpectEqualForEachChars(CK_UDEF, L"#$@\\");
	ExpectEqualForEachChars(CK_CSYM, L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz");
}

TEST(WhatKindOfChar, HankakuKana)
{
	ExpectEqualForEachChars(CK_KATA, L"｡｢｣､･ｦｧｨｩｪｫｬｭｮｯｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ");
}

TEST(WhatKindOfChar, Latin1)
{
	ExpectEqualForEachChars(CK_LATIN, L"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽžſ");
}

TEST(WhatKindOfChar, ZenkakuSpace)
{
	EXPECT_EQ(CK_ZEN_SPACE, WhatKindOfChar(L'　'));
}

TEST(WhatKindOfChar, Nobasu)
{
	EXPECT_EQ(CK_ZEN_NOBASU, WhatKindOfChar(L'ー'));
}

TEST(WhatKindOfChar, Dakuten)
{
	EXPECT_EQ(CK_ZEN_DAKU, WhatKindOfChar(L'゛'));
	EXPECT_EQ(CK_ZEN_DAKU, WhatKindOfChar(L'゜'));
}

TEST(WhatKindOfChar, ZenkakuSymbols)
{
	ExpectEqualForEachChars(CK_ZEN_CSYM, L"＿０１２３４５６７８９ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｔｓｔｕｖｗｘｙｚ");
}

TEST(WhatKindOfChar, ZenkakuKigou)
{
	ExpectEqualForEachChars(CK_ZEN_KIGO, L"、。，．・：；？！´｀¨＾￣〃―‐／＼～∥｜…‥‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭♪†‡¶◯");
}

TEST(WhatKindOfChar, Hiragana)
{
	ExpectEqualForEachChars(CK_HIRA, L"ぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんゔゕゖゝゞ");
}

TEST(WhatKindOfChar, Katakana)
{
	ExpectEqualForEachChars(CK_ZEN_KATA, L"ァアィイゥウェエォオカガキギクグケゲコゴサザシジスズセゼソゾタダチヂッツヅテデトドナニヌネノハバパヒビピフブプヘベペホボポマミムメモャヤュユョヨラリルレロヮワヰヱヲンヴヵヶヷヸヹヺヽヾ");
}

TEST(WhatKindOfChar, Greek)
{
	ExpectEqualForEachChars(CK_GREEK, L"ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡ΢ΣΤΥΦΧΨΩΪΫάέήίΰαβγδεζηθικλμνξοπρςστυφχψω");
}

TEST(WhatKindOfChar, Cyrillic)
{
	ExpectEqualForEachChars(CK_ZEN_ROS, L"ЀЁЂЃЄЅІЇЈЉЊЋЌЍЎЏАБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюяѐёђѓєѕіїјљњћќѝўџѠѡѢѣѤѥѦѧѨѩѪѫѬѭѮѯѰѱѲѳѴѵѶѷѸѹѺѻѼѽѾѿҀҁ҂҃҄҅҆҇҈҉ҊҋҌҍҎҏҐґҒғҔҕҖҗҘҙҚқҜҝҞҟҠҡҢңҤҥҦҧҨҩҪҫҬҭҮүҰұҲҳҴҵҶҷҸҹҺһҼҽҾҿӀӁӂӃӄӅӆӇӈӉӊӋӌӍӎӏӐӑӒӓӔӕӖӗӘәӚӛӜӝӞӟӠӡӢӣӤӥӦӧӨөӪӫӬӭӮӯӰӱӲӳӴӵӶӷӸӹӺӻӼӽӾӿԀԁԂԃԄԅԆԇԈԉԊԋԌԍԎԏԐԑԒԓԔԕԖԗԘԙԚԛԜԝԞԟԠԡԢԣԤԥԦԧԨԩԪԫԬԭԮԯ");
	ExpectEqualForEachChars(CK_ZEN_ROS, L"\x2de0\x2de1\x2de2\x2de3\x2de4\x2de5\x2de6\x2de7\x2de8\x2de9\x2dea\x2deb\x2dec\x2ded\x2dee\x2def\x2df0\x2df1\x2df2\x2df3\x2df4\x2df5\x2df6\x2df7\x2df8\x2df9\x2dfa\x2dfb\x2dfc\x2dfd\x2dfe\x2dff");
	ExpectEqualForEachChars(CK_ZEN_ROS, L"ꙀꙁꙂꙃꙄꙅꙆꙇꙈꙉꙊꙋꙌꙍꙎꙏꙐꙑꙒꙓꙔꙕꙖꙗꙘꙙꙚꙛꙜꙝꙞꙟꙠꙡꙢꙣꙤꙥꙦꙧꙨꙩꙪꙫꙬꙭꙮ꙯꙰꙱꙲꙳ꙴꙵꙶꙷꙸꙹꙺꙻ꙼꙽꙾ꙿꚀꚁꚂꚃꚄꚅꚆꚇꚈꚉꚊꚋꚌꚍꚎꚏꚐꚑꚒꚓꚔꚕꚖꚗꚘꚙꚚꚛꚜꚝꚞꚟ");
}

TEST(WhatKindOfChar, BoxDrawing)
{
	ExpectEqualForEachChars(CK_ZEN_SKIGO, L"─━│┃┄┅┆┇┈┉┊┋┌┍┎┏┐┑┒┓└┕┖┗┘┙┚┛├┝┞┟┠┡┢┣┤┥┦┧┨┩┪┫┬┭┮┯┰┱┲┳┴┵┶┷┸┹┺┻┼┽┾┿╀╁╂╃╄╅╆╇╈╉╊╋╌╍╎╏═║╒╓╔╕╖╗╘╙╚╛╜╝╞╟╠╡╢╣╤╥╦╧╨╩╪╫╬╭╮╯╰╱╲╳╴╵╶╷╸╹╺╻╼╽╾╿");
}

TEST(WhatKindOfChar, SurrogatePairs)
{
//	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfChar(L"🌸", 2, 0));
//	EXPECT_EQ(CK_ZEN_ETC, CWordParse::WhatKindOfChar(L"𠮷", 2, 0));
	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfChar(L"\xd83c\xdf38", 2, 0));
	EXPECT_EQ(CK_ZEN_ETC, CWordParse::WhatKindOfChar(L"\xd842\xdfb7", 2, 0));
}

TEST(WhatKindOfChar, IVS)
{
//	EXPECT_EQ(CK_ETC, CWordParse::WhatKindOfChar(L"葛󠄀", 3, 0));
	EXPECT_EQ(CK_ZEN_ETC, CWordParse::WhatKindOfChar(L"\U0000845B\U000E0100"/*葛󠄀*/, 3, 0));
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
	EXPECT_FALSE(CWordParse::SearchPrevWordPosition(L"",
				CLogicInt(0), CLogicInt(0), nullptr, FALSE));
}

TEST(SearchPrevWordPosition, RespectsBooleanArgument)
{
	CLogicInt columnNew;
	bool result;
	result = CWordParse::SearchPrevWordPosition(L" sakura !",
			CLogicInt(9), CLogicInt(8), &columnNew, FALSE);
	EXPECT_TRUE(result);
	EXPECT_EQ(1, columnNew);

	result = CWordParse::SearchPrevWordPosition(L" sakura !",
			CLogicInt(9), CLogicInt(8), &columnNew, TRUE);
	EXPECT_TRUE(result);
	EXPECT_EQ(7, columnNew);
}

TEST(SearchNextWordPosition, ReturnsFalseWhenIndexIsAtEndOfString)
{
	EXPECT_FALSE(CWordParse::SearchNextWordPosition(L"",
				CLogicInt(0), CLogicInt(0), nullptr, FALSE));
}

TEST(SearchNextWordPosition, StopsAtIncompatibleKinds)
{
	CLogicInt columnNew;
	EXPECT_TRUE(CWordParse::SearchNextWordPosition(L"sakura!",
				CLogicInt(7), CLogicInt(0), &columnNew, FALSE));
	EXPECT_EQ(6, columnNew);
}

TEST(SearchNextWordPosition, RespectsBooleanArgument)
{
	CLogicInt columnNew;
	bool result;
	result = CWordParse::SearchNextWordPosition(L"sakura editor",
			CLogicInt(13), CLogicInt(0), &columnNew, FALSE);
	EXPECT_TRUE(result);
	EXPECT_EQ(7, columnNew);

	result = CWordParse::SearchNextWordPosition(L"sakura editor",
			CLogicInt(13), CLogicInt(0), &columnNew, TRUE);
	EXPECT_TRUE(result);
	EXPECT_EQ(6, columnNew);
}

TEST(SearchNextWordPosition4KW, ReturnsFalseWhenIndexIsAtEndOfString)
{
	EXPECT_FALSE(CWordParse::SearchNextWordPosition4KW(L"",
				CLogicInt(0), CLogicInt(0), nullptr, FALSE));
}

TEST(SearchNextWordPosition4KW, StopsAtIncompatibleKinds)
{
	CLogicInt columnNew;
	EXPECT_TRUE(CWordParse::SearchNextWordPosition4KW(L"@sakura!",
				CLogicInt(8), CLogicInt(0), &columnNew, FALSE));
	EXPECT_EQ(7, columnNew);
}

TEST(WhereCurrentWord_2, ReturnsFalseIfIndexIsAtNewLineOrEOS)
{
	CLogicInt from, to;
	EXPECT_FALSE(CWordParse::WhereCurrentWord_2(L"sakura", CLogicInt(6),
			CLogicInt(6), false, &from, &to, nullptr, nullptr));
	EXPECT_EQ(6, from);
	EXPECT_EQ(6, to);
	EXPECT_FALSE(CWordParse::WhereCurrentWord_2(L"sakura\n", CLogicInt(7),
			CLogicInt(6), false, &from, &to, nullptr, nullptr));
	EXPECT_EQ(6, from);
	EXPECT_EQ(6, to);
}

TEST(WhereCurrentWord_2, RespectsExtEolFlag)
{
	CLogicInt from, to;
	EXPECT_FALSE(CWordParse::WhereCurrentWord_2(L"sakura\x85", CLogicInt(7),
			CLogicInt(6), true, &from, &to, nullptr, nullptr));
	EXPECT_EQ(6, from);
	EXPECT_EQ(6, to);
	EXPECT_TRUE(CWordParse::WhereCurrentWord_2(L"sakura\x85", CLogicInt(7),
			CLogicInt(6), false, &from, &to, nullptr, nullptr));
	EXPECT_EQ(6, from);
	EXPECT_EQ(7, to);
}

TEST(WhereCurrentWord_2, ReturnsCMemoryIfSpecified)
{
	CLogicInt from, to;
	CNativeW word, wordLeft;
	bool result = CWordParse::WhereCurrentWord_2(L"sakura editor",
		   CLogicInt(13), CLogicInt(10), false, &from, &to, &word, &wordLeft);
	EXPECT_TRUE(result);
	EXPECT_EQ(7, from);
	EXPECT_EQ(13, to);
	EXPECT_EQ(word, L"editor");
	EXPECT_EQ(wordLeft, L"edi");
}
