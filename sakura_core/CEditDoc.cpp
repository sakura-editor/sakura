//	$Id$
/*!	@file
	@brief 文書関連情報の管理

	@author Norio Nakatani
	@date	1998/03/13 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, mik, jepro
	Copyright (C) 2002, YAZAKI, hor, genta, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditDoc.h"
#include "debug.h"
//#include "keycode.h"
#include "funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include <DLGS.H>
#include "CShareData.h"
#include "CEditWnd.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "global.h"
#include "CFuncInfoArr.h" /// 2002/2/3 aroka
#include "CSMacroMgr.h"///
#include "CMarkMgr.h"///
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CPrintPreview.h"

#define IDT_ROLLMOUSE	1

//	May 12, 2000 genta 初期化方法変更
CEditDoc::CEditDoc() :
	m_cNewLineCode( EOL_CRLF ),		//	New Line Type
	m_cSaveLineCode( EOL_NONE ),		//	保存時のLine Type
	m_bGrepRunning( FALSE ),		/* Grep処理中 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
//	m_bPrintPreviewMode( FALSE ),	/* 印刷プレビューモードか */
	m_nCommandExecNum( 0 ),			/* コマンド実行回数 */
	m_hwndReferer( NULL ),			/* 参照元ウィンドウ */
	m_nRefererX( 0 ),				/* 参照元 行頭からのバイト位置桁 */
	m_nRefererLine( 0 ),			/* 参照元行 折り返し無しの物理行位置 */
	m_bReadOnly( FALSE ),			/* 読み取り専用モード */
	m_bDebugMode( FALSE ),			/* デバッグモニタモード */
	m_bGrepMode( FALSE ),			/* Grepモードか */
	m_nCharCode( 0 ),				/* 文字コード種別 */
	m_nActivePaneIndex( 0 ),
//@@@ 2002.01.14 YAZAKI 不使用のため
//	m_pcOpeBlk( NULL ),				/* 操作ブロック */
	m_bDoing_UndoRedo( FALSE ),		/* アンドゥ・リドゥの実行中か */
	m_nFileShareModeOld( 0 ),		/* ファイルの排他制御モード */
	m_hLockedFile( NULL ),			/* ロックしているファイルのハンドル */
	m_pszAppName( "EditorClient" ),
	m_hInstance( NULL ),
	m_hWnd( NULL ),
	m_nSettingTypeLocked( false ),	//	設定値変更可能フラグ
	m_bIsModified( false )	/* 変更フラグ */ // Jan. 22, 2002 genta 型変更
{
//	m_pcDlgTest = new CDlgTest;

	m_szFilePath[0] = '\0';			/* 現在編集中のファイルのパス */
	strcpy( m_szGrepKey, "" );
	/* 共有データ構造体のアドレスを返す */
//	m_cShareData.Init();

	m_pShareData = CShareData::getInstance()->GetShareData();
	int doctype = CShareData::getInstance()->GetDocumentType( m_szFilePath );
	SetDocumentType( doctype, true );

	/* OPENFILENAMEの初期化 */
	memset( &m_ofn, 0, sizeof( OPENFILENAME ) );
	m_ofn.lStructSize = sizeof( OPENFILENAME );
	m_ofn.nFilterIndex = 3;
	GetCurrentDirectory( _MAX_PATH, m_szInitialDir );	/* 「開く」での初期ディレクトリ */
	strcpy( m_szDefaultWildCard, "*.*" );				/* 「開く」での最初のワイルドカード */
	/* CHOOSEFONTの初期化 */
	memset( &m_cf, 0, sizeof( CHOOSEFONT ) );
	m_cf.lStructSize = sizeof( m_cf );
	m_cf.hwndOwner = m_hWnd;
	m_cf.hDC = NULL;
	m_cf.lpLogFont = &(m_pShareData->m_Common.m_lf);
	m_cf.Flags = CF_FIXEDPITCHONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
//#ifdef _DEBUG
//	m_cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
//#endif


	/* レイアウト管理情報の初期化 */
	m_cLayoutMgr.Create( &m_cDocLineMgr );
	/* レイアウト情報の変更 */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		ref.m_nMaxLineSize,
		ref.m_bWordWrap,			/* 英文ワードラップをする */
		ref.m_nTabSpace,
		ref.m_szLineComment,		/* 行コメントデリミタ */
		ref.m_szLineComment2,		/* 行コメントデリミタ2 */
		ref.m_szLineComment3,		/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
		ref.m_szBlockCommentFrom,	/* ブロックコメントデリミタ(From) */
		ref.m_szBlockCommentTo,		/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		ref.m_szBlockCommentFrom2,	/* ブロックコメントデリミタ2(From) */
		ref.m_szBlockCommentTo2,	/* ブロックコメントデリミタ2(To) */
//#endif
		ref.m_nStringType,			/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		TRUE,
		NULL,/*hwndProgress*/
		ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* シングルクォーテーション文字列を表示する */
		ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
	);
//	MYTRACE( "CEditDoc::CEditDoc()おわり\n" );

	//	Aug, 21, 2000 genta
	//	自動保存の設定
	ReloadAutoSaveParam();

	//	Sep, 29, 2001 genta
	//	マクロ
	m_pcSMacroMgr = new CSMacroMgr;
	//strcpy(m_pszCaption, "sakura");	//@@@	YAZAKI
	
	//	m_FileTimeの初期化
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;
	return;
}


CEditDoc::~CEditDoc()
{
//	delete (CDialog*)m_pcDlgTest;
//	m_pcDlgTest = NULL;

	if( m_hWnd != NULL ){
		DestroyWindow( m_hWnd );
	}
	/* ファイルの排他ロック解除 */
	delete m_pcSMacroMgr;
	DoFileUnLock();
	return;
}





/////////////////////////////////////////////////////////////////////////////
//
//	CEditDoc::Create
//	BOOL Create(HINSTANCE hInstance, HWND hwndParent)
//
//	説明
//	  ウィンドウの作成等
//
/////////////////////////////////////////////////////////////////////////////
BOOL CEditDoc::Create(
	HINSTANCE hInstance,
	HWND hwndParent,
	CImageListMgr* pcIcons
 )
{
	HWND		hWndArr[4];
	CEditWnd*	pCEditWnd;
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

	/* 分割フレーム作成 */
	pCEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
	m_cSplitterWnd.Create( m_hInstance, m_hwndParent, pCEditWnd );

	/* ビュー */
	m_cEditViewArr[0].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 0, /*FALSE,*/ TRUE  );
	m_cEditViewArr[1].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 1, /*TRUE ,*/ FALSE );
	m_cEditViewArr[2].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 2, /*TRUE ,*/ FALSE );
	m_cEditViewArr[3].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 3, /*TRUE ,*/ FALSE );

	m_cEditViewArr[0].OnKillFocus();
	m_cEditViewArr[1].OnKillFocus();
	m_cEditViewArr[2].OnKillFocus();
	m_cEditViewArr[3].OnKillFocus();

	m_cEditViewArr[0].OnSetFocus();

	/* 子ウィンドウの設定 */
	hWndArr[0] = m_cEditViewArr[0].m_hWnd;
	hWndArr[1] = m_cEditViewArr[1].m_hWnd;
	hWndArr[2] = m_cEditViewArr[2].m_hWnd;
	hWndArr[3] = m_cEditViewArr[3].m_hWnd;
	m_cSplitterWnd.SetChildWndArr( hWndArr );
	m_hWnd = m_cSplitterWnd.m_hWnd;


	//	Oct. 2, 2001 genta
	m_cFuncLookup.Init( m_hInstance, m_pcSMacroMgr, &m_pShareData->m_Common );

	/* 設定プロパティシートの初期化１ */
//@@	m_cProp1.Create( m_hInstance, m_hWnd );
	//	Sep. 29, 2001 genta マクロクラスを渡すように
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	m_cPropCommon.Create( m_hInstance, m_hWnd, pcIcons, m_pcSMacroMgr, &(pCEditWnd->m_CMenuDrawer) );
	m_cPropTypes.Create( m_hInstance, m_hWnd );

	/* 入力補完ウィンドウ作成 */
	m_cHokanMgr.DoModeless( m_hInstance, m_cEditViewArr[0].m_hWnd, (LPARAM)&(m_cEditViewArr[0]) );

	return TRUE;
}





/*
|| メッセージディスパッチャ
*/
LRESULT CEditDoc::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	switch( uMsg ){
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
		m_cEditViewArr[0].DispatchEvent( hwnd, uMsg, wParam, lParam );
		m_cEditViewArr[1].DispatchEvent( hwnd, uMsg, wParam, lParam );
		m_cEditViewArr[2].DispatchEvent( hwnd, uMsg, wParam, lParam );
		m_cEditViewArr[3].DispatchEvent( hwnd, uMsg, wParam, lParam );
		return 0L;
	default:
		return m_cEditViewArr[m_nActivePaneIndex].DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
}



void CEditDoc::OnMove( int x, int y, int nWidth, int nHeight )
{
//	m_cSplitterWnd.OnMove( x, y, nWidth, nHeight );
	::MoveWindow( m_cSplitterWnd.m_hWnd, x, y, nWidth, nHeight, TRUE );

	return;
}




/*! テキストが選択されているか */
BOOL CEditDoc::IsTextSelected( void )
{
	return m_cEditViewArr[m_nActivePaneIndex].IsTextSelected();
}




BOOL CEditDoc::SelectFont( LOGFONT* plf )
{
	m_cf.hwndOwner = m_hWnd;
	m_cf.lpLogFont = plf;
	if( TRUE != ChooseFont( &m_cf ) ){
#ifdef _DEBUG
		DWORD nErr;
		nErr = CommDlgExtendedError();
		switch( nErr ){
		case CDERR_FINDRESFAILURE:	MYTRACE( "CDERR_FINDRESFAILURE \n" );	break;
		case CDERR_INITIALIZATION:	MYTRACE( "CDERR_INITIALIZATION \n" );	break;
		case CDERR_LOCKRESFAILURE:	MYTRACE( "CDERR_LOCKRESFAILURE \n" );	break;
		case CDERR_LOADRESFAILURE:	MYTRACE( "CDERR_LOADRESFAILURE \n" );	break;
		case CDERR_LOADSTRFAILURE:	MYTRACE( "CDERR_LOADSTRFAILURE \n" );	break;
		case CDERR_MEMALLOCFAILURE:	MYTRACE( "CDERR_MEMALLOCFAILURE\n" );	break;
		case CDERR_MEMLOCKFAILURE:	MYTRACE( "CDERR_MEMLOCKFAILURE \n" );	break;
		case CDERR_NOHINSTANCE:		MYTRACE( "CDERR_NOHINSTANCE \n" );		break;
		case CDERR_NOHOOK:			MYTRACE( "CDERR_NOHOOK \n" );			break;
		case CDERR_NOTEMPLATE:		MYTRACE( "CDERR_NOTEMPLATE \n" );		break;
		case CDERR_STRUCTSIZE:		MYTRACE( "CDERR_STRUCTSIZE \n" );		break;
		case CFERR_MAXLESSTHANMIN:	MYTRACE( "CFERR_MAXLESSTHANMIN \n" );	break;
		case CFERR_NOFONTS:			MYTRACE( "CFERR_NOFONTS \n" );			break;
		}
#endif
		return FALSE;
	}else{
//		MYTRACE( "LOGFONT.lfPitchAndFamily = " );
//		if( plf->lfPitchAndFamily & DEFAULT_PITCH ){
//			MYTRACE( "DEFAULT_PITCH " );
//		}
//		if( plf->lfPitchAndFamily & FIXED_PITCH ){
//			MYTRACE( "FIXED_PITCH " );
//		}
//		if( plf->lfPitchAndFamily & VARIABLE_PITCH ){
//			MYTRACE( "VARIABLE_PITCH " );
//		}
//		if( plf->lfPitchAndFamily & FF_DECORATIVE  ){
//			MYTRACE( "FF_DECORATIVE " );
//		}
//		if( plf->lfPitchAndFamily & FF_DONTCARE ){
//			MYTRACE( "FF_DONTCARE " );
//		}
//		if( plf->lfPitchAndFamily & FF_MODERN ){
//			MYTRACE( "FF_MODERN " );
//		}
//		if( plf->lfPitchAndFamily & FF_ROMAN ){
//			MYTRACE( "FF_ROMAN " );
//		}
//		if( plf->lfPitchAndFamily & FF_SCRIPT ){
//			MYTRACE( "FF_SCRIPT " );
//		}
//		if( plf->lfPitchAndFamily & FF_SWISS ){
//			MYTRACE( "FF_SWISS " );
//		}
//		MYTRACE( "\n" );

//		MYTRACE( "/* LOGFONTの初期化 */\n" );
//		MYTRACE( "memset( &m_pShareData->m_Common.m_lf, 0, sizeof(LOGFONT) );\n" );
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfHeight			= %d;\n", m_pShareData->m_Common.m_lf.lfHeight			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfWidth			= %d;\n", m_pShareData->m_Common.m_lf.lfWidth			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfEscapement		= %d;\n", m_pShareData->m_Common.m_lf.lfEscapement		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfOrientation		= %d;\n", m_pShareData->m_Common.m_lf.lfOrientation		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfWeight			= %d;\n", m_pShareData->m_Common.m_lf.lfWeight			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfItalic			= %d;\n", m_pShareData->m_Common.m_lf.lfItalic			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfUnderline		= %d;\n", m_pShareData->m_Common.m_lf.lfUnderline		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfStrikeOut		= %d;\n", m_pShareData->m_Common.m_lf.lfStrikeOut		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfCharSet			= %d;\n", m_pShareData->m_Common.m_lf.lfCharSet			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfOutPrecision	= %d;\n", m_pShareData->m_Common.m_lf.lfOutPrecision	);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfClipPrecision	= %d;\n", m_pShareData->m_Common.m_lf.lfClipPrecision	);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfQuality			= %d;\n", m_pShareData->m_Common.m_lf.lfQuality			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfPitchAndFamily	= %d;\n", m_pShareData->m_Common.m_lf.lfPitchAndFamily	);
//		MYTRACE( "strcpy( m_pShareData->m_Common.m_lf.lfFaceName, \"%s\" );\n", m_pShareData->m_Common.m_lf.lfFaceName	);

	}

	return TRUE;
}




