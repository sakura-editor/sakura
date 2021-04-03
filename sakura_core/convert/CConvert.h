﻿/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CCONVERT_781CEC40_5400_4D47_959B_0718AEA82A9B_H_
#define SAKURA_CCONVERT_781CEC40_5400_4D47_959B_0718AEA82A9B_H_
#pragma once

//2007.10.02 kobake CEditViewから分離

#include "CSelectLang.h"
#include "Funccode_enum.h"	// EFunctionCode
#include "String_define.h"
#include "basis/SakuraBasis.h"
#include "debug/Debug2.h"
#include "util/MessageBoxF.h"

class CNativeW;

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
#endif /* SAKURA_CCONVERT_781CEC40_5400_4D47_959B_0718AEA82A9B_H_ */
