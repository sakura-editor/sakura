﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include <shellapi.h>// HDROP
#include "CClipboard.h"
#include "doc/CEditDoc.h"
#include "charset/CCodeMediator.h"
#include "charset/CCodeFactory.h"
#include "charset/CShiftJis.h"
#include "charset/CUtf8.h"
#include "CEol.h"
#include "mem/CNativeA.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CClipboard::CClipboard(HWND hwnd)
{
	m_hwnd = hwnd;
	m_bOpenResult = ::OpenClipboard(hwnd);
}

CClipboard::~CClipboard()
{
	Close();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CClipboard::Empty()
{
	EmptyClipboard();
}

void CClipboard::Close()
{
	if(m_bOpenResult){
		::CloseClipboard();
		m_bOpenResult=FALSE;
	}
}

bool CClipboard::SetText(
	const wchar_t*	pData,			//!< コピーするUNICODE文字列
	size_t			nDataLen,		//!< pDataの長さ（文字単位）
	bool			bColumnSelect,
	bool			bLineSelect,
	UINT			uFormat
)
{
	if( !m_bOpenResult ){
		return false;
	}

	/*
	// テキスト形式のデータ (CF_OEMTEXT)
	HGLOBAL hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nTextLen + 1
	);
	if( hgClipText ){
		char* pszClip = static_cast<char*>(::GlobalLock(hgClipText));
		memcpy( pszClip, pszText, nTextLen );
		pszClip[nTextLen] = '\0';
		::GlobalUnlock( hgClipText );
		::SetClipboardData( CF_OEMTEXT, hgClipText );
	}
	*/

	// UNICODE形式のデータ (CF_UNICODETEXT)
	HGLOBAL hgClipText = nullptr;
	bool bUnicodeText = (uFormat == (UINT)-1 || uFormat == CF_UNICODETEXT);
	while(bUnicodeText){
		//領域確保
		hgClipText = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			(nDataLen + 1) * sizeof(wchar_t)
		);
		if( !hgClipText )break;

		//確保した領域にデータをコピー
		wchar_t* pszClip = static_cast<wchar_t*>(::GlobalLock(hgClipText));
		wmemcpy( pszClip, pData, nDataLen );	//データ
		pszClip[nDataLen] = L'\0';				//終端ヌル
		::GlobalUnlock( hgClipText );

		//クリップボードに設定
		SetClipboardData( CF_UNICODETEXT, hgClipText );
		bUnicodeText = false;
	}
	//	1回しか通らない. breakでここまで飛ぶ

	// バイナリ形式のデータ
	//	(size_t) 「データ」の長さ
	//	「データ」
	HGLOBAL hgClipSakura = nullptr;
	//サクラエディタ専用フォーマットを取得
	CLIPFORMAT	uFormatSakuraClip = CClipboard::GetSakuraFormat();
	bool bSakuraText = (uFormat == (UINT)-1 || uFormat == uFormatSakuraClip);
	while(bSakuraText){
		if( 0 == uFormatSakuraClip )break;

		//領域確保
		hgClipSakura = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			sizeof(size_t) + (nDataLen + 1) * sizeof(wchar_t)
		);
		if( !hgClipSakura )break;

		//確保した領域にデータをコピー
		BYTE* pClip = static_cast<BYTE*>(::GlobalLock(hgClipSakura));
		*((size_t*)pClip) = nDataLen; pClip += sizeof(nDataLen);						//データの長さ
		wmemcpy( (wchar_t*)pClip, pData, nDataLen ); pClip += nDataLen*sizeof(wchar_t);	//データ
		*((wchar_t*)pClip) = L'\0'; pClip += sizeof(wchar_t);							//終端ヌル
		::GlobalUnlock( hgClipSakura );

		//クリップボードに設定
		SetClipboardData( uFormatSakuraClip, hgClipSakura );
		bSakuraText = false;
	}
	//	1回しか通らない. breakでここまで飛ぶ

	// 矩形選択を示すダミーデータ
	HGLOBAL hgClipMSDEVColumn = nullptr;
	if( bColumnSelect ){
		UINT uFormat = ::RegisterClipboardFormat( L"MSDEVColumnSelect" );
		if( 0 != uFormat ){
			hgClipMSDEVColumn = GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVColumn ){
				BYTE* pClip = static_cast<BYTE*>(::GlobalLock(hgClipMSDEVColumn));
				pClip[0] = 0;
				::GlobalUnlock( hgClipMSDEVColumn );
				SetClipboardData( uFormat, hgClipMSDEVColumn );
			}
		}
	}

	/* 行選択を示すダミーデータ */
	HGLOBAL hgClipMSDEVLine = nullptr;		// VS2008 以前の形式
	if( bLineSelect ){
		UINT uFormat = ::RegisterClipboardFormat( L"MSDEVLineSelect" );
		if( 0 != uFormat ){
			hgClipMSDEVLine = GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVLine ){
				BYTE* pClip = (BYTE*)::GlobalLock( hgClipMSDEVLine );
				pClip[0] = 0x01;
				::GlobalUnlock( hgClipMSDEVLine );
				SetClipboardData( uFormat, hgClipMSDEVLine );
			}
		}
	}
	HGLOBAL hgClipMSDEVLine2 = nullptr;	// VS2010 形式
	if( bLineSelect ){
		UINT uFormat = ::RegisterClipboardFormat( L"VisualStudioEditorOperationsLineCutCopyClipboardTag" );
		if( 0 != uFormat ){
			hgClipMSDEVLine2 = GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVLine2 ){
				BYTE* pClip = (BYTE*)::GlobalLock( hgClipMSDEVLine2 );
				pClip[0] = 0x01;	// ※ ClipSpy で調べるとデータはこれとは違うが内容には無関係に動くっぽい
				::GlobalUnlock( hgClipMSDEVLine2 );
				SetClipboardData( uFormat, hgClipMSDEVLine2 );
			}
		}
	}

	if( bColumnSelect && !hgClipMSDEVColumn ){
		return false;
	}
	if( bLineSelect && !(hgClipMSDEVLine && hgClipMSDEVLine2) ){
		return false;
	}
	if( !(hgClipText && hgClipSakura) ){
		return false;
	}
	return true;
}

