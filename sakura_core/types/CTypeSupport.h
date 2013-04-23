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
#ifndef SAKURA_CTYPESUPPORT_21FC7075_96B4_4572_BA60_A6550E11AC0B9_H_
#define SAKURA_CTYPESUPPORT_21FC7075_96B4_4572_BA60_A6550E11AC0B9_H_

#include "uiparts/CGraphics.h"
#include "doc/CEditDoc.h"
#include "view/CEditView.h"
#include "view/CViewFont.h"
#include "view/colors/CColorStrategy.h"

//2007.08.28 kobake �ǉ�
/*!�^�C�v�T�|�[�g�N���X
	���̂Ƃ���^�C�v�ʐݒ�̐F���擾�̕⏕
*/
class CTypeSupport{
private:
	static const COLORREF INVALID_COLOR=0xFFFFFFFF; //�����ȐF�萔

public:
	CTypeSupport(const CEditView* pEditView, EColorIndexType eColorIdx)
	: m_pFontset(&pEditView->GetFontset())
	, m_nColorIdx(ToColorInfoArrIndex(eColorIdx))
	{
		assert(0 <= m_nColorIdx);
		m_pTypes = &pEditView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();
		m_pColorInfoArr = &m_pTypes->m_ColorInfoArr[m_nColorIdx];

		m_gr = NULL;
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
		return m_pColorInfoArr->m_colTEXT;
	}

	//!�w�i�F
	COLORREF GetBackColor() const
	{
		return m_pColorInfoArr->m_colBACK;
	}

	//!�\�����邩�ǂ���
	bool IsDisp() const
	{
		return m_pColorInfoArr->m_bDisp;
	}

	//!�������ǂ���
	bool IsFatFont() const
	{
		return m_pColorInfoArr->m_bFatFont;
	}

	//!�����������ǂ���
	bool HasUnderLine() const
	{
		return m_pColorInfoArr->m_bUnderLine;
	}


	const ColorInfo& GetColorInfo() const
	{
		return *m_pColorInfoArr;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �`��                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void FillBack(CGraphics& gr,const RECT& rc)
	{
		gr.FillSolidMyRect(rc, m_pColorInfoArr->m_colBACK);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �ݒ�                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	HFONT GetTypeFont()
	{
		return m_pFontset->ChooseFontHandle(
			m_pColorInfoArr->m_bFatFont,
			m_pColorInfoArr->m_bUnderLine
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
	const ColorInfo*		m_pColorInfoArr;

	CGraphics* m_gr;        //�ݒ��ύX����HDC
};

#endif /* SAKURA_CTYPESUPPORT_21FC7075_96B4_4572_BA60_A6550E11AC0B9_H_ */
/*[EOF]*/
