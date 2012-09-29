/*!	@file
	@brief UxTheme ���I���[�h

	UxTheme (Windows thmeme manager) �ւ̓��I�A�N�Z�X�N���X

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

#include "CDllHandler.h"

/*!
	@brief UxTheme ���I���[�h

	UxTheme �R���|�[�l���g�̓��I���[�h���T�|�[�g����N���X
*/
class SAKURA_CORE_API CUxTheme : public TSingleton<CUxTheme>, public CDllImp {
public:
	friend class TSingleton<CUxTheme>;
private:
	CUxTheme();

public:
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

public:
	// UxTheme API Wrapper Functions
	BOOL IsThemeActive( VOID );
	HRESULT SetWindowTheme( HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList );
};

#endif



