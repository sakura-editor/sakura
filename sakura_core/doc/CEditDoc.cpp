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
#include "doc/CEditDoc.h"
#include "debug/Debug.h"
#include "funccode.h"
#include "debug/CRunningTimer.h"
#include "charcode.h"
#include <DLGS.H>
#include "CShareData.h"
#include "window/CEditWnd.h"
#include "sakura_rc.h"
#include "global.h"
#include "CFuncInfoArr.h" /// 2002/2/3 aroka
#include "CMarkMgr.h"///
#include "doc/CDocLine.h" /// 2002/2/3 aroka
#include "CPrintPreview.h"
#include "dlg/CDlgFileUpdateQuery.h"
#include <assert.h> /// 2002/11/2 frozen
#include "CClipboard.h"
#include "doc/CLayout.h"	// 2007.08.22 ryoji 追加
#include "mem/CMemoryIterator.h"	// 2007.08.22 ryoji 追加
#include "charset/CCodeMediator.h"
#include "util/file.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "util/format.h"
#include "util/module.h"
#include "CEditApp.h"
#include "util/other_util.h"
#include "CSakuraEnvironment.h"
#include "CNormalProcess.h"
#include "CControlTray.h"
#include "docplus/CModifyManager.h"

#define IDT_ROLLMOUSE	1

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
	@note
		m_pcEditWnd はコンストラクタ内では使用しないこと．

	@date 2000.05.12 genta 初期化方法変更
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
	@date 2004.06.21 novice タグジャンプ機能追加
*/
CEditDoc::CEditDoc(CEditApp* pcApp)
: m_pcEditWnd(pcApp->m_pcEditWnd)
, m_nCommandExecNum( 0 )			/* コマンド実行回数 */
, m_cDocFile(this)
, m_cDocOutline(this)
, m_cDocType(this)
, m_cDocEditor(this)
, m_cDocFileOperation(this)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::CEditDoc" );

	// レイアウト管理情報の初期化
	m_cLayoutMgr.Create( this, &m_cDocLineMgr );

	// レイアウト情報の変更
	STypeConfig& ref = m_cDocType.GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		ref
	);

	//	自動保存の設定	//	Aug, 21, 2000 genta
	m_cAutoSaveAgent.ReloadAutoSaveParam();

	//$$ CModifyManager インスタンスを生成
	CModifyManager::Instance();
}


CEditDoc::~CEditDoc()
{
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CEditDoc::Clear()
{
	// ファイルの排他ロック解除
	m_cDocFile.DoFileUnLock();

	// アンドゥ・リドゥバッファのクリア
	m_cDocEditor.m_cOpeBuf.ClearAll();

	// テキストデータのクリア
	m_cDocLineMgr.DeleteAllLine();

	// ファイルパスとアイコンのクリア
	SetFilePathAndIcon( _T("") );

	// ファイルのタイムスタンプのクリア
	m_cDocFile.m_sFileInfo.cFileTime.ClearFILETIME();

	// 「基本」のタイプ別設定を適用
	m_cDocType.SetDocumentType( CShareData::getInstance()->GetDocumentType( m_cDocFile.GetFilePath() ), true );
	STypeConfig& ref = m_cDocType.GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		ref
	);
}

