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
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include "COpeBlk.h"
#include "Debug.h"



COpeBlk::COpeBlk()
{
	m_nCOpeArrNum = 0;	/* 操作の数 */
	m_ppCOpeArr = NULL;	/* 操作の配列 */
}

COpeBlk::~COpeBlk()
{
	int		i;
	/* 操作の配列を削除する */
	if( 0 < m_nCOpeArrNum && NULL != m_ppCOpeArr ){
		for( i = 0; i < m_nCOpeArrNum; ++i ){
			if( NULL != m_ppCOpeArr[i] ){
				delete m_ppCOpeArr[i];
				m_ppCOpeArr[i] = NULL;
			}
		}
		free( m_ppCOpeArr );
		m_ppCOpeArr = NULL;
		m_nCOpeArrNum = 0;
	}
}


/* 操作の追加 */
int COpeBlk::AppendOpe( COpe* pcOpe )
{
	if( -1 == pcOpe->m_nCaretPosX_PHY_Before	/* カーソル位置 改行単位行先頭からのバイト数（０開始）*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_Before	/* カーソル位置 改行単位行の行番号（０開始）*/
	 || -1 == pcOpe->m_nCaretPosX_PHY_After		/* カーソル位置 改行単位行先頭からのバイト数（０開始）*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_After		/* カーソル位置 改行単位行の行番号（０開始）*/
	){
		MYMESSAGEBOX( NULL,
			MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
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
	if( 0 == m_nCOpeArrNum ){
		m_ppCOpeArr = (COpe**)malloc( sizeof( COpe* ) );
	}else{
		m_ppCOpeArr = (COpe**)realloc( (void*)m_ppCOpeArr,  sizeof( COpe* ) * (m_nCOpeArrNum + 1 ) );
	}
	if( NULL == m_ppCOpeArr ){
		MessageBox( 0, "COpeBlk::AppendOpe() error", "メモリ確保に失敗しました。\n非常に危険な状態です。", MB_OK );
		return FALSE;
	}
	m_ppCOpeArr[m_nCOpeArrNum] = pcOpe;
	m_nCOpeArrNum++;
	return TRUE;
}


/* 操作を返す */
COpe* COpeBlk::GetOpe( int nIndex )
{
	if( m_nCOpeArrNum <= nIndex ){
		return NULL;
	}
	return m_ppCOpeArr[nIndex];
}


/* 編集操作要素ブロックのダンプ */
void COpeBlk::DUMP( void )
{
#ifdef _DEBUG
	int i;
	for( i = 0; i < m_nCOpeArrNum; ++i ){
		MYTRACE_A( "\tCOpeBlk.m_ppCOpeArr[%d]----\n", i );
		m_ppCOpeArr[i]->DUMP();
	}
#endif
}


/*[EOF]*/
