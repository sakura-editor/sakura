#include "stdafx.h"
#include "CConvert.h"
#include "Funccode.h"
#include "CEol.h"
#include "charset/charcode.h"
#include "CConvert_ToLower.h"
#include "CConvert_ToUpper.h"
#include "CConvert_ToHankaku.h"
#include "CConvert_TabToSpace.h"
#include "CConvert_SpaceToTab.h"
#include "CConvert_ZenkanaToHankana.h"
#include "CConvert_ZeneisuToHaneisu.h"
#include "CConvert_HaneisuToZeneisu.h"
#include "CConvert_HankanaToZenkana.h"
#include "CConvert_HankanaToZenhira.h"
#include "CConvert_ToZenhira.h"
#include "CConvert_ToZenkana.h"
#include "CConvert_Trim.h"

/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
void CConvertMediator::ConvMemory( CNativeW* pCMemory, EFunctionCode nFuncCode, int nTabWidth )
{
	switch( nFuncCode ){
	//������ϊ��A���`
	case F_TOLOWER:					CConvert_ToLower().CallConvert(pCMemory);			break;	// ������
	case F_TOUPPER:					CConvert_ToUpper().CallConvert(pCMemory);			break;	// �啶��
	case F_TOHANKAKU:				CConvert_ToHankaku().CallConvert(pCMemory);			break;	// �S�p�����p
	case F_TOHANKATA:				CConvert_ZenkanaToHankana().CallConvert(pCMemory);	break;	// �S�p�J�^�J�i�����p�J�^�J�i
	case F_TOZENEI:					CConvert_HaneisuToZeneisu().CallConvert(pCMemory);	break;	// ���p�p�����S�p�p��
	case F_TOHANEI:					CConvert_ZeneisuToHaneisu().CallConvert(pCMemory);	break;	// �S�p�p�������p�p��
	case F_TOZENKAKUKATA:			CConvert_ToZenkana().CallConvert(pCMemory);			break;	// ���p�{�S�Ђ灨�S�p�E�J�^�J�i
	case F_TOZENKAKUHIRA:			CConvert_ToZenhira().CallConvert(pCMemory);			break;	// ���p�{�S�J�^���S�p�E�Ђ炪��
	case F_HANKATATOZENKATA:	CConvert_HankanaToZenkana().CallConvert(pCMemory);	break;	// ���p�J�^�J�i���S�p�J�^�J�i
	case F_HANKATATOZENHIRA:	CConvert_HankanaToZenhira().CallConvert(pCMemory);	break;	// ���p�J�^�J�i���S�p�Ђ炪��
	//������ϊ��A���`
	case F_TABTOSPACE:				CConvert_TabToSpace(nTabWidth).CallConvert(pCMemory);break;	// TAB����
	case F_SPACETOTAB:				CConvert_SpaceToTab(nTabWidth).CallConvert(pCMemory);break;	// �󔒁�TAB
	case F_LTRIM:					CConvert_Trim(true).CallConvert(pCMemory);			break;	// 2001.12.03 hor
	case F_RTRIM:					CConvert_Trim(false).CallConvert(pCMemory);			break;	// 2001.12.03 hor
	}
	return;
}


