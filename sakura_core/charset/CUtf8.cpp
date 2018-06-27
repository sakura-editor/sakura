// 2008.11.10 変換ロジックを書き直す

#include "StdAfx.h"
#include "CUtf8.h"
#include "charset/codechecker.h"

// 非依存推奨
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

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
int CUtf8::Utf8ToUni( const char* pSrc, const int nSrcLen, wchar_t* pDst, bool bCESU8Mode )
{
	const unsigned char *pr, *pr_end;
	unsigned short *pw;
	int nclen;
	ECharSet echarset;

	if( nSrcLen < 1 ){
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc+nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for( ; ; ){

		// 文字をチェック
		if( bCESU8Mode != true ){
			nclen = CheckUtf8Char( reinterpret_cast<const char*>(pr), pr_end-pr, &echarset, true, 0 );
		}else{
			nclen = CheckCesu8Char( reinterpret_cast<const char*>(pr), pr_end-pr, &echarset, 0 );
		}
		if( nclen < 1 ){
			break;
		}

		// 変換
		if( echarset != CHARSET_BINARY ){
			pw += _Utf8ToUni_char( pr, nclen, pw, bCESU8Mode );
			pr += nclen;
		}else{
			if( nclen != 1 ){	// 保護コード
				nclen = 1;
			}
			pw += BinToText( pr, 1, pw );
			++pr;
		}
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



//! UTF-8→Unicodeコード変換
// 2007.08.13 kobake 作成
EConvertResult CUtf8::_UTF8ToUnicode( const CMemory& cSrc, CNativeW* pDstMem, bool bCESU8Mode/*, bool decodeMime*/ )
{
	// エラー状態
	bool bError = false;

	// データ取得
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( cSrc.GetRawPtr(&nSrcLen) );
 
	const char* psrc = pSrc;
	int nsrclen = nSrcLen;

//	CMemory cmem;
//	// MIME ヘッダーデコード
//	if( decodeMime == true ){
//		bool bret = MIMEHeaderDecode( pSrc, nSrcLen, &cmem, CODE_UTF8 );
//		if( bret == true ){
//			psrc = reinterpret_cast<char*>( cmem.GetRawPtr() );
//			nsrclen = cmem.GetRawLength();
//		}
//	}

	// 必要なバッファサイズを調べて確保する
	wchar_t* pDst;
	try{
		pDst = new wchar_t[nsrclen];
	}catch( ... ){
		pDst = NULL;
	}
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// 変換
	int nDstLen = Utf8ToUni( psrc, nsrclen, pDst, bCESU8Mode );

	// pDstMem を更新
	pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

	// 後始末
	delete [] pDst;

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
	char* pDst;
	try{
		pDst = new char[nSrcLen * 3];
	}catch( ... ){
		pDst = NULL;
	}
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
EConvertResult CUtf8::_UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar, const bool bCESUMode)
{
	CNativeW		cBuff;
	EConvertResult	res;
	int				i;
	TCHAR*			pd;
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
			auto_sprintf( pd, _T("%02X"), *ps);
		}
	}else{
		auto_sprintf( pd, _T("?%02X"), *ps );
	}

	return RESULT_COMPLETE;
}