/*! ファイルを開く */
BOOL CEditDoc::FileRead(
	char*	pszPath,	//!< [in/out]
	BOOL*	pbOpened,
	int		nCharCode,			/*!< [in] 文字コード自動判別 */
	BOOL	bReadOnly,			/*!< [in] 読み取り専用か */
	BOOL	bConfirmCodeChange	/*!< [in] 文字コード変更時の確認をするかどうか */
)
{
	int				i;
	HWND			hWndOwner;
	BOOL			bRet;
	FileInfo		fi;
	FileInfo*		pfi;
	HWND			hwndProgress;
	CWaitCursor		cWaitCursor( m_hWnd );
	BOOL			bIsExistInMRU;
	int				nRet;
	BOOL			bFileIsExist;
	int				doctype;

	m_bReadOnly = bReadOnly;	/* 読み取り専用モード */

//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	CMRU			cMRU;

	/* ファイルの存在チェック */
	bFileIsExist = FALSE;
	if( -1 == _access( pszPath, 0 ) ){
	}else{
		HANDLE			hFind;
		WIN32_FIND_DATA	w32fd;
		hFind = ::FindFirstFile( pszPath, &w32fd );
		::FindClose( hFind );
//? 2000.01.18 システム属性のファイルが開けない問題
//?		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
//?		}else{
			bFileIsExist = TRUE;
//?		}
		/* フォルダが指定された場合 */
		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			/* 指定フォルダで「開くダイアログ」を表示 */
			{
				char*		pszPathNew = new char[_MAX_PATH];
//				int			nCharCode;
//				BOOL		bReadOnly;

				strcpy( pszPathNew, "" );

				/* 「ファイルを開く」ダイアログ */
				nCharCode = CODE_AUTODETECT;	/* 文字コード自動判別 */
				bReadOnly = FALSE;
//				::ShowWindow( m_hWnd, SW_SHOW );
				if( !OpenFileDialog( m_hWnd, pszPath, pszPathNew, &nCharCode, &bReadOnly ) ){
					delete [] pszPathNew;
					return FALSE;
				}
				strcpy( pszPath, pszPathNew );
				delete [] pszPathNew;
				if( -1 == _access( pszPath, 0 ) ){
					bFileIsExist = FALSE;
				}else{
					bFileIsExist = TRUE;
				}
			}
		}

	}




	CEditWnd* pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}
	*pbOpened = FALSE;
	bRet = TRUE;
	if( NULL == pszPath ){
		MYMESSAGEBOX(
			m_hWnd,
			MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
			"バグじゃぁあああ！！！",
			"CEditDoc::FileRead()\n\nNULL == pszPath\n【対処】エラーの出た状況を作者に連絡してくださいね。"
		);
		return FALSE;
	}

	/* 指定ファイルが開かれているか調べる */
	if( CShareData::getInstance()->IsPathOpened( pszPath, &hWndOwner ) ){
		::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
//		pfi = (FileInfo*)m_pShareData->m_szWork;
		pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

		/* アクティブにする */
		ActivateFrameWindow( hWndOwner );

		*pbOpened = TRUE;
		/* MRUリストへの登録 */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
		cMRU.Add( pfi );

		bRet = FALSE;
		goto end_of_func;
	}
	for( i = 0; i < 4; ++i ){
		if( m_cEditViewArr[i].IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			m_cEditViewArr[i].DisableSelectArea( TRUE );
		}
	}

	strcpy( m_szFilePath, pszPath ); /* 現在編集中のファイルのパス */


	/* 指定された文字コード種別に変更する */
	//	Oct. 25, 2000 genta
	//	文字コードとして異常な値が設定された場合の対応
	//	-1以上CODE_MAX未満のみ受け付ける
	//	Oct. 26, 2000 genta
	//	CODE_AUTODETECTはこの範囲から外れているから個別にチェック
	if( ( -1 <= nCharCode && nCharCode < CODE_CODEMAX ) || nCharCode == CODE_AUTODETECT )
		m_nCharCode = nCharCode;

	/* MRUリストに存在するか調べる  存在するならばファイル情報を返す */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	if ( cMRU.GetFileInfo( pszPath, &fi ) ){
		bIsExistInMRU = TRUE;

//		m_cDlgJump.m_bPLSQL = fi.m_bPLSQL;			/* 行ジャンプが PL/SQLモードか */
//		m_cDlgJump.m_nPLSQL_E1 = fi.m_nPLSQL_E1;	/* 行ジャンプが PL/SQLモードのときの基点 */

		if( -1 == m_nCharCode ){
			/* 前回に指定された文字コード種別に変更する */
			m_nCharCode = fi.m_nCharCode;
		}
		/* ファイルが存在しない */
		if( FALSE == bFileIsExist &&
			CODE_AUTODETECT == m_nCharCode	/* 文字コード自動判別 */
		){
			m_nCharCode = 0;
		}
		if( CODE_AUTODETECT == m_nCharCode ){	/* 文字コード自動判別 */
			/*
			|| ファイルの日本語コードセット判別
			||
			|| 【戻り値】
			||	SJIS	0
			||	JIS		1
			||	EUC		2
			||	Unicode	3
			||	エラー	-1
			*/
			m_nCharCode = CMemory::CheckKanjiCodeOfFile( pszPath );
			if( -1 == m_nCharCode ){
				::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
					"%s\n文字コードの判別処理でエラーが発生しました。",
					pszPath
				);
				strcpy( m_szFilePath, "" );
				bRet = FALSE;
				goto end_of_func;
			}
		}
		if( m_nCharCode != fi.m_nCharCode ){
			if( bConfirmCodeChange ){
				char*	pszCodeName = NULL;
				char*	pszCodeNameNew = NULL;
				switch( fi.m_nCharCode ){
				case CODE_SJIS:		/* SJIS */		pszCodeName = "SJIS";break;	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
				case CODE_JIS:		/* JIS */		pszCodeName = "JIS";break;
				case CODE_EUC:		/* EUC */		pszCodeName = "EUC";break;
				case CODE_UNICODE:	/* Unicode */	pszCodeName = "Unicode";break;
				case CODE_UTF8:		/* UTF-8 */		pszCodeName = "UTF-8";break;
				case CODE_UTF7:		/* UTF-7 */		pszCodeName = "UTF-7";break;
				}
				switch( m_nCharCode ){
				case CODE_SJIS:		/* SJIS */		pszCodeNameNew = "SJIS";break;	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
				case CODE_JIS:		/* JIS */		pszCodeNameNew = "JIS";break;
				case CODE_EUC:		/* EUC */		pszCodeNameNew = "EUC";break;
				case CODE_UNICODE:	/* Unicode */	pszCodeNameNew = "Unicode";break;
				case CODE_UTF8:		/* UTF-8 */		pszCodeNameNew = "UTF-8";break;
				case CODE_UTF7:		/* UTF-7 */		pszCodeNameNew = "UTF-7";break;
				}
				if( pszCodeName != NULL ){
					::MessageBeep( MB_ICONQUESTION );
					nRet = MYMESSAGEBOX(
						m_hWnd,
						MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
						"文字コード情報",
						"%s\n\nこのファイルは、前回は別の文字コード %s で開かれています。\n前回と同じ文字コードを使いますか？\n\n・[はい(Y)]  ＝%s\n・[いいえ(N)]＝%s\n・[キャンセル]＝開きません",
						m_szFilePath, pszCodeName, pszCodeName, pszCodeNameNew
					);
					if( IDYES == nRet ){
						/* 前回に指定された文字コード種別に変更する */
						m_nCharCode = fi.m_nCharCode;
					}else
					if( IDCANCEL == nRet ){
						m_nCharCode = 0;
						strcpy( m_szFilePath, "" );
						bRet = FALSE;
						goto end_of_func;
					}
				}else{
					MYMESSAGEBOX(
						m_hWnd,
						MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
						"バグじゃぁあああ！！！",
						"【対処】エラーの出た状況を作者に連絡してください。"
					);
					strcpy( m_szFilePath, "" );
					bRet = FALSE;
					goto end_of_func;
				}
			}
		}
	}else{
		bIsExistInMRU = FALSE;
		/* ファイルが存在しない */
		if( FALSE == bFileIsExist &&
			CODE_AUTODETECT == m_nCharCode		/* 文字コード自動判別 */
		){
			m_nCharCode = 0;
		}
		if( CODE_AUTODETECT == m_nCharCode ){	/* 文字コード自動判別 */
			/*
			|| ファイルの日本語コードセット判別
			||
			|| 【戻り値】
			||	SJIS	0
			||	JIS		1
			||	EUC		2
			||	Unicode	3
			||	エラー	-1
			*/
			m_nCharCode = CMemory::CheckKanjiCodeOfFile( pszPath );
			if( -1 == m_nCharCode ){
				::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
					"%s\n文字コードの判別処理でエラーが発生しました。",
					pszPath
				);
				strcpy( m_szFilePath, "" );
				bRet = FALSE;
				goto end_of_func;
			}
		}
	}
	if( -1 == m_nCharCode ){
		m_nCharCode = 0;
	}

//		if( (_access( pszPath, 0 )) == -1 ){
//			::MYMESSAGEBOX(
//				m_hwndParent,
//				MB_OK | MB_ICONSTOP | MB_TOPMOST,
//				GSTR_APPNAME,
//				"\'%s\'\nファイルは存在しません。 新規に作成します。",
//				pszPath
//			);
//
//			strcpy( m_szFilePath, pszPath ); /* 現在編集中のファイルのパス */
//			m_nCharCode = CODE_SJIS;
//
//			return TRUE;
//		}

	//	Nov. 12, 2000 genta ロングファイル名の取得を前方に移動
	char szWork[MAX_PATH];
	/* ロングファイル名を取得する */
	if( TRUE == ::GetLongFileName( pszPath, szWork ) ){
		strcpy( m_szFilePath, szWork );
	}

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
	doctype = CShareData::getInstance()->GetDocumentType( m_szFilePath );
	SetDocumentType( doctype, true );

	/* ファイルが存在しない */
	if( FALSE == bFileIsExist ){
//		::MessageBeep( MB_ICONINFORMATION );

		::MYMESSAGEBOX(
			m_hwndParent,
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
			GSTR_APPNAME,
//			"\'%s\'\nファイルは存在しません。 ファイルを保存したときに、ディスク上にファイルが作成されます。",
			"%s\nというファイルは存在しません。\n\nファイルを保存したときに、ディスク上にこのファイルが作成されます。",	//Mar. 24, 2001 jepro 若干修正
			pszPath
		);

//		::MessageBeep( MB_ICONHAND );
//		::MYMESSAGEBOX( hwndParent, MB_OK | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME,
//			"'%s'\nファイルが存在しません。",  pszPath
//
//		);
	}else{
		/* ファイルを読む */
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}
		if( FALSE == m_cDocLineMgr.ReadFile( m_szFilePath, m_hWnd, hwndProgress,
			m_nCharCode, &m_FileTime, m_pShareData->m_Common.GetAutoMIMEdecode() ) ){
			strcpy( m_szFilePath, "" );
			bRet = FALSE;
			goto end_of_func;
		}
