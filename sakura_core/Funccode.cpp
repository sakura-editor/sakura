//	$Id$
/*!	@file
	@brief �@�\���ޒ�`

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//	Sept. 14, 2000 Jepro note: functions & commands list
//	�L�[���[�h�F�R�}���h�ꗗ����
//	�����ɓo�^����Ă���R�}���h�����ʐݒ�̋@�\��ʂɕ\������A�L�[���蓖�Ăɂ��ݒ�ł���悤�ɂȂ�
//	���̃t�@�C���́u�R�}���h�ꗗ�v�̃��j���[�̏��Ԃ�\���ɂ��g���Ă���
//	sakura_rc.rc�t�@�C���̉��̂ق��ɂ���String Table���Q�Ƃ̂���

#include "funccode.h"

//using namespace nsFuncCode;

const char* nsFuncCode::ppszFuncKind[] = {
//	"--����`--",	//Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɕύX
//	Oct. 16, 2000 JEPRO �\���̏��Ԃ����j���[�o�[�̂���ɍ��킹��悤�ɏ�������ւ���(���̌ʂ̂��̂��S��)
	"�t�@�C������n",
	"�ҏW�n",
	"�J�[�\���ړ��n",
	"�I���n",		//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v�������Ȃ����̂Łu�I���n�v�Ƃ��ēƗ���(�T�u���j���[���͍\����ł��Ȃ��̂�)
	"��`�I���n",	//Oct. 17, 2000 JEPRO �u�I���n�v�Ɉꏏ�ɂ���Ƒ����Ȃ肷����̂Łu��`�I���n�v���Ɨ�������
	"�N���b�v�{�[�h�n",
	"�}���n",
	"�ϊ��n",
	"�����n",
	"���[�h�؂�ւ��n",
	"�ݒ�n",
	"�}�N���n",
	//	Oct. 15, 2001 genta �J�X�^�����j���[�̕�������͓��I�ɕύX�\�ɂ��邽�߂�������͊O���D
//	"�J�X�^�����j���[",	//Oct. 21, 2000 JEPRO �u���̑��v����Ɨ�������
	"�E�B���h�E�n",
	"�x��",
	"���̑�"
};
const int nsFuncCode::nFuncKindNum = sizeof(nsFuncCode::ppszFuncKind) / sizeof(nsFuncCode::ppszFuncKind[0]);

//	From Here Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�Ɉȉ���4�s���R�����g�A�E�g�ɕύX
//const int pnFuncList_Undef[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List0��List_Undef)
//	0
//};
//int	nFincList_Undef_Num = sizeof( pnFuncList_Undef ) / sizeof( pnFuncList_Undef[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List0��List_Undef)
//	To Here Oct. 14, 2000


/* �t�@�C������n */
const int pnFuncList_File[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List5��List_File)
	F_FILENEW			,	//�V�K�쐬
	F_FILEOPEN			,	//�J��
	F_FILESAVE			,	//�㏑���ۑ�
	F_FILESAVEAS_DIALOG	,	//���O��t���ĕۑ�
	F_FILECLOSE			,	//����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
	F_FILECLOSE_OPEN	,	//���ĊJ��
	F_WINCLOSE			,	//�E�B���h�E�����	//Oct.17,2000 �R�}���h�{�Ƃ́u�E�B���h�E�n�v	//Feb. 18, 2001	JEPRO ������ړ�����
	F_FILE_REOPEN_SJIS		,//SJIS�ŊJ������
	F_FILE_REOPEN_JIS		,//JIS�ŊJ������
	F_FILE_REOPEN_EUC		,//EUC�ŊJ������
	F_FILE_REOPEN_UNICODE	,//Unicode�ŊJ������
	F_FILE_REOPEN_UTF8		,//UTF-8�ŊJ������
	F_FILE_REOPEN_UTF7		,//UTF-7�ŊJ������
	F_PRINT				,	//���
	F_PRINT_PREVIEW		,	//����v���r���[
	F_PRINT_PAGESETUP	,	//����y�[�W�ݒ�	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����ύX
	F_OPEN_HfromtoC		,	//������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
	F_OPEN_HHPP			,	//������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
	F_OPEN_CCPP			,	//������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
	F_ACTIVATE_SQLPLUS			,	/* Oracle SQL*Plus���A�N�e�B�u�\�� */	//Sept. 20, 2000 �u�R���p�C���vJEPRO �A�N�e�B�u�\������Ɉړ�����
	F_PLSQL_COMPILE_ON_SQLPLUS	,	/* Oracle SQL*Plus�Ŏ��s */	//Sept. 20, 2000 jepro �����́u�R���p�C���v���u���s�v�ɓ���
	F_BROWSE			,	//�u���E�Y
	F_READONLY			,	//�ǂݎ���p
	F_PROPERTY_FILE		,	/* �t�@�C���̃v���p�e�B */
	F_WIN_CLOSEALL		,	//���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)	//Oct.17,2000 �R�}���h�{�Ƃ́u�E�B���h�E�n�v	//Feb. 18, 2001	JEPRO �ォ��ړ�����
	F_EXITALL				//�T�N���G�f�B�^�̑S�I��	//Dec. 27, 2000 JEPRO �ǉ�
};
const int nFincList_File_Num = sizeof( pnFuncList_File ) / sizeof( pnFuncList_File[0] );	//Oct. 16, 2000 JEPRO �z�񖼕ύX(FuncList5��FuncList_File)


