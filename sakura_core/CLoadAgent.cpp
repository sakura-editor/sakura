/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "CLoadAgent.h"
#include "CReadManager.h"
#include "_main/CAppMode.h"
#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "env/CDocTypeManager.h"
#include "env/CShareData.h"
#include "doc/CEditDoc.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "uiparts/CVisualProgress.h"
#include "util/file.h"

ECallbackResult CLoadAgent::OnCheckLoad(SLoadInfo* pLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// リロード要求の場合は、継続。
	if(pLoadInfo->bRequestReload)goto next;

	//フォルダが指定された場合は「ファイルを開く」ダイアログを表示し、実際のファイル入力を促す
	if( IsDirectory(pLoadInfo->cFilePath) ){
		std::vector<std::tstring> files;
		SLoadInfo sLoadInfo(_T(""), CODE_AUTODETECT, false);
		bool bDlgResult = pcDoc->m_cDocFileOperation.OpenFileDialog(
			CEditWnd::getInstance()->GetHwnd(),
			pLoadInfo->cFilePath,	//指定されたフォルダ
			&sLoadInfo,
			files
		);
		if( !bDlgResult ){
			return CALLBACK_INTERRUPT; //キャンセルされた場合は中断
		}
		size_t nSize = files.size();
		if( 0 < nSize ){
			sLoadInfo.cFilePath = files[0].c_str();
			// 他のファイルは新規ウィンドウ
			for( size_t i = 1; i < nSize; i++ ){
				SLoadInfo sFilesLoadInfo = sLoadInfo;
				sFilesLoadInfo.cFilePath = files[i].c_str();
				CControlTray::OpenNewEditor(
					G_AppInstance(),
					CEditWnd::getInstance()->GetHwnd(),
					sFilesLoadInfo,
					NULL,
					true
				);
			}
		}
		*pLoadInfo = sLoadInfo;
	}

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
			CEditWnd::getInstance()->GetHwnd(),
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
				CEditWnd::getInstance()->GetHwnd(),
				LS(STR_NOT_EXSIST_SAVE),	//Mar. 24, 2001 jepro 若干修正
				pLoadInfo->cFilePath.GetBufferPointer()
			);
		}
	}

	// 読み取り可能チェック
	do{
		CFile cFile(pLoadInfo->cFilePath.c_str());

		//ファイルが存在しない場合はチェック省略
		if(!cFile.IsFileExist())break;

		// ロックは一時的に解除してチェックする（チェックせずに後戻りできないところまで進めるより安全）
		// ※ ロックしていてもアクセス許可の変更によって読み取れなくなっていることもある
		bool bLock = (pLoadInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) && pcDoc->m_cDocFile.IsFileLocking());
		if( bLock ) pcDoc->m_cDocFileOperation.DoFileUnlock();

		//チェック
		if(!cFile.IsFileReadable()){
			if( bLock ) pcDoc->m_cDocFileOperation.DoFileLock(false);
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				LS(STR_LOADAGENT_ERR_OPEN),
				pLoadInfo->cFilePath.c_str()
			);
			return CALLBACK_INTERRUPT; //ファイルが存在しているのに読み取れない場合は中断
		}
		if( bLock ) pcDoc->m_cDocFileOperation.DoFileLock(false);
	}
	while(false);	//	1回しか通らない. breakでここまで飛ぶ

	// ファイルサイズチェック
	if( GetDllShareData().m_Common.m_sFile.m_bAlertIfLargeFile ){
		WIN32_FIND_DATA wfd;
		HANDLE nFind = ::FindFirstFile( pLoadInfo->cFilePath.c_str(), &wfd );
		if( nFind != INVALID_HANDLE_VALUE ){
			::FindClose( nFind );
			LARGE_INTEGER nFileSize;
			nFileSize.HighPart = wfd.nFileSizeHigh;
			nFileSize.LowPart = wfd.nFileSizeLow;
			// GetDllShareData().m_Common.m_sFile.m_nAlertFileSize はMB単位
			if( (nFileSize.QuadPart>>20) >= (GetDllShareData().m_Common.m_sFile.m_nAlertFileSize) ){
				int nRet = MYMESSAGEBOX( CEditWnd::getInstance()->GetHwnd(),
					MB_ICONQUESTION | MB_YESNO | MB_TOPMOST,
					GSTR_APPNAME,
					LS(STR_LOADAGENT_BIG_FILE),
					GetDllShareData().m_Common.m_sFile.m_nAlertFileSize );
				if( nRet != IDYES ){
					return CALLBACK_INTERRUPT;
				}
			}
		}
	}

	return CALLBACK_CONTINUE;
}

