#include "stdafx.h"
#include "CCodeChecker.h"
#include "io/CIoBridge.h"

#include "doc/CEditDoc.h"
#include "doc/CDocLineMgr.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     セーブ時チェック                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

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
		pcDoc->m_cDocLineMgr, pSaveInfo->eCharCode
	);

	//ユーザ問い合わせ
	if(nTmpResult==RESULT_LOSESOME){
		int nDlgResult = MYMESSAGEBOX(
			CEditWnd::Instance()->GetHwnd(),
			MB_YESNO | MB_ICONWARNING,
			GSTR_APPNAME,
			_T("文字エンコード %ts で保存しようとしていますが、\r\n")
			_T("文字コード変換により一部の文字情報が失われます。\r\n")
			_T("保存処理を続行しますか？"),
			CCodeTypeName(pSaveInfo->eCharCode).Normal()
		);
		switch(nDlgResult){
		case IDYES:		break; //続行
		case IDNO:		return CALLBACK_INTERRUPT; //中断
		case IDCANCEL:	return CALLBACK_INTERRUPT; //中断
		}
	}
	return CALLBACK_CONTINUE;
}

void CCodeChecker::OnFinalSave(ESaveResult eSaveResult)
{
	//カキコ結果
	if(eSaveResult==SAVED_LOSESOME){
		ErrorMessage(CEditWnd::Instance()->GetHwnd(), _T("一部の文字情報が、セーブ時の変換により失われました"));
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ロード時チェック                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CCodeChecker::OnFinalLoad(ELoadResult eLoadResult)
{
	if(eLoadResult==LOADED_LOSESOME){
		ErrorMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("一部の文字情報が、ロード時の変換により失われました")
		);
	}
}
