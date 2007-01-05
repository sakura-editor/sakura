/*!	@file
	@brief コマンドラインパーサ

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CEditAppより分離
	Copyright (C) 2002, genta, Moca
	Copyright (C) 2005, D.S.Koba, genta, susu
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include "CCommandLine.h"
#include "CMemory.h"
#include "etc_uty.h"
#include <tchar.h>
#include <io.h>
#include <string.h>
#include "CRunningTimer.h"
// 関数をマクロ再定義するので my_icmp.h は最後に置く	// 2006.10.25 ryoji
#include "my_icmp.h"
#include "charcode.h"  // 2006.06.28 rastiv

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
#define CMDLINEOPT_SX			7
#define CMDLINEOPT_SY			8
#define CMDLINEOPT_WX			9
#define CMDLINEOPT_WY			10
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
	@date 2006.10.25 ryoji オプション文字列の大文字小文字を区別しない
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
		{"SX", 2,			CMDLINEOPT_SX},
		{"SY", 2,			CMDLINEOPT_SY},
		{"WX", 2,			CMDLINEOPT_WX},
		{"WY", 2,			CMDLINEOPT_WY},
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
			_memicmp( str, ptr->opt, ptr->len ) == 0 ){		// 2006.10.25 ryoji memcmp() -> _memicmp()
			*arg = str + ptr->len + 1;
			return ptr->value;
		}
	}

	//	引数がない場合
	for( ptr = _COptWoA; ptr->opt != NULL; ptr++ ){
		if( len == ptr->len &&	//	長さチェック
			//	文字列の比較
			_memicmp( str, ptr->opt, ptr->len ) == 0 ){		// 2006.10.25 ryoji memcmp() -> _memicmp()
			return ptr->value;
		}
	}
	return 0;	//	該当無し
}

/*! コマンドラインの解析

	WinMain()から呼び出される。
	
	@date 2005-08-24 D.S.Koba 関数のstaticをやめ，メンバ変数を引数で渡すのをやめる
	
	@note
	これが呼び出された時点では共有メモリの初期化が完了していないため，
	共有メモリにアクセスしてはならない．
*/
void CCommandLine::ParseCommandLine( void )
{
	MY_RUNNINGTIMER( cRunningTimer, "CCommandLine::Parse" );

	//	May 30, 2000 genta
	//	実行ファイル名をもとに漢字コードを固定する．
	{
		char	exename[512];
		::GetModuleFileName( NULL, exename, 512 );

		int		len = strlen( exename );

		for( char *p = exename + len - 1; p > exename; p-- ){
			if( *p == '.' ){
				if( '0' <= p[-1] && p[-1] <= '6' )
					m_fi.m_nCharCode = p[-1] - '0';
				break;
			}
		}
	}


	TCHAR	szPath[_MAX_PATH + 1];
	bool	bFind = false;
	int		nPos;
	int		i, j;
//	WIN32_FIND_DATA	w32fd;
//	HANDLE			hFind;
	if( m_pszCmdLineSrc[0] != '-' ){
		memset( (char*)szPath, 0, sizeof( szPath ) );
		i = 0;
		j = 0;
		for( ; i < sizeof( szPath ) - 1 && i <= (int)lstrlen(m_pszCmdLineSrc); ++i ){
			if( m_pszCmdLineSrc[i] != ' ' && m_pszCmdLineSrc[i] != '\0' ){
				szPath[j] = m_pszCmdLineSrc[i];
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
			szPath[j] = m_pszCmdLineSrc[i];
			++j;
		}
	}
	if( bFind ){
		strcpy( m_fi.m_szPath, szPath );	/* ファイル名 */
		nPos = j + 1;
	}else{
		nPos = 0;
	}
	LPSTR pszCmdLineWork = new char[lstrlen( m_pszCmdLineSrc ) + 1];
	strcpy( pszCmdLineWork, m_pszCmdLineSrc );
	int nCmdLineWorkLen = lstrlen( pszCmdLineWork );
	LPSTR pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	while( pszToken != NULL ){
		if( !bFind && pszToken[0] != '-' ){
			if( pszToken[0] == '\"' ){
				CMemory cmWork;
				//	Nov. 3, 2005 genta
				//	末尾のクォーテーションが無い場合を考慮して，
				//	最後がダブルクォートの場合のみ取り除く
				//	ファイル名には使えない文字なのでファイル名に含まれている場合は考慮不要
				//	またSHIFT-JISの2バイト目の考慮も不要
				//	Nov. 27, 2005 genta
				//	引数がダブルクォート1つの場合に，その1つを最初と最後の1つずつと
				//	見間違えて，インデックス-1にアクセスしてしまうのを防ぐために長さをチェックする
				//	ファイル名の後ろにあるOptionを解析するため，ループは継続
				int len = lstrlen( pszToken + 1 );
				if( len > 0 ){
					cmWork.SetData( &pszToken[1], len - ( pszToken[len] == '"' ? 1 : 0 ));
					cmWork.Replace( "\"\"", "\"" );
					strcpy( m_fi.m_szPath, cmWork.GetPtr() );	/* ファイル名 */
				}
				else {
					m_fi.m_szPath[0] = '\0';
				}
			}else{
				strcpy( m_fi.m_szPath, pszToken );							/* ファイル名 */
			}

			// Nov. 11, 2005 susu
			// 不正なファイル名のままだとファイル保存時ダイアログが出なくなるので
			// 簡単なファイルチェックを行うように修正
			if (!memcmp(m_fi.m_szPath, "file:///", 8)) {
				char tmp_str[_MAX_PATH + 1];
				strcpy(tmp_str, &(m_fi.m_szPath[8]));
				strcpy(m_fi.m_szPath, tmp_str);
			}
			int len = strlen(m_fi.m_szPath);
			for (int i = 0; i < len ; i ++) {
				if ( (m_fi.m_szPath[i] == '<' ||	//	0x3C
					  m_fi.m_szPath[i] == '>' ||	//	0x3E
					  m_fi.m_szPath[i] == '?' ||	//	0x3F
					  m_fi.m_szPath[i] == '"' ||	//	0x22
					  m_fi.m_szPath[i] == '|' ||	//	0x7C
					  m_fi.m_szPath[i] == '*' ||	//	0x2A
					  0
					 ) &&
					( i ==0 || (i > 0 && ! _IS_SJIS_1( (unsigned char)(m_fi.m_szPath[i - 1] )) ))){
						char msg_str[_MAX_PATH + 1];
						sprintf( msg_str, "%s\r\n上記のファイル名は不正です。ファイル名に \\ / : * ? \" < > | の文字は使えません。 ", m_fi.m_szPath );
						MessageBox( NULL, msg_str, "FileNameError", MB_OK);
						m_fi.m_szPath[0] = '\0';
						break;
				}
			}

		}else{
			++pszToken;	//	先頭の'-'はskip
			char *arg;
			switch( CheckCommandLine( pszToken, &arg ) ){
			case CMDLINEOPT_X: //	X
				/* 行桁指定を1開始にした */
				m_fi.m_nX = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_Y:	//	Y
				m_fi.m_nY = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_VX:	// VX
				/* 行桁指定を1開始にした */
				m_fi.m_nViewLeftCol = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_VY:	//	VY
				/* 行桁指定を1開始にした */
				m_fi.m_nViewTopLine = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_SX: //	SX
				m_fi.m_nWindowSizeX = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_SY:	//	SY
				m_fi.m_nWindowSizeY = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_WX: //	WX
				m_fi.m_nWindowOriginX = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_WY:	//	WY
				m_fi.m_nWindowOriginY = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_TYPE:	//	TYPE
				//	Mar. 7, 2002 genta
				//	ファイルタイプの強制指定
				strncpy( m_fi.m_szDocType, arg, MAX_DOCTYPE_LEN );
				m_fi.m_szDocType[ MAX_DOCTYPE_LEN ]= '\0';
				break;
			case CMDLINEOPT_CODE:	//	CODE
				m_fi.m_nCharCode = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_R:	//	R
				m_bReadOnly = true;
				break;
			case CMDLINEOPT_NOWIN:	//	NOWIN
				m_bNoWindow = true;
				break;
			case CMDLINEOPT_GREPMODE:	//	GREPMODE
				m_bGrepMode = true;
				break;
			case CMDLINEOPT_GREPDLG:	//	GREPDLG
				m_bGrepDlg = true;
				break;
			case CMDLINEOPT_GKEY:	//	GKEY
				//	前後の""を取り除く
				m_gi.cmGrepKey.SetData( arg + 1,  lstrlen( arg ) - 2 );
				m_gi.cmGrepKey.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GFILE:	//	GFILE
				//	前後の""を取り除く
				m_gi.cmGrepFile.SetData( arg + 1,  lstrlen( arg ) - 2 );
				m_gi.cmGrepFile.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GFOLDER:	//	GFOLDER
				m_gi.cmGrepFolder.SetData( arg + 1,  lstrlen( arg ) - 2 );
				m_gi.cmGrepFolder.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GOPT:	//	GOPT
				for( ; *arg != '\0' ; ++arg ){
					switch( *arg ){
					case 'S':	/* サブフォルダからも検索する */
						m_gi.bGrepSubFolder = true;	break;
					case 'L':	/* 英大文字と英小文字を区別する */
						m_gi.bGrepNoIgnoreCase = true;	break;
					case 'R':	/* 正規表現 */
						m_gi.bGrepRegularExp = true;	break;
					case 'K':	/* 文字コード自動判別 */
						// 2002/09/21 Moca 互換性保持のための処理
						m_gi.nGrepCharSet = CODE_AUTODETECT;	break;
					case 'P':	/* 行を出力するか該当部分だけ出力するか */
						m_gi.bGrepOutputLine = true;	break;
					case 'W':	/* 単語単位で探す */
						m_gi.bGrepWordOnly = true;	break;
					case '1':	/* Grep: 出力形式 */
						m_gi.nGrepOutputStyle = 1;	break;
					case '2':	/* Grep: 出力形式 */
						m_gi.nGrepOutputStyle = 2;	break;
					}
				}
				break;
			// 2002/09/21 Moca Grepでの文字コードセット 追加
			case CMDLINEOPT_GCODE:
				m_gi.nGrepCharSet = AtoiOptionInt( arg );	break;
			case CMDLINEOPT_DEBUGMODE:
				m_bDebugMode = true;
				break;
			}
		}
		pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, " " );
	}
	delete [] pszCmdLineWork;

	/* ファイル名 */
	if( '\0' != m_fi.m_szPath[0] ){
		/* ショートカット(.lnk)の解決 */
		if( TRUE == ResolveShortcutLink( NULL, m_fi.m_szPath, szPath ) ){
			strcpy( m_fi.m_szPath, szPath );
		}
		/* ロングファイル名を取得する */
		if( TRUE == ::GetLongFileName( m_fi.m_szPath, szPath ) ){
			strcpy( m_fi.m_szPath, szPath );
		}

		/* MRUから情報取得 */

	}

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
	
	@date 2005-08-24 D.S.Koba ParseCommandLine()変更によりメンバ変数に初期値代入
*/
CCommandLine::CCommandLine(LPSTR cmd) : 
	m_pszCmdLineSrc(cmd)
{
	m_bGrepMode				= false;
	m_bGrepDlg				= false;
	m_bDebugMode			= false;
	m_bNoWindow				= false;
	m_gi.bGrepSubFolder		= false;
	m_gi.bGrepNoIgnoreCase	= false;
	m_gi.bGrepRegularExp	= false;
	m_gi.nGrepCharSet		= CODE_SJIS;
	m_gi.bGrepOutputLine	= false;
	m_gi.bGrepWordOnly		= false;
	m_gi.nGrepOutputStyle	= 1;
	m_bReadOnly				= false;
	
	ParseCommandLine();
}
/*[EOF]*/
