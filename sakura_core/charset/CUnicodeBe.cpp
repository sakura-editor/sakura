#include "stdafx.h"
#include "CUnicodeBe.h"

//! UnicodeBE��Unicode�R�[�h�ϊ�
// 2007.08.13 kobake �쐬
EConvertResult CUnicodeBe::UnicodeBEToUnicode( CMemory* pMem )
{
	pMem->SwapHLByte();
	return RESULT_COMPLETE;
}

EConvertResult CUnicodeBe::UnicodeToUnicodeBE( CMemory* pMem )
{
	pMem->SwapHLByte();
	return RESULT_COMPLETE;
}
