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
#ifndef SAKURA_DISPPOS_5705EE1C_0A31_419D_81A1_07DA95E57D879_H_
#define SAKURA_DISPPOS_5705EE1C_0A31_419D_81A1_07DA95E57D879_H_

#include "doc/CEditDoc.h"
#include "doc/layout/CLayoutMgr.h"
#include "doc/layout/CLayout.h"

struct DispPos{
public:
	DispPos(int nDx,int nDy)
	: m_nDx(nDx)
	, m_nDy(nDy)
	{
		m_ptDrawOrigin.x=0;
		m_ptDrawOrigin.y=0;
		m_ptDrawLayout.x=CLayoutInt(0);
		m_ptDrawLayout.y=CLayoutInt(0);
		m_nLineRef=CLayoutInt(0);
		//キャッシュ
		m_pcLayoutRef = CEditDoc::GetInstance(0)->m_cLayoutMgr.GetTopLayout();
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         描画位置                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//固定値
	void InitDrawPos(const POINT& pt)
	{
		m_ptDrawOrigin=pt;
		m_ptDrawLayout.x=m_ptDrawLayout.y=CLayoutInt(0);
	}

	//取得
	CMyPoint GetDrawPos() const
	{
		return CMyPoint(
			m_ptDrawOrigin.x + (Int)m_ptDrawLayout.x * m_nDx,
			m_ptDrawOrigin.y + (Int)m_ptDrawLayout.y * m_nDy
		);
	}
	int GetCharWidth() const
	{
		return m_nDx;
	}
	int GetCharHeight() const
	{
		return m_nDy;
	}

	//進む
	void ForwardDrawCol (CLayoutXInt nColOffset ){ m_ptDrawLayout.x += nColOffset; }
	void ForwardDrawLine(int nOffsetLine){ m_ptDrawLayout.y += nOffsetLine; }

	//リセット
	void ResetDrawCol(){ m_ptDrawLayout.x = CLayoutInt(0); }

	//取得
	CLayoutInt GetDrawCol() const{ return m_ptDrawLayout.x; }
	CLayoutInt GetDrawLine() const{ return m_ptDrawLayout.y; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     テキスト参照位置                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//変更
	void SetLayoutLineRef(CLayoutInt nOffsetLine)
	{
		m_nLineRef = nOffsetLine;
		//キャッシュ更新
		m_pcLayoutRef = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( m_nLineRef );
	}
	void ForwardLayoutLineRef(int nOffsetLine);


	//取得
	CLayoutInt		GetLayoutLineRef() const{ return m_nLineRef; }
	const CLayout*	GetLayoutRef() const{ return m_pcLayoutRef; }


private:
	//固定要素
	int				m_nDx;			//半角文字の文字間隔。固定。
	int				m_nDy;			//半角文字の行間隔。固定。
	POINT			m_ptDrawOrigin;	//描画位置基準。単位はピクセル。固定。

	//描画位置
	CLayoutPoint	m_ptDrawLayout; //描画位置。相対レイアウト単位。

	//テキスト参照位置
	CLayoutInt		m_nLineRef; //絶対レイアウト単位。

	//キャッシュ############
	const CLayout*		m_pcLayoutRef;
};

#endif /* SAKURA_DISPPOS_5705EE1C_0A31_419D_81A1_07DA95E57D879_H_ */
/*[EOF]*/
