//	$Id$
/*!	@file
	@brief �c�[���o�[�f�[�^�̏�����

	@author Norio Nakatani, Jepro
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CShareData.h"
#include "global.h"

CShareData::CShareData()
{
	m_pszAppName = GSTR_CSHAREDATA;
	m_hFileMap   = NULL;
	m_pShareData = NULL;
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
	SetTBBUTTONVal( &m_tbMyButton[0], 0, 0, 0, TBSTYLE_SEP, 0, 0 );		//�Z�p���[�^
	struct TBUTTONDATA {
		int			idCommand;
		BYTE		fsState;
		BYTE		fsStyle;
		DWORD		dwData;
		int			iString;
	};
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
//					+ �J�X�^�����j���[	(10�i��32��: 289-320) */
//		�E�B���h�E--- �E�B���h�E�n		(11�i��22��: 321-342)
//		�w���v------- �x��				(11�i�ڎc���10��: 343-352)
//			�����̑�					(12�i��32��: 353-384)
//	��1.�u�}���n�v�̓��j���[�ł́u�ҏW�v�ɓ����Ă���
//	��2.�u���̑��v�̓��j���[�ɂ͓����Ă��Ȃ����̂����� (���݉����Ȃ��̂�12�i�ڂ�ݒ肵�ĂȂ�)
//	��3.�u�R�}���h�ꗗ�v�Ŋ����ďd�����Ă���Ă���R�}���h�͂��́u�{�Ɓv�̕��ɔz�u����
//	��4.�u�R�}���h�ꗗ�v�ɓ����ĂȂ��R�}���h���킩���Ă���͈͂ňʒu�\��ɂ��Ă�����
//  ��5. F_DISABLE �͖���`�p(�_�~�[�Ƃ��Ă��g��)
//	��6. ���[�U�[�p�Ɋm�ۂ��ꂽ�ꏊ�͓��ɂȂ��̂Ŋe�i�̋󂢂Ă�����̕����g���Ă��������B

	TBUTTONDATA tbd[] = {
/* �t�@�C������n(1�i��32��: 1-32) */
/*  1 */		F_FILENEW					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�V�K�쐬
/*  2 */		F_FILEOPEN					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J��
/*  3 */		F_FILESAVE					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�㏑���ۑ�
/*  4 */		F_FILESAVEAS				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���O��t���ĕۑ�	//Sept. 18, 2000 JEPRO �ǉ�
/*  5 */		F_FILECLOSE					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
/*  6 */		F_FILECLOSE_OPEN			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���ĊJ��
/*  7 */		F_FILE_REOPEN_SJIS			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//SJIS�ŊJ������
/*  8 */		F_FILE_REOPEN_JIS			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//JIS�ŊJ������
/*  9 */		F_FILE_REOPEN_EUC			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//EUC�ŊJ������
/* 10 */		F_FILE_REOPEN_UNICODE		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//Unicode�ŊJ������
/* 11 */		F_FILE_REOPEN_UTF8			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//UTF-8�ŊJ������
/* 12 */		F_FILE_REOPEN_UTF7			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//UTF-7�ŊJ������
/* 13 */		F_PRINT						, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���
/* 14 */		F_PRINT_PREVIEW				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//����v���r���[
/* 15 */		F_PRINT_PAGESETUP			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//����y�[�W�ݒ�	//Sept. 21, 2000 JEPRO �ǉ�
/* 16 */		F_OPEN_HfromtoC				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
/* 17 */		F_OPEN_HHPP					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
/* 18 */		F_OPEN_CCPP					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
/* 19 */		F_ACTIVATE_SQLPLUS			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//Oracle SQL*Plus���A�N�e�B�u�\�� */	//Sept. 20, 2000 JEPRO �ǉ�
/* 20 */		F_PLSQL_COMPILE_ON_SQLPLUS	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//Oracle SQL*Plus�Ŏ��s */	//Sept. 17, 2000 jepro �����́u�R���p�C���v���u���s�v�ɓ���
/* 21 */		F_BROWSE					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�u���E�Y
/* 22 */		F_PROPERTY_FILE				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�t�@�C���̃v���p�e�B//Sept. 16, 2000 JEPRO mytool1.bmp�ɂ������u�t�@�C���̃v���p�e�B�v�A�C�R����IDB_MYTOOL�ɃR�s�[
/* 23 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 24 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 25 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 26 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 27 */		F_EXITALL					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�T�N���G�f�B�^�̑S�I��	//Dec. 27, 2000 JEPRO �ǉ�
/* 28 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 29 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 30 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 31 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 32 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �ҏW�n(2�i��32��: 32-64) */
/* 33 */		F_UNDO				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���ɖ߂�(Undo)
/* 34 */		F_REDO				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//��蒼��(Redo)
/* 35 */		F_DELETE			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�폜
/* 36 */		F_DELETE_BACK		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�[�\���O���폜
/* 37 */		F_WordDeleteToStart	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�P��̍��[�܂ō폜
/* 38 */		F_WordDeleteToEnd	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�P��̉E�[�܂ō폜
/* 39 */		F_WordDelete		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�P��폜
/* 40 */		F_WordCut			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�P��؂���
/* 41 */		F_LineDeleteToStart	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�s���܂ō폜(���s�P��)
/* 42 */		F_LineDeleteToEnd	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�s���܂ō폜(���s�P��)
/* 43 */		F_LineCutToStart	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�s���܂Ő؂���(���s�P��)
/* 44 */		F_LineCutToEnd		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�s���܂Ő؂���(���s�P��)
/* 45 */		F_DELETE_LINE		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�s�폜(�܂�Ԃ��P��)
/* 46 */		F_CUT_LINE			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�s�؂���(���s�P��)
/* 47 */		F_DUPLICATELINE		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�s�̓�d��(�܂�Ԃ��P��)
/* 48 */		F_INDENT_TAB		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//TAB�C���f���g
/* 49 */		F_UNINDENT_TAB		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�tTAB�C���f���g
/* 50 */		F_INDENT_SPACE		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//SPACE�C���f���g
/* 51 */		F_UNINDENT_SPACE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�tSPACE�C���f���g
/* 52 */		F_DISABLE/*F_WORDSREFERENCE*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�P�ꃊ�t�@�����X	//�A�C�R������
/* 53 */		F_LTRIM				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//LTRIM		// 2001.12.03 hor
/* 54 */		F_RTRIM				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//RTRIM		// 2001.12.03 hor
/* 55 */		F_SORT_ASC			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//SORT_ASC	// 2001.12.06 hor
/* 56 */		F_SORT_DESC			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//SORT_DES	// 2001.12.06 hor
/* 57 */		F_MARGE				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//MARGE		// 2001.12.06 hor
/* 58 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 59 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 60 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 61 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 62 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 63 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 64 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �J�[�\���ړ��n(3�i��32��: 65-96) */
/* 65 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 66 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 67 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 68 */		F_UP			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�[�\����ړ�
/* 69 */		F_DOWN			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�[�\�����ړ�
/* 70 */		F_LEFT			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�[�\�����ړ�
/* 71 */		F_RIGHT			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�[�\���E�ړ�
/* 72 */		F_UP2			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�[�\����ړ�(�Q�s����)
/* 73 */		F_DOWN2			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�[�\�����ړ�(�Q�s����)
/* 74 */		F_WORDLEFT		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�P��̍��[�Ɉړ�
/* 75 */		F_WORDRIGHT		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�P��̉E�[�Ɉړ�
/* 76 */		F_GOLINETOP		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�s���Ɉړ�(�܂�Ԃ��P��)
/* 77 */		F_GOLINEEND		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�s���Ɉړ�(�܂�Ԃ��P��)
/* 78 */		F_HalfPageUp	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
/* 79 */		F_HalfPageDown	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
/* 80 */		F_1PageUp		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
/* 81 */		F_1PageDown		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
/* 82 */		F_DISABLE/*F_DISPLAYTOP*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//��ʂ̐擪�Ɉړ�(������)
/* 83 */		F_DISABLE/*F_DISPLAYEND*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//��ʂ̍Ō�Ɉړ�(������)
/* 84 */		F_GOFILETOP		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�t�@�C���̐擪�Ɉړ�
/* 85 */		F_GOFILEEND		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�t�@�C���̍Ō�Ɉړ�
/* 86 */		F_CURLINECENTER	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�[�\���s���E�B���h�E������
/* 87 */		F_JUMPPREV		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�ړ�����: �O��	//Sept. 28, 2000 JEPRO �ǉ�
/* 88 */		F_JUMPNEXT		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�ړ�����: ����	//Sept. 28, 2000 JEPRO �ǉ�
/* 89 */		F_WndScrollDown	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�e�L�X�g���P�s���փX�N���[��	//Jun. 28, 2001 JEPRO �ǉ�
/* 90 */		F_WndScrollUp	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�e�L�X�g���P�s��փX�N���[��	//Jun. 28, 2001 JEPRO �ǉ�
/* 91 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 92 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 93 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 94 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 95 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 96 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �I���n(4�i��32��: 97-128) */
/* 97 */		F_SELECTWORD		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���݈ʒu�̒P��I��
/* 98 */		F_SELECTALL			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���ׂđI��	//Sept. 21, 2000 JEPRO �ǉ�
/* 99 */		F_BEGIN_SEL			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�͈͑I���J�n
/* 100 */		F_UP_SEL			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�J�[�\����ړ�
/* 101 */		F_DOWN_SEL			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�J�[�\�����ړ�
/* 102 */		F_LEFT_SEL			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�J�[�\�����ړ�
/* 103 */		F_RIGHT_SEL			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�J�[�\���E�ړ�
/* 104 */		F_UP2_SEL			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�J�[�\����ړ�(�Q�s����)
/* 105 */		F_DOWN2_SEL			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
/* 106 */		F_WORDLEFT_SEL		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�P��̍��[�Ɉړ�
/* 107 */		F_WORDRIGHT_SEL		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�P��̉E�[�Ɉړ�
/* 108 */		F_GOLINETOP_SEL		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
/* 109 */		F_GOLINEEND_SEL		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
/* 110 */		F_HalfPageUp_Sel	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
/* 111 */		F_HalfPageDown_Sel	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
/* 112 */		F_1PageUp_Sel		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
/* 113 */		F_1PageDown_Sel		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
/* 114 */		F_DISABLE/*F_DISPLAYTOP_SEL*/, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)��ʂ̐擪�Ɉړ�(������)
/* 115 */		F_DISABLE/*F_DISPLAYEND_SEL*/, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)��ʂ̍Ō�Ɉړ�(������)
/* 116 */		F_GOFILETOP_SEL		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�t�@�C���̐擪�Ɉړ�
/* 117 */		F_GOFILEEND_SEL		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(�͈͑I��)�t�@�C���̍Ō�Ɉړ�
/* 118 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 119 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 120 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 121 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 122 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 123 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 124 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 125 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 126 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 127 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 128 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* ��`�I���n(5�i��32��: 129-160) */ //(��. ��`�I���n�̂قƂ�ǂ͖�����)
/* 129 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 130 */		F_DISABLE/*F_BOXSELALL*/		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//��`�ł��ׂđI��
/* 131 */		F_BEGIN_BOX						, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//��`�͈͑I���J�n	//Sept. 29, 2000 JEPRO �ǉ�
/* 132 */		F_DISABLE/*F_UP_BOX*/			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�J�[�\����ړ�
/* 133 */		F_DISABLE/*F_DOWN_BOX*/			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�J�[�\�����ړ�
/* 134 */		F_DISABLE/*F_LEFT_BOX*/			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�J�[�\�����ړ�
/* 135 */		F_DISABLE/*F_RIGHT_BOX*/		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�J�[�\���E�ړ�
/* 136 */		F_DISABLE/*F_UP2_BOX*/			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�J�[�\����ړ�(�Q�s����)
/* 137 */		F_DISABLE/*F_DOWN2_BOX*/		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�J�[�\�����ړ�(�Q�s����)
/* 138 */		F_DISABLE/*F_WORDLEFT_BOX*/		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�P��̍��[�Ɉړ�
/* 139 */		F_DISABLE/*F_WORDRIGHT_BOX*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�P��̉E�[�Ɉړ�
/* 140 */		F_DISABLE/*F_GOLINETOP_BOX*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
/* 141 */		F_DISABLE/*F_GOLINEEND_BOX*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
/* 142 */		F_DISABLE/*F_HalfPageUp_Box*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)���y�[�W�A�b�v
/* 143 */		F_DISABLE/*F_HalfPageDown_Box*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)���y�[�W�_�E��
/* 144 */		F_DISABLE/*F_1PageUp_Box*/		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�P�y�[�W�A�b�v
/* 145 */		F_DISABLE/*F_1PageDown_Box*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�P�y�[�W�_�E��
/* 146 */		F_DISABLE/*F_DISABLE/*F_DISPLAYTOP_BOX*/, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)��ʂ̐擪�Ɉړ�(������)
/* 147 */		F_DISABLE/*F_DISPLAYEND_BOX*/, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)��ʂ̍Ō�Ɉړ�(������)
/* 148 */		F_DISABLE/*F_GOFILETOP_BOX*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�t�@�C���̐擪�Ɉړ�
/* 149 */		F_DISABLE/*F_GOFILEEND_BOX*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//(��`�I��)�t�@�C���̍Ō�Ɉړ�
/* 150 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 151 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 152 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 153 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 154 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 155 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 156 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 157 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 158 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 159 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 160 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �N���b�v�{�[�h�n(6�i��24��: 161-184) */
/* 161 */		F_CUT						, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
/* 162 */		F_COPY						, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
/* 163 */		F_COPY_CRLF					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//CRLF���s�ŃR�s�[
/* 164 */		F_PASTE						, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�\��t��(�N���b�v�{�[�h����\��t��)
/* 165 */		F_PASTEBOX					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//��`�\��t��(�N���b�v�{�[�h����\��t��)
/* 166 */		F_DISABLE/*F_INSTEXT*/		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�e�L�X�g��\��t��	(�����J�R�}���h�H�������H)
/* 167 */		F_DISABLE/*F_ADDTAIL*/		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�Ō�Ƀe�L�X�g��ǉ�	(�����J�R�}���h�H�������H)
/* 168 */		F_COPYLINES					, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�I��͈͓��S�s�R�s�[	//Sept. 30, 2000 JEPRO �ǉ�
/* 169 */		F_COPYLINESASPASSAGE		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�I��͈͓��S�s���p���t���R�s�[	//Sept. 30, 2000 JEPRO �ǉ�
/* 170 */		F_COPYLINESWITHLINENUMBER	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�I��͈͓��S�s�s�ԍ��t���R�s�[	//Sept. 30, 2000 JEPRO �ǉ�
/* 171 */		F_COPYPATH	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���̃t�@�C���̃p�X�����R�s�[ //added Oct. 22, 2000 JEPRO				//Nov. 5, 2000 JEPRO �ǉ�
/* 172 */		F_COPYTAG	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[ //added Oct. 22, 2000 JEPRO	//Nov. 5, 2000 JEPRO �ǉ�
/* 173 */		F_CREATEKEYBINDLIST			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�L�[���蓖�Ĉꗗ���R�s�[ //added Oct. 22, 2000 JEPRO	//Dec. 25, 2000 JEPRO �A�C�R���ǉ�
/* 174 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 175 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 176 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 177 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 178 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 179 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 180 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 181 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 182 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 183 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 184 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �}���n(6�i�ڎc��8��: 185-192) */
/* 185 */		F_INS_DATE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���t�}��	//Nov. 5, 2000 JEPRO �ǉ�
/* 186 */		F_INS_TIME	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�����}��	//Nov. 5, 2000 JEPRO �ǉ�
/* 187 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 188 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 189 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 190 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 191 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 192 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �ϊ��n(7�i��32��: 193-224) */
/* 193 */		F_TOLOWER				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�p�啶�����p������
/* 194 */		F_TOUPPER				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�p���������p�啶��
/* 195 */		F_TOHANKAKU				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�S�p�����p
/* 196 */		F_TOZENKAKUKATA			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���p�{�S�Ђ灨�S�p�E�J�^�J�i	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
/* 197 */		F_TOZENKAKUHIRA			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���p�{�S�J�^���S�p�E�Ђ炪��	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
/* 198 */		F_HANKATATOZENKAKUKATA	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���p�J�^�J�i���S�p�J�^�J�i	//Sept. 18, 2000 JEPRO �ǉ�
/* 199 */		F_HANKATATOZENKAKUHIRA	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���p�J�^�J�i���S�p�Ђ炪��	//Sept. 18, 2000 JEPRO �ǉ�
/* 200 */		F_TABTOSPACE			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//TAB����	//Sept. 20, 2000 JEPRO �ǉ�
/* 201 */		F_CODECNV_AUTO2SJIS		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�������ʁ�SJIS�R�[�h�ϊ�
/* 202 */		F_CODECNV_EMAIL			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//E-Mail(JIS��SIJIS)�R�[�h�ϊ�
/* 203 */		F_CODECNV_EUC2SJIS		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//EUC��SJIS�R�[�h�ϊ�
/* 204 */		F_CODECNV_UNICODE2SJIS	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//Unicode��SJIS�R�[�h�ϊ�
/* 205 */		F_CODECNV_UTF82SJIS		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//UTF-8��SJIS�R�[�h�ϊ�
/* 206 */		F_CODECNV_UTF72SJIS		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//UTF-7��SJIS�R�[�h�ϊ�
/* 207 */		F_CODECNV_SJIS2JIS		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//SJIS��JIS�R�[�h�ϊ�
/* 208 */		F_CODECNV_SJIS2EUC		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//SJIS��EUC�R�[�h�ϊ�
/* 209 */		F_CODECNV_SJIS2UTF8		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//SJIS��UTF-8�R�[�h�ϊ�
/* 210 */		F_CODECNV_SJIS2UTF7		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//SJIS��UTF-7�R�[�h�ϊ�
/* 211 */		F_BASE64DECODE			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//Base64�f�R�[�h���ĕۑ�	//Sept. 28, 2000 JEPRO �ǉ�
/* 212 */		F_UUDECODE				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//uudecode���ăt�@�C���ɕۑ�//Sept. 28, 2000 JEPRO �ǉ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX
/* 213 */		F_SPACETOTAB			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�󔒁�TAB	//Jun. 01, 2001 JEPRO �ǉ�
/* 214 */		F_TOZENEI				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���p�p�����S�p�p�� //July. 30, 2001 Misaka �ǉ�
/* 215 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 216 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 217 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 218 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 219 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 220 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 221 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 222 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 223 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 224 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �����n(8�i��32��: 225-256) */
/* 225 */		F_SEARCH_DIALOG		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//����(�P�ꌟ���_�C�A���O)
/* 226 */		F_SEARCH_NEXT		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//��������
/* 227 */		F_SEARCH_PREV		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�O������
/* 228 */		F_REPLACE			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�u��	//Sept. 21, 2000 JEPRO �ǉ�
/* 229 */		F_SEARCH_CLEARMARK	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�����}�[�N�̃N���A
/* 230 */		F_GREP				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//Grep
/* 231 */		F_JUMP				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�w��s�փW�����v		//Sept. 21, 2000 JEPRO �ǉ�
/* 232 */		F_OUTLINE			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�A�E�g���C�����
/* 233 */		F_TAGJUMP			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�^�O�W�����v�@�\			//Sept. 21, 2000 JEPRO �ǉ�
/* 234 */		F_TAGJUMPBACK		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�^�O�W�����v�o�b�N�@�\	//Sept. 21, 2000 JEPRO �ǉ�
/* 235 */		F_COMPARE			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�t�@�C�����e��r	//Sept. 21, 2000 JEPRO �ǉ�
/* 236 */		F_BRACKETPAIR		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�Ί��ʂ̌���	//Sept. 20, 2000 JEPRO �ǉ�
/* 237 */		F_BOOKMARK_SET		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�u�b�N�}�[�N�ݒ�E����	// 2001.12.03 hor
/* 238 */		F_BOOKMARK_NEXT		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���̃u�b�N�}�[�N��		// 2001.12.03 hor
/* 239 */		F_BOOKMARK_PREV		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�O�̃u�b�N�}�[�N��		// 2001.12.03 hor
/* 240 */		F_BOOKMARK_RESET	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�u�b�N�}�[�N�̑S����		// 2001.12.03 hor
/* 241 */		F_BOOKMARK_VIEW		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�u�b�N�}�[�N�̈ꗗ		// 2001.12.03 hor
/* 242 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 243 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 244 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 245 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 246 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 247 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 248 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 249 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 250 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 251 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 252 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 253 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 254 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 255 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 256 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* ���[�h�؂�ւ��n(9�i��4��: 257-260) */
/* 257 */		F_CHGMOD_INS, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�}���^�㏑�����[�h�؂�ւ�	//Nov. 5, 2000 JEPRO �ǉ�
/* 258 */		F_CANCEL_MODE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�e�탂�[�h�̎�����			//Nov. 7, 2000 JEPRO �ǉ�
/* 259 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 260 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �ݒ�n(9�i�ڎ���16��: 261-276) */
/* 261 */		F_SHOWTOOLBAR		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�c�[���o�[�̕\��
/* 262 */		F_SHOWFUNCKEY		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�t�@���N�V�����L�[�̕\��
/* 263 */		F_SHOWSTATUSBAR		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�X�e�[�^�X�o�[�̕\��
/* 264 */		F_TYPE_LIST			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�^�C�v�ʐݒ�ꗗ	//Sept. 18, 2000 JEPRO �ǉ�
/* 265 */		F_OPTION_TYPE		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�^�C�v�ʐݒ�
/* 266 */		F_OPTION			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���ʐݒ�			//Sept. 16, 2000 jepro �������u�ݒ�v���p�e�B�V�[�g�v����ύX
/* 267 */		F_FONT				, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�t�H���g�ݒ�
/* 268 */		F_WRAPWINDOWWIDTH	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���݂̃E�B���h�E���Ő܂�Ԃ�	//	Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
/* 269 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 270 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 271 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 272 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 273 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 274 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 275 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 276 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �}�N���n(9�i�ڍŌ��12��: 277-288) */
/* 277 */		F_RECKEYMACRO	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�L�[�}�N���̋L�^�J�n�^�I��
/* 278 */		F_SAVEKEYMACRO	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�L�[�}�N���̕ۑ�		//Sept. 21, 2000 JEPRO �ǉ�
/* 279 */		F_LOADKEYMACRO	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�L�[�}�N���̓ǂݍ���	//Sept. 21, 2000 JEPRO �ǉ�
/* 280 */		F_EXECKEYMACRO	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�L�[�}�N���̎��s		//Sept. 16, 2000 JEPRO �������Ɉړ�����
/* 281 */		F_EXECCOMMAND	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�O���R�}���h���s//Sept. 20, 2000 JEPRO ���̂�CMMAND����COMMAND�ɕύX(EXECCMMAND��EXECCMMAND)
/* 282 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 283 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 284 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 285 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 286 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 287 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 288 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �J�X�^�����j���[(10�i��32��: 289-320) */
/* 289 */		F_MENU_RBUTTON	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�E�N���b�N���j���[ 	//Sept. 30, 2000 JEPRO �ǉ�
/* 290 */		F_CUSTMENU_1	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[1
/* 291 */		F_CUSTMENU_2	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[2
/* 292 */		F_CUSTMENU_3	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[3
/* 293 */		F_CUSTMENU_4	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[4
/* 294 */		F_CUSTMENU_5	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[5
/* 295 */		F_CUSTMENU_6	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[6
/* 296 */		F_CUSTMENU_7	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[7
/* 297 */		F_CUSTMENU_8	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[8
/* 298 */		F_CUSTMENU_9	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[9
/* 299 */		F_CUSTMENU_10	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[10
/* 300 */		F_DISABLE/*F_CUSTMENU_11*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[11	//�A�C�R������
/* 301 */		F_DISABLE/*F_CUSTMENU_12*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[12	//�A�C�R������
/* 302 */		F_DISABLE/*F_CUSTMENU_13*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[13	//�A�C�R������
/* 303 */		F_DISABLE/*F_CUSTMENU_14*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[14	//�A�C�R������
/* 304 */		F_DISABLE/*F_CUSTMENU_15*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[15	//�A�C�R������
/* 305 */		F_DISABLE/*F_CUSTMENU_16*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[16	//�A�C�R������
/* 306 */		F_DISABLE/*F_CUSTMENU_17*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[17	//�A�C�R������
/* 307 */		F_DISABLE/*F_CUSTMENU_18*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[18	//�A�C�R������
/* 308 */		F_DISABLE/*F_CUSTMENU_19*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[19	//�A�C�R������
/* 309 */		F_DISABLE/*F_CUSTMENU_20*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[20	//�A�C�R������
/* 310 */		F_DISABLE/*F_CUSTMENU_21*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[21	//�A�C�R������
/* 311 */		F_DISABLE/*F_CUSTMENU_22*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[22	//�A�C�R������
/* 312 */		F_DISABLE/*F_CUSTMENU_23*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[23	//�A�C�R������
/* 313 */		F_DISABLE/*F_CUSTMENU_24*/	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�J�X�^�����j���[24	//�A�C�R������
/* 314 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 315 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 316 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 317 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 318 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 319 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 320 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �E�B���h�E�n(11�i��22��: 321-342) */
/* 321 */		F_SPLIT_V		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�㉺�ɕ���	//Sept. 16, 2000 jepro �������u�c�v����u�㉺�Ɂv�ɕύX
/* 322 */		F_SPLIT_H		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���E�ɕ���	//Sept. 16, 2000 jepro �������u���v����u���E�Ɂv�ɕύX
/* 323 */		F_SPLIT_VH		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
/* 324 */		F_WINCLOSE		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�E�B���h�E�����
/* 325 */		F_WIN_CLOSEALL	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���ׂẴE�B���h�E�����	//Sept. 18, 2000 JEPRO �ǉ�	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
/* 329 */		F_NEXTWINDOW	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���̃E�B���h�E
/* 330 */		F_PREVWINDOW	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�O�̃E�B���h�E
/* 326 */		F_CASCADE		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�d�˂ĕ\��
/* 237 */		F_TILE_V		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�㉺�ɕ��ׂĕ\��
/* 328 */		F_TILE_H		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���E�ɕ��ׂĕ\��
/* 331 */		F_MAXIMIZE_V	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�c�����ɍő剻
/* 332 */		F_MAXIMIZE_H	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�������ɍő剻 //2001.02.10 by MIK
/* 333 */		F_MINIMIZE_ALL	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���ׂčŏ���					//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
/* 334 */		F_REDRAW		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�ĕ`��						//Sept. 30, 2000 JEPRO �ǉ�
/* 335 */		F_WIN_OUTPUT	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�A�E�g�v�b�g�E�B���h�E�\��	//Sept. 18, 2000 JEPRO �ǉ�
/* 336 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 337 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 338 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 339 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 340 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 341 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 342 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* �x��(11�i�ڎc���10��: 343-352) */
/* 343 */		F_HOKAN			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���͕⊮
/* 344 */		F_HELP_CONTENTS , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�w���v�ڎ�			//Nov. 25, 2000 JEPRO �ǉ�
/* 345 */		F_HELP_SEARCH	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�w���v�L�[���[�h����	//Nov. 25, 2000 JEPRO �ǉ�
/* 346 */		F_MENU_ALLFUNC	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�R�}���h�ꗗ			//Sept. 30, 2000 JEPRO �ǉ�
/* 347 */		F_EXTHELP1		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�O���w���v�P
/* 348 */		F_EXTHTMLHELP	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�O��HTML�w���v
/* 349 */		F_ABOUT			, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�o�[�W�������	//Dec. 24, 2000 JEPRO �ǉ�
/* 350 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 351 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 352 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[

/* ���̑�(12�i��32��: 353-384) */
///* 353 */		F_SENDMAIL		, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//���[�����M	//Oct. 22, 2000 JEPRO (���������[���@�\�͎���ł���)
/* 354 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[(�u????�v�p�\��)	//Sept. 20, 2000 JEPRO �_�~�[�Ƃ���F_DISABLE���g����̂𔭌��I
/* 355 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 356 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 357 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 358 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 359 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 360 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 361 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 362 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 363 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 364 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 365 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 366 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 367 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 368 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 369 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 370 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 371 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 372 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 373 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 374 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 375 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 376 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 377 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 378 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 379 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 380 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 381 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 382 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 383 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0,	//�_�~�[
/* 384 */		F_DISABLE	, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0	//�ŏI�s�p�_�~�[(Jepro note: �ŏI�s���ɂ̓J���}��t���Ȃ�����)

};
	int tbd_num = sizeof( tbd ) / sizeof( tbd[0] );
	for( int i = 0; i < tbd_num; i++ ){
		SetTBBUTTONVal(
			&m_tbMyButton[i+1],
			i,
			tbd[i].idCommand,
			tbd[i].fsState,
			tbd[i].fsStyle,
			tbd[i].dwData,
			tbd[i].iString
		);
	}
	m_nMyButtonNum = tbd_num + 1;
	return;
}


/*[EOF]*/
