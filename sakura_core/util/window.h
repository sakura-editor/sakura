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
#ifndef SAKURA_WINDOW_A0833476_5E32_46BE_87B6_ECD55F10D34A_H_
#define SAKURA_WINDOW_A0833476_5E32_46BE_87B6_ECD55F10D34A_H_
#pragma once

/*!
	@brief 画面 DPI スケーリング
	@note 96 DPI ピクセルを想定しているデザインをどれだけスケーリングするか

	@date 2009.10.01 ryoji 高DPI対応用に作成
*/
class CDPI{
	static void Init()
	{
		if( !bInitialized )
		{
			HDC hDC = GetDC(NULL);
			nDpiX = GetDeviceCaps(hDC, LOGPIXELSX);
			nDpiY = GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(NULL, hDC);
			bInitialized = true;
		}
	}
	static int nDpiX;
	static int nDpiY;
	static bool bInitialized;
public:
	static int ScaleX(int x){Init(); return ::MulDiv(x, nDpiX, 96);}
	static int ScaleY(int y){Init(); return ::MulDiv(y, nDpiY, 96);}
	static int UnscaleX(int x){Init(); return ::MulDiv(x, 96, nDpiX);}
	static int UnscaleY(int y){Init(); return ::MulDiv(y, 96, nDpiY);}
	static void ScaleRect(LPRECT lprc)
	{
		lprc->left = ScaleX(lprc->left);
		lprc->right = ScaleX(lprc->right);
		lprc->top = ScaleY(lprc->top);
		lprc->bottom = ScaleY(lprc->bottom);
	}
	static void UnscaleRect(LPRECT lprc)
	{
		lprc->left = UnscaleX(lprc->left);
		lprc->right = UnscaleX(lprc->right);
		lprc->top = UnscaleY(lprc->top);
		lprc->bottom = UnscaleY(lprc->bottom);
	}
	static int PointsToPixels(int pt, int ptMag = 1){Init(); return ::MulDiv(pt, nDpiY, 72 * ptMag);}	// ptMag: 引数のポイント数にかかっている倍率
	static int PixelsToPoints(int px, int ptMag = 1){Init(); return ::MulDiv(px * ptMag, 72, nDpiY);}	// ptMag: 戻り値のポイント数にかける倍率
};

inline int DpiScaleX(int x){return CDPI::ScaleX(x);}
inline int DpiScaleY(int y){return CDPI::ScaleY(y);}
inline int DpiUnscaleX(int x){return CDPI::UnscaleX(x);}
inline int DpiUnscaleY(int y){return CDPI::UnscaleY(y);}
inline void DpiScaleRect(LPRECT lprc){CDPI::ScaleRect(lprc);}
inline void DpiUnscaleRect(LPRECT lprc){CDPI::UnscaleRect(lprc);}
inline int DpiPointsToPixels(int pt, int ptMag = 1){return CDPI::PointsToPixels(pt, ptMag);}
inline int DpiPixelsToPoints(int px, int ptMag = 1){return CDPI::PixelsToPoints(px, ptMag);}

void ActivateFrameWindow(HWND hwnd);	/* アクティブにする */

/*
||	処理中のユーザー操作を可能にする
||	ブロッキングフック(?)(メッセージ配送)
*/
BOOL BlockingHook( HWND hwndDlgCancel );

#define GA_ROOTOWNER2	100

HWND MyGetAncestor( HWND hWnd, UINT gaFlags );	// 指定したウィンドウの祖先のハンドルを取得する	// 2007.07.01 ryoji

//チェックボックス
inline void CheckDlgButtonBool(HWND hDlg, int nIDButton, bool bCheck)
{
	CheckDlgButton(hDlg,nIDButton,bCheck?BST_CHECKED:BST_UNCHECKED);
}
inline bool IsDlgButtonCheckedBool(HWND hDlg, int nIDButton)
{
	return (IsDlgButtonChecked(hDlg,nIDButton) & BST_CHECKED) != 0;
}

