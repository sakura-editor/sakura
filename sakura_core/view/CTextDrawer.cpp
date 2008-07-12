#include "stdafx.h"
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
#include "doc/CLayout.h"

const CTextArea* CTextDrawer::GetTextArea() const
{
	return &m_pEditView->GetTextArea();
}

using namespace std;

EColorIndexType CTextDrawer::_GetColorIdx(EColorIndexType nColorIdx,bool bSearchStringMode) const
{
	if(bSearchStringMode)return COLORIDX_SEARCH;						//�����q�b�g�F
	if(CTypeSupport(m_pEditView,nColorIdx).IsDisp())return nColorIdx;	//����F
	return COLORIDX_TEXT;												//�ʏ�F
}


/*
�e�L�X�g�\��
@@@ 2002.09.22 YAZAKI    const unsigned char* pData���Aconst char* pData�ɕύX
@@@ 2007.08.25 kobake �߂�l�� void �ɕύX�B���� x, y �� DispPos �ɕύX
*/
void CTextDrawer::DispText( HDC hdc, DispPos* pDispPos, const wchar_t* pData, int nLength ) const
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
	vector<int> vDxArray;
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

	if( rcClip.left < rcClip.right
	 && rcClip.left < pArea->GetAreaRight() && rcClip.right > pArea->GetAreaLeft()
	 && rcClip.top >= pArea->GetAreaTop()
	){

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
			ExtTextOutOption(),
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

void CTextDrawer::DispText( HDC hdc, int x, int y, const wchar_t* pData, int nLength ) const
{
	DispPos sPos(m_pEditView->GetTextMetrics().GetHankakuDx(),m_pEditView->GetTextMetrics().GetHankakuDy());
	sPos.InitDrawPos(CMyPoint(x,y));
	DispText(hdc,&sPos,pData,nLength);
}


/*!
EOF�L���̕`��
@date 2004.05.29 genta  MIK����̃A�h�o�C�X�ɂ��֐��ɂ����肾��
@date 2007.08.28 kobake ���� nCharWidth �폜
@date 2007.08.28 kobake ���� fuOptions �폜
@date 2007.08.30 kobake ���� EofColInfo �폜
*/
void CTextDrawer::DispEOF(
	HDC              hdc,      //!< [in] �`��Ώۂ�Device Context
	DispPos*         pDispPos  //!< [in] �\�����W
) const
{
	// �`��Ɏg���F���
	CTypeSupport cEofType(m_pEditView,COLORIDX_EOF);

	//�K�v�ȃC���^�[�t�F�[�X���擾
	const CTextMetrics* pMetrics=&m_pEditView->GetTextMetrics();
	const CTextArea* pArea=GetTextArea();

	//�萔
	const wchar_t	szEof[] = L"[EOF]";
	const int		nEofLen = _countof(szEof) - 1;

	//�N���b�s���O�̈���v�Z
	RECT rcClip;
	if(pArea->GenerateClipRect(&rcClip,*pDispPos,nEofLen))
	{
		//�F�ݒ�
		cEofType.SetColors(hdc);
		cEofType.SetFont(hdc);

		//�`��
		::ExtTextOutW_AnyBuild(
			hdc,
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
//                         ���s�`��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	May 23, 2000 genta
/*!
��ʕ`��⏕�֐�:
�s���̉��s�}�[�N�����s�R�[�h�ɂ���ď���������i���C���j

@note bBold��TRUE�̎��͉���1�h�b�g���炵�ďd�ˏ������s�����A
���܂葾�������Ȃ��B

@date 2001.12.21 YAZAKI ���s�L���̕`��������ύX�B�y���͂��̊֐����ō��悤�ɂ����B
						���̐擪���Asx, sy�ɂ��ĕ`�惋�[�`�����������B
*/
void CTextDrawer::_DrawEOL(
	HDC      hdc,     //!< Device Context Handle
	int      nPosX,   //!< �`����WX
	int      nPosY,   //!< �`����WY
	int      nWidth,  //!< �`��G���A�̃T�C�YX
	int      nHeight, //!< �`��G���A�̃T�C�YY
	CEol     cEol,    //!< �s���R�[�h���
	bool     bBold,   //!< TRUE: ����
	COLORREF pColor   //!< �F
) const
{
	int sx, sy;	//	���̐擪
	HANDLE	hPen;
	HPEN	hPenOld;
	hPen = ::CreatePen( PS_SOLID, 1, pColor );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );

	switch( cEol.GetType() ){
	case EOL_CRLF:	//	�������
		sx = nPosX;
		sy = nPosY + ( nHeight / 2);
		::MoveToEx( hdc, sx + nWidth, sy - nHeight / 4, NULL );	//	���
		::LineTo(   hdc, sx + nWidth, sy );			//	����
		::LineTo(   hdc, sx, sy );					//	�擪��
		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	�擪���牺��
		::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	�擪������
		if ( bBold ) {
			::MoveToEx( hdc, sx + nWidth + 1, sy - nHeight / 4, NULL );	//	��ցi�E�ւ��炷�j
			++sy;
			::LineTo( hdc, sx + nWidth + 1, sy );	//	�E�ցi�E�ɂЂƂ���Ă���j
			::LineTo(   hdc, sx, sy );					//	�擪��
			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	�擪���牺��
			::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	�擪������
		}
		break;
	case EOL_CR:	//	���������	// 2007.08.17 ryoji EOL_LF -> EOL_CR
		sx = nPosX;
		sy = nPosY + ( nHeight / 2 );
		::MoveToEx( hdc, sx + nWidth, sy, NULL );	//	�E��
		::LineTo(   hdc, sx, sy );					//	�擪��
		::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	�擪���牺��
		::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	�擪������
		if ( bBold ) {
			++sy;
			::MoveToEx( hdc, sx + nWidth, sy, NULL );	//	�E��
			::LineTo(   hdc, sx, sy );					//	�擪��
			::LineTo(   hdc, sx + nHeight / 4, sy + nHeight / 4 );	//	�擪���牺��
			::MoveToEx( hdc, sx, sy, NULL);				//	�擪�֖߂�
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4 );	//	�擪������
		}
		break;
	case EOL_LF:	//	���������	// 2007.08.17 ryoji EOL_CR -> EOL_LF
		sx = nPosX + ( nWidth / 2 );
		sy = nPosY + ( nHeight * 3 / 4 );
		::MoveToEx( hdc, sx, nPosY + nHeight / 4 + 1, NULL );	//	���
		::LineTo(   hdc, sx, sy );								//	�ォ�牺��
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	���̂܂܍����
		::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
		::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	�����ĉE���
		if( bBold ){
			++sx;
			::MoveToEx( hdc, sx, nPosY + nHeight / 4 + 1, NULL );
			::LineTo(   hdc, sx, sy );								//	�ォ�牺��
			::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4);	//	���̂܂܍����
			::MoveToEx( hdc, sx, sy, NULL);							//	���̐�[�ɖ߂�
			::LineTo(   hdc, sx + nHeight / 4, sy - nHeight / 4);	//	�����ĉE���
		}
		break;
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}


//2007.08.30 kobake �ǉ�
void CTextDrawer::DispEOL(HDC hdc, DispPos* pDispPos, CEol cEol, bool bSearchStringMode) const
{
	const CEditView* pView=m_pEditView;

	RECT rcClip2;
	if(pView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,2)){

		// �F����
		CTypeSupport cSupport(pView,_GetColorIdx(COLORIDX_CRLF,bSearchStringMode));
		cSupport.SetFont(hdc);
		cSupport.SetColors(hdc);

		// 2003.08.17 ryoji ���s�����������Ȃ��悤��
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			L"  ",
			2,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		// ���s�L���̕\��
		if( CTypeSupport(pView,COLORIDX_CRLF).IsDisp() ){
			int nPosX = pDispPos->GetDrawPos().x;
			int nPosY = pDispPos->GetDrawPos().y;
			
			// From Here 2003.08.17 ryoji ���s�����������Ȃ��悤��

			// ���[�W�����쐬�A�I���B
			HRGN hRgn;
			hRgn = ::CreateRectRgnIndirect(&rcClip2);
			::SelectClipRgn(hdc, hRgn);
			
			//@@@ 2001.12.21 YAZAKI
			_DrawEOL(
				hdc,
				nPosX + 1,
				nPosY,
				pView->GetTextMetrics().GetHankakuWidth(),
				pView->GetTextMetrics().GetHankakuHeight(),
				cEol,
				cSupport.IsFatFont(),
				cSupport.GetTextColor()
			);

			// ���[�W�����j��
			::SelectClipRgn(hdc, NULL);
			::DeleteObject(hRgn);
			
			// To Here 2003.08.17 ryoji ���s�����������Ȃ��悤��
		}
	}

	//�`��ʒu��i�߂�
	pDispPos->ForwardDrawCol(1);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �^�u�`��                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispTab( HDC hdc, DispPos* pDispPos, EColorIndexType nColorIdx ) const
{
	DispPos& sPos=*pDispPos;
	const CEditView* pView=m_pEditView;

	//�萔
	static const wchar_t* pszSPACES = L"        ";

	//�K�v�ȃC���^�[�t�F�[�X
	const CTextMetrics* pMetrics=&m_pEditView->GetTextMetrics();
	const CTextArea* pArea=GetTextArea();
	STypeConfig* TypeDataPtr = &pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	int nLineHeight = pMetrics->GetHankakuDy();
	int nCharWidth = pMetrics->GetHankakuDx();


	CTypeSupport cTabType(pView,COLORIDX_TAB);

	//	Sep. 22, 2002 genta ���ʎ��̂����肾��
	//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
	int tabDispWidth = (Int)pView->m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( sPos.GetDrawCol() );

	// �^�u�L����\������
	RECT rcClip2;
	rcClip2.left = sPos.GetDrawPos().x;
	rcClip2.right = rcClip2.left + nCharWidth * tabDispWidth;
	if( rcClip2.left < pArea->GetAreaLeft() ){
		rcClip2.left = pArea->GetAreaLeft();
	}
	if( rcClip2.left < rcClip2.right &&
		rcClip2.left < pArea->GetAreaRight() && rcClip2.right > pArea->GetAreaLeft() ){

		rcClip2.top = sPos.GetDrawPos().y;
		rcClip2.bottom = sPos.GetDrawPos().y + nLineHeight;
		// TAB��\�����邩�H
		if( cTabType.IsDisp() && !TypeDataPtr->m_bTabArrow ){	//�^�u�ʏ�\��	//@@@ 2003.03.26 MIK

			// �T�|�[�g�N���X
			CTypeSupport cSupport(pView,nColorIdx);

			// �t�H���g��I��
			cSupport.SetFont(hdc);
			cSupport.SetColors(hdc);

			//@@@ 2001.03.16 by MIK
			::ExtTextOutW_AnyBuild(
				hdc,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption(),
				&rcClip2,
				TypeDataPtr->m_szTabViewString,
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);
		}else{
			CTypeSupport cSearchType(pView,COLORIDX_SEARCH);
			if( nColorIdx == COLORIDX_SEARCH ){
				cSearchType.SetBkColor(hdc);
			}
			::ExtTextOutW_AnyBuild(
				hdc, sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption(),
				&rcClip2,
				pszSPACES,
				tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);
			cSearchType.RewindColors(hdc);
			
			//�^�u���\��	//@@@ 2003.03.26 MIK
			if( cTabType.IsDisp()
			 && TypeDataPtr->m_bTabArrow
			 && rcClip2.left <= sPos.GetDrawPos().x ) // Apr. 1, 2003 MIK �s�ԍ��Əd�Ȃ�
			{
				_DrawTabArrow(
					hdc,
					sPos.GetDrawPos().x,
					sPos.GetDrawPos().y,
					pMetrics->GetHankakuWidth(),
					pMetrics->GetHankakuHeight(),
					cTabType.IsFatFont(),
					cTabType.GetTextColor()
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
void CTextDrawer::_DrawTabArrow(
	HDC hdc,
	int nPosX,   //�s�N�Z��X
	int nPosY,   //�s�N�Z��Y
	int nWidth,  //�s�N�Z��W
	int nHeight, //�s�N�Z��H
	int bBold,
	COLORREF pColor
) const
{
	HPEN hPen    = ::CreatePen( PS_SOLID, 1, pColor );
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );

	nWidth--;

	//	���̐擪
	int sx = nPosX + nWidth;
	int sy = nPosY + ( nHeight / 2 );

	::MoveToEx( hdc, sx - nWidth, sy, NULL );				//	����
	::LineTo(   hdc, sx, sy );								//	�Ō��
	::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	�Ōォ�牺��
	::MoveToEx( hdc, sx, sy, NULL);							//	�Ō�֖߂�
	::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	�Ōォ����
	if ( bBold ) {
		++sy;
		::MoveToEx( hdc, sx - nWidth, sy, NULL );				//	����
		::LineTo(   hdc, sx, sy );								//	�Ō��
		::LineTo(   hdc, sx - nHeight / 4, sy + nHeight / 4 );	//	�Ōォ�牺��
		::MoveToEx( hdc, sx, sy, NULL);							//	�Ō�֖߂�
		::LineTo(   hdc, sx - nHeight / 4, sy - nHeight / 4 );	//	�Ōォ����
	}

	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �X�y�[�X�`��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispZenkakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode ) const
{
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	RECT rcClip2;
	if(GetTextArea()->GenerateClipRect(&rcClip2,*pDispPos,2))
	{
		// �F����
		CTypeSupport cSupport(m_pEditView,_GetColorIdx(COLORIDX_ZENSPACE,bSearchStringMode));
		cSupport.SetFont(hdc);
		cSupport.SetColors(hdc);

		//�`�敶����
		const wchar_t* szZenSpace =
			CTypeSupport(m_pEditView,COLORIDX_ZENSPACE).IsDisp()?L"��":L"�@";

		//�`��
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			szZenSpace,
			wcslen(szZenSpace),
			m_pEditView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(2);
}

void CTextDrawer::DispHankakuSpace( HDC hdc, DispPos* pDispPos, bool bSearchStringMode) const
{
	//�N���b�s���O��`���v�Z�B��ʊO�Ȃ�`�悵�Ȃ�
	CMyRect rcClip;
	if(m_pEditView->GetTextArea().GenerateClipRect(&rcClip,*pDispPos,1))
	{
		// �F����
		CTypeSupport cSupport(m_pEditView,_GetColorIdx(COLORIDX_SPACE,bSearchStringMode));
		cSupport.SetFont(hdc);
		cSupport.SetColors(hdc);
		
		//������"o"�̉��������o��
		CMyRect rcClipBottom=rcClip;
		rcClipBottom.top=rcClip.top+rcClip.Height()/2;
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClipBottom,
			L"o",
			1,
			m_pEditView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		
		//�㔼���͕��ʂ̋󔒂ŏo�́i"o"�̏㔼���������j
		CMyRect rcClipTop=rcClip;
		rcClipTop.bottom=rcClip.top+rcClip.Height()/2;
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClipTop,
			L" ",
			1,
			m_pEditView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//�ʒu�i�߂�
	pDispPos->ForwardDrawCol(1);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �܂�Ԃ��`��                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispWrap(HDC hdc, DispPos* pDispPos) const
{
	const CEditView* pView=m_pEditView;

	RECT rcClip2;
	if(pView->GetTextArea().GenerateClipRect(&rcClip2,*pDispPos,1))
	{
		//�T�|�[�g�N���X
		CTypeSupport cWrapType(pView,COLORIDX_WRAP);

		//�`�敶����ƐF�̌���
		const wchar_t* szText;
		if( cWrapType.IsDisp() )
		{
			szText = L"<";
			cWrapType.SetFont(hdc);
			cWrapType.SetColors(hdc);
		}
		else
		{
			szText = L" ";
		}

		//�`��
		::ExtTextOutW_AnyBuild(
			hdc,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y,
			ExtTextOutOption(),
			&rcClip2,
			szText,
			wcslen(szText),
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ��(����)�s�`��                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! ��s��`��BEOF��`�悵���ꍇ��true��Ԃ��B
bool CTextDrawer::DispEmptyLine(HDC hdc, DispPos* pDispPos) const
{
	bool bEof=false;

	const CEditView* pView=m_pEditView;
	CTypeSupport cEofType(pView,COLORIDX_EOF);
	CTypeSupport cTextType(pView,COLORIDX_TEXT);

	const CLayoutInt nWrapKetas = pView->m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas();

	int nYPrev = pDispPos->GetDrawPos().y;
	
	if( m_pEditView->IsBkBitmap() ){
	}else{
		// �w�i�`��
		RECT rcClip;
		pView->GetTextArea().GenerateClipRectLine(&rcClip,*pDispPos);
		cTextType.FillBack(hdc,rcClip);
	}

	// EOF�L���̕\��
	CLayoutInt nCount = pView->m_pcEditDoc->m_cLayoutMgr.GetLineCount();

	// �h�L�������g����(nCount==0)�B������1�s��(pDispPos->GetLayoutLineRef() == 0)�B�\�����1�s��(m_nViewTopLine==0)
	if( nCount == 0 && pView->GetTextArea().GetViewTopLine() == 0 && pDispPos->GetLayoutLineRef() == 0 ){
		// EOF�L���̕\��
		if( cEofType.IsDisp() ){
			DispEOF(hdc,pDispPos);
		}

		//�`��Y�ʒu�i�߂�
		pDispPos->ForwardDrawLine(1);
		bEof = true;
	}
	else{
		//�ŏI�s�̎��̍s
		if( nCount > 0 && pDispPos->GetLayoutLineRef() == nCount ){
			//�ŏI�s�̎擾
			const wchar_t*	pLine;
			CLogicInt		nLineLen;
			const CLayout*	pcLayout;
			pLine = pView->m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCount - CLayoutInt(1), &nLineLen, &pcLayout );
			
			//�ŏI�s�̌���
			CLayoutInt nLineCols = pView->LineIndexToColmn( pcLayout, nLineLen );

			if( WCODE::IsLineDelimiter(pLine[nLineLen-1]) || nLineCols >= nWrapKetas ){
				// EOF�L���̕\��
				if( cEofType.IsDisp() ){
					DispEOF(hdc,pDispPos);
				}

				//�`��Y�ʒu�i�߂�
				pDispPos->ForwardDrawLine(1);
				bEof = true;
			}
		}
	}

	// 2006.04.29 Moca �I�������̂��ߏc��������ǉ�
	DispVerticalLines( hdc, nYPrev, nYPrev + pView->GetTextMetrics().GetHankakuDy(),  CLayoutInt(0), CLayoutInt(-1) );

	return bEof;
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
	HDC			hdc,		//!< ��悷��E�B���h�E��DC
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
	const int nPosXLeft   = max( pView->GetTextArea().GetAreaLeft() + (Int)(nLeftCol  - pView->GetTextArea().GetViewLeftCol()) * nCharDx, pView->GetTextArea().GetAreaLeft() );
	const int nPosXRight  = min( pView->GetTextArea().GetAreaLeft() + (Int)(nRightCol - pView->GetTextArea().GetViewLeftCol()) * nCharDx, pView->GetTextArea().GetAreaRight() );
	const int nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	bool bOddLine = ((((nLineHeight % 2) ? (Int)pView->GetTextArea().GetViewTopLine() : 0) + pView->GetTextArea().GetAreaTop() + nTop) % 2 == 1);

	// ����
	const bool bBold = cVertType.IsFatFont();
	// �h�b�g��(����������]�p/�e�X�g�p)
	const bool bDot = cVertType.HasUnderLine();
	const bool bExorPen = ( cVertType.GetTextColor() == cTextType.GetBackColor() );
	HPEN hPen;
	int nROP_Old = 0;
	if( bExorPen ){
		hPen = ::CreatePen( PS_SOLID, 0, cVertType.GetBackColor() );
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
	}else{
		hPen = ::CreatePen( PS_SOLID, 0, cVertType.GetTextColor() );
	}
	HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );

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
							::MoveToEx( hdc, nPosX, y, NULL );
							::LineTo( hdc, nPosX, y + 1 );
						}
						if( bBold && nPosXLeft <= nPosXBold ){
							::MoveToEx( hdc, nPosXBold, y, NULL );
							::LineTo( hdc, nPosXBold, y + 1 );
						}
					}
				}else{
					if( nPosX < nPosXRight ){
						::MoveToEx( hdc, nPosX, nTop, NULL );
						::LineTo( hdc, nPosX, nBottom );
					}
					if( bBold && nPosXLeft <= nPosXBold ){
						::MoveToEx( hdc, nPosXBold, nTop, NULL );
						::LineTo( hdc, nPosXBold, nBottom );
					}
				}
			}
		}
	}
	if( bExorPen ){
		::SetROP2( hdc, nROP_Old );
	}
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          �s�ԍ�                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CTextDrawer::DispLineNumber(
	HDC						hdc,
	const CLayout*			pcLayout,
	int						nLineNum,
	int						y
) const
{
	const CEditView* pView=m_pEditView;
	const STypeConfig* pTypes=&pView->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	int				nLineHeight = pView->GetTextMetrics().GetHankakuDy();
	int				nCharWidth = pView->GetTextMetrics().GetHankakuDx();
	//	Sep. 23, 2002 genta ���ʎ��̂����肾��
	int				nLineNumAreaWidth = pView->GetTextArea().m_nViewAlignLeftCols * nCharWidth;

	CTypeSupport cTextType(pView,COLORIDX_TEXT);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                     nColorIndex������                       //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	EColorIndexType nColorIndex = COLORIDX_GYOU;	/* �s�ԍ� */
	const CDocLine*	pCDocLine;
	if( pcLayout ){
		pCDocLine = pcLayout->GetDocLineRef();

		if( pView->m_pcEditDoc->m_cDocEditor.IsModified() && CModifyVisitor().IsLineModified(pCDocLine) ){		/* �ύX�t���O */
			if( CTypeSupport(pView,COLORIDX_GYOU_MOD).IsDisp() )	// 2006.12.12 ryoji
				nColorIndex = COLORIDX_GYOU_MOD;	/* �s�ԍ��i�ύX�s�j */
		}
	}

	EDiffMark type = CDiffLineGetter(pCDocLine).GetLineDiffMark();
	{
		//DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
		if( type )
		{
			switch( type )
			{
			case MARK_DIFF_APPEND:	//�ǉ�
				if( CTypeSupport(pView,COLORIDX_DIFF_APPEND).IsDisp() )
					nColorIndex = COLORIDX_DIFF_APPEND;
				break;
			case MARK_DIFF_CHANGE:	//�ύX
				if( CTypeSupport(pView,COLORIDX_DIFF_CHANGE).IsDisp() )
					nColorIndex = COLORIDX_DIFF_CHANGE;
				break;
			case MARK_DIFF_DELETE:	//�폜
			case MARK_DIFF_DEL_EX:	//�폜
				if( CTypeSupport(pView,COLORIDX_DIFF_DELETE).IsDisp() )
					nColorIndex = COLORIDX_DIFF_DELETE;
				break;
			}
		}
	}

	// 02/10/16 ai
	// �u�b�N�}�[�N�̕\��
	if(CBookmarkGetter(pCDocLine).IsBookmarked()){
		if( CTypeSupport(pView,COLORIDX_MARK).IsDisp() ) {
			nColorIndex = COLORIDX_MARK;
		}
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//             ���肳�ꂽnColorIndex���g���ĕ`��               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	CTypeSupport cColorType(pView,nColorIndex);
	CTypeSupport cMarkType(pView,COLORIDX_MARK);

	if( CTypeSupport(pView,COLORIDX_GYOU).IsDisp() ){ /* �s�ԍ��\���^��\�� */
		wchar_t szLineNum[18];

		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		if( pTypes->m_bLineNumIsCRLF ){
			/* �_���s�ԍ��\�����[�h */
			if( NULL == pcLayout || 0 != pcLayout->GetLogicOffset() ){
				wcscpy( szLineNum, L" " );
			}else{
				_itow( pcLayout->GetLogicLineNo() + 1, szLineNum, 10 );	/* �Ή�����_���s�ԍ� */
			}
		}else{
			/* �����s�i���C�A�E�g�s�j�ԍ��\�����[�h */
			_itow( nLineNum + 1, szLineNum, 10 );
		}

		int nLineCols = wcslen( szLineNum );

		//�F�A�t�H���g
		cTextType.SetBkColor(hdc);    //�w�i�F�F�e�L�X�g�̔w�i�F //	Sep. 23, 2002 �]�����e�L�X�g�̔w�i�F�ɂ���
		cColorType.SetTextColor(hdc); //�e�L�X�g�F�s�ԍ��̐F
		cColorType.SetFont(hdc);      //�t�H���g�F�s�ԍ��̃t�H���g

		// �]���𖄂߂�
		RECT	rcClip;
		rcClip.left   = nLineNumAreaWidth;
		rcClip.right  = pView->GetTextArea().GetAreaLeft();
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOutW_AnyBuild(
			hdc,
			rcClip.left,
			y,
			ExtTextOutOption(),
			&rcClip,
			L" ",
			1,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		
		//	Sep. 23, 2002 �]�����e�L�X�g�̔w�i�F�ɂ��邽�߁C�w�i�F�̐ݒ���ړ�
		SetBkColor( hdc, cColorType.GetBackColor() );		/* �s�ԍ��w�i�̐F */

		int drawNumTop = (pView->GetTextArea().m_nViewAlignLeftCols - nLineCols - 1) * ( nCharWidth );

		/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		if( 2 == pTypes->m_nLineTermType ){
			//	Sep. 22, 2002 genta
			szLineNum[ nLineCols ] = pTypes->m_cLineTermChar;
			szLineNum[ ++nLineCols ] = '\0';
		}
		rcClip.left = 0;

		//	Sep. 23, 2002 genta
		rcClip.right = nLineNumAreaWidth;
		rcClip.top = y;
		rcClip.bottom = y + nLineHeight;
		::ExtTextOutW_AnyBuild( hdc,
			drawNumTop,
			y,
			ExtTextOutOption(),
			&rcClip,
			szLineNum,
			nLineCols,
			pView->GetTextMetrics().GetDxArray_AllHankaku()
		);

		/* �s�ԍ���؂� 0=�Ȃ� 1=�c�� 2=�C�� */
		if( 1 == pTypes->m_nLineTermType ){
			cColorType.SetSolidPen(hdc,0);
			::MoveToEx( hdc, nLineNumAreaWidth - 2, y, NULL );
			::LineTo( hdc, nLineNumAreaWidth - 2, y + nLineHeight );
			cColorType.RewindPen(hdc);
		}
		cColorType.RewindColors(hdc);
		cTextType.RewindColors(hdc);
		cColorType.RewindFont(hdc);
	}
	else{
		RECT	rcClip;
		// �s�ԍ��G���A�̔w�i�`��
		rcClip.left   = 0;
		rcClip.right  = pView->GetTextArea().GetAreaLeft() - GetDllShareData().m_Common.m_sWindow.m_nLineNumRightSpace; //	Sep. 23 ,2002 genta �]���̓e�L�X�g�F�̂܂܎c��
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		cColorType.FillBack(hdc,rcClip);
		
		// Mar. 5, 2003, Moca
		// �s�ԍ��ƃe�L�X�g�̌��Ԃ̕`��
		rcClip.left   = rcClip.right;
		rcClip.right  = pView->GetTextArea().GetAreaLeft();
		rcClip.top    = y;
		rcClip.bottom = y + nLineHeight;
		cTextType.FillBack(hdc,rcClip);
	}

	// 2001.12.03 hor
	/* �Ƃ肠�����u�b�N�}�[�N�ɏc�� */
	if(CBookmarkGetter(pCDocLine).IsBookmarked() && !cMarkType.IsDisp() )
	{
		cColorType.SetSolidPen(hdc,2);
		::MoveToEx( hdc, 1, y, NULL );
		::LineTo( hdc, 1, y + nLineHeight );
		cColorType.RewindPen(hdc);
	}

	if( type )	//DIFF�����}�[�N�\��	//@@@ 2002.05.25 MIK
	{
		int	cy = y + nLineHeight / 2;

		cColorType.SetSolidPen(hdc,1);

		switch( type )
		{
		case MARK_DIFF_APPEND:	//�ǉ�
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 6, cy );
			::MoveToEx( hdc, 4, cy - 2, NULL );
			::LineTo  ( hdc, 4, cy + 3 );
			break;

		case MARK_DIFF_CHANGE:	//�ύX
			::MoveToEx( hdc, 3, cy - 4, NULL );
			::LineTo  ( hdc, 3, cy );
			::MoveToEx( hdc, 3, cy + 2, NULL );
			::LineTo  ( hdc, 3, cy + 3 );
			break;

		case MARK_DIFF_DELETE:	//�폜
			cy -= 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 5, cy );
			::LineTo  ( hdc, 3, cy + 2 );
			::LineTo  ( hdc, 3, cy );
			::LineTo  ( hdc, 7, cy + 4 );
			break;
		
		case MARK_DIFF_DEL_EX:	//�폜(EOF)
			cy += 3;
			::MoveToEx( hdc, 3, cy, NULL );
			::LineTo  ( hdc, 5, cy );
			::LineTo  ( hdc, 3, cy - 2 );
			::LineTo  ( hdc, 3, cy );
			::LineTo  ( hdc, 7, cy - 4 );
			break;
		}

		cColorType.RewindPen(hdc);
	}
}
