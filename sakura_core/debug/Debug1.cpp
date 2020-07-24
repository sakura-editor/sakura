/*!	@file
	@brief デバッグ用関数

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani DebugOut()に微妙～な修正
	@date 2002/01/17 aroka 型の修正
	@date 2013/03/03 Uchi MessageBox用関数を分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "debug/Debug1.h"

#include <stdio.h>

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! @brief 書式付きデバッガ出力

	@param[in] pszFormat printfの書式付き文字列
	@param[in] argList 書式化するパラメータ

	引数で与えられた情報をDebugStringとして出力する．
*/
void VDebugOut( LPCWSTR pszFormat, va_list argList )
{
	// 出力される文字数をカウントしてメモリ確保
	const int cchBufLen = ::_vscwprintf( pszFormat, argList );
	auto memBuf = std::make_unique<WCHAR[]>( cchBufLen + 1 );
	auto *pszBuf = memBuf.get();

	// メッセージを整形する
	::_vsnwprintf_s( pszBuf, cchBufLen + 1, _TRUNCATE, pszFormat, argList );

	// デバッガに出力する
	::OutputDebugString( pszBuf );
}

/*! @brief 書式付きデバッガ出力

	@param[in] lpFmt printfの書式付き文字列

	引数で与えられた情報をDebugStringとして出力する．
*/
void DebugOut( LPCWSTR pszFormat, ...)
{
	va_list argList;
	va_start( argList, pszFormat );
	VDebugOut( pszFormat, argList );
	va_end( argList );
}

//! コンストラクタ
SakuraTrace::SakuraTrace(const char* pszFile, const int nLine)
	: m_pszFile( pszFile )
	, m_nLine( nLine )
{
}

/*!
 * 演算子オーバーロード()の実装
 *
 * オブジェクトインスタンスに対し、関数のように引数を渡したときに呼ばれる演算子。
 */
void SakuraTrace::operator()( LPCWSTR lpFormat, ... ) const
{
	CNativeW cMemFormat;
	cMemFormat.AppendStringF( L"%hs(%d): %s", m_pszFile, m_nLine, lpFormat );
	const auto *pszFormat = cMemFormat.GetStringPtr();

	va_list argList;
	va_start( argList, pszFormat );
	VDebugOut( pszFormat, argList );
	va_end( argList );
}
