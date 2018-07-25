#include "StdAfx.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "convert_util.h"

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
