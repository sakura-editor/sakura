/*!	@file
	@brief 編集操作要素ブロック

	@author Norio Nakatani
	@date 1998/06/09 新規作成
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
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

COpeBlk::COpeBlk()
{
	m_refCount = 0;
}

COpeBlk::~COpeBlk()
{
	/* 操作の配列を削除する */
	int size = (int)m_ppCOpeArr.size();
	for( int i = 0; i < size; ++i ){
		SAFE_DELETE(m_ppCOpeArr[i]);
	}
	m_ppCOpeArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 操作の追加 */
bool COpeBlk::AppendOpe( COpe* pcOpe )
{
	if(pcOpe->m_ptCaretPos_PHY_Before.HasNegative() || pcOpe->m_ptCaretPos_PHY_After.HasNegative()){
		TopErrorMessage( NULL,
			_T("COpeBlk::AppendOpe() error.\n")
			_T("Bug.\n")
			_T("pcOpe->m_ptCaretPos_PHY_Before = %d,%d\n")
			_T("pcOpe->m_ptCaretPos_PHY_After = %d,%d\n"),
			pcOpe->m_ptCaretPos_PHY_Before.x,
			pcOpe->m_ptCaretPos_PHY_Before.y,
			pcOpe->m_ptCaretPos_PHY_After.x,
			pcOpe->m_ptCaretPos_PHY_After.y
		);
	}

	/* 配列のメモリサイズを調整 */
	m_ppCOpeArr.push_back(pcOpe);
	return true;
}


/* 操作を返す */
COpe* COpeBlk::GetOpe( int nIndex )
{
	if( GetNum() <= nIndex ){
		return NULL;
	}
	return m_ppCOpeArr[nIndex];
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 編集操作要素ブロックのダンプ */
void COpeBlk::DUMP( void )
{
#ifdef _DEBUG
	int i;
	int size = GetNum();
	for( i = 0; i < size; ++i ){
		MYTRACE( _T("\tCOpeBlk.m_ppCOpeArr[%d]----\n"), i );
		m_ppCOpeArr[i]->DUMP();
	}
#endif
}



