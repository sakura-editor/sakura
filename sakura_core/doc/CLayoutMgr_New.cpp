/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI, novice
	Copyright (C) 2003, genta
	Copyright (C) 2004, Moca, genta
	Copyright (C) 2005, D.S.Koba, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include <mbstring.h>
#include "CLayoutMgr.h"
#include "charset/charcode.h"
#include "debug/Debug.h"
#include <commctrl.h>
#include "debug/CRunningTimer.h"
#include "doc/CLayout.h"/// 2002/2/10 aroka
#include "doc/CDocLine.h"/// 2002/2/10 aroka
#include "doc/CDocLineMgr.h"// 2002/2/10 aroka
#include "mem/CMemory.h"/// 2002/2/10 aroka
#include "mem/CMemoryIterator.h"
#include "doc/CEditDoc.h" /// 2003/07/20 genta
#include "util/window.h"

//���C�A�E�g���֑̋��^�C�v	//@@@ 2002.04.20 MIK
#define	KINSOKU_TYPE_NONE			0	//�Ȃ�
#define	KINSOKU_TYPE_WORDWRAP		1	//���[�h���b�v��
#define	KINSOKU_TYPE_KINSOKU_HEAD	2	//�s���֑���
#define	KINSOKU_TYPE_KINSOKU_TAIL	3	//�s���֑���
#define	KINSOKU_TYPE_KINSOKU_KUTO	4	//��Ǔ_�Ԃ牺����



/*!
	���݂̐܂�Ԃ��������ɍ��킹�đS�f�[�^�̃��C�A�E�g�����Đ������܂�

	@date 2004.04.03 Moca TAB���g����Ɛ܂�Ԃ��ʒu�������̂�h�����߁C
		nPosX���C���f���g���܂ޕ���ێ�����悤�ɕύX�DnMaxLineKetas��
		�Œ�l�ƂȂ������C�����R�[�h�̒u�������͔����čŏ��ɒl��������悤�ɂ����D
*/
void CLayoutMgr::_DoLayout()
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::_DoLayout" );

	CLogicInt	nLineNum;
	CDocLine*	pCDocLine;
	/*	�\�����X�ʒu
		2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
	*/
	CLayoutInt			nPosX; //���C�A�E�g�P��
	EColorIndexType		nCOMMENTMODE;
	EColorIndexType		nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	int			nAllLineNum;

	CLayoutInt	nIndent;			//	�C���f���g��
	CLayout*	pLayoutCalculated;	//	�C���f���g���v�Z�ς݂�CLayout.
	CLayoutInt	nMaxLineKetas;

