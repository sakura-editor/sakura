//	$Id$
/************************************************************************

	COpeBuf.cpp

	アンドゥ・リドゥバッファ
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/9  新規作成

************************************************************************/
#include "COpeBuf.h"
#include "debug.h"

/* COpeBufクラス構築 */
COpeBuf::COpeBuf()
{
	m_nCOpeBlkArrNum = 0;	/* 操作ブロックの数 */
	m_ppCOpeBlkArr = NULL;	/* 操作ブロックの配列 */
	m_nCurrentPointer = 0;	/* 現在位置 */
	m_nNoModifiedIndex = 0;	/* 無変更な状態になった位置 */

	return;

}




/* COpeBufクラス消滅 */
COpeBuf::~COpeBuf()
{
	int		i;
	/* 操作ブロックの配列を削除する */
	if( 0 >= m_nCOpeBlkArrNum && NULL != m_ppCOpeBlkArr ){
		return;
	}
	for( i = 0; i < m_nCOpeBlkArrNum; ++i ){
		if( NULL != m_ppCOpeBlkArr[i] ){
			delete m_ppCOpeBlkArr[i];
			m_ppCOpeBlkArr[i] = NULL;
		}
	}
	free( m_ppCOpeBlkArr );
	m_ppCOpeBlkArr = NULL;
	m_nCOpeBlkArrNum = 0;
	return;
}





/* 操作の追加 */
int COpeBuf::AppendOpeBlk( COpeBlk* pcOpeBlk )
{
	int i;
//	pcOpeBlk->DUMP();
	/* 現在位置より後ろ（アンドゥ対象）がある場合は、消去 */
	if( m_nCurrentPointer < m_nCOpeBlkArrNum ){
		for( i = m_nCurrentPointer; i < m_nCOpeBlkArrNum; ++i ){
			if( NULL != m_ppCOpeBlkArr[i] ){
				delete m_ppCOpeBlkArr[i];
				m_ppCOpeBlkArr[i] = NULL;
			}
		}
		m_nCOpeBlkArrNum = m_nCurrentPointer;
		/* 配列の要素数がゼロになった場合は配列のメモリを解放 */
		if( 0 == m_nCOpeBlkArrNum ){
			free( m_ppCOpeBlkArr );
			m_ppCOpeBlkArr = NULL;
		}
	}
	/* 配列のメモリサイズを調整 */
	if( 0 == m_nCOpeBlkArrNum ){
		m_ppCOpeBlkArr = (COpeBlk**)malloc( sizeof( COpeBlk* ) );
	}else{
		m_ppCOpeBlkArr = (COpeBlk**)realloc( (void*)m_ppCOpeBlkArr,  sizeof( COpeBlk* ) * (m_nCOpeBlkArrNum + 1 ) );
	}
	if( NULL == m_ppCOpeBlkArr ){
		::MessageBox( 0, "COpeBlk::AppendOpe() error", "メモリ確保に失敗しました。\n非常に危険な状態です。", MB_OK );
		return FALSE;
	}
	m_ppCOpeBlkArr[m_nCOpeBlkArrNum] = pcOpeBlk;
	m_nCOpeBlkArrNum++;
	m_nCurrentPointer++;
	return TRUE;
}





/* Undo可能な状態か */
int COpeBuf::IsEnableUndo( void )
{
	if( 0 < m_nCOpeBlkArrNum && 0 < m_nCurrentPointer ){
		return TRUE;
	}else{
		return FALSE;
	}
}





/* Redo可能な状態か */
int	COpeBuf::IsEnableRedo( void )
{
	if( 0 < m_nCOpeBlkArrNum && m_nCurrentPointer < m_nCOpeBlkArrNum ){
		return TRUE;
	}else{
		return FALSE;
	}
}





/* 現在のUndo対象の操作ブロックを返す */
COpeBlk* COpeBuf::DoUndo( int* pbModified )
{
	/* Undo可能な状態か */
	if( !IsEnableUndo() ){
		return FALSE;
	}
	m_nCurrentPointer--;
	if( m_nCurrentPointer == m_nNoModifiedIndex ){		/* 無変更な状態になった位置 */
		*pbModified = FALSE;
	}else{
		*pbModified = TRUE;
	}
	return m_ppCOpeBlkArr[m_nCurrentPointer];
}





/* 現在のRedo対象の操作ブロックを返す */
COpeBlk* COpeBuf::DoRedo( int* pbModified )
{
	COpeBlk*	pcOpeBlk;
	/* Redo可能な状態か */
	if( !IsEnableRedo() ){
		return FALSE;
	}
	pcOpeBlk = m_ppCOpeBlkArr[m_nCurrentPointer];
	m_nCurrentPointer++;
	if( m_nCurrentPointer == m_nNoModifiedIndex ){		/* 無変更な状態になった位置 */
		*pbModified = FALSE;
	}else{
		*pbModified = TRUE;
	}
	return pcOpeBlk;
}



/* 現在位置で無変更な状態になったことを通知 */
void COpeBuf::SetNoModified( void )
{
	m_nNoModifiedIndex = m_nCurrentPointer;	/* 無変更な状態になった位置 */
	return;
}


/* アンドゥ・リドゥバッファのダンプ */
void COpeBuf::DUMP( void )
{
#ifdef _DEBUG
	int i;
	MYTRACE( "COpeBuf.m_nCurrentPointer=[%d]----\n", m_nCurrentPointer );
	for( i = 0; i < m_nCOpeBlkArrNum; ++i ){
		MYTRACE( "COpeBuf.m_ppCOpeBlkArr[%d]----\n", i );
		m_ppCOpeBlkArr[i]->DUMP();
	}
	MYTRACE( "COpeBuf.m_nCurrentPointer=[%d]----\n", m_nCurrentPointer );
#endif
	return;
}


/* 全要素のクリア */
void COpeBuf::ClearAll( void )
{
	int		i;
	/* 操作ブロックの配列を削除する */
	if( 0 < m_nCOpeBlkArrNum && NULL != m_ppCOpeBlkArr ){
		for( i = 0; i < m_nCOpeBlkArrNum; ++i ){
			if( NULL != m_ppCOpeBlkArr[i] ){
				delete m_ppCOpeBlkArr[i];
				m_ppCOpeBlkArr[i] = NULL;
			}
		}
		free( m_ppCOpeBlkArr );
		m_ppCOpeBlkArr = NULL;
		m_nCOpeBlkArrNum = 0;
	}
	m_nCurrentPointer = 0;	/* 現在位置 */
	m_nNoModifiedIndex = 0;	/* 無変更な状態になった位置 */
	return;
}

/*[EOF]*/
