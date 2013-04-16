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

#include "CEol.h"
#include "mem/CMemory.h"

#include "docplus/CBookmarkManager.h"
#include "docplus/CDiffManager.h"
#include "docplus/CModifyManager.h"

class COpeBlk;

//!	�����f�[�^1�s
class CDocLine{
protected:
	friend class CDocLineMgr; //######��
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CDocLine();
	~CDocLine();

	//����
	bool			IsEmptyLine() const;		//	����CDocLine����s�i�X�y�[�X�A�^�u�A���s�L���݂̂̍s�j���ǂ����B

	//�f�[�^�擾
	CLogicInt		GetLengthWithoutEOL() const			{ return m_cLine.GetStringLength() - m_cEol.GetLen(); } //!< �߂�l�͕����P�ʁB
	const wchar_t*	GetPtr() const						{ return m_cLine.GetStringPtr(); }
	CLogicInt		GetLengthWithEOL() const			{ return m_cLine.GetStringLength(); }	//	CMemoryIterator�p
#ifdef USE_STRICT_INT
	const wchar_t*	GetDocLineStrWithEOL(int* pnLen) const //###���̖��O�A���̑Ώ�
	{
		CLogicInt n;
		const wchar_t* p = GetDocLineStrWithEOL(&n);
		*pnLen = n;
		return p;
	}
#endif
	const wchar_t*	GetDocLineStrWithEOL(CLogicInt* pnLen) const //###���̖��O�A���̑Ώ�
	{
		if(this){
			*pnLen = GetLengthWithEOL(); return GetPtr();
		}
		else{
			*pnLen = 0; return NULL;
		}
	}
	CStringRef GetStringRefWithEOL() const //###���̖��O�A���̑Ώ�
	{
		if(this){
			return CStringRef(GetPtr(),GetLengthWithEOL());
		}
		else{
			return CStringRef(NULL,0);
		}
	}
	const CEol& GetEol() const{ return m_cEol; }
	void SetEol(const CEol& cEol, COpeBlk* pcOpeBlk);

	const CNativeW& _GetDocLineDataWithEOL() const { return m_cLine; } //###��

	//�f�[�^�ݒ�
	void SetDocLineString(const wchar_t* pData, int nLength);
	void SetDocLineString(const CNativeW& cData);

	//�`�F�[������
	CDocLine* GetPrevLine(){ return m_pPrev; }
	CDocLine* GetNextLine(){ return m_pNext; }
	void _SetPrevLine(CDocLine* pcDocLine){ m_pPrev = pcDocLine; }
	void _SetNextLine(CDocLine* pcDocLine){ m_pNext = pcDocLine; }
	

private: //####
	CDocLine*	m_pPrev;	//!< ��O�̗v�f
	CDocLine*	m_pNext;	//!< ���̗v�f
private:
	CNativeW	m_cLine;	//!< �f�[�^  2007.10.11 kobake �|�C���^�ł͂Ȃ��A���̂����悤�ɕύX
	CEol		m_cEol;		//!< �s���R�[�h
public:
	//�g����� $$������
	struct MarkType{
		CLineModified	m_cModified;	//�ύX�t���O
		CLineBookmarked	m_cBookmarked;	//�u�b�N�}�[�N
		CLineDiffed		m_cDiffmarked;	//DIFF�������
	};
	MarkType m_sMark;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINE_H_ */



