//	$Id$
/*!	@file
	�v���Z�X�ԋ��L�f�[�^�ւ̃A�N�Z�X

	@author Norio Nakatani
	@date 1998/05/26  �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*! @file
	�v���Z�X�ԋ��L�f�[�^�ւ̃A�N�Z�X

	@author Norio Nakatani
	@date May. 26, 1998
*/

//#include <stdio.h>
#include <io.h>
#include "CShareData.h"
#include "CEditApp.h"
#include "mymessage.h"
#include "debug.h"
#include "global.h"
#include "etc_uty.h"
struct ARRHEAD {
	int		nLength;
	int		nItemNum;
};

//!	���L�������̃o�[�W����
/*!
	���L�������̃o�[�W�����ԍ��B���L�������̌`����ύX�����Ƃ��͂�����1���₷�B

	���̒l�͋��L�������̃o�[�W�����t�B�[���h�Ɋi�[����A�قȂ�\���̋��L��������
	�g���G�f�B�^�������ɋN�����Ȃ��悤�ɂ���B

	�ݒ�\�Ȓl�� 1�`unsinged int�̍ő�l

	@sa Init()
*/
const unsigned int uShareDataVersion = 15;

/*!
	���L�������̈悪����ꍇ�̓v���Z�X�̃A�h���X��Ԃ���
	���łɃ}�b�v����Ă���t�@�C�� �r���[���A���}�b�v����B
*/
CShareData::~CShareData()
{
	if( NULL != m_pShareData ){
		/* �v���Z�X�̃A�h���X��Ԃ��� ���łɃ}�b�v����Ă���t�@�C�� �r���[���A���}�b�v���܂� */
		::UnmapViewOfFile( m_pShareData );
		m_pShareData = NULL;
	}
	return;
}


//! CShareData�N���X�̏���������
/*!
	CShareData�N���X�𗘗p����O�ɕK���Ăяo�����ƁB

	@retval true ����������
	@retval false ���������s

	@note ���ɑ��݂��鋤�L�������̃o�[�W���������̃G�f�B�^���g�����̂�
	�قȂ�ꍇ�͒v���I�G���[��h�����߂�false��Ԃ��܂��BWinMain()
	��Init()�Ɏ��s����ƃ��b�Z�[�W���o���ăG�f�B�^�̋N���𒆎~���܂��B
*/
bool CShareData::Init( void )
{
	int		i;
	int		j;
	char	szExeFolder[_MAX_PATH + 1];
	char	szPath[_MAX_PATH + 1];
//	int		nCharChars;
//	char	szDrive[_MAX_DRIVE];
//	char	szDir[_MAX_DIR];

	/* exe�̂���t�H���_ */
	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, sizeof( szPath )
	);
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, szExeFolder, NULL );
	strcat( szExeFolder, "\\" );

	/* �t�@�C���}�b�s���O�I�u�W�F�N�g */
	m_hFileMap = ::CreateFileMapping(
		(HANDLE)0xFFFFFFFF,
		NULL,
		PAGE_READWRITE | SEC_COMMIT,
		0,
		sizeof( DLLSHAREDATA ),
		m_pszAppName
	);
	if( NULL == m_hFileMap ){
		::MessageBox(
			NULL,
			"CreateFileMapping()�Ɏ��s���܂���",
			"�\�����ʃG���[",
			MB_OK | MB_APPLMODAL | MB_ICONSTOP
		);
		return false;
	}
	if( GetLastError() != ERROR_ALREADY_EXISTS ){
		/* �I�u�W�F�N�g�����݂��Ă��Ȃ������ꍇ */
		/* �t�@�C���̃r���[�� �Ăяo�����v���Z�X�̃A�h���X��ԂɃ}�b�v���܂� */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		m_pShareData->m_vStructureVersion = uShareDataVersion;
		m_pShareData->m_CKeyMacroMgr.Clear();			/* �L�[���[�h�}�N���̃o�b�t�@ */
		m_pShareData->m_bRecordingKeyMacro = FALSE;		/* �L�[�{�[�h�}�N���̋L�^�� */
		m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
		m_pShareData->m_dwProductVersionMS = 0L;
		m_pShareData->m_dwProductVersionLS = 0L;
		m_pShareData->m_hwndTray = NULL;
		m_pShareData->m_hAccel = NULL;
		m_pShareData->m_hwndDebug = NULL;
		m_pShareData->m_nSequences = 0;					/* �E�B���h�E�A�� */
		m_pShareData->m_nEditArrNum = 0;
		m_pShareData->m_Common.m_nMRUArrNum_MAX = 15;	/* �t�@�C���̗���MAX */	//Oct. 14, 2000 JEPRO �������₵��(10��15)
		m_pShareData->m_nMRUArrNum = 0;
		for( i = 0; i < MAX_MRU; ++i ){
			m_pShareData->m_fiMRUArr[i].m_nViewTopLine = 0;
			m_pShareData->m_fiMRUArr[i].m_nViewLeftCol = 0;
//			m_pShareData->m_fiMRUArr[i].m_nCaretPosX = 0;
//			m_pShareData->m_fiMRUArr[i].m_nCaretPosY = 0;
			m_pShareData->m_fiMRUArr[i].m_nX = 0;
			m_pShareData->m_fiMRUArr[i].m_nY = 0;
			m_pShareData->m_fiMRUArr[i].m_bIsModified = 0;
			m_pShareData->m_fiMRUArr[i].m_nCharCode = 0;
			strcpy( m_pShareData->m_fiMRUArr[i].m_szPath, "" );
		}

		m_pShareData->m_Common.m_nOPENFOLDERArrNum_MAX = 15;	/* �t�H���_�̗���MAX */	//Oct. 14, 2000 JEPRO �������₵��(10��15)
		m_pShareData->m_nOPENFOLDERArrNum = 0;
		for( i = 0; i < MAX_OPENFOLDER; ++i ){
			strcpy( m_pShareData->m_szOPENFOLDERArr[i], "" );
		}

		m_pShareData->m_nSEARCHKEYArrNum = 0;
		for( i = 0; i < MAX_SEARCHKEY; ++i ){
			strcpy( m_pShareData->m_szSEARCHKEYArr[i], "" );
		}
		m_pShareData->m_nREPLACEKEYArrNum = 0;
		for( i = 0; i < MAX_REPLACEKEY; ++i ){
			strcpy( m_pShareData->m_szREPLACEKEYArr[i], "" );
		}
		m_pShareData->m_nGREPFILEArrNum = 0;
		for( i = 0; i < MAX_GREPFILE; ++i ){
			strcpy( m_pShareData->m_szGREPFILEArr[i], "" );
		}
		m_pShareData->m_nGREPFILEArrNum = 1;
		strcpy( m_pShareData->m_szGREPFILEArr[0], "*.*" );

		m_pShareData->m_nGREPFOLDERArrNum = 0;
		for( i = 0; i < MAX_GREPFOLDER; ++i ){
			strcpy( m_pShareData->m_szGREPFOLDERArr[i], "" );
		}
		strcpy( m_pShareData->m_szMACROFOLDER, szExeFolder );	/* �}�N���p�t�H���_ */
		strcpy( m_pShareData->m_szIMPORTFOLDER, szExeFolder );	/* �ݒ�C���|�[�g�p�t�H���_ */

		i = 0;
		wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, "����ݒ� %d", i + 1 );	/* ����ݒ�̖��O */
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceHan, "�l�r ����" );				/* ����t�H���g */
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szPrintFontFaceZen, "�l�r ����" );				/* ����t�H���g */
		m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth = 12;  								/* ����t�H���g��(1/10mm�P��) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintFontHeight = m_pShareData->m_PrintSettingArr[i].m_nPrintFontWidth * 2;	/* ����t�H���g����(1/10mm�P�ʒP��) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintDansuu = 1;			/* �i�g�̒i�� */
		m_pShareData->m_PrintSettingArr[i].m_nPrintDanSpace = 70; 		/* �i�ƒi�̌���(1/10mm) */
		m_pShareData->m_PrintSettingArr[i].m_bPrintWordWrap = TRUE;		/* �p�����[�h���b�v���� */
		m_pShareData->m_PrintSettingArr[i].m_bPrintLineNumber = FALSE;	/* �s�ԍ���������� */
		m_pShareData->m_PrintSettingArr[i].m_nPrintLineSpacing = 30;	/* ����t�H���g�s�� �����̍����ɑ΂��銄��(%) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintMarginTY = 100;		/* ����p���}�[�W�� ��(1/10mm�P��) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintMarginBY = 200;		/* ����p���}�[�W�� ��(1/10mm�P��) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintMarginLX = 200;		/* ����p���}�[�W�� ��(1/10mm�P��) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintMarginRX = 100;		/* ����p���}�[�W�� �E(1/10mm�P��) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintPaperOrientation = DMORIENT_PORTRAIT;	/* �p������ DMORIENT_PORTRAIT (1) �܂��� DMORIENT_LANDSCAPE (2) */
		m_pShareData->m_PrintSettingArr[i].m_nPrintPaperSize = DMPAPER_A4;	/* �p���T�C�Y */
		/* �v�����^�ݒ� DEVMODE�p */
		CPrint::GetDefaultPrinterInfo( &(m_pShareData->m_PrintSettingArr[i].m_mdmDevMode) );
		m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[0] = TRUE;
		m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[1] = FALSE;
		m_pShareData->m_PrintSettingArr[i].m_bHeaderUse[2] = FALSE;
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[0], "&f" );
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[1], "" );
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szHeaderForm[2], "" );
		m_pShareData->m_PrintSettingArr[i].m_bFooterUse[0] = TRUE;
		m_pShareData->m_PrintSettingArr[i].m_bFooterUse[1] = FALSE;
		m_pShareData->m_PrintSettingArr[i].m_bFooterUse[2] = FALSE;
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[0], "&C- &P -" );
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[1], "" );
		strcpy( m_pShareData->m_PrintSettingArr[i].m_szFooterForm[2], "" );
		for( i = 1; i < MAX_PRINTSETTINGARR; ++i ){
			m_pShareData->m_PrintSettingArr[i] = m_pShareData->m_PrintSettingArr[0];
			wsprintf( m_pShareData->m_PrintSettingArr[i].m_szPrintSettingName, "����ݒ� %d", i + 1 );	/* ����ݒ�̖��O */
		}
