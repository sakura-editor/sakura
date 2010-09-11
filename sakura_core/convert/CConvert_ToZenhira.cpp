#include "stdafx.h"
#include "CConvert_ToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!�ł������S�p�Ђ炪�Ȃɂ���
bool CConvert_ToZenhira::DoConvert(CNativeW* pcData)
{
	//���J�i���S�p�J�i
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //�����������邱�Ƃ͂����Ă������邱�Ƃ͖����̂ŁA����łn�j
	int nBufLen = 0;
	Convert_HankanaToZenkana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nBufLen);

	//�S�J�i���S�p�Ђ炪��
	Convert_ZenkanaToZenhira(pBuf, nBufLen);

	//���p�p�����S�p�p��
	Convert_HanEisuu_To_ZenEisuu(pBuf, nBufLen);

	//�ݒ�
	pcData->SetString(pBuf, nBufLen);

	//�o�b�t�@���
	delete[] pBuf;

	return true;
}
