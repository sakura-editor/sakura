//	$Id$
/*!	@file
	@brief �}�N��

	@author Norio Nakatani
	@author genta
	
	@date Sep. 29, 2001
	@date 20011229 aroka �o�O�C���A�R�����g�ǉ�
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CSMacroMgr.h"
#include "CEditView.h"
#include "CPPAMacroMgr.h"
#include "CPPA.h"

CSMacroMgr::MacroFuncInfo CSMacroMgr::m_MacroFuncInfoArr[] = 
{
//	�@�\ID			�֐���			����				��Ɨp�o�b�t�@

	/* �t�@�C������n */
	{F_FILENEW,						"S_FileNew",			"",					NULL}, //�V�K�쐬
	{F_FILEOPEN,					"S_FileOpen",			"(str: string)",	NULL}, //�J��
	{F_FILESAVE,					"S_FileSave",			"",					NULL}, //�㏑���ۑ�
	{F_FILESAVEAS,					"S_FileSaveAs",			"(str: string; charcode, eolcode: Integer)",	NULL}, //���O��t���ĕۑ�
	{F_FILECLOSE,					"S_FileClose",			"",					NULL}, //����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
//	{F_FILECLOSE_OPEN,				"S_FileCloseOpen",		"(str: string)",	NULL}, //���ĊJ��
	{F_FILE_REOPEN_SJIS,			"S_FileReopenSJIS",		"",					NULL}, //SJIS�ŊJ������
	{F_FILE_REOPEN_JIS,				"S_FileReopenJIS",		"",					NULL}, //JIS�ŊJ������
	{F_FILE_REOPEN_EUC,				"S_FileReopenEUC",		"",					NULL}, //EUC�ŊJ������
	{F_FILE_REOPEN_UNICODE,			"S_FileReopenUNICODE",	"",					NULL}, //Unicode�ŊJ������
	{F_FILE_REOPEN_UTF8,			"S_FileReopenUTF8",		"",					NULL}, //UTF-8�ŊJ������
	{F_FILE_REOPEN_UTF7,			"S_FileReopenUTF7",		"",					NULL}, //UTF-7�ŊJ������
	{F_PRINT,						"S_Print",				"",					NULL}, //���
//	{F_PRINT_DIALOG,				"S_PrintDialog",		"",					NULL}, //����_�C�A���O
	{F_PRINT_PREVIEW,				"S_PrintPreview",		"",					NULL}, //����v���r���[
	{F_PRINT_PAGESETUP,				"S_PrintPageSetup",		"",					NULL}, //����y�[�W�ݒ�	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	{F_OPEN_HfromtoC,				"S_OpenHfromtoC",		"",					NULL}, //������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
	{F_OPEN_HHPP,					"S_OpenHHpp",			"",					NULL}, //������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	{F_OPEN_CCPP,					"S_OpenCCpp",			"",					NULL}, //������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	{F_ACTIVATE_SQLPLUS,			"S_ActivateSQLPLUS",	"",					NULL}, /* Oracle SQL*Plus���A�N�e�B�u�\�� */
	{F_PLSQL_COMPILE_ON_SQLPLUS,	"S_ExecSQLPLUS",		"",					NULL}, /* Oracle SQL*Plus�Ŏ��s */
	{F_BROWSE,						"S_Browse",				"",					NULL}, //�u���E�Y
	{F_READONLY,					"S_ReadOnly",			"",					NULL}, //�ǂݎ���p
	{F_PROPERTY_FILE,				"S_PropertyFile",		"",					NULL}, //�t�@�C���̃v���p�e�B
	{F_EXITALL,						"S_ExitAll",			"",					NULL}, //�T�N���G�f�B�^�̑S�I��	//Dec. 27, 2000 JEPRO �ǉ�

	/* �ҏW�n */
	{F_CHAR,				"S_Char",				"(Val: integer)",	NULL}, //��������
//	{F_IME_CHAR,			"S_CharIme",			"",					NULL}, //�S�p��������
	{F_UNDO,				"S_Undo",				"",					NULL}, //���ɖ߂�(Undo)
	{F_REDO,				"S_Redo",				"",					NULL}, //��蒼��(Redo)
	{F_DELETE,				"S_Delete",				"",					NULL}, //�폜
	{F_DELETE_BACK,			"S_DeleteBack",			"",					NULL}, //�J�[�\���O���폜
	{F_WordDeleteToStart,	"S_WordDeleteToStart",	"",					NULL}, //�P��̍��[�܂ō폜
	{F_WordDeleteToEnd,		"S_WordDeleteToEnd",	"",					NULL}, //�P��̉E�[�܂ō폜
	{F_WordCut,				"S_WordCut",			"",					NULL}, //�P��؂���
	{F_WordDelete,			"S_WordDelete",			"",					NULL}, //�P��폜
	{F_LineCutToStart,		"S_LineCutToStart",		"",					NULL}, //�s���܂Ő؂���(���s�P��)
	{F_LineCutToEnd,		"S_LineCutToEnd",		"",					NULL}, //�s���܂Ő؂���(���s�P��)
	{F_LineDeleteToStart,	"S_LineDeleteToStart",	"",					NULL}, //�s���܂ō폜(���s�P��)
	{F_LineDeleteToEnd,		"S_LineDeleteToEnd",	"",					NULL}, //�s���܂ō폜(���s�P��)
	{F_CUT_LINE,			"S_CutLine",			"",					NULL}, //�s�؂���(�܂�Ԃ��P��)
	{F_DELETE_LINE,			"S_DeleteLine",			"",					NULL}, //�s�폜(�܂�Ԃ��P��)
	{F_DUPLICATELINE,		"S_DuplicateLine",		"",					NULL}, //�s�̓�d��(�܂�Ԃ��P��)
	{F_INDENT_TAB,			"S_IndentTab",			"",					NULL}, //TAB�C���f���g
	{F_UNINDENT_TAB,		"S_UnindentTab",		"",					NULL}, //�tTAB�C���f���g
	{F_INDENT_SPACE,		"S_IndentSpace",		"",					NULL}, //SPACE�C���f���g
	{F_UNINDENT_SPACE,		"S_UnindentSpace",		"",					NULL}, //�tSPACE�C���f���g
	{F_WORDSREFERENCE,		"S_WordReference",		"",					NULL}, //�P�ꃊ�t�@�����X
	{F_LTRIM,				"S_LTrim",				"",					NULL}, //��(�擪)�̋󔒂��폜 2001.12.03 hor
	{F_RTRIM,				"S_RTrim",				"",					NULL}, //�E(����)�̋󔒂��폜 2001.12.03 hor
	{F_SORT_ASC,			"S_SortAsc",			"",					NULL}, //�I���s�̏����\�[�g 2001.12.06 hor
	{F_SORT_DESC,			"S_SortDesc",			"",					NULL}, //�I���s�̍~���\�[�g 2001.12.06 hor
	{F_MERGE,				"S_Merge",				"",					NULL}, //�I���s�̃}�[�W 2001.12.06 hor

	/* �J�[�\���ړ��n */
	{F_UP,					"S_Up",					"",					NULL}, //�J�[�\����ړ�
	{F_DOWN,				"S_Down",				"",					NULL}, //�J�[�\�����ړ�
	{F_LEFT,				"S_Left",				"",					NULL}, //�J�[�\�����ړ�
	{F_RIGHT,				"S_Right",				"",					NULL}, //�J�[�\���E�ړ�
	{F_UP2,					"S_Up2",				"",					NULL}, //�J�[�\����ړ�(�Q�s����)
	{F_DOWN2,				"S_Down2",				"",					NULL}, //�J�[�\�����ړ�(�Q�s����)
	{F_WORDLEFT,			"S_WordLeft",			"",					NULL}, //�P��̍��[�Ɉړ�
	{F_WORDRIGHT,			"S_WordRight",			"",					NULL}, //�P��̉E�[�Ɉړ�
	{F_GOLINETOP,			"S_GoLineTop",			"(flg: integer)",	NULL}, //�s���Ɉړ�(�܂�Ԃ��P��)
	{F_GOLINEEND,			"S_GoLineEnd",			"",					NULL}, //�s���Ɉړ�(�܂�Ԃ��P��)
	{F_HalfPageUp,			"S_HalfPageUp",			"",					NULL}, //���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_HalfPageDown,		"S_HalfPageDown",		"",					NULL}, //���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_1PageUp,				"S_1PageUp",			"",					NULL}, //�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	{F_1PageDown,			"S_1PageDown",			"",					NULL}, //�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	{F_GOFILETOP,			"S_GoFileTop",			"",					NULL}, //�t�@�C���̐擪�Ɉړ�
	{F_GOFILEEND,			"S_GoFileEnd",			"",					NULL}, //�t�@�C���̍Ō�Ɉړ�
	{F_CURLINECENTER,		"S_CurLineCenter",		"",					NULL}, //�J�[�\���s���E�B���h�E������
	{F_JUMPPREV,			"S_MoveHistPrev",		"",					NULL}, //�ړ�����: �O��
	{F_JUMPNEXT,			"S_MoveHistNext",		"",					NULL}, //�ړ�����: ����
	{F_WndScrollDown,		"S_F_WndScrollDown",	"",					NULL}, //�e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	{F_WndScrollUp,			"S_F_WndScrollUp",		"",					NULL}, //�e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o

	/* �I���n */	//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v�������Ȃ����̂Łu�I���n�v�Ƃ��ēƗ���(�T�u���j���[���͍\����ł��Ȃ��̂�)
	{F_SELECTWORD,			"S_SelectWord",			"",					NULL}, //���݈ʒu�̒P��I��
	{F_SELECTALL,			"S_SelectAll",			"",					NULL}, //���ׂđI��
	{F_BEGIN_SEL,			"S_BeginSelect",		"",					NULL}, //�͈͑I���J�n Mar. 5, 2001 genta ���̏C��
	{F_UP_SEL,				"S_Up_Sel",				"",					NULL}, //(�͈͑I��)�J�[�\����ړ�
	{F_DOWN_SEL,			"S_Down_Sel",			"",					NULL}, //(�͈͑I��)�J�[�\�����ړ�
	{F_LEFT_SEL,			"S_Left_Sel",			"",					NULL}, //(�͈͑I��)�J�[�\�����ړ�
	{F_RIGHT_SEL,			"S_Right_Sel",			"",					NULL}, //(�͈͑I��)�J�[�\���E�ړ�
	{F_UP2_SEL,				"S_Up2_Sel",			"",					NULL}, //(�͈͑I��)�J�[�\����ړ�(�Q�s����)
	{F_DOWN2_SEL,			"S_Down2_Sel",			"",					NULL}, //(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
	{F_WORDLEFT_SEL,		"S_WordLeft_Sel",		"",					NULL}, //(�͈͑I��)�P��̍��[�Ɉړ�
	{F_WORDRIGHT_SEL,		"S_WordRight_Sel",		"",					NULL}, //(�͈͑I��)�P��̉E�[�Ɉړ�
	{F_GOLINETOP_SEL,		"S_GoLineTop_Sel",		"",					NULL}, //(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	{F_GOLINEEND_SEL,		"S_GoLineEnd_Sel",		"",					NULL}, //(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	{F_HalfPageUp_Sel,		"S_HalfPageUp_Sel",		"",					NULL}, //(�͈͑I��)���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_HalfPageDown_Sel,	"S_HalfPageDown_Sel",	"",					NULL}, //(�͈͑I��)���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_1PageUp_Sel,			"S_1PageUp_Sel",		"",					NULL}, //(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	{F_1PageDown_Sel,		"S_1PageDown_Sel",		"",					NULL}, //(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	{F_GOFILETOP_SEL,		"S_GoFileTop_Sel",		"",					NULL}, //(�͈͑I��)�t�@�C���̐擪�Ɉړ�
	{F_GOFILEEND_SEL,		"S_GoFileEnd_Sel",		"",					NULL}, //(�͈͑I��)�t�@�C���̍Ō�Ɉړ�

	/* ��`�I���n */	//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
	{F_BEGIN_BOX,			"S_BeginBoxSelect",		"",					NULL}, //��`�͈͑I���J�n

	/* �N���b�v�{�[�h�n */
	{F_CUT,						"S_Cut",						"",					NULL}, //�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	{F_COPY,					"S_Copy",						"",					NULL}, //�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	{F_PASTE,					"S_Paste",						"",					NULL}, //�\��t��(�N���b�v�{�[�h����\��t��)
	{F_COPY_ADDCRLF,			"S_CopyAddCRLF",				"",					NULL}, //�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[
	{F_COPY_CRLF,				"S_CopyCRLF",					"",					NULL}, //CRLF���s�ŃR�s�[(�I��͈͂����s�R�[�h=CRLF�ŃR�s�[)
	{F_PASTEBOX,				"S_PasteBox",					"",					NULL}, //��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	{F_INSTEXT,					"S_InsText",					"(str: string)",	NULL}, // �e�L�X�g��\��t��
	{F_ADDTAIL,					"S_AddTail",					"(str: string)",	NULL}, // �Ō�Ƀe�L�X�g��ǉ�
	{F_COPYLINES,				"S_CopyLines",					"",					NULL}, //�I��͈͓��S�s�R�s�[
	{F_COPYLINESASPASSAGE,		"S_CopyLinesAsPassage",			"",					NULL}, //�I��͈͓��S�s���p���t���R�s�[
	{F_COPYLINESWITHLINENUMBER,	"S_CopyLinesWithLineNumber",	"",					NULL}, //�I��͈͓��S�s�s�ԍ��t���R�s�[
	{F_COPYPATH,				"S_CopyPath",					"",					NULL}, //���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	{F_COPYFNAME,				"S_CopyFilename",				"",					NULL}, //���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ // 2002/2/3 aroka
	{F_COPYTAG,					"S_CopyTag",					"",					NULL}, //���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[	//Sept. 15, 2000 jepro ��Ɠ��������ɂȂ��Ă����̂��C��
	{F_CREATEKEYBINDLIST,		"S_CopyKeyBindList",			"",					NULL}, //�L�[���蓖�Ĉꗗ���R�s�[	//Sept. 15, 2000 JEPRO �ǉ� //Dec. 25, 2000 ����

	/* �}���n */
	{F_INS_DATE,				"S_InsertDate",				"",			NULL}, // ���t�}��
	{F_INS_TIME,				"S_InsertTime",				"",			NULL}, // �����}��

	/* �ϊ��n */
	{F_TOLOWER,		 			"S_ToLower",				"",			NULL}, //�p�啶�����p������
	{F_TOUPPER,		 			"S_ToUpper",				"",			NULL}, //�p���������p�啶��
	{F_TOHANKAKU,		 		"S_ToHankaku",				"",			NULL}, /* �S�p�����p */
	{F_TOZENEI,		 			"S_ToZenEi",				"",			NULL}, /* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	{F_TOHANEI,		 			"S_ToHanEi",				"",			NULL}, /* �S�p�p�������p�p�� */
	{F_TOZENKAKUKATA,	 		"S_ToZenKata",				"",			NULL}, /* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	{F_TOZENKAKUHIRA,	 		"S_ToZenHira",				"",			NULL}, /* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	{F_HANKATATOZENKAKUKATA,	"S_HanKataToZenKata",		"",			NULL}, /* ���p�J�^�J�i���S�p�J�^�J�i */
	{F_HANKATATOZENKAKUHIRA,	"S_HanKataToZenHira",		"",			NULL}, /* ���p�J�^�J�i���S�p�Ђ炪�� */
	{F_TABTOSPACE,				"S_TABToSPACE",				"",			NULL}, /* TAB���� */
	{F_SPACETOTAB,				"S_SPACEToTAB",				"",			NULL}, /* �󔒁�TAB */ //#### Stonee, 2001/05/27
	{F_CODECNV_AUTO2SJIS,		"S_AutoToSJIS",				"",			NULL}, /* �������ʁ�SJIS�R�[�h�ϊ� */
	{F_CODECNV_EMAIL,			"S_JIStoSJIS",				"",			NULL}, //E-Mail(JIS��SJIS)�R�[�h�ϊ�
	{F_CODECNV_EUC2SJIS,		"S_EUCtoSJIS",				"",			NULL}, //EUC��SJIS�R�[�h�ϊ�
	{F_CODECNV_UNICODE2SJIS,	"S_CodeCnvUNICODEtoJIS",	"",			NULL}, //Unicode��SJIS�R�[�h�ϊ�
	{F_CODECNV_UTF82SJIS,		"S_UTF8toSJIS",				"",			NULL}, /* UTF-8��SJIS�R�[�h�ϊ� */
	{F_CODECNV_UTF72SJIS,		"S_UTF7toSJIS",				"",			NULL}, /* UTF-7��SJIS�R�[�h�ϊ� */
	{F_CODECNV_SJIS2JIS,		"S_SJIStoJIS",				"",			NULL}, /* SJIS��JIS�R�[�h�ϊ� */
	{F_CODECNV_SJIS2EUC,		"S_SJIStoEUC",				"",			NULL}, /* SJIS��EUC�R�[�h�ϊ� */
	{F_CODECNV_SJIS2UTF8,		"S_SJIStoUTF8",				"",			NULL}, /* SJIS��UTF-8�R�[�h�ϊ� */
	{F_CODECNV_SJIS2UTF7,		"S_SJIStoUTF7",				"",			NULL}, /* SJIS��UTF-7�R�[�h�ϊ� */
	{F_BASE64DECODE,	 		"S_Base64Decode",			"",			NULL}, //Base64�f�R�[�h���ĕۑ�
	{F_UUDECODE,		 		"S_Uudecode",				"",			NULL}, //uudecode���ĕۑ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX


	/* �����n */
	{F_SEARCH_DIALOG,			"S_SearchDialog",			"",			NULL}, //����(�P�ꌟ���_�C�A���O)
	{F_SEARCH_NEXT,				"S_SearchNext",				"(str: String; flg: Integer)",			NULL}, //��������
	{F_SEARCH_PREV,				"S_SearchPrev",				"(str: String; flg: Integer)",			NULL}, //�O������
	{F_REPLACE_DIALOG,			"S_ReplaceDialog",			"",			NULL}, //�u��(�u���_�C�A���O)
	{F_REPLACE,					"S_Replace",				"(before, after: String; flg: Integer)",	NULL}, //�u��(���s)
	{F_REPLACE_ALL,				"S_ReplaceAll",				"(before, after: String; flg: Integer)",	NULL}, //���ׂĒu��(���s)
	{F_SEARCH_CLEARMARK,		"S_SearchClearMark",		"",			NULL}, //�����}�[�N�̃N���A
	{F_GREP,					"S_Grep",					"(str, file, folder: String; flg: Integer)",			NULL}, //Grep
	{F_JUMP,					"S_Jump",					"(line, flg: Integer)",			NULL}, //�w��s�w�W�����v
	{F_OUTLINE,					"S_Outline",				"",			NULL}, //�A�E�g���C�����
	{F_TAGJUMP,					"S_TagJump",				"",			NULL}, //�^�O�W�����v�@�\
	{F_TAGJUMPBACK,				"S_TagJumpBack",			"",			NULL}, //�^�O�W�����v�o�b�N�@�\
	{F_COMPARE,					"S_Compare",				"",			NULL}, //�t�@�C�����e��r
	{F_BRACKETPAIR,				"S_BracketPair",			"",			NULL}, //�Ί��ʂ̌���
// From Here 2001.12.03 hor
	{F_BOOKMARK_SET,			"S_BookmarkSet",			"",			NULL}, //�u�b�N�}�[�N�ݒ�E����
	{F_BOOKMARK_NEXT,			"S_BookmarkNext",			"",			NULL}, //���̃u�b�N�}�[�N��
	{F_BOOKMARK_PREV,			"S_BookmarkPrev",			"",			NULL}, //�O�̃u�b�N�}�[�N��
	{F_BOOKMARK_RESET,			"S_BookmarkReset",			"",			NULL}, //�u�b�N�}�[�N�̑S����
	{F_BOOKMARK_VIEW,			"S_BookmarkView",			"",			NULL}, //�u�b�N�}�[�N�̈ꗗ
// To Here 2001.12.03 hor
	{F_BOOKMARK_PATTERN,		"S_BookmarkPattern",		"(str: String; flg: Integer)",	NULL}, // 2002.01.16 hor �w��p�^�[���Ɉ�v����s���}�[�N

	/* ���[�h�؂�ւ��n */
	{F_CHGMOD_INS,				"S_ChgmodINS",				"",			NULL}, //�}���^�㏑�����[�h�؂�ւ�
	{F_CANCEL_MODE,				"S_CancelMode",				"",			NULL}, //�e�탂�[�h�̎�����

	/* �ݒ�n */
	{F_SHOWTOOLBAR,				"S_ShowToolbar",			"",			NULL}, /* �c�[���o�[�̕\�� */
	{F_SHOWFUNCKEY,				"S_ShowFunckey",			"",			NULL}, /* �t�@���N�V�����L�[�̕\�� */
	{F_SHOWSTATUSBAR,			"S_ShowStatusbar",			"",			NULL}, /* �X�e�[�^�X�o�[�̕\�� */
	{F_TYPE_LIST,				"S_TypeList",				"",			NULL}, /* �^�C�v�ʐݒ�ꗗ */
	{F_OPTION_TYPE,				"S_OptionType",				"",			NULL}, /* �^�C�v�ʐݒ� */
	{F_OPTION,					"S_OptionCommon",			"",			NULL}, /* ���ʐݒ� */
	{F_FONT,					"S_SelectFont",				"",			NULL}, /* �t�H���g�ݒ� */
	{F_WRAPWINDOWWIDTH,			"S_WrapWindowWidth",		"",			NULL}, /* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX

	//	Oct. 9, 2001 genta �ǉ�
	{F_EXECCOMMAND,				"S_ExecCommand",			"(str: String; flg: Integer)",	NULL}, /* �O���R�}���h���s */

	/* �J�X�^�����j���[ */
	{F_MENU_RBUTTON,			"S_RMenu",					"",			NULL}, /* �E�N���b�N���j���[ */
	{F_CUSTMENU_1,				"S_CustMenu1",				"",			NULL}, /* �J�X�^�����j���[1 */
	{F_CUSTMENU_2,				"S_CustMenu2",				"",			NULL}, /* �J�X�^�����j���[2 */
	{F_CUSTMENU_3,				"S_CustMenu3",				"",			NULL}, /* �J�X�^�����j���[3 */
	{F_CUSTMENU_4,				"S_CustMenu4",				"",			NULL}, /* �J�X�^�����j���[4 */
	{F_CUSTMENU_5,				"S_CustMenu5",				"",			NULL}, /* �J�X�^�����j���[5 */
	{F_CUSTMENU_6,				"S_CustMenu6",				"",			NULL}, /* �J�X�^�����j���[6 */
	{F_CUSTMENU_7,				"S_CustMenu7",				"",			NULL}, /* �J�X�^�����j���[7 */
	{F_CUSTMENU_8,				"S_CustMenu8",				"",			NULL}, /* �J�X�^�����j���[8 */
	{F_CUSTMENU_9,				"S_CustMenu9",				"",			NULL}, /* �J�X�^�����j���[9 */
	{F_CUSTMENU_10,				"S_CustMenu10",				"",			NULL}, /* �J�X�^�����j���[10 */
	{F_CUSTMENU_11,				"S_CustMenu11",				"",			NULL}, /* �J�X�^�����j���[11 */
	{F_CUSTMENU_12,				"S_CustMenu12",				"",			NULL}, /* �J�X�^�����j���[12 */
	{F_CUSTMENU_13,				"S_CustMenu13",				"",			NULL}, /* �J�X�^�����j���[13 */
	{F_CUSTMENU_14,				"S_CustMenu14",				"",			NULL}, /* �J�X�^�����j���[14 */
	{F_CUSTMENU_15,				"S_CustMenu15",				"",			NULL}, /* �J�X�^�����j���[15 */
	{F_CUSTMENU_16,				"S_CustMenu16",				"",			NULL}, /* �J�X�^�����j���[16 */
	{F_CUSTMENU_17,				"S_CustMenu17", 			"",			NULL}, /* �J�X�^�����j���[17 */
	{F_CUSTMENU_18,				"S_CustMenu18",				"",			NULL}, /* �J�X�^�����j���[18 */
	{F_CUSTMENU_19,				"S_CustMenu19",				"",			NULL}, /* �J�X�^�����j���[19 */
	{F_CUSTMENU_20,				"S_CustMenu20",				"",			NULL}, /* �J�X�^�����j���[20 */
	{F_CUSTMENU_21,				"S_CustMenu21",				"",			NULL}, /* �J�X�^�����j���[21 */
	{F_CUSTMENU_22,				"S_CustMenu22",				"",			NULL}, /* �J�X�^�����j���[22 */
	{F_CUSTMENU_23,				"S_CustMenu23",				"",			NULL}, /* �J�X�^�����j���[23 */
	{F_CUSTMENU_24,				"S_CustMenu24",				"",			NULL}, /* �J�X�^�����j���[24 */

	/* �E�B���h�E�n */
	{F_SPLIT_V,					"S_SplitWinV",				"",			NULL}, //�㉺�ɕ���	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	{F_SPLIT_H,					"S_SplitWinH",				"",			NULL}, //���E�ɕ���	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	{F_SPLIT_VH,				"S_SplitWinVH",				"",			NULL}, //�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	{F_WINCLOSE,				"S_WinClose",				"",			NULL}, //�E�B���h�E�����
	{F_WIN_CLOSEALL,			"S_WinCloseAll",			"",			NULL}, //���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
	{F_CASCADE,					"S_CascadeWin",				"",			NULL}, //�d�˂ĕ\��
	{F_TILE_V,					"S_TileWinV",				"",			NULL}, //�㉺�ɕ��ׂĕ\��
	{F_TILE_H,					"S_TileWinH",				"",			NULL}, //���E�ɕ��ׂĕ\��
	{F_NEXTWINDOW,				"S_NextWindow",				"",			NULL}, //���̃E�B���h�E
	{F_PREVWINDOW,				"S_PrevWindow",				"",			NULL}, //�O�̃E�B���h�E
	{F_MAXIMIZE_V,				"S_MaximizeV",				"",			NULL}, //�c�����ɍő剻
	{F_MAXIMIZE_H,				"S_MaximizeH",				"",			NULL}, //�������ɍő剻 //2001.02.10 by MIK
	{F_MINIMIZE_ALL,			"S_MinimizeAll",			"",			NULL}, //���ׂčŏ���	//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	{F_REDRAW,					"S_ReDraw",					"",			NULL}, //�ĕ`��
	{F_WIN_OUTPUT,				"S_ActivateWinOutput",		"",			NULL}, //�A�E�g�v�b�g�E�B���h�E�\��

	/* �x�� */
	{F_HOKAN,					"S_Complete",				"",			NULL}, /* ���͕⊮ */	//Oct. 15, 2000 JEPRO �����ĂȂ������̂ŉp����t���ē���Ă݂�
	{F_HELP_CONTENTS,			"S_HelpContents",			"",			NULL}, /* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO �ǉ�
	{F_HELP_SEARCH,				"S_HelpSearch",				"",			NULL}, /* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO �ǉ�
	{F_MENU_ALLFUNC,			"S_CommandList",			"",			NULL}, /* �R�}���h�ꗗ */
	{F_EXTHELP1,				"S_ExtHelp1",				"",			NULL}, /* �O���w���v�P */
	{F_EXTHTMLHELP,				"S_ExtHtmlHelp",			"",			NULL}, /* �O��HTML�w���v */
	{F_ABOUT,					"S_About",					"",			NULL}, /* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	//	�I�[
	{0,	NULL, NULL, NULL}
};
//int	CSMacroMgr::m_nMacroFuncInfoArrNum = sizeof( CSMacroMgr::m_MacroFuncInfoArr ) / sizeof( CSMacroMgr::m_MacroFuncInfoArr[0] );

CSMacroMgr::CSMacroMgr()
{
//	m_cShareData.Init();
	m_pShareData = CShareData::getInstance()->GetShareData();
	
	if ( CPPAMacroMgr::m_cPPA.Init() ){
		//	PPA.DLL�A��
		int i;
		for (i=0; i<MAX_CUSTMACRO; i++){
			m_cSavedKeyMacro[i] = new CPPAMacroMgr;
		}
		m_cKeyMacro = new CPPAMacroMgr;
	}
	else {
		//	PPA.DLL�i�V
		int i;
		for (i=0; i<MAX_CUSTMACRO; i++){
			m_cSavedKeyMacro[i] = new CKeyMacroMgr;
		}
		m_cKeyMacro = new CKeyMacroMgr;
	}
}

CSMacroMgr::~CSMacroMgr()
{
	//- 20011229 add by aroka
	ClearAll();
	
	//	PPA.DLL�A���i�V����
	int i;
	for (i=0; i<MAX_CUSTMACRO; i++){
		delete m_cSavedKeyMacro[i];
	}
	delete m_cKeyMacro;
}

/* �L�[�}�N���̃o�b�t�@���N���A���� */
void CSMacroMgr::ClearAll( void )
{
	int i;
	for (i = 0; i < MAX_CUSTMACRO; i++){
		m_cSavedKeyMacro[i]->ClearAll();
	}
	m_cKeyMacro->ClearAll();
}

