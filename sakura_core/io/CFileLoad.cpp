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
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include <stdlib.h>
#include <string.h>
#include <algorithm>
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

/*
	@note Win32APIで実装
*/

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
	int insertPosition = (int)str.length() - 3;
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
	m_eMode			= FLMODE_CLOSE;	// Jun. 08, 2003 Moca

	m_nLineIndex	= -1;
}

/*! デストラクタ */
CFileLoad::~CFileLoad( void )
{
	FileClose();
}

void CFileLoad::Prepare( const CFileLoad& other, size_t nReadBufOffsetBegin, size_t nReadBufOffsetEnd )
{
	m_pEencoding	= other.m_pEencoding;

	// ファイルハンドルの所有権なし
	m_hFile			= NULL;
	m_hFileMapping	= NULL;

	m_nFileSize		= other.m_nFileSize;
	m_CharCode		= other.m_CharCode;
	m_pCodeBase		= other.m_pCodeBase;
	m_encodingTrait = other.m_encodingTrait;
	m_bBomExist		= other.m_bBomExist;
	m_nFlag 		= other.m_nFlag;
	m_bEolEx		= other.m_bEolEx;
	m_nMaxEolLen	= other.m_nMaxEolLen;
	m_cLineTemp		= other.m_cLineTemp;

	m_nLineIndex	= -1;
	m_eMode			= FLMODE_READY;
	m_nTempResult	= RESULT_FAILURE;

	m_pReadBufTop			= other.m_pReadBufTop;
	m_nReadBufOffsetCurrent = nReadBufOffsetBegin;
	m_nReadBufOffsetBegin	= nReadBufOffsetBegin;
	m_nReadBufOffsetEnd		= nReadBufOffsetEnd;
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
	if( 0xFFFFFFFFU == fileSize.LowPart ){
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

	m_pReadBufTop = NULL;
	if( 0 < m_nFileSize ){
		m_hFileMapping = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
		if( m_hFileMapping != NULL ){
			m_pReadBufTop = (const char*)MapViewOfFile( m_hFileMapping, FILE_MAP_READ, 0, 0, 0 );
		}
		if( m_pReadBufTop == NULL ){
			FileClose();
			throw CError_FileOpen();
		}
	}

	if( CharCode == CODE_AUTODETECT ){
		CCodeMediator mediator(*m_pEencoding);
		const size_t nKanjiCheckLen = (size_t)(std::min)(m_nFileSize, m_nAutoDetectReadLen);
		CharCode = mediator.CheckKanjiCode(m_pReadBufTop, nKanjiCheckLen);
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

	m_nFileDataLen = m_nReadBufOffsetEnd = (size_t)m_nFileSize;
	bool bBom = false;
	if( 0 < m_nFileSize ){
		const int nBomCheckLen = (int)(std::min)(m_nFileSize, 10LL);
		CMemory headData(m_pReadBufTop, nBomCheckLen);
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
	m_pCodeBase->GetEol( &m_memEols[0], EEolType::next_line );
	m_pCodeBase->GetEol( &m_memEols[1], EEolType::line_separator );
	m_pCodeBase->GetEol( &m_memEols[2], EEolType::paragraph_separator );
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
	if( m_hFile != NULL ){
		if( m_pReadBufTop != NULL ){
			(void)UnmapViewOfFile( m_pReadBufTop );
			m_pReadBufTop = NULL;
		}
		if( m_hFileMapping != NULL ){
			CloseHandle( m_hFileMapping );
			m_hFileMapping = NULL;
		}
		if( NULL != m_hFile ){
			::CloseHandle( m_hFile );
			m_hFile = NULL;
		}
		if( NULL != m_pCodeBase ){
			delete m_pCodeBase;
			m_pCodeBase = NULL;
		}
	}
	m_nReadBufOffsetCurrent = 0;
	m_nReadBufOffsetBegin	= 0;
	m_nReadBufOffsetEnd		= 0;
	m_nFileSize		=  0;
	m_nFileDataLen	=  0;
	m_CharCode		= CODE_DEFAULT;
	m_bBomExist		= false; // From Here Jun. 08, 2003
	m_nFlag 		=  0;
	m_eMode			= FLMODE_CLOSE;
	m_nLineIndex	= -1;
}

/*! 1行読み込み
	UTF-7場合、データ内のNEL,PS,LS等の改行までを1行として取り出す
*/
EConvertResult CFileLoad::ReadLine( CNativeW* pUnicodeBuffer, CEol* pcEol )
{
	if( m_CharCode != CODE_UTF7 && m_CharCode != CP_UTF7 ){
		return ReadLine_core( pUnicodeBuffer, pcEol );
	}
	if( m_nReadOffset2 == m_cLineTemp.GetStringLength() ){
		CEol cEol;
		EConvertResult e = ReadLine_core( &m_cLineTemp, &cEol );
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

/*!
	次の論理行を文字コード変換してロードする
	順次アクセス専用
	GetNextLineのような動作をする
	@return	NULL以外	1行を保持しているデータの先頭アドレスを返す。永続的ではない一時的な領域。
			NULL		データがなかった
*/
EConvertResult CFileLoad::ReadLine_core(
	CNativeW*	pUnicodeBuffer,	//!< [out] UNICODEデータ受け取りバッファ。改行も含めて読み取る。
	CEol*		pcEol			//!< [i/o]
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
	m_cLineBuffer.SetRawDataHoldBuffer("",0);

	// 1行取り出し ReadBuf -> m_memLine
	//	Oct. 19, 2002 genta while条件を整理
	size_t		nBufLineLen;
	size_t		nEolLen;
	const char* pLine = GetNextLineCharCode(
		m_pReadBufTop,
		m_nReadBufOffsetEnd,
		&nBufLineLen,
		&m_nReadBufOffsetCurrent,
		pcEol,
		&nEolLen
	);
	if( pLine != NULL ){
		m_cLineBuffer.AppendRawData( pLine, nBufLineLen + nEolLen );
	}

	// 文字コード変換 cLineBuffer -> pUnicodeBuffer
	EConvertResult eConvertResult = CIoBridge::FileToImpl(m_cLineBuffer, pUnicodeBuffer, m_pCodeBase,m_nFlag);
	if(eConvertResult==RESULT_LOSESOME){
		eRet = RESULT_LOSESOME;
	}

	m_nLineIndex++;

	// 2012.10.21 Moca BOMの除去(UTF-7対応)
	if( m_nLineIndex == 0 ){
		if( m_bBomExist && 1 <= pUnicodeBuffer->GetStringLength() ){
			if( pUnicodeBuffer->GetStringPtr()[0] == 0xfeff ){
				CNativeW tmp(pUnicodeBuffer->GetStringPtr() + 1, pUnicodeBuffer->GetStringLength() - 1);
				*pUnicodeBuffer = std::move(tmp);
			}
		}
	}
	if( 0 == pUnicodeBuffer->GetStringLength() ){
		eRet = RESULT_FAILURE;
	}

	return eRet;
}

/*!
	 現在の進行率を取得する
	 @return 0% - 100%  若干誤差が出る
*/
int CFileLoad::GetPercent( void ){
	int nRet;
	const size_t nSize = (m_nReadBufOffsetEnd - m_nReadBufOffsetBegin);
	if( 0 == nSize || m_nReadBufOffsetEnd <= m_nReadBufOffsetCurrent ){
		nRet = 100;
	}else{
		nRet = static_cast<int>((m_nReadBufOffsetCurrent - m_nReadBufOffsetBegin) * 100.0 / nSize);
	}
	return nRet;
}

/*!
	指定オフセットを含む行の次行の先頭オフセットを取得する (ただし指定オフセットが行頭を指す場合はその位置をそのまま返す)
	@param[in]	nOffset	指定オフセット (ファイル先頭を基準とするバイト単位)
	@returns	次行の先頭オフセット (ファイル先頭を基準とするバイト単位)
*/
size_t CFileLoad::GetNextLineOffset( size_t nOffset )
{
	// 探し始めるオフセットが半端にならぬよう調整
	const size_t nAlignBytes =
		((m_encodingTrait == ENCODING_TRAIT_UTF32LE) || (m_encodingTrait == ENCODING_TRAIT_UTF32BE)) ? 4 :
		((m_encodingTrait == ENCODING_TRAIT_UTF16LE) || (m_encodingTrait == ENCODING_TRAIT_UTF16BE)) ? 2 :
		1;
	size_t nOffsetBegin = (std::min)(nOffset, (size_t)m_nFileSize);
	nOffsetBegin = nOffsetBegin - (nOffsetBegin % nAlignBytes);
	if( nOffsetBegin < nAlignBytes ){
		return 0;
	}
	nOffsetBegin -= nAlignBytes;

	size_t nLineLenDummy = 0;
	CEol cEolDummy;
	size_t nEolLenDummy = 0;
	(void)GetNextLineCharCode( m_pReadBufTop, (size_t)m_nFileSize, &nLineLenDummy, &nOffsetBegin, &cEolDummy, &nEolLenDummy );

	return nOffsetBegin;
}

/*!
	GetNextLineの汎用文字コード版
*/
const char* CFileLoad::GetNextLineCharCode(
	const char*	pData,		//!< [in]	検索文字列
	size_t		nDataLen,	//!< [in]	検索文字列のバイト数
	size_t*		pnLineLen,	//!< [out]	1行のバイト数を返すただしEOLは含まない
	size_t*		pnBgn,		//!< [i/o]	検索文字列のバイト単位のオフセット位置
	CEol*		pcEol,		//!< [i/o]	EOL
	size_t*		pnEolLen	//!< [out]	EOLのバイト数 (Unicodeで困らないように)
){
	const size_t nbgn = *pnBgn;
	size_t i;

	pcEol->SetType( EEolType::none );

	if( nDataLen <= nbgn ){
		*pnLineLen = 0;
		*pnEolLen = 0;
		return NULL;
	}
	const unsigned char* pUData = (const unsigned char*)pData; // signedだと符号拡張でNELがおかしくなるので
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
	size_t nLen = nDataLen;
	size_t neollen = 0;
	switch( m_encodingTrait ){
	case ENCODING_TRAIT_ERROR://
	case ENCODING_TRAIT_ASCII:
		{
			static const EEolType eEolEx[] = {
				EEolType::next_line,
				EEolType::line_separator,
				EEolType::paragraph_separator,
			};
			nLen = nDataLen;
			for( i = nbgn; i < nDataLen; ++i ){
				if( pData[i] == '\r' || pData[i] == '\n' ){
					pcEol->SetTypeByStringForFile( &pData[i], nDataLen - i );
					neollen = (size_t)pcEol->GetLen();
					break;
				}
				if( m_bEolEx ){
					int k;
					for( k = 0; k < (int)_countof(eEolEx); k++ ){
						if( 0 != m_memEols[k].GetRawLength() && i + m_memEols[k].GetRawLength() - 1 < nDataLen
								&& 0 == memcmp( m_memEols[k].GetRawPtr(), pData + i, m_memEols[k].GetRawLength()) ){
							pcEol->SetType(eEolEx[k]);
							neollen = (size_t)m_memEols[k].GetRawLength();
							break;
						}
					}
					if( k != (int)_countof(eEolEx) ){
						break;
					}
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
				neollen = (size_t)pcEol->GetLen() * sizeof(wchar_t);
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
				neollen = (size_t)pcEol->GetLen() * sizeof(wchar_t);
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
				neollen = (size_t)pcEol->GetLen() * 4U;
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
				neollen = (size_t)pcEol->GetLen() * 4U;
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
					pcEol->SetType(EEolType::next_line);
					neollen = 1U;
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
				pcEol->SetTypeByStringForFile( szEof, t_min(nDataLen - i, (size_t)2) );
				neollen = (size_t)pcEol->GetLen();
				break;
			}
		}
		break;
	}

	if( neollen < 1U ){
		// EOLがなかった場合
		if( i != nDataLen ){
			i = nDataLen;		// 最後の半端なバイトを落とさないように
		}
	}

	*pnBgn = i + neollen;
	*pnLineLen = i - nbgn;
	*pnEolLen = neollen;

	return &pData[nbgn];
}
