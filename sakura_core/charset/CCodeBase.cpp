#include "stdafx.h"
#include "CCodeBase.h"
#include "charcode.h"

void CCodeBase::GetBom(CMemory* pcmemBom){ pcmemBom->Clear(); }					//!< BOMデータ取得

// 表示用16表示	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CCodeBase::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	if (WCODE::IsUTF16High(cSrc[0]) && iSLen >= 2 && WCODE::IsUTF16Low(cSrc[1])) {
		// サロゲートペア
		if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispSPCodepoint) {
			auto_sprintf( pDst, _T("%05x"), 0x10000 + ((cSrc[0] & 0x3FF)<<10) + (cSrc[1] & 0x3FF));
		}
		else {
			auto_sprintf( pDst, _T("%04x%04x"), cSrc[0], cSrc[1]);
		}
	}
	else {
		auto_sprintf( pDst, _T("%04x"), cSrc[0]);
	}

	return RESULT_COMPLETE;
}
