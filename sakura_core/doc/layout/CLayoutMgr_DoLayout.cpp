#include "StdAfx.h"
#include "doc/CEditDoc.h" /// 2003/07/20 genta
#include "doc/layout/CLayoutMgr.h"
#include "doc/layout/CLayout.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLine.h"/// 2002/2/10 aroka
#include "doc/logic/CDocLineMgr.h"// 2002/2/10 aroka
#include "charset/charcode.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "util/window.h"
#include "debug/CRunningTimer.h"


//���C�A�E�g���֑̋��^�C�v	//@@@ 2002.04.20 MIK
#define	KINSOKU_TYPE_NONE			0	//�Ȃ�
#define	KINSOKU_TYPE_WORDWRAP		1	//���[�h���b�v��
#define	KINSOKU_TYPE_KINSOKU_HEAD	2	//�s���֑���
#define	KINSOKU_TYPE_KINSOKU_TAIL	3	//�s���֑���
#define	KINSOKU_TYPE_KINSOKU_KUTO	4	//��Ǔ_�Ԃ牺����


//2008.07.27 kobake
static bool _GetKeywordLength(
	const CStringRef&	cLineStr,		//!< [in]
	CLogicInt			nPos,			//!< [in]
	CLogicInt*			p_nWordBgn,		//!< [out]
	CLogicInt*			p_nWordLen,		//!< [out]
	CLayoutInt*			p_nWordKetas	//!< [out]
)
{
	//�L�[���[�h�����J�E���g����
	CLogicInt nWordBgn = nPos;
	CLogicInt nWordLen = CLogicInt(0);
	CLayoutInt nWordKetas = CLayoutInt(0);
	while(nPos<cLineStr.GetLength() && IS_KEYWORD_CHAR(cLineStr.At(nPos))){
		CLayoutInt k = CNativeW::GetKetaOfChar( cLineStr, nPos);
		if(0 == k)k = CLayoutInt(1);

		nWordLen+=1;
		nWordKetas+=k;
		nPos++;
	}
	//����
	if(nWordLen>0){
		*p_nWordBgn = nWordBgn;
		*p_nWordLen = nWordLen;
		*p_nWordKetas = nWordKetas;
		return true;
	}
	else{
		return false;
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ���i�X�e�[�^�X                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CLayout* CLayoutMgr::SLayoutWork::_CreateLayout(CLayoutMgr* mgr)
{
	return mgr->CreateLayout(
		this->pcDocLine,
		CLogicPoint(this->nBgn, this->nCurLine),
		this->nPos - this->nBgn,
		this->pcColorStrategy_Prev->GetStrategyColorSafe(),
		this->nIndent,
		this->nPosX
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���i                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CLayoutMgr::_DoKinsokuSkip(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	int	nEol = pWork->pcDocLine->GetEol().GetLen();

	if( KINSOKU_TYPE_NONE != pWork->nKinsokuType )
	{
		//�֑������̍Ō���ɒB������֑�����������������
		if( pWork->nPos >= pWork->nWordBgn + pWork->nWordLen )
		{
			if( pWork->nKinsokuType == KINSOKU_TYPE_KINSOKU_KUTO && pWork->nPos == pWork->nWordBgn + pWork->nWordLen )
			{
				if( ! (m_sTypeConfig.m_bKinsokuRet && (pWork->nPos == pWork->cLineStr.GetLength() - nEol) && nEol ) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
				{
					(this->*pfOnLine)(pWork);
				}
			}

			pWork->nWordLen = CLogicInt(0);
			pWork->nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		}
		return true;
	}
	else{
		return false;
	}
}

void CLayoutMgr::_DoWordWrap(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	if( m_sTypeConfig.m_bWordWrap	/* �p�����[�h���b�v������ */
	 && pWork->nKinsokuType == KINSOKU_TYPE_NONE )
	{
		/* �p�P��̐擪�� */
		if( pWork->nPos >= pWork->nBgn && IS_KEYWORD_CHAR(pWork->cLineStr.At(pWork->nPos)) ){
			// �L�[���[�h�����擾
			CLayoutInt nWordKetas;
			_GetKeywordLength(
				pWork->cLineStr, pWork->nPos,
				&pWork->nWordBgn, &pWork->nWordLen, &nWordKetas
			);

			pWork->nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK

			if( pWork->nPosX+nWordKetas>=m_sTypeConfig.m_nMaxLineKetas && pWork->nPos-pWork->nBgn>0 )
			{
				(this->*pfOnLine)(pWork);
			}
		}
	}
}

void CLayoutMgr::_DoKutoBurasage(SLayoutWork* pWork)
{
	if( m_sTypeConfig.m_bKinsokuKuto && (m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX < 2) && (pWork->nKinsokuType == KINSOKU_TYPE_NONE) )
	{
		// 2007.09.07 kobake   ���C�A�E�g�ƃ��W�b�N�̋��
		CLayoutInt nCharKetas = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos );

		if( IsKinsokuPosKuto(m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX, nCharKetas) && IsKinsokuKuto( pWork->cLineStr.At(pWork->nPos) ) )
		{
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 1;
			pWork->nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
		}
	}
}

void CLayoutMgr::_DoGyotoKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	if( m_sTypeConfig.m_bKinsokuHead
	 && (pWork->nPos+1 < pWork->cLineStr.GetLength())	// 2007.02.17 ryoji �ǉ�
	 && (m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX < 4)
	 && ( pWork->nPosX > pWork->nIndent )	//	2004.04.09 pWork->nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
	 && (pWork->nKinsokuType == KINSOKU_TYPE_NONE) )
	{
		// 2007.09.07 kobake   ���C�A�E�g�ƃ��W�b�N�̋��
		CLayoutInt nCharKetas2 = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos );
		CLayoutInt nCharKetas3 = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos+1 );

		if( IsKinsokuPosHead( m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX, nCharKetas2, nCharKetas3 )
		 && IsKinsokuHead( pWork->cLineStr.At(pWork->nPos+1) )
		 && ! IsKinsokuHead( pWork->cLineStr.At(pWork->nPos) )	//1�����O���s���֑��łȂ�
		 && ! IsKinsokuKuto( pWork->cLineStr.At(pWork->nPos) ) )	//��Ǔ_�łȂ�
		{
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 2;
			pWork->nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;

			(this->*pfOnLine)(pWork);
		}
	}
}

void CLayoutMgr::_DoGyomatsuKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	if( m_sTypeConfig.m_bKinsokuTail
	 && (pWork->nPos+1 < pWork->cLineStr.GetLength())	// 2007.02.17 ryoji �ǉ�
	 && (m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX < 4)
	 && ( pWork->nPosX > pWork->nIndent )	//	2004.04.09 pWork->nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
	 && (pWork->nKinsokuType == KINSOKU_TYPE_NONE) )
	{	/* �s���֑����� && �s���t�� && �s���łȂ�����(�����ɋ֑����Ă��܂�����) */
		CLayoutInt nCharKetas2 = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos );
		CLayoutInt nCharKetas3 = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos+1 );

		if( IsKinsokuPosTail(m_sTypeConfig.m_nMaxLineKetas - pWork->nPosX, nCharKetas2, nCharKetas3) && IsKinsokuTail(pWork->cLineStr.At(pWork->nPos)) ){
			pWork->nWordBgn = pWork->nPos;
			pWork->nWordLen = 1;
			pWork->nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
			
			(this->*pfOnLine)(pWork);
		}
	}
}

