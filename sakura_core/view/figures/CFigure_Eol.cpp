#include "stdafx.h"
#include "CFigure_Eol.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"

//�܂�Ԃ��`��
void _DispWrap(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

//EOF�`��֐�
//���ۂɂ� pX �� nX ���X�V�����B
//2004.05.29 genta
//2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
//2007.08.25 kobake �������� nCharWidth, nLineHeight ���폜
//2007.08.28 kobake ���� fuOptions ���폜
void _DispEOF( CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

//��(����)�s�`��
bool _DispEmptyLine(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView);

//���s�L���`��
//2007.08.30 kobake �ǉ�
void _DispEOL(CGraphics& gr, DispPos* pDispPos, CEol cEol, const CEditView* pcView);


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

CLayoutInt CFigure_Eol::GetLayoutLength(const wchar_t* pText, CLayoutInt nStartCol) const
{
	return CLayoutInt(2);
}

// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
//$$ �������\�B
bool CFigure_Eol::DrawImp(SColorStrategyInfo* pInfo)
{
	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	CEditView* pcView = &CEditWnd::Instance()->GetActiveView();

	//�R���t�B�O
	CTypeSupport		cTextType	(pcView,COLORIDX_TEXT);
	int					nLineHeight	= pcView->GetTextMetrics().GetHankakuDy();
	const CLayoutInt	nWrapKeta	= pcDoc->m_cLayoutMgr.GetMaxLineKetas();	// �܂�Ԃ���

	// ���s�擾
	const CLayout*	pcLayout2 = CEditDoc::GetInstance(0)->m_cLayoutMgr.SearchLineByLayoutY( pInfo->pDispPos->GetLayoutLineRef() );
	CEol cEol = pcLayout2->GetLayoutEol();

	// �s���L����`��
	{
		// ���s�����݂����ꍇ�́A���s�L����\��
		if(cEol.GetLen()){
			m_cEol = cEol;
			__super::DrawImp(pInfo);
			pInfo->nPosInLogic+=cEol.GetLen();
		}
		// �ŏI�s�̏ꍇ�́AEOF��\��
		else if(pInfo->pDispPos->GetLayoutLineRef()+1==CEditDoc::GetInstance(0)->m_cLayoutMgr.GetLineCount() && pInfo->pDispPos->GetDrawCol() < nWrapKeta){
			if( TypeDataPtr->m_ColorInfoArr[COLORIDX_EOF].m_bDisp ){
				_DispEOF(pInfo->gr,pInfo->pDispPos,pInfo->pcView);
			}
			pInfo->nPosInLogic+=CLogicInt(1);
		}
		// ����ȊO�ł́A�܂�Ԃ��L����\��
		else{
			_DispWrap(pInfo->gr,pInfo->pDispPos,pcView);
			pInfo->nPosInLogic+=CLogicInt(1);
		}
	}

	// �s���w�i�`��
	RECT rcClip;
	if(pInfo->pcView->GetTextArea().GenerateClipRectRight(&rcClip,*pInfo->pDispPos)){
		cTextType.FillBack(pInfo->gr,rcClip);
	}

	// �c���`��
	pInfo->pcView->GetTextDrawer().DispVerticalLines(
		pInfo->gr,
		pInfo->pDispPos->GetDrawPos().y,
		pInfo->pDispPos->GetDrawPos().y + nLineHeight,
		CLayoutInt(0),
		CLayoutInt(-1)
	);



	return true;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ��(����)�s�`�����                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ��s��`��BEOF��`�悵���ꍇ��true��Ԃ��B
bool _DispEmptyLine(CGraphics& gr, DispPos* pDispPos, const CEditView* pcView)
{
	bool bEof=false;

	CTypeSupport cEofType(pcView,COLORIDX_EOF);
	CTypeSupport cTextType(pcView,COLORIDX_TEXT);

	const CLayoutInt nWrapKetas = pcView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	int nYPrev = pDispPos->GetDrawPos().y;
	
	if( !pcView->IsBkBitmap() ){
		// �w�i�`��
		RECT rcClip;
		pcView->GetTextArea().GenerateClipRectLine(&rcClip,*pDispPos);
		cTextType.FillBack(gr,rcClip);
	}

	// EOF�L���̕\��
	CLayoutInt nCount = pcView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();

	// �h�L�������g����(nCount==0)�B������1�s��(pDispPos->GetLayoutLineRef() == 0)�B�\�����1�s��(m_nViewTopLine==0)
	if( nCount == 0 && pcView->GetTextArea().GetViewTopLine() == 0 && pDispPos->GetLayoutLineRef() == 0 ){
		// EOF�L���̕\��
		if( cEofType.IsDisp() ){
			_DispEOF(gr,pDispPos,pcView);
		}

		bEof = true;
	}
	else{
		//�ŏI�s�̎��̍s
		if( nCount > 0 && pDispPos->GetLayoutLineRef() == nCount ){
			//�ŏI�s�̎擾
			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			const CLayout*	pcLayout;
			pLine = pcView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - CLayoutInt(1), &nLineLen, &pcLayout );
			
			//�ŏI�s�̌���
			CLayoutInt nLineCols = pcView->LineIndexToColmn( pcLayout, nLineLen );

			if( WCODE::IsLineDelimiter(pLine[nLineLen-1]) || nLineCols >= nWrapKetas ){
				// EOF�L���̕\��
				if( cEofType.IsDisp() ){
					_DispEOF(gr,pDispPos,pcView);
				}

				bEof = true;
			}
		}
	}

	// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
	pcView->GetTextDrawer().DispVerticalLines(
		gr,
		nYPrev,
		nYPrev + pcView->GetTextMetrics().GetHankakuDy(),
		CLayoutInt(0),
		CLayoutInt(-1)
	);

	return bEof;
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
			ExtTextOutOption(),
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
void CFigure_Eol::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView) const
{
	_DispEOL(gr, pDispPos, m_cEol, pcView);
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
			ExtTextOutOption(),
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
void _DispEOL(CGraphics& gr, DispPos* pDispPos, CEol cEol, const CEditView* pcView)
{
	RECT rcClip2;
	if(pcView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,2)){
		// 2003.08.17 ryoji ���s�����������Ȃ��悤��
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
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
			_DrawEOL(gr, rcEol, cEol, CTypeSupport(pcView,COLORIDX_EOL).IsFatFont(), GetTextColor(gr));

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
		sx = rcEol.left;						//X���[
		sy = rcEol.top + ( rcEol.Height() / 2);	//Y���S
		::MoveToEx( gr, sx + rcEol.Width(), sy - rcEol.Height() / 4, NULL );	//	���
		::LineTo(   gr, sx + rcEol.Width(), sy );			//	����
		::LineTo(   gr, sx, sy );					//	�擪��
		::LineTo(   gr, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	�擪���牺��
		::MoveToEx( gr, sx, sy, NULL);				//	�擪�֖߂�
		::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	�擪������
		if ( bBold ) {
			::MoveToEx( gr, sx + rcEol.Width() + 1, sy - rcEol.Height() / 4, NULL );	//	��ցi�E�ւ��炷�j
			++sy;
			::LineTo( gr, sx + rcEol.Width() + 1, sy );	//	�E�ցi�E�ɂЂƂ���Ă���j
			::LineTo(   gr, sx, sy );					//	�擪��
			::LineTo(   gr, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	�擪���牺��
			::MoveToEx( gr, sx, sy, NULL);				//	�擪�֖߂�
			::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	�擪������
		}
		break;
	case EOL_CR:	//	���������	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		sx = rcEol.left;
		sy = rcEol.top + ( rcEol.Height() / 2 );
		::MoveToEx( gr, sx + rcEol.Width(), sy, NULL );	//	�E��
		::LineTo(   gr, sx, sy );					//	�擪��
		::LineTo(   gr, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	�擪���牺��
		::MoveToEx( gr, sx, sy, NULL);				//	�擪�֖߂�
		::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	�擪������
		if ( bBold ) {
			++sy;
			::MoveToEx( gr, sx + rcEol.Width(), sy, NULL );	//	�E��
			::LineTo(   gr, sx, sy );					//	�擪��
			::LineTo(   gr, sx + rcEol.Height() / 4, sy + rcEol.Height() / 4 );	//	�擪���牺��
			::MoveToEx( gr, sx, sy, NULL);				//	�擪�֖߂�
			::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4 );	//	�擪������
		}
		break;
	case EOL_LF:	//	���������	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		sx = rcEol.left + ( rcEol.Width() / 2 );
		sy = rcEol.top + ( rcEol.Height() * 3 / 4 );
		::MoveToEx( gr, sx, rcEol.top + rcEol.Height() / 4 + 1, NULL );	//	���
		::LineTo(   gr, sx, sy );								//	�ォ�牺��
		::LineTo(   gr, sx - rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	���̂܂܍����
		::MoveToEx( gr, sx, sy, NULL);							//	���̐�[�ɖ߂�
		::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	�����ĉE���
		if( bBold ){
			++sx;
			::MoveToEx( gr, sx, rcEol.top + rcEol.Height() / 4 + 1, NULL );
			::LineTo(   gr, sx, sy );								//	�ォ�牺��
			::LineTo(   gr, sx - rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	���̂܂܍����
			::MoveToEx( gr, sx, sy, NULL);							//	���̐�[�ɖ߂�
			::LineTo(   gr, sx + rcEol.Height() / 4, sy - rcEol.Height() / 4);	//	�����ĉE���
		}
		break;
	}
}
