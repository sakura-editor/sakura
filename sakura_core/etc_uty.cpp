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

//#include <stdio.h>
#include <io.h>
#include "etc_uty.h"
#include "debug.h"
#include "CMemory.h"
#include "funccode.h"	//Stonee, 2001/02/23


/* 日付をフォーマット */
const char* MyGetDateFormat( char* pszDest, int nDestLen, int nDateFormatType, const char* pszDateFormat )
{
	SYSTEMTIME systime;
	const char* pszForm;
	DWORD dwFlags;
	::GetLocalTime( &systime );
	if( 0 == nDateFormatType ){
		dwFlags = DATE_LONGDATE;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = pszDateFormat;
	}
	::GetDateFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}




/* 時刻をフォーマット */
const char* MyGetTimeFormat( char* pszDest, int nDestLen, int nTimeFormatType, const char* pszTimeFormat )
{
	SYSTEMTIME systime;
	const char* pszForm;
	DWORD dwFlags;
	::GetLocalTime( &systime );
	if( 0 == nTimeFormatType ){
		dwFlags = 0;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = pszTimeFormat;
	}
	::GetTimeFormat( LOCALE_USER_DEFAULT, dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}




int CALLBACK MYBrowseCallbackProc(
	HWND hwnd,
	UINT uMsg,
	LPARAM lParam,
	LPARAM lpData
)
{
	switch( uMsg ){
	case BFFM_INITIALIZED:
//		MYTRACE( "BFFM_INITIALIZED (char*)lpData = [%s]\n", (char*)lpData );
		::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData );
		break;
	case BFFM_SELCHANGED:
//		MYTRACE( "BFFM_SELCHANGED\n" );
		break;
	}
	return 0;

}




/* フォルダ選択ダイアログ */
BOOL SelectDir( HWND hWnd, const char* pszTitle, const char* pszInitFolder, char* strFolderName )
{
	BOOL	bRes;
	char	szInitFolder[MAX_PATH];

	strcpy( szInitFolder, pszInitFolder );
	/* フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない*/
	CutLastYenFromDirectoryPath( szInitFolder );

	// SHBrowseForFolder()関数に渡す構造体
	BROWSEINFO bi;
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = strFolderName;
	bi.lpszTitle = pszTitle;
	bi.ulFlags = BIF_RETURNONLYFSDIRS/* | BIF_EDITBOX*//* | BIF_STATUSTEXT*/;
	bi.lpfn = MYBrowseCallbackProc;
	bi.lParam = (LPARAM)szInitFolder;
	bi.iImage = 0;
	// アイテムＩＤリストを返す
	// ITEMIDLISTはアイテムの一意を表す構造体
	ITEMIDLIST* pList = ::SHBrowseForFolder(&bi);
	if( NULL != pList ){
		// SHGetPathFromIDList()関数はアイテムＩＤリストの物理パスを探してくれる
		bRes = ::SHGetPathFromIDList( pList, strFolderName );
		// :SHBrowseForFolder()で取得したアイテムＩＤリストを削除
		::CoTaskMemFree( pList );
		if( bRes ){
			return TRUE;
		}else{
			return FALSE;
		}
	}
	return FALSE;
}




/* パス名に対するアイテムＩＤリストを作成する */
ITEMIDLIST* CreateItemIDList( const char* pszPath )
{
	ITEMIDLIST*		pIDL;
	IShellFolder*	pDesktopFolder;
	OLECHAR			ochPath[MAX_PATH + 1];
	ULONG			chEaten;			//文字列のサイズを受け取ります。
	ULONG			dwAttributes;		//属性を受け取ります。
	HRESULT			hRes;
	if( '\0' == pszPath[0] ){
		return NULL;
	}
	if( ::SHGetDesktopFolder( &pDesktopFolder ) != NOERROR ){
		return NULL;
	}
	//  これをしないとインターフェイスはダメなのです。
	::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszPath, -1, ochPath, MAX_PATH );
	//  実際にITEMIDLISTを取得します。
	hRes = pDesktopFolder->ParseDisplayName( NULL, NULL, ochPath, &chEaten, &pIDL, &dwAttributes);
	if( hRes != NOERROR ){
		pIDL = NULL;
	}
	pDesktopFolder->Release();
	return pIDL;
}




/* アイテムＩＤリストを削除する */
BOOL DeleteItemIDList( ITEMIDLIST* pIDL )
{
	IMalloc*	pMalloc;
	//  IMallocインターフェイスへのポインタを取得します。
	if( NOERROR != ::SHGetMalloc( &pMalloc ) ){
		return FALSE;
	}
	//  アイテムＩＤリストをリリースします。
	if( NULL != pIDL ){
		pMalloc->Free( pIDL );
	}
	pMalloc->Release();
	return TRUE;
}




/* 拡張子を調べる */
BOOL CheckEXT( const char* pszPath, const char* pszExt )
{
	char	szExt[_MAX_EXT];
	char*	pszWork;
	_splitpath( pszPath, NULL, NULL, NULL, szExt );
	pszWork = szExt;
	if( pszWork[0] == '.' ){
		pszWork++;
	}
	if( 0 == _stricmp( pszExt, pszWork ) ){
		return TRUE;
	}else{
		return FALSE;
	}
}




char* my_strtok( char* pszStr, int nStrLen, int* pnPos, char* pszDelemitor )
{
	int		i;
	int		nBgn = -1;
	int		bComment = FALSE;
	i = *pnPos;
	while( i < nStrLen ){
		if( NULL == strchr( pszDelemitor, pszStr[i] ) ){
			break;
		}
		++i;
	}
	if( i >= nStrLen ){
		return NULL;
	}
	nBgn = i;
	while( i < nStrLen ){
		if( bComment ){
			if( '\"' == pszStr[i] ){
				if( i + 1 < nStrLen ){
					if( '\"' != pszStr[i + 1] ){
						bComment = FALSE;
						goto last_of_loop;
					}else{
						++i;
					}
				}
			}
		}else{
			if( '\"' == pszStr[i] ){
				bComment = TRUE;
				goto last_of_loop;
			}
			if( NULL != strchr( pszDelemitor, pszStr[i] ) ){
				break;
			}
		}
last_of_loop:;
		++i;
	}
	pszStr[i] = '\0';
	*pnPos = i + 1;
	return &pszStr[nBgn];
}




/* ヘルプファイルのフルパスを返す */
char* GetHelpFilePath( char* pszHelpFile )
{
//	int		i;
	long	lPathLen;
	char	szHelpFile[_MAX_PATH + 1];
//	int		nCharChars;
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	/* ヘルプファイルのファイルパス */
	lPathLen = ::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szHelpFile, sizeof(szHelpFile)
	);
	_splitpath( szHelpFile, szDrive, szDir, NULL, NULL );
	strcpy( szHelpFile, szDrive );
	strcat( szHelpFile, szDir );
	strcat( szHelpFile, "sakura.hlp" );
	strcpy( pszHelpFile, szHelpFile );
	return pszHelpFile;
}




/* 相対パス→絶対パス */
BOOL GetAbsolutePath( const char* pszFilePathSrc, char* pszFilePathDes, BOOL bChangeCurrentFolder )
{
	char			szCurFolderOld[MAX_PATH];
	char			szCurFolder[MAX_PATH];
//	HANDLE			nFind;
//	WIN32_FIND_DATA	wfd;
	char			szPathSrc[MAX_PATH];
//	char			szPath2[MAX_PATH];
//	char			szPath3[MAX_PATH];
//	char			szPath4[MAX_PATH];
//	int				nPathSrcLen;
//	int				i;
	int				nCharChars;
//	int				nDirBgn;

	char			szDrive[_MAX_DRIVE];
	char			szDir[_MAX_DIR];
	char			szFname[_MAX_FNAME];
	char			szExt[_MAX_EXT];
	int				nFolderLen;
//	int				nDriveOrg;
	BOOL			bRet;
	bRet = TRUE;


	/* カレントフォルダの退避 */
	if( !bChangeCurrentFolder ){	/* カレントフォルダを変更するか */
		::GetCurrentDirectory( MAX_PATH, szCurFolderOld );
	}

	_splitpath( pszFilePathSrc, szDrive, szDir, szFname, szExt );
//	MYTRACE( "_splitpath 関数によるパス名の分解:\n" );
//	MYTRACE( " pszFilePathSrc    : %s\n", pszFilePathSrc );
//	MYTRACE( "  ドライブ    : %s\n", szDrive );
//	MYTRACE( "  ディレクトリ: %s\n", szDir );
//	MYTRACE( "  ファイル名  : %s\n", szFname );
//	MYTRACE( "  拡張子      : %s\n", szExt );
	_makepath( szCurFolder, szDrive, szDir, NULL, NULL );
//	MYTRACE( "  SetCurrentDirectory     : %s\n", szCurFolder );
	if( 0 < strlen( szCurFolder ) && 0 == ::SetCurrentDirectory( szCurFolder ) ){
		bRet =  FALSE;
		goto end_of_func;
	}
	::GetCurrentDirectory( MAX_PATH, szCurFolder );
//	MYTRACE( "  GetCurrentDirectory     : %s\n", szCurFolder );

	/* フォルダの最後が半角かつ'\\'の場合は、取り除く */
	nFolderLen = strlen( szCurFolder );
	if( 0 < nFolderLen ){
		nCharChars = &szCurFolder[nFolderLen] - CMemory::MemCharPrev( szCurFolder, nFolderLen, &szCurFolder[nFolderLen] );
		if( 1 == nCharChars && '\\' == szCurFolder[nFolderLen - 1] ){
			szCurFolder[nFolderLen - 1] = '\0';
		}
	}
	wsprintf( szPathSrc, "%s\\%s%s", szCurFolder, szFname, szExt );
//	MYTRACE( "  絶対パス      : %s\n", szPathSrc );
	strcpy( pszFilePathDes, szPathSrc );
end_of_func:;
	/* カレントフォルダの復帰 */
	if( !bChangeCurrentFolder ){	/* カレントフォルダを変更するか */
		::SetCurrentDirectory( szCurFolderOld );
	}
	return bRet;

}




