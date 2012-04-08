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
#ifndef SAKURA_CVIEWCOMMANDER_5F4F7A80_2BEC_4B1D_A637_B922375FF14C9_H_
#define SAKURA_CVIEWCOMMANDER_5F4F7A80_2BEC_4B1D_A637_B922375FF14C9_H_

class CEditView;
enum EFunctionCode;
class CEditDoc;
struct DLLSHAREDATA;
class COpeBlk;
class CCaret;
class CEditWnd;
#include "CEol.h"
// #include "types/CType.h" // OUTLINE_DEFAULT

class CViewCommander{
public:
	CViewCommander(CEditView* pEditView) : m_pCommanderView(pEditView)
	{
		m_bPrevCommand = 0;
	}

public:
	//�O���ˑ�
	CEditDoc* GetDocument();
	CEditWnd* GetEditWindow();
	HWND GetMainWindow();
	COpeBlk* GetOpeBlk();
	void ClearOpeBlk();
	void SetOpeBlk(COpeBlk* p);
	CLayoutRange& GetSelect();
	CCaret& GetCaret();

private:
	CEditView*		m_pCommanderView;

public:
	/* �L�[���s�[�g��� */
	int				m_bPrevCommand;



private:
	enum EIndentType {
		INDENT_NONE,
		INDENT_TAB,
		INDENT_SPACE
	};

	// -- -- -- -- �ȉ��A�R�}���h�����֐��Q -- -- -- -- //
public:
	BOOL HandleCommand(
		EFunctionCode	nCommand,
		bool			bRedraw,
		LPARAM			lparam1,
		LPARAM			lparam2,
		LPARAM			lparam3,
		LPARAM			lparam4
	);

	/* �t�@�C������n */
	void Command_FILENEW( void );				/* �V�K�쐬 */
	void Command_FILENEW_NEWWINDOW( void );		/* �V�K�쐬�i�^�u�ŊJ���Łj */
	/* �t�@�C�����J�� */
	// Oct. 2, 2001 genta �}�N���p�ɋ@�\�g��
	// Mar. 30, 2003 genta �����ǉ�
	void Command_FILEOPEN(
		const WCHAR*	filename		= NULL,
		ECodeType		nCharCode		= CODE_AUTODETECT,
		bool			bViewMode	= false
	);
	
	/* �㏑���ۑ� */ // Feb. 28, 2004 genta �����ǉ�, Jan. 24, 2005 genta �����ǉ�
	bool Command_FILESAVE( bool warnbeep = true, bool askname = true );	
	bool Command_FILESAVEAS_DIALOG();									/* ���O��t���ĕۑ� */
	BOOL Command_FILESAVEAS( const WCHAR* filename, EEolType eEolType);	/* ���O��t���ĕۑ� */
	BOOL Command_FILESAVEALL( void );					/* �S�ď㏑���ۑ� */ // Jan. 23, 2005 genta
	void Command_FILECLOSE( void );						/* �J����(����) */	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	/* ���ĊJ��*/
	// Mar. 30, 2003 genta �����ǉ�
	void Command_FILECLOSE_OPEN( LPCWSTR filename = NULL,
		ECodeType nCharCode = CODE_AUTODETECT, bool bViewMode = false );
	
	void Command_FILE_REOPEN( ECodeType nCharCode, bool bNoConfirm );		/* �ăI�[�v�� */	//Dec. 4, 2002 genta �����ǉ�

