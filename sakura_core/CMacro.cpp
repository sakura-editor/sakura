//	$Id$
/*!	@file
	�L�[�{�[�h�}�N��

	@author Norio Nakatani
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

#include "CMacro.h"
#include "funccode.h"

struct MacroFuncInfo {
	int		m_nFuncID;
	char*	m_pszFuncName;
};

MacroFuncInfo m_MacroFuncInfoArr[] =
{
//	F_DISABLE					0	//���g�p

	/* �t�@�C������n */
	F_FILENEW					, "FileNew",			//�V�K�쐬
	F_FILEOPEN					, "FileOpen",			//�J��
	F_FILESAVE					, "FileSave",			//�㏑���ۑ�
	F_FILESAVEAS				, "FileSaveAs",			//���O��t���ĕۑ�
	F_FILECLOSE					, "FileClose",			//����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	F_FILECLOSE_OPEN			, "FileCloseOpen",		//���ĊJ��
	F_FILE_REOPEN_SJIS			, "FileReopenSJIS",		//SJIS�ŊJ������
	F_FILE_REOPEN_JIS			, "FileReopenJIS",		//JIS�ŊJ������
	F_FILE_REOPEN_EUC			, "FileReopenEUC",		//EUC�ŊJ������
	F_FILE_REOPEN_UNICODE		, "FileReopenUNICODE",	//Unicode�ŊJ������
	F_FILE_REOPEN_UTF8			, "FileReopenUTF8",		//UTF-8�ŊJ������
	F_FILE_REOPEN_UTF7			, "FileReopenUTF7",		//UTF-7�ŊJ������
	F_PRINT						, "Print",				//���
//	F_PRINT_DIALOG				, "PrintDialog",		//����_�C�A���O
	F_PRINT_PREVIEW				, "PrintPreview",		//����v���r���[
	F_PRINT_PAGESETUP			, "PrintPageSetup",		//����y�[�W�ݒ�	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	F_OPEN_HfromtoC				, "OpenHfromtoC",		//������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
	F_OPEN_HHPP					, "OpenHHpp",			//������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	F_OPEN_CCPP					, "OpenCCpp",			//������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	F_ACTIVATE_SQLPLUS			, "ActivateSQLPLUS",	/* Oracle SQL*Plus���A�N�e�B�u�\�� */
	F_PLSQL_COMPILE_ON_SQLPLUS	, "ExecSQLPLUS",		/* Oracle SQL*Plus�Ŏ��s */
	F_BROWSE					, "Browse",				//�u���E�Y
	F_PROPERTY_FILE				, "PropertyFile",		//�t�@�C���̃v���p�e�B
	F_EXITALL					, "ExitAll",			//�T�N���G�f�B�^�̑S�I��	//Dec. 27, 2000 JEPRO �ǉ�

	/* �ҏW�n */
	F_CHAR						, "Char",				//��������
	F_IME_CHAR					, "CharIme",			//�S�p��������
	F_UNDO						, "Undo",				//���ɖ߂�(Undo)
	F_REDO						, "Redo",				//��蒼��(Redo)
	F_DELETE					, "Delete",				//�폜
	F_DELETE_BACK				, "DeleteBack",			//�J�[�\���O���폜
	F_WordDeleteToStart			, "WordDeleteToStart",	//�P��̍��[�܂ō폜
	F_WordDeleteToEnd			, "WordDeleteToEnd",	//�P��̉E�[�܂ō폜
	F_WordCut					, "WordCut",			//�P��؂���
	F_WordDelete				, "WordDleete",			//�P��폜
	F_LineCutToStart			, "LineCutToStart",		//�s���܂Ő؂���(���s�P��)
	F_LineCutToEnd				, "LineCutToEnd",		//�s���܂Ő؂���(���s�P��)
	F_LineDeleteToStart			, "LineDeleteToStart",	//�s���܂ō폜(���s�P��)
	F_LineDeleteToEnd			, "LineDeleteToEnd",	//�s���܂ō폜(���s�P��)
	F_CUT_LINE					, "CutLine",			//�s�؂���(�܂�Ԃ��P��)
	F_DELETE_LINE				, "DeleteLine",			//�s�폜(�܂�Ԃ��P��)
	F_DUPLICATELINE				, "DuplicateLine",		//�s�̓�d��(�܂�Ԃ��P��)
	F_INDENT_TAB				, "IndentTab",			//TAB�C���f���g
	F_UNINDENT_TAB				, "UnindentTab",		//�tTAB�C���f���g
	F_INDENT_SPACE				, "IndentSpace",		//SPACE�C���f���g
	F_UNINDENT_SPACE			, "UnindentSpace",		//�tSPACE�C���f���g
	F_WORDSREFERENCE			, "WordReference",		//�P�ꃊ�t�@�����X

	/* �J�[�\���ړ��n */
	F_UP						, "Up",				//�J�[�\����ړ�
	F_DOWN						, "Down",			//�J�[�\�����ړ�
	F_LEFT						, "Left",			//�J�[�\�����ړ�
	F_RIGHT						, "Right",			//�J�[�\���E�ړ�
	F_UP2						, "Up2",			//�J�[�\����ړ�(�Q�s����)
	F_DOWN2						, "Down2",			//�J�[�\�����ړ�(�Q�s����)
	F_WORDLEFT					, "WordLeft",		//�P��̍��[�Ɉړ�
	F_WORDRIGHT					, "WordRight",		//�P��̉E�[�Ɉړ�
	F_GOLINETOP					, "GoLineTop",		//�s���Ɉړ�(�܂�Ԃ��P��)
	F_GOLINEEND					, "GoLineEnd",		//�s���Ɉړ�(�܂�Ԃ��P��)
