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
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2011, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>	// Apr. 03, 2003 genta
#include <DLGS.H>
#include <OleCtl.h>
#include "global.h"
#include "window/CEditWnd.h"
#include "CAppMode.h"
#include "CClipboard.h"
#include "CCodeChecker.h"
#include "CControlTray.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "CNormalProcess.h"
#include "CPrintPreview.h"
#include "CVisualProgress.h"
#include "charset/CCodeMediator.h"
#include "charset/charcode.h"
#include "debug/CRunningTimer.h"
#include "debug/Debug.h"
#include "dlg/CDlgFileUpdateQuery.h"
#include "doc/CDocLine.h" /// 2002/2/3 aroka
#include "doc/CEditDoc.h"
#include "doc/CLayout.h"	// 2007.08.22 ryoji 追加
#include "docplus/CModifyManager.h"
#include "env/CSakuraEnvironment.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "func/Funccode.h"
#include "mem/CMemoryIterator.h"	// 2007.08.22 ryoji 追加
#include "outline/CFuncInfoArr.h" /// 2002/2/3 aroka
#include "macro/CSMacroMgr.h"
#include "util/file.h"
#include "util/format.h"
#include "util/module.h"
#include "util/other_util.h"
#include "util/string_ex2.h"
#include "util/window.h"
#include "sakura_rc.h"

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
, m_hBackImg(NULL)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditDoc::CEditDoc" );

	// レイアウト管理情報の初期化
	m_cLayoutMgr.Create( this, &m_cDocLineMgr );

	// レイアウト情報の変更
	// 2008.06.07 nasukoji	折り返し方法の追加に対応
	// 「指定桁で折り返す」以外の時は折り返し幅をMAXLINEKETASで初期化する
	// 「右端で折り返す」は、この後のOnSize()で再設定される
	STypeConfig ref = m_cDocType.GetDocumentAttribute();
	if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH ){
		ref.m_nMaxLineKetas = MAXLINEKETAS;
	}
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		ref
	);

	//	自動保存の設定	//	Aug, 21, 2000 genta
	m_cAutoSaveAgent.ReloadAutoSaveParam();

	//$$ CModifyManager インスタンスを生成
	CModifyManager::Instance();

	//$$ CCodeChecker インスタンスを生成
	CCodeChecker::Instance();

	// 2008.06.07 nasukoji	テキストの折り返し方法を初期化
	m_nTextWrapMethodCur = m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;	// 折り返し方法
	m_bTextWrapMethodCurTemp = false;									// 一時設定適用中を解除

	// 文字コード種別を初期化
	m_cDocFile.m_sFileInfo.eCharCode = static_cast<ECodeType>( CShareData::getInstance()->GetShareData()->m_Types[0].m_eDefaultCodetype );
	m_cDocFile.m_sFileInfo.bBomExist = ( CShareData::getInstance()->GetShareData()->m_Types[0].m_bDefaultBom != FALSE );
	m_cDocEditor.m_cNewLineCode = static_cast<EEolType>( CShareData::getInstance()->GetShareData()->m_Types[0].m_eDefaultEoltype );

	// 排他制御オプションを初期化
	m_cDocFile.SetShareMode( GetDllShareData().m_Common.m_sFile.m_nFileShareMode );
}


