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

#ifndef SAKURA_DEBUG1_587B8A50_4B0A_4E5E_A638_40FB1EC301CA_H_
#define SAKURA_DEBUG1_587B8A50_4B0A_4E5E_A638_40FB1EC301CA_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#if defined(_DEBUG) || defined(USE_RELPRINT)
void DebugOutW( LPCWSTR lpFmt, ...);
void DebugOutA( LPCSTR lpFmt, ...);
#endif	// _DEBUG || USE_RELPRINT

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 デバッグ用メッセージ出力                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACEはリリースモードではコンパイルエラーとなるようにしてあるので，
	MYTRACEを使う場合には必ず#ifdef _DEBUG ～ #endif で囲む必要がある．
*/
#ifdef _DEBUG
	#ifdef _UNICODE
	#define MYTRACE DebugOutW
	#else
	#define MYTRACE DebugOutA
	#endif
#else
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
#endif

//#ifdef _DEBUG～#endifで囲まなくても良い版
#ifdef _DEBUG
	#ifdef _UNICODE
	#define DEBUG_TRACE DebugOutW
	#else
	#define DEBUG_TRACE DebugOutA
	#endif
#elif (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__ )
	#define DEBUG_TRACE(...)
#else
	// Not support C99 variable macro
	inline void DEBUG_TRACE( ... ){}
#endif

//RELEASE版でも出力する版 (RELEASEでのみ発生するバグを監視する目的)
#ifdef USE_RELPRINT
	#ifdef _UNICODE
	#define RELPRINT DebugOutW
	#else
	#define RELPRINT DebugOutA
	#endif
#else
	#define RELPRINT   Do_not_define_USE_RELPRINT
#endif	// USE_RELPRINT


///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_DEBUG1_587B8A50_4B0A_4E5E_A638_40FB1EC301CA_H_ */