//	F_ROLLDOWN					, "RollDown",		//�X�N���[���_�E��
//	F_ROLLUP					, "RollUp",			//�X�N���[���A�b�v
	F_HalfPageUp				, "HalfPageUp",		//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	F_HalfPageDown				, "HalfPageDown",	//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	F_1PageUp					, "1PageUp",		//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	F_1PageDown					, "1PageDown",		//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	F_GOFILETOP					, "GoFileTop",		//�t�@�C���̐擪�Ɉړ�
	F_GOFILEEND					, "GoFileEnd",		//�t�@�C���̍Ō�Ɉړ�
	F_CURLINECENTER				, "CurLineCenter",	//�J�[�\���s���E�B���h�E������
	F_JUMPPREV					, "MoveHistPrev",	//�ړ�����: �O��
	F_JUMPNEXT					, "MoveHistNext",	//�ړ�����: ����
	F_WndScrollDown				, "F_WndScrollDown",//�e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	F_WndScrollUp				, "F_WndScrollUp",	//�e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o

	/* �I���n */	//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v�������Ȃ����̂Łu�I���n�v�Ƃ��ēƗ���(�T�u���j���[���͍\����ł��Ȃ��̂�)
	F_SELECTWORD				, "SelectWord",			//���݈ʒu�̒P��I��
	F_SELECTALL					, "SelectAll",			//���ׂđI��
	F_BEGIN_SEL					, "BeginSelect",		//�͈͑I���J�n Mar. 5, 2001 genta ���̏C��
	F_UP_SEL					, "Up_Sel",				//(�͈͑I��)�J�[�\����ړ�
	F_DOWN_SEL					, "Down_Sel",			//(�͈͑I��)�J�[�\�����ړ�
	F_LEFT_SEL					, "Left_Sel",			//(�͈͑I��)�J�[�\�����ړ�
	F_RIGHT_SEL					, "Right_Sel",			//(�͈͑I��)�J�[�\���E�ړ�
	F_UP2_SEL					, "Up2_Sel",			//(�͈͑I��)�J�[�\����ړ�(�Q�s����)
	F_DOWN2_SEL					, "Down2_Sel",			//(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
	F_WORDLEFT_SEL				, "WordLeft_Sel",		//(�͈͑I��)�P��̍��[�Ɉړ�
	F_WORDRIGHT_SEL				, "WordRight_Sel",		//(�͈͑I��)�P��̉E�[�Ɉړ�
	F_GOLINETOP_SEL				, "GoLineTop_Sel",		//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	F_GOLINEEND_SEL				, "GoLineEnd_Sel",		//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
//	F_ROLLDOWN_SEL				, "RollDown_Sel",		//(�͈͑I��)�X�N���[���_�E��
//	F_ROLLUP_SEL				, "RollUp_Sel",			//(�͈͑I��)�X�N���[���A�b�v
	F_HalfPageUp_Sel			, "HalfPageUp_Sel",		//(�͈͑I��)���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	F_HalfPageDown_Sel			, "HalfPageDown_Sel",	//(�͈͑I��)���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	F_1PageUp_Sel				, "1PageUp_Sel",		//(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	F_1PageDown_Sel				, "1PageDown_Sel",		//(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	F_GOFILETOP_SEL				, "GoFileTop_Sel",		//(�͈͑I��)�t�@�C���̐擪�Ɉړ�
	F_GOFILEEND_SEL				, "GoFileEnd_Sel",		//(�͈͑I��)�t�@�C���̍Ō�Ɉړ�

	/* ��`�I���n */	//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
//	case F_BOXSELALL			, "BoxSelectAll",		//��`�ł��ׂđI��
	F_BEGIN_BOX					, "BeginBoxSelect",		//��`�͈͑I���J�n
