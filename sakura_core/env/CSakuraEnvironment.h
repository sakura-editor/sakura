#pragma once

#include <string>

class CSakuraEnvironment{
public:
	static CEditWnd* GetMainWindow();
	static void ExpandParameter(const wchar_t* pszSource, wchar_t* pszBuffer, int nBufferLen);
	static std::tstring GetDlgInitialDir();

	static void ResolvePath(TCHAR* pszPath); //!< ショートカットの解決とロングファイル名へ変換を行う。
private:
	static const wchar_t* _ExParam_SkipCond(const wchar_t* pszSource, int part); // Mar. 31, 2003 genta ExpandParameter補助関数
	static int _ExParam_Evaluate( const wchar_t* pCond );
};


//ウィンドウ管理
/* 指定ウィンドウが、編集ウィンドウのフレームウィンドウかどうか調べる */
BOOL IsSakuraMainWindow( HWND hWnd );

