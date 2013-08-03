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
#include "Debug.h"



COpeBlk::COpeBlk()
{
	m_refCount = 0;
}

COpeBlk::~COpeBlk()
{
	int		i;
	/* 操作の配列を削除する */
	int size = (int)m_ppCOpeArr.size();
	for( i = 0; i < size; ++i ){
		delete m_ppCOpeArr[i];
	}
	m_ppCOpeArr.clear();
}


/* 操作の追加 */
bool COpeBlk::AppendOpe( COpe* pcOpe )
{
	if( -1 == pcOpe->m_ptCaretPos_PHY_Before.x	/* カーソル位置 改行単位行先頭からのバイト数（０開始）*/
	 || -1 == pcOpe->m_ptCaretPos_PHY_Before.y	/* カーソル位置 改行単位行の行番号（０開始）*/
	 || -1 == pcOpe->m_ptCaretPos_PHY_After.x		/* カーソル位置 改行単位行先頭からのバイト数（０開始）*/
	 || -1 == pcOpe->m_ptCaretPos_PHY_After.y		/* カーソル位置 改行単位行の行番号（０開始）*/
	){
		TopErrorMessage( NULL,
			_T("COpeBlk::AppendOpe() error.\n")
			_T("バグ\n")
			_T("pcOpe->m_ptCaretPos_PHY_Before.x = %d\n")
			_T("pcOpe->m_ptCaretPos_PHY_Before.y = %d\n")
			_T("pcOpe->m_ptCaretPos_PHY_After.x = %d\n")
			_T("pcOpe->m_ptCaretPos_PHY_After.y = %d\n"),
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


/*[EOF]*/