/*! @brief�L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�

	@param nFuncID [in] �@�\�ԍ�
	@param lParam1 [in] �p�����[�^�B
	@param mbuf [in] �ǂݍ��ݐ�}�N���o�b�t�@

*/
int CSMacroMgr::Append( int idx, /*CSMacroMgr::Macro1& mbuf, */ int nFuncID, LPARAM lParam1, CEditView* pcEditView )
{
	if (idx == STAND_KEYMACRO){
		m_cKeyMacro->Append( nFuncID, lParam1, pcEditView );
	}
	else {
		m_cSavedKeyMacro[idx]->Append( nFuncID, lParam1, pcEditView );
	}
	return TRUE;
}


/*!	@brief �L�[�{�[�h�}�N���̎��s

	CShareData����t�@�C�������擾���A���s����B

	@param hInstance [in] �C���X�^���X
	@param hwndParent [in] �e�E�B���h�E��
	@param pViewClass [in] macro���s�Ώۂ�View
	@param idx [in] �}�N���ԍ��B
*/
BOOL CSMacroMgr::Exec( int idx , HINSTANCE hInstance, CEditView* pcEditView )
{
	if( idx == STAND_KEYMACRO ){
		m_cKeyMacro->ExecKeyMacro( pcEditView );
		return TRUE;	//	�K��TRUE�H
	}
	if( idx < 0 || MAX_CUSTMACRO <= idx )	//	�͈̓`�F�b�N
		return FALSE;

	/* �ǂݍ��ݑO���A����ǂݍ��ސݒ�̏ꍇ�́A�t�@�C����ǂݍ��݂Ȃ��� */
	if( !m_cSavedKeyMacro[idx]->IsReady() || CShareData::getInstance()->BeReloadWhenExecuteMacro( idx )){
		//	CShareData����A�}�N���t�@�C�������擾
		char* p = CShareData::getInstance()->GetMacroFilename( idx );
		if ( p == NULL){
			return FALSE;
		}
		char ptr[_MAX_PATH * 2];
		strcpy(ptr, p);

		Clear( idx );	//	��x�N���A���Ă���ǂݍ��ށi�O�̂��߁j
		if( !Load( idx, hInstance, ptr ) )
			return FALSE;
	}

	m_cSavedKeyMacro[idx]->ExecKeyMacro(pcEditView);
	pcEditView->Redraw();	//	�K�v�H
	return TRUE;
}

