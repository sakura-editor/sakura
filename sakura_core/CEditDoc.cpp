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
	Copyright (C) 2002, YAZAKI, hor, genta, aroka, MIK
	Copyright (C) 2003, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include <stdlib.h>
#include <time.h>
#include <string.h>	// Apr. 03, 2003 genta
#include <io.h>
#include "CEditDoc.h"
#include "debug.h"
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
#include "CDlgFileUpdateQuery.h"
#include <assert.h> /// 2002/11/2 frozen
#include "my_icmp.h" // 2002/11/30 Moca 追加


#define IDT_ROLLMOUSE	1

/*!
	May 12, 2000 genta 初期化方法変更
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@note m_pcEditWnd はコンストラクタ内では使用しないこと．
*/
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
	m_bBomExist( FALSE ),			//	Jul. 26, 2003 ryoji BOM
	m_nActivePaneIndex( 0 ),
//@@@ 2002.01.14 YAZAKI 不使用のため
//	m_pcOpeBlk( NULL ),				/* 操作ブロック */
	m_bDoing_UndoRedo( FALSE ),		/* アンドゥ・リドゥの実行中か */
	m_nFileShareModeOld( 0 ),		/* ファイルの排他制御モード */
	m_hLockedFile( NULL ),			/* ロックしているファイルのハンドル */
	m_pszAppName( "EditorClient" ),
	m_hInstance( NULL ),
	m_hWnd( NULL ),
	m_eWatchUpdate( CEditDoc::WU_QUERY ),
	m_nSettingTypeLocked( false ),	//	設定値変更可能フラグ
	m_nSettingType( 0 ),	// Sep. 11, 2002 genta
	m_bIsModified( false )	/* 変更フラグ */ // Jan. 22, 2002 genta 型変更
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::CEditDoc" );
//	m_pcDlgTest = new CDlgTest;

	m_szFilePath[0] = '\0';			/* 現在編集中のファイルのパス */
	strcpy( m_szGrepKey, "" );
	/* 共有データ構造体のアドレスを返す */

	m_pShareData = CShareData::getInstance()->GetShareData();
	//	Sep. 11, 2002 genta 削除
	//	SetDocumentTypeはコンストラクタ中では使わない．
	//int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	//SetDocumentType( doctype, true );

	strcpy( m_szDefaultWildCard, "*.*" );				/* 「開く」での最初のワイルドカード */


	/* レイアウト管理情報の初期化 */
	m_cLayoutMgr.Create( this, &m_cDocLineMgr );
	/* レイアウト情報の変更 */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		NULL,/*hwndProgress*/
		ref
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
//	@date Sep. 29, 2001 genta マクロクラスを渡すように
//	@date 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
/////////////////////////////////////////////////////////////////////////////
BOOL CEditDoc::Create(
	HINSTANCE hInstance,
	HWND hwndParent,
	CImageListMgr* pcIcons
 )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::Create" );

	HWND		hWndArr[4];
	CEditWnd*	pCEditWnd;
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

	/* 分割フレーム作成 */
	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
	m_cSplitterWnd.Create( m_hInstance, m_hwndParent, pCEditWnd );

	/* ビュー */
	m_cEditViewArr[0].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 0, /*FALSE,*/ TRUE  );
	m_cEditViewArr[1].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 1, /*TRUE ,*/ FALSE );
	m_cEditViewArr[2].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 2, /*TRUE ,*/ FALSE );
	m_cEditViewArr[3].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 3, /*TRUE ,*/ FALSE );

#if 0
	YAZAKI 不要な処理と思われる。
	m_cEditViewArr[0].OnKillFocus();
	m_cEditViewArr[1].OnKillFocus();
	m_cEditViewArr[2].OnKillFocus();
	m_cEditViewArr[3].OnKillFocus();
#endif

	m_cEditViewArr[0].OnSetFocus();

	/* 子ウィンドウの設定 */
	hWndArr[0] = m_cEditViewArr[0].m_hWnd;
	hWndArr[1] = m_cEditViewArr[1].m_hWnd;
	hWndArr[2] = m_cEditViewArr[2].m_hWnd;
	hWndArr[3] = m_cEditViewArr[3].m_hWnd;
	m_cSplitterWnd.SetChildWndArr( hWndArr );
	m_hWnd = m_cSplitterWnd.m_hWnd;

	MY_TRACETIME( cRunningTimer, "View created" );

	//	Oct. 2, 2001 genta
	m_cFuncLookup.Init( m_hInstance, m_pcSMacroMgr, &m_pShareData->m_Common );

	/* 設定プロパティシートの初期化１ */
	m_cPropCommon.Create( m_hInstance, m_hWnd, pcIcons, m_pcSMacroMgr, &(pCEditWnd->m_CMenuDrawer) );
	m_cPropTypes.Create( m_hInstance, m_hWnd );

	MY_TRACETIME( cRunningTimer, "End: PropSheet" );

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
	// 2004.02.16 Moca CHOOSEFONTをメンバから外す
	CHOOSEFONT cf;
	/* CHOOSEFONTの初期化 */
	::ZeroMemory( &cf, sizeof( CHOOSEFONT ) );
	cf.lStructSize = sizeof( cf );
	cf.hwndOwner = m_hWnd;
	cf.hDC = NULL;
//	cf.lpLogFont = &(m_pShareData->m_Common.m_lf);
	cf.Flags = CF_FIXEDPITCHONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