	void Command_PRINT( void );					/* ���*/
	void Command_PRINT_PREVIEW( void );			/* ����v���r���[*/
	void Command_PRINT_PAGESETUP( void );		/* ����y�[�W�ݒ� */	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	BOOL Command_OPEN_HfromtoC( BOOL );			/* ������C/C++�w�b�_(�\�[�X)���J�� */	//Feb. 7, 2001 JEPRO �ǉ�
	BOOL Command_OPEN_HHPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* ������C/C++�w�b�_�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	BOOL Command_OPEN_CCPP( BOOL bCheckOnly, BOOL bBeepWhenMiss );				/* ������C/C++�\�[�X�t�@�C�����J�� */	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	void Command_ACTIVATE_SQLPLUS( void );		/* Oracle SQL*Plus���A�N�e�B�u�\�� */
	void Command_PLSQL_COMPILE_ON_SQLPLUS( void );/* Oracle SQL*Plus�Ŏ��s */
	void Command_BROWSE( void );				/* �u���E�Y */
	void Command_VIEWMODE( void );				/* �r���[���[�h */
	void Command_PROPERTY_FILE( void );			/* �t�@�C���̃v���p�e�B */
	void Command_EXITALLEDITORS( void );		/* �ҏW�̑S�I�� */	// 2007.02.13 ryoji �ǉ�
	void Command_EXITALL( void );				/* �T�N���G�f�B�^�̑S�I�� */	//Dec. 27, 2000 JEPRO �ǉ�
	BOOL Command_PUTFILE( LPCWSTR, ECodeType, int );	/* ��ƒ��t�@�C���̈ꎞ�o�� maru 2006.12.10 */
	BOOL Command_INSFILE( LPCWSTR, ECodeType, int );	/* �L�����b�g�ʒu�Ƀt�@�C���}�� maru 2006.12.10 */
	void Command_TEXTWRAPMETHOD( int );			/* �e�L�X�g�̐܂�Ԃ����@��ύX���� */		// 2008.05.30 nasukoji

	/* �ҏW�n */
	void Command_WCHAR( wchar_t );			/* �������� */ //2007.09.02 kobake Command_CHAR(char)��Command_WCHAR(wchar_t)�ɕύX
	void Command_IME_CHAR( WORD );			/* �S�p�������� */
	void Command_UNDO( void );				/* ���ɖ߂�(Undo) */
	void Command_REDO( void );				/* ��蒼��(Redo) */
	void Command_DELETE( void );			/* �J�[�\���ʒu�܂��͑I���G���A���폜 */
	void Command_DELETE_BACK( void );		/* �J�[�\���O���폜 */
	void Command_WordDeleteToStart( void );	/* �P��̍��[�܂ō폜 */
	void Command_WordDeleteToEnd( void );	/* �P��̉E�[�܂ō폜 */
	void Command_WordCut( void );			/* �P��؂��� */
	void Command_WordDelete( void );		/* �P��폜 */
	void Command_LineCutToStart( void );	//�s���܂Ő؂���(���s�P��)
	void Command_LineCutToEnd( void );		//�s���܂Ő؂���(���s�P��)
	void Command_LineDeleteToStart( void );	/* �s���܂ō폜(���s�P��) */
	void Command_LineDeleteToEnd( void );  	//�s���܂ō폜(���s�P��)
	void Command_CUT_LINE( void );			/* �s�؂���(�܂�Ԃ��P��) */
	void Command_DELETE_LINE( void );		/* �s�폜(�܂�Ԃ��P��) */
	void Command_DUPLICATELINE( void );		/* �s�̓�d��(�܂�Ԃ��P��) */
	void Command_INDENT( wchar_t cChar, EIndentType = INDENT_NONE ); /* �C���f���g ver 1 */
// From Here 2001.12.03 hor
//	void Command_INDENT( const char*, int );/* �C���f���g ver0 */
	void Command_INDENT( const wchar_t*, CLogicInt , EIndentType = INDENT_NONE );/* �C���f���g ver0 */
// To Here 2001.12.03 hor
	void Command_UNINDENT( wchar_t wcChar );	/* �t�C���f���g */
//	void Command_WORDSREFERENCE( void );	/* �P�ꃊ�t�@�����X */
	void Command_TRIM(BOOL);				// 2001.12.03 hor
	void Command_SORT(BOOL);				// 2001.12.06 hor
	void Command_MERGE(void);				// 2001.12.06 hor
	void Command_Reconvert(void);			/* ���j���[����̍ĕϊ��Ή� minfu 2002.04.09 */
	void Command_CtrlCode_Dialog(void);		/* �R���g���[���R�[�h�̓���(�_�C�A���O) */	//@@@ 2002.06.02 MIK