/* ロングファイル名を取得する */
BOOL GetLongFileName( const char* pszFilePathSrc, char* pszFilePathDes )
{
//	HANDLE			nFind;
//	WIN32_FIND_DATA	wfd;
//	char			szPathSrc[MAX_PATH];
//	char			szPath2[MAX_PATH];
//	char			szPath3[MAX_PATH];
//	int				nPathSrcLen;
//	int				i;
//	int				j;
//	int				nCharChars;
//	int				nDriveOrg;
	/* 相対パス→絶対パス */
	if( FALSE == GetAbsolutePath( pszFilePathSrc, pszFilePathDes, TRUE ) ){
		return FALSE;
	}

	ITEMIDLIST* pIDL;
	/* パス名に対するアイテムＩＤリストを作成する */
	if( NULL == ( pIDL = CreateItemIDList( pszFilePathSrc ) ) ){
		//	May 9, 2000 genta
		char * name;
		int length;
		length = ::GetFullPathName( pszFilePathSrc, MAX_PATH, pszFilePathDes, &name );
//		::MessageBox( NULL, pszFilePathDes, "GetFullPathName", MB_OK );
		if( length <= 0 || MAX_PATH <= length ){
			::MessageBox( NULL, pszFilePathDes, "Too Long Path", MB_OK );
			return FALSE;
		}
	}
	else{
		// アイテムＩＤリストの物理パスを取得
		::SHGetPathFromIDList( pIDL, pszFilePathDes );

		/* アイテムＩＤリストを削除する */
		DeleteItemIDList( pIDL );
	}
//	::MessageBox( NULL, pszFilePathDes, pszFilePathSrc, MB_OK );
	return TRUE;

//-2000.1.6 自前の処理をやめた
//-
//-	nPathSrcLen = strlen( szPathSrc );
//-	if( 2 < nPathSrcLen
//-	 && '\\' == szPathSrc[0]
//-	 && '\\' == szPathSrc[1]
//-	){
//-		/* \\サーバ名\シェア名\... */
//-		i = 2 - 1;
//-		for( j = 0; j < 2; ++j ){
//-			++i;
//-			for( ; i < nPathSrcLen; ++i ){
//-				nCharChars = CMemory::MemCharNext( szPathSrc, nPathSrcLen, &szPathSrc[i] ) - &szPathSrc[i];
//-				if( 1 == nCharChars && '\\' == szPathSrc[i] ){
//-					break;
//-				}
//-				if( 2 == nCharChars ){
//-					++i;
//-				}
//-			}
//-		}
//-		nDriveOrg = i;
//-	}else{
//-		/* ドライブ名\... */
//-		nDriveOrg = 2;
//-	}
//-	strcpy( szPath2, "" );
//-	nFind = ::FindFirstFile( szPathSrc, (WIN32_FIND_DATA*)&wfd );
//-	if( INVALID_HANDLE_VALUE == nFind ){
//-		return FALSE;
//-	}
//-	::FindClose( nFind );
//-	while( 1 ){
//-			strcpy( szPath3, "\\" );
//-			strcat( szPath3, wfd.cFileName );
//-			strcat( szPath3, szPath2 );
//-			strcpy( szPath2, szPath3 );
//-		nPathSrcLen = strlen( szPathSrc );
//-		nCharChars = &szPathSrc[nPathSrcLen] - CMemory::MemCharPrev( szPathSrc, nPathSrcLen, &szPathSrc[nPathSrcLen] );
//-		for( i = nPathSrcLen - nCharChars; i >= 0; i-- ){
//-			if( 1 == nCharChars && '\\' == szPathSrc[i] ){
//-				if( 0 == strcmp( ".", wfd.cFileName ) ){
//-					strcpy( szPath3, &szPathSrc[i] );
//-					strcat( szPath3, szPath2 );
//-					strcpy( szPath2, szPath3 );
//-				}
//-				szPathSrc[i] = '\0';
//-				break;
//-			}
//-			nCharChars = &szPathSrc[i] - CMemory::MemCharPrev( szPathSrc, nPathSrcLen, &szPathSrc[i] );
//-			if( 2 == nCharChars ){
//-				i--;
//-			}
//-		}
//-		if( 0 > i ){
//-			break;
//-		}
//-		if( nDriveOrg >= i ){
//-			strcpy( szPath3, szPathSrc );
//-			strcat( szPath3, szPath2 );
//-			strcpy( szPath2, szPath3 );
//-			break;
//-		}
//-		if( 2 == strlen( szPathSrc ) ){
//-			if( szPathSrc[1] == ':' ){
//-				strcpy( szPath3, szPathSrc );
//-				strcat( szPath3, szPath2 );
//-				strcpy( szPath2, szPath3 );
//-				break;
//-			}
//-		}
//-		nFind = ::FindFirstFile( szPathSrc, (WIN32_FIND_DATA*)&wfd );
//-		if( INVALID_HANDLE_VALUE == nFind ){
//-			strcpy( szPath3, szPathSrc );
//-			strcat( szPath3, szPath2 );
//-			strcpy( szPath2, szPath3 );
//-			break;
//-		}
//-		::FindClose( nFind );
//-	}
//-	strcpy( pszFilePathDes, szPath2 );
//-	/* 最悪の場合を考えて、存在しないロングパス名を生成した場合は、もとのパス名にする */
//-	if( -1 == _access( pszFilePathDes, 0 ) ){
//-		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, "作者に教えて欲しいエラー",
//-			"ロングファイルネームの生成でエラーが出ましたので、\n元のファイル名を使います。\n\nもとのファイル名=[%s]\n失敗したロングファイル名=[%s]",
//-			pszFilePathSrc, pszFilePathDes
//-		);
//-		strcpy( pszFilePathDes, pszFilePathSrc );
//-	}
//-	return TRUE;
}




/* ファイルのフルパスを、フォルダとファイル名に分割 */
/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
void SplitPath_FolderAndFile( const char* pszFilePath, char* pszFolder, char* pszFile )
{
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	char	szFname[_MAX_FNAME];
	char	szExt[_MAX_EXT];
	int		nFolderLen;
	int		nCharChars;
	_splitpath( pszFilePath, szDrive, szDir, szFname, szExt );
	if( NULL != pszFolder ){
		strcpy( pszFolder, szDrive );
		strcat( pszFolder, szDir );
		/* フォルダの最後が半角かつ'\\'の場合は、取り除く */
		nFolderLen = strlen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CMemory::MemCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && '\\' == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = '\0';
			}
		}
	}
	if( NULL != pszFile ){
		strcpy( pszFile, szFname );
		strcat( pszFile, szExt );
	}
	return;
}




/* システムリソースを調べる
	Win16 の時は、GetFreeSystemResources という関数がありました。しかし、Win32 ではありません。
	サンクを作るだの DLL を作るだのは難しすぎます。簡単な方法を説明します。
	お使いの Windows95 の [アクセサリ]-[システムツール] にリソースメータがあるのなら、
	c:\windows\system\rsrc32.dll があるはずです。これは、リソースメータという Win32 アプリが、
	Win16 の GetFreeSystemResources 関数を呼ぶ為の DLL です。これを使いましょう。
*/
BOOL GetSystemResources(
	int*	pnSystemResources,
	int*	pnUserResources,
	int*	pnGDIResources
)
{
	#define GFSR_SYSTEMRESOURCES	0x0000
	#define GFSR_GDIRESOURCES		0x0001
	#define GFSR_USERRESOURCES		0x0002
	HINSTANCE	hlib;
	int (CALLBACK *GetFreeSystemResources)( int );

	hlib = ::LoadLibrary( "RSRC32.dll" );
	if( (int)hlib > 32 ){
		GetFreeSystemResources = (int (CALLBACK *)( int ))GetProcAddress(
			hlib,
			"_MyGetFreeSystemResources32@4"
		);
		if( GetFreeSystemResources != NULL ){
			*pnSystemResources = GetFreeSystemResources( GFSR_SYSTEMRESOURCES );
			*pnUserResources = GetFreeSystemResources( GFSR_USERRESOURCES );
			*pnGDIResources = GetFreeSystemResources( GFSR_GDIRESOURCES );
			::FreeLibrary( hlib );
			return TRUE;
		}else{
			::FreeLibrary( hlib );
			return FALSE;
		}
	}else{
		return FALSE;
	}
}




