/*!	@file
	@brief ファイル読み込みクラス

	@author Moca
	@date 2002/08/30 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca, genta
	Copyright (C) 2003, Moca, ryoji
	Copyright (C) 2006, rastiv

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include <string.h>
#include "_main/global.h"
#include "mem/CMemory.h"
#include "CEol.h"
#include "io/CFileLoad.h"
#include "charset/charcode.h"
#include "io/CIoBridge.h"
#include "charset/CCodeFactory.h" ////
#include "charset/CCodePage.h"
#include "charset/CCodeMediator.h"
#include "util/string_ex2.h"
#include "charset/CESI.h"
#include "window/CEditWnd.h"

#if defined(_M_X64) || defined(_M_IX86)
#include <intrin.h>
#include "util/x86_x64_instruction_set.h"
#endif

/*! ロード用バッファサイズ */
constexpr size_t g_nBufSize = 1024 * 64;

bool CFileLoad::IsLoadableSize(ULONGLONG size, bool ignoreLimit)
{
	// 上限無視
	if (ignoreLimit)return true;

	// 判定
	return size < CFileLoad::GetLimitSize();
}

ULONGLONG CFileLoad::GetLimitSize()
{
#ifdef _WIN64
	// 64bit の場合
	// 実質上限は設けないこととする (x64 対応しながらここは要検討)
	return ULLONG_MAX;
#else
	// 32bit の場合
	// だいたい 2GB くらいを上限とする (既存コードがそうなっていたのでそれを踏襲)
	return 0x80000000;
#endif
}

//! 人にとって見やすいサイズ文字列を作る (例: "2 GB", "10 GB", "400 MB", "32 KB")
//  …と言いつつ、今のところは MB 単位での表示にします。適宜必要あれば改良してください。
std::wstring CFileLoad::GetSizeStringForHuman(ULONGLONG size)
{
	// bytes to megabytes
	ULONGLONG megabytes = size / 1024 / 1024;

	// to string
	wchar_t buf[32];
	swprintf_s(buf, _countof(buf), L"%I64u", megabytes);
	std::wstring str = buf;

	// https://stackoverflow.com/questions/7276826/c-format-number-with-commas
	// コンマ区切り文字列
	int insertPosition = str.length() - 3;
	while (insertPosition > 0) {
		str.insert(insertPosition, L",");
		insertPosition -= 3;
	}

	// 単位付けて返す
	return str + L" MB";
}

/*! コンストラクタ */
CFileLoad::CFileLoad( const SEncodingConfig& encode )
{
	m_pEencoding = &encode;

	m_hFile			= NULL;
	m_nFileSize		= 0;
	m_nFileDataLen	= 0;
	m_CharCode		= CODE_DEFAULT;
	m_pCodeBase		= NULL;////
	m_encodingTrait = ENCODING_TRAIT_ASCII;
	m_bBomExist		= false;	// Jun. 08, 2003 Moca
	m_nFlag 		= 0;
	m_nReadLength	= 0;
	m_eMode			= FLMODE_CLOSE;	// Jun. 08, 2003 Moca

	m_nLineIndex	= -1;

	m_pReadBuf = NULL;
	m_nReadDataLen    = 0;
	m_nReadBufOffSet  = 0;
}

/*! デストラクタ */
CFileLoad::~CFileLoad( void )
{
	if( NULL != m_hFile ){
		FileClose();
	}
	if( NULL != m_pReadBuf ){
		VirtualFree( m_pReadBuf, 0, MEM_RELEASE );
	}
	if( NULL != m_pCodeBase ){
		delete m_pCodeBase;
	}
}

