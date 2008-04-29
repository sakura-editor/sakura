#pragma once

#include "mem/CMemory.h"
#include "charset/CCodeBase.h"

class CIoBridge{
public:
	//! ���������̃G���R�[�h�֕ϊ�
	static EConvertResult FileToImpl(
		const CMemory&	cSrc,			//!< [in]  �ϊ���������
		CNativeW*		pDst,			//!< [out] �ϊ��惁����(UNICODE)
		ECodeType		eCharcodeFrom,	//!< [in]  �ϊ����������̕����R�[�h
		int				nFlag			//!< [in]  bit 0: MIME Encode���ꂽ�w�b�_��decode���邩�ǂ���
	);

	//! �t�@�C���̃G���R�[�h�֕ύX
	static EConvertResult ImplToFile(
		const CNativeW&		cSrc,		//!< [in]  �ϊ���������(UNICODE)
		CMemory*			pDst,		//!< [out] �ϊ��惁����
		ECodeType			eCharcodeTo	//!< [in]  �ϊ��惁�����̕����R�[�h
	);
};

