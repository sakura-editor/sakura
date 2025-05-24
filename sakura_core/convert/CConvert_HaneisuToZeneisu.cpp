/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "convert_util.h"
#include "mem/CNativeW.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!半角英数→全角英数
bool CConvert_HaneisuToZeneisu::DoConvert(CNativeW* pcData)
{
	//半角英数→全角英数
	Convert_HaneisuToZeneisu(pcData->GetStringPtr(), pcData->GetStringLength());

	return true;
}
