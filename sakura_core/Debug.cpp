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
#include "global.h"

#ifdef _DEBUG
	int gm_ProfileOutput = 0;
#endif

//デバッグウォッチ用の型
struct TestArrayA{ char    a[100]; };
struct TestArrayW{ wchar_t a[100]; };
struct TestArrayI{ int     a[100]; };

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include "Debug3.h"

/*! @brief 書式付きデバッガ出力

	@param[in] lpFmt printfの書式付き文字列

	引数で与えられた情報をDebugStringとして出力する．
*/
#ifdef _UNICODE
void DebugOutW( LPCWSTR lpFmt, ...)
{
	//整形
	static WCHAR szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	auto_vsprintf( szText, lpFmt, argList );

	//出力
	::OutputDebugStringW( szText );
#ifdef USE_DEBUGMON
	DebugMonitor_Output(NULL, to_wchar(szText));
#endif

	//ウェイト
	::Sleep(1);	// Norio Nakatani, 2001/06/23 大量にトレースするときのために

	va_end(argList);
	return;
}
#endif

void DebugOutA( LPCSTR lpFmt, ...)
{
	//整形
	static CHAR szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	::tchar_vsprintf( szText, lpFmt, argList );

	//出力
	::OutputDebugStringA( szText );
#ifdef USE_DEBUGMON
	DebugMonitor_Output(NULL, to_wchar(szText));
#endif

	//ウェイト
	::Sleep(1);	// Norio Nakatani, 2001/06/23 大量にトレースするときのために

	va_end(argList);
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "global.h"
#include "CEditWnd.h"
HWND GetMessageBoxOwner(HWND hwndOwner)
{
	if(hwndOwner==NULL && g_pcEditWnd){
		return g_pcEditWnd->GetHwnd();
	}
	else{
		return hwndOwner;
	}
}

/*!
	書式付きメッセージボックス

	引数で与えられた情報をダイアログボックスで表示する．
	デバッグ目的以外でも使用できる．
*/
SAKURA_CORE_API int VMessageBoxF_W(
	HWND		hwndOwner,	//!< [in] オーナーウィンドウのハンドル
	UINT		uType,		//!< [in] メッセージボックスのスタイル (MessageBoxと同じ形式)
	LPCWSTR		lpCaption,	//!< [in] メッセージボックスのタイトル
	LPCWSTR		lpText,		//!< [in] 表示するテキスト。printf仕様の書式指定が可能。
	va_list&	v			//!< [in/out] 引数リスト
)
{
	hwndOwner=GetMessageBoxOwner(hwndOwner);
	//整形
	static WCHAR szBuf[16000];
	auto_vsprintf(szBuf,lpText,v);
	//API呼び出し
#ifdef _UNICODE
	return ::MessageBoxW( hwndOwner, szBuf, lpCaption, uType);
#else
	return ::MessageBoxA( hwndOwner, to_achar(szBuf), to_achar(lpCaption), uType);
#endif
}

SAKURA_CORE_API int VMessageBoxF_A(
	HWND		hwndOwner,	//!< [in] オーナーウィンドウのハンドル
	UINT		uType,		//!< [in] メッセージボックスのスタイル (MessageBoxと同じ形式)
	LPCSTR		lpCaption,	//!< [in] メッセージボックスのタイトル
	LPCSTR		lpText,		//!< [in] 表示するテキスト。printf仕様の書式指定が可能。
	va_list&	v			//!< [in/out] 引数リスト
)
{
	hwndOwner=GetMessageBoxOwner(hwndOwner);
	//整形
	static ACHAR szBuf[16000];
	tchar_vsprintf(szBuf,lpText,v);
	//API呼び出し
	return ::MessageBoxA( hwndOwner, szBuf, lpCaption, uType);
}

SAKURA_CORE_API int MessageBoxF_W( HWND hwndOwner, UINT uType, LPCWSTR lpCaption, LPCWSTR lpText, ... )
{
	va_list v;
	va_start(v,lpText);
	int nRet = VMessageBoxF_W(hwndOwner, uType, lpCaption, lpText, v);
	va_end(v);
	return nRet;
}
SAKURA_CORE_API int MessageBoxF_A( HWND hwndOwner, UINT uType, LPCSTR lpCaption, LPCSTR lpText, ... )
{
	va_list v;
	va_start(v,lpText);
	int nRet = VMessageBoxF_A(hwndOwner, uType, lpCaption, lpText, v);
	va_end(v);
	return nRet;
}






void AssertError( LPCTSTR pszFile, long nLine, BOOL bIsError )
{
	if( !bIsError ){
		TCHAR psz[1000];
		::auto_sprintf(psz, _T("%ts\n行 %d でASSERT正当性チェックエラー"), pszFile, nLine );
		::MessageBox( NULL, psz, _T("MYASSERT"), MB_OK );
	}
	return;
}


//! メッセージボックスを表示。キャプションにはexe名。
void DBMSG_IMP(const ACHAR* msg)
{
	//EXE名を取得。(初回呼び出し時のみ計算)
	static const char* exe=NULL;
	if(exe==NULL){
		static char exepath[_MAX_PATH];
		GetModuleFileNameA(NULL,exepath,_countof(exepath));
		const char* p=strrchr(exepath,'\\');
		if(p)exe=p+1;
		else exe=exepath;
	}

	//メッセージ
	MessageBoxA(NULL,msg,exe,MB_OK);
}


/*[EOF]*/