	/* �J�[�\���ړ��n */
	//	Oct. 24, 2001 genta �@�\�g���̂��߈����ǉ�
	void Command_MOVECURSOR(CLogicPoint pos, int option);
	void Command_MOVECURSORLAYOUT(CLayoutPoint pos, int option);
	int Command_UP( bool bSelect, bool bRepeat, int line = 0 );			/* �J�[�\����ړ� */
	int Command_DOWN( bool bSelect, bool bRepeat );			/* �J�[�\�����ړ� */
	int  Command_LEFT( bool, bool );			/* �J�[�\�����ړ� */
	void Command_RIGHT( bool bSelect, bool bIgnoreCurrentSelection, bool bRepeat );	/* �J�[�\���E�ړ� */
	void Command_UP2( bool bSelect );				/* �J�[�\����ړ��i�Q�s�Âj */
	void Command_DOWN2( bool bSelect );				/* �J�[�\�����ړ��i�Q�s�Âj */
	void Command_WORDLEFT( bool bSelect );			/* �P��̍��[�Ɉړ� */
	void Command_WORDRIGHT( bool bSelect );			/* �P��̉E�[�Ɉړ� */
	//	Oct. 29, 2001 genta �}�N�������@�\�g��
	void Command_GOLINETOP( bool bSelect, int lparam );	/* �s���Ɉړ��i�܂�Ԃ��P�ʁj */
	void Command_GOLINEEND( bool bSelect, int );		/* �s���Ɉړ��i�܂�Ԃ��P�ʁj */
//	void Command_ROLLDOWN( int );			/* �X�N���[���_�E�� */
//	void Command_ROLLUP( int );				/* �X�N���[���A�b�v */
	void Command_HalfPageUp( bool bSelect );			//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	void Command_HalfPageDown( bool bSelect );		//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	void Command_1PageUp( bool bSelect );			//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	void Command_1PageDown( bool bSelect );			//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	void Command_GOFILETOP( bool bSelect );			/* �t�@�C���̐擪�Ɉړ� */
	void Command_GOFILEEND( bool bSelect );			/* �t�@�C���̍Ō�Ɉړ� */
	void Command_CURLINECENTER( void );		/* �J�[�\���s���E�B���h�E������ */
	void Command_JUMPHIST_PREV(void);		// �ړ�����: �O��
	void Command_JUMPHIST_NEXT(void);		// �ړ�����: ����
	void Command_JUMPHIST_SET(void);		// ���݈ʒu���ړ������ɓo�^
	void Command_WndScrollDown(void);		// �e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	void Command_WndScrollUp(void);			// �e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o
	void Command_GONEXTPARAGRAPH( bool bSelect );	// ���̒i���֐i��
	void Command_GOPREVPARAGRAPH( bool bSelect );	// �O�̒i���֖߂�

	/* �I���n */
	bool Command_SELECTWORD( void );		/* ���݈ʒu�̒P��I�� */
	void Command_SELECTALL( void );			/* ���ׂđI�� */
	void Command_SELECTLINE( int lparam );	/* 1�s�I�� */	// 2007.10.13 nasukoji
	void Command_BEGIN_SELECT( void );		/* �͈͑I���J�n */

	/* ��`�I���n */
//	void Command_BOXSELECTALL( void );		/* ��`�ł��ׂđI�� */
	void Command_BEGIN_BOXSELECT( void );	/* ��`�͈͑I���J�n */
	int Command_UP_BOX( BOOL );				/* (��`�I��)�J�[�\����ړ� */