/*!
	ファイルを開く
	@param pFileName [in] ファイル名
	@param bBigFile  [in] 2GB以上のファイルを開くか。Grep=true, 32bit版はその他=falseで運用
	@param CharCode  [in] ファイルの文字コード．
	@param nFlag [in] 文字コードのオプション
	@param pbBomExist [out] BOMの有無
	@date 2003.06.08 Moca CODE_AUTODETECTを指定できるように変更
	@date 2003.07.26 ryoji BOM引数追加
*/
ECodeType CFileLoad::FileOpen( LPCWSTR pFileName, bool bBigFile, ECodeType CharCode, int nFlag, bool* pbBomExist )
{
	HANDLE	hFile;
	ULARGE_INTEGER	fileSize;
	ECodeType	nBomCode;

	// FileCloseを呼んでからにしてください
	if( NULL != m_hFile ){
#ifdef _DEBUG
		::MessageBox( NULL, L"CFileLoad::FileOpen\nFileCloseを呼んでからにしてください" , NULL, MB_OK );
#endif
		throw CError_FileOpen();
	}
	hFile = ::CreateFile(
		pFileName,
		GENERIC_READ,
		//	Oct. 18, 2002 genta FILE_SHARE_WRITE 追加
		//	他プロセスが書き込み中のファイルを開けるように
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// 共有
		NULL,						// セキュリティ記述子
		OPEN_EXISTING,				// 作成方法
		FILE_FLAG_SEQUENTIAL_SCAN,	// ファイル属性
		NULL						// テンプレートファイルのハンドル
	);
	if( hFile == INVALID_HANDLE_VALUE ){
		throw CError_FileOpen();
	}
	m_hFile = hFile;

	// GetFileSizeEx は Win2K以上
	fileSize.LowPart = ::GetFileSize( hFile, &fileSize.HighPart );
	if( 0xFFFFFFFF == fileSize.LowPart ){
		DWORD lastError = ::GetLastError();
		if( NO_ERROR != lastError ){
			FileClose();
			throw CError_FileOpen();
		}
	}
	if (!CFileLoad::IsLoadableSize(fileSize.QuadPart, bBigFile)) {
		// ファイルが大きすぎる(2GB位)
		FileClose();
		throw CError_FileOpen(CError_FileOpen::TOO_BIG);
	}
	m_nFileSize = fileSize.QuadPart;
//	m_eMode = FLMODE_OPEN;

	// From Here Jun. 08, 2003 Moca 文字コード判定
	// データ読み込み
	Buffering();

	nBomCode = CCodeMediator::DetectUnicodeBom( m_pReadBuf, m_nReadDataLen );
	if( CharCode == CODE_AUTODETECT ){
		if( nBomCode != CODE_NONE ){
			CharCode = nBomCode;
		}else{
			CCodeMediator mediator(*m_pEencoding);
			CharCode = mediator.CheckKanjiCode( m_pReadBuf, m_nReadDataLen );
		}
	}
	// To Here Jun. 08, 2003
	// 不正な文字コードのときはデフォルト(SJIS:無変換)を設定
	if( !IsValidCodeOrCPType(CharCode) ){
		CharCode = CODE_DEFAULT;
	}
	m_CharCode = CharCode;
	m_pCodeBase=CCodeFactory::CreateCodeBase(m_CharCode, m_nFlag);
	m_encodingTrait = CCodePage::GetEncodingTrait(m_CharCode);
	m_nFlag = nFlag;

	m_nFileDataLen = m_nFileSize;
	bool bBom = false;
	if( 0 < m_nReadDataLen ){
		CMemory headData(m_pReadBuf, t_min(m_nReadDataLen, 10));
		CNativeW headUni;
		CIoBridge::FileToImpl(headData, &headUni, m_pCodeBase, m_nFlag);
		if( 1 <= headUni.GetStringLength() && headUni.GetStringPtr()[0] == 0xfeff ){
			bBom = true;
		}
	}
	if( bBom ){
		//	Jul. 26, 2003 ryoji BOMの有無をパラメータで返す
		m_bBomExist = true;
		if( pbBomExist != NULL ){
			*pbBomExist = true;
		}
	}else{
		//	Jul. 26, 2003 ryoji BOMの有無をパラメータで返す
		if( pbBomExist != NULL ){
			*pbBomExist = false;
		}
	}
	
	// To Here Jun. 13, 2003 Moca BOMの除去
	m_eMode = FLMODE_READY;
//	m_cmemLine.AllocBuffer( 256 );
	m_pCodeBase->GetEol( &m_memEols[0], EOL_NEL );
	m_pCodeBase->GetEol( &m_memEols[1], EOL_LS );
	m_pCodeBase->GetEol( &m_memEols[2], EOL_PS );
	bool bEolEx = false;
	int  nMaxEolLen = 0;
	for( int k = 0; k < (int)_countof(m_memEols); k++ ){
		if( 0 != m_memEols[k].GetRawLength() ){
			bEolEx = true;
			nMaxEolLen = t_max(nMaxEolLen, m_memEols[k].GetRawLength());
		}
	}
	m_bEolEx = bEolEx;
	m_nMaxEolLen = nMaxEolLen;
	if(	false == GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol ){
		m_bEolEx = false;
	}

	m_nReadOffset2 = 0;
	m_nTempResult = RESULT_FAILURE;
	m_cLineTemp.SetString(L"");
	return m_CharCode;
}

