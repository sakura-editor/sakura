#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_Tab.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "types/CTypeSupport.h"

//2007.08.28 kobake �ǉ�
void _DispTab( CGraphics& gr, DispPos* pDispPos, const CEditView* pcView );
//�^�u���`��֐�	//@@@ 2003.03.26 MIK
void _DrawTabArrow( CGraphics& gr, int nPosX, int nPosY, int nWidth, int nHeight, bool bBold, COLORREF pColor );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CFigure_Tab                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_Tab::Match(const wchar_t* pText) const
{
	if( pText[0] == WCODE::TAB ){
		return true;
	}
	return false;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �`�����                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! TAB�`��
	@date 2001.03.16 by MIK
	@date 2002.09.22 genta ���ʎ��̂����肾��
	@date 2002.09.23 genta LayoutMgr�̒l���g��
	@date 2003.03.26 MIK �^�u���\��
	@date 2013.05.31 novice TAB�\���Ή�(�����w��/�Z�����/�������)
*/
void CFigure_Tab::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	DispPos& sPos=*pDispPos;

	//�K�v�ȃC���^�[�t�F�[�X
	const CTextMetrics* pMetrics=&pcView->GetTextMetrics();
	const CTextArea* pArea=&pcView->GetTextArea();

	int nLineHeight = pMetrics->GetHankakuDy();
	int nCharWidth = pMetrics->GetHankakuDx();

	CTypeSupport cTabType(pcView,COLORIDX_TAB);

	// ���ꂩ��`�悷��^�u��
	int tabDispWidth = (Int)pcView->m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( sPos.GetDrawCol() );

	// �^�u�L���̈�
	RECT rcClip2;
	rcClip2.left = sPos.GetDrawPos().x;
	rcClip2.right = rcClip2.left + nCharWidth * tabDispWidth;
	if( rcClip2.left < pArea->GetAreaLeft() ){
		rcClip2.left = pArea->GetAreaLeft();
	}
	rcClip2.top = sPos.GetDrawPos().y;
	rcClip2.bottom = sPos.GetDrawPos().y + nLineHeight;

	if( pArea->IsRectIntersected(rcClip2) ){
		if( cTabType.IsDisp() && TABARROW_STRING == m_pTypeData->m_bTabArrow ){	//�^�u�ʏ�\��	//@@@ 2003.03.26 MIK
			//@@@ 2001.03.16 by MIK
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
				&rcClip2,
				m_pTypeData->m_szTabViewString,
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);
		}else{
			//�w�i
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
				&rcClip2,
				L"        ",
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);

			//�^�u���\��
			if( cTabType.IsDisp() ){
				// �����F�⑾�����ǂ��������݂� DC ���璲�ׂ�	// 2009.05.29 ryoji 
				// �i�����}�b�`���̏󋵂ɏ_��ɑΉ����邽�߁A�����͋L���̐F�w��ɂ͌��ߑł����Ȃ��j
				//	�������ǂ����ݒ������l�ɂ��� 2013/4/11 Uchi
				// 2013.06.21 novice �����F�A������CGraphics����擾

				if( TABARROW_SHORT == m_pTypeData->m_bTabArrow ){
					if( rcClip2.left <= sPos.GetDrawPos().x ){ // Apr. 1, 2003 MIK �s�ԍ��Əd�Ȃ�
						_DrawTabArrow(
							gr,
							sPos.GetDrawPos().x,
							sPos.GetDrawPos().y,
							pMetrics->GetHankakuWidth(),
							pMetrics->GetHankakuHeight(),
							gr.GetCurrentMyFontBold() || m_pTypeData->m_ColorInfoArr[COLORIDX_TAB].m_bBoldFont,
							gr.GetCurrentTextForeColor()
						);
					}
				} else if( TABARROW_LONG == m_pTypeData->m_bTabArrow ){
					int	nPosLeft = rcClip2.left > sPos.GetDrawPos().x ? rcClip2.left : sPos.GetDrawPos().x;
					_DrawTabArrow(
						gr,
						nPosLeft,
						sPos.GetDrawPos().y,
						nCharWidth * tabDispWidth - (nPosLeft -  sPos.GetDrawPos().x),	// Tab Area��t�� 2013/4/11 Uchi
						pMetrics->GetHankakuHeight(),
						gr.GetCurrentMyFontBold() || m_pTypeData->m_ColorInfoArr[COLORIDX_TAB].m_bBoldFont,
						gr.GetCurrentTextForeColor()
					);
				}
			}
		}
	}

	//X��i�߂�
	sPos.ForwardDrawCol(tabDispWidth);
}



/*
	�^�u���`��֐�
*/
void _DrawTabArrow(
	CGraphics&	gr,
	int			nPosX,   //�s�N�Z��X
	int			nPosY,   //�s�N�Z��Y
	int			nWidth,  //�s�N�Z��W
	int			nHeight, //�s�N�Z��H
	bool		bBold,
	COLORREF	pColor
)
{
	// �y���ݒ�
	gr.PushPen( pColor, 0 );

	// ���̐擪
	int sx = nPosX + nWidth - 2;
	int sy = nPosY + ( nHeight / 2 );
	int sa = nHeight / 4;								// �V��size

	DWORD pp[] = { 3, 2 };
	POINT pt[5];
	pt[0].x = nPosX;	//�u���v���[����E�[
	pt[0].y = sy;
	pt[1].x = sx;		//�u�^�v�E�[����΂ߍ���
	pt[1].y = sy;
	pt[2].x = sx - sa;	//	���̐�[�ɖ߂�
	pt[2].y = sy + sa;
	pt[3].x = sx;		//�u�_�v�E�[����΂ߍ���
	pt[3].y = sy;
	pt[4].x = sx - sa;
	pt[4].y = sy - sa;
	::PolyPolyline( gr, pt, pp, _countof(pp));

	if( bBold ){
		pt[0].x += 0;	//�u���v���[����E�[
		pt[0].y += 1;
		pt[1].x += 0;	//�u�^�v�E�[����΂ߍ���
		pt[1].y += 1;
		pt[2].x += 0;	//	���̐�[�ɖ߂�
		pt[2].y += 1;
		pt[3].x += 0;	//�u�_�v�E�[����΂ߍ���
		pt[3].y += 1;
		pt[4].x += 0;
		pt[4].y += 1;
		::PolyPolyline( gr, pt, pp, _countof(pp));
	}

	gr.PopPen();
}
