#include "stdafx.h"
#include "CStream.h"

//�R���X�g���N�^�E�f�X�g���N�^
CStream::CStream(const TCHAR* tszPath, const TCHAR* tszMode)
{
	m_fp = NULL;
	Open(tszPath,tszMode);
}

CStream::CStream()
{
	m_fp = NULL;
}

CStream::~CStream()
{
	Close();
}

//����
void CStream::Open(const TCHAR* tszPath, const TCHAR* tszMode)
{
	Close(); //���ɊJ���Ă�����A��x����
	m_fp = _tfopen(tszPath,tszMode);
}

void CStream::Close()
{
	if(m_fp){
		fclose(m_fp);
		m_fp=NULL;
	}
}

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