/*!
	ファイルを閉じる
	読み込み用バッファとm_memLineもクリアされる
*/
void CFileLoad::FileClose( void )
{
	if ( NULL != m_pReadBuf ){
		VirtualFree( m_pReadBuf, 0, MEM_RELEASE );
		m_pReadBuf = NULL;
	}
	m_nReadDataLen    = 0;
	m_nReadBufOffSet  = 0;
	if( NULL != m_hFile ){
		::CloseHandle( m_hFile );
		m_hFile = NULL;
	}
	if( NULL != m_pCodeBase ){
		delete m_pCodeBase;
		m_pCodeBase = NULL;
	}
	m_nFileSize		=  0;
	m_nFileDataLen	=  0;
	m_CharCode		= CODE_DEFAULT;
	m_bBomExist		= false; // From Here Jun. 08, 2003
	m_nFlag 		=  0;
	m_nReadLength	=  0;
	m_eMode			= FLMODE_CLOSE;
	m_nLineIndex	= -1;
}

/*! 1行読み込み
	UTF-7場合、データ内のNEL,PS,LS等の改行までを1行として取り出す
*/
EConvertResult CFileLoad::ReadLine(
	CNativeW*	pUnicodeBuffer,
	CEol*		pcEol,
	bool&		bHasNoTab,
	bool&		bOnlyASCII
	)
{
	if( m_CharCode != CODE_UTF7 && m_CharCode != CP_UTF7 ){
		return ReadLine_core( pUnicodeBuffer, pcEol, bHasNoTab, bOnlyASCII );
	}
	if( m_nReadOffset2 == m_cLineTemp.GetStringLength() ){
		CEol cEol;
		EConvertResult e = ReadLine_core( &m_cLineTemp, &cEol, bHasNoTab, bOnlyASCII );
		if( e == RESULT_FAILURE ){
			pUnicodeBuffer->_GetMemory()->SetRawDataHoldBuffer( L"", 0 );
			*pcEol = cEol;
			return RESULT_FAILURE;
		}
		m_nReadOffset2 = 0;
		m_nTempResult = e;
	}
	int  nOffsetTemp = m_nReadOffset2;
	int  nRetLineLen;
	CEol cEolTemp;
	const wchar_t* pRet = GetNextLineW( m_cLineTemp.GetStringPtr(), m_cLineTemp.GetStringLength(),
				&nRetLineLen, &m_nReadOffset2, &cEolTemp, GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol );
	if( m_cLineTemp.GetStringLength() == m_nReadOffset2 && nOffsetTemp == 0 ){
		// 途中に改行がない限りは、swapを使って中身のコピーを省略する
		pUnicodeBuffer->swap(m_cLineTemp);
		if( 0 < m_cLineTemp.GetStringLength() ){
			m_cLineTemp._GetMemory()->SetRawDataHoldBuffer( L"", 0 );
		}
		m_nReadOffset2 = 0;
	}else{
		// 改行が途中にあった。必要分をコピー
		pUnicodeBuffer->_GetMemory()->SetRawDataHoldBuffer( L"", 0 );
		pUnicodeBuffer->AppendString( pRet, nRetLineLen + cEolTemp.GetLen() );
	}
	*pcEol = cEolTemp;
	return m_nTempResult;
}