//#ifdef _DEBUG
//		m_cDocLineMgr.DUMP();
//#endif

	}

	/* レイアウト情報の変更 */
	{
		Types& ref = GetDocumentAttribute();
		m_cLayoutMgr.SetLayoutInfo(
			ref.m_nMaxLineSize,
			ref.m_bWordWrap,			/* 英文ワードラップをする */
			ref.m_nTabSpace,
			ref.m_szLineComment,		/* 行コメントデリミタ */
			ref.m_szLineComment2,		/* 行コメントデリミタ2 */
			ref.m_szLineComment3,		/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
			ref.m_szBlockCommentFrom,	/* ブロックコメントデリミタ(From) */
			ref.m_szBlockCommentTo,		/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
			ref.m_szBlockCommentFrom2,	/* ブロックコメントデリミタ2(From) */
			ref.m_szBlockCommentTo2,	/* ブロックコメントデリミタ2(To) */
//#endif
			ref.m_nStringType,			/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
			TRUE,
			hwndProgress,
			ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* シングルクォーテーション文字列を表示する */
			ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
		);
	}

	/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */
	InitAllView();

	//	Nov. 20, 2000 genta
	//	IME状態の設定
	SetImeMode( GetDocumentAttribute().m_nImeState );

	if( bIsExistInMRU && m_pShareData->m_Common.GetRestoreCurPosition() ){
//#ifdef _DEBUG
//	if( FALSE == m_bDebugMode ){
//		m_cShareData.TraceOut( "bIsExistInMRU==TRUE [%s] fi.m_nX=%d, fi.m_nY=%d\n", fi.m_szPath, fi.m_nX, fi.m_nY );
//	}
//#endif
		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		int		nCaretPosX;
		int		nCaretPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			fi.m_nX,
			fi.m_nY,
			&nCaretPosX,
			&nCaretPosY
		);
		if( nCaretPosY >= m_cLayoutMgr.GetLineCount() ){
			/*ファイルの最後に移動 */
//			m_cEditViewArr[m_nActivePaneIndex].Command_GOFILEEND(FALSE);
			m_cEditViewArr[m_nActivePaneIndex].HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
		}else{
			m_cEditViewArr[m_nActivePaneIndex].m_nViewTopLine = fi.m_nViewTopLine; // 2001/10/20 novice
			m_cEditViewArr[m_nActivePaneIndex].m_nViewLeftCol = fi.m_nViewLeftCol; // 2001/10/20 novice
			m_cEditViewArr[m_nActivePaneIndex].MoveCursor( nCaretPosX, nCaretPosY, TRUE );
			m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX_Prev =
				m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;
		}
	}
	// 2002.01.16 hor ブックマーク復元
	if( bIsExistInMRU ){
		if( m_pShareData->m_Common.GetRestoreBookmarks() ){
			m_cDocLineMgr.SetBookMarks(fi.m_szMarkLines);
		}
	}else{
		strcpy(fi.m_szMarkLines,"");
	}
	SetFileInfo( &fi );

	//	May 12, 2000 genta
	//	改行コードの設定
	//	May 12, 2000 genta
	{
		SetNewLineCode( EOL_CRLF );
		CDocLine*	pFirstlineinfo = m_cDocLineMgr.GetLineInfo( 0 );
		if( pFirstlineinfo != NULL ){
			enumEOLType t = (enumEOLType)pFirstlineinfo->m_cEol;
			if( t != EOL_NONE && t != EOL_UNKNOWN )
				SetNewLineCode( t );
		}
	}

	/* MRUリストへの登録 */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	cMRU.Add( &fi );

end_of_func:;
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	if( TRUE == bRet && 0 < lstrlen( m_szFilePath ) ){
		/* ファイルの排他ロック */
		DoFileLock();
	}
	return bRet;
}


/*!	@brief ファイルの保存
	
	@param pszPath [in] 保存ファイル名
	@param cEolType [in] 改行コード種別
	
	pszPathはNULLであってはならない。
	
	@date Feb. 9, 2001 genta 改行コード用引数追加
*/
BOOL CEditDoc::FileWrite( const char* pszPath, enumEOLType cEolType )
{
	BOOL		bRet;
	FileInfo	fi;
	HWND		hwndProgress;
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	CMRU		cMRU;
	//	Feb. 9, 2001 genta
	CEOL	cEol( cEolType );

	if( m_bReadOnly ){	/* 読み取り専用モード */
		::MessageBeep( MB_ICONHAND );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONSTOP | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\n\nは読み取り専用モードで開いています。 上書き保存はできません。\n\n名前を付けて保存をすればいいと思います。",
			lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）"
		);
		return FALSE;
	}


	bRet = TRUE;

	CEditWnd* pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}


	/* ファイルの排他ロック解除 */
	DoFileUnLock();

	if( m_pShareData->m_Common.m_bBackUp ){	/* バックアップの作成 */
		MakeBackUp();
	}

	CWaitCursor cWaitCursor( m_hWnd );
	if( FALSE == m_cDocLineMgr.WriteFile( pszPath, m_hWnd, hwndProgress, m_nCharCode, &m_FileTime, cEol ) ){
		bRet = FALSE;
		goto end_of_func;
	}
	
	/* ロングファイル名を取得する。（上書き保存のときのみ） */
	char szWork[MAX_PATH];
	if( TRUE == ::GetLongFileName( m_szFilePath, szWork ) ){
		strcpy( m_szFilePath, szWork );
	}

	int	v;
	for( v = 0; v < 4; ++v ){
		if( m_nActivePaneIndex != v ){
			m_cEditViewArr[v].RedrawAll();
		}
	}
	m_cEditViewArr[m_nActivePaneIndex].RedrawAll();

	strcpy( m_szFilePath, pszPath ); /* 現在編集中のファイルのパス */

	SetModified(false,false);	//	Jan. 22, 2002 genta 関数化 更新フラグのクリア

	SetFileInfo( &fi );

	/* MRUリストへの登録 */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	cMRU.Add( &fi );

	/* 現在位置で無変更な状態になったことを通知 */
	m_cOpeBuf.SetNoModified();

	m_bReadOnly = FALSE;	/* 読み取り専用モード */

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();
end_of_func:;

	if( 0 < lstrlen( m_szFilePath ) &&
		FALSE == m_bReadOnly && /* 読み取り専用モード ではない */
		TRUE == bRet
	){
		/* ファイルの排他ロック */
		DoFileLock();
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}

	return bRet;
}



/* 「ファイルを開く」ダイアログ */
BOOL CEditDoc::OpenFileDialog(
	HWND		hwndParent,
	const char*	pszOpenFolder,	//NULL以外を指定すると初期フォルダを指定できる
	char*		pszPath,		//開くファイルのパスを受け取るアドレス
	int*		pnCharCode,		//指定された文字コード種別を受け取るアドレス
	BOOL*		pbReadOnly		//読み取り専用か
)
{
	/* アクティブにする */
	ActivateFrameWindow( hwndParent );
	ActivateFrameWindow( hwndParent );

//	int		i;
//	int		j;
	char**	ppszMRU;
	char**	ppszOPENFOLDER;

	/* MRUリストのファイルのリスト */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new char*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);


	/* OPENFOLDERリストのファイルのリスト */
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* ファイルオープンダイアログの初期化 */
	if( 0 == lstrlen( m_szFilePath ) ){
		if( NULL == pszOpenFolder ){
			m_cDlgOpenFile.Create(
				m_hInstance,
				/*NULL*//*m_hWnd*/hwndParent,
				m_szDefaultWildCard,
				ppszMRU[0],
				(const char **)ppszMRU,
				(const char **)ppszOPENFOLDER
			);
		}else{
			char*	pszFolderNew = new char[MAX_PATH];
			int		nDummy;
			int		nCharChars;
			strcpy( pszFolderNew, pszOpenFolder );
			nDummy = lstrlen( pszFolderNew );
			/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
			nCharChars = &pszFolderNew[nDummy] - CMemory::MemCharPrev( pszFolderNew, nDummy, &pszFolderNew[nDummy] );
			if( 1 == nCharChars && pszFolderNew[nDummy - 1] == '\\' ){
			}else{
				strcat( pszFolderNew, "\\" );
			}


			m_cDlgOpenFile.Create(
				m_hInstance,
				/*NULL*//*m_hWnd*/hwndParent,
				m_szDefaultWildCard,
				pszFolderNew/*pszOpenFolder*/,
				(const char **)ppszMRU,
				(const char **)ppszOPENFOLDER
			);
			delete [] pszFolderNew;
		}
	}else{
		if( NULL == pszOpenFolder ){
			m_cDlgOpenFile.Create(
				m_hInstance,
				/*NULL*//*m_hWnd*/hwndParent,
				m_szDefaultWildCard,
				m_szFilePath,
				(const char **)ppszMRU,
				(const char **)ppszOPENFOLDER
			);
		}else{
			char*	pszFolderNew = new char[MAX_PATH];
			int		nDummy;
			int		nCharChars;
			strcpy( pszFolderNew, pszOpenFolder );
			nDummy = lstrlen( pszFolderNew );
			/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
			nCharChars = &pszFolderNew[nDummy] - CMemory::MemCharPrev( pszFolderNew, nDummy, &pszFolderNew[nDummy] );
			if( 1 == nCharChars && pszFolderNew[nDummy - 1] == '\\' ){
			}else{
				strcat( pszFolderNew, "\\" );
			}

			m_cDlgOpenFile.Create(
				m_hInstance,
				/*NULL*//*m_hWnd*/hwndParent,
				m_szDefaultWildCard,
				pszFolderNew/*pszOpenFolder*/,
				(const char **)ppszMRU,
				(const char **)ppszOPENFOLDER
			);
			delete [] pszFolderNew;
		}
	}
	if( m_cDlgOpenFile.DoModalOpenDlg( pszPath, pnCharCode, pbReadOnly ) ){
		delete [] ppszMRU;
		delete [] ppszOPENFOLDER;
		return TRUE;
	}else{
		delete [] ppszMRU;
		delete [] ppszOPENFOLDER;
		return FALSE;
	}
}



//pszOpenFolder pszOpenFolder


/* 「ファイル名を付けて保存」ダイアログ

	ファイル名をつけて保存ダイアログを表示して、
	　pszPath：保存ファイル名
	　pnCharCode：保存文字コードセット
	　pcEol：保存改行コード
	を取得する。
*/
//	Feb. 9, 2001 genta	改行コードを示す引数追加
BOOL CEditDoc::SaveFileDialog( char* pszPath, int* pnCharCode, CEOL* pcEol )
{
	char**	ppszMRU;		//	最近のファイル
	char**	ppszOPENFOLDER;	//	最近のフォルダ
	BOOL	bret;

	/* MRUリストのファイルのリスト */
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new char*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);

	/* OPENFOLDERリストのファイルのリスト */
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* ファイル保存ダイアログの初期化 */
	/* ファイル名の無いファイルだったら、ppszMRU[0]をデフォルトファイル名として？ppszOPENFOLDERじゃない？ */
	if( 0 == lstrlen( m_szFilePath ) ){
		m_cDlgOpenFile.Create( m_hInstance, /*NULL*/m_hWnd, m_szDefaultWildCard, ppszMRU[0], (const char **)ppszMRU, (const char **)ppszOPENFOLDER );
	}else{
		m_cDlgOpenFile.Create( m_hInstance, /*NULL*/m_hWnd, m_szDefaultWildCard, m_szFilePath, (const char **)ppszMRU, (const char **)ppszOPENFOLDER );
	}

	/* ダイアログを表示 */
	bret = m_cDlgOpenFile.DoModalSaveDlg( pszPath, pnCharCode, pcEol );

	delete [] ppszMRU;
	delete [] ppszOPENFOLDER;
	return bret;
}





/*! 共通設定 プロパティシート */
BOOL CEditDoc::OpenPropertySheet( int nPageNum/*, int nActiveItem*/ )
{
	int		i;
//	BOOL	bModify;
	m_cPropCommon.m_Common = m_pShareData->m_Common;
	m_cPropCommon.m_nKeyNameArrNum = m_pShareData->m_nKeyNameArrNum;
	for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
		m_cPropCommon.m_pKeyNameArr[i] = m_pShareData->m_pKeyNameArr[i];
	}
	m_cPropCommon.m_CKeyWordSetMgr = m_pShareData->m_CKeyWordSetMgr;
	for( i = 0; i < MAX_TYPES; ++i ){
		m_cPropCommon.m_Types[i] = m_pShareData->m_Types[i];
	}
	/* マクロ関係
	@@@ 2002.01.03 YAZAKI 共通設定『マクロ』がタブを切り替えるだけで設定が保存されないように。
	*/
	for( i = 0; i < MAX_CUSTMACRO; ++i ){
		m_cPropCommon.m_MacroTable[i] = m_pShareData->m_MacroTable[i];
	}
	strcpy(m_cPropCommon.m_szMACROFOLDER, m_pShareData->m_szMACROFOLDER);

	/* プロパティシートの作成 */
	if( m_cPropCommon.DoPropertySheet( nPageNum/*, nActiveItem*/ ) ){
//		/* 変更されたか？ */
//		if( 0 != memcmp( m_pShareData->m_pKeyNameArr, m_cPropCommon.m_pKeyNameArr, sizeof( m_pShareData->m_pKeyNameArr ) ) ){
//			m_pShareData->m_bKeyBindModify = TRUE;	/* 変更フラグ キー割り当て */
			for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
//				if( 0 != memcmp( &m_cPropCommon.m_pKeyNameArr[i], &m_pShareData->m_pKeyNameArr[i], sizeof( m_cPropCommon.m_pKeyNameArr[i] ) ) ){
					m_pShareData->m_pKeyNameArr[i] = m_cPropCommon.m_pKeyNameArr[i];
//					m_pShareData->m_bKeyBindModifyArr[i] = TRUE;	/* 変更フラグ キー割り当て(キーごと) */
//				}
			}
//		}
//		/* 変更状況を調査 */
//		bModify = m_pShareData->m_CKeyWordSetMgr.IsModify(
//			m_cPropCommon.m_CKeyWordSetMgr,
//			&m_pShareData->m_bKeyWordSetModifyArr[0]
//		);
//		if( bModify ){
//			m_pShareData->m_bKeyWordSetModify = TRUE;
			m_pShareData->m_CKeyWordSetMgr = m_cPropCommon.m_CKeyWordSetMgr;
//		}

//		/* 変更フラグ(共通設定の全体) のセット */
//		if( 0 != memcmp( &m_pShareData->m_Common, &m_cPropCommon.m_Common, sizeof( Common ) ) ){
//			/* 変更フラグ(共通設定の全体) のセット */
//			m_pShareData->m_nCommonModify = TRUE;
			m_pShareData->m_Common = m_cPropCommon.m_Common;
//		}else{
//		}

