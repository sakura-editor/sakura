/*!	@file
	@brief �ҏW����v�f

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "COpe.h"
#include "mem/CMemory.h"// 2002/2/10 aroka


// COpe�N���X�\�z
COpe::COpe(EOpeCode eCode)
{
	m_nOpe = eCode;					// ������

	m_ptCaretPos_PHY_Before.Set(CLogicInt(-1),CLogicInt(-1));	//�J�[�\���ʒu
	m_ptCaretPos_PHY_After.Set(CLogicInt(-1),CLogicInt(-1));	//�J�[�\���ʒu

}




/* COpe�N���X���� */
COpe::~COpe()
{
}

/* �ҏW����v�f�̃_���v */
void COpe::DUMP( void )
{
	DEBUG_TRACE( _T("\t\tm_nOpe                  = [%d]\n"), m_nOpe               );
	DEBUG_TRACE( _T("\t\tm_ptCaretPos_PHY_Before = [%d,%d]\n"), m_ptCaretPos_PHY_Before.x, m_ptCaretPos_PHY_Before.y   );
	DEBUG_TRACE( _T("\t\tm_ptCaretPos_PHY_After  = [%d,%d]\n"), m_ptCaretPos_PHY_After.x, m_ptCaretPos_PHY_After.y   );
//	DEBUG_TRACE( _T("\t\tm_nDataLen              = [%d]\n"),  m_nDataLen           );
//	DEBUG_TRACE( _T("\t\tm_pcmemData             = [%ls]\n"), m_pcmemData.GetStringPtr() );
	return;
}



