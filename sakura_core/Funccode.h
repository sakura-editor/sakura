//	$Id$
/*!	@file
	�@�\�Ɋւ��邢�낢��
	
	�@�\�ԍ���`

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

#ifndef _FUNCCODE_H_
#define _FUNCCODE_H_

/* ����`�p(�_�~�[�Ƃ��Ă��g��) */	//Oct. 17, 2000 jepro noted
#define F_DISABLE		0	//���g�p


/* �t�@�C������n */
#define F_FILENEW					30101	//�V�K�쐬
#define F_FILEOPEN					30102	//�t�@�C�����J��
#define F_FILESAVE					30103	//�㏑���ۑ�
#define F_FILESAVEAS				30104	//���O��t���ĕۑ�
#define F_FILECLOSE					30105	//����(����)	//Oct. 17, 2000 jepro �u�t�@�C�������v�Ƃ����L���v�V������ύX
#define F_FILECLOSE_OPEN			30107	//���ĊJ��
#define F_FILE_REOPEN_SJIS			30111	//SJIS�ŊJ������
#define F_FILE_REOPEN_JIS			30112	//JIS�ŊJ������
#define F_FILE_REOPEN_EUC			30113	//EUC�ŊJ������
#define F_FILE_REOPEN_UNICODE		30114	//Unicode�ŊJ������
#define F_FILE_REOPEN_UTF8			30115	//UTF-8�ŊJ������
#define F_FILE_REOPEN_UTF7			30116	//UTF-7�ŊJ������
#define F_PRINT						30150	//���
#define F_PRINT_PREVIEW				30151	//����v���r���[
#define F_PRINT_PAGESETUP			30152	//����y�[�W�ݒ�	//Sept. 14, 2000 jepro �u����̃y�[�W���C�A�E�g�̐ݒ�v����u����y�[�W�ݒ�v�ɕύX
//#define F_PRINT_DIALOG				30151	//����_�C�A���O
#define F_OPEN_HHPP					30160	//������C/C++�w�b�_�t�@�C�����J��	//Feb. 9, 2001 jepro�u.c�܂���.cpp�Ɠ�����.h���J���v����ύX
#define F_OPEN_CCPP					30161	//������C/C++�\�[�X�t�@�C�����J��	//Feb. 9, 2001 jepro�u.h�Ɠ�����.c(�Ȃ����.cpp)���J���v����ύX
#define	F_OPEN_HfromtoC				30162	//������C/C++�w�b�_(�\�[�X)���J��	//Feb. 7, 2001 JEPRO �ǉ�
#define F_ACTIVATE_SQLPLUS			30170	/* Oracle SQL*Plus���A�N�e�B�u�\�� */
#define F_PLSQL_COMPILE_ON_SQLPLUS	30171	/* Oracle SQL*Plus�Ŏ��s */	//Sept. 17, 2000 jepro �����́u�R���p�C���v���u���s�v�ɓ���
#define F_BROWSE					30180	//�u���E�Y
#define F_PROPERTY_FILE				30190	/* �t�@�C���̃v���p�e�B */
#define F_EXITALL					30195	/* �e�L�X�g�G�f�B�^�̑S�I�� */	//Dec. 27, 2000 JEPRO �ǉ�


