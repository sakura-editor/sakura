#pragma once

#include <string>

class CSakuraEnvironment{
public:
	static CEditWnd* GetMainWindow();
	static void ExpandParameter(const wchar_t* pszSource, wchar_t* pszBuffer, int nBufferLen);
	static std::tstring GetDlgInitialDir();

	static void ResolvePath(TCHAR* pszPath); //!< �V���[�g�J�b�g�̉����ƃ����O�t�@�C�����֕ϊ����s���B
private:
	static const wchar_t* _ExParam_SkipCond(const wchar_t* pszSource, int part); // Mar. 31, 2003 genta ExpandParameter�⏕�֐�
	static int _ExParam_Evaluate( const wchar_t* pCond );
};