// ASCII文字列をWIDE文字列化
static
size_t Ascii2Uni(const char* __restrict pSrc, const size_t nSrcLen, wchar_t* __restrict pDst)
{
	size_t i = 0;
#if defined(_M_X64) || defined(_M_IX86)
	// 1文字ずつ処理すると時間が掛かるのでSIMD使用
	if (InstructionSet::getInstance()->AVX2()) {
		size_t n16 = nSrcLen / 16;
		const char* __restrict pr = pSrc;
		wchar_t* __restrict pw = pDst;
		for (; i < n16; ++i) {
			__m128i r = _mm_loadu_si128((const __m128i*)pr); pr += 16;
			__m256i w = _mm256_cvtepu8_epi16(r);
			_mm256_storeu_si256((__m256i*)pw, w); pw += 16;
		}
		i = n16 * 16;
#if defined(_M_X64)
	}else {
#else
	}else if (InstructionSet::getInstance()->SSE2()) {
#endif
		size_t n16 = nSrcLen / 16;
		__m128i zero = _mm_setzero_si128();
		const char* __restrict pr = pSrc;
		wchar_t* __restrict pw = pDst;
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
		pDst[i] = pSrc[i];
	}
	return nSrcLen;
}

/*!
	次の論理行を文字コード変換してロードする
	順次アクセス専用
	GetNextLineのような動作をする
	@return	NULL以外	1行を保持しているデータの先頭アドレスを返す。永続的ではない一時的な領域。
			NULL		データがなかった
*/
EConvertResult CFileLoad::ReadLine_core(
	CNativeW*	pUnicodeBuffer,	//!< [out] UNICODEデータ受け取りバッファ。改行も含めて読み取る。
	CEol*		pcEol,			//!< [i/o]
	bool&		bHasNoTab,
	bool&		bOnlyASCII
)
{
	EConvertResult eRet = RESULT_COMPLETE;

#ifdef _DEBUG
	if( m_eMode < FLMODE_READY ){
		MYTRACE( L"CFileLoad::ReadLine(): m_eMode = %d\n", m_eMode );
		return RESULT_FAILURE;
	}
#endif
	//行データバッファ (文字コード変換無しの生のデータ)
	m_cLineBuffer._SetRawLength(0);

	pcEol->SetType( EOL_NONE );
	const bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
	// 1行取り出し ReadBuf -> m_memLine
	//	Oct. 19, 2002 genta while条件を整理
	int			nBufLineLen;
	int			nEolLen;
	int			nBufferNext;
	for (;;) {
		const char*	pData = m_pReadBuf;				//!< [in]	検索文字列
		int			nDataLen = m_nReadDataLen;		//!< [in]	検索文字列のバイト数
		int*		pnLineLen = &nBufLineLen;		//!< [out]	1行のバイト数を返すただしEOLは含まない
		int*		pnBgn = &m_nReadBufOffSet;		//!< [i/o]	検索文字列のバイト単位のオフセット位置
		int*		pnEolLen = &nEolLen;			//!< [out]	EOLのバイト数 (Unicodeで困らないように)
		int*		pnBufferNext = &nBufferNext;	//!< [out]	次回持越しバッファ長(EOLの断片)

		int nbgn = *pnBgn;
		int i;
		if( nDataLen <= nbgn ){
			break;
		}

		*pnBufferNext = 0;

		const unsigned char* pUData = (const unsigned char*)pData; // signedだと符号拡張でNELがおかしくなるので
		int nLen = nDataLen;
		int neollen = 0;
		switch( m_encodingTrait ){
		case ENCODING_TRAIT_ERROR://
		case ENCODING_TRAIT_ASCII:
			if( m_bEolEx ){
				static const EEolType eEolEx[] = {
					EOL_NEL,
					EOL_LS,
					EOL_PS,
				};
				for( i = nbgn; i < nDataLen; ++i ){
					if( pData[i] == '\r' || pData[i] == '\n' ){
						pcEol->SetTypeByStringForFile( &pData[i], nDataLen - i );
						neollen = pcEol->GetLen();
						break;
					}
					if( m_bEolEx ){
						int k;
						for( k = 0; k < (int)_countof(eEolEx); k++ ){
							if( 0 != m_memEols[k].GetRawLength() && i + m_memEols[k].GetRawLength() - 1 < nDataLen
									&& 0 == memcmp( m_memEols[k].GetRawPtr(), pData + i, m_memEols[k].GetRawLength()) ){
								pcEol->SetType(eEolEx[k]);
								neollen = m_memEols[k].GetRawLength();
								break;
							}
						}
						if( k != (int)_countof(eEolEx) ){
							break;
						}
					}
				}
				// UTF-8のNEL,PS,LS断片の検出
				if( i == nDataLen ){
					for( i = t_max(0, nDataLen - m_nMaxEolLen - 1); i < nDataLen; i++ ){
						int k;
						bool bSet = false;
						for( k = 0; k < (int)_countof(eEolEx); k++ ){
							int nCompLen = t_min(nDataLen - i, m_memEols[k].GetRawLength());
							if( 0 != nCompLen && 0 == memcmp(m_memEols[k].GetRawPtr(), pData + i, nCompLen) ){
								*pnBufferNext = t_max(*pnBufferNext, nCompLen);
								bSet = true;
							}
						}
						if( bSet ){
							break;
						}
					}
					i = nDataLen;
				}

			}
			else {
				bHasNoTab = true;
				bOnlyASCII = true;
				i = nbgn;
#if defined(_M_X64) || defined(_M_IX86)
				const int remain = nDataLen - i;
				if (InstructionSet::getInstance()->AVX2()) {
					const int n32 = remain / 32;
					const __m256i maskCR = _mm256_set1_epi8('\r');
					const __m256i maskLF = _mm256_set1_epi8('\n');
					const __m256i maskTAB = _mm256_set1_epi8('\t');
					const __m256i* pc = (const __m256i*)(&pData[i]);
					int j;
					for (j=0; j<n32; ++j) {
						__m256i c = _mm256_loadu_si256(pc + j);
						__m256i matchCR = _mm256_cmpeq_epi8(c, maskCR);
						__m256i matchLF = _mm256_cmpeq_epi8(c, maskLF);
						__m256i matchTAB = _mm256_cmpeq_epi8(c, maskTAB);
						__m256i matchCRorLF = _mm256_or_si256(matchCR, matchLF);
						if (_mm256_movemask_epi8(matchCRorLF))
							break;
						if (_mm256_movemask_epi8(matchTAB))
							bHasNoTab = false;
					}
					i += j * 32;
#if defined(_M_X64)
				}else {
#else
				}else if (InstructionSet::getInstance()->SSE2()) {
#endif
					const int n16 = remain / 16;
					const __m128i maskCR = _mm_set1_epi8('\r');
					const __m128i maskLF = _mm_set1_epi8('\n');
					const __m128i maskTAB = _mm_set1_epi8('\t');
					const __m128i* pc = (const __m128i*)(&pData[i]);
					int j;
					for (j=0; j<n16; ++j) {
						__m128i c = _mm_loadu_si128(pc + j);
						__m128i matchCR = _mm_cmpeq_epi8(c, maskCR);
						__m128i matchLF = _mm_cmpeq_epi8(c, maskLF);
						__m128i matchTAB = _mm_cmpeq_epi8(c, maskTAB);
						__m128i matchCRorLF = _mm_or_si128(matchCR, matchLF);
						if (_mm_movemask_epi8(matchCRorLF))
							break;
						if (_mm_movemask_epi8(matchTAB))
							bHasNoTab = false;
					}
					i += j * 16;
				}
#endif // #if defined(_M_X64) || defined(_M_IX86)
				for(; i < nDataLen; ++i) {
					char c = pData[i];
					if (c >= 0) {
						if (c == '\r' || c == '\n') {
							pcEol->SetTypeByStringForFile( &pData[i], nDataLen - i );
							neollen = pcEol->GetLen();
							break;
						}
						else if (c == '\t') {
							bHasNoTab = false;
						}
					}
					else {
						bOnlyASCII = false;
					}
				}
			}
			break;
		case ENCODING_TRAIT_UTF16LE:
			nLen = nDataLen - 1;
			for( i = nbgn; i < nLen; i += 2 ){
				wchar_t c = static_cast<wchar_t>((pUData[i + 1] << 8) | pUData[i]);
				if( WCODE::IsLineDelimiter(c, bExtEol) ){
					pcEol->SetTypeByStringForFile_uni( &pData[i], nDataLen - i );
					neollen = (Int)pcEol->GetLen() * sizeof(wchar_t);
					break;
				}
			}
			break;
		case ENCODING_TRAIT_UTF16BE:
			nLen = nDataLen - 1;
			for( i = nbgn; i < nLen; i += 2 ){
				wchar_t c = static_cast<wchar_t>((pUData[i] << 8) | pUData[i + 1]);
				if( WCODE::IsLineDelimiter(c, bExtEol) ){
					pcEol->SetTypeByStringForFile_unibe( &pData[i], nDataLen - i );
					neollen = (Int)pcEol->GetLen() * sizeof(wchar_t);
					break;
				}
			}
			break;
		case ENCODING_TRAIT_UTF32LE:
			nLen = nDataLen - 3;
			for( i = nbgn; i < nLen; i += 4 ){
				wchar_t c = static_cast<wchar_t>((pUData[i+1] << 8) | pUData[i]);
				if( pUData[i+3] == 0x00 && pUData[i+2] == 0x00 && WCODE::IsLineDelimiter(c, bExtEol) ){
					wchar_t c2;
					int eolTempLen;
					if( i + 4 < nLen && pUData[i+7] == 0x00 && pUData[i+6] == 0x00 ){
						c2 = static_cast<wchar_t>((pUData[i+5] << 8) | pUData[i+4]);
						eolTempLen = 2 * sizeof(wchar_t);
					}else{
						c2 = 0x0000;
						eolTempLen = 1 * sizeof(wchar_t);
					}
					wchar_t pDataTmp[2] = {c, c2};
					pcEol->SetTypeByStringForFile_uni( reinterpret_cast<char *>(pDataTmp), eolTempLen );
					neollen = (Int)pcEol->GetLen() * 4;
					break;
				}
			}
			break;
		case ENCODING_TRAIT_UTF32BE:
			nLen = nDataLen - 3;
			for( i = nbgn; i < nLen; i += 4 ){
				wchar_t c = static_cast<wchar_t>((pUData[i+2] << 8) | pUData[i+3]);
				if( pUData[i] == 0x00 && pUData[i+1] == 0x00 && WCODE::IsLineDelimiter(c, bExtEol) ){
					wchar_t c2;
					int eolTempLen;
					if( i + 4 < nLen && pUData[i+4] == 0x00 && pUData[i+5] == 0x00 ){
						c2 = static_cast<wchar_t>((pUData[i+6] << 8) | pUData[i+7]);
						eolTempLen = 2 * sizeof(wchar_t);
					}else{
						c2 = 0x0000;
						eolTempLen = 1 * sizeof(wchar_t);
					}
					wchar_t pDataTmp[2] = {c, c2};
					pcEol->SetTypeByStringForFile_uni( reinterpret_cast<char *>(pDataTmp), eolTempLen );
					neollen = (Int)pcEol->GetLen() * 4;
					break;
				}
			}
			break;
		case ENCODING_TRAIT_EBCDIC_CRLF:
		case ENCODING_TRAIT_EBCDIC:
			// EOLコード変換しつつ設定
			for( i = nbgn; i < nDataLen; ++i ){
				if( m_encodingTrait == ENCODING_TRAIT_EBCDIC && bExtEol ){
					if( pData[i] == '\x15' ){
						pcEol->SetType(EOL_NEL);
						neollen = 1;
						break;
					}
				}
				if( pData[i] == '\x0d' || pData[i] == '\x25' ){
					char szEof[3] = {
						(pData[i]  == '\x25' ? '\x0a' : '\x0d'),
						(pData[i+1]== '\x25' ? '\x0a' : (char)
							(pData[i+1] == '\x0a' ? 0 : // EBCDIC の"\x0aがLFにならないように細工する
								(i + 1 < nDataLen ? pData[i+1] : 0))),
						0
					};
					pcEol->SetTypeByStringForFile( szEof, t_min(nDataLen - i,2) );
					neollen = (Int)pcEol->GetLen();
					break;
				}
			}
			break;
		}

		if( neollen < 1 ){
			// EOLがなかった場合
			if( i != nDataLen ){
				i = nDataLen;		// 最後の半端なバイトを落とさないように
			}
		}else{
			// CRの場合は、CRLFかもしれないので次のバッファへ送る
			if( *pcEol == EOL_CR ){
				*pnBufferNext = neollen;
			}
		}

		*pnBgn = i + neollen;
		*pnLineLen = i - nbgn;
		*pnEolLen = neollen;

		auto pLine = &pData[nbgn];

		// ReadBufから1行を取得するとき、改行コードが欠ける可能性があるため
		if( m_nReadDataLen <= m_nReadBufOffSet && FLMODE_READY == m_eMode ){// From Here Jun. 13, 2003 Moca
			int n = 128;
			int nMinAllocSize = m_cLineBuffer.GetRawLength() + nEolLen - nBufferNext + 100;
			while( n < nMinAllocSize ){
				n *= 2;
			}
			m_cLineBuffer.AllocBuffer( n );
			m_cLineBuffer.AppendRawData( pLine, nBufLineLen + nEolLen - nBufferNext );
			m_nReadBufOffSet -= nBufferNext;
			// バッファロード   File -> ReadBuf
			Buffering();
			if( 0 == nBufferNext && 0 < nEolLen ){
				// ぴったり行出力
				break;
			}
		}else{
			m_cLineBuffer.AppendRawData( pLine, nBufLineLen + nEolLen );
			break;
		}
	}
	m_nReadLength += m_cLineBuffer.GetRawLength();

	// 文字コード変換 cLineBuffer -> pUnicodeBuffer
	EConvertResult eConvertResult;

	if (bOnlyASCII) {
		int nSrcLen = m_cLineBuffer.GetRawLength();
		pUnicodeBuffer->AllocStringBuffer( nSrcLen + 1 );
		size_t nDstLen = Ascii2Uni((const char*)m_cLineBuffer.GetRawPtr(), nSrcLen, pUnicodeBuffer->GetStringPtr());
		pUnicodeBuffer->_SetStringLength( (int)nDstLen );
		eConvertResult = RESULT_COMPLETE;
	}
	else {
		eConvertResult = CIoBridge::FileToImpl(m_cLineBuffer,pUnicodeBuffer,m_pCodeBase,m_nFlag);
		if(eConvertResult==RESULT_LOSESOME){
			eRet = RESULT_LOSESOME;
		}
	}

	m_nLineIndex++;

	// 2012.10.21 Moca BOMの除去(UTF-7対応)
	if( m_nLineIndex == 0 ){
		if( m_bBomExist && 1 <= pUnicodeBuffer->GetStringLength() ){
			if( pUnicodeBuffer->GetStringPtr()[0] == 0xfeff ){
				CNativeW tmp(pUnicodeBuffer->GetStringPtr() + 1, pUnicodeBuffer->GetStringLength() - 1);
				*pUnicodeBuffer = tmp;
			}
		}
	}
	if( 0 == pUnicodeBuffer->GetStringLength() ){
		eRet = RESULT_FAILURE;
	}

	return eRet;
}

