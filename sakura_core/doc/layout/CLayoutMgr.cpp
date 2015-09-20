/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, YAZAKI, genta, aroka
	Copyright (C) 2003, genta, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, D.S.Koba, Moca
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "doc/layout/CLayoutMgr.h"
#include "doc/layout/CLayout.h"/// 2002/2/10 aroka
#include "doc/CEditDoc.h"
#include "doc/CDocReader.h" // for _DEBUG
#include "doc/CDocEditor.h"
#include "doc/logic/CDocLine.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLineMgr.h"/// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/CMemory.h"/// 2002/2/10 aroka
#include "mem/CMemoryIterator.h" // 2006.07.29 genta
#include "view/CViewFont.h"
#include "view/CTextMetrics.h"
#include "basis/SakuraBasis.h"
#include "CSearchAgent.h"
#include "debug/CRunningTimer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CLayoutMgr::CLayoutMgr()
: m_getIndentOffset( &CLayoutMgr::getIndentOffset_Normal )	//	Oct. 1, 2002 genta	//	Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
{
	m_pcDocLineMgr = NULL;
	m_pTypeConfig = NULL;
	m_nMaxLineKetas = CKetaXInt(MAXLINEKETAS);
	m_nTabSpace = CKetaXInt(4);
	m_tsvInfo.m_nTsvMode = TSV_MODE_NONE;
	m_pszKinsokuHead_1.clear();						/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_1.clear();						/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();						/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK

	m_nTextWidth = CLayoutInt(0);			// �e�L�X�g�ő啝�̋L��		// 2009.08.28 nasukoji
	m_nTextWidthMaxLine = CLayoutInt(0);	// �ő啝�̃��C�A�E�g�s		// 2009.08.28 nasukoji

	Init();
}

CLayoutMgr::~CLayoutMgr()
{
	_Empty();

	m_pszKinsokuHead_1.clear();	/* �s���֑� */
	m_pszKinsokuTail_1.clear();	/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();	/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK
}


/*
||
|| �s�f�[�^�Ǘ��N���X�̃|�C���^�����������܂�
||
*/
void CLayoutMgr::Create( CEditDoc* pcEditDoc, CDocLineMgr* pcDocLineMgr )
{
	_Empty();
	Init();
	//	Jun. 20, 2003 genta EditDoc�ւ̃|�C���^�ǉ�
	m_pcEditDoc = pcEditDoc;
	m_pcDocLineMgr = pcDocLineMgr;
}

void CLayoutMgr::Init()
{
	m_pLayoutTop = NULL;
	m_pLayoutBot = NULL;
	m_nPrevReferLine = CLayoutInt(0);
	m_pLayoutPrevRefer = NULL;
	m_nLines = CLayoutInt(0);			/* �S�����s�� */
	m_nLineTypeBot = COLORIDX_DEFAULT;

	// EOF���C�A�E�g�ʒu�L��	//2006.10.07 Moca
	m_nEOFLine = CLayoutInt(-1);
	m_nEOFColumn = CLayoutInt(-1);
}



void CLayoutMgr::_Empty()
{
	CLayout* pLayout = m_pLayoutTop;
	while( pLayout ){
		CLayout* pLayoutNext = pLayout->GetNextLayout();
		delete pLayout;
		pLayout = pLayoutNext;
	}
}




