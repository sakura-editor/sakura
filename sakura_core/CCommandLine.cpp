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
//#include "CShareData.h"	2002/03/24 YAZAKI
#include "etc_uty.h"
#include <tchar.h>
#include <io.h>
#include <string.h>
#include "CRunningTimer.h"

CCommandLine* CCommandLine::_instance = NULL;

/* コマンドラインオプション用定数 */
#define CMDLINEOPT_R			1002
#define CMDLINEOPT_NOWIN		1003
#define CMDLINEOPT_GREPMODE		1100
#define CMDLINEOPT_GREPDLG		1101
#define CMDLINEOPT_DEBUGMODE	1999
#define CMDLINEOPT_X			1
#define CMDLINEOPT_Y			2
#define CMDLINEOPT_VX			3
#define CMDLINEOPT_VY			4
#define CMDLINEOPT_TYPE			5
#define CMDLINEOPT_CODE			6
#define CMDLINEOPT_GKEY			101
#define CMDLINEOPT_GFILE		102
#define CMDLINEOPT_GFOLDER		103
#define CMDLINEOPT_GOPT			104
#define CMDLINEOPT_GCODE		105
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
		{"R", 1,			CMDLINEOPT_R},
		{"NOWIN", 5,		CMDLINEOPT_NOWIN},
		{"GREPMODE", 8,		CMDLINEOPT_GREPMODE},
		{"GREPDLG", 7,		CMDLINEOPT_GREPDLG},
		{"DEBUGMODE", 9,	CMDLINEOPT_DEBUGMODE},
		{NULL, 0, 0}
	};

	/*!
		コマンドラインオプション
		後ろに引数を取るもの
	*/
	static const _CmdLineOpt _COptWithA[] = {
		{"X", 1,			CMDLINEOPT_X},
		{"Y", 1,			CMDLINEOPT_Y},
		{"VX", 2,			CMDLINEOPT_VX},
		{"VY", 2,			CMDLINEOPT_VY},
		{"CODE", 4,			CMDLINEOPT_CODE},	// 2002/09/20 Moca _COptWoAから移動
		{"TYPE", 4,			CMDLINEOPT_TYPE},	//!< タイプ別設定 Mar. 7, 2002 genta
		{"GKEY", 4,			CMDLINEOPT_GKEY},
		{"GFILE", 5,		CMDLINEOPT_GFILE},
		{"GFOLDER", 7,		CMDLINEOPT_GFOLDER},
		{"GOPT", 4,			CMDLINEOPT_GOPT},
		{"GCODE", 5,		CMDLINEOPT_GCODE},	// 2002/09/21 Moca 追加
		{NULL, 0, 0}
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
	int*		pnGrepCharSet,
	bool*		pbGrepOutputLine,
	bool*		pbGrepWordOnly,
	int	*		pnGrepOutputStyle,
	bool*		pbDebugMode,
	bool*		pbNoWindow,	//!< [out] TRUE: 編集Windowを開かない
	FileInfo*	pfi,
	bool*		pbReadOnly	//!< [out] TRUE: Read Only
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CCommandLine::Parse" );

	bool			bGrepMode;
	bool			bGrepDlg;
	CMemory			cmGrepKey;
	CMemory			cmGrepFile;
	CMemory			cmGrepFolder;
	bool			bGrepSubFolder;
	bool			bGrepLoHiCase;
	bool			bGrepRegularExp;
	bool			bGrepOutputLine;
	bool			bGrepWordOnly;
	int				nGrepOutputStyle;
	int				nGrepCharSet;
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
	bGrepOutputLine = false;
	bGrepWordOnly = false;
	nGrepOutputStyle = 1;
	nGrepCharSet = CODE_SJIS;
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
				if( '0' <= p[-1] && p[-1] <= '6' )
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
				strcpy( fi.m_szPath, cmWork.GetPtr() );	/* ファイル名 */
			}else{
				strcpy( fi.m_szPath, pszToken );							/* ファイル名 */
			}
		}else{
			++pszToken;	//	先頭の'-'はskip
			char *arg;
			switch( CheckCommandLine( pszToken, &arg ) ){
			case CMDLINEOPT_X: //	X
				/* 行桁指定を1開始にした */
				fi.m_nX = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_Y:	//	Y
				fi.m_nY = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_VX:	// VX
				/* 行桁指定を1開始にした */
				fi.m_nViewLeftCol = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_VY:	//	VY
				/* 行桁指定を1開始にした */
				fi.m_nViewTopLine = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_TYPE:	//	TYPE
				//	Mar. 7, 2002 genta
				//	ファイルタイプの強制指定
				strncpy( fi.m_szDocType, arg, MAX_DOCTYPE_LEN );
				fi.m_szDocType[ MAX_DOCTYPE_LEN ]= '\0';
				break;
			case CMDLINEOPT_CODE:	//	CODE
				fi.m_nCharCode = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_R:	//	R
				bReadOnly = true;
				break;
			case CMDLINEOPT_NOWIN:	//	NOWIN
				bNoWindow = true;
				break;
			case CMDLINEOPT_GREPMODE:	//	GREPMODE
				bGrepMode = true;
				break;
			case CMDLINEOPT_GREPDLG:	//	GREPDLG
				bGrepDlg = true;
				break;
			case CMDLINEOPT_GKEY:	//	GKEY
				//	前後の""を取り除く
				cmGrepKey.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepKey.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GFILE:	//	GFILE
				//	前後の""を取り除く
				cmGrepFile.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFile.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GFOLDER:	//	GFOLDER
				cmGrepFolder.SetData( arg + 1,  lstrlen( arg ) - 2 );
				cmGrepFolder.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GOPT:	//	GOPT
				for( ; *arg != '\0' ; ++arg ){
					switch( *arg ){
					case 'S':	/* サブフォルダからも検索する */
						bGrepSubFolder = true;	break;
					case 'L':	/* 英大文字と英小文字を区別する */
						bGrepLoHiCase = true;	break;
					case 'R':	/* 正規表現 */
						bGrepRegularExp = true;	break;
					case 'K':	/* 文字コード自動判別 */
						// 2002/09/21 Moca 互換性保持のための処理
						nGrepCharSet = CODE_AUTODETECT;	break;
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
			// 2002/09/21 Moca Grepでの文字コードセット 追加
			case CMDLINEOPT_GCODE:
				nGrepCharSet = AtoiOptionInt( arg );	break;
			case CMDLINEOPT_DEBUGMODE:
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
	*pnGrepCharSet				= nGrepCharSet;
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
		&m_gi.nGrepCharSet,		//  2002/09/20 Moca
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