//�L�[���[�h�F�f�t�H���g�L�[���蓖��
/********************/
/* ���ʐݒ�̋K��l */
/********************/
		struct KEYDATAINIT {
			short			nKeyCode;
			char*			pszKeyName;
			short			nFuncCode_0;
			short			nFuncCode_1;
			short			nFuncCode_2;
			short			nFuncCode_3;
			short			nFuncCode_4;
			short			nFuncCode_5;
			short			nFuncCode_6;
			short			nFuncCode_7;
		};
		static KEYDATAINIT	KeyDataInit[] = {
		//Sept. 1, 2000 Jepro note: key binding
		//Feb. 17, 2001 jepro note 2: ���Ԃ�2�i�ŉ���3�r�b�g[Alt][Ctrl][Shift]�̑g�����̏�(�����2���������l)
		//		0,		1,		 2(000), 3(001),4(010),	5(011),		6(100),	7(101),		8(110),		9(111)
		//		keycode, keyname, �Ȃ�, Shitf+, Ctrl+, Shift+Ctrl+, Alt+, Shit+Alt+, Ctrl+Alt+, Shift+Ctrl+Alt+
		//
			/* �}�E�X�{�^�� */
			{ 0, "�_�u���N���b�N",F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD, F_SELECTWORD },
		//Feb. 19, 2001 JEPRO Alt�ƉE�N���b�N�̑g�����͌����Ȃ��̂ŉE�N���b�N���j���[�̃L�[���蓖�Ă��͂�����
			{ 0, "�E�N���b�N",F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, 0, 0, 0, 0 },
			/* �t�@���N�V�����L�[ */
		//	From Here Sept. 14, 2000 JEPRO
		//	VK_F1,"F1", F_EXTHTMLHELP, 0, F_EXTHELP1, 0, 0, 0, 0, 0,
		//	Shift+F1 �Ɂu�R�}���h�ꗗ�v, Alt+F1 �Ɂu�w���v�ڎ��v, Shift+Alt+F1 �Ɂu�L�[���[�h�����v��ǉ�	//Nov. 25, 2000 JEPRO �E���Ă����̂��C���E����
		//Dec. 25, 2000 JEPRO Shift+Ctrl+F1 �Ɂu�o�[�W�������v��ǉ�
		//	{ VK_F1,"F1", F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, 0, 0, 0, 0, 0 },
			{ VK_F1,"F1", F_EXTHTMLHELP, F_MENU_ALLFUNC, F_EXTHELP1, F_ABOUT, F_HELP_CONTENTS, F_HELP_SEARCH, 0, 0 },
		//	To Here Sept. 14, 2000
			{ VK_F2,"F2", 0, 0, 0, 0, 0, 0, 0, 0 },
			//Sept. 21, 2000 JEPRO	Ctrl+F3 �Ɂu�����}�[�N�̃N���A�v��ǉ�
			{ VK_F3,"F3", F_SEARCH_NEXT, F_SEARCH_PREV, F_SEARCH_CLEARMARK, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Alt+F4 �Ɂu�E�B���h�E�����v, Shift+Alt+F4 �Ɂu���ׂẴE�B���h�E�����v��ǉ�
			//	Ctrl+F4�Ɋ��蓖�Ă��Ă����u�c���ɕ����v���u����(����)�v�ɕύX�� Shift+Ctrl+F4 �Ɂu���ĊJ���v��ǉ�
			//Jan. 14, 2001 Ctrl+Alt+F4 �Ɂu�e�L�X�g�G�f�B�^�̑S�I���v��ǉ�
			//Jun. 2001�u�T�N���G�f�B�^�̑S�I���v�ɉ���
			{ VK_F4,"F4", F_SPLIT_V, F_SPLIT_H, F_FILECLOSE, F_FILECLOSE_OPEN, F_WINCLOSE, F_WIN_CLOSEALL, F_EXITALL, 0 },
		//	From Here Sept. 20, 2000 JEPRO Ctrl+F5 �Ɂu�O���R�}���h���s�v��ǉ�  �Ȃ��}�N������CMMAND ����COMMAND �ɕύX�ς�
		//	{ VK_F5,"F5", F_PLSQL_COMPILE_ON_SQLPLUS, 0, F_EXECCOMMAND, 0, 0, 0, 0, 0 },
		//	To Here Sept. 20, 2000
			//Oct. 28, 2000 F5 �́u�ĕ`��v�ɕύX	//Jan. 14, 2001 Alt+F5 �Ɂuuudecode���ĕۑ��v, Ctrl+ Alt+F5 �ɁuTAB���󔒁v��ǉ�
			//	May 28, 2001 genta	S-C-A-F5��SPACE-to-TAB��ǉ�
			{ VK_F5,"F5", F_REDRAW, 0, F_EXECCOMMAND, 0, F_UUDECODE, 0, F_TABTOSPACE, F_SPACETOTAB },
			//Jan. 14, 2001 JEPRO	Ctrl+F6 �Ɂu�p�啶�����p�������v, Alt+F6 �ɁuBase64�f�R�[�h���ĕۑ��v��ǉ�
			{ VK_F6,"F6", F_BEGIN_SEL, F_BEGIN_BOX, F_TOLOWER, 0, F_BASE64DECODE, 0, 0, 0 },
			//Jan. 14, 2001 JEPRO	Ctrl+F7 �Ɂu�p���������p�啶���v, Alt+F7 �ɁuUTF-7��SJIS�R�[�h�ϊ��v, Shift+Alt+F7 �ɁuSJIS��UTF-7�R�[�h�ϊ��v, Ctrl+Alt+F7 �ɁuUTF-7�ŊJ�������v��ǉ�
			{ VK_F7,"F7", F_CUT, 0, F_TOUPPER, 0, F_CODECNV_UTF72SJIS, F_CODECNV_SJIS2UTF7, F_FILE_REOPEN_UTF7, 0 },
			//Nov. 9, 2000 JEPRO	Shift+F8 �ɁuCRLF���s�ŃR�s�[�v��ǉ�
			//Jan. 14, 2001 JEPRO	Ctrl+F8 �Ɂu�S�p�����p�v, Alt+F8 �ɁuUTF-8��SJIS�R�[�h�ϊ��v, Shift+Alt+F8 �ɁuSJIS��UTF-8�R�[�h�ϊ��v, Ctrl+Alt+F8 �ɁuUTF-8�ŊJ�������v��ǉ�
			{ VK_F8,"F8", F_COPY, F_COPY_CRLF, F_TOHANKAKU, 0, F_CODECNV_UTF82SJIS, F_CODECNV_SJIS2UTF8, F_FILE_REOPEN_UTF8, 0 },
			//Jan. 14, 2001 JEPRO	Ctrl+F9 �Ɂu���p�{�S�Ђ灨�S�p�E�J�^�J�i�v, Alt+F9 �ɁuUnicode��SJIS�R�[�h�ϊ��v, Ctrl+Alt+F9 �ɁuUnicode�ŊJ�������v��ǉ�
			{ VK_F9,"F9", F_PASTE, F_PASTEBOX, F_TOZENKAKUKATA, 0, F_CODECNV_UNICODE2SJIS, 0, F_FILE_REOPEN_UNICODE, 0 },
			//Oct. 28, 2000 JEPRO F10 �ɁuSQL*Plus�Ŏ��s�v��ǉ�(F5����̈ړ�)
			//Jan. 14, 2001 JEPRO	Ctrl+F10 �Ɂu���p�{�S�J�^���S�p�E�Ђ炪�ȁv, Alt+F10 �ɁuEUC��SJIS�R�[�h�ϊ��v, Shift+Alt+F10 �ɁuSJIS��EUC�R�[�h�ϊ��v, Ctrl+Alt+F10 �ɁuEUC�ŊJ�������v��ǉ�
			{ VK_F10,"F10", F_PLSQL_COMPILE_ON_SQLPLUS, F_DUPLICATELINE, F_TOZENKAKUHIRA, 0, F_CODECNV_EUC2SJIS, F_CODECNV_SJIS2EUC, F_FILE_REOPEN_EUC, 0 },
			//Jan. 14, 2001 JEPRO	Shift+F11 �ɁuSQL*Plus���A�N�e�B�u�\���v, Ctrl+F11 �Ɂu���p�J�^�J�i���S�p�J�^�J�i�v, Alt+F11 �ɁuE-Mail(JIS��SJIS)�R�[�h�ϊ��v, Shift+Alt+F11 �ɁuSJIS��JIS�R�[�h�ϊ��v, Ctrl+Alt+F11 �ɁuJIS�ŊJ�������v��ǉ�
			{ VK_F11,"F11", F_OUTLINE, F_ACTIVATE_SQLPLUS, F_HANKATATOZENKAKUKATA, 0, F_CODECNV_EMAIL, F_CODECNV_SJIS2JIS, F_FILE_REOPEN_JIS, 0 },
			//Jan. 14, 2001 JEPRO	Ctrl+F12 �Ɂu���p�J�^�J�i���S�p�Ђ炪�ȁv, Alt+F12 �Ɂu�������ʁ�SJIS�R�[�h�ϊ��v, Ctrl+Alt+F11 �ɁuSJIS�ŊJ�������v��ǉ�
			{ VK_F12,"F12", F_TAGJUMP, F_TAGJUMPBACK, F_HANKATATOZENKAKUHIRA, 0, F_CODECNV_AUTO2SJIS, 0, F_FILE_REOPEN_SJIS, 0 },
			{ VK_F13,"F13", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F14,"F14", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F15,"F15", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F16,"F16", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F17,"F17", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F18,"F18", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F19,"F19", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F20,"F20", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F21,"F21", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F22,"F22", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F23,"F23", 0, 0, 0, 0, 0, 0, 0, 0 },
			{ VK_F24,"F24", 0, 0, 0, 0, 0, 0, 0, 0 },
			/* ����L�[ */
			{ VK_TAB,"Tab",F_INDENT_TAB, F_UNINDENT_TAB, F_NEXTWINDOW, F_PREVWINDOW, 0, 0, 0, 0 },
			//Sept. 1, 2000 JEPRO	Alt+Enter �Ɂu�t�@�C���̃v���p�e�B�v��ǉ�	//Oct. 15, 2000 JEPRO Ctrl+Enter �Ɂu�t�@�C�����e��r�v��ǉ�
 			{ VK_RETURN,"Enter",0, 0, F_COMPARE, 0, F_PROPERTY_FILE, 0, 0, 0 },
			{ VK_ESCAPE,"Esc",F_CANCEL_MODE, 0, 0, 0, 0, 0, 0, 0 },
//			{ VK_BACK,"BackSpace",F_DELETE_BACK, 0, F_WordDeleteToStart, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO �����̂Ŗ��̂��ȗ��`�ɕύX(BackSpace��BkSp)
			{ VK_BACK,"BkSp",F_DELETE_BACK, 0, F_WordDeleteToStart, 0, 0, 0, 0, 0 },
//			{ VK_INSERT,"Insert",F_CHGMOD_INS, F_PASTE, F_COPY, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(Insert��Ins)
			{ VK_INSERT,"Ins",F_CHGMOD_INS, F_PASTE, F_COPY, 0, 0, 0, 0, 0 },
//			{ VK_DELETE,"Delete",F_DELETE, 0, F_WordDeleteToEnd, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(Delete��Del)
			//Jun. 26, 2001 JEPRO	Shift+Del �Ɂu�؂���v��ǉ�
			{ VK_DELETE,"Del",F_DELETE, F_CUT, F_WordDeleteToEnd, 0, 0, 0, 0, 0 },
			{ VK_HOME,"Home",F_GOLINETOP, F_GOLINETOP_SEL, F_GOFILETOP, F_GOFILETOP_SEL, 0, 0, 0, 0 },
			{ VK_END,"End(Help)",F_GOLINEEND, F_GOLINEEND_SEL, F_GOFILEEND, F_GOFILEEND_SEL, 0, 0, 0, 0 },
			{ VK_LEFT,"��",F_LEFT, F_LEFT_SEL/*F_GOLINETOP*/, F_WORDLEFT, F_WORDLEFT_SEL, F_BEGIN_BOX, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+Alt+���Ɂu�c�����ɍő剻�v��ǉ�
//			{ VK_UP,"��",F_UP, F_UP_SEL, F_UP2, F_UP2_SEL, F_BEGIN_BOX, 0, 0, F_MAXIMIZE_V },
			//Jun. 27, 2001 JEPRO
			//	Ctrl+���Ɋ��蓖�Ă��Ă����u�J�[�\����ړ�(�Q�s����)�v���u�e�L�X�g���P�s���փX�N���[���v�ɕύX
			{ VK_UP,"��",F_UP, F_UP_SEL, F_WndScrollDown, F_UP2_SEL, F_BEGIN_BOX, 0, 0, F_MAXIMIZE_V },
			//2001.02.10 by MIK Shift+Ctrl+Alt+���Ɂu�������ɍő剻�v��ǉ�
			{ VK_RIGHT,"��",F_RIGHT, F_RIGHT_SEL/*F_GOLINEEND*/, F_WORDRIGHT, F_WORDRIGHT_SEL, F_BEGIN_BOX, 0, 0, F_MAXIMIZE_H },
			//Sept. 14, 2000 JEPRO
			//	Ctrl+���Ɋ��蓖�Ă��Ă����u�E�N���b�N���j���[�v���u�J�[�\�����ړ�(�Q�s����)�v�ɕύX
			//	����ɕt�����Ă���Ɂu�E�N���b�N���j���[�v��Ctrl�{Alt�{���ɕύX
//			{ VK_DOWN,"��",F_DOWN, F_DOWN_SEL, F_DOWN2, F_DOWN2_SEL, F_BEGIN_BOX, 0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
			//Jun. 27, 2001 JEPRO
			//	Ctrl+���Ɋ��蓖�Ă��Ă����u�J�[�\�����ړ�(�Q�s����)�v���u�e�L�X�g���P�s��փX�N���[���v�ɕύX
			{ VK_DOWN,"��",F_DOWN, F_DOWN_SEL, F_WndScrollUp, F_DOWN2_SEL, F_BEGIN_BOX, 0, F_MENU_RBUTTON, F_MINIMIZE_ALL },
//			{ VK_PRIOR,"RollDown(PageUp)",F_ROLLDOWN, F_ROLLDOWN_SEL, 0, 0, 0, 0, 0, 0 },
//			{ VK_NEXT,"RollUp(PageDown)",F_ROLLUP, F_ROLLUP_SEL, 0, 0, 0, 0, 0, 0 },
			//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn �ɂ��ꂼ��u�P�y�[�W�_�E���v, �u(�I��)�P�y�[�W�_�E���v��ǉ�
			//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�Ɍ���(RollUp��PgDn) //Oct. 10, 2000 JEPRO ���̕ύX
			{ VK_NEXT,"PgDn(RollUp)",F_HalfPageDown, F_HalfPageDown_Sel, F_1PageDown, F_1PageDown_Sel, 0, 0, 0, 0 },
			//Oct. 15, 2000 JEPRO Ctrl+PgUp, Shift+Ctrl+PgDn �ɂ��ꂼ��u�P�y�[�W�A�b�v�v, �u(�I��)�P�y�[�W�A�b�v�v��ǉ�
			//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�Ɍ���(RollDown��PgUp) //Oct. 10, 2000 JEPRO ���̕ύX
			{ VK_PRIOR,"PgUp(RollDn)",F_HalfPageUp, F_HalfPageUp_Sel, F_1PageUp, F_1PageUp_Sel, 0, 0, 0, 0 },
//			{ VK_SPACE,"SpaceBar",F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO ���̂�VC++�ɍ��킹�ȗ��`�ɕύX(SpaceBar��Space)
			{ VK_SPACE,"Space",F_INDENT_SPACE, F_UNINDENT_SPACE, F_HOKAN, 0, 0, 0, 0, 0 },
			/* ���� */
			//Oct. 7, 2000 JEPRO	Ctrl+0 ���u�^�C�v�ʐݒ�ꗗ�v���u����`�v�ɕύX
			//Jan. 13, 2001 JEPRO	Alt+0 �Ɂu�J�X�^�����j���[10�v, Shift+Alt+0 �Ɂu�J�X�^�����j���[20�v��ǉ�
			{ '0', "0",0, 0, 0, 0, F_CUSTMENU_10, F_CUSTMENU_20, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+1 ���u�^�C�v�ʐݒ�v���u�c�[���o�[�̕\���v�ɕύX
			//Jan. 13, 2001 JEPRO	Alt+1 �Ɂu�J�X�^�����j���[1�v, Shift+Alt+1 �Ɂu�J�X�^�����j���[11�v��ǉ�
			//Jan. 19, 2001 JEPRO	Shift+Ctrl+1 �Ɂu�J�X�^�����j���[21�v��ǉ�
			{ '1', "1",0, 0, F_SHOWTOOLBAR, F_CUSTMENU_21, F_CUSTMENU_1, F_CUSTMENU_11, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+2 ���u���ʐݒ�v���u�t�@���N�V�����L�[�̕\���v�ɕύX
			//Jan. 13, 2001 JEPRO	Alt+2 ���u�A�E�g�v�b�g�v���u�J�X�^�����j���[2�v�ɕύX���u�A�E�g�v�b�g�v�� Alt+O �Ɉړ�, Shift+Alt+2 �Ɂu�J�X�^�����j���[12�v��ǉ�
			//Jan. 19, 2001 JEPRO	Shift+Ctrl+2 �Ɂu�J�X�^�����j���[22�v��ǉ�
			{ '2', "2",0, 0, F_SHOWFUNCKEY, F_CUSTMENU_22, F_CUSTMENU_2/*F_WIN_OUTPUT*/, F_CUSTMENU_12, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+3 ���u�t�H���g�ݒ�v���u�X�e�[�^�X�o�[�̕\���v�ɕύX
			//Jan. 13, 2001 JEPRO	Alt+3 �Ɂu�J�X�^�����j���[3�v, Shift+Alt+3 �Ɂu�J�X�^�����j���[13�v��ǉ�
			//Jan. 19, 2001 JEPRO	Shift+Ctrl+3 �Ɂu�J�X�^�����j���[23�v��ǉ�
			{ '3', "3",0, 0, F_SHOWSTATUSBAR, F_CUSTMENU_23, F_CUSTMENU_3, F_CUSTMENU_13, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+4 ���u�c�[���o�[�̕\���v���u�^�C�v�ʐݒ�ꗗ�v�ɕύX
			//Jan. 13, 2001 JEPRO	Alt+4 �Ɂu�J�X�^�����j���[4�v, Shift+Alt+4 �Ɂu�J�X�^�����j���[14�v��ǉ�
			//Jan. 19, 2001 JEPRO	Shift+Ctrl+4 �Ɂu�J�X�^�����j���[24�v��ǉ�
			{ '4', "4",0, 0, F_TYPE_LIST, F_CUSTMENU_24, F_CUSTMENU_4, F_CUSTMENU_14, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+5 ���u�t�@���N�V�����L�[�̕\���v���u�^�C�v�ʐݒ�v�ɕύX
			//Jan. 13, 2001 JEPRO	Alt+5 �Ɂu�J�X�^�����j���[5�v, Shift+Alt+5 �Ɂu�J�X�^�����j���[15�v��ǉ�
			{ '5', "5",0, 0, F_OPTION_TYPE, 0, F_CUSTMENU_5, F_CUSTMENU_15, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+6 ���u�X�e�[�^�X�o�[�̕\���v���u���ʐݒ�v�ɕύX
			//Jan. 13, 2001 JEPRO	Alt+6 �Ɂu�J�X�^�����j���[6�v, Shift+Alt+6 �Ɂu�J�X�^�����j���[16�v��ǉ�
			{ '6', "6",0, 0, F_OPTION, 0, F_CUSTMENU_6, F_CUSTMENU_16, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+7 �Ɂu�t�H���g�ݒ�v��ǉ�
			//Jan. 13, 2001 JEPRO	Alt+7 �Ɂu�J�X�^�����j���[7�v, Shift+Alt+7 �Ɂu�J�X�^�����j���[17�v��ǉ�
			{ '7', "7",0, 0, F_FONT, 0, F_CUSTMENU_7, F_CUSTMENU_17, 0, 0 },
			//Jan. 13, 2001 JEPRO	Alt+8 �Ɂu�J�X�^�����j���[8�v, Shift+Alt+8 �Ɂu�J�X�^�����j���[18�v��ǉ�
			{ '8', "8",0, 0, 0, 0, F_CUSTMENU_8, F_CUSTMENU_18, 0, 0 },
			//Jan. 13, 2001 JEPRO	Alt+9 �Ɂu�J�X�^�����j���[9�v, Shift+Alt+9 �Ɂu�J�X�^�����j���[19�v��ǉ�
			{ '9', "9",0, 0, 0, 0, F_CUSTMENU_9, F_CUSTMENU_19, 0, 0 },
			/* �A���t�@�x�b�g */
			{ 'A', "A",0, 0, F_SELECTALL, 0, 0, 0, 0, 0 },
			//Jan. 13, 2001 JEPRO	Ctrl+B �Ɂu�u���E�Y�v��ǉ�
			{ 'B', "B",0, 0, F_BROWSE, 0, 0, 0, 0, 0 },
			//Jan. 16, 2001 JEPRO	SHift+Ctrl+C �Ɂu.h�Ɠ�����.c(�Ȃ����.cpp)���J���v��ǉ�
			//Feb. 07, 2001 JEPRO	SHift+Ctrl+C ���u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v���u������C/C++�w�b�_(�\�[�X)���J���v�ɕύX
			{ 'C', "C",0, 0, F_COPY, F_OPEN_HfromtoC, 0, 0, 0, 0 },
			//Jan. 16, 2001 JEPRO	Ctrl+D �Ɂu�P��؂���v, Shift+Ctrl+D �Ɂu�P��폜�v��ǉ�
			{ 'D', "D",0, 0, F_WordCut, F_WordDelete, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+Alt+E �Ɂu�d�˂ĕ\���v��ǉ�
			//Jan. 16, 2001	JEPRO	Ctrl+E �Ɂu�s�؂���(�܂�Ԃ��P��)�v, Shift+Ctrl+E �Ɂu�s�폜(�܂�Ԃ��P��)�v��ǉ�
			{ 'E', "E",0, 0, F_CUT_LINE, F_DELETE_LINE, 0, 0, F_CASCADE, 0 },
			{ 'F', "F",0, 0, F_SEARCH_DIALOG, 0, 0, 0, 0, 0 },
			{ 'G', "G",0, 0, F_GREP, 0, 0, 0, 0, 0 },
			//Oct. 07, 2000 JEPRO	Ctrl+Alt+H �Ɂu�㉺�ɕ��ׂĕ\���v��ǉ�
			//Jan. 16, 2001 JEPRO	Ctrl+H ���u�J�[�\���O���폜�v���u�J�[�\���s���E�B���h�E�����ցv�ɕύX��	Shift+Ctrl+H �Ɂu.c�܂���.cpp�Ɠ�����.h���J���v��ǉ�
			//Feb. 07, 2001 JEPRO	SHift+Ctrl+H ���u.c�܂���.cpp�Ɠ�����.h���J���v���u������C/C++�w�b�_(�\�[�X)���J���v�ɕύX
			{ 'H', "H",0, 0, F_CURLINECENTER, F_OPEN_HfromtoC, 0, 0, F_TILE_V, 0 },
			//Jan. 21, 2001	JEPRO	Ctrl+I �Ɂu�s�̓�d���v��ǉ�
			{ 'I', "I",0, 0, F_DUPLICATELINE, 0, 0, 0, 0, 0 },
			{ 'J', "J",0, 0, F_JUMP, 0, 0, 0, 0, 0 },
			//Jan. 16, 2001	JEPRO	Ctrl+K �Ɂu�s���܂Ő؂���(���s�P��)�v, Shift+Ctrl+E �Ɂu�s���܂ō폜(���s�P��)�v��ǉ�
			{ 'K', "K",0, 0, F_LineCutToEnd, F_LineDeleteToEnd, 0, 0, 0, 0 },
			//Jan. 14, 2001 JEPRO	Ctrl+Alt+L �Ɂu�p�啶�����p�������v, Shift+Ctrl+Alt+L �Ɂu�p���������p�啶���v��ǉ�
			//Jan. 16, 2001 Ctrl+L ���u�J�[�\���s���E�B���h�E�����ցv���u�L�[�}�N���̓ǂݍ��݁v�ɕύX���u�J�[�\���s���E�B���h�E�����ցv�� Ctrl+H �Ɉړ�
			{ 'L', "L",0, 0, F_LOADKEYMACRO, F_EXECKEYMACRO, 0, 0, F_TOLOWER, F_TOUPPER },
			//Jan. 16, 2001 JEPRO	Ctrl+M �Ɂu�L�[�}�N���̕ۑ��v��ǉ�
			{ 'M', "M",0, 0, F_SAVEKEYMACRO, F_RECKEYMACRO, 0, 0, 0, 0 },
			//Oct. 20, 2000 JEPRO	Alt+N �Ɂu�ړ�����: ���ցv��ǉ�
			{ 'N', "N",0, 0, F_FILENEW, 0, F_JUMPNEXT, 0, 0, 0 },
			//Jan. 13, 2001 JEPRO	Alt+O �Ɂu�A�E�g�v�b�g�v��ǉ�
			{ 'O', "O",0, 0, F_FILEOPEN, 0, F_WIN_OUTPUT, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+P �Ɂu����v, Shift+Ctrl+P �Ɂu����v���r���[�v, Ctrl+Alt+P �Ɂu�y�[�W�ݒ�v��ǉ�
			//Oct. 20, 2000 JEPRO	Alt+P �Ɂu�ړ�����: �O�ցv��ǉ�
			{ 'P', "P",0, 0, F_PRINT, F_PRINT_PREVIEW, F_JUMPPREV, 0, F_PRINT_PAGESETUP, 0 },
			//Jan. 24, 2001	JEPRO	Ctrl+Q �Ɂu�L�[���蓖�Ĉꗗ���R�s�[�v��ǉ�
			{ 'Q', "Q",0, 0, F_CREATEKEYBINDLIST, 0, 0, 0, 0, 0 },
			{ 'R', "R",0, 0, F_REPLACE, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+S �Ɂu���O��t���ĕۑ��v��ǉ�
			{ 'S', "S",0, 0, F_FILESAVE, F_FILESAVEAS, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+Alt+T �Ɂu���E�ɕ��ׂĕ\���v��ǉ�
			//Jan. 21, 2001	JEPRO	Ctrl+T �Ɂu�^�O�W�����v�v, Shift+Ctrl+T �Ɂu�^�O�W�����v�o�b�N�v��ǉ�
			{ 'T', "T",0, 0, F_TAGJUMP, F_TAGJUMPBACK, 0, 0, F_TILE_H, 0 },
			//Oct. 7, 2000 JEPRO	Ctrl+Alt+U �Ɂu���݂̃E�B���h�E���Ő܂�Ԃ��v��ǉ�
			//Jan. 16, 2001	JEPRO	Ctrl+U �Ɂu�s���܂Ő؂���(���s�P��)�v, Shift+Ctrl+U �Ɂu�s���܂ō폜(���s�P��)�v��ǉ�
			{ 'U', "U",0, 0, F_LineCutToStart, F_LineDeleteToStart, 0, 0, F_WRAPWINDOWWIDTH, 0 },
			{ 'V', "V",0, 0, F_PASTE, 0, 0, 0, 0, 0 },
			{ 'W', "W",0, 0, F_SELECTWORD, 0, 0, 0, 0, 0 },
			//Jan. 13, 2001 JEPRO	Alt+X ���u�J�X�^�����j���[1�v���u����`�v�ɕύX���u�J�X�^�����j���[1�v�� Alt+1 �Ɉړ�
			{ 'X', "X",0, 0, F_CUT, 0, 0, 0, 0, 0 },
			{ 'Y', "Y",0, 0, F_REDO, 0, 0, 0, 0, 0 },
			{ 'Z', "Z",0, 0, F_UNDO, 0, 0, 0, 0, 0 },
			/* �L�� */
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+- �Ɂu�㉺�ɕ����v��ǉ�
			{ 0x00bd, "-",0, 0, 0, F_SPLIT_V, 0, 0, 0, 0 },
			{ 0x00de, "^",0, 0, F_COPYTAG, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+\ �Ɂu���E�ɕ����v��ǉ�
			{ 0x00dc, "\\",0, 0, F_COPYPATH, F_SPLIT_H, 0, 0, 0, 0 },
			//Sept. 20, 2000 JEPRO	Ctrl+@ �Ɂu�t�@�C�����e��r�v��ǉ�  //Oct. 15, 2000 JEPRO�u�I��͈͓��S�s�R�s�[�v�ɕύX
			{ 0x00c0, "@",0, 0, F_COPYLINES, 0, 0, 0, 0, 0 },
			//	Aug. 16, 2000 genta
			//	���Ό����̊��ʂɂ����ʌ�����ǉ�
			{ 0x00db, "[",0, 0, F_BRACKETPAIR, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	Shift+Ctrl+; �Ɂu�c���ɕ����v��ǉ�	//Jan. 16, 2001	Alt+; �Ɂu���t�}���v��ǉ�
			{ 0x00bb, ";",0, 0, 0, F_SPLIT_VH, F_INS_DATE, 0, 0, 0 },
			//Sept. 14, 2000 JEPRO	Ctrl+: �Ɂu�I��͈͓��S�s�s�ԍ��t���R�s�[�v��ǉ�	//Jan. 16, 2001	Alt+: �Ɂu�����}���v��ǉ�
			{ 0x00ba, ":",0, 0, F_COPYLINESWITHLINENUMBER, 0, F_INS_TIME, 0, 0, 0 },
			{ 0x00dd, "]",0, 0, F_BRACKETPAIR, 0, 0, 0, 0, 0 },
			{ 0x00bc, ",",0, 0, 0, 0, 0, 0, 0, 0 },
			//Sept. 14, 2000 JEPRO	Ctrl+. �Ɂu�I��͈͓��S�s���p���t���R�s�[�v��ǉ�
			{ 0x00be, ".",0, 0, F_COPYLINESASPASSAGE, 0, 0, 0, 0, 0 },
			{ 0x00bf, "/",0, 0, F_HOKAN, 0, 0, 0, 0, 0 },
			//	Nov. 15, 2000 genta PC/AT�L�[�{�[�h�ɍ��킹�ăL�[�R�[�h��ύX
			//	PC98�~�ς̂��߁C�]���̃L�[�R�[�h�ɑΉ����鍀�ڂ�ǉ��D
			{ 0x00e2, "_",0, 0, F_UNDO, 0, 0, 0, 0, 0 },
			{ 0x00df, "_(PC-98)",0, 0, F_UNDO, 0, 0, 0, 0, 0 },
			//Oct. 7, 2000 JEPRO	�����ĕ\��������Ȃ������łĂ��Ă��܂��̂ŃA�v���P�[�V�����L�[���A�v���L�[�ɒZ�k
			{ VK_APPS, "�A�v���L�[",F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON, F_MENU_RBUTTON }
		};
		int	nKeyDataInitNum = sizeof( KeyDataInit ) / sizeof( KeyDataInit[0] );
		for( i = 0; i < nKeyDataInitNum; ++i ){
			SetKeyNameArrVal( m_pShareData, i,
				KeyDataInit[i].nKeyCode,
				KeyDataInit[i].pszKeyName,
				KeyDataInit[i].nFuncCode_0,
				KeyDataInit[i].nFuncCode_1,
				KeyDataInit[i].nFuncCode_2,
				KeyDataInit[i].nFuncCode_3,
				KeyDataInit[i].nFuncCode_4,
				KeyDataInit[i].nFuncCode_5,
				KeyDataInit[i].nFuncCode_6,
				KeyDataInit[i].nFuncCode_7
			 );
		}
		m_pShareData->m_nKeyNameArrNum = nKeyDataInitNum;




//	From Here Sept. 19, 2000 JEPRO �R�����g�A�E�g�ɂȂ��Ă������߂̃u���b�N�𕜊������̉����R�����g�A�E�g
//	MS �S�V�b�N�W���X�^�C��10pt�ɐݒ�
//		/* LOGFONT�̏����� */
		memset( &m_pShareData->m_Common.m_lf, 0, sizeof( LOGFONT ) );
		m_pShareData->m_Common.m_lf.lfHeight			= -13;
		m_pShareData->m_Common.m_lf.lfWidth				= 0;
		m_pShareData->m_Common.m_lf.lfEscapement		= 0;
		m_pShareData->m_Common.m_lf.lfOrientation		= 0;
		m_pShareData->m_Common.m_lf.lfWeight			= 400;
		m_pShareData->m_Common.m_lf.lfItalic			= 0x0;
		m_pShareData->m_Common.m_lf.lfUnderline			= 0x0;
		m_pShareData->m_Common.m_lf.lfStrikeOut			= 0x0;
		m_pShareData->m_Common.m_lf.lfCharSet			= 0x80;
		m_pShareData->m_Common.m_lf.lfOutPrecision		= 0x3;
		m_pShareData->m_Common.m_lf.lfClipPrecision		= 0x2;
		m_pShareData->m_Common.m_lf.lfQuality			= 0x1;
		m_pShareData->m_Common.m_lf.lfPitchAndFamily	= 0x31;
		strcpy( m_pShareData->m_Common.m_lf.lfFaceName, "�l�r �S�V�b�N" );

//		/* LOGFONT�̏����� */
//		memset( &m_pShareData->m_Common.m_lf, 0, sizeof( LOGFONT ) );
//		m_pShareData->m_Common.m_lf.lfHeight			= -19;
//		m_pShareData->m_Common.m_lf.lfWidth				= 0;
//		m_pShareData->m_Common.m_lf.lfEscapement		= 0;
//		m_pShareData->m_Common.m_lf.lfOrientation		= 0;
//		m_pShareData->m_Common.m_lf.lfWeight			= 400;
//		m_pShareData->m_Common.m_lf.lfItalic			= 0;
//		m_pShareData->m_Common.m_lf.lfUnderline			= 0;
//		m_pShareData->m_Common.m_lf.lfStrikeOut			= 0;
//		m_pShareData->m_Common.m_lf.lfCharSet			= 128;
//		m_pShareData->m_Common.m_lf.lfOutPrecision		= 1;
//		m_pShareData->m_Common.m_lf.lfClipPrecision		= 2;
//		m_pShareData->m_Common.m_lf.lfQuality			= 1;
//		m_pShareData->m_Common.m_lf.lfPitchAndFamily	= 1;
//		strcpy( m_pShareData->m_Common.m_lf.lfFaceName, "FixedSys" );
//	To Here Sept. 19,2000

		m_pShareData->m_Common.m_bFontIs_FIXED_PITCH = TRUE;				/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */



//		m_pShareData->m_Common.m_lfPrintFont = m_pShareData->m_Common.m_lf;	/* ����p�t�H���g */
//		m_pShareData->m_Common.m_lfPrintFont.lfHeight = -400;				/* 0.01mm�P�� */

//		/* �v�����^�ݒ� DEVMODE�p */
//		CPrint::GetDefaultPrinterInfo( &(m_pShareData->m_Common.m_mdmDevMode ) );



		/* �o�b�N�A�b�v */
		m_pShareData->m_Common.m_bBackUp = FALSE;				/* �o�b�N�A�b�v�̍쐬 */
		m_pShareData->m_Common.m_bBackUpDialog = TRUE;			/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
		m_pShareData->m_Common.m_bBackUpFolder = FALSE;			/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
		m_pShareData->m_Common.m_szBackUpFolder[0] = '\0';		/* �o�b�N�A�b�v���쐬����t�H���_ */
		m_pShareData->m_Common.m_nBackUpType = 2;				/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
		m_pShareData->m_Common.m_nBackUpType_Opt1 = BKUP_YEAR | BKUP_MONTH | BKUP_DAY;
																/* �o�b�N�A�b�v�t�@�C�����F���t */
		m_pShareData->m_Common.m_nBackUpType_Opt2 = ('b' << 16 ) + 10;
																/* �o�b�N�A�b�v�t�@�C�����F�A�Ԃ̐��Ɛ擪���� */
		m_pShareData->m_Common.m_nBackUpType_Opt3 = 5;			/* �o�b�N�A�b�v�t�@�C�����FOption3 */
		m_pShareData->m_Common.m_nBackUpType_Opt4 = 0;			/* �o�b�N�A�b�v�t�@�C�����FOption4 */
		m_pShareData->m_Common.m_nBackUpType_Opt5 = 0;			/* �o�b�N�A�b�v�t�@�C�����FOption5 */
		m_pShareData->m_Common.m_nBackUpType_Opt6 = 0;			/* �o�b�N�A�b�v�t�@�C�����FOption6 */

		m_pShareData->m_Common.m_nFileShareMode = OF_SHARE_DENY_WRITE;/* �t�@�C���̔r�����䃂�[�h */

		m_pShareData->m_Common.m_nCaretType = 0;				/* �J�[�\���̃^�C�v 0=win 1=dos */
		m_pShareData->m_Common.m_bIsINSMode = TRUE;				/* �}���^�㏑�����[�h */
		m_pShareData->m_Common.m_bIsFreeCursorMode = FALSE;		/* �t���[�J�[�\�����[�h�� */	//Oct. 29, 2000 JEPRO �u�Ȃ��v�ɕύX
		m_pShareData->m_Common.m_bAutoIndent = TRUE;			/* �I�[�g�C���f���g */
		m_pShareData->m_Common.m_bAutoIndent_ZENSPACE = TRUE;	/* ���{��󔒂��C���f���g */

		//	Oct. 27, 2000 genta
		m_pShareData->m_Common.m_bRestoreCurPosition = TRUE;	//	�J�[�\���ʒu����

//		m_pShareData->m_Common.m_bEnableLineISlog = TRUE;		/* ���p�~���s�ԍ����  �����s�^�_���s */

		m_pShareData->m_Common.m_bRegularExp = 0;				/* 1==���K�\�� */
		m_pShareData->m_Common.m_bLoHiCase = 0;					/* 1==�p�啶���������̋�� */
		m_pShareData->m_Common.m_bWordOnly = 0;					/* 1==�P��̂݌��� */
		m_pShareData->m_Common.m_bSelectedArea = FALSE;			/* �I��͈͓��u�� */
		m_pShareData->m_Common.m_szExtHelp1[0] = '\0';			/* �O���w���v�P */
		m_pShareData->m_Common.m_szExtHtmlHelp[0] = '\0';		/* �O��HTML�w���v */

		m_pShareData->m_Common.m_bNOTIFYNOTFOUND = TRUE;		/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */

		m_pShareData->m_Common.m_bExitConfirm = FALSE;			/* �I�����̊m�F������ */
		m_pShareData->m_Common.m_nRepeatedScrollLineNum = 3;	/* �L�[���s�[�g���̃X�N���[���s�� */
		m_pShareData->m_Common.m_nRepeatedScroll_Smooth = FALSE;/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */

//		strcpy( m_pShareData->m_Common.m_szEMailUserName, "");		/* ���[�����[�U�[�� */
//		strcpy( m_pShareData->m_Common.m_szEMailUserAddress, "");	/* ���[���A�h���X */
//		strcpy( m_pShareData->m_Common.m_szSMTPServer, "");			/* SMTP�z�X�g���E�A�h���X */
//		m_pShareData->m_Common.m_nSMTPPort = 25;					/* SMTP�|�[�g�ԍ�(�ʏ��25) */

		m_pShareData->m_Common.m_bAddCRLFWhenCopy = FALSE;		/* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
		m_pShareData->m_Common.m_bGrepSubFolder = TRUE;			/* Grep: �T�u�t�H���_������ */
		m_pShareData->m_Common.m_bGrepOutputLine = TRUE;		/* Grep: �s���o�͂��邩�Y�����������o�͂��邩 */
		m_pShareData->m_Common.m_nGrepOutputStyle = 1;			/* Grep: �o�͌`�� */

		m_pShareData->m_Common.m_bGTJW_RETURN = TRUE;			/* �G���^�[�L�[�Ń^�O�W�����v */
		m_pShareData->m_Common.m_bGTJW_LDBLCLK = TRUE;			/* �_�u���N���b�N�Ń^�O�W�����v */

//�L�[���[�h�F�c�[���o�[����
		/* �c�[���o�[�{�^���\���� */
//Sept. 16, 2000 JEPRO
//	CShareData_new2.cpp�łł��邾���n���ƂɏW�܂�悤�ɃA�C�R���̏��Ԃ�啝�ɓ���ւ����̂ɔ����ȉ��̏����ݒ�l��ύX
		i = -1;
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 1;		//�V�K�쐬
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 2;		//�t�@�C�����J��
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 3;		//�㏑���ۑ�		//Sept. 16, 2000 JEPRO 3��11�ɕύX	//Oct. 25, 2000 11��3
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 4;		//���O��t���ĕۑ�	//Sept. 19, 2000 JEPRO �ǉ�
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 161;	//�؂���	//Sept. 16, 2000 JEPRO 4��16�ɕύX	//Oct. 25, 2000 16��161	//Feb. 24, 2001 �O����
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 162;	//�R�s�[	//Sept. 16, 2000 JEPRO 5��17�ɕύX	//Oct. 25, 2000 171��62	//Feb. 24, 2001 �O����
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 164;	//�\��t��	//Sept. 16, 2000 JEPRO 6��18�ɕύX	//Oct. 25, 2000 18��164	//Feb. 24, 2001 �O����
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;		//Feb. 24, JEPRO 2001 �O����
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 33;	//���ɖ߂�(Undo)	//Sept. 16, 2000 JEPRO 7��19�ɕύX	//Oct. 25, 2000 19��33
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 34;	//��蒼��(Redo)	//Sept. 16, 2000 JEPRO 8��20�ɕύX	//Oct. 25, 2000 20��34
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 87;	//�ړ�����: �O��	//Dec. 24, 2000 JEPRO �ǉ�
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 88;	//�ړ�����: ����	//Dec. 24, 2000 JEPRO �ǉ�
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 225;	//����		//Sept. 16, 2000 JEPRO 9��22�ɕύX	//Oct. 25, 2000 22��225
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 226;	//��������	//Sept. 16, 2000 JEPRO 16��23�ɕύX	//Oct. 25, 2000 23��226
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 227;	//�O������	//Sept. 16, 2000 JEPRO 17��24�ɕύX	//Oct. 25, 2000 24��227
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 228;	//�u��		// Oct. 7, 2000 JEPRO �ǉ�
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 229;	//�����}�[�N�̃N���A	//Sept. 16, 2000 JEPRO 41��25�ɕύX(Oct. 7, 2000 25��26)	//Oct. 25, 2000 25��229
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 230;	//Grep		//Sept. 16, 2000 JEPRO 14��31�ɕύX	//Oct. 25, 2000 31��230
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 232;	//�A�E�g���C�����	//Dec. 24, 2000 JEPRO �ǉ�
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 264;	//�^�C�v�ʐݒ�ꗗ	//Sept. 16, 2000 JEPRO �ǉ�
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 265;	//�^�C�v�ʐݒ�		//Sept. 16, 2000 JEPRO 18��36�ɕύX	//Oct. 25, 2000 36��265
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 266;	//���ʐݒ�			//Sept. 16, 2000 JEPRO 10��37�ɕύX �������u�ݒ�v���p�e�B�V�[�g�v����ύX	//Oct. 25, 2000 37��266
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 267;	//�t�H���g�ݒ�		//Sept. 16, 2000 JEPRO 11��38�ɕύX	//Oct. 25, 2000 38��267	//Dec. 24, 2000 �O����
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 268;	//���݂̃E�B���h�E���Ő܂�Ԃ�	//Sept. 16, 2000 JEPRO 42��39�ɕύX	//Oct. 25, 2000 39��268	//Feb. 24, 2001 �O����
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;

//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 12;	//������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 13;	//������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX

//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 277;	//�L�[�}�N���̋L�^�J�n�^�I��	//Sept. 16, 2000 JEPRO 55��68�ɕύX	//Oct. 25, 2000 68��277	//Dec. 24, 2000 �O����
//		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 280;	//�L�[�}�N���̎��s	//Sept. 16, 2000 JEPRO 58��71�ɕύX	//Oct. 25, 2000 71��280	//Dec. 24, 2000 �O����
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 0;		//Oct. 8, 2000 jepro ���s�̂��߂ɒǉ�
		m_pShareData->m_Common.m_nToolBarButtonIdxArr[++i] = 346;	//�R�}���h�ꗗ	//Oct. 8, 2000 JEPRO �ǉ�


		m_pShareData->m_Common.m_nToolBarButtonNum = ++i;		/* �c�[���o�[�{�^���̐� */
		m_pShareData->m_Common.m_bToolBarIsFlat = TRUE;			/* �t���b�g�c�[���o�[�ɂ���^���Ȃ� */


		m_pShareData->m_Common.m_bDispTOOLBAR = TRUE;			/* ����E�B���h�E���J�����Ƃ��c�[���o�[��\������ */
		m_pShareData->m_Common.m_bDispSTATUSBAR = TRUE;			/* ����E�B���h�E���J�����Ƃ��X�e�[�^�X�o�[��\������ */
		m_pShareData->m_Common.m_bDispFUNCKEYWND = FALSE;		/* ����E�B���h�E���J�����Ƃ��t�@���N�V�����L�[��\������ */
		m_pShareData->m_Common.m_nFUNCKEYWND_Place = 1;			/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
		m_pShareData->m_Common.m_bSplitterWndHScroll = TRUE;	// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�
		m_pShareData->m_Common.m_bSplitterWndVScroll = TRUE;	// 2001/06/20 asa-o �����E�B���h�E�̐����X�N���[���̓������Ƃ�

		/* �J�X�^�����j���[��� */
		wsprintf( m_pShareData->m_Common.m_szCustMenuNameArr[0], "�E�N���b�N���j���[", i );
		for( i = 1; i < MAX_CUSTOM_MENU; ++i ){
			wsprintf( m_pShareData->m_Common.m_szCustMenuNameArr[i], "���j���[%d", i );
			m_pShareData->m_Common.m_nCustMenuItemNumArr[i] = 0;
			for( j = 0; j < MAX_CUSTOM_MENU_ITEMS; ++j ){
				m_pShareData->m_Common.m_nCustMenuItemFuncArr[i][j] = 0;
				m_pShareData->m_Common.m_nCustMenuItemKeyArr [i][j] = '\0';
			}
		}

		/* �J�X�^�����j���[ �K��l */

		/* �E�N���b�N���j���[ */
		int n;
		n = 0;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_UNDO;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'U';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_REDO;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'R';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_CUT;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'T';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPY;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'C';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_PASTE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'P';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_DELETE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'D';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPY_CRLF;	//Nov. 9, 2000 JEPRO �uCRLF���s�ŃR�s�[�v��ǉ�
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'L';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_PASTEBOX;	//Nov. 9, 2000 JEPRO �u��`�\��t���v�𕜊�
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'X';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_SELECTALL;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'A';
		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOLOWER;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'L';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOUPPER;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'U';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOHANKAKU;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOZENKAKUHIRA;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TOZENKAKUKATA;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_OPTION_TYPE;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '1';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_OPTION;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '2';
//		n++;
//		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_FONT;
//		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '3';
//		n++;

		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;		//Oct. 3, 2000 JEPRO �ȉ��Ɂu�^�O�W�����v�v�Ɓu�^�O�W�����v�o�b�N�v��ǉ�
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMP;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'G';		//Nov. 9, 2000 JEPRO �u�R�s�[�v�ƃo�b�e�B���O���Ă����A�N�Z�X�L�[��ύX(T��G)
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_TAGJUMPBACK;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'B';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;		//Oct. 15, 2000 JEPRO �ȉ��Ɂu�I��͈͓��S�s�R�s�[�v�Ɓu���p���t���R�s�[�v��ǉ�
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPYLINES;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '@';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPYLINESASPASSAGE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '.';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\0';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_COPYPATH;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = '\\';
		n++;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[0][n] = F_PROPERTY_FILE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [0][n] = 'F';		//Nov. 9, 2000 JEPRO �u��蒼���v�ƃo�b�e�B���O���Ă����A�N�Z�X�L�[��ύX(R��F)
		n++;
		m_pShareData->m_Common.m_nCustMenuItemNumArr[0] = n;

		/* �J�X�^�����j���[�P */
		m_pShareData->m_Common.m_nCustMenuItemNumArr[1] = 7;
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][0] = F_FILEOPEN;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][0] = 'O';		//Sept. 14, 2000 JEPRO �ł��邾���W���ݒ�l�ɍ��킹��悤�ɕύX (F��O)
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][1] = F_FILESAVE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][1] = 'S';
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][2] = F_NEXTWINDOW;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][2] = 'N';		//Sept. 14, 2000 JEPRO �ł��邾���W���ݒ�l�ɍ��킹��悤�ɕύX (O��N)
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][3] = F_TOLOWER;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][3] = 'L';
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][4] = F_TOUPPER;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][4] = 'U';
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][6] = 0;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][6] = '\0';
		m_pShareData->m_Common.m_nCustMenuItemFuncArr[1][6] = F_WINCLOSE;
		m_pShareData->m_Common.m_nCustMenuItemKeyArr [1][6] = 'C';
		/* ���o���L�� */
		strcpy( m_pShareData->m_Common.m_szMidashiKigou, "�P�Q�R�S�T�U�V�W�X�O�i(�m[�u�w�y�������������������������E��������@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]���O�l�ܘZ������\���Q��" );
		/* ���p�� */
		strcpy( m_pShareData->m_Common.m_szInyouKigou, "> " );		/* ���p�� */
		m_pShareData->m_Common.m_bUseHokan = FALSE;					/* ���͕⊮�@�\���g�p���� */

		// 2001/06/14 asa-o �⊮�ƃL�[���[�h�w���v�̓^�C�v�ʂɈړ������̂ō폜
//		strcpy( m_pShareData->m_Common.m_szHokanFile, "" );			/* ���͕⊮ �P��t�@�C�� */
//		m_pShareData->m_Common.m_bUseKeyWordHelp = FALSE;			/* �L�[���[�h�w���v���g�p���� */
//		strcpy( m_pShareData->m_Common.m_szKeyWordHelpFile, "" );	/* �����t�@�C�� */
		m_pShareData->m_Common.m_bGrepKanjiCode_AutoDetect = FALSE;	/* Grep: �����R�[�h�������� */
		// 2001/06/19 asa-o �^�C�v�ʂɈړ������̂ō폜
//		m_pShareData->m_Common.m_bHokanLoHiCase = FALSE;			/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */
		m_pShareData->m_Common.m_bSaveWindowSize = TRUE;			/* �E�B���h�E�T�C�Y�p�� */
		m_pShareData->m_Common.m_nWinSizeType = SIZE_RESTORED;
		m_pShareData->m_Common.m_nWinSizeCX = CW_USEDEFAULT;
		m_pShareData->m_Common.m_nWinSizeCY = 0;
		m_pShareData->m_Common.m_bUseTaskTray = TRUE;				/* �^�X�N�g���C�̃A�C�R�����g�� */
		m_pShareData->m_Common.m_bStayTaskTray = TRUE;				/* �^�X�N�g���C�̃A�C�R�����풓 */
		m_pShareData->m_Common.m_wTrayMenuHotKeyCode = 'Z';			/* �^�X�N�g���C���N���b�N���j���[ �L�[ */
		m_pShareData->m_Common.m_wTrayMenuHotKeyMods = HOTKEYF_ALT | HOTKEYF_CONTROL;	/* �^�X�N�g���C���N���b�N���j���[ �L�[ */
		m_pShareData->m_Common.m_bUseOLE_DragDrop = TRUE;			/* OLE�ɂ��h���b�O & �h���b�v���g�� */
		m_pShareData->m_Common.m_bUseOLE_DropSource = TRUE;			/* OLE�ɂ��h���b�O���ɂ��邩 */
		m_pShareData->m_Common.m_bDispExitingDialog = FALSE;		/* �I���_�C�A���O��\������ */
		m_pShareData->m_Common.m_bEnableUnmodifiedOverwrite = FALSE;/* ���ύX�ł��㏑�����邩 */
		m_pShareData->m_Common.m_bSelectClickedURL = TRUE;			/* URL���N���b�N���ꂽ��I�����邩 */
		m_pShareData->m_Common.m_bGrepExitConfirm = FALSE;			/* Grep���[�h�ŕۑ��m�F���邩 */
//		m_pShareData->m_Common.m_bRulerDisp = TRUE;					/* ���[���[�\�� */
		m_pShareData->m_Common.m_nRulerHeight = 13;					/* ���[���[�̍��� */
		m_pShareData->m_Common.m_nRulerBottomSpace = 0;				/* ���[���[�ƃe�L�X�g�̌��� */
		m_pShareData->m_Common.m_nRulerType = 0;					/* ���[���[�̃^�C�v */
		m_pShareData->m_Common.m_bCopyAndDisablSelection = FALSE;	/* �R�s�[������I������ */
		m_pShareData->m_Common.m_bHtmlHelpIsSingle = TRUE;			/* HtmlHelp�r���[�A�͂ЂƂ� */
		m_pShareData->m_Common.m_bCompareAndTileHorz = TRUE;		/* ������r��A���E�ɕ��ׂĕ\�� */
//		m_pShareData->m_Common.m_bCompareAndTileHorz = TRUE;		/* ������r��A���E�ɕ��ׂĕ\�� */	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�
		/* 1999.11.15 */
		m_pShareData->m_Common.m_bDropFileAndClose = FALSE;			/* �t�@�C�����h���b�v�����Ƃ��͕��ĊJ�� */
		m_pShareData->m_Common.m_nDropFileNumMax = 8;				/* ��x�Ƀh���b�v�\�ȃt�@�C���� */
		m_pShareData->m_Common.m_bCheckFileTimeStamp = TRUE;		/* �X�V�̊Ď� */
		m_pShareData->m_Common.m_bNotOverWriteCRLF = TRUE;			/* ���s�͏㏑�����Ȃ� */
		::SetRect( &m_pShareData->m_Common.m_rcOpenDialog, 0, 0, 0, 0 );	/* �u�J���v�_�C�A���O�̃T�C�Y�ƈʒu */
		m_pShareData->m_Common.m_bAutoCloseDlgFind = TRUE;			/* �����_�C�A���O�������I�ɕ��� */
		m_pShareData->m_Common.m_bScrollBarHorz = TRUE;				/* �����X�N���[���o�[���g�� */
		m_pShareData->m_Common.m_bAutoCloseDlgFuncList = FALSE;		/* �A�E�g���C�� �_�C�A���O�������I�ɕ��� */	//Nov. 18, 2000 JEPRO TRUE��FALSE �ɕύX
		m_pShareData->m_Common.m_bAutoCloseDlgReplace = TRUE;		/* �u�� �_�C�A���O�������I�ɕ��� */
		m_pShareData->m_Common.m_bAutoColmnPaste = TRUE;			/* ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t�� */

		m_pShareData->m_Common.m_bHokanKey_RETURN	= TRUE;			/* VK_RETURN �⊮����L�[���L��/���� */
		m_pShareData->m_Common.m_bHokanKey_TAB		= FALSE;		/* VK_TAB   �⊮����L�[���L��/���� */
		m_pShareData->m_Common.m_bHokanKey_RIGHT	= TRUE;			/* VK_RIGHT �⊮����L�[���L��/���� */
		m_pShareData->m_Common.m_bHokanKey_SPACE	= FALSE;		/* VK_SPACE �⊮����L�[���L��/���� */


/***********
�����w��q �Ӗ�
d �N�����̓��B�擪�� 0 �͕t���܂���B
dd �N�����̓��B1 ���̏ꍇ�A�擪�� 0 ���t���܂��B
ddd �j���B���P�[���� LOCALE_SABBREVDAYNAME ���g���܂��B
dddd �j���B���P�[���� LOCALE_SDAYNAME ���g���܂��B
M �N�����̌��B�擪�� 0 �͕t���܂���B
MM �N�����̌��B1 ���̏ꍇ�A�擪�� 0 ���t���܂��B
MMM �N�����̌��B���P�[���� LOCALE_SABBREVMONTHNAME ���g���܂��B
MMMM �N�����̌��B���P�[���� LOCALE_SMONTHNAME ���g���܂��B
y �N�����̔N�B�擪�� 0 �͕t���܂���B
yy �N�����̔N�B1 ���̏ꍇ�A�擪�� 0 ���t���܂��B
yyyy �N�����̔N�B4 ���ŕ\����܂��B
gg �u�����v�Ȃǂ̎��������������B���P�[���� CAL_SERASTRING ���g���܂��B���t�����̏��������Ȃ��Ƃ��́A��������܂��B
************/

/***********
�����w��q �Ӗ�
h 12 ���Ԑ��̎��ԁB�擪�� 0 �͕t���܂���B
hh 12 ���Ԑ��̎��ԁB�K�v�ɉ����āA�擪�� 0 ���t���܂��B
H 24 ���Ԑ��̎��ԁB�擪�� 0 �͕t���܂���B
HH 24 ���Ԑ��̎��ԁB�K�v�ɉ����āA�擪�� 0 ���t���܂��B
m ���B�擪�� 0 �͕t���܂���B
mm ���B�K�v�ɉ����āA�擪�� 0 ���t���܂��B
s �b�B�擪�� 0 �͕t���܂���B
ss �b�B�K�v�ɉ����āA�擪�� 0 ���t���܂��B
t �����}�[�J�[�B�u A �v�u P �v�ȂǁB
tt �����}�[�J�[�B�u AM �v�u PM �v�u�ߑO�v�u�ߌ�v�ȂǁB
************/

		m_pShareData->m_Common.m_nDateFormatType = 0;	//���t�����̃^�C�v
		strcpy( m_pShareData->m_Common.m_szDateFormat, "yyyy\'�N\'M\'��\'d\'��(\'dddd\')\'" );	//���t����
		m_pShareData->m_Common.m_nTimeFormatType = 0;	//���������̃^�C�v
		strcpy( m_pShareData->m_Common.m_szTimeFormat, "tthh\'��\'mm\'��\'ss\'�b\'"  );			//��������

		m_pShareData->m_Common.m_bMenuIcon = TRUE;		/* ���j���[�ɃA�C�R����\������ */

		//	Nov. 12, 2000 genta
		m_pShareData->m_Common.m_bAutoMIMEdecode = TRUE;	//�t�@�C���ǂݍ��ݎ���MIME�̃f�R�[�h���s����	//Jul. 13, 2001 JEPRO

//		/* MRU �O���R�}���h */
//		m_pShareData->m_Common.m_MRU_ExtCmd.m_nExtCmdArrNum = 0;
//		for( i = 0; i < MAX_EXTCMDMRUNUM; i++ ){
//			/* ������ */
//			m_pShareData->m_Common.m_MRU_ExtCmd.m_ExtCmdArr[i].Init();
//		}
		for( i = 0; i < MAX_CMDARR; i++ ){
			/* ������ */
			m_pShareData->m_szCmdArr[i][0] = '\0';
		}
		m_pShareData->m_nCmdArrNum = 0;

//�L�[���[�h�F�f�t�H���g�J���[�ݒ�
/************************/
/* �^�C�v�ʐݒ�̋K��l */
/************************/
		int nIdx = 0;
		m_pShareData->m_Types[nIdx].m_nMaxLineSize = 10240;				/* �܂�Ԃ������� */
		m_pShareData->m_Types[nIdx].m_nColmSpace = 0;					/* �����ƕ����̌��� */
		m_pShareData->m_Types[nIdx].m_nLineSpace = 1;					/* �s�Ԃ̂����� */
		m_pShareData->m_Types[nIdx].m_nTabSpace = 4;					/* TAB�̕����� */
		m_pShareData->m_Types[nIdx].m_nKeyWordSetIdx = -1;				/* �L�[���[�h�Z�b�g */
		m_pShareData->m_Types[nIdx].m_nKeyWordSetIdx2 = -1;				/* �L�[���[�h�Z�b�g2 */	//Dec. 4, 2000, MIK
//#ifdef COMPILE_TAB_VIEW  //@@@ 2001.03.16 by MIK
		strcpy( m_pShareData->m_Types[nIdx].m_szTabViewString, "^       " );	/* TAB�\�������� */
//#endif
		strcpy( m_pShareData->m_Types[nIdx].m_szLineComment,  "" );		/* �s�R�����g�f���~�^ */
		strcpy( m_pShareData->m_Types[nIdx].m_szLineComment2, "" );		/* �s�R�����g�f���~�^2 */
		strcpy( m_pShareData->m_Types[nIdx].m_szLineComment3, "" );		/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
		strcpy( m_pShareData->m_Types[nIdx].m_szBlockCommentFrom, "" );	/* �u���b�N�R�����g�f���~�^(From) */
		strcpy( m_pShareData->m_Types[nIdx].m_szBlockCommentTo, "" );	/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		strcpy( m_pShareData->m_Types[nIdx].m_szBlockCommentFrom2, "" );/* �u���b�N�R�����g�f���~�^2(From) */
		strcpy( m_pShareData->m_Types[nIdx].m_szBlockCommentTo2, "" );	/* �u���b�N�R�����g�f���~�^2(To) */
//#endif
		m_pShareData->m_Types[nIdx].m_nLineCommentPos = -1;				/* �s�R�����g1 ���ʒu */
		m_pShareData->m_Types[nIdx].m_nLineCommentPos2 = -1;			/* �s�R�����g2 ���ʒu */
		m_pShareData->m_Types[nIdx].m_nLineCommentPos3 = -1;			/* �s�R�����g3 ���ʒu */	//Jun. 01, 2001 JEPRO �ǉ�
		m_pShareData->m_Types[nIdx].m_nStringType = 0;					/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
		strcpy( m_pShareData->m_Types[nIdx].m_szIndentChars, "" );		/* ���̑��̃C���f���g�Ώە��� */

		m_pShareData->m_Types[nIdx].m_nColorInfoArrNum = COLORIDX_LAST;

		// 2001/06/14 Start by asa-o
		strcpy( m_pShareData->m_Types[nIdx].m_szHokanFile, "" );		/* ���͕⊮ �P��t�@�C�� */
		m_pShareData->m_Types[nIdx].m_bUseKeyWordHelp = FALSE;			/* �L�[���[�h�w���v���g�p���� */
		strcpy( m_pShareData->m_Types[nIdx].m_szKeyWordHelpFile, "" );	/* �����t�@�C�� */
		// 2001/06/14 End

		// 2001/06/19 asa-o
		m_pShareData->m_Types[nIdx].m_bHokanLoHiCase = FALSE;			/* ���͕⊮�@�\�F�p�啶���������𓯈ꎋ���� */

/**
		static const char* ppszTypeName[] = {
			"�e�L�X�g",							// CI[00]
			"�s�ԍ�",							// CI[01]
			"���s�L��",							// CI[02]
			"TAB�L��",							// CI[03]
			"���{���",						// CI[04]
			"EOF�L��",							// CI[05]
			"�����L�[���[�h",					// CI[06]
			"�R�����g",							// CI[07]
			"�V���O���N�H�[�e�[�V����������",	// CI[08]
			"�_�u���N�H�[�e�[�V����������",		// CI[09]
			"�J�[�\���s�A���_�[���C��",			// CI[10]
			"�܂�Ԃ��L��",						// CI[11]
			"�R���g���[���R�[�h",				// CI[12]
			"URL",								// CI[13]
			"����������",						// CI[14]
			"�s�ԍ�(�ύX�s)",					// CI[15]
			"���[���["							// CI[16]
		};
**/

/*	From Here Sept. 18, 2000 JEPRO ���Ԃ�啝�ɓ���ւ���
		static ColorInfoIni ColorInfo_DEFAULT[] = {
			"�e�L�X�g",				TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 255, 255 ),
			"�s�ԍ�",				TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
			"���s�L��",				TRUE , FALSE, FALSE, RGB( 0, 128, 255 )		, RGB( 255, 255, 255 ),
			"TAB�L��",				TRUE , FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 255, 255 ),
			"���{���",			TRUE , FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 255, 255 ),
			"EOF�L��",				TRUE , FALSE, FALSE, RGB( 0, 255, 255 )		, RGB( 0, 0, 0 ) },
			"�����L�[���[�h",		TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 255, 255 ),
			"�R�����g",				TRUE , FALSE, FALSE, RGB( 0, 128, 0 )		, RGB( 255, 255, 255 ),
		//	From Here Sept. 4, 2000 JEPRO
		//	�V���O���N�H�[�e�[�V����������ɐF�����蓖�Ă邪�F�����\���͂��Ȃ�
		//	"�V���O���N�H�[�e�[�V����������", FALSE, FALSE, FALSE, RGB( 0, 0, 0 )		, RGB( 255, 255, 255 ),
			"�V���O���N�H�[�e�[�V����������", FALSE, FALSE, FALSE, RGB( 64, 128, 128 )	, RGB( 255, 255, 255 ),
		//	To Here Sept. 4, 2000
			"�_�u���N�H�[�e�[�V����������", TRUE, FALSE, FALSE, RGB( 128, 0, 64 ),RGB( 255, 255, 255 ),
			"�J�[�\���s�A���_�[���C��",  TRUE, FALSE, FALSE, RGB( 0, 0, 255 )	, RGB( 255, 255, 255 ),
			"�܂�Ԃ��L��",			TRUE , FALSE, FALSE, RGB( 255, 0, 255 )		, RGB( 255, 255, 255 ),
			"�R���g���[���R�[�h",	TRUE , FALSE, FALSE, RGB( 255, 255, 0 )		, RGB( 255, 255, 255 ),
			"URL",					TRUE , FALSE, TRUE , RGB( 0, 0, 255 )		, RGB( 255, 255, 255 ),
			"����������",			TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 255, 0 ),
			"�s�ԍ�(�ύX�s)",		TRUE , TRUE , FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
			"���[���[",				TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 239, 239, 239 )
		};
*/


/**
		static const char* ppszTypeName[] = {	//���C�O��Color Index
//Dec.26, 2000 jepro ��� UR1.2.24���狭���L�[���[�h��1���������ȉ��͂���ȑO�̂���
			"�e�L�X�g",							// CI[00]
			"���[���[",							// CI[16]
			"�J�[�\���s�A���_�[���C��",			// CI[10]
			"�s�ԍ�",							// CI[01]
			"�s�ԍ�(�ύX�s)",					// CI[15]
			"TAB�L��",							// CI[03]
			"���{���",						// CI[04]
			"�R���g���[���R�[�h",				// CI[12]
			"���s�L��",							// CI[02]
			"�܂�Ԃ��L��",						// CI[11]
			"EOF�L��",							// CI[05]
			"����������",						// CI[14]
			"�����L�[���[�h",					// CI[06]
			"�R�����g",							// CI[07]
			"�V���O���N�H�[�e�[�V����������",	// CI[08]
			"�_�u���N�H�[�e�[�V����������",		// CI[09]
			"URL"								// CI[13]
		};
**/

		static ColorInfoIni ColorInfo_DEFAULT[] = {
		//	Nov. 9, 2000 Jepro note: color setting (�ڍׂ� CshareData.h ���Q�Ƃ̂���)
		//	0,							1(Disp),	 2(FatFont),3(UnderLIne) , 4(colTEXT),	5(colBACK),
		//	szName(���ږ�),				�F�����^�\��, ����,		����,		�����F,		�w�i�F,
		//
		//Oct. 8, 2000 JEPRO �w�i�F��^����RGB(255,255,255)��(255,251,240)�ɕύX(ῂ�������������)
			"�e�L�X�g",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 251, 240 ),
			"���[���[",							TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 239, 239, 239 ),
			"�J�[�\���s�A���_�[���C��",			TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
			"�s�ԍ�",							TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
			"�s�ԍ�(�ύX�s)",					TRUE , TRUE , FALSE, RGB( 0, 0, 255 )		, RGB( 239, 239, 239 ),
			"TAB�L��",							TRUE , FALSE, FALSE, RGB( 128, 128, 128 )	, RGB( 255, 251, 240 ),	//Jan. 19, 2001 JEPRO RGB(192,192,192)���Z���O���[�ɕύX
			"���{���",						TRUE , FALSE, FALSE, RGB( 192, 192, 192 )	, RGB( 255, 251, 240 ),
			"�R���g���[���R�[�h",				TRUE , FALSE, FALSE, RGB( 255, 255, 0 )		, RGB( 255, 251, 240 ),
			"���s�L��",							TRUE , FALSE, FALSE, RGB( 0, 128, 255 )		, RGB( 255, 251, 240 ),
			"�܂�Ԃ��L��",						TRUE , FALSE, FALSE, RGB( 255, 0, 255 )		, RGB( 255, 251, 240 ),
			"EOF�L��",							TRUE , FALSE, FALSE, RGB( 0, 255, 255 )		, RGB( 0, 0, 0 ),
//#ifdef COMPILE_COLOR_DIGIT
			"���p���l",							FALSE, FALSE, FALSE, RGB( 235, 0, 0 )		, RGB( 255, 251, 240 ),	//@@@ 2001.02.17 by MIK		//Mar. 7, 2001 JEPRO RGB(0,0,255)��ύX  Mar.10, 2001 �W���͐F�Ȃ���
//#endif
			"����������",						TRUE , FALSE, FALSE, RGB( 0, 0, 0 )			, RGB( 255, 255, 0 ),
			"�����L�[���[�h1",					TRUE , FALSE, FALSE, RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
			"�����L�[���[�h2",					TRUE , FALSE, FALSE, RGB( 255, 128, 0 )		, RGB( 255, 251, 240 ),	//Dec. 4, 2000 MIK added	//Jan. 19, 2001 JEPRO �L�[���[�h1�Ƃ͈Ⴄ�F�ɕύX
			"�R�����g",							TRUE , FALSE, FALSE, RGB( 0, 128, 0 )		, RGB( 255, 251, 240 ),
		//Sept. 4, 2000 JEPRO �V���O���N�H�[�e�[�V����������ɐF�����蓖�Ă邪�F�����\���͂��Ȃ�
		//Oct. 17, 2000 JEPRO �F�����\������悤�ɕύX(�ŏ���FALSE��TRUE)
		//"�V���O���N�H�[�e�[�V����������", FALSE, FALSE, FALSE, RGB( 0, 0, 0 ), RGB( 255, 255, 255 ),
			"�V���O���N�H�[�e�[�V����������",	TRUE , FALSE, FALSE, RGB( 64, 128, 128 )	, RGB( 255, 251, 240 ),
			"�_�u���N�H�[�e�[�V����������",		TRUE , FALSE, FALSE, RGB( 128, 0, 64 )		, RGB( 255, 251, 240 ),
			"URL",								TRUE , FALSE, TRUE , RGB( 0, 0, 255 )		, RGB( 255, 251, 240 ),
  		};
//	To Here Sept. 18, 2000


		for( i = 0; i < COLORIDX_LAST; ++i ){
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_nColorIdx		= i;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bDisp			= ColorInfo_DEFAULT[i].m_bDisp;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bFatFont		= ColorInfo_DEFAULT[i].m_bFatFont;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_bUnderLine		= ColorInfo_DEFAULT[i].m_bUnderLine;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_colTEXT			= ColorInfo_DEFAULT[i].m_colTEXT;
			m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_colBACK			= ColorInfo_DEFAULT[i].m_colBACK;
//			strcpy( m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_szName, ppszTypeName[i] );
			strcpy( m_pShareData->m_Types[nIdx].m_ColorInfoArr[i].m_szName, ColorInfo_DEFAULT[i].m_pszName );
		}
		m_pShareData->m_Types[nIdx].m_bLineNumIsCRLF = TRUE;				/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		m_pShareData->m_Types[nIdx].m_nLineTermType = 1;					/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		m_pShareData->m_Types[nIdx].m_cLineTermChar = ':';					/* �s�ԍ���؂蕶�� */
		m_pShareData->m_Types[nIdx].m_bWordWrap = FALSE;					/* �p�����[�h���b�v������ */
		m_pShareData->m_Types[nIdx].m_nCurrentPrintSetting = 0;				/* ���ݑI�����Ă������ݒ� */
		m_pShareData->m_Types[nIdx].m_nDefaultOutline = OUTLINE_TEXT;		/* �A�E�g���C����͕��@ */
		m_pShareData->m_Types[nIdx].m_nSmartIndent = SMARTINDENT_NONE;		/* �X�}�[�g�C���f���g��� */
		m_pShareData->m_Types[nIdx].m_nImeState = IME_CMODE_NOCONVERSION;	/* IME���� */


		static char* pszTypeNameArr[] = {
			"��{",
			"�e�L�X�g",
			"C/C++",
			"HTML",
			"PL/SQL",
			"COBOL",
			"Java",
			"�A�Z���u��",
			"AWK",
			"MS-DOS�o�b�`�t�@�C��",
			"Pascal",
			"TeX",				//Oct. 31, 2000 JEPRO TeX  ���[�U�ɑ���
			"Perl",				//Jul. 08, 2001 JEPRO Perl ���[�U�ɑ���
			"Visual Basic",		//JUl. 10, 2001 JEPRO VB   ���[�U�ɑ���
			"���b�`�e�L�X�g",	//JUl. 10, 2001 JEPRO WinHelp���̂ɂ���P����
			"�ݒ�t�@�C��",		//Nov. 9, 2000 JEPRO Windows�W����ini, inf, cnf�t�@�C����sakura�L�[���[�h�ݒ�t�@�C��.kwd, �F�ݒ�t�@�C��.col ���ǂ߂�悤�ɂ���
			"�ݒ�17",			//From Here Jul. 12, 2001 JEPRO �^�C�v�ʐݒ�̐ݒ萔��16��20�ɑ��₵��
			"�ݒ�18",
			"�ݒ�19",
			"�ݒ�20"			//To Here Jul. 12, 2001
		};
		static char* pszTypeExts[] = {
			"",
			"txt,doc,1st,err,ps",	//Nov. 15, 2000 JEPRO PostScript�t�@�C�����ǂ߂�悤�ɂ��� //Jan. 12, 2001 JEPRO readme.1st ���ǂ߂�悤�ɂ��� //Feb. 12, 2001 JEPRO .err �G���[���b�Z�[�W
//			"c,cpp,cxx,h",
			"c,cpp,cxx,cc,cp,c++,h,hpp,hxx,hh,hp,h++,rc,dsw,dsp,dep,mak,hm",	//Oct. 31, 2000 JEPRO VC++�̐�������e�L�X�g�t�@�C�����ǂ߂�悤�ɂ���
				//Feb. 7, 2001 JEPRO .cc/cp/c++/.hpp/hxx/hh/hp/h++��ǉ�	//Mar. 15, 2001 JEPRO .hm��ǉ�
//			"html,htm,shtml",
			"html,htm,shtml,plg",	//Oct. 31, 2000 JEPRO VC++�̐�������e�L�X�g�t�@�C�����ǂݍ��߂�悤�ɂ���
			"sql,plsql",
			"cbl,cpy,pco,cob",	//Jun. 04, 2001 JEPRO KENCH���̏����ɏ]���ǉ�
			"java,jav",
			"asm",
			"awk",
			"bat",
			"dpr,pas",
			"tex,ltx,sty,bib,log,blg,aux,bbl,toc,lof,lot,idx,ind,glo",		//Oct. 31, 2000 JEPRO TeX ���[�U�ɑ���	//Mar. 10, 2001 JEPRO �ǉ�
			"cgi,pl,pm",			//Jul. 08, 2001 JEPRO �ǉ�
			"bas,frm,cls,ctl,pag",	//Jul. 09, 2001 JEPRO �ǉ�
			"rtf",					//Jul. 10, 2001 JEPRO �ǉ�
			"ini,inf,cnf,kwd,col",	//Nov. 9, 2000 JEPRO Windows�W����ini, inf, cnf�t�@�C����sakura�L�[���[�h�ݒ�t�@�C��.kwd, �F�ݒ�t�@�C��.col ���ǂ߂�悤�ɂ���
			"",						//From Here Jul. 12, 2001 JEPRO �^�C�v�ʐݒ�̐ݒ萔��16��20�ɑ��₵��
			"",
			"",
			""						//To Here Jul. 12, 2001
		};

		m_pShareData->m_Types[0].m_nIdx = 0;
		strcpy( m_pShareData->m_Types[0].m_szTypeName, pszTypeNameArr[0] );				/* �^�C�v�����F���� */
		strcpy( m_pShareData->m_Types[0].m_szTypeExts, pszTypeExts[0] );				/* �^�C�v�����F�g���q���X�g */
		for( nIdx = 1; nIdx < MAX_TYPES; ++nIdx ){
			m_pShareData->m_Types[nIdx] = m_pShareData->m_Types[0];
			m_pShareData->m_Types[nIdx].m_nIdx = nIdx;
			strcpy( m_pShareData->m_Types[nIdx].m_szTypeName, pszTypeNameArr[nIdx] );	/* �^�C�v�����F���� */
			strcpy( m_pShareData->m_Types[nIdx].m_szTypeExts, pszTypeExts[nIdx] );		/* �^�C�v�����F�g���q���X�g */
			m_pShareData->m_Types[nIdx].m_nKeyWordSetIdx2 = -1;	//Dec. 4, 2000 MIK
		}


		/* ��{ */
		strcpy( m_pShareData->m_Types[0].m_szLineComment, "" );			/* �s�R�����g�f���~�^ */
		strcpy( m_pShareData->m_Types[0].m_szLineComment2, "" );		/* �s�R�����g�f���~�^2 */
		strcpy( m_pShareData->m_Types[0].m_szBlockCommentFrom, "" );	/* �u���b�N�R�����g�f���~�^(From) */
		strcpy( m_pShareData->m_Types[0].m_szBlockCommentTo, "" );		/* �u���b�N�R�����g�f���~�^(To) */
		m_pShareData->m_Types[0].m_nMaxLineSize = 10240;				/* �܂�Ԃ������� */
//		m_pShareData->m_Types[0].m_nDefaultOutline = OUTLINE_UNKNOWN;	/* �A�E�g���C����͕��@ */	//Jul. 08, 2001 JEPRO �g��Ȃ��悤�ɕύX
		m_pShareData->m_Types[0].m_nDefaultOutline = OUTLINE_TEXT;		/* �A�E�g���C����͕��@ */
		//Oct. 17, 2000 JEPRO	�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[0].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//Sept. 4, 2000 JEPRO	�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[0].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;

		nIdx = 0;
		/* �e�L�X�g */
		strcpy( m_pShareData->m_Types[1].m_szLineComment, "" );			/* �s�R�����g�f���~�^ */
		strcpy( m_pShareData->m_Types[1].m_szLineComment2, "" );		/* �s�R�����g�f���~�^2 */
		strcpy( m_pShareData->m_Types[1].m_szBlockCommentFrom, "" );	/* �u���b�N�R�����g�f���~�^(From) */
		strcpy( m_pShareData->m_Types[1].m_szBlockCommentTo, "" );		/* �u���b�N�R�����g�f���~�^(To) */
		//From Here Sept. 20, 2000 JEPRO �e�L�X�g�̋K��l��80��120�ɕύX(�s��ꗗ.txt��������x�ǂ݂₷������)
//		m_pShareData->m_Types[1].m_nMaxLineSize = 80;					/* �܂�Ԃ������� */
		m_pShareData->m_Types[1].m_nMaxLineSize = 120;					/* �܂�Ԃ������� */
		//To Here Sept. 20, 2000
		m_pShareData->m_Types[1].m_nDefaultOutline = OUTLINE_TEXT;		/* �A�E�g���C����͕��@ */
		//Oct. 17, 2000 JEPRO	�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[1].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//Sept. 4, 2000 JEPRO	�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[1].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;


		// nIdx = 1;
		/* C/C++ */
		strcpy( m_pShareData->m_Types[2].m_szLineComment, "//" );			/* �s�R�����g�f���~�^ */
		strcpy( m_pShareData->m_Types[2].m_szLineComment2, "" );			/* �s�R�����g�f���~�^2 */
		strcpy( m_pShareData->m_Types[2].m_szBlockCommentFrom, "/*" );		/* �u���b�N�R�����g�f���~�^(From) */
		strcpy( m_pShareData->m_Types[2].m_szBlockCommentTo, "*/" );		/* �u���b�N�R�����g�f���~�^(To) */
		strcpy( m_pShareData->m_Types[2].m_szBlockCommentFrom2, "#if 0" );	/* �u���b�N�R�����g�f���~�^2(From) */	//Jul. 11, 2001 JEPRO
		strcpy( m_pShareData->m_Types[2].m_szBlockCommentTo2, "#endif" );	/* �u���b�N�R�����g�f���~�^2(To) */
		m_pShareData->m_Types[2].m_nKeyWordSetIdx = 0;						/* �L�[���[�h�Z�b�g */
		m_pShareData->m_Types[2].m_nDefaultOutline = OUTLINE_CPP;			/* �A�E�g���C����͕��@ */
		m_pShareData->m_Types[2].m_nSmartIndent = SMARTINDENT_CPP;			/* �X�}�[�g�C���f���g��� */
		//Mar. 10, 2001 JEPRO	���p���l��F�����\��
		m_pShareData->m_Types[2].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;


		/* HTML */
		strcpy( m_pShareData->m_Types[3].m_szLineComment, "" );				/* �s�R�����g�f���~�^ */
		strcpy( m_pShareData->m_Types[3].m_szLineComment2, "" );			/* �s�R�����g�f���~�^2 */
		strcpy( m_pShareData->m_Types[3].m_szBlockCommentFrom, "<!--" );	/* �u���b�N�R�����g�f���~�^(From) */
		strcpy( m_pShareData->m_Types[3].m_szBlockCommentTo, "-->" );		/* �u���b�N�R�����g�f���~�^(To) */
		m_pShareData->m_Types[3].m_nStringType = 0;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		m_pShareData->m_Types[3].m_nKeyWordSetIdx = 1;						/* �L�[���[�h�Z�b�g */

		// nIdx = 3;
		/* PL/SQL */
		strcpy( m_pShareData->m_Types[4].m_szLineComment, "--" );			/* �s�R�����g�f���~�^ */
		strcpy( m_pShareData->m_Types[4].m_szLineComment2, "" );			/* �s�R�����g�f���~�^2 */
		strcpy( m_pShareData->m_Types[4].m_szBlockCommentFrom, "/*" );		/* �u���b�N�R�����g�f���~�^(From) */
		strcpy( m_pShareData->m_Types[4].m_szBlockCommentTo, "*/" );		/* �u���b�N�R�����g�f���~�^(To) */
		m_pShareData->m_Types[4].m_nStringType = 1;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		strcpy( m_pShareData->m_Types[4].m_szIndentChars, "|��" );			/* ���̑��̃C���f���g�Ώە��� */
		m_pShareData->m_Types[4].m_nKeyWordSetIdx = 2;						/* �L�[���[�h�Z�b�g */
		m_pShareData->m_Types[4].m_nDefaultOutline = OUTLINE_PLSQL;			/* �A�E�g���C����͕��@ */

		/* COBOL */
//		strcpy( m_pShareData->m_Types[5].m_szLineComment, "      *" );		/* �s�R�����g�f���~�^ */
		strcpy( m_pShareData->m_Types[5].m_szLineComment, "*" );			/* �s�R�����g�f���~�^ */	//Jun. 02, 2001 JEPRO �C��
		strcpy( m_pShareData->m_Types[5].m_szLineComment2, "D" );			/* �s�R�����g�f���~�^2 */	//Jun. 04, 2001 JEPRO �ǉ�
		strcpy( m_pShareData->m_Types[5].m_szBlockCommentFrom, "" );		/* �u���b�N�R�����g�f���~�^(From) */
		strcpy( m_pShareData->m_Types[5].m_szBlockCommentTo, "" );			/* �u���b�N�R�����g�f���~�^(To) */
		m_pShareData->m_Types[5].m_nLineCommentPos = 6;						/* �s�R�����g1 ���ʒu */	//Jun. 02, 2001 JEPRO �ǉ�
		m_pShareData->m_Types[5].m_nLineCommentPos2 = 6;					/* �s�R�����g2 ���ʒu */	//Jun. 04, 2001 JEPRO �ǉ�
		m_pShareData->m_Types[5].m_nStringType = 1;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		strcpy( m_pShareData->m_Types[5].m_szIndentChars, "*" );			/* ���̑��̃C���f���g�Ώە��� */
		m_pShareData->m_Types[5].m_nKeyWordSetIdx = 3;						/* �L�[���[�h�Z�b�g */		//Jul. 10, 2001 JEPRO
		m_pShareData->m_Types[5].m_nDefaultOutline = OUTLINE_COBOL;			/* �A�E�g���C����͕��@ */


		/* Java */
		strcpy( m_pShareData->m_Types[6].m_szLineComment, "//" );			/* �s�R�����g�f���~�^ */
		strcpy( m_pShareData->m_Types[6].m_szLineComment2, "" );			/* �s�R�����g�f���~�^2 */
		strcpy( m_pShareData->m_Types[6].m_szBlockCommentFrom, "/*" );		/* �u���b�N�R�����g�f���~�^(From) */
		strcpy( m_pShareData->m_Types[6].m_szBlockCommentTo, "*/" );		/* �u���b�N�R�����g�f���~�^(To) */
		m_pShareData->m_Types[6].m_nKeyWordSetIdx = 4;						/* �L�[���[�h�Z�b�g */
		m_pShareData->m_Types[6].m_nDefaultOutline = OUTLINE_JAVA;			/* �A�E�g���C����͕��@ */
		m_pShareData->m_Types[6].m_nSmartIndent = SMARTINDENT_CPP;			/* �X�}�[�g�C���f���g��� */
		//Mar. 10, 2001 JEPRO	���p���l��F�����\��
		m_pShareData->m_Types[6].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;

		/* �A�Z���u�� */
		m_pShareData->m_Types[7].m_nDefaultOutline = OUTLINE_ASM;			/* �A�E�g���C����͕��@ */
		//Mar. 10, 2001 JEPRO	���p���l��F�����\��
		m_pShareData->m_Types[7].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;

		/* awk */
		strcpy( m_pShareData->m_Types[8].m_szLineComment, "#" );			/* �s�R�����g�f���~�^ */
		m_pShareData->m_Types[8].m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
		m_pShareData->m_Types[8].m_nKeyWordSetIdx = 6;						/* �L�[���[�h�Z�b�g */

		/* MS-DOS�o�b�`�t�@�C�� */
		strcpy( m_pShareData->m_Types[9].m_szLineComment, "REM " );			/* �s�R�����g�f���~�^ */
//		strcpy( m_pShareData->m_Types[9].m_szLineComment2, ":" );			/* �s�R�����g�f���~�^2 */	//Oct. 31, 2000 JEPRO ':' ��ǉ�(�p�����ꂽ)
		m_pShareData->m_Types[9].m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
		m_pShareData->m_Types[9].m_nKeyWordSetIdx = 7;						/* �L�[���[�h�Z�b�g */

		/* Pascal */
		strcpy( m_pShareData->m_Types[10].m_szLineComment, "//" );			/* �s�R�����g�f���~�^ */				//Nov. 5, 2000 JEPRO �ǉ�
		strcpy( m_pShareData->m_Types[10].m_szBlockCommentFrom, "{" );		/* �u���b�N�R�����g�f���~�^(From) */	//Nov. 5, 2000 JEPRO �ǉ�
		strcpy( m_pShareData->m_Types[10].m_szBlockCommentTo, "}" );		/* �u���b�N�R�����g�f���~�^(To) */		//Nov. 5, 2000 JEPRO �ǉ�
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		strcpy( m_pShareData->m_Types[10].m_szBlockCommentFrom2, "(*" );	/* �u���b�N�R�����g�f���~�^2(From) */
		strcpy( m_pShareData->m_Types[10].m_szBlockCommentTo2, "*)" );		/* �u���b�N�R�����g�f���~�^2(To) */
//#endif
		m_pShareData->m_Types[10].m_nStringType = 1;						/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */	//Nov. 5, 2000 JEPRO �ǉ�
		m_pShareData->m_Types[10].m_nKeyWordSetIdx = 8;						/* �L�[���[�h�Z�b�g */
		//Mar. 10, 2001 JEPRO	���p���l��F�����\��
		m_pShareData->m_Types[2].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;

		//From Here Oct. 31, 2000 JEPRO
		/* TeX */
		strcpy( m_pShareData->m_Types[11].m_szLineComment, "%" );			/* �s�R�����g�f���~�^ */
		m_pShareData->m_Types[11].m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
		m_pShareData->m_Types[11].m_nKeyWordSetIdx  = 9;					/* �L�[���[�h�Z�b�g */
		m_pShareData->m_Types[11].m_nKeyWordSetIdx2 = 10;					/* �L�[���[�h�Z�b�g2 */	//Jan. 19, 2001 JEPRO
		//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
		//URL�ɃA���_�[���C���������Ȃ�(����ς��߂�)
//		m_pShareData->m_Types[11].m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;
		//To Here Oct. 31, 2000

		//From Here Jul. 08, 2001 JEPRO
		/* Perl */
		strcpy( m_pShareData->m_Types[12].m_szLineComment, "#" );			/* �s�R�����g�f���~�^ */
		m_pShareData->m_Types[12].m_nDefaultOutline = OUTLINE_PERL;			/* �A�E�g���C����͕��@ */
		m_pShareData->m_Types[12].m_nKeyWordSetIdx  = 11;					/* �L�[���[�h�Z�b�g */
		m_pShareData->m_Types[12].m_nKeyWordSetIdx2 = 12;					/* �L�[���[�h�Z�b�g2 */
		m_pShareData->m_Types[12].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* ���p���l��F�����\�� */
		//To Here Jul. 08, 2001

		//From Here Jul. 10, 2001 JEPRO
		/* Visual Basic */
		strcpy( m_pShareData->m_Types[13].m_szLineComment, "'" );			/* �s�R�����g�f���~�^ */
		m_pShareData->m_Types[13].m_nDefaultOutline = OUTLINE_VB;			/* �A�E�g���C����͕��@ */
		m_pShareData->m_Types[13].m_nKeyWordSetIdx  = 13;					/* �L�[���[�h�Z�b�g */
		m_pShareData->m_Types[13].m_nKeyWordSetIdx2 = 14;					/* �L�[���[�h�Z�b�g2 */
		m_pShareData->m_Types[13].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* ���p���l��F�����\�� */
		m_pShareData->m_Types[13].m_nStringType = 1;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[13].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;

		/* ���b�`�e�L�X�g */
		m_pShareData->m_Types[14].m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
		m_pShareData->m_Types[14].m_nKeyWordSetIdx  = 15;					/* �L�[���[�h�Z�b�g */
		m_pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = TRUE;	/* ���p���l��F�����\�� */
		m_pShareData->m_Types[14].m_nStringType = 0;							/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
		//URL�ɃA���_�[���C���������Ȃ�
		m_pShareData->m_Types[14].m_ColorInfoArr[COLORIDX_URL].m_bDisp = FALSE;
		//To Here Jul. 10, 2001

		//From Here Nov. 9, 2000 JEPRO
		/* �ݒ�t�@�C�� */
		strcpy( m_pShareData->m_Types[15].m_szLineComment, "//" );			/* �s�R�����g�f���~�^ */
		strcpy( m_pShareData->m_Types[15].m_szLineComment2, ";" );			/* �s�R�����g�f���~�^2 */
		m_pShareData->m_Types[15].m_nDefaultOutline = OUTLINE_TEXT;			/* �A�E�g���C����͕��@ */
		//�V���O���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[15].m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
		//�_�u���N�H�[�e�[�V�����������F�����\�����Ȃ�
		m_pShareData->m_Types[15].m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
		//To Here Nov. 9, 2000


		/* �����L�[���[�h�̃e�X�g�f�[�^ */
		m_pShareData->m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = 0;

		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "C/C++", TRUE );			/* �Z�b�g 0�̒ǉ� */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "HTML", FALSE );			/* �Z�b�g 1�̒ǉ� */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "PL/SQL", FALSE );		/* �Z�b�g 2�̒ǉ� */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "COBOL", TRUE );			/* �Z�b�g 3�̒ǉ� */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Java", TRUE );			/* �Z�b�g 4�̒ǉ� */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "CORBA IDL", TRUE );		/* �Z�b�g 5�̒ǉ� */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "AWK", TRUE );			/* �Z�b�g 6�̒ǉ� */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "MS-DOS batch", FALSE );	/* �Z�b�g 7�̒ǉ� */	//Oct. 31, 2000 JEPRO '�o�b�`�t�@�C��'��'batch' �ɒZ�k
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Pascal", FALSE );		/* �Z�b�g 8�̒ǉ� */	//Nov. 5, 2000 JEPRO ��E�������̋�ʂ�'���Ȃ�'�ɕύX
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "TeX", TRUE );			/* �Z�b�g 9�̒ǉ� */	//Sept. 2, 2000 jepro Tex ��TeX �ɏC�� Bool�l�͑�E�������̋��
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "TeX2", TRUE );			/* �Z�b�g10�̒ǉ� */	//Jan. 19, 2001 JEPRO �ǉ�
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Perl", TRUE );			/* �Z�b�g11�̒ǉ� */
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Perl2", TRUE );			/* �Z�b�g12�̒ǉ� */	//Jul. 10, 2001 JEPRO Perl����ϐ��𕪗��E�Ɨ�
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Visual Basic", FALSE );	/* �Z�b�g13�̒ǉ� */	//Jul. 10, 2001 JEPRO
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "Visual Basic2", FALSE );	/* �Z�b�g14�̒ǉ� */	//Jul. 10, 2001 JEPRO
		m_pShareData->m_CKeyWordSetMgr.AddKeyWordSet( "���b�`�e�L�X�g", TRUE );	/* �Z�b�g15�̒ǉ� */	//Jul. 10, 2001 JEPRO



		/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ� */
		static const char*	ppszKeyWordsCPP[] = {
			"#define",
			"#elif",
			"#else",
			"#endif",
			"#error",
			"#if",
			"#ifdef",
			"#ifndef",
			"#include",
			"#line",
			"#pragma",
			"#undef",
			"__declspec",
			"__FILE__",
			"asm",
			"auto",
			"break",
			"bool",
			"case",
			"catch",
			"char",
			"class",
			"const",
			"continue",
			"default",
			"define",
			"delete",
			"do",
			"double",
			"elif",
			"else",
			"endif",
			"enum",
			"error",
			"explicit",
			"extern",
			"false",
			"float",
			"for",
			"friend",
			"goto",
			"if",
			"ifdef",
			"ifndef",
			"include",
			"inline",
			"int",
			"line",
			"long",
			"mutable",
			"namespace",
			"new",
			"operator",
			"pragma",
			"private",
			"protected",
			"public",
			"register",
			"return",
			"short",
			"signed",
			"sizeof",
			"static",
			"struct",
			"switch",
			"template",
			"this",
			"throw",
			"try",
			"true",
			"typedef",
			"typename",
			"undef",
			"union",
			"unsigned",
			"using",
			"virtual",
			"void",
			"volatile",
			"while"
		};
		static int nKeyWordsCPP_Num = sizeof( ppszKeyWordsCPP ) / sizeof( ppszKeyWordsCPP[0] );
		for( i = 0; i < nKeyWordsCPP_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 0, ppszKeyWordsCPP[i] );
		}


		static const char*	ppszKeyWordsHTML[] = {
			"_blank",
			"_parent",
			"_self",
			"_top",
			"A",
			"ABBR",
			"ABOVE",
			"absbottom",
			"absmiddle",
			"ACCESSKEY",
			"ACRONYM",
			"ACTION",
			"ADDRESS",
			"ALIGN",
			"all",
			"APPLET",
			"AREA",
			"AUTOPLAY",
			"AUTOSTART",
			"B",
			"BACKGROUND",
			"BASE",
			"BASEFONT",
			"baseline",
			"BEHAVIOR",
			"BELOW",
			"BGCOLOR",
			"BGSOUND",
			"BIG",
			"BLINK",
			"BLOCKQUOTE",
			"BODY",
			"BORDER",
			"BORDERCOLOR",
			"BORDERCOLORDARK",
			"BORDERCOLORLIGHT",
			"BOTTOM",
			"box",
			"BR",
			"BUTTON",
			"CAPTION",
			"CELLPADDING",
			"CELLSPACING",
			"CENTER",
			"CHALLENGE",
			"char",
			"checkbox",
			"CHECKED",
			"CITE",
			"CLEAR",
			"CLIP",
			"CODE",
			"CODEBASE",
			"CODETYPE",
			"COL",
			"COLGROUP",
			"COLOR",
			"COLS",
			"COLSPAN",
			"COMMENT",
			"CONTROLS",
			"DATA",
			"DD",
			"DECLARE",
			"DEFER",
			"DEL",
			"DELAY",
			"DFN",
			"DIR",
			"DIRECTION",
			"DISABLED",
			"DIV",
			"DL",
			"DOCTYPE",
			"DT",
			"EM",
			"EMBED",
			"ENCTYPE",
			"FACE",
			"FIELDSET",
			"file",
			"FONT",
			"FOR",
			"FORM",
			"FRAME",
			"FRAMEBORDER",
			"FRAMESET",
			"GET",
			"groups",
			"GROUPS",
			"GUTTER",
			"H1",
			"H2",
			"H3",
			"H4",
			"H5",
			"H6",
			"H7",
			"HEAD",
			"HEIGHT",
			"HIDDEN",
			"Hn",
			"HR",
			"HREF",
			"hsides",
			"HSPACE",
			"HTML",
			"I",
			"ID",
			"IFRAME",
			"ILAYER",
			"image",
			"IMG",
			"INDEX",
			"inherit",
			"INPUT",
			"INS",
			"ISINDEX",
			"JavaScript",
			"justify",
			"KBD",
			"KEYGEN",
			"LABEL",
			"LANGUAGE",
			"LAYER",
			"LEFT",
			"LEGEND",
			"lhs",
			"LI",
			"LINK",
			"LISTING",
			"LOOP",
			"MAP",
			"MARQUEE",
			"MAXLENGTH",
			"MENU",
			"META",
			"METHOD",
			"METHODS",
			"MIDDLE",
			"MULTICOL",
			"MULTIPLE",
			"NAME",
			"NEXT",
			"NEXTID",
			"NOBR",
			"NOEMBED",
			"NOFRAMES",
			"NOLAYER",
			"none",
			"NOSAVE",
			"NOSCRIPT",
			"NOTAB",
			"NOWRAP",
			"OBJECT",
			"OL",
			"onBlur",
			"onChange",
			"onClick",
			"onFocus",
			"onLoad",
			"onMouseOut",
			"onMouseOver",
			"onReset",
			"onSelect",
			"onSubmit",
			"OPTION",
			"P",
			"PAGEX",
			"PAGEY",
			"PALETTE",
			"PANEL",
			"PARAM",
			"PARENT",
			"password",
			"PLAINTEXT",
			"PLUGINSPAGE",
			"POST",
			"PRE",
			"PREVIOUS",
			"Q",
			"radio",
			"REL",
			"REPEAT",
			"reset",
			"REV",
			"rhs",
			"RIGHT",
			"rows",
			"ROWSPAN",
			"RULES",
			"S",
			"SAMP",
			"SAVE",
			"SCRIPT",
			"SCROLLAMOUNT",
			"SCROLLDELAY",
			"SELECT",
			"SELECTED",
			"SERVER",
			"SHAPES",
			"show",
			"SIZE",
			"SMALL",
			"SONG",
			"SPACER",
			"SPAN",
			"SRC",
			"STANDBY",
			"STRIKE",
			"STRONG",
			"STYLE",
			"SUB",
			"submit",
			"SUMMARY",
			"SUP",
			"TABINDEX",
			"TABLE",
			"TARGET",
			"TBODY",
			"TD",
			"TEXT",
			"TEXTAREA",
			"textbottom",
			"TEXTFOCUS",
			"textmiddle",
			"texttop",
			"TFOOT",
			"TH",
			"THEAD",
			"TITLE",
			"TOP",
			"TR",
			"TT",
			"TXTCOLOR",
			"TYPE",
			"U",
			"UL",
			"URN",
			"USEMAP",
			"VALIGN",
			"VALUE",
			"VALUETYPE",
			"VAR",
			"VISIBILITY",
			"void",
			"vsides",
			"VSPACE",
			"WBR",
			"WIDTH",
			"WRAP",
			"XMP"
		};
		static int nKeyWordsHTML_Num = sizeof( ppszKeyWordsHTML ) / sizeof( ppszKeyWordsHTML[0] );
		for( i = 0; i < nKeyWordsHTML_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 1, ppszKeyWordsHTML[i] );
		}


		static const char*	ppszKeyWordsPLSQL[] = {
			"AND",
			"AS",
			"BEGIN",
			"BINARY_INTEGER",
			"BODY",
			"BOOLEAN",
			"BY",
			"CHAR",
			"CHR",
			"COMMIT",
			"COUNT",
			"CREATE",
			"CURSOR",
			"DATE",
			"DECLARE",
			"DEFAULT",
			"DELETE",
			"ELSE",
			"ELSIF",
			"END",
			"ERRORS",
			"EXCEPTION",
			"FALSE",
			"FOR",
			"FROM",
			"FUNCTION",
			"GOTO",
			"HTP",
			"IDENT_ARR",
			"IF",
			"IN",
			"INDEX",
			"INTEGER",
			"IS",
			"LOOP",
			"NOT",
			"NO_DATA_FOUND",
			"NULL",
			"NUMBER",
			"OF",
			"OR",
			"ORDER",
			"OUT",
			"OWA_UTIL",
			"PACKAGE",
			"PRAGMA",
			"PRN",
			"PROCEDURE",
			"REPLACE",
			"RESTRICT_REFERENCES",
			"RETURN",
			"ROWTYPE",
			"SELECT",
			"SHOW",
			"SUBSTR",
			"TABLE",
			"THEN",
			"TRUE",
			"TYPE",
			"UPDATE",
			"VARCHAR",
			"VARCHAR2",
			"WHEN",
			"WHERE",
			"WHILE",
			"WNDS",
			"WNPS",
			"RAISE",
			"INSERT",
			"INTO",
			"VALUES",
			"SET",
			"SYSDATE",
			"RTRIM",
			"LTRIM",
			"TO_CHAR",
			"DUP_VAL_ON_INDEX",
			"ROLLBACK",
			"OTHERS",
			"SQLCODE"
		};
		static int nKeyWordsPLSQL_Num = sizeof( ppszKeyWordsPLSQL ) / sizeof( ppszKeyWordsPLSQL[0] );
		for( i = 0; i < nKeyWordsPLSQL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 2, ppszKeyWordsPLSQL[i] );
		}


//Jul. 10, 2001 JEPRO �ǉ�
		static const char*	ppszKeyWordsCOBOL[] = {
			"ACCEPT",
			"ADD",
			"ADVANCING",
			"AFTER",
			"ALL",
			"AND",
			"ARGUMENT",
			"ASSIGN",
			"AUTHOR",
			"BEFORE",
			"BLOCK",
			"BY",
			"CALL",
			"CHARACTERS",
			"CLOSE",
			"COMP",
			"COMPILED",
			"COMPUTE",
			"COMPUTER",
			"CONFIGURATION",
			"CONSOLE",
			"CONTAINS",
			"CONTINUE",
			"CONTROL",
			"COPY",
			"DATA",
			"DELETE",
			"DISPLAY",
			"DIVIDE",
			"DIVISION",
			"ELSE",
			"END",
			"ENVIRONMENT",
			"EVALUATE",
			"EXAMINE",
			"EXIT",
			"EXTERNAL",
			"FD",
			"FILE",
			"FILLER",
			"FROM",
			"GIVING",
			"GO",
			"GOBACK",
			"HIGH-VALUE",
			"IDENTIFICATION"
			"IF",
			"INITIALIZE",
			"INPUT",
			"INTO",
			"IS",
			"LABEL",
			"LINKAGE",
			"LOW-VALUE",
			"MODE",
			"MOVE",
			"NOT",
			"OBJECT",
			"OCCURS",
			"OF",
			"ON",
			"OPEN",
			"OR",
			"OTHER",
			"OUTPUT",
			"PERFORM",
			"PIC",
			"PROCEDURE",
			"PROGRAM",
			"READ",
			"RECORD",
			"RECORDING",
			"REDEFINES",
			"REMAINDER",
			"REMARKS",
			"REPLACING",
			"REWRITE",
			"ROLLBACK",
			"SECTION",
			"SELECT",
			"SOURCE",
			"SPACE",
			"STANDARD",
			"STOP",
			"STORAGE",
			"SYSOUT",
			"TEST",
			"THEN",
			"TO",
			"TODAY",
			"TRANSFORM",
			"UNTIL",
			"UPON",
			"USING",
			"VALUE",
			"VARYING",
			"WHEN",
			"WITH",
			"WORKING",
			"WRITE",
			"WRITTEN",
			"ZERO"
		};
		static int nKeyWordsCOBOL_Num = sizeof( ppszKeyWordsCOBOL ) / sizeof( ppszKeyWordsCOBOL[0] );
		for( i = 0; i < nKeyWordsCOBOL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 3, ppszKeyWordsCOBOL[i] );
		}


		static const char*	ppszKeyWordsJAVA[] = {
			"abstract",
			"boolean",
			"break",
			"byte",
			"case",
			"catch",
			"char",
			"class",
			"const",
			"continue",
			"default",
			"do",
			"double",
			"else",
			"extends",
			"final",
			"finally",
			"float",
			"for",
			"goto",
			"if",
			"implements",
			"import",
			"instanceof",
			"int",
			"interface",
			"long",
			"native",
			"new",
			"package",
			"private",
			"protected",
			"public",
			"return",
			"short",
			"static",
			"super",
			"switch",
			"synchronized",
			"this",
			"throw",
			"throws",
			"transient",
			"try",
			"void",
			"volatile",
			"while"
		};
		static int nKeyWordsJAVA_Num = sizeof( ppszKeyWordsJAVA ) / sizeof( ppszKeyWordsJAVA[0] );
		for( i = 0; i < nKeyWordsJAVA_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 4, ppszKeyWordsJAVA[i] );
		}


		static const char*	ppszKeyWordsCORBA_IDL[] = {
			"any",
			"attribute",
			"boolean",
			"case",
			"char",
			"const",
			"context",
			"default",
			"double",
			"enum",
			"exception",
			"FALSE",
			"fixed",
			"float",
			"in",
			"inout",
			"interface",
			"long",
			"module",
			"Object",
			"octet",
			"oneway",
			"out",
			"raises",
			"readonly",
			"sequence",
			"short",
			"string",
			"struct",
			"switch",
			"TRUE",
			"typedef",
			"unsigned",
			"union",
			"void",
			"wchar",
			"wstring"
		};
		static int nKeyWordsCORBA_IDL_Num = sizeof( ppszKeyWordsCORBA_IDL ) / sizeof( ppszKeyWordsCORBA_IDL[0] );
		for( i = 0; i < nKeyWordsCORBA_IDL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 5, ppszKeyWordsCORBA_IDL[i] );
		}


		static const char*	ppszKeyWordsAWK[] = {
			"BEGIN",
			"END",
			"next",
			"exit",
			"func",
			"function",
			"return",
			"if",
			"else",
			"for",
			"in",
			"do",
			"while",
			"break",
			"continue",
			"$0",
			"$1",
			"$2",
			"$3",
			"$4",
			"$5",
			"$6",
			"$7",
			"$8",
			"$9",
			"$10",
			"$11",
			"$12",
			"$13",
			"$14",
			"$15",
			"$16",
			"$17",
			"$18",
			"$19",
			"$20",
			"FS",
			"OFS",
			"NF",
			"RS",
			"ORS",
			"NR",
			"FNR",
			"ARGV",
			"ARGC",
			"ARGIND",
			"FILENAME",
			"ENVIRON",
			"ERRNO",
			"OFMT",
			"CONVFMT",
			"FIELDWIDTHS",
			"IGNORECASE",
			"RLENGTH",
			"RSTART",
			"SUBSEP",
			"delete",
			"index",
			"jindex",
			"length",
			"jlength",
			"substr",
			"jsubstr",
			"match",
			"split",
			"sub",
			"gsub",
			"sprintf",
			"tolower",
			"toupper",
			"print",
			"printf",
			"getline",
			"system",
			"close",
			"sin",
			"cos",
			"atan2",
			"exp",
			"log",
			"int",
			"sqrt",
			"srand",
			"rand",
			"strftime",
			"systime"
		};
		static int nKeyWordsAWK_Num = sizeof( ppszKeyWordsAWK ) / sizeof( ppszKeyWordsAWK[0] );
		for( i = 0; i < nKeyWordsAWK_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 6, ppszKeyWordsAWK[i] );
		}


		static const char*	ppszKeyWordsBAT[] = {
			"PATH",
			"PROMPT",
			"TEMP",
			"TMP",
			"TZ",
			"CONFIG",
			"COMSPEC",
			"DIRCMD",
			"COPYCMD",
			"winbootdir",
			"windir",
			"DIR",
			"CALL",
			"CHCP",
			"RENAME",
			"REN",
			"ERASE",
			"DEL",
			"TYPE",
			"REM",
			"COPY",
			"PAUSE",
			"DATE",
			"TIME",
			"VER",
			"VOL",
			"CD",
			"CHDIR",
			"MD",
			"MKDIR",
			"RD",
			"RMDIR",
			"BREAK",
			"VERIFY",
			"SET",
//			"PROMPT",	//Oct. 31, 2000 JEPRO ����2�͊��ɓo�^����Ă���̂ŏȗ�
//			"PATH",
			"EXIT",
			"CTTY",
			"ECHO",
			"@ECHO",	//Oct. 31, 2000 JEPRO '@' �������\�ɂ����̂Œǉ�
			"LOCK",
			"UNLOCK",
			"GOTO",
			"SHIFT",
			"IF",
			"FOR",
			"DO",	//Nov. 2, 2000 JEPRO �ǉ�
			"IN",	//Nov. 2, 2000 JEPRO �ǉ�
			"ELSE",	//Nov. 2, 2000 JEPRO �ǉ� Win2000�Ŏg����
			"CLS",
			"TRUENAME",
			"LOADHIGH",
			"LH",
			"LFNFOR",
			"ON",
			"OFF",
			"NOT",
			"ERRORLEVEL",
			"EXIST",
			"NUL",
			"CON",
			"AUX",
			"COM1",
			"COM2",
			"COM3",
			"COM4",
			"PRN",
			"LPT1",
			"LPT2",
			"LPT3",
			"CLOCK",
			"CLOCK$",
			"CONFIG$"
		};
		static int nKeyWordsBAT_Num = sizeof( ppszKeyWordsBAT ) / sizeof( ppszKeyWordsBAT[0] );
		for( i = 0; i < nKeyWordsBAT_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 7, ppszKeyWordsBAT[i] );
		}


		static const char*	ppszKeyWordsPASCAL[] = {
			"and",
			"exports",
			"mod",
			"shr",
			"array",
			"file",
			"nil",
			"string",
			"as",
			"finalization",
			"not",
			"stringresource",
			"asm",
			"finally",
			"object",
			"then",
			"begin",
			"for",
			"of",
			"case",
			"function",
			"or",
			"to",
			"class",
			"goto",
			"out",
			"try",
			"const",
			"if",
			"packed",
			"type",
			"constructor",
			"implementation",
			"procedure",
			"unit",
			"destructor",
			"in",
			"program",
			"until",
			"dispinterface",
			"inherited",
			"property",
			"uses",
			"div",
			"initialization",
			"raise",
			"var",
			"do",
			"inline",
			"record",
			"while",
			"downto",
			"interface",
			"repeat",
			"with",
			"else",
			"is",
			"resourcestring",
			"xor",
			"end",
			"label",
			"set",
			"except",
			"library",
			"shl",
			"private",
			"public",
			"published",
			"protected",
			"override"
		};
		static int nKeyWordsPASCAL_Num = sizeof( ppszKeyWordsPASCAL ) / sizeof( ppszKeyWordsPASCAL[0] );
		for( i = 0; i < nKeyWordsPASCAL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 8, ppszKeyWordsPASCAL[i] );
		}


		static const char*	ppszKeyWordsTEX[] = {
//Nov. 20, 2000 JEPRO	�啝�ǉ� & �኱�C���E�폜 --�قƂ�ǃR�}���h�̂�
			"error",
			"Warning",
//			"center",
//			"document",
//			"enumerate",
//			"eqnarray",
//			"figure",
//			"itemize",
//			"minipage",
//			"tabbing",
//			"table",
//			"tabular",
//			"\\!",
//			"\\#",
//			"\\%",
//			"\\&",
//			"\\(",
//			"\\)",
//			"\\+",
//			"\\,",
//			"\\-",
//			"\\:",
//			"\\;",
//			"\\<",
//			"\\=",
//			"\\>",
			"\\aa",
			"\\AA",
			"\\acute",
			"\\addcontentsline",
			"\\addtocounter",
			"\\addtolength",
			"\\ae",
			"\\AE",
			"\\aleph",
			"\\alpha",
			"\\alph",
			"\\Alph",
			"\\and",
			"\\angle",
			"\\appendix",
			"\\approx",
			"\\arabic",
			"\\arccos",
			"\\arctan",
			"\\arg",
			"\\arrayrulewidth",
			"\\arraystretch",
			"\\ast",
			"\\atop",
			"\\author",
			"\\b",
			"\\backslash",
			"\\bar",
			"\\baselineskip",
			"\\baselinestretch",
			"\\begin",
			"\\beta",
			"\\bf",
			"\\bibitem",
			"\\bibliography",
			"\\bibliographystyle",
			"\\big",
			"\\Big",
			"\\bigcap",
			"\\bigcirc",
			"\\bigcup",
			"\\bigg",
			"\\Bigg",
			"\\Biggl",
			"\\Biggm",
			"\\biggl",
			"\\biggm",
			"\\biggr",
			"\\Biggr",
			"\\bigl",
			"\\bigm",
			"\\Bigm",
			"\\Bigl",
			"\\bigodot",
			"\\bigoplus",
			"\\bigotimes",
			"\\bigr",
			"\\Bigr",
			"\\bigskip",
			"\\bigtriangledown",
			"\\bigtriangleup",
			"\\boldmath",
			"\\bot",
			"\\Box",
			"\\brace",
			"\\breve",
			"\\bullet",
			"\\bye",
			"\\c",
			"\\cal",
			"\\cap",
			"\\caption",
			"\\cc",
			"\\cdot",
			"\\cdots",
			"\\centering",
			"\\chapter",
			"\\check",
			"\\chi",
			"\\choose",
			"\\circ",
			"\\circle",
			"\\cite",
			"\\clearpage",
			"\\cline",
			"\\closing",
			"\\clubsuit",
			"\\colon",
			"\\columnsep",
			"\\columnseprule",
			"\\cong",
			"\\cot",
			"\\coth",
			"\\cr",
			"\\cup",
			"\\d",
			"\\dag",
			"\\dagger",
			"\\date",
			"\\dashbox",
			"\\ddag",
			"\\ddot",
			"\\ddots",
			"\\def",
			"\\deg",
			"\\delta",
			"\\Delta",
			"\\det",
			"\\diamond",
			"\\diamondsuit",
			"\\dim",
			"\\displaystyle",
			"\\documentclass",
			"\\documentstyle",
			"\\dot",
			"\\doteq",
			"\\dotfill",
			"\\Downarrow",
			"\\downarrow",
			"\\ell",
			"\\em",
			"\\emptyset",
			"\\encl",
			"\\end",
			"\\enspace",
			"\\enskip",
			"\\epsilon",
			"\\eqno",
			"\\equiv",
			"\\evensidemargin",
			"\\eta",
			"\\exists",
			"\\exp",
			"\\fbox",
			"\\fboxrule",
			"\\flat",
			"\\footnote",
			"\\footnotesize",
			"\\forall",
			"\\frac",
			"\\frame",
			"\\framebox",
			"\\gamma",
			"\\Gamma",
			"\\gcd",
			"\\ge",
			"\\geq",
			"\\gets",
			"\\gg",
			"\\grave",
			"\\gt",
			"\\H",
			"\\hat",
			"\\hbar",
			"\\hbox",
			"\\headsep",
			"\\heartsuit",
			"\\hfil",
			"\\hfill",
			"\\hline",
			"\\hom",
			"\\hrulefill",
			"\\hskip",
			"\\hspace",
			"\\hspace*",
			"\\huge",
			"\\Huge",
			"\\i",
			"\\Im",
			"\\imath",
			"\\in",
			"\\include",
			"\\includegraphics",
			"\\includeonly",
			"\\indent",
			"\\index",
			"\\inf",
			"\\infty",
			"\\input",
			"\\int",
			"\\iota",
			"\\it",
			"\\item",
			"\\itemsep",
			"\\j",
			"\\jmath",
			"\\kappa",
			"\\ker",
			"\\kern",
			"\\kill",
			"\\l",
			"\\L",
			"\\label",
			"\\lambda",
			"\\Lambda",
			"\\land",
			"\\langle",
			"\\large",
			"\\Large",
			"\\LARGE",
			"\\LaTeX",
			"\\LaTeXe",
			"\\lceil",
			"\\ldots",
			"\\le",
			"\\leftarrow",
			"\\Leftarrow",
			"\\lefteqn",
			"\\leftharpoondown",
			"\\leftharpoonup",
			"\\leftmargin",
			"\\leftrightarrow",
			"\\Leftrightarrow",
			"\\leq",
			"\\leqno",
			"\\lfloor",
			"\\lg",
			"\\lim",
			"\\liminf",
			"\\limsup",
			"\\line",
			"\\linebreak",
			"\\linewidth",
			"\\listoffigures",
			"\\listoftables",
			"\\ll",
			"\\llap",
			"\\ln",
			"\\lnot",
			"\\log",
			"\\longleftarrow",
			"\\Longleftarrow",
			"\\longleftrightarrow",
			"\\Longleftrightarrow",
			"\\longrightarrow",
			"\\Longrightarrow",
			"\\lor",
			"\\lower",
			"\\magstep",
			"\\makeatletter",
			"\\makeatother",
			"\\makebox",
			"\\makeindex",
			"\\maketitle",
			"\\makelabels",
			"\\mathop",
			"\\mapsto",
			"\\markboth",
			"\\markright",
			"\\mathstrut",
			"\\max",
			"\\mbox",
			"\\mc",
			"\\medskip",
			"\\mid",
			"\\min",
			"\\mit",
			"\\mp",
			"\\mu",
			"\\multicolumn",
			"\\multispan",
			"\\multiput",
			"\\nabla",
			"\\natural",
			"\\ne",
			"\\neg",
			"\\nearrow",
			"\\nwarrow",
			"\\neq",
			"\\newblock",
			"\\newcommand",
			"\\newenvironment",
			"\\newfont",
			"\\newlength",
			"\\newline",
			"\\newpage",
			"\\newtheorem",
			"\\ni",
			"\\noalign",
			"\\noindent",
			"\\nolimits",
			"\\nolinebreak",
			"\\nonumber",
			"\\nopagebreak",
			"\\normalsize",
			"\\not",
			"\\notice",
			"\\notin",
			"\\nu",
			"\\o",
			"\\O",
			"\\oddsidemargin",
			"\\odot",
			"\\oe",
			"\\OE",
			"\\oint",
			"\\Omega",
			"\\omega",
			"\\ominus",
			"\\oplus",
			"\\opening",
			"\\otimes",
			"\\owns",
			"\\overleftarrow",
			"\\overline",
			"\\overrightarrow",
			"\\overvrace",
			"\\oval",
			"\\P",
			"\\pagebreak",
			"\\pagenumbering",
			"\\pageref",
			"\\pagestyle",
			"\\par",
			"\\parallel",
			"\\paragraph",
			"\\parbox",
			"\\parindent",
			"\\parskip",
			"\\partial",
			"\\perp",
			"\\phi",
			"\\Phi",
			"\\pi",
			"\\Pi",
			"\\pm",
			"\\Pr",
			"\\prime",
			"\\printindex",
			"\\prod",
			"\\propto",
			"\\ps",
			"\\psi",
			"\\Psi",
			"\\put",
			"\\qquad",
			"\\quad",
			"\\raisebox",
			"\\rangle",
			"\\rceil",
			"\\Re",
			"\\ref",
			"\\renewcommand",
			"\\renewenvironment",
			"\\rfloor",
			"\\rho",
			"\\right",
			"\\rightarrow",
			"\\Rightarrow",
			"\\rightharpoondown",
			"\\rightharpoonup",
			"\\rightleftharpoonup",
			"\\rightmargin",
			"\\rm",
			"\\rule",
			"\\roman",
			"\\Roman",
			"\\S",
			"\\samepage",
			"\\sb",
			"\\sc",
			"\\scriptsize",
			"\\scriptscriptstyle",
			"\\scriptstyle",
			"\\searrow",
			"\\sec",
			"\\section",
			"\\setcounter",
			"\\setlength",
			"\\settowidth",
			"\\setminus",
			"\\sf",
			"\\sharp",
			"\\sigma",
			"\\Sigma",
			"\\signature",
			"\\sim",
			"\\simeq",
			"\\sin",
			"\\sinh",
			"\\sl",
			"\\sloppy",
			"\\small",
			"\\smash",
			"\\smallskip",
			"\\sp",
			"\\spadesuit",
			"\\special",
			"\\sqrt",
			"\\ss",
			"\\star",
			"\\stackrel",
			"\\strut",
			"\\subparagraph",
			"\\subsection",
			"\\subset",
			"\\subseteq",
			"\\subsubsection",
			"\\sum",
			"\\sup",
			"\\supset",
			"\\supseteq",
			"\\swarrow",
			"\\t",
			"\\tableofcontents",
			"\\tan",
			"\\tanh",
			"\\tau",
			"\\TeX",
			"\\textbf",
			"\\textgreater",
			"\\textgt",
			"\\textheight",
			"\\textit",
			"\\textless",
			"\\textmc",
			"\\textrm",
			"\\textsc",
			"\\textsf",
			"\\textsl",
			"\\textstyle",
			"\\texttt",
			"\\textwidth",
			"\\thanks",
			"\\thebibliography",
			"\\theequation",
			"\\thepage",
			"\\thesection",
			"\\theta",
			"\\Theta",
			"\\thicklines",
			"\\thinlines",
			"\\thinspace",
			"\\thisepage",
			"\\thisepagestyle",
			"\\tie",
			"\\tilde",
			"\\times",
			"\\tiny",
			"\\title",
			"\\titlepage",
			"\\to",
			"\\toaddress",
			"\\topmargin",
			"\\triangle",
			"\\tt",
			"\\twocolumn",
			"\\u",
			"\\underline",
			"\\undervrace",
			"\\unitlength",
			"\\Uparrow",
			"\\uparrow",
			"\\updownarrow",
			"\\Updownarrow",
			"\\uplus",
			"\\upsilon",
			"\\Upsilon",
			"\\usepackage",
			"\\v",
			"\\varepsilon",
			"\\varphi",
			"\\varpi",
			"\\varrho",
			"\\varsigma",
			"\\vartheta",
			"\\vbox",
			"\\vcenter",
			"\\vec",
			"\\vector",
			"\\vee",
			"\\verb",
			"\\verb*",
			"\\verbatim",
			"\\vert",
			"\\Vert",
			"\\vfil",
			"\\vfill",
			"\\vrule",
			"\\vskip",
			"\\vspace",
			"\\vspace*",
			"\\wedge",
			"\\widehat",
			"\\widetilde",
			"\\wp",
			"\\wr",
			"\\wrapfigure",
			"\\xi",
			"\\Xi",
			"\\zeta"//,
//			"\\[",
//			"\\\"",
//			"\\\'",
//			"\\\\",
//			"\\]",
//			"\\^",
//			"\\_",
//			"\\`",
//			"\\{",
//			"\\|",
//			"\\}",
//			"\\~",
		};
		static int nKeyWordsTEX_Num = sizeof( ppszKeyWordsTEX ) / sizeof( ppszKeyWordsTEX[0] );
		for( i = 0; i < nKeyWordsTEX_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 9, ppszKeyWordsTEX[i] );
		}


