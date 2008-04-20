#include "stdafx.h"
#include "CLoadAgent.h"
#include "CControlTray.h"
#include "util/file.h"

ECallbackResult CLoadAgent::OnCheckLoad(SLoadInfo* pLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// �����[�h�v���̏ꍇ�́A�p���B
	if(pLoadInfo->bRequestReload)goto next;

	// ���̃E�B���h�E�Ŋ��ɊJ����Ă���ꍇ�́A������A�N�e�B�u�ɂ���
	HWND	hWndOwner;
	if( CShareData::getInstance()->ActiveAlreadyOpenedWindow(pLoadInfo->cFilePath, &hWndOwner, pLoadInfo->eCharCode) ){
		pLoadInfo->bOpened = true;
		return CALLBACK_INTERRUPT;
	}

	// ���݂̃E�B���h�E�ɑ΂��ăt�@�C����ǂݍ��߂Ȃ��ꍇ�́A�V���ȃE�B���h�E���J���A�����Ƀt�@�C����ǂݍ��܂���
	if(!pcDoc->IsAcceptLoad()){
		CControlTray::OpenNewEditor(
			CNormalProcess::Instance()->GetProcessInstance(),
			CEditWnd::Instance()->GetHwnd(),
			*pLoadInfo
		);
		return CALLBACK_INTERRUPT;
	}

next:
	// �I�v�V�����F�J�����Ƃ����t�@�C�������݂��Ȃ��Ƃ��x������
	if( GetDllShareData().m_Common.m_sFile.GetAlertIfFileNotExist() ){
		if(!fexist(pLoadInfo->cFilePath)){
			InfoBeep();
			//	Feb. 15, 2003 genta Popup�E�B���h�E��\�����Ȃ��悤�ɁD
			//	�����ŃX�e�[�^�X���b�Z�[�W���g���Ă���ʂɕ\������Ȃ��D
			TopInfoMessage(
				CEditWnd::Instance()->GetHwnd(),
				_T("%ts\n�Ƃ����t�@�C���͑��݂��܂���B\n\n�t�@�C����ۑ������Ƃ��ɁA�f�B�X�N��ɂ��̃t�@�C�����쐬����܂��B"),	//Mar. 24, 2001 jepro �኱�C��
				pLoadInfo->cFilePath.GetBufferPointer()
			);
		}
	}

	// �ǂݎ��\�`�F�b�N
	if(pcDoc->m_cDocFile.IsFileExist() && !pcDoc->m_cDocFile.IsFileReadable()){
		ErrorMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("\'%ls\'\n")
			_T("�Ƃ����t�@�C�����J���܂���B\n")
			_T("�ǂݍ��݃A�N�Z�X��������܂���B"),
			pLoadInfo->cFilePath.c_str()
		);
		return CALLBACK_INTERRUPT; //�t�@�C�������݂��Ă���̂ɓǂݎ��Ȃ��ꍇ�͒��f
	}

	return CALLBACK_CONTINUE;
}

void CLoadAgent::OnBeforeLoad(const SLoadInfo& sLoadInfo)
{
}

void CLoadAgent::OnLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* �����f�[�^�̃N���A */
	pcDoc->InitDoc(); //$$

	//�t�@�C�������݂���ꍇ�̓t�@�C����ǂ�
	if(fexist(sLoadInfo.cFilePath)){
		//CDocLineMgr�̍\��
		CReadManager cReader;
		CProgressSubject* pOld = CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&cReader);
		BOOL bReadResult = cReader.ReadFile_To_CDocLineMgr(
			&pcDoc->m_cDocLineMgr,
			sLoadInfo,
			&pcDoc->m_cDocFile.m_sFileInfo
		);
		CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(pOld);
	}

	/* ���C�A�E�g���̕ύX */
	CProgressSubject* pOld = CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(&pcDoc->m_cLayoutMgr);
	pcDoc->m_cLayoutMgr.SetLayoutInfo(true, pcDoc->m_cDocType.GetDocumentAttribute());
	CEditApp::Instance()->m_pcVisualProgress->CProgressListener::Listen(pOld);

	//�p�X���m��
	pcDoc->SetFilePathAndIcon( sLoadInfo.cFilePath );
}

void CLoadAgent::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
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

	// �������
	CDocumentType	doctype = CShareData::getInstance()->GetDocumentType( pcDoc->m_cDocFile.GetFilePath() );
	pcDoc->m_cDocType.SetDocumentType( doctype, true );

	//�ĕ`�� $$�s��
	CEditWnd::Instance()->GetActiveView().SetDrawSwitch(true);
	CEditWnd::Instance()->Views_RedrawAll(); //�r���[�ĕ`��
	InvalidateRect( CEditWnd::Instance()->GetHwnd(), NULL, TRUE );
	//m_cEditViewArr[m_nActivePaneIndex].DrawCaretPosInfo();
	CCaret& cCaret = CEditWnd::Instance()->GetActiveView().GetCaret();
	cCaret.MoveCursor(cCaret.GetCaretLayoutPos(),true);
	CEditWnd::Instance()->GetActiveView().AdjustScrollBars();
}
