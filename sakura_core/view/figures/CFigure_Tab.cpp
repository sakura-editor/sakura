#include "stdafx.h"
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
//                         CFigure_Tab                           //
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

//	Sep. 22, 2002 genta ���ʎ��̂����肾��
//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
//@@@ 2001.03.16 by MIK
//@@@ 2003.03.26 MIK �^�u���\��
void CFigure_Tab::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	DispPos& sPos=*pDispPos;

	//�K�v�ȃC���^�[�t�F�[�X
	const CTextMetrics* pMetrics=&pcView->GetTextMetrics();
	const CTextArea* pArea=&pcView->GetTextArea();
	STypeConfig* TypeDataPtr = &pcView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

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
		if( cTabType.IsDisp() && !TypeDataPtr->m_bTabArrow ){	//�^�u�ʏ�\��	//@@@ 2003.03.26 MIK
			//@@@ 2001.03.16 by MIK
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
				&rcClip2,
				TypeDataPtr->m_szTabViewString,
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
			if( cTabType.IsDisp() && TypeDataPtr->m_bTabArrow && rcClip2.left <= sPos.GetDrawPos().x ){ // Apr. 1, 2003 MIK �s�ԍ��Əd�Ȃ�
				// �����F�⑾�����ǂ��������݂� DC ���璲�ׂ�	// 2009.05.29 ryoji 
				// �i�����}�b�`���̏󋵂ɏ_��ɑΉ����邽�߁A�����͋L���̐F�w��ɂ͌��ߑł����Ȃ��j
				TEXTMETRIC tm;
				::GetTextMetrics(gr, &tm);
				LONG lfWeightNormal = CShareData::getInstance()->GetShareData()->m_Common.m_sView.m_lf.lfWeight;
				_DrawTabArrow(
					gr,
					sPos.GetDrawPos().x,
					sPos.GetDrawPos().y,
					pMetrics->GetHankakuWidth(),
					pMetrics->GetHankakuHeight(),
					tm.tmWeight > lfWeightNormal,
					::GetTextColor(gr)//cTabType.GetTextColor()
				);
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
	CGraphics&	_gr,
	int			nPosX,   //�s�N�Z��X
	int			nPosY,   //�s�N�Z��Y
	int			nWidth,  //�s�N�Z��W
	int			nHeight, //�s�N�Z��H
	bool		bBold,
	COLORREF	pColor
)
{
	// �ꎞ�I�ȃy�����g�p���邽�߁A�V���� CGraphics �I�u�W�F�N�g���쐬�B
	CGraphics gr(_gr);

	// �y���ݒ�
	gr.SetPen( pColor );

	// ������
	nWidth--;

	// ���̐擪
	int sx = nPosX + nWidth;
	int sy = nPosY + ( nHeight / 2 );

	for(int i = 0; i < (bBold?2:1); i++){
		int y = sy + i;
		gr.DrawLine(sx - nWidth,	y,	sx,					y				);	//�u���v���[����E�[
		gr.DrawLine(sx,				y,	sx - nHeight / 4,	y + nHeight / 4	);	//�u�^�v�E�[����΂ߍ���
		gr.DrawLine(sx,				y,	sx - nHeight / 4,	y - nHeight / 4	);	//�u�_�v�E�[����΂ߍ���
	}
}