bool CClipboard::SetHtmlText(const CNativeW& cmemBUf)
{
	if( !m_bOpenResult ){
		return false;
	}

	CNativeA cmemUtf8;
	CUtf8().UnicodeToCode(cmemBUf, cmemUtf8._GetMemory());

	CNativeA cmemHeader;
	size_t size = cmemUtf8.GetStringLength() + 134;
	cmemHeader.AppendString("Version:0.9\r\n");
	cmemHeader.AppendString("StartHTML:00000097\r\n");
	cmemHeader.AppendStringF("EndHTML:%08Id\r\n", size + 36);
	cmemHeader.AppendString("StartFragment:00000134\r\n");
	cmemHeader.AppendStringF("EndFragment:%08Id\r\n", size);
	cmemHeader.AppendString("<html><body>\r\n<!--StartFragment -->\r\n");
	CNativeA cmemFooter;
	cmemFooter.AppendString("\r\n<!--EndFragment-->\r\n</body></html>\r\n");

	HGLOBAL hgClipText = nullptr;
	size_t nLen = cmemHeader.GetStringLength() + cmemUtf8.GetStringLength() + cmemFooter.GetStringLength();
	//領域確保
	hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nLen + 1
	);
	if( !hgClipText ) return false;

	//確保した領域にデータをコピー
	char* pszClip = static_cast<char*>(::GlobalLock(hgClipText));
	memcpy_raw( pszClip, cmemHeader.GetStringPtr(), cmemHeader.GetStringLength() );	//データ
	memcpy_raw( pszClip + cmemHeader.GetStringLength(), cmemUtf8.GetStringPtr(), cmemUtf8.GetStringLength() );	//データ
	memcpy_raw( pszClip + cmemHeader.GetStringLength() + cmemUtf8.GetStringLength(), cmemFooter.GetStringPtr(), cmemFooter.GetStringLength() );	//データ
	pszClip[nLen] = '\0';				//終端ヌル
	::GlobalUnlock( hgClipText );

	//クリップボードに設定
	UINT uFormat = ::RegisterClipboardFormat( L"HTML Format" );
	SetClipboardData( uFormat, hgClipText );
	return true;
}

