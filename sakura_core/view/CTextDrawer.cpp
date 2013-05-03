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

#include "StdAfx.h"
#include "CTextDrawer.h"
#include <vector>
#include "CTextMetrics.h"
#include "CTextArea.h"
#include "CViewFont.h"
#include "CEol.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "types/CTypeSupport.h"
#include "charset/charcode.h"
#include "doc/layout/CLayout.h"
#include "view/colors/CColorStrategy.h"


const CTextArea* CTextDrawer::GetTextArea() const
{
	return &m_pEditView->GetTextArea();
}

using namespace std;

EColorIndexType CTextDrawer::_GetColorIdx(EColorIndexType nColorIdx) const
{
	if(CTypeSupport(m_pEditView,nColorIdx).IsDisp())return nColorIdx;	//����F
	return COLORIDX_TEXT;												//�ʏ�F
}


/*
�e�L�X�g�\��
@@@ 2002.09.22 YAZAKI    const unsigned char* pData���Aconst char* pData�ɕύX
@@@ 2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
*/
void CTextDrawer::DispText( HDC hdc, DispPos* pDispPos, const wchar_t* pData, int nLength, bool bTransparent ) const
{
	if( 0 >= nLength ){
		return;
	}
	int x=pDispPos->GetDrawPos().x;
	int y=pDispPos->GetDrawPos().y;

	//�K�v�ȃC���^�[�t�F�[�X���擾
	const CTextMetrics* pMetrics=&m_pEditView->GetTextMetrics();
	const CTextArea* pArea=GetTextArea();

	//�����Ԋu�z��𐶐�
	static vector<int> vDxArray(1);
	const int* pDxArray=pMetrics->GenerateDxArray(&vDxArray,pData,nLength,this->m_pEditView->GetTextMetrics().GetHankakuDx());

	//������̃s�N�Z����
	int nTextWidth=pMetrics->CalcTextWidth(pData,nLength,pDxArray);

	//�e�L�X�g�̕`��͈͂̋�`�����߂� -> rcClip
	CMyRect rcClip;
	rcClip.left   = x;
	rcClip.right  = x + nTextWidth;
	rcClip.top    = y;
	rcClip.bottom = y + m_pEditView->GetTextMetrics().GetHankakuDy();
	if( rcClip.left < pArea->GetAreaLeft() ){
		rcClip.left = pArea->GetAreaLeft();
	}

	//�����Ԋu
	int nDx = m_pEditView->GetTextMetrics().GetHankakuDx();

	if( pArea->IsRectIntersected(rcClip) && rcClip.top >= pArea->GetAreaTop() ){

		//@@@	From Here 2002.01.30 YAZAKI ExtTextOutW_AnyBuild�̐������
		if( rcClip.Width() > pArea->GetAreaWidth() ){
			rcClip.right = rcClip.left + pArea->GetAreaWidth();
		}

		// �E�B���h�E�̍��ɂ��ӂꂽ������ -> nBefore
		// 2007.09.08 kobake�� �u�E�B���h�E�̍��v�ł͂Ȃ��u�N���b�v�̍��v�����Ɍv�Z�����ق����`��̈��ߖ�ł��邪�A
		//                        �o�O���o��̂��|���̂łƂ肠�������̂܂܁B
		int nBeforeLogic = 0;
		CLayoutInt nBeforeLayout = CLayoutInt(0);
		if ( x < 0 ){
			int nLeftLayout = ( 0 - x ) / nDx - 1;
			while (nBeforeLayout < nLeftLayout){
				nBeforeLayout += CNativeW::GetKetaOfChar( pData, nLength, nBeforeLogic );
				nBeforeLogic  += CNativeW::GetSizeOfChar( pData, nLength, nBeforeLogic );
			}
		}

		/*
		// �E�B���h�E�̉E�ɂ��ӂꂽ������ -> nAfter
		int nAfterLayout = 0;
		if ( rcClip.right < x + nTextWidth ){
			//	-1���Ă��܂����i������͂�����ˁH�j
			nAfterLayout = (x + nTextWidth - rcClip.right) / nDx - 1;
		}
		*/

		// �`��J�n�ʒu
		int nDrawX = x + (Int)nBeforeLayout * nDx;

		// ���ۂ̕`�敶����|�C���^
		const wchar_t* pDrawData          = &pData[nBeforeLogic];
		int            nDrawDataMaxLength = nLength - nBeforeLogic;

		// ���ۂ̕����Ԋu�z��
		const int* pDrawDxArray = &pDxArray[nBeforeLogic];

		// �`�悷�镶���񒷂����߂� -> nDrawLength
		int nRequiredWidth = rcClip.right - nDrawX; //���߂�ׂ��s�N�Z����
		if(nRequiredWidth <= 0)goto end;
		int nWorkWidth = 0;
		int nDrawLength = 0;
		while(nWorkWidth < nRequiredWidth)
		{
			if(nDrawLength >= nDrawDataMaxLength)break;
			nWorkWidth += pDrawDxArray[nDrawLength++];
		}
		// �T���Q�[�g�y�A�΍�	2008/7/5 Uchi	Update 7/8 Uchi
		if (nDrawLength < nDrawDataMaxLength && pDrawDxArray[nDrawLength] == 0) {
			nDrawLength++;
		}

		//�`��
		::ExtTextOutW_AnyBuild(
			hdc,
			nDrawX,					//X
			y,						//Y
			ExtTextOutOption() & ~(bTransparent? ETO_OPAQUE: 0),
			&rcClip,
			pDrawData,				//������
			nDrawLength,			//������
			pDrawDxArray			//�����Ԋu�̓������z��
		);
	}

end:
	//�`��ʒu��i�߂�
	pDispPos->ForwardDrawCol(nTextWidth / nDx);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �w�茅�c��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�w�茅�c���̕`��
	@date 2005.11.08 Moca �V�K�쐬
	@date 2006.04.29 Moca �����E�_���̃T�|�[�g�B�I�𒆂̔��]�΍�ɍs���Ƃɍ�悷��悤�ɕύX
	    �c���̐F���e�L�X�g�̔w�i�F�Ɠ����ꍇ�́A�c���̔w�i�F��EXOR�ō�悷��
	@note Common::m_nVertLineOffset�ɂ��A�w�茅�̑O�̕����̏�ɍ�悳��邱�Ƃ�����B
*/
void CTextDrawer::DispVerticalLines(
	CGraphics&	gr,			//!< ��悷��E�B���h�E��DC
	int			nTop,		//!< ����������[�̃N���C�A���g���Wy
	int			nBottom,	//!< �����������[�̃N���C�A���g���Wy
	CLayoutInt	nLeftCol,	//!< ���������͈͂̍����̎w��
	CLayoutInt	nRightCol	//!< ���������͈͂̉E���̎w��(-1�Ŗ��w��)
) const
{
	const CEditView* pView=m_pEditView;

	const STypeConfig&	typeData = pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	CTypeSupport cVertType(pView,COLORIDX_VERTLINE);
	CTypeSupport cTextType(pView,COLORIDX_TEXT);

	if(!cVertType.IsDisp())return;

	nLeftCol = t_max( pView->GetTextArea().GetViewLeftCol(), nLeftCol );

	const CLayoutInt nWrapKetas  = pView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
	const int nCharDx  = pView->GetTextMetrics().GetHankakuDx();
	if( nRightCol < 0 ){
		nRightCol = nWrapKetas;
	}
	const int nPosXOffset = GetDllShareData().m_Common.m_sWindow.m_nVertLineOffset + pView->GetTextArea().GetAreaLeft();
	const int nPosXLeft   = t_max( pView->GetTextArea().GetAreaLeft() + (Int)(nLeftCol  - pView->GetTextArea().GetViewLeftCol()) * nCharDx, pView->GetTextArea().GetAreaLeft() );
	const int nPosXRight  = t_min( pView->GetTextArea().GetAreaLeft() + (Int)(nRightCol - pView->GetTextArea().GetViewLeftCol()) * nCharDx, pView->GetTextArea().GetAreaRight() );
	const int nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	bool bOddLine = ((((nLineHeight % 2) ? (Int)pView->GetTextArea().GetViewTopLine() : 0) + pView->GetTextArea().GetAreaTop() + nTop) % 2 == 1);

	// ����
	const bool bBold = cVertType.IsBoldFont();
	// �h�b�g��(����������]�p/�e�X�g�p)
	const bool bDot = cVertType.HasUnderLine();
	const bool bExorPen = ( cVertType.GetTextColor() == cTextType.GetBackColor() );
	int nROP_Old = 0;
	if( bExorPen ){
		gr.SetPen( cVertType.GetBackColor() );
		nROP_Old = ::SetROP2( gr, R2_NOTXORPEN );
	}
	else{
		gr.SetPen( cVertType.GetTextColor() );
	}

	int k;
	for( k = 0; k < MAX_VERTLINES && typeData.m_nVertLineIdx[k] != 0; k++ ){
		// nXCol��1�J�n�BGetTextArea().GetViewLeftCol()��0�J�n�Ȃ̂Œ��ӁB
		CLayoutInt nXCol = typeData.m_nVertLineIdx[k];
		CLayoutInt nXColEnd = nXCol;
		CLayoutInt nXColAdd = CLayoutInt(1);
		// nXCol���}�C�i�X���ƌJ��Ԃ��Bk+1���I���l�Ak+2���X�e�b�v���Ƃ��ė��p����
		if( nXCol < 0 ){
			if( k < MAX_VERTLINES - 2 ){
				nXCol = -nXCol;
				nXColEnd = typeData.m_nVertLineIdx[++k];
				nXColAdd = typeData.m_nVertLineIdx[++k];
				if( nXColEnd < nXCol || nXColAdd <= 0 ){
					continue;
				}
				// ���͈͂̎n�߂܂ŃX�L�b�v
				if( nXCol < pView->GetTextArea().GetViewLeftCol() ){
					nXCol = pView->GetTextArea().GetViewLeftCol() + nXColAdd - (pView->GetTextArea().GetViewLeftCol() - nXCol) % nXColAdd;
				}
			}else{
				k += 2;
				continue;
			}
		}
		for(; nXCol <= nXColEnd; nXCol += nXColAdd ){
			if( nWrapKetas < nXCol ){
				break;
			}
			int nPosX = nPosXOffset + (Int)( nXCol - 1 - pView->GetTextArea().GetViewLeftCol() ) * nCharDx;
			// 2006.04.30 Moca ���̈����͈́E���@��ύX
			// �����̏ꍇ�A����������悷��\��������B
			int nPosXBold = nPosX;
			if( bBold ){
				nPosXBold -= 1;
			}
			if( nPosXRight <= nPosXBold ){
				break;
			}
			if( nPosXLeft <= nPosX ){
				if( bDot ){
					// �_���ō��B1�h�b�g�̐����쐬
					int y = nTop;
					// �X�N���[�����Ă������؂�Ȃ��悤�ɍ��W�𒲐�
					if( bOddLine ){
						y++;
					}
					for( ; y < nBottom; y += 2 ){
						if( nPosX < nPosXRight ){
							::MoveToEx( gr, nPosX, y, NULL );
							::LineTo( gr, nPosX, y + 1 );
						}
						if( bBold && nPosXLeft <= nPosXBold ){
							::MoveToEx( gr, nPosXBold, y, NULL );
							::LineTo( gr, nPosXBold, y + 1 );
						}
					}
				}else{
					if( nPosX < nPosXRight ){
						::MoveToEx( gr, nPosX, nTop, NULL );
						::LineTo( gr, nPosX, nBottom );
					}
					if( bBold && nPosXLeft <= nPosXBold ){
						::MoveToEx( gr, nPosXBold, nTop, NULL );
						::LineTo( gr, nPosXBold, nBottom );
					}
				}
			}
		}
	}
	if( bExorPen ){
		::SetROP2( gr, nROP_Old );
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �܂�Ԃ����c��                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�܂�Ԃ����c���̕`��
	@date 2009.10.24 ryoji �V�K�쐬
*/
void CTextDrawer::DispWrapLine(
	CGraphics&	gr,			//!< ��悷��E�B���h�E��DC
	int			nTop,		//!< ����������[�̃N���C�A���g���Wy
	int			nBottom		//!< �����������[�̃N���C�A���g���Wy
) const
{
	const CEditView* pView = m_pEditView;
	CTypeSupport cWrapType(pView, COLORIDX_WRAP);
	if( !cWrapType.IsDisp() ) return;

	const CTextArea& rArea = *GetTextArea();
	const CLayoutInt nWrapKetas = pView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();
	const int nCharDx = pView->GetTextMetrics().GetHankakuDx();
	int nXPos = rArea.GetAreaLeft() + (Int)( nWrapKetas - rArea.GetViewLeftCol() ) * nCharDx;
	//	2005.11.08 Moca �������ύX
	if( rArea.GetAreaLeft() < nXPos && nXPos < rArea.GetAreaRight() ){
		/// �܂�Ԃ��L���̐F�̃y����ݒ�
		gr.PushPen(cWrapType.GetTextColor(), 0);

		::MoveToEx( gr, nXPos, nTop, NULL );
		::LineTo( gr, nXPos, nBottom );

		gr.PopPen();
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �s�ԍ�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispLineNumber(
	CGraphics&		gr,
	CLayoutInt		nLineNum,
	int				y
) const
{
	//$$ �������FSearchLineByLayoutY�ɃL���b�V������������
	const CLayout*	pcLayout = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( nLineNum );

	const CEditView* pView=m_pEditView;
	const STypeConfig* pTypes=&pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	int				nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int				nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	// �s�ԍ��\������X��	Sep. 23, 2002 genta ���ʎ��̂����肾��
	//int				nLineNumAreaWidth = pView->GetTextArea().m_nViewAlignLeftCols * nCharWidth;
	int				nLineNumAreaWidth = pView->GetTextArea().GetAreaLeft() - GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace;	// 2009.03.26 ryoji

	CTypeSupport cTextType(pView,COLORIDX_TEXT);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     nColorIndex������                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	EColorIndexType nColorIndex = COLORIDX_GYOU;	/* �s�ԍ� */
	const CDocLine*	pCDocLine = NULL;
	if( pcLayout ){
		pCDocLine = pcLayout->GetDocLineRef();

		if( pView->m_pcEditDoc->m_cDocEditor.IsModified() && CModifyVisitor().IsLineModified(pCDocLine) ){		/* �ύX�t���O */
			if( CTypeSupport(pView,COLORIDX_GYOU_MOD).IsDisp() )	// 2006.12.12 ryoji
				nColorIndex = COLORIDX_GYOU_MOD;	/* �s�ԍ��i�ύX�s�j */
		}
	}

	if(pCDocLine){
		//DIFF�F�ݒ�
		CDiffLineGetter(pCDocLine).GetDiffColor(&nColorIndex);

		// 02/10/16 ai
		// �u�b�N�}�[�N�̕\��
		if(CBookmarkGetter(pCDocLine).IsBookmarked()){
			if( CTypeSupport(pView,COLORIDX_MARK).IsDisp() ) {
				nColorIndex = COLORIDX_MARK;
			}
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             ���肳�ꂽnColorIndex���g���ĕ`��               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	CTypeSupport cColorType(pView,nColorIndex);
	CTypeSupport cMarkType(pView,COLORIDX_MARK);

	//�Y���s�̍s�ԍ��G���A��`
	RECT	rcLineNum;
	rcLineNum.left = 0;
	rcLineNum.right = nLineNumAreaWidth;
	rcLineNum.top = y;
	rcLineNum.bottom = y + nLineHeight;
	
	bool bTrans = pView->IsBkBitmap() && cTextType.GetBackColor() == cColorType.GetBackColor();
	bool bTransText = pView->IsBkBitmap();

	if(!pcLayout){
		//�s�����݂��Ȃ��ꍇ�́A�e�L�X�g�`��F�œh��Ԃ�
		if( !bTransText ){
			cTextType.FillBack(gr,rcLineNum);
		}
	}
	else if( CTypeSupport(pView,COLORIDX_GYOU).IsDisp() ){ /* �s�ԍ��\���^��\�� */
		gr.PushTextForeColor(cColorType.GetTextColor());	//�e�L�X�g�F�s�ԍ��̐F
		gr.PushTextBackColor( cColorType.GetBackColor() );	//�e�L�X�g�F�s�ԍ��w�i�̐F
		gr.PushMyFont(cColorType.GetTypeFont());			//�t�H���g�F�s�ԍ��̃t�H���g

		//�`�敶����
		wchar_t szLineNum[18];
		int nLineCols;
		int nLineNumCols;
		{
			/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
			if( pTypes->m_bLineNumIsCRLF ){
				/* �_���s�ԍ��\�����[�h */
				if( NULL == pcLayout || 0 != pcLayout->GetLogicOffset() ){ //�܂�Ԃ����C�A�E�g�s
					wcscpy( szLineNum, L" " );
				}else{
					_itow( pcLayout->GetLogicLineNo() + 1, szLineNum, 10 );	/* �Ή�����_���s�ԍ� */
				}
			}else{
				/* �����s�i���C�A�E�g�s�j�ԍ��\�����[�h */
				_itow( (Int)nLineNum + 1, szLineNum, 10 );
			}
			nLineCols = wcslen( szLineNum );
			nLineNumCols = nLineCols; // 2010.08.17 Moca �ʒu����ɍs�ԍ���؂�͊܂߂Ȃ�

			/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
			if( 2 == pTypes->m_nLineTermType ){
				//	Sep. 22, 2002 genta
				szLineNum[ nLineCols ] = pTypes->m_cLineTermChar;
				szLineNum[ ++nLineCols ] = '\0';
			}
		}

		//	Sep. 23, 2002 genta
		int drawNumTop = (pView->GetTextArea().m_nViewAlignLeftCols - nLineNumCols - 1) * ( nCharWidth );
		::ExtTextOutW_AnyBuild( gr,
			drawNumTop,
			y,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rcLineNum,
			szLineNum,
			nLineCols,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		if( 1 == pTypes->m_nLineTermType ){
			gr.PushPen(cColorType.GetTextColor(),0);
			::MoveToEx( gr, nLineNumAreaWidth - 2, y, NULL );
			::LineTo( gr, nLineNumAreaWidth - 2, y + nLineHeight );
			gr.PopPen();
		}

		gr.PopTextForeColor();
		gr.PopTextBackColor();
		gr.PopMyFont();
	}
	else{
		// �s�ԍ��G���A�̔w�i�`��
		if( !bTrans ){
			cColorType.FillBack(gr,rcLineNum);
		}
	}

	//�s�����`�� ($$$�����\��)
	if(pCDocLine)
	{
		// 2001.12.03 hor
		/* �Ƃ肠�����u�b�N�}�[�N�ɏc�� */
		if(CBookmarkGetter(pCDocLine).IsBookmarked() && !cMarkType.IsDisp() )
		{
			gr.PushPen(cColorType.GetTextColor(),2);
			::MoveToEx( gr, 1, y, NULL );
			::LineTo( gr, 1, y + nLineHeight );
			gr.PopPen();
		}

		//DIFF�}�[�N�`��
		CDiffLineGetter(pCDocLine).DrawDiffMark(gr,y,nLineHeight,cColorType);
	}

	// �s�ԍ��ƃe�L�X�g�̌��Ԃ̕`��
	if( !bTransText ){
		RECT rcRest;
		rcRest.left   = rcLineNum.right;
		rcRest.right  = pView->GetTextArea().GetAreaLeft();
		rcRest.top    = y;
		rcRest.bottom = y + nLineHeight;
		cTextType.FillBack(gr,rcRest);
	}
}

