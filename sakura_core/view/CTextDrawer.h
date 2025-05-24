/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CTEXTDRAWER_1AC4291D_DBBD_4A04_8974_26BC505B5318_H_
#define SAKURA_CTEXTDRAWER_1AC4291D_DBBD_4A04_8974_26BC505B5318_H_
#pragma once

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
#endif /* SAKURA_CTEXTDRAWER_1AC4291D_DBBD_4A04_8974_26BC505B5318_H_ */