/* �ҏW�n */
#define F_CHAR				30200	//��������
#define F_IME_CHAR			30201	//�S�p��������
#define F_UNDO				30210	//���ɖ߂�(Undo)
#define F_REDO				30211	//��蒼��(Redo)
#define F_DELETE			30221	//�J�[�\���ʒu���폜
#define F_DELETE_BACK		30222	//�J�[�\���̑O���폜
#define F_WordDeleteToStart	30230	//�P��̍��[�܂ō폜
#define F_WordDeleteToEnd	30231	//�P��̉E�[�܂ō폜
#define F_WordCut			30232	//�P��؂���
#define F_WordDelete		30233	//�P��폜
#define F_LineCutToStart	30240	//�s���܂Ő؂���(���s�P��)
#define F_LineCutToEnd		30241	//�s���܂Ő؂���(���s�P��)
#define F_LineDeleteToStart	30242	//�s���܂ō폜(���s�P��)
#define F_LineDeleteToEnd	30243	//�s���܂ō폜(���s�P��)
#define F_CUT_LINE			30244	//�s�؂���(�܂�Ԃ��P��)
#define F_DELETE_LINE		30245	//�s�폜(�܂�Ԃ��P��)
#define F_DUPLICATELINE		30250	//�s�̓�d��(�܂�Ԃ��P��)
#define F_INDENT_TAB		30260	//TAB�C���f���g
#define F_UNINDENT_TAB		30261	//�tTAB�C���f���g
#define F_INDENT_SPACE		30262	//SPACE�C���f���g
#define F_UNINDENT_SPACE	30263	//�tSPACE�C���f���g
#define F_WORDSREFERENCE	30270	//�P�ꃊ�t�@�����X


/* �J�[�\���ړ��n */
#define F_UP				30311	//�J�[�\����ړ�
#define F_DOWN				30312	//�J�[�\�����ړ�
#define F_LEFT				30313	//�J�[�\�����ړ�
#define F_RIGHT				30314	//�J�[�\���E�ړ�
#define F_UP2				30315	//�J�[�\����ړ�(�Q�s����)
#define F_DOWN2				30316	//�J�[�\�����ړ�(�Q�s����)
#define F_WORDLEFT			30320	//�P��̍��[�Ɉړ�
#define F_WORDRIGHT			30321	//�P��̉E�[�Ɉړ�
//#define F_GOLINETOP		30330	//�s���Ɉړ�(���s�P��)
//#define F_GOLINEEND		30331	//�s���Ɉړ�(���s�P��)
#define F_GOLINETOP			30332	//�s���Ɉړ�(�܂�Ԃ��P��)
#define F_GOLINEEND			30333	//�s���Ɉړ�(�܂�Ԃ��P��)
//#define F_ROLLDOWN			30340	//�X�N���[���_�E��//Oct. 10, 2000 JEPRO ���̔��y�[�W�A�b�v�ɖ��̕ύX
//#define F_ROLLUP			30341	//�X�N���[���A�b�v//Oct. 10, 2000 JEPRO ���̔��y�[�W�_�E���ɖ��̕ύX
#define F_HalfPageUp		30340	//���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
#define F_HalfPageDown		30341	//���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
#define F_1PageUp			30342	//�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
#define F_1PageDown			30343	//�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
//#define F_DISPLAYTOP		30344	//��ʂ̐擪�Ɉړ�(������)
//#define F_DISPLAYEND		30345	//��ʂ̍Ō�Ɉړ�(������)
#define F_GOFILETOP			30350	//�t�@�C���̐擪�Ɉړ�
#define F_GOFILEEND			30351	//�t�@�C���̍Ō�Ɉړ�
#define F_CURLINECENTER		30360	//�J�[�\���s���E�B���h�E������
#define F_JUMPPREV			30370	//�ړ�����: �O��
#define F_JUMPNEXT			30371	//�ړ�����: ����
#define F_WndScrollUp		30380	//��ʂ���ւP�s�X�N���[��	// 2001/06/20 asa-o
#define F_WndScrollDown		30381	//��ʂ����ւP�s�X�N���[��	// 2001/06/20 asa-o