//#ifdef _DEBUG
//	cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
//#endif
	cf.lpLogFont = plf;
	if( FALSE == ChooseFont( &cf ) ){
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

	//	From Here Oct. 19, 2002 genta
	//	読み込みアクセス権が無い場合には漢字コード判定でファイルを
	//	開けないので文字コード判別エラーと出てしまう．
	//	より適切なメッセージを出すため，読めないファイルは
	//	事前に判定・排除する
	//
	//	_accessではロックされたファイルの状態を取得できないので
	//	実際にファイルを開いて確認する
	if( bFileIsExist){
		HANDLE hTest = 	CreateFile( pszPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( hTest == INVALID_HANDLE_VALUE ){
			// 読み込みアクセス権がない
			::MYMESSAGEBOX(
				m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%s\'\nというファイルを開けません。\n読み込みアクセス権がありません。"),
				pszPath
			 );
			return FALSE;
		}
		else {
			CloseHandle( hTest );
		}
	}
	//	To Here Oct. 19, 2002 genta

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
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

	//	Sep. 10, 2002 genta
	SetFilePath( pszPath ); /* 現在編集中のファイルのパス */


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
				//	Sep. 10, 2002 genta
				SetFilePath( "" );
				bRet = FALSE;
				goto end_of_func;
			}
		}
		if( m_nCharCode != fi.m_nCharCode ){
			if( bConfirmCodeChange ){
				char*	pszCodeName = NULL;
				char*	pszCodeNameNew = NULL;

				// gm_pszCodeNameArr_1 を使うように変更 Moca. 2002/05/26
				if( -1 < fi.m_nCharCode && fi.m_nCharCode < CODE_CODEMAX ){
					pszCodeName = (char*)gm_pszCodeNameArr_1[fi.m_nCharCode];
				}
				if( -1 < m_nCharCode && m_nCharCode < CODE_CODEMAX ){
					pszCodeNameNew = (char*)gm_pszCodeNameArr_1[m_nCharCode];
				}
				if( pszCodeName != NULL ){
					::MessageBeep( MB_ICONQUESTION );
					nRet = MYMESSAGEBOX(
						m_hWnd,
						MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
						"文字コード情報",
						"%s\n\nこのファイルは、前回は別の文字コード %s で開かれています。\n前回と同じ文字コードを使いますか？\n\n・[はい(Y)]  ＝%s\n・[いいえ(N)]＝%s\n・[キャンセル]＝開きません",
						GetFilePath(), pszCodeName, pszCodeName, pszCodeNameNew
					);
					if( IDYES == nRet ){
						/* 前回に指定された文字コード種別に変更する */
						m_nCharCode = fi.m_nCharCode;
					}else
					if( IDCANCEL == nRet ){
						m_nCharCode = 0;
						//	Sep. 10, 2002 genta
						SetFilePath( "" );
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
					//	Sep. 10, 2002 genta
					SetFilePath( "" );
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
				//	Sep. 10, 2002 genta
				SetFilePath( "" );
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
		//	Sep. 10, 2002 genta
		SetFilePath( szWork );
	}

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
	doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	SetDocumentType( doctype, true );

	//	From Here Jul. 26, 2003 ryoji BOMの有無の初期状態を設定
	switch( m_nCharCode ){
	case CODE_UNICODE:
	case CODE_UNICODEBE:
		m_bBomExist = TRUE;
		break;
	case CODE_UTF8:
	default:
		m_bBomExist = FALSE;
		break;
	}
	//	To Here Jul. 26, 2003 ryoji BOMの有無の初期状態を設定

	/* ファイルが存在しない */
	if( FALSE == bFileIsExist ){
//		::MessageBeep( MB_ICONINFORMATION );

		//	Feb. 15, 2003 genta Popupウィンドウを表示しないように．
		//	ここでステータスメッセージを使っても画面に表示されない．
		//::MYMESSAGEBOX(
		//	m_hwndParent,
		//	MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
		//	GSTR_APPNAME,
//			"\'%s\'\nファイルは存在しません。 ファイルを保存したときに、ディスク上にファイルが作成されます。",
		//	"%s\nというファイルは存在しません。\n\nファイルを保存したときに、ディスク上にこのファイルが作成されます。",	//Mar. 24, 2001 jepro 若干修正
		//	pszPath
		//);

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
		//	Jul. 26, 2003 ryoji BOM引数追加
		if( FALSE == m_cDocLineMgr.ReadFile( GetFilePath(), m_hWnd, hwndProgress,
			m_nCharCode, &m_FileTime, m_pShareData->m_Common.GetAutoMIMEdecode(), &m_bBomExist ) ){
			//	Sep. 10, 2002 genta
			SetFilePath( "" );
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
			TRUE,
			hwndProgress,
			ref
		);
	}

	/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */
	InitAllView();

	//	Nov. 20, 2000 genta
	//	IME状態の設定
	SetImeMode( GetDocumentAttribute().m_nImeState );

	if( bIsExistInMRU && m_pShareData->m_Common.GetRestoreCurPosition() ){
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
			// From Here Mar. 28, 2003 MIK
			// 改行の真ん中にカーソルが来ないように。
			CDocLine *pTmpDocLine = m_cDocLineMgr.GetLineInfo( nCaretPosY );
			if( pTmpDocLine ){
				if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_nX ) nCaretPosX--;
			}
			// To Here Mar. 28, 2003 MIK
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
	{
		SetNewLineCode( EOL_CRLF );
		CDocLine*	pFirstlineinfo = m_cDocLineMgr.GetLineInfo( 0 );
		if( pFirstlineinfo != NULL ){
			enumEOLType t = (enumEOLType)pFirstlineinfo->m_cEol;
			if( t != EOL_NONE && t != EOL_UNKNOWN ){
				SetNewLineCode( t );
			}
		}
	}

	/* MRUリストへの登録 */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	cMRU.Add( &fi );
	
	/* カレントディレクトリの変更 */
	{
		char	szCurDir[_MAX_PATH];
		char	szDrive[_MAX_DRIVE], szDir[_MAX_DIR];
		_splitpath( GetFilePath(), szDrive, szDir, NULL, NULL );
		strcpy( szCurDir, szDrive);
		strcat( szCurDir, szDir );
		::SetCurrentDirectory( szCurDir );
	}

end_of_func:;
	//	2004.05.13 Moca 改行コードの設定内からここに移動
	m_cEditViewArr[m_nActivePaneIndex].DrawCaretPosInfo();

	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	if( TRUE == bRet && IsFilePathAvailable() ){
		/* ファイルの排他ロック */
		DoFileLock();
	}
	//	From Here Jul. 26, 2003 ryoji エラーの時は規定のBOM設定とする
	if( FALSE == bRet ){
		switch( m_nCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_bBomExist = TRUE;
			break;
		case CODE_UTF8:
		default:
			m_bBomExist = FALSE;
			break;
		}
	}
	//	To Here Jul. 26, 2003 ryoji
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

	//	Jun.  5, 2004 genta ここでReadOnlyチェックをすると，ファイル名を変更しても
	//	保存できなくなってしまうので，チェックを上書き保存処理へ移動．

	//	Sep. 7, 2003 genta
	//	保存が完了するまではファイル更新の通知を抑制する
	WatchUpdate wuSave = m_eWatchUpdate;
	m_eWatchUpdate = WU_NONE;

	bRet = TRUE;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
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
		//	Jun.  5, 2004 genta ファイル名を与えるように．戻り値に応じた処理を追加．
		switch( MakeBackUp( pszPath )){
		case 2:	//	中断指示
			return FALSE;
		case 3: //	ファイルエラー
			if( IDYES != ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				"ファイル保存",
				"バックアップの作成に失敗しました．元ファイルへの上書きを継続して行いますか．"
			)){
				return FALSE;
			}
		break;
		}
	}

	CWaitCursor cWaitCursor( m_hWnd );
	//	Jul. 26, 2003 ryoji BOM引数追加
	if( FALSE == m_cDocLineMgr.WriteFile( pszPath, m_hWnd, hwndProgress,
		m_nCharCode, &m_FileTime, cEol , m_bBomExist ) ){
		bRet = FALSE;
		goto end_of_func;
	}
	/* 行変更状態をすべてリセット */
	m_cDocLineMgr.ResetAllModifyFlag();
	
#if 0
	/* ロングファイル名を取得する。（上書き保存のときのみ） */
	char szWork[MAX_PATH];
	if( TRUE == ::GetLongFileName( GetFilePath(), szWork ) ){
		//	Sep. 10, 2002 genta
		SetFilePath( szWork );
	}
#endif

	int	v;
	for( v = 0; v < 4; ++v ){
		if( m_nActivePaneIndex != v ){
			m_cEditViewArr[v].RedrawAll();
		}
	}
	m_cEditViewArr[m_nActivePaneIndex].RedrawAll();

	//	Sep. 10, 2002 genta
	SetFilePath( pszPath ); /* 現在編集中のファイルのパス */

	SetModified(false,false);	//	Jan. 22, 2002 genta 関数化 更新フラグのクリア

	//	Mar. 30, 2003 genta サブルーチンにまとめた
	AddToMRU();

	/* 現在位置で無変更な状態になったことを通知 */
	m_cOpeBuf.SetNoModified();

	m_bReadOnly = FALSE;	/* 読み取り専用モード */

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();
end_of_func:;

	if( IsFilePathAvailable() &&
		FALSE == m_bReadOnly && /* 読み取り専用モード ではない */
		TRUE == bRet
	){
		/* ファイルの排他ロック */
		DoFileLock();
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	//	Sep. 7, 2003 genta
	//	ファイル更新の通知を元に戻す
	m_eWatchUpdate = wuSave;


	return bRet;
}



/* 「ファイルを開く」ダイアログ */
//	Mar. 30, 2003 genta	ファイル名未定時の初期ディレクトリをカレントフォルダに
BOOL CEditDoc::OpenFileDialog(
	HWND		hwndParent,
	const char*	pszOpenFolder,	//<! [in]  NULL以外を指定すると初期フォルダを指定できる
	char*		pszPath,		//<! [out] 開くファイルのパスを受け取るアドレス
	int*		pnCharCode,		//<! [out] 指定された文字コード種別を受け取るアドレス
	BOOL*		pbReadOnly		//<! [out] 読み取り専用か
)
{
	/* アクティブにする */
	ActivateFrameWindow( hwndParent );

	const char*	pszDefFolder;
	char*	pszCurDir = NULL;
	char**	ppszMRU;
	char**	ppszOPENFOLDER;
	BOOL	bRet;

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

	/* 初期フォルダの設定 */
	// pszFolderはフォルダ名だが、ファイル名付きパスを渡してもCDlgOpenFile側で処理してくれる
	if( NULL != pszOpenFolder ){
		pszDefFolder = pszOpenFolder;
	}else{
		if( IsFilePathAvailable() ){
			pszDefFolder = GetFilePath();
		// Mar. 28, 2003 genta カレントディレクトリをMRUより優先させる
		//}else if( ppszMRU[0] != NULL && ppszMRU[0][0] != '\0' ){ // Sep. 9, 2002 genta
		//	pszDefFolder = ppszMRU[0];
		}else{ // 2002.10.25 Moca
			int nCurDir;
			pszCurDir = new char[_MAX_PATH];
			nCurDir = ::GetCurrentDirectory( _MAX_PATH, pszCurDir );
			if( 0 == nCurDir || _MAX_PATH < nCurDir ){
				pszDefFolder = "";
			}else{
				pszDefFolder = pszCurDir;
			}
		}
	}
	/* ファイルオープンダイアログの初期化 */
	m_cDlgOpenFile.Create(
		m_hInstance,
		hwndParent,
		m_szDefaultWildCard,
		pszDefFolder,
		(const char **)ppszMRU,
		(const char **)ppszOPENFOLDER
	);
	
	bRet = m_cDlgOpenFile.DoModalOpenDlg( pszPath, pnCharCode, pbReadOnly );

	delete [] ppszMRU;
	delete [] ppszOPENFOLDER;
	delete [] pszCurDir;
	return bRet;
}


//pszOpenFolder pszOpenFolder


/*! 「ファイル名を付けて保存」ダイアログ

	@param pszPath [out]	保存ファイル名
	@param pnCharCode [out]	保存文字コードセット
	@param pcEol [out]		保存改行コード

	@date 2001.02.09 genta	改行コードを示す引数追加
	@date 2003.03.30 genta	ファイル名未定時の初期ディレクトリをカレントフォルダに
	@date 2003.07.20 ryoji	BOMの有無を示す引数追加
*/
BOOL CEditDoc::SaveFileDialog( char* pszPath, int* pnCharCode, CEOL* pcEol, BOOL* pbBomExist )
{
	char**	ppszMRU;		//	最近のファイル
	char**	ppszOPENFOLDER;	//	最近のフォルダ
	const char*	pszDefFolder; // デフォルトフォルダ
	char*	pszCurDir = NULL;
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
	// ファイル名の無いときはカレントフォルダをデフォルトにします。Mar. 30, 2003 genta
	// 掲示板要望 No.2699 (2003/02/05)
	if( !IsFilePathAvailable() ){
		// 2002.10.25 Moca さんのコードを流用 Mar. 23, 2003 genta
		int nCurDir;
		pszCurDir = new char[_MAX_PATH];
		nCurDir = ::GetCurrentDirectory( _MAX_PATH, pszCurDir );
		if( 0 == nCurDir || _MAX_PATH < nCurDir ){
			pszDefFolder = "";
		}else{
			pszDefFolder = pszCurDir;
		}
	}else{
		pszDefFolder = GetFilePath();
	}
	m_cDlgOpenFile.Create( m_hInstance, /*NULL*/m_hWnd, m_szDefaultWildCard, pszDefFolder,
		(const char **)ppszMRU, (const char **)ppszOPENFOLDER );

	/* ダイアログを表示 */
	//	Jul. 26, 2003 ryoji pbBomExist追加
	bret = m_cDlgOpenFile.DoModalSaveDlg( pszPath, pnCharCode, pcEol, pbBomExist );

	delete [] ppszMRU;
	delete [] ppszOPENFOLDER;
	delete [] pszCurDir;
	return bret;
}





/*! 共通設定 プロパティシート */
BOOL CEditDoc::OpenPropertySheet( int nPageNum/*, int nActiveItem*/ )
{
	int		i;
//	BOOL	bModify;
	
	// 2002.12.11 Moca この部分で行われていたデータのコピーをCPropCommonに移動・関数化
	// 共通設定の一時設定領域にSharaDataをコピーする
	m_cPropCommon.InitData();
	
	/* プロパティシートの作成 */
	if( m_cPropCommon.DoPropertySheet( nPageNum/*, nActiveItem*/ ) ){

		// 2002.12.11 Moca この部分で行われていたデータのコピーをCPropCommonに移動・関数化
		// ShareData に 設定を適用・コピーする
		m_cPropCommon.ApplyData();

		/* アクセラレータテーブルの再作成 */
		::SendMessage( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* フォントが変わった */
		for( i = 0; i < 4; ++i ){
			m_cEditViewArr[i].m_cTipWnd.ChangeFont( &(m_pShareData->m_Common.m_lf_kh) );
		}

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
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更しProperySheet内で取得するように
	//m_cPropTypes.m_CKeyWordSetMgr = m_pShareData->m_CKeyWordSetMgr;

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

	char	pszCap[1024];	//	Nov. 6, 2000 genta オーバーヘッド軽減のためHeap→Stackに変更

//	/* アイコン化されていない時はフルパス */
//	/* アイコン化されている時はファイル名のみ */
//	if( ::IsIconic( m_hWnd ) ){
//		bKillFocus = TRUE;
//	}else{
//		bKillFocus = FALSE;
//	}

	// From Here Apr. 04, 2003 genta / Apr.05 ShareDataのパラメータ利用に
	if( bKillFocus ){
		ExpandParameter( m_pShareData->m_Common.m_szWindowCaptionInactive,
			pszCap, sizeof( pszCap ));
	}
	else {
		ExpandParameter( m_pShareData->m_Common.m_szWindowCaptionActive,
			pszCap, sizeof( pszCap ));
	}
	// To Here Apr. 04, 2003 genta

	::SetWindowText( m_hwndParent, pszCap );

	//@@@ From Here 2003.06.13 MIK
	//タブウインドウのファイル名を通知
	ExpandParameter( m_pShareData->m_Common.m_szTabWndCaption, pszCap, sizeof( pszCap ));
	m_pcEditWnd->ChangeFileNameNotify( pszCap );
	//@@@ To Here 2003.06.13 MIK

	return;
}

/*! バックアップの作成
	@author genta
	@date 2001.06.12 asa-o
		ファイルの時刻を元にバックアップファイル名を作成する機能
	@date 2001.12.11 MIK バックアップファイルをゴミ箱に入れる機能
	@date 2004.06.05 genta バックアップ対象ファイルを引数で与えるように．
		名前を付けて保存の時は自分のバックアップを作っても無意味なので．
		また，バックアップも保存も行わない選択肢を追加．

	@retval 0 バックアップ作成失敗．
	@retval 1 バックアップ作成成功．
	@retval 2 バックアップ作成失敗．保存中断指示．
	@retval 3 ファイル操作エラーによるバックアップ作成失敗．
*/
int CEditDoc::MakeBackUp( const char* target_file )
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

	/* バックアップソースの存在チェック */
	if( (_access( target_file, 0 )) == -1 ){
		return 0;
	}

	/* パスの分解 */
	_splitpath( target_file, szDrive, szDir, szFname, szExt );

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
		wsprintf( pBase, "%s.bak", szFname );
		break;
	case 5: //	Jun.  5, 2005 genta 1の拡張子を残す版
		wsprintf( pBase, "%s%s.bak", szFname, szExt );
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

			hFile = ::CreateFile(target_file,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
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
	case 6: //	Jun.  5, 2005 genta 3の拡張子を残す版
		//	Aug. 15, 2000 genta
		//	ここでは作成するバックアップファイル名のみ生成する．
		//	ファイル名のRotationは確認ダイアログの後で行う．
		{
			//	Jun.  5, 2005 genta 拡張子を残せるように処理起点を操作する
			char* ptr;
			if( m_pShareData->m_Common.GetBackupType() == 3 ){
				ptr = szExt;
			}
			else {
				ptr = szExt + strlen( szExt );
			}
			*ptr   = '.';
			*++ptr = m_pShareData->m_Common.GetBackupExtChar();
			*++ptr = '0';
			*++ptr = '0';
			*++ptr = '\0';
		}
		wsprintf( pBase, "%s%s", szFname, szExt );
		break;
	}

	//@@@ 2002.03.23 start ネットワーク・リムーバブルドライブの場合はごみ箱に放り込まない
	bool dustflag = false;
	if( m_pShareData->m_Common.m_bBackUpDustBox ){
		if( szPath[0] == '\\' && szPath[1] == '\\' ) dustflag = true;
		else if( isalpha(szPath[0]) ){
			char	szDriveType[_MAX_DRIVE+1];	// "A:\"登録用
			long	lngRet;
			sprintf(szDriveType, "%c:\\", toupper(szPath[0]));
			lngRet = GetDriveType( szDriveType );
			if( lngRet == DRIVE_REMOVABLE || lngRet == DRIVE_CDROM || lngRet == DRIVE_REMOTE ) dustflag = true;
		}
	}
	//@@@ 2002.03.23 end

	if( m_pShareData->m_Common.m_bBackUpDialog ){	/* バックアップの作成前に確認 */
		::MessageBeep( MB_ICONQUESTION );
//From Here Feb. 27, 2001 JEPROtest キャンセルもできるようにし、メッセージを追加した
//		if( IDYES != MYMESSAGEBOX(
//			m_hWnd,
//			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
//			"バックアップ作成の確認",
//			"変更される前に、バックアップファイルを作成します。\nよろしいですか？\n\n%s\n    ↓\n%s\n\n",
//			IsFilePathAvailable() ? GetFilePath() : "（無題）",
//			szPath
//		) ){
//			return FALSE;
//		}
		if( m_pShareData->m_Common.m_bBackUpDustBox && dustflag == false ){	//@@@ 2001.12.11 add start MIK	//2002.03.23
			nRet = ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				"バックアップ作成の確認",
				"変更される前に、バックアップファイルを作成します。\nよろしいですか？  [いいえ(N)] を選ぶと作成せずに上書き（または名前を付けて）保存になります。\n\n%s\n    ↓\n%s\n\n作成したバックアップファイルをごみ箱に放り込みます。\n",
				target_file,
				szPath
			);
		}else{	//@@@ 2001.12.11 add end MIK
			nRet = ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				"バックアップ作成の確認",
				"変更される前に、バックアップファイルを作成します。\nよろしいですか？  [いいえ(N)] を選ぶと作成せずに上書き（または名前を付けて）保存になります。\n\n%s\n    ↓\n%s\n\n",
				IsFilePathAvailable() ? GetFilePath() : "（無題）",
				szPath
			);	//Jul. 06, 2001 jepro [名前を付けて保存] の場合もあるのでメッセージを修正
		}	//@@@ 2001.12.11 add MIK
		//	Jun.  5, 2005 genta 戻り値変更
		if( IDNO == nRet ){
			return 0;//	保存継続
		}else if( IDCANCEL == nRet ){
			return 2;// 保存中断
		}
//To Here Feb. 27, 2001
	}

	//	From Here Aug. 16, 2000 genta
	//	Jun.  5, 2005 genta 1の拡張子を残す版を追加
	if( m_pShareData->m_Common.GetBackupType() == 3 ||
		m_pShareData->m_Common.GetBackupType() == 6 ){
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
				//	Jun.  5, 2005 genta 戻り値変更
				//	失敗しても保存は継続
				return 0;
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
				//	Jun.  5, 2005 genta 戻り値変更
				//	失敗しても保存は継続
				return 0;
			}
		}
	}
	//	To Here Aug. 16, 2000 genta

	//::MessageBox( NULL, szPath, "直前のバックアップファイル", MB_OK );
	/* バックアップの作成 */
	if( ::CopyFile( GetFilePath(), szPath, FALSE ) ){
		/* 正常終了 */
		//@@@ 2001.12.11 start MIK
		if( m_pShareData->m_Common.m_bBackUpDustBox && dustflag == false ){	//@@@ 2002.03.23 ネットワーク・リムーバブルドライブでない
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
		//	Jun.  5, 2005 genta 戻り値変更
		return 3;
	}
	//	Jun.  5, 2005 genta 戻り値変更
	return 1;
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
	if( -1 == _access( GetFilePath(), 0 ) ){
		/* ファイルの排他制御モード */
		m_nFileShareModeOld = 0;
		return;
	}else{
		/* ファイルの排他制御モード */
		m_nFileShareModeOld = m_pShareData->m_Common.m_nFileShareMode;
	}


	/* ファイルを開いていない */
	if( ! IsFilePathAvailable() ){
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
	if( -1 == _access( GetFilePath(), 2 ) ){	/* アクセス権：書き込み許可 */
#if 0
		// Apr. 28, 2000 genta: Request from Koda

		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"現在\n%s\nは読取専用に設定されています。 または、書き込みのアクセス権がありません。",
			IsFilePathAvailable() ? GetFilePath() : "（無題）"
		);
#endif
		m_hLockedFile = NULL;
		/* 親ウィンドウのタイトルを更新 */
		SetParentCaption();
		return;
	}


	m_hLockedFile = ::_lopen( GetFilePath(), OF_READWRITE );
	_lclose( m_hLockedFile );
	if( HFILE_ERROR == m_hLockedFile ){
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\nは現在他のプロセスによって書込みが禁止されています。",
			IsFilePathAvailable() ? GetFilePath() : "（無題）"
		);
		m_hLockedFile = NULL;
		/* 親ウィンドウのタイトルを更新 */
		SetParentCaption();
		return;
	}
	m_hLockedFile = ::_lopen( GetFilePath(), nAccessMode | m_pShareData->m_Common.m_nFileShareMode );
	if( HFILE_ERROR == m_hLockedFile ){
		switch( m_pShareData->m_Common.m_nFileShareMode ){
		case OF_SHARE_EXCLUSIVE:	/* 読み書き */
			pszMode = "読み書き禁止モード";
			break;
		case OF_SHARE_DENY_WRITE:	/* 書き */
			pszMode = "書き込み禁止モード";
			break;
		default:
			pszMode = "未定義のモード（問題があります）";
			break;
		}
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\nを%sでロックできませんでした。\n現在このファイルに対する排他制御は無効となります。",
			IsFilePathAvailable() ? GetFilePath() : "（無題）",
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

	@param bVisibleMemberFunc クラス、構造体定義内のメンバ関数の宣言をアウトライン解析結果に登録する場合はtrue

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
	- 3: クラス("クラス")
	- 4: 構造体 ("構造体")
	- 5: 列挙体("列挙体")
	- 6: 共用体("共用体")
	- 7: 名前空間("名前空間")

	@param pcFuncInfoArr [out] 関数一覧を返すためのクラス。
	ここに関数のリストを登録する。
*/
void CEditDoc::MakeFuncList_C( CFuncInfoArr* pcFuncInfoArr ,bool bVisibleMemberFunc )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;

	// 2002/10/27 frozen　ここから
	// nNestLevelを nNestLevel_global を nNestLevel_func に分割した。
	int			nNestLevel_global = 0;	// nNestLevel_global 関数外の {}のレベル  
	int			nNestLevel_func   = 0;	//	nNestLevel_func 関数の定義、および関数内の	{}のレベル
//	int			nNestLevel2;		//	nNestLevel2	()に対する位置 // 2002/10/27 frozen nNastLevel_fparamとnMode2のM2_FUNC_NAME_ENDで代用
	int			nNestLevel_fparam = 0;	// ()のレベル
	int			nNestPoint_class = 0; // 外側から何番目の{がクラスの定義を囲む{か？ (一番外側なら1、0なら無し。bVisibleMemberFuncがfalseの時のみ有効。trueでは常に0)
	// 2002/10/27 frozen　ここまで

	int			nCharChars;			//	多バイト文字を読み飛ばすためのもの
	char		szWordPrev[256];	//	1つ前のword
	char		szWord[256];		//	現在解読中のwordを入れるところ
	int			nWordIdx = 0;
	int			nMaxWordLeng = 100;	//	許容されるwordの最大長さ
	int			nMode;				//	現在のstate

	// 2002/10/27 frozen　ここから
	//! 状態2
	enum MODE2
	{
		M2_NORMAL			= 0x00,	//!< 通常
		M2_NAMESPACE_SAVE	= 0x11,	//!< ネームスペース名調査中
			// 「通常」状態で単語 "class" "struct" "union" "enum" "namespace"を読み込むと、この状態になり、';' '{' ',' '>' '='を読み込むと「通常」になる。
			//
			// ':' を読み込むと「ネームスペース名調査完了」へ移行すると同時に
			// szWordをszTokenNameに保存し、あとで ':' 又は '{' の直前の単語が調べられるようにしている。
			// これは "__declspec( dllexport )"のように"class"とクラス名の間にキーワードが書いてある場合でもクラス名を取得できるようにするため。
			//
			// '<' を読み込むと「テンプレートクラス名調査中」に移行する。
		M2_TEMPLATE_SAVE	= 0x12, //!< テンプレートクラス名調査中
			// ';' '{'を読み込むと「通常」になる。
			// また、この状態の間は単語を区切る方法を一時的に変更し、
			// 「template_name <paramA,paramB>」のような文字列を一つの単語をみなすようにする。
			// これは特殊化したクラステンプレートを実装する際の構文で有効に働く。	
		M2_NAMESPACE_END	= 0x13,	//!< ネームスペース名調査完了。(';' '{' を読み込んだ時点で「通常」になる。 )
		M2_FUNC_NAME_END	= 0x14, //!< 関数名調査完了。(';' '{' を読み込んだ時点で「通常」になる。 )
		M2_AFTER_EQUAL		= 0x05,	//!< '='の後。
			//「通常」かつ nNestLevel_fparam==0 で'='が見つかるとこの状態になる。（ただし "opreator"の直後は除く）
			// ';'が見つかると「通常」に戻る。
			// int val=abs(-1);
			// のような文が関数とみなされないようにするために使用する。
		M2_AFTER_ITEM		= 0x10,
	} nMode2 = M2_NORMAL;
//	char		szFuncName[256];	//	関数名

	const int	nNamespaceNestMax	= 32;			//!< ネスト可能なネームスペース、クラス等の最大数
	int			nNamespaceLen[nNamespaceNestMax+1];	//!< ネームスペース全体の長さ
	const int	nNamespaceLenMax 	= 512;			//!< 最大のネームスペース全体の長さ
	char		szNamespace[nNamespaceLenMax];		//!< 現在のネームスペース(終端が\0になっているとは限らないので注意)
	const int 	nItemNameLenMax	 	= 256;
	char		szItemName[nItemNameLenMax];		//!< すぐ前の 関数名 or クラス名 or 構造体名 or 共用体名 or 列挙体名 or ネームスペース名
	// 例えば下のコードの←の部分での
	// szNamespaceは"Namespace\ClassName\"
	// nMamespaceLenは{10,20}
	// nNestLevel_globalは2となる。
	//
	//　namespace Namespace{
	//　class ClassName{
	//　←
	//　}}

	int			nItemLine;			//!< すぐ前の 関数 or クラス or 構造体 or 共用体 or 列挙体 or ネームスペースのある行
	int			nItemFuncId;

//	int			nFuncLine;
//	int			nFuncId;
	
//	int			nFuncNum;			// 使っていないようなので削除
	// 2002/10/27 frozen　ここまで

	//	Mar. 4, 2001 genta
	bool		bLineTop;			//	行頭かどうかを判別するためのフラグ
									//	プリプロセッサ指令用

	//	Mar. 4, 2001 genta
//	bool		bCppInitSkip;		//	C++のメンバー変数、親クラスの初期化子をSKIP // 2002/10/27 frozen nMode2の機能で代用
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	szNamespace[0] = '\0';	// 2002/10/27 frozen
	nNamespaceLen[0] = 0;	// 2002/10/27 frozen
	nMode = 0;
//	FuncNum = 0;
//	bCppInitSkip = false;
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
						if( pLine[i] == ':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									strcpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								strncat( szItemName, szWord, nItemNameLenMax - strlen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = '\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}
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
					// 2002/10/27 frozen　ここから
					if( nMode2 == M2_NAMESPACE_SAVE )
						strcpy( szItemName, szWord );
					else if( nMode2 == M2_TEMPLATE_SAVE)
					{
						strncat( szItemName, szWord, nItemNameLenMax - strlen(szItemName) );
						szItemName[ nItemNameLenMax - 1 ] = '\0';
					}
					else if( nNestLevel_func == 0 && nMode2 == M2_NORMAL )
					{
						nItemFuncId = 0;
						if( strcmp(szWord,"class")==0 )
							nItemFuncId = 3;
						if( strcmp(szWord,"struct")==0 )
							nItemFuncId = 4;
						else if( strcmp(szWord,"namespace")==0 )
							nItemFuncId = 7;
						else if( strcmp(szWord,"enum")==0 )
							nItemFuncId = 5;
						else if( strcmp(szWord,"union")==0 )
							nItemFuncId = 6;
						if( nItemFuncId != 0 )
						{
							nMode2 = M2_NAMESPACE_SAVE;
							nItemLine = nLineCount + 1;
							strcpy(szItemName,"無名");
						}
					}
					// 2002/10/27 frozen　ここまで

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
				
				// 2002/10/27 frozen ここから
				if( '{' == pLine[i] )
				{
					int nItemNameLen;
					if( nNestLevel_func !=0)
						++nNestLevel_func;
					else if(
							(nMode2 & M2_AFTER_ITEM) != 0  &&
							nNestLevel_global < nNamespaceNestMax &&
							(nNamespaceLen[nNestLevel_global] +  (nItemNameLen = strlen(szItemName)) + 10 + 1) < nNamespaceLenMax)
					// ３番目の(&&の後の)条件
					// バッファが足りない場合は項目の追加を行わない。
					// +10は追加する文字列の最大長(追加する文字列は"::定義位置"が最長)
					// +1は終端NUL文字
					{
						strcpy( &szNamespace[nNamespaceLen[nNestLevel_global]] , szItemName);
						if( nMode2 == M2_FUNC_NAME_END )
							++ nNestLevel_func;
						else
						{
							++ nNestLevel_global;
							nNamespaceLen[nNestLevel_global] = nNamespaceLen[nNestLevel_global-1] + nItemNameLen;
							if( nItemFuncId == 7)
								strcpy(&szNamespace[nNamespaceLen[nNestLevel_global]],"::定義位置");
							else
							{
								szNamespace[nNamespaceLen[nNestLevel_global]] = '\0';
								szNamespace[nNamespaceLen[nNestLevel_global]+1] = ':';
								if(bVisibleMemberFunc == false && nNestPoint_class == 0)
									nNestPoint_class = nNestLevel_global;
							}
						}
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
							nItemLine - 1,
							&nPosX,
							&nPosY
						);
						pcFuncInfoArr->AppendData( nItemLine, nPosY + 1 , szNamespace, nItemFuncId);
						if( nMode2 != M2_FUNC_NAME_END )
						{
							szNamespace[nNamespaceLen[nNestLevel_global]] = ':';
							nNamespaceLen[nNestLevel_global] += 2;
						}
					}
					else
					{
						if(nMode2 == M2_FUNC_NAME_END)
							++ nNestLevel_func;
						else
						{
							++ nNestLevel_global;
							if ( nNestLevel_global <= nNamespaceNestMax )
								nNamespaceLen[nNestLevel_global]=nNamespaceLen[nNestLevel_global-1];
						}
					}
					// bCppInitSkip = false;	//	Mar. 4, 2001 genta
					nMode = 0;
					nMode2 = M2_NORMAL;
					// nNestLevel2 = 0;
					continue;
				}else
				// 2002/10/27 frozen ここまで
				
				if( '}' == pLine[i] ){
					//  2002/10/27 frozen ここから
//					nNestLevel2 = 0;
					if(nNestLevel_func == 0)
					{
						if(nNestLevel_global!=0)
						{
							if(nNestLevel_global == nNestPoint_class)
								nNestPoint_class = 0;
							--nNestLevel_global;
						}
					}
					else
						--nNestLevel_func;
					//  2002/10/27 frozen ここまで
					nMode = 0;
					nMode2 = M2_NORMAL;
					continue;
				}else
				if( '(' == pLine[i] ){
					//  2002/10/27 frozen ここから
//					if( nNestLevel == 0 && !bCppInitSkip ){
//						strcpy( szFuncName, szWordPrev );
//						nFuncLine = nLineCount + 1;
//						nNestLevel2 = 1;
//					}
//					nMode = 0;
					if( nNestLevel_func == 0 && nMode2 == M2_NORMAL )
					{
						if(nNestLevel_fparam==0)
						{
							strcpy( szItemName, szWordPrev);
							nItemLine = nLineCount + 1;
						}
						++ nNestLevel_fparam;
					}
					//  2002/10/27 frozen ここまで
					continue;
				}else
				if( ')' == pLine[i] ){
					//  2002/10/27 frozen ここから
//					if( 1 == nNestLevel2 ){
//						nNestLevel2 = 2;
//					}
//					nMode = 0;
					if( nNestLevel_fparam > 0)
					{
						--nNestLevel_fparam;
						if( nNestLevel_fparam == 0)
						{
							nMode2 = M2_FUNC_NAME_END;
							nItemFuncId = 2;
						}
					}
					//  2002/10/27 frozen ここまで
					continue;
				}else
				if( ';' == pLine[i] ){
					//  2002/10/27 frozen ここから
//					if( 2 == nNestLevel2 ){
//						//	閉じ括弧')'の後の';' すなわち関数宣言
//						if( 0 != strcmp( "sizeof", szFuncName ) ){
//							nFuncId = 1;
//							++nFuncNum;
//							/*
//							  カーソル位置変換
//							  物理位置(行頭からのバイト数、折り返し無し行位置)
//							  →
//							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
//							*/
//							int		nPosX;
//							int		nPosY;
//							m_cLayoutMgr.CaretPos_Phys2Log(
//								0,
//								nFuncLine - 1,
//								&nPosX,
//								&nPosY
//							);
//							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szFuncName, nFuncId);
////						pcFuncInfoArr->AppendData( nFuncLine, szFuncName, nFuncId );
//						}
//					}
//					nNestLevel2 = 0;
					if(
						nMode2 == M2_FUNC_NAME_END &&
						nNestLevel_global < nNamespaceNestMax &&
						(nNamespaceLen[nNestLevel_global] + strlen(szItemName)) < nNamespaceLenMax &&
						nNestPoint_class == 0)
					// ３番目の(&&の後の)条件
					// バッファが足りない場合は項目の追加を行わない。
					{
						strcpy( &szNamespace[nNamespaceLen[ nNestLevel_global]] , szItemName);

						nItemFuncId = 1;
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
							nItemLine - 1,
							&nPosX,
							&nPosY
						);
						pcFuncInfoArr->AppendData( nItemLine, nPosY + 1, szNamespace, nItemFuncId);
					}
					nMode2 = M2_NORMAL;
					//  2002/10/27 frozen ここまで
					nMode = 0;
					continue;
				}else{
					if( C_IsWordChar( pLine[i] ) ){
						//  2002/10/27 frozen ここから削除
//						if( 2 == nNestLevel2 ){
//							//	閉じ括弧が無いけどとりあえず登録しちゃう
//							if( 0 != strcmp( "sizeof", szFuncName ) ){
//								nFuncId = 2;
//								++nFuncNum;
//								/*
//								  カーソル位置変換
//								  物理位置(行頭からのバイト数、折り返し無し行位置)
//								  →
//								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
//								*/
//								int		nPosX;
//								int		nPosY;
//								m_cLayoutMgr.CaretPos_Phys2Log(
//									0,
//									nFuncLine - 1,
//									&nPosX,
//									&nPosY
//								);
//								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
//							}
//							nNestLevel2 = 0;
//							//	Mar 4, 2001 genta	初期化子だったときはそれ以降の登録を制限する
//							if( pLine[i] == ':' )
//								bCppInitSkip = true;
//						}
						//  2002/10/27 frozen ここまで削除

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
						
						if( pLine[i] == ':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									strcpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								strncat( szItemName, szWord, nItemNameLenMax - strlen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = '\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}

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

						// 2002/10/27 frozen ここから
						if( nMode2 == M2_NAMESPACE_SAVE )
						{
							if( pLine[i] == '>' || pLine[i] == ',' || pLine[i] == '=')
								// '<' の前に '>' , ',' , '=' があったので、おそらく
								// 前にあった"class"はテンプレートパラメータの型を表していたと考えられる。
								// よって、クラス名の調査は終了。
								// '>' はテンプレートパラメータの終了
								// ',' はテンプレートパラメータの区切り
								// '=' はデフォルトテンプレートパラメータの指定
								nMode2 = M2_NORMAL; 
							else if( pLine[i] == '<' )
								nMode2 = M2_TEMPLATE_SAVE;
						}
						else if( pLine[i] == '=' && nNestLevel_func == 0 && nNestLevel_fparam==0 && nMode2 == M2_NORMAL && strcmp(szWordPrev,"operator")!=0)
							nMode2 = M2_AFTER_EQUAL;

						if( nMode2 == M2_TEMPLATE_SAVE)
						{
							int nItemNameLen = strlen(szItemName);
							if(nItemNameLen + 1 < nItemNameLenMax )
							{
								szItemName[nItemNameLen] = pLine[i];
								szItemName[nItemNameLen + 1 ] = '\0';
							}
						}
						// 2002/10/27 frozen ここまで
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





/*!	テキスト・トピックリスト作成
	
	@date 2002.04.01 YAZAKI CDlgFuncList::SetText()を使用するように改訂。
	@date 2002.11.03 Moca 階層が最大値を超えるとバッファオーバーランするのを修正
		最大値以上は追加せずに無視する
*/
void CEditDoc::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						nCharChars;
	int						nCharChars2;
	const char*				pszStarts;
	int						nStartsLen;
	char*					pszText;


	pszStarts = m_pShareData->m_Common.m_szMidashiKigou; 	/* 見出し記号 */
	nStartsLen = lstrlen( pszStarts );

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	char pszStack[nMaxStack][32];
	char szTitle[32];			//	一時領域
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
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
					strncpy( szTitle, &pszStarts[j], nCharChars);	//	szTitleに保持。
					szTitle[nCharChars] = '\0';
					break;
				}
			}
		}
		if( j >= nStartsLen ){
			continue;
		}
		/* 見出し文字に(が含まれていることが前提になっている! */
		if( nCharChars == 1 && pLine[i] == '(' ){
			if( pLine[i + 1] >= '0' && pLine[i + 1] <= '9' )  {
				strcpy( szTitle, "(0)" );
			}
			else if ( pLine[i + 1] >= 'A' && pLine[i + 1] <= 'Z' ) {
				strcpy( szTitle, "(A)" );
			}
			else if ( pLine[i + 1] >= 'a' && pLine[i + 1] <= 'z' ) {
				strcpy( szTitle, "(a)" );
			}
			else {
				continue;
			}
		}else
		if( 2 == nCharChars ){
			// 2003.06.28 Moca 1桁目から始まっていないと同一レベルと認識されずに
			//	どんどん子ノードになってしまうのを，字下げによらず同一レベルと認識されるように
			/* 全角数字 */
			if( pLine[i] == 0x82 && ( pLine[i + 1] >= 0x4f && pLine[i + 1] <= 0x58 ) ) {
				strcpy( szTitle, "０" );
			}
			/* ①～⑳ */
			else if( pLine[i] == 0x87 && ( pLine[i + 1] >= 0x40 && pLine[i + 1] <= 0x53 ) ){
				strcpy( szTitle, "①" );
			}
			/* Ⅰ～Ⅹ */
			else if( pLine[i] == 0x87 && ( pLine[i + 1] >= 0x54 && pLine[i + 1] <= 0x5d ) ){
				strcpy( szTitle, "Ⅰ" );
			}
			// 2003.06.28 Moca 漢数字も同一階層に
			//	漢数字が異なる＝番号が異なると異なる見出し記号と認識されていたのを
			//	皆同じ階層と識別されるように
			else{
				char szCheck[3];
				szCheck[0] = pLine[i];
				szCheck[1] = pLine[i + 1];
				szCheck[2] = '\0';
				/* 一～十 */
				if( NULL != strstr( "〇一二三四五六七八九十百零壱弐参伍", szCheck ) ){
					strcpy( szTitle, "一" );
				}
			}
		}
		/*	「見出し記号」に含まれる文字で始まるか、
			(0、(1、...(9、(A、(B、...(Z、(a、(b、...(z
			で始まる行は、アウトライン結果に表示する。
		*/
		pszText = new char[nLineLen + 1];
		memcpy( pszText, (const char *)&pLine[i], nLineLen );
		pszText[nLineLen] = '\0';
		for( i = 0; i < (int)lstrlen(pszText); ++i ){
			if( pszText[i] == CR ||
				pszText[i] == LF ){
				pszText[i] = '\0';
			}
		}
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
		/* nDepthを計算 */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = strcmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	ループ途中でbreak;してきた。＝今までに同じ見出しが存在していた。
			//	ので、同じレベルに合わせてAppendData.
			nDepth = k;
		}
		else if( nMaxStack > k ){
			//	いままでに同じ見出しが存在しなかった。
			//	ので、pszStackにコピーしてAppendData.
			strcpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーラン
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}


