#include "stdafx.h"
#include "io/CFile.h"
#include "window/CEditWnd.h" // �ύX�\��
#include <io.h>

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CFile::CFile(LPCTSTR pszPath)
: m_nFileShareModeOld( SHAREMODE_NOT_EXCLUSIVE )
, m_hLockedFile( INVALID_HANDLE_VALUE )
{
	if(pszPath){
		SetFilePath(pszPath);
	}
}

CFile::~CFile()
{
	FileUnlock();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �e�픻��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFile::IsFileExist() const
{
	return fexist(GetFilePath());
}

bool CFile::HasWritablePermission() const
{
	return -1 != _taccess( GetFilePath(), 2 );
}

bool CFile::IsFileWritable() const
{
	//�������߂邩����
	// Note. ���̃v���Z�X�������I�ɏ������݋֎~���Ă��邩�ǂ���
	//       �� GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE �Ń`�F�b�N����
	//          ���ۂ̃t�@�C���ۑ�������Ɠ����� _tfopen �� _T("wb") ���g�p���Ă���
	HANDLE hFile = CreateFile(
		this->GetFilePath(),			//�t�@�C����
		GENERIC_WRITE,					//�������[�h
		FILE_SHARE_READ | FILE_SHARE_WRITE,	//�ǂݏ������L
		NULL,							//����̃Z�L�����e�B�L�q�q
		OPEN_EXISTING,					//�t�@�C�������݂��Ȃ���Ύ��s
		FILE_ATTRIBUTE_NORMAL,			//���ɑ����͎w�肵�Ȃ�
		NULL							//�e���v���[�g����
	);
	if(hFile==INVALID_HANDLE_VALUE){
		return false;
	}
	CloseHandle(hFile);
	return true;
}

bool CFile::IsFileReadable() const
{
	HANDLE hTest = CreateFile(
		this->GetFilePath(),
		GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN,
		NULL
	);
	if(hTest==INVALID_HANDLE_VALUE){
		// �ǂݍ��݃A�N�Z�X�����Ȃ�
		return false;
	}
	CloseHandle( hTest );
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ���b�N                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �t�@�C���̔r�����b�N����
void CFile::FileUnlock()
{
	//�N���[�Y
	if( m_hLockedFile != INVALID_HANDLE_VALUE ){
		::CloseHandle( m_hLockedFile );
		m_hLockedFile = INVALID_HANDLE_VALUE;
	}
	// �t�@�C���̔r�����䃂�[�h
	m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;
}


//! �t�@�C���̔r�����b�N
bool CFile::FileLock( EShareMode eShareMode )
{
	// ���b�N����
	FileUnlock();

	// �t�@�C���̑��݃`�F�b�N
	if( !this->IsFileExist() ){
		return false;
	}

	// ���[�h�ݒ�
	if(eShareMode==SHAREMODE_NOT_EXCLUSIVE)return true;
	
	//�t���O
	DWORD dwShareMode=0;
	switch(eShareMode){
	case SHAREMODE_NOT_EXCLUSIVE:	return true;										break; //�r�����䖳��
	case SHAREMODE_DENY_READWRITE:	dwShareMode = 0;									break; //�ǂݏ����֎~�����L����
	case SHAREMODE_DENY_WRITE:		dwShareMode = FILE_SHARE_READ;						break; //�������݋֎~���ǂݍ��݂̂ݔF�߂�
	default:						dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;	break; //�֎~�����Ȃ����ǂݏ������ɔF�߂�
	}

	//�I�[�v��
	m_hLockedFile = CreateFile(
		this->GetFilePath(),			//�t�@�C����
		GENERIC_READ,					//�ǂݏ����^�C�v
		dwShareMode,					//���L���[�h
		NULL,							//����̃Z�L�����e�B�L�q�q
		OPEN_EXISTING,					//�t�@�C�������݂��Ȃ���Ύ��s
		FILE_ATTRIBUTE_NORMAL,			//���ɑ����͎w�肵�Ȃ�
		NULL							//�e���v���[�g����
	);

	//����
	if( INVALID_HANDLE_VALUE == m_hLockedFile ){
		const TCHAR*	pszMode;
		switch( eShareMode ){
		case SHAREMODE_DENY_READWRITE:	pszMode = _T("�ǂݏ����֎~���[�h"); break;
		case SHAREMODE_DENY_WRITE:		pszMode = _T("�������݋֎~���[�h"); break;
		default:						pszMode = _T("����`�̃��[�h�i��肪����܂��j"); break;
		}
		WarningBeep();
		TopWarningMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("%ts\n��%ts�Ń��b�N�ł��܂���ł����B\n���݂��̃t�@�C���ɑ΂���r������͖����ƂȂ�܂��B"),
			GetFilePathClass().IsValidPath() ? GetFilePath() : _T("�i����j"),
			pszMode
		);
		return false;
	}
	m_nFileShareModeOld = eShareMode;

	return true;
}


