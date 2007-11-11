#include "stdafx.h"
#include "CUnicodeBe.h"

//! UnicodeBE¨UnicodeƒR[ƒh•ÏŠ·
// 2007.08.13 kobake ì¬
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