/*! テキストを取得する
	@param [out] cmemBuf 取得したテキストの格納先
	@param [in,out] pbColumnSelect 矩形選択形式
	@param [in,out] pbLineSelect 行選択形式
	@param [in] cEol HDROP形式のときの改行コード
	@param [in] uGetFormat クリップボード形式
*/
bool CClipboard::GetText(IWBuffer* cmemBuf, bool* pbColumnSelect, bool* pbLineSelect, const CEol& cEol, UINT uGetFormat)
{
	if( !m_bOpenResult ){
		return false;
	}
	if( nullptr != pbColumnSelect ){
		*pbColumnSelect = false;
	}
	if( nullptr != pbLineSelect ){
		*pbLineSelect = false;
	}

	//矩形選択や行選択のデータがあれば取得
	if( nullptr != pbColumnSelect || nullptr != pbLineSelect ){
		UINT uFormat = 0;
		while( ( uFormat = EnumClipboardFormats( uFormat ) ) != 0 ){
			// Jul. 2, 2005 genta : check return value of GetClipboardFormatName
			WCHAR szFormatName[128];
			if( ::GetClipboardFormatName( uFormat, szFormatName, _countof(szFormatName) - 1 ) ){
				if( nullptr != pbColumnSelect && 0 == lstrcmpi( L"MSDEVColumnSelect", szFormatName ) ){
					*pbColumnSelect = true;
					break;
				}
				if( nullptr != pbLineSelect && 0 == lstrcmpi( L"MSDEVLineSelect", szFormatName ) ){
					*pbLineSelect = true;
					break;
				}
				if( nullptr != pbLineSelect && 0 == lstrcmpi( L"VisualStudioEditorOperationsLineCutCopyClipboardTag", szFormatName ) ){
					*pbLineSelect = true;
					break;
				}
			}
		}
	}

	//サクラ形式のデータがあれば取得
	CLIPFORMAT uFormatSakuraClip = CClipboard::GetSakuraFormat();
	if( (uGetFormat == -1 || uGetFormat == uFormatSakuraClip)
		&& IsClipboardFormatAvailable( uFormatSakuraClip ) ){
		HGLOBAL hSakura = GetClipboardData( uFormatSakuraClip );
		if (hSakura != nullptr) {
			BYTE* pData = (BYTE*)::GlobalLock(hSakura);
			size_t nLength        = *((size_t*)pData);
			const wchar_t* szData = (const wchar_t*)(pData + sizeof(size_t));
			cmemBuf->Append( szData, nLength );
			::GlobalUnlock(hSakura);
			return true;
		}
	}

	//UNICODE形式のデータがあれば取得
	// From Here 2005/05/29 novice UNICODE TEXT 対応処理を追加
	HGLOBAL hUnicode = nullptr;
	if( uGetFormat == -1 || uGetFormat == CF_UNICODETEXT ){
		hUnicode = GetClipboardData( CF_UNICODETEXT );
	}
	if( hUnicode != nullptr ){
		wchar_t* szData = static_cast<wchar_t*>(::GlobalLock(hUnicode));
		cmemBuf->Append( szData, GlobalSize(hUnicode) / 2 - 1);
		::GlobalUnlock(hUnicode);
		return true;
	}
	//	To Here 2005/05/29 novice

	//OEMTEXT形式のデータがあれば取得
	HGLOBAL hText = nullptr;
	if( uGetFormat == -1 || uGetFormat == CF_OEMTEXT ){
		hText = GetClipboardData( CF_OEMTEXT );
	}
	if( hText != nullptr ){
		char* szData = static_cast<char*>(::GlobalLock(hText));
		//SJIS→UNICODE
		CMemory cmemSjis( szData, GlobalSize(hText) );
		CNativeW cmemUni;
		CShiftJis::SJISToUnicode(cmemSjis, &cmemUni);
		cmemSjis.Reset();
		// '\0'までを取得
		cmemUni._SetStringLength(wcslen(cmemUni.GetStringPtr()));
		cmemBuf->Append(cmemUni.GetStringPtr(), (size_t)cmemUni.GetStringLength());
		::GlobalUnlock(hText);
		return true;
	}

	/* 2008.09.10 bosagami パス貼り付け対応 */
	//HDROP形式のデータがあれば取得
	if( (uGetFormat == -1 || uGetFormat == CF_HDROP)
		&& IsClipboardFormatAvailable(CF_HDROP) ){
		HDROP hDrop = (HDROP)GetClipboardData(CF_HDROP);
		if(hDrop != nullptr){
			WCHAR sTmpPath[_MAX_PATH + 1] = {0};
			const int nMaxCnt = DragQueryFile(hDrop, 0xFFFFFFFF, nullptr, 0);

			for(int nLoop = 0; nLoop < nMaxCnt; nLoop++){
				DragQueryFile(hDrop, nLoop, sTmpPath, _countof(sTmpPath) - 1);
				// 2012.10.05 Moca ANSI版に合わせて最終行にも改行コードをつける
				cmemBuf->Append(sTmpPath, wcslen(sTmpPath));
				if(nMaxCnt > 1){
					cmemBuf->Append( cEol.GetValue2(), cEol.GetLen() );
				}
			}
			return true;
		}
	}

	return false;
}

