﻿/*!	@file
	@brief 編集操作要素ブロック

	@author Norio Nakatani
	@date 1998/06/09 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_COPEBLK_2A5AF589_5860_4512_9E59_51CC2ECC37C1_H_
#define SAKURA_COPEBLK_2A5AF589_5860_4512_9E59_51CC2ECC37C1_H_
#pragma once

class COpeBlk;

#include "COpe.h"
#include <vector>

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 編集操作要素ブロック
	
	COpe を複数束ねるためのもの。Undo, Redoはこのブロック単位で行われる。
*/
class COpeBlk {

	using Me = COpeBlk;

public:
	//コンストラクタ・デストラクタ
	COpeBlk();
	COpeBlk(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	COpeBlk(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~COpeBlk();

	//インターフェース
	int GetNum() const{ return (int)m_ppCOpeArr.size(); }	//!< 操作の数を返す
	bool AppendOpe( COpe* pcOpe );							//!< 操作の追加
	COpe* GetOpe( int nIndex );								//!< 操作を返す
	void AddRef() { m_refCount++; }	//!< 参照カウンタ増加
	int Release() { return m_refCount > 0 ? --m_refCount : 0; }	//!< 参照カウンタ減少
	int GetRefCount() const { return m_refCount; }	//!< 参照カウンタ取得
	int SetRefCount(int val) {  return m_refCount = val > 0? val : 0; }	//!< 参照カウンタ設定

	//デバッグ
	void DUMP();									//!< 編集操作要素ブロックのダンプ

private:
	//メンバ変数
	std::vector<COpe*>	m_ppCOpeArr;	//!< 操作の配列

	//参照カウンタ
	//　HandleCommand内から再帰的にHandleCommandが呼ばれる場合、
	//  内側のHandleCommand終了時にCOpeBlkが破棄されて後続の処理に影響が出るのを防ぐため、
	//　参照カウンタを用いて一番外側のHandleCommand終了時のみCOpeBlkを破棄する。
	//　COpeBlkをnewしたときにAddRef()するのが作法だが、しなくても使える。
	int m_refCount;
};

//////////////////////////////////////////////////////////////////////12
#endif /* SAKURA_COPEBLK_2A5AF589_5860_4512_9E59_51CC2ECC37C1_H_ */