//ダイアログアイテムの有効化
inline bool DlgItem_Enable(HWND hwndDlg, int nIDDlgItem, bool nEnable)
{
	return FALSE != ::EnableWindow( ::GetDlgItem(hwndDlg, nIDDlgItem), nEnable?TRUE:FALSE);
}

// 幅計算補助クラス
// 最大の幅を報告します
class CTextWidthCalc
{
	using Me = CTextWidthCalc;

public:
	CTextWidthCalc(HWND hParentDlg, int nID);
	CTextWidthCalc(HWND hwndThis);
	CTextWidthCalc(HFONT font);
	CTextWidthCalc(HDC hdc);
	CTextWidthCalc(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CTextWidthCalc(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	virtual ~CTextWidthCalc();
	void Reset(){ nCx = 0; nExt = 0; }
	void SetCx(int cx = 0){ nCx = cx; }
	void SetDefaultExtend(int extCx = 0){ nExt = 0; }
	bool SetWidthIfMax(int width);
	bool SetWidthIfMax(int width, int extCx);
	bool SetTextWidthIfMax(LPCWSTR pszText);
	bool SetTextWidthIfMax(LPCWSTR pszText, int extCx);
	int GetTextWidth(LPCWSTR pszText) const;
	int GetTextHeight() const;
	HDC GetDC() const{ return hDC; }
	int GetCx(){ return nCx; }
	// 算出方法がよく分からないので定数にしておく
	// 制御不要なら ListViewはLVSCW_AUTOSIZE等推奨
	enum StaticMagicNambers{
		//! スクロールバーとアイテムの間の隙間
		WIDTH_MARGIN_SCROLLBER = 8,
		//! リストビューヘッダー マージン
		WIDTH_LV_HEADER = 17,
		//! リストビューのマージン
		WIDTH_LV_ITEM_NORMAL  = 14,
		//! リストビューのチェックボックスとマージンの幅
		WIDTH_LV_ITEM_CHECKBOX = 30,
	};
private:
	HWND  hwnd;
	HDC   hDC;
	HFONT hFont;
	HFONT hFontOld;
	int nCx;
	int nExt;
	bool  bHDCComp;
	bool  bFromDC;
};

class CFontAutoDeleter
{
private:
	HFONT m_hFont = nullptr;

	using Me = CFontAutoDeleter;

	void	Clear() noexcept;

public:
	CFontAutoDeleter() = default;
	CFontAutoDeleter(const Me& other);
	Me& operator = (const Me& other);
	CFontAutoDeleter(Me&& other) noexcept;
	Me& operator = (Me&& other) noexcept;
	virtual ~CFontAutoDeleter() noexcept;

	void	SetFont( const HFONT& hFontOld, const HFONT& hFont, const HWND& hWnd );
	void	ReleaseOnDestroy();

	[[nodiscard]] HFONT	GetFont() const { return m_hFont; }
};

class CDCFont
{
	using Me = CDCFont;

public:
	CDCFont(LOGFONT& font, HWND hwnd = NULL){
		m_hwnd = hwnd;
		m_hDC = ::GetDC(hwnd);
		m_hFont = ::CreateFontIndirect(&font);
		m_hFontOld = (HFONT)::SelectObject(m_hDC, m_hFont);
	}
	CDCFont(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CDCFont(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CDCFont(){
		if( m_hDC ){
			::SelectObject(m_hDC, m_hFontOld);
			::ReleaseDC(m_hwnd, m_hDC);
			m_hDC = NULL;
			::DeleteObject(m_hFont);
			m_hFont = NULL;
		}
	}
	HDC GetHDC(){ return m_hDC; }
private:
	HWND  m_hwnd;
	HDC   m_hDC;
	HFONT m_hFontOld;
	HFONT m_hFont;
};

HFONT UpdateDialogFont( HWND hwnd, BOOL force = FALSE );

bool GetSystemAccentColor( COLORREF* pColorOut );

#endif /* SAKURA_WINDOW_A0833476_5E32_46BE_87B6_ECD55F10D34A_H_ */