/*! ���C�A�E�g���̕ύX
	@param bDoLayout [in] ���C�A�E�g���̍č쐬
	@param refType [in] �^�C�v�ʐݒ�
*/
void CLayoutMgr::SetLayoutInfo(
	bool				bDoLayout,
	bool				bBlockingHook,
	const STypeConfig&	refType,
	CKetaXInt			nTabSpace,
	int					nTsvMode,
	CKetaXInt			nMaxLineKetas,
	CLayoutXInt			nCharLayoutXPerKeta,
	const LOGFONT*		pLogfont
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::SetLayoutInfo" );

	assert_warning( (!bDoLayout && m_nMaxLineKetas == nMaxLineKetas) || bDoLayout );
	assert_warning( (!bDoLayout && m_nTabSpace == refType.m_nTabSpace) || bDoLayout );

	//�^�C�v�ʐݒ�
	m_pTypeConfig = &refType;
	m_nMaxLineKetas = nMaxLineKetas;
	m_nTabSpace = nTabSpace;
	int nTsvModeOld = m_tsvInfo.m_nTsvMode;
	m_tsvInfo.m_nTsvMode = nTsvMode;
	if (nTsvModeOld != nTsvMode && nTsvMode != TSV_MODE_NONE) {
		m_tsvInfo.CalcTabLength(this->m_pcDocLineMgr);
	}
	m_nSpacing = refType.m_nColumnSpace;
	if( nCharLayoutXPerKeta == -1 )
	{
		// View�������Ă�t�H���g���͌Â��A���傤���Ȃ��̂Ŏ����ō��
		HWND hwnd = NULL;
		HDC hdc = ::GetDC(hwnd);
		CViewFont viewFont(pLogfont);
		CTextMetrics temp;
		temp.Update(hdc, viewFont.GetFontHan(), refType.m_nLineSpace, refType.m_nColumnSpace);
		m_nCharLayoutXPerKeta = temp.GetHankakuWidth() + m_pTypeConfig->m_nColumnSpace;
		::ReleaseDC(hwnd, hdc);
	}else{
		m_nCharLayoutXPerKeta = nCharLayoutXPerKeta;
	}
	// �ő啶�����̌v�Z
	m_tsvInfo.m_nMaxCharLayoutX = WCODE::CalcPxWidthByFont(L'W');
	if (m_tsvInfo.m_nMaxCharLayoutX < m_nCharLayoutXPerKeta) {
		m_tsvInfo.m_nMaxCharLayoutX = m_nCharLayoutXPerKeta;
	}

	//	Oct. 1, 2002 genta �^�C�v�ɂ���ď����֐���ύX����
	//	���������Ă�����e�[�u���ɂ��ׂ�
	switch ( refType.m_nIndentLayout ){	/* �܂�Ԃ���2�s�ڈȍ~���������\�� */	//@@@ 2002.09.29 YAZAKI
	case 1:
		//	Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
		m_getIndentOffset = &CLayoutMgr::getIndentOffset_Tx2x;
		break;
	case 2:
		m_getIndentOffset = &CLayoutMgr::getIndentOffset_LeftSpace;
		break;
	default:
		m_getIndentOffset = &CLayoutMgr::getIndentOffset_Normal;
		break;
	}

	//��Ǔ_�Ԃ牺������	// 2009.08.07 ryoji
	//refType.m_szKinsokuKuto �� m_pszKinsokuKuto_1
	m_pszKinsokuKuto_1.clear();
	if(refType.m_bKinsokuKuto){	// 2009.08.06 ryoji m_bKinsokuKuto�ŐU�蕪����(Fix)
		for( const wchar_t* p = refType.m_szKinsokuKuto; *p; p++ ){
			m_pszKinsokuKuto_1.push_back_unique(*p);
		}
	}

	//�s���֑�����
	//refType.m_szKinsokuHead �� (��Ǔ_�ȊO) m_pszKinsokuHead_1
	m_pszKinsokuHead_1.clear();
	for( const wchar_t* p = refType.m_szKinsokuHead; *p; p++ ){
		if(m_pszKinsokuKuto_1.exist(*p)){
			continue;
		}
		else{
			m_pszKinsokuHead_1.push_back_unique(*p);
		}
	}

	//�s���֑�����
	//refType.m_szKinsokuTail �� m_pszKinsokuTail_1
	m_pszKinsokuTail_1.clear();
	for( const wchar_t* p = refType.m_szKinsokuTail; *p; p++ ){
		m_pszKinsokuTail_1.push_back_unique(*p);
	}

	//���C�A�E�g
	if( bDoLayout ){
		_DoLayout(bBlockingHook);
	}
}




/*!
	@brief �w�肳�ꂽ�����s�̃��C�A�E�g�����擾

	@param nLineNum [in] �����s�ԍ� (0�`)
*/
const CLayout* CLayoutMgr::SearchLineByLayoutY(
	CLayoutInt nLineLayout
) const
{
	CLayoutInt nLineNum = nLineLayout;

	CLayout*	pLayout;
	CLayoutInt	nCount;
	if( CLayoutInt(0) == m_nLines ){
		return NULL;
	}

	//	Mar. 19, 2003 Moca nLineNum�����̏ꍇ�̃`�F�b�N��ǉ�
	if( CLayoutInt(0) > nLineNum || nLineNum >= m_nLines ){
		if( CLayoutInt(0) > nLineNum ){
			DEBUG_TRACE( _T("CLayoutMgr::SearchLineByLayoutY() nLineNum = %d\n"), nLineNum );
		}
		return NULL;
	}
//	/*+++++++ �ᑬ�� +++++++++*/
//	if( nLineNum < (m_nLines / 2) ){
//		nCount = 0;
//		pLayout = m_pLayoutTop;
//		while( NULL != pLayout ){
//			if( nLineNum == nCount ){
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetNextLayout();
//			nCount++;
//		}
//	}else{
//		nCount = m_nLines - 1;
//		pLayout = m_pLayoutBot;
//		while( NULL != pLayout ){
//			if( nLineNum == nCount ){
//				m_pLayoutPrevRefer = pLayout;
//				m_nPrevReferLine = nLineNum;
//				return pLayout;
//			}
//			pLayout = pLayout->GetPrevLayout();
//			nCount--;
//		}
//	}


	/*+++++++�킸���ɍ�����+++++++*/
	// 2004.03.28 Moca m_pLayoutPrevRefer���ATop,Bot�̂ق����߂��ꍇ�́A������𗘗p����
	CLayoutInt nPrevToLineNumDiff = t_abs( m_nPrevReferLine - nLineNum );
	if( m_pLayoutPrevRefer == NULL
	  || nLineNum < nPrevToLineNumDiff
	  || m_nLines - nLineNum < nPrevToLineNumDiff
	){
		if( nLineNum < (m_nLines / 2) ){
			nCount = CLayoutInt(0);
			pLayout = m_pLayoutTop;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				nCount++;
			}
		}else{
			nCount = m_nLines - CLayoutInt(1);
			pLayout = m_pLayoutBot;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				nCount--;
			}
		}
	}else{
		if( nLineNum == m_nPrevReferLine ){
			return m_pLayoutPrevRefer;
		}
		else if( nLineNum > m_nPrevReferLine ){
			nCount = m_nPrevReferLine + CLayoutInt(1);
			pLayout = m_pLayoutPrevRefer->GetNextLayout();
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetNextLayout();
				nCount++;
			}
		}
		else{
			nCount = m_nPrevReferLine - CLayoutInt(1);
			pLayout = m_pLayoutPrevRefer->GetPrevLayout();
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->GetPrevLayout();
				nCount--;
			}
		}
	}
	return NULL;
}


