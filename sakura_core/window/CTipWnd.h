﻿/*!	@file
	@brief ツールチップ

	@author Norio Nakatani
	@date 1998/10/30 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, aroka
	Copyright (C) 2006, genta, fon
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CTIPWND_E3DB6CF2_888C_42F5_953E_D923221F0B16_H_
#define SAKURA_CTIPWND_E3DB6CF2_888C_42F5_953E_D923221F0B16_H_
#pragma once

class CTipWnd;

#include "CWnd.h"
#include "mem/CNativeW.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CTipWnd final : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CTipWnd();
	~CTipWnd();
	void Create( HINSTANCE hInstance, HWND hwndParent );	/* 初期化 */

	/*
	||  Attributes & Operations
	*/
	void Show( int nX, int nY, RECT* pRect = NULL );	/* Tipを表示 */
	void Hide( void );	/* Tipを消す */
	void GetWindowSize(LPRECT pRect);		// 2001/06/19 asa-o ウィンドウのサイズを得る

	void ChangeFont( LOGFONT* lf ){
		if ( m_hFont ){
			::DeleteObject( m_hFont );
		}
		m_hFont = ::CreateFontIndirect( lf );
	}

protected: // 2002/2/10 aroka アクセス権変更
	HFONT		m_hFont;

public:
	CNativeW	m_cKey;			/* キーの内容データ */
	BOOL		m_KeyWasHit;	/* キーがヒットしたか */
	int			m_nSearchLine;	/* 辞書のヒット行 */	// 2006.04.10 fon
	int			m_nSearchDict;	/* ヒット辞書番号 */	// 2006.04.10 fon

	CNativeW	m_cInfo;		/* Tipの内容データ */
	bool		m_bAlignLeft;	// 右側揃えでチップを表示

protected:
	/*
	||  実装ヘルパ関数
	*/
	void ComputeWindowSize( HDC hdc, RECT* prcResult );	/* ウィンドウのサイズを決める */
	void DrawTipText( HDC hdc, const RECT& rcPaint );	/* ウィンドウのテキストを表示 */

	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;/* 描画処理 */

public:
	/* Tipの内容データを取得する */
	const CNativeW& GetInfoText() const { return m_cInfo; }
};

/* Tipの内容データを設定するためにエスケープ解除を行う */
const WCHAR* UnEscapeInfoText( CNativeW& cInfo );

#endif /* SAKURA_CTIPWND_E3DB6CF2_888C_42F5_953E_D923221F0B16_H_ */
