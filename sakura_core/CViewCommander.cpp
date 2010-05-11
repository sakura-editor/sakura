/*!	@file
	@brief CEditView�N���X�̃R�}���h�����n�֐��Q

	@author Norio Nakatani
	@date	1998/07/17 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta, �݂�
	Copyright (C) 2001, MIK, Stonee, Misaka, asa-o, novice, hor, YAZAKI
	Copyright (C) 2002, hor, YAZAKI, novice, genta, aroka, Azumaiya, minfu, MIK, oak, ���Ȃӂ�, Moca, ai
	Copyright (C) 2003, MIK, genta, �����, zenryaku, Moca, ryoji, naoh, KEITA, ���イ��
	Copyright (C) 2004, isearch, Moca, gis_dur, genta, crayonzen, fotomo, MIK, novice, �݂��΂�, Kazika
	Copyright (C) 2005, genta, novice, �����, MIK, Moca, D.S.Koba, aroka, ryoji, maru
	Copyright (C) 2006, genta, aroka, ryoji, �����, fon, yukihane, Moca
	Copyright (C) 2007, ryoji, maru, Uchi
	Copyright (C) 2008, ryoji, nasukoji
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
// 2007.10.25 kobake CViewCommander�N���X�ɕ���

#include "StdAfx.h"
#include "CViewCommander.h"

#include <stdlib.h>
#include <io.h>
#include <mbstring.h>
#include "sakura_rc.h"
#include "view/CEditView.h"
#include "debug/Debug.h"
#include "func/Funccode.h"
#include "debug/CRunningTimer.h"
#include "charset/charcode.h"
#include "CControlTray.h"
#include "CWaitCursor.h"
#include "window/CSplitterWnd.h"
//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
#include "macro/CSMacroMgr.h"
#include "typeprop/CDlgTypeList.h"
#include "dlg/CDlgProperty.h"
#include "dlg/CDlgCompare.h"
#include "global.h"
#include <htmlhelp.h>
#include "debug/CRunningTimer.h"
#include "dlg/CDlgExec.h"
#include "dlg/CDlgAbout.h"	//Dec. 24, 2000 JEPRO �ǉ�
#include "COpe.h"/// 2002/2/3 aroka �ǉ� from here
#include "COpeBlk.h"///
#include "doc/CLayout.h"///
#include "window/CEditWnd.h"///
#include "outline/CFuncInfoArr.h"
#include "CMarkMgr.h"///
#include "doc/CDocLine.h"///
#include "macro/CSMacroMgr.h"///
#include "dlg/CDlgCancel.h"// 2002/2/8 hor
#include "CPrintPreview.h"
#include "mem/CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "dlg/CDlgCancel.h"
#include "dlg/CDlgTagJumpList.h"
#include "dlg/CDlgTagsMake.h"	//@@@ 2003.05.12 MIK
#include "COsVersionInfo.h"
#include "convert/CDecode_Base64Decode.h"
#include "convert/CDecode_UuDecode.h"
#include "io/CBinaryStream.h"
#include "CEditApp.h"
#include "util/window.h"
#include "util/file.h"
#include "util/module.h"
#include "util/shell.h"
#include "util/string_ex2.h"
#include "util/os.h"
#include "view/CEditView.h"
#include "window/CEditWnd.h"
#include "charset/CCodeFactory.h"
#include "io/CFileLoad.h"
#include "env/CSakuraEnvironment.h"
#include "plugin/CJackManager.h"
#include "plugin/COutlineIfObj.h"

//�O���ˑ�
CEditDoc* CViewCommander::GetDocument()
{
	return m_pCommanderView->m_pcEditDoc;
}
CEditWnd* CViewCommander::GetEditWindow()
{
	return m_pCommanderView->m_pcEditWnd;
}
HWND CViewCommander::GetMainWindow()
{
	return ::GetParent( m_pCommanderView->m_hwndParent );
}
COpeBlk* CViewCommander::GetOpeBlk()
{
	return m_pCommanderView->m_pcOpeBlk;
}
void CViewCommander::ClearOpeBlk()
{
	delete m_pCommanderView->m_pcOpeBlk;
	m_pCommanderView->m_pcOpeBlk=NULL;
}
void CViewCommander::SetOpeBlk(COpeBlk* p)
{
	m_pCommanderView->m_pcOpeBlk = p;
}
CLayoutRange& CViewCommander::GetSelect()
{
	return m_pCommanderView->GetSelectionInfo().m_sSelect;
}
CCaret& CViewCommander::GetCaret()
{
	return m_pCommanderView->GetCaret();
}



/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
BOOL CViewCommander::HandleCommand(
	EFunctionCode	nCommand,
	bool			bRedraw,
	LPARAM			lparam1,
	LPARAM			lparam2,
	LPARAM			lparam3,
	LPARAM			lparam4
)
{
	BOOL	bRet = TRUE;
	bool	bRepeat = false;
	int		nFuncID;

	//	May. 19, 2006 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX�����̂�
	//	����16�r�b�g�݂̂����o��
	//	Jul.  7, 2007 genta �萔�Ɣ�r���邽�߂ɃV�t�g���Ȃ��Ŏg��
	int nCommandFrom = nCommand & ~0xffff;
	nCommand = (EFunctionCode)LOWORD( nCommand );


	// -------------------------------------
	//	Jan. 10, 2005 genta
	//	Call message translators
	// -------------------------------------
	m_pCommanderView->TranslateCommand_grep( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );
	m_pCommanderView->TranslateCommand_isearch( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );

	//	Aug, 14. 2000 genta
	if( GetDocument()->IsModificationForbidden( nCommand ) ){
		return TRUE;
	}

	++GetDocument()->m_nCommandExecNum;		/* �R�}���h���s�� */
//	if( nCommand != F_COPY ){
		/* ����Tip������ */
		m_pCommanderView->m_cTipWnd.Hide();
		m_pCommanderView->m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
//	}
	/* ����v���r���[���[�h�� */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta
	if( pCEditWnd->m_pPrintPreview && F_PRINT_PREVIEW != nCommand ){
		ErrorBeep();
		return -1;
	}
	/* �L�[���s�[�g��� */
	if( m_bPrevCommand == nCommand ){
		bRepeat = TRUE;
	}
	m_bPrevCommand = nCommand;
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro &&									/* �L�[�{�[�h�}�N���̋L�^�� */
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow() &&	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
		( nCommandFrom & FA_NONRECORD ) != FA_NONRECORD	/* 2007.07.07 genta �L�^�}���t���O off */
	){
		/* �L�[���s�[�g��Ԃ��Ȃ����� */
		bRepeat = FALSE;
		/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		//F_EXECEXTMACRO�R�}���h�̓t�@�C����I��������Ƀ}�N�������m�肷�邽�ߌʂɋL�^����B
		if( CSMacroMgr::CanFuncIsKeyMacro( nCommand ) &&
			nCommand != F_EXECEXTMACRO	//F_EXECEXTMACRO�͌ʂŋL�^���܂�
		){
			/* �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
			//@@@ 2002.1.24 m_CKeyMacroMgr��CEditDoc�ֈړ�
			CEditApp::Instance()->m_pcSMacroMgr->Append( STAND_KEYMACRO, nCommand, lparam1, m_pCommanderView );
		}
	}

	//	2007.07.07 genta �}�N�����s���t���O�̐ݒ�
	//	�}�N������̃R�}���h���ǂ�����nCommandFrom�ł킩�邪
	//	nCommandFrom�������ŐZ��������̂���ςȂ̂ŁC�]���̃t���O�ɂ��l���R�s�[����
	m_pCommanderView->m_bExecutingKeyMacro = ( nCommandFrom & FA_FROMMACRO ) ? true : false;

	/* �L�[�{�[�h�}�N���̎��s�� */
	if( m_pCommanderView->m_bExecutingKeyMacro ){
		/* �L�[���s�[�g��Ԃ��Ȃ����� */
		bRepeat = FALSE;
	}

	//	From Here Sep. 29, 2001 genta �}�N���̎��s�@�\�ǉ�
	if( F_USERMACRO_0 <= nCommand && nCommand < F_USERMACRO_0 + MAX_CUSTMACRO ){
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���i�C���^�[�t�F�[�X�̕ύX�j
		if( !CEditApp::Instance()->m_pcSMacroMgr->Exec( nCommand - F_USERMACRO_0, G_AppInstance(), m_pCommanderView,
			nCommandFrom & FA_NONRECORD )){
			InfoMessage(
				this->m_pCommanderView->m_hwndParent,
				_T("�}�N�� %d (%ts) �̎��s�Ɏ��s���܂����B"),
				nCommand - F_USERMACRO_0,
				CEditApp::Instance()->m_pcSMacroMgr->GetFile( nCommand - F_USERMACRO_0 )
			);
		}
		return TRUE;
	}
	//	To Here Sep. 29, 2001 genta �}�N���̎��s�@�\�ǉ�

	// -------------------------------------
	//	Jan. 10, 2005 genta
	//	Call mode basis message handler
	// -------------------------------------
	m_pCommanderView->PreprocessCommand_hokan(nCommand);
	if( m_pCommanderView->ProcessCommand_isearch( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 ))
		return TRUE;

	// -------------------------------------
	//	Jan. 10, 2005 genta �R�����g
	//	�������O�ł�Undo�o�b�t�@�̏������ł��Ă��Ȃ��̂�
	//	�����̑�����s���Ă͂����Ȃ�
	//@@@ 2002.2.2 YAZAKI HandleCommand����HandleCommand���Ăяo���Ȃ����ɑΏ��i��������p������H�j
	if( NULL == GetOpeBlk() ){	/* ����u���b�N */
		SetOpeBlk(new COpeBlk);
	}
	
	//	Jan. 10, 2005 genta �R�����g
	//	��������ł�switch�̌���Undo�𐳂����o�^���邽�߁C
	//	�r���ŏ����̑ł��؂���s���Ă͂����Ȃ�
	// -------------------------------------

	switch( nCommand ){
	case F_WCHAR:	/* �������� */
		{
			/* �R���g���[���R�[�h���͋֎~ */
			if(WCODE::IsControlCode((wchar_t)lparam1)){
				ErrorBeep();
			}else{
				Command_WCHAR( (wchar_t)lparam1 );
			}
		}
		break;

	/* �t�@�C������n */
	case F_FILENEW:		Command_FILENEW();break;			/* �V�K�쐬 */
	//	Oct. 2, 2001 genta �}�N���p�@�\�g��
	case F_FILEOPEN:			Command_FILEOPEN((const WCHAR*)lparam1);break;			/* �t�@�C�����J�� */
	case F_FILEOPEN_DROPDOWN:	Command_FILEOPEN((const WCHAR*)lparam1);break;			/* �t�@�C�����J��(�h���b�v�_�E��) */	//@@@ 2002.06.15 MIK
	case F_FILESAVE:			bRet = Command_FILESAVE();break;	/* �㏑���ۑ� */
	case F_FILESAVEAS_DIALOG:	bRet = Command_FILESAVEAS_DIALOG();break;	/* ���O��t���ĕۑ� */
	case F_FILESAVEAS:			bRet = Command_FILESAVEAS((const WCHAR*)lparam1,(EEolType)lparam3);break;	/* ���O��t���ĕۑ� */
	case F_FILESAVEALL:			bRet = Command_FILESAVEALL();break;	/* �S�Ă̕ҏW�E�B���h�E�ŏ㏑���ۑ� */ // Jan. 23, 2005 genta
	case F_FILESAVE_QUIET:		bRet = Command_FILESAVE(false,false); break;	/* �Â��ɏ㏑���ۑ� */ // Jan. 24, 2005 genta
	case F_FILESAVECLOSE:
		//	Feb. 28, 2004 genta �ۑ�������
		//	�ۑ����s�v�Ȃ�P�ɕ���
		{	// Command_FILESAVE()�Ƃ͕ʂɕۑ��s�v���`�F�b�N	//### Command_FILESAVE() �͎��ۂɕۑ������ꍇ���� true ��Ԃ��悤�ɂȂ����i�d�l�ύX�H�j
			if( !GetDllShareData().m_Common.m_sFile.m_bEnableUnmodifiedOverwrite && !GetDocument()->m_cDocEditor.IsModified() ){
				Command_WINCLOSE();
				break;
			}
		}
		if( Command_FILESAVE( false, true )){
			Command_WINCLOSE();
		}
		break;
	case F_FILECLOSE:										//����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
		Command_FILECLOSE();
		break;
	case F_FILECLOSE_OPEN:	/* ���ĊJ�� */
		Command_FILECLOSE_OPEN();
		break;
	case F_FILE_REOPEN:				Command_FILE_REOPEN( GetDocument()->GetDocumentEncoding(), lparam1!=0 );break;//	Dec. 4, 2002 genta
	case F_FILE_REOPEN_SJIS:		Command_FILE_REOPEN( CODE_SJIS, lparam1!=0 );break;		//SJIS�ŊJ������
	case F_FILE_REOPEN_JIS:			Command_FILE_REOPEN( CODE_JIS, lparam1!=0 );break;		//JIS�ŊJ������
	case F_FILE_REOPEN_EUC:			Command_FILE_REOPEN( CODE_EUC, lparam1!=0 );break;		//EUC�ŊJ������
	case F_FILE_REOPEN_UNICODE:		Command_FILE_REOPEN( CODE_UNICODE, lparam1!=0 );break;	//Unicode�ŊJ������
	case F_FILE_REOPEN_UNICODEBE: 	Command_FILE_REOPEN( CODE_UNICODEBE, lparam1!=0 );break;	//UnicodeBE�ŊJ������
	case F_FILE_REOPEN_UTF8:		Command_FILE_REOPEN( CODE_UTF8, lparam1!=0 );break;		//UTF-8�ŊJ������
	case F_FILE_REOPEN_CESU8:		Command_FILE_REOPEN( CODE_CESU8, lparam1!=0 );break;	//CESU-8�ŊJ���Ȃ���
	case F_FILE_REOPEN_UTF7:		Command_FILE_REOPEN( CODE_UTF7, lparam1!=0 );break;		//UTF-7�ŊJ������
	case F_PRINT:				Command_PRINT();break;					/* ��� */
	case F_PRINT_PREVIEW:		Command_PRINT_PREVIEW();break;			/* ����v���r���[ */
	case F_PRINT_PAGESETUP:		Command_PRINT_PAGESETUP();break;		/* ����y�[�W�ݒ� */	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	case F_OPEN_HfromtoC:		bRet = Command_OPEN_HfromtoC( (BOOL)lparam1 );break;	/* ������C/C++�w�b�_(�\�[�X)���J�� */	//Feb. 7, 2001 JEPRO �ǉ�
//	case F_OPEN_HHPP:			bRet = Command_OPEN_HHPP( (BOOL)lparam1, TRUE );break;		/* ������C/C++�w�b�_�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX		del 2008/6/23 Uchi
//	case F_OPEN_CCPP:			bRet = Command_OPEN_CCPP( (BOOL)lparam1, TRUE );break;		/* ������C/C++�\�[�X�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX	del 2008/6/23 Uchi
	case F_ACTIVATE_SQLPLUS:	Command_ACTIVATE_SQLPLUS();break;		/* Oracle SQL*Plus���A�N�e�B�u�\�� */
	case F_PLSQL_COMPILE_ON_SQLPLUS:									/* Oracle SQL*Plus�Ŏ��s */
		Command_PLSQL_COMPILE_ON_SQLPLUS();
		break;
	case F_BROWSE:				Command_BROWSE();break;				/* �u���E�Y */
	case F_VIEWMODE:			Command_VIEWMODE();break;			/* �r���[���[�h */
	case F_PROPERTY_FILE:		Command_PROPERTY_FILE();break;		/* �t�@�C���̃v���p�e�B */
	case F_EXITALLEDITORS:		Command_EXITALLEDITORS();break;		/* �ҏW�̑S�I�� */	// 2007.02.13 ryoji �ǉ�
	case F_EXITALL:				Command_EXITALL();break;			/* �T�N���G�f�B�^�̑S�I�� */	//Dec. 26, 2000 JEPRO �ǉ�
	case F_PUTFILE:				Command_PUTFILE((LPCWSTR)lparam1, (ECodeType)lparam2, (int)lparam3);break;	/* ��ƒ��t�@�C���̈ꎞ�o�� */ //maru 2006.12.10
	case F_INSFILE:				Command_INSFILE((LPCWSTR)lparam1, (ECodeType)lparam2, (int)lparam3);break;	/* �L�����b�g�ʒu�Ƀt�@�C���}�� */ //maru 2006.12.10

	/* �ҏW�n */
	case F_UNDO:				Command_UNDO();break;				/* ���ɖ߂�(Undo) */
	case F_REDO:				Command_REDO();break;				/* ��蒼��(Redo) */
	case F_DELETE:				Command_DELETE(); break;			//�폜
	case F_DELETE_BACK:			Command_DELETE_BACK(); break;		//�J�[�\���O���폜
	case F_WordDeleteToStart:	Command_WordDeleteToStart(); break;	//�P��̍��[�܂ō폜
	case F_WordDeleteToEnd:		Command_WordDeleteToEnd(); break;	//�P��̉E�[�܂ō폜
	case F_WordDelete:			Command_WordDelete(); break;		//�P��폜
	case F_WordCut:				Command_WordCut(); break;			//�P��؂���
	case F_LineCutToStart:		Command_LineCutToStart(); break;	//�s���܂Ő؂���(���s�P��)
	case F_LineCutToEnd:		Command_LineCutToEnd(); break;		//�s���܂Ő؂���(���s�P��)
	case F_LineDeleteToStart:	Command_LineDeleteToStart(); break;	//�s���܂ō폜(���s�P��)
	case F_LineDeleteToEnd:		Command_LineDeleteToEnd(); break;	//�s���܂ō폜(���s�P��)
	case F_CUT_LINE:			Command_CUT_LINE();break;			//�s�؂���(�܂�Ԃ��P��)
	case F_DELETE_LINE:			Command_DELETE_LINE();break;		//�s�폜(�܂�Ԃ��P��)
	case F_DUPLICATELINE:		Command_DUPLICATELINE();break;		//�s�̓�d��(�܂�Ԃ��P��)
	case F_INDENT_TAB:			Command_INDENT( WCODE::TAB, INDENT_TAB );break;	//TAB�C���f���g
	case F_UNINDENT_TAB:		Command_UNINDENT( WCODE::TAB );break;		//�tTAB�C���f���g
	case F_INDENT_SPACE:		Command_INDENT( WCODE::SPACE, INDENT_SPACE );break;	//SPACE�C���f���g
	case F_UNINDENT_SPACE:			Command_UNINDENT( WCODE::SPACE );break;	//�tSPACE�C���f���g
//	case F_WORDSREFERENCE:			Command_WORDSREFERENCE();break;		/* �P�ꃊ�t�@�����X */
	case F_LTRIM:					Command_TRIM(TRUE);break;			// 2001.12.03 hor
	case F_RTRIM:					Command_TRIM(FALSE);break;			// 2001.12.03 hor
	case F_SORT_ASC:				Command_SORT(TRUE);break;			// 2001.12.06 hor
	case F_SORT_DESC:				Command_SORT(FALSE);break;			// 2001.12.06 hor
	case F_MERGE:					Command_MERGE();break;				// 2001.12.06 hor
	case F_RECONVERT:				Command_Reconvert();break;			/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */ 

	/* �J�[�\���ړ��n */
	case F_IME_CHAR:		Command_IME_CHAR( (WORD)lparam1 ); break;					//�S�p��������
	case F_UP:				Command_UP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, bRepeat ); break;				//�J�[�\����ړ�
	case F_DOWN:			Command_DOWN( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, bRepeat ); break;			//�J�[�\�����ړ�
	case F_LEFT:			Command_LEFT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, bRepeat ); break;			//�J�[�\�����ړ�
	case F_RIGHT:			Command_RIGHT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, FALSE, bRepeat ); break;	//�J�[�\���E�ړ�
	case F_UP2:				Command_UP2( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;						//�J�[�\����ړ�(�Q�s�Â�)
	case F_DOWN2:			Command_DOWN2( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;					//�J�[�\�����ړ�(�Q�s�Â�)
	case F_WORDLEFT:		Command_WORDLEFT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				/* �P��̍��[�Ɉړ� */
	case F_WORDRIGHT:		Command_WORDRIGHT( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				/* �P��̉E�[�Ɉړ� */
	//	0ct. 29, 2001 genta �}�N�������@�\�g��
	case F_GOLINETOP:		Command_GOLINETOP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, lparam1  ); break;		//�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOLINEEND:		Command_GOLINEEND( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, FALSE ); break;		//�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_ROLLDOWN:		Command_ROLLDOWN( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//�X�N���[���_�E��
//	case F_ROLLUP:			Command_ROLLUP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;					//�X�N���[���A�b�v
	case F_HalfPageUp:		Command_HalfPageUp( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	case F_HalfPageDown:	Command_HalfPageDown( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;			//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	case F_1PageUp:			Command_1PageUp( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;					//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	case F_1PageDown:		Command_1PageDown( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	case F_GOFILETOP:		Command_GOFILETOP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND:		Command_GOFILEEND( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;				//�t�@�C���̍Ō�Ɉړ�
	case F_CURLINECENTER:	Command_CURLINECENTER(); break;								/* �J�[�\���s���E�B���h�E������ */
	case F_JUMPHIST_PREV:	Command_JUMPHIST_PREV(); break;								//�ړ�����: �O��
	case F_JUMPHIST_NEXT:	Command_JUMPHIST_NEXT(); break;								//�ړ�����: ����
	case F_JUMPHIST_SET:	Command_JUMPHIST_SET(); break;								//���݈ʒu���ړ������ɓo�^
	case F_WndScrollDown:	Command_WndScrollDown(); break;								//�e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	case F_WndScrollUp:		Command_WndScrollUp(); break;								//�e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o
	case F_GONEXTPARAGRAPH:	Command_GONEXTPARAGRAPH( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;			//���̒i���֐i��
	case F_GOPREVPARAGRAPH:	Command_GOPREVPARAGRAPH( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ); break;			//�O�̒i���֖߂�

	/* �I���n */
	case F_SELECTWORD:		Command_SELECTWORD( );break;					//���݈ʒu�̒P��I��
	case F_SELECTALL:		Command_SELECTALL();break;						//���ׂđI��
	case F_SELECTLINE:		Command_SELECTLINE( lparam1 );break;			//1�s�I��	// 2007.10.13 nasukoji
	case F_BEGIN_SEL:		Command_BEGIN_SELECT();break;					/* �͈͑I���J�n */
	case F_UP_SEL:			Command_UP( TRUE, bRepeat, lparam1 ); break;	//(�͈͑I��)�J�[�\����ړ�
	case F_DOWN_SEL:		Command_DOWN( TRUE, bRepeat ); break;			//(�͈͑I��)�J�[�\�����ړ�
	case F_LEFT_SEL:		Command_LEFT( TRUE, bRepeat ); break;			//(�͈͑I��)�J�[�\�����ړ�
	case F_RIGHT_SEL:		Command_RIGHT( TRUE, FALSE, bRepeat ); break;	//(�͈͑I��)�J�[�\���E�ړ�
	case F_UP2_SEL:			Command_UP2( TRUE ); break;						//(�͈͑I��)�J�[�\����ړ�(�Q�s����)
	case F_DOWN2_SEL:		Command_DOWN2( TRUE );break;					//(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
	case F_WORDLEFT_SEL:	Command_WORDLEFT( TRUE );break;					//(�͈͑I��)�P��̍��[�Ɉړ�
	case F_WORDRIGHT_SEL:	Command_WORDRIGHT( TRUE );break;				//(�͈͑I��)�P��̉E�[�Ɉړ�
	case F_GOLINETOP_SEL:	Command_GOLINETOP( TRUE, 0 );break;				//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOLINEEND_SEL:	Command_GOLINEEND( TRUE, FALSE );break;			//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_ROLLDOWN_SEL:	Command_ROLLDOWN( TRUE ); break;				//(�͈͑I��)�X�N���[���_�E��
//	case F_ROLLUP_SEL:		Command_ROLLUP( TRUE ); break;					//(�͈͑I��)�X�N���[���A�b�v
	case F_HalfPageUp_Sel:	Command_HalfPageUp( TRUE ); break;				//(�͈͑I��)���y�[�W�A�b�v
	case F_HalfPageDown_Sel:Command_HalfPageDown( TRUE ); break;			//(�͈͑I��)���y�[�W�_�E��
	case F_1PageUp_Sel:		Command_1PageUp( TRUE ); break;					//(�͈͑I��)�P�y�[�W�A�b�v
	case F_1PageDown_Sel:	Command_1PageDown( TRUE ); break;				//(�͈͑I��)�P�y�[�W�_�E��
	case F_GOFILETOP_SEL:	Command_GOFILETOP( TRUE );break;				//(�͈͑I��)�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND_SEL:	Command_GOFILEEND( TRUE );break;				//(�͈͑I��)�t�@�C���̍Ō�Ɉړ�
	case F_GONEXTPARAGRAPH_SEL:	Command_GONEXTPARAGRAPH( TRUE ); break;			//���̒i���֐i��
	case F_GOPREVPARAGRAPH_SEL:	Command_GOPREVPARAGRAPH( TRUE ); break;			//�O�̒i���֖߂�

	/* ��`�I���n */
//	case F_BOXSELALL:		Command_BOXSELECTALL();break;		//��`�ł��ׂđI��
	case F_BEGIN_BOX:		Command_BEGIN_BOXSELECT();break;	/* ��`�͈͑I���J�n */
//	case F_UP_BOX:			Command_UP_BOX( bRepeat ); break;			//(��`�I��)�J�[�\����ړ�
//	case F_DOWN_BOX:		Command_DOWN( TRUE, bRepeat ); break;		//(��`�I��)�J�[�\�����ړ�
//	case F_LEFT_BOX:		Command_LEFT( TRUE, bRepeat ); break;		//(��`�I��)�J�[�\�����ړ�
//	case F_RIGHT_BOX:		Command_RIGHT( TRUE, FALSE, bRepeat ); break;//(��`�I��)�J�[�\���E�ړ�
//	case F_UP2_BOX:			Command_UP2( TRUE ); break;					//(��`�I��)�J�[�\����ړ�(�Q�s����)
//	case F_DOWN2_BOX:		Command_DOWN2( TRUE );break;				//(��`�I��)�J�[�\�����ړ�(�Q�s����)
//	case F_WORDLEFT_BOX:	Command_WORDLEFT( TRUE );break;				//(��`�I��)�P��̍��[�Ɉړ�
//	case F_WORDRIGHT_BOX:	Command_WORDRIGHT( TRUE );break;			//(��`�I��)�P��̉E�[�Ɉړ�
//	case F_GOLINETOP_BOX:	Command_GOLINETOP( TRUE, FALSE );break;		//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_GOLINEEND_BOX:	Command_GOLINEEND( TRUE, FALSE );break;		//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_HalfPageUp_Box:	Command_HalfPageUp( TRUE ); break;			//(��`�I��)���y�[�W�A�b�v
//	case F_HalfPageDown_Box:Command_HalfPageDown( TRUE ); break;		//(��`�I��)���y�[�W�_�E��
//	case F_1PageUp_Box:		Command_1PageUp( TRUE ); break;				//(��`�I��)�P�y�[�W�A�b�v
//	case F_1PageDown_Box:	Command_1PageDown( TRUE ); break;			//(��`�I��)�P�y�[�W�_�E��
//	case F_GOFILETOP_Box:	Command_GOFILETOP( TRUE );break;			//(��`�I��)�t�@�C���̐擪�Ɉړ�
//	case F_GOFILEEND_Box:	Command_GOFILEEND( TRUE );break;			//(��`�I��)�t�@�C���̍Ō�Ɉړ�

	/* �N���b�v�{�[�h�n */
	case F_CUT:						Command_CUT();break;					//�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	case F_COPY:					Command_COPY( false, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy );break;			//�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_ADDCRLF:			Command_COPY( false, true );break;		//�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_CRLF:				Command_COPY( false, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy, EOL_CRLF );break;	//CRLF���s�ŃR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_PASTE:					Command_PASTE();break;					//�\��t��(�N���b�v�{�[�h����\��t��)
	case F_PASTEBOX:				Command_PASTEBOX();break;				//��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	case F_INSTEXT_W:				Command_INSTEXT( bRedraw, (const wchar_t*)lparam1, CLogicInt(-1), lparam2!=0 );break;/* �e�L�X�g��\��t�� */ // 2004.05.14 Moca ���������������ǉ�(-1��\0�I�[�܂�)
	case F_ADDTAIL_W:				Command_ADDTAIL( (const wchar_t*)lparam1, (int)lparam2 );break;	/* �Ō�Ƀe�L�X�g��ǉ� */
	case F_COPYFNAME:				Command_COPYFILENAME();break;			//���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ / /2002/2/3 aroka
	case F_COPYPATH:				Command_COPYPATH();break;				//���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	case F_COPYTAG:					Command_COPYTAG();break;				//���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[	//Sept. 15, 2000 jepro ��Ɠ��������ɂȂ��Ă����̂��C��
	case F_COPYLINES:				Command_COPYLINES();break;				//�I��͈͓��S�s�R�s�[
	case F_COPYLINESASPASSAGE:		Command_COPYLINESASPASSAGE();break;		//�I��͈͓��S�s���p���t���R�s�[
	case F_COPYLINESWITHLINENUMBER:	Command_COPYLINESWITHLINENUMBER();break;//�I��͈͓��S�s�s�ԍ��t���R�s�[
	case F_CREATEKEYBINDLIST:		Command_CREATEKEYBINDLIST();break;		//�L�[���蓖�Ĉꗗ���R�s�[ //Sept. 15, 2000 JEPRO �ǉ� //Dec. 25, 2000 ����

	/* �}���n */
	case F_INS_DATE:				Command_INS_DATE();break;	//���t�}��
	case F_INS_TIME:				Command_INS_TIME();break;	//�����}��
    case F_CTRL_CODE_DIALOG:		Command_CtrlCode_Dialog();break;	/* �R���g���[���R�[�h�̓���(�_�C�A���O) */	//@@@ 2002.06.02 MIK

	/* �ϊ� */
	case F_TOLOWER:					Command_TOLOWER();break;				/* ������ */
	case F_TOUPPER:					Command_TOUPPER();break;				/* �啶�� */
	case F_TOHANKAKU:				Command_TOHANKAKU();break;				/* �S�p�����p */
	case F_TOHANKATA:				Command_TOHANKATA();break;				/* �S�p�J�^�J�i�����p�J�^�J�i */	//Aug. 29, 2002 ai
	case F_TOZENEI:					Command_TOZENEI();break;				/* �S�p�����p */					//July. 30, 2001 Misaka
	case F_TOHANEI:					Command_TOHANEI();break;				/* ���p���S�p */
	case F_TOZENKAKUKATA:			Command_TOZENKAKUKATA();break;			/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	case F_TOZENKAKUHIRA:			Command_TOZENKAKUHIRA();break;			/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	case F_HANKATATOZENKATA:	Command_HANKATATOZENKAKUKATA();break;	/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENHIRA:	Command_HANKATATOZENKAKUHIRA();break;	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_TABTOSPACE:				Command_TABTOSPACE();break;				/* TAB���� */
	case F_SPACETOTAB:				Command_SPACETOTAB();break;				/* �󔒁�TAB */  //---- Stonee, 2001/05/27
	case F_CODECNV_AUTO2SJIS:		Command_CODECNV_AUTO2SJIS();break;		/* �������ʁ�SJIS�R�[�h�ϊ� */
	case F_CODECNV_EMAIL:			Command_CODECNV_EMAIL();break;			/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	case F_CODECNV_EUC2SJIS:		Command_CODECNV_EUC2SJIS();break;		/* EUC��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UNICODE2SJIS:	Command_CODECNV_UNICODE2SJIS();break;	/* Unicode��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UNICODEBE2SJIS:	Command_CODECNV_UNICODEBE2SJIS();break;	/* UnicodeBE��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF82SJIS:		Command_CODECNV_UTF82SJIS();break;		/* UTF-8��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF72SJIS:		Command_CODECNV_UTF72SJIS();break;		/* UTF-7��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2JIS:		Command_CODECNV_SJIS2JIS();break;		/* SJIS��JIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2EUC:		Command_CODECNV_SJIS2EUC();break;		/* SJIS��EUC�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF8:		Command_CODECNV_SJIS2UTF8();break;		/* SJIS��UTF-8�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF7:		Command_CODECNV_SJIS2UTF7();break;		/* SJIS��UTF-7�R�[�h�ϊ� */
	case F_BASE64DECODE:			Command_BASE64DECODE();break;			/* Base64�f�R�[�h���ĕۑ� */
	case F_UUDECODE:				Command_UUDECODE();break;				/* uudecode���ĕۑ� */	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX

	/* �����n */
	case F_SEARCH_DIALOG:		Command_SEARCH_DIALOG();break;												//����(�P�ꌟ���_�C�A���O)
	case F_SEARCH_BOX:			Command_SEARCH_BOX();break;		// Jan. 13, 2003 MIK					//����(�{�b�N�X)	// 2006.06.04 yukihane Command_SEARCH_BOX()
	case F_SEARCH_NEXT:			Command_SEARCH_NEXT( true, bRedraw, (HWND)lparam1, (const WCHAR*)lparam2 );break;	//��������
	case F_SEARCH_PREV:			Command_SEARCH_PREV( bRedraw, (HWND)lparam1 );break;						//�O������
	case F_REPLACE_DIALOG:	//�u��(�u���_�C�A���O)
		/* �ċA�����΍� */
		if( NULL != GetOpeBlk() ){	/* ����u���b�N */
			ClearOpeBlk();
		}
		Command_REPLACE_DIALOG();	//@@@ 2002.2.2 YAZAKI �_�C�A���O�Ăяo���ƁA���s�𕪗�
		break;
	case F_REPLACE:				Command_REPLACE( (HWND)lparam1 );break;			//�u�����s @@@ 2002.2.2 YAZAKI
	case F_REPLACE_ALL:			Command_REPLACE_ALL();break;		//���ׂĒu�����s(�ʏ�) 2002.2.8 hor 2006.04.02 �����
	case F_SEARCH_CLEARMARK:	Command_SEARCH_CLEARMARK();break;	//�����}�[�N�̃N���A
	case F_GREP_DIALOG:	//Grep�_�C�A���O�̕\��
		/* �ċA�����΍� */
		if( NULL != GetOpeBlk() ){	/* ����u���b�N */
			ClearOpeBlk();
		}
		Command_GREP_DIALOG();
		break;
	case F_GREP:			Command_GREP();break;							//Grep
	case F_JUMP_DIALOG:		Command_JUMP_DIALOG();break;					//�w��s�w�W�����v�_�C�A���O�̕\��
	case F_JUMP:			Command_JUMP();break;							//�w��s�w�W�����v
	case F_OUTLINE:			bRet = Command_FUNCLIST( (int)lparam1 );break;	//�A�E�g���C�����
	case F_OUTLINE_TOGGLE:	bRet = Command_FUNCLIST( SHOW_TOGGLE );break;	//�A�E�g���C�����(toggle) // 20060201 aroka
	case F_TAGJUMP:			Command_TAGJUMP(lparam1 != 0);break;			/* �^�O�W�����v�@�\ */ //	Apr. 03, 2003 genta �����ǉ�
	case F_TAGJUMP_CLOSE:	Command_TAGJUMP(true);break;					/* �^�O�W�����v(���E�B���h�EClose) *///	Apr. 03, 2003 genta
	case F_TAGJUMPBACK:		Command_TAGJUMPBACK();break;					/* �^�O�W�����v�o�b�N�@�\ */
	case F_TAGS_MAKE:		Command_TagsMake();break;						//�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	case F_DIRECT_TAGJUMP:	Command_TagJumpByTagsFile();break;				/* �_�C���N�g�^�O�W�����v�@�\ */	//@@@ 2003.04.15 MIK
	case F_TAGJUMP_KEYWORD:	Command_TagJumpByTagsFileKeyword( (const wchar_t*)lparam1 );break;	/* @@ 2005.03.31 MIK �L�[���[�h���w�肵�ă_�C���N�g�^�O�W�����v�@�\ */
	case F_COMPARE:			Command_COMPARE();break;						/* �t�@�C�����e��r */
	case F_DIFF_DIALOG:		Command_Diff_Dialog();break;					/* DIFF�����\��(�_�C�A���O) */	//@@@ 2002.05.25 MIK
	case F_DIFF:			Command_Diff( (const WCHAR*)lparam1, (int)lparam2 );break;		/* DIFF�����\�� */	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	case F_DIFF_NEXT:		Command_Diff_Next();break;						/* DIFF�����\��(����) */		//@@@ 2002.05.25 MIK
	case F_DIFF_PREV:		Command_Diff_Prev();break;						/* DIFF�����\��(�O��) */		//@@@ 2002.05.25 MIK
	case F_DIFF_RESET:		Command_Diff_Reset();break;						/* DIFF�����\��(�S����) */		//@@@ 2002.05.25 MIK
	case F_BRACKETPAIR:		Command_BRACKETPAIR();	break;					//�Ί��ʂ̌���
// From Here 2001.12.03 hor
	case F_BOOKMARK_SET:	Command_BOOKMARK_SET();break;					/* �u�b�N�}�[�N�ݒ�E���� */
	case F_BOOKMARK_NEXT:	Command_BOOKMARK_NEXT();break;					/* ���̃u�b�N�}�[�N�� */
	case F_BOOKMARK_PREV:	Command_BOOKMARK_PREV();break;					/* �O�̃u�b�N�}�[�N�� */
	case F_BOOKMARK_RESET:	Command_BOOKMARK_RESET();break;					/* �u�b�N�}�[�N�̑S���� */
	case F_BOOKMARK_VIEW:	bRet = Command_FUNCLIST( (BOOL)lparam1 ,OUTLINE_BOOKMARK );break;	//�A�E�g���C�����
// To Here 2001.12.03 hor
	case F_BOOKMARK_PATTERN:Command_BOOKMARK_PATTERN();break;				// 2002.01.16 hor �w��p�^�[���Ɉ�v����s���}�[�N
	case F_JUMP_SRCHSTARTPOS:	Command_JUMP_SRCHSTARTPOS();break;			// �����J�n�ʒu�֖߂� 02/06/26 ai
	
	/* ���[�h�؂�ւ��n */
	case F_CHGMOD_INS:		Command_CHGMOD_INS();break;		//�}���^�㏑�����[�h�؂�ւ�
	// From Here 2003.06.23 Moca
	// F_CHGMOD_EOL_xxx �̓}�N���ɋL�^����Ȃ����AF_CHGMOD_EOL�̓}�N���ɋL�^�����̂ŁA�}�N���֐��𓝍��ł���Ƃ�����͂�
	case F_CHGMOD_EOL_CRLF:	HandleCommand( F_CHGMOD_EOL, bRedraw, EOL_CRLF, 0, 0, 0 );break;	//���͂�����s�R�[�h��CRLF�ɐݒ�
	case F_CHGMOD_EOL_LF:	HandleCommand( F_CHGMOD_EOL, bRedraw, EOL_LF, 0, 0, 0 );break;	//���͂�����s�R�[�h��LF�ɐݒ�
	case F_CHGMOD_EOL_CR:	HandleCommand( F_CHGMOD_EOL, bRedraw, EOL_CR, 0, 0, 0 );break;	//���͂�����s�R�[�h��CR�ɐݒ�
	// 2006.09.03 Moca F_CHGMOD_EOL�� break �Y��̏C��
	case F_CHGMOD_EOL:		Command_CHGMOD_EOL( (enumEOLType)lparam1 );break;	//���͂�����s�R�[�h��ݒ�
	// To Here 2003.06.23 Moca
	case F_CANCEL_MODE:		Command_CANCEL_MODE();break;	//�e�탂�[�h�̎�����

	/* �ݒ�n */
	case F_SHOWTOOLBAR:		Command_SHOWTOOLBAR();break;	/* �c�[���o�[�̕\��/��\�� */
	case F_SHOWFUNCKEY:		Command_SHOWFUNCKEY();break;	/* �t�@���N�V�����L�[�̕\��/��\�� */
	case F_SHOWTAB:			Command_SHOWTAB();break;		/* �^�u�̕\��/��\�� */	//@@@ 2003.06.10 MIK
	case F_SHOWSTATUSBAR:	Command_SHOWSTATUSBAR();break;	/* �X�e�[�^�X�o�[�̕\��/��\�� */
	case F_TYPE_LIST:		Command_TYPE_LIST();break;		/* �^�C�v�ʐݒ�ꗗ */
	case F_OPTION_TYPE:		Command_OPTION_TYPE();break;	/* �^�C�v�ʐݒ� */
	case F_OPTION:			Command_OPTION();break;			/* ���ʐݒ� */
	case F_FONT:			Command_FONT();break;			/* �t�H���g�ݒ� */
	case F_WRAPWINDOWWIDTH:	Command_WRAPWINDOWWIDTH();break;/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
	case F_FAVORITE:		Command_Favorite();break;		//�����̊Ǘ�	//@@@ 2003.04.08 MIK
	//	Jan. 29, 2005 genta ���p���̐ݒ�
	case F_SET_QUOTESTRING:	Command_SET_QUOTESTRING((const WCHAR*)lparam1);	break;
	case F_TMPWRAPNOWRAP:	HandleCommand( F_TEXTWRAPMETHOD, bRedraw, WRAP_NO_TEXT_WRAP, 0, 0, 0 );break;	// �܂�Ԃ��Ȃ��i�ꎞ�ݒ�j			// 2008.05.30 nasukoji
	case F_TMPWRAPSETTING:	HandleCommand( F_TEXTWRAPMETHOD, bRedraw, WRAP_SETTING_WIDTH, 0, 0, 0 );break;	// �w�茅�Ő܂�Ԃ��i�ꎞ�ݒ�j		// 2008.05.30 nasukoji
	case F_TMPWRAPWINDOW:	HandleCommand( F_TEXTWRAPMETHOD, bRedraw, WRAP_WINDOW_WIDTH, 0, 0, 0 );break;	// �E�[�Ő܂�Ԃ��i�ꎞ�ݒ�j		// 2008.05.30 nasukoji
	case F_TEXTWRAPMETHOD:	Command_TEXTWRAPMETHOD( (int)lparam1 );break;		// �e�L�X�g�̐܂�Ԃ����@		// 2008.05.30 nasukoji
	case F_SELECT_COUNT_MODE:	Command_SELECT_COUNT_MODE( (int)lparam1 );break;	// �����J�E���g�̕��@		// 2009.07.06 syat

	/* �}�N���n */
	case F_RECKEYMACRO:		Command_RECKEYMACRO();break;	/* �L�[�}�N���̋L�^�J�n�^�I�� */
	case F_SAVEKEYMACRO:	Command_SAVEKEYMACRO();break;	/* �L�[�}�N���̕ۑ� */
	case F_LOADKEYMACRO:	Command_LOADKEYMACRO();break;	/* �L�[�}�N���̓ǂݍ��� */
	case F_EXECKEYMACRO:									/* �L�[�}�N���̎��s */
		/* �ċA�����΍� */
		if( NULL != GetOpeBlk() ){	/* ����u���b�N */
			ClearOpeBlk();
		}
		Command_EXECKEYMACRO();break;
	case F_EXECEXTMACRO:
		/* �ċA�����΍� */
		if( NULL != GetOpeBlk() ){	/* ����u���b�N */
			ClearOpeBlk();
		}
		/* ���O���w�肵�ă}�N�����s */
		Command_EXECEXTMACRO( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		break;
	//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
	//	case F_EXECCMMAND:		Command_EXECCMMAND();break;	/* �O���R�}���h���s */
	case F_EXECMD_DIALOG:
		/* �ċA�����΍� */// 2001/06/23 N.Nakatani
		if( NULL != GetOpeBlk() ){	/* ����u���b�N */
			ClearOpeBlk();
		}
		//Command_EXECCOMMAND_DIALOG((const char*)lparam1);	/* �O���R�}���h���s */
		Command_EXECCOMMAND_DIALOG();	/* �O���R�}���h���s */	//	���������ĂȂ��݂����Ȃ̂�
		break;
	//	To Here Sept. 20, 2000
	case F_EXECMD:
		//Command_EXECCOMMAND((const char*)lparam1);
		Command_EXECCOMMAND((LPCWSTR)lparam1, (int)lparam2);	//	2006.12.03 maru �����̊g���̂���
		break;

	/* �J�X�^�����j���[ */
	case F_MENU_RBUTTON:	/* �E�N���b�N���j���[ */
		/* �ċA�����΍� */
		if( NULL != GetOpeBlk() ){	/* ����u���b�N */
			ClearOpeBlk();
		}
		Command_MENU_RBUTTON();
		break;
	case F_CUSTMENU_1:  /* �J�X�^�����j���[1 */
	case F_CUSTMENU_2:  /* �J�X�^�����j���[2 */
	case F_CUSTMENU_3:  /* �J�X�^�����j���[3 */
	case F_CUSTMENU_4:  /* �J�X�^�����j���[4 */
	case F_CUSTMENU_5:  /* �J�X�^�����j���[5 */
	case F_CUSTMENU_6:  /* �J�X�^�����j���[6 */
	case F_CUSTMENU_7:  /* �J�X�^�����j���[7 */
	case F_CUSTMENU_8:  /* �J�X�^�����j���[8 */
	case F_CUSTMENU_9:  /* �J�X�^�����j���[9 */
	case F_CUSTMENU_10: /* �J�X�^�����j���[10 */
	case F_CUSTMENU_11: /* �J�X�^�����j���[11 */
	case F_CUSTMENU_12: /* �J�X�^�����j���[12 */
	case F_CUSTMENU_13: /* �J�X�^�����j���[13 */
	case F_CUSTMENU_14: /* �J�X�^�����j���[14 */
	case F_CUSTMENU_15: /* �J�X�^�����j���[15 */
	case F_CUSTMENU_16: /* �J�X�^�����j���[16 */
	case F_CUSTMENU_17: /* �J�X�^�����j���[17 */
	case F_CUSTMENU_18: /* �J�X�^�����j���[18 */
	case F_CUSTMENU_19: /* �J�X�^�����j���[19 */
	case F_CUSTMENU_20: /* �J�X�^�����j���[20 */
	case F_CUSTMENU_21: /* �J�X�^�����j���[21 */
	case F_CUSTMENU_22: /* �J�X�^�����j���[22 */
	case F_CUSTMENU_23: /* �J�X�^�����j���[23 */
	case F_CUSTMENU_24: /* �J�X�^�����j���[24 */
		/* �ċA�����΍� */
		if( NULL != GetOpeBlk() ){	/* ����u���b�N */
			ClearOpeBlk();
		}
		nFuncID = Command_CUSTMENU( nCommand - F_CUSTMENU_1 + 1 );
		if( 0 != nFuncID ){
			/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
//			HandleCommand( nFuncID, TRUE, 0, 0, 0, 0 );
			::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nFuncID, 0 ), (LPARAM)NULL );
		}
		break;

	/* �E�B���h�E�n */
	case F_SPLIT_V:			Command_SPLIT_V();break;	/* �㉺�ɕ��� */	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	case F_SPLIT_H:			Command_SPLIT_H();break;	/* ���E�ɕ��� */	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	case F_SPLIT_VH:		Command_SPLIT_VH();break;	/* �c���ɕ��� */	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	case F_WINCLOSE:		Command_WINCLOSE();break;	//�E�B���h�E�����
	case F_WIN_CLOSEALL:	/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�����L�̂悤�ɕύX
		//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
		Command_FILECLOSEALL();
		break;
	case F_BIND_WINDOW:		Command_BIND_WINDOW();break;	//�������ĕ\�� 2004.07.14 Kazika �V�K�ǉ�
	case F_CASCADE:			Command_CASCADE();break;		//�d�˂ĕ\��
	case F_TILE_V:			Command_TILE_V();break;			//�㉺�ɕ��ׂĕ\��
	case F_TILE_H:			Command_TILE_H();break;			//���E�ɕ��ׂĕ\��
	case F_MAXIMIZE_V:		Command_MAXIMIZE_V();break;		//�c�����ɍő剻
	case F_MAXIMIZE_H:		Command_MAXIMIZE_H();break;		//�������ɍő剻 //2001.02.10 by MIK
	case F_MINIMIZE_ALL:	Command_MINIMIZE_ALL();break;	/* ���ׂčŏ��� */	//	Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	case F_REDRAW:			Command_REDRAW();break;			/* �ĕ`�� */
	case F_WIN_OUTPUT:		Command_WIN_OUTPUT();break;		//�A�E�g�v�b�g�E�B���h�E�\��
	case F_TRACEOUT:		Command_TRACEOUT((const wchar_t*)lparam1, (int)lparam2);break;		//�}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\�� maru 2006.04.26
	case F_TOPMOST:			Command_WINTOPMOST( lparam1 );break;	//��Ɏ�O�ɕ\�� Moca
	case F_WINLIST:			Command_WINLIST( nCommandFrom );break;		/* �E�B���h�E�ꗗ�|�b�v�A�b�v�\������ */	// 2006.03.23 fon // 2006.05.19 genta �����ǉ�
	case F_GROUPCLOSE:		Command_GROUPCLOSE();break;		/* �O���[�v����� */		// 2007.06.20 ryoji �ǉ�
	case F_NEXTGROUP:		Command_NEXTGROUP();break;		/* ���̃O���[�v */			// 2007.06.20 ryoji �ǉ�
	case F_PREVGROUP:		Command_PREVGROUP();break;		/* �O�̃O���[�v */			// 2007.06.20 ryoji �ǉ�
	case F_TAB_MOVERIGHT:	Command_TAB_MOVERIGHT();break;	/* �^�u���E�Ɉړ� */		// 2007.06.20 ryoji �ǉ�
	case F_TAB_MOVELEFT:	Command_TAB_MOVELEFT();break;	/* �^�u�����Ɉړ� */		// 2007.06.20 ryoji �ǉ�
	case F_TAB_SEPARATE:	Command_TAB_SEPARATE();break;	/* �V�K�O���[�v */			// 2007.06.20 ryoji �ǉ�
	case F_TAB_JOINTNEXT:	Command_TAB_JOINTNEXT();break;	/* ���̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji �ǉ�
	case F_TAB_JOINTPREV:	Command_TAB_JOINTPREV();break;	/* �O�̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji �ǉ�
	case F_TAB_CLOSEOTHER:	Command_TAB_CLOSEOTHER();break;	/* ���̃^�u�ȊO����� */	// 2008.11.22 syat �ǉ�
	case F_TAB_CLOSELEFT:	Command_TAB_CLOSELEFT();break;	/* �������ׂĕ��� */		// 2008.11.22 syat �ǉ�
	case F_TAB_CLOSERIGHT:	Command_TAB_CLOSERIGHT();break;	/* �E�����ׂĕ��� */		// 2008.11.22 syat �ǉ�

	/* �x�� */
	case F_HOKAN:			Command_HOKAN();break;			//���͕⊮
	case F_HELP_CONTENTS:	Command_HELP_CONTENTS();break;	/* �w���v�ڎ� */				//Nov. 25, 2000 JEPRO �ǉ�
	case F_HELP_SEARCH:		Command_HELP_SEARCH();break;	/* �w���v�g�L�[���[�h���� */	//Nov. 25, 2000 JEPRO �ǉ�
	case F_TOGGLE_KEY_SEARCH:	Command_ToggleKeySearch();break;	/* �L�����b�g�ʒu�̒P���������������@�\ON-OFF */	// 2006.03.24 fon
	case F_MENU_ALLFUNC:									/* �R�}���h�ꗗ */
		/* �ċA�����΍� */
		if( NULL != GetOpeBlk() ){	/* ����u���b�N */
			ClearOpeBlk();
		}
		Command_MENU_ALLFUNC();break;
	case F_EXTHELP1:	Command_EXTHELP1();break;		/* �O���w���v�P */
	case F_EXTHTMLHELP:	/* �O��HTML�w���v */
		//	Jul. 5, 2002 genta
		Command_EXTHTMLHELP( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		break;
	case F_ABOUT:	Command_ABOUT();break;				/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	/* ���̑� */
//	case F_SENDMAIL:	Command_SENDMAIL();break;		/* ���[�����M */

	default:
		//�v���O�C���R�}���h�����s����
		{
			CPlug::Array plugs;
			CJackManager::Instance()->GetUsablePlug( PP_COMMAND, nCommand, &plugs );

			if( plugs.size() > 0 ){
				//�C���^�t�F�[�X�I�u�W�F�N�g����
				CWSHIfObj::List params;
				//�v���O�C���Ăяo��
				( *plugs.begin() )->Invoke( m_pCommanderView, params );

				/* �t�H�[�J�X�ړ����̍ĕ`�� */
				m_pCommanderView->RedrawAll();
				break;
			}
		}

	}

	/* �A���h�D�o�b�t�@�̏��� */
	if( NULL != GetOpeBlk() ){
		if( 0 < GetOpeBlk()->GetNum() ){	/* ����̐���Ԃ� */
			/* ����̒ǉ� */
			GetDocument()->m_cDocEditor.m_cOpeBuf.AppendOpeBlk( GetOpeBlk() );

			if( GetDocument()->m_cDocEditor.m_cOpeBuf.GetCurrentPointer() == 1 )	// �SUndo��Ԃ���̕ύX���H	// 2009.03.26 ryoji
				m_pCommanderView->Call_OnPaint( PAINT_LINENUMBER, false );	// ���y�C���̍s�ԍ��i�ύX�s�j�\�����X�V �� �ύX�s�݂̂̕\���X�V�ōς܂��Ă���ꍇ�����邽��

			if( !GetEditWindow()->UpdateTextWrap() )	// �܂�Ԃ����@�֘A�̍X�V	// 2008.06.10 ryoji
				GetEditWindow()->RedrawAllViews( m_pCommanderView );	//	���̃y�C���̕\�����X�V
		}
		else{
			delete GetOpeBlk();
		}
		SetOpeBlk(NULL);
	}

	return bRet;
}



/////////////////////////////////// �ȉ��̓R�}���h�Q (Oct. 17, 2000 jepro note) ///////////////////////////////////////////

/*! �J�[�\����ړ� */
int CViewCommander::Command_UP( bool bSelect, bool bRepeat, int lines )
{
	//	From Here Oct. 24, 2001 genta
	if( lines != 0 ){
		GetCaret().Cursor_UPDOWN( CLayoutInt(lines), FALSE );
		return 1;
	}
	//	To Here Oct. 24, 2001 genta


	int		nRepeat = 0;

	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	if( !GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScroll_Smooth ){
		CLayoutInt i;
		if( !bRepeat ){
			i = CLayoutInt(-1);
		}else{
			i = -1 * GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum;	/* �L�[���s�[�g���̃X�N���[���s�� */
		}
		GetCaret().Cursor_UPDOWN( i, bSelect );
		nRepeat = -1 * (Int)i;
	}
	else{
		++nRepeat;
		if( GetCaret().Cursor_UPDOWN( CLayoutInt(-1), bSelect )!=0 && bRepeat ){
			for( int i = 0; i < GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum - 1; ++i ){		/* �L�[���s�[�g���̃X�N���[���s�� */
				::UpdateWindow( m_pCommanderView->GetHwnd() );	//	YAZAKI
				GetCaret().Cursor_UPDOWN( CLayoutInt(-1), bSelect );
				++nRepeat;
			}
		}
	}
	return nRepeat;
}




/* �J�[�\�����ړ� */
int CViewCommander::Command_DOWN( bool bSelect, bool bRepeat )
{
	int		nRepeat;
	nRepeat = 0;
	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	if( !GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScroll_Smooth ){
		CLayoutInt i;
		if( !bRepeat ){
			i = CLayoutInt(1);
		}else{
			i = GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum;	/* �L�[���s�[�g���̃X�N���[���s�� */
		}
		GetCaret().Cursor_UPDOWN( i, bSelect );
		nRepeat = (Int)i;
	}else{
		++nRepeat;
		if( GetCaret().Cursor_UPDOWN(CLayoutInt(1),bSelect)!=0 && bRepeat ){
			for( int i = 0; i < GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum - 1; ++i ){	/* �L�[���s�[�g���̃X�N���[���s�� */
				//	�����ōĕ`��B
				::UpdateWindow( m_pCommanderView->GetHwnd() );	//	YAZAKI
				GetCaret().Cursor_UPDOWN( CLayoutInt(1), bSelect );
				++nRepeat;
			}
		}
	}
	return nRepeat;
}




/*! @brief �J�[�\�����ړ�

	@date 2004.03.28 Moca EOF�����̍s�ȍ~�̓r���ɃJ�[�\��������Ɨ�����o�O�C���D
			pcLayout == NULL���L�����b�g�ʒu���s���ȊO�̏ꍇ��
			2��if�̂ǂ���ɂ����Ă͂܂�Ȃ����C���̂��Ƃ�MoveCursor�ɂēK����
			�ʒu�Ɉړ���������D
*/
int CViewCommander::Command_LEFT( bool bSelect, bool bRepeat )
{
	int		nRepCount;
	int		nRepeat;
	int		nRes;
	if( bRepeat ){
		nRepeat = 2;
	}else{
		nRepeat = 1;
	}
	for( nRepCount = 0; nRepCount < nRepeat; ++nRepCount ){
		CLayoutPoint ptPos(CLayoutInt(0), GetCaret().GetCaretLayoutPos().GetY2());
		const CLayout*	pcLayout;
		if( bSelect ){
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̃J�[�\���ʒu����I�����J�n���� */
				m_pCommanderView->GetSelectionInfo().BeginSelectArea();
			}
		}else{
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ��`�͈͑I�𒆂� */
				if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
					/* 2�_��Ίp�Ƃ����`�����߂� */
					CLayoutRange rcSel;
					TwoPointToRange(
						&rcSel,
						GetSelect().GetFrom(),	// �͈͑I���J�n
						GetSelect().GetTo()		// �͈͑I���I��
					);
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
					/* �J�[�\����I���J�n�ʒu�Ɉړ� */
					GetCaret().MoveCursor( rcSel.GetFrom(), TRUE );
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
				else{
					ptPos = GetSelect().GetFrom();
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
					/* �J�[�\����I���J�n�ʒu�Ɉړ� */
					GetCaret().MoveCursor( ptPos, TRUE );
					GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
				}
				nRes = 1;
				goto end_of_func;
			}
		}
		/* ���ݍs�̃f�[�^���擾 */
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
		/* �J�[�\�������[�ɂ��� */
		if( GetCaret().GetCaretLayoutPos().GetX2() == (pcLayout ? pcLayout->GetIndent() : CLayoutInt(0))){
			if( GetCaret().GetCaretLayoutPos().GetY2() > 0 ){
				pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() - CLayoutInt(1) );
				CMemoryIterator it( pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace() );
				while( !it.end() ){
					it.scanNext();
					if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
						ptPos.x += it.getColumnDelta();
						break;
					}
					it.addDelta();
				}
				ptPos.x += it.getColumn() - it.getColumnDelta();
				ptPos.y --;
			}else{
				nRes = 0;
				goto end_of_func;
			}
		}
		//  2004.03.28 Moca EOF�����̍s�ȍ~�̓r���ɃJ�[�\��������Ɨ�����o�O�C��
		else if( pcLayout ){
			CMemoryIterator it( pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				if ( it.getColumn() + it.getColumnDelta() > GetCaret().GetCaretLayoutPos().GetX2() - 1 ){
					ptPos.x += it.getColumnDelta();
					break;
				}
				it.addDelta();
			}
			ptPos.x += it.getColumn() - it.getColumnDelta();
			//	Oct. 18, 2002 YAZAKI
			if( it.getIndex() >= pcLayout->GetLengthWithEOL() ){
				ptPos.x = GetCaret().GetCaretLayoutPos().GetX2() - CLayoutInt(1);
			}
		}
		GetCaret().MoveCursor( ptPos, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/*	���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX�D
			
				2004.04.02 Moca 
				�L�����b�g�ʒu���s���������ꍇ��MoveCursor�̈ړ����ʂ�
				�����ŗ^�������W�Ƃ͈قȂ邱�Ƃ����邽�߁C
				ptPos�̑���Ɏ��ۂ̈ړ����ʂ��g���悤�ɁD
			*/
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		}
		nRes = 1;
		goto end_of_func;
end_of_func:;
	}
	return nRes;
}




/* �J�[�\���E�ړ� */
void CViewCommander::Command_RIGHT( bool bSelect, bool bIgnoreCurrentSelection, bool bRepeat )
{
	int nRepeat; //��������
	if( bRepeat ){
		nRepeat = 2;
	}else{
		nRepeat = 1;
	}
	for( int nRepCount = 0; nRepCount < nRepeat; ++nRepCount ){
		CLayoutPoint ptPos;
		ptPos.y = GetCaret().GetCaretLayoutPos().GetY2();
		const CLayout*	pcLayout;

		// 2003.06.28 Moca [EOF]�݂̂̍s�ɃJ�[�\��������Ƃ��ɉE�������Ă��I���������ł��Ȃ�����
		// �Ή����邽�߁A���ݍs�̃f�[�^���擾���ړ�
		if( !bIgnoreCurrentSelection ){
			if( bSelect ){
				if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
					/* ���݂̃J�[�\���ʒu����I�����J�n���� */
					m_pCommanderView->GetSelectionInfo().BeginSelectArea();
				}
			}
			else{
				if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
					/* ��`�͈͑I�𒆂� */
					if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
						/* 2�_��Ίp�Ƃ����`�����߂� */
						CLayoutRange rcSel;
						TwoPointToRange(
							&rcSel,
							GetSelect().GetFrom(),	// �͈͑I���J�n
							GetSelect().GetTo()		// �͈͑I���I��
						);
						
						/* ���݂̑I��͈͂��I����Ԃɖ߂� */
						m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );

						/* �J�[�\����I���I���ʒu�Ɉړ� */
						GetCaret().MoveCursor( rcSel.GetFrom(), TRUE );
						GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
					}
					else{
						ptPos = GetSelect().GetTo();

						/* ���݂̑I��͈͂��I����Ԃɖ߂� */
						m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
						if( ptPos.y >= GetDocument()->m_cLayoutMgr.GetLineCount() ){
							/* �t�@�C���̍Ō�Ɉړ� */
							Command_GOFILEEND(FALSE);
						}
						else{
							/* �J�[�\����I���I���ʒu�Ɉړ� */
							GetCaret().MoveCursor( ptPos, TRUE );
							GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
						}
					}
					goto end_of_func;
				}
			}
		}
//		2003.06.28 Moca [EOF]�݂̂̍s�ɃJ�[�\��������Ƃ��ɉE�������Ă��I���������ł��Ȃ����ɑΉ�

		/* ���ݍs�̃f�[�^���擾 */
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
		//	2004.04.02 EOF�ȍ~�ɃJ�[�\�����������Ƃ��ɉE�������Ă������N���Ȃ������̂��AEOF�Ɉړ�����悤��
		if( pcLayout )
		{
			int nIndex = 0;
			CMemoryIterator it( pcLayout, GetDocument()->m_cLayoutMgr.GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				if ( it.getColumn() > GetCaret().GetCaretLayoutPos().GetX2() ){
					break;
				}
				if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
					nIndex += it.getIndexDelta();
					break;
				}
				it.addDelta();
			}
			ptPos.x = it.getColumn(); //�V�����L�����b�g�ʒu
			nIndex += it.getIndex();
			if( nIndex >= pcLayout->GetLengthWithEOL() ){
				/* �t���[�J�[�\�����[�h�� */
				if( (
					GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode
				 || m_pCommanderView->GetSelectionInfo().IsTextSelected() && m_pCommanderView->GetSelectionInfo().IsBoxSelecting()	/* ��`�͈͑I�� */
					)
				 &&
					/* ���s�ŏI����Ă��邩 */
					( EOL_NONE != pcLayout->GetLayoutEol() )
				){
					/*-- �t���[�J�[�\�����[�h�̏ꍇ --*/
					if( ptPos.x <= GetCaret().GetCaretLayoutPos().GetX2() ){
						/* �ŏI�s�� */
						if( GetCaret().GetCaretLayoutPos().GetY2() + 1 == GetDocument()->m_cLayoutMgr.GetLineCount() ){
							/* ���s�ŏI����Ă��邩 */
							if( EOL_NONE != pcLayout->GetLayoutEol().GetType() ){
								ptPos.x = GetCaret().GetCaretLayoutPos().GetX2() + 1;
							}else{
								ptPos.x = GetCaret().GetCaretLayoutPos().GetX2();
							}
						}else{
							ptPos.x = GetCaret().GetCaretLayoutPos().GetX2() + 1;
						}
					}else{
						ptPos.x = ptPos.x;
					}
				}
				else{
					/*-- �t���[�J�[�\�����[�h�ł͂Ȃ��ꍇ --*/
					/* �ŏI�s�� */
					if( GetCaret().GetCaretLayoutPos().GetY2() + 1 == GetDocument()->m_cLayoutMgr.GetLineCount() ){
						/* ���s�ŏI����Ă��邩 */
						if( EOL_NONE != pcLayout->GetLayoutEol().GetType() ){
							ptPos.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : CLayoutInt(0);
							++ptPos.y;
						}
						else{
						}
					}
					else{
						if( ptPos.x <= GetCaret().GetCaretLayoutPos().GetX2()
							&& EOL_NONE == pcLayout->GetLayoutEol()
							&& pcLayout->GetNextLayout()
						){
							nRepeat++;	// ���C�A�E�g�s�̉E�[�͎��̍s�̐擪�Ƙ_���I�ɓ����Ȃ̂ł���ɉE��	// 2007.02.19 ryoji
						}
						ptPos.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : CLayoutInt(0);
						++ptPos.y;
					}
				}
				//	�L�����b�g�ʒu���܂�Ԃ��ʒu���E���������ꍇ�̏���
				//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
				if( ptPos.x >= GetDocument()->m_cLayoutMgr.GetMaxLineKetas() ){
					if( GetCaret().GetCaretLayoutPos().GetX2() >= GetDocument()->m_cLayoutMgr.GetMaxLineKetas()
						&& pcLayout->GetNextLayout()
					){
						nRepeat++;	// ���C�A�E�g�s�̉E�[�͎��̍s�̐擪�Ƙ_���I�ɓ����Ȃ̂ł���ɉE��	// 2007.02.19 ryoji
					}
					ptPos.x = pcLayout->GetNextLayout() ? pcLayout->GetNextLayout()->GetIndent() : CLayoutInt(0);
					++ptPos.y;
				}
			}
		}else{
			// pcLayout��NULL�̏ꍇ��ptPos.x=0�ɒ���
			ptPos.x = CLayoutInt(0);
		}
		GetCaret().MoveCursor( ptPos, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( GetCaret().GetCaretLayoutPos() );
		}

end_of_func:;
	}
	return;
}




//	From Here Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL/UP/DOWN��PAGE/DOWN/UP)���邽�߂Ɉȉ����R�����g�A�E�g
///* �X�N���[���A�b�v */
//void CViewCommander::Command_ROLLUP( bool bSelect )
//{
//	GetCaret().Cursor_UPDOWN( ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
//	return;
//}
//
//
//
//
///* �X�N���[���_�E�� */
//void CViewCommander::Command_ROLLDOWN( bool bSelect )
//{
//	GetCaret().Cursor_UPDOWN( - ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
//	return;
//}
//	To Here Oct. 6, 2000




/* ���y�[�W�A�b�v */	//Oct. 6, 2000 JEPRO added (���͏]���̃X�N���[���_�E�����̂���)
void CViewCommander::Command_HalfPageUp( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( - ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
	return;
}




/* ���y�[�W�_�E�� */	//Oct. 6, 2000 JEPRO added (���͏]���̃X�N���[���A�b�v���̂���)
void CViewCommander::Command_HalfPageDown( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 ), bSelect );
	return;
}




/*! �P�y�[�W�A�b�v

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�A�b�v�ɓ���ύX
*/	//Oct. 10, 2000 JEPRO added
void CViewCommander::Command_1PageUp( bool bSelect )
{
//	GetCaret().Cursor_UPDOWN( - m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�A�b�v
	if(m_pCommanderView->GetTextArea().GetViewTopLine()>=m_pCommanderView->GetTextArea().m_nViewRowNum-1){
		m_pCommanderView->SetDrawSwitch(false);
		CLayoutInt nViewTopLine=GetCaret().GetCaretLayoutPos().GetY2()-m_pCommanderView->GetTextArea().GetViewTopLine();
		GetCaret().Cursor_UPDOWN( -m_pCommanderView->GetTextArea().m_nViewRowNum+1, bSelect );
		//	Sep. 11, 2004 genta �����X�N���[�������̂���
		//	m_pCommanderView->RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( GetCaret().GetCaretLayoutPos().GetY2()-nViewTopLine ));
		m_pCommanderView->SetDrawSwitch(true);
		m_pCommanderView->RedrawAll();
		
	}else{
		GetCaret().Cursor_UPDOWN( -m_pCommanderView->GetTextArea().m_nViewRowNum+1, bSelect );
	}
	return;
}




/*!	�P�y�[�W�_�E��

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�_�E���ɓ���ύX
*/
void CViewCommander::Command_1PageDown( bool bSelect )
{
//	GetCaret().Cursor_UPDOWN( m_pCommanderView->GetTextArea().m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�_�E��
	if(m_pCommanderView->GetTextArea().GetViewTopLine()+m_pCommanderView->GetTextArea().m_nViewRowNum <= GetDocument()->m_cLayoutMgr.GetLineCount() ){ //- m_pCommanderView->GetTextArea().m_nViewRowNum){
		m_pCommanderView->SetDrawSwitch(false);
		CLayoutInt nViewTopLine=GetCaret().GetCaretLayoutPos().GetY2()-m_pCommanderView->GetTextArea().GetViewTopLine();
		GetCaret().Cursor_UPDOWN( m_pCommanderView->GetTextArea().m_nViewRowNum-1, bSelect );
		//	Sep. 11, 2004 genta �����X�N���[�������̂���
		//	m_pCommanderView->RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		m_pCommanderView->SyncScrollV( m_pCommanderView->ScrollAtV( GetCaret().GetCaretLayoutPos().GetY2()-nViewTopLine ));
		m_pCommanderView->SetDrawSwitch(true);
		m_pCommanderView->RedrawAll();
	}else{
		GetCaret().Cursor_UPDOWN( m_pCommanderView->GetTextArea().m_nViewRowNum , bSelect );
		Command_DOWN( bSelect, TRUE );
	}

	return;
}




/* �J�[�\����ړ�(�Q�s�Â�) */
void CViewCommander::Command_UP2( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( CLayoutInt(-2), bSelect );
	return;
}




/* �J�[�\�����ړ�(�Q�s�Â�) */
void CViewCommander::Command_DOWN2( bool bSelect )
{
	GetCaret().Cursor_UPDOWN( CLayoutInt(2), bSelect );
	return;
}




/*! @brief �s���Ɉړ�

	@date Oct. 29, 2001 genta �}�N���p�@�\�g��(�p�����[�^�ǉ�) + goto�r��
	@date May. 15, 2002 oak   ���s�P�ʈړ�
	@date Oct.  7, 2002 YAZAKI �璷�Ȉ��� bLineTopOnly ���폜
	@date Jun. 18, 2007 maru �s������ɑS�p�󔒂̃C���f���g�ݒ���l������
*/
void CViewCommander::Command_GOLINETOP(
	bool	bSelect,	//!< [in] �I���̗L���Btrue: �I�����Ȃ���ړ��Bfalse: �I�����Ȃ��ňړ��B
	int		lparam		/*!< [in] �}�N������g�p����g���t���O
								  @li 0: �L�[����Ɠ���(default)
								  @li 1: �J�[�\���ʒu�Ɋ֌W�Ȃ��s���Ɉړ��B
								  @li 4: �I�����Ĉړ�(������)
								  @li 8: ���s�P�ʂŐ擪�Ɉړ�(������)
						*/
)
{
	using namespace WCODE;

	// lparam�̉���
	bool	bLineTopOnly = ((lparam & 1) != 0);
	if( lparam & 4 ){
		bSelect = true;
	}


	CLayoutPoint ptCaretPos;
	if ( lparam & 8 ){
		/* ���s�P�ʎw��̏ꍇ�́A�����s���ʒu����ړI�_���ʒu�����߂� */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0,GetCaret().GetCaretLogicPos().y),
			&ptCaretPos
		);
	}
	else{
		const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
		ptCaretPos.x = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);
		ptCaretPos.y = GetCaret().GetCaretLayoutPos().GetY2();
	}
	if( !bLineTopOnly ){
		/* �ړI�s�̃f�[�^���擾 */
		/* ���s�P�ʎw��ŁA�擪����󔒂�1�܂�Ԃ��s�ȏ㑱���Ă���ꍇ�͎��̍s�f�[�^���擾 */
		CLayoutInt nPosY_Layout;
		CLogicInt  nPosX_Logic;

		nPosY_Layout = ptCaretPos.y - 1;
		const CLayout*	pcLayout;
		bool			bZenSpace = GetDocument()->m_cDocType.GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
		
		CLogicInt		nLineLen;
		do {
			++nPosY_Layout;
			const wchar_t*	pLine = GetDocument()->m_cLayoutMgr.GetLineStr( nPosY_Layout, &nLineLen, &pcLayout );
			if( !pLine ){
				return;
			}
			for( nPosX_Logic = 0; nPosX_Logic < nLineLen; ++nPosX_Logic ){
				if(WCODE::IsIndentChar(pLine[nPosX_Logic],bZenSpace!=0))continue;
				
				if(WCODE::IsLineDelimiter(pLine[nPosX_Logic]) ){
					nPosX_Logic = 0;	// �󔒂܂��̓^�u����щ��s�����̍s������
				}
				break;
			}
		}
		while (( lparam & 8 ) && (nPosX_Logic >= nLineLen) && (GetDocument()->m_cLayoutMgr.GetLineCount() - 1 > nPosY_Layout) );
		
		if( nPosX_Logic >= nLineLen ){
			/* �܂�Ԃ��P�ʂ̍s����T���ĕ����s���܂œ��B����
			�܂��́A�ŏI�s�̂��߉��s�R�[�h�ɑ��������ɍs���ɓ��B���� */
			nPosX_Logic = 0;
		}
		
		if(0 == nPosX_Logic) nPosY_Layout = ptCaretPos.y;	/* �����s�̈ړ��Ȃ� */
		
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		CLayoutInt nPosX_Layout = m_pCommanderView->LineIndexToColmn( pcLayout, nPosX_Logic );
		CLayoutPoint ptPos(nPosX_Layout, nPosY_Layout);
		if( GetCaret().GetCaretLayoutPos() != ptPos ){
			ptCaretPos = ptPos;
		}
	}

	//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	m_pCommanderView->MoveCursorSelecting( ptCaretPos, bSelect );
}




// �s���Ɉړ�(�܂�Ԃ��P��)
void CViewCommander::Command_GOLINEEND( bool bSelect, int bIgnoreCurrentSelection )
{
	if( !bIgnoreCurrentSelection ){
		if( bSelect ){
			if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̃J�[�\���ʒu����I�����J�n���� */
				m_pCommanderView->GetSelectionInfo().BeginSelectArea();
			}
		}else{
			if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
			}
		}
	}

	// ���ݍs�̃f�[�^����A���̃��C�A�E�g�����擾
	CLayoutPoint	nPosXY = GetCaret().GetCaretLayoutPos();
	nPosXY.x = CLayoutInt(0);
	const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nPosXY.y );
	if(pcLayout)
		nPosXY.x = pcLayout->GetIndent() + pcLayout->CalcLayoutWidth(GetDocument()->m_cLayoutMgr);

	// �L�����b�g�ړ�
	GetCaret().MoveCursor( nPosXY, true );
	GetCaret().m_nCaretPosX_Prev = nPosXY.x;
	if( bSelect ){
		// ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX
		m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( nPosXY );
	}
}




/* �t�@�C���̐擪�Ɉړ� */
void CViewCommander::Command_GOFILETOP( bool bSelect )
{
	/* �擪�փJ�[�\�����ړ� */
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();

	//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	CLayoutPoint pt(
		!m_pCommanderView->GetSelectionInfo().IsBoxSelecting()? CLayoutInt(0): GetCaret().GetCaretLayoutPos().GetX2(),
		CLayoutInt(0)
	);
	m_pCommanderView->MoveCursorSelecting( pt, bSelect );	//	�ʏ�́A(0, 0)�ֈړ��B�{�b�N�X�I�𒆂́A(GetCaret().GetCaretLayoutPos().GetX2(), 0)�ֈړ�
}




/* �t�@�C���̍Ō�Ɉړ� */
void CViewCommander::Command_GOFILEEND( bool bSelect )
{
// 2001.12.13 hor BOX�I�𒆂Ƀt�@�C���̍Ō�ɃW�����v�����[EOF]�̍s�����]�����܂܂ɂȂ�̏C��
	if( !bSelect && m_pCommanderView->GetSelectionInfo().IsTextSelected() ) m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );	// 2001.12.21 hor Add
	m_pCommanderView->AddCurrentLineToHistory();
	GetCaret().Cursor_UPDOWN( GetDocument()->m_cLayoutMgr.GetLineCount() , bSelect );
	Command_DOWN( bSelect, TRUE );
	if ( !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){							// 2002/04/18 YAZAKI
		/*	2004.04.19 fotomo
			���s�̂Ȃ��ŏI�s�őI�����Ȃ��當�����ֈړ������ꍇ��
			�I��͈͂��������Ȃ��ꍇ��������ɑΉ�
		*/
		Command_GOLINEEND( bSelect, FALSE );				// 2001.12.21 hor Add
	}
	GetCaret().MoveCursor( GetCaret().GetCaretLayoutPos(), TRUE );	// 2001.12.21 hor Add
	// 2002.02.16 hor ��`�I�𒆂��������O�̃J�[�\���ʒu�����Z�b�g
	if( !(m_pCommanderView->GetSelectionInfo().IsTextSelected() && m_pCommanderView->GetSelectionInfo().IsBoxSelecting()) ) GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	// �I��͈͏�񃁃b�Z�[�W��\������	// 2009.05.06 ryoji �ǉ�
	if( bSelect ){
		m_pCommanderView->GetSelectionInfo().PrintSelectionInfoMsg();
	}
}




/* �P��̍��[�Ɉړ� */
void CViewCommander::Command_WORDLEFT( bool bSelect )
{
	CLogicInt		nIdx;
	if( bSelect ){
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			m_pCommanderView->GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){		/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}

	const CLayout* pcLayout;
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		bool bIsFreeCursorModeOld = GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* �t���[�J�[�\�����[�h�� */
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* �J�[�\�����ړ� */
		Command_LEFT( bSelect, false );
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* �t���[�J�[�\�����[�h�� */
		return;
	}

	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

	/* ���݈ʒu�̍��̒P��̐擪�ʒu�𒲂ׂ� */
	CLayoutPoint ptLayoutNew;
	int nResult=GetDocument()->m_cLayoutMgr.PrevWord(
		GetCaret().GetCaretLayoutPos().GetY2(),
		nIdx,
		&ptLayoutNew,
		GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if( nResult ){
		/* �s���ς���� */
		if( ptLayoutNew.y != GetCaret().GetCaretLayoutPos().GetY2() ){
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptLayoutNew.GetY2() );
			if( NULL == pcLayout ){
				return;
			}
		}

		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColmn( pcLayout, ptLayoutNew.x );
		*/

		/* �J�[�\���ړ� */
		GetCaret().MoveCursor( ptLayoutNew, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptLayoutNew );
		}
	}else{
		bool bIsFreeCursorModeOld = GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* �t���[�J�[�\�����[�h�� */
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* �J�[�\�����ړ� */
		Command_LEFT( bSelect, false );
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* �t���[�J�[�\�����[�h�� */
	}
	return;
}




/* �P��̉E�[�Ɉړ� */
void CViewCommander::Command_WORDRIGHT( bool bSelect )
{
	CLogicInt	nIdx;
	CLayoutInt	nCurLine;
	if( bSelect ){
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			m_pCommanderView->GetSelectionInfo().BeginSelectArea();
		}
	}else{
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}
	bool	bTryAgain = false;
try_again:;
	nCurLine = GetCaret().GetCaretLayoutPos().GetY2();
	const CLayout* pcLayout;
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nCurLine );
	if( NULL == pcLayout ){
		return;
	}
	if( bTryAgain ){
		const wchar_t*	pLine = pcLayout->GetPtr();
		if( pLine[0] != L' ' && pLine[0] != WCODE::TAB ){
			return;
		}
	}
	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

	/* ���݈ʒu�̉E�̒P��̐擪�ʒu�𒲂ׂ� */
	CLayoutPoint ptLayoutNew;
	int nResult = GetDocument()->m_cLayoutMgr.NextWord(
		nCurLine,
		nIdx,
		&ptLayoutNew,
		GetDllShareData().m_Common.m_sGeneral.m_bStopsBothEndsWhenSearchWord
	);
	if( nResult ){
		/* �s���ς���� */
		if( ptLayoutNew.y != nCurLine ){
			pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( ptLayoutNew.GetY2() );
			if( NULL == pcLayout ){
				return;
			}
		}
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		ptLayoutNew.x = m_pCommanderView->LineIndexToColmn( pcLayout, ptLayoutNew.x );
		*/
		// �J�[�\���ړ�
		GetCaret().MoveCursor( ptLayoutNew, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( bSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( ptLayoutNew );
		}
	}
	else{
		bool	bIsFreeCursorModeOld = GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode;	/* �t���[�J�[�\�����[�h�� */
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = false;
		/* �J�[�\���E�ړ� */
		Command_RIGHT( bSelect, false, false );
		GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* �t���[�J�[�\�����[�h�� */
		if( !bTryAgain ){
			bTryAgain = true;
			goto try_again;
		}
	}
	return;
}



/**	�I��͈͂��N���b�v�{�[�h�ɃR�s�[
 	@param bIgnoreLockAndDisable [in] �I��͈͂��������邩�H
 	@param bAddCRLFWhenCopy [in] �܂�Ԃ��ʒu�ɉ��s�R�[�h��}�����邩�H
 	@param neweol [in] �R�s�[����Ƃ���EOL�B

	@date 2007.11.18 ryoji �u�I���Ȃ��ŃR�s�[���\�ɂ���v�I�v�V���������ǉ�
*/
void CViewCommander::Command_COPY(
	bool		bIgnoreLockAndDisable,	//!< [in] �I��͈͂��������邩�H
	bool		bAddCRLFWhenCopy,		//!< [in] �܂�Ԃ��ʒu�ɉ��s�R�[�h��}�����邩�H
	enumEOLType	neweol					//!< [in] �R�s�[����Ƃ���EOL�B
)
{
	CNativeW	cmemBuf;
	bool		bBeginBoxSelect = false;

	/* �N���b�v�{�[�h�ɓ����ׂ��e�L�X�g�f�[�^���AcmemBuf�Ɋi�[���� */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��I�����́A�J�[�\���s���R�s�[���� */
		if( !GetDllShareData().m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// �������Ȃ��i�����炳�Ȃ��j
		}
		m_pCommanderView->CopyCurLine(
			bAddCRLFWhenCopy,
			neweol,
			GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste
		);
	}
	else{
		/* �e�L�X�g���I������Ă���Ƃ��́A�I��͈͂̃f�[�^���擾 */

		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			bBeginBoxSelect = TRUE;
		}
		/* �I��͈͂̃f�[�^���擾 */
		/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
		if( !m_pCommanderView->GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, bAddCRLFWhenCopy, neweol ) ){
			ErrorBeep();
			return;
		}

		/* �N���b�v�{�[�h�Ƀf�[�^cmemBuf�̓��e��ݒ� */
		if( !m_pCommanderView->MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect, FALSE ) ){
			ErrorBeep();
			return;
		}
	}

	/* �I��͈͂̌�Еt�� */
	if( !bIgnoreLockAndDisable ){
		/* �I����Ԃ̃��b�N */
		if( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = FALSE;
		}
	}
	if( GetDllShareData().m_Common.m_sEdit.m_bCopyAndDisablSelection ){	/* �R�s�[������I������ */
		/* �e�L�X�g���I������Ă��邩 */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}
	return;
}




/** �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)

	@date 2007.11.18 ryoji �u�I���Ȃ��ŃR�s�[���\�ɂ���v�I�v�V���������ǉ�
*/
void CViewCommander::Command_CUT( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	CNativeW	cmemBuf;
	bool	bBeginBoxSelect;
	/* �͈͑I��������Ă��Ȃ� */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��I�����́A�J�[�\���s��؂��� */
		if( !GetDllShareData().m_Common.m_sEdit.m_bEnableNoSelectCopy ){	// 2007.11.18 ryoji
			return;	// �������Ȃ��i�����炳�Ȃ��j
		}
		//�s�؂���(�܂�Ԃ��P��)
		Command_CUT_LINE();
		return;
	}
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		bBeginBoxSelect = true;
	}else{
		bBeginBoxSelect = false;
	}

	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
	if( !m_pCommanderView->GetSelectedData( &cmemBuf, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}
	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	if( !m_pCommanderView->MySetClipboardData( cmemBuf.GetStringPtr(), cmemBuf.GetStringLength(), bBeginBoxSelect ) ){
		ErrorBeep();
		return;
	}

	/* �J�[�\���ʒu�܂��͑I���G���A���폜 */
	m_pCommanderView->DeleteData( TRUE );
	return;
}




//�J�[�\���ʒu�܂��͑I���G���A���폜
void CViewCommander::Command_DELETE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){		/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		// 2008.08.03 nasukoji	�I��͈͂Ȃ���DELETE�����s�����ꍇ�A�J�[�\���ʒu�܂Ŕ��p�X�y�[�X��}����������s���폜���Ď��s�ƘA������
		if( GetDocument()->m_cLayoutMgr.GetLineCount() > GetCaret().GetCaretLayoutPos().GetY2() ){
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
			if( pcLayout ){
				CLayoutInt nLineLen;
				CLogicInt nIndex;
				nIndex = m_pCommanderView->LineColmnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
				if( nLineLen != 0 ){	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					if( EOL_NONE != pcLayout->GetLayoutEol().GetType() ){	// �s�I�[�͉��s�R�[�h��?
						Command_INSTEXT( TRUE, L"", CLogicInt(0), FALSE );	// �J�[�\���ʒu�܂Ŕ��p�X�y�[�X�}��
					}else{	// �s�I�[���܂�Ԃ�
						// �܂�Ԃ��s���ł̓X�y�[�X�}����A���̕������폜����	// 2009.02.19 ryoji

						// �t���[�J�[�\�����̐܂�Ԃ��z���ʒu�ł̍폜�͂ǂ�����̂��Ó����悭�킩��Ȃ���
						// ��t���[�J�[�\�����i���傤�ǃJ�[�\�����܂�Ԃ��ʒu�ɂ���j�ɂ͎��̍s�̐擪�������폜������

						if( nLineLen < GetCaret().GetCaretLayoutPos().GetX2() ){	// �܂�Ԃ��s���ƃJ�[�\���̊ԂɌ��Ԃ�����
							Command_INSTEXT( TRUE, L"", CLogicInt(0), FALSE );	// �J�[�\���ʒu�܂Ŕ��p�X�y�[�X�}��
							pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
							nIndex = m_pCommanderView->LineColmnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
						}
						if( nLineLen != 0 ){	// �i�X�y�[�X�}������j�܂�Ԃ��s���Ȃ玟�������폜���邽�߂Ɏ��s�̐擪�Ɉړ�����K�v������
							if( pcLayout->GetNextLayout() != NULL ){	// �ŏI�s���ł͂Ȃ�
								CLayoutPoint ptLay;
								CLogicPoint ptLog(pcLayout->GetLogicOffset() + nIndex, pcLayout->GetLogicLineNo());
								GetDocument()->m_cLayoutMgr.LogicToLayout( ptLog, &ptLay );
								GetCaret().MoveCursor( ptLay, TRUE );
								GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
							}
						}
					}
				}
			}
		}
	}
	m_pCommanderView->DeleteData( TRUE );
	return;
}




//�J�[�\���O���폜
void CViewCommander::Command_DELETE_BACK( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	//	May 29, 2004 genta ���ۂɍ폜���ꂽ�������Ȃ��Ƃ��̓t���O�����ĂȂ��悤��
	//GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){				/* �e�L�X�g���I������Ă��邩 */
		m_pCommanderView->DeleteData( TRUE );
	}
	else{
		CLayoutPoint	ptLayoutPos_Old = GetCaret().GetCaretLayoutPos();
		CLogicPoint		ptLogicPos_Old = GetCaret().GetCaretLogicPos();
		BOOL	bBool = Command_LEFT( FALSE, FALSE );
		if( bBool ){
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
			if( pcLayout ){
				CLayoutInt nLineLen;
				CLogicInt nIdx = m_pCommanderView->LineColmnToIndex2( pcLayout, GetCaret().GetCaretLayoutPos().GetX2(), &nLineLen );
				if( nLineLen == 0 ){	// �܂�Ԃ�����s�R�[�h���E�̏ꍇ�ɂ� nLineLen �ɍs�S�̂̕\������������
					// �E����̈ړ��ł͐܂�Ԃ����������͍폜���邪���s�͍폜���Ȃ�
					// ������i���̍s�̍s������j�̈ړ��ł͉��s���폜����
					if( nIdx < pcLayout->GetLengthWithoutEOL() || GetCaret().GetCaretLayoutPos().GetY2() < ptLayoutPos_Old.GetY2() ){
						if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
							/* ����̒ǉ� */
							GetOpeBlk()->AppendOpe(
								new CMoveCaretOpe(
									ptLogicPos_Old,
									GetCaret().GetCaretLogicPos()
								)
							);
						}
						m_pCommanderView->DeleteData( TRUE );
					}
				}
			}
		}
	}
	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta �֐���
}




//�P��̉E�[�܂ō폜
void CViewCommander::Command_WordDeleteToEnd( void )
{
	CMemory	cmemData;

	/* ��`�I����Ԃł͎��s�s�\((��������蔲������)) */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			ErrorBeep();
			return;
		}
	}
	/* �P��̉E�[�Ɉړ� */
	CViewCommander::Command_WORDRIGHT( TRUE );
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}
	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		CMoveCaretOpe*	pcOpe = new CMoveCaretOpe();
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetFrom(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe( pcOpe );
	}
	/* �폜 */
	m_pCommanderView->DeleteData( TRUE );
}




//�P��̍��[�܂ō폜
void CViewCommander::Command_WordDeleteToStart( void )
{
	/* ��`�I����Ԃł͎��s�s�\(��������蔲������) */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			ErrorBeep();
			return;
		}
	}

	// �P��̍��[�Ɉړ�
	CViewCommander::Command_WORDLEFT( TRUE );
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		//$$ �����͖�肪�����炵���ł����B
		//   ������ COpe �����A�Ӗ��t��(EOpeCode�w��)������Ă��Ȃ��̂ŁA
		//   ���߂ĉ��炩�̖��O�t�������Ă����Ȃ��ƁA�Ӑ}���ǂݎ��܂���B
		COpe*	pcOpe = new COpe;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			GetSelect().GetTo(),
			&pcOpe->m_ptCaretPos_PHY_Before
		);
		pcOpe->m_ptCaretPos_PHY_After = pcOpe->m_ptCaretPos_PHY_Before;	// �����̃L�����b�g�ʒu

		// ����̒ǉ�
		GetOpeBlk()->AppendOpe( pcOpe );
	}

	// �폜
	m_pCommanderView->DeleteData( TRUE );
}




//�P��؂���
void CViewCommander::Command_WordCut( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
		Command_CUT();
		return;
	}
	//���݈ʒu�̒P��I��
	Command_SELECTWORD();
	/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
	if ( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		//	�P��I���őI���ł��Ȃ�������A���̕�����I�Ԃ��Ƃɒ���B
		Command_RIGHT( TRUE, FALSE, FALSE );
	}
	Command_CUT();
	return;
}




//�P��폜
void CViewCommander::Command_WordDelete( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* �폜 */
		m_pCommanderView->DeleteData( TRUE );
		return;
	}
	//���݈ʒu�̒P��I��
	Command_SELECTWORD();
	/* �폜 */
	m_pCommanderView->DeleteData( TRUE );
	return;
}




//�s���܂Ő؂���(���s�P��)
void CViewCommander::Command_LineCutToStart( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
		Command_CUT();
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos );
	if( GetCaret().GetCaretLayoutPos() == ptPos ){
		ErrorBeep();
		return;
	}

	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	CLayoutRange sRange(ptPos,GetCaret().GetCaretLayoutPos());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/*�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
	Command_CUT();
}




//�s���܂Ő؂���(���s�P��)
void CViewCommander::Command_LineCutToEnd( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
		Command_CUT();
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	if( EOL_NONE == pCLayout->GetDocLineRef()->GetEol() ){	/* ���s�R�[�h�̎�� */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}
	else{
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL() - pCLayout->GetDocLineRef()->GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if( GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x ){
		ErrorBeep();
		return;
	}

	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	CLayoutRange sRange(GetCaret().GetCaretLayoutPos(),ptPos);
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/*�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
	Command_CUT();
}




//�s���܂ō폜(���s�P��)
void CViewCommander::Command_LineDeleteToStart( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		m_pCommanderView->DeleteData( TRUE );
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, pCLayout->GetLogicLineNo()), &ptPos );
	if( GetCaret().GetCaretLayoutPos() == ptPos ){
		ErrorBeep();
		return;
	}

	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	CLayoutRange sRange(ptPos,GetCaret().GetCaretLayoutPos());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* �I��̈�폜 */
	m_pCommanderView->DeleteData( TRUE );
}




//�s���܂ō폜(���s�P��)
void CViewCommander::Command_LineDeleteToEnd( void )
{
	CLayout*	pCLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		m_pCommanderView->DeleteData( TRUE );
		return;
	}
	pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		ErrorBeep();
		return;
	}

	CLayoutPoint ptPos;

	if( EOL_NONE == pCLayout->GetDocLineRef()->GetEol() ){	/* ���s�R�[�h�̎�� */
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}else{
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(
				pCLayout->GetDocLineRef()->GetLengthWithEOL() - pCLayout->GetDocLineRef()->GetEol().GetLen(),
				pCLayout->GetLogicLineNo()
			),
			&ptPos
		);
	}

	if( GetCaret().GetCaretLayoutPos().GetY2() == ptPos.y && GetCaret().GetCaretLayoutPos().GetX2() >= ptPos.x ){
		ErrorBeep();
		return;
	}

	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	CLayoutRange sRange( GetCaret().GetCaretLayoutPos(), ptPos );
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* �I��̈�폜 */
	m_pCommanderView->DeleteData( TRUE );
}




//�s�؂���(�܂�Ԃ��P��)
void CViewCommander::Command_CUT_LINE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		ErrorBeep();
		return;
	}

	const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().y );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	// 2007.10.04 ryoji �����ȑf��
	m_pCommanderView->CopyCurLine(
		GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy,
		EOL_UNKNOWN,
		GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste
	);
	Command_DELETE_LINE();
	return;
}




/* �s�폜(�܂�Ԃ��P��) */
void CViewCommander::Command_DELETE_LINE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	const CLayout*	pcLayout;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		ErrorBeep();
		return;
	}
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}
	GetSelect().SetFrom(CLayoutPoint(CLayoutInt(0),GetCaret().GetCaretLayoutPos().GetY2()    ));	//�͈͑I���J�n�ʒu
	GetSelect().SetTo  (CLayoutPoint(CLayoutInt(0),GetCaret().GetCaretLayoutPos().GetY2() + 1));	//�͈͑I���I���ʒu

	CLayoutPoint ptCaretPos_OLD = GetCaret().GetCaretLayoutPos();

	Command_DELETE();
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL != pcLayout ){
		// 2003-04-30 �����
		// �s�폜������A�t���[�J�[�\���łȂ��̂ɃJ�[�\���ʒu���s�[���E�ɂȂ�s��Ή�
		// �t���[�J�[�\�����[�h�łȂ��ꍇ�́A�J�[�\���ʒu�𒲐�����
		if( !GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode ) {
			CLogicInt nIndex;

			CLayoutInt tmp;
			nIndex = m_pCommanderView->LineColmnToIndex2( pcLayout, ptCaretPos_OLD.GetX2(), &tmp );
			ptCaretPos_OLD.x=tmp;

			if (ptCaretPos_OLD.x > 0) {
				ptCaretPos_OLD.x--;
			} else {
				ptCaretPos_OLD.x = m_pCommanderView->LineIndexToColmn( pcLayout, nIndex );
			}
		}
		/* ����O�̈ʒu�փJ�[�\�����ړ� */
		GetCaret().MoveCursor( ptCaretPos_OLD, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			/* ����̒ǉ� */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),
					GetCaret().GetCaretLogicPos()
				)
			);
		}
	}
	return;
}




/* ���ׂđI�� */
void CViewCommander::Command_SELECTALL( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}

	/* �擪�փJ�[�\�����ړ� */
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	//	Jul. 29, 2006 genta �I���ʒu�̖����𐳊m�Ɏ擾����
	//	�}�N������擾�����ꍇ�ɐ������͈͂��擾�ł��Ȃ�����
	//int nX, nY;
	CLayoutRange sRange;
	sRange.SetFrom(CLayoutPoint(0,0));
	GetDocument()->m_cLayoutMgr.GetEndLayoutPos(sRange.GetToPointer());
	m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

	/* �I��̈�`�� */
	m_pCommanderView->GetSelectionInfo().DrawSelectArea();
}




/*!	1�s�I��
	@brief �J�[�\���ʒu��1�s�I������
	@param lparam [in] �}�N������g�p����g���t���O�i�g���p�ɗ\��j

	note ���s�P�ʂőI�����s���B

	@date 2007.11.15 nasukoji	�V�K�쐬
*/
void CViewCommander::Command_SELECTLINE( int lparam )
{
	// ���s�P�ʂ�1�s�I������
	Command_GOLINETOP( FALSE, 0x9 );	// �����s���Ɉړ�

	m_pCommanderView->GetSelectionInfo().m_bBeginLineSelect = TRUE;		// �s�P�ʑI��

	CLayoutPoint ptCaret;

	// �ŉ��s�i�����s�j�łȂ�
	if(GetCaret().GetCaretLogicPos().y < GetDocument()->m_cDocLineMgr.GetLineCount() ){
		// 1�s��̕����s���烌�C�A�E�g�s�����߂�
		GetDocument()->m_cLayoutMgr.LogicToLayout( CLogicPoint(0, GetCaret().GetCaretLogicPos().y + 1), &ptCaret );

		// �J�[�\�������̕����s���ֈړ�����
		m_pCommanderView->MoveCursorSelecting( ptCaret, TRUE );
		
		// �ړ���̃J�[�\���ʒu���擾����
		ptCaret = GetCaret().GetCaretLayoutPos().Get();
	}else{
		// �J�[�\�����ŉ��s�i���C�A�E�g�s�j�ֈړ�����
		m_pCommanderView->MoveCursorSelecting( CLayoutPoint(CLayoutInt(0), GetDocument()->m_cLayoutMgr.GetLineCount()), TRUE );
		Command_GOLINEEND( TRUE, FALSE );	// �s���Ɉړ�

		// �I��������̂������i[EOF]�݂̂̍s�j���͑I����ԂƂ��Ȃ�
		if(( ! m_pCommanderView->GetSelectionInfo().IsTextSelected() )&&
		   ( GetCaret().GetCaretLogicPos().y >= GetDocument()->m_cDocLineMgr.GetLineCount() ))
		{
			// ���݂̑I��͈͂��I����Ԃɖ߂�
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
		}
	}

	if( m_pCommanderView->GetSelectionInfo().m_bBeginLineSelect ){
		// �͈͑I���J�n�s�E�J�������L��
		m_pCommanderView->GetSelectionInfo().m_sSelect.SetTo( ptCaret );
		m_pCommanderView->GetSelectionInfo().m_sSelectBgn.SetTo( ptCaret );
	}

	return;
}




/* ���݈ʒu�̒P��I�� */
bool CViewCommander::Command_SELECTWORD( void )
{
	CLayoutRange sRange;
	CLogicInt	nIdx;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}
	const CLayout*	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		return false;	//	�P��I���Ɏ��s
	}
	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );

	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	if( GetDocument()->m_cLayoutMgr.WhereCurrentWord(	GetCaret().GetCaretLayoutPos().GetY2(), nIdx, &sRange, NULL, NULL ) ){

		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		// 2007.10.15 kobake ���Ƀ��C�A�E�g�P�ʂȂ̂ŕϊ��͕s�v
		/*
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( sRange.GetFrom().GetY2() );
		sRange.SetFromX( m_pCommanderView->LineIndexToColmn( pcLayout, sRange.GetFrom().x ) );
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( sRange.GetTo().GetY2() );
		sRange.SetToX( m_pCommanderView->LineIndexToColmn( pcLayout, sRange.GetTo().x ) );
		*/

		/* �I��͈͂̕ύX */
		//	2005.06.24 Moca
		m_pCommanderView->GetSelectionInfo().SetSelectArea( sRange );

		/* �P��̐擪�ɃJ�[�\�����ړ� */
		GetCaret().MoveCursor( sRange.GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		/* �I��̈�`�� */
		m_pCommanderView->GetSelectionInfo().DrawSelectArea();
		return true;	//	�P��I���ɐ����B
	}
	else {
		return false;	//	�P��I���Ɏ��s
	}
}




/** �\��t��(�N���b�v�{�[�h����\��t��)
	@date 2007.10.04 ryoji MSDEVLineSelect�`���̍s�R�s�[�Ή�������ǉ��iVS2003/2005�̃G�f�B�^�Ɨގ��̋����Ɂj
*/
void CViewCommander::Command_PASTE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	//�����v
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	BOOL		bBox;
	int			i;

	// �N���b�v�{�[�h����f�[�^���擾 -> cmemClip, bColmnSelect
	CNativeW	cmemClip;
	bool		bColmnSelect;
	bool		bLineSelect = false;
	if( !m_pCommanderView->MyGetClipboardData( cmemClip, &bColmnSelect, GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste? &bLineSelect: NULL ) ){
		ErrorBeep();
		return;
	}

	// ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t��
	if( GetDllShareData().m_Common.m_sEdit.m_bAutoColmnPaste ){
		// ��`�R�s�[�̃f�[�^�Ȃ��`�\��t��
		if( bColmnSelect ){
			Command_PASTEBOX();
			return;
		}
	}

	// �N���b�v�{�[�h�f�[�^�擾 -> pszText, nTextLen
	CLogicInt		nTextLen;
	const wchar_t*	pszText = cmemClip.GetStringPtr( &nTextLen );

	// 2007.10.04 ryoji
	// �s�R�s�[�iMSDEVLineSelect�`���j�̃e�L�X�g�Ŗ��������s�ɂȂ��Ă��Ȃ���Ή��s��ǉ�����
	// �����C�A�E�g�܂�Ԃ��̍s�R�s�[�������ꍇ�͖��������s�ɂȂ��Ă��Ȃ�
	if( bLineSelect ){
		// ��CR��LF��2�o�C�g������2�o�C�g�ڂƂ��Ĉ����邱�Ƃ͂Ȃ��̂Ŗ��������Ŕ���iCMemory::GetSizeOfChar()�Q�Ɓj
		if( pszText[nTextLen - 1] != WCODE::CR && pszText[nTextLen - 1] != WCODE::LF ){
			cmemClip.AppendString(GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
			pszText = cmemClip.GetStringPtr( &nTextLen );
		}
	}

	// �e�L�X�g���I������Ă��邩
	bBox = FALSE;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		// ��`�͈͑I�𒆂�
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			bBox = TRUE;
		}
		else{
			//	Jun. 23, 2000 genta
			//	����s�̍s���ȍ~�݂̂��I������Ă���ꍇ�ɂ͑I�𖳂��ƌ��Ȃ�
			CLogicInt		len;
			int pos;
			const wchar_t	*line;
			const CLayout* pcLayout;
			line = GetDocument()->m_cLayoutMgr.GetLineStr( GetSelect().GetFrom().GetY2(), &len, &pcLayout );

			pos = ( line == NULL ) ? 0 : m_pCommanderView->LineColmnToIndex( pcLayout, GetSelect().GetFrom().GetX2() );

			//	�J�n�ʒu���s�������ŁA�I���ʒu������s
			if( pos >= len && GetSelect().IsLineOne()){
				GetCaret().SetCaretLayoutPos(CLayoutPoint(GetSelect().GetFrom().x, GetCaret().GetCaretLayoutPos().y)); //�L�����b�gX�ύX
				m_pCommanderView->GetSelectionInfo().DisableSelectArea(false);
			}
			else{
				// �f�[�^�u�� �폜&�}���ɂ��g����
				// �s�R�s�[�̓\��t���ł͑I��͈͍͂폜�i��ōs���ɓ\��t����j	// 2007.10.04 ryoji
				m_pCommanderView->ReplaceData_CEditView(
					GetSelect(),				// �I��͈�
					NULL,					// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
					/*
					pszText,				// �}������f�[�^
					nTextLen,				// �}������f�[�^�̒���
					*/
					bLineSelect? L"": pszText,	// �}������f�[�^
					bLineSelect? CLogicInt(0): nTextLen,	// �}������f�[�^�̒���
					true,
					m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
				);
#ifdef _DEBUG
				gm_ProfileOutput = FALSE;
#endif
				if( !bLineSelect )	// 2007.10.04 ryoji
					return;
			}
		}
	}
	if( bBox ){
		//���s�܂ł𔲂��o�� pszText -> szPaste
		for( i = 0; i < nTextLen; i++  ){
			if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
				break;
			}
		}
		wchar_t szPaste[1024];
		wmemcpy( szPaste, pszText, i );
		szPaste[i] = L'\0';

		//szPaste��}��
		Command_INDENT( szPaste, CLogicInt(i) );
	}
	else{
		GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

		CLogicInt nPosX_PHY_Delta;
		if( bLineSelect ){	// 2007.10.04 ryoji
			/* �}���|�C���g�i�܂�Ԃ��P�ʍs���j�ɃJ�[�\�����ړ� */
			CLogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	// ����O�̃L�����b�g�ʒu
			Command_GOLINETOP( FALSE, 1 );								// �s���Ɉړ�(�܂�Ԃ��P��)
			CLogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	// �����̃L�����b�g�ʒu

			// �}���|�C���g�ƌ��̈ʒu�Ƃ̍���������
			nPosX_PHY_Delta = ptCaretBefore.x - ptCaretAfter.x;

			//UNDO�p�L�^
			if( !m_pCommanderView->m_bDoing_UndoRedo ){
				GetOpeBlk()->AppendOpe(
					new CMoveCaretOpe(
						ptCaretBefore,	/* ����O�̃L�����b�g�ʒu */
						ptCaretAfter	/* �����̃L�����b�g�ʒu */
					)
				);
			}
		}

		// ���݈ʒu�Ƀf�[�^��}��
		CLayoutPoint ptLayoutNew; //�}�����ꂽ�����̎��̈ʒu
		m_pCommanderView->InsertData_CEditView(
			GetCaret().GetCaretLayoutPos(),
			pszText,
			nTextLen,
			&ptLayoutNew,
			true
		);

		//		::GlobalUnlock(hglb);
		// �}���f�[�^�̍Ō�փJ�[�\�����ړ�
		GetCaret().MoveCursor( ptLayoutNew, TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( bLineSelect ){	// 2007.10.04 ryoji
			/* ���̈ʒu�փJ�[�\�����ړ� */
			CLogicPoint ptCaretBefore = GetCaret().GetCaretLogicPos();	//����O�̃L�����b�g�ʒu
			CLayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				ptCaretBefore + CLogicPoint(nPosX_PHY_Delta, CLogicInt(0)),
				&ptLayout
			);
			GetCaret().MoveCursor( ptLayout, true );					//�J�[�\���ړ�
			CLogicPoint ptCaretAfter = GetCaret().GetCaretLogicPos();	//�����̃L�����b�g�ʒu

			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().x;

			//UNDO�p�L�^
			if( !m_pCommanderView->m_bDoing_UndoRedo ){
				GetOpeBlk()->AppendOpe(
					new CMoveCaretOpe(
						ptCaretBefore,	/* ����O�̃L�����b�g�ʒu�w */
						ptCaretAfter	/* �����̃L�����b�g�ʒu�w */
					)
				);
			}
		}
	}
	return;

}


/*! �e�L�X�g��\��t��
	@date 2004.05.14 Moca '\\0'���󂯓����悤�ɁA�����ɒ�����ǉ�
*/
void CViewCommander::Command_INSTEXT(
	bool			bRedraw,		//!< 
	const wchar_t*	pszText,		//!< [in] �\��t���镶����B
	CLogicInt		nTextLen,		//!< [in] pszText�̒����B-1���w�肷��ƁApszText��NUL�I�[������Ƃ݂Ȃ��Ē����������v�Z����
	bool			bNoWaitCursor	//!< 
)
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	CWaitCursor*	pcWaitCursor;

	GetDocument()->m_cDocEditor.SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	if( bNoWaitCursor ){
		pcWaitCursor = NULL;
	}else{
		pcWaitCursor = new CWaitCursor( m_pCommanderView->GetHwnd() );
	}

	if( nTextLen < 0 ){
		nTextLen = CLogicInt(wcslen( pszText ));
	}

	/* �e�L�X�g���I������Ă��邩 */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			// i = strcspn(pszText, CRLF);
			// 2004.05.14 Moca strcspn�ł�'\0'�������Ȃ��̂�
			CLogicInt i;
			for( i = CLogicInt(0); i < nTextLen; i++ ){
				if( pszText[i] == WCODE::CR 
				 || pszText[i] == WCODE::LF ){
					break;
				}
			}
			Command_INDENT( pszText, i );
		}
		else{
			/* �f�[�^�u�� �폜&�}���ɂ��g���� */
			m_pCommanderView->ReplaceData_CEditView(
				GetSelect(),			//�I��͈�
				NULL,				// �폜���ꂽ�f�[�^�̃R�s�[(NULL�\)
				pszText,			// �}������f�[�^
				nTextLen,			// �}������f�[�^�̒���
				bRedraw,
				m_pCommanderView->m_bDoing_UndoRedo?NULL:m_pCommanderView->m_pcOpeBlk
			);
#ifdef _DEBUG
				gm_ProfileOutput = FALSE;
#endif
		}
	}
	else
	{
		GetDocument()->m_cDocEditor.SetModified(true,true);	/* �ύX�t���O */
		
		//	Jun. 13, 2004 genta �s�v�ȃ`�F�b�N�H
		if( nTextLen < 0 ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONWARNING, GSTR_APPNAME,
				_T("�o�O����Ȃ��́H @Command_INSTEXT") );
			nTextLen = CLogicInt(wcslen( pszText ));
		}

		/* ���݈ʒu�Ƀf�[�^��}�� */
		CLayoutPoint ptLayoutNew;	// �}�����ꂽ�����̎��̈ʒu
		m_pCommanderView->InsertData_CEditView( GetCaret().GetCaretLayoutPos(), pszText, nTextLen, &ptLayoutNew, true );

		/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
		GetCaret().MoveCursor( ptLayoutNew, bRedraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
	}

	if( NULL != pcWaitCursor ){
		delete pcWaitCursor;
	}
	return;
}

//<< 2002/03/28 Azumaiya
// �������f�[�^����`�\��t���p�̃f�[�^�Ɖ��߂��ď�������B
//  �Ȃ��A���̊֐��� Command_PASTEBOX(void) �ƁA
// 2769 : GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta
// ����A
// 3057 : m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
// �Ԃ܂ŁA�ꏏ�ł��B
//  �ł����A�R�����g���������A#if 0 �̂Ƃ����������肵�Ă��܂��̂ŁACommand_PASTEBOX(void) ��
// �c���悤�ɂ��܂���(���ɂ��̊֐����g�����g�����o�[�W�������R�����g�ŏ����Ă����܂���)�B
//  �Ȃ��A�ȉ��ɂ�����悤�� Command_PASTEBOX(void) �ƈႤ�Ƃ��낪����̂Œ��ӂ��Ă��������B
// > �Ăяo�������ӔC�������āA
// �E�}�E�X�ɂ��͈͑I�𒆂ł���B
// �E���݂̃t�H���g�͌Œ蕝�t�H���g�ł���B
// �� 2 �_���`�F�b�N����B
// > �ĕ`����s��Ȃ�
// �ł��B
//  �Ȃ��A�������Ăяo�����Ɋ��҂���킯�́A�u���ׂĒu���v�̂悤�ȉ�����A���ŌĂяo��
// �Ƃ��ɁA�ŏ��Ɉ��`�F�b�N����΂悢���̂�������`�F�b�N����͖̂��ʂƔ��f�������߂ł��B
// @note 2004.06.30 ���݁A���ׂĒu���ł͎g�p���Ă��Ȃ�
void CViewCommander::Command_PASTEBOX( const wchar_t *szPaste, int nPasteSize )
{
	/* �����̓���͎c���Ă��������̂����A�Ăяo�����ŐӔC�������Ă���Ă��炤���ƂɕύX�B
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// �}�E�X�ɂ��͈͑I��
	{
		ErrorBeep();
		return;
	}
	if( !GetDllShareData().m_Common.m_bFontIs_FIXED_PITCH )	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	{
		return;
	}
	*/

	int				nBgn;
	int				nPos;
	CLayoutInt		nCount;
	CLayoutPoint	ptLayoutNew;	//�}�����ꂽ�����̎��̈ʒu
	BOOL			bAddLastCR;
	CLayoutInt		nInsPosX;

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor

	// �Ƃ肠�����I��͈͂��폜
	// 2004.06.30 Moca m_pCommanderView->GetSelectionInfo().IsTextSelected()���Ȃ��Ɩ��I�����A�ꕶ�������Ă��܂�
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		m_pCommanderView->DeleteData( FALSE/*TRUE 2002.01.25 hor*/ );
	}

	CLayoutPoint ptCurOld = GetCaret().GetCaretLayoutPos();

	nCount = CLayoutInt(0);

	// Jul. 10, 2005 genta �\��t���f�[�^�̍Ō��CR/LF�������ꍇ�̑΍�
	//	�f�[�^�̍Ō�܂ŏ��� i.e. nBgn��nPasteSize�𒴂�����I��
	//for( nPos = 0; nPos < nPasteSize; )
	for( nBgn = nPos = 0; nBgn < nPasteSize; )
	{
		// Jul. 10, 2005 genta �\��t���f�[�^�̍Ō��CR/LF��������
		//	�ŏI�s��Paste�����������Ȃ��̂ŁC
		//	�f�[�^�̖����ɗ����ꍇ�͋����I�ɏ�������悤�ɂ���
		if( szPaste[nPos] == WCODE::CR || szPaste[nPos] == WCODE::LF || nPos == nPasteSize )
		{
			/* ���݈ʒu�Ƀf�[�^��}�� */
			if( nPos - nBgn > 0 ){
				m_pCommanderView->InsertData_CEditView(
					ptCurOld + CLayoutPoint(CLayoutInt(0), nCount),
					&szPaste[nBgn],
					nPos - nBgn,
					&ptLayoutNew,
					false
				);
			}

			/* ���̍s�̑}���ʒu�փJ�[�\�����ړ� */
			GetCaret().MoveCursor( ptCurOld + CLayoutPoint(CLayoutInt(0), nCount), false );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			/* �J�[�\���s���Ō�̍s���s���ɉ��s�������A�}�����ׂ��f�[�^���܂�����ꍇ */
			bAddLastCR = FALSE;
			const CLayout*	pcLayout;
			CLogicInt		nLineLen = CLogicInt(0);
			const wchar_t*	pLine;
			pLine = GetDocument()->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pcLayout );

			if( NULL != pLine && 1 <= nLineLen )
			{
				if( pLine[nLineLen - 1] == WCODE::CR || pLine[nLineLen - 1] == WCODE::LF )
				{
				}
				else
				{
					bAddLastCR = TRUE;
				}
			}
			else
			{ // 2001/10/02 novice
				bAddLastCR = TRUE;
			}

			if( bAddLastCR )
			{
//				MYTRACE_A( " �J�[�\���s���Ō�̍s���s���ɉ��s�������A\n�}�����ׂ��f�[�^���܂�����ꍇ�͍s���ɉ��s��}���B\n" );
				nInsPosX = m_pCommanderView->LineIndexToColmn( pcLayout, nLineLen );

				m_pCommanderView->InsertData_CEditView(
					CLayoutPoint(nInsPosX, GetCaret().GetCaretLayoutPos().GetY2()),
					GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2(),
					GetDocument()->m_cDocEditor.GetNewLineCode().GetLen(),
					&ptLayoutNew,
					false
				);
			}

			if(
				(nPos + 1 < nPasteSize ) &&
				(
// 2004.06.30 Moca WORD*�ł͔�x86�ŋ��E�s����̉\������
				 ( szPaste[nPos] == L'\n' && szPaste[nPos + 1] == L'\r') ||
				 ( szPaste[nPos] == L'\r' && szPaste[nPos + 1] == L'\n')
//				 ((WORD *)(szPaste + nPos))[0] == MAKEWORD('\n', L'\r') ||
//				 ((WORD *)(szPaste + nPos))[0] == MAKEWORD('\r', L'\n')
				)
			  )
			{
				nBgn = nPos + 2;
			}
			else
			{
				nBgn = nPos + 1;
			}

			nPos = nBgn;
			++nCount;
		}
		else
		{
			++nPos;
		}
	}

	/* �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ� */
	GetCaret().MoveCursor( ptCurOld, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	if( !m_pCommanderView->m_bDoing_UndoRedo )	/* �A���h�D�E���h�D�̎��s���� */
	{
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe( 
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}

	m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
	return;
}

// �ꉞ�ACommand_PASTEBOX(char *, int) ���g���� Command_PASTEBOX(void) �������Ă����܂��B
/* ��`�\��t��(�N���b�v�{�[�h�����`�\��t��) */
// 2004.06.29 Moca ���g�p���������̂�L���ɂ���
//	�I���W�i����Command_PASTEBOX(void)�͂΂�����폜 (genta)
void CViewCommander::Command_PASTEBOX( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )	// �}�E�X�ɂ��͈͑I��
	{
		ErrorBeep();
		return;
	}


	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	{
		return;
	}

	// �N���b�v�{�[�h����f�[�^���擾
	CNativeW	cmemClip;
	if( !m_pCommanderView->MyGetClipboardData( cmemClip, NULL ) ){
		ErrorBeep();
		return;
	}
	// 2004.07.13 Moca \0�R�s�[�΍�
	int nstrlen;
	const wchar_t *lptstr = cmemClip.GetStringPtr( &nstrlen );

	Command_PASTEBOX(lptstr, nstrlen);
	m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
	m_pCommanderView->Redraw();			// 2002.01.25 hor
}

//>> 2002/03/29 Azumaiya

/* wchar_t1���̕�������� */
void CViewCommander::Command_WCHAR( wchar_t wcChar )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	CLogicInt		nPos;
	CLogicInt		nCharChars;
	CLogicInt		nIdxTo;
	CLayoutInt		nPosX;

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	/* ���݈ʒu�Ƀf�[�^��}�� */
	nPosX = CLayoutInt(-1);
	CNativeW cmemDataW2;
	cmemDataW2 = wcChar;
	if( WCODE::IsLineDelimiter(wcChar) ){ 
		/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */
		// enumEOLType nWorkEOL;
		CEol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
		cmemDataW2.SetString( cWork.GetValue2(), cWork.GetLen() );

		/* �e�L�X�g���I������Ă��邩 */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			m_pCommanderView->DeleteData( TRUE );
		}
		if( GetDocument()->m_cDocType.GetDocumentAttribute().m_bAutoIndent ){	/* �I�[�g�C���f���g */
			const CLayout* pCLayout;
			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			pLine = GetDocument()->m_cLayoutMgr.GetLineStr( GetCaret().GetCaretLayoutPos().GetY2(), &nLineLen, &pCLayout );
			if( NULL != pCLayout ){
				const CDocLine* pcDocLine;
				pcDocLine = GetDocument()->m_cDocLineMgr.GetLine( pCLayout->GetLogicLineNo() );
				pLine = pcDocLine->GetDocLineStrWithEOL( &nLineLen );
				if( NULL != pLine ){
					/*
					  �J�[�\���ʒu�ϊ�
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					  ��
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					*/
					CLogicPoint ptXY;
					GetDocument()->m_cLayoutMgr.LayoutToLogic(
						GetCaret().GetCaretLayoutPos(),
						&ptXY
					);

					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					nIdxTo = m_pCommanderView->LineColmnToIndex( pcDocLine, GetCaret().GetCaretLayoutPos().GetX2() );
					for( nPos = CLogicInt(0); nPos < nLineLen && nPos < ptXY.GetX2(); ){
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );

						/* ���̑��̃C���f���g���� */
						if( 0 < nCharChars
						 && pLine[nPos] != L'\0'	// ���̑��̃C���f���g������ L'\0' �͊܂܂�Ȃ�	// 2009.02.04 ryoji L'\0'���C���f���g����Ă��܂����C��
						 && 0 < (int)wcslen( GetDocument()->m_cDocType.GetDocumentAttribute().m_szIndentChars )
						){
							wchar_t szCurrent[10];
							wmemcpy( szCurrent, &pLine[nPos], nCharChars );
							szCurrent[nCharChars] = L'\0';
							/* ���̑��̃C���f���g�Ώە��� */
							if( NULL != wcsstr(
								GetDocument()->m_cDocType.GetDocumentAttribute().m_szIndentChars,
								szCurrent
							) ){
								goto end_of_for;
							}
						}

						bool bZenSpace=GetDocument()->m_cDocType.GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
						if(nCharChars==1 && WCODE::IsIndentChar(pLine[nPos],bZenSpace))
						{
							//���֐i��
						}
						else break;

end_of_for:;
						nPos += nCharChars;
					}
					if( nPos > 0 ){
						nPosX = m_pCommanderView->LineIndexToColmn( pcDocLine, nPos );
					}

					//�C���f���g�擾
					CNativeW cmemIndent;
					cmemIndent.SetString( pLine, nPos );

					//�C���f���g�t��
					cmemDataW2.AppendNativeData(cmemIndent);
				}
			}
		}
	}
	else{
		/* �e�L�X�g���I������Ă��邩 */
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			/* ��`�͈͑I�𒆂� */
			if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
				Command_INDENT( wcChar );
				return;
			}else{
				m_pCommanderView->DeleteData( TRUE );
			}
		}
		else{
			if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */ ){
				DelCharForOverwrite();	// �㏑���p�̈ꕶ���폜	// 2009.04.11 ryoji
			}
		}
	}

	//UNICODE�ɕϊ�����
	CNativeW cmemUnicode;
	cmemUnicode.SetNativeData(cmemDataW2);

	//�{���ɑ}������
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		cmemUnicode.GetStringPtr(),
		cmemUnicode.GetStringLength(),
		&ptLayoutNew,
		true
	);

	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
	GetCaret().MoveCursor( ptLayoutNew, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	/* �X�}�[�g�C���f���g */
	ESmartIndentType nSIndentType = GetDocument()->m_cDocType.GetDocumentAttribute().m_eSmartIndent;
	switch( nSIndentType ){	/* �X�}�[�g�C���f���g��� */
	case SMARTINDENT_NONE:
		break;
	case SMARTINDENT_CPP:
		/* C/C++�X�}�[�g�C���f���g���� */
		m_pCommanderView->SmartIndent_CPP( wcChar );
		break;
	default:
		//�v���O�C�����猟������
		{
			CPlug::Array plugs;
			CJackManager::Instance()->GetUsablePlug( PP_SMARTINDENT, nSIndentType, &plugs );

			if( plugs.size() > 0 ){
				//�C���^�t�F�[�X�I�u�W�F�N�g����
				CWSHIfObj::List params;
				//�v���O�C���Ăяo��
				( *plugs.begin() )->Invoke( m_pCommanderView, params );				break;
			}
		}
		break;
	}

	/* 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜 */
	if( WCODE::CR == wcChar && GetDocument()->m_cDocType.GetDocumentAttribute().m_bRTrimPrevLine ){	/* ���s���ɖ����̋󔒂��폜 */
		/* �O�̍s�ɂ��閖���̋󔒂��폜���� */
		m_pCommanderView->RTrimPrevLine();
	}

	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta �֐���
}




/*!
	@brief 2�o�C�g��������
	
	WM_IME_CHAR�ő����Ă�����������������D
	�������C�}�����[�h�ł�WM_IME_CHAR�ł͂Ȃ�WM_IME_COMPOSITION�ŕ������
	�擾����̂ł����ɂ͗��Ȃ��D

	@param wChar [in] SJIS�����R�[�h�D��ʂ�1�o�C�g�ځC���ʂ�2�o�C�g�ځD
	
	@date 2002.10.06 genta �����̏㉺�o�C�g�̈Ӗ����t�]�D
		WM_IME_CHAR��wParam�ɍ��킹���D
*/
void CViewCommander::Command_IME_CHAR( WORD wChar )
{
	if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() ){	/* �}�E�X�ɂ��͈͑I�� */
		ErrorBeep();
		return;
	}

	CMemory			cmemData;

	//	Oct. 6 ,2002 genta �㉺�t�]
	if( 0 == (wChar & 0xff00) ){
		Command_WCHAR( wChar & 0xff );
		return;
	}
	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	// Oct. 6 ,2002 genta �o�b�t�@�Ɋi�[����
	// Aug. 15, 2007 kobake WCHAR�o�b�t�@�ɕϊ�����
	wchar_t szWord[2]={wChar,0};
	CLogicInt nWord=CLogicInt(1);

	/* �e�L�X�g���I������Ă��邩 */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			Command_INDENT( szWord, nWord );	//	Oct. 6 ,2002 genta 
			return;
		}else{
			m_pCommanderView->DeleteData( TRUE );
		}
	}
	else{
		if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */ ){
			DelCharForOverwrite();	// �㏑���p�̈ꕶ���폜	// 2009.04.11 ryoji
		}
	}

	//	Oct. 6 ,2002 genta 
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView( GetCaret().GetCaretLayoutPos(), szWord, nWord, &ptLayoutNew, true );

	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
	GetCaret().MoveCursor( ptLayoutNew, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

	m_pCommanderView->PostprocessCommand_hokan();	//	Jan. 10, 2005 genta �֐���
}




/*! �}���^�㏑�����[�h�؂�ւ�

	@date 2005.10.02 genta InsMode�֐���
*/
void CViewCommander::Command_CHGMOD_INS( void )
{
	/* �}�����[�h���H */
	if( m_pCommanderView->IsInsMode() ){
		m_pCommanderView->SetInsMode( false );
	}else{
		m_pCommanderView->SetInsMode( true );
	}
	/* �L�����b�g�̕\���E�X�V */
	GetCaret().ShowEditCaret();
	/* �L�����b�g�̍s���ʒu��\������ */
	GetCaret().ShowCaretPosInfo();
}


/*!
����(�{�b�N�X)�R�}���h���s.
�c�[���o�[�̌����{�b�N�X�Ƀt�H�[�J�X���ړ�����.
	@date 2006.06.04 yukihane �V�K�쐬
*/
void CViewCommander::Command_SEARCH_BOX( void )
{
	const CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;
	pCEditWnd->m_cToolbar.SetFocusSearchBox();
}

/* ����(�P�ꌟ���_�C�A���O) */
void CViewCommander::Command_SEARCH_DIALOG( void )
{
//	int			nRet;

	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	CNativeW		cmemCurText;
	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	/* ����������������� */
	wcscpy( GetEditWindow()->m_cDlgFind.m_szText, cmemCurText.GetStringPtr() );

	/* �����_�C�A���O�̕\�� */
	if( NULL == GetEditWindow()->m_cDlgFind.GetHwnd() ){
		GetEditWindow()->m_cDlgFind.DoModeless( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)&GetEditWindow()->GetActiveView() );
	}
	else{
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( GetEditWindow()->m_cDlgFind.GetHwnd() );
		::DlgItem_SetText( GetEditWindow()->m_cDlgFind.GetHwnd(), IDC_COMBO_TEXT, cmemCurText.GetStringT() );
	}
	return;
}



/* �O������ */
void CViewCommander::Command_SEARCH_PREV( bool bReDraw, HWND hwndParent )
{
	bool		bSelecting;
	bool		bSelectingLock_Old;
	bool		bFound = false;
	bool		bRedo = false;			//	hor
	CLayoutInt	nLineNumOld;
	CLogicInt	nIdxOld;
	const CLayout* pcLayout = NULL;
	CLayoutInt nLineNum;
	CLogicInt nIdx;

	CLayoutRange sRangeA;
	sRangeA.Set(GetCaret().GetCaretLayoutPos());

	CLayoutRange sSelectBgn_Old;
	CLayoutRange sSelect_Old;

	//	bFlag1 = FALSE;
	bSelecting = FALSE;
	if( L'\0' == GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0][0] ){
		goto end_of_func;
	}
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		sSelectBgn_Old = m_pCommanderView->GetSelectionInfo().m_sSelectBgn; //�͈͑I��(���_)
		sSelect_Old = GetSelect();
		
		bSelectingLock_Old = m_pCommanderView->GetSelectionInfo().m_bSelectingLock;
		
		/* ��`�͈͑I�𒆂� */
		if( !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && TRUE == m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){	/* �I����Ԃ̃��b�N */
			bSelecting = TRUE;
		}
		else{
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( bReDraw );
		}
	}

	nLineNum = GetCaret().GetCaretLayoutPos().GetY2();
	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );


	if( NULL == pcLayout ){
		// pcLayout��NULL�ƂȂ�̂́A[EOF]����O���������ꍇ
		// �P�s�O�Ɉړ����鏈��
		nLineNum--;
		if( nLineNum < 0 ){
			goto end_of_func;
		}
		pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		if( NULL == pcLayout ){
			goto end_of_func;
		}
		// �J�[�\�����ړ��͂�߂� nIdx�͍s�̒����Ƃ��Ȃ���[EOF]������s��O�����������ɍŌ�̉��s�������ł��Ȃ� 2003.05.04 �����
		CLayout* pCLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
		nIdx = CLogicInt(pCLayout->GetDocLineRef()->GetLengthWithEOL() + 1);		// �s���̃k������(\0)�Ƀ}�b�`�����邽�߂�+1 2003.05.16 �����
	} else {
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
	}
	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	if(!m_pCommanderView->ChangeCurRegexp())return;

	bRedo		=	TRUE;		//	hor
	nLineNumOld	=	nLineNum;	//	hor
	nIdxOld		=	nIdx;		//	hor
re_do:;							//	hor
	/* ���݈ʒu���O�̈ʒu���������� */
	if( GetDocument()->m_cLayoutMgr.SearchWord(
		nLineNum,								// �����J�n���C�A�E�g�s
		nIdx,									// �����J�n�f�[�^�ʒu
		m_pCommanderView->m_szCurSrchKey,							// ��������
		SEARCH_BACKWARD,						// 0==�O������ 1==�������
		m_pCommanderView->m_sCurSearchOption,						// �����I�v�V����
		&sRangeA,								// �}�b�`���C�A�E�g�͈�
		&m_pCommanderView->m_CurRegexp							// ���K�\���R���p�C���f�[�^
	) ){
		if( bSelecting ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRangeA.GetFrom() );
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* �I����Ԃ̃��b�N */
		}else{
			/* �I��͈͂̕ύX */
			//	2005.06.24 Moca
			m_pCommanderView->GetSelectionInfo().SetSelectArea( sRangeA );

			if( bReDraw ){
				/* �I��̈�`�� */
				m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			}
		}
		/* �J�[�\���ړ� */
		//	Sep. 8, 2000 genta
		m_pCommanderView->AddCurrentLineToHistory();
		GetCaret().MoveCursor( sRangeA.GetFrom(), bReDraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		bFound = TRUE;
	}else{
		/* �t�H�[�J�X�ړ����̍ĕ`�� */
//		m_pCommanderView->RedrawAll();	hor �R�����g��
		if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( bReDraw );
		}
		if( bSelecting ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* �I����Ԃ̃��b�N */
			/* �I��͈͂̕ύX */
			m_pCommanderView->GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old;
			GetSelect() = sSelect_Old;

			/* �J�[�\���ړ� */
			GetCaret().MoveCursor( sRangeA.GetFrom(), bReDraw );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			/* �I��̈�`�� */
			m_pCommanderView->GetSelectionInfo().DrawSelectArea();
		}
	}
end_of_func:;
// From Here 2002.01.26 hor �擪�i�����j����Č���
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&	// ������Ȃ�����
			bRedo		// �ŏ��̌���
		){
			nLineNum=GetDocument()->m_cLayoutMgr.GetLineCount()-CLayoutInt(1);
			nIdx=CLogicInt(MAXLINEKETAS);
			bRedo=FALSE;
			goto re_do;	// ��������Č���
		}
	}
	if(bFound){
		if((nLineNumOld < nLineNum)||(nLineNumOld == nLineNum && nIdxOld < nIdx))
			m_pCommanderView->SendStatusMessage(_T("����������Č������܂���"));
	}else{
		m_pCommanderView->SendStatusMessage(_T("��������܂���ł���"));
//	if( !bFound ){
// To Here 2002.01.26 hor
		ErrorBeep();
		if( bReDraw	&&
			GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND 	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
		){
			if( NULL == hwndParent ){
				hwndParent = m_pCommanderView->GetHwnd();
			}
			InfoMessage(
				hwndParent,
				_T("�O��(��) �ɕ����� '%ls' ���P��������܂���B"),	//Jan. 25, 2001 jepro ���b�Z�[�W���኱�ύX
				m_pCommanderView->m_szCurSrchKey
			);
		}
	}
	return;
}




/*! ��������
	@param bChangeCurRegexp ���L�f�[�^�̌�����������g��
	@date 2003.05.22 ����� �����}�b�`�΍�D�s���E�s�������������D
	@date 2004.05.30 Moca bChangeCurRegexp=true�ŏ]���ʂ�Bfalse�ŁACEditView�̌��ݐݒ肳��Ă��錟���p�^�[�����g��
*/
void CViewCommander::Command_SEARCH_NEXT(
	bool			bChangeCurRegexp,
	bool			bRedraw,
	HWND			hwndParent,
	const WCHAR*	pszNotFoundMessage
)
{
	bool		bSelecting;
	bool		bFlag1;
	bool		bSelectingLock_Old;
	bool		bFound = false;
	const CLayout* pcLayout;
	bool b0Match = false;		//!< �����O�Ń}�b�`���Ă��邩�H�t���O by �����
	const wchar_t *pLine;
	CLogicInt nIdx;
	CLogicInt	nLineLen;
	CLayoutInt	nLineNum;

	CLayoutRange sRangeA;
	sRangeA.Set(GetCaret().GetCaretLayoutPos());

	CLayoutRange sSelectBgn_Old;
	CLayoutRange sSelect_Old;
	CLayoutInt  nLineNumOld;

	bSelecting = FALSE;
	//	2004.05.30 Moca bChangeCurRegexp�ɉ����đΏە������ς���
	if( bChangeCurRegexp  && L'\0' == GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0][0] 
	 || !bChangeCurRegexp && L'\0' == m_pCommanderView->m_szCurSrchKey[0] ){
		goto end_of_func;
	}

	// �����J�n�ʒu�𒲐�
	bFlag1 = FALSE;
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ��`�͈͑I�𒆂łȂ� & �I����Ԃ̃��b�N */
		if( !m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && m_pCommanderView->GetSelectionInfo().m_bSelectingLock ){
			bSelecting = TRUE;
			bSelectingLock_Old = m_pCommanderView->GetSelectionInfo().m_bSelectingLock;

			sSelectBgn_Old = m_pCommanderView->GetSelectionInfo().m_sSelectBgn; //�͈͑I��(���_)
			sSelect_Old = GetSelect();

			if( PointCompare(m_pCommanderView->GetSelectionInfo().m_sSelectBgn.GetFrom(),GetCaret().GetCaretLayoutPos()) >= 0 ){
				// �J�[�\���ړ�
				GetCaret().SetCaretLayoutPos(GetSelect().GetFrom());
				if (GetSelect().IsOne()) {
					// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͂P�����i�߂�(�����}�b�`�΍�) by �����
					b0Match = true;
				}
				bFlag1 = TRUE;
			}
			else{
				// �J�[�\���ړ�
				GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
				if (GetSelect().IsOne()) {
					// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͂P�����i�߂�(�����}�b�`�΍�) by �����
					b0Match = true;
				}
			}
		}
		else{
			/* �J�[�\���ړ� */
			GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
			if (GetSelect().IsOne()) {
				// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͂P�����i�߂�(�����}�b�`�΍�) by �����
				b0Match = true;
			}

			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_pCommanderView->GetSelectionInfo().DisableSelectArea( bRedraw );
		}
	}
	nLineNum = GetCaret().GetCaretLayoutPos().GetY2();
	nLineLen = CLogicInt(0); // 2004.03.17 Moca NULL == pLine�̂Ƃ��AnLineLen�����ݒ�ɂȂ藎����o�O�΍�
	pLine = GetDocument()->m_cLayoutMgr.GetLineStr(nLineNum, &nLineLen, &pcLayout);

	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
// 2002.02.08 hor EOF�݂̂̍s��������������Ă��Č����\�� (2/2)
	nIdx = pcLayout ? m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() ) : CLogicInt(0);
	if( b0Match ) {
		// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͕����s�łP�����i�߂�(�����}�b�`�΍�)
		if( nIdx < nLineLen ) {
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nIdx += CLogicInt(CNativeW::GetSizeOfChar(pLine, nLineLen, nIdx) == 2 ? 2 : 1);
		} else {
			// �O�̂��ߍs���͕ʏ���
			++nIdx;
		}
	}

	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	// 2004.05.30 Moca CEditView�̌��ݐݒ肳��Ă��錟���p�^�[�����g����悤��
	if(bChangeCurRegexp && !m_pCommanderView->ChangeCurRegexp())return;

	bool bRedo			= TRUE;		//	hor
	nLineNumOld = nLineNum;	//	hor
	int  nIdxOld		= nIdx;		//	hor

re_do:;
	 /* ���݈ʒu�����̈ʒu���������� */
	// 2004.05.30 Moca ������GetShareData()���烁���o�ϐ��ɕύX�B���̃v���Z�X/�X���b�h�ɏ����������Ă��܂�Ȃ��悤�ɁB
	int nSearchResult=GetDocument()->m_cLayoutMgr.SearchWord(
		nLineNum,						// �����J�n���C�A�E�g�s
		nIdx,							// �����J�n�f�[�^�ʒu
		m_pCommanderView->m_szCurSrchKey,					// ��������
		SEARCH_FORWARD,					// 0==�O������ 1==�������
		m_pCommanderView->m_sCurSearchOption,				// �����I�v�V����
		&sRangeA,						// �}�b�`���C�A�E�g�͈�
		&m_pCommanderView->m_CurRegexp					// ���K�\���R���p�C���f�[�^
	);
	if( nSearchResult ){
		// �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
		if( bFlag1 && sRangeA.GetFrom()==GetCaret().GetCaretLayoutPos() ){
			CLogicRange sRange_Logic;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(sRangeA,&sRange_Logic);

			nLineNum = sRangeA.GetTo().GetY2();
			nIdx     = sRange_Logic.GetTo().GetX2();
			if( sRange_Logic.GetFrom() == sRange_Logic.GetTo() ) { // ��0�}�b�`�ł̖������[�v�΍�B
				nIdx += 1; // wchar_t����i�߂邾���ł͑���Ȃ���������Ȃ����B
			}
			goto re_do;
		}

		if( bSelecting ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			m_pCommanderView->GetSelectionInfo().ChangeSelectAreaByCurrentCursor( sRangeA.GetTo() );
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* �I����Ԃ̃��b�N */
		}
		else{
			/* �I��͈͂̕ύX */
			//	2005.06.24 Moca
			m_pCommanderView->GetSelectionInfo().SetSelectArea( sRangeA );

			if( bRedraw ){
				/* �I��̈�`�� */
				m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			}
		}

		/* �J�[�\���ړ� */
		//	Sep. 8, 2000 genta
		if ( m_pCommanderView->GetDrawSwitch() ) m_pCommanderView->AddCurrentLineToHistory();	// 2002.02.16 hor ���ׂĒu���̂Ƃ��͕s�v
		GetCaret().MoveCursor( sRangeA.GetFrom(), bRedraw );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		bFound = TRUE;
	}
	else{
		if( bSelecting ){
			m_pCommanderView->GetSelectionInfo().m_bSelectingLock = bSelectingLock_Old;	/* �I����Ԃ̃��b�N */

			/* �I��͈͂̕ύX */
			m_pCommanderView->GetSelectionInfo().m_sSelectBgn = sSelectBgn_Old; //�͈͑I��(���_)
			GetSelect().SetFrom(sSelect_Old.GetFrom());
			GetSelect().SetTo(sRangeA.GetFrom());

			/* �J�[�\���ړ� */
			GetCaret().MoveCursor( sRangeA.GetFrom(), bRedraw );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			if( bRedraw ){
				/* �I��̈�`�� */
				m_pCommanderView->GetSelectionInfo().DrawSelectArea();
			}
		}
	}

end_of_func:;
// From Here 2002.01.26 hor �擪�i�����j����Č���
	if(GetDllShareData().m_Common.m_sSearch.m_bSearchAll){
		if(!bFound	&&		// ������Ȃ�����
			bRedo	&&		// �ŏ��̌���
			m_pCommanderView->GetDrawSwitch()	// �S�Ēu���̎��s������Ȃ�
		){
			nLineNum=CLayoutInt(0);
			nIdx=CLogicInt(0);
			bRedo=FALSE;
			goto re_do;		// �擪����Č���
		}
	}

	if(bFound){
		if((nLineNumOld > nLineNum)||(nLineNumOld == nLineNum && nIdxOld > nIdx))
			m_pCommanderView->SendStatusMessage(_T("���擪����Č������܂���"));
	}
	else{
		GetCaret().ShowEditCaret();	// 2002/04/18 YAZAKI
		GetCaret().ShowCaretPosInfo();	// 2002/04/18 YAZAKI
		m_pCommanderView->SendStatusMessage(_T("��������܂���ł���"));
// To Here 2002.01.26 hor

		/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
		ErrorBeep();
		if( bRedraw	&& GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND ){
			if( NULL == hwndParent ){
				hwndParent = m_pCommanderView->GetHwnd();
			}
			if( NULL == pszNotFoundMessage ){
				InfoMessage(
					hwndParent,
					_T("���(��) �ɕ����� '%ls' ���P��������܂���B"),
					m_pCommanderView->m_szCurSrchKey
				);
			}
			else{
				InfoMessage(hwndParent, _T("%ls"),pszNotFoundMessage);
			}
		}
	}
}




/* �e�탂�[�h�̎����� */
void CViewCommander::Command_CANCEL_MODE( void )
{
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}
	m_pCommanderView->GetSelectionInfo().m_bSelectingLock = FALSE;	/* �I����Ԃ̃��b�N */
	return;
}




/* �͈͑I���J�n */
void CViewCommander::Command_BEGIN_SELECT( void )
{
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		m_pCommanderView->GetSelectionInfo().BeginSelectArea();
	}
	
	//	���b�N�̉����؂�ւ�
	if ( m_pCommanderView->GetSelectionInfo().m_bSelectingLock ) {
		m_pCommanderView->GetSelectionInfo().m_bSelectingLock = FALSE;	/* �I����Ԃ̃��b�N���� */
	}
	else {
		m_pCommanderView->GetSelectionInfo().m_bSelectingLock = TRUE;	/* �I����Ԃ̃��b�N */
	}
	return;
}




/* ��`�͈͑I���J�n */
void CViewCommander::Command_BEGIN_BOXSELECT( void )
{
	if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH ){	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		return;
	}

//@@@ 2002.01.03 YAZAKI �͈͑I�𒆂�Shift+F6�����s����ƑI��͈͂��N���A����Ȃ����ɑΏ�
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}

	/* ���݂̃J�[�\���ʒu����I�����J�n���� */
	m_pCommanderView->GetSelectionInfo().BeginSelectArea();

	m_pCommanderView->GetSelectionInfo().m_bSelectingLock = true;	/* �I����Ԃ̃��b�N */
	m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);	/* ��`�͈͑I�� */
	return;
}




/* �V�K�쐬 */
void CViewCommander::Command_FILENEW( void )
{
	/* �V���ȕҏW�E�B���h�E���N�� */
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath = _T("");
	sLoadInfo.eCharCode = CODE_NONE;
	sLoadInfo.bViewMode = false;
	CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo );
	return;
}



/*! @brief �t�@�C�����J��

	@date 2003.03.30 genta �u���ĊJ���v���痘�p���邽�߂Ɉ����ǉ�
	@date 2004.10.09 genta ������CEditDoc�ֈړ�
*/
void CViewCommander::Command_FILEOPEN( const WCHAR* filename, ECodeType nCharCode, bool bViewMode )
{
	//���[�h���
	SLoadInfo sLoadInfo(filename?to_tchar(filename):_T(""), nCharCode, bViewMode);

	//�K�v�ł���΁u�t�@�C�����J���v�_�C�A���O
	if(!sLoadInfo.cFilePath.IsValidPath()){
		bool bDlgResult = GetDocument()->m_cDocFileOperation.OpenFileDialog(
			CEditWnd::Instance()->GetHwnd(),	//[in]  �I�[�i�[�E�B���h�E
			NULL,								//[in]  �t�H���_
			&sLoadInfo							//[out] ���[�h���󂯎��
		);
		if(!bDlgResult)return;
	}

	//�J��
	GetDocument()->m_cDocFileOperation.FileLoad( &sLoadInfo );
}




/* ����(����) */	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
void CViewCommander::Command_FILECLOSE( void )
{
	GetDocument()->m_cDocFileOperation.FileClose();
}




/*! @brief ���ĊJ��

	@date 2003.03.30 genta �J���_�C�A���O�ŃL�����Z�������Ƃ����̃t�@�C�����c��悤�ɁB
				���ł�FILEOPEN�Ɠ����悤�Ɉ�����ǉ����Ă���
*/
void CViewCommander::Command_FILECLOSE_OPEN( LPCWSTR filename, ECodeType nCharCode, bool bViewMode )
{
	GetDocument()->m_cDocFileOperation.FileCloseOpen( SLoadInfo(to_tchar(filename), nCharCode, bViewMode) );
}




/*! �㏑���ۑ�

	F_FILESAVEALL�Ƃ̑g�ݍ��킹�݂̂Ŏg����R�}���h�D
	@param warnbeep [in] true: �ۑ��s�v or �ۑ��֎~�̂Ƃ��Ɍx�����o��
	@param askname	[in] true: �t�@�C�������ݒ�̎��ɓ��͂𑣂�

	@date 2004.02.28 genta ����warnbeep�ǉ�
	@date 2005.01.24 genta ����askname�ǉ�

*/
bool CViewCommander::Command_FILESAVE( bool warnbeep, bool askname )
{
	CEditDoc* pcDoc = GetDocument();

	//�t�@�C�������w�肳��Ă��Ȃ��ꍇ�́u���O��t���ĕۑ��v�̃t���[�֑J��
	if( !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		if( !askname )
			return false;	// �ۑ����Ȃ�
		return pcDoc->m_cDocFileOperation.FileSaveAs();
	}

	//�Z�[�u���
	SSaveInfo sSaveInfo;
	pcDoc->GetSaveInfo(&sSaveInfo);
	sSaveInfo.cEol = EOL_NONE; //���s�R�[�h���ϊ�
	sSaveInfo.bOverwriteMode = true; //�㏑���v��

	//�㏑������
	if(!warnbeep)CEditApp::Instance()->m_cSoundSet.MuteOn();
	bool bRet = pcDoc->m_cDocFileOperation.DoSaveFlow(&sSaveInfo);
	if(!warnbeep)CEditApp::Instance()->m_cSoundSet.MuteOff();

	return bRet;
}

/* ���O��t���ĕۑ��_�C�A���O */
bool CViewCommander::Command_FILESAVEAS_DIALOG()
{
	return 	GetDocument()->m_cDocFileOperation.FileSaveAs();
}


/* ���O��t���ĕۑ�
	filename�ŕۑ��BNULL�͌��ցB
*/
BOOL CViewCommander::Command_FILESAVEAS( const WCHAR* filename, EEolType eEolType )
{
	return 	GetDocument()->m_cDocFileOperation.FileSaveAs(filename, eEolType);
}

/*!	�S�ď㏑���ۑ�

	�ҏW���̑S�ẴE�B���h�E�ŏ㏑���ۑ����s���D
	�������C�㏑���ۑ��̎w�����o���݂̂Ŏ��s���ʂ̊m�F�͍s��Ȃ��D

	�㏑���֎~�y�уt�@�C�������ݒ�̃E�B���h�E�ł͉����s��Ȃ��D

	@date 2005.01.24 genta �V�K�쐬
*/
BOOL CViewCommander::Command_FILESAVEALL( void )
{
	CAppNodeGroupHandle(0).SendMessageToAllEditors(
		WM_COMMAND,
		MAKELONG( F_FILESAVE_QUIET, 0 ),
		0,
		NULL
	);
	return TRUE;
}


/*!	���ݕҏW���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[
	2002/2/3 aroka
*/
void CViewCommander::Command_COPYFILENAME( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		const WCHAR* pszFile = to_wchar(GetDocument()->m_cDocFile.GetFileName());
		m_pCommanderView->MySetClipboardData( pszFile , wcslen( pszFile ), false );
	}
	else{
		ErrorBeep();
	}
}




/* ���ݕҏW���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[ */
void CViewCommander::Command_COPYPATH( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		const TCHAR* szPath = GetDocument()->m_cDocFile.GetFilePath();
		m_pCommanderView->MySetClipboardData( szPath, _tcslen(szPath), false );
	}
	else{
		ErrorBeep();
	}
}




//	May 9, 2000 genta
/* ���ݕҏW���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���N���b�v�{�[�h�ɃR�s�[ */
void CViewCommander::Command_COPYTAG( void )
{
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		wchar_t	buf[ MAX_PATH + 20 ];

		CLogicPoint ptColLine;

		//	�_���s�ԍ��𓾂�
		GetDocument()->m_cLayoutMgr.LayoutToLogic( GetCaret().GetCaretLayoutPos(), &ptColLine );

		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		auto_sprintf( buf, L"%ts (%d,%d): ", GetDocument()->m_cDocFile.GetFilePath(), ptColLine.y+1, ptColLine.x+1 );
		m_pCommanderView->MySetClipboardData( buf, wcslen( buf ), false );
	}
	else{
		ErrorBeep();
	}
}

/*! �w��s�փW�����v�_�C�A���O�̕\��
	2002.2.2 YAZAKI
*/
void CViewCommander::Command_JUMP_DIALOG( void )
{
	if( !GetEditWindow()->m_cDlgJump.DoModal(
		G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument()
	) ){
		return;
	}
}



/* �w��s�w�W�����v */
void CViewCommander::Command_JUMP( void )
{
	const wchar_t*	pLine;
	int			nMode;
	int			bValidLine;
	int			nCurrentLine;
	int			nCommentBegin;

	if( 0 == GetDocument()->m_cLayoutMgr.GetLineCount() ){
		ErrorBeep();
		return;
	}

	/* �s�ԍ� */
	int	nLineNum; //$$ �P�ʍ���
	nLineNum = GetEditWindow()->m_cDlgJump.m_nLineNum;

	if( !GetEditWindow()->m_cDlgJump.m_bPLSQL ){	/* PL/SQL�\�[�X�̗L���s�� */
		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		if( GetDllShareData().m_bLineNumIsCRLF_ForJump ){
			if( CLogicInt(0) >= nLineNum ){
				nLineNum = CLogicInt(1);
			}
			/*
			  �J�[�\���ʒu�ϊ�
			  ���W�b�N�ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			CLayoutPoint ptPosXY;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineNum - 1),
				&ptPosXY
			);
			nLineNum = (Int)ptPosXY.y + 1;
		}
		else{
			if( 0 >= nLineNum ){
				nLineNum = 1;
			}
			if( nLineNum > GetDocument()->m_cLayoutMgr.GetLineCount() ){
				nLineNum = (Int)GetDocument()->m_cLayoutMgr.GetLineCount();
			}
		}
		//	Sep. 8, 2000 genta
		m_pCommanderView->AddCurrentLineToHistory();
		//	2006.07.09 genta �I����Ԃ��������Ȃ��悤��
		m_pCommanderView->MoveCursorSelecting( CLayoutPoint(0, nLineNum - 1), m_pCommanderView->GetSelectionInfo().m_bSelectingLock, _CARETMARGINRATE / 3 );
		return;
	}
	if( 0 >= nLineNum ){
		nLineNum = 1;
	}
	nMode = 0;
	nCurrentLine = GetEditWindow()->m_cDlgJump.m_nPLSQL_E2 - 1;

	int	nLineCount; //$$ �P�ʍ���
	nLineCount = GetEditWindow()->m_cDlgJump.m_nPLSQL_E1 - 1;

	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	if( !GetDocument()->m_cDocType.GetDocumentAttribute().m_bLineNumIsCRLF ){ //���C�A�E�g�P��
		/*
		  �J�[�\���ʒu�ϊ�
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		  ��
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		*/
		CLogicPoint ptPosXY;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			CLayoutPoint(0,nLineCount),
			&ptPosXY
		);
		nLineCount = ptPosXY.y;
	}

	for( ; nLineCount <  GetDocument()->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		CLogicInt	nLineLen;
		CLogicInt	nBgn = CLogicInt(0);
		CLogicInt	i;
		pLine = GetDocument()->m_cDocLineMgr.GetLine(CLogicInt(nLineCount))->GetDocLineStrWithEOL(&nLineLen);
		bValidLine = FALSE;
		for( i = CLogicInt(0); i < nLineLen; ++i ){
			if( L' ' != pLine[i] &&
				WCODE::TAB != pLine[i]
			){
				break;
			}
		}
		nBgn = i;
		for( i = nBgn; i < nLineLen; ++i ){
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				bValidLine = TRUE;
				if( L'\'' == pLine[i] ){
					if( i > 0 && L'\\' == pLine[i - 1] ){
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 21 == nMode ){
				bValidLine = TRUE;
				if( L'"' == pLine[i] ){
					if( i > 0 && L'\\' == pLine[i - 1] ){
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					if( /*nCommentBegin != nLineCount &&*/ nCommentBegin != 0){
						bValidLine = TRUE;
					}
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'-' == pLine[i] &&  L'-' == pLine[i + 1] ){
					bValidLine = TRUE;
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					nCommentBegin = nLineCount;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L'"' == pLine[i] ){
					nMode = 21;
					continue;
				}else{
					bValidLine = TRUE;
				}
			}
		}
		/* �R�����g�ǂݍ��ݒ� */
		if( 8 == nMode ){
			if( nCommentBegin != 0){
				bValidLine = TRUE;
			}
			/* �R�����g�u���b�N���̉��s�����̍s */
			if( WCODE::CR == pLine[nBgn] ||
				WCODE::LF == pLine[nBgn] ){
				bValidLine = FALSE;
			}
		}
		if( bValidLine ){
			++nCurrentLine;
			if( nCurrentLine >= nLineNum ){
				break;
			}
		}
	}
	/*
	  �J�[�\���ʒu�ϊ�
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	  ��
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	*/
	CLayoutPoint ptPos;
	GetDocument()->m_cLayoutMgr.LogicToLayout(
		CLogicPoint(0, nLineCount),
		&ptPos
	);
	//	Sep. 8, 2000 genta
	m_pCommanderView->AddCurrentLineToHistory();
	//	2006.07.09 genta �I����Ԃ��������Ȃ��悤��
	m_pCommanderView->MoveCursorSelecting( ptPos, m_pCommanderView->GetSelectionInfo().m_bSelectingLock, _CARETMARGINRATE / 3 );
}




/* �t�H���g�ݒ� */
void CViewCommander::Command_FONT( void )
{
	HWND	hwndFrame;
	hwndFrame = GetMainWindow();

	/* �t�H���g�ݒ�_�C�A���O */
	LOGFONT cLogfont = GetDllShareData().m_Common.m_sView.m_lf;
	INT nPointSize;
	if( MySelectFont( &cLogfont, &nPointSize, CEditWnd::Instance()->m_cSplitterWnd.GetHwnd() )  ){
		GetDllShareData().m_Common.m_sView.m_lf = cLogfont;
		GetDllShareData().m_Common.m_sView.m_nPointSize = nPointSize;

		// �������L���b�V���̏�����	// 2008/5/15 Uchi
		InitCharWidthCache(cLogfont);
		InitCharWidthCacheCommon();

//		/* �ύX�t���O �t�H���g */
//		GetDllShareData().m_bFontModify = TRUE;

		if( GetDllShareData().m_Common.m_sView.m_lf.lfPitchAndFamily & FIXED_PITCH  ){
			GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH = TRUE;	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		}else{
			GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH = FALSE;	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		}
		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0, (LPARAM)hwndFrame, hwndFrame
		);

		/* �L�����b�g�̕\�� */
//		::HideCaret( GetHwnd() );
//		::ShowCaret( GetHwnd() );

//		/* �A�N�e�B�u�ɂ��� */
//		/* �A�N�e�B�u�ɂ��� */
//		ActivateFrameWindow( hwndFrame );
	}
	return;
}




/* ���ʐݒ� */
void CViewCommander::Command_OPTION( void )
{
	/* �ݒ�v���p�e�B�V�[�g �e�X�g�p */
	CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheet( -1/*, -1*/ );
}




/* �^�C�v�ʐݒ� */
void CViewCommander::Command_OPTION_TYPE( void )
{
	CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheetTypes( -1, GetDocument()->m_cDocType.GetDocumentType() );
}




/* �^�C�v�ʐݒ�ꗗ */
void CViewCommander::Command_TYPE_LIST( void )
{
	CDlgTypeList			cDlgTypeList;
	CDlgTypeList::SResult	sResult;
	sResult.cDocumentType = GetDocument()->m_cDocType.GetDocumentType();
	if( cDlgTypeList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), &sResult ) ){
		//	Nov. 29, 2000 genta
		//	�ꎞ�I�Ȑݒ�K�p�@�\�𖳗���ǉ�
		if( sResult.bTempChange ){
			GetDocument()->m_cDocType.SetDocumentType( sResult.cDocumentType, true );
			GetDocument()->m_cDocType.LockDocumentType();
			/* �ݒ�ύX�𔽉f������ */
			GetDocument()->m_bTextWrapMethodCurTemp = false;	// �܂�Ԃ����@�̈ꎞ�ݒ�K�p��������	// 2008.06.08 ryoji
			GetDocument()->OnChangeSetting();

			// 2006.09.01 ryoji �^�C�v�ύX�㎩�����s�}�N�������s����
			GetDocument()->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnTypeChanged );
		}
		else{
			/* �^�C�v�ʐݒ� */
			CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheetTypes( -1, sResult.cDocumentType );
		}
	}
	return;
}




/* �s�̓�d��(�܂�Ԃ��P��) */
void CViewCommander::Command_DUPLICATELINE( void )
{
	int				bCRLF;
	int				bAddCRLF;
	CNativeW		cmemBuf;
	const CLayout*	pcLayout;

	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );
	}

	pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL == pcLayout ){
		ErrorBeep();
		return;
	}

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}

	CLayoutPoint ptCaretPosOld = GetCaret().GetCaretLayoutPos() + CLayoutPoint(0,1);

	//�s���Ɉړ�(�܂�Ԃ��P��)
	Command_GOLINETOP( m_pCommanderView->GetSelectionInfo().m_bSelectingLock, 0x1 /* �J�[�\���ʒu�Ɋ֌W�Ȃ��s���Ɉړ� */ );

	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}



	/* ��d���������s�𒲂ׂ�
	||	�E���s�ŏI����Ă���
	||	�E���s�ŏI����Ă��Ȃ�
	||	�E�ŏI�s�ł���
	||	���܂�Ԃ��łȂ�
	||	�E�ŏI�s�łȂ�
	||	���܂�Ԃ��ł���
	*/
	bCRLF = ( EOL_NONE == pcLayout->GetLayoutEol() ) ? FALSE : TRUE;

	bAddCRLF = FALSE;
	if( !bCRLF ){
		if( GetCaret().GetCaretLayoutPos().GetY2() == GetDocument()->m_cLayoutMgr.GetLineCount() - 1 ){
			bAddCRLF = TRUE;
		}
	}

	cmemBuf.SetString( pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() + pcLayout->GetLayoutEol().GetLen() );	//	��pcLayout->GetLengthWithEOL()�́AEOL�̒�����K��1�ɂ���̂Ŏg���Ȃ��B
	if( bAddCRLF ){
		/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */
		CEol cWork = GetDocument()->m_cDocEditor.GetNewLineCode();
		cmemBuf.AppendString( cWork.GetValue2(), cWork.GetLen() );
	}

	/* ���݈ʒu�Ƀf�[�^��}�� */
	CLayoutPoint ptLayoutNew;
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		&ptLayoutNew,
		true
	);

	/* �J�[�\�����ړ� */
	GetCaret().MoveCursor( ptCaretPosOld, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();


	if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		/* ����̒ǉ� */
		GetOpeBlk()->AppendOpe(
			new CMoveCaretOpe(
				GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
				GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
			)
		);
	}
	return;
}




/* ������ */
void CViewCommander::Command_TOLOWER( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_TOLOWER );
	return;
}




/* �啶�� */
void CViewCommander::Command_TOUPPER( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_TOUPPER );
	return;
}




/* �S�p�����p */
void CViewCommander::Command_TOHANKAKU( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_TOHANKAKU );
	return;
}


/* �S�p�J�^�J�i�����p�J�^�J�i */		//Aug. 29, 2002 ai
void CViewCommander::Command_TOHANKATA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_TOHANKATA );
	return;
}


/*! ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
void CViewCommander::Command_TOZENEI( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_TOZENEI );
	return;
}

/*! �S�p�p�������p�p�� */
void CViewCommander::Command_TOHANEI( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_TOHANEI );
	return;
}


/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
void CViewCommander::Command_TOZENKAKUKATA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_TOZENKAKUKATA );
	return;
}




/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
void CViewCommander::Command_TOZENKAKUHIRA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_TOZENKAKUHIRA );
	return;
}




/* ���p�J�^�J�i���S�p�J�^�J�i */
void CViewCommander::Command_HANKATATOZENKAKUKATA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_HANKATATOZENKATA );
	return;
}




/* ���p�J�^�J�i���S�p�Ђ炪�� */
void CViewCommander::Command_HANKATATOZENKAKUHIRA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_HANKATATOZENHIRA );
	return;
}




/* TAB���� */
void CViewCommander::Command_TABTOSPACE( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_TABTOSPACE );
	return;
}

/* �󔒁�TAB */ //---- Stonee, 2001/05/27
void CViewCommander::Command_SPACETOTAB( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_SPACETOTAB );
	return;
}



//#define F_HANKATATOZENKATA	30557	/* ���p�J�^�J�i���S�p�J�^�J�i */
//#define F_HANKATATOZENHIRA	30558	/* ���p�J�^�J�i���S�p�Ђ炪�� */




/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_EMAIL( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_EMAIL );
	return;
}




/* EUC��SJIS�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_EUC2SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_EUC2SJIS );
	return;
}




/* Unicode��SJIS�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_UNICODE2SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UNICODE2SJIS );
	return;
}




/* UnicodeBE��SJIS�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_UNICODEBE2SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UNICODEBE2SJIS );
	return;
}




/* SJIS��JIS�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_SJIS2JIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2JIS );
	return;
}




/* SJIS��EUC�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_SJIS2EUC( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2EUC );
	return;
}




/* UTF-8��SJIS�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_UTF82SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UTF82SJIS );
	return;
}




/* UTF-7��SJIS�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_UTF72SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_UTF72SJIS );
	return;
}




/* SJIS��UTF-7�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_SJIS2UTF7( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2UTF7 );
	return;
}




/* SJIS��UTF-8�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_SJIS2UTF8( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_SJIS2UTF8 );
	return;
}




/* �������ʁ�SJIS�R�[�h�ϊ� */
void CViewCommander::Command_CODECNV_AUTO2SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	m_pCommanderView->ConvSelectedArea( F_CODECNV_AUTO2SJIS );
	return;
}




/*!	�A�E�g���C�����
	
	2002/3/13 YAZAKI nOutlineType��nListType�𓝍��B
*/
// �g�O���p�̃t���O�ɕύX 20060201 aroka
BOOL CViewCommander::Command_FUNCLIST(
	int nAction,
	int _nOutlineType
)
{
	static bool bIsProcessing = false;	//�A�E�g���C����͏������t���O

	//�A�E�g���C���v���O�C�����ł�Editor.Outline�Ăяo���ɂ��ē����֎~����
	if( bIsProcessing )return FALSE;

	bIsProcessing = true;

	EOutlineType nOutlineType = (EOutlineType)_nOutlineType; //2007.11.29 kobake

//	if( bCheckOnly ){
//		return TRUE;
//	}

	static CFuncInfoArr	cFuncInfoArr;
//	int		nLine;
//	int		nListType;
	tstring sTitleOverride;				//�v���O�C���ɂ��_�C�A���O�^�C�g���㏑��

	//	2001.12.03 hor & 2002.3.13 YAZAKI
	if( nOutlineType == OUTLINE_DEFAULT ){
		/* �^�C�v�ʂɐݒ肳�ꂽ�A�E�g���C����͕��@ */
		nOutlineType = GetDocument()->m_cDocType.GetDocumentAttribute().m_eDefaultOutline;
	}

	if( NULL != GetEditWindow()->m_cDlgFuncList.GetHwnd() && nAction != SHOW_RELOAD ){
		switch(nAction ){
		case SHOW_NORMAL: // �A�N�e�B�u�ɂ���
			//	�J���Ă�����̂Ǝ�ʂ������Ȃ�Active�ɂ��邾���D�قȂ�΍ĉ��
			if( GetEditWindow()->m_cDlgFuncList.CheckListType( nOutlineType )){
				ActivateFrameWindow( GetEditWindow()->m_cDlgFuncList.GetHwnd() );
				bIsProcessing = false;
				return TRUE;
			}
			break;
		case SHOW_TOGGLE: // ����
			//	�J���Ă�����̂Ǝ�ʂ������Ȃ����D�قȂ�΍ĉ��
			if( GetEditWindow()->m_cDlgFuncList.CheckListType( nOutlineType )){
				::SendMessageAny( GetEditWindow()->m_cDlgFuncList.GetHwnd(), WM_CLOSE, 0, 0 );
				bIsProcessing = false;
				return TRUE;
			}
			break;
		default:
			break;
		}
	}

	/* ��͌��ʃf�[�^����ɂ��� */
	cFuncInfoArr.Empty();

	switch( nOutlineType ){
//	case OUTLINE_C:			GetDocument()->MakeFuncList_C( &cFuncInfoArr );break;
	case OUTLINE_CPP:
		GetDocument()->m_cDocOutline.MakeFuncList_C( &cFuncInfoArr );
		/* C����W���ی�ψ�������ʏ��������ӏ�(�R) */
		if( CheckEXT( GetDocument()->m_cDocFile.GetFilePath(), _T("c") ) ){
			nOutlineType = OUTLINE_C;	/* �����C�֐��ꗗ���X�g�r���[�ɂȂ� */
		}
		break;
	case OUTLINE_PLSQL:		GetDocument()->m_cDocOutline.MakeFuncList_PLSQL( &cFuncInfoArr );break;
	case OUTLINE_JAVA:		GetDocument()->m_cDocOutline.MakeFuncList_Java( &cFuncInfoArr );break;
	case OUTLINE_COBOL:		GetDocument()->m_cDocOutline.MakeTopicList_cobol( &cFuncInfoArr );break;
	case OUTLINE_ASM:		GetDocument()->m_cDocOutline.MakeTopicList_asm( &cFuncInfoArr );break;
	case OUTLINE_PERL:		GetDocument()->m_cDocOutline.MakeFuncList_Perl( &cFuncInfoArr );break;	//	Sep. 8, 2000 genta
	case OUTLINE_VB:		GetDocument()->m_cDocOutline.MakeFuncList_VisualBasic( &cFuncInfoArr );break;	//	June 23, 2001 N.Nakatani
	case OUTLINE_WZTXT:		GetDocument()->m_cDocOutline.MakeTopicList_wztxt(&cFuncInfoArr);break;		// 2003.05.20 zenryaku �K�w�t�e�L�X�g �A�E�g���C�����
	case OUTLINE_HTML:		GetDocument()->m_cDocOutline.MakeTopicList_html(&cFuncInfoArr);break;		// 2003.05.20 zenryaku HTML �A�E�g���C�����
	case OUTLINE_TEX:		GetDocument()->m_cDocOutline.MakeTopicList_tex(&cFuncInfoArr);break;		// 2003.07.20 naoh TeX �A�E�g���C�����
	case OUTLINE_BOOKMARK:	GetDocument()->m_cDocOutline.MakeFuncList_BookMark( &cFuncInfoArr );break;	//	2001.12.03 hor
	case OUTLINE_FILE:		GetDocument()->m_cDocOutline.MakeFuncList_RuleFile( &cFuncInfoArr );break;	//	2002.04.01 YAZAKI �A�E�g���C����͂Ƀ��[���t�@�C���𓱓�
//	case OUTLINE_UNKNOWN:	//Jul. 08, 2001 JEPRO �g��Ȃ��悤�ɕύX
	case OUTLINE_PYTHON:	GetDocument()->m_cDocOutline.MakeFuncList_python(&cFuncInfoArr);break;		// 2007.02.08 genta
	case OUTLINE_ERLANG:	GetDocument()->m_cDocOutline.MakeFuncList_Erlang(&cFuncInfoArr);break;		// 2009.08.10 genta
	case OUTLINE_TEXT:
		//	fall though
		//	�����ɂ͉�������Ă͂����Ȃ� 2007.02.28 genta ���ӏ���
	default:
		//�v���O�C�����猟������
		{
			CPlug::Array plugs;
			CJackManager::Instance()->GetUsablePlug( PP_OUTLINE, nOutlineType, &plugs );

			if( plugs.size() > 0 ){
				//�C���^�t�F�[�X�I�u�W�F�N�g����
				CWSHIfObj::List params;
				COutlineIfObj* objOutline = new COutlineIfObj( cFuncInfoArr );
				objOutline->AddRef();
				params.push_back( objOutline );
				//�v���O�C���Ăяo��
				( *plugs.begin() )->Invoke( m_pCommanderView, params );

				nOutlineType = objOutline->m_nListType;			//�_�C�A���O�̕\�����@�����㏑��
				sTitleOverride = objOutline->m_sOutlineTitle;	//�_�C�A���O�^�C�g�����㏑��

				objOutline->Release();
				break;
			}
		}

		//����ȊO
		GetDocument()->m_cDocOutline.MakeTopicList_txt( &cFuncInfoArr );
		break;
	}

	/* ��͑Ώۃt�@�C���� */
	_tcscpy( cFuncInfoArr.m_szFilePath, GetDocument()->m_cDocFile.GetFilePath() );

	/* �A�E�g���C�� �_�C�A���O�̕\�� */
	CLayoutPoint poCaret = GetCaret().GetCaretLayoutPos();
	if( NULL == GetEditWindow()->m_cDlgFuncList.GetHwnd() ){
		GetEditWindow()->m_cDlgFuncList.DoModeless(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			(LPARAM)m_pCommanderView,
			&cFuncInfoArr,
			poCaret.GetY2() + CLayoutInt(1),
			poCaret.GetX2() + CLayoutInt(1),
			nOutlineType,
			GetDocument()->m_cDocType.GetDocumentAttribute().m_bLineNumIsCRLF	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		);
	}else{
		/* �A�N�e�B�u�ɂ��� */
		GetEditWindow()->m_cDlgFuncList.Redraw( nOutlineType, &cFuncInfoArr, poCaret.GetY2() + 1, poCaret.GetX2() + 1 );
		ActivateFrameWindow( GetEditWindow()->m_cDlgFuncList.GetHwnd() );
	}

	// �_�C�A���O�^�C�g�����㏑��
	if( ! sTitleOverride.empty() ){
		GetEditWindow()->m_cDlgFuncList.SetWindowText( sTitleOverride.c_str() );
	}

	bIsProcessing = false;
	return TRUE;
}




/* �㉺�ɕ��� */	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
void CViewCommander::Command_SPLIT_V( void )
{
	GetEditWindow()->m_cSplitterWnd.VSplitOnOff();
	return;
}




/* ���E�ɕ��� */	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
void CViewCommander::Command_SPLIT_H( void )
{
	GetEditWindow()->m_cSplitterWnd.HSplitOnOff();
	return;
}




/* �c���ɕ��� */	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
void CViewCommander::Command_SPLIT_VH( void )
{
	GetEditWindow()->m_cSplitterWnd.VHSplitOnOff();
	return;
}




//From Here Nov. 25, 2000 JEPRO
/* �w���v�ڎ� */
void CViewCommander::Command_HELP_CONTENTS( void )
{
	ShowWinHelpContents( m_pCommanderView->GetHwnd(), CEditApp::Instance()->GetHelpFilePath() );	//	�ڎ���\������
	return;
}




/* �w���v�L�[���[�h���� */
void CViewCommander::Command_HELP_SEARCH( void )
{
	MyWinHelp( m_pCommanderView->GetHwnd(), CEditApp::Instance()->GetHelpFilePath(), HELP_KEY, (ULONG_PTR)_T("") );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	return;
}
//To Here Nov. 25, 2000




/*! �L�����b�g�ʒu�̒P�����������ON-OFF

	@date 2006.03.24 fon �V�K�쐬
*/
void CViewCommander::Command_ToggleKeySearch( void )
{	/* ���ʐݒ�_�C�A���O�̐ݒ���L�[���蓖�Ăł��؂�ւ�����悤�� */
	if( GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord ){
		GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord = FALSE;
	}else{
		GetDllShareData().m_Common.m_sSearch.m_bUseCaretKeyWord = TRUE;
	}
}




/* �R�}���h�ꗗ */
void CViewCommander::Command_MENU_ALLFUNC( void )
{

	UINT	uFlags;
	POINT	po;
	HMENU	hMenu;
	HMENU	hMenuPopUp;
	int		i;
	int		j;
	int		nId;

//	From Here Sept. 15, 2000 JEPRO
//	�T�u���j���[�A���Ɂu���̑��v�̃R�}���h�ɑ΂��ăX�e�[�^�X�o�[�ɕ\�������L�[�A�T�C�����
//	���j���[�ŉB��Ȃ��悤�ɉE�ɂ��炵��
//	(�{���͂��́u�R�}���h�ꗗ�v���j���[���_�C�A���O�ɕύX���o�[���܂�Ŏ��R�Ɉړ��ł���悤�ɂ�����)
//	po.x = 0;
	po.x = 540;
//	To Here Sept. 15, 2000 (Oct. 7, 2000 300��500; Nov. 3, 2000 500��540)
	po.y = 0;
	::ClientToScreen( m_pCommanderView->GetHwnd(), &po );

	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->GetMenuDrawer().ResetContents();

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = GetDocument()->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
//Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɕύX�������Ƃ�1��(�J�[�\���ړ��n)���O�ɃV�t�g���ꂽ(���̕ύX�ɂ���� i=1��i=0 �ƕύX)
	//	Oct. 3, 2001 genta
	for( i = 0; i < FuncLookup.GetCategoryCount(); i++ ){
		hMenuPopUp = ::CreatePopupMenu();
		for( j = 0; j < FuncLookup.GetItemCount(i); j++ ){
			//	Oct. 3, 2001 genta
			int code = FuncLookup.Pos2FuncCode( i, j, false );	// 2007.11.02 ryoji ���o�^�}�N����\���𖾎��w��
			if( code != 0 ){
				WCHAR	szLabel[300];
				FuncLookup.Pos2FuncName( i, j, szLabel, 256 );
				uFlags = MF_BYPOSITION | MF_STRING | MF_ENABLED;
				//	Oct. 3, 2001 genta
				pCEditWnd->GetMenuDrawer().MyAppendMenu( hMenuPopUp, uFlags, code, szLabel );
			}
		}
		//	Oct. 3, 2001 genta
		pCEditWnd->GetMenuDrawer().MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , FuncLookup.Category2Name(i) );
//		pCEditWnd->GetMenuDrawer().MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , nsFuncCode::ppszFuncKind[i] );
	}

	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetMainWindow()/*GetHwnd()*/,
		NULL
	);
	::DestroyMenu( hMenu );
	if( 0 != nId ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
//		HandleCommand( nFuncID, TRUE, 0, 0, 0, 0 );
		::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nId, 0 ), (LPARAM)NULL );
	}
	return;
}




/* �O���w���v�P */
void CViewCommander::Command_EXTHELP1( void )
{
retry:;
	if( CHelpManager().ExtWinHelpIsSet( GetDocument()->m_cDocType.GetDocumentType() ) == false){
//	if( 0 == wcslen( GetDllShareData().m_Common.m_szExtHelp1 ) ){
		ErrorBeep();
//From Here Sept. 15, 2000 JEPRO
//		[Esc]�L�[��[x]�{�^���ł����~�ł���悤�ɕύX
		if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
//To Here Sept. 15, 2000
			_T("�O���w���v�P���ݒ肳��Ă��܂���B\n�������ݒ肵�܂���?")
		) ){
			/* ���ʐݒ� �v���p�e�B�V�[�g */
			if( !CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHELP1*/ ) ){
				return;
			}
			goto retry;
		}
		//	Jun. 15, 2000 genta
		else{
			return;
		}
	}

	CNativeW		cmemCurText;
	const TCHAR*	helpfile = CHelpManager().GetExtWinHelp( GetDocument()->m_cDocType.GetDocumentType() );

	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	m_pCommanderView->GetCurrentTextForSearch( cmemCurText );
	if( _IS_REL_PATH( helpfile ) ){
		// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
		// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		TCHAR path[_MAX_PATH];
		GetInidirOrExedir( path, helpfile );
		::WinHelp( m_pCommanderView->m_hwndParent, path, HELP_KEY, (ULONG_PTR)cmemCurText.GetStringPtr() );
		return;
	}
	::WinHelp( m_pCommanderView->m_hwndParent, helpfile , HELP_KEY, (ULONG_PTR)cmemCurText.GetStringPtr() );
	return;
}




/*!
	�O��HTML�w���v
	
	@param helpfile [in] HTML�w���v�t�@�C�����DNULL�̂Ƃ��̓^�C�v�ʂɐݒ肳�ꂽ�t�@�C���D
	@param kwd [in] �����L�[���[�h�DNULL�̂Ƃ��̓J�[�\���ʒuor�I�����ꂽ���[�h
	@date 2002.07.05 genta �C�ӂ̃t�@�C���E�L�[���[�h�̎w�肪�ł���悤�����ǉ�
*/
void CViewCommander::Command_EXTHTMLHELP( const WCHAR* _helpfile, const WCHAR* kwd )
{
	const TCHAR* helpfile = to_tchar(_helpfile);

	HWND		hwndHtmlHelp;
	int			nLen;

	DBPRINT_A("helpfile=%ls\n",helpfile);

	//	From Here Jul. 5, 2002 genta
	const TCHAR *filename = NULL;
	if ( helpfile == NULL || helpfile[0] == _T('\0') ){
		while( !CHelpManager().ExtHTMLHelpIsSet( GetDocument()->m_cDocType.GetDocumentType()) ){
			ErrorBeep();
	//	From Here Sept. 15, 2000 JEPRO
	//		[Esc]�L�[��[x]�{�^���ł����~�ł���悤�ɕύX
			if( IDYES != ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
	//	To Here Sept. 15, 2000
				_T("�O��HTML�w���v���ݒ肳��Ă��܂���B\n�������ݒ肵�܂���?")
			) ){
				return;
			}
			/* ���ʐݒ� �v���p�e�B�V�[�g */
			if( !CEditApp::Instance()->m_pcPropertyManager->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHTMLHELP*/ ) ){
				return;
			}
		}
		filename = CHelpManager().GetExtHTMLHelp( GetDocument()->m_cDocType.GetDocumentType() );
	}
	else {
		filename = helpfile;
	}
	//	To Here Jul. 5, 2002 genta

	//	Jul. 5, 2002 genta
	//	�L�[���[�h�̊O���w����\��
	CNativeW	cmemCurText;
	if( kwd != NULL && kwd[0] != _T('\0') ){
		cmemCurText.SetString( kwd );
	}
	else {
		/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
		m_pCommanderView->GetCurrentTextForSearch( cmemCurText );
	}

	/* HtmlHelp�r���[�A�͂ЂƂ� */
	if( CHelpManager().HTMLHelpIsSingle( GetDocument()->m_cDocType.GetDocumentType() ) ){
		// �^�X�N�g���C�̃v���Z�X��HtmlHelp���N��������
		// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
		// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		TCHAR* pWork=GetDllShareData().m_sWorkBuffer.GetWorkBuffer<TCHAR>();
		if( _IS_REL_PATH( filename ) ){
			GetInidirOrExedir( pWork, filename );
		}else{
			_tcscpy( pWork, filename ); //	Jul. 5, 2002 genta
		}
		nLen = _tcslen( pWork );
		_tcscpy( &pWork[nLen + 1], cmemCurText.GetStringT() );
		hwndHtmlHelp = (HWND)::SendMessageAny(
			GetDllShareData().m_sHandles.m_hwndTray,
			MYWM_HTMLHELP,
			(WPARAM)GetMainWindow(),
			0
		);
	}
	else{
		/* ������HtmlHelp���N�������� */
		HH_AKLINK	link;
		link.cbStruct = sizeof( link ) ;
		link.fReserved = FALSE ;
		link.pszKeywords = cmemCurText.GetStringT();
		link.pszUrl = NULL;
		link.pszMsgText = NULL;
		link.pszMsgTitle = NULL;
		link.pszWindow = NULL;
		link.fIndexOnFail = TRUE;

		// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
		// 2007.05.21 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
		if( _IS_REL_PATH( filename ) ){
			TCHAR path[_MAX_PATH];
			GetInidirOrExedir( path, filename );
			//	Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().m_sHandles.m_hwndTray*/,
				path, //	Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}else{
			//	Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*GetDllShareData().m_sHandles.m_hwndTray*/,
				filename, //	Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}
	}

	//	Jul. 6, 2001 genta hwndHtmlHelp�̃`�F�b�N��ǉ�
	if( hwndHtmlHelp != NULL ){
		::BringWindowToTop( hwndHtmlHelp );
	}

	return;
}




//From Here Dec. 25, 2000 JEPRO
/* �o�[�W������� */
void CViewCommander::Command_ABOUT( void )
{
	CDlgAbout cDlgAbout;
	cDlgAbout.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd() );
	return;
}
//To Here Dec. 25, 2000




/* �E�N���b�N���j���[ */
void CViewCommander::Command_MENU_RBUTTON( void )
{
	int			nId;
	int			nLength;
//	HGLOBAL		hgClip;
//	char*		pszClip;
	int			i;
	/* �|�b�v�A�b�v���j���[(�E�N���b�N) */
	nId = m_pCommanderView->CreatePopUpMenu_R();
	if( 0 == nId ){
		return;
	}
	switch( nId ){
	case IDM_COPYDICINFO:
		const TCHAR*	pszStr;
		pszStr = m_pCommanderView->m_cTipWnd.m_cInfo.GetStringPtr( &nLength );

		TCHAR*		pszWork;
		pszWork = new TCHAR[nLength + 1];
		auto_memcpy( pszWork, pszStr, nLength );
		pszWork[nLength] = _T('\0');

		// �����ڂƓ����悤�ɁA\n �� CR+LF�֕ϊ�����
		for( i = 0; i < nLength ; ++i){
			if( pszWork[i] == _T('\\') && pszWork[i + 1] == _T('n')){
				pszWork[i] =     WCODE::CR;
				pszWork[i + 1] = WCODE::LF;
			}
		}
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		m_pCommanderView->MySetClipboardData( pszWork, nLength, false );
		delete[] pszWork;

		break;

	case IDM_JUMPDICT:
		/* �L�[���[�h�����t�@�C�����J�� */
		if(GetDocument()->m_cDocType.GetDocumentAttribute().m_bUseKeyWordHelp){		/* �L�[���[�h�����Z���N�g���g�p���� */	// 2006.04.10 fon
			//	Feb. 17, 2007 genta ���΃p�X�����s�t�@�C����ŊJ���悤��
			m_pCommanderView->TagJumpSub(
				GetDocument()->m_cDocType.GetDocumentType()->m_KeyHelpArr[m_pCommanderView->m_cTipWnd.m_nSearchDict].m_szPath,
				CMyPoint(1, m_pCommanderView->m_cTipWnd.m_nSearchLine),
				0,
				true
			);
		}
		break;

	default:
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
//		HandleCommand( nId, TRUE, 0, 0, 0, 0 );
		::PostMessageCmd( GetMainWindow(), WM_COMMAND, MAKELONG( nId, 0 ),  (LPARAM)NULL );
		break;
	}
	return;
}




/* �C���f���g ver1 */
void CViewCommander::Command_INDENT( wchar_t wcChar, EIndentType eIndent )
{
	using namespace WCODE;

#if 1	// ���������c���ΑI�𕝃[�����ő�ɂ���i�]���݊������j�B�����Ă� Command_INDENT() ver0 ���K�؂ɓ��삷��悤�ɕύX���ꂽ�̂ŁA�폜���Ă����ɕs�s���ɂ͂Ȃ�Ȃ��B
	// From Here 2001.12.03 hor
	/* SPACEorTAB�C�����f���g�ŋ�`�I�������[���̎��͑I��͈͂��ő�ɂ��� */
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if( INDENT_NONE != eIndent && m_pCommanderView->GetSelectionInfo().IsBoxSelecting() && GetSelect().GetFrom().x==GetSelect().GetTo().x ){
		GetSelect().SetToX( GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
		m_pCommanderView->RedrawAll();
		return;
	}
	// To Here 2001.12.03 hor
#endif
	Command_INDENT( &wcChar, CLogicInt(1), eIndent );
	return;
}




/* �C���f���g ver0 */
/*
	�I�����ꂽ�e�s�͈̔͂̒��O�ɁA�^����ꂽ������( pData )��}������B
	@param eIndent �C���f���g�̎��
*/
void CViewCommander::Command_INDENT( const wchar_t* const pData, const CLogicInt nDataLen, EIndentType eIndent )
{
	if( nDataLen <= 0 ) return;

	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );
	CLayoutRange sSelectOld;		//�͈͑I��
	CLayoutPoint ptInserted;		//�}����̑}���ʒu
	const struct {
		bool operator()( const wchar_t ch ) const
		{ return ch == WCODE::SPACE || ch == WCODE::TAB; }
	} IsIndentChar;
	struct SSoftTabData {
		SSoftTabData( CLayoutInt nTab ) : m_szTab(NULL), m_nTab((Int)nTab) {}
		~SSoftTabData() { delete []m_szTab; }
		operator const wchar_t* ()
		{
			if( !m_szTab ){
				m_szTab = new wchar_t[m_nTab];
				wmemset( m_szTab, WCODE::SPACE, m_nTab );
			}
			return m_szTab;
		}
		int Len( CLayoutInt nCol ) { return m_nTab - ((Int)nCol % m_nTab); }
		wchar_t* m_szTab;
		int m_nTab;
	} stabData( GetDocument()->m_cLayoutMgr.GetTabSpace() );

	const bool bSoftTab = ( eIndent == INDENT_TAB && GetDocument()->m_cDocType.GetDocumentAttribute().m_bInsSpace );
	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
		if( INDENT_NONE != eIndent && !bSoftTab ){
			// ����`�I���ł͂Ȃ��̂� Command_WCHAR ����Ăі߂������悤�Ȃ��Ƃ͂Ȃ�
			Command_WCHAR( pData[0] );	// 1��������
		}
		else{
			// ����`�I���ł͂Ȃ��̂ł����֗���͎̂��ۂɂ̓\�t�g�^�u�̂Ƃ�����
			if( bSoftTab && !m_pCommanderView->IsInsMode() ){
				DelCharForOverwrite();
			}
			m_pCommanderView->InsertData_CEditView(
				GetCaret().GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(GetCaret().GetCaretLayoutPos().GetX2()),
				&ptInserted,
				true
			);
			GetCaret().MoveCursor( ptInserted, TRUE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
		return;
	}
	m_pCommanderView->SetDrawSwitch(false);	// 2002.01.25 hor
	/* ��`�͈͑I�𒆂� */
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		// From Here 2001.12.03 hor
		/* �㏑���[�h�̂Ƃ��͑I��͈͍폜 */
		if( ! m_pCommanderView->IsInsMode() /* Oct. 2, 2005 genta */){
			sSelectOld = GetSelect();
			m_pCommanderView->DeleteData( FALSE );
			GetSelect() = sSelectOld;
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);
		}
		// To Here 2001.12.03 hor

		/* 2�_��Ίp�Ƃ����`�����߂� */
		CLayoutRange rcSel;
		TwoPointToRange(
			&rcSel,
			GetSelect().GetFrom(),	// �͈͑I���J�n
			GetSelect().GetTo()		// �͈͑I���I��
		);
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( FALSE/*TRUE 2002.01.25 hor*/ );

		/*
			�����𒼑O�ɑ}�����ꂽ�������A����ɂ�茳�̈ʒu����ǂꂾ�����ɂ��ꂽ���B
			����ɏ]����`�I��͈͂����ɂ��炷�B
		*/
		CLayoutInt minOffset( -1 );
		/*
			���S�p�����̍����̌������ɂ���
			(1) eIndent == INDENT_TAB �̂Ƃ�
				�I��͈͂��^�u���E�ɂ���Ƃ��Ƀ^�u����͂���ƁA�S�p�����̑O�����I��͈͂���
				�͂ݏo���Ă���s�Ƃ����łȂ��s�Ń^�u�̕����A1����ݒ肳�ꂽ�ő�܂łƑ傫���قȂ�A
				�ŏ��ɑI������Ă���������I��͈͓��ɂƂǂ߂Ă������Ƃ��ł��Ȃ��Ȃ�B
				�ŏ��͋�`�I��͈͓��ɂ��ꂢ�Ɏ��܂��Ă���s�ɂ̓^�u��}�������A������Ƃ����͂�
				�o���Ă���s�ɂ����^�u��}�����邱�ƂƂ��A����ł͂ǂ̍s�ɂ��^�u���}������Ȃ�
				�Ƃ킩�����Ƃ��͂�蒼���ă^�u��}������B
			(2) eIndent == INDENT_SPACE �̂Ƃ��i���]���݊��I�ȓ���j
				��1�őI�����Ă���ꍇ�̂ݑS�p�����̍���������������B
				�ŏ��͋�`�I��͈͓��ɂ��ꂢ�Ɏ��܂��Ă���s�ɂ̓X�y�[�X��}�������A������Ƃ����͂�
				�o���Ă���s�ɂ����X�y�[�X��}�����邱�ƂƂ��A����ł͂ǂ̍s�ɂ��X�y�[�X���}������Ȃ�
				�Ƃ킩�����Ƃ��͂�蒼���ăX�y�[�X��}������B
		*/
		bool alignFullWidthChar = eIndent == INDENT_TAB && 0 == rcSel.GetFrom().x % this->GetDocument()->m_cLayoutMgr.GetTabSpace();
#if 1	// ���������c���ΑI��1��SPACE�C���f���g�őS�p�����𑵂���@�\(2)���ǉ������B
		alignFullWidthChar = alignFullWidthChar || (eIndent == INDENT_SPACE && 1 == rcSel.GetTo().x - rcSel.GetFrom().x);
#endif
		for( bool insertionWasDone = false; ; alignFullWidthChar = false ) {
			minOffset = CLayoutInt( -1 );
			for( CLayoutInt nLineNum = rcSel.GetFrom().y; nLineNum <= rcSel.GetTo().y; ++nLineNum ){
				const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );
				//	Nov. 6, 2002 genta NULL�`�F�b�N�ǉ�
				//	���ꂪ�Ȃ���EOF�s���܂ދ�`�I�𒆂̕�������͂ŗ�����
				CLogicInt nIdxFrom, nIdxTo;
				CLayoutInt xLayoutFrom, xLayoutTo;
				bool reachEndOfLayout = false;
				if( pcLayout ) {
					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					const struct {
						CLayoutInt keta;
						CLogicInt* outLogicX;
						CLayoutInt* outLayoutX;
					} sortedKetas[] = {
						{ rcSel.GetFrom().x, &nIdxFrom, &xLayoutFrom },
						{ rcSel.GetTo().x, &nIdxTo, &xLayoutTo },
						{ CLayoutInt(-1), 0, 0 }
					};
					CMemoryIterator it( pcLayout, this->GetDocument()->m_cLayoutMgr.GetTabSpace() );
					for( int i = 0; 0 <= sortedKetas[i].keta; ++i ) {
						for( ; ! it.end(); it.addDelta() ) {
							if( sortedKetas[i].keta == it.getColumn() ) {
								break;
							}
							it.scanNext();
							if( sortedKetas[i].keta < it.getColumn() + it.getColumnDelta() ) {
								break;
							}
						}
						*sortedKetas[i].outLogicX = it.getIndex();
						*sortedKetas[i].outLayoutX = it.getColumn();
					}
					reachEndOfLayout = it.end();
				}else{
					nIdxFrom = nIdxTo = CLogicInt(0);
					xLayoutFrom = xLayoutTo = CLayoutInt(0);
					reachEndOfLayout = true;
				}
				const bool emptyLine = ! pcLayout || 0 == pcLayout->GetLengthWithoutEOL();
				const bool selectionIsOutOfLine = reachEndOfLayout && (
					(pcLayout && pcLayout->GetLayoutEol() != EOL_NONE) ? xLayoutFrom == xLayoutTo : xLayoutTo < rcSel.GetFrom().x
				);

				// ���͕����̑}���ʒu
				const CLayoutPoint ptInsert( selectionIsOutOfLine ? rcSel.GetFrom().x : xLayoutFrom, nLineNum );

				/* TAB��X�y�[�X�C���f���g�̎� */
				if( INDENT_NONE != eIndent ) {
					if( emptyLine || selectionIsOutOfLine ) {
						continue; // �C���f���g�������C���f���g�Ώۂ����݂��Ȃ�����(���s�����̌����s)�ɑ}�����Ȃ��B
					}
					/*
						���͂��C���f���g�p�̕����̂Ƃ��A��������œ��͕�����}�����Ȃ����Ƃ�
						�C���f���g�𑵂��邱�Ƃ��ł���B
						http://sakura-editor.sourceforge.net/cgi-bin/cyclamen/cyclamen.cgi?log=dev&v=4103
					*/
					if( nIdxFrom == nIdxTo // ��`�I��͈͂̉E�[�܂łɔ͈͂̍��[�ɂ��镶���̖������܂܂�Ă��炸�A
						&& ! selectionIsOutOfLine && pcLayout && IsIndentChar( pcLayout->GetPtr()[nIdxFrom] ) // ���́A�����̊܂܂�Ă��Ȃ��������C���f���g�����ł���A
						&& rcSel.GetFrom().x < rcSel.GetTo().x // ��0��`�I���ł͂Ȃ�(<<�݊����ƃC���f���g�����}���̎g������̂��߂ɏ��O����)�Ƃ��B
					) {
						continue;
					}
					/*
						�S�p�����̍����̌�����
					*/
					if( alignFullWidthChar
						&& (ptInsert.x == rcSel.GetFrom().x || (pcLayout && IsIndentChar( pcLayout->GetPtr()[nIdxFrom] )))
					) {	// �����̍������͈͂ɂ҂�������܂��Ă���
						minOffset = CLayoutInt(0);
						continue;
					}
				}

				/* ���݈ʒu�Ƀf�[�^��}�� */
				m_pCommanderView->InsertData_CEditView(
					ptInsert,
					!bSoftTab? pData: stabData,
					!bSoftTab? nDataLen: stabData.Len(ptInsert.x),
					&ptInserted,
					false
				);
				insertionWasDone = true;
				minOffset = std::min(
					0 <= minOffset ? minOffset : this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas(),
					ptInsert.x <= ptInserted.x ? ptInserted.x - ptInsert.x : std::max( CLayoutInt(0), this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas() - ptInsert.x)
				);

				GetCaret().MoveCursor( ptInserted, FALSE );
				GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			}
			if( insertionWasDone || !alignFullWidthChar ) {
				break; // ���[�v�̕K�v�͂Ȃ��B(1.�����̑}�����s��ꂽ����B2.�����ł͂Ȃ��������̑}�����T���������ł͂Ȃ�����)
			}
		}

		// �}�����ꂽ�����̕������I��͈͂����ɂ��炵�ArcSel�ɃZ�b�g����B
		if( 0 < minOffset ) {
			rcSel.GetFromPointer()->x = std::min( rcSel.GetFrom().x + minOffset, this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
			rcSel.GetToPointer()->x = std::min( rcSel.GetTo().x + minOffset, this->GetDocument()->m_cLayoutMgr.GetMaxLineKetas() );
		}

		/* �J�[�\�����ړ� */
		GetCaret().MoveCursor( rcSel.GetFrom(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			/* ����̒ǉ� */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
		GetSelect().SetFrom(rcSel.GetFrom());	//�͈͑I���J�n�ʒu
		GetSelect().SetTo(rcSel.GetTo());		//�͈͑I���I���ʒu
		m_pCommanderView->GetSelectionInfo().SetBoxSelect(true);
	}
	else if( GetSelect().IsLineOne() ){	// �ʏ�I��(1�s��)
		if( INDENT_NONE != eIndent && !bSoftTab ){
			// ����`�I���ł͂Ȃ��̂� Command_WCHAR ����Ăі߂������悤�Ȃ��Ƃ͂Ȃ�
			Command_WCHAR( pData[0] );	// 1��������
		}
		else{
			// ����`�I���ł͂Ȃ��̂ł����֗���͎̂��ۂɂ̓\�t�g�^�u�̂Ƃ�����
			m_pCommanderView->DeleteData( false );
			m_pCommanderView->InsertData_CEditView(
				GetCaret().GetCaretLayoutPos(),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(GetCaret().GetCaretLayoutPos().GetX2()),
				&ptInserted,
				false
			);
			GetCaret().MoveCursor( ptInserted, TRUE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		}
	}
	else{	// �ʏ�I��(�����s)
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelect().GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelect().GetTo().y  ));
		if( GetSelect().GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}

		// ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( FALSE );

		for( CLayoutInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().GetY2(); i++ ){
			CLayoutInt nLineCountPrev = GetDocument()->m_cLayoutMgr.GetLineCount();
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( i );
			if( NULL == pcLayout ||						//	�e�L�X�g������EOL�̍s�͖���
				pcLayout->GetLogicOffset() > 0 ||				//	�܂�Ԃ��s�͖���
				pcLayout->GetLengthWithoutEOL() == 0 ){	//	���s�݂̂̍s�͖�������B
				continue;
			}

			/* �J�[�\�����ړ� */
			GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), i), FALSE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			/* ���݈ʒu�Ƀf�[�^��}�� */
			m_pCommanderView->InsertData_CEditView(
				CLayoutPoint(CLayoutInt(0),i),
				!bSoftTab? pData: stabData,
				!bSoftTab? nDataLen: stabData.Len(CLayoutInt(0)),
				&ptInserted,
				false
			);
			/* �J�[�\�����ړ� */
			GetCaret().MoveCursor( ptInserted, FALSE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();

			if ( nLineCountPrev != GetDocument()->m_cLayoutMgr.GetLineCount() ){
				//	�s�����ω�����!!
				sSelectOld.GetToPointer()->y += GetDocument()->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}

		GetSelect() = sSelectOld;

		// From Here 2001.12.03 hor
		GetCaret().MoveCursor( GetSelect().GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
		// To Here 2001.12.03 hor
	}
	/* �ĕ`�� */
	m_pCommanderView->SetDrawSwitch(true);	// 2002.01.25 hor
	m_pCommanderView->RedrawAll();			// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
	return;
}




/* �t�C���f���g */
void CViewCommander::Command_UNINDENT( wchar_t wcChar )
{
	//	Aug. 9, 2003 genta
	//	�I������Ă��Ȃ��ꍇ�ɋt�C���f���g�����ꍇ��
	//	���Ӄ��b�Z�[�W���o��
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* wchar_t1���̕������� */
		Command_WCHAR( wcChar );	//	2003.10.09 zenryaku�x�����o�����C����͈ȑO�̂܂܂ɂ��� 
		m_pCommanderView->SendStatusMessage(_T("���t�C���f���g�͑I�����̂�"));
		return;
	}

	//�����v
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	CMemory		cmemBuf;

	/* ��`�͈͑I�𒆂� */
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		ErrorBeep();
//**********************************************
//	 ���^�t�C���f���g�ɂ��ẮA�ۗ��Ƃ��� (1998.10.22)
//**********************************************
	}
	else{
		GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

		CLayoutRange sSelectOld;	//�͈͑I��
		sSelectOld.SetFrom(CLayoutPoint(CLayoutInt(0),GetSelect().GetFrom().y));
		sSelectOld.SetTo  (CLayoutPoint(CLayoutInt(0),GetSelect().GetTo().y  ));
		if( GetSelect().GetTo().x > 0 ){
			sSelectOld.GetToPointer()->y++;
		}

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pCommanderView->GetSelectionInfo().DisableSelectArea( FALSE );

		CLogicInt		nDelLen;
		for( CLayoutInt i = sSelectOld.GetFrom().GetY2(); i < sSelectOld.GetTo().GetY2(); i++ ){
			CLayoutInt nLineCountPrev = GetDocument()->m_cLayoutMgr.GetLineCount();

			const CLayout*	pcLayout;
			CLogicInt		nLineLen;
			const wchar_t*	pLine = GetDocument()->m_cLayoutMgr.GetLineStr( i, &nLineLen, &pcLayout );
			if( NULL == pcLayout || pcLayout->GetLogicOffset() > 0 ){ //�܂�Ԃ��ȍ~�̍s�̓C���f���g�������s��Ȃ�
				continue;
			}

			if( WCODE::TAB == wcChar ){
				if( pLine[0] == wcChar ){
					nDelLen = CLogicInt(1);
				}
				else{
					//����锼�p�X�y�[�X�� (1�`�^�u����) -> nDelLen
					CLogicInt i;
					CLogicInt nTabSpaces = CLogicInt((Int)GetDocument()->m_cLayoutMgr.GetTabSpace());
					for( i = CLogicInt(0); i < nLineLen; i++ ){
						if( WCODE::SPACE != pLine[i] ){
							break;
						}
						//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
						if( i >= nTabSpaces ){
							break;
						}
					}
					if( 0 == i ){
						continue;
					}
					nDelLen = i;
				}
			}
			else{
				if( pLine[0] != wcChar ){
					continue;
				}
				nDelLen = CLogicInt(1);
			}

			/* �J�[�\�����ړ� */
			GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), i), FALSE );
			GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
			

			CNativeW	pcMemDeleted;
			/* �w��ʒu�̎w�蒷�f�[�^�폜 */
			m_pCommanderView->DeleteData2(
				CLayoutPoint(CLayoutInt(0),i),
				nDelLen,	// 2001.12.03 hor
				&pcMemDeleted
			);
			if ( nLineCountPrev != GetDocument()->m_cLayoutMgr.GetLineCount() ){
				//	�s�����ω�����!!
				sSelectOld.GetToPointer()->y += GetDocument()->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}
		GetSelect() = sSelectOld;	//�͈͑I��

		// From Here 2001.12.03 hor
		GetCaret().MoveCursor( GetSelect().GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
		if( !m_pCommanderView->m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos(),	// ����O�̃L�����b�g�ʒu
					GetCaret().GetCaretLogicPos()	// �����̃L�����b�g�ʒu
				)
			);
		}
		// To Here 2001.12.03 hor
	}

	/* �ĕ`�� */
	m_pCommanderView->RedrawAll();	// 2002.01.25 hor	// 2009.07.25 ryoji Redraw()->RedrawAll()
}

/* �Ō�Ƀe�L�X�g��ǉ� */
void CViewCommander::Command_ADDTAIL(
	const wchar_t*	pszData,	//!< �ǉ�����e�L�X�g
	int				nDataLen	//!< �ǉ�����e�L�X�g�̒����B�����P�ʁB-1���w�肷��ƁA�e�L�X�g�I�[�܂ŁB
)
{
	//�e�L�X�g�������v�Z
	if(nDataLen==-1 && pszData!=NULL)nDataLen=wcslen(pszData);

	GetDocument()->m_cDocEditor.SetModified(true,true);	//	Jan. 22, 2002 genta

	/*�t�@�C���̍Ō�Ɉړ� */
	Command_GOFILEEND( FALSE );

	/* ���݈ʒu�Ƀf�[�^��}�� */
	CLayoutPoint ptLayoutNew;	// �}�����ꂽ�����̎��̈ʒu
	m_pCommanderView->InsertData_CEditView(
		GetCaret().GetCaretLayoutPos(),
		pszData,
		nDataLen,
		&ptLayoutNew,
		true
	);

	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
	// Sep. 2, 2002 ���Ȃӂ� �A���_�[���C���̕\�����c���Ă��܂������C��
	GetCaret().MoveCursor( ptLayoutNew, TRUE );
	GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();
}




/*! �^�O�W�����v

	@param bClose [in] true:���E�B���h�E�����

	@date 2003.04.03 genta ���E�B���h�E����邩�ǂ����̈�����ǉ�
	@date 2004.05.13 Moca �s���ʒu�̎w�肪�����ꍇ�́A�s�����ړ����Ȃ�
*/
bool CViewCommander::Command_TAGJUMP( bool bClose )
{
	//	2004.05.13 Moca �����l��1�ł͂Ȃ����̈ʒu���p������悤��
	// 0�ȉ��͖��w�舵���B(1�J�n)
	int			nJumpToLine;
	int			nJumpToColm;
	nJumpToLine = 0;
	nJumpToColm = 0;

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CLogicPoint ptXY;
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&ptXY
	);
	/* ���ݍs�̃f�[�^���擾 */
	CLogicInt		nLineLen;
	const wchar_t*	pLine;
	pLine = GetDocument()->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen);
	if( NULL == pLine ){
		goto can_not_tagjump_end;
	}

	//�t�@�C�����o�b�t�@
	wchar_t		szJumpToFile[1024];
	int			nBgn;
	int			nPathLen;
	wmemset( szJumpToFile, 0, _countof(szJumpToFile) );

	/* WZ���̃^�O���X�g�� */
	if( 0 == wmemcmp( pLine, L"��\"", 2 ) ){
		if( IsFilePath( &pLine[2], &nBgn, &nPathLen ) ){
			wmemcpy( szJumpToFile, &pLine[2 + nBgn], nPathLen );
			GetLineColm( &pLine[2] + nPathLen, &nJumpToLine, &nJumpToColm );
		}
		else{
			goto can_not_tagjump;
		}
	}
	else if(0 == wmemcmp( pLine, L"�E", 1 )){
		GetLineColm( &pLine[1], &nJumpToLine, &nJumpToColm );
		ptXY.y--;

		for( ; 0 <= ptXY.y; ptXY.y-- ){
			pLine = GetDocument()->m_cDocLineMgr.GetLine(ptXY.GetY2())->GetDocLineStrWithEOL(&nLineLen);
			if( NULL == pLine ){
				goto can_not_tagjump;
			}
			if( 0 == wmemcmp( pLine, L"�E", 1 ) ){
				continue;
			}
			else if( 0 == wmemcmp( pLine, L"��\"", 2 ) ){
				if( IsFilePath( &pLine[2], &nBgn, &nPathLen ) ){
					wmemcpy( szJumpToFile, &pLine[2 + nBgn], nPathLen );
					break;
				}
				else{
					goto can_not_tagjump;
				}
			}
			else{
				goto can_not_tagjump;
			}
		}
	}
	else{
		//@@@ 2001.12.31 YAZAKI
		const wchar_t *p = pLine;
		const wchar_t *p_end = p + nLineLen;

		//	From Here Aug. 27, 2001 genta
		//	Borland �`���̃��b�Z�[�W�����TAG JUMP
		while( p < p_end ){
			//	skip space
			for( ; p < p_end && ( *p == L' ' || *p == L'\t' || *p == L'\n' ); ++p )
				;
			if( p >= p_end )
				break;
		
			//	Check Path
			if( IsFilePath( p, &nBgn, &nPathLen ) ){
				wmemcpy( szJumpToFile, &p[nBgn], nPathLen );
				GetLineColm( &p[nBgn + nPathLen], &nJumpToLine, &nJumpToColm );
				break;
			}
			//	Jan. 04, 2001 genta Directory��ΏۊO�ɂ����̂ŕ�����ɂ͏_��ɑΉ�
			//	break;	//@@@ 2001.12.31 YAZAKI �uworking ...�v���ɑΏ�
			//	skip non-space
			for( ; p < p_end && ( *p != L' ' && *p != L'\t' ); ++p )
				;
		}
		if( szJumpToFile[0] == L'\0' ){
			if( !Command_TagJumpByTagsFile() )	//@@@ 2003.04.13
				goto can_not_tagjump;
			return true;
		}
		//	From Here Aug. 27, 2001 genta
	}

	//	Apr. 21, 2003 genta bClose�ǉ�
	if( !m_pCommanderView->TagJumpSub( to_tchar(szJumpToFile), CMyPoint(nJumpToColm, nJumpToLine), bClose ) )	//@@@ 2003.04.13
		goto can_not_tagjump;

	return true;

can_not_tagjump:;
can_not_tagjump_end:;
	m_pCommanderView->SendStatusMessage(_T("�^�O�W�����v�ł��܂���"));	//@@@ 2003.04.13
	return false;
}




/* �^�O�W�����v�o�b�N */
void CViewCommander::Command_TAGJUMPBACK( void )
{
// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	TagJump tagJump;

	/* �^�O�W�����v���̎Q�� */
	if( !CTagJumpManager().PopTagJump(&tagJump) || !IsSakuraMainWindow(tagJump.hwndReferer) ){
		m_pCommanderView->SendStatusMessage(_T("�^�O�W�����v�o�b�N�ł��܂���"));
		// 2004.07.10 Moca m_TagJumpNum��0�ɂ��Ȃ��Ă������Ǝv��
		// GetDllShareData().m_TagJumpNum = 0;
		return;
	}

	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( tagJump.hwndReferer );

	/* �J�[�\�����ړ������� */
	memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &(tagJump.point), sizeof( tagJump.point ) );
	::SendMessageAny( tagJump.hwndReferer, MYWM_SETCARETPOS, 0, 0 );

	return;
}

/*
	�_�C���N�g�^�O�W�����v

	@author	MIK
	@date	2003.04.13	�V�K�쐬
	@date	2003.05.12	�t�H���_�K�w���l�����ĒT��
*/
bool CViewCommander::Command_TagJumpByTagsFile( void )
{
	CNativeW	cmemKey;
	int		i;
	TCHAR	szCurrentPath[1024];	//�J�����g�t�H���_
	TCHAR	szTagFile[1024];		//�^�O�t�@�C��
	TCHAR	szLineData[1024];		//�s�o�b�t�@
	TCHAR	s[5][1024];
	int		n2;
	int		nRet;
	int		nMatch;						//��v��
	CDlgTagJumpList	cDlgTagJumpList;	//�^�O�W�����v���X�g
	FILE	*fp;
	bool	bNoTag = true;
	int		nLoop;

	//���݃J�[�\���ʒu�̃L�[���擾����B
	m_pCommanderView->GetCurrentTextForSearch( cmemKey );
	if( 0 == cmemKey.GetStringLength() ) return false;	//�L�[���Ȃ��Ȃ�I���

	if( ! GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ) return false;

	// �t�@�C�����ɉ����ĒT���񐔂����肷��
	_tcscpy( szCurrentPath, GetDocument()->m_cDocFile.GetFilePath() );
	nLoop = CalcDirectoryDepth( szCurrentPath );

	if( nLoop <  0 ) nLoop =  0;
	if( nLoop > (_MAX_PATH/2) ) nLoop = (_MAX_PATH/2);	//\A\B\C...�̂悤�ȂƂ�1�t�H���_��2���������̂�...

		//�p�X���̂ݎ��o���B
		cDlgTagJumpList.SetFileName( szCurrentPath );
		szCurrentPath[ _tcslen( szCurrentPath ) - _tcslen( GetDocument()->m_cDocFile.GetFileName() ) ] = _T('\0');

		for( i = 0; i <= nLoop; i++ )
		{
			//�^�O�t�@�C�������쐬����B
			auto_sprintf( szTagFile, _T("%ts%ls"), szCurrentPath, TAG_FILENAME );

			//�^�O�t�@�C�����J���B
			fp = _tfopen( szTagFile, _T("r") );
			if( fp )
			{
				bNoTag = false;
				nMatch = 0;
				while( _fgetts( szLineData, _countof( szLineData ), fp ) )
				{
					if( szLineData[0] <= _T('!') ) goto next_line;	//�R�����g�Ȃ�X�L�b�v
					//chop( szLineData );

					s[0][0] = s[1][0] = s[2][0] = s[3][0] = s[4][0] = _T('\0');
					n2 = 0;
					//	2004.06.04 Moca �t�@�C����/�p�X�ɃX�y�[�X���܂܂�Ă���Ƃ���
					//	�_�C���N�g�^�O�W�����v�Ɏ��s���Ă���
					//	sscanf ��%[^\t\r\n] �ŃX�y�[�X��ǂ݂Ƃ�悤�ɕύX
					//	@@ 2005.03.31 MIK TAG_FORMAT�萔��
					nRet = _stscanf(
						szLineData, 
						TAG_FORMAT,	//tags�t�H�[�}�b�g
						s[0], s[1], &n2, s[3], s[4]
					);
					if( nRet < 4 ) goto next_line;
					if( n2 <= 0 ) goto next_line;	//�s�ԍ��s��(-excmd=n���w�肳��ĂȂ�����)

					if( 0 != wcscmp( to_wchar(s[0]), cmemKey.GetStringPtr() ) ) goto next_line;

					//	@@ 2005.03.31 MIK �K�w�p�����[�^�ǉ�
					cDlgTagJumpList.AddParam( s[0], s[1], n2, s[3], s[4], i );
					nMatch++;
					continue;

next_line:
					if( nMatch ) break;
				}

				//�t�@�C�������B
				fclose( fp );

				//��������ΑI�����Ă��炤�B
				if( nMatch > 1 )
				{
					if( ! cDlgTagJumpList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)0 ) ) 
					{
						nMatch = 0;
						return true;	//�L�����Z��
					}
				}

				//�^�O�W�����v����B
				if( nMatch > 0 )
				{
					//	@@ 2005.03.31 MIK �K�w�p�����[�^�ǉ�
					int depth;
					if( false == cDlgTagJumpList.GetSelectedParam( s[0], s[1], &n2, s[3], s[4], &depth ) )
					{
						return false;
					}

					/*
					 * s[0] �L�[
					 * s[1] �t�@�C����
					 * n2   �s�ԍ�
					 * s[3] �^�C�v
					 * s[4] �R�����g
					 * depth (�����̂ڂ�)�K�w��
					 */

					//���S�p�X�����쐬����B
					TCHAR	*p;
					p = s[1];
					if( p[0] == _T('\\') )	//�h���C�u�Ȃ���΃p�X���H
					{
						//	2003.09.20 Moca �p�X����
						if( p[1] == _T('\\') )	//�l�b�g���[�N�p�X���H
						{
							_tcscpy( szTagFile, p );	//�������H���Ȃ��B
						}
						else
						{
							//�h���C�u���H�����ق����悢�H
							_tcscpy( szTagFile, p );	//�������H���Ȃ��B
						}
					}
					else if( isalpha( p[0] ) && p[1] == L':' )	//��΃p�X���H
					{
						_tcscpy( szTagFile, p );	//�������H���Ȃ��B
					}
					else
					{
						auto_sprintf( szTagFile, _T("%ts%ls"), szCurrentPath, p );
					}

					return m_pCommanderView->TagJumpSub( szTagFile, CMyPoint(0, n2) );
				}
			}	//fp

			//�J�����g�p�X��1�K�w��ցB
			_tcscat( szCurrentPath, _T("..\\") );
		}

	return false;
}

/*
	�^�O�t�@�C�����쐬����B

	@author	MIK
	@date	2003.04.13	�V�K�쐬
	@date	2003.05.12	�_�C�A���O�\���Ńt�H���_�����ׂ����w��ł���悤�ɂ����B
	@date 2008.05.05 novice GetModuleHandle(NULL)��NULL�ɕύX
*/
bool CViewCommander::Command_TagsMake( void )
{
#define	CTAGS_COMMAND	_T("ctags.exe")

	TCHAR	szTargetPath[1024 /*_MAX_PATH+1*/ ];
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() )
	{
		_tcscpy( szTargetPath, GetDocument()->m_cDocFile.GetFilePath() );
		szTargetPath[ _tcslen( szTargetPath ) - _tcslen( GetDocument()->m_cDocFile.GetFileName() ) ] = _T('\0');
	}
	else
	{
		TCHAR	szTmp[1024];
		::GetModuleFileName(
			NULL,
			szTmp, _countof( szTmp )
		);
		/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
		/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
		::SplitPath_FolderAndFile( szTmp, szTargetPath, NULL );
	}

	//�_�C�A���O��\������
	CDlgTagsMake	cDlgTagsMake;
	if( !cDlgTagsMake.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0, szTargetPath ) ) return false;

	TCHAR	cmdline[1024];
	/* exe�̂���t�H���_ */
	TCHAR	szExeFolder[_MAX_PATH + 1];

	GetExedir( cmdline, CTAGS_COMMAND );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	//ctags.exe�̑��݃`�F�b�N
	if( -1 == ::GetFileAttributes( cmdline ) )
	{
		WarningMessage( m_pCommanderView->GetHwnd(), _T( "�^�O�쐬�R�}���h���s�͎��s���܂����B\n\nCTAGS.EXE ��������܂���B" ) );
		return false;
	}

	HANDLE	hStdOutWrite, hStdOutRead;
	CDlgCancel	cDlgCancel;
	CWaitCursor	cWaitCursor( m_pCommanderView->GetHwnd() );

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );

	//�q�v���Z�X�̕W���o�͂Ɛڑ�����p�C�v���쐬
	SECURITY_ATTRIBUTES	sa;
	ZeroMemory( &sa, sizeof(sa) );
	sa.nLength              = sizeof(sa);
	sa.bInheritHandle       = TRUE;
	sa.lpSecurityDescriptor = NULL;
	hStdOutRead = hStdOutWrite = 0;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE )
	{
		//�G���[
		return false;
	}

	//�p���s�\�ɂ���
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), NULL,
				0, FALSE, DUPLICATE_SAME_ACCESS );

	//CreateProcess�ɓn��STARTUPINFO���쐬
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(sui) );
	sui.cb          = sizeof(sui);
	sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sui.wShowWindow = SW_HIDE;
	sui.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
	sui.hStdOutput  = hStdOutWrite;
	sui.hStdError   = hStdOutWrite;

	//	To Here Dec. 28, 2002 MIK

	TCHAR	options[1024];
	_tcscpy( options, _T("--excmd=n") );	//�f�t�H���g�̃I�v�V����
	if( cDlgTagsMake.m_nTagsOpt & 0x0001 ) _tcscat( options, _T(" -R") );	//�T�u�t�H���_���Ώ�
	if( _tcslen( cDlgTagsMake.m_szTagsCmdLine ) )	//�ʎw��̃R�}���h���C��
	{
		_tcscat( options, _T(" ") );
		_tcscat( options, cDlgTagsMake.m_szTagsCmdLine );
	}
	_tcscat( options, _T(" *") );	//�z���̂��ׂẴt�@�C��

	//OS�o�[�W�����擾
	COsVersionInfo cOsVer;
	//�R�}���h���C��������쐬(MAX:1024)
	if (cOsVer.IsWin32NT())
	{
		//	2006.08.04 genta add /D to disable autorun
		auto_sprintf( cmdline, _T("cmd.exe /D /C \"\"%ts\\%ts\" %ts\""),
				szExeFolder,	//sakura.exe�p�X
				CTAGS_COMMAND,	//ctags.exe
				options			//ctags�I�v�V����
			);
	}
	else
	{
		auto_sprintf( cmdline, _T("command.com /C \"%ts\\%ts\" %ts"),
				szExeFolder,	//sakura.exe�p�X
				CTAGS_COMMAND,	//ctags.exe
				options			//ctags�I�v�V����
			);
	}

	//�R�}���h���C�����s
	BOOL bProcessResult = CreateProcess(
		NULL, cmdline, NULL, NULL, TRUE,
		CREATE_NEW_CONSOLE, NULL, cDlgTagsMake.m_szPath, &sui, &pi
	);
	if( !bProcessResult)
	{
		WarningMessage( m_pCommanderView->GetHwnd(), _T("�^�O�쐬�R�}���h���s�͎��s���܂����B\n\n%ts"), cmdline );
		goto finish;
	}

	{
		DWORD	read_cnt;
		DWORD	new_cnt;
		char	work[1024];
		bool	bLoopFlag = true;

		//���f�_�C�A���O�\��
		HWND	hwndCancel;
		HWND	hwndMsg;
		hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), m_pCommanderView->m_hwndParent, IDD_EXECRUNNING );
		hwndMsg = ::GetDlgItem( hwndCancel, IDC_STATIC_CMD );
		::SendMessage( hwndMsg, WM_SETTEXT, 0, (LPARAM)L"�^�O�t�@�C�����쐬���ł��B" );

		//���s���ʂ̎�荞��
		do {
			// Jun. 04, 2003 genta CPU��������炷���߂�200msec�҂�
			// ���̊ԃ��b�Z�[�W�������؂�Ȃ��悤�ɑ҂�����WaitForSingleObject����
			// MsgWaitForMultipleObject�ɕύX
			switch( MsgWaitForMultipleObjects( 1, &pi.hProcess, FALSE, 200, QS_ALLEVENTS )){
				case WAIT_OBJECT_0:
					//�I�����Ă���΃��[�v�t���O��FALSE�Ƃ���
					//���������[�v�̏I�������� �v���Z�X�I�� && �p�C�v����
					bLoopFlag = FALSE;
					break;
				case WAIT_OBJECT_0 + 1:
					//�������̃��[�U�[������\�ɂ���
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						break;
					}
					break;
				default:
					break;
			}

			//���f�{�^�������`�F�b�N
			if( cDlgCancel.IsCanceled() )
			{
				//�w�肳�ꂽ�v���Z�X�ƁA���̃v���Z�X�������ׂẴX���b�h���I�������܂��B
				::TerminateProcess( pi.hProcess, 0 );
				break;
			}

			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) )	//�p�C�v�̒��̓ǂݏo���ҋ@���̕��������擾
			{
				if( new_cnt > 0 )												//�ҋ@���̂��̂�����
				{
					if( new_cnt >= _countof(work) - 2 )							//�p�C�v����ǂݏo���ʂ𒲐�
					{
						new_cnt = _countof(work) - 2;
					}
					::ReadFile( hStdOutRead, &work[0], new_cnt, &read_cnt, NULL );	//�p�C�v����ǂݏo��
					if( read_cnt == 0 )
					{
						continue;
					}
					// 2003.11.09 ���イ��
					//	����I���̎��̓��b�Z�[�W���o�͂���Ȃ��̂�
					//	�����o�͂��ꂽ��G���[���b�Z�[�W�ƌ��Ȃ��D
					else {
						//�I������
						CloseHandle( hStdOutWrite );
						CloseHandle( hStdOutRead  );
						if( pi.hProcess ) CloseHandle( pi.hProcess );
						if( pi.hThread  ) CloseHandle( pi.hThread  );

						cDlgCancel.CloseDialog( TRUE );

						work[ read_cnt ] = L'\0';	// Nov. 15, 2003 genta �\���p��0�I�[����
						WarningMessage( m_pCommanderView->GetHwnd(), _T("�^�O�쐬�R�}���h���s�͎��s���܂����B\n\n%hs"), work ); // 2003.11.09 ���イ��

						return true;
					}
				}
			}
			Sleep(0);
		} while( bLoopFlag || new_cnt > 0 );

	}


finish:
	//�I������
	CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead  );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread  ) CloseHandle( pi.hThread  );

	cDlgCancel.CloseDialog( TRUE );

	InfoMessage(m_pCommanderView->GetHwnd(), _T("�^�O�t�@�C���̍쐬���I�����܂����B"));

	return true;
}

/*!
	�L�[���[�h���w�肵�ă^�O�W�����v

	@author MIK
	@date 2005.03.31 �V�K�쐬
*/
bool CViewCommander::Command_TagJumpByTagsFileKeyword( const wchar_t* keyword )
{
	CMemory	cmemKey;
	CDlgTagJumpList	cDlgTagJumpList;
	TCHAR	s[5][1024];
	int		n2;
	int depth;
	TCHAR	szTagFile[1024];		//�^�O�t�@�C��
	TCHAR	szCurrentPath[1024];

	if( ! GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ) return false;
	_tcscpy( szCurrentPath, GetDocument()->m_cDocFile.GetFilePath() );

	cDlgTagJumpList.SetFileName( szCurrentPath );
	cDlgTagJumpList.SetKeyword( keyword );

	szCurrentPath[ _tcslen( szCurrentPath ) - _tcslen( GetDocument()->m_cDocFile.GetFileName() ) ] = _T('\0');

	if( ! cDlgTagJumpList.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)1 ) ) 
	{
		return true;	//�L�����Z��
	}

	//�^�O�W�����v����B
	if( false == cDlgTagJumpList.GetSelectedParam( s[0], s[1], &n2, s[3], s[4], &depth ) )
	{
		return false;
	}

	/*
	 * s[0] �L�[
	 * s[1] �t�@�C����
	 * n2   �s�ԍ�
	 * s[3] �^�C�v
	 * s[4] �R�����g
	 * depth (�����̂ڂ�)�K�w��
	 */

	//���S�p�X�����쐬����B
	TCHAR	*p;
	p = s[1];
	if( p[0] == _T('\\') )	//�h���C�u�Ȃ���΃p�X���H
	{
		if( p[1] == _T('\\') )	//�l�b�g���[�N�p�X���H
		{
			_tcscpy( szTagFile, p );	//�������H���Ȃ��B
		}
		else
		{
			//�h���C�u���H�����ق����悢�H
			_tcscpy( szTagFile, p );	//�������H���Ȃ��B
		}
	}
	else if( _istalpha( p[0] ) && p[1] == _T(':') )	//��΃p�X���H
	{
		_tcscpy( szTagFile, p );	//�������H���Ȃ��B
	}
	else
	{
		for( int i = 0; i < depth; i++ )
		{
			_tcscat( szCurrentPath, _T("..\\") );
		}
		auto_sprintf( szTagFile, _T("%ls%ls"), szCurrentPath, p );
	}

	return m_pCommanderView->TagJumpSub( szTagFile, CMyPoint(0, n2) );
}



/* C/C++�w�b�_�t�@�C�� �I�[�v���@�\ */		//Feb. 10, 2001 jepro	�������u�C���N���[�h�t�@�C���v����ύX
//BOOL CViewCommander::Command_OPENINCLUDEFILE( BOOL bCheckOnly )
BOOL CViewCommander::Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca �w�b�_�E�\�[�X�̃R�[�h�𓝍����폜
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, source_ext, header_ext,
		_countof(source_ext), _countof(header_ext),
		_T("C/C++�w�b�_�t�@�C���̃I�[�v���Ɏ��s���܂����B") );
}




/* C/C++�\�[�X�t�@�C�� �I�[�v���@�\ */
//BOOL CViewCommander::Command_OPENCCPP( BOOL bCheckOnly )	//Feb. 10, 2001 JEPRO	�R�}���h�����኱�ύX
BOOL CViewCommander::Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca �w�b�_�E�\�[�X�̃R�[�h�𓝍����폜
	static const TCHAR* source_ext[] = { _T("c"), _T("cpp"), _T("cxx"), _T("cc"), _T("cp"), _T("c++") };
	static const TCHAR* header_ext[] = { _T("h"), _T("hpp"), _T("hxx"), _T("hh"), _T("hp"), _T("h++") };
	return m_pCommanderView->OPEN_ExtFromtoExt(
		bCheckOnly, bBeepWhenMiss, header_ext, source_ext,
		_countof(header_ext), _countof(source_ext),
		_T("C/C++�\�[�X�t�@�C���̃I�[�v���Ɏ��s���܂����B"));
}




//From Here Feb. 10, 2001 JEPRO �ǉ�
/* C/C++�w�b�_�t�@�C���܂��̓\�[�X�t�@�C�� �I�[�v���@�\ */
BOOL CViewCommander::Command_OPEN_HfromtoC( BOOL bCheckOnly )
{
	if ( Command_OPEN_HHPP( bCheckOnly, FALSE ) )	return TRUE;
	if ( Command_OPEN_CCPP( bCheckOnly, FALSE ) )	return TRUE;
	ErrorBeep();
	return FALSE;
// 2002/03/24 YAZAKI �R�[�h�̏d�����팸
// 2003.06.28 Moca �R�����g�Ƃ��Ďc���Ă����R�[�h���폜
}

//Start 2004.07.14 Kazika �ǉ�
/*!	@brief �������ĕ\��

	�^�u�E�B���h�E�̌����A�񌋍���؂�ւ���R�}���h�ł��B
	[���ʐݒ�]->[�E�B���h�E]->[�^�u�\�� �܂Ƃ߂Ȃ�]�̐؂�ւ��Ɠ����ł��B
	@author Kazika
	@date 2004.07.14 Kazika �V�K�쐬
	@date 2007.06.20 ryoji GetDllShareData().m_TabWndWndpl�̔p�~�C�O���[�vID���Z�b�g
*/
void CViewCommander::Command_BIND_WINDOW( void )
{
	//�^�u���[�h�ł���Ȃ��
	if (GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd)
	{
		//�^�u�E�B���h�E�̐ݒ��ύX
		GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin = !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin;

		// �܂Ƃ߂�Ƃ��� WS_EX_TOPMOST ��Ԃ𓯊�����	// 2007.05.18 ryoji
		if( !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
		{
			GetDocument()->m_pcEditWnd->WindowTopMost(
				( (DWORD)::GetWindowLongPtr( GetDocument()->m_pcEditWnd->GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST )? 1: 2
			);
		}

		//Start 2004.08.27 Kazika �ύX
		//�^�u�E�B���h�E�̐ݒ��ύX���u���[�h�L���X�g����
		CAppNodeManager::Instance()->ResetGroupId();
		CAppNodeGroupHandle(0).PostMessageToAllEditors(
			MYWM_TAB_WINDOW_NOTIFY,						//�^�u�E�B���h�E�C�x���g
			(WPARAM)((GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin) ? TWNT_MODE_DISABLE : TWNT_MODE_ENABLE),//�^�u���[�h�L��/�������C�x���g
			(LPARAM)GetDocument()->m_pcEditWnd->GetHwnd(),	//CEditWnd�̃E�B���h�E�n���h��
			m_pCommanderView->GetHwnd());									//�������g
		//End 2004.08.27 Kazika
	}
}
//End 2004.07.14 Kazika

/*!	@brief �d�˂ĕ\��

	@date 2002.01.08 YAZAKI �u���E�ɕ��ׂĕ\���v����ƁA
		���ōő剻����Ă���G�N�X�v���[�����u���̑傫���v�ɂȂ�o�O�C���B
	@date 2003.06.12 MIK �^�u�E�C���h�E���͓��삵�Ȃ��悤��
	@date 2004.03.19 crayonzen �J�����g�E�B���h�E���Ō�ɔz�u�D
		�E�B���h�E�������ꍇ��2���ڈȍ~�͉E�ɂ��炵�Ĕz�u�D
	@date 2004.03.20 genta Z-Order�̏ォ�珇�ɕ��ׂĂ����悤�ɁD(SetWindowPos�𗘗p)
	@date 2007.06.20 ryoji �^�u���[�h�͉��������O���[�v�P�ʂŕ��ׂ�
*/
void CViewCommander::Command_CASCADE( void )
{
	int i;

	/* ���݊J���Ă���ҏW���̃��X�g���擾���� */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		struct WNDARR {
			HWND	hWnd;
			int		newX;
			int		newY;
		};

		WNDARR*	pWndArr = new WNDARR[nRowNum];
		int		count = 0;	//	�����ΏۃE�B���h�E�J�E���g
		// Mar. 20, 2004 genta ���݂̃E�B���h�E�𖖔��Ɏ����Ă����̂Ɏg��
		int		current_win_index = -1;

		// -----------------------------------------
		//	�E�B���h�E(�n���h��)���X�g�̍쐬
		// -----------------------------------------

		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].GetHwnd() ) ){	//	�ŏ������Ă���E�B���h�E�͖����B
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].GetHwnd() ) ){	//	�s���E�B���h�E�͖����B
				continue;
			}
			//	Mar. 20, 2004 genta
			//	���݂̃E�B���h�E�𖖔��Ɏ����Ă������߂����ł̓X�L�b�v
			if( pEditNodeArr[i].GetHwnd() == CEditWnd::Instance()->GetHwnd() ){
				current_win_index = i;
				continue;
			}
			pWndArr[count].hWnd = pEditNodeArr[i].GetHwnd();
			count++;
		}

		//	Mar. 20, 2004 genta
		//	���݂̃E�B���h�E�𖖔��ɑ}�� inspired by crayonzen
		if( current_win_index >= 0 ){
			pWndArr[count].hWnd = pEditNodeArr[current_win_index].GetHwnd();
			count++;
		}

		//	�f�X�N�g�b�v�T�C�Y�𓾂�
		RECT	rcDesktop;
		//	May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect( m_pCommanderView->GetHwnd(), &rcDesktop );
		
		int width = (rcDesktop.right - rcDesktop.left ) * 4 / 5; // Mar. 9, 2003 genta �������Z�݂̂ɂ���
		int height = (rcDesktop.bottom - rcDesktop.top ) * 4 / 5;
		int w_delta = ::GetSystemMetrics(SM_CXSIZEFRAME) + ::GetSystemMetrics(SM_CXSIZE);
		int h_delta = ::GetSystemMetrics(SM_CYSIZEFRAME) + ::GetSystemMetrics(SM_CYSIZE);
		int w_offset = rcDesktop.left; //Mar. 19, 2004 crayonzen ��Βl���ƃG�N�X�v���[���[�̃E�B���h�E�ɏd�Ȃ�̂�
		int h_offset = rcDesktop.top; //�����l���f�X�N�g�b�v���Ɏ��߂�B

		// -----------------------------------------
		//	���W�v�Z
		//
		//	Mar. 19, 2004 crayonzen
		//		������f�X�N�g�b�v�̈�ɍ��킹��(�^�X�N�o�[����E���ɂ���ꍇ�̂���)�D
		//		�E�B���h�E���E������͂ݏo���獶��ɖ߂邪�C
		//		2���ڈȍ~�͊J�n�ʒu���E�ɂ��炵�ăA�C�R����������悤�ɂ���D
		//
		//	Mar. 20, 2004 genta �����ł͌v�Z�l��ۊǂ��邾���ŃE�B���h�E�̍Ĕz�u�͍s��Ȃ�
		// -----------------------------------------

		int roundtrip = 0; //�Q�x�ڂ̕`��ȍ~�Ŏg�p����J�E���g
		int sw_offset = w_delta; //�E�X���C�h�̕�

		for(i = 0; i < count; ++i ){
			if (w_offset + width > rcDesktop.right || h_offset + height > rcDesktop.bottom){
				++roundtrip;
				if ((rcDesktop.right - rcDesktop.left) - sw_offset * roundtrip < width){
					//	����ȏ�E�ɂ��点�Ȃ��Ƃ��͂��傤���Ȃ����獶��ɖ߂�
					roundtrip = 0;
				}
				//	�E�B���h�E�̈�̍���ɃZ�b�g
				//	craonzen �����l�C��(�Q�x�ڈȍ~�̕`��ŏ����ÂX���C�h)
				w_offset = rcDesktop.left + sw_offset * roundtrip;
				h_offset = rcDesktop.top;
			}
			
			pWndArr[i].newX = w_offset;
			pWndArr[i].newY = h_offset;

			w_offset += w_delta;
			h_offset += h_delta;
		}

		// -----------------------------------------
		//	�ő剻/��\������
		//	�ő剻���ꂽ�E�B���h�E�����ɖ߂��D���ꂪ�Ȃ��ƁC�ő剻�E�B���h�E��
		//	�ő剻��Ԃ̂܂ܕ��ёւ����Ă��܂��C���̌�ő剻���삪�ςɂȂ�D
		//
		//	Sep. 04, 2004 genta
		// -----------------------------------------
		for( i = 0; i < count; i++ ){
			::ShowWindow( pWndArr[i].hWnd, SW_RESTORE | SW_SHOWNA );
		}

		// -----------------------------------------
		//	�E�B���h�E�z�u
		//
		//	Mar. 20, 2004 genta API��f���Ɏg����Z-Order�̏ォ�牺�̏��ŕ��ׂ�D
		// -----------------------------------------

		// �܂��J�����g���őO�ʂ�
		i = count - 1;
		
		::SetWindowPos(
			pWndArr[i].hWnd, HWND_TOP,
			pWndArr[i].newX, pWndArr[i].newY,
			width, height,
			0
		);

		// �c���1�����ɓ���Ă���
		while( --i >= 0 ){
			::SetWindowPos(
				pWndArr[i].hWnd, pWndArr[i + 1].hWnd,
				pWndArr[i].newX, pWndArr[i].newY,
				width, height,
				SWP_NOACTIVATE
			);
		}

		delete [] pWndArr;
		delete [] pEditNodeArr;
	}
	return;
}




//���E�ɕ��ׂĕ\��
void CViewCommander::Command_TILE_H( void )
{
	int i;

	/* ���݊J���Ă���ҏW���̃��X�g���擾���� */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	�f�X�N�g�b�v�T�C�Y�𓾂�
		RECT	rcDesktop;
		//	May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect( m_pCommanderView->GetHwnd(), &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].GetHwnd() ) ){	//	�ŏ������Ă���E�B���h�E�͖����B
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].GetHwnd() ) ){	//	�s���E�B���h�E�͖����B
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	���݂̃E�B���h�E��擪�Ɏ����Ă���
			if( pEditNodeArr[i].GetHwnd() == CEditWnd::Instance()->GetHwnd() ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = CEditWnd::Instance()->GetHwnd();
			}
			else {
				phwndArr[count] = pEditNodeArr[i].GetHwnd();
			}
			//	To Here Jul. 28, 2002 genta
			count++;
		}
		int width = (rcDesktop.right - rcDesktop.left ) / count;
		for(i = 0; i < count; ++i ){
			//	Jul. 21, 2002 genta
			::ShowWindow( phwndArr[i], SW_RESTORE );
			::SetWindowPos(
				phwndArr[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // Oct. 18, 2003 genta �^�X�N�o�[�����ɂ���ꍇ���l��
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
		::SetFocus( phwndArr[0] );	// Aug. 17, 2002 MIK
		delete [] phwndArr;
		delete [] pEditNodeArr;
	}
	return;
}




//�㉺�ɕ��ׂĕ\��
void CViewCommander::Command_TILE_V( void )
{
	int i;

	/* ���݊J���Ă���ҏW���̃��X�g���擾���� */
	EditNode*	pEditNodeArr;
	int			nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/, TRUE );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	�f�X�N�g�b�v�T�C�Y�𓾂�
		RECT	rcDesktop;
		//	May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect( m_pCommanderView->GetHwnd(), &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].GetHwnd() ) ){	//	�ŏ������Ă���E�B���h�E�͖����B
				continue;
			}
			if( !::IsWindowVisible( pEditNodeArr[i].GetHwnd() ) ){	//	�s���E�B���h�E�͖����B
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	���݂̃E�B���h�E��擪�Ɏ����Ă���
			if( pEditNodeArr[i].GetHwnd() == CEditWnd::Instance()->GetHwnd() ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = CEditWnd::Instance()->GetHwnd();
			}
			else {
				phwndArr[count] = pEditNodeArr[i].GetHwnd();
			}
			//	To Here Jul. 28, 2002 genta
			count++;
		}
		int height = (rcDesktop.bottom - rcDesktop.top ) / count;
		for(i = 0; i < count; ++i ){
			//	Jul. 21, 2002 genta
			::ShowWindow( phwndArr[i], SW_RESTORE );
			::SetWindowPos(
				phwndArr[i], 0,
				rcDesktop.left, rcDesktop.top + height * i, //Mar. 19, 2004 crayonzen ��[����
				rcDesktop.right - rcDesktop.left, height,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
		::SetFocus( phwndArr[0] );	// Aug. 17, 2002 MIK

		delete [] phwndArr;
		delete [] pEditNodeArr;
	}
	return;
}



//�c�����ɍő剻
void CViewCommander::Command_MAXIMIZE_V( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;
	hwndFrame = GetMainWindow();
	::GetWindowRect( hwndFrame, &rcOrg );
	//	May 01, 2004 genta �}���`���j�^�Ή�
	::GetMonitorWorkRect( hwndFrame, &rcDesktop );
	::SetWindowPos(
		hwndFrame, 0,
		rcOrg.left, rcDesktop.top,
		rcOrg.right - rcOrg.left, rcDesktop.bottom - rcDesktop.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}




//2001.02.10 Start by MIK: �������ɍő剻
//�������ɍő剻
void CViewCommander::Command_MAXIMIZE_H( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;

	hwndFrame = GetMainWindow();
	::GetWindowRect( hwndFrame, &rcOrg );
	//	May 01, 2004 genta �}���`���j�^�Ή�
	::GetMonitorWorkRect( hwndFrame, &rcDesktop );
	::SetWindowPos(
		hwndFrame, 0,
		rcDesktop.left, rcOrg.top,
		rcDesktop.right - rcDesktop.left, rcOrg.bottom - rcOrg.top,
		SWP_NOOWNERZORDER | SWP_NOZORDER
	);
	return;
}
//2001.02.10 End: �������ɍő剻




/* ���ׂčŏ��� */	//	Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
void CViewCommander::Command_MINIMIZE_ALL( void )
{
	HWND*	phWndArr;
	int		i;
	int		j;
	j = GetDllShareData().m_sNodes.m_nEditArrNum;
	if( 0 == j ){
		return;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = GetDllShareData().m_sNodes.m_pEditArr[i].GetHwnd();
	}
	for( i = 0; i < j; ++i ){
		if( IsSakuraMainWindow( phWndArr[i] ) )
		{
			if( ::IsWindowVisible( phWndArr[i] ) )
				::ShowWindow( phWndArr[i], SW_MINIMIZE );
		}
	}
	delete [] phWndArr;
	return;
}




//�u��(�u���_�C�A���O)
void CViewCommander::Command_REPLACE_DIALOG( void )
{
	BOOL		bSelected = FALSE;

	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	CNativeW	cmemCurText;
	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText );	// 2006.08.23 ryoji �_�C�A���O��p�֐��ɕύX

	/* ����������������� */
	wcscpy( GetEditWindow()->m_cDlgReplace.m_szText, cmemCurText.GetStringPtr() );
	wcsncpy( GetEditWindow()->m_cDlgReplace.m_szText2, GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0], MAX_PATH - 1 );	// 2006.08.23 ryoji �O��̒u���㕶����������p��
	GetEditWindow()->m_cDlgReplace.m_szText2[MAX_PATH - 1] = L'\0';

	if ( m_pCommanderView->GetSelectionInfo().IsTextSelected() && !GetSelect().IsLineOne() ) {
		bSelected = TRUE;	//�I��͈͂��`�F�b�N���ă_�C�A���O�\��
	}else{
		bSelected = FALSE;	//�t�@�C���S�̂��`�F�b�N���ă_�C�A���O�\��
	}
	/* �u���I�v�V�����̏����� */
	GetEditWindow()->m_cDlgReplace.m_nReplaceTarget=0;	/* �u���Ώ� */
	GetEditWindow()->m_cDlgReplace.m_nPaste=FALSE;		/* �\��t����H */
// To Here 2001.12.03 hor

	/* �u���_�C�A���O�̕\�� */
	//	From Here Jul. 2, 2001 genta �u���E�B���h�E��2�d�J����}�~
	if( !::IsWindow( GetEditWindow()->m_cDlgReplace.GetHwnd() ) ){
		GetEditWindow()->m_cDlgReplace.DoModeless( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)m_pCommanderView, bSelected );
	}
	else {
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( GetEditWindow()->m_cDlgReplace.GetHwnd() );
		::DlgItem_SetText( GetEditWindow()->m_cDlgReplace.GetHwnd(), IDC_COMBO_TEXT, cmemCurText.GetStringT() );
	}
	//	To Here Jul. 2, 2001 genta �u���E�B���h�E��2�d�J����}�~
	return;
}

/*! �u�����s
	
	@date 2002/04/08 �e�E�B���h�E���w�肷��悤�ɕύX�B
	@date 2003.05.17 ����� �����O�}�b�`�̖����u������Ȃ�
*/
void CViewCommander::Command_REPLACE( HWND hwndParent )
{
	if ( hwndParent == NULL ){	//	�e�E�B���h�E���w�肳��Ă��Ȃ���΁ACEditView���e�B
		hwndParent = m_pCommanderView->GetHwnd();
	}
	//2002.02.10 hor
	int nPaste			=	GetEditWindow()->m_cDlgReplace.m_nPaste;
	int nReplaceTarget	=	GetEditWindow()->m_cDlgReplace.m_nReplaceTarget;
	int	bRegularExp		=	GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp;
	int nFlag			=	GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bLoHiCase ? 0x01 : 0x00;

	// From Here 2001.12.03 hor
	if( nPaste && !GetDocument()->m_cDocEditor.IsEnablePaste()){
		OkMessage( hwndParent, _T("�N���b�v�{�[�h�ɗL���ȃf�[�^������܂���I") );
		::CheckDlgButton( GetEditWindow()->m_cDlgReplace.GetHwnd(), IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( GetEditWindow()->m_cDlgReplace.GetHwnd(), IDC_COMBO_TEXT2 ), TRUE );
		return;	//	���sreturn;
	}

	// 2002.01.09 hor
	// �I���G���A������΁A���̐擪�ɃJ�[�\�����ڂ�
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
			GetCaret().MoveCursor( GetSelect().GetFrom(), true );
		} else {
//			HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );
			Command_LEFT( FALSE, FALSE );
		}
	}
	// To Here 2002.01.09 hor
	
	// ��`�I���H
//			bBeginBoxSelect = m_pCommanderView->GetSelectionInfo().IsBoxSelecting();

	/* �J�[�\�����ړ� */
	//HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );	//�H�H�H
	// To Here 2001.12.03 hor

	/* �e�L�X�g�I������ */
	/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );

	// 2004.06.01 Moca �������ɁA���̃v���Z�X�ɂ����m_aReplaceKeys�������������Ă����v�Ȃ悤��
	const CNativeW	cMemRepKey( GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0] );

	/* �������� */
	Command_SEARCH_NEXT( true, TRUE, hwndParent, 0 );

	/* �e�L�X�g���I������Ă��邩 */
	if( m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		// From Here 2001.12.03 hor
		CLayoutPoint ptTmp(0,0);
		if ( !bRegularExp ) {
			// ���K�\������ ����Q��($&)�Ŏ�������̂ŁA���K�\���͏��O
			if(nReplaceTarget==1){	//�}���ʒu�ֈړ�
				ptTmp = GetSelect().GetTo() - GetSelect().GetFrom();
				GetSelect().Clear(-1);
			}
			else if(nReplaceTarget==2){	//�ǉ��ʒu�ֈړ�
				// ���K�\�������O�����̂ŁA�u������̕��������s������玟�̍s�̐擪�ֈړ��v�̏������폜
				GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
				GetSelect().Clear(-1);
			}
			else{
				// �ʒu�w��Ȃ��̂ŁA�������Ȃ�
			}
		}
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		/* �e�L�X�g��\��t�� */
		if(nPaste){
			Command_PASTE();
		} else if ( bRegularExp ) { /* �����^�u��  1==���K�\�� */
			// ��ǂ݂ɑΉ����邽�߂ɕ����s���܂ł��g���悤�ɕύX 2005/03/27 �����
			// 2002/01/19 novice ���K�\���ɂ�镶����u��
			CMemory cmemory;
			CBregexp cRegexp;

			if( !InitRegexp( m_pCommanderView->GetHwnd(), cRegexp, true ) ){
				return;	//	���sreturn;
			}

			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
			const wchar_t* pLine = pcLayout->GetDocLineRef()->GetPtr();
			CLogicInt nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetSelect().GetFrom().GetX2() ) + pcLayout->GetLogicOffset();
			CLogicInt nLen = pcLayout->GetDocLineRef()->GetLengthWithEOL();
			// ���K�\���őI���n�_�E�I�_�ւ̑}�����L�q
			//	Jun. 6, 2005 �����
			// ������ł́u�����̌��̕��������s�������玟�̍s���ֈړ��v�������ł��Ȃ�
			// �� Oct. 30, �u�����̌��̕��������s��������E�E�v�̏�������߂�i�N������Ȃ��݂����Ȃ̂Łj
			// Nov. 9, 2005 ����� ���K�\���őI���n�_�E�I�_�ւ̑}�����@��ύX(��)
			CNativeW cMemMatchStr; cMemMatchStr.SetString(L"$&");
			CNativeW cMemRepKey2;
			if (nReplaceTarget == 1) {	//�I���n�_�֑}��
				cMemRepKey2 = cMemRepKey;
				cMemRepKey2 += cMemMatchStr;
			} else if (nReplaceTarget == 2) { // �I���I�_�֑}��
				cMemRepKey2 = cMemMatchStr;
				cMemRepKey2 += cMemRepKey;
			} else {
				cMemRepKey2 = cMemRepKey;
			}
			cRegexp.Compile( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0], cMemRepKey2.GetStringPtr(), nFlag);
			if( cRegexp.Replace(pLine, nLen, nIdx) ){
				// From Here Jun. 6, 2005 �����
				// �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				// �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				// �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				CLayoutMgr& rLayoutMgr = GetDocument()->m_cLayoutMgr;
				CLogicInt matchLen = cRegexp.GetMatchLen();
				CLogicInt nIdxTo = nIdx + matchLen;		// ����������̖���
				if (matchLen == 0) {
					// �O�����}�b�`�̎�(�����u���ɂȂ�Ȃ��悤�ɂP�����i�߂�)
					if (nIdxTo < nLen) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxTo += CLogicInt(CNativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					}
					// �����u�����Ȃ��悤�ɁA�P�������₵���̂łP�����I���ɕύX
					// �I���n�_�E�I�_�ւ̑}���̏ꍇ���O�����}�b�`���͓���͓����ɂȂ�̂�
					rLayoutMgr.LogicToLayout( CLogicPoint(nIdxTo, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji �s�ʒu���擾����
				}
				// �s�����猟�������񖖔��܂ł̕�����
				CLogicInt colDiff = nLen - nIdxTo;
				//	Oct. 22, 2005 Karoto
				//	\r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				if (colDiff < pcLayout->GetDocLineRef()->GetEol().GetLen()) {
					// ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					colDiff = CLogicInt(0);
					rLayoutMgr.LogicToLayout( CLogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji �ǉ�
				}
				// �u���㕶����ւ̏�������(�s�����猟�������񖖔��܂ł̕���������)
				Command_INSTEXT( FALSE, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE );
				// To Here Jun. 6, 2005 �����
			}
		}else{
			//	HandleCommand( F_INSTEXT_W, FALSE, (LPARAM)GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0], FALSE, 0, 0 );
			Command_INSTEXT( FALSE, cMemRepKey.GetStringPtr(), cMemRepKey.GetStringLength(), TRUE );
		}

		// �}����̌����J�n�ʒu�𒲐�
		if(nReplaceTarget==1){
			GetCaret().SetCaretLayoutPos(GetCaret().GetCaretLayoutPos()+ptTmp);
		}

		// To Here 2001.12.03 hor
		/* �Ō�܂Œu����������OK�����܂Œu���O�̏�Ԃ��\�������̂ŁA
		** �u����A������������O�ɏ������� 2003.05.17 �����
		*/
		m_pCommanderView->Redraw();

		/* �������� */
		Command_SEARCH_NEXT( true, TRUE, hwndParent, LTEXT("�Ō�܂Œu�����܂����B") );
	}
}

/*! ���ׂĒu�����s

	@date 2003.05.22 ����� �����}�b�`�΍�D�s���E�s�������Ȃǌ�����
	@date 2006.03.31 ����� �s�u���@�\�ǉ�
	@date 2007.01.16 ryoji �s�u���@�\��S�u���̃I�v�V�����ɕύX
	@date 2009.09.20 genta �����`�E��ŋ�`�I�����ꂽ�̈�̒u�����s���Ȃ�
*/
void CViewCommander::Command_REPLACE_ALL()
{




	//2002.02.10 hor
	int nPaste			= GetEditWindow()->m_cDlgReplace.m_nPaste;
	int nReplaceTarget	= GetEditWindow()->m_cDlgReplace.m_nReplaceTarget;
	int	bRegularExp		= GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp;
	int bSelectedArea	= GetDllShareData().m_Common.m_sSearch.m_bSelectedArea;
	int bConsecutiveAll	= GetDllShareData().m_Common.m_sSearch.m_bConsecutiveAll;	/* �u���ׂĒu���v�͒u���̌J�Ԃ� */	// 2007.01.16 ryoji

	GetEditWindow()->m_cDlgReplace.m_bCanceled=false;
	GetEditWindow()->m_cDlgReplace.m_nReplaceCnt=0;

	// From Here 2001.12.03 hor
	if( nPaste && !GetDocument()->m_cDocEditor.IsEnablePaste() ){
		OkMessage( m_pCommanderView->GetHwnd(), _T("�N���b�v�{�[�h�ɗL���ȃf�[�^������܂���I") );
		::CheckDlgButton( m_pCommanderView->GetHwnd(), IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( m_pCommanderView->GetHwnd(), IDC_COMBO_TEXT2 ), TRUE );
		return;	// TRUE;
	}
	// To Here 2001.12.03 hor

	bool		bBeginBoxSelect; // ��`�I���H
	if(m_pCommanderView->GetSelectionInfo().IsTextSelected()){
		bBeginBoxSelect=m_pCommanderView->GetSelectionInfo().IsBoxSelecting();
	}
	else{
		bSelectedArea=FALSE;
		bBeginBoxSelect=false;
	}

	/* �\������ON/OFF */
	bool bDisplayUpdate = false;

	m_pCommanderView->SetDrawSwitch(bDisplayUpdate);

	int	nAllLineNum = (Int)GetDocument()->m_cLayoutMgr.GetLineCount();

	/* �i���\��&���~�_�C�A���O�̍쐬 */
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), m_pCommanderView->GetHwnd(), IDD_REPLACERUNNING );
	::EnableWindow( m_pCommanderView->GetHwnd(), FALSE );
	::EnableWindow( ::GetParent( m_pCommanderView->GetHwnd() ), FALSE );
	::EnableWindow( ::GetParent( ::GetParent( m_pCommanderView->GetHwnd() ) ), FALSE );
	//<< 2002/03/26 Azumaiya
	// ����Z�|���Z�������ɐi���󋵂�\����悤�ɁA�V�t�g���Z������B
	int nShiftCount;
	for ( nShiftCount = 0; SHRT_MAX < nAllLineNum; nShiftCount++ )
	{
		nAllLineNum/=2;
	}
	//>> 2002/03/26 Azumaiya

	/* �v���O���X�o�[������ */
	HWND		hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS_REPLACE );
	::SendMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, nAllLineNum ) );
	int			nNewPos = 0;
 	::SendMessageAny( hwndProgress, PBM_SETPOS, nNewPos, 0 );

	/* �u���������� */
	int			nReplaceNum = 0;
	HWND		hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
	TCHAR szLabel[64];
	_itot( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	CLayoutRange sRangeA;	//�I��͈�
	CLogicPoint ptColLineP;

	// From Here 2001.12.03 hor
	if (bSelectedArea){
		/* �I��͈͒u�� */
		/* �I��͈͊J�n�ʒu�̎擾 */
		sRangeA = GetSelect();

		//	From Here 2007.09.20 genta ��`�͈͂̑I��u�����ł��Ȃ�
		//	�����`�E��ƑI�������ꍇ�Cm_nSelectColmTo < m_nSelectColmFrom �ƂȂ邪�C
		//	�͈̓`�F�b�N�� colFrom < colTo �����肵�Ă���̂ŁC
		//	��`�I���̏ꍇ�͍���`�E���w��ɂȂ�悤������ꊷ����D
		if( bBeginBoxSelect && sRangeA.GetTo().x < sRangeA.GetFrom().x )
			t_swap(sRangeA.GetFromPointer()->x,sRangeA.GetToPointer()->x);
		//	To Here 2007.09.20 genta ��`�͈͂̑I��u�����ł��Ȃ�

		GetDocument()->m_cLayoutMgr.LayoutToLogic(
			sRangeA.GetTo(),
			&ptColLineP
		);
		//�I��͈͊J�n�ʒu�ֈړ�
		GetCaret().MoveCursor( sRangeA.GetFrom(), bDisplayUpdate );
	}
	else{
		/* �t�@�C���S�̒u�� */
		/* �t�@�C���̐擪�Ɉړ� */
	//	HandleCommand( F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0 );
		Command_GOFILETOP(bDisplayUpdate);
	}

	CLayoutPoint ptLast = GetCaret().GetCaretLayoutPos();

	/* �e�L�X�g�I������ */
	/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	m_pCommanderView->GetSelectionInfo().DisableSelectArea( bDisplayUpdate );
	/* �������� */
	Command_SEARCH_NEXT( true, bDisplayUpdate, 0, 0 );
	// To Here 2001.12.03 hor

	//<< 2002/03/26 Azumaiya
	// �������������Ƃ��ŗD��ɑg��ł݂܂����B
	// ���[�v�̊O�ŕ�����̒��������ł���̂ŁA�ꎞ�ϐ����B
	const wchar_t *szREPLACEKEY;		// �u���㕶����B
	bool		bColmnSelect;	// ��`�\��t�����s�����ǂ����B
	CNativeW	cmemClip;		// �u���㕶����̃f�[�^�i�f�[�^���i�[���邾���ŁA���[�v���ł͂��̌`�ł̓f�[�^�������܂���j�B

	// �N���b�v�{�[�h����̃f�[�^�\��t�����ǂ����B
	if( nPaste != 0 )
	{
		// �N���b�v�{�[�h����f�[�^���擾�B
		if ( !m_pCommanderView->MyGetClipboardData( cmemClip, &bColmnSelect ) )
		{
			ErrorBeep();
			return;
		}

		// ��`�\��t����������Ă��āA�N���b�v�{�[�h�̃f�[�^����`�I���̂Ƃ��B
		if ( GetDllShareData().m_Common.m_sEdit.m_bAutoColmnPaste && bColmnSelect )
		{
			// �}�E�X�ɂ��͈͑I��
			if( m_pCommanderView->GetSelectionInfo().IsMouseSelecting() )
			{
				ErrorBeep();
				return;
			}

			// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
			if( !GetDllShareData().m_Common.m_sView.m_bFontIs_FIXED_PITCH )
			{
				return;
			}
		}
		else
		// �N���b�v�{�[�h����̃f�[�^�͕��ʂɈ����B
		{
			bColmnSelect = false;
		}
	}
	else
	{
		// 2004.05.14 Moca �S�u���̓r���ő��̃E�B���h�E�Œu�������Ƃ܂����̂ŃR�s�[����
		cmemClip.SetString( GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0] );
	}

	CLogicInt nREPLACEKEY;			// �u���㕶����̒����B
	szREPLACEKEY = cmemClip.GetStringPtr(&nREPLACEKEY);

	// �擾�ɃX�e�b�v�������肻���ȕϐ��Ȃǂ��A�ꎞ�ϐ�������B
	// �Ƃ͂����A�����̑�������邱�Ƃɂ���ē�������N���b�N���͍��킹�Ă� 1 ���[�v�Ő��\���Ǝv���܂��B
	// ���S�N���b�N�����[�v�̃I�[�_�[����l���Ă�����Ȃɓ��͂��Ȃ��悤�Ɏv���܂����ǁE�E�E�B
	BOOL bAddCRLFWhenCopy = GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy;
	BOOL &bCANCEL = cDlgCancel.m_bCANCEL;
	CDocLineMgr& rDocLineMgr = GetDocument()->m_cDocLineMgr;
	CLayoutMgr& rLayoutMgr = GetDocument()->m_cLayoutMgr;

	//  �N���X�֌W�����[�v�̒��Ő錾���Ă��܂��ƁA�����[�v���ƂɃR���X�g���N�^�A�f�X�g���N�^��
	// �Ă΂�Ēx���Ȃ�̂ŁA�����Ő錾�B
	CMemory cmemory;
	CBregexp cRegexp;
	// �����������l�ɖ����[�v���Ƃɂ��ƒx���̂ŁA�ŏ��ɍς܂��Ă��܂��B
	if( bRegularExp )
	{
		if ( !InitRegexp( m_pCommanderView->GetHwnd(), cRegexp, true ) )
		{
			return;
		}

		const CNativeW	cMemRepKey( szREPLACEKEY, wcslen(szREPLACEKEY) );
		// Nov. 9, 2005 ����� ���K�\���őI���n�_�E�I�_�ւ̑}�����@��ύX(��)
		CNativeW cMemRepKey2;
		CNativeW cMemMatchStr;
		cMemMatchStr.SetString(L"$&");
		if (nReplaceTarget == 1 ) {	//�I���n�_�֑}��
			cMemRepKey2 = cMemRepKey;
			cMemRepKey2 += cMemMatchStr;
		} else if (nReplaceTarget == 2) { // �I���I�_�֑}��
			cMemRepKey2 = cMemMatchStr;
			cMemRepKey2 += cMemRepKey;
		} else {
			cMemRepKey2 = cMemRepKey;
		}
		// ���K�\���I�v�V�����̐ݒ�2006.04.01 �����
		int nFlag = (GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bLoHiCase ? CBregexp::optCaseSensitive : CBregexp::optNothing);
		nFlag |= (bConsecutiveAll ? CBregexp::optNothing : CBregexp::optGlobal);	// 2007.01.16 ryoji
		cRegexp.Compile(GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0], cMemRepKey2.GetStringPtr(), nFlag);
	}

	//$$ �P�ʍ���
	CLayoutPoint	ptOld;						//������̑I��͈�
	/*CLogicInt*/int		lineCnt;					//�u���O�̍s��
	/*CLayoutInt*/int		linDif = (0);		//�u����̍s����
	/*CLayoutInt*/int		colDif = (0);		//�u����̌�����
	/*CLayoutInt*/int		linPrev = (0);	//�O��̌����s(��`) @@@2001.12.31 YAZAKI warning�ގ�
	/*CLogicInt*/int		linOldLen = (0);	//������̍s�̒���
	/*CLayoutInt*/int		linNext;					//����̌����s(��`)

	/* �e�L�X�g���I������Ă��邩 */
	while( m_pCommanderView->GetSelectionInfo().IsTextSelected() )
	{
		/* �L�����Z�����ꂽ�� */
		if( bCANCEL )
		{
			break;
		}

		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( hwndCancel ) )
		{
			return;// -1;
		}

		// 128 ���Ƃɕ\���B
		if( 0 == (nReplaceNum & 0x7F ) )
		// ���Ԃ��Ƃɐi���󋵕`�悾�Ǝ��Ԏ擾���x���Ȃ�Ǝv�����A������̕������R���Ǝv���̂ŁE�E�E�B
		// �Ǝv�������ǁA�t�ɂ�����̕������R�ł͂Ȃ��̂ŁA��߂�B
		{
			nNewPos = (Int)GetSelect().GetFrom().GetY2() >> nShiftCount;
			::PostMessageAny( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			_itot( nReplaceNum, szLabel, 10 );
			::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );
		}

		// From Here 2001.12.03 hor
		/* ������̈ʒu���m�F */
		if( bSelectedArea )
		{
			// ��`�I��
			//	o ���C�A�E�g���W���`�F�b�N���Ȃ���u������
			//	o �܂�Ԃ�������ƕςɂȂ邩���E�E�E
			//
			if ( bBeginBoxSelect )
			{
				// �������̍s�����L��
				lineCnt = (Int)rLayoutMgr.GetLineCount();
				// ������͈̔͏I�[
				ptOld = GetSelect().GetTo();
				// �O��̌����s�ƈႤ�H
				if(ptOld.y!=linPrev){
					colDif=(0);
				}
				linPrev=(Int)ptOld.GetY2();
				// �s�͔͈͓��H
				if ((sRangeA.GetTo().y+linDif == ptOld.y && sRangeA.GetTo().GetX2()+colDif < ptOld.x) ||
					(sRangeA.GetTo().y+linDif <  ptOld.y)) {
					break;
				}
				// ���͔͈͓��H
				if(!(sRangeA.GetFrom().x<=GetSelect().GetFrom().x && ptOld.GetX2()<=sRangeA.GetTo().GetX2()+colDif)){
					if(ptOld.x<sRangeA.GetTo().GetX2()+colDif){
						linNext=(Int)GetSelect().GetTo().GetY2();
					}else{
						linNext=(Int)GetSelect().GetTo().GetY2()+1;
					}
					//���̌����J�n�ʒu�փV�t�g
					GetCaret().SetCaretLayoutPos(CLayoutPoint(sRangeA.GetFrom().x,CLayoutInt(linNext)));
					// 2004.05.30 Moca ���݂̌�����������g���Č�������
					Command_SEARCH_NEXT( false, bDisplayUpdate, 0, 0 );
					colDif=(0);
					continue;
				}
			}
			// ���ʂ̑I��
			//	o �������W���`�F�b�N���Ȃ���u������
			//
			else {
				// �������̍s�����L��
				lineCnt = rDocLineMgr.GetLineCount();

				// ������͈̔͏I�[
				CLogicPoint ptOldTmp;
				rLayoutMgr.LayoutToLogic(
					GetSelect().GetTo(),
					&ptOldTmp
				);
				ptOld=CLayoutPoint(ptOldTmp); //$$ ���C�A�E�g�^�ɖ�����胍�W�b�N�^�����B�C��������

				// �u���O�̍s�̒���(���s�͂P�����Ɛ�����)��ۑ����Ă����āA�u���O��ōs�ʒu���ς�����ꍇ�Ɏg�p
				linOldLen = rDocLineMgr.GetLine(ptOldTmp.GetY2())->GetLengthWithoutEOL() + CLogicInt(1);

				// �s�͔͈͓��H
				// 2007.01.19 ryoji �����ǉ�: �I���I�_���s��(ptColLineP.x == 0)�ɂȂ��Ă���ꍇ�͑O�̍s�̍s���܂ł�I��͈͂Ƃ݂Ȃ�
				// �i�I���n�_���s���Ȃ炻�̍s���͑I��͈͂Ɋ܂݁A�I�_���s���Ȃ炻�̍s���͑I��͈͂Ɋ܂܂Ȃ��A�Ƃ���j
				// �_���I�ɏ����ςƎw�E����邩������Ȃ����A���p��͂��̂悤�ɂ����ق����]�܂����P�[�X�������Ǝv����B
				// ���s�I���ōs���܂ł�I��͈͂ɂ�������ł��AUI��͎��̍s�̍s���ɃJ�[�\�����s��
				// ���I�_�̍s�����u^�v�Ƀ}�b�`��������������P�����ȏ�I�����ĂˁA�Ƃ������ƂŁD�D�D
				// $$ �P�ʍ��݂��܂��肾���ǁA���v�H�H
				if ((ptColLineP.y+linDif == (Int)ptOld.y && (ptColLineP.x+colDif < (Int)ptOld.x || ptColLineP.x == 0))
					|| ptColLineP.y+linDif < (Int)ptOld.y) {
					break;
				}
			}
		}


		CLayoutPoint ptTmp(0,0);

		if ( !bRegularExp ) {
			// ���K�\������ ����Q��($&)�Ŏ�������̂ŁA���K�\���͏��O
			if( nReplaceTarget == 1 )	//�}���ʒu�Z�b�g
			{
				ptTmp.x = GetSelect().GetTo().x - GetSelect().GetFrom().x;
				ptTmp.y = GetSelect().GetTo().y - GetSelect().GetFrom().y;
				GetSelect().Clear(-1);
			}
			else if( nReplaceTarget == 2 )	//�ǉ��ʒu�Z�b�g
			{
				// ���K�\�������O�����̂ŁA�u������̕��������s������玟�̍s�̐擪�ֈړ��v�̏������폜
				GetCaret().SetCaretLayoutPos(GetSelect().GetTo());
				GetSelect().Clear(-1);
		    }
			else {
				// �ʒu�w��Ȃ��̂ŁA�������Ȃ�
			}
		}

		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		/* �e�L�X�g��\��t�� */
		if( nPaste )
		{
			if ( !bColmnSelect )
			{
				/* �{���� Command_INSTEXT ���g���ׂ��Ȃ�ł��傤���A���ʂȏ���������邽�߂ɒ��ڂ������B
				** ��m_nSelectXXX��-1�̎��� m_pCommanderView->ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽��
				**   ���ڂ������̂�߂��B2003.05.18 by �����
				*/
				Command_INSTEXT( FALSE, szREPLACEKEY, nREPLACEKEY, TRUE );
			}
			else
			{
				Command_PASTEBOX(szREPLACEKEY, nREPLACEKEY);
				// �ĕ`����s��Ȃ��Ƃǂ�Ȍ��ʂ��N���Ă���̂������炸�݂��Ƃ��Ȃ��̂ŁE�E�E�B
				m_pCommanderView->AdjustScrollBars(); // 2007.07.22 ryoji
				m_pCommanderView->Redraw();
			}
		}
		// 2002/01/19 novice ���K�\���ɂ�镶����u��
		else if( bRegularExp ) /* �����^�u��  1==���K�\�� */
		{
			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const CLayout* pcLayout = rLayoutMgr.SearchLineByLayoutY(GetSelect().GetFrom().GetY2());
			const wchar_t* pLine = pcLayout->GetDocLineRef()->GetPtr();
			CLogicInt nIdx = m_pCommanderView->LineColmnToIndex( pcLayout, GetSelect().GetFrom().GetX2() ) + pcLayout->GetLogicOffset();
			CLogicInt nLen = pcLayout->GetDocLineRef()->GetLengthWithEOL();
			CLogicInt colDiff = CLogicInt(0);
			if( !bConsecutiveAll ){	// �ꊇ�u��
				// 2007.01.16 ryoji
				// �I��͈͒u���̏ꍇ�͍s���̑I��͈͖����܂Œu���͈͂��k�߁C
				// ���̈ʒu���L������D
				if( bSelectedArea ){
					if( bBeginBoxSelect ){	// ��`�I��
						CLogicPoint ptWork;
						rLayoutMgr.LayoutToLogic(
							CLayoutPoint(sRangeA.GetTo().x,ptOld.y),
							&ptWork
						);
						ptColLineP.x = ptWork.x;
						if( nLen - pcLayout->GetDocLineRef()->GetEol().GetLen() > ptColLineP.x + colDif )
							nLen = ptColLineP.GetX2() + CLogicInt(colDif);
					} else {	// �ʏ�̑I��
						if( ptColLineP.y+linDif == (Int)ptOld.y ){ //$$ �P�ʍ���
							if( nLen - pcLayout->GetDocLineRef()->GetEol().GetLen() > ptColLineP.x + colDif )
								nLen = ptColLineP.GetX2() + CLogicInt(colDif);
						}
					}
				}

				if(pcLayout->GetDocLineRef()->GetLengthWithoutEOL() < nLen)
					ptOld.x = (CLayoutInt)(Int)pcLayout->GetDocLineRef()->GetLengthWithoutEOL() + 1; //$$ �P�ʍ���
				else
					ptOld.x = (CLayoutInt)(Int)nLen; //$$ �P�ʍ���
			}

			if( int nReplace = cRegexp.Replace(pLine, nLen, nIdx) ){
				nReplaceNum += nReplace;
				if ( !bConsecutiveAll ) { // 2006.04.01 �����	// 2007.01.16 ryoji
					// �s�P�ʂł̒u������
					// �I��͈͂𕨗��s���܂łɂ̂΂�
					rLayoutMgr.LogicToLayout( CLogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );
				} else {
				    // From Here Jun. 6, 2005 �����
				    // �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				    // �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				    // �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				    CLogicInt matchLen = cRegexp.GetMatchLen();
				    CLogicInt nIdxTo = nIdx + matchLen;		// ����������̖���
				    if (matchLen == 0) {
					    // �O�����}�b�`�̎�(�����u���ɂȂ�Ȃ��悤�ɂP�����i�߂�)
					    if (nIdxTo < nLen) {
						    // 2005-09-02 D.S.Koba GetSizeOfChar
						    nIdxTo += CLogicInt(CNativeW::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					    }
					    // �����u�����Ȃ��悤�ɁA�P�������₵���̂łP�����I���ɕύX
					    // �I���n�_�E�I�_�ւ̑}���̏ꍇ���O�����}�b�`���͓���͓����ɂȂ�̂�
						rLayoutMgr.LogicToLayout( CLogicPoint(nIdxTo, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji �s�ʒu���擾����
				    }
				    // �s�����猟�������񖖔��܂ł̕�����
					colDiff =  nLen - nIdxTo;
					ptOld.x = (CLayoutInt)(Int)nIdxTo;	// 2007.01.19 ryoji �ǉ�  // $$ �P�ʍ���
				    //	Oct. 22, 2005 Karoto
				    //	\r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				    if (colDiff < pcLayout->GetDocLineRef()->GetEol().GetLen()) {
					    // ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					    colDiff = CLogicInt(0);
						rLayoutMgr.LogicToLayout( CLogicPoint(nLen, pcLayout->GetLogicLineNo()), GetSelect().GetToPointer() );	// 2007.01.19 ryoji �ǉ�
						ptOld.x = (CLayoutInt)(Int)pcLayout->GetDocLineRef()->GetLengthWithoutEOL() + 1;	// 2007.01.19 ryoji �ǉ� //$$ �P�ʍ���
				    }
				}
				// �u���㕶����ւ̏�������(�s�����猟�������񖖔��܂ł̕���������)
				Command_INSTEXT( FALSE, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE );
				// To Here Jun. 6, 2005 �����
			}
		}
		else
		{
			/* �{���͌��R�[�h���g���ׂ��Ȃ�ł��傤���A���ʂȏ���������邽�߂ɒ��ڂ������B
			** ��m_nSelectXXX��-1�̎��� m_pCommanderView->ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽�ߒ��ڂ������̂�߂��B2003.05.18 �����
			*/
			Command_INSTEXT( FALSE, szREPLACEKEY, nREPLACEKEY, TRUE );
			++nReplaceNum;
		}

		// �}����̈ʒu����
		if( nReplaceTarget == 1 )
		{
			GetCaret().SetCaretLayoutPos(GetCaret().GetCaretLayoutPos()+ptTmp);
			if (!bBeginBoxSelect)
			{
				CLogicPoint p;
				rLayoutMgr.LayoutToLogic(
					GetCaret().GetCaretLayoutPos(),
					&p
				);
				GetCaret().SetCaretLogicPos(p);
			}
		}

		// �Ō�ɒu�������ʒu���L��
		ptLast = GetCaret().GetCaretLayoutPos();

		/* �u����̈ʒu���m�F */
		if( bSelectedArea )
		{
			// �������u���̍s�␳�l�擾
			if( bBeginBoxSelect )
			{
				colDif += (Int)(ptLast.GetX2() - ptOld.GetX2());
				linDif += (Int)(rLayoutMgr.GetLineCount() - lineCnt);
			}
			else{
				// �u���O�̌���������̍ŏI�ʒu�� ptOld
				// �u����̃J�[�\���ʒu
				CLogicPoint ptTmp2 = GetCaret().GetCaretLogicPos();
				int linDif_thistime = rDocLineMgr.GetLineCount() - lineCnt;	// ����u���ł̍s���ω�
				linDif += linDif_thistime;
				if( ptColLineP.y + linDif == ptTmp2.y)
				{
					// �ŏI�s�Œu���������A���́A�u���̌��ʁA�I���G���A�ŏI�s�܂œ��B������
					// �ŏI�s�Ȃ̂ŁA�u���O��̕������̑����Ō��ʒu�𒲐�����
					colDif += (Int)ptTmp2.GetX2() - (Int)ptOld.GetX2(); //$$ �P�ʍ���

					// �A���A�ȉ��̏ꍇ�͒u���O��ōs���قȂ��Ă��܂��̂ŁA�s�̒����ŕ␳����K�v������
					// �P�j�ŏI�s���O�ōs�A�����N����A�s�������Ă���ꍇ�i�s�A���Ȃ̂ŁA���ʒu�͒u����̃J�[�\�����ʒu����������j
					// �@�@ptTmp2.x-ptOld.x���ƁA\r\n �� "" �u���ōs�A�������ꍇ�ɁA���ʒu�����ɂȂ莸�s����i���Ƃ͑O�s�̌��̕��ɂȂ邱�ƂȂ̂ŕ␳����j
					// �@�@����u���ł̍s���̕ω�(linDif_thistime)�ŁA�ŏI�s���s�A�����ꂽ���ǂ��������邱�Ƃɂ���
					// �Q�j���s��u�������iptTmp2.y!=ptOld.y�j�ꍇ�A���s��u������ƒu����̌��ʒu�����s�̌��ʒu�ɂȂ��Ă��邽��
					//     ptTmp2.x-ptOld.x���ƁA���̐��ƂȂ�A\r\n �� \n �� \n �� "abc" �ȂǂŌ��ʒu�������
					//     ������O�s�̒����Œ�������K�v������
					if (linDif_thistime < 0 || ptTmp2.y != (Int)ptOld.y) { //$$ �P�ʍ���
						colDif += linOldLen;
					}
				}
			}
		}
		// To Here 2001.12.03 hor

		/* �������� */
		// 2004.05.30 Moca ���݂̌�����������g���Č�������
		Command_SEARCH_NEXT( false, bDisplayUpdate, 0, 0 );
	}

	if( 0 < nAllLineNum )
	{
		nNewPos = (Int)GetSelect().GetFrom().GetY2() >> nShiftCount;
		::SendMessageAny( hwndProgress, PBM_SETPOS, nNewPos, 0 );
	}
	//>> 2002/03/26 Azumaiya

	_itot( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	if( !cDlgCancel.IsCanceled() ){
		nNewPos = nAllLineNum;
		::SendMessageAny( hwndProgress, PBM_SETPOS, nNewPos, 0 );
	}
	cDlgCancel.CloseDialog( 0 );
	::EnableWindow( m_pCommanderView->GetHwnd(), TRUE );
	::EnableWindow( ::GetParent( m_pCommanderView->GetHwnd() ), TRUE );
	::EnableWindow( ::GetParent( ::GetParent( m_pCommanderView->GetHwnd() ) ), TRUE );

	// From Here 2001.12.03 hor

	/* �e�L�X�g�I������ */
	m_pCommanderView->GetSelectionInfo().DisableSelectArea( TRUE );

	/* �J�[�\���E�I��͈͕��� */
	if((!bSelectedArea) ||			// �t�@�C���S�̒u��
	   (cDlgCancel.IsCanceled())) {		// �L�����Z�����ꂽ
		// �Ō�ɒu������������̉E��
		GetCaret().MoveCursor( ptLast, TRUE );
	}
	else{
		if (bBeginBoxSelect) {
			// ��`�I��
			m_pCommanderView->GetSelectionInfo().SetBoxSelect(bBeginBoxSelect);
			sRangeA.GetToPointer()->y += linDif;
			if(sRangeA.GetTo().y<0)sRangeA.SetToY(CLayoutInt(0));
		}
		else{
			// ���ʂ̑I��
			ptColLineP.x+=colDif;
			if(ptColLineP.x<0)ptColLineP.x=0;
			ptColLineP.y+=linDif;
			if(ptColLineP.y<0)ptColLineP.y=0;
			GetDocument()->m_cLayoutMgr.LogicToLayout(
				ptColLineP,
				sRangeA.GetToPointer()
			);
		}
		if(sRangeA.GetFrom().y<sRangeA.GetTo().y || sRangeA.GetFrom().x<sRangeA.GetTo().x){
			m_pCommanderView->GetSelectionInfo().SetSelectArea( sRangeA );	// 2009.07.25 ryoji
		}
		GetCaret().MoveCursor( sRangeA.GetTo(), TRUE );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX2();	// 2009.07.25 ryoji
	}
	// To Here 2001.12.03 hor

	GetEditWindow()->m_cDlgReplace.m_bCanceled = (cDlgCancel.IsCanceled() != FALSE);
	GetEditWindow()->m_cDlgReplace.m_nReplaceCnt=nReplaceNum;
	m_pCommanderView->SetDrawSwitch(true);
	ActivateFrameWindow( GetMainWindow() );
}



/* �J�[�\���s���E�B���h�E������ */
void CViewCommander::Command_CURLINECENTER( void )
{
	CLayoutInt		nViewTopLine;
	nViewTopLine = GetCaret().GetCaretLayoutPos().GetY2() - ( m_pCommanderView->GetTextArea().m_nViewRowNum / 2 );

	// sui 02/08/09
	if( 0 > nViewTopLine )	nViewTopLine = CLayoutInt(0);
	
	CLayoutInt nScrollLines = nViewTopLine - m_pCommanderView->GetTextArea().GetViewTopLine();	//Sep. 11, 2004 genta �����p�ɍs�����L��
	m_pCommanderView->GetTextArea().SetViewTopLine( nViewTopLine );
	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();
	// sui 02/08/09

	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	m_pCommanderView->SyncScrollV( nScrollLines );
}




/* Base64�f�R�[�h���ĕۑ� */
void CViewCommander::Command_BASE64DECODE( void )
{
	/* �e�L�X�g���I������Ă��邩 */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}
	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
	CNativeW	ctextBuf;
	if( !m_pCommanderView->GetSelectedData( &ctextBuf, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}

	/* Base64�f�R�[�h */
	CMemory cmemBuf;
	bool bret = CDecode_Base64Decode().CallDecode(ctextBuf, &cmemBuf);
	if( !bret ){
		return;
	}

	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */
	TCHAR		szPath[_MAX_PATH] = _T("");
	if( !GetDocument()->m_cDocFileOperation.SaveFileDialog( szPath ) ){
		return;
	}

	//�f�[�^
	int nDataLen;
	const void* pData = cmemBuf.GetRawPtr(&nDataLen);

	//�J�L�R
	CBinaryOutputStream out(szPath);
	if(!out)goto err;
	if( nDataLen != out.Write(pData, nDataLen) )goto err;

	return;

err:
	ErrorBeep();
	ErrorMessage( m_pCommanderView->GetHwnd(), _T("�t�@�C���̏������݂Ɏ��s���܂����B\n\n%ts"), szPath );
}




/* uudecode���ĕۑ� */
void CViewCommander::Command_UUDECODE( void )
{
	/* �e�L�X�g���I������Ă��邩 */
	if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
		ErrorBeep();
		return;
	}

	// �I��͈͂̃f�[�^���擾 -> cmemBuf
	// ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ�
	CNativeW	ctextBuf;
	if( !m_pCommanderView->GetSelectedData( &ctextBuf, FALSE, NULL, FALSE, GetDllShareData().m_Common.m_sEdit.m_bAddCRLFWhenCopy ) ){
		ErrorBeep();
		return;
	}

	// uudecode(�f�R�[�h)  ctextBuf -> cmemBin, szPath
	CMemory cmemBin;
	TCHAR szPath[_MAX_PATH]=_T("");
	CDecode_UuDecode decoder;
	if( !decoder.CallDecode(ctextBuf, &cmemBin) ){
		return;
	}
	decoder.CopyFilename( szPath );

	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */
	if( !GetDocument()->m_cDocFileOperation.SaveFileDialog( szPath ) ){
		return;
	}

	//�f�[�^
	int nDataLen;
	const void* pData = cmemBin.GetRawPtr(&nDataLen);

	//�J�L�R
	CBinaryOutputStream out(szPath);
	if( !out )goto err;
	if( nDataLen != out.Write(pData,nDataLen) )goto err;

	//����
	return;

err:
	ErrorBeep();
	ErrorMessage( m_pCommanderView->GetHwnd(), _T("�t�@�C���̏������݂Ɏ��s���܂����B\n\n%ts"), szPath );
}




/* �ĕ`�� */
void CViewCommander::Command_REDRAW( void )
{
	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();
	return;
}




/* Oracle SQL*Plus�Ŏ��s */
void CViewCommander::Command_PLSQL_COMPILE_ON_SQLPLUS( void )
{
//	HGLOBAL		hgClip;
//	char*		pszClip;
	HWND		hwndSQLPLUS;
	int			nRet;
	BOOL		nBool;
	TCHAR		szPath[MAX_PATH + 2];
	BOOL		bResult;

	hwndSQLPLUS = ::FindWindow( _T("SqlplusWClass"), _T("Oracle SQL*Plus") );
	if( NULL == hwndSQLPLUS ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("Oracle SQL*Plus�Ŏ��s���܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n") );
		return;
	}
	/* �e�L�X�g���ύX����Ă���ꍇ */
	if( GetDocument()->m_cDocEditor.IsModified() ){
		nRet = ::MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			GSTR_APPNAME,
			_T("%ts\n�͕ύX����Ă��܂��B Oracle SQL*Plus�Ŏ��s����O�ɕۑ����܂����H"),
			GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ? GetDocument()->m_cDocFile.GetFilePath() : _T("(����)")
		);
		switch( nRet ){
		case IDYES:
			if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
				//nBool = HandleCommand( F_FILESAVE, TRUE, 0, 0, 0, 0 );
				nBool = Command_FILESAVE();
			}else{
				//nBool = HandleCommand( F_FILESAVEAS_DIALOG, TRUE, 0, 0, 0, 0 );
				nBool = Command_FILESAVEAS_DIALOG();
			}
			if( !nBool ){
				return;
			}
			break;
		case IDNO:
			return;
		case IDCANCEL:
		default:
			return;
		}
	}
	if( GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		/* �t�@�C���p�X�ɋ󔒂��܂܂�Ă���ꍇ�̓_�u���N�H�[�e�[�V�����ň͂� */
		//	2003.10.20 MIK �R�[�h�ȗ���
		if( _tcschr( GetDocument()->m_cDocFile.GetFilePath(), TCODE::SPACE ) ? TRUE : FALSE ){
			auto_sprintf( szPath, _T("@\"%ts\"\r\n"), GetDocument()->m_cDocFile.GetFilePath() );
		}else{
			auto_sprintf( szPath, _T("@%ts\r\n"), GetDocument()->m_cDocFile.GetFilePath() );
		}
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		m_pCommanderView->MySetClipboardData( szPath, _tcslen( szPath ), false );

		/* Oracle SQL*Plus���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndSQLPLUS );

		/* Oracle SQL*Plus�Ƀy�[�X�g�̃R�}���h�𑗂� */
		DWORD_PTR	dwResult;
		bResult = ::SendMessageTimeout(
			hwndSQLPLUS,
			WM_COMMAND,
			MAKELONG( 201, 0 ),
			0,
			SMTO_ABORTIFHUNG | SMTO_NORMAL,
			3000,
			&dwResult
		);
		if( !bResult ){
			TopErrorMessage( m_pCommanderView->GetHwnd(), _T("Oracle SQL*Plus����̔���������܂���B\n���΂炭�҂��Ă���Ăю��s���Ă��������B") );
		}
	}else{
		ErrorBeep();
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("SQL���t�@�C���ɕۑ����Ȃ���Oracle SQL*Plus�Ŏ��s�ł��܂���B\n") );
		return;
	}
	return;
}




/* Oracle SQL*Plus���A�N�e�B�u�\�� */
void CViewCommander::Command_ACTIVATE_SQLPLUS( void )
{
	HWND		hwndSQLPLUS;
	hwndSQLPLUS = ::FindWindow( _T("SqlplusWClass"), _T("Oracle SQL*Plus") );
	if( NULL == hwndSQLPLUS ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("Oracle SQL*Plus���A�N�e�B�u�\�����܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n") );
		return;
	}
	/* Oracle SQL*Plus���A�N�e�B�u�ɂ��� */
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndSQLPLUS );
	return;
}




/* �r���[���[�h */
void CViewCommander::Command_VIEWMODE( void )
{
	//�r���[���[�h�𔽓]
	CAppMode::Instance()->SetViewMode(!CAppMode::Instance()->IsViewMode());

	// �e�E�B���h�E�̃^�C�g�����X�V
	this->GetEditWindow()->UpdateCaption();
}

/* �t�@�C���̃v���p�e�B */
void CViewCommander::Command_PROPERTY_FILE( void )
{
#ifdef _DEBUG
	{
		/* �S�s�f�[�^��Ԃ��e�X�g */
		wchar_t*	pDataAll;
		int		nDataAllLen;
		CRunningTimer cRunningTimer( "CViewCommander::Command_PROPERTY_FILE �S�s�f�[�^��Ԃ��e�X�g" );
		cRunningTimer.Reset();
		pDataAll = CDocReader(GetDocument()->m_cDocLineMgr).GetAllData( &nDataAllLen );
//		MYTRACE_A( "�S�f�[�^�擾             (%d�o�C�g) ���v����(�~���b) = %d\n", nDataAllLen, cRunningTimer.Read() );
		free( pDataAll );
		pDataAll = NULL;
//		MYTRACE_A( "�S�f�[�^�擾�̃������J�� (%d�o�C�g) ���v����(�~���b) = %d\n", nDataAllLen, cRunningTimer.Read() );
	}
#endif


	CDlgProperty	cDlgProperty;
//	cDlgProperty.Create( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument() );
	cDlgProperty.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)GetDocument() );
	return;
}




/* �ҏW�̑S�I�� */	// 2007.02.13 ryoji �ǉ�
void CViewCommander::Command_EXITALLEDITORS( void )
{
	CControlTray::CloseAllEditor( TRUE, GetMainWindow(), TRUE, 0 );
	return;
}

/* �T�N���G�f�B�^�̑S�I�� */	//Dec. 27, 2000 JEPRO �ǉ�
void CViewCommander::Command_EXITALL( void )
{
	CControlTray::TerminateApplication( GetMainWindow() );	// 2006.12.25 ryoji �����ǉ�
	return;
}




/* �O���[�v����� */	// 2007.06.20 ryoji �ǉ�
void CViewCommander::Command_GROUPCLOSE( void )
{
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin ){
		int nGroup = CAppNodeManager::Instance()->GetEditNode( GetMainWindow() )->GetGroup();
		CControlTray::CloseAllEditor( TRUE, GetMainWindow(), TRUE, nGroup );
	}
	return;
}

/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
void CViewCommander::Command_FILECLOSEALL( void )
{
	int nGroup = CAppNodeManager::Instance()->GetEditNode( GetMainWindow() )->GetGroup();
	CControlTray::CloseAllEditor( TRUE, GetMainWindow(), FALSE, nGroup );	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
	return;
}




/* �E�B���h�E����� */
void CViewCommander::Command_WINCLOSE( void )
{
	/* ���� */
	::PostMessageAny( GetMainWindow(), MYWM_CLOSE, FALSE, 0 );	// 2007.02.13 ryoji WM_CLOSE��MYWM_CLOSE�ɕύX
	return;
}

//�A�E�g�v�b�g�E�B���h�E�\��
void CViewCommander::Command_WIN_OUTPUT( void )
{
	// 2010.05.11 Moca CShareData::OpenDebugWindow()�ɓ���
	// ���b�Z�[�W�\���E�B���h�E��View����e�ɕύX
	// TraceOut�o�R�ł�CODE_UNICODE,������ł�CODE_SJIS�������̂𖳎w��ɕύX
	CShareData::getInstance()->OpenDebugWindow( GetMainWindow(), true );
	return;
}




/* �J�X�^�����j���[�\�� */
int CViewCommander::Command_CUSTMENU( int nMenuIdx )
{
	HMENU		hMenu;
	int			nId;
	POINT		po;
	int			i;
	UINT		uFlags;

	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->GetMenuDrawer().ResetContents();
	
	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = GetDocument()->m_cFuncLookup;

	if( nMenuIdx < 0 || MAX_CUSTOM_MENU <= nMenuIdx ){
		return 0;
	}
	if( 0 == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx] ){
		return 0;
	}
	hMenu = ::CreatePopupMenu();
	for( i = 0; i < GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( 0 == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		}else{
			//	Oct. 3, 2001 genta
			WCHAR		szLabel[300];
			WCHAR		szLabel2[300];
			const WCHAR*	pszMenuLabel = szLabel2;
			FuncLookup.Funccode2Name( GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* �L�[ */
			if( L'\0' == GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				pszMenuLabel = szLabel;
			}else{
				auto_sprintf( szLabel2, LTEXT("%ls (&%hc)"),
					szLabel,
					GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nMenuIdx][i]
				);
			}
			/* �@�\�����p�\�����ׂ� */
			if( IsFuncEnable( GetDocument(), &GetDllShareData(), GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
			pCEditWnd->GetMenuDrawer().MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				GetDllShareData().m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nMenuIdx][i] , pszMenuLabel );
		}
	}
	po.x = 0;
	po.y = 0;
	//2002/04/08 YAZAKI �J�X�^�����j���[���}�E�X�J�[�\���̈ʒu�ɕ\������悤�ɕύX�B
	::GetCursorPos( &po );
	po.y -= 4;
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		,
		po.x,
		po.y,
		0,
		GetMainWindow(),
		NULL
	);
	::DestroyMenu( hMenu );
	return nId;
}




//�I��͈͓��S�s�R�s�[
void CViewCommander::Command_COPYLINES( void )
{
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	m_pCommanderView->CopySelectedAllLines(
		NULL,	/* ���p�� */
		FALSE	/* �s�ԍ���t�^���� */
	);
	return;
}




//�I��͈͓��S�s���p���t���R�s�[
void CViewCommander::Command_COPYLINESASPASSAGE( void )
{
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	m_pCommanderView->CopySelectedAllLines(
		GetDllShareData().m_Common.m_sFormat.m_szInyouKigou,	/* ���p�� */
		FALSE 									/* �s�ԍ���t�^���� */
	);
	return;
}




//�I��͈͓��S�s�s�ԍ��t���R�s�[
void CViewCommander::Command_COPYLINESWITHLINENUMBER( void )
{
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	m_pCommanderView->CopySelectedAllLines(
		NULL,	/* ���p�� */
		TRUE	/* �s�ԍ���t�^���� */
	);
	return;
}




////�L�[���蓖�Ĉꗗ���R�s�[
	//Dec. 26, 2000 JEPRO //Jan. 24, 2001 JEPRO debug version (directed by genta)
void CViewCommander::Command_CREATEKEYBINDLIST( void )
{
	CNativeW		cMemKeyList;

	CKeyBind::CreateKeyBindList(
		G_AppInstance(),
		GetDllShareData().m_Common.m_sKeyBind.m_nKeyNameArrNum,
		GetDllShareData().m_Common.m_sKeyBind.m_pKeyNameArr,
		cMemKeyList,
		&GetDocument()->m_cFuncLookup,	//	Oct. 31, 2001 genta �ǉ�
		FALSE	// 2007.02.22 ryoji �ǉ�
	);

	// Windows�N���b�v�{�[�h�ɃR�s�[
	//2004.02.17 Moca �֐���
	SetClipboardText( CEditWnd::Instance()->m_cSplitterWnd.GetHwnd(), cMemKeyList.GetStringPtr(), cMemKeyList.GetStringLength() );
}

/* �t�@�C�����e��r */
void CViewCommander::Command_COMPARE( void )
{
	HWND		hwndCompareWnd;
	TCHAR		szPath[_MAX_PATH + 1];
	CMyPoint	poDes;
	CDlgCompare	cDlgCompare;
	BOOL		bDefferent;
	const wchar_t*	pLineSrc;
	CLogicInt		nLineLenSrc;
	const wchar_t*	pLineDes;
	int			nLineLenDes;
	HWND		hwndMsgBox;	//@@@ 2003.06.12 MIK

	/* ��r��A���E�ɕ��ׂĕ\�� */
	cDlgCompare.m_bCompareAndTileHorz = GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz;
	BOOL bDlgCompareResult = cDlgCompare.DoModal(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		(LPARAM)GetDocument(),
		GetDocument()->m_cDocFile.GetFilePath(),
		GetDocument()->m_cDocEditor.IsModified(),
		szPath,
		&hwndCompareWnd
	);
	if( !bDlgCompareResult ){
		return;
	}
	/* ��r��A���E�ɕ��ׂĕ\�� */
	GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz = cDlgCompare.m_bCompareAndTileHorz;

	//�^�u�E�C���h�E���͋֎~	//@@@ 2003.06.12 MIK
	if( TRUE  == GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd
	 && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		hwndMsgBox = m_pCommanderView->GetHwnd();
		GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz = FALSE;
	}
	else
	{
		hwndMsgBox = hwndCompareWnd;
	}


	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CLogicPoint	poSrc;
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&poSrc
	);

	// �J�[�\���ʒu�擾 -> poDes
	{
		::SendMessageAny( hwndCompareWnd, MYWM_GETCARETPOS, 0, 0 );
		CLogicPoint* ppoCaretDes = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<CLogicPoint>();
		poDes.x = ppoCaretDes->x;
		poDes.y = ppoCaretDes->y;
	}
	bDefferent = TRUE;
	pLineSrc = GetDocument()->m_cDocLineMgr.GetLine(poSrc.GetY2())->GetDocLineStrWithEOL(&nLineLenSrc);
	/* �s(���s�P��)�f�[�^�̗v�� */
	nLineLenDes = ::SendMessageAny( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	pLineDes = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<EDIT_CHAR>();
	while( 1 ){
		if( pLineSrc == NULL &&	0 == nLineLenDes ){
			bDefferent = FALSE;
			break;
		}
		if( pLineSrc == NULL || 0 == nLineLenDes ){
			break;
		}
		if( nLineLenDes > (int)GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>() ){
			TopErrorMessage( m_pCommanderView->GetHwnd(),
				_T("��r��̃t�@�C��\n%ts\n%d�����𒴂���s������܂��B\n")
				_T("��r�ł��܂���B"),
				szPath,
				GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<EDIT_CHAR>()
			);
			return;
		}
		for( ; poSrc.x < nLineLenSrc; ){
			if( poDes.x >= nLineLenDes ){
				goto end_of_compare;
			}
			if( pLineSrc[poSrc.x] != pLineDes[poDes.x] ){
				goto end_of_compare;
			}
			poSrc.x++;
			poDes.x++;
		}
		if( poDes.x < nLineLenDes ){
			goto end_of_compare;
		}
		poSrc.x = 0;
		poSrc.y++;
		poDes.x = 0;
		poDes.y++;
		pLineSrc = GetDocument()->m_cDocLineMgr.GetLine(poSrc.GetY2())->GetDocLineStrWithEOL(&nLineLenSrc);
		/* �s(���s�P��)�f�[�^�̗v�� */
		nLineLenDes = ::SendMessageAny( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	}
end_of_compare:;
	/* ��r��A���E�ɕ��ׂĕ\�� */
//From Here Oct. 10, 2000 JEPRO	�`�F�b�N�{�b�N�X���{�^��������Έȉ��̍s(To Here �܂�)�͕s�v�̂͂�����
//	���܂������Ȃ������̂Ō��ɖ߂��Ă���c
	if( GetDllShareData().m_Common.m_sCompare.m_bCompareAndTileHorz ){
		HWND* phwndArr = new HWND[2];
		phwndArr[0] = GetMainWindow();
		phwndArr[1] = hwndCompareWnd;
		
		int i;	// Jan. 28, 2002 genta ���[�v�ϐ� int�̐錾��O�ɏo�����D
				// �݊����΍�Dfor��()���Ő錾����ƌÂ��K�i�ƐV�����K�i�Ŗ�������̂ŁD
		for( i = 0; i < 2; ++i ){
			if( ::IsZoomed( phwndArr[i] ) ){
				::ShowWindow( phwndArr[i], SW_RESTORE );
			}
		}
		//	�f�X�N�g�b�v�T�C�Y�𓾂� 2002.1.24 YAZAKI
		RECT	rcDesktop;
		//	May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect( phwndArr[0], &rcDesktop );
		int width = (rcDesktop.right - rcDesktop.left ) / 2;
		for( i = 1; i >= 0; i-- ){
			::SetWindowPos(
				phwndArr[i], 0,
				width * i + rcDesktop.left, rcDesktop.top, // Oct. 18, 2003 genta �^�X�N�o�[�����ɂ���ꍇ���l��
				width, rcDesktop.bottom - rcDesktop.top,
				SWP_NOOWNERZORDER | SWP_NOZORDER
			);
		}
//		::TileWindows( NULL, MDITILE_VERTICAL, NULL, 2, phwndArr );
		delete [] phwndArr;
	}
//To Here Oct. 10, 2000

	//	2002/05/11 YAZAKI �e�E�B���h�E�����܂��ݒ肵�Ă݂�B
	if( !bDefferent ){
		TopInfoMessage( hwndMsgBox, _T("�قȂ�ӏ��͌�����܂���ł����B") );
	}
	else{
//		TopInfoMessage( hwndMsgBox, _T("�قȂ�ӏ���������܂����B") );
		/* �J�[�\�����ړ�������
			��r����́A�ʃv���Z�X�Ȃ̂Ń��b�Z�[�W���΂��B
		*/
		memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poDes, sizeof( poDes ) );
		::SendMessageAny( hwndCompareWnd, MYWM_SETCARETPOS, 0, 0 );

		/* �J�[�\�����ړ������� */
		memcpy_raw( GetDllShareData().m_sWorkBuffer.GetWorkBuffer<void>(), &poSrc, sizeof( poSrc ) );
		::PostMessageAny( GetMainWindow(), MYWM_SETCARETPOS, 0, 0 );
		TopWarningMessage( hwndMsgBox, _T("�قȂ�ӏ���������܂����B") );	// �ʒu��ύX���Ă��烁�b�Z�[�W	2008/4/27 Uchi
	}

	/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( GetMainWindow() );
	return;
}




/*! �c�[���o�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutToolBar(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void CViewCommander::Command_SHOWTOOLBAR( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispTOOLBAR = ((NULL == pCEditWnd->m_cToolbar.GetToolbarHwnd())? TRUE: FALSE);	/* �c�[���o�[�\�� */
	pCEditWnd->LayoutToolBar();
	pCEditWnd->EndLayoutBars();

	//�S�E�C���h�E�ɕύX��ʒm����B
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TOOLBAR,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}




/*! �X�e�[�^�X�o�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutStatusBar(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void CViewCommander::Command_SHOWSTATUSBAR( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispSTATUSBAR = ((NULL == pCEditWnd->m_cStatusBar.GetStatusHwnd())? TRUE: FALSE);	/* �X�e�[�^�X�o�[�\�� */
	pCEditWnd->LayoutStatusBar();
	pCEditWnd->EndLayoutBars();

	//�S�E�C���h�E�ɕύX��ʒm����B
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_STATUSBAR,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}




/*! �t�@���N�V�����L�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutFuncKey(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void CViewCommander::Command_SHOWFUNCKEY( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sWindow.m_bDispFUNCKEYWND = ((NULL == pCEditWnd->m_CFuncKeyWnd.GetHwnd())? TRUE: FALSE);	/* �t�@���N�V�����L�[�\�� */
	pCEditWnd->LayoutFuncKey();
	pCEditWnd->EndLayoutBars();

	//�S�E�C���h�E�ɕύX��ʒm����B
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_FUNCKEY,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}

//@@@ From Here 2003.06.10 MIK
/*! �^�u(�E�C���h�E)�̕\��/��\��

	@author MIK
	@date 2003.06.10 �V�K�쐬
	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutTabBar(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
	@date 2007.06.20 ryoji �O���[�vID���Z�b�g
 */
void CViewCommander::Command_SHOWTAB( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd = ((NULL == pCEditWnd->m_cTabWnd.GetHwnd())? TRUE: FALSE);	/* �^�u�o�[�\�� */
	pCEditWnd->LayoutTabBar();
	pCEditWnd->EndLayoutBars();

	// �܂Ƃ߂�Ƃ��� WS_EX_TOPMOST ��Ԃ𓯊�����	// 2007.05.18 ryoji
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd && !GetDllShareData().m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		GetDocument()->m_pcEditWnd->WindowTopMost(
			( (DWORD)::GetWindowLongPtr( GetDocument()->m_pcEditWnd->GetHwnd(), GWL_EXSTYLE ) & WS_EX_TOPMOST )? 1: 2
		);
	}

	//�S�E�C���h�E�ɕύX��ʒm����B
	CAppNodeManager::Instance()->ResetGroupId();
	CAppNodeGroupHandle(0).PostMessageToAllEditors(
		MYWM_BAR_CHANGE_NOTIFY,
		(WPARAM)MYBCN_TAB,
		(LPARAM)pCEditWnd->GetHwnd(),
		pCEditWnd->GetHwnd()
	);
}
//@@@ To Here 2003.06.10 MIK



/* ��� */
void CViewCommander::Command_PRINT( void )
{
	// �g���Ă��Ȃ��������폜 2003.05.04 �����
	Command_PRINT_PREVIEW();
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* ������s */
	pCEditWnd->m_pPrintPreview->OnPrint();
}




/* ����v���r���[ */
void CViewCommander::Command_PRINT_PREVIEW( void )
{
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* ����v���r���[���[�h�̃I��/�I�t */
	pCEditWnd->PrintPreviewModeONOFF();
	return;
}




/* ����̃y�[�W���C�A�E�g�̐ݒ� */
void CViewCommander::Command_PRINT_PAGESETUP( void )
{
	BOOL		bRes;
	CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* ����y�[�W�ݒ� */
	bRes = pCEditWnd->OnPrintPageSetting();
	return;
}




/* �u���E�Y */
void CViewCommander::Command_BROWSE( void )
{
	if( !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() ){
		ErrorBeep();
		return;
	}
//	char	szURL[MAX_PATH + 64];
//	auto_sprintf( szURL, L"%ls", GetDocument()->m_cDocFile.GetFilePath() );
	/* URL���J�� */
//	::ShellExecuteEx( NULL, L"open", szURL, NULL, NULL, SW_SHOW );

    SHELLEXECUTEINFO info; 
    info.cbSize =sizeof(info);
    info.fMask = 0;
    info.hwnd = NULL;
    info.lpVerb = NULL;
    info.lpFile = GetDocument()->m_cDocFile.GetFilePath();
    info.lpParameters = NULL;
    info.lpDirectory = NULL;
    info.nShow = SW_SHOWNORMAL;
    info.hInstApp = 0;
    info.lpIDList = NULL;
    info.lpClass = NULL;
    info.hkeyClass = 0; 
    info.dwHotKey = 0;
    info.hIcon =0;

	::ShellExecuteEx(&info);

	return;
}




/* �L�[�}�N���̋L�^�J�n�^�I�� */
void CViewCommander::Command_RECKEYMACRO( void )
{
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro ){									/* �L�[�{�[�h�}�N���̋L�^�� */
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;							/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
		//@@@ 2002.1.24 YAZAKI �L�[�}�N�����}�N���p�t�H���_�ɁuRecKey.mac�v�Ƃ������ŕۑ�
		TCHAR szInitDir[MAX_PATH];
		int nRet;
		// 2003.06.23 Moca �L�^�p�L�[�}�N���̃t���p�X��CShareData�o�R�Ŏ擾
		nRet = CShareData::getInstance()->GetMacroFilename( -1, szInitDir, MAX_PATH ); 
		if( nRet <= 0 ){
			ErrorMessage( m_pCommanderView->GetHwnd(), _T("�}�N���t�@�C�����쐬�ł��܂���ł����B\n�t�@�C�����̎擾�G���[ nRet=%d"), nRet );
			return;
		}else{
			_tcscpy( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szInitDir );
		}
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		int nSaveResult=CEditApp::Instance()->m_pcSMacroMgr->Save(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName
		);
		if ( !nSaveResult ){
			ErrorMessage(	m_pCommanderView->GetHwnd(), _T("�}�N���t�@�C�����쐬�ł��܂���ł����B\n\n%ts"), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
	}else{
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = GetMainWindow();;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
		/* �L�[�}�N���̃o�b�t�@���N���A���� */
		//@@@ 2002.1.24 m_CKeyMacroMgr��CEditDoc�ֈړ�
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		CEditApp::Instance()->m_pcSMacroMgr->Clear(STAND_KEYMACRO);
//		GetDocument()->m_CKeyMacroMgr.ClearAll();
//		GetDllShareData().m_CKeyMacroMgr.Clear();
	}
	/* �e�E�B���h�E�̃^�C�g�����X�V */
	this->GetEditWindow()->UpdateCaption();

	/* �L�����b�g�̍s���ʒu��\������ */
	GetCaret().ShowCaretPosInfo();
}




/* �L�[�}�N���̕ۑ� */
void CViewCommander::Command_SAVEKEYMACRO( void )
{
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

	//	Jun. 16, 2002 genta
	if( !CEditApp::Instance()->m_pcSMacroMgr->IsSaveOk() ){
		//	�ۑ��s��
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("�ۑ��\�ȃ}�N��������܂���D�L�[�{�[�h�}�N���ȊO�͕ۑ��ł��܂���D") );
	}

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	_tcscpy( szPath, _T("") );
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if( _IS_REL_PATH( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER ) ){
		GetInidirOrExedir( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );
	}else{
		_tcscpy( szInitDir, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER );	/* �}�N���p�t�H���_ */
	}
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
		_T("*.mac"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, NULL );
//	wcscat( GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER, L"\\" );

	/* �L�[�{�[�h�}�N���̕ۑ� */
	//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
	//@@@ 2002.1.24 YAZAKI
	if ( !CEditApp::Instance()->m_pcSMacroMgr->Save( STAND_KEYMACRO, G_AppInstance(), szPath ) ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("�}�N���t�@�C�����쐬�ł��܂���ł����B\n\n%ts"), szPath );
	}
	return;
}




/* �L�[�}�N���̎��s */
void CViewCommander::Command_EXECKEYMACRO( void )
{
	//@@@ 2002.1.24 YAZAKI �L�^���͏I�����Ă�����s
	if (GetDllShareData().m_sFlags.m_bRecordingKeyMacro){
		Command_RECKEYMACRO();
	}
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

	/* �L�[�{�[�h�}�N���̎��s */
	//@@@ 2002.1.24 YAZAKI
	if ( GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName[0] ){
		//	�t�@�C�����ۑ�����Ă�����
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		BOOL bLoadResult = CEditApp::Instance()->m_pcSMacroMgr->Load(
			STAND_KEYMACRO,
			G_AppInstance(),
			GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName,
			NULL
		);
		if ( !bLoadResult ){
			ErrorMessage( m_pCommanderView->GetHwnd(), _T("�t�@�C�����J���܂���ł����B\n\n%ts"), GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName );
		}
		else {
			//	2007.07.20 genta : flags�I�v�V�����ǉ�
			CEditApp::Instance()->m_pcSMacroMgr->Exec( STAND_KEYMACRO, G_AppInstance(), m_pCommanderView, 0 );
		}
	}

	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();

	return;
}




/*! �L�[�}�N���̓ǂݍ���
	@date 2005/02/20 novice �f�t�H���g�̊g���q�ύX
 */
void CViewCommander::Command_LOADKEYMACRO( void )
{
	GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
	GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	const TCHAR*		pszFolder;
	_tcscpy( szPath, _T("") );
	pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	if( _IS_REL_PATH( pszFolder ) ){
		GetInidirOrExedir( szInitDir, pszFolder );
	}else{
		_tcscpy( szInitDir, pszFolder );	/* �}�N���p�t�H���_ */
	}
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		G_AppInstance(),
		m_pCommanderView->GetHwnd(),
// 2005/02/20 novice �f�t�H���g�̊g���q�ύX
// 2005/07/13 novice ���l�ȃ}�N�����T�|�[�g���Ă���̂Ńf�t�H���g�͑S�ĕ\���ɂ���
		_T("*.*"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}

	/* �L�[�{�[�h�}�N���̓ǂݍ��� */
	//@@@ 2002.1.24 YAZAKI �ǂݍ��݂Ƃ������A�t�@�C�������R�s�[���邾���B���s���O�ɓǂݍ���
	_tcscpy(GetDllShareData().m_Common.m_sMacro.m_szKeyMacroFileName, szPath);
//	GetDllShareData().m_CKeyMacroMgr.LoadKeyMacro( G_AppInstance(), m_pCommanderView->GetHwnd(), szPath );
	return;
}


/*! ���O���w�肵�ă}�N�����s
	@param pszPath	�}�N���̃t�@�C���p�X�A�܂��̓}�N���̃R�[�h�B
	@param pszType	��ʁBNULL�̏ꍇ�t�@�C���w��A����ȊO�̏ꍇ�͌���̊g���q���w��

	@date 2008.10.23 syat �V�K�쐬
	@date 2008.12.21 syat �����u��ʁv��ǉ�
 */
void CViewCommander::Command_EXECEXTMACRO( const WCHAR* pszPathW, const WCHAR* pszTypeW )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];	//�t�@�C���I���_�C�A���O�̏����t�H���_
	const TCHAR*	pszFolder;					//�}�N���t�H���_
	const TCHAR*	pszPath = NULL;				//��1������TCHAR*�ɕϊ�����������
	const TCHAR*	pszType = NULL;				//��2������TCHAR*�ɕϊ�����������
	HWND			hwndRecordingKeyMacro = NULL;

	if ( pszPathW != NULL ) {
		//to_tchar()�Ŏ擾�����������delete���Ȃ����ƁB
		pszPath = to_tchar( pszPathW );
		pszType = to_tchar( pszTypeW );

	} else {
		// �t�@�C�����w�肳��Ă��Ȃ��ꍇ�A�_�C�A���O��\������
		_tcscpy( szPath, _T("") );
		pszFolder = GetDllShareData().m_Common.m_sMacro.m_szMACROFOLDER;

		if( _IS_REL_PATH( pszFolder ) ){
			GetInidirOrExedir( szInitDir, pszFolder );
		}else{
			_tcscpy( szInitDir, pszFolder );	/* �}�N���p�t�H���_ */
		}
		/* �t�@�C���I�[�v���_�C�A���O�̏����� */
		cDlgOpenFile.Create(
			G_AppInstance(),
			m_pCommanderView->GetHwnd(),
			_T("*.*"),
			szInitDir
		);
		if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
			return;
		}
		pszPath = szPath;
		pszType = NULL;
	}

	//�L�[�}�N���L�^���̏ꍇ�A�ǉ�����
	if( GetDllShareData().m_sFlags.m_bRecordingKeyMacro &&									/* �L�[�{�[�h�}�N���̋L�^�� */
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow()	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	){
		CEditApp::Instance()->m_pcSMacroMgr->Append( STAND_KEYMACRO, F_EXECEXTMACRO, (LPARAM)pszPath, m_pCommanderView );

		//�L�[�}�N���̋L�^���ꎞ��~����
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = FALSE;
		hwndRecordingKeyMacro = GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	}

	//�Â��ꎞ�}�N���̑ޔ�
	CMacroManagerBase* oldMacro = CEditApp::Instance()->m_pcSMacroMgr->SetTempMacro( NULL );

	BOOL bLoadResult = CEditApp::Instance()->m_pcSMacroMgr->Load(
		TEMP_KEYMACRO,
		G_AppInstance(),
		pszPath,
		pszType
	);
	if ( !bLoadResult ){
		ErrorMessage( m_pCommanderView->GetHwnd(), _T("�}�N���̓ǂݍ��݂Ɏ��s���܂����B\n\n%ts"), pszPath );
	}
	else {
		CEditApp::Instance()->m_pcSMacroMgr->Exec( TEMP_KEYMACRO, G_AppInstance(), m_pCommanderView, FA_NONRECORD | FA_FROMMACRO );
	}

	// �I�������J��
	CEditApp::Instance()->m_pcSMacroMgr->Clear( TEMP_KEYMACRO );
	if ( oldMacro != NULL ) {
		CEditApp::Instance()->m_pcSMacroMgr->SetTempMacro( oldMacro );
	}

	// �L�[�}�N���L�^���������ꍇ�͍ĊJ����
	if ( hwndRecordingKeyMacro != NULL ) {
		GetDllShareData().m_sFlags.m_bRecordingKeyMacro = TRUE;
		GetDllShareData().m_sFlags.m_hwndRecordingKeyMacro = hwndRecordingKeyMacro;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	}

	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();

	return;
}


/*! ���݂̃E�B���h�E���Ő܂�Ԃ�

	@date 2002.01.14 YAZAKI ���݂̃E�B���h�E���Ő܂�Ԃ���Ă���Ƃ��́A�ő�l�ɂ���悤��
	@date 2002.04.08 YAZAKI �Ƃ��ǂ��E�B���h�E���Ő܂�Ԃ���Ȃ����Ƃ�����o�O�C���B
	@date 2005.08.14 genta �����ł̐ݒ�͋��ʐݒ�ɔ��f���Ȃ��D
	@date 2005.10.22 aroka ���݂̃E�B���h�E�����ő�l�������^�C�v�̏����l ���g�O���ɂ���

	@note �ύX���鏇����ύX�����Ƃ���CEditWnd::InitMenu()���ύX���邱��
	@sa CEditWnd::InitMenu()
*/
void CViewCommander::Command_WRAPWINDOWWIDTH( void )	//	Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
{
	// Jan. 8, 2006 genta ���菈����m_pCommanderView->GetWrapMode()�ֈړ�
	CEditView::TOGGLE_WRAP_ACTION nWrapMode;
	CLayoutInt newKetas;
	
	nWrapMode = m_pCommanderView->GetWrapMode( &newKetas );
	GetDocument()->m_nTextWrapMethodCur = WRAP_SETTING_WIDTH;
	GetDocument()->m_bTextWrapMethodCurTemp = !( GetDocument()->m_nTextWrapMethodCur == GetDocument()->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod );
	if( nWrapMode == CEditView::TGWRAP_NONE ){
		return;	// �܂�Ԃ����͌��̂܂�
	}

	GetEditWindow()->ChangeLayoutParam( true, GetDocument()->m_cLayoutMgr.GetTabSpace(), newKetas );
	

	//	Aug. 14, 2005 genta ���ʐݒ�ւ͔��f�����Ȃ�
//	GetDocument()->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas = m_nViewColNum;

	m_pCommanderView->GetTextArea().SetViewLeftCol( CLayoutInt(0) );		/* �\����̈�ԍ��̌�(0�J�n) */

	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();
	return;
}




//�����}�[�N�̐ؑւ�	// 2001.12.03 hor �N���A �� �ؑւ� �ɕύX
void CViewCommander::Command_SEARCH_CLEARMARK( void )
{
// From Here 2001.12.03 hor

	//�����}�[�N�̃Z�b�g

	if(m_pCommanderView->GetSelectionInfo().IsTextSelected()){

		// ����������擾
		CNativeW	cmemCurText;
		m_pCommanderView->GetCurrentTextForSearch( cmemCurText );

		// ����������ݒ�
		int i,j;
		wcscpy( m_pCommanderView->m_szCurSrchKey, cmemCurText.GetStringPtr() );
		for( i = 0; i < GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size(); ++i ){
			if( 0 == wcscmp( m_pCommanderView->m_szCurSrchKey, GetDllShareData().m_sSearchKeywords.m_aSearchKeys[i] ) ){
				break;
			}
		}
		if( i < GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size() ){
			for( j = i; j > 0; j-- ){
				wcscpy( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[j], GetDllShareData().m_sSearchKeywords.m_aSearchKeys[j - 1] );
			}
		}
		else{
			for( j = MAX_SEARCHKEY - 1; j > 0; j-- ){
				wcscpy( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[j], GetDllShareData().m_sSearchKeywords.m_aSearchKeys[j - 1] );
			}
			GetDllShareData().m_sSearchKeywords.m_aSearchKeys.resize( t_min<int>(MAX_SEARCHKEY, GetDllShareData().m_sSearchKeywords.m_aSearchKeys.size()+1) );
		}
		wcscpy( GetDllShareData().m_sSearchKeywords.m_aSearchKeys[0], cmemCurText.GetStringPtr() );

		// �����I�v�V�����ݒ�
		GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bRegularExp=false;	//���K�\���g��Ȃ�
		GetDllShareData().m_Common.m_sSearch.m_sSearchOption.bWordOnly=false;		//�P��Ō������Ȃ�
		m_pCommanderView->ChangeCurRegexp(); // 2002.11.11 Moca ���K�\���Ō���������C�F�������ł��Ă��Ȃ�����

		// �ĕ`��
		m_pCommanderView->RedrawAll();
		return;
	}
// To Here 2001.12.03 hor

	//�����}�[�N�̃N���A

	m_pCommanderView->m_bCurSrchKeyMark = false;	/* ����������̃}�[�N */
	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	m_pCommanderView->RedrawAll();
	return;
}




//! �t�@�C���̍ăI�[�v��
void CViewCommander::Command_FILE_REOPEN(
	ECodeType	nCharCode,	//!< [in] �J�������ۂ̕����R�[�h
	bool		bNoConfirm	//!< [in] �t�@�C�����X�V���ꂽ�ꍇ�Ɋm�F���s��u�Ȃ��v���ǂ����Btrue:�m�F���Ȃ� false:�m�F����
)
{
	CEditDoc* pcDoc = GetDocument();
	if( !bNoConfirm && fexist(pcDoc->m_cDocFile.GetFilePath()) && pcDoc->m_cDocEditor.IsModified() ){
		int nDlgResult = MYMESSAGEBOX(
			m_pCommanderView->GetHwnd(),
			MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ts\n\n���̃t�@�C���͕ύX����Ă��܂��B\n�ă��[�h���s���ƕύX�������܂����A��낵���ł���?"),
			pcDoc->m_cDocFile.GetFilePath()
		);
		if( IDOK == nDlgResult ){
			//�p���B���֐i��
		}else{
			return; //���f
		}
	}

	// ����t�@�C���̍ăI�[�v��
	pcDoc->m_cDocFileOperation.ReloadCurrentFile( nCharCode );
}




//���t�}��
void CViewCommander::Command_INS_DATE( void )
{
	// ���t���t�H�[�}�b�g
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetDateFormat( systime, szText, _countof( szText ) - 1 );

	// �e�L�X�g��\��t�� ver1
	Command_INSTEXT( TRUE, to_wchar(szText), CLogicInt(-1), TRUE );
}




//�����}��
void CViewCommander::Command_INS_TIME( void )
{
	// �������t�H�[�}�b�g
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );

	// �e�L�X�g��\��t�� ver1
	Command_INSTEXT( TRUE, to_wchar(szText), CLogicInt(-1), TRUE );
}


/*! �O���R�}���h���s�_�C�A���O�\��
	@date 2002.02.02 YAZAKI.
*/
void CViewCommander::Command_EXECCOMMAND_DIALOG( void )
{
	CDlgExec cDlgExec;

	/* ���[�h���X�_�C�A���O�̕\�� */
	if( !cDlgExec.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), 0 ) ){
		return;
	}

	m_pCommanderView->AddToCmdArr( cDlgExec.m_szCommand );
	const WCHAR* cmd_string = to_wchar(cDlgExec.m_szCommand);

	//HandleCommand( F_EXECMD, TRUE, (LPARAM)cmd_string, 0, 0, 0);	//	�O���R�}���h���s�R�}���h�̔��s
	HandleCommand( F_EXECMD, TRUE, (LPARAM)cmd_string, (LPARAM)(GetDllShareData().m_nExecFlgOpt), 0, 0);	//	�O���R�}���h���s�R�}���h�̔��s	
}

//�O���R�}���h���s
//	Sept. 20, 2000 JEPRO  ����CMMAND��COMMAND�ɕύX
//	Oct. 9, 2001   genta  �}�N���Ή��̂��߈����ǉ�
//  2002.2.2       YAZAKI �_�C�A���O�Ăяo�����ƃR�}���h���s���𕪗�
//void CEditView::Command_EXECCOMMAND( const char *cmd_string )
void CViewCommander::Command_EXECCOMMAND( LPCWSTR cmd_string, const int nFlgOpt)	//	2006.12.03 maru �����̊g��
{
	//	From Here Aug. 21, 2001 genta
	//	�p�����[�^�u�� (���b��)
	const int bufmax = 1024;
	wchar_t buf[bufmax + 1];
	CSakuraEnvironment::ExpandParameter(cmd_string, buf, bufmax);
	
	// �q�v���Z�X�̕W���o�͂����_�C���N�g����
	//ExecCmd( buf, GetDllShareData().m_bGetStdout );	//	2006.12.03 maru �}�N������̌Ăяo���ł̓I�v�V������ۑ������Ȃ�����
	m_pCommanderView->ExecCmd( to_tchar(buf), nFlgOpt );
	//	To Here Aug. 21, 2001 genta
	return;
}






//	Jun. 16, 2000 genta
//	�Ί��ʂ̌���
void CViewCommander::Command_BRACKETPAIR( void )
{
	CLayoutPoint ptColLine;
	//int nLine, nCol;

	int mode = 3;
	/*
	bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
	bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
	bit2(out) : ���������ʒu         0:���      1:�O
	*/
	if( m_pCommanderView->SearchBracket( GetCaret().GetCaretLayoutPos(), &ptColLine, &mode ) ){	// 02/09/18 ai
		//	2005.06.24 Moca
		//	2006.07.09 genta �\���X�V�R��F�V�K�֐��ɂđΉ�
		m_pCommanderView->MoveCursorSelecting( ptColLine, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
	else{
		//	���s�����ꍇ�� nCol/nLine�ɂ͗L���Ȓl�������Ă��Ȃ�.
		//	�������Ȃ�
	}
}


//	���݈ʒu���ړ������ɓo�^����
void CViewCommander::Command_JUMPHIST_SET( void )
{
	m_pCommanderView->AddCurrentLineToHistory();
}


//	From HERE Sep. 8, 2000 genta
//	�ړ�������O�ւ��ǂ�
//
void CViewCommander::Command_JUMPHIST_PREV( void )
{
	// 2001.12.13 hor
	// �ړ������̍Ō�Ɍ��݂̈ʒu���L������
	// ( ���̗������擾�ł��Ȃ��Ƃ��͒ǉ����Ė߂� )
	if( !m_pCommanderView->m_cHistory->CheckNext() ){
		m_pCommanderView->AddCurrentLineToHistory();
		m_pCommanderView->m_cHistory->PrevValid();
	}

	if( m_pCommanderView->m_cHistory->CheckPrev() ){
		if( ! m_pCommanderView->m_cHistory->PrevValid() ){
			::MessageBox( NULL, _T("Inconsistent Implementation"), _T("PrevValid"), MB_OK );
		}
		CLayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(m_pCommanderView->m_cHistory->GetCurrent().GetPos(), m_pCommanderView->m_cHistory->GetCurrent().GetLineNo()),
			&pt
		);
		//	2006.07.09 genta �I�����l��
		m_pCommanderView->MoveCursorSelecting( pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
}

//	�ړ����������ւ��ǂ�
void CViewCommander::Command_JUMPHIST_NEXT( void )
{
	if( m_pCommanderView->m_cHistory->CheckNext() ){
		if( ! m_pCommanderView->m_cHistory->NextValid() ){
			::MessageBox( NULL, _T("Inconsistent Implementation"), _T("NextValid"), MB_OK );
		}

		CLayoutPoint pt;
		GetDocument()->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(m_pCommanderView->m_cHistory->GetCurrent().GetPos(), m_pCommanderView->m_cHistory->GetCurrent().GetLineNo()),
			&pt );

		//	2006.07.09 genta �I�����l��
		m_pCommanderView->MoveCursorSelecting( pt, m_pCommanderView->GetSelectionInfo().m_bSelectingLock );
	}
}
//	To HERE Sep. 8, 2000 genta

/* ���̃O���[�v */			// 2007.06.20 ryoji
void CViewCommander::Command_NEXTGROUP( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->NextGroup();
}

/* �O�̃O���[�v */			// 2007.06.20 ryoji
void CViewCommander::Command_PREVGROUP( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->PrevGroup();
}

/* �^�u���E�Ɉړ� */		// 2007.06.20 ryoji
void CViewCommander::Command_TAB_MOVERIGHT( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->MoveRight();
}

/* �^�u�����Ɉړ� */		// 2007.06.20 ryoji
void CViewCommander::Command_TAB_MOVELEFT( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->MoveLeft();
}

/* �V�K�O���[�v */			// 2007.06.20 ryoji
void CViewCommander::Command_TAB_SEPARATE( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->Separate();
}

/* ���̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji
void CViewCommander::Command_TAB_JOINTNEXT( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->JoinNext();
}

/* �O�̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji
void CViewCommander::Command_TAB_JOINTPREV( void )
{
	CTabWnd* pcTabWnd = &GetDocument()->m_pcEditWnd->m_cTabWnd;
	if( pcTabWnd->GetHwnd() == NULL )
		return;
	pcTabWnd->JoinPrev();
}

/* ���̃^�u�ȊO����� */	// 2008.11.22 syat
// 2009.12.26 syat ���̃E�B���h�E�ȊO�����Ƃ̌��p��
void CViewCommander::Command_TAB_CLOSEOTHER( void )
{
	int nGroup = 0;

	// �E�B���h�E�ꗗ���擾����
	EditNode* pEditNode;
	int nCount = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNode, TRUE );
	if( 0 >= nCount )return;

	for( int i = 0; i < nCount; i++ ){
		if( pEditNode[i].m_hWnd == GetMainWindow() ){
			pEditNode[i].m_hWnd = NULL;		//�������g�͕��Ȃ�
			nGroup = pEditNode[i].m_nGroup;
		}
	}

	//�I���v�����o��
	CAppNodeGroupHandle(nGroup).RequestCloseEditor( pEditNode, nCount, FALSE, TRUE, GetMainWindow() );
	delete []pEditNode;
	return;
}

/* �������ׂĕ��� */		// 2008.11.22 syat
void CViewCommander::Command_TAB_CLOSELEFT( void )
{
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd ){
		int nGroup = 0;

		// �E�B���h�E�ꗗ���擾����
		EditNode* pEditNode;
		int nCount = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNode, TRUE );
		BOOL bSelfFound = FALSE;
		if( 0 >= nCount )return;

		for( int i = 0; i < nCount; i++ ){
			if( pEditNode[i].m_hWnd == GetMainWindow() ){
				pEditNode[i].m_hWnd = NULL;		//�������g�͕��Ȃ�
				nGroup = pEditNode[i].m_nGroup;
				bSelfFound = TRUE;
			}else if( bSelfFound ){
				pEditNode[i].m_hWnd = NULL;		//�E�͕��Ȃ�
			}
		}

		//�I���v�����o��
		CAppNodeGroupHandle(nGroup).RequestCloseEditor( pEditNode, nCount, FALSE, TRUE, GetMainWindow() );
		delete []pEditNode;
	}
	return;
}

/* �E�����ׂĕ��� */		// 2008.11.22 syat
void CViewCommander::Command_TAB_CLOSERIGHT( void )
{
	if( GetDllShareData().m_Common.m_sTabBar.m_bDispTabWnd ){
		int nGroup = 0;

		// �E�B���h�E�ꗗ���擾����
		EditNode* pEditNode;
		int nCount = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNode, TRUE );
		BOOL bSelfFound = FALSE;
		if( 0 >= nCount )return;

		for( int i = 0; i < nCount; i++ ){
			if( pEditNode[i].m_hWnd == GetMainWindow() ){
				pEditNode[i].m_hWnd = NULL;		//�������g�͕��Ȃ�
				nGroup = pEditNode[i].m_nGroup;
				bSelfFound = TRUE;
			}else if( !bSelfFound ){
				pEditNode[i].m_hWnd = NULL;		//���͕��Ȃ�
			}
		}

		//�I���v�����o��
		CAppNodeGroupHandle(nGroup).RequestCloseEditor( pEditNode, nCount, FALSE, TRUE, GetMainWindow() );
		delete []pEditNode;
	}
	return;
}


/*!	@brief �ҏW���̓��e��ʖ��ۑ�

	��ɕҏW���̈ꎞ�t�@�C���o�͂Ȃǂ̖ړI�Ɏg�p����D
	���݊J���Ă���t�@�C��(m_szFilePath)�ɂ͉e�����Ȃ��D

	@retval	TRUE ����I��
	@retval	FALSE �t�@�C���쐬�Ɏ��s

	@author	maru
	@date	2006.12.10 maru �V�K�쐬
*/
BOOL CViewCommander::Command_PUTFILE(
	LPCWSTR		filename,	//!< [in] filename �o�̓t�@�C����
	ECodeType	nCharCode,	//!< [in] nCharCode �����R�[�h�w��
							//!<  @li CODE_xxxxxxxxxx:�e�핶���R�[�h
							//!<  @li CODE_AUTODETECT:���݂̕����R�[�h���ێ�
	int			nFlgOpt		//!< [in] nFlgOpt ����I�v�V����
							//!<  @li 0x01:�I��͈͂��o�� (��I����Ԃł���t�@�C�����o�͂���)
)
{
	BOOL		bResult = TRUE;
	ECodeType	nSaveCharCode = nCharCode;
	if(filename[0] == L'\0') {
		return FALSE;
	}
	
	if(nSaveCharCode == CODE_AUTODETECT) nSaveCharCode = GetDocument()->GetDocumentEncoding();
	
	//	2007.09.08 genta CEditDoc::FileWrite()�ɂȂ���č����v�J�[�\��
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	std::auto_ptr<CCodeBase> pcSaveCode( CCodeFactory::CreateCodeBase(nSaveCharCode,0) );
	
	if(nFlgOpt & 0x01)
	{	/* �I��͈͂��o�� */
		try
		{
			CBinaryOutputStream out(to_tchar(filename),true);

			//BOM�o��
			if ( GetDocument()->m_cDocFile.IsBomExist() ) {
				CMemory cmemBom;
				pcSaveCode->GetBom(&cmemBom);
				if(cmemBom.GetRawLength()>0)
					out.Write(cmemBom.GetRawPtr(),cmemBom.GetRawLength());
				else
					PleaseReportToAuthor( NULL, _T("CEditView::Command_PUTFILE/BOM Error\nSaveCharCode=%d"), nSaveCharCode );
			}

			// �I��͈͂̎擾 -> cMem
			CNativeW cMem;
			m_pCommanderView->GetSelectedData(&cMem, FALSE, NULL, FALSE, FALSE);

			// �������ݎ��̃R�[�h�ϊ� -> cDst
			CMemory cDst;
			pcSaveCode->UnicodeToCode(cMem, &cDst);

			//����
			if( 0 < cDst.GetRawLength() )
				out.Write(cDst.GetRawPtr(),cDst.GetRawLength());
		}
		catch(CError_FileOpen)
		{
			WarningMessage(
				NULL,
				_T("\'%ls\'\n")
				_T("�t�@�C����ۑ��ł��܂���B\n")
				_T("�p�X�����݂��Ȃ����A���̃A�v���P�[�V�����Ŏg�p����Ă���\��������܂��B"),
				filename
			);
			bResult = FALSE;
		}
		catch(CError_FileWrite)
		{
			WarningMessage(
				NULL,
				_T("�t�@�C���̏������ݒ��ɃG���[���������܂����B")
			);
			bResult = FALSE;
		}
	}
	else {	/* �t�@�C���S�̂��o�� */
		HWND		hwndProgress;
		CEditWnd*	pCEditWnd = GetDocument()->m_pcEditWnd;
		
		if( NULL != pCEditWnd ){
			hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
		}else{
			hwndProgress = NULL;
		}
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}

		// �ꎞ�t�@�C���o��
		EConvertResult eRet = CWriteManager().WriteFile_From_CDocLineMgr(
			GetDocument()->m_cDocLineMgr,
			SSaveInfo(
				to_tchar(filename),
				nSaveCharCode,
				EOL_NONE,
				GetDocument()->m_cDocFile.IsBomExist()
			)
		);
		bResult = (eRet != RESULT_FAILURE);

		if(hwndProgress) ::ShowWindow( hwndProgress, SW_HIDE );
	}
	return bResult;
}

/*!	@brief �J�[�\���ʒu�Ƀt�@�C����}��

	���݂̃J�[�\���ʒu�Ɏw��̃t�@�C����ǂݍ��ށD

	@param[in] filename ���̓t�@�C����
	@param[in] nCharCode �����R�[�h�w��
		@li	CODE_xxxxxxxxxx:�e�핶���R�[�h
		@li	CODE_AUTODETECT:�O�񕶎��R�[�h�������͎������ʂ̌��ʂɂ��
	@param[in] nFlgOpt ����I�v�V�����i���݂͖���`�D0���w��̂��Ɓj

	@retval	TRUE ����I��
	@retval	FALSE �t�@�C���I�[�v���Ɏ��s

	@author	maru
	@date	2006.12.10 maru �V�K�쐬
*/
BOOL CViewCommander::Command_INSFILE( LPCWSTR filename, ECodeType nCharCode, int nFlgOpt )
{
	CFileLoad	cfl;
	CEol cEol;
	int			nLineNum = 0;

	CDlgCancel*	pcDlgCancel = NULL;
	HWND		hwndCancel = NULL;
	HWND		hwndProgress = NULL;
	BOOL		bResult = TRUE;

	if(filename[0] == L'\0') {
		return FALSE;
	}

	//	2007.09.08 genta CEditDoc::FileLoad()�ɂȂ���č����v�J�[�\��
	CWaitCursor cWaitCursor( m_pCommanderView->GetHwnd() );

	// �͈͑I�𒆂Ȃ�}������I����Ԃɂ��邽��	/* 2007.04.29 maru */
	BOOL	bBeforeTextSelected = m_pCommanderView->GetSelectionInfo().IsTextSelected();
	CLayoutPoint ptFrom;
	/*
	int	nLineFrom, nColmFrom;
	*/
	if (bBeforeTextSelected){
		ptFrom = m_pCommanderView->GetSelectionInfo().m_sSelect.GetFrom();
		/*
		nLineFrom = m_nSelectLineFrom;
		nColmFrom = m_nSelectColmFrom;
		*/
	}


	ECodeType	nSaveCharCode = nCharCode;
	if(nSaveCharCode == CODE_AUTODETECT) {
		EditInfo		fi;
		CMRU			cMRU;
		if ( cMRU.GetEditInfo( to_tchar(filename), &fi ) ){
				nSaveCharCode = fi.m_nCharCode;
		} else {
			nSaveCharCode = GetDocument()->GetDocumentEncoding();
		}
	}
	
	/* �����܂ł��ĕ����R�[�h�����肵�Ȃ��Ȃ�ǂ����������� */
	if( 0 > nSaveCharCode || nSaveCharCode > CODE_CODEMAX ) nSaveCharCode = CODE_SJIS;
	
	try{
		// �t�@�C�����J��
		cfl.FileOpen( to_tchar(filename), nSaveCharCode, NULL );

		/* �t�@�C���T�C�Y��65KB���z������i���_�C�A���O�\�� */
		if ( 0x10000 < cfl.GetFileSize() ) {
			pcDlgCancel = new CDlgCancel;
			if( NULL != ( hwndCancel = pcDlgCancel->DoModeless( ::GetModuleHandle( NULL ), NULL, IDD_OPERATIONRUNNING ) ) ){
				hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS );
				::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100) );
				::SendMessage( hwndProgress, PBM_SETPOS, 0, 0 );
			}
		}

		// ReadLine�̓t�@�C������ �����R�[�h�ϊ����ꂽ1�s��ǂݏo���܂�
		// �G���[����throw CError_FileRead �𓊂��܂�
		CNativeW cBuf;
		while( RESULT_FAILURE != cfl.ReadLine( &cBuf, &cEol ) ){

			const wchar_t*	pLine = cBuf.GetStringPtr();
			int			nLineLen = cBuf.GetStringLength();

			++nLineNum;
			Command_INSTEXT(false, pLine, CLogicInt(nLineLen), true);

			/* �i���_�C�A���O�L�� */
			if( NULL == pcDlgCancel ){
				continue;
			}
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				break;
			}
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				break;
			}
			if( 0 == ( nLineNum & 0xFF ) ){
				::PostMessage( hwndProgress, PBM_SETPOS, cfl.GetPercent(), 0 );
				m_pCommanderView->Redraw();
			}
		}
		// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
		cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		WarningMessage( NULL, _T("file open error [%ls]"), filename );
		bResult = FALSE;
	}
	catch( CError_FileRead ){
		WarningMessage( NULL, _T("�t�@�C���̓ǂݍ��ݒ��ɃG���[���������܂����B") );
		bResult = FALSE;
	} // ��O�����I���

	if( NULL != pcDlgCancel ){
		delete pcDlgCancel;
	}
	if (bBeforeTextSelected){	// �}�����ꂽ������I����Ԃ�
		m_pCommanderView->GetSelectionInfo().SetSelectArea(
			CLayoutRange(
				ptFrom,
				/*
				nLineFrom, nColmFrom,
				*/
				GetCaret().GetCaretLayoutPos()
				/*
				m_nCaretPosY, m_nCaretPosX
				*/
			)
		);
		m_pCommanderView->GetSelectionInfo().DrawSelectArea();
	}
	m_pCommanderView->Redraw();
	return bResult;
}


/*!
	@brief �e�L�X�g�̐܂�Ԃ����@��ύX����
	
	@param[in] nWrapMethod �܂�Ԃ����@
		WRAP_NO_TEXT_WRAP  : �܂�Ԃ��Ȃ�
		WRAP_SETTING_WIDTH ; �w�茅�Ő܂�Ԃ�
		WRAP_WINDOW_WIDTH  ; �E�[�Ő܂�Ԃ�
	
	@note �E�B���h�E�����E�ɕ�������Ă���ꍇ�A�����̃E�B���h�E����܂�Ԃ����Ƃ���B
	
	@date 2008.05.31 nasukoji	�V�K�쐬
	@date 2009.08.28 nasukoji	�e�L�X�g�̍ő啝���Z�o����
*/
void CViewCommander::Command_TEXTWRAPMETHOD( int nWrapMethod )
{
	CEditDoc* pcDoc = GetDocument();

	// ���݂̐ݒ�l�Ɠ����Ȃ牽�����Ȃ�
	if( pcDoc->m_nTextWrapMethodCur == nWrapMethod )
		return;

	int nWidth;

	switch( nWrapMethod ){
	case WRAP_NO_TEXT_WRAP:		// �܂�Ԃ��Ȃ�
		nWidth = MAXLINEKETAS;	// �A�v���P�[�V�����̍ő啝�Ő܂�Ԃ�
		break;

	case WRAP_SETTING_WIDTH:	// �w�茅�Ő܂�Ԃ�
		nWidth = (Int)pcDoc->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas;
		break;

	case WRAP_WINDOW_WIDTH:		// �E�[�Ő܂�Ԃ�
		// �E�B���h�E�����E�ɕ�������Ă���ꍇ�͍����̃E�B���h�E�����g�p����
		nWidth = (Int)m_pCommanderView->ViewColNumToWrapColNum( GetEditWindow()->m_pcEditViewArr[0]->GetTextArea().m_nViewColNum );
		break;

	default:
		return;		// �s���Ȓl�̎��͉������Ȃ�
	}

	pcDoc->m_nTextWrapMethodCur = nWrapMethod;	// �ݒ���L��

	// �܂�Ԃ����@�̈ꎞ�ݒ�K�p�^�ꎞ�ݒ�K�p����	// 2008.06.08 ryoji
	pcDoc->m_bTextWrapMethodCurTemp = !( pcDoc->m_cDocType.GetDocumentAttribute().m_nTextWrapMethod == nWrapMethod );

	// �܂�Ԃ��ʒu��ύX
	GetEditWindow()->ChangeLayoutParam( false, pcDoc->m_cLayoutMgr.GetTabSpace(), (CLayoutInt)nWidth );

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�Ȃ�e�L�X�g�ő啝���Z�o�A����ȊO�͕ϐ����N���A
	if( pcDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
		pcDoc->m_cLayoutMgr.CalculateTextWidth();		// �e�L�X�g�ő啝���Z�o����
		pcDoc->m_pcEditWnd->RedrawAllViews( NULL );		// �X�N���[���o�[�̍X�V���K�v�Ȃ̂ōĕ\�������s����
	}else{
		pcDoc->m_cLayoutMgr.ClearLayoutLineWidth();		// �e�s�̃��C�A�E�g�s���̋L�����N���A����
	}
}

/* 	�㏑���p�̈ꕶ���폜	2009.04.11 ryoji */
void CViewCommander::DelCharForOverwrite( void )
{
	bool bEol = false;
	BOOL bDelete = TRUE;
	const CLayout* pcLayout = GetDocument()->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().GetY2() );
	if( NULL != pcLayout ){
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		CLogicInt nIdxTo = m_pCommanderView->LineColmnToIndex( pcLayout, GetCaret().GetCaretLayoutPos().GetX2() );
		if( nIdxTo >= pcLayout->GetLengthWithoutEOL() ){
			bEol = true;	// ���݈ʒu�͉��s�܂��͐܂�Ԃ��Ȍ�
			if( pcLayout->GetLayoutEol() != EOL_NONE ){
				if( GetDllShareData().m_Common.m_sEdit.m_bNotOverWriteCRLF ){	/* ���s�͏㏑�����Ȃ� */
					/* ���݈ʒu�����s�Ȃ�΍폜���Ȃ� */
					bDelete = FALSE;
				}
			}
		}
	}
	if( bDelete ){
		/* �㏑�����[�h�Ȃ̂ŁA���݈ʒu�̕������P�������� */
		if( bEol ){
			Command_DELETE();	//�s�����ł͍ĕ`�悪�K�v���s���Ȍ�̍폜����������
		}else{
			m_pCommanderView->DeleteData( FALSE );
		}
	}
}

/*!
	@brief �����J�E���g���@��ύX����
	
	@param[in] nMode �����J�E���g���@
		SELECT_COUNT_TOGGLE  : �����J�E���g���@���g�O��
		SELECT_COUNT_BY_CHAR ; �������ŃJ�E���g
		SELECT_COUNT_BY_BYTE ; �o�C�g���ŃJ�E���g
*/
void CViewCommander::Command_SELECT_COUNT_MODE( int nMode )
{
	//�ݒ�ɂ͕ۑ������AView���Ɏ��t���O��ݒ�
	//BOOL* pbDispSelCountByByte = &GetDllShareData().m_Common.m_sStatusbar.m_bDispSelCountByByte;
	ESelectCountMode* pnSelectCountMode = &GetEditWindow()->m_nSelectCountMode;

	if( nMode == SELECT_COUNT_TOGGLE ){
		//�������̃o�C�g���g�O��
		ESelectCountMode nCurrentMode;
		if( *pnSelectCountMode == SELECT_COUNT_TOGGLE ){
			nCurrentMode = ( GetDllShareData().m_Common.m_sStatusbar.m_bDispSelCountByByte ?
								SELECT_COUNT_BY_BYTE :
								SELECT_COUNT_BY_CHAR );
		}else{
			nCurrentMode = *pnSelectCountMode;
		}
		*pnSelectCountMode = ( nCurrentMode == SELECT_COUNT_BY_BYTE ?
								SELECT_COUNT_BY_CHAR :
								SELECT_COUNT_BY_BYTE );
	}else if( nMode == SELECT_COUNT_BY_BYTE || nMode == SELECT_COUNT_BY_CHAR ){
		*pnSelectCountMode = ( ESelectCountMode )nMode;
	}
}