/* システムリソースのチェック */
BOOL CheckSystemResources( const char* pszAppName )
{
	int		nSystemResources;
	int		nUserResources;
	int		nGDIResources;
	char*	pszResourceName;
	/* システムリソースの取得 */
	if( GetSystemResources( &nSystemResources, &nUserResources,	&nGDIResources ) ){
//		MYTRACE( "nSystemResources=%d\n", nSystemResources );
//		MYTRACE( "nUserResources=%d\n", nUserResources );
//		MYTRACE( "nGDIResources=%d\n", nGDIResources );
		pszResourceName = NULL;
		if( nSystemResources <= 5 ){
			pszResourceName = "システム ";
		}else
		if( nUserResources <= 5 ){
			pszResourceName = "ユーザー ";
		}else
		if( nGDIResources <= 5 ){
			pszResourceName = "GDI ";
		}
		if( NULL != pszResourceName ){
			::MessageBeep( MB_ICONHAND );
			::MessageBeep( MB_ICONHAND );
//			if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNO | MB_ICONSTOP | MB_APPLMODAL | MB_TOPMOST, pszAppName,
			::MYMESSAGEBOX( NULL, MB_OK | /*MB_YESNO | */ MB_ICONSTOP | MB_APPLMODAL | MB_TOPMOST, pszAppName,
				"%sリソースが極端に不足しています。\n\
このまま%sを起動すると、正常に動作しない可能性があります。\n\
新しい%sの起動を中断します。\n\
\n\
システム リソース\t残り  %d%%\n\
User リソース\t残り  %d%%\n\
GDI リソース\t残り  %d%%\n\n",
				pszResourceName,
				pszAppName,
				pszAppName,
				nSystemResources,
				nUserResources,
				nGDIResources
			);
//			) ){
				return FALSE;
//			}
		}
	}
	return TRUE;
}




struct VS_VERSION_INFO_HEAD {
	WORD	wLength;
	WORD	wValueLength;
	WORD	bText;
	WCHAR	szKey[16];
	VS_FIXEDFILEINFO Value;
};

/* リソースから製品バージョンの取得 */
void GetAppVersionInfo(
	HINSTANCE	hInstance,
	int			nVersionResourceID,
	DWORD*		pdwProductVersionMS,
	DWORD*		pdwProductVersionLS
)
{
	HRSRC					hRSRC;
	HGLOBAL					hgRSRC;
	VS_VERSION_INFO_HEAD*	pVVIH;
	/* リソースから製品バージョンの取得 */
	*pdwProductVersionMS = 0;
	*pdwProductVersionLS = 0;
	if( NULL != ( hRSRC = ::FindResource( hInstance, MAKEINTRESOURCE(nVersionResourceID), RT_VERSION ) )
	 && NULL != ( hgRSRC = ::LoadResource( hInstance, hRSRC ) )
	 && NULL != ( pVVIH = (VS_VERSION_INFO_HEAD*)::LockResource( hgRSRC ) )
	){
		*pdwProductVersionMS = pVVIH->Value.dwProductVersionMS;
		*pdwProductVersionLS = pVVIH->Value.dwProductVersionLS;
	}
	return;

}




/* Windowsバージョンのチェック */
BOOL CheckWindowsVersion( const char* pszAppName )
{
	OSVERSIONINFO	osvi;
	memset( (void *)&osvi, 0, sizeof( osvi ) );
	osvi.dwOSVersionInfoSize = sizeof( osvi );
	if( ::GetVersionEx( &osvi ) ){
		if( osvi.dwMajorVersion < 4 ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, pszAppName,
				"このアプリケーションを実行するには、\nWindows95以上 または WindowsNT4.0以上のOSが必要です。\nアプリケーションを終了します。"
			);
			return FALSE;
		}
	}else{
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, pszAppName,
			"OSのバージョンが取得できません。\nアプリケーションを終了します。"
		);
		return FALSE;
	}
	return TRUE;
}

