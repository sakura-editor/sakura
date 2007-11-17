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
	Convert_HankanaToZenkana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf);

	//�S�p�J�i���S�p�Ђ炪��
	Convert_ZenkanaToZenhira(pBuf, wcslen(pBuf));

	//�ݒ�
	pcData->SetString(pBuf);

	//�o�b�t�@���
	delete[] pBuf;

	return true;
}
