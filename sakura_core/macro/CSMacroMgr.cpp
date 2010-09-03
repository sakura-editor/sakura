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
	Copyright (C) 2006, �����, fon, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, nasukoji, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "macro/CSMacroMgr.h"
#include "macro/CPPAMacroMgr.h"
#include "macro/CWSHManager.h"
#include "macro/CMacroFactory.h"
#include "env/CShareData.h"
#include "view/CEditView.h"
#include "debug/CRunningTimer.h"

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoNotCommandArr[] = 
{
	//ID					�֐���							����										�߂�l�̌^	m_pszData
	{F_GETFILENAME,			LTEXT("GetFilename"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //�t�@�C������Ԃ�
	{F_GETSAVEFILENAME,		LTEXT("GetSaveFilename"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //�ۑ����̃t�@�C������Ԃ� 2006.09.04 ryoji
	{F_GETSELECTED,			LTEXT("GetSelectedString"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //�I�𕔕�
	{F_EXPANDPARAMETER,		LTEXT("ExpandParameter"),		{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //���ꕶ���̓W�J
	{F_GETLINESTR,			LTEXT("GetLineStr"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, // �w��_���s�̎擾 2003.06.01 Moca
	{F_GETLINECOUNT,		LTEXT("GetLineCount"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �S�_���s���̎擾 2003.06.01 Moca
	{F_CHGTABWIDTH,			LTEXT("ChangeTabWidth"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //�^�u�T�C�Y�ύX 2004.03.16 zenryaku
	{F_ISTEXTSELECTED,		LTEXT("IsTextSelected"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //�e�L�X�g���I������Ă��邩 2005.7.30 maru
	{F_GETSELLINEFROM,		LTEXT("GetSelectLineFrom"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �I���J�n�s�̎擾 2005.7.30 maru
	{F_GETSELCOLMFROM,		LTEXT("GetSelectColmFrom"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �I���J�n���̎擾 2005.7.30 maru
	{F_GETSELLINETO,		LTEXT("GetSelectLineTo"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �I���I���s�̎擾 2005.7.30 maru
	{F_GETSELCOLMTO,		LTEXT("GetSelectColmTo"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �I���I�����̎擾 2005.7.30 maru
	{F_ISINSMODE,			LTEXT("IsInsMode"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �}���^�㏑�����[�h�̎擾 2005.7.30 maru
	{F_GETCHARCODE,			LTEXT("GetCharCode"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �����R�[�h�擾 2005.07.31 maru
	{F_GETLINECODE,			LTEXT("GetLineCode"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // ���s�R�[�h�擾 2005.08.05 maru
	{F_ISPOSSIBLEUNDO,		LTEXT("IsPossibleUndo"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // Undo�\�����ׂ� 2005.08.05 maru
	{F_ISPOSSIBLEREDO,		LTEXT("IsPossibleRedo"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // Redo�\�����ׂ� 2005.08.05 maru
	{F_CHGWRAPCOLM,			LTEXT("ChangeWrapColm"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //�܂�Ԃ����ύX 2008.06.19 ryoji
	{F_ISCURTYPEEXT,		LTEXT("IsCurTypeExt"),			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �w�肵���g���q�����݂̃^�C�v�ʐݒ�Ɋ܂܂�Ă��邩�ǂ����𒲂ׂ� 2006.09.04 ryoji
	{F_ISSAMETYPEEXT,		LTEXT("IsSameTypeExt"),			{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, // �Q�̊g���q�������^�C�v�ʐݒ�Ɋ܂܂�Ă��邩�ǂ����𒲂ׂ� 2006.09.04 ryoji

	//	�I�[
	//	Jun. 27, 2002 genta
	//	�I�[�Ƃ��Ă͌����Č���Ȃ����̂��g���ׂ��Ȃ̂ŁC
	//	FuncID��-1�ɕύX�D(0�͎g����)
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

MacroFuncInfo CSMacroMgr::m_MacroFuncInfoArr[] = 
{
//	�@�\�ԍ�			�֐���			����				��Ɨp�o�b�t�@

	/* �t�@�C������n */
	{F_FILENEW,						LTEXT("FileNew"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�V�K�쐬
	{F_FILEOPEN,					LTEXT("FileOpen"),				{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J��
	{F_FILESAVE,					LTEXT("FileSave"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�㏑���ۑ�
	{F_FILESAVEALL,					LTEXT("FileSaveAll"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�㏑���ۑ�
	{F_FILESAVEAS,					LTEXT("FileSaveAs"),			{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //���O��t���ĕۑ�
	{F_FILECLOSE,					LTEXT("FileClose"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	{F_FILECLOSE_OPEN,				LTEXT("FileCloseOpen"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ĊJ��
	{F_FILE_REOPEN,					LTEXT("FileReopen"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SJIS�ŊJ������	//Dec. 4, 2002 genta
	{F_FILE_REOPEN_SJIS,			LTEXT("FileReopenSJIS"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SJIS�ŊJ������
	{F_FILE_REOPEN_JIS,				LTEXT("FileReopenJIS"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //JIS�ŊJ������
	{F_FILE_REOPEN_EUC,				LTEXT("FileReopenEUC"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //EUC�ŊJ������
	{F_FILE_REOPEN_UNICODE,			LTEXT("FileReopenUNICODE"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Unicode�ŊJ������
	{F_FILE_REOPEN_UNICODEBE,		LTEXT("FileReopenUNICODEBE"),	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UnicodeBE�ŊJ������
	{F_FILE_REOPEN_UTF8,			LTEXT("FileReopenUTF8"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UTF-8�ŊJ������
	{F_FILE_REOPEN_CESU8,			LTEXT("FileReopenCESU8"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //CESU-8�ŊJ������
	{F_FILE_REOPEN_UTF7,			LTEXT("FileReopenUTF7"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //UTF-7�ŊJ������
	{F_PRINT,						LTEXT("Print"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���
//	{F_PRINT_DIALOG,				LTEXT("PrintDialog"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����_�C�A���O
	{F_PRINT_PREVIEW,				LTEXT("PrintPreview"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����v���r���[
	{F_PRINT_PAGESETUP,				LTEXT("PrintPageSetup"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����y�[�W�ݒ�	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	{F_OPEN_HfromtoC,				LTEXT("OpenHfromtoC"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
//	{F_OPEN_HHPP,					LTEXT("OpenHHpp"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX		del 2008/6/23 Uchi
//	{F_OPEN_CCPP,					LTEXT("OpenCCpp"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX	del 2008/6/23 Uchi
	{F_ACTIVATE_SQLPLUS,			LTEXT("ActivateSQLPLUS"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* Oracle SQL*Plus���A�N�e�B�u�\�� */
	{F_PLSQL_COMPILE_ON_SQLPLUS,	LTEXT("ExecSQLPLUS"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* Oracle SQL*Plus�Ŏ��s */
	{F_BROWSE,						LTEXT("Browse"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u���E�Y
	{F_VIEWMODE,					LTEXT("ViewMode"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�r���[���[�h
	{F_VIEWMODE,					LTEXT("ReadOnly"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�r���[���[�h(��)
	{F_PROPERTY_FILE,				LTEXT("PropertyFile"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�t�@�C���̃v���p�e�B
	{F_EXITALLEDITORS,				LTEXT("ExitAllEditors"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�ҏW�̑S�I��	// 2007.02.13 ryoji �ǉ�
	{F_EXITALL,						LTEXT("ExitAll"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�T�N���G�f�B�^�̑S�I��	//Dec. 27, 2000 JEPRO �ǉ�
	{F_PUTFILE,						LTEXT("PutFile"),				{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},   VT_EMPTY,	NULL}, // ��ƒ��t�@�C���̈ꎞ�o�� 2006.12.10 maru
	{F_INSFILE,						LTEXT("InsFile"),				{VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY},   VT_EMPTY,	NULL}, // �L�����b�g�ʒu�Ƀt�@�C���}�� 2006.12.10 maru

	/* �ҏW�n */
	{F_WCHAR,				LTEXT("Char"),					{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��������
	{F_IME_CHAR,			LTEXT("CharIme"),				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�S�p��������
	{F_UNDO,				LTEXT("Undo"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ɖ߂�(Undo)
	{F_REDO,				LTEXT("Redo"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��蒼��(Redo)
	{F_DELETE,				LTEXT("Delete"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�폜
	{F_DELETE_BACK,			LTEXT("DeleteBack"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\���O���폜
	{F_WordDeleteToStart,	LTEXT("WordDeleteToStart"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��̍��[�܂ō폜
	{F_WordDeleteToEnd,		LTEXT("WordDeleteToEnd"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��̉E�[�܂ō폜
	{F_WordCut,				LTEXT("WordCut"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��؂���
	{F_WordDelete,			LTEXT("WordDelete"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��폜
	{F_LineCutToStart,		LTEXT("LineCutToStart"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���܂Ő؂���(���s�P��)
	{F_LineCutToEnd,		LTEXT("LineCutToEnd"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���܂Ő؂���(���s�P��)
	{F_LineDeleteToStart,	LTEXT("LineDeleteToStart"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���܂ō폜(���s�P��)
	{F_LineDeleteToEnd,		LTEXT("LineDeleteToEnd"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���܂ō폜(���s�P��)
	{F_CUT_LINE,			LTEXT("CutLine"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s�؂���(�܂�Ԃ��P��)
	{F_DELETE_LINE,			LTEXT("DeleteLine"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s�폜(�܂�Ԃ��P��)
	{F_DUPLICATELINE,		LTEXT("DuplicateLine"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s�̓�d��(�܂�Ԃ��P��)
	{F_INDENT_TAB,			LTEXT("IndentTab"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //TAB�C���f���g
	{F_UNINDENT_TAB,		LTEXT("UnindentTab"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�tTAB�C���f���g
	{F_INDENT_SPACE,		LTEXT("IndentSpace"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //SPACE�C���f���g
	{F_UNINDENT_SPACE,		LTEXT("UnindentSpace"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�tSPACE�C���f���g
//	{F_WORDSREFERENCE,		LTEXT("WordReference"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P�ꃊ�t�@�����X
	{F_LTRIM,				LTEXT("LTrim"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��(�擪)�̋󔒂��폜 2001.12.03 hor
	{F_RTRIM,				LTEXT("RTrim"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�E(����)�̋󔒂��폜 2001.12.03 hor
	{F_SORT_ASC,			LTEXT("SortAsc"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I���s�̏����\�[�g 2001.12.06 hor
	{F_SORT_DESC,			LTEXT("SortDesc"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I���s�̍~���\�[�g 2001.12.06 hor
	{F_MERGE,				LTEXT("Merge"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I���s�̃}�[�W 2001.12.06 hor

	/* �J�[�\���ړ��n */
	{F_UP,					LTEXT("Up"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\����ړ�
	{F_DOWN,				LTEXT("Down"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\�����ړ�
	{F_LEFT,				LTEXT("Left"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\�����ړ�
	{F_RIGHT,				LTEXT("Right"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\���E�ړ�
	{F_UP2,					LTEXT("Up2"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\����ړ�(�Q�s����)
	{F_DOWN2,				LTEXT("Down2"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\�����ړ�(�Q�s����)
	{F_WORDLEFT,			LTEXT("WordLeft"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��̍��[�Ɉړ�
	{F_WORDRIGHT,			LTEXT("WordRight"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P��̉E�[�Ɉړ�
	{F_GOLINETOP,			LTEXT("GoLineTop"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���Ɉړ�(�܂�Ԃ��P��)
	{F_GOLINEEND,			LTEXT("GoLineEnd"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�s���Ɉړ�(�܂�Ԃ��P��)
	{F_HalfPageUp,			LTEXT("HalfPageUp"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_HalfPageDown,		LTEXT("HalfPageDown"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_1PageUp,				LTEXT("PageUp"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	{F_1PageUp,				LTEXT("1PageUp"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	{F_1PageDown,			LTEXT("PageDown"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	{F_1PageDown,			LTEXT("1PageDown"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	{F_GOFILETOP,			LTEXT("GoFileTop"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�t�@�C���̐擪�Ɉړ�
	{F_GOFILEEND,			LTEXT("GoFileEnd"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�t�@�C���̍Ō�Ɉړ�
	{F_CURLINECENTER,		LTEXT("CurLineCenter"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�J�[�\���s���E�B���h�E������
	{F_JUMPHIST_PREV,		LTEXT("MoveHistPrev"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�ړ�����: �O��
	{F_JUMPHIST_NEXT,		LTEXT("MoveHistNext"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�ړ�����: ����
	{F_JUMPHIST_SET,		LTEXT("MoveHistSet"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���݈ʒu���ړ������ɓo�^
	{F_WndScrollDown,		LTEXT("F_WndScrollDown"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	{F_WndScrollUp,			LTEXT("F_WndScrollUp"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o
	{F_GONEXTPARAGRAPH,		LTEXT("GoNextParagraph"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̒i���ֈړ�
	{F_GOPREVPARAGRAPH,		LTEXT("GoPrevParagraph"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̒i���ֈړ�

	/* �I���n */	//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v�������Ȃ����̂Łu�I���n�v�Ƃ��ēƗ���(�T�u���j���[���͍\����ł��Ȃ��̂�)
	{F_SELECTWORD,			LTEXT("SelectWord"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���݈ʒu�̒P��I��
	{F_SELECTALL,			LTEXT("SelectAll"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ׂđI��
	{F_SELECTLINE,			LTEXT("SelectLine"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //1�s�I��	// 2007.10.13 nasukoji
	{F_BEGIN_SEL,			LTEXT("BeginSelect"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�͈͑I���J�n Mar. 5, 2001 genta ���̏C��
	{F_UP_SEL,				LTEXT("Up_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\����ړ�
	{F_DOWN_SEL,			LTEXT("Down_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\�����ړ�
	{F_LEFT_SEL,			LTEXT("Left_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\�����ړ�
	{F_RIGHT_SEL,			LTEXT("Right_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\���E�ړ�
	{F_UP2_SEL,				LTEXT("Up2_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\����ړ�(�Q�s����)
	{F_DOWN2_SEL,			LTEXT("Down2_Sel"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
	{F_WORDLEFT_SEL,		LTEXT("WordLeft_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P��̍��[�Ɉړ�
	{F_WORDRIGHT_SEL,		LTEXT("WordRight_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P��̉E�[�Ɉړ�
	{F_GOLINETOP_SEL,		LTEXT("GoLineTop_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	{F_GOLINEEND_SEL,		LTEXT("GoLineEnd_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	{F_HalfPageUp_Sel,		LTEXT("HalfPageUp_Sel"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_HalfPageDown_Sel,	LTEXT("HalfPageDown_Sel"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	{F_1PageUp_Sel,			LTEXT("PageUp_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	{F_1PageUp_Sel,			LTEXT("1PageUp_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	{F_1PageDown_Sel,		LTEXT("PageDown_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	{F_1PageDown_Sel,		LTEXT("1PageDown_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	{F_GOFILETOP_SEL,		LTEXT("GoFileTop_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�t�@�C���̐擪�Ɉړ�
	{F_GOFILEEND_SEL,		LTEXT("GoFileEnd_Sel"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //(�͈͑I��)�t�@�C���̍Ō�Ɉړ�
	{F_GONEXTPARAGRAPH_SEL,	LTEXT("GoNextParagraph_Sel"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̒i���ֈړ�
	{F_GOPREVPARAGRAPH_SEL,	LTEXT("GoPrevParagraph_Sel"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̒i���ֈړ�

	/* ��`�I���n */	//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
	{F_BEGIN_BOX,			LTEXT("BeginBoxSelect"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��`�͈͑I���J�n

	/* �N���b�v�{�[�h�n */
	{F_CUT,						LTEXT("Cut"),						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	{F_COPY,					LTEXT("Copy"),						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	{F_PASTE,					LTEXT("Paste"),						{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�\��t��(�N���b�v�{�[�h����\��t��)
	{F_COPY_ADDCRLF,			LTEXT("CopyAddCRLF"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[
	{F_COPY_CRLF,				LTEXT("CopyCRLF"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //CRLF���s�ŃR�s�[(�I��͈͂����s�R�[�h=CRLF�ŃR�s�[)
	{F_PASTEBOX,				LTEXT("PasteBox"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
	{F_INSTEXT_W,				LTEXT("InsText"),					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // �e�L�X�g��\��t��
	{F_ADDTAIL_W,				LTEXT("AddTail"),					{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // �Ō�Ƀe�L�X�g��ǉ�
	{F_COPYLINES,				LTEXT("CopyLines"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I��͈͓��S�s�R�s�[
	{F_COPYLINESASPASSAGE,		LTEXT("CopyLinesAsPassage"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I��͈͓��S�s���p���t���R�s�[
	{F_COPYLINESWITHLINENUMBER,	LTEXT("CopyLinesWithLineNumber"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�I��͈͓��S�s�s�ԍ��t���R�s�[
	{F_COPYPATH,				LTEXT("CopyPath"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	{F_COPYFNAME,				LTEXT("CopyFilename"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ // 2002/2/3 aroka
	{F_COPYTAG,					LTEXT("CopyTag"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[	//Sept. 15, 2000 jepro ��Ɠ��������ɂȂ��Ă����̂��C��
	{F_CREATEKEYBINDLIST,		LTEXT("CopyKeyBindList"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�L�[���蓖�Ĉꗗ���R�s�[	//Sept. 15, 2000 JEPRO �ǉ� //Dec. 25, 2000 ����

	/* �}���n */
	{F_INS_DATE,				LTEXT("InsertDate"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // ���t�}��
	{F_INS_TIME,				LTEXT("InsertTime"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // �����}��
	{F_CTRL_CODE_DIALOG,		LTEXT("CtrlCodeDialog"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�R���g���[���R�[�h�̓���(�_�C�A���O)	//@@@ 2002.06.02 MIK

	/* �ϊ��n */
	{F_TOLOWER,		 			LTEXT("ToLower"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //������
	{F_TOUPPER,		 			LTEXT("ToUpper"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�啶��
	{F_TOHANKAKU,		 		LTEXT("ToHankaku"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �S�p�����p */
	{F_TOHANKATA,		 		LTEXT("ToHankata"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �S�p�J�^�J�i�����p�J�^�J�i */	//Aug. 29, 2002 ai
	{F_TOZENEI,		 			LTEXT("ToZenEi"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	{F_TOHANEI,		 			LTEXT("ToHanEi"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �S�p�p�������p�p�� */
	{F_TOZENKAKUKATA,	 		LTEXT("ToZenKata"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	{F_TOZENKAKUHIRA,	 		LTEXT("ToZenHira"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	{F_HANKATATOZENKATA,	LTEXT("HanKataToZenKata"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�J�^�J�i���S�p�J�^�J�i */
	{F_HANKATATOZENHIRA,	LTEXT("HanKataToZenHira"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���p�J�^�J�i���S�p�Ђ炪�� */
	{F_TABTOSPACE,				LTEXT("TABToSPACE"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* TAB���� */
	{F_SPACETOTAB,				LTEXT("SPACEToTAB"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �󔒁�TAB */ //---- Stonee, 2001/05/27
	{F_CODECNV_AUTO2SJIS,		LTEXT("AutoToSJIS"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �������ʁ�SJIS�R�[�h�ϊ� */
	{F_CODECNV_EMAIL,			LTEXT("JIStoSJIS"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //E-Mail(JIS��SJIS)�R�[�h�ϊ�
	{F_CODECNV_EUC2SJIS,		LTEXT("EUCtoSJIS"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //EUC��SJIS�R�[�h�ϊ�
	{F_CODECNV_UNICODE2SJIS,	LTEXT("CodeCnvUNICODEtoSJIS"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Unicode��SJIS�R�[�h�ϊ�
	{F_CODECNV_UNICODEBE2SJIS,	LTEXT("CodeCnvUNICODEBEtoSJIS"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // UnicodeBE��SJIS�R�[�h�ϊ�
	{F_CODECNV_UTF82SJIS,		LTEXT("UTF8toSJIS"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* UTF-8��SJIS�R�[�h�ϊ� */
	{F_CODECNV_UTF72SJIS,		LTEXT("UTF7toSJIS"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* UTF-7��SJIS�R�[�h�ϊ� */
	{F_CODECNV_SJIS2JIS,		LTEXT("SJIStoJIS"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS��JIS�R�[�h�ϊ� */
	{F_CODECNV_SJIS2EUC,		LTEXT("SJIStoEUC"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS��EUC�R�[�h�ϊ� */
	{F_CODECNV_SJIS2UTF8,		LTEXT("SJIStoUTF8"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS��UTF-8�R�[�h�ϊ� */
	{F_CODECNV_SJIS2UTF7,		LTEXT("SJIStoUTF7"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* SJIS��UTF-7�R�[�h�ϊ� */
	{F_BASE64DECODE,	 		LTEXT("Base64Decode"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //Base64�f�R�[�h���ĕۑ�
	{F_UUDECODE,		 		LTEXT("Uudecode"),					{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //uudecode���ĕۑ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX


	/* �����n */
	{F_SEARCH_DIALOG,			LTEXT("SearchDialog"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //����(�P�ꌟ���_�C�A���O)
	{F_SEARCH_NEXT,				LTEXT("SearchNext"),		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��������
	{F_SEARCH_PREV,				LTEXT("SearchPrev"),		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O������
	{F_REPLACE_DIALOG,			LTEXT("ReplaceDialog"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u��(�u���_�C�A���O)
	{F_REPLACE,					LTEXT("Replace"),			{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //�u��(���s)
	{F_REPLACE_ALL,				LTEXT("ReplaceAll"),		{VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY},	VT_EMPTY,	NULL}, //���ׂĒu��(���s)
	{F_SEARCH_CLEARMARK,		LTEXT("SearchClearMark"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�����}�[�N�̃N���A
	{F_JUMP_SRCHSTARTPOS,		LTEXT("SearchStartPos"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�����J�n�ʒu�֖߂�			// 02/06/26 ai
	{F_GREP,					LTEXT("Grep"),				{VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_I4   },	VT_EMPTY,	NULL}, //Grep
	{F_JUMP,					LTEXT("Jump"),				{VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�w��s�w�W�����v
	{F_OUTLINE,					LTEXT("Outline"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�A�E�g���C�����
	{F_TAGJUMP,					LTEXT("TagJump"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�^�O�W�����v�@�\
	{F_TAGJUMPBACK,				LTEXT("TagJumpBack"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�^�O�W�����v�o�b�N�@�\
	{F_TAGS_MAKE,				LTEXT("TagMake"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
	{F_DIRECT_TAGJUMP,			LTEXT("DirectTagJump"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�_�C���N�g�^�O�W�����v�@�\	//@@@ 2003.04.15 MIK
	{F_TAGJUMP_KEYWORD,			LTEXT("KeywordTagJump"),	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�L�[���[�h���w�肵�ă_�C���N�g�^�O�W�����v�@�\ //@@@ 2005.03.31 MIK
	{F_COMPARE,					LTEXT("Compare"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�t�@�C�����e��r
	{F_DIFF_DIALOG,				LTEXT("DiffDialog"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��(�_�C�A���O)	//@@@ 2002.05.25 MIK
	{F_DIFF,					LTEXT("Diff"),				{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��				//@@@ 2002.05.25 MIK	// 2005.10.03 maru
	{F_DIFF_NEXT,				LTEXT("DiffNext"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��(����)			//@@@ 2002.05.25 MIK
	{F_DIFF_PREV,				LTEXT("DiffPrev"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��(�O��)			//@@@ 2002.05.25 MIK
	{F_DIFF_RESET,				LTEXT("DiffReset"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //DIFF�����\��(�S����)		//@@@ 2002.05.25 MIK
	{F_BRACKETPAIR,				LTEXT("BracketPair"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�Ί��ʂ̌���
// From Here 2001.12.03 hor
	{F_BOOKMARK_SET,			LTEXT("BookmarkSet"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u�b�N�}�[�N�ݒ�E����
	{F_BOOKMARK_NEXT,			LTEXT("BookmarkNext"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃u�b�N�}�[�N��
	{F_BOOKMARK_PREV,			LTEXT("BookmarkPrev"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̃u�b�N�}�[�N��
	{F_BOOKMARK_RESET,			LTEXT("BookmarkReset"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u�b�N�}�[�N�̑S����
	{F_BOOKMARK_VIEW,			LTEXT("BookmarkView"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�u�b�N�}�[�N�̈ꗗ
// To Here 2001.12.03 hor
	{F_BOOKMARK_PATTERN,		LTEXT("BookmarkPattern"),	{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, // 2002.01.16 hor �w��p�^�[���Ɉ�v����s���}�[�N

	/* ���[�h�؂�ւ��n */
	{F_CHGMOD_INS,				LTEXT("ChgmodINS"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�}���^�㏑�����[�h�؂�ւ�
	{F_CHGMOD_EOL,				LTEXT("ChgmodEOL"),			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���͉��s�R�[�h�w�� 2003.06.23 Moca
	{F_CANCEL_MODE,				LTEXT("CancelMode"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�e�탂�[�h�̎�����

	/* �}�N���n */
	{F_EXECEXTMACRO,			LTEXT("ExecExternalMacro"),	{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���O���w�肵�ă}�N�����s

	/* �ݒ�n */
	{F_SHOWTOOLBAR,				LTEXT("ShowToolbar"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �c�[���o�[�̕\�� */
	{F_SHOWFUNCKEY,				LTEXT("ShowFunckey"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �t�@���N�V�����L�[�̕\�� */
	{F_SHOWTAB,					LTEXT("ShowTab"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �^�u�̕\�� */	//@@@ 2003.06.10 MIK
	{F_SHOWSTATUSBAR,			LTEXT("ShowStatusbar"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �X�e�[�^�X�o�[�̕\�� */
	{F_TYPE_LIST,				LTEXT("TypeList"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �^�C�v�ʐݒ�ꗗ */
	{F_OPTION_TYPE,				LTEXT("OptionType"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �^�C�v�ʐݒ� */
	{F_OPTION,					LTEXT("OptionCommon"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���ʐݒ� */
	{F_FONT,					LTEXT("SelectFont"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �t�H���g�ݒ� */
	{F_WRAPWINDOWWIDTH,			LTEXT("WrapWindowWidth"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
	{F_FAVORITE,				LTEXT("OptionFavorite"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �����̊Ǘ� */	//@@@ 2003.04.08 MIK
	{F_SET_QUOTESTRING,			LTEXT("SetMsgQuoteStr"),	{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���ʐݒ聨���������p���̐ݒ� */	//Jan. 29, 2005 genta
	{F_TEXTWRAPMETHOD,			LTEXT("TextWrapMethod"),	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �e�L�X�g�̐܂�Ԃ����@ */	// 2008.05.30 nasukoji
	{F_SELECT_COUNT_MODE,		LTEXT("SelectCountMode"),	{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �e�L�X�g�̐܂�Ԃ����@ */	// 2008.05.30 nasukoji


	//	Oct. 9, 2001 genta �ǉ�
	{F_EXECMD,				LTEXT("ExecCommand"),		{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �O���R�}���h���s */

	/* �J�X�^�����j���[ */
	{F_MENU_RBUTTON,			LTEXT("RMenu"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �E�N���b�N���j���[ */
	{F_CUSTMENU_1,				LTEXT("CustMenu1"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[1 */
	{F_CUSTMENU_2,				LTEXT("CustMenu2"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[2 */
	{F_CUSTMENU_3,				LTEXT("CustMenu3"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[3 */
	{F_CUSTMENU_4,				LTEXT("CustMenu4"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[4 */
	{F_CUSTMENU_5,				LTEXT("CustMenu5"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[5 */
	{F_CUSTMENU_6,				LTEXT("CustMenu6"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[6 */
	{F_CUSTMENU_7,				LTEXT("CustMenu7"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[7 */
	{F_CUSTMENU_8,				LTEXT("CustMenu8"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[8 */
	{F_CUSTMENU_9,				LTEXT("CustMenu9"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[9 */
	{F_CUSTMENU_10,				LTEXT("CustMenu10"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[10 */
	{F_CUSTMENU_11,				LTEXT("CustMenu11"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[11 */
	{F_CUSTMENU_12,				LTEXT("CustMenu12"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[12 */
	{F_CUSTMENU_13,				LTEXT("CustMenu13"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[13 */
	{F_CUSTMENU_14,				LTEXT("CustMenu14"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[14 */
	{F_CUSTMENU_15,				LTEXT("CustMenu15"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[15 */
	{F_CUSTMENU_16,				LTEXT("CustMenu16"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[16 */
	{F_CUSTMENU_17,				LTEXT("CustMenu17"), 		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[17 */
	{F_CUSTMENU_18,				LTEXT("CustMenu18"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[18 */
	{F_CUSTMENU_19,				LTEXT("CustMenu19"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[19 */
	{F_CUSTMENU_20,				LTEXT("CustMenu20"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[20 */
	{F_CUSTMENU_21,				LTEXT("CustMenu21"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[21 */
	{F_CUSTMENU_22,				LTEXT("CustMenu22"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[22 */
	{F_CUSTMENU_23,				LTEXT("CustMenu23"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[23 */
	{F_CUSTMENU_24,				LTEXT("CustMenu24"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �J�X�^�����j���[24 */

	/* �E�B���h�E�n */
	{F_SPLIT_V,					LTEXT("SplitWinV"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�㉺�ɕ���	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	{F_SPLIT_H,					LTEXT("SplitWinH"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���E�ɕ���	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	{F_SPLIT_VH,				LTEXT("SplitWinVH"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	{F_WINCLOSE,				LTEXT("WinClose"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�E�B���h�E�����
	{F_WIN_CLOSEALL,			LTEXT("WinCloseAll"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
	{F_CASCADE,					LTEXT("CascadeWin"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�d�˂ĕ\��
	{F_TILE_V,					LTEXT("TileWinV"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�㉺�ɕ��ׂĕ\��
	{F_TILE_H,					LTEXT("TileWinH"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���E�ɕ��ׂĕ\��
	{F_NEXTWINDOW,				LTEXT("NextWindow"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃E�B���h�E
	{F_PREVWINDOW,				LTEXT("PrevWindow"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̃E�B���h�E
	{F_WINLIST,					LTEXT("WindowList"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�E�B���h�E�ꗗ�|�b�v�A�b�v�\��	// 2006.03.23 fon
	{F_MAXIMIZE_V,				LTEXT("MaximizeV"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�c�����ɍő剻
	{F_MAXIMIZE_H,				LTEXT("MaximizeH"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�������ɍő剻 //2001.02.10 by MIK
	{F_MINIMIZE_ALL,			LTEXT("MinimizeAll"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���ׂčŏ���	//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	{F_REDRAW,					LTEXT("ReDraw"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�ĕ`��
	{F_WIN_OUTPUT,				LTEXT("ActivateWinOutput"),	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�A�E�g�v�b�g�E�B���h�E�\��
	{F_TRACEOUT,				LTEXT("TraceOut"),			{VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�}�N���p�A�E�g�v�b�g�E�B���h�E�ɏo��	2006.04.26 maru
	{F_TOPMOST,					LTEXT("WindowTopMost"),		{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //��Ɏ�O�ɕ\��
	{F_GROUPCLOSE,				LTEXT("GroupClose"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O���[�v�����	// 2007.06.20 ryoji
	{F_NEXTGROUP,				LTEXT("NextGroup"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃O���[�v	// 2007.06.20 ryoji
	{F_PREVGROUP,				LTEXT("PrevGroup"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̃O���[�v	// 2007.06.20 ryoji
	{F_TAB_MOVERIGHT,			LTEXT("TabMoveRight"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�^�u���E�Ɉړ�	// 2007.06.20 ryoji
	{F_TAB_MOVELEFT,			LTEXT("TabMoveLeft"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�^�u�����Ɉړ�	// 2007.06.20 ryoji
	{F_TAB_SEPARATE,			LTEXT("TabSeparate"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�V�K�O���[�v	// 2007.06.20 ryoji
	{F_TAB_JOINTNEXT,			LTEXT("TabJointNext"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃O���[�v�Ɉړ�	// 2007.06.20 ryoji
	{F_TAB_JOINTPREV,			LTEXT("TabJointPrev"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�O�̃O���[�v�Ɉړ�	// 2007.06.20 ryoji
	{F_TAB_CLOSEOTHER,			LTEXT("TabCloseOther"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //���̃^�u�ȊO�����	// 2010/3/14 Uchi
	{F_TAB_CLOSELEFT,			LTEXT("TabCloseLeft"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�������ׂĕ���		// 2010/3/14 Uchi
	{F_TAB_CLOSERIGHT,			LTEXT("TabCloseRight"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, //�E�����ׂĕ���		// 2010/3/14 Uchi

	/* �x�� */
	{F_HOKAN,					LTEXT("Complete"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* ���͕⊮ */	//Oct. 15, 2000 JEPRO �����ĂȂ������̂ŉp����t���ē���Ă݂�
	{F_HELP_CONTENTS,			LTEXT("HelpContents"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO �ǉ�
	{F_HELP_SEARCH,				LTEXT("HelpSearch"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO �ǉ�
	{F_MENU_ALLFUNC,			LTEXT("CommandList"),		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �R�}���h�ꗗ */
	{F_EXTHELP1,				LTEXT("ExtHelp1"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �O���w���v�P */
	//	Jul. 5, 2002 genta �����ǉ�
	{F_EXTHTMLHELP,				LTEXT("ExtHtmlHelp"),		{VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �O��HTML�w���v */
	{F_ABOUT,					LTEXT("About"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}, /* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�

	//	�I�[
	//	Jun. 27, 2002 genta
	//	�I�[�Ƃ��Ă͌����Č���Ȃ����̂��g���ׂ��Ȃ̂ŁC
	//	FuncID��-1�ɕύX�D(0�͎g����)
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};
//int	CSMacroMgr::m_nMacroFuncInfoArrNum = _countof( CSMacroMgr::m_MacroFuncInfoArr );

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
	m_pTempMacro = NULL;

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
	delete m_pTempMacro;
	m_pTempMacro = NULL;
}

/*! @brief�L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ�

	@param mbuf [in] �ǂݍ��ݐ�}�N���o�b�t�@
	
	@date 2002.06.16 genta �L�[�}�N���̑���Ή��̂��ߕύX
*/
int CSMacroMgr::Append(
	int				idx,		//!<
	EFunctionCode	nFuncID,	//!< [in] �@�\�ԍ�
	LPARAM			lParam1,	//!< [in] �p�����[�^�B
	CEditView*		pcEditView	//!< 
)
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
	return TRUE;
}


/*!	@brief �L�[�{�[�h�}�N���̎��s

	CShareData����t�@�C�������擾���A���s����B

	@param hInstance [in] �C���X�^���X
	@param hwndParent [in] �e�E�B���h�E��
	@param pViewClass [in] macro���s�Ώۂ�View
	@param idx [in] �}�N���ԍ��B
	@param flags [in] �}�N�����s�t���O�DHandleCommand�ɓn���I�v�V�����D

	@date 2007.07.16 genta flags�ǉ�
*/
BOOL CSMacroMgr::Exec( int idx , HINSTANCE hInstance, CEditView* pcEditView, int flags )
{
	if( idx == STAND_KEYMACRO ){
		//	Jun. 16, 2002 genta
		//	�L�[�}�N���ȊO�̃T�|�[�g�ɂ��NULL�̉\�����o�Ă����̂Ŕ���ǉ�
		if( m_pKeyMacro != NULL ){
			//	Sep. 15, 2005 FILE
			//	Jul. 01, 2007 �}�N���̑��d���s���ɔ����Ē��O�̃}�N���ԍ���ޔ�
			int prevmacro = SetCurrentIdx( idx );
			m_pKeyMacro->ExecKeyMacro( pcEditView, flags );
			SetCurrentIdx( prevmacro );
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
	if( idx == TEMP_KEYMACRO ){		// �ꎞ�}�N��
		if( m_pTempMacro != NULL ){
			SetCurrentIdx( idx );
			m_pTempMacro->ExecKeyMacro( pcEditView, flags );
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
		TCHAR ptr[_MAX_PATH * 2];
		int n = CShareData::getInstance()->GetMacroFilename( idx, ptr, _countof(ptr) );
		if ( n <= 0 ){
			return FALSE;
		}

		if( !Load( idx, hInstance, ptr, NULL ) )
			return FALSE;
	}

	//	Sep. 15, 2005 FILE
	//	Jul. 01, 2007 �}�N���̑��d���s���ɔ����Ē��O�̃}�N���ԍ���ޔ�
	int prevmacro = SetCurrentIdx( idx );
	SetCurrentIdx( idx );
	m_cSavedKeyMacro[idx]->ExecKeyMacro(pcEditView, flags);
	SetCurrentIdx( prevmacro );

	pcEditView->Redraw();	//	�K�v�H
	return TRUE;
}

/*! �L�[�{�[�h�}�N���̓ǂݍ���

	@param idx [in] �ǂݍ��ݐ�}�N���o�b�t�@�ԍ�
	@param pszPath [in] �}�N���t�@�C�����A�܂��̓R�[�h������
	@param pszType [in] ��ʁBNULL�̏ꍇ�t�@�C������ǂݍ��ށBNULL�ȊO�̏ꍇ�͌���̊g���q

	�ǂݍ��݂Ɏ��s�����Ƃ��̓}�N���o�b�t�@�̃I�u�W�F�N�g�͉������C
	NULL���ݒ肳���D

	@author Norio Nakatani, YAZAKI, genta
*/
BOOL CSMacroMgr::Load( int idx, HINSTANCE hInstance, const TCHAR* pszPath, const TCHAR* pszType )
{
	CMacroManagerBase** ppMacro = Idx2Ptr( idx );

	if( ppMacro == NULL ){
#ifdef _DEBUG
	MYTRACE_A( "CSMacroMgr::Load() Out of range: idx=%d Path=%ts\n", idx, pszPath);
#endif
	}
	//	�o�b�t�@�N���A
	delete *ppMacro;
	*ppMacro = NULL;
	
	const TCHAR *ext;
	if( pszType == NULL ){				//�t�@�C���w��
		//�t�@�C���̊g���q���擾����
		ext = _tcsrchr( pszPath, _T('.'));
		//	Feb. 02, 2004 genta .�������ꍇ��ext==NULL�ƂȂ�̂�NULL�`�F�b�N�ǉ�
		if( ext != NULL ){
			const TCHAR *chk = _tcsrchr( ext, _T('\\') );
			if( chk != NULL ){	//	.�̂��Ƃ�\���������炻��͊g���q�̋�؂�ł͂Ȃ�
								//	\��������2�o�C�g�ڂ̏ꍇ���g���q�ł͂Ȃ��B
				ext = NULL;
			}
		}
		if(ext != NULL){
			++ext;
		}
	}else{								//�R�[�h�w��
		ext = pszType;
	}

	*ppMacro = CMacroFactory::Instance()->Create(ext);
	if( *ppMacro == NULL )
		return FALSE;
	BOOL bRet;
	if( pszType == NULL ){
		bRet = (*ppMacro)->LoadKeyMacro(hInstance, pszPath);
	}else{
		bRet = (*ppMacro)->LoadKeyMacroStr(hInstance, pszPath);
	}

	//	From Here Jun. 16, 2002 genta
	//	�ǂݍ��݃G���[���̓C���X�^���X�폜
	if( bRet ){
		return TRUE;
	}
	else {
		delete *ppMacro;
		*ppMacro = NULL;
	}
	//	To Here Jun. 16, 2002 genta
	return FALSE;
}

/** �}�N���I�u�W�F�N�g�����ׂĔj������(�L�[�{�[�h�}�N���ȊO)

	�}�N���̓o�^��ύX�����ꍇ�ɁC�ύX�O�̃}�N����
	�����������s����Ă��܂��̂�h���D

	@date 2007.10.19 genta �V�K�쐬
*/
void CSMacroMgr::UnloadAll(void)
{
	for ( int idx = 0; idx < MAX_CUSTMACRO; idx++ ){
		delete m_cSavedKeyMacro[idx];
		m_cSavedKeyMacro[idx] = NULL;
	}
	
}

/*! �L�[�{�[�h�}�N���̕ۑ�

	@param idx [in] �ǂݍ��ݐ�}�N���o�b�t�@�ԍ�
	@param pszPath [in] �}�N���t�@�C����
	@param hInstance [in] �C���X�^���X�n���h��

	@author YAZAKI
*/
BOOL CSMacroMgr::Save( int idx, HINSTANCE hInstance, const TCHAR* pszPath )
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
	
	@return ���������Ƃ���pszFuncName�D������Ȃ������Ƃ���NULL�D
	
	@note
	���ꂼ��C������i�[�̈�̎w���悪NULL�̎��͕�������i�[���Ȃ��D
	�������CpszFuncName��NULL�ɂ��Ă��܂��Ɩ߂�l�����NULL�ɂȂ���
	�������肪�s���Ȃ��Ȃ�D
	
	@date 2002.06.16 genta �V�݂�GetFuncInfoById(int)������Ŏg���悤�ɁD
*/
WCHAR* CSMacroMgr::GetFuncInfoByID(
	HINSTANCE	hInstance,			//!< [in] ���\�[�X�擾�̂��߂�Instance Handle
	int			nFuncID,			//!< [in] �@�\�ԍ�
	WCHAR*		pszFuncName,		//!< [out] �֐����D���̐�ɂ͍Œ��֐����{1�o�C�g�̃��������K�v�D
	WCHAR*		pszFuncNameJapanese	//!< [out] �@�\�����{��DNULL���e. ���̐�ɂ�256�o�C�g�̃��������K�v�D
)
{
	const MacroFuncInfo* MacroInfo = GetFuncInfoByID( nFuncID );
	if( MacroInfo != NULL ){
		if( pszFuncName != NULL ){
			auto_strcpy( pszFuncName, MacroInfo->m_pszFuncName );
			WCHAR *p = pszFuncName;
			while (*p){
				if (*p == LTEXT('(')){
					*p = LTEXT('\0');
					break;
				}
				*p++;
			}
		}
		//	Jun. 16, 2002 genta NULL�̂Ƃ��͉������Ȃ��D
		if( pszFuncNameJapanese != NULL ){
			ApiWrap::LoadStringW_AnyBuild( hInstance, nFuncID, pszFuncNameJapanese, 255 );
		}
		return pszFuncName;
	}
	return NULL;
}

/*!
	�֐����iS_xxxx�j����@�\�ԍ��Ƌ@�\�����{����擾�D
	�֐�����S_�Ŏn�܂�ꍇ�Ǝn�܂�Ȃ��ꍇ�̗����ɑΉ��D

	@return ���������Ƃ��͋@�\�ԍ��D������Ȃ������Ƃ���-1�D
	
	@note
	pszFuncNameJapanese �̎w���悪NULL�̎��͓��{�ꖼ���i�[���Ȃ��D
	
	@date 2002.06.16 genta ���[�v���̕�����R�s�[��r��
*/
EFunctionCode CSMacroMgr::GetFuncInfoByName(
	HINSTANCE		hInstance,				//!< [in]  ���\�[�X�擾�̂��߂�Instance Handle
	const WCHAR*	pszFuncName,			//!< [in]  �֐���
	WCHAR*			pszFuncNameJapanese		//!< [out] �@�\�����{��D���̐�ɂ�256�o�C�g�̃��������K�v�D
)
{
	//	Jun. 16, 2002 genta
	const WCHAR* normalizedFuncName;
	
	//	S_�Ŏn�܂��Ă��邩
	if( pszFuncName == NULL ){
		return F_INVALID;
	}
	if( pszFuncName[0] == LTEXT('S') && pszFuncName[1] == LTEXT('_') ){
		normalizedFuncName = pszFuncName + 2;
	}
	else {
		normalizedFuncName = pszFuncName;
	}

	// �R�}���h�֐�������
	for( int i = 0; m_MacroFuncInfoArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == auto_strcmp( normalizedFuncName, m_MacroFuncInfoArr[i].m_pszFuncName )){
			EFunctionCode nFuncID = m_MacroFuncInfoArr[i].m_nFuncID;
			if( pszFuncNameJapanese != NULL ){
				::LoadStringW_AnyBuild( hInstance, nFuncID, pszFuncNameJapanese, 255 );
			}
			return nFuncID;
		}
	}
	// ��R�}���h�֐�������
	for( int i = 0; m_MacroFuncInfoNotCommandArr[i].m_pszFuncName != NULL; ++i ){
		if( 0 == auto_strcmp( normalizedFuncName, m_MacroFuncInfoNotCommandArr[i].m_pszFuncName )){
			EFunctionCode nFuncID = m_MacroFuncInfoNotCommandArr[i].m_nFuncID;
			if( pszFuncNameJapanese != NULL ){
				::LoadStringW_AnyBuild( hInstance, nFuncID, pszFuncNameJapanese, 255 );
			}
			return nFuncID;
		}
	}
	return F_INVALID;
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
	case F_FILE_REOPEN_CESU8		://CESU-8�ŊJ������	// 2010/3/20 Uchi
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
//	case F_PUTFILE					://��ƒ��t�@�C���̈ꎞ�o��	2006.12.10 maru
//	case F_INSFILE					://�L�����b�g�ʒu�Ƀt�@�C���}��	2006.12.10 maru


	/* �ҏW�n */
	case F_WCHAR					://��������
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
	case F_CURLINECENTER			://�J�[�\���s���E�B���h�E������
	case F_JUMPHIST_PREV			://�ړ�����: �O��
	case F_JUMPHIST_NEXT			://�ړ�����: ����
	case F_JUMPHIST_SET				://���݈ʒu���ړ������ɓo�^

	/* �I���n */	//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v�������Ȃ����̂œƗ�������(�I��)���ړ�(�T�u���j���[���͍\����ł��Ȃ��̂�)
	case F_SELECTWORD				://���݈ʒu�̒P��I��
	case F_SELECTALL				://���ׂđI��
	case F_SELECTLINE				://1�s�I��	// 2007.10.06 nasukoji
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
	case F_INSTEXT_W					://�e�L�X�g��\��t��
//	case F_ADDTAIL_W					://�Ō�Ƀe�L�X�g��ǉ�
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
	case F_HANKATATOZENKATA		:/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENHIRA		:/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_TABTOSPACE				:/* TAB���� */
	case F_SPACETOTAB				:/* �󔒁�TAB */  //---- Stonee, 2001/05/27
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

	/* �}�N���n */
//	case F_RECKEYMACRO				://�L�[�}�N���̋L�^�J�n�^�I��
//	case F_SAVEKEYMACRO				://�L�[�}�N���̕ۑ�
//	case F_LOADKEYMACRO				://�L�[�}�N���̓ǂݍ���
//	case F_EXECKEYMACRO				://�L�[�}�N���̎��s
	case F_EXECEXTMACRO				://���O���w�肵�ă}�N�����s

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
//	case F_FAVORITE					:/* �����̊Ǘ� */	//@@@ 2003.04.08 MIK
//	case F_TMPWRAPNOWRAP			:// �܂�Ԃ��Ȃ��i�ꎞ�ݒ�j		// 2008.05.30 nasukoji
//	case F_TMPWRAPSETTING			:// �w�茅�Ő܂�Ԃ��i�ꎞ�ݒ�j	// 2008.05.30 nasukoji
//	case F_TMPWRAPWINDOW			:// �E�[�Ő܂�Ԃ��i�ꎞ�ݒ�j		// 2008.05.30 nasukoji
	case F_TEXTWRAPMETHOD			:// �e�L�X�g�̐܂�Ԃ����@			// 2008.05.30 nasukoji
	case F_SELECT_COUNT_MODE		:// �����J�E���g�̕��@���擾�A�ݒ�	// 2009.07.06 syat

	case F_EXECMD				:/* �O���R�}���h���s */	//@@@2002.2.2 YAZAKI �ǉ�

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
//	case F_GROUPCLOSE				://�O���[�v�����	// 2007.06.20 ryoji
//	case F_NEXTGROUP				://���̃O���[�v	// 2007.06.20 ryoji
//	case F_PREVGROUP				://�O�̃O���[�v	// 2007.06.20 ryoji
//	case F_TAB_MOVERIGHT			://�^�u���E�Ɉړ�	// 2007.06.20 ryoji
//	case F_TAB_MOVELEFT				://�^�u�����Ɉړ�	// 2007.06.20 ryoji
//	case F_TAB_SEPARATE				://�V�K�O���[�v	// 2007.06.20 ryoji
//	case F_TAB_JOINTNEXT			://���̃O���[�v�Ɉړ�	// 2007.06.20 ryoji
//	case F_TAB_JOINTPREV			://�O�̃O���[�v�Ɉړ�	// 2007.06.20 ryoji

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
	
	@param idx [in] �}�N���ԍ�(0-), STAND_KEYMACRO�͕W���L�[�}�N���o�b�t�@�ATEMP_KEYMACRO�͈ꎞ�}�N���o�b�t�@��\���D
	@return �I�u�W�F�N�g�ʒu�ւ̃|�C���^�D�}�N���ԍ����s���ȏꍇ��NULL�D
*/
CMacroManagerBase** CSMacroMgr::Idx2Ptr(int idx)
{
	//	Jun. 16, 2002 genta
	//	�L�[�}�N���ȊO�̃}�N����ǂݍ��߂�悤��
	if ( idx == STAND_KEYMACRO ){
		return &m_pKeyMacro;
	}
	else if ( idx == TEMP_KEYMACRO ){
		return &m_pTempMacro;
	}
	else if ( 0 <= idx && idx < MAX_CUSTMACRO ){
		return &m_cSavedKeyMacro[idx];
	}

#ifdef _DEBUG
	MYTRACE_A( "CSMacroMgr::Idx2Ptr() Out of range: idx=%d\n", idx);
#endif

	return NULL;
}

/*!
	�L�[�{�[�h�}�N���̕ۑ����\���ǂ���
	
	@retval TRUE �ۑ��\
	@retval FALSE �ۑ��s��
*/
bool CSMacroMgr::IsSaveOk(void)
{
	return dynamic_cast<CKeyMacroMgr*>( m_pKeyMacro ) == NULL ? false : true;
}

/*!
	�ꎞ�}�N������������
	
	@param newMacro [in] �V�����}�N���o�b�t�@�̃|�C���^�D
	@return �O�̈ꎞ�}�N���o�b�t�@�̃|�C���^�D
*/
CMacroManagerBase* CSMacroMgr::SetTempMacro( CMacroManagerBase *newMacro )
{
	CMacroManagerBase *oldMacro = m_pTempMacro;

	m_pTempMacro = newMacro;

	return oldMacro;
}
