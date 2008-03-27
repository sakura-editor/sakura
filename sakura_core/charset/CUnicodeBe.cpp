//	この行は文字化け対策用です．消さないでください

#include "stdafx.h"
#include "CUnicodeBe.h"
#include "CEol.h"

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

void CUnicodeBe::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF16BE_BOM[]={0xFE,0xFF};
	pcmemBom->SetRawData(UTF16BE_BOM, sizeof(UTF16BE_BOM));
}

void CUnicodeBe::GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const void* pData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		"",					0 * sizeof(wchar_t),	// EOL_NONE
		"\x00\x0d\x00\x0a",	2 * sizeof(wchar_t),	// EOL_CRLF
		"\x00\x0a",			1 * sizeof(wchar_t),	// EOL_LF
		"\x00\x0d",			1 * sizeof(wchar_t),	// EOL_CR
	};
	pcmemEol->SetRawData(aEolTable[eEolType].pData,aEolTable[eEolType].nLen);
}



/*
	行端子の種類を調べるUnicodeBE版
	@param pszData 調査対象文字列へのポインタ
	@param nDataLen 調査対象文字列の長さ(wchar_tの長さ)
	@return 改行コードの種類。終端子が見つからなかったときはEOL_NONEを返す。
*/
static EEolType GetEOLTypeUniBE( const wchar_t* pszData, int nDataLen )
{
	/*! 行終端子のデータの配列(UnicodeBE版) 2000.05.30 Moca */
	static const wchar_t* aEolTable[EOL_TYPE_NUM] = {
		L"",									// EOL_NONE
		(const wchar_t*)"\x00\x0d\x00\x0a\x00",	// EOL_CRLF
		(const wchar_t*)"\x00\x0a\x00",			// EOL_LF
		(const wchar_t*)"\x00\x0d\x00"			// EOL_CR
	};

	/* 改行コードの長さを調べる */

	for( int i = 1; i < EOL_TYPE_NUM; ++i ){
		CEol cEol((EEolType)i);
		if( cEol.GetLen()<=nDataLen && 0==auto_memcmp(pszData,aEolTable[i],cEol.GetLen()) ){
			return gm_pnEolTypeArr[i];
		}
	}
	return EOL_NONE;
}

/*!
	GetNextLineのwchar_t版(ビックエンディアン用)
	GetNextLineより作成
	static メンバ関数
*/
const wchar_t* GetNextLineWB(
	const wchar_t*	pData,	//!< [in]	検索文字列
	int			nDataLen,	//!< [in]	検索文字列の文字数
	int*		pnLineLen,	//!< [out]	1行の文字数を返すただしEOLは含まない
	int*		pnBgn,		//!< [i/o]	検索文字列のオフセット位置
	CEol*		pcEol		//!< [i/o]	EOL
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		// 改行コードがあった
		if( pData[i] == (wchar_t)0x0a00 || pData[i] == (wchar_t)0x0d00 ){
			// 行終端子の種類を調べる
			pcEol->SetType( GetEOLTypeUniBE( &pData[i], nDataLen - i ) );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}

