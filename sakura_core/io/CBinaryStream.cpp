#include "stdafx.h"
#include "CBinaryStream.h"


CBinaryInputStream::CBinaryInputStream(LPCTSTR tszFilePath)
: CStream(tszFilePath,_T("rb"))
{
}

//! �X�g���[���́u�c��v�T�C�Y���擾
int CBinaryInputStream::GetLength()
{
	long nCur = ftell(m_fp);
	fseek(m_fp, 0, SEEK_END);
	long nDataLen = ftell(m_fp);
	fseek(m_fp, nCur, SEEK_SET);
	return nDataLen;
}

//! �f�[�^�𖳕ϊ��œǂݍ��ށB�߂�l�͓ǂݍ��񂾃o�C�g���B
int CBinaryInputStream::Read(void* pBuffer, int nSizeInBytes)
{
	return fread(pBuffer,1,nSizeInBytes,m_fp);
}



CBinaryOutputStream::CBinaryOutputStream(LPCTSTR tszFilePath)
: CStream(tszFilePath,_T("wb"))
{
}

//! �f�[�^�𖳕ϊ��ŏ������ށB�߂�l�͏������񂾃o�C�g���B
int CBinaryOutputStream::Write(const void* pBuffer, int nSizeInBytes)
{
	return fwrite(pBuffer,1,nSizeInBytes,m_fp);
}

