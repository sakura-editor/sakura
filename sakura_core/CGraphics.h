/*
2008.05.20 kobake �쐬
*/

#pragma once

#include <windows.h>
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
	void SetClipping(const RECT& rc);
	void ClearClipping();

	//�F�ݒ�
public:
	void SetForegroundColor(COLORREF color);	//!< �`��F��ݒ�
	void SetBackgroundColor(COLORREF color);	//!< �w�i�F��ݒ�

	//�e�L�X�g�����F
public:
	void PushTextForeColor(COLORREF color);
	void PopTextForeColor();
	void ClearTextForeColor();
	void SetTextForeColor(COLORREF color);

	//�e�L�X�g�w�i�F
public:
	void PushTextBackColor(COLORREF color);
	void PopTextBackColor();
	void ClearTextBackColor();
	void SetTextBackColor(COLORREF color);

	//�e�L�X�g���[�h
public:
	void SetTextBackTransparent(bool b);

	//�e�L�X�g
public:
	void RestoreTextColors();

	//�t�H���g
public:
	void PushMyFont(HFONT hFont);
	void PopMyFont();
	void ClearMyFont();
	void SetMyFont(HFONT hFont);				//!< �t�H���g�ݒ�

	//�y��
public:
	void PushPen(COLORREF color, int nPenWidth, int nStyle = PS_SOLID);
	void PopPen();
	void SetPen(COLORREF color);
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

	void SetBrushColor(COLORREF color);
	HBRUSH GetCurrentBrush() const{ return m_vBrushes.size()?m_vBrushes.back():NULL; }

	//�`��
public:
	void DrawLine(int x1, int y1, int x2, int y2);		//����
	void DrawDotLine(int x1, int y1, int x2, int y2);	//�_��
	void FillMyRect(const RECT& rc);					//��`�h��ׂ�
	void FillSolidMyRect(const RECT& rc, COLORREF color);		//��`�h��ׂ�
	void FillMyRectTextBackColor(const RECT& rc);				//��`�h��ׂ�

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