// From Here Jul. 5, 2001 shoji masami
/*! NTプラットフォームかどうか調べる

	@retval TRUE NT platform
	@retval FALSE non-NT platform
*/
bool CheckWindowsVersionNT( void )
{
	OSVERSIONINFO osVer;
	osVer.dwOSVersionInfoSize = sizeof(osVer);
	GetVersionEx(&osVer);

	if (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
		return true;	// NT系

	return false;		// それ以外
}
// To Here Jul. 5, 2001 shoji masami



/* アクティブにする */
void ActivateFrameWindow( HWND hwnd )
{
	if( ::IsIconic( hwnd ) ){
		::ShowWindow( hwnd, SW_RESTORE );
	}
	::ShowWindow( hwnd, SW_SHOW );
	::SetForegroundWindow( hwnd );
	return;
}




/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
BOOL IsURL( const char* pszText, int nTextLen, int* pnUrlLen )
{
	BOOL			bFindURL;
//	int				nTextLen;
	int				nCharChars;
	int				i;
	const char*		ppszURLHEADARR[]={
		"mailto:",
		"http://",
		"https://",
		"ftp://",
		"file://",
		"gopher://",
		"news:",
		"nntp://",
		"telnet://",
		"wais://",
		"prospero://",
	};
	const int pnURLHEADLENARR[]={
		7,
		7,
		8,
		6,
		7,
		9,
		5,
		7,
		9,
		7,
		11
	};
	int			nURLHEADNUM = sizeof( ppszURLHEADARR ) / sizeof( ppszURLHEADARR[0] );
	const char*	pszHeahHead = "mhfgntwp";
	const int	nHeadHeadLen = 8;
	int			nURLHeadLen;
//	nTextLen = strlen( pszText );
	bFindURL = FALSE;
	for( i = 0 ; i < nHeadHeadLen; ++i ){
		if( pszText[0] == pszHeahHead[i] ){
			break;
		}
	}
	if( i < nHeadHeadLen ){
		for( i = 0; i < nURLHEADNUM; ++i ){
//			nURLHeadLen = strlen( ppszURLHEADARR[i] );
			nURLHeadLen = pnURLHEADLENARR[i];

			if( nTextLen >= nURLHeadLen
			 && 0 == memcmp( pszText, ppszURLHEADARR[i], nURLHeadLen ) ){
				bFindURL = TRUE;
				break;
			}
		}
	}
	if( bFindURL ){
		if( 0 == i ){
			/* メールアドレスか? */
			/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
			if( TRUE == IsMailAddress( &pszText[nURLHeadLen], strlen( &pszText[nURLHeadLen] ), pnUrlLen ) ){
				*pnUrlLen  = *pnUrlLen + nURLHeadLen;
				return TRUE;
			}else{
				return FALSE;
			}
		}

		for( i = nURLHeadLen; i < nTextLen; ++i ){
			nCharChars = CMemory::MemCharNext( pszText, nTextLen, &pszText[i] ) - &pszText[i];
			/* URL文字列の終わりに達したか */
			if( 0 == nCharChars ){
				break;
			}
			if( 2 == nCharChars ){
				break;
//				++i;
//				continue;
			}
			if( ' '  == pszText[i]
			 || '"'  == pszText[i]
			 || '\'' == pszText[i]
			 || '<'  == pszText[i]
			 || '>'  == pszText[i]
			 || '\r' == pszText[i]
			 || '\n' == pszText[i]
			 || '\t' == pszText[i]
			 || '\0' == pszText[i]
			 || ')'  == pszText[i]
			 || '}'  == pszText[i]		//Stonee, 2001/06/24  URLの区切り判定を修正
			 || '{'  == pszText[i]		//Stonee, 2001/06/24
			){
				break;
			}
		}
		if( 0 == i - nURLHeadLen ){
			return FALSE;
		}
		*pnUrlLen = i;
		return TRUE;
	}
	/* メールアドレスか? */
	/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
	if( TRUE == IsMailAddress( pszText, strlen( pszText ), pnUrlLen ) ){
		return TRUE;
	}
	return FALSE;
}




/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
BOOL IsMailAddress( const char* pszBuf, int nBufLen, int* pnAddressLenfth )
{
//	int		i;
	int		j;
//	int		wk_nBegin;
//	int		wk_nEnd;
//	int		nAtPos;
	int		nDotCount;
//	int		nAlphaCount;
	int		nBgn;
//	int		nLoop;
//	BOOL bDot = FALSE;


	j = 0;
	if( (pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
	 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
	 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
//	 || (pszBuf[j] == '.')
//	 || (pszBuf[j] == '-')
//	 || (pszBuf[j] == '_')
//	 || (pszBuf[j] == '=')
//	 || (pszBuf[j] == '%')
//	 || (pszBuf[j] == '$')
//	   )
	){
		j++;
	}else{
		return FALSE;
	}
	while( j < nBufLen - 2 &&
		(
		(pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
	 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
	 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
	 || (pszBuf[j] == '.')
	 || (pszBuf[j] == '-')
	 || (pszBuf[j] == '_')
//	 || (pszBuf[j] == '=')
//	 || (pszBuf[j] == '%')
//	 || (pszBuf[j] == '$')
		)
	){
		j++;
	}
	if( j == 0 || j >= nBufLen - 2  ){
		return FALSE;
	}
	if( '@' != pszBuf[j] ){
		return FALSE;
	}
//	nAtPos = j;
	j++;
	nDotCount = 0;
//	nAlphaCount = 0;


	while( 1 ){
		nBgn = j;
		while( j < nBufLen &&
			(
			(pszBuf[j] >= 'a' && pszBuf[j] <= 'z')
		 || (pszBuf[j] >= 'A' && pszBuf[j] <= 'Z')
		 || (pszBuf[j] >= '0' && pszBuf[j] <= '9')
//		 || (pszBuf[j] == '.')
		 || (pszBuf[j] == '-')
		 || (pszBuf[j] == '_')
		//	 || (pszBuf[j] == '=')
		//	 || (pszBuf[j] == '%')
		//	 || (pszBuf[j] == '$')
			)
		){
			j++;
		}
		if( 0 == j - nBgn ){
			return FALSE;
		}
		if( '.' != pszBuf[j] ){
			if( 0 == nDotCount ){
				return FALSE;
			}else{
				break;
			}
		}else{
			nDotCount++;
			j++;
		}
	}
	if( NULL != pnAddressLenfth ){
		*pnAddressLenfth = j;
	}
	return TRUE;
}




//@@@ 2001.02.17 Start by MIK
//#ifdef COMPILE_COLOR_DIGIT
/*
 * 数値なら長さを返す。
 * 10進数の整数または小数。16進数(正数)。
 * 文字列   数値(色分け)
 * ---------------------
 * 123      123
 * 0123     0123
 * 0xfedc   0xfedc
 * -123     -123
 * &H9a     &H9a     (ただしソース中の#ifを有効にしたとき)
 * -0x89a   0x89a
 * 0.5      0.5
 * 0.56.1   0.56 , 1 (ただしソース中の#ifを有効にしたら"0.56.1"になる)
 * .5       5        (ただしソース中の#ifを有効にしたら".5"になる)
 * -.5      5        (ただしソース中の#ifを有効にしたら"-.5"になる)
 * 123.     123
 * 0x567.8  0x567 , 8
 */
int IsNumber(const char* buf, int offset, int length)
{
	const char *p, *q;
	int i = 0;
	int d;

	/* 高速化のためポインタを使う! */
	p = &buf[offset];
	//q = &buf[length];	/* 数字以外のときの高速化のためここではしない */

	if( *p >= '0' && *p <= '9' ){
		q = &buf[length];
		/* 8,10,16進数 */
		p++;
		if( p < q ){
			if( *(p - 1) == '0' && *p == 'x' ){
				/* 16進数 */
				p++;
				for( i = 2; p < q; p++, i++ ){
					if( (*p < '0' || *p > '9')
					&& (*p < 'A' || *p > 'F')
					&& (*p < 'a' || *p > 'f') ){
						break;
					}
				}
				/* "0x" のときは "0" が数値になる */
				if( i == 2 ){
					return 1;
				}
			}else{
				/* 10進数または8進数 */
				for( i = 1, d = 0; p < q; p++, i++ ){
					if( *p < '0' || *p > '9' ){
						if( *p == '.' ){
							d++;
							if( d > 1 ) break;
							//if( d > 1 )	return 0;
						}else{
							break;
						}
					}
				}
				/* "." で終わるときは "." は入れない */
				if( *(p - 1) == '.' ){
					i--;
				}
			}
		}else{
			return 1;
		}
	}else if( *p == '-' /* || *p == '+' */ ){
		q = &buf[length];
		/* マイナスの10進数 */
		p++;
		if( p < q ){
			if( *p == '0' ){
				p++;
				if( p < q ){
					if( *p == 'x' ){
						p++;
						if( p < q ){						//***16進数にマイナスがないときはここを有効に。
						//for( i = 3; p < q; p++, i++ ){	//***16進数にマイナスもあるときはここを有効に。
							if( (*p >= '0' && *p <= '9')
							||  (*p >= 'A' && *p <= 'F')
							||  (*p >= 'a' && *p <= 'f') ){
								//break;					//***16進数にマイナスもあるときはここを有効に。
								/* 16進数なのでマイナス符号ではない */
								return 0;					//***16進数にマイナスがないときはここを有効に。
							}
						}
						//if( i == 3 ){						//***16進数にマイナスもあるときはここを有効に。
							/* "-0x" のうち "-0" が数値 */
							return 2;
						//}									//***16進数にマイナスもあるときはここを有効に。
					}else{
						for( i = 2, d = 0; p < q; p++, i++ ){
							if( *p < '0' || *p > '9' ){
								if( *p == '.' ){
									d++;
									if( d > 1 ) break;
									//if( d > 1 ) return 0;
								}else{
									break;
								}
							}
						}
						/* "." で終わるときは "." は入れない */
						if( *(p - 1) == '.' ){
							i--;
						}
					}
				}else{
					return 2;
				}
			}else{
				for( i = 1, d = 0; p < q; p++, i++ ){
					if( *p < '0' || *p > '9' ){
						if( *p == '.' ){
							d++;
							if( d > 1 || i == 1 ) break;	/* こっちは -.5 を数値としない */
							//if( d > 1 ) break;			/* こっちは -.5 を数値とする */
							//if( i == 1 ) break;			/* -.5 を数値としない */
						}else{
							break;
						}
					}
				}
				if( i == 1 ){
					/* マイナス符号ではなかった */
					return 0;
				}else if( *(p - 1) == '.' ){
					if( i == 2 ){
						i = 0;
					}else{
						i--;
					}
				}
			}
		}else{
			return 0;
		}
//#if 0  /* 小数点始まりの数字 : これだと "0.5.1.6" が全部数値になってしまいます */
	}else if( *p == '.' ){
		/* 小数点始まりの数値 */
		q = &buf[length];
		p++;
		for( i = 1, d = 1; p < q; p++, i++ ){
			if( *p < '0' || *p > '9' ){
				d++;
				if( d > 1 ) break;
				//if( d > 1 ) return 0;
				break;
			}
		}
		if( i == 1 ){
			return 0;
		}
//#endif
#if 0  /* VBの16進数 */
	}else if( *p == '&' ){
		q = &buf[length];
		/* VBの16進数 */
		p++;
		if( p < q ){
			if( *p == 'H' || *p == 'h' ){
				p++;
				for( i = 2; p < q; p++, i++ ){
					if( (*p < '0' || *p > '9')
					&&  (*p < 'A' || *p > 'F')
					&&  (*p < 'a' || *p > 'f') ){
						break;
					}
				}
				/* "&H" のときは数値ではない */
				if( i == 2 ){
					return 0;
				}
			}else{
				return 0;
			}
		}else{
			return 0;
		}
#endif
	}else{
		/* 数値ではない */
		return 0;
	}

	return i;
}
//#endif
//@@@ 2001.02.17 End by MIK




BOOL IsFilePath( const char* pLine, int* pnBgn, int* pnPathLen )
{
	int		i;
	int		j;
	int		nLineLen;
	char	szJumpToFile[1024];
	memset( szJumpToFile, 0, sizeof( szJumpToFile ) );

	nLineLen = strlen( pLine );

	for( i = 0; i < nLineLen; ++i ){
		if( ' ' != pLine[i] &&
			'\t' != pLine[i] &&
			'\"' != pLine[i]
		){
			break;
		}
	}
	if( i < nLineLen &&
		'#' == pLine[i]
	){
		for( ; i < nLineLen; ++i ){
			if( '<'  == pLine[i] ||
				'\"' == pLine[i]
			){
				++i;
				break;
			}
		}
	}
	if( i >= nLineLen ){
		goto can_not_tagjump;
	}
	*pnBgn = i;
	j = 0;
	for( ; i <= nLineLen && j + 1 < sizeof(szJumpToFile); ++i ){
		if( ( i == nLineLen    ||
			  pLine[i] == ' '  ||
			  pLine[i] == '('  ||
			  pLine[i] == '\r' ||
			  pLine[i] == '\n' ||
			  pLine[i] == '\0' ||
			  pLine[i] == '>'  ||
			  pLine[i] == '"'
			) &&
			0 < strlen( szJumpToFile )
		){
			if( -1 != _access( (const char *)szJumpToFile, 0 ) ){
				i--;
				break;
			}
		}
		if( pLine[i] == '\r'  ||
			pLine[i] == '\n' ){
			break;
		}
//	From Here Sept. 27, 2000 JEPRO タグジャンプできないのは以下の文字が1バイトコードで現れるときのみとした。
//	(SJIS2バイトコードの2バイト目に現れる場合はパス名使用禁止文字とは認識しないで無視するように変更)
//		if( /*pLine[i] == '/' ||*/
//			pLine[i] == '<' ||
//			pLine[i] == '>' ||
//			pLine[i] == '?' ||
//			pLine[i] == '"' ||
//			pLine[i] == '|' ||
//			pLine[i] == '*'
//		){
//			goto can_not_tagjump;
//		}
//
//		szJumpToFile[j] = pLine[i];
//		j++;
//	}
//  To Here comment out
//	From Here Sept. 27, 2000 JEPRO added
//			  Oct. 3, 2000 JEPRO corrected
		if( ( /*pLine[i] == '/' ||*/
			 pLine[i] == '<' ||	//	0x3C
			 pLine[i] == '>' ||	//	0x3E
			 pLine[i] == '?' ||	//	0x3F
			 pLine[i] == '"' ||	//	0x22
			 pLine[i] == '|' ||	//	0x7C
			 pLine[i] == '*'	//	0x2A
			) &&
			/* 上の文字がSJIS2バイトコードの2バイト目でないことを、1つ前の文字がSJIS2バイトコードの1バイト目でないことで判断する */
			( i > 0 &&
				(
				 (unsigned char)pLine[i - 1] < (unsigned char)0x81 ||
				 ( (unsigned char)0x9F < (unsigned char)pLine[i - 1] && (unsigned char)pLine[i - 1] < (unsigned char)0xE0 ) ||
				   (unsigned char)0xEF < (unsigned char)pLine[i - 1]
				)
			) ){
			goto can_not_tagjump;
		}else{
		szJumpToFile[j] = pLine[i];
		j++;
		}
	}
//	To Here Sept. 27, 2000
//	if( i >= nLineLen ){
//		return FALSE;
//	}
	if( 0 < strlen( szJumpToFile ) &&
		-1 != _access( (const char *)szJumpToFile, 0 ) )
	{
	}else{
//#ifdef _DEBUG
//		m_cShareData.TraceOut( "ファイルが存在しない。szJumpToFile=[%s]\n", szJumpToFile );
//#endif
		goto can_not_tagjump;
	}


	*pnPathLen = strlen( szJumpToFile );
	return TRUE;
	can_not_tagjump:;
	return FALSE;

}




void GetLineColm( const char* pLine, int* pnJumpToLine, int* pnJumpToColm )
{
	int		i;
	int		j;
	int		nLineLen;
	char	szNumber[32];
	nLineLen = strlen( pLine );
	i = 0;
	for( ; i < nLineLen; ++i ){
		if( pLine[i] >= '0' &&
			pLine[i] <= '9' ){
			break;
		}
	}
	memset( szNumber, 0, sizeof( szNumber ) );
	if( i >= nLineLen ){
	}else{
		/* 行位置 改行単位行番号(1起点)の抽出 */
		j = 0;
		for( ; i < nLineLen && j + 1 < sizeof( szNumber ); ){
			szNumber[j] = pLine[i];
			j++;
			++i;
			if( pLine[i] >= '0' &&
				pLine[i] <= '9' ){
				continue;
			}
			break;
		}
		*pnJumpToLine = atoi( szNumber );

		/* 桁位置 改行単位行先頭からのバイト数(1起点)の抽出 */
		if( i < nLineLen && pLine[i] == ',' ){
			memset( szNumber, 0, sizeof( szNumber ) );
			j = 0;
			++i;
			for( ; i < nLineLen && j + 1 < sizeof( szNumber ); ){
				szNumber[j] = pLine[i];
				j++;
				++i;
				if( pLine[i] >= '0' &&
					pLine[i] <= '9' ){
					continue;
				}
				break;
			}
			*pnJumpToColm = atoi( szNumber );
		}
	}
	return;
}




//	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
//	/* bLFisOK→LFだけでも改行とみなす */
//	const char* GetNextLine( const char* pText, int nTextLen, int* pnLineLen, int* pnBgn, BOOL* pbEOL, BOOL bLFisOK )
//	{
//		int		i;
//		int		nBgn;
//		nBgn = *pnBgn;
//		if( nBgn >= nTextLen ){
//			return NULL;
//		}
//		if( NULL != pbEOL ){
//			*pbEOL = TRUE;
//		}
//		for( i = *pnBgn; i < nTextLen; ++i ){
//			if( bLFisOK &&
//				'\n' == pText[i]
//			){
//				*pnBgn = i + 1;
//				break;
//			}
//			if( '\r' == pText[i]
//			 && i + 1 < nTextLen
//			 && '\n' == pText[i + 1]
//			){
//				*pnBgn = i + 2;
//				break;
//			}
//		}
//		if( i >= nTextLen ){
//			*pnBgn = i;
//			if( NULL != pbEOL ){
//				*pbEOL = FALSE;
//			}
//		}
//		*pnLineLen = i - nBgn;
//		return &pText[nBgn];
//	}
//




/* CR0LF0,CRLF,LFCR,LF,CRで区切られる「行」を返す。改行コードは行長に加えない */
const char* GetNextLine(
	const char*		pData,
	int				nDataLen,
	int*			pnLineLen,
	int*			pnBgn,
	CEOL*			pcEol
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	//	May 15, 2000 genta
	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		/* 改行コードがあった */
		if( pData[i] == '\n' || pData[i] == '\r' ){
			/* 行終端子の種類を調べる */
			 pcEol->GetTypeFromString( &pData[i], nDataLen - i );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}




/* 指定長以下のテキストに切り分ける */
const char* GetNextLimitedLengthText( const char* pText, int nTextLen, int nLimitLen, int* pnLineLen, int* pnBgn )
{
	int		i;
	int		nBgn;
	int		nCharChars;
	nBgn = *pnBgn;
	if( nBgn >= nTextLen ){
		return NULL;
	}
	for( i = nBgn; i + 1 < nTextLen; ++i ){
		nCharChars = CMemory::MemCharNext( pText, nTextLen, &pText[i] ) - &pText[i];
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( i + nCharChars - nBgn >= nLimitLen ){
			break;
		}
		i += ( nCharChars - 1 );
	}
	*pnBgn = i;
	*pnLineLen = i - nBgn;
	return &pText[nBgn];
}




/* データを指定バイト数以内に切り詰める */
int LimitStringLengthB( const char* pszData, int nDataLength, int nLimitLengthB, CMemory& cmemDes )
{
	int	i;
	int	nCharChars;
	int	nDesLen;
	nDesLen = 0;
	for( i = 0; i < nDataLength; ){
		nCharChars = CMemory::MemCharNext( pszData, nDataLength, &pszData[i] ) - &pszData[i];
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( nDesLen + nCharChars > nLimitLengthB ){
			break;
		}
		nDesLen += nCharChars;
		i += nCharChars;
	}
	cmemDes.SetData( pszData, nDesLen );
	return nDesLen;
}




/* フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない */
void CutLastYenFromDirectoryPath( char* pszFolder )
{
	if( 3 == strlen( pszFolder )
	 && pszFolder[1] == ':'
	 && pszFolder[2] == '\\'
	){
		/* ドライブ名:\ */
	}else{
		/* フォルダの最後が半角かつ'\\'の場合は、取り除く */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = strlen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CMemory::MemCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && '\\' == pszFolder[nFolderLen - 1] ){
				pszFolder[nFolderLen - 1] = '\0';
			}
		}
	}
	return;
}




/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
void AddLastYenFromDirectoryPath( char* pszFolder )
{
	if( 3 == strlen( pszFolder )
	 && pszFolder[1] == ':'
	 && pszFolder[2] == '\\'
	){
		/* ドライブ名:\ */
	}else{
		/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
		int	nFolderLen;
		int	nCharChars;
		nFolderLen = strlen( pszFolder );
		if( 0 < nFolderLen ){
			nCharChars = &pszFolder[nFolderLen] - CMemory::MemCharPrev( pszFolder, nFolderLen, &pszFolder[nFolderLen] );
			if( 1 == nCharChars && '\\' == pszFolder[nFolderLen - 1] ){
			}else{
				pszFolder[nFolderLen] = '\\';
				pszFolder[nFolderLen + 1] = '\0';
			}
		}
	}
	return;
}




/* ショートカット(.lnk)の解決 */
BOOL ResolveShortcutLink( HWND hwnd, LPCSTR lpszLinkFile, LPSTR lpszPath )
{
	BOOL			bRes;
	HRESULT			hRes;
	IShellLink*		pIShellLink;
	IPersistFile*	pIPersistFile;
	char			szGotPath[MAX_PATH];
	char			szDescription[MAX_PATH];
	WIN32_FIND_DATA	wfd;
	WCHAR			wsz[MAX_PATH];
	/* 初期化 */
	pIShellLink = NULL;
	pIPersistFile = NULL;
	*lpszPath = 0; // assume failure
	bRes = FALSE;
	szGotPath[0] = '\0';

	// COMライブラリの初期化
	hRes = ::CoInitialize( NULL );
	if( hRes == E_OUTOFMEMORY
	 || hRes == E_INVALIDARG
	 || hRes == E_UNEXPECTED
	){
		return FALSE;
	}

	// Get a pointer to the IShellLink interface.
	hRes = 0;
	if( SUCCEEDED( hRes = ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pIShellLink ) ) ){
		// Get a pointer to the IPersistFile interface.
		if( SUCCEEDED(hRes = pIShellLink->QueryInterface( IID_IPersistFile, (void**)&pIPersistFile ) ) ){
			// Ensure that the string is Unicode.
			MultiByteToWideChar( CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH );
			// Load the shortcut.
			if( SUCCEEDED(hRes = pIPersistFile->Load( wsz, STGM_READ ) ) ){
				// Resolve the link.
				if( SUCCEEDED( hRes = pIShellLink->Resolve(hwnd, SLR_ANY_MATCH ) ) ){
					// Get the path to the link target.
					if( SUCCEEDED( hRes = pIShellLink->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH ) ) ){
						// Get the description of the target.
						if( SUCCEEDED(hRes = pIShellLink->GetDescription(szDescription, MAX_PATH ) ) ){
							if( '\0' != szGotPath[0] ){
								/* 正常終了 */
								lstrcpy( lpszPath, szGotPath );
								bRes = TRUE;
							}
						}
					}
				}
			}
		}
	}
	// Release the pointer to the IPersistFile interface.
	if( NULL != pIPersistFile ){
		pIPersistFile->Release();
		pIPersistFile = NULL;
	}
	// Release the pointer to the IShellLink interface.
	if( NULL != pIShellLink ){
		pIShellLink->Release();
		pIShellLink = NULL;
	}
	// COMライブラリの後始末処理
	::CoUninitialize();
	return bRes;
}




