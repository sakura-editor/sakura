/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#include "util/tchar_convert.h"

namespace testing {

/*!
 * @brief 既知のHRESULT値に対応する文字列リテラルを得る
 *
 * @param hr HRESULT値。
 * @return 対応する文字列リテラル。既知でなければ空を返す。
 */
inline std::string HResultName(HRESULT hr)
{
	switch (hr) {
	case S_OK: return "S_OK";
	case S_FALSE: return "S_FALSE";
	case E_FAIL: return "E_FAIL";
	case E_INVALIDARG: return "E_INVALIDARG";
	case E_ACCESSDENIED: return "E_ACCESSDENIED";
	case E_POINTER: return "E_POINTER";
	case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
	case E_NOTIMPL: return "E_NOTIMPL";
	case E_NOINTERFACE: return "E_NOINTERFACE";
	case E_UNEXPECTED: return "E_UNEXPECTED";

	default:
		return "";
	}
}

/*!
 * @brief HRESULT値に対応するエラーメッセージを得る
 *
 * @param hr HRESULT値。
 * @return 対応するエラーメッセージ。取得できない場合は空を返す。
 */
inline std::string FormatHResultMessage(HRESULT hr)
{
	// HRESULT値から_com_errorを生成する。
	_com_error err{ hr };

	// エラーメッセージを取得する。
	const auto msg = err.ErrorMessage();

	return msg ? cxx::to_string(msg, CP_UTF8) : "";
}

/*!
 * @brief HRESULT値を人間が読める形式で説明する文字列を得る
 *
 * @param hr HRESULT値。
 * @return 説明文字列。
 */
inline std::string DescribeHResult(HRESULT hr)
{
	const auto code = std::format("(0x{:X})", static_cast<uint32_t>(hr));
	const auto name = HResultName(hr);
	const auto msg  = FormatHResultMessage(hr);

	if (name.empty() && msg.empty()) {
		return code;
	}

	if (msg.empty()) {
		return std::format(R"({} "{}")", code, msg);
	}

	return std::format(R"({}{} "{}")", name, code, msg);
}

/*!
 * @brief HRESULTの値が期待される値と等しいことを確認するためのアサーション関数
 *
 * @param actual 評価されたHRESULT値。
 * @param expected 期待値。
 * @param actual_expr 評価された式。
 * @param expected_expr 期待値を表す式。
 */
inline ::testing::AssertionResult HResultEq(
	HRESULT actual,
	HRESULT expected,
	const char* actual_expr,
	const char* expected_expr [[maybe_unused]] // HRESULT値はマクロ定義するので、使い物にならない。
)
{
	if (actual == expected) {
		return ::testing::AssertionSuccess();
	}

	return ::testing::AssertionFailure()
		<< actual_expr << " returned " << std::data(DescribeHResult(actual))
		<< ", expected " << std::data(DescribeHResult(expected));
}

} // namespace testing

//! @brief HRESULTの値が期待される値と等しいことを確認するためのマクロ
#define EXPECT_HRESULT_EQ(actual, expected)						\
	EXPECT_PRED_FORMAT2(										\
		[](const char* a, const char* e, auto av, auto ev) {	\
			return testing::HResultEq(av, ev, a, e);			\
		},														\
		actual, expected)
