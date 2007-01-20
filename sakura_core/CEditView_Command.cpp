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
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <io.h>
#include <mbstring.h>
#include "sakura_rc.h"
#include "CEditView.h"
#include "debug.h"
#include "funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "CEditApp.h"
#include "CWaitCursor.h"
#include "CSplitterWnd.h"
//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
#include "CSMacroMgr.h"
#include "etc_uty.h"
#include "CDlgTypeList.h"
#include "CDlgProperty.h"
#include "CDlgCompare.h"
#include "global.h"
#include <htmlhelp.h>
#include "CRunningtimer.h"
#include "CDlgExec.h"
#include "CDlgAbout.h"	//Dec. 24, 2000 JEPRO �ǉ�
#include "COpe.h"/// 2002/2/3 aroka �ǉ� from here
#include "COpeBlk.h"///
#include "CLayout.h"///
#include "CEditWnd.h"///
#include "CFuncInfoArr.h"///
#include "CMarkMgr.h"///
#include "CDocLine.h"///
#include "CSMacroMgr.h"///
#include "mymessage.h"/// 2002/2/3 aroka �ǉ� to here
#include "CDlgCancel.h"// 2002/2/8 hor
#include "CPrintPreview.h"
#include "CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "CDlgCancel.h"
#include "CDlgTagJumpList.h"
#include "CDlgTagsMake.h"	//@@@ 2003.05.12 MIK
#include "COsVersionInfo.h"
#include "my_icmp.h"

/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
BOOL CEditView::HandleCommand(
	int		nCommand,
	BOOL	bRedraw,
	LPARAM	lparam1,
	LPARAM	lparam2,
	LPARAM	lparam3,
	LPARAM	lparam4
)
{
	BOOL	bRet = TRUE;
	BOOL	bRepeat = FALSE;
	int		nFuncID;

	//	May. 19, 2006 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX�����̂�
	//	����16�r�b�g�݂̂����o��
	int	nCommandFrom = HIWORD( nCommand );
	nCommand = LOWORD( nCommand );

	//	Oct. 30, 2004 genta
	//	�����̏㉺�o�C�g���P��WM_IME_CHAR�ł͂Ȃ��ʁX��WM_CHAR�ɂė���
	//	�P�[�X�̎�舵���������擪�Ɉړ��D
	//	* �}�N���ŕ������ċL�^����Ȃ��悤��
	//	* �g���b�v�����ꍇ��2�o�C�g�ڂɔ������Ă��܂��̂�h��
	//	From Here Oct. 5, 2002 genta
	//	WM_CHAR�ł��銿���R�[�h���󂯓����
	if( nCommand == F_CHAR ){
		// �����ɂ�CEditView�̃����o�[�ɂ��ׂ������������b�Z�[�W�̋~�ς͂���ł��\��
		static unsigned int ucSjis1 = 0;
		if( ucSjis1 == 0 ){
			if( _IS_SJIS_1( (unsigned char)lparam1 )){
				ucSjis1 = lparam1;
				return TRUE;
			}
		}
		else {
			//	�ꕶ���O��SJIS��1�o�C�g�ڂ����Ă���
			if( _IS_SJIS_2( (unsigned char)lparam1 )){
				lparam1 = (ucSjis1 << 8 | lparam1 );
				nCommand = F_IME_CHAR;
			}
			ucSjis1 = 0;
		}
	}
	//	To Here Oct. 5, 2002 genta

	// -------------------------------------
	//	Jan. 10, 2005 genta
	//	Call message translators
	// -------------------------------------
	TranslateCommand_grep( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );
	TranslateCommand_isearch( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 );

	//	Aug, 14. 2000 genta
	if( m_pcEditDoc->IsModificationForbidden( nCommand ) ){
		return TRUE;
	}

//	if( 0 == m_pcEditDoc->m_nCommandExecNum ){
//		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, "���ڂ̃R�}���h" );
//	}
	++m_pcEditDoc->m_nCommandExecNum;		/* �R�}���h���s�� */
//	if( nCommand != F_COPY ){
		/* ����Tip������ */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
//	}
	/* ����v���r���[���[�h�� */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
//	if( TRUE == m_pcEditDoc->m_bPrintPreviewMode &&
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	if( pCEditWnd->m_pPrintPreview &&
		F_PRINT_PREVIEW != nCommand
	){
		::MessageBeep( MB_ICONHAND );
		return -1;
	}
	/* �L�[���s�[�g��� */
	if( m_bPrevCommand == nCommand ){
		bRepeat = TRUE;
	}
	m_bPrevCommand = nCommand;
	if( m_pShareData->m_bRecordingKeyMacro &&									/* �L�[�{�[�h�}�N���̋L�^�� */
		m_pShareData->m_hwndRecordingKeyMacro == ::GetParent( m_hwndParent )	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	){
		/* �L�[���s�[�g��Ԃ��Ȃ����� */
		bRepeat = FALSE;
		/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		if( CSMacroMgr::CanFuncIsKeyMacro( nCommand ) ){
			/* �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
			//@@@ 2002.1.24 m_CKeyMacroMgr��CEditDoc�ֈړ�
			m_pcEditDoc->m_pcSMacroMgr->Append( STAND_KEYMACRO, nCommand, lparam1, this );
		}
	}
	/* �L�[�{�[�h�}�N���̎��s�� */
	if( m_bExecutingKeyMacro ){
		/* �L�[���s�[�g��Ԃ��Ȃ����� */
		bRepeat = FALSE;
	}

	//	From Here Sep. 29, 2001 genta �}�N���̎��s�@�\�ǉ�
	if( F_USERMACRO_0 <= nCommand && nCommand < F_USERMACRO_0 + MAX_CUSTMACRO ){
		m_bExecutingKeyMacro = TRUE;
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���i�C���^�[�t�F�[�X�̕ύX�j
		if( !m_pcEditDoc->m_pcSMacroMgr->Exec( nCommand - F_USERMACRO_0, m_hInstance, this )){
			::MYMESSAGEBOX( m_hwndParent,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"�}�N�� %d (%s) �̎��s�Ɏ��s���܂����B", nCommand - F_USERMACRO_0,
				m_pcEditDoc->m_pcSMacroMgr->GetFile( nCommand - F_USERMACRO_0 )
			);
		}
		m_bExecutingKeyMacro = FALSE;
		return TRUE;
	}
	//	To Here Sep. 29, 2001 genta �}�N���̎��s�@�\�ǉ�

	// -------------------------------------
	//	Jan. 10, 2005 genta
	//	Call mode basis message handler
	// -------------------------------------
	PreprocessCommand_hokan(nCommand);
	if( ProcessCommand_isearch( nCommand, bRedraw, lparam1, lparam2, lparam3, lparam4 ))
		return TRUE;

	// -------------------------------------
	//	Jan. 10, 2005 genta �R�����g
	//	�������O�ł�Undo�o�b�t�@�̏������ł��Ă��Ȃ��̂�
	//	�����̑�����s���Ă͂����Ȃ�
	//@@@ 2002.2.2 YAZAKI HandleCommand����HandleCommand���Ăяo���Ȃ����ɑΏ��i��������p������H�j
	if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
		
	}
	else {
		m_pcOpeBlk = new COpeBlk;
	}
	
	//	Jan. 10, 2005 genta �R�����g
	//	��������ł�switch�̌���Undo�𐳂����o�^���邽�߁C
	//	�r���ŏ����̑ł��؂���s���Ă͂����Ȃ�
	// -------------------------------------


//	if( !m_pcEditDoc->m_bDebugMode ){
//		char*		szCommandName[256];
//		/* �f�o�b�O���j�^�ɏo�� */
//		if( 0 < ::LoadString( m_hInstance, nCommand, (LPTSTR)szCommandName, sizeof( szCommandName ) ) ){
//			m_cShareData.TraceOut( "nCommand=%d %s\n", nCommand, szCommandName );
//		}else{
//			m_cShareData.TraceOut( "nCommand=%d [nknown]\n", nCommand );
//		}
//	}

	switch( nCommand ){
	case F_CHAR:	/* �������� */
		/* �R���g���[���R�[�h���͋֎~ */
		if(
			( ( (unsigned char)0x0 <= (unsigned char)lparam1 && (unsigned char)lparam1 <= (unsigned char)0x1F ) ||
			  ( (unsigned char)'~' <  (unsigned char)lparam1 && (unsigned char)lparam1 <  (unsigned char)'�'  ) ||
			  ( (unsigned char)'�' <  (unsigned char)lparam1 && (unsigned char)lparam1 <= (unsigned char)0xff )
			) &&
			(unsigned char)lparam1 != TAB && (unsigned char)lparam1 != CR && (unsigned char)lparam1 != LF
		){
			::MessageBeep( MB_ICONHAND );
		}else{
			Command_CHAR( (char)lparam1 );
		}
		break;

	/* �t�@�C������n */
	case F_FILENEW:		Command_FILENEW();break;			/* �V�K�쐬 */
	//	Oct. 2, 2001 genta �}�N���p�@�\�g��
	case F_FILEOPEN:	Command_FILEOPEN((const char*)lparam1);break;			/* �t�@�C�����J�� */
	case F_FILEOPEN_DROPDOWN:	Command_FILEOPEN((const char*)lparam1);break;			/* �t�@�C�����J��(�h���b�v�_�E��) */	//@@@ 2002.06.15 MIK
	case F_FILESAVE:	bRet = Command_FILESAVE();break;	/* �㏑���ۑ� */
	case F_FILESAVEAS_DIALOG:	bRet = Command_FILESAVEAS_DIALOG();break;	/* ���O��t���ĕۑ� */
	case F_FILESAVEAS:	bRet = Command_FILESAVEAS((const char*)lparam1);break;	/* ���O��t���ĕۑ� */
	case F_FILESAVEALL:	bRet = Command_FILESAVEALL();break;	/* �S�Ă̕ҏW�E�B���h�E�ŏ㏑���ۑ� */ // Jan. 23, 2005 genta
	case F_FILESAVE_QUIET:	bRet = Command_FILESAVE(false,false); break;	/* �Â��ɏ㏑���ۑ� */ // Jan. 24, 2005 genta
	case F_FILESAVECLOSE:
		//	Feb. 28, 2004 genta �ۑ�������
		//	�ۑ����s�v�Ȃ�P�ɕ���
		if( Command_FILESAVE( false, true )){
			Command_WINCLOSE();
		}
		break;
	case F_FILECLOSE:										//����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
		/* �ċA�����΍� */
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
		Command_FILECLOSE();
		break;
	case F_FILECLOSE_OPEN:	/* ���ĊJ�� */
		/* �ċA�����΍� */
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
		Command_FILECLOSE_OPEN();
		break;
	case F_FILE_REOPEN:			Command_FILE_REOPEN( m_pcEditDoc->m_nCharCode, lparam1 );break;//	Dec. 4, 2002 genta
	case F_FILE_REOPEN_SJIS:	Command_FILE_REOPEN( CODE_SJIS, lparam1 );break;		//SJIS�ŊJ������
	case F_FILE_REOPEN_JIS:		Command_FILE_REOPEN( CODE_JIS, lparam1 );break;		//JIS�ŊJ������
	case F_FILE_REOPEN_EUC:		Command_FILE_REOPEN( CODE_EUC, lparam1 );break;		//EUC�ŊJ������
	case F_FILE_REOPEN_UNICODE:	Command_FILE_REOPEN( CODE_UNICODE, lparam1 );break;	//Unicode�ŊJ������
	case F_FILE_REOPEN_UNICODEBE: 	Command_FILE_REOPEN( CODE_UNICODEBE, lparam1 );break;	//UnicodeBE�ŊJ������
	case F_FILE_REOPEN_UTF8:	Command_FILE_REOPEN( CODE_UTF8, lparam1 );break;		//UTF-8�ŊJ������
	case F_FILE_REOPEN_UTF7:	Command_FILE_REOPEN( CODE_UTF7, lparam1 );break;		//UTF-7�ŊJ������
	case F_PRINT:				Command_PRINT();break;					/* ��� */
	case F_PRINT_PREVIEW:		Command_PRINT_PREVIEW();break;			/* ����v���r���[ */
	case F_PRINT_PAGESETUP:		Command_PRINT_PAGESETUP();break;		/* ����y�[�W�ݒ� */	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	case F_OPEN_HfromtoC:		bRet = Command_OPEN_HfromtoC( (BOOL)lparam1 );break;	/* ������C/C++�w�b�_(�\�[�X)���J�� */	//Feb. 7, 2001 JEPRO �ǉ�
	case F_OPEN_HHPP:			bRet = Command_OPEN_HHPP( (BOOL)lparam1, TRUE );break;		/* ������C/C++�w�b�_�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	case F_OPEN_CCPP:			bRet = Command_OPEN_CCPP( (BOOL)lparam1, TRUE );break;		/* ������C/C++�\�[�X�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	case F_ACTIVATE_SQLPLUS:	Command_ACTIVATE_SQLPLUS();break;		/* Oracle SQL*Plus���A�N�e�B�u�\�� */
	case F_PLSQL_COMPILE_ON_SQLPLUS:									/* Oracle SQL*Plus�Ŏ��s */
		/* �ċA�����΍� */
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
		Command_PLSQL_COMPILE_ON_SQLPLUS();
		break;
	case F_BROWSE:				Command_BROWSE();break;				/* �u���E�Y */
	case F_READONLY:			Command_READONLY();break;			/* �ǂݎ���p */
	case F_PROPERTY_FILE:		Command_PROPERTY_FILE();break;		/* �t�@�C���̃v���p�e�B */
	case F_EXITALL:				Command_EXITALL();break;			/* �T�N���G�f�B�^�̑S�I�� */	//Dec. 26, 2000 JEPRO �ǉ�

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
// From Here 2001.12.03 hor
	case F_INDENT_TAB:			Command_INDENT_TAB();break;			//TAB�C���f���g
//	case F_INDENT_TAB:												//TAB�C���f���g
//		/* �e�L�X�g���Q�s�ȏ�ɂ܂������đI������Ă��邩 */
//		if( IsTextSelected() &&
//			0 != ( m_nSelectLineFrom - m_nSelectLineTo )
//		){
//			Command_INDENT( TAB );
//		}else{
//			/* �P�o�C�g�������� */
//			Command_CHAR( (char)TAB );
//		}
//		break;
// To Here 2001.12.03 hor
	case F_UNINDENT_TAB:		Command_UNINDENT( TAB );break;		//�tTAB�C���f���g
	case F_INDENT_SPACE:											//SPACE�C���f���g
		/* �e�L�X�g���Q�s�ȏ�ɂ܂������đI������Ă��邩 */
		if( IsTextSelected() &&
			0 != ( m_nSelectLineFrom - m_nSelectLineTo )
		){
			Command_INDENT( SPACE );
		}else{
			/* �P�o�C�g�������� */
			Command_CHAR( (char)' ' );
		}
		break;
	case F_UNINDENT_SPACE:			Command_UNINDENT( SPACE );break;	//�tSPACE�C���f���g
//	case F_WORDSREFERENCE:			Command_WORDSREFERENCE();break;		/* �P�ꃊ�t�@�����X */
	case F_LTRIM:					Command_TRIM(TRUE);break;			// 2001.12.03 hor
	case F_RTRIM:					Command_TRIM(FALSE);break;			// 2001.12.03 hor
	case F_SORT_ASC:				Command_SORT(TRUE);break;			// 2001.12.06 hor
	case F_SORT_DESC:				Command_SORT(FALSE);break;			// 2001.12.06 hor
	case F_MERGE:					Command_MERGE();break;				// 2001.12.06 hor
	case F_RECONVERT:				Command_Reconvert();break;			/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */ 

	/* �J�[�\���ړ��n */
	case F_IME_CHAR:		Command_IME_CHAR( (WORD)lparam1 ); break;					//�S�p��������
	case F_UP:				Command_UP( m_bSelectingLock, bRepeat ); break;				//�J�[�\����ړ�
	case F_DOWN:			Command_DOWN( m_bSelectingLock, bRepeat ); break;			//�J�[�\�����ړ�
	case F_LEFT:			Command_LEFT( m_bSelectingLock, bRepeat ); break;			//�J�[�\�����ړ�
	case F_RIGHT:			Command_RIGHT( m_bSelectingLock, FALSE, bRepeat ); break;	//�J�[�\���E�ړ�
	case F_UP2:				Command_UP2( m_bSelectingLock ); break;						//�J�[�\����ړ�(�Q�s�Â�)
	case F_DOWN2:			Command_DOWN2( m_bSelectingLock ); break;					//�J�[�\�����ړ�(�Q�s�Â�)
	case F_WORDLEFT:		Command_WORDLEFT( m_bSelectingLock ); break;				/* �P��̍��[�Ɉړ� */
	case F_WORDRIGHT:		Command_WORDRIGHT( m_bSelectingLock ); break;				/* �P��̉E�[�Ɉړ� */
	//	0ct. 29, 2001 genta �}�N�������@�\�g��
	case F_GOLINETOP:		Command_GOLINETOP( m_bSelectingLock, lparam1  ); break;		//�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOLINEEND:		Command_GOLINEEND( m_bSelectingLock, FALSE ); break;		//�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_ROLLDOWN:		Command_ROLLDOWN( m_bSelectingLock ); break;				//�X�N���[���_�E��
//	case F_ROLLUP:			Command_ROLLUP( m_bSelectingLock ); break;					//�X�N���[���A�b�v
	case F_HalfPageUp:		Command_HalfPageUp( m_bSelectingLock ); break;				//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	case F_HalfPageDown:	Command_HalfPageDown( m_bSelectingLock ); break;			//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	case F_1PageUp:			Command_1PageUp( m_bSelectingLock ); break;					//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	case F_1PageDown:		Command_1PageDown( m_bSelectingLock ); break;				//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	case F_GOFILETOP:		Command_GOFILETOP( m_bSelectingLock ); break;				//�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND:		Command_GOFILEEND( m_bSelectingLock ); break;				//�t�@�C���̍Ō�Ɉړ�
	case F_CURLINECENTER:	Command_CURLINECENTER(); break;								/* �J�[�\���s���E�B���h�E������ */
	case F_JUMPHIST_PREV:	Command_JUMPHIST_PREV(); break;								//�ړ�����: �O��
	case F_JUMPHIST_NEXT:	Command_JUMPHIST_NEXT(); break;								//�ړ�����: ����
	case F_JUMPHIST_SET:	Command_JUMPHIST_SET(); break;								//���݈ʒu���ړ������ɓo�^
	case F_WndScrollDown:	Command_WndScrollDown(); break;								//�e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	case F_WndScrollUp:		Command_WndScrollUp(); break;								//�e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o
	case F_GONEXTPARAGRAPH:	Command_GONEXTPARAGRAPH( m_bSelectingLock ); break;			//���̒i���֐i��
	case F_GOPREVPARAGRAPH:	Command_GOPREVPARAGRAPH( m_bSelectingLock ); break;			//�O�̒i���֖߂�

	/* �I���n */
	case F_SELECTWORD:		Command_SELECTWORD( );break;					//���݈ʒu�̒P��I��
	case F_SELECTALL:		Command_SELECTALL();break;						//���ׂđI��
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
	case F_COPY:					Command_COPY( FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy );break;			//�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_ADDCRLF:			Command_COPY( FALSE, TRUE );break;		//�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_CRLF:				Command_COPY( FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy, EOL_CRLF );break;	//CRLF���s�ŃR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_PASTE:					Command_PASTE();break;					//�\��t��(�N���b�v�{�[�h����\��t��)
	case F_PASTEBOX:				Command_PASTEBOX();break;				//��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	case F_INSTEXT:					Command_INSTEXT( bRedraw, (const char*)lparam1, -1, (BOOL)lparam2 );break;/* �e�L�X�g��\��t�� */ // 2004.05.14 Moca ���������������ǉ�(-1��\0�I�[�܂�)
	case F_ADDTAIL:					Command_ADDTAIL( (const char*)lparam1, (int)lparam2 );break;	/* �Ō�Ƀe�L�X�g��ǉ� */
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
	case F_TOLOWER:					Command_TOLOWER();break;				/* �p�啶�����p������ */
	case F_TOUPPER:					Command_TOUPPER();break;				/* �p���������p�啶�� */
	case F_TOHANKAKU:				Command_TOHANKAKU();break;				/* �S�p�����p */
	case F_TOHANKATA:				Command_TOHANKATA();break;				/* �S�p�J�^�J�i�����p�J�^�J�i */	//Aug. 29, 2002 ai
	case F_TOZENEI:					Command_TOZENEI();break;				/* �S�p�����p */					//July. 30, 2001 Misaka
	case F_TOHANEI:					Command_TOHANEI();break;				/* ���p���S�p */
	case F_TOZENKAKUKATA:			Command_TOZENKAKUKATA();break;			/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	case F_TOZENKAKUHIRA:			Command_TOZENKAKUHIRA();break;			/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	case F_HANKATATOZENKAKUKATA:	Command_HANKATATOZENKAKUKATA();break;	/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENKAKUHIRA:	Command_HANKATATOZENKAKUHIRA();break;	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_TABTOSPACE:				Command_TABTOSPACE();break;				/* TAB���� */
	case F_SPACETOTAB:				Command_SPACETOTAB();break;				/* �󔒁�TAB */  //#### Stonee, 2001/05/27
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
	case F_SEARCH_NEXT:			Command_SEARCH_NEXT( true, bRedraw, (HWND)lparam1, (const char*)lparam2 );break;	//��������
	case F_SEARCH_PREV:			Command_SEARCH_PREV( bRedraw, (HWND)lparam1 );break;						//�O������
	case F_REPLACE_DIALOG:	//�u��(�u���_�C�A���O)
		/* �ċA�����΍� */
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
		Command_REPLACE_DIALOG();	//@@@ 2002.2.2 YAZAKI �_�C�A���O�Ăяo���ƁA���s�𕪗�
		break;
	case F_REPLACE:				Command_REPLACE( (HWND)lparam1 );break;			//�u�����s @@@ 2002.2.2 YAZAKI
	case F_REPLACE_ALL:			Command_REPLACE_ALL();break;		//���ׂĒu�����s(�ʏ�) 2002.2.8 hor 2006.04.02 �����
	case F_SEARCH_CLEARMARK:	Command_SEARCH_CLEARMARK();break;	//�����}�[�N�̃N���A
	case F_GREP_DIALOG:	//Grep�_�C�A���O�̕\��
		/* �ċA�����΍� */
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
		Command_GREP_DIALOG();
		break;
	case F_GREP:			Command_GREP();break;							//Grep
	case F_JUMP_DIALOG:		Command_JUMP_DIALOG();break;					//�w��s�w�W�����v�_�C�A���O�̕\��
	case F_JUMP:			Command_JUMP();break;							//�w��s�w�W�����v
	case F_OUTLINE:			bRet = Command_FUNCLIST( (int)lparam1 );break;	//�A�E�g���C�����
	case F_OUTLINE_TOGGLE:	bRet = Command_FUNCLIST( SHOW_TOGGLE );break;	//�A�E�g���C�����(toggle) // 20060201 aroka
	case F_TAGJUMP:			Command_TAGJUMP(lparam1 != 0);break;			/* �^�O�W�����v�@�\ */ //	Apr. 03, 2003 genta �����ǉ�
	case F_TAGJUMP_CLOSE:	Command_TAGJUMP(true);break;					/* �^�O�W�����v(���E�B���h�Eclose) *///	Apr. 03, 2003 genta
	case F_TAGJUMPBACK:		Command_TAGJUMPBACK();break;					/* �^�O�W�����v�o�b�N�@�\ */
	case F_TAGS_MAKE:		Command_TagsMake();break;						//�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	case F_DIRECT_TAGJUMP:	Command_TagJumpByTagsFile();break;				/* �_�C���N�g�^�O�W�����v�@�\ */	//@@@ 2003.04.15 MIK
	case F_TAGJUMP_KEYWORD:	Command_TagJumpByTagsFileKeyword( (const char*)lparam1 );break;	/* @@ 2005.03.31 MIK �L�[���[�h���w�肵�ă_�C���N�g�^�O�W�����v�@�\ */
	case F_COMPARE:			Command_COMPARE();break;						/* �t�@�C�����e��r */
	case F_DIFF_DIALOG:		Command_Diff_Dialog();break;					/* DIFF�����\��(�_�C�A���O) */	//@@@ 2002.05.25 MIK
	case F_DIFF:			Command_Diff( (const char*)lparam1, (int)lparam2 );break;		/* DIFF�����\�� */	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
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
	case F_FAVORITE:		Command_Favorite();break;		//���C�ɓ���	//@@@ 2003.04.08 MIK
	//	Jan. 29, 2005 genta ���p���̐ݒ�
	case F_SET_QUOTESTRING:	Command_SET_QUOTESTRING((const char*)lparam1);	break;

	/* �}�N���n */
	case F_RECKEYMACRO:		Command_RECKEYMACRO();break;	/* �L�[�}�N���̋L�^�J�n�^�I�� */
	case F_SAVEKEYMACRO:	Command_SAVEKEYMACRO();break;	/* �L�[�}�N���̕ۑ� */
	case F_LOADKEYMACRO:	Command_LOADKEYMACRO();break;	/* �L�[�}�N���̓ǂݍ��� */
	case F_EXECKEYMACRO:									/* �L�[�}�N���̎��s */
		/* �ċA�����΍� */
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
		Command_EXECKEYMACRO();break;
	//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
	//	case F_EXECCMMAND:		Command_EXECCMMAND();break;	/* �O���R�}���h���s */
	case F_EXECCOMMAND_DIALOG:
		/* �ċA�����΍� */// 2001/06/23 N.Nakatani
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
		Command_EXECCOMMAND_DIALOG((const char*)lparam1);	/* �O���R�}���h���s */
		break;
	//	To Here Sept. 20, 2000
	case F_EXECCOMMAND:
		Command_EXECCOMMAND((const char*)lparam1);
		break;

	/* �J�X�^�����j���[ */
	case F_MENU_RBUTTON:	/* �E�N���b�N���j���[ */
		/* �ċA�����΍� */
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
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
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
		nFuncID = Command_CUSTMENU( nCommand - F_CUSTMENU_1 + 1 );
		if( 0 != nFuncID ){
			/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
//			HandleCommand( nFuncID, TRUE, 0, 0, 0, 0 );
			::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ), (LPARAM)NULL );
		}
		break;

	/* �E�B���h�E�n */
	case F_SPLIT_V:			Command_SPLIT_V();break;	/* �㉺�ɕ��� */	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	case F_SPLIT_H:			Command_SPLIT_H();break;	/* ���E�ɕ��� */	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	case F_SPLIT_VH:		Command_SPLIT_VH();break;	/* �c���ɕ��� */	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	case F_WINCLOSE:		Command_WINCLOSE();break;	//�E�B���h�E�����
	case F_WIN_CLOSEALL:	/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�����L�̂悤�ɕύX
		//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
		/* �ċA�����΍� */
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
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
	case F_TRACEOUT:		Command_TRACEOUT((const char*)lparam1, (int)lparam2);break;		//�}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\�� maru 2006.04.26
	case F_TOPMOST:			Command_WINTOPMOST( lparam1 );break;	//��Ɏ�O�ɕ\�� Moca
	case F_WINLIST:			Command_WINLIST( nCommandFrom );break;		/* �E�B���h�E�ꗗ�|�b�v�A�b�v�\������ */	// 2006.03.23 fon // 2006.05.19 genta �����ǉ�

	/* �x�� */
	case F_HOKAN:			Command_HOKAN();break;			//���͕⊮
	case F_HELP_CONTENTS:	Command_HELP_CONTENTS();break;	/* �w���v�ڎ� */				//Nov. 25, 2000 JEPRO �ǉ�
	case F_HELP_SEARCH:		Command_HELP_SEARCH();break;	/* �w���v�g�L�[���[�h���� */	//Nov. 25, 2000 JEPRO �ǉ�
	case F_TOGGLE_KEY_SEARCH:	Command_ToggleKeySearch();break;	/* �L�����b�g�ʒu�̒P���������������@�\ON-OFF */	// 2006.03.24 fon
	case F_MENU_ALLFUNC:									/* �R�}���h�ꗗ */
		/* �ċA�����΍� */
		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
			delete m_pcOpeBlk;
			m_pcOpeBlk = NULL;
		}
		Command_MENU_ALLFUNC();break;
	case F_EXTHELP1:	Command_EXTHELP1();break;		/* �O���w���v�P */
	case F_EXTHTMLHELP:	/* �O��HTML�w���v */
		//	Jul. 5, 2002 genta
		Command_EXTHTMLHELP( (const char*)lparam1, (const char*)lparam2 ); break;
	case F_ABOUT:	Command_ABOUT();break;				/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	/* ���̑� */
//	case F_SENDMAIL:	Command_SENDMAIL();break;		/* ���[�����M */

	}

	/* �A���h�D�o�b�t�@�̏��� */
	if( NULL != m_pcOpeBlk ){
		if( 0 < m_pcOpeBlk->GetNum() ){	/* ����̐���Ԃ� */
			/* ����̒ǉ� */
			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );

			m_pcEditDoc->RedrawInactivePane();	//	���̃y�C���̕\��
#if 0
		//	2001/06/21 Start by asa-o: ���̃y�C���̕\����Ԃ��X�V
			m_pcEditDoc->m_cEditViewArr[m_nMyIndex^1].Redraw();
			m_pcEditDoc->m_cEditViewArr[m_nMyIndex^2].Redraw();
			m_pcEditDoc->m_cEditViewArr[(m_nMyIndex^1)^2].Redraw();
			DrawCaretPosInfo();
		//	2001/06/21 End
#endif
		}else{
			delete m_pcOpeBlk;
		}
		m_pcOpeBlk = NULL;
	}

	return bRet;
}



/////////////////////////////////// �ȉ��̓R�}���h�Q (Oct. 17, 2000 jepro note) ///////////////////////////////////////////

/*! �J�[�\����ړ� */
int CEditView::Command_UP( int bSelect, BOOL bRepeat, int lines )
{
	//	From Here Oct. 24, 2001 genta
	if( lines != 0 ){
		Cursor_UPDOWN( lines, FALSE );
		return 1;
	}
	//	To Here Oct. 24, 2001 genta

	int		i;
	int		nRepeat;
	nRepeat = 0;
//m_pShareData->m_Common.m_nRepeatedScrollLineNum;		/* �L�[���s�[�g���̃X�N���[���s�� */
//m_pShareData->m_Common.m_nRepeatedScroll_Smooth;		/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */

	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	if( !m_pShareData->m_Common.m_nRepeatedScroll_Smooth ){
		if( !bRepeat ){
			i = -1;
		}else{
			i = -1 * m_pShareData->m_Common.m_nRepeatedScrollLineNum;	/* �L�[���s�[�g���̃X�N���[���s�� */
		}
		Cursor_UPDOWN( i, bSelect );
		nRepeat = -1 * i;
	}else{
		++nRepeat;
		if( Cursor_UPDOWN( -1, bSelect ) && bRepeat ){
			for( i = 0; i < m_pShareData->m_Common.m_nRepeatedScrollLineNum - 1; ++i ){		/* �L�[���s�[�g���̃X�N���[���s�� */
				::UpdateWindow( m_hWnd );	//	YAZAKI
				Cursor_UPDOWN( -1, bSelect );
				++nRepeat;
			}
		}
	}
	return nRepeat;
}




/* �J�[�\�����ړ� */
int CEditView::Command_DOWN( int bSelect, BOOL bRepeat )
{
	int		i;
	int		nRepeat;
	nRepeat = 0;
	/* �L�[���s�[�g���̃X�N���[�������炩�ɂ��邩 */
	if( !m_pShareData->m_Common.m_nRepeatedScroll_Smooth ){
		if( !bRepeat ){
			i = 1;
		}else{
			i = m_pShareData->m_Common.m_nRepeatedScrollLineNum;	/* �L�[���s�[�g���̃X�N���[���s�� */
		}
		Cursor_UPDOWN( i, bSelect );
		nRepeat = i;
	}else{
		++nRepeat;
		if( Cursor_UPDOWN( 1, bSelect ) && bRepeat ){
			for( i = 0; i < m_pShareData->m_Common.m_nRepeatedScrollLineNum - 1; ++i ){	/* �L�[���s�[�g���̃X�N���[���s�� */
				//	�����ōĕ`��B
				::UpdateWindow( m_hWnd );	//	YAZAKI
				Cursor_UPDOWN( 1, bSelect );
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
int CEditView::Command_LEFT( int bSelect, BOOL bRepeat )
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
		int				nPosX = 0;
		int				nPosY = m_nCaretPosY;
		RECT			rcSel;
		const CLayout*	pcLayout;
		if( bSelect ){
			if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̃J�[�\���ʒu����I�����J�n���� */
				BeginSelectArea();
			}
		}else{
			if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ��`�͈͑I�𒆂� */
				if( m_bBeginBoxSelect ){
					/* 2�_��Ίp�Ƃ����`�����߂� */
					TwoPointToRect(
						&rcSel,
						m_nSelectLineFrom,		/* �͈͑I���J�n�s */
						m_nSelectColmFrom,		/* �͈͑I���J�n�� */
						m_nSelectLineTo,		/* �͈͑I���I���s */
						m_nSelectColmTo			/* �͈͑I���I���� */
					);
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					DisableSelectArea( TRUE );
					/* �J�[�\����I���J�n�ʒu�Ɉړ� */
					MoveCursor( rcSel.left, rcSel.top, TRUE );
					m_nCaretPosX_Prev = m_nCaretPosX;
				}else{
					nPosX = m_nSelectColmFrom;
					nPosY = m_nSelectLineFrom;
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					DisableSelectArea( TRUE );
					/* �J�[�\����I���J�n�ʒu�Ɉړ� */
					MoveCursor( nPosX, nPosY, TRUE );
					m_nCaretPosX_Prev = m_nCaretPosX;
				}
				nRes = 1;
				goto end_of_func;
			}
		}
		/* ���ݍs�̃f�[�^���擾 */
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
		/* �J�[�\�������[�ɂ��� */
		if( m_nCaretPosX == (pcLayout ? pcLayout->GetIndent() : 0)){
			if( m_nCaretPosY > 0 ){
				pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY - 1 );
				CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
				while( !it.end() ){
					it.scanNext();
					if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
						nPosX += it.getColumnDelta();
						break;
					}
					it.addDelta();
				}
				nPosX += it.getColumn() - it.getColumnDelta();
				nPosY --;
			}else{
				nRes = 0;
				goto end_of_func;
			}
		}
		//  2004.03.28 Moca EOF�����̍s�ȍ~�̓r���ɃJ�[�\��������Ɨ�����o�O�C��
		else if( pcLayout ){
			CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				if ( it.getColumn() + it.getColumnDelta() > m_nCaretPosX - 1 ){
					nPosX += it.getColumnDelta();
					break;
				}
				it.addDelta();
			}
			nPosX += it.getColumn() - it.getColumnDelta();
			//	Oct. 18, 2002 YAZAKI
			if( it.getIndex() >= pcLayout->GetLengthWithEOL() ){
				nPosX = m_nCaretPosX - 1;
			}
		}
		MoveCursor( nPosX, nPosY, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( bSelect ){
			/*	���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX�D
			
				2004.04.02 Moca 
				�L�����b�g�ʒu���s���������ꍇ��MoveCursor�̈ړ����ʂ�
				�����ŗ^�������W�Ƃ͈قȂ邱�Ƃ����邽�߁C
				nPosX, nPosY�̑���Ɏ��ۂ̈ړ����ʂ��g���悤�ɁD
			*/
			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		}
		nRes = 1;
		goto end_of_func;
end_of_func:;
	}
	return nRes;
}




/* �J�[�\���E�ړ� */
void CEditView::Command_RIGHT( int bSelect, int bIgnoreCurrentSelection, BOOL bRepeat )
{
	int		nRepCount;
	int		nRepeat;
	if( bRepeat ){
		nRepeat = 2;
	}else{
		nRepeat = 1;
	}
	for( nRepCount = 0; nRepCount < nRepeat; ++nRepCount ){
		int			nPosX;
		int			nPosY = m_nCaretPosY;
		RECT		rcSel;
		const CLayout*	pcLayout;
		// 2003.06.28 Moca [EOF]�݂̂̍s�ɃJ�[�\��������Ƃ��ɉE�������Ă��I���������ł��Ȃ�����
		// �Ή����邽�߁A���ݍs�̃f�[�^���擾���ړ�
		if( !bIgnoreCurrentSelection ){
			if( bSelect ){
				if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
					/* ���݂̃J�[�\���ʒu����I�����J�n���� */
					BeginSelectArea();
				}
			}else{
				if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
					/* ��`�͈͑I�𒆂� */
					if( m_bBeginBoxSelect ){
						/* 2�_��Ίp�Ƃ����`�����߂� */
						TwoPointToRect(
							&rcSel,
							m_nSelectLineFrom,		/* �͈͑I���J�n�s */
							m_nSelectColmFrom,		/* �͈͑I���J�n�� */
							m_nSelectLineTo,		/* �͈͑I���I���s */
							m_nSelectColmTo			/* �͈͑I���I���� */
						);
						/* ���݂̑I��͈͂��I����Ԃɖ߂� */
						DisableSelectArea( TRUE );
						/* �J�[�\����I���I���ʒu�Ɉړ� */
						MoveCursor( rcSel.right, rcSel.bottom, TRUE );
						m_nCaretPosX_Prev = m_nCaretPosX;
					}else{
						nPosX = m_nSelectColmTo;
						nPosY = m_nSelectLineTo;

						/* ���݂̑I��͈͂��I����Ԃɖ߂� */
						DisableSelectArea( TRUE );
						if( nPosY >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
							/* �t�@�C���̍Ō�Ɉړ� */
							Command_GOFILEEND(FALSE);
						}else{
							/* �J�[�\����I���I���ʒu�Ɉړ� */
							MoveCursor( nPosX, nPosY, TRUE );
							m_nCaretPosX_Prev = m_nCaretPosX;
						}
					}
					goto end_of_func;
				}
			}
		}
//		2003.06.28 Moca [EOF]�݂̂̍s�ɃJ�[�\��������Ƃ��ɉE�������Ă��I���������ł��Ȃ����ɑΉ�
		/* ���ݍs�̃f�[�^���擾 */
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
		//	2004.04.02 EOF�ȍ~�ɃJ�[�\�����������Ƃ��ɉE�������Ă������N���Ȃ������̂��AEOF�Ɉړ�����悤��
		if( pcLayout )
		{
			int nIndex = 0;
			CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
			while( !it.end() ){
				it.scanNext();
				if ( it.getColumn() + it.getColumnDelta() > m_nCaretPosX + it.getColumnDelta() ){
					break;
				}
				if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
					nIndex += it.getIndexDelta();
					break;
				}
				it.addDelta();
			}
			nPosX = it.getColumn();
			nIndex += it.getIndex();
			if( nIndex >= pcLayout->GetLengthWithEOL() ){
				/* �t���[�J�[�\�����[�h�� */
				if( (
					m_pShareData->m_Common.m_bIsFreeCursorMode
				 || IsTextSelected() && m_bBeginBoxSelect	/* ��`�͈͑I�� */
					)
				 &&
					/* ���s�ŏI����Ă��邩 */
					( EOL_NONE != pcLayout->m_cEol )
				){
					/*-- �t���[�J�[�\�����[�h�̏ꍇ --*/
					if( nPosX <= m_nCaretPosX ){
						/* �ŏI�s�� */
						if( m_nCaretPosY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
							/* ���s�ŏI����Ă��邩 */
							if( EOL_NONE != pcLayout->m_cEol.GetType() ){
								nPosX = m_nCaretPosX + 1;
							}else{
								nPosX = m_nCaretPosX;
							}
						}else{
							nPosX = m_nCaretPosX + 1;
						}
					}else{
						nPosX = nPosX;
					}
				}else{
					/*-- �t���[�J�[�\�����[�h�ł͂Ȃ��ꍇ --*/
					/* �ŏI�s�� */
					if( m_nCaretPosY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
						/* ���s�ŏI����Ă��邩 */
						if( EOL_NONE != pcLayout->m_cEol.GetType() ){
							nPosX = pcLayout->m_pNext ? pcLayout->m_pNext->GetIndent() : 0;
							++nPosY;
						}else{
						}
					}else{
						nPosX = pcLayout->m_pNext ? pcLayout->m_pNext->GetIndent() : 0;
						++nPosY;
					}
				}
			}
			//	�L�����b�g�ʒu���܂�Ԃ��ʒu���E���������ꍇ�̏���
			//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
			if( nPosX >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() ){
				if( m_pcEditDoc->GetDocumentAttribute().m_bKinsokuRet
				 || m_pcEditDoc->GetDocumentAttribute().m_bKinsokuKuto )	//@@@ 2002.04.16 MIK
				{
					if( m_pcEditDoc->m_cLayoutMgr.IsEndOfLine( nPosY, nPosX ) )	//@@@ 2002.04.18
					{
						nPosX = 0;
						++nPosY;
					}
				}
				else
				{
					nPosX = 0;
					++nPosY;
				}
			}
		}else{
			// pcLayout��NULL�̏ꍇ��nPosX=0�ɒ���
			nPosX = 0;
		}
		MoveCursor( nPosX, nPosY, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( bSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		}

end_of_func:;
	}
	return;
}




//	From Here Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL/UP/DOWN��PAGE/DOWN/UP)���邽�߂Ɉȉ����R�����g�A�E�g
///* �X�N���[���A�b�v */
//void CEditView::Command_ROLLUP( int bSelect )
//{
//	Cursor_UPDOWN( ( m_nViewRowNum / 2 ), bSelect );
//	return;
//}
//
//
//
//
///* �X�N���[���_�E�� */
//void CEditView::Command_ROLLDOWN( int bSelect )
//{
//	Cursor_UPDOWN( - ( m_nViewRowNum / 2 ), bSelect );
//	return;
//}
//	To Here Oct. 6, 2000




/* ���y�[�W�A�b�v */	//Oct. 6, 2000 JEPRO added (���͏]���̃X�N���[���_�E�����̂���)
void CEditView::Command_HalfPageUp( int bSelect )
{
	Cursor_UPDOWN( - ( m_nViewRowNum / 2 ), bSelect );
	return;
}




/* ���y�[�W�_�E�� */	//Oct. 6, 2000 JEPRO added (���͏]���̃X�N���[���A�b�v���̂���)
void CEditView::Command_HalfPageDown( int bSelect )
{
	Cursor_UPDOWN( ( m_nViewRowNum / 2 ), bSelect );
	return;
}




/*! �P�y�[�W�A�b�v

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�A�b�v�ɓ���ύX
*/	//Oct. 10, 2000 JEPRO added
void CEditView::Command_1PageUp( int bSelect )
{
//	Cursor_UPDOWN( - m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�A�b�v
	if(m_nViewTopLine>=m_nViewRowNum-1){
		m_bDrawSWITCH = FALSE;
		int nViewTopLine=m_nCaretPosY-m_nViewTopLine;
		Cursor_UPDOWN( -m_nViewRowNum+1, bSelect );
		//	Sep. 11, 2004 genta �����X�N���[�������̂���
		//	RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		SyncScrollV( ScrollAtV( m_nCaretPosY-nViewTopLine ));
		m_bDrawSWITCH = TRUE;
		RedrawAll();
		
	}else{
		Cursor_UPDOWN( -m_nViewRowNum+1, bSelect );
	}
	return;
}




/*!	�P�y�[�W�_�E��

	@date 2000.10.10 JEPRO �쐬
	@date 2001.12.13 hor ��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂�
		�P�y�[�W�_�E���ɓ���ύX
*/
void CEditView::Command_1PageDown( int bSelect )
{
//	Cursor_UPDOWN( m_nViewRowNum, bSelect );

// 2001.12.03 hor
//		���������C�N�ɁA��ʂɑ΂���J�[�\���ʒu�͂��̂܂܂łP�y�[�W�_�E��
	if(m_nViewTopLine+m_nViewRowNum <= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){ //- m_nViewRowNum){
		m_bDrawSWITCH = FALSE;
		int nViewTopLine=m_nCaretPosY-m_nViewTopLine;
		Cursor_UPDOWN( m_nViewRowNum-1, bSelect );
		//	Sep. 11, 2004 genta �����X�N���[�������̂���
		//	RedrawAll�ł͂Ȃ�ScrollAt���g���悤��
		SyncScrollV( ScrollAtV( m_nCaretPosY-nViewTopLine ));
		m_bDrawSWITCH = TRUE;
		RedrawAll();
	}else{
		Cursor_UPDOWN( m_nViewRowNum , bSelect );
		Command_DOWN( bSelect, TRUE );
	}

	return;
}




/* �J�[�\����ړ�(�Q�s�Â�) */
void CEditView::Command_UP2( int bSelect )
{
	Cursor_UPDOWN( -2, bSelect );
	return;
}




/* �J�[�\�����ړ�(�Q�s�Â�) */
void CEditView::Command_DOWN2( int bSelect )
{
	Cursor_UPDOWN( 2, bSelect );
	return;
}




/*! @brief �s���Ɉړ�

	@param bSelect [in] �I���̗L���Btrue: �I�����Ȃ���ړ��Bfalse: �I�����Ȃ��ňړ��B
	@param lparam [in] �}�N������g�p����g���t���O
		@li 0: �L�[����Ɠ���(default)
		@li 1: �J�[�\���ʒu�Ɋ֌W�Ȃ��s���Ɉړ��B
		@li 4: �I�����Ĉړ�(������)
		@li 8: ���s�P�ʂŐ擪�Ɉړ�(������)
	
	@date Oct. 29, 2001 genta �}�N���p�@�\�g��(�p�����[�^�ǉ�) + goto�r��
	@date May. 15, 2002 oak   ���s�P�ʈړ�
	@date Oct.  7, 2002 YAZAKI �璷�Ȉ��� bLineTopOnly ���폜
*/
void CEditView::Command_GOLINETOP( int bSelect, int lparam )
{
	bool			bLineTopOnly = false;
	int				nCaretPosX;
	int				nCaretPosY;
	int				nPos;
	int				nPosY;
	const CLayout*	pcLayout;

	if( lparam & 1 ){
		bLineTopOnly = true;
	}
	
	if( lparam & 4 ){
		bSelect = TRUE;
	}

	if ( lparam & 8 ){
		/* ���s�P�ʎw��̏ꍇ�́A�����s���ʒu����ړI�_���ʒu�����߂� */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
			0,
			m_nCaretPosY_PHY,
			&nCaretPosX,
			&nCaretPosY
		);
	}else{
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
		nCaretPosX = pcLayout ? pcLayout->GetIndent() : 0;
		nCaretPosY = m_nCaretPosY;
	}
	if( !bLineTopOnly ){
		/* �ړI�s�̃f�[�^���擾 */
		/* ���s�P�ʎw��ŁA�擪����󔒂�1�܂�Ԃ��s�ȏ㑱���Ă���ꍇ�͎��̍s�f�[�^���擾 */
		nPosY = nCaretPosY - 1;
		const char*		pLine;
		int				nLineLen;
		const CLayout*	pcLayout;
		do {
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ++nPosY, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				return;
			}
			for( nPos = 0; nPos < nLineLen; ++nPos ){
				if( ' ' != pLine[nPos] && '\t' != pLine[nPos] ){
					if( CR == pLine[nPos] || LF == pLine[nPos] ){
						nPos = nLineLen;
					}
					break;
				}
			}
		} while (( lparam & 8 ) && (nPos >= nLineLen) && (m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 > nPosY) );
		if( nPos >= nLineLen ){
			nPos = 0;
			nPosY = nCaretPosY;
		}
		/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
		nPos = LineIndexToColmn( pcLayout, nPos );
		if( (m_nCaretPosX != nPos) || (m_nCaretPosY != nPosY) ){
			nCaretPosX = nPos;
			nCaretPosY = nPosY;
		}
	}

	//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	MoveCursorSelecting( nCaretPosX, nCaretPosY, bSelect );
}




/* �s���Ɉړ�(�܂�Ԃ��P��) */
void CEditView::Command_GOLINEEND( int bSelect, int bIgnoreCurrentSelection )
{
	int				nPosX = 0;
	int				nPosY = m_nCaretPosY;
	const CLayout*	pcLayout;
	if( !bIgnoreCurrentSelection ){
		if( bSelect ){
			if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̃J�[�\���ʒu����I�����J�n���� */
				BeginSelectArea();
			}
		}else{
			if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				DisableSelectArea( TRUE );
			}
		}
	}
	/* ���ݍs�̃f�[�^���擾 */
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
			nPosX += it.getColumnDelta();
			break;
		}
		it.addDelta();
	}
	nPosX += it.getColumn() - it.getColumnDelta();
	if( it.getIndex() >= (pcLayout ? pcLayout->GetLengthWithEOL() : 0) ){
		if( m_nCaretPosY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
			/* ���s�ŏI����Ă��邩 */
			if( EOL_NONE != pcLayout->m_cEol ){
			}else{
				nPosX += it.getColumnDelta();
			}
		}
	}
	
	MoveCursor( nPosX, m_nCaretPosY, TRUE );
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( bSelect ){
		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
		ChangeSelectAreaByCurrentCursor( nPosX, m_nCaretPosY );
	}
	return;
}




/* �t�@�C���̐擪�Ɉړ� */
void CEditView::Command_GOFILETOP( int bSelect )
{
	/* �擪�փJ�[�\�����ړ� */
	//	Sep. 8, 2000 genta
	AddCurrentLineToHistory();

	//	2006.07.09 genta �V�K�֐��ɂ܂Ƃ߂�
	MoveCursorSelecting( (!m_bBeginBoxSelect)? 0: m_nCaretPosX, 0, bSelect );	//	�ʏ�́A(0, 0)�ֈړ��B�{�b�N�X�I�𒆂́A(m_nCaretPosX, 0)�ֈړ�
}




/* �t�@�C���̍Ō�Ɉړ� */
void CEditView::Command_GOFILEEND( int bSelect )
{
// 2001.12.13 hor BOX�I�𒆂Ƀt�@�C���̍Ō�ɃW�����v�����[EOF]�̍s�����]�����܂܂ɂȂ�̏C��
	if( !bSelect && IsTextSelected() ) DisableSelectArea( TRUE );	// 2001.12.21 hor Add
	AddCurrentLineToHistory();
	Cursor_UPDOWN( m_pcEditDoc->m_cLayoutMgr.GetLineCount() , bSelect );
	Command_DOWN( bSelect, TRUE );
	if ( !m_bBeginBoxSelect ){							// 2002/04/18 YAZAKI
		/*	2004.04.19 fotomo
			���s�̂Ȃ��ŏI�s�őI�����Ȃ��當�����ֈړ������ꍇ��
			�I��͈͂��������Ȃ��ꍇ��������ɑΉ�
		*/
		Command_GOLINEEND( bSelect, FALSE );				// 2001.12.21 hor Add
	}
	MoveCursor( m_nCaretPosX, m_nCaretPosY, TRUE );	// 2001.12.21 hor Add
	// 2002.02.16 hor ��`�I�𒆂��������O�̃J�[�\���ʒu�����Z�b�g
	if( !(IsTextSelected() && m_bBeginBoxSelect) ) m_nCaretPosX_Prev = m_nCaretPosX;
	return;
// 2001.12.13 hor �ȉ����R�����g (////�͂��Ƃ��ƃR�����g)
//	HDC			hdc;
//	PAINTSTRUCT	ps;
//	int bIsFreeCursorMode_old = m_pShareData->m_Common.m_bIsFreeCursorMode;
//	int	nLastLine;
//	if( 0 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
//		return;
//	}
//	m_pShareData->m_Common.m_bIsFreeCursorMode = FALSE;
//	if( bSelect ){
//		if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
//			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
//			BeginSelectArea();
//		}
////		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
////		ChangeSelectAreaByCurrentCursor( 0, 0 );
//	}else{
//		if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
//			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
//			DisableSelectArea( TRUE );
//		}
//	}
//	nLastLine = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1;
//	//	Sep. 8, 2000 genta
//	AddCurrentLineToHistory();
//	MoveCursor( 0, nLastLine, TRUE );
//	if( bSelect ){
//		m_nSelectLineTo = m_nCaretPosY;		/* �͈͑I���I���s */
//	}
//	Command_DOWN( bSelect, TRUE );
//	Command_GOLINEEND( bSelect, TRUE );
//	Command_RIGHT( bSelect, TRUE, FALSE );
////��߂�	m_nCaretPosX_Prev = m_nCaretPosX;
//	m_pShareData->m_Common.m_bIsFreeCursorMode = bIsFreeCursorMode_old;
//	/* �ĕ`�� */
//	hdc = ::GetDC( m_hWnd );
//	ps.rcPaint.left = 0;
//	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
//	ps.rcPaint.top = m_nViewAlignTop;
//	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//	OnKillFocus();
//	OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//	OnSetFocus();
//	::ReleaseDC( m_hWnd, hdc );
//	return;
}




/* �P��̍��[�Ɉړ� */
void CEditView::Command_WORDLEFT( int bSelect )
{
	int				nIdx;
	int				nLineNew;
	int				nColmNew;
	BOOL			bIsFreeCursorModeOld;
	if( bSelect ){
		if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){		/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( TRUE );
		}
	}

	const CLayout* pcLayout;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
	if( NULL == pcLayout ){
		bIsFreeCursorModeOld = m_pShareData->m_Common.m_bIsFreeCursorMode;	/* �t���[�J�[�\�����[�h�� */
		m_pShareData->m_Common.m_bIsFreeCursorMode = FALSE;
		/* �J�[�\�����ړ� */
		Command_LEFT( bSelect, FALSE );
		m_pShareData->m_Common.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* �t���[�J�[�\�����[�h�� */
		return;
	}
	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );
	/* ���݈ʒu�̍��̒P��̐擪�ʒu�𒲂ׂ� */
	if( m_pcEditDoc->m_cLayoutMgr.PrevWord( m_nCaretPosY, nIdx, &nLineNew, &nColmNew, m_pShareData->m_Common.m_bStopsBothEndsWhenSearchWord ) ){
		/* �s���ς���� */
		if( nLineNew != m_nCaretPosY ){
			pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineNew );
			if( NULL == pcLayout ){
				return;
			}
		}
		/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
		nColmNew = LineIndexToColmn( pcLayout, nColmNew );
		/* �J�[�\���ړ� */
		MoveCursor( nColmNew, nLineNew, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( bSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			ChangeSelectAreaByCurrentCursor( nColmNew, nLineNew );
		}
	}else{
		bIsFreeCursorModeOld = m_pShareData->m_Common.m_bIsFreeCursorMode;	/* �t���[�J�[�\�����[�h�� */
		m_pShareData->m_Common.m_bIsFreeCursorMode = FALSE;
		/* �J�[�\�����ړ� */
		Command_LEFT( bSelect, FALSE );
//		if( 0 < m_nCaretPosY ){
//			/* �s���Ɉړ� */
//			Command_GOLINEEND( bSelect, FALSE );
//		}
		m_pShareData->m_Common.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* �t���[�J�[�\�����[�h�� */
	}
	return;
}




/* �P��̉E�[�Ɉړ� */
void CEditView::Command_WORDRIGHT( int bSelect )
{
	int			nIdx;
	int			nCurLine;
	int			nLineNew;
	int			nColmNew;
	int			bTryAgain;
	BOOL		bIsFreeCursorModeOld;
	if( bSelect ){
		if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( TRUE );
		}
	}
	bTryAgain = FALSE;
try_again:;
	nCurLine = m_nCaretPosY;
	const CLayout* pcLayout;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nCurLine );
	if( NULL == pcLayout ){
		return;
	}
	if( bTryAgain ){
		const char*	pLine = pcLayout->GetPtr();
		if( pLine[0] != ' ' && pLine[0] != TAB ){
			return;
		}
	}
	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );
	/* ���݈ʒu�̉E�̒P��̐擪�ʒu�𒲂ׂ� */
	if( m_pcEditDoc->m_cLayoutMgr.NextWord( nCurLine, nIdx, &nLineNew, &nColmNew, m_pShareData->m_Common.m_bStopsBothEndsWhenSearchWord ) ){
		/* �s���ς���� */
		if( nLineNew != nCurLine ){
			pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineNew );
			if( NULL == pcLayout ){
				return;
			}
		}
		/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
		nColmNew = LineIndexToColmn( pcLayout, nColmNew );
		/* �J�[�\���ړ� */
		MoveCursor( nColmNew, nLineNew, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( bSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			ChangeSelectAreaByCurrentCursor( nColmNew, nLineNew );
		}
	}else{
		bIsFreeCursorModeOld = m_pShareData->m_Common.m_bIsFreeCursorMode;	/* �t���[�J�[�\�����[�h�� */
		m_pShareData->m_Common.m_bIsFreeCursorMode = FALSE;
		/* �J�[�\���E�ړ� */
		Command_RIGHT( bSelect, FALSE, FALSE );
		m_pShareData->m_Common.m_bIsFreeCursorMode = bIsFreeCursorModeOld;	/* �t���[�J�[�\�����[�h�� */
		if( FALSE == bTryAgain ){
			bTryAgain = TRUE;
			goto try_again;
		}
	}
	return;
}



/*!	�I��͈͂��N���b�v�{�[�h�ɃR�s�[
	@param bIgnoreLockAndDisable [in] �I��͈͂��������邩�H
	@param bAddCRLFWhenCopy [in] �܂�Ԃ��ʒu�ɉ��s�R�[�h��}�����邩�H
	@param neweol [in] �R�s�[����Ƃ���EOL�B
*/
void CEditView::Command_COPY(
	int bIgnoreLockAndDisable,
	BOOL bAddCRLFWhenCopy,
	enumEOLType neweol
)
{
	CMemory			cmemBuf;
	BOOL			bBeginBoxSelect = FALSE;

	/* �N���b�v�{�[�h�ɓ����ׂ��e�L�X�g�f�[�^���AcmemBuf�Ɋi�[���� */
	if( IsTextSelected() ){
		/* �e�L�X�g���I������Ă���Ƃ��́A�I��͈͂̃f�[�^���擾 */

		if( m_bBeginBoxSelect ){
			bBeginBoxSelect = TRUE;
		}
		/* �I��͈͂̃f�[�^���擾 */
		/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
		if( FALSE == GetSelectedData( cmemBuf, FALSE, NULL, FALSE, bAddCRLFWhenCopy, neweol ) ){
			::MessageBeep( MB_ICONHAND );
			return;
		}
	}else{
		/* ��I�����́A�J�[�\���s���R�s�[���� */

		const CLayout*	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
		if( NULL == pcLayout ){
			return;
		}
		cmemBuf.SetData( pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() );
		if( pcLayout->m_cEol.GetLen() != 0 ){
			cmemBuf.AppendSz(
				( neweol == EOL_UNKNOWN ) ?
					pcLayout->m_cEol.GetValue() : CEOL(neweol).GetValue()
			);
		}
	}

	/* �N���b�v�{�[�h�Ƀf�[�^cmemBuf�̓��e��ݒ� */
	if( FALSE == MySetClipboardData( cmemBuf.GetPtr(), cmemBuf.GetLength(), bBeginBoxSelect ) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}

	/* �I��͈͂̌�Еt�� */
	if( !bIgnoreLockAndDisable ){
		/* �I����Ԃ̃��b�N */
		if( m_bSelectingLock ){
			m_bSelectingLock = FALSE;
		}
	}
	if( m_pShareData->m_Common.m_bCopyAndDisablSelection ){	/* �R�s�[������I������ */
		/* �e�L�X�g���I������Ă��邩 */
		if( IsTextSelected() ){
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( TRUE );
		}
	}
	return;
}




/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
void CEditView::Command_CUT( void )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}

	CMemory	cmemBuf;
	BOOL	bBeginBoxSelect;
	/* �͈͑I��������Ă��Ȃ� */
	if( !IsTextSelected() ){
		/* ��I�����́A�J�[�\���s��؂��� */
		//�s�؂���(�܂�Ԃ��P��)
		Command_CUT_LINE();
		return;
	}
	if( m_bBeginBoxSelect ){
		bBeginBoxSelect = TRUE;
	}else{
		bBeginBoxSelect = FALSE;
	}
	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta
	//SetParentCaption();	/* �e�E�B���h�E�̃^�C�g�����X�V */


	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
	if( FALSE == GetSelectedData( cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	if( FALSE == MySetClipboardData( cmemBuf.GetPtr(), cmemBuf.GetLength(), bBeginBoxSelect ) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}

	/* �J�[�\���ʒu�܂��͑I���G���A���폜 */
	DeleteData( TRUE );
	return;
}




//�J�[�\���ʒu�܂��͑I���G���A���폜
void CEditView::Command_DELETE( void )
{
	if( m_bBeginSelect ){		/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}

	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		DeleteData( TRUE );
		return;
	}
	DeleteData( TRUE );
	return;
}




//�J�[�\���O���폜
void CEditView::Command_DELETE_BACK( void )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}

	COpe*		pcOpe = NULL;
	BOOL		bBool;
	int			nPosX;
	int			nPosY;
	//	May 29, 2004 genta ���ۂɍ폜���ꂽ�������Ȃ��Ƃ��̓t���O�����ĂȂ��悤��
	//m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta
	if( IsTextSelected() ){				/* �e�L�X�g���I������Ă��邩 */
		DeleteData( TRUE );
		m_pcEditDoc->SetModified(true,true);	//	May 29, 2004 genta
	}else{
		nPosX = m_nCaretPosX;
		nPosY = m_nCaretPosY;
		bBool = Command_LEFT( FALSE, FALSE );
		if( bBool ){
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				pcOpe = new COpe;
				pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
//				pcOpe->m_nCaretPosX_Before = nPosX;			/* ����O�̃L�����b�g�ʒu�w */
//				pcOpe->m_nCaretPosY_Before = nPosY;			/* ����O�̃L�����b�g�ʒu�x */
//				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//					pcOpe->m_nCaretPosX_Before,
//					pcOpe->m_nCaretPosY_Before,
//					&pcOpe->m_nCaretPosX_PHY_Before,
//					&pcOpe->m_nCaretPosY_PHY_Before
//				);
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					nPosX,
					nPosY,
					&pcOpe->m_nCaretPosX_PHY_Before,
					&pcOpe->m_nCaretPosY_PHY_Before
				);


//				pcOpe->m_nCaretPosX_After = m_nCaretPosX;	/* �����̃L�����b�g�ʒu�w */
//				pcOpe->m_nCaretPosY_After = m_nCaretPosY;	/* �����̃L�����b�g�ʒu�x */
//				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//					pcOpe->m_nCaretPosX_After,
//					pcOpe->m_nCaretPosY_After,
//					&pcOpe->m_nCaretPosX_PHY_After,
//					&pcOpe->m_nCaretPosY_PHY_After
//				);
				pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
				pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
				/* ����̒ǉ� */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
			DeleteData( TRUE );
			m_pcEditDoc->SetModified(true,true);	//	May 29, 2004 genta
		}
	}
	PostprocessCommand_hokan();	//	Jan. 10, 2005 genta �֐���
	return;
}




//�P��̉E�[�܂ō폜
void CEditView::Command_WordDeleteToEnd( void )
{
	COpe*	pcOpe = NULL;
	CMemory	cmemData;

	/* ��`�I����Ԃł͎��s�s�\((��������蔲������)) */
	if( IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_bBeginBoxSelect ){
			::MessageBeep( MB_ICONHAND );
			return;
		}
	}
	/* �P��̉E�[�Ɉړ� */
	CEditView::Command_WORDRIGHT( TRUE );
	if( !IsTextSelected() ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;							/* ������ */
//		pcOpe->m_nCaretPosX_Before = m_nSelectColmFrom;			/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = m_nSelectLineFrom;			/* ����O�̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_Before,
//			pcOpe->m_nCaretPosY_Before,
//			&pcOpe->m_nCaretPosX_PHY_Before,
//			&pcOpe->m_nCaretPosY_PHY_Before
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			m_nSelectColmFrom,
			m_nSelectLineFrom,
			&pcOpe->m_nCaretPosX_PHY_Before,
			&pcOpe->m_nCaretPosY_PHY_Before
		);


//		pcOpe->m_nCaretPosX_After = pcOpe->m_nCaretPosX_Before;			/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = pcOpe->m_nCaretPosY_Before;			/* �����̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	/* �폜 */
	DeleteData( TRUE );
	return;
}




//�P��̍��[�܂ō폜
void CEditView::Command_WordDeleteToStart( void )
{
	COpe*	pcOpe = NULL;
	CMemory	cmemData;
	/* ��`�I����Ԃł͎��s�s�\(��������蔲������) */
	if( IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_bBeginBoxSelect ){
			::MessageBeep( MB_ICONHAND );
			return;
		}
	}
	/* �P��̍��[�Ɉړ� */
	CEditView::Command_WORDLEFT( TRUE );
	if( !IsTextSelected() ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
//		pcOpe->m_nOpe = OPE_MOVECARET;					/* ������ */
//		pcOpe->m_nCaretPosX_Before = m_nSelectColmTo;	/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = m_nSelectLineTo;	/* ����O�̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_Before,
//			pcOpe->m_nCaretPosY_Before,
//			&pcOpe->m_nCaretPosX_PHY_Before,
//			&pcOpe->m_nCaretPosY_PHY_Before
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			m_nSelectColmTo,
			m_nSelectLineTo,
			&pcOpe->m_nCaretPosX_PHY_Before,
			&pcOpe->m_nCaretPosY_PHY_Before
		);


//		pcOpe->m_nCaretPosX_After = pcOpe->m_nCaretPosX_Before;			/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = pcOpe->m_nCaretPosY_Before;			/* �����̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	/* �폜 */
	DeleteData( TRUE );
	return;
}




//�P��؂���
void CEditView::Command_WordCut( void )
{
	if( IsTextSelected() ){
		/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
		Command_CUT();
		return;
	}
	//���݈ʒu�̒P��I��
	Command_SELECTWORD();
	/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
	if ( !IsTextSelected() ){
		//	�P��I���őI���ł��Ȃ�������A���̕�����I�Ԃ��Ƃɒ���B
		Command_RIGHT( TRUE, FALSE, FALSE );
	}
	Command_CUT();
	return;
}




//�P��폜
void CEditView::Command_WordDelete( void )
{
	if( IsTextSelected() ){
		/* �폜 */
		DeleteData( TRUE );
		return;
	}
	//���݈ʒu�̒P��I��
	Command_SELECTWORD();
	/* �폜 */
	DeleteData( TRUE );
	return;
}




//�s���܂Ő؂���(���s�P��)
void CEditView::Command_LineCutToStart( void )
{
	int			nX;
	int			nY;
	CLayout*	pCLayout;
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
		Command_CUT();
		return;
	}
	pCLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( 0, pCLayout->m_nLinePhysical, &nX, &nY );
	if( m_nCaretPosX == nX && m_nCaretPosY == nY ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	SetSelectArea( nY, nX, m_nCaretPosY, m_nCaretPosX );
//	/* �I��̈�`�� */
//	DrawSelectArea();
	/*�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
	Command_CUT();
	return;
}




//�s���܂Ő؂���(���s�P��)
void CEditView::Command_LineCutToEnd( void )
{
	int			nX;
	int			nY;
	CLayout*	pCLayout;
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
		Command_CUT();
		return;
	}
	pCLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	if( EOL_NONE == pCLayout->m_pCDocLine->m_cEol ){	/* ���s�R�[�h�̎�� */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( pCLayout->m_pCDocLine->m_pLine->GetLength() , pCLayout->m_nLinePhysical, &nX, &nY );
	}else{
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( pCLayout->m_pCDocLine->m_pLine->GetLength() - 1, pCLayout->m_nLinePhysical, &nX, &nY );
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( pCLayout->m_pCDocLine->m_pLine->GetLength() - pCLayout->m_pCDocLine->m_cEol.GetLen(), pCLayout->m_nLinePhysical, &nX, &nY );
	}
	if( ( m_nCaretPosX == nX && m_nCaretPosY == nY )
	 || ( m_nCaretPosX >  nX && m_nCaretPosY == nY )
	){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	SetSelectArea( m_nCaretPosY, m_nCaretPosX, nY, nX );
//	/* �I��̈�`�� */
//	DrawSelectArea();
	/*�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜) */
	Command_CUT();
	return;
}




//�s���܂ō폜(���s�P��)
void CEditView::Command_LineDeleteToStart( void )
{
	int			nX;
	int			nY;
	CLayout*	pCLayout;
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		DeleteData( TRUE );
		return;
	}
	pCLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( 0, pCLayout->m_nLinePhysical, &nX, &nY );
	if( m_nCaretPosX == nX && m_nCaretPosY == nY ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	SetSelectArea( nY, nX, m_nCaretPosY, m_nCaretPosX );
//	/* �I��̈�`�� */
//	DrawSelectArea();
	/* �I��̈�폜 */
	DeleteData( TRUE );
	return;
}




//�s���܂ō폜(���s�P��)
void CEditView::Command_LineDeleteToEnd( void )
{
	int			nX;
	int			nY;
	CLayout*	pCLayout;
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		DeleteData( TRUE );
		return;
	}
	pCLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );	/* �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ� */
	if( NULL == pCLayout ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	if( EOL_NONE == pCLayout->m_pCDocLine->m_cEol ){	/* ���s�R�[�h�̎�� */
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( pCLayout->m_pCDocLine->m_pLine->GetLength() , pCLayout->m_nLinePhysical, &nX, &nY );
	}else{
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( pCLayout->m_pCDocLine->m_pLine->GetLength() - 1, pCLayout->m_nLinePhysical, &nX, &nY );
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( pCLayout->m_pCDocLine->m_pLine->GetLength() - pCLayout->m_pCDocLine->m_cEol.GetLen(), pCLayout->m_nLinePhysical, &nX, &nY );
	}
	if( ( m_nCaretPosX == nX && m_nCaretPosY == nY )
	 || ( m_nCaretPosX >  nX && m_nCaretPosY == nY )
	){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �I��͈͂̕ύX */
	//	2005.06.24 Moca
	SetSelectArea( m_nCaretPosY, m_nCaretPosX, nY, nX );
//	/* �I��̈�`�� */
//	DrawSelectArea();
	/* �I��̈�폜 */
	DeleteData( TRUE );
	return;
}




//�s�؂���(�܂�Ԃ��P��)
void CEditView::Command_CUT_LINE( void )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}

	int				nCaretPosX_OLD;
	int				nCaretPosY_OLD;
	const CLayout*	pcLayout;
	COpe*			pcOpe = NULL;
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		::MessageBeep( MB_ICONHAND );
		return;
	}
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
	if( NULL == pcLayout ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	m_nSelectLineFrom = m_nCaretPosY;		/* �͈͑I���J�n�s */
	m_nSelectColmFrom = m_nCaretPosX; 		/* �͈͑I���J�n�� */
	m_nSelectLineTo = m_nCaretPosY;			/* �͈͑I���I���s */
	m_nSelectColmTo = m_nCaretPosX + 1;		/* �͈͑I���I���� */
	nCaretPosX_OLD = m_nCaretPosX;
	nCaretPosY_OLD = m_nCaretPosY;
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	Command_COPYLINES();
	Command_DELETE();
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
	if( NULL != pcLayout ){
		// 2003-04-30 �����
		// �s�폜������A�t���[�J�[�\���łȂ��̂ɃJ�[�\���ʒu���s�[���E�ɂȂ�s��Ή�
		// �t���[�J�[�\�����[�h�łȂ��ꍇ�́A�J�[�\���ʒu�𒲐�����
		if( !m_pShareData->m_Common.m_bIsFreeCursorMode ) {
			int nIndex;
			nIndex = LineColmnToIndex2( pcLayout, nCaretPosX_OLD, nCaretPosX_OLD );
			if (nCaretPosX_OLD > 0) {
				nCaretPosX_OLD--;
			} else {
				nCaretPosX_OLD = LineIndexToColmn( pcLayout, nIndex );
			}
		}
		/* ����O�̈ʒu�փJ�[�\�����ړ� */
		MoveCursor( nCaretPosX_OLD, nCaretPosY_OLD, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;					/* ������ */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */

			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}
	return;
}




/* �s�폜(�܂�Ԃ��P��) */
void CEditView::Command_DELETE_LINE( void )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}

	int				nCaretPosX_OLD;
	int				nCaretPosY_OLD;
	COpe*			pcOpe = NULL;
	const CLayout*	pcLayout;
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		::MessageBeep( MB_ICONHAND );
		return;
	}
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
	if( NULL == pcLayout ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	m_nSelectLineFrom = m_nCaretPosY;		/* �͈͑I���J�n�s */
	m_nSelectColmFrom = 0; 					/* �͈͑I���J�n�� */
	m_nSelectLineTo = m_nCaretPosY + 1;		/* �͈͑I���I���s */
	m_nSelectColmTo = 0;					/* �͈͑I���I���� */

	nCaretPosX_OLD = m_nCaretPosX;
	nCaretPosY_OLD = m_nCaretPosY;

	Command_DELETE();
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
	if( NULL != pcLayout ){
		// 2003-04-30 �����
		// �s�폜������A�t���[�J�[�\���łȂ��̂ɃJ�[�\���ʒu���s�[���E�ɂȂ�s��Ή�
		// �t���[�J�[�\�����[�h�łȂ��ꍇ�́A�J�[�\���ʒu�𒲐�����
		if( !m_pShareData->m_Common.m_bIsFreeCursorMode ) {
			int nIndex;
			nIndex = LineColmnToIndex2( pcLayout, nCaretPosX_OLD, nCaretPosX_OLD );
			if (nCaretPosX_OLD > 0) {
				nCaretPosX_OLD--;
			} else {
				nCaretPosX_OLD = LineIndexToColmn( pcLayout, nIndex );
			}
		}
		/* ����O�̈ʒu�փJ�[�\�����ړ� */
		MoveCursor( nCaretPosX_OLD, nCaretPosY_OLD, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;					/* ������ */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */

			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}
	return;
}




/* ���ׂđI�� */
void CEditView::Command_SELECTALL( void )
{
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( TRUE );
	}
	/* �擪�փJ�[�\�����ړ� */
	//	Sep. 8, 2000 genta
	AddCurrentLineToHistory();
	// MoveCursor( 0, 0, TRUE );
	m_nCaretPosX_Prev = m_nCaretPosX;

	/* �S�̂�I������ */
	//	2005.06.24 Moca
	//SetSelectArea( 0, 0, m_pcEditDoc->m_cLayoutMgr.GetLineCount(), 0 );
	//	Jul. 29, 2006 genta �I���ʒu�̖����𐳊m�Ɏ擾����
	//	�}�N������擾�����ꍇ�ɐ������͈͂��擾�ł��Ȃ�����
	int nX, nY;
	m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos(nX, nY);
	SetSelectArea( 0, 0, nY, nX );

	/* �I��̈�`�� */
	DrawSelectArea();
	return;
}




/* ���݈ʒu�̒P��I�� */
bool CEditView::Command_SELECTWORD( void )
{
	int				nLineFrom;
	int				nColmFrom;
	int				nLineTo;
	int				nColmTo;
	int				nIdx;
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( TRUE );
	}
	const CLayout*	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
	if( NULL == pcLayout ){
		return false;	//	�P��I���Ɏ��s
	}
	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
	nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );

	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		m_nCaretPosY, nIdx,
		&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL ) ){

		/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineFrom );
		nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineTo );
		nColmTo = LineIndexToColmn( pcLayout, nColmTo );

		/* �I��͈͂̕ύX */
		//	2005.06.24 Moca
		SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );

		/* �P��̐擪�ɃJ�[�\�����ړ� */
		MoveCursor( nColmTo, nLineTo, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;

		/* �I��̈�`�� */
		DrawSelectArea();
		return true;	//	�P��I���ɐ����B
	}
	else {
		return false;	//	�P��I���Ɏ��s
	}
}




/* �\��t��(�N���b�v�{�[�h����\��t��) */
void CEditView::Command_PASTE( void )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}


//	HGLOBAL		hglb;
	char*		pszText;
	int			nNewLine;		/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int			nNewPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*		pcOpe = NULL;
	CWaitCursor cWaitCursor( m_hWnd );
	BOOL		bBox;
	char		szPaste[1024];
	int			i;
	int			nTextLen;

	/* �N���b�v�{�[�h����f�[�^���擾 */
	CMemory		cmemClip;
	BOOL		bColmnSelect;
	if( FALSE == MyGetClipboardData( cmemClip, &bColmnSelect ) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}

	/* ��`�R�s�[�̃e�L�X�g�͏�ɋ�`�\��t�� */
	if( m_pShareData->m_Common.m_bAutoColmnPaste ){
		/* ��`�R�s�[�̃f�[�^�Ȃ��`�\��t�� */
		if( bColmnSelect ){
			Command_PASTEBOX();
			return;
		}
	}
	pszText = cmemClip.GetPtr( &nTextLen );

	/* �e�L�X�g���I������Ă��邩 */
	bBox = FALSE;
	if( IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_bBeginBoxSelect ){
			bBox = TRUE;
		}else{
//			DeleteData( TRUE );
			//	Jun. 23, 2000 genta
			//	����s�̍s���ȍ~�݂̂��I������Ă���ꍇ�ɂ͑I�𖳂��ƌ��Ȃ�
			int			len, pos;
			const char	*line;
			const CLayout* pcLayout;
			line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineFrom, &len, &pcLayout );

			pos = ( line == NULL ) ? 0 : LineColmnToIndex( pcLayout, m_nSelectColmFrom );
			if( pos >= len &&	//	�J�n�ʒu���s��������
				m_nSelectLineFrom == m_nSelectLineTo	//	�I���ʒu������s
				){
				m_nCaretPosX = m_nSelectColmFrom;
				DisableSelectArea(false);
			}
			else{

				/* �f�[�^�u�� �폜&�}���ɂ��g���� */
				ReplaceData_CEditView(
					m_nSelectLineFrom,		/* �͈͑I���J�n�s */
					m_nSelectColmFrom,		/* �͈͑I���J�n�� */
					m_nSelectLineTo,		/* �͈͑I���I���s */
					m_nSelectColmTo,		/* �͈͑I���I���� */
					NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
					pszText,				/* �}������f�[�^ */
					nTextLen,				/* �}������f�[�^�̒��� */
					TRUE
				);
#ifdef _DEBUG
					gm_ProfileOutput = FALSE;
#endif
				return;
			}
		}
	}
	if( bBox ){
		for( i = 0; i < (int)nTextLen/*lstrlen( pszText )*/; i++  ){
			if( pszText[i] == CR || pszText[i] == LF ){
				break;
			}
		}
		memcpy( szPaste, pszText, i );
		szPaste[i] = '\0';
//		Command_INDENT( szPaste, lstrlen( szPaste ) );
		Command_INDENT( szPaste, i );
	}else{
		m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta
		if( !m_bDoing_UndoRedo ){						/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
//			pcOpe->m_nCaretPosX_Before = m_nCaretPosX;	/* ����O�̃L�����b�g�ʒu�w */
//			pcOpe->m_nCaretPosY_Before = m_nCaretPosY;	/* ����O�̃L�����b�g�ʒu�x */
//			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//				pcOpe->m_nCaretPosX_Before,
//				pcOpe->m_nCaretPosY_Before,
//				&pcOpe->m_nCaretPosX_PHY_Before,
//				&pcOpe->m_nCaretPosY_PHY_Before
//			);
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */
		}
		/* ���݈ʒu�Ƀf�[�^��}�� */
		InsertData_CEditView( m_nCaretPosX, m_nCaretPosY, pszText, nTextLen/*lstrlen(pszText)*/, &nNewLine, &nNewPos, pcOpe, TRUE );
//		::GlobalUnlock(hglb);
		/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
		MoveCursor( nNewPos, nNewLine, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){								/* �A���h�D�E���h�D�̎��s���� */
//			pcOpe->m_nCaretPosX_After = m_nCaretPosX;			/* �����̃L�����b�g�ʒu�w */
//			pcOpe->m_nCaretPosY_After = m_nCaretPosY;			/* �����̃L�����b�g�ʒu�x */
//			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//				pcOpe->m_nCaretPosX_After,
//				pcOpe->m_nCaretPosY_After,
//				&pcOpe->m_nCaretPosX_PHY_After,
//				&pcOpe->m_nCaretPosY_PHY_After
//			);
			pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}
	return;

}




///* �e�L�X�g��\��t�� ver0 */
//void CEditView::Command_INSTEXT( BOOL bRedraw, const char* pData, int nDataLen )
//{
//	char*	pszText;
//	pszText = new char[ nDataLen + 1];
//	memcpy( pszText, pData, nDataLen );
//	pszText[nDataLen] = '\0';
//	Command_INSTEXT( bRedraw, pszText );
//	delete [] pszText;
//	return;
//}




/*! �e�L�X�g��\��t��
	@param pszText  [in] �\��t���镶����B
	@param nTextLen [in] pszText�̒����B-1���w�肷��ƁApszText��NUL�I�[������Ƃ݂Ȃ��Ē����������v�Z����
	@date 2004.05.14 Moca '\\0'���󂯓����悤�ɁA�����ɒ�����ǉ�
*/
void CEditView::Command_INSTEXT( BOOL bRedraw, const char* pszText, int nTextLen, BOOL bNoWaitCursor )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}

//	HGLOBAL			hglb;
//	char*			lptstr;
	int				nNewLine;			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int				nNewPos;			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*			pcOpe = NULL;
	CWaitCursor*	pcWaitCursor;
	int				i;

	m_pcEditDoc->SetModified(true,bRedraw);	//	Jan. 22, 2002 genta
	if( bNoWaitCursor ){
		pcWaitCursor = NULL;
	}else{
		pcWaitCursor = new CWaitCursor( m_hWnd );
	}

	if( nTextLen < 0 ){
		nTextLen = lstrlen( pszText );
	}

////////////////////�f�o�b�O�p�e�X�g����������
//#ifdef _DEBUG
//	if( IsTextSelected()
//	 && FALSE == m_bBeginBoxSelect	/* ��`�͈͑I�𒆂ł͂Ȃ� */
//	){
//		/* �f�[�^�u�� �폜&�}���ɂ��g���� */
//		ReplaceData_CEditView(
//			pszText,				/* �}������f�[�^ */
//			strlen( pszText ),		/* �}������f�[�^�̒��� */
//			bRedraw,
//		);
//		return;
//	}
//#endif _DEBUG
////////////////////���������������f�o�b�O�p�e�X�g


	/* �e�L�X�g���I������Ă��邩 */
	if( IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_bBeginBoxSelect ){
			// i = strcspn(pszText, CRLF);
			// 2004.05.14 Moca strcspn�ł�'\0'�������Ȃ��̂�
			for( i = 0; i < nTextLen; i++ ){
				if( pszText[i] == CR 
				 || pszText[i] == LF ){
					break;
				}
			}
			Command_INDENT( pszText, i );
		}
		else{
			/* �f�[�^�u�� �폜&�}���ɂ��g���� */
			ReplaceData_CEditView(
				m_nSelectLineFrom,		/* �͈͑I���J�n�s */
				m_nSelectColmFrom,		/* �͈͑I���J�n�� */
				m_nSelectLineTo,		/* �͈͑I���I���s */
				m_nSelectColmTo,		/* �͈͑I���I���� */
				NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
				pszText,				/* �}������f�[�^ */
				nTextLen,		/* �}������f�[�^�̒��� */
				bRedraw
			);
#ifdef _DEBUG
				gm_ProfileOutput = FALSE;
#endif
		}
	}
	else
	{
		m_pcEditDoc->SetModified(true,true);	/* �ύX�t���O */
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */
		}
		
		//	Jun. 13, 2004 genta �s�v�ȃ`�F�b�N�H
		if( nTextLen < 0 ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONWARNING, GSTR_APPNAME,
				_T("�o�O����Ȃ��́H @Command_INSTEXT") );
			nTextLen = lstrlen( pszText );
		}
		/* ���݈ʒu�Ƀf�[�^��}�� */
		InsertData_CEditView( m_nCaretPosX, m_nCaretPosY, pszText, nTextLen, &nNewLine, &nNewPos, pcOpe, TRUE );
		/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
		MoveCursor( nNewPos, nNewLine, bRedraw/*TRUE 2002.02.16 hor */ );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}

	if( NULL != pcWaitCursor ){
		delete pcWaitCursor;
	}
	return;
}

//<< 2002/03/28 Azumaiya
// �������f�[�^����`�\��t���p�̃f�[�^�Ɖ��߂��ď�������B
//  �Ȃ��A���̊֐��� Command_PASTEBOX(void) �ƁA
// 2769 : m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta
// ����A
// 3057 : m_bDrawSWITCH=TRUE;	// 2002.01.25 hor
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
void CEditView::Command_PASTEBOX( const char *szPaste, int nPasteSize )
{
	/* �����̓���͎c���Ă��������̂����A�Ăяo�����ŐӔC�������Ă���Ă��炤���ƂɕύX�B
	if( m_bBeginSelect )	// �}�E�X�ɂ��͈͑I��
	{
		::MessageBeep( MB_ICONHAND );
		return;
	}


	if( FALSE == m_pShareData->m_Common.m_bFontIs_FIXED_PITCH )	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	{
		return;
	}
	*/

	int				nBgn;
	int				nPos;
	int				nCount;
	int				nNewLine;		/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int				nNewPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	int				nCurXOld;
	int				nCurYOld;
	COpe*			pcOpe = NULL;
	BOOL			bAddLastCR;
	int				nInsPosX;

	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

	m_bDrawSWITCH = FALSE;	// 2002.01.25 hor

	// �Ƃ肠�����I��͈͂��폜
	// 2004.06.30 Moca IsTextSelected()���Ȃ��Ɩ��I�����A�ꕶ�������Ă��܂�
	if( IsTextSelected() ){
		DeleteData( FALSE/*TRUE 2002.01.25 hor*/ );
	}

	nCurXOld = m_nCaretPosX;
	nCurYOld = m_nCaretPosY;

	nCount = 0;

	// Jul. 10, 2005 genta �\��t���f�[�^�̍Ō��CR/LF�������ꍇ�̑΍�
	//	�f�[�^�̍Ō�܂ŏ��� i.e. nBgn��nPasteSize�𒴂�����I��
	//for( nPos = 0; nPos < nPasteSize; )
	for( nBgn = nPos = 0; nBgn < nPasteSize; )
	{
		// Jul. 10, 2005 genta �\��t���f�[�^�̍Ō��CR/LF��������
		//	�ŏI�s��Paste�����������Ȃ��̂ŁC
		//	�f�[�^�̖����ɗ����ꍇ�͋����I�ɏ�������悤�ɂ���
		if( szPaste[nPos] == CR || szPaste[nPos] == LF ||
			nPos == nPasteSize )
		{
			if( !m_bDoing_UndoRedo )	/* �A���h�D�E���h�D�̎��s���� */
			{
				pcOpe = new COpe;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
												nCurXOld,
												nCurYOld + nCount,
												&pcOpe->m_nCaretPosX_PHY_Before,
												&pcOpe->m_nCaretPosY_PHY_Before
												);
			}

			/* ���݈ʒu�Ƀf�[�^��}�� */
			if( nPos - nBgn > 0 ){
				InsertData_CEditView(
								nCurXOld,
								nCurYOld + nCount,
								&szPaste[nBgn],
								nPos - nBgn,
								&nNewLine,
								&nNewPos,
								pcOpe,
								FALSE/*TRUE 2002.01.25 hor*/
							);
			}

			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
												nNewPos,
												nNewLine,
												&pcOpe->m_nCaretPosX_PHY_After,
												&pcOpe->m_nCaretPosY_PHY_After
												);

				/* ����̒ǉ� */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}

			/* ���̍s�̑}���ʒu�փJ�[�\�����ړ� */
			MoveCursor( nCurXOld, nCurYOld + nCount, FALSE/*TRUE 2002.01.25 hor*/ );
			m_nCaretPosX_Prev = m_nCaretPosX;
			/* �J�[�\���s���Ō�̍s���s���ɉ��s�������A�}�����ׂ��f�[�^���܂�����ꍇ */
			bAddLastCR = FALSE;
			const CLayout*	pcLayout;
			int				nLineLen = 0;
			const char*		pLine;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );

			if( NULL != pLine && 1 <= nLineLen )
			{
				if( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF )
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
//				MYTRACE( " �J�[�\���s���Ō�̍s���s���ɉ��s�������A\n�}�����ׂ��f�[�^���܂�����ꍇ�͍s���ɉ��s��}���B\n" );
				nInsPosX = LineIndexToColmn( pcLayout, nLineLen );
				if( !m_bDoing_UndoRedo )	/* �A���h�D�E���h�D�̎��s���� */
				{
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
														nInsPosX,
														m_nCaretPosY,
														&pcOpe->m_nCaretPosX_PHY_Before,
														&pcOpe->m_nCaretPosY_PHY_Before
														);
				}

				InsertData_CEditView(
								nInsPosX,
								m_nCaretPosY,
								m_pcEditDoc->GetNewLineCode().GetValue(),
								m_pcEditDoc->GetNewLineCode().GetLen(),
								&nNewLine,
								&nNewPos,
								pcOpe,
								FALSE/*TRUE 2002.01.25 hor*/
								);

				if( !m_bDoing_UndoRedo )	/* �A���h�D�E���h�D�̎��s���� */
				{
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
													nNewPos,
													nNewLine,
													&pcOpe->m_nCaretPosX_PHY_After,
													&pcOpe->m_nCaretPosY_PHY_After
													);

					/* ����̒ǉ� */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}
			}

			if(
				(nPos + 1 < nPasteSize ) &&
				(
// 2004.06.30 Moca WORD*�ł͔�x86�ŋ��E�s����̉\������
				 ( szPaste[nPos] == '\n' && szPaste[nPos + 1] == '\r') ||
				 ( szPaste[nPos] == '\r' && szPaste[nPos + 1] == '\n')
//				 ((WORD *)(szPaste + nPos))[0] == MAKEWORD('\n', '\r') ||
//				 ((WORD *)(szPaste + nPos))[0] == MAKEWORD('\r', '\n')
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
	MoveCursor( nCurXOld, nCurYOld, TRUE );
	m_nCaretPosX_Prev = m_nCaretPosX;

	if( !m_bDoing_UndoRedo )	/* �A���h�D�E���h�D�̎��s���� */
	{
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;						/* ������ */
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */

		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	m_bDrawSWITCH = TRUE;	// 2002.01.25 hor
	return;
}

// �ꉞ�ACommand_PASTEBOX(char *, int) ���g���� Command_PASTEBOX(void) �������Ă����܂��B
/* ��`�\��t��(�N���b�v�{�[�h�����`�\��t��) */
// 2004.06.29 Moca ���g�p���������̂�L���ɂ���
//	�I���W�i����Command_PASTEBOX(void)�͂΂�����폜 (genta)
void CEditView::Command_PASTEBOX( void )
{
	if( m_bBeginSelect )	// �}�E�X�ɂ��͈͑I��
	{
		::MessageBeep( MB_ICONHAND );
		return;
	}


	if( FALSE == m_pShareData->m_Common.m_bFontIs_FIXED_PITCH )	// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
	{
		return;
	}

	// �N���b�v�{�[�h����f�[�^���擾
	CMemory			cmemClip;
	if( FALSE == MyGetClipboardData( cmemClip, NULL ) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	// 2004.07.13 Moca \0�R�s�[�΍�
	int nstrlen;
	const char *lptstr = cmemClip.GetPtr( &nstrlen );

	Command_PASTEBOX(lptstr, nstrlen);

	Redraw();			// 2002.01.25 hor
	return;
}

//>> 2002/03/29 Azumaiya

/* �P�o�C�g�������� */
void CEditView::Command_CHAR( char cChar )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}


	CMemory			cmemData;
	CMemory			cmemIndent;
	int				nPos;
	int				nCharChars;
	int				nIdxTo;
	int				nPosX;
	int				nNewLine;	/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int				nNewPos;	/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*			pcOpe = NULL;
	char			szCurrent[10];
	const CLayout*	pcLayout;

	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

	/* ���݈ʒu�Ƀf�[�^��}�� */
	nPosX = -1;
	cmemData = cChar;
	if( cChar == CR ||
		cChar == LF ){
		/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */
		// enumEOLType nWorkEOL;
		CEOL cWork = m_pcEditDoc->GetNewLineCode();
		cmemData.SetData( cWork.GetValue(), cWork.GetLen() );

		/* �e�L�X�g���I������Ă��邩 */
		if( IsTextSelected() ){
			DeleteData( TRUE );
		}
		if( m_pcEditDoc->GetDocumentAttribute().m_bAutoIndent ){	/* �I�[�g�C���f���g */
			const CLayout* pCLayout;
			const char*		pLine;
			int				nLineLen;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pCLayout );
			if( NULL != pCLayout ){
				const CDocLine* pcDocLine;
				pcDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo( pCLayout->m_nLinePhysical );
				pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( pCLayout->m_nLinePhysical, &nLineLen );
				if( NULL != pLine ){
					/*
					  �J�[�\���ʒu�ϊ�
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					  ��
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					*/
					int		nX;
					int		nY;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						m_nCaretPosX,
						m_nCaretPosY,
						&nX,
						&nY
					);

					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					nIdxTo = LineColmnToIndex( pcDocLine, m_nCaretPosX );
					for( nPos = 0; nPos < /*nIdxTo*/nLineLen && nPos < nX; ){
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );

						/* ���̑��̃C���f���g���� */
						if( 0 < nCharChars
						 && 0 < (int)lstrlen( m_pcEditDoc->GetDocumentAttribute().m_szIndentChars )
						){
							memcpy( szCurrent, &pLine[nPos], nCharChars );
							szCurrent[nCharChars] = '\0';
							/* ���̑��̃C���f���g�Ώە��� */
							if( NULL != _mbsstr(
								(const unsigned char*)m_pcEditDoc->GetDocumentAttribute().m_szIndentChars,
								(const unsigned char*)szCurrent
							) ){
								goto end_of_for;
							}
						}
						if( nCharChars == 1 ){
							if( pLine[nPos] == SPACE ||
								pLine[nPos] == TAB ){
							}else{
								break;
							}
						}else
						if( nCharChars == 2 ){
							if( m_pcEditDoc->GetDocumentAttribute().m_bAutoIndent_ZENSPACE ){	/* ���{��󔒂��C���f���g */
								if( pLine[nPos    ] == (char)0x81 &&
									pLine[nPos + 1] == (char)0x40 ){
								}else{
									break;
								}
							}else{
								break;
							}
						}else
						if( nCharChars == 0 ){
							break;
						}else{
							break;
						}
						end_of_for:;
						nPos += nCharChars;
					}
					if( nPos > 0 ){
						nPosX = LineIndexToColmn( pcDocLine, nPos );
					}
					cmemIndent.SetData( pLine, nPos );
					cmemData += cmemIndent;
				}
			}
		}
	}else{
		/* �e�L�X�g���I������Ă��邩 */
		if( IsTextSelected() ){
			/* ��`�͈͑I�𒆂� */
			if( m_bBeginBoxSelect ){
				Command_INDENT( cChar );
				return;
			}else{
				DeleteData( TRUE );
			}
		}else{
			if( ! IsInsMode() /* Oct. 2, 2005 genta */ ){
				BOOL bDelete = TRUE;
				if( m_pShareData->m_Common.m_bNotOverWriteCRLF ){	/* ���s�͏㏑�����Ȃ� */
					pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
					if( NULL != pcLayout ){
						/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
						nIdxTo = LineColmnToIndex( pcLayout, m_nCaretPosX );
						if( nIdxTo == pcLayout->GetLengthWithoutEOL() ){

							/* ���݈ʒu�����s�Ȃ�΍폜���Ȃ� */
							bDelete = FALSE;
						}
					}
				}
				if( bDelete ){
					/* �㏑�����[�h�Ȃ̂ŁA���݈ʒu�̕������P�������� */
					DeleteData( FALSE );
				}
			}
		}
	}
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
//		pcOpe->m_nCaretPosX_Before = m_nCaretPosX;			/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = m_nCaretPosY;			/* ����O�̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_Before,
//			pcOpe->m_nCaretPosY_Before,
//			&pcOpe->m_nCaretPosX_PHY_Before,
//			&pcOpe->m_nCaretPosY_PHY_Before
//		);
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */
	}


	InsertData_CEditView(
		m_nCaretPosX,
		m_nCaretPosY,
		cmemData.GetPtr(),
		cmemData.GetLength(),
		&nNewLine,
		&nNewPos,
		pcOpe,
		TRUE
	);
	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
	MoveCursor( nNewPos, nNewLine, TRUE );
	m_nCaretPosX_Prev = m_nCaretPosX;
//?	if( nPosX != -1 ){
//?		MoveCursor( nPosX, m_nCaretPosY, TRUE );
//?		m_nCaretPosX_Prev = m_nCaretPosX;
//?	}
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//		pcOpe->m_nCaretPosX_After = m_nCaretPosX;	/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = m_nCaretPosY;	/* �����̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_After,
//			pcOpe->m_nCaretPosY_After,
//			&pcOpe->m_nCaretPosX_PHY_After,
//			&pcOpe->m_nCaretPosY_PHY_After
//		);
		pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}


	/* �X�}�[�g�C���f���g */
	if( SMARTINDENT_CPP == m_pcEditDoc->GetDocumentAttribute().m_nSmartIndent ){	/* �X�}�[�g�C���f���g��� */
		/* C/C++�X�}�[�g�C���f���g���� */
		SmartIndent_CPP( cChar );
	}

	/* 2005.10.11 ryoji ���s���ɖ����̋󔒂��폜 */
	if( CR == cChar && TRUE == m_pcEditDoc->GetDocumentAttribute().m_bRTrimPrevLine ){	/* ���s���ɖ����̋󔒂��폜 */
		/* �O�̍s�ɂ��閖���̋󔒂��폜���� */
		RTrimPrevLine();
	}

	PostprocessCommand_hokan();	//	Jan. 10, 2005 genta �֐���
	return;
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
void CEditView::Command_IME_CHAR( WORD wChar )
{
	if( m_bBeginSelect ){	/* �}�E�X�ɂ��͈͑I�� */
		::MessageBeep( MB_ICONHAND );
		return;
	}

	int				nIdxTo;
	CMemory			cmemData;
	int				nNewLine;		/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int				nNewPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*			pcOpe = NULL;
	char	sWord[2];
//	const CLayout*	pcLayout;
	//	Oct. 6 ,2002 genta �㉺�t�]
	if( 0 == (wChar & 0xff00) ){
		Command_CHAR( wChar & 0xff );
		return;
	}
	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

	//	Oct. 6 ,2002 genta �o�b�t�@�Ɋi�[����
	sWord[0] = (wChar >> 8) & 0xff;
	sWord[1] = wChar & 0xff;

	/* �e�L�X�g���I������Ă��邩 */
	if( IsTextSelected() ){
		/* ��`�͈͑I�𒆂� */
		if( m_bBeginBoxSelect ){
			Command_INDENT( &sWord[0], 2 );	//	Oct. 6 ,2002 genta 
			return;
		}else{
			DeleteData( TRUE );
		}
	}else{
		if( ! IsInsMode() /* Oct. 2, 2005 genta */ ){
			BOOL bDelete = TRUE;
			if( m_pShareData->m_Common.m_bNotOverWriteCRLF ){	/* ���s�͏㏑�����Ȃ� */
				const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
				if( NULL != pcLayout ){
					/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
					nIdxTo = LineColmnToIndex( pcLayout, m_nCaretPosX );
					if( nIdxTo == pcLayout->GetLengthWithoutEOL() ){
						/* ���݈ʒu�����s�Ȃ�΍폜���Ȃ� */
						bDelete = FALSE;
					}
				}
			}
			if( bDelete ){
				/* �㏑�����[�h�Ȃ̂ŁA���݈ʒu�̕������P�������� */
				DeleteData( FALSE );
			}
		}
	}
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
//		pcOpe->m_nCaretPosX_Before = m_nCaretPosX;			/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = m_nCaretPosY;			/* ����O�̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_Before,
//			pcOpe->m_nCaretPosY_Before,
//			&pcOpe->m_nCaretPosX_PHY_Before,
//			&pcOpe->m_nCaretPosY_PHY_Before
//		);
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */
	}
	//	Oct. 6 ,2002 genta 
	InsertData_CEditView( m_nCaretPosX, m_nCaretPosY, &sWord[0], 2, &nNewLine, &nNewPos, pcOpe, TRUE );

	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
	MoveCursor( nNewPos, nNewLine, TRUE );
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//		pcOpe->m_nCaretPosX_After = m_nCaretPosX;			/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = m_nCaretPosY;			/* �����̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_After,
//			pcOpe->m_nCaretPosY_After,
//			&pcOpe->m_nCaretPosX_PHY_After,
//			&pcOpe->m_nCaretPosY_PHY_After
//		);
		pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	PostprocessCommand_hokan();	//	Jan. 10, 2005 genta �֐���
	return;
}




/*! �}���^�㏑�����[�h�؂�ւ�

	@date 2005.10.02 genta InsMode�֐���
*/
void CEditView::Command_CHGMOD_INS( void )
{
	/* �}�����[�h���H */
	if( IsInsMode() ){
		SetInsMode( false );
	}else{
		SetInsMode( true );
	}
	/* �L�����b�g�̕\���E�X�V */
	ShowEditCaret();
	/* �L�����b�g�̍s���ʒu��\������ */
	DrawCaretPosInfo();
	return;
}


/*!
����(�{�b�N�X)�R�}���h���s.
�c�[���o�[�̌����{�b�N�X�Ƀt�H�[�J�X���ړ�����.
	@date 2006.06.04 yukihane �V�K�쐬
*/
void CEditView::Command_SEARCH_BOX( void )
{
	const CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;
	pCEditWnd->SetFocusSearchBox();
}

/* ����(�P�ꌟ���_�C�A���O) */
void CEditView::Command_SEARCH_DIALOG( void )
{
//	int			nRet;
	CMemory		cmemCurText;

	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	GetCurrentTextForSearch( cmemCurText );

	/* ����������������� */
	strcpy( m_pcEditDoc->m_cDlgFind.m_szText, cmemCurText.GetPtr() );

	/* �����_�C�A���O�̕\�� */
	if( NULL == m_pcEditDoc->m_cDlgFind.m_hWnd ){
		m_pcEditDoc->m_cDlgFind.DoModeless( m_hInstance, m_hWnd, (LPARAM)&m_pcEditDoc->m_cEditViewArr[m_pcEditDoc->m_nActivePaneIndex] );
	}else{
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( m_pcEditDoc->m_cDlgFind.m_hWnd );
		::SetDlgItemText( m_pcEditDoc->m_cDlgFind.m_hWnd, IDC_COMBO_TEXT, cmemCurText.GetPtr() );
	}
	return;
}


/* ���K�\���̌����p�^�[����K�v�ɉ����čX�V����(���C�u�������g�p�ł��Ȃ��Ƃ���FALSE��Ԃ�) */
/* 2002.01.16 hor ���ʃ��W�b�N���֐��ɂ��������E�E�E */
BOOL CEditView::ChangeCurRegexp(void)
{
	BOOL	bChangeState;
	if( FALSE == m_bCurSrchKeyMark
	 || 0 != strcmp( m_szCurSrchKey, m_pShareData->m_szSEARCHKEYArr[0] )
	 || m_bCurSrchRegularExp != m_pShareData->m_Common.m_bRegularExp
	 || m_bCurSrchLoHiCase != m_pShareData->m_Common.m_bLoHiCase
	 || m_bCurSrchWordOnly != m_pShareData->m_Common.m_bWordOnly
	){
		bChangeState = TRUE;
	}else{
		bChangeState = FALSE;
	}

	m_bCurSrchKeyMark = TRUE;									/* ����������̃}�[�N */
	strcpy( m_szCurSrchKey, m_pShareData->m_szSEARCHKEYArr[0] );/* ���������� */
	m_bCurSrchRegularExp = m_pShareData->m_Common.m_bRegularExp;/* �����^�u��  1==���K�\�� */
	m_bCurSrchLoHiCase = m_pShareData->m_Common.m_bLoHiCase;	/* �����^�u��  1==�啶���������̋�� */
	m_bCurSrchWordOnly = m_pShareData->m_Common.m_bWordOnly;	/* �����^�u��  1==�P��̂݌��� */
	/* ���K�\�� */
	if( m_bCurSrchRegularExp
	 && bChangeState
	){
		//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
		if( !InitRegexp( m_hWnd, m_CurRegexp, true ) ){
			return FALSE;
		}
		int nFlag = 0x00;
		nFlag |= m_bCurSrchLoHiCase ? 0x01 : 0x00;
		/* �����p�^�[���̃R���p�C�� */
		m_CurRegexp.Compile( m_szCurSrchKey, nFlag );
	}

	if( bChangeState ){
		/* �t�H�[�J�X�ړ����̍ĕ`�� */
		RedrawAll();
	}

	return TRUE;
}


/* �O������ */
void CEditView::Command_SEARCH_PREV( BOOL bReDraw, HWND hwndParent )
{
	int			nLineNum;
	int			nIdx;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	BOOL		bSelecting;
//	int			nSelectLineBgn_Old;
//	int			nSelectColBgn_Old;
	int			nSelectLineBgnFrom_Old;
	int			nSelectColBgnFrom_Old;
	int			nSelectLineBgnTo_Old;
	int			nSelectColBgnTo_Old;
	int			nSelectLineFrom_Old;
	int			nSelectColFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColTo_Old;
	BOOL		bSelectingLock_Old;
//	BOOL		bFlag1;
	BOOL		bFound = FALSE;
	BOOL		bRedo = FALSE;			//	hor
	int			nLineNumOld,nIdxOld;	//	hor
	const CLayout* pcLayout = NULL;

	nLineFrom = m_nCaretPosY;
	nColmFrom = m_nCaretPosX;
	nLineTo = m_nCaretPosY;
	nColmTo = m_nCaretPosX;
//	bFlag1 = FALSE;
	bSelecting = FALSE;
	if( '\0' == m_pShareData->m_szSEARCHKEYArr[0][0] ){
		goto end_of_func;
	}
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		nSelectLineBgnFrom_Old = m_nSelectLineBgnFrom;	/* �͈͑I���J�n�s(���_) */
		nSelectColBgnFrom_Old = m_nSelectColmBgnFrom;	/* �͈͑I���J�n��(���_) */
		nSelectLineBgnTo_Old = m_nSelectLineBgnTo;		/* �͈͑I���J�n�s(���_) */
		nSelectColBgnTo_Old = m_nSelectColmBgnTo;		/* �͈͑I���J�n��(���_) */
		nSelectLineFrom_Old = m_nSelectLineFrom;
		nSelectColFrom_Old = m_nSelectColmFrom;
		nSelectLineTo_Old = m_nSelectLineTo;
		nSelectColTo_Old = m_nSelectColmTo;
		bSelectingLock_Old = m_bSelectingLock;
		/* ��`�͈͑I�𒆂� */
		if( !m_bBeginBoxSelect && TRUE == m_bSelectingLock ){	/* �I����Ԃ̃��b�N */
			bSelecting = TRUE;
//			bSelectingLock_Old = m_bSelectingLock;
//			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
//			DisableSelectArea( bReDraw );

		}else{
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( bReDraw );
		}
	}
	nLineNum = m_nCaretPosY;
	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );
	if( NULL == pcLayout ){
		// pcLayout��NULL�ƂȂ�̂́A[EOF]����O���������ꍇ
		// �P�s�O�Ɉړ����鏈��
		nLineNum--;
		if( nLineNum < 0 ){
			goto end_of_func;
		}
		pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );
		if( NULL == pcLayout ){
			goto end_of_func;
		}
	//		/* �J�[�\�����ړ� */
	//		Command_LEFT( FALSE, FALSE );
			// �J�[�\�����ړ��͂�߂� nIdx�͍s�̒����Ƃ��Ȃ���[EOF]������s��O�����������ɍŌ�̉��s�������ł��Ȃ� 2003.05.04 �����
			CLayout* pCLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );
			nIdx = pCLayout->m_pCDocLine->m_pLine->GetLength() + 1;		// �s���̃k������(\0)�Ƀ}�b�`�����邽�߂�+1 2003.05.16 �����
	} else {
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
		nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );
	}
	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	if(!ChangeCurRegexp())return;

	bRedo		=	TRUE;		//	hor
	nLineNumOld	=	nLineNum;	//	hor
	nIdxOld		=	nIdx;		//	hor
re_do:;							//	hor
	/* ���݈ʒu���O�̈ʒu���������� */
	if( m_pcEditDoc->m_cLayoutMgr.SearchWord(
		nLineNum, 								/* �����J�n�s */
		nIdx, 									/* �����J�n�ʒu */
		m_szCurSrchKey,							/* �������� */
		FALSE,									/* 0==�O������ 1==������� */
		m_bCurSrchRegularExp,					/* 1==���K�\�� */
		m_bCurSrchLoHiCase,						/* 1==�啶���������̋�� */
		m_bCurSrchWordOnly,						/* 1==�P��̂݌��� */
		&nLineFrom,								/* �}�b�`���C�A�E�g�sfrom */
		&nColmFrom, 							/* �}�b�`���C�A�E�g�ʒufrom */
		&nLineTo, 								/* �}�b�`���C�A�E�g�sto */
		&nColmTo, 								/* �}�b�`���C�A�E�g�ʒuto */
		&m_CurRegexp							/* ���K�\���R���p�C���f�[�^ */
	) ){
		if( bSelecting ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
			m_bSelectingLock = bSelectingLock_Old;	/* �I����Ԃ̃��b�N */
		}else{
			/* �I��͈͂̕ύX */
			//	2005.06.24 Moca
			SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );

			if( bReDraw ){
				/* �I��̈�`�� */
				DrawSelectArea();
			}
		}
		/* �J�[�\���ړ� */
		//	Sep. 8, 2000 genta
		AddCurrentLineToHistory();
		MoveCursor( nColmFrom, nLineFrom, bReDraw );
		m_nCaretPosX_Prev = m_nCaretPosX;
		bFound = TRUE;
	}else{
		/* �t�H�[�J�X�ړ����̍ĕ`�� */
//		RedrawAll();	hor �R�����g��
		if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( bReDraw );
		}
		if( bSelecting ){
			m_bSelectingLock = bSelectingLock_Old;	/* �I����Ԃ̃��b�N */
			/* �I��͈͂̕ύX */
			m_nSelectLineBgnFrom = nSelectLineBgnFrom_Old;	/* �͈͑I���J�n�s(���_) */
			m_nSelectColmBgnFrom = nSelectColBgnFrom_Old;	/* �͈͑I���J�n��(���_) */
			m_nSelectLineBgnTo = nSelectLineBgnTo_Old;		/* �͈͑I���J�n�s(���_) */
			m_nSelectColmBgnTo = nSelectColBgnTo_Old;		/* �͈͑I���J�n��(���_) */

			m_nSelectLineFrom =	nSelectLineFrom_Old;
			m_nSelectColmFrom = nSelectColFrom_Old;
			m_nSelectLineTo = nSelectLineTo_Old;
			m_nSelectColmTo = nSelectColTo_Old;
			/* �J�[�\���ړ� */
			MoveCursor( nColmFrom, nLineFrom, bReDraw );
			m_nCaretPosX_Prev = m_nCaretPosX;
			/* �I��̈�`�� */
			DrawSelectArea();
		}
	}
end_of_func:;
// From Here 2002.01.26 hor �擪�i�����j����Č���
	if(m_pShareData->m_Common.m_bSearchAll){
		if(!bFound	&&	// ������Ȃ�����
			bRedo		// �ŏ��̌���
		){
			nLineNum=m_pcEditDoc->m_cLayoutMgr.GetLineCount()-1;
			nIdx=MAXLINESIZE;
			bRedo=FALSE;
			goto re_do;	// ��������Č���
		}
	}
	if(bFound){
		if((nLineNumOld < nLineNum)||(nLineNumOld == nLineNum && nIdxOld < nIdx))
			SendStatusMessage("����������Č������܂���");
	}else{
		SendStatusMessage("��������܂���ł���");
//	if( FALSE == bFound ){
// To Here 2002.01.26 hor
		::MessageBeep( MB_ICONHAND );
		if( bReDraw	&&
			m_pShareData->m_Common.m_bNOTIFYNOTFOUND 	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
		){
			if( NULL == hwndParent ){
				hwndParent = m_hWnd;
			}
			::MYMESSAGEBOX( hwndParent,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
//				"�� �O���ɁA������ '%s' ��������܂���B",
				"�O��(��) �ɕ����� '%s' ���P��������܂���B",	//Jan. 25, 2001 jepro ���b�Z�[�W���኱�ύX
				m_szCurSrchKey
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
void CEditView::Command_SEARCH_NEXT( bool bChangeCurRegexp, BOOL bRedraw, HWND hwndParent, const char* pszNotFoundMessage )
{

//#ifdef _DEBUG
//	gm_ProfileOutput = 1;
//	CRunningTimer*  pCRunningTimer = new CRunningTimer( (const char*)"CEditView::Command_SEARCH_NEXT()" );
//#endif
	int			nLineNum;
	int			nIdx;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	BOOL		bSelecting;
//	int			nSelectLineBgn_Old;
//	int			nSelectColBgn_Old;
	int			nSelectLineBgnFrom_Old;
	int			nSelectColBgnFrom_Old;
	int			nSelectLineBgnTo_Old;
	int			nSelectColBgnTo_Old;
	int			nSelectLineFrom_Old;
	int			nSelectColFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColTo_Old;
//	int			nSelectLineFrom;
//	int			nSelectColmFrom;
//	int			nSelectLineTo;
//	int			nSelectColmTo;
	BOOL		bFlag1;
	BOOL		bSelectingLock_Old;
	BOOL		bFound = FALSE;
	BOOL		bRedo = FALSE;			//	hor
	int			nLineNumOld,nIdxOld;	//	hor
	const CLayout* pcLayout;
	bool b0Match = false;		//!< �����O�Ń}�b�`���Ă��邩�H�t���O by �����
	int nLineLen;
	const char *pLine;

	nLineFrom = m_nCaretPosY;
	nColmFrom = m_nCaretPosX;
	nLineTo = m_nCaretPosY;
	nColmTo = m_nCaretPosX;

	bSelecting = FALSE;
	//	2004.05.30 Moca bChangeCurRegexp�ɉ����đΏە������ς���
	if( bChangeCurRegexp  && '\0' == m_pShareData->m_szSEARCHKEYArr[0][0] 
	 || !bChangeCurRegexp && '\0' == m_szCurSrchKey[0] ){
		goto end_of_func;
	}

	// �����J�n�ʒu�𒲐�
	bFlag1 = FALSE;
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ��`�͈͑I�𒆂łȂ� & �I����Ԃ̃��b�N */
		if( !m_bBeginBoxSelect && TRUE == m_bSelectingLock ){
			bSelecting = TRUE;
			bSelectingLock_Old = m_bSelectingLock;
			nSelectLineBgnFrom_Old = m_nSelectLineBgnFrom;	/* �͈͑I���J�n�s(���_) */
			nSelectColBgnFrom_Old = m_nSelectColmBgnFrom;	/* �͈͑I���J�n��(���_) */
			nSelectLineBgnTo_Old = m_nSelectLineBgnTo;		/* �͈͑I���J�n�s(���_) */
			nSelectColBgnTo_Old = m_nSelectColmBgnTo;		/* �͈͑I���J�n��(���_) */
			nSelectLineFrom_Old = m_nSelectLineFrom;
			nSelectColFrom_Old = m_nSelectColmFrom;
			nSelectLineTo_Old = m_nSelectLineTo;
			nSelectColTo_Old = m_nSelectColmTo;

			if( ( m_nSelectLineBgnFrom >  m_nCaretPosY ) ||
				( m_nSelectLineBgnFrom == m_nCaretPosY && m_nSelectColmBgnFrom >= m_nCaretPosX )
			){
				/* �J�[�\���ړ� */
				m_nCaretPosX=m_nSelectColmFrom;
				m_nCaretPosY=m_nSelectLineFrom;
				if (m_nSelectColmTo == m_nSelectColmFrom && m_nSelectLineTo == m_nSelectLineFrom) {
					// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͂P�����i�߂�(�����}�b�`�΍�) by �����
					b0Match = true;
				}
				bFlag1 = TRUE;
			}else{
				/* �J�[�\���ړ� */
				m_nCaretPosX=m_nSelectColmTo;
				m_nCaretPosY=m_nSelectLineTo;
				if (m_nSelectColmTo == m_nSelectColmFrom && m_nSelectLineTo == m_nSelectLineFrom) {
					// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͂P�����i�߂�(�����}�b�`�΍�) by �����
					b0Match = true;
				}
			}

//			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
//			DisableSelectArea( bRedraw );
		}else{
			/* �J�[�\���ړ� */
			m_nCaretPosX=m_nSelectColmTo;
			m_nCaretPosY=m_nSelectLineTo;
			if (m_nSelectColmTo == m_nSelectColmFrom && m_nSelectLineTo == m_nSelectLineFrom) {
				// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͂P�����i�߂�(�����}�b�`�΍�) by �����
				b0Match = true;
			}

			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( bRedraw );
		}
	}
	nLineNum = m_nCaretPosY;
	nLineLen = 0; // 2004.03.17 Moca NULL == pLine�̂Ƃ��AnLineLen�����ݒ�ɂȂ藎����o�O�΍�
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr(nLineNum, &nLineLen, &pcLayout);
	/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
// 2002.02.08 hor EOF�݂̂̍s��������������Ă��Č����\�� (2/2)
	nIdx = pcLayout ? LineColmnToIndex( pcLayout, m_nCaretPosX ) : 0;
	if( b0Match ) {
		// ���݁A�����O�Ń}�b�`���Ă���ꍇ�͕����s�łP�����i�߂�(�����}�b�`�΍�)
		if( nIdx < nLineLen ) {
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nIdx += (CMemory::GetSizeOfChar(pLine, nLineLen, nIdx) == 2 ? 2 : 1);
		} else {
			// �O�̂��ߍs���͕ʏ���
			++nIdx;
		}
	}

	// 2002.01.16 hor
	// ���ʕ����̂����肾��
	// 2004.05.30 Moca CEditView�̌��ݐݒ肳��Ă��錟���p�^�[�����g����悤��
	if(bChangeCurRegexp && !ChangeCurRegexp())return;

	bRedo		=	TRUE;		//	hor
	nLineNumOld	=	nLineNum;	//	hor
	nIdxOld		=	nIdx;		//	hor
re_do:;
	 /* ���݈ʒu�����̈ʒu���������� */
	// 2004.05.30 Moca ������m_pShareData���烁���o�ϐ��ɕύX�B���̃v���Z�X/�X���b�h�ɏ����������Ă��܂�Ȃ��悤�ɁB
	if( m_pcEditDoc->m_cLayoutMgr.SearchWord(
		nLineNum, 								/* �����J�n�s */
		nIdx, 									/* �����J�n�ʒu */
		m_szCurSrchKey,							/* �������� */
		TRUE,									/* 0==�O������ 1==������� */
		m_bCurSrchRegularExp,					/* 1==���K�\�� */
		m_bCurSrchLoHiCase,						/* 1==�p�啶���������̋�� */
		m_bCurSrchWordOnly,						/* 1==�P��̂݌��� */
		&nLineFrom,								/* �}�b�`���C�A�E�g�sfrom */
		&nColmFrom, 							/* �}�b�`���C�A�E�g�ʒufrom */
		&nLineTo, 								/* �}�b�`���C�A�E�g�sto */
		&nColmTo, 								/* �}�b�`���C�A�E�g�ʒuto */
		&m_CurRegexp							/* ���K�\���R���p�C���f�[�^ */
	) ){

//		/* �t�H�[�J�X�ړ����̍ĕ`�� */
//		RedrawAll();

		/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
		if( bFlag1 && m_nCaretPosX == nColmFrom && m_nCaretPosY == nLineFrom ){
			nLineNum = nLineTo;
			nIdx = nColmTo;
			goto re_do;
		}

		if( bSelecting ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
//			ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
			m_bSelectingLock = bSelectingLock_Old;	/* �I����Ԃ̃��b�N */
		}else{
			/* �I��͈͂̕ύX */
			//	2005.06.24 Moca
			SetSelectArea( nLineFrom, nColmFrom, nLineTo, nColmTo );

			if( bRedraw ){
				/* �I��̈�`�� */
				DrawSelectArea();
			}
//			if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
//				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
//				DisableSelectArea( bRedraw );
//			}
		}

		/* �J�[�\���ړ� */
		//	Sep. 8, 2000 genta
		if ( m_bDrawSWITCH ) AddCurrentLineToHistory();	// 2002.02.16 hor ���ׂĒu���̂Ƃ��͕s�v
		MoveCursor( nColmFrom, nLineFrom, bRedraw );
		m_nCaretPosX_Prev = m_nCaretPosX;
		bFound = TRUE;
	}else{
//		/* �t�H�[�J�X�ړ����̍ĕ`�� */
//		RedrawAll();

		if( bSelecting ){
			m_bSelectingLock = bSelectingLock_Old;	/* �I����Ԃ̃��b�N */

			/* �I��͈͂̕ύX */
			m_nSelectLineBgnFrom = nSelectLineBgnFrom_Old;	/* �͈͑I���J�n�s(���_) */
			m_nSelectColmBgnFrom = nSelectColBgnFrom_Old;	/* �͈͑I���J�n��(���_) */
			m_nSelectLineBgnTo = nSelectLineBgnTo_Old;		/* �͈͑I���J�n�s(���_) */
			m_nSelectColmBgnTo = nSelectColBgnTo_Old;		/* �͈͑I���J�n��(���_) */
			m_nSelectLineFrom =	nSelectLineFrom_Old;
			m_nSelectColmFrom = nSelectColFrom_Old;
//			m_nSelectLineTo = nSelectLineTo_Old;
//			m_nSelectColmTo = nSelectColTo_Old;
			m_nSelectLineTo = nLineFrom;
			m_nSelectColmTo = nColmFrom;

			/* �J�[�\���ړ� */
			MoveCursor( nColmFrom, nLineFrom, bRedraw );
			m_nCaretPosX_Prev = m_nCaretPosX;

			if( bRedraw ){
				/* �I��̈�`�� */
				DrawSelectArea();
			}
		}
	}

//	/* �J�[�\���ړ� */
//	MoveCursor( nColmFrom, nLineFrom, bRedraw );
//	m_nCaretPosX_Prev = m_nCaretPosX;

end_of_func:;
// From Here 2002.01.26 hor �擪�i�����j����Č���
	if(m_pShareData->m_Common.m_bSearchAll){
		if(!bFound	&&		// ������Ȃ�����
			bRedo	&&		// �ŏ��̌���
			m_bDrawSWITCH	// �S�Ēu���̎��s������Ȃ�
		){
			nLineNum=0;
			nIdx=0;
			bRedo=FALSE;
			goto re_do;		// �擪����Č���
		}
	}

	if(bFound){
		if((nLineNumOld > nLineNum)||(nLineNumOld == nLineNum && nIdxOld > nIdx))
			SendStatusMessage("���擪����Č������܂���");
	}else{
		ShowEditCaret();	// 2002/04/18 YAZAKI
		DrawCaretPosInfo();	// 2002/04/18 YAZAKI
		SendStatusMessage("��������܂���ł���");
// To Here 2002.01.26 hor
		::MessageBeep( MB_ICONHAND );
		if( bRedraw	&&
			m_pShareData->m_Common.m_bNOTIFYNOTFOUND	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
		){
			if( NULL == hwndParent ){
				hwndParent = m_hWnd;
			}
			if( NULL == pszNotFoundMessage ){
				::MYMESSAGEBOX( hwndParent,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
					"���(��) �ɕ����� '%s' ���P��������܂���B",	//Jan. 25, 2001 jepro ���b�Z�[�W���኱�ύX
					m_szCurSrchKey
				);
			}else{
				::MYMESSAGEBOX( hwndParent, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
					pszNotFoundMessage
				);
			}
		}
	}
//#ifdef _DEBUG
//	delete pCRunningTimer;
//	pCRunningTimer = NULL;
//
//	gm_ProfileOutput = 0;
//#endif
	return;
}




/* �e�탂�[�h�̎����� */
void CEditView::Command_CANCEL_MODE( void )
{
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( TRUE );
	}
	m_bSelectingLock = FALSE;	/* �I����Ԃ̃��b�N */
	return;
}




/* �͈͑I���J�n */
void CEditView::Command_BEGIN_SELECT( void )
{
	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		BeginSelectArea();
	}
	
	//	���b�N�̉����؂�ւ�
	if ( m_bSelectingLock ) {
		m_bSelectingLock = FALSE;	/* �I����Ԃ̃��b�N���� */
	}
	else {
		m_bSelectingLock = TRUE;	/* �I����Ԃ̃��b�N */
	}
	return;
}




/* ��`�͈͑I���J�n */
void CEditView::Command_BEGIN_BOXSELECT( void )
{
	if( FALSE == m_pShareData->m_Common.m_bFontIs_FIXED_PITCH ){	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		return;
	}

//@@@ 2002.01.03 YAZAKI �͈͑I�𒆂�Shift+F6�����s����ƑI��͈͂��N���A����Ȃ����ɑΏ�
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( TRUE );
	}
	/* ���݂̃J�[�\���ʒu����I�����J�n���� */
	BeginSelectArea();
//	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
//		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
//		BeginSelectArea();
//	}
	
	m_bSelectingLock = TRUE;	/* �I����Ԃ̃��b�N */
	m_bBeginBoxSelect = TRUE;	/* ��`�͈͑I�� */
	return;
}




/* �V�K�쐬 */
void CEditView::Command_FILENEW( void )
{
	/* �V���ȕҏW�E�B���h�E���N�� */
	CEditApp::OpenNewEditor( m_hInstance, m_hWnd, (char*)NULL, 0, FALSE );
	return;
}



/*! @brief �t�@�C�����J��

	@date 2003.03.30 genta �u���ĊJ���v���痘�p���邽�߂Ɉ����ǉ�
	@date 2004.10.09 genta ������CEditDoc�ֈړ�
*/
void CEditView::Command_FILEOPEN( const char *filename, int nCharCode, BOOL bReadOnly )
{
	m_pcEditDoc->OpenFile( filename, nCharCode, bReadOnly );
}




/* ����(����) */	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
void CEditView::Command_FILECLOSE( void )
{
	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
	if( !m_pcEditDoc->OnFileClose() ){
		return;
	}
	/* �����f�[�^�̃N���A */
	m_pcEditDoc->Init();

	/* �S�r���[�̏����� */
	m_pcEditDoc->InitAllView();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();

	return;
}




/*! @brief ���ĊJ��

	@date 2003.03.30 genta �J���_�C�A���O�ŃL�����Z�������Ƃ����̃t�@�C�����c��悤�ɁB
				���ł�FILEOPEN�Ɠ����悤�Ɉ�����ǉ����Ă���
*/
void CEditView::Command_FILECLOSE_OPEN( const char *filename, int nCharCode, BOOL bReadOnly )
{
	/* �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F & �ۑ����s */
	if( !m_pcEditDoc->OnFileClose() ){
		return;
	}

	// Mar. 30, 2003 genta
	char	pszPath[_MAX_PATH];

	if( filename == NULL ){
		pszPath[0] = '\0';
		if( !m_pcEditDoc->OpenFileDialog( m_hWnd, NULL, pszPath, &nCharCode, &bReadOnly ) ){
			return;
		}
	}

	/* �����f�[�^�̃N���A */
	m_pcEditDoc->Init();

	/* �S�r���[�̏����� */
	m_pcEditDoc->InitAllView();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();

	/* �t�@�C�����J�� */
	// Mar. 30, 2003 genta
	// Oct.  9, 2004 genta CEditDoc�ֈړ��������Ƃɂ��ύX
	m_pcEditDoc->OpenFile(( filename ? filename : pszPath ), nCharCode, bReadOnly );

	return;
}




/*! �㏑���ۑ�

	F_FILESAVEALL�Ƃ̑g�ݍ��킹�݂̂Ŏg����R�}���h�D
	@param warnbeep [in] true: �ۑ��s�v or �ۑ��֎~�̂Ƃ��Ɍx�����o��
	@param askname	[in] true: �t�@�C�������ݒ�̎��ɓ��͂𑣂�

	@date 2004.02.28 genta ����warnbeep�ǉ�
	@date 2005.01.24 genta ����askname�ǉ�

*/
BOOL CEditView::Command_FILESAVE( bool warnbeep, bool askname )
{

	/* ���ύX�ł��㏑�����邩 */
	if( FALSE == m_pShareData->m_Common.m_bEnableUnmodifiedOverwrite
	 && !m_pcEditDoc->IsModified()	// �ύX�t���O
	 ){
	 	//	Feb. 28, 2004 genta
	 	//	�ۑ��s�v�ł��x�������o���ė~�����Ȃ��ꍇ������
	 	if( warnbeep ){
			::MessageBeep( MB_ICONHAND );
		}
		return TRUE;
	}

	if( !m_pcEditDoc->IsFilePathAvailable() ){
		if( ! askname ){
			return FALSE;
		}
		//	Feb. 28, 2004 genta SAVEAS�̌��ʂ��������Ԃ���Ă��Ȃ�����
		//	���̏����Ƒg�ݍ��킹��Ƃ��ɖ�肪������
		return Command_FILESAVEAS_DIALOG();
	}
	else {
		//	Jun.  5, 2004 genta
		//	�ǂ݂Ƃ��p�̃`�F�b�N��CEditDoc����㏑���ۑ������Ɉړ�
		if( m_pcEditDoc->m_bReadOnly ){	/* �ǂݎ���p���[�h */
			if( warnbeep ){
				::MessageBeep( MB_ICONHAND );
				MYMESSAGEBOX(
					m_hWnd,
					MB_OK | MB_ICONSTOP | MB_TOPMOST,
					GSTR_APPNAME,
					"%s\n\n�͓ǂݎ���p���[�h�ŊJ���Ă��܂��B �㏑���ۑ��͂ł��܂���B\n\n"
					"���O��t���ĕۑ�������΂����Ǝv���܂��B",
					m_pcEditDoc->IsFilePathAvailable() ? m_pcEditDoc->GetFilePath() : "�i����j"
				);
			}
			return FALSE;
		}

		if( m_pcEditDoc->SaveFile( m_pcEditDoc->GetFilePath() ) ){	//	m_nCharCode, m_cSaveLineCode��ύX�����ɕۑ�
			/* �L�����b�g�̍s���ʒu��\������ */
			DrawCaretPosInfo();
			return TRUE;
		}
	}
	return FALSE;
}

/* ���O��t���ĕۑ��_�C�A���O */
BOOL CEditView::Command_FILESAVEAS_DIALOG()
{
	//	Aug. 16, 2000 genta
	//	���݂̃t�@�C�����������l�ŗ^���Ȃ�
	//	May 18, 2001 genta
	//	���݂̃t�@�C������^���Ȃ��̂͏㏑���֎~�̎��̂�
	//	�����łȂ��ꍇ�ɂ͌��݂̃t�@�C�����������l�Ƃ��Đݒ肷��B
	char szPath[_MAX_PATH + 1];
	if( m_pcEditDoc->IsReadOnly() )
		szPath[0] = '\0';
	else
		strcpy( szPath, m_pcEditDoc->GetFilePath() );

	//	Feb. 9, 2001 genta
	//	Jul. 26, 2003 ryoji BOM�̗L����^����p�����[�^
	if( m_pcEditDoc->SaveFileDialog( szPath, &m_pcEditDoc->m_nCharCode, &m_pcEditDoc->m_cSaveLineCode, &m_pcEditDoc->m_bBomExist ) ){
		//	Jun.  5, 2004 genta
		//	�ǂ݂Ƃ��p�̃`�F�b�N��CEditDoc����㏑���ۑ������Ɉړ�
		//	�����ŏ㏑�������̂�h��
		if( m_pcEditDoc->m_bReadOnly && strcmp( szPath, m_pcEditDoc->GetFilePath()) == 0 ){
			::MessageBeep( MB_ICONHAND );
			MYMESSAGEBOX(
				m_hWnd,
				MB_OK | MB_ICONSTOP | MB_TOPMOST,
				GSTR_APPNAME,
				"�ǂݎ���p���[�h�ł͓���t�@�C���ւ̏㏑���ۑ��͂ł��܂���B"
			);
		}
		else {
			Command_FILESAVEAS( szPath );
			return TRUE;
		}
	}
	return FALSE;
}


/* ���O��t���ĕۑ�
	filename�ŕۑ��BNULL�͌��ցB
*/
BOOL CEditView::Command_FILESAVEAS( const char *filename )
{
	if( m_pcEditDoc->SaveFile( filename ) ){
		/* �L�����b�g�̍s���ʒu��\������ */
		DrawCaretPosInfo();
		m_pcEditDoc->OnChangeSetting();	//	�^�C�v�ʐݒ�̕ύX���w���B
		//	�ăI�[�v��
		//	Jul. 26, 2003 ryoji ���݊J���Ă���̂Ɠ����R�[�h�ŊJ������
		m_pcEditDoc->ReloadCurrentFile( m_pcEditDoc->m_nCharCode, FALSE );
		return TRUE;
	}
	return FALSE;
}

/*!	�S�ď㏑���ۑ�

	�ҏW���̑S�ẴE�B���h�E�ŏ㏑���ۑ����s���D
	�������C�㏑���ۑ��̎w�����o���݂̂Ŏ��s���ʂ̊m�F�͍s��Ȃ��D

	�㏑���֎~�y�уt�@�C�������ݒ�̃E�B���h�E�ł͉����s��Ȃ��D

	@date 2005.01.24 genta �V�K�쐬
*/
BOOL CEditView::Command_FILESAVEALL( void )
{
	CShareData::getInstance()->SendMessageToAllEditors(
		WM_COMMAND, MAKELONG( F_FILESAVE_QUIET, 0 ), (LPARAM)0, NULL);
	return TRUE;
}


/*!	���ݕҏW���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[
	2002/2/3 aroka
*/
void CEditView::Command_COPYFILENAME( void )
{
	if( m_pcEditDoc->IsFilePathAvailable() ){
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		const char *pszFile;
		pszFile = m_pcEditDoc->GetFileName();
		MySetClipboardData( pszFile , lstrlen( pszFile ), FALSE );
	}else{
		::MessageBeep( MB_ICONHAND );
	}
	return;
}




/* ���ݕҏW���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[ */
void CEditView::Command_COPYPATH( void )
{
//	HGLOBAL		hgClip;
//	char*		pszClip;
//	if( 0 < lstrlen( m_pcEditDoc->GetFilePath() ) ){
	if( m_pcEditDoc->IsFilePathAvailable() ){
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		MySetClipboardData( m_pcEditDoc->GetFilePath(), lstrlen( m_pcEditDoc->GetFilePath() ), FALSE );

//		/* Windows�N���b�v�{�[�h�ɃR�s�[ */
//		hgClip = ::GlobalAlloc(
//			GMEM_MOVEABLE | GMEM_DDESHARE,
//			lstrlen( m_pcEditDoc->GetFilePath() ) + 1
//		);
//		pszClip = (char*)::GlobalLock( hgClip );
//		strcpy( pszClip, (char*)m_pcEditDoc->GetFilePath() );
//		::GlobalUnlock( hgClip );
//		::OpenClipboard( m_hWnd );
//		::EmptyClipboard();
//		::SetClipboardData( CF_OEMTEXT, hgClip );
//		::CloseClipboard();
	}else{
		::MessageBeep( MB_ICONHAND );
	}
	return;

}




//	May 9, 2000 genta
/* ���ݕҏW���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���N���b�v�{�[�h�ɃR�s�[ */
void CEditView::Command_COPYTAG( void )
{
	if( m_pcEditDoc->IsFilePathAvailable() ){
		char	buf[ MAX_PATH + 20 ];
		int		line, col;

		//	�_���s�ԍ��𓾂�
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys( m_nCaretPosX, m_nCaretPosY, &col, &line );

		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		wsprintf( buf, "%s (%d,%d): ", m_pcEditDoc->GetFilePath(), line+1, col+1 );
		MySetClipboardData( buf, lstrlen( buf ), FALSE );
	}else{
		::MessageBeep( MB_ICONHAND );
	}
	return;

}

/*! �w��s�փW�����v�_�C�A���O�̕\��
	2002.2.2 YAZAKI
*/
void CEditView::Command_JUMP_DIALOG( void )
{
	if( !m_pcEditDoc->m_cDlgJump.DoModal(
		m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc
	) ){
		return;
	}
}



/* �w��s�w�W�����v */
void CEditView::Command_JUMP( void )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nMode;
	int			nLineNum;
	int			bValidLine;
	int			nCurrentLine;
	int			nCommentBegin;
	int			nBgn;
#if 0
	2002.2.2 YAZAKI �_�C�A���O�Ăяo�����ƁA�R�}���h���s���𕪗�
//	int			nCharChars;
//	m_pcEditDoc->m_cDlgJump.Create( m_hInstance, m_hWnd, (void *)m_pcEditDoc );
	if( !m_pcEditDoc->m_cDlgJump.DoModal(
		m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
//		m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//		&m_pcEditDoc->m_hwndActiveDialog						/* �A�N�e�B�u�Ȏq�_�C�A���O */
	) ){
//		::MessageBeep( MB_ICONHAND );	//Feb. 20, 2001 JEPRO [�L�����Z��]���ɖ�x�����̐��̂͂���(�R�����g�A�E�g�ɂ���)
		return;
	}
#endif
	if( 0 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �s�ԍ� */
	nLineNum = m_pcEditDoc->m_cDlgJump.m_nLineNum;
	if( !m_pcEditDoc->m_cDlgJump.m_bPLSQL ){	/* PL/SQL�\�[�X�̗L���s�� */
		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
//		if( m_pcEditDoc->m_cDlgJump.m_bLineNumIsCRLF ){
		if( m_pShareData->m_bLineNumIsCRLF ){
			if( 0 >= nLineNum ){
				nLineNum = 1;
			}
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/
			int		nPosX;
			int		nPosY;
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				0,
				nLineNum - 1,
				&nPosX,
				&nPosY
			);
			nLineNum = nPosY + 1;
		}else{
			if( 0 >= nLineNum ){
				nLineNum = 1;
			}
			if( nLineNum > m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
				nLineNum = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			}
		}
		//	Sep. 8, 2000 genta
		AddCurrentLineToHistory();
		//	2006.07.09 genta �I����Ԃ��������Ȃ��悤��
		MoveCursorSelecting( 0, nLineNum - 1, m_bSelectingLock, _CARETMARGINRATE / 3 );
		return;
	}
	if( 0 >= nLineNum ){
		nLineNum = 1;
	}
	nMode = 0;
	nCurrentLine = m_pcEditDoc->m_cDlgJump.m_nPLSQL_E2 - 1;
	nLineCount = m_pcEditDoc->m_cDlgJump.m_nPLSQL_E1 - 1;
	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	if( FALSE == m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){
		/*
		  �J�[�\���ʒu�ϊ�
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		  ��
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		*/
		int nPosX,nPosY;
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			0,
			nLineCount,
			(int*)&nPosX,
			(int*)&nPosY
		);
		nLineCount = nPosY;
	}
//	for( ; nLineCount <  m_pcEditDoc->m_cLayoutMgr.GetLineCount(); ++nLineCount ){
	for( ; nLineCount <  m_pcEditDoc->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
//		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineCount, &nLineLen );
		pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		bValidLine = FALSE;
		nBgn = 0;
		for( i = 0; i < nLineLen; ++i ){
			if( ' ' != pLine[i] &&
				TAB != pLine[i]
			){
				break;
			}
		}
		nBgn = i;
		for( i = nBgn; i < nLineLen; ++i ){
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				bValidLine = TRUE;
				if( '\'' == pLine[i] ){
					if( i > 0 && '\\' == pLine[i - 1] ){
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
				if( '"' == pLine[i] ){
					if( i > 0 && '\\' == pLine[i - 1] ){
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
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
				if( '\t' == pLine[i] ||
					' ' == pLine[i] ||
					CR == pLine[i] ||
					LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && '-' == pLine[i] &&  '-' == pLine[i + 1] ){
					bValidLine = TRUE;
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					nCommentBegin = nLineCount;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
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
			if( CR == pLine[nBgn] ||
				LF == pLine[nBgn] ){
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
	int		nPosX;
	int		nPosY;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
		0,
		nLineCount,
		&nPosX,
		&nPosY
	);
	//	Sep. 8, 2000 genta
	AddCurrentLineToHistory();
	//	2006.07.09 genta �I����Ԃ��������Ȃ��悤��
	MoveCursorSelecting( nPosX, nPosY, m_bSelectingLock, _CARETMARGINRATE / 3 );
}




/* �t�H���g�ݒ� */
void CEditView::Command_FONT( void )
{
	HWND	hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	/* �t�H���g�ݒ�_�C�A���O */
	LOGFONT cLogfont = m_pShareData->m_Common.m_lf;
	if( m_pcEditDoc->SelectFont( &cLogfont )  ){
		m_pShareData->m_Common.m_lf = cLogfont;

//		/* �ύX�t���O �t�H���g */
//		m_pShareData->m_bFontModify = TRUE;

		if( m_pShareData->m_Common.m_lf.lfPitchAndFamily & FIXED_PITCH  ){
			m_pShareData->m_Common.m_bFontIs_FIXED_PITCH = TRUE;	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		}else{
			m_pShareData->m_Common.m_bFontIs_FIXED_PITCH = FALSE;	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		}
		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CShareData::getInstance()->PostMessageToAllEditors(
//		m_cShareData.SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			(WPARAM)0, (LPARAM)hwndFrame, hwndFrame
		);

		/* �L�����b�g�̕\�� */
//		::HideCaret( m_hWnd );
//		::ShowCaret( m_hWnd );

//		/* �A�N�e�B�u�ɂ��� */
//		/* �A�N�e�B�u�ɂ��� */
//		ActivateFrameWindow( hwndFrame );
	}
	return;
}




/* ���ʐݒ� */
void CEditView::Command_OPTION( void )
{
	/* �ݒ�v���p�e�B�V�[�g �e�X�g�p */
	m_pcEditDoc->OpenPropertySheet( -1/*, -1*/ );

	return;
}




/* �^�C�v�ʐݒ� */
void CEditView::Command_OPTION_TYPE( void )
{
	m_pcEditDoc->OpenPropertySheetTypes( -1, m_pcEditDoc->GetDocumentType() );

	return;
}




/* �^�C�v�ʐݒ�ꗗ */
void CEditView::Command_TYPE_LIST( void )
{
	CDlgTypeList	cDlgTypeList;
	int				nSettingType;
//	cDlgTypeList.Create( m_hInstance, m_hWnd );
	nSettingType = m_pcEditDoc->GetDocumentType();
	if( cDlgTypeList.DoModal( m_hInstance, m_hWnd, &nSettingType ) ){
		//	Nov. 29, 2000 genta
		//	�ꎞ�I�Ȑݒ�K�p�@�\�𖳗���ǉ�
		if( nSettingType & PROP_TEMPCHANGE_FLAG ){
			m_pcEditDoc->SetDocumentType( nSettingType & ~PROP_TEMPCHANGE_FLAG, true );
			m_pcEditDoc->LockDocumentType();
			/* �ݒ�ύX�𔽉f������ */
			m_pcEditDoc->OnChangeSetting();
		}
		else{
			/* �^�C�v�ʐݒ� */
			m_pcEditDoc->OpenPropertySheetTypes( -1, nSettingType );
		}
	}
	return;
}




/* �s�̓�d��(�܂�Ԃ��P��) */
void CEditView::Command_DUPLICATELINE( void )
{
	int				nCaretPosXOld;
	int				nCaretPosYOld;
	COpe*			pcOpe = NULL;
	int				nNewLine;
	int				nNewPos;
//	int				i;
	int				bCRLF;
	int				bAddCRLF;
	CMemory			cmemBuf;
	const CLayout*	pcLayout;

	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( TRUE );
	}

	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
	if( NULL == pcLayout ){
		::MessageBeep( MB_ICONHAND );
		return;
	}

	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;									/* ������ */
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	nCaretPosXOld = m_nCaretPosX;
	nCaretPosYOld = m_nCaretPosY + 1;

	//�s���Ɉړ�(�܂�Ԃ��P��)
	Command_GOLINETOP( m_bSelectingLock, 0x1 /* �J�[�\���ʒu�Ɋ֌W�Ȃ��s���Ɉړ� */ );

	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;									/* ������ */
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}



	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */
	}

	/* ��d���������s�𒲂ׂ�
	||	�E���s�ŏI����Ă���
	||	�E���s�ŏI����Ă��Ȃ�
	||	�E�ŏI�s�ł���
	||	���܂�Ԃ��łȂ�
	||	�E�ŏI�s�łȂ�
	||	���܂�Ԃ��ł���
	*/
	bCRLF = ( EOL_NONE == pcLayout->m_cEol ) ? FALSE : TRUE;

	bAddCRLF = FALSE;
	if( !bCRLF ){
		if( m_nCaretPosY == m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 ){
			bAddCRLF = TRUE;
		}
	}

	cmemBuf.SetData( pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() + pcLayout->m_cEol.GetLen() );	//	��pcLayout->GetLengthWithEOL()�́AEOL�̒�����K��1�ɂ���̂Ŏg���Ȃ��B
	if( bAddCRLF ){
		/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */
		CEOL cWork = m_pcEditDoc->GetNewLineCode();
		cmemBuf.Append( cWork.GetValue(), cWork.GetLen() );
	}

	/* ���݈ʒu�Ƀf�[�^��}�� */
	InsertData_CEditView(
		m_nCaretPosX,
		m_nCaretPosY,
		(char*)cmemBuf.GetPtr(),
		cmemBuf.GetLength(),
		&nNewLine,
		&nNewPos,
		pcOpe,
		TRUE
	);

	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//		pcOpe->m_nCaretPosX_After = nNewPos;	/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = nNewLine;	/* �����̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_After,
//			pcOpe->m_nCaretPosY_After,
//			&pcOpe->m_nCaretPosX_PHY_After,
//			&pcOpe->m_nCaretPosY_PHY_After
//		);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			nNewPos,
			nNewLine,
			&pcOpe->m_nCaretPosX_PHY_After,
			&pcOpe->m_nCaretPosY_PHY_After
		);

		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}

	/* �J�[�\�����ړ� */
	MoveCursor( nCaretPosXOld, nCaretPosYOld, TRUE );
	m_nCaretPosX_Prev = m_nCaretPosX;


	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
		pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
//		pcOpe->m_nCaretPosX_Before = m_nCaretPosX;	/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = m_nCaretPosY;	/* ����O�̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_Before,
//			pcOpe->m_nCaretPosY_Before,
//			&pcOpe->m_nCaretPosX_PHY_Before,
//			&pcOpe->m_nCaretPosY_PHY_Before
//		);
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */

//		pcOpe->m_nCaretPosX_After = m_nCaretPosX;						/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = m_nCaretPosY;						/* �����̃L�����b�g�ʒu�x */
//		pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;				/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;				/* �����̃L�����b�g�ʒu�x */
		pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	return;
}




/* �p�啶�����p������ */
void CEditView::Command_TOLOWER( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_TOLOWER );
	return;
}




/* �p���������p�啶�� */
void CEditView::Command_TOUPPER( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_TOUPPER );
	return;
}




/* �S�p�����p */
void CEditView::Command_TOHANKAKU( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_TOHANKAKU );
	return;
}


/* �S�p�J�^�J�i�����p�J�^�J�i */		//Aug. 29, 2002 ai
void CEditView::Command_TOHANKATA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_TOHANKATA );
	return;
}


/*! ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
void CEditView::Command_TOZENEI( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_TOZENEI );
	return;
}

/*! �S�p�p�������p�p�� */
void CEditView::Command_TOHANEI( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_TOHANEI );
	return;
}


/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
void CEditView::Command_TOZENKAKUKATA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_TOZENKAKUKATA );
	return;
}




/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
void CEditView::Command_TOZENKAKUHIRA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_TOZENKAKUHIRA );
	return;
}




/* ���p�J�^�J�i���S�p�J�^�J�i */
void CEditView::Command_HANKATATOZENKAKUKATA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_HANKATATOZENKAKUKATA );
	return;
}




/* ���p�J�^�J�i���S�p�Ђ炪�� */
void CEditView::Command_HANKATATOZENKAKUHIRA( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_HANKATATOZENKAKUHIRA );
	return;
}




/* TAB���� */
void CEditView::Command_TABTOSPACE( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_TABTOSPACE );
	return;
}

/* �󔒁�TAB */ //#### Stonee, 2001/05/27
void CEditView::Command_SPACETOTAB( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_SPACETOTAB );
	return;
}



//#define F_HANKATATOZENKAKUKATA	30557	/* ���p�J�^�J�i���S�p�J�^�J�i */
//#define F_HANKATATOZENKAKUHIRA	30558	/* ���p�J�^�J�i���S�p�Ђ炪�� */




/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
void CEditView::Command_CODECNV_EMAIL( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_EMAIL );
	return;
}




/* EUC��SJIS�R�[�h�ϊ� */
void CEditView::Command_CODECNV_EUC2SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_EUC2SJIS );
	return;
}




/* Unicode��SJIS�R�[�h�ϊ� */
void CEditView::Command_CODECNV_UNICODE2SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_UNICODE2SJIS );
	return;
}




/* UnicodeBE��SJIS�R�[�h�ϊ� */
void CEditView::Command_CODECNV_UNICODEBE2SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_UNICODEBE2SJIS );
	return;
}




/* SJIS��JIS�R�[�h�ϊ� */
void CEditView::Command_CODECNV_SJIS2JIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_SJIS2JIS );
	return;
}




/* SJIS��EUC�R�[�h�ϊ� */
void CEditView::Command_CODECNV_SJIS2EUC( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_SJIS2EUC );
	return;
}




/* UTF-8��SJIS�R�[�h�ϊ� */
void CEditView::Command_CODECNV_UTF82SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_UTF82SJIS );
	return;
}




/* UTF-7��SJIS�R�[�h�ϊ� */
void CEditView::Command_CODECNV_UTF72SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_UTF72SJIS );
	return;
}




/* SJIS��UTF-7�R�[�h�ϊ� */
void CEditView::Command_CODECNV_SJIS2UTF7( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_SJIS2UTF7 );
	return;
}




/* SJIS��UTF-8�R�[�h�ϊ� */
void CEditView::Command_CODECNV_SJIS2UTF8( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_SJIS2UTF8 );
	return;
}




/* �������ʁ�SJIS�R�[�h�ϊ� */
void CEditView::Command_CODECNV_AUTO2SJIS( void )
{
	/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
	ConvSelectedArea( F_CODECNV_AUTO2SJIS );
	return;
}




/*!	�A�E�g���C�����
	
	2002/3/13 YAZAKI nOutlineType��nListType�𓝍��B
*/
//BOOL CEditView::Command_FUNCLIST( BOOL bCheckOnly )	//	2001.12.03 hor �u�b�N�}�[�N�p�̃t���O��ǉ�
BOOL CEditView::Command_FUNCLIST( int nAction/*nReLoad,bCheckOnly*/, int nOutlineType ) // �g�O���p�̃t���O�ɕύX 20060201 aroka
{
//	if( bCheckOnly ){
//		return TRUE;
//	}

	static CFuncInfoArr	cFuncInfoArr;
//	int		nLine;
//	int		nListType;

	//	2001.12.03 hor & 2002.3.13 YAZAKI
	if( nOutlineType == OUTLINE_DEFAULT ){
		/* �^�C�v�ʂɐݒ肳�ꂽ�A�E�g���C����͕��@ */
		nOutlineType = m_pcEditDoc->GetDocumentAttribute().m_nDefaultOutline;
	}

	if( NULL != m_pcEditDoc->m_cDlgFuncList.m_hWnd && nAction != SHOW_RELOAD ){
		switch(nAction ){
		case SHOW_NORMAL: // �A�N�e�B�u�ɂ���
			//	�J���Ă�����̂Ǝ�ʂ������Ȃ�Active�ɂ��邾���D�قȂ�΍ĉ��
			if( m_pcEditDoc->m_cDlgFuncList.CheckListType( nOutlineType )){
				ActivateFrameWindow( m_pcEditDoc->m_cDlgFuncList.m_hWnd );
				return TRUE;
			}
			break;
		case SHOW_TOGGLE: // ����
			//	�J���Ă�����̂Ǝ�ʂ������Ȃ����D�قȂ�΍ĉ��
			if( m_pcEditDoc->m_cDlgFuncList.CheckListType( nOutlineType )){
				::SendMessage( m_pcEditDoc->m_cDlgFuncList.m_hWnd, WM_CLOSE, 0, 0 );
				return TRUE;
			}
			break;
		default:
			break;
		}
	}
//	if( NULL != m_pcEditDoc->m_cDlgFuncList.m_hWnd && !nReLoad ){
//		/* �A�N�e�B�u�ɂ��� */
////		m_pcEditDoc->m_cDlgFuncList.m_nCurLine = m_nCaretPosY + 1;	// 2002/04/18 YAZAKI
//		//	Oct. 5, 2002 genta
//		//	�J���Ă�����̂Ǝ�ʂ������Ȃ�Active�ɂ��邾���D�قȂ�΍ĉ��
//		if( m_pcEditDoc->m_cDlgFuncList.CheckListType( nOutlineType )){
//			ActivateFrameWindow( m_pcEditDoc->m_cDlgFuncList.m_hWnd );
//			return TRUE;
//		}
//	}

	/* ��͌��ʃf�[�^����ɂ��� */
	cFuncInfoArr.Empty();

	switch( nOutlineType ){
//	case OUTLINE_C:			m_pcEditDoc->MakeFuncList_C( &cFuncInfoArr );break;
	case OUTLINE_CPP:
		m_pcEditDoc->MakeFuncList_C( &cFuncInfoArr );
		/* C����W���ی�ψ�������ʏ��������ӏ�(�R) */
		if( CheckEXT( m_pcEditDoc->GetFilePath(), "c" ) ){
			nOutlineType = OUTLINE_C;	/* �����C�֐��ꗗ���X�g�r���[�ɂȂ� */
		}
		break;
	case OUTLINE_PLSQL:		m_pcEditDoc->MakeFuncList_PLSQL( &cFuncInfoArr );break;
	case OUTLINE_JAVA:		m_pcEditDoc->MakeFuncList_Java( &cFuncInfoArr );break;
	case OUTLINE_COBOL:		m_pcEditDoc->MakeTopicList_cobol( &cFuncInfoArr );break;
	case OUTLINE_ASM:		m_pcEditDoc->MakeTopicList_asm( &cFuncInfoArr );break;
	case OUTLINE_PERL:		m_pcEditDoc->MakeFuncList_Perl( &cFuncInfoArr );break;	//	Sep. 8, 2000 genta
	case OUTLINE_VB:		m_pcEditDoc->MakeFuncList_VisualBasic( &cFuncInfoArr );break;	//	June 23, 2001 N.Nakatani
	case OUTLINE_WZTXT:		m_pcEditDoc->MakeTopicList_wztxt(&cFuncInfoArr);break;		// 2003.05.20 zenryaku �K�w�t�e�L�X�g �A�E�g���C�����
	case OUTLINE_HTML:		m_pcEditDoc->MakeTopicList_html(&cFuncInfoArr);break;		// 2003.05.20 zenryaku HTML �A�E�g���C�����
	case OUTLINE_TEX:		m_pcEditDoc->MakeTopicList_tex(&cFuncInfoArr);break;		// 2003.07.20 naoh TeX �A�E�g���C�����
	case OUTLINE_BOOKMARK:	m_pcEditDoc->MakeFuncList_BookMark( &cFuncInfoArr );break;	//	2001.12.03 hor
	case OUTLINE_FILE:		m_pcEditDoc->MakeFuncList_RuleFile( &cFuncInfoArr );break;	//	2002.04.01 YAZAKI �A�E�g���C����͂Ƀ��[���t�@�C���𓱓�
	case OUTLINE_TEXT:
//	case OUTLINE_UNKNOWN:	//Jul. 08, 2001 JEPRO �g��Ȃ��悤�ɕύX
	default:
		m_pcEditDoc->MakeTopicList_txt( &cFuncInfoArr );
		break;
	}

	/* ��͑Ώۃt�@�C���� */
	strcpy( cFuncInfoArr.m_szFilePath, m_pcEditDoc->GetFilePath() );

	/* �A�E�g���C�� �_�C�A���O */
//	m_pcEditDoc->m_cDlgFuncList.Create(
//		m_hInstance,
//		/*m_pcEditDoc->*/m_hWnd,
//		&cFuncInfoArr,
//		m_nCaretPosY + 1,
//		nListType,
//		m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//	);

//	/* �A�E�g���C�� �_�C�A���O�����[�_���ɂ��邩 */
//	if( ????? ){
//		nLine = m_pcEditDoc->m_cDlgFuncList.DoModal();
//		if( nLine > 0 ){
//			/* ��`�͈͑I�𒆂� */
//			if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
//				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
//				DisableSelectArea( TRUE );
//			}
//
//			/*
//			  �J�[�\���ʒu�ϊ�
//			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
//			  ��
//			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
//			*/
//			int		nPosX;
//			int		nPosY;
//			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
//				0,
//				nLine - 1,
//				&nPosX,
//				&nPosY
//			);
//			/* �J�[�\����I���J�n�ʒu�Ɉړ� */
//			MoveCursor( 0, nPosY, TRUE, _CARETMARGINRATE / 3 );
//			m_nCaretPosX_Prev = m_nCaretPosX;
//		}
//	}else{
		/* �A�E�g���C�� �_�C�A���O�̕\�� */
		if( NULL == m_pcEditDoc->m_cDlgFuncList.m_hWnd ){
//			m_pcEditDoc->m_cDlgFuncList.DoModeless( this );
			m_pcEditDoc->m_cDlgFuncList.DoModeless(
				m_hInstance,
				/*m_pcEditDoc->*/m_hWnd,
				(LPARAM)this,
				&cFuncInfoArr,
				m_nCaretPosY + 1,
				nOutlineType,
				m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
			);
		}else{
			/* �A�N�e�B�u�ɂ��� */
			m_pcEditDoc->m_cDlgFuncList.Redraw( nOutlineType, &cFuncInfoArr, m_nCaretPosY + 1 );
			ActivateFrameWindow( m_pcEditDoc->m_cDlgFuncList.m_hWnd );
//			::SetDlgItemText( m_pcEditDoc->m_cDlgFuncList.m_hWnd, IDC_COMBO_TEXT, cmemCurText.GetPtr() );
		}
//	}

	return TRUE;
}




/* �㉺�ɕ��� */	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
void CEditView::Command_SPLIT_V( void )
{
	m_pcEditDoc->m_cSplitterWnd.VSplitOnOff();
	return;
}




/* ���E�ɕ��� */	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
void CEditView::Command_SPLIT_H( void )
{
	m_pcEditDoc->m_cSplitterWnd.HSplitOnOff();
	return;
}




/* �c���ɕ��� */	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
void CEditView::Command_SPLIT_VH( void )
{
	m_pcEditDoc->m_cSplitterWnd.VHSplitOnOff();
	return;
}




//From Here Nov. 25, 2000 JEPRO
/* �w���v�ڎ� */
void CEditView::Command_HELP_CONTENTS( void )
{
	char	szHelp[_MAX_PATH + 1];
	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	::GetHelpFilePath( szHelp );
	ShowWinHelpContents( m_hWnd, szHelp );	//	�ڎ���\������
	return;
}




/* �w���v�L�[���[�h���� */
void CEditView::Command_HELP_SEARCH( void )
{
	char	szHelp[_MAX_PATH + 1];
	/* �w���v�t�@�C���̃t���p�X��Ԃ� */
	::GetHelpFilePath( szHelp );
	MyWinHelp( m_hWnd, szHelp, HELP_KEY, (ULONG_PTR)"" );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
	return;
}
//To Here Nov. 25, 2000




/*! �L�����b�g�ʒu�̒P�����������ON-OFF

	@date 2006.03.24 fon �V�K�쐬
*/
void CEditView::Command_ToggleKeySearch( void )
{	/* ���ʐݒ�_�C�A���O�̐ݒ���L�[���蓖�Ăł��؂�ւ�����悤�� */
	if(TRUE == m_pShareData->m_Common.m_bUseCaretKeyWord ){
		m_pShareData->m_Common.m_bUseCaretKeyWord = FALSE;
	}else{
		m_pShareData->m_Common.m_bUseCaretKeyWord = TRUE;
	}
}




/* �R�}���h�ꗗ */
void CEditView::Command_MENU_ALLFUNC( void )
{

	char	szLabel[300];
//	char	szLabel2[300];
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
	::ClientToScreen( m_hWnd, &po );

	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->m_CMenuDrawer.ResetContents();

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = m_pcEditDoc->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
//Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɕύX�������Ƃ�1��(�J�[�\���ړ��n)���O�ɃV�t�g���ꂽ(���̕ύX�ɂ���� i=1��i=0 �ƕύX)
	//	Oct. 3, 2001 genta
	for( i = 0; i < FuncLookup.GetCategoryCount(); i++ ){
		hMenuPopUp = ::CreatePopupMenu();
		for( j = 0; j < FuncLookup.GetItemCount(i); j++ ){
			//	Oct. 3, 2001 genta
			int code = FuncLookup.Pos2FuncCode( i, j );
			if( code != 0 ){
				FuncLookup.Pos2FuncName( i, j, szLabel, 256 );
				//::LoadString( m_hInstance, nsFuncCode::ppnFuncListArr[i][j], szLabel, 256 );
				uFlags = MF_BYPOSITION | MF_STRING | MF_ENABLED;
		//			uFlags = MF_BYPOSITION | MF_STRING | MF_DISABLED | MF_GRAYED;
		//			pCEditWnd->m_CMenuDrawer.MyAppendMenu( hMenuPopUp, uFlags, nsFuncCode::ppnFuncListArr[i][j] , szLabel );
				//	Oct. 3, 2001 genta
				pCEditWnd->m_CMenuDrawer.MyAppendMenu( hMenuPopUp, uFlags, code, szLabel );
			}
		}
		//	Oct. 3, 2001 genta
		pCEditWnd->m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , FuncLookup.Category2Name(i) );
//		pCEditWnd->m_CMenuDrawer.MyAppendMenu( hMenu, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT)hMenuPopUp , nsFuncCode::ppszFuncKind[i] );
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
		::GetParent( m_hwndParent )/*m_hWnd*/,
		NULL
	);
	::DestroyMenu( hMenu );
	if( 0 != nId ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
//		HandleCommand( nFuncID, TRUE, 0, 0, 0, 0 );
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nId, 0 ), (LPARAM)NULL );
	}
	return;
}




/* �O���w���v�P */
void CEditView::Command_EXTHELP1( void )
{
retry:;
	if( CShareData::getInstance()->ExtWinHelpIsSet( m_pcEditDoc->GetDocumentType() ) == false){
//	if( 0 == strlen( m_pShareData->m_Common.m_szExtHelp1 ) ){
		::MessageBeep( MB_ICONHAND );
//From Here Sept. 15, 2000 JEPRO
//		[Esc]�L�[��[x]�{�^���ł����~�ł���悤�ɕύX
//		if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
		if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
//To Here Sept. 15, 2000
			"�O���w���v�P���ݒ肳��Ă��܂���B\n�������ݒ肵�܂���?"
		) ){
			/* ���ʐݒ� �v���p�e�B�V�[�g */
			if( !m_pcEditDoc->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHELP1*/ ) ){
				return;
			}
			goto retry;
		}
		//	Jun. 15, 2000 genta
		else{
			return;
		}
	}

	CMemory		cmemCurText;
	const char*	helpfile = CShareData::getInstance()->GetExtWinHelp( m_pcEditDoc->GetDocumentType() );
	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	GetCurrentTextForSearch( cmemCurText );
	if( _IS_REL_PATH( helpfile ) ){
		// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
		char path[_MAX_PATH];
		GetExecutableDir( path, helpfile );
		::WinHelp( m_hwndParent, path, HELP_KEY, (ULONG_PTR)(char*)cmemCurText.GetPtr() );
		return;
	}
	::WinHelp( m_hwndParent, helpfile , HELP_KEY, (ULONG_PTR)(char*)cmemCurText.GetPtr() );
	return;
}




/*!
	�O��HTML�w���v
	
	@param helpfile [in] HTML�w���v�t�@�C�����DNULL�̂Ƃ��̓^�C�v�ʂɐݒ肳�ꂽ�t�@�C���D
	@param kwd [in] �����L�[���[�h�DNULL�̂Ƃ��̓J�[�\���ʒuor�I�����ꂽ���[�h
	@date 2002.07.05 genta �C�ӂ̃t�@�C���E�L�[���[�h�̎w�肪�ł���悤�����ǉ�
*/
void CEditView::Command_EXTHTMLHELP( const char* helpfile, const char* kwd )
{
	HWND		hwndHtmlHelp;
//	HWND		hwndHtmlHelpChild;
	CMemory		cmemCurText;
	int			nLen;

	//	From Here Jul. 5, 2002 genta
	const char *filename = NULL;
	if ( helpfile == NULL || helpfile[0] == '\0' ){
		while( !CShareData::getInstance()->ExtHTMLHelpIsSet( m_pcEditDoc->GetDocumentType()) ){
			::MessageBeep( MB_ICONHAND );
	//	From Here Sept. 15, 2000 JEPRO
	//		[Esc]�L�[��[x]�{�^���ł����~�ł���悤�ɕύX
	//		if( IDYES == ::MYMESSAGEBOX( NULL, MB_YESNO | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
			if( IDYES != ::MYMESSAGEBOX( NULL, MB_YESNOCANCEL | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_TOPMOST, GSTR_APPNAME,
	//	To Here Sept. 15, 2000
				"�O��HTML�w���v���ݒ肳��Ă��܂���B\n�������ݒ肵�܂���?"
			) ){
				return;
			}
			/* ���ʐݒ� �v���p�e�B�V�[�g */
			if( !m_pcEditDoc->OpenPropertySheet( ID_PAGENUM_HELPER/*, IDC_EDIT_EXTHTMLHELP*/ ) ){
				return;
			}
		}
		filename = CShareData::getInstance()->GetExtHTMLHelp( m_pcEditDoc->GetDocumentType() );
	}
	else {
		filename = helpfile;
	}
	//	To Here Jul. 5, 2002 genta

	//	Jul. 5, 2002 genta
	//	�L�[���[�h�̊O���w����\��
	if( kwd != NULL && kwd[0] != '\0' ){
		cmemCurText.SetDataSz( kwd );
	}
	else {
		/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
		GetCurrentTextForSearch( cmemCurText );
	}

	/* HtmlHelp�r���[�A�͂ЂƂ� */
	if( CShareData::getInstance()->HTMLHelpIsSingle( m_pcEditDoc->GetDocumentType() ) ){
//	if( m_pShareData->m_Common.m_bHtmlHelpIsSingle ){
		// �^�X�N�g���C�̃v���Z�X��HtmlHelp���N��������
		// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
		if( _IS_REL_PATH( filename ) ){
			GetExecutableDir( m_pShareData->m_szWork, filename );
		}else{
			strcpy( m_pShareData->m_szWork, filename ); //	Jul. 5, 2002 genta
		}
		nLen = lstrlen( m_pShareData->m_szWork );
		strcpy( &m_pShareData->m_szWork[nLen + 1], cmemCurText.GetPtr() );
		hwndHtmlHelp = (HWND)::SendMessage( m_pShareData->m_hwndTray, MYWM_HTMLHELP, (WPARAM)::GetParent( m_hwndParent ), 0 );
	}else{
		/* ������HtmlHelp���N�������� */
		HH_AKLINK	link;
		link.cbStruct = sizeof( HH_AKLINK ) ;
		link.fReserved = FALSE ;
		link.pszKeywords = (char*)cmemCurText.GetPtr();
		link.pszUrl = NULL;
		link.pszMsgText = NULL;
		link.pszMsgTitle = NULL;
		link.pszWindow = NULL;
		link.fIndexOnFail = TRUE;

		// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
		if( _IS_REL_PATH( filename ) ){
			char path[_MAX_PATH];
			GetExecutableDir( path, filename );
			//	Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*m_pShareData->m_hwndTray*/,
				path, //	Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}else{
			//	Jul. 6, 2001 genta HtmlHelp�̌Ăяo�����@�ύX
			hwndHtmlHelp = OpenHtmlHelp(
				NULL/*m_pShareData->m_hwndTray*/,
				filename, //	Jul. 5, 2002 genta
				HH_KEYWORD_LOOKUP,
				(DWORD_PTR)&link
			);
		}
	}



//	if( NULL != hwndHtmlHelp ){
//		hwndHtmlHelpChild = ::FindWindowEx( hwndHtmlHelp, NULL, "#32770", NULL );
//		if( NULL != hwndHtmlHelpChild ){
//			hwndHtmlHelp = hwndHtmlHelpChild;
//		}else{
//		}
//	}
//	int		nResult;
//	nResult = ::PostMessage(
//		hwndHtmlHelpChild,
//		WM_MOUSEACTIVATE,
//		(WPARAM) hwndHtmlHelp,					// �A�v���P�[�V�����̃E�B���h�E���w��
//		MAKELPARAM( HTCLIENT, WM_LBUTTONDOWN )	// �Ƃ肠�����N���C�A���g�̈��LBUTTONDOWN�ɂ��Ă���
//	);
//	if ( nResult == MA_ACTIVATE || nResult == MA_ACTIVATEANDEAT ){
//		::SetFocus( hwndHtmlHelpChild );		// �A�N�e�B�u�ɂ���ꍇ�����t�H�[�J�X���ڂ�
//		/* �A�N�e�B�u�ɂ��� */
//		ActivateFrameWindow( hwndHtmlHelpChild );
//	}
//
//	HWND	hToplevel = ::FindWindow( "#32770", "�ݒ�" );
//	HWND	hWnd = ::FindWindowEx( hToplevel, NULL, "Edit", NULL );
//	if ( hwndHtmlHelp != NULL ){
//		DWORD	dwPID;
//		DWORD	dwTID = ::GetWindowThreadProcessId( hwndHtmlHelp, &dwPID );
//		::AttachThreadInput( ::GetCurrentThreadId(), dwTID, TRUE) ;
//		::SetFocus( hwndHtmlHelp );
//		::AttachThreadInput( ::GetCurrentThreadId(), dwTID, FALSE );
//	}

/* ������HtmlHelp���N�������� */
//	hwndHtmlHelp = ::HtmlHelp(
//		NULL/*m_pShareData->m_hwndTray*/,
//		m_pShareData->m_Common.m_szExtHtmlHelp,
//		HH_DISPLAY_TOPIC,
//		(DWORD)0
//	);
//	HH_AKLINK	link;
//	link.cbStruct = sizeof(HH_AKLINK) ;
//	link.fReserved = FALSE ;
//	link.pszKeywords = (char*)cmemCurText.GetPtr();
//	link.pszUrl = NULL;
//	link.pszMsgText = NULL;
//	link.pszMsgTitle = NULL;
//	link.pszWindow = NULL;
//	link.fIndexOnFail = TRUE;
//	hwndHtmlHelp = ::HtmlHelp(
//		NULL/*m_pShareData->m_hwndTray*/,
//		m_pShareData->m_Common.m_szExtHtmlHelp,
//		HH_KEYWORD_LOOKUP,
//		(DWORD)&link
//	);


//	memcpy( m_pShareData->m_szWork, (void*)&link, sizeof(HH_AKLINK) );

//	/* �A�N�e�B�u�ɂ��� */
//	ActivateFrameWindow( hwndHtmlHelp );
//	hwndHtmlHelpChild = ::FindWindowEx( hwndHtmlHelp, NULL, "#32770", NULL );
//	if( NULL != hwndHtmlHelpChild ){
//		::BringWindowToTop( hwndHtmlHelpChild );

//		/* �A�N�e�B�u�ɂ��� */
//		ActivateFrameWindow( hwndHtmlHelpChild );
//		::SetFocus( hwndHtmlHelpChild );
//		::SetActiveWindow( hwndHtmlHelpChild );
//		::PostMessage( hwndHtmlHelpChild, WM_ACTIVATEAPP, TRUE, NULL );
//	}else{
//		::BringWindowToTop( hwndHtmlHelp );

//		/* �A�N�e�B�u�ɂ��� */
//		ActivateFrameWindow( hwndHtmlHelp );
//		::SetFocus( hwndHtmlHelp );
//		::SetActiveWindow( hwndHtmlHelp );
//		::PostMessage( hwndHtmlHelp, WM_ACTIVATEAPP, TRUE, NULL );
//	}

	//	Jul. 6, 2001 genta hwndHtmlHelp�̃`�F�b�N��ǉ�
	if( hwndHtmlHelp != NULL ){
		::BringWindowToTop( hwndHtmlHelp );
	}

	return;
}




//From Here Dec. 25, 2000 JEPRO
/* �o�[�W������� */
void CEditView::Command_ABOUT( void )
{
	CDlgAbout cDlgAbout;
	cDlgAbout.DoModal( m_hInstance, m_hWnd );
	return;
}
//To Here Dec. 25, 2000




/* �E�N���b�N���j���[ */
void CEditView::Command_MENU_RBUTTON( void )
{
	int			nId;
	char*		pszStr;
	int			nLength;
//	HGLOBAL		hgClip;
//	char*		pszClip;
	char*		pszWork;
	int			i;
	/* �|�b�v�A�b�v���j���[(�E�N���b�N) */
	nId = CreatePopUpMenu_R();
	if( 0 == nId ){
		return;
	}
	switch( nId ){
	case IDM_COPYDICINFO:
		pszStr = m_cTipWnd.m_cInfo.GetPtr( &nLength );

		pszWork = (char*)malloc( nLength + 1);
		memcpy( pszWork, pszStr, nLength );
		pszWork[nLength] = '\0' ;
		// �����ڂƓ����悤�ɁA\n �� CR+LF�֕ϊ�����
		for( i = 0; i < nLength ; ++i){
			if( pszWork[i] == '\\' && pszWork[i + 1] == 'n'){
				pszWork[i] = '\x0d' ;
				pszWork[i + 1] = '\x0a' ;
			}
		}
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		MySetClipboardData( pszWork, nLength, FALSE );
		free( pszWork );

//		pszStr = m_cTipWnd.m_cInfo.GetPtr( &nLength );
//		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
//		MySetClipboardData( pszStr, nLength, FALSE );

//		/* Windows�N���b�v�{�[�h�ɃR�s�[ */
//		hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, nLength + 1 );
//		pszClip = (char*)::GlobalLock( hgClip );
//		memcpy( pszClip, pszStr, nLength + 1 );
//		::GlobalUnlock( hgClip );
//		::OpenClipboard( m_hWnd );
//		::EmptyClipboard();
//		::SetClipboardData( CF_OEMTEXT, hgClip );
//		::CloseClipboard();
		break;

	case IDM_JUMPDICT:
		/* �L�[���[�h�����t�@�C�����J�� */
		if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp)		/* �L�[���[�h�����Z���N�g���g�p���� */	// 2006.04.10 fon
			TagJumpSub( m_pShareData->m_Types[m_pcEditDoc->GetDocumentType()].m_KeyHelpArr[m_cTipWnd.m_nSearchDict].m_szPath, m_cTipWnd.m_nSearchLine, 1, 0 );
		break;

	default:
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
//		HandleCommand( nId, TRUE, 0, 0, 0, 0 );
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nId, 0 ),  (LPARAM)NULL );
		break;
	}
	return;
}





//typedef BOOL (*LPSENDTEXTMAIL) ( const char*, long, const char*, const char*, const char*, const char*, const char*, const char*, const char*, const char*, long, const char**, long*, long, const char**, const char**, BOOL, BOOL, HWND, HWND, char* );







// From Here 2001.12.03 hor
/* �C���f���g ver2 */
void CEditView::Command_INDENT_TAB( void )
{
	if(!m_pcEditDoc->GetDocumentAttribute().m_bInsSpace){
		if(IsTextSelected() && m_nSelectLineFrom!=m_nSelectLineTo){
			Command_INDENT( TAB );
		}else{
			Command_CHAR( (char)TAB );
		}
		return;
	}
	if(IsTextSelected() && m_bBeginBoxSelect && m_nSelectColmFrom==m_nSelectColmTo){
		Command_INDENT( TAB );
		return;
	}
	int		nCol	=	0;
	//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
	int		nTab	=	m_pcEditDoc->m_cLayoutMgr.GetTabSpace();
	int		nSpace	=	0;

	//�C���f���g�J�n�ʒu�̎擾
	if ( IsTextSelected() ) {
		nCol = (m_nSelectColmFrom<m_nSelectColmTo)?m_nSelectColmFrom:m_nSelectColmTo;
	}else{
		nCol = m_nCaretPosX;
	}

	//�C���f���g�������擾
	nSpace = nTab-(nCol%nTab);
	if (nSpace==0) nSpace = nTab;

	//TAB���������X�y�[�X�C���f���g
	if (IsTextSelected() && m_nSelectLineFrom == m_nSelectLineTo ) {
		ReplaceData_CEditView(
			m_nSelectLineFrom,		/* �͈͑I���J�n�s */
			m_nSelectColmFrom,		/* �͈͑I���J�n�� */
			m_nSelectLineTo,		/* �͈͑I���I���s */
			m_nSelectColmTo,		/* �͈͑I���I���� */
			NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
			// Sep. 22, 2002 genta TAB�̍ő啝��64�Ɋg��
			"                                                                ",				/* �}������f�[�^ */
			nSpace,					/* �}������f�[�^�̒��� */
			TRUE
		);
		return;
	}
	// Sep. 22, 2002 genta TAB�̍ő啝��64�Ɋg��
	Command_INDENT( "                                                                " , nSpace , TRUE);

}
// To Here 2001.12.03 hor




/* �C���f���g ver1 */
void CEditView::Command_INDENT( char cChar )
{
	// From Here 2001.12.03 hor
	/* SPACEorTAB�C�����f���g�ŋ�`�I�������[���̎��͑I��͈͂��ő�ɂ��� */
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	if((cChar==SPACE || cChar==TAB) && m_bBeginBoxSelect && m_nSelectColmFrom==m_nSelectColmTo ){
		m_nSelectColmTo=m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
		RedrawAll();
		return;
	}
	// To Here 2001.12.03 hor
	char szWork[2];
	wsprintf( szWork, "%c", cChar );
	Command_INDENT( szWork, lstrlen( szWork ) );
	return;
}




/* �C���f���g ver0 */
//void CEditView::Command_INDENT( const char* pData, int nDataLen )	// 2001.12.03 hor
void CEditView::Command_INDENT( const char* pData, int nDataLen , BOOL bIndent )
{
	int			nSelectLineFromOld;	/* �͈͑I���J�n�s */
	int			nSelectColFromOld; 	/* �͈͑I���J�n�� */
	int			nSelectLineToOld;	/* �͈͑I���I���s */
	int			nSelectColToOld;	/* �͈͑I���I���� */
//	CMemory*	pcMemDeleted;
	CMemory		cMem;
	CWaitCursor cWaitCursor( m_hWnd );
	COpe*		pcOpe = NULL;
	int			nNewLine;			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int			nNewPos;			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	int			i;
//	HDC			hdc;
//	PAINTSTRUCT	ps;
//	char		szWork[16];
	CMemory		cmemBuf;
	RECT		rcSel;
	int			nPosX;
	int			nPosY;
	int			nIdxFrom;
	int			nIdxTo;
	int			nLineNum;
	int			nDelPos;
	int			nDelLen;
//	int			nDelPosNext; 2001/09/24
//	int			nDelLenNext;
	int*		pnKey_CharCharsArr;
	pnKey_CharCharsArr = NULL;

	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

	if( !IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
//		/* 1�o�C�g�������� */
		char*	pszWork;
		pszWork = new char[nDataLen + 1];
		memcpy( pszWork, pData, nDataLen );
		pszWork[nDataLen] = '\0';
		// �e�L�X�g��\��t�� 2004.05.14 Moca �����������ŗ^����
		Command_INSTEXT( TRUE, pszWork, nDataLen, FALSE );
		delete [] pszWork;
		return;
	}
	m_bDrawSWITCH=FALSE;	// 2002.01.25 hor
	/* ��`�͈͑I�𒆂� */
	if( m_bBeginBoxSelect ){
		// From Here 2001.12.03 hor
		/* �㏑���[�h�̂Ƃ��͑I��͈͍폜 */
		if( ! IsInsMode() /* Oct. 2, 2005 genta */){
			nSelectLineFromOld	= m_nSelectLineFrom;
			nSelectColFromOld	= m_nSelectColmFrom;
			nSelectLineToOld	= m_nSelectLineTo;
			nSelectColToOld		= m_nSelectColmTo;
			DeleteData( FALSE );
			m_nSelectLineFrom	= nSelectLineFromOld;
			m_nSelectColmFrom	= nSelectColFromOld;
			m_nSelectLineTo		= nSelectLineToOld;
			m_nSelectColmTo		= nSelectColToOld;
			m_bBeginBoxSelect	= TRUE;
		}
		// To Here 2001.12.03 hor

		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,		/* �͈͑I���J�n�s */
			m_nSelectColmFrom,		/* �͈͑I���J�n�� */
			m_nSelectLineTo,		/* �͈͑I���I���s */
			m_nSelectColmTo			/* �͈͑I���I���� */
		);
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( FALSE/*TRUE 2002.01.25 hor*/ );

		// From Here 2001.12.03 hor
		/* �C���f���g���͋󔒍s�ɏ����܂Ȃ� */
		if( 1 == nDataLen && ( SPACE == pData[0] || TAB == pData[0] ) ){
			bIndent=TRUE;
		}
		// To Here 2001.12.03 hor

		for( nLineNum = rcSel.top; nLineNum < rcSel.bottom + 1; nLineNum++ ){
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );
			//	Nov. 6, 2002 genta NULL�`�F�b�N�ǉ�
			//	���ꂪ�Ȃ���EOF�s���܂ދ�`�I�𒆂̕�������͂ŗ�����
			const char* pLine;
			if( pcLayout != NULL && NULL != (pLine = pcLayout->GetPtr()) ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom = LineColmnToIndex( pcLayout, rcSel.left );
				nIdxTo = LineColmnToIndex( pcLayout, rcSel.right );

				for( i = nIdxFrom; i <= nIdxTo; ++i ){
					if( pLine[i] == CR || pLine[i] == LF ){
						nIdxTo = i;
						break;
					}
				}
			}else{
				nIdxFrom = 0;
				nIdxTo = 0;
			}
			nDelPos = nIdxFrom;
			nDelLen = nIdxTo - nIdxFrom;

			/* TAB��X�y�[�X�C���f���g�̎� */
			if( bIndent && 0 == nDelLen ) {
				continue;
			}

			//	Nov. 6, 2002 genta pcLayout��NULL�̏ꍇ���l��
			nPosX = ( pcLayout == NULL ? 0 : LineIndexToColmn( pcLayout, nDelPos ));
			nPosY = nLineNum;
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				pcOpe = new COpe;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					nPosX,
					nPosY,
					&pcOpe->m_nCaretPosX_PHY_Before,
					&pcOpe->m_nCaretPosY_PHY_Before
				);
			}
			/* ���݈ʒu�Ƀf�[�^��}�� */
			InsertData_CEditView(
				rcSel.left/*nPosX*/,
				nPosY,
				pData,		// cmemBuf.GetPtr(),	// 2001.12.03 hor
				nDataLen,	// cmemBuf.GetLength(),		// 2001.12.03 hor
				&nNewLine,
				&nNewPos,
				pcOpe,
				FALSE
			);
			MoveCursor( nNewPos, nNewLine, FALSE );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
				pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
				/* ����̒ǉ� */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}
		//	nextline:;	// 2001.12.03 hor
		}
		/* �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ� */
		MoveCursor( rcSel.left, rcSel.top, FALSE );

		/* �}��������̏�� */
		CDocLineMgr::CreateCharCharsArr(
			(const unsigned char *)pData, nDataLen,
			&pnKey_CharCharsArr
		);
		for( i = 0; i < nDataLen; ){
			/* �J�[�\���E�ړ� */
			Command_RIGHT( FALSE, TRUE, FALSE );
			i+= pnKey_CharCharsArr[i];
		}
		if( NULL != pnKey_CharCharsArr ){
			delete [] pnKey_CharCharsArr;
		}
	// From Here 2001.12.03 hor 
		//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
		rcSel.right += m_nCaretPosX-rcSel.left;
		if( rcSel.right>m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() ){
			rcSel.right=m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
		}
	// To Here 2001.12.03 hor
		rcSel.left = m_nCaretPosX;

		/* �J�[�\�����ړ� */
		MoveCursor( rcSel.left, rcSel.top, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;

		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */

			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		m_nSelectLineFrom = rcSel.top;			/* �͈͑I���J�n�s */
		m_nSelectColmFrom = rcSel.left; 		/* �͈͑I���J�n�� */
		m_nSelectLineTo = rcSel.bottom;			/* �͈͑I���I���s */
	//	m_nSelectColmTo = nNewPos;				/* �͈͑I���I���� */	// 2001.12.03 hor
		m_nSelectColmTo = rcSel.right;			/* �͈͑I���I���� */	// 2001.12.03 hor
		m_bBeginBoxSelect = TRUE;
	}else{
		nSelectLineFromOld = m_nSelectLineFrom;	/* �͈͑I���J�n�s */
		nSelectColFromOld = 0;					/* �͈͑I���J�n�� */
		nSelectLineToOld = m_nSelectLineTo;		/* �͈͑I���I���s */
		if( m_nSelectColmTo > 0 ){
			++nSelectLineToOld;					/* �͈͑I���I���s */
		}
		nSelectColToOld = 0;					/* �͈͑I���I���� */

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( FALSE );

		for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
			int nLineCountPrev = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( i );
			if( NULL == pcLayout ||						//	�e�L�X�g������EOL�̍s�͖���
				pcLayout->m_nOffset > 0 ||				//	�܂�Ԃ��s�͖���
				pcLayout->GetLengthWithoutEOL() == 0 ){	//	���s�݂̂̍s�͖�������B
				continue;
			}

			/* �J�[�\�����ړ� */
			MoveCursor( 0, i, FALSE );
			m_nCaretPosX_Prev = m_nCaretPosX;

			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				pcOpe = new COpe;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					0,
					i,
					&pcOpe->m_nCaretPosX_PHY_Before,
					&pcOpe->m_nCaretPosY_PHY_Before
				);
			}
			/* ���݈ʒu�Ƀf�[�^��}�� */
			InsertData_CEditView(
				0,
				i,
				pData,		//	cmemBuf.GetPtr(),	// 2001.12.03 hor
				nDataLen,	//	cmemBuf.GetLength(),	// 2001.12.03 hor
				&nNewLine,
				&nNewPos,
				pcOpe,
				FALSE
			);
			/* �J�[�\�����ړ� */
			MoveCursor( nNewPos, nNewLine, FALSE );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
				pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
				/* ����̒ǉ� */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}

			if ( nLineCountPrev != m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
				//	�s�����ω�����!!
				nSelectLineToOld += m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}

		m_nSelectLineFrom = nSelectLineFromOld;	/* �͈͑I���J�n�s */
		m_nSelectColmFrom = nSelectColFromOld; 	/* �͈͑I���J�n�� */
		m_nSelectLineTo = nSelectLineToOld;		/* �͈͑I���I���s */
		m_nSelectColmTo = nSelectColToOld;		/* �͈͑I���I���� */
		// From Here 2001.12.03 hor
		MoveCursor( m_nSelectColmTo, m_nSelectLineTo, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		// To Here 2001.12.03 hor
	}
	/* �ĕ`�� */
	m_bDrawSWITCH=TRUE;	// 2002.01.25 hor
	Redraw();			// 2002.01.25 hor
	return;
}




/* �t�C���f���g */
void CEditView::Command_UNINDENT( char cChar )
{
	//	Aug. 9, 2003 genta
	//	�I������Ă��Ȃ��ꍇ�ɋt�C���f���g�����ꍇ��
	//	���Ӄ��b�Z�[�W���o��
	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �P�o�C�g�������� */
		Command_CHAR( cChar );	//	2003.10.09 zenryaku�x�����o�����C����͈ȑO�̂܂܂ɂ��� 
		SendStatusMessage("���t�C���f���g�͑I�����̂�");
		return;
	}

	int			nSelectLineFromOld;	/* �͈͑I���J�n�s */
	int			nSelectColFromOld; 	/* �͈͑I���J�n�� */
	int			nSelectLineToOld;	/* �͈͑I���I���s */
	int			nSelectColToOld;	/* �͈͑I���I���� */
	const char*	pLine;
	int			nLineLen;
	CMemory*	pcMemDeleted;
	CMemory		cMem;
	CWaitCursor cWaitCursor( m_hWnd );
	COpe*		pcOpe = NULL;
	int			i;
	CMemory		cmemBuf;

	//	Aug. 9, 2003 genta
	//	�d���폜
	//if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
	//	/* 1�o�C�g�������� */
	//	Command_CHAR( cChar );
	//	return;
	//}
	/* ��`�͈͑I�𒆂� */
	if( m_bBeginBoxSelect ){
		::MessageBeep( MB_ICONHAND );
//**********************************************
//	 ���^�t�C���f���g�ɂ��ẮA�ۗ��Ƃ��� (1998.10.22)
//**********************************************
	}else{
		m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta

		nSelectLineFromOld = m_nSelectLineFrom;	/* �͈͑I���J�n�s */
		nSelectColFromOld = 0;					/* �͈͑I���J�n�� */
		nSelectLineToOld = m_nSelectLineTo;		/* �͈͑I���I���s */
		if( m_nSelectColmTo > 0 ){
			nSelectLineToOld++;					/* �͈͑I���I���s */
		}
		nSelectColToOld = 0;					/* �͈͑I���I���� */

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( FALSE );

		int				nDelLen;
		for( i = nSelectLineFromOld; i < nSelectLineToOld; i++ ){
			int nLineCountPrev = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			const CLayout* pcLayout;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( i, &nLineLen, &pcLayout );
			if( NULL == pcLayout || pcLayout->m_nOffset > 0 ){
				continue;
			}
			if( TAB == cChar ){
				if( pLine[0] == cChar ){
					nDelLen = 1;
				}else{

					int i;
					for( i = 0; i < nLineLen; i++ ){
						if( SPACE != pLine[i] ){
							break;
						}
						//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
						if( i >= m_pcEditDoc->m_cLayoutMgr.GetTabSpace() ){
							break;
						}
					}
					if( 0 == i ){
						continue;
					}
					nDelLen = i;
				}
			}else{
				if( pLine[0] != cChar ){
					continue;
				}
				nDelLen = 1;
			}

			/* �J�[�\�����ړ� */
			MoveCursor( 0, i, FALSE );
			m_nCaretPosX_Prev = m_nCaretPosX;
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				pcOpe = new COpe;
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					0,
					i,
					&pcOpe->m_nCaretPosX_PHY_Before,
					&pcOpe->m_nCaretPosY_PHY_Before
				);
			}else{
				pcOpe = NULL;
			}
			pcMemDeleted = new CMemory;
			/* �w��ʒu�̎w�蒷�f�[�^�폜 */
			DeleteData2(
				0/*rcSel.left*/,
				i/*nLineNum + 1*/,
				nDelLen/*nLineLen*/,	// 2001.12.03 hor
				pcMemDeleted,
				pcOpe				/* �ҏW����v�f COpe */
//				FALSE,
//				FALSE
			);
			if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
					0,
					i,
					&pcOpe->m_nCaretPosX_PHY_After,
					&pcOpe->m_nCaretPosY_PHY_After
				);
				/* ����̒ǉ� */
				m_pcOpeBlk->AppendOpe( pcOpe );
			}else{
				delete pcMemDeleted;
				pcMemDeleted = NULL;
			}
			if ( nLineCountPrev != m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
				//	�s�����ω�����!!
				nSelectLineToOld += m_pcEditDoc->m_cLayoutMgr.GetLineCount() - nLineCountPrev;
			}
		}
		m_nSelectLineFrom = nSelectLineFromOld;	/* �͈͑I���J�n�s */
		m_nSelectColmFrom = nSelectColFromOld; 	/* �͈͑I���J�n�� */
		m_nSelectLineTo = nSelectLineToOld;		/* �͈͑I���I���s */
		m_nSelectColmTo = nSelectColToOld;		/* �͈͑I���I���� */
		// From Here 2001.12.03 hor
		MoveCursor( m_nSelectColmTo, m_nSelectLineTo, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;				/* ������ */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		// To Here 2001.12.03 hor
	}
	/* �ĕ`�� */
//	hdc = ::GetDC( m_hWnd );
//	ps.rcPaint.left = 0;
//	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
//	ps.rcPaint.top = m_nViewAlignTop;
//	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//	OnKillFocus();
//	OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//	OnSetFocus();
//	::ReleaseDC( m_hWnd, hdc );
	Redraw();	// 2002.01.25 hor
	return;
}

/* �Ō�Ƀe�L�X�g��ǉ� */
void CEditView::Command_ADDTAIL( const char* pszData, int nDataLen )
{
	int		nNewLine;					/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int		nNewPos;					/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*	pcOpe = NULL;

	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta
	/*�t�@�C���̍Ō�Ɉړ� */
	Command_GOFILEEND( FALSE );
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
		pcOpe = new COpe;
//		pcOpe->m_nCaretPosX_Before = m_nCaretPosX;	/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = m_nCaretPosY;	/* ����O�̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_Before,
//			pcOpe->m_nCaretPosY_Before,
//			&pcOpe->m_nCaretPosX_PHY_Before,
//			&pcOpe->m_nCaretPosY_PHY_Before
//		);
		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
	}
	/* ���݈ʒu�Ƀf�[�^��}�� */
	InsertData_CEditView(
		m_nCaretPosX,
		m_nCaretPosY,
		(char*)pszData,
		nDataLen,
		&nNewLine,
		&nNewPos,
		pcOpe,
		TRUE
	);
	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
	// Sep. 2, 2002 ���Ȃӂ� �A���_�[���C���̕\�����c���Ă��܂������C��
	MoveCursor( nNewPos, nNewLine, TRUE );
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//		pcOpe->m_nCaretPosX_After = m_nCaretPosX;			/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = m_nCaretPosY;			/* �����̃L�����b�g�ʒu�x */
//		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//			pcOpe->m_nCaretPosX_After,
//			pcOpe->m_nCaretPosY_After,
//			&pcOpe->m_nCaretPosX_PHY_After,
//			&pcOpe->m_nCaretPosY_PHY_After
//		);
		pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
		pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
		/* ����̒ǉ� */
		m_pcOpeBlk->AppendOpe( pcOpe );
	}
	return;
}




/*! �^�O�W�����v

	@param bClose [in] true:���E�B���h�E�����

	@date 2003.04.03 genta ���E�B���h�E����邩�ǂ����̈�����ǉ�
	@date 2004.05.13 Moca �s���ʒu�̎w�肪�����ꍇ�́A�s�����ړ����Ȃ�
*/
bool CEditView::Command_TAGJUMP( bool bClose )
{
	const char*	pLine;
	int			nLineLen;
	int			nJumpToLine;
	int			nJumpToColm;
	char		szJumpToFile[1024];
//	HWND		hwndOwner;
//	POINT		poCaret;
	int			nPathLen;
	int			nBgn;
	memset( szJumpToFile, 0, sizeof(szJumpToFile) );
	//	2004.05.13 Moca �����l��1�ł͂Ȃ����̈ʒu���p������悤��
	// 0�ȉ��͖��w�舵���B(1�J�n)
	nJumpToLine = 0;
	nJumpToColm = 0;
	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	int		nX;
	int		nY;
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nCaretPosX,
		m_nCaretPosY,
		&nX,
		&nY
	);
	/* ���ݍs�̃f�[�^���擾 */
	pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nY, &nLineLen );
	if( NULL == pLine ){
		goto can_not_tagjump_end;
	}
	/* WZ���̃^�O���X�g�� */
	if( 0 == memcmp( pLine, "�E", 2 )
	 || 0 == memcmp( pLine, "��\"", 3 )
	){
		if( 0 == memcmp( pLine, "��\"", 3 ) ){
			if( IsFilePath( &pLine[3], &nBgn, &nPathLen ) ){
				memcpy( szJumpToFile, &pLine[3 + nBgn], nPathLen );
				GetLineColm( &pLine[3] + nPathLen, &nJumpToLine, &nJumpToColm );
			}else{
				goto can_not_tagjump;
			}
		}else{
			GetLineColm( &pLine[2], &nJumpToLine, &nJumpToColm );
			nY--;

			for( ; 0 <= nY; nY-- ){
				pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nY, &nLineLen );
				if( NULL == pLine ){
					goto can_not_tagjump;
				}
				if( 0 == memcmp( pLine, "�E", 2 ) ){
					continue;
				}else
				if( 0 == memcmp( pLine, "��\"", 3 ) ){
					if( IsFilePath( &pLine[3], &nBgn, &nPathLen ) ){
						memcpy( szJumpToFile, &pLine[3 + nBgn], nPathLen );
						break;
					}else{
						goto can_not_tagjump;
					}
				}else{
					goto can_not_tagjump;
				}
			}
		}
	}else{
		//@@@ 2001.12.31 YAZAKI
		const char *p = pLine;
		const char *p_end = p + nLineLen;
#if 0
		//	Jan. 04, 2001 genta Directory��ΏۊO�ɂ����̂ŕ�����ɂ͏_��ɑΉ�
		if ( 0 == memcmp( pLine, "�x�� W", 6 ) ){
			p += 6;
			for( ; p < p_end && ( *p != ' ' ); ++p )
				;
		}else
		if ( 0 == memcmp( pLine, "�G���[ E", 8) ){
			p += 6;
			for( ; p < p_end && ( *p != ' ' ); ++p )
				;
		}else
		if ( 0 == memcmp( pLine, "�v���I�G���[ F", 14) ){
			p += 6;
			for( ; p < p_end && ( *p != ' ' ); ++p )
				;
		}else{
		}
#endif
		//	From Here Aug. 27, 2001 genta
		//	Borland �`���̃��b�Z�[�W�����TAG JUMP
		while( p < p_end ){
			//	skip space
			for( ; p < p_end && ( *p == ' ' || *p == '\t' || *p == '\n' ); ++p )
				;
			if( p >= p_end )
				break;
		
			//	Check Path
			if( IsFilePath( p, &nBgn, &nPathLen ) ){
				memcpy( szJumpToFile, &p[nBgn], nPathLen );
				GetLineColm( &p[nBgn + nPathLen], &nJumpToLine, &nJumpToColm );
				break;
			}
			//	Jan. 04, 2001 genta Directory��ΏۊO�ɂ����̂ŕ�����ɂ͏_��ɑΉ�
			//	break;	//@@@ 2001.12.31 YAZAKI �uworking ...�v���ɑΏ�
			//	skip non-space
			for( ; p < p_end && ( *p != ' ' && *p != '\t' ); ++p )
				;
		}
		if( szJumpToFile[0] == '\0' )
		{
			if( false == Command_TagJumpByTagsFile() )	//@@@ 2003.04.13
				goto can_not_tagjump;
			return true;
		}
		//	From Here Aug. 27, 2001 genta
	}

	//	Apr. 21, 2003 genta bClose�ǉ�
	if( false == TagJumpSub( szJumpToFile, nJumpToLine, nJumpToColm, bClose ) )	//@@@ 2003.04.13
		goto can_not_tagjump;

	return true;

can_not_tagjump:;
can_not_tagjump_end:;
//@@@ YAZAKI 2001.12.31 ���邳���B
//	::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
//		"�^�O�W�����v�ł��܂���B\n[%s]", szJumpToFile
//	);
	SendStatusMessage("�^�O�W�����v�ł��܂���");	//@@@ 2003.04.13
	return false;
}




/* �^�O�W�����v�o�b�N */
void CEditView::Command_TAGJUMPBACK( void )
{
// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	TagJump tagJump;

	/* �^�O�W�����v���̎Q�� */
	if( !CShareData::getInstance()->PopTagJump(&tagJump) || !CShareData::IsEditWnd(tagJump.hwndReferer) ){
		SendStatusMessage("�^�O�W�����v�o�b�N�ł��܂���");
		// 2004.07.10 Moca m_TagJumpNum��0�ɂ��Ȃ��Ă������Ǝv��
		// m_pShareData->m_TagJumpNum = 0;
		return;
	}

	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( tagJump.hwndReferer );

	/* �J�[�\�����ړ������� */
	memcpy( m_pShareData->m_szWork, (void*)&(tagJump.point), sizeof( tagJump.point ) );
	::SendMessage( tagJump.hwndReferer, MYWM_SETCARETPOS, 0, 0 );

#if 0
	HWND hwndReferer = m_pcEditDoc->m_hwndReferer;
	if( NULL == hwndReferer ){	/* �Q�ƌ��E�B���h�E */
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�^�O�W�����v�o�b�N�ł��܂���B\n�߂�悪����܂���B\n"
		);
		return;
	}
	/* �E�B���h�E���ҏW�E�B���h�E�̃t���[���E�B���h�E���ǂ������ׂ� */
	if( !CShareData::IsEditWnd( hwndReferer ) ){
		m_pcEditDoc->m_hwndReferer = NULL;
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�^�O�W�����v�o�b�N�ł��܂���B\n�߂�悪����܂���B\n"
		);
		return;
	}
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndReferer );
	if( 0 < m_pcEditDoc->m_nRefererLine ){
		/* �J�[�\�����ړ������� */
		POINT poCaret;
		poCaret.x = m_pcEditDoc->m_nRefererX;
		poCaret.y = m_pcEditDoc->m_nRefererLine;
		memcpy( m_pShareData->m_szWork, (void*)&poCaret, sizeof( poCaret ) );
		::SendMessage( hwndReferer, MYWM_SETCARETPOS, 0, 0 );
	}
#endif
	return;
}

/*
	�_�C���N�g�^�O�W�����v

	@author	MIK
	@date	2003.04.13	�V�K�쐬
	@date	2003.05.12	�t�H���_�K�w���l�����ĒT��
*/
bool CEditView::Command_TagJumpByTagsFile( void )
{
	CMemory	cmemKey;
	int		i;
	char	szCurrentPath[1024];	//�J�����g�t�H���_
	char	szTagFile[1024];		//�^�O�t�@�C��
	char	szLineData[1024];		//�s�o�b�t�@
	char	s[5][1024];
	int		n2;
	int		nRet;
	int		nMatch;						//��v��
	CDlgTagJumpList	cDlgTagJumpList;	//�^�O�W�����v���X�g
	FILE	*fp;
	bool	bNoTag = true;
	int		nLoop;

	//���݃J�[�\���ʒu�̃L�[���擾����B
	GetCurrentTextForSearch( cmemKey );
	if( 0 == cmemKey.GetLength() ) return false;	//�L�[���Ȃ��Ȃ�I���

	if( ! m_pcEditDoc->IsFilePathAvailable() ) return false;

	// �t�@�C�����ɉ����ĒT���񐔂����肷��
	strcpy( szCurrentPath, m_pcEditDoc->GetFilePath() );
	nLoop = CalcDirectoryDepth( szCurrentPath );

	if( nLoop <  0 ) nLoop =  0;
	if( nLoop > (_MAX_PATH/2) ) nLoop = (_MAX_PATH/2);	//\A\B\C...�̂悤�ȂƂ�1�t�H���_��2���������̂�...

		//�p�X���̂ݎ��o���B
		cDlgTagJumpList.SetFileName( szCurrentPath );
		szCurrentPath[ strlen( szCurrentPath ) - strlen( m_pcEditDoc->GetFileName() ) ] = '\0';

		for( i = 0; i <= nLoop; i++ )
		{
			//�^�O�t�@�C�������쐬����B
			wsprintf( szTagFile, "%s%s", szCurrentPath, TAG_FILENAME );

			//�^�O�t�@�C�����J���B
			fp = fopen( szTagFile, "r" );
			if( fp )
			{
				bNoTag = false;
				nMatch = 0;
				while( fgets( szLineData, sizeof( szLineData ), fp ) )
				{
					if( szLineData[0] <= '!' ) goto next_line;	//�R�����g�Ȃ�X�L�b�v
					//chop( szLineData );

					s[0][0] = s[1][0] = s[2][0] = s[3][0] = s[4][0] = '\0';
					n2 = 0;
					//	2004.06.04 Moca �t�@�C����/�p�X�ɃX�y�[�X���܂܂�Ă���Ƃ���
					//	�_�C���N�g�^�O�W�����v�Ɏ��s���Ă���
					//	sscanf ��%[^\t\r\n] �ŃX�y�[�X��ǂ݂Ƃ�悤�ɕύX
					//	@@ 2005.03.31 MIK TAG_FORMAT�萔��
					nRet = sscanf( szLineData, 
						TAG_FORMAT,	//tags�t�H�[�}�b�g
						s[0], s[1], &n2, s[3], s[4]
						);
					if( nRet < 4 ) goto next_line;
					if( n2 <= 0 ) goto next_line;	//�s�ԍ��s��(-excmd=n���w�肳��ĂȂ�����)

					if( 0 != strcmp( s[0], cmemKey.GetPtr() ) ) goto next_line;

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
					if( ! cDlgTagJumpList.DoModal( m_hInstance, m_hWnd, (LPARAM)0 ) ) 
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
					char	*p;
					p = s[1];
					if( p[0] == '\\' )	//�h���C�u�Ȃ���΃p�X���H
					{
						//	2003.09.20 Moca �p�X����
						if( p[1] == '\\' )	//�l�b�g���[�N�p�X���H
						{
							strcpy( szTagFile, p );	//�������H���Ȃ��B
						}
						else
						{
							//�h���C�u���H�����ق����悢�H
							strcpy( szTagFile, p );	//�������H���Ȃ��B
						}
					}
					else if( isalpha( p[0] ) && p[1] == ':' )	//��΃p�X���H
					{
						strcpy( szTagFile, p );	//�������H���Ȃ��B
					}
					else
					{
						wsprintf( szTagFile, "%s%s", szCurrentPath, p );
					}

					return TagJumpSub( szTagFile, n2, 0 );
				}
			}	//fp

			//�J�����g�p�X��1�K�w��ցB
			strcat( szCurrentPath, "..\\" );
		}

	return false;
}

/*
	�w��t�@�C���̎w��ʒu�Ƀ^�O�W�����v����B

	@param nJumpToLine [in] �_���s�ԍ�(1�J�n)�B0�ȉ����w�肵����s�W�����v�͂��Ȃ��B
	@param nJumpToColm [in] �_���s�P�ʂ̍s���̈ʒu(1�J�n)
	@param bClose [in] true: ���E�B���h�E����� / false: ���E�B���h�E����Ȃ�

	@author	MIK
	@date	2003.04.13	�V�K�쐬
	@date	2003.04.21 genta bClose�ǉ�
	@date	2004.05.29 Moca 0�ȉ����w�肳�ꂽ�Ƃ��́A�P������
*/
bool CEditView::TagJumpSub( const char *pszFileName, int nJumpToLine, int nJumpToColm, bool bClose )
{
	HWND	hwndOwner;
	POINT	poCaret;
	char	szJumpToFile[1024];
	char	szWork[1024];
	// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
	TagJump	tagJump;

	// �Q�ƌ��E�B���h�E�ۑ�
	tagJump.hwndReferer = m_pcEditDoc->m_hwndParent;

	strcpy( szJumpToFile, pszFileName );

	/* �����O�t�@�C�������擾���� */
	if( TRUE == ::GetLongFileName( szJumpToFile, szWork ) )
	{
		strcpy( szJumpToFile, szWork );
	}

	//	Apr. 2003 genta ���邩�ǂ����͈����ɂ��
	//	grep���ʂ���Enter�ŃW�����v����Ƃ����Ctrl����ړ�
	if( bClose )
	{
		Command_WINCLOSE();	//	���킷�邾���B
	}

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
// 2004/07/05 �݂��΂�
// ����t�@�C������SendMesssage�� m_nCaretPosX,m_nCaretPosY���X�V����Ă��܂��A
// �W�����v��̏ꏊ���W�����v���Ƃ��ĕۑ�����Ă��܂��Ă���̂ŁA
// ���̑O�ŕۑ�����悤�ɕύX�B

	/* �J�[�\���ʒu�ϊ� */
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nCaretPosX,
		m_nCaretPosY,
		(int*)&tagJump.point.x,
		(int*)&tagJump.point.y
	);

	// �^�O�W�����v���̕ۑ�
	CShareData::getInstance()->PushTagJump(&tagJump);


	/* �w��t�@�C�����J����Ă��邩���ׂ� */
	/* �J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h�����Ԃ� */
	/* �t�@�C�����J���Ă��邩 */
	if( CShareData::getInstance()->IsPathOpened( (const char*)szJumpToFile, &hwndOwner ) )
	{
		// 2004.05.13 Moca �}�C�i�X�l�͖���
		if( 0 < nJumpToLine ){
			/* �J�[�\�����ړ������� */
			poCaret.y = nJumpToLine - 1;
			if( 0 < nJumpToColm ){
				poCaret.x = nJumpToColm - 1;
			}else{
				poCaret.x = 0;
			}
			memcpy( m_pShareData->m_szWork, (void*)&poCaret, sizeof(poCaret) );
			::SendMessage( hwndOwner, MYWM_SETCARETPOS, 0, 0 );
		}
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndOwner );
	}
	else
	{
		/* �V�����J�� */
		FileInfo	inf;
		bool		bSuccess;

		strcpy( inf.m_szPath, szJumpToFile );
		inf.m_nX           = nJumpToColm - 1;
		inf.m_nY           = nJumpToLine - 1;
		inf.m_nViewLeftCol = inf.m_nViewTopLine = -1;
		inf.m_nCharCode    = CODE_AUTODETECT;

		bSuccess = CEditApp::OpenNewEditor2(
			m_hInstance,
			m_pShareData->m_hwndTray,
			&inf,
			FALSE,	/* �ǂݎ���p�� */
			true	//	�������[�h�ŊJ��
		);

		if( ! bSuccess )	//	�t�@�C�����J���Ȃ�����
			return false;

		//	Apr. 23, 2001 genta
		//	hwndOwner�ɒl������Ȃ��Ȃ��Ă��܂������߂�
		//	Tag Jump Back�����삵�Ȃ��Ȃ��Ă����̂��C��
		if( FALSE == CShareData::getInstance()->IsPathOpened( (const char*)szJumpToFile, &hwndOwner ) )
			return false;
	}

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
#if 0
	/* �^�O�W�����v���ʒm */
	memcpy( m_pShareData->m_szWork, (void*)&poCaret, sizeof( poCaret ) );
	::SendMessage( hwndOwner, MYWM_SETREFERER, (WPARAM)(m_pcEditDoc->m_hwndParent), 0 );
#endif

	return true;
}

/*
	�^�O�t�@�C�����쐬����B

	@author	MIK
	@date	2003.04.13	�V�K�쐬
	@date	2003.05.12	�_�C�A���O�\���Ńt�H���_�����ׂ����w��ł���悤�ɂ����B
*/
bool CEditView::Command_TagsMake( void )
{
#define	CTAGS_COMMAND	"ctags.exe"

	char	szTargetPath[1024 /*_MAX_PATH+1*/ ];
	if( m_pcEditDoc->IsFilePathAvailable() )
	{
		strcpy( szTargetPath, m_pcEditDoc->GetFilePath() );
		szTargetPath[ strlen( szTargetPath ) - strlen( m_pcEditDoc->GetFileName() ) ] = '\0';
	}
	else
	{
		char	szTmp[1024];
		::GetModuleFileName(
			::GetModuleHandle( NULL ),
			szTmp, sizeof( szTmp )
		);
		/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
		/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
		::SplitPath_FolderAndFile( szTmp, szTargetPath, NULL );
	}

	//�_�C�A���O��\������
	CDlgTagsMake	cDlgTagsMake;
	if( FALSE == cDlgTagsMake.DoModal( m_hInstance, m_hWnd, (LPARAM)0, szTargetPath ) ) return false;

/*
	{
		char	*msg;
		char	szTmp[1024];
		wsprintf( szTmp, "%s%s", cDlgTagsMake.m_szPath, TAG_FILENAME );
		if( -1 == ::GetFileAttributes( szTmp ) ) 
			msg = "�_�C���N�g�^�O�W�����v�p�̃^�O�t�@�C�����쐬���܂����H";
		else
			msg = "�^�O�t�@�C���͑��݂��܂��B\n\n�_�C���N�g�^�O�W�����v�p�̃^�O�t�@�C�����č쐬���܂����H";
		
		int	nRet;
		nRet = MYMESSAGEBOX(
			m_hWnd,
			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
			"�^�O�t�@�C���̍쐬",
			msg
			);
		if( IDYES != nRet ) return false; 
	}
*/

	char	cmdline[1024];
	/* exe�̂���t�H���_ */
	char	szPath[_MAX_PATH + 1];
	char	szExeFolder[_MAX_PATH + 1];
	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, sizeof( szPath )
	);
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, szExeFolder, NULL );

	//ctags.exe�̑��݃`�F�b�N
	wsprintf( cmdline, "%s\\%s", szExeFolder, CTAGS_COMMAND );
	if( -1 == ::GetFileAttributes( cmdline ) )
	{
		::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
			_T( "�^�O�쐬�R�}���h���s�͎��s���܂����B\n\nCTAGS.EXE ��������܂���B" ) );
		return false;
	}

	HANDLE	hStdOutWrite, hStdOutRead;
	CDlgCancel	cDlgCancel;
	CWaitCursor	cWaitCursor( m_hWnd );

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );

	//�q�v���Z�X�̕W���o�͂Ɛڑ�����p�C�v���쐬
	SECURITY_ATTRIBUTES	sa;
	ZeroMemory( &sa, sizeof(SECURITY_ATTRIBUTES) );
	sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
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
	ZeroMemory( &sui, sizeof(STARTUPINFO) );
	sui.cb          = sizeof(STARTUPINFO);
	sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sui.wShowWindow = SW_HIDE;
	sui.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
	sui.hStdOutput  = hStdOutWrite;
	sui.hStdError   = hStdOutWrite;

	//	To Here Dec. 28, 2002 MIK

	char	options[1024];
	strcpy( options, "--excmd=n" );	//�f�t�H���g�̃I�v�V����
	if( cDlgTagsMake.m_nTagsOpt & 0x0001 ) strcat( options, " -R" );	//�T�u�t�H���_���Ώ�
	if( strlen( cDlgTagsMake.m_szTagsCmdLine ) )	//�ʎw��̃R�}���h���C��
	{
		strcat( options, " " );
		strcat( options, cDlgTagsMake.m_szTagsCmdLine );
	}
	strcat( options, " *" );	//�z���̂��ׂẴt�@�C��

	//OS�o�[�W�����擾
	COsVersionInfo cOsVer;
	//�R�}���h���C��������쐬(MAX:1024)
	if (cOsVer.IsWin32NT())
	{
		//	2006.08.04 genta add /D to disable autorun
		wsprintf( cmdline, "cmd.exe /D /C \"\"%s\\%s\" %s\"",
				szExeFolder,	//sakura.exe�p�X
				CTAGS_COMMAND,	//ctags.exe
				options			//ctags�I�v�V����
			);
	}
	else
	{
		wsprintf( cmdline, "command.com /C \"%s\\%s\" %s",
				szExeFolder,	//sakura.exe�p�X
				CTAGS_COMMAND,	//ctags.exe
				options			//ctags�I�v�V����
			);
	}

	//�R�}���h���C�����s
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
			CREATE_NEW_CONSOLE, NULL, cDlgTagsMake.m_szPath, &sui, &pi ) == FALSE )
	{
		::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
			"�^�O�쐬�R�}���h���s�͎��s���܂����B\n\n%s", cmdline );
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
		hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );
		hwndMsg = ::GetDlgItem( hwndCancel, IDC_STATIC_CMD );
		::SendMessage( hwndMsg, WM_SETTEXT, 0, (LPARAM)"�^�O�t�@�C�����쐬���ł��B" );

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
					if( !::BlockingHook( cDlgCancel.m_hWnd ) ){
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
					if( new_cnt >= sizeof(work) - 2 )							//�p�C�v����ǂݏo���ʂ𒲐�
					{
						new_cnt = sizeof(work) - 2;
					}
					ReadFile( hStdOutRead, &work[0], new_cnt, &read_cnt, NULL );	//�p�C�v����ǂݏo��
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

						work[ read_cnt ] = '\0';	// Nov. 15, 2003 genta �\���p��0�I�[����
						::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
						"�^�O�쐬�R�}���h���s�͎��s���܂����B\n\n%s", work ); // 2003.11.09 ���イ��

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

	::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
		"�^�O�t�@�C���̍쐬���I�����܂����B" );

	return true;
}

/*!
	�L�[���[�h���w�肵�ă^�O�W�����v

	@author MIK
	@date 2005.03.31 �V�K�쐬
*/
bool CEditView::Command_TagJumpByTagsFileKeyword( const char* keyword )
{
	CMemory	cmemKey;
	CDlgTagJumpList	cDlgTagJumpList;
	char	s[5][1024];
	int		n2;
	int depth;
	char	szTagFile[1024];		//�^�O�t�@�C��
	char	szCurrentPath[1024];

	if( ! m_pcEditDoc->IsFilePathAvailable() ) return false;
	strcpy( szCurrentPath, m_pcEditDoc->GetFilePath() );

	cDlgTagJumpList.SetFileName( szCurrentPath );
	cDlgTagJumpList.SetKeyword( keyword );

	szCurrentPath[ strlen( szCurrentPath ) - strlen( m_pcEditDoc->GetFileName() ) ] = '\0';

	if( ! cDlgTagJumpList.DoModal( m_hInstance, m_hWnd, (LPARAM)1 ) ) 
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
	char	*p;
	p = s[1];
	if( p[0] == '\\' )	//�h���C�u�Ȃ���΃p�X���H
	{
		if( p[1] == '\\' )	//�l�b�g���[�N�p�X���H
		{
			strcpy( szTagFile, p );	//�������H���Ȃ��B
		}
		else
		{
			//�h���C�u���H�����ق����悢�H
			strcpy( szTagFile, p );	//�������H���Ȃ��B
		}
	}
	else if( isalpha( p[0] ) && p[1] == ':' )	//��΃p�X���H
	{
		strcpy( szTagFile, p );	//�������H���Ȃ��B
	}
	else
	{
		for( int i = 0; i < depth; i++ )
		{
			strcat( szCurrentPath, "..\\" );
		}
		wsprintf( szTagFile, "%s%s", szCurrentPath, p );
	}

	return TagJumpSub( szTagFile, n2, 0 );
}



/* C/C++�w�b�_�t�@�C�� �I�[�v���@�\ */		//Feb. 10, 2001 jepro	�������u�C���N���[�h�t�@�C���v����ύX
//BOOL CEditView::Command_OPENINCLUDEFILE( BOOL bCheckOnly )
BOOL CEditView::Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca �w�b�_�E�\�[�X�̃R�[�h�𓝍����폜
	static const char* source_ext[] = { "c", "cpp", "cxx", "cc", "cp", "c++" };
	static const char* header_ext[] = { "h", "hpp", "hxx", "hh", "hp", "h++" };
	return OPEN_ExtFromtoExt( bCheckOnly, bBeepWhenMiss, source_ext, header_ext,
		sizeof(source_ext) / sizeof(source_ext[0]), sizeof(header_ext) / sizeof(header_ext[0]),
		"C/C++�w�b�_�t�@�C���̃I�[�v���Ɏ��s���܂����B" );
}




/* C/C++�\�[�X�t�@�C�� �I�[�v���@�\ */
//BOOL CEditView::Command_OPENCCPP( BOOL bCheckOnly )	//Feb. 10, 2001 JEPRO	�R�}���h�����኱�ύX
BOOL CEditView::Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss )
{
	// 2003.06.28 Moca �w�b�_�E�\�[�X�̃R�[�h�𓝍����폜
	static const char* source_ext[] = { "c", "cpp", "cxx", "cc", "cp", "c++" };
	static const char* header_ext[] = { "h", "hpp", "hxx", "hh", "hp", "h++" };
	return OPEN_ExtFromtoExt( bCheckOnly, bBeepWhenMiss, header_ext, source_ext,
		sizeof(header_ext) / sizeof(header_ext[0]), sizeof(source_ext) / sizeof(source_ext[0]),
		"C/C++�\�[�X�t�@�C���̃I�[�v���Ɏ��s���܂����B");
}


/*! �w��g���q�̃t�@�C���ɑΉ�����t�@�C�����J���⏕�֐�

	@param bCheckOnly [in]		true: �`�F�b�N�̂ݍs���ăt�@�C���͊J���Ȃ�
	@param bBeepWhenMiss [in]	true: �t�@�C�����J���Ȃ������ꍇ�Ɍx�������o��
	@param file_ext [in]		�����ΏۂƂ���g���q
	@param file_extno [in]		�����Ώۊg���q���X�g�̗v�f��
	@param open_ext [in]		�J���ΏۂƂ���g���q
	@param open_extno [in]		�J���Ώۊg���q���X�g�̗v�f��
	@param errmes [in]			�t�@�C�����J���Ȃ������ꍇ�ɕ\������G���[���b�Z�[�W

	@date 2003.06.28 Moca �w�b�_�E�\�[�X�t�@�C���I�[�v���@�\�̃R�[�h�𓝍�
*/
BOOL CEditView::OPEN_ExtFromtoExt( BOOL bCheckOnly, BOOL bBeepWhenMiss,
	const char* file_ext[], const char* open_ext[], int file_extno, int open_extno,
	const char* errmes )
{
//From Here Feb. 7, 2001 JEPRO �ǉ�
	int		i;
	BOOL	bwantopen_c;
//To Here Feb. 7, 2001

	/* �ҏW���t�@�C���̊g���q�𒲂ׂ� */
//Feb. 7, 2001 JEPRO ����ł��R�����g�A�E�g
//	if( CheckEXT( m_pcEditDoc->GetFilePath(), "h" ) ){
//	}else{
//		if( !bCheckOnly ){
//			::MessageBeep( MB_ICONHAND );
//		}
//		return FALSE;
//	}

//From Here Feb. 7, 2001 JEPRO �ǉ�
	for( i = 0; i < open_extno; i++ ){
		if( CheckEXT( m_pcEditDoc->GetFilePath(), open_ext[i] ) ){
			bwantopen_c = TRUE;
			goto open_c;
		}
	}
	if( bBeepWhenMiss ){
		::MessageBeep( MB_ICONHAND );
	}
	return FALSE;

open_c:;
//To Here Feb. 7, 2001

	char	szPath[_MAX_PATH];
	char	szDrive[_MAX_DRIVE];
	char	szDir[_MAX_DIR];
	char	szFname[_MAX_FNAME];
	char	szExt[_MAX_EXT];
	HWND	hwndOwner;

	_splitpath( m_pcEditDoc->GetFilePath(), szDrive, szDir, szFname, szExt );
//Feb. 7, 2001 JEPRO ����ł��R�����g�A�E�g
//	_makepath( szPath, szDrive, szDir, szFname, "c" );
//	if( -1 == _access( (const char *)szPath, 0 ) ){
//		_makepath( szPath, szDrive, szDir, szFname, "cpp" );
//		if( -1 == _access( (const char *)szPath, 0 ) ){
//			_makepath( szPath, szDrive, szDir, szFname, "cxx" );
//			if( -1 == _access( (const char *)szPath, 0 ) ){
//				if( !bCheckOnly ){
//					::MessageBeep( MB_ICONHAND );
//				}
//				return FALSE;
//			}
//		}
//	}
//	if( bCheckOnly ){
//		return TRUE;
//	}

//From Here Feb. 7, 2001 JEPRO �ǉ�
	for( i = 0; i < file_extno; i++ ){
		_makepath( szPath, szDrive, szDir, szFname, file_ext[i] );
		if( -1 == _access( (const char *)szPath, 0 ) ){
			if( i < file_extno - 1 )
				continue;
			if( bBeepWhenMiss ){
				::MessageBeep( MB_ICONHAND );
			}
			return FALSE;
		}
		break;
	}
	if( bCheckOnly ){
		return TRUE;
	}
//To Here Feb. 7, 2001

	/* �w��t�@�C�����J����Ă��邩���ׂ� */
	/* �J����Ă���ꍇ�͊J���Ă���E�B���h�E�̃n���h�����Ԃ� */
	/* �t�@�C�����J���Ă��邩 */
	if( CShareData::getInstance()->IsPathOpened( (const char*)szPath, &hwndOwner ) ){
	}else{
		/* �V�����J�� */
		char	szPath2[_MAX_PATH + 3];
		if( strchr( szPath, ' ' ) ){
			wsprintf( szPath2, "\"%s\"", szPath );
		}else{
			strcpy( szPath2, szPath );
		}
		/* �����R�[�h�͂��̃t�@�C���ɍ��킹�� */
		CEditApp::OpenNewEditor(
			m_hInstance,
			m_pShareData->m_hwndTray,
			szPath2,
			m_pcEditDoc->m_nCharCode,
			FALSE,	/* �ǂݎ���p�� */
			true
		);
		/* �t�@�C�����J���Ă��邩 */
		if( CShareData::getInstance()->IsPathOpened( (const char*)szPath, &hwndOwner ) ){
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				"%s\n\n%s\n\n", errmes, szPath
			);
			return FALSE;
		}
	}
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndOwner );

// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
// 2004/07/09 genta/Moca �^�O�W�����v�o�b�N�̓o�^����菜����Ă������A
//            ������ł��]���ǂ���o�^����
	TagJump	tagJump;
	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nCaretPosX,
		m_nCaretPosY,
		(int*)&tagJump.point.x,
		(int*)&tagJump.point.y
	);
	tagJump.hwndReferer = m_pcEditDoc->m_hwndParent;
	// �^�O�W�����v���̕ۑ�
	CShareData::getInstance()->PushTagJump(&tagJump);
#if 0
	/* �^�O�W�����v���ʒm */
	memcpy( m_pShareData->m_szWork, (void*)&poCaret, sizeof( poCaret ) );
	::SendMessage( hwndOwner, MYWM_SETREFERER, (WPARAM)(m_pcEditDoc->m_hwndParent), 0 );
#endif
	return TRUE;
}




//From Here Feb. 10, 2001 JEPRO �ǉ�
/* C/C++�w�b�_�t�@�C���܂��̓\�[�X�t�@�C�� �I�[�v���@�\ */
BOOL CEditView::Command_OPEN_HfromtoC( BOOL bCheckOnly )
{
	if ( Command_OPEN_HHPP( bCheckOnly, FALSE ) )	return TRUE;
	if ( Command_OPEN_CCPP( bCheckOnly, FALSE ) )	return TRUE;
	::MessageBeep( MB_ICONHAND );
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
*/
void CEditView::Command_BIND_WINDOW( void )
{
	//�^�u���[�h�ł���Ȃ��
	if (m_pShareData->m_Common.m_bDispTabWnd)
	{
		//�^�u�E�B���h�E�̐ݒ��ύX
		m_pShareData->m_Common.m_bDispTabWndMultiWin = !m_pShareData->m_Common.m_bDispTabWndMultiWin;

		// 2006.02.07 ryoji �E�C���h�E�����X�V����
		m_pShareData->m_TabWndWndpl.length = sizeof( m_pShareData->m_TabWndWndpl );
		::GetWindowPlacement( m_pcEditDoc->m_pcEditWnd->m_hWnd, &(m_pShareData->m_TabWndWndpl) );

		//Start 2004.08.27 Kazika �ύX
		//�^�u�E�B���h�E�̐ݒ��ύX���u���[�h�L���X�g����
		CShareData::getInstance()->PostMessageToAllEditors(
			MYWM_TAB_WINDOW_NOTIFY,						//�^�u�E�B���h�E�C�x���g
			(WPARAM)((m_pShareData->m_Common.m_bDispTabWndMultiWin) ? TWNT_MODE_DISABLE : TWNT_MODE_ENABLE),//�^�u���[�h�L��/�������C�x���g
			(LPARAM)m_pcEditDoc->m_pcEditWnd->m_hWnd,	//CEditWnd�̃E�B���h�E�n���h��
			m_hWnd);									//�������g
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
*/
void CEditView::Command_CASCADE( void )
{
	int i;

	//�^�u�E�C���h�E���͋֎~	//@@@ 2003.06.12 MIK
	//Start 2004.07.15 Kazika �^�u�E�B���h�E���̓^�u���[�h���������Ď��s
	if( TRUE  == m_pShareData->m_Common.m_bDispTabWnd
	 && FALSE == m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		//�^�u�E�B���h�E���܂Ƃ߂Ȃ��ݒ�ɕύX
		m_pShareData->m_Common.m_bDispTabWndMultiWin = TRUE;
	}
	//End 2004.07.15 Kazika

	/* ���݊J���Ă���ҏW���̃��X�g���擾���� */
	EditNode*	pEditNodeArr;
	int			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/ );

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
			if( ::IsIconic( pEditNodeArr[i].m_hWnd ) ){	//	�ŏ������Ă���E�B���h�E�͖����B
				if( !::IsWindowVisible( pEditNodeArr[i].m_hWnd ) )	// 2006.02.06 ryoji �����������Ă���
					::ShowWindow( pEditNodeArr[i].m_hWnd, SW_SHOWNA );
				continue;
			}
			//	Mar. 20, 2004 genta
			//	���݂̃E�B���h�E�𖖔��Ɏ����Ă������߂����ł̓X�L�b�v
			if( pEditNodeArr[i].m_hWnd == m_pcEditDoc->m_hwndParent ){
				current_win_index = i;
				continue;
			}
			pWndArr[count].hWnd = pEditNodeArr[i].m_hWnd;
			count++;
		}

		//	Mar. 20, 2004 genta
		//	���݂̃E�B���h�E�𖖔��ɑ}�� inspired by crayonzen
		if( current_win_index >= 0 ){
			pWndArr[count].hWnd = pEditNodeArr[current_win_index].m_hWnd;
			count++;
		}

		//	�f�X�N�g�b�v�T�C�Y�𓾂�
		RECT	rcDesktop;
		//	May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect( m_hWnd, &rcDesktop );
		
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
void CEditView::Command_TILE_H( void )
{
	int i;

	//�^�u�E�C���h�E���͋֎~	//@@@ 2003.06.12 MIK
	//Start 2004.07.15 Kazika �^�u�E�B���h�E���̓^�u���[�h���������Ď��s
	if( TRUE  == m_pShareData->m_Common.m_bDispTabWnd
	 && FALSE == m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		//�^�u�E�B���h�E���܂Ƃ߂Ȃ��ݒ�ɕύX
		m_pShareData->m_Common.m_bDispTabWndMultiWin = TRUE;
	}
	//End 2004.07.15 Kazika

	/* ���݊J���Ă���ҏW���̃��X�g���擾���� */
	EditNode*	pEditNodeArr;
	int			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/ );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	�f�X�N�g�b�v�T�C�Y�𓾂�
		RECT	rcDesktop;
		//	May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect( m_hWnd, &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].m_hWnd ) ){	//	�ŏ������Ă���E�B���h�E�͖����B
				if( !::IsWindowVisible( pEditNodeArr[i].m_hWnd ) )	// 2006.02.06 ryoji �����������Ă���
					::ShowWindow( pEditNodeArr[i].m_hWnd, SW_SHOWNA );
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	���݂̃E�B���h�E��擪�Ɏ����Ă���
			if( pEditNodeArr[i].m_hWnd == m_pcEditDoc->m_hwndParent ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = m_pcEditDoc->m_hwndParent;
			}
			else {
				phwndArr[count] = pEditNodeArr[i].m_hWnd;
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
void CEditView::Command_TILE_V( void )
{
	int i;

	//�^�u�E�C���h�E���͋֎~	//@@@ 2003.06.12 MIK
	//Start 2004.07.15 Kazika �^�u�E�B���h�E���̓^�u���[�h���������Ď��s
	if( TRUE  == m_pShareData->m_Common.m_bDispTabWnd
	 && FALSE == m_pShareData->m_Common.m_bDispTabWndMultiWin )// return;
	{
		//�^�u�E�B���h�E���܂Ƃ߂Ȃ��ݒ�ɕύX
		m_pShareData->m_Common.m_bDispTabWndMultiWin = TRUE;
	}
	//End 2004.07.15 Kazika

	/* ���݊J���Ă���ҏW���̃��X�g���擾���� */
	EditNode*	pEditNodeArr;
	int			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE/*FALSE*/ );

	if( nRowNum > 0 ){
		HWND*	phwndArr = new HWND[nRowNum];
		int		count = 0;
		//	�f�X�N�g�b�v�T�C�Y�𓾂�
		RECT	rcDesktop;
		//	May 01, 2004 genta �}���`���j�^�Ή�
		::GetMonitorWorkRect( m_hWnd, &rcDesktop );
		for( i = 0; i < nRowNum; ++i ){
			if( ::IsIconic( pEditNodeArr[i].m_hWnd ) ){	//	�ŏ������Ă���E�B���h�E�͖����B
				if( !::IsWindowVisible( pEditNodeArr[i].m_hWnd ) )	// 2006.02.06 ryoji �����������Ă���
					::ShowWindow( pEditNodeArr[i].m_hWnd, SW_SHOWNA );
				continue;
			}
			//	From Here Jul. 28, 2002 genta
			//	���݂̃E�B���h�E��擪�Ɏ����Ă���
			if( pEditNodeArr[i].m_hWnd == m_pcEditDoc->m_hwndParent ){
				phwndArr[count] = phwndArr[0];
				phwndArr[0] = m_pcEditDoc->m_hwndParent;
			}
			else {
				phwndArr[count] = pEditNodeArr[i].m_hWnd;
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
void CEditView::Command_MAXIMIZE_V( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;
	hwndFrame = ::GetParent( m_hwndParent );
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
void CEditView::Command_MAXIMIZE_H( void )
{
	HWND	hwndFrame;
	RECT	rcOrg;
	RECT	rcDesktop;

	hwndFrame = ::GetParent( m_hwndParent );
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
void CEditView::Command_MINIMIZE_ALL( void )
{
	HWND*	phWndArr;
	int		i;
	int		j;
	j = m_pShareData->m_nEditArrNum;
	if( 0 == j ){
		return;
	}
	phWndArr = new HWND[j];
	for( i = 0; i < j; ++i ){
		phWndArr[i] = m_pShareData->m_pEditArr[i].m_hWnd;
	}
	for( i = 0; i < j; ++i ){
		if( CShareData::IsEditWnd( phWndArr[i] ) )
		{
			if( TRUE  == m_pShareData->m_Common.m_bDispTabWnd
			 && FALSE == m_pShareData->m_Common.m_bDispTabWndMultiWin )	//@@@ 2003.06.13 MIK
			{
				if( ::IsWindowVisible( phWndArr[i] ) )
					::ShowWindow( phWndArr[i], SW_MINIMIZE );
				else
					::ShowWindow( phWndArr[i], SW_HIDE );
			}
			else
			{
				::ShowWindow( phWndArr[i], SW_MINIMIZE );
			}
		}
	}
	delete [] phWndArr;
	return;
}




//�u��(�u���_�C�A���O)
void CEditView::Command_REPLACE_DIALOG( void )
{
	CMemory		cmemCurText;
	BOOL		bSelected = FALSE;

// From Here 2001.12.03 hor
//		1.����������̏������W�b�N��Command_SEARCH_DIALOG�Ɠ������̂�
//		2.�I���󋵂ɂ���Ēu���_�C���O�͈̔͑I����ؑւ�
//
//	/* ����������������� */
//	m_pcEditDoc->m_cDlgReplace.m_szText[0] = '\0';
//	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
//		/* �I��͈͂̃f�[�^���擾 */
//		if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE ) ){
//			/* ��������������݈ʒu�̒P��ŏ����� */
//			strncpy( m_pcEditDoc->m_cDlgReplace.m_szText, cmemCurText.GetPtr(), _MAX_PATH - 1 );
//			m_pcEditDoc->m_cDlgReplace.m_szText[_MAX_PATH - 1] = '\0';
//		}
//		/* ��`�͈͑I�𒆂� */
//		if( m_bBeginBoxSelect ){
//			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
//			DisableSelectArea( TRUE );
//			bSelected = FALSE;
//		}else{
//			bSelected = TRUE;
//		}
//	}else{
//		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
//		if( NULL != pLine ){
//			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
//			nIdx = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
//			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
//			if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
//				m_nCaretPosY, nIdx,
//				&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL ) ){
//				/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
//				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen );
//				nColmFrom = LineIndexToColmn( pLine, nLineLen, nColmFrom );
//				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen );
//				nColmTo = LineIndexToColmn( pLine, nLineLen, nColmTo );
//				/* �I��͈͂̕ύX */
//				m_nSelectLineBgnFrom = nLineFrom;	/* �͈͑I���J�n�s(���_) */
//				m_nSelectColmBgnFrom = nColmFrom;	/* �͈͑I���J�n��(���_) */
//				m_nSelectLineBgnTo = nLineFrom;		/* �͈͑I���J�n�s(���_) */
//				m_nSelectColmBgnTo = nColmFrom;		/* �͈͑I���J�n��(���_) */
//				m_nSelectLineFrom =	nLineFrom;
//				m_nSelectColmFrom = nColmFrom;
//				m_nSelectLineTo = nLineTo;
//				m_nSelectColmTo = nColmTo;
//				/* �I��͈͂̃f�[�^���擾 */
//				if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE ) ){
//					/* ��������������݈ʒu�̒P��ŏ����� */
//					strncpy( m_pcEditDoc->m_cDlgReplace.m_szText, cmemCurText.GetPtr(), MAX_PATH - 1 );
//					m_pcEditDoc->m_cDlgReplace.m_szText[MAX_PATH - 1] = '\0';
//				}
//				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
//				DisableSelectArea( FALSE );
//			}
//		}
//	}
//	/* ����������͉��s�܂� */
//	for( i = 0; i < (int)lstrlen( m_pcEditDoc->m_cDlgReplace.m_szText ); ++i ){
//		if( m_pcEditDoc->m_cDlgReplace.m_szText[i] == CR ||
//			m_pcEditDoc->m_cDlgReplace.m_szText[i] == LF ){
//			m_pcEditDoc->m_cDlgReplace.m_szText[i] = '\0';
//			break;
//		}
//	}

	/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
	GetCurrentTextForSearch( cmemCurText );

	/* ����������������� */
	strcpy( m_pcEditDoc->m_cDlgReplace.m_szText, cmemCurText.GetPtr() );

	if ( IsTextSelected() && m_nSelectLineFrom!=m_nSelectLineTo ) {
		bSelected = TRUE;	//�I��͈͂��`�F�b�N���ă_�C�A���O�\��
	}else{
		bSelected = FALSE;	//�t�@�C���S�̂��`�F�b�N���ă_�C�A���O�\��
	}
	/* �u���I�v�V�����̏����� */
	m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget=0;	/* �u���Ώ� */
	m_pcEditDoc->m_cDlgReplace.m_nPaste=FALSE;		/* �\��t����H */
// To Here 2001.12.03 hor

	/* �u���_�C�A���O�̕\�� */
//	nRet = m_pcEditDoc->m_cDlgReplace.DoModal( (LPARAM)this, bSelected );
//	MYTRACE( "nRet=%d\n", nRet );
	//	From Here Jul. 2, 2001 genta �u���E�B���h�E��2�d�J����}�~
	if( !::IsWindow( m_pcEditDoc->m_cDlgReplace.m_hWnd ) ){
		m_pcEditDoc->m_cDlgReplace.DoModeless( m_hInstance, m_hWnd/*::GetParent( m_hwndParent )*/, (LPARAM)this, bSelected );
	}
	else {
			/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( m_pcEditDoc->m_cDlgReplace.m_hWnd );
		::SetDlgItemText( m_pcEditDoc->m_cDlgReplace.m_hWnd, IDC_COMBO_TEXT, cmemCurText.GetPtr() );
	}
	//	To Here Jul. 2, 2001 genta �u���E�B���h�E��2�d�J����}�~
	return;
}

/*! �u�����s
	
	@date 2002/04/08 �e�E�B���h�E���w�肷��悤�ɕύX�B
	@date 2003.05.17 ����� �����O�}�b�`�̖����u������Ȃ�
*/
void CEditView::Command_REPLACE( HWND hwndParent )
{
	if ( hwndParent == NULL ){	//	�e�E�B���h�E���w�肳��Ă��Ȃ���΁ACEditView���e�B
		hwndParent = m_hWnd;
	}
	//2002.02.10 hor
	int nPaste			=	m_pcEditDoc->m_cDlgReplace.m_nPaste;
	int nReplaceTarget	=	m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget;
	int	bRegularExp		=	m_pShareData->m_Common.m_bRegularExp;
	int nFlag			=	m_pShareData->m_Common.m_bLoHiCase ? 0x01 : 0x00;

	// From Here 2001.12.03 hor
	if( nPaste && !m_pcEditDoc->IsEnablePaste()){
		::MYMESSAGEBOX( hwndParent, MB_OK , GSTR_APPNAME,"�N���b�v�{�[�h�ɗL���ȃf�[�^������܂���I");
		::CheckDlgButton( m_pcEditDoc->m_cDlgReplace.m_hWnd, IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( m_pcEditDoc->m_cDlgReplace.m_hWnd, IDC_COMBO_TEXT2 ), TRUE );
		return;	//	���sreturn;
	}

	// 2002.01.09 hor
	// �I���G���A������΁A���̐擪�ɃJ�[�\�����ڂ�
	if( IsTextSelected() ){
		if( m_bBeginBoxSelect ){
			MoveCursor( m_nSelectColmFrom,
						m_nSelectLineFrom,
						TRUE );
		} else {
//			HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );
			Command_LEFT( FALSE, FALSE );
		}
	}
	// To Here 2002.01.09 hor
	
	// ��`�I���H
//			bBeginBoxSelect = m_bBeginBoxSelect;

	/* �J�[�\�����ړ� */
	//HandleCommand( F_LEFT, TRUE, 0, 0, 0, 0 );	//�H�H�H
	// To Here 2001.12.03 hor

	/* �e�L�X�g�I������ */
	/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	DisableSelectArea( TRUE );

	// 2004.06.01 Moca �������ɁA���̃v���Z�X�ɂ����m_szREPLACEKEYArr�������������Ă����v�Ȃ悤��
	const CMemory	cMemRepKey( m_pShareData->m_szREPLACEKEYArr[0], _tcslen(m_pShareData->m_szREPLACEKEYArr[0]) );

	/* �������� */
	Command_SEARCH_NEXT( true, TRUE, hwndParent, 0 );

	/* �e�L�X�g���I������Ă��邩 */
	if( IsTextSelected() ){
		// From Here 2001.12.03 hor
		int colTmp = 0;
		int linTmp = 0;
		if ( !bRegularExp ) {
			// ���K�\������ ����Q��($&)�Ŏ�������̂ŁA���K�\���͏��O
			if(nReplaceTarget==1){	//�}���ʒu�ֈړ�
				colTmp = m_nSelectColmTo - m_nSelectColmFrom;
				linTmp = m_nSelectLineTo - m_nSelectLineFrom;
				m_nSelectColmFrom=-1;
				m_nSelectLineFrom=-1;
				m_nSelectColmTo	 =-1;
				m_nSelectLineTo	 =-1;
			}else
			if(nReplaceTarget==2){	//�ǉ��ʒu�ֈړ�
				// ���K�\�������O�����̂ŁA�u������̕��������s������玟�̍s�̐擪�ֈړ��v�̏������폜
				m_nCaretPosX = m_nSelectColmTo;
				m_nCaretPosY = m_nSelectLineTo;
				m_nSelectColmFrom=-1;
				m_nSelectLineFrom=-1;
				m_nSelectColmTo	 =-1;
				m_nSelectLineTo	 =-1;
			}
			else
			{
				// �ʒu�w��Ȃ��̂ŁA�������Ȃ�
			}
		}
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		/* �e�L�X�g��\��t�� */
		//HandleCommand( F_INSTEXT, TRUE, (LPARAM)m_szText2, FALSE, 0, 0 );
		if(nPaste){
		//	HandleCommand( F_PASTE, 0, 0, 0, 0, 0 );
			Command_PASTE();
		} else if ( bRegularExp ) { /* �����^�u��  1==���K�\�� */
			// ��ǂ݂ɑΉ����邽�߂ɕ����s���܂ł��g���悤�ɕύX 2005/03/27 �����
			// 2002/01/19 novice ���K�\���ɂ�镶����u��
			CMemory cmemory;
			CBregexp cRegexp;

			if( !InitRegexp( m_hWnd, cRegexp, true ) ){
				return;	//	���sreturn;
			}

			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.Search(m_nSelectLineFrom);
			const char* pLine = pcLayout->m_pCDocLine->GetPtr();
			int nIdx = LineColmnToIndex( pcLayout, m_nSelectColmFrom ) + pcLayout->m_nOffset;
			int nLen = pcLayout->m_pCDocLine->GetLength();
			// ���K�\���őI���n�_�E�I�_�ւ̑}�����L�q
			//	Jun. 6, 2005 �����
			// ������ł́u�����̌��̕��������s�������玟�̍s���ֈړ��v�������ł��Ȃ�
			// �� Oct. 30, �u�����̌��̕��������s��������E�E�v�̏�������߂�i�N������Ȃ��݂����Ȃ̂Łj
			// Nov. 9, 2005 ����� ���K�\���őI���n�_�E�I�_�ւ̑}�����@��ύX(��)
			CMemory cMemMatchStr = CMemory(_T("$&"), _tcslen(_T("$&")));
			CMemory cMemRepKey2;
			if (nReplaceTarget == 1) {	//�I���n�_�֑}��
				cMemRepKey2 = cMemRepKey;
				cMemRepKey2 += cMemMatchStr;
			} else if (nReplaceTarget == 2) { // �I���I�_�֑}��
				cMemRepKey2 = cMemMatchStr;
				cMemRepKey2 += cMemRepKey;
			} else {
				cMemRepKey2 = cMemRepKey;
			}
			cRegexp.Compile( m_pShareData->m_szSEARCHKEYArr[0], cMemRepKey2.GetPtr(), nFlag);
			if( cRegexp.Replace(pLine, nLen, nIdx) ){
				// From Here Jun. 6, 2005 �����
				// �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				// �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				// �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				CLayoutMgr& rLayoutMgr = m_pcEditDoc->m_cLayoutMgr;
				int matchLen = cRegexp.GetMatchLen();
				int nIdxTo = nIdx + matchLen;		// ����������̖���
				if (matchLen == 0) {
					// �O�����}�b�`�̎�(�����u���ɂȂ�Ȃ��悤�ɂP�����i�߂�)
					if (nIdxTo < nLen) {
						// 2005-09-02 D.S.Koba GetSizeOfChar
						nIdxTo += (CMemory::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					}
					// �����u�����Ȃ��悤�ɁA�P�������₵���̂łP�����I���ɕύX
					// �I���n�_�E�I�_�ւ̑}���̏ꍇ���O�����}�b�`���͓���͓����ɂȂ�̂�
					rLayoutMgr.CaretPos_Phys2Log( nIdxTo, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );	// 2007.01.19 ryoji �s�ʒu���擾����
				}
				// �s�����猟�������񖖔��܂ł̕�����
				int colDiff = nLen - nIdxTo;
				//	Oct. 22, 2005 Karoto
				//	\r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				if (colDiff < pcLayout->m_pCDocLine->m_cEol.GetLen()) {
					// ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					colDiff = 0;
					rLayoutMgr.CaretPos_Phys2Log( nLen, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );	// 2007.01.19 ryoji �ǉ�
				}
				// �u���㕶����ւ̏�������(�s�����猟�������񖖔��܂ł̕���������)
				Command_INSTEXT( FALSE, cRegexp.GetString(), cRegexp.GetStringLen() - colDiff, TRUE );
				// To Here Jun. 6, 2005 �����
			}
		}else{
			//	HandleCommand( F_INSTEXT, FALSE, (LPARAM)m_pShareData->m_szREPLACEKEYArr[0], FALSE, 0, 0 );
			Command_INSTEXT( FALSE, cMemRepKey.GetPtr(), cMemRepKey.GetLength(), TRUE );
		}
		// �}����̌����J�n�ʒu�𒲐�
		if(nReplaceTarget==1){
			m_nCaretPosX+=colTmp;
			m_nCaretPosY+=linTmp;
		}
		// To Here 2001.12.03 hor
		/* �Ō�܂Œu����������OK�����܂Œu���O�̏�Ԃ��\�������̂ŁA
		** �u����A������������O�ɏ������� 2003.05.17 �����
		*/
		Redraw();
		/* �������� */
	//	HandleCommand( F_SEARCH_NEXT, TRUE, (LPARAM)m_hWnd, (LPARAM)"�Ō�܂Œu�����܂����B", 0, 0 );
		Command_SEARCH_NEXT( true, TRUE, hwndParent, "�Ō�܂Œu�����܂����B" );
	}
}

/*! ���ׂĒu�����s

	@date 2003.05.22 ����� �����}�b�`�΍�D�s���E�s�������Ȃǌ�����
	@date 2006.03.31 ����� �s�u���@�\�ǉ�
	@date 2007.01.16 ryoji �s�u���@�\��S�u���̃I�v�V�����ɕύX
*/
void CEditView::Command_REPLACE_ALL()
{
	int			nNewPos;
	int			nReplaceNum;
	char		szLabel[64];
	int			nAllLineNum;
	HWND		hwndProgress;
	HWND		hwndStatic;

// From Here 2001.12.03 hor
	int			colFrom;		//�I��͈͊J�n��
	int			linFrom;		//�I��͈͊J�n�s
	int			colTo,colToP;	//�I��͈͏I����
	int			linTo,linToP;	//�I��͈͏I���s
	int			colDif = 0;		//�u����̌�����
	int			linDif = 0;		//�u����̍s����
	int			colOld = 0;		//������̑I��͈͎���
	int			linOld = 0;		//������̍s
	int			linOldLen = 0;	//������̍s�̒���
	int			lineCnt;		//�u���O�̍s��
	int			linPrev = 0;	//�O��̌����s(��`) @@@2001.12.31 YAZAKI warning�ގ�
	int			linNext;		//����̌����s(��`)
	int			colTmp,linTmp,colLast,linLast;
	int			bBeginBoxSelect; // ��`�I���H

	//2002.02.10 hor
	int nPaste			= m_pcEditDoc->m_cDlgReplace.m_nPaste;
	int nReplaceTarget	= m_pcEditDoc->m_cDlgReplace.m_nReplaceTarget;
	int	bRegularExp		= m_pShareData->m_Common.m_bRegularExp;
	int bSelectedArea	= m_pShareData->m_Common.m_bSelectedArea;
	int bConsecutiveAll	= m_pShareData->m_Common.m_bConsecutiveAll;	/* �u���ׂĒu���v�͒u���̌J�Ԃ� */	// 2007.01.16 ryoji

	m_pcEditDoc->m_cDlgReplace.m_bCanceled=false;
	m_pcEditDoc->m_cDlgReplace.m_nReplaceCnt=0;

	// From Here 2001.12.03 hor
	if( nPaste && !m_pcEditDoc->IsEnablePaste() ){
		::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,"�N���b�v�{�[�h�ɗL���ȃf�[�^������܂���I");
		::CheckDlgButton( m_hWnd, IDC_CHK_PASTE, FALSE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_TEXT2 ), TRUE );
		return;	// TRUE;
	}
	// To Here 2001.12.03 hor

	if(IsTextSelected()){
		bBeginBoxSelect=m_bBeginBoxSelect;
	}
	else{
		bSelectedArea=FALSE;
		bBeginBoxSelect=FALSE;
	}

	/* �\������ON/OFF */
	BOOL bDisplayUpdate = FALSE;

	m_bDrawSWITCH = bDisplayUpdate;
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel;
	nAllLineNum = m_pcEditDoc->m_cLayoutMgr.GetLineCount();

	/* �i���\��&���~�_�C�A���O�̍쐬 */
	hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hWnd, IDD_REPLACERUNNING );
	::EnableWindow( m_hWnd, FALSE );
	::EnableWindow( ::GetParent( m_hWnd ), FALSE );
	::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), FALSE );
	//<< 2002/03/26 Azumaiya
	// ����Z�|���Z�������ɐi���󋵂�\����悤�ɁA�V�t�g���Z������B
	int nShiftCount;
	for ( nShiftCount = 0; SHRT_MAX < nAllLineNum; nShiftCount++ )
	{
		nAllLineNum >>= 1;
	}
	//>> 2002/03/26 Azumaiya

	/* �v���O���X�o�[������ */
	hwndProgress = ::GetDlgItem( hwndCancel, IDC_PROGRESS_REPLACE );
//	::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
	::SendMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, nAllLineNum ) );
	nNewPos = 0;
 	::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );

	/* �u���������� */
	nReplaceNum = 0;
	hwndStatic = ::GetDlgItem( hwndCancel, IDC_STATIC_KENSUU );
	_itoa( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	// From Here 2001.12.03 hor
	if (bSelectedArea){
		/* �I��͈͒u�� */
		/* �I��͈͊J�n�ʒu�̎擾 */
		colFrom = m_nSelectColmFrom;
		linFrom = m_nSelectLineFrom;
		colTo   = m_nSelectColmTo;
		linTo   = m_nSelectLineTo;
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
			colTo,
			linTo,
			&colToP,
			&linToP
		);
		//�I��͈͊J�n�ʒu�ֈړ�
		MoveCursor( colFrom, linFrom, bDisplayUpdate );
	}
	else{
		/* �t�@�C���S�̒u�� */
		/* �t�@�C���̐擪�Ɉړ� */
	//	HandleCommand( F_GOFILETOP, bDisplayUpdate, 0, 0, 0, 0 );
		Command_GOFILETOP(bDisplayUpdate);
	}
	colLast=m_nCaretPosX;
	linLast=m_nCaretPosY;

	/* �e�L�X�g�I������ */
	/* ���݂̑I��͈͂��I����Ԃɖ߂� */
	DisableSelectArea( bDisplayUpdate );
	/* �������� */
	Command_SEARCH_NEXT( true, bDisplayUpdate, 0, 0 );
	// To Here 2001.12.03 hor

	//<< 2002/03/26 Azumaiya
	// �������������Ƃ��ŗD��ɑg��ł݂܂����B
	// ���[�v�̊O�ŕ�����̒��������ł���̂ŁA�ꎞ�ϐ����B
	char *szREPLACEKEY;			// �u���㕶����B
	int nREPLACEKEY;			// �u���㕶����̒����B
	BOOL		bColmnSelect;	// ��`�\��t�����s�����ǂ����B
	CMemory		cmemClip;		// �u���㕶����̃f�[�^�i�f�[�^���i�[���邾���ŁA���[�v���ł͂��̌`�ł̓f�[�^�������܂���j�B

	// �N���b�v�{�[�h����̃f�[�^�\��t�����ǂ����B
	if( nPaste != 0 )
	{
		// �N���b�v�{�[�h����f�[�^���擾�B
		if ( FALSE == MyGetClipboardData( cmemClip, &bColmnSelect ) )
		{
			::MessageBeep( MB_ICONHAND );
			return;
		}

		// ��`�\��t����������Ă��āA�N���b�v�{�[�h�̃f�[�^����`�I���̂Ƃ��B
		if ( m_pShareData->m_Common.m_bAutoColmnPaste == TRUE && bColmnSelect == TRUE )
		{
			// �}�E�X�ɂ��͈͑I��
			if( m_bBeginSelect )
			{
				::MessageBeep( MB_ICONHAND );
				return;
			}

			// ���݂̃t�H���g�͌Œ蕝�t�H���g�ł���
			if( FALSE == m_pShareData->m_Common.m_bFontIs_FIXED_PITCH )
			{
				return;
			}
		}
		else
		// �N���b�v�{�[�h����̃f�[�^�͕��ʂɈ����B
		{
			bColmnSelect = FALSE;
		}

		// �f�[�^�ւ̃|�C���^�Ƃ��̒������擾�B
//		szREPLACEKEY = cmemClip.GetPtr(&nREPLACEKEY);
	}
	else
	{
		// �f�[�^�ւ̃|�C���^���Z�b�g�B
//		szREPLACEKEY = m_pShareData->m_szREPLACEKEYArr[0];
		// �����������擾�B
//		nREPLACEKEY = strlen(szREPLACEKEY);
		// 2004.05.14 Moca �S�u���̓r���ő��̃E�B���h�E�Œu�������Ƃ܂����̂ŃR�s�[����
		cmemClip.SetDataSz( m_pShareData->m_szREPLACEKEYArr[0] );
	}
	szREPLACEKEY = cmemClip.GetPtr(&nREPLACEKEY);

	// �擾�ɃX�e�b�v�������肻���ȕϐ��Ȃǂ��A�ꎞ�ϐ�������B
	// �Ƃ͂����A�����̑�������邱�Ƃɂ���ē�������N���b�N���͍��킹�Ă� 1 ���[�v�Ő��\���Ǝv���܂��B
	// ���S�N���b�N�����[�v�̃I�[�_�[����l���Ă�����Ȃɓ��͂��Ȃ��悤�Ɏv���܂����ǁE�E�E�B
	BOOL bAddCRLFWhenCopy = m_pShareData->m_Common.m_bAddCRLFWhenCopy;
	BOOL &bCANCEL = cDlgCancel.m_bCANCEL;
	CDocLineMgr& rDocLineMgr = m_pcEditDoc->m_cDocLineMgr;
	CLayoutMgr& rLayoutMgr = m_pcEditDoc->m_cLayoutMgr;

	//  �N���X�֌W�����[�v�̒��Ő錾���Ă��܂��ƁA�����[�v���ƂɃR���X�g���N�^�A�f�X�g���N�^��
	// �Ă΂�Ēx���Ȃ�̂ŁA�����Ő錾�B
	CMemory cmemory;
	CBregexp cRegexp;
	// �����������l�ɖ����[�v���Ƃɂ��ƒx���̂ŁA�ŏ��ɍς܂��Ă��܂��B
	if( bRegularExp )
	{
		if ( !InitRegexp( m_hWnd, cRegexp, true ) )
		{
			return;
		}

		const CMemory	cMemRepKey( szREPLACEKEY, _tcslen(szREPLACEKEY) );
		// Nov. 9, 2005 ����� ���K�\���őI���n�_�E�I�_�ւ̑}�����@��ύX(��)
		CMemory cMemRepKey2;
		CMemory cMemMatchStr = CMemory(_T("$&"), _tcslen(_T("$&")));
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
		int nFlag = (m_pShareData->m_Common.m_bLoHiCase ? CBregexp::optCaseSensitive : CBregexp::optNothing);
		nFlag |= (bConsecutiveAll ? CBregexp::optNothing : CBregexp::optGlobal);	// 2007.01.16 ryoji
		cRegexp.Compile(m_pShareData->m_szSEARCHKEYArr[0], cMemRepKey2.GetPtr(), nFlag);
	}

	/* �e�L�X�g���I������Ă��邩 */
	while( IsTextSelected() )
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
			nNewPos = m_nSelectLineFrom >> nShiftCount;
			::PostMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
			_itoa( nReplaceNum, szLabel, 10 );
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
				lineCnt = rLayoutMgr.GetLineCount();
				// ������͈̔͏I�[
				colOld = m_nSelectColmTo;
				linOld = m_nSelectLineTo;
				// �O��̌����s�ƈႤ�H
				if(linOld!=linPrev){
					colDif=0;
				}
				linPrev=linOld;
				// �s�͔͈͓��H
				if ((linTo+linDif == linOld && colTo+colDif < colOld) ||
					(linTo+linDif <  linOld)) {
					break;
				}
				// ���͔͈͓��H
				if(!((colFrom<=m_nSelectColmFrom)&&
					 (colOld<=colTo+colDif))){
					if(colOld<colTo+colDif){
						linNext=m_nSelectLineTo;
					}else{
						linNext=m_nSelectLineTo+1;
					}
					//���̌����J�n�ʒu�փV�t�g
					m_nCaretPosX=colFrom;
					m_nCaretPosY=linNext;
					// 2004.05.30 Moca ���݂̌�����������g���Č�������
					Command_SEARCH_NEXT( false, bDisplayUpdate, 0, 0 );
					colDif=0;
					continue;
				}
			}
			else
			// ���ʂ̑I��
			//	o �������W���`�F�b�N���Ȃ���u������
			//
			{
				// �������̍s�����L��
				lineCnt = rDocLineMgr.GetLineCount();
				// ������͈̔͏I�[
				rLayoutMgr.CaretPos_Log2Phys(
									m_nSelectColmTo,
									m_nSelectLineTo,
									&colOld,
									&linOld
									);
				// �u���O�̍s�̒���(���s�͂P�����Ɛ�����)��ۑ����Ă����āA�u���O��ōs�ʒu���ς�����ꍇ�Ɏg�p
				linOldLen = rDocLineMgr.GetLineInfo(linOld)->GetLengthWithoutEOL() + 1;
				// �s�͔͈͓��H
				// 2007.01.19 ryoji �����ǉ�: �I���I�_���s��(colToP == 0)�ɂȂ��Ă���ꍇ�͑O�̍s�̍s���܂ł�I��͈͂Ƃ݂Ȃ�
				// �i�I���n�_���s���Ȃ炻�̍s���͑I��͈͂Ɋ܂݁A�I�_���s���Ȃ炻�̍s���͑I��͈͂Ɋ܂܂Ȃ��A�Ƃ���j
				// �_���I�ɏ����ςƎw�E����邩������Ȃ����A���p��͂��̂悤�ɂ����ق����]�܂����P�[�X�������Ǝv����B
				// ���s�I���ōs���܂ł�I��͈͂ɂ�������ł��AUI��͎��̍s�̍s���ɃJ�[�\�����s��
				// ���I�_�̍s�����u^�v�Ƀ}�b�`��������������P�����ȏ�I�����ĂˁA�Ƃ������ƂŁD�D�D
				if ((linToP+linDif == linOld && (colToP+colDif < colOld || colToP == 0))
					|| linToP+linDif < linOld) {
					break;
				}
			}
		}

		colTmp = 0;
		linTmp = 0;
		if ( !bRegularExp ) {
			// ���K�\������ ����Q��($&)�Ŏ�������̂ŁA���K�\���͏��O
			if( nReplaceTarget == 1 )	//�}���ʒu�Z�b�g
			{
				colTmp = m_nSelectColmTo - m_nSelectColmFrom;
				linTmp = m_nSelectLineTo - m_nSelectLineFrom;
                m_nSelectColmFrom=-1;
                m_nSelectLineFrom=-1;
                m_nSelectColmTo	 =-1;
                m_nSelectLineTo	 =-1;
			}
			else if( nReplaceTarget == 2 )	//�ǉ��ʒu�Z�b�g
			{
				// ���K�\�������O�����̂ŁA�u������̕��������s������玟�̍s�̐擪�ֈړ��v�̏������폜
				m_nCaretPosX = m_nSelectColmTo;
				m_nCaretPosY = m_nSelectLineTo;
			    m_nSelectColmFrom=-1;
			    m_nSelectLineFrom=-1;
			    m_nSelectColmTo	 =-1;
			    m_nSelectLineTo	 =-1;
		    } else {
				// �ʒu�w��Ȃ��̂ŁA�������Ȃ�
			}
		}

		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		/* �e�L�X�g��\��t�� */
		if( nPaste )
		{
//			Command_PASTE();
			if ( bColmnSelect == FALSE )
			{
				/* �{���� Command_INSTEXT ���g���ׂ��Ȃ�ł��傤���A���ʂȏ���������邽�߂ɒ��ڂ������B
				** ��m_nSelectXXX��-1�̎��� ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽��
				**   ���ڂ������̂�߂��B2003.05.18 by �����
				*/
				Command_INSTEXT( FALSE, szREPLACEKEY, nREPLACEKEY, TRUE );
			}
			else
			{
				Command_PASTEBOX(szREPLACEKEY, nREPLACEKEY);
				// �ĕ`����s��Ȃ��Ƃǂ�Ȍ��ʂ��N���Ă���̂������炸�݂��Ƃ��Ȃ��̂ŁE�E�E�B
				Redraw();
			}
		}
		// 2002/01/19 novice ���K�\���ɂ�镶����u��
		else if( bRegularExp ) /* �����^�u��  1==���K�\�� */
		{
			// �����s�A�����s���A�����s�ł̌����}�b�`�ʒu
			const CLayout* pcLayout = rLayoutMgr.Search(m_nSelectLineFrom);
			const char* pLine = pcLayout->m_pCDocLine->GetPtr();
			int nIdx = LineColmnToIndex( pcLayout, m_nSelectColmFrom ) + pcLayout->m_nOffset;
			int nLen = pcLayout->m_pCDocLine->GetLength();
			int colDiff = 0;
			if( !bConsecutiveAll ){	// �ꊇ�u��
				// 2007.01.16 ryoji
				// �I��͈͒u���̏ꍇ�͍s���̑I��͈͖����܂Œu���͈͂��k�߁C
				// ���̈ʒu���L������D
				if( bSelectedArea ){
					if( bBeginBoxSelect ){	// ��`�I��
						int wk;
						rLayoutMgr.CaretPos_Log2Phys(
											colTo,
											linOld,
											&colToP,
											&wk
											);
						if( nLen - pcLayout->m_pCDocLine->m_cEol.GetLen() > colToP + colDif )
							nLen = colToP + colDif;
					} else {	// �ʏ�̑I��
						if( linToP+linDif == linOld ){
							if( nLen - pcLayout->m_pCDocLine->m_cEol.GetLen() > colToP + colDif )
								nLen = colToP + colDif;
						}
					}
				}

				if(pcLayout->m_pCDocLine->GetLengthWithoutEOL() < nLen)
					colOld = pcLayout->m_pCDocLine->GetLengthWithoutEOL() + 1;
				else
					colOld = nLen;
			}
			if( int nReplace = cRegexp.Replace(pLine, nLen, nIdx) ){
				nReplaceNum += nReplace;
				if ( !bConsecutiveAll ) { // 2006.04.01 �����	// 2007.01.16 ryoji
					// �s�P�ʂł̒u������
					// �I��͈͂𕨗��s���܂łɂ̂΂�
					rLayoutMgr.CaretPos_Phys2Log( nLen, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );
				} else {
				    // From Here Jun. 6, 2005 �����
				    // �����s���܂�INSTEXT������@�́A�L�����b�g�ʒu�𒲐�����K�v������A
				    // �L�����b�g�ʒu�̌v�Z�����G�ɂȂ�B�i�u����ɉ��s������ꍇ�ɕs������j
				    // �����ŁAINSTEXT���镶���񒷂𒲐�������@�ɕύX����i���͂������̕����킩��₷���j
				    int matchLen = cRegexp.GetMatchLen();
				    int nIdxTo = nIdx + matchLen;		// ����������̖���
				    if (matchLen == 0) {
					    // �O�����}�b�`�̎�(�����u���ɂȂ�Ȃ��悤�ɂP�����i�߂�)
					    if (nIdxTo < nLen) {
						    // 2005-09-02 D.S.Koba GetSizeOfChar
						    nIdxTo += (CMemory::GetSizeOfChar(pLine, nLen, nIdxTo) == 2 ? 2 : 1);
					    }
					    // �����u�����Ȃ��悤�ɁA�P�������₵���̂łP�����I���ɕύX
					    // �I���n�_�E�I�_�ւ̑}���̏ꍇ���O�����}�b�`���͓���͓����ɂȂ�̂�
						rLayoutMgr.CaretPos_Phys2Log( nIdxTo, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );	// 2007.01.19 ryoji �s�ʒu���擾����
				    }
				    // �s�����猟�������񖖔��܂ł̕�����
					colDiff =  nLen - nIdxTo;
					colOld = nIdxTo;	// 2007.01.19 ryoji �ǉ�
				    //	Oct. 22, 2005 Karoto
				    //	\r��u������Ƃ��̌���\n�������Ă��܂����̑Ή�
				    if (colDiff < pcLayout->m_pCDocLine->m_cEol.GetLen()) {
					    // ���s�ɂ������Ă�����A�s�S�̂�INSTEXT����B
					    colDiff = 0;
						rLayoutMgr.CaretPos_Phys2Log( nLen, pcLayout->m_nLinePhysical, &m_nSelectColmTo, &m_nSelectLineTo );	// 2007.01.19 ryoji �ǉ�
						colOld = pcLayout->m_pCDocLine->GetLengthWithoutEOL() + 1;	// 2007.01.19 ryoji �ǉ�
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
			** ��m_nSelectXXX��-1�̎��� ReplaceData_CEditView�𒼐ڂ������Ɠ���s�ǂƂȂ邽�ߒ��ڂ������̂�߂��B2003.05.18 �����
			*/
			Command_INSTEXT( FALSE, szREPLACEKEY, nREPLACEKEY, TRUE );
			++nReplaceNum;
		}

		// �}����̈ʒu����
		if( nReplaceTarget == 1 )
		{
			m_nCaretPosX+=colTmp;
			m_nCaretPosY+=linTmp;
			if (!bBeginBoxSelect)
			{
				rLayoutMgr.CaretPos_Log2Phys(
										m_nCaretPosX,
										m_nCaretPosY,
										&m_nCaretPosX_PHY,
										&m_nCaretPosY_PHY
										);
			}
		}

		// �Ō�ɒu�������ʒu���L��
		colLast = m_nCaretPosX;
		linLast = m_nCaretPosY;

		/* �u����̈ʒu���m�F */
		if( bSelectedArea )
		{
			// �������u���̍s�␳�l�擾
			if( bBeginBoxSelect )
			{
				colDif += colLast - colOld;
				linDif += rLayoutMgr.GetLineCount() - lineCnt;
			}
			else
			{
				// �u���O�̌���������̍ŏI�ʒu�� colOld, linOld
				// �u����̃J�[�\���ʒu
				colTmp = m_nCaretPosX_PHY;
				linTmp = m_nCaretPosY_PHY;
				int linDif_thistime = rDocLineMgr.GetLineCount() - lineCnt;	// ����u���ł̍s���ω�
				linDif += linDif_thistime;
				if( linToP + linDif == linTmp)
				{
					// �ŏI�s�Œu���������A���́A�u���̌��ʁA�I���G���A�ŏI�s�܂œ��B������
					// �ŏI�s�Ȃ̂ŁA�u���O��̕������̑����Ō��ʒu�𒲐�����
					colDif += colTmp - colOld;
					// �A���A�ȉ��̏ꍇ�͒u���O��ōs���قȂ��Ă��܂��̂ŁA�s�̒����ŕ␳����K�v������
					// �P�j�ŏI�s���O�ōs�A�����N����A�s�������Ă���ꍇ�i�s�A���Ȃ̂ŁA���ʒu�͒u����̃J�[�\�����ʒu����������j
					// �@�@colTmp-colOld���ƁA\r\n �� "" �u���ōs�A�������ꍇ�ɁA���ʒu�����ɂȂ莸�s����i���Ƃ͑O�s�̌��̕��ɂȂ邱�ƂȂ̂ŕ␳����j
					// �@�@����u���ł̍s���̕ω�(linDif_thistime)�ŁA�ŏI�s���s�A�����ꂽ���ǂ��������邱�Ƃɂ���
					// �Q�j���s��u�������ilinTmp!=linOld�j�ꍇ�A���s��u������ƒu����̌��ʒu�����s�̌��ʒu�ɂȂ��Ă��邽��
					//     colTmp-colOld���ƁA���̐��ƂȂ�A\r\n �� \n �� \n �� "abc" �ȂǂŌ��ʒu�������
					//     ������O�s�̒����Œ�������K�v������
					if (linDif_thistime < 0 || linTmp != linOld) {
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
		nNewPos = m_nSelectLineFrom >> nShiftCount;
		::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
	}
	//>> 2002/03/26 Azumaiya

	_itoa( nReplaceNum, szLabel, 10 );
	::SendMessage( hwndStatic, WM_SETTEXT, 0, (LPARAM)szLabel );

	if( !cDlgCancel.IsCanceled() ){
		nNewPos = nAllLineNum;
		::SendMessage( hwndProgress, PBM_SETPOS, nNewPos, 0 );
	}
	cDlgCancel.CloseDialog( 0 );
	::EnableWindow( m_hWnd, TRUE );
	::EnableWindow( ::GetParent( m_hWnd ), TRUE );
	::EnableWindow( ::GetParent( ::GetParent( m_hWnd ) ), TRUE );

	// From Here 2001.12.03 hor

	/* �e�L�X�g�I������ */
	DisableSelectArea( TRUE );

	/* �J�[�\���E�I��͈͕��� */
	if((!bSelectedArea) ||			// �t�@�C���S�̒u��
	   (cDlgCancel.IsCanceled())) {		// �L�����Z�����ꂽ
		// �Ō�ɒu������������̉E��
		MoveCursor( colLast, linLast, TRUE );
	}else{
		if (bBeginBoxSelect) {
		// ��`�I��
			m_bBeginBoxSelect=bBeginBoxSelect;
			linTo+=linDif;
			if(linTo<0)linTo=0;
		}else{
		// ���ʂ̑I��
			colToP+=colDif;
			if(colToP<0)colToP=0;
			linToP+=linDif;
			if(linToP<0)linToP=0;
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
				colToP,
				linToP,
				&colTo,
				&linTo
			);
		}
		if(linFrom<linTo || colFrom<colTo){
			m_nSelectLineFrom = linFrom;
			m_nSelectColmFrom = colFrom;
			m_nSelectLineTo   = linTo;
			m_nSelectColmTo   = colTo;
		}
		MoveCursor( colTo, linTo, TRUE );
	}
	// To Here 2001.12.03 hor

	m_pcEditDoc->m_cDlgReplace.m_bCanceled = (cDlgCancel.IsCanceled() != FALSE);
	m_pcEditDoc->m_cDlgReplace.m_nReplaceCnt=nReplaceNum;
	m_bDrawSWITCH = TRUE;
	ActivateFrameWindow( ::GetParent( m_hwndParent ) );
}



//	/* �P�ꃊ�t�@�����X*/
//	void CEditView::Command_WORDSREFERENCE( void )
//	{
//	int				nNewLine;		/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int				nNewPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
//	COpe*			pcOpe = NULL;
//	CWaitCursor		cWaitCursor;
//	CDlgWords		cDlgWords;
//
//	/* ������ */
//	cDlgWords.Create( m_hInstance, m_hWnd, (void*)m_pcEditDoc );
//
//	/* ���[�_���_�C�A���O�̕\�� */
//	if( FALSE == cDlgWords.DoModal() ){
//		return;
//	}
//
//
//	/* �e�L�X�g���I������Ă��邩 */
//	if( IsTextSelected() ){
//		DeleteData( TRUE );
//	}
//
//	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//		pcOpe = new COpe;
//		pcOpe->m_nCaretPosX_Before = m_nCaretPosX;			/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_Before = m_nCaretPosY;			/* ����O�̃L�����b�g�ʒu�x */
//		pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */
//	}
//
//	/* ���݈ʒu�Ƀf�[�^��}�� */
//	InsertData_CEditView( m_nCaretPosX, m_nCaretPosY, cDlgWords.m_szWord, lstrlen( cDlgWords.m_szWord ), &nNewLine, &nNewPos, pcOpe, TRUE );
//
//	/* �}���f�[�^�̍Ō�փJ�[�\�����ړ� */
//	MoveCursor( nNewPos, nNewLine, TRUE );
//	m_nCaretPosX_Prev = m_nCaretPosX;
//
//	if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
//		pcOpe->m_nCaretPosX_After = m_nCaretPosX;			/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_After = m_nCaretPosY;			/* �����̃L�����b�g�ʒu�x */
//		pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
//		pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
//		/* ����̒ǉ� */
//		m_pcOpeBlk->AppendOpe( pcOpe );
//	}
//
//	m_pcEditDoc->SetModified(true,true);	//	Jan. 22, 2002 genta//
//	return;
//	}




/* �J�[�\���s���E�B���h�E������ */
void CEditView::Command_CURLINECENTER( void )
{
	int		nViewTopLine;
	nViewTopLine = m_nCaretPosY - ( m_nViewRowNum / 2 );

	// sui 02/08/09
	if( 0 > nViewTopLine )	nViewTopLine = 0;
	
	int nScrollLines = nViewTopLine - m_nViewTopLine;	//Sep. 11, 2004 genta �����p�ɍs�����L��
	m_nViewTopLine = nViewTopLine;
	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	RedrawAll();
	// sui 02/08/09

	//	Sep. 11, 2004 genta �����X�N���[���̊֐���
	SyncScrollV( nScrollLines );

	return;
}




/* Base64�f�R�[�h���ĕۑ� */
void CEditView::Command_BASE64DECODE( void )
{
	CMemory		cmemBuf;
	char		szPath[_MAX_PATH];
	HFILE		hFile;

	/* �e�L�X�g���I������Ă��邩 */
	if( !IsTextSelected() ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
	if( FALSE == GetSelectedData( cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* Base64�f�R�[�h */
	cmemBuf.BASE64Decode();
	strcpy( szPath, "" );

	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */
	if( !m_pcEditDoc->SaveFileDialog( (char*)szPath,  NULL ) ){
		return;
	}
	if(HFILE_ERROR == (hFile = _lcreat( szPath, 0 ) ) ){
		::MessageBeep( MB_ICONHAND );
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�t�@�C���̍쐬�Ɏ��s���܂����B\n\n%s", szPath
		);
		return;
	}
	if( HFILE_ERROR == _lwrite( hFile, cmemBuf.GetPtr(), cmemBuf.GetLength() ) ){
		::MessageBeep( MB_ICONHAND );
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�t�@�C���̏������݂Ɏ��s���܂����B\n\n%s", szPath
		);
	}
	_lclose( hFile );
	return;
}




/* uudecode���ĕۑ� */
void CEditView::Command_UUDECODE( void )
{
	CMemory		cmemBuf;
	char		szPath[_MAX_PATH];
	HFILE		hFile;
	/* �e�L�X�g���I������Ă��邩 */
	if( !IsTextSelected() ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
	if( FALSE == GetSelectedData( cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	strcpy( szPath, "" );

	/* uudecode(�f�R�[�h) */	//Oct. 17, 2000 jepro �������uUUENCODE������(�f�R�[�h) �v����ύX
	cmemBuf.UUDECODE( szPath );

	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */
	if( !m_pcEditDoc->SaveFileDialog( (char*)szPath,  NULL ) ){
		return;
	}
	if(HFILE_ERROR == (hFile = _lcreat( szPath, 0 ) ) ){
		::MessageBeep( MB_ICONHAND );
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�t�@�C���̍쐬�Ɏ��s���܂����B\n\n%s", szPath
		);
		return;
	}
	if( HFILE_ERROR == _lwrite( hFile, cmemBuf.GetPtr(), cmemBuf.GetLength() ) ){
		::MessageBeep( MB_ICONHAND );
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�t�@�C���̏������݂Ɏ��s���܂����B\n\n%s", szPath
		);
	}
	_lclose( hFile );
	return;
}




/* �ĕ`�� */
void CEditView::Command_REDRAW( void )
{
	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	RedrawAll();
	return;
}




/* Oracle SQL*Plus�Ŏ��s */
void CEditView::Command_PLSQL_COMPILE_ON_SQLPLUS( void )
{
//	HGLOBAL		hgClip;
//	char*		pszClip;
	HWND		hwndSQLPLUS;
	int			nRet;
	BOOL		nBool;
	char		szPath[MAX_PATH + 2];
	BOOL		bResult;
// Modified by KEITA for WIN64 2003.9.6
//	DWORD		dwResult;
	ULONG_PTR	dwResult;

	hwndSQLPLUS = ::FindWindow( "SqlplusWClass", "Oracle SQL*Plus" );
	if( NULL == hwndSQLPLUS ){
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"Oracle SQL*Plus�Ŏ��s���܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n"
		);
		return;
	}
	/* �e�L�X�g���ύX����Ă���ꍇ */
	if( m_pcEditDoc->IsModified() ){
		nRet = ::MYMESSAGEBOX(
			m_hWnd,
			MB_YESNOCANCEL | MB_ICONEXCLAMATION,
			GSTR_APPNAME,
			"%s\n�͕ύX����Ă��܂��B Oracle SQL*Plus�Ŏ��s����O�ɕۑ����܂����H",
			m_pcEditDoc->IsFilePathAvailable() ? m_pcEditDoc->GetFilePath() : "(����)"
		);
		switch( nRet ){
		case IDYES:
			if( m_pcEditDoc->IsFilePathAvailable() ){
				nBool = HandleCommand( F_FILESAVE, TRUE, 0, 0, 0, 0 );
			}else{
				nBool = HandleCommand( F_FILESAVEAS_DIALOG, TRUE, 0, 0, 0, 0 );
			}
			if( FALSE == nBool ){
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
	if( m_pcEditDoc->IsFilePathAvailable() ){
		/* �t�@�C���p�X�ɋ󔒂��܂܂�Ă���ꍇ�̓_�u���N�H�[�e�[�V�����ň͂� */
		//	2003.10.20 MIK �R�[�h�ȗ���
		if( _tcschr( m_pcEditDoc->GetFilePath(), SPACE ) ? TRUE : FALSE ){
			wsprintf( szPath, "@\"%s\"\r\n", m_pcEditDoc->GetFilePath() );
		}else{
			wsprintf( szPath, "@%s\r\n", m_pcEditDoc->GetFilePath() );
		}
		/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
		MySetClipboardData( szPath, lstrlen( szPath ), FALSE );

		/* Oracle SQL*Plus���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndSQLPLUS );

		/* Oracle SQL*Plus�Ƀy�[�X�g�̃R�}���h�𑗂� */
		bResult = ::SendMessageTimeout(
			hwndSQLPLUS, WM_COMMAND, MAKELONG( 201, 0 ), 0,
			SMTO_ABORTIFHUNG | SMTO_NORMAL,
			3000,
			&dwResult
		);
		if( !bResult ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_TOPMOST | MB_ICONSTOP, GSTR_APPNAME,
				"Oracle SQL*Plus����̔���������܂���B\n���΂炭�҂��Ă���Ăю��s���Ă��������B"
			);
		}
	}else{
		::MessageBeep( MB_ICONHAND );
		::MYMESSAGEBOX( m_hWnd,
			 MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"SQL���t�@�C���ɕۑ����Ȃ���Oracle SQL*Plus�Ŏ��s�ł��܂���B\n"
		);
		return;
	}
	return;
}




/* Oracle SQL*Plus���A�N�e�B�u�\�� */
void CEditView::Command_ACTIVATE_SQLPLUS( void )
{
	HWND		hwndSQLPLUS;
	hwndSQLPLUS = ::FindWindow( "SqlplusWClass", "Oracle SQL*Plus" );
	if( NULL == hwndSQLPLUS ){
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"Oracle SQL*Plus���A�N�e�B�u�\�����܂��B\n\n\nOracle SQL*Plus���N������Ă��܂���B\n"
		);
		return;
	}
	/* Oracle SQL*Plus���A�N�e�B�u�ɂ��� */
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndSQLPLUS );
	return;
}




/* �ǂݎ���p */
void CEditView::Command_READONLY( void )
{
	m_pcEditDoc->m_bReadOnly ^= 1;

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	m_pcEditDoc->SetParentCaption();
}

/* �t�@�C���̃v���p�e�B */
void CEditView::Command_PROPERTY_FILE( void )
{
#ifdef _DEBUG
	{
		/* �S�s�f�[�^��Ԃ��e�X�g */
		char*	pDataAll;
		int		nDataAllLen;
		CRunningTimer cRunningTimer( "CEditView::Command_PROPERTY_FILE �S�s�f�[�^��Ԃ��e�X�g" );
		cRunningTimer.Reset();
		pDataAll = m_pcEditDoc->m_cDocLineMgr.GetAllData( &nDataAllLen );
//		MYTRACE( "�S�f�[�^�擾             (%d�o�C�g) ���v����(�~���b) = %d\n", nDataAllLen, cRunningTimer.Read() );
		free( pDataAll );
		pDataAll = NULL;
//		MYTRACE( "�S�f�[�^�擾�̃������J�� (%d�o�C�g) ���v����(�~���b) = %d\n", nDataAllLen, cRunningTimer.Read() );
	}
#endif


	CDlgProperty	cDlgProperty;
//	cDlgProperty.Create( m_hInstance, m_hWnd, (void *)m_pcEditDoc );
	cDlgProperty.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc );
	return;
}




/* �T�N���G�f�B�^�̑S�I�� */	//Dec. 27, 2000 JEPRO �ǉ�
void CEditView::Command_EXITALL( void )
{
	CEditApp::TerminateApplication();
	return;
}




/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
void CEditView::Command_FILECLOSEALL( void )
{
	CEditApp::CloseAllEditor();
	return;
}




/* �E�B���h�E����� */
void CEditView::Command_WINCLOSE( void )
{
	/* ���� */
	::PostMessage( ::GetParent( m_hwndParent ), WM_CLOSE, 0, 0 );
	return;
}

//�A�E�g�v�b�g�E�B���h�E�\��
void CEditView::Command_WIN_OUTPUT( void )
{
	if( NULL == m_pShareData->m_hwndDebug
		|| !CShareData::IsEditWnd( m_pShareData->m_hwndDebug )
	){
		CEditApp::OpenNewEditor( NULL, NULL, "-DEBUGMODE", CODE_SJIS, FALSE, true );
#if 0
		//	Jun. 25, 2001 genta OpenNewEditor��sync�@�\�𗘗p����悤�ɕύX
		//�A�E�g�v�b�g�E�C���h�E���o����܂�5�b���炢�҂B
		CRunningTimer wait_timer( NULL );
		while( NULL == m_pShareData->m_hwndDebug && 5000 > wait_timer.Read() ){
			Sleep(1);
		}
		Sleep(10);
#endif
	}else{
		/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( m_pShareData->m_hwndDebug );
	}
	return;
}




/* �J�X�^�����j���[�\�� */
int CEditView::Command_CUSTMENU( int nMenuIdx )
{
	HMENU		hMenu;
	int			nId;
	POINT		po;
	int			i;
	char		szLabel[300];
	char		szLabel2[300];
	UINT		uFlags;
//	BOOL		bBool;

	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->m_CMenuDrawer.ResetContents();
	
	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = m_pcEditDoc->m_cFuncLookup;

	if( nMenuIdx < 0 || MAX_CUSTOM_MENU <= nMenuIdx ){
		return 0;
	}
	if( 0 == m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx] ){
		return 0;
	}
	hMenu = ::CreatePopupMenu();
	for( i = 0; i < m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( 0 == m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		}else{
			//	Oct. 3, 2001 genta
			FuncLookup.Funccode2Name( m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
//			::LoadString( m_hInstance, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* �L�[ */
			if( '\0' == m_pShareData->m_Common.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				strcpy( szLabel2, szLabel );
			}else{
				wsprintf( szLabel2, "%s (&%c)", szLabel, m_pShareData->m_Common.m_nCustMenuItemKeyArr[nMenuIdx][i] );
			}
			/* �@�\�����p�\�����ׂ� */
			if( TRUE == CEditWnd::IsFuncEnable( m_pcEditDoc, m_pShareData, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
//			bBool = ::AppendMenu( hMenu, uFlags, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel2 );
			pCEditWnd->m_CMenuDrawer.MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] , szLabel2 );
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
		::GetParent( m_hwndParent )/*m_hWnd*/,
		NULL
	);
	::DestroyMenu( hMenu );
	return nId;
}




//�I��͈͓��S�s�R�s�[
void CEditView::Command_COPYLINES( void )
{
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	CopySelectedAllLines(
		NULL,	/* ���p�� */
		FALSE	/* �s�ԍ���t�^���� */
	);
	return;
}




//�I��͈͓��S�s���p���t���R�s�[
void CEditView::Command_COPYLINESASPASSAGE( void )
{
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	CopySelectedAllLines(
		m_pShareData->m_Common.m_szInyouKigou,	/* ���p�� */
		FALSE 									/* �s�ԍ���t�^���� */
	);
	return;
}




//�I��͈͓��S�s�s�ԍ��t���R�s�[
void CEditView::Command_COPYLINESWITHLINENUMBER( void )
{
	/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
	CopySelectedAllLines(
		NULL,	/* ���p�� */
		TRUE	/* �s�ԍ���t�^���� */
	);
	return;
}




////�L�[���蓖�Ĉꗗ���R�s�[
	//Dec. 26, 2000 JEPRO //Jan. 24, 2001 JEPRO debug version (directed by genta)
void CEditView::Command_CREATEKEYBINDLIST( void )
{
	CMemory		cMemKeyList;

	CKeyBind::CreateKeyBindList(
	m_hInstance,
	m_pShareData->m_nKeyNameArrNum,
	m_pShareData->m_pKeyNameArr,
	cMemKeyList,
	&m_pcEditDoc->m_cFuncLookup	//	Oct. 31, 2001 genta �ǉ�
	 );

	// Windows�N���b�v�{�[�h�ɃR�s�[
	//2004.02.17 Moca �֐���
	SetClipboardText( m_pcEditDoc->m_hWnd, cMemKeyList.GetPtr(), cMemKeyList.GetLength() );
	return;
}

/* �t�@�C�����e��r */
void CEditView::Command_COMPARE( void )
{
	HWND		hwndCompareWnd;
	char		szPath[_MAX_PATH + 1];
	POINT		poSrc;
	POINT		poDes;
	CDlgCompare	cDlgCompare;
	BOOL		bDefferent;
	const char*	pLineSrc;
	int			nLineLenSrc;
	const char*	pLineDes;
	int			nLineLenDes;
	POINT*		ppoCaretDes;
	HWND		hwndMsgBox;	//@@@ 2003.06.12 MIK
//	cDlgCompare.Create( m_hInstance, m_hWnd, (void *)m_pcEditDoc );
	/* ��r��A���E�ɕ��ׂĕ\�� */
	cDlgCompare.m_bCompareAndTileHorz = m_pShareData->m_Common.m_bCompareAndTileHorz;
//	cDlgCompare.m_bCompareAndTileHorz = m_pShareData->m_Common.m_bCompareAndTileHorz;	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�
//	if( FALSE == cDlgCompare.DoModal( m_pcEditDoc->GetFilePath(), m_pcEditDoc->IsModified(), szPath, &hwndCompareWnd ) ){
	if( FALSE == cDlgCompare.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc, m_pcEditDoc->GetFilePath(),
		m_pcEditDoc->IsModified(), szPath, &hwndCompareWnd ) ){
		return;
	}
	/* ��r��A���E�ɕ��ׂĕ\�� */
	m_pShareData->m_Common.m_bCompareAndTileHorz = cDlgCompare.m_bCompareAndTileHorz;
//	m_pShareData->m_Common.m_bCompareAndTileHorz = cDlgCompare.m_bCompareAndTileHorz;	//Oct. 10, 2000 JEPRO �`�F�b�N�{�b�N�X���{�^��������΂��̍s�͕s�v�̂͂�

	//�^�u�E�C���h�E���͋֎~	//@@@ 2003.06.12 MIK
	if( TRUE  == m_pShareData->m_Common.m_bDispTabWnd
	 && FALSE == m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		hwndMsgBox = m_hWnd;
		m_pShareData->m_Common.m_bCompareAndTileHorz = FALSE;
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
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nCaretPosX,
		m_nCaretPosY,
		(int*)&poSrc.x,
		(int*)&poSrc.y
	);
	/* �J�[�\���ʒu�擾�v�� */
	::SendMessage( hwndCompareWnd, MYWM_GETCARETPOS, 0, 0 );
	ppoCaretDes = (POINT*)m_pShareData->m_szWork;
	poDes.x = ppoCaretDes->x;
	poDes.y = ppoCaretDes->y;
	bDefferent = TRUE;
	pLineDes = m_pShareData->m_szWork;
	pLineSrc = m_pcEditDoc->m_cDocLineMgr.GetLineStr( poSrc.y, &nLineLenSrc );
	/* �s(���s�P��)�f�[�^�̗v�� */
	nLineLenDes = ::SendMessage( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	while( 1 ){
		if( pLineSrc == NULL &&	0 == nLineLenDes ){
			bDefferent = FALSE;
			break;
		}
		if( pLineSrc == NULL || 0 == nLineLenDes ){
			break;
		}
		if( nLineLenDes > sizeof( m_pShareData->m_szWork ) ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				"��r��̃t�@�C��\n%s\n%d�o�C�g�𒴂���s������܂��B\n��r�ł��܂���B", szPath, sizeof( m_pShareData->m_szWork )
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
		pLineSrc = m_pcEditDoc->m_cDocLineMgr.GetLineStr( poSrc.y, &nLineLenSrc );
		/* �s(���s�P��)�f�[�^�̗v�� */
		nLineLenDes = ::SendMessage( hwndCompareWnd, MYWM_GETLINEDATA, poDes.y, 0 );
	}
end_of_compare:;
	/* ��r��A���E�ɕ��ׂĕ\�� */
//From Here Oct. 10, 2000 JEPRO	�`�F�b�N�{�b�N�X���{�^��������Έȉ��̍s(To Here �܂�)�͕s�v�̂͂�����
//	���܂������Ȃ������̂Ō��ɖ߂��Ă���c
	if( m_pShareData->m_Common.m_bCompareAndTileHorz ){
		HWND* phwndArr = new HWND[2];
		phwndArr[0] = ::GetParent( m_hwndParent );
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
	if( FALSE == bDefferent ){
		//::MYMESSAGEBOX( hwndCompareWnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, GSTR_APPNAME,
		::MYMESSAGEBOX( hwndMsgBox, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, GSTR_APPNAME,
			"�قȂ�ӏ��͌�����܂���ł����B"
		);
	}else{
		//::MYMESSAGEBOX( hwndCompareWnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, GSTR_APPNAME,
		::MYMESSAGEBOX( hwndMsgBox, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, GSTR_APPNAME,
			"�قȂ�ӏ���������܂����B"
		);
		/* �J�[�\�����ړ�������
			��r����́A�ʃv���Z�X�Ȃ̂Ń��b�Z�[�W���΂��B
		*/
		memcpy( m_pShareData->m_szWork, (void*)&poDes, sizeof( poDes ) );
		::SendMessage( hwndCompareWnd, MYWM_SETCARETPOS, 0, 0 );

		/* �J�[�\�����ړ������� */
		memcpy( m_pShareData->m_szWork, (void*)&poSrc, sizeof( poSrc ) );
		::PostMessage( ::GetParent( m_hwndParent ), MYWM_SETCARETPOS, 0, 0 );
	}
	/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
	/* �A�N�e�B�u�ɂ��� */
//	ActivateFrameWindow( hwndCompareWnd );

	/* �J���Ă���E�B���h�E���A�N�e�B�u�ɂ��� */
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( ::GetParent( m_hwndParent ) );
	return;
}




/*! �c�[���o�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutToolBar(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void CEditView::Command_SHOWTOOLBAR( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	m_pShareData->m_Common.m_bDispTOOLBAR = ((NULL == pCEditWnd->m_hwndToolBar)? TRUE: FALSE);	/* �c�[���o�[�\�� */
	pCEditWnd->LayoutToolBar();
	pCEditWnd->EndLayoutBars();

	//�S�E�C���h�E�ɕύX��ʒm����B
	CShareData::getInstance()->PostMessageToAllEditors( MYWM_BAR_CHANGE_NOTIFY, (WPARAM)MYBCN_TOOLBAR, (LPARAM)pCEditWnd->m_hWnd, pCEditWnd->m_hWnd );
}




/*! �X�e�[�^�X�o�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutStatusBar(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void CEditView::Command_SHOWSTATUSBAR( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	m_pShareData->m_Common.m_bDispSTATUSBAR = ((NULL == pCEditWnd->m_hwndStatusBar)? TRUE: FALSE);	/* �X�e�[�^�X�o�[�\�� */
	pCEditWnd->LayoutStatusBar();
	pCEditWnd->EndLayoutBars();

	//�S�E�C���h�E�ɕύX��ʒm����B
	CShareData::getInstance()->PostMessageToAllEditors( MYWM_BAR_CHANGE_NOTIFY, (WPARAM)MYBCN_STATUSBAR, (LPARAM)pCEditWnd->m_hWnd, pCEditWnd->m_hWnd );
}




/*! �t�@���N�V�����L�[�̕\��/��\��

	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutFuncKey(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
*/
void CEditView::Command_SHOWFUNCKEY( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	m_pShareData->m_Common.m_bDispFUNCKEYWND = ((NULL == pCEditWnd->m_CFuncKeyWnd.m_hWnd)? TRUE: FALSE);	/* �t�@���N�V�����L�[�\�� */
	pCEditWnd->LayoutFuncKey();
	pCEditWnd->EndLayoutBars();

	//�S�E�C���h�E�ɕύX��ʒm����B
	CShareData::getInstance()->PostMessageToAllEditors( MYWM_BAR_CHANGE_NOTIFY, (WPARAM)MYBCN_FUNCKEY, (LPARAM)pCEditWnd->m_hWnd, pCEditWnd->m_hWnd );
}

//@@@ From Here 2003.06.10 MIK
/*! �^�u(�E�C���h�E)�̕\��/��\��

	@author MIK
	@date 2003.06.10 �V�K�쐬
	@date 2006.12.19 ryoji �\���ؑւ� CEditWnd::LayoutTabBar(), CEditWnd::EndLayoutBars() �ōs���悤�ɕύX
 */
void CEditView::Command_SHOWTAB( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	m_pShareData->m_Common.m_bDispTabWnd = ((NULL == pCEditWnd->m_cTabWnd.m_hWnd)? TRUE: FALSE);	/* �^�u�o�[�\�� */
	pCEditWnd->LayoutTabBar();
	pCEditWnd->EndLayoutBars();

// pCEditWnd->EndLayoutBars()�̒���WM_SIZE��������̂ŃE�C���h�E���X�V�̏����͂����ł͕s�v
	//if( m_pShareData->m_Common.m_bDispTabWnd )
	//{
	//	// �E�C���h�E�����X�V����
	//	m_pShareData->m_TabWndWndpl.length = sizeof( m_pShareData->m_TabWndWndpl );
	//	::GetWindowPlacement( m_pcEditDoc->m_pcEditWnd->m_hWnd, &(m_pShareData->m_TabWndWndpl) );
	//}

	//�S�E�C���h�E�ɕύX��ʒm����B
	CShareData::getInstance()->PostMessageToAllEditors( MYWM_BAR_CHANGE_NOTIFY, (WPARAM)MYBCN_TAB, (LPARAM)pCEditWnd->m_hWnd, pCEditWnd->m_hWnd );
}
//@@@ To Here 2003.06.10 MIK



/* ��� */
void CEditView::Command_PRINT( void )
{
	// �g���Ă��Ȃ��������폜 2003.05.04 �����
	Command_PRINT_PREVIEW();
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* ������s */
	pCEditWnd->m_pPrintPreview->OnPrint();
}




/* ����v���r���[ */
void CEditView::Command_PRINT_PREVIEW( void )
{
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* ����v���r���[���[�h�̃I��/�I�t */
	pCEditWnd->PrintPreviewModeONOFF();
	return;
}




/* ����̃y�[�W���C�A�E�g�̐ݒ� */
void CEditView::Command_PRINT_PAGESETUP( void )
{
	BOOL		bRes;
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta

	/* ����y�[�W�ݒ� */
	bRes = pCEditWnd->OnPrintPageSetting();
	return;
}




/* �u���E�Y */
void CEditView::Command_BROWSE( void )
{
	if( !m_pcEditDoc->IsFilePathAvailable() ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
//	char	szURL[MAX_PATH + 64];
//	wsprintf( szURL, "%s", m_pcEditDoc->GetFilePath() );
	/* URL���J�� */
//	::ShellExecuteEx( NULL, "open", szURL, NULL, NULL, SW_SHOW );

    SHELLEXECUTEINFO info; 
    info.cbSize =sizeof(SHELLEXECUTEINFO);
    info.fMask = 0;
    info.hwnd = NULL;
    info.lpVerb = NULL;
    info.lpFile = m_pcEditDoc->GetFilePath();
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
void CEditView::Command_RECKEYMACRO( void )
{
	if( m_pShareData->m_bRecordingKeyMacro ){									/* �L�[�{�[�h�}�N���̋L�^�� */
		m_pShareData->m_bRecordingKeyMacro = FALSE;
		m_pShareData->m_hwndRecordingKeyMacro = NULL;							/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
		//@@@ 2002.1.24 YAZAKI �L�[�}�N�����}�N���p�t�H���_�ɁuRecKey.mac�v�Ƃ������ŕۑ�
		char szInitDir[MAX_PATH];
		int nRet;
		// 2003.06.23 Moca �L�^�p�L�[�}�N���̃t���p�X��CShareData�o�R�Ŏ擾
		nRet = CShareData::getInstance()->GetMacroFilename( -1, szInitDir, MAX_PATH ); 
		if( nRet <= 0 ){
			::MYMESSAGEBOX(	m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				"�}�N���t�@�C�����쐬�ł��܂���ł����B\n�t�@�C�����̎擾�G���[ nRet=%d", nRet
			);
			return;
		}else{
			strcpy( m_pShareData->m_szKeyMacroFileName, szInitDir );
		}
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//		if ( FALSE == m_pcEditDoc->m_CKeyMacroMgr.SaveKeyMacro( m_hInstance, m_pShareData->m_szKeyMacroFileName ) ){
		if ( FALSE == m_pcEditDoc->m_pcSMacroMgr->Save( STAND_KEYMACRO, m_hInstance, m_pShareData->m_szKeyMacroFileName ) ){
			::MYMESSAGEBOX(	m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				"�}�N���t�@�C�����쐬�ł��܂���ł����B\n\n%s", m_pShareData->m_szKeyMacroFileName
			);
		}
	}else{
		m_pShareData->m_bRecordingKeyMacro = TRUE;
		m_pShareData->m_hwndRecordingKeyMacro = ::GetParent( m_hwndParent );;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
		/* �L�[�}�N���̃o�b�t�@���N���A���� */
		//@@@ 2002.1.24 m_CKeyMacroMgr��CEditDoc�ֈړ�
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
		m_pcEditDoc->m_pcSMacroMgr->Clear(STAND_KEYMACRO);
//		m_pcEditDoc->m_CKeyMacroMgr.ClearAll();
//		m_pShareData->m_CKeyMacroMgr.Clear();
	}
	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();

	/* �L�����b�g�̍s���ʒu��\������ */
	DrawCaretPosInfo();

	return;
}




/* �L�[�}�N���̕ۑ� */
void CEditView::Command_SAVEKEYMACRO( void )
{
	m_pShareData->m_bRecordingKeyMacro = FALSE;
	m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

	//	Jun. 16, 2002 genta
	if( !m_pcEditDoc->m_pcSMacroMgr->IsSaveOk() ){
		//	�ۑ��s��
		::MYMESSAGEBOX(	m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�ۑ��\�ȃ}�N��������܂���D�L�[�{�[�h�}�N���ȊO�͕ۑ��ł��܂���D" );
	}

	CDlgOpenFile	cDlgOpenFile;
	char			szPath[_MAX_PATH + 1];
	char			szInitDir[_MAX_PATH + 1];
	strcpy( szPath, "" );
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	if( _IS_REL_PATH( m_pShareData->m_szMACROFOLDER ) ){
		GetExecutableDir( szInitDir, m_pShareData->m_szMACROFOLDER );
	}else{
		strcpy( szInitDir, m_pShareData->m_szMACROFOLDER );	/* �}�N���p�t�H���_ */
	}
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		m_hInstance,
		m_hWnd,
		"*.mac",
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
//	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szMACROFOLDER, NULL );
//	strcat( m_pShareData->m_szMACROFOLDER, "\\" );

	/* �L�[�{�[�h�}�N���̕ۑ� */
	//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
	//@@@ 2002.1.24 YAZAKI
//	if ( FALSE == m_pcEditDoc->m_CKeyMacroMgr.SaveKeyMacro( m_hInstance, szPath ) ){
	if ( FALSE == m_pcEditDoc->m_pcSMacroMgr->Save( STAND_KEYMACRO, m_hInstance, szPath ) ){
		::MYMESSAGEBOX(	m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�}�N���t�@�C�����쐬�ł��܂���ł����B\n\n%s", szPath
		);
	}
//	m_pShareData->m_CKeyMacroMgr.SaveKeyMacro( m_hInstance, m_hWnd, szPath );
	return;
}




/* �L�[�}�N���̎��s */
void CEditView::Command_EXECKEYMACRO( void )
{
	//@@@ 2002.1.24 YAZAKI �L�^���͏I�����Ă�����s
	if (m_pShareData->m_bRecordingKeyMacro){
		Command_RECKEYMACRO();
	}
	m_pShareData->m_bRecordingKeyMacro = FALSE;
	m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

	/* �L�[�{�[�h�}�N���̎��s�� */
	m_bExecutingKeyMacro = TRUE;

	/* �L�[�{�[�h�}�N���̎��s */
	//@@@ 2002.1.24 YAZAKI
	if ( m_pShareData->m_szKeyMacroFileName[0] ){
		//	�t�@�C�����ۑ�����Ă�����
		//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//		if ( FALSE == m_pcEditDoc->m_CKeyMacroMgr.LoadKeyMacro( m_hInstance, m_pShareData->m_szKeyMacroFileName ) ){
		if ( FALSE == m_pcEditDoc->m_pcSMacroMgr->Load( STAND_KEYMACRO, m_hInstance, m_pShareData->m_szKeyMacroFileName ) ){
			::MYMESSAGEBOX(	m_hWnd, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				"�t�@�C�����J���܂���ł����B\n\n%s", m_pShareData->m_szKeyMacroFileName
			);
		}
		else {
//			m_pcEditDoc->m_CKeyMacroMgr.ExecKeyMacro( this );
			m_pcEditDoc->m_pcSMacroMgr->Exec( STAND_KEYMACRO, m_hInstance, this );
		}
	}
//	m_pShareData->m_CKeyMacroMgr.ExecKeyMacro( (void*)this );

	/* �L�[�{�[�h�}�N���̎��s�� */
	m_bExecutingKeyMacro = FALSE;

	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	RedrawAll();

	return;
}




/*! �L�[�}�N���̓ǂݍ���
	@date 2005/02/20 novice �f�t�H���g�̊g���q�ύX
 */
void CEditView::Command_LOADKEYMACRO( void )
{
	m_pShareData->m_bRecordingKeyMacro = FALSE;
	m_pShareData->m_hwndRecordingKeyMacro = NULL;	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */

	CDlgOpenFile	cDlgOpenFile;
	char			szPath[_MAX_PATH + 1];
	char			szInitDir[_MAX_PATH + 1];
	const char*		pszFolder;
	strcpy( szPath, "" );
	pszFolder = m_pShareData->m_szMACROFOLDER;
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X
	if( _IS_REL_PATH( pszFolder ) ){
		GetExecutableDir( szInitDir, pszFolder );
	}else{
		strcpy( szInitDir, pszFolder );	/* �}�N���p�t�H���_ */
	}
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		m_hInstance,
		m_hWnd,
// 2005/02/20 novice �f�t�H���g�̊g���q�ύX
// 2005/07/13 novice ���l�ȃ}�N�����T�|�[�g���Ă���̂Ńf�t�H���g�͑S�ĕ\���ɂ���
		"*.*",
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}

	/* �L�[�{�[�h�}�N���̓ǂݍ��� */
	//@@@ 2002.1.24 YAZAKI �ǂݍ��݂Ƃ������A�t�@�C�������R�s�[���邾���B���s���O�ɓǂݍ���
	strcpy(m_pShareData->m_szKeyMacroFileName, szPath);
//	m_pShareData->m_CKeyMacroMgr.LoadKeyMacro( m_hInstance, m_hWnd, szPath );
	return;
}

/*!	@brief �܂�Ԃ��̓��������

	�g�O���R�}���h�u���݂̃E�B���h�E���Ő܂�Ԃ��v���s�����ꍇ�̓�������肷��
	
	@retval TGWRAP_NONE No action
	@retval TGWRAP_FULL �ő�l
	@retval TGWRAP_WINDOW �E�B���h�E��
	@retval TGWRAP_PROP �ݒ�l

	@date 2006.01.08 genta ���j���[�\���œ���̔�����g�����߁CCommand_WRAPWINDOWWIDTH()��蕪���D
	@date 2006.01.08 genta ���������������
*/
CEditView::TOGGLE_WRAP_ACTION CEditView::GetWrapMode( int& newWidth )
{
	//@@@ 2002.01.14 YAZAKI ���݂̃E�B���h�E���Ő܂�Ԃ���Ă���Ƃ��́A�ő�l�ɂ���R�}���h�B
	//2002/04/08 YAZAKI �Ƃ��ǂ��E�B���h�E���Ő܂�Ԃ���Ȃ����Ƃ�����o�O�C���B
	// 20051022 aroka ���݂̃E�B���h�E�����ő�l�������^�C�v�̏����l ���g�O���ɂ���R�}���h
	// �E�B���h�E��==�����^�C�v||�ő�l==�����^�C�v �̏ꍇ�����邽�ߔ��菇���ɒ��ӂ���B
	/*	Jan.  8, 2006 genta
		���イ������̗v�]�ɂ�蔻����@���čl�D���݂̕��ɍ��킹��̂��ŗD��ɁD
	
		��{����F �ݒ�l���E�B���h�E��
			��(�E�B���h�E���ƍ����Ă��Ȃ����)���E�B���h�E������֖߂�
			��(�E�B���h�E���ƍ����Ă�����)���ő�l���ݒ�l
			�������C�ő�l==�ݒ�l�̏ꍇ�ɂ͍ő�l���ݒ�l�̑J�ڂ��ȗ�����ď�ɖ߂�
			
			�E�B���h�E�����ɒ[�ɋ����ꍇ�ɂ̓E�B���h�E���ɍ��킹�邱�Ƃ͏o���Ȃ����C
			�ݒ�l�ƍő�l�̃g�O���͉\�D

		1)���݂̐܂�Ԃ���==�E�B���h�E�� : �ő�l
		2)���݂̐܂�Ԃ���!=�E�B���h�E��
		3)���E�B���h�E�����ɒ[�ɋ����ꍇ
		4)�@�����܂�Ԃ���!=�ő�l : �ő�l
		5)�@�����܂�Ԃ���==�ő�l
		6)�@�@�@�����ő�l==�ݒ�l : �ύX�s�\
		7)�@�@�@�����ő�l!=�ݒ�l : �ݒ�l
		8)���E�B���h�E�����\���ɂ���
		9)�@�����܂�Ԃ���==�ő�l
		a)�@�@�@�����ő�l!=�ݒ�l : �ݒ�l
	 	b)�@�@�@�����ő�l==�ݒ�l : �E�B���h�E��
		c)�@�����E�B���h�E��
	*/
	
	if (m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() == m_nViewColNum ){
		// a)
		newWidth = MAXLINESIZE;
		return TGWRAP_FULL;
	}
	else if( 10 > m_nViewColNum - 1 ){ // 2)
		// 3)
		if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() != MAXLINESIZE ){
			// 4)
			newWidth = MAXLINESIZE;
			return TGWRAP_FULL;
		}
		else if( m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize == MAXLINESIZE ){ // 5)
			// 6)
			return TGWRAP_NONE;
		}
		else { // 7)
			newWidth = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize;
			return TGWRAP_PROP;
		}
	}
	else { // 8)
		if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() == MAXLINESIZE && // 9)
			m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize != MAXLINESIZE ){
			// a)
			newWidth = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize;
			return TGWRAP_PROP;
			
		}
		else {	// b) c)
			//	���݂̃E�B���h�E��
			newWidth = m_nViewColNum;
			return TGWRAP_WINDOW;
		}
	}
}

/*! ���݂̃E�B���h�E���Ő܂�Ԃ�

	@date 2002.01.14 YAZAKI ���݂̃E�B���h�E���Ő܂�Ԃ���Ă���Ƃ��́A�ő�l�ɂ���悤��
	@date 2002.04.08 YAZAKI �Ƃ��ǂ��E�B���h�E���Ő܂�Ԃ���Ȃ����Ƃ�����o�O�C���B
	@date 2005.08.14 genta �����ł̐ݒ�͋��ʐݒ�ɔ��f���Ȃ��D
	@date 2005.10.22 aroka ���݂̃E�B���h�E�����ő�l�������^�C�v�̏����l ���g�O���ɂ���

	@note �ύX���鏇����ύX�����Ƃ���CEditWnd::InitMenu()���ύX���邱��
	@sa CEditWnd::InitMenu()
*/
void CEditView::Command_WRAPWINDOWWIDTH( void )	//	Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
{
	// Jan. 8, 2006 genta ���菈����GetWrapMode()�ֈړ�
	int newWidth;
	
	if( GetWrapMode( newWidth ) == TGWRAP_NONE ){
		return;
	}

	m_pcEditDoc->ChangeLayoutParam( true, m_pcEditDoc->m_cLayoutMgr.GetTabSpace(), newWidth );
	

	//	Aug. 14, 2005 genta ���ʐݒ�ւ͔��f�����Ȃ�
//	m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize = m_nViewColNum;

//	m_pcEditDoc->OnChangeSetting();	/* �r���[�ɐݒ�ύX�𔽉f������ */

	/* �ݒ�ύX�𔽉f������ */
//	CShareData::getInstance()->SendMessageToAllEditors(
//		MYWM_CHANGESETTING, (WPARAM)0, (LPARAM)0, ::GetParent( m_hwndParent )
//	);	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */

	m_nViewLeftCol = 0;		/* �\����̈�ԍ��̌�(0�J�n) */

	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	RedrawAll();
	return;
}




//�����}�[�N�̐ؑւ�	// 2001.12.03 hor �N���A �� �ؑւ� �ɕύX
void CEditView::Command_SEARCH_CLEARMARK( void )
{
// From Here 2001.12.03 hor

	//�����}�[�N�̃Z�b�g

	if(IsTextSelected()){

		// ����������擾
		CMemory	cmemCurText;
		GetCurrentTextForSearch( cmemCurText );

		// ����������ݒ�
		int i,j;
		strcpy( m_szCurSrchKey, cmemCurText.GetPtr() );
		for( i = 0; i < m_pShareData->m_nSEARCHKEYArrNum; ++i ){
			if( 0 == strcmp( m_szCurSrchKey, m_pShareData->m_szSEARCHKEYArr[i] ) ){
				break;
			}
		}
		if( i < m_pShareData->m_nSEARCHKEYArrNum ){
			for( j = i; j > 0; j-- ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[j], m_pShareData->m_szSEARCHKEYArr[j - 1] );
			}
		}else{
			for( j = MAX_SEARCHKEY - 1; j > 0; j-- ){
				strcpy( m_pShareData->m_szSEARCHKEYArr[j], m_pShareData->m_szSEARCHKEYArr[j - 1] );
			}
			++m_pShareData->m_nSEARCHKEYArrNum;
			if( m_pShareData->m_nSEARCHKEYArrNum > MAX_SEARCHKEY ){
				m_pShareData->m_nSEARCHKEYArrNum = MAX_SEARCHKEY;
			}
		}
		strcpy( m_pShareData->m_szSEARCHKEYArr[0], cmemCurText.GetPtr() );

		// �����I�v�V�����ݒ�
		m_pShareData->m_Common.m_bRegularExp=0;	//���K�\���g��Ȃ�
		m_pShareData->m_Common.m_bWordOnly=0;	//�P��Ō������Ȃ�
//		m_bCurSrchKeyMark=TRUE;
		ChangeCurRegexp(); // 2002.11.11 Moca ���K�\���Ō���������C�F�������ł��Ă��Ȃ�����

		// �ĕ`��
		RedrawAll();
		return;
	}
// To Here 2001.12.03 hor

	//�����}�[�N�̃N���A

	m_bCurSrchKeyMark = FALSE;	/* ����������̃}�[�N */
	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	RedrawAll();
	return;
}




/*! @brief �t�@�C���̍ăI�[�v��
	
	@param nCharCode [in] �J�������ۂ̕����R�[�h
	@param bNoConfirm [in] �t�@�C�����X�V���ꂽ�ꍇ�Ɋm�F���s�����D
		0: �m�F����, !0: �m�F���Ȃ�
*/
void CEditView::Command_FILE_REOPEN( int nCharCode, int bNoConfirm )
{
	if( bNoConfirm == 0 && (  -1 != _access( m_pcEditDoc->GetFilePath(), 0 ))
	 && m_pcEditDoc->IsModified()	/* �ύX�t���O */
	){
		if( IDOK != MYMESSAGEBOX( m_hWnd, MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME,
			"%s\n\n���̃t�@�C���͕ύX����Ă��܂��B\n�ă��[�h���s���ƕύX�������܂����A��낵���ł���?",
			m_pcEditDoc->GetFilePath()
		) ){
			return;
		}
	}
	/* ����t�@�C���̍ăI�[�v�� */
	 m_pcEditDoc->ReloadCurrentFile(
		nCharCode,					/* �����R�[�h��� */
		m_pcEditDoc->m_bReadOnly	/* �ǂݎ���p���[�h */
	);
	/* �L�����b�g�̍s���ʒu��\������ */
	DrawCaretPosInfo();
	return;

}




//���t�}��
void CEditView::Command_INS_DATE( void )
{
	/* ���t���t�H�[�}�b�g */
	char szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CShareData::getInstance()->MyGetDateFormat( systime, szText, sizeof( szText ) - 1 );
	/* �e�L�X�g��\��t�� ver1 */
	Command_INSTEXT( TRUE, szText, -1, TRUE );
	return;
}




//�����}��
void CEditView::Command_INS_TIME( void )
{
	/* �������t�H�[�}�b�g */
	char szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CShareData::getInstance()->MyGetTimeFormat( systime, szText, sizeof( szText ) - 1 );
	/* �e�L�X�g��\��t�� ver1 */
	Command_INSTEXT( TRUE, szText, -1, TRUE );
	return;
}


/*! �O���R�}���h���s�_�C�A���O�\��
	@date 2002.02.02 YAZAKI.
*/
void CEditView::Command_EXECCOMMAND_DIALOG( const char *cmd )
{
	const char *cmd_string;	//	Oct. 9, 2001 genta
	CDlgExec cDlgExec;

	if( cmd == NULL ){
		/* ���[�h���X�_�C�A���O�̕\�� */
		if( FALSE == cDlgExec.DoModal( m_hInstance, m_hWnd, 0 ) ){
			return;
		}
	//	MYTRACE( "cDlgExec.m_szCommand=[%s]\n", cDlgExec.m_szCommand );

		AddToCmdArr( cDlgExec.m_szCommand );
		cmd_string = cDlgExec.m_szCommand;
	}
	else {
		cmd_string = cmd;
	}
	HandleCommand( F_EXECCOMMAND, TRUE, (LPARAM)cmd_string, 0, 0, 0);	//	�O���R�}���h���s�R�}���h�̔��s
}

//�O���R�}���h���s
//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	void CEditView::Command_EXECCMMAND( void )
//	Oct. 9, 2001 genta �}�N���Ή��̂��߈����ǉ�
//@@@ 2002.2.2 YAZAKI �_�C�A���O�Ăяo�����ƃR�}���h���s���𕪗�
void CEditView::Command_EXECCOMMAND( const char *cmd_string )
//	To Here Sept. 20, 2000
{
#if 0
	const char *cmd_string;	//	Oct. 9, 2001 genta
	CDlgExec cDlgExec;

	if( cmd == NULL ){
		/* ���[�h���X�_�C�A���O�̕\�� */
		if( FALSE == cDlgExec.DoModal( m_hInstance, m_hWnd, 0 ) ){
			return;
		}
	//	MYTRACE( "cDlgExec.m_szCommand=[%s]\n", cDlgExec.m_szCommand );

		AddToCmdArr( cDlgExec.m_szCommand );
		cmd_string = cDlgExec.m_szCommand;
	}
	else {
		cmd_string = cmd;
	}
#endif
	//	From Here Aug. 21, 2001 genta
	//	�p�����[�^�u�� (���b��)
	const int bufmax = 1024;
	char buf[bufmax + 1];
	m_pcEditDoc->ExpandParameter(cmd_string, buf, bufmax);
	
	// �q�v���Z�X�̕W���o�͂����_�C���N�g����
	ExecCmd( buf, m_pShareData->m_bGetStdout );
	//	To Here Aug. 21, 2001 genta
	return;
}




void CEditView::AddToCmdArr( const char* szCmd )
{
/*
//	CMemory*	pcmWork;
	int			i;
	int			j;
//	pcmWork = NULL;
//	pcmWork = new CMemory( szCmd, lstrlen( szCmd ) );
	
	for( i = 0; i < m_pShareData->m_nCmdArrNum; ++i ){
		if( 0 == strcmp( szCmd, m_pShareData->m_szCmdArr[i] ) ){
			break;
		}
	}
	if( i < m_pShareData->m_nCmdArrNum ){
		for( j = i; j > 0; j-- ){
			strcpy( m_pShareData->m_szCmdArr[j], m_pShareData->m_szCmdArr[j - 1] );
		}
	}else{
		for( j = MAX_CMDARR - 1; j > 0; j-- ){
			strcpy( m_pShareData->m_szCmdArr[j], m_pShareData->m_szCmdArr[j - 1] );
		}
		++m_pShareData->m_nCmdArrNum;
		if( m_pShareData->m_nCmdArrNum > MAX_CMDARR ){
			m_pShareData->m_nCmdArrNum = MAX_CMDARR;
		}
	}
	strcpy( m_pShareData->m_szCmdArr[0], szCmd );
//	delete pcmWork;
//	pcmWork = NULL;
*/
	CRecent	cRecentCmd;

	cRecentCmd.EasyCreate( RECENT_FOR_CMD );
	cRecentCmd.AppendItem( szCmd );
	cRecentCmd.Terminate();

	return;
}




//	Jun. 16, 2000 genta
//	�Ί��ʂ̌���
void CEditView::Command_BRACKETPAIR( void )
{
	int nLine, nCol;
	int mode = 3;
	/*
	bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
	bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
	bit2(out) : ���������ʒu         0:���      1:�O
	*/
	if( SearchBracket( m_nCaretPosX, m_nCaretPosY, &nCol, &nLine, &mode ) ){	// 02/09/18 ai
		//	2005.06.24 Moca
		//	2006.07.09 genta �\���X�V�R��F�V�K�֐��ɂđΉ�
		MoveCursorSelecting( nCol, nLine, m_bSelectingLock );
	}
	else{
		//	���s�����ꍇ�� nCol/nLine�ɂ͗L���Ȓl�������Ă��Ȃ�.
		//	�������Ȃ�
	}
}


//	���݈ʒu���ړ������ɓo�^����
void CEditView::Command_JUMPHIST_SET( void )
{
	AddCurrentLineToHistory();
}


//	From HERE Sep. 8, 2000 genta
//	�ړ�������O�ւ��ǂ�
//
void CEditView::Command_JUMPHIST_PREV( void )
{
	// 2001.12.13 hor
	// �ړ������̍Ō�Ɍ��݂̈ʒu���L������
	// ( ���̗������擾�ł��Ȃ��Ƃ��͒ǉ����Ė߂� )
	if( !m_cHistory->CheckNext() ){
		AddCurrentLineToHistory();
		m_cHistory->PrevValid();
	}

	if( m_cHistory->CheckPrev() ){
		int x, y;
		if( ! m_cHistory->PrevValid() ){
			::MessageBox( NULL, "Inconsistent Implementation", "PrevValid", MB_OK );
		}
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
			m_cHistory->GetCurrent().GetPos(),
			m_cHistory->GetCurrent().GetLine(),
			&x, &y );
		//	2005.06.24 Moca
		//MoveCursor( x, y, TRUE );
		//	2006.07.09 genta �I�����l��
		MoveCursorSelecting( x, y, m_bSelectingLock );
	}
}

//	�ړ����������ւ��ǂ�
void CEditView::Command_JUMPHIST_NEXT( void )
{
	if( m_cHistory->CheckNext() ){
		int x, y;
		if( ! m_cHistory->NextValid() ){
			::MessageBox( NULL, "Inconsistent Implementation", "NextValid", MB_OK );
		}
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
			m_cHistory->GetCurrent().GetPos(),
			m_cHistory->GetCurrent().GetLine(),
			&x, &y );
		//	2005.06.24 Moca
		//MoveCursor( x, y, TRUE );
		//	2006.07.09 genta �I�����l��
		MoveCursorSelecting( x, y, m_bSelectingLock );
	}
}
//	To HERE Sep. 8, 2000 genta


/*[EOF]*/
