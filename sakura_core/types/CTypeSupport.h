#pragma once

#include "view/CEditView.h"
#include "CShareData.h"
#include "doc/CEditDoc.h"

//2007.08.28 kobake �ǉ�
//!�^�C�v�T�|�[�g�N���X
class CTypeSupport{
private:
	static const COLORREF INVALID_COLOR=0xFFFFFFFF; //�����ȐF�萔

public:
	CTypeSupport(const CEditView* pEditView,EColorIndexType nColorIdx)
	: m_pFontset(&pEditView->GetFontset())
	, m_nColorIdx(nColorIdx)
	{
		m_pTypes = &pEditView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

		m_hdc=NULL;
		m_hfntOld=NULL;
		m_hpnNew=NULL;
		m_hpnOld=NULL;
		m_clrOldBack=INVALID_COLOR;
		m_clrOldText=INVALID_COLOR;
	}
	virtual ~CTypeSupport()
	{
		if(m_hdc){
			//�t�H���g�����ɖ߂�
			RewindFont(m_hdc);
			//�y�������ɖ߂�
			RewindPen(m_hdc);
			//�F�����ɖ߂�
			RewindColors(m_hdc);
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �擾                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//!�O�i�F(�����F)
	COLORREF GetTextColor() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colTEXT;
	}

	//!�w�i�F
	COLORREF GetBackColor() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colBACK;
	}

	//!�\�����邩�ǂ���
	bool IsDisp() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bDisp;
	}

	//!�������ǂ���
	bool IsFatFont() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bFatFont;
	}

	//!�����������ǂ���
	bool HasUnderLine() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bUnderLine;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void FillBack(HDC hdc,const RECT& rc)
	{
		HBRUSH hBrush = ::CreateSolidBrush( m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void SetFont(HDC hdc)
	{
		m_hdc=hdc;

		HFONT hfntOld = (HFONT)::SelectObject( hdc,
			m_pFontset->ChooseFontHandle(
				m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bFatFont,
				m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bUnderLine
			)
		);

		if(m_hfntOld==NULL)
			m_hfntOld=hfntOld;
	}
	void RewindFont(HDC hdc)
	{
		//���ɖ߂�
		if(m_hfntOld){
			SelectObject(m_hdc,m_hfntOld);
			m_hfntOld=NULL;
		}
	}

	void SetSolidPen(HDC hdc,int nPenWidth)
	{
		m_hdc=hdc;

		//�쐬����
		HPEN hpnNew = ::CreatePen( PS_SOLID, 0, this->GetTextColor() );

		//�I������
		HPEN hpnOld = (HPEN)::SelectObject( hdc, hpnNew );
		if(m_hpnOld==NULL)
			m_hpnOld=hpnOld;

		//�����ւ���
		if(m_hpnNew)DeleteObject(m_hpnNew);
		m_hpnNew=hpnNew;
	}
	void RewindPen(HDC hdc)
	{
		//���ɖ߂�
		if(m_hpnOld){
			SelectObject(m_hdc,m_hpnOld);
			m_hpnOld=NULL;
		}

		//�j������
		if(m_hpnNew){
			DeleteObject(m_hpnNew);
			m_hpnNew=NULL;
		}
	}

	void SetBkColor(HDC hdc)
	{
		m_hdc=hdc;

		COLORREF clrOldBack = ::SetBkColor  ( hdc, m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colBACK );
		if(m_clrOldBack==INVALID_COLOR)m_clrOldBack=clrOldBack;
	}
	void SetTextColor(HDC hdc)
	{
		m_hdc=hdc;

		COLORREF clrOldText = ::SetTextColor( hdc, m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colTEXT );
		if(m_clrOldText==INVALID_COLOR)m_clrOldText=clrOldText;
	}
	void SetColors(HDC hdc)
	{
		this->SetBkColor(hdc);
		this->SetTextColor(hdc);
	}
	void RewindColors(HDC hdc)
	{
		if(m_clrOldBack!=INVALID_COLOR){
			::SetBkColor(hdc,m_clrOldBack);
			m_clrOldBack=INVALID_COLOR;
		}
		if(m_clrOldText!=INVALID_COLOR){
			::SetTextColor(hdc,m_clrOldText);
			m_clrOldText=INVALID_COLOR;
		}
	}


private:
	const CViewFont*		m_pFontset;
	const STypeConfig*		m_pTypes;
	EColorIndexType			m_nColorIdx;

	HDC       m_hdc;        //�ݒ��ύX����HDC
	HFONT     m_hfntOld;    //SetFont     �ȑO�ɑI������Ă����t�H���g
	HPEN      m_hpnNew;     //SetSolidPen �ō쐬���ꂽ�t�H���g
	HPEN      m_hpnOld;     //SetSolidPen �ȑO�ɑI������Ă����y��
	COLORREF  m_clrOldBack; //SetBkColor  �ȑO�ɐݒ肳��Ă����F
	COLORREF  m_clrOldText; //SetTextColor�ȑO�ɐݒ肳��Ă����F
};



