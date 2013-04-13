#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_Eol.h"
#include "types/CTypeSupport.h"
#include "doc/CLayout.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "window/CEditWnd.h"

//�܂�Ԃ��`��
void _DispWrap(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

//EOF�`��֐�
//���ۂɂ� pX �� nX ���X�V�����B
//2004.05.29 genta
//2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
//2007.08.25 kobake �������� nCharWidth, nLineHeight ���폜
//2007.08.28 kobake ���� fuOptions ���폜
//void _DispEOF( CGraphics& gr, DispPos* pDispPos, const CEditView* pcView, bool bTrans);

//���s�L���`��
//2007.08.30 kobake �ǉ�
void _DispEOL(CGraphics& gr, DispPos* pDispPos, CEol cEol, const CEditView* pcView, bool bTrans);


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        CFigure_Eol                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_Eol::Match(const wchar_t* pText) const
{
	if(pText[0]==L'\r' && pText[1]==L'\n' && pText[2]==L'\0')return true;
	if(pText[0]==L'\n' && pText[1]==L'\0')return true;
	if(pText[0]==L'\r' && pText[1]==L'\0')return true;
	return false;
}

// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
//$$ �������\�B
bool CFigure_Eol::DrawImp(SColorStrategyInfo* pInfo)
{
	CEditView* pcView = pInfo->pcView;
	const CEditDoc* pcDoc =  pcView->m_pcEditDoc;

	// ���s�擾
	const CLayout*	pcLayout2 = pcDoc->m_cLayoutMgr.SearchLineByLayoutY(pInfo->pDispPos->GetLayoutLineRef());
	CEol cEol = pcLayout2->GetLayoutEol();
	if(cEol.GetLen()){
		m_cEol = cEol;
		// this->CFigureSpace::DrawImp(pInfo);
		{
			// CFigureSpace::DrawImp_StyleSelect���ǂ��B�I���E�����F��D�悷��
			CTypeSupport cCurrentType(pcView, pInfo->GetCurrentColor());	// ���ӂ̐F�i���݂̎w��F/�I��F�j
			CTypeSupport cCurrentType2(pcView, pInfo->GetCurrentColor2());	// ���ӂ̐F�i���݂̎w��F�j
			CTypeSupport cTextType(pcView, COLORIDX_TEXT);				// �e�L�X�g�̎w��F
			CTypeSupport cSpaceType(pcView, pcView->GetTextDrawer()._GetColorIdx(GetColorIdx()));	// �󔒂̎w��F
			CTypeSupport cSearchType(pcView, COLORIDX_SEARCH);	// �����F(EOL�ŗL)
			CTypeSupport cCurrentTypeBg(pcView, pInfo->GetCurrentColorBg());
			CTypeSupport& cCurrentType3 = (cCurrentType2.GetBackColor() == cTextType.GetBackColor() ? cCurrentTypeBg: cCurrentType2);
			COLORREF crText;
			COLORREF crBack;
			bool bSelecting = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2();
			bool blendColor = bSelecting && cCurrentType.GetTextColor() == cCurrentType.GetBackColor(); // �I�������F
			CTypeSupport& currentStyle = blendColor ? cCurrentType2 : cCurrentType;
			CTypeSupport *pcText, *pcBack;
			if( bSelecting && !blendColor ){
				// �I�𕶎��F�Œ�w��
				pcText = &cCurrentType;
				pcBack = &cCurrentType;
			}else if( pInfo->GetCurrentColor2() == COLORIDX_SEARCH ){
				// �����F�D��
				pcText = &cSearchType;
				pcBack = &cSearchType;
			}else{
				pcText = cSpaceType.GetTextColor() == cTextType.GetTextColor() ? &cCurrentType2 : &cSpaceType;
				pcBack = cSpaceType.GetBackColor() == cTextType.GetBackColor() ? &cCurrentType3 : &cSpaceType;
			}
			if( blendColor ){
				// �����F(�����F��D�悵��)
				crText = pcView->GetTextColorByColorInfo2(cCurrentType.GetColorInfo(), pcText->GetColorInfo());
				crBack = pcView->GetBackColorByColorInfo2(cCurrentType.GetColorInfo(), pcBack->GetColorInfo());
			}else{
				crText = pcText->GetTextColor();
				crBack = pcBack->GetBackColor();
			}
			pInfo->gr.PushTextForeColor(crText);
			pInfo->gr.PushTextBackColor(crBack);
			bool bTrans = pcView->IsBkBitmap() && cTextType.GetBackColor() == crBack;
			pInfo->gr.PushMyFont(
				pInfo->pcView->GetFontset().ChooseFontHandle(cSpaceType.IsFatFont() || currentStyle.IsFatFont(), cSpaceType.HasUnderLine())
			);
			
			DispPos sPos(*pInfo->pDispPos);	// ���݈ʒu���o���Ă���
			_DispEOL(pInfo->gr, pInfo->pDispPos, cEol, pcView, bTrans);
			DrawImp_StylePop(pInfo);
			DrawImp_DrawUnderline(pInfo, sPos);
		}
		pInfo->nPosInLogic+=cEol.GetLen();
	}

	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �܂�Ԃ��`�����                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// �܂�Ԃ��`��
void _DispWrap(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView)
{
	RECT rcClip2;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,1))
	{
		//�T�|�[�g�N���X
		CTypeSupport cWrapType(pcView,COLORIDX_WRAP);
		bool bTrans = pcView->IsBkBitmap() && cWrapType.GetBackColor() == CTypeSupport(pcView,COLORIDX_TEXT).GetBackColor();

		//�`�敶����ƐF�̌���
		const wchar_t* szText;
		if( cWrapType.IsDisp() )
		{
			szText = L"<";
			cWrapType.SetGraphicsState_WhileThisObj(gr);
		}
		else
		{
			szText = L" ";
		}

		//�`��
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip2,
			szText,
			wcslen(szText),
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}
	pDispPos->ForwardDrawCol(1);
}
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       EOF�`�����                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CFigure_Eol::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	_DispEOL(gr, pDispPos, m_cEol, pcView, bTrans);
}

/*!
EOF�L���̕`��
@date 2004.05.29 genta  MIK����̃A�h�o�C�X�ɂ��֐��ɂ����肾��
@date 2007.08.28 kobake ���� nCharWidth �폜
@date 2007.08.28 kobake ���� fuOptions �폜
@date 2007.08.30 kobake ���� EofColInfo �폜
*/
void _DispEOF(
	CGraphics&			gr,			//!< [in] �`��Ώۂ�Device Context
	DispPos*			pDispPos,	//!< [in] �\�����W
	const CEditView*	pcView
)
{
	// �`��Ɏg���F���
	CTypeSupport cEofType(pcView,COLORIDX_EOF);
	if(!cEofType.IsDisp())
		return;
	CTypeSupport cTextType(pcView,COLORIDX_TEXT);
	bool bTrans = pcView->IsBkBitmap() && cEofType.GetBackColor() == cTextType.GetBackColor();

	//�K�v�ȃC���^�[�t�F�[�X���擾
	const CTextMetrics* pMetrics=&pcView->GetTextMetrics();
	const CTextArea* pArea=&pcView->GetTextArea();

	//�萔
	const wchar_t	szEof[] = L"[EOF]";
	const int		nEofLen = _countof(szEof) - 1;

	//�N���b�s���O�̈���v�Z
	RECT rcClip;
	if(pArea->GenerateClipRect(&rcClip,*pDispPos,nEofLen))
	{
		//�F�ݒ�
		cEofType.SetGraphicsState_WhileThisObj(gr);

		//�`��
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip,
			szEof,
			nEofLen,
			pMetrics->GetDxArray_AllHankaku()
		);
	}

	//�`��ʒu��i�߂�
	pDispPos->ForwardDrawCol(nEofLen);
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ���s�`�����                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//��ʕ`��⏕�֐�
//May 23, 2000 genta
//@@@ 2001.12.21 YAZAKI ���s�L���̏����������ς������̂ŏC��
void _DrawEOL(
	CGraphics&		gr,
	const CMyRect&	rcEol,
	CEol			cEol,
	bool			bBold,
	COLORREF		pColor
);