/* 既存データのクリア */
void CEditDoc::InitDoc()
{
	CAppMode::Instance()->SetViewMode(false);	// ビューモード $$ 今後OnClearDocを用意したい
	wcscpy( CAppMode::Instance()->m_szGrepKey, L"" );	//$$

	CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode = false;	/* Grepモード */	//$$同上
	m_cAutoReloadAgent.m_eWatchUpdate = WU_QUERY; // Dec. 4, 2002 genta 更新監視方法 $$

	// 2005.06.24 Moca バグ修正
	//	アウトプットウィンドウで「閉じて(無題)」を行ってもアウトプットウィンドウのまま
	if( CAppMode::Instance()->IsDebugMode() ){
		CAppMode::Instance()->SetDebugModeOFF();
	}

//	Sep. 10, 2002 genta
//	アイコン設定はファイル名設定と一体化のためここからは削除

	Clear();

	/* 変更フラグ */
	m_cDocEditor.SetModified(false,false);	//	Jan. 22, 2002 genta

	/* 文字コード種別 */
	m_cDocFile.m_sFileInfo.eCharCode = CODE_DEFAULT;
	m_cDocFile.m_sFileInfo.bBomExist = false;	//	Jul. 26, 2003 ryoji

	//	May 12, 2000
	m_cDocEditor.m_cNewLineCode.SetType( EOL_CRLF );
	
	//	Oct. 2, 2005 genta 挿入モード
	m_cDocEditor.SetInsMode( GetDllShareData().m_Common.m_sGeneral.m_bIsINSMode );
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
	CImageListMgr* pcIcons
 )
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::Create" );

	//	Oct. 2, 2001 genta
	m_cFuncLookup.Init( GetDllShareData().m_MacroTable, &GetDllShareData().m_Common );


	MY_TRACETIME( cRunningTimer, "End: PropSheet" );

	return TRUE;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           設定                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	ファイル名の設定
	
	ファイル名を設定すると同時に，ウィンドウアイコンを適切に設定する．
	
	@param szFile [in] ファイルのパス名
	
	@author genta
	@date 2002.09.09