/* �I���n */
#define F_SELECTWORD		30400	//���݈ʒu�̒P��I��
#define F_SELECTALL			30401	//���ׂđI��
#define F_BEGIN_SEL			30410	//�͈͑I���J�n
#define F_UP_SEL			30411	//(�͈͑I��)�J�[�\����ړ�
#define F_DOWN_SEL			30412	//(�͈͑I��)�J�[�\�����ړ�
#define F_LEFT_SEL			30413	//(�͈͑I��)�J�[�\�����ړ�
#define F_RIGHT_SEL			30414	//(�͈͑I��)�J�[�\���E�ړ�
#define F_UP2_SEL			30415	//(�͈͑I��)�J�[�\����ړ�(�Q�s����)
#define F_DOWN2_SEL			30416	//(�͈͑I��)�J�[�\�����ړ�(�Q�s����)
#define F_WORDLEFT_SEL		30420	//(�͈͑I��)�P��̍��[�Ɉړ�
#define F_WORDRIGHT_SEL		30421	//(�͈͑I��)�P��̉E�[�Ɉړ�
//#define F_GOLINETOP_SEL		30430	//(�͈͑I��)�s���Ɉړ�(���s�P��)
//#define F_GOLINEEND_SEL		30431	//(�͈͑I��)�s���Ɉړ�(���s�P��)
#define F_GOLINETOP_SEL		30432	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
#define F_GOLINEEND_SEL		30433	//(�͈͑I��)�s���Ɉړ�(�܂�Ԃ��P��)
//#define F_ROLLDOWN_SEL		30440	//(�͈͑I��)�X�N���[���_�E��//Oct. 10, 2000 JEPRO ���̔��y�[�W�A�b�v�ɖ��̕ύX
//#define F_ROLLUP_SEL		30441	//(�͈͑I��)�X�N���[���A�b�v//Oct. 10, 2000 JEPRO ���̔��y�[�W�_�E���ɖ��̕ύX
#define F_HalfPageUp_Sel	30440	//(�͈͑I��)���y�[�W�A�b�v	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
#define F_HalfPageDown_Sel	30441	//(�͈͑I��)���y�[�W�_�E��	//Oct. 6, 2000 JEPRO ���̂�PC-AT�݊��@�n�ɕύX(ROLL��PAGE) //Oct. 10, 2000 JEPRO ���̕ύX
#define F_1PageUp_Sel		30442	//(�͈͑I��)�P�y�[�W�A�b�v	//Oct. 10, 2000 JEPRO �]���̃y�[�W�A�b�v�𔼃y�[�W�A�b�v�Ɩ��̕ύX���P�y�[�W�A�b�v��ǉ�
#define F_1PageDown_Sel		30443	//(�͈͑I��)�P�y�[�W�_�E��	//Oct. 10, 2000 JEPRO �]���̃y�[�W�_�E���𔼃y�[�W�_�E���Ɩ��̕ύX���P�y�[�W�_�E����ǉ�
//#define F_DISPLAYTOP_SEL	30444	//(�͈͑I��)��ʂ̐擪�Ɉړ�(������)
//#define F_DISPLAYEND_SEL	30445	//(�͈͑I��)��ʂ̍Ō�Ɉړ�(������)
#define F_GOFILETOP_SEL		30450	//(�͈͑I��)�t�@�C���̐擪�Ɉړ�
#define F_GOFILEEND_SEL		30451	//(�͈͑I��)�t�@�C���̍Ō�Ɉړ�


/* ��`�I���n */
//#define F_BOXSELALL		30500	//��`�ł��ׂđI��
#define	F_BEGIN_BOX			30510	//��`�͈͑I���J�n
//Oct. 17, 2000 JEPRO �ȉ��ɋ�`�I���̃R�}���h���̂ݏ������Ă�����
//#define F_UP_BOX			30511	//(��`�I��)�J�[�\����ړ�
//#define F_DOWN_BOX			30512	//(��`�I��)�J�[�\�����ړ�
//#define F_LEFT_BOX			30513	//(��`�I��)�J�[�\�����ړ�
//#define F_RIGHT_BOX			30514	//(��`�I��)�J�[�\���E�ړ�
//#define F_UP2_BOX			30515	//(��`�I��)�J�[�\����ړ�(�Q�s����)
//#define F_DOWN2_BOX			30516	//(��`�I��)�J�[�\�����ړ�(�Q�s����)
//#define F_WORDLEFT_BOX		30520	//(��`�I��)�P��̍��[�Ɉړ�
//#define F_WORDRIGHT_BOX		30521	//(��`�I��)�P��̉E�[�Ɉړ�
////#define F_GOLINETOP_BOX		30530	//(��`�I��)�s���Ɉړ�(���s�P��)
////#define F_GOLINEEND_BOX		30531	//(��`�I��)�s���Ɉړ�(���s�P��)
//#define F_GOLINETOP_BOX		30532	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
//#define F_GOLINEEND_BOX		30533	//(��`�I��)�s���Ɉړ�(�܂�Ԃ��P��)
//#define F_HalfPageUp_Box	30540	//(��`�I��)���y�[�W�A�b�v
//#define F_HalfPageDown_Box	30541	//(��`�I��)���y�[�W�_�E��
//#define F_1PageUp_Box		30542	//(��`�I��)�P�y�[�W�A�b�v
//#define F_1PageDown_Box		30543	//(��`�I��)�P�y�[�W�_�E��
////#define F_DISPLAYTOP_BOX	30444	//(��`�I��)��ʂ̐擪�Ɉړ�(������)
////#define F_DISPLAYEND_BOX	30445	//(��`�I��)��ʂ̍Ō�Ɉړ�(������)
//#define F_GOFILETOP_BOX		30550	//(��`�I��)�t�@�C���̐擪�Ɉړ�
//#define F_GOFILEEND_BOX		30551	//(��`�I��)�t�@�C���̍Ō�Ɉړ�


/* �N���b�v�{�[�h�n */
#define F_CUT						30601	//�؂���(�I��͈͂��N���b�v�{�[�h�ɃR�s�[���č폜)
#define F_COPY						30602	//�R�s�[(�I��͈͂��N���b�v�{�[�h�ɃR�s�[)
#define F_COPY_CRLF					30603	//CRLF���s�ŃR�s�[
#define F_PASTE						30604	//�\��t��(�N���b�v�{�[�h����\��t��)
#define F_PASTEBOX					30605	//��`�\��t��(�N���b�v�{�[�h�����`�\��t��)
#define F_INSTEXT					30606	//�e�L�X�g��\��t��
#define F_ADDTAIL					30607	//�Ō�Ƀe�L�X�g��ǉ�
#define F_COPYLINES					30610	//�I��͈͓��S�s�R�s�[
#define F_COPYLINESASPASSAGE		30611	//�I��͈͓��S�s���p���t���R�s�[
#define F_COPYLINESWITHLINENUMBER	30612	//�I��͈͓��S�s�s�ԍ��t���R�s�[
#define F_COPYPATH					30620	//���̃t�@�C���̃p�X�����N���b�v�{�[�h�ɃR�s�[
#define F_COPYTAG					30621	//���̃t�@�C���̃p�X���ƃJ�[�\���ʒu���R�s�[
//Sept. 16, 2000 JEPRO �V���[�g�J�b�g�L�[�����܂������Ȃ��̂Ŏ��s�͎E���Ă���	//Dec. 25, 2000 ����
#define F_CREATEKEYBINDLIST			30630	//�L�[���蓖�Ĉꗗ���R�s�[ //Sept. 15, 2000 JEPRO ��̍s��IDM_TEST�̂܂܂ł͂��܂������Ȃ��̂�F�ɕς��ēo�^


/* �}���n */
#define F_INS_DATE				30790	//���t�}��
#define F_INS_TIME				30791	//�����}��


