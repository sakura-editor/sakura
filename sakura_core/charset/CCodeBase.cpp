// この行は文字化け対策のためのものです。
#include "StdAfx.h"
#include "CCodeBase.h"
#include "charcode.h"
#include "convert/convert_util2.h"
#include "charset/codechecker.h"
#include "CEol.h"

// 非依存推奨
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

void CCodeBase::GetBom(CMemory* pcmemBom){ pcmemBom->Clear(); }					//!< BOMデータ取得

// 表示用16表示	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CCodeBase::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar)
{
	if (IsUTF16High(cSrc[0]) && iSLen >= 2 && IsUTF16Low(cSrc[1])) {
		// サロゲートペア
		if (psStatusbar->m_bDispSPCodepoint) {
			auto_sprintf( pDst, _T("U+%05X"), 0x10000 + ((cSrc[0] & 0x3FF)<<10) + (cSrc[1] & 0x3FF));
		}
		else {
			auto_sprintf( pDst, _T("%04X%04X"), cSrc[0], cSrc[1]);
		}
	}
	else {
		auto_sprintf( pDst, _T("U+%04X"), cSrc[0] );
	}

	return RESULT_COMPLETE;
}









/*!
	MIME デコーダー

	@param[out] pcMem デコード済みの文字列を格納
*/
bool CCodeBase::MIMEHeaderDecode( const char* pSrc, const int nSrcLen, CMemory* pcMem, const ECodeType eCodetype )
{
	ECodeType ecodetype;
	int nskip_bytes;

	// ソースを取得
	pcMem->AllocBuffer( nSrcLen );
	char* pdst = reinterpret_cast<char*>( pcMem->GetRawPtr() );
	if( pdst == NULL ){
		pcMem->SetRawData( "", 0 );
		return false;
	}

	CMemory cmembuf;
	int i = 0;
	int j = 0;
	while( i < nSrcLen ){
		if( pSrc[i] != '=' ){
			pdst[j] = pSrc[i];
			++i;
			++j;
			continue;
		}
		nskip_bytes = _DecodeMimeHeader( &pSrc[i], nSrcLen-i, &cmembuf, &ecodetype );
		if( nskip_bytes < 1 ){
			pdst[j] = pSrc[i];
			++i;
			++j;
		}else{
			if( ecodetype == eCodetype ){
				// eChartype が ecodetype と一致している場合にだけ、
				// 変換結果をコピー
				memcpy( &pdst[j], cmembuf.GetRawPtr(), cmembuf.GetRawLength() );
				i += nskip_bytes;
				j += cmembuf.GetRawLength();
			}else{
				memcpy( &pdst[j], &pSrc[i], nskip_bytes );
				i += nskip_bytes;
				j += nskip_bytes;
			}
		}
	}

	pcMem->_SetRawLength( j );
	return true;
}

/*!
	改行データ取得
*/
// CShiftJisより移動 2010/6/13 Uchi
void CCodeBase::S_GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const char* szData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		{ "",			0 },	// EOL_NONE
		{ "\x0d\x0a",	2 },	// EOL_CRLF
		{ "\x0a",		1 },	// EOL_LF
		{ "\x0d",		1 },	// EOL_CR
		{ "",			0 },	// EOL_NEL
		{ "",			0 },	// EOL_LS
		{ "",			0 },	// EOL_PS
	};
	pcmemEol->SetRawData(aEolTable[eEolType].szData,aEolTable[eEolType].nLen);
}
