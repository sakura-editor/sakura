#pragma once

#include "CConvert.h"

class CConvert_Base64Decode : public CConvert{
public:
	//CConvert�C���^�[�t�F�[�X
	bool DoConvert(CNativeW* pcData)
	{
		//$$$ �������ɂ��Afalse��Ԃ�
		return false;
	}

public:
	//����
	static void BASE64Decode(const CNativeW* pcSrc, CMemory* pcDst);	// Base64�f�R�[�h
	template <class CHAR_TYPE>
	static long MemBASE64_Decode( const CHAR_TYPE* pszSrc, long nSrcLen, BYTE* pDst );	/* Base64�f�R�[�h */
};