bool CClipboard::GetText(CNativeW* cmemBuf, bool* pbColumnSelect, bool* pbLineSelect, const CEol& cEol, UINT uGetFormat)
{
	CNativeWBuffer buff(cmemBuf);
	return GetText(&buff, pbColumnSelect, pbLineSelect, cEol, uGetFormat);
}

bool CClipboard::GetText(std::wstring* cmemBuf, bool* pbColumnSelect, bool* pbLineSelect, const CEol& cEol, UINT uGetFormat)
{
	StdWStringBuffer buff(cmemBuf);
	return GetText(&buff, pbColumnSelect, pbLineSelect, cEol, uGetFormat);
}

struct SSystemClipFormatNames
{
	CLIPFORMAT	m_nClipFormat;
	const wchar_t* m_pszName;
};
static const SSystemClipFormatNames sClipFormatNames[] =
{
	{CF_TEXT        ,L"CF_TEXT"},
	{CF_BITMAP      ,L"CF_BITMAP"},
	{CF_METAFILEPICT,L"CF_METAFILEPICT"},
	{CF_SYLK        ,L"CF_SYLK"},
	{CF_DIF         ,L"CF_DIF"},
	{CF_TIFF        ,L"CF_TIFF"},
	{CF_OEMTEXT     ,L"CF_OEMTEXT"},
	{CF_DIB         ,L"CF_DIB"},
	{CF_PALETTE     ,L"CF_PALETTE"},
	{CF_PENDATA     ,L"CF_PENDATA"},
	{CF_RIFF        ,L"CF_RIFF"},
	{CF_WAVE        ,L"CF_WAVE"},
	{CF_UNICODETEXT ,L"CF_UNICODETEXT"},
	{CF_ENHMETAFILE ,L"CF_ENHMETAFILE"},
	{CF_HDROP       ,L"CF_HDROP"},
	{CF_LOCALE      ,L"CF_LOCALE"},
	{CF_DIBV5       ,L"CF_DIBV5"},
};

