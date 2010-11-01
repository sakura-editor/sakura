#include "stdafx.h"
#include "CConvert_ToZenkata.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!�ł������S�p�J�^�J�i�ɂ���
bool CConvert_ToZenkata::DoConvert(CNativeW* pcData)
{
	//���p�J�i���S�p�J�i
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //�����������邱�Ƃ͂����Ă������邱�Ƃ͖����̂ŁA����łn�j
	int nBufLen = 0;
	Convert_HankataToZenkata(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nBufLen);

	//�S�p�Ђ灨�S�p�J�i
	Convert_ZenhiraToZenkata(pBuf, nBufLen);

	//���p�p�����S�p�p��
	Convert_HaneisuToZeneisu(pBuf, nBufLen);

	//�ݒ�
	pcData->SetString(pBuf, nBufLen);

	//�o�b�t�@���
	delete[] pBuf;

	return true;
}