//�܂�Ԃ��ꍇ��true��Ԃ�
bool CLayoutMgr::_DoTab(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	//	Sep. 23, 2002 genta ��������������̂Ŋ֐����g��
	CLayoutInt nCharKetas = GetActualTabSpace( pWork->nPosX );
	if( pWork->nPosX + nCharKetas > m_sTypeConfig.m_nMaxLineKetas ){
		(this->*pfOnLine)(pWork);
		return true;
	}
	pWork->nPosX += nCharKetas;
	pWork->nPos += CLogicInt(1);
	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ������                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_MakeOneLine(SLayoutWork* pWork, PF_OnLine pfOnLine)
{
	int	nEol = pWork->pcDocLine->GetEol().GetLen(); //########���̂����s�v�ɂȂ�
	int nEol_1 = nEol - 1;
	if( 0 >	nEol_1 ){
		nEol_1 = 0;
	}

	if(pWork->pcColorStrategy)pWork->pcColorStrategy->InitStrategyStatus();

	//1���W�b�N�s����������܂Ń��[�v
	while( pWork->nPos < pWork->cLineStr.GetLength() - CLogicInt(nEol_1) ){
		//	�C���f���g���̌v�Z�R�X�g�������邽�߂̕���
		if ( pWork->pLayout && pWork->pLayout != pWork->pLayoutCalculated && pWork->nBgn ){
			//	�v�Z
			//	Oct, 1, 2002 genta Indent�T�C�Y���擾����悤�ɕύX
			pWork->nIndent = (this->*m_getIndentOffset)( pWork->pLayout );

			//	�v�Z�ς�
			pWork->pLayoutCalculated = pWork->pLayout;
		}

SEARCH_START:;
		//�֑��������Ȃ�X�L�b�v����	@@@ 2002.04.20 MIK
		if(_DoKinsokuSkip(pWork, pfOnLine)){ }
		else{
			// ���[�h���b�v����
			_DoWordWrap(pWork, pfOnLine);

			// ��Ǔ_�̂Ԃ炳��
			_DoKutoBurasage(pWork);

			// �s���֑�
			_DoGyotoKinsoku(pWork, pfOnLine);

			// �s���֑�
			_DoGyomatsuKinsoku(pWork, pfOnLine);
		}

		//@@@ 2002.09.22 YAZAKI
		bool bGotoSEARCH_START = CColorStrategyPool::getInstance()->CheckColorMODE( &pWork->pcColorStrategy, pWork->nPos, pWork->cLineStr );
		if ( bGotoSEARCH_START )
			goto SEARCH_START;
		
		if( pWork->cLineStr.At(pWork->nPos) == WCODE::TAB ){
			if(_DoTab(pWork, pfOnLine)){
				continue;
			}
		}
		else{
			if( pWork->nPos>=pWork->cLineStr.GetLength() ){
				break;
			}
			// 2007.09.07 kobake   ���W�b�N���ƃ��C�A�E�g�������
			CLayoutInt nCharKetas = CNativeW::GetKetaOfChar( pWork->cLineStr, pWork->nPos );
//			if( 0 == nCharKetas ){				// �폜 �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
//				nCharKetas = CLayoutInt(1);
//			}

			if( pWork->nPosX + nCharKetas > m_sTypeConfig.m_nMaxLineKetas ){
				if( pWork->nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
				{
					if( ! (m_sTypeConfig.m_bKinsokuRet && (pWork->nPos == pWork->cLineStr.GetLength() - nEol) && nEol) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
					{
						(this->*pfOnLine)(pWork);
						continue;
					}
				}
			}
			pWork->nPos+= 1;
			pWork->nPosX += nCharKetas;
		}
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �{����(�S��)                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_OnLine1(SLayoutWork* pWork)
{
	AddLineBottom( pWork->_CreateLayout(this) );
	m_nLineTypeBot = pWork->pcColorStrategy->GetStrategyColorSafe();
	pWork->pLayout = m_pLayoutBot;
	pWork->pcColorStrategy_Prev = pWork->pcColorStrategy;
	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
	pWork->nPosX = pWork->nIndent = (this->*m_getIndentOffset)( pWork->pLayout );
	pWork->pLayoutCalculated = pWork->pLayout;
}

/*!
	���݂̐܂�Ԃ��������ɍ��킹�đS�f�[�^�̃��C�A�E�g�����Đ������܂�

	@date 2004.04.03 Moca TAB���g����Ɛ܂�Ԃ��ʒu�������̂�h�����߁C
		nPosX���C���f���g���܂ޕ���ێ�����悤�ɕύX�Dm_sTypeConfig.m_nMaxLineKetas��
		�Œ�l�ƂȂ������C�����R�[�h�̒u�������͔����čŏ��ɒl��������悤�ɂ����D
*/
void CLayoutMgr::_DoLayout()
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::_DoLayout" );

	/*	�\�����X�ʒu
		2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
	*/
	int			nAllLineNum;

	if( GetListenerCount() != 0 ){
		NotifyProgress(0);
		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) )return;
	}

	_Empty();
	Init();
	
	//	Nov. 16, 2002 genta
	//	�܂�Ԃ��� <= TAB���̂Ƃ��������[�v����̂�����邽�߁C
	//	TAB���܂�Ԃ����ȏ�̎���TAB=4�Ƃ��Ă��܂�
	//	�܂�Ԃ����̍ŏ��l=10�Ȃ̂ł��̒l�͖��Ȃ�
	if( m_sTypeConfig.m_nTabSpace >= m_sTypeConfig.m_nMaxLineKetas ){
		m_sTypeConfig.m_nTabSpace = CLayoutInt(4);
	}

	nAllLineNum = m_pcDocLineMgr->GetLineCount();

	SLayoutWork	_sWork;
	SLayoutWork* pWork = &_sWork;
	pWork->pcDocLine				= m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr�ύX
	pWork->pLayout					= m_pLayoutBot;
	pWork->pcColorStrategy			= NULL;
	pWork->pcColorStrategy_Prev		= NULL;
	pWork->nCurLine					= CLogicInt(0);

	while( NULL != pWork->pcDocLine ){
		pWork->cLineStr		= pWork->pcDocLine->GetStringRefWithEOL();
		pWork->nKinsokuType	= KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= CLogicInt(0);
		pWork->nPos			= CLogicInt(0);
		pWork->nWordBgn		= CLogicInt(0);
		pWork->nWordLen		= CLogicInt(0);
		pWork->nPosX		= CLayoutInt(0);	// �\�����X�ʒu
		pWork->nIndent		= CLayoutInt(0);	// �C���f���g��
		pWork->pLayoutCalculated	= NULL;		// �C���f���g���v�Z�ς݂�CLayout.


		_MakeOneLine(pWork, &CLayoutMgr::_OnLine1);

		if( pWork->nPos - pWork->nBgn > 0 ){
// 2002/03/13 novice
			if( pWork->pcColorStrategy->GetStrategyColorSafe() == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
				pWork->pcColorStrategy = NULL;
			}
			AddLineBottom( pWork->_CreateLayout(this) );
			m_nLineTypeBot = pWork->pcColorStrategy->GetStrategyColorSafe();
			pWork->pcColorStrategy_Prev = pWork->pcColorStrategy;
		}

		// ���̍s��
		pWork->nCurLine++;
		pWork->pcDocLine = pWork->pcDocLine->GetNextLine();
		
		// �������̃��[�U�[������\�ɂ���
		if( GetListenerCount()!=0 && 0 < nAllLineNum && 0 == ( pWork->nCurLine % 1024 ) ){
			NotifyProgress(pWork->nCurLine * 100 / nAllLineNum);
			if( !::BlockingHook( NULL ) )return;
		}

// 2002/03/13 novice
		if( pWork->pcColorStrategy_Prev->GetStrategyColorSafe() == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
			pWork->pcColorStrategy_Prev = NULL;
		}
		pWork->pcColorStrategy = pWork->pcColorStrategy_Prev;
	}

	m_nPrevReferLine = CLayoutInt(0);
	m_pLayoutPrevRefer = NULL;

	if( GetListenerCount()!=0 ){
		NotifyProgress(0);
		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) )return;
	}
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �{����(�͈͎w��)                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CLayoutMgr::_OnLine2(SLayoutWork* pWork)
{
	//@@@ 2002.09.23 YAZAKI �œK��
	if( pWork->bNeedChangeCOMMENTMODE ){
		pWork->pLayout = pWork->pLayout->GetNextLayout();
		pWork->pLayout->SetColorTypePrev(pWork->pcColorStrategy_Prev->GetStrategyColorSafe());
		(*pWork->pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
	}
	else {
		pWork->pLayout = InsertLineNext( pWork->pLayout, pWork->_CreateLayout(this) );
	}
	pWork->pcColorStrategy_Prev = pWork->pcColorStrategy;

	pWork->nBgn = pWork->nPos;
	// 2004.03.28 Moca pWork->nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
	pWork->nPosX = pWork->nIndent = (this->*m_getIndentOffset)( pWork->pLayout );
	pWork->pLayoutCalculated = pWork->pLayout;
	if( ( pWork->ptDelLogicalFrom.GetY2() == pWork->nCurLine && pWork->ptDelLogicalFrom.GetX2() < pWork->nPos ) ||
		( pWork->ptDelLogicalFrom.GetY2() < pWork->nCurLine )
	){
		(pWork->nModifyLayoutLinesNew)++;
	}
}

/*!
	�w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	
	@date 2002.10.07 YAZAKI rename from "DoLayout3_New"
	@date 2004.04.03 Moca TAB���g����Ɛ܂�Ԃ��ʒu�������̂�h�����߁C
		pWork->nPosX���C���f���g���܂ޕ���ێ�����悤�ɕύX�Dm_sTypeConfig.m_nMaxLineKetas��
		�Œ�l�ƂȂ������C�����R�[�h�̒u�������͔����čŏ��ɒl��������悤�ɂ����D
	@date 2009.08.28 nasukoji	�e�L�X�g�ő啝�̎Z�o�ɑΉ�

	@note 2004.04.03 Moca
		_DoLayout�Ƃ͈���ă��C�A�E�g��񂪃��X�g���Ԃɑ}������邽�߁C
		�}�����m_nLineTypeBot�փR�����g���[�h���w�肵�Ă͂Ȃ�Ȃ�
		����ɍŏI�s�̃R�����g���[�h���I���ԍۂɊm�F���Ă���D
*/
CLayoutInt CLayoutMgr::DoLayout_Range(
	CLayout*		pLayoutPrev,
	CLogicInt		nLineNum,
	CLogicPoint		_ptDelLogicalFrom,
	EColorIndexType	nCurrentLineType,
	const CalTextWidthArg*	pctwArg,
	CLayoutInt*		_pnExtInsLineNum
)
{
	*_pnExtInsLineNum = CLayoutInt(0);

	CLogicInt	nLineNumWork = CLogicInt(0);

	// 2006.12.01 Moca �r���ɂ܂ōč\�z�����ꍇ��EOF�ʒu�����ꂽ�܂�
	//	�X�V����Ȃ��̂ŁC�͈͂ɂ�����炸�K�����Z�b�g����D
	m_nEOFColumn = CLayoutInt(-1);
	m_nEOFLine = CLayoutInt(-1);

	SLayoutWork _sWork;
	SLayoutWork* pWork = &_sWork;
	pWork->pLayout					= pLayoutPrev;
	pWork->pcColorStrategy			= CColorStrategyPool::getInstance()->GetStrategyByColor(nCurrentLineType);
	pWork->pcColorStrategy_Prev		= pWork->pcColorStrategy;
	pWork->bNeedChangeCOMMENTMODE	= false;
	if( NULL == pWork->pLayout ){
		pWork->nCurLine = CLogicInt(0);
	}else{
		pWork->nCurLine = pWork->pLayout->GetLogicLineNo() + CLogicInt(1);
	}
	pWork->pcDocLine				= m_pcDocLineMgr->GetLine( pWork->nCurLine );
	pWork->nModifyLayoutLinesNew	= CLayoutInt(0);
	//����
	pWork->ptDelLogicalFrom		= _ptDelLogicalFrom;
	pWork->pnExtInsLineNum		= _pnExtInsLineNum;

	if(pWork->pcColorStrategy)pWork->pcColorStrategy->InitStrategyStatus();

	while( NULL != pWork->pcDocLine ){
		pWork->cLineStr		= pWork->pcDocLine->GetStringRefWithEOL();
		pWork->nKinsokuType	= KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
		pWork->nBgn			= CLogicInt(0);
		pWork->nPos			= CLogicInt(0);
		pWork->nWordBgn		= CLogicInt(0);
		pWork->nWordLen		= CLogicInt(0);
		pWork->nPosX		= CLayoutInt(0);			// �\�����X�ʒu
		pWork->nIndent		= CLayoutInt(0);			// �C���f���g��
		pWork->pLayoutCalculated	= pWork->pLayout;	// �C���f���g���v�Z�ς݂�CLayout.

		_MakeOneLine(pWork, &CLayoutMgr::_OnLine2);

		if( pWork->nPos - pWork->nBgn > 0 ){
// 2002/03/13 novice
			if( pWork->pcColorStrategy->GetStrategyColorSafe() == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
				pWork->pcColorStrategy = NULL;
			}
			//@@@ 2002.09.23 YAZAKI �œK��
			_OnLine2(pWork);
		}

		nLineNumWork++;
		pWork->nCurLine++;

		/* �ړI�̍s��(nLineNum)�ɒB�������A�܂��͒ʂ�߂����i���s�����������j���m�F */
		//@@@ 2002.09.23 YAZAKI �œK��
		if( nLineNumWork >= nLineNum ){
			if( pWork->pLayout && pWork->pLayout->m_pNext 
				&& ( pWork->pcColorStrategy_Prev->GetStrategyColorSafe() != pWork->pLayout->m_pNext->GetColorTypePrev() )
			){
				//	COMMENTMODE���قȂ�s�������܂����̂ŁA���̍s�����̍s�ƍX�V���Ă����܂��B
				pWork->bNeedChangeCOMMENTMODE = true;
			}else{
				break;	//	while( NULL != pWork->pcDocLine ) �I��
			}
		}
		pWork->pcDocLine = pWork->pcDocLine->GetNextLine();
// 2002/03/13 novice
		if( pWork->pcColorStrategy_Prev->GetStrategyColorSafe() == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
			pWork->pcColorStrategy_Prev = NULL;
		}
		pWork->pcColorStrategy = pWork->pcColorStrategy_Prev;
	}


	// 2004.03.28 Moca EOF�����̘_���s�̒��O�̍s�̐F�������m�F�E�X�V���ꂽ
	if( pWork->nCurLine == m_pcDocLineMgr->GetLineCount() ){
		m_nLineTypeBot = pWork->pcColorStrategy_Prev->GetStrategyColorSafe();
		// 2006.10.01 Moca �ŏI�s���ύX���ꂽ�BEOF�ʒu����j������B
		m_nEOFColumn = CLayoutInt(-1);
		m_nEOFLine = CLayoutInt(-1);
	}

	// 2009.08.28 nasukoji	�e�L�X�g���ҏW���ꂽ��ő啝���Z�o����
	CalculateTextWidth_Range(pctwArg);

// 1999.12.22 ���C�A�E�g��񂪂Ȃ��Ȃ��ł͂Ȃ��̂�
//	m_nPrevReferLine = 0;
//	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	return pWork->nModifyLayoutLinesNew;
}

/*!
	@brief �e�L�X�g���ҏW���ꂽ��ő啝���Z�o����

	@param[in] pctwArg �e�L�X�g�ő啝�Z�o�p�\����

	@note �u�܂�Ԃ��Ȃ��v�I�����̂݃e�L�X�g�ő啝���Z�o����D
	      �ҏW���ꂽ�s�͈̔͂ɂ��ĎZ�o����i���L�𖞂����ꍇ�͑S�s�j
	      �@�폜�s�Ȃ����F�ő啝�̍s���s���ȊO�ɂĉ��s�t���ŕҏW����
	      �@�폜�s���莞�F�ő啝�̍s���܂�ŕҏW����
	      pctwArg->nDelLines �������̎��͍폜�s�Ȃ��D

	@date 2009.08.28 nasukoji	�V�K�쐬
*/
void CLayoutMgr::CalculateTextWidth_Range( const CalTextWidthArg* pctwArg )
{
	if( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){	// �u�܂�Ԃ��Ȃ��v
		CLayoutInt	nCalTextWidthLinesFrom(0);	// �e�L�X�g�ő啝�̎Z�o�J�n���C�A�E�g�s
		CLayoutInt	nCalTextWidthLinesTo(0);	// �e�L�X�g�ő啝�̎Z�o�I�����C�A�E�g�s
		BOOL bCalTextWidth        = TRUE;		// �e�L�X�g�ő啝�̎Z�o�v����ON
		CLayoutInt nInsLineNum    = m_nLines - pctwArg->nAllLinesOld;		// �ǉ��폜�s��

		// �폜�s�Ȃ����F�ő啝�̍s���s���ȊO�ɂĉ��s�t���ŕҏW����
		// �폜�s���莞�F�ő啝�̍s���܂�ŕҏW����

		if(( pctwArg->nDelLines < CLayoutInt(0)  && Int(m_nTextWidth) &&
		     Int(nInsLineNum) && Int(pctwArg->nColmFrom) && m_nTextWidthMaxLine == pctwArg->nLineFrom )||
		   ( pctwArg->nDelLines >= CLayoutInt(0) && Int(m_nTextWidth) &&
		     pctwArg->nLineFrom <= m_nTextWidthMaxLine && m_nTextWidthMaxLine <= pctwArg->nLineFrom + pctwArg->nDelLines ))
		{
			// �S���C���𑖍�����
			nCalTextWidthLinesFrom = -1;
			nCalTextWidthLinesTo   = -1;
		}else if( Int(nInsLineNum) || Int(pctwArg->bInsData) ){		// �ǉ��폜�s �܂��� �ǉ������񂠂�
			// �ǉ��폜�s�݂̂𑖍�����
			nCalTextWidthLinesFrom = pctwArg->nLineFrom;

			// �ŏI�I�ɕҏW���ꂽ�s���i3�s�폜2�s�ǉ��Ȃ�2�s�ǉ��j
			// �@1�s��MAXLINEKETAS�𒴂���ꍇ�s��������Ȃ��Ȃ邪�A������ꍇ�͂��̐�̌v�Z���̂�
			// �@�s�v�Ȃ̂Ōv�Z���Ȃ����߂��̂܂܂Ƃ���B
			CLayoutInt nEditLines = nInsLineNum + ((pctwArg->nDelLines > 0) ? pctwArg->nDelLines : CLayoutInt(0));
			nCalTextWidthLinesTo   = pctwArg->nLineFrom + ((nEditLines > 0) ? nEditLines : CLayoutInt(0));

			// �ő啝�̍s���㉺����̂��v�Z
			if( Int(m_nTextWidth) && Int(nInsLineNum) && m_nTextWidthMaxLine >= pctwArg->nLineFrom )
				m_nTextWidthMaxLine += nInsLineNum;
		}else{
			// �ő啝�ȊO�̍s�����s���܂܂��Ɂi1�s���Łj�ҏW����
			bCalTextWidth = FALSE;		// �e�L�X�g�ő啝�̎Z�o�v����OFF
		}

#if defined( _DEBUG ) && defined( _UNICODE )
		static int testcount = 0;
		testcount++;

		// �e�L�X�g�ő啝���Z�o����
		if( bCalTextWidth ){
//			MYTRACE_W( L"CLayoutMgr::DoLayout_Range(%d) nCalTextWidthLinesFrom=%d nCalTextWidthLinesTo=%d\n", testcount, nCalTextWidthLinesFrom, nCalTextWidthLinesTo );
			CalculateTextWidth( FALSE, nCalTextWidthLinesFrom, nCalTextWidthLinesTo );
//			MYTRACE_W( L"CLayoutMgr::DoLayout_Range() m_nTextWidthMaxLine=%d\n", m_nTextWidthMaxLine );
		}else{
//			MYTRACE_W( L"CLayoutMgr::DoLayout_Range(%d) FALSE\n", testcount );
		}
#else
		// �e�L�X�g�ő啝���Z�o����
		if( bCalTextWidth )
			CalculateTextWidth( FALSE, nCalTextWidthLinesFrom, nCalTextWidthLinesTo );
#endif
	}
}