	/* �N���b�v�{�[�h�n */
	void Command_CUT( void );						/* �؂���i�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜�j*/
	void Command_COPY( bool, bool bAddCRLFWhenCopy, EEolType neweol = EOL_UNKNOWN );/* �R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[) */
	void Command_PASTE( int option );						/* �\��t���i�N���b�v�{�[�h����\��t���j*/
	void Command_PASTEBOX( int option );					/* ��`�\��t���i�N���b�v�{�[�h�����`�\��t���j*/
	//<< 2002/03/29 Azumaiya
	/* ��`�\��t���i�����n���ł̒���t���j*/
	void Command_PASTEBOX( const wchar_t *szPaste, int nPasteSize );
	//>> 2002/03/29 Azumaiya
//	void Command_INSTEXT( BOOL, const char*, int );	/* �e�L�X�g��\��t�� ver0 */
	//void Command_INSTEXT( BOOL, const char*, BOOL );/* �e�L�X�g��\��t�� ver1 */
	void Command_INSBOXTEXT( const wchar_t *, int ); // ��`�\��t��
	void Command_INSTEXT( bool bRedraw, const wchar_t*, CLogicInt, bool bNoWaitCursor, bool bLinePaste = false ); // 2004.05.14 Moca �e�L�X�g��\��t�� '\0'�Ή�
	void Command_ADDTAIL( const wchar_t* pszData, int nDataLen);	/* �Ō�Ƀe�L�X�g��ǉ� */
	void Command_COPYFILENAME( void );				/* ���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ */ //2002/2/3 aroka
	void Command_COPYPATH( void );					/* ���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[ */
	void Command_COPYTAG( void );					/* ���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[ */
	void Command_COPYLINES( void );					/* �I��͈͓��S�s�R�s�[ */
	void Command_COPYLINESASPASSAGE( void );		/* �I��͈͓��S�s���p���t���R�s�[ */
	void Command_COPYLINESWITHLINENUMBER( void );	/* �I��͈͓��S�s�s�ԍ��t���R�s�[ */
	void Command_CREATEKEYBINDLIST( void );			// �L�[���蓖�Ĉꗗ���R�s�[ //Sept. 15, 2000 JEPRO	Command_�̍������킩��Ȃ��̂ŎE���Ă���


	/* �}���n */
	void Command_INS_DATE( void );	//���t�}��
	void Command_INS_TIME( void );	//�����}��

	/* �ϊ��n */
	void Command_TOLOWER( void );				/* ������ */
	void Command_TOUPPER( void );				/* �啶�� */
	void Command_TOZENKAKUKATA( void );			/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	void Command_TOZENKAKUHIRA( void );			/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	void Command_TOHANKAKU( void );				/* �S�p�����p */
	void Command_TOHANKATA( void );				/* �S�p�J�^�J�i�����p�J�^�J�i */	//Aug. 29, 2002 ai
	void Command_TOZENEI( void );				/* ���p�p�����S�p�p�� */ //July. 30, 2001 Misaka
	void Command_TOHANEI( void );				/* �S�p�p�������p�p�� */ //@@@ 2002.2.11 YAZAKI
	void Command_HANKATATOZENKAKUKATA( void );	/* ���p�J�^�J�i���S�p�J�^�J�i */
	void Command_HANKATATOZENKAKUHIRA( void );	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	void Command_TABTOSPACE( void );			/* TAB���� */
	void Command_SPACETOTAB( void );			/* �󔒁�TAB */  //---- Stonee, 2001/05/27
	void Command_CODECNV_AUTO2SJIS( void );		/* �������ʁ�SJIS�R�[�h�ϊ� */
	void Command_CODECNV_EMAIL( void );			/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	void Command_CODECNV_EUC2SJIS( void );		/* EUC��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UNICODE2SJIS( void );	/* Unicode��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UNICODEBE2SJIS( void );	/* UnicodeBE��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UTF82SJIS( void );		/* UTF-8��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_UTF72SJIS( void );		/* UTF-7��SJIS�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2JIS( void );		/* SJIS��JIS�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2EUC( void );		/* SJIS��EUC�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2UTF8( void );		/* SJIS��UTF-8�R�[�h�ϊ� */
	void Command_CODECNV_SJIS2UTF7( void );		/* SJIS��UTF-7�R�[�h�ϊ� */
	void Command_BASE64DECODE( void );			/* Base64�f�R�[�h���ĕۑ� */
	void Command_UUDECODE( void );				/* uudecode���ĕۑ� */	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX

	/* �����n */
	void Command_SEARCH_BOX( void );					/* ����(�{�b�N�X) */	// 2006.06.04 yukihane
	void Command_SEARCH_DIALOG( void );					/* ����(�P�ꌟ���_�C�A���O) */
	void Command_SEARCH_NEXT( bool, bool, HWND, const WCHAR* );/* �������� */
	void Command_SEARCH_PREV( bool bRedraw, HWND );				/* �O������ */
	void Command_REPLACE_DIALOG( void );				/* �u��(�u���_�C�A���O) */
	void Command_REPLACE( HWND hwndParent );			/* �u��(���s) 2002/04/08 YAZAKI �e�E�B���h�E���w�肷��悤�ɕύX */
	void Command_REPLACE_ALL();							/* ���ׂĒu��(���s) */
	void Command_SEARCH_CLEARMARK( void );				/* �����}�[�N�̃N���A */
	void Command_JUMP_SRCHSTARTPOS( void );				/* �����J�n�ʒu�֖߂� */	// 02/06/26 ai


	void Command_GREP_DIALOG( void );					/* Grep�_�C�A���O�̕\�� */
	void Command_GREP( void );							/* Grep */
	void Command_JUMP_DIALOG( void );					/* �w��s�w�W�����v�_�C�A���O�̕\�� */
	void Command_JUMP( void );							/* �w��s�w�W�����v */
// From Here 2001.12.03 hor
	BOOL Command_FUNCLIST( int nAction, int nOutlineType );	/* �A�E�g���C����� */ // 20060201 aroka
// To Here 2001.12.03 hor
	// Apr. 03, 2003 genta �����ǉ�
	bool Command_TAGJUMP( bool bClose = false );		/* �^�O�W�����v�@�\ */
	void Command_TAGJUMPBACK( void );					/* �^�O�W�����v�o�b�N�@�\ */
	bool Command_TagJumpByTagsFileMsg( bool );				//�_�C���N�g�^�O�W�����v(�ʒm��)
	bool Command_TagJumpByTagsFile( void );				//�_�C���N�g�^�O�W�����v	//@@@ 2003.04.13 MIK

	bool Command_TagsMake( void );						//�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	bool Command_TagJumpByTagsFileKeyword( const wchar_t* keyword );	//	@@ 2005.03.31 MIK
	void Command_COMPARE( void );						/* �t�@�C�����e��r */
	void Command_Diff_Dialog( void );					/* DIFF�����\���_�C�A���O */	//@@@ 2002.05.25 MIK
	void Command_Diff( const WCHAR* szTmpFile2, int nFlgOpt );	/* DIFF�����\�� */	//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	void Command_Diff_Next( void );						/* ���̍����� */	//@@@ 2002.05.25 MIK
	void Command_Diff_Prev( void );						/* �O�̍����� */	//@@@ 2002.05.25 MIK
	void Command_Diff_Reset( void );					/* �����̑S���� */	//@@@ 2002.05.25 MIK
	void Command_BRACKETPAIR( void );					/* �Ί��ʂ̌��� */
// From Here 2001.12.03 hor
	void Command_BOOKMARK_SET( void );					/* �u�b�N�}�[�N�ݒ�E���� */
	void Command_BOOKMARK_NEXT( void );					/* ���̃u�b�N�}�[�N�� */
	void Command_BOOKMARK_PREV( void );					/* �O�̃u�b�N�}�[�N�� */
	void Command_BOOKMARK_RESET( void );				/* �u�b�N�}�[�N�̑S���� */
// To Here 2001.12.03 hor
	void Command_BOOKMARK_PATTERN( void );				// 2002.01.16 hor �w��p�^�[���Ɉ�v����s���}�[�N