//@@@ 2002.09.23 YAZAKI CLayout*���쐬����Ƃ���͕������āAInsertLineNext()�Ƌ��ʉ�
void CLayoutMgr::AddLineBottom( CLayout* pLayout )
{
	if(	CLayoutInt(0) == m_nLines ){
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
	}else{
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		m_pLayoutBot = pLayout;
	}
	pLayout->m_pNext = NULL;
	m_nLines++;
	return;
}

//@@@ 2002.09.23 YAZAKI CLayout*���쐬����Ƃ���͕������āAAddLineBottom()�Ƌ��ʉ�
CLayout* CLayoutMgr::InsertLineNext( CLayout* pLayoutPrev, CLayout* pLayout )
{
	CLayout* pLayoutNext;

	if(	CLayoutInt(0) == m_nLines ){
		/* �� */
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
		m_pLayoutTop->m_pNext = NULL;
	}
	else if( NULL == pLayoutPrev ){
		/* �擪�ɑ}�� */
		m_pLayoutTop->m_pPrev = pLayout;
		pLayout->m_pPrev = NULL;
		pLayout->m_pNext = m_pLayoutTop;
		m_pLayoutTop = pLayout;
	}else
	if( NULL == pLayoutPrev->GetNextLayout() ){
		/* �Ō�ɑ}�� */
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		pLayout->m_pNext = NULL;
		m_pLayoutBot = pLayout;
	}else{
		/* �r���ɑ}�� */
		pLayoutNext = pLayoutPrev->GetNextLayout();
		pLayoutPrev->m_pNext = pLayout;
		pLayoutNext->m_pPrev = pLayout;
		pLayout->m_pPrev = pLayoutPrev;
		pLayout->m_pNext = pLayoutNext;
	}
	m_nLines++;
	return pLayout;
}

/* CLayout���쐬����
	@@@ 2002.09.23 YAZAKI
	@date 2009.08.28 nasukoji	���C�A�E�g���������ɒǉ�
*/
CLayout* CLayoutMgr::CreateLayout(
	CDocLine*		pCDocLine,
	CLogicPoint		ptLogicPos,
	CLogicInt		nLength,
	EColorIndexType	nTypePrev,
	CLayoutInt		nIndent,
	CLayoutInt		nPosX,
	CLayoutColorInfo*	colorInfo
)
{
	CLayout* pLayout = new CLayout(
		pCDocLine,
		ptLogicPos,
		nLength,
		nTypePrev,
		nIndent,
		colorInfo
	);

	if( EOL_NONE == pCDocLine->GetEol() ){
		pLayout->m_cEol.SetType( EOL_NONE );/* ���s�R�[�h�̎�� */
	}else{
		if( pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL() >
			pCDocLine->GetLengthWithEOL() - pCDocLine->GetEol().GetLen()
		){
			pLayout->m_cEol = pCDocLine->GetEol();/* ���s�R�[�h�̎�� */
		}else{
			pLayout->m_cEol = EOL_NONE;/* ���s�R�[�h�̎�� */
		}
	}

	// 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I�����̂݃��C�A�E�g�����L������
	// �u�܂�Ԃ��Ȃ��v�ȊO�Ōv�Z���Ȃ��̂̓p�t�H�[�}���X�ቺ��h���ړI�Ȃ̂ŁA
	// �p�t�H�[�}���X�̒ቺ���C�ɂȂ�Ȃ����Ȃ�S�Ă̐܂�Ԃ����@�Ōv�Z����
	// �悤�ɂ��Ă��ǂ��Ǝv���B
	// �i���̏ꍇCLayoutMgr::CalculateTextWidth()�̌Ăяo���ӏ����`�F�b�N�j
	pLayout->SetLayoutWidth( ( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ) ? nPosX : CLayoutInt(0) );

	return pLayout;
}


/*
|| �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver0

	@date 2002/2/10 aroka CMemory�ύX
*/
const wchar_t* CLayoutMgr::GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen ) const //#####�������
{
	const CLayout* pLayout;
	if( NULL == ( pLayout = SearchLineByLayoutY( nLine )	) ){
		return NULL;
	}
	*pnLineLen = CLogicInt(pLayout->GetLengthWithEOL());
	return pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset();
}

