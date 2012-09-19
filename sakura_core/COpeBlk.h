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

class COpeBlk;

#ifndef _COPEBLK_H_
#define _COPEBLK_H_

#include "COpe.h"




/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 編集操作要素ブロック
	
	COpe を複数束ねるためのもの。Undo, Redoはこのブロック単位で行われる。
*/
class COpeBlk {
public:
	//コンストラクタ・デストラクタ
	COpeBlk();
	~COpeBlk();

	//インターフェース
	int GetNum() const{ return m_nCOpeArrNum; }	//!< 操作の数を返す
	int AppendOpe( COpe* pcOpe );							//!< 操作の追加
	COpe* GetOpe( int nIndex );								//!< 操作を返す

	//デバッグ
	void DUMP();									//!< 編集操作要素ブロックのダンプ

private:
	//メンバ変数
	int m_nCOpeArrNum;	//!< 操作の数
	COpe**	m_ppCOpeArr;	//!< 操作の配列
};



//////////////////////////////////////////////////////////////////////12
#endif /* _COPEBLK_H_ */


/*[EOF]*/