//Jan. 19, 2001 JEPRO	TeX �̃L�[���[�h2�Ƃ��ĐV�K�ǉ� & �ꕔ���� --���R�}���h�ƃI�v�V�����������S
		static const char*	ppszKeyWordsTEX2[] = {
//	���R�}���h
//Jan. 19, 2001 JEPRO �{����{}�t���ŃL�[���[�h�ɂ������������P��Ƃ��ĔF�����Ă���Ȃ��̂Ŏ~�߂�
			"abstract",
			"array"
			"center",
			"description",
			"document",
			"displaymath",
			"em",
			"enumerate",
			"eqnarray",
			"eqnarray*",
			"equation",
			"figure",
			"figure*",
			"floatingfigure",
			"flushleft",
			"flushright",
			"itemize",
			"letter",
			"list",
			"math",
			"minipage",
			"multicols",
			"namelist",
			"picture",
			"quotation",
			"quote",
			"sloppypar",
			"subeqnarray",
			"subeqnarray*",
			"subequations",
			"subfigure",
			"tabbing",
			"table",
			"table*",
			"tabular",
			"tabular*",
			"tatepage",
			"thebibliography",
			"theindex",
			"titlepage",
			"trivlist",
			"verbatim",
			"verbatim*",
			"verse",
			"wrapfigure",
//
//	�X�^�C���I�v�V����
			"a4",
			"a4j",
			"a5",
			"a5j",
			"Alph",
			"alph",
			"annote",
			"arabic",
			"b4",
			"b4j",
			"b5",
			"b5j",
			"bezier",
			"booktitle",
			"boxedminipage",
			"boxit",
//			"bp",
//			"cm",
			"dbltopnumber",
//			"dd",
			"eclepsf",
			"eepic",
			"enumi",
			"enumii",
			"enumiii",
			"enumiv",
			"epic",
			"epsbox",
			"epsf",
			"fancybox",
			"fancyheadings",
			"fleqn",
			"footnote",
			"howpublished",
			"jabbrv",
			"jalpha",
//			"article",
			"jarticle",
			"jsarticle",
//			"book",
			"jbook",
			"jsbook",
//			"letter",
			"jletter",
//			"plain",
			"jplain",
//			"report",
			"jreport",
			"jtwocolumn",
			"junsrt",
			"leqno",
			"makeidx",
			"markboth",
			"markright",
//			"mm",
			"multicol",
			"myheadings",
			"openbib",
//			"pc",
//			"pt",
			"secnumdepth",
//			"sp",
			"titlepage",
			"tjarticle",
			"topnumber",
			"totalnumber",
			"twocolumn",
			"twoside",
			"yomi"//,
//			"zh",
//			"zw"
		};
		static int nKeyWordsTEX2_Num = sizeof( ppszKeyWordsTEX2 ) / sizeof( ppszKeyWordsTEX2[0] );
		for( i = 0; i < nKeyWordsTEX2_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 10, ppszKeyWordsTEX2[i] );
		}


		static const char*	ppszKeyWordsPERL[] = {
//Jul. 10, 2001 JEPRO	�ϐ����Q�����L�[���[�h�Ƃ��ĕ�������
			"break",
			"continue",
			"do",
			"elsif",
			"else",
			"for",
			"foreach",
			"goto",
			"if",
			"last",
			"next",
			"return",
			"sub",
			"undef",
			"unless",
			"until",
			"while",
			"abs",
			"accept",
			"alarm",
			"atan2",
			"bind",
			"binmode",
			"bless",
			"caller",
			"chdir",
			"chmod",
			"chomp",
			"chop",
			"chown",
			"chr",
			"chroot",
			"close",
			"closedir",
			"connect",
			"continue",
			"cos",
			"crypt",
			"dbmclose",
			"dbmopen",
			"defined",
			"delete",
			"die",
			"do",
			"dump",
			"each",
			"eof",
			"eval",
			"exec",
			"exists",
			"exit",
			"exp",
			"fcntl",
			"fileno",
			"flock",
			"fork",
			"format",
			"formline",
			"getc",
			"getlogin",
			"getpeername",
			"getpgrp",
			"getppid",
			"getpriority",
			"getpwnam",
			"getgrnam",
			"gethostbyname",
			"getnetbyname",
			"getprotobyname",
			"getpwuid",
			"getgrgid",
			"getservbyname",
			"gethostbyaddr",
			"getnetbyaddr",
			"getprotobynumber",
			"getservbyport",
			"getpwent",
			"getgrent",
			"gethostent",
			"getnetent",
			"getprotoent",
			"getservent",
			"setpwent",
			"setgrent",
			"sethostent",
			"setnetent",
			"setprotoent",
			"setservent",
			"endpwent",
			"endgrent",
			"endhostent",
			"endnetent",
			"endprotoent",
			"endservent",
			"getsockname",
			"getsockopt",
			"glob",
			"gmtime",
			"goto",
			"grep",
			"hex",
			"import",
			"index",
			"int",
			"ioctl",
			"join",
			"keys",
			"kill",
			"last",
			"lc",
			"lcfirst",
			"length",
			"link",
			"listen",
			"local",
			"localtime",
			"log",
			"lstat",
//			"//m",
			"map",
			"mkdir",
			"msgctl",
			"msgget",
			"msgsnd",
			"msgrcv",
			"my",
			"next",
			"no",
			"oct",
			"open",
			"opendir",
			"ord",
			"pack",
			"package",
			"pipe",
			"pop",
			"pos",
			"print",
			"printf",
			"prototype",
			"push",
//			"//q",
			"qq",
			"qr",
			"qx",
			"qw",
			"quotemeta",
			"rand",
			"read",
			"readdir",
			"readline",
			"readlink",
			"readpipe",
			"recv",
			"redo",
			"ref",
			"rename",
			"require",
			"reset",
			"return",
			"reverse",
			"rewinddir",
			"rindex",
			"rmdir",
//			"//s",
			"scalar",
			"seek",
			"seekdir",
			"select",
			"semctl",
			"semget",
			"semop",
			"send",
			"setpgrp",
			"setpriority",
			"setsockopt",
			"shift",
			"shmctl",
			"shmget",
			"shmread",
			"shmwrite",
			"shutdown",
			"sin",
			"sleep",
			"socket",
			"socketpair",
			"sort",
			"splice",
			"split",
			"sprintf",
			"sqrt",
			"srand",
			"stat",
			"study",
			"sub",
			"substr",
			"symlink",
			"syscall",
			"sysopen",
			"sysread",
			"sysseek",
			"system",
			"syswrite",
			"tell",
			"telldir",
			"tie",
			"tied",
			"time",
			"times",
			"tr",
			"truncate",
			"uc",
			"ucfirst",
			"umask",
			"undef",
			"unlink",
			"unpack",
			"untie",
			"unshift",
			"use",
			"utime",
			"values",
			"vec",
			"wait",
			"waitpid",
			"wantarray",
			"warn",
			"write"
		};
		static int nKeyWordsPERL_Num = sizeof( ppszKeyWordsPERL ) / sizeof( ppszKeyWordsPERL[0] );
		for( i = 0; i < nKeyWordsPERL_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 11, ppszKeyWordsPERL[i] );
		}


		static const char*	ppszKeyWordsPERL2[] = {
//Jul. 10, 2001 JEPRO	�ϐ����Q�����L�[���[�h�Ƃ��ĕ�������
			"$ARGV",
			"$_",
			"$1",
			"$2",
			"$3",
			"$4",
			"$5",
			"$6",
			"$7",
			"$8",
			"$9",
			"$0",
			"$MATCH",
			"$&",
			"$PREMATCH",
			"$`",
			"$POSTMATCH",
			"$'",
			"$LAST_PAREN_MATCH",
			"$+",
			"$MULTILINE_MATCHING",
			"$*",
			"$INPUT_LINE_NUMBER",
			"$NR",
			"$.",
			"$INPUT_RECORD_SEPARATOR",
			"$RS",
			"$/",
			"$OUTPUT_AUTOFLUSH",
			"$|",
			"$OUTPUT_FIELD_SEPARATOR",
			"$OFS",
			"$,",
			"$OUTPUT_RECORD_SEPARATOR",
			"$ORS",
			"$\\",
			"$LIST_SEPARATOR",
			"$\"",
			"$SUBSCRIPT_SEPARATOR",
			"$SUBSEP",
			"$;",
			"$OFMT",
			"$#",
			"$FORMAT_PAGE_NUMBER",
			"$%",
			"$FORMAT_LINES_PER_PAGE",
			"$=",
			"$FORMAT_LINES_LEFT",
			"$-",
			"$FORMAT_NAME",
			"$~",
			"$FORMAT_TOP_NAME",
			"$^",
			"$FORMAT_LINE_BREAK_CHARACTERS",
			"$:",
			"$FORMAT_FORMFEED",
			"$^L",
			"$ACCUMULATOR",
			"$^A",
			"$CHILD_ERROR",
			"$?",
			"$OS_ERROR",
			"$ERRNO",
			"$!",
			"$EVAL_ERROR",
			"$@",
			"$PROCESS_ID",
			"$PID",
			"$$",
			"$REAL_USER_ID",
			"$UID",
			"$<",
			"$EFFECTIVE_USER_ID",
			"$EUID",
			"$>",
			"$REAL_GROUP_ID",
			"$GID",
			"$(",
			"$EFFECTIVE_GROUP_ID",
			"$EGID",
			"$)",
			"$PROGRAM_NAME",
			"$0",
			"$[",
			"$PERL_VERSION",
			"$]",
			"$DEBUGGING",
			"$^D",
			"$SYSTEM_FD_MAX",
			"$^F",
			"$INPLACE_EDIT",
			"$^I",
			"$PERLDB",
			"$^P",
			"$BASETIME",
			"$^T",
			"$WARNING",
			"$^W",
			"$EXECUTABLE_NAME",
			"$^X",
			"$ARGV",
			"$ENV",
			"$SIG"
		};
		static int nKeyWordsPERL2_Num = sizeof( ppszKeyWordsPERL2 ) / sizeof( ppszKeyWordsPERL2[0] );
		for( i = 0; i < nKeyWordsPERL2_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 12, ppszKeyWordsPERL2[i] );
		}


