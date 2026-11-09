/*! @file
	@brief Mac 版单元测试的预编译头（代替 src/test/resources/pch.h）

	与原版的区别：
	- 不包含 env/DLLSHAREDATA.h（Mac 版核心库还没有共享数据）
	- 让 EXPECT_STREQ 等宏支持 char16_t 的字符串（Mac 上 WCHAR 是 char16_t）

	Copyright (C) 2026, Sakura Editor Organization
	SPDX-License-Identifier: Zlib
*/
#pragma once

#include "StdAfx.h"

#include <filesystem>
#include <string>

// GoogleTest の文字列比較マクロ（EXPECT_STREQ など）は char と wchar_t にしか対応していない。
// gtest.h より前に char16_t 版の比較関数を宣言しておく。
#include <gtest/gtest.h>

namespace testing {
namespace internal {

inline AssertionResult CmpHelperSTREQ(const char* lhs_expression, const char* rhs_expression,
                                      const char16_t* lhs, const char16_t* rhs)
{
	if (lhs == rhs || (lhs && rhs && sakura::u16::strcmp(lhs, rhs) == 0)) return AssertionSuccess();
	return EqFailure(lhs_expression, rhs_expression,
		PrintToString(lhs ? std::u16string(lhs) : std::u16string(u"(null)")),
		PrintToString(rhs ? std::u16string(rhs) : std::u16string(u"(null)")), false);
}

inline AssertionResult CmpHelperSTRNE(const char* s1_expression, const char* s2_expression,
                                      const char16_t* s1, const char16_t* s2)
{
	if (!(s1 == s2 || (s1 && s2 && sakura::u16::strcmp(s1, s2) == 0))) return AssertionSuccess();
	return AssertionFailure() << "Expected: (" << s1_expression << ") != (" << s2_expression
		<< "), actual: " << PrintToString(std::u16string(s1 ? s1 : u"")) << " vs " << PrintToString(std::u16string(s2 ? s2 : u""));
}

inline AssertionResult CmpHelperSTRCASEEQ(const char* lhs_expression, const char* rhs_expression,
                                          const char16_t* lhs, const char16_t* rhs)
{
	if (lhs == rhs || (lhs && rhs && sakura::u16::stricmp(lhs, rhs) == 0)) return AssertionSuccess();
	return EqFailure(lhs_expression, rhs_expression,
		PrintToString(lhs ? std::u16string(lhs) : std::u16string(u"(null)")),
		PrintToString(rhs ? std::u16string(rhs) : std::u16string(u"(null)")), true);
}

} // namespace internal
} // namespace testing

// gtest の Message（ASSERT_* << ... の追加メッセージ）に char16_t の文字列を流せるようにする
inline ::testing::Message& operator<<(::testing::Message& m, const char16_t* s)
{
	return m << (s ? sakura::u16::to_utf8(s) : std::string("(null)"));
}
inline ::testing::Message& operator<<(::testing::Message& m, char16_t* s)
{
	return m << static_cast<const char16_t*>(s);
}
inline ::testing::Message& operator<<(::testing::Message& m, const std::u16string& s)
{
	return m << sakura::u16::to_utf8(std::u16string_view(s));
}

// テストではGMockを使う前提にする
#include <gmock/gmock.h>

#include "Funccode_enum.h"

// 原版的 pch.h 通过 DLLSHAREDATA.h 间接包含的常用类型
#include "basis/CMyString.h"
#include "basis/SakuraBasis.h"

// gMock の文字列マッチャー（StrEq など）も char16_t に対応させる
namespace testing {
namespace sakura_internal {

class U16StrMatcher {
public:
	U16StrMatcher(std::u16string expected, bool expectEq, bool ignoreCase)
		: m_expected(std::move(expected)), m_expectEq(expectEq), m_ignoreCase(ignoreCase) {}

	template <typename T>
	bool MatchAndExplain(const T& value, MatchResultListener* listener) const {
		// gMock の StrEqualityMatcher と同じく、文字列に変換できるものは何でも受け付ける
		std::u16string actual;
		if constexpr (std::is_convertible_v<const T&, const char16_t*>) {
			const char16_t* p = value;
			if (!p) { *listener << "which is NULL"; return !m_expectEq; }
			actual = p;
		} else if constexpr (std::is_same_v<T, std::filesystem::path>) {
			actual = value.u16string();
		} else {
			actual = std::u16string(value);
		}
		const bool eq = m_ignoreCase ? sakura::u16::stricmp(actual.c_str(), m_expected.c_str()) == 0 : actual == m_expected;
		*listener << "which is " << PrintToString(actual);
		return eq == m_expectEq;
	}
	void DescribeTo(std::ostream* os) const {
		*os << (m_expectEq ? "is " : "isn't ") << (m_ignoreCase ? "equal (ignoring case) to " : "equal to ") << PrintToString(m_expected);
	}
	void DescribeNegationTo(std::ostream* os) const {
		*os << (m_expectEq ? "isn't " : "is ") << (m_ignoreCase ? "equal (ignoring case) to " : "equal to ") << PrintToString(m_expected);
	}

private:
	std::u16string m_expected;
	bool m_expectEq;
	bool m_ignoreCase;
};

} // namespace sakura_internal

inline auto StrEq(std::u16string_view s) { return MakePolymorphicMatcher(sakura_internal::U16StrMatcher(std::u16string(s), true, false)); }
inline auto StrNe(std::u16string_view s) { return MakePolymorphicMatcher(sakura_internal::U16StrMatcher(std::u16string(s), false, false)); }
inline auto StrCaseEq(std::u16string_view s) { return MakePolymorphicMatcher(sakura_internal::U16StrMatcher(std::u16string(s), true, true)); }
inline auto StrCaseNe(std::u16string_view s) { return MakePolymorphicMatcher(sakura_internal::U16StrMatcher(std::u16string(s), false, true)); }
inline auto StartsWith(std::u16string_view s) {
	return Truly([e = std::u16string(s)](const auto& v) {
		const std::u16string_view a(v);
		return a.substr(0, e.size()) == e;
	});
}
} // namespace testing

// マッチャーのusing（src/test/resources/pch.h と同じ）
using ::testing::_;
using ::testing::ContainsRegex;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::Invoke;
using ::testing::IsEmpty;
using ::testing::IsFalse;
using ::testing::IsNull;
using ::testing::IsTrue;
using ::testing::Le;
using ::testing::Lt;
using ::testing::MatchesRegex;
using ::testing::Ne;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StartsWith;
using ::testing::StrCaseEq;
using ::testing::StrCaseNe;
using ::testing::StrEq;
using ::testing::StrNe;
using ::testing::ThrowsMessage;
