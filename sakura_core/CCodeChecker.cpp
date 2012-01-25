#include "StdAfx.h"
#include "CCodeChecker.h"
#include "io/CIoBridge.h"

#include "doc/CEditDoc.h"
#include "doc/CDocLineMgr.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �Z�[�u���`�F�b�N                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! CDocLineMgr���ێ�����f�[�^���w�蕶���R�[�h�ň��S�ɕۑ��ł��邩�ǂ������肷��
static EConvertResult _CheckSavingCharcode(const CDocLineMgr& pcDocLineMgr, ECodeType eCodeType)
{
	CDocLine*	pcDocLine = pcDocLineMgr.GetDocLineTop();
	while( pcDocLine ){
		// �R�[�h�ϊ� pcDocLine -> cmemTmp
		CMemory cmemTmp;
		EConvertResult e = CIoBridge::ImplToFile(
			pcDocLine->_GetDocLineDataWithEOL(),
			&cmemTmp,
			eCodeType
		);
		if(e!=RESULT_COMPLETE){
			return e;
		}

		//���̍s��
		pcDocLine = pcDocLine->GetNextLine();
	}
	return RESULT_COMPLETE;
}


ECallbackResult CCodeChecker::OnCheckSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//�w�蕶���R�[�h�ň��S�ɕۑ��ł��邩�ǂ�������
	EConvertResult nTmpResult = _CheckSavingCharcode(
		pcDoc->m_cDocLineMgr, pSaveInfo->eCharCode
	);

	//���[�U�₢���킹
	if(nTmpResult==RESULT_LOSESOME){
		int nDlgResult = MYMESSAGEBOX(
			CEditWnd::Instance()->GetHwnd(),
			MB_YESNO | MB_ICONWARNING,
			GSTR_APPNAME,
			_T("�����G���R�[�h %ts �ŕۑ����悤�Ƃ��Ă��܂����A\r\n")
			_T("�����R�[�h�ϊ��ɂ��ꕔ�̕�����񂪎����܂��B\r\n")
			_T("�ۑ������𑱍s���܂����H"),
			CCodeTypeName(pSaveInfo->eCharCode).Normal()
		);
		switch(nDlgResult){
		case IDYES:		break; //���s
		case IDNO:		return CALLBACK_INTERRUPT; //���f
		case IDCANCEL:	return CALLBACK_INTERRUPT; //���f
		}
	}
	return CALLBACK_CONTINUE;
}

void CCodeChecker::OnFinalSave(ESaveResult eSaveResult)
{
	//�J�L�R����
	if(eSaveResult==SAVED_LOSESOME){
		ErrorMessage(CEditWnd::Instance()->GetHwnd(), _T("�ꕔ�̕�����񂪁A�Z�[�u���̕ϊ��ɂ�莸���܂���"));
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ���[�h���`�F�b�N                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CCodeChecker::OnFinalLoad(ELoadResult eLoadResult)
{
	if(eLoadResult==LOADED_LOSESOME){
		ErrorMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("�ꕔ�̕�����񂪁A���[�h���̕ϊ��ɂ�莸���܂���")
		);
	}
}