/*! ルールファイルの1行を管理する構造体

	@date 2002.04.01 YAZAKI
*/
struct oneRule {
	char szMatch[256];
	int  nLength;
	char szGroupName[256];
};

/*! ルールファイルを読み込み、ルール構造体の配列を作成する

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca 引数nMaxCountを追加。バッファ長チェックをするように変更
*/
int CEditDoc::ReadRuleFile( char* pszFilename, oneRule* pcOneRule, int nMaxCount )
{
	long	i;
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	FILE*	pFile = fopen_absexe( pszFilename, "r" );
	if( NULL == pFile ){
		return 0;
	}
	char	szLine[10240];
	const char*	pszDelimit = " /// ";
	const char*	pszKeySeps = ",\0";
	char*	pszWork;
	int nDelimitLen = strlen( pszDelimit );
	int nCount = 0;
	while( NULL != fgets( szLine, sizeof(szLine), pFile ) && nCount < nMaxCount ){
		pszWork = strstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != ';' ){
			*pszWork = '\0';
			pszWork += nDelimitLen;

			/* 最初のトークンを取得します。 */
			char* pszToken = strtok( szLine, pszKeySeps );
			while( NULL != pszToken ){
//				nRes = _stricmp( pszKey, pszToken );
				for( i = 0; i < (int)lstrlen(pszWork); ++i ){
					if( pszWork[i] == '\r' ||
						pszWork[i] == '\n' ){
						pszWork[i] = '\0';
						break;
					}
				}
				strncpy( pcOneRule[nCount].szMatch, pszToken, 255 );
				strncpy( pcOneRule[nCount].szGroupName, pszWork, 255 );
				pcOneRule[nCount].szMatch[255] = '\0';
				pcOneRule[nCount].szGroupName[255] = '\0';
				pcOneRule[nCount].nLength = strlen(pcOneRule[nCount].szMatch);
				nCount++;
				pszToken = strtok( NULL, pszKeySeps );
			}
		}
	}
	fclose( pFile );
	return nCount;
}

