// 2007.10.19 kobake
// �Ȃ񂩃V�F�����ۂ��@�\�̊֐��Q

#pragma once


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
BOOL MySelectFont( LOGFONT* plf, INT* piPointSize, HWND hwndDlgOwner );	// 2009.10.01 ryoji �|�C���g�T�C�Y�i1/10�|�C���g�P�ʁj�����ǉ�
