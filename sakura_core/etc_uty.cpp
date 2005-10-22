//	$Id$
/*!	@file
	@brief 共通関数群

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, shoji masami, stonee, MIK, YAZAKI
	Copyright (C) 2002, genta, aroka, hor, MIK, 鬼
	Copyright (C) 2003, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//	Sep. 10, 2005 genta GetLongPathNameのエミュレーション関数の実体生成のため
#define COMPILE_NEWAPIS_STUBS

#include <io.h>
#include <memory.h>		// Apr. 03, 2003 genta
#include "etc_uty.h"
#include "debug.h"
#include "CMemory.h"
#include "funccode.h"	//Stonee, 2001/02/23

#include "WINNETWK.H"	//Stonee, 2001/12/21
#include "sakura.hh"	//YAZAKI, 2001/12/11
#include "CEol.h"// 2002/2/3 aroka
#include "CBregexp.h"// 2002/2/3 aroka
#include "COsVersionInfo.h"
#include "my_icmp.h" // 2002/11/30 Moca 追加

#include "CShareData.h"
#include "CMRU.h"
#include "CMRUFolder.h"
#include "CMultiMonitor.h"	//	2004.05.01 genta
#include "Keycode.h"// novice 2004/10/10

//	CShareDataへ移動
/* 日付をフォーマット */
//const char* MyGetDateFormat( char* pszDest, int nDestLen, int nDateFormatType, const char* pszDateFormat )

/* 時刻をフォーマット */
//const char* MyGetTimeFormat( char* pszDest, int nDestLen, int nTimeFormatType, const char* pszTimeFormat )

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

/*!
	空白を含むファイル名を考慮したトークンの分割
	
	先頭にある連続した区切り文字は無視する．
	
	@param pBuffer [in] 文字列バッファ(終端があること)
	@param nLen [in] 文字列の長さ
	@param pnOffset [in/out] オフセット
	@param pDelimiter [in] 区切り文字
	@return トークン

	@date 2004.02.15 みく 最適化
*/
TCHAR* my_strtok( TCHAR* pBuffer, int nLen, int* pnOffset, const TCHAR* pDelimiter )
{
	int i = *pnOffset;
	TCHAR* p;

	do {
		bool bFlag = false;	//ダブルコーテーションの中か？
		if( i >= nLen ) return NULL;
		p = &pBuffer[i];
		for( ; i < nLen; i++ )
		{
			if( pBuffer[i] == _T('"') ) bFlag = ! bFlag;
			if( ! bFlag )
			{
				if( _tcschr( pDelimiter, pBuffer[i] ) )
				{
					pBuffer[i++] = _T('\0');
					break;
				}
			}
		}
		*pnOffset = i;
	} while( ! *p );	//空のトークンなら次を探す
	return p;
}




/*! ヘルプファイルのフルパスを返す
 
    @param pszHelpFile [in] パスを格納するバッファ
    @param nMaxLen [in] バッファに格納可能な文字数：デフォルト値＝_MAX_PATH
    
    @return パスを格納したバッファのポインタ
 
    @note 実行ファイルと同じ位置の sakura.hlp ファイルを返す。
        パスが UNC のときは _MAX_PATH に収まらない可能性がある。
        
 
    @date 2002/01/19 aroka ；nMaxLen 引数追加
*/
char* GetHelpFilePath( char* pszHelpFile, unsigned int nMaxLen )
{
//	int		i;
	unsigned long	lPathLen;
	char	szHelpFile[_MAX_PATH + 1];
//	int		nCharChars;
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	/* ヘルプファイルのファイルパス */
	lPathLen = ::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szHelpFile, sizeof(szHelpFile)
	);
	if( lPathLen > nMaxLen ){
		*pszHelpFile = '\0';
		return pszHelpFile;
	}

	_splitpath( szHelpFile, szDrive, szDir, NULL, NULL );
	if( strlen(szDrive) + strlen(szDir) + strlen("sakura.hlp") > nMaxLen ){
		*pszHelpFile = '\0';
		return pszHelpFile;
	}
	strcpy( szHelpFile, szDrive );
	strcat( szHelpFile, szDir );
	strcat( szHelpFile, "sakura.hlp" );
	strncpy( pszHelpFile, szHelpFile, nMaxLen );
	return pszHelpFile;
}

/*! ロングファイル名を取得する 

	@param[in] pszFilePathSrc 変換元パス名
	@param[out] pszFilePathDes 結果書き込み先 (長さMAX_PATHの領域が必要)

	@date Oct. 2, 2005 genta GetFilePath APIを使って書き換え
	@date Oct. 4, 2005 genta 相対パスが絶対パスに直されなかった
	@date Oct. 5, 2005 Moca  相対パスを絶対パスに変換するように
*/
BOOL GetLongFileName( const char* pszFilePathSrc, char* pszFilePathDes )
{
	TCHAR* name;
	TCHAR szBuf[_MAX_PATH + 1];
	int len = ::GetFullPathName( pszFilePathSrc, _MAX_PATH, szBuf, &name );
	if( len <= 0 || _MAX_PATH <= len ){
		len = ::GetLongPathName( pszFilePathSrc, pszFilePathDes, _MAX_PATH );
		if( len <= 0 || _MAX_PATH < len ){
			return FALSE;
		}
		return TRUE;
	}
	len = ::GetLongPathName( szBuf, pszFilePathDes, _MAX_PATH );
	if( len <= 0 || _MAX_PATH < len ){
		lstrcpy( pszFilePathDes, szBuf );
	}
	return TRUE;
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

/*! リソースから製品バージョンの取得
	@date 2004.05.13 Moca 一度取得したらキャッシュする
*/
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
	static bool bLoad = false;
	static DWORD dwVersionMS = 0;
	static DWORD dwVersionLS = 0;
	if( hInstance == NULL && bLoad ){
		*pdwProductVersionMS = dwVersionMS;
		*pdwProductVersionLS = dwVersionLS;
		return;
	}
	if( NULL != ( hRSRC = ::FindResource( hInstance, MAKEINTRESOURCE(nVersionResourceID), RT_VERSION ) )
	 && NULL != ( hgRSRC = ::LoadResource( hInstance, hRSRC ) )
	 && NULL != ( pVVIH = (VS_VERSION_INFO_HEAD*)::LockResource( hgRSRC ) )
	){
		*pdwProductVersionMS = pVVIH->Value.dwProductVersionMS;
		*pdwProductVersionLS = pVVIH->Value.dwProductVersionLS;
		dwVersionMS = pVVIH->Value.dwProductVersionMS;
		dwVersionLS = pVVIH->Value.dwProductVersionLS;
	}
	if( hInstance == NULL ){
		bLoad = true;
	}
	return;

}




/* アクティブにする */
void ActivateFrameWindow( HWND hwnd )
{
	if( ::IsIconic( hwnd ) ){
		::ShowWindow( hwnd, SW_RESTORE );
	}
	else if ( ::IsZoomed( hwnd ) ){
		::ShowWindow( hwnd, SW_MAXIMIZE );
	}
	else {
		::ShowWindow( hwnd, SW_SHOW );
	}
	::SetForegroundWindow( hwnd );
	::BringWindowToTop( hwnd );
	return;
}