//		/* 共通設定とキーワード設定が無変更の場合は、なにもしない */
//		if( FALSE == m_pShareData->m_nCommonModify
//		 && FALSE == m_pShareData->m_bKeyWordSetModify
//		){
//			return FALSE;
//		}

		for( i = 0; i < MAX_TYPES; ++i ){
//			/* 変更されたか？ */
//			if( 0 != memcmp( &m_pShareData->m_Types[i], &m_cPropCommon.m_Types[i], sizeof( Types ) ) ){
				/* 変更された設定値のコピー */
				m_pShareData->m_Types[i] = m_cPropCommon.m_Types[i];
//				/* 変更フラグ(タイプ別設定) のセット */
//				m_pShareData->m_nTypesModifyArr[i] = TRUE;
//			}
		}

		/* マクロ関係 */
		for( i = 0; i < MAX_CUSTMACRO; ++i ){
			m_pShareData->m_MacroTable[i] = m_cPropCommon.m_MacroTable[i];
		}
		strcpy(m_pShareData->m_szMACROFOLDER, m_cPropCommon.m_szMACROFOLDER);

		/* アクセラレータテーブルの再作成 */
		::SendMessage( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* 設定変更を反映させる */
		CShareData::getInstance()->SendMessageToAllEditors( MYWM_CHANGESETTING, (WPARAM)0, (LPARAM)0, m_hwndParent );	/* 全編集ウィンドウへメッセージをポストする */

		return TRUE;
	}else{
		return FALSE;
	}
}



/*! タイプ別設定 プロパティシート */
BOOL CEditDoc::OpenPropertySheetTypes( int nPageNum, int nSettingType )
{
	m_cPropTypes.m_Types = m_pShareData->m_Types[nSettingType];
	m_cPropTypes.m_CKeyWordSetMgr = m_pShareData->m_CKeyWordSetMgr;

	/* プロパティシートの作成 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){
//		/* 変更されたか？ */
//		if( 0 == memcmp( &m_pShareData->m_Types[nSettingType], &m_cPropTypes.m_Types, sizeof( Types ) ) ){
//			/* 無変更 */
//			return FALSE;
//		}
//		/* 変更フラグ(タイプ別設定) のセット */
//		m_pShareData->m_nTypesModifyArr[nSettingType] = TRUE;
		/* 変更された設定値のコピー */
		m_pShareData->m_Types[nSettingType] = m_cPropTypes.m_Types;

//		/* 折り返し桁数が変更された */
//		if( m_cPropTypes.m_nMaxLineSize_org != m_cPropTypes.m_Types.m_nMaxLineSize){
//			/*アンドゥ・リドゥバッファのクリア */
//			/* 全要素のクリア */
//			m_cOpeBuf.ClearAll();
//		}
		/* アクセラレータテーブルの再作成 */
		::SendMessage( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* 設定変更を反映させる */
		CShareData::getInstance()->SendMessageToAllEditors( MYWM_CHANGESETTING, (WPARAM)0, (LPARAM)0, m_hwndParent );	/* 全編集ウィンドウへメッセージをポストする */

		return TRUE;
	}else{
		return FALSE;
	}
}



/* Undo(元に戻す)可能な状態か？ */
BOOL CEditDoc::IsEnableUndo( void )
{
	return m_cOpeBuf.IsEnableUndo();
}



/*! Redo(やり直し)可能な状態か？ */
BOOL CEditDoc::IsEnableRedo( void )
{
	return m_cOpeBuf.IsEnableRedo();
}




/*! クリップボードから貼り付け可能か？ */
BOOL CEditDoc::IsEnablePaste( void )
{
	UINT uFormatSakuraClip;
	uFormatSakuraClip = ::RegisterClipboardFormat( "SAKURAClip" );
	if( ::IsClipboardFormatAvailable( CF_OEMTEXT )
	 || ::IsClipboardFormatAvailable( uFormatSakuraClip )
	){
		return TRUE;
	}
	return FALSE;
}





/*! 親ウィンドウのタイトルを更新

	@param bKillFocus [in] true: Activeの表示 / false: Inactiveの表示
*/
void CEditDoc::SetParentCaption( BOOL bKillFocus )
{
	if( NULL == m_hWnd ){
		return;
	}
	if( !m_cEditViewArr[m_nActivePaneIndex].m_bDrawSWITCH ){
		return;
	}


	HWND	hwnd;
	char	pszCap[1024];	//	Nov. 6, 2000 genta オーバーヘッド軽減のためHeap→Stackに変更

//	/* アイコン化されていない時はフルパス */
//	/* アイコン化されている時はファイル名のみ */
//	if( ::IsIconic( m_hWnd ) ){
//		bKillFocus = TRUE;
//	}else{
//		bKillFocus = FALSE;
//	}


	const char*	pszAppName = GSTR_APPNAME;
	char*		pszMode;
	char*		pszKeyMacroRecking;


	hwnd = m_hwndParent;

	if( m_bReadOnly ){	/* 読み取り専用モード */
		pszMode = "（読み取り専用）";
	}else
	if( 0 != m_nFileShareModeOld && /* ファイルの排他制御モード */
		NULL == m_hLockedFile		/* ロックしていない */
	){
		pszMode = "（上書き禁止）";
	}else{
		pszMode = "";
	}

	if( TRUE == m_pShareData->m_bRecordingKeyMacro &&	/* キーボードマクロの記録中 */
		m_pShareData->m_hwndRecordingKeyMacro == hwnd	/* キーボードマクロを記録中のウィンドウ */
	){
		pszKeyMacroRecking = "  【キーマクロの記録中】";
	}else{
		pszKeyMacroRecking = "";
	}



	if( m_bGrepMode ){
		/* データを指定バイト数以内に切り詰める */
		CMemory		cmemDes;
		int			nDesLen;
		const char*	pszDes;
		LimitStringLengthB( m_szGrepKey, lstrlen( m_szGrepKey ), 64, cmemDes );
		pszDes = cmemDes.GetPtr( NULL );
		nDesLen = lstrlen( pszDes );
//		wsprintf( pszCap, "【Grep】\"%s%s\" - %s",
		wsprintf( pszCap, "%s%s - %s",
			pszDes, ( (int)lstrlen( m_szGrepKey ) > nDesLen ) ? "・・・":"",
			pszAppName
		);
//#ifdef _DEBUG
	}else
	if( m_bDebugMode ){
		wsprintf( pszCap, "アウトプット - %s%s",
			pszAppName,
			m_bReadOnly ? "（上書き禁止）" : ""	/* 読み取り専用モード */
		 );
//#endif
	}else{

		if( 0 < lstrlen( m_szFilePath ) && (::IsIconic( hwnd ) || bKillFocus ) ){
			char szFname[_MAX_FNAME];
			char szExt[_MAX_EXT];
			_splitpath( m_szFilePath, NULL, NULL, szFname, szExt );
			//Oct. 11, 2000 jepro note： アクティブでない時のタイトル表示
			wsprintf(
				pszCap,
				"%s%s%s - %s %d.%d.%d.%d %s%s",	//Jul. 06, 2001 jepro UR はもう付けなくなったのを忘れていた
				szFname, szExt,
				IsModified() ? "（更新）" : "",	/* 変更フラグ */
				pszAppName,
				HIWORD( m_pShareData->m_dwProductVersionMS ),
				LOWORD( m_pShareData->m_dwProductVersionMS ),
				HIWORD( m_pShareData->m_dwProductVersionLS ),
				LOWORD( m_pShareData->m_dwProductVersionLS ),
				pszMode,	/* モード */
				pszKeyMacroRecking
			);
		}else{

			//Oct. 11, 2000 jepro note： アクティブな時のタイトル表示
			wsprintf(
				pszCap,
				"%s%s - %s %d.%d.%d.%d %s%s",		//Jul. 06, 2001 jepro UR はもう付けなくなったのを忘れていた
				lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）",
				IsModified() ? "（更新）" : "",	/* 変更フラグ */
				pszAppName,
				HIWORD( m_pShareData->m_dwProductVersionMS ),
				LOWORD( m_pShareData->m_dwProductVersionMS ),
				HIWORD( m_pShareData->m_dwProductVersionLS ),
				LOWORD( m_pShareData->m_dwProductVersionLS ),
				pszMode,	/* モード */
				pszKeyMacroRecking
			);
		}
	}
	// delete [] pszCap;
	//if (strcmp( m_pszCaption, pszCap ) != 0){
		::SetWindowText( hwnd, pszCap );
	//	strcpy( m_pszCaption, pszCap );
	//}
	return;
}




/*! バックアップの作成
	@author genta
	@date 2001.06.12 asa-o
		ファイルの時刻を元にバックアップファイル名を作成する機能
	@date 2001.12.11 MIK バックアップファイルをゴミ箱に入れる機能
	
*/
BOOL CEditDoc::MakeBackUp( void )
{
	time_t	ltime;
	struct	tm *today, *gmt, xmas = { 0, 0, 12, 25, 11, 93 };
	char	szTime[64];
	char	szForm[64];
	char	szPath[_MAX_PATH];
	char	szDrive[_MAX_DIR];
	char	szDir[_MAX_DIR];
	char	szFname[_MAX_FNAME];
	char	szExt[_MAX_EXT];
//	int		nLen;
	int		nRet;
	char*	pBase;

	/* ファイル名が付いているか */
	if( 0 >= lstrlen( m_szFilePath ) ){
		return FALSE;
	}

	/* バックアップソースの存在チェック */
	if( (_access( m_szFilePath, 0 )) == -1 ){
		return FALSE;
	}

	/* パスの分解 */
	_splitpath( m_szFilePath, szDrive, szDir, szFname, szExt );

	if( m_pShareData->m_Common.m_bBackUpFolder ){	/* 指定フォルダにバックアップを作成する */
		strcpy( szPath, m_pShareData->m_Common.m_szBackUpFolder );
		/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
		AddLastYenFromDirectoryPath( szPath );
	}
	else{
		wsprintf( szPath, "%s%s", szDrive, szDir );
	}
	pBase = szPath + strlen( szPath );

	/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
	switch( m_pShareData->m_Common.GetBackupType() ){
	case 1:
		wsprintf( pBase, "%s%s", szFname, ".bak" );
		break;
	case 2:	//	日付，時刻
		_tzset();
		_strdate( szTime );
		time( &ltime );				/* システム時刻を得ます */
		gmt = gmtime( &ltime );		/* 万国標準時に変換する */
		today = localtime( &ltime );/* 現地時間に変換する */

		strcpy( szForm, "" );
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_YEAR) ){	/* バックアップファイル名：日付の年 */
			strcat( szForm, "%Y" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_MONTH) ){	/* バックアップファイル名：日付の月 */
			strcat( szForm, "%m" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_DAY) ){	/* バックアップファイル名：日付の日 */
			strcat( szForm, "%d" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_HOUR) ){	/* バックアップファイル名：日付の時 */
			strcat( szForm, "%H" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_MIN) ){	/* バックアップファイル名：日付の分 */
			strcat( szForm, "%M" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_SEC) ){	/* バックアップファイル名：日付の秒 */
			strcat( szForm, "%S" );
		}
		/* YYYYMMDD時分秒 形式に変換 */
		strftime( szTime, sizeof( szTime ) - 1, szForm, today );
		wsprintf( pBase, "%s_%s%s", szFname, szTime, szExt );
		break;
//	2001/06/12 Start by asa-o: ファイルに付ける日付を前回の保存時(更新日時)にする
	case 4:	//	日付，時刻
		{
			HANDLE		hFile;
			FILETIME	LastWriteTime,
						LocalTime;
			SYSTEMTIME	SystemTime;

			hFile = ::CreateFile(m_szFilePath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			::GetFileTime(hFile,NULL,NULL,&LastWriteTime);			// ファイルのタイプスタンプを取得(更新日時のみ)
			CloseHandle(hFile);
			::FileTimeToLocalFileTime(&LastWriteTime,&LocalTime);	// 現地時刻に変換
			::FileTimeToSystemTime(&LocalTime,&SystemTime);			// システムタイムに変換

			strcpy( szTime, "" );
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_YEAR) ){	/* バックアップファイル名：日付の年 */
				wsprintf(szTime,"%d",SystemTime.wYear);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_MONTH) ){	/* バックアップファイル名：日付の月 */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wMonth);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_DAY) ){	/* バックアップファイル名：日付の日 */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wDay);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_HOUR) ){	/* バックアップファイル名：日付の時 */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wHour);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_MIN) ){	/* バックアップファイル名：日付の分 */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wMinute);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_SEC) ){	/* バックアップファイル名：日付の秒 */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wSecond);
			}
			wsprintf( pBase, "%s_%s%s", szFname, szTime, szExt );
		}
		break;
// 2001/06/12 End

	case 3: //	?xx : xx = 00~99, ?は任意の文字
		//	Aug. 15, 2000 genta
		//	ここでは作成するバックアップファイル名のみ生成する．
		//	ファイル名のRotationは確認ダイアログの後で行う．
		szExt[0] = '.';
		szExt[1] = m_pShareData->m_Common.GetBackupExtChar();
		szExt[2] = '0';
		szExt[3] = '0';
		szExt[4] = '\0';

		wsprintf( pBase, "%s%s", szFname, szExt );
		break;
	}

	if( m_pShareData->m_Common.m_bBackUpDialog ){	/* バックアップの作成前に確認 */
		::MessageBeep( MB_ICONQUESTION );
//From Here Feb. 27, 2001 JEPROtest キャンセルもできるようにし、メッセージを追加した
//		if( IDYES != MYMESSAGEBOX(
//			m_hWnd,
//			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
//			"バックアップ作成の確認",
//			"変更される前に、バックアップファイルを作成します。\nよろしいですか？\n\n%s\n    ↓\n%s\n\n",
//			lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）",
//			szPath
//		) ){
//			return FALSE;
//		}
		if( m_pShareData->m_Common.m_bBackUpDustBox ){	//@@@ 2001.12.11 add start MIK
			nRet = ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				"バックアップ作成の確認",
				"変更される前に、バックアップファイルを作成します。\nよろしいですか？  [いいえ(N)] を選ぶと作成せずに上書き（または名前を付けて）保存になります。\n\n%s\n    ↓\n%s\n\n作成したバックアップファイルをごみ箱に放り込みます。\n",
				lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）",
				szPath
			);
		}else{	//@@@ 2001.12.11 add end MIK
			nRet = ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				"バックアップ作成の確認",
				"変更される前に、バックアップファイルを作成します。\nよろしいですか？  [いいえ(N)] を選ぶと作成せずに上書き（または名前を付けて）保存になります。\n\n%s\n    ↓\n%s\n\n",
				lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）",
				szPath
			);	//Jul. 06, 2001 jepro [名前を付けて保存] の場合もあるのでメッセージを修正
		}	//@@@ 2001.12.11 add MIK
		if( IDNO == nRet ){
			return FALSE;
		}else if( IDCANCEL == nRet ){
			return FALSE;// FALSE ではダメですが何を返せばいいのかわかりません。。。
		}