void CLoadAgent::OnBeforeLoad(SLoadInfo* pLoadInfo)
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

	// 文書種別確定
	pcDoc->m_cDocType.SetDocumentType( sLoadInfo.nType, true );
	pcDoc->m_pcEditWnd->m_pcViewFontMiniMap->UpdateFont(&pcDoc->m_pcEditWnd->GetLogfont());
	InitCharWidthCache( pcDoc->m_pcEditWnd->m_pcViewFontMiniMap->GetLogfont(), CWM_FONT_MINIMAP );
	SelectCharWidthCache( CWM_FONT_EDIT, pcDoc->m_pcEditWnd->GetLogfontCacheMode() );
	InitCharWidthCache( pcDoc->m_pcEditWnd->GetLogfont() );
	pcDoc->m_pcEditWnd->m_pcViewFont->UpdateFont(&pcDoc->m_pcEditWnd->GetLogfont());

	// 起動と同時に読む場合は予めアウトライン解析画面を配置しておく
	// （ファイル読み込み開始とともにビューが表示されるので、あとで配置すると画面のちらつきが大きいの）
	if( !pcDoc->m_pcEditWnd->m_cDlgFuncList.m_bEditWndReady ){
		pcDoc->m_pcEditWnd->m_cDlgFuncList.Refresh();
		HWND hEditWnd = pcDoc->m_pcEditWnd->GetHwnd();
		if( !::IsIconic( hEditWnd ) && pcDoc->m_pcEditWnd->m_cDlgFuncList.GetHwnd() ){
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::SendMessageAny( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
	}

	//ファイルが存在する場合はファイルを読む
	if(fexist(sLoadInfo.cFilePath)){
		//CDocLineMgrの構成
		CReadManager cReader;
		CProgressSubject* pOld = CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(&cReader);
		EConvertResult eReadResult = cReader.ReadFile_To_CDocLineMgr(
			&pcDoc->m_cDocLineMgr,
			sLoadInfo,
			&pcDoc->m_cDocFile.m_sFileInfo
		);
		if(eReadResult==RESULT_LOSESOME){
			eRet = LOADED_LOSESOME;
		}
		CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(pOld);
	}
	else{
		// 存在しないときもドキュメントに文字コードを反映する
		const STypeConfig& types = pcDoc->m_cDocType.GetDocumentAttribute();
		pcDoc->m_cDocFile.SetCodeSet( sLoadInfo.eCharCode, 
			( sLoadInfo.eCharCode == types.m_encoding.m_eDefaultCodetype ) ?
				types.m_encoding.m_bDefaultBom : CCodeTypeName( sLoadInfo.eCharCode ).IsBomDefOn() );
	}

	/* レイアウト情報の変更 */
	// 2008.06.07 nasukoji	折り返し方法の追加に対応
	// 「指定桁で折り返す」以外の時は折り返し幅をMAXLINEKETASで初期化する
	// 「右端で折り返す」は、この後のOnSize()で再設定される
	const STypeConfig& ref = pcDoc->m_cDocType.GetDocumentAttribute();
	CKetaXInt nMaxLineKetas = ref.m_nMaxLineKetas;
	if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH )
		nMaxLineKetas = CKetaXInt(MAXLINEKETAS);

	CProgressSubject* pOld = CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(&pcDoc->m_cLayoutMgr);
	pcDoc->m_cLayoutMgr.SetLayoutInfo( true, true, ref, ref.m_nTabSpace, ref.m_nTsvMode, nMaxLineKetas, CLayoutXInt(-1), &pcDoc->m_pcEditWnd->GetLogfont() );
	pcDoc->m_pcEditWnd->ClearViewCaretPosInfo();
	if (pcDoc->m_cLayoutMgr.m_tsvInfo.m_nTsvMode != TSV_MODE_NONE) {
		pcDoc->m_cLayoutMgr.m_tsvInfo.CalcTabLength(pcDoc->m_cLayoutMgr.m_pcDocLineMgr);
	}
	
	CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(pOld);

	return eRet;
}

void CLoadAgent::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* 親ウィンドウのタイトルを更新 */
	pcDoc->m_pcEditWnd->UpdateCaption();

	// -- -- ※ InitAllViewでやってたこと -- -- //	// 2009.08.28 nasukoji	CEditView::OnAfterLoad()からここに移動
	pcDoc->m_nCommandExecNum=0;

	// テキストの折り返し方法を初期化
	pcDoc->m_nTextWrapMethodCur = pcDoc->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;	// 折り返し方法
	pcDoc->m_bTextWrapMethodCurTemp = false;													// 一時設定適用中を解除
	pcDoc->m_blfCurTemp = false;
	pcDoc->m_bTabSpaceCurTemp = false;

	// 2009.08.28 nasukoji	「折り返さない」ならテキスト最大幅を算出、それ以外は変数をクリア
	if( pcDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		pcDoc->m_cLayoutMgr.CalculateTextWidth();		// テキスト最大幅を算出する
	else
		pcDoc->m_cLayoutMgr.ClearLayoutLineWidth();		// 各行のレイアウト行長の記憶をクリアする
}


void CLoadAgent::OnFinalLoad(ELoadResult eLoadResult)
{
	CEditDoc* pcDoc = GetListeningDoc();

	if(eLoadResult==LOADED_FAILURE){
		pcDoc->SetFilePathAndIcon( _T("") );
		pcDoc->m_cDocFile.SetBomDefoult();
	}
	if(eLoadResult==LOADED_LOSESOME){
		CAppMode::getInstance()->SetViewMode(true);
	}

	//再描画 $$不足
	// CEditWnd::getInstance()->GetActiveView().SetDrawSwitch(true);
	bool bDraw = CEditWnd::getInstance()->GetActiveView().GetDrawSwitch();
	if( bDraw ){
		CEditWnd::getInstance()->Views_RedrawAll(); //ビュー再描画
		InvalidateRect( CEditWnd::getInstance()->GetHwnd(), NULL, TRUE );
	}
	CCaret& cCaret = CEditWnd::getInstance()->GetActiveView().GetCaret();
	cCaret.MoveCursor(cCaret.GetCaretLayoutPos(),true);
	CEditWnd::getInstance()->GetActiveView().AdjustScrollBars();
}
