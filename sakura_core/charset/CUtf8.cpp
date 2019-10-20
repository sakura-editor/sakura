/*! @file */
// 2008.11.10 変換ロジックを書き直す

#include "StdAfx.h"
#include "CUtf8.h"
#include "charset/codechecker.h"

// 非依存推奨
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

#if defined(_M_X64) || defined(_M_IX86)
#include <intrin.h>
#include "util/x86_x64_instruction_set.h"
#endif

//! BOMデータ取得
void CUtf8::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
	pcmemBom->SetRawData(UTF8_BOM, sizeof(UTF8_BOM));
}

void CUtf8::GetEol(CMemory* pcmemEol, EEolType eEolType){
	static const struct{
		const char* szData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		"",			0,	// EOL_NONE
		"\x0d\x0a",	2,	// EOL_CRLF
		"\x0a",		1,	// EOL_LF
		"\x0d",		1,	// EOL_CR
		"\xc2\x85",			2,	// EOL_NEL
		"\xe2\x80\xa8",		3,	// EOL_LS
		"\xe2\x80\xa9",		3,	// EOL_PS
	};
	pcmemEol->SetRawData(aEolTable[eEolType].szData,aEolTable[eEolType].nLen);
}

/*!
	UTF-8 → Unicode 実装

	@param[in] bCESU8Mode CESU-8 を処理する場合 true
*/
int CUtf8::Utf8ToUni( const char* pSrc, const int nSrcLen, wchar_t* pDst, bool bCESU8Mode, bool bOnlyASCII )
{
	const unsigned char * __restrict pr, * __restrict pr_end;
	unsigned short * __restrict pw;
	int nclen;
	ECharSet echarset;

	if( nSrcLen < 1 ){
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc+nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	if (bOnlyASCII) {
		// ASCII文字列をWIDE文字列化
		int i = 0;
#if defined(_M_X64) || defined(_M_IX86)
		// 1文字ずつ処理すると時間が掛かるのでSIMD使用
		if (InstructionSet::getInstance()->AVX2()) {
			int n16 = nSrcLen / 16;
			for (; i < n16; ++i) {
				__m128i r = _mm_loadu_si128((const __m128i*)pr); pr += 16;
				__m256i w = _mm256_cvtepu8_epi16(r);
				_mm256_storeu_si256((__m256i*)pw, w); pw += 16;
			}
			i = n16 * 16;
#if defined(_M_X64)
		}else {
#else
		}else if (InstructionSet::SSE2()) {
#endif
			int n16 = nSrcLen / 16;
			__m128i zero = _mm_setzero_si128();
			for (; i < n16; ++i) {
				__m128i r = _mm_loadu_si128((const __m128i*)pr); pr += 16;
				__m128i wl = _mm_unpacklo_epi8(r, zero);
				__m128i wh = _mm_unpackhi_epi8(r, zero);
				_mm_storeu_si128((__m128i*)pw, wl); pw += 8;
				_mm_storeu_si128((__m128i*)pw, wh); pw += 8;
			}
			i = n16 * 16;
		}
#endif // #if defined(_M_X64) || defined(_M_IX86)
		// 余りはスカラー処理
		for (; i < nSrcLen; ++i) {
			pw[i] = pr[i];
		}
		return nSrcLen;
	} // if (bOnlyASCII) 

	if (bCESU8Mode) {
		ptrdiff_t remainLen = pr_end - pr;
		while (remainLen > 0){
			// 文字をチェック
			nclen = CheckCesu8Char( reinterpret_cast<const char*>(pr), remainLen, echarset );
			// 変換
			if( echarset != CHARSET_BINARY ){
				pw += _Utf8ToUni_char_CESUMode( pr, nclen, pw );
				pr += nclen;
			}else{
				if( nclen != 1 ){	// 保護コード
					nclen = 1;
				}
				pw += BinToText( pr, 1, pw );
				++pr;
			}
			remainLen = pr_end - pr;
		}
	}else {
		int i = 0;
		while (i < nSrcLen && (pw[i] = pr[i]) < 0x80) {
			++i;
		}
		pw += i;
		pr += i;
		while (pr < pr_end) {
			// 文字をチェック
			nclen = CheckUtf8Char( reinterpret_cast<const char*>(pr), pr_end - pr, echarset, true );
			// 変換
			if( echarset != CHARSET_BINARY ){
				pw += _Utf8ToUni_char( pr, nclen, pw );
				pr += nclen;
			}else{
				if( nclen != 1 ){	// 保護コード
					nclen = 1;
				}
				pw += BinToText( pr, 1, pw );
				++pr;
			}
		}
	}
	return pw - reinterpret_cast<unsigned short*>(pDst);
}

//! UTF-8→Unicodeコード変換
// 2007.08.13 kobake 作成
EConvertResult CUtf8::_UTF8ToUnicode( const CMemory& cSrc, CNativeW* pDstMem, bool bCESU8Mode, bool bOnlyASCII )
{
	// エラー状態
	bool bError = false;

	// データ取得
	int nSrcLen = cSrc.GetRawLength();
	const char* pSrc = reinterpret_cast<const char*>( cSrc.GetRawPtr() );

	if( &cSrc == pDstMem->_GetMemory() )
	{
		// 必要なバッファサイズを調べて確保する
		wchar_t* pDst = new (std::nothrow) wchar_t[nSrcLen];
		if( pDst == NULL ){
			return RESULT_FAILURE;
		}

		// 変換
		int nDstLen = Utf8ToUni( pSrc, nSrcLen, pDst, bCESU8Mode, bOnlyASCII );

		// pDstMem を更新
		pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

		// 後始末
		delete [] pDst;
	}
	else
	{
		// 変換先バッファサイズを設定してメモリ領域確保
		pDstMem->AllocStringBuffer( nSrcLen + 1 );
		wchar_t* pDst = pDstMem->GetStringPtr();

		// 変換
		size_t nDstLen = Utf8ToUni(pSrc, nSrcLen, pDst, bCESU8Mode, bOnlyASCII);

		// pDstMem を更新
		pDstMem->_SetStringLength( nDstLen );
	}

	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}

/*!
	Unicode -> UTF-8 実装

	@param[in] bCESU8Mode CESU-8 を処理する場合 true
*/
int CUtf8::UniToUtf8( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool* pbError, bool bCESU8Mode )
{
	const unsigned short* pr = reinterpret_cast<const unsigned short*>(pSrc);
	const unsigned short* pr_end = reinterpret_cast<const unsigned short*>(pSrc+nSrcLen);
	unsigned char* pw = reinterpret_cast<unsigned char*>(pDst);
	int nclen;
	bool berror=false;
	ECharSet echarset;

	while( (nclen = CheckUtf16leChar(reinterpret_cast<const wchar_t*>(pr), pr_end-pr, &echarset, 0)) > 0 ){
		// 保護コード
		switch( echarset ){
		case CHARSET_UNI_NORMAL:
			nclen = 1;
			break;
		case CHARSET_UNI_SURROG:
			nclen = 2;
			break;
		default:
			echarset = CHARSET_BINARY;
			nclen = 1;
		}
		if( echarset != CHARSET_BINARY ){
			pw += _UniToUtf8_char( pr, nclen, pw, bCESU8Mode );
			pr += nclen;
		}else{
			if(  nclen == 1 && IsBinaryOnSurrogate(static_cast<wchar_t>(*pr)) ){
				*pw = static_cast<unsigned char>( TextToBin(*pr) & 0x000000ff );
				++pw;
			}else{
				berror = true;
				*pw = '?';
				++pw;
			}
			++pr;
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned char*>(pDst);
}

//! コード変換 Unicode→UTF-8
EConvertResult CUtf8::_UnicodeToUTF8( const CNativeW& cSrc, CMemory* pDstMem, bool bCesu8Mode )
{
	// エラー状態
	bool bError = false;

	// ソースを取得
	const wchar_t* pSrc = cSrc.GetStringPtr();
	int nSrcLen = cSrc.GetStringLength();

	// 必要なバッファサイズを調べてメモリを確保
	char* pDst = new (std::nothrow) char[nSrcLen * 3];
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// 変換
	int nDstLen = UniToUtf8( pSrc, nSrcLen, pDst, &bError, bCesu8Mode );

	// pDstMem を更新
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	// 後始末
	delete [] pDst;

	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}

// 文字コード表示用	UNICODE → Hex 変換	2008/6/21 Uchi
EConvertResult CUtf8::_UnicodeToHex(const wchar_t* cSrc, const int iSLen, WCHAR* pDst, const CommonSetting_Statusbar* psStatusbar, const bool bCESUMode)
{
	CNativeW		cBuff;
	EConvertResult	res;
	int				i;
	WCHAR*			pd;
	unsigned char*	ps;
	bool			bbinary=false;

	if (psStatusbar->m_bDispUtf8Codepoint) {
		// Unicodeで表示
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}
	cBuff.AllocStringBuffer(4);
	// 1文字データバッファ
	if (IsUTF16High(cSrc[0]) && iSLen >= 2 && IsUTF16Low(cSrc[1])) {
		cBuff._GetMemory()->SetRawDataHoldBuffer(cSrc, 4);
	}
	else {
		cBuff._GetMemory()->SetRawDataHoldBuffer(cSrc, 2);
		if( IsBinaryOnSurrogate(cSrc[0]) ){
			bbinary = true;
		}
	}

	// UTF-8/CESU-8 変換
	if (bCESUMode != true) {
		res = UnicodeToUTF8(cBuff, cBuff._GetMemory());
	}
	else {
		res = UnicodeToCESU8(cBuff, cBuff._GetMemory());
	}
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex変換
	ps = reinterpret_cast<unsigned char*>( cBuff._GetMemory()->GetRawPtr() );
	pd = pDst;
	if( bbinary == false ){
		for (i = cBuff._GetMemory()->GetRawLength(); i >0; i--, ps ++, pd += 2) {
			auto_sprintf( pd, L"%02X", *ps);
		}
	}else{
		auto_sprintf( pd, L"?%02X", *ps );
	}

	return RESULT_COMPLETE;
}
