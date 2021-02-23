/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#include "StdAfx.h"
#include "CViewCalc.h"
#include "mem/CMemoryIterator.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"

//外部依存
CLayoutInt CViewCalc::GetTabSpace() const
{
	return m_pOwner->m_pcEditDoc->m_cLayoutMgr.GetTabSpace();
}

CTsvModeInfo& CViewCalc::GetTsvMode() const
{
	return m_pOwner->m_pcEditDoc->m_cLayoutMgr.m_tsvInfo;
}

CPixelXInt CViewCalc::GetCharSpacing() const
{
	return m_pOwner->m_pcEditDoc->m_cLayoutMgr.GetCharSpacing();
}

/* 指定された桁に対応する行のデータ内の位置を調べる Ver1
	
	@@@ 2002.09.28 YAZAKI CDocLine版
*/
CLogicInt CViewCalc::LineColumnToIndex( const CDocLine* pcDocLine, CLayoutInt nColumn ) const
{
	CLogicInt i2 = CLogicInt(0);
	CMemoryIterator it( pcDocLine, GetTabSpace(), GetTsvMode(), m_pOwner->m_pcEditDoc->m_cLayoutMgr.GetWidthPerKeta(), GetCharSpacing() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}

/* 指定された桁に対応する行のデータ内の位置を調べる Ver1
	
	@@@ 2002.09.28 YAZAKI CLayoutが必要になりました。
*/
CLogicInt CViewCalc::LineColumnToIndex( const CLayout* pcLayout, CLayoutInt nColumn ) const
{
	CLogicInt i2 = CLogicInt(0);
	CMemoryIterator it = m_pOwner->m_pcEditDoc->m_cLayoutMgr.CreateCMemoryIterator(pcLayout);
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}

/* 指定された桁に対応する行のデータ内の位置を調べる Ver0 */
/* 指定された桁より、行が短い場合はpnLineAllColLenに行全体の表示桁数を返す */
/* それ以外の場合はpnLineAllColLenに０をセットする
	
	@@@ 2002.09.28 YAZAKI CLayoutが必要になりました。
*/
CLogicInt CViewCalc::LineColumnToIndex2( const CLayout* pcLayout, CLayoutInt nColumn, CLayoutInt* pnLineAllColLen ) const
{
	*pnLineAllColLen = CLayoutInt(0);

	CLogicInt i2 = CLogicInt(0);
	CLayoutInt nPosX2 = CLayoutInt(0);
	CMemoryIterator it = m_pOwner->m_pcEditDoc->m_cLayoutMgr.CreateCMemoryIterator(pcLayout);
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	if( i2 >= pcLayout->GetLengthWithEOL() ){
		nPosX2 += it.getColumn();
		*pnLineAllColLen = nPosX2;
	}
	return i2;
}

/*
||	指定された行のデータ内の位置に対応する桁の位置を調べる
||
||	@@@ 2002.09.28 YAZAKI CLayoutが必要になりました。
*/
CLayoutInt CViewCalc::LineIndexToColumn( const CLayout* pcLayout, CLogicInt nIndex ) const
{
	//	以下、iterator版
	CLayoutInt nPosX2 = CLayoutInt(0);
	CMemoryIterator it = m_pOwner->m_pcEditDoc->m_cLayoutMgr.CreateCMemoryIterator(pcLayout);
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > nIndex ){
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
}

/*
||	指定された行のデータ内の位置に対応する桁の位置を調べる
||
||	@@@ 2002.09.28 YAZAKI CDocLine版
*/
CLayoutInt CViewCalc::LineIndexToColumn( const CDocLine* pcDocLine, CLogicInt nIndex ) const
{
	CLayoutInt nPosX2 = CLayoutInt(0);
	CMemoryIterator it( pcDocLine, GetTabSpace(), GetTsvMode(), m_pOwner->m_pcEditDoc->m_cLayoutMgr.GetWidthPerKeta(), GetCharSpacing() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > nIndex ){
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
}
