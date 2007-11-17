#include "stdafx.h"
#include "CIoBridge.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"

//! ���������̃G���R�[�h�֕ϊ�
EConvertResult CIoBridge::FileToImpl(
	const CMemory*	pSrc,			//!< [in]  �ϊ���������
	CNativeW*		pDst,			//!< [out] �ϊ��惁����(UNICODE)
	ECodeType		eCharcodeFrom,	//!< [in]  �ϊ����������̕����R�[�h
	int				nFlag			//!< [in]  bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
)
{
	//�C�ӂ̕����R�[�h����Unicode�֕ϊ�����
	CCodeBase* pCode=CCodeFactory::CreateCodeBase(eCharcodeFrom,nFlag);
	EConvertResult ret = pCode->CodeToUnicode(pSrc,pDst);
	delete pCode;

	//����
	return ret;
}

EConvertResult CIoBridge::ImplToFile(
	const CNativeW*	pSrc,		//!< [in]  �ϊ���������(UNICODE)
	CMemory*			pDst,		//!< [out] �ϊ��惁����
	ECodeType			eCharcodeTo	//!< [in]  �ϊ��惁�����̕����R�[�h
)
{
	// Unicode����C�ӂ̕����R�[�h�֕ϊ�����
	CCodeBase* pCode=CCodeFactory::CreateCodeBase(eCharcodeTo,0);
	EConvertResult ret = pCode->UnicodeToCode(pSrc,pDst);
	delete pCode;

	//����
	return ret;
}

