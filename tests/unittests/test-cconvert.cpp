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
#include "convert/CConvert.h"

#include "testing/GuiAwareTestSuite.hpp"

#include <ostream>
#include <tuple>

#include "convert/CConvert_HaneisuToZeneisu.h"
#include "convert/CConvert_HankataToZenhira.h"
#include "convert/CConvert_HankataToZenkata.h"
#include "convert/CConvert_SpaceToTab.h"
#include "convert/CConvert_TabToSpace.h"
#include "convert/CConvert_ToHankaku.h"
#include "convert/CConvert_ToLower.h"
#include "convert/CConvert_ToUpper.h"
#include "convert/CConvert_ToZenhira.h"
#include "convert/CConvert_ToZenkata.h"
#include "convert/CConvert_Trim.h"
#include "convert/CConvert_ZeneisuToHaneisu.h"
#include "convert/CConvert_ZenkataToHankata.h"

//! googletestの出力に機能IDを出力させる
std::ostream& operator << (std::ostream& os, const EFunctionCode& eFuncCode);

namespace convert {

TEST(CConvert, ZenkataToHankata)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(L"アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヰヱヲンァィゥェォッャュョヮヵヶヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポ。、「」・ ");
	expected.SetString(L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｲｴｦﾝｧｨｩｪｫｯｬｭｮﾜｶｹｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟ｡､｢｣･ ");
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 長音符は前の文字がカタカナだった場合に限って変換する。
	// 長音符が連続する場合は前の文字と同様に処理する。
	actual.SetString(L"ーーナーーベーーパーー。ー");
	expected.SetString(L"ーーﾅｰｰﾍﾞｰｰﾊﾟｰｰ｡ー");
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 濁点・半濁点は清音の後に現れた場合のみ変換する。
	actual.SetString(L"゛゜\u3099\u309A");
	expected.SetString(L"゛゜\u3099\u309A");
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"サ゛゛フ゜゜サ\u3099\u3099フ\u309A\u309A");
	expected.SetString(L"ｻﾞ゛ﾌﾟ゜ｻﾞ\u3099ﾌﾟ\u309A");
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"ザ゛プ゜ザ\u3099プ\u309A");
	expected.SetString(L"ｻﾞ゛ﾌﾟ゜ｻﾞ\u3099ﾌﾟ\u309A");
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"ー゛・゜");
	expected.SetString(L"ー゛･゜");
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, HankataToZenkata)
{
	CNativeW actual(L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｯｬｭｮｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟｰﾞﾟ｡､｢｣･");
	CNativeW expected(L"アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲンァィゥェォッャュョヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポー゛゜。、「」・");
	EXPECT_TRUE(CConvert_HankataToZenkata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, HankataToZenhira)
{
	CNativeW actual(L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｯｬｭｮｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟｰﾞﾟ｡､｢｣･ ");
	CNativeW expected(L"あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわをんぁぃぅぇぉっゃゅょゔがぎぐげござじずぜぞだぢづでどばびぶべぼわ゛ヸヹを゛ぱぴぷぺぽー゛゜。、「」・ ");
	EXPECT_TRUE(CConvert_HankataToZenhira().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, HaneisuToZeneisu)
{
	CNativeW actual(L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_");
	CNativeW expected(L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿");
	EXPECT_TRUE(CConvert_HaneisuToZeneisu().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ZeneisuToHaneisu)
{
	CNativeW actual(L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿");
	CNativeW expected(L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_");
	EXPECT_TRUE(CConvert_ZeneisuToHaneisu().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToLower)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	expected.SetString(L"abcdefghijklmnopqrstuvwxyz");
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ");
	expected.SetString(L"ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ");
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// ギリシャ文字
	actual.SetString(L"ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ");
	expected.SetString(L"αβγδεζηθικλμνξοπρστυφχψω");
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// キリル文字
	actual.SetString(L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ");
	expected.SetString(L"абвгдежзийклмнопрстуфхцчшщъыьэюя");
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 拡張キリル文字
	actual.SetString(L"ЀЁЂЃЄЅІЇЈЉЊЋЌЍЎЏ");
	expected.SetString(L"ѐёђѓєѕіїјљњћќѝўџ");
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin-1
	actual.SetString(L"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß");
	expected.SetString(L"àáâãäåæçèéêëìíîïðñòóôõö×øùúûüýþß");
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended-A
	actual.SetString(L"ĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽžſ");
	expected.SetString(L"āāăăąąććĉĉċċččďďđđēēĕĕėėęęěěĝĝğğġġģģĥĥħħĩĩīīĭĭįįiıĳĳĵĵķķĸĺĺļļľľŀŀłłńńņņňňŉŋŋōōŏŏőőœœŕŕŗŗřřśśŝŝşşššţţťťŧŧũũūūŭŭůůűűųųŵŵŷŷÿźźżżžžſ");
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended-B
	actual.SetString(L"ƀƁƂƃƄƅƆƇƈƉƊƋƌƍƎƏƐƑƒƓƔƕƖƗƘƙƚƛƜƝƞƟƠơƢƣƤƥƦƧƨƩƪƫƬƭƮƯưƱƲƳƴƵƶƷƸƹƺƻƼƽƾƿǀǁǂǃǄǅǆǇǈǉǊǋǌǍǎǏǐǑǒǓǔǕǖǗǘǙǚǛǜǝǞǟǠǡǢǣǤǥǦǧǨǩǪǫǬǭǮǯǰǱǲǳǴǵǶǷǸǹǺǻǼǽǾǿȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟȠȡȢȣȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿɀɁɂɃɄɅɆɇɈɉɊɋɌɍɎɏ");
	expected.SetString(L"ƁƁƃƃƅƅƆƈƈƊƊƌƌƍǝƏƐƒƒƓƔƕƖƗƙƙƚƛƜƝƞƟơơƣƣƥƥƦƨƨƩƪƫƭƭƮưưƱƲƴƴƶƶƷƹƹƺƻƽƽƾƿǀǁǂǃǆǆǆǉǉǉǌǌǌǎǎǐǐǒǒǔǔǖǖǘǘǚǚǜǜǝǟǟǡǡǣǣǥǥǧǧǩǩǫǫǭǭǯǯǰǳǳǳǵǵƕƿǹǹǻǻǽǽǿǿȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟȠȡȢȣȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿɀɁɂɃɄɅɆɇɈɉɊɋɌɍɎɏ");
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended Additional
	actual.SetString(L"ḀḁḂḃḄḅḆḇḈḉḊḋḌḍḎḏḐḑḒḓḔḕḖḗḘḙḚḛḜḝḞḟḠḡḢḣḤḥḦḧḨḩḪḫḬḭḮḯḰḱḲḳḴḵḶḷḸḹḺḻḼḽḾḿṀṁṂṃṄṅṆṇṈṉṊṋṌṍṎṏṐṑṒṓṔṕṖṗṘṙṚṛṜṝṞṟṠṡṢṣṤṥṦṧṨṩṪṫṬṭṮṯṰṱṲṳṴṵṶṷṸṹṺṻṼṽṾṿẀẁẂẃẄẅẆẇẈẉẊẋẌẍẎẏẐẑẒẓẔẕẖẗẘẙẚẛẜẝẞẟẠạẢảẤấẦầẨẩẪẫẬậẮắẰằẲẳẴẵẶặẸẹẺẻẼẽẾếỀềỂểỄễỆệỈỉỊịỌọỎỏỐốỒồỔổỖỗỘộỚớỜờỞởỠỡỢợỤụỦủỨứỪừỬửỮữỰựỲỳỴỵỶỷỸỹỺỻỼỽỾỿ");
	expected.SetString(L"ḁḁḃḃḅḅḇḇḉḉḋḋḍḍḏḏḑḑḓḓḕḕḗḗḙḙḛḛḝḝḟḟḡḡḣḣḥḥḧḧḩḩḫḫḭḭḯḯḱḱḳḳḵḵḷḷḹḹḻḻḽḽḿḿṁṁṃṃṅṅṇṇṉṉṋṋṍṍṏṏṑṑṓṓṕṕṗṗṙṙṛṛṝṝṟṟṡṡṣṣṥṥṧṧṩṩṫṫṭṭṯṯṱṱṳṳṵṵṷṷṹṹṻṻṽṽṿṿẁẁẃẃẅẅẇẇẉẉẋẋẍẍẏẏẑẑẓẓẕẕẖẗẘẙẚẛẜẝẞẟạạảảấấầầẩẩẫẫậậắắằằẳẳẵẵặặẹẹẻẻẽẽếếềềểểễễệệỉỉịịọọỏỏốốồồổổỗỗộộớớờờởởỡỡợợụụủủứứừừửửữữựựỳỳỵỵỷỷỹỹỻỻỽỽỿỿ");
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToUpper)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(L"abcdefghijklmnopqrstuvwxyz");
	expected.SetString(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ");
	expected.SetString(L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ");
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// ギリシャ文字
	actual.SetString(L"αβγδεζηθικλμνξοπρστυφχψω");
	expected.SetString(L"ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ");
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// キリル文字
	actual.SetString(L"абвгдежзийклмнопрстуфхцчшщъыьэюя");
	expected.SetString(L"АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ");
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 拡張キリル文字
	actual.SetString(L"ѐёђѓєѕіїјљњћќѝўџ");
	expected.SetString(L"ЀЁЂЃЄЅІЇЈЉЊЋЌЍЎЏ");
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin-1
	actual.SetString(L"àáâãäåæçèéêëìíîïðñòóôõö×øùúûüýþÿ");
	expected.SetString(L"ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞŸ");
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended-A
	actual.SetString(L"ĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽžſ");
	expected.SetString(L"ĀĀĂĂĄĄĆĆĈĈĊĊČČĎĎĐĐĒĒĔĔĖĖĘĘĚĚĜĜĞĞĠĠĢĢĤĤĦĦĨĨĪĪĬĬĮĮİIĲĲĴĴĶĶĸĹĹĻĻĽĽĿĿŁŁŃŃŅŅŇŇŉŊŊŌŌŎŎŐŐŒŒŔŔŖŖŘŘŚŚŜŜŞŞŠŠŢŢŤŤŦŦŨŨŪŪŬŬŮŮŰŰŲŲŴŴŶŶŸŹŹŻŻŽŽſ");
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended-B
	actual.SetString(L"ƀƁƂƃƄƅƆƇƈƉƊƋƌƍƎƏƐƑƒƓƔƕƖƗƘƙƚƛƜƝƞƟƠơƢƣƤƥƦƧƨƩƪƫƬƭƮƯưƱƲƳƴƵƶƷƸƹƺƻƼƽƾƿǀǁǂǃǄǅǆǇǈǉǊǋǌǍǎǏǐǑǒǓǔǕǖǗǘǙǚǛǜǝǞǟǠǡǢǣǤǥǦǧǨǩǪǫǬǭǮǯǰǱǲǳǴǵǶǷǸǹǺǻǼǽǾǿȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟȠȡȢȣȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿɀɁɂɃɄɅɆɇɈɉɊɋɌɍɎɏ");
	expected.SetString(L"ƀƀƂƂƄƄƆƇƇƉƉƋƋƍƎƏƐƑƑƓƔǶƖƗƘƘƚƛƜƝƞƟƠƠƢƢƤƤƦƧƧƩƪƫƬƬƮƯƯƱƲƳƳƵƵƷƸƸƺƻƼƼƾǷǀǁǂǃǄǄǄǇǇǇǊǊǊǍǍǏǏǑǑǓǓǕǕǗǗǙǙǛǛƎǞǞǠǠǢǢǤǤǦǦǨǨǪǪǬǬǮǮǰǱǱǱǴǴǶǷǸǸǺǺǼǼǾǾȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟȠȡȢȣȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿɀɁɂɃɄɅɆɇɈɉɊɋɌɍɎɏ");
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended Additional
	actual.SetString(L"ḀḁḂḃḄḅḆḇḈḉḊḋḌḍḎḏḐḑḒḓḔḕḖḗḘḙḚḛḜḝḞḟḠḡḢḣḤḥḦḧḨḩḪḫḬḭḮḯḰḱḲḳḴḵḶḷḸḹḺḻḼḽḾḿṀṁṂṃṄṅṆṇṈṉṊṋṌṍṎṏṐṑṒṓṔṕṖṗṘṙṚṛṜṝṞṟṠṡṢṣṤṥṦṧṨṩṪṫṬṭṮṯṰṱṲṳṴṵṶṷṸṹṺṻṼṽṾṿẀẁẂẃẄẅẆẇẈẉẊẋẌẍẎẏẐẑẒẓẔẕẖẗẘẙẚẛẜẝẞẟẠạẢảẤấẦầẨẩẪẫẬậẮắẰằẲẳẴẵẶặẸẹẺẻẼẽẾếỀềỂểỄễỆệỈỉỊịỌọỎỏỐốỒồỔổỖỗỘộỚớỜờỞởỠỡỢợỤụỦủỨứỪừỬửỮữỰựỲỳỴỵỶỷỸỹỺỻỼỽỾỿ");
	expected.SetString(L"ḀḀḂḂḄḄḆḆḈḈḊḊḌḌḎḎḐḐḒḒḔḔḖḖḘḘḚḚḜḜḞḞḠḠḢḢḤḤḦḦḨḨḪḪḬḬḮḮḰḰḲḲḴḴḶḶḸḸḺḺḼḼḾḾṀṀṂṂṄṄṆṆṈṈṊṊṌṌṎṎṐṐṒṒṔṔṖṖṘṘṚṚṜṜṞṞṠṠṢṢṤṤṦṦṨṨṪṪṬṬṮṮṰṰṲṲṴṴṶṶṸṸṺṺṼṼṾṾẀẀẂẂẄẄẆẆẈẈẊẊẌẌẎẎẐẐẒẒẔẔẖẗẘẙẚẛẜẝẞẟẠẠẢẢẤẤẦẦẨẨẪẪẬẬẮẮẰẰẲẲẴẴẶẶẸẸẺẺẼẼẾẾỀỀỂỂỄỄỆỆỈỈỊỊỌỌỎỎỐỐỒỒỔỔỖỖỘỘỚỚỜỜỞỞỠỠỢỢỤỤỦỦỨỨỪỪỬỬỮỮỰỰỲỲỴỴỶỶỸỸỺỺỼỼỾỾ");
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToZenhira)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｯｬｭｮｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟｰﾞﾟ｡､｢｣･");
	expected.SetString(L"あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわをんぁぃぅぇぉっゃゅょゔがぎぐげござじずぜぞだぢづでどばびぶべぼヷヸヹヺぱぴぷぺぽー゛゜。、「」・");
	EXPECT_TRUE(CConvert_ToZenhira().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヰヱヲンァィゥェォッャュョヮヵヶヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポ。、「」・ﾞﾟ｡､｢｣･");
	expected.SetString(L"あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわゐゑをんぁぃぅぇぉっゃゅょゎヵヶゔがぎぐげござじずぜぞだぢづでどばびぶべぼヷヸヹヺぱぴぷぺぽ。、「」・゛゜。、「」・");
	EXPECT_TRUE(CConvert_ToZenhira().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_");
	expected.SetString(L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿");
	EXPECT_TRUE(CConvert_ToZenhira().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToZenkata)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｯｬｭｮｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟｰﾞﾟ｡､｢｣･");
	expected.SetString(L"アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲンァィゥェォッャュョヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポー゛゜。、「」・");
	EXPECT_TRUE(CConvert_ToZenkata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"ぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんゔゕゖゝゞ");
	expected.SetString(L"ァアィイゥウェエォオカガキギクグケゲコゴサザシジスズセゼソゾタダチヂッツヅテデトドナニヌネノハバパヒビピフブプヘベペホボポマミムメモャヤュユョヨラリルレロヮワヰヱヲンヴヵヶヽヾ");
	EXPECT_TRUE(CConvert_ToZenkata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_");
	expected.SetString(L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿");
	EXPECT_TRUE(CConvert_ToZenkata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToHankaku)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿");
	expected.SetString(L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_");
	EXPECT_TRUE(CConvert_ToHankaku().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"ぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんゔゕゖゝゞ");
	expected.SetString(L"ｧｱｨｲｩｳｪｴｫｵｶｶﾞｷｷﾞｸｸﾞｹｹﾞｺｺﾞｻｻﾞｼｼﾞｽｽﾞｾｾﾞｿｿﾞﾀﾀﾞﾁﾁﾞｯﾂﾂﾞﾃﾃﾞﾄﾄﾞﾅﾆﾇﾈﾉﾊﾊﾞﾊﾟﾋﾋﾞﾋﾟﾌﾌﾞﾌﾟﾍﾍﾞﾍﾟﾎﾎﾞﾎﾟﾏﾐﾑﾒﾓｬﾔｭﾕｮﾖﾗﾘﾙﾚﾛﾜﾜｲｴｦﾝｳﾞｶｹゝゞ");
	EXPECT_TRUE(CConvert_ToHankaku().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヰヱヲンァィゥェォッャュョヮヵヶヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポヽヾ");
	expected.SetString(L"ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｲｴｦﾝｧｨｩｪｫｯｬｭｮﾜｶｹｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟヽヾ");
	EXPECT_TRUE(CConvert_ToHankaku().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(L"。、「」・ﾞﾟ｡､｢｣･");
	expected.SetString(L"｡､｢｣･ﾞﾟ｡､｢｣･");
	EXPECT_TRUE(CConvert_ToHankaku().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, TabToSpace)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(L"");
	EXPECT_FALSE(CConvert_TabToSpace(4, 0, false).DoConvert(&actual));

	// タブだけ変換
	actual.SetString(L"\t");
	expected.SetString(L"    ");
	EXPECT_TRUE(CConvert_TabToSpace(4, 0, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// タブを中途桁から変換
	actual.SetString(L"\t");
	expected.SetString(L" ");
	EXPECT_TRUE(CConvert_TabToSpace(4, 3, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// タブの前に文字
	actual.SetString(L"a\t");
	expected.SetString(L"a   ");
	EXPECT_TRUE(CConvert_TabToSpace(4, 0, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 複数行
	actual.SetString(L"\t\n\t");
	expected.SetString(L"    \n    ");
	EXPECT_TRUE(CConvert_TabToSpace(4, 0, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, SpaceToTab)
{
	class FakeCache : public CCharWidthCache {
	public:
		bool CalcHankakuByFont(wchar_t ch) override {
			return ch != L'あ';
		}
	} cache;

	CNativeW actual;
	CNativeW expected;

	actual.SetString(L"");
	EXPECT_FALSE(CConvert_SpaceToTab(4, 0, false, cache).DoConvert(&actual));

	// 空白をタブに変換
	actual.SetString(L"        ");
	expected.SetString(L"\t\t");
	EXPECT_TRUE(CConvert_SpaceToTab(4, 0, false, cache).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// タブは変換しない
	actual.SetString(L"\t\t");
	expected.SetString(L"\t\t");
	EXPECT_TRUE(CConvert_SpaceToTab(4, 0, false, cache).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 空白の数がタブ幅より少ない場合は変換しない
	actual.SetString(L"  ");
	expected.SetString(L"  ");
	EXPECT_TRUE(CConvert_SpaceToTab(4, 0, false, cache).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 文字と空白の混在
	actual.SetString(L" aあ     b ");
	expected.SetString(L" aあ\t b ");
	EXPECT_TRUE(CConvert_SpaceToTab(8, 0, false, cache).DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, Trim)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(L"");
	EXPECT_TRUE(CConvert_Trim(false, false).DoConvert(&actual));

	// bLeftがtrueなら左の空白を削除する。
	actual.SetString(L"  やよ  ");
	expected.SetString(L"  やよ");
	EXPECT_TRUE(CConvert_Trim(false, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// bLeftがfalseなら右の空白を削除する。
	actual.SetString(L"  やよ  ");
	expected.SetString(L"やよ  ");
	EXPECT_TRUE(CConvert_Trim(true, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

//!変換テストのためのテストパラメータ型
using ConvTestParamType = std::tuple<EFunctionCode, std::wstring_view, std::wstring_view, bool>;

//!変換テストのためのフィクスチャクラス
struct ConvTest : public testing::TGuiAware<::testing::TestWithParam<ConvTestParamType>> {
	using Base = testing::TGuiAware<::testing::TestWithParam<ConvTestParamType>>;

	ConvTest() = default;
};

/*!
 * @brief 機能コードによるバッファ変換のテスト
 */
TEST_P(ConvTest, test)
{
	const auto eFuncCode = std::get<0>(GetParam());
	const auto source = std::get<1>(GetParam());
	const auto expected = std::get<2>(GetParam());
	const auto result = std::get<3>(GetParam());
	SEncodingConfig sEncodingConfig;
	CCharWidthCache cCharWidthCache;
	CConversionFacade facade(
		4,								// タブ幅(タブ幅が半角スペース何個分かを指定する)
		0,								// 変換開始桁位置
		false,							// 拡張改行コードを有効にするかどうか
		sEncodingConfig,				// 文字コード自動検出のオプション
		cCharWidthCache					// 文字幅キャッシュ
	);

	CNativeW cmemBuf(source);
	if (result) {
		EXPECT_TRUE(facade.ConvMemory(eFuncCode, cmemBuf));
	} else {
		EXPECT_MSGBOX(facade.ConvMemory(eFuncCode, cmemBuf), GSTR_APPNAME, L"変換でエラーが発生しました");
	}

	EXPECT_THAT(cmemBuf.GetStringPtr(), StrEq(expected.data()));
}

/*!
 * @brief パラメータテストをインスタンス化する
 *  各変換機能の正常系をチェックするパターンで実体化させる
 */
INSTANTIATE_TEST_SUITE_P(CConvert
	, ConvTest
	, ::testing::Values(
		ConvTestParamType{ F_TOLOWER,					L"AbＣｄ",						L"abｃｄ",						true },
		ConvTestParamType{ F_TOUPPER,					L"AbＣｄ",						L"ABＣＤ",						true },
		ConvTestParamType{ F_TOHANKAKU,					L"カナかなｶﾅ",					L"ｶﾅｶﾅｶﾅ",						true },
		ConvTestParamType{ F_TOHANKATA,					L"カナかなｶﾅ",					L"ｶﾅかなｶﾅ",					true },
		ConvTestParamType{ F_TOZENEI,					L"AbＣｄ",						L"ＡｂＣｄ",					true },
		ConvTestParamType{ F_TOHANEI,					L"AbＣｄ",						L"AbCd",						true },
		ConvTestParamType{ F_TOZENKAKUKATA,				L"カナかなｶﾅ",					L"カナカナカナ",				true },
		ConvTestParamType{ F_TOZENKAKUHIRA,				L"カナかなｶﾅ",					L"かなかなかな",				true },
		ConvTestParamType{ F_HANKATATOZENKATA,			L"カナかなｶﾅ",					L"カナかなカナ",				true },
		ConvTestParamType{ F_HANKATATOZENHIRA,			L"カナかなｶﾅ",					L"カナかなかな",				true },
		ConvTestParamType{ F_TABTOSPACE,				L"\t",							L"    ",						true },
		ConvTestParamType{ F_SPACETOTAB,				L"    ",						L"\t",							true },
		ConvTestParamType{ F_LTRIM,						L" x ",							L"x ",							true },
		ConvTestParamType{ F_RTRIM,						L" x ",							L" x",							true },
		ConvTestParamType{ F_CODECNV_EMAIL,				L"\x1b$B2=$1%i%C%?\x1b(B!!",	L"化けラッタ!!",				true },
		ConvTestParamType{ F_CODECNV_EUC2SJIS,			L"ｲｽ､ｱ･鬣ﾃ･ｿ!!",				L"化けラッタ!!",				true },
		ConvTestParamType{ F_CODECNV_UNICODE2SJIS,		L"\x16S",						L"化",							true },
		ConvTestParamType{ F_CODECNV_UNICODEBE2SJIS,	L"S\x16",						L"化",							true },
		ConvTestParamType{ F_CODECNV_UTF82SJIS,			L"蛹悶￠繝ｩ繝\xDC83\xDCE3ち!!",	L"化けラッタ!!",				true },
		ConvTestParamType{ F_CODECNV_UTF72SJIS,			L"+UxYwUTDpMMMwvwAhACE-",		L"化けラッタ!!",				true },
		ConvTestParamType{ F_CODECNV_AUTO2SJIS,			L"化けラッタ!!",				L"化けラッタ!!",				false },
		ConvTestParamType{ F_CODECNV_AUTO2SJIS,			L"\x1b$B2=$1%i%C%?\x1b(B!!",	L"化けラッタ!!",				true },
		ConvTestParamType{ F_CODECNV_AUTO2SJIS,			L"ｲｽ､ｱ･鬣ﾃ･ｿ!!",				L"化けラッタ!!",				true },
		ConvTestParamType{ F_CODECNV_SJIS2JIS,			L"化けラッタ!!",				L"\x1b$B2=$1%i%C%?\x1b(B!!",	true },
		ConvTestParamType{ F_CODECNV_SJIS2EUC,			L"化けラッタ!!",				L"ｲｽ､ｱ･鬣ﾃ･ｿ!!",				true },
		ConvTestParamType{ F_CODECNV_SJIS2UTF8,			L"化けラッタ!!",				L"蛹悶￠繝ｩ繝\xDC83\xDCE3ち!!",	true },
		ConvTestParamType{ F_CODECNV_SJIS2UTF7,			L"化けラッタ!!",				L"+UxYwUTDpMMMwvwAhACE-",		true }
	)
);

} // namespace convert
