#include "StdAfx.h"
#include "CAutoReloadAgent.h"
//#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "dlg/CDlgFileUpdateQuery.h"
#include "sakura_rc.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CAutoReloadAgent::CAutoReloadAgent()
: m_eWatchUpdate( WU_QUERY )
, m_nPauseCount(0)
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CAutoReloadAgent::OnBeforeSave(const SSaveInfo& sSaveInfo)
{
	//	Sep. 7, 2003 genta
	//	�ۑ�����������܂ł̓t�@�C���X�V�̒ʒm��}������
	PauseWatching();
}

void CAutoReloadAgent::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	//	Sep. 7, 2003 genta
	//	�t�@�C���X�V�̒ʒm�����ɖ߂�
	ResumeWatching();

	// ���O��t���ĕۑ�����ă��[�h���������ꂽ���̕s��������ǉ��iANSI�łƂ̍��فj	// 2009.08.12 ryoji
	if(!sSaveInfo.bOverwriteMode){
		m_eWatchUpdate = WU_QUERY;	// �u���O��t���ĕۑ��v�őΏۃt�@�C�����ύX���ꂽ�̂ōX�V�Ď����@���f�t�H���g�ɖ߂�
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CAutoReloadAgent::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();
	//pcDoc->m_cDocFile.m_sFileInfo.cFileTime.SetFILETIME(ftime); //#####���ɐݒ�ς݂̂͂�
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �e�픻��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool CAutoReloadAgent::_ToDoChecking() const
{
	const CommonSetting_File& setting = GetDllShareData().m_Common.m_sFile;
	if(IsPausing())return false;
	if(!setting.m_bCheckFileTimeStamp)return false;	//�X�V�̊Ď��ݒ�
	if(m_eWatchUpdate==WU_NONE)return false;
	if(setting.m_nFileShareMode!=SHAREMODE_NOT_EXCLUSIVE)return false; // �t�@�C���̔r�����䃂�[�h
	HWND hwndActive = ::GetActiveWindow();
	if(hwndActive==NULL)return false;	/* �A�N�e�B�u�H */
	if(hwndActive!=CEditWnd::getInstance()->GetHwnd())return false;
	if(!GetListeningDoc()->m_cDocFile.GetFilePathClass().IsValidPath())return false;
	if(GetListeningDoc()->m_cDocFile.GetDocFileTime().IsZero()) return false;	/* ���ݕҏW���̃t�@�C���̃^�C���X�^���v */
	return true;
}

bool CAutoReloadAgent::_IsFileUpdatedByOther(FILETIME* pNewFileTime) const
{
	/* �t�@�C���X�^���v���`�F�b�N���� */
	// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX�i�t�@�C�������b�N����Ă��Ă��^�C���X�^���v�擾�\�j
	CFileTime ftime;
	if( GetLastWriteTimestamp( GetListeningDoc()->m_cDocFile.GetFilePath(), &ftime )){
		if( 0 != ::CompareFileTime( &GetListeningDoc()->m_cDocFile.GetDocFileTime().GetFILETIME(), &ftime.GetFILETIME() ) )	//	Aug. 13, 2003 wmlhq �^�C���X�^���v���Â��ύX����Ă���ꍇ�����o�ΏۂƂ���
		{
			*pNewFileTime = ftime.GetFILETIME();
			return true;
		}
	}
	return false;
}

/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
void CAutoReloadAgent::CheckFileTimeStamp()
{
	if(!_ToDoChecking())return;

	CEditDoc* pcDoc = GetListeningDoc();

	//�^�C���X�^���v�Ď�
	FILETIME ftime;
	if(!_IsFileUpdatedByOther(&ftime))return;
	pcDoc->m_cDocFile.m_sFileInfo.cFileTime.SetFILETIME(ftime); //�^�C���X�^���v�X�V

	//	From Here Dec. 4, 2002 genta
	switch( m_eWatchUpdate ){
	case WU_NOTIFY:
		{
			//�t�@�C���X�V�̂��m�点 -> �X�e�[�^�X�o�[
			TCHAR szText[40];
			const CFileTime& ctime = pcDoc->m_cDocFile.GetDocFileTime();
			auto_sprintf( szText, _T("���t�@�C���X�V %02d:%02d:%02d"), ctime->wHour, ctime->wMinute, ctime->wSecond );
			pcDoc->m_pcEditWnd->SendStatusMessage( szText );
		}	
		break;
	default:
		{
			PauseWatching(); // �X�V�Ď��̗}��

			CDlgFileUpdateQuery dlg( pcDoc->m_cDocFile.GetFilePath(), pcDoc->m_cDocEditor.IsModified() );
			int result = dlg.DoModal(
				G_AppInstance(),
				CEditWnd::getInstance()->GetHwnd(),
				IDD_FILEUPDATEQUERY,
				0
			);

			switch( result ){
			case 1:	// �ēǍ�
				/* ����t�@�C���̍ăI�[�v�� */
				pcDoc->m_cDocFileOperation.ReloadCurrentFile( pcDoc->m_cDocFile.m_sFileInfo.eCharCode );
				m_eWatchUpdate = WU_QUERY;
				break;
			case 2:	// �Ȍ�ʒm���b�Z�[�W�̂�
				m_eWatchUpdate = WU_NOTIFY;
				break;
			case 3:	// �Ȍ�X�V���Ď����Ȃ�
				m_eWatchUpdate = WU_NONE;
				break;
			case 0:	// CLOSE
			default:
				m_eWatchUpdate = WU_QUERY;
				break;
			}

			ResumeWatching(); //�Ď��ĊJ
		}
		break;
	}
	//	To Here Dec. 4, 2002 genta
}
