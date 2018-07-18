/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CVIEWSELECT_F4CBAF6E_90C8_44D2_B6EC_7FE066968A8D9_H_
#define SAKURA_CVIEWSELECT_F4CBAF6E_90C8_44D2_B6EC_7FE066968A8D9_H_

class CEditView;

#include "basis/SakuraBasis.h"
#include "doc/layout/CLayout.h"

class CViewSelect{
public:
	CEditView* GetEditView(){ return m_pcEditView; }
	const CEditView* GetEditView() const{ return m_pcEditView; }

public:
	CViewSelect(CEditView* pcEditView);
	void CopySelectStatus(CViewSelect* pSelect) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      選択範囲の変更                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DisableSelectArea( bool bDraw, bool bDrawBracketCursorLine = true ); //!< 現在の選択範囲を非選択状態に戻す

	void BeginSelectArea( const CLayoutPoint* po = NULL );								// 現在のカーソル位置から選択を開始する
	void ChangeSelectAreaByCurrentCursor( const CLayoutPoint& ptCaretPos );			// 現在のカーソル位置によって選択範囲を変更
	void ChangeSelectAreaByCurrentCursorTEST( const CLayoutPoint& ptCaretPos, CLayoutRange* pSelect );// 現在のカーソル位置によって選択範囲を変更

	//!選択範囲を指定する(原点未選択)
	// 2005.06.24 Moca
	void SetSelectArea( const CLayoutRange& sRange )
	{
		m_sSelectBgn.Set(sRange.GetFrom());
		m_sSelect = sRange;
	}

	//!単語選択開始
	void SelectBeginWord()
	{
		m_bBeginSelect     = true;				/* 範囲選択中 */
		m_bBeginBoxSelect  = false;			/* 矩形範囲選択中でない */
		m_bBeginLineSelect = false;			/* 行単位選択中 */
		m_bBeginWordSelect = true;			/* 単語単位選択中 */
	}

	//!矩形選択開始
	void SelectBeginBox()
	{
		m_bBeginSelect     = true;			/* 範囲選択中 */
		m_bBeginBoxSelect  = true;		/* 矩形範囲選択中 */
		m_bBeginLineSelect = false;		/* 行単位選択中 */
		m_bBeginWordSelect = false;		/* 単語単位選択中 */
	}

	//!謎の選択開始
	void SelectBeginNazo()
	{
		m_bBeginSelect     = true;			/* 範囲選択中 */
//		m_bBeginBoxSelect  = false;		/* 矩形範囲選択中でない */
		m_bBeginLineSelect = false;		/* 行単位選択中 */
		m_bBeginWordSelect = false;		/* 単語単位選択中 */
	}

	//!範囲選択終了
	void SelectEnd()
	{
		m_bBeginSelect = false;
	}

	//!m_bBeginBoxSelectを設定。
	void SetBoxSelect(bool b)
	{
		m_bBeginBoxSelect = b;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void DrawSelectArea(bool bDrawBracketCursorLine = true);		//!< 指定行の選択領域の描画
private:
	void DrawSelectArea2(HDC) const;		//!< 指定範囲の選択領域の描画
	void DrawSelectAreaLine(			//!< 指定行の選択領域の描画
		HDC					hdc,		//!< [in] 描画領域のDevice Context Handle
		CLayoutInt			nLineNum,	//!< [in] 描画対象行(レイアウト行)
		const CLayoutRange&	sRange		//!< [in] 選択範囲(レイアウト単位)
	) const;
public:
	void GetSelectAreaLineFromRange(CLayoutRange& ret, CLayoutInt nLineNum, const CLayout* pcLayout, const CLayoutRange& sRange) const;
	void GetSelectAreaLine(CLayoutRange& ret, CLayoutInt nLineNum, const CLayout* pcLayout) const{
		GetSelectAreaLineFromRange(ret, nLineNum, pcLayout, m_sSelect);
	}
	CLayoutRange GetSelectAreaLine(CLayoutInt nLineNum, const CLayout* pcLayout) const{
		CLayoutRange ret;
		GetSelectAreaLineFromRange(ret, nLineNum, pcLayout, m_sSelect);
		return ret;
	}
	//! 選択情報データの作成	2005.07.09 genta
	void PrintSelectionInfoMsg() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         状態取得                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//! テキストが選択されているか
	// 2002/03/29 Azumaiya インライン関数化
	bool IsTextSelected() const
	{
		return m_sSelect.IsValid();
//		return 0!=(
//			~((DWORD)(m_sSelect.nLineFrom | m_sSelect.nLineTo | m_sSelect.nColumnFrom | m_sSelect.nColumnTo)) >> 31
//			);
	}

	//! テキストの選択中か
	// 2002/03/29 Azumaiya インライン関数化
	bool IsTextSelecting() const
	{
		// ジャンプ回数を減らして、一気に判定。
		return m_bSelectingLock || IsTextSelected();
	}

	//!マウスで選択中か
	bool IsMouseSelecting() const
	{
		return m_bBeginSelect;
	}

	//!矩形選択中か
	bool IsBoxSelecting() const
	{
		return m_bBeginBoxSelect;
	}


private:
	//参照
	CEditView*	m_pcEditView;

public:


	bool	m_bDrawSelectArea;		// 選択範囲を描画したか	// 02/12/13 ai

	// 選択状態
	bool	m_bSelectingLock;		// 選択状態のロック
private:
	bool	m_bBeginSelect;			// 範囲選択中
	bool	m_bBeginBoxSelect;		// 矩形範囲選択中
	bool	m_bSelectAreaChanging;	// 選択範囲変更中
	int		m_nLastSelectedByteLen;	// 前回選択時の選択バイト数

public:
	bool	m_bBeginLineSelect;		// 行単位選択中
	bool	m_bBeginWordSelect;		// 単語単位選択中

	// 選択範囲を保持するための変数群
	// これらはすべて折り返し行と、折り返し桁を保持している。
	CLayoutRange m_sSelectBgn; //範囲選択(原点)
	CLayoutRange m_sSelect;    //範囲選択
	CLayoutRange m_sSelectOld; //範囲選択Old

	CMyPoint	m_ptMouseRollPosOld;	// マウス範囲選択前回位置(XY座標)
};

/*
m_sSelectOldについて
	DrawSelectArea()に現在の選択範囲を教えて差分のみ描画するためのもの
	現在の選択範囲をOldへコピーした上で新しい選択範囲をSelectに設定して
	DrawSelectArea()を呼びだすことで新しい範囲が描かれる．
*/

#endif /* SAKURA_CVIEWSELECT_F4CBAF6E_90C8_44D2_B6EC_7FE066968A8D9_H_ */
/*[EOF]*/
