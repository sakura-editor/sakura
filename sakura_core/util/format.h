/*! @file */
// 2007.10.20 kobake 書式関連
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_FORMAT_A006AC9B_ADE2_499D_9CC6_00A649F32B4F_H_
#define SAKURA_FORMAT_A006AC9B_ADE2_499D_9CC6_00A649F32B4F_H_
#pragma once

#include "basis/primitive.h"

#include "util/format/PrintfArg.hpp"

#include <span>
#include <string>
#include <string_view>

#include <stdio.h>

namespace cxx {

/*!
 * @brief 文字列に書式付きデータを書き込みます。
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <typename... Params>
int _snprintf_s(
	WCHAR* pBuffer,
	size_t nBufferSIze,
	size_t nMaxCount,
	_In_z_ _Printf_format_string_ const WCHAR* format,
	const Params&... params
)
{
	// 変換した引数リストをCランタイムの実装に転送する
	return ::_snwprintf_s(
		pBuffer,
		nBufferSIze,
		nMaxCount,
		format,
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1(std::as_const(params)))...
	);
}

// Ascii版
template <typename... Params>
int _snprintf_s(
	ACHAR* pBuffer,
	size_t nBufferSIze,
	size_t nMaxCount,
	_In_z_ _Printf_format_string_ const ACHAR* format,
	const Params&... params
)
{
	// 変換した引数リストをCランタイムの実装に転送する
	return ::_snprintf_s(
		pBuffer,
		nBufferSIze,
		nMaxCount,
		format,
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1(std::as_const(params)))...
	);
}

/*!
 * @brief 文字列に書式付きデータを書き込みます。
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <typename CharT, basis::WritableBuffer<CharT> A1, typename... Params>
int _snprintf_s(
	A1& dst,
	size_t count,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// 出力先を固定長バッファとして扱う
	auto buffer = std::span{ dst };

	// パラメータ展開で転送する
	return cxx::_snprintf_s(
		std::data(buffer),
		std::size(buffer),
		count,
		format,
		std::as_const(params)...
	);
}

/*!
 * @brief 書式付きデータの文字数をカウントします。
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれる文字数
 * @retval < 0 エラー発生
 */
template <typename... Params>
int _scprintf(
	_In_z_ _Printf_format_string_ LPCWSTR format,
	const Params&... params
)
{
	// 変換した引数リストをCランタイムの実装に転送する
	return ::_scwprintf(
		format,
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1(std::as_const(params)))...
	);
}

// Ascii版
template <typename... Params>
int _scprintf(
	_In_z_ _Printf_format_string_ LPCSTR format,
	const Params&... params
)
{
	// 変換した引数リストをCランタイムの実装に転送する
	return ::_scprintf(
		format,
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1(std::as_const(params)))...
	);
}