//2007.08.30 kobake �ǉ�
void _DispEOL(CGraphics& gr, DispPos* pDispPos, CEol cEol, const CEditView* pcView, bool bTrans)
{
	RECT rcClip2;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,2)){
		// 2003.08.17 ryoji ���s�����������Ȃ��悤��
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcClip2,
			L"  ",
			2,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		// ���s�L���̕\��
		if( CTypeSupport(pcView,COLORIDX_EOL).IsDisp() ){
			// From Here 2003.08.17 ryoji ���s�����������Ȃ��悤��

			// ���[�W�����쐬�A�I���B
			gr.SetClipping(rcClip2);
			
			// �`��̈�
			CMyRect rcEol;
			rcEol.SetPos(pDispPos->GetDrawPos().x + 1, pDispPos->GetDrawPos().y);
			rcEol.SetSize(pcView->GetTextMetrics().GetHankakuWidth(), pcView->GetTextMetrics().GetHankakuHeight());

			// �`��
			// �����F�⑾�����ǂ��������݂� DC ���璲�ׂ�	// 2009.05.29 ryoji 
			// �i�����}�b�`���̏󋵂ɏ_��ɑΉ����邽�߁A�����͋L���̐F�w��ɂ͌��ߑł����Ȃ��j
			TEXTMETRIC tm;
			::GetTextMetrics(gr, &tm);
			LONG lfWeightNormal = pcView->m_pcEditWnd->GetLogfont().lfWeight;
			_DrawEOL(gr, rcEol, cEol, tm.tmWeight > lfWeightNormal, GetTextColor(gr));

			// ���[�W�����j��
			gr.ClearClipping();
			
			// To Here 2003.08.17 ryoji ���s�����������Ȃ��悤��
		}
	}

	//�`��ʒu��i�߂�(2��)
	pDispPos->ForwardDrawCol(2);
}


