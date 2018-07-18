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
#ifndef SAKURA_CSAKURAENVIRONMENT_95CFC65B_9C67_4ABF_A681_5A4652A3C5D3_H_
#define SAKURA_CSAKURAENVIRONMENT_95CFC65B_9C67_4ABF_A681_5A4652A3C5D3_H_

#include <string>

class CEditWnd;

class CSakuraEnvironment{
public:
	static CEditWnd* GetMainWindow();
	static void ExpandParameter(const wchar_t* pszSource, wchar_t* pszBuffer, int nBufferLen);
	static std::tstring GetDlgInitialDir(bool bControlProcess = false);

	static void ResolvePath(TCHAR* pszPath); //!< ショートカットの解決とロングファイル名へ変換を行う。
private:
	static const wchar_t* _ExParam_SkipCond(const wchar_t* pszSource, int part); // Mar. 31, 2003 genta ExpandParameter補助関数
	static int _ExParam_Evaluate( const wchar_t* pCond );
};


//ウィンドウ管理
/* 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる */
BOOL IsSakuraMainWindow( HWND hWnd );


#endif /* SAKURA_CSAKURAENVIRONMENT_95CFC65B_9C67_4ABF_A681_5A4652A3C5D3_H_ */
/*[EOF]*/