CEditDoc::~CEditDoc()
{
	if( m_hBackImg ){
		::DeleteObject( m_hBackImg );
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CEditDoc::Clear()
{
	// ファイルの排他ロック解除
	m_cDocFileOperation.DoFileUnlock();

	// 書込み禁止のクリア
	m_cDocLocker.Clear();

	// アンドゥ・リドゥバッファのクリア
	m_cDocEditor.m_cOpeBuf.ClearAll();

	// テキストデータのクリア
	m_cDocLineMgr.DeleteAllLine();

	// ファイルパスとアイコンのクリア
	SetFilePathAndIcon( _T("") );

	// ファイルのタイムスタンプのクリア
	m_cDocFile.m_sFileInfo.cFileTime.ClearFILETIME();

	// 「基本」のタイプ別設定を適用
	m_cDocType.SetDocumentType( CDocTypeManager().GetDocumentTypeOfPath( m_cDocFile.GetFilePath() ), true );
	// 2008.06.07 nasukoji	折り返し方法の追加に対応
	STypeConfig ref = m_cDocType.GetDocumentAttribute();
	if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH ){
		ref.m_nMaxLineKetas = MAXLINEKETAS;
	}
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
	m_cDocFile.m_sFileInfo.eCharCode = static_cast<ECodeType>( CShareData::getInstance()->GetShareData()->m_Types[0].m_eDefaultCodetype );
	m_cDocFile.m_sFileInfo.bBomExist = ( CShareData::getInstance()->GetShareData()->m_Types[0].m_bDefaultBom != FALSE );
	m_cDocEditor.m_cNewLineCode = static_cast<EEolType>( CShareData::getInstance()->GetShareData()->m_Types[0].m_eDefaultEoltype );

	//	Oct. 2, 2005 genta 挿入モード
	m_cDocEditor.SetInsMode( GetDllShareData().m_Common.m_sGeneral.m_bIsINSMode );
}

void CEditDoc::SetBackgroundImage()
{
	CFilePath path = m_cDocType.GetDocumentAttribute().m_szBackImgPath.c_str();
	if( m_hBackImg ){
		::DeleteObject( m_hBackImg );
		m_hBackImg = NULL;
	}
	if( 0 == path[0] ){
		return;
	}
	if( _IS_REL_PATH(path.c_str()) ){
		CFilePath fullPath;
		GetInidirOrExedir( &fullPath[0], &path[0] );
		path = fullPath;
	}
	const TCHAR* ext = path.GetExt();
	if( 0 != auto_stricmp(ext, _T(".bmp")) ){
		HANDLE hFile = ::CreateFile(path.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
		if( hFile == INVALID_HANDLE_VALUE ){
			return;
		}
		DWORD fileSize  = ::GetFileSize(hFile, NULL);
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, fileSize);
		if( hGlobal == NULL ){
			::CloseHandle(hFile);
			return;
		}
		DWORD nRead;
		BOOL bRead = ::ReadFile(hFile, GlobalLock(hGlobal), fileSize, &nRead, NULL);
		::CloseHandle(hFile);
		hFile = NULL;
		if( !bRead ){
			::GlobalFree(hGlobal);
			return;
		}
		::GlobalUnlock(hGlobal);
		{
			IPicture* iPicture = NULL;
			IStream*  iStream = NULL;
			//hGlobalの管理を移譲
			if( S_OK != ::CreateStreamOnHGlobal(hGlobal, TRUE, &iStream) ){
				GlobalFree(hGlobal);
			}else{
				if( S_OK != ::OleLoadPicture(iStream, fileSize, FALSE, IID_IPicture, (void**)&iPicture) ){
				}else{
					HBITMAP hBitmap = NULL;
					short imgType = PICTYPE_NONE;
					if( S_OK == iPicture->get_Type(&imgType) && imgType == PICTYPE_BITMAP &&
					    S_OK == iPicture->get_Handle((OLE_HANDLE*)&hBitmap) ){
						m_nBackImgWidth = m_nBackImgHeight = 1;
						m_hBackImg = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);
					}
				}
			}
			if( iStream )  iStream->Release();
			if( iPicture ) iPicture->Release();
		}
	}else{
		m_hBackImg = (HBITMAP)::LoadImage(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	}
	if( m_hBackImg ){
		BITMAP bmp;
		GetObject(m_hBackImg, sizeof(BITMAP), &bmp);
		m_nBackImgWidth  = bmp.bmWidth;
		m_nBackImgHeight = bmp.bmHeight;
		if( 0 == m_nBackImgWidth || 0 == m_nBackImgHeight ){
			::DeleteObject(m_hBackImg);
			m_hBackImg = NULL;
		}
	}
}

/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */
void CEditDoc::InitAllView( void )
{

	m_nCommandExecNum = 0;	/* コマンド実行回数 */

	// 2008.05.30 nasukoji	テキストの折り返し方法を初期化
	m_nTextWrapMethodCur = m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;	// 折り返し方法
	m_bTextWrapMethodCurTemp = false;									// 一時設定適用中を解除

	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if( m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		m_cLayoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
	else
		m_cLayoutMgr.ClearLayoutLineWidth();	// 各行のレイアウト行長の記憶をクリアする

	// CEditWndに引越し
	m_pcEditWnd->InitAllViews();

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
	m_cFuncLookup.Init( GetDllShareData().m_Common.m_sMacro.m_MacroTable, &GetDllShareData().m_Common );

	SetBackgroundImage();

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

//! ドキュメントのデフォルト文字コードを取得
ECodeType CEditDoc::GetDefaultDocumentEncoding() const
{
	int ntype = m_cDocType.GetDocumentType().GetIndex();
	return static_cast<ECodeType>( CShareData::getInstance()->GetShareData()->m_Types[ntype].m_eDefaultCodetype );
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
	pfi->m_nType = m_cDocType.GetDocumentType();

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
	//	編集可能の場合
	if( IsEditable() )
		return false; // 常に書き換え許可

	//	編集禁止の場合
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
	case F_HANKATATOZENKATA:
	case F_HANKATATOZENHIRA:
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
	@date 2008.05.30 nasukoji	テキストの折り返し方法の変更処理を追加
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
		m_cDocFile.SetShareMode( GetDllShareData().m_Common.m_sFile.m_nFileShareMode );

		/* ファイルの排他ロック解除 */
		m_cDocFileOperation.DoFileUnlock();

		// ファイル書込可能のチェック処理
		bool bOld = m_cDocLocker.IsDocWritable();
		m_cDocLocker.CheckWritable(bOld);	// 書込可から不可に遷移したときだけメッセージを出す（出過ぎると鬱陶しいよね？）
		if(bOld != m_cDocLocker.IsDocWritable()){
			pCEditWnd->UpdateCaption();
		}

		/* ファイルの排他ロック */
		if( m_cDocLocker.IsDocWritable() ){
			m_cDocFileOperation.DoFileLock();
		}
	}

	/* 共有データ構造体のアドレスを返す */
	CFileNameManager::Instance()->TransformFileName_MakeCache();

	// 文書種別
	m_cDocType.SetDocumentType( CDocTypeManager().GetDocumentTypeOfPath( m_cDocFile.GetFilePath() ), false );

	CLogicPoint* posSaveAry = m_pcEditWnd->SavePhysPosOfAllView();

	/* レイアウト情報の作成 */
	STypeConfig ref = m_cDocType.GetDocumentAttribute();
	{
		// 2008.06.07 nasukoji	折り返し方法の追加に対応
		// 折り返し方法の一時設定とタイプ別設定が一致したら一時設定適用中は解除
		if( m_nTextWrapMethodCur == ref.m_nTextWrapMethod )
			m_bTextWrapMethodCurTemp = false;		// 一時設定適用中を解除

		// 一時設定適用中でなければ折り返し方法変更
		if( !m_bTextWrapMethodCurTemp )
			m_nTextWrapMethodCur = ref.m_nTextWrapMethod;	// 折り返し方法

		// 指定桁で折り返す：タイプ別設定を使用
		// 右端で折り返す：仮に現在の折り返し幅を使用
		// 上記以外：MAXLINEKETASを使用
		if( m_nTextWrapMethodCur != WRAP_SETTING_WIDTH ){
			if( m_nTextWrapMethodCur == WRAP_WINDOW_WIDTH )
				ref.m_nMaxLineKetas = m_cLayoutMgr.GetMaxLineKetas();	// 現在の折り返し幅
			else
				ref.m_nMaxLineKetas = MAXLINEKETAS;
		}
	}
	CProgressSubject* pOld = CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&m_cLayoutMgr);
	m_cLayoutMgr.SetLayoutInfo(true,ref);
	CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(pOld);

	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if( m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		m_cLayoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
	else
		m_cLayoutMgr.ClearLayoutLineWidth();	// 各行のレイアウト行長の記憶をクリアする

	/* ビューに設定変更を反映させる */
	int viewCount = m_pcEditWnd->GetAllViewCount();
	for( i = 0; i < viewCount; ++i ){
		m_pcEditWnd->GetView(i).OnChangeSetting();
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

	//GREPモードで、かつ、「GREPモードで保存確認するか」がOFFだったら、保存確認しない
	// 2011.11.13 GrepモードでGrep直後は"未編集"状態になっているが保存確認が必要
	if( CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode ){
		if( !GetDllShareData().m_Common.m_sSearch.m_bGrepExitConfirm ){
			return TRUE;
		}
	}else{
		//テキストが変更されていない場合は保存確認しない
		if( !m_cDocEditor.IsModified() ) return TRUE;
	}

	// -- -- 保存確認 -- -- //
	TCHAR szGrepTitle[90];
	LPCTSTR pszTitle = m_cDocFile.GetFilePathClass().IsValidPath() ? m_cDocFile.GetFilePath() : _T("(無題)");
	if( CEditApp::Instance()->m_pcGrepAgent->m_bGrepMode ){
		LPCWSTR		pszGrepKey = CAppMode::Instance()->m_szGrepKey;
		int			nLen = (int)wcslen( pszGrepKey );
		CNativeW	cmemDes;
		LimitStringLengthW( pszGrepKey , nLen, 64, cmemDes );
		auto_sprintf( szGrepTitle, _T("【Grep】%ls%ts"),
			cmemDes.GetStringPtr(),
			( nLen > cmemDes.GetStringLength() ) ? _T("...") : _T("")
		);
		pszTitle = szGrepTitle;
	}
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
			pszTitle
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
			pszTitle
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

/*!	@brief マクロ自動実行

	@param type [in] 自動実行マクロ番号
	@return

	@author ryoji
	@date 2006.09.01 ryoji 作成
	@date 2007.07.20 genta HandleCommandに追加情報を渡す．
		自動実行マクロで発行したコマンドはキーマクロに保存しない
*/
void CEditDoc::RunAutoMacro( int idx, LPCTSTR pszSaveFilePath )
{
	// 開ファイル／タイプ変更時はアウトラインを再解析する
	if( pszSaveFilePath == NULL ){
		m_pcEditWnd->m_cDlgFuncList.Refresh();
	}

	static bool bRunning = false;
	if( bRunning )
		return;	// 再入り実行はしない

	bRunning = true;
	if( CEditApp::Instance()->m_pcSMacroMgr->IsEnabled(idx) ){
		if( !( ::GetAsyncKeyState(VK_SHIFT) & 0x8000 ) ){	// Shift キーが押されていなければ実行
			if( NULL != pszSaveFilePath )
				m_cDocFile.SetSaveFilePath(pszSaveFilePath);
			//	2007.07.20 genta 自動実行マクロで発行したコマンドはキーマクロに保存しない
			HandleCommand((EFunctionCode)(( F_USERMACRO_0 + idx ) | FA_NONRECORD) );
			m_cDocFile.SetSaveFilePath(_T(""));
		}
	}
	bRunning = false;
}

