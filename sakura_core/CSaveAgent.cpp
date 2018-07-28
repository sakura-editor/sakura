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
#include "doc/CDocListener.h" // 親クラス
#include "CSaveAgent.h"
#include "doc/CEditDoc.h"
#include "doc/CDocVisitor.h"
#include "window/CEditWnd.h"
#include "uiparts/CVisualProgress.h"
#include "uiparts/CWaitCursor.h"
#include "CWriteManager.h"
#include "io/CBinaryStream.h"
#include "CEditApp.h"
#include "_main/CAppMode.h"
#include "env/CShareData.h"

CSaveAgent::CSaveAgent()
{
}


ECallbackResult CSaveAgent::OnCheckSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//	Jun.  5, 2004 genta
	//	ビューモードのチェックをCEditDocから上書き保存処理に移動
	//	同名で上書きされるのを防ぐ
	if( CAppMode::getInstance()->IsViewMode() && pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) ){
		ErrorBeep();
		TopErrorMessage( CEditWnd::getInstance()->GetHwnd(), LS(STR_SAVEAGENT_VIEW_FILE) );
		return CALLBACK_INTERRUPT;
	}

	// 他ウィンドウで開いているか確認する	// 2009.04.07 ryoji
	if( !pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) ){
		HWND hwndOwner;
		if( CShareData::getInstance()->IsPathOpened( pSaveInfo->cFilePath, &hwndOwner ) ){
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				LS(STR_SAVEAGENT_OTHER),
				(LPCTSTR)pSaveInfo->cFilePath
			);
			return CALLBACK_INTERRUPT;
		}
	}

	// 書込可能チェック ######### スマートじゃない。ホントは書き込み時エラーチェック検出機構を用意したい
	{
		// ロックは一時的に解除してチェックする（チェックせずに後戻りできないところまで進めるより安全）
		// ※ ロックしていてもファイル属性やアクセス許可の変更によって書き込めなくなっていることもある
		bool bLock = (pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) && pcDoc->m_cDocFile.IsFileLocking());
		if( bLock ) pcDoc->m_cDocFileOperation.DoFileUnlock();
		try{
			bool bExist = fexist(pSaveInfo->cFilePath);
			CStream out(pSaveInfo->cFilePath, _T("ab"), true);	// 実際の保存は "wb" だがここは "ab"（ファイル内容は破棄しない）でチェックする	// 2009.08.21 ryoji
			out.Close();
			if(!bExist){
				::DeleteFile(pSaveInfo->cFilePath);
			}
		}
		catch(CError_FileOpen){
			// ※ たとえ上書き保存の場合でもここでの失敗では書込み禁止へは遷移しない
			if( bLock ) pcDoc->m_cDocFileOperation.DoFileLock(false);
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				LS(STR_SAVEAGENT_OTHER_APP),
				pSaveInfo->cFilePath.c_str()
			);
			return CALLBACK_INTERRUPT;
		}
		if( bLock ) pcDoc->m_cDocFileOperation.DoFileLock(false);
	}

	return CALLBACK_CONTINUE;
}



void CSaveAgent::OnBeforeSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//改行コード統一
	CDocVisitor(pcDoc).SetAllEol(sSaveInfo.cEol);
}

void CSaveAgent::OnSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//カキコ
	CWriteManager cWriter;
	CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(&cWriter);
	cWriter.WriteFile_From_CDocLineMgr(
		pcDoc->m_cDocLineMgr,
		sSaveInfo
	);

	//セーブ情報の確定
	pcDoc->SetFilePathAndIcon( sSaveInfo.cFilePath );
	pcDoc->m_cDocFile.SetCodeSet( sSaveInfo.eCharCode, sSaveInfo.bBomExist );
	if(sSaveInfo.cEol.IsValid()){
		pcDoc->m_cDocEditor.SetNewLineCode(sSaveInfo.cEol);
	}
}

void CSaveAgent::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* 更新後のファイル時刻の取得
	 * CloseHandle前ではFlushFileBuffersを呼んでもタイムスタンプが更新
	 * されないことがある。
	 */
	GetLastWriteTimestamp( pcDoc->m_cDocFile.GetFilePath(), &pcDoc->m_cDocFile.GetFileTime() );

	// タイプ別設定の変更を指示。
	// 上書き（明示的な上書きや自動保存）では変更しない
	// ---> 上書きの場合は一時的な折り返し桁変更やタブ幅変更を維持したままにする
	if(!sSaveInfo.bOverwriteMode){
		pcDoc->OnChangeSetting();
	}
}



void CSaveAgent::OnFinalSave(ESaveResult eSaveResult)
{
}