//To Here Feb. 27, 2001
	}

	//	From Here Aug. 16, 2000 genta
	if( m_pShareData->m_Common.GetBackupType() == 3 ){
		//	既に存在するBackupをずらす処理
		int				i;

		//	ファイル検索用
		HANDLE			hFind;
		WIN32_FIND_DATA	fData;

		pBase = pBase + strlen( pBase ) - 2;	//	2: 拡張子の最後の2桁の意味
		//::MessageBox( NULL, pBase, "書き換え場所", MB_OK );

		//------------------------------------------------------------------
		//	1. 該当ディレクトリ中のbackupファイルを1つずつ探す
		for( i = 0; i <= 99; i++ ){	//	最大値に関わらず，99（2桁の最大値）まで探す
			//	ファイル名をセット
			wsprintf( pBase, "%02d", i );

			hFind = ::FindFirstFile( szPath, &fData );
			if( hFind == INVALID_HANDLE_VALUE ){
				//	検索に失敗した == ファイルは存在しない
				break;
			}
			::FindClose( hFind );
			//	見つかったファイルの属性をチェック
			//	は面倒くさいからしない．
			//	同じ名前のディレクトリがあったらどうなるのだろう...
		}
		--i;

		//------------------------------------------------------------------
		//	2. 最大値から制限数-1番までを削除
		int boundary = m_pShareData->m_Common.GetBackupCount();
		boundary = boundary > 0 ? boundary - 1 : 0;	//	最小値は0
		//::MessageBox( NULL, pBase, "書き換え場所", MB_OK );

		for( ; i >= boundary; --i ){
			//	ファイル名をセット
			wsprintf( pBase, "%02d", i );
			if( ::DeleteFile( szPath ) == 0 ){
				::MessageBox( m_hWnd, szPath, "削除失敗", MB_OK );
				return FALSE;
				//	失敗した場合
				//	後で考える
			}
		}

		//	この位置でiは存在するバックアップファイルの最大番号を表している．

		//	3. そこから0番まではコピーしながら移動
		char szNewPath[MAX_PATH];
		char *pNewNrBase;

		strcpy( szNewPath, szPath );
		pNewNrBase = szNewPath + strlen( szNewPath ) - 2;

		for( ; i >= 0; --i ){
			//	ファイル名をセット
			wsprintf( pBase, "%02d", i );
			wsprintf( pNewNrBase, "%02d", i + 1 );

			//	ファイルの移動
			if( ::MoveFile( szPath, szNewPath ) == 0 ){
				//	失敗した場合
				//	後で考える
				::MessageBox( m_hWnd, szPath, "移動失敗", MB_OK );
				return FALSE;
			}
		}
	}
	//	To Here Aug. 16, 2000 genta

	//::MessageBox( NULL, szPath, "直前のバックアップファイル", MB_OK );
	/* バックアップの作成 */
	if( ::CopyFile( m_szFilePath, szPath, FALSE ) ){
		/* 正常終了 */
		//@@@ 2001.12.11 start MIK
		if( m_pShareData->m_Common.m_bBackUpDustBox ){
			char	szDustPath[_MAX_PATH+1];
			strcpy(szDustPath, szPath);
			szDustPath[strlen(szDustPath) + 1] = '\0';
			SHFILEOPSTRUCT	fos;
			fos.hwnd   = m_hWnd;
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDustPath;
			fos.pTo    = NULL;
			fos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;	//ダイアログなし
			//fos.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY;
			//fos.fFlags = FOF_ALLOWUNDO;	//ダイアログが表示される。
			fos.fAnyOperationsAborted = true; //false;
			fos.hNameMappings = NULL;
			fos.lpszProgressTitle = NULL; //"バックアップファイルをごみ箱に移動しています...";
			if( ::SHFileOperation(&fos) == 0 ){
				/* 正常終了 */
			}else{
				/* エラー終了 */
			}
		}
		//@@@ 2001.12.11 end MIK
	}else{
		/* エラー終了 */
	}
	return TRUE;
}




/* ファイルの排他ロック */
void CEditDoc::DoFileLock( void )
{
	char*	pszMode;
	int		nAccessMode;
	BOOL	bCheckOnly;

	/* ロックしている */
	if( NULL != m_hLockedFile ){
		/* ロック解除 */
		::_lclose( m_hLockedFile );
		m_hLockedFile = NULL;
	}

	/* ファイルが存在しない */
	if( -1 == _access( m_szFilePath, 0 ) ){
		/* ファイルの排他制御モード */
		m_nFileShareModeOld = 0;
		return;
	}else{
		/* ファイルの排他制御モード */
		m_nFileShareModeOld = m_pShareData->m_Common.m_nFileShareMode;
	}


	/* ファイルを開いていない */
	if( 0 == lstrlen( m_szFilePath ) ){
		return;
	}
	/* 読み取り専用モード */
	if( TRUE == m_bReadOnly ){
		return;
	}


	nAccessMode = 0;
	if( m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_DENY_WRITE ||
		m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_EXCLUSIVE ){
		bCheckOnly = FALSE;
	}else{
		/* 排他制御しないけどロックされているかのチェックは行うのでreturnしない */
//		return;
		bCheckOnly = TRUE;
	}
	/* 書込み禁止かどうか調べる */
	if( -1 == _access( m_szFilePath, 2 ) ){	/* アクセス権：書き込み許可 */
#if 0
		// Apr. 28, 2000 genta: Request from Koda

		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"現在\n%s\nは読取専用に設定されています。 または、書き込みのアクセス権がありません。",
			lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）"
		);
#endif
		m_hLockedFile = NULL;
		/* 親ウィンドウのタイトルを更新 */
		SetParentCaption();
		return;
	}


	m_hLockedFile = ::_lopen( m_szFilePath, OF_READWRITE );
	_lclose( m_hLockedFile );
	if( HFILE_ERROR == m_hLockedFile ){
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\nは現在他のプロセスによって書込みが禁止されています。",
			lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）"
		);
		m_hLockedFile = NULL;
		/* 親ウィンドウのタイトルを更新 */
		SetParentCaption();
		return;
	}
	m_hLockedFile = ::_lopen( m_szFilePath, nAccessMode | m_pShareData->m_Common.m_nFileShareMode );
	if( HFILE_ERROR == m_hLockedFile ){
		switch( m_pShareData->m_Common.m_nFileShareMode ){
		case OF_SHARE_EXCLUSIVE:	/* 読み書き */
			pszMode = "読み書き禁止モード";
			break;
		case OF_SHARE_DENY_WRITE:	/* 書き */
			pszMode = "書き込み禁止モード";
			break;
		}
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\nを%sでロックできませんでした。\n現在このファイルに対する排他制御は無効となります。",
			lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）",
			pszMode
		);
		/* 親ウィンドウのタイトルを更新 */
		SetParentCaption();
		return;
	}
	/* 排他制御しないけどロックされているかのチェックは行う場合 */
	if( bCheckOnly ){
		/* ロックを解除する */
		DoFileUnLock();

	}
	return;
}


/* ファイルの排他ロック解除 */
void CEditDoc::DoFileUnLock( void )
{
	if( NULL != m_hLockedFile ){
		/* ロック解除 */
		::_lclose( m_hLockedFile );
		m_hLockedFile = NULL;
		/* ファイルの排他制御モード */
		m_nFileShareModeOld = 0;
	}
	return;
}

//	Mar. 15, 2000 genta
//	From Here
/*!
	スペースの判定
*/
inline bool C_IsSpace( char c ){
	return ('\t' == c ||
			 ' ' == c ||
			  CR == c ||
			  LF == c
	);
}

/*!
	関数に用いることができる文字かどうかの判定
*/
inline bool C_IsWordChar( char c ){
	return ( '_' == c ||
			 ':' == c ||
			 '~' == c ||
			('a' <= c && c <= 'z' )||
			('A' <= c && c <= 'Z' )||
			('0' <= c && c <= '9' )
		);
}
//	To Here

//	From Here Apr. 1, 2001 genta
/*!
	特殊な関数名 "operator" かどうかを判定する。

	文字列が"operator"それ自身か、あるいは::の後ろにoperatorと続いて
	終わっているときにoperatorと判定。

	演算子の評価順序を保証するため2つのif文に分けてある

	@param szStr 判定対象の文字列
	@param nLen 文字列の長さ。
	本質的には不要であるが、高速化のために既にある値を利用する。
*/
bool C_IsOperator( char* szStr, int nLen	)
{
	if( nLen >= 8 && szStr[ nLen - 1 ] == 'r' ){
		if( nLen > 8 ?
				strcmp( szStr + nLen - 9, ":operator" ) == 0 :	// メンバー関数による定義
				strcmp( szStr, "operator" ) == 0	// friend関数による定義
		 ){
		 	return true;
		}
	}
	return false;
}
//	To Here Apr. 1, 2001 genta