//@@@ 2002.01.24 Start by MIK
/*!
	文字列がURLかどうかを検査する。
	
	@param pszLine [in] 文字列
	@param nLineLen [in] 文字列の長さ
	@param pnMatchLen [out] URLの長さ
	
	@retval TRUE URLである
	@retval FALSE URLでない
	
	@note 関数内に定義したテーブルは必ず static const 宣言にすること(性能に影響します)。
		url_char の値は url_table の配列番号+1 になっています。
		新しい URL を追加する場合は #define 値を修正してください。
		url_table は頭文字がアルファベット順になるように並べてください。
*/
BOOL IsURL( const char *pszLine, int nLineLen, int *pnMatchLen )
{
	struct _url_table_t {
		char	name[12];
		int		length;
		bool	is_mail;
	};
	static const struct _url_table_t	url_table[] = {
		/* アルファベット順 */
		"file://",		7,	false, /* 1 */
		"ftp://",		6,	false, /* 2 */
		"gopher://",	9,	false, /* 3 */
		"http://",		7,	false, /* 4 */
		"https://",		8,	false, /* 5 */
		"mailto:",		7,	true,  /* 6 */
		"news:",		5,	false, /* 7 */
		"nntp://",		7,	false, /* 8 */
		"prospero://",	11,	false, /* 9 */
		"telnet://",	9,	false, /* 10 */
		"tp://",		5,	false, /* 11 */	//2004.02.02
		"ttp://",		6,	false, /* 12 */	//2004.02.02
		"wais://",		7,	false, /* 13 */
		"{",			0,	false  /* 14 */  /* '{' is 'z'+1 : terminate */
	};

/* テーブルの保守性を高めるための定義 */
	const char urF = 1;
	const char urG = 3;
	const char urH = 4;
	const char urM = 6;
	const char urN = 7;
	const char urP = 9;
	const char urT = 10;
	const char urW = 13;	//2004.02.02

	static const char	url_char[] = {
	  /* +0  +1  +2  +3  +4  +5  +6  +7  +8  +9  +A  +B  +C  +D  +E  +F */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +00: */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +10: */
		  0, -1,  0, -1, -1, -1, -1,  0,  0,  0,  0, -1, -1, -1, -1, -1,	/* +20: " !"#$%&'()*+,-./" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0, -1,	/* +30: "0123456789:;<=>?" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* +40: "@ABCDEFGHIJKLMNO" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0,  0, -1,	/* +50: "PQRSTUVWXYZ[\]^_" */
		  0, -1, -1, -1, -1, -1,urF,urG,urH, -1, -1, -1, -1,urM,urN, -1,	/* +60: "`abcdefghijklmno" */
		urP, -1, -1, -1,urT, -1, -1,urW, -1, -1, -1,  0,  0,  0, -1,  0,	/* +70: "pqrstuvwxyz{|}~ " */
		/* あと128バイト犠牲にすればif文を2箇所削除できる */
		/* 0    : not url char
		 * -1   : url char
		 * other: url head char --> url_table array number + 1
		 */
	};

	const unsigned char	*p = (const unsigned char*)pszLine;
	const struct _url_table_t	*urlp;
	int	i;

	if( *p & 0x80 ) return FALSE;	/* 2バイト文字 */
	if( 0 < url_char[*p] ){	/* URL開始文字 */
		for(urlp = &url_table[url_char[*p]-1]; urlp->name[0] == *p; urlp++){	/* URLテーブルを探索 */
			if( (urlp->length <= nLineLen) && (memcmp(urlp->name, pszLine, urlp->length) == 0) ){	/* URLヘッダは一致した */
				p += urlp->length;	/* URLヘッダ分をスキップする */
				if( urlp->is_mail ){	/* メール専用の解析へ */
					if( IsMailAddress((const char*)p, nLineLen - urlp->length, pnMatchLen) ){
						*pnMatchLen = *pnMatchLen + urlp->length;
						return TRUE;
					}
					return FALSE;
				}
				for(i = urlp->length; i < nLineLen; i++, p++){	/* 通常の解析へ */
					if( (*p & 0x80) || (!(url_char[*p])) ) break;	/* 終端に達した */
				}
				if( i == urlp->length ) return FALSE;	/* URLヘッダだけ */
				*pnMatchLen = i;
				return TRUE;
			}
		}
	}
	return IsMailAddress(pszLine, nLineLen, pnMatchLen);
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




//@@@ 2001.11.07 Start by MIK
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
/*
 * 半角数値
 *   1, 1.2, 1.2.3, .1, 0xabc, 1L, 1F, 1.2f, 0x1L, 0x2F, -.1, -1, 1e2, 1.2e+3, 1.2e-3, -1e0
 *   10進数, 16進数, LF接尾語, 浮動小数点数, 負符号
 *   IPアドレスのドット連結(本当は数値じゃないんだよね)
 */
int IsNumber(const char *buf, int offset, int length)
{
	register const char *p, *q;
	register int i = 0;
	register int d = 0;
	register int f = 0;

	p = &buf[offset];
	q = &buf[length];

	if( *p == '0' )  /* 10進数,Cの16進数 */
	{
		p++; i++;
		if( ( p < q ) && ( *p == 'x' ) )  /* Cの16進数 */
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= '0' && *p <= '9' )
				 || ( *p >= 'A' && *p <= 'F' )
				 || ( *p >= 'a' && *p <= 'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "0x" なら "0" だけが数値 */
			if( i == 2 ) return 1;
			
			/* 接尾語 */
			if( p < q )
			{
				if( *p == 'L' || *p == 'l' || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p >= '0' && *p <= '9' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( *p == '.' )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
						}
					}
					else if( *p == 'E' || *p == 'e' )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == '+' || *(p + 1) == '-' )
							 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == '.' ) return i - 1;  /* 最後が "." なら含めない */
			/* 接尾語 */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == '.' )
		{
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( *p == '.' )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
						}
					}
					else if( *p == 'E' || *p == 'e' )
					{
						if( f == 1 ) break;  /* 指数部に入っている */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == '+' || *(p + 1) == '-' )
							 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= '0' && *(p + 1) <= '9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == '.' ) return i - 1;  /* 最後が "." なら含めない */
			/* 接尾語 */
			if( p < q )
			{
				if( *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == 'E' || *p == 'e' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < '0' || *p > '9' )
				{
					if( ( *p == '+' || *p == '-' ) && ( *(p - 1) == 'E' || *(p - 1) == 'e' ) )
					{
						if( p + 1 < q )
						{
							if( *(p + 1) < '0' || *(p + 1) > '9' )
							{
								/* "0E+", "0E-" */
								break;
							}
						}
						else
						{
							/* "0E-", "0E+" */
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( i == 2 ) return 1;  /* "0E", 0e" なら "0" が数値 */
			/* 接尾語 */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else
		{
			/* "0" だけが数値 */
			/*if( *p == '.' ) return i - 1;*/  /* 最後が "." なら含めない */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
				 || *p == 'F' || *p == 'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
	}

	else if( *p >= '1' && *p <= '9' )  /* 10進数 */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		if( *(p - 1) == '.' ) return i - 1;  /* 最後が "." なら含めない */
		/* 接尾語 */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
			 || *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == '-' )  /* マイナス */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "-", "-." だけなら数値でない */
		//@@@ 2001.11.09 start MIK
		//if( i <= 2 ) return 0;
		//if( *(p - 1)  == '.' ) return i - 1;  /* 最後が "." なら含めない */
		if( i == 1 ) return 0;
		if( *(p - 1) == '.' )
		{
			i--;
			if( i == 1 ) return 0;
			return i;
		}  //@@@ 2001.11.09 end MIK
		/* 接尾語 */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == 'L' || *p == 'l' ))
			 || *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == '.' )  /* 小数点 */
	{
		d++;
		p++; i++;
		while( p < q )
		{
			if( *p < '0' || *p > '9' )
			{
				if( *p == '.' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == '.' ) break;  /* "." が連続なら中断 */
					}
				}
				else if( *p == 'E' || *p == 'e' )
				{
					if( f == 1 ) break;  /* 指数部に入っている */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == '+' || *(p + 1) == '-' )
						 && ( *(p + 2) >= '0' && *(p + 2) <= '9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= '0' && *(p + 1) <= '9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "." だけなら数値でない */
		if( i == 1 ) return 0;
		if( *(p - 1)  == '.' ) return i - 1;  /* 最後が "." なら含めない */
		/* 接尾語 */
		if( p < q )
		{
			if( *p == 'F' || *p == 'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

#if 0
	else if( *p == '&' )  /* VBの16進数 */
	{
		p++; i++;
		if( ( p < q ) && ( *p == 'H' ) )
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= '0' && *p <= '9' )
				 || ( *p >= 'A' && *p <= 'F' )
				 || ( *p >= 'a' && *p <= 'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "&H" だけなら数値でない */
			if( i == 2 ) i = 0;
			return i;
		}

		/* "&" だけなら数値でない */
		return 0;
	}
#endif

	/* 数値ではない */
	return 0;
}
//@@@ 2001.11.07 End by MIK




/**	ファイルの存在チェック

	指定されたパスのファイルが存在するかどうかを確認する。
	
	@param path [in] 調べるパス名
	@param bFileOnly [in] true: ファイルのみ対象 / false: ディレクトリも対象
	
	@retval true  ファイルは存在する
	@retval false ファイルは存在しない
	
	@author genta
	@date 2002.01.04 新規作成
*/
bool IsFileExists(const char* path, bool bFileOnly)
{
	WIN32_FIND_DATA fd;
	::ZeroMemory( &fd, sizeof(fd));

	HANDLE hFind = ::FindFirstFile( path, &fd );
	if( hFind != INVALID_HANDLE_VALUE ){
		::FindClose( hFind );
		if( bFileOnly == false ||
			( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
			return true;
	}
	return false;
}
/**	ファイル名の切り出し

	指定文字列からファイル名と認識される文字列を取り出し、
	先頭Offset及び長さを返す。
	
	@param pLine [in] 探査対象文字列
	@param pnBgn [out] 先頭offset。pLine + *pnBgnがファイル名先頭へのポインタ。
	@param pnPathLen [out] ファイル名の長さ
	@param bFileOnly [in] true: ファイルのみ対象 / false: ディレクトリも対象
	
	@retval true ファイル名発見
	@retval false ファイル名は見つからなかった
	
	@date 2002.01.04 genta ディレクトリを検査対象外にする機能を追加
	@date 2003.01.15 matsumo gccのエラーメッセージ(:区切り)でもファイルを検出可能に
	@date 2004.05.29 genta C:\からファイルCが切り出されるのを防止
	@date 2004.11.13 genta/Moca ファイル名先頭の*?を考慮
	@date 2005.01.10 genta 変数名変更 j -> cur_pos
	@date 2005.01.23 genta 警告抑制のため，gotoをreturnに変更
	
*/
bool IsFilePath( const char* pLine, int* pnBgn, int* pnPathLen, bool bFileOnly )
{
	int		i;
	int		nLineLen;
	char	szJumpToFile[1024];
	memset( szJumpToFile, 0, sizeof( szJumpToFile ) );

	nLineLen = strlen( pLine );

	//先頭の空白を読み飛ばす
	for( i = 0; i < nLineLen; ++i ){
		if( ' ' != pLine[i] &&
			'\t' != pLine[i] &&
			'\"' != pLine[i]
		){
			break;
		}
	}

	//	#include <ファイル名>の考慮
	//	#で始まるときは"または<まで読み飛ばす
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

	//	この時点で既に行末に達していたらファイル名は見つからない
	if( i >= nLineLen ){
		return false;
	}
	*pnBgn = i;
	int cur_pos = 0;
	for( ; i <= nLineLen && cur_pos + 1 < sizeof(szJumpToFile); ++i ){
		if( ( i == nLineLen    ||
			  pLine[i] == ' '  ||
			  pLine[i] == '\t' ||	//@@@ 2002.01.08 YAZAKI タブ文字も。
			  pLine[i] == '('  ||
			  pLine[i] == '\r' ||
			  pLine[i] == '\n' ||
			  pLine[i] == '\0' ||
			  pLine[i] == '>'  ||
			  // May 29, 2004 genta C:\の:はファイル区切りと見なして欲しくない
			  ( cur_pos > 1 && pLine[i] == ':' ) ||   //@@@ 2003/1/15/ matsumo (for gcc)
			  pLine[i] == '"'
			) &&
			0 < strlen( szJumpToFile )
		){
			//	Jan. 04, 2002 genta
			//	ファイル存在確認方法変更
			//if( -1 != _access( (const char *)szJumpToFile, 0 ) )
			if( IsFileExists(szJumpToFile, bFileOnly))
			{
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
//			return false;
//		}
//
//		szJumpToFile[cur_pos] = pLine[i];
//		cur_pos++;
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
			//	Oct. 5, 2002 genta
			//	2004.11.13 Moca/genta 先頭に上の文字がある場合の考慮を追加
			( i == 0 || ( i > 0 && ! _IS_SJIS_1( (unsigned char)pLine[i - 1] ))) ){
			return false;
		}else{
		szJumpToFile[cur_pos] = pLine[i];
		cur_pos++;
		}
	}
//	To Here Sept. 27, 2000
//	if( i >= nLineLen ){
//		return FALSE;
//	}
	//	Jan. 04, 2002 genta
	//	ファイル存在確認方法変更
	if( 0 < strlen( szJumpToFile ) &&
		IsFileExists(szJumpToFile, bFileOnly))
	//	-1 != _access( (const char *)szJumpToFile, 0 ) )
	{
		//	Jan. 04, 2002 genta
		//	あまりに変なコーディングなので移動
		*pnPathLen = strlen( szJumpToFile );
		return true;
	}else{
//#ifdef _DEBUG
//		m_cShareData.TraceOut( "ファイルが存在しない。szJumpToFile=[%s]\n", szJumpToFile );
//#endif
	}

	return false;

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




/*! 指定長以下のテキストに切り分ける

	@param pText [in] 切り分け対象となる文字列へのポインタ
	@param nTextLen [in] 切り分け対象となる文字列全体の長さ
	@param nLimitLen [in] 切り分ける長さ
	@param pnLineLen [out] 実際に取り出された文字列の長さ
	@param pnBgn [i/o] 入力: 切り分け開始位置, 出力: 取り出された文字列の次の位置

	@note 2003.05.25 未使用のようだ
*/
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
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pText, nTextLen, i );
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
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pszData, nDataLength, i );
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


/*!	文字列が指定された文字で終わっていなかった場合には
	末尾にその文字を付加する．

	@param pszPath [i/o]操作する文字列
	@param nMaxLen [in]バッファ長
	@param c [in]追加したい文字
	@retval  0 \が元から付いていた
	@retval  1 \を付加した
	@retval -1 バッファが足りず、\を付加できなかった
	@date 2003.06.24 Moca 新規作成
*/
int AddLastChar( char* pszPath, int nMaxLen, char c ){
	int pos = strlen( pszPath );
	// 何もないときは\を付加
	if( 0 == pos ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[0] = c;
		pszPath[1] = '\0';
		return 1;
	}
	// 最後が\でないときも\を付加(日本語を考慮)
	else if( *::CharPrev( pszPath, &pszPath[pos] ) != c ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[pos] = c;
		pszPath[pos + 1] = '\0';
		return 1;
	}
	return 0;
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
//	hRes = 0;
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




/*!
	処理中のユーザー操作を可能にする
	ブロッキングフック(?)（メッセージ配送

	@date 2003.07.04 genta 一回の呼び出しで複数メッセージを処理するように
*/
BOOL BlockingHook( HWND hwndDlgCancel )
{
		MSG		msg;
		BOOL	ret;
		//	Jun. 04, 2003 genta メッセージをあるだけ処理するように
		while(( ret = (BOOL)::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) != 0 ){
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

	@par history
	2001.12.22 YAZAKI sakura.hhを参照するように変更
*/
int FuncID_To_HelpContextID( int nFuncID )
{
	switch( nFuncID ){

	/* ファイル操作系 */
	case F_FILENEW:				return HLP000025;			//新規作成
	case F_FILEOPEN:			return HLP000015;			//開く
	case F_FILEOPEN_DROPDOWN:	return HLP000015;			//開く(ドロップダウン)	//@@@ 2002.06.15 MIK
	case F_FILESAVE:			return HLP000020;			//上書き保存
	case F_FILESAVEAS_DIALOG:	return HLP000021;			//名前を付けて保存
	case F_FILECLOSE:			return HLP000017;			//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	case F_FILECLOSE_OPEN:		return HLP000119;			//閉じて開く
	case F_FILE_REOPEN:			return HLP000283;			//開き直す	//@@@ 2003.06.15 MIK
	case F_FILE_REOPEN_SJIS:	return HLP000156;			//SJISで開き直す
	case F_FILE_REOPEN_JIS:		return HLP000157;			//JISで開き直す
	case F_FILE_REOPEN_EUC:		return HLP000158;			//EUCで開き直す
	case F_FILE_REOPEN_UNICODE:	return HLP000159;			//Unicodeで開き直す
	case F_FILE_REOPEN_UNICODEBE:	return HLP000256;		//UnicodeBEで開き直す
	case F_FILE_REOPEN_UTF8:	return HLP000160;			//UTF-8で開き直す
	case F_FILE_REOPEN_UTF7:	return HLP000161;			//UTF-7で開き直す
	case F_PRINT:				return HLP000162;			//印刷				//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	case F_PRINT_PREVIEW:		return HLP000120;			//印刷プレビュー
	case F_PRINT_PAGESETUP:		return HLP000122;			//印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	case F_OPEN_HfromtoC:		return HLP000192;			//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
	case F_OPEN_HHPP:			return HLP000024;			//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	case F_OPEN_CCPP:			return HLP000026;			//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	case F_ACTIVATE_SQLPLUS:	return HLP000132;			/* Oracle SQL*Plusをアクティブ表示 */
	case F_PLSQL_COMPILE_ON_SQLPLUS:	return HLP000027;	/* Oracle SQL*Plusで実行 */
	case F_BROWSE:				return HLP000121;			//ブラウズ
	case F_READONLY:			return HLP000249;			//読み取り専用
	case F_PROPERTY_FILE:		return HLP000022;			/* ファイルのプロパティ */

//	case IDM_EXITALL:			return 28;			//テキストエディタの全終了
	case F_EXITALL:			return HLP000028;				//サクラエディタの全終了	//Dec. 26, 2000 JEPRO F_に変更


	/* 編集系 */
	case F_UNDO:						return HLP000032;	//元に戻す(Undo)
	case F_REDO:						return HLP000033;	//やり直し(Redo)
	case F_DELETE:						return HLP000041;	//削除
	case F_DELETE_BACK:					return HLP000042;	//カーソル前を削除
	case F_WordDeleteToStart:			return HLP000166;	//単語の左端まで削除
	case F_WordDeleteToEnd:				return HLP000167;	//単語の右端まで削除
	case F_WordCut:						return HLP000169;	//単語切り取り
	case F_WordDelete:					return HLP000168;	//単語削除
	case F_LineCutToStart:				return HLP000172;	//行頭まで切り取り(改行単位)
	case F_LineCutToEnd:				return HLP000173;	//行末まで切り取り(改行単位)
	case F_LineDeleteToStart:			return HLP000170;	//行頭まで削除(改行単位)
	case F_LineDeleteToEnd:				return HLP000171;	//行末まで削除(改行単位)
	case F_CUT_LINE:					return HLP000174;	//行切り取り(折り返し単位)
	case F_DELETE_LINE:					return HLP000137;	//行削除(折り返し単位)
	case F_DUPLICATELINE:				return HLP000043;	//行の二重化(折り返し単位)
	case F_INDENT_TAB:					return HLP000113;	//TABインデント
	case F_UNINDENT_TAB:				return HLP000113;	//逆TABインデント
	case F_INDENT_SPACE:				return HLP000114;	//SPACEインデント
	case F_UNINDENT_SPACE:				return HLP000114;	//逆SPACEインデント
	case F_RECONVERT:					return HLP000218;	//再変換
//	case ORDSREFERENCE:					return ;	//単語リファレンス


	/* カーソル移動系 */
//	case F_UP:				return ;	//カーソル上移動
//	case F_DOWN:			return ;	//カーソル下移動
//	case F_LEFT:			return ;	//カーソル左移動
//	case F_RIGHT:			return ;	//カーソル右移動
	case F_UP2:				return HLP000220;	//カーソル上移動(２行ごと)
	case F_DOWN2:			return HLP000221;	//カーソル下移動(２行ごと)
	case F_WORDLEFT:		return HLP000222;	//単語の左端に移動
	case F_WORDRIGHT:		return HLP000223;	//単語の右端に移動
	case F_GOLINETOP:		return HLP000224;	//行頭に移動(折り返し単位)
	case F_GOLINEEND:		return HLP000225;	//行末に移動(折り返し単位)
//	case F_ROLLDOWN:		return ;	//スクロールダウン
//	case F_ROLLUP:			return ;	//スクロールアップ
	case F_HalfPageUp:		return HLP000245;	//半ページアップ	//Oct. 17, 2000 JEPRO 以下「１ページダウン」まで追加
	case F_HalfPageDown:	return HLP000246;	//半ページダウン
	case F_1PageUp:			return HLP000226;	//１ページアップ
	case F_1PageDown:		return HLP000227;	//１ページダウン
	case F_GOFILETOP:		return HLP000228;	//ファイルの先頭に移動
	case F_GOFILEEND:		return HLP000229;	//ファイルの最後に移動
	case F_CURLINECENTER:	return HLP000230;	//カーソル行をウィンドウ中央へ
	case F_JUMP_SRCHSTARTPOS:	return HLP000264; //検索開始位置へ戻る
	case F_JUMPHIST_PREV:		return HLP000231;	//移動履歴: 前へ	//Oct. 17, 2000 JEPRO 以下「移動履歴:次へ」まで追加
	case F_JUMPHIST_NEXT:		return HLP000232;	//移動履歴: 次へ
	case F_JUMPHIST_SET:	return HLP000265;	//現在位置を移動履歴に登録
	case F_WndScrollDown:	return HLP000198;	//テキストを１行下へスクロール	//Jul. 05, 2001 JEPRO 追加
	case F_WndScrollUp:		return HLP000199;	//テキストを１行上へスクロール	//Jul. 05, 2001 JEPRO 追加
	case F_GONEXTPARAGRAPH:	return HLP000262;	//前の段落に移動
	case F_GOPREVPARAGRAPH:	return HLP000263;	//前の段落に移動

	/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動
	case F_SELECTWORD:		return HLP000045;	//現在位置の単語選択
	case F_SELECTALL:		return HLP000044;	//すべて選択
	case F_BEGIN_SEL:		return HLP000233;	//範囲選択開始
//	case F_UP_SEL:			return ;	//(範囲選択)カーソル上移動
//	case F_DOWN_SEL:		return ;	//(範囲選択)カーソル下移動
//	case F_LEFT_SEL:		return ;	//(範囲選択)カーソル左移動
//	case F_RIGHT_SEL:		return ;	//(範囲選択)カーソル右移動
	case F_UP2_SEL:			return HLP000234;	//(範囲選択)カーソル上移動(２行ごと)
	case F_DOWN2_SEL:		return HLP000235;	//(範囲選択)カーソル下移動(２行ごと)
	case F_WORDLEFT_SEL:	return HLP000236;	//(範囲選択)単語の左端に移動
	case F_WORDRIGHT_SEL:	return HLP000237;	//(範囲選択)単語の右端に移動
	case F_GONEXTPARAGRAPH_SEL:	return HLP000273;	//(範囲選択)前の段落に移動	//@@@ 2003.06.15 MIK
	case F_GOPREVPARAGRAPH_SEL:	return HLP000274;	//(範囲選択)前の段落に移動	//@@@ 2003.06.15 MIK
	case F_GOLINETOP_SEL:	return HLP000238;	//(範囲選択)行頭に移動(折り返し単位)
	case F_GOLINEEND_SEL:	return HLP000239;	//(範囲選択)行末に移動(折り返し単位)
//	case F_ROLLDOWN_SEL:	return ;	//(範囲選択)スクロールダウン
//	case F_ROLLUP_SEL:		return ;	//(範囲選択)スクロールアップ
	case F_HalfPageUp_Sel:	return HLP000247;	//(範囲選択)半ページアップ		//Oct. 17, 2000 JEPRO 以下「１ページダウン」まで追加
	case F_HalfPageDown_Sel:return HLP000248;	//(範囲選択)半ページダウン
	case F_1PageUp_Sel:		return HLP000240;	//(範囲選択)１ページアップ
	case F_1PageDown_Sel:	return HLP000241;	//(範囲選択)１ページダウン
	case F_GOFILETOP_SEL:	return HLP000242;	//(範囲選択)ファイルの先頭に移動
	case F_GOFILEEND_SEL:	return HLP000243;	//(範囲選択)ファイルの最後に移動
//	case F_GONEXTPARAGRAPH_SEL:	return HLP000262;	//(範囲選択)次の段落へ
//	case F_GOPREVPARAGRAPH_SEL:	return HLP000263;	//(範囲選択)前の段落へ


	/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
//	case F_BOXSELALL:		return ;	//矩形ですべて選択
	case F_BEGIN_BOX:		return HLP000244;	//矩形範囲選択開始
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
	/* 整形系 2002/04/17 YAZAKI */
	case F_LTRIM:		return HLP000210;	//左(先頭)の空白を削除
	case F_RTRIM:		return HLP000211;	//右(末尾)の空白を削除
	case F_SORT_ASC:	return HLP000212;	//選択行の昇順ソート
	case F_SORT_DESC:	return HLP000213;	//選択行の降順ソート
	case F_MERGE:		return HLP000214;	//選択行のマージ

	/* クリップボード系 */
	case F_CUT:				return HLP000034;			//切り取り(選択範囲をクリップボードにコピーして削除)
	case F_COPY:			return HLP000035;			//コピー(選択範囲をクリップボードにコピー)
	case F_COPY_ADDCRLF:	return HLP000219;			//折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	case F_COPY_CRLF:		return HLP000163;			//CRLF改行でコピー(選択範囲をクリップボードにコピー)	//Feb. 23, 2001 JEPRO 抜けていたので追加
	case F_PASTE:			return HLP000039;			//貼り付け(クリップボードから貼り付け)
	case F_PASTEBOX:		return HLP000040;			//矩形貼り付け(クリップボードから矩形貼り付け)
//	case F_INSTEXT:			return ;			// テキストを貼り付け
	case F_COPYLINES:				return HLP000036;	//選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		return HLP000037;	//選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	return HLP000038;	//選択範囲内全行行番号付きコピー
	case F_COPYPATH:		return HLP000056;			//このファイルのパス名をクリップボードにコピー
	case F_COPYTAG:			return HLP000175;			//このファイルのパス名とカーソル位置をコピー	//Oct. 17, 2000 JEPRO 追加
	case F_COPYFNAME:		return HLP000303;			//このファイル名をクリップボードにコピー // 2002/2/3 aroka
//	case IDM_TEST_CREATEKEYBINDLIST:	return 57;	//キー割り当て一覧をクリップボードへコピー	//Sept. 15, 2000 jepro「リスト」を「一覧」に変更
	case F_CREATEKEYBINDLIST:		return HLP000057;	//キー割り当て一覧をクリップボードへコピー	//Sept. 15, 2000 JEPRO 「リスト」を「一覧」に変更、IDM＿TESTをFに変更したがうまくいかないので殺してある	//Dec. 25, 2000 復活


	/* 挿入系 */
	case F_INS_DATE:				return HLP000164;	// 日付挿入
	case F_INS_TIME:				return HLP000165;	// 時刻挿入
	case F_CTRL_CODE_DIALOG:		return HLP000255;	/* コントロールコード入力 */


	/* 変換系 */
	case F_TOLOWER:					return HLP000047;	//英大文字→英小文字
	case F_TOUPPER:					return HLP000048;	//英小文字→英大文字
	case F_TOHANKAKU:				return HLP000049;	/* 全角→半角 */
	case F_TOHANKATA:				return HLP000258;	//全角カタカナ→半角カタカナ
	case F_TOZENKAKUKATA:			return HLP000050;	/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA:			return HLP000051;	/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKAKUKATA:	return HLP000123;	/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENKAKUHIRA:	return HLP000124;	/* 半角カタカナ→全角ひらがな */
	case F_TOZENEI:					return HLP000200;	/* 半角英数→全角英数 */			//July. 30, 2001 Misaka //Stonee, 2001/09/26 番号修正
	case F_TOHANEI:					return HLP000215;	/* 全角英数→半角英数 */			//@@@ 2002.2.11 YAZAKI
	case F_TABTOSPACE:				return HLP000182;	/* TAB→空白 */
	case F_SPACETOTAB:				return HLP000196;	/* 空白→TAB */	//#### Stonee, 2001/05/27	//Jul. 03, 2001 JEPRO 番号修正
	case F_CODECNV_AUTO2SJIS:		return HLP000178;	/* 自動判別→SJISコード変換 */
	case F_CODECNV_EMAIL:			return HLP000052;	//E-Mail(JIS→SJIS)コード変換
	case F_CODECNV_EUC2SJIS:		return HLP000053;	//EUC→SJISコード変換
	case F_CODECNV_UNICODE2SJIS:	return HLP000179;	//Unicode→SJISコード変換
	case F_CODECNV_UNICODEBE2SJIS:	return HLP000257;	//UnicodeBE→SJISコード変換
	case F_CODECNV_UTF82SJIS:		return HLP000142;	/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS:		return HLP000143; /* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2JIS:		return HLP000117;	/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC:		return HLP000118;	/* SJIS→EUCコード変換 */
	case F_CODECNV_SJIS2UTF8:		return HLP000180;	/* SJIS→UTF-8コード変換 */
	case F_CODECNV_SJIS2UTF7:		return HLP000181;	/* SJIS→UTF-7コード変換 */
	case F_BASE64DECODE:			return HLP000054;	//Base64デコードして保存
	case F_UUDECODE:				return HLP000055;	//uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更


	/* 検索系 */
	case F_SEARCH_DIALOG:		return HLP000059;	//検索(単語検索ダイアログ)
	case F_SEARCH_BOX:			return HLP000059;	//検索(ボックス) Jan. 13, 2003 MIK
	case F_SEARCH_NEXT:			return HLP000061;	//次を検索
	case F_SEARCH_PREV:			return HLP000060;	//前を検索
	case F_REPLACE_DIALOG:		return HLP000062;	//置換(置換ダイアログ)
	case F_SEARCH_CLEARMARK:	return HLP000136;	//検索マークのクリア
	case F_GREP_DIALOG:			return HLP000067;	//Grep
	case F_JUMP_DIALOG:			return HLP000063;	//指定行へジャンプ
	case F_OUTLINE:				return HLP000064;	//アウトライン解析
	case F_TAGJUMP:				return HLP000065;	//タグジャンプ機能
	case F_TAGJUMPBACK:			return HLP000066;	//タグジャンプバック機能
	case F_TAGS_MAKE:			return HLP000280;	//タグファイルの作成	//@@@ 2003.04.13 MIK
	case F_TAGJUMP_LIST:		return HLP000281;	//タグジャンプ一覧	//@@@ 2003.04.17 MIK
	case F_DIRECT_TAGJUMP:		return HLP000281;	//ダイレクトタグジャンプ	//@@@ 2003.04.17 MIK
	case F_COMPARE:				return HLP000116;	//ファイル内容比較
	case F_DIFF_DIALOG:			return HLP000251;	//DIFF差分表示(ダイアログ)	//@@@ 2002.05.25 MIK
//	case F_DIFF:				return HLP000251;	//DIFF差分表示	//@@@ 2002.05.25 MIK
	case F_DIFF_NEXT:			return HLP000252;	//次の差分へ	//@@@ 2002.05.25 MIK
	case F_DIFF_PREV:			return HLP000253;	//前の差分へ	//@@@ 2002.05.25 MIK
	case F_DIFF_RESET:			return HLP000254;	//差分の全解除	//@@@ 2002.05.25 MIK
	case F_BRACKETPAIR:			return HLP000183;	//対括弧の検索	//Oct. 17, 2000 JEPRO 追加
	case F_BOOKMARK_SET:		return HLP000205;	//ブックマーク設定・解除
	case F_BOOKMARK_NEXT:		return HLP000206;	//次のブックマークへ
	case F_BOOKMARK_PREV:		return HLP000207;	//前のブックマークへ
	case F_BOOKMARK_RESET:		return HLP000208;	//ブックマークの全解除
	case F_BOOKMARK_VIEW:		return HLP000209;	//ブックマークの一覧

	/* モード切り替え系 */
	case F_CHGMOD_INS:		return HLP000046;	//挿入／上書きモード切り替え
	case F_CHGMOD_EOL_CRLF:	return HLP000285;	//入力改行コード指定	// 2003.09.23 Moca
	case F_CHGMOD_EOL_CR:	return HLP000285;	//入力改行コード指定	// 2003.09.23 Moca
	case F_CHGMOD_EOL_LF:	return HLP000285;	//入力改行コード指定	// 2003.09.23 Moca
	case F_CANCEL_MODE:		return HLP000194;	//各種モードの取り消し


	/* 設定系 */
	case F_SHOWTOOLBAR:		return HLP000069;	/* ツールバーの表示 */
	case F_SHOWFUNCKEY:		return HLP000070;	/* ファンクションキーの表示 */
	case F_SHOWTAB:			return HLP000282;	/* タブの表示 */	//@@@ 2003.06.10 MIK
	case F_SHOWSTATUSBAR:	return HLP000134;	/* ステータスバーの表示 */
	case F_TYPE_LIST:		return HLP000072;	/* タイプ別設定一覧 */
	case F_OPTION_TYPE:		return HLP000073;	/* タイプ別設定 */
	case F_OPTION:			return HLP000076;	/* 共通設定 */
//From here 設定ダイアログ用のhelpトピックIDを追加  Stonee, 2001/05/18
	case F_TYPE_SCREEN:		return HLP000074;	/* タイプ別設定『スクリーン』 */
	case F_TYPE_COLOR:		return HLP000075;	/* タイプ別設定『カラー』 */
	case F_TYPE_HELPER:		return HLP000197;	/* タイプ別設定『支援』 */	//Jul. 03, 2001 JEPRO 追加
	case F_TYPE_REGEX_KEYWORD:	return HLP000203;	/* タイプ別設定『正規表現キーワード』 */	//@@@ 2001.11.17 add MIK
	case F_OPTION_GENERAL:	return HLP000081;	/* 共通設定『全般』 */
	case F_OPTION_WINDOW:	return HLP000146;	/* 共通設定『ウィンドウ』 */
	case F_OPTION_EDIT:		return HLP000144;	/* 共通設定『編集』 */
	case F_OPTION_FILE:		return HLP000083;	/* 共通設定『ファイル』 */
	case F_OPTION_BACKUP:	return HLP000145;	/* 共通設定『バックアップ』 */
	case F_OPTION_FORMAT:	return HLP000082;	/* 共通設定『書式』 */
	case F_OPTION_URL:		return HLP000147;	/* 共通設定『クリッカブルURL』 */
	case F_OPTION_GREP:		return HLP000148;	/* 共通設定『Grep』 */
	case F_OPTION_KEYBIND:	return HLP000084;	/* 共通設定『キー割り当て』 */
	case F_OPTION_CUSTMENU:	return HLP000087;	/* 共通設定『カスタムメニュー』 */
	case F_OPTION_TOOLBAR:	return HLP000085;	/* 共通設定『ツールバー』 */
	case F_OPTION_KEYWORD:	return HLP000086;	/* 共通設定『強調キーワード』 */
	case F_OPTION_HELPER:	return HLP000088;	/* 共通設定『支援』 */
//To here  Stonee, 2001/05/18
	case F_OPTION_MACRO:	return HLP000201;	/* 共通設定『マクロ』 */	//@@@ 2002.01.02
	case F_OPTION_FNAME:	return HLP000277;	/* 共通設定 『全般』プロパティ */	// 2002.12.09 Moca Add	//重複回避
	case F_FONT:			return HLP000071;	/* フォント設定 */
	case F_WRAPWINDOWWIDTH:	return HLP000184;	/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更	//Jul. 03, 2001 JEPRO 番号修正
	case F_FAVORITE:		return HLP000279;	/* お気に入りの設定 */	//@@@ 2003.04.08 MIK

	/* マクロ */
	case F_RECKEYMACRO:		return HLP000125;	/* キーマクロ記録開始／終了 */
	case F_SAVEKEYMACRO:	return HLP000127;	/* キーマクロ保存 */
	case F_LOADKEYMACRO:	return HLP000128;	/* キーマクロ読み込み */
	case F_EXECKEYMACRO:	return HLP000126;	/* キーマクロ実行 */
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	case F_EXECCMMAND:		return 103; /* 外部コマンド実行 */
	case F_EXECCOMMAND_DIALOG:	return HLP000103; /* 外部コマンド実行 */
//	To Here Sept. 20, 2000


	/* カスタムメニュー */
	case F_MENU_RBUTTON:	return HLP000195;	/* 右クリックメニュー */
	case F_CUSTMENU_1:	return HLP000186;	/* カスタムメニュー1 */
	case F_CUSTMENU_2:	return HLP000186;	/* カスタムメニュー2 */
	case F_CUSTMENU_3:	return HLP000186;	/* カスタムメニュー3 */
	case F_CUSTMENU_4:	return HLP000186;	/* カスタムメニュー4 */
	case F_CUSTMENU_5:	return HLP000186;	/* カスタムメニュー5 */
	case F_CUSTMENU_6:	return HLP000186;	/* カスタムメニュー6 */
	case F_CUSTMENU_7:	return HLP000186;	/* カスタムメニュー7 */
	case F_CUSTMENU_8:	return HLP000186;	/* カスタムメニュー8 */
	case F_CUSTMENU_9:	return HLP000186;	/* カスタムメニュー9 */
	case F_CUSTMENU_10:	return HLP000186;	/* カスタムメニュー10 */
	case F_CUSTMENU_11:	return HLP000186;	/* カスタムメニュー11 */
	case F_CUSTMENU_12:	return HLP000186;	/* カスタムメニュー12 */
	case F_CUSTMENU_13:	return HLP000186;	/* カスタムメニュー13 */
	case F_CUSTMENU_14:	return HLP000186;	/* カスタムメニュー14 */
	case F_CUSTMENU_15:	return HLP000186;	/* カスタムメニュー15 */
	case F_CUSTMENU_16:	return HLP000186;	/* カスタムメニュー16 */
	case F_CUSTMENU_17:	return HLP000186;	/* カスタムメニュー17 */
	case F_CUSTMENU_18:	return HLP000186;	/* カスタムメニュー18 */
	case F_CUSTMENU_19:	return HLP000186;	/* カスタムメニュー19 */
	case F_CUSTMENU_20:	return HLP000186;	/* カスタムメニュー20 */
	case F_CUSTMENU_21:	return HLP000186;	/* カスタムメニュー21 */
	case F_CUSTMENU_22:	return HLP000186;	/* カスタムメニュー22 */
	case F_CUSTMENU_23:	return HLP000186;	/* カスタムメニュー23 */
	case F_CUSTMENU_24:	return HLP000186;	/* カスタムメニュー24 */


	/* ウィンドウ系 */
	case F_SPLIT_V:			return HLP000093;	//上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	case F_SPLIT_H:			return HLP000094;	//左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	case F_SPLIT_VH:		return HLP000095;	//縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	case F_WINCLOSE:		return HLP000018;	//ウィンドウを閉じる
	case F_WIN_CLOSEALL:	return HLP000019;	//すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	case F_NEXTWINDOW:		return HLP000092;	//次のウィンドウ
	case F_PREVWINDOW:		return HLP000091;	//前のウィンドウ
	case F_CASCADE:			return HLP000138;	//重ねて表示
	case F_TILE_V:			return HLP000140;	//上下に並べて表示
	case F_TILE_H:			return HLP000139;	//左右に並べて表示
	case F_MAXIMIZE_V:		return HLP000141;	//縦方向に最大化
	case F_MAXIMIZE_H:		return HLP000098;	//横方向に最大化	//2001.02.10 by MIK
	case F_MINIMIZE_ALL:	return HLP000096;	//すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	case F_REDRAW:			return HLP000187;	//再描画
	case F_WIN_OUTPUT:		return HLP000188;	//アウトプットウィンドウ表示


	/* 支援 */
	case F_HOKAN:			return HLP000111;	/* 入力補完機能 */
//Sept. 15, 2000→Nov. 25, 2000 JEPRO	//ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
	case F_HELP_CONTENTS:	return HLP000100;	//ヘルプ目次			//Nov. 25, 2000 JEPRO
	case F_HELP_SEARCH:		return HLP000101;	//ヘルプキーワード検索	//Nov. 25, 2000 JEPRO「トピックの」→「キーワード」に変更
	case F_MENU_ALLFUNC:	return HLP000189;	/* コマンド一覧 */
	case F_EXTHELP1:		return HLP000190;	/* 外部ヘルプ１ */
	case F_EXTHTMLHELP:		return HLP000191;	/* 外部HTMLヘルプ */
	case F_ABOUT:			return HLP000102;	//バージョン情報	//Dec. 24, 2000 JEPRO F_に変更


	/* その他 */
//	case F_SENDMAIL:		return ;	/* メール送信 */

	default:
		// From Here 2003.09.23 Moca
		if( IDM_SELMRU <= nFuncID && nFuncID < IDM_SELMRU + MAX_MRU ){
			return HLP000029;	//最近使ったファイル
		}else if( IDM_SELOPENFOLDER <= nFuncID && nFuncID < IDM_SELOPENFOLDER + MAX_OPENFOLDER ){
			return HLP000023;	//最近使ったフォルダ
		}else if( IDM_SELWINDOW <= nFuncID && nFuncID < IDM_SELWINDOW + MAX_EDITWINDOWS ){
			return HLP000097;	//ウィンドウリスト
		}
		// To Here 2003.09.23 Moca
		return 0;
	}
}

/*!クリープボードにText形式でコピーする
	@param hwnd [in] クリップボードのオーナー
	@param pszText [in] 設定するテキスト
	@param length [in] 有効なテキストの長さ
	
	@retval true コピー成功
	@retval false コピー失敗。場合によってはクリップボードに元の内容が残る
	@date 2004.02.17 Moca 各所のソースを統合
*/
SAKURA_CORE_API bool SetClipboardText( HWND hwnd, const char* pszText, int length )
{
	HGLOBAL		hgClip;
	char*		pszClip;

	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, length + 1 );
	if( NULL == hgClip ){
		return false;
	}
	pszClip = (char*)::GlobalLock( hgClip );
	if( NULL == pszClip ){
		::GlobalFree( hgClip );
		return false;
	}
	memcpy( pszClip, pszText, length );
	pszClip[length] = 0;
	::GlobalUnlock( hgClip );
	if( FALSE == ::OpenClipboard( hwnd ) ){
		::GlobalFree( hgClip );
		return false;
	}
	::EmptyClipboard();
	::SetClipboardData( CF_OEMTEXT, hgClip );
	::CloseClipboard();

	return true;
}

//	From Here Jun. 26, 2001 genta
/*!
	与えられた正規表現ライブラリの初期化を行う．
	メッセージフラグがONで初期化に失敗したときはメッセージを表示する．

	@param hWnd [in] ダイアログボックスのウィンドウハンドル。
			バージョン番号の設定が不要であればNULL。
	@param rRegexp [in] チェックに利用するCBregexpクラスへの参照
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
	@param nOption [in] 大文字と小文字を無視して比較するフラグ // 2002/2/1 hor追加

	@retval true 正規表現は規則通り
	@retval false 文法に誤りがある。または、ライブラリが使用できない。
*/
bool CheckRegexpSyntax( const char* szPattern, HWND hWnd, bool bShowMessage, int nOption )
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		return false;
	}
	if( !cRegexp.Compile( szPattern, nOption ) ){	// 2002/2/1 hor追加
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

HWND OpenHtmlHelp( HWND hWnd, LPCSTR szFile, UINT uCmd, DWORD_PTR data, bool msgflag )
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



// Stonee, 2001/12/21
// NetWork上のリソースに接続するためのダイアログを出現させる
// NO_ERROR:成功 ERROR_CANCELLED:キャンセル それ以外:失敗
// プロジェクトの設定でリンクモジュールにMpr.libを追加のこと
DWORD NetConnect ( const char strNetWorkPass[] )
{
	NETRESOURCE nr;
	//char sPassWord[] = "\0";	//パスワード
	//char sUser[] = "\0";		//ユーザー名
	DWORD dwRet;				//戻り値　エラーコードはWINERROR.Hを参照
	char sTemp[256];
	char sDrive[] = "";
    int i;

	if (strlen(strNetWorkPass) < 3)	return ERROR_BAD_NET_NAME;  //UNCではない。
	if (strNetWorkPass[0] != '\\' && strNetWorkPass[1] != '\\')	return ERROR_BAD_NET_NAME;  //UNCではない。

	//3文字目から数えて最初の\の直前までを切り出す
	sTemp[0] = '\\';
	sTemp[1] = '\\';
	for (i = 2; strNetWorkPass[i] != '\0'; i++) {
		if (strNetWorkPass[i] == '\\') break;
		sTemp[i] = strNetWorkPass[i];
	}
	sTemp[i] = '\0';	//終端

/*
	MYMESSAGEBOX(
		0,
		MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
		"！！",
		sTemp
	);
*/
	ZeroMemory( &nr, sizeof( nr ) );
	nr.dwScope = RESOURCE_GLOBALNET;
	nr.dwType = RESOURCETYPE_DISK;
	nr.dwDisplayType = RESOURCEDISPLAYTYPE_SHARE;
	nr.dwUsage = RESOURCEUSAGE_CONNECTABLE;
	nr.lpLocalName = sDrive;
	nr.lpRemoteName = sTemp;

	//ユーザー認証ダイアログを表示
	dwRet = WNetAddConnection3(0, &nr, NULL, NULL, CONNECT_UPDATE_PROFILE | CONNECT_INTERACTIVE);

	return dwRet;
}

/*! 文字のエスケープ

	@param org [in] 変換したい文字列
	@param buf [out] 返還後の文字列を入れるバッファ
	@param cesc  [in] エスケープしないといけない文字
	@param cwith [in] エスケープに使う文字
	
	@retval 出力したバイト数 (Unicodeの場合は文字数)

	文字列中にそのまま使うとまずい文字がある場合にその文字の前に
	エスケープキャラクタを挿入するために使う．

	@note 変換後のデータは最大で元の文字列の2倍になる
	@note この関数は2バイト文字の考慮を行っていない

	@author genta
	@date 2002/01/04 新規作成
	@date 2002/01/30 genta &専用(dupamp)から一般の文字を扱えるように拡張．
		dupampはinline関数にした．
	@date 2002/02/01 genta bugfix エスケープする文字とされる文字の出力順序が逆だった
	@date 2004/06/19 genta Generic mapping対応
*/
int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith)
{
	TCHAR *out = buf;
	for( ; *org != _T('\0'); ++org, ++out ){
		if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = _T('\0');
	return out - buf;
}

/*! 文字のエスケープ

	@param org [in] 変換したい文字列
	@param buf [out] 返還後の文字列を入れるバッファ
	@param cesc  [in] エスケープしないといけない文字
	@param cwith [in] エスケープに使う文字
	
	@retval 出力したバイト数

	文字列中にそのまま使うとまずい文字がある場合にその文字の前に
	エスケープキャラクタを挿入するために使う．

	@note 変換後のデータは最大で元の文字列の2倍になる
	@note この関数は2バイト文字の考慮を行っている
	
	@note 2003.05.25 未使用のようだ
*/
int cescape_j(const char* org, char* buf, char cesc, char cwith)
{
	char *out = buf;
	for( ; *org != '\0'; ++org, ++out ){
		if( _IS_SJIS_1( (unsigned char)*org ) ){
			*out = *org;
			++out; ++org;
		}
		else if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = '\0';
	return out - buf;
}

/*	ヘルプの目次を表示
	目次タブを表示。問題があるバージョンでは、目次ページを表示。
*/
void ShowWinHelpContents( HWND hwnd, LPCTSTR lpszHelp )
{
	COsVersionInfo cOsVer;
	if ( cOsVer.HasWinHelpContentsProblem() ){
		/* 目次ページを表示する */
		::WinHelp( hwnd, lpszHelp, HELP_CONTENTS , 0 );
		return;
	}
	/* 目次タブを表示する */
	::WinHelp( hwnd, lpszHelp, HELP_COMMAND, (ULONG_PTR)"CONTENTS()" );
	return;
}


/*
 * カラー名からインデックス番号に変換する
 */
SAKURA_CORE_API int GetColorIndexByName( const char *name )
{
	int	i;
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if( strcmp( name, (const char*)colorIDXKeyName[i] ) == 0 ) return i;
	}
	return -1;
}

/*
 * インデックス番号からカラー名に変換する
 */
SAKURA_CORE_API const char* GetColorNameByIndex( int index )
{
	return colorIDXKeyName[index];
}

/*!
	@brief レジストリから文字列を読み出す．
	
	@param Hive [in] HIVE
	@param Path [in] レジストリキーへのパス
	@param Item [in] レジストリアイテム名．NULLで標準のアイテム．
	@param Buffer [out] 取得文字列を格納する場所
	@param BufferSize [in] Bufferの指す領域のサイズ
	
	@retval true 値の取得に成功
	@retval false 値の取得に失敗
	
	@author 鬼
	@date 2002.09.10 genta CWSH.cppから移動
*/
bool ReadRegistry(HKEY Hive, char const *Path, char const *Item, char *Buffer, unsigned BufferSize)
{
	bool Result = false;
	
	HKEY Key;
	if(RegOpenKeyEx(Hive, Path, 0, KEY_READ, &Key) == ERROR_SUCCESS)
	{
		ZeroMemory(Buffer, BufferSize);

		DWORD dwType = REG_SZ;
		DWORD dwDataLen = BufferSize - 1;
		
		Result = (RegQueryValueEx(Key, Item, NULL, &dwType, reinterpret_cast<unsigned char*>(Buffer), &dwDataLen) == ERROR_SUCCESS);
		
		RegCloseKey(Key);
	}
	return Result;
}

/*!
	@brief exeファイルのあるディレクトリ，または指定されたファイル名のフルパスを返す．
	
	@param pDir [out] EXEファイルのあるディレクトリを返す場所．
		予め_MAX_PATHのバッファを用意しておくこと．
	@param szFile [in] ディレクトリ名に結合するファイル名．
	
	@author genta
	@date 2002.12.02 genta
*/
void GetExecutableDir( char* pDir, const char* szFile )
{
	if( pDir == NULL )
		return;
	
	char	szPath[_MAX_PATH];
	// sakura.exe のパスを取得
	::GetModuleFileName( ::GetModuleHandle(NULL), szPath, sizeof(szPath) );
	if( szFile == NULL ){
		SplitPath_FolderAndFile( szPath, pDir, NULL );
	}
	else {
		char	szDir[_MAX_PATH];
		SplitPath_FolderAndFile( szPath, szDir, NULL );
		wsprintf( pDir, "%s\\%s", szDir, szFile );
	}
}


/*!
	@brief アプリケーションアイコンの取得
	
	アイコンファイルが存在する場合はそこから，無い場合は
	リソースファイルから取得する
	
	@param hInst [in] Instance Handle
	@param nResource [in] デフォルトアイコン用Resource ID
	@param szFile [in] アイコンファイル名
	@param bSmall [in] true: small icon (16x16) / false: large icon (32x32)
	
	@return アイコンハンドル．失敗した場合はNULL．
	
	@date 2002.12.02 genta 新規作成
	@author genta
*/
HICON GetAppIcon( HINSTANCE hInst, int nResource, const char* szFile, bool bSmall )
{
	// サイズの設定
	int size = ( bSmall ? 16 : 32 );

	char szPath[_MAX_PATH];
	
	GetExecutableDir( szPath, szFile );
	
	HICON hIcon;
	// ファイルからの読み込みをまず試みる
	hIcon = (HICON)::LoadImage( NULL, szPath, IMAGE_ICON, size, size,
			LR_SHARED | LR_LOADFROMFILE );
	if( hIcon != NULL ){
		return hIcon;
	}
	
	//	ファイルからの読み込みに失敗したらリソースから取得
	hIcon = (HICON)::LoadImage( hInst, MAKEINTRESOURCE(nResource),
		IMAGE_ICON, size, size, LR_SHARED );
	
	return hIcon;
}

/*! fnameが相対パスの場合は、実行ファイルのパスからの相対パスとして開く
	@author Moca
	@date 2003.06.23
*/
FILE* fopen_absexe(const char* fname, const char* mode)
{
	if( _IS_REL_PATH( fname ) ){
		char path[_MAX_PATH];
		GetExecutableDir( path, fname );
		return fopen( path, mode );
	}
	return fopen( fname, mode );
	
}

/*! fnameが相対パスの場合は、実行ファイルのパスからの相対パスとして開く
	@author Moca
	@date 2003.06.23
*/
HFILE _lopen_absexe(LPCSTR fname, int mode)
{
	// fnameが相対パス
	if( _IS_REL_PATH( fname ) ){
		char path[_MAX_PATH];
		GetExecutableDir( path, fname );
		return _lopen( path, mode );
	}
	return _lopen( fname, mode );
}


/*! 文字数制限機能付きstrncpy

	コピー先のバッファサイズから溢れないようにstrncpyする。
	バッファが不足する場合には2バイト文字の切断もあり得る。
	末尾の\0は付与されないが、コピーはコピー先バッファサイズ-1までにしておく。

	@param dst [in] コピー先領域へのポインタ
	@param dst_count [in] コピー先領域のサイズ
	@param src [in] コピー元
	@param src_count [in] コピーする文字列の末尾

	@retval 実際にコピーされたコピー先領域の1つ後を指すポインタ

	@author genta
	@date 2003.04.03 genta
*/
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	memcpy( dst, src, src_count );
	return dst + src_count;
}

/*! @brief ディレクトリの深さを計算する

	与えられたパス名からディレクトリの深さを計算する．
	パスの区切りは\．ルートディレクトリが深さ0で，サブディレクトリ毎に
	深さが1ずつ上がっていく．
 
	@param path [in] 深さを調べたいファイル/ディレクトリのフルパス

	@date 2003.04.30 genta 新規作成
*/
int CalcDirectoryDepth(const char* path)
{
	int depth = 0;
 
	//	とりあえず\の数を数える
	for( const char *p = path; *p != '\0'; ++p ){
		//	2バイト文字は区切りではない
		if( _IS_SJIS_1(*(unsigned const char*)p)){ // unsignedにcastしないと判定を誤る
			++p;
			if( *p == '\0' )
				break;
		}
		else if( *p == '\\' ){
			++depth;
			//	フルパスには入っていないはずだが念のため
			//	.\はカレントディレクトリなので，深さに関係ない．
			while( p[1] == '.' && p[2] == '\\' ){
				p += 2;
			}
		}
	}
 
	//	補正
	//	ドライブ名はパスの深さに数えない
	if(( 'A' <= (path[0] & ~0x20)) && ((path[0] & ~0x20) <= 'Z' ) &&
		path[1] == ':' && path[2] == '\\' ){
		//フルパス
		--depth; // C:\ の \ はルートの記号なので階層深さではない
	}
	else if( path[0] == '\\' ){
		if( path[1] == '\\' ){
			//	ネットワークパス
			//	先頭の2つはネットワークを表し，その次はホスト名なので
			//	ディレクトリ階層とは無関係
			depth -= 3;
		}
		else {
			//	ドライブ名無しのフルパス
			//	先頭の\は対象外
			--depth;
		}
	}
	return depth;
}

//	From Here May 01, 2004 genta MutiMonitor
CMultiMonitor	g_MultiMonitor;

bool GetMonitorWorkRect(HWND hWnd, LPRECT rcDesktop)
{
	return g_MultiMonitor.GetMonitorWorkRect( hWnd, rcDesktop );
}
//	To Here May 01, 2004 genta


// novice 2004/10/10 マウスサイドボタン対応
/*!
	Shift,Ctrl,Altキー状態の取得

	@retval nIdx Shift,Ctrl,Altキー状態
	@date 2004.10.10 関数化
*/
int getCtrlKeyState(void)
{
	int nIdx = 0;

	/* Shiftキーが押されているなら */
	if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
		nIdx |= _SHIFT;
	}
	/* Ctrlキーが押されているなら */
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		nIdx |= _CTRL;
	}
	/* Altキーが押されているなら */
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		nIdx |= _ALT;
	}

	return nIdx;
}


/*[EOF]*/
