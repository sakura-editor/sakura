#include "StdAfx.h"
#include "CDocLocker.h"
#include "CDocFile.h"
#include "window/CEditWnd.h"



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CDocLocker::CDocLocker()
: m_bIsDocWritable(true)
{
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ���[�h�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CDocLocker::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//�������߂邩����
	CheckWritable(!sLoadInfo.bViewMode);
	if( !m_bIsDocWritable ){
		return;
	}

	// �t�@�C���̔r�����b�N
	pcDoc->m_cDocFileOperation.DoFileLock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �Z�[�u�O��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CDocLocker::OnBeforeSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// �t�@�C���̔r�����b�N����
	pcDoc->m_cDocFileOperation.DoFileUnlock();
}

void CDocLocker::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// �������߂邩����
	m_bIsDocWritable = true;

	// �t�@�C���̔r�����b�N
	pcDoc->m_cDocFileOperation.DoFileLock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�F�b�N                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �������߂邩����
void CDocLocker::CheckWritable(bool bMsg)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// �t�@�C�������݂��Ȃ��ꍇ (�u�J���v�ŐV�����t�@�C�����쐬��������) �́A�ȉ��̏����͍s��Ȃ�
	if( !fexist(pcDoc->m_cDocFile.GetFilePath()) ){
		m_bIsDocWritable = true;
		return;
	}

	// �ǂݎ���p�t�@�C���̏ꍇ�́A�ȉ��̏����͍s��Ȃ�
	if( !pcDoc->m_cDocFile.HasWritablePermission() ){
		m_bIsDocWritable = false;
		return;
	}

	// �������߂邩����
	CDocFile& cDocFile = pcDoc->m_cDocFile;
	m_bIsDocWritable = cDocFile.IsFileWritable();
	if(!m_bIsDocWritable && bMsg){
		// �r������Ă���ꍇ�������b�Z�[�W���o��
		// ���̑��̌����i�t�@�C���V�X�e���̃Z�L�����e�B�ݒ�Ȃǁj�ł͓ǂݎ���p�Ɠ��l�Ƀ��b�Z�[�W���o���Ȃ�
		if( ::GetLastError() == ERROR_SHARING_VIOLATION ){
			TopWarningMessage(
				CEditWnd::getInstance()->GetHwnd(),
				_T("%ts\n�͌��ݑ��̃v���Z�X�ɂ���ď����݂��֎~����Ă��܂��B"),
				cDocFile.GetFilePathClass().IsValidPath() ? cDocFile.GetFilePath() : _T("(����)")
			);
		}
	}
}