/*!	�w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver1
	@date 2002/03/24 YAZAKI GetLineStr( int nLine, int* pnLineLen )�Ɠ�������ɕύX�B
*/
const wchar_t* CLayoutMgr::GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen, const CLayout** ppcLayoutDes ) const
{
	if( NULL == ( (*ppcLayoutDes) = SearchLineByLayoutY( nLine )	) ){
		return NULL;
	}
	*pnLineLen = (*ppcLayoutDes)->GetLengthWithEOL();
	return (*ppcLayoutDes)->m_pCDocLine->GetPtr() + (*ppcLayoutDes)->GetLogicOffset();
}

/*
|| �w�肳�ꂽ�ʒu�����C�A�E�g�s�̓r���̍s�����ǂ������ׂ�

	@date 2002/4/27 MIK
*/
bool CLayoutMgr::IsEndOfLine(
	const CLayoutPoint& ptLinePos
)
{
	const CLayout* pLayout;

	if( NULL == ( pLayout = SearchLineByLayoutY( ptLinePos.GetY2() )	) )
	{
		return false;
	}

	if( EOL_NONE == pLayout->GetLayoutEol().GetType() )
	{	/* ���̍s�ɉ��s�͂Ȃ� */
		/* ���̍s�̍Ōォ�H */
		if( ptLinePos.x == (Int)pLayout->GetLengthWithEOL() ) return true; //$$ �P�ʍ���
	}

	return false;
}

/*!	@brief �t�@�C�������̃��C�A�E�g�ʒu���擾����

	�t�@�C�������܂őI������ꍇ�ɐ��m�Ȉʒu����^���邽��

	�����̊֐��ł͕����s���烌�C�A�E�g�ʒu��ϊ�����K�v������C
	�����ɖ��ʂ��������߁C��p�֐����쐬
	
	@date 2006.07.29 genta
	@date 2006.10.01 Moca �����o�ŕێ�����悤�ɁB�f�[�^�ύX���ɂ́A_DoLayout/DoLayout_Range�Ŗ����ɂ���B
*/
void CLayoutMgr::GetEndLayoutPos(
	CLayoutPoint* ptLayoutEnd //[out]
)
{
	if( -1 != m_nEOFLine ){
		ptLayoutEnd->x = m_nEOFColumn;
		ptLayoutEnd->y = m_nEOFLine;
		return;
	}

	if( CLayoutInt(0) == m_nLines || m_pLayoutBot == NULL ){
		// �f�[�^����
		ptLayoutEnd->x = CLayoutInt(0);
		ptLayoutEnd->y = CLayoutInt(0);
		m_nEOFColumn = ptLayoutEnd->x;
		m_nEOFLine = ptLayoutEnd->y;
		return;
	}

	CLayout *btm = m_pLayoutBot;
	if( btm->m_cEol != EOL_NONE ){
		//	�����ɉ��s������
		ptLayoutEnd->Set(CLayoutInt(0), GetLineCount());
	}
	else {
		CMemoryIterator it = CreateCMemoryIterator(btm);
		while( !it.end() ){
			it.scanNext();
			it.addDelta();
		}
		ptLayoutEnd->Set(it.getColumn(), GetLineCount() - CLayoutInt(1));
		// [EOF]�̂ݐ܂�Ԃ��̂͂�߂�	// 2009.02.17 ryoji
		//// 2006.10.01 Moca Start [EOF]�݂̂̃��C�A�E�g�s�����������Ă����o�O���C��
		//if( GetMaxLineKetas() <= ptLayoutEnd->GetX2() ){
		//	ptLayoutEnd->SetX(CLayoutInt(0));
		//	ptLayoutEnd->y++;
		//}
		//// 2006.10.01 Moca End
	}
	m_nEOFColumn = ptLayoutEnd->x;
	m_nEOFLine = ptLayoutEnd->y;
}








