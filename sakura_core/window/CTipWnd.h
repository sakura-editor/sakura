/*!	@file
	@brief ツールチップ

	@author Norio Nakatani
	@date 1998/10/30 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, aroka
	Copyright (C) 2006, genta, fon

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CTipWnd;

#ifndef _CTIPWND_H_
#define _CTIPWND_H_

#include "CWnd.h"
#include "mem/CMemory.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CTipWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CTipWnd();
	~CTipWnd();
	void Create( HINSTANCE, HWND );	/* 初期化 */

	/*
	||  Attributes & Operations
	*/
	void Show( int, int, const TCHAR*, RECT* pRect = NULL );	/* Tipを表示 */
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

	CNativeT	m_cInfo;		/* Tipの内容データ */
	bool		m_bAlignLeft;	// 右側揃えでチップを表示

protected:
	/*
	||  実装ヘルパ関数
	*/
	void ComputeWindowSize( HDC, HFONT, const TCHAR*, RECT* );	/* ウィンドウのサイズを決める */
	void DrawTipText( HDC, HFONT, const TCHAR* );	/* ウィンドウのテキストを表示 */

	/* 仮想関数 */
	//	Jan. 9, 2006 genta
	virtual void AfterCreateWindow( void );

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );/* 描画処理 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CTIPWND_H_ */



