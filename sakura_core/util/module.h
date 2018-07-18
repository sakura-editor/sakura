/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_MODULE_4F382EF5_EF52_47E1_A774_5CDFB545AB25_H_
#define SAKURA_MODULE_4F382EF5_EF52_47E1_A774_5CDFB545AB25_H_

void GetAppVersionInfo( HINSTANCE, int, DWORD*, DWORD* );	/* リソースから製品バージョンの取得 */

HICON GetAppIcon( HINSTANCE hInst, int nResource, const TCHAR* szFile, bool bSmall = false);

DWORD GetDllVersion( LPCTSTR lpszDllName );	// シェルやコモンコントロール DLL のバージョン番号を取得	// 2006.06.17 ryoji

void ChangeCurrentDirectoryToExeDir();

//! カレントディレクトリ移動機能付LoadLibrary
HMODULE LoadLibraryExedir( LPCTSTR pszDll);

#endif /* SAKURA_MODULE_4F382EF5_EF52_47E1_A774_5CDFB545AB25_H_ */
/*[EOF]*/
