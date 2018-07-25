/*!	@file
	@brief 編集操作要素

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "COpe.h"
#include "mem/CMemory.h"// 2002/2/10 aroka


// COpeクラス構築
COpe::COpe(EOpeCode eCode)
{
	assert( eCode != OPE_UNKNOWN );
	m_nOpe = eCode;					// 操作種別

	m_ptCaretPos_PHY_Before.Set(CLogicInt(-1),CLogicInt(-1));	//カーソル位置
	m_ptCaretPos_PHY_After.Set(CLogicInt(-1),CLogicInt(-1));	//カーソル位置

}




/* COpeクラス消滅 */
COpe::~COpe()
{
}

/* 編集操作要素のダンプ */
void COpe::DUMP( void )
{
	DEBUG_TRACE( _T("\t\tm_nOpe                  = [%d]\n"), m_nOpe               );
	DEBUG_TRACE( _T("\t\tm_ptCaretPos_PHY_Before = [%d,%d]\n"), m_ptCaretPos_PHY_Before.x, m_ptCaretPos_PHY_Before.y   );
	DEBUG_TRACE( _T("\t\tm_ptCaretPos_PHY_After  = [%d,%d]\n"), m_ptCaretPos_PHY_After.x, m_ptCaretPos_PHY_After.y   );
	return;
}

/* 編集操作要素のダンプ */
void CDeleteOpe::DUMP( void )
{
	COpe::DUMP();
	DEBUG_TRACE( _T("\t\tm_ptCaretPos_PHY_To     = [%d,%d]\n"), m_ptCaretPos_PHY_To.x, m_ptCaretPos_PHY_To.y );
	DEBUG_TRACE( _T("\t\tm_cOpeLineData.size         = [%d]\n"), m_cOpeLineData.size() );
	for( size_t i = 0; i < m_cOpeLineData.size(); i++ ){
		DEBUG_TRACE( _T("\t\tm_cOpeLineData[%d].nSeq         = [%d]\n"), m_cOpeLineData[i].nSeq );
		DEBUG_TRACE( _T("\t\tm_cOpeLineData[%d].cmemLine     = [%ls]\n"), m_cOpeLineData[i].cmemLine.GetStringPtr() );		
	}
	return;
}

/* 編集操作要素のダンプ */
void CInsertOpe::DUMP( void )
{
	COpe::DUMP();
	DEBUG_TRACE( _T("\t\tm_cOpeLineData.size         = [%d]\n"), m_cOpeLineData.size() );
	for( size_t i = 0; i < m_cOpeLineData.size(); i++ ){
		DEBUG_TRACE( _T("\t\tm_cOpeLineData[%d].nSeq         = [%d]\n"), m_cOpeLineData[i].nSeq );
		DEBUG_TRACE( _T("\t\tm_cOpeLineData[%d].cmemLine     = [%ls]\n"), m_cOpeLineData[i].cmemLine.GetStringPtr() );		
	}
	return;
}


