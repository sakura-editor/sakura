#include "stdafx.h"
#include "CLoadAgent.h"
#include "CControlTray.h"
#include "util/file.h"

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
			CNormalProcess::Instance()->GetProcessInstance(),
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
	if(pcDoc->m_cDocFile.IsFileExist() && !pcDoc->m_cDocFile.IsFileReadable()){
		ErrorMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("\'%ls\'\n")
			_T("というファイルを開けません。\n")
			_T("読み込みアクセス権がありません。"),
			pLoadInfo->cFilePath.c_str()
		);
		return CALLBACK_INTERRUPT; //ファイルが存在しているのに読み取れない場合は中断
	}

	return CALLBACK_CONTINUE;
}

void CLoadAgent::OnBeforeLoad(const SLoadInfo& sLoadInfo)
{
}

void CLoadAgent::OnLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* 既存データのクリア */
	pcDoc->InitDoc(); //$$

	//ファイルが存在する場合はファイルを読む
	if(fexist(sLoadInfo.cFilePath)){
		//CDocLineMgrの構成
		CReadManager cReader;
		CProgressSubject* pOld = CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&cReader);
		BOOL bReadResult = cReader.ReadFile_To_CDocLineMgr(
			&pcDoc->m_cDocLineMgr,
			sLoadInfo,
			&pcDoc->m_cDocFile.m_sFileInfo
		);
		CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(pOld);
	}

	/* レイアウト情報の変更 */
	CProgressSubject* pOld = CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&pcDoc->m_cLayoutMgr);
	pcDoc->m_cLayoutMgr.SetLayoutInfo(true, pcDoc->m_cDocType.GetDocumentAttribute());
	CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(pOld);

	//パスを確定
	pcDoc->SetFilePathAndIcon( sLoadInfo.cFilePath );
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

	// 文書種別
	CDocumentType	doctype = CShareData::getInstance()->GetDocumentType( pcDoc->m_cDocFile.GetFilePath() );
	pcDoc->m_cDocType.SetDocumentType( doctype, true );

	//再描画 $$不足
	CEditWnd::Instance()->GetActiveView().SetDrawSwitch(true);
	CEditWnd::Instance()->Views_RedrawAll(); //ビュー再描画
	InvalidateRect( CEditWnd::Instance()->GetHwnd(), NULL, TRUE );
	//m_cEditViewArr[m_nActivePaneIndex].DrawCaretPosInfo();
	CCaret& cCaret = CEditWnd::Instance()->GetActiveView().GetCaret();
	cCaret.MoveCursor(cCaret.GetCaretLayoutPos(),true);
	CEditWnd::Instance()->GetActiveView().AdjustScrollBars();
}