/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */
CLayout* CLayoutMgr::DeleteLayoutAsLogical(
	CLayout*	pLayoutInThisArea,
	CLayoutInt	nLineOf_pLayoutInThisArea,
	CLogicInt	nLineFrom,
	CLogicInt	nLineTo,
	CLogicPoint	ptDelLogicalFrom,
	CLayoutInt*	pnDeleteLines
)
{
	CLayout* pLayout;
	CLayout* pLayoutWork;
	CLayout* pLayoutNext;

	*pnDeleteLines = CLayoutInt(0);
	if( CLayoutInt(0) == m_nLines){	/* �S�����s�� */
		return NULL;
	}
	if( NULL == pLayoutInThisArea ){
		return NULL;
	}

	// 1999.11.22
	m_pLayoutPrevRefer = pLayoutInThisArea->GetPrevLayout();
	m_nPrevReferLine = nLineOf_pLayoutInThisArea - CLayoutInt(1);


	/* �͈͓��擪�ɊY�����郌�C�A�E�g�����T�[�` */
	pLayoutWork = pLayoutInThisArea->GetPrevLayout();
	while( NULL != pLayoutWork && nLineFrom <= pLayoutWork->GetLogicLineNo()){
		pLayoutWork = pLayoutWork->GetPrevLayout();
	}



	if( NULL == pLayoutWork ){
		pLayout	= m_pLayoutTop;
	}else{
		pLayout = pLayoutWork->GetNextLayout();
	}
	while( NULL != pLayout ){
		if( pLayout->GetLogicLineNo() > nLineTo ){
			break;
		}
		pLayoutNext = pLayout->GetNextLayout();
		if( NULL == pLayoutWork ){
			/* �擪�s�̏��� */
			m_pLayoutTop = pLayout->GetNextLayout();
			if( NULL != pLayout->GetNextLayout() ){
				pLayout->m_pNext->m_pPrev = NULL;
			}
		}else{
			pLayoutWork->m_pNext = pLayout->GetNextLayout();
			if( NULL != pLayout->GetNextLayout() ){
				pLayout->m_pNext->m_pPrev = pLayoutWork;
			}
		}
//		if( m_pLayoutPrevRefer == pLayout ){
//			// 1999.12.22 �O�ɂ��炷�����ł悢�̂ł�
//			m_pLayoutPrevRefer = pLayout->GetPrevLayout();
//			--m_nPrevReferLine;
//		}

		if( ( ptDelLogicalFrom.GetY2() == pLayout->GetLogicLineNo() &&
			  ptDelLogicalFrom.GetX2() < pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL() ) ||
			( ptDelLogicalFrom.GetY2() < pLayout->GetLogicLineNo() )
		){
			(*pnDeleteLines)++;
		}

		if( m_pLayoutPrevRefer == pLayout ){
			DEBUG_TRACE( _T("�o�O�o�O\n") );
		}

		delete pLayout;

		m_nLines--;	/* �S�����s�� */
		if( NULL == pLayoutNext ){
			m_pLayoutBot = pLayoutWork;
		}
		pLayout = pLayoutNext;
	}
//	MYTRACE( _T("(*pnDeleteLines)=%d\n"), (*pnDeleteLines) );

	return pLayoutWork;
}




/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
void CLayoutMgr::ShiftLogicalLineNum( CLayout* pLayoutPrev, CLogicInt nShiftLines )
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::ShiftLogicalLineNum" );

	CLayout* pLayout;
	if( 0 == nShiftLines ){
		return;
	}
	if( NULL == pLayoutPrev ){
		pLayout = m_pLayoutTop;
	}else{
		pLayout = pLayoutPrev->GetNextLayout();
	}
	/* ���C�A�E�g���S�̂��X�V����(�ȂȁA�Ȃ��!!!) */
	while( NULL != pLayout ){
		pLayout->OffsetLogicLineNo(nShiftLines);	/* �Ή�����_���s�ԍ� */
		pLayout = pLayout->GetNextLayout();
	}
	return;
}


bool CLayoutMgr::ChangeLayoutParam(
	CKetaXInt	nTabSize,
	int			nTsvMode,
	CKetaXInt	nMaxLineKetas
)
{
	if( nTabSize < 1 || nTabSize > 64 ) { return false; }
	if( nMaxLineKetas < MINLINEKETAS || nMaxLineKetas > MAXLINEKETAS ){ return false; }

	m_nTabSpace = nTabSize;
	int nTsvModeOld = m_tsvInfo.m_nTsvMode;
	m_tsvInfo.m_nTsvMode = nTsvMode;
	if (nTsvModeOld != nTsvMode) {
		m_tsvInfo.CalcTabLength(this->m_pcDocLineMgr);
	}
	m_nMaxLineKetas = nMaxLineKetas;

	_DoLayout(true);

	return true;
}





/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
bool CLayoutMgr::WhereCurrentWord(
	CLayoutInt		nLineNum,
	CLogicInt		nIdx,
	CLayoutRange*	pSelect,		//!< [out]
	CNativeW*		pcmcmWord,		//!< [out]
	CNativeW*		pcmcmWordLeft	//!< [out]
)
{
	const CLayout* pLayout = SearchLineByLayoutY( nLineNum );
	if( NULL == pLayout ){
		return false;
	}

	// ���݈ʒu�̒P��͈̔͂𒲂ׂ� -> ���W�b�N�P��pSelect, pcmemWord, pcmemWordLeft
	CLogicInt nFromX;
	CLogicInt nToX;
	bool nRetCode = CSearchAgent(m_pcDocLineMgr).WhereCurrentWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + CLogicInt(nIdx),
		&nFromX,
		&nToX,
		pcmcmWord,
		pcmcmWordLeft
	);

	if( nRetCode ){
		/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
		CLayoutPoint ptFrom;
		LogicToLayout( CLogicPoint(nFromX, pLayout->GetLogicLineNo()), &ptFrom, nLineNum );
		pSelect->SetFrom(ptFrom);

		CLayoutPoint ptTo;
		LogicToLayout( CLogicPoint(nToX, pLayout->GetLogicLineNo()), &ptTo, nLineNum );
		pSelect->SetTo(ptTo);
	}
	return nRetCode;

}





