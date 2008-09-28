#include "stdafx.h"
#include "CUtf8.h"

//! BOMデータ取得
void CUtf8::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF8_BOM[]={0xEF,0xBB,0xBF};
	pcmemBom->SetRawData(UTF8_BOM, sizeof(UTF8_BOM));
}

//! UTF-8→Unicodeコード変換
// 2007.08.13 kobake 作成
EConvertResult CUtf8::UTF8ToUnicode( const CMemory& cSrcMem, CNativeW* pDstMem )
{
	//データ取得
	int nSrcLen;
	const char* pData = (char*)cSrcMem.GetRawPtr(&nSrcLen);

	//必要なバッファサイズを調べる
	size_t nDstLen=MultiByteToWideChar(
		CP_UTF8,
		0,
		pData,
		nSrcLen,
		NULL,
		0
	);

	//バッファ確保
	pDstMem->Clear();
	pDstMem->AllocStringBuffer(nDstLen+1);
	wchar_t* pDst = pDstMem->GetStringPtr();

	//変換
	nDstLen=MultiByteToWideChar(
		CP_UTF8,
		0,
		pData,
		nSrcLen,
		pDst,
		nDstLen
	);
	pDst[nDstLen]=L'\0';

	//CNativeW更新
	pDstMem->_SetStringLength(nDstLen);

	return RESULT_COMPLETE; //大丈夫だと思う
}


//! コード変換 Unicode→UTF-8
EConvertResult CUtf8::UnicodeToUTF8( const CNativeW& cSrcMem, CMemory* pDstMem )
{
	//データ取得
	int nSrcLen;
	const wchar_t* pSrc = cSrcMem.GetStringPtr(&nSrcLen);

	//必要なバッファサイズを調べる
	size_t nDstLen=WideCharToMultiByte(
		CP_UTF8,
		0,
		pSrc,
		nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	//バッファ確保
	pDstMem->Clean();
	pDstMem->AllocBuffer( (nDstLen+1) * sizeof(char) );
	char* pDst = reinterpret_cast<char*>(pDstMem->GetRawPtr());

	//変換
	nDstLen=WideCharToMultiByte(
		CP_UTF8,
		0,
		pSrc,
		nSrcLen,
		pDst,
		nDstLen,
		NULL,
		NULL
	);
	pDst[nDstLen]='\0';

	//CMemory更新
	pDstMem->_SetRawLength(nDstLen);

	return RESULT_COMPLETE; //大丈夫だと思う
}

// 文字コード表示用	UNICODE → Hex 変換	2008/6/21 Uchi
EConvertResult CUtf8::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	static CNativeW	cBuffSrc;
	static CMemory	cBuffDst;
	EConvertResult	res;
	int				i;
	TCHAR*			pd; 
	unsigned char*	ps; 

	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUtf8Codepoint) {
		// Unicodeで表示
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	// 1文字データバッファ
	if (WCODE::IsUTF16High(cSrc[0]) && iSLen >= 2 && WCODE::IsUTF16Low(cSrc[1])) {
		cBuffSrc.SetStringW(cSrc,2);
	}
	else {
		cBuffSrc.SetStringW(cSrc,1);
	}
	cBuffDst.SetRawData("",0);

	// RTF-8 変換
	res = UnicodeToUTF8(cBuffSrc, &cBuffDst);
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex変換
	for (i = cBuffDst.GetRawLength(), ps = (unsigned char*)cBuffDst.GetRawPtr(), pd = pDst; i >0; i--, ps ++, pd += 2) {
		auto_sprintf( pd, _T("%02x"), *ps);
	}

	return RESULT_COMPLETE;
}
