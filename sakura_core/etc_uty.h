//	$Id$
/*!	@file
	@brief 共通関数群

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001-2002, genta
	Copyright (C) 2001, shoji masami, Stonee, MIK
	Copyright (C) 2002, aroka, hor, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _ETC_UTY_H_
#define _ETC_UTY_H_

#include <windows.h>
#include "global.h"
//#include "CMemory.h"// 2002/2/3 aroka ヘッダ軽量化
#include <shlobj.h>
//#include "CEol.h"// 2002/2/3 aroka ヘッダ軽量化
//#include "CBregexp.h"	//	Jun. 26, 2001 genta  2002/2/3 aroka ヘッダ軽量化
#include "CHtmlHelp.h"	//	Jul.  6, 2001 genta
class CMemory;// 2002/2/3 aroka ヘッダ軽量化
class CEOL;// 2002/2/3 aroka ヘッダ軽量化
class CBregexp;// 2002/2/3 aroka ヘッダ軽量化

//@@@ 2002.2.9 YAZAKI CShareDataに移動
//SAKURA_CORE_API const char* MyGetDateFormat( SYSTEMTIME& systime, char* pszDest, int nDestLen, int nDateFormatType, const char* pszDateFormat );/* 日付をフォーマット */
//SAKURA_CORE_API const char* MyGetTimeFormat( SYSTEMTIME &systime, char* pszDest, int nDestLen, int nTimeFormatType, const char* pszTimeFormat );/* 時刻をフォーマット */
SAKURA_CORE_API void CutLastYenFromDirectoryPath( char* );/* フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない*/
SAKURA_CORE_API void AddLastYenFromDirectoryPath( char* );/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
SAKURA_CORE_API int LimitStringLengthB( const char*, int, int, CMemory& );/* データを指定バイト数以内に切り詰める */
SAKURA_CORE_API const char* GetNextLimitedLengthText( const char*, int, int, int*, int* );/* 指定長以下のテキストに切り分ける */
//SAKURA_CORE_API const char* GetNextLine( const char*, int, int*, int*, BOOL*, BOOL );/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
SAKURA_CORE_API const char* GetNextLine( const char*, int, int*, int*, CEOL* );/* CR0LF0,CRLF,LFCR,LF,CRで区切られる「行」を返す。改行コードは行長に加えない */
SAKURA_CORE_API void GetLineColm( const char*, int*, int* );
SAKURA_CORE_API bool IsFilePath( const char*, int*, int*, bool = true );
SAKURA_CORE_API bool IsFileExists(const char* path, bool bFileOnly = false);
SAKURA_CORE_API BOOL IsURL( const char*, int, int* );/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
SAKURA_CORE_API BOOL IsMailAddress( const char*, int, int* );	/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
//#ifdef COMPILE_COLOR_DIGIT
SAKURA_CORE_API int IsNumber( const char*, int, int );/* 数値ならその長さを返す */	//@@@ 2001.02.17 by MIK
//#endif
SAKURA_CORE_API void ActivateFrameWindow( HWND );	/* アクティブにする */
SAKURA_CORE_API BOOL GetSystemResources( int*, int*, int* );	/* システムリソースを調べる */
SAKURA_CORE_API BOOL CheckSystemResources( const char* );	/* システムリソースのチェック */
//SAKURA_CORE_API BOOL CheckWindowsVersion( const char* pszAppName );	/* Windowsバージョンのチェック */
// Jul. 5, 2001 shoji masami
//SAKURA_CORE_API bool CheckWindowsVersionNT( void );	/* NTプラットフォームかどうか */
SAKURA_CORE_API void GetAppVersionInfo( HINSTANCE, int, DWORD*, DWORD* );	/* リソースから製品バージョンの取得 */
SAKURA_CORE_API void SplitPath_FolderAndFile( const char*, char*, char* );	/* ファイルのフルパスを、フォルダとファイル名に分割 */
SAKURA_CORE_API BOOL GetAbsolutePath( const char*, char*, BOOL );	/* 相対パス→絶対パス */
SAKURA_CORE_API BOOL GetLongFileName( const char*, char* );	/* ロングファイル名を取得する */
SAKURA_CORE_API char* GetHelpFilePath( char* , unsigned int nMaxLen = _MAX_PATH );	/* ヘルプファイルのフルパスを返す */// 20020119 aroka
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
SAKURA_CORE_API bool CheckRegexpSyntax( const char* szPattern, HWND hWnd, bool bShowMessage, int nOption = -1 );// 2002/2/1 hor追加
SAKURA_CORE_API bool InitRegexp( HWND hWnd, CBregexp& rRegexp, bool bShowMessage );

SAKURA_CORE_API HWND OpenHtmlHelp( HWND hWnd, LPCSTR szFile, UINT uCmd, DWORD data,bool msgflag = true);
SAKURA_CORE_API DWORD NetConnect ( const char strNetWorkPass[] );

SAKURA_CORE_API int cescape(const char* org, char* out, char cesc, char cwith);
SAKURA_CORE_API int cescape_j(const char* org, char* out, char cesc, char cwith);

/* ヘルプの目次を表示 */
SAKURA_CORE_API void ShowWinHelpContents( HWND hwnd, LPCTSTR lpszHelp );

/*!	&の二重化
	メニューに含まれる&を&&に置き換える
	@author genta
	@date 2002/01/30 cescapeに拡張し，
*/
inline void dupamp(const char* org, char* out)
{	cescape( org, out, '&', '&' ); }
///////////////////////////////////////////////////////////////////////

/* カラー名＜＞インデックス番号の変換 */	//@@@ 2002.04.30
SAKURA_CORE_API int GetColorIndexByName( const char *name );
SAKURA_CORE_API const char* GetColorNameByIndex( int index );

//	Sep. 10, 2002 genta CWSH.cppからの移動に伴う追加
bool ReadRegistry(HKEY Hive, char const *Path, char const *Item, char *Buffer, unsigned BufferSize);

#endif /* _ETC_UTY_H_ */


/*[EOF]*/