/*!
	@brief C/C++関数リスト作成

	@par MODE一覧
	- 0	通常
	- 20	Single quotation文字列読み込み中
	- 21	Double quotation文字列読み込み中
	- 8	コメント読み込み中
	- 1	単語読み込み中
	- 2	記号列読み込み中
	- 999	長過ぎる単語無視中

	@par FuncIdの値の意味
	10の位で目的別に使い分けている．C/C++用は10位が0
	- 1: 宣言
	- 2: 通常の関数 (追加文字列無し)

	@param pcFuncInfoArr [out] 関数一覧を返すためのクラス。
	ここに関数のリストを登録する。
*/
void CEditDoc::MakeFuncList_C( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nNestLevel;			//	nNestLevel	{}のレベル
	int			nNestLevel2;		//	nNestLevel2	()に対する位置
	int			nCharChars;			//	多バイト文字を読み飛ばすためのもの
	char		szWordPrev[256];	//	1つ前のword
	char		szWord[256];		//	現在解読中のwordを入れるところ
	int			nWordIdx = 0;
	int			nMaxWordLeng = 100;	//	許容されるwordの最大長さ
	int			nMode;				//	現在のstate
	char		szFuncName[256];	//	関数名
	int			nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	//	Mar. 4, 2001 genta
	bool		bLineTop;			//	行頭かどうかを判別するためのフラグ
									//	プリプロセッサ指令用
	//	Mar. 4, 2001 genta
	bool		bCppInitSkip;		//	C++のメンバー変数、親クラスの初期化子をSKIP
	nNestLevel = 0;
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	nNestLevel2 = 0;
	nFuncNum = 0;
	bCppInitSkip = false;
//	for( nLineCount = 0; nLineCount <  m_cLayoutMgr.GetLineCount(); ++nLineCount ){
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
//		pLine = m_cLayoutMgr.GetLineStr( nLineCount, &nLineLen );
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		//	Mar. 4, 2001 genta
		bLineTop = true;
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 1 < nCharChars ){
				i += (nCharChars - 1);
				bLineTop = false;
				continue;
			}
			/* エスケープシーケンスは常に取り除く */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* ダブルクォーテーション文字列読み込み中 */
			if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( C_IsWordChar( pLine[i] ) ){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					//	From Here Mar. 31, 2001 genta
					//	operatorキーワード(演算子overload)の対応
					//	ただし、operatorキーワードの後ろにスペースが入っているとうまく動かない。
					if( C_IsOperator( szWord, nWordIdx + 1 ) ){
						//	operatorだ！
						/*  overloadする演算子一覧
							& && &=
							| || |=
							+ ++ +=
							- -- -= -> ->*
							* *=
							/ /=
							% %=
							^ ^=
							! !=
							= ==
							< <= << <<=
							> >= >> >>=
							()
							[]
							~
							,
						*/
						int oplen = 0;	// 演算子本体部の文字列長
						switch( pLine[i] ){
						case '&': // no break
						case '|': // no break
						case '+':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == pLine[ i ] ||
									pLine[ i + 1 ] == '=' )
									oplen = 2;
							}
							break;
						case '-':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == '-' ||
									pLine[ i + 1 ] == '=' )
									oplen = 2;
								else if( pLine[ i + 1 ] == '>' ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == '*' )
											oplen = 3;
									}
								}
							}
							break;
						case '*': // no break
						case '/': // no break
						case '%': // no break
						case '^': // no break
						case '!': // no break
						case '=':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == '=' )
									oplen = 2;
							}
							break;
						case '<': // no break
						case '>':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == pLine[ i ] ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == '=' )
											oplen = 3;
									}
								}
								else if( pLine[ i + 1 ] == '=' )
									oplen = 2;
							}
							break;
						case '(':
							if( i + 1 < nLineLen )
								if( pLine[ i + 1 ] == /* 括弧対応対策 ( */ ')' )
									oplen = 2;
							break;
						case '[':
							if( i + 1 < nLineLen )
								if( pLine[ i + 1 ] == /* 括弧対応対策 [ */ ']' )
									oplen = 2;
							break;
						case '~': // no break
						case ',':
							oplen = 2;
							break;
						}

						//	oplen の長さだけキーワードに追加
						for( ; oplen > 0 ; oplen--, i++ ){
							++nWordIdx;
							szWord[nWordIdx] = pLine[i];
						}
						szWord[nWordIdx + 1] = '\0';
							// 記号列の処理を行う前は記号列のiは記号列の先頭を指していた。
							// この時点でiは記号列の1つ後を指している

							// operatorの後ろに不正な文字がある場合の動作
							// ( で始まる場合はoperatorという関数と認識される
							// それ以外の記号だと従来通り記号列がglobalのしたに現れる。

							// 演算子が抜けている場合の動作
							// 引数部が()の場合はそれが演算子と見なされるため、その行は関数定義と認識されない
							// それ以外の場合はoperatorという関数と認識される
					}
					//	To Here Mar. 31, 2001 genta
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
				if( C_IsWordChar( pLine[i] ) ||
					C_IsSpace( pLine[i] ) ||
					 '{' == pLine[i] ||
					 '}' == pLine[i] ||
					 '(' == pLine[i] ||
					 ')' == pLine[i] ||
					 ';' == pLine[i] ||
					'\'' == pLine[i] ||
					 '"' == pLine[i] ||
					 '/' == pLine[i]
				){
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}else{

					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( C_IsSpace( pLine[i] ) ){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( C_IsSpace( pLine[i] ) )
					continue;
				//	Mar 4, 2001 genta
				//	プリプロセッサ指令は無視する
				if( bLineTop && '#' == pLine[i] )
					break;
				bLineTop = false;
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '/' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = 21;
					continue;
				}else
				if( '{' == pLine[i] ){
					if( 2 == nNestLevel2 ){
						//	閉じ括弧)の後で{ すなわち関数の始まり
						if( 0 != strcmp( "sizeof", szFuncName ) ){
							nFuncId = 2;
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
						}
					}
					nNestLevel2 = 0;
					bCppInitSkip = false;	//	Mar. 4, 2001 genta
					++nNestLevel;
					nMode = 0;
					continue;
				}else
				if( '}' == pLine[i] ){
					nNestLevel2 = 0;
					nNestLevel--;
					nMode = 0;
					continue;
				}else
				if( '(' == pLine[i] ){
					if( nNestLevel == 0 && !bCppInitSkip ){
						strcpy( szFuncName, szWordPrev );
						nFuncLine = nLineCount + 1;
						nNestLevel2 = 1;
					}
					nMode = 0;
					continue;
				}else
				if( ')' == pLine[i] ){
					if( 1 == nNestLevel2 ){
						nNestLevel2 = 2;
					}
					nMode = 0;
					continue;
				}else
				if( ';' == pLine[i] ){
					if( 2 == nNestLevel2 ){
						//	閉じ括弧')'の後の';' すなわち関数宣言
						if( 0 != strcmp( "sizeof", szFuncName ) ){
							nFuncId = 1;
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szFuncName, nFuncId );
//							pcFuncInfoArr->AppendData( nFuncLine, szFuncName, nFuncId );
						}
					}
					nNestLevel2 = 0;
					nMode = 0;
					continue;
				}else{
					if( C_IsWordChar( pLine[i] ) ){
						if( 2 == nNestLevel2 ){
							//	閉じ括弧が無いけどとりあえず登録しちゃう
							if( 0 != strcmp( "sizeof", szFuncName ) ){
								nFuncId = 2;
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
								*/
								int		nPosX;
								int		nPosY;
								m_cLayoutMgr.CaretPos_Phys2Log(
									0,
									nFuncLine - 1,
									&nPosX,
									&nPosY
								);
								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
							}
							nNestLevel2 = 0;
							//	Mar 4, 2001 genta	初期化子だったときはそれ以降の登録を制限する
							if( pLine[i] == ':' )
								bCppInitSkip = true;
						}

						//	//	Mar. 15, 2000 genta
						//	From Here
						//	直前のwordの最後が::か，あるいは直後のwordの先頭が::なら
						//	クラス限定子と考えて両者を接続する．

						{
							int pos = strlen( szWordPrev ) - 2;
							if( //	前の文字列の末尾チェック
								( pos > 0 &&	szWordPrev[pos] == ':' &&
								szWordPrev[pos + 1] == ':' ) ||
								//	次の文字列の先頭チェック
								( i < nLineLen - 1 && pLine[i] == ':' &&
									pLine[i+1] == ':' )
							){
								//	前の文字列に続ける
								strcpy( szWord, szWordPrev );
								nWordIdx = strlen( szWord );
							}
							//	From Here Apr. 1, 2001 genta
							//	operator new/delete 演算子の対応
							else if( C_IsOperator( szWordPrev, pos + 2 ) ){
								//	スペースを入れて、前の文字列に続ける
								szWordPrev[pos + 2] = ' ';
								szWordPrev[pos + 3] = '\0';
								strcpy( szWord, szWordPrev );
								nWordIdx = strlen( szWord );
							}
							//	To Here Apr. 1, 2001 genta
							else{
								nWordIdx = 0;
							}
						}
						//	strcpy( szWordPrev, szWord );	不要？
						//	To Here

						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						//	//	Mar. 15, 2000 genta
						//	From Here
						//	長さチェックは必須
						if( nWordIdx < nMaxWordLeng ){
							nMode = 1;
						}
						else{
							nMode = 999;
						}
						//	To Here
					}else{
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						nMode = 2;
					}
				}
			}
		}
	}
	return;
}




/*! PL/SQL関数リスト作成 */
void CEditDoc::MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	char		szFuncName[80];
	int			nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;

	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	nFuncNum = 0;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
//			if( 1 < nCharChars ){
//				i += (nCharChars - 1);
//				continue;
//			}
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					if( i + 1 < nLineLen && '\'' == pLine[i + 1] ){
						++i;
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i + 1 < nLineLen && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					'_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )
					) )
				 || 2 == nCharChars
				){
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}else{
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "FUNCTION" ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;

					}else
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "PROCEDURE" ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "PACKAGE" ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else
					if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == _stricmp( szWord, "BODY" ) ){
						nFuncOrProc = 4;
						nParseCnt = 1;
					}else
					if( 1 == nParseCnt ){
						if( 1 == nFuncOrProc ||
							2 == nFuncOrProc ||
							3 == nFuncOrProc ||
							4 == nFuncOrProc ){
							++nParseCnt;
							strcpy( szFuncName, szWord );
						}else
						if( 3 == nFuncOrProc ){

						}
					}else
					if( 2 == nParseCnt ){
						if( 0 == _stricmp( szWord, "IS" ) ){
							if( 1 == nFuncOrProc ){
								nFuncId = 11;	/* ファンクション本体 */
							}else
							if( 2 == nFuncOrProc ){
								nFuncId = 21;	/* プロシージャ本体 */
							}else
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* パッケージ仕様部 */
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* パッケージ本体 */
							}
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szFuncName, nFuncId );
							nParseCnt = 0;
						}
						if( 0 == _stricmp( szWord, "AS" ) ){
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* パッケージ仕様部 */
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
								*/
								int		nPosX;
								int		nPosY;
								m_cLayoutMgr.CaretPos_Phys2Log(
									0,
									nFuncLine - 1,
									&nPosX,
									&nPosY
								);
								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
								nParseCnt = 0;
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* パッケージ本体 */
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
								*/
								int		nPosX;
								int		nPosY;
								m_cLayoutMgr.CaretPos_Phys2Log(
									0,
									nFuncLine - 1,
									&nPosX,
									&nPosY
								);
								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
								nParseCnt = 0;
							}
						}
					}
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
				if( '_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i] ||
					 '{' == pLine[i] ||
					 '}' == pLine[i] ||
					 '(' == pLine[i] ||
					 ')' == pLine[i] ||
					 ';' == pLine[i] ||
					'\'' == pLine[i] ||
					 '/' == pLine[i] ||
					 '-' == pLine[i]
				){
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}else{
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && '-' == pLine[i] &&  '-' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( ';' == pLine[i] ){
					if( 2 == nParseCnt ){
						if( 1 == nFuncOrProc ){
							nFuncId = 10;	/* ファンクション宣言 */
						}else{
							nFuncId = 20;	/* プロシージャ宣言 */
						}
						++nFuncNum;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nFuncLine - 1,
							&nPosX,
							&nPosY
						);
						pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
						nParseCnt = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( (1 == nCharChars && (
						'_' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )
						) )
					 || 2 == nCharChars
					){
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}





/* テキスト・トピックリスト作成 */
void CEditDoc::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						nCharChars;
	int						nCharChars2;
	char*					pszStarts;
	int						nStartsLen;
	char*					pszText;

	pszStarts = m_pShareData->m_Common.m_szMidashiKigou; 	/* 見出し記号 */
	nStartsLen = lstrlen( pszStarts );

//	for( nLineCount = 0; nLineCount <  m_cLayoutMgr.GetLineCount(); ++nLineCount ){
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
//		pLine = (const unsigned char *)m_cLayoutMgr.GetLineStr( nLineCount, &nLineLen );
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == ' ' ||
				pLine[i] == '\t'){
				continue;
			}else
			if( i + 1 < nLineLen && pLine[i] == 0x81 && pLine[i + 1] == 0x40 ){
				++i;
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}
		nCharChars = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
		for( j = 0; j < nStartsLen; j+=nCharChars2 ){
			nCharChars2 = CMemory::MemCharNext( pszStarts, nStartsLen, &pszStarts[j] ) - &pszStarts[j];
			if( nCharChars == nCharChars2 ){
				if( 0 == memcmp( &pLine[i], &pszStarts[j], nCharChars ) ){
					break;
				}
			}
		}
		if( j >= nStartsLen ){
			continue;
		}
		if( pLine[i] == '(' ){
			if( ( pLine[i + 1] >= '0' && pLine[i + 1] <= '9' ) ||
				( pLine[i + 1] >= 'A' && pLine[i + 1] <= 'Z' ) ||
				( pLine[i + 1] >= 'a' && pLine[i + 1] <= 'z' )
			){
			}else{
				continue;
			}
		}
		pszText = new char[nLineLen + 1];
		memcpy( pszText, (const char *)&pLine[i], nLineLen );
		pszText[nLineLen] = '\0';
		for( i = 0; i < (int)lstrlen(pszText); ++i ){
			if( pszText[i] == CR ||
				pszText[i] == LF ){
				pszText[i] = '\0';
			}
		}
//		MYTRACE( "pszText=[%s]\n", pszText );
		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		int		nPosX;
		int		nPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			0,
			nLineCount,
			&nPosX,
			&nPosY
		);
		pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)pszText, 0 );
//		pcFuncInfoArr->AppendData( nLineCount + 1, (char *)pszText, 0 );
		delete [] pszText;
	}
	return;
}




/*! COBOL アウトライン解析 */
void CEditDoc::MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
//	int						j;
//	int						nCharChars;
//	int						nCharChars2;
//	int						nStartsLen;
//	char*					pszText;
	int						k;
//	int						m;
	char					szDivision[1024];
	char					szLabel[1024];
	const char*				pszKeyWord;
	int						nKeyWordLen;
	BOOL					bDivision;

	szDivision[0] = '\0';
	szLabel[0] =  '\0';


	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		/* コメント行か */
		if( 7 <= nLineLen && pLine[6] == '*' ){
			continue;
		}
		/* ラベル行か */
		if( 8 <= nLineLen && pLine[7] != ' ' ){
			k = 0;
			for( i = 7; i < nLineLen; ){
				if( pLine[i] == '.'
				 || pLine[i] == CR
				 || pLine[i] == LF
				){
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if( pLine[i - 1] == ' ' ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != ' ' ){
							break;
						}
					}
				}
			}
			szLabel[k] = '\0';
//			MYTRACE( "szLabel=[%s]\n", szLabel );



			pszKeyWord = "division";
			nKeyWordLen = lstrlen( pszKeyWord );
			bDivision = FALSE;
			for( i = 0; i <= (int)lstrlen( szLabel ) - nKeyWordLen; ++i ){
				if( 0 == memicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					szLabel[i + nKeyWordLen] = '\0';
					strcpy( szDivision, szLabel );
					bDivision = TRUE;
					break;
				}
			}
			if( bDivision ){
				continue;
			}
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/

			int		nPosX;
			int		nPosY;
			char	szWork[1024];
			m_cLayoutMgr.CaretPos_Phys2Log(
				0,
				nLineCount,
				&nPosX,
				&nPosY
			);
			wsprintf( szWork, "%s::%s", szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)szWork, 0 );
		}
	}
	return;
}




