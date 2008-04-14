#include "stdafx.h"
#include "CDocFileOperation.h"
#include "CWaitCursor.h"
#include "CControlTray.h"
#include "util/window.h"
#include "CSakuraEnvironment.h"
#include "doc/CDocVisitor.h"


bool CDocFileOperation::_ToDoLock() const
{
	// ファイルを開いていない
	if( !m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath() )return false;

	// ビューモード
	if( CAppMode::Instance()->IsViewMode() )return false;
	
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ロードUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 「ファイルを開く」ダイアログ */
//	Mar. 30, 2003 genta	ファイル名未定時の初期ディレクトリをカレントフォルダに
bool CDocFileOperation::OpenFileDialog(
	HWND				hwndParent,		//!< [in]
	const TCHAR*		pszOpenFolder,	//!< [in]     NULL以外を指定すると初期フォルダを指定できる
	SLoadInfo*			pLoadInfo		//!< [in/out] ロード情報
)
{
	/* アクティブにする */
	ActivateFrameWindow( hwndParent );

	// ファイルオープンダイアログを表示
	m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.Create(
		CNormalProcess::Instance()->GetProcessInstance(),
		hwndParent,
		_T("*.*"),
		pszOpenFolder ? pszOpenFolder : CSakuraEnvironment::GetDlgInitialDir().c_str(),	// 初期フォルダ
		CMRU().GetPathList(),															// MRUリストのファイルのリスト
		CMRUFolder().GetPathList()														// OPENFOLDERリストのファイルのリスト
	);
	return m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.DoModalOpenDlg( pLoadInfo );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ロードフロー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDocFileOperation::DoLoadFlow(SLoadInfo* pLoadInfo)
{
	ELoadResult eLoadResult = LOADED_FAILURE;

	try{
		//セーブ前チェック
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyCheckLoad(pLoadInfo))throw CFlowInterruption();

		//セーブ処理
		m_pcDocRef->NotifyBeforeLoad(*pLoadInfo);	//前処理
		m_pcDocRef->NotifyLoad(*pLoadInfo);			//本処理
		m_pcDocRef->NotifyAfterLoad(*pLoadInfo);		//後処理

		//結果
		eLoadResult = LOADED_OK; //###仮
	}
	catch(CFlowInterruption){
		eLoadResult = LOADED_INTERRUPT;
	}
	catch(...){
		//予期せぬ例外が発生した場合も NotifyFinalLoad は必ず呼ぶ！
		m_pcDocRef->NotifyFinalLoad(LOADED_FAILURE);
		throw;
	}

	//最終処理
	m_pcDocRef->NotifyFinalLoad(eLoadResult);

	return eLoadResult==LOADED_OK;
}

//! ファイルを開く
bool CDocFileOperation::FileLoad(
	SLoadInfo*	pLoadInfo		//!< [in/out]
)
{
	return DoLoadFlow(pLoadInfo);
}

//! 同一ファイルの再オープン
void CDocFileOperation::ReloadCurrentFile(
	ECodeType	nCharCode		//!< [in] 文字コード種別
)
{
	bool bViewMode = CAppMode::Instance()->IsViewMode();	// ビューモード

	if( !fexist(m_pcDocRef->m_cDocFile.GetFilePath()) ){
		/* ファイルが存在しない */
		//	Jul. 26, 2003 ryoji BOMを標準設定に
		m_pcDocRef->m_cDocFile.m_sFileInfo.eCharCode = nCharCode;
		switch( m_pcDocRef->m_cDocFile.m_sFileInfo.eCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_pcDocRef->m_cDocFile.m_sFileInfo.bBomExist = true;
			break;
		case CODE_UTF8:
		default:
			m_pcDocRef->m_cDocFile.m_sFileInfo.bBomExist = false;
			break;
		}
		return;
	}

	//カーソル位置保存
	CLayoutInt		nViewTopLine = m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	/* 表示域の一番上の行(0開始) */
	CLayoutInt		nViewLeftCol = m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	/* 表示域の一番左の桁(0開始) */
	CLayoutPoint	ptCaretPosXY = m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos();

	//文字コード確認フラグオフ
	bool backup_bQueryIfCodeChange = GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange; //####仮
	GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange = false; //####仮

	//ロード
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath=m_pcDocRef->m_cDocFile.GetFilePath();
	sLoadInfo.eCharCode=nCharCode;
	sLoadInfo.bViewMode=bViewMode;
	this->DoLoadFlow(&sLoadInfo);

	//文字コード確認フラグ復元
	GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange = backup_bQueryIfCodeChange; //####仮

	// カーソル位置復元 (※ここではオプションのカーソル位置復元（＝改行単位）が指定されていない場合でも復元する)
	// 2007.08.23 ryoji 表示領域復元
	if( ptCaretPosXY.GetY2() < m_pcDocRef->m_cLayoutMgr.GetLineCount() ){
		m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().SetViewTopLine(nViewTopLine);
		m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().SetViewLeftCol(nViewLeftCol);
	}
	m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().MoveCursorProperly( ptCaretPosXY, TRUE );	// 2007.08.23 ryoji MoveCursor()->MoveCursorProperly()
	m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().m_nCaretPosX_Prev = m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX2();
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         セーブUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



//pszOpenFolder pszOpenFolder


/*! 「ファイル名を付けて保存」ダイアログ
	@date 2001.02.09 genta	改行コードを示す引数追加
	@date 2003.03.30 genta	ファイル名未定時の初期ディレクトリをカレントフォルダに
	@date 2003.07.20 ryoji	BOMの有無を示す引数追加
	@date 2006.11.10 ryoji	ユーザー指定の拡張子を状況依存で変化させる
*/
bool CDocFileOperation::SaveFileDialog(
	SSaveInfo*	pSaveInfo	//!< [out]
)
{
	//ダイアログオープン時のディレクトリを決定
	std::tstring strDefFolder; // デフォルトフォルダ
	{
		// ファイルパスのディレクトリ
		strDefFolder = this->m_pcDocRef->m_cDocFile.GetFilePathClass().GetDirPath();

		// カレントディレクトリ
		if(strDefFolder.length()==0){
			TCHAR	buf[_MAX_PATH];
			int		n = ::GetCurrentDirectory( _MAX_PATH, buf );
			if( 0 != n && n < _MAX_PATH )strDefFolder = buf;
		}
	}

	//拡張子指定
	TCHAR	szDefaultWildCard[_MAX_PATH + 10];	// ユーザー指定拡張子
	{
		//ファイルパスが無い場合は *.txt とする
		if(!this->m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath()){
			_tcscpy(szDefaultWildCard, _T("*.txt"));
		}
		//ファイルパスが有る場合は *.(今の拡張子) とする
		else{
			LPCTSTR szExt = this->m_pcDocRef->m_cDocFile.GetFilePathClass().GetExt();
			if(!*szExt)szExt = _T(".*");
			_tcscpy(szDefaultWildCard, _T("*"));
			_tcscat(szDefaultWildCard, szExt);
		}

		//「新規から保存時は全ファイル表示」オプション
		if( GetDllShareData().m_Common.m_sFile.m_bNoFilterSaveNew )
			_tcscat(szDefaultWildCard, _T(";*.*"));	// 全ファイル表示
	}

	// ダイアログを表示
	m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.Create(
		CNormalProcess::Instance()->GetProcessInstance(),
		CEditWnd::Instance()->GetHwnd(),
		szDefaultWildCard,
		strDefFolder.c_str(),
		CMRU().GetPathList(),		//	最近のファイル
		CMRUFolder().GetPathList()	//	最近のフォルダ
	);
	return m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.DoModalSaveDlg( pSaveInfo, pSaveInfo->eCharCode == CODE_CODEMAX );
}

//!「ファイル名を付けて保存」ダイアログ
bool CDocFileOperation::SaveFileDialog(LPTSTR szPath)
{
	SSaveInfo sSaveInfo;
	sSaveInfo.cFilePath = szPath;
	sSaveInfo.eCharCode = CODE_CODEMAX; //###トリッキー
	bool bRet = SaveFileDialog(&sSaveInfo);
	_tcscpy_s(szPath, _MAX_PATH, sSaveInfo.cFilePath);
	return bRet;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       セーブフロー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDocFileOperation::DoSaveFlow(const SSaveInfo& _sSaveInfo)
{
	ESaveResult eSaveResult = SAVED_FAILURE;
	SSaveInfo sSaveInfo = _sSaveInfo;

	try{
		//セーブ前チェック
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyCheckSave(&sSaveInfo))throw CFlowInterruption();

		//セーブ前おまけ処理
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyPreBeforeSave(&sSaveInfo))throw CFlowInterruption();

		//セーブ処理
		m_pcDocRef->NotifyBeforeSave(sSaveInfo);	//前処理
		m_pcDocRef->NotifySave(sSaveInfo);			//本処理
		m_pcDocRef->NotifyAfterSave(sSaveInfo);		//後処理

		//結果
		eSaveResult = SAVED_OK; //###仮
	}
	catch(CFlowInterruption){
		eSaveResult = SAVED_INTERRUPT;
	}
	catch(...){
		//予期せぬ例外が発生した場合も NotifyFinalSave は必ず呼ぶ！
		m_pcDocRef->NotifyFinalSave(SAVED_FAILURE);
		throw;
	}

	//最終処理
	m_pcDocRef->NotifyFinalSave(eSaveResult);

	return eSaveResult==SAVED_OK;
}


/*! 上書き保存

	@return 保存が行われたor保存不要のため何も行わなかったときにtrueを返す

	@date 2004.06.05 genta  ビューモードのチェックをCEditDocから上書き保存処理に移動
	@date 2006.12.30 ryoji  CEditView::Command_FILESAVE()から処理本体を切り出し
	@date 2008.03.20 kobake 戻り値の仕様を定義
*/
bool CDocFileOperation::FileSave()
{
	//ファイル名が指定されていない場合は「名前を付けて保存」のフローへ遷移
	if( !m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath() ){
		return FileSaveAs();
	}

	//セーブ情報
	SSaveInfo sSaveInfo;
	m_pcDocRef->GetSaveInfo(&sSaveInfo);
	sSaveInfo.cEol = EOL_NONE; //改行コード無変換

	//上書き処理
	return m_pcDocRef->m_cDocFileOperation.DoSaveFlow( sSaveInfo );
}



/*! 名前を付けて保存フロー

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS_DIALOG()から処理本体を切り出し
*/
bool CDocFileOperation::FileSaveAs()
{
	//セーブ情報
	SSaveInfo sSaveInfo;
	m_pcDocRef->GetSaveInfo(&sSaveInfo);
	if(CAppMode::Instance()->IsViewMode())sSaveInfo.cFilePath = _T(""); //※読み込み専用モードのときはファイル名を指定しない

	//ダイアログ表示
	if(!SaveFileDialog(&sSaveInfo))return false;

	//セーブ処理
	return DoSaveFlow(sSaveInfo);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         クローズ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/* 閉じて(無題)

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
bool CDocFileOperation::FileClose()
{
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	if( !m_pcDocRef->OnFileClose() ){
		return false;
	}

	/* 既存データのクリア */
	m_pcDocRef->InitDoc();

	/* 全ビューの初期化 */
	m_pcDocRef->InitAllView();

	/* 親ウィンドウのタイトルを更新 */
	m_pcDocRef->m_pcEditWnd->UpdateCaption();


	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 閉じて開く
	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
void CDocFileOperation::FileCloseOpen( const SLoadInfo& _sLoadInfo )
{
	//閉じる
	if(!FileClose()){
		return;
	}

	//ファイル名指定が無い場合はダイアログで入力させる
	SLoadInfo sLoadInfo = _sLoadInfo;
	if( sLoadInfo.cFilePath.Length()==0 ){
		if( !OpenFileDialog( CEditWnd::Instance()->GetHwnd(), NULL, &sLoadInfo ) ){
			return;
		}
	}

	//開く
	FileLoad(&sLoadInfo);
}
