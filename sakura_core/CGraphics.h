/*
2008.05.20 kobake 作成
*/

#pragma once

#include <windows.h>
#include <vector>

//! オリジナル値保存クラス
template <class T>
class TOriginalHolder{
public:
	TOriginalHolder<T>()
	{
		m_data = 0;
		m_hold = false;
	}
	void Clear()
	{
		m_data = 0;
		m_hold = false;
	}
	void AssignOnce(const T& t)
	{
		if(!m_hold){
			m_data = t;
			m_hold = true;
		}
	}
	const T& Get() const
	{
		return m_data;
	}
	bool HasData() const
	{
		return m_hold;
	}
private:
	T		m_data;
	bool	m_hold;
};


//! 描画管理
//最新実装：ブラシ
class CGraphics{
public:
	CGraphics(const CGraphics& rhs){ Init(rhs.m_hdc); }
	CGraphics(HDC hdc = NULL){ Init(hdc); }
	~CGraphics();
	void Init(HDC hdc);

	operator HDC() const{ return m_hdc; }

	//クリッピング
private:
	void _InitClipping();
public:
	void PushClipping(const RECT& rc);
	void PopClipping();
	void SetClipping(const RECT& rc);
	void ClearClipping();

	//色設定
public:
	void SetForegroundColor(COLORREF color);	//!< 描画色を設定
	void SetBackgroundColor(COLORREF color);	//!< 背景色を設定

	//テキスト文字色
public:
	void PushTextForeColor(COLORREF color);
	void PopTextForeColor();
	void ClearTextForeColor();
	void SetTextForeColor(COLORREF color);

	//テキスト背景色
public:
	void PushTextBackColor(COLORREF color);
	void PopTextBackColor();
	void ClearTextBackColor();
	void SetTextBackColor(COLORREF color);

	//テキストモード
public:
	void SetTextBackTransparent(bool b);

	//テキスト
public:
	void RestoreTextColors();

	//フォント
public:
	void PushMyFont(HFONT hFont);
	void PopMyFont();
	void ClearMyFont();
	void SetMyFont(HFONT hFont);				//!< フォント設定

	//ペン
public:
	void PushPen(COLORREF color, int nPenWidth, int nStyle = PS_SOLID);
	void PopPen();
	void SetPen(COLORREF color);
	void ClearPen();
	COLORREF GetPenColor() const;

	//ブラシ
public:
	void _InitBrushColor();
	void PushBrushColor(
		COLORREF color	//!< ブラシの色。(COLORREF)-1 にすると、透明ブラシとなる。
	);
	void PopBrushColor();
	void ClearBrush();

	void SetBrushColor(COLORREF color);
	HBRUSH GetCurrentBrush() const{ return m_vBrushes.size()?m_vBrushes.back():NULL; }

	//描画
public:
	void DrawLine(int x1, int y1, int x2, int y2);		//直線
	void DrawDotLine(int x1, int y1, int x2, int y2);	//点線
	void FillMyRect(const RECT& rc);					//矩形塗り潰し
	static void DrawDropRect(LPCRECT lpRectNew, SIZE sizeNew, LPCRECT lpRectLast, SIZE sizeLast);	// ドロップ先の矩形を描画する

private:
	//型
	typedef TOriginalHolder<COLORREF>	COrgColor;
	typedef TOriginalHolder<int>		COrgInt;
private:
	HDC					m_hdc;

	//クリッピング
	std::vector<HRGN>		m_vClippingRgns;

	//テキスト
	std::vector<COLORREF>	m_vTextForeColors;
	std::vector<COLORREF>	m_vTextBackColors;
	std::vector<HFONT>		m_vFonts;

	//テキスト
	COrgInt				m_nTextModeOrg;

	//ペン
	HPEN				m_hpnOrg;
	std::vector<HPEN>	m_vPens;

	//ブラシ
	std::vector<HBRUSH>	m_vBrushes;
	HBRUSH				m_hbrOrg;
	HBRUSH				m_hbrCurrent;
	bool				m_bDynamicBrush;	//m_hbrCurrentを動的に作成した場合はtrue
};
