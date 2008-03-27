#include "stdafx.h"
#include "CCodeChecker.h"
#include "io/CIoBridge.h"

//! CDocLineMgrが保持するデータを指定文字コードで安全に保存できるかどうか判定する
static EConvertResult _CheckSavingCharcode(const CDocLineMgr& pcDocLineMgr, ECodeType eCodeType)
{
	CDocLine*	pcDocLine = pcDocLineMgr.GetDocLineTop();
	while( pcDocLine ){
		// コード変換 pcDocLine -> cmemTmp
		CMemory cmemTmp;
		EConvertResult e = CIoBridge::ImplToFile(
			pcDocLine->_GetDocLineDataWithEOL(),
			&cmemTmp,
			eCodeType
		);
		if(e!=RESULT_COMPLETE){
			return e;
		}

		//次の行へ
		pcDocLine = pcDocLine->GetNextLine();
	}
	return RESULT_COMPLETE;
}


ECallbackResult CCodeChecker::OnCheckSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//指定文字コードで安全に保存できるかどうか判定
	EConvertResult nTmpResult = _CheckSavingCharcode(
		pcDoc->m_cDocLineMgr, pcDoc->m_cDocFile.m_sFileInfo.eCharCode
	);

	//ユーザ問い合わせ
	if(nTmpResult!=RESULT_COMPLETE){
		int nDlgResult = MYMESSAGEBOX(
			CEditWnd::Instance()->GetHwnd(),
			MB_YESNO | MB_ICONWARNING,
			GSTR_APPNAME,
			_T("文字コード変換により一部の文字情報が失われます。\n")
			_T("保存処理を続行しますか？")
		);
		switch(nDlgResult){
		case IDYES:		break; //続行
		case IDNO:		return CALLBACK_INTERRUPT; //中断
		case IDCANCEL:	return CALLBACK_INTERRUPT; //中断
		}
	}
	return CALLBACK_CONTINUE;
}

void CCodeChecker::OnFinishSave(ESaveResult eSaveResult)
{
	//カキコ結果
	if(eSaveResult==SAVED_LOSESOME){
		ErrorMessage(CEditWnd::Instance()->GetHwnd(), _T("一部の文字情報が、変換により失われました"));
	}
}

