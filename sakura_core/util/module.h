/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_MODULE_865764F5_F6C8_4B99_B2DB_A29F375539FE_H_
#define SAKURA_MODULE_865764F5_F6C8_4B99_B2DB_A29F375539FE_H_
#pragma once

void GetAppVersionInfo( HINSTANCE hInstance, int nVersionResourceID,
					    DWORD* pdwProductVersionMS, DWORD* pdwProductVersionLS );	/* リソースから製品バージョンの取得 */

HICON GetAppIcon( HINSTANCE hInst, int nResource, const WCHAR* szFile, bool bSmall = false);

DWORD GetDllVersion( LPCWSTR lpszDllName );	// シェルやコモンコントロール DLL のバージョン番号を取得	// 2006.06.17 ryoji

void ChangeCurrentDirectoryToExeDir();

//! カレントディレクトリ移動機能付LoadLibrary
HMODULE LoadLibraryExedir( LPCWSTR pszDll);
#endif /* SAKURA_MODULE_865764F5_F6C8_4B99_B2DB_A29F375539FE_H_ */
