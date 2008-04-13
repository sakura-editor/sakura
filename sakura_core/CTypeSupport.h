#pragma once

#include "view/CEditView.h"
#include "CShareData.h"
#include "CEditDoc.h"

//2007.08.28 kobake 追加
//!タイプサポートクラス
class CTypeSupport{
private:
	static const COLORREF INVALID_COLOR=0xFFFFFFFF; //無効な色定数

public:
	CTypeSupport(const CEditView* pEditView,int nColorIdx)
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
			//フォントを元に戻す
			RewindFont(m_hdc);
			//ペンを元に戻す
			RewindPen(m_hdc);
			//色を元に戻す
			RewindColors(m_hdc);
		}
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           取得                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//!前景色(文字色)
	COLORREF GetTextColor() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colTEXT;
	}

	//!背景色
	COLORREF GetBackColor() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colBACK;
	}

	//!表示するかどうか
	bool IsDisp() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bDisp!=FALSE;
	}

	//!太字かどうか
	bool IsFatFont() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bFatFont!=FALSE;
	}

	//!下線を持つかどうか
	bool HasUnderLine() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bUnderLine!=FALSE;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void FillBack(HDC hdc,const RECT& rc)
	{
		HBRUSH hBrush = ::CreateSolidBrush( m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colBACK );
		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
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
		//元に戻す
		if(m_hfntOld){
			SelectObject(m_hdc,m_hfntOld);
			m_hfntOld=NULL;
		}
	}

	void SetSolidPen(HDC hdc,int nPenWidth)
	{
		m_hdc=hdc;

		//作成する
		HPEN hpnNew = ::CreatePen( PS_SOLID, 0, this->GetTextColor() );

		//選択する
		HPEN hpnOld = (HPEN)::SelectObject( hdc, hpnNew );
		if(m_hpnOld==NULL)
			m_hpnOld=hpnOld;

		//差し替える
		if(m_hpnNew)DeleteObject(m_hpnNew);
		m_hpnNew=hpnNew;
	}
	void RewindPen(HDC hdc)
	{
		//元に戻す
		if(m_hpnOld){
			SelectObject(m_hdc,m_hpnOld);
			m_hpnOld=NULL;
		}

		//破棄する
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
	const CViewFont* m_pFontset;
	const Types*     m_pTypes;
	int              m_nColorIdx;

	HDC       m_hdc;        //設定を変更したHDC
	HFONT     m_hfntOld;    //SetFont     以前に選択されていたフォント
	HPEN      m_hpnNew;     //SetSolidPen で作成されたフォント
	HPEN      m_hpnOld;     //SetSolidPen 以前に選択されていたペン
	COLORREF  m_clrOldBack; //SetBkColor  以前に設定されていた色
	COLORREF  m_clrOldText; //SetTextColor以前に設定されていた色
};