// 2002/03/13 novice
	nCOMMENTMODE = COLORIDX_TEXT;
	nCOMMENTMODE_Prev = COLORIDX_TEXT;

	
	if( GetListenerCount() != 0 ){
		NotifyProgress(0);
		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) )return;
	}

	_Empty();
	Init();
	nLineNum = CLogicInt(0);
	
	//	Nov. 16, 2002 genta
	//	�܂�Ԃ��� <= TAB���̂Ƃ��������[�v����̂�����邽�߁C
	//	TAB���܂�Ԃ����ȏ�̎���TAB=4�Ƃ��Ă��܂�
	//	�܂�Ԃ����̍ŏ��l=10�Ȃ̂ł��̒l�͖��Ȃ�
	if( m_sTypeConfig.m_nTabSpace >= m_sTypeConfig.m_nMaxLineKetas ){
		m_sTypeConfig.m_nTabSpace = CLayoutInt(4);
	}

	pCDocLine = m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr�ύX

	nCOMMENTEND = 0;
	nAllLineNum = m_pcDocLineMgr->GetLineCount();

	/*
		2004.03.28 Moca TAB�v�Z�𐳂������邽�߂ɃC���f���g�𕝂Œ������邱�Ƃ͂��Ȃ�
		nMaxLineKetas�͕ύX���Ȃ��̂ŁC������m_nMaxLineKetas��ݒ肷��D
	*/
	nMaxLineKetas = m_sTypeConfig.m_nMaxLineKetas;

	while( NULL != pCDocLine ){
		CLogicInt		nLineLen;
		const wchar_t*	pLine = pCDocLine->GetDocLineStrWithEOL( &nLineLen );
		nPosX = CLayoutInt(0);

		CLogicInt	nBgn = CLogicInt(0);
		CLogicInt	nPos = CLogicInt(0); // ��������̈ʒu(offset) ���W�b�N�P��

		CLogicInt	nWordBgn = CLogicInt(0);
		CLogicInt	nWordLen = CLogicInt(0);

		int			nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int	nEol = pCDocLine->GetEol().GetLen();
		int nEol_1 = nEol - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		nIndent = CLayoutInt(0);	//	�C���f���g��
		pLayoutCalculated = NULL;	//	�C���f���g���v�Z�ς݂�CLayout.

		while( nPos < nLineLen - CLogicInt(nEol_1) ){
			//	�C���f���g���̌v�Z�R�X�g�������邽�߂̕���
			if ( m_pLayoutBot && m_pLayoutBot!=pLayoutCalculated && nBgn )
			{
				//	�v�Z
				//	Oct, 1, 2002 genta Indent�T�C�Y���擾����悤�ɕύX
				nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
				
				//	�v�Z�ς�
				pLayoutCalculated = m_pLayoutBot;
			}


			SEARCH_START:;
			
			//�֑��������Ȃ�X�L�b�v����	@@@ 2002.04.20 MIK
			if( KINSOKU_TYPE_NONE != nKinsokuType )
			{
				//�֑������̍Ō���ɒB������֑�����������������
				if( nPos >= nWordBgn + nWordLen )
				{
					if( nKinsokuType == KINSOKU_TYPE_KINSOKU_KUTO && nPos == nWordBgn + nWordLen )
					{
						if( ! (m_sTypeConfig.m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
						{
							AddLineBottom(
								CreateLayout(
									pCDocLine,
									CLogicPoint(nBgn, nLineNum),
									nPos - nBgn,
									nCOMMENTMODE_Prev,
									nIndent
								)
							);
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
						}
					}
					
					nWordLen = CLogicInt(0);
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ���[�h���b�v���� */
				if( m_sTypeConfig.m_bWordWrap	/* �p�����[�h���b�v������ */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
					/* �p�P��̐擪�� */
					int nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( nPos>=nBgn && nCharChars==1 && IS_KEYWORD_CHAR(pLine[nPos]) ){
						// 2007.09.07 kobake ���C�A�E�g�ƃ��W�b�N�̋��
						// �L�[���[�h������̏I�[��T��
						// -> iLogic  (���W�b�N�P�ʏI�[)
						// -> iLayout (���C�A�E�g�P�ʏI�[)
						CLogicInt	iLogic = nPos + CLogicInt(1);
						CLayoutInt	iLayout = nPosX + CNativeW::GetKetaOfChar( pLine, nLineLen, nPos);
						while( iLogic < nLineLen )
						{
							CLogicInt nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, iLogic );
							if( 0 == nCharChars2 ){
								nCharChars2 = CLogicInt(1);
							}
							CLayoutInt nCharKetas = CNativeW::GetKetaOfChar( pLine, nLineLen, iLogic);

							if( nCharChars2 != 1 )break;
							if( !IS_KEYWORD_CHAR( pLine[iLogic] ) )break;

							iLogic+=nCharChars2;
							iLayout+=nCharKetas;
						}
						nWordBgn = nPos;
						nWordLen = iLogic - nPos;
						CLayoutInt nWordKetas = iLayout - nPosX;

						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK
						
						if( nPosX+nWordKetas>=nMaxLineKetas && nPos-nBgn>0 )
						{
							AddLineBottom(
								CreateLayout(
									pCDocLine,
									CLogicPoint(nBgn, nLineNum),
									nPos - nBgn,
									nCOMMENTMODE_Prev,
									nIndent
								)
							);
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
						}
					}
				}

				//@@@ 2002.04.07 MIK start
				/* ��Ǔ_�̂Ԃ炳�� */
				if( m_sTypeConfig.m_bKinsokuKuto && (nMaxLineKetas - nPosX < 2) && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					// 2007.09.07 kobake   ���C�A�E�g�ƃ��W�b�N�̋��
					CLogicInt  nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas  = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos );

					if( IsKinsokuPosKuto( nMaxLineKetas - nPosX, nCharKetas ) && IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* �s���֑� */
				if( m_sTypeConfig.m_bKinsokuHead
				 && (nMaxLineKetas - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					CLogicInt nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					CLogicInt nCharChars3 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );
					CLayoutInt nCharKetas2 = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas3 = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosHead( nMaxLineKetas - nPosX, nCharKetas2, nCharKetas3 )
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1�����O���s���֑��łȂ�
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//��Ǔ_�łȂ�
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
						nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
						pLayoutCalculated = m_pLayoutBot;
					}
				}

				/* �s���֑� */
				if( m_sTypeConfig.m_bKinsokuTail
				 && (nMaxLineKetas - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* �s���֑����� && �s���t�� && �s���łȂ�����(�����ɋ֑����Ă��܂�����) */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					CLogicInt nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					CLogicInt nCharChars3 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );
					CLayoutInt nCharKetas2 = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas3 = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosTail( nMaxLineKetas - nPosX, nCharKetas2, nCharKetas3 )
						&& IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
						nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
						pLayoutCalculated = m_pLayoutBot;
					}
				}
				//@@@ 2002.04.08 MIK end
			}	//if( KINSOKU_TYPE_NONE != nKinsokuTyoe ) �֑�������

			//@@@ 2002.09.22 YAZAKI
			bool bGotoSEARCH_START = _CheckColorMODE( &nCOMMENTMODE, &nCOMMENTEND, nPos, nLineLen, pLine );
			if ( bGotoSEARCH_START )
				goto SEARCH_START;
			
			if( pLine[nPos] == WCODE::TAB ){
				//	Sep. 23, 2002 genta ��������������̂Ŋ֐����g��
				CLayoutInt nCharKetas = GetActualTabSpace( nPosX );
				if( nPosX + nCharKetas > nMaxLineKetas ){
					AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
					m_nLineTypeBot = nCOMMENTMODE;
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					nBgn = nPos;
					// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
					nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
					pLayoutCalculated = m_pLayoutBot;
					continue;
				}
				nPosX += nCharKetas;
				nPos += CLogicInt(1);
			}else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				// 2007.09.07 kobake   ���W�b�N���ƃ��C�A�E�g�������
				CLogicInt nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
				if( 0 == nCharChars2 ){
					nCharChars2 = CLogicInt(1);
					break;	//@@@ 2002.04.16 MIK
				}
				CLayoutInt nCharKetas = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos );
//				if( 0 == nCharKetas ){				// �폜 �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
//					nCharKetas = CLayoutInt(1);
//				}

				if( nPosX + nCharKetas > nMaxLineKetas ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_sTypeConfig.m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*m_getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
							continue;
						}	//@@@ 2002.04.14 MIK
					}
				}
				nPos+= CLogicInt(nCharChars2);
				nPosX += nCharKetas;
			}
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			AddLineBottom( CreateLayout(pCDocLine, CLogicPoint(nBgn, nLineNum), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
			m_nLineTypeBot = nCOMMENTMODE;
			nCOMMENTMODE_Prev = nCOMMENTMODE;
		}
		nLineNum++;
		if( GetListenerCount()!=0 && 0 < nAllLineNum && 0 == ( nLineNum % 1024 ) ){
			NotifyProgress(nLineNum * 100 / nAllLineNum);
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( NULL ) )return;
		}