*/
void CEditDoc::SetFilePathAndIcon(const TCHAR* szFile)
{
	TCHAR szWork[MAX_PATH];
	if( ::GetLongFileName( szFile, szWork ) ){
		szFile = szWork;
	}
	m_cDocFile.SetFilePath(szFile);
	m_cDocType.SetDocumentIcon();
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           属性                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ドキュメントの文字コードを取得
ECodeType CEditDoc::GetDocumentEncoding() const
{
	return m_cDocFile.m_sFileInfo.eCharCode;
}

//! ドキュメントの文字コードを設定
void CEditDoc::SetDocumentEncoding(ECodeType eCharCode)
{
	if(!IsValidCodeType(eCharCode))return; //無効な範囲を受け付けない

	m_cDocFile.m_sFileInfo.eCharCode = eCharCode;
}

void CEditDoc::GetSaveInfo(SSaveInfo* pSaveInfo) const
{
	pSaveInfo->cFilePath = this->m_cDocFile.GetFilePath(),
	pSaveInfo->eCharCode = this->m_cDocFile.m_sFileInfo.eCharCode;
	pSaveInfo->cEol      = this->m_cDocEditor.m_cNewLineCode; //編集時改行コードを保存時改行コードとして設定
	pSaveInfo->bBomExist = this->m_cDocFile.m_sFileInfo.bBomExist;
}


/* 編集ファイル情報を格納 */
void CEditDoc::GetEditInfo(
	EditInfo* pfi	//!< [out]
) const
{
	//ファイルパス
	_tcscpy(pfi->m_szPath, m_cDocFile.GetFilePath());

	//表示域
	pfi->m_nViewTopLine = m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	/* 表示域の一番上の行(0開始) */
	pfi->m_nViewLeftCol = m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	/* 表示域の一番左の桁(0開始) */

	//キャレット位置
	pfi->m_ptCursor.Set(m_pcEditWnd->GetActiveView().GetCaret().GetCaretLogicPos());

	//各種状態
	pfi->m_bIsModified = m_cDocEditor.IsModified();			/* 変更フラグ */
	pfi->m_nCharCode = m_cDocFile.m_sFileInfo.eCharCode;	/* 文字コード種別 */

	//GREPモード
	pfi->m_bIsGrep = CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode;
	wcscpy( pfi->m_szGrepKey, CAppMode::Instance()->m_szGrepKey );

	//デバッグモニタ (アウトプットウインドウ) モード
	pfi->m_bIsDebug = CAppMode::Instance()->IsDebugMode();
}


//	From Here Aug. 14, 2000 genta
//
//	書き換えが禁止されているかどうか
//	戻り値: true: 禁止 / false: 許可
//
bool CEditDoc::IsModificationForbidden( EFunctionCode nCommand )
{
	//	ビューモードでも上書き禁止でもなければ
	if( !CAppMode::Instance()->IsViewMode() && m_cDocLocker.IsDocWritable() )
		return false; // 常に書き換え許可

	//	上書き禁止モードの場合
	//	暫定Case文: 実際にはもっと効率の良い方法を使うべき
	switch( nCommand ){
	//	ファイルを書き換えるコマンドは使用禁止
	case F_WCHAR:
	case F_IME_CHAR:
	case F_DELETE:
	case F_DELETE_BACK:
	case F_WordDeleteToEnd:
	case F_WordDeleteToStart:
	case F_WordDelete:
	case F_WordCut:
	case F_LineDeleteToStart:
	case F_LineDeleteToEnd:
	case F_LineCutToStart:
	case F_LineCutToEnd:
	case F_DELETE_LINE:
	case F_CUT_LINE:
	case F_DUPLICATELINE:
	case F_INDENT_TAB:
	case F_UNINDENT_TAB:
	case F_INDENT_SPACE:
	case F_UNINDENT_SPACE:
	case F_CUT:
	case F_PASTE:
	case F_INS_DATE:
	case F_INS_TIME:
	case F_CTRL_CODE_DIALOG:	//@@@ 2002.06.02 MIK
	case F_INSTEXT_W:
	case F_ADDTAIL_W:
	case F_PASTEBOX:
	case F_REPLACE_DIALOG:
	case F_REPLACE:
	case F_REPLACE_ALL:
	case F_CODECNV_EMAIL:
	case F_CODECNV_EUC2SJIS:
	case F_CODECNV_UNICODE2SJIS:
	case F_CODECNV_UNICODEBE2SJIS:
	case F_CODECNV_SJIS2JIS:
	case F_CODECNV_SJIS2EUC:
	case F_CODECNV_UTF82SJIS:
	case F_CODECNV_UTF72SJIS:
	case F_CODECNV_SJIS2UTF7:
	case F_CODECNV_SJIS2UTF8:
	case F_CODECNV_AUTO2SJIS:
	case F_TOLOWER:
	case F_TOUPPER:
	case F_TOHANKAKU:
	case F_TOHANKATA:				// 2002/08/29 ai
	case F_TOZENEI:					// 2001/07/30 Misaka
	case F_TOHANEI:
	case F_TOZENKAKUKATA:
	case F_TOZENKAKUHIRA:
	case F_HANKATATOZENKAKUKATA:
	case F_HANKATATOZENKAKUHIRA:
	case F_TABTOSPACE:
	case F_SPACETOTAB:  //---- Stonee, 2001/05/27
	case F_HOKAN:
	case F_CHGMOD_INS:
	case F_LTRIM:		// 2001.12.03 hor
	case F_RTRIM:		// 2001.12.03 hor
	case F_SORT_ASC:	// 2001.12.11 hor
	case F_SORT_DESC:	// 2001.12.11 hor
	case F_MERGE:		// 2001.12.11 hor
	case F_UNDO:		// 2007.10.12 genta
	case F_REDO:		// 2007.10.12 genta
		return true;
	}
	return false;	//	デフォルトで書き換え許可
}
//	To Here Aug. 14, 2000 genta


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           状態                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! @brief このウィンドウで新しいファイルを開けるか

	新しいウィンドウを開かずに現在のウィンドウを再利用できるかどうかのテストを行う．
	変更済み，ファイルを開いている，Grepウィンドウ，アウトプットウィンドウの場合には
	再利用不可．

	@author Moca
	@date 2005.06.24 Moca
*/
bool CEditDoc::IsAcceptLoad() const
{
	if(m_cDocEditor.IsModified())return false;
	if(m_cDocFile.GetFilePathClass().IsValidPath())return false;
	if(CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode)return false;
	if(CAppMode::Instance()->IsDebugMode())return false;
	return true;
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! コマンドコードによる処理振り分け

	@param[in] nCommand MAKELONG( コマンドコード，送信元識別子 )

	@date 2006.05.19 genta 上位16bitに送信元の識別子が入るように変更
	@date 2007.06.20 ryoji グループ内で巡回するように変更
*/
BOOL CEditDoc::HandleCommand( EFunctionCode nCommand )
{
	//	May. 19, 2006 genta 上位16bitに送信元の識別子が入るように変更したので
	//	下位16ビットのみを取り出す
	switch( LOWORD( nCommand )){
	case F_PREVWINDOW:	//前のウィンドウ
		{
			int nPane = m_pcEditWnd->m_cSplitterWnd.GetPrevPane();
			if( -1 != nPane ){
				m_pcEditWnd->SetActivePane( nPane );
			}else{
				CControlTray::ActiveNextWindow();
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	//次のウィンドウ
		{
			int nPane = m_pcEditWnd->m_cSplitterWnd.GetNextPane();
			if( -1 != nPane ){
				m_pcEditWnd->SetActivePane( nPane );
			}
			else{
				CControlTray::ActivePrevWindow();
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
void CEditDoc::OnChangeSetting()
{
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
	if( m_cDocFile.GetShareMode() != GetDllShareData().m_Common.m_sFile.m_nFileShareMode ){
		/* ファイルの排他ロック */
		m_cDocFile.DoFileLock(GetDllShareData().m_Common.m_sFile.m_nFileShareMode);
	}

	/* 共有データ構造体のアドレスを返す */
	CShareData::getInstance()->TransformFileName_MakeCache();

	// 文書種別
	m_cDocType.SetDocumentType( CShareData::getInstance()->GetDocumentType( m_cDocFile.GetFilePath() ), false );

	CLogicPoint* posSaveAry = m_pcEditWnd->SavePhysPosOfAllView();

	/* レイアウト情報の作成 */
	const STypeConfig& ref = m_cDocType.GetDocumentAttribute();
	CProgressSubject* pOld = CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&m_cLayoutMgr);
	m_cLayoutMgr.SetLayoutInfo(true,ref);
	CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(pOld);

	/* ビューに設定変更を反映させる */
	for( i = 0; i < 4; ++i ){
		m_pcEditWnd->m_pcEditViewArr[i]->OnChangeSetting();
	}
	m_pcEditWnd->RestorePhysPosOfAllView( posSaveAry );
	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}

/*! ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行

	@retval TRUE: 終了して良い / FALSE: 終了しない
*/
BOOL CEditDoc::OnFileClose()
{
	int			nRet;
	int			nBool;

	//クローズ事前処理
	ECallbackResult eBeforeCloseResult = NotifyBeforeClose();
	if(eBeforeCloseResult==CALLBACK_INTERRUPT)return FALSE;


	// デバッグモニタモードのときは保存確認しない
	if(CAppMode::Instance()->IsDebugMode())return TRUE;

	//テキストが変更されていない場合は保存確認しない
	if(!m_cDocEditor.IsModified())return TRUE;

	//GREPモードで、かつ、「GREPモードで保存確認するか」がOFFだったら、保存確認しない
	if( CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode ){
		if( !GetDllShareData().m_Common.m_sSearch.m_bGrepExitConfirm ){
			return TRUE;
		}
	}

	// -- -- 保存確認 -- -- //
	
	/* ウィンドウをアクティブにする */
	HWND	hwndMainFrame = CEditWnd::Instance()->GetHwnd();
	ActivateFrameWindow( hwndMainFrame );
	if( CAppMode::Instance()->IsViewMode() ){	/* ビューモード */
		::MessageBeep( MB_ICONQUESTION );
		nRet = ::MYMESSAGEBOX(
			hwndMainFrame,
			MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ts\nは変更されています。 閉じる前に保存しますか？\n\nビューモードで開いているので、名前を付けて保存すればいいと思います。\n"),
			m_cDocFile.GetFilePathClass().IsValidPath() ? m_cDocFile.GetFilePath() : _T("（無題）")
		);
		switch( nRet ){
		case IDYES:
			nBool = m_cDocFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			return FALSE;
		}
	}
	else{
		::MessageBeep( MB_ICONQUESTION );
		nRet = ::MYMESSAGEBOX(
			hwndMainFrame,
			MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ts\nは変更されています。 閉じる前に保存しますか？"),
			m_cDocFile.GetFilePathClass().IsValidPath() ? m_cDocFile.GetFilePath() : _T("（無題）")
		);
		switch( nRet ){
		case IDYES:
			if( m_cDocFile.GetFilePathClass().IsValidPath() ){
				nBool = m_cDocFileOperation.FileSave();	// 2006.12.30 ryoji
			}
			else{
				nBool = m_cDocFileOperation.FileSaveAs();	// 2006.12.30 ryoji
			}
			return nBool;
		case IDNO:
			return TRUE;
		case IDCANCEL:
		default:
			return FALSE;
		}
	}
}

