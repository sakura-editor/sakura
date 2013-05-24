#include "StdAfx.h"
#include "CCodeChecker.h"
#include "io/CIoBridge.h"
#include "charset/CCodeFactory.h" ////

#include "doc/CEditDoc.h"
#include "doc/logic/CDocLineMgr.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �Z�[�u���`�F�b�N                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! CDocLineMgr���ێ�����f�[�^�ɈقȂ���s�R�[�h�����݂��Ă��邩�ǂ������肷��
static bool _CheckSavingEolcode(const CDocLineMgr& pcDocLineMgr, CEol cEolType)
{
	bool bMix = false;
	if( cEolType == EOL_NONE ){	//���s�R�[�h�ϊ��Ȃ�
		CEol cEolCheck;	//��r�Ώ�EOL
		CDocLine* pcDocLine = pcDocLineMgr.GetDocLineTop();
		if( pcDocLine ){
			cEolCheck = pcDocLine->GetEol();
		}
		while( pcDocLine ){
			CEol cEol = pcDocLine->GetEol();
			if( cEol != cEolCheck && cEol != EOL_NONE ){
				bMix = true;
				break;
			}
			pcDocLine = pcDocLine->GetNextLine();
		}
	}
	return bMix;
}

//! CDocLineMgr���ێ�����f�[�^���w�蕶���R�[�h�ň��S�ɕۑ��ł��邩�ǂ������肷��
static EConvertResult _CheckSavingCharcode(const CDocLineMgr& pcDocLineMgr, ECodeType eCodeType)
{
	CDocLine*	pcDocLine = pcDocLineMgr.GetDocLineTop();
	CCodeBase* pCodeBase=CCodeFactory::CreateCodeBase(eCodeType,0);
	while( pcDocLine ){
		// �R�[�h�ϊ� pcDocLine -> cmemTmp
		CMemory cmemTmp;
		EConvertResult e = CIoBridge::ImplToFile(
			pcDocLine->_GetDocLineDataWithEOL(),
			&cmemTmp,
			pCodeBase
		);
		if(e!=RESULT_COMPLETE){
			delete pCodeBase;
			return e;
		}

		//���̍s��
		pcDocLine = pcDocLine->GetNextLine();
	}
	delete pCodeBase;
	return RESULT_COMPLETE;
}


ECallbackResult CCodeChecker::OnCheckSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//���s�R�[�h�����݂��Ă��邩�ǂ�������
	bool bTmpResult = false;
	if( pcDoc->m_cDocType.GetDocumentType()->m_bChkEnterAtEnd ){
		bTmpResult = _CheckSavingEolcode(
			pcDoc->m_cDocLineMgr, pSaveInfo->cEol
		);
	}

	//���[�U�₢���킹
	if (bTmpResult) {
		int nDlgResult = MYMESSAGEBOX(
			CEditWnd::getInstance()->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONWARNING,
			GSTR_APPNAME,
			_T("���s�R�[�h�����݂��Ă��܂��B\r\n")
			_T("���݂̓��͉��s�R�[�h %ts �ɓ��ꂵ�܂����H"),
			pcDoc->m_cDocEditor.GetNewLineCode().GetName()
		);
		switch(nDlgResult){
		case IDYES:		pSaveInfo->cEol = pcDoc->m_cDocEditor.GetNewLineCode(); break; //����
		case IDNO:		break; //���s
		case IDCANCEL:	return CALLBACK_INTERRUPT; //���f
		}
	}

	//�w�蕶���R�[�h�ň��S�ɕۑ��ł��邩�ǂ�������
	EConvertResult nTmpResult = _CheckSavingCharcode(
		pcDoc->m_cDocLineMgr, pSaveInfo->eCharCode
	);

	//���[�U�₢���킹
	if(nTmpResult==RESULT_LOSESOME){
		int nDlgResult = MYMESSAGEBOX(
			CEditWnd::getInstance()->GetHwnd(),
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
		ErrorMessage(CEditWnd::getInstance()->GetHwnd(), _T("�ꕔ�̕�����񂪁A�Z�[�u���̕ϊ��ɂ�莸���܂���"));
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ���[�h���`�F�b�N                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CCodeChecker::OnFinalLoad(ELoadResult eLoadResult)
{
	if(eLoadResult==LOADED_LOSESOME){
		ErrorMessage(
			CEditWnd::getInstance()->GetHwnd(),
			_T("�ꕔ�̕�����񂪁A���[�h���̕ϊ��ɂ�莸���܂���")
		);
	}
}