/*
	F_UP_BOX					, "Up_Box", 			//(��`�I��)�J�[�\����ړ�
	F_DOWN_BOX					, "Down_Box",			//(��`�I��)�J�[�\�����ړ�
	F_LEFT_BOX					, "Left_Box",			//(��`�I��)�J�[�\�����ړ�
	F_RIGHT_BOX					, "Right_Box",			//(��`�I��)�J�[�\���E�ړ�
	F_UP2_BOX					, "Up2_Box",			//(��`�I��)�J�[�\����ړ�(�Q�s����)
	F_DOWN2_BOX					, "Down2_Box",			//(��`�I��)�J�[�\�����ړ�(�Q�s����)
	F_WORDLEFT_BOX				, "WordLeft_Box",		//(��`�I��)�P��̍��[�Ɉړ�
	F_WORDRIGHT_BOX				, "WordRight_Box",		//(��`�I��)�P��̉E�[�Ɉړ�
	F_GOLINETOP_BOX				, "GoLineTop_Box",		//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
	F_GOLINEEND_BOX				, "GoLineEnd_Box",		//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
	F_HalfPageUp_Box			, "HalfPageUp_Box",		//(��`�I��)���y�[�W�A�b�v
	F_HalfPageDown_Box			, "HalfPAgeDown_Box",	//(��`�I��)���y�[�W�_�E��
	F_1PageUp_Box				, "1PageUp_Box",		//(��`�I��)�P�y�[�W�A�b�v
	F_1PageDown_Box				, "1PageDown_Box",		//(��`�I��)�P�y�[�W�_�E��
	F_GOFILETOP_BOX				, "GoFileTop_Box",		//(��`�I��)�t�@�C���̐擪�Ɉړ�
	F_GOFILEEND_BOX				, "GoFileEnd_Box",		//(��`�I��)�t�@�C���̍Ō�Ɉړ�
*/

	/* �N���b�v�{�[�h�n */
	F_CUT						, "Cut",					//�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	F_COPY						, "Copy",					//�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	F_PASTE						, "Paste",					//�\��t��(�N���b�v�{�[�h����\��t��)
	F_COPY_CRLF					, "CopyCRLF",				//CRLF���s�ŃR�s�[(�I��͈͂����s�R�[�h=CRLF�ŃR�s�[)
	F_PASTEBOX					, "PasteBox",				//��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	F_INSTEXT					, "InsText",				// �e�L�X�g��\��t��
	F_ADDTAIL					, "AddTail",				// �Ō�Ƀe�L�X�g��ǉ�
	F_COPYLINES					, "CopyLines",				//�I��͈͓��S�s�R�s�[
	F_COPYLINESASPASSAGE		, "CopyLinesAsPassage",		//�I��͈͓��S�s���p���t���R�s�[
	F_COPYLINESWITHLINENUMBER 	, "CopyLinesWithLineNumber",//�I��͈͓��S�s�s�ԍ��t���R�s�[
	F_COPYPATH					, "CopyPath",				//���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	F_COPYTAG					, "CopyTag",				//���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[	//Sept. 15, 2000 jepro ��Ɠ��������ɂȂ��Ă����̂��C��
	F_CREATEKEYBINDLIST			, "CopyKeyBindList",		//�L�[���蓖�Ĉꗗ���R�s�[	//Sept. 15, 2000 JEPRO �ǉ� //Dec. 25, 2000 ����

	/* �}���n */
	F_INS_DATE					, "InsertDate",			// ���t�}��
	F_INS_TIME					, "InsertTime",			// �����}��

	/* �ϊ��n */
	F_TOLOWER		 			, "ToLower",			//�p�啶�����p������
	F_TOUPPER		 			, "ToUpper",			//�p���������p�啶��
	F_TOHANKAKU		 			, "ToHankaku",			/* �S�p�����p */
	F_TOZENEI		 			, "ToZenEi",			/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	F_TOZENKAKUKATA	 			, "ToZenKata",			/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	F_TOZENKAKUHIRA	 			, "ToZenHira",			/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	F_HANKATATOZENKAKUKATA		, "HanKataToZenKata",	/* ���p�J�^�J�i���S�p�J�^�J�i */
	F_HANKATATOZENKAKUHIRA		, "HanKataToZenHira",	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	F_TABTOSPACE				, "TABToSPACE",			/* TAB���� */
	F_SPACETOTAB				, "SPACEToTAB",			/* �󔒁�TAB */ //#### Stonee, 2001/05/27
	F_CODECNV_AUTO2SJIS			, "AutoToSJIS",			/* �������ʁ�SJIS�R�[�h�ϊ� */
	F_CODECNV_EMAIL				, "JIStoSJIS",			//E-Mail(JIS��SJIS)�R�[�h�ϊ�
	F_CODECNV_EUC2SJIS			, "EUCtoSJIS",			//EUC��SJIS�R�[�h�ϊ�
	F_CODECNV_UNICODE2SJIS		, "CodeCnvUNICODEtoJIS",//Unicode��SJIS�R�[�h�ϊ�
	F_CODECNV_UTF82SJIS			, "UTF8toSJIS",			/* UTF-8��SJIS�R�[�h�ϊ� */
	F_CODECNV_UTF72SJIS			, "UTF7toSJIS",			/* UTF-7��SJIS�R�[�h�ϊ� */
	F_CODECNV_SJIS2JIS			, "SJIStoJIS",			/* SJIS��JIS�R�[�h�ϊ� */
	F_CODECNV_SJIS2EUC			, "SJIStoEUC",			/* SJIS��EUC�R�[�h�ϊ� */
	F_CODECNV_SJIS2UTF8			, "SJIStoUTF8",			/* SJIS��UTF-8�R�[�h�ϊ� */
	F_CODECNV_SJIS2UTF7			, "SJIStoUTF7",			/* SJIS��UTF-7�R�[�h�ϊ� */
	F_BASE64DECODE	 			, "Base64Decode",		//Base64�f�R�[�h���ĕۑ�
	F_UUDECODE		 			, "Uudecode",			//uudecode���ĕۑ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX


	/* �����n */
	F_SEARCH_DIALOG				, "SearchDialog",		//����(�P�ꌟ���_�C�A���O)
	F_SEARCH_NEXT				, "SearchNext",			//��������
	F_SEARCH_PREV				, "SearchPrev",			//�O������
	F_REPLACE					, "Replace",			//�u��(�u���_�C�A���O)
	F_SEARCH_CLEARMARK			, "SearchClearMark",	//�����}�[�N�̃N���A
	F_GREP						, "Grep",				//Grep
	F_JUMP						, "Jump",				//�w��s�w�W�����v
	F_OUTLINE					, "Outline",			//�A�E�g���C�����
	F_TAGJUMP					, "TagJump",			//�^�O�W�����v�@�\
	F_TAGJUMPBACK				, "TagJumpBack",		//�^�O�W�����v�o�b�N�@�\
	F_COMPARE					, "Compare",			//�t�@�C�����e��r
	F_BRACKETPAIR				, "BracketPair",		//�Ί��ʂ̌���

	/* ���[�h�؂�ւ��n */
	F_CHGMOD_INS				, "ChgmodINS",		//�}���^�㏑�����[�h�؂�ւ�
	F_CANCEL_MODE				, "CancelMode",		//�e�탂�[�h�̎�����

	/* �ݒ�n */
	F_SHOWTOOLBAR				, "ShowToolbar",	/* �c�[���o�[�̕\�� */
	F_SHOWFUNCKEY				, "ShowFunckey",	/* �t�@���N�V�����L�[�̕\�� */
	F_SHOWSTATUSBAR				, "ShowStatusbar",	/* �X�e�[�^�X�o�[�̕\�� */
	F_TYPE_LIST					, "TypeList",		/* �^�C�v�ʐݒ�ꗗ */
	F_OPTION_TYPE				, "OptionType",		/* �^�C�v�ʐݒ� */
	F_OPTION					, "OptionCommon",	/* ���ʐݒ� */
	F_FONT						, "SelectFont",		/* �t�H���g�ݒ� */
	F_WRAPWINDOWWIDTH			, "WrapWindowWidth",/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX

	/* �J�X�^�����j���[ */
	F_MENU_RBUTTON				, "RMenu",			/* �E�N���b�N���j���[ */
	F_CUSTMENU_1				, "CustMenu1",		/* �J�X�^�����j���[1 */
	F_CUSTMENU_2				, "CustMenu2",		/* �J�X�^�����j���[2 */
	F_CUSTMENU_3				, "CustMenu3",		/* �J�X�^�����j���[3 */
	F_CUSTMENU_4				, "CustMenu4",		/* �J�X�^�����j���[4 */
	F_CUSTMENU_5				, "CustMenu5",		/* �J�X�^�����j���[5 */
	F_CUSTMENU_6				, "CustMenu6",		/* �J�X�^�����j���[6 */
	F_CUSTMENU_7				, "CustMenu7",		/* �J�X�^�����j���[7 */
	F_CUSTMENU_8				, "CustMenu8",		/* �J�X�^�����j���[8 */
	F_CUSTMENU_9				, "CustMenu9",		/* �J�X�^�����j���[9 */
	F_CUSTMENU_10				, "CustMenu10",		/* �J�X�^�����j���[10 */
	F_CUSTMENU_11				, "CustMenu11",		/* �J�X�^�����j���[11 */
	F_CUSTMENU_12				, "CustMenu12",		/* �J�X�^�����j���[12 */
	F_CUSTMENU_13				, "CustMenu13",		/* �J�X�^�����j���[13 */
	F_CUSTMENU_14				, "CustMenu14",		/* �J�X�^�����j���[14 */
	F_CUSTMENU_15				, "CustMenu15",		/* �J�X�^�����j���[15 */
	F_CUSTMENU_16				, "CustMenu16",		/* �J�X�^�����j���[16 */
	F_CUSTMENU_17				, "CustMenu17", 	/* �J�X�^�����j���[17 */
	F_CUSTMENU_18				, "CustMenu18",		/* �J�X�^�����j���[18 */
	F_CUSTMENU_19				, "CustMenu19",		/* �J�X�^�����j���[19 */
	F_CUSTMENU_20				, "CustMenu20",		/* �J�X�^�����j���[20 */
	F_CUSTMENU_21				, "CustMenu21",		/* �J�X�^�����j���[21 */
	F_CUSTMENU_22				, "CustMenu22",		/* �J�X�^�����j���[22 */
	F_CUSTMENU_23				, "CustMenu23",		/* �J�X�^�����j���[23 */
	F_CUSTMENU_24				, "CustMenu24",		/* �J�X�^�����j���[24 */

	/* �E�B���h�E�n */
	F_SPLIT_V					, "SplitWinV",			//�㉺�ɕ���	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	F_SPLIT_H					, "SplitWinH",			//���E�ɕ���	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	F_SPLIT_VH					, "SplitWinVH",			//�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	F_WINCLOSE					, "WinClose",			//�E�B���h�E�����
	F_WIN_CLOSEALL				, "WinCloseAll",		//���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
	F_CASCADE					, "CascadeWin",			//�d�˂ĕ\��
	F_TILE_V					, "TileWinV",			//�㉺�ɕ��ׂĕ\��
	F_TILE_H					, "TileWinH",			//���E�ɕ��ׂĕ\��
	F_NEXTWINDOW				, "NextWindow",			//���̃E�B���h�E
	F_PREVWINDOW				, "PrevWindow",			//�O�̃E�B���h�E
	F_MAXIMIZE_V				, "MaximizeV",			//�c�����ɍő剻
	F_MAXIMIZE_H				, "MaximizeH",			//�������ɍő剻 //2001.02.10 by MIK
	F_MINIMIZE_ALL				, "MinimizeAll",		//���ׂčŏ���	//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	F_REDRAW					, "ReDraw",				//�ĕ`��
	F_WIN_OUTPUT				, "ActivateWinOutput",	//�A�E�g�v�b�g�E�B���h�E�\��

	/* �x�� */
	F_HOKAN						, "Complete",		/* ���͕⊮ */	//Oct. 15, 2000 JEPRO �����ĂȂ������̂ŉp����t���ē���Ă݂�
	F_HELP_CONTENTS				, "HelpContents",	/* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO �ǉ�
	F_HELP_SEARCH				, "HelpSearch",		/* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO �ǉ�
	F_MENU_ALLFUNC				, "CommandList",	/* �R�}���h�ꗗ */
	F_EXTHELP1					, "ExtHelp1",		/* �O���w���v�P */
	F_EXTHTMLHELP				, "ExtHtmlHelp",	/* �O��HTML�w���v */
	F_ABOUT						, "About",			/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	/* ���̑� */
//	F_SENDMAIL					, "SendMail",		/* ���[�����M */	//Oct. 17, 2000 JEPRO ���[���@�\�͎���ł���̂ŃR�����g�A�E�g�ɂ���

};
int	m_nMacroFuncInfoArrNum = sizeof( m_MacroFuncInfoArr ) / sizeof( m_MacroFuncInfoArr[0] );



CMacro::CMacro( void )
{
	return;
}


CMacro::~CMacro( void )
{
	return;
}

/*
||  Attributes & Operations
*/
/* �@�\ID���֐����C�@�\�����{�� */
char* CMacro::GetFuncInfoByID( HINSTANCE hInstance, int nFincID, char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	for( i = 0; i < m_nMacroFuncInfoArrNum; ++i ){
		if( m_MacroFuncInfoArr[i].m_nFuncID == nFincID ){
			strcpy( pszFuncName, m_MacroFuncInfoArr[i].m_pszFuncName );
			::LoadString( hInstance, nFincID, pszFuncNameJapanese, 255 );
			return pszFuncName;
		}
	}
	return NULL;
}

/* �֐������@�\ID�C�@�\�����{�� */
int CMacro::GetFuncInfoByName( HINSTANCE hInstance, const char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	int		nFincID;
	for( i = 0; i < m_nMacroFuncInfoArrNum; ++i ){
		if( 0 == strcmp( pszFuncName, m_MacroFuncInfoArr[i].m_pszFuncName ) ){
			nFincID = m_MacroFuncInfoArr[i].m_nFuncID;
			::LoadString( hInstance, nFincID, pszFuncNameJapanese, 255 );
			return nFincID;
		}
	}
	return -1;
}

