#pragma once

#include "CConvert.h"

class CConvert_UuDecode : public CConvert{
public:
	bool DoConvert(CNativeW* pcData);


	// uudecode(�f�R�[�h)
	static void UUDECODE(
		const CNativeW&	pcSrc,
		CMemory*			pcDst,
		TCHAR*				pszFileName
	);
};
