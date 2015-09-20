#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_ZenSpace.h"
#include "types/CTypeSupport.h"

void Draw_ZenSpace( CGraphics& gr, const CMyRect& rc );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigure_ZenSpace                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenSpace::Match(const wchar_t* pText, int nTextLen) const
{
	if( pText[0] == L'�@' ){
		return true;
	}
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! �S�p�X�y�[�X�`��
void CFigure_ZenSpace::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	// 2010.09.21 PP�p�����ǉ�
	// �v���|�[�V���i���ł́A�S�pSP�Ɓ��̕����Ⴄ���Ƃ�����B�Ⴄ�ꍇ�͓Ǝ��ɕ`��
	CTypeSupport cZenSpace(pcView, COLORIDX_ZENSPACE);

	int dx[1];
	dx[0] = pcView->GetTextMetrics().CalcTextWidth3(L"�@", 1);

	RECT rc;
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	if(pcView->GetTextArea().GenerateClipRect(&rc, *pDispPos, CHabaXInt(dx[0])))
	{
		int u25a1Dx = pcView->GetTextMetrics().CalcTextWidth3(L"��", 1);
		bool bDrawMySelf = dx[0] != u25a1Dx;
		const wchar_t* pZenSp = (bDrawMySelf ? L"�@" : L"��");
		int fontNo = WCODE::GetFontNo(*pZenSp);
		if( fontNo ){
			SFONT sFont;
			sFont.m_sFontAttr = gr.GetCurrentMyFontAttr();
			sFont.m_hFont = pcView->GetFontset().ChooseFontHandle(fontNo, sFont.m_sFontAttr);
			gr.PushMyFont(sFont);
		}
		int nHeightMargin = pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
		//�`��
		ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y + nHeightMargin,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			pZenSp,
			1,
			dx
		);
		if( fontNo ){
			gr.PopMyFont();
		}
		if( bDrawMySelf ){
			gr.PushClipping(rc); // FIXME: ���m�ɂ�CombineRgn RGN_AND ���K�v
			
			// �S�pSP�̑傫���w��
			CMyRect rcZenSp;
			// ���F�x�[�X���C������
			rcZenSp.SetPos(pDispPos->GetDrawPos().x, pDispPos->GetDrawPos().y);
			rcZenSp.SetSize(dx[0]- pcView->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nColumnSpace,
				pcView->GetTextMetrics().GetHankakuHeight());

			// �`��
			// �����F�⑾�����ǂ��������݂� DC ���璲�ׂ�	// 2009.05.29 ryoji 
			// �i�����}�b�`���̏󋵂ɏ_��ɑΉ����邽�߁A�����͋L���̐F�w��ɂ͌��ߑł����Ȃ��j
			Draw_ZenSpace(gr, rcZenSp);

			// ���[�W�����j��
			gr.PopClipping();
			
			// To Here 2003.08.17 ryoji ���s�����������Ȃ��悤��
		}
		
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(CLayoutXInt(dx[0]));
}

void Draw_ZenSpace( CGraphics& gr, const CMyRect& rc )
{
	TEXTMETRIC tm;
	tm.tmAscent = 0;
	::GetTextMetrics(gr, &tm);
	// �����`�ɂ���
	CMyRect rc2;
	int minWidth = std::max<int>(1, std::min<int>(tm.tmAscent, std::min<int>(rc.Height(), rc.Width())) - 2);
	minWidth -= (minWidth + 5) / 10;
	rc2.SetPos(
		rc.left + (rc.Width() - minWidth) / 2,
		rc.top  + tm.tmAscent - minWidth
	);
	rc2.SetSize(minWidth, minWidth);
	gr.PushPen(::GetTextColor(gr), 1);
	gr.DrawRect(rc2);
	gr.PopPen();
}