//Jul. 10, 2001 JEPRO �ǉ�
		static const char*	ppszKeyWordsVB[] = {
			"And",
			"As",
			"Attribute",
			"Begin",
			"BeginProperty",
			"Boolean",
			"ByVal",
			"Byte",
			"Call",
			"Case",
			"Const",
			"Currency",
			"Date",
			"Declare",
			"Dim",
			"Do",
			"Double",
			"Each",
			"Else",
			"ElseIf",
			"Empty",
			"End",
			"EndProperty",
			"Error",
			"Eqv",
			"Exit",
			"False",
			"For",
			"Friend",
			"Function",
			"Get",
			"GoTo",
			"If",
			"Imp",
			"Integer",
			"Is",
			"Let",
			"Like",
			"Long",
			"Loop",
			"Me",
			"Mod",
			"New",
			"Next",
			"Not",
			"Null",
			"Object",
			"On",
			"Option",
			"Or",
			"Private",
			"Property",
			"Public",
			"RSet",
			"ReDim",
			"Rem",
			"Resume",
			"Select",
			"Set",
			"Single",
			"Static",
			"Step",
			"Stop",
			"String",
			"Sub",
			"Then",
			"To",
			"True",
			"Type",
			"Wend",
			"While",
			"With",
			"Xor",
			"#If",
			"#Else",
			"#End",
			"#Const",
			"AddressOf",
			"Alias",
			"Append",
			"Array",
			"ByRef",
			"Explicit",
			"Global",
			"In",
			"Lib",
			"Nothing",
			"Optional",
			"Output",
			"Terminate",
			"Until",
//=========================================================
// �ȉ���VB.NET(VB7)�ł̔p�~�����肵�Ă���L�[���[�h�ł�
//=========================================================
			"DefBool",
			"DefByte",
			"DefCur",
			"DefDate",
			"DefDbl",
			"DefInt",
			"DefLng",
			"DefObj",
			"DefSng",
			"DefStr",
			"DefVar",
			"LSet",
			"GoSub",
			"Return",
			"Variant",
//			"Option Base
//			"As Any
//=========================================================
// �ȉ���VB.NET�p�L�[���[�h�ł�
//=========================================================
			//BitAnd
			//BitOr
			//BitNot
			//BitXor
			//Delegate
			//Short
			//Structure
		};
		static int nKeyWordsVB_Num = sizeof( ppszKeyWordsVB ) / sizeof( ppszKeyWordsVB[0] );
		for( i = 0; i < nKeyWordsVB_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 13, ppszKeyWordsVB[i] );
		}


