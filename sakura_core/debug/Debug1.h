/*!	@file
	@brief デバッグ用関数

	@author Norio Nakatani

	@date 2013/03/03 Uchi MessageBox用関数を分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#pragma once

#include <stdarg.h>

#include <Windows.h>

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void VDebugOut( LPCWSTR pszFormat, va_list argList );
void DebugOut( LPCWSTR pszFormat, ... );

/*!
 * ATLTRACE互換のTRACEを実現するためのファンクタ。
 */
struct SakuraTrace final
{
	const char*	m_pszFile;
	const int	m_nLine;

	//! コンストラクタ
	SakuraTrace( const char* pszFile, const int nLine );

	//! 演算子オーバーロード
	void operator()( LPCWSTR pszFormat, ... ) const;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 デバッグ用メッセージ出力                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// ATLTRACE互換版
#ifdef _DEBUG
	#define SAKURA_TRACE			SakuraTrace( __FILE__, __LINE__ )
	#define TRACE( format, ... )	SAKURA_TRACE( (format), ##__VA_ARGS__ )
#else
	#define TRACE( ... )
#endif

/*
	MYTRACEはリリースモードではコンパイルエラーとなるようにしてあるので，
	MYTRACEを使う場合には必ず#ifdef _DEBUG ～ #endif で囲む必要がある．
*/
#ifdef _DEBUG
	#define MYTRACE		DebugOut
#else
	#define MYTRACE		Do_not_use_the_MYTRACE_function_if_release_mode
#endif

//#ifdef _DEBUG～#endifで囲まなくても良い版
#ifdef _DEBUG
	#define DEBUG_TRACE DebugOut
#else
	#define DEBUG_TRACE(...)
#endif

//RELEASE版でも出力する版 (RELEASEでのみ発生するバグを監視する目的)
#ifdef USE_RELPRINT
	#define RELPRINT	DebugOut
#else
	#define RELPRINT	Do_not_define_USE_RELPRINT
#endif	// USE_RELPRINT
