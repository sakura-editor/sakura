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
	Copyright (C) 2010, ryoji
	Copyright (C) 2011, ryoji
	Copyright (C) 2012, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
// 2007.10.25 kobake CViewCommander�N���X�ɕ���

#include "StdAfx.h"
#include "CViewCommander.h"

#include "view/CEditView.h"
//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
#include "macro/CSMacroMgr.h"
#include "COpeBlk.h"/// 2002/2/3 aroka �ǉ�
#include "window/CEditWnd.h"/// 2002/2/3 aroka �ǉ�
#include "CEditApp.h"
#include "plugin/CJackManager.h"


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


	if( m_pCommanderView->m_nAutoScrollMode && F_AUTOSCROLL != nCommand ){
		m_pCommanderView->AutoScrollExit();
	}
	m_pCommanderView->GetCaret().m_bClearStatus = true;
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
			CEditApp::getInstance()->m_pcSMacroMgr->Append( STAND_KEYMACRO, nCommand, lparam1, m_pCommanderView );
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
		if( !CEditApp::getInstance()->m_pcSMacroMgr->Exec( nCommand - F_USERMACRO_0, G_AppInstance(), m_pCommanderView,
			nCommandFrom & FA_NONRECORD )){
			InfoMessage(
				this->m_pCommanderView->m_hwndParent,
				_T("�}�N�� %d (%ts) �̎��s�Ɏ��s���܂����B"),
				nCommand - F_USERMACRO_0,
				CEditApp::getInstance()->m_pcSMacroMgr->GetFile( nCommand - F_USERMACRO_0 )
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
	GetOpeBlk()->AddRef();	//�Q�ƃJ�E���^����

	//	Jan. 10, 2005 genta �R�����g
	//	��������ł�switch�̌���Undo�𐳂����o�^���邽�߁C
	//	�r���ŏ����̑ł��؂���s���Ă͂����Ȃ�
	// -------------------------------------

	switch( nCommand ){
	case F_WCHAR:	/* �������� */
		{
			Command_WCHAR( (wchar_t)lparam1 );
		}
		break;

	/* �t�@�C������n */
	case F_FILENEW:				Command_FILENEW();break;			/* �V�K�쐬 */
	case F_FILENEW_NEWWINDOW:	Command_FILENEW_NEWWINDOW();break;
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
	case F_FILE_REOPEN_LATIN1:		Command_FILE_REOPEN( CODE_LATIN1, lparam1!=0 );break;	//Latin1�ŊJ���Ȃ���	// 2010/3/20 Uchi
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
	case F_MOVECURSOR:			Command_MOVECURSOR(CLogicPoint(CLogicInt((int)lparam2), CLogicInt((int)lparam1)), (int)lparam3); break;
	case F_MOVECURSORLAYOUT:	Command_MOVECURSORLAYOUT(CLayoutPoint(CLayoutInt((int)lparam2), CLayoutInt((int)lparam1)), (int)lparam3); break;
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
	case F_AUTOSCROLL:	Command_AUTOSCROLL(); break;	//�I�[�g�X�N���[��

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
	case F_PASTE:					Command_PASTE( (int)lparam1 );break;				//�\��t��(�N���b�v�{�[�h����\��t��)
	case F_PASTEBOX:				Command_PASTEBOX( (int)lparam1 );break;				//��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	case F_INSBOXTEXT:				Command_INSBOXTEXT((const wchar_t*)lparam1, (int)lparam2 );break;				//��`�e�L�X�g�}��
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
		ClearOpeBlk();
		Command_REPLACE_DIALOG();	//@@@ 2002.2.2 YAZAKI �_�C�A���O�Ăяo���ƁA���s�𕪗�
		break;
	case F_REPLACE:				Command_REPLACE( (HWND)lparam1 );break;			//�u�����s @@@ 2002.2.2 YAZAKI
	case F_REPLACE_ALL:			Command_REPLACE_ALL();break;		//���ׂĒu�����s(�ʏ�) 2002.2.8 hor 2006.04.02 �����
	case F_SEARCH_CLEARMARK:	Command_SEARCH_CLEARMARK();break;	//�����}�[�N�̃N���A
	case F_GREP_DIALOG:	//Grep�_�C�A���O�̕\��
		/* �ċA�����΍� */
		ClearOpeBlk();
		Command_GREP_DIALOG();
		break;
	case F_GREP:			Command_GREP();break;							//Grep
	case F_JUMP_DIALOG:		Command_JUMP_DIALOG();break;					//�w��s�w�W�����v�_�C�A���O�̕\��
	case F_JUMP:			Command_JUMP();break;							//�w��s�w�W�����v
	case F_OUTLINE:			bRet = Command_FUNCLIST( (int)lparam1, OUTLINE_DEFAULT );break;	//�A�E�g���C�����
	case F_OUTLINE_TOGGLE:	bRet = Command_FUNCLIST( SHOW_TOGGLE, OUTLINE_DEFAULT );break;	//�A�E�g���C�����(toggle) // 20060201 aroka
	case F_TAGJUMP:			Command_TAGJUMP(lparam1 != 0);break;			/* �^�O�W�����v�@�\ */ //	Apr. 03, 2003 genta �����ǉ�
	case F_TAGJUMP_CLOSE:	Command_TAGJUMP(true);break;					/* �^�O�W�����v(���E�B���h�EClose) *///	Apr. 03, 2003 genta
	case F_TAGJUMPBACK:		Command_TAGJUMPBACK();break;					/* �^�O�W�����v�o�b�N�@�\ */
	case F_TAGS_MAKE:		Command_TagsMake();break;						//�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	case F_DIRECT_TAGJUMP:	Command_TagJumpByTagsFileMsg( true );break;				/* �_�C���N�g�^�O�W�����v�@�\ */	//@@@ 2003.04.15 MIK
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
	case F_CHG_CHARSET:		Command_CHG_CHARSET( (ECodeType)lparam1, lparam2 != 0 );break;	//�����R�[�h�Z�b�g�w��	2010/6/14 Uchi
	// From Here 2003.06.23 Moca
	// F_CHGMOD_EOL_xxx �̓}�N���ɋL�^����Ȃ����AF_CHGMOD_EOL�̓}�N���ɋL�^�����̂ŁA�}�N���֐��𓝍��ł���Ƃ�����͂�
	case F_CHGMOD_EOL_CRLF:	HandleCommand( F_CHGMOD_EOL, bRedraw, EOL_CRLF, 0, 0, 0 );break;	//���͂�����s�R�[�h��CRLF�ɐݒ�
	case F_CHGMOD_EOL_LF:	HandleCommand( F_CHGMOD_EOL, bRedraw, EOL_LF, 0, 0, 0 );break;	//���͂�����s�R�[�h��LF�ɐݒ�
	case F_CHGMOD_EOL_CR:	HandleCommand( F_CHGMOD_EOL, bRedraw, EOL_CR, 0, 0, 0 );break;	//���͂�����s�R�[�h��CR�ɐݒ�
	// 2006.09.03 Moca F_CHGMOD_EOL�� break �Y��̏C��
	case F_CHGMOD_EOL:		Command_CHGMOD_EOL( (EEolType)lparam1 );break;	//���͂�����s�R�[�h��ݒ�
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
		ClearOpeBlk();
		Command_EXECKEYMACRO();break;
	case F_EXECEXTMACRO:
		/* �ċA�����΍� */
		ClearOpeBlk();
		/* ���O���w�肵�ă}�N�����s */
		Command_EXECEXTMACRO( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		break;
	//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
	//	case F_EXECCMMAND:		Command_EXECCMMAND();break;	/* �O���R�}���h���s */
	case F_EXECMD_DIALOG:
		/* �ċA�����΍� */// 2001/06/23 N.Nakatani
		ClearOpeBlk();
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
		ClearOpeBlk();
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
		ClearOpeBlk();
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
		ClearOpeBlk();
		Command_MENU_ALLFUNC();break;
	case F_EXTHELP1:	Command_EXTHELP1();break;		/* �O���w���v�P */
	case F_EXTHTMLHELP:	/* �O��HTML�w���v */
		//	Jul. 5, 2002 genta
		Command_EXTHTMLHELP( (const WCHAR*)lparam1, (const WCHAR*)lparam2 );
		break;
	case F_ABOUT:	Command_ABOUT();break;				/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	/* ���̑� */
//	case F_SENDMAIL:	Command_SENDMAIL();break;		/* ���[�����M */

	case F_0: break; // F_0�Ńv���O�C�������s�����o�O�΍�	// �� rev1886 �̖��͌Ăь��ő΍􂵂������S�قƂ��Ďc��

	default:
		//�v���O�C���R�}���h�����s����
		{
			CPlug::Array plugs;
			CJackManager::getInstance()->GetUsablePlug( PP_COMMAND, nCommand, &plugs );

			if( plugs.size() > 0 ){
				assert_warning( 1 == plugs.size() );
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
	m_pCommanderView->SetUndoBuffer( true );

	return bRet;
}



CLogicInt CViewCommander::ConvertEol(const wchar_t* pszText, CLogicInt nTextLen, wchar_t* pszConvertedText)
{
	// original by 2009.02.28 salarm
	CLogicInt nConvertedTextLen;
	CEol eol = GetDocument()->m_cDocEditor.GetNewLineCode();

	nConvertedTextLen = 0;
	for( int i = 0; i < nTextLen; i++ ){
		if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
			if( pszText[i] == WCODE::CR ){
				if( i + 1 < nTextLen && pszText[i + 1] == WCODE::LF ){
					i++;
				}
			}
			wmemcpy( &pszConvertedText[nConvertedTextLen], eol.GetValue2(), eol.GetLen() );
			nConvertedTextLen += eol.GetLen();
		} else {
			pszConvertedText[nConvertedTextLen++] = pszText[i];
		}
	}
	return nConvertedTextLen;
}



/*!
	@brief �����Ō�����Ȃ��Ƃ��̌x���i���b�Z�[�W�{�b�N�X�^�T�E���h�j

	@date 2010.04.21 ryoji	�V�K�쐬�i���J���ŗp�����Ă����ގ��R�[�h�̋��ʉ��j
*/
void CViewCommander::AlertNotFound(HWND hwnd, LPCTSTR format, ...)
{
	if( GetDllShareData().m_Common.m_sSearch.m_bNOTIFYNOTFOUND
		&& m_pCommanderView->GetDrawSwitch()	// �� ���Ԃ�u�S�Ēu���v���s������̑�p�i�i���Ƃ� Command_SEARCH_NEXT() �̒��ł����g�p����Ă����j
	){
		if( NULL == hwnd ){
			hwnd = m_pCommanderView->GetHwnd();
		}
		//InfoMessage(hwnd, format, __VA_ARGS__);
		va_list p;
		va_start(p, format);
		VMessageBoxF(hwnd, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME, format, p);
		va_end(p);
	}else{
		DefaultBeep();
	}
}