/*! �L�[�{�[�h�}�N���̓ǂݍ���

	@param idx [in] �ǂݍ��ݐ�}�N���o�b�t�@�ԍ�
	@param pszPath [in] �}�N���t�@�C����

	@author Norio Nakatani
*/
BOOL CSMacroMgr::Load( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	if ( idx == STAND_KEYMACRO ){
		m_cKeyMacro->ClearAll();
		return m_cKeyMacro->LoadKeyMacro(hInstance, pszPath );
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		m_cSavedKeyMacro[idx]->ClearAll();
		return m_cSavedKeyMacro[idx]->LoadKeyMacro(hInstance, pszPath );
	}
	return FALSE;
}

/*! �L�[�{�[�h�}�N���̕ۑ�

	@param idx [in] �ǂݍ��ݐ�}�N���o�b�t�@�ԍ�
	@param pszPath [in] �}�N���t�@�C����

	@author YAZAKI
*/
BOOL CSMacroMgr::Save( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	if ( idx == STAND_KEYMACRO ){
		return m_cKeyMacro->SaveKeyMacro(hInstance, pszPath );
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		return m_cSavedKeyMacro[idx]->SaveKeyMacro(hInstance, pszPath );
	}
	return FALSE;
}

void CSMacroMgr::Clear( int idx )
{
	if ( idx == STAND_KEYMACRO ){
		m_cKeyMacro->ClearAll();
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		m_cSavedKeyMacro[idx]->ClearAll();
	}
}