	/* ���[�h�؂�ւ��n */
	void Command_CHGMOD_INS( void );	/* �}���^�㏑�����[�h�؂�ւ� */
	void Command_CHGMOD_EOL( EEolType );	/* ���͂�����s�R�[�h��ݒ� 2003.06.23 moca */
	void Command_CANCEL_MODE( int whereCursorIs = 0 );	/* �e�탂�[�h�̎����� */

	/* �ݒ�n */
	void Command_SHOWTOOLBAR( void );		/* �c�[���o�[�̕\��/��\�� */
	void Command_SHOWFUNCKEY( void );		/* �t�@���N�V�����L�[�̕\��/��\�� */
	void Command_SHOWTAB( void );			/* �^�u�̕\��/��\�� */	//@@@ 2003.06.10 MIK
	void Command_SHOWSTATUSBAR( void );		/* �X�e�[�^�X�o�[�̕\��/��\�� */
	void Command_TYPE_LIST( void );			/* �^�C�v�ʐݒ�ꗗ */
	void Command_OPTION_TYPE( void );		/* �^�C�v�ʐݒ� */
	void Command_OPTION( void );			/* ���ʐݒ� */
	void Command_FONT( void );				/* �t�H���g�ݒ� */
	void Command_WRAPWINDOWWIDTH( void );	/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
	void Command_Favorite( void );	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
	void Command_SET_QUOTESTRING( const wchar_t* );	//	Jan. 29, 2005 genta ���p���̐ݒ�
	void Command_SELECT_COUNT_MODE( int nMode );	/* �����J�E���g���@ */	//2009.07.06 syat

	/* �}�N���n */
	void Command_RECKEYMACRO( void );	/* �L�[�}�N���̋L�^�J�n�^�I�� */
	void Command_SAVEKEYMACRO( void );	/* �L�[�}�N���̕ۑ� */
	void Command_LOADKEYMACRO( void );	/* �L�[�}�N���̓ǂݍ��� */
	void Command_EXECKEYMACRO( void );	/* �L�[�}�N���̎��s */
	void Command_EXECEXTMACRO( const WCHAR* path, const WCHAR* type );	/* ���O���w�肵�ă}�N�����s */
//	From Here 2006.12.03 maru �����̊g���D
//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	void Command_EXECCMMAND( void );	/* �O���R�}���h���s */
	//	Oct. 9, 2001 genta �}�N���Ή��̂��ߋ@�\�g��
//	void Command_EXECCOMMAND_DIALOG( const WCHAR* cmd );	/* �O���R�}���h���s�_�C�A���O�\�� */
//	void Command_EXECCOMMAND( const WCHAR* cmd );	/* �O���R�}���h���s */
	void Command_EXECCOMMAND_DIALOG( void );	/* �O���R�}���h���s�_�C�A���O�\�� */	//	�����g���ĂȂ��݂����Ȃ̂�
	//	�}�N������̌Ăяo���ł̓I�v�V������ۑ������Ȃ����߁ACommand_EXECCOMMAND_DIALOG���ŏ������Ă����D
	void Command_EXECCOMMAND( LPCWSTR cmd, const int nFlgOpt );	/* �O���R�}���h���s */
//	To Here Sept. 20, 2000
//	To Here 2006.12.03 maru �����̊g��

	/* �J�X�^�����j���[ */
	void Command_MENU_RBUTTON( void );	/* �E�N���b�N���j���[ */
	int Command_CUSTMENU( int );		/* �J�X�^�����j���[�\�� */

