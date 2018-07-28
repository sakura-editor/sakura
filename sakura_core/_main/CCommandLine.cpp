/*!	@file
	@brief コマンドラインパーサ

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CControlTrayより分離
	Copyright (C) 2002, genta, Moca
	Copyright (C) 2005, D.S.Koba, genta, susu
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "CCommandLine.h"
#include "mem/CMemory.h"
#include <tchar.h>
#include <io.h>
#include <string.h>
#include "debug/CRunningTimer.h"
#include "charset/charcode.h"  // 2006.06.28 rastiv
#include "io/CTextStream.h"
#include "util/shell.h"
#include "util/file.h"
#include "env/CSakuraEnvironment.h"

/* コマンドラインオプション用定数 */
#define CMDLINEOPT_R			1002 //!< ビューモード
#define CMDLINEOPT_NOWIN		1003 //!< タスクトレイのみ起動
#define CMDLINEOPT_WRITEQUIT	1004 //!< SakuExtとの連動専用
#define CMDLINEOPT_GREPMODE		1100 //!< Grep実行モードで起動
#define CMDLINEOPT_GREPDLG		1101 //!< サクラエディタが起動すると同時にGrepダイアログを表示
#define CMDLINEOPT_DEBUGMODE	1999 //!< アウトプット用のウィンドウとして起動
#define CMDLINEOPT_NOMOREOPT	1998 //!< これ以降引数無効
#define CMDLINEOPT_AT			11   //!< レスポンスファイルを指定
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
#define CMDLINEOPT_GREPR		108  //!< Grepの置換文字列
#define CMDLINEOPT_GROUP		500  //!< タブモードのグループを指定して開く
#define CMDLINEOPT_PROF			501  //!< プロファイルを選択
#define CMDLINEOPT_PROFMGR		502  //!< プロファイルマネージャを起動時に表示

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
		bool bLen0;
	};

	/*!
		コマンドラインオプション
		後ろに引数を取らないもの
	*/
	static const _CmdLineOpt _COptWoA[] = {
		{_T("R"),			1,	CMDLINEOPT_R, false},
		{_T("-"),			1,	CMDLINEOPT_NOMOREOPT, false},
		{_T("NOWIN"),		5,	CMDLINEOPT_NOWIN, false},
		{_T("WQ"),			2,	CMDLINEOPT_WRITEQUIT, false},	// 2007.05.19 ryoji sakuext用に追加
		{_T("GREPMODE"),	8,	CMDLINEOPT_GREPMODE, false},
		{_T("GREPDLG"),		7,	CMDLINEOPT_GREPDLG, false},
		{_T("DEBUGMODE"),	9,	CMDLINEOPT_DEBUGMODE, false},
		{_T("PROFMGR"),		7,	CMDLINEOPT_PROFMGR, false},
		{NULL, 0, 0}
	};

	/*!
		コマンドラインオプション
		後ろに引数を取るもの
	*/
	static const _CmdLineOpt _COptWithA[] = {
		{_T("@"),		1,			CMDLINEOPT_AT, false},
		{_T("X"),		1,			CMDLINEOPT_X, false},
		{_T("Y"),		1,			CMDLINEOPT_Y, false},
		{_T("VX"),		2,			CMDLINEOPT_VX, false},
		{_T("VY"),		2,			CMDLINEOPT_VY, false},
		{_T("SX"),		2,			CMDLINEOPT_SX, false},
		{_T("SY"),		2,			CMDLINEOPT_SY, false},
		{_T("WX"),		2,			CMDLINEOPT_WX, false},
		{_T("WY"),		2,			CMDLINEOPT_WY, false},
		{_T("CODE"),	4,			CMDLINEOPT_CODE, false},	// 2002/09/20 Moca _COptWoAから移動
		{_T("TYPE"),	4,			CMDLINEOPT_TYPE, false},	//!< タイプ別設定 Mar. 7, 2002 genta
		{_T("GKEY"),	4,			CMDLINEOPT_GKEY, false},
		{_T("GREPR"),	5,			CMDLINEOPT_GREPR, true},
		{_T("GFILE"),	5,			CMDLINEOPT_GFILE, false},
		{_T("GFOLDER"),	7,			CMDLINEOPT_GFOLDER, false},
		{_T("GOPT"),	4,			CMDLINEOPT_GOPT, false},
		{_T("GCODE"),	5,			CMDLINEOPT_GCODE, false},	// 2002/09/21 Moca 追加
		{_T("GROUP"),	5,			CMDLINEOPT_GROUP, false},	// 2007.06.26 ryoji
		{_T("M"),		1,			CMDLINEOPT_M, false},		// 2009.06.14 syat
		{_T("MTYPE"),	5,			CMDLINEOPT_MTYPE, false},	// 2009.06.14 syat
		{_T("PROF"),	4,			CMDLINEOPT_PROF, true},	// 2013.12.20 Moca
		{NULL, 0, 0}
	};

	const _CmdLineOpt *ptr;
	int len = lstrlen( str );

	//	引数がある場合を先に確認
	for( ptr = _COptWithA; ptr->opt != NULL; ptr++ )
	{
		if( len >= ptr->len &&	//	長さが足りているか
			( str[ptr->len] == '=' || str[ptr->len] == ':' ) &&	//	オプション部分の長さチェック
			auto_memicmp( str, ptr->opt, ptr->len ) == 0 )	//	文字列の比較	// 2006.10.25 ryoji memcmp() -> _memicmp()
		{
			*arg = str + ptr->len + 1;				// 引数開始位置
			*arglen = len - ptr->len - 1;
			if (**arg == '"') {						// 引数先頭に"があれば削除
				(*arg)++;
				(*arglen)--;
				if (*arglen > 0 && (*arg)[(*arglen)-1] == '"') {	// 引数末尾に"があれば削除
					(*arg)[(*arglen)-1] = '\0';
					(*arglen)--;
				}
			}
			if (*arglen <= 0 && !(ptr->bLen0)) {
				return 0;		//2010.06.12 syat 値なしはオプションとして認めない
			}
			return ptr->value;
		}
	}

	//	引数がない場合
	for( ptr = _COptWoA; ptr->opt != NULL; ptr++ )
	{
		if( len == ptr->len &&	//	長さチェック
			auto_memicmp( str, ptr->opt, ptr->len ) == 0 )	//	文字列の比較
		{
			*arglen = 0;
			return ptr->value;
		}
	}
	return 0;	//	該当無し
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
void CCommandLine::ParseCommandLine( LPCTSTR pszCmdLineSrc, bool bResponse )
{
	MY_RUNNINGTIMER( cRunningTimer, "CCommandLine::Parse" );

	//	May 30, 2000 genta
	//	実行ファイル名をもとに漢字コードを固定する．
	{
		TCHAR	exename[512];
		::GetModuleFileName( NULL, exename, _countof(exename) );
		WCHAR	wexename[512];
		auto_strcpy( wexename, to_wchar(exename) );

		int		len = wcslen( wexename );

		for( int i = len - 1; 0 <= i; i-- ){
			if(wexename[i] == L'.' ){
				wexename[i] = L'\0';
				int k = i - 1;
				for(; 0 < k && WCODE::Is09(wexename[k]); k-- ){}
				if( k < 0 || !WCODE::Is09(wexename[k])){
					k++;
				}
				if( WCODE::Is09(wexename[k]) ){
					ECodeType n = (ECodeType)_wtoi(&wexename[k]);
					if(IsValidCodeOrCPType(n)){
						m_fi.m_nCharCode = n;
					}
				}
				break;
			}
		}
	}


	TCHAR	szPath[_MAX_PATH];
	bool	bFind = false;				// ファイル名発見フラグ
	bool	bParseOptDisabled = false;	// 2007.09.09 genta オプション解析を行わなず，ファイル名として扱う
	int		nPos;
	int		i = 0;
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
		CSakuraEnvironment::ResolvePath(szPath);
		_tcscpy( m_fi.m_szPath, szPath );	/* ファイル名 */
		nPos = i + 1;
	}else{
		m_fi.m_szPath[0] = _T('\0');
		nPos = 0;
	}

	CNativeT cmResponseFile = _T("");
	LPTSTR pszCmdLineWork = new TCHAR[lstrlen( pszCmdLineSrc ) + 1];
	_tcscpy( pszCmdLineWork, pszCmdLineSrc );
	int nCmdLineWorkLen = lstrlen( pszCmdLineWork );
	LPTSTR pszToken = my_strtok<TCHAR>( pszCmdLineWork, nCmdLineWorkLen, &nPos, _T(" ") );
	while( pszToken != NULL )
	{
		DEBUG_TRACE( _T("OPT=[%ts]\n"), pszToken );

		//	2007.09.09 genta オプション判定ルール変更．オプション解析停止と""で囲まれたオプションを考慮
		if( ( bParseOptDisabled ||
			! (pszToken[0] == '-' || pszToken[0] == '"' && pszToken[1] == '-' ) )){

			if( pszToken[0] == _T('\"') ){
				CNativeT cmWork;
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
					_tcscpy_s( szPath, _countof(szPath), cmWork.GetStringPtr() );	/* ファイル名 */
				}
				else {
					szPath[0] = _T('\0');
				}
			}
			else{
				_tcscpy_s( szPath, _countof(szPath), pszToken );		/* ファイル名 */
			}

			// Nov. 11, 2005 susu
			// 不正なファイル名のままだとファイル保存時ダイアログが出なくなるので
			// 簡単なファイルチェックを行うように修正
			if (_tcsncmp_literal(szPath, _T("file:///"))==0) {
				_tcscpy(szPath, &(szPath[8]));
			}
			int len = _tcslen(szPath);
			for (int i = 0; i < len ; ) {
				if ( !TCODE::IsValidFilenameChar(szPath[i]) ){
					TCHAR msg_str[_MAX_PATH + 1];
					_stprintf(
						msg_str,
						LS(STR_CMDLINE_PARSECMD1),
						szPath
					);
					MessageBox( NULL, msg_str, _T("FileNameError"), MB_OK);
					szPath[0] = _T('\0');
					break;
				}
				int nChars = t_max(1, int(CNativeT::GetCharNext( szPath, len, szPath + i ) - (szPath + i)));
				i += nChars;
			}

			if (szPath[0] != _T('\0')) {
				CSakuraEnvironment::ResolvePath(szPath);
				if (m_fi.m_szPath[0] == _T('\0')) {
					_tcscpy(m_fi.m_szPath, szPath );
				}
				else {
					m_vFiles.push_back( szPath );
				}
			}
		}
		else{
			if( *pszToken == '"' ){
				++pszToken;	// 2007.09.09 genta 先頭の"はスキップ
				int tokenlen = _tcslen( pszToken );
				if( pszToken[ tokenlen-1 ] == '"' ){	// 2009.06.14 syat 末尾の"を取り除く
					pszToken[ tokenlen-1 ] = '\0';
				}
			}
			++pszToken;	//	先頭の'-'はskip
			TCHAR *arg = NULL;
			int nArgLen;
			switch( CheckCommandLine( pszToken, &arg, &nArgLen ) ){
			case CMDLINEOPT_AT:
				cmResponseFile.SetStringT( arg, nArgLen );
				break;
			case CMDLINEOPT_X: //	X
				/* 行桁指定を1開始にした */
				m_fi.m_ptCursor.x = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_Y:	//	Y
				m_fi.m_ptCursor.y = AtoiOptionInt( arg ) - 1;
				break;
			case CMDLINEOPT_VX:	// VX
				/* 行桁指定を1開始にした */
				m_fi.m_nViewLeftCol = CLayoutInt( AtoiOptionInt( arg ) - 1 );
				break;
			case CMDLINEOPT_VY:	//	VY
				/* 行桁指定を1開始にした */
				m_fi.m_nViewTopLine = CLayoutInt( AtoiOptionInt( arg ) - 1 );
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
					m_fi.m_szDocType[ nArgLen < MAX_DOCTYPE_LEN ? nArgLen : MAX_DOCTYPE_LEN ] = L'\0';
				}
				break;
			case CMDLINEOPT_CODE:	//	CODE
				m_fi.m_nCharCode = (ECodeType)AtoiOptionInt( arg );
				break;
			case CMDLINEOPT_R:	//	R
				m_bViewMode = true;
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
					auto_strcpy( m_fi.m_szDocType , _T("grepout") );
				}
				break;
			case CMDLINEOPT_GREPDLG:	//	GREPDLG
				m_bGrepDlg = true;
				break;
			case CMDLINEOPT_GKEY:	//	GKEY
				//	前後の""を取り除く
				m_gi.cmGrepKey.SetStringT( arg,  lstrlen( arg ) );
				m_gi.cmGrepKey.Replace( L"\"\"", L"\"" );
				break;
			case CMDLINEOPT_GREPR:	//	GREPR
				//	前後の""を取り除く
				m_gi.cmGrepRep.SetStringT( arg,  lstrlen( arg ) );
				m_gi.cmGrepRep.Replace( L"\"\"", L"\"" );
				m_gi.bGrepReplace = true;
				break;
			case CMDLINEOPT_GFILE:	//	GFILE
				//	前後の""を取り除く
				m_gi.cmGrepFile.SetStringT( arg,  lstrlen( arg ) );
				m_gi.cmGrepFile.Replace( _T("\"\""), _T("\"") );
				break;
			case CMDLINEOPT_GFOLDER:	//	GFOLDER
				m_gi.cmGrepFolder.SetString( arg,  lstrlen( arg ) );
				m_gi.cmGrepFolder.Replace( _T("\"\""), _T("\"") );
				break;
			case CMDLINEOPT_GOPT:	//	GOPT
				for( ; *arg != '\0' ; ++arg ){
					switch( *arg ){
					case 'X':
						m_gi.bGrepCurFolder = true;	break;
					case 'U':
						m_gi.bGrepStdout = true;	break;
					case 'H':
						m_gi.bGrepHeader = false;	break;
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
						// 結果出力：[行を出力]/該当部分/否マッチ行
						m_gi.nGrepOutputLineType = 1;	break;
					case 'N':
						// 結果出力：行を出力/該当部分/[否マッチ行]
						m_gi.nGrepOutputLineType = 2;	break;
					case 'W':
						// 単語単位で探す
						m_gi.sGrepSearchOption.bWordOnly = true;	break;
					case '1':
						// Grep: 出力形式
						m_gi.nGrepOutputStyle = 1;	break;
					case '2':
						// Grep: 出力形式
						m_gi.nGrepOutputStyle = 2;	break;
					case '3':
						// Grep: 出力形式
						m_gi.nGrepOutputStyle = 3;	break;
					case 'F':
						m_gi.bGrepOutputFileOnly = true;	break;
					case 'B':
						m_gi.bGrepOutputBaseFolder = true;	break;
					case 'D':
						m_gi.bGrepSeparateFolder = true;	break;
					case 'C':
						m_gi.bGrepPaste = true;	break;
					case 'O':
						m_gi.bGrepBackup = true;	break;
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
					auto_strcpy( m_fi.m_szDocType , _T("output") );
				}
				break;
			case CMDLINEOPT_NOMOREOPT:	// 2007.09.09 genta これ以降引数無効
				bParseOptDisabled = true;
				break;
			case CMDLINEOPT_M:			// 2009.06.14 syat 追加
				m_cmMacro.SetStringT( arg );
				m_cmMacro.Replace( L"\"\"", L"\"" );
				break;
			case CMDLINEOPT_MTYPE:		// 2009.06.14 syat 追加
				m_cmMacroType.SetStringT( arg );
				break;
			case CMDLINEOPT_PROF:		// 2013.12.20 Moca 追加
				m_cmProfile.SetStringT( arg );
				m_bSetProfile = true;
				break;
			case CMDLINEOPT_PROFMGR:
				m_bProfileMgr = true;
				break;
			}
		}
		pszToken = my_strtok<TCHAR>( pszCmdLineWork, nCmdLineWorkLen, &nPos, _T(" ") );
	}
	delete [] pszCmdLineWork;

	// レスポンスファイル解析
	if( cmResponseFile.GetStringLength() && bResponse ){
		CTextInputStream input(cmResponseFile.GetStringPtr());
		if( !input.Good() ){
			return;
		}
		std::tstring responseData;
		while(input){
			responseData += to_tchar(input.ReadLineW().c_str());
		}
		ParseCommandLine( responseData.c_str(), false );
	}

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
	m_bProfileMgr			= false;
	m_bSetProfile			= false;
	m_gi.bGrepSubFolder		= false;
	m_gi.sGrepSearchOption.Reset();
	m_gi.bGrepCurFolder		= false;
	m_gi.bGrepStdout		= false;
	m_gi.bGrepHeader		= true;
	m_gi.nGrepCharSet		= CODE_SJIS;
	m_gi.nGrepOutputLineType	= 0;
	m_gi.nGrepOutputStyle	= 1;
	m_gi.bGrepOutputFileOnly = false;
	m_gi.bGrepOutputBaseFolder = false;
	m_gi.bGrepSeparateFolder = false;
	m_gi.bGrepReplace		= false;
	m_gi.bGrepPaste			= false;
	m_gi.bGrepBackup		= false;
	m_bViewMode			= false;
	m_nGroup				= -1;		// 2007.06.26 ryoji
	m_cmProfile.SetString(L"");
}

