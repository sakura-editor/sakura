#pragma once

#include "CStream.h"

class CBinaryInputStream : public CStream{
public:
	CBinaryInputStream(LPCTSTR tszFilePath);

public:
	//! �X�g���[���́u�c��v�T�C�Y���擾
	int GetLength();

	//! �f�[�^�𖳕ϊ��œǂݍ��ށB�߂�l�͓ǂݍ��񂾃o�C�g���B
	int Read(void* pBuffer, int nSizeInBytes);
};

class CBinaryOutputStream : public COutputStream{
public:
	CBinaryOutputStream(LPCTSTR tszFilePath, bool bExceptionMode = false);
};

