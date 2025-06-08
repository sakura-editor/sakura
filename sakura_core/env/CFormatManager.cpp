/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "DLLSHAREDATA.h"

#include "CFormatManager.h"
#include "CSelectLang.h"

/*! 日付をフォーマット
	systime：時刻データ
	
	pszDest：フォーマット済みテキスト格納用バッファ
	nDestLen：pszDestの長さ
	
	pszDateFormat：
		カスタムのときのフォーマット
*/
const WCHAR* CFormatManager::MyGetDateFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen )
{
	return MyGetDateFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_Common.m_sFormat.m_nDateFormatType,
		m_pShareData->m_Common.m_sFormat.m_szDateFormat
	);
}

const WCHAR* CFormatManager::MyGetDateFormat(
	const SYSTEMTIME&		systime,
	WCHAR*		pszDest,
	int				nDestLen,
	int				nDateFormatType,
	const WCHAR*	szDateFormat
)
{
	const WCHAR* pszForm;
	DWORD dwFlags;
	if( 0 == nDateFormatType ){
		dwFlags = DATE_LONGDATE;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szDateFormat;
	}
	::GetDateFormat( CSelectLang::getDefaultLangId(), dwFlags, &systime, pszForm, pszDest, nDestLen );
	return pszDest;
}

/* 時刻をフォーマット */
const WCHAR* CFormatManager::MyGetTimeFormat( const SYSTEMTIME& systime, WCHAR* pszDest, int nDestLen )
{
	return MyGetTimeFormat(
		systime,
		pszDest,
		nDestLen,
		m_pShareData->m_Common.m_sFormat.m_nTimeFormatType,
		m_pShareData->m_Common.m_sFormat.m_szTimeFormat
	);
}

/* 時刻をフォーマット */
const WCHAR* CFormatManager::MyGetTimeFormat(
	const SYSTEMTIME&	systime,
	WCHAR*			pszDest,
	int					nDestLen,
	int					nTimeFormatType,
	const WCHAR*		szTimeFormat
)
{
	const WCHAR* pszForm;
	DWORD dwFlags;
	if( 0 == nTimeFormatType ){
		dwFlags = 0;
		pszForm = NULL;
	}else{
		dwFlags = 0;
		pszForm = szTimeFormat;
	}
	::GetTimeFormat(CSelectLang::getDefaultLangId(), dwFlags, &systime, pszForm, pszDest, nDestLen);
	return pszDest;
}