/*
||	処理中のユーザー操作を可能にする
||	ブロッキングフック(?)（メッセージ配送
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
		MSG		msg;
		BOOL	ret;
		ret = (BOOL)::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );
		if( ret ){
			if ( msg.message == WM_QUIT ){
				return FALSE;
			}
			if( NULL != hwndDlgCancel && IsDialogMessage( hwndDlgCancel, &msg ) ){
			}else{
				::TranslateMessage( &msg );
				::DispatchMessage( &msg );
			}
		}
		return TRUE/*ret*/;
}




//! 機能番号に応じてヘルプトピック番号を返す
/*!
	@author Stonee
	@date	2001/02/23
	@param nFuncID 機能番号
	@return ヘルプトピック番号。該当IDが無い場合には0を返す。

	内容はcase文の羅列。

*/
int FuncID_To_HelpContextID( int nFuncID )
{
	switch( nFuncID ){

	/* ファイル操作系 */
	case F_FILENEW:				return 25;			//新規作成
	case F_FILEOPEN:			return 15;			//開く
	case F_FILESAVE:			return 20;			//上書き保存
	case F_FILESAVEAS:			return 21;			//名前を付けて保存
	case F_FILECLOSE:			return 17;			//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	case F_FILECLOSE_OPEN:		return 119;			//閉じて開く
	case F_FILE_REOPEN_SJIS:	return 156;			//SJISで開き直す
	case F_FILE_REOPEN_JIS:		return 157;			//JISで開き直す
	case F_FILE_REOPEN_EUC:		return 158;			//EUCで開き直す
	case F_FILE_REOPEN_UNICODE:	return 159;			//Unicodeで開き直す
	case F_FILE_REOPEN_UTF8:	return 160;			//UTF-8で開き直す
	case F_FILE_REOPEN_UTF7:	return 161;			//UTF-7で開き直す
	case F_PRINT:				return 162;			//印刷				//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	case F_PRINT_PREVIEW:		return 120;			//印刷プレビュー
	case F_PRINT_PAGESETUP:		return 122;			//印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	case F_OPEN_HfromtoC:		return 192;			//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
	case F_OPEN_HHPP:			return 24;			//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	case F_OPEN_CCPP:			return 26;			//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	case F_ACTIVATE_SQLPLUS:	return 132;			/* Oracle SQL*Plusをアクティブ表示 */
	case F_PLSQL_COMPILE_ON_SQLPLUS:	return 27;	/* Oracle SQL*Plusで実行 */
	case F_BROWSE:				return 121;			//ブラウズ
	case F_PROPERTY_FILE:		return 22;			/* ファイルのプロパティ */

//	case IDM_EXITALL:			return 28;			//テキストエディタの全終了
	case F_EXITALL:			return 28;				//サクラエディタの全終了	//Dec. 26, 2000 JEPRO F_に変更


	/* 編集系 */
	case F_UNDO:						return 32;	//元に戻す(Undo)
	case F_REDO:						return 33;	//やり直し(Redo)
	case F_DELETE:						return 41;	//削除
	case F_DELETE_BACK:					return 42;	//カーソル前を削除
	case F_WordDeleteToStart:			return 166;	//単語の左端まで削除
	case F_WordDeleteToEnd:				return 167;	//単語の右端まで削除
	case F_WordCut:						return 169;	//単語切り取り
	case F_WordDelete:					return 168;	//単語削除
	case F_LineCutToStart:				return 172;	//行頭まで切り取り(改行単位)
	case F_LineCutToEnd:				return 173;	//行末まで切り取り(改行単位)
	case F_LineDeleteToStart:			return 170;	//行頭まで削除(改行単位)
	case F_LineDeleteToEnd:				return 171;	//行末まで削除(改行単位)
	case F_CUT_LINE:					return 174;	//行切り取り(折り返し単位)
	case F_DELETE_LINE:					return 137;	//行削除(折り返し単位)
	case F_DUPLICATELINE:				return 43;	//行の二重化(折り返し単位)
	case F_INDENT_TAB:					return 113;	//TABインデント
	case F_UNINDENT_TAB:				return 113;	//逆TABインデント
	case F_INDENT_SPACE:				return 114;	//SPACEインデント
	case F_UNINDENT_SPACE:				return 114;	//逆SPACEインデント
//	case ORDSREFERENCE:					return ;	//単語リファレンス


	/* カーソル移動系 */
//	case F_UP:				return ;	//カーソル上移動
//	case F_DOWN:			return ;	//カーソル下移動
//	case F_LEFT:			return ;	//カーソル左移動
//	case F_RIGHT:			return ;	//カーソル右移動
//	case F_UP2:				return ;	//カーソル上移動(２行ごと)
//	case F_DOWN2:			return ;	//カーソル下移動(２行ごと)
//	case F_WORDLEFT:		return ;	//単語の左端に移動
//	case F_WORDRIGHT:		return ;	//単語の右端に移動
//	case F_GOLINETOP:		return ;	//行頭に移動(折り返し単位)
//	case F_GOLINEEND:		return ;	//行末に移動(折り返し単位)
//	case F_ROLLDOWN:		return ;	//スクロールダウン
//	case F_ROLLUP:			return ;	//スクロールアップ
//	case F_HalfPageUp:		return ;	//半ページアップ	//Oct. 17, 2000 JEPRO 以下「１ページダウン」まで追加
//	case F_HalfPageDown:	return ;	//半ページダウン
//	case F_1PageUp:			return ;	//１ページアップ
//	case F_1PageDown:		return ;	//１ページダウン
//	case F_GOFILETOP:		return ;	//ファイルの先頭に移動
//	case F_GOFILEEND:		return ;	//ファイルの最後に移動
//	case F_CURLINECENTER:	return ;	//カーソル行をウィンドウ中央へ
//	case F_JUMPPREV:		return ;	//移動履歴: 前へ	//Oct. 17, 2000 JEPRO 以下「移動履歴:次へ」まで追加
//	case F_JUMPNEXT:		return ;	//移動履歴: 次へ
	case F_WndScrollDown:	return 198;	//テキストを１行下へスクロール	//Jul. 05, 2001 JEPRO 追加
	case F_WndScrollUp:		return 199;	//テキストを１行上へスクロール	//Jul. 05, 2001 JEPRO 追加


	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動
	case F_SELECTWORD:		return 45;	//現在位置の単語選択
	case F_SELECTALL:		return 44;	//すべて選択
//	case F_BEGIN_SEL:		return ;	//範囲選択開始
//	case F_UP_SEL:			return ;	//(範囲選択)カーソル上移動
//	case F_DOWN_SEL:		return ;	//(範囲選択)カーソル下移動
//	case F_LEFT_SEL:		return ;	//(範囲選択)カーソル左移動
//	case F_RIGHT_SEL:		return ;	//(範囲選択)カーソル右移動
//	case F_UP2_SEL:			return ;	//(範囲選択)カーソル上移動(２行ごと)
//	case F_DOWN2_SEL:		return ;	//(範囲選択)カーソル下移動(２行ごと)
//	case F_WORDLEFT_SEL:	return ;	//(範囲選択)単語の左端に移動
//	case F_WORDRIGHT_SEL:	return ;	//(範囲選択)単語の右端に移動
//	case F_GOLINETOP_SEL:	return ;	//(範囲選択)行頭に移動(折り返し単位)
//	case F_GOLINEEND_SEL:	return ;	//(範囲選択)行末に移動(折り返し単位)
//	case F_ROLLDOWN_SEL:	return ;	//(範囲選択)スクロールダウン
//	case F_ROLLUP_SEL:		return ;	//(範囲選択)スクロールアップ
//	case F_HalfPageUp_Sel:	return ;	//(範囲選択)半ページアップ		//Oct. 17, 2000 JEPRO 以下「１ページダウン」まで追加
//	case F_HalfPageDown_Sel:return ;	//(範囲選択)半ページダウン
//	case F_1PageUp_Sel:		return ;	//(範囲選択)１ページアップ
//	case F_1PageDown_Sel:	return ;	//(範囲選択)１ページダウン
//	case F_GOFILETOP_SEL:	return ;	//(範囲選択)ファイルの先頭に移動
//	case F_GOFILEEND_SEL:	return ;	//(範囲選択)ファイルの最後に移動


	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
//	case F_BOXSELALL:		return ;	//矩形ですべて選択
//	case F_BEGIN_BOX:		return ;	//矩形範囲選択開始
/*
	case F_UP_BOX:			return ;	//(矩形選択)カーソル上移動	//Oct. 17, 2000 JEPRO 以下「ファイルの最後に移動」まで追加
	case F_DOWN_BOX:		return ;	//(矩形選択)カーソル下移動
	case F_LEFT_BOX:		return ;	/(矩形選択)カーソル左移動
	case F_RIGHT_BOX:		return ;	//(矩形選択)カーソル右移動
	case F_UP2_BOX:			return ;	//(矩形選択)カーソル上移動(２行ごと)
	case F_DOWN2_BOX:		return ;	//(矩形選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_BOX:	return ;	//(矩形選択)単語の左端に移動
	case F_WORDRIGHT_BOX:	return ;	//(矩形選択)単語の右端に移動
	case F_GOLINETOP_BOX:	return ;	//(矩形選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_BOX:	return ;	//(矩形選択)行末に移動(折り返し単位)
	case F_HalfPageUp_Box:	return ;	//(矩形選択)半ページアップ
	case F_HalfPageDown_Box:return ;	//(矩形選択)半ページダウン
	case F_1PageUp_Box:		return ;	//(矩形選択)１ページアップ
	case F_1PageDown_Box:	return ;	//(矩形選択)１ページダウン
	case F_GOFILETOP_BOX:	return ;	//(矩形選択)ファイルの先頭に移動
	case F_GOFILEEND_BOX:	return ;	//(矩形選択)ファイルの最後に移動
*/


	/* クリップボード系 */
	case F_CUT:				return 34;			//切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY:			return 35;			//コピー(選択範囲をクリップボードにコピー)
	case F_COPY_CRLF:		return 163;			//CRLF改行でコピー(選択範囲をクリップボードにコピー)	//Feb. 23, 2001 JEPRO 抜けていたので追加
	case F_PASTE:			return 39;			//貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX:		return 40;			//矩形貼り付け(クリップボードから矩形貼り付け)
//	case F_INSTEXT:			return ;			// テキストを貼り付け
	case F_COPYLINES:				return 36;	//選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		return 37;	//選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	return 38;	//選択範囲内全行行番号付きコピー
	case F_COPYPATH:		return 56;			//このファイルのパス名をクリップボードにコピー
	case F_COPYTAG:			return 175;			//このファイルのパス名とカーソル位置をコピー	//Oct. 17, 2000 JEPRO 追加
//	case IDM_TEST_CREATEKEYBINDLIST:	return 57;	//キー割り当て一覧をクリップボードへコピー	//Sept. 15, 2000 jepro「リスト」を「一覧」に変更
	case F_CREATEKEYBINDLIST:		return 57;	//キー割り当て一覧をクリップボードへコピー	//Sept. 15, 2000 JEPRO 「リスト」を「一覧」に変更、IDM＿TESTをFに変更したがうまくいかないので殺してある	//Dec. 25, 2000 復活


	/* 挿入系 */
	case F_INS_DATE:				return 164;	// 日付挿入
	case F_INS_TIME:				return 165;	// 時刻挿入


	/* 変換系 */
	case F_TOLOWER:					return 47;	//英大文字→英小文字
	case F_TOUPPER:					return 48;	//英小文字→英大文字
	case F_TOHANKAKU:				return 49;	/* 全角→半角 */
	case F_TOZENKAKUKATA:			return 50;	/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA:			return 51;	/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKAKUKATA:	return 123;	/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENKAKUHIRA:	return 124;	/* 半角カタカナ→全角ひらがな */
	case F_TOZENEI:					return 125;	/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	case F_TABTOSPACE:				return 182;	/* TAB→空白 */
	case F_SPACETOTAB:				return 196;	/* 空白→TAB */ //#### Stonee, 2001/05/27	//Jul. 03, 2001 JEPRO 番号修正
	case F_CODECNV_AUTO2SJIS:		return 178;	/* 自動判別→SJISコード変換 */
	case F_CODECNV_EMAIL:			return 52;	//E-Mail(JIS→SJIS)コード変換
	case F_CODECNV_EUC2SJIS:		return 53;	//EUC→SJISコード変換
	case F_CODECNV_UNICODE2SJIS:	return 179;	//Unicode→SJISコード変換
	case F_CODECNV_UTF82SJIS:		return 142;	/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS:		return 143; /* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2JIS:		return 117;	/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC:		return 118;	/* SJIS→EUCコード変換 */
	case F_CODECNV_SJIS2UTF8:		return 180;	/* SJIS→UTF-8コード変換 */
	case F_CODECNV_SJIS2UTF7:		return 181;	/* SJIS→UTF-7コード変換 */
	case F_BASE64DECODE:			return 54;	//Base64デコードして保存
	case F_UUDECODE:				return 55;	//uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更


	/* 検索系 */
	case F_SEARCH_DIALOG:		return 59;	//検索(単語検索ダイアログ)
	case F_SEARCH_NEXT:			return 61;	//次を検索
	case F_SEARCH_PREV:			return 60;	//前を検索
	case F_REPLACE:				return 62;	//置換(置換ダイアログ)
	case F_SEARCH_CLEARMARK:	return 136;	//検索マークのクリア
	case F_GREP:				return 67;	//Grep
	case F_JUMP:				return 63;	//指定行ヘジャンプ
	case F_OUTLINE:				return 64;	//アウトライン解析
	case F_TAGJUMP:				return 65;	//タグジャンプ機能
	case F_TAGJUMPBACK:			return 66;	//タグジャンプバック機能
	case F_COMPARE:				return 116;	//ファイル内容比較
	case F_BRACKETPAIR:			return 183;	//対括弧の検索	//Oct. 17, 2000 JEPRO 追加


	/* モード切り替え系 */
	case F_CHGMOD_INS:		return 46;	//挿入／上書きモード切り替え
	case F_CANCEL_MODE:		return 194;	//各種モードの取り消し


	/* 設定系 */
	case F_SHOWTOOLBAR:		return 69;	/* ツールバーの表示 */
	case F_SHOWFUNCKEY:		return 70;	/* ファンクションキーの表示 */
	case F_SHOWSTATUSBAR:	return 134;	/* ステータスバーの表示 */
	case F_TYPE_LIST:		return 72;	/* タイプ別設定一覧 */
	case F_OPTION_TYPE:		return 73;	/* タイプ別設定 */
	case F_OPTION:			return 76;	/* 共通設定 */
//From here 設定ダイアログ用のhelpトピックIDを追加  Stonee, 2001/05/18
	case F_TYPE_SCREEN:		return 74;	/* タイプ別設定『スクリーン』 */
	case F_TYPE_COLOR:		return 75;	/* タイプ別設定『カラー』 */
	case F_TYPE_HELPER:		return 197;	/* タイプ別設定『支援』 */	//Jul. 03, 2001 JEPRO 追加
	case F_OPTION_GENERAL:	return 81;	/* 共通設定『全般』 */
	case F_OPTION_WINDOW:	return 146;	/* 共通設定『ウィンドウ』 */
	case F_OPTION_EDIT:		return 144;	/* 共通設定『編集』 */
	case F_OPTION_FILE:		return 83;	/* 共通設定『ファイル』 */
	case F_OPTION_BACKUP:	return 145;	/* 共通設定『バックアップ』 */
	case F_OPTION_FORMAT:	return 82;	/* 共通設定『書式』 */
	case F_OPTION_URL:		return 147;	/* 共通設定『クリッカブルURL』 */
	case F_OPTION_GREP:		return 148;	/* 共通設定『Grep』 */
	case F_OPTION_KEYBIND:	return 84;	/* 共通設定『キー割り当て』 */
	case F_OPTION_CUSTMENU:	return 87;	/* 共通設定『カスタムメニュー』 */
	case F_OPTION_TOOLBAR:	return 85;	/* 共通設定『ツールバー』 */
	case F_OPTION_KEYWORD:	return 86;	/* 共通設定『強調キーワード』 */
	case F_OPTION_HELPER:	return 88;	/* 共通設定『支援』 */
//To here  Stonee, 2001/05/18
	case F_FONT:			return 71;	/* フォント設定 */
	case F_WRAPWINDOWWIDTH:	return 184;	/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更	//Jul. 03, 2001 JEPRO 番号修正


	/* マクロ */
	case F_RECKEYMACRO:		return 125;	/* キーマクロ記録開始／終了 */
	case F_SAVEKEYMACRO:	return 127;	/* キーマクロ保存 */
	case F_LOADKEYMACRO:	return 128;	/* キーマクロ読み込み */
	case F_EXECKEYMACRO:	return 126;	/* キーマクロ実行 */
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	case F_EXECCMMAND:		return 103; /* 外部コマンド実行 */
	case F_EXECCOMMAND:		return 103; /* 外部コマンド実行 */
//	To Here Sept. 20, 2000


	/* カスタムメニュー */
	case F_MENU_RBUTTON:	return 195	;	/* 右クリックメニュー */
	case F_CUSTMENU_1:	return 186;	/* カスタムメニュー1 */
	case F_CUSTMENU_2:	return 186;	/* カスタムメニュー2 */
	case F_CUSTMENU_3:	return 186;	/* カスタムメニュー3 */
	case F_CUSTMENU_4:	return 186;	/* カスタムメニュー4 */
	case F_CUSTMENU_5:	return 186;	/* カスタムメニュー5 */
	case F_CUSTMENU_6:	return 186;	/* カスタムメニュー6 */
	case F_CUSTMENU_7:	return 186;	/* カスタムメニュー7 */
	case F_CUSTMENU_8:	return 186;	/* カスタムメニュー8 */
	case F_CUSTMENU_9:	return 186;	/* カスタムメニュー9 */
	case F_CUSTMENU_10:	return 186;	/* カスタムメニュー10 */
	case F_CUSTMENU_11:	return 186;	/* カスタムメニュー11 */
	case F_CUSTMENU_12:	return 186;	/* カスタムメニュー12 */
	case F_CUSTMENU_13:	return 186;	/* カスタムメニュー13 */
	case F_CUSTMENU_14:	return 186;	/* カスタムメニュー14 */
	case F_CUSTMENU_15:	return 186;	/* カスタムメニュー15 */
	case F_CUSTMENU_16:	return 186;	/* カスタムメニュー16 */
	case F_CUSTMENU_17:	return 186;	/* カスタムメニュー17 */
	case F_CUSTMENU_18:	return 186;	/* カスタムメニュー18 */
	case F_CUSTMENU_19:	return 186;	/* カスタムメニュー19 */
	case F_CUSTMENU_20:	return 186;	/* カスタムメニュー20 */
	case F_CUSTMENU_21:	return 186;	/* カスタムメニュー21 */
	case F_CUSTMENU_22:	return 186;	/* カスタムメニュー22 */
	case F_CUSTMENU_23:	return 186;	/* カスタムメニュー23 */
	case F_CUSTMENU_24:	return 186;	/* カスタムメニュー24 */


	/* ウィンドウ系 */
	case F_SPLIT_V:			return 93;	//上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	case F_SPLIT_H:			return 94;	//左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	case F_SPLIT_VH:		return 95;	//縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	case F_WINCLOSE:		return 18;	//ウィンドウを閉じる
	case F_WIN_CLOSEALL:	return 19;	//すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	case F_NEXTWINDOW:		return 92;	//次のウィンドウ
	case F_PREVWINDOW:		return 91;	//前のウィンドウ
	case F_CASCADE:			return 138;	//重ねて表示
	case F_TILE_V:			return 140;	//上下に並べて表示
	case F_TILE_H:			return 139;	//左右に並べて表示
	case F_MAXIMIZE_V:		return 141;	//縦方向に最大化
	case F_MAXIMIZE_H:		return 98;	//横方向に最大化	//2001.02.10 by MIK
	case F_MINIMIZE_ALL:	return 96;	//すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	case F_REDRAW:			return 187;	//再描画
	case F_WIN_OUTPUT:		return 188;	//アウトプットウィンドウ表示


	/* 支援 */
	case F_HOKAN:			return 111;	/* 入力補完機能 */
//Sept. 15, 2000→Nov. 25, 2000 JEPRO	//ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
//	case IDM_HELP_CONTENTS:	return 100;	//ヘルプ目次
	case F_HELP_CONTENTS:	return 100;	//ヘルプ目次			//Nov. 25, 2000 JEPRO
//	case IDM_HELP_SEARCH:	return 101;	//ヘルプトピックの検索
	case F_HELP_SEARCH:		return 101;	//ヘルプキーワード検索	//Nov. 25, 2000 JEPRO「トピックの」→「キーワード」に変更
	case F_MENU_ALLFUNC:	return 189;	/* コマンド一覧 */
	case F_EXTHELP1:		return 190;	/* 外部ヘルプ１ */
	case F_EXTHTMLHELP:		return 191;	/* 外部HTMLヘルプ */
//	case IDM_ABOUT:			return 102;	//バージョン情報
	case F_ABOUT:			return 102;	//バージョン情報	//Dec. 24, 2000 JEPRO F_に変更


	/* その他 */
//	case F_SENDMAIL:		return ;	/* メール送信 */

	default:
		return 0;
	}
}

