/*!	@file
	@brief 文書関連情報の管理

	@author Norio Nakatani
	@date	1998/03/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, YAZAKI, jepro, novice, asa-o, MIK,
	Copyright (C) 2002, YAZAKI, hor, genta, aroka, frozen, Moca, MIK
	Copyright (C) 2003, MIK, genta, ryoji, Moca, zenryaku, naoh, wmlhq
	Copyright (C) 2004, genta, novice, Moca, MIK, zenryaku
	Copyright (C) 2005, genta, naoh, FILE, Moca, ryoji, D.S.Koba, aroka
	Copyright (C) 2006, genta, ryoji, aroka
	Copyright (C) 2007, ryoji, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>	// Apr. 03, 2003 genta
#include <io.h>
#include <Cderr.h> // Nov. 3, 2005 genta
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
#include "global.h"
#include "CFuncInfoArr.h" /// 2002/2/3 aroka
#include "CSMacroMgr.h"///
#include "CMarkMgr.h"///
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CPrintPreview.h"
#include "CDlgFileUpdateQuery.h"
#include <assert.h> /// 2002/11/2 frozen
#include "my_icmp.h" // 2002/11/30 Moca 追加
#include "my_sp.h" // 2005/11/22 aroka 追加
#include "CClipboard.h"
#include "CLayout.h"	// 2007.08.22 ryoji 追加
#include "CMemoryIterator.h"	// 2007.08.22 ryoji 追加
#include "charset/CCodeMediator.h"
#include "io/io_util.h"
#include "util/file.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "util/format.h"
#include "util/module.h"
#include "CEditApp.h"
#include "util/other_util.h"

#define IDT_ROLLMOUSE	1

/*!
	May 12, 2000 genta 初期化方法変更
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@note m_pcEditWnd はコンストラクタ内では使用しないこと．
*/
CEditDoc::CEditDoc(CEditApp* pcApp)
: m_pcEditWnd(pcApp->m_pcEditWnd)
, m_cNewLineCode( EOL_CRLF )		//	New Line Type
, m_cSaveLineCode( EOL_NONE )		//	保存時のLine Type
, m_bGrepRunning( FALSE )		/* Grep処理中 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
, m_nCommandExecNum( 0 )			/* コマンド実行回数 */
// 2004/06/21 novice タグジャンプ機能追加
, m_bReadOnly( false )			/* 読み取り専用モード */
, m_bDebugMode( FALSE )			/* デバッグモニタモード */
, m_bGrepMode( FALSE )			/* Grepモードか */
, m_nCharCode( CODE_DEFAULT )	/* 文字コード種別 */
, m_bBomExist( FALSE )			//	Jul. 26, 2003 ryoji BOM
, m_bDoing_UndoRedo( FALSE )		/* アンドゥ・リドゥの実行中か */
, m_nFileShareModeOld( SHAREMODE_NOT_EXCLUSIVE )		/* ファイルの排他制御モード */
, m_hLockedFile( NULL )			/* ロックしているファイルのハンドル */
, m_pszAppName( _T("EditorClient") )
, m_hInstance( NULL )
, m_eWatchUpdate( CEditDoc::WU_QUERY )
, m_nSettingTypeLocked( false )	//	設定値変更可能フラグ
, m_nSettingType( 0 )	// Sep. 11, 2002 genta
, m_bInsMode( true )	// Oct. 2, 2005 genta
, m_bIsModified( false )	/* 変更フラグ */ // Jan. 22, 2002 genta 型変更
{
	MY_RUNNINGTIMER( cRunningTimer, L"CEditDoc::CEditDoc" );
//	m_pcDlgTest = new CDlgTest;

	m_szFilePath[0] = '\0';			/* 現在編集中のファイルのパス */
	wcscpy( m_szGrepKey, L"" );
	/* 共有データ構造体のアドレスを返す */

	m_pShareData = CShareData::getInstance()->GetShareData();
	//	Sep. 11, 2002 genta 削除
	//	SetDocumentTypeはコンストラクタ中では使わない．
	//int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	//SetDocumentType( doctype, true );


	/* レイアウト管理情報の初期化 */
	m_cLayoutMgr.Create( this, &m_cDocLineMgr );
	/* レイアウト情報の変更 */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		NULL,/*hwndProgress*/
		ref
	);
//	MYTRACE_A( "CEditDoc::CEditDoc()おわり\n" );

	//	Aug, 21, 2000 genta
	//	自動保存の設定
	ReloadAutoSaveParam();

	//	Sep, 29, 2001 genta
	//	マクロ
	m_pcSMacroMgr = new CSMacroMgr;
	//wcscpy(m_pszCaption, L"sakura");	//@@@	YAZAKI
	
	//	m_FileTimeの初期化
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;

	//	Oct. 2, 2005 genta 挿入モード
	SetInsMode( m_pShareData->m_Common.m_sGeneral.m_bIsINSMode != FALSE );

	return;
}


