#include "stdafx.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ƒCƒ“ƒ^[ƒtƒF[ƒX                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!‘SŠp‰p”¨”¼Šp‰p”
bool CConvert_ZeneisuToHaneisu::DoConvert(CNativeW* pcData)
{
	//‘SŠp‰p”¨”¼Šp‰p”
	Convert_ZeneisuToHaneisu(pcData->GetStringPtr(), pcData->GetStringLength());

	return true;
}