/*! ルールファイルを元に、トピックリストを作成

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca ネストの深さが最大値を超えるとバッファオーバーランするのを修正
		最大値以上は追加せずに無視する
*/
void CEditDoc::MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	char*					pszText;

	/* ルールファイルの内容をバッファに読み込む */
	oneRule test[1024];	//	1024個許可。
	int nCount = ReadRuleFile(GetDocumentAttribute().m_szOutlineRuleFilename, test, 1024 );
	if ( nCount < 1 ){
		return;
	}

	/*	ネストの深さは、32レベルまで、ひとつのヘッダは、最長256文字まで区別
		（256文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	char pszStack[nMaxStack][256];
	char szTitle[256];			//	一時領域
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
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
		for( j = 0; j < nCount; j++ ){
			if ( 0 == strncmp( (const char*)&pLine[i], test[j].szMatch, test[j].nLength ) ){
				strcpy( szTitle, test[j].szGroupName );
				break;
			}
		}
		if( j >= nCount ){
			continue;
		}
		/*	ルールにマッチした行は、アウトライン結果に表示する。
		*/
		pszText = new char[nLineLen + 1];
		memcpy( pszText, (const char *)&pLine[i], nLineLen );
		pszText[nLineLen] = '\0';
		int nTextLen = lstrlen( pszText );
		for( i = 0; i < nTextLen; ++i ){
			if( pszText[i] == CR ||
				pszText[i] == LF ){
				pszText[i] = '\0';
				break;
			}
		}
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
		/* nDepthを計算 */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = strcmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	ループ途中でbreak;してきた。＝今までに同じ見出しが存在していた。
			//	ので、同じレベルに合わせてAppendData.
			nDepth = k;
		}
		else if( nMaxStack> k ){
			//	いままでに同じ見出しが存在しなかった。
			//	ので、pszStackにコピーしてAppendData.
			strcpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーランするから規制する
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)pszText, 0, nDepth );
			nDepth++;
		}
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


