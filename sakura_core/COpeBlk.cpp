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
	for( i = 0; i < (int)m_ppCOpeArr.size(); ++i ){
		delete m_ppCOpeArr[i];
	}
	m_ppCOpeArr.clear();
}


/* 操作の追加 */
bool COpeBlk::AppendOpe( COpe* pcOpe )
{
	if( -1 == pcOpe->m_nCaretPosX_PHY_Before	/* カーソル位置 改行単位行先頭からのバイト数（０開始）*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_Before	/* カーソル位置 改行単位行の行番号（０開始）*/
	 || -1 == pcOpe->m_nCaretPosX_PHY_After		/* カーソル位置 改行単位行先頭からのバイト数（０開始）*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_After		/* カーソル位置 改行単位行の行番号（０開始）*/
	){
		TopErrorMessage( NULL,
			_T("COpeBlk::AppendOpe() error.\n")
			_T("バグ\n")
			_T("pcOpe->m_nCaretPosX_PHY_Before = %d\n")
			_T("pcOpe->m_nCaretPosY_PHY_Before = %d\n")
			_T("pcOpe->m_nCaretPosX_PHY_After = %d\n")
			_T("pcOpe->m_nCaretPosY_PHY_After = %d\n"),
			pcOpe->m_nCaretPosX_PHY_Before,
			pcOpe->m_nCaretPosY_PHY_Before,
			pcOpe->m_nCaretPosX_PHY_After,
			pcOpe->m_nCaretPosY_PHY_After
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
	for( i = 0; i < GetNum(); ++i ){
		MYTRACE( _T("\tCOpeBlk.m_ppCOpeArr[%d]----\n"), i );
		m_ppCOpeArr[i]->DUMP();
	}
#endif
}


/*[EOF]*/
