//	$Id$
/*!	@file
	@brief �����f�[�^1�s

	@author Norio Nakatani
	@date 2001/12/03 hor ������(bookmark)�@�\�ǉ��ɔ��������o�[�ǉ�
	@date 2001/12/18 hor bookmark, �C���t���O�̃A�N�Z�X�֐���
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDocLine;

#ifndef _CDOCLINE_H_
#define _CDOCLINE_H_

#include "CEOL.h"
#include "CMemory.h"

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif

//!	�����f�[�^1�s
class CDocLine
{
public:
	/*
	||  Constructors
	*/
	CDocLine();
	~CDocLine();


	CDocLine*	m_pPrev;	/*!< ��O�̗v�f */
	CDocLine*	m_pNext;	/*!< ���̗v�f */
	CMemory*	m_pLine;	/*!< �f�[�^ */
//	int			m_nType;	/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
	CEOL		m_cEol;		/*!< �s���R�[�h */
//	enumEOLType	m_enumEOLType;	/* ���s�R�[�h�̎�� */
//	int			m_nEOLLen;		/* ���s�R�[�h�̒��� */
//	int			m_nModifyCount;	/* �ύX�� */

	bool		IsModifyed  ( void ) const { return m_bModify;  }	// �ύX�t���O�̏�Ԃ��擾����	2001.12.18 hor
	void		SetModifyFlg( bool bFlg )  { m_bModify = bFlg;  }	// �ύX�t���O�̏�Ԃ��w�肷��	2001.12.18 hor

	bool		IsBookMarked( void ) const { return m_bBookMark;}	// �u�b�N�}�[�N�̏�Ԃ��擾���� 2001.12.14 hor
	void		SetBookMark ( bool bFlg )  { m_bBookMark = bFlg;}	// �u�b�N�}�[�N�̏�Ԃ��w�肷�� 2001.12.14 hor

	bool		IsEmptyLine( void );	//	����CDocLine����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ����B
	int			GetLengthWithoutEOL( void ){ return m_pLine->GetLength() - m_cEol.GetLen(); }
	char*		GetPtr( void )             { return m_pLine->GetPtr(); }
protected:
	bool		m_bModify;					/*!< �ύX�t���O */
	bool		m_bBookMark;				/*!< �u�b�N�}�[�N */
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINE_H_ */


/*[EOF]*/
