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
 * @brief 画面 DPI スケーリング
 * @note 96 DPI ピクセルを想定しているデザインをどれだけスケーリングするか
 * 
 * @date 2009.10.01 ryoji 高DPI対応用に作成
 */
class CDPI final {
private:
	using CDpiHolder = std::unique_ptr<CDPI>;

	static constexpr auto DEFAULT_DPI = 96;

	static constexpr auto POINTS_PER_INCH = 72;

	static inline CDpiHolder gm_Instance = nullptr;

	using Me = CDPI;

public:
	static CDPI& Instance();

	CDPI() noexcept;

	LONG ScaleX(LONG x) const noexcept { return ::MulDiv(x, m_DpiX, DEFAULT_DPI); }
	LONG ScaleY(LONG y) const noexcept { return ::MulDiv(y, m_DpiY, DEFAULT_DPI); }
	LONG UnscaleX(LONG x) const noexcept { return ::MulDiv(x, DEFAULT_DPI, m_DpiX); }
	LONG UnscaleY(LONG y) const noexcept { return ::MulDiv(y, DEFAULT_DPI, m_DpiY); }
	LONG PointsToPixels(LONG pt, LONG ptMag = 1) const noexcept { return ::MulDiv(pt, m_DpiY, POINTS_PER_INCH * ptMag); }	// ptMag: 引数のポイント数にかかっている倍率
	LONG PixelsToPoints(LONG px, LONG ptMag = 1) const noexcept { return ::MulDiv(px * ptMag, POINTS_PER_INCH, m_DpiY); }	// ptMag: 戻り値のポイント数にかける倍率

	void ScaleRect(LPRECT lprc) const noexcept;
	void UnscaleRect(LPRECT lprc) const noexcept;

private:
	LONG m_DpiX = DEFAULT_DPI;
	LONG m_DpiY = DEFAULT_DPI;
};

LONG	DpiScaleX(LONG x);
LONG	DpiScaleY(LONG y);
LONG	DpiUnscaleX(LONG x);
LONG	DpiUnscaleY(LONG y);
void	DpiScaleRect(LPRECT lprc);
void	DpiUnscaleRect(LPRECT lprc);
LONG	DpiPointsToPixels(LONG pt, LONG ptMag = 1);
LONG	DpiPixelsToPoints(LONG px, LONG ptMag = 1);

void ActivateFrameWindow(HWND hwnd);	/* アクティブにする */

/*
||	処理中のユーザー操作を可能にする
||	ブロッキングフック(?)(メッセージ配送)
*/
BOOL BlockingHook( HWND hwndDlgCancel );

constexpr int GA_ROOTOWNER2 = 100;

HWND MyGetAncestor( HWND hWnd, UINT gaFlags );	// 指定したウィンドウの祖先のハンドルを取得する	// 2007.07.01 ryoji

namespace apiwrap {

void	CheckDlgButton(HWND hDlg, int nIDButton, bool bCheck = true);
bool	EnableDlgItem(HWND hWndDlg, int nIDDlgItem, bool nEnable = true);
bool	IsDlgButtonChecked(HWND hDlg, int nIDButton);
bool	IsDlgItemEnabled(HWND hWndDlg, int nIDDlgItem);

} // namespace apiwrap

//チェックボックス
inline void CheckDlgButtonBool(HWND hDlg, int nIDButton, bool bCheck)
{
	apiwrap::CheckDlgButton(hDlg,nIDButton, bCheck);
}
inline bool IsDlgButtonCheckedBool(HWND hDlg, int nIDButton)
{
	return apiwrap::IsDlgButtonChecked(hDlg, nIDButton);
}

//ダイアログアイテムの有効化
inline bool DlgItem_Enable(HWND hwndDlg, int nIDDlgItem, bool nEnable)
{
	return apiwrap::EnableDlgItem(hwndDlg, nIDDlgItem, nEnable);
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