/*! アセンブラ アウトライン解析 */
void CEditDoc::MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nNestLevel;
//	int			nNestLevel2;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
//	char		szFuncName[80];
//	int			nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	char		szProcName[1024];
	BOOL		bProcReading;

	nNestLevel = 0;
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	nFuncNum = 0;
	szProcName[0] = '\0';
	bProcReading = FALSE;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 1 < nCharChars ){
				i += (nCharChars - 1);
				continue;
			}

			/* エスケープシーケンスは常に取り除く */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* ダブルクォーテーション文字列読み込み中 */
			if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( '_' == pLine[i] ||
					':' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'.' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					if( 0 == stricmp( "proc", szWord ) ){
						strcpy( szProcName, szWordPrev );
						bProcReading = TRUE;
						nFuncId = 50;
						++nFuncNum;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nLineCount/*nFuncLine - 1*/,
							&nPosX,
							&nPosY
						);
//						char szWork[256];
						pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, szProcName, nFuncId );
					}else
					if( 0 == stricmp( "endp", szWord ) ){
						nFuncId = 52;
						++nFuncNum;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nLineCount/*nFuncLine - 1*/,
							&nPosX,
							&nPosY
						);
//						char szWork[256];
						pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, szWordPrev, nFuncId );

						strcpy( szProcName, "" );
						bProcReading = FALSE;
					}else{
						/* ラベル */
						if( 0 <= i - 1 && ':' == pLine[i - 1] ){
							nFuncId = 51;
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nLineCount/*nFuncLine - 1*/,
								&nPosX,
								&nPosY
							);
	//						char szWork[256];
							pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, szWord, nFuncId );

						}

					}
					strcpy( szWordPrev , szWord );

					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					continue;
				}else
				/* ラインコメントの識別子がある場合、次の行の処理へ */
				if( i < nLineLen - 1 && ';' == pLine[i] ){
					break;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = 21;
					continue;
				}else{
					if( '_' == pLine[i] ||
						':' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )||
						'.' == pLine[i]
					){
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						nMode = 1;
					}else{
						nMode = 0;
					}
				}
			}
		}
	}
	return;
}




/* アクティブなペインを設定 */
void  CEditDoc::SetActivePane( int nIndex )
{
	m_cEditViewArr[m_nActivePaneIndex].OnKillFocus();

	m_nActivePaneIndex = nIndex;	/* アクティブなビュー */

	m_cEditViewArr[m_nActivePaneIndex].OnSetFocus();

	m_cEditViewArr[m_nActivePaneIndex].RedrawAll();	/* フォーカス移動時の再描画 */

	m_cSplitterWnd.SetActivePane( nIndex );

	if( NULL != m_cDlgFind.m_hWnd ){		/* 「検索」ダイアログ */
		/* モードレス時：検索対象となるビューの変更 */
		m_cDlgFind.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}
	if( NULL != m_cDlgReplace.m_hWnd ){	/* 「置換」ダイアログ */
		/* モードレス時：検索対象となるビューの変更 */
		m_cDlgReplace.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}
	if( NULL != m_cHokanMgr.m_hWnd ){	/* 「入力補完」ダイアログ */
		m_cHokanMgr.Hide();
		/* モードレス時：検索対象となるビューの変更 */
		m_cHokanMgr.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}

//	2001/06/20 Start by asa-o:	アクティブでないペインのカーソルアンダーバーを非表示
	m_cEditViewArr[m_nActivePaneIndex].CaretUnderLineON(TRUE);
	m_cEditViewArr[m_nActivePaneIndex^1].CaretUnderLineOFF(TRUE);
	m_cEditViewArr[m_nActivePaneIndex^2].CaretUnderLineOFF(TRUE);
	m_cEditViewArr[(m_nActivePaneIndex^2)^1].CaretUnderLineOFF(TRUE);
//	2001/06/20 End

	return;
}



/* アクティブなペインを取得 */
int CEditDoc::GetActivePane( void )
{
	return m_nActivePaneIndex;
}






/* コマンドコードによる処理振り分け */
BOOL CEditDoc::HandleCommand( int nCommand )
{
	int				i;
	int				j;
	int				nRowNum;
	int				nPane;
	HWND			hwndWork;
	EditNode*		pEditNodeArr;
	switch( nCommand ){
	case F_PREVWINDOW:	//前のウィンドウ
		nPane = m_cSplitterWnd.GetPrevPane();
		if( -1 != nPane ){
			SetActivePane( nPane );
		}else{
			/* 現在開いている編集窓のリストを得る */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* 自分のウィンドウを調べる */
				for( i = 0; i < nRowNum; ++i ){
					j = 0;
					if( m_hwndParent == pEditNodeArr[i].m_hWnd ){
						j = i;
						break;
					}
				}
				if( j == 0 ){
					j = nRowNum - 1;
				}else{
					j--;
				}
				/* 次のウィンドウをアクティブにする */
				hwndWork = pEditNodeArr[j].m_hWnd;
				/* アクティブにする */
				ActivateFrameWindow( hwndWork );
//				if( ::IsIconic( hwndWork ) ){
//					::ShowWindow( hwndWork, SW_RESTORE );
//				}else{
//					::ShowWindow( hwndWork, SW_SHOW );
//				}
//				::SetForegroundWindow( hwndWork );
//				::SetActiveWindow( hwndWork );
				/* 最後のペインをアクティブにする */
				::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
				delete [] pEditNodeArr;
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	//次のウィンドウ
		nPane = m_cSplitterWnd.GetNextPane();
		if( -1 != nPane ){
			SetActivePane( nPane );
		}else{
			/* 現在開いている編集窓のリストを得る */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* 自分のウィンドウを調べる */
				for( i = 0; i < nRowNum; ++i ){
					j = 0;
					if( m_hwndParent == pEditNodeArr[i].m_hWnd ){
						j = i;
						break;
					}
				}
				if( j == nRowNum - 1 ){
					j = 0;
				}else{
					++j;
				}
				/* 次のウィンドウをアクティブにする */
				hwndWork = pEditNodeArr[j].m_hWnd;
				/* アクティブにする */
				ActivateFrameWindow( hwndWork );
//				if( ::IsIconic( hwndWork ) ){
//					::ShowWindow( hwndWork, SW_RESTORE );
//				}else{
//					::ShowWindow( hwndWork, SW_SHOW );
//				}
//				::SetForegroundWindow( hwndWork );
//				::SetActiveWindow( hwndWork );

				/* 最初のペインをアクティブにする */
				::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
				delete [] pEditNodeArr;
			}
		}
		return TRUE;

	default:
		return m_cEditViewArr[m_nActivePaneIndex].HandleCommand( nCommand, TRUE, 0, 0, 0, 0 );
	}
}




/* ビューに設定変更を反映させる */
void CEditDoc::OnChangeSetting( void )
{
//	return;
	int			i;
	HWND		hwndProgress;

	CEditWnd*	pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );

	pCEditWnd->m_CFuncKeyWnd.m_nCurrentKeyState = -1;

	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}


	/* ファイルの排他モード変更 */
	if( m_nFileShareModeOld != m_pShareData->m_Common.m_nFileShareMode ){
		/* ファイルの排他ロック解除 */
		DoFileUnLock();
		/* ファイルの排他ロック */
		DoFileLock();
	}
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
	int doctype = CShareData::getInstance()->GetDocumentType( m_szFilePath );
	SetDocumentType( doctype, false );

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	int		nX[4];
	int		nY[4];
	for( i = 0; i < 4; ++i ){
		m_cLayoutMgr.CaretPos_Log2Phys(
			m_cEditViewArr[i].m_nCaretPosX,
			m_cEditViewArr[i].m_nCaretPosY,
			&nX[i],
			&nY[i]
		);
	}

	/* レイアウト情報の作成 */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		ref.m_nMaxLineSize,
		ref.m_bWordWrap,			/* 英文ワードラップをする */
		ref.m_nTabSpace,
		ref.m_szLineComment,		/* 行コメントデリミタ */
		ref.m_szLineComment2,		/* 行コメントデリミタ2 */
		ref.m_szLineComment3,		/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
		ref.m_szBlockCommentFrom,	/* ブロックコメントデリミタ(From) */
		ref.m_szBlockCommentTo,		/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		ref.m_szBlockCommentFrom2,	/* ブロックコメントデリミタ2(From) */
		ref.m_szBlockCommentTo2,	/* ブロックコメントデリミタ2(To) */
//#endif
		ref.m_nStringType,			/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		TRUE,
		hwndProgress,
		ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* シングルクォーテーション文字列を表示する */
		ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
	); /* レイアウト情報の変更 */

	/* ビューに設定変更を反映させる */
	for( i = 0; i < 4; ++i ){
		m_cEditViewArr[i].OnChangeSetting();
		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		int		nPosX;
		int		nPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			nX[i],
			nY[i],
			&nPosX,
			&nPosY
		);
		if( nPosY >= m_cLayoutMgr.GetLineCount() ){
			/*ファイルの最後に移動 */
//			m_cEditViewArr[i].Command_GOFILEEND(FALSE);
			m_cEditViewArr[i].HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
		}else{
			m_cEditViewArr[i].MoveCursor( nPosX, nPosY, TRUE );
		}
	}
}




/* 編集ファイル情報を格納 */
void CEditDoc::SetFileInfo( FileInfo* pfi )
{
	int		nX;
	int		nY;

	strcpy( pfi->m_szPath, m_szFilePath );
	pfi->m_nViewTopLine = m_cEditViewArr[m_nActivePaneIndex].m_nViewTopLine;	/* 表示域の一番上の行(0開始) */
	pfi->m_nViewLeftCol = m_cEditViewArr[m_nActivePaneIndex].m_nViewLeftCol;	/* 表示域の一番左の桁(0開始) */
	//	pfi->m_nCaretPosX = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;	/* ビュー左端からのカーソル桁位置(０開始) */
	//	pfi->m_nCaretPosY = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY;	/* ビュー上端からのカーソル行位置(０開始) */

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	m_cLayoutMgr.CaretPos_Log2Phys(
		m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX,	/* ビュー左端からのカーソル桁位置(０開始) */
		m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY,	/* ビュー上端からのカーソル行位置(０開始) */
		&nX,
		&nY
	);
	pfi->m_nX = nX;		/* カーソル 物理位置(行頭からのバイト数) */
	pfi->m_nY = nY;		/* カーソル 物理位置(折り返し無し行位置) */


	pfi->m_bIsModified = IsModified() ? TRUE : FALSE;			/* 変更フラグ */
	pfi->m_nCharCode = m_nCharCode;				/* 文字コード種別 */
//	pfi->m_bPLSQL = m_cDlgJump.m_bPLSQL,		/* 行ジャンプが PL/SQLモードか */
//	pfi->m_nPLSQL_E1 = m_cDlgJump.m_nPLSQL_E1;	/* 行ジャンプが PL/SQLモードのときの基点 */

	pfi->m_bIsGrep = m_bGrepMode;
	strcpy( pfi->m_szGrepKey, m_szGrepKey );
	return;

}


/* タグジャンプ元など参照元の情報を保持する */
void CEditDoc::SetReferer( HWND hwndReferer, int nRefererX, int nRefererLine )
{
	m_hwndReferer	= hwndReferer;	/* 参照元ウィンドウ */
	m_nRefererX		= nRefererX;	/* 参照元  行頭からのバイト位置桁 */
	m_nRefererLine	= nRefererLine;	/* 参照元行  折り返し無しの物理行位置 */
	return;
}



/*! ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行

	@retval TRUE: 終了して良い / FALSE: 終了しない
*/
BOOL CEditDoc::OnFileClose( void )
{
	int			nRet;
	int			nBool;
	FileInfo	fi;
	HWND		hwndMainFrame;
	hwndMainFrame = ::GetParent( m_hWnd );
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	CMRU		cMRU;

	/* MRUリストの登録 */
	SetFileInfo( &fi );

	// 2002.01.16 hor ブックマーク記録
	strcpy( fi.m_szMarkLines, m_cDocLineMgr.GetBookMarks() );

	/* MRUリストへの登録 */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	cMRU.Add( &fi );

	if( m_bGrepRunning ){		/* Grep処理中 */
		::MYMESSAGEBOX(
			hwndMainFrame,
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"Grepの処理中です。\n"
		);
		return FALSE;
	}


	/* テキストが変更されている場合 */
	if( IsModified()
	&& FALSE == m_bDebugMode	/* デバッグモニタモードのときは保存確認しない */
//	&& FALSE == m_bReadOnly		/* 読み取り専用モード */
	){
		if( TRUE == m_bGrepMode ){	/* Grepモードのとき */
			/* Grepモードで保存確認するか */
			if( FALSE == m_pShareData->m_Common.m_bGrepExitConfirm ){
				return TRUE;
			}
		}
		/* ウィンドウをアクティブにする */
		/* アクティブにする */
		ActivateFrameWindow( hwndMainFrame );
		if( m_bReadOnly ){	/* 読み取り専用モード */
			::MessageBeep( MB_ICONQUESTION );
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				"%s\nは変更されています。 閉じる前に保存しますか？\n\n読み取り専用で開いているので、名前を付けて保存すればいいと思います。\n",
				lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）"
			);
			switch( nRet ){
			case IDYES:
//				if( 0 < lstrlen( m_szFilePath ) ){
//					nBool = HandleCommand( F_FILESAVE );
//				}else{
					nBool = HandleCommand( F_FILESAVEAS_DIALOG );
//				}
				return nBool;
			case IDNO:
				return TRUE;
			case IDCANCEL:
			default:
				return FALSE;
			}
		}else{
			::MessageBeep( MB_ICONQUESTION );
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				"%s\nは変更されています。 閉じる前に保存しますか？",
				lstrlen( m_szFilePath ) ? m_szFilePath : "（無題）"
			);
			switch( nRet ){
			case IDYES:
				if( 0 < lstrlen( m_szFilePath ) ){
					nBool = HandleCommand( F_FILESAVE );
				}else{
					nBool = HandleCommand( F_FILESAVEAS_DIALOG );
				}
				return nBool;
			case IDNO:
				return TRUE;
			case IDCANCEL:
			default:
				return FALSE;
			}
		}
	}else{
		return TRUE;
	}
}


