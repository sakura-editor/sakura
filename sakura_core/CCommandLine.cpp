//	$Id$
/*!	@file
	@brief コマンドラインパーサ

	@author aroka
	@date	2002/01/08 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CEditAppより分離

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "CCommandLine.h"
#include "CMemory.h"
#include "CShareData.h"
#include "etc_uty.h"
#include <tchar.h>
#include <io.h>
#include <string.h>

CCommandLine* CCommandLine::_instance=0;

/*!
	コマンドラインのチェックを行って、オプション番号と
	引数がある場合はその先頭アドレスを返す。
	CCommandLine::ParseCommandLine()で使われる。

	@return オプションの番号。どれにも該当しないときは0を返す。

	@author genta
	@date Apr. 6, 2001
*/
int CCommandLine::CheckCommandLine(
	LPSTR  str, //!< [in] 検証する文字列（先頭の-は含まない）
	char** arg	//!< [out] 引数がある場合はその先頭へのポインタ
)
{

	/*!
		コマンドラインオプション解析用構造体配列
	*/
	struct _CmdLineOpt {
		LPCSTR opt;	//!< オプション文字列
		int len;	//!< オプションの文字列長（計算を省くため）
		int value;	//!< 変換後の値
	};

	/*!
		コマンドラインオプション
		後ろに引数を取らないもの
	*/
	static const _CmdLineOpt _COptWoA[] = {
		"CODE", 4, 1001,
		"R", 1, 1002,
		"NOWIN", 5, 1003,
		"GREPMODE", 8, 1100,
		"GREPDLG", 7, 1101,
		"DEBUGMODE", 9, 1999,
		NULL, 0, 0
	};

	/*!
		コマンドラインオプション
		後ろに引数を取るもの
	*/
	static const _CmdLineOpt _COptWithA[] = {
		"X", 1, 1,
		"Y", 1, 2,
		"VX", 2, 3,
		"VY", 2, 4,
		"TYPE", 4, 5,	//!< タイプ別設定 Mar. 7, 2002 genta
		"GKEY", 4, 101,
		"GFILE", 5, 102,
		"GFOLDER", 7, 103,
		"GOPT", 4, 104,
		NULL, 0, 0
	};

	const _CmdLineOpt *ptr;
	int len = lstrlen( str );

	//	引数がある場合を先に確認
	for( ptr = _COptWithA; ptr->opt != NULL; ptr++ ){
		if( len >= ptr->len &&	//	長さが足りているか
			//	オプション部分の長さチェック
			( str[ptr->len] == '=' || str[ptr->len] == ':' ) &&
			//	文字列の比較
			memcmp( str, ptr->opt, ptr->len ) == 0 ){
			*arg = str + ptr->len + 1;
			return ptr->value;
		}
	}

	//	引数がない場合
	for( ptr = _COptWoA; ptr->opt != NULL; ptr++ ){
		if( len == ptr->len &&	//	長さチェック
			//	文字列の比較
			memcmp( str, ptr->opt, ptr->len ) == 0 ){
			return ptr->value;
		}
	}
	return 0;	//	該当無し
}