CEditDoc::~CEditDoc()
{
//	delete (CDialog*)m_pcDlgTest;
//	m_pcDlgTest = NULL;

	if( GetSplitterHwnd() != NULL ){
		DestroyWindow( GetSplitterHwnd() );
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
	CImageListMgr* pcIcons
 )
{
	MY_RUNNINGTIMER( cRunningTimer, L"CEditDoc::Create" );

	m_hInstance = hInstance;


	//	Oct. 2, 2001 genta
	m_cFuncLookup.Init( m_hInstance, m_pShareData->m_MacroTable, &m_pShareData->m_Common );


	MY_TRACETIME( cRunningTimer, L"End: PropSheet" );

	return TRUE;
}












BOOL CEditDoc::SelectFont( LOGFONT* plf )
{
	// 2004.02.16 Moca CHOOSEFONTをメンバから外す
	CHOOSEFONT cf;
	/* CHOOSEFONTの初期化 */
	::ZeroMemory( &cf, sizeof( cf ) );
	cf.lStructSize = sizeof( cf );
	cf.hwndOwner = GetSplitterHwnd();
	cf.hDC = NULL;
//	cf.lpLogFont = &(m_pShareData->m_Common.m_lf);
	cf.Flags = CF_FIXEDPITCHONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

	//FIXEDフォント以外
	#ifdef USE_UNFIXED_FONT
		cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	#endif

	cf.lpLogFont = plf;
	if( !ChooseFont( &cf ) ){
#ifdef _DEBUG
		DWORD nErr;
		nErr = CommDlgExtendedError();
		switch( nErr ){
		case CDERR_FINDRESFAILURE:	MYTRACE_A( "CDERR_FINDRESFAILURE \n" );	break;
		case CDERR_INITIALIZATION:	MYTRACE_A( "CDERR_INITIALIZATION \n" );	break;
		case CDERR_LOCKRESFAILURE:	MYTRACE_A( "CDERR_LOCKRESFAILURE \n" );	break;
		case CDERR_LOADRESFAILURE:	MYTRACE_A( "CDERR_LOADRESFAILURE \n" );	break;
		case CDERR_LOADSTRFAILURE:	MYTRACE_A( "CDERR_LOADSTRFAILURE \n" );	break;
		case CDERR_MEMALLOCFAILURE:	MYTRACE_A( "CDERR_MEMALLOCFAILURE\n" );	break;
		case CDERR_MEMLOCKFAILURE:	MYTRACE_A( "CDERR_MEMLOCKFAILURE \n" );	break;
		case CDERR_NOHINSTANCE:		MYTRACE_A( "CDERR_NOHINSTANCE \n" );		break;
		case CDERR_NOHOOK:			MYTRACE_A( "CDERR_NOHOOK \n" );			break;
		case CDERR_NOTEMPLATE:		MYTRACE_A( "CDERR_NOTEMPLATE \n" );		break;
		case CDERR_STRUCTSIZE:		MYTRACE_A( "CDERR_STRUCTSIZE \n" );		break;
		case CFERR_MAXLESSTHANMIN:	MYTRACE_A( "CFERR_MAXLESSTHANMIN \n" );	break;
		case CFERR_NOFONTS:			MYTRACE_A( "CFERR_NOFONTS \n" );			break;
		}
#endif
		return FALSE;
	}

	return TRUE;
}




/*! ファイルを開く

	@return 成功: TRUE/pbOpened==FALSE,
			既に開かれている: FALSE/pbOpened==TRUE
			失敗: FALSE/pbOpened==FALSE

	@note genta 近いうちに見直した方がいいな．

	@date 2000.01.18 システム属性のファイルが開けない問題
	@date 2000.05,12 genta 改行コードの設定
	@date 2000.10.25 genta 文字コードの異常な値をチェック
	@date 2000.11.20 genta IME状態の設定
	@date 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	@date 2002.01.16 hor ブックマーク復元
	@date 2002.10.19 genta 読み取り不可のファイルは文字コード判別で失敗する
	@date 2003.03.28 MIK 改行の真ん中にカーソルが来ないように
	@date 2003.07.26 ryoji BOM引数追加
	@date 2002.05.26 Moca gm_pszCodeNameArr_Normal を使うように変更
	@date 2004.06.18 moca ファイルが開けなかった場合にpbOpenedがFALSEに初期化されていなかった．
	@date 2004.10.09 genta 存在しないファイルを開こうとしたときに
					フラグに応じて警告を出す（以前の動作）ように
	@date 2006.12.16 じゅうじ 前回の文字コードを優先する
	@date 2007.03.12 maru ファイルが存在しなくても前回の文字コードを継承
						多重オープン処理をCEditDoc::IsPathOpenedに移動
*/
bool CEditDoc::FileRead(
	TCHAR*		pszPath,			//!< [in/out]
	bool*		pbOpened,			//!< [out] すでに開かれていたか
	ECodeType	nCharCode,			/*!< [in] 文字コード種別 */
	bool		bReadOnly,			/*!< [in] 読み取り専用か */
	bool		bConfirmCodeChange	/*!< [in] 文字コード変更時の確認をするかどうか */
)
{
	HWND			hWndOwner;
	bool			bRet;
	FileInfo		fi;
	HWND			hwndProgress;
	CWaitCursor		cWaitCursor( GetSplitterHwnd() );
	BOOL			bIsExistInMRU;
	int				nRet;
	BOOL			bFileIsExist;
	int				doctype;

	*pbOpened = FALSE;	// 2004.06.18 Moca 初期化ミス
	m_bReadOnly = bReadOnly;	/* 読み取り専用モード */

//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	CMRU			cMRU;

	/* ファイルの存在チェック */
	bFileIsExist = FALSE;
	if( -1 == _taccess( pszPath, 0 ) ){
	}else{
		HANDLE			hFind;
		WIN32_FIND_DATA	w32fd;
		hFind = ::FindFirstFile( pszPath, &w32fd );
		::FindClose( hFind );

		bFileIsExist = TRUE;

		/* フォルダが指定された場合 */
		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			/* 指定フォルダで「開くダイアログ」を表示 */
			{
				TCHAR pszPathNew[_MAX_PATH]=_T("");

				/* 「ファイルを開く」ダイアログ */
				nCharCode = CODE_AUTODETECT;	/* 文字コード自動判別 */
				bReadOnly = FALSE;
//				::ShowWindow( GetSplitterHwnd(), SW_SHOW );
				if( !OpenFileDialog( GetSplitterHwnd(), pszPath, pszPathNew, &nCharCode, &bReadOnly ) ){
					return FALSE;
				}
				_tcscpy( pszPath, pszPathNew );

				if( -1 == _taccess( pszPath, 0 ) ){
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
				GetSplitterHwnd(), MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%ls\'\n")
				_T("というファイルを開けません。\n")
				_T("読み込みアクセス権がありません。"),
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
		hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
	}else{
		hwndProgress = NULL;
	}
	bRet = true;
	if( NULL == pszPath ){
		MYMESSAGEBOX_A(
			GetSplitterHwnd(),
			MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
			"バグじゃぁあああ！！！",
			"CEditDoc::FileRead()\n\nNULL == pszPath\n【対処】エラーの出た状況を作者に連絡してくださいね。"
		);
		return FALSE;
	}
	/* 指定ファイルが開かれているか調べる */
	if( CShareData::getInstance()->IsPathOpened(pszPath, &hWndOwner, nCharCode) ){	/* 2007.03.12 maru 多重オープン処理はIsPathOpenedにまとめる */
		*pbOpened = TRUE;
		bRet = false;
		goto end_of_func;
	}

	//ビューのテキスト選択解除
	m_pcEditWnd->Views_DisableSelectArea(true);

	//	Sep. 10, 2002 genta
	SetFilePath( pszPath ); /* 現在編集中のファイルのパス */


	/* 指定された文字コード種別に変更する */
	//	Oct. 25, 2000 genta
	//	文字コードとして異常な値が設定された場合の対応
	//	-1以上CODE_MAX未満のみ受け付ける
	//	Oct. 26, 2000 genta
	//	CODE_AUTODETECTはこの範囲から外れているから個別にチェック
	//  Aug. 14, 2007 kobake
	//  nCharCodeのチェックを専用の関数に委譲
	if( IsInECodeType(nCharCode) )
		m_nCharCode = nCharCode;
	
	/* MRUリストに存在するか調べる  存在するならばファイル情報を返す */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	if ( cMRU.GetFileInfo( pszPath, &fi ) ){
		bIsExistInMRU = TRUE;

		if( -1 == m_nCharCode ){
			/* 前回に指定された文字コード種別に変更する */
			m_nCharCode = fi.m_nCharCode;
		}
		
		if( !bConfirmCodeChange && ( CODE_AUTODETECT == m_nCharCode ) ){	// 文字コード指定の再オープンなら前回を無視
			m_nCharCode = fi.m_nCharCode;
		}
		if( (FALSE == bFileIsExist) && (CODE_AUTODETECT == m_nCharCode) ){
			/* 存在しないファイルの文字コード指定なしなら前回を継承 */
			m_nCharCode = fi.m_nCharCode;
		}
	} else {
		bIsExistInMRU = FALSE;
	}

	/* 文字コード自動判別 */
	if( CODE_AUTODETECT == m_nCharCode ) {
		if( FALSE == bFileIsExist ){	/* ファイルが存在しない */
			m_nCharCode = CODE_DEFAULT;
		} else {
			m_nCharCode = CCodeMediator::CheckKanjiCodeOfFile( pszPath );
			if( CODE_ERROR == m_nCharCode ){
				::MYMESSAGEBOX( GetSplitterHwnd(), MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
					_T("%ls\n")
					_T("文字コードの判別処理でエラーが発生しました。"),
					pszPath
				);
				//	Sep. 10, 2002 genta
				SetFilePath( _T("") );
				bRet = false;
				goto end_of_func;
			}
		}
	}
	/* 文字コードが異なるときに確認する */
	if( bConfirmCodeChange && bIsExistInMRU ){
		if (m_nCharCode != fi.m_nCharCode ) {	// MRU の文字コードと判別が異なる
			const TCHAR* pszCodeName = NULL;
			const TCHAR* pszCodeNameNew = NULL;

			// gm_pszCodeNameArr_Normal を使うように変更 Moca. 2002/05/26
			if(IsValidCodeType(fi.m_nCharCode)){
				pszCodeName = gm_pszCodeNameArr_Normal[fi.m_nCharCode];
			}
			if(IsValidCodeType(m_nCharCode)){
				pszCodeNameNew = gm_pszCodeNameArr_Normal[m_nCharCode];
			}
			if( pszCodeName != NULL ){
				::MessageBeep( MB_ICONQUESTION );
				nRet = MYMESSAGEBOX_A(
					GetSplitterHwnd(),
					MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
					"文字コード情報",
					"%ts\n\nこのファイルは、前回は別の文字コード %ts で開かれています。\n"
					"前回と同じ文字コードを使いますか？\n"
					"\n"
					"・[はい(Y)]  ＝%ts\n"
					"・[いいえ(N)]＝%ts\n"
					"・[キャンセル]＝開きません",
					GetFilePath(),
					pszCodeName,
					pszCodeName,
					pszCodeNameNew
				);
				if( IDYES == nRet ){
					/* 前回に指定された文字コード種別に変更する */
					m_nCharCode = fi.m_nCharCode;
				}
				else if( IDCANCEL == nRet ){
					m_nCharCode = CODE_DEFAULT;
					//	Sep. 10, 2002 genta
					SetFilePath( _T("") );
					bRet = false;
					goto end_of_func;
				}
			}else{
				MYMESSAGEBOX_A(
					GetSplitterHwnd(),
					MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
					"バグじゃぁあああ！！！",
					"【対処】エラーの出た状況を作者に連絡してください。"
				);
				//	Sep. 10, 2002 genta
				SetFilePath( _T("") );
				bRet = false;
				goto end_of_func;
			}
		}
	}
	if( -1 == m_nCharCode ){
		m_nCharCode = CODE_DEFAULT;
	}

	//	Nov. 12, 2000 genta ロングファイル名の取得を前方に移動
	TCHAR szWork[MAX_PATH];
	/* ロングファイル名を取得する */
	if( ::GetLongFileName( pszPath, szWork ) ){
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

		//	Oct. 09, 2004 genta フラグに応じて警告を出す（以前の動作）ように
		if( m_pShareData->m_Common.m_sFile.GetAlertIfFileNotExist() ){
			::MessageBeep( MB_ICONINFORMATION );

			//	Feb. 15, 2003 genta Popupウィンドウを表示しないように．
			//	ここでステータスメッセージを使っても画面に表示されない．
			::MYMESSAGEBOX(
				GetOwnerHwnd(),
				MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
				GSTR_APPNAME,
				_T("%ls\nというファイルは存在しません。\n\nファイルを保存したときに、ディスク上にこのファイルが作成されます。"),	//Mar. 24, 2001 jepro 若干修正
				pszPath
			);
		}
	}else{
		/* ファイルを読む */
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}
		//	Jul. 26, 2003 ryoji BOM引数追加
		if( FALSE == m_cDocLineMgr.ReadFile( GetFilePath(), GetSplitterHwnd(), hwndProgress,
			m_nCharCode, &m_FileTime, m_pShareData->m_Common.m_sFile.GetAutoMIMEdecode(), &m_bBomExist ) ){
			//	Sep. 10, 2002 genta
			SetFilePath( _T("") );
			bRet = false;
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

	if( bIsExistInMRU && m_pShareData->m_Common.m_sFile.GetRestoreCurPosition() ){
		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		CLayoutPoint ptCaretPos;
		m_cLayoutMgr.LogicToLayout(
			fi.m_ptCursor,
			&ptCaretPos
		);
		if( ptCaretPos.GetY2() >= m_cLayoutMgr.GetLineCount() ){
			/*ファイルの最後に移動 */
			m_pcEditWnd->GetActiveView().GetCommander().HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
		}else{
			m_pcEditWnd->GetActiveView().GetTextArea().SetViewTopLine( fi.m_nViewTopLine ); // 2001/10/20 novice
			m_pcEditWnd->GetActiveView().GetTextArea().SetViewLeftCol( fi.m_nViewLeftCol ); // 2001/10/20 novice
			// From Here Mar. 28, 2003 MIK
			// 改行の真ん中にカーソルが来ないように。
			const CDocLine *pTmpDocLine = m_cDocLineMgr.GetLineInfo( fi.m_ptCursor.GetY2() );	// 2008.08.22 ryoji 改行単位の行番号を渡すように修正
			if( pTmpDocLine ){
				if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_ptCursor.x ) ptCaretPos.x--;
			}
			// To Here Mar. 28, 2003 MIK
			m_pcEditWnd->GetActiveView().GetCaret().MoveCursor( ptCaretPos, TRUE );
			m_pcEditWnd->GetActiveView().GetCaret().m_nCaretPosX_Prev =
				m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX2();
		}
	}
	// 2002.01.16 hor ブックマーク復元
	if( bIsExistInMRU ){
		if( m_pShareData->m_Common.m_sFile.GetRestoreBookmarks() ){
			m_cDocLineMgr.SetBookMarks(fi.m_szMarkLines);
		}
	}else{
		wcscpy(fi.m_szMarkLines,L"");
	}
	GetFileInfo( &fi );

	//	May 12, 2000 genta
	//	改行コードの設定
	{
		SetNewLineCode( EOL_CRLF );
		CDocLine*	pFirstlineinfo = m_cDocLineMgr.GetLineInfo( CLogicInt(0) );
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
		TCHAR	szCurDir[_MAX_PATH];
		TCHAR	szDrive[_MAX_DRIVE];
		TCHAR	szDir[_MAX_DIR];
		_tsplitpath( GetFilePath(), szDrive, szDir, NULL, NULL );
		_tcscpy( szCurDir, szDrive);
		_tcscat( szCurDir, szDir );
		::SetCurrentDirectory( szCurDir );
	}

end_of_func:;
	//	2004.05.13 Moca 改行コードの設定内からここに移動
	m_pcEditWnd->GetActiveView().GetCaret().DrawCaretPosInfo();

	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	if( bRet && IsFilePathAvailable() ){
		/* ファイルの排他ロック */
		DoFileLock();
	}
	//	From Here Jul. 26, 2003 ryoji エラーの時は規定のBOM設定とする
	if( !bRet ){
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
BOOL CEditDoc::FileWrite( const TCHAR* pszPath, enumEOLType cEolType )
{
	BOOL		bRet = TRUE;

	//	Sep. 7, 2003 genta
	//	保存が完了するまではファイル更新の通知を抑制する
	WatchUpdate wuSave = m_eWatchUpdate;
	m_eWatchUpdate = WU_NONE;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta

	//プログレスバー
	HWND		hwndProgress;
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
	}else{
		hwndProgress = NULL;
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	// ファイルの排他ロック解除
	DoFileUnLock();

	if( m_pShareData->m_Common.m_sBackup.m_bBackUp ){	/* バックアップの作成 */
		//	Jun.  5, 2004 genta ファイル名を与えるように．戻り値に応じた処理を追加．
		switch( MakeBackUp( pszPath )){
		case 2:	//	中断指示
			return FALSE;
		case 3: //	ファイルエラー
			if( IDYES != ::MYMESSAGEBOX_A(
				GetSplitterHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				"ファイル保存",
				"バックアップの作成に失敗しました．元ファイルへの上書きを継続して行いますか．"
			)){
				return FALSE;
			}
		break;
		}
	}

	//砂時計
	CWaitCursor cWaitCursor( GetSplitterHwnd() );

	//カキコ
	EConvertResult nWriteResult = m_cDocLineMgr.WriteFile(
		pszPath,
		GetSplitterHwnd(),
		hwndProgress,
		m_nCharCode,
		&m_FileTime,
		CEOL(cEolType),
		m_bBomExist
	);

	//カキコ結果
	if( nWriteResult == RESULT_FAILURE ){
		bRet = FALSE;
		goto end_of_func;
	}
	else if(nWriteResult == RESULT_LOSESOME){
		ErrorMessage(NULL, _T("一部のテキストデータが、変換により失われました"));
	}

	/* 行変更状態をすべてリセット */
	m_cDocLineMgr.ResetAllModifyFlag();
	
	//ビュー再描画
	m_pcEditWnd->Views_RedrawAll();

	//	Sep. 10, 2002 genta
	SetFilePath( pszPath ); /* 現在編集中のファイルのパス */

	SetModified(false,false);	//	Jan. 22, 2002 genta 関数化 更新フラグのクリア

	//	Mar. 30, 2003 genta サブルーチンにまとめた
	AddToMRU();

	/* 現在位置で無変更な状態になったことを通知 */
	m_cOpeBuf.SetNoModified();

	m_bReadOnly = false;	/* 読み取り専用モード */

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();
end_of_func:;

	if( IsFilePathAvailable() &&
		!m_bReadOnly && /* 読み取り専用モード ではない */
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
	HWND				hwndParent,
	const TCHAR*		pszOpenFolder,	//<! [in]  NULL以外を指定すると初期フォルダを指定できる
	TCHAR*				pszPath,		//<! [out] 開くファイルのパスを受け取るアドレス
	ECodeType*			pnCharCode,		//<! [out] 指定された文字コード種別を受け取るアドレス
	bool*				pbReadOnly		//<! [out] 読み取り専用か
)
{
	/* アクティブにする */
	ActivateFrameWindow( hwndParent );

	TCHAR**	ppszMRU;
	TCHAR**	ppszOPENFOLDER;
	BOOL	bRet;

	/* MRUリストのファイルのリスト */
//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new TCHAR*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);


	/* OPENFOLDERリストのファイルのリスト */
//@@@ 2001.12.26 YAZAKI OPENFOLDERリストは、CMRUFolderにすべて依頼する
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new TCHAR*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* 初期フォルダの設定 */
	// pszFolderはフォルダ名だが、ファイル名付きパスを渡してもCDlgOpenFile側で処理してくれる
	SFilePath pszCurDir;
	const TCHAR* pszDefFolder;
	if( pszOpenFolder!=NULL ){
		pszDefFolder = pszOpenFolder;
	}else{
		if( IsFilePathAvailable() ){
			pszDefFolder = GetFilePath();
		// Mar. 28, 2003 genta カレントディレクトリをMRUより優先させる
		//}else if( ppszMRU[0] != NULL && ppszMRU[0][0] != '\0' ){ // Sep. 9, 2002 genta
		//	pszDefFolder = ppszMRU[0];
		}else{ // 2002.10.25 Moca
			int nCurDir;
			nCurDir = ::GetCurrentDirectory( _MAX_PATH, pszCurDir );
			if( 0 == nCurDir || _MAX_PATH < nCurDir ){
				pszDefFolder = _T("");
			}else{
				pszDefFolder = pszCurDir;
			}
		}
	}
	/* ファイルオープンダイアログの初期化 */
	m_pcEditWnd->m_cDlgOpenFile.Create2(
		m_hInstance,
		hwndParent,
		_T("*.*"),
		pszDefFolder,
		ppszMRU,
		ppszOPENFOLDER
	);
	
	bRet = m_pcEditWnd->m_cDlgOpenFile.DoModalOpenDlg( pszPath, pnCharCode, pbReadOnly );

	delete [] ppszMRU;
	delete [] ppszOPENFOLDER;
//	delete [] pszCurDir;
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
	@date 2006.11.10 ryoji	ユーザー指定の拡張子を状況依存で変化させる
*/
BOOL CEditDoc::SaveFileDialog( TCHAR* pszPath, ECodeType* pnCharCode, CEOL* pcEol, BOOL* pbBomExist )
{
	TCHAR**	ppszMRU;		//	最近のファイル
	TCHAR**	ppszOPENFOLDER;	//	最近のフォルダ
	TCHAR*	pszCurDir = NULL;
	TCHAR	szDefaultWildCard[_MAX_PATH + 10];	// ユーザー指定拡張子
	BOOL	bret;

	/* MRUリストのファイルのリスト */
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new TCHAR*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);

	/* OPENFOLDERリストのファイルのリスト */
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new TCHAR*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* ファイル保存ダイアログの初期化 */
	/* ファイル名の無いファイルだったら、ppszMRU[0]をデフォルトファイル名として？ppszOPENFOLDERじゃない？ */
	// ファイル名の無いときはカレントフォルダをデフォルトにします。Mar. 30, 2003 genta
	// 掲示板要望 No.2699 (2003/02/05)
	const TCHAR*	pszDefFolder; // デフォルトフォルダ
	if( !IsFilePathAvailable() ){
		// 2002.10.25 Moca さんのコードを流用 Mar. 23, 2003 genta
		int nCurDir;
		pszCurDir = new TCHAR[_MAX_PATH];
		nCurDir = ::GetCurrentDirectory( _MAX_PATH, pszCurDir );
		if( 0 == nCurDir || _MAX_PATH < nCurDir ){
			pszDefFolder = _T("");
		}else{
			pszDefFolder = pszCurDir;
		}
		_tcscpy(szDefaultWildCard, _T("*.txt"));
		if( m_pShareData->m_Common.m_sFile.m_bNoFilterSaveNew )
			_tcscat(szDefaultWildCard, _T(";*.*"));	// 全ファイル表示
	}else{
		pszDefFolder = GetFilePath();
		TCHAR	szExt[_MAX_EXT];
		_tsplitpath(GetFilePath(), NULL, NULL, NULL, szExt);
		if( szExt[0] == _T('.') && szExt[1] != _T('\0') ){
			_tcscpy(szDefaultWildCard, _T("*"));
			_tcscat(szDefaultWildCard, szExt);
			if( m_pShareData->m_Common.m_sFile.m_bNoFilterSaveFile )
				_tcscat(szDefaultWildCard, _T(";*.*"));	// 全ファイル表示
		}else{
			_tcscpy(szDefaultWildCard, _T("*.*"));
		}
	}
	m_pcEditWnd->m_cDlgOpenFile.Create2( m_hInstance, GetSplitterHwnd(), szDefaultWildCard, pszDefFolder,
		ppszMRU, ppszOPENFOLDER );

	/* ダイアログを表示 */
	//	Jul. 26, 2003 ryoji pbBomExist追加
	bret = m_pcEditWnd->m_cDlgOpenFile.DoModalSaveDlg( pszPath, pnCharCode, pcEol, pbBomExist );

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
	m_pcEditWnd->m_cPropCommon.InitData();
	
	/* プロパティシートの作成 */
	if( m_pcEditWnd->m_cPropCommon.DoPropertySheet( nPageNum/*, nActiveItem*/ ) ){

		// 2002.12.11 Moca この部分で行われていたデータのコピーをCPropCommonに移動・関数化
		// ShareData に 設定を適用・コピーする
		// 2007.06.20 ryoji グループ化に変更があったときはグループIDをリセットする
		BOOL bGroup = (m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin);
		m_pcEditWnd->m_cPropCommon.ApplyData();
		m_pcSMacroMgr->UnloadAll();	// 2007.10.19 genta マクロ登録変更を反映するため，読み込み済みのマクロを破棄する
		if( bGroup != (m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CShareData::getInstance()->ResetGroupId();
		}

		/* アクセラレータテーブルの再作成 */
		::SendMessageAny( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* フォントが変わった */
		for( i = 0; i < 4; ++i ){
			m_pcEditWnd->m_pcEditViewArr[i]->m_cTipWnd.ChangeFont( &(m_pShareData->m_Common.m_sHelper.m_lf_kh) );
		}

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)GetOwnerHwnd(),
			GetOwnerHwnd()
		);

		return TRUE;
	}else{
		return FALSE;
	}
}



/*! タイプ別設定 プロパティシート */
BOOL CEditDoc::OpenPropertySheetTypes( int nPageNum, int nSettingType )
{
	m_pcEditWnd->m_cPropTypes.SetTypeData( m_pShareData->m_Types[nSettingType] );
	// Mar. 31, 2003 genta メモリ削減のためポインタに変更しProperySheet内で取得するように
	//m_cPropTypes.m_CKeyWordSetMgr = m_pShareData->m_CKeyWordSetMgr;

	/* プロパティシートの作成 */
	if( m_pcEditWnd->m_cPropTypes.DoPropertySheet( nPageNum ) ){
		/* 変更された設定値のコピー */
		m_pcEditWnd->m_cPropTypes.GetTypeData( m_pShareData->m_Types[nSettingType] );

		/* アクセラレータテーブルの再作成 */
		::SendMessageAny( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* 設定変更を反映させる */
		/* 全編集ウィンドウへメッセージをポストする */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)GetOwnerHwnd(),
			GetOwnerHwnd()
		);

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
	return CClipboard::HasValidData()?TRUE:FALSE;
}





/*! 親ウィンドウのタイトルを更新

	@date 2007.03.08 ryoji bKillFocusパラメータを除去
*/
void CEditDoc::SetParentCaption( void )
{
	if( NULL == GetSplitterHwnd() ){
		return;
	}
	if( !m_pcEditWnd->GetActiveView().GetDrawSwitch() ){
		return;
	}

//	/* アイコン化されていない時はフルパス */
//	/* アイコン化されている時はファイル名のみ */
//	if( ::IsIconic( GetSplitterHwnd() ) ){
//		bKillFocus = TRUE;
//	}else{
//		bKillFocus = FALSE;
//	}

	wchar_t	pszCap[1024];	//	Nov. 6, 2000 genta オーバーヘッド軽減のためHeap→Stackに変更

	// From Here Apr. 04, 2003 genta / Apr.05 ShareDataのパラメータ利用に
	if( !m_pcEditWnd->IsActiveApp() ){	// 2007.03.08 ryoji bKillFocusをIsActiveApp()に変更
		ExpandParameter(
			to_wchar(m_pShareData->m_Common.m_sWindow.m_szWindowCaptionInactive),
			pszCap,
			_countof( pszCap )
		);
	}
	else {
		ExpandParameter(
			to_wchar(m_pShareData->m_Common.m_sWindow.m_szWindowCaptionActive),
			pszCap,
			_countof( pszCap )
		);
	}
	// To Here Apr. 04, 2003 genta

	TCHAR tszBuf[256];
	_wcstotcs(tszBuf, pszCap, _countof(tszBuf));
	::SetWindowText( GetOwnerHwnd(), tszBuf );

	//@@@ From Here 2003.06.13 MIK
	//タブウインドウのファイル名を通知
	ExpandParameter( m_pShareData->m_Common.m_sTabBar.m_szTabWndCaption, pszCap, _countof( pszCap ));
	m_pcEditWnd->ChangeFileNameNotify( to_tchar(pszCap), GetFilePath(), m_bGrepMode );	// 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加
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
	@date 2005.11.26 aroka ファイル名生成をFormatBackUpPathに分離

	@retval 0 バックアップ作成失敗．
	@retval 1 バックアップ作成成功．
	@retval 2 バックアップ作成失敗．保存中断指示．
	@retval 3 ファイル操作エラーによるバックアップ作成失敗．
	
	@todo Advanced modeでの世代管理
*/
int CEditDoc::MakeBackUp( const TCHAR* target_file )
{
	int		nRet;

	/* バックアップソースの存在チェック */
	//	Aug. 21, 2005 genta 書き込みアクセス権がない場合も
	//	ファイルがない場合と同様に何もしない
	if( (_taccess( target_file, 2 )) == -1 ){
		return 0;
	}

	if( m_pShareData->m_Common.m_sBackup.m_bBackUpFolder ){	/* 指定フォルダにバックアップを作成する */
		//	Aug. 21, 2005 genta 指定フォルダがない場合に警告
		if( (_taccess( m_pShareData->m_Common.m_sBackup.m_szBackUpFolder, 0 )) == -1 ){

			int nMsgResult = ::MYMESSAGEBOX_A(
				GetSplitterHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				"バックアップエラー",
				"以下のバックアップフォルダが見つかりません．\n%ts\n"
				"バックアップを作成せずに上書き保存してよろしいですか．",
				m_pShareData->m_Common.m_sBackup.m_szBackUpFolder
			);
			if( nMsgResult == IDYES ){
				return 0;//	保存継続
			}
			else {
				return 2;// 保存中断
			}
		}
	}

	TCHAR	szPath[_MAX_PATH];
	FormatBackUpPath( szPath, target_file );

	//@@@ 2002.03.23 start ネットワーク・リムーバブルドライブの場合はごみ箱に放り込まない
	bool dustflag = false;
	if( m_pShareData->m_Common.m_sBackup.m_bBackUpDustBox ){
		dustflag = !IsLocalDrive( szPath );
	}
	//@@@ 2002.03.23 end

	if( m_pShareData->m_Common.m_sBackup.m_bBackUpDialog ){	/* バックアップの作成前に確認 */
		::MessageBeep( MB_ICONQUESTION );
//From Here Feb. 27, 2001 JEPROtest キャンセルもできるようにし、メッセージを追加した
//		if( IDYES != MYMESSAGEBOX(
//			GetSplitterHwnd(),
//			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
//			"バックアップ作成の確認",
//			"変更される前に、バックアップファイルを作成します。\nよろしいですか？\n\n%ls\n    ↓\n%ls\n\n",
//			IsFilePathAvailable() ? GetFilePath() : "（無題）",
//			szPath
//		) ){
//			return FALSE;
//		}
		if( m_pShareData->m_Common.m_sBackup.m_bBackUpDustBox && dustflag == false ){	//@@@ 2001.12.11 add start MIK	//2002.03.23
			nRet = ::MYMESSAGEBOX_A(
				GetSplitterHwnd(),
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				"バックアップ作成の確認",
				"変更される前に、バックアップファイルを作成します。\nよろしいですか？  [いいえ(N)] を選ぶと作成せずに上書き（または名前を付けて）保存になります。\n\n%ls\n    ↓\n%ls\n\n作成したバックアップファイルをごみ箱に放り込みます。\n",
				target_file,
				szPath
			);
		}else{	//@@@ 2001.12.11 add end MIK
			nRet = ::MYMESSAGEBOX_A(
				GetSplitterHwnd(),
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				"バックアップ作成の確認",
				"変更される前に、バックアップファイルを作成します。\nよろしいですか？  [いいえ(N)] を選ぶと作成せずに上書き（または名前を付けて）保存になります。\n\n%ls\n    ↓\n%ls\n\n",
				//IsFilePathAvailable() ? GetFilePath() : "（無題）",
				//	Aug. 21, 2005 genta 現在のファイルではなくターゲットファイルをバックアップするように
				target_file,
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
	if( m_pShareData->m_Common.m_sBackup.GetBackupType() == 3 ||
		m_pShareData->m_Common.m_sBackup.GetBackupType() == 6 ){
		//	既に存在するBackupをずらす処理
		int				i;

		//	ファイル検索用
		HANDLE			hFind;
		WIN32_FIND_DATA	fData;

		TCHAR*	pBase = szPath + _tcslen( szPath ) - 2;	//	2: 拡張子の最後の2桁の意味
		//::MessageBoxA( NULL, pBase, _T("書き換え場所"), MB_OK );

		//------------------------------------------------------------------
		//	1. 該当ディレクトリ中のbackupファイルを1つずつ探す
		for( i = 0; i <= 99; i++ ){	//	最大値に関わらず，99（2桁の最大値）まで探す
			//	ファイル名をセット
			auto_sprintf( pBase, _T("%02d"), i );

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
		int boundary = m_pShareData->m_Common.m_sBackup.GetBackupCount();
		boundary = boundary > 0 ? boundary - 1 : 0;	//	最小値は0
		//::MessageBoxA( NULL, pBase, _T("書き換え場所"), MB_OK );

		for( ; i >= boundary; --i ){
			//	ファイル名をセット
			auto_sprintf( pBase, _T("%02d"), i );
			if( ::DeleteFile( szPath ) == 0 ){
				::MessageBox( GetSplitterHwnd(), szPath, _T("削除失敗"), MB_OK );
				//	Jun.  5, 2005 genta 戻り値変更
				//	失敗しても保存は継続
				return 0;
				//	失敗した場合
				//	後で考える
			}
		}

		//	この位置でiは存在するバックアップファイルの最大番号を表している．

		//	3. そこから0番まではコピーしながら移動
		TCHAR szNewPath[MAX_PATH];
		TCHAR *pNewNrBase;

		_tcscpy( szNewPath, szPath );
		pNewNrBase = szNewPath + _tcslen( szNewPath ) - 2;

		for( ; i >= 0; --i ){
			//	ファイル名をセット
			auto_sprintf( pBase, _T("%02d"), i );
			auto_sprintf( pNewNrBase, _T("%02d"), i + 1 );

			//	ファイルの移動
			if( ::MoveFile( szPath, szNewPath ) == 0 ){
				//	失敗した場合
				//	後で考える
				::MessageBox( GetSplitterHwnd(), szPath, _T("移動失敗"), MB_OK );
				//	Jun.  5, 2005 genta 戻り値変更
				//	失敗しても保存は継続
				return 0;
			}
		}
	}
	//	To Here Aug. 16, 2000 genta

	//::MessageBoxA( NULL, szPath, _T("直前のバックアップファイル"), MB_OK );
	/* バックアップの作成 */
	//	Aug. 21, 2005 genta 現在のファイルではなくターゲットファイルをバックアップするように
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath( szPath, szDrive, szDir, szFname, szExt );
	TCHAR	szPath2[MAX_PATH];
	auto_sprintf( szPath2, _T("%ts%ts"), szDrive, szDir );

	HANDLE			hFind;
	WIN32_FIND_DATA	fData;

	hFind = ::FindFirstFile( szPath2, &fData );
	if( hFind == INVALID_HANDLE_VALUE ){
		//	検索に失敗した == ファイルは存在しない
		::CreateDirectory( szPath2, NULL );
	}
	::FindClose( hFind );

	if( ::CopyFile( target_file, szPath, FALSE ) ){
		/* 正常終了 */
		//@@@ 2001.12.11 start MIK
		if( m_pShareData->m_Common.m_sBackup.m_bBackUpDustBox && dustflag == false ){	//@@@ 2002.03.23 ネットワーク・リムーバブルドライブでない
			TCHAR	szDustPath[_MAX_PATH+1];
			_tcscpy(szDustPath, szPath);
			szDustPath[_tcslen(szDustPath) + 1] = _T('\0');
			SHFILEOPSTRUCT	fos;
			fos.hwnd   = GetSplitterHwnd();
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDustPath;
			fos.pTo    = NULL;
			fos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;	//ダイアログなし
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

/*! バックアップの作成

	@param[out] szNewPath バックアップ先パス名
	@param[in]  target_file バックアップ元パス名

	@author aroka
	@date 2005.11.29 aroka
		MakeBackUpから分離．書式を元にバックアップファイル名を作成する機能追加

	@retval true
	
	@todo Advanced modeでの世代管理
*/
bool CEditDoc::FormatBackUpPath( TCHAR* szNewPath, const TCHAR* target_file )
{
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];

	/* パスの分解 */
	_tsplitpath( target_file, szDrive, szDir, szFname, szExt );

	if( m_pShareData->m_Common.m_sBackup.m_bBackUpFolder ){	/* 指定フォルダにバックアップを作成する */
		_tcscpy( szNewPath, m_pShareData->m_Common.m_sBackup.m_szBackUpFolder );
		/* フォルダの最後が半角かつ'\\'でない場合は、付加する */
		AddLastYenFromDirectoryPath( szNewPath );
	}
	else{
		auto_sprintf( szNewPath, _T("%ts%ts"), szDrive, szDir );
	}

	/* 相対フォルダを挿入 */
	if( !m_pShareData->m_Common.m_sBackup.m_bBackUpPathAdvanced ){
		time_t	ltime;
		struct	tm *today, *gmt;
		wchar_t	szTime[64];
		wchar_t	szForm[64];

		TCHAR*	pBase;
		pBase = szNewPath + _tcslen( szNewPath );

		/* バックアップファイル名のタイプ 1=(.bak) 2=*_日付.* */
		switch( m_pShareData->m_Common.m_sBackup.GetBackupType() ){
		case 1:
			auto_sprintf( pBase, _T("%ls.bak"), szFname );
			break;
		case 5: //	Jun.  5, 2005 genta 1の拡張子を残す版
			auto_sprintf( pBase, _T("%ts%ts.bak"), szFname, szExt );
			break;
		case 2:	//	日付，時刻
			_tzset();
			_wstrdate( szTime );
			time( &ltime );				/* システム時刻を得ます */
			gmt = gmtime( &ltime );		/* 万国標準時に変換する */
			today = localtime( &ltime );/* 現地時間に変換する */

			wcscpy( szForm, L"" );
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_YEAR) ){	/* バックアップファイル名：日付の年 */
				wcscat( szForm, L"%Y" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_MONTH) ){	/* バックアップファイル名：日付の月 */
				wcscat( szForm, L"%m" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_DAY) ){	/* バックアップファイル名：日付の日 */
				wcscat( szForm, L"%d" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_HOUR) ){	/* バックアップファイル名：日付の時 */
				wcscat( szForm, L"%H" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_MIN) ){	/* バックアップファイル名：日付の分 */
				wcscat( szForm, L"%M" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_SEC) ){	/* バックアップファイル名：日付の秒 */
				wcscat( szForm, L"%S" );
			}
			/* YYYYMMDD時分秒 形式に変換 */
			wcsftime( szTime, _countof( szTime ) - 1, szForm, today );
			auto_sprintf( pBase, _T("%ts_%ls%ls"), szFname, szTime, szExt );
			break;
	//	2001/06/12 Start by asa-o: ファイルに付ける日付を前回の保存時(更新日時)にする
		case 4:	//	日付，時刻
			{
				FILETIME	LastWriteTime,
							LocalTime;
				SYSTEMTIME	SystemTime;

				// 2005.10.20 ryoji FindFirstFileを使うように変更
				if( ! GetLastWriteTimestamp( target_file, LastWriteTime )){
					LastWriteTime.dwHighDateTime = LastWriteTime.dwLowDateTime = 0;
				}
				::FileTimeToLocalFileTime(&LastWriteTime,&LocalTime);	// 現地時刻に変換
				::FileTimeToSystemTime(&LocalTime,&SystemTime);			// システムタイムに変換

				wcscpy( szTime, L"" );
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_YEAR) ){	/* バックアップファイル名：日付の年 */
					auto_sprintf(szTime,L"%d",SystemTime.wYear);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_MONTH) ){	/* バックアップファイル名：日付の月 */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wMonth);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_DAY) ){	/* バックアップファイル名：日付の日 */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wDay);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_HOUR) ){	/* バックアップファイル名：日付の時 */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wHour);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_MIN) ){	/* バックアップファイル名：日付の分 */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wMinute);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_SEC) ){	/* バックアップファイル名：日付の秒 */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wSecond);
				}
				auto_sprintf( pBase, _T("%ts_%ls%ts"), szFname, szTime, szExt );
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
				TCHAR* ptr;
				if( m_pShareData->m_Common.m_sBackup.GetBackupType() == 3 ){
					ptr = szExt;
				}
				else {
					ptr = szExt + _tcslen( szExt );
				}
				*ptr   = _T('.');
				*++ptr = m_pShareData->m_Common.m_sBackup.GetBackupExtChar();
				*++ptr = _T('0');
				*++ptr = _T('0');
				*++ptr = _T('\0');
			}
			auto_sprintf( pBase, _T("%ts%ts"), szFname, szExt );
			break;
		}

	}else{ // 詳細設定使用する
		TCHAR szFormat[1024];

		switch( m_pShareData->m_Common.m_sBackup.GetBackupTypeAdv() ){
		case 4:	//	ファイルの日付，時刻
			{
				FILETIME	LastWriteTime,
							LocalTime;
				SYSTEMTIME	SystemTime;

				// 2005.10.20 ryoji FindFirstFileを使うように変更
				if( ! GetLastWriteTimestamp( target_file, LastWriteTime )){
					LastWriteTime.dwHighDateTime = LastWriteTime.dwLowDateTime = 0;
				}
				::FileTimeToLocalFileTime(&LastWriteTime,&LocalTime);	// 現地時刻に変換
				::FileTimeToSystemTime(&LocalTime,&SystemTime);			// システムタイムに変換

				GetDateTimeFormat( szFormat, _countof(szFormat), m_pShareData->m_Common.m_sBackup.m_szBackUpPathAdvanced , SystemTime );
			}
			break;
		case 2:	//	現在の日付，時刻
		default:
			{
				time_t	ltime;
				struct	tm *today;

				time( &ltime );				/* システム時刻を得ます */
				today = localtime( &ltime );/* 現地時間に変換する */

				/* YYYYMMDD時分秒 形式に変換 */
				_tcsftime( szFormat, _countof( szFormat ) - 1, m_pShareData->m_Common.m_sBackup.m_szBackUpPathAdvanced , today );
			}
			break;
		}

		{
			// make keys
			// $0-$9に対応するフォルダ名を切り出し
			TCHAR keybuff[1024];
			_tcscpy( keybuff, szDir );
			CutLastYenFromDirectoryPath( keybuff );

			TCHAR *folders[10];
			{
				//	Jan. 9, 2006 genta VC6対策
				int idx;
				for( idx=0; idx<10; ++idx ){
					folders[idx] = 0;
				}
				folders[0] = szFname;

				for( idx=1; idx<10; ++idx ){
					TCHAR *cp;
					cp = auto_strchr(keybuff, _T('\\'));
					if( cp != NULL ){
						folders[idx] = cp+1;
						*cp = _T('\0');
					}
					else{
						break;
					}
				}
			}
			{
				// $0-$9を置換
				//wcscpy( szNewPath, L"" );
				TCHAR *q= szFormat;
				TCHAR *q2 = szFormat;
				while( *q ){
					if( *q==_T('$') ){
						++q;
						if( isdigit(*q) ){
							q[-1] = _T('\0');
							_tcscat( szNewPath, q2 );
							if( folders[*q-_T('0')] != 0 ){
								_tcscat( szNewPath, folders[*q-_T('0')] );
							}
							q2 = q+1;
						}
					}
					++q;
				}
				_tcscat( szNewPath, q2 );
			}
		}
		{
			TCHAR temp[1024];
			TCHAR *cp;
			//	2006.03.25 Aroka szExt[0] == '\0'のときのオーバラン問題を修正
			TCHAR *ep = (szExt[0]!=0) ? &szExt[1] : &szExt[0];

			while( _tcschr( szNewPath, _T('*') ) ){
				_tcscpy( temp, szNewPath );
				cp = _tcschr( temp, _T('*') );
				*cp = 0;
				auto_sprintf( szNewPath, _T("%ts%ts%ts"), temp, ep, cp+1 );
			}
			//	??はバックアップ連番にしたいところではあるが，
			//	連番処理は末尾の2桁にしか対応していないので
			//	使用できない文字?を_に変換してお茶を濁す
			while(( cp = _tcschr( szNewPath, _T('?') ) ) != NULL){
				*cp = _T('_');
			}
		}
	}
	return true;
}

