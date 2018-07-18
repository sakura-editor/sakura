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
#ifndef SAKURA_CTEXTAREA_BE5C17FA_E8D8_4659_9AA4_552DF90288CC9_H_
#define SAKURA_CTEXTAREA_BE5C17FA_E8D8_4659_9AA4_552DF90288CC9_H_

class CViewFont;
class CEditView;
class CLayoutMgr;
#include "DispPos.h"


class CTextArea{
public:
	CTextArea(CEditView* pEditView);
	virtual ~CTextArea();
	void CopyTextAreaStatus(CTextArea* pDst) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     ビュー情報を取得                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//!表示される最初の行
	CLayoutInt GetViewTopLine() const
	{
		return m_nViewTopLine;
	}
	void SetViewTopLine(CLayoutInt nLine)
	{
		m_nViewTopLine=nLine;
	}

	//!表示域の一番左の桁
	CLayoutInt GetViewLeftCol() const
	{
		return m_nViewLeftCol;
	}
	void SetViewLeftCol(CLayoutInt nLeftCol)
	{
		m_nViewLeftCol=nLeftCol;
	}

	//!右にはみ出した最初の列を返す
	CLayoutInt GetRightCol() const
	{
		return m_nViewLeftCol + m_nViewColNum;
	}

	//!下にはみ出した最初の行を返す
	CLayoutInt GetBottomLine() const
	{
		return m_nViewTopLine + m_nViewRowNum;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                   領域を取得(ピクセル)                      //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int GetAreaLeft() const
	{
		return m_nViewAlignLeft;
	}
	int GetAreaTop() const
	{
		return m_nViewAlignTop;
	}
	int GetAreaRight() const
	{
		return m_nViewAlignLeft + m_nViewCx;
	}
	int GetAreaBottom() const
	{
		return m_nViewAlignTop + m_nViewCy;
	}
	CMyRect GetAreaRect() const
	{
		return CMyRect(GetAreaLeft(), GetAreaTop(), GetAreaRight(), GetAreaBottom());
	}

	int GetAreaWidth() const
	{
		return m_nViewCx;
	}
	int GetAreaHeight() const
	{
		return m_nViewCy;
	}

	int GetTopYohaku() const
	{
		return m_nTopYohaku;
	}
	void SetTopYohaku(int nPixel)
	{
		m_nTopYohaku=nPixel;
	}
	int GetLeftYohaku() const
	{
		return m_nLeftYohaku;
	}
	void SetLeftYohaku(int nPixel)
	{
		m_nLeftYohaku=nPixel;
	}
	// 行番号の幅(余白なし)
	int GetLineNumberWidth() const
	{
		return m_nViewAlignLeft - m_nLeftYohaku;
	}

	//! クライアントサイズ更新
	void TextArea_OnSize(
		const CMySize& sizeClient, //!< ウィンドウのクライアントサイズ
		int nCxVScroll,            //!< 垂直スクロールバーの横幅
		int nCyHScroll             //!< 水平スクロールバーの縦幅
	);

	//! 行番号表示に必要な幅を設定
	bool DetectWidthOfLineNumberArea( bool bRedraw );

	//! 行番号表示に必要な桁数を計算
	int  DetectWidthOfLineNumberArea_calculate(const CLayoutMgr*, bool bLayout=false) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           判定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	bool IsRectIntersected(const RECT& rc) const
	{
		//rcが無効またはゼロ領域の場合はfalse
		if( rc.left >= rc.right )return false;
		if( rc.top  >= rc.bottom )return false;

		if( rc.left >= this->GetAreaRight() )return false; //右外
		if( rc.right <= this->GetAreaLeft() )return false; //左外
		if( rc.top >= this->GetAreaBottom() )return false; //下外
		if( rc.bottom <= this->GetAreaTop() )return false; //上外
		
		return true;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        その他取得                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	int GetRulerHeight() const
	{
		return m_nViewAlignTop - GetTopYohaku();
	}
	//! ドキュメント左端のクライアント座標を取得 (つまり、スクロールされた状態であれば、マイナスを返す)
	int GetDocumentLeftClientPointX() const;

	//計算
	//! クライアント座標からレイアウト位置に変換する
	void ClientToLayout(CMyPoint ptClient, CLayoutPoint* pptLayout) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void UpdateAreaMetrics();
	void SetAreaLeft(int nAreaLeft)
	{
		m_nViewAlignLeft = nAreaLeft;
	}
	void SetAreaTop(int nAreaTop)
	{
		m_nViewAlignTop = nAreaTop;
	}
	void OffsetViewTopLine(CLayoutInt nOff)
	{
		m_nViewTopLine += nOff;
	}
protected:
	void UpdateViewColRowNums();

public:


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         サポート                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//$ Generateなんていう大げさな名前じゃなくて、Get～で良い気がしてきた
	//!クリッピング矩形を作成。表示範囲外だった場合はfalseを返す。
	void GenerateCharRect(RECT* rc,const DispPos& sPos,CLayoutXInt nColumns) const;
	bool TrimRectByArea(RECT* rc) const;
	bool GenerateClipRect(RECT* rc,const DispPos& sPos,CLayoutXInt nColumns) const;
	bool GenerateClipRectRight(RECT* rc,const DispPos& sPos) const; //!< 右端まで全部
	bool GenerateClipRectLine(RECT* rc,const DispPos& sPos) const;  //!< 行全部

	void GenerateTopRect   (RECT* rc, CLayoutInt nLineCount) const;
	void GenerateBottomRect(RECT* rc, CLayoutInt nLineCount) const;
	void GenerateLeftRect  (RECT* rc, CLayoutInt nColCount ) const;
	void GenerateRightRect (RECT* rc, CLayoutInt nColCount ) const;

	void GenerateLineNumberRect(RECT* rc) const;

	void GenerateTextAreaRect(RECT* rc) const;

	int GenerateYPx(CLayoutYInt nLineNum) const;

private:
	//参照
	CEditView*	m_pEditView;

public:
	/* 画面情報 */
	//ピクセル
private:
	int		m_nViewAlignLeft;		/* 表示域の左端座標 */
	int		m_nViewAlignTop;		/* 表示域の上端座標 */
private:
	int		m_nTopYohaku;
	int		m_nLeftYohaku;
private:
	int		m_nViewCx;				/* 表示域の幅 */
	int		m_nViewCy;				/* 表示域の高さ */

	//テキスト
private:
	CLayoutInt	m_nViewTopLine;			/* 表示域の一番上の行(0開始) */
public:
	CLayoutInt	m_nViewRowNum;			/* 表示域の行数 */

private:
	CLayoutInt	m_nViewLeftCol;			/* 表示域の一番左の桁(0開始) */
public:
	CLayoutInt	m_nViewColNum;			/* 表示域の桁数 */

	//その他
	int		m_nViewAlignLeftCols;	/* 行番号域の桁数 */
};

#endif /* SAKURA_CTEXTAREA_BE5C17FA_E8D8_4659_9AA4_552DF90288CC9_H_ */
/*[EOF]*/
