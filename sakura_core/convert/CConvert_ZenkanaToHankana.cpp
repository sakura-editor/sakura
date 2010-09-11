#include "stdafx.h"
#include "CConvert_ZenkanaToHankana.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!�S�p�J�i�����p�J�i
bool CConvert_ZenkanaToHankana::DoConvert(CNativeW* pcData)
{
	//�S�p�J�^�J�i�����p�J�^�J�i
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()*2+1]; //���_���̉e���ŁA�ő�2�{�ɂ܂Ŗc��オ��\��������̂ŁA2�{�̃o�b�t�@���m��
	int nBufLen = 0;
	Convert_ZenKana_To_HanKana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nBufLen);
	pcData->SetString(pBuf, nBufLen);
	delete[] pBuf;

	return true;
}
