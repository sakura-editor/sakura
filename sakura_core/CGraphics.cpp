/*
2008.05.20 kobake �쐬
*/

#include "stdafx.h"
#include "CGraphics.h"
#include "util/std_macro.h"

class CGDIStock
{
public:
	CGDIStock(){};
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

static CGDIStock s_cGDIStock;	// �B��� CGDIStock �I�u�W�F�N�g

void CGraphics::Init(HDC hdc)
{
	m_hdc = hdc;
	//�y��
	m_hpnOrg = NULL;
	//�u���V
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
//                       �N���b�s���O                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


void CGraphics::_InitClipping()
{
	if(m_vClippingRgns.empty()){
		//���̃N���b�s���O�̈���擾
		RECT rcDummy = {0,0,1,1};
		HRGN hrgnOrg = ::CreateRectRgnIndirect(&rcDummy);
		int nRet = ::GetClipRgn(m_hdc,hrgnOrg);
		if(nRet!=1){
			::DeleteObject(hrgnOrg);
			hrgnOrg = NULL;
		}
		//�ۑ�
		m_vClippingRgns.push_back(hrgnOrg);
	}
}

void CGraphics::PushClipping(const RECT& rc)
{
	_InitClipping();
	//�V�����쐬��HDC�ɐݒ聨�X�^�b�N�ɕۑ�
	HRGN hrgnNew = CreateRectRgnIndirect(&rc);
	::SelectClipRgn(m_hdc,hrgnNew);
	m_vClippingRgns.push_back(hrgnNew);
}

void CGraphics::PopClipping()
{
	if(m_vClippingRgns.size()>=2){
		//�Ō�̗v�f���폜
		::DeleteObject(m_vClippingRgns.back());
		m_vClippingRgns.pop_back();
		//���̎��_�̍Ō�̗v�f��HDC�ɐݒ�
		::SelectClipRgn(m_hdc,m_vClippingRgns.back());
	}
}

void CGraphics::ClearClipping()
{
	//���̃N���b�s���O�ɖ߂�
	if(!m_vClippingRgns.empty()){
		::SelectClipRgn(m_hdc,m_vClippingRgns[0]);
	}
	//�̈�����ׂč폜
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
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �`��F��ݒ�
void CGraphics::SetForegroundColor(COLORREF color)
{
	//�e�L�X�g�O�i�F
	SetTextForeColor(color);

	//�y��
//	SetPen(color);
}

//! �w�i�F��ݒ�
void CGraphics::SetBackgroundColor(COLORREF color)
{
	//�e�L�X�g�w�i�F
	SetTextBackColor(color);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �e�L�X�g�����F                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushTextForeColor(COLORREF color)
{
	//�ݒ�
	COLORREF cOld = ::SetTextColor(m_hdc,color);
	//�L�^
	if(m_vTextForeColors.size()==0){
		m_vTextForeColors.push_back(cOld);
	}
	m_vTextForeColors.push_back(color);
}

void CGraphics::PopTextForeColor()
{
	//�߂�
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
//                      �e�L�X�g�w�i�F                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushTextBackColor(COLORREF color)
{
	//�ݒ�
	COLORREF cOld = ::SetBkColor(m_hdc,color);
	//�L�^
	if(m_vTextBackColors.size()==0){
		m_vTextBackColors.push_back(cOld);
	}
	m_vTextBackColors.push_back(color);
}

void CGraphics::PopTextBackColor()
{
	//�߂�
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
//                      �e�L�X�g���[�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::SetTextBackTransparent(bool b)
{
	m_nTextModeOrg.AssignOnce( ::SetBkMode(m_hdc,b?TRANSPARENT:OPAQUE) );
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �e�L�X�g                            //
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
//                         �t�H���g                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::PushMyFont(HFONT hFont)
{
	//�ݒ�
	HFONT hfntOld = (HFONT)SelectObject(m_hdc, hFont);

	//�L�^
	if(m_vFonts.empty()){
		m_vFonts.push_back(hfntOld);
	}
	m_vFonts.push_back(hFont);
}

void CGraphics::PopMyFont()
{
	//�߂�
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

//! �t�H���g�ݒ�
void CGraphics::SetMyFont(HFONT hFont)
{
	ClearMyFont();
	PushMyFont(hFont);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �y��                              //
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
	//�I��������
	HPEN hpnNew = NULL;
	if(m_vPens.size()>=2){
		hpnNew = m_vPens[m_vPens.size()-2];
	}
	else{
		hpnNew = m_hpnOrg;
	}

	//�I��
	HPEN hpnOld = NULL;
	if(hpnNew){
		hpnOld = (HPEN)SelectObject(m_hdc,hpnNew);
	}

	//�폜
	if(!m_vPens.empty()){
		DeleteObject(m_vPens.back());
		m_vPens.pop_back();
	}

	//�I���W�i��
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

//$$note: ������
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
//                          �u���V                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::_InitBrushColor()
{
	if(m_vBrushes.empty()){
		//���̃u���V���擾
		HBRUSH hbrOrg = (HBRUSH)::SelectObject(m_hdc,::GetStockObject(NULL_BRUSH));
		::SelectObject(m_hdc,hbrOrg); //���ɖ߂�
		//�ۑ�
		m_vBrushes.push_back(hbrOrg);
	}
}

void CGraphics::PushBrushColor(COLORREF color)
{
	//####�����Ō������ł���

	_InitBrushColor();
	//�V�����쐬��HDC�ɐݒ聨�X�^�b�N�ɕۑ�
	HBRUSH hbrNew = (color!=(COLORREF)-1)?CreateSolidBrush(color):(HBRUSH)GetStockObject(NULL_BRUSH);
	::SelectObject(m_hdc,hbrNew);
	m_vBrushes.push_back(hbrNew);
}

void CGraphics::PopBrushColor()
{
	if(m_vBrushes.size()>=2){
		//�Ōォ��2�Ԗڂ̗v�f��HDC�ɐݒ�
		::SelectObject(m_hdc,m_vBrushes[m_vBrushes.size()-2]);
		//�Ō�̗v�f���폜
		::DeleteObject(m_vBrushes.back());
		m_vBrushes.pop_back();
	}
}

void CGraphics::ClearBrush()
{
	//���̃u���V�ɖ߂�
	if(!m_vBrushes.empty()){
		::SelectObject(m_hdc,m_vBrushes[0]);
	}
	//�u���V�����ׂč폜 (0�ԗv�f�ȊO)
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
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CGraphics::DrawLine(int x1, int y1, int x2, int y2)
{
	::MoveToEx(m_hdc,x1,y1,NULL);
	::LineTo(m_hdc,x2,y2);
}

//$$note:������
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
		//�_�`��
		ApiWrap::SetPixelSurely(m_hdc,x,y,c);
		
		//�i�߂�
		x+=mx;
		y+=my;

		//��������
		if(mx>0 && x>=x2)break;
		if(mx<0 && x<=x2)break;
		if(my>0 && y>=y2)break;
		if(my<0 && y<=y2)break;
	}
}

//��`�h��ׂ�
void CGraphics::FillMyRect(const RECT& rc)
{
	::FillRect(m_hdc,&rc,GetCurrentBrush());
#ifdef _DEBUG
	::SetPixel(m_hdc,-1,-1,0); //###########����
#endif
}

// �h���b�v���`�`��p�̃��[�W�������쐬����
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

// �h���b�v���`�`��p�̃u���V���擾����
static HBRUSH GetDropRectBrush()
{
	static HBRUSH s_hBrush = NULL;
	if(!s_hBrush){
		WORD wBits[8] = {0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA};
		HBITMAP hBitmap = ::CreateBitmap(8, 8, 1, 1, wBits);
		if(hBitmap){
			s_hBrush = ::CreatePatternBrush(hBitmap);
			::DeleteObject(hBitmap);
			s_cGDIStock.Register(s_hBrush);	// �I�����j���p�ɃX�g�b�N���Ă���
		}
	}
	return s_hBrush;
}

// �h���b�v��̋�`��`�悷��
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
