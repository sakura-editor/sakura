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

#include "debug.h"

#ifdef _DEBUG
	int gm_ProfileOutput = 0;
#endif


//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif



void DebugOut( LPCTSTR lpFmt, ...)
{
//	CShareData cShareData;
//	cShareData.Init();

	static char szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
//	cShareData.TraceOut( lpFmt, argList );
	wvsprintf( szText, lpFmt, argList );
	OutputDebugString( szText );
	va_end(argList);
	return;
}





int DebugOutDialog(
	HWND	hWndParent,
	UINT	nStyle,
	LPCTSTR	pszTitle,
	LPCTSTR lpFmt,
	...
)
{
	static char szText[16000];
	va_list	argList;
	int		nRet;
	va_start(argList, lpFmt);
	wvsprintf( szText, lpFmt, argList );
	nRet = ::MessageBox( hWndParent,  szText, pszTitle, nStyle );
	va_end(argList);
	return nRet;
}


//void MYASSERT( LPCTSTR pszFile, long nLine, BOOL bIsError )
//{
//	AssertError( bIsError, pszFile, nLine );
//	return;
//}


void AssertError( LPCTSTR pszFile, long nLine, BOOL bIsError )
{
	if( !bIsError ){
		char psz[1000];
		wsprintf(psz, "%s\n行 %d でASSERT正当性チェックエラー", pszFile, nLine );
		MessageBox( NULL, psz, "MYASSERT", MB_OK );
	}
	return;
}


/*[EOF]*/