static CLIPFORMAT GetClipFormat(const wchar_t* pFormatName)
{
	CLIPFORMAT uFormat = (CLIPFORMAT)-1;
	if( pFormatName[0] == L'\0' ){
		return uFormat;
	}
	for(int i = 0; i < _countof(sClipFormatNames); i++){
		if( 0 == _wcsicmp(pFormatName, sClipFormatNames[i].m_pszName) ){
			uFormat = sClipFormatNames[i].m_nClipFormat;
		}
	}
	if( uFormat == (CLIPFORMAT)-1 ){
		bool bNumber = true;
		for( int i =0; pFormatName[i]; i++ ){
			if( !WCODE::Is09(pFormatName[i]) ){
				bNumber = false;
			}
		}
		if( bNumber ){
			uFormat = _wtoi(pFormatName);
		}else{
			uFormat = ::RegisterClipboardFormat( pFormatName );
		}
	}
	return uFormat;
}

bool CClipboard::IsIncludeClipboardFormat(const wchar_t* pFormatName)
{
	CLIPFORMAT uFormat = GetClipFormat(pFormatName);
	if( IsClipboardFormatAvailable(uFormat) ){
		return true;
	}
	return false;
}

static int GetEndModeByMode(int nMode, int nEndMode)
{
	if( nEndMode == -1 ){
		switch(nMode){
			case -1: nEndMode = 0; break;
			case CODE_AUTODETECT: nEndMode = 0; break;
			case CODE_SJIS: nEndMode = 1; break;
			case CODE_EUC: nEndMode = 1; break;
			case CODE_LATIN1: nEndMode = 1; break;
			case CODE_UNICODE: nEndMode = 2; break;
			case CODE_UNICODEBE: nEndMode = 2; break;
			case CODE_UTF8: nEndMode = 1; break;
			case CODE_CESU8: nEndMode = 1; break;
			case CODE_UTF7: nEndMode = 1; break;
			default: nEndMode = 0; break;
		}
	}
	return nEndMode;
}

bool CClipboard::SetClipboardByFormat(const CStringRef& cstr, const wchar_t* pFormatName, int nMode, int nEndMode)
{
	CLIPFORMAT uFormat = GetClipFormat(pFormatName);
	if( uFormat == (CLIPFORMAT)-1 ){
		return false;
	}
	if( nMode == -2 ){
		if( uFormat == CF_UNICODETEXT || uFormat == GetSakuraFormat() ){
			return SetText(cstr.GetPtr(), cstr.GetLength(), false, false, uFormat);
		}
		return false;
	}

	CMemory cmemBuf;
	char* pBuf = nullptr;
	size_t nTextByteLen = 0;
	if( nMode == -1 ){
		// バイナリモード U+00 - U+ffを0x00 - 0xffにマッピング
		cmemBuf.AllocBuffer(cstr.GetLength());
		cmemBuf._SetRawLength(cstr.GetLength());
		pBuf = (char*)cmemBuf.GetRawPtr();
		size_t len = cstr.GetLength();
		const wchar_t* pMem = cstr.GetPtr();
		for(size_t i = 0; i < len; i++){
			pBuf[i] = (unsigned char)pMem[i];
			if( 0xff < pMem[i] ){
				return false;
			}
		}
		nTextByteLen = len;
 	}else{
		ECodeType eMode = (ECodeType)nMode;
		if( !IsValidCodeType(eMode) ){
			return false;
		}
		if( eMode == CODE_UNICODE ){
			pBuf = (char*)cstr.GetPtr();
			nTextByteLen = cstr.GetLength() * sizeof(wchar_t);
		}else{
			CCodeBase* pCode = CCodeFactory::CreateCodeBase(eMode, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode());
			if( RESULT_FAILURE == pCode->UnicodeToCode(cstr, &cmemBuf) ){
				return false;
			}
			delete pCode;
			pBuf = (char*)cmemBuf.GetRawPtr();
			nTextByteLen = cmemBuf.GetRawLength();
		}
 	}
	nEndMode = GetEndModeByMode(nMode, nEndMode);
	size_t nulLen = 0;
	switch( nEndMode ){
	case 1: nulLen = 1; break;
	case 2: nulLen = 2; break;
	case 4: nulLen = 4; break;
	case 0: nulLen = 0; break;
	default: nulLen = 0; break;
	}
	HGLOBAL hgClipText = GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nTextByteLen + nulLen
	);
	if( !hgClipText ){
		return false;
	}
	char* pszClip = static_cast<char*>(::GlobalLock(hgClipText));
	memcpy( pszClip, pBuf, nTextByteLen );
	if( nulLen ){
		memset( &pszClip[nTextByteLen], 0, nulLen );
	}
	::GlobalUnlock( hgClipText );
	SetClipboardData( uFormat, hgClipText );

	return true;
}

