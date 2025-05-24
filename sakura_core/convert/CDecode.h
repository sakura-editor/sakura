/*!	@file
	@brief デコーダーのインターフェース

	@author
*/
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
