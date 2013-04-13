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
#ifndef SAKURA_CGRAPHICS_BA5156BF_99C6_4854_8131_CE8B091A5EFF9_H_
#define SAKURA_CGRAPHICS_BA5156BF_99C6_4854_8131_CE8B091A5EFF9_H_

/*
2008.05.20 kobake �쐬
*/

#include <Windows.h>
#include <vector>

//! �I���W�i���l�ۑ��N���X
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


//! �`��Ǘ�
//�ŐV�����F�u���V
class CGraphics{
public:
	CGraphics(const CGraphics& rhs){ Init(rhs.m_hdc); }
	CGraphics(HDC hdc = NULL){ Init(hdc); }
	~CGraphics();
	void Init(HDC hdc);

	operator HDC() const{ return m_hdc; }

	//�N���b�s���O
private:
	void _InitClipping();
public:
	void PushClipping(const RECT& rc);
	void PopClipping();
	void ClearClipping();
	void SetClipping(const RECT& rc)
	{
		ClearClipping();
		PushClipping(rc);
	}

	//�F�ݒ�
public:
	//! �`��F��ݒ�
	void SetForegroundColor(COLORREF color)
	{
		//�e�L�X�g�O�i�F
		SetTextForeColor(color);
	}
	//! �w�i�F��ݒ�
	void SetBackgroundColor(COLORREF color)
	{
		//�e�L�X�g�w�i�F
		SetTextBackColor(color);
	}

	//�e�L�X�g�����F
public:
	void PushTextForeColor(COLORREF color);
	void PopTextForeColor();
	void ClearTextForeColor();
	void SetTextForeColor(COLORREF color)
	{
		ClearTextForeColor();
		PushTextForeColor(color);
	}

	//�e�L�X�g�w�i�F
public:
	void PushTextBackColor(COLORREF color);
	void PopTextBackColor();
	void ClearTextBackColor();
	void SetTextBackColor(COLORREF color)
	{
		ClearTextBackColor();
		PushTextBackColor(color);
	}

	//�e�L�X�g���[�h
public:
	void SetTextBackTransparent(bool b)
	{
		m_nTextModeOrg.AssignOnce( ::SetBkMode(m_hdc,b?TRANSPARENT:OPAQUE) );
	}

	//�e�L�X�g
public:
	void RestoreTextColors();

	//�t�H���g
public:
	void PushMyFont(HFONT hFont);
	void PopMyFont();
	void ClearMyFont();
	//! �t�H���g�ݒ�
	void SetMyFont(HFONT hFont)
	{
		ClearMyFont();
		PushMyFont(hFont);
	}

	//�y��
public:
	void PushPen(COLORREF color, int nPenWidth, int nStyle = PS_SOLID);
	void PopPen();
	void SetPen(COLORREF color)
	{
		ClearPen();
		PushPen(color,1);
	}
	void ClearPen();
	COLORREF GetPenColor() const;

	//�u���V
public:
	void _InitBrushColor();
	void PushBrushColor(
		COLORREF color	//!< �u���V�̐F�B(COLORREF)-1 �ɂ���ƁA�����u���V�ƂȂ�B
	);
	void PopBrushColor();
	void ClearBrush();

	void SetBrushColor(COLORREF color)
	{
		ClearBrush();
		PushBrushColor(color);
	}
	HBRUSH GetCurrentBrush() const{ return m_vBrushes.size()?m_vBrushes.back():NULL; }

	//�`��
public:
	//! ����
	void DrawLine(int x1, int y1, int x2, int y2)
	{
		::MoveToEx(m_hdc,x1,y1,NULL);
		::LineTo(m_hdc,x2,y2);
	}
	void DrawDotLine(int x1, int y1, int x2, int y2);	//�_��
	//! ��`�h��ׂ�
	void FillMyRect(const RECT& rc)
	{
		::FillRect(m_hdc,&rc,GetCurrentBrush());
#ifdef _DEBUG
		::SetPixel(m_hdc,-1,-1,0); //###########����
#endif
	}
	//! ��`�h��ׂ�
	void FillSolidMyRect(const RECT& rc, COLORREF color)
	{
		PushTextBackColor(color);
		FillMyRectTextBackColor(rc);
		PopTextBackColor();
	}
	//! ��`�h��ׂ�
	void FillMyRectTextBackColor(const RECT& rc)
	{
		::ExtTextOut(m_hdc, rc.left, rc.top, ETO_OPAQUE|ETO_CLIPPED, &rc, _T(""), 0, NULL);
	}

	static void DrawDropRect(LPCRECT lpRectNew, SIZE sizeNew, LPCRECT lpRectLast, SIZE sizeLast);	// �h���b�v��̋�`��`�悷��

private:
	//�^
	typedef TOriginalHolder<COLORREF>	COrgColor;
	typedef TOriginalHolder<int>		COrgInt;
private:
	HDC					m_hdc;

	//�N���b�s���O
	std::vector<HRGN>		m_vClippingRgns;

	//�e�L�X�g
	std::vector<COLORREF>	m_vTextForeColors;
	std::vector<COLORREF>	m_vTextBackColors;
	std::vector<HFONT>		m_vFonts;

	//�e�L�X�g
	COrgInt				m_nTextModeOrg;

	//�y��
	HPEN				m_hpnOrg;
	std::vector<HPEN>	m_vPens;

	//�u���V
	std::vector<HBRUSH>	m_vBrushes;
	HBRUSH				m_hbrOrg;
	HBRUSH				m_hbrCurrent;
	bool				m_bDynamicBrush;	//m_hbrCurrent�𓮓I�ɍ쐬�����ꍇ��true
};

#endif /* SAKURA_CGRAPHICS_BA5156BF_99C6_4854_8131_CE8B091A5EFF9_H_ */
/*[EOF]*/
