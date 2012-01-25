#include "StdAfx.h"
#include "CBinaryStream.h"


CBinaryInputStream::CBinaryInputStream(LPCTSTR tszFilePath)
: CStream(tszFilePath,_T("rb"))
{
}

//! �X�g���[���́u�c��v�T�C�Y���擾
int CBinaryInputStream::GetLength()
{
	long nCur = ftell(GetFp());
	fseek(GetFp(), 0, SEEK_END);
	long nDataLen = ftell(GetFp());
	fseek(GetFp(), nCur, SEEK_SET);
	return nDataLen;
}

//! �f�[�^�𖳕ϊ��œǂݍ��ށB�߂�l�͓ǂݍ��񂾃o�C�g���B
int CBinaryInputStream::Read(void* pBuffer, int nSizeInBytes)
{
	return fread(pBuffer,1,nSizeInBytes,GetFp());
}



CBinaryOutputStream::CBinaryOutputStream(LPCTSTR tszFilePath, bool bExceptionMode)
: COutputStream(tszFilePath,_T("wb"),bExceptionMode)
{
}


