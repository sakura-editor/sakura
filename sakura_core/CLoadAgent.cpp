#include "StdAfx.h"
#include "CLoadAgent.h"
#include "CReadManager.h"
#include "_main/CAppMode.h"
#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "env/CDocTypeManager.h"
#include "env/CShareData.h"
#include "doc/CEditDoc.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "CVisualProgress.h"
#include "util/file.h"

ECallbackResult CLoadAgent::OnCheckLoad(SLoadInfo* pLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// �����[�h�v���̏ꍇ�́A�p���B
	if(pLoadInfo->bRequestReload)goto next;

	//�t�H���_���w�肳�ꂽ�ꍇ�́u�t�@�C�����J���v�_�C�A���O��\�����A���ۂ̃t�@�C�����͂𑣂�
	if( IsDirectory(pLoadInfo->cFilePath) ){
		std::vector<std::tstring> files;
		SLoadInfo sLoadInfo(_T(""), CODE_AUTODETECT, false);
		bool bDlgResult = pcDoc->m_cDocFileOperation.OpenFileDialog(
			CEditWnd::getInstance()->GetHwnd(),
			pLoadInfo->cFilePath,	//�w�肳�ꂽ�t�H���_
			&sLoadInfo,
			files
		);
		if( !bDlgResult ){
			return CALLBACK_INTERRUPT; //�L�����Z�����ꂽ�ꍇ�͒��f
		}
		if( 0 < files.size() ){
			sLoadInfo.cFilePath = files[0].c_str();
			// ���̃t�@�C���͐V�K�E�B���h�E
			for( size_t i = 1; i < files.size(); i++ ){
				SLoadInfo sFilesLoadInfo = sLoadInfo;
				sFilesLoadInfo.cFilePath = files[i].c_str();
				CControlTray::OpenNewEditor(
					G_AppInstance(),
					CEditWnd::getInstance()->GetHwnd(),
					sFilesLoadInfo,
					NULL,
					true
				);
			}
		}
		*pLoadInfo = sLoadInfo;
	}

	// ���̃E�B���h�E�Ŋ��ɊJ����Ă���ꍇ�́A������A�N�e�B�u�ɂ���
	HWND	hWndOwner;
	if( CShareData::getInstance()->ActiveAlreadyOpenedWindow(pLoadInfo->cFilePath, &hWndOwner, pLoadInfo->eCharCode) ){
		pLoadInfo->bOpened = true;
		return CALLBACK_INTERRUPT;
	}

	// ���݂̃E�B���h�E�ɑ΂��ăt�@�C����ǂݍ��߂Ȃ��ꍇ�́A�V���ȃE�B���h�E���J���A�����Ƀt�@�C����ǂݍ��܂���
	if(!pcDoc->IsAcceptLoad()){
		CControlTray::OpenNewEditor(
			G_AppInstance(),
			CEditWnd::getInstance()->GetHwnd(),
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
				CEditWnd::getInstance()->GetHwnd(),
				_T("%ts\n�Ƃ����t�@�C���͑��݂��܂���B\n\n�t�@�C����ۑ������Ƃ��ɁA�f�B�X�N��ɂ��̃t�@�C�����쐬����܂��B"),	//Mar. 24, 2001 jepro �኱�C��
				pLoadInfo->cFilePath.GetBufferPointer()
			);
		}
	}

	// �ǂݎ��\�`�F�b�N
	do{
		CFile cFile(pLoadInfo->cFilePath.c_str());

		//�t�@�C�������݂��Ȃ��ꍇ�̓`�F�b�N�ȗ�
		if(!cFile.IsFileExist())break;

		// ���b�N�͈ꎞ�I�ɉ������ă`�F�b�N����i�`�F�b�N�����Ɍ�߂�ł��Ȃ��Ƃ���܂Ői�߂�����S�j
		// �� ���b�N���Ă��Ă��A�N�Z�X���̕ύX�ɂ���ēǂݎ��Ȃ��Ȃ��Ă��邱�Ƃ�����
		bool bLock = (pLoadInfo->IsSamePath(pcDoc->m_cDocFile.GetFilePath()) && pcDoc->m_cDocFile.IsFileLocking());
		if( bLock ) pcDoc->m_cDocFileOperation.DoFileUnlock();

		//�`�F�b�N
		if(!cFile.IsFileReadable()){
			if( bLock ) pcDoc->m_cDocFileOperation.DoFileLock(false);
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				_T("\'%ts\'\n")
				_T("�Ƃ����t�@�C�����J���܂���B\n")
				_T("�ǂݍ��݃A�N�Z�X��������܂���B"),
				pLoadInfo->cFilePath.c_str()
			);
			return CALLBACK_INTERRUPT; //�t�@�C�������݂��Ă���̂ɓǂݎ��Ȃ��ꍇ�͒��f
		}
		if( bLock ) pcDoc->m_cDocFileOperation.DoFileLock(false);
	}
	while(false);

	// �t�@�C���T�C�Y�`�F�b�N
	if( GetDllShareData().m_Common.m_sFile.m_bAlertIfLargeFile ){
		WIN32_FIND_DATA wfd;
		HANDLE nFind = ::FindFirstFile( pLoadInfo->cFilePath.c_str(), &wfd );
		if( nFind != INVALID_HANDLE_VALUE ){
			::FindClose( nFind );
			LARGE_INTEGER nFileSize;
			nFileSize.HighPart = wfd.nFileSizeHigh;
			nFileSize.LowPart = wfd.nFileSizeLow;
			// GetDllShareData().m_Common.m_sFile.m_nAlertFileSize ��MB�P��
			if( (nFileSize.QuadPart>>20) >= (GetDllShareData().m_Common.m_sFile.m_nAlertFileSize) ){
				int nRet = MYMESSAGEBOX( CEditWnd::getInstance()->GetHwnd(),
					MB_ICONQUESTION | MB_YESNO | MB_TOPMOST,
					GSTR_APPNAME,
					_T("�t�@�C���T�C�Y��%dMB�ȏ゠��܂��B�J���܂����H"),
					GetDllShareData().m_Common.m_sFile.m_nAlertFileSize );
				if( nRet != IDYES ){
					return CALLBACK_INTERRUPT;
				}
			}
		}
	}

	return CALLBACK_CONTINUE;
}

