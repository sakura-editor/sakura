//	$Id$
/*!	@file
	�A�v���P�[�V������`�̃��b�Z�[�W

	@author Norio Nakatani
	@date 1998/5/15 �V�K�쐬
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

#include <windows.h>

#define MYWM_DOSPLIT        (WM_APP+1500)
#define MYWM_SETACTIVEPANE  (WM_APP+1510)
#define MYWM_CHANGESETTING  (WM_APP+1520)	/* �ݒ肪�ύX���ꂽ���Ƃ̒ʒm���b�Z�[�W */

/* �^�X�N�g���C����̒ʒm���b�Z�[�W */
#define MYWM_NOTIFYICON		(WM_APP+100)




//	/* �G�f�B�^����g���C�ւ̎��ȓo�^�v�� */
//	#define	MYWM_ADD_EDITOR	(WM_APP+101)

//	/* �G�f�B�^����g���C�ւ̎��ȍ폜�v�� */
//	#define	MYWM_DEL_EDITOR	(WM_APP+102)

//	/* �G�f�B�^����g���C�ւ̃A�N�Z�����[�^��Ǘv�� */
//	#define	MYWM_GETACCELFUNC	(WM_APP+103)




/* �g���C����G�f�B�^�ւ̏I���v�� */
#define	MYWM_CLOSE			(WM_APP+200)

//	/* �g���C����G�f�B�^�ւ̃A�N�Z�����[�^�e�[�u���X�V�ʒm */
//	#define	MYWM_SETACCEL		(WM_APP+201)

///* �g���C����G�f�B�^�ւ̐ݒ�ύX�ʒm */
//#define	MYWM_SETOPTION		(WM_APP+202)

/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C����� �v���ʒm */
#define	MYWM_GETFILEINFO	(WM_APP+203)

/* �J�[�\���ʒu�ύX�ʒm */
#define	MYWM_SETCARETPOS	(WM_APP+204)

/* �J�[�\���ʒu�擾�v�� */
#define	MYWM_GETCARETPOS	(WM_APP+205)

/* �e�L�X�g�ǉ��ʒm(���L�f�[�^�o�R) */
#define	MYWM_ADDSTRING		(WM_APP+206)

/* �^�O�W�����v���ʒm */
#define	MYWM_SETREFERER		(WM_APP+207)

/* �s(���s�P��)�f�[�^�̗v�� */
#define	MYWM_GETLINEDATA	(WM_APP+208)


/* �ҏW�E�B���h�E�I�u�W�F�N�g����̃I�u�W�F�N�g�폜�v�� */
#define	MYWM_DELETE_ME		(WM_APP+209)

/* �V�����ҏW�E�B���h�E�̍쐬�˗�(�R�}���h���C����n��) */
#define	MYWM_OPENNEWEDITOR	(WM_APP+210)

///* �ҏW�E�B���h�E�I�u�W�F�N�g����̃A�N�e�B�u�v�� */
//#define	MYWM_ACTIVATE_ME		(WM_APP+211)

#define	MYWM_HTMLHELP			(WM_APP+212)

///* ���񂽂������? */
//#define	MYWM_AREYOUGREP			(WM_APP+212)


//	/* �G�f�B�^����g���C�ւ̕ҏW�t�@�C����� �ݒ�ʒm */
//	#define	MYWM_SETFILEINFO	(WM_APP+204)


//	/* �g���C�Ǘ��E�B���h�E����̑S�I�����b�Z�[�W */
//	#define	MYWM_CLOSEALL		(WM_APP+900)


/*[EOF]*/
