/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���

	@author Norio Nakatani
	@date 1998/3/11 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CLayout;
class CLayoutMgr;

#ifndef _CLAYOUT_H_
#define _CLAYOUT_H_



#include "CEol.h"// 2002/2/10 aroka
#include "doc/CDocLine.h"// 2002/4/21 YAZAKI
#include "mem/CMemory.h"// 2002/4/21 YAZAKI
#include "view/colors/EColorIndexType.h"


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CLayout
{
protected:
	friend class CLayoutMgr; //####��
public:
	/*
	||  Constructors
	*/
	//2007.08.23 kobake �R���X�g���N�^�Ń����o�ϐ�������������悤�ɂ���
	CLayout(
		const CDocLine*	pcDocLine,		//!< ���f�[�^�ւ̎Q��
		CLogicPoint		ptLogicPos,		//!< ���f�[�^�Q�ƈʒu
		CLogicInt		nLength,		//!< ���f�[�^���f�[�^��
		EColorIndexType	nTypePrev,
		CLayoutInt		nTypeIndent
	)
	{
		m_pPrev			= NULL;
		m_pNext			= NULL;
		m_pCDocLine		= pcDocLine;
		m_ptLogicPos	= ptLogicPos;	// ���f�[�^�Q�ƈʒu
		m_nLength		= nLength;		// ���f�[�^���f�[�^��
		m_nTypePrev		= nTypePrev;	// �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V����������
		m_nIndent		= nTypeIndent;	// ���̃��C�A�E�g�s�̃C���f���g�� @@@ 2002.09.23 YAZAKI
	}
	~CLayout();
	void DUMP( void );
	
	// m_ptLogicPos.x�ŕ␳�������Ƃ̕�����𓾂�
	const wchar_t* GetPtr() const   { return m_pCDocLine->GetPtr() + m_ptLogicPos.x; }
	CLogicInt GetLengthWithEOL() const    { return m_nLength;	}	//	������EOL�͏��1�����ƃJ�E���g�H�H
	CLogicInt GetLengthWithoutEOL() const { return m_nLength - (m_cEol.GetLen() ? 1 : 0);	}
	//CLogicInt GetLength() const {	return m_nLength;	}	//	CMemoryIterator�p�iEOL�܂ށj
	CLayoutInt GetIndent() const {	return m_nIndent;	}	//!< ���̃��C�A�E�g�s�̃C���f���g�T�C�Y���擾�B�P�ʂ͔��p�����B	CMemoryIterator�p

	//�擾�C���^�[�t�F�[�X
	CLogicInt GetLogicLineNo() const{ if(this)return m_ptLogicPos.GetY2(); else return CLogicInt(-1); } //$$$������
	CLogicInt GetLogicOffset() const{ return m_ptLogicPos.GetX2(); }
	CLogicPoint GetLogicPos() const{ return m_ptLogicPos; }
	EColorIndexType GetColorTypePrev() const{ return m_nTypePrev; } //#########����
	CLayoutInt GetLayoutWidth() const{ return m_nLayoutWidth; }		// 2009.08.28 nasukoji	���̃��C�A�E�g�s�̉��s���܂ރ��C�A�E�g����Ԃ�

	//�ύX�C���^�[�t�F�[�X
	void OffsetLogicLineNo(CLogicInt n){ m_ptLogicPos.y+=n; }
	void SetColorTypePrev(EColorIndexType n)
	{
		m_nTypePrev=n;
	}
	void SetLayoutWidth(CLayoutInt nWidth){ m_nLayoutWidth = nWidth; }

	//!���C�A�E�g�����v�Z�B�C���f���g�����s���܂܂Ȃ��B2007.10.11 kobake
	CLayoutInt CalcLayoutWidth(const CLayoutMgr& cLayoutMgr) const;

	//! �I�t�Z�b�g�l�����C�A�E�g�P�ʂɕϊ����Ď擾�B2007.10.17 kobake
	CLayoutInt CalcLayoutOffset(const CLayoutMgr& cLayoutMgr) const;

	//! ������Q�Ƃ��擾
	CStringRef GetStringRef() const{ return CStringRef(GetPtr(), GetLengthWithEOL()); }

	//�`�F�[������
	CLayout* GetPrevLayout() const{ return m_pPrev; }
	CLayout* GetNextLayout() const{ return m_pNext; }
	void _SetPrevLayout(CLayout* pcLayout){ m_pPrev = pcLayout; }
	void _SetNextLayout(CLayout* pcLayout){ m_pNext = pcLayout; }

	//���f�[�^�Q��
	const CDocLine* GetDocLineRef() const{ if(this)return m_pCDocLine; else return NULL; } //$$note:������

	//���̑������Q��
	const CEol& GetLayoutEol() const{ return m_cEol; }

private:
	CLayout*			m_pPrev;
	CLayout*			m_pNext;

	//�f�[�^�Q�Ɣ͈�
	const CDocLine*		m_pCDocLine;		//!< ���f�[�^�ւ̎Q��
	CLogicPoint			m_ptLogicPos;		//!< �Ή����郍�W�b�N�Q�ƈʒu
	CLogicInt			m_nLength;			//!< ���̃��C�A�E�g�s�̒����B�����P�ʁB
	
	//���̑�����
	EColorIndexType		m_nTypePrev;		//!< �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V����������
	CLayoutInt			m_nIndent;			//!< ���̃��C�A�E�g�s�̃C���f���g�� @@@ 2002.09.23 YAZAKI
	CEol				m_cEol;
	CLayoutInt			m_nLayoutWidth;		//!< ���̃��C�A�E�g�s�̉��s���܂ރ��C�A�E�g���i�u�܂�Ԃ��Ȃ��v�I�����̂݁j	// 2009.08.28 nasukoji
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUT_H_ */



