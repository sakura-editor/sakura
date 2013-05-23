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
#include "doc/CDocReader.h" // for _DEBUG
#include "doc/logic/CDocLine.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLineMgr.h"/// 2002/2/10 aroka
#include "charset/charcode.h"
#include "mem/CMemory.h"/// 2002/2/10 aroka
#include "mem/CMemoryIterator.h" // 2006.07.29 genta
#include "basis/SakuraBasis.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "CSearchAgent.h"
#include "debug/CRunningTimer.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CLayoutMgr::CLayoutMgr()
: m_getIndentOffset( &CLayoutMgr::getIndentOffset_Normal )	//	Oct. 1, 2002 genta	//	Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
{
	m_pcDocLineMgr = NULL;
	m_sTypeConfig.m_bWordWrap = true;				// �p�����[�h���b�v������
	m_sTypeConfig.m_nTabSpace = CLayoutInt(8);		/* TAB�����X�y�[�X */
	m_sTypeConfig.m_nStringType = 0;				/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
	m_sTypeConfig.m_bKinsokuHead = false;			// �s���֑�				//@@@ 2002.04.08 MIK
	m_sTypeConfig.m_bKinsokuTail = false;			// �s���֑�				//@@@ 2002.04.08 MIK
	m_sTypeConfig.m_bKinsokuRet  = false;			// ���s�������Ԃ牺����	//@@@ 2002.04.13 MIK
	m_sTypeConfig.m_bKinsokuKuto = false;			// ��Ǔ_���Ԃ牺����	//@@@ 2002.04.17 MIK
	m_pszKinsokuHead_1.clear();						/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_1.clear();						/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();						/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK

	// 2005.11.21 Moca �F�����t���O�������o�Ŏ���
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = false; 
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;

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
	@param bDoRayout [in] ���C�A�E�g���̍č쐬
	@param refType [in] �^�C�v�ʐݒ�
*/
void CLayoutMgr::SetLayoutInfo(
	bool				bDoRayout,
	const STypeConfig&	refType
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::SetLayoutInfo" );

	//�^�C�v�ʐݒ�
	m_sTypeConfig = refType;
	
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
	if(m_sTypeConfig.m_bKinsokuKuto){	// 2009.08.06 ryoji m_bKinsokuKuto�ŐU�蕪����(Fix)
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
	if( bDoRayout ){
		_DoLayout();
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
	CLayoutInt		nPosX
)
{
	CLayout* pLayout = new CLayout(
		pCDocLine,
		ptLogicPos,
		nLength,
		nTypePrev,
		nIndent
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
		CMemoryIterator it( btm, GetTabSpace() );
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


/*!	�s�������폜

	@date 2002/03/24 YAZAKI bUndo�폜
	@date 2009/08/28 nasukoji	�e�L�X�g�ő啝�̎Z�o�ɑΉ�
*/
void CLayoutMgr::DeleteData_CLayoutMgr(
	CLayoutInt	nLineNum,
	CLogicInt	nDelPos,
	CLogicInt	nDelLen,
	CLayoutInt*	pnModifyLayoutLinesOld,
	CLayoutInt*	pnModifyLayoutLinesNew,
	CLayoutInt*	pnDeleteLayoutLines,
	CNativeW*	cmemDeleted				//!< [out] �폜���ꂽ�f�[�^
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CLayoutMgr::DeleteData_CLayoutMgr" );
#endif
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLayout*		pLayout;
	CLayout*		pLayoutPrev;
	CLayout*		pLayoutWork;
	CLogicInt		nModLineOldFrom;	/* �e���̂������ύX�O�̍s(from) */
	CLogicInt		nModLineOldTo;		/* �e���̂������ύX�O�̍s(to) */
	CLogicInt		nDelLineOldFrom;	/* �폜���ꂽ�ύX�O�_���s(from) */
	CLogicInt		nDelLineOldNum;		/* �폜���ꂽ�s�� */
	CLogicInt		nRowNum;
	CLogicInt		nDelStartLogicalLine;
	CLogicInt		nDelStartLogicalPos;
	EColorIndexType	nCurrentLineType;
	CLayoutInt		nLineWork;

	/* ���ݍs�̃f�[�^���擾 */
	pLine = GetLineStr( nLineNum, &nLineLen );
	if( NULL == pLine ){
		return;
	}
	pLayout = m_pLayoutPrevRefer;
	nDelStartLogicalLine = pLayout->GetLogicLineNo();
	nDelStartLogicalPos  = nDelPos + pLayout->GetLogicOffset();

	pLayoutWork = pLayout;
	nLineWork = nLineNum;
	while( 0 != pLayoutWork->GetLogicOffset() ){
		pLayoutWork = pLayoutWork->GetPrevLayout();
		--nLineWork;
	}
	nCurrentLineType = pLayoutWork->GetColorTypePrev();

	/* �e�L�X�g�̃f�[�^���폜 */
	CDocEditAgent(m_pcDocLineMgr).DeleteData_CDocLineMgr(
		nDelStartLogicalLine,
		nDelStartLogicalPos,
		nDelLen,
		&nModLineOldFrom,
		&nModLineOldTo,
		&nDelLineOldFrom,
		&nDelLineOldNum,
		cmemDeleted
	);

//	DUMP();

	/*--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---*/
	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
	/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */
	CLayoutInt		nAllLinesOld = m_nLines;
	pLayoutPrev = DeleteLayoutAsLogical(
		pLayoutWork,
		nLineWork,
		nModLineOldFrom,
		nModLineOldTo,
		CLogicPoint(nDelStartLogicalPos, nDelStartLogicalLine),
		pnModifyLayoutLinesOld
	);

	/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
	/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
	/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
	ShiftLogicalLineNum( pLayoutPrev, nDelLineOldNum * (-1) );

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->GetLogicLineNo();
		}
	}else{
		if( NULL == pLayoutPrev->GetNextLayout() ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->GetLogicLineNo() - CLogicInt(1);
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->GetLogicLineNo() -
				pLayoutPrev->GetLogicLineNo() - CLogicInt(1);
		}
	}

	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�̈�����ݒ�
	CalTextWidthArg ctwArg;
	ctwArg.nLineFrom    = nLineNum;				// �ҏW�J�n�s
	ctwArg.nColumnFrom  = CLayoutInt(nDelPos);	// �ҏW�J�n��
	ctwArg.nDelLines    = 0;					// �폜�s��1�s
	ctwArg.nAllLinesOld = nAllLinesOld;			// �ҏW�O�̃e�L�X�g�s��
	ctwArg.bInsData     = FALSE;				// �ǉ�������Ȃ�

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	CLayoutInt	nAddInsLineNum;
	*pnModifyLayoutLinesNew = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		CLogicPoint(nDelStartLogicalPos, nDelStartLogicalLine),
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	*pnDeleteLayoutLines = nAllLinesOld - m_nLines + nAddInsLineNum;
	return;
}






/*!	������}��

	@date 2002/03/24 YAZAKI bUndo�폜
	@date 2009/08/28 nasukoji	�e�L�X�g�ő啝�̎Z�o�ɑΉ�
*/
void CLayoutMgr::InsertData_CLayoutMgr(
	CLayoutInt		nLineNum,
	CLogicInt		nInsPos,
	const wchar_t*	pInsData,
	CLogicInt		nInsDataLen,
	CLayoutInt*		pnModifyLayoutLinesOld,
	CLayoutInt*		pnInsLineNum,			// �}���ɂ���đ��������C�A�E�g�s�̐�
	CLayoutPoint*	pptNewLayout			// �}�����ꂽ�����̎��̈ʒu
)
{
	CLayout*		pLayout;
	CLayout*		pLayoutPrev;
	CLayout*		pLayoutWork = NULL;
	CLogicInt		nInsStartLogicalLine;
	CLogicInt		nInsStartLogicalPos;
	CLogicInt		nInsLineNum;
	CLogicInt		nRowNum;
	EColorIndexType	nCurrentLineType;
	CLayoutInt		nLineWork;


	// ���ݍs�̃f�[�^���擾 -> pLine, nLineLen
	CLogicInt		nLineLen;
	const wchar_t*	pLine = GetLineStr( nLineNum, &nLineLen );
	if( !pLine ){
		/*
			2004.04.02 FILE / Moca �J�[�\���ʒu�s���̂��߁A��e�L�X�g��
			nLineNum��0�łȂ��Ƃ��ɗ�����΍�D�f�[�^����ł��邱�Ƃ�
			�J�[�\���ʒu�ł͂Ȃ����s���Ŕ��肷�邱�Ƃł��m���ɁD
		*/
		if( m_nLines == CLayoutInt(0) )
		{
			/* ��̃e�L�X�g�̐擪�ɍs�����ꍇ */
			pLayout = NULL;
			nLineWork = CLayoutInt(0);
			nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
			nInsStartLogicalPos  = CLogicInt(0);
			nCurrentLineType = COLORIDX_DEFAULT;
		}
		else{
			using namespace WCODE;

			pLine = GetLineStr( m_nLines - CLayoutInt(1), &nLineLen );
			//�I�[2�����̂ǂ��炩��cr,lf�̂����ꂩ���܂܂�Ă���ꍇ
			if( ( nLineLen > 0 && ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF )) ||
				( nLineLen > 1 && ( pLine[nLineLen - 2] == CR || pLine[nLineLen - 2] == LF )) ){
				// ��łȂ��e�L�X�g�̍Ō�ɍs�����ꍇ
				pLayout = NULL;
				nLineWork = CLayoutInt(0);
				nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
				nInsStartLogicalPos  = CLogicInt(0);
				nCurrentLineType = m_nLineTypeBot;
			}
			else{
				/* ��łȂ��e�L�X�g�̍Ō�̍s��ύX����ꍇ */
				nLineNum = m_nLines	- CLayoutInt(1);
				nInsPos = nLineLen;
				pLayout = m_pLayoutPrevRefer;
				nLineWork = m_nPrevReferLine;


				nInsStartLogicalLine = pLayout->GetLogicLineNo();
				nInsStartLogicalPos  = nInsPos + pLayout->GetLogicOffset();
				nCurrentLineType = pLayout->GetColorTypePrev();
			}
		}
	}else{
		pLayout = m_pLayoutPrevRefer;
		nLineWork = m_nPrevReferLine;


		nInsStartLogicalLine = pLayout->GetLogicLineNo();
		nInsStartLogicalPos  = nInsPos + pLayout->GetLogicOffset();
		nCurrentLineType = pLayout->GetColorTypePrev();
	}

	if( NULL != pLayout ){
		pLayoutWork = pLayout;
		while( pLayoutWork != NULL && 0 != pLayoutWork->GetLogicOffset() ){
			pLayoutWork = pLayoutWork->GetPrevLayout();
			nLineWork--;
		}
		if( NULL != pLayoutWork ){
			nCurrentLineType = pLayoutWork->GetColorTypePrev();
		}else{
			nCurrentLineType = COLORIDX_DEFAULT;
		}
	}


	// �f�[�^�̑}��
	CLogicPoint ptNewPos;	//�}�����ꂽ�����̎��̃f�[�^�ʒu
	CDocEditAgent(m_pcDocLineMgr).InsertData_CDocLineMgr(
		nInsStartLogicalLine,
		nInsStartLogicalPos,
		pInsData,
		nInsDataLen,
		&nInsLineNum,
		&ptNewPos
	);


	//--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---
	// �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜����
	// �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ�
	CLayoutInt	nAllLinesOld = m_nLines;
	if( NULL != pLayout ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,
			nInsStartLogicalLine,
			nInsStartLogicalLine,
			CLogicPoint(nInsStartLogicalPos, nInsStartLogicalLine),
			pnModifyLayoutLinesOld
		);
	}else{
		pLayoutPrev = m_pLayoutBot;
	}

	// �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g����
	// �_���s���폜���ꂽ�ꍇ�͂O��菬�����s��
	// �_���s���}�����ꂽ�ꍇ�͂O���傫���s��
	if( pLine ){
		ShiftLogicalLineNum( pLayoutPrev, nInsLineNum );
	}

	// �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->GetLogicLineNo();
		}
	}
	else{
		if( NULL == pLayoutPrev->GetNextLayout() ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->GetLogicLineNo() - CLogicInt(1);
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->GetLogicLineNo() -
				pLayoutPrev->GetLogicLineNo() - CLogicInt(1);
		}
	}

	// 2009.08.28 nasukoji	�e�L�X�g�ő啝�Z�o�p�̈�����ݒ�
	CalTextWidthArg ctwArg;
	ctwArg.nLineFrom    = nLineNum;				// �ҏW�J�n�s
	ctwArg.nColumnFrom  = CLayoutInt(nInsPos);	// �ҏW�J�n��
	ctwArg.nDelLines    = -1;					// �폜�s�Ȃ�
	ctwArg.nAllLinesOld = nAllLinesOld;			// �ҏW�O�̃e�L�X�g�s��
	ctwArg.bInsData     = TRUE;					// �ǉ������񂠂�

	// �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	CLayoutInt		nAddInsLineNum;
	DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		CLogicPoint(nInsStartLogicalPos, nInsStartLogicalLine),
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	*pnInsLineNum = m_nLines - nAllLinesOld + nAddInsLineNum;

	// �_���ʒu�����C�A�E�g�ʒu�ϊ�
	LogicToLayout( ptNewPos, pptNewLayout );
	return;
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
	CLayoutInt	nTabSize,
	CLayoutInt	nMaxLineKetas
)
{
	if( nTabSize < 1 || nTabSize > 64 ) { return false; }
	if( nMaxLineKetas < MINLINEKETAS || nMaxLineKetas > MAXLINEKETAS ){ return false; }

	m_sTypeConfig.m_nTabSpace = nTabSize;
	m_sTypeConfig.m_nMaxLineKetas = nMaxLineKetas;

	_DoLayout();

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
	const wchar_t*			pszPattern,			//!< [in] ��������
	ESearchDirection		eSearchDirection,	//!< [in] ��������
	const SSearchOption&	sSearchOption,		//!< [in] �����I�v�V����
	CLayoutRange*			pMatchRange,		//!< [out] �}�b�`���C�A�E�g�͈�
	CBregexp*				pRegexp				//!< [in]  ���K�\���R���p�C���f�[�^  Jun. 26, 2001 genta
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
		pszPattern,
		eSearchDirection,
		sSearchOption,
		&cLogicRange, //pMatchRange,
		pRegexp			/* ���K�\���R���p�C���f�[�^ */
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
	if(nLineHint==0){
		nCaretPosY = CLayoutInt(ptLogic.y);

		// [�q���g�����̏ꍇ]
		// ���W�b�N�s <= ���C�A�E�g�s �����藧����A
		// �T�[�`�J�n�n�_���ł��邾���ړI�n�֋߂Â���
		pLayout = SearchLineByLayoutY( nCaretPosY );
		if( !pLayout ){
			if( 0 < m_nLines ){
				pptLayout->SetY( m_nLines );
			}
			return;
		}
	}
	else{
		nCaretPosY = nLineHint;

		// [�q���g�L��̏ꍇ]
		pLayout = SearchLineByLayoutY(nCaretPosY);
		if(!pLayout) pLayout = SearchLineByLayoutY( nCaretPosY = CLayoutInt(0) );
		
		//���W�b�NY���ł�������ꍇ�́A��v����܂Ńf�N�������g (
		while(pLayout->GetLogicLineNo() > ptLogic.GetY2()){
			pLayout = pLayout->GetPrevLayout();
			nCaretPosY--;
		}

		//���W�b�NY�������ꍇ�́A���W�b�NY���̍ŏ����C�A�E�gY���J�n�n�_�Ƃ���
		if(pLayout->GetLogicLineNo() == ptLogic.GetY2()){
			while(pLayout->m_pPrev && pLayout->GetPrevLayout()->GetLogicLineNo() == ptLogic.GetY2()){
				pLayout = pLayout->GetPrevLayout();
				nCaretPosY--;
			}
		}

	}


	//	Layout���P����ɐi�߂Ȃ���ptLogic.y�������s�Ɉ�v����Layout��T��
	do{
		if( ptLogic.GetY2() == pLayout->GetLogicLineNo() ){
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
				if( pData[i] ==	WCODE::TAB ){
					// Sep. 23, 2002 genta �����o�[�֐����g���悤��
					nCharKetas = GetActualTabSpace( nCaretPosX );
				}
				else{
					nCharKetas = CNativeW::GetKetaOfChar( pData, nDataLen, i );
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
				if( pData[nDataLen - 1] == L'\r' || pData[nDataLen - 1] == L'\n' ){
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
		if( pData[i] == WCODE::TAB ){
			nCharKetas = GetActualTabSpace( nX );
		}
		else{
			nCharKetas = CNativeW::GetKetaOfChar( pData, nDataLen, i );
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
	MYTRACE( _T("m_nMaxLineKetas=%d\n"), m_sTypeConfig.m_nMaxLineKetas );

	MYTRACE( _T("m_nTabSpace=%d\n"), m_sTypeConfig.m_nTabSpace );
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



