﻿/*!	@file
	@brief CDicMgrクラス定義

	@author Norio Nakatani
	@date	1998/11/05 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDICMGR_70378DC4_733D_4F64_8D2A_E23C548E19EB_H_
#define SAKURA_CDICMGR_70378DC4_733D_4F64_8D2A_E23C548E19EB_H_
#pragma once

#include <Windows.h>
#include "util/container.h"
#include "_main/global.h"

class CNativeW;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDicMgr
{
public:
	/*
	||  Constructors
	*/
	CDicMgr();

	/*
	||  Attributes & Operations
	*/
//	BOOL Open( char* );
	static BOOL Search( const wchar_t* pszKey, const int nCmpLen,
						CNativeW** ppcmemKey, CNativeW** ppcmemMean,
						const WCHAR* pszKeyWordHelpFile, int * pLine );	// 2006.04.10 fon (const int,CMemory**,int*)引数を追加
	static int HokanSearch( const wchar_t* pszKey, bool bHokanLoHiCase,
							vector_ex<std::wstring>& vKouho, int nMaxKouho,
							const WCHAR* pszKeyWordFile );
//	BOOL Close( char* );

protected:
	/*
	||  実装ヘルパ関数
	*/
};
#endif /* SAKURA_CDICMGR_70378DC4_733D_4F64_8D2A_E23C548E19EB_H_ */