	/* �E�B���h�E�n */
	void Command_SPLIT_V( void );		/* �㉺�ɕ��� */	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	void Command_SPLIT_H( void );		/* ���E�ɕ��� */	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	void Command_SPLIT_VH( void );		/* �c���ɕ��� */	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	void Command_WINCLOSE( void );		/* �E�B���h�E����� */
	void Command_FILECLOSEALL( void );	/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	void Command_BIND_WINDOW( void );	/* �������ĕ\�� */	//2004.07.14 Kazika �V�K�ǉ�
	void Command_CASCADE( void );		/* �d�˂ĕ\�� */
	void Command_TILE_V( void );		/* �㉺�ɕ��ׂĕ\�� */
	void Command_TILE_H( void );		/* ���E�ɕ��ׂĕ\�� */
	void Command_MAXIMIZE_V( void );	/* �c�����ɍő剻 */
	void Command_MAXIMIZE_H( void );	/* �������ɍő剻 */  //2001.02.10 by MIK
	void Command_MINIMIZE_ALL( void );	/* ���ׂčŏ��� */
	void Command_REDRAW( void );		/* �ĕ`�� */
	void Command_WIN_OUTPUT( void );	//�A�E�g�v�b�g�E�B���h�E�\��
	void Command_TRACEOUT( const wchar_t* outputstr , int );	//�}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\�� maru 2006.04.26
	void Command_WINTOPMOST( LPARAM );		// ��Ɏ�O�ɕ\�� 2004.09.21 Moca
	void Command_WINLIST( int nCommandFrom );		/* �E�B���h�E�ꗗ�|�b�v�A�b�v�\������ */	// 2006.03.23 fon // 2006.05.19 genta �����ǉ�
	void Command_GROUPCLOSE( void );	/* �O���[�v����� */		// 2007.06.20 ryoji
	void Command_NEXTGROUP( void );		/* ���̃O���[�v */			// 2007.06.20 ryoji
	void Command_PREVGROUP( void );		/* �O�̃O���[�v */			// 2007.06.20 ryoji
	void Command_TAB_MOVERIGHT( void );	/* �^�u���E�Ɉړ� */		// 2007.06.20 ryoji
	void Command_TAB_MOVELEFT( void );	/* �^�u�����Ɉړ� */		// 2007.06.20 ryoji
	void Command_TAB_SEPARATE( void );	/* �V�K�O���[�v */			// 2007.06.20 ryoji
	void Command_TAB_JOINTNEXT( void );	/* ���̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji
	void Command_TAB_JOINTPREV( void );	/* �O�̃O���[�v�Ɉړ� */	// 2007.06.20 ryoji
	void Command_TAB_CLOSEOTHER( void );/* ���̃^�u�ȊO����� */	// 2008.11.22 syat
	void Command_TAB_CLOSELEFT( void );	/* �������ׂĕ��� */		// 2008.11.22 syat
	void Command_TAB_CLOSERIGHT( void );/* �E�����ׂĕ��� */		// 2008.11.22 syat


	void Command_ToggleKeySearch( void );	/* �L�����b�g�ʒu�̒P���������������@�\ON-OFF */	// 2006.03.24 fon

	void Command_HOKAN( void );			/* ���͕⊮ */
	void Command_HELP_CONTENTS( void );	/* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO added
	void Command_HELP_SEARCH( void );	/* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO added
	void Command_MENU_ALLFUNC( void );	/* �R�}���h�ꗗ */
	void Command_EXTHELP1( void );		/* �O���w���v�P */
	//	Jul. 5, 2002 genta
	void Command_EXTHTMLHELP( const WCHAR* helpfile = NULL, const WCHAR* kwd = NULL );	/* �O��HTML�w���v */
	void Command_ABOUT( void );			/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	/* ���̑� */
//@@@ 2002.01.14 YAZAKI �s�g�p�̂���
//	void Command_SENDMAIL( void );		/* ���[�����M */

private:
	void DelCharForOverwrite( void );	// �㏑���p�̈ꕶ���폜	// 2009.04.11 ryoji
	CLogicInt ConvertEol(const wchar_t* pszText, CLogicInt nTextLen, wchar_t* pszConvertedText);
	bool Sub_PreProcTagJumpByTagsFile( TCHAR* szCurrentPath, int count ); // �^�O�W�����v�̑O����

};

#endif /* SAKURA_CVIEWCOMMANDER_5F4F7A80_2BEC_4B1D_A637_B922375FF14C9_H_ */
/*[EOF]*/
