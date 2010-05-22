/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���

	@author Norio Nakatani
	@date 1998/3/11 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "doc/CLayout.h"
#include "debug/Debug.h"
#include "charset/charcode.h"
#include "CLayoutMgr.h"
#include "CBregexp.h" // CLayoutMgr�̒�`�ŕK�v



CLayout::~CLayout()
{
	return;
}

void CLayout::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE_A( "\n\n��CLayout::DUMP()======================\n" );
	MYTRACE_A( "m_ptLogicPos.y=%d\t\t�Ή�����_���s�ԍ�\n", m_ptLogicPos.y );
	MYTRACE_A( "m_ptLogicPos.x=%d\t\t�Ή�����_���s�̐擪����̃I�t�Z�b�g\n", m_ptLogicPos.x );
	MYTRACE_A( "m_nLength=%d\t\t�Ή�����_���s�̃n�C�g��\n", (int)m_nLength );
	MYTRACE_A( "m_nTypePrev=%d\t\t�^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� \n", m_nTypePrev );
	MYTRACE_A( "======================\n" );
#endif
	return;
}

//!���C�A�E�g�����v�Z�B�C���f���g�����s���܂܂Ȃ��B
//2007.10.11 kobake �쐬
//2007.11.29 kobake �^�u�����v�Z����Ă��Ȃ������̂��C��
CLayoutInt CLayout::CalcLayoutWidth(const CLayoutMgr& cLayoutMgr) const
{
	//�\�[�X
	const wchar_t* pText    = m_pCDocLine->GetPtr();
	CLogicInt      nTextLen = m_pCDocLine->GetLengthWithoutEOL();

	//�v�Z
	CLayoutInt nWidth = CLayoutInt(0);
	for(CLogicInt i=m_ptLogicPos.GetX2();i<m_ptLogicPos.GetX2()+m_nLength;i++){
		if(pText[i]==WCODE::TAB){
			nWidth += cLayoutMgr.GetActualTabSpace(nWidth);
		}
		else{
			nWidth += CNativeW::GetKetaOfChar(pText,nTextLen,i);
		}
	}
	return nWidth;
}

//! �I�t�Z�b�g�l�����C�A�E�g�P�ʂɕϊ����Ď擾�B2007.10.17 kobake
CLayoutInt CLayout::CalcLayoutOffset(const CLayoutMgr& cLayoutMgr) const
{
	CLayoutInt nRet(0);
	if(this->GetLogicOffset()){
		const wchar_t* pLine = this->m_pCDocLine->GetPtr();
		int nLineLen = this->m_pCDocLine->GetLengthWithEOL();
		for(int i=0;i<GetLogicOffset();i++){
			if(pLine[i]==WCODE::TAB){
				nRet+=cLayoutMgr.GetActualTabSpace(nRet);
			}
			else{
				nRet+=CNativeW::GetKetaOfChar(pLine,nLineLen,i);
			}
		}
	}
	return nRet;
}



