/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_WINDOW_A0833476_5E32_46BE_87B6_ECD55F10D34A_H_
#define SAKURA_WINDOW_A0833476_5E32_46BE_87B6_ECD55F10D34A_H_
#pragma once

#include "cxx/ResourceHolder.hpp"

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
			HDC hDC = GetDC(nullptr);
			nDpiX = GetDeviceCaps(hDC, LOGPIXELSX);
			nDpiY = GetDeviceCaps(hDC, LOGPIXELSY);
			ReleaseDC(nullptr, hDC);
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
class CTextWidthCalc final
{
private:
	using FontHolder = cxx::ResourceHolder<&::DeleteObject, HFONT>;
	using MemDcHolder = cxx::ResourceHolder<&::DeleteDC>;
	using SelectionHolder = cxx::ResourceHolder<&::SelectObject>;

	using Me = CTextWidthCalc;

public:
	CTextWidthCalc(HWND hParentDlg, int nID);
	explicit CTextWidthCalc(HWND hWnd);
	explicit CTextWidthCalc(HFONT hFont);
	explicit CTextWidthCalc(_In_opt_ HDC hDC);
	CTextWidthCalc(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CTextWidthCalc(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CTextWidthCalc() = default;

	void Reset(){ nCx = 0; nExt = 0; }
	void SetCx(int cx = 0){ nCx = cx; }
	void SetDefaultExtend(int extCx = 0){ nExt = extCx; }
	bool SetWidthIfMax(int width);
	bool SetWidthIfMax(int width, int extCx);
	bool SetTextWidthIfMax(LPCWSTR pszText);
	bool SetTextWidthIfMax(LPCWSTR pszText, int extCx);
	int GetTextWidth(LPCWSTR pszText) const;
	int GetTextHeight() const;
	HDC GetDC() const { return hDC; }
	int GetCx() const { return nCx; }
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
	MemDcHolder		hDC = nullptr;
	FontHolder		hFont = nullptr;
	SelectionHolder	hFontOld;

	int		nCx = 0;
	int		nExt = 0;
};

/*!
 * @brief フォントハンドルを管理するクラス
 *
 * @note C++にスマートポインターが存在しなかった頃の歴史的遺物。
 */
class CFontAutoDeleter final
{
private:
	using FontHolder = cxx::ResourceHolder<&::DeleteObject, HFONT>;

	using Me = CFontAutoDeleter;

public:
	CFontAutoDeleter() = default;
	CFontAutoDeleter(const Me& other);
	Me& operator = (const Me& other);
	CFontAutoDeleter(Me&& other) noexcept;
	Me& operator = (Me&& other) noexcept;
	~CFontAutoDeleter() noexcept;

	void	SetFont( const HFONT& hFontOld, const HFONT& hFont, const HWND& hWnd );
	void	ReleaseOnDestroy();

	[[nodiscard]] HFONT	GetFont() const { return m_hFont; }

private:
	void	Clear() noexcept;

	FontHolder m_hFont = nullptr;
};

/*!
 * @brief 指定したフォントで描画を行うために必要なリソースを管理するクラス
 *
 * @note 通常の描画でGetDCを呼び出すシーンはない。
 * @note このクラスを利用するコードはイレギュラーである可能性が高い。
 */
class CDCFont final
{
private:
	using FontHolder = cxx::ResourceHolder<&::DeleteObject, HFONT>;
	using SelectionHolder = cxx::ResourceHolder<&::SelectObject>;
	using WindowDcHolder = cxx::ResourceHolder<&::ReleaseDC>;

	using Me = CDCFont;

public:
	explicit CDCFont(const LOGFONT& font, HWND hWnd = nullptr)
		: m_hDC(hWnd)
		, m_hFontOld(m_hDC)
	{
		m_hDC = ::GetWindowDC( hWnd );
		m_hFont = ::CreateFontIndirectW(&font);
		m_hFontOld = ::SelectObject(m_hDC, m_hFont);
	}
	CDCFont(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CDCFont(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CDCFont() = default;

	HDC GetHDC() const { return m_hDC; }

private:
	WindowDcHolder	m_hDC;
	FontHolder		m_hFont;
	SelectionHolder	m_hFontOld;
};

HFONT UpdateDialogFont( HWND hwnd, BOOL force = FALSE );

bool GetSystemAccentColor( COLORREF* pColorOut );

#define PSZ_ARGS(pszText) (pszText), int(pszText ? ::wcslen(pszText) : 0)

#endif /* SAKURA_WINDOW_A0833476_5E32_46BE_87B6_ECD55F10D34A_H_ */