//Jul. 10, 2001 JEPRO �ǉ�
		static const char*	ppszKeyWordsVB2[] = {
			"AppActivate",
			"Beep",
			"BeginTrans",
			"ChDir",
			"ChDrive",
			"Close",
			"CommitTrans",
			"CompactDatabase",
			"Date",
			"DeleteSetting",
			"Erase",
			"FileCopy",
			"FreeLocks",
			"Input",
			"Kill",
			"Load",
			"Lock",
			"Mid",
			"MidB",
			"MkDir",
			"Name",
			"Open",
			"Print",
			"Put",
			"Randomize",
			"RegisterDatabase",
			"RepairDatabase",
			"Reset",
			"RmDir",
			"Rollback",
			"SavePicture",
			"SaveSetting",
			"Seek",
			"SendKeys",
			"SetAttr",
			"SetDataAccessOption",
			"SetDefaultWorkspace",
			"Time",
			"Unload",
			"Unlock",
			"Width",
			"Write",
			"Array",
			"Asc",
			"AscB",
			"Atn",
			"CBool",
			"CByte",
			"CCur",
			"CDate",
			"CDbl",
			"CInt",
			"CLng",
			"CSng",
			"CStr",
			"CVErr",
			"CVar",
			"Choose",
			"Chr",
			"ChrB",
			"Command",
			"Cos",
			"CreateDatabase",
			"CreateObject",
			"CurDir",
			"DDB",
			"Date",
			"DateAdd",
			"DateDiff",
			"DatePart",
			"DateSerial",
			"DateValue",
			"Day",
			"Dir",
			"DoEvents",
			"EOF",
			"Environ",
			"Error",
			"Exp",
			"FV",
			"FileAttr",
			"FileDateTime",
			"FileLen",
			"Fix",
			"Format",
			"FreeFile",
			"GetAllSettings",
			"GetAttr",
			"GetObject",
			"GetSetting",
			"Hex",
			"Hour",
			"IIf",
			"IMEStatus",
			"IPmt",
			"IRR",
			"InStr",
			"Input",
			"Int",
			"IsArray",
			"IsDate",
			"IsEmpty",
			"IsError",
			"IsMissing",
			"IsNull",
			"IsNumeric",
			"IsObject",
			"LBound",
			"LCase",
			"LOF",
			"LTrim",
			"Left",
			"LeftB",
			"Len",
			"LoadPicture",
			"Loc",
			"Log",
			"MIRR",
			"Mid",
			"MidB",
			"Minute",
			"Month",
			"MsgBox",
			"NPV",
			"NPer",
			"Now",
			"Oct",
			"OpenDatabase",
			"PPmt",
			"PV",
			"Partition",
			"Pmt",
			"QBColor",
			"RGB",
			"RTrim",
			"Rate",
			"ReadProperty",
			"Right",
			"RightB",
			"Rnd",
			"SLN",
			"SYD",
			"Second",
			"Seek",
			"Sgn",
			"Shell",
			"Sin",
			"Space",
			"Spc",
			"Sqr",
			"Str",
			"StrComp",
			"StrConv",
			"Switch",
			"Tab",
			"Tan",
			"Time",
			"TimeSerial",
			"TimeValue",
			"Timer",
			"Trim",
			"TypeName",
			"UBound",
			"UCase",
			"Val",
			"VarType",
			"Weekday",
			"Year",
			"Hide",
			"Line",
			"Refresh",
			"Show",
//=========================================================
// �ȉ���VB.NET(VB7)�ł̔p�~�����肵�Ă���L�[���[�h�ł�
//=========================================================
//$�t���֐��e��
			"Dir$",
			"LCase$",
			"Left$",
			"LeftB$",
			"Mid$",
			"MidB$",
			"RightB$",
			"Right$",
			"Space$",
			"Str$",
			"String$",
			"Trim$",
			"UCase$",
//VB5,6�̉B���֐�
			"VarPtr",
			"StrPtr",
			"ObjPtr",
			"VarPrtArray",
			"VarPtrStringArray"
		};
		static int nKeyWordsVB2_Num = sizeof( ppszKeyWordsVB2 ) / sizeof( ppszKeyWordsVB2[0] );
		for( i = 0; i < nKeyWordsVB2_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 14, ppszKeyWordsVB2[i] );
		}


