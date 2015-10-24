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
	Copyright (C) 2006, ryoji, rastiv
	Copyright (C) 2007, ryoji, genta
	Copyright (C) 2009, syat

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "CCommandLine.h"
#include "CMemory.h"
#include <tchar.h>
#include <io.h>
#include <string.h>
#include "CRunningTimer.h"
#include "etc_uty.h"
#include "file.h"
#include "my_icmp.h"
#include "charcode.h"  // 2006.06.28 rastiv
#include "Debug.h"

/* コマンドラインオプション用定数 */
#define CMDLINEOPT_R			1002 //!< 読み取り専用
#define CMDLINEOPT_NOWIN		1003 //!< タスクトレイのみ起動
#define CMDLINEOPT_WRITEQUIT	1004 //!< SakuExtとの連動専用
#define CMDLINEOPT_GREPMODE		1100 //!< Grep実行モードで起動
#define CMDLINEOPT_GREPDLG		1101 //!< サクラエディタが起動すると同時にGrepダイアログを表示
#define CMDLINEOPT_DEBUGMODE	1999 //!< アウトプット用のウィンドウとして起動
#define CMDLINEOPT_NOMOREOPT	1998 //!< これ以降引数無効
#define CMDLINEOPT_X			1    //!< ファイルを開いたときのカーソルの桁位置を指定
#define CMDLINEOPT_Y			2    //!< ファイルを開いたときのカーソルの行位置を指定
#define CMDLINEOPT_VX			3    //!< スクロール：ウィンドウ左端の桁位置
#define CMDLINEOPT_VY			4    //!< スクロール：ウィンドウ上端の行位置
#define CMDLINEOPT_TYPE			5    //!< タイプ別設定
#define CMDLINEOPT_CODE			6    //!< 文字コード種別
#define CMDLINEOPT_SX			7    //!< ウィンドウの幅
#define CMDLINEOPT_SY			8    //!< ウィンドウの高さ
#define CMDLINEOPT_WX			9    //!< ウィンドウ左上のX座標
#define CMDLINEOPT_WY			10   //!< ウィンドウ左上のY座標
#define CMDLINEOPT_GKEY			101  //!< Grepの検索文字列
#define CMDLINEOPT_GFILE		102  //!< Grepの検索対象のファイル
#define CMDLINEOPT_GFOLDER		103  //!< Grepの検索対象のフォルダ
#define CMDLINEOPT_GOPT			104  //!< Grepの条件
#define CMDLINEOPT_GCODE		105  //!< Grepでの文字コードを指定
#define CMDLINEOPT_M			106  //!< 起動時に実行するマクロのファイル名を指定
#define CMDLINEOPT_MTYPE		107  //!< マクロの種類を拡張子名で指定
#define CMDLINEOPT_GROUP		500  //!< タブモードのグループを指定して開く

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
	LPTSTR	str,		//!< [in] 検証する文字列（先頭の-は含まない）
	int quotelen,		//!< [in] オプション末尾の引用符の長さ．オプション全体が引用符で囲まれている場合の考慮．
	TCHAR** arg,		//!< [out] 引数がある場合はその先頭へのポインタ
	int*	arglen		//!< [out] 引数の長さ
)
{
	/*!
		コマンドラインオプション解析用構造体配列
	*/
	struct _CmdLineOpt {
		LPCTSTR opt;	//!< オプション文字列
		int len;		//!< オプションの文字列長（計算を省くため）
		int value;		//!< 変換後の値
	};

	/*!
		コマンドラインオプション
		後ろに引数を取らないもの
	*/
	static const _CmdLineOpt _COptWoA[] = {
		{_T("R"),			1,	CMDLINEOPT_R},
		{_T("-"),			1,	CMDLINEOPT_NOMOREOPT},
		{_T("NOWIN"),		5,	CMDLINEOPT_NOWIN},
		{_T("WQ"),			2,	CMDLINEOPT_WRITEQUIT},	// 2007.05.19 ryoji sakuext用に追加
		{_T("GREPMODE"),	8,	CMDLINEOPT_GREPMODE},
		{_T("GREPDLG"),		7,	CMDLINEOPT_GREPDLG},
		{_T("DEBUGMODE"),	9,	CMDLINEOPT_DEBUGMODE},
		{NULL, 0, 0}
	};

	/*!
		コマンドラインオプション
		後ろに引数を取るもの
	*/
	static const _CmdLineOpt _COptWithA[] = {
		{_T("X"),		1,			CMDLINEOPT_X},
		{_T("Y"),		1,			CMDLINEOPT_Y},
		{_T("VX"),		2,			CMDLINEOPT_VX},
		{_T("VY"),		2,			CMDLINEOPT_VY},
		{_T("SX"),		2,			CMDLINEOPT_SX},
		{_T("SY"),		2,			CMDLINEOPT_SY},
		{_T("WX"),		2,			CMDLINEOPT_WX},
		{_T("WY"),		2,			CMDLINEOPT_WY},
		{_T("CODE"),	4,			CMDLINEOPT_CODE},	// 2002/09/20 Moca _COptWoAから移動
		{_T("TYPE"),	4,			CMDLINEOPT_TYPE},	//!< タイプ別設定 Mar. 7, 2002 genta
		{_T("GKEY"),	4,			CMDLINEOPT_GKEY},
		{_T("GFILE"),	5,			CMDLINEOPT_GFILE},
		{_T("GFOLDER"),	7,			CMDLINEOPT_GFOLDER},
		{_T("GOPT"),	4,			CMDLINEOPT_GOPT},
		{_T("GCODE"),	5,			CMDLINEOPT_GCODE},	// 2002/09/21 Moca 追加
		{_T("GROUP"),	5,			CMDLINEOPT_GROUP},	// 2007.06.26 ryoji
		{_T("M"),		1,			CMDLINEOPT_M},		// 2009.07.19 syat
		{_T("MTYPE"),	5,			CMDLINEOPT_MTYPE},	// 2009.07.19 syat
		{NULL, 0, 0}
	};

	const _CmdLineOpt *ptr;
	int len = lstrlen( str ) - quotelen;

	//	引数がある場合を先に確認
	for( ptr = _COptWithA; ptr->opt != NULL; ptr++ )
	{
		if( len >= ptr->len &&	//	長さが足りているか
			//	オプション部分の長さチェック
			( str[ptr->len] == '=' || str[ptr->len] == ':' ) &&
			//	文字列の比較
			my_memicmp( str, ptr->opt, ptr->len ) == 0 ){		// 2006.10.25 ryoji memcmp() -> _memicmp()
			*arg = str + ptr->len + 1;
			*arglen = len - ptr->len;
			return ptr->value;
		}
	}

	//	引数がない場合
	for( ptr = _COptWoA; ptr->opt != NULL; ptr++ )
	{
		if( len == ptr->len &&	//	長さチェック
			my_memicmp( str, ptr->opt, ptr->len ) == 0 )	//	文字列の比較	// 2006.10.25 ryoji memcmp() -> _memicmp()
		{
			*arglen = 0;
			return ptr->value;
		}
	}
	return 0;	//	該当無し
}