static int GetLengthByMode(HGLOBAL hClipData, const BYTE* pData, int nMode, int nEndMode)
{
	size_t nMemLength = ::GlobalSize(hClipData);
	nEndMode = GetEndModeByMode(nMode, nEndMode);
	size_t nLength;
	if( nEndMode == 1 ) {
		nLength = strnlen((const char *)pData, nMemLength);
	}else if( nEndMode == 2 ){
		nLength = wcsnlen((const wchar_t *)pData, nMemLength / 2) * 2;
	}else if( nEndMode == 4 ){
		const wchar32_t* pData32 = (const wchar32_t*)pData;
		const size_t len = nMemLength / 4;
		nLength = 0;
		while( pData32[nLength] != 0 && nLength < len ){
			nLength++;
		}
		nLength *= 4;
	}else{
		nLength = nMemLength;
	}
	return nLength;
}

/*!
	指定のクリップボード形式で取得
	
	@param nMode -2:通常のサクラの処理, -1:バイナリモード, それ以外:文字コード
	@param nEndMode -1:文字コードに依存 0:GlobalSize 1:strlen 2:wcslen 4:wchar32_tの文字列
	@date 2013.06.12 Moca 新規作成
*/
bool CClipboard::GetClipboardByFormat(CNativeW& mem, const wchar_t* pFormatName, int nMode, int nEndMode, const CEol& cEol)
{
	mem.SetString(L"");
	CLIPFORMAT uFormat = GetClipFormat(pFormatName);
	if( uFormat == (CLIPFORMAT)-1 ){
		return false;
	}
	if( !IsClipboardFormatAvailable(uFormat) ){
		return false;
	}
	if( nMode == -2 ){
		bool bret = false;
		if( -1 != GetDataType() ){
			bret = GetText(&mem, nullptr, nullptr, cEol, uFormat);
			if( !bret ){
				mem.SetString(L"");
			}
		}
		return bret;
	}
	HGLOBAL hClipData = GetClipboardData( uFormat );
	if( hClipData != nullptr ){
		bool retVal = true;
		const BYTE* pData = (BYTE*)GlobalLock( hClipData );
		if( pData == nullptr ){
			return false;
		}

		// 長さオプションの解釈
		size_t nLength = GetLengthByMode(hClipData, pData, nMode, nEndMode);

		// エンコードオプション
		if( nMode == -1 ){
			// バイナリモード。1byteをU+00-U+ffにマッピング
			mem.AllocStringBuffer(nLength);
			mem._SetStringLength(nLength);
			wchar_t* pBuf = (wchar_t *)mem.GetStringPtr();
			for( size_t i = 0; i < nLength; i++ ){
				pBuf[i] = (unsigned char)pData[i];
			}
		}else{
			ECodeType eMode = (ECodeType)nMode;
			if( !IsValidCodeType(eMode) ){
				{
					const STypeConfig& type = CEditDoc::GetInstance(0)->m_cDocType.GetDocumentAttribute();
					CCodeMediator mediator(type.m_encoding);
					eMode = mediator.CheckKanjiCode((const char*)pData, nLength);
				}
				if( !IsValidCodeType(eMode) ){
					eMode = CODE_DEFAULT;
				}
				if( -1 == nEndMode ){
					// nLength 再設定
					nLength = GetLengthByMode(hClipData, pData, eMode, nEndMode);
				}
			}
			if( eMode == CODE_UNICODE ){
				mem.SetString((wchar_t *)pData, nLength / sizeof(wchar_t));
			}else{
				CMemory cmem;
				cmem.SetRawData(pData, nLength);
				if( nullptr != cmem.GetRawPtr() ){
					CCodeBase* pCode = CCodeFactory::CreateCodeBase(eMode, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode());
					if( RESULT_FAILURE == pCode->CodeToUnicode(cmem, &mem) ){
						mem.SetString(L"");
						retVal = false;
					}
					delete pCode;
				}
			}
		}
		::GlobalUnlock(hClipData);
		return retVal;
	}
	return false;
}

