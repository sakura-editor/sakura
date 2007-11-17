#include "stdafx.h"
#include "CConvert_ToZenkana.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!�ł������S�p�J�^�J�i�ɂ���
bool CConvert_ToZenkana::DoConvert(CNativeW* pcData)
{
	//���p�J�i���S�p�J�i
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()+1]; //�����������邱�Ƃ͂����Ă������邱�Ƃ͖����̂ŁA����łn�j
	Convert_HankanaToZenkana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf);

	//�S�p�Ђ灨�S�p�J�i
	Convert_ZenhiraToZenkana(pBuf, wcslen(pBuf));

	//�ݒ�
	pcData->SetString(pBuf);

	//�o�b�t�@���
	delete[] pBuf;

	return true;
}
