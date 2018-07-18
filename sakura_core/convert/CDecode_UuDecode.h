// この行は文字化け対策のものです
/*!	@file
	@brief Unix-to-Unix Decode

	@author 
*/

/*
	Copyright (C)

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
#ifndef SAKURA_CDECODE_UUDECODE_2082FDBF_E5F4_4173_87E3_A862FA4F694B_H_
#define SAKURA_CDECODE_UUDECODE_2082FDBF_E5F4_4173_87E3_A862FA4F694B_H_

#include "convert/CDecode.h"

class CDecode_UuDecode : public CDecode{

	TCHAR m_aFilename[_MAX_PATH];
public:
	bool DoDecode(const CNativeW& cData, CMemory* pDst);
	void CopyFilename( TCHAR *pcDst ) const { _tcscpy( pcDst, m_aFilename ); }
};

#endif /* SAKURA_CDECODE_UUDECODE_2082FDBF_E5F4_4173_87E3_A862FA4F694B_H_ */
/*[EOF]*/
