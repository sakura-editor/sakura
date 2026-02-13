/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSAKURAENVIRONMENT_4B226B3A_5208_4C29_9D2E_E42DA8EFD875_H_
#define SAKURA_CSAKURAENVIRONMENT_4B226B3A_5208_4C29_9D2E_E42DA8EFD875_H_
#pragma once

#include <string>

class CEditWnd;

class CSakuraEnvironment{
public:
	static CEditWnd* GetMainWindow();
	static void ExpandParameter(const wchar_t* pszSource, wchar_t* pszBuffer, int nBufferLen);
	static std::wstring GetDlgInitialDir(bool bControlProcess = false);

	static void ResolvePath(WCHAR* pszPath); //!< ショートカットの解決とロングファイル名へ変換を行う。
private:
	static const wchar_t* _ExParam_SkipCond(const wchar_t* pszSource, int part); // Mar. 31, 2003 genta ExpandParameter補助関数
	static int _ExParam_Evaluate( const wchar_t* pCond );
};

//ウィンドウ管理
/* 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる */
BOOL IsSakuraMainWindow( HWND hWnd );
#endif /* SAKURA_CSAKURAENVIRONMENT_4B226B3A_5208_4C29_9D2E_E42DA8EFD875_H_ */
