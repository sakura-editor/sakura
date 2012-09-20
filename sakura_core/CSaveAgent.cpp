#include "StdAfx.h"
#include "doc/CDocListener.h" // �e�N���X
#include "CSaveAgent.h"
#include "doc/CEditDoc.h"
#include "doc/CDocVisitor.h"
#include "window/CEditWnd.h"
#include "CVisualProgress.h"
#include "CWriteManager.h"
#include "CWaitCursor.h"
#include "io/CBinaryStream.h"
#include "CEditApp.h"
#include "CAppMode.h"
#include "env/CShareData.h"

CSaveAgent::CSaveAgent()
{
}


ECallbackResult CSaveAgent::OnCheckSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//	Jun.  5, 2004 genta
	//	�r���[���[�h�̃`�F�b�N��CEditDoc����㏑���ۑ������Ɉړ�
	//	�����ŏ㏑�������̂�h��
	if( CAppMode::getInstance()->IsViewMode() && pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) ){
		ErrorBeep();
		TopErrorMessage( CEditWnd::getInstance()->GetHwnd(), _T("�r���[���[�h�ł͓���t�@�C���ւ̕ۑ��͂ł��܂���B") );
		return CALLBACK_INTERRUPT;
	}

	// ���E�B���h�E�ŊJ���Ă��邩�m�F����	// 2009.04.07 ryoji
	if( !pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) ){
		HWND hwndOwner;
		if( CShareData::getInstance()->IsPathOpened( pSaveInfo->cFilePath, &hwndOwner ) ){
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				_T("\'%ts\'\n")
				_T("�t�@�C����ۑ��ł��܂���B\n")
				_T("���̃E�B���h�E�Ŏg�p���ł��B"),
				(LPCTSTR)pSaveInfo->cFilePath
			);
			return CALLBACK_INTERRUPT;
		}
	}

	// �����\�`�F�b�N ######### �X�}�[�g����Ȃ��B�z���g�͏������ݎ��G���[�`�F�b�N���o�@�\��p�ӂ�����
	{
		// ���b�N�͈ꎞ�I�ɉ������ă`�F�b�N����i�`�F�b�N�����Ɍ�߂�ł��Ȃ��Ƃ���܂Ői�߂�����S�j
		// �� ���b�N���Ă��Ă��t�@�C��������A�N�Z�X���̕ύX�ɂ���ď������߂Ȃ��Ȃ��Ă��邱�Ƃ�����
		bool bLock = (pSaveInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) && pcDoc->m_cDocFile.IsFileLocking());
		if( bLock ) pcDoc->m_cDocFileOperation.DoFileUnlock();
		try{
			bool bExist = fexist(pSaveInfo->cFilePath);
			CStream out(pSaveInfo->cFilePath, _T("ab"), true);	// ���ۂ̕ۑ��� "wb" ���������� "ab"�i�t�@�C�����e�͔j�����Ȃ��j�Ń`�F�b�N����	// 2009.08.21 ryoji
			out.Close();
			if(!bExist){
				::DeleteFile(pSaveInfo->cFilePath);
			}
		}
		catch(CError_FileOpen){
			// �� ���Ƃ��㏑���ۑ��̏ꍇ�ł������ł̎��s�ł͏����݋֎~�ւ͑J�ڂ��Ȃ�
			if( bLock ) pcDoc->m_cDocFileOperation.DoFileLock(false);
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				_T("\'%ts\'\n")
				_T("�t�@�C����ۑ��ł��܂���B\n")
				_T("�p�X�����݂��Ȃ����A���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B"),
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

	//���s�R�[�h����
	CDocVisitor(pcDoc).SetAllEol(sSaveInfo.cEol);
}

void CSaveAgent::OnSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//�J�L�R
	CWriteManager cWriter;
	CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(&cWriter);
	EConvertResult eSaveResult = cWriter.WriteFile_From_CDocLineMgr(
		pcDoc->m_cDocLineMgr,
		sSaveInfo
	);

	//�Z�[�u���̊m��
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

	/* �X�V��̃t�@�C�������̎擾
	 * CloseHandle�O�ł�FlushFileBuffers���Ă�ł��^�C���X�^���v���X�V
	 * ����Ȃ����Ƃ�����B
	 */
	GetLastWriteTimestamp( pcDoc->m_cDocFile.GetFilePath(), &pcDoc->m_cDocFile.m_sFileInfo.cFileTime );

	// �^�C�v�ʐݒ�̕ύX���w���B
	// �㏑���i�����I�ȏ㏑���⎩���ۑ��j�ł͕ύX���Ȃ�
	// ---> �㏑���̏ꍇ�͈ꎞ�I�Ȑ܂�Ԃ����ύX��^�u���ύX���ێ������܂܂ɂ���
	if(!sSaveInfo.bOverwriteMode){
		pcDoc->OnChangeSetting();
	}
}



void CSaveAgent::OnFinalSave(ESaveResult eSaveResult)
{
}
