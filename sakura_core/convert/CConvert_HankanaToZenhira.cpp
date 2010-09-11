#include "stdafx.h"
#include "CConvert_HankanaToZenhira.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!���p�J�i���S�p�Ђ炪��
bool CConvert_HankanaToZenhira::DoConvert(CNativeW* pcData)
{
	//���p�J�i���S�p�J�i
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //�����������邱�Ƃ͂����Ă������邱�Ƃ͖����̂ŁA����łn�j
	int nDstLen = 0;
	Convert_HankanaToZenkana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);

	//�S�p�J�i���S�p�Ђ炪��
	Convert_ZenkanaToZenhira(pBuf, nDstLen);

	//�ݒ�
	pcData->SetString(pBuf, nDstLen);

	//�o�b�t�@���
	delete[] pBuf;

	return true;
}