/*!
 * @brief 文字列に書式付きデータを書き込みます。
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <typename CharT, basis::WritableBuffer<CharT> A1, typename... Params>
int _sprintf_s(
	A1& dst,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメータ展開で転送する
	return cxx::_snprintf_s(
		dst,
		_TRUNCATE,
		format,
		std::as_const(params)...
	);
}

template <typename CharT, typename... Params>
int _sprintf_s(
	CharT* pBuffer,
	size_t nBufferSize,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// 出力先を固定長バッファとして扱う
	auto buffer = std::span{ pBuffer, nBufferSize };

	// パラメータ展開で転送する
	return cxx::_sprintf_s(
		buffer,
		format,
		std::as_const(params)...
	);
}

} // namespace cxx

// 20051121 aroka
std::wstring GetDateTimeFormat( std::wstring_view format, const SYSTEMTIME& systime );
UINT32 ParseVersion( const WCHAR* ver );	//バージョン番号の解析
int CompareVersion( const WCHAR* verA, const WCHAR* verB );	//バージョン番号の比較

/*!
 * @brief 固定長バッファに書式付きデータを書き込みます。
 *
 * バッファサイズを考慮しないバージョン。
 *
 * 使用しないでください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
// TODO: いつか削除する
template <typename... Args>
int auto_sprintf(
	WCHAR* buf,
	_In_z_ _Printf_format_string_ const WCHAR* format,
	const Args&... args
)
{
	using namespace cxx;

	// C標準のswprintfはバッファサイズを要求するので、準拠しないバージョンを呼ぶ
	return ::_swprintf(
		buf,
		format,
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1(std::as_const(args)))...
	);
}

#ifdef ENABLE_UNUSED_LEGACY_CODES

template <typename... Args>
int auto_sprintf(
	ACHAR* buf,
	_In_z_ _Printf_format_string_ const ACHAR* format,
	const Args&... args
)
{
	using namespace cxx;

	return std::sprintf(
		buf,
		format,
		PrintfArg::ConvertArgPhase2(PrintfArg::ConvertArgPhase1(std::as_const(args)))...
	);
}

#endif // ifdef ENABLE_UNUSED_LEGACY_CODES

template <typename CharT, basis::WritableBuffer<CharT> A, typename... Params>
int auto_snprintf_s(
	A& dst,
	size_t count,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメータ展開で転送する
	return cxx::_snprintf_s(
		dst,
		count,
		format,
		std::as_const(params)...
	);
}

template <typename CharT, typename... Params>
int auto_snprintf_s(
	CharT* pBuffer,
	size_t nBufferSize,
	size_t count,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメータ展開で転送する
	return cxx::_snprintf_s(
		pBuffer,
		nBufferSize,
		count,
		format,
		std::as_const(params)...
	);
}

template <typename CharT, basis::WritableBuffer<CharT> A, typename... Params>
int auto_sprintf_s(
	A& dst,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	return cxx::_sprintf_s(
		dst,
		format,
		std::as_const(params)...
	);
}

template <typename CharT, typename... Params>
int auto_sprintf_s(
	CharT* pBuffer,
	size_t nBufferSize,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメータ展開で転送する
	return cxx::_sprintf_s(
		pBuffer,
		nBufferSize,
		format,
		std::as_const(params)...
	);
}

template <typename CharT, basis::WritableBuffer<CharT> A, typename... Params>
int auto_sprintf(
	A& dst,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメータ展開で転送する
	return cxx::_sprintf_s(
		dst,
		format,
		std::as_const(params)...
	);
}

/*!
 * @brief 固定長バッファに書式付きデータを書き込みます。
 *
 * バッファサイズを考慮しない実装を救済するためのもの。
 *
 * 必ずしも移行不要。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <basis::WritableBuffer<WCHAR> A, typename... Args>
int auto_sprintf(
	A& buf,
	_In_z_ _Printf_format_string_ const WCHAR* format,
	const Args&... args
)
{
	// パラメータ展開で転送する
	return cxx::_sprintf_s(
		buf,
		format,
		std::as_const(args)...
	);
}

/*!
 * @brief 文字列に書式付きデータを書き込みます。
 *
 * 生配列をSFilePathに置換できるよう用意したもの。
 *
 * セキュリティ上問題があるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <basis::WritableBuffer<WCHAR> A, typename... Params>
int swprintf_s(
	A& dst,
	_In_z_ _Printf_format_string_ const WCHAR* format,
	const Params&... params
)
{
	return auto_sprintf_s(
		dst,
		format,
		std::as_const(params)...
	);
}

/*!
 * @brief 固定長バッファに書式付きデータを書き込みます。
 *
 * バッファサイズを考慮しない実装を救済するためのもの。
 *
 * 必ずしも移行不要だが「使用禁止」だと思ってください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <basis::WritableBuffer<WCHAR> A, typename... Args>
int _swprintf(
	A& buf,
	_In_z_ _Printf_format_string_ const WCHAR* format,
	const Args&... args
)
{
	// パラメータ展開で転送する
	return cxx::_sprintf_s(
		buf,
		format,
		std::as_const(args)...
	);
}

/*!
 * @brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
 * 	事前に確保したバッファに結果を書き込む高速バージョン。
 *  メモリ確保に時間がかかるほど巨大データは想定しておらず有意な差が出ることを保証しない。
 *
 * @param[in, out] out フォーマットされたテキストを受け取る変数
 * @param[in] format フォーマット文字列
 * @param[in, opt] params 引数リスト
 * @returns 出力された文字数。NUL終端を含まない。
 * @retval >= 0 正常終了
 * @retval < 0  異常終了
 */
template <typename CharT, typename... Params>
int strprintf(
	std::basic_string<CharT>& out,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
#if 0 // パラメーターなしで呼び出すケースが多いのでチェックを無効化する

	// パラメーターがない場合、コンパイルエラーにする
	static_assert(
		0 < sizeof...(params),
		"One or more paramaters should be passed"
	);

#endif

	using namespace cxx;

	// 引数を変換する（その１）
	const auto phase1ConvertedArgs = std::make_tuple(
		PrintfArg::ConvertArgPhase1(std::as_const(params))...
	);

	// 整形によって出力される文字数をカウント
	const int count = std::apply(
		[format] (const auto&... args) {
			return cxx::_scprintf(
				format,
				std::as_const(args)...
			);
		},
		phase1ConvertedArgs
	);

	// 出力文字数が0未満ならエラー、戻り値は空。
	if (count <= 0) return count;

	// 出力先バッファを確保する
	out.resize(count);

	// 整形を実行する
	return std::apply(
		[&out, format] (const auto&... args) {
			return cxx::_sprintf_s(
				out.data(),
				out.size() + 1,
				format,
				std::as_const(args)...
			);
		},
		phase1ConvertedArgs
	);
}

/*!
 * @brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
 * 	動的にバッファを確保する簡易バージョン
 *
 * @param[in] format フォーマット文字列
 * @param[in, opt] params 引数リスト
 * @returns フォーマットされた文字列
 */
template <typename CharT, typename... Params>
auto strprintf(
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
) -> std::basic_string<CharT>
{
	// 出力先バッファを用意する
	std::basic_string<CharT> out;

	// 整形を実行する
	const auto formatted = strprintf(out, format, std::as_const(params)...);

	if (formatted <= 0) return {};

	// NUL終端する
	out.resize(formatted);

	return out;
}

#endif /* SAKURA_FORMAT_A006AC9B_ADE2_499D_9CC6_00A649F32B4F_H_ */
