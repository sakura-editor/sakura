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

#ifndef _SAKURA_UXTHEME_H_
#define _SAKURA_UXTHEME_H_

#if defined(_MSC_VER) && _MSC_VER>=1500
#include <vsstyle.h>
#else
typedef HANDLE HTHEME;
enum TABPARTS {
	TABP_TABITEM = 1,
	TABP_TABITEMLEFTEDGE = 2,
	TABP_TABITEMRIGHTEDGE = 3,
	TABP_TABITEMBOTHEDGE = 4,
	TABP_TOPTABITEM = 5,
	TABP_TOPTABITEMLEFTEDGE = 6,
	TABP_TOPTABITEMRIGHTEDGE = 7,
	TABP_TOPTABITEMBOTHEDGE = 8,
	TABP_PANE = 9,
	TABP_BODY = 10,
	TABP_AEROWIZARDBODY = 11,
};
enum TABITEMSTATES {
	TIS_NORMAL = 1,
	TIS_HOT = 2,
	TIS_SELECTED = 3,
	TIS_DISABLED = 4,
	TIS_FOCUSED = 5,
};
#endif
#include "CDllHandler.h"
#include "util/design_template.h"

/*!
	@brief UxTheme 動的ロード

	UxTheme コンポーネントの動的ロードをサポートするクラス
*/
class CUxTheme : public TSingleton<CUxTheme>, public CDllImp {
	friend class TSingleton<CUxTheme>;
	CUxTheme();
	virtual ~CUxTheme();

protected:
	bool m_bInitialized;

	bool InitThemeDll( TCHAR* str = NULL );
	virtual bool InitDllImp();
	virtual LPCTSTR GetDllNameImp(int nIndex);

protected:
	// UxTheme API Entry Points
	BOOL (WINAPI* m_pfnIsThemeActive)( VOID );
	HRESULT (WINAPI* m_pfnSetWindowTheme)( HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList );
	HTHEME (WINAPI* m_pfnOpenThemeData)( HWND hwnd, LPCWSTR pszClassList );
	HRESULT (WINAPI* m_pfnDrawThemeBackground)( HTHEME htheme, HDC hdc, int iPartId, int iStateId, RECT* prc, RECT* prcClip );
	HRESULT (WINAPI* m_pfnDrawThemeParentBackground)( HWND hwnd, HDC hdc, RECT* prc );
	BOOL (WINAPI* m_pfnIsThemeBackgroundPartiallyTransparent)( HTHEME htheme, int iPartId, int iStateId );

public:
	// UxTheme API Wrapper Functions
	BOOL IsThemeActive( VOID );
	HRESULT SetWindowTheme( HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList );
	HTHEME OpenThemeData( HWND hwnd, LPCWSTR pszClassList );
	HRESULT DrawThemeBackground( HTHEME htheme, HDC hdc, int iPartId, int iStateId, RECT* prc, RECT* prcClip );
	HRESULT DrawThemeParentBackground( HWND hwnd, HDC hdc, RECT* prc );
	BOOL IsThemeBackgroundPartiallyTransparent( HTHEME htheme, int iPartId, int iStateId );
};

#endif



