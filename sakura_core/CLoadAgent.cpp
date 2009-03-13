#include "stdafx.h"
#include "CLoadAgent.h"
#include "CControlTray.h"
#include "util/file.h"
#include "CAppMode.h"

ECallbackResult CLoadAgent::OnCheckLoad(SLoadInfo* pLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// リロード要求の場合は、継続。
	if(pLoadInfo->bRequestReload)goto next;

	// 他のウィンドウで既に開かれている場合は、それをアクティブにする
	HWND	hWndOwner;
	if( CShareData::getInstance()->ActiveAlreadyOpenedWindow(pLoadInfo->cFilePath, &hWndOwner, pLoadInfo->eCharCode) ){
		pLoadInfo->bOpened = true;
		return CALLBACK_INTERRUPT;
	}

	// 現在のウィンドウに対してファイルを読み込めない場合は、新たなウィンドウを開き、そこにファイルを読み込ませる
	if(!pcDoc->IsAcceptLoad()){
		CControlTray::OpenNewEditor(
			G_AppInstance(),
			CEditWnd::Instance()->GetHwnd(),
			*pLoadInfo
		);
		return CALLBACK_INTERRUPT;
	}

next:
	// オプション：開こうとしたファイルが存在しないとき警告する
	if( GetDllShareData().m_Common.m_sFile.GetAlertIfFileNotExist() ){
		if(!fexist(pLoadInfo->cFilePath)){
			InfoBeep();
			//	Feb. 15, 2003 genta Popupウィンドウを表示しないように．
			//	ここでステータスメッセージを使っても画面に表示されない．
			TopInfoMessage(
				CEditWnd::Instance()->GetHwnd(),
				_T("%ts\nというファイルは存在しません。\n\nファイルを保存したときに、ディスク上にこのファイルが作成されます。"),	//Mar. 24, 2001 jepro 若干修正
				pLoadInfo->cFilePath.GetBufferPointer()
			);
		}
	}

	// 読み取り可能チェック
	do{
		CFile cFile(pLoadInfo->cFilePath.c_str());

		//ファイルが存在しない場合はチェック省略
		if(!cFile.IsFileExist())break;

		//ロックしている場合はチェック省略
		if(pLoadInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) && pcDoc->m_cDocFile.IsFileLocking())break;

		//チェック
		if(!cFile.IsFileReadable()){
			ErrorMessage(
				CEditWnd::Instance()->GetHwnd(),
				_T("\'%ls\'\n")
				_T("というファイルを開けません。\n")
				_T("読み込みアクセス権がありません。"),
				pLoadInfo->cFilePath.c_str()
			);
			return CALLBACK_INTERRUPT; //ファイルが存在しているのに読み取れない場合は中断
		}
	}
	while(false);

	return CALLBACK_CONTINUE;
}

void CLoadAgent::OnBeforeLoad(const SLoadInfo& sLoadInfo)
{
}

ELoadResult CLoadAgent::OnLoad(const SLoadInfo& sLoadInfo)
{
	ELoadResult eRet = LOADED_OK;
	CEditDoc* pcDoc = GetListeningDoc();

	/* 既存データのクリア */
	pcDoc->InitDoc(); //$$

	// パスを確定
	pcDoc->SetFilePathAndIcon( sLoadInfo.cFilePath );

	//ファイルが存在する場合はファイルを読む
	if(fexist(sLoadInfo.cFilePath)){
		//CDocLineMgrの構成
		CReadManager cReader;
		CProgressSubject* pOld = CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&cReader);
		EConvertResult eReadResult = cReader.ReadFile_To_CDocLineMgr(
			&pcDoc->m_cDocLineMgr,
			sLoadInfo,
			&pcDoc->m_cDocFile.m_sFileInfo
		);
		if(eReadResult==RESULT_LOSESOME){
			eRet = LOADED_LOSESOME;
		}
		CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(pOld);
	}

	// 文書種別
	pcDoc->m_cDocType.SetDocumentType( CDocTypeManager().GetDocumentTypeOfPath( sLoadInfo.cFilePath ), true );

	/* レイアウト情報の変更 */
	// 2008.06.07 nasukoji	折り返し方法の追加に対応
	// 「指定桁で折り返す」以外の時は折り返し幅をMAXLINEKETASで初期化する
	// 「右端で折り返す」は、この後のOnSize()で再設定される
	STypeConfig ref = pcDoc->m_cDocType.GetDocumentAttribute();
	if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH )
		ref.m_nMaxLineKetas = MAXLINEKETAS;

	CProgressSubject* pOld = CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&pcDoc->m_cLayoutMgr);
	pcDoc->m_cLayoutMgr.SetLayoutInfo(true, ref);
	CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(pOld);

	return eRet;
}

void CLoadAgent::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* 親ウィンドウのタイトルを更新 */
	pcDoc->m_pcEditWnd->UpdateCaption();
}


void CLoadAgent::OnFinalLoad(ELoadResult eLoadResult)
{
	CEditDoc* pcDoc = GetListeningDoc();

	if(eLoadResult==LOADED_FAILURE){
		pcDoc->SetFilePathAndIcon( _T("") );
		pcDoc->m_cDocFile.m_sFileInfo.bBomExist = false;
		if(pcDoc->m_cDocFile.m_sFileInfo.eCharCode==CODE_UNICODE || pcDoc->m_cDocFile.m_sFileInfo.eCharCode==CODE_UNICODEBE)pcDoc->m_cDocFile.m_sFileInfo.bBomExist = true;
	}
	if(eLoadResult==LOADED_LOSESOME){
		CAppMode::Instance()->SetViewMode(true);
	}

	//再描画 $$不足
	CEditWnd::Instance()->GetActiveView().SetDrawSwitch(true);
	CEditWnd::Instance()->Views_RedrawAll(); //ビュー再描画
	InvalidateRect( CEditWnd::Instance()->GetHwnd(), NULL, TRUE );
	//m_cEditViewArr[m_nActivePaneIndex].DrawCaretPosInfo();
	CCaret& cCaret = CEditWnd::Instance()->GetActiveView().GetCaret();
	cCaret.MoveCursor(cCaret.GetCaretLayoutPos(),true);
	CEditWnd::Instance()->GetActiveView().AdjustScrollBars();
}
