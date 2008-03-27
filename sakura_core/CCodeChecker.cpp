#include "stdafx.h"
#include "CCodeChecker.h"
#include "io/CIoBridge.h"

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
		pcDoc->m_cDocLineMgr, pcDoc->m_cDocFile.m_sFileInfo.eCharCode
	);

	//���[�U�₢���킹
	if(nTmpResult!=RESULT_COMPLETE){
		int nDlgResult = MYMESSAGEBOX(
			CEditWnd::Instance()->GetHwnd(),
			MB_YESNO | MB_ICONWARNING,
			GSTR_APPNAME,
			_T("�����R�[�h�ϊ��ɂ��ꕔ�̕�����񂪎����܂��B\n")
			_T("�ۑ������𑱍s���܂����H")
		);
		switch(nDlgResult){
		case IDYES:		break; //���s
		case IDNO:		return CALLBACK_INTERRUPT; //���f
		case IDCANCEL:	return CALLBACK_INTERRUPT; //���f
		}
	}
	return CALLBACK_CONTINUE;
}

void CCodeChecker::OnFinishSave(ESaveResult eSaveResult)
{
	//�J�L�R����
	if(eSaveResult==SAVED_LOSESOME){
		ErrorMessage(CEditWnd::Instance()->GetHwnd(), _T("�ꕔ�̕�����񂪁A�ϊ��ɂ�莸���܂���"));
	}
}