//Jul. 10, 2001 JEPRO �ǉ�
		static const char*	ppszKeyWordsRTF[] = {
			"\\ansi",
			"\\b",
			"\\bin",
			"\\box",
			"\\brdrb",
			"\\brdrbar",
			"\\brdrdb",
			"\\brdrdot",
			"\\brdrl",
			"\\brdrr",
			"\\brdrs",
			"\\brdrsh",
			"\\brdrt",
			"\\brdrth",
			"\\cell",
			"\\cellx",
			"\\cf",
			"\\chftn",
			"\\clmgf",
			"\\clmrg",
			"\\colortbl",
			"\\deff",
			"\\f",
			"\\fi",
			"\\field",
			"\\fldrslt",
			"\\fonttbl",
			"\\footnote",
			"\\fs",
			"\\i"
			"\\intbl",
			"\\keep",
			"\\keepn",
			"\\li",
			"\\line",
			"\\mac",
			"\\page",
			"\\par",
			"\\pard",
			"\\pc",
			"\\pich",
			"\\pichgoal",
			"\\picscalex",
			"\\picscaley",
			"\\pict",
			"\\picw",
			"\\picwgoal",
			"\\plain",
			"\\qc",
			"\\ql",
			"\\qr",
			"\\ri",
			"\\row",
			"\\rtf",
			"\\sa",
			"\\sb",
			"\\scaps",
			"\\sect",
			"\\sl",
			"\\strike",
			"\\tab",
			"\\tqc",
			"\\tqr",
			"\\trgaph",
			"\\trleft",
			"\\trowd",
			"\\trqc",
			"\\trql",
			"\\tx",
			"\\ul",
			"\\uldb",
			"\\v",
			"\\wbitmap",
			"\\wbmbitspixel",
			"\\wbmplanes",
			"\\wbmwidthbytes",
			"\\wmetafile",
			"bmc",
			"bml",
			"bmr",
			"emc",
			"eml",
			"emr"
		};
		static int nKeyWordsRTF_Num = sizeof( ppszKeyWordsRTF ) / sizeof( ppszKeyWordsRTF[0] );
		for( i = 0; i < nKeyWordsRTF_Num; ++i ){
			m_pShareData->m_CKeyWordSetMgr.AddKeyWord( 15, ppszKeyWordsRTF[i] );
		}

		//	From Here Sep. 14, 2001 genta
		//	Macro�o�^�̏�����
		MacroRec *mptr = m_pShareData->m_MacroTable;
		for( i = 0; i < MAX_CUSTMACRO; ++i, ++mptr ){
		//	mptr->m_bEnabled = FALSE;	// Oct 4. 2001 deleted by genta
			mptr->m_szName[0] = '\0';
			mptr->m_szFile[0] = '\0';
		}
		//	To Here Sep. 14, 2001 genta
		

	}else{
		/* �I�u�W�F�N�g�����łɑ��݂���ꍇ */
		/* �t�@�C���̃r���[�� �Ăяo�����v���Z�X�̃A�h���X��ԂɃ}�b�v���܂� */
		m_pShareData = (DLLSHAREDATA*)::MapViewOfFile(
			m_hFileMap,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			0
		);
		//	From Here Oct. 27, 2000 genta
		if( m_pShareData->m_vStructureVersion != uShareDataVersion ){
			//	���̋��L�f�[�^�̈�͎g���Ȃ��D
			//	�n���h�����������
			::UnmapViewOfFile( m_pShareData );
			m_pShareData = NULL;
			return false;
		}
		//	To Here Oct. 27, 2000 genta
	}
	return true;
}




