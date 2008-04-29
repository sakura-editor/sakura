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

#include "stdafx.h"
#include "COpe.h"
#include "debug/Debug.h"
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
#ifdef _DEBUG
	MYTRACE_A( "\t\tm_nOpe                  = [%d]\n", m_nOpe               );
	MYTRACE_A( "\t\tm_ptCaretPos_PHY_Before = [%d,%d]\n", m_ptCaretPos_PHY_Before.x, m_ptCaretPos_PHY_Before.y   );
	MYTRACE_A( "\t\tm_ptCaretPos_PHY_After  = [%d,%d]\n", m_ptCaretPos_PHY_After.x, m_ptCaretPos_PHY_After.y   );
//	MYTRACE_A( "\t\tm_nDataLen              = [%d]\n",  m_nDataLen           );
//	MYTRACE_A( "\t\tm_pcmemData             = [%ls]\n", m_pcmemData.GetStringPtr() );
#endif
	return;
}



