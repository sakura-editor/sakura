/*!	@file
	@brief �}�N��

	@author Norio Nakatani
	@author genta
	@date Sep. 29, 2001 �쐬
	@date 20011229 aroka �o�O�C���A�R�����g�ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, genta, aroka, MIK, asa-o, hor, Misaka, jepro, Stonee
	Copyright (C) 2002, YAZAKI, MIK, aroka, hor, genta, ai
	Copyright (C) 2003, MIK, genta, Moca
	Copyright (C) 2004, genta, zenryaku
	Copyright (C) 2005, MIK, genta, maru, FILE
	Copyright (C) 2006, �����, fon
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CSMacroMgr.h"
#include "CEditView.h"
#include "CPPAMacroMgr.h"
#include "CWSHManager.h"
#include "CMacroFactory.h"
#include <stdio.h>
#include <assert.h> // �����p
#include "debug.h"
#include "CRunningTimer.h"

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoNotCommandArr[] = 
{
	{F_GETFILENAME,	"GetFilename",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL}, //�t�@�C������Ԃ�
	//	Oct. 19, 2002 genta
	{F_GETSELECTED,	"GetSelectedString",	{VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL}, //�I�𕔕�
	{F_EXPANDPARAMETER,	"ExpandParameter",	{VT_BSTR, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL}, //���ꕶ���̓W�J
	{F_GETLINESTR, "GetLineStr", {VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_BSTR, NULL }, // �w��_���s�̎擾 2003.06.01 Moca
	{F_GETLINECOUNT, "GetLineCount", {VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // �S�_���s���̎擾 2003.06.01 Moca
	{F_CHGTABWIDTH,			"ChangeTabWidth",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},  VT_I4, NULL }, //�^�u�T�C�Y�ύX 2004.03.16 zenryaku
	{F_ISTEXTSELECTED,		"IsTextSelected",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, //�e�L�X�g���I������Ă��邩 2005.7.30 maru
	{F_GETSELLINEFROM,		"GetSelectLineFrom",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // �I���J�n�s�̎擾 2005.7.30 maru
	{F_GETSELCOLMFROM,		"GetSelectColmFrom",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // �I���J�n���̎擾 2005.7.30 maru
	{F_GETSELLINETO,		"GetSelectLineTo",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // �I���I���s�̎擾 2005.7.30 maru
	{F_GETSELCOLMTO,		"GetSelectColmTo",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // �I���I�����̎擾 2005.7.30 maru
	{F_ISINSMODE,			"IsInsMode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // �}���^�㏑�����[�h�̎擾 2005.7.30 maru
	{F_GETCHARCODE,			"GetCharCode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // �����R�[�h�擾 2005.07.31 maru
	{F_GETLINECODE,			"GetLineCode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // ���s�R�[�h�擾 2005.08.05 maru
	{F_ISPOSSIBLEUNDO,		"IsPossibleUndo",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // Undo�\�����ׂ� 2005.08.05 maru
	{F_ISPOSSIBLEREDO,		"IsPossibleRedo",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY}, VT_I4, NULL }, // Redo�\�����ׂ� 2005.08.05 maru
	
	//	�I�[
	//	Jun. 27, 2002 genta
	//	�I�[�Ƃ��Ă͌����Č���Ȃ����̂��g���ׂ��Ȃ̂ŁC
	//	FuncID��-1�ɕύX�D(0�͎g����)
	{-1,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoArr[] = 
{
//	�@�\�ԍ�			�֐���			����				��Ɨp�o�b�t�@

	/* �t�@�C������n */
	{F_FILENEW,						"FileNew",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�V�K�쐬
	{F_FILEOPEN,					"FileOpen",				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J��
	{F_FILESAVE,					"FileSave",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�㏑���ۑ�
	{F_FILESAVEALL,					"FileSaveAll",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�㏑���ۑ�
	{F_FILESAVEAS,					"FileSaveAs",			{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //���O��t���ĕۑ�
	{F_FILECLOSE,					"FileClose",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	{F_FILECLOSE_OPEN,				"FileCloseOpen",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ĊJ��
	{F_FILE_REOPEN,					"FileReopen",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SJIS�ŊJ������	//Dec. 4, 2002 genta
	{F_FILE_REOPEN_SJIS,			"FileReopenSJIS",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SJIS�ŊJ������
	{F_FILE_REOPEN_JIS,				"FileReopenJIS",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //JIS�ŊJ������
	{F_FILE_REOPEN_EUC,				"FileReopenEUC",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //EUC�ŊJ������
	{F_FILE_REOPEN_UNICODE,			"FileReopenUNICODE",	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Unicode�ŊJ������
	{F_FILE_REOPEN_UNICODEBE,		"FileReopenUNICODEBE",	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UnicodeBE�ŊJ������
	{F_FILE_REOPEN_UTF8,			"FileReopenUTF8",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UTF-8�ŊJ������
	{F_FILE_REOPEN_UTF7,			"FileReopenUTF7",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UTF-7�ŊJ������
	{F_PRINT,						"Print",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���
//	{F_PRINT_DIALOG,				"PrintDialog",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����_�C�A���O
	{F_PRINT_PREVIEW,				"PrintPreview",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����v���r���[
	{F_PRINT_PAGESETUP,				"PrintPageSetup",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����y�[�W�ݒ�	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	{F_OPEN_HfromtoC,				"OpenHfromtoC",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
	{F_OPEN_HHPP,					"OpenHHpp",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	{F_OPEN_CCPP,					"OpenCCpp",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	{F_ACTIVATE_SQLPLUS,			"ActivateSQLPLUS",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* Oracle SQL*Plus���A�N�e�B�u�\�� */
	{F_PLSQL_COMPILE_ON_SQLPLUS,	"ExecSQLPLUS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* Oracle SQL*Plus�Ŏ��s */
	{F_BROWSE,						"Browse",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u���E�Y
	{F_READONLY,					"ReadOnly",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�ǂݎ���p
	{F_PROPERTY_FILE,				"PropertyFile",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�t�@�C���̃v���p�e�B
	{F_EXITALLEDITORS,				"ExitAllEditors",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�ҏW�̑S�I��	// 2007.02.13 ryoji �ǉ�
	{F_EXITALL,						"ExitAll",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�T�N���G�f�B�^�̑S�I��	//Dec. 27, 2000 JEPRO �ǉ�

	/* �ҏW�n */
	{F_CHAR,				"Char",					{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��������
	{F_IME_CHAR,			"CharIme",				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�S�p��������
	{F_UNDO,				"Undo",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ɖ߂�(Undo)
	{F_REDO,				"Redo",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��蒼��(Redo)
	{F_DELETE,				"Delete",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�폜
	{F_DELETE_BACK,			"DeleteBack",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\���O���폜
	{F_WordDeleteToStart,	"WordDeleteToStart",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��̍��[�܂ō폜
	{F_WordDeleteToEnd,		"WordDeleteToEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��̉E�[�܂ō폜
	{F_WordCut,				"WordCut",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��؂���
	{F_WordDelete,			"WordDelete",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��폜
	{F_LineCutToStart,		"LineCutToStart",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���܂Ő؂���(���s�P��)
	{F_LineCutToEnd,		"LineCutToEnd",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���܂Ő؂���(���s�P��)
	{F_LineDeleteToStart,	"LineDeleteToStart",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���܂ō폜(���s�P��)
	{F_LineDeleteToEnd,		"LineDeleteToEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���܂ō폜(���s�P��)
	{F_CUT_LINE,			"CutLine",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s�؂���(�܂�Ԃ��P��)
	{F_DELETE_LINE,			"DeleteLine",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s�폜(�܂�Ԃ��P��)
	{F_DUPLICATELINE,		"DuplicateLine",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s�̓�d��(�܂�Ԃ��P��)
	{F_INDENT_TAB,			"IndentTab",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //TAB�C���f���g
	{F_UNINDENT_TAB,		"UnindentTab",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�tTAB�C���f���g
	{F_INDENT_SPACE,		"IndentSpace",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SPACE�C���f���g
	{F_UNINDENT_SPACE,		"UnindentSpace",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�tSPACE�C���f���g
//	{F_WORDSREFERENCE,		"WordReference",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P�ꃊ�t�@�����X
	{F_LTRIM,				"LTrim",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��(�擪)�̋󔒂��폜 2001.12.03 hor
	{F_RTRIM,				"RTrim",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�E(����)�̋󔒂��폜 2001.12.03 hor
	{F_SORT_ASC,			"SortAsc",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I���s�̏����\�[�g 2001.12.06 hor
	{F_SORT_DESC,			"SortDesc",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I���s�̍~���\�[�g 2001.12.06 hor
	{F_MERGE,				"Merge",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I���s�̃}�[�W 2001.12.06 hor

	/* �J�[�\���ړ��n */
	{F_UP,					"Up",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\����ړ�
	{F_DOWN,				"Down",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\�����ړ�
	{F_LEFT,				"Left",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\�����ړ�
	{F_RIGHT,				"Right",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\���E�ړ�
	{F_UP2,					"Up2",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\����ړ�(�Q�s����)
	{F_DOWN2,				"Down2",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\�����ړ�(�Q�s����)
	{F_WORDLEFT,			"WordLeft",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��̍��[�Ɉړ�
	{F_WORDRIGHT,			"WordRight",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��̉E�[�Ɉړ�
	{F_GOLINETOP,			"GoLineTop",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���Ɉړ�(�܂�Ԃ��P��)
	{F_GOLINEEND,			"GoLineEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���Ɉړ�(�܂�Ԃ��P��)
	{F_HalfPageUp,			"HalfPageUp",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_HalfPageDown,		"HalfPageDown",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_1PageUp,				"1PageUp",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	{F_1PageDown,			"1PageDown",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	{F_GOFILETOP,			"GoFileTop",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�t�@�C���̐擪�Ɉړ�
	{F_GOFILEEND,			"GoFileEnd",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�t�@�C���̍Ō�Ɉړ�
	{F_CURLINECENTER,		"CurLineCenter",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\���s���E�B���h�E������
	{F_JUMPHIST_PREV,		"MoveHistPrev",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�ړ�����: �O��
	{F_JUMPHIST_NEXT,		"MoveHistNext",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�ړ�����: ����
	{F_JUMPHIST_SET,		"MoveHistSet",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���݈ʒu���ړ������ɓo�^
	{F_WndScrollDown,		"F_WndScrollDown",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	{F_WndScrollUp,			"F_WndScrollUp",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o
	{F_GONEXTPARAGRAPH,		"GoNextParagraph",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̒i���ֈړ�
	{F_GOPREVPARAGRAPH,		"GoPrevParagraph",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̒i���ֈړ�

	/* �I���n */	//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v�������Ȃ����̂Łu�I���n�v�Ƃ��ēƗ���(�T�u���j���[���͍\����ł��Ȃ��̂�)
	{F_SELECTWORD,			"SelectWord",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���݈ʒu�̒P��I��
	{F_SELECTALL,			"SelectAll",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ׂđI��
	{F_BEGIN_SEL,			"BeginSelect",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�͈͑I���J�n Mar. 5, 2001 genta ���̏C��
	{F_UP_SEL,				"Up_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\����ړ�
	{F_DOWN_SEL,			"Down_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\�����ړ�
	{F_LEFT_SEL,			"Left_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\�����ړ�
	{F_RIGHT_SEL,			"Right_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\���E�ړ�
	{F_UP2_SEL,				"Up2_Sel",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\����ړ�(�Q�s����)
	{F_DOWN2_SEL,			"Down2_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
	{F_WORDLEFT_SEL,		"WordLeft_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P��̍��[�Ɉړ�
	{F_WORDRIGHT_SEL,		"WordRight_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P��̉E�[�Ɉړ�
	{F_GOLINETOP_SEL,		"GoLineTop_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	{F_GOLINEEND_SEL,		"GoLineEnd_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	{F_HalfPageUp_Sel,		"HalfPageUp_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_HalfPageDown_Sel,	"HalfPageDown_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_1PageUp_Sel,			"1PageUp_Sel",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	{F_1PageDown_Sel,		"1PageDown_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	{F_GOFILETOP_SEL,		"GoFileTop_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�t�@�C���̐擪�Ɉړ�
	{F_GOFILEEND_SEL,		"GoFileEnd_Sel",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�t�@�C���̍Ō�Ɉړ�
	{F_GONEXTPARAGRAPH_SEL,	"GoNextParagraph_Sel",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̒i���ֈړ�
	{F_GOPREVPARAGRAPH_SEL,	"GoPrevParagraph_Sel",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̒i���ֈړ�

	/* ��`�I���n */	//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
	{F_BEGIN_BOX,			"BeginBoxSelect",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��`�͈͑I���J�n

	/* �N���b�v�{�[�h�n */
	{F_CUT,						"Cut",						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	{F_COPY,					"Copy",						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	{F_PASTE,					"Paste",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�\��t��(�N���b�v�{�[�h����\��t��)
	{F_COPY_ADDCRLF,			"CopyAddCRLF",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[
	{F_COPY_CRLF,				"CopyCRLF",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //CRLF���s�ŃR�s�[(�I��͈͂����s�R�[�h=CRLF�ŃR�s�[)
	{F_PASTEBOX,				"PasteBox",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	{F_INSTEXT,					"InsText",					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // �e�L�X�g��\��t��
	{F_ADDTAIL,					"AddTail",					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // �Ō�Ƀe�L�X�g��ǉ�
	{F_COPYLINES,				"CopyLines",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I��͈͓��S�s�R�s�[
	{F_COPYLINESASPASSAGE,		"CopyLinesAsPassage",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I��͈͓��S�s���p���t���R�s�[
	{F_COPYLINESWITHLINENUMBER,	"CopyLinesWithLineNumber",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I��͈͓��S�s�s�ԍ��t���R�s�[
	{F_COPYPATH,				"CopyPath",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	{F_COPYFNAME,				"CopyFilename",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ // 2002/2/3 aroka
	{F_COPYTAG,					"CopyTag",					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[	//Sept. 15, 2000 jepro ��Ɠ��������ɂȂ��Ă����̂��C��
	{F_CREATEKEYBINDLIST,		"CopyKeyBindList",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�L�[���蓖�Ĉꗗ���R�s�[	//Sept. 15, 2000 JEPRO �ǉ� //Dec. 25, 2000 ����

	/* �}���n */
	{F_INS_DATE,				"InsertDate",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // ���t�}��
	{F_INS_TIME,				"InsertTime",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // �����}��
	{F_CTRL_CODE_DIALOG,		"CtrlCodeDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�R���g���[���R�[�h�̓���(�_�C�A���O)	//@@@ 2002.06.02 MIK

	/* �ϊ��n */
	{F_TOLOWER,		 			"ToLower",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //������
	{F_TOUPPER,		 			"ToUpper",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�啶��
	{F_TOHANKAKU,		 		"ToHankaku",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �S�p�����p */
	{F_TOHANKATA,		 		"ToHankata",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �S�p�J�^�J�i�����p�J�^�J�i */	//Aug. 29, 2002 ai
	{F_TOZENEI,		 			"ToZenEi",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	{F_TOHANEI,		 			"ToHanEi",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �S�p�p�������p�p�� */
	{F_TOZENKAKUKATA,	 		"ToZenKata",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	{F_TOZENKAKUHIRA,	 		"ToZenHira",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	{F_HANKATATOZENKAKUKATA,	"HanKataToZenKata",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�J�^�J�i���S�p�J�^�J�i */
	{F_HANKATATOZENKAKUHIRA,	"HanKataToZenHira",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�J�^�J�i���S�p�Ђ炪�� */
	{F_TABTOSPACE,				"TABToSPACE",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* TAB���� */
	{F_SPACETOTAB,				"SPACEToTAB",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �󔒁�TAB */ //#### Stonee, 2001/05/27
	{F_CODECNV_AUTO2SJIS,		"AutoToSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �������ʁ�SJIS�R�[�h�ϊ� */
	{F_CODECNV_EMAIL,			"JIStoSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //E-Mail(JIS��SJIS)�R�[�h�ϊ�
	{F_CODECNV_EUC2SJIS,		"EUCtoSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //EUC��SJIS�R�[�h�ϊ�
	{F_CODECNV_UNICODE2SJIS,	"CodeCnvUNICODEtoSJIS",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Unicode��SJIS�R�[�h�ϊ�
	{F_CODECNV_UNICODEBE2SJIS,	"CodeCnvUNICODEBEtoSJIS",{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // UnicodeBE��SJIS�R�[�h�ϊ�
	{F_CODECNV_UTF82SJIS,		"UTF8toSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* UTF-8��SJIS�R�[�h�ϊ� */
	{F_CODECNV_UTF72SJIS,		"UTF7toSJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* UTF-7��SJIS�R�[�h�ϊ� */
	{F_CODECNV_SJIS2JIS,		"SJIStoJIS",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS��JIS�R�[�h�ϊ� */
	{F_CODECNV_SJIS2EUC,		"SJIStoEUC",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS��EUC�R�[�h�ϊ� */
	{F_CODECNV_SJIS2UTF8,		"SJIStoUTF8",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS��UTF-8�R�[�h�ϊ� */
	{F_CODECNV_SJIS2UTF7,		"SJIStoUTF7",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS��UTF-7�R�[�h�ϊ� */
	{F_BASE64DECODE,	 		"Base64Decode",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Base64�f�R�[�h���ĕۑ�
	{F_UUDECODE,		 		"Uudecode",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //uudecode���ĕۑ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX


	/* �����n */
	{F_SEARCH_DIALOG,			"SearchDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����(�P�ꌟ���_�C�A���O)
	{F_SEARCH_NEXT,				"SearchNext",		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��������
	{F_SEARCH_PREV,				"SearchPrev",		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O������
	{F_REPLACE_DIALOG,			"ReplaceDialog",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u��(�u���_�C�A���O)
	{F_REPLACE,					"Replace",			{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //�u��(���s)
	{F_REPLACE_ALL,				"ReplaceAll",		{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //���ׂĒu��(���s)
	{F_SEARCH_CLEARMARK,		"SearchClearMark",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�����}�[�N�̃N���A
	{F_JUMP_SRCHSTARTPOS,		"SearchStartPos",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�����J�n�ʒu�֖߂�			// 02/06/26 ai
	{F_GREP,					"Grep",				{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_I4   },	VT_EMPTY,	NULL}, //Grep
	{F_JUMP,					"Jump",				{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�w��s�w�W�����v
	{F_OUTLINE,					"Outline",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�A�E�g���C�����
	{F_TAGJUMP,					"TagJump",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�^�O�W�����v�@�\
	{F_TAGJUMPBACK,				"TagJumpBack",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�^�O�W�����v�o�b�N�@�\
	{F_TAGS_MAKE,				"TagMake",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	{F_DIRECT_TAGJUMP,			"DirectTagJump",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�_�C���N�g�^�O�W�����v�@�\	//@@@ 2003.04.15 MIK
	{F_TAGJUMP_KEYWORD,			"KeywordTagJump",	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�L�[���[�h���w�肵�ă_�C���N�g�^�O�W�����v�@�\ //@@@ 2005.03.31 MIK
	{F_COMPARE,					"Compare",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�t�@�C�����e��r
	{F_DIFF_DIALOG,				"DiffDialog",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��(�_�C�A���O)	//@@@ 2002.05.25 MIK
	{F_DIFF,					"Diff",				{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��				//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	{F_DIFF_NEXT,				"DiffNext",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��(����)			//@@@ 2002.05.25 MIK
	{F_DIFF_PREV,				"DiffPrev",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��(�O��)			//@@@ 2002.05.25 MIK
	{F_DIFF_RESET,				"DiffReset",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��(�S����)		//@@@ 2002.05.25 MIK
	{F_BRACKETPAIR,				"BracketPair",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�Ί��ʂ̌���
// From Here 2001.12.03 hor
	{F_BOOKMARK_SET,			"BookmarkSet",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u�b�N�}�[�N�ݒ�E����
	{F_BOOKMARK_NEXT,			"BookmarkNext",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃u�b�N�}�[�N��
	{F_BOOKMARK_PREV,			"BookmarkPrev",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̃u�b�N�}�[�N��
	{F_BOOKMARK_RESET,			"BookmarkReset",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u�b�N�}�[�N�̑S����
	{F_BOOKMARK_VIEW,			"BookmarkView",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u�b�N�}�[�N�̈ꗗ
// To Here 2001.12.03 hor
	{F_BOOKMARK_PATTERN,		"BookmarkPattern",	{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 2002.01.16 hor �w��p�^�[���Ɉ�v����s���}�[�N

	/* ���[�h�؂�ւ��n */
	{F_CHGMOD_INS,				"ChgmodINS",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�}���^�㏑�����[�h�؂�ւ�
	{F_CHGMOD_EOL,				"ChgmodEOL",		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���͉��s�R�[�h�w�� 2003.06.23 Moca
	{F_CANCEL_MODE,				"CancelMode",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�e�탂�[�h�̎�����

	/* �ݒ�n */
	{F_SHOWTOOLBAR,				"ShowToolbar",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �c�[���o�[�̕\�� */
	{F_SHOWFUNCKEY,				"ShowFunckey",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �t�@���N�V�����L�[�̕\�� */
	{F_SHOWTAB,					"ShowTab",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �^�u�̕\�� */	//@@@ 2003.06.10 MIK
	{F_SHOWSTATUSBAR,			"ShowStatusbar",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �X�e�[�^�X�o�[�̕\�� */
	{F_TYPE_LIST,				"TypeList",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �^�C�v�ʐݒ�ꗗ */
	{F_OPTION_TYPE,				"OptionType",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �^�C�v�ʐݒ� */
	{F_OPTION,					"OptionCommon",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���ʐݒ� */
	{F_FONT,					"SelectFont",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �t�H���g�ݒ� */
	{F_WRAPWINDOWWIDTH,			"WrapWindowWidth",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
	{F_FAVORITE,				"OptionFavorite",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���C�ɓ���̐ݒ� */	//@@@ 2003.04.08 MIK
	{F_SET_QUOTESTRING,			"SetMsgQuoteStr",	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���ʐݒ聨���������p���̐ݒ� */	//Jan. 29, 2005 genta


	//	Oct. 9, 2001 genta �ǉ�
	{F_EXECCOMMAND,				"ExecCommand",		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �O���R�}���h���s */

	/* �J�X�^�����j���[ */
	{F_MENU_RBUTTON,			"RMenu",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �E�N���b�N���j���[ */
	{F_CUSTMENU_1,				"CustMenu1",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[1 */
	{F_CUSTMENU_2,				"CustMenu2",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[2 */
	{F_CUSTMENU_3,				"CustMenu3",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[3 */
	{F_CUSTMENU_4,				"CustMenu4",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[4 */
	{F_CUSTMENU_5,				"CustMenu5",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[5 */
	{F_CUSTMENU_6,				"CustMenu6",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[6 */
	{F_CUSTMENU_7,				"CustMenu7",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[7 */
	{F_CUSTMENU_8,				"CustMenu8",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[8 */
	{F_CUSTMENU_9,				"CustMenu9",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[9 */
	{F_CUSTMENU_10,				"CustMenu10",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[10 */
	{F_CUSTMENU_11,				"CustMenu11",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[11 */
	{F_CUSTMENU_12,				"CustMenu12",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[12 */
	{F_CUSTMENU_13,				"CustMenu13",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[13 */
	{F_CUSTMENU_14,				"CustMenu14",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[14 */
	{F_CUSTMENU_15,				"CustMenu15",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[15 */
	{F_CUSTMENU_16,				"CustMenu16",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[16 */
	{F_CUSTMENU_17,				"CustMenu17", 		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[17 */
	{F_CUSTMENU_18,				"CustMenu18",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[18 */
	{F_CUSTMENU_19,				"CustMenu19",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[19 */
	{F_CUSTMENU_20,				"CustMenu20",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[20 */
	{F_CUSTMENU_21,				"CustMenu21",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[21 */
	{F_CUSTMENU_22,				"CustMenu22",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[22 */
	{F_CUSTMENU_23,				"CustMenu23",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[23 */
	{F_CUSTMENU_24,				"CustMenu24",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[24 */

	/* �E�B���h�E�n */
	{F_SPLIT_V,					"SplitWinV",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�㉺�ɕ���	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	{F_SPLIT_H,					"SplitWinH",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���E�ɕ���	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	{F_SPLIT_VH,				"SplitWinVH",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	{F_WINCLOSE,				"WinClose",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�E�B���h�E�����
	{F_WIN_CLOSEALL,			"WinCloseAll",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
	{F_CASCADE,					"CascadeWin",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�d�˂ĕ\��
	{F_TILE_V,					"TileWinV",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�㉺�ɕ��ׂĕ\��
	{F_TILE_H,					"TileWinH",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���E�ɕ��ׂĕ\��
	{F_NEXTWINDOW,				"NextWindow",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃E�B���h�E
	{F_PREVWINDOW,				"PrevWindow",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̃E�B���h�E
	{F_WINLIST,					"WindowList",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�E�B���h�E�ꗗ�|�b�v�A�b�v�\��	// 2006.03.23 fon
	{F_MAXIMIZE_V,				"MaximizeV",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�c�����ɍő剻
	{F_MAXIMIZE_H,				"MaximizeH",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�������ɍő剻 //2001.02.10 by MIK
	{F_MINIMIZE_ALL,			"MinimizeAll",			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ׂčŏ���	//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	{F_REDRAW,					"ReDraw",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�ĕ`��
	{F_WIN_OUTPUT,				"ActivateWinOutput",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�A�E�g�v�b�g�E�B���h�E�\��
	{F_TRACEOUT,				"TraceOut",				{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�}�N���p�A�E�g�v�b�g�E�B���h�E�ɏo��	2006.04.26 maru
	{F_TOPMOST,					"WindowTopMost",	{VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��Ɏ�O�ɕ\��

	/* �x�� */
	{F_HOKAN,					"Complete",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���͕⊮ */	//Oct. 15, 2000 JEPRO �����ĂȂ������̂ŉp����t���ē���Ă݂�
	{F_HELP_CONTENTS,			"HelpContents",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO �ǉ�
	{F_HELP_SEARCH,				"HelpSearch",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO �ǉ�
	{F_MENU_ALLFUNC,			"CommandList",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �R�}���h�ꗗ */
	{F_EXTHELP1,				"ExtHelp1",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �O���w���v�P */
	//	Jul. 5, 2002 genta �����ǉ�
	{F_EXTHTMLHELP,				"ExtHtmlHelp",	{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �O��HTML�w���v */
	{F_ABOUT,					"About",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	//	�I�[
	//	Jun. 27, 2002 genta
	//	�I�[�Ƃ��Ă͌����Č���Ȃ����̂��g���ׂ��Ȃ̂ŁC
	//	FuncID��-1�ɕύX�D(0�͎g����)
	{-1,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};
//int	CSMacroMgr::m_nMacroFuncInfoArrNum = sizeof( CSMacroMgr::m_MacroFuncInfoArr ) / sizeof( CSMacroMgr::m_MacroFuncInfoArr[0] );

/*!
	@date 2002.02.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2002.04.29 genta �I�u�W�F�N�g�̎��͎̂��s���܂Ő������Ȃ��B
*/
CSMacroMgr::CSMacroMgr()
{
	MY_RUNNINGTIMER( cRunningTimer, "CSMacroMgr::CSMacroMgr" );
	
	m_pShareData = CShareData::getInstance()->GetShareData();
	
	CPPAMacroMgr::declare();
	CKeyMacroMgr::declare();
	CWSHMacroManager::declare();
	
	int i;
	for ( i = 0 ; i < MAX_CUSTMACRO ; i++ ){
		m_cSavedKeyMacro[i] = NULL;
	}
	//	Jun. 16, 2002 genta
	m_pKeyMacro = NULL;

	//	Sep. 15, 2005 FILE
	SetCurrentIdx( INVALID_MACRO_IDX );
}

CSMacroMgr::~CSMacroMgr()
{
	//- 20011229 add by aroka
	ClearAll();
	
	//	Jun. 16, 2002 genta
	//	ClearAll�Ɠ��������������̂ō폜
}

/*! �L�[�}�N���̃o�b�t�@���N���A���� */
void CSMacroMgr::ClearAll( void )
{
	int i;
	for (i = 0; i < MAX_CUSTMACRO; i++){
		//	Apr. 29, 2002 genta
		delete m_cSavedKeyMacro[i];
		m_cSavedKeyMacro[i] = NULL;
	}
	//	Jun. 16, 2002 genta
	delete m_pKeyMacro;
	m_pKeyMacro = NULL;
}

/*! @brief�L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�

	@param nFuncID [in] �@�\�ԍ�
	@param lParam1 [in] �p�����[�^�B
	@param mbuf [in] �ǂݍ��ݐ�}�N���o�b�t�@
	
	@date 2002.06.16 genta �L�[�}�N���̑���Ή��̂��ߕύX

*/
int CSMacroMgr::Append( int idx, /*CSMacroMgr::Macro1& mbuf, */ int nFuncID, LPARAM lParam1, CEditView* pcEditView )
{
	assert( idx == STAND_KEYMACRO );
	if (idx == STAND_KEYMACRO){
		CKeyMacroMgr* pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		if( pKeyMacro == NULL ){
			//	1. ���̂��܂������ꍇ
			//	2. CKeyMacroMgr�ȊO�̕��������Ă����ꍇ
			//	������ɂ��Ă��Đ�������D
			delete m_pKeyMacro;
			m_pKeyMacro = new CKeyMacroMgr;
			pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		}
		pKeyMacro->Append( nFuncID, lParam1, pcEditView );
	}
//	else {
		//m_cSavedKeyMacro[idx]->Append( nFuncID, lParam1, pcEditView );
//	}
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
		//	Jun. 16, 2002 genta
		//	�L�[�}�N���ȊO�̃T�|�[�g�ɂ��NULL�̉\�����o�Ă����̂Ŕ���ǉ�
		if( m_pKeyMacro != NULL ){
			//	Sep. 15, 2005 FILE
			SetCurrentIdx( idx );
			m_pKeyMacro->ExecKeyMacro( pcEditView );
			//	Sep. 15, 2005 FILE
			SetCurrentIdx( INVALID_MACRO_IDX );
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	if( idx < 0 || MAX_CUSTMACRO <= idx )	//	�͈̓`�F�b�N
		return FALSE;

	/* �ǂݍ��ݑO���A����ǂݍ��ސݒ�̏ꍇ�́A�t�@�C����ǂݍ��݂Ȃ��� */
	//	Apr. 29, 2002 genta
	if( m_cSavedKeyMacro[idx] == NULL || CShareData::getInstance()->BeReloadWhenExecuteMacro( idx )){
		//	CShareData����A�}�N���t�@�C�������擾
		//	Jun. 08, 2003 Moca �Ăяo�����Ńp�X����p��
		//	Jun. 16, 2003 genta ������������ƕύX
		char ptr[_MAX_PATH * 2];
		int n = CShareData::getInstance()->GetMacroFilename( idx, ptr, sizeof(ptr) );
		if ( n <= 0 ){
			return FALSE;
		}

		if( !Load( idx, hInstance, ptr ) )
			return FALSE;
	}

	//	Sep. 15, 2005 FILE
	SetCurrentIdx( idx );
	m_cSavedKeyMacro[idx]->ExecKeyMacro(pcEditView);
	//	Sep. 15, 2005 FILE
	SetCurrentIdx( INVALID_MACRO_IDX );

	pcEditView->Redraw();	//	�K�v�H
	return TRUE;
}

/*! �L�[�{�[�h�}�N���̓ǂݍ���

	@param idx [in] �ǂݍ��ݐ�}�N���o�b�t�@�ԍ�
	@param pszPath [in] �}�N���t�@�C����

	�ǂݍ��݂Ɏ��s�����Ƃ��̓}�N���o�b�t�@�̃I�u�W�F�N�g�͉������C
	NULL���ݒ肳���D

	@author Norio Nakatani, YAZAKI, genta
*/
BOOL CSMacroMgr::Load( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	CMacroManagerBase** ppMacro = Idx2Ptr( idx );

	if( ppMacro == NULL ){
#ifdef _DEBUG
	MYTRACE( "CSMacroMgr::Load() Out of range: idx=%d Path=%s\n", idx, pszPath);
#endif
	}
	//	�o�b�t�@�N���A
	delete *ppMacro;
	*ppMacro = NULL;
	
	const char *ext = strrchr( pszPath, '.');
	//	Feb. 02, 2004 genta .�������ꍇ��ext==NULL�ƂȂ�̂�NULL�`�F�b�N�ǉ�
	if( ext != NULL ){
		const char *chk = strrchr( ext, '\\' );
		if( chk != NULL ){	//	.�̂��Ƃ�\���������炻��͊g���q�̋�؂�ł͂Ȃ�
							//	\��������2�o�C�g�ڂ̏ꍇ���g���q�ł͂Ȃ��B
			ext = NULL;
		}
	}
	if(ext != NULL){
		++ext;
	}
	*ppMacro = CMacroFactory::Instance()->Create(ext);
	if( *ppMacro == NULL )
		return FALSE;
	//	From Here Jun. 16, 2002 genta
	//	�ǂݍ��݃G���[���̓C���X�^���X�폜
	if( (*ppMacro)->LoadKeyMacro(hInstance, pszPath )){
		return TRUE;
	}
	else {
		delete *ppMacro;
		*ppMacro = NULL;
	}
	//	To Here Jun. 16, 2002 genta
	return FALSE;
}

/*! �L�[�{�[�h�}�N���̕ۑ�

	@param idx [in] �ǂݍ��ݐ�}�N���o�b�t�@�ԍ�
	@param pszPath [in] �}�N���t�@�C����
	@param hInstance [in] �C���X�^���X�n���h��

	@author YAZAKI
*/
BOOL CSMacroMgr::Save( int idx/* CSMacroMgr::Macro1& mbuf */, HINSTANCE hInstance, const char* pszPath )
{
	assert( idx == STAND_KEYMACRO );
	if ( idx == STAND_KEYMACRO ){
		CKeyMacroMgr* pKeyMacro = dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro );
		if( pKeyMacro != NULL ){
			return pKeyMacro->SaveKeyMacro(hInstance, pszPath );
		}
		//	Jun. 27, 2002 genta
		//	��}�N���̏ꍇ�͐���I���ƌ��Ȃ��D
		if( m_pKeyMacro == NULL ){
			return TRUE;
		}

	}
//	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
//		return m_cSavedKeyMacro[idx]->SaveKeyMacro(hInstance, pszPath );
//	}
	return FALSE;
}

/*
	�w�肳�ꂽ�}�N�����N���A����
	
	@param idx [in] �}�N���ԍ�(0-), STAND_KEYMACRO�͕W���L�[�}�N���o�b�t�@��\���D
*/
void CSMacroMgr::Clear( int idx )
{
	CMacroManagerBase **ppMacro = Idx2Ptr( idx );
	if( ppMacro != NULL ){
		delete *ppMacro;
		*ppMacro = NULL;
	}
}

/*
||  Attributes & Operations
*/
/*
	�w�肳�ꂽID�ɑΉ�����MacroInfo�\���̂ւ̃|�C���^��Ԃ��D
	�Y������ID�ɑΉ�����\���̂��Ȃ����NULL��Ԃ��D

	@param nFuncID [in] �@�\�ԍ�
	@return �\���̂ւ̃|�C���^�D������Ȃ����NULL
	
	@date 2002.06.16 genta
	@data 2003.02.24 m_MacroFuncInfoNotCommandArr�������Ώۂɂ���
*/
const MacroFuncInfo* CSMacroMgr::GetFuncInfoByID( int nFuncID )
{
	int i;
	//	Jun. 27, 2002 genta
	//	�Ԑl���R�[�h0�Ƃ��ďE���Ă��܂��̂ŁC�z��T�C�Y�ɂ�锻�����߂��D
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoArr[i].m_nFuncID == nFuncID ){
			return &m_MacroFuncInfoArr[i];
		}
	}
	for( i = 0; m_MacroFuncInfoNotCommandArr[i].m_pszFuncName != NULL; ++i ){
		if( m_MacroFuncInfoNotCommandArr[i].m_nFuncID == nFuncID ){
			return &m_MacroFuncInfoNotCommandArr[i];
		}
	}
	return NULL;
}

/*!
	�@�\�ԍ�����֐����Ƌ@�\�����{����擾
	
	@param hInstance [in] ���\�[�X�擾�̂��߂�Instance Handle
	@param nFuncID [in] �@�\�ԍ�
	@param pszFuncName [out] �֐����D���̐�ɂ͍Œ��֐����{1�o�C�g�̃��������K�v�D
	@param pszFuncNameJapanese [out] �@�\�����{��D���̐�ɂ�256�o�C�g�̃��������K�v�D
	@return ���������Ƃ���pszFuncName�D������Ȃ������Ƃ���NULL�D
	
	@note
	���ꂼ��C������i�[�̈�̎w���悪NULL�̎��͕�������i�[���Ȃ��D
	�������CpszFuncName��NULL�ɂ��Ă��܂��Ɩ߂�l�����NULL�ɂȂ���
	�������肪�s���Ȃ��Ȃ�D
	
	@date 2002.06.16 genta �V�݂�GetFuncInfoById(int)������Ŏg���悤�ɁD
*/
char* CSMacroMgr::GetFuncInfoByID( HINSTANCE hInstance, int nFuncID, char* pszFuncName, char* pszFuncNameJapanese )
{
	const MacroFuncInfo* MacroInfo = GetFuncInfoByID( nFuncID );
	if( MacroInfo != NULL ){
		if( pszFuncName != NULL ){
			strcpy( pszFuncName, MacroInfo->m_pszFuncName );
			char *p = pszFuncName;
			while (*p){
				if (*p == '('){
					*p = '\0';
					break;
				}
				*p++;
			}
		}
		//	Jun. 16, 2002 genta NULL�̂Ƃ��͉������Ȃ��D
		if( pszFuncNameJapanese != NULL ){
			::LoadString( hInstance, nFuncID, pszFuncNameJapanese, 255 );
		}
		return pszFuncName;
	}
	return NULL;
}

/*!
	�֐����iS_xxxx�j����@�\�ԍ��Ƌ@�\�����{����擾�D
	�֐�����S_�Ŏn�܂�ꍇ�Ǝn�܂�Ȃ��ꍇ�̗����ɑΉ��D

	@param hInstance [in] ���\�[�X�擾�̂��߂�Instance Handle
	@param pszFuncName [in] �֐���
	@param pszFuncNameJapanese [out] �@�\�����{��D���̐�ɂ�256�o�C�g�̃��������K�v�D
	@return ���������Ƃ��͋@�\�ԍ��D������Ȃ������Ƃ���-1�D
	
	@note
	pszFuncNameJapanese �̎w���悪NULL�̎��͓��{�ꖼ���i�[���Ȃ��D
	
	@date 2002.06.16 genta ���[�v���̕�����R�s�[��r��
*/
int CSMacroMgr::GetFuncInfoByName( HINSTANCE hInstance, const char* pszFuncName, char* pszFuncNameJapanese )
{
	int		i;
	int		nFuncID;
	//	Jun. 16, 2002 genta
	const char *normalizedFuncName;
	
	//	S_�Ŏn�܂��Ă��邩
	if( pszFuncName == NULL ){
		return -1;
	}
	if( pszFuncName[0] == 'S' && pszFuncName[1] == '_' ){
		normalizedFuncName = pszFuncName + 2;
	}
	else {
		normalizedFuncName = pszFuncName;
	}
	
	char szBuffer[1024] = "S_";
	for( i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == strcmp( normalizedFuncName, m_MacroFuncInfoArr[i].m_pszFuncName )){
			nFuncID = m_MacroFuncInfoArr[i].m_nFuncID;
			if( pszFuncNameJapanese != NULL ){
				::LoadString( hInstance, nFuncID, pszFuncNameJapanese, 255 );
			}
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
	case F_FILE_REOPEN				://�J������	//Dec. 4, 2002 genta
	case F_FILE_REOPEN_SJIS			://SJIS�ŊJ������
	case F_FILE_REOPEN_JIS			://JIS�ŊJ������
	case F_FILE_REOPEN_EUC			://EUC�ŊJ������
	case F_FILE_REOPEN_UNICODE		://Unicode�ŊJ������
	case F_FILE_REOPEN_UNICODEBE	://UnicodeBE�ŊJ������
	case F_FILE_REOPEN_UTF8			://UTF-8�ŊJ������
	case F_FILE_REOPEN_UTF7			://UTF-7�ŊJ������
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
//	case F_EXITALLEDITORS			://�ҏW�̑S�I��	// 2007.02.13 ryoji �ǉ�
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
	case F_JUMPHIST_PREV			://�ړ�����: �O��
	case F_JUMPHIST_NEXT			://�ړ�����: ����
	case F_JUMPHIST_SET				://���݈ʒu���ړ������ɓo�^

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
//	case F_CTRL_CODE_DIALOG			://�R���g���[���R�[�h�̓���(�_�C�A���O)	//@@@ 2002.06.02 MIK

	/* �ϊ��n */
	case F_TOLOWER		 			://������
	case F_TOUPPER		 			://�啶��
	case F_TOHANKAKU		 		:/* �S�p�����p */
	case F_TOHANKATA		 		:/* �S�p�J�^�J�i�����p�J�^�J�i */	//Aug. 29, 2002 ai
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
	case F_CODECNV_UNICODEBE2SJIS	://UnicodeBE��SJIS�R�[�h�ϊ�
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
	case F_JUMP_SRCHSTARTPOS		://�����J�n�ʒu�֖߂�		// 02/06/26 ai
	case F_GREP						://Grep
//	case F_JUMP_DIALOG				://�w��s�w�W�����v
	case F_JUMP						://�w��s�փW�����v @@@ 2002.2.2 YAZAKI
//	case F_OUTLINE					://�A�E�g���C�����
	case F_TAGJUMP					://�^�O�W�����v�@�\
	case F_TAGJUMPBACK				://�^�O�W�����v�o�b�N�@�\
//	case F_TAGS_MAKE				://�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
//	case F_COMPARE					://�t�@�C�����e��r
//	case F_DIFF_DIALOG				://DIFF�����\��(�_�C�A���O)	//@@@ 2002.05.25 MIK
//	case F_DIFF						://DIFF�����\��				//@@@ 2002.05.25 MIK
//	case F_DIFF_NEXT				://DIFF�����\��(����)		//@@@ 2002.05.25 MIK
//	case F_DIFF_PREV				://DIFF�����\��(�O��)		//@@@ 2002.05.25 MIK
//	case F_DIFF_RESET				://DIFF�����\��(�S����)		//@@@ 2002.05.25 MIK
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
	case F_CHGMOD_EOL				://���͉��s�R�[�h�w��	2003.06.23 Moca

	case F_CANCEL_MODE				://�e�탂�[�h�̎�����

	/* �ݒ�n */
//	case F_SHOWTOOLBAR				:/* �c�[���o�[�̕\�� */
//	case F_SHOWFUNCKEY				:/* �t�@���N�V�����L�[�̕\�� */
//	case F_SHOWTAB					:/* �^�u�̕\�� */
//	case F_SHOWSTATUSBAR			:/* �X�e�[�^�X�o�[�̕\�� */
//	case F_TYPE_LIST				:/* �^�C�v�ʐݒ�ꗗ */
//	case F_OPTION_TYPE				:/* �^�C�v�ʐݒ� */
//	case F_OPTION					:/* ���ʐݒ� */
//	case F_FONT						:/* �t�H���g�ݒ� */
//	case F_WRAPWINDOWWIDTH			:/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 15, 2000 JEPRO
//	case F_FAVORITE					:/* ���C�ɓ���̐ݒ� */	//@@@ 2003.04.08 MIK

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
//	case F_TRACEOUT					://�}�N���p�A�E�g�v�b�g�E�B���h�E�ɕ\��	2006.04.26 maru
	case F_TOPMOST					://��Ɏ�O�ɕ\��

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

/*!
	�}�N���ԍ�����Ή�����}�N���I�u�W�F�N�g�i�[�ʒu�ւ̃|�C���^�ւ̕ϊ�
	
	@param idx [in] �}�N���ԍ�(0-), STAND_KEYMACRO�͕W���L�[�}�N���o�b�t�@��\���D
	@return �I�u�W�F�N�g�ʒu�ւ̃|�C���^�D�}�N���ԍ����s���ȏꍇ��NULL�D
*/
CMacroManagerBase** CSMacroMgr::Idx2Ptr(int idx)
{
	//	Jun. 16, 2002 genta
	//	�L�[�}�N���ȊO�̃}�N����ǂݍ��߂�悤��
	if ( idx == STAND_KEYMACRO ){
		return &m_pKeyMacro;
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		return &m_cSavedKeyMacro[idx];
	}

#ifdef _DEBUG
	MYTRACE( "CSMacroMgr::Idx2Ptr() Out of range: idx=%d\n", idx);
#endif

	return NULL;
}

/*!
	�L�[�{�[�h�}�N���̕ۑ����\���ǂ���
	
	@retval TRUE �ۑ��\
	@retval FALSE �ۑ��s��
*/
BOOL CSMacroMgr::IsSaveOk(void)
{
	return dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro ) == NULL ? FALSE : TRUE;
}
/*[EOF]*/