/*! アセンブラ アウトライン解析

	@author MIK
	@date 2004.04.12 作り直し
*/
void CEditDoc::MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr )
{
	int nTotalLine;

	nTotalLine = m_cDocLineMgr.GetLineCount();

	for( int nLineCount = 0; nLineCount < nTotalLine; nLineCount++ ){
		const TCHAR* pLine;
		int nLineLen;
		TCHAR* pTmpLine;
		int length;
		int offset;
#define MAX_ASM_TOKEN 2
		TCHAR* token[MAX_ASM_TOKEN];
		int j;
		TCHAR* p;

		//1行取得する。
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( pLine == NULL ) break;

		//作業用にコピーを作成する。バイナリがあったらその後ろは知らない。
		pTmpLine = _tcsdup( pLine );
		if( pTmpLine == NULL ) break;
		if( _tcslen( pTmpLine ) >= nLineLen ){	//バイナリを含んでいたら短くなるので...
			pTmpLine[ nLineLen ] = _T('\0');	//指定長で切り詰め
		}

		//行コメント削除
		p = _tcsstr( pTmpLine, _T(";") );
		if( p ) *p = _T('\0');

		length = _tcslen( pTmpLine );
		offset = 0;

		//トークンに分割
		for( j = 0; j < MAX_ASM_TOKEN; j++ ) token[ j ] = NULL;
		for( j = 0; j < MAX_ASM_TOKEN; j++ ){
			token[ j ] = my_strtok( pTmpLine, length, &offset, _T(" \t\r\n") );
			if( token[ j ] == NULL ) break;
			//トークンに含まれるべき文字でないか？
			if( _tcsstr( token[ j ], _T("\"")) != NULL
			 || _tcsstr( token[ j ], _T("\\")) != NULL
			 || _tcsstr( token[ j ], _T("'" )) != NULL ){
				token[ j ] = NULL;
				break;
			}
		}

		if( token[ 0 ] != NULL ){	//トークンが1個以上ある
			int nFuncId = -1;
			TCHAR* entry_token = NULL;

			length = _tcslen( token[ 0 ] );
			if( length >= 2
			 && token[ 0 ][ length - 1 ] == _T(':') ){	//ラベル
				token[ 0 ][ length - 1 ] = _T('\0');
				nFuncId = 51;
				entry_token = token[ 0 ];
			}else
			if( token[ 1 ] != NULL ){	//トークンが2個以上ある
				if( my_stricmp( token[ 1 ], _T("proc") ) == 0 ){	//関数
					nFuncId = 50;
					entry_token = token[ 0 ];
				}else
				if( my_stricmp( token[ 1 ], _T("endp") ) == 0 ){	//関数終了
					nFuncId = 52;
					entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("macro") ) == 0 ){	//マクロ
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("struc") ) == 0 ){	//構造体
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				}
			}

			if( nFuncId >= 0 ){
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
				pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, entry_token, nFuncId );
			}
		}

		free( pTmpLine );
	}

	return;
}



