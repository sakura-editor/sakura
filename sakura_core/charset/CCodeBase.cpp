// ���̍s�͕��������΍�̂��߂̂��̂ł��B
#include "stdafx.h"
#include "CCodeBase.h"
#include "charcode.h"

void CCodeBase::GetBom(CMemory* pcmemBom){ pcmemBom->Clear(); }					//!< BOM�f�[�^�擾

// �\���p16�\��	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
EConvertResult CCodeBase::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	if (WCODE::IsUTF16High(cSrc[0]) && iSLen >= 2 && WCODE::IsUTF16Low(cSrc[1])) {
		// �T���Q�[�g�y�A
		if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispSPCodepoint) {
			auto_sprintf( pDst, _T("u%05x"), 0x10000 + ((cSrc[0] & 0x3FF)<<10) + (cSrc[1] & 0x3FF));
		}
		else {
			auto_sprintf( pDst, _T("%04x%04x"), cSrc[0], cSrc[1]);
		}
	}
	else {
		auto_sprintf( pDst, _T("u%04x"), cSrc[0]);
	}

	return RESULT_COMPLETE;
}