//		pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->GetNextLine();;
// 2002/03/13 novice
		if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
			nCOMMENTMODE_Prev = COLORIDX_TEXT;
		}
		nCOMMENTMODE = nCOMMENTMODE_Prev;
		nCOMMENTEND = 0;
	}
	m_nPrevReferLine = CLayoutInt(0);
	m_pLayoutPrevRefer = NULL;

	if( GetListenerCount()!=0 ){
		NotifyProgress(0);
		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) )return;
	}
}





/*!
	�w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	
	@date 2002.10.07 YAZAKI rename from "DoLayout3_New"
	@date 2004.04.03 Moca TAB���g����Ɛ܂�Ԃ��ʒu�������̂�h�����߁C
		nPosX���C���f���g���܂ޕ���ێ�����悤�ɕύX�DnMaxLineKetas��
		�Œ�l�ƂȂ������C�����R�[�h�̒u�������͔����čŏ��ɒl��������悤�ɂ����D

	@note 2004.04.03 Moca
		_DoLayout�Ƃ͈���ă��C�A�E�g��񂪃��X�g���Ԃɑ}������邽�߁C
		�}�����m_nLineTypeBot�փR�����g���[�h���w�肵�Ă͂Ȃ�Ȃ�
		����ɍŏI�s�̃R�����g���[�h���I���ԍۂɊm�F���Ă���D
*/
CLayoutInt CLayoutMgr::DoLayout_Range(
	CLayout*		pLayoutPrev,
	CLogicInt		nLineNum,
	CLogicPoint		ptDelLogicalFrom,
	EColorIndexType	nCurrentLineType,
	CLayoutInt*		pnExtInsLineNum
)
{
	*pnExtInsLineNum = CLayoutInt(0);

	bool		bNeedChangeCOMMENTMODE = false;	//@@@ 2002.09.23 YAZAKI bAdd�𖼏̕ύX
	CLayoutInt	nMaxLineKetas= m_sTypeConfig.m_nMaxLineKetas;
	CLogicInt	nLineNumWork = CLogicInt(0);

	CLayout*	pLayout = pLayoutPrev;

	CLogicInt	nCurLine;
	if( NULL == pLayout ){
		nCurLine = CLogicInt(0);
	}else{
		nCurLine = pLayout->GetLogicLineNo() + CLogicInt(1);
	}

	EColorIndexType		nCOMMENTMODE = nCurrentLineType;
	EColorIndexType		nCOMMENTMODE_Prev = nCOMMENTMODE;

	CDocLine*	pCDocLine = m_pcDocLineMgr->GetLine( nCurLine );

	int			nCOMMENTEND = 0;
	CLayoutInt	nModifyLayoutLinesNew = CLayoutInt(0);

	// 2006.12.01 Moca �r���ɂ܂ōč\�z�����ꍇ��EOF�ʒu�����ꂽ�܂�
	//	�X�V����Ȃ��̂ŁC�͈͂ɂ�����炸�K�����Z�b�g����D
	m_nEOFColumn = CLayoutInt(-1);
	m_nEOFLine = CLayoutInt(-1);

	while( NULL != pCDocLine ){
		CLogicInt		nLineLen;
		const wchar_t*	pLine = pCDocLine->GetDocLineStrWithEOL( &nLineLen );
		CLayoutInt		nPosX = CLayoutInt(0); //�\�����X�ʒu

		// ��������̈ʒu(offset)
		CLogicInt	nBgn = CLogicInt(0);
		CLogicInt	nPos = CLogicInt(0);

		CLogicInt	nWordBgn = CLogicInt(0);
		CLogicInt	nWordLen = CLogicInt(0);

		int			nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int	nEol = pCDocLine->GetEol().GetLen();
		int nEol_1 = nEol - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		CLayoutInt	nIndent = CLayoutInt(0);				//	�C���f���g��
		CLayout*	pLayoutCalculated = pLayout;	//	�C���f���g���v�Z�ς݂�CLayout.

		while( nPos < nLineLen - CLogicInt(nEol_1) ){
			//	�C���f���g���̌v�Z�R�X�g�������邽�߂̕���
			if ( pLayout && pLayout != pLayoutCalculated ){

				 //	�v�Z
				//	Oct, 1, 2002 genta Indent�T�C�Y���擾����悤�ɕύX
				nIndent = (this->*m_getIndentOffset)( pLayout );

				//	�v�Z�ς�
				pLayoutCalculated = pLayout;
			}

			SEARCH_START:;
			
			//�֑��������Ȃ�X�L�b�v����	@@@ 2002.04.20 MIK
			if( KINSOKU_TYPE_NONE != nKinsokuType )
			{
				//�֑������̍Ō���ɒB������֑�����������������
				if( nPos >= nWordBgn + nWordLen )
				{
					if( nKinsokuType==KINSOKU_TYPE_KINSOKU_KUTO && nPos==nWordBgn+nWordLen )
					{
						if( ! (m_sTypeConfig.m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
						{
							//@@@ 2002.09.23 YAZAKI �œK��
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->GetNextLayout();
								pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
								(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
								( ptDelLogicalFrom.GetY2() < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
						}
					}

					nWordLen = CLogicInt(0);
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ���[�h���b�v���� */
				if( m_sTypeConfig.m_bWordWrap	/* �p�����[�h���b�v������ */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
					/* �p�P��̐擪�� */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( nPos >= nBgn && nCharChars == 1 && IS_KEYWORD_CHAR(pLine[nPos]) ){
						/* �L�[���[�h������̏I�[��T�� */
						// 2007.09.07 kobake ���C�A�E�g�ƃ��W�b�N�̋��
						CLogicInt	iLogic = nPos + CLogicInt(1);
						CLayoutInt	iLayout = nPosX + CNativeW::GetKetaOfChar( pLine, nLineLen, nPos);
						while(iLogic < nLineLen){
							CLogicInt nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, iLogic );
							if( 0 == nCharChars2 ){
								nCharChars2 = CLogicInt(1);
							}
							CLayoutInt nCharKetas = CNativeW::GetKetaOfChar( pLine, nLineLen, iLogic);
							if( 0 == nCharKetas ){
								nCharKetas = CLayoutInt(1);
							}

							if( nCharChars2 != 1 )break;
							if( !IS_KEYWORD_CHAR(pLine[iLogic]) )break;

							iLogic+=nCharChars2;
							iLayout+=nCharKetas;
						}
						nWordBgn = nPos;
						nWordLen = iLogic - nPos;
						CLayoutInt nWordKetas = iLayout - nPosX;
						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK

						if( nPosX+nWordKetas>=nMaxLineKetas && nPos-nBgn>0 )
						{
							//@@@ 2002.09.23 YAZAKI �œK��
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->GetNextLayout();
								pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
								(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
								( ptDelLogicalFrom.GetY2() < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
//?							continue;
						}
					}
				}

				//@@@ 2002.04.07 MIK start
				/* ��Ǔ_�̂Ԃ炳�� */
				if( m_sTypeConfig.m_bKinsokuKuto
				 && (nMaxLineKetas - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					// 2007.09.07 kobake   ���C�A�E�g�ƃ��W�b�N�̋��
					CLogicInt  nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos );

					if( IsKinsokuPosKuto( nMaxLineKetas - nPosX, nCharKetas )
						&& IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* �s���֑� */
				if( m_sTypeConfig.m_bKinsokuHead
				 && (nMaxLineKetas - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					// 2007.09.07 kobake   ���C�A�E�g�ƃ��W�b�N�̋��
					CLogicInt  nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas2 = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos );
					CLogicInt  nCharChars3 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );
					CLayoutInt nCharKetas3 = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosHead( nMaxLineKetas - nPosX, nCharKetas2, nCharKetas3 )
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1�����O���s���֑��łȂ�
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//��Ǔ_�łȂ�
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						//@@@ 2002.09.23 YAZAKI �œK��
						if( bNeedChangeCOMMENTMODE ){
							pLayout = pLayout->GetNextLayout();
							pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
							(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
						}
						else {
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						}
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
						nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
						pLayoutCalculated = pLayout;
						if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
							( ptDelLogicalFrom.GetY2() < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}

				/* �s���֑� */
				if( m_sTypeConfig.m_bKinsokuTail
				 && (nMaxLineKetas - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* �s���֑����� && �s���t�� && �s���łȂ�����(�����ɋ֑����Ă��܂�����) */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					// 2007.09.07 kobake   ���C�A�E�g�ƃ��W�b�N�̋��
					CLogicInt  nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
					CLayoutInt nCharKetas2 = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos );
					CLogicInt  nCharChars3 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );
					CLayoutInt nCharKetas3 = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosTail( nMaxLineKetas - nPosX, nCharKetas2, nCharKetas3 )
						&& IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						//@@@ 2002.09.23 YAZAKI �œK��
						if( bNeedChangeCOMMENTMODE ){
							pLayout = pLayout->GetNextLayout();
							pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
							(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
						}
						else {
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						}
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
						nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
						pLayoutCalculated = pLayout;
						if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
							( ptDelLogicalFrom.GetY2() < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}
				//@@@ 2002.04.08 MIK end
			}	// if( nKinsokuType != KINSOKU_TYPE_NONE )

			//@@@ 2002.09.22 YAZAKI
			bool bGotoSEARCH_START = _CheckColorMODE( &nCOMMENTMODE, &nCOMMENTEND, nPos, nLineLen, pLine );
			if ( bGotoSEARCH_START )
				goto SEARCH_START;

			CLayoutInt nCharKetas;
			if( pLine[nPos] == WCODE::TAB ){
				//	Sep. 23, 2002 genta ��������������̂Ŋ֐����g��
				nCharKetas = GetActualTabSpace( nPosX );
				if( nPosX + nCharKetas > nMaxLineKetas ){
					//@@@ 2002.09.23 YAZAKI �œK��
					if( bNeedChangeCOMMENTMODE ){
						pLayout = pLayout->GetNextLayout();
						pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
						(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
					}
					else {
						pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
					}
					nCOMMENTMODE_Prev = nCOMMENTMODE;

					nBgn = nPos;
					// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
					nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
					pLayoutCalculated = pLayout;
					if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
						( ptDelLogicalFrom.GetY2() < nCurLine )
					){
						(nModifyLayoutLinesNew)++;;
					}
					continue;
				}
				nPos++;
			}
			else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				// 2007.09.07 kobake   ���C�A�E�g�ƃ��W�b�N�̋��
				nCharKetas = CNativeW::GetKetaOfChar( pLine, nLineLen, nPos );
//				if( 0 == nCharKetas ){				// �폜 �T���Q�[�g�y�A�΍�	2008/7/5 Uchi
//					nCharKetas = CLayoutInt(1);
//				}
				CLogicInt nCharChars2 = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos);
				if( 0 == nCharChars2 ){
					break;
				}
				if( nPosX + nCharKetas > nMaxLineKetas ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_sTypeConfig.m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							//@@@ 2002.09.23 YAZAKI �œK��
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->GetNextLayout();
								pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
								(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*m_getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
								( ptDelLogicalFrom.GetY2() < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
							continue;
						}
					}
				}
				nPos+= nCharChars2;
			}

			nPosX += nCharKetas;
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			//@@@ 2002.09.23 YAZAKI �œK��
			if( bNeedChangeCOMMENTMODE ){
				pLayout = pLayout->GetNextLayout();
				pLayout->SetColorTypePrev(nCOMMENTMODE_Prev);
				(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
			}
			else {
				pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, CLogicPoint(nBgn, nCurLine), nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
			}
			nCOMMENTMODE_Prev = nCOMMENTMODE;

			if( ( ptDelLogicalFrom.GetY2() == nCurLine && ptDelLogicalFrom.GetX2() < nPos ) ||
				( ptDelLogicalFrom.GetY2() < nCurLine )
			){
				(nModifyLayoutLinesNew)++;;
			}
		}

		nLineNumWork++;
		nCurLine++;

		/* �ړI�̍s��(nLineNum)�ɒB�������A�܂��͒ʂ�߂����i���s�����������j���m�F */
		//@@@ 2002.09.23 YAZAKI �œK��
		if( nLineNumWork >= nLineNum ){
			if( NULL != pLayout
			 && NULL != pLayout->m_pNext
			 && ( nCOMMENTMODE_Prev != pLayout->m_pNext->GetColorTypePrev() )
			){
				//	COMMENTMODE���قȂ�s�������܂����̂ŁA���̍s�����̍s�ƍX�V���Ă����܂��B
				bNeedChangeCOMMENTMODE = true;
			}else{
				break;	//	while( NULL != pCDocLine ) �I��
			}
		}
		pCDocLine = pCDocLine->GetNextLine();
// 2002/03/13 novice
		if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
			nCOMMENTMODE_Prev = COLORIDX_TEXT;
		}
		nCOMMENTMODE = nCOMMENTMODE_Prev;
		nCOMMENTEND = 0;

	}

	// 2004.03.28 Moca EOF�����̘_���s�̒��O�̍s�̐F�������m�F�E�X�V���ꂽ
	if( nCurLine == m_pcDocLineMgr->GetLineCount() ){
		m_nLineTypeBot = nCOMMENTMODE_Prev;
		// 2006.10.01 Moca �ŏI�s���ύX���ꂽ�BEOF�ʒu����j������B
		m_nEOFColumn = CLayoutInt(-1);
		m_nEOFLine = CLayoutInt(-1);
	}

// 1999.12.22 ���C�A�E�g��񂪂Ȃ��Ȃ��ł͂Ȃ��̂�
//	m_nPrevReferLine = 0;
//	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	return nModifyLayoutLinesNew;
}

/*!
	�s���֑������ɊY�����邩�𒲂ׂ�D

	@param[in] pLine ���ׂ镶���ւ̃|�C���^
	@param[in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool CLayoutMgr::IsKinsokuHead( const wchar_t *pLine, CLogicInt length )
{
	if(length==1 && m_pszKinsokuHead_1.size()){
		wchar_t wc=pLine[0];
		return m_pszKinsokuHead_1.exist(wc);
	}
	else{
		return false;
	}
}

/*!
	�s���֑������ɊY�����邩�𒲂ׂ�D

	@param[in] pLine ���ׂ镶���ւ̃|�C���^
	@param[in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool CLayoutMgr::IsKinsokuTail( const wchar_t *pLine, CLogicInt length )
{
	if(length==1 && m_pszKinsokuTail_1.size()){
		wchar_t wc=pLine[0];
		return m_pszKinsokuTail_1.exist(wc);
	}
	else{
		return false;
	}
}


/*!
	�֑��Ώۋ�Ǔ_�ɊY�����邩�𒲂ׂ�D

	@param [in] pLine  ���ׂ镶���ւ̃|�C���^
	@param [in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool CLayoutMgr::IsKinsokuKuto( const wchar_t *pLine, CLogicInt length )
{
	if(length==1 && m_pszKinsokuKuto_1.size()){
		wchar_t wc=pLine[0];
		return m_pszKinsokuKuto_1.exist(wc);
	}
	else{
		return false;
	}
}

/*!
	@param[in] nRest �s�̎c�蕶����
	@param[in] nCharChars ���݈ʒu�̕����T�C�Y
	@param[in] nCharChars2 ���݈ʒu�̎��̕����T�C�Y

	@date 2005-08-20 D.S.Koba _DoLayout()��DoLayout_Range()���番��
*/
bool CLayoutMgr::IsKinsokuPosHead(
	CLayoutInt nRest,
	CLayoutInt nCharKetas,
	CLayoutInt nCharKetas2
)
{
	switch( (Int)nRest )
	{
	//    321012  ���}�W�b�N�i���o�[
	// 3 "��j" : 22 "�j"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
	// 2  "Z�j" : 12 "�j"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
	// 2  "��j": 22 "�j"�Ő܂�Ԃ��̂Ƃ�
	// 2  "��)" : 21 ")"�Ő܂�Ԃ��̂Ƃ�
	// 1   "Z�j": 12 "�j"�Ő܂�Ԃ��̂Ƃ�
	// 1   "Z)" : 11 ")"�Ő܂�Ԃ��̂Ƃ�
	//���������O���H
	// ���������A"��Z"�������֑��Ȃ珈�����Ȃ��B
	case 3:	// 3�����O
		if( nCharKetas == 2 && nCharKetas2 == 2 ){
			return true;
		}
		break;
	case 2:	// 2�����O
		if( nCharKetas == 2 ){
			return true;
		}
		else if( nCharKetas == 1 && nCharKetas2 == 2 ){
			return true;
		}
		break;
	case 1:	// 1�����O
		if( nCharKetas == 1 ){
			return true;
		}
		break;
	}
	return false;
}

/*!
	@param[in] nRest �s�̎c�蕶����
	@param[in] nCharChars ���݈ʒu�̕����T�C�Y
	@param[in] nCharChars2 ���݈ʒu�̎��̕����T�C�Y

	@date 2005-08-20 D.S.Koba _DoLayout()��DoLayout_Range()���番��
*/
bool CLayoutMgr::IsKinsokuPosTail(
	CLayoutInt nRest,
	CLayoutInt nCharKetas,
	CLayoutInt nCharKetas2
)
{
	switch( (Int)nRest )
	{
	case 3:	// 3�����O
		if( nCharKetas == 2 && nCharKetas2 == 2){
			// "�i��": "��"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	case 2:	// 2�����O
		if( nCharKetas == 2 ){
			// "�i��": "��"�Ő܂�Ԃ��̂Ƃ�
			return true;
		}
		else if( nCharKetas == 1 && nCharKetas2 == 2){
			// "(��": "��"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	case 1:	// 1�����O
		if( nCharKetas == 1 ){
			// "(��": "��"�Ő܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	}
	return false;
}

int CLayoutMgr::Match_Quote( wchar_t wcQuote, int nPos, int nLineLen, const wchar_t* pLine )
{
	int nCharChars;
	int i;
	for( i = nPos; i < nLineLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if(	m_sTypeConfig.m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && pLine[i] == L'\\' ){
				++i;
			}else
			if( 1 == nCharChars && pLine[i] == wcQuote ){
				return i + 1;
			}
		}
		else if(	m_sTypeConfig.m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && pLine[i] == wcQuote ){
				if( i + 1 < nLineLen && pLine[i + 1] == wcQuote ){
					++i;
				}else{
					return i + 1;
				}
			}
		}
		if( 2 == nCharChars ){
			++i;
		}
	}
	return nLineLen;
}

/*!
	@brief �s�̒������v�Z���� (2�s�ڈȍ~�̎���������)
	
	���������s��Ȃ��̂ŁC���0��Ԃ��D
	�����͎g��Ȃ��D
	
	@return 1�s�̕\�������� (���0)
	
	@author genta
	@date 2002.10.01
*/
CLayoutInt CLayoutMgr::getIndentOffset_Normal( CLayout* )
{
	return CLayoutInt(0);
}

/*!
	@brief �C���f���g�����v�Z���� (Tx2x)
	
	�O�̍s�̍Ō��TAB�̈ʒu���C���f���g�ʒu�Ƃ��ĕԂ��D
	�������C�c�蕝��6���������̏ꍇ�̓C���f���g���s��Ȃ��D
	
	@author Yazaki
	@return �C���f���g���ׂ�������
	
	@date 2002.10.01 
	@date 2002.10.07 YAZAKI ���̕ύX, ����������
*/
CLayoutInt CLayoutMgr::getIndentOffset_Tx2x( CLayout* pLayoutPrev )
{
	//	�O�̍s�������Ƃ��́A�C���f���g�s�v�B
	if ( pLayoutPrev == NULL ) return CLayoutInt(0);

	CLayoutInt nIpos = pLayoutPrev->GetIndent();

	//	�O�̍s���܂�Ԃ��s�Ȃ�΂���ɍ��킹��
	if( pLayoutPrev->GetLogicOffset() > 0 )
		return nIpos;
	
	CMemoryIterator it( pLayoutPrev, m_sTypeConfig.m_nTabSpace );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && it.getCurrentChar() == WCODE::TAB ){
			nIpos = it.getColumn() + it.getColumnDelta();
		}
		it.addDelta();
	}
	if ( m_sTypeConfig.m_nMaxLineKetas - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	������߂�
	}
	return nIpos;	//	�C���f���g
}

/*!
	@brief �C���f���g�����v�Z���� (�X�y�[�X��������)
	
	�_���s�s���̃z���C�g�X�y�[�X�̏I���C���f���g�ʒu�Ƃ��ĕԂ��D
	�������C�c�蕝��6���������̏ꍇ�̓C���f���g���s��Ȃ��D
	
	@author genta
	@return �C���f���g���ׂ�������
	
	@date 2002.10.01 
*/
CLayoutInt CLayoutMgr::getIndentOffset_LeftSpace( CLayout* pLayoutPrev )
{
	//	�O�̍s�������Ƃ��́A�C���f���g�s�v�B
	if ( pLayoutPrev == NULL ) return CLayoutInt(0);

	//	�C���f���g�̌v�Z
	CLayoutInt nIpos = pLayoutPrev->GetIndent();
	
	//	Oct. 5, 2002 genta
	//	�܂�Ԃ���3�s�ڈȍ~��1�O�̍s�̃C���f���g�ɍ��킹��D
	if( pLayoutPrev->GetLogicOffset() > 0 )
		return nIpos;
	
	//	2002.10.07 YAZAKI �C���f���g�̌v�Z
	CMemoryIterator it( pLayoutPrev, m_sTypeConfig.m_nTabSpace );

	//	Jul. 20, 2003 genta �����C���f���g�ɏ���������ɂ���
	bool bZenSpace = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bAutoIndent_ZENSPACE;
	const wchar_t* szSpecialIndentChar = m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_szIndentChars;
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && WCODE::IsIndentChar(it.getCurrentChar(),bZenSpace) )
		{
			//	�C���f���g�̃J�E���g���p������
		}
		//	Jul. 20, 2003 genta �C���f���g�Ώە���
		else if( szSpecialIndentChar[0] != L'\0' ){
			wchar_t buf[3]; // �����̒�����1 or 2
			wmemcpy( buf, it.getCurrentPos(), it.getIndexDelta() );
			buf[ it.getIndexDelta() ] = L'\0';
			if( NULL != wcsstr( szSpecialIndentChar, buf )){
				//	�C���f���g�̃J�E���g���p������
			}
			else {
				nIpos = it.getColumn();	//	�I��
				break;
			}
		}
		else {
			nIpos = it.getColumn();	//	�I��
			break;
		}
		it.addDelta();
	}
	if ( m_sTypeConfig.m_nMaxLineKetas - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	������߂�
	}
	return nIpos;	//	�C���f���g
}