/*
||  Attributes & Operations
*/
/* �@�\ID���֐����C�@�\�����{�� */
char* CSMacroMgr::GetFuncInfoByID( HINSTANCE hInstance, int nFuncID, char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoArr[i].m_nFuncID == nFuncID ){
			strcpy( pszFuncName, m_MacroFuncInfoArr[i].m_pszFuncName );
			char *p = pszFuncName;
			while (*p){
				if (*p == '('){
					*p = '\0';
					break;
				}
				*p++;
			}
			::LoadString( hInstance, nFuncID, pszFuncNameJapanese, 255 );
			return pszFuncName;
		}
	}
	return NULL;
}

/* �֐����iS_xxxx�j���@�\ID�C�@�\�����{�� */
int CSMacroMgr::GetFuncInfoByName( HINSTANCE hInstance, const char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	int		nFuncID;
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == strcmp( pszFuncName, m_MacroFuncInfoArr[i].m_pszFuncName ) ){
			nFuncID = m_MacroFuncInfoArr[i].m_nFuncID;
			::LoadString( hInstance, nFuncID, pszFuncNameJapanese, 255 );
			return nFuncID;
		}
	}
	return -1;
}

/* �L�[�}�N���ɋL�^�\�ȋ@�\���ǂ����𒲂ׂ� */
BOOL CSMacroMgr::CanFuncIsKeyMacro( int nFuncID )
{
	switch( nFuncID ){
	/* �t�@�C������n */
//	case F_FILENEW					://�V�K�쐬
//	case F_FILEOPEN					://�J��
//	case F_FILESAVE					://�㏑���ۑ�
//	case F_FILESAVEAS_DIALOG		://���O��t���ĕۑ�
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
	case F_LTRIM					:// 2001.12.03 hor
	case F_RTRIM					:// 2001.12.03 hor
	case F_SORT_ASC					:// 2001.12.06 hor
	case F_SORT_DESC				:// 2001.12.06 hor
	case F_MERGE					:// 2001.12.06 hor

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
	case F_COPY_ADDCRLF				://�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[
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
	case F_COPYFNAME				://���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ // 2002/2/3 aroka
	case F_CREATEKEYBINDLIST		://�L�[���蓖�Ĉꗗ���R�s�[	//Sept. 15, 2000 JEPRO �ǉ�	//Dec. 25, 2000 ����

	/* �}���n */
	case F_INS_DATE					:// ���t�}��
	case F_INS_TIME					:// �����}��

	/* �ϊ��n */
	case F_TOLOWER		 			://�p�啶�����p������
	case F_TOUPPER		 			://�p���������p�啶��
	case F_TOHANKAKU		 		:/* �S�p�����p */
	case F_TOZENEI			 		:/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	case F_TOHANEI			 		:/* �S�p�p�������p�p�� */
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
	case F_REPLACE					://�u��(���s)
	case F_REPLACE_ALL				://���ׂĒu��(���s)
	case F_SEARCH_CLEARMARK			://�����}�[�N�̃N���A
	case F_GREP						://Grep
//	case F_JUMP_DIALOG				://�w��s�w�W�����v
	case F_JUMP						://�w��s�փW�����v @@@ 2002.2.2 YAZAKI
//	case F_OUTLINE					://�A�E�g���C�����
	case F_TAGJUMP					://�^�O�W�����v�@�\
	case F_TAGJUMPBACK				://�^�O�W�����v�o�b�N�@�\
//	case F_COMPARE					://�t�@�C�����e��r
	case F_BRACKETPAIR				://�Ί��ʂ̌���
// From Here 2001.12.03 hor
	case F_BOOKMARK_SET				://�u�b�N�}�[�N�ݒ�E����
	case F_BOOKMARK_NEXT			://���̃u�b�N�}�[�N��
	case F_BOOKMARK_PREV			://�O�̃u�b�N�}�[�N��
	case F_BOOKMARK_RESET			://�u�b�N�}�[�N�̑S����
//	case F_BOOKMARK_VIEW			://�u�b�N�}�[�N�̈ꗗ
// To Here 2001.12.03 hor
	case F_BOOKMARK_PATTERN			://���������ĊY���s���}�[�N	// 2002.02.08 hor

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

	case F_EXECCOMMAND				:/* �O���R�}���h���s */	//@@@2002.2.2 YAZAKI �ǉ�

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
	case F_REDRAW					://�ĕ`��
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