//	From Here Jun. 26, 2001 genta
/*!
	与えられた正規表現ライブラリの初期化を行う．
	メッセージフラグがONで初期化に失敗したときはメッセージを表示する．

	@param hWnd [in] ダイアログボックスのウィンドウハンドル。
			バージョン番号の設定が不要であればNULL。
	@param pRegexp [in] チェックに利用するCBregexpクラスへの参照
	@param bShowMessage [in] 初期化失敗時にエラーメッセージを出すフラグ

	@retval true 初期化成功
	@retval false 初期化に失敗
*/
bool InitRegexp( HWND hWnd, CBregexp& rRegexp, bool bShowMessage )
{
	if( !rRegexp.Init() ){
		if( bShowMessage ){
			::MessageBeep( MB_ICONEXCLAMATION );
			::MessageBox( hWnd, "BREGEXP.DLLが見つかりません。\r\n"
				"正規表現を利用するにはBREGEXP.DLLが必要です。\r\n"
				"入手方法はヘルプを参照してください。",
				"情報", MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}

/*!
	正規表現ライブラリの存在を確認し、あればバージョン情報を指定コンポーネントにセットする。
	失敗した場合には空文字列をセットする。

	@param hWnd [in] ダイアログボックスのウィンドウハンドル。
			バージョン番号の設定が不要であればNULL。
	@param nCmpId [in] バージョン文字列を設定するコンポーネントID
	@param bShowMessage [in] 初期化失敗時にエラーメッセージを出すフラグ

	@retval true バージョン番号の設定に成功
	@retval false 正規表現ライブラリの初期化に失敗
*/
bool CheckRegexpVersion( HWND hWnd, int nCmpId, bool bShowMessage )
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		if( hWnd != NULL ){
			::SetDlgItemText( hWnd, nCmpId, " ");
		}
		return false;
	}
	if( hWnd != NULL ){
		::SetDlgItemText( hWnd, nCmpId, cRegexp.GetVersion() );
	}
	return true;
}

/*!
	正規表現が規則に従っているかをチェックする。

	@param szPattern [in] チェックする正規表現
	@param hWnd [in] メッセージボックスの親ウィンドウ
	@param bShowMessage [in] 初期化失敗時にエラーメッセージを出すフラグ

	@retval true 正規表現は規則通り
	@retval false 文法に誤りがある。または、ライブラリが使用できない。
*/
bool CheckRegexpSyntax( const char* szPattern, HWND hWnd, bool bShowMessage )
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		return false;
	}
	if( !cRegexp.Compile( szPattern ) ){
		if( bShowMessage ){
			::MessageBox( hWnd, cRegexp.GetLastMessage(),
				"正規表現エラー", MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}
//	To Here Jun. 26, 2001 genta


//	From Here Jun. 26, 2001 genta
/*!
	HTML Helpコンポーネントのアクセスを提供する。
	内部で保持すべきデータは特になく、至る所から使われるのでGlobal変数にするが、
	直接のアクセスはOpenHtmlHelp()関数のみから行う。
	他のファイルからはCHtmlHelpクラスは隠されている。
*/
CHtmlHelp g_cHtmlHelp;

/*!
	HTML Helpを開く

	HTML Helpが利用可能であれば引数をそのまま渡し、そうでなければメッセージを表示する。

	@param hWnd [in] 呼び出し元ウィンドウのウィンドウハンドル
	@param szFile [in] HTML Helpのファイル名。
				不等号に続けてウィンドウタイプ名を指定可能。
	@param uCmd [in] HTML Help に渡すコマンド
	@param data [in] コマンドに応じたデータ
	@param msgflag [in] エラーメッセージを表示するか。省略時はtrue。

	@return 開いたヘルプウィンドウのウィンドウハンドル。開けなかったときはNULL。
*/

HWND OpenHtmlHelp( HWND hWnd, LPCSTR szFile, UINT uCmd, DWORD data, bool msgflag )
{
	if( g_cHtmlHelp.Init() ){
		return g_cHtmlHelp.HtmlHelp( hWnd, szFile, uCmd, data );
	}
	if( msgflag ){
		::MessageBox( hWnd, "HHCTRL.OCXが見つかりません。\r\n"
			"HTMLヘルプを利用するにはHHCTRL.OCXが必要です。\r\n",
			"情報", MB_OK | MB_ICONEXCLAMATION );
	}
	return NULL;
}


//	To Here Jun. 26, 2001 genta


/*[EOF]*/
