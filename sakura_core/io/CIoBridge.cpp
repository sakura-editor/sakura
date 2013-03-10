#include "StdAfx.h"
#include "io/CIoBridge.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "CEol.h"

//! ���������̃G���R�[�h�֕ϊ�
EConvertResult CIoBridge::FileToImpl(
	const CMemory&		cSrc,		//!< [in]  �ϊ���������
	CNativeW*			pDst,		//!< [out] �ϊ��惁����(UNICODE)
	CCodeBase*			pCode,		//!< [in]  �ϊ����������̕����R�[�h
	int					nFlag		//!< [in]  bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
)
{
	//�C�ӂ̕����R�[�h����Unicode�֕ϊ�����
	EConvertResult ret = pCode->CodeToUnicode(cSrc,pDst);

	//����
	return ret;
}

EConvertResult CIoBridge::ImplToFile(
	const CNativeW&		cSrc,		//!< [in]  �ϊ���������(UNICODE)
	CMemory*			pDst,		//!< [out] �ϊ��惁����
	CCodeBase*			pCode		//!< [in]  �ϊ��惁�����̕����R�[�h
)
{
	// Unicode����C�ӂ̕����R�[�h�֕ϊ�����
	EConvertResult ret = pCode->UnicodeToCode(cSrc,pDst);

	//����
	return ret;
}


