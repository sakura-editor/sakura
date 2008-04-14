#include "stdafx.h"
#include "CDocFileOperation.h"
#include "CWaitCursor.h"
#include "CControlTray.h"
#include "util/window.h"
#include "CSakuraEnvironment.h"
#include "doc/CDocVisitor.h"


bool CDocFileOperation::_ToDoLock() const
{
	// �t�@�C�����J���Ă��Ȃ�
	if( !m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath() )return false;

	// �r���[���[�h
	if( CAppMode::Instance()->IsViewMode() )return false;
	
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ���[�hUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �u�t�@�C�����J���v�_�C�A���O */
//	Mar. 30, 2003 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
bool CDocFileOperation::OpenFileDialog(
	HWND				hwndParent,		//!< [in]
	const TCHAR*		pszOpenFolder,	//!< [in]     NULL�ȊO���w�肷��Ə����t�H���_���w��ł���
	SLoadInfo*			pLoadInfo		//!< [in/out] ���[�h���
)
{
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndParent );

	// �t�@�C���I�[�v���_�C�A���O��\��
	m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.Create(
		CNormalProcess::Instance()->GetProcessInstance(),
		hwndParent,
		_T("*.*"),
		pszOpenFolder ? pszOpenFolder : CSakuraEnvironment::GetDlgInitialDir().c_str(),	// �����t�H���_
		CMRU().GetPathList(),															// MRU���X�g�̃t�@�C���̃��X�g
		CMRUFolder().GetPathList()														// OPENFOLDER���X�g�̃t�@�C���̃��X�g
	);
	return m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.DoModalOpenDlg( pLoadInfo );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ���[�h�t���[                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDocFileOperation::DoLoadFlow(SLoadInfo* pLoadInfo)
{
	ELoadResult eLoadResult = LOADED_FAILURE;

	try{
		//�Z�[�u�O�`�F�b�N
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyCheckLoad(pLoadInfo))throw CFlowInterruption();

		//�Z�[�u����
		m_pcDocRef->NotifyBeforeLoad(*pLoadInfo);	//�O����
		m_pcDocRef->NotifyLoad(*pLoadInfo);			//�{����
		m_pcDocRef->NotifyAfterLoad(*pLoadInfo);		//�㏈��

		//����
		eLoadResult = LOADED_OK; //###��
	}
	catch(CFlowInterruption){
		eLoadResult = LOADED_INTERRUPT;
	}
	catch(...){
		//�\�����ʗ�O�����������ꍇ�� NotifyFinalLoad �͕K���ĂԁI
		m_pcDocRef->NotifyFinalLoad(LOADED_FAILURE);
		throw;
	}

	//�ŏI����
	m_pcDocRef->NotifyFinalLoad(eLoadResult);

	return eLoadResult==LOADED_OK;
}

//! �t�@�C�����J��
bool CDocFileOperation::FileLoad(
	SLoadInfo*	pLoadInfo		//!< [in/out]
)
{
	return DoLoadFlow(pLoadInfo);
}

//! ����t�@�C���̍ăI�[�v��
void CDocFileOperation::ReloadCurrentFile(
	ECodeType	nCharCode		//!< [in] �����R�[�h���
)
{
	bool bViewMode = CAppMode::Instance()->IsViewMode();	// �r���[���[�h

	if( !fexist(m_pcDocRef->m_cDocFile.GetFilePath()) ){
		/* �t�@�C�������݂��Ȃ� */
		//	Jul. 26, 2003 ryoji BOM��W���ݒ��
		m_pcDocRef->m_cDocFile.m_sFileInfo.eCharCode = nCharCode;
		switch( m_pcDocRef->m_cDocFile.m_sFileInfo.eCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_pcDocRef->m_cDocFile.m_sFileInfo.bBomExist = true;
			break;
		case CODE_UTF8:
		default:
			m_pcDocRef->m_cDocFile.m_sFileInfo.bBomExist = false;
			break;
		}
		return;
	}

	//�J�[�\���ʒu�ۑ�
	CLayoutInt		nViewTopLine = m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	/* �\����̈�ԏ�̍s(0�J�n) */
	CLayoutInt		nViewLeftCol = m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	/* �\����̈�ԍ��̌�(0�J�n) */
	CLayoutPoint	ptCaretPosXY = m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos();

	//�����R�[�h�m�F�t���O�I�t
	bool backup_bQueryIfCodeChange = GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange; //####��
	GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange = false; //####��

	//���[�h
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath=m_pcDocRef->m_cDocFile.GetFilePath();
	sLoadInfo.eCharCode=nCharCode;
	sLoadInfo.bViewMode=bViewMode;
	this->DoLoadFlow(&sLoadInfo);

	//�����R�[�h�m�F�t���O����
	GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange = backup_bQueryIfCodeChange; //####��

	// �J�[�\���ʒu���� (�������ł̓I�v�V�����̃J�[�\���ʒu�����i�����s�P�ʁj���w�肳��Ă��Ȃ��ꍇ�ł���������)
	// 2007.08.23 ryoji �\���̈敜��
	if( ptCaretPosXY.GetY2() < m_pcDocRef->m_cLayoutMgr.GetLineCount() ){
		m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().SetViewTopLine(nViewTopLine);
		m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().SetViewLeftCol(nViewLeftCol);
	}
	m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().MoveCursorProperly( ptCaretPosXY, TRUE );	// 2007.08.23 ryoji MoveCursor()->MoveCursorProperly()
	m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().m_nCaretPosX_Prev = m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX2();
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �Z�[�uUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



//pszOpenFolder pszOpenFolder


/*! �u�t�@�C������t���ĕۑ��v�_�C�A���O
	@date 2001.02.09 genta	���s�R�[�h�����������ǉ�
	@date 2003.03.30 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
	@date 2003.07.20 ryoji	BOM�̗L�������������ǉ�
	@date 2006.11.10 ryoji	���[�U�[�w��̊g���q���󋵈ˑ��ŕω�������
*/
bool CDocFileOperation::SaveFileDialog(
	SSaveInfo*	pSaveInfo	//!< [out]
)
{
	//�_�C�A���O�I�[�v�����̃f�B���N�g��������
	std::tstring strDefFolder; // �f�t�H���g�t�H���_
	{
		// �t�@�C���p�X�̃f�B���N�g��
		strDefFolder = this->m_pcDocRef->m_cDocFile.GetFilePathClass().GetDirPath();

		// �J�����g�f�B���N�g��
		if(strDefFolder.length()==0){
			TCHAR	buf[_MAX_PATH];
			int		n = ::GetCurrentDirectory( _MAX_PATH, buf );
			if( 0 != n && n < _MAX_PATH )strDefFolder = buf;
		}
	}

	//�g���q�w��
	TCHAR	szDefaultWildCard[_MAX_PATH + 10];	// ���[�U�[�w��g���q
	{
		//�t�@�C���p�X�������ꍇ�� *.txt �Ƃ���
		if(!this->m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath()){
			_tcscpy(szDefaultWildCard, _T("*.txt"));
		}
		//�t�@�C���p�X���L��ꍇ�� *.(���̊g���q) �Ƃ���
		else{
			LPCTSTR szExt = this->m_pcDocRef->m_cDocFile.GetFilePathClass().GetExt();
			if(!*szExt)szExt = _T(".*");
			_tcscpy(szDefaultWildCard, _T("*"));
			_tcscat(szDefaultWildCard, szExt);
		}

		//�u�V�K����ۑ����͑S�t�@�C���\���v�I�v�V����
		if( GetDllShareData().m_Common.m_sFile.m_bNoFilterSaveNew )
			_tcscat(szDefaultWildCard, _T(";*.*"));	// �S�t�@�C���\��
	}

	// �_�C�A���O��\��
	m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.Create(
		CNormalProcess::Instance()->GetProcessInstance(),
		CEditWnd::Instance()->GetHwnd(),
		szDefaultWildCard,
		strDefFolder.c_str(),
		CMRU().GetPathList(),		//	�ŋ߂̃t�@�C��
		CMRUFolder().GetPathList()	//	�ŋ߂̃t�H���_
	);
	return m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.DoModalSaveDlg( pSaveInfo, pSaveInfo->eCharCode == CODE_CODEMAX );
}

//!�u�t�@�C������t���ĕۑ��v�_�C�A���O
bool CDocFileOperation::SaveFileDialog(LPTSTR szPath)
{
	SSaveInfo sSaveInfo;
	sSaveInfo.cFilePath = szPath;
	sSaveInfo.eCharCode = CODE_CODEMAX; //###�g���b�L�[
	bool bRet = SaveFileDialog(&sSaveInfo);
	_tcscpy_s(szPath, _MAX_PATH, sSaveInfo.cFilePath);
	return bRet;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �Z�[�u�t���[                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDocFileOperation::DoSaveFlow(const SSaveInfo& _sSaveInfo)
{
	ESaveResult eSaveResult = SAVED_FAILURE;
	SSaveInfo sSaveInfo = _sSaveInfo;

	try{
		//�Z�[�u�O�`�F�b�N
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyCheckSave(&sSaveInfo))throw CFlowInterruption();

		//�Z�[�u�O���܂�����
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyPreBeforeSave(&sSaveInfo))throw CFlowInterruption();

		//�Z�[�u����
		m_pcDocRef->NotifyBeforeSave(sSaveInfo);	//�O����
		m_pcDocRef->NotifySave(sSaveInfo);			//�{����
		m_pcDocRef->NotifyAfterSave(sSaveInfo);		//�㏈��

		//����
		eSaveResult = SAVED_OK; //###��
	}
	catch(CFlowInterruption){
		eSaveResult = SAVED_INTERRUPT;
	}
	catch(...){
		//�\�����ʗ�O�����������ꍇ�� NotifyFinalSave �͕K���ĂԁI
		m_pcDocRef->NotifyFinalSave(SAVED_FAILURE);
		throw;
	}

	//�ŏI����
	m_pcDocRef->NotifyFinalSave(eSaveResult);

	return eSaveResult==SAVED_OK;
}


/*! �㏑���ۑ�

	@return �ۑ����s��ꂽor�ۑ��s�v�̂��߉����s��Ȃ������Ƃ���true��Ԃ�

	@date 2004.06.05 genta  �r���[���[�h�̃`�F�b�N��CEditDoc����㏑���ۑ������Ɉړ�
	@date 2006.12.30 ryoji  CEditView::Command_FILESAVE()���珈���{�̂�؂�o��
	@date 2008.03.20 kobake �߂�l�̎d�l���`
*/
bool CDocFileOperation::FileSave()
{
	//�t�@�C�������w�肳��Ă��Ȃ��ꍇ�́u���O��t���ĕۑ��v�̃t���[�֑J��
	if( !m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath() ){
		return FileSaveAs();
	}

	//�Z�[�u���
	SSaveInfo sSaveInfo;
	m_pcDocRef->GetSaveInfo(&sSaveInfo);
	sSaveInfo.cEol = EOL_NONE; //���s�R�[�h���ϊ�

	//�㏑������
	return m_pcDocRef->m_cDocFileOperation.DoSaveFlow( sSaveInfo );
}



/*! ���O��t���ĕۑ��t���[

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS_DIALOG()���珈���{�̂�؂�o��
*/
bool CDocFileOperation::FileSaveAs()
{
	//�Z�[�u���
	SSaveInfo sSaveInfo;
	m_pcDocRef->GetSaveInfo(&sSaveInfo);
	if(CAppMode::Instance()->IsViewMode())sSaveInfo.cFilePath = _T(""); //���ǂݍ��ݐ�p���[�h�̂Ƃ��̓t�@�C�������w�肵�Ȃ�

	//�_�C�A���O�\��
	if(!SaveFileDialog(&sSaveInfo))return false;

	//�Z�[�u����
	return DoSaveFlow(sSaveInfo);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �N���[�Y                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/* ����(����)

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()���珈���{�̂�؂�o��
*/
bool CDocFileOperation::FileClose()
{
	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
	if( !m_pcDocRef->OnFileClose() ){
		return false;
	}

	/* �����f�[�^�̃N���A */
	m_pcDocRef->InitDoc();

	/* �S�r���[�̏����� */
	m_pcDocRef->InitAllView();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcDocRef->m_pcEditWnd->UpdateCaption();


	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���̑�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ���ĊJ��
	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()���珈���{�̂�؂�o��
*/
void CDocFileOperation::FileCloseOpen( const SLoadInfo& _sLoadInfo )
{
	//����
	if(!FileClose()){
		return;
	}

	//�t�@�C�����w�肪�����ꍇ�̓_�C�A���O�œ��͂�����
	SLoadInfo sLoadInfo = _sLoadInfo;
	if( sLoadInfo.cFilePath.Length()==0 ){
		if( !OpenFileDialog( CEditWnd::Instance()->GetHwnd(), NULL, &sLoadInfo ) ){
			return;
		}
	}

	//�J��
	FileLoad(&sLoadInfo);
}