void strncpyWithCheckOverflow(char *dest, int destCount, char *src, int srcCount)
{
	if( destCount < srcCount + 1 ){
		dest[0] = '\0';
		TopErrorMessage( NULL,
					  _T("ファイルパスが長すぎます。 ANSI 版では %d バイト以上の絶対パスを扱えません。"),
					  destCount );
	}else{
		strcpy(dest, src);
	}
}


/*! コマンドラインの解析

	WinMain()から呼び出される。
	
	@date 2005-08-24 D.S.Koba 関数のstaticをやめ，メンバ変数を引数で渡すのをやめる
	@date 2007.09.09 genta Visual Studioが各々の引数をお節介にも""で囲む問題に対応．
		オプションが""で囲まれた場合に対応する．
		そうすると-で始まるファイル名を指定できなくなるので，
		それ以降オプション解析をしないという "--" オプションを新設する．
	@date 2012.02.25 novice 複数ファイル読み込み

	@note
	これが呼び出された時点では共有メモリの初期化が完了していないため，
	共有メモリにアクセスしてはならない．
*/
void CCommandLine::ParseCommandLine( LPCTSTR pszCmdLineSrc )
{
	MY_RUNNINGTIMER( cRunningTimer, "CCommandLine::Parse" );

	//	May 30, 2000 genta
	//	実行ファイル名をもとに漢字コードを固定する．
	{
		TCHAR	exename[512];
		::GetModuleFileName( NULL, exename, _countof(exename) );

		int		len = _tcslen( exename );

		for( TCHAR *p = exename + len - 1; p > exename; p-- ){
			if( *p == _T('.') ){
				ECodeType n = (ECodeType)(p[-1] - _T('0'));
				if(IsValidCodeType(n))
					m_fi.m_nCharCode = n;
				break;
			}
		}
	}


	TCHAR	szPath[_MAX_PATH];
	bool	bFind = false;				// ファイル名発見フラグ
	bool	bParseOptDisabled = false;	// 2007.09.09 genta オプション解析を行わなず，ファイル名として扱う
	int		nPos;
	int		i;
	if( pszCmdLineSrc[0] != _T('-') ){
		for( i = 0; i < _countof( szPath ); ++i ){
			if( pszCmdLineSrc[i] == _T(' ') || pszCmdLineSrc[i] == _T('\0') ){
				/* ファイルの存在をチェック */
				szPath[i] = _T('\0');	// 終端文字
				if( fexist(szPath) ){
					bFind = true;
					break;
				}
				if( pszCmdLineSrc[i] == _T('\0') ){
					break;
				}
			}
			szPath[i] = pszCmdLineSrc[i];
		}
	}
	if( bFind ){
		ResolvePath(szPath);
		_tcscpy( m_fi.m_szPath, szPath );	/* ファイル名 */
		nPos = i + 1;
	}else{
		m_fi.m_szPath[0] = _T('\0');
		nPos = 0;
	}

	LPTSTR pszCmdLineWork = new TCHAR[lstrlen( pszCmdLineSrc ) + 1];
	_tcscpy( pszCmdLineWork, pszCmdLineSrc );
	int nCmdLineWorkLen = lstrlen( pszCmdLineWork );
	LPTSTR pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, _T(" ") );
	while( pszToken != NULL )
	{
		DEBUG_TRACE( _T("OPT=[%ts]\n"), pszToken );

		//	2007.09.09 genta オプション判定ルール変更．オプション解析停止と""で囲まれたオプションを考慮
		if( ( bParseOptDisabled ||
			! (pszToken[0] == '-' || pszToken[0] == '"' && pszToken[1] == '-' ) )){

			if( pszToken[0] == _T('\"') ){
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
					cmWork.SetString( &pszToken[1], len - ( pszToken[len] == _T('"') ? 1 : 0 ));
					cmWork.Replace( _T("\"\""), _T("\"") );
					strncpyWithCheckOverflow(szPath, _countof(szPath), cmWork.GetStringPtr(), cmWork.GetStringLength());
				}
				else {
					szPath[0] = _T('\0');
				}
			}
			else{
				strncpyWithCheckOverflow(szPath, _countof(szPath), pszToken, strlen(pszToken));
			}

			// Nov. 11, 2005 susu
			// 不正なファイル名のままだとファイル保存時ダイアログが出なくなるので
			// 簡単なファイルチェックを行うように修正
			if (!memcmp(szPath, _T("file:///"), 8)) {
				char tmp_str[_MAX_PATH + 1];
				_tcscpy(tmp_str, &(szPath[8]));
				_tcscpy(szPath, tmp_str);
			}
			int len = _tcslen(szPath);
			for (int i = 0; i < len ; i ++) {
				if ( (szPath[i] == '<' ||	//	0x3C
					  szPath[i] == '>' ||	//	0x3E
					  szPath[i] == '?' ||	//	0x3F
					  szPath[i] == '"' ||	//	0x22
					  szPath[i] == '|' ||	//	0x7C
					  szPath[i] == '*' ||	//	0x2A
					  0
					 ) &&
					( i ==0 || (i > 0 && ! _IS_SJIS_1( (unsigned char)(szPath[i - 1] )) ))){
						TCHAR msg_str[_MAX_PATH + 1];
						_stprintf(
							msg_str,
							_T("%s\r\n")
							_T("上記のファイル名は不正です。ファイル名に \\ / : * ? \" < > | の文字は使えません。 "),
							szPath
						);
						MessageBox( NULL, msg_str, _T("FileNameError"), MB_OK);
						szPath[0] = _T('\0');
						break;
				}
			}

			if (szPath[0] != _T('\0')) {
				ResolvePath(szPath);
				if (m_fi.m_szPath[0] == _T('\0')) {
					_tcscpy(m_fi.m_szPath, szPath );
				}
				else {
					m_vFiles.push_back( szPath );
				}
			}
		}
		else{
			int nQuoteLen = 0;
			if( *pszToken == '"' ){
				++pszToken;	// 2007.09.09 genta 先頭の"はスキップ
				nQuoteLen = 1;
				int tokenlen = _tcslen( pszToken );
				if( pszToken[ tokenlen-1 ] == '"' ){	// 2009.07.19 syat 末尾の"を取り除く
					pszToken[ tokenlen-1 ] = '\0';
				}
			}
			++pszToken;	//	先頭の'-'はskip
			TCHAR *arg = NULL;
			int nArgLen;
			switch( CheckCommandLine( pszToken, nQuoteLen, &arg, &nArgLen ) ){
			case CMDLINEOPT_X: //	X
				/* 行桁指定を1開始にした */
				m_fi.m_ptCursor.x = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_Y:	//	Y
				m_fi.m_ptCursor.y = AtoiOptionInt( arg ) - 1;
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
				{
					_tcsncpy( m_fi.m_szDocType, arg, MAX_DOCTYPE_LEN );
					m_fi.m_szDocType[ nArgLen < MAX_DOCTYPE_LEN ? nArgLen : MAX_DOCTYPE_LEN ]= '\0';
				}
				break;
			case CMDLINEOPT_CODE:	//	CODE
				m_fi.m_nCharCode = (ECodeType)AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_R:	//	R
				m_bReadOnly = true;
				break;
			case CMDLINEOPT_NOWIN:	//	NOWIN
				m_bNoWindow = true;
				break;
			case CMDLINEOPT_WRITEQUIT:	//	WRITEQUIT	// 2007.05.19 ryoji sakuext用に追加
				m_bWriteQuit = true;
				m_bNoWindow = true;	// 2007.09.05 ryoji -WQを指定されたら-NOWINも指定されたとして扱う
				break;
			case CMDLINEOPT_GREPMODE:	//	GREPMODE
				m_bGrepMode = true;
				if( _T('\0') == m_fi.m_szDocType[0] ){
					_tcscpy( m_fi.m_szDocType , _T("grepout") );
				}
				break;
			case CMDLINEOPT_GREPDLG:	//	GREPDLG
				m_bGrepDlg = true;
				break;
			case CMDLINEOPT_GKEY:	//	GKEY
				//	前後の""を取り除く
				m_gi.cmGrepKey.SetString( arg + 1,  lstrlen( arg ) - 2 );
				m_gi.cmGrepKey.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_GFILE:	//	GFILE
				//	前後の""を取り除く
				m_gi.cmGrepFile.SetString( arg + 1,  lstrlen( arg ) - 2 );
				m_gi.cmGrepFile.Replace( _T("\"\""), _T("\"") );
				break;
			case CMDLINEOPT_GFOLDER:	//	GFOLDER
				m_gi.cmGrepFolder.SetString( arg + 1,  lstrlen( arg ) - 2 );
				m_gi.cmGrepFolder.Replace( _T("\"\""), _T("\"") );
				break;
			case CMDLINEOPT_GOPT:	//	GOPT
				for( ; *arg != '\0' ; ++arg ){
					switch( *arg ){
					case 'S':
						// サブフォルダからも検索する
						m_gi.bGrepSubFolder = true;	break;
					case 'L':
						// 英大文字と英小文字を区別する
						m_gi.sGrepSearchOption.bLoHiCase = true;	break;
					case 'R':
						// 正規表現
						m_gi.sGrepSearchOption.bRegularExp = true;	break;
					case 'K':
						// 文字コード自動判別
						// 2002/09/21 Moca 互換性保持のための処理
						m_gi.nGrepCharSet = CODE_AUTODETECT;	break;
					case 'P':
						// 行を出力するか該当部分だけ出力するか
						m_gi.bGrepOutputLine = true;	break;
					case 'W':
						// 単語単位で探す
						m_gi.sGrepSearchOption.bWordOnly = true;	break;
					case '1':
						// Grep: 出力形式
						m_gi.nGrepOutputStyle = 1;	break;
					case '2':
						// Grep: 出力形式
						m_gi.nGrepOutputStyle = 2;	break;
					}
				}
				break;
			// 2002/09/21 Moca Grepでの文字コードセット 追加
			case CMDLINEOPT_GCODE:
				m_gi.nGrepCharSet = (ECodeType)AtoiOptionInt( arg );	break;
			case CMDLINEOPT_GROUP:	// GROUP	// 2007.06.26 ryoji
				m_nGroup = AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_DEBUGMODE:
				m_bDebugMode = true;
				// 2010.06.16 Moca -TYPE=output 扱いとする
				if( _T('\0') == m_fi.m_szDocType[0] ){
					_tcscpy( m_fi.m_szDocType , _T("output") );
				}
				break;
			case CMDLINEOPT_NOMOREOPT:	// 2007.09.09 genta これ以降引数無効
				bParseOptDisabled = true;
				break;
			case CMDLINEOPT_M:			// 2009.07.19 syat 追加
				m_cmMacro.SetString( arg );
				m_cmMacro.Replace( "\"\"", "\"" );
				break;
			case CMDLINEOPT_MTYPE:		// 2009.07.19 syat 追加
				m_cmMacroType.SetString( arg );
				break;
			}
		}
		pszToken = my_strtok( pszCmdLineWork, nCmdLineWorkLen, &nPos, _T(" ") );
	}
	delete [] pszCmdLineWork;

	return;
}

/*! 
	コンストラクタ
	
	@date 2005-08-24 D.S.Koba ParseCommandLine()変更によりメンバ変数に初期値代入
*/
CCommandLine::CCommandLine()
{
	m_bGrepMode				= false;
	m_bGrepDlg				= false;
	m_bDebugMode			= false;
	m_bNoWindow				= false;
	m_bWriteQuit			= false;
	m_gi.bGrepSubFolder		= false;
	m_gi.sGrepSearchOption.Reset();
	m_gi.nGrepCharSet		= CODE_SJIS;
	m_gi.bGrepOutputLine	= false;
	m_gi.nGrepOutputStyle	= 1;
	m_bReadOnly				= false;
	m_nGroup				= -1;		// 2007.06.26 ryoji
}
/*[EOF]*/
