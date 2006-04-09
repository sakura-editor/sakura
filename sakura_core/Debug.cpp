/*!	@file
	@brief デバッグ用関数

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani DebugOut()に微妙～な修正
	@date 2002/01/17 aroka 型の修正
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "debug.h"
#include <tchar.h>

#ifdef _DEBUG
	int gm_ProfileOutput = 0;
#endif


//#ifdef _DEBUG
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif


//! 書式付きデバッガ出力
void DebugOut( LPCTSTR lpFmt, ...)
{
	static TCHAR szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	::wvsprintf( szText, lpFmt, argList );
	::OutputDebugString( szText );

	::Sleep(1);	// Norio Nakatani, 2001/06/23 大量にトレースするときのために

	va_end(argList);
	return;
}



//! 書式付きメッセージボックス
int DebugOutDialog(
	HWND	hWndParent,
	UINT	nStyle,
	LPCTSTR	pszTitle,
	LPCTSTR lpFmt,
	...
)
{
	static TCHAR szText[16000];
	va_list	argList;
	int		nRet;
	va_start(argList, lpFmt);
	::wvsprintf( szText, lpFmt, argList );
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
		TCHAR psz[1000];
		::wsprintf(psz, _T("%s\n行 %d でASSERT正当性チェックエラー"), pszFile, nLine );
		::MessageBox( NULL, psz, _T("MYASSERT"), MB_OK );
	}
	return;
}


/*[EOF]*/