/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
int CLayoutMgr::PrevOrNextWord(
	CLayoutInt		nLineNum,
	CLogicInt		nIdx,
	CLayoutPoint*	pptLayoutNew,
	BOOL			bLEFT,
	BOOL			bStopsBothEnds
)
{
	const CLayout*	pLayout = SearchLineByLayoutY( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}

	// ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ�
	CLogicInt	nPosNew;
	int			nRetCode = CSearchAgent(m_pcDocLineMgr).PrevOrNextWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nPosNew,
		bLEFT,
		bStopsBothEnds
	);

	if( nRetCode ){
		/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
		LogicToLayout(
			CLogicPoint(nPosNew,pLayout->GetLogicLineNo()),
			pptLayoutNew,
			nLineNum
		);
	}
	return nRetCode;
}





//! �P�ꌟ��
/*
	@retval 0 ������Ȃ�
*/
int CLayoutMgr::SearchWord(
	CLayoutInt				nLine,				//!< [in] �����J�n���C�A�E�g�s
	CLogicInt				nIdx,				//!< [in] �����J�n�f�[�^�ʒu
	ESearchDirection		eSearchDirection,	//!< [in] ��������
	CLayoutRange*			pMatchRange,		//!< [out] �}�b�`���C�A�E�g�͈�
	const CSearchStringPattern&	pattern
)
{
	int			nRetCode;
	const CLayout*	pLayout;
	pLayout = this->SearchLineByLayoutY( nLine );
	if( NULL == pLayout ){
		return FALSE;
	}

	// �P�ꌟ�� -> cLogicRange (�f�[�^�ʒu)
	CLogicRange cLogicRange;
	nRetCode = CSearchAgent(m_pcDocLineMgr).SearchWord(
		CLogicPoint(pLayout->GetLogicOffset() + nIdx, pLayout->GetLogicLineNo()),
		eSearchDirection,
		&cLogicRange, //pMatchRange,
		pattern
	);

	// �_���ʒu�����C�A�E�g�ʒu�ϊ�
	// cLogicRange -> pMatchRange
	if( nRetCode ){
		LogicToLayout(
			cLogicRange,
			pMatchRange
		);
	}
	return nRetCode;
}





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �P�ʂ̕ϊ�                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief �J�[�\���ʒu�ϊ� ���������C�A�E�g

	�����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	�����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)

	@date 2004.06.16 Moca �C���f���g�\���̍ۂ�TAB���܂ލs�̍��W����C��
	@date 2007.09.06 kobake �֐�����CaretPos_Phys2Log����LogicToLayout�ɕύX