/* �ҏW�n */
const int pnFuncList_Edit[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List3��List_Edit)
	F_UNDO				,	//���ɖ߂�(Undo)
	F_REDO				,	//��蒼��(Redo)
	F_DELETE			,	//�폜
	F_DELETE_BACK		,	//�J�[�\���O���폜
	F_WordDeleteToStart	,	//�P��̍��[�܂ō폜
	F_WordDeleteToEnd	,	//�P��̉E�[�܂ō폜
	F_WordCut			,	//�P��؂���
	F_WordDelete		,	//�P��폜
	F_LineCutToStart	,	//�s���܂Ő؂���(���s�P��)
	F_LineCutToEnd		,	//�s���܂Ő؂���(���s�P��)
	F_LineDeleteToStart	,	//�s���܂ō폜(���s�P��)
	F_LineDeleteToEnd	,	//�s���܂ō폜(���s�P��)
	F_CUT_LINE			,	//�s�؂���(�܂�Ԃ��P��)
	F_DELETE_LINE		,	//�s�폜(�܂�Ԃ��P��)
	F_DUPLICATELINE		,	//�s�̓�d��(�܂�Ԃ��P��)
	F_INDENT_TAB		,	//TAB�C���f���g
	F_UNINDENT_TAB		,	//�tTAB�C���f���g
	F_INDENT_SPACE		,	//SPACE�C���f���g
	F_UNINDENT_SPACE	,	//�tSPACE�C���f���g
	F_LTRIM				,	//��(�擪)�̋󔒂��폜	2001.12.03 hor
	F_RTRIM				,	//�E(����)�̋󔒂��폜	2001.12.03 hor
	F_SORT_ASC			,	//�I���s�̏����\�[�g	2001.12.06 hor
	F_SORT_DESC			,	//�I���s�̍~���\�[�g	2001.12.06 hor
	F_MERGE				,	//�I���s�̃}�[�W		2001.12.06 hor
	F_RECONVERT				//�ĕϊ� 				2002.04.09 minfu
