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
#ifndef SAKURA_WINDOW_E7B899CD_2106_4A3B_BBA1_EB29FD9640F39_H_
#define SAKURA_WINDOW_E7B899CD_2106_4A3B_BBA1_EB29FD9640F39_H_

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

void ActivateFrameWindow( HWND );	/* アクティブにする */

/*
||	処理中のユーザー操作を可能にする
||	ブロッキングフック(?)(メッセージ配送)
*/
BOOL BlockingHook( HWND hwndDlgCancel );


#ifndef GA_PARENT
#define GA_PARENT		1
#define GA_ROOT			2
#define GA_ROOTOWNER	3
#endif
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
public:
	CTextWidthCalc(HWND hParentDlg, int nID);
	CTextWidthCalc(HWND hwndThis);
	CTextWidthCalc(HFONT font);
	CTextWidthCalc(HDC hdc);
	virtual ~CTextWidthCalc();
	void Reset(){ nCx = 0; nExt = 0; }
	void SetCx(int cx = 0){ nCx = cx; }
	void SetDefaultExtend(int extCx = 0){ nExt = 0; }
	bool SetWidthIfMax(int width);
	bool SetWidthIfMax(int width, int extCx);
	bool SetTextWidthIfMax(LPCTSTR pszText);
	bool SetTextWidthIfMax(LPCTSTR pszText, int extCx);
	int GetTextWidth(LPCTSTR pszText) const;
	int GetTextHeight() const;
	HDC GetDC() const{ return hDC; }
	int GetCx(){ return nCx; }
	// 算出方法がよく分からないので定数にしておく
	// 制御不要なら ListViewはLVSCW_AUTOSIZE等推奨
	enum StaticMagicNambers{
		//! スクロールバーとアイテムの間の隙間
		WIDTH_MARGIN_SCROLLBER = 8,
		//! リストビューヘッダ マージン
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
public:
	CFontAutoDeleter();
	~CFontAutoDeleter();
	void SetFont( HFONT hfontOld, HFONT hfont, HWND hwnd );
	void ReleaseOnDestroy();
	// void Release();

private:
	HFONT m_hFontOld;
	HFONT m_hFont;
	HWND  m_hwnd;
};

class CDCFont
{
public:
	CDCFont(LOGFONT& font, HWND hwnd = NULL){
		m_hwnd = hwnd;
		m_hDC = ::GetDC(hwnd);
		m_hFont = ::CreateFontIndirect(&font);
		m_hFontOld = (HFONT)::SelectObject(m_hDC, m_hFont);
	}
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

#endif /* SAKURA_WINDOW_E7B899CD_2106_4A3B_BBA1_EB29FD9640F39_H_ */
/*[EOF]*/
