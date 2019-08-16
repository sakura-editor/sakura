/*! @file */
/*
2008.05.20 kobake 作成
*/

#include "StdAfx.h"
#include "CGraphics.h"
#include "util/std_macro.h"

class CGDIStock
{
public:
	CGDIStock(){}
	~CGDIStock()
	{
		while(!m_vObjects.empty()){
			::DeleteObject(m_vObjects.back());
			m_vObjects.pop_back();
		}
	}
	bool Register(HGDIOBJ hObject)
	{
		if(hObject){
			m_vObjects.push_back(hObject);
			return true;
		}
		return false;
	}
protected:
	std::vector<HGDIOBJ> m_vObjects;
};

static CGDIStock s_cGDIStock;	// 唯一の CGDIStock オブジェクト

void CGraphics::Init(HDC hdc)
{
	m_hdc = hdc;
	m_hbrCurrent = NULL;
	m_bDynamicBrush = false;
}

CGraphics::~CGraphics()
{
	ClearClipping();
	ClearMyFont();
	ClearPen();
	ClearBrush();
	if (m_hdcOrg && m_dcState) {
		::RestoreDC(m_hdcOrg, m_dcState);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       クリッピング                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushClipping(const RECT& rc)
{
	//新しく作成→HDCに設定→スタックに保存
	HRGN hrgnNew = CreateRectRgnIndirect(&rc);
	::SelectClipRgn(m_hdc,hrgnNew);
	m_vClippingRgns.push_back(hrgnNew);
}

void CGraphics::PopClipping()
{
	if(m_vClippingRgns.size()){
		//最後の要素を削除
		::DeleteObject(m_vClippingRgns.back());
		m_vClippingRgns.pop_back();
		if(m_vClippingRgns.size()){
			//この時点の最後の要素をHDCに設定
			::SelectClipRgn(m_hdc,m_vClippingRgns.back());
		}else{
			::SelectClipRgn(m_hdc, NULL);
		}
	}
}

void CGraphics::ClearClipping()
{
	//領域をすべて削除
	size_t nSize = m_vClippingRgns.size();
	for(size_t i=0;i<nSize;i++){
		::DeleteObject(m_vClippingRgns[i]);
	}
	m_vClippingRgns.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキスト文字色                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushTextForeColor(COLORREF color)
{
	//設定
	COLORREF cOld = ::SetTextColor(m_hdc,color);
	//記録
	if(m_vTextForeColors.empty()){
		m_vTextForeColors.push_back(cOld);
	}
	m_vTextForeColors.push_back(color);
}

void CGraphics::PopTextForeColor()
{
	//戻す
	if(m_vTextForeColors.size()>=2){
		m_vTextForeColors.pop_back();
		::SetTextColor(m_hdc,m_vTextForeColors.back());
	}
}

void CGraphics::ClearTextForeColor()
{
	if(!m_vTextForeColors.empty()){
		::SetTextColor(m_hdc,m_vTextForeColors[0]);
		m_vTextForeColors.clear();
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキスト背景色                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushTextBackColor(COLORREF color)
{
	//設定
	COLORREF cOld = ::SetBkColor(m_hdc,color);
	//記録
	if(m_vTextBackColors.empty()){
		m_vTextBackColors.push_back(cOld);
	}
	m_vTextBackColors.push_back(color);
}

void CGraphics::PopTextBackColor()
{
	//戻す
	if(m_vTextBackColors.size()>=2){
		m_vTextBackColors.pop_back();
		::SetBkColor(m_hdc,m_vTextBackColors.back());
	}
}

void CGraphics::ClearTextBackColor()
{
	if(!m_vTextBackColors.empty()){
		::SetBkColor(m_hdc,m_vTextBackColors[0]);
		m_vTextBackColors.clear();
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         フォント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushMyFont(const SFONT& sFont)
{
	//設定
	HFONT hFontOld = (HFONT)SelectObject(m_hdc, sFont.m_hFont);
	(void)hFontOld;
	m_vFonts.push_back(sFont);
}

void CGraphics::PopMyFont()
{
	//戻す
	if(m_vFonts.size()){
		m_vFonts.pop_back();
		if(m_vFonts.size()){
			SelectObject(m_hdc, m_vFonts.back().m_hFont);
		}
	}
}

void CGraphics::ClearMyFont()
{
	m_vFonts.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ペン                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushPen(COLORREF color, int nPenWidth, int nStyle)
{
	HPEN hpnNew = CreatePen(nStyle,nPenWidth,color);
	HPEN hpnOld = (HPEN)SelectObject(m_hdc,hpnNew);
	m_vPens.push_back(hpnNew);
}

void CGraphics::PopPen()
{
	if(m_vPens.size()){
		DeleteObject(m_vPens.back());
		m_vPens.pop_back();
		if(m_vPens.size()){
			SelectObject(m_hdc,m_vPens.back());
		}
	}
}

void CGraphics::ClearPen()
{
	size_t nSize = m_vPens.size();
	for(size_t i=0;i<nSize;i++){
		DeleteObject(m_vPens[i]);
	}
	m_vPens.clear();
}

//$$note: 高速化
COLORREF CGraphics::GetPenColor() const
{
	if(m_vPens.size()){
		LOGPEN logpen;
		if(GetObject(m_vPens.back(), sizeof(logpen), &logpen)){
			return logpen.lopnColor;
		}
	}
	return 0;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ブラシ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushBrushColor(COLORREF color)
{
	//新しく作成→HDCに設定→スタックに保存
	HBRUSH hbrNew = (color!=(COLORREF)-1)?CreateSolidBrush(color):(HBRUSH)GetStockObject(NULL_BRUSH);
	::SelectObject(m_hdc,hbrNew);
	m_vBrushes.push_back(hbrNew);
}

void CGraphics::PopBrushColor()
{
	if(m_vBrushes.size()){
		//最後の要素を削除
		::DeleteObject(m_vBrushes.back());
		m_vBrushes.pop_back();
		if(m_vBrushes.size()){
			::SelectObject(m_hdc,m_vBrushes.back());
		}
	}
}

void CGraphics::ClearBrush()
{
	//ブラシをすべて削除
	size_t nSize = (int)m_vBrushes.size();
	for(size_t i=0;i<nSize;i++){
		::DeleteObject(m_vBrushes[i]);
	}
	m_vBrushes.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           直線                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//$$note:高速化
void CGraphics::DrawDotLine(int x1, int y1, int x2, int y2)
{
	COLORREF c = GetPenColor();
	int my = t_unit(y2 - y1) * 2;
	int mx = t_unit(x2 - x1) * 2;
	if(!mx && !my)return;
	int x = x1;
	int y = y1;
	if(!mx && !my)return;
	for (;;) {
		//点描画
		ApiWrap::SetPixelSurely(m_hdc,x,y,c);

		//進める
		x+=mx;
		y+=my;

		//条件判定
		if(mx>0 && x>=x2)break;
		if(mx<0 && x<=x2)break;
		if(my>0 && y>=y2)break;
		if(my<0 && y<=y2)break;
	}
}

// ドロップ先矩形描画用のリージョンを作成する
static HRGN CreateDropRectRgn(LPCRECT lpRect, SIZE size)
{
	HRGN hRgnOutside = ::CreateRectRgnIndirect(lpRect);
	RECT rc = *lpRect;
	::InflateRect(&rc, -size.cx, -size.cy);
	::IntersectRect(&rc, &rc, lpRect);
	HRGN hRgnInside = ::CreateRectRgnIndirect(&rc);
	HRGN hRgn = ::CreateRectRgn(0, 0, 0, 0);
	::CombineRgn(hRgn, hRgnOutside, hRgnInside, RGN_XOR);
	if(hRgnOutside) ::DeleteObject(hRgnOutside);
	if(hRgnInside) ::DeleteObject(hRgnInside);
	return hRgn;
}

// ドロップ先矩形描画用のブラシを取得する
static HBRUSH GetDropRectBrush()
{
	static HBRUSH s_hBrush = NULL;
	if(!s_hBrush){
		WORD wBits[8] = {0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA};
		HBITMAP hBitmap = ::CreateBitmap(8, 8, 1, 1, wBits);
		if(hBitmap){
			s_hBrush = ::CreatePatternBrush(hBitmap);
			::DeleteObject(hBitmap);
			s_cGDIStock.Register(s_hBrush);	// 終了時破棄用にストックしておく
		}
	}
	return s_hBrush;
}

// ドロップ先の矩形を描画する
void CGraphics::DrawDropRect(LPCRECT lpRectNew, SIZE sizeNew, LPCRECT lpRectLast, SIZE sizeLast)
{
	if(!lpRectNew && !lpRectLast)
		return;

	HWND hwndDt = ::GetDesktopWindow();
	HDC hdc = ::GetDCEx(hwndDt, NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);

	HRGN hRgnNew = NULL;
	HRGN hRgnUpdate = NULL;
	if(lpRectNew){
		hRgnNew = CreateDropRectRgn(lpRectNew, sizeNew);
	}
	if(lpRectLast){
		HRGN hRgnLast = CreateDropRectRgn(lpRectLast, sizeLast);
		if(lpRectNew){
			hRgnUpdate = ::CreateRectRgn(0, 0, 0, 0);
			::CombineRgn(hRgnUpdate, hRgnLast, hRgnNew, RGN_XOR);
			::DeleteObject(hRgnLast);
		}else{
			hRgnUpdate = hRgnLast;
		}
	}

	RECT rc;
	::SelectClipRgn(hdc, hRgnUpdate? hRgnUpdate: hRgnNew);
	::GetClipBox(hdc, &rc);

	HBRUSH hBrush = GetDropRectBrush();
	HBRUSH hBrushOld = (HBRUSH)::SelectObject(hdc, hBrush);

	::PatBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATINVERT);

	::SelectObject(hdc, hBrushOld);
	::SelectClipRgn(hdc, NULL);

	if(hRgnNew) ::DeleteObject(hRgnNew);
	if(hRgnUpdate) ::DeleteObject(hRgnUpdate);

	::ReleaseDC(hwndDt, hdc);
}

void CGraphics::DrawRect(int x1, int y1, int x2, int y2)
{
	--x2;
	--y2;
	::MoveToEx(m_hdc, x1, y1, NULL);
	::LineTo(m_hdc, x2, y1);
	::LineTo(m_hdc, x2, y2);
	::LineTo(m_hdc, x1, y2);
	::LineTo(m_hdc, x1, y1);
}
