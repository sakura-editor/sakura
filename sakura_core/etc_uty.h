/*!	@file
	@brief 共通関数群

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001-2002, genta
	Copyright (C) 2001, shoji masami, Stonee, MIK
	Copyright (C) 2002, aroka, hor, MIK, YAZAKI
	Copyright (C) 2003, genta, Moca
	Copyright (C) 2004, genta, novice
	Copyright (C) 2005, genta, aroka
	Copyright (C) 2006, ryoji, rastiv
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, kobake

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _ETC_UTY_H_
#define _ETC_UTY_H_

#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "global.h"
#include <shlobj.h>

// 2007.12.21 ryoji
// Windows SDK for Vista 以降には NewApis.h は含まれないので使用をあきらめる
// multimon.h もいずれは削除されてしまうかもしれないので使用しない
// Win95/NT4.0には存在しない API を使うようになるのでこれら古い OS への対応は不可です
//
// Note.
// 新しい Windows SDK を使用する場合、NewApis.h, multimon.h を除外するので
// WINVER に 0x0400 のような古い OS 対応の値を指定するとコンパイルエラーになります
// 明示指定がない場合のデフォルトは 0x0600 以上なので特に指定する必要はありません
// ちなみに通常の Windows.h 利用では昔から次のようにちょっと変なところがあります
// ・GetLongPathName() API は Win95(0x0400) 非対応なのに 0x0400 にしてもエラーになりません
// ・マルチモニタ関連 API は Win98(0x0410) 対応なのに 0x0500 以上にしないとエラーになります

#ifndef _INC_SDKDDKVER	// 新しい Windows SDK では windows.h が sdkddkver.h を include する
#define WANT_GETLONGPATHNAME_WRAPPER
#include <NewAPIs.h>
#include <MultiMon.h>
#endif

#if (_MSC_VER >= 1500) && (WINVER <= 0x0400)
#include <multimon.h>
#endif

/*!
	@brief 画面 DPI スケーリング
	@note 96 DPI ピクセルを想定しているデザインをどれだけスケーリングするか

	@date 2009.10.01 ryoji 高DPI対応用に作成
*/
class CDPI{
	static void Init()
	{
		if( !bInitialized )
		{
			HDC hDC = GetDC(NULL);
			nDpiX = GetDeviceCaps(hDC, LOGPIXELSX);
			nDpiY = GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(NULL, hDC);
			bInitialized = true;
		}
	}
	static int nDpiX;
	static int nDpiY;
	static bool bInitialized;
public:
	static int ScaleX(int x){Init(); return ::MulDiv(x, nDpiX, 96);}
	static int ScaleY(int y){Init(); return ::MulDiv(y, nDpiY, 96);}
	static int UnscaleX(int x){Init(); return ::MulDiv(x, 96, nDpiX);}
	static int UnscaleY(int y){Init(); return ::MulDiv(y, 96, nDpiY);}
	static void ScaleRect(LPRECT lprc)
	{
		lprc->left = ScaleX(lprc->left);
		lprc->right = ScaleX(lprc->right);
		lprc->top = ScaleY(lprc->top);
		lprc->bottom = ScaleY(lprc->bottom);
	}
	static void UnscaleRect(LPRECT lprc)
	{
		lprc->left = UnscaleX(lprc->left);
		lprc->right = UnscaleX(lprc->right);
		lprc->top = UnscaleY(lprc->top);
		lprc->bottom = UnscaleY(lprc->bottom);
	}
	static int PointsToPixels(int pt, int ptMag = 1){Init(); return ::MulDiv(pt, nDpiY, 72 * ptMag);}	// ptMag: 引数のポイント数にかかっている倍率
	static int PixelsToPoints(int px, int ptMag = 1){Init(); return ::MulDiv(px * ptMag, 72, nDpiY);}	// ptMag: 戻り値のポイント数にかける倍率
};

inline int DpiScaleX(int x){return CDPI::ScaleX(x);}
inline int DpiScaleY(int y){return CDPI::ScaleY(y);}
inline int DpiUnscaleX(int x){return CDPI::UnscaleX(x);}
inline int DpiUnscaleY(int y){return CDPI::UnscaleY(y);}
inline void DpiScaleRect(LPRECT lprc){CDPI::ScaleRect(lprc);}
inline void DpiUnscaleRect(LPRECT lprc){CDPI::UnscaleRect(lprc);}
inline int DpiPointsToPixels(int pt, int ptMag = 1){return CDPI::PointsToPixels(pt, ptMag);}
inline int DpiPixelsToPoints(int px, int ptMag = 1){return CDPI::PixelsToPoints(px, ptMag);}

#ifndef GA_PARENT
#define GA_PARENT		1
#define GA_ROOT			2
#define GA_ROOTOWNER	3
#endif
#define GA_ROOTOWNER2	100

#include "CHtmlHelp.h"	//	Jul.  6, 2001 genta
class CMemory;// 2002/2/3 aroka ヘッダ軽量化
class CEol;// 2002/2/3 aroka ヘッダ軽量化
class CBregexp;// 2002/2/3 aroka ヘッダ軽量化

BOOL MyWinHelp(HWND hWndMain, UINT uCommand, DWORD_PTR dwData);	/* WinHelp のかわりに HtmlHelp を呼び出す */	// 2006.07.22 ryoji

