#include "StdAfx.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "convert_util.h"

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
