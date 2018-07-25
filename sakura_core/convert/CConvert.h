/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_CCONVERT_BF272748_9EF0_4F5A_952C_8BED216658F9_H_
#define SAKURA_CCONVERT_BF272748_9EF0_4F5A_952C_8BED216658F9_H_

//2007.10.02 kobake CEditViewから分離

#include "Funccode_enum.h"	// EFunctionCode

class CConvertMediator{
public:
	//! 機能種別によるバッファの変換
	static void ConvMemory( CNativeW* pCMemory, EFunctionCode nFuncCode, CKetaXInt nTabWidth, int nStartColumn );

protected:
	static void Command_TRIM2( CNativeW* pCMemory , BOOL bLeft );
};

class CConvert{
public:
	virtual ~CConvert(){}

	//インターフェース
	void CallConvert( CNativeW* pcData )
	{
		bool bRet=DoConvert(pcData);
		if(!bRet){
			ErrorMessage(NULL,LS(STR_CONVERT_ERR));
		}
	}

	//実装
	virtual bool DoConvert( CNativeW* pcData )=0;
};

#endif /* SAKURA_CCONVERT_BF272748_9EF0_4F5A_952C_8BED216658F9_H_ */
/*[EOF]*/
