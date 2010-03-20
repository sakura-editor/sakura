/*!	@file
	@brief ���j���[�Ǘ����\��

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, Jepro
	Copyright (C) 2001, jepro, MIK, Misaka, YAZAKI, hor, genta
	Copyright (C) 2002, MIK, genta, YAZAKI, ai, Moca, hor, aroka
	Copyright (C) 2003, MIK, genta, Moca
	Copyright (C) 2004, Kazika, genta, Moca, isearch
	Copyright (C) 2005, genta, MIK, aroka
	Copyright (C) 2006, aroka, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CMenuDrawer.h"
#include "sakura_rc.h"
#include "global.h"
#include "debug/Debug.h"
#include "window/CSplitBoxWnd.h"
#include "window/CEditWnd.h"
#include "CImageListMgr.h"


void FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr)
{
//	ASSERT_VALID(this);
//	ASSERT(m_hDC != NULL);

	RECT rect;
	::SetBkColor( hdc, clr );
	::SetRect( &rect, x, y, x + cx, y + cy );
	::ExtTextOutW_AnyBuild( hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL );
}


//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CMenuDrawer::CMenuDrawer()
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_nMenuItemNum = 0;
	m_nMenuHeight = 0;
	m_hFontMenu = NULL;
	m_hFontMenuUndelLine = NULL;

//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B	/* �c�[���o�[�̃{�^�� TBBUTTON�\���� */
	/* �c�[���o�[�̃{�^�� TBBUTTON�\���� */
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
	m_tbMyButton.resize( 1 );
	SetTBBUTTONVal( &m_tbMyButton[0], -1, 0, 0, TBSTYLE_SEP, 0, 0 );	//�Z�p���[�^	// 2007.11.02 ryoji �A�C�R���̖���`��(-1)
#if 0
	2002/04/26 ���p�Ȕėp���͔r���B
	struct TBUTTONDATA {
		int			idCommand;
		BYTE		fsState;
		BYTE		fsStyle;
		DWORD		dwData;
		int			iString;
	};
