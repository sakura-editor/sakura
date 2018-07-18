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
#ifndef SAKURA_CTEXTDRAWER_AECD0708_CE8E_489C_A9ED_484CBAB9523E9_H_
#define SAKURA_CTEXTDRAWER_AECD0708_CE8E_489C_A9ED_484CBAB9523E9_H_

class CTextMetrics;
class CTextArea;
class CViewFont;
class CEol;
class CEditView;
class CLayout;
#include "DispPos.h"

class CGraphics;

class CTextDrawer{
public:
	CTextDrawer(const CEditView* pEditView) : m_pEditView(pEditView) { }
	virtual ~CTextDrawer(){}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         外部依存                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//領域のインスタンスを求める
	const CTextArea* GetTextArea() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     インターフェース                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//2007.08.25 kobake 戻り値を void に変更。引数 x, y を DispPos に変更
	//実際には pX と nX が更新される。
	void DispText( HDC hdc, DispPos* pDispPos, int marginy, const wchar_t* pData, int nLength, bool bTransparent = false ) const; // テキスト表示

	//!	ノート線描画
	void DispNoteLine( CGraphics& gr, int nTop, int nBottom, int nLeft, int nRight ) const;

	// -- -- 指定桁縦線描画 -- -- //
	//!	指定桁縦線描画関数	// 2005.11.08 Moca
	void DispVerticalLines( CGraphics& gr, int nTop, int nBottom, CLayoutInt nLeftCol, CLayoutInt nRightCol ) const;

	// -- -- 折り返し桁縦線描画 -- -- //
	void DispWrapLine( CGraphics& gr, int nTop, int nBottom ) const;

	// -- -- 行番号 -- -- //
	void DispLineNumber( CGraphics& gr, CLayoutInt nLineNum, int y ) const;		// 行番号表示

private:
	const CEditView* m_pEditView;
};

#endif /* SAKURA_CTEXTDRAWER_AECD0708_CE8E_489C_A9ED_484CBAB9523E9_H_ */
/*[EOF]*/
