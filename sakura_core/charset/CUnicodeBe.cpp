//	この行は文字化け対策用です．消さないでください

#include "stdafx.h"
#include "CUnicodeBe.h"

//! UnicodeBE→Unicodeコード変換
// 2007.08.13 kobake 作成
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
