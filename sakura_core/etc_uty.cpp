//	$Id$
//////////////
// etc_uty.cpp
//	Copyright (C) 1998-2000, Norio Nakatani

//#include <stdio.h>
#include <io.h>
#include "etc_uty.h"
#include "debug.h"
#include "CMemory.h"


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
//		MYTRACE( "BFFM_INITIALIZED (char*)lpData = [%s]\n", (char*)lpData);
		::SendMessage( hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)lpData );
		break;
	case BFFM_SELCHANGED:
//		MYTRACE( "BFFM_SELCHANGED\n");
		break;
	}
	return 0;

}




/* フォルダ選択ダイアログ */
BOOL SelectDir(HWND hWnd, const char* pszTitle, const char* pszInitFolder, char* strFolderName)
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
	ULONG			chEaten;  //文字列のサイズを受け取ります。
	ULONG			dwAttributes;     //属性を受け取ります。
	HRESULT			hRes;
	if( '\0' == pszPath[0] ){
		return NULL;
	}
	if( ::SHGetDesktopFolder( &pDesktopFolder ) != NOERROR ){
		return NULL;
	}
	//　これをしないとインターフェイスはダメなのです。
	::MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszPath, -1, ochPath, MAX_PATH );
	//　実際にITEMIDLISTを取得します。
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
	//　IMallocインターフェイスへのポインタを取得します。
	if( NOERROR != ::SHGetMalloc( &pMalloc ) ){
		return FALSE;
	}
	//　アイテムＩＤリストをリリースします。
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
	else {
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
//-	while(1){
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
//-			if( szPathSrc[1] == ':'	){
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
//-		::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, "作者に教えて欲しいエラー",
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
     　Win16 の時は、GetFreeSystemResources という関数がありました。しかし、Win32 ではあ
     りません。サンクを作るだの DLL を作るだのは難しすぎます。簡単な方法を説明します。
     　お使いの Windows95 の [アクセサリ]-[システムツール] にリソースメータがあるのなら、
     c:\windows\system\rsrc32.dll があるはずです。これは、リソースメータという Win32 アプ
     リが、Win16 の GetFreeSystemResources 関数を呼ぶ為の DLL です。これを使いましょう。
*/
BOOL GetSystemResources(
	int*	pnSystemResources,
	int*	pnUserResources,
	int*	pnGDIResources
)
{
	#define GFSR_SYSTEMRESOURCES 0x0000
	#define GFSR_GDIRESOURCES 0x0001
	#define GFSR_USERRESOURCES 0x0002
	HINSTANCE	hlib;
	int (CALLBACK *GetFreeSystemResources)( int );

	hlib = ::LoadLibrary( "RSRC32.dll" );
	if( (int)hlib > 32 ) {
		GetFreeSystemResources = (int (CALLBACK *)( int ))GetProcAddress(
			hlib,
			"_MyGetFreeSystemResources32@4"
		);
		if( GetFreeSystemResources != NULL ) {
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
	if( GetSystemResources(	&nSystemResources, &nUserResources,	&nGDIResources ) ){
//		MYTRACE( "nSystemResources=%d\n", nSystemResources  );
//		MYTRACE( "nUserResources=%d\n", nUserResources  );
//		MYTRACE( "nGDIResources=%d\n", nGDIResources  );
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
    WORD  wLength;
    WORD  wValueLength;
    WORD  bText;
	WCHAR szKey[16];
	VS_FIXEDFILEINFO Value;
};

/* リソースから製品バージョンの取得 */
void GetAppVersionInfo( 
	HINSTANCE	hInstance , 
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
	OSVERSIONINFO			osvi;
	memset( (void *)&osvi, 0, sizeof( osvi ) );
	osvi.dwOSVersionInfoSize = sizeof( osvi );
	if( ::GetVersionEx( &osvi ) ){
		if( osvi.dwMajorVersion < 4 ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, pszAppName,
				"このアプリケーションを実行するには、\nWindows95以上 または WindowsNT4.0以上のOSが必要です。\nアプリケーションを終了します。"
			);
			return FALSE;
		}
	}else{
		::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION, pszAppName,
			"OSのバージョンが取得できません。\nアプリケーションを終了します。"
		);
		return FALSE;
	}
	return TRUE;
}


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
BOOL IsURL( const char*	pszText, int nTextLen, int* pnUrlLen )
{
	BOOL			bFindURL;
//	int				nTextLen;
	int				nCharChars;
	int				i;
	const char*	ppszURLHEADARR[]={
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
	int	nURLHEADNUM = sizeof( ppszURLHEADARR ) / sizeof( ppszURLHEADARR[0] );
	const char*	pszHeahHead = "mhfgntwp";
	const int nHeadHeadLen = 8;
	int	nURLHeadLen;
//	nTextLen = strlen( pszText );
	bFindURL = FALSE;
	for( i = 0 ;i < nHeadHeadLen; ++i ){
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
	

	while(1){
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







BOOL IsFilePath( const char* pLine, int* pnBgn, int* pnPathLen )
{
	int		i;
	int		j;
	int		nLineLen;
	char	szJumpToFile[1024];
	memset( szJumpToFile, 0, sizeof(szJumpToFile) );

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
			  pLine[i] == '\r'   ||
			  pLine[i] == '\n'   ||
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
	memset( szNumber, 0, sizeof(szNumber) );
	if( i >= nLineLen ){
	}else{
		/* 行位置 改行単位行番号(1起点)の抽出 */
		j = 0;
		for( ; i < nLineLen && j + 1 < sizeof(szNumber); ){
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
			memset( szNumber, 0, sizeof(szNumber) );
			j = 0;
			++i;
			for( ; i < nLineLen && j + 1 < sizeof(szNumber); ){
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
//	const char* GetNextLine( const char* pText, int nTextLen, int* pnLineLen, int* pnBgn, BOOL* pbEOL, BOOL bLFisOK   )
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


/* フォルダの最後が半角かつ'\\'の場合は、取り除く "c:\\"等のルートは取り除かない*/
void CutLastYenFromDirectoryPath( char*	pszFolder )
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
void AddLastYenFromDirectoryPath( char*	pszFolder )
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
BOOL ResolveShortcutLink(HWND hwnd, LPCSTR lpszLinkFile, LPSTR lpszPath)
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
	if( SUCCEEDED(hRes = ::CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&pIShellLink )) ){
		// Get a pointer to the IPersistFile interface.
		if( SUCCEEDED(hRes = pIShellLink->QueryInterface( IID_IPersistFile, (void**)&pIPersistFile )) ){
			// Ensure that the string is Unicode.
			MultiByteToWideChar(CP_ACP, 0, lpszLinkFile, -1, wsz, MAX_PATH );
			// Load the shortcut.
			if( SUCCEEDED(hRes = pIPersistFile->Load(wsz, STGM_READ)) ){
				// Resolve the link.
				if( SUCCEEDED(hRes = pIShellLink->Resolve(hwnd, SLR_ANY_MATCH)) ){
					// Get the path to the link target.
					if( SUCCEEDED(hRes = pIShellLink->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA *)&wfd, SLGP_SHORTPATH )) ){
						// Get the description of the target.
						if( SUCCEEDED(hRes = pIShellLink->GetDescription(szDescription, MAX_PATH)) ){
							if( '\0' != szGotPath[0] ){
								/* 正常終了 */
								lstrcpy(lpszPath, szGotPath);
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
||   処理中のユーザー操作を可能にする
||　ブロッキングフック(?)（メッセージ配送）
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
        MSG msg;
        BOOL ret;
        ret = (BOOL)::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
        if(ret){
                if ( msg.message == WM_QUIT ){
                        return FALSE;
                }
                if( NULL != hwndDlgCancel && IsDialogMessage( hwndDlgCancel, &msg) ){
                }else{
                        ::TranslateMessage(&msg);
                        ::DispatchMessage(&msg);
                }
        }
        return TRUE/*ret*/;
}



/* [EOF] */

