//	$Id$
///////////////////////////////////////////////////////////////////////
//
//  debug.h
//
//	Copyright (C) 1998-2000, Norio Nakatani
//

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <windows.h>
#include "global.h"

SAKURA_CORE_API void AssertError( LPCTSTR pszFile, long nLine, BOOL bIsError );
SAKURA_CORE_API void DebugOut( LPCTSTR lpFmt, ...);
//void DebugOutDialog( LPCTSTR lpFmt, ...);
SAKURA_CORE_API int DebugOutDialog( HWND, UINT, LPCTSTR, LPCTSTR, ... );

//void MYASSERT( LPCTSTR, long, BOOL );


#ifdef _DEBUG
	#define MYTRACE DebugOut
#endif
#ifndef _DEBUG
	#define MYTRACE Do_not_use_the_MYTRACE_function_if_release_mode
#endif

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
	#define MYASSERT( _file, _line, b) AssertError( (_file), (_line), (b))
#endif
#ifndef _DEBUG
	#define MYASSERT( _file, _line, b) AssertError( (_file), (_line), (b))
#endif
*/

#ifdef _DEBUG
	SAKURA_CORE_API extern int gm_ProfileOutput;
#endif



///////////////////////////////////////////////////////////////////////
#endif /* _DEBUG_H_ */

/*[EOF]*/