/* �ϊ��n */
#define F_TOLOWER				30800	//�p�啶�����p������
#define F_TOUPPER				30801	//�p���������p�啶��
#define F_TOHANKAKU				30810	/* �S�p�����p */
#define F_TOZENKAKUKATA			30811	/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
#define F_TOZENKAKUHIRA			30812	/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
#define F_HANKATATOZENKAKUKATA	30813	/* ���p�J�^�J�i���S�p�J�^�J�i */
#define F_HANKATATOZENKAKUHIRA	30814	/* ���p�J�^�J�i���S�p�Ђ炪�� */
#define F_TABTOSPACE			30830	/* TAB���� */
#define F_SPACETOTAB			30831	/* �󔒁�TAB *///#### Stonee, 2001/05/27
#define F_CODECNV_AUTO2SJIS		30850	/* �������ʁ�SJIS�R�[�h�ϊ� */
#define F_CODECNV_EMAIL			30851	//E-Mail(JIS��SJIS)�R�[�h�ϊ�
#define F_CODECNV_EUC2SJIS		30852	//EUC��SJIS�R�[�h�ϊ�
#define F_CODECNV_UNICODE2SJIS	30853	//Unicode��SJIS�R�[�h�ϊ�
#define F_CODECNV_UTF82SJIS		30854	/* UTF-8��SJIS�R�[�h�ϊ� */
#define F_CODECNV_UTF72SJIS		30855	/* UTF-7��SJIS�R�[�h�ϊ� */
#define F_CODECNV_SJIS2JIS		30860	/* SJIS��JIS�R�[�h�ϊ� */
#define F_CODECNV_SJIS2EUC		30861	/* SJIS��EUC�R�[�h�ϊ� */
#define F_CODECNV_SJIS2UTF8		30862	/* SJIS��UTF-8�R�[�h�ϊ� */
#define F_CODECNV_SJIS2UTF7		30863	/* SJIS��UTF-7�R�[�h�ϊ� */
#define F_BASE64DECODE			30870	//Base64�f�R�[�h���ĕۑ�
#define F_UUDECODE				30880	//uudecode���ĕۑ�	//Oct. 17, 2000 jepro �������u�I�𕔕���UUENCODE�f�R�[�h�v����ύX


/* �����n */
#define F_SEARCH_DIALOG		30901	//����(�P�ꌟ���_�C�A���O)
#define F_SEARCH_NEXT		30902	//��������
#define F_SEARCH_PREV		30903	//�O������
#define F_REPLACE			30904	//�u��(�u���_�C�A���O)
#define F_SEARCH_CLEARMARK	30905	//�����}�[�N�̃N���A
#define F_GREP				30910	//Grep
#define F_JUMP				30920	//�w��s�փW�����v
#define F_OUTLINE			30930	//�A�E�g���C�����
#define F_TAGJUMP			30940	//�^�O�W�����v�@�\
#define F_TAGJUMPBACK		30941	//�^�O�W�����v�o�b�N�@�\
#define F_COMPARE			30950	//�t�@�C�����e��r
#define F_BRACKETPAIR		30960	//�Ί��ʂ̌���


/* ���[�h�؂�ւ��n */
#define F_CHGMOD_INS		31001	//�}���^�㏑�����[�h�؂�ւ�
#define F_CANCEL_MODE		31099	//�e�탂�[�h�̎�����


/* �ݒ�n */
#define F_SHOWTOOLBAR		31100	/* �c�[���o�[�̕\�� */
#define F_SHOWFUNCKEY		31101	/* �t�@���N�V�����L�[�̕\�� */
#define F_SHOWSTATUSBAR		31102	/* �X�e�[�^�X�o�[�̕\�� */
#define F_TYPE_LIST			31110	/* �^�C�v�ʐݒ�ꗗ */
#define F_OPTION_TYPE		31111	/* �^�C�v�ʐݒ� */
#define F_OPTION			31112	/* ���ʐݒ� */