void CLoadAgent::OnBeforeLoad(SLoadInfo* pLoadInfo)
{
}

ELoadResult CLoadAgent::OnLoad(const SLoadInfo& sLoadInfo)
{
	ELoadResult eRet = LOADED_OK;
	CEditDoc* pcDoc = GetListeningDoc();

	/* �����f�[�^�̃N���A */
	pcDoc->InitDoc(); //$$

	// �p�X���m��
	pcDoc->SetFilePathAndIcon( sLoadInfo.cFilePath );

	// ������ʊm��
	pcDoc->m_cDocType.SetDocumentType( sLoadInfo.nType, true );

	// �N���Ɠ����ɓǂޏꍇ�͗\�߃A�E�g���C����͉�ʂ�z�u���Ă���
	// �i�t�@�C���ǂݍ��݊J�n�ƂƂ��Ƀr���[���\�������̂ŁA���ƂŔz�u����Ɖ�ʂ̂�������傫���́j
	if( !pcDoc->m_pcEditWnd->m_cDlgFuncList.m_bEditWndReady ){
		pcDoc->m_pcEditWnd->m_cDlgFuncList.Refresh();
		HWND hEditWnd = pcDoc->m_pcEditWnd->GetHwnd();
		if( !::IsIconic( hEditWnd ) && pcDoc->m_pcEditWnd->m_cDlgFuncList.GetHwnd() ){
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::SendMessageAny( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
	}

	//�t�@�C�������݂���ꍇ�̓t�@�C����ǂ�
	if(fexist(sLoadInfo.cFilePath)){
		//CDocLineMgr�̍\��
		CReadManager cReader;
		CProgressSubject* pOld = CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(&cReader);
		EConvertResult eReadResult = cReader.ReadFile_To_CDocLineMgr(
			&pcDoc->m_cDocLineMgr,
			sLoadInfo,
			&pcDoc->m_cDocFile.m_sFileInfo
		);
		if(eReadResult==RESULT_LOSESOME){
			eRet = LOADED_LOSESOME;
		}
		CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(pOld);
	}
	else{
		// ���݂��Ȃ��Ƃ����h�L�������g�ɕ����R�[�h�𔽉f����
		pcDoc->m_cDocFile.m_sFileInfo.eCharCode = sLoadInfo.eCharCode;
		STypeConfig& types = CDocTypeManager().GetTypeSetting( sLoadInfo.nType );
		if ( sLoadInfo.eCharCode == types.m_encoding.m_eDefaultCodetype ){
			pcDoc->m_cDocFile.m_sFileInfo.bBomExist = ( types.m_encoding.m_bDefaultBom != FALSE );	// 2011.01.24 ryoji �f�t�H���gBOM
		}
		else{
			pcDoc->m_cDocFile.m_sFileInfo.bBomExist = CCodeTypeName( sLoadInfo.eCharCode ).IsBomDefOn();
		}
	}

	/* ���C�A�E�g���̕ύX */
	// 2008.06.07 nasukoji	�܂�Ԃ����@�̒ǉ��ɑΉ�
	// �u�w�茅�Ő܂�Ԃ��v�ȊO�̎��͐܂�Ԃ�����MAXLINEKETAS�ŏ���������
	// �u�E�[�Ő܂�Ԃ��v�́A���̌��OnSize()�ōĐݒ肳���
	STypeConfig ref = pcDoc->m_cDocType.GetDocumentAttribute();
	if( ref.m_nTextWrapMethod != WRAP_SETTING_WIDTH )
		ref.m_nMaxLineKetas = MAXLINEKETAS;

	CProgressSubject* pOld = CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(&pcDoc->m_cLayoutMgr);
	pcDoc->m_cLayoutMgr.SetLayoutInfo(true, ref);
	CEditApp::getInstance()->m_pcVisualProgress->CProgressListener::Listen(pOld);

	return eRet;
}

void CLoadAgent::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	pcDoc->m_pcEditWnd->UpdateCaption();

	// -- -- �� InitAllView�ł���Ă����� -- -- //	// 2009.08.28 nasukoji	CEditView::OnAfterLoad()���炱���Ɉړ�
	pcDoc->m_nCommandExecNum=0;

	// �e�L�X�g�̐܂�Ԃ����@��������
	pcDoc->m_nTextWrapMethodCur = pcDoc->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod;	// �܂�Ԃ����@
	pcDoc->m_bTextWrapMethodCurTemp = false;													// �ꎞ�ݒ�K�p��������

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( pcDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP )
		pcDoc->m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
	else
		pcDoc->m_cLayoutMgr.ClearLayoutLineWidth();		// �e�s�̃��C�A�E�g�s���̋L�����N���A����
}


void CLoadAgent::OnFinalLoad(ELoadResult eLoadResult)
{
	CEditDoc* pcDoc = GetListeningDoc();

	if(eLoadResult==LOADED_FAILURE){
		pcDoc->SetFilePathAndIcon( _T("") );
		pcDoc->m_cDocFile.m_sFileInfo.bBomExist = CCodeTypeName( pcDoc->m_cDocFile.m_sFileInfo.eCharCode ).IsBomDefOn();
	}
	if(eLoadResult==LOADED_LOSESOME){
		CAppMode::getInstance()->SetViewMode(true);
	}

	//�ĕ`�� $$�s��
	CEditWnd::getInstance()->GetActiveView().SetDrawSwitch(true);
	CEditWnd::getInstance()->Views_RedrawAll(); //�r���[�ĕ`��
	InvalidateRect( CEditWnd::getInstance()->GetHwnd(), NULL, TRUE );
	//m_cEditViewArr[m_nActivePaneIndex].DrawCaretPosInfo();
	CCaret& cCaret = CEditWnd::getInstance()->GetActiveView().GetCaret();
	cCaret.MoveCursor(cCaret.GetCaretLayoutPos(),true);
	CEditWnd::getInstance()->GetActiveView().AdjustScrollBars();
}
