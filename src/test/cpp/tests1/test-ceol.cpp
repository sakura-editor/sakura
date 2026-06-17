/*
	Copyright (C) 2021-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "basis/CEol.h"

#include "charset/CCodeFactory.h"

namespace basis {

/*!
	CEolのテスト
 */
TEST(CEol, CEol)
{
	// 初期値は none 
	CEol cEol;
	EXPECT_THAT(cEol.GetType(), Eq(EEolType::none));
	EXPECT_THAT(static_cast<EEolType>(cEol), Eq(EEolType::none));

	// 代入したら変更できる
	cEol = EEolType::line_feed;
	EXPECT_THAT(cEol.GetType(), Eq(EEolType::line_feed));

	// コピーの確認
	CEol cCopied = cEol;
	EXPECT_THAT(cCopied.GetType(), Eq(cEol.GetType()));

	// EEolTypeは変な値でも格納できる
	const auto eBadValue = static_cast<EEolType>(100);
	EXPECT_THAT(static_cast<int>(eBadValue), Eq(100));

	// CEolは変な値を格納できない（入れようとした場合CRLFになる）
	cEol = eBadValue;
	EXPECT_THAT(cEol.GetType(), Eq(EEolType::cr_and_lf));
}

//! EOLテストのためのパラメータ型
using CEolTestParam = std::tuple<EEolType, bool, std::wstring, std::wstring>;

//! EOLテストのためのフィクスチャクラス
class CEolTest : public ::testing::TestWithParam<CEolTestParam> {};

/*!
	CEolのテスト
 */
TEST_P(CEolTest, test)
{
	const auto  eEolType  = std::get<0>(GetParam());
	const auto  isValid   = std::get<1>(GetParam());
	const auto& eolName   = std::get<2>(GetParam());
	const auto& eolValue  = std::get<3>(GetParam());

	CEol cEol{ eEolType };

	EXPECT_THAT(cEol.GetType(),   Eq(eEolType));
	EXPECT_THAT(cEol.IsValid(),   Eq(isValid));
	EXPECT_THAT(cEol.GetValue2(), StrEq(eolValue));
	EXPECT_THAT(cEol.GetLen(),    Eq(int(std::size(eolValue))));
	EXPECT_THAT(cEol.GetName(),   StrEq(eolName));
}

/*!
 * @brief パラメータテストをインスタンス化する
 */
INSTANTIATE_TEST_SUITE_P(CEolCases
	, CEolTest
	, ::testing::Values(
		CEolTestParam{ EEolType::none,                false, L"改行無", {}        },
		CEolTestParam{ EEolType::cr_and_lf,           true,  L"CRLF",   L"\r\n"   },
		CEolTestParam{ EEolType::line_feed,           true,  L"LF",     L"\n"     },
		CEolTestParam{ EEolType::carriage_return,     true,  L"CR",     L"\r"     },
		CEolTestParam{ EEolType::next_line,           true,  L"NEL",    L"\x85",  },
		CEolTestParam{ EEolType::line_separator,      true,  L"LS",     L"\u2028" },
		CEolTestParam{ EEolType::paragraph_separator, true,  L"PS",     L"\u2029" }
	)
);

//! EOL検出テストのためのパラメータ型
using DetectEolTestParam = std::tuple<ECodeType, EEolType, CNativeW>;

//! EOL検出テストのためのフィクスチャクラス
class DetectEolTest : public ::testing::TestWithParam<DetectEolTestParam> {};

/*!
 * Eol検出テスト
 */
TEST_P(DetectEolTest, test)
{
	const auto  eCodeType = std::get<0>(GetParam());
	const auto  eEolType  = std::get<1>(GetParam());
	const auto& line      = std::get<2>(GetParam());

	CEol cEol{ static_cast<EEolType>(static_cast<int>(eEolType) + 1) };

	const auto pCode = CCodeFactory::CreateCodeBase(eCodeType);

	CMemory m;
	pCode->UnicodeToCode(line, &m);

	switch (eCodeType) {
	case CODE_UTF16LE: cEol.SetTypeByStringForFile_uni(LPCSTR(m.GetRawPtr()), size_t(m.GetRawLength())); break;
	case CODE_UTF16BE: cEol.SetTypeByStringForFile_unibe(LPCSTR(m.GetRawPtr()), size_t(m.GetRawLength())); break;
	default: FAIL(); break;
	}

	EXPECT_THAT(cEol, Eq(eEolType));
}

/*!
 * @brief パラメータテストをインスタンス化する
 */
INSTANTIATE_TEST_SUITE_P(DetectEolCases
	, DetectEolTest
	, ::testing::Values(
		DetectEolTestParam{ ECodeType::CODE_UTF16LE, EEolType::none,                L""       },
		DetectEolTestParam{ ECodeType::CODE_UTF16LE, EEolType::cr_and_lf,           L"\r\n"   },
		DetectEolTestParam{ ECodeType::CODE_UTF16LE, EEolType::line_feed,           L"\n"     },
		DetectEolTestParam{ ECodeType::CODE_UTF16LE, EEolType::carriage_return,     L"\r"     },
		DetectEolTestParam{ ECodeType::CODE_UTF16LE, EEolType::next_line,           L"\x85"   },
		DetectEolTestParam{ ECodeType::CODE_UTF16LE, EEolType::line_separator,      L"\u2028" },
		DetectEolTestParam{ ECodeType::CODE_UTF16LE, EEolType::paragraph_separator, L"\u2029" },
		DetectEolTestParam{ ECodeType::CODE_UTF16BE, EEolType::none,                L""       },
		DetectEolTestParam{ ECodeType::CODE_UTF16BE, EEolType::cr_and_lf,           L"\r\n"   },
		DetectEolTestParam{ ECodeType::CODE_UTF16BE, EEolType::line_feed,           L"\n"     },
		DetectEolTestParam{ ECodeType::CODE_UTF16BE, EEolType::carriage_return,     L"\r"     },
		DetectEolTestParam{ ECodeType::CODE_UTF16BE, EEolType::next_line,           L"\x85"   },
		DetectEolTestParam{ ECodeType::CODE_UTF16BE, EEolType::line_separator,      L"\u2028" },
		DetectEolTestParam{ ECodeType::CODE_UTF16BE, EEolType::paragraph_separator, L"\u2029" }
	)
);

} // namespace basis
