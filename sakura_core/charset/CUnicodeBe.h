/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CUNICODEBE_484B0FE6_3896_4E2E_83BA_CC8718999735_H_
#define SAKURA_CUNICODEBE_484B0FE6_3896_4E2E_83BA_CC8718999735_H_

#include "CCodeBase.h"
#include "CUnicode.h"
#include "CEol.h"

class CUnicodeBe : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return UnicodeBEToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToUnicodeBE(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetBom(CMemory* pcmemBom);	//!< BOM�f�[�^�擾
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< ���s�f�[�^�擾

public:

	inline static EConvertResult UnicodeBEToUnicode(CMemory* pMem)
		{ return CUnicode::_UnicodeToUnicode_in(pMem,true); }	// UnicodeBE �� Unicode�R�[�h�ϊ� //2007.08.13 kobake �ǉ�
	inline static EConvertResult UnicodeToUnicodeBE(CMemory* pMem)
		{ return CUnicode::_UnicodeToUnicode_out(pMem,true); }	// Unicode   �� UnicodeBE�R�[�h�ϊ�

};

#endif /* SAKURA_CUNICODEBE_484B0FE6_3896_4E2E_83BA_CC8718999735_H_ */
/*[EOF]*/
