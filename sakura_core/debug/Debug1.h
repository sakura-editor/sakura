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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#if defined(_DEBUG) || defined(USE_RELPRINT)
void DebugOutW(LPCWSTR lpFmt, ...);
#endif // _DEBUG || USE_RELPRINT

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 デバッグ用メッセージ出力                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACEはリリースモードではコンパイルエラーとなるようにしてあるので，
	MYTRACEを使う場合には必ず#ifdef _DEBUG ～ #endif で囲む必要がある．
*/
#ifdef _DEBUG
#define MYTRACE(...)
#else
#define MYTRACE(...)
#endif

//#ifdef _DEBUG～#endifで囲まなくても良い版
#ifdef _DEBUG
#define DEBUG_TRACE(...)
#else
#define DEBUG_TRACE(...)
#endif

//RELEASE版でも出力する版 (RELEASEでのみ発生するバグを監視する目的)
#ifdef USE_RELPRINT
#define RELPRINT(...)
#else
#define RELPRINT(...)
#endif // USE_RELPRINT