/*! 階層付きテキスト アウトライン解析

	@author zenryaku
	@date 2003.05.20 zenryaku 新規作成
	@date 2003.05.25 genta 実装方法一部修正
	@date 2003.06.21 Moca 階層が2段以上深くなる場合を考慮
*/
void CEditDoc::MakeTopicList_wztxt(CFuncInfoArr* pcFuncInfoArr)
{
	int levelPrev = 0;

	for(int nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const char*	pLine;
		int			nLineLen;

		pLine = m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta 判定順序変更
		if( *pLine == '.' )
		{
			const char* pPos;	//	May 25, 2003 genta
			int			nLength;
			char		szTitle[1024];

			//	ピリオドの数＝階層の深さを数える
			for( pPos = pLine + 1 ; *pPos == '.' ; ++pPos )
				;

			int	nPosX;
			int	nPosY;
			m_cLayoutMgr.CaretPos_Phys2Log(
				0,
				nLineCount,
				&nPosX,
				&nPosY
			);
			
			int level = pPos - pLine;

			// 2003.06.27 Moca 階層が2段位上深くなるときは、無題の要素を追加
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (無題)を挿入
				//	ただし，TAG一覧には出力されないように
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData( nLineCount+1, nPosY+1,
						"(無題)", FUNCINFO_NOCLIPTEXT, dummyLevel - 1 );
				}
			}
			levelPrev = level;

			nLength = wsprintf(szTitle,"%d - ", level );
			
			char *pDest = szTitle + nLength; // 書き込み先
			char *pDestEnd = szTitle + sizeof(szTitle) - 2;
			
			while( pDest < pDestEnd )
			{
				if( *pPos =='\r' || *pPos =='\n' || *pPos == '\0')
				{
					break;
				}
				//	May 25, 2003 genta 2バイト文字の切断を防ぐ
				else if( _IS_SJIS_1( *pPos )){
					*pDest++ = *pPos++;
					*pDest++ = *pPos++;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = '\0';
			pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1,szTitle, 0, level - 1);
		}
	}
}

/*! HTML アウトライン解析

	@author zenryaku
	@date 2003.05.20 zenryaku 新規作成
	@date 2004.04.20 Moca コメント処理と、不明な終了タグを無視する処理を追加
*/
void CEditDoc::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						k;
	BOOL					bEndTag;
	BOOL					bCommentTag = FALSE;

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	char pszStack[nMaxStack][32];
	char szTitle[32];			//	一時領域
	for(nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	(const unsigned char *)m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine)
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			// 2004.04.20 Moca コメントを処理する
			if( bCommentTag )
			{
				if( i < nLineLen - 4 && 0 == memcmp( "-->", pLine + i , 3 ) )
				{
					bCommentTag = FALSE;
					i += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if(pLine[i]!='<' || nDepth>=nMaxStack)
			{
				continue;
			}
			bEndTag	=	FALSE;
			if(pLine[++i]=='/')
			{
				i++;
				bEndTag	=	TRUE;
			}
			for(j=0;i+j<nLineLen && j<sizeof(szTitle)-1;j++)
			{
				if((pLine[i+j]<'a' || pLine[i+j]>'z') &&
					(pLine[i+j]<'A' || pLine[i+j]>'Z') &&
					!(j!=0 && pLine[i+j]>='0' && pLine[i+j]<='9'))
				{
					break;
				}
				szTitle[j]	=	pLine[i+j];
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here コメントを処理する
				if( i < nLineLen - 3 && 0 == memcmp( "!--", pLine + i, 3 ) )
				{
					bCommentTag = TRUE;
					i += 3;
				}
				// 2004.04.20 Moca To Here
				continue;
			}
			szTitle[j]	=	'\0';
			if(bEndTag)
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca 追加
				// 終了タグ
				while(nDepth>0)
				{
					nDepth--;
					if(!_stricmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca ツリー中と一致しないときは、この終了タグは無視
				if( nDepth == 0 )
				{
					if(_stricmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}
			}
			else
			{
				if(_stricmp(szTitle,"br") && _stricmp(szTitle,"area") &&
					_stricmp(szTitle,"base") && _stricmp(szTitle,"frame") && _stricmp(szTitle,"param"))
				{
					int		nPosX;
					int		nPosY;

					m_cLayoutMgr.CaretPos_Phys2Log(
						i,
						nLineCount,
						&nPosX,
						&nPosY
					);

					if(_stricmp(szTitle,"hr") && _stricmp(szTitle,"meta") && _stricmp(szTitle,"link") &&
						_stricmp(szTitle,"input") && _stricmp(szTitle,"img") && _stricmp(szTitle,"area") &&
						_stricmp(szTitle,"base") && _stricmp(szTitle,"frame") && _stricmp(szTitle,"param"))
					{
						// 終了タグなしを除く全てのタグらしきものを判定
						strcpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<sizeof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if(pLine[i+j]=='>')
								{
									break;
								}
							}
							szTitle[k++]	=	' ';
							for(j-=k-1;i+j+k<nLineLen && k<sizeof(szTitle)-1;k++)
							{
								if(pLine[i+j+k]=='<' || pLine[i+j+k]=='\r' || pLine[i+j+k]=='\n')
								{
									break;
								}
								szTitle[k]	=	pLine[i+j+k];
							}
							j	+=	k-1;
						}
						szTitle[k]	=	'\0';
						pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1,szTitle,0,nDepth++);
					}
					else
					{
						for(;i+j<nLineLen && j<sizeof(szTitle)-1;j++)
						{
							if(pLine[i+j]=='>')
							{
								break;
							}
							szTitle[j]	=	pLine[i+j];
						}
						szTitle[j]	=	'\0';
						pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1,szTitle,0,nDepth);
					}
				}
			}
			i	+=	j;
		}
	}
}

/*! TeX アウトライン解析

	@author naoh
	@date 2003.07.21 naoh 新規作成
*/
void CEditDoc::MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr)
{
	const char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						k;

	const int nMaxStack = 8;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	char szTag[32], szTitle[256];			//	一時領域
	int thisSection=0, lastSection = 0;	// 現在のセクション種類と一つ前のセクション種類
	int stackSection[nMaxStack];		// 各深さでのセクションの番号
	int nStartTitlePos;					// \section{dddd} の dddd の部分の始まる番号
	int bNoNumber;						// * 付の場合はセクション番号を付けない

	for(nLineCount=0;nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	(const char *)m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine) break;
		for(i=0;i<nLineLen-1;i++)
		{
			if(pLine[i] == '%') break;
			if(pLine[i] != '\\' || nDepth>=nMaxStack) continue;
			++i;
			for(j=0;i+j<nLineLen && j<sizeof(szTag)-1;j++)
			{
				if(pLine[i+j] == '{'){
					bNoNumber = (pLine[i+j-1] == '*');
					nStartTitlePos = j+i+1;
					break;
				}
				szTag[j]	=	pLine[i+j];
			}
			if(j==0) continue;
			if(bNoNumber){
				szTag[j-1] = '\0';
			}else{
				szTag[j]	 = '\0';
			}
//			MessageBox(NULL, szTitle, "", MB_OK);

			if(!strcmp(szTag,"subsubsection")) thisSection = 4;
			else if(!strcmp(szTag,"subsection")) thisSection = 3;
			else if(!strcmp(szTag,"section")) thisSection = 2;
			else if(!strcmp(szTag,"chapter")) thisSection = 1;
			else thisSection = 0;
			if( thisSection > 0)
			{
				// sectionの中身取得
				for(k=0;nStartTitlePos+k<nLineLen && k<sizeof(szTitle)-1;k++)
				{
					if(pLine[k+nStartTitlePos] == '}') break;
					szTitle[k]	=	pLine[k+nStartTitlePos];
				}
				szTitle[k] = '\0';

				int		nPosX;
				int		nPosY;
				TCHAR tmpstr[256];
				TCHAR secstr[4];

				m_cLayoutMgr.CaretPos_Phys2Log(
					i,
					nLineCount,
					&nPosX,
					&nPosY
				);

				int sabunSection = thisSection - lastSection;
				if(lastSection == 0){
					nDepth = 0;
					stackSection[0] = 1;
				}else{
					nDepth += sabunSection;
					if(sabunSection > 0){
						if(nDepth >= nMaxStack) nDepth=nMaxStack-1;
						stackSection[nDepth] = 1;
					}else{
						if(nDepth < 0) nDepth=0;
						++stackSection[nDepth];
					}
				}
				tmpstr[0] = '\0';
				if(!bNoNumber){
					for(k=0; k<=nDepth; k++){
						sprintf(secstr, "%d.", stackSection[k]);
						strcat(tmpstr, secstr);
					}
					strcat(tmpstr, " ");
				}
				strcat(tmpstr, szTitle);
				pcFuncInfoArr->AppendData(nLineCount+1,nPosY+1, tmpstr, 0, nDepth);
				if(!bNoNumber) lastSection = thisSection;
			}
			i	+=	j;
		}
	}
}