//	/*
//	|| ���̎��s�t�@�C���̂���f�B���N�g����Ԃ��܂�(��)"c:\\Program Files\\caspy\\"
//	||
//	*/
//	long CShareData::GetModuleDir( char* pszDir, long nSize )
//	{
//		char	szDrive[_MAX_DRIVE];
//		char	szDir[_MAX_DIR];
//		::GetModuleFileName(
//			::GetModuleHandle( NULL ),
//			pszDir, nSize
//		);
//		_splitpath( pszDir, szDrive, szDir, NULL, NULL );
//		strcpy( pszDir, szDrive );
//		strcat( pszDir, szDir );
//		return strlen( pszDir );
//	}




/* KEYDATA�z��Ƀf�[�^���Z�b�g */
void CShareData::SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	char*			pszKeyName,
	short			nFuncCode_0,
	short			nFuncCode_1,
	short			nFuncCode_2,
	short			nFuncCode_3,
	short			nFuncCode_4,
	short			nFuncCode_5,
	short			nFuncCode_6,
	short			nFuncCode_7
 )
 {
	pShareData->m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	strcpy( pShareData->m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[0] = nFuncCode_0;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[1] = nFuncCode_1;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[2] = nFuncCode_2;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[3] = nFuncCode_3;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[4] = nFuncCode_4;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[5] = nFuncCode_5;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[6] = nFuncCode_6;
	pShareData->m_pKeyNameArr[nIdx].m_nFuncCodeArr[7] = nFuncCode_7;
 	return;
 }


/* KEYDATA�z��Ƀf�[�^���Z�b�g */
void CShareData::SetKeyNameArrVal(
	DLLSHAREDATA*	pShareData,
	int				nIdx,
	short			nKeyCode,
	char*			pszKeyName
 )
 {
	pShareData->m_pKeyNameArr[nIdx].m_nKeyCode = nKeyCode;
	strcpy( pShareData->m_pKeyNameArr[nIdx].m_szKeyName, pszKeyName );
	return;
 }




/* TBBUTTON�\���̂Ƀf�[�^���Z�b�g */
void CShareData::SetTBBUTTONVal(
	TBBUTTON*	ptb,
	int			iBitmap,
	int			idCommand,
	BYTE		fsState,
	BYTE		fsStyle,
	DWORD		dwData,
	int			iString
)
{
/*
typedef struct _TBBUTTON {
	int iBitmap;	// �{�^�� �C���[�W�� 0 ����n�܂�C���f�b�N�X
	int idCommand;	// �{�^���������ꂽ�Ƃ��ɑ�����R�}���h
	BYTE fsState;	// �{�^���̏��--�ȉ����Q��
	BYTE fsStyle;	// �{�^�� �X�^�C��--�ȉ����Q��
	DWORD dwData;	// �A�v���P�[�V����-��`���ꂽ�l
	int iString;	// �{�^���̃��x��������� 0 ����n�܂�C���f�b�N�X
} TBBUTTON;
*/

 	ptb->iBitmap	= iBitmap;
 	ptb->idCommand	= idCommand;
 	ptb->fsState	= fsState;
 	ptb->fsStyle	= fsStyle;
 	ptb->dwData		= dwData;
 	ptb->iString	= iString;
	return;
 }




/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
DLLSHAREDATA* CShareData::GetShareData( const char* pszFilePath, int* pnSettingType )
{

	char	szExt[_MAX_EXT];
	char	szText[256];
	int		i;
	char*	pszToken;
	char*	pszSeps = " ;,";
	BOOL	bFound;
	if( NULL != pnSettingType ){
		*pnSettingType = 0;
		if( NULL != pszFilePath && 0 < (int)strlen( pszFilePath ) ){
			_splitpath( pszFilePath, NULL, NULL, NULL, szExt );
			if( szExt[0] == '.' ){
				char	szExt2[_MAX_EXT];
				strcpy( szExt2, szExt );
				strcpy( szExt, &szExt2[1] );
			}

			bFound = FALSE;
			for( i = 0; i < MAX_TYPES; ++i ){
				strcpy( szText, m_pShareData->m_Types[i].m_szTypeExts );
				pszToken = strtok( szText, pszSeps );
				while( NULL != pszToken ){
					if( 0 == _stricmp( szExt, pszToken ) ){
						bFound = TRUE;
						break;
					}
					pszToken = strtok( NULL, pszSeps );
				}
				if( bFound ){
					break;
				}
			}
			if( i < MAX_TYPES ){
				*pnSettingType = i;
			}else{
				*pnSettingType = 0;
			}
		}
	}
	return m_pShareData;
}





/* �ҏW�E�B���h�E���X�g�ւ̓o�^ */
BOOL CShareData::AddEditWndList( HWND hWnd )
{
	int		i;
	int		j;
	/* �����E�B���h�E�n���h��������ꍇ�͐擪�Ɏ����Ă��� */
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( hWnd == m_pShareData->m_pEditArr[i].m_hWnd ){
			break;
		}
	}
	if( i < m_pShareData->m_nEditArrNum ){
		for( j = i; j > 0; j-- ){
			m_pShareData->m_pEditArr[j] = m_pShareData->m_pEditArr[j - 1];
		}
	}else{
		if( m_pShareData->m_nEditArrNum + 1 > MAX_EDITWINDOWS ){
			/* ����ȏ�o�^�ł��Ȃ� */
			return FALSE;
		}

		for( j = MAX_EDITWINDOWS - 1; j > 0; j-- ){
			m_pShareData->m_pEditArr[j] = m_pShareData->m_pEditArr[j - 1];
		}
		m_pShareData->m_nEditArrNum++;
		if( m_pShareData->m_nEditArrNum > MAX_EDITWINDOWS ){
			m_pShareData->m_nEditArrNum = MAX_EDITWINDOWS;
//#ifdef _DEBUG
//			/* �f�o�b�O���j�^�ɏo�� */
//			TraceOut( "%s(%d): m_nEditArrNum=%d\n", __FILE__, __LINE__, hWnd, m_pShareData->m_nEditArrNum );
//#endif
			/* ����ȏ�o�^�ł��Ȃ� */
			return FALSE;
		}
	}
	m_pShareData->m_pEditArr[0].m_hWnd = hWnd;

	/* �E�B���h�E�A�� */

	if( 0 == ::GetWindowLong( hWnd, 4 ) ){
		m_pShareData->m_nSequences++;
		::SetWindowLong( hWnd, 4, (LONG)m_pShareData->m_nSequences );
	}
	return TRUE;
}





/* �ҏW�E�B���h�E���X�g����̍폜 */
void CShareData::DeleteEditWndList( HWND hWnd )
{
	int		i;
	int		j;

	/* �E�B���h�E�n���h���̌��� */
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( hWnd == m_pShareData->m_pEditArr[i].m_hWnd ){
			break;
		}
	}
	if( i >= m_pShareData->m_nEditArrNum ){
		return;
	}
	for( j = i; j < m_pShareData->m_nEditArrNum - 1; ++j ){
		m_pShareData->m_pEditArr[j] = m_pShareData->m_pEditArr[j + 1];
	}
	m_pShareData->m_nEditArrNum--;
	return;
}

//!	MRU���X�g�ւ̓o�^
/*!
	@param pfi [in] �ǉ�����t�@�C���̏��

	�Y���t�@�C���������[�o�u���f�B�X�N��ɂ���ꍇ�ɂ�MRU List�ւ̓o�^�͍s��Ȃ��B

	@par History
	2001.03.29 �����[�o�u���f�B�X�N��̃t�@�C����o�^���Ȃ��悤�ɂ����B(by �݂�)
*/
void CShareData::AddMRUList( FileInfo* pfi )
{
	if( 0 == strlen( pfi->m_szPath ) ){
		return;
	}
	int		i;
	int		j;
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];

	_splitpath( pfi->m_szPath, szDrive, szDir, NULL, NULL );

	//@@@ 2001.03.29 Start by MIK
	char	szDriveType[_MAX_DRIVE+1];	// "A:\"
	long	lngRet;
	char	c;

	c = szDrive[0];
	if( c >= 'a' && c <= 'z' ){
		c = c - ('a' - 'A');
	}
	if( c >= 'A' && c <= 'Z' ){
		sprintf( szDriveType, "%c:\\", c );
		lngRet = GetDriveType( szDriveType );
		if( DRIVE_REMOVABLE	== lngRet
		 || DRIVE_CDROM		== lngRet){
			return;
		}
	}

	char*	pszFolder = new char[_MAX_PATH + 1];
	//@@@ 2001.03.29 End by MIK
	strcpy( pszFolder, szDrive );
	strcat( pszFolder, szDir );
	if( 0 < strlen( pszFolder ) ){
		/* �J�����t�H���_ ���X�g�ւ̓o�^ */
		AddOPENFOLDERList( pszFolder );
	}
	/* ����PATH������ꍇ�͐擪�ֈړ������� */
	for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
		if( 0 == _stricmp( pfi->m_szPath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
			break;
		}
	}
	if( i < m_pShareData->m_nMRUArrNum ){
		for( j = i; j > 0; j-- ){
			m_pShareData->m_fiMRUArr[j] = m_pShareData->m_fiMRUArr[j - 1];
		}
		m_pShareData->m_fiMRUArr[0] = *pfi;
	}else{
		for( j = MAX_MRU - 1; j > 0; j-- ){
			m_pShareData->m_fiMRUArr[j] = m_pShareData->m_fiMRUArr[j - 1];
		}
		m_pShareData->m_fiMRUArr[0] = *pfi;
		m_pShareData->m_nMRUArrNum++;
		if( m_pShareData->m_nMRUArrNum > MAX_MRU ){
			m_pShareData->m_nMRUArrNum = MAX_MRU;
		}
	}
	delete [] pszFolder;
	return;
}