#endif
//	�L�[���[�h�F�A�C�R������(�A�C�R���C���f�b�N�X)
//	Sept. 16, 2000 Jepro note: �A�C�R���o�^���j���[
//	�ȉ��̓o�^�̓c�[���o�[�����łȂ��A�C�R���������ׂẴ��j���[�ŗ��p����Ă���
//	�����̓r�b�g�}�b�v���\�[�X��IDB_MYTOOL�ɓo�^����Ă���A�C�R���̐擪����̏��Ԃ̂悤�ł���
//	�A�C�R���������Ɠo�^�ł���悤�ɉ�����16dotsx218=2048dots�Ɋg��
//	�c��15dots����16dots�ɂ��āu�v�����^�v�A�C�R����u�w���v1�v�́A���������Ă��镔����������15dots�܂ł����\������Ȃ��炵�����ʂȂ�
//	��
//	Sept. 17, 2000 �c16dot�ڂ�\���ł���悤�ɂ���
//	�C�������t�@�C���ɂ�JEPRO_16thdot�ƃR�����g���Ă���̂ł����Ԉ���Ă����炻����L�[���[�h�ɂ��Č������Ă�������(Sept. 28, 2000���� 6�ӏ��ύX)
//	IDB_MYTOOL��16dot�ڂɌ��₷���悤�ɉ�16dots�Â̋�؂�ɂȂ�ڈ��t����
//
//	Sept. 16, 2000 ���₷���悤�ɉ���20��(���邢��32��)�Âɔz�񂵂悤�Ƃ������z��\����ς��Ȃ���΂��܂��i�[�ł��Ȃ��̂�
//	�������������̂��挈(�����񂽎����C��ur3��13�ŉ���)
//
//	Sept. 16, 2000 JEPRO �ł��邾���n���ƂɏW�܂�悤�ɏ��Ԃ�啝�ɓ���ւ���  ����ɔ���CShareData.cpp�Őݒ肵�Ă��鏉���ݒ�l���ύX
//	Oct. 22, 2000 JEPRO �A�C�R���̃r�b�g�}�b�v���\�[�X��2�����z�u���\�ɂȂ������ߍ��{�I�ɔz�u�]������
//	�E�z�u�̊�{�́u�R�}���h�ꗗ�v�ɓ����Ă���@�\(�R�}���h)��	�Ȃ��u�R�}���h�ꗗ�v���̂́u���j���[�o�[�v�̏��ɂ����悻�������Ă���
//	�E�A�C�R���r�b�g�}�b�v�t�@�C���ɂ͉�32��X13�i���邪�L���ɂ��Ă���̂�11�i�܂�(12�i�ڂ͊�{�I�ɍ�Ɨp, 13�i�ڂ͎���i�ȂǕۊǗp)
//	�E���j���[�ɑ�����n����ъe�n�̒i�Ƃ̊֌W�͎��̒ʂ�(Oct. 22, 2000 ����)�F
//		�t�@�C��----- �t�@�C������n	(1�i��32��: 1-32)
//		�ҏW--------- �ҏW�n			(2�i��32��: 33-64)
//		�ړ�--------- �J�[�\���ړ��n	(3�i��32��: 65-96)
//		�I��--------- �I���n			(4�i��32��: 97-128)
//					+ ��`�I���n		(5�i��32��: 129-160) //(��. ��`�I���n�̂قƂ�ǂ͖�����)
//					+ �N���b�v�{�[�h�n	(6�i��24��: 161-184)
//			���}���n					(6�i�ڎc���8��: 185-192)
//		�ϊ�--------- �ϊ��n			(7�i��32��: 193-224)
//		����--------- �����n			(8�i��32��: 225-256)
//		�c�[��------- ���[�h�؂�ւ��n	(9�i��4��: 257-260)
//					+ �ݒ�n			(9�i�ڎ���16��: 261-276)
//					+ �}�N���n			(9�i�ڍŌ��12��: 277-288)
//					+ �O���}�N��		(12�i��12��: 353-372)
//					+ �J�X�^�����j���[	(10�i��32��: 289-320)
//		�E�B���h�E--- �E�B���h�E�n		(11�i��22��: 321-342)
//		�w���v------- �x��				(11�i�ڎc���10��: 343-352)
//			�����̑�					(12�i�ڎc���10��: 373-384)
//	��1.�u�}���n�v�̓��j���[�ł́u�ҏW�v�ɓ����Ă���
//	��2.�u���̑��v�̓��j���[�ɂ͓����Ă��Ȃ����̂����� (���݉����Ȃ��̂�12�i�ڂ�ݒ肵�ĂȂ�)
//	��3.�u�R�}���h�ꗗ�v�Ŋ����ďd�����Ă���Ă���R�}���h�͂��́u�{�Ɓv�̕��ɔz�u����
//	��4.�u�R�}���h�ꗗ�v�ɓ����ĂȂ��R�}���h���킩���Ă���͈͂ňʒu�\��ɂ��Ă�����
//  ��5. F_DISABLE �͖���`�p(�_�~�[�Ƃ��Ă��g��)
//	��6. ���[�U�[�p�Ɋm�ۂ��ꂽ�ꏊ�͓��ɂȂ��̂Ŋe�i�̋󂢂Ă�����̕����g���Ă��������B

	int /* TBUTTONDATA */ tbd[] = {
/* �t�@�C������n(1�i��32��: 1-32) */
/*  1 */		F_FILENEW					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�V�K�쐬
/*  2 */		F_FILEOPEN					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J��
/*  3 */		F_FILESAVE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�㏑���ۑ�
/*  4 */		F_FILESAVEAS_DIALOG			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���O��t���ĕۑ�	//Sept. 18, 2000 JEPRO �ǉ�
/*  5 */		F_FILECLOSE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
/*  6 */		F_FILECLOSE_OPEN			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���ĊJ��
/*  7 */		F_FILE_REOPEN_SJIS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJIS�ŊJ������
/*  8 */		F_FILE_REOPEN_JIS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//JIS�ŊJ������
/*  9 */		F_FILE_REOPEN_EUC			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//EUC�ŊJ������
/* 10 */		F_FILE_REOPEN_UNICODE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Unicode�ŊJ������
/* 11 */		F_FILE_REOPEN_UTF8			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UTF-8�ŊJ������
/* 12 */		F_FILE_REOPEN_UTF7			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UTF-7�ŊJ������
/* 13 */		F_PRINT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���
/* 14 */		F_PRINT_PREVIEW				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//����v���r���[
/* 15 */		F_PRINT_PAGESETUP			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//����y�[�W�ݒ�	//Sept. 21, 2000 JEPRO �ǉ�
/* 16 */		F_OPEN_HfromtoC				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
/* 17 */		F_DISABLE	/*F_OPEN_HHPP*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX		del 2008/6/23 Uchi
/* 18 */		F_DISABLE	/*F_OPEN_CCPP*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX	del 2008/6/23 Uchi
/* 19 */		F_ACTIVATE_SQLPLUS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Oracle SQL*Plus���A�N�e�B�u�\�� */	//Sept. 20, 2000 JEPRO �ǉ�
/* 20 */		F_PLSQL_COMPILE_ON_SQLPLUS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Oracle SQL*Plus�Ŏ��s */	//Sept. 17, 2000 jepro �����́u�R���p�C���v���u���s�v�ɓ���
/* 21 */		F_BROWSE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�u���E�Y
/* 22 */		F_PROPERTY_FILE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�t�@�C���̃v���p�e�B//Sept. 16, 2000 JEPRO mytool1.bmp�ɂ������u�t�@�C���̃v���p�e�B�v�A�C�R����IDB_MYTOOL�ɃR�s�[
/* 23 */		F_VIEWMODE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�r���[���[�h
/* 24 */		F_FILE_REOPEN_UNICODEBE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UnicodeBE�ŊJ������ // Moca, 2002/05/26 �ǉ�
/* 25 */		F_FILEOPEN_DROPDOWN			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J��(�h���b�v�_�E��)
/* 26 */		F_FILE_REOPEN				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J���Ȃ���
/* 27 */		F_EXITALL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�T�N���G�f�B�^�̑S�I��	//Dec. 27, 2000 JEPRO �ǉ�
/* 28 */		F_FILESAVECLOSE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�ۑ����ĕ��� Feb. 28, 2004 genta
/* 29 */		F_DISABLE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 30 */		F_FILESAVEALL				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�S�ď㏑���ۑ� Jan. 24, 2005 genta
/* 31 */		F_EXITALLEDITORS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�ҏW�̑S�I��	// 2007.02.13 ryoji �ǉ�
/* 32 */		F_FILE_REOPEN_CESU8			/* . TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,  //CESU-8�ŊJ���Ȃ���

/* �ҏW�n(2�i��32��: 32-64) */
/* 33 */		F_UNDO							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���ɖ߂�(Undo)
/* 34 */		F_REDO							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//��蒼��(Redo)
/* 35 */		F_DELETE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�폜
/* 36 */		F_DELETE_BACK					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�[�\���O���폜
/* 37 */		F_WordDeleteToStart				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�P��̍��[�܂ō폜
/* 38 */		F_WordDeleteToEnd				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�P��̉E�[�܂ō폜
/* 39 */		F_WordDelete					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�P��폜
/* 40 */		F_WordCut						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�P��؂���
/* 41 */		F_LineDeleteToStart				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�s���܂ō폜(���s�P��)
/* 42 */		F_LineDeleteToEnd				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�s���܂ō폜(���s�P��)
/* 43 */		F_LineCutToStart				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�s���܂Ő؂���(���s�P��)
/* 44 */		F_LineCutToEnd					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�s���܂Ő؂���(���s�P��)
/* 45 */		F_DELETE_LINE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�s�폜(�܂�Ԃ��P��)
/* 46 */		F_CUT_LINE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�s�؂���(���s�P��)
/* 47 */		F_DUPLICATELINE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�s�̓�d��(�܂�Ԃ��P��)
/* 48 */		F_INDENT_TAB					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//TAB�C���f���g
/* 49 */		F_UNINDENT_TAB					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�tTAB�C���f���g
/* 50 */		F_INDENT_SPACE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SPACE�C���f���g
/* 51 */		F_UNINDENT_SPACE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�tSPACE�C���f���g
/* 52 */		F_DISABLE/*F_WORDSREFERENCE*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�P�ꃊ�t�@�����X	//�A�C�R������
/* 53 */		F_LTRIM							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//LTRIM		// 2001.12.03 hor
/* 54 */		F_RTRIM							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//RTRIM		// 2001.12.03 hor
/* 55 */		F_SORT_ASC						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SORT_ASC	// 2001.12.06 hor
/* 56 */		F_SORT_DESC						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SORT_DES	// 2001.12.06 hor
/* 57 */		F_MERGE							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//MERGE		// 2001.12.06 hor
/* 58 */		F_RECONVERT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�ĕϊ�	// 2002.4.12 YAZAKI
/* 59 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 60 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 61 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 62 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 63 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 64 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[

/* �J�[�\���ړ��n(3�i��32��: 65-96) */
/* 65 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 66 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 67 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 68 */		F_UP							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�[�\����ړ�
/* 69 */		F_DOWN							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�[�\�����ړ�
/* 70 */		F_LEFT							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�[�\�����ړ�
/* 71 */		F_RIGHT							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�[�\���E�ړ�
/* 72 */		F_UP2							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�[�\����ړ�(�Q�s����)
/* 73 */		F_DOWN2							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�[�\�����ړ�(�Q�s����)
/* 74 */		F_WORDLEFT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�P��̍��[�Ɉړ�
/* 75 */		F_WORDRIGHT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�P��̉E�[�Ɉړ�
/* 76 */		F_GOLINETOP						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�s���Ɉړ�(�܂�Ԃ��P��)
/* 77 */		F_GOLINEEND						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�s���Ɉړ�(�܂�Ԃ��P��)
/* 78 */		F_HalfPageUp					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
/* 79 */		F_HalfPageDown					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
/* 80 */		F_1PageUp						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
/* 81 */		F_1PageDown						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
/* 82 */		F_DISABLE/*F_DISPLAYTOP*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//��ʂ̐擪�Ɉړ�(������)
/* 83 */		F_DISABLE/*F_DISPLAYEND*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//��ʂ̍Ō�Ɉړ�(������)
/* 84 */		F_GOFILETOP						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�t�@�C���̐擪�Ɉړ�
/* 85 */		F_GOFILEEND						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�t�@�C���̍Ō�Ɉړ�
/* 86 */		F_CURLINECENTER					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�[�\���s���E�B���h�E������
/* 87 */		F_JUMPHIST_PREV						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�ړ�����: �O��	//Sept. 28, 2000 JEPRO �ǉ�
/* 88 */		F_JUMPHIST_NEXT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�ړ�����: ����	//Sept. 28, 2000 JEPRO �ǉ�
/* 89 */		F_WndScrollDown					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�e�L�X�g���P�s���փX�N���[��	//Jun. 28, 2001 JEPRO �ǉ�
/* 90 */		F_WndScrollUp					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�e�L�X�g���P�s��փX�N���[��	//Jun. 28, 2001 JEPRO �ǉ�
/* 91 */		F_GONEXTPARAGRAPH				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̒i����
/* 92 */		F_GOPREVPARAGRAPH				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O�̒i����
/* 93 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 94 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 95 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 96 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[

/* �I���n(4�i��32��: 97-128) */
/* 97 */		F_SELECTWORD					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���݈ʒu�̒P��I��
/* 98 */		F_SELECTALL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���ׂđI��	//Sept. 21, 2000 JEPRO �ǉ�
/* 99 */		F_BEGIN_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�͈͑I���J�n
/* 100 */		F_UP_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�J�[�\����ړ�
/* 101 */		F_DOWN_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�J�[�\�����ړ�
/* 102 */		F_LEFT_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�J�[�\�����ړ�
/* 103 */		F_RIGHT_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�J�[�\���E�ړ�
/* 104 */		F_UP2_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�J�[�\����ړ�(�Q�s����)
/* 105 */		F_DOWN2_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
/* 106 */		F_WORDLEFT_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�P��̍��[�Ɉړ�
/* 107 */		F_WORDRIGHT_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�P��̉E�[�Ɉړ�
/* 108 */		F_GOLINETOP_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
/* 109 */		F_GOLINEEND_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
/* 110 */		F_HalfPageUp_Sel				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
/* 111 */		F_HalfPageDown_Sel				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
/* 112 */		F_1PageUp_Sel					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
/* 113 */		F_1PageDown_Sel					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
/* 114 */		F_DISABLE/*F_DISPLAYTOP_SEL*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)��ʂ̐擪�Ɉړ�(������)
/* 115 */		F_DISABLE/*F_DISPLAYEND_SEL*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)��ʂ̍Ō�Ɉړ�(������)
/* 116 */		F_GOFILETOP_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�t�@�C���̐擪�Ɉړ�
/* 117 */		F_GOFILEEND_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�t�@�C���̍Ō�Ɉړ�
/* 118 */		F_GONEXTPARAGRAPH_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)���̒i����
/* 119 */		F_GOPREVPARAGRAPH_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(�͈͑I��)�O�̒i����
/* 120 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 121 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 122 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 123 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 124 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 125 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 126 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 127 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 128 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[

/* ��`�I���n(5�i��32��: 129-160) */ //(��. ��`�I���n�̂قƂ�ǂ͖�����)
/* 129 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 130 */		F_DISABLE/*F_BOXSELALL*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//��`�ł��ׂđI��
/* 131 */		F_BEGIN_BOX								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//��`�͈͑I���J�n	//Sept. 29, 2000 JEPRO �ǉ�
/* 132 */		F_DISABLE/*F_UP_BOX*/					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�J�[�\����ړ�
/* 133 */		F_DISABLE/*F_DOWN_BOX*/					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�J�[�\�����ړ�
/* 134 */		F_DISABLE/*F_LEFT_BOX*/					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�J�[�\�����ړ�
/* 135 */		F_DISABLE/*F_RIGHT_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�J�[�\���E�ړ�
/* 136 */		F_DISABLE/*F_UP2_BOX*/					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�J�[�\����ړ�(�Q�s����)
/* 137 */		F_DISABLE/*F_DOWN2_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�J�[�\�����ړ�(�Q�s����)
/* 138 */		F_DISABLE/*F_WORDLEFT_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�P��̍��[�Ɉړ�
/* 139 */		F_DISABLE/*F_WORDRIGHT_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�P��̉E�[�Ɉړ�
/* 140 */		F_DISABLE/*F_GOLINETOP_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
/* 141 */		F_DISABLE/*F_GOLINEEND_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
/* 142 */		F_DISABLE/*F_HalfPageUp_Box*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)���y�[�W�A�b�v
/* 143 */		F_DISABLE/*F_HalfPageDown_Box*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)���y�[�W�_�E��
/* 144 */		F_DISABLE/*F_1PageUp_Box*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�P�y�[�W�A�b�v
/* 145 */		F_DISABLE/*F_1PageDown_Box*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�P�y�[�W�_�E��
/* 146 */		F_DISABLE/*F_DISABLE/*F_DISPLAYTOP_BOX*//* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)��ʂ̐擪�Ɉړ�(������)
/* 147 */		F_DISABLE/*F_DISPLAYEND_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)��ʂ̍Ō�Ɉړ�(������)
/* 148 */		F_DISABLE/*F_GOFILETOP_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�t�@�C���̐擪�Ɉړ�
/* 149 */		F_DISABLE/*F_GOFILEEND_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(��`�I��)�t�@�C���̍Ō�Ɉړ�
/* 150 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 151 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 152 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 153 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 154 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 155 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 156 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 157 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 158 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 159 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 160 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[

/* �N���b�v�{�[�h�n(6�i��24��: 161-184) */
/* 161 */		F_CUT							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
/* 162 */		F_COPY							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
/* 163 */		F_COPY_CRLF						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//CRLF���s�ŃR�s�[
/* 164 */		F_PASTE							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�\��t��(�N���b�v�{�[�h����\��t��)
/* 165 */		F_PASTEBOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//��`�\��t��(�N���b�v�{�[�h����\��t��)
/* 166 */		F_DISABLE/*F_INSTEXT_W*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�e�L�X�g��\��t��	(�����J�R�}���h�H�������H)
/* 167 */		F_DISABLE/*F_ADDTAIL_W*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�Ō�Ƀe�L�X�g��ǉ�	(�����J�R�}���h�H�������H)
/* 168 */		F_COPYLINES						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�I��͈͓��S�s�R�s�[	//Sept. 30, 2000 JEPRO �ǉ�
/* 169 */		F_COPYLINESASPASSAGE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�I��͈͓��S�s���p���t���R�s�[	//Sept. 30, 2000 JEPRO �ǉ�
/* 170 */		F_COPYLINESWITHLINENUMBER		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�I��͈͓��S�s�s�ԍ��t���R�s�[	//Sept. 30, 2000 JEPRO �ǉ�
/* 171 */		F_COPYPATH						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̃t�@�C���̃p�X�����R�s�[ //added Oct. 22, 2000 JEPRO				//Nov. 5, 2000 JEPRO �ǉ�
/* 172 */		F_COPYTAG						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[ //added Oct. 22, 2000 JEPRO	//Nov. 5, 2000 JEPRO �ǉ�
/* 173 */		F_CREATEKEYBINDLIST				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�L�[���蓖�Ĉꗗ���R�s�[ //added Oct. 22, 2000 JEPRO	//Dec. 25, 2000 JEPRO �A�C�R���ǉ�
/* 174 */		F_COPYFNAME						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ //2002/2/3 aroka
/* 175 */		F_COPY_ADDCRLF					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[
/* 176 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 177 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 178 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 179 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 180 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 181 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 182 */		F_CHGMOD_EOL_CRLF,	// 2003.06.23 Moca
/* 183 */		F_CHGMOD_EOL_LF,	// 2003.06.23 Moca
/* 184 */		F_CHGMOD_EOL_CR,	// 2003.06.23 Moca

/* �}���n(6�i�ڎc��8��: 185-192) */
/* 185 */		F_INS_DATE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���t�}��	//Nov. 5, 2000 JEPRO �ǉ�
/* 186 */		F_INS_TIME	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�����}��	//Nov. 5, 2000 JEPRO �ǉ�
/* 187 */		F_CTRL_CODE_DIALOG	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�R���g���[���R�[�h�̓���(�_�C�A���O)	//@@@ 2002.06.02 MIK
/* 188 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 189 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 190 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 191 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 192 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[

/* �ϊ��n(7�i��32��: 193-224) */
/* 193 */		F_TOLOWER				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//������
/* 194 */		F_TOUPPER				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�啶��
/* 195 */		F_TOHANKAKU				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�S�p�����p
/* 196 */		F_TOZENKAKUKATA			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���p�{�S�Ђ灨�S�p�E�J�^�J�i	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
/* 197 */		F_TOZENKAKUHIRA			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���p�{�S�J�^���S�p�E�Ђ炪��	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
/* 198 */		F_HANKATATOZENKATA	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���p�J�^�J�i���S�p�J�^�J�i	//Sept. 18, 2000 JEPRO �ǉ�
/* 199 */		F_HANKATATOZENHIRA	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���p�J�^�J�i���S�p�Ђ炪��	//Sept. 18, 2000 JEPRO �ǉ�
/* 200 */		F_TABTOSPACE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//TAB����	//Sept. 20, 2000 JEPRO �ǉ�
/* 201 */		F_CODECNV_AUTO2SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�������ʁ�SJIS�R�[�h�ϊ�
/* 202 */		F_CODECNV_EMAIL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//E-Mail(JIS��SIJIS)�R�[�h�ϊ�
/* 203 */		F_CODECNV_EUC2SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//EUC��SJIS�R�[�h�ϊ�
/* 204 */		F_CODECNV_UNICODE2SJIS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Unicode��SJIS�R�[�h�ϊ�
/* 205 */		F_CODECNV_UTF82SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UTF-8��SJIS�R�[�h�ϊ�
/* 206 */		F_CODECNV_UTF72SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UTF-7��SJIS�R�[�h�ϊ�
/* 207 */		F_CODECNV_SJIS2JIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJIS��JIS�R�[�h�ϊ�
/* 208 */		F_CODECNV_SJIS2EUC		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJIS��EUC�R�[�h�ϊ�
/* 209 */		F_CODECNV_SJIS2UTF8		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJIS��UTF-8�R�[�h�ϊ�
/* 210 */		F_CODECNV_SJIS2UTF7		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJIS��UTF-7�R�[�h�ϊ�
/* 211 */		F_BASE64DECODE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Base64�f�R�[�h���ĕۑ�	//Sept. 28, 2000 JEPRO �ǉ�
/* 212 */		F_UUDECODE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//uudecode���ăt�@�C���ɕۑ�//Sept. 28, 2000 JEPRO �ǉ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX
/* 213 */		F_SPACETOTAB			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�󔒁�TAB	//Jun. 01, 2001 JEPRO �ǉ�
/* 214 */		F_TOZENEI				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���p�p�����S�p�p�� //July. 30, 2001 Misaka �ǉ�
/* 215 */		F_TOHANEI				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�S�p�p�������p�p��
/* 216 */		F_CODECNV_UNICODEBE2SJIS/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UnicodeBE��SJIS�R�[�h�ϊ�	//Moca, 2002/05/26
/* 217 */		F_TOHANKATA				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�S�p�J�^�J�i�����p�J�^�J�i	//Aug. 29, 2002 ai
/* 218 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 219 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 220 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 221 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 222 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 223 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 224 */		F_OUTLINE_TOGGLE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�A�E�g���C�����(toggle) // 20060201 aroka

/* �����n(8�i��32��: 225-256) */
/* 225 */		F_SEARCH_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//����(�P�ꌟ���_�C�A���O)
/* 226 */		F_SEARCH_NEXT		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//��������
/* 227 */		F_SEARCH_PREV		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O������
/* 228 */		F_REPLACE_DIALOG	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�u��	//Sept. 21, 2000 JEPRO �ǉ�
/* 229 */		F_SEARCH_CLEARMARK	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�����}�[�N�̃N���A
/* 230 */		F_GREP_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Grep
/* 231 */		F_JUMP_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�w��s�փW�����v		//Sept. 21, 2000 JEPRO �ǉ�
/* 232 */		F_OUTLINE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�A�E�g���C�����
/* 233 */		F_TAGJUMP			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�^�O�W�����v�@�\			//Sept. 21, 2000 JEPRO �ǉ�
/* 234 */		F_TAGJUMPBACK		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�^�O�W�����v�o�b�N�@�\	//Sept. 21, 2000 JEPRO �ǉ�
/* 235 */		F_COMPARE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�t�@�C�����e��r	//Sept. 21, 2000 JEPRO �ǉ�
/* 236 */		F_BRACKETPAIR		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�Ί��ʂ̌���	//Sept. 20, 2000 JEPRO �ǉ�
/* 237 */		F_BOOKMARK_SET		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�u�b�N�}�[�N�ݒ�E����	// 2001.12.03 hor
/* 238 */		F_BOOKMARK_NEXT		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̃u�b�N�}�[�N��		// 2001.12.03 hor
/* 239 */		F_BOOKMARK_PREV		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O�̃u�b�N�}�[�N��		// 2001.12.03 hor
/* 240 */		F_BOOKMARK_RESET	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�u�b�N�}�[�N�̑S����		// 2001.12.03 hor
/* 241 */		F_BOOKMARK_VIEW		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�u�b�N�}�[�N�̈ꗗ		// 2001.12.03 hor
/* 242 */		F_DIFF_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//DIFF�����\��	//@@@ 2002.05.25 MIK
/* 243 */		F_DIFF_NEXT			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̍�����	//@@@ 2002.05.25 MIK
/* 244 */		F_DIFF_PREV			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O�̍�����	//@@@ 2002.05.25 MIK
/* 245 */		F_DIFF_RESET		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�����̑S����	//@@@ 2002.05.25 MIK
/* 246 */		F_SEARCH_BOX		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//����(�{�b�N�X)
/* 247 */		F_JUMP_SRCHSTARTPOS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�����J�n�ʒu�֖߂�	// 02/06/26 ai
/* 248 */		F_TAGS_MAKE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�^�O�t�@�C���̍쐬	//@@@ 2003.04.13 MIK
/* 249 */		F_DIRECT_TAGJUMP	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�C���N�g�^�O�W�����v	//@@@ 2003.04.15 MIK
/* 250 */		F_ISEARCH_NEXT		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���C���N�������^���T�[�` //2004.10.13 isearch
/* 251 */		F_ISEARCH_PREV		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//����C���N�������^���T�[�` //2004.10.13 isearch
/* 252 */		F_ISEARCH_REGEXP_NEXT	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���K�\���O���C���N�������^���T�[�` //2004.10.13 isearch
/* 253 */		F_ISEARCH_REGEXP_PREV	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���K�\���O���C���N�������^���T�[�` //2004.10.13 isearch
/* 254 */		F_ISEARCH_MIGEMO_NEXT	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//MIGEMO�O���C���N�������^���T�[�` //2004.10.13 isearch
/* 255 */		F_ISEARCH_MIGEMO_PREV	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//MIGEMO�O���C���N�������^���T�[�` //2004.10.13 isearch
/* 256 */		F_TAGJUMP_KEYWORD	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�L�[���[�h���w�肵�ă_�C���N�g�^�O�W�����v //2005.03.31 MIK

/* ���[�h�؂�ւ��n(9�i��4��: 257-260) */
/* 257 */		F_CHGMOD_INS	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�}���^�㏑�����[�h�؂�ւ�	//Nov. 5, 2000 JEPRO �ǉ�
/* 258 */		F_CANCEL_MODE	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�e�탂�[�h�̎�����			//Nov. 7, 2000 JEPRO �ǉ�
/* 259 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 260 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[

/* �ݒ�n(9�i�ڎ���16��: 261-276) */
/* 261 */		F_SHOWTOOLBAR		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�c�[���o�[�̕\��
/* 262 */		F_SHOWFUNCKEY		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�t�@���N�V�����L�[�̕\��
/* 263 */		F_SHOWSTATUSBAR		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�X�e�[�^�X�o�[�̕\��
/* 264 */		F_TYPE_LIST			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�^�C�v�ʐݒ�ꗗ	//Sept. 18, 2000 JEPRO �ǉ�
/* 265 */		F_OPTION_TYPE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�^�C�v�ʐݒ�
/* 266 */		F_OPTION			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���ʐݒ�			//Sept. 16, 2000 jepro �������u�ݒ�v���p�e�B�V�[�g�v����ύX
/* 267 */		F_FONT				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�t�H���g�ݒ�
/* 268 */		F_WRAPWINDOWWIDTH	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���݂̃E�B���h�E���Ő܂�Ԃ�	//	Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
/* 269 */		F_FAVORITE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�����̊Ǘ�	//@@@ 2003.04.08 MIK
/* 270 */		F_SHOWTAB			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�^�u�̕\��	//@@@ 2003.06.10 MIK
/* 271 */		F_DISABLE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 272 */		F_TOGGLE_KEY_SEARCH	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�L�[���[�h�w���v�����\�� 2007.03.92 genta
/* 273 */		F_TMPWRAPNOWRAP		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�܂�Ԃ��Ȃ��i�ꎞ�ݒ�j			// 2008.05.30 nasukoji
/* 274 */		F_TMPWRAPSETTING	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�w�茅�Ő܂�Ԃ��i�ꎞ�ݒ�j		// 2008.05.30 nasukoji
/* 275 */		F_TMPWRAPWINDOW		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�E�[�Ő܂�Ԃ��i�ꎞ�ݒ�j		// 2008.05.30 nasukoji
/* 276 */		F_SELECT_COUNT_MODE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�����J�E���g���@	//2009.07.06 syat

/* �}�N���n(9�i�ڍŌ��12��: 277-288) */
/* 277 */		F_RECKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�L�[�}�N���̋L�^�J�n�^�I��
/* 278 */		F_SAVEKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�L�[�}�N���̕ۑ�		//Sept. 21, 2000 JEPRO �ǉ�
/* 279 */		F_LOADKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�L�[�}�N���̓ǂݍ���	//Sept. 21, 2000 JEPRO �ǉ�
/* 280 */		F_EXECKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�L�[�}�N���̎��s		//Sept. 16, 2000 JEPRO �������Ɉړ�����
/* 281 */		F_EXECMD_DIALOG			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���R�}���h���s//Sept. 20, 2000 JEPRO ���̂�CMMAND����COMMAND�ɕύX(EXECCMMAND��EXECCMMAND)
/* 282 */		F_EXECEXTMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���O���w�肵�ă}�N�����s	//2008.10.22 syat �ǉ�
/* 283 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 284 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 285 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 286 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 287 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 288 */		F_TAB_CLOSEOTHER		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̃^�u�ȊO�����		// 2009.07.07 syat,�E�B���h�E�n�ɋ󂫂��Ȃ��̂ŏ�̍s��N�H

/* �J�X�^�����j���[(10�i��32��: 289-320) */
/* 289 */		F_MENU_RBUTTON				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�E�N���b�N���j���[ 	//Sept. 30, 2000 JEPRO �ǉ�
/* 290 */		F_CUSTMENU_1				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[1
/* 291 */		F_CUSTMENU_2				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[2
/* 292 */		F_CUSTMENU_3				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[3
/* 293 */		F_CUSTMENU_4				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[4
/* 294 */		F_CUSTMENU_5				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[5
/* 295 */		F_CUSTMENU_6				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[6
/* 296 */		F_CUSTMENU_7				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[7
/* 297 */		F_CUSTMENU_8				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[8
/* 298 */		F_CUSTMENU_9				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[9
/* 299 */		F_CUSTMENU_10				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[10
/* 300 */		F_DISABLE/*F_CUSTMENU_11*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[11	//�A�C�R������
/* 301 */		F_DISABLE/*F_CUSTMENU_12*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[12	//�A�C�R������
/* 302 */		F_DISABLE/*F_CUSTMENU_13*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[13	//�A�C�R������
/* 303 */		F_DISABLE/*F_CUSTMENU_14*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[14	//�A�C�R������
/* 304 */		F_DISABLE/*F_CUSTMENU_15*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[15	//�A�C�R������
/* 305 */		F_DISABLE/*F_CUSTMENU_16*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[16	//�A�C�R������
/* 306 */		F_DISABLE/*F_CUSTMENU_17*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[17	//�A�C�R������
/* 307 */		F_DISABLE/*F_CUSTMENU_18*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[18	//�A�C�R������
/* 308 */		F_DISABLE/*F_CUSTMENU_19*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[19	//�A�C�R������
/* 309 */		F_DISABLE/*F_CUSTMENU_20*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[20	//�A�C�R������
/* 310 */		F_DISABLE/*F_CUSTMENU_21*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[21	//�A�C�R������
/* 311 */		F_DISABLE/*F_CUSTMENU_22*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[22	//�A�C�R������
/* 312 */		F_DISABLE/*F_CUSTMENU_23*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[23	//�A�C�R������
/* 313 */		F_CUSTMENU_24				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�J�X�^�����j���[24	//�A�C�R������
/* 314 */		F_TAB_MOVERIGHT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�^�u���E�Ɉړ�	// 2007.06.20 ryoji
/* 315 */		F_TAB_MOVELEFT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�^�u�����Ɉړ�	// 2007.06.20 ryoji
/* 316 */		F_TAB_SEPARATE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�V�K�O���[�v	// 2007.06.20 ryoji
/* 317 */		F_TAB_JOINTNEXT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̃O���[�v�Ɉړ�	// 2007.06.20 ryoji
/* 318 */		F_TAB_JOINTPREV				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O�̃O���[�v�Ɉړ�	// 2007.06.20 ryoji
/* 319 */		F_TAB_CLOSERIGHT			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�E�����ׂĕ���		// 2009.07.07 syat
/* 320 */		F_TAB_CLOSELEFT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�������ׂĕ���		// 2009.07.07 syat

/* �E�B���h�E�n(11�i��22��: 321-342) */
/* 321 */		F_SPLIT_V		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�㉺�ɕ���	//Sept. 16, 2000 jepro �������u�c�v����u�㉺�Ɂv�ɕύX
/* 322 */		F_SPLIT_H		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���E�ɕ���	//Sept. 16, 2000 jepro �������u���v����u���E�Ɂv�ɕύX
/* 323 */		F_SPLIT_VH		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
/* 324 */		F_WINCLOSE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�E�B���h�E�����
/* 325 */		F_WIN_CLOSEALL	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���ׂẴE�B���h�E�����	//Sept. 18, 2000 JEPRO �ǉ�	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
/* 329 */		F_NEXTWINDOW	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̃E�B���h�E
/* 330 */		F_PREVWINDOW	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O�̃E�B���h�E
/* 326 */		F_CASCADE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�d�˂ĕ\��
/* 237 */		F_TILE_V		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�㉺�ɕ��ׂĕ\��
/* 328 */		F_TILE_H		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���E�ɕ��ׂĕ\��
/* 331 */		F_MAXIMIZE_V	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�c�����ɍő剻
/* 332 */		F_MAXIMIZE_H	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�������ɍő剻 //2001.02.10 by MIK
/* 333 */		F_MINIMIZE_ALL	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���ׂčŏ���					//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
/* 334 */		F_REDRAW		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�ĕ`��						//Sept. 30, 2000 JEPRO �ǉ�
/* 335 */		F_WIN_OUTPUT	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�A�E�g�v�b�g�E�B���h�E�\��	//Sept. 18, 2000 JEPRO �ǉ�
/* 336 */		F_BIND_WINDOW	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�������ĕ\��	2004.07.14 kazika / Design Oct. 1, 2004 genta
/* 337 */		F_TOPMOST		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//��Ɏ�O�ɕ\�� 2004.09.21 Moca
/* 338 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 339 */		F_WINLIST		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�E�B���h�E�ꗗ�|�b�v�A�b�v�\��	// 2006.03.23 fon
/* 340 */		F_GROUPCLOSE	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���[�v�����	// 2007.06.20 ryoji
/* 341 */		F_NEXTGROUP		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���̃O���[�v	// 2007.06.20 ryoji
/* 342 */		F_PREVGROUP		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O�̃O���[�v	// 2007.06.20 ryoji

/* �x��(11�i�ڎc���10��: 343-352) */
/* 343 */		F_HOKAN			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//���͕⊮
/* 344 */		F_HELP_CONTENTS /*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�w���v�ڎ�			//Nov. 25, 2000 JEPRO �ǉ�
/* 345 */		F_HELP_SEARCH	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�w���v�L�[���[�h����	//Nov. 25, 2000 JEPRO �ǉ�
/* 346 */		F_MENU_ALLFUNC	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�R�}���h�ꗗ			//Sept. 30, 2000 JEPRO �ǉ�
/* 347 */		F_EXTHELP1		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���w���v�P
/* 348 */		F_EXTHTMLHELP	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O��HTML�w���v
/* 349 */		F_ABOUT			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�o�[�W�������	//Dec. 24, 2000 JEPRO �ǉ�
/* 350 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 351 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 352 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[

//2002.01.17 hor ����̑���̃G���A���O���}�N���p�Ɋ�����
/* �O���}�N��(12�i��32��: 353-384) */
/* 353 */		F_USERMACRO_0		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���@
/* 354 */		F_USERMACRO_0+1		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���A
/* 355 */		F_USERMACRO_0+2		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���B
/* 356 */		F_USERMACRO_0+3		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���C
/* 357 */		F_USERMACRO_0+4		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���D
/* 358 */		F_USERMACRO_0+5		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���E
/* 359 */		F_USERMACRO_0+6		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���F
/* 360 */		F_USERMACRO_0+7		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���G
/* 361 */		F_USERMACRO_0+8		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���H
/* 362 */		F_USERMACRO_0+9		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���I
/* 363 */		F_USERMACRO_0+10	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���J
/* 364 */		F_USERMACRO_0+11	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���K
/* 365 */		F_USERMACRO_0+12	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���L
/* 366 */		F_USERMACRO_0+13	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���M
/* 367 */		F_USERMACRO_0+14	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���N
/* 368 */		F_USERMACRO_0+15	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���O
/* 369 */		F_USERMACRO_0+16	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���P
/* 370 */		F_USERMACRO_0+17	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���Q
/* 371 */		F_USERMACRO_0+18	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���R
/* 372 */		F_USERMACRO_0+19	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�O���}�N���S
/* 373 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 374 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 375 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 376 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 377 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 378 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 379 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 380 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 381 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 382 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�_�~�[
/* 383 */		F_PLUGCOMMAND_FIRST	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�v���O�C���R�}���h�p�ɗ\��
//	2007.10.17 genta 384�͐܂�Ԃ��}�[�N�Ƃ��Ďg�p���Ă���̂ŃA�C�R���Ƃ��Ă͎g�p�ł��Ȃ�
/* 384 */		F_TOOLBARWRAP		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//�ܕԂŗ\��ς݂ł��邱�Ƃ��������z�A�C�R��
/* 385 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */	//�ŏI�s�p�_�~�[(Jepro note: �ŏI�s���ɂ̓J���}��t���Ȃ�����)

};
	int tbd_num = _countof( tbd );
	BYTE	style;	//@@@ 2002.06.15 MIK

	m_tbMyButton.resize( tbd_num + 1 );

	for( int i = 0; i < tbd_num; i++ ){

		if( tbd[i] == F_TOOLBARWRAP ){
			// �c�[���o�[���s�p�̉��z�{�^���i���ۂ͕\������Ȃ��j // 20050809 aroka
			//	2007.10.12 genta �܂�Ԃ��{�^�����Ō�̃f�[�^�Əd�Ȃ��Ă��邪�C
			//	�C���f�b�N�X��ύX�����sakura.ini�������p���Ȃ��Ȃ�̂�
			//	�d�������m�ł��̂܂܂ɂ���
			SetTBBUTTONVal(
				&m_tbMyButton[i+1],
				-1,						// 2007.11.02 ryoji �A�C�R���̖���`��(-1)
				F_MENU_NOT_USED_FIRST,			//	tbd[i].idCommand,
				TBSTATE_ENABLED|TBSTATE_WRAP,	//	tbd[i].fsState,
				TBSTYLE_SEP,			//	tbd[i].fsStyle,
				0,						//	tbd[i].dwData,
				0						//	tbd[i].iString
			);
			continue;
		}

		switch( tbd[i] )	//@@@ 2002.06.15 MIK
		{
		case F_FILEOPEN_DROPDOWN:
			style = TBSTYLE_DROPDOWN;	//�h���b�v�_�E��
			break;

		case F_SEARCH_BOX:
			style = TBSTYLE_COMBOBOX;	//�R���{�{�b�N�X
			break;

		default:
			style = TBSTYLE_BUTTON;	//�{�^��
			break;
		}

		//	m_tbMyButton[0]�ɂ̓Z�p���[�^�������Ă��邽�߁B
		SetTBBUTTONVal(
			&m_tbMyButton[i+1],
			(tbd[i] == F_DISABLE)? -1: i,	// 2007.11.02 ryoji �A�C�R���̖���`��(-1)
			tbd[i],				//	tbd[i].idCommand,
			TBSTATE_ENABLED,	//	tbd[i].fsState,
			style /*TBSTYLE_BUTTON*/,		//	tbd[i].fsStyle,
			0,					//	tbd[i].dwData,
			0					//	tbd[i].iString
		);
	}

	m_nMyButtonNum = tbd_num + 1;	//	+ 1�́A�Z�p���[�^�̕�
	return;
}


CMenuDrawer::~CMenuDrawer()
{
	if( NULL != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = NULL;
	}
	if( NULL != m_hFontMenuUndelLine ){
		::DeleteObject( m_hFontMenuUndelLine );
		m_hFontMenuUndelLine = NULL;
	}
	return;
}

void CMenuDrawer::Create( HINSTANCE hInstance, HWND hWndOwner, CImageListMgr* pcIcons )
{
	m_hInstance = hInstance;
	m_hWndOwner = hWndOwner;
	m_pcIcons = pcIcons;

	return;
}


void CMenuDrawer::ResetContents( void )
{
	int		i;
	LOGFONT	lf;
	for( i = 0; i < m_nMenuItemNum; ++i ){
		m_cmemMenuItemStrArr[i].SetString(_T(""));
		m_nMenuItemFuncArr[i] = 0;
	}
	m_nMenuItemNum = 0;

	NONCLIENTMETRICS	ncm;
	// �ȑO�̃v���b�g�t�H�[���� WINVER >= 0x0600 �Œ�`�����\���̂̃t���T�C�Y��n���Ǝ��s����	// 2007.12.21 ryoji
	ncm.cbSize = CCSIZEOF_STRUCT( NONCLIENTMETRICS, lfMessageFont );
	::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0 );

	m_nMenuHeight = ncm.iMenuHeight;
	if( 21 > m_nMenuHeight ){
		m_nMenuHeight = 21;
	}

	if( NULL != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = NULL;
	}
	if( NULL != m_hFontMenuUndelLine ){
		::DeleteObject( m_hFontMenuUndelLine );
		m_hFontMenuUndelLine = NULL;
	}
	lf = ncm.lfMenuFont;
	m_hFontMenu = ::CreateFontIndirect( &lf );
	lf.lfUnderline = TRUE;
	m_hFontMenuUndelLine = ::CreateFontIndirect( &lf );
//@@@ 2002.01.03 YAZAKI �s�g�p�̂���
//	m_nMaxTab = 0;
//	m_nMaxTabLen = 0;
	return;
}




/* ���j���[�A�C�e���̕`��T�C�Y���v�Z */
int CMenuDrawer::MeasureItem( int nFuncID, int* pnItemHeight )
{


	*pnItemHeight = m_nMenuHeight;
	const TCHAR* pszLabel;
	RECT rc;
	HDC hdc;
	HFONT hFontOld;

	if( NULL == ( pszLabel = GetLabel( nFuncID ) ) ){
		return 0;
	}
	hdc = ::GetDC( m_hWndOwner );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );
	::DrawText( hdc, pszLabel, _tcslen( pszLabel ), &rc, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_CALCRECT );
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWndOwner, hdc );


//	*pnItemHeight = 20;
//	*pnItemHeight = 2 + 15 + 1;
	//@@@ 2002.2.2 YAZAKI Windows�̐ݒ�Ń��j���[�̃t�H���g��傫������ƕ\�����������ɑΏ�
	*pnItemHeight = GetSystemMetrics(SM_CYMENU);

	return rc.right - rc.left + 20 + 8;
//	return m_nMaxTab + 16 + m_nMaxTabLen;

}

/* ���j���[���ڂ�ǉ� */
void CMenuDrawer::MyAppendMenu(
	HMENU			hMenu,
	int				nFlag,
	int				nFuncId,
	const TCHAR*	pszLabel,
	BOOL			bAddKeyStr,
	int				nForceIconId	//���C�ɓ���	//@@@ 2003.04.08 MIK
)
{
	TCHAR		szLabel[256];
	int			nFlagAdd = 0;
	int			i;

	if( nForceIconId == -1 ) nForceIconId = nFuncId;	//���C�ɓ���	//@@@ 2003.04.08 MIK

	szLabel[0] = _T('\0');
	if( NULL != pszLabel ){
		_tcsncpy( szLabel, pszLabel, _countof( szLabel ) - 1 );
		szLabel[ _countof( szLabel ) - 1 ] = _T('\0');
	}
	if( nFuncId != 0 ){
		/* ���j���[���x���̍쐬 */
		CKeyBind::GetMenuLabel(
			m_hInstance,
			m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
			m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr,
			nFuncId,
			szLabel,
			bAddKeyStr
		 );

		/* �A�C�R���p�r�b�g�}�b�v�������̂́A�I�[�i�[�h���E�ɂ��� */

		if( m_nMenuItemNum + 1 > MAX_MENUITEMS ){
			TopErrorMessage(	NULL,
				_T("CMenuDrawer::MyAppendMenu()�G���[\n")
				_T("\n")
				_T("CMenuDrawer���Ǘ��ł��郁�j���[�A�C�e���̏����CMenuDrawer::MAX_MENUITEMS==%d�ł��B\n "),
				MAX_MENUITEMS
			);
		}
		else{

			m_nMenuItemBitmapIdxArr[m_nMenuItemNum] = -1;
			m_nMenuItemFuncArr[m_nMenuItemNum] = nFuncId;
			m_cmemMenuItemStrArr[m_nMenuItemNum].SetString( szLabel, _tcslen( szLabel ) );
//#ifdef _DEBUG
			/* ���j���[���ڂ��I�[�i�[�`��ɂ��� */
			/* ���j���[�ɃA�C�R����\������ */
			if( m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
				nFlagAdd = MF_OWNERDRAW;
			}
//#endif
			/* �@�\�̃r�b�g�}�b�v�����邩�ǂ������ׂĂ��� */
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
//			for( i = 0; i < m_cShareData.m_nMyButtonNum; ++i ){
//				if( nFuncId == m_cShareData.m_tbMyButton[i].idCommand ){
//					/* �@�\�̃r�b�g�}�b�v�̏����o���Ă��� */
//					m_nMenuItemBitmapIdxArr[m_nMenuItemNum] = m_cShareData.m_tbMyButton[i].iBitmap;
//					break;
//				}
//			}
			for( i = 0; i < m_nMyButtonNum; ++i ){
				//if( nFuncId == m_tbMyButton[i].idCommand ){
				if( nForceIconId == m_tbMyButton[i].idCommand ){	//���C�ɓ���	//@@@ 2003.04.08 MIK
					/* �@�\�̃r�b�g�}�b�v�̏����o���Ă��� */
					m_nMenuItemBitmapIdxArr[m_nMenuItemNum] = m_tbMyButton[i].iBitmap;
					break;
				}
			}
			m_nMenuItemNum++;
		}
	}

	MENUITEMINFO mii;
	memset_raw( &mii, 0, sizeof( mii ) );
	//	Aug. 31, 2001 genta
	mii.cbSize = SIZEOF_MENUITEMINFO; //Win95�΍�ς݂�sizeof(MENUITEMINFO)�l

	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = 0;
	if( MF_OWNERDRAW	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_OWNERDRAW;
	if( MF_SEPARATOR	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_SEPARATOR;
	if( MF_STRING		& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_STRING;
	if( MF_MENUBREAK	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_MENUBREAK;
	if( MF_MENUBARBREAK	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_MENUBARBREAK;

	mii.fState = 0;
	if( MF_GRAYED		& ( nFlag | nFlagAdd ) ) mii.fState |= MFS_GRAYED;
	if( MF_CHECKED		& ( nFlag | nFlagAdd ) ) mii.fState |= MFS_CHECKED;

	mii.wID = nFuncId;
	mii.hSubMenu = (nFlag&MF_POPUP)?((HMENU)nFuncId):NULL;
	mii.hbmpChecked = NULL;
	mii.hbmpUnchecked = NULL;
	mii.dwItemData = (ULONG_PTR)this;
	mii.dwTypeData = szLabel;
	mii.cch = 0;

	// ���j���[���̎w�肳�ꂽ�ʒu�ɁA�V�������j���[���ڂ�}�����܂��B
	::InsertMenuItem( hMenu, 0xFFFFFFFF, TRUE, &mii );
	return;
}




/*! ���j���[�A�C�e���`��
	@date 2001.12.21 YAZAKI �f�o�b�O���[�h�ł����j���[��I��������n�C���C�g�B
	@date 2003.08.27 Moca �V�X�e���J���[�̃u���V��CreateSolidBrush�����GetSysColorBrush��
*/
void CMenuDrawer::DrawItem( DRAWITEMSTRUCT* lpdis )
{

//	int			i;
	int			j;
	int			nItemIndex;
	HDC			hdc;
	int			nItemStrLen;
	HFONT		hFontOld;
	int			nIndentLeft;
	int			nIndentRight;
	int			nTextTopMargin;
	RECT		rc1;
	HBRUSH		hBrush;
	RECT		rcText;
	int			nBkModeOld;

	nIndentLeft = 26;
	nIndentRight = 8;
	nTextTopMargin = 3;

/*	for( i = 0; i < m_nMenuItemNum; ++i ){
		if( (int)lpdis->itemID == m_nMenuItemFuncArr[i] ){
			break;
		}
	}
	if( i >= m_nMenuItemNum ){
		return;
	}
	nItemIndex = i;
*/
//@@@ 2002.01.03 YAZAKI �ɗ̓����o�֐����g�p����悤�ɁB
	nItemIndex = Find( (int)lpdis->itemID );
	const TCHAR*	pszItemStr;
	pszItemStr = m_cmemMenuItemStrArr[nItemIndex].GetStringPtr( &nItemStrLen );

	hdc = lpdis->hDC;
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );


	/* �A�C�e����`�h��Ԃ� */
	hBrush = ::GetSysColorBrush( COLOR_MENU );
	::FillRect( hdc, &lpdis->rcItem, hBrush );


	/* �A�C�e�����I������Ă��� */
	nBkModeOld = ::SetBkMode( hdc, TRANSPARENT );
	if( lpdis->itemState & ODS_SELECTED ){
		hBrush = ::GetSysColorBrush( COLOR_HIGHLIGHT );
		rc1 = lpdis->rcItem;
		if( -1 != m_nMenuItemBitmapIdxArr[nItemIndex] || lpdis->itemState & ODS_CHECKED ){
			rc1.left += (nIndentLeft - 2);
		}
		/* �I���n�C���C�g��` */
		::FillRect( hdc, &rc1, hBrush );

		/* �A�C�e�����g�p�s�� */
		if( lpdis->itemState & ODS_DISABLED ){
			::SetTextColor( hdc, ::GetSysColor( COLOR_MENU/*COLOR_3DSHADOW*/ ) );
		}else{
			::SetTextColor( hdc, ::GetSysColor( COLOR_HIGHLIGHTTEXT/*COLOR_MENUTEXT*//*COLOR_3DHIGHLIGHT*/ ) );
		}
	}else{
		/* �A�C�e�����g�p�s�� */
		if( lpdis->itemState & ODS_DISABLED ){
			::SetTextColor( hdc, ::GetSysColor( COLOR_3DSHADOW ) );
		}else{
			::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
		}
	}

#ifdef _DEBUG
	TCHAR	szText[1024];
	MENUITEMINFO mii;
	// ���j���[���ڂɊւ�������擾���܂��B
	memset_raw( &mii, 0, sizeof( mii ) );

	mii.cbSize = SIZEOF_MENUITEMINFO; // Win95�΍�ς݂�sizeof(MENUITEMINFO)�l

	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = MFT_STRING;
	_tcscpy( szText, _T("--unknown--") );
	mii.dwTypeData = szText;
	mii.cch = _countof( szText ) - 1;
	if( 0 != ::GetMenuItemInfo( (HMENU)lpdis->hwndItem, lpdis->itemID, FALSE, &mii )
	 && NULL == mii.hSubMenu
	 && 0 == /* CEditWnd */::FuncID_To_HelpContextID( (EFunctionCode)lpdis->itemID ) 	/* �@�\ID�ɑΉ����郁�j���[�R���e�L�X�g�ԍ���Ԃ� */
	){
		//@@@ 2001.12.21 YAZAKI
		if( lpdis->itemState & ODS_SELECTED ){
			::SetTextColor( hdc, ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );	//	�n�C���C�g�J���[
		}
		else {
			::SetTextColor( hdc, RGB( 0, 0, 255 ) );	//	�����Ă�B
		}
//		::SetTextColor( hdc, RGB( 0, 0, 255 ) );
	}
#endif

	rcText = lpdis->rcItem;
	rcText.left += nIndentLeft;
	rcText.right -= nIndentRight;

	/* TAB�����̑O�ƌ��ɕ������ăe�L�X�g��`�悷�� */
	for( j = 0; j < nItemStrLen; ++j ){
		if( pszItemStr[j] == _T('\t') ){
			break;
		}
	}
	/* TAB�����̌�둤�̃e�L�X�g��`�悷�� */
	if( j < nItemStrLen ){
		/* �A�C�e�����g�p�s�� */
		if( lpdis->itemState & ODS_DISABLED && !(lpdis->itemState & ODS_SELECTED)  ){
			COLORREF colOld = ::SetTextColor( hdc, ::GetSysColor( COLOR_3DHIGHLIGHT ) );
				rcText.left++;
				rcText.top++;
				rcText.right++;
				rcText.bottom++;
				::DrawText(
					hdc,
					&pszItemStr[j + 1],
					_tcslen( &pszItemStr[j + 1] ),
					&rcText,
					DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_RIGHT
				);
				rcText.left--;
				rcText.top--;
				rcText.right--;
				rcText.bottom--;
				::SetTextColor( hdc, colOld );
		}
		::DrawText(
			hdc,
			&pszItemStr[j + 1],
			_tcslen( &pszItemStr[j + 1] ),
			&rcText,
			DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_RIGHT
		);
	}
	/* TAB�����̑O���̃e�L�X�g��`�悷�� */
	/* �A�C�e�����g�p�s�� */
	if( lpdis->itemState & ODS_DISABLED && !(lpdis->itemState & ODS_SELECTED)  ){
		COLORREF colOld = ::SetTextColor( hdc, ::GetSysColor( COLOR_3DHIGHLIGHT ) );

		rcText.left++;
		rcText.top++;
		rcText.right++;
		rcText.bottom++;
		::DrawText( hdc, pszItemStr, j, &rcText, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_LEFT );

		rcText.left--;
		rcText.top--;
		rcText.right--;
		rcText.bottom--;
		::SetTextColor( hdc, colOld );
	}
	::DrawText(
		hdc,
		pszItemStr,
		j,
		&rcText,
		DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_LEFT
	);
	::SelectObject( hdc, hFontOld  );
	::SetBkMode( hdc, nBkModeOld );

	/* �`�F�b�N��ԂȂ牚��3D�g��`�悷�� */
	if( lpdis->itemState & ODS_CHECKED ){
		/* �A�C�R�����͂ޘg */
		CSplitBoxWnd::Draw3dRect(
			hdc, lpdis->rcItem.left + 1, lpdis->rcItem.top,
			2 + 16 + 2, lpdis->rcItem.bottom - lpdis->rcItem.top,
			::GetSysColor( COLOR_3DSHADOW ),
			::GetSysColor( COLOR_3DHILIGHT )
		);
		/* �A�C�e�����I������Ă��Ȃ��ꍇ��3D�g�̒��𖾂邭�h��Ԃ� */
		if( lpdis->itemState & ODS_SELECTED ){
		}else{
//			HBRUSH hbr = ::CreateSolidBrush( ::GetSysColor( COLOR_3DHILIGHT ) );
//			HBRUSH hbr = ::CreateSolidBrush( ::GetSysColor( COLOR_3DLIGHT ) );
			HBRUSH hbr = ::GetSysColorBrush( COLOR_3DLIGHT );
			HBRUSH hbrOld = (HBRUSH)::SelectObject( hdc, hbr );
			RECT rc;
			::SetRect( &rc, lpdis->rcItem.left + 1 + 1, lpdis->rcItem.top + 1, lpdis->rcItem.left + 1 + 1 + 16 + 2, lpdis->rcItem.top + 1+ 15 + 2 );
			::FillRect( hdc, &rc, hbr );
			::SelectObject( hdc, hbrOld );
//			::DeleteObject( hbr );
		}
	}


	/* �@�\�̉摜�����݂���Ȃ烁�j���[�A�C�R��?��`�悷�� */
	if( -1 != m_nMenuItemBitmapIdxArr[nItemIndex] ){
		/* 3D�g��`�悷�� */
		/* �A�C�e�����I������Ă��� */
		if( lpdis->itemState & ODS_SELECTED ){
			/* �A�C�e�����g�p�s�� */
			if( lpdis->itemState & ODS_DISABLED /*&& !(lpdis->itemState & ODS_SELECTED)*/  ){
			}else{
				if( lpdis->itemState & ODS_CHECKED ){
				}else{
					/* �A�C�R�����͂ޘg */
					CSplitBoxWnd::Draw3dRect(
						hdc, lpdis->rcItem.left + 1, lpdis->rcItem.top,
						2 + 16 + 2, lpdis->rcItem.bottom - lpdis->rcItem.top,
						::GetSysColor( COLOR_3DHILIGHT ),
						::GetSysColor( COLOR_3DSHADOW )
					 );
				}
			}
		}

		/* �A�C�e�����g�p�s�� */
		if( lpdis->itemState & ODS_DISABLED ){
			/* �W�F�A�C�R�� */
			// 2003.09.04 Moca SetTextColor����K�v�͖���
//			COLORREF cOld;
//			cOld = SetTextColor( hdc, GetSysColor(COLOR_3DSHADOW) );	//Oct. 24, 2000 ����͕W���ł�RGB(128,128,128)�Ɠ���
//			cOld = SetTextColor( hdc, RGB(132,132,132) );	//Oct. 24, 2000 JEPRO ����������������
			m_pcIcons->Draw( m_nMenuItemBitmapIdxArr[nItemIndex],
				hdc,	//	Target DC
				lpdis->rcItem.left + 1,	//	X
				//@@@ 2002.1.29 YAZAKI Windows�̐ݒ�Ń��j���[�̃t�H���g��傫������ƕ\�����������ɑΏ�
				//lpdis->rcItem.top + 1,		//	Y
				lpdis->rcItem.top + GetSystemMetrics(SM_CYMENU)/2 - 8,	//	Y
				ILD_MASK
			);
//			SetTextColor( hdc, cOld );

		}else{
/*
			COLORREF colBk;
			if( lpdis->itemState & ODS_CHECKED && !( lpdis->itemState & ODS_SELECTED ) ){
				colBk = ::GetSysColor( COLOR_3DLIGHT );
//				colBk = ::GetSysColor( COLOR_3DHILIGHT );
			}else{
				colBk = ::GetSysColor( COLOR_MENU );
			}
*/
			/* �ʏ�̃A�C�R�� */
			m_pcIcons->Draw( m_nMenuItemBitmapIdxArr[nItemIndex],
				hdc,	//	Target DC
				lpdis->rcItem.left + 1 + 1,	//	X
				//@@@ 2002.1.29 YAZAKI Windows�̐ݒ�Ń��j���[�̃t�H���g��傫������ƕ\�����������ɑΏ�
				lpdis->rcItem.top + GetSystemMetrics(SM_CYMENU)/2 - 8,	//	Y
				//lpdis->rcItem.top + 1,		//	Y
				ILD_NORMAL
			);
		}

	}else{
		if( lpdis->itemState & ODS_CHECKED ){
			/* �A�C�R�����Ȃ��`�F�b�N�}�[�N�̕\�� */
			int nX, nY;
			nX = lpdis->rcItem.left + 1 + 8;
			nY = lpdis->rcItem.top + 8;
			::MoveToEx( hdc, nX, nY, NULL );
			::LineTo( hdc, nX + 1, nY + 1 );
			::LineTo( hdc, nX + 4, nY - 2 );

			nY++;
			::MoveToEx( hdc, nX, nY, NULL );
			::LineTo( hdc, nX + 1, nY + 1 );
			::LineTo( hdc, nX + 4, nY - 2 );

		}
	}

	return;
}


/** �R�}���h�R�[�h����c�[���o�[�{�^������INDEX�𓾂�

	@param idCommand [in] �R�}���h�R�[�h
	@param bOnlyFunc [in] �L���ȋ@�\�͈̔͂Ō�������

	@retval �݂���Ȃ����-1��Ԃ��B

	@date 2005.08.09 aroka m_nMyButtonNum�B���̂��ߒǉ�
	@date 2005.11.02 ryoji bOnlyFunc�p�����[�^��ǉ�
 */
int CMenuDrawer::FindIndexFromCommandId( int idCommand, bool bOnlyFunc )
{
	if( bOnlyFunc ){
		// �@�\�͈̔͊O�i�Z�p���[�^��܂�Ԃ��ȂǓ��ʂȂ��́j�͏��O����
		if ( !( F_MENU_FIRST <= idCommand && idCommand < F_MENU_NOT_USED_FIRST )
			&& !( F_PLUGCOMMAND_FIRST <= idCommand && idCommand < F_PLUGCOMMAND_LAST )){
			return -1;
		}
	}

	int nIndex = -1;
	for( int i = 0; i < m_nMyButtonNum; i++ ){
		if( m_tbMyButton[i].idCommand == idCommand ){
			nIndex = i;
			break;
		}
	}

	return nIndex;
}

/** �C���f�b�N�X����{�^�����𓾂�

	@param index [in] �{�^�����̃C���f�b�N�X
	@retval �{�^�����

	@date 2007.11.02 ryoji �͈͊O�̏ꍇ�͖���`�̃{�^������Ԃ��悤��
 */
TBBUTTON CMenuDrawer::getButton( int index ) const
{
	if( 0 <= index && index < m_nMyButtonNum )
		return m_tbMyButton[index];

	// �͈͊O�Ȃ疢��`�̃{�^�������쐬���ĕԂ�
	// �isakura.ini�ɔ͈͊O�C���f�b�N�X���w�肪�������ꍇ�ȂǁA���S���̂��߁j
	TBBUTTON tbb;
	SetTBBUTTONVal( &tbb, -1, F_DISABLE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 );
	return tbb;
}

int CMenuDrawer::Find( int nFuncID )
{
	int i;
	for( i = 0; i < m_nMenuItemNum; ++i ){
		if( (int)nFuncID == m_nMenuItemFuncArr[i] ){
			break;
		}
	}
	if( i >= m_nMenuItemNum ){
		return -1;
	}else{
		return i;
	}
}


const TCHAR* CMenuDrawer::GetLabel( int nFuncID )
{
	int i;
	if( -1 == ( i = Find( nFuncID ) ) ){
		return NULL;
	}
	return m_cmemMenuItemStrArr[i].GetStringPtr();
}

TCHAR CMenuDrawer::GetAccelCharFromLabel( const TCHAR* pszLabel )
{
	int i;
	for( i = 0; i + 1 < (int)_tcslen( pszLabel ); ++i ){
		if( _T('&') == pszLabel[i] ){
			if( _T('&') == pszLabel[i + 1]  ){
				i++;
			}else{
				return _totupper( pszLabel[i + 1] );
			}
		}
	}
	return _T('\0');
}





/*! ���j���[�A�N�Z�X�L�[�������̏���(WM_MENUCHAR����) */
LRESULT CMenuDrawer::OnMenuChar( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	TCHAR				chUser;
	UINT				fuFlag;
	HMENU				hmenu;
	int i;
	chUser = (TCHAR) LOWORD(wParam);	// character code
	fuFlag = (UINT) HIWORD(wParam);		// menu flag
	hmenu = (HMENU) lParam;				// handle to menu
//	MYTRACE_A( "::GetMenuItemCount( %xh )==%d\n", hmenu, ::GetMenuItemCount( hmenu ) );

	//	Oct. 27, 2000 genta
	if( 0 <= chUser && chUser < ' '){
		chUser += '@';
	}
	else {
		chUser = _totupper( chUser );
	}

	struct WorkData{
		int				idx;
		MENUITEMINFO	mii;
	};

	WorkData vecAccel[100];
	int nAccelNum;
	int nAccelSel;
	nAccelNum = 0;
	nAccelSel = 99999;
	for( i = 0; i < ::GetMenuItemCount( hmenu ); i++ ){
		TCHAR	szText[1024];
		// ���j���[���ڂɊւ�������擾���܂��B
		MENUITEMINFO		mii;
		memset_raw( &mii, 0, sizeof( mii ) );

		mii.cbSize = SIZEOF_MENUITEMINFO; //Win95�΍�ς݂�sizeof(MENUITEMINFO)�l

		mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
		mii.fType = MFT_STRING;
		_tcscpy( szText, _T("--unknown--") );
		mii.dwTypeData = szText;
		mii.cch = _countof( szText ) - 1;
		if( 0 == ::GetMenuItemInfo( hmenu, i, TRUE, &mii ) ){
			continue;
		}
		const TCHAR* pszLabel;
		if( NULL == ( pszLabel = GetLabel( mii.wID ) ) ){
			continue;
		}
		if( chUser == GetAccelCharFromLabel( pszLabel ) ){
			vecAccel[nAccelNum].idx = i;
			vecAccel[nAccelNum].mii = mii;
			if( /*-1 == nAccelSel ||*/ MFS_HILITE & mii.fState ){
				nAccelSel = nAccelNum;
			}
			nAccelNum++;
		}
	}
//	MYTRACE_A( "%d\n", (int)mapAccel.size() );
	if( 0 == nAccelNum ){
		return  MAKELONG( 0, MNC_IGNORE );
	}
	if( 1 == nAccelNum ){
		return  MAKELONG( vecAccel[0].idx, MNC_EXECUTE );
	}
//	MYTRACE_A( "nAccelSel=%d nAccelNum=%d\n", nAccelSel, nAccelNum );
	if( nAccelSel + 1 >= nAccelNum ){
//		MYTRACE_A( "vecAccel[0].idx=%d\n", vecAccel[0].idx );
		return  MAKELONG( vecAccel[0].idx, MNC_SELECT );
	}else{
//		MYTRACE_A( "vecAccel[nAccelSel + 1].idx=%d\n", vecAccel[nAccelSel + 1].idx );
		return  MAKELONG( vecAccel[nAccelSel + 1].idx, MNC_SELECT );
	}
}

//	Jul. 21, 2003 genta
//	�R�����g�A�E�g����Ă����������폜 (CImageList�ōė��p)


/* TBBUTTON�\���̂Ƀf�[�^���Z�b�g */
void CMenuDrawer::SetTBBUTTONVal(
	TBBUTTON*	ptb,
	int			iBitmap,
	int			idCommand,
	BYTE		fsState,
	BYTE		fsStyle,
	DWORD_PTR	dwData,
	INT_PTR		iString
) const
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

//�c�[���o�[�{�^����ǉ�����
void CMenuDrawer::AddToolButton( int iBitmap, int iCommand )
{
	TBBUTTON tbb;

	SetTBBUTTONVal( &tbb, iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 );

	//�Ōォ��Q�Ԗڂɑ}������B��ԍŌ�͔ԕ��ŌŒ�B
	m_tbMyButton.insert( m_tbMyButton.end() - 1, tbb );
	m_nMyButtonNum++;
}
