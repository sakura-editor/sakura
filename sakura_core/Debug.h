//	$Id$
/*!	@file
	デバッグ用関数

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