//From here �ݒ�_�C�A���O�p�̋@�\�ԍ���p��  Stonee, 2001/05/18
#define F_TYPE_SCREEN		31115	/* �^�C�v�ʐݒ�w�X�N���[���x */
#define F_TYPE_COLOR		31116	/* �^�C�v�ʐݒ�w�J���[�x */
#define F_OPTION_GENERAL	32000	/* ���ʐݒ�w�S�ʁx */
#define F_OPTION_WINDOW		32001	/* ���ʐݒ�w�E�B���h�E�x */
#define F_OPTION_EDIT		32002	/* ���ʐݒ�w�ҏW�x */
#define F_OPTION_FILE		32003	/* ���ʐݒ�w�t�@�C���x */
#define F_OPTION_BACKUP		32004	/* ���ʐݒ�w�o�b�N�A�b�v�x */
#define F_OPTION_FORMAT		32005	/* ���ʐݒ�w�����x */
#define F_OPTION_URL		32006	/* ���ʐݒ�w�N���b�J�u��URL�x */
#define F_OPTION_GREP		32007	/* ���ʐݒ�wGrep�x */
#define F_OPTION_KEYBIND	32008	/* ���ʐݒ�w�L�[���蓖�āx */
#define F_OPTION_CUSTMENU	32009	/* ���ʐݒ�w�J�X�^�����j���[�x */
#define F_OPTION_TOOLBAR	32010	/* ���ʐݒ�w�c�[���o�[�x */
#define F_OPTION_KEYWORD	32011	/* ���ʐݒ�w�����L�[���[�h�x */
#define F_OPTION_HELPER		32012	/* ���ʐݒ�w�x���x */
//To here  Stonee, 2001/05/18

#define F_FONT				31120	/* �t�H���g�ݒ� */
#define F_WRAPWINDOWWIDTH	31140	//���݂̃E�B���h�E���Ő܂�Ԃ� */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH �� WRAPWINDOWWIDTH �ɕύX


/* �}�N���n */
#define F_RECKEYMACRO		31250	/* �L�[�}�N���̋L�^�J�n�^�I�� */
#define F_SAVEKEYMACRO		31251	/* �L�[�}�N���̕ۑ� */
#define F_LOADKEYMACRO		31252	/* �L�[�}�N���̓ǂݍ��� */
#define F_EXECKEYMACRO		31253	/* �L�[�}�N���̎��s */
//	From Here Sept. 20, 2000 JEPRO ����CMMAND��COMMAND�ɕύX
//	#define F_EXECCMMAND		31270	/* �O���R�}���h���s */
#define F_EXECCOMMAND		31270	/* �O���R�}���h���s */
//	To Here Sept. 20, 2000

//	Jul. 4, 2000 genta
#define F_USERMACRO_0		31200	/* �o�^�}�N���J�n */
#define SIZE_CUSTMACRO		100		/* �o�^�ł���}�N���̐� */


/* �J�X�^�����j���[ */
#define F_MENU_RBUTTON		31580	/* �E�N���b�N���j���[ */
#define F_CUSTMENU_1		31501	/* �J�X�^�����j���[1 */
#define F_CUSTMENU_2		31502	/* �J�X�^�����j���[2 */
#define F_CUSTMENU_3		31503	/* �J�X�^�����j���[3 */
#define F_CUSTMENU_4		31504	/* �J�X�^�����j���[4 */
#define F_CUSTMENU_5		31505	/* �J�X�^�����j���[5 */
#define F_CUSTMENU_6		31506	/* �J�X�^�����j���[6 */
#define F_CUSTMENU_7		31507	/* �J�X�^�����j���[7 */
#define F_CUSTMENU_8		31508	/* �J�X�^�����j���[8 */
#define F_CUSTMENU_9		31509	/* �J�X�^�����j���[9 */
#define F_CUSTMENU_10		31510	/* �J�X�^�����j���[10 */
#define F_CUSTMENU_11		31511	/* �J�X�^�����j���[11 */
#define F_CUSTMENU_12		31512	/* �J�X�^�����j���[12 */
#define F_CUSTMENU_13		31513	/* �J�X�^�����j���[13 */
#define F_CUSTMENU_14		31514	/* �J�X�^�����j���[14 */
#define F_CUSTMENU_15		31515	/* �J�X�^�����j���[15 */
#define F_CUSTMENU_16		31516	/* �J�X�^�����j���[16 */
#define F_CUSTMENU_17		31517	/* �J�X�^�����j���[17 */
#define F_CUSTMENU_18		31518	/* �J�X�^�����j���[18 */
#define F_CUSTMENU_19		31519	/* �J�X�^�����j���[19 */
#define F_CUSTMENU_20		31520	/* �J�X�^�����j���[20 */
#define F_CUSTMENU_21		31521	/* �J�X�^�����j���[21 */
#define F_CUSTMENU_22		31522	/* �J�X�^�����j���[22 */
#define F_CUSTMENU_23		31523	/* �J�X�^�����j���[23 */
#define F_CUSTMENU_24		31524	/* �J�X�^�����j���[24 */


