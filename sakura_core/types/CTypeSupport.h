#pragma once

#include "view/CEditView.h"
#include "env/CShareData.h"
#include "doc/CEditDoc.h"
#include "view/colors/CColorStrategy.h"

//2007.08.28 kobake 追加
//!タイプサポートクラス
class CTypeSupport{
private:
	static const COLORREF INVALID_COLOR=0xFFFFFFFF; //無効な色定数

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
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bDisp;
	}

	//!太字かどうか
	bool IsFatFont() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bFatFont;
	}

	//!下線を持つかどうか
	bool HasUnderLine() const
	{
		return m_pTypes->m_ColorInfoArr[m_nColorIdx].m_bUnderLine;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void FillBack(CGraphics& gr,const RECT& rc)
	{
		gr.PushBrushColor( m_pTypes->m_ColorInfoArr[m_nColorIdx].m_colBACK );
		gr.FillMyRect(rc);
		gr.PopBrushColor();
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
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

		//テキスト色
		gr.PushTextBackColor(GetBackColor());
		gr.PushTextForeColor(GetTextColor());

		//フォント
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

	CGraphics* m_gr;        //設定を変更したHDC
};