/* アクティブなペインを設定 */
void  CEditDoc::SetActivePane( int nIndex )
{
	m_cEditViewArr[m_nActivePaneIndex].OnKillFocus();
	m_cEditViewArr[m_nActivePaneIndex].m_cUnderLine.CaretUnderLineOFF(TRUE);	//	2002/05/11 YAZAKI

	/* アクティブなビューを切り替える */
	m_nActivePaneIndex = nIndex;

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

	//	2002/05/08 YAZAKI OnKillFocus()とOnSetFocus()で、アンダーラインを制御するようにした。
	//	2001/06/20 Start by asa-o:	アクティブでないペインのカーソルアンダーバーを非表示
	//	m_cEditViewArr[m_nActivePaneIndex].CaretUnderLineON(TRUE);
	//	m_cEditViewArr[m_nActivePaneIndex^1].CaretUnderLineOFF(TRUE);
	//	m_cEditViewArr[m_nActivePaneIndex^2].CaretUnderLineOFF(TRUE);
	//	m_cEditViewArr[(m_nActivePaneIndex^2)^1].CaretUnderLineOFF(TRUE);
	//	2001/06/20 End

	return;
}



/* アクティブなペインを取得 */
int CEditDoc::GetActivePane( void )
{
	return m_nActivePaneIndex;
}



/* 非アクティブなペインをRedrawする */
void CEditDoc::RedrawInactivePane(void)
{
	if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
		m_cEditViewArr[m_nActivePaneIndex^1].Redraw();
	}
	if ( m_cSplitterWnd.GetAllSplitRows() == 2 ){
		m_cEditViewArr[m_nActivePaneIndex^2].Redraw();
		if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
			m_cEditViewArr[(m_nActivePaneIndex^1)^2].Redraw();
		}
	}
}

/* すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する） */
BOOL CEditDoc::DetectWidthOfLineNumberAreaAllPane( BOOL bRedraw )
{
	if ( m_cEditViewArr[m_nActivePaneIndex].DetectWidthOfLineNumberArea( bRedraw ) ){
		/* ActivePaneで計算したら、再設定・再描画が必要と判明した */
		if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
			m_cEditViewArr[m_nActivePaneIndex^1].DetectWidthOfLineNumberArea( bRedraw );
		}
		else {
			//	表示されていないので再描画しない
			m_cEditViewArr[m_nActivePaneIndex^1].DetectWidthOfLineNumberArea( FALSE );
		}
		if ( m_cSplitterWnd.GetAllSplitRows() == 2 ){
			m_cEditViewArr[m_nActivePaneIndex^2].DetectWidthOfLineNumberArea( bRedraw );
			if ( m_cSplitterWnd.GetAllSplitCols() == 2 ){
				m_cEditViewArr[(m_nActivePaneIndex^1)^2].DetectWidthOfLineNumberArea( bRedraw );
			}
		}
		else {
			m_cEditViewArr[m_nActivePaneIndex^2].DetectWidthOfLineNumberArea( FALSE );
			m_cEditViewArr[(m_nActivePaneIndex^1)^2].DetectWidthOfLineNumberArea( FALSE );
		}
		return TRUE;
	}
	return FALSE;
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
				m_pcEditWnd->TabWnd_SucceedWindowPlacement( m_hwndParent, hwndWork );	//@@@ 2003.06.23 MIK
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
				m_pcEditWnd->TabWnd_SucceedWindowPlacement( m_hwndParent, hwndWork );	//@@@ 2003.06.23 MIK
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




/*! ビューに設定変更を反映させる

	@date 2004.06.09 Moca レイアウト再構築中にProgress Barを表示する．

*/
void CEditDoc::OnChangeSetting( void )
{
//	return;
	int			i;
	HWND		hwndProgress = NULL;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta

	pCEditWnd->m_CFuncKeyWnd.m_nCurrentKeyState = -1;

	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
		//	Status Barが表示されていないときはm_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
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
	CShareData::getInstance()->TransformFileName_MakeCache();
	int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
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
		TRUE,
		hwndProgress,
		ref
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
		// 2004.04.03 Moca MoveCursor内でファイルの最後に移動するようにした
//		if( nPosY >= m_cLayoutMgr.GetLineCount() ){
			/*ファイルの最後に移動 */
//			m_cEditViewArr[i].Command_GOFILEEND(FALSE);
//			m_cEditViewArr[i].HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
//		}else{
			m_cEditViewArr[i].MoveCursor( nPosX, nPosY, TRUE );
			m_cEditViewArr[i].m_nCaretPosX_Prev = m_cEditViewArr[i].m_nCaretPosX;
//		}
	}
	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}




