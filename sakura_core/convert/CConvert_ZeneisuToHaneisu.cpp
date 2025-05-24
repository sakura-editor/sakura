/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "convert_util.h"
#include "mem/CNativeW.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!全角英数→半角英数
bool CConvert_ZeneisuToHaneisu::DoConvert(CNativeW* pcData)
{
	//全角英数→半角英数
	Convert_ZeneisuToHaneisu(pcData->GetStringPtr(), pcData->GetStringLength());

	return true;
}