//! クリップボード内に、サクラエディタで扱えるデータがあればtrue
bool CClipboard::HasValidData()
{
	//扱える形式が１つでもあればtrue
	if(::IsClipboardFormatAvailable(CF_OEMTEXT))return true;
	if(::IsClipboardFormatAvailable(CF_UNICODETEXT))return true;
	if(::IsClipboardFormatAvailable(GetSakuraFormat()))return true;
	/* 2008.09.10 bosagami パス貼り付け対応 */
	if(::IsClipboardFormatAvailable(CF_HDROP))return true;
	return false;
}

//!< サクラエディタ独自のクリップボードデータ形式
CLIPFORMAT CClipboard::GetSakuraFormat()
{
	/*
		2007.09.30 kobake

		UNICODE形式でクリップボードデータを保持するよう変更したため、
		以前のバージョンのクリップボードデータと競合しないように
		フォーマット名を変更
	*/
	return (CLIPFORMAT)::RegisterClipboardFormat( L"SAKURAClipW" );
}

//!< クリップボードデータ形式(CF_UNICODETEXT等)の取得
int CClipboard::GetDataType() const
{
	//扱える形式が１つでもあればtrue
	// 2013.06.11 GetTextの取得順に変更
	if(IsClipboardFormatAvailable(GetSakuraFormat()))return GetSakuraFormat();
	if(IsClipboardFormatAvailable(CF_UNICODETEXT))return CF_UNICODETEXT;
	if(IsClipboardFormatAvailable(CF_OEMTEXT))return CF_OEMTEXT;
	if(IsClipboardFormatAvailable(CF_HDROP))return CF_HDROP;
	return -1;
}

HANDLE CClipboard::SetClipboardData(UINT uFormat, HANDLE hMem) const {
	return ::SetClipboardData(uFormat, hMem);
}

HANDLE CClipboard::GetClipboardData(UINT uFormat) const {
	return ::GetClipboardData(uFormat);
}

BOOL CClipboard::EmptyClipboard() const {
	return ::EmptyClipboard();
}

BOOL CClipboard::IsClipboardFormatAvailable(UINT format) const {
	return ::IsClipboardFormatAvailable(format);
}

UINT CClipboard::EnumClipboardFormats(UINT format) const {
	return ::EnumClipboardFormats(format);
}

HGLOBAL CClipboard::GlobalAlloc(UINT uFlags, SIZE_T dwBytes) const {
	return ::GlobalAlloc(uFlags, dwBytes);
}

LPVOID CClipboard::GlobalLock(HGLOBAL hMem) const {
	return ::GlobalLock(hMem);
}