/* 編集ファイル情報を格納 */
void CEditDoc::SetFileInfo( FileInfo* pfi )
{
	int		nX;
	int		nY;

	strcpy( pfi->m_szPath, GetFilePath() );
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

	//デバッグモニタ(アウトプットウインドウ)
	pfi->m_bIsDebug = m_bDebugMode;

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
	HWND		hwndMainFrame;
	hwndMainFrame = ::GetParent( m_hWnd );

	//	Mar. 30, 2003 genta サブルーチンにまとめた
	AddToMRU();

	if( m_bGrepRunning ){		/* Grep処理中 */
		/* アクティブにする */
		ActivateFrameWindow( hwndMainFrame );	//@@@ 2003.06.25 MIK
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
				IsFilePathAvailable() ? GetFilePath() : "（無題）"
			);
			switch( nRet ){
			case IDYES:
//				if( IsFilePathAvailable() ){
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
				IsFilePathAvailable() ? GetFilePath() : "（無題）"
			);
			switch( nRet ){
			case IDYES:
				if( IsFilePathAvailable() ){
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
	m_eWatchUpdate = WU_QUERY; // Dec. 4, 2002 genta 更新監視方法

//	Sep. 10, 2002 genta
//	アイコン設定はファイル名設定と一体化のためここからは削除

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
	//	Sep. 10, 2002 genta
	//	アイコンも同時に初期化される
	SetFilePath( "" );

	/* 現在編集中のファイルのタイムスタンプ */
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;


	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
	int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	SetDocumentType( doctype, true );

	/* レイアウト管理情報の初期化 */
	/* レイアウト情報の変更 */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		NULL,/*hwndProgress*/
		ref
	);


	/* 変更フラグ */
	SetModified(false,false);	//	Jan. 22, 2002 genta

	/* 文字コード種別 */
	m_nCharCode = 0;
	m_bBomExist = FALSE;	//	Jul. 26, 2003 ryoji

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
		m_cEditViewArr[i].m_nCaretPosX_Prev = 0;
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
	 // Dec. 4, 2002 genta
	 && m_eWatchUpdate != WU_NONE
	 && m_pShareData->m_Common.m_nFileShareMode == 0	/* ファイルの排他制御モード */
	 && NULL != ( hwndActive = ::GetActiveWindow() )	/* アクティブ? */
	 && hwndActive == m_hwndParent
	 && IsFilePathAvailable()
	 && ( m_FileTime.dwLowDateTime != 0 || m_FileTime.dwHighDateTime != 0 ) 	/* 現在編集中のファイルのタイムスタンプ */

	){
		do {
			/* ファイルスタンプをチェックする */
//			MYTRACE( "ファイルスタンプをチェックする\n" );

			FILETIME	FileTimeNow;
			HFILE		hFile;
			BOOL		bWork;
			LONG		lWork;

			hFile = _lopen( GetFilePath(), OF_READ );
			if( HFILE_ERROR == hFile ){
				break;
			}
			bWork = ::GetFileTime( (HANDLE)hFile, NULL, NULL, &FileTimeNow );
			_lclose( hFile );
			if( 0 == bWork ){
				break;
			}
			lWork = ::CompareFileTime( &m_FileTime, &FileTimeNow );
			//	Aug. 13, 2003 wmlhq タイムスタンプが古く変更されている場合も検出対象とする
			if( 0 != lWork ){
				bUpdate = TRUE;
//				MYTRACE( "★更新されています★★★★★★★★★★★\n" );
				m_FileTime = FileTimeNow;
			}
		} while(0);
	}

	//	From Here Dec. 4, 2002 genta
	if( bUpdate ){
		switch( m_eWatchUpdate ){
		case WU_NOTIFY:
			{
				char szText[40];
				//	現在時刻の取得
				SYSTEMTIME st;
				FILETIME lft;
				if( ::FileTimeToLocalFileTime( &m_FileTime, &lft ) &&
					::FileTimeToSystemTime( &lft, &st )){
					// nothing to do
				}
				else {
					//	ファイル時刻の変換に失敗した場合は
					//	現在時刻でごまかす
					::GetLocalTime( &st );
				}
				wsprintf( szText, "★ファイル更新 %02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond );
				m_pcEditWnd->SendStatusMessage( szText );
			}	
			break;
		default:
			{
				m_eWatchUpdate = WU_NONE; // 更新監視の抑制

				CDlgFileUpdateQuery dlg( GetFilePath(), IsModified() );
				int result = dlg.DoModal( m_hInstance, m_hWnd, IDD_FILEUPDATEQUERY, 0 );

				switch( result ){
				case 1:	// 再読込
					/* 同一ファイルの再オープン */
					ReloadCurrentFile( m_nCharCode, m_bReadOnly );
					m_eWatchUpdate = WU_QUERY;
					break;
				case 2:	// 以後通知メッセージのみ
					m_eWatchUpdate = WU_NOTIFY;
					break;
				case 3:	// 以後更新を監視しない
					m_eWatchUpdate = WU_NONE;
					break;
				case 0:	// CLOSE
				default:
					m_eWatchUpdate = WU_QUERY;
					break;
				}
			}
			break;
		}
	}
	//	To Here Dec. 4, 2002 genta
	return;
}





/*! 同一ファイルの再オープン */
void CEditDoc::ReloadCurrentFile(
	BOOL	nCharCode,		/*!< [in] 文字コード種別 */
	BOOL	bReadOnly		/*!< [in] 読み取り専用モード */
)
{
	if( -1 == _access( GetFilePath(), 0 ) ){
		/* ファイルが存在しない */
		//	Jul. 26, 2003 ryoji BOMを標準設定に
		m_nCharCode = nCharCode;
		switch( m_nCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_bBomExist = TRUE;
			break;
		case CODE_UTF8:
		default:
			m_bBomExist = FALSE;
			break;
		}
		return;
	}


	BOOL	bOpened;
	char	szFilePath[MAX_PATH];
	int		nCaretPosX;
	int		nCaretPosY;
	nCaretPosX = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;
	nCaretPosY = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY;

	strcpy( szFilePath, GetFilePath() );

	// Mar. 30, 2003 genta ブックマーク保存のためMRUへ登録
	AddToMRU();

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
	m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX_Prev = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;

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

	特殊文字は以下の通り
	@li $  $自身
	@li F  開いているファイルのフルパス。名前がなければ(無題)。
	@li f  開いているファイルの名前（ファイル名+拡張子のみ）
	@li g  開いているファイルの名前（拡張子除く）
	@li /  開いているファイルの名前（フルパス。パスの区切りが/）
	@li N  開いているファイルの名前(簡易表示)
	@li C  現在選択中のテキスト
	@li x  現在の物理桁位置(先頭からのバイト数1開始)
	@li y  現在の物理行位置(1開始)
	@li d  現在の日付(共通設定の日付書式)
	@li t  現在の時刻(共通設定の時刻書式)
	@li p  現在のページ
	@li P  総ページ
	@li D  ファイルのタイムスタンプ(共通設定の日付書式)
	@li T  ファイルのタイムスタンプ(共通設定の時刻書式)
	@li V  エディタのバージョン文字列
	@li h  Grep検索キーの先頭32byte

	@date 2003.04.03 genta strncpy_ex導入によるfor文の削減
*/
void CEditDoc::ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen)
{
	
	// Apr. 03, 2003 genta 固定文字列をまとめる
	static const char PRINT_PREVIEW_ONLY[] = "(印刷プレビューでのみ使用できます)";
	const int PRINT_PREVIEW_ONLY_LEN = sizeof( PRINT_PREVIEW_ONLY ) - 1;
	static const char NO_TITLE[] = "(無題)";
	const int NO_TITLE_LEN = sizeof( NO_TITLE ) - 1;
	static const char NOT_SAVED[] = "(保存されていません)";
	const int NOT_SAVED_LEN = sizeof( NOT_SAVED ) - 1;

	const char *p, *r;	//	p：目的のバッファ。r：作業用のポインタ。
	char *q, *q_max;
	for( p = pszSource, q = pszBuffer, q_max = pszBuffer + nBufferLen; *p != '\0' && q < q_max;){
		if( *p != '$' ){
			*q++ = *p++;
			continue;
		}
		switch( *(++p) ){
		case '$':	//	 $$ -> $
			*q++ = *p++;
			break;
		case 'F':	//	開いているファイルの名前（フルパス）
			if ( !IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				r = GetFilePath();
				q = strncpy_ex( q, q_max - q, r, strlen( r ));
				++p;
			}
			break;
		case 'f':	//	開いているファイルの名前（ファイル名+拡張子のみ）
			// Oct. 28, 2001 genta
			//	ファイル名のみを渡すバージョン
			//	ポインタを末尾に
			if ( ! IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				r = GetFileName(); // 2002.10.13 Moca ファイル名(パスなし)を取得。日本語対応
				//	万一\\が末尾にあってもその後ろには\0があるのでアクセス違反にはならない。
				q = strncpy_ex( q, q_max - q, r, strlen( r ));
				++p;
			}
			break;
		case 'g':	//	開いているファイルの名前（拡張子を除くファイル名のみ）
			//	From Here Sep. 16, 2002 genta
			if ( ! IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	ポインタを末尾に
				const char *dot_position, *end_of_path;
				r = GetFileName(); // 2002.10.13 Moca ファイル名(パスなし)を取得。日本語対応
				end_of_path = dot_position =
					r + strlen( r );
				//	後ろから.を探す
				for( --dot_position ; dot_position > r && *dot_position != '.'
					; --dot_position )
					;
				//	rと同じ場所まで行ってしまった⇔.が無かった
				if( dot_position == r )
					dot_position = end_of_path;

				q = strncpy_ex( q, q_max - q, r, dot_position - r );
				++p;
			}
			break;
			//	To Here Sep. 16, 2002 genta
		case '/':	//	開いているファイルの名前（フルパス。パスの区切りが/）
			// Oct. 28, 2001 genta
			if ( !IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	パスの区切りとして'/'を使うバージョン
				for( r = GetFilePath(); *r != '\0' && q < q_max; ++r, ++q ){
					if( *r == '\\' )
						*q = '/';
					else
						*q = *r;
				}
				++p;
			}
			break;
		//	From Here 2003/06/21 Moca
		case 'N':
			if( !IsFilePathAvailable() ){
				q = strncpy_ex( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			}
			else {
				char szText[1024];
				CShareData::getInstance()->GetTransformFileNameFast( GetFilePath(), szText, 1023 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		//	To Here 2003/06/21 Moca
		//	From Here Jan. 15, 2002 hor
		case 'C':	//	現在選択中のテキスト
			{
				CMemory cmemCurText;
				m_cEditViewArr[m_nActivePaneIndex].GetCurrentTextForSearch( cmemCurText );
				q = strncpy_ex( q, q_max - q, cmemCurText.GetPtr(), cmemCurText.GetLength());
				++p;
			}
		//	To Here Jan. 15, 2002 hor
			break;
		//	From Here 2002/12/04 Moca
		case 'x':	//	現在の物理桁位置(先頭からのバイト数1開始)
			{
				char szText[11];
				_itot( m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX_PHY + 1, szText, 10 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		case 'y':	//	現在の物理行位置(1開始)
			{
				char szText[11];
				_itot( m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY_PHY + 1, szText, 10 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		//	To Here 2002/12/04 Moca
		case 'd':	//	共通設定の日付書式
			{
				char szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetDateFormat( systime, szText, sizeof( szText ) - 1 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		case 't':	//	共通設定の時刻書式
			{
				char szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetTimeFormat( systime, szText, sizeof( szText ) - 1 );
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			break;
		case 'p':	//	現在のページ
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetCurPageNum() + 1, szText, 10);
					q = strncpy_ex( q, q_max - q, szText, strlen(szText));
					++p;
				}
				else {
					q = strncpy_ex( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case 'P':	//	総ページ
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetAllPageNum(), szText, 10);
					q = strncpy_ex( q, q_max - q, szText, strlen(szText));
					++p;
				}
				else {
					q = strncpy_ex( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
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
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			else {
				q = strncpy_ex( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
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
				q = strncpy_ex( q, q_max - q, szText, strlen(szText));
				++p;
			}
			else {
				q = strncpy_ex( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case 'V':	// Apr. 4, 2003 genta
			// Version number
			{
				char buf[28]; // 6(符号含むWORDの最大長) * 4 + 4(固定部分)
				//	2004.05.13 Moca バージョン番号は、プロセスごとに取得する
				DWORD dwVersionMS, dwVersionLS;
				GetAppVersionInfo( NULL, VS_VERSION_INFO,
					&dwVersionMS, &dwVersionLS );
				int len = sprintf( buf, "%d.%d.%d.%d",
					HIWORD( dwVersionMS ),
					LOWORD( dwVersionMS ),
					HIWORD( dwVersionLS ),
					LOWORD( dwVersionLS )
				);
				q = strncpy_ex( q, q_max - q, buf, len );
				++p;
			}
			break;
		case 'h':	//	Apr. 4, 2003 genta
			//	Grep Key文字列 MAX 32文字
			//	中身はSetParentCaption()より移植
			{
				CMemory		cmemDes;
				LimitStringLengthB( m_szGrepKey, lstrlen( m_szGrepKey ),
					(q_max - q > 32 ? 32 : q_max - q - 3), cmemDes );
				if( (int)lstrlen( m_szGrepKey ) > cmemDes.GetLength() ){
					cmemDes.Append( "...", 3 );
				}
				q = strncpy_ex( q, q_max - q, cmemDes.GetPtr(), cmemDes.GetLength());
				++p;
			}
			break;
		//	Mar. 31, 2003 genta
		//	条件分岐
		//	${cond:string1$:string2$:string3$}
		//	
		case '{':	// 条件分岐
			{
				int cond;
				cond = ExParam_Evaluate( p + 1 );
				while( *p != '?' && *p != '\0' )
					++p;
				if( *p == '\0' )
					break;
				p = ExParam_SkipCond( p + 1, cond );
			}
			break;
		case ':':	// 条件分岐の中間
			//	条件分岐の末尾までSKIP
			p = ExParam_SkipCond( p + 1, -1 );
			break;
		case '}':	// 条件分岐の末尾
			//	特にすることはない
			++p;
			break;
		default:
			*q++ = '$';
			*q++ = *p++;
			break;
		}
	}
	*q = '\0';
}

/*! @brief 処理の読み飛ばし

	条件分岐の構文 ${cond:A0$:A1$:A2$:..$} において，
	指定した番号に対応する位置の先頭へのポインタを返す．
	指定番号に対応する物が無ければ$}の次のポインタを返す．

	${が登場した場合にはネストと考えて$}まで読み飛ばす．

	@param pszSource [in] スキャンを開始する文字列の先頭．cond:の次のアドレスを渡す．
	@param part [in] 移動する番号＝読み飛ばす$:の数．-1を与えると最後まで読み飛ばす．

	@return 移動後のポインタ．該当領域の先頭かあるいは$}の直後．

	@author genta
	@date 2003.03.31 genta 作成
*/
const char* CEditDoc::ExParam_SkipCond(const char* pszSource, int part)
{
	if( part == 0 )
		return pszSource;
	
	int nest = 0;	// 入れ子のレベル
	bool next = true;	// 継続フラグ
	const char *p;
	for( p = pszSource; next && *p != '\0'; ++p ) {
		if( *p == '$' && p[1] != '\0' ){ // $が末尾なら無視
			switch( *(++p)){
			case '{':	// 入れ子の開始
				++nest;
				break;
			case '}':
				if( nest == 0 ){
					//	終了ポイントに達した
					next = false; 
				}
				else {
					//	ネストレベルを下げる
					--nest;
				}
				break;
			case ':':
				if( nest == 0 && --part == 0){ // 入れ子でない場合のみ
					//	目的のポイント
					next = false;
				}
				break;
			}
		}
	}
	return p;
}

/*!	@brief 条件の評価

	@param pCond [in] 条件種別先頭．'?'までを条件と見なして評価する
	@return 評価の値

	@note
	ポインタの読み飛ばし作業は行わないので，'?'までの読み飛ばしは
	呼び出し側で別途行う必要がある．

	@author genta
	@date 2003.03.31 genta 作成

*/
int CEditDoc::ExParam_Evaluate( const char* pCond )
{
	switch( *pCond ){
	case 'R': // 読みとり専用
		if( m_bReadOnly ){	/* 読み取り専用モード */
			return 0;
		}else
		if( 0 != m_nFileShareModeOld && /* ファイルの排他制御モード */
			NULL == m_hLockedFile		/* ロックしていない */
		){
			return 1;
		}else{
			return 2;
		}
	case 'w': // Grepモード/Output Mode
		if( m_bGrepMode ){
			return 0;
		}else if( m_bDebugMode ){
			return 1;
		}else {
			return 2;
		}
	case 'M': // キーボードマクロの記録中
		if( TRUE == m_pShareData->m_bRecordingKeyMacro &&
		m_pShareData->m_hwndRecordingKeyMacro == m_hwndParent ){ /* ウィンドウ */
			return 0;
		}else {
			return 1;
		}
	case 'U': // 更新
		if( IsModified()){
			return 0;
		}
		else {
			return 1;
		}
	case 'I': // アイコン化されているか
		if( ::IsIconic( m_hwndParent )){
			return 0;
		} else {
 			return 1;
 		}
	default:
		return 0;
	}
	return 0;
}

/*[EOF]*/