//!フォント選択ダイアログ
BOOL MySelectFont( LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner, bool );   // 2009.10.01 ryoji ポイントサイズ（1/10ポイント単位）引数追加

int AddLastChar( TCHAR*, int, TCHAR );/* 2003.06.24 Moca 最後の文字が指定された文字でないときは付加する */
int LimitStringLengthB( const char*, int, int, CMemory& );/* データを指定バイト数以内に切り詰める */
const char* GetNextLimitedLengthText( const char*, int, int, int*, int* );/* 指定長以下のテキストに切り分ける */
const char* GetNextLine( const char*, int, int*, int*, CEol* );/* CR0LF0,CRLF,LFCR,LF,CRで区切られる「行」を返す。改行コードは行長に加えない */
void GetLineColumn( const char*, int*, int* );
BOOL IsURL( const char*, int, int* );/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
BOOL IsMailAddress( const char*, int, int* );	/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
int IsNumber( const char*, int, int );/* 数値ならその長さを返す */	//@@@ 2001.02.17 by MIK
void ActivateFrameWindow( HWND );	/* アクティブにする */
void GetAppVersionInfo( HINSTANCE, int, DWORD*, DWORD* );	/* リソースから製品バージョンの取得 */
BOOL GetAbsolutePath( const char*, char*, BOOL );	/* 相対パス→絶対パス */
BOOL CheckEXT( const TCHAR*, const TCHAR* );	/* 拡張子を調べる */
char* my_strtok( char*, int, int*, const char* );
/* Shell Interface系(?) */
BOOL SelectDir(HWND, const TCHAR*, const TCHAR*, TCHAR* );	/* フォルダ選択ダイアログ */
//不要＆実装間違いのため
//ITEMIDLIST* CreateItemIDList( const char* );	/* パス名に対するアイテムＩＤリストを取得する */
//BOOL DeleteItemIDList( ITEMIDLIST* );/* アイテムＩＤリストを削除する */
BOOL ResolveShortcutLink(HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath);/* ショートカット(.lnk)の解決 */
void ResolvePath(TCHAR* pszPath); //!< ショートカットの解決とロングファイル名へ変換を行う。

/*
||	処理中のユーザー操作を可能にする
||	ブロッキングフック(?)(メッセージ配送)
*/
BOOL BlockingHook( HWND hwndDlgCancel );

HWND OpenHtmlHelp( HWND hWnd, LPCTSTR szFile, UINT uCmd, DWORD_PTR data, bool msgflag = true);
DWORD NetConnect ( const char strNetWorkPass[] );

int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith);
int cescape_j(const char* org, char* out, char cesc, char cwith);

/* ヘルプの目次を表示 */
void ShowWinHelpContents( HWND hwnd );

/*!	&の二重化
	メニューに含まれる&を&&に置き換える
	@author genta
	@date 2002/01/30 cescapeに拡張し，
	@date 2004/06/19 genta Generic mapping
*/
inline void dupamp(const TCHAR* org, TCHAR* out)
{	cescape( org, out, _T('&'), _T('&') ); }


/*
	scanf的安全スキャン

	使用例:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		//結果: a[0]=1, a[1]=23, a[2]=4 となる。
*/
int scan_ints(
	const TCHAR*	pszData,	//!< [in]  データ文字列
	const TCHAR*	pszFormat,	//!< [in]  データフォーマット
	int*			anBuf		//!< [out] 取得した数値 (要素数は最大32まで)
);

///////////////////////////////////////////////////////////////////////

/* カラー名＜＞インデックス番号の変換 */	//@@@ 2002.04.30
int GetColorIndexByName( const char *name );
const char* GetColorNameByIndex( int index );

//	Dec. 2, 2002 genta
HICON GetAppIcon( HINSTANCE hInst, int nResource, const TCHAR* szFile, bool bSmall = false);

//	Apr. 03, 2003 genta
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count);

//	Apr. 30, 2003 genta
//	ディレクトリの深さを調べる
int CalcDirectoryDepth(const TCHAR* path);

HWND MyGetAncestor( HWND hWnd, UINT gaFlags );	// 指定したウィンドウの祖先のハンドルを取得する	// 2007.07.01 ryoji

// 20051121 aroka
bool GetDateTimeFormat( TCHAR* szResult, int size, const TCHAR* format, const SYSTEMTIME& systime );
UINT32 ParseVersion( const TCHAR* ver );	//バージョン番号の解析
int CompareVersion( const TCHAR* verA, const TCHAR* verB );	//バージョン番号の比較

int getCtrlKeyState();

// 2005.11.26 aroka
bool IsLocalDrive( const TCHAR* pszDrive );

DWORD GetDllVersion( LPCTSTR lpszDllName );	// シェルやコモンコントロール DLL のバージョン番号を取得	// 2006.06.17 ryoji

void ChangeCurrentDirectoryToExeDir();
HMODULE LoadLibraryExedir(LPCTSTR pszDll);

BOOL GetSpecialFolderPath( int nFolder, LPTSTR pszPath );	// 特殊フォルダのパスを取得する	// 2007.05.19 ryoji
INT_PTR MyPropertySheet( LPPROPSHEETHEADER lppsph );	// 独自拡張プロパティシート	// 2007.05.24 ryoji

#endif /* _ETC_UTY_H_ */


/*[EOF]*/
