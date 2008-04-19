/*!	@file
	@brief UxTheme 動的ロード

	UxTheme (Windows thmeme manager) への動的アクセスクラス

	@author ryoji
	@date Apr. 1, 2007
*/
/*
	Copyright (C) 2007, ryoji

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

#include "stdafx.h"
#include "CUxTheme.h"

CUxTheme CUxTheme::m_cUxTheme;	// Singleton

CUxTheme::CUxTheme()
 : m_bInitialized(false)
{
}

CUxTheme::~CUxTheme()
{
}


/*! DLL のロード

	一度しか LoadLibrary() 実行しないこと以外は CDllHandler::Init() と同じ
	（UxTheme 未対応 OS での LoadLibrary() 失敗の繰返しを防ぐ）

	@author ryoji
	@date 2007.04.01 ryoji 新規
*/
bool CUxTheme::Init( TCHAR* str )
{
	if( m_bInitialized )
		return IsAvailable();

	m_bInitialized = true;
	return CDllHandler::Init( str );
}

/*!
	UxTheme のファイル名を渡す
*/
LPCTSTR CUxTheme::GetDllName( LPCTSTR )
{
	return _T("UxTheme.dll");
}

int CUxTheme::InitDll(void)
{
	const ImportTable table[] = {
		{ &m_pfnIsThemeActive,		"IsThemeActive" },
		{ &m_pfnSetWindowTheme,		"SetWindowTheme" },
		{ NULL, 0 }
	};

	if( !RegisterEntries( table ) ){
		return 1;
	}

	return 0;
}

/*! IsThemeActive API Wrapper */
BOOL CUxTheme::IsThemeActive( VOID )
{
	if( !Init() )
		return FALSE;
	return m_pfnIsThemeActive();
}

/*! SetWindowTheme API Wrapper */
HRESULT CUxTheme::SetWindowTheme( HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList )
{
	if( !Init() )
		return S_FALSE;
	return m_pfnSetWindowTheme( hwnd, pszSubAppName, pszSubIdList );
}