//		F_WORDSREFERENCE		//�P�ꃊ�t�@�����X
};
const int nFincList_Edit_Num = sizeof( pnFuncList_Edit ) / sizeof( pnFuncList_Edit[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List3��List_Edit)


/* �J�[�\���ړ��n */
const int pnFuncList_Move[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List1��List_Move)
	F_UP				,	//�J�[�\����ړ�
	F_DOWN				,	//�J�[�\�����ړ�
	F_LEFT				,	//�J�[�\�����ړ�
	F_RIGHT				,	//�J�[�\���E�ړ�
	F_UP2				,	//�J�[�\����ړ�(�Q�s����)
	F_DOWN2				,	//�J�[�\�����ړ�(�Q�s����)
	F_WORDLEFT			,	//�P��̍��[�Ɉړ�
	F_WORDRIGHT			,	//�P��̉E�[�Ɉړ�
	F_GOLINETOP			,	//�s���Ɉړ�(�܂�Ԃ��P��)
	F_GOLINEEND			,	//�s���Ɉړ�(�܂�Ԃ��P��)
//	F_ROLLDOWN			,	//�X�N���[���_�E��
//	F_ROLLUP			,	//�X�N���[���A�b�v
	F_HalfPageUp		,	//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	F_HalfPageDown		,	//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	F_1PageUp			,	//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	F_1PageDown			,	//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	F_GOFILETOP			,	//�t�@�C���̐擪�Ɉړ�
	F_GOFILEEND			,	//�t�@�C���̍Ō�Ɉړ�
	F_CURLINECENTER		,	//�J�[�\���s���E�B���h�E������
	F_JUMP_DIALOG		,	//�w��s�w�W�����v	//Sept. 17, 2000 JEPRO �R�}���h�{�Ƃ́u�����n�v
	F_JUMPPREV			,	//�ړ�����: �O��
	F_JUMPNEXT			,	//�ړ�����: ����
	F_WndScrollDown		,	//�e�L�X�g���P�s���փX�N���[��	// 2001/06/20 asa-o
	F_WndScrollUp		,	//�e�L�X�g���P�s��փX�N���[��	// 2001/06/20 asa-o
	F_GONEXTPARAGRAPH	,	//���̒i����
	F_GOPREVPARAGRAPH		//�O�̒i����
};
const int nFincList_Move_Num = sizeof( pnFuncList_Move ) / sizeof( pnFuncList_Move[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List1��List_Move)


/* �I���n */	//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v����(�I��)���ړ�
const int pnFuncList_Select[] = {
	F_SELECTWORD			,	//���݈ʒu�̒P��I��
	F_SELECTALL				,	//���ׂđI��
	F_BEGIN_SEL				,	//�͈͑I���J�n
	F_UP_SEL				,	//(�͈͑I��)�J�[�\����ړ�
	F_DOWN_SEL				,	//(�͈͑I��)�J�[�\�����ړ�
	F_LEFT_SEL				,	//(�͈͑I��)�J�[�\�����ړ�
	F_RIGHT_SEL				,	//(�͈͑I��)�J�[�\���E�ړ�
	F_UP2_SEL				,	//(�͈͑I��)�J�[�\����ړ�(�Q�s����)
	F_DOWN2_SEL				,	//(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
	F_WORDLEFT_SEL			,	//(�͈͑I��)�P��̍��[�Ɉړ�
	F_WORDRIGHT_SEL			,	//(�͈͑I��)�P��̉E�[�Ɉړ�
	F_GOLINETOP_SEL			,	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
	F_GOLINEEND_SEL			,	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
//	F_ROLLDOWN_SEL			,	//(�͈͑I��)�X�N���[���_�E��
//	F_ROLLUP_SEL			,	//(�͈͑I��)�X�N���[���A�b�v
	F_HalfPageUp_Sel		,	//(�͈͑I��)���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	F_HalfPageDown_Sel		,	//(�͈͑I��)���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
	F_1PageUp_Sel			,	//(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
	F_1PageDown_Sel			,	//(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
	F_GOFILETOP_SEL			,	//(�͈͑I��)�t�@�C���̐擪�Ɉړ�
	F_GOFILEEND_SEL			,	//(�͈͑I��)�t�@�C���̍Ō�Ɉړ�
	F_GONEXTPARAGRAPH_SEL	,	//(�͈͑I��)���̒i����
	F_GOPREVPARAGRAPH_SEL		//(�͈͑I��)�O�̒i����
};
const int nFincList_Select_Num = sizeof( pnFuncList_Select ) / sizeof( pnFuncList_Select[0] );


/* ��`�I���n */	//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
const int pnFuncList_Box[] = {
//	F_BOXSELALL			,	//��`�ł��ׂđI��
	F_BEGIN_BOX			,	//��`�͈͑I���J�n
/*
	F_UP_BOX				//(��`�I��)�J�[�\����ړ�
	F_DOWN_BOX			,	//(��`�I��)�J�[�\�����ړ�
	F_LEFT_BOX			,	//(��`�I��)�J�[�\�����ړ�
	F_RIGHT_BOX			,	//(��`�I��)�J�[�\���E�ړ�
	F_UP2_BOX			,	//(��`�I��)�J�[�\����ړ�(�Q�s����)
	F_DOWN2_BOX			,	//(��`�I��)�J�[�\�����ړ�(�Q�s����)
	F_WORDLEFT_BOX		,	//(��`�I��)�P��̍��[�Ɉړ�
	F_WORDRIGHT_BOX		,	//(��`�I��)�P��̉E�[�Ɉړ�
	F_GOLINETOP_BOX		,	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
	F_GOLINEEND_BOX		,	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
	F_HalfPageUp_Box	,	//(��`�I��)���y�[�W�A�b�v
	F_HalfPageDown_Box	,	//(��`�I��)���y�[�W�_�E��
	F_1PageUp_Box		,	//(��`�I��)�P�y�[�W�A�b�v
	F_1PageDown_Box		,	//(��`�I��)�P�y�[�W�_�E��
	F_GOFILETOP_BOX		,	//(��`�I��)�t�@�C���̐擪�Ɉړ�
	F_GOFILEEND_BOX			//(��`�I��)�t�@�C���̍Ō�Ɉړ�
*/
};
const int nFincList_Box_Num = sizeof( pnFuncList_Box ) / sizeof( pnFuncList_Box[0] );


/* �N���b�v�{�[�h�n */
const int pnFuncList_Clip[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List2��List_Clip)
	F_CUT						,	//�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
	F_COPY						,	//�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	F_COPY_ADDCRLF				,	//�܂�Ԃ��ʒu�ɉ��s�����ăR�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
	F_COPY_CRLF					,	//CRLF���s�ŃR�s�[
	F_PASTE						,	//�\��t��(�N���b�v�{�[�h����\��t��)
	F_PASTEBOX					,	//��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
//	F_INSTEXT					,	//�e�L�X�g��\��t��		//Oct. 22, 2000 JEPRO �����ɒǉ�������������@�\�Ȃ̂��s���Ȃ̂ŃR�����g�A�E�g�ɂ��Ă���
//	F_ADDTAIL					,	//�Ō�Ƀe�L�X�g��ǉ�		//Oct. 22, 2000 JEPRO �����ɒǉ�������������@�\�Ȃ̂��s���Ȃ̂ŃR�����g�A�E�g�ɂ��Ă���
	F_COPYLINES					,	//�I��͈͓��S�s�R�s�[
	F_COPYLINESASPASSAGE		,	//�I��͈͓��S�s���p���t���R�s�[
	F_COPYLINESWITHLINENUMBER	,	//�I��͈͓��S�s�s�ԍ��t���R�s�[
	F_COPYFNAME					,	//���̃t�@�C�������N���b�v�{�[�h�ɃR�s�[ //2002/2/3 aroka
	F_COPYPATH					,	//���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
	F_COPYTAG					,	//���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[	//Sept. 14, 2000 JEPRO ���j���[�ɍ��킹�ĉ��Ɉړ�
	F_CREATEKEYBINDLIST				//�L�[���蓖�Ĉꗗ���R�s�[	//Sept. 15, 2000 JEPRO IDM_TEST�̂܂܂ł͂��܂������Ȃ��̂�F�ɕς��ēo�^	//Dec. 25, 2000 ����
};
const int nFincList_Clip_Num = sizeof( pnFuncList_Clip ) / sizeof( pnFuncList_Clip[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List1��List_Move)


/* �}���n */
const int pnFuncList_Insert[] = {
	F_INS_DATE				,	// ���t�}��
	F_INS_TIME					// �����}��
};
const int nFincList_Insert_Num = sizeof( pnFuncList_Insert ) / sizeof( pnFuncList_Insert[0] );


/* �ϊ��n */
const int pnFuncList_Convert[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List6��List_Convert)
	F_TOLOWER				,	//�p�啶�����p������
	F_TOUPPER				,	//�p���������p�啶��
	F_TOHANKAKU				,	/* �S�p�����p */
	F_TOZENEI				,	/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	F_TOHANEI				,	/* �S�p�p�������p�p�� */
	F_TOZENKAKUKATA			,	/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	F_TOZENKAKUHIRA			,	/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	F_HANKATATOZENKAKUKATA	,	/* ���p�J�^�J�i���S�p�J�^�J�i */
	F_HANKATATOZENKAKUHIRA	,	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	F_TABTOSPACE			,	/* TAB���� */
	F_SPACETOTAB			,	/* �󔒁�TAB */  //#### Stonee, 2001/05/27
	F_CODECNV_AUTO2SJIS		,	/* �������ʁ�SJIS�R�[�h�ϊ� */
	F_CODECNV_EMAIL			,	//E-Mail(JIS��SJIS)�R�[�h�ϊ�
	F_CODECNV_EUC2SJIS		,	//EUC��SJIS�R�[�h�ϊ�
	F_CODECNV_UNICODE2SJIS	,	//Unicode��SJIS�R�[�h�ϊ�
	F_CODECNV_UTF82SJIS		,	/* UTF-8��SJIS�R�[�h�ϊ� */
	F_CODECNV_UTF72SJIS		,	/* UTF-7��SJIS�R�[�h�ϊ� */
	F_CODECNV_SJIS2JIS		,	/* SJIS��JIS�R�[�h�ϊ� */
	F_CODECNV_SJIS2EUC		,	/* SJIS��EUC�R�[�h�ϊ� */
	F_CODECNV_SJIS2UTF8		,	/* SJIS��UTF-8�R�[�h�ϊ� */
	F_CODECNV_SJIS2UTF7		,	/* SJIS��UTF-7�R�[�h�ϊ� */
	F_BASE64DECODE			,	//Base64�f�R�[�h���ĕۑ�
	F_UUDECODE					//uudecode���ĕۑ�
	//Sept. 30, 2000JEPRO �R�����g�A�E�g����Ă������̂𕜊�������(���삵�Ȃ��̂����H)
	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX
};
const int nFincList_Convert_Num = sizeof( pnFuncList_Convert ) / sizeof( pnFuncList_Convert[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List6��List_Convert)


/* �����n */
const int pnFuncList_Search[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List4��List_Search)
	F_SEARCH_DIALOG		,	//����(�P�ꌟ���_�C�A���O)
	F_SEARCH_NEXT		,	//��������	//Sept. 16, 2000 JEPRO "��"��"�O"�̑O�Ɉړ�
	F_SEARCH_PREV		,	//�O������
	F_REPLACE_DIALOG	,	//�u��
	F_SEARCH_CLEARMARK	,	//�����}�[�N�̃N���A
	F_GREP_DIALOG		,	//Grep
	F_JUMP_DIALOG		,	//�w��s�w�W�����v
	F_OUTLINE			,	//�A�E�g���C�����
	F_TAGJUMP			,	//�^�O�W�����v�@�\
	F_TAGJUMPBACK		,	//�^�O�W�����v�o�b�N�@�\
	F_COMPARE			,	//�t�@�C�����e��r
	F_BRACKETPAIR		,	//�Ί��ʂ̌���
	F_BOOKMARK_SET		,	//�u�b�N�}�[�N�ݒ�E����
	F_BOOKMARK_NEXT		,	//���̃u�b�N�}�[�N��
	F_BOOKMARK_PREV		,	//�O�̃u�b�N�}�[�N��
	F_BOOKMARK_RESET	,	//�u�b�N�}�[�N�̑S����
	F_BOOKMARK_VIEW			//�u�b�N�}�[�N�̈ꗗ
};
const int nFincList_Search_Num = sizeof( pnFuncList_Search ) / sizeof( pnFuncList_Search[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List4��List_Search)


/* ���[�h�؂�ւ��n */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List8��List_Mode)
const int pnFuncList_Mode[] = {
	F_CHGMOD_INS		,	//�}���^�㏑�����[�h�؂�ւ�
	F_CANCEL_MODE			//�e�탂�[�h�̎�����
};
const int nFincList_Mode_Num = sizeof( pnFuncList_Mode ) / sizeof( pnFuncList_Mode[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List8��List_Mode)


/* �ݒ�n */
const int pnFuncList_Set[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List9��List_Set)
	F_SHOWTOOLBAR		,	/* �c�[���o�[�̕\�� */
	F_SHOWFUNCKEY		,	/* �t�@���N�V�����L�[�̕\�� */	//Sept. 14, 2000 JEPRO �t�@���N�V�����L�[�ƃX�e�[�^�X�o�[�����ւ�
	F_SHOWSTATUSBAR		,	/* �X�e�[�^�X�o�[�̕\�� */
	F_TYPE_LIST			,	/* �^�C�v�ʐݒ�ꗗ */			//Sept. 17, 2000 JEPRO �ݒ�n�ɓ����ĂȂ������̂Œǉ�
	F_OPTION_TYPE		,	/* �^�C�v�ʐݒ� */
	F_OPTION			,	/* ���ʐݒ� */
	F_FONT				,	/* �t�H���g�ݒ� */
	F_WRAPWINDOWWIDTH	,	/* ���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX
	F_PRINT_PAGESETUP		//����y�[�W�ݒ�				//Sept. 14, 2000 JEPRO �u����̃y�[�W���C�A�E�g�̐ݒ�v���u����y�[�W�ݒ�v�ɕύX	//Oct. 17, 2000 �R�}���h�{�Ƃ́u�t�@�C������n�v
};
int		nFincList_Set_Num = sizeof( pnFuncList_Set ) / sizeof( pnFuncList_Set[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List9��List_Set)


/* �}�N���n */
const int pnFuncList_Macro[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List10��List_Macro)
	F_RECKEYMACRO	,	/* �L�[�}�N���̋L�^�J�n�^�I�� */
	F_SAVEKEYMACRO	,	/* �L�[�}�N���̕ۑ� */
	F_LOADKEYMACRO	,	/* �L�[�}�N���̓ǂݍ��� */
	F_EXECKEYMACRO	,	/* �L�[�}�N���̎��s */
//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	F_EXECCMMAND		/* �O���R�}���h���s */
	F_EXECCOMMAND_DIALOG	/* �O���R�}���h���s */
//	To Here Sept. 20, 2000

};
const int nFincList_Macro_Num = sizeof( pnFuncList_Macro) / sizeof( pnFuncList_Macro[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List10��List_Macro)


/* �J�X�^�����j���[ */	//Oct. 21, 2000 JEPRO �u���̑��v���番���Ɨ���
#if 0
//	From Here Oct. 15, 2001 genta
//	�J�X�^�����j���[�̕�����𓮓I�ɕύX�\�ɂ��邽�߂���͍폜�D
const int pnFuncList_Menu[] = {
	F_MENU_RBUTTON				,	/* �E�N���b�N���j���[ */
	F_CUSTMENU_1				,	/* �J�X�^�����j���[1 */
	F_CUSTMENU_2				,	/* �J�X�^�����j���[2 */
	F_CUSTMENU_3				,	/* �J�X�^�����j���[3 */
	F_CUSTMENU_4				,	/* �J�X�^�����j���[4 */
	F_CUSTMENU_5				,	/* �J�X�^�����j���[5 */
	F_CUSTMENU_6				,	/* �J�X�^�����j���[6 */
	F_CUSTMENU_7				,	/* �J�X�^�����j���[7 */
	F_CUSTMENU_8				,	/* �J�X�^�����j���[8 */
	F_CUSTMENU_9				,	/* �J�X�^�����j���[9 */
	F_CUSTMENU_10				,	/* �J�X�^�����j���[10 */
	F_CUSTMENU_11				,	/* �J�X�^�����j���[11 */
	F_CUSTMENU_12				,	/* �J�X�^�����j���[12 */
	F_CUSTMENU_13				,	/* �J�X�^�����j���[13 */
	F_CUSTMENU_14				,	/* �J�X�^�����j���[14 */
	F_CUSTMENU_15				,	/* �J�X�^�����j���[15 */
	F_CUSTMENU_16				,	/* �J�X�^�����j���[16 */
	F_CUSTMENU_17				,	/* �J�X�^�����j���[17 */
	F_CUSTMENU_18				,	/* �J�X�^�����j���[18 */
	F_CUSTMENU_19				,	/* �J�X�^�����j���[19 */
	F_CUSTMENU_20				,	/* �J�X�^�����j���[20 */
	F_CUSTMENU_21				,	/* �J�X�^�����j���[21 */
	F_CUSTMENU_22				,	/* �J�X�^�����j���[22 */
	F_CUSTMENU_23				,	/* �J�X�^�����j���[23 */
	F_CUSTMENU_24				 	/* �J�X�^�����j���[24 */
};
const int nFincList_Menu_Num = sizeof( pnFuncList_Menu ) / sizeof( pnFuncList_Menu[0] );	//Oct. 21, 2000 JEPRO �u���̑��v���番���Ɨ���
#endif

/* �E�B���h�E�n */
const int pnFuncList_Win[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List7��List_Win)
	F_SPLIT_V			,	//�㉺�ɕ���	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
	F_SPLIT_H			,	//���E�ɕ���	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
	F_SPLIT_VH			,	//�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
	F_WINCLOSE			,	//�E�B���h�E�����
	F_WIN_CLOSEALL		,	//���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
	F_NEXTWINDOW		,	//���̃E�B���h�E
	F_PREVWINDOW		,	//�O�̃E�B���h�E
	F_CASCADE			,	//�d�˂ĕ\��
	F_TILE_V			,	//�㉺�ɕ��ׂĕ\��
	F_TILE_H			,	//���E�ɕ��ׂĕ\��
	F_MAXIMIZE_V		,	//�c�����ɍő剻
	F_MAXIMIZE_H		,	//�������ɍő剻 //2001.02.10 by MIK
	F_MINIMIZE_ALL		,	//���ׂčŏ���	//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
	F_REDRAW			,	//�ĕ`��
	F_WIN_OUTPUT			//�A�E�g�v�b�g�E�B���h�E�\��
};
const int nFincList_Win_Num = sizeof( pnFuncList_Win ) / sizeof( pnFuncList_Win[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List7��List_Win)


/* �x�� */
const int pnFuncList_Support[] = {	//Oct. 16, 2000 JEPRO �ϐ����ύX(List11��List_Support)
	F_HOKAN						,	/* ���͕⊮ */
//Sept. 15, 2000��Nov. 25, 2000 JEPRO //�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s���C���E����
	F_HELP_CONTENTS				,	/* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO �ǉ�
	F_HELP_SEARCH				,	/* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO �ǉ�
	F_MENU_ALLFUNC				,	/* �R�}���h�ꗗ */
	F_EXTHELP1					,	/* �O���w���v�P */
	F_EXTHTMLHELP				,	/* �O��HTML�w���v */
	F_ABOUT							/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�
};
const int nFincList_Support_Num = sizeof( pnFuncList_Support ) / sizeof( pnFuncList_Support[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List11��List_Support)


/* ���̑� */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List12��List_Others)
const int pnFuncList_Others[] = {
//	F_SENDMAIL					,	/* ���[�����M */
	F_DISABLE				//Oct. 21, 2000 JEPRO �����Ȃ��ƃG���[�ɂȂ��Ă��܂��̂Ń_�~�[��[����`]�����Ă���
};
const int nFincList_Others_Num = sizeof( pnFuncList_Others ) / sizeof( pnFuncList_Others[0] );	//Oct. 16, 2000 JEPRO �ϐ����ύX(List12��List_Others)





const int nsFuncCode::pnFuncListNumArr[] = {
//	nFincList_Undef_Num,	//Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɕύX	//Oct. 16, 2000 JEPRO �ϐ����ύX(List0��List_Undef)
	nFincList_File_Num,		/* �t�@�C������n */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List5��List_File)
	nFincList_Edit_Num,		/* �ҏW�n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List3��List_Edit)
	nFincList_Move_Num,		/* �J�[�\���ړ��n */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List1��List_Move)
	nFincList_Select_Num,	/* �I���n */			//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v����(�I��)���ړ�
	nFincList_Box_Num,		/* ��`�I���n */		//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
	nFincList_Clip_Num,		/* �N���b�v�{�[�h�n */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List2��List_Clip)
	nFincList_Insert_Num,	/* �}���n */
	nFincList_Convert_Num,	/* �ϊ��n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List6��List_Convert)
	nFincList_Search_Num,	/* �����n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List4��List_Search)
	nFincList_Mode_Num,		/* ���[�h�؂�ւ��n */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List8��List_Mode)
	nFincList_Set_Num,		/* �ݒ�n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List9��List_Set)
	nFincList_Macro_Num,	/* �}�N���n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List10��List_Macro)
//	�J�X�^�����j���[�̕�����𓮓I�ɕύX�\�ɂ��邽�߂���͍폜
//	nFincList_Menu_Num,		/* �J�X�^�����j���[ */	//Oct. 21, 2000 JEPRO �u���̑��v���番���Ɨ���
	nFincList_Win_Num,		/* �E�B���h�E�n */		//Oct. 16, 2000 JEPRO �ϐ����ύX(List7��List_Win)
	nFincList_Support_Num,	/* �x�� */				//Oct. 16, 2000 JEPRO �ϐ����ύX(List11��List_Support)
	nFincList_Others_Num	/* ���̑� */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List12��List_Others)
};
const int *	nsFuncCode::ppnFuncListArr[] = {
//	(int*)pnFuncList_Undef,	//Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɕύX	//Oct. 16, 2000 JEPRO �ϐ����ύX(List0��List_Undef)
	(int*)pnFuncList_File,	/* �t�@�C������n */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List5��List_File)
	(int*)pnFuncList_Edit,	/* �ҏW�n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List3��List_Edit)
	(int*)pnFuncList_Move,	/* �J�[�\���ړ��n */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List1��List_Move)
	(int*)pnFuncList_Select,/* �I���n */			//Oct. 15, 2000 JEPRO �u�J�[�\���ړ��n�v����(�I��)���ړ�  (��`�I��)�͐V�݂��ꎟ�悱���ɂ���
	(int*)pnFuncList_Box,	/* ��`�I���n */		//Oct. 17, 2000 JEPRO (��`�I��)���V�݂��ꎟ�悱���ɂ���
	(int*)pnFuncList_Clip,	/* �N���b�v�{�[�h�n */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List2��List_Clip)
	(int*)pnFuncList_Insert,/* �}���n */
	(int*)pnFuncList_Convert,/* �ϊ��n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List6��List_Convert)
	(int*)pnFuncList_Search,/* �����n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List4��List_Search)
	(int*)pnFuncList_Mode,	/* ���[�h�؂�ւ��n */	//Oct. 16, 2000 JEPRO �ϐ����ύX(List8��List_Mode)
	(int*)pnFuncList_Set,	/* �ݒ�n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List9��List_Set)
	(int*)pnFuncList_Macro,	/* �}�N���n */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List10��List_Macro)
//	�J�X�^�����j���[�̕�����𓮓I�ɕύX�\�ɂ��邽�߂���͍폜
//	(int*)pnFuncList_Menu,	/* �J�X�^�����j���[ */	//Oct. 21, 2000 JEPRO�u���̑��v���番���Ɨ���
	(int*)pnFuncList_Win,	/* �E�B���h�E�n */		//Oct. 16, 2000 JEPRO �ϐ����ύX(List7��List_Win)
	(int*)pnFuncList_Support,/* �x�� */				//Oct. 16, 2000 JEPRO �ϐ����ύX(List11��List_Support)
	(int*)pnFuncList_Others	/* ���̑� */			//Oct. 16, 2000 JEPRO �ϐ����ύX(List12��List_Others)
};
const int nsFuncCode::nFincListNumArrNum = sizeof( nsFuncCode::pnFuncListNumArr ) / sizeof( nsFuncCode::pnFuncListNumArr[0] );


/*[EOF]*/
