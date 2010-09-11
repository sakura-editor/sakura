#include "stdafx.h"
#include "CConvert_ToHankaku.h"
#include "convert_util.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!���p�ɂł�����̂͑S�����p�ɕϊ�
bool CConvert_ToHankaku::DoConvert(CNativeW* pcData)
{
	//�S�p�Ђ炪�ȁ��S�p�J�^�J�i
	Convert_ZenhiraToZenkana(pcData->GetStringPtr(), pcData->GetStringLength());

	//�S�p�p�������p�p��
	Convert_ZenEisuu_To_HanEisuu(pcData->GetStringPtr(), pcData->GetStringLength());

	//�S�p�J�^�J�i�����p�J�^�J�i
	wchar_t* pBuf = new wchar_t[pcData->GetStringLength()*2+1]; //���_���̉e���ŁA�ő�2�{�ɂ܂Ŗc��オ��\��������̂ŁA2�{�̃o�b�t�@���m��
	int nDstLen = 0;
	Convert_ZenKana_To_HanKana(pcData->GetStringPtr(), pcData->GetStringLength(), pBuf, &nDstLen);
	pcData->SetString(pBuf, nDstLen);
	delete[] pBuf;

	return true;
}
