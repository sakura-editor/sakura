//	$Id$
/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���

	@author Norio Nakatani
	@date 1998/3/11 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CLayout;

#ifndef _CLAYOUT_H_
#define _CLAYOUT_H_


#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif

//#include "CDocLine.h"
#include "CEol.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/4/21 YAZAKI
#include "CMemory.h"// 2002/4/21 YAZAKI
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CLayout
{
public:
	/*
	||  Constructors
	*/
	CLayout();
	~CLayout();
	void DUMP( void );
	
	/* m_nOffset�ŕ␳�������Ƃ̕�����𓾂� */
	char*	GetPtr() const {	return m_pCDocLine->m_pLine->GetPtr() + m_nOffset;	};
	int		GetLengthWithEOL() const {	return m_nLength;	};	//	������EOL�͏��1�����ƃJ�E���g�H�H
	int		GetLengthWithoutEOL() const {	return m_nLength - (m_cEol.GetLen() ? 1 : 0);	};

public:
	CLayout*		m_pPrev;
	CLayout*		m_pNext;
	int				m_nLinePhysical;		/*!< �Ή�������s�P�ʂ̍s�̔ԍ� */
	const CDocLine*	m_pCDocLine;
//	const char*		m_pLine;
	int				m_nOffset;		/*!< �Ή�������s�P�ʂ̍s������̃I�t�Z�b�g */
	int				m_nLength;		/*!< ���̃��C�A�E�g�s�̒���(�n�C�g��) */
	int				m_nTypePrev;	/*!< �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
	int				m_nTypeNext;	/*!< �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
//	enumEOLType		m_enumEOLType;	/*!< ���s�R�[�h�̎�� */
//	int				m_nEOLLen;		/*!< ���s�R�[�h�̒��� */
	CEOL			m_cEol;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUT_H_ */


/*[EOF]*/
