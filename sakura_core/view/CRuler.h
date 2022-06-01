﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CRULER_2D5B66C2_E499_4C3D_8113_F26B9C28F940_H_
#define SAKURA_CRULER_2D5B66C2_E499_4C3D_8113_F26B9C28F940_H_
#pragma once

class CTextArea;
class CEditView;
class CEditDoc;
class CTextMetrics;
class CGraphics;

class CRuler{
	using Me = CRuler;

public:
	CRuler(const CEditView* pEditView, const CEditDoc* pEditDoc);
	CRuler(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CRuler(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CRuler();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     インターフェース                        //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! ルーラー描画 (背景とキャレット)
	void DispRuler( HDC );

	//! ルーラーの背景のみ描画 2007.08.29 kobake 追加
	void DrawRulerBg(CGraphics& gr);

	void SetRedrawFlag(){ m_bRedrawRuler = true; }
	bool GetRedrawFlag(){ return m_bRedrawRuler; }

private:
	//! ルーラーのキャレットのみ描画 2002.02.25 Add By KK
	void DrawRulerCaret( CGraphics& gr );

	void _DrawRulerCaret( CGraphics& gr, int nCaretDrawX, int nCaretWidth );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	//参照
	const CEditView*	m_pEditView;
	const CEditDoc*		m_pEditDoc;

	//状態
	bool	m_bRedrawRuler;		// ルーラー全体を描き直す時 = true      2002.02.25 Add By KK
	int		m_nOldRulerDrawX;	// 前回描画したルーラーのキャレット位置 2002.02.25 Add By KK  2007.08.26 kobake 名前変更
	int		m_nOldRulerWidth;	// 前回描画したルーラーのキャレット幅   2002.02.25 Add By KK  2007.08.26 kobake 名前変更

	HFONT	m_hFont; // ルーラー上の数字用フォント
	int		m_nRulerHeight;
	std::vector<POINT> m_apt;
	std::vector<DWORD> m_asz;
};
#endif /* SAKURA_CRULER_2D5B66C2_E499_4C3D_8113_F26B9C28F940_H_ */
