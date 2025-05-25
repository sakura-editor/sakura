/*! @file */
/*
	Copyright (C) 2012, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CAUTOSCROLLWND_F588E196_7D77_4DFA_AAB0_A2D95FFB8849_H_
#define SAKURA_CAUTOSCROLLWND_F588E196_7D77_4DFA_AAB0_A2D95FFB8849_H_
#pragma once

#include "CWnd.h"

class CEditView;
class CMyPoint;

class CAutoScrollWnd final: public CWnd
{
public:
	CAutoScrollWnd();
	virtual ~CAutoScrollWnd();
	HWND Create( HINSTANCE hInstance, HWND hwndParent, bool bVertical, bool bHorizontal,
				 const CMyPoint& point, CEditView* view );
	void Close();

private:
	HBITMAP	m_hCenterImg;
	CEditView*	m_cView;
protected:
	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnLButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;
	LRESULT OnRButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;
	LRESULT OnMButtonDown(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;
	LRESULT OnPaint(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;
};
#endif /* SAKURA_CAUTOSCROLLWND_F588E196_7D77_4DFA_AAB0_A2D95FFB8849_H_ */
