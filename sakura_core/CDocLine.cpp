//	$Id$
/*!	@file
	@brief �����f�[�^1�s

	@author Norio Nakatani
	
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CDocLine.h"
#include "CMemory.h"

CDocLine::CDocLine()
	: m_pPrev( NULL ), m_pNext( NULL ), m_pLine( NULL )
{
//	m_bMark.m_bAllMark  = 0;
	m_bMark.m_bMarkArray.m_bModify   = 1;	//true
	m_bMark.m_bMarkArray.m_bBookMark = 0;	//false
	m_bMark.m_bMarkArray.m_bDiffMark = 0;
}

CDocLine::~CDocLine()
{
	//	delete��NULL��P���ɖ�������̂�NULL����͕s�v
	delete m_pLine;
}

/* ��s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ������擾����
	true�F��s���B
	false�F��s����Ȃ����B

	2002/04/26 YAZAKI
*/
bool CDocLine::IsEmptyLine( void )
{
	char* pLine = GetPtr();
	int nLineLen = GetLengthWithoutEOL();
	int i;
	for ( i = 0; i < nLineLen; i++ ){
		if (pLine[i] != ' ' && pLine[i] != '\t'){
			return false;	//	�X�y�[�X�ł��^�u�ł��Ȃ���������������false�B
		}
	}
	return true;	//	���ׂăX�y�[�X���^�u������������true�B
}

/*[EOF]*/
