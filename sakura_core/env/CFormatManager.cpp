﻿/*! @file */
/*
	2008.05.18 kobake CShareData から分離
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
