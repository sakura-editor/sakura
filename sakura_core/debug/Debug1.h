/*!	@file
	@brief デバッグ用関数

	@author Norio Nakatani

	@date 2013/03/03 Uchi MessageBox用関数を分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_DEBUG1_382EF8C2_DA86_410F_80D9_7F357A356C80_H_
#define SAKURA_DEBUG1_382EF8C2_DA86_410F_80D9_7F357A356C80_H_
#pragma once

#include "util/string_ex.h"

#include <vadefs.h>

#include <Windows.h>
#include <tchar.h>

#include <string_view>

namespace cxx {

/*!
 * @brief デバッガーに文字列を送信します。
 *
 * @param[in] outputString 出力する文字列
 */
void OutputDebugStringW(
	std::wstring_view outputString
);

} // namespace cxx

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#if defined(_DEBUG) || defined(USE_RELPRINT)

/*!
 * @brief 書式付きデバッガ出力
 *
 * 引数で与えられた情報をDebugStringとして出力する．
 *
 * @param[in] format printfの書式付き文字列
 * @param[in, opt] args 引数リスト
 *
 * @date 2001/06/23 Norio Nakatani, 大量にトレースするときのためにウェイトを入れた
 */
template <typename... Args>
void DebugOutW(
	_In_z_ _Printf_format_string_ LPCWSTR format,
	const Args&... args
)
{
	// 出力文字列を整形する
	const auto outputString = strprintf(format, std::as_const(args)...);

	// デバッガーに文字列を送信する
	cxx::OutputDebugStringW(outputString);

	//ウェイト
	::Sleep(1);
}

#endif	// _DEBUG || USE_RELPRINT

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 デバッグ用メッセージ出力                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACEはリリースモードではコンパイルエラーとなるようにしてあるので，
	MYTRACEを使う場合には必ず#ifdef _DEBUG ～ #endif で囲む必要がある．
*/
#ifdef _DEBUG
	#define MYTRACE DebugOutW
#else
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
#endif

//#ifdef _DEBUG～#endifで囲まなくても良い版
#ifdef _DEBUG
	#define DEBUG_TRACE DebugOutW
#else
	#define DEBUG_TRACE(...)
#endif

//RELEASE版でも出力する版 (RELEASEでのみ発生するバグを監視する目的)
#ifdef USE_RELPRINT
	#define RELPRINT DebugOutW
#else
	#define RELPRINT   Do_not_define_USE_RELPRINT
#endif	// USE_RELPRINT

//トレース出力（トレース箇所のファイルパスと行番号を出力してエラー解析を容易にする目的）
#ifdef _DEBUG
	#define TRACE( format, ... )	DEBUG_TRACE( _T("%hs(%d): ") _T(format) _T("\n"), __FILE__, __LINE__, __VA_ARGS__ )
#else
	#define TRACE( ... )
#endif

#endif /* SAKURA_DEBUG1_382EF8C2_DA86_410F_80D9_7F357A356C80_H_ */