/* �J�����t�H���_ ���X�g�ւ̓o�^ */
void CShareData::AddOPENFOLDERList( const char* pszFolder )
{
	if( 0 == strlen( pszFolder ) ){
		return;
	}

	int		i;
	int		j;
	char*	pszWork = new char[_MAX_PATH + 1];
	/* ����PATH������ꍇ�͐擪�ֈړ������� */
	for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
		if( 0 == _stricmp( pszFolder, m_pShareData->m_szOPENFOLDERArr[i] ) ){
			break;
		}
	}
	if( i < m_pShareData->m_nOPENFOLDERArrNum ){
		for( j = i; j > 0; j-- ){
			strcpy( m_pShareData->m_szOPENFOLDERArr[j], m_pShareData->m_szOPENFOLDERArr[j - 1] );
		}
		strcpy( m_pShareData->m_szOPENFOLDERArr[0], pszFolder );
	}else{
		for( j = MAX_OPENFOLDER - 1; j > 0; j-- ){
			strcpy( m_pShareData->m_szOPENFOLDERArr[j], m_pShareData->m_szOPENFOLDERArr[j - 1] );
		}
		strcpy( m_pShareData->m_szOPENFOLDERArr[0], pszFolder );
		m_pShareData->m_nOPENFOLDERArrNum++;
		if( m_pShareData->m_nOPENFOLDERArrNum > MAX_OPENFOLDER ){
			m_pShareData->m_nOPENFOLDERArrNum = MAX_OPENFOLDER;
		}
	}
	delete [] pszWork;
	return;
}


/*!
	�w�肳�ꂽ���O�̃t�@�C����MRU���X�g�ɑ��݂��邩���ׂ�B���݂���Ȃ�΃t�@�C������Ԃ��B

	@param pszPath [in] ��������t�@�C����
	@param pfi [out] �f�[�^�����������Ƃ��Ƀt�@�C�������i�[����̈�B
		�Ăяo�����ŗ̈�����炩���ߗp�ӂ���K�v������B
	@retval TRUE  �t�@�C�������������Bpfi�Ƀt�@�C����񂪊i�[����Ă���B
	@retval FALSE �w�肳�ꂽ�t�@�C����MRU List�ɖ����B
*/
BOOL CShareData::IsExistInMRUList( const char* pszPath, FileInfo* pfi )
{
	int		i;
	for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
		if( 0 == _stricmp( pszPath, m_pShareData->m_fiMRUArr[i].m_szPath ) ){
			break;
		}
	}
	if( i < m_pShareData->m_nMRUArrNum ){
		*pfi = m_pShareData->m_fiMRUArr[i];
		return TRUE;
	}else{
		return FALSE;
	}
}




/* ���L�f�[�^�̃��[�h */
BOOL CShareData::LoadShareData( void )
{
	return ShareData_IO_2( TRUE );
}




/* ���L�f�[�^�̕ۑ� */
void CShareData::SaveShareData( void )
{
	ShareData_IO_2( FALSE );
	return;
}




/* �S�ҏW�E�B���h�E�֏I���v�����o�� */
BOOL CShareData::RequestCloseAllEditor( void )
{
	HWND*	phWndArr;
	int		i;
	int		j;

	j = m_pShareData->m_nEditArrNum;
	if( 0 == j ){
		return TRUE;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = m_pShareData->m_pEditArr[i].m_hWnd;
	}
	for( i = 0; i < j; ++i ){
		if( IsEditWnd( phWndArr[i] ) ){
			/* �E�B���h�E���A�N�e�B�u�ɂ��� */
			/* �A�N�e�B�u�ɂ��� */
			ActivateFrameWindow( phWndArr[i] );
			/* �g���C����G�f�B�^�ւ̏I���v�� */
			if( !::SendMessage( phWndArr[i], MYWM_CLOSE, 0, 0 ) ){
				delete [] phWndArr;
				return FALSE;
			}
		}
	}
	delete [] phWndArr;
	return TRUE;
}




/* �w��t�@�C�����J����Ă��邩���ׂ� */
/* �J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h�����Ԃ� */
BOOL CShareData::IsPathOpened( const char* pszPath, HWND* phwndOwner )
{
	int			i;
	FileInfo*	pfi;
	*phwndOwner = NULL;

	/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */
	if( 0 ==  GetEditorWindowsNum() ){
		return FALSE;
	}
	//	Oct. 11, 2001 genta �t�@�C��������� stricmp��bcc�ł����Ғʂ蓮��������
	setlocale ( LC_ALL, "C" );
	
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
			::SendMessage( m_pShareData->m_pEditArr[i].m_hWnd, MYWM_GETFILEINFO, 1, 0 );
//			pfi = (FileInfo*)m_pShareData->m_szWork;
			pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;
			/* ����p�X�̃t�@�C�������ɊJ����Ă��邩 */
			if( 0 == _stricmp( pfi->m_szPath, pszPath ) ){
				*phwndOwner = m_pShareData->m_pEditArr[i].m_hWnd;
				return TRUE;
			}
		}
	}
	return FALSE;
}





/* ���݂̕ҏW�E�B���h�E�̐��𒲂ׂ� */
int CShareData::GetEditorWindowsNum( void )
{
	int		i;
	int		j;
	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			j++;
		}
	}
	return j;

}



/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
BOOL CShareData::PostMessageToAllEditors(
	UINT		uMsg,		/* �|�X�g���郁�b�Z�[�W */
	WPARAM		wParam,		/* ��1���b�Z�[�W �p�����[�^ */
	LPARAM		lParam,		/* ��2���b�Z�[�W �p�����[�^ */
	HWND		m_hWndLast	/* �Ō�ɑ��肽���E�B���h�E */
 )
{
	HWND*	phWndArr;
	int		i;
	int		j;
	j = m_pShareData->m_nEditArrNum;
	if( 0 == j ){
		return TRUE;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = m_pShareData->m_pEditArr[i].m_hWnd;
	}
	for( i = 0; i < j; ++i ){
		if( NULL != m_hWndLast && phWndArr[i] != m_hWndLast ){
			if( IsEditWnd( phWndArr[i] ) ){
				/* �g���C����G�f�B�^�փ��b�Z�[�W���|�X�g */
				::PostMessage( phWndArr[i], uMsg, wParam, lParam );
			}
		}
	}
	if( NULL != m_hWndLast && IsEditWnd( m_hWndLast ) ){
		/* �g���C����G�f�B�^�փ��b�Z�[�W���|�X�g */
		::PostMessage( m_hWndLast, uMsg, wParam, lParam );
	}
	delete [] phWndArr;
	return TRUE;
}


/* �S�ҏW�E�B���h�E�փ��b�Z�[�W�𑗂� */
BOOL CShareData::SendMessageToAllEditors(
	UINT		uMsg,		/* �|�X�g���郁�b�Z�[�W */
	WPARAM		wParam,		/* ��1���b�Z�[�W �p�����[�^ */
	LPARAM		lParam,		/* ��2���b�Z�[�W �p�����[�^ */
	HWND		m_hWndLast	/* �Ō�ɑ��肽���E�B���h�E */
 )
{
	HWND*	phWndArr;
	int		i;
	int		j;

	j = m_pShareData->m_nEditArrNum;
	if( 0 == j ){
		return TRUE;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = m_pShareData->m_pEditArr[i].m_hWnd;
	}
	for( i = 0; i < j; ++i ){
		if( NULL != m_hWndLast && phWndArr[i] != m_hWndLast ){
			if( IsEditWnd( phWndArr[i] ) ){
				/* �g���C����G�f�B�^�փ��b�Z�[�W���|�X�g */
				::SendMessage( phWndArr[i], uMsg, wParam, lParam );
			}
		}
	}
	if( NULL != m_hWndLast && IsEditWnd( m_hWndLast ) ){
		/* �g���C����G�f�B�^�փ��b�Z�[�W���|�X�g */
		::SendMessage( m_hWndLast, uMsg, wParam, lParam );
	}
	delete [] phWndArr;
	return TRUE;
}


/* �w��E�B���h�E���A�ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ� */
BOOL CShareData::IsEditWnd( HWND hWnd )
{
	char	szClassName[64];
	if( !::IsWindow( hWnd ) ){
		return FALSE;
	}
	if( 0 == ::GetClassName( hWnd, szClassName, sizeof(szClassName) - 1 ) ){
		return FALSE;
	}
	if(0 == strcmp( GSTR_EDITWINDOWNAME, szClassName ) ){
		return TRUE;
	}else{
		return FALSE;
	}

}

/* ���݊J���Ă���ҏW�E�B���h�E�̔z���Ԃ� */
/* �z��̗v�f����Ԃ� �v�f��>0 �̏ꍇ�͌Ăяo�����Ŕz���delete���Ă������� */
int CShareData::GetOpenedWindowArr( EditNode** ppEditNode, BOOL bSort )
{
	int			nRowNum;
	int			i;
	int			j;
	int			k;
	int			nMinIdx;
	int			nMin;
	HWND*		phWndArr;

	nRowNum = 0;
	// phWndArr = NULL;
	*ppEditNode = NULL;
	j = 0;
	for( i = 0; i < m_pShareData->m_nEditArrNum; ++i ){
		if( CShareData::IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
			j++;
		}
	}
	if( j > 0 ){
		phWndArr = new HWND[j];
		*ppEditNode = new EditNode[j];
		nRowNum = 0;
		for( i = 0;i < j; ++i ){
			phWndArr[i] = NULL;
		}
		k = 0;
		for( i = 0; i < m_pShareData->m_nEditArrNum && k < j; ++i ){
			if( CShareData::IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) ){
				phWndArr[k] = m_pShareData->m_pEditArr[i].m_hWnd;
				k++;
			}
		}
		if( bSort ){
			while( 1 ){
				nMinIdx = 99999;
				nMin = 99999;
				for( i = 0; i < j; ++i ){
					if( phWndArr[i] != NULL &&
						nMin > ::GetWindowLong( phWndArr[i], 4 )
					){
						nMinIdx = i;
						nMin = ::GetWindowLong( phWndArr[i], 4 );
					}
				}
				if( nMinIdx != 99999 ){
					i = nMinIdx;
					(*ppEditNode)[nRowNum].m_nIndex = i;
					(*ppEditNode)[nRowNum].m_hWnd = m_pShareData->m_pEditArr[i].m_hWnd;
					nRowNum++;
					phWndArr[i] = NULL;
				}else{
					break;
				}
			}
		}else{
			for( i = 0; i < k; ++i ){
				(*ppEditNode)[i].m_nIndex = i;
				(*ppEditNode)[i].m_hWnd = phWndArr[i];
			}
			nRowNum = k;
		}

//		LRESULT	lSuccrss;
//		DWORD	dwResult;
//		for( i = 0; i < nRowNum; ++i ){
//			(*ppEditNode)[i].m_szGrepKey[0] = '\0';
//			lSuccrss = ::SendMessageTimeout(
//				(*ppEditNode)[i].m_hWnd, MYWM_AREYOUGREP, 0, 0,
//				SMTO_ABORTIFHUNG | SMTO_NORMAL,
//				3000,
//				&dwResult
//			);
//			if( !lSuccrss ){
//				(*ppEditNode)[i].m_bIsGrep = -1;				/* Grep�̃E�B���h�E�� */
//			}else{
//				(*ppEditNode)[i].m_bIsGrep = (BOOL)dwResult;	/* Grep�̃E�B���h�E�� */
//				strcpy( (*ppEditNode)[i].m_szGrepKey, m_szWork );
//			}
//		}
		delete [] phWndArr;
	}
	return nRowNum;
}


/*!
	�A�E�g�v�b�g�E�C���h�E�ɏo��

	�A�E�g�v�b�g�E�C���h�E��������΃I�[�v������
	@param lpFmt [in] �����w�蕶����
*/
void CShareData::TraceOut( LPCTSTR lpFmt, ... )
{

	if( NULL == m_pShareData->m_hwndDebug
	|| !IsEditWnd( m_pShareData->m_hwndDebug )
	){
		CEditApp::OpenNewEditor( NULL, NULL, "-DEBUGMODE", CODE_SJIS, FALSE, true );
#if 0
		//	Jun. 25, 2001 genta OpenNewEditor�̓����@�\�𗘗p����悤�ɕύX
		//	2001/06/23 N.Nakatani �����o��܂ŃE�G�C�g��������悤�ɏC��
		//�A�E�g�v�b�g�E�C���h�E���o����܂�5�b���炢�҂B
		CRunningTimer wait_timer( NULL );
		while( NULL == m_pShareData->m_hwndDebug && 5000 > wait_timer.Read() ){
			Sleep(1);
		}
		Sleep(10);
		if( NULL == m_pShareData->m_hwndDebug ){
			return;
		}
#endif
		/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( m_pShareData->m_hwndDebug );
	}
	va_list argList;
	va_start( argList, lpFmt );
	wvsprintf( m_pShareData->m_szWork, lpFmt, argList );
	va_end( argList );
	::SendMessage( m_pShareData->m_hwndDebug, MYWM_ADDSTRING, 0, 0 );
	return;
}

/*!
	MRU��OPENFOLDER���X�g�̑��݃`�F�b�N�Ȃ�
	���݂��Ȃ��t�@�C����t�H���_��MRU��OPENFOLDER���X�g����폜����

	@note ���݂͎g���Ă��Ȃ��悤���B
*/
void CShareData::CheckMRUandOPENFOLDERList( void )
{
	int		i;
	int		j;
	/* MRU���X�g */
	for( i = 0; i < m_pShareData->m_nMRUArrNum; ++i ){
		/* ���݃`�F�b�N */
		if( -1 == _access( m_pShareData->m_fiMRUArr[i].m_szPath, 0 ) ){
			for( j = i + 1; j < m_pShareData->m_nMRUArrNum; ++j ){
				m_pShareData->m_fiMRUArr[j - 1] = m_pShareData->m_fiMRUArr[j];
			}
			i--;
			m_pShareData->m_nMRUArrNum--;
		}
	}
	/* OPENFOLDER���X�g */
	for( i = 0; i < m_pShareData->m_nOPENFOLDERArrNum; ++i ){
		/* ���݃`�F�b�N */
		if( -1 == _access( m_pShareData->m_szOPENFOLDERArr[i], 0 ) ){
			for( j = i + 1; j < m_pShareData->m_nOPENFOLDERArrNum; ++j ){
				strcpy( m_pShareData->m_szOPENFOLDERArr[j - 1], m_pShareData->m_szOPENFOLDERArr[j] );
			}
			i--;
			m_pShareData->m_nOPENFOLDERArrNum--;
		}
	}
	return;
}

//	/* �L�[���̂̃Z�b�g */
//	void CShareData::SetKeyNames( DLLSHAREDATA* pShareData )
//	{
//	int		i;
//	i = 0;
//	SetKeyNameArrVal( pShareData, i++, 0,"�_�u���N���b�N" );
//	SetKeyNameArrVal( pShareData, i++, 0,"�E�N���b�N" );
//	/* �t�@���N�V�����L�[ */
//	SetKeyNameArrVal( pShareData, i++, VK_F1,"F1" );
//	SetKeyNameArrVal( pShareData, i++, VK_F2,"F2" );
//	SetKeyNameArrVal( pShareData, i++, VK_F3,"F3" );
//	SetKeyNameArrVal( pShareData, i++, VK_F4,"F4" );
//	SetKeyNameArrVal( pShareData, i++, VK_F5,"F5" );
//	SetKeyNameArrVal( pShareData, i++, VK_F6,"F6" );
//	SetKeyNameArrVal( pShareData, i++, VK_F7,"F7" );
//	SetKeyNameArrVal( pShareData, i++, VK_F8,"F8" );
//	SetKeyNameArrVal( pShareData, i++, VK_F9,"F9" );
//	SetKeyNameArrVal( pShareData, i++, VK_F10,"F10" );
//	SetKeyNameArrVal( pShareData, i++, VK_F11,"F11" );
//	SetKeyNameArrVal( pShareData, i++, VK_F12,"F12" );
//	SetKeyNameArrVal( pShareData, i++, VK_F13,"F13" );
//	SetKeyNameArrVal( pShareData, i++, VK_F14,"F14" );
//	SetKeyNameArrVal( pShareData, i++, VK_F15,"F15" );
//	SetKeyNameArrVal( pShareData, i++, VK_F16,"F16" );
//	SetKeyNameArrVal( pShareData, i++, VK_F17,"F17" );
//	SetKeyNameArrVal( pShareData, i++, VK_F18,"F18" );
//	SetKeyNameArrVal( pShareData, i++, VK_F19,"F19" );
//	SetKeyNameArrVal( pShareData, i++, VK_F20,"F20" );
//	SetKeyNameArrVal( pShareData, i++, VK_F21,"F21" );
//	SetKeyNameArrVal( pShareData, i++, VK_F22,"F22" );
//	SetKeyNameArrVal( pShareData, i++, VK_F23,"F23" );
//	SetKeyNameArrVal( pShareData, i++, VK_F24,"F24" );
//	/* ����L�[ */
//	SetKeyNameArrVal( pShareData, i++, VK_TAB,"Tab" );
//	SetKeyNameArrVal( pShareData, i++, VK_RETURN,"Enter" );
//	SetKeyNameArrVal( pShareData, i++, VK_ESCAPE,"Esc" );
//	SetKeyNameArrVal( pShareData, i++, VK_BACK,"BackSpace" );
//	SetKeyNameArrVal( pShareData, i++, VK_INSERT,"Insert" );
//	SetKeyNameArrVal( pShareData, i++, VK_DELETE,"Delete" );
//	SetKeyNameArrVal( pShareData, i++, VK_HOME,"Home" );
//	SetKeyNameArrVal( pShareData, i++, VK_END,"End(Help)" );
//	SetKeyNameArrVal( pShareData, i++, VK_LEFT,"��" );
//	SetKeyNameArrVal( pShareData, i++, VK_UP,"��" );
//	SetKeyNameArrVal( pShareData, i++, VK_RIGHT,"��" );
//	SetKeyNameArrVal( pShareData, i++, VK_DOWN,"��" );
//	SetKeyNameArrVal( pShareData, i++, VK_PRIOR,"RollDown(PageUp)" );
//	SetKeyNameArrVal( pShareData, i++, VK_NEXT,"RollUp(PageDown)" );
//	SetKeyNameArrVal( pShareData, i++, VK_SPACE,"SpaceBar" );
//	/* ���� */
//	SetKeyNameArrVal( pShareData, i++, '0', "0" );
//	SetKeyNameArrVal( pShareData, i++, '1', "1" );
//	SetKeyNameArrVal( pShareData, i++, '2', "2" );
//	SetKeyNameArrVal( pShareData, i++, '3', "3" );
//	SetKeyNameArrVal( pShareData, i++, '4', "4" );
//	SetKeyNameArrVal( pShareData, i++, '5', "5" );
//	SetKeyNameArrVal( pShareData, i++, '6', "6" );
//	SetKeyNameArrVal( pShareData, i++, '7', "7" );
//	SetKeyNameArrVal( pShareData, i++, '8', "8" );
//	SetKeyNameArrVal( pShareData, i++, '9', "9" );
//	/* �A���t�@�x�b�g */
//	SetKeyNameArrVal( pShareData, i++, 'A', "A" );
//	SetKeyNameArrVal( pShareData, i++, 'B', "B" );
//	SetKeyNameArrVal( pShareData, i++, 'C', "C" );
//	SetKeyNameArrVal( pShareData, i++, 'D', "D" );
//	SetKeyNameArrVal( pShareData, i++, 'E', "E" );
//	SetKeyNameArrVal( pShareData, i++, 'F', "F" );
//	SetKeyNameArrVal( pShareData, i++, 'G', "G" );
//	SetKeyNameArrVal( pShareData, i++, 'H', "H" );
//	SetKeyNameArrVal( pShareData, i++, 'I', "I" );
//	SetKeyNameArrVal( pShareData, i++, 'J', "J" );
//	SetKeyNameArrVal( pShareData, i++, 'K', "K" );
//	SetKeyNameArrVal( pShareData, i++, 'L', "L" );
//	SetKeyNameArrVal( pShareData, i++, 'M', "M" );
//	SetKeyNameArrVal( pShareData, i++, 'N', "N" );
//	SetKeyNameArrVal( pShareData, i++, 'O', "O" );
//	SetKeyNameArrVal( pShareData, i++, 'P', "P" );
//	SetKeyNameArrVal( pShareData, i++, 'Q', "Q" );
//	SetKeyNameArrVal( pShareData, i++, 'R', "R" );
//	SetKeyNameArrVal( pShareData, i++, 'S', "S" );
//	SetKeyNameArrVal( pShareData, i++, 'T', "T" );
//	SetKeyNameArrVal( pShareData, i++, 'U', "U" );
//	SetKeyNameArrVal( pShareData, i++, 'V', "V" );
//	SetKeyNameArrVal( pShareData, i++, 'W', "W" );
//	SetKeyNameArrVal( pShareData, i++, 'X', "X" );
//	SetKeyNameArrVal( pShareData, i++, 'Y', "Y" );
//	SetKeyNameArrVal( pShareData, i++, 'Z', "Z" );
//	/* �L�� */
//	SetKeyNameArrVal( pShareData, i++, 0x00bd, "-" );
//	SetKeyNameArrVal( pShareData, i++, 0x00de, "^" );
//	SetKeyNameArrVal( pShareData, i++, 0x00dc, "\\" );
//	SetKeyNameArrVal( pShareData, i++, 0x00c0, "@" );
//	SetKeyNameArrVal( pShareData, i++, 0x00db, "[" );
//	SetKeyNameArrVal( pShareData, i++, 0x00bb, ";" );
//	SetKeyNameArrVal( pShareData, i++, 0x00ba, ":" );
//	SetKeyNameArrVal( pShareData, i++, 0x00dd, "]" );
//	SetKeyNameArrVal( pShareData, i++, 0x00bc, "," );
//	SetKeyNameArrVal( pShareData, i++, 0x00be, "." );
//	SetKeyNameArrVal( pShareData, i++, 0x00bf, "/" );
//	SetKeyNameArrVal( pShareData, i++, 0x00df, "_" );
//	return;
//	}


/*[EOF]*/