/*! コマンドラインの解析

	WinMain()から呼び出される。
	
	@note
	これが呼び出された時点では共有メモリの初期化が完了していないため，
	共有メモリにアクセスしてはならない．
*/
void CCommandLine::ParseCommandLine(
	LPCSTR	pszCmdLineSrc,	//!< [in]コマンドライン文字列
	bool*		pbGrepMode,	//!< [out] TRUE: Grep Mode
	bool*		pbGrepDlg,	//!< [out] TRUE: Grep Dialog表示
	CMemory*	pcmGrepKey,	//!< [out] GrepのKey
	CMemory*	pcmGrepFile,
	CMemory*	pcmGrepFolder,
	bool*		pbGrepSubFolder,
	bool*		pbGrepLoHiCase,
	bool*		pbGrepRegularExp,
	bool*		pbGrepKanjiCode_AutoDetect,
	bool*		pbGrepOutputLine,
	bool*		pbGrepWordOnly,
	int	*		pnGrepOutputStyle,
	bool*		pbDebugMode,
	bool*		pbNoWindow,	//!< [out] TRUE: 編集Windowを開かない
	FileInfo*	pfi,
	bool*		pbReadOnly	//!< [out] TRUE: Read Only
)
{
	bool			bGrepMode;
	bool			bGrepDlg;
	CMemory			cmGrepKey;
	CMemory			cmGrepFile;
	CMemory			cmGrepFolder;
	bool			bGrepSubFolder;
	bool			bGrepLoHiCase;
	bool			bGrepRegularExp;
	bool			bGrepKanjiCode_AutoDetect;
	bool			bGrepOutputLine;
	bool			bGrepWordOnly;
	int				nGrepOutputStyle;
	bool			bDebugMode;
	bool			bNoWindow;
	FileInfo		fi;
	bool			bReadOnly;
	LPSTR			pszCmdLineWork;
	int				nCmdLineWorkLen;
	bool			bFind;
//	WIN32_FIND_DATA	w32fd;
//	HANDLE			hFind;
	TCHAR			szPath[_MAX_PATH + 1];
	int				i;
	int				j;
	int				nPos;
	LPSTR			pszToken;
	CMemory			cmWork;
	//const LPSTR	pszOpt;
	//int			nOptLen;

	bGrepMode = false;
	bGrepDlg = false;
	bGrepSubFolder = false;
	bGrepLoHiCase = false;
	bGrepRegularExp = false;
	bGrepKanjiCode_AutoDetect = false;
	bGrepOutputLine = false;
	bGrepWordOnly = false;
	nGrepOutputStyle = 1;
	bDebugMode = false;
	bNoWindow = false;

	//	Oct. 19, 2001 genta 初期値を-1にして，指定有り/無しを判別可能にしてみた
	//	Mar. 7, 2002 genta 初期化はコンストラクタで行う．
//	fi.m_nViewTopLine = -1;				/* 表示域の一番上の行(0開始) */
//	fi.m_nViewLeftCol = -1;				/* 表示域の一番左の桁(0開始) */
//	fi.m_nX = -1;						/* カーソル 物理位置(行頭からのバイト数) */
//	fi.m_nY = -1;						/* カーソル 物理位置(折り返し無し行位置) */
//	fi.m_bIsModified = 0;				/* 変更フラグ */
//	fi.m_nCharCode = CODE_AUTODETECT;	/* 文字コード種別 *//* 文字コード自動判別 */
//	fi.m_szPath[0] = '\0';				/* ファイル名 */
	bReadOnly = false;					/* 読み取り専用か */

	//	May 30, 2000 genta
	//	実行ファイル名をもとに漢字コードを固定する．
	{
		char	exename[512];
		::GetModuleFileName( NULL, exename, 512 );

		int		len = strlen( exename );

		for( char *p = exename + len - 1; p > exename; p-- ){
			if( *p == '.' ){
				if( '0' <= p[-1] && p[-1] <= '5' )
					fi.m_nCharCode = p[-1] - '0';
				break;
			}
		}
	}





	bFind = false;
	if( pszCmdLineSrc[0] != '-' ){
		memset( (char*)szPath, 0, sizeof( szPath ) );
		i = 0;
		j = 0;
		for( ; i < sizeof( szPath ) - 1 && i <= (int)lstrlen(pszCmdLineSrc); ++i ){
			if( pszCmdLineSrc[i] != ' ' && pszCmdLineSrc[i] != '\0' ){
				szPath[j] = pszCmdLineSrc[i];
				++j;
				continue;
			}
			/* ファイルの存在と、ファイルかどうかをチェック */
			if( -1 != _access( szPath, 0 ) ){
//? 2000.01.18 システム属性のファイルが開けない問題
//?				hFind = ::FindFirstFile( szPath, &w32fd );
//?				::FindClose( hFind );
//?				if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ||
//?					w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
//?				}else{
					bFind = true;
					break;
//?				}
			}
			szPath[j] = pszCmdLineSrc[i];
			++j;
		}
	}
	if( bFind ){
		strcpy( fi.m_szPath, szPath );	/* ファイル名 */
		nPos = j + 1;
	}else{
		nPos = 0;
	}
	pszCmdLineWork = new char[lstrlen( pszCmdLineSrc ) + 1];
	strcpy( pszCmdLineWork, pszCmdLineSrc );
	nCmdLineWorkLen = lstrlen( pszCmdLineWork );
	pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	while( pszToken != NULL ){
		if( !bFind && pszToken[0] != '-' ){
			if( pszToken[0] == '\"' ){
				cmWork.SetData( &pszToken[1],  lstrlen( pszToken ) - 2 );
				cmWork.Replace( "\"\"", "\"" );
				strcpy( fi.m_szPath, cmWork.GetPtr( NULL/*&nDummy*/ ) );	/* ファイル名 */
			}else{
				strcpy( fi.m_szPath, pszToken );							/* ファイル名 */
			}
		}else{
			++pszToken;	//	先頭の'-'はskip
			char *arg;
			switch( CheckCommandLine( pszToken, &arg ) ){
			case 1: //	X
				/* 行桁指定を1開始にした */
				fi.m_nX = atoi( arg ) - 1;
				break;
			case 2:	//	Y
				fi.m_nY = atoi( arg ) - 1;
				break;
			case 3:	// VX
				/* 行桁指定を1開始にした */
				fi.m_nViewLeftCol = atoi( arg ) - 1;
				break;
			case 4:	//	VY
				/* 行桁指定を1開始にした */
				fi.m_nViewTopLine = atoi( arg ) - 1;
				break;
			case 5:	//	T
				//	Mar. 7, 2002 genta
				//	ファイルタイプの強制指定
				strncpy( fi.m_szDocType, arg, MAX_DOCTYPE_LEN );
				fi.m_szDocType[ MAX_DOCTYPE_LEN ]= '\0';
				break;
			case 1001:	//	CODE
				fi.m_nCharCode = atoi( arg );
				break;
			case 1002:	//	R
				bReadOnly = true;
				break;
			case 1003:	//	NOWIN
				bNoWindow = true;
				break;
			case 1100:	//	GREPMODE
				bGrepMode = true;
				break;
			case 1101:	//	GREPDLG
				bGrepDlg = true;
				break;
			case 101:	//	GKEY
				//	前後の""を取り除く
				cmGrepKey.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepKey.Replace( "\"\"", "\"" );
				break;
			case 102:	//	GFILE
				//	前後の""を取り除く
				cmGrepFile.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFile.Replace( "\"\"", "\"" );
				break;
			case 103:	//	GFOLDER
				cmGrepFolder.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFolder.Replace( "\"\"", "\"" );
				break;
			case 104:	//	GOPT
				for( ; *arg != '\0' ; ++arg ){
					switch( *arg ){
					case 'S':	/* サブフォルダからも検索する */
						bGrepSubFolder = true;	break;
					case 'L':	/* 英大文字と英小文字を区別する */
						bGrepLoHiCase = true;	break;
					case 'R':	/* 正規表現 */
						bGrepRegularExp = true;	break;
					case 'K':	/* 文字コード自動判別 */
						bGrepKanjiCode_AutoDetect = true;	break;
					case 'P':	/* 行を出力するか該当部分だけ出力するか */
						bGrepOutputLine = true;	break;
					case 'W':	/* 単語単位で探す */
						bGrepWordOnly = true;	break;
					case '1':	/* Grep: 出力形式 */
						nGrepOutputStyle = 1;	break;
					case '2':	/* Grep: 出力形式 */
						nGrepOutputStyle = 2;	break;
					}
				}
				break;
			case 1999:
				bDebugMode = true;
				break;
			}
		}
		pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	}
	delete [] pszCmdLineWork;

	/* ファイル名 */
	if( '\0' != fi.m_szPath[0] ){
		/* ショートカット(.lnk)の解決 */
		if( TRUE == ResolveShortcutLink( NULL, fi.m_szPath, szPath ) ){
			strcpy( fi.m_szPath, szPath );
		}
		/* ロングファイル名を取得する */
		if( TRUE == ::GetLongFileName( fi.m_szPath, szPath ) ){
			strcpy( fi.m_szPath, szPath );
		}

		/* MRUから情報取得 */

	}

	/* 処理結果を格納 */
	*pbGrepMode					= bGrepMode;
	*pbGrepDlg					= bGrepDlg;
	*pcmGrepKey					= cmGrepKey;
	*pcmGrepFile				= cmGrepFile;
	*pcmGrepFolder				= cmGrepFolder;
	*pbGrepSubFolder			= bGrepSubFolder;
	*pbGrepLoHiCase				= bGrepLoHiCase;
	*pbGrepRegularExp			= bGrepRegularExp;
	*pbGrepKanjiCode_AutoDetect = bGrepKanjiCode_AutoDetect;
	*pbGrepOutputLine			= bGrepOutputLine;
	*pbGrepWordOnly				= bGrepWordOnly;
	*pnGrepOutputStyle			= nGrepOutputStyle;
	*pbDebugMode				= bDebugMode;
	*pbNoWindow					= bNoWindow;
	*pfi						= fi;
	*pbReadOnly					= bReadOnly;
	return;
}

/*! 
	シングルトン：プロセスで唯一のインスタンス
*/
CCommandLine* CCommandLine::Instance(LPSTR cmd)
{
		if( !_instance ){
			_instance = new CCommandLine(cmd);
		}
		return _instance;
}

/*! 
	コンストラクタ
*/
CCommandLine::CCommandLine(LPSTR cmd) : 
	m_pszCmdLineSrc(cmd)
{
	ParseCommandLine(
		m_pszCmdLineSrc,
		&m_bGrepMode,
		&m_bGrepDlg,
		&m_gi.cmGrepKey,
		&m_gi.cmGrepFile,
		&m_gi.cmGrepFolder,
		&m_gi.bGrepSubFolder,
		&m_gi.bGrepNoIgnoreCase,
		&m_gi.bGrepRegularExp,
		&m_gi.bGrepKanjiCode_AutoDetect,
		&m_gi.bGrepOutputLine,
		&m_gi.bGrepWordOnly,	//	Jun. 25, 2001 genta
		&m_gi.nGrepOutputStyle,
		&m_bDebugMode,
		&m_bNoWindow,
		&m_fi,
		&m_bReadOnly
	);
}

/*[EOF]*/
