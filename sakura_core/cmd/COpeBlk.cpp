/*!	@file
	@brief 編集操作要素ブロック

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <stdlib.h>
#include "cmd/COpeBlk.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

COpeBlk::COpeBlk()
{
	return;
}

COpeBlk::~COpeBlk()
{
	return;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 操作の追加 */
bool COpeBlk::AppendOpe( COpe* pcOpe )
{
	if(pcOpe->m_ptCaretPos_PHY_Before.HasNegative() || pcOpe->m_ptCaretPos_PHY_After.HasNegative()){
		TopErrorMessage( nullptr,
			L"COpeBlk::AppendOpe() error.\n"
			L"Bug.\n"
			L"pcOpe->m_ptCaretPos_PHY_Before = %d,%d\n"
			L"pcOpe->m_ptCaretPos_PHY_After = %d,%d\n",
			int(pcOpe->m_ptCaretPos_PHY_Before.x),
			int(pcOpe->m_ptCaretPos_PHY_Before.y),
			int(pcOpe->m_ptCaretPos_PHY_After.x),
			int(pcOpe->m_ptCaretPos_PHY_After.y)
		);
	}

	/* 配列のメモリサイズを調整 */
	m_ppCOpeArr.emplace_back(pcOpe);
	return true;
}

/* 操作を返す */
COpe* COpeBlk::GetOpe( int nIndex )
{
	if( GetNum() <= nIndex ){
		return nullptr;
	}
	return m_ppCOpeArr[nIndex].get();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 編集操作要素ブロックのダンプ */
void COpeBlk::DUMP( void )
{
#if ENABLE_DEBUG_DUMP

	int i;
	int size = GetNum();
	for( i = 0; i < size; ++i ){
		MYTRACE( L"\tCOpeBlk.m_ppCOpeArr[%d]----\n", i );
		m_ppCOpeArr[i]->DUMP();
	}

#endif // if ENABLE_DEBUG_DUMP

	return;
}
