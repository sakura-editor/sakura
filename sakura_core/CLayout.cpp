//	$Id$
/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���

	@author Norio Nakatani
	@date 1998/3/11 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CLayout.h"
#include "debug.h"



CLayout::CLayout()
{
	m_pPrev = NULL;
	m_pNext = NULL;
	m_nLinePhysical = 0;	/* �Ή�����_���s�ԍ� */
	m_pCDocLine = NULL;
	m_nOffset = 0;	/* �Ή�����_���s�̐擪����̃I�t�Z�b�g */
	m_nLength = 0;	/* �Ή�����_���s�̃n�C�g�� */
	m_nIndent = 0;	/* ���̃��C�A�E�g�s�̃C���f���g�� @@@ 2002.09.23 YAZAKI */
	m_nTypePrev = 0;/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
	return;
}


CLayout::~CLayout()
{
	return;
}

void CLayout::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE( "\n\n��CLayout::DUMP()======================\n" );
	MYTRACE( "m_nLinePhysical=%d\t\t�Ή�����_���s�ԍ�\n", m_nLinePhysical );
	MYTRACE( "m_nOffset=%d\t\t�Ή�����_���s�̐擪����̃I�t�Z�b�g\n", m_nOffset );
	MYTRACE( "m_nLength=%d\t\t�Ή�����_���s�̃n�C�g��\n", m_nLength );
	MYTRACE( "m_nTypePrev=%d\t\t�^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� \n", m_nTypePrev );
	MYTRACE( "======================\n" );
#endif
	return;
}


/*[EOF]*/