/* �E�B���h�E�n */
#define F_SPLIT_V			31310	//�㉺�ɕ���	//Sept. 17, 2000 jepro �����́u�c�v���u�㉺�Ɂv�ɕύX
#define F_SPLIT_H			31311	//���E�ɕ���	//Sept. 17, 2000 jepro �����́u���v���u���E�Ɂv�ɕύX
#define F_SPLIT_VH			31312	//�c���ɕ���	//Sept. 17, 2000 jepro �����Ɂu�Ɂv��ǉ�
#define F_WINCLOSE			31320	//�E�B���h�E�����
#define F_WIN_CLOSEALL		31321	//���ׂẴE�B���h�E�����	//Oct. 17, 2000 JEPRO ���O��ύX(F_FILECLOSEALL��F_WIN_CLOSEALL)
#define F_CASCADE			31330	//�d�˂ĕ\��
#define F_TILE_V			31331	//�㉺�ɕ��ׂĕ\��
#define F_TILE_H			31332	//���E�ɕ��ׂĕ\��
#define F_NEXTWINDOW		31340	//���̃E�B���h�E
#define F_PREVWINDOW		31341	//�O�̃E�B���h�E
#define F_MAXIMIZE_V		31350	//�c�����ɍő剻
#define F_MINIMIZE_ALL		31351	//���ׂčŏ���		//Sept. 17, 2000 jepro �����́u�S�āv���u���ׂāv�ɓ���
#define F_MAXIMIZE_H		31352	//�������ɍő剻	//2001.02.10 by MIK
#define F_REDRAW			31360	//�ĕ`��
#define F_WIN_OUTPUT		31370	//�A�E�g�v�b�g�E�B���h�E�\��


/* �x�� */
#define F_HOKAN				31430		/* ���͕⊮ */
//Sept. 16, 2000��Nov. 25, 2000 JEPRO //�V���[�g�J�b�g�L�[�����܂������Ȃ��̂ŎE���Ă���������2�s���C���E����
#define F_HELP_CONTENTS		31440		/* �w���v�ڎ� */			//Nov. 25, 2000 JEPRO �ǉ�
#define F_HELP_SEARCH		31441		/* �w���v�L�[���[�h���� */	//Nov. 25, 2000 JEPRO �ǉ�
#define F_MENU_ALLFUNC		31445		/* �R�}���h�ꗗ */
#define F_EXTHELP1			31450		/* �O���w���v�P */
#define F_EXTHTMLHELP		31451		/* �O��HTML�w���v */
#define F_ABOUT				31455		/* �o�[�W������� */	//Dec. 24, 2000 JEPRO �ǉ�


/* ���̑� */
//#define F_SENDMAIL		31570		/* ���[�����M */	//Oct. 17, 2000 JEPRO ���[���@�\�͎���ł���̂ŃR�����g�A�E�g�ɂ���




/* �@�\�ꗗ�Ɋւ���f�[�^�錾 */
namespace nsFuncCode{
	extern const char*	ppszFuncKind[];
	extern const int	nFuncKindNum;
	extern const int	pnFuncListNumArr[];
	extern const int*	ppnFuncListArr[];
	extern const int	nFincListNumArrNum;
};
///////////////////////////////////////////////////////////////////////
#endif /* _FUNCCODE_H_ */


/*[EOF]*/