*/
void CLayoutMgr::LogicToLayout(
	const CLogicPoint&	ptLogic,	//!< [in]  ���W�b�N�ʒu
	CLayoutPoint*		pptLayout,	//!< [out] ���C�A�E�g�ʒu
	CLayoutInt			nLineHint	//!< [in]  ���C�A�E�gY�l�̃q���g�B���߂�l�ɋ߂��l��n���ƍ����Ɍ����ł���B
)
{
	pptLayout->Clear();

	if(GetLineCount()==0)return; //�ϊ��s��

	// �T�[�`�J�n�n�_ -> pLayout, nCaretPosX, nCaretPosY
	CLayoutInt		nCaretPosX = CLayoutInt(0);
	CLayoutInt		nCaretPosY;
	const CLayout*	pLayout;
	// 2013.05.15 �q���g�A����Ȃ��̏����𓝍�
	{
		nLineHint = t_min(GetLineCount() - 1, nLineHint);
		nCaretPosY = t_max(CLayoutInt(ptLogic.y), nLineHint);

		// 2013.05.12 m_pLayoutPrevRefer������
		if( nCaretPosY <= m_nPrevReferLine && m_pLayoutPrevRefer
			&& m_pLayoutPrevRefer->GetLogicLineNo() <= ptLogic.y ){
			// �q���g��茻�݈ʒu�̂ق�����납�������炢�ŋ߂�
			nCaretPosY = CLayoutInt(ptLogic.y - m_pLayoutPrevRefer->GetLogicLineNo()) + m_nPrevReferLine;
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}else{
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}
		if( !pLayout ){
			pptLayout->SetY( m_nLines );
			return;
		}
		
		//���W�b�NY���ł�������ꍇ�́A��v����܂Ńf�N�������g (
		while(pLayout->GetLogicLineNo() > ptLogic.GetY2()){
			pLayout = pLayout->GetPrevLayout();
			nCaretPosY--;
		}

		//���W�b�NY��������Offset���s���߂��Ă���ꍇ�͖߂�
		if(pLayout->GetLogicLineNo() == ptLogic.GetY2()){
			while( pLayout->GetPrevLayout() && pLayout->GetPrevLayout()->GetLogicLineNo() == ptLogic.GetY2()
				&& ptLogic.x < pLayout->GetLogicOffset() ){
				pLayout = pLayout->GetPrevLayout();
				nCaretPosY--;
			}
		}
	}


	//	Layout���P����ɐi�߂Ȃ���ptLogic.y�������s�Ɉ�v����Layout��T��
	do{
		if( ptLogic.GetY2() == pLayout->GetLogicLineNo() ){
			// 2013.05.10 Moca ������
			const CLayout* pLayoutNext = pLayout->GetNextLayout();
			if( pLayoutNext && ptLogic.GetY2() ==pLayoutNext->GetLogicLineNo()
					&& pLayoutNext->GetLogicOffset() <= ptLogic.x ){
				nCaretPosY++;
				pLayout = pLayout->GetNextLayout();
				continue;
			}

			//	2004.06.16 Moca �C���f���g�\���̍ۂɈʒu�������(TAB�ʒu����ɂ��)
			//	TAB���𐳊m�Ɍv�Z����ɂ͓�������C���f���g���������Ă����K�v������D
			nCaretPosX = pLayout->GetIndent();
			const wchar_t*	pData;
			pData = pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset(); // 2002/2/10 aroka CMemory�ύX
			CLogicInt	nDataLen = (CLogicInt)pLayout->GetLengthWithEOL();

			CLogicInt i;
			for( i = CLogicInt(0); i < nDataLen; ++i ){
				if( pLayout->GetLogicOffset() + i >= ptLogic.x ){
					break;
				}

				//�������W�b�N�� -> nCharChars
				CLogicInt nCharChars = CNativeW::GetSizeOfChar( pData, nDataLen, i );
				if( nCharChars == 0 )
					nCharChars = CLogicInt(1);

				//�������C�A�E�g�� -> nCharKetas
				CLayoutInt nCharKetas;
				if( pData[i] ==	WCODE::TAB || ( pData[i] == L',' && m_tsvInfo.m_nTsvMode == TSV_MODE_CSV) ){
					nCharKetas = GetActualTsvSpace( nCaretPosX, pData[i] );
				}
				else{
					nCharKetas = GetLayoutXOfChar( pData, nDataLen, i );
				}
//				if( nCharKetas == 0 )				// �폜 �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
//					nCharKetas = CLayoutInt(1);

				//���C�A�E�g���Z
				nCaretPosX += nCharKetas;

				//���W�b�N���Z
				if( pData[i] ==	WCODE::TAB ){
					nCharChars = CLogicInt(1);
				}
				i += nCharChars - CLogicInt(1);
			}
			if( i < nDataLen ){
				//	ptLogic.x, ptLogic.y�����̍s�̒��Ɍ��������烋�[�v�ł��؂�
				break;
			}

			if( !pLayout->GetNextLayout() ){
				//	���Y�ʒu�ɒB���Ă��Ȃ��Ă��C���C�A�E�g�����Ȃ�f�[�^�����̃��C�A�E�g�ʒu��Ԃ��D
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + CLayoutInt(pLayout->GetLayoutEol().GetLen()>0?1:0);
				break;
			}

			if( ptLogic.y < pLayout->m_pNext->GetLogicLineNo() ){
				//	����Layout�����Y�����s���߂��Ă��܂��ꍇ�̓f�[�^�����̃��C�A�E�g�ʒu��Ԃ��D
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + CLayoutInt(pLayout->GetLayoutEol().GetLen()>0?1:0);
				break;
			}
		}
		if( ptLogic.GetY2() < pLayout->GetLogicLineNo() ){
			//	�ӂ��͂����ɂ͗��Ȃ��Ǝv����... (genta)
			//	Layout�̎w�������s���T���Ă���s������w���Ă�����ł��؂�
			break;
		}

		//	���̍s�֐i��
		nCaretPosY++;
		pLayout = pLayout->GetNextLayout();
	}
	while( pLayout );

	//	2004.06.16 Moca �C���f���g�\���̍ۂ̈ʒu����C��
	pptLayout->Set(
		pLayout ? nCaretPosX : CLayoutInt(0),
		nCaretPosY
	);
}

