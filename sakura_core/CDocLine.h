/*!	@file
	@brief �����f�[�^1�s

	@author Norio Nakatani

	@date 2001/12/03 hor ������(bookmark)�@�\�ǉ��ɔ��������o�[�ǉ�
	@date 2001/12/18 hor bookmark, �C���t���O�̃A�N�Z�X�֐���
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDocLine;

#ifndef _CDOCLINE_H_
#define _CDOCLINE_H_

#include "CEOL.h"
#include "CMemory.h"


//@@@ 2002.05.25 MIK
#define	MARK_DIFF_APPEND	1	//�ǉ�
#define	MARK_DIFF_CHANGE	2	//�ύX
#define	MARK_DIFF_DELETE	3	//�폜
#define	MARK_DIFF_DEL_EX	4	//�폜(EOF�ȍ~)

//!	�����f�[�^1�s
class CDocLine
{
public:
	/*
	||  Constructors
	*/
	CDocLine();
	~CDocLine();


	CDocLine*	m_pPrev;	//!< ��O�̗v�f
	CDocLine*	m_pNext;	//!< ���̗v�f
	CNativeW	m_cLine;	//!< �f�[�^  2007.10.11 kobake �|�C���^�ł͂Ȃ��A���̂����悤�ɕύX
	CEOL		m_cEol;		//!< �s���R�[�h

	bool		IsModifyed  ( void ) const { return m_bMark.m_bMarkArray.m_bModify ? true : false; }	// �ύX�t���O�̏�Ԃ��擾����	//@@@ 2002.05.25 MIK
	void		SetModifyFlg( bool bFlg )  { m_bMark.m_bMarkArray.m_bModify = bFlg ? 1 : 0; }	// �ύX�t���O�̏�Ԃ��w�肷��	//@@@ 2002.05.25 MIK

	bool		IsBookMarked( void ) const { return m_bMark.m_bMarkArray.m_bBookMark ? true : false; }	// �u�b�N�}�[�N�̏�Ԃ��擾����	//@@@ 2002.05.25 MIK
	void		SetBookMark ( bool bFlg )  { m_bMark.m_bMarkArray.m_bBookMark = bFlg ? 1 : 0; }	// �u�b�N�}�[�N�̏�Ԃ��w�肷��	//@@@ 2002.05.25 MIK

	int			IsDiffMarked( void ) const { return (int)m_bMark.m_bMarkArray.m_bDiffMark; }	//������Ԃ��擾����	//@@@ 2002.05.25 MIK
	void		SetDiffMark( int type )    { m_bMark.m_bMarkArray.m_bDiffMark = type; }		//������Ԃ�ݒ肷��	//@@@ 2002.05.25 MIK

	bool		IsEmptyLine( void ) const;	//	����CDocLine����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ����B
	CLogicInt	GetLengthWithoutEOL( void ) const	{ return m_cLine.GetStringLength() - m_cEol.GetLen(); } //!< �߂�l�͕����P�ʁB
	const wchar_t*	GetPtr( void ) const			{ return m_cLine.GetStringPtr(); }
	CLogicInt	GetLength() const					{ return m_cLine.GetStringLength(); }	//	CMemoryIterator�p

	//! ������Q�Ƃ��擾
	CStringRef GetStringRef() const{ return CStringRef(GetPtr(), GetLength()); }
protected:
	//�}�[�N���
	union {
		unsigned char	m_bAllMark;
		struct Mark_tag {
			unsigned m_bModify		: 1;	//�ύX�t���O
			unsigned m_bBookMark	: 1;	//�u�b�N�}�[�N
			unsigned m_bDiffMark	: 3;	//DIFF�������
		} m_bMarkArray;
	} m_bMark;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINE_H_ */


/*[EOF]*/
