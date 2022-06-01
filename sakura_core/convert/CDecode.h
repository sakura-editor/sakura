﻿/*!	@file
	@brief デコーダーのインターフェース

	@author
*/
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CDECODE_2E6ED248_B211_48AB_BD69_032199189323_H_
#define SAKURA_CDECODE_2E6ED248_B211_48AB_BD69_032199189323_H_
#pragma once

#include "CSelectLang.h"
#include "String_define.h"
#include "mem/CNativeW.h"

class CDecode {
public:
	virtual ~CDecode() { }

	//インターフェース
	bool CallDecode( const CNativeW& pcData, CMemory* pDest )
	{
		bool bRet=DoDecode(pcData, pDest);
		if(!bRet){
			ErrorMessage(NULL,LS(STR_CONVERT_ERR));
			pDest->SetRawData( "", 0 );
			return false;
		}
		return true;
	}

	//実装
	virtual bool DoDecode( const CNativeW& pcData, CMemory* pDest )=0;
};
#endif /* SAKURA_CDECODE_2E6ED248_B211_48AB_BD69_032199189323_H_ */
