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
#ifndef SAKURA_CUNICODE_95AC7095_F71E_458B_80B3_1AA4036E25109_H_
#define SAKURA_CUNICODE_95AC7095_F71E_458B_80B3_1AA4036E25109_H_

// IsUtf16SurrogHi()�AIsUtf16SurrogLow() �֐���charset/codechecker.h �Ɉړ�

#include "CCodeBase.h"

class CUnicode : public CCodeBase{
public:
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){	//!< ����R�[�h �� UNICODE    �ϊ�
		return UnicodeToUnicode_in(cSrc, pDst);
	}
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){	//!< UNICODE    �� ����R�[�h �ϊ�
		return UnicodeToUnicode_out(cSrc, pDst);
	}
	void GetBom(CMemory* pcmemBom);	//!< BOM�f�[�^�擾
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< ���s�f�[�^�擾

public:
	//����
	static EConvertResult _UnicodeToUnicode_in(const CMemory& cSrc, CNativeW* pDstMem, const bool bBigEndian);		// Unicode   �� Unicode (���͑�)
	static EConvertResult _UnicodeToUnicode_out(const CNativeW& cSrc, CMemory* pDstMem, const bool bBigEndian);	// Unicode   �� Unicode (�o�͑�)
	inline static EConvertResult UnicodeToUnicode_in(const CMemory& cSrc, CNativeW* pDst){ return _UnicodeToUnicode_in(cSrc, pDst, false); }
	inline static EConvertResult UnicodeToUnicode_out(const CNativeW& cSrc, CMemory* pDst){ return _UnicodeToUnicode_out(cSrc, pDst, false); }

};

#endif /* SAKURA_CUNICODE_95AC7095_F71E_458B_80B3_1AA4036E25109_H_ */
/*[EOF]*/
