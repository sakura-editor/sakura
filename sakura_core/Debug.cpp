//	$Id$
///////////////////////////////////////////////////////////////////////
//
//  debug.cpp
//	Copyright (C) 1998-2000, Norio Nakatani
//
//

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
