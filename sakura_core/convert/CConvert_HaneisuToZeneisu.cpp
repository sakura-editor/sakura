#include "stdafx.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!���p�p�����S�p�p��
bool CConvert_HaneisuToZeneisu::DoConvert(CNativeW* pcData)
{
	//���p�p�����S�p�p��
	Convert_HanEisuu_To_ZenEisuu(pcData->GetStringPtr(), pcData->GetStringLength());

	return true;
}
