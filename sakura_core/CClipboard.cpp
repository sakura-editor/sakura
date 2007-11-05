#include "stdafx.h"
#include "CClipboard.h"
#include "charset/CShiftJis.h"

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
	::EmptyClipboard();
}

void CClipboard::Close()
{
	if(m_bOpenResult){
		::CloseClipboard();
		m_bOpenResult=FALSE;
	}
}

bool CClipboard::SetText(const wchar_t* pszText,bool bColmnSelect, bool bLineSelect)
{
	UINT		uFormat;

	// ヌル終端までの長さ
	int nTextLen = wcslen( pszText );

	/*
	// テキスト形式のデータ (CF_OEMTEXT)
	HGLOBAL hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nTextLen + 1
	);
	if( hgClipText ){
		char* pszClip = GlobalLockChar( hgClipText );
		memcpy( pszClip, pszText, nTextLen );
		pszClip[nTextLen] = '\0';
		::GlobalUnlock( hgClipText );
		::SetClipboardData( CF_OEMTEXT, hgClipText );
	}
	*/
	// UNICODE形式のデータ (CF_UNICODETEXT)
	HGLOBAL hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		(nTextLen + 1) * sizeof(wchar_t)
	);
	if( hgClipText ){
		wchar_t* pszClip = GlobalLockWChar( hgClipText );
		wmemcpy( pszClip, pszText, nTextLen + 1 );
		::GlobalUnlock( hgClipText );
		::SetClipboardData( CF_UNICODETEXT, hgClipText );
	}

	// バイナリ形式のデータ
	//	(int) 「データ」の長さ
	//	「データ」
	UINT	uFormatSakuraClip = CClipboard::GetSakuraFormat();
	HGLOBAL	hgClipSakura = NULL;
	if( 0 != uFormatSakuraClip ){
		hgClipSakura = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			sizeof(int) + (nTextLen + 1) * sizeof(wchar_t)
		);
		if( hgClipSakura ){
			BYTE* pClip = GlobalLockBYTE( hgClipSakura );
			*((int*)pClip) = nTextLen;
			wmemcpy( (wchar_t*)(pClip + sizeof(int)), pszText, nTextLen + 1 );
			::GlobalUnlock( hgClipSakura );
			::SetClipboardData( uFormatSakuraClip, hgClipSakura );
		}
	}

	// 矩形選択を示すダミーデータ
	HGLOBAL hgClipMSDEVColm = NULL;
	if( bColmnSelect ){
		uFormat = ::RegisterClipboardFormat( _T("MSDEVColumnSelect") );
		if( 0 != uFormat ){
			hgClipMSDEVColm = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVColm ){
				BYTE* pClip = GlobalLockBYTE( hgClipMSDEVColm );
				pClip[0] = 0;
				::GlobalUnlock( hgClipMSDEVColm );
				::SetClipboardData( uFormat, hgClipMSDEVColm );
			}
		}
	}

	/* 行選択を示すダミーデータ */
	HGLOBAL		hgClipMSDEVLine = NULL;
	if( bLineSelect ){
		uFormat = ::RegisterClipboardFormat( _T("MSDEVLineSelect") );
		if( 0 != uFormat ){
			hgClipMSDEVLine = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVLine ){
				BYTE* pClip = (BYTE*)::GlobalLock( hgClipMSDEVLine );
				pClip[0] = 0x01;
				::GlobalUnlock( hgClipMSDEVLine );
				::SetClipboardData( uFormat, hgClipMSDEVLine );
			}
		}
	}

	if( bColmnSelect && !hgClipMSDEVColm ){
		return FALSE;
	}
	if( bLineSelect && !hgClipMSDEVLine ){
		return FALSE;
	}
	if( !hgClipText && !hgClipSakura ){
		return FALSE;
	}
	return true;
}

//! テキストを取得する
bool CClipboard::GetText(CNativeW2* cmemBuf, BOOL* pbColmnSelect, BOOL* pbLineSelect)
{
	if( NULL != pbColmnSelect ){
		*pbColmnSelect = FALSE;
	}

	//矩形選択のデータがあれば取得
	if( NULL != pbColmnSelect ){
		// 矩形選択のテキストデータがクリップボードにあるか
		UINT uFormat = 0;
		while( uFormat = ::EnumClipboardFormats( uFormat ) ){
			// Jul. 2, 2005 genta : check return value of GetClipboardFormatName
			TCHAR szFormatName[128];
			if( ::GetClipboardFormatName( uFormat, szFormatName, _countof(szFormatName) - 1 ) ){
				if( NULL != pbColmnSelect && 0 == lstrcmp( _T("MSDEVColumnSelect"), szFormatName ) ){
					*pbColmnSelect = TRUE;
					break;
				}
				if( NULL != pbLineSelect && 0 == lstrcmp( _T("MSDEVLineSelect"), szFormatName ) ){
					*pbLineSelect = TRUE;
					break;
				}
			}
		}
	}
	
	//サクラ形式のデータがあれば取得
	UINT uFormatSakuraClip = CClipboard::GetSakuraFormat();
	if( ::IsClipboardFormatAvailable( uFormatSakuraClip ) ){
		HGLOBAL hSakura = ::GetClipboardData( uFormatSakuraClip );
		if (hSakura != NULL) {
			BYTE* pData = (BYTE*)::GlobalLock(hSakura);
			size_t nLength        = *((int*)pData);
			const wchar_t* szData = (const wchar_t*)(pData + sizeof(int));
			cmemBuf->SetString( szData, nLength );
			::GlobalUnlock(hSakura);
			::CloseClipboard();
			return TRUE;
		}
	}

	//UNICODE形式のデータがあれば取得
	// From Here 2005/05/29 novice UNICODE TEXT 対応処理を追加
	HGLOBAL hUnicode = ::GetClipboardData( CF_UNICODETEXT );
	if( hUnicode != NULL ){
		wchar_t* szData = GlobalLockWChar(hUnicode);
		cmemBuf->SetString( szData );
		::GlobalUnlock(hUnicode);
		::CloseClipboard();
		return TRUE;
	}
	//	To Here 2005/05/29 novice

	//OEMTEXT形式のデータがあれば取得
	HGLOBAL hText = ::GetClipboardData( CF_OEMTEXT );
	if( hText != NULL ){
		char* szData = GlobalLockChar(hText);
		//SJIS→UNICODE
		CMemory cmemSjis( szData, GlobalSize(szData) );
		CShiftJis::SJISToUnicode(&cmemSjis);
		cmemBuf->SetString( reinterpret_cast<const wchar_t*>(cmemSjis.GetRawPtr()) );
		::GlobalUnlock(hText);
		::CloseClipboard();
		return TRUE;
	}

	::CloseClipboard();
	return FALSE;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  staticインターフェース                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


//! クリップボード内に、サクラエディタで扱えるデータがあればtrue
bool CClipboard::HasValidData()
{
	//扱える形式が１つでもあればtrue
	if(::IsClipboardFormatAvailable(CF_OEMTEXT))return true;
	if(::IsClipboardFormatAvailable(CF_UNICODETEXT))return true;
	if(::IsClipboardFormatAvailable(GetSakuraFormat()))return true;
	return false;
}

//!< サクラエディタ独自のクリップボードデータ形式
UINT CClipboard::GetSakuraFormat()
{
	/*
		2007.09.30 kobake

		UNICODE形式でクリップボードデータを保持するよう変更したため、
		以前のバージョンのクリップボードデータと競合しないように
		フォーマット名を変更
	*/
	return ::RegisterClipboardFormat( _T("SAKURAClipW") );
}

