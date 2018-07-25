/*!	@file
	@brief アンドゥ・リドゥバッファ

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "COpeBuf.h"
#include "COpeBlk.h"// 2002/2/10 aroka


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* COpeBufクラス構築 */
COpeBuf::COpeBuf()
{
	m_nCurrentPointer = 0;	/* 現在位置 */
	m_nNoModifiedIndex = 0;	/* 無変更な状態になった位置 */
}

/* COpeBufクラス消滅 */
COpeBuf::~COpeBuf()
{
	/* 操作ブロックの配列を削除する */
	int size = (int)m_vCOpeBlkArr.size();
	for( int i = 0; i < size; ++i ){
		SAFE_DELETE(m_vCOpeBlkArr[i]);
	}
	m_vCOpeBlkArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           状態                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* Undo可能な状態か */
bool COpeBuf::IsEnableUndo() const
{
	return 0 < m_vCOpeBlkArr.size() && 0 < m_nCurrentPointer;
}

/* Redo可能な状態か */
bool COpeBuf::IsEnableRedo() const
{
	return 0 < m_vCOpeBlkArr.size() && m_nCurrentPointer < (int)m_vCOpeBlkArr.size();
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           操作                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 操作の追加 */
bool COpeBuf::AppendOpeBlk( COpeBlk* pcOpeBlk )
{
	/* 現在位置より後ろ（アンドゥ対象）がある場合は、消去 */
	int size = (int)m_vCOpeBlkArr.size();
	if( m_nCurrentPointer < size ){
		for( int i = m_nCurrentPointer; i < size; ++i ){
			SAFE_DELETE(m_vCOpeBlkArr[i]);
		}
		m_vCOpeBlkArr.resize(m_nCurrentPointer);
	}
	/* 配列のメモリサイズを調整 */
	m_vCOpeBlkArr.push_back(pcOpeBlk);
	m_nCurrentPointer++;
	return true;
}

/* 全要素のクリア */
void COpeBuf::ClearAll()
{
	/* 操作ブロックの配列を削除する */
	int size = (int)m_vCOpeBlkArr.size();
	for( int i = 0; i < size; ++i ){
		SAFE_DELETE(m_vCOpeBlkArr[i]);
	}
	m_vCOpeBlkArr.clear();
	m_nCurrentPointer = 0;	/* 現在位置 */
	m_nNoModifiedIndex = 0;	/* 無変更な状態になった位置 */
}

/* 現在位置で無変更な状態になったことを通知 */
void COpeBuf::SetNoModified()
{
	m_nNoModifiedIndex = m_nCurrentPointer;	/* 無変更な状態になった位置 */
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           使用                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 現在のUndo対象の操作ブロックを返す */
COpeBlk* COpeBuf::DoUndo( bool* pbModified )
{
	/* Undo可能な状態か */
	if( !IsEnableUndo() ){
		return NULL;
	}
	m_nCurrentPointer--;
	if( m_nCurrentPointer == m_nNoModifiedIndex ){		/* 無変更な状態になった位置 */
		*pbModified = false;
	}else{
		*pbModified = true;
	}
	return m_vCOpeBlkArr[m_nCurrentPointer];
}

/* 現在のRedo対象の操作ブロックを返す */
COpeBlk* COpeBuf::DoRedo( bool* pbModified )
{
	COpeBlk*	pcOpeBlk;
	/* Redo可能な状態か */
	if( !IsEnableRedo() ){
		return NULL;
	}
	pcOpeBlk = m_vCOpeBlkArr[m_nCurrentPointer];
	m_nCurrentPointer++;
	if( m_nCurrentPointer == m_nNoModifiedIndex ){		/* 無変更な状態になった位置 */
		*pbModified = false;
	}else{
		*pbModified = true;
	}
	return pcOpeBlk;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* アンドゥ・リドゥバッファのダンプ */
void COpeBuf::DUMP()
{
#ifdef _DEBUG
	int i;
	MYTRACE( _T("COpeBuf.m_nCurrentPointer=[%d]----\n"), m_nCurrentPointer );
	int size = (int)m_vCOpeBlkArr.size();
	for( i = 0; i < size; ++i ){
		MYTRACE( _T("COpeBuf.m_vCOpeBlkArr[%d]----\n"), i );
		m_vCOpeBlkArr[i]->DUMP();
	}
	MYTRACE( _T("COpeBuf.m_nCurrentPointer=[%d]----\n"), m_nCurrentPointer );
#endif
}