/*!
	@brief �J�[�\���ʒu�ϊ�  ���C�A�E�g������

	���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	�������ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)

	@date 2007.09.06 kobake �֐�����CaretPos_Log2Phys��LayoutToLogic�ɕύX
*/
void CLayoutMgr::LayoutToLogicEx(
	const CLayoutPoint&	ptLayout,	//!< [in]  ���C�A�E�g�ʒu
	CLogicPointEx*		pptLogic	//!< [out] ���W�b�N�ʒu
) const
{
	pptLogic->Set(CLogicInt(0), CLogicInt(0));
	pptLogic->ext = 0;
	pptLogic->haba = m_nCharLayoutXPerKeta;
	if( ptLayout.GetY2() > m_nLines ){
		//2007.10.11 kobake Y�l���Ԉ���Ă����̂ŏC��
		//pptLogic->Set(0, m_nLines);
		pptLogic->Set(CLogicInt(0), m_pcDocLineMgr->GetLineCount());
		return;
	}

	CLogicInt		nDataLen;
	const wchar_t*	pData;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �x�l�̌���                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	BOOL			bEOF = FALSE;
	CLayoutInt		nX;
	const CLayout*	pcLayout = SearchLineByLayoutY( ptLayout.GetY2() );
	if( !pcLayout ){
		if( 0 < ptLayout.y ){
			pcLayout = SearchLineByLayoutY( ptLayout.GetY2() - CLayoutInt(1) );
			if( NULL == pcLayout ){
				pptLogic->Set(CLogicInt(0), m_pcDocLineMgr->GetLineCount());
				return;
			}
			else{
				pData = GetLineStr( ptLayout.GetY2() - CLayoutInt(1), &nDataLen );
				if( WCODE::IsLineDelimiter(pData[nDataLen - 1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
					pptLogic->Set(CLogicInt(0), m_pcDocLineMgr->GetLineCount());
					return;
				}
				else{
					pptLogic->y = m_pcDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka CDocLineMgr�ύX
					bEOF = TRUE;
					// nX = CLayoutInt(MAXLINEKETAS);
					nX = pcLayout->GetIndent();
					goto checkloop;

				}
			}
		}
		//2007.10.11 kobake Y�l���Ԉ���Ă����̂ŏC��
		//pptLogic->Set(0, m_nLines);
		pptLogic->Set(CLogicInt(0), m_pcDocLineMgr->GetLineCount());
		return;
	}
	else{
		pptLogic->y = pcLayout->GetLogicLineNo();
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �w�l�̌���                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pData = GetLineStr( ptLayout.GetY2(), &nDataLen );
	nX = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);

checkloop:;
	CLogicInt	i;
	for( i = CLogicInt(0); i < nDataLen; ++i )
	{
		//�������W�b�N�� -> nCharChars
		CLogicInt	nCharChars;
		nCharChars = CNativeW::GetSizeOfChar( pData, nDataLen, i );
		if( nCharChars == 0 )
			nCharChars = CLogicInt(1);
		
		//�������C�A�E�g�� -> nCharKetas
		CLayoutInt	nCharKetas;
		if( pData[i] == WCODE::TAB || (pData[i] == L',' && m_tsvInfo.m_nTsvMode == TSV_MODE_CSV ) ){
			nCharKetas = GetActualTsvSpace( nX, pData[i] );
		}
		else{
			nCharKetas = GetLayoutXOfChar( pData, nDataLen, i );
		}
//		if( nCharKetas == 0 )				// �폜 �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
//			nCharKetas = CLayoutInt(1);

		//���C�A�E�g���Z
		if( nX + nCharKetas > ptLayout.GetX2() && !bEOF ){
			break;
		}
		nX += nCharKetas;

		//���W�b�N���Z
		if( pData[i] ==	WCODE::TAB ){
			nCharChars = CLogicInt(1);
		}
		i += nCharChars - CLogicInt(1);
	}
	i += pcLayout->GetLogicOffset();
	pptLogic->x = i;
	pptLogic->ext = ptLayout.GetX2() - nX;
	return;
}


void CLayoutMgr::LayoutToLogic( const CLayoutPoint& ptLayout, CLogicPoint* pptLogic ) const
{
	CLogicPointEx ptEx;
	LayoutToLogicEx( ptLayout, &ptEx );
	*pptLogic = ptEx;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �e�X�g�p�Ƀ��C�A�E�g�����_���v */
void CLayoutMgr::DUMP()
{
#ifdef _DEBUG
	const wchar_t* pData;
	CLogicInt nDataLen;
	MYTRACE( _T("------------------------\n") );
	MYTRACE( _T("m_nLines=%d\n"), m_nLines );
	MYTRACE( _T("m_pLayoutTop=%08lxh\n"), m_pLayoutTop );
	MYTRACE( _T("m_pLayoutBot=%08lxh\n"), m_pLayoutBot );
	MYTRACE( _T("m_nMaxLineKetas=%d\n"), m_nMaxLineKetas );

	MYTRACE( _T("m_nTabSpace=%d\n"), m_nTabSpace );
	CLayout* pLayout;
	CLayout* pLayoutNext;
	pLayout = m_pLayoutTop;
	while( NULL != pLayout ){
		pLayoutNext = pLayout->GetNextLayout();
		MYTRACE( _T("\t-------\n") );
		MYTRACE( _T("\tthis=%08lxh\n"), pLayout );
		MYTRACE( _T("\tm_pPrev =%08lxh\n"),		pLayout->GetPrevLayout() );
		MYTRACE( _T("\tm_pNext =%08lxh\n"),		pLayout->GetNextLayout() );
		MYTRACE( _T("\tm_nLinePhysical=%d\n"),	pLayout->GetLogicLineNo() );
		MYTRACE( _T("\tm_nOffset=%d\n"),		pLayout->GetLogicOffset() );
		MYTRACE( _T("\tm_nLength=%d\n"),		pLayout->GetLengthWithEOL() );
		MYTRACE( _T("\tm_enumEOLType =%ls\n"),	pLayout->GetLayoutEol().GetName() );
		MYTRACE( _T("\tm_nEOLLen =%d\n"),		pLayout->GetLayoutEol().GetLen() );
		MYTRACE( _T("\tm_nTypePrev=%d\n"),		pLayout->GetColorTypePrev() );
		pData = CDocReader(*m_pcDocLineMgr).GetLineStr( pLayout->GetLogicLineNo(), &nDataLen );
		MYTRACE( _T("\t[%ls]\n"), pData );
		pLayout = pLayoutNext;
	}
	MYTRACE( _T("------------------------\n") );
#endif
	return;
}



