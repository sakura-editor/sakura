/*
2008.05.20 kobake 作成
*/

#include "stdafx.h"
#include "CGraphics.h"
#include "util/std_macro.h"

void CGraphics::Init(HDC hdc)
{
	m_hdc = hdc;
	//ペン
	m_hpnOrg = NULL;
	//ブラシ
	m_hbrOrg = NULL;
	m_hbrCurrent = NULL;
	m_bDynamicBrush = NULL;
}

CGraphics::~CGraphics()
{
	ClearClipping();
	ClearMyFont();
	ClearPen();
	ClearBrush();
	RestoreTextColors();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       クリッピング                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


void CGraphics::_InitClipping()
{
	if(m_vClippingRgns.empty()){
		//元のクリッピング領域を取得
		RECT rcDummy = {0,0,1,1};
		HRGN hrgnOrg = ::CreateRectRgnIndirect(&rcDummy);
		int nRet = ::GetClipRgn(m_hdc,hrgnOrg);
		if(nRet!=1){
			::DeleteObject(hrgnOrg);
			hrgnOrg = NULL;
		}
		//保存
		m_vClippingRgns.push_back(hrgnOrg);
	}
}

void CGraphics::PushClipping(const RECT& rc)
{
	_InitClipping();
	//新しく作成→HDCに設定→スタックに保存
	HRGN hrgnNew = CreateRectRgnIndirect(&rc);
	::SelectClipRgn(m_hdc,hrgnNew);
	m_vClippingRgns.push_back(hrgnNew);
}

void CGraphics::PopClipping()
{
	if(m_vClippingRgns.size()>=2){
		//最後の要素を削除
		::DeleteObject(m_vClippingRgns.back());
		m_vClippingRgns.pop_back();
		//この時点の最後の要素をHDCに設定
		::SelectClipRgn(m_hdc,m_vClippingRgns.back());
	}
}

void CGraphics::ClearClipping()
{
	//元のクリッピングに戻す
	if(!m_vClippingRgns.empty()){
		::SelectClipRgn(m_hdc,m_vClippingRgns[0]);
	}
	//領域をすべて削除
	for(int i=0;i<(int)m_vClippingRgns.size();i++){
		::DeleteObject(m_vClippingRgns[i]);
	}
	m_vClippingRgns.clear();
}

void CGraphics::SetClipping(const RECT& rc)
{
	ClearClipping();
	PushClipping(rc);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           総合                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 描画色を設定
void CGraphics::SetForegroundColor(COLORREF color)
{
	//テキスト前景色
	SetTextForeColor(color);

	//ペン
//	SetPen(color);
}

//! 背景色を設定
void CGraphics::SetBackgroundColor(COLORREF color)
{
	//テキスト背景色
	SetTextBackColor(color);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキスト文字色                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushTextForeColor(COLORREF color)
{
	//設定
	COLORREF cOld = ::SetTextColor(m_hdc,color);
	//記録
	if(m_vTextForeColors.size()==0){
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

void CGraphics::SetTextForeColor(COLORREF color)
{
	ClearTextForeColor();
	PushTextForeColor(color);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキスト背景色                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushTextBackColor(COLORREF color)
{
	//設定
	COLORREF cOld = ::SetBkColor(m_hdc,color);
	//記録
	if(m_vTextBackColors.size()==0){
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

void CGraphics::SetTextBackColor(COLORREF color)
{
	ClearTextBackColor();
	PushTextBackColor(color);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      テキストモード                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::SetTextBackTransparent(bool b)
{
	m_nTextModeOrg.AssignOnce( ::SetBkMode(m_hdc,b?TRANSPARENT:OPAQUE) );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         テキスト                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::RestoreTextColors()
{
	PopTextForeColor();
	PopTextBackColor();
	if(m_nTextModeOrg.HasData()){
		::SetBkMode(m_hdc,m_nTextModeOrg.Get());
		m_nTextModeOrg.Clear();
	}
}
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         フォント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushMyFont(HFONT hFont)
{
	//設定
	HFONT hfntOld = (HFONT)SelectObject(m_hdc, hFont);

	//記録
	if(m_vFonts.empty()){
		m_vFonts.push_back(hfntOld);
	}
	m_vFonts.push_back(hFont);
}

void CGraphics::PopMyFont()
{
	//戻す
	if(m_vFonts.size()>=2){
		m_vFonts.pop_back();
	}
	if(!m_vFonts.empty()){
		SelectObject(m_hdc,m_vFonts.back());
	}
}

void CGraphics::ClearMyFont()
{
	if(!m_vFonts.empty()){
		SelectObject(m_hdc,m_vFonts[0]);
	}
	m_vFonts.clear();
}

//! フォント設定
void CGraphics::SetMyFont(HFONT hFont)
{
	ClearMyFont();
	PushMyFont(hFont);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ペン                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushPen(COLORREF color, int nPenWidth, int nStyle)
{
	HPEN hpnNew = CreatePen(nStyle,nPenWidth,color);
	HPEN hpnOld = (HPEN)SelectObject(m_hdc,hpnNew);
	m_vPens.push_back(hpnNew);
	if(!m_hpnOrg){
		m_hpnOrg = hpnOld;
	}
}

void CGraphics::PopPen()
{
	//選択する候補
	HPEN hpnNew = NULL;
	if(m_vPens.size()>=2){
		hpnNew = m_vPens[m_vPens.size()-2];
	}
	else{
		hpnNew = m_hpnOrg;
	}

	//選択
	HPEN hpnOld = NULL;
	if(hpnNew){
		hpnOld = (HPEN)SelectObject(m_hdc,hpnNew);
	}

	//削除
	if(!m_vPens.empty()){
		DeleteObject(m_vPens.back());
		m_vPens.pop_back();
	}

	//オリジナル
	if(m_vPens.empty()){
		m_hpnOrg = NULL;
	}
}

void CGraphics::SetPen(COLORREF color)
{
	ClearPen();
	PushPen(color,1);
}

void CGraphics::ClearPen()
{
	if(m_hpnOrg){
		SelectObject(m_hdc,m_hpnOrg);
		m_hpnOrg = NULL;
	}
	for(int i=0;i<(int)m_vPens.size();i++){
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
	else{
		return 0;
	}
	return 0;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ブラシ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::_InitBrushColor()
{
	if(m_vBrushes.empty()){
		//元のブラシを取得
		HBRUSH hbrOrg = (HBRUSH)::SelectObject(m_hdc,::GetStockObject(NULL_BRUSH));
		::SelectObject(m_hdc,hbrOrg); //元に戻す
		//保存
		m_vBrushes.push_back(hbrOrg);
	}
}

void CGraphics::PushBrushColor(COLORREF color)
{
	//####ここで効率化できる

	_InitBrushColor();
	//新しく作成→HDCに設定→スタックに保存
	HBRUSH hbrNew = (color!=(COLORREF)-1)?CreateSolidBrush(color):(HBRUSH)GetStockObject(NULL_BRUSH);
	::SelectObject(m_hdc,hbrNew);
	m_vBrushes.push_back(hbrNew);
}

void CGraphics::PopBrushColor()
{
	if(m_vBrushes.size()>=2){
		//最後から2番目の要素をHDCに設定
		::SelectObject(m_hdc,m_vBrushes[m_vBrushes.size()-2]);
		//最後の要素を削除
		::DeleteObject(m_vBrushes.back());
		m_vBrushes.pop_back();
	}
}

void CGraphics::ClearBrush()
{
	//元のブラシに戻す
	if(!m_vBrushes.empty()){
		::SelectObject(m_hdc,m_vBrushes[0]);
	}
	//ブラシをすべて削除 (0番要素以外)
	for(int i=1;i<(int)m_vBrushes.size();i++){
		::DeleteObject(m_vBrushes[i]);
	}
	m_vBrushes.resize(t_min(1,(int)m_vBrushes.size()));
}


void CGraphics::SetBrushColor(COLORREF color)
{
	ClearBrush();
	PushBrushColor(color);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           直線                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::DrawLine(int x1, int y1, int x2, int y2)
{
	::MoveToEx(m_hdc,x1,y1,NULL);
	::LineTo(m_hdc,x2,y2);
}

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
	while(1){
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

//矩形塗り潰し
void CGraphics::FillMyRect(const RECT& rc)
{
	::FillRect(m_hdc,&rc,GetCurrentBrush());
#ifdef _DEBUG
	::SetPixel(m_hdc,-1,-1,0); //###########実験
#endif
}