/* 既存データのクリア */
void CEditDoc::Init( void )
{
//	int types;

	m_bReadOnly = FALSE;	/* 読み取り専用モード */
	strcpy( m_szGrepKey, "" );
	m_bGrepMode = FALSE;	/* Grepモード */

//@@@ 2001.12.26 YAZAKI 大きいアイコンと小さいアイコンを別々にする。
	HICON	hIconBig, hIconSmall;
#ifdef _DEBUG
	hIconBig = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_DEBUG ) );
	hIconSmall = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE( IDI_ICON_DEBUG ), IMAGE_ICON, 16, 16, 0);
#else
	hIconBig = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_STD ) );
	hIconSmall = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE( IDI_ICON_STD ), IMAGE_ICON, 16, 16, 0);
#endif
	::SendMessage( m_hwndParent, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall );
	::SendMessage( m_hwndParent, WM_SETICON, ICON_BIG, (LPARAM)hIconBig );

//	HICON	hIcon;
//#ifdef _DEBUG
//	hIcon = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_DEBUG ) );
//#else
//	hIcon = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_STD ) );
//#endif
//	::SendMessage( m_hwndParent, WM_SETICON, ICON_SMALL, (LPARAM)NULL );
//	::SendMessage( m_hwndParent, WM_SETICON, ICON_SMALL, (LPARAM)hIcon );
//	::SendMessage( m_hwndParent, WM_SETICON, ICON_BIG, (LPARAM)NULL );
//	::SendMessage( m_hwndParent, WM_SETICON, ICON_BIG, (LPARAM)hIcon );

	/* ファイルの排他ロック解除 */
	DoFileUnLock();

	/* ファイルの排他制御モード */
	m_nFileShareModeOld = 0;


	/*アンドゥ・リドゥバッファのクリア */
	/* 全要素のクリア */
	m_cOpeBuf.ClearAll();

	/* テキストデータのクリア */
	m_cDocLineMgr.Empty();
	m_cDocLineMgr.Init();

	/* 現在編集中のファイルのパス */
	m_szFilePath[0] = '\0';

	/* 現在編集中のファイルのタイムスタンプ */
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;


	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
	int doctype = CShareData::getInstance()->GetDocumentType( m_szFilePath );
	SetDocumentType( doctype, true );

	/* レイアウト管理情報の初期化 */
	//	m_cLayoutMgr.Create( &m_cDocLineMgr, GetDocumentAttribute().m_nMaxLineSize, GetDocumentAttribute().m_nTabSpace ) ;
	/* レイアウト情報の変更 */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		ref.m_nMaxLineSize,
		ref.m_bWordWrap,			/* 英文ワードラップをする */
		ref.m_nTabSpace,
		ref.m_szLineComment,		/* 行コメントデリミタ */
		ref.m_szLineComment2,		/* 行コメントデリミタ2 */
		ref.m_szLineComment3,		/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
		ref.m_szBlockCommentFrom,	/* ブロックコメントデリミタ(From) */
		ref.m_szBlockCommentTo,		/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		ref.m_szBlockCommentFrom2,	/* ブロックコメントデリミタ2(From) */
		ref.m_szBlockCommentTo2,	/* ブロックコメントデリミタ2(To) */
//#endif
		ref.m_nStringType,			/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
		TRUE,
		NULL,/*hwndProgress*/
		ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* シングルクォーテーション文字列を表示する */
		ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
	);


	/* 変更フラグ */
	SetModified(false,false);	//	Jan. 22, 2002 genta

	/* 文字コード種別 */
	m_nCharCode = 0;

	//	May 12, 2000
	m_cNewLineCode.SetType( EOL_CRLF );

	return;
}

/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */
void CEditDoc::InitAllView( void )
{
	int		i;

	m_nCommandExecNum = 0;	/* コマンド実行回数 */
	/* 先頭へカーソルを移動 */
	for( i = 0; i < 4; ++i ){
		//	Apr. 1, 2001 genta
		// 移動履歴の消去
		m_cEditViewArr[i].m_cHistory->Flush();

		/* 現在の選択範囲を非選択状態に戻す */
		m_cEditViewArr[i].DisableSelectArea( FALSE );

		m_cEditViewArr[i].OnChangeSetting();
		m_cEditViewArr[i].MoveCursor( 0, 0, TRUE );
	}

	return;
}


/* ファイルのタイムスタンプのチェック処理 */
void CEditDoc::CheckFileTimeStamp( void )
{
	HWND		hwndActive;
	BOOL		bUpdate;
	bUpdate = FALSE;
	if( m_pShareData->m_Common.m_bCheckFileTimeStamp	/* 更新の監視 */
	 && m_pShareData->m_Common.m_nFileShareMode == 0	/* ファイルの排他制御モード */
	 && NULL != ( hwndActive = ::GetActiveWindow() )	/* アクティブ? */
	 && hwndActive == m_hwndParent
	 && 0 < lstrlen( m_szFilePath )
	 && ( m_FileTime.dwLowDateTime != 0 || m_FileTime.dwHighDateTime != 0 ) 	/* 現在編集中のファイルのタイムスタンプ */

	){
		while( 1 ){
			/* ファイルスタンプをチェックする */
//			MYTRACE( "ファイルスタンプをチェックする\n" );

			FILETIME	FileTimeNow;
			HFILE		hFile;
			BOOL		bWork;
			LONG		lWork;

			hFile = _lopen( m_szFilePath, OF_READ );
			if( HFILE_ERROR == hFile ){
				break;
			}
			bWork = ::GetFileTime( (HANDLE)hFile, NULL, NULL, &FileTimeNow );
			_lclose( hFile );
			if( 0 == bWork ){
				break;
			}
			lWork = ::CompareFileTime( &m_FileTime, &FileTimeNow );
			if( -1 == lWork ){
				bUpdate = TRUE;
//				MYTRACE( "★更新されています★★★★★★★★★★★\n" );
				m_FileTime = FileTimeNow;
			}
			break;
		}
	}

	if( !bUpdate ){
		return;
	}
	if( IDYES != MYMESSAGEBOX( m_hwndParent, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME,
		"%s\n\nこのファイルは外部のエディタ等で変更されています。%s",
		m_szFilePath,
		(IsModified())?"\n再ロードを行うと変更が失われますがよろしいですか?":"再ロードしますか?"
	) ){
		return;
	}

	int		nCharCode;				/* 文字コード種別 */
	BOOL	bReadOnly;				/* 読み取り専用モード */
	nCharCode = m_nCharCode;		/* 文字コード種別 */
	bReadOnly = m_bReadOnly;		/* 読み取り専用モード */
	/* 同一ファイルの再オープン */
	ReloadCurrentFile(
		nCharCode,		/* 文字コード種別 */
		bReadOnly		/* 読み取り専用モード */
	);
	return;
}





/*! 同一ファイルの再オープン */
void CEditDoc::ReloadCurrentFile(
	BOOL	nCharCode,		/*!< [in] 文字コード種別 */
	BOOL	bReadOnly		/*!< [in] 読み取り専用モード */
)
{
	if( -1 == _access( m_szFilePath, 0 ) ){
		/* ファイルが存在しない */
		m_nCharCode = nCharCode;
		return;
	}


	BOOL	bOpened;
	char	szFilePath[MAX_PATH];
	int		nCaretPosX;
	int		nCaretPosY;
	nCaretPosX = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;
	nCaretPosY = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY;

	strcpy( szFilePath, m_szFilePath );

	/* 既存データのクリア */
	Init();

	/* 全ビューの初期化 */
	InitAllView();

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();

	/* ファイル読み込み */
	FileRead(
		szFilePath,
		&bOpened,
		nCharCode,	/* 文字コード自動判別 */
		bReadOnly,	/* 読み取り専用か */
		FALSE		/* 文字コード変更時の確認をするかどうか */
	);

	m_cEditViewArr[m_nActivePaneIndex].MoveCursor( nCaretPosX, nCaretPosY, TRUE );
}

//	From Here Nov. 20, 2000 genta
/*!	IME状態の設定
	
	@param mode [in] IMEのモード
	
	@date Nov 20, 2000 genta
*/
void CEditDoc::SetImeMode( int mode )
{
	DWORD	conv, sent;
	HIMC	hIme;

	hIme = ImmGetContext( m_hwndParent );

	//	最下位ビットはIME自身のOn/Off制御
	if( ( mode & 3 ) == 2 ){
		ImmSetOpenStatus( hIme, FALSE );
	}
	if( ( mode >> 2 ) > 0 ){
		ImmGetConversionStatus( hIme, &conv, &sent );

		switch( mode >> 2 ){
		case 1:	//	FullShape
			conv |= IME_CMODE_FULLSHAPE;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 2:	//	FullShape & Hiragana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
			conv &= ~( IME_CMODE_KATAKANA | IME_CMODE_NOCONVERSION );
			break;
		case 3:	//	FullShape & Katakana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE | IME_CMODE_KATAKANA;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 4: //	Non-Conversion
			conv |= IME_CMODE_NOCONVERSION;
			break;
		}
		ImmSetConversionStatus( hIme, conv, sent );
	}
	if( ( mode & 3 ) == 1 ){
		ImmSetOpenStatus( hIme, TRUE );
	}
	ImmReleaseContext( m_hwndParent, hIme );
}
//	To Here Nov. 20, 2000 genta


/*!	$xの展開

*/
void CEditDoc::ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen)
{
	const char *p, *r;	//	p：目的のバッファ。r：作業用のポインタ。
	char *q, *q_max;
	for( p = pszSource, q = pszBuffer, q_max = pszBuffer + nBufferLen; *p != '\0' && q < q_max; ++p, ++q){
		if( *p != '$' ){
			*q = *p;
			continue;
		}
		switch( p[1] ){
		case '$':	//	 $$ -> $
			*q = *p;
			++p;
			break;
		case 'F':	//	開いているファイルの名前（フルパス）
			if (m_szFilePath[0] == '\0'){
				memcpy(q, "(無題)", 6);
				q += 6 - 1;
				++p;
			} 
			else {
				for( r = m_szFilePath; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q; // genta
				++p;
			}
			break;
		case 'f':	//	開いているファイルの名前（ファイル名のみ）
			// Oct. 28, 2001 genta
			//	ファイル名のみを渡すバージョン
			//	ポインタを末尾に
			if (m_szFilePath[0] == '\0'){
				memcpy(q, "(無題)", 6);
				q += 6 - 1;
				++p;
			} 
			else {
				r = m_szFilePath + strlen( m_szFilePath );
				
				//	後ろから区切りを探す
				for( --r; r >= m_szFilePath && *r != '\\' ; --r )
					;
				//	\\が無かった場合は１つ目の条件によって先頭の１つ前にポインタがある。
				//	万一\\が末尾にあってもその後ろには\0があるのでアクセス違反にはならない。
				for( ++r ; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		case '/':	//	開いているファイルの名前（フルパス。パスの区切りが/）
			// Oct. 28, 2001 genta
			if (m_szFilePath[0] == '\0'){
				memcpy(q, "(無題)", 6);
				q += 6 - 1;
				++p;
			} 
			else {
				//	パスの区切りとして'/'を使うバージョン
				for( r = m_szFilePath; *r != '\0' && q < q_max; ++r, ++q ){
					if( *r == '\\' )
						*q = '/';
					else
						*q = *r;
				}
				--q;
				++p;
			}
			break;
		//	From Here Jan. 15, 2002 hor
		case 'C':	//	現在選択中のテキスト
			{
				CMemory cmemCurText;
				m_cEditViewArr[m_nActivePaneIndex].GetCurrentTextForSearch( cmemCurText );
				for( r = cmemCurText.GetPtr( NULL ); *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
		//	To Here Jan. 15, 2002 hor
			break;
		case 'd':	//	共通設定の日付書式
			{
				char szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetDateFormat( systime, szText, sizeof( szText ) - 1 );
				for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		case 't':	//	共通設定の時刻書式
			{
				char szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetTimeFormat( systime, szText, sizeof( szText ) - 1 );
				for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		case 'p':	//	現在のページ
			{
				CEditWnd* pcEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetCurPageNum() + 1, szText, 10);
					for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
						*q = *r;
					--q;
					++p;
				}
				else {
					for ( r = "(印刷プレビューでのみ使用できます)"; *r != '\0' && q < q_max; ++r, ++q )
						*q = *r;
					--q;
					++p;
				}
			}
			break;
		case 'P':	//	総ページ
			{
				CEditWnd* pcEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetAllPageNum(), szText, 10);
					for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
						*q = *r;
					--q;
					++p;
				}
				else {
					for ( r = "(印刷プレビューでのみ使用できます)"; *r != '\0' && q < q_max; ++r, ++q )
						*q = *r;
					--q;
					++p;
				}
			}
			break;
		case 'D':	//	タイムスタンプ
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				char szText[1024];
				CShareData::getInstance()->MyGetDateFormat( systimeL, szText, sizeof( szText ) - 1 );
				for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			else {
				for ( r = "(保存されていません)"; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		case 'T':	//	タイムスタンプ
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				char szText[1024];
				CShareData::getInstance()->MyGetTimeFormat( systimeL, szText, sizeof( szText ) - 1 );
				for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			else {
				for ( r = "(保存されていません)"; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		default:
			*q = *p;
			break;
		}
	}
	*q = '\0';
}
/*[EOF]*/
