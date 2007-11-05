#pragma once

#include "CConvert.h"

class CConvert_UuDecode : public CConvert{
public:
	bool DoConvert(CNativeW2* pcData);


	// uudecode(デコード)
	static void UUDECODE(
		const CNativeW2&	pcSrc,
		CMemory*			pcDst,
		TCHAR*				pszFileName
	);
};
