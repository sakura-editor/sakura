//	$Id$
/*!	@file
	共通関数群

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

#ifndef _ETC_UTY_H_
#define _ETC_UTY_H_

#include <windows.h>
#include "global.h"
#include "CMemory.h"
#include <shlobj.h>
#include "CEol.h"
#include "CBregexp.h"	//	Jun. 26, 2001 genta
#include "CHtmlHelp.h"	//	Jul.  6, 2001 genta

SAKURA_CORE_API const char* MyGetDateFormat( char*, int, int, const char* );/* 日付をフォーマット */
SAKURA_CORE_API const char* MyGetTimeFormat( char*, int, int, const char* );/* 時刻をフォーマット */
SAKURA_CORE_API void CutLastYenFromDirectoryPath( char* );/* フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない*/
SAKURA_CORE_API void AddLastYenFromDirectoryPath( char* );/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
SAKURA_CORE_API int LimitStringLengthB( const char*, int, int, CMemory& );/* データを指定バイト数以内に切り詰める */
SAKURA_CORE_API const char* GetNextLimitedLengthText( const char*, int, int, int*, int* );/* 指定長以下のテキストに切り分ける */
//SAKURA_CORE_API const char* GetNextLine( const char*, int, int*, int*, BOOL*, BOOL );/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
SAKURA_CORE_API const char* GetNextLine( const char*, int, int*, int*, CEOL* );/* CR0LF0,CRLF,LFCR,LF,CRで区切られる「行」を返す。改行コードは行長に加えない */
SAKURA_CORE_API void GetLineColm( const char*, int*, int* );
SAKURA_CORE_API BOOL IsFilePath( const char*, int*, int* );
SAKURA_CORE_API BOOL IsURL( const char*, int, int* );/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
SAKURA_CORE_API BOOL IsMailAddress( const char*, int, int* );	/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
//#ifdef COMPILE_COLOR_DIGIT
SAKURA_CORE_API int IsNumber( const char*, int, int );/* 数値ならその長さを返す */	//@@@ 2001.02.17 by MIK
//#endif
SAKURA_CORE_API void ActivateFrameWindow( HWND );	/* アクティブにする */
SAKURA_CORE_API BOOL GetSystemResources( int*, int*, int* );	/* システムリソースを調べる */
SAKURA_CORE_API BOOL CheckSystemResources( const char* );	/* システムリソースのチェック */
SAKURA_CORE_API BOOL CheckWindowsVersion( const char* pszAppName );	/* Windowsバージョンのチェック */
// Jul. 5, 2001 shoji masami
SAKURA_CORE_API bool CheckWindowsVersionNT( void );	/* NTプラットフォームかどうか */
SAKURA_CORE_API void GetAppVersionInfo( HINSTANCE, int, DWORD*, DWORD* );	/* リソースから製品バージョンの取得 */
SAKURA_CORE_API void SplitPath_FolderAndFile( const char*, char*, char* );	/* ファイルのフルパスを、フォルダとファイル名に分割 */
SAKURA_CORE_API BOOL GetAbsolutePath( const char*, char*, BOOL );	/* 相対パス→絶対パス */
SAKURA_CORE_API BOOL GetLongFileName( const char*, char* );	/* ロングファイル名を取得する */
SAKURA_CORE_API char* GetHelpFilePath( char* );	/* ヘルプファイルのフルパスを返す */
SAKURA_CORE_API BOOL CheckEXT( const char*, const char* );	/* 拡張子を調べる */
SAKURA_CORE_API char* my_strtok( char*, int, int*, char* );
/* Shell Interface系(?) */
SAKURA_CORE_API BOOL SelectDir(HWND, const char*, const char*, char* );	/* フォルダ選択ダイアログ */
SAKURA_CORE_API ITEMIDLIST* CreateItemIDList( const char* );	/* パス名に対するアイテムＩＤリストを取得する */
SAKURA_CORE_API BOOL DeleteItemIDList( ITEMIDLIST* );/* アイテムＩＤリストを削除する */
SAKURA_CORE_API BOOL ResolveShortcutLink(HWND hwnd, LPCSTR lpszLinkFile, LPSTR lpszPath);/* ショートカット(.lnk)の解決 */

/*
||	処理中のユーザー操作を可能にする
||	ブロッキングフック(?)(メッセージ配送)
*/
SAKURA_CORE_API BOOL BlockingHook( HWND hwndDlgCancel );

/*機能番号に対応したヘルプトピックIDを返す*/
SAKURA_CORE_API int FuncID_To_HelpContextID( int nFuncID );	//Stonee, 2001/02/23

//	Jun. 26, 2001 genta
//!	正規表現ライブラリのバージョン取得
SAKURA_CORE_API bool CheckRegexpVersion( HWND hWnd, int nCmpId, bool bShowMsg = false );
SAKURA_CORE_API bool CheckRegexpSyntax( const char* szPattern, HWND hWnd, bool bShowMessage );
SAKURA_CORE_API bool InitRegexp( HWND hWnd, CBregexp& rRegexp, bool bShowMessage );

SAKURA_CORE_API HWND OpenHtmlHelp( HWND hWnd, LPCSTR szFile, UINT uCmd, DWORD data,bool msgflag = true);

///////////////////////////////////////////////////////////////////////
#endif /* _ETC_UTY_H_ */


/*[EOF]*/
