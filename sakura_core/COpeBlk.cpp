//	$Id$
/*!	@file
	@brief 編集操作要素ブロック

	@author Norio Nakatani
	@date 1998/06/09 新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include "COpeBlk.h"
#include "debug.h"



/* COpeBlkクラス構築 */
COpeBlk::COpeBlk()
{
	Init();
	return;
}

void COpeBlk::Init( void )
{
	m_nCOpeArrNum = 0;	/* 操作の数 */
	m_ppCOpeArr = NULL;	/* 操作の配列 */
}

void COpeBlk::Empty( void )
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
	return;
}



/* COpeBlkクラス消滅 */
COpeBlk::~COpeBlk()
{
	Empty();

}





/* 操作の追加 */
int COpeBlk::AppendOpe( COpe* pcOpe/*, CLayoutMgr* pCLayoutMgr*/ )
{
	if( -1 == pcOpe->m_nCaretPosX_PHY_Before	/* カーソル位置 改行単位行先頭からのバイト数（０開始）*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_Before	/* カーソル位置 改行単位行の行番号（０開始）*/
	 || -1 == pcOpe->m_nCaretPosX_PHY_After		/* カーソル位置 改行単位行先頭からのバイト数（０開始）*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_After		/* カーソル位置 改行単位行の行番号（０開始）*/
	){
//		MessageBox( 0, "COpeBlk::AppendOpe() error", "バグ", MB_OK );
		MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			"COpeBlk::AppendOpe() error.\n バグ\n pcOpe->m_nCaretPosX_PHY_Before = %d\npcOpe->m_nCaretPosY_PHY_Before = %d\npcOpe->m_nCaretPosX_PHY_After = %d\npcOpe->m_nCaretPosY_PHY_After = %d\n",
			pcOpe->m_nCaretPosX_PHY_Before,
			pcOpe->m_nCaretPosY_PHY_Before,
			pcOpe->m_nCaretPosX_PHY_After,
			pcOpe->m_nCaretPosY_PHY_After
		);
	}
//	/*
//	  カーソル位置変換
//	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
//	  →
//	  物理位置(行頭からのバイト数、折り返し無し行位置)
//	*/
//	int		nX;
//	int		nY;
//	pCLayoutMgr->CaretPos_Log2Phys(
//		pcOpe->m_nCaretPosX_Before,
//		pcOpe->m_nCaretPosY_Before,
//		&pcOpe->m_nCaretPosX_PHY_Before,
//		&pcOpe->m_nCaretPosY_PHY_Before
//	);
//	pCLayoutMgr->CaretPos_Log2Phys(
//		pcOpe->m_nCaretPosX_After,
//		pcOpe->m_nCaretPosY_After,
//		&pcOpe->m_nCaretPosX_PHY_After,
//		&pcOpe->m_nCaretPosY_PHY_After
//	);


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
		MYTRACE( "\tCOpeBlk.m_ppCOpeArr[%d]----\n", i );
		m_ppCOpeArr[i]->DUMP();
	}
#endif
	return;
}


/*[EOF]*/
