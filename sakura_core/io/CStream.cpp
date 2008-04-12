#include "stdafx.h"
#include "CStream.h"
#include <string>

//	::fflush(m_hFile);
//  �l�b�g���[�N��̃t�@�C���������Ă���ꍇ�A
//	�������݌��Flush���s���ƃf�b�g���b�N���������邱�Ƃ�����̂ŁA
//	Close����::fflush���Ăяo���Ă͂����܂���B
//  �ڍׁFhttp://www.microsoft.com/japan/support/faq/KBArticles2.asp?URL=/japan/support/kb/articles/jp288/7/94.asp


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  �t�@�C����������N���X                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

class CFileAttribute{
public:
	CFileAttribute(const TCHAR* tszPath)
	: m_strPath(tszPath)
	, m_bAttributeChanged(false)
	, m_dwAttribute(0)
	{
	}

	//! �w�葮������菜��
	void PopAttribute(DWORD dwPopAttribute)
	{
		if(m_bAttributeChanged)return; //���Ɏ�菜���ς�

		m_dwAttribute = ::GetFileAttributes(m_strPath.c_str());
		if( m_dwAttribute != (DWORD)-1 ){
			if(m_dwAttribute & dwPopAttribute){
				DWORD dwNewAttribute = m_dwAttribute & ~dwPopAttribute;
				::SetFileAttributes(m_strPath.c_str(), dwNewAttribute);
				m_bAttributeChanged=true;
			}
		}
	}
	
	//! ���������ɖ߂�
	void RestoreAttribute()
	{
		if(m_bAttributeChanged)
			::SetFileAttributes(m_strPath.c_str(),m_dwAttribute);
		m_bAttributeChanged = false;
		m_dwAttribute = 0;
	}
private:
	std::tstring	m_strPath;
	bool			m_bAttributeChanged;
	DWORD			m_dwAttribute;
};

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CStream::CStream(const TCHAR* tszPath, const TCHAR* tszMode, bool bExceptionMode)
{
	m_fp = NULL;
	m_pcFileAttribute = NULL;
	m_bExceptionMode = bExceptionMode;
	Open(tszPath,tszMode);
}

/*
CStream::CStream()
{
	m_fp = NULL;
	m_pcFileAttribute = NULL;
	m_bExceptionMode = false;
}
*/

CStream::~CStream()
{
	Close();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    �I�[�v���E�N���[�Y                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//
void CStream::Open(const TCHAR* tszPath, const TCHAR* tszMode)
{
	Close(); //���ɊJ���Ă�����A��x����

	//�����ύX�F�B��or�V�X�e���t�@�C����C�̊֐��œǂݏ����ł��Ȃ��̂ő�����ύX����
	m_pcFileAttribute = new CFileAttribute(tszPath);
	m_pcFileAttribute->PopAttribute(FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

	//�I�[�v��
	m_fp = _tfopen(tszPath,tszMode);
	if(!m_fp){
		Close(); //��������
	}

	//�G���[����
	if(!m_fp && IsExceptionMode()){
		throw CError_FileOpen();
	}
}

void CStream::Close()
{
	//�N���[�Y
	if(m_fp){
		fclose(m_fp);
		m_fp=NULL;
	}

	//��������
	if(m_pcFileAttribute){
		m_pcFileAttribute->RestoreAttribute();
		SAFE_DELETE(m_pcFileAttribute);
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CStream::SeekSet(	//!< �V�[�N
	long offset	//!< �X�g���[���擪����̃I�t�Z�b�g 
)
{
	fseek(m_fp,offset,SEEK_SET);
}

void CStream::SeekEnd(   //!< �V�[�N
	long offset //!< �X�g���[���I�[����̃I�t�Z�b�g
)
{
	fseek(m_fp,offset,SEEK_END);
}