/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
BOOL CMacro::CanFuncIsKeyMacro( int nFuncID )
{
	switch( nFuncID ){
	/* �t�@�C������n */
//	case F_FILENEW					://�V�K�쐬
//	case F_FILEOPEN					://�J��
//	case F_FILESAVE					://�㏑���ۑ�
//	case F_FILESAVEAS				://���O��t���ĕۑ�
//	case F_FILECLOSE				://����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
//	case F_FILECLOSE_OPEN			://���ĊJ��
//	case F_FILE_REOPEN_SJIS			://SJIS�ŊJ������
//	case F_FILE_REOPEN_JIS			://JIS�ŊJ������
//	case F_FILE_REOPEN_EUC			://EUC�ŊJ������
//	case F_FILE_REOPEN_UNICODE		://Unicode�ŊJ������
//	case F_FILE_REOPEN_UTF8			://UTF-8�ŊJ������
//	case F_FILE_REOPEN_UTF7			://UTF-7�ŊJ������
//	case F_PRINT					://���
//	case F_PRINT_DIALOG				://����_�C�A���O
//	case F_PRINT_PREVIEW			://����v���r���[
//	case F_PRINT_PAGESETUP			://����y�[�W�ݒ�	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
//	case F_OPEN_HfromtoC:			://������C/C++�w�b�_(�\�[�X)���J��	//Feb. 9, 2001 JEPRO �ǉ�
//	case F_OPEN_HHPP				://������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
//	case F_OPEN_CCPP				://������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
//	case F_ACTIVATE_SQLPLUS			:/* Oracle SQL*Plus���A�N�e�B�u�\�� */
//	case F_PLSQL_COMPILE_ON_SQLPLUS	:/* Oracle SQL*Plus�Ŏ��s */	//Sept. 17, 2000 jepro �����́u�R���p�C���v���u���s�v�ɓ���
///	case F_BROWSE					://�u���E�Y
//	case F_PROPERTY_FILE			://�t�@�C���̃v���p�e�B
//	case F_EXITALL					://�T�N���G�f�B�^�̑S�I��	//Dec. 27, 2000 JEPRO �ǉ�

	/* �ҏW�n */
	case F_CHAR						://��������
	case F_IME_CHAR					://�S�p��������
	case F_UNDO						://���ɖ߂�(Undo)
	case F_REDO						://��蒼��(Redo)
	case F_DELETE					://�폜
	case F_DELETE_BACK				://�J�[�\���O���폜
	case F_WordDeleteToStart		://�P��̍��[�܂ō폜
	case F_WordDeleteToEnd			://�P��̉E�[�܂ō폜
	case F_WordCut					://�P��؂���
	case F_WordDelete				://�P��폜
	case F_LineCutToStart			://�s���܂Ő؂���(���s�P��)
	case F_LineCutToEnd				://�s���܂Ő؂���(���s�P��)
	case F_LineDeleteToStart		://�s���܂ō폜(���s�P��)
	case F_LineDeleteToEnd			://�s���܂ō폜(���s�P��)
	case F_CUT_LINE					://�s�؂���(�܂�Ԃ��P��)
	case F_DELETE_LINE				://�s�폜(�܂�Ԃ��P��)
	case F_DUPLICATELINE			://�s�̓�d��(�܂�Ԃ��P��)
	case F_INDENT_TAB				://TAB�C���f���g
	case F_UNINDENT_TAB				://�tTAB�C���f���g
	case F_INDENT_SPACE				://SPACE�C���f���g
	case F_UNINDENT_SPACE			://�tSPACE�C���f���g

	/* �J�[�\���ړ��n */
	case F_UP						://�J�[�\����ړ�
	case F_DOWN						://�J�[�\�����ړ�
	case F_LEFT						://�J�[�\�����ړ�
	case F_RIGHT					://�J�[�\���E�ړ�
//	case F_ROLLDOWN					://�X�N���[���_�E��
//	case F_ROLLUP					://�X�N���[���A�b�v
//	case F_HalfPageUp				://���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//	case F_HalfPageDown				://���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//	case F_1PageUp					://�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
//	case F_1PageDown				://�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	case F_UP2						://�J�[�\����ړ�(�Q�s����)
	case F_DOWN2					://�J�[�\�����ړ�(�Q�s����)
	case F_GOLINETOP				://�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOLINEEND				://�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOFILETOP				://�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND				://�t�@�C���̍Ō�Ɉړ�
	case F_WORDLEFT					://�P��̍��[�Ɉړ�
	case F_WORDRIGHT				://�P��̉E�[�Ɉړ�
//	case F_CURLINECENTER			://�J�[�\���s���E�B���h�E������
	case F_JUMPPREV					://�ړ�����: �O��
	case F_JUMPNEXT					://�ړ�����: ����

	/* �I���n */	//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v�������Ȃ����̂œƗ�������(�I��)���ړ�(�T�u���j���[���͍\����ł��Ȃ��̂�)
	case F_SELECTWORD				://���݈ʒu�̒P��I��
	case F_SELECTALL				://���ׂđI��
	case F_BEGIN_SEL				://�͈͑I���J�n
	case F_UP_SEL					://(�͈͑I��)�J�[�\����ړ�
	case F_DOWN_SEL					://(�͈͑I��)�J�[�\�����ړ�
	case F_LEFT_SEL					://(�͈͑I��)�J�[�\�����ړ�
	case F_RIGHT_SEL				://(�͈͑I��)�J�[�\���E�ړ�
	case F_UP2_SEL					://(�͈͑I��)�J�[�\����ړ�(�Q�s����)
	case F_DOWN2_SEL				://(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
	case F_WORDLEFT_SEL				://(�͈͑I��)�P��̍��[�Ɉړ�
	case F_WORDRIGHT_SEL			://(�͈͑I��)�P��̉E�[�Ɉړ�
	case F_GOLINETOP_SEL			://(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOLINEEND_SEL			://(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
//	case F_ROLLDOWN_SEL				://(�͈͑I��)�X�N���[���_�E��
//	case F_ROLLUP_SEL				://(�͈͑I��)�X�N���[���A�b�v
//	case F_HalfPageUp_Sel			://(�͈͑I��)���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//	case F_HalfPageDown_Sel			://(�͈͑I��)���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
//	case F_1PageUp_Sel				://(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
//	case F_1PageDown_Sel			://(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	case F_GOFILETOP_SEL			://(�͈͑I��)�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND_SEL			://(�͈͑I��)�t�@�C���̍Ō�Ɉړ�

	/* ��`�I���n */	//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
//	case F_BOXSELALL				//��`�ł��ׂđI��
	case F_BEGIN_BOX				://��`�͈͑I���J�n
/*
	case F_UP_BOX					://(��`�I��)�J�[�\����ړ�
	case F_DOWN_BOX					://(��`�I��)�J�[�\�����ړ�
	case F_LEFT_BOX					://(��`�I��)�J�[�\�����ړ�
	case F_RIGHT_BOX				://(��`�I��)�J�[�\���E�ړ�
	case F_UP2_BOX					://(��`�I��)�J�[�\����ړ�(�Q�s����)
	case F_DOWN2_BOX				://(��`�I��)�J�[�\�����ړ�(�Q�s����)
	case F_WORDLEFT_BOX				://(��`�I��)�P��̍��[�Ɉړ�
	case F_WORDRIGHT_BOX			://(��`�I��)�P��̉E�[�Ɉړ�
	case F_GOLINETOP_BOX			://(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
	case F_GOLINEEND_BOX			://(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
	case F_HalfPageUp_Box			://(��`�I��)���y�[�W�A�b�v
	case F_HalfPageDown_Box			://(��`�I��)���y�[�W�_�E��
	case F_1PageUp_Box				://(��`�I��)�P�y�[�W�A�b�v
	case F_1PageDown_Box			://(��`�I��)�P�y�[�W�_�E��
	case F_GOFILETOP_BOX			://(��`�I��)�t�@�C���̐擪�Ɉړ�
	case F_GOFILEEND_BOX			://(��`�I��)�t�@�C���̍Ō�Ɉړ�
*/
	/* �N���b�v�{�[�h�n */
	case F_CUT						://�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	case F_COPY						://�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	case F_COPY_CRLF				://CRLF���s�ŃR�s�[(�I��͈͂����s�R�[�h=CRLF�ŃR�s�[)
	case F_PASTE					://�\��t��(�N���b�v�{�[�h����\��t��)
	case F_PASTEBOX					://��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	case F_INSTEXT					://�e�L�X�g��\��t��
//	case F_ADDTAIL					://�Ō�Ƀe�L�X�g��ǉ�
	case F_COPYLINES				://�I��͈͓��S�s�R�s�[
	case F_COPYLINESASPASSAGE		://�I��͈͓��S�s���p���t���R�s�[
	case F_COPYLINESWITHLINENUMBER 	://�I��͈͓��S�s�s�ԍ��t���R�s�[
	case F_COPYPATH					://���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	case F_COPYTAG					://���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[	//Sept. 15, 2000 jepro ��Ɠ��������ɂȂ��Ă����̂��C��
	case F_CREATEKEYBINDLIST		://�L�[���蓖�Ĉꗗ���R�s�[	//Sept. 15, 2000 JEPRO �ǉ�	//Dec. 25, 2000 ����

	/* �}���n */
	case F_INS_DATE					:// ���t�}��
	case F_INS_TIME					:// �����}��

	/* �ϊ��n */
	case F_TOLOWER		 			://�p�啶�����p������
	case F_TOUPPER		 			://�p���������p�啶��
	case F_TOHANKAKU		 		:/* �S�p�����p */
	case F_TOZENEI			 		:/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	case F_TOZENKAKUKATA	 		:/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	case F_TOZENKAKUHIRA	 		:/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	case F_HANKATATOZENKAKUKATA		:/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENKAKUHIRA		:/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_TABTOSPACE				:/* TAB���� */
	case F_SPACETOTAB				:/* �󔒁�TAB */  //#### Stonee, 2001/05/27
	case F_CODECNV_AUTO2SJIS		:/* �������ʁ�SJIS�R�[�h�ϊ� */
	case F_CODECNV_EMAIL			://E-Mail(JIS��SJIS)�R�[�h�ϊ�
	case F_CODECNV_EUC2SJIS			://EUC��SJIS�R�[�h�ϊ�
	case F_CODECNV_UNICODE2SJIS		://Unicode��SJIS�R�[�h�ϊ�
	case F_CODECNV_UTF82SJIS		:/* UTF-8��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF72SJIS		:/* UTF-7��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2JIS			:/* SJIS��JIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2EUC			:/* SJIS��EUC�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF8		:/* SJIS��UTF-8�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF7		:/* SJIS��UTF-7�R�[�h�ϊ� */
//	case F_BASE64DECODE	 			://Base64�f�R�[�h���ĕۑ�
//	case F_UUDECODE		 			://uudecode���ĕۑ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX

	/* �����n */
//	case F_SEARCH_DIALOG			://����(�P�ꌟ���_�C�A���O)
	case F_SEARCH_NEXT				://��������
	case F_SEARCH_PREV				://�O������
//	case F_REPLACE					://�u��(�u���_�C�A���O)
	case F_SEARCH_CLEARMARK			://�����}�[�N�̃N���A
//	case F_GREP						://Grep
//	case F_JUMP						://�w��s�w�W�����v
//	case F_OUTLINE					://�A�E�g���C�����
	case F_TAGJUMP					://�^�O�W�����v�@�\
	case F_TAGJUMPBACK				://�^�O�W�����v�o�b�N�@�\
//	case F_COMPARE					://�t�@�C�����e��r
	case F_BRACKETPAIR				://�Ί��ʂ̌���

	/* ���[�h�؂�ւ��n */
	case F_CHGMOD_INS				://�}���^�㏑�����[�h�؂�ւ�
	case F_CANCEL_MODE				://�e�탂�[�h�̎�����

	/* �ݒ�n */
//	case F_SHOWTOOLBAR				:/* �c�[���o�[�̕\�� */
//	case F_SHOWFUNCKEY				:/* �t�@���N�V�����L�[�̕\�� */
//	case F_SHOWSTATUSBAR			:/* �X�e�[�^�X�o�[�̕\�� */
//	case F_TYPE_LIST				:/* �^�C�v�ʐݒ�ꗗ */
//	case F_OPTION_TYPE				:/* �^�C�v�ʐݒ� */
//	case F_OPTION					:/* ���ʐݒ� */
//	case F_FONT						:/* �t�H���g�ݒ� */
//	case F_WRAPWINDOWWIDTH			:/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 15, 2000 JEPRO

	/* �J�X�^�����j���[ */
//	case F_MENU_RBUTTON				:/* �E�N���b�N���j���[ */
//	case F_CUSTMENU_1				:/* �J�X�^�����j���[1 */
//	case F_CUSTMENU_2				:/* �J�X�^�����j���[2 */
//	case F_CUSTMENU_3				:/* �J�X�^�����j���[3 */
//	case F_CUSTMENU_4				:/* �J�X�^�����j���[4 */
//	case F_CUSTMENU_5				:/* �J�X�^�����j���[5 */
//	case F_CUSTMENU_6				:/* �J�X�^�����j���[6 */
//	case F_CUSTMENU_7				:/* �J�X�^�����j���[7 */
//	case F_CUSTMENU_8				:/* �J�X�^�����j���[8 */
//	case F_CUSTMENU_9				:/* �J�X�^�����j���[9 */
//	case F_CUSTMENU_10				:/* �J�X�^�����j���[10 */
//	case F_CUSTMENU_11				:/* �J�X�^�����j���[11 */
//	case F_CUSTMENU_12				:/* �J�X�^�����j���[12 */
//	case F_CUSTMENU_13				:/* �J�X�^�����j���[13 */
//	case F_CUSTMENU_14				:/* �J�X�^�����j���[14 */
//	case F_CUSTMENU_15				:/* �J�X�^�����j���[15 */
//	case F_CUSTMENU_16				:/* �J�X�^�����j���[16 */
//	case F_CUSTMENU_17				:/* �J�X�^�����j���[17 */
//	case F_CUSTMENU_18				:/* �J�X�^�����j���[18 */
//	case F_CUSTMENU_19				:/* �J�X�^�����j���[19 */
//	case F_CUSTMENU_20				:/* �J�X�^�����j���[20 */
//	case F_CUSTMENU_21				:/* �J�X�^�����j���[21 */
//	case F_CUSTMENU_22				:/* �J�X�^�����j���[22 */
//	case F_CUSTMENU_23				:/* �J�X�^�����j���[23 */
//	case F_CUSTMENU_24				:/* �J�X�^�����j���[24 */

	/* �E�B���h�E�n */
//	case F_SPLIT_V					://�㉺�ɕ���	//Sept. 16, 2000 jepro �������u�c�v����u�㉺�Ɂv�ɕύX
//	case F_SPLIT_H					://���E�ɕ���	//Sept. 16, 2000 jepro �������u���v����u���E�Ɂv�ɕύX
//	case F_SPLIT_VH					://�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
//	case F_WINCLOSE					://�E�B���h�E�����
//	case F_WIN_CLOSEALL				://���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
//	case F_NEXTWINDOW				://���̃E�B���h�E
//	case F_PREVWINDOW				://�O�̃E�B���h�E
//	case F_CASCADE					://�d�˂ĕ\��
//	case F_TILE_V					://�㉺�ɕ��ׂĕ\��
//	case F_TILE_H					://���E�ɕ��ׂĕ\��
//	case F_MAXIMIZE_V				://�c�����ɍő剻
//	case F_MINIMIZE_ALL				://���ׂčŏ���	//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
//	case F_REDRAW					://�ĕ`��
	case F_WIN_OUTPUT				://�A�E�g�v�b�g�E�B���h�E�\��

	/* �x�� */
//  case F_HOKAN					:/* ���͕⊮ */				//Oct. 15, 2000 JEPRO �����ĂȂ������̂œ���Ă݂�
//	case F_HELP_CONTENTS			:/* �w���v�ڎ� */			//Dec. 25, 2000 JEPRO �ǉ�
//	case F_HELP_SEARCH				:/* �w���v�L�[���[�h���� */	//Dec. 25, 2000 JEPRO �ǉ�
//	case F_MENU_ALLFUNC				:/* �R�}���h�ꗗ */
//	case F_EXTHELP1					:/* �O���w���v�P */
//	case F_EXTHTMLHELP				:/* �O��HTML�w���v */
//	case F_ABOUT					:/* �o�[�W������� */		//Dec. 25, 2000 JEPRO �ǉ�

	/* ���̑� */
//	case F_SENDMAIL					:/* ���[�����M */
		return TRUE;
	}
	return FALSE;

}


/*[EOF]*/
