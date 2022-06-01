﻿/*!	@file
	@brief アンドゥ・リドゥバッファ

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_COPEBUF_6320338E_171D_4A96_B110_6A9E8407492A_H_
#define SAKURA_COPEBUF_6320338E_171D_4A96_B110_6A9E8407492A_H_
#pragma once

class COpeBuf;

#include <vector>
#include "_main/global.h"
class COpeBlk;/// 2002/2/10 aroka

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief アンドゥ・リドゥバッファ
*/
class COpeBuf {

	using Me = COpeBuf;

public:
	//コンストラクタ・デストラクタ
	COpeBuf();
	COpeBuf(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	COpeBuf(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~COpeBuf();

	//状態
	bool IsEnableUndo() const;					//!< Undo可能な状態か
	bool IsEnableRedo() const;					//!< Redo可能な状態か
	int GetCurrentPointer( void ) const { return m_nCurrentPointer; }	/* 現在位置を返す */	// 2007.12.09 ryoji
	int GetNextSeq() const { return m_nCurrentPointer + 1; }
	int GetNoModifiedSeq() const { return m_nNoModifiedIndex; }

	//操作
	void ClearAll();							//!< 全要素のクリア
	bool AppendOpeBlk( COpeBlk* pcOpeBlk );		//!< 操作ブロックの追加
	void SetNoModified();						//!< 現在位置で無変更な状態になったことを通知

	//使用
	COpeBlk* DoUndo( bool* pbModified );		//!< 現在のUndo対象の操作ブロックを返す
	COpeBlk* DoRedo( bool* pbModified );		//!< 現在のRedo対象の操作ブロックを返す

	//デバッグ
	void DUMP();								//!< 編集操作要素ブロックのダンプ

private:
	std::vector<COpeBlk*>	m_vCOpeBlkArr;		//!< 操作ブロックの配列
	int						m_nCurrentPointer;	//!< 現在位置
	int						m_nNoModifiedIndex;	//!< 無変更な状態になった位置
};
#endif /* SAKURA_COPEBUF_6320338E_171D_4A96_B110_6A9E8407492A_H_ */
