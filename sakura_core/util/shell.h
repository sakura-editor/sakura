// 2007.10.19 kobake
// �Ȃ񂩃V�F�����ۂ��@�\�̊֐��Q
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
#ifndef SAKURA_SHELL_A129670C_6564_4E0D_AF52_E323B0C7CA099_H_
#define SAKURA_SHELL_A129670C_6564_4E0D_AF52_E323B0C7CA099_H_

BOOL MyWinHelp(HWND hwndCaller, LPCTSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);	/* WinHelp �̂����� HtmlHelp ���Ăяo�� */	// 2006.07.22 ryoji

/* Shell Interface�n(?) */
SAKURA_CORE_API BOOL SelectDir(HWND, const TCHAR*, const TCHAR*, TCHAR* );	/* �t�H���_�I���_�C�A���O */
SAKURA_CORE_API BOOL ResolveShortcutLink(HWND hwnd, LPCTSTR lpszLinkFile, LPTSTR lpszPath);/* �V���[�g�J�b�g(.lnk)�̉��� */

SAKURA_CORE_API HWND OpenHtmlHelp( HWND hWnd, LPCTSTR szFile, UINT uCmd, DWORD_PTR data,bool msgflag = true);
SAKURA_CORE_API DWORD NetConnect ( const TCHAR strNetWorkPass[] );

/* �w���v�̖ڎ���\�� */
SAKURA_CORE_API void ShowWinHelpContents( HWND hwnd, LPCTSTR lpszHelp );

BOOL GetSpecialFolderPath( int nFolder, LPTSTR pszPath );	// ����t�H���_�̃p�X���擾����	// 2007.05.19 ryoji



int MyPropertySheet( LPPROPSHEETHEADER lppsph );	// �Ǝ��g���v���p�e�B�V�[�g	// 2007.05.24 ryoji


//!�t�H���g�I���_�C�A���O
BOOL MySelectFont( LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner, bool );	// 2009.10.01 ryoji �|�C���g�T�C�Y�i1/10�|�C���g�P�ʁj�����ǉ�

#endif /* SAKURA_SHELL_A129670C_6564_4E0D_AF52_E323B0C7CA099_H_ */
/*[EOF]*/
