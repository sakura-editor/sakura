//	$Id$
/************************************************************************

	CDocLine.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDocLine;

#ifndef _CDOCLINE_H_
#define _CDOCLINE_H_

#include "CMemory.h"
#include "CEOL.h"

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDocLine
{
public:
	/*
	||  Constructors
	*/
	CDocLine();
	~CDocLine();


	CDocLine*	m_pPrev;
	CDocLine*	m_pNext;
	CMemory*	m_pLine;
//	int			m_nType;	/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
	int			m_bModify;	/* �ύX�t���O */
	CEOL		m_cEol;		/* �s���R�[�h */
//	enumEOLType	m_enumEOLType;	/* ���s�R�[�h�̎�� */
//	int			m_nEOLLen;		/* ���s�R�[�h�̒��� */ 		
//	int			m_nModifyCount;	/* �ύX�� */
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINE_H_ */

/*[EOF]*/