//	May 23, 2000 genta
/*!
��ʕ`��⏕�֐�:
�s���̉��s�}�[�N�����s�R�[�h�ɂ���ď���������i���C���j

@note bBold��TRUE�̎��͉���1�h�b�g���炵�ďd�ˏ������s�����A
���܂葾�������Ȃ��B

@date 2001.12.21 YAZAKI ���s�L���̕`��������ύX�B�y���͂��̊֐����ō��悤�ɂ����B
						���̐擪���Asx, sy�ɂ��ĕ`�惋�[�`�����������B
*/
void _DrawEOL(
	CGraphics&		gr,		//!< Device Context Handle
	const CMyRect&	rcEol,		//!< �`��̈�
	CEol			cEol,		//!< �s���R�[�h���
	bool			bBold,		//!< TRUE: ����
	COLORREF		pColor		//!< �F
)
{
	int sx, sy;	//	���̐擪
	gr.SetPen( pColor );

	switch( cEol.GetType() ){
	case EOL_CRLF:	//	�������
	{
		sx = rcEol.left;						//X���[
		sy = rcEol.top + ( rcEol.Height() / 2);	//Y���S
		DWORD pp[] = { 3, 3 };
		POINT pt[6];
		pt[0].x = sx + rcEol.Width();	//	���
		pt[0].y = sy - rcEol.Height() / 4;
		pt[1].x = sx + rcEol.Width();	//	����
		pt[1].y = sy;
		pt[2].x = sx;	//	�擪��
		pt[2].y = sy;
		pt[3].x = sx + rcEol.Height() / 4;	//	�擪���牺��
		pt[3].y = sy + rcEol.Height() / 4;
		pt[4].x = sx;	//	�擪�֖߂�
		pt[4].y = sy;
		pt[5].x = sx + rcEol.Height() / 4;	//	�擪������
		pt[5].y = sy - rcEol.Height() / 4;
		::PolyPolyline( gr, pt, pp, _countof(pp));

		if ( bBold ) {
			pt[0].x += 1;	//	��ցi�E�ւ��炷�j
			pt[0].y += 0;
			pt[1].x += 1;	//	�E�ցi�E�ɂЂƂ���Ă���j
			pt[1].y += 1;
			pt[2].x += 0;	//	�擪��
			pt[2].y += 1;
			pt[3].x += 0;	//	�擪���牺��
			pt[3].y += 1;
			pt[4].x += 0;	//	�擪�֖߂�
			pt[4].y += 1;
			pt[5].x += 0;	//	�擪������
			pt[5].y += 1;
			::PolyPolyline( gr, pt, pp, _countof(pp));
		}
	}
		break;
	case EOL_CR:	//	���������	// 2007.08.17 ryoji EOL_LF -> EOL_CR
	{
		sx = rcEol.left;
		sy = rcEol.top + ( rcEol.Height() / 2 );
		DWORD pp[] = { 3, 2 };
		POINT pt[5];
		pt[0].x = sx + rcEol.Width();	//	�E��
		pt[0].y = sy;
		pt[1].x = sx;	//	�擪��
		pt[1].y = sy;
		pt[2].x = sx + rcEol.Height() / 4;	//	�擪���牺��
		pt[2].y = sy + rcEol.Height() / 4;
		pt[3].x = sx;	//	�擪�֖߂�
		pt[3].y = sy;
		pt[4].x = sx + rcEol.Height() / 4;	//	�擪������
		pt[4].y = sy - rcEol.Height() / 4;
		::PolyPolyline( gr, pt, pp, _countof(pp));

		if ( bBold ) {
			pt[0].x += 0;	//	�E��
			pt[0].y += 1;
			pt[1].x += 0;	//	�擪��
			pt[1].y += 1;
			pt[2].x += 0;	//	�擪���牺��
			pt[2].y += 1;
			pt[3].x += 0;	//	�擪�֖߂�
			pt[3].y += 1;
			pt[4].x += 0;	//	�擪������
			pt[4].y += 1;
			::PolyPolyline( gr, pt, pp, _countof(pp));
		}
	}
		break;
	case EOL_LF:	//	���������	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		sx = rcEol.left + ( rcEol.Width() / 2 );
		sy = rcEol.top + ( rcEol.Height() * 3 / 4 );
		DWORD pp[] = { 3, 2 };
		POINT pt[5];
		pt[0].x = sx;	//	���
		pt[0].y = rcEol.top + rcEol.Height() / 4 + 1;
		pt[1].x = sx;	//	�ォ�牺��
		pt[1].y = sy;
		pt[2].x = sx - rcEol.Height() / 4;	//	���̂܂܍����
		pt[2].y = sy - rcEol.Height() / 4;
		pt[3].x = sx;	//	���̐�[�ɖ߂�
		pt[3].y = sy;
		pt[4].x = sx + rcEol.Height() / 4;	//	�����ĉE���
		pt[4].y = sy - rcEol.Height() / 4;
		::PolyPolyline( gr, pt, pp, _countof(pp));

		if( bBold ){
			pt[0].x += 1;	//	���
			pt[0].y += 0;
			pt[1].x += 1;	//	�ォ�牺��
			pt[1].y += 0;
			pt[2].x += 1;	//	���̂܂܍����
			pt[2].y += 0;
			pt[3].x += 1;	//	���̐�[�ɖ߂�
			pt[3].y += 0;
			pt[4].x += 1;	//	�����ĉE���
			pt[4].y += 0;
			::PolyPolyline( gr, pt, pp, _countof(pp));
		}
		break;
	}
}