/* ファイルの排他ロック */
void CEditDoc::DoFileLock( void )
{
	BOOL	bCheckOnly;

	/* ロックしている */
	if( NULL != m_hLockedFile ){
		/* ロック解除 */
		::CloseHandle( m_hLockedFile );
		m_hLockedFile = NULL;
	}

	/* ファイルが存在しない */
	if( -1 == _taccess( GetFilePath(), 0 ) ){
		/* ファイルの排他制御モード */
		m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;
		return;
	}else{
		/* ファイルの排他制御モード */
		m_nFileShareModeOld = m_pShareData->m_Common.m_sFile.m_nFileShareMode;
	}


	/* ファイルを開いていない */
	if( ! IsFilePathAvailable() ){
		return;
	}
	/* 読み取り専用モード */
	if( m_bReadOnly ){
		return;
	}


	if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_WRITE ||
		m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_READWRITE ){
		bCheckOnly = FALSE;
	}
	else{
		/* 排他制御しないけどロックされているかのチェックは行うのでreturnしない */
//		return;
		bCheckOnly = TRUE;
	}
	/* 書込み禁止かどうか調べる */
	if( -1 == _taccess( GetFilePath(), 2 ) ){	/* アクセス権：書き込み許可 */
		m_hLockedFile = NULL;
		/* 親ウィンドウのタイトルを更新 */
		SetParentCaption();
		return;
	}

	//書き込めるか検査
	if( !IsFileWritable(GetFilePath()) ){
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			GetSplitterHwnd(),
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ts\nは現在他のプロセスによって書込みが禁止されています。"),
			IsFilePathAvailable() ? GetFilePath() : _T("（無題）")
		);
		m_hLockedFile = NULL;
		/* 親ウィンドウのタイトルを更新 */
		SetParentCaption();
		return;
	}

	//↓↑※kobake注: この間の微小な時間差に注意。もっと良い処理があるはず。

	//オープン
	m_hLockedFile = OpenFile_InShareMode( GetFilePath(), m_pShareData->m_Common.m_sFile.m_nFileShareMode );
	if( INVALID_HANDLE_VALUE == m_hLockedFile ){
		const TCHAR*	pszMode;
		switch( m_pShareData->m_Common.m_sFile.m_nFileShareMode ){
		case SHAREMODE_DENY_READWRITE:	/* 読み書き */
			pszMode = _T("読み書き禁止モード");
			break;
		case SHAREMODE_DENY_WRITE:	/* 書き */
			pszMode = _T("書き込み禁止モード");
			break;
		default:
			pszMode = _T("未定義のモード（問題があります）");
			break;
		}
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			GetSplitterHwnd(),
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ls\nを%lsでロックできませんでした。\n現在このファイルに対する排他制御は無効となります。"),
			IsFilePathAvailable() ? GetFilePath() : _T("（無題）"),
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
		::CloseHandle( m_hLockedFile );
		m_hLockedFile = NULL;

		/* ファイルの排他制御モード */
		m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;
	}
	return;
}

