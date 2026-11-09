/*
	Copyright (C) 2021-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "convert/CConvert.h"

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

#include "config/system_constants.h"

#if defined(_WIN32)	// Mac 版: エディタ画面を使うテストと tchar_convert のテストは Windows のみ（tchar_convert は移植計画のステップ 17 で移植する）
#include "util/os/MockUser32.hpp"
#include "util/tchar_convert.h"

#include "dlg/ModalDialogCloser.hpp"
#include "env/ShareDataTestSuite.hpp"
#include "window/EditorTestSuite.hpp"
#endif

TEST(CConvert, ZenkataToHankata)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(LTEXT("アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヰヱヲンァィゥェォッャュョヮヵヶヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポ。、「」・ "));
	expected.SetString(LTEXT("ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｲｴｦﾝｧｨｩｪｫｯｬｭｮﾜｶｹｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟ｡､｢｣･ "));
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 長音符は前の文字がカタカナだった場合に限って変換する。
	// 長音符が連続する場合は前の文字と同様に処理する。
	actual.SetString(LTEXT("ーーナーーベーーパーー。ー"));
	expected.SetString(LTEXT("ーーﾅｰｰﾍﾞｰｰﾊﾟｰｰ｡ー"));
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 濁点・半濁点は清音の後に現れた場合のみ変換する。
	actual.SetString(LTEXT("゛゜\u3099\u309A"));
	expected.SetString(LTEXT("゛゜\u3099\u309A"));
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("サ゛゛フ゜゜サ\u3099\u3099フ\u309A\u309A"));
	expected.SetString(LTEXT("ｻﾞ゛ﾌﾟ゜ｻﾞ\u3099ﾌﾟ\u309A"));
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("ザ゛プ゜ザ\u3099プ\u309A"));
	expected.SetString(LTEXT("ｻﾞ゛ﾌﾟ゜ｻﾞ\u3099ﾌﾟ\u309A"));
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("ー゛・゜"));
	expected.SetString(LTEXT("ー゛･゜"));
	EXPECT_TRUE(CConvert_ZenkataToHankata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, HankataToZenkata)
{
	CNativeW actual(LTEXT("ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｯｬｭｮｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟｰﾞﾟ｡､｢｣･"));
	CNativeW expected(LTEXT("アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲンァィゥェォッャュョヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポー゛゜。、「」・"));
	EXPECT_TRUE(CConvert_HankataToZenkata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, HankataToZenhira)
{
	CNativeW actual(LTEXT("ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｯｬｭｮｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟｰﾞﾟ｡､｢｣･ "));
	CNativeW expected(LTEXT("あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわをんぁぃぅぇぉっゃゅょゔがぎぐげござじずぜぞだぢづでどばびぶべぼわ゛ヸヹを゛ぱぴぷぺぽー゛゜。、「」・ "));
	EXPECT_TRUE(CConvert_HankataToZenhira().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, HaneisuToZeneisu)
{
	CNativeW actual(LTEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_"));
	CNativeW expected(LTEXT("ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿"));
	EXPECT_TRUE(CConvert_HaneisuToZeneisu().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ZeneisuToHaneisu)
{
	CNativeW actual(LTEXT("ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿"));
	CNativeW expected(LTEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_"));
	EXPECT_TRUE(CConvert_ZeneisuToHaneisu().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToLower)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(LTEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
	expected.SetString(LTEXT("abcdefghijklmnopqrstuvwxyz"));
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ"));
	expected.SetString(LTEXT("ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ"));
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// ギリシャ文字
	actual.SetString(LTEXT("ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ"));
	expected.SetString(LTEXT("αβγδεζηθικλμνξοπρστυφχψω"));
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// キリル文字
	actual.SetString(LTEXT("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"));
	expected.SetString(LTEXT("абвгдежзийклмнопрстуфхцчшщъыьэюя"));
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 拡張キリル文字
	actual.SetString(LTEXT("ЀЁЂЃЄЅІЇЈЉЊЋЌЍЎЏ"));
	expected.SetString(LTEXT("ѐёђѓєѕіїјљњћќѝўџ"));
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin-1
	actual.SetString(LTEXT("ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß"));
	expected.SetString(LTEXT("àáâãäåæçèéêëìíîïðñòóôõö×øùúûüýþß"));
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended-A
	actual.SetString(LTEXT("ĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽžſ"));
	expected.SetString(LTEXT("āāăăąąććĉĉċċččďďđđēēĕĕėėęęěěĝĝğğġġģģĥĥħħĩĩīīĭĭįįiıĳĳĵĵķķĸĺĺļļľľŀŀłłńńņņňňŉŋŋōōŏŏőőœœŕŕŗŗřřśśŝŝşşššţţťťŧŧũũūūŭŭůůűűųųŵŵŷŷÿźźżżžžſ"));
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended-B
	actual.SetString(LTEXT("ƀƁƂƃƄƅƆƇƈƉƊƋƌƍƎƏƐƑƒƓƔƕƖƗƘƙƚƛƜƝƞƟƠơƢƣƤƥƦƧƨƩƪƫƬƭƮƯưƱƲƳƴƵƶƷƸƹƺƻƼƽƾƿǀǁǂǃǄǅǆǇǈǉǊǋǌǍǎǏǐǑǒǓǔǕǖǗǘǙǚǛǜǝǞǟǠǡǢǣǤǥǦǧǨǩǪǫǬǭǮǯǰǱǲǳǴǵǶǷǸǹǺǻǼǽǾǿȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟȠȡȢȣȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿɀɁɂɃɄɅɆɇɈɉɊɋɌɍɎɏ"));
	expected.SetString(LTEXT("ƁƁƃƃƅƅƆƈƈƊƊƌƌƍǝƏƐƒƒƓƔƕƖƗƙƙƚƛƜƝƞƟơơƣƣƥƥƦƨƨƩƪƫƭƭƮưưƱƲƴƴƶƶƷƹƹƺƻƽƽƾƿǀǁǂǃǆǆǆǉǉǉǌǌǌǎǎǐǐǒǒǔǔǖǖǘǘǚǚǜǜǝǟǟǡǡǣǣǥǥǧǧǩǩǫǫǭǭǯǯǰǳǳǳǵǵƕƿǹǹǻǻǽǽǿǿȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟȠȡȢȣȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿɀɁɂɃɄɅɆɇɈɉɊɋɌɍɎɏ"));
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended Additional
	actual.SetString(LTEXT("ḀḁḂḃḄḅḆḇḈḉḊḋḌḍḎḏḐḑḒḓḔḕḖḗḘḙḚḛḜḝḞḟḠḡḢḣḤḥḦḧḨḩḪḫḬḭḮḯḰḱḲḳḴḵḶḷḸḹḺḻḼḽḾḿṀṁṂṃṄṅṆṇṈṉṊṋṌṍṎṏṐṑṒṓṔṕṖṗṘṙṚṛṜṝṞṟṠṡṢṣṤṥṦṧṨṩṪṫṬṭṮṯṰṱṲṳṴṵṶṷṸṹṺṻṼṽṾṿẀẁẂẃẄẅẆẇẈẉẊẋẌẍẎẏẐẑẒẓẔẕẖẗẘẙẚẛẜẝẞẟẠạẢảẤấẦầẨẩẪẫẬậẮắẰằẲẳẴẵẶặẸẹẺẻẼẽẾếỀềỂểỄễỆệỈỉỊịỌọỎỏỐốỒồỔổỖỗỘộỚớỜờỞởỠỡỢợỤụỦủỨứỪừỬửỮữỰựỲỳỴỵỶỷỸỹỺỻỼỽỾỿ"));
	expected.SetString(LTEXT("ḁḁḃḃḅḅḇḇḉḉḋḋḍḍḏḏḑḑḓḓḕḕḗḗḙḙḛḛḝḝḟḟḡḡḣḣḥḥḧḧḩḩḫḫḭḭḯḯḱḱḳḳḵḵḷḷḹḹḻḻḽḽḿḿṁṁṃṃṅṅṇṇṉṉṋṋṍṍṏṏṑṑṓṓṕṕṗṗṙṙṛṛṝṝṟṟṡṡṣṣṥṥṧṧṩṩṫṫṭṭṯṯṱṱṳṳṵṵṷṷṹṹṻṻṽṽṿṿẁẁẃẃẅẅẇẇẉẉẋẋẍẍẏẏẑẑẓẓẕẕẖẗẘẙẚẛẜẝẞẟạạảảấấầầẩẩẫẫậậắắằằẳẳẵẵặặẹẹẻẻẽẽếếềềểểễễệệỉỉịịọọỏỏốốồồổổỗỗộộớớờờởởỡỡợợụụủủứứừừửửữữựựỳỳỵỵỷỷỹỹỻỻỽỽỿỿ"));
	EXPECT_TRUE(CConvert_ToLower().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToUpper)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(LTEXT("abcdefghijklmnopqrstuvwxyz"));
	expected.SetString(LTEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ"));
	expected.SetString(LTEXT("ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ"));
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// ギリシャ文字
	actual.SetString(LTEXT("αβγδεζηθικλμνξοπρστυφχψω"));
	expected.SetString(LTEXT("ΑΒΓΔΕΖΗΘΙΚΛΜΝΞΟΠΡΣΤΥΦΧΨΩ"));
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// キリル文字
	actual.SetString(LTEXT("абвгдежзийклмнопрстуфхцчшщъыьэюя"));
	expected.SetString(LTEXT("АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ"));
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 拡張キリル文字
	actual.SetString(LTEXT("ѐёђѓєѕіїјљњћќѝўџ"));
	expected.SetString(LTEXT("ЀЁЂЃЄЅІЇЈЉЊЋЌЍЎЏ"));
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin-1
	actual.SetString(LTEXT("àáâãäåæçèéêëìíîïðñòóôõö×øùúûüýþÿ"));
	expected.SetString(LTEXT("ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞŸ"));
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended-A
	actual.SetString(LTEXT("ĀāĂăĄąĆćĈĉĊċČčĎďĐđĒēĔĕĖėĘęĚěĜĝĞğĠġĢģĤĥĦħĨĩĪīĬĭĮįİıĲĳĴĵĶķĸĹĺĻļĽľĿŀŁłŃńŅņŇňŉŊŋŌōŎŏŐőŒœŔŕŖŗŘřŚśŜŝŞşŠšŢţŤťŦŧŨũŪūŬŭŮůŰűŲųŴŵŶŷŸŹźŻżŽžſ"));
	expected.SetString(LTEXT("ĀĀĂĂĄĄĆĆĈĈĊĊČČĎĎĐĐĒĒĔĔĖĖĘĘĚĚĜĜĞĞĠĠĢĢĤĤĦĦĨĨĪĪĬĬĮĮİIĲĲĴĴĶĶĸĹĹĻĻĽĽĿĿŁŁŃŃŅŅŇŇŉŊŊŌŌŎŎŐŐŒŒŔŔŖŖŘŘŚŚŜŜŞŞŠŠŢŢŤŤŦŦŨŨŪŪŬŬŮŮŰŰŲŲŴŴŶŶŸŹŹŻŻŽŽſ"));
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended-B
	actual.SetString(LTEXT("ƀƁƂƃƄƅƆƇƈƉƊƋƌƍƎƏƐƑƒƓƔƕƖƗƘƙƚƛƜƝƞƟƠơƢƣƤƥƦƧƨƩƪƫƬƭƮƯưƱƲƳƴƵƶƷƸƹƺƻƼƽƾƿǀǁǂǃǄǅǆǇǈǉǊǋǌǍǎǏǐǑǒǓǔǕǖǗǘǙǚǛǜǝǞǟǠǡǢǣǤǥǦǧǨǩǪǫǬǭǮǯǰǱǲǳǴǵǶǷǸǹǺǻǼǽǾǿȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟȠȡȢȣȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿɀɁɂɃɄɅɆɇɈɉɊɋɌɍɎɏ"));
	expected.SetString(LTEXT("ƀƀƂƂƄƄƆƇƇƉƉƋƋƍƎƏƐƑƑƓƔǶƖƗƘƘƚƛƜƝƞƟƠƠƢƢƤƤƦƧƧƩƪƫƬƬƮƯƯƱƲƳƳƵƵƷƸƸƺƻƼƼƾǷǀǁǂǃǄǄǄǇǇǇǊǊǊǍǍǏǏǑǑǓǓǕǕǗǗǙǙǛǛƎǞǞǠǠǢǢǤǤǦǦǨǨǪǪǬǬǮǮǰǱǱǱǴǴǶǷǸǸǺǺǼǼǾǾȀȁȂȃȄȅȆȇȈȉȊȋȌȍȎȏȐȑȒȓȔȕȖȗȘșȚțȜȝȞȟȠȡȢȣȤȥȦȧȨȩȪȫȬȭȮȯȰȱȲȳȴȵȶȷȸȹȺȻȼȽȾȿɀɁɂɃɄɅɆɇɈɉɊɋɌɍɎɏ"));
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// Latin Extended Additional
	actual.SetString(LTEXT("ḀḁḂḃḄḅḆḇḈḉḊḋḌḍḎḏḐḑḒḓḔḕḖḗḘḙḚḛḜḝḞḟḠḡḢḣḤḥḦḧḨḩḪḫḬḭḮḯḰḱḲḳḴḵḶḷḸḹḺḻḼḽḾḿṀṁṂṃṄṅṆṇṈṉṊṋṌṍṎṏṐṑṒṓṔṕṖṗṘṙṚṛṜṝṞṟṠṡṢṣṤṥṦṧṨṩṪṫṬṭṮṯṰṱṲṳṴṵṶṷṸṹṺṻṼṽṾṿẀẁẂẃẄẅẆẇẈẉẊẋẌẍẎẏẐẑẒẓẔẕẖẗẘẙẚẛẜẝẞẟẠạẢảẤấẦầẨẩẪẫẬậẮắẰằẲẳẴẵẶặẸẹẺẻẼẽẾếỀềỂểỄễỆệỈỉỊịỌọỎỏỐốỒồỔổỖỗỘộỚớỜờỞởỠỡỢợỤụỦủỨứỪừỬửỮữỰựỲỳỴỵỶỷỸỹỺỻỼỽỾỿ"));
	expected.SetString(LTEXT("ḀḀḂḂḄḄḆḆḈḈḊḊḌḌḎḎḐḐḒḒḔḔḖḖḘḘḚḚḜḜḞḞḠḠḢḢḤḤḦḦḨḨḪḪḬḬḮḮḰḰḲḲḴḴḶḶḸḸḺḺḼḼḾḾṀṀṂṂṄṄṆṆṈṈṊṊṌṌṎṎṐṐṒṒṔṔṖṖṘṘṚṚṜṜṞṞṠṠṢṢṤṤṦṦṨṨṪṪṬṬṮṮṰṰṲṲṴṴṶṶṸṸṺṺṼṼṾṾẀẀẂẂẄẄẆẆẈẈẊẊẌẌẎẎẐẐẒẒẔẔẖẗẘẙẚẛẜẝẞẟẠẠẢẢẤẤẦẦẨẨẪẪẬẬẮẮẰẰẲẲẴẴẶẶẸẸẺẺẼẼẾẾỀỀỂỂỄỄỆỆỈỈỊỊỌỌỎỎỐỐỒỒỔỔỖỖỘỘỚỚỜỜỞỞỠỠỢỢỤỤỦỦỨỨỪỪỬỬỮỮỰỰỲỲỴỴỶỶỸỸỺỺỼỼỾỾ"));
	EXPECT_TRUE(CConvert_ToUpper().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToZenhira)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(LTEXT("ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｯｬｭｮｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟｰﾞﾟ｡､｢｣･"));
	expected.SetString(LTEXT("あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわをんぁぃぅぇぉっゃゅょゔがぎぐげござじずぜぞだぢづでどばびぶべぼヷヸヹヺぱぴぷぺぽー゛゜。、「」・"));
	EXPECT_TRUE(CConvert_ToZenhira().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヰヱヲンァィゥェォッャュョヮヵヶヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポ。、「」・ﾞﾟ｡､｢｣･"));
	expected.SetString(LTEXT("あいうえおかきくけこさしすせそたちつてとなにぬねのはひふへほまみむめもやゆよらりるれろわゐゑをんぁぃぅぇぉっゃゅょゎヵヶゔがぎぐげござじずぜぞだぢづでどばびぶべぼヷヸヹヺぱぴぷぺぽ。、「」・゛゜。、「」・"));
	EXPECT_TRUE(CConvert_ToZenhira().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_"));
	expected.SetString(LTEXT("ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿"));
	EXPECT_TRUE(CConvert_ToZenhira().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToZenkata)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(LTEXT("ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｦﾝｧｨｩｪｫｯｬｭｮｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟｰﾞﾟ｡､｢｣･"));
	expected.SetString(LTEXT("アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヲンァィゥェォッャュョヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポー゛゜。、「」・"));
	EXPECT_TRUE(CConvert_ToZenkata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("ぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんゔゕゖゝゞ"));
	expected.SetString(LTEXT("ァアィイゥウェエォオカガキギクグケゲコゴサザシジスズセゼソゾタダチヂッツヅテデトドナニヌネノハバパヒビピフブプヘベペホボポマミムメモャヤュユョヨラリルレロヮワヰヱヲンヴヵヶヽヾ"));
	EXPECT_TRUE(CConvert_ToZenkata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_"));
	expected.SetString(LTEXT("ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿"));
	EXPECT_TRUE(CConvert_ToZenkata().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, ToHankaku)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(LTEXT("ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ０１２３４５６７８９　，．＋－＊／％＝｜＆＾￥＠；：”‘’＜＞（）｛｝［］！？＃＄￣＿"));
	expected.SetString(LTEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 ,.+-*/%=|&^\\@;:\"`'<>(){}[]!?#$~_"));
	EXPECT_TRUE(CConvert_ToHankaku().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("ぁあぃいぅうぇえぉおかがきぎくぐけげこごさざしじすずせぜそぞただちぢっつづてでとどなにぬねのはばぱひびぴふぶぷへべぺほぼぽまみむめもゃやゅゆょよらりるれろゎわゐゑをんゔゕゖゝゞ"));
	expected.SetString(LTEXT("ｧｱｨｲｩｳｪｴｫｵｶｶﾞｷｷﾞｸｸﾞｹｹﾞｺｺﾞｻｻﾞｼｼﾞｽｽﾞｾｾﾞｿｿﾞﾀﾀﾞﾁﾁﾞｯﾂﾂﾞﾃﾃﾞﾄﾄﾞﾅﾆﾇﾈﾉﾊﾊﾞﾊﾟﾋﾋﾞﾋﾟﾌﾌﾞﾌﾟﾍﾍﾞﾍﾟﾎﾎﾞﾎﾟﾏﾐﾑﾒﾓｬﾔｭﾕｮﾖﾗﾘﾙﾚﾛﾜﾜｲｴｦﾝｳﾞｶｹゝゞ"));
	EXPECT_TRUE(CConvert_ToHankaku().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("アイウエオカキクケコサシスセソタチツテトナニヌネノハヒフヘホマミムメモヤユヨラリルレロワヰヱヲンァィゥェォッャュョヮヵヶヴガギグゲゴザジズゼゾダヂヅデドバビブベボヷヸヹヺパピプペポヽヾ"));
	expected.SetString(LTEXT("ｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜｲｴｦﾝｧｨｩｪｫｯｬｭｮﾜｶｹｳﾞｶﾞｷﾞｸﾞｹﾞｺﾞｻﾞｼﾞｽﾞｾﾞｿﾞﾀﾞﾁﾞﾂﾞﾃﾞﾄﾞﾊﾞﾋﾞﾌﾞﾍﾞﾎﾞﾜﾞｲﾞｴﾞｦﾞﾊﾟﾋﾟﾌﾟﾍﾟﾎﾟヽヾ"));
	EXPECT_TRUE(CConvert_ToHankaku().DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	actual.SetString(LTEXT("。、「」・ﾞﾟ｡､｢｣･"));
	expected.SetString(LTEXT("｡､｢｣･ﾞﾟ｡､｢｣･"));
	EXPECT_TRUE(CConvert_ToHankaku().DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, TabToSpace)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(LTEXT(""));
	EXPECT_FALSE(CConvert_TabToSpace(4, 0, false).DoConvert(&actual));

	// タブだけ変換
	actual.SetString(LTEXT("\t"));
	expected.SetString(LTEXT("    "));
	EXPECT_TRUE(CConvert_TabToSpace(4, 0, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// タブを中途桁から変換
	actual.SetString(LTEXT("\t"));
	expected.SetString(LTEXT(" "));
	EXPECT_TRUE(CConvert_TabToSpace(4, 3, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// タブの前に文字
	actual.SetString(LTEXT("a\t"));
	expected.SetString(LTEXT("a   "));
	EXPECT_TRUE(CConvert_TabToSpace(4, 0, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 複数行
	actual.SetString(LTEXT("\t\n\t"));
	expected.SetString(LTEXT("    \n    "));
	EXPECT_TRUE(CConvert_TabToSpace(4, 0, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, SpaceToTab)
{
	class FakeCache : public CCharWidthCache {
	public:
		bool CalcHankakuByFont(WCHAR ch) override {
			return ch != LCHAR('あ');
		}
	} cache;

	CNativeW actual;
	CNativeW expected;

	actual.SetString(LTEXT(""));
	EXPECT_FALSE(CConvert_SpaceToTab(4, 0, false, cache).DoConvert(&actual));

	// 空白をタブに変換
	actual.SetString(LTEXT("        "));
	expected.SetString(LTEXT("\t\t"));
	EXPECT_TRUE(CConvert_SpaceToTab(4, 0, false, cache).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// タブは変換しない
	actual.SetString(LTEXT("\t\t"));
	expected.SetString(LTEXT("\t\t"));
	EXPECT_TRUE(CConvert_SpaceToTab(4, 0, false, cache).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 空白の数がタブ幅より少ない場合は変換しない
	actual.SetString(LTEXT("  "));
	expected.SetString(LTEXT("  "));
	EXPECT_TRUE(CConvert_SpaceToTab(4, 0, false, cache).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// 文字と空白の混在
	actual.SetString(LTEXT(" aあ     b "));
	expected.SetString(LTEXT(" aあ\t b "));
	EXPECT_TRUE(CConvert_SpaceToTab(8, 0, false, cache).DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

TEST(CConvert, Trim)
{
	CNativeW actual;
	CNativeW expected;

	actual.SetString(LTEXT(""));
	EXPECT_TRUE(CConvert_Trim(false, false).DoConvert(&actual));

	// bLeftがtrueなら左の空白を削除する。
	actual.SetString(LTEXT("  やよ  "));
	expected.SetString(LTEXT("  やよ"));
	EXPECT_TRUE(CConvert_Trim(false, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);

	// bLeftがfalseなら右の空白を削除する。
	actual.SetString(LTEXT("  やよ  "));
	expected.SetString(LTEXT("やよ  "));
	EXPECT_TRUE(CConvert_Trim(true, false).DoConvert(&actual));
	EXPECT_EQ(actual, expected);
}

#if defined(_MSC_VER) && defined(_DEBUG)

//!変換テストのためのテストパラメータ型
using ConvTestParamType = std::tuple<EFunctionCode, sakura::wstring_view, sakura::wstring_view>;

//!変換テストのためのフィクスチャクラス
struct ConvTest : public ::testing::TestWithParam<ConvTestParamType>, public window::EditorTestSuite, public window::UiaTestSuite {
	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		SetUpUiaTestSuite();

		SetUpEditor();
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		TearDownEditor();

		TearDownUiaTestSuite();
	}

	/*!
	 * テストが実行される直前に毎回呼ばれる関数
	 */
	void SetUp() override
	{
		User32::setInstance<MockUser32>();

		auto pUser32 = (MockUser32*)User32::getInstance();

		EXPECT_CALL(*pUser32, MessageBoxExW(_, _, _, _, _))
			.WillRepeatedly(Return(MB_OK));
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override
	{
		// 強制的に「編集なし」にする
		pcEditDoc->m_cDocEditor.m_bIsDocModified = false;

		// キューに溜まったメッセージは全部捨てる
		MSG msg{};
		while (::PeekMessageW(&msg, nullptr, 0L, 0L, PM_REMOVE)) ;

		User32::resetInstance();

		TearDownUia();
	}

};

/*!
 * @brief 機能コードによるバッファ変換のテスト
 */
TEST_P(ConvTest, test)
{
	const sakura::wstring expected{ std::get<2>(GetParam()) };

	const auto hWnd = pcEditWnd->GetHwnd();

	// 全選択してゴミを削除する
	FORWARD_WM_COMMAND(hWnd, F_SELECTALL, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);
	FORWARD_WM_COMMAND(hWnd, F_DELETE, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	auto pShareData = GetDllShareDataPtr();
	auto& sWorkBuffer = pShareData->m_sWorkBuffer;
	auto buffer = std::span(sWorkBuffer.GetWorkBuffer<WCHAR>(), sWorkBuffer.GetWorkBufferCount<WCHAR>());

	const auto source = std::get<1>(GetParam());
	::wcsncpy_s(std::data(buffer), std::size(buffer), std::data(source), std::size(source));

	// 末尾にデータを入れて全選択する
	pcEditWnd->DispatchEvent(hWnd, MYWM_ADDSTRINGLEN_W, std::size(source), 0L);
	FORWARD_WM_COMMAND(hWnd, F_SELECTALL, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	// バッファ変換機能を呼び出す
	const auto eFuncCode = std::get<0>(GetParam());
	FORWARD_WM_COMMAND(hWnd, eFuncCode, nullptr, BN_CLICKED, pcEditWnd->DispatchEvent);

	// 全データを取得する
	const auto nLineLen = static_cast<size_t>(pcEditWnd->DispatchEvent(hWnd, MYWM_GETLINEDATA, 0L, 0L));
	sakura::wstring_view actual{ std::data(buffer), nLineLen };
	EXPECT_THAT(actual, StrEq(expected));
}

/*!
 * @brief パラメータテストをインスタンス化する
 *  各変換機能の正常系をチェックするパターンで実体化させる
 */
INSTANTIATE_TEST_CASE_P(ParameterizedTestConv
	, ConvTest
	, ::testing::Values(
		ConvTestParamType{ F_TOLOWER,					LTEXT("AbＣｄ"),			LTEXT("abｃｄ") },
		ConvTestParamType{ F_TOUPPER,					LTEXT("AbＣｄ"),			LTEXT("ABＣＤ") },
		ConvTestParamType{ F_TOHANKAKU,					LTEXT("カナかなｶﾅ"),		LTEXT("ｶﾅｶﾅｶﾅ") },
		ConvTestParamType{ F_TOHANKATA,					LTEXT("カナかなｶﾅ"),		LTEXT("ｶﾅかなｶﾅ") },
		ConvTestParamType{ F_TOZENEI,					LTEXT("AbＣｄ"),			LTEXT("ＡｂＣｄ") },
		ConvTestParamType{ F_TOHANEI,					LTEXT("AbＣｄ"),			LTEXT("AbCd") },
		ConvTestParamType{ F_TOZENKAKUKATA,				LTEXT("カナかなｶﾅ"),		LTEXT("カナカナカナ") },
		ConvTestParamType{ F_TOZENKAKUHIRA,				LTEXT("カナかなｶﾅ"),		LTEXT("かなかなかな") },
		ConvTestParamType{ F_HANKATATOZENKATA,			LTEXT("カナかなｶﾅ"),		LTEXT("カナかなカナ") },
		ConvTestParamType{ F_HANKATATOZENHIRA,			LTEXT("カナかなｶﾅ"),		LTEXT("カナかなかな") },
		ConvTestParamType{ F_TABTOSPACE,				LTEXT("\t"),				LTEXT("    ") },
		ConvTestParamType{ F_SPACETOTAB,				LTEXT("    "),			LTEXT("\t") },
		ConvTestParamType{ F_LTRIM,						LTEXT(" x "),				LTEXT("x ") },
		ConvTestParamType{ F_RTRIM,						LTEXT(" x "),				LTEXT(" x") },
		ConvTestParamType{ F_CODECNV_EMAIL,				LTEXT("\x1b$B2=$1%i%C%?\x1b(B!!"),	LTEXT("化けラッタ!!") },
		ConvTestParamType{ F_CODECNV_EUC2SJIS,			LTEXT("ｲｽ､ｱ･鬣ﾃ･ｿ!!"),				LTEXT("化けラッタ!!") },
		ConvTestParamType{ F_CODECNV_UNICODE2SJIS,		LTEXT(""),							LTEXT("") },							//FIXME: 機能しないため、呼出確認のみ。
		ConvTestParamType{ F_CODECNV_UNICODEBE2SJIS,	LTEXT(""),							LTEXT("") },							//FIXME: 機能しないため、呼出確認のみ
		ConvTestParamType{ F_CODECNV_UTF82SJIS,			LTEXT(""),							LTEXT("") },							//FIXME: 機能しないため、呼出確認のみ
		ConvTestParamType{ F_CODECNV_UTF72SJIS,			LTEXT("+UxYwUTDpMMMwvwAhACE-"),		LTEXT("化けラッタ!!") },
		ConvTestParamType{ F_CODECNV_AUTO2SJIS,			LTEXT("化けラッタ!!"),				LTEXT("化けラッタ!!") },
		ConvTestParamType{ F_CODECNV_AUTO2SJIS,			LTEXT("\x1b$B2=$1%i%C%?\x1b(B!!"),	LTEXT("化けラッタ!!") },
		ConvTestParamType{ F_CODECNV_AUTO2SJIS,			LTEXT("ｲｽ､ｱ･鬣ﾃ･ｿ!!"),				LTEXT("化けラッタ!!") },
		ConvTestParamType{ F_CODECNV_SJIS2JIS,			LTEXT("化けラッタ!!"),				LTEXT("\x1b$B2=$1%i%C%?\x1b(B!!") },
		ConvTestParamType{ F_CODECNV_SJIS2EUC,			LTEXT("化けラッタ!!"),				LTEXT("ｲｽ､ｱ･鬣ﾃ･ｿ!!") },
		ConvTestParamType{ F_CODECNV_SJIS2UTF8,			LTEXT(""),							LTEXT("") },							//FIXME: 機能しないため、呼出確認のみ
		ConvTestParamType{ F_CODECNV_SJIS2UTF7,			LTEXT("化けラッタ!!"),				LTEXT("+UxYwUTDpMMMwvwAhACE-") }
	)
);

#endif // if defined(_MSC_VER) && defined(_DEBUG)

#if defined(_WIN32)	// Mac 版: tchar_convert は移植計画のステップ 17 で移植する

namespace cxx {

TEST(to_achar, test001)
{
	// ASCIIのみの文字列
	EXPECT_THAT(to_achar(LTEXT("abc123")), StrEq("abc123"));
}

TEST(to_achar, test002)
{
	// 日本語を含む文字列
	EXPECT_THAT(to_achar(LTEXT("あい")), StrEq("あい"));
}

TEST(to_achar, test101)
{
	// 空文字列
	EXPECT_THAT(to_achar(LTEXT("")), StrEq(""));
}

TEST(to_achar, test102)
{
	// 長さ0の文字列
	EXPECT_THAT(to_achar(nullptr, 0), StrEq(""));
}

TEST(to_achar, test103)
{
	// 不正値(NULL)
	EXPECT_THAT(to_achar(nullptr), StrEq(""));
}

TEST(to_string, test001)
{
	// 空文字列
	EXPECT_THAT(cxx::to_string(LTEXT("")), StrEq(""));
}

TEST(to_string, test002)
{
	// ロケールを設定
	::SetThreadUILanguage(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN));


	// 日本語を含む文字列
	EXPECT_THAT(cxx::to_string(LTEXT("ABCabc123あいう愛生")), StrEq("ABCabc123あいう愛生"));
}

TEST(to_string, test003)
{
	// 日本語を含む文字列をUTF8に変換
	EXPECT_THAT(cxx::to_string(LTEXT("ABCabc123あいう愛生"), CP_UTF8), StrEq((LPCSTR)u8"ABCabc123あいう愛生"));
}

TEST(to_string, test004)
{
	// カラー絵文字「男性のシンボル」をUTF8に変換
	EXPECT_THAT(cxx::to_string(LTEXT("\U0001F6B9"), CP_UTF8), StrEq((LPCSTR)u8"\U0001F6B9"));
}

TEST(to_string, test101)
{
	// ロケールを設定
	::SetThreadUILanguage(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN));

	// カラー絵文字「男性のシンボル」（サロゲートペアはSJISに変換できない）
	EXPECT_ANY_THROW(cxx::to_string(LTEXT("\U0001F6B9")));
}

TEST(to_wchar, test001)
{
	// ASCIIのみの文字列
	EXPECT_THAT(to_wchar("abc123"), StrEq(LTEXT("abc123")));
}

TEST(to_wchar, test002)
{
	// 日本語を含む文字列
	EXPECT_THAT(to_wchar("あい"), StrEq(LTEXT("あい")));
}

TEST(to_wchar, test101)
{
	// 空文字列
	EXPECT_THAT(to_wchar(""), StrEq(LTEXT("")));
}

TEST(to_wchar, test102)
{
	// 長さ0の文字列
	EXPECT_THAT(to_wchar(nullptr, 0), StrEq(LTEXT("")));
}

TEST(to_wchar, test103)
{
	// 不正値(NULL)
	EXPECT_THAT(to_wchar(nullptr), StrEq(LTEXT("")));
}

TEST(to_wstring, test001)
{
	// 空文字列
	EXPECT_THAT(cxx::to_wstring(""), StrEq(LTEXT("")));
}

TEST(to_wstring, test002)
{
	// ロケールを設定
	::SetThreadUILanguage(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN));


	// 日本語を含む文字列
	EXPECT_THAT(cxx::to_wstring("ABCabc123あいう愛生"), StrEq(LTEXT("ABCabc123あいう愛生")));
}

TEST(to_wstring, test003)
{
	// 日本語を含む文字列をUTF8に変換
	EXPECT_THAT(cxx::to_wstring((LPCSTR)u8"ABCabc123あいう愛生", CP_UTF8), StrEq(LTEXT("ABCabc123あいう愛生")));
}

TEST(to_wstring, test004)
{
	// カラー絵文字「男性のシンボル」をUTF8に変換
	EXPECT_THAT(cxx::to_wstring((LPCSTR)u8"\U0001F6B9", CP_UTF8), StrEq(LTEXT("\U0001F6B9")));
}

TEST(to_wstring, test101)
{
	// ロケールを設定
	::SetThreadUILanguage(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN));

	// SJIS仕様違反 先行バイトのみ
	EXPECT_ANY_THROW(cxx::to_wstring("\x81"));

	// SJIS仕様違反 後続バイト値が不正
	EXPECT_ANY_THROW(cxx::to_wstring("\x81\x7F"));

	// SJIS仕様違反 後続バイト値が範囲外1
	EXPECT_ANY_THROW(cxx::to_wstring("\x81\x3F"));

	// SJIS仕様違反 後続バイト値が範囲外2
	EXPECT_ANY_THROW(cxx::to_wstring("\x81\xFD"));
}

TEST(WideCharToMultiByte, test101)
{
	std::string buffer = "dummy";

	// 空文字列
	EXPECT_THAT(cxx::WideCharToMultiByte(CP_UTF8, LTEXT(""), buffer), Eq(0));
	EXPECT_THAT(buffer, StrEq(""));
}

} // namespace cxx

#endif // defined(_WIN32)
