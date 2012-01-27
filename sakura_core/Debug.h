/*!	@file
	@brief デバッグ用関数

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <windows.h>
#include "global.h"

SAKURA_CORE_API void AssertError( LPCTSTR pszFile, long nLine, BOOL bIsError );
SAKURA_CORE_API void DebugOut( LPCTSTR lpFmt, ...);
//void DebugOutDialog( LPCTSTR lpFmt, ...);
SAKURA_CORE_API int DebugOutDialog( HWND, UINT, LPCTSTR, LPCTSTR, ... );

//void MYASSERT( LPCTSTR, long, BOOL );

/*
	MYTRACEはリリースモードではコンパイルエラーとなるようにしてあるので，
	MYTRACEを使う場合には必ず#ifdef _DEBUG ～ #endif で囲む必要がある．
	
*/
#ifdef _DEBUG
	#define MYTRACE DebugOut
#endif
#ifndef _DEBUG
	#define MYTRACE Do_not_use_the_MYTRACE_function_if_release_mode
#endif

//#ifdef _DEBUG～#endifで囲まなくても良い版
#ifdef _DEBUG
#define DBPRINT DebugOut
#else
#if (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__ )
#define DBPRINT(...)
#else
// Not support C99 variable macro
inline void DBPRINT( ... ){};
#endif
#endif // _DEBUG

//#ifdef _DEBUG
	#define MYMESSAGEBOX DebugOutDialog
//#endif
//#ifndef _DEBUG
//	#define MYMESSAGEBOX DebugOutDialog
//#endif

//#ifdef _DEBUG
	#define MYASSERT AssertError
//#endif
//#ifndef _DEBUG
//	#define MYASSERT raise_compile_error
//#endif


/*
#ifdef _DEBUG
	#define MYASSERT( _file, _line, b ) AssertError( (_file), (_line), (b) )
#endif
#ifndef _DEBUG
	#define MYASSERT( _file, _line, b ) AssertError( (_file), (_line), (b) )
#endif
*/

#ifdef _DEBUG
	//!	設定している場所はあるが，参照している場所がない変数
	SAKURA_CORE_API extern int gm_ProfileOutput;
#endif



///////////////////////////////////////////////////////////////////////
#endif /* _DEBUG_H_ */


/*[EOF]*/