/*!
	バッファにデータを読み込む
	@note エラー時は throw する
*/
void CFileLoad::Buffering( void )
{
	DWORD	ReadSize;

	// メモリー確保
	if( NULL == m_pReadBuf ){
		/*! ロード用バッファサイズ */
		m_pReadBuf = (char *)::VirtualAlloc(NULL, g_nBufSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		if( NULL == m_pReadBuf ){
			throw CError_FileRead(); // メモリー確保に失敗
		}
		m_nReadDataLen = 0;
		m_nReadBufOffSet = 0;
	}
	// ReadBuf内にデータが残っている
	else if( m_nReadBufOffSet < m_nReadDataLen ){
		m_nReadDataLen -= m_nReadBufOffSet;
		memmove( m_pReadBuf, &m_pReadBuf[m_nReadBufOffSet], m_nReadDataLen );
		m_nReadBufOffSet = 0;
	}
	else{
		m_nReadBufOffSet = 0;
		m_nReadDataLen = 0;
	}
	// ファイルの読み込み
	if( !::ReadFile( m_hFile, &m_pReadBuf[m_nReadDataLen], g_nBufSize - m_nReadDataLen, &ReadSize, NULL ) )
		throw CError_FileRead();
	if( 0 == ReadSize ){
		m_eMode = FLMODE_READBUFEND;	// ファイルなどの終わりに達したらしい
	}
	m_nReadDataLen += ReadSize;
}

/*!
	 現在の進行率を取得する
	 @return 0% - 100%  若干誤差が出る
*/
int CFileLoad::GetPercent( void ){
	int nRet;
	if( 0 == m_nFileDataLen || m_nReadLength > m_nFileDataLen ){
		nRet = 100;
	}else{
		nRet = static_cast<int>(m_nReadLength * 100 / m_nFileDataLen);
	}
	return nRet;
}
