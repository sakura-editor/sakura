/*!	@file
	@brief �ҏW����v�f�u���b�N

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <stdlib.h>
#include "COpeBlk.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

COpeBlk::COpeBlk()
{
	m_refCount = 0;
}

COpeBlk::~COpeBlk()
{
	/* ����̔z����폜���� */
	for( int i = 0; i < (int)m_ppCOpeArr.size(); ++i ){
		SAFE_DELETE(m_ppCOpeArr[i]);
	}
	m_ppCOpeArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ����̒ǉ� */
bool COpeBlk::AppendOpe( COpe* pcOpe )
{
	if(pcOpe->m_ptCaretPos_PHY_Before.HasNegative() || pcOpe->m_ptCaretPos_PHY_After.HasNegative()){
		TopErrorMessage( NULL,
			_T("COpeBlk::AppendOpe() error.\n")
			_T("�o�O\n")
			_T("pcOpe->m_nCaretPos_PHY_Before = %d,%d\n")
			_T("pcOpe->m_nCaretPos_PHY_After = %d,%d\n"),
			pcOpe->m_ptCaretPos_PHY_Before.x,
			pcOpe->m_ptCaretPos_PHY_Before.y,
			pcOpe->m_ptCaretPos_PHY_After.x,
			pcOpe->m_ptCaretPos_PHY_After.y
		);
	}

	/* �z��̃������T�C�Y�𒲐� */
	m_ppCOpeArr.push_back(pcOpe);
	return true;
}


/* �����Ԃ� */
COpe* COpeBlk::GetOpe( int nIndex )
{
	if( GetNum() <= nIndex ){
		return NULL;
	}
	return m_ppCOpeArr[nIndex];
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �ҏW����v�f�u���b�N�̃_���v */
void COpeBlk::DUMP( void )
{
#ifdef _DEBUG
	int i;
	for( i = 0; i < GetNum(); ++i ){
		MYTRACE( _T("\tCOpeBlk.m_ppCOpeArr[%d]----\n"), i );
		m_ppCOpeArr[i]->DUMP();
	}
#endif
}



