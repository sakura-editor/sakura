#include "stdafx.h"
#include "CSaveAgent.h"
#include "doc/CDocVisitor.h"
#include "CWaitCursor.h"
#include "io/CBinaryStream.h"

CSaveAgent::CSaveAgent()
{
}


ECallbackResult CSaveAgent::OnCheckSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//	Jun.  5, 2004 genta
	//	ビューモードのチェックをCEditDocから上書き保存処理に移動
	//	同名で上書きされるのを防ぐ
	if( CAppMode::Instance()->IsViewMode() && pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) ){
		ErrorBeep();
		TopErrorMessage( CEditWnd::Instance()->GetHwnd(), _T("ビューモードでは同一ファイルへの保存はできません。") );
		return CALLBACK_INTERRUPT;
	}

	//オプション：無変更でも上書きするか
	if( !GetDllShareData().m_Common.m_sFile.m_bEnableUnmodifiedOverwrite ){
		// 上書きの場合
		if(pSaveInfo->bOverwriteMode){
			// 無変更の場合は警告音を出し、終了
			if(!pcDoc->m_cDocEditor.IsModified() && pSaveInfo->cEol==EOL_NONE){ //※改行コード指定保存がリクエストされた場合は、「変更があったもの」とみなす
				CEditApp::Instance()->m_cSoundSet.NeedlessToSaveBeep();
				return CALLBACK_INTERRUPT;
			}
		}
	}

	// 書込可能チェック ######### スマートじゃない。ホントは書き込み時エラーチェック検出機構を用意したい
	if(!pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) || !pcDoc->m_cDocFileOperation._ToDoLock()){ //名前を付けて保存 or ロックしてない
		CFile cFile;
		cFile.SetFilePath(pSaveInfo->cFilePath);
		try{
			if(fexist(pSaveInfo->cFilePath)){
				if(!cFile.IsFileWritable()){
					throw CError_FileOpen();
				}
			}
			else{
				CBinaryOutputStream out(pSaveInfo->cFilePath);
				out.Close();
				::DeleteFile(pSaveInfo->cFilePath);
			}
		}
		catch(CError_FileOpen){
			ErrorMessage(
				CEditWnd::Instance()->GetHwnd(),
				_T("\'%ts\'\n")
				_T("ファイルを保存できません。\n")
				_T("パスが存在しないか、他のアプリケーションで使用されている可能性があります。"),
				pSaveInfo->cFilePath.c_str()
			);
			return CALLBACK_INTERRUPT;
		}
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
	CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&cWriter);
	EConvertResult eSaveResult = cWriter.WriteFile_From_CDocLineMgr(
		pcDoc->m_cDocLineMgr,
		sSaveInfo
	);

	//セーブ情報の確定
	pcDoc->SetFilePathAndIcon( sSaveInfo.cFilePath );
	pcDoc->m_cDocFile.m_sFileInfo.eCharCode = sSaveInfo.eCharCode;
	pcDoc->m_cDocFile.m_sFileInfo.bBomExist = sSaveInfo.bBomExist;
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
	GetLastWriteTimestamp( pcDoc->m_cDocFile.GetFilePath(), &pcDoc->m_cDocFile.m_sFileInfo.cFileTime );

	// タイプ別設定の変更を指示。
	pcDoc->OnChangeSetting();
}



void CSaveAgent::OnFinalSave(ESaveResult eSaveResult)
{
}