/*
	C関数リスト作成はCEditDoc_FuncList1.cppへ移動
*/

/*! PL/SQL関数リスト作成 */
void CEditDoc::MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	wchar_t		szFuncName[80];
	CLogicInt	nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	nFuncNum = 0;
	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
//			if( 1 < nCharChars ){
//				i += (nCharChars - 1);
//				continue;
//			}
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( L'\'' == pLine[i] ){
					if( i + 1 < nLineLen && L'\'' == pLine[i + 1] ){
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
				if( i + 1 < nLineLen && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
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
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
				else{
					if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"FUNCTION" ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PROCEDURE" ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PACKAGE" ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == wcsicmp( szWord, L"BODY" ) ){
						nFuncOrProc = 4;
						nParseCnt = 1;
					}
					else if( 1 == nParseCnt ){
						if( 1 == nFuncOrProc ||
							2 == nFuncOrProc ||
							3 == nFuncOrProc ||
							4 == nFuncOrProc ){
							++nParseCnt;
							wcscpy( szFuncName, szWord );
						}else
						if( 3 == nFuncOrProc ){

						}
					}else
					if( 2 == nParseCnt ){
						if( 0 == wcsicmp( szWord, L"IS" ) ){
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
							CLayoutPoint ptPos;
							m_cLayoutMgr.LogicToLayout(
								CLogicPoint(0, nFuncLine - 1),
								&ptPos
							);
							pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1), szFuncName, nFuncId );
							nParseCnt = 0;
						}
						if( 0 == wcsicmp( szWord, L"AS" ) ){
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* パッケージ仕様部 */
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
								*/
								CLayoutPoint ptPos;
								m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
							else if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* パッケージ本体 */
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
								*/
								CLayoutPoint ptPos;
								m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
						}
					}
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
				if( L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'\t' == pLine[i] ||
					 L' ' == pLine[i] ||
					 WCODE::CR == pLine[i] ||
					 WCODE::LF == pLine[i] ||
					 L'{' == pLine[i] ||
					 L'}' == pLine[i] ||
					 L'(' == pLine[i] ||
					 L')' == pLine[i] ||
					 L';' == pLine[i] ||
					L'\'' == pLine[i] ||
					 L'/' == pLine[i] ||
					 L'-' == pLine[i]
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
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
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'-' == pLine[i] &&  L'-' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L';' == pLine[i] ){
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
						CLayoutPoint ptPos;
						m_cLayoutMgr.LogicToLayout(
							CLogicPoint(0, nFuncLine - 1),
							&ptPos
						);
						pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
						nParseCnt = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( (1 == nCharChars && (
						L'_' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
						) )
					 || 2 == nCharChars
					){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
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
	@date 2002.11.03 Moca	階層が最大値を超えるとバッファオーバーランするのを修正
							最大値以上は追加せずに無視する
	@date 2007.8頃   kobake 機械的にUNICODE化
	@date 2007.11.29 kobake UNICODE対応できてなかったので修正
*/
void CEditDoc::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	using namespace WCODE;

	//見出し記号
	const wchar_t*	pszStarts = m_pShareData->m_Common.m_sFormat.m_szMidashiKigou;
	int				nStartsLen = wcslen( pszStarts );

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	一時領域
	CLogicInt				nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//行取得
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine )break;

		//行頭の空白飛ばし
		int i;
		for( i = 0; i < nLineLen; ++i ){
			if( WCODE::isBlank(pLine[i]) ){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//先頭文字が見出し記号のいずれかであれば、次へ進む
		if(NULL==wcschr(pszStarts,pLine[0]))continue;

		//見出し種類の判別 -> szTitle
		if( pLine[i] == L'(' ){
			if(0){}
			else if ( IsInRange(pLine[i + 1], L'0', L'9') ) wcscpy( szTitle, L"(0)" ); //数字
			else if ( IsInRange(pLine[i + 1], L'A', L'Z') ) wcscpy( szTitle, L"(A)" ); //英大文字
			else if ( IsInRange(pLine[i + 1], L'a', L'z') ) wcscpy( szTitle, L"(a)" ); //英小文字
			else continue; //※「(」の次が英数字で無い場合、見出しとみなさない
		}
		else if( IsInRange(pLine[i], L'０', L'９') ) wcscpy( szTitle, L"０" ); // 全角数字
		else if( IsInRange(pLine[i], L'①', L'⑳') ) wcscpy( szTitle, L"①" ); // ①～⑳
		else if( IsInRange(pLine[i], L'Ⅰ', L'Ⅹ') ) wcscpy( szTitle, L"Ⅰ" ); // Ⅰ～Ⅹ
		else if( wcschr(L"〇一二三四五六七八九十百零壱弐参伍", pLine[i]) ) wcscpy( szTitle, L"一" ); //漢数字
		else{
			szTitle[0]=pLine[i];
			szTitle[1]=L'\0';
		}

		/*	「見出し記号」に含まれる文字で始まるか、
			(0、(1、...(9、(A、(B、...(Z、(a、(b、...(z
			で始まる行は、アウトライン結果に表示する。
		*/

		//行文字列から改行を取り除く pLine -> pszText
		wchar_t*	pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		for( i = 0; i < (int)wcslen(pszText); ++i ){
			if( pszText[i] == CR || pszText[i] == LF )pszText[i] = L'\0';
		}

		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		CLayoutPoint ptPos;
		m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepthを計算 */
		int k;
		bool bAppend = true;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
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
			wcscpy(pszStack[nDepth], szTitle);
		}
		else{
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーラン
			// nDepth = nMaxStack;
			bAppend = false;
		}
		
		if( bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}


/*! ルールファイルの1行を管理する構造体

	@date 2002.04.01 YAZAKI
	@date 2007.11.29 kobake 名前変更: oneRule→SOneRule
*/
struct SOneRule {
	wchar_t szMatch[256];
	int		nLength;
	wchar_t szGroupName[256];
};

/*! ルールファイルを読み込み、ルール構造体の配列を作成する

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca 引数nMaxCountを追加。バッファ長チェックをするように変更
*/
int CEditDoc::ReadRuleFile( const TCHAR* pszFilename, SOneRule* pcOneRule, int nMaxCount )
{
	long	i;
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	FILE*	pFile = _tfopen_absini( pszFilename, _T("r") );
	if( NULL == pFile ){
		return 0;
	}
	wchar_t	szLine[LINEREADBUFSIZE];
	const wchar_t*	pszDelimit = L" /// ";
	const wchar_t*	pszKeySeps = L",\0";
	wchar_t*	pszWork;
	int nDelimitLen = wcslen( pszDelimit );
	int nCount = 0;
	while( NULL != fgetws( szLine, _countof(szLine), pFile ) && nCount < nMaxCount ){
		pszWork = wcsstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != L';' ){
			*pszWork = L'\0';
			pszWork += nDelimitLen;

			/* 最初のトークンを取得します。 */
			wchar_t* pszToken = wcstok( szLine, pszKeySeps );
			while( NULL != pszToken ){
//				nRes = wcsicmp( pszKey, pszToken );
				for( i = 0; i < (int)wcslen(pszWork); ++i ){
					if( pszWork[i] == L'\r' ||
						pszWork[i] == L'\n' ){
						pszWork[i] = L'\0';
						break;
					}
				}
				wcsncpy( pcOneRule[nCount].szMatch, pszToken, 255 );
				wcsncpy( pcOneRule[nCount].szGroupName, pszWork, 255 );
				pcOneRule[nCount].szMatch[255] = L'\0';
				pcOneRule[nCount].szGroupName[255] = L'\0';
				pcOneRule[nCount].nLength = wcslen(pcOneRule[nCount].szMatch);
				nCount++;
				pszToken = wcstok( NULL, pszKeySeps );
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
	@date 2007.11.29 kobake SOneRule test[1024] でスタックが溢れていたのを修正
*/
void CEditDoc::MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr )
{
	wchar_t*		pszText;

	/* ルールファイルの内容をバッファに読み込む */
	auto_array_ptr<SOneRule> test = new SOneRule[1024];	// 1024個許可。 2007.11.29 kobake スタック使いすぎなので、ヒープに確保するように修正。
	int nCount = ReadRuleFile(GetDocumentAttribute().m_szOutlineRuleFilename, test.get(), 1024 );
	if ( nCount < 1 ){
		return;
	}

	/*	ネストの深さは、32レベルまで、ひとつのヘッダは、最長256文字まで区別
		（256文字まで同じだったら同じものとして扱います）
	*/
	const int	nMaxStack = 32;	//	ネストの最深
	int			nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t		pszStack[nMaxStack][256];
	wchar_t		szTitle[256];			//	一時領域
	for( CLogicInt nLineCount = CLogicInt(0); nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//行取得
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}

		//行頭の空白飛ばし
		int		i;
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == L' ' || pLine[i] == L'\t' || pLine[i] == L'　'){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//先頭文字が見出し記号のいずれかであれば、次へ進む
		int		j;
		for( j = 0; j < nCount; j++ ){
			if ( 0 == wcsncmp( &pLine[i], test[j].szMatch, test[j].nLength ) ){
				wcscpy( szTitle, test[j].szGroupName );
				break;
			}
		}
		if( j >= nCount ){
			continue;
		}

		/*	ルールにマッチした行は、アウトライン結果に表示する。
		*/

		//行文字列から改行を取り除く pLine -> pszText
		pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		int nTextLen = wcslen( pszText );
		for( i = 0; i < nTextLen; ++i ){
			if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
				pszText[i] = L'\0';
				break;
			}
		}

		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		CLayoutPoint ptPos;
		m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepthを計算 */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
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
			wcscpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーランするから規制する
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}



/*! COBOL アウトライン解析 */
void CEditDoc::MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				k;
	wchar_t			szDivision[1024];
	wchar_t			szLabel[1024];
	const wchar_t*	pszKeyWord;
	int				nKeyWordLen;
	BOOL			bDivision;

	szDivision[0] = L'\0';
	szLabel[0] =  L'\0';


	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		/* コメント行か */
		if( 7 <= nLineLen && pLine[6] == L'*' ){
			continue;
		}
		/* ラベル行か */
		if( 8 <= nLineLen && pLine[7] != L' ' ){
			k = 0;
			for( i = 7; i < nLineLen; ){
				if( pLine[i] == '.'
				 || pLine[i] == WCODE::CR
				 || pLine[i] == WCODE::LF
				){
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if( pLine[i - 1] == L' ' ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != L' ' ){
							break;
						}
					}
				}
			}
			szLabel[k] = L'\0';
//			MYTRACE_A( "szLabel=[%ls]\n", szLabel );



			pszKeyWord = L"division";
			nKeyWordLen = wcslen( pszKeyWord );
			bDivision = FALSE;
			for( i = 0; i <= (int)wcslen( szLabel ) - nKeyWordLen; ++i ){
				if( 0 == auto_memicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					szLabel[i + nKeyWordLen] = L'\0';
					wcscpy( szDivision, szLabel );
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

			CLayoutPoint ptPos;
			wchar_t	szWork[1024];
			m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			auto_sprintf( szWork, L"%ls::%ls", szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , szWork, 0 );
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
	CLogicInt nTotalLine;

	nTotalLine = m_cDocLineMgr.GetLineCount();

	for( CLogicInt nLineCount = CLogicInt(0); nLineCount < nTotalLine; nLineCount++ ){
		const WCHAR* pLine;
		CLogicInt nLineLen;
		WCHAR* pTmpLine;
		int length;
		int offset;
#define MAX_ASM_TOKEN 2
		WCHAR* token[MAX_ASM_TOKEN];
		int j;
		WCHAR* p;

		//1行取得する。
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( pLine == NULL ) break;

		//作業用にコピーを作成する。バイナリがあったらその後ろは知らない。
		pTmpLine = wcsdup( pLine );
		if( pTmpLine == NULL ) break;
		if( wcslen( pTmpLine ) >= (unsigned int)nLineLen ){	//バイナリを含んでいたら短くなるので...
			pTmpLine[ nLineLen ] = L'\0';	//指定長で切り詰め
		}

		//行コメント削除
		p = wcsstr( pTmpLine, L";" );
		if( p ) *p = L'\0';

		length = wcslen( pTmpLine );
		offset = 0;

		//トークンに分割
		for( j = 0; j < MAX_ASM_TOKEN; j++ ) token[ j ] = NULL;
		for( j = 0; j < MAX_ASM_TOKEN; j++ ){
			token[ j ] = my_strtok<WCHAR>( pTmpLine, length, &offset, L" \t\r\n" );
			if( token[ j ] == NULL ) break;
			//トークンに含まれるべき文字でないか？
			if( wcsstr( token[ j ], L"\"") != NULL
			 || wcsstr( token[ j ], L"\\") != NULL
			 || wcsstr( token[ j ], L"'" ) != NULL ){
				token[ j ] = NULL;
				break;
			}
		}

		if( token[ 0 ] != NULL ){	//トークンが1個以上ある
			int nFuncId = -1;
			WCHAR* entry_token = NULL;

			length = wcslen( token[ 0 ] );
			if( length >= 2
			 && token[ 0 ][ length - 1 ] == L':' ){	//ラベル
				token[ 0 ][ length - 1 ] = L'\0';
				nFuncId = 51;
				entry_token = token[ 0 ];
			}
			else if( token[ 1 ] != NULL ){	//トークンが2個以上ある
				if( wcsicmp( token[ 1 ], L"proc" ) == 0 ){	//関数
					nFuncId = 50;
					entry_token = token[ 0 ];
				}else
				if( wcsicmp( token[ 1 ], L"endp" ) == 0 ){	//関数終了
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
				CLayoutPoint ptPos;
				m_cLayoutMgr.LogicToLayout(
					CLogicPoint(0, nLineCount),
					&ptPos
				);
				pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1), entry_token, nFuncId );
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

	for(CLogicInt nLineCount=CLogicInt(0);nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const wchar_t*	pLine;
		CLogicInt		nLineLen;

		pLine = m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta 判定順序変更
		if( *pLine == L'.' )
		{
			const wchar_t* pPos;	//	May 25, 2003 genta
			int			nLength;
			wchar_t		szTitle[1024];

			//	ピリオドの数＝階層の深さを数える
			for( pPos = pLine + 1 ; *pPos == L'.' ; ++pPos )
				;

			CLayoutPoint ptPos;
			m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			
			int level = pPos - pLine;

			// 2003.06.27 Moca 階層が2段位上深くなるときは、無題の要素を追加
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (無題)を挿入
				//	ただし，TAG一覧には出力されないように
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData(
						nLineCount+CLogicInt(1),
						ptPos.GetY2()+CLayoutInt(1),
						L"(無題)",
						FUNCINFO_NOCLIPTEXT,
						dummyLevel - 1
					);
				}
			}
			levelPrev = level;

			nLength = auto_sprintf(szTitle,L"%d - ", level );
			
			wchar_t *pDest = szTitle + nLength; // 書き込み先
			wchar_t *pDestEnd = szTitle + _countof(szTitle) - 2;
			
			while( pDest < pDestEnd )
			{
				if( *pPos ==L'\r' || *pPos ==L'\n' || *pPos == L'\0')
				{
					break;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = L'\0';
			pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle, 0, level - 1);
		}
	}
}

/*! HTML アウトライン解析

	@author zenryaku
	@date 2003.05.20 zenryaku 新規作成
	@date 2004.04.19 zenryaku 空要素を判定
	@date 2004.04.20 Moca コメント処理と、不明な終了タグを無視する処理を追加
*/
void CEditDoc::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;
	BOOL			bEndTag;
	BOOL			bCommentTag = FALSE;

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	一時領域
	CLogicInt			nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine)
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			// 2004.04.20 Moca コメントを処理する
			if( bCommentTag )
			{
				if( i < nLineLen - 3 && 0 == wmemcmp( L"-->", pLine + i , 3 ) )
				{
					bCommentTag = FALSE;
					i += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if(pLine[i]!=L'<' || nDepth>=nMaxStack)
			{
				continue;
			}
			bEndTag	=	FALSE;
			if(pLine[++i]==L'/')
			{
				i++;
				bEndTag	=	TRUE;
			}
			for(j=0;i+j<nLineLen && j<_countof(szTitle)-1;j++)
			{
				if((pLine[i+j]<L'a' || pLine[i+j]>L'z') &&
					(pLine[i+j]<L'A' || pLine[i+j]>L'Z') &&
					!(j!=0 && pLine[i+j]>=L'0' && pLine[i+j]<=L'9'))
				{
					break;
				}
				szTitle[j]	=	pLine[i+j];
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here コメントを処理する
				if( i < nLineLen - 3 && 0 == wmemcmp( L"!--", pLine + i, 3 ) )
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
					if(!wcsicmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca ツリー中と一致しないときは、この終了タグは無視
				if( nDepth == 0 )
				{
					if(wcsicmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}
			}
			else
			{
				if(wcsicmp(szTitle,L"br") && wcsicmp(szTitle,L"area") &&
					wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
				{
					CLayoutPoint ptPos;

					m_cLayoutMgr.LogicToLayout(
						CLogicPoint(i, nLineCount),
						&ptPos
					);

					if(wcsicmp(szTitle,L"hr") && wcsicmp(szTitle,L"meta") && wcsicmp(szTitle,L"link") &&
						wcsicmp(szTitle,L"input") && wcsicmp(szTitle,L"img") && wcsicmp(szTitle,L"area") &&
						wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
					{
						// 終了タグなしを除く全てのタグらしきものを判定
						wcscpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<_countof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if(pLine[i+j]==L'/' && pLine[i+j+1]==L'>')
								{
									bEndTag	=	TRUE;
									break;
								}
								else if(pLine[i+j]==L'>')
								{
									break;
								}
							}
							if(!bEndTag)
							{
								szTitle[k++]	=	L' ';
								for(j-=k-1;i+j+k<nLineLen && k<_countof(szTitle)-1;k++)
								{
									if(pLine[i+j+k]==L'<' || pLine[i+j+k]==L'\r' || pLine[i+j+k]==L'\n')
									{
										break;
									}
									szTitle[k]	=	pLine[i+j+k];
								}
							j += k-1;
							}
						}
						szTitle[k]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,(bEndTag ? nDepth : nDepth++));
					}
					else
					{
						for(;i+j<nLineLen && j<_countof(szTitle)-1;j++)
						{
							if(pLine[i+j]=='>')
							{
								break;
							}
							szTitle[j]	=	pLine[i+j];
						}
						szTitle[j]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,nDepth);
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
	@date 2005.01.03 naoh 「マ」などの"}"を含む文字に対する修正、prosperのslideに対応
*/
void CEditDoc::MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;

	const int nMaxStack = 8;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t szTag[32], szTitle[256];			//	一時領域
	int thisSection=0, lastSection = 0;	// 現在のセクション種類と一つ前のセクション種類
	int stackSection[nMaxStack];		// 各深さでのセクションの番号
	int nStartTitlePos;					// \section{dddd} の dddd の部分の始まる番号
	int bNoNumber;						// * 付の場合はセクション番号を付けない

	// 一行ずつ
	CLogicInt	nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine) break;
		// 一文字ずつ
		for(i=0;i<nLineLen-1;i++)
		{
			if(pLine[i] == L'%') break;	// コメントなら以降はいらない
			if(nDepth>=nMaxStack)continue;
			if(pLine[i] != L'\\')continue;	// 「\」がないなら次の文字へ
			++i;
			// 見つかった「\」以降の文字列チェック
			for(j=0;i+j<nLineLen && j<_countof(szTag)-1;j++)
			{
				if(pLine[i+j] == L'{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1チェック
					bNoNumber = (pLine[i+j-1] == '*');
					nStartTitlePos = j+i+1;
					break;
				}
				szTag[j] = pLine[i+j];
			}
			if(j==0) continue;
			if(bNoNumber){
				szTag[j-1] = L'\0';
			}else{
				szTag[j]   = L'\0';
			}
//			MessageBoxA(NULL, szTitle, L"", MB_OK);

			thisSection = 0;
			if(!wcscmp(szTag,L"subsubsection")) thisSection = 4;
			else if(!wcscmp(szTag,L"subsection")) thisSection = 3;
			else if(!wcscmp(szTag,L"section")) thisSection = 2;
			else if(!wcscmp(szTag,L"chapter")) thisSection = 1;
			else if(!wcscmp(szTag,L"begin")) {		// beginなら prosperのslideの可能性も考慮
				// さらに{slide}{}まで読みとっておく
				if(wcsstr(pLine, L"{slide}")){
					k=0;
					for(j=nStartTitlePos+1;i+j<nLineLen && j<_countof(szTag)-1;j++)
					{
						if(pLine[i+j] == '{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1チェック
							nStartTitlePos = j+i+1;
							break;
						}
						szTag[k++]	=	pLine[i+j];
					}
					szTag[k] = '\0';
					thisSection = 1;
				}
			}

			if( thisSection > 0)
			{
				// sectionの中身取得
				for(k=0;nStartTitlePos+k<nLineLen && k<_countof(szTitle)-1;k++)
				{
					if(_IS_SJIS_1((unsigned char)pLine[k+nStartTitlePos])) {
						szTitle[k] = pLine[k+nStartTitlePos];
						k++;	// 次はチェック不要
					} else if(pLine[k+nStartTitlePos] == '}') {
						break;
					}
					szTitle[k] = pLine[k+nStartTitlePos];
				}
				szTitle[k] = '\0';

				CLayoutPoint ptPos;

				WCHAR tmpstr[256];
				WCHAR secstr[4];

				m_cLayoutMgr.LogicToLayout(
					CLogicPoint(i, nLineCount),
					&ptPos
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
				tmpstr[0] = L'\0';
				if(!bNoNumber){
					for(k=0; k<=nDepth; k++){
						auto_sprintf(secstr, L"%d.", stackSection[k]);
						wcscat(tmpstr, secstr);
					}
					wcscat(tmpstr, L" ");
				}
				wcscat(tmpstr, szTitle);
				pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1), tmpstr, 0, nDepth);
				if(!bNoNumber) lastSection = thisSection;
			}
			i	+=	j;
		}
	}
}








/*! コマンドコードによる処理振り分け

	@param[in] nCommand MAKELONG( コマンドコード，送信元識別子 )

	@date 2006.05.19 genta 上位16bitに送信元の識別子が入るように変更
	@date 2007.06.20 ryoji グループ内で巡回するように変更
*/
BOOL CEditDoc::HandleCommand( EFunctionCode nCommand )
{
	int				i;
	int				j;
	int				nGroup;
	int				nRowNum;
	int				nPane;
	HWND			hwndWork;
	EditNode*		pEditNodeArr;
	//	May. 19, 2006 genta 上位16bitに送信元の識別子が入るように変更したので
	//	下位16ビットのみを取り出す
	switch( LOWORD( nCommand )){
	case F_PREVWINDOW:	//前のウィンドウ
		nPane = m_pcEditWnd->m_cSplitterWnd.GetPrevPane();
		if( -1 != nPane ){
			m_pcEditWnd->SetActivePane( nPane );
		}else{
			/* 現在開いている編集窓のリストを得る */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* 自分のウィンドウを調べる */
				nGroup = 0;
				for( i = 0; i < nRowNum; ++i )
				{
					if( GetOwnerHwnd() == pEditNodeArr[i].GetHwnd() )
					{
						nGroup = pEditNodeArr[i].m_nGroup;
						break;
					}
				}
				if( i < nRowNum )
				{
					// 前のウィンドウ
					for( j = i - 1; j >= 0; --j )
					{
						if( nGroup == pEditNodeArr[j].m_nGroup )
							break;
					}
					if( j < 0 )
					{
						for( j = nRowNum - 1; j > i; --j )
						{
							if( nGroup == pEditNodeArr[j].m_nGroup )
								break;
						}
					}
					/*
<<<<<<< .mine
					if( i != j )
					{
						// 次のウィンドウをアクティブにする
						hwndWork = pEditNodeArr[j].GetSplitterHwnd();
						// アクティブにする
						ActivateFrameWindow( hwndWork );
						// 最後のペインをアクティブにする
						::PostMessageAny( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
					}
=======
					*/
					/* 前のウィンドウをアクティブにする */
					hwndWork = pEditNodeArr[j].GetHwnd();
					/* アクティブにする */
					ActivateFrameWindow( hwndWork );
					/* 最後のペインをアクティブにする */
					::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
//>>>>>>> .r1121
				}
				delete [] pEditNodeArr;
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	//次のウィンドウ
		nPane = m_pcEditWnd->m_cSplitterWnd.GetNextPane();
		if( -1 != nPane ){
			m_pcEditWnd->SetActivePane( nPane );
		}
		else{
			/* 現在開いている編集窓のリストを得る */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* 自分のウィンドウを調べる */
				nGroup = 0;
				for( i = 0; i < nRowNum; ++i )
				{
					if( GetOwnerHwnd() == pEditNodeArr[i].GetHwnd() )
					{
						nGroup = pEditNodeArr[i].m_nGroup;
						break;
					}
				}
				if( i < nRowNum )
				{
					// 次のウィンドウ
					for( j = i + 1; j < nRowNum; ++j )
					{
						if( nGroup == pEditNodeArr[j].m_nGroup )
							break;
					}
					if( j >= nRowNum )
					{
						for( j = 0; j < i; ++j )
						{
							if( nGroup == pEditNodeArr[j].m_nGroup )
								break;
						}
					}
					/*
<<<<<<< .mine
					if( i != j )
					{
						// 次のウィンドウをアクティブにする
						hwndWork = pEditNodeArr[j].GetSplitterHwnd();
						// アクティブにする
						ActivateFrameWindow( hwndWork );
						// 最初のペインをアクティブにする
						::PostMessageAny( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
					}
=======
					*/
					/* 次のウィンドウをアクティブにする */
					hwndWork = pEditNodeArr[j].GetHwnd();
					/* アクティブにする */
					ActivateFrameWindow( hwndWork );
					/* 最初のペインをアクティブにする */
					::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
//>>>>>>> .r1121
				}
				delete [] pEditNodeArr;
			}
		}
		return TRUE;

	default:
		return m_pcEditWnd->GetActiveView().GetCommander().HandleCommand( nCommand, TRUE, 0, 0, 0, 0 );
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

	//pCEditWnd->m_CFuncKeyWnd.Timer_ONOFF( FALSE ); // 20060126 aroka

	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
		//	Status Barが表示されていないときはm_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	/* ファイルの排他モード変更 */
	if( m_nFileShareModeOld != m_pShareData->m_Common.m_sFile.m_nFileShareMode ){
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

	CLogicPoint* posSaveAry = m_pcEditWnd->SavePhysPosOfAllView();

	/* レイアウト情報の作成 */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		hwndProgress,
		ref
	); /* レイアウト情報の変更 */

	/* ビューに設定変更を反映させる */
	for( i = 0; i < 4; ++i ){
		m_pcEditWnd->m_pcEditViewArr[i]->OnChangeSetting();
	}
	m_pcEditWnd->RestorePhysPosOfAllView( posSaveAry );
	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}




/* 編集ファイル情報を格納 */
void CEditDoc::GetFileInfo( FileInfo* pfi ) const
{
	_tcscpy(pfi->m_szPath, GetFilePath());

	pfi->m_nViewTopLine = m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	/* 表示域の一番上の行(0開始) */
	pfi->m_nViewLeftCol = m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	/* 表示域の一番左の桁(0開始) */
	//	pfi->GetCaretLayoutPos().GetX() = m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX();	/* ビュー左端からのカーソル桁位置(０開始) */
	//	pfi->GetCaretLayoutPos().GetY() = m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetY();	/* ビュー上端からのカーソル行位置(０開始) */

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	CLogicPoint ptXY;
	m_cLayoutMgr.LayoutToLogic(
		m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos(),	/* ビュー左上端からのカーソル桁位置(０開始) */
		&ptXY
	);
	pfi->m_ptCursor.Set(ptXY); //カーソル 物理位置(行頭からのバイト数, 折り返し無し行位置)

	pfi->m_bIsModified = IsModified() ? TRUE : FALSE;			/* 変更フラグ */
	pfi->m_nCharCode = m_nCharCode;				/* 文字コード種別 */

	pfi->m_bIsGrep = m_bGrepMode;
	wcscpy( pfi->m_szGrepKey, m_szGrepKey );

	//デバッグモニタ(アウトプットウインドウ)
	pfi->m_bIsDebug = m_bDebugMode;

	return;

}


// 2004/06/21 novice タグジャンプ機能追加
#if 0
/* タグジャンプ元など参照元の情報を保持する */
void CEditDoc::SetReferer( HWND hwndReferer, int nRefererX, int nRefererLine )
{
	m_hwndReferer	= hwndReferer;	/* 参照元ウィンドウ */
	m_nRefererX		= nRefererX;	/* 参照元  行頭からのバイト位置桁 */
	m_nRefererLine	= nRefererLine;	/* 参照元行  折り返し無しの物理行位置 */
	return;
}
#endif



/*! ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行

	@retval TRUE: 終了して良い / FALSE: 終了しない
*/
BOOL CEditDoc::OnFileClose( void )
{
	int			nRet;
	int			nBool;
	HWND		hwndMainFrame;
	hwndMainFrame = ::GetParent( GetSplitterHwnd() );

	//	Mar. 30, 2003 genta サブルーチンにまとめた
	AddToMRU();

	if( m_bGrepRunning ){		/* Grep処理中 */
		/* アクティブにする */
		ActivateFrameWindow( hwndMainFrame );	//@@@ 2003.06.25 MIK
		::MYMESSAGEBOX(
			hwndMainFrame,
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("Grepの処理中です。\n")
		);
		return FALSE;
	}


	/* テキストが変更されている場合 */
	if( IsModified()
	&& FALSE == m_bDebugMode	/* デバッグモニタモードのときは保存確認しない */
	){
		if( TRUE == m_bGrepMode ){	/* Grepモードのとき */
			/* Grepモードで保存確認するか */
			if( FALSE == m_pShareData->m_Common.m_sSearch.m_bGrepExitConfirm ){
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
				_T("%ts\nは変更されています。 閉じる前に保存しますか？\n\n読み取り専用で開いているので、名前を付けて保存すればいいと思います。\n"),
				IsFilePathAvailable() ? GetFilePath() : _T("（無題）")
			);
			switch( nRet ){
			case IDYES:
//				if( IsFilePathAvailable() ){
//					nBool = HandleCommand( F_FILESAVE );
//				}else{
					//nBool = HandleCommand( F_FILESAVEAS_DIALOG );
					nBool = FileSaveAs_Dialog();	// 2006.12.30 ryoji
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
				_T("%ts\nは変更されています。 閉じる前に保存しますか？"),
				IsFilePathAvailable() ? GetFilePath() : _T("（無題）")
			);
			switch( nRet ){
			case IDYES:
				if( IsFilePathAvailable() ){
					//nBool = HandleCommand( F_FILESAVE );
					nBool = FileSave();	// 2006.12.30 ryoji
				}else{
					//nBool = HandleCommand( F_FILESAVEAS_DIALOG );
					nBool = FileSaveAs_Dialog();	// 2006.12.30 ryoji
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

	m_bReadOnly = false;	/* 読み取り専用モード */
	wcscpy( m_szGrepKey, L"" );
	m_bGrepMode = FALSE;	/* Grepモード */
	m_eWatchUpdate = WU_QUERY; // Dec. 4, 2002 genta 更新監視方法

	// 2005.06.24 Moca バグ修正
	//	アウトプットウィンドウで「閉じて(無題)」を行ってもアウトプットウィンドウのまま
	if( m_bDebugMode ){
		m_pcEditWnd->SetDebugModeOFF();
	}

//	Sep. 10, 2002 genta
//	アイコン設定はファイル名設定と一体化のためここからは削除

	/* ファイルの排他ロック解除 */
	DoFileUnLock();

	/* ファイルの排他制御モード */
	m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;


	/*アンドゥ・リドゥバッファのクリア */
	/* 全要素のクリア */
	m_cOpeBuf.ClearAll();

	/* テキストデータのクリア */
	m_cDocLineMgr.Empty();
	m_cDocLineMgr.Init();

	/* 現在編集中のファイルのパス */
	//	Sep. 10, 2002 genta
	//	アイコンも同時に初期化される
	SetFilePath( _T("") );

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
	m_nCharCode = CODE_DEFAULT;
	m_bBomExist = FALSE;	//	Jul. 26, 2003 ryoji

	//	May 12, 2000
	m_cNewLineCode.SetType( EOL_CRLF );
	
	//	Oct. 2, 2005 genta 挿入モード
	SetInsMode( m_pShareData->m_Common.m_sGeneral.m_bIsINSMode != FALSE );

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
		m_pcEditWnd->m_pcEditViewArr[i]->m_cHistory->Flush();

		/* 現在の選択範囲を非選択状態に戻す */
		m_pcEditWnd->m_pcEditViewArr[i]->GetSelectionInfo().DisableSelectArea( FALSE );

		m_pcEditWnd->m_pcEditViewArr[i]->OnChangeSetting();
		m_pcEditWnd->m_pcEditViewArr[i]->GetCaret().MoveCursor( CLayoutPoint(0, 0), TRUE );
		m_pcEditWnd->m_pcEditViewArr[i]->GetCaret().m_nCaretPosX_Prev = CLayoutInt(0);
	}

	return;
}


/* ファイルのタイムスタンプのチェック処理 */
void CEditDoc::CheckFileTimeStamp( void )
{
	HWND		hwndActive;
	BOOL		bUpdate;
	bUpdate = FALSE;
	if( m_pShareData->m_Common.m_sFile.m_bCheckFileTimeStamp	/* 更新の監視 */
	 // Dec. 4, 2002 genta
	 && m_eWatchUpdate != WU_NONE
	 && m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_NOT_EXCLUSIVE	/* ファイルの排他制御モード */
	 && NULL != ( hwndActive = ::GetActiveWindow() )	/* アクティブ? */
	 && hwndActive == GetOwnerHwnd()
	 && IsFilePathAvailable()
	 && ( m_FileTime.dwLowDateTime != 0 || m_FileTime.dwHighDateTime != 0 ) 	/* 現在編集中のファイルのタイムスタンプ */

	){
		/* ファイルスタンプをチェックする */

		// 2005.10.20 ryoji FindFirstFileを使うように変更（ファイルがロックされていてもタイムスタンプ取得可能）
		FILETIME ftime;
		if( GetLastWriteTimestamp( GetFilePath(), ftime )){
			if( 0 != ::CompareFileTime( &m_FileTime, &ftime ) )	//	Aug. 13, 2003 wmlhq タイムスタンプが古く変更されている場合も検出対象とする
			{
				bUpdate = TRUE;
				m_FileTime = ftime;
			}
		}
	}

	//	From Here Dec. 4, 2002 genta
	if( bUpdate ){
		switch( m_eWatchUpdate ){
		case WU_NOTIFY:
			{
				TCHAR szText[40];
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
				auto_sprintf( szText, _T("★ファイル更新 %02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond );
				m_pcEditWnd->SendStatusMessage( szText );
			}	
			break;
		default:
			{
				m_eWatchUpdate = WU_NONE; // 更新監視の抑制

				CDlgFileUpdateQuery dlg( GetFilePath(), IsModified() );
				int result = dlg.DoModal( m_hInstance, GetSplitterHwnd(), IDD_FILEUPDATEQUERY, 0 );

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
	ECodeType	nCharCode,		/*!< [in] 文字コード種別 */
	bool	bReadOnly		/*!< [in] 読み取り専用モード */
)
{
	if( -1 == _taccess( GetFilePath(), 0 ) ){
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


	bool	bOpened;
	CLayoutInt	nViewTopLine;
	CLayoutInt	nViewLeftCol;
	CLayoutPoint ptCaretPosXY;
	nViewTopLine = m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	/* 表示域の一番上の行(0開始) */
	nViewLeftCol = m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	/* 表示域の一番左の桁(0開始) */
	ptCaretPosXY = m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos();

	TCHAR	szFilePath[MAX_PATH];
	_tcscpy( szFilePath, GetFilePath() );

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

	// レイアウト行単位のカーソル位置復元
	// ※ここではオプションのカーソル位置復元（＝改行単位）が指定されていない場合でも復元する
	// 2007.08.23 ryoji 表示領域復元
	if( ptCaretPosXY.GetY2() < m_cLayoutMgr.GetLineCount() ){
		m_pcEditWnd->GetActiveView().GetTextArea().SetViewTopLine(nViewTopLine);
		m_pcEditWnd->GetActiveView().GetTextArea().SetViewLeftCol(nViewLeftCol);
	}
	m_pcEditWnd->GetActiveView().GetCaret().MoveCursorProperly( ptCaretPosXY, TRUE );	// 2007.08.23 ryoji MoveCursor()->MoveCursorProperly()
	m_pcEditWnd->GetActiveView().GetCaret().m_nCaretPosX_Prev = m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX2();

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

	hIme = ImmGetContext( GetOwnerHwnd() );

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
	ImmReleaseContext( GetOwnerHwnd(), hIme );
}
//	To Here Nov. 20, 2000 genta


/*!	$xの展開

	特殊文字は以下の通り
	@li $  $自身
	@li A  アプリ名
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
	@li S  サクラエディタのフルパス
	@li I  iniファイルのフルパス
	@li M  現在実行しているマクロファイルパス

	@date 2003.04.03 genta wcsncpy_ex導入によるfor文の削減
	@date 2005.09.15 FILE 特殊文字S, M追加
	@date 2007.09.21 kobake 特殊文字A(アプリ名)を追加
*/
void CEditDoc::ExpandParameter(const WChar* pszSource, WChar* pszBuffer, int nBufferLen)
{
	// Apr. 03, 2003 genta 固定文字列をまとめる
	static const wchar_t	PRINT_PREVIEW_ONLY[]	= L"(印刷プレビューでのみ使用できます)";
	const int				PRINT_PREVIEW_ONLY_LEN	= _countof( PRINT_PREVIEW_ONLY ) - 1;
	static const wchar_t	NO_TITLE[]				= L"(無題)";
	const int				NO_TITLE_LEN			= _countof( NO_TITLE ) - 1;
	static const wchar_t	NOT_SAVED[]				= L"(保存されていません)";
	const int				NOT_SAVED_LEN			= _countof( NOT_SAVED ) - 1;

	const wchar_t *p, *r;	//	p：目的のバッファ。r：作業用のポインタ。
	wchar_t *q, *q_max;

	for( p = pszSource, q = pszBuffer, q_max = pszBuffer + nBufferLen; *p != '\0' && q < q_max;){
		if( *p != '$' ){
			*q++ = *p++;
			continue;
		}
		switch( *(++p) ){
		case L'$':	//	 $$ -> $
			*q++ = *p++;
			break;
		case L'A':	//アプリ名
			q = wcs_pushW( q, q_max - q, GSTR_APPNAME_W, wcslen(GSTR_APPNAME_W) );
			++p;
			break;
		case L'F':	//	開いているファイルの名前（フルパス）
			if ( !IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				r = to_wchar(GetFilePath());
				q = wcs_pushW( q, q_max - q, r, wcslen( r ));
				++p;
			}
			break;
		case L'f':	//	開いているファイルの名前（ファイル名+拡張子のみ）
			// Oct. 28, 2001 genta
			//	ファイル名のみを渡すバージョン
			//	ポインタを末尾に
			if ( ! IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				// 2002.10.13 Moca ファイル名(パスなし)を取得。日本語対応
				//	万一\\が末尾にあってもその後ろには\0があるのでアクセス違反にはならない。
				q = wcs_pushT( q, q_max - q, GetFileName());
				++p;
			}
			break;
		case L'g':	//	開いているファイルの名前（拡張子を除くファイル名のみ）
			//	From Here Sep. 16, 2002 genta
			if ( ! IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	ポインタを末尾に
				const wchar_t *dot_position, *end_of_path;
				r = to_wchar(GetFileName()); // 2002.10.13 Moca ファイル名(パスなし)を取得。日本語対応
				end_of_path = dot_position =
					r + wcslen( r );
				//	後ろから.を探す
				for( --dot_position ; dot_position > r && *dot_position != '.'
					; --dot_position )
					;
				//	rと同じ場所まで行ってしまった⇔.が無かった
				if( dot_position == r )
					dot_position = end_of_path;

				q = wcs_pushW( q, q_max - q, r, dot_position - r );
				++p;
			}
			break;
			//	To Here Sep. 16, 2002 genta
		case L'/':	//	開いているファイルの名前（フルパス。パスの区切りが/）
			// Oct. 28, 2001 genta
			if ( !IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	パスの区切りとして'/'を使うバージョン
				for( r = to_wchar(GetFilePath()); *r != L'\0' && q < q_max; ++r, ++q ){
					if( *r == L'\\' )
						*q = L'/';
					else
						*q = *r;
				}
				++p;
			}
			break;
		//	From Here 2003/06/21 Moca
		case L'N':
			if( !IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			}
			else {
				TCHAR szText[1024];
				CShareData::getInstance()->GetTransformFileNameFast( GetFilePath(), szText, 1023 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		//	To Here 2003/06/21 Moca
		//	From Here Jan. 15, 2002 hor
		case L'C':	//	現在選択中のテキスト
			{
				CNativeW cmemCurText;
				m_pcEditWnd->GetActiveView().GetCurrentTextForSearch( cmemCurText );

				q = wcs_pushW( q, q_max - q, cmemCurText.GetStringPtr(), cmemCurText.GetStringLength());
				++p;
			}
		//	To Here Jan. 15, 2002 hor
			break;
		//	From Here 2002/12/04 Moca
		case L'x':	//	現在の物理桁位置(先頭からのバイト数1開始)
			{
				wchar_t szText[11];
				_itow( m_pcEditWnd->GetActiveView().GetCaret().GetCaretLogicPos().x + 1, szText, 10 );
				q = wcs_pushW( q, q_max - q, szText);
				++p;
			}
			break;
		case L'y':	//	現在の物理行位置(1開始)
			{
				wchar_t szText[11];
				_itow( m_pcEditWnd->GetActiveView().GetCaret().GetCaretLogicPos().y + 1, szText, 10 );
				q = wcs_pushW( q, q_max - q, szText);
				++p;
			}
			break;
		//	To Here 2002/12/04 Moca
		case L'd':	//	共通設定の日付書式
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetDateFormat( systime, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		case L't':	//	共通設定の時刻書式
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		case L'p':	//	現在のページ
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					wchar_t szText[1024];
					_itow(pcEditWnd->m_pPrintPreview->GetCurPageNum() + 1, szText, 10);
					q = wcs_pushW( q, q_max - q, szText, wcslen(szText));
					++p;
				}
				else {
					q = wcs_pushW( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case L'P':	//	総ページ
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					wchar_t szText[1024];
					_itow(pcEditWnd->m_pPrintPreview->GetAllPageNum(), szText, 10);
					q = wcs_pushW( q, q_max - q, szText);
					++p;
				}
				else {
					q = wcs_pushW( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case L'D':	//	タイムスタンプ
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				TCHAR szText[1024];
				CShareData::getInstance()->MyGetDateFormat( systimeL, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			else {
				q = wcs_pushW( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case L'T':	//	タイムスタンプ
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				TCHAR szText[1024];
				CShareData::getInstance()->MyGetTimeFormat( systimeL, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			else {
				q = wcs_pushW( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case L'V':	// Apr. 4, 2003 genta
			// Version number
			{
				wchar_t buf[28]; // 6(符号含むWORDの最大長) * 4 + 4(固定部分)
				//	2004.05.13 Moca バージョン番号は、プロセスごとに取得する
				DWORD dwVersionMS, dwVersionLS;
				GetAppVersionInfo( NULL, VS_VERSION_INFO,
					&dwVersionMS, &dwVersionLS );
				int len = auto_sprintf( buf, L"%d.%d.%d.%d",
					HIWORD( dwVersionMS ),
					LOWORD( dwVersionMS ),
					HIWORD( dwVersionLS ),
					LOWORD( dwVersionLS )
				);
				q = wcs_pushW( q, q_max - q, buf, len );
				++p;
			}
			break;
		case L'h':	//	Apr. 4, 2003 genta
			//	Grep Key文字列 MAX 32文字
			//	中身はSetParentCaption()より移植
			{
				CNativeW	cmemDes;
				// m_szGrepKey → cmemDes
				LimitStringLengthW( m_szGrepKey, wcslen( m_szGrepKey ),
					(q_max - q > 32 ? 32 : q_max - q - 3), cmemDes );
				if( (int)wcslen( m_szGrepKey ) > cmemDes.GetStringLength() ){
					cmemDes.AppendString(L"...");
				}
				q = wcs_pushW( q, q_max - q, cmemDes.GetStringPtr(), cmemDes.GetStringLength());
				++p;
			}
			break;
		case L'S':	//	Sep. 15, 2005 FILE
			//	サクラエディタのフルパス
			{
				SFilePath	szPath;

				::GetModuleFileName( ::GetModuleHandle( NULL ), szPath, _countof2(szPath) );
				q = wcs_pushT( q, q_max - q, szPath );
				++p;
			}
			break;
		case 'I':	//	May. 19, 2007 ryoji
			//	iniファイルのフルパス
			{
				TCHAR	szPath[_MAX_PATH + 1];
				CShareData::getInstance()->GetIniFileName( szPath );
				q = wcs_pushT( q, q_max - q, szPath );
				++p;
			}
			break;
		case 'M':	//	Sep. 15, 2005 FILE
			//	現在実行しているマクロファイルパスの取得
			{
				// 実行中マクロのインデックス番号 (INVALID_MACRO_IDX:無効 / STAND_KEYMACRO:標準マクロ)
				switch( m_pcSMacroMgr->GetCurrentIdx() ){
				case INVALID_MACRO_IDX:
					break;
				case STAND_KEYMACRO:
					{
						TCHAR* pszMacroFilePath = CShareData::getInstance()->GetShareData()->m_szKeyMacroFileName;
						q = wcs_pushT( q, q_max - q, pszMacroFilePath );
					}
					break;
				default:
					{
						TCHAR szMacroFilePath[_MAX_PATH * 2];
						int n = CShareData::getInstance()->GetMacroFilename( m_pcSMacroMgr->GetCurrentIdx(), szMacroFilePath, _countof(szMacroFilePath) );
						if ( 0 < n ){
							q = wcs_pushT( q, q_max - q, szMacroFilePath );
						}
					}
					break;
				}
				++p;
			}
			break;
		//	Mar. 31, 2003 genta
		//	条件分岐
		//	${cond:string1$:string2$:string3$}
		//	
		case L'{':	// 条件分岐
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
		case L':':	// 条件分岐の中間
			//	条件分岐の末尾までSKIP
			p = ExParam_SkipCond( p + 1, -1 );
			break;
		case L'}':	// 条件分岐の末尾
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
const wchar_t* CEditDoc::ExParam_SkipCond(const wchar_t* pszSource, int part)
{
	if( part == 0 )
		return pszSource;
	
	int nest = 0;	// 入れ子のレベル
	bool next = true;	// 継続フラグ
	const wchar_t *p;
	for( p = pszSource; next && *p != L'\0'; ++p ) {
		if( *p == L'$' && p[1] != L'\0' ){ // $が末尾なら無視
			switch( *(++p)){
			case L'{':	// 入れ子の開始
				++nest;
				break;
			case L'}':
				if( nest == 0 ){
					//	終了ポイントに達した
					next = false; 
				}
				else {
					//	ネストレベルを下げる
					--nest;
				}
				break;
			case L':':
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
int CEditDoc::ExParam_Evaluate( const wchar_t* pCond )
{
	switch( *pCond ){
	case L'R': // 読みとり専用
		if( m_bReadOnly ){	/* 読み取り専用モード */
			return 0;
		}
		else if( SHAREMODE_NOT_EXCLUSIVE != m_nFileShareModeOld && /* ファイルの排他制御モード */
			NULL == m_hLockedFile		/* ロックしていない */
		){
			return 1;
		}
		else{
			return 2;
		}
	case L'w': // Grepモード/Output Mode
		if( m_bGrepMode ){
			return 0;
		}else if( m_bDebugMode ){
			return 1;
		}else {
			return 2;
		}
	case L'M': // キーボードマクロの記録中
		if( TRUE == m_pShareData->m_bRecordingKeyMacro &&
		m_pShareData->m_hwndRecordingKeyMacro == GetOwnerHwnd() ){ /* ウィンドウ */
			return 0;
		}else {
			return 1;
		}
	case L'U': // 更新
		if( IsModified()){
			return 0;
		}
		else {
			return 1;
		}
	case L'I': // アイコン化されているか
		if( ::IsIconic( GetOwnerHwnd() )){
			return 0;
		} else {
 			return 1;
 		}
	default:
		return 0;
	}
	return 0;
}



HWND CEditDoc::GetSplitterHwnd() const
{
	return m_pcEditWnd->m_cSplitterWnd.GetHwnd();
}

HWND CEditDoc::GetOwnerHwnd() const
{
	return m_pcEditWnd->GetHwnd();
}
