#include "stdafx.h"
#include "CDocFileOperation.h"
#include "CWaitCursor.h"
#include "CControlTray.h"
#include "util/window.h"
#include "env/CSakuraEnvironment.h"
#include "doc/CDocVisitor.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���b�N                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDocFileOperation::_ToDoLock() const
{
	// �t�@�C�����J���Ă��Ȃ�
	if( !m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath() )return false;

	// �r���[���[�h
	if( CAppMode::Instance()->IsViewMode() )return false;

	// �r���ݒ�
	if( GetDllShareData().m_Common.m_sFile.m_nFileShareMode == SHAREMODE_NOT_EXCLUSIVE )return false;

	return true;
}

void CDocFileOperation::DoFileLock()
{
	if(this->_ToDoLock()){
		m_pcDocRef->m_cDocFile.FileLock(GetDllShareData().m_Common.m_sFile.m_nFileShareMode);
	}
}

void CDocFileOperation::DoFileUnlock()
{
	m_pcDocRef->m_cDocFile.FileUnlock();
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
		G_AppInstance(),
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
		//���[�h�O�`�F�b�N
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyCheckLoad(pLoadInfo))throw CFlowInterruption();

		//���[�h����
		m_pcDocRef->NotifyBeforeLoad(*pLoadInfo);			//�O����
		eLoadResult = m_pcDocRef->NotifyLoad(*pLoadInfo);	//�{����
		m_pcDocRef->NotifyAfterLoad(*pLoadInfo);			//�㏈��
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
		// �J�[�\���ʒu�\�����X�V����	// 2008.07.22 ryoji
		m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();
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
	sLoadInfo.bViewMode=CAppMode::Instance()->IsViewMode();
	sLoadInfo.bRequestReload=true;
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
	// �ꎞ�K�p��g���q�Ȃ��̏ꍇ�̊g���q���^�C�v�ʐݒ肩�玝���Ă���
	// 2008/6/14 �傫������ Uchi
	TCHAR	szDefaultWildCard[_MAX_PATH + 10];	// ���[�U�[�w��g���q
	{
		LPCTSTR	szExt;
		TCHAR	szWork[64];
		TCHAR*	pStr;
		TCHAR*	pEnd;

		CTypeConfig	nSettingType = m_pcDocRef->m_cDocType.GetDocumentType();
		//�t�@�C���p�X�������ꍇ�� *.txt �Ƃ���
		if(!this->m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath()){
			szExt = _T("");
		}
		else {
			szExt = this->m_pcDocRef->m_cDocFile.GetFilePathClass().GetExt();
		}
		if (nSettingType.GetIndex() == 0) {
			// ��{
			if (szExt[0] == _T('\0')) { 
				// �t�@�C���p�X�������܂��͊g���q�Ȃ�
				_tcscpy(szDefaultWildCard, _T("*.txt"));
			}
			else {
				// �g���q����
				_tcscpy(szDefaultWildCard, _T("*"));
				_tcscat(szDefaultWildCard, szExt);
			}
		}
		else {
			szDefaultWildCard[0] = _T('\0'); 
			if (szExt[0] != _T('\0')) {
				// �t�@�C���p�X������A�g���q����̏ꍇ�A�g�b�v�Ɏw��
				_tcscpy(szDefaultWildCard, _T("*"));
				_tcscat(szDefaultWildCard, szExt);
			}
			// �g���q���w��ɍ��킹��
			pStr = pEnd = CDocTypeManager().GetTypeSetting(nSettingType).m_szTypeExts;
			do {
				if (*pEnd == _T('\0') || *pEnd == _T(',')) {
					auto_strncpy(szWork, pStr, pEnd - pStr);
					szWork[pEnd - pStr]= _T('\0');
					if (szExt[0] == _T('\0') || auto_stricmp(szWork, szExt + 1) != 0) {
						// �g���q�w��Ȃ��A�܂��̓}�b�`�����g���q�łȂ�
						if (szDefaultWildCard[0] != _T('\0')) {
							_tcscat(szDefaultWildCard, _T(";"));
						}
						_tcscat(szDefaultWildCard, _T("*."));
						_tcscat(szDefaultWildCard, szWork);
					}
					pStr = pEnd + 1;
				}
			} while	(*pEnd++ != _T('\0'));
		}

		if(!this->m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath()){
			//�u�V�K����ۑ����͑S�t�@�C���\���v�I�v�V����	// 2008/6/15 �o�O�t�B�b�N�X Uchi
			if( GetDllShareData().m_Common.m_sFile.m_bNoFilterSaveNew )
				_tcscat(szDefaultWildCard, _T(";*.*"));	// �S�t�@�C���\��
		}
		else {
			//�u�V�K�ȊO����ۑ����͑S�t�@�C���\���v�I�v�V����
			if( GetDllShareData().m_Common.m_sFile.m_bNoFilterSaveFile )
				_tcscat(szDefaultWildCard, _T(";*.*"));	// �S�t�@�C���\��
		}
	}

	// �_�C�A���O��\��
	m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.Create(
		G_AppInstance(),
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

bool CDocFileOperation::DoSaveFlow(SSaveInfo* pSaveInfo)
{
	ESaveResult eSaveResult = SAVED_FAILURE;

	try{
		//�Z�[�u�O�`�F�b�N
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyCheckSave(pSaveInfo))throw CFlowInterruption();

		//�Z�[�u�O���܂�����
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyPreBeforeSave(pSaveInfo))throw CFlowInterruption();

		//�Z�[�u����
		m_pcDocRef->NotifyBeforeSave(*pSaveInfo);	//�O����
		m_pcDocRef->NotifySave(*pSaveInfo);			//�{����
		m_pcDocRef->NotifyAfterSave(*pSaveInfo);	//�㏈��

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
	sSaveInfo.bOverwriteMode = true; //�㏑���v��

	//�㏑������
	return m_pcDocRef->m_cDocFileOperation.DoSaveFlow(&sSaveInfo);
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
	return DoSaveFlow(&sSaveInfo);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �N���[�Y                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*
	����(����)�B
	���[�U�L�����Z�����쓙�ɂ��N���[�Y����Ȃ������ꍇ�� false ��Ԃ��B

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
