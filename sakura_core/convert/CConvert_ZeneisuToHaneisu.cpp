#include "stdafx.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!�S�p�p�������p�p��
bool CConvert_ZeneisuToHaneisu::DoConvert(CNativeW* pcData)
{
	//�S�p�p�������p�p��
	Convert_ZenEisuu_To_HanEisuu(pcData->GetStringPtr(), pcData->GetStringLength());

	return true;
}
