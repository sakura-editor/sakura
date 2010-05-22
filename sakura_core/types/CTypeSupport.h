#pragma once

// #include "CGraphics.h"	// stdafx.h�Q��
// #include "types/CTypes.h"	// STypeConfig ���Ԃ�CLayoutMgr.h��include
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "view/colors/CColorStrategy.h"
// #include "view/CViewFont.h"	// in view/CEditView.h



//2007.08.28 kobake �ǉ�
/*!�^�C�v�T�|�[�g�N���X
	���̂Ƃ���^�C�v�ʐݒ�̐F���擾�̕⏕
*/
class CTypeSupport{
private:
	static const COLORREF INVALID_COLOR=0xFFFFFFFF; //�����ȐF�萔

public:
	CTypeSupport(const CEditView* pEditView,EColorIndexType eColorIdx)
	: m_pFontset(&pEditView->GetFontset())
	, m_nColorIdx(ToColorInfoArrIndex(eColorIdx))
	{
		assert(0 <= m_nColorIdx);
		m_pTypes = &pEditView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

		m_gr=NULL;
	}
	virtual ~CTypeSupport()
	{
		if(m_gr){
			RewindGraphicsState(*m_gr);
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
	void FillBack(CGraphics& gr,const RECT& rc)
	{
		gr.PushBrushColor( m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colBACK );
		gr.FillMyRect(rc);
		gr.PopBrushColor();
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	HFONT GetTypeFont()
	{
		return m_pFontset->ChooseFontHandle(
			m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bFatFont,
			m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bUnderLine
		);
	}
	void SetGraphicsState_WhileThisObj(CGraphics& gr)
	{
		if(m_gr){
			RewindGraphicsState(*m_gr);
		}

		m_gr = &gr;

		//�e�L�X�g�F
		gr.PushTextBackColor(GetBackColor());
		gr.PushTextForeColor(GetTextColor());

		//�t�H���g
		gr.PushMyFont(GetTypeFont());
	}
	void RewindGraphicsState(CGraphics& gr)
	{
		if(m_gr){
			gr.PopTextBackColor();
			gr.PopTextForeColor();
			gr.PopMyFont();
			m_gr = NULL;
		}
	}

private:
	const CViewFont*		m_pFontset;
	const STypeConfig*		m_pTypes;
	int						m_nColorIdx;

	CGraphics* m_gr;        //�ݒ��ύX����HDC
};



