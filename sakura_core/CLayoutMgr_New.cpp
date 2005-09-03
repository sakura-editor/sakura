//	$Id$
/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK
	Copyright (C) 2004, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include <mbstring.h>
#include "CLayoutMgr.h"
#include "charcode.h"
#include "etc_uty.h"
#include "debug.h"
#include <commctrl.h>
#include "CRunningTimer.h"
#include "CLayout.h"/// 2002/2/10 aroka
#include "CDocLine.h"/// 2002/2/10 aroka
#include "CDocLineMgr.h"// 2002/2/10 aroka
#include "CMemory.h"/// 2002/2/10 aroka
#include "CMemoryIterator.h"
#include "CEditDoc.h" /// 2003/07/20 genta

//���C�A�E�g���֑̋��^�C�v	//@@@ 2002.04.20 MIK
#define	KINSOKU_TYPE_NONE			0	//�Ȃ�
#define	KINSOKU_TYPE_WORDWRAP		1	//���[�h���b�v��
#define	KINSOKU_TYPE_KINSOKU_HEAD	2	//�s���֑���
#define	KINSOKU_TYPE_KINSOKU_TAIL	3	//�s���֑���
#define	KINSOKU_TYPE_KINSOKU_KUTO	4	//��Ǔ_�Ԃ牺����



/*!
	���݂̐܂�Ԃ��������ɍ��킹�đS�f�[�^�̃��C�A�E�g�����Đ������܂�

	@date 2004.04.03 Moca TAB���g����Ɛ܂�Ԃ��ʒu�������̂�h�����߁C
		nPosX���C���f���g���܂ޕ���ێ�����悤�ɕύX�DnMaxLineSize��
		�Œ�l�ƂȂ������C�����R�[�h�̒u�������͔����čŏ��ɒl��������悤�ɂ����D
*/
void CLayoutMgr::DoLayout(
		HWND	hwndProgress,
		BOOL	bDispSSTRING,	/* �V���O���N�H�[�e�[�V�����������\������ */
		BOOL	bDispWSTRING	/* �_�u���N�H�[�e�[�V�����������\������ */
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::DoLayout" );

	int			nLineNum;
	int			nLineLen;
	CDocLine*	pCDocLine;
	const char* pLine;
	int			nBgn;
	//!	��������̈ʒu(offset)
	int			nPos;
	/*	�\�����X�ʒu
		2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
	*/
	int			nPosX;
	int			nCOMMENTMODE;
	int			nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	int			nWordBgn;
	int			nWordLen;
	int			nAllLineNum;
	int			nKinsokuType;	//@@@ 2002.04.20 MIK

	int			nIndent;			//	�C���f���g��
	CLayout*	pLayoutCalculated;	//	�C���f���g���v�Z�ς݂�CLayout.
	int			nMaxLineSize;

// 2002/03/13 novice
	nCOMMENTMODE = COLORIDX_TEXT;
	nCOMMENTMODE_Prev = COLORIDX_TEXT;

	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) ){
			return;
		}
	}

	Empty();
	Init();
	nLineNum = 0;
	
	//	Nov. 16, 2002 genta
	//	�܂�Ԃ��� <= TAB���̂Ƃ��������[�v����̂�����邽�߁C
	//	TAB���܂�Ԃ����ȏ�̎���TAB=4�Ƃ��Ă��܂�
	//	�܂�Ԃ����̍ŏ��l=10�Ȃ̂ł��̒l�͖��Ȃ�
	if( m_nTabSpace >= m_nMaxLineSize ){
		m_nTabSpace = 4;
	}

//	pLine = m_pcDocLineMgr->GetFirstLinrStr( &nLineLen );
	pCDocLine = m_pcDocLineMgr->GetDocLineTop(); // 2002/2/10 aroka CDocLineMgr�ύX

// 2005-08-20 D.S.Koba �폜
//// 2002/03/13 novice
//	if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
//		nCOMMENTMODE_Prev = COLORIDX_TEXT;
//	}
//	nCOMMENTMODE = nCOMMENTMODE_Prev;
	nCOMMENTEND = 0;
	nAllLineNum = m_pcDocLineMgr->GetLineCount();

	/*
		2004.03.28 Moca TAB�v�Z�𐳂������邽�߂ɃC���f���g�𕝂Œ������邱�Ƃ͂��Ȃ�
		nMaxLineSize�͕ύX���Ȃ��̂ŁC������m_nMaxLineSize��ݒ肷��D
	*/
	nMaxLineSize = m_nMaxLineSize;

	while( NULL != pCDocLine ){
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		nPosX = 0;
		nBgn = 0;
		nPos = 0;
		nWordBgn = 0;
		nWordLen = 0;
		nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int	nEol = pCDocLine->m_cEol.GetLen();
		int nEol_1 = nEol - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		nIndent = 0;				//	�C���f���g��
		pLayoutCalculated = NULL;	//	�C���f���g���v�Z�ς݂�CLayout.

		while( nPos < nLineLen - nEol_1 ){
			//	�C���f���g���̌v�Z�R�X�g�������邽�߂̕���
			if ( m_pLayoutBot && 
				 m_pLayoutBot != pLayoutCalculated &&
				 nBgn ){
				//	�v�Z
				//	Oct, 1, 2002 genta Indent�T�C�Y���擾����悤�ɕύX
				nIndent = (this->*getIndentOffset)( m_pLayoutBot );
				// 2004.03.28 Moca nMaxLineSize���������@���ƁA�^�u���̌v�Z������Ȃ��̂ŁAnPosX�̏����l��nIndent�ɂ���
				//	nMaxLineSize = m_nMaxLineSize - nIndent;
				
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
					if( nKinsokuType == KINSOKU_TYPE_KINSOKU_KUTO
					 && nPos == nWordBgn + nWordLen )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
						{
							AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
						}
					}
					
					nWordLen = 0;
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ���[�h���b�v���� */
				if( m_bWordWrap	/* �p�����[�h���b�v������ */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
//				if( 0 == nWordLen ){
					/* �p�P��̐擪�� */
					//int nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					int nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( nPos >= nBgn &&
						nCharChars == 1 &&
//						( pLine[nPos] == '#' || __iscsym( pLine[nPos] ) )
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						/* �L�[���[�h������̏I�[��T�� */
						int	i;
						// 2005-08-27 D.S.Koba �ȗ���
						for( i = nPos + 1; i < nLineLen; ++i ){
							//int nCharChars2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars2 ){
								nCharChars2 = 1;
							}
							if( nCharChars2 != 1 || !IS_KEYWORD_CHAR( pLine[i] ) ){
								break;
							}
						}
						nWordBgn = nPos;
						nWordLen = i - nPos;
						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK
						if( nPosX + i - nPos >= nMaxLineSize
						 && nPos - nBgn > 0
						){
							AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
//?							continue;
						}
					}
//				}else{
//					if( nPos == nWordBgn + nWordLen ){
//						nWordLen = 0;
//					}
//				}
				}

				//@@@ 2002.04.07 MIK start
				/* ��Ǔ_�̂Ԃ炳�� */
				if( m_bKinsokuKuto
				 && (nMaxLineSize - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					
					if( IsKinsokuPosKuto( nMaxLineSize - nPosX, nCharChars2 )
						&& IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* �s���֑� */
				if( m_bKinsokuHead
				 && (nMaxLineSize - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					int nCharChars3 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosHead( nMaxLineSize - nPosX, nCharChars2, nCharChars3 )
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1�����O���s���֑��łȂ�
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//��Ǔ_�łȂ�
					{
						//nPos += nCharChars2 + nCharChars3; nPosX += nCharChars2 + nCharChars3;
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
						nPosX = nIndent = (this->*getIndentOffset)( m_pLayoutBot );
						pLayoutCalculated = m_pLayoutBot;
					}
				}

				/* �s���֑� */
				if( m_bKinsokuTail
				 && (nMaxLineSize - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* �s���֑����� && �s���t�� && �s���łȂ�����(�����ɋ֑����Ă��܂�����) */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					int nCharChars3 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosTail( nMaxLineSize - nPosX, nCharChars2, nCharChars3 )
						&& IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						m_nLineTypeBot = nCOMMENTMODE;
						nCOMMENTMODE_Prev = nCOMMENTMODE;
						nBgn = nPos;
						// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
						nPosX = nIndent = (this->*getIndentOffset)( m_pLayoutBot );
						pLayoutCalculated = m_pLayoutBot;
					}
				}
				//@@@ 2002.04.08 MIK end
			}	//if( KINSOKU_TYPE_NONE != nKinsokuTyoe ) �֑�������

			//@@@ 2002.09.22 YAZAKI
			bool bGotoSEARCH_START = CheckColorMODE( nCOMMENTMODE, nCOMMENTEND, nPos, nLineLen, pLine, bDispSSTRING, bDispWSTRING );
			if ( bGotoSEARCH_START )
				goto SEARCH_START;
			
			if( pLine[nPos] == TAB ){
				//	Sep. 23, 2002 genta ��������������̂Ŋ֐����g��
				int nCharChars2 = GetActualTabSpace( nPosX );
				if( nPosX + nCharChars2 > nMaxLineSize ){
					AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
					m_nLineTypeBot = nCOMMENTMODE;
					nCOMMENTMODE_Prev = nCOMMENTMODE;
					nBgn = nPos;
					// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
					nPosX = nIndent = (this->*getIndentOffset)( m_pLayoutBot );
					pLayoutCalculated = m_pLayoutBot;
					continue;
				}
				nPosX += nCharChars2;
				nCharChars2 = 1;
				nPos+= nCharChars2;
			}else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
				if( 0 == nCharChars2 ){
					nCharChars2 = 1;
					break;	//@@@ 2002.04.16 MIK
				}
				if( nPosX + nCharChars2 > nMaxLineSize ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							m_nLineTypeBot = nCOMMENTMODE;
							nCOMMENTMODE_Prev = nCOMMENTMODE;
							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*getIndentOffset)( m_pLayoutBot );
							pLayoutCalculated = m_pLayoutBot;
							continue;
						}	//@@@ 2002.04.14 MIK
					}
				}
				nPos+= nCharChars2;
				nPosX += nCharChars2;
			}
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			AddLineBottom( CreateLayout(pCDocLine, nLineNum, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
			m_nLineTypeBot = nCOMMENTMODE;
			nCOMMENTMODE_Prev = nCOMMENTMODE;
		}
		nLineNum++;
		if( NULL != hwndProgress && 0 < nAllLineNum && 0 == ( nLineNum % 1024 ) ){
			::PostMessage( hwndProgress, PBM_SETPOS, nLineNum * 100 / nAllLineNum , 0 );
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( NULL ) ){
				return;
			}
		}
//		pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->m_pNext;;
// 2002/03/13 novice
		if( nCOMMENTMODE_Prev == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
			nCOMMENTMODE_Prev = COLORIDX_TEXT;
		}
		nCOMMENTMODE = nCOMMENTMODE_Prev;
		nCOMMENTEND = 0;
	}
	m_nPrevReferLine = 0;
	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;

	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) ){
			return;
		}
	}
	return;
}





/*!
	�w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g����
	
	@date 2002.10.07 YAZAKI rename from "DoLayout3_New"
	@date 2004.04.03 Moca TAB���g����Ɛ܂�Ԃ��ʒu�������̂�h�����߁C
		nPosX���C���f���g���܂ޕ���ێ�����悤�ɕύX�DnMaxLineSize��
		�Œ�l�ƂȂ������C�����R�[�h�̒u�������͔����čŏ��ɒl��������悤�ɂ����D

	@note 2004.04.03 Moca
		DoLayout�Ƃ͈���ă��C�A�E�g��񂪃��X�g���Ԃɑ}������邽�߁C
		�}�����m_nLineTypeBot�փR�����g���[�h���w�肵�Ă͂Ȃ�Ȃ�
		����ɍŏI�s�̃R�����g���[�h���I���ԍۂɊm�F���Ă���D
*/
int CLayoutMgr::DoLayout_Range(
			CLayout* pLayoutPrev,
			int		nLineNum,
			int		nDelLogicalLineFrom,
			int		nDelLogicalColFrom,
			int		nCurrentLineType,
			int*	pnExtInsLineNum,
			BOOL	bDispSSTRING,	/* �V���O���N�H�[�e�[�V�����������\������ */
			BOOL	bDispWSTRING	/* �_�u���N�H�[�e�[�V�����������\������ */
)
{
	int			nLineNumWork;
	int			nLineLen;
	int			nCurLine;
	CDocLine*	pCDocLine;
	const char* pLine;
	int			nBgn;
	//!	��������̈ʒu(offset)
	int			nPos;
	/*	�\�����X�ʒu
		2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
	*/
	int			nPosX;
	CLayout*	pLayout;
	int			nModifyLayoutLinesNew;
	int			nCOMMENTMODE;
	int			nCOMMENTMODE_Prev;
	int			nCOMMENTEND;
	bool		bNeedChangeCOMMENTMODE = false;	//@@@ 2002.09.23 YAZAKI bAdd�𖼏̕ύX
	int			nWordBgn;
	int			nWordLen;
	int			nKinsokuType;	//@@@ 2002.04.20 MIK

	int			nIndent;			//	�C���f���g��
	CLayout*	pLayoutCalculated;	//	�C���f���g���v�Z�ς݂�CLayout.
	
	//	2004.04.09 genta �֐����ł͒l���ω����Ȃ��̂Ń��[�v�̊O�ɏo��
	int			nMaxLineSize= m_nMaxLineSize;

	nLineNumWork = 0;
	*pnExtInsLineNum = 0;
	//	Jun. 22, 2005 Moca �Ō�̍s[EOF�̂�]�̂��߂ɂ�linenum==0�ł��������K�v
	//if( 0 == nLineNum ){
	//	return 0;
	//}
	pLayout = pLayoutPrev;
	if( NULL == pLayout ){
		nCurLine = 0;
	}else{
		nCurLine = pLayout->m_nLinePhysical + 1;
	}
	nCOMMENTMODE = nCurrentLineType;
	nCOMMENTMODE_Prev = nCOMMENTMODE;

//	pLine = m_pcDocLineMgr->GetLineStr( nCurLine, &nLineLen );
	pCDocLine = m_pcDocLineMgr->GetLineInfo( nCurLine );



//	if( nCOMMENTMODE_Prev == 1 ){	/* �s�R�����g�ł��� */
//		nCOMMENTMODE_Prev = 0;
//	}
//	nCOMMENTMODE = nCOMMENTMODE_Prev;
	nCOMMENTEND = 0;

	nModifyLayoutLinesNew = 0;

	while( NULL != pCDocLine ){
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );
		nPosX = 0;
		nBgn = 0;
		nPos = 0;
		nWordBgn = 0;
		nWordLen = 0;
		nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK

		int	nEol = pCDocLine->m_cEol.GetLen();
		int nEol_1 = nEol - 1;
		if( 0 >	nEol_1 ){
			nEol_1 = 0;
		}

		nIndent = 0;				//	�C���f���g��
		pLayoutCalculated = pLayout;	//	�C���f���g���v�Z�ς݂�CLayout.

		while( nPos < nLineLen - nEol_1 ){
			//	�C���f���g���̌v�Z�R�X�g�������邽�߂̕���
			if ( pLayout && 
				 pLayout != pLayoutCalculated ){
				//	�v�Z
				//	Oct, 1, 2002 genta Indent�T�C�Y���擾����悤�ɕύX
				nIndent = (this->*getIndentOffset)( pLayout );
				// 2004.03.28 Moca nMaxLineSize���������@���ƁA�^�u���̌v�Z������Ȃ��̂ŁAnPosX�̏����l��nIndent�ɂ���
				//	nMaxLineSize = m_nMaxLineSize - nIndent;
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
					if( nKinsokuType == KINSOKU_TYPE_KINSOKU_KUTO
					 && nPos == nWordBgn + nWordLen )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol ) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
						{
							//@@@ 2002.09.23 YAZAKI �œK��
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
								(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
						}
					}

					nWordLen = 0;
					nKinsokuType = KINSOKU_TYPE_NONE;	//@@@ 2002.04.20 MIK
				}
			}
			else
			{
			
				/* ���[�h���b�v���� */
				if( m_bWordWrap	/* �p�����[�h���b�v������ */
				 && nKinsokuType == KINSOKU_TYPE_NONE )
				{
					/* �p�P��̐擪�� */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( nPos >= nBgn &&
						nCharChars == 1 &&
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						/* �L�[���[�h������̏I�[��T�� */
						int	i;
						// 2005-08-27 D.S.Koba �ȗ���
						for( i = nPos + 1; i < nLineLen; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars2 ){
								nCharChars2 = 1;
							}
							if( nCharChars2 != 1 || !IS_KEYWORD_CHAR( pLine[i] ) ){
								break;
							}
						}
						nWordBgn = nPos;
						nWordLen = i - nPos;
						nKinsokuType = KINSOKU_TYPE_WORDWRAP;	//@@@ 2002.04.20 MIK

						if( nPosX + i - nPos >= nMaxLineSize
						 && nPos - nBgn > 0
						){
							//@@@ 2002.09.23 YAZAKI �œK��
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
								(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
//?							continue;
						}
					}
				}

				//@@@ 2002.04.07 MIK start
				/* ��Ǔ_�̂Ԃ炳�� */
				if( m_bKinsokuKuto
				 && (nMaxLineSize - nPosX < 2)
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );

					if( IsKinsokuPosKuto( nMaxLineSize - nPosX, nCharChars2 )
						&& IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_KUTO;
					}
				}

				/* �s���֑� */
				if( m_bKinsokuHead
				 && (nMaxLineSize - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					int nCharChars3 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosHead( nMaxLineSize - nPosX, nCharChars2, nCharChars3 )
					 && IsKinsokuHead( &pLine[nPos+nCharChars2], nCharChars3 )
					 && ! IsKinsokuHead( &pLine[nPos], nCharChars2 )	//1�����O���s���֑��łȂ�
					 && ! IsKinsokuKuto( &pLine[nPos], nCharChars2 ) )	//��Ǔ_�łȂ�
					{
						//nPos += nCharChars2 + nCharChars3; nPosX += nCharChars2 + nCharChars3;
						nWordBgn = nPos;
						nWordLen = nCharChars2 + nCharChars3;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_HEAD;
						//@@@ 2002.09.23 YAZAKI �œK��
						if( bNeedChangeCOMMENTMODE ){
							pLayout = pLayout->m_pNext;
							pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
							(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
						}
						else {
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						}
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
						nPosX = nIndent = (this->*getIndentOffset)( pLayout );
						pLayoutCalculated = pLayout;
						if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
							( nDelLogicalLineFrom < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}

				/* �s���֑� */
				if( m_bKinsokuTail
				 && (nMaxLineSize - nPosX < 4)
				 && ( nPosX > nIndent )	//	2004.04.09 nPosX�̉��ߕύX�̂��߁C�s���`�F�b�N���ύX
				 && (nKinsokuType == KINSOKU_TYPE_NONE) )
				{	/* �s���֑����� && �s���t�� && �s���łȂ�����(�����ɋ֑����Ă��܂�����) */
					// 2005-09-02 D.S.Koba GetSizeOfChar
					int nCharChars2 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					int nCharChars3 = CMemory::GetSizeOfChar( pLine, nLineLen, nPos+nCharChars2 );

					if( IsKinsokuPosTail( nMaxLineSize - nPosX, nCharChars2, nCharChars3 )
						&& IsKinsokuTail( &pLine[nPos], nCharChars2 ) )
					{
						//nPos += nCharChars2; nPosX += nCharChars2;
						nWordBgn = nPos;
						nWordLen = nCharChars2;
						nKinsokuType = KINSOKU_TYPE_KINSOKU_TAIL;
						//@@@ 2002.09.23 YAZAKI �œK��
						if( bNeedChangeCOMMENTMODE ){
							pLayout = pLayout->m_pNext;
							pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
							(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
						}
						else {
							pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
						}
						nCOMMENTMODE_Prev = nCOMMENTMODE;

						nBgn = nPos;
						// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
						nPosX = nIndent = (this->*getIndentOffset)( pLayout );
						pLayoutCalculated = pLayout;
						if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
							( nDelLogicalLineFrom < nCurLine )
						){
							(nModifyLayoutLinesNew)++;;
						}
					}
				}
				//@@@ 2002.04.08 MIK end
			}	// if( nKinsokuType != KINSOKU_TYPE_NONE )

			//@@@ 2002.09.22 YAZAKI
			bool bGotoSEARCH_START = CheckColorMODE( nCOMMENTMODE, nCOMMENTEND, nPos, nLineLen, pLine, bDispSSTRING, bDispWSTRING );
			if ( bGotoSEARCH_START )
				goto SEARCH_START;

			int nCharChars;
			if( pLine[nPos] == TAB ){
				//	Sep. 23, 2002 genta ��������������̂Ŋ֐����g��
				nCharChars = GetActualTabSpace( nPosX );
				if( nPosX + nCharChars > nMaxLineSize ){
					//@@@ 2002.09.23 YAZAKI �œK��
					if( bNeedChangeCOMMENTMODE ){
						pLayout = pLayout->m_pNext;
						pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
						(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
					}
					else {
						pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
					}
					nCOMMENTMODE_Prev = nCOMMENTMODE;

					nBgn = nPos;
					// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
					nPosX = nIndent = (this->*getIndentOffset)( pLayout );
					pLayoutCalculated = pLayout;
					if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
						( nDelLogicalLineFrom < nCurLine )
					){
						(nModifyLayoutLinesNew)++;;
					}
					continue;
				}
				nPos++;
			}else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
				if( 0 == nCharChars ){
					nCharChars = 1;
					break;	//@@@ 2002.04.16 MIK
				}
				if( nPosX + nCharChars > nMaxLineSize ){
					if( nKinsokuType != KINSOKU_TYPE_KINSOKU_KUTO )
					{
						if( ! (m_bKinsokuRet && (nPos == nLineLen - nEol) && nEol) )	//���s�������Ԃ牺����		//@@@ 2002.04.14 MIK
						{	//@@@ 2002.04.14 MIK
							//@@@ 2002.09.23 YAZAKI �œK��
							if( bNeedChangeCOMMENTMODE ){
								pLayout = pLayout->m_pNext;
								pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
								(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
							}
							else {
								pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
							}
							nCOMMENTMODE_Prev = nCOMMENTMODE;

							nBgn = nPos;
							// 2004.03.28 Moca nPosX�̓C���f���g�����܂ނ悤�ɕύX(TAB�ʒu�����̂���)
							nPosX = nIndent = (this->*getIndentOffset)( pLayout );
							pLayoutCalculated = pLayout;
							if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
								( nDelLogicalLineFrom < nCurLine )
							){
								(nModifyLayoutLinesNew)++;;
							}
							continue;
						}
					}
				}
				nPos+= nCharChars;
			}

			nPosX += nCharChars;
		}
		if( nPos - nBgn > 0 ){
// 2002/03/13 novice
			if( nCOMMENTMODE == COLORIDX_COMMENT ){	/* �s�R�����g�ł��� */
				nCOMMENTMODE = COLORIDX_TEXT;
			}
			//@@@ 2002.09.23 YAZAKI �œK��
			if( bNeedChangeCOMMENTMODE ){
				pLayout = pLayout->m_pNext;
				pLayout->m_nTypePrev = nCOMMENTMODE_Prev;
				(*pnExtInsLineNum)++;								//	�ĕ`�悵�Ăق����s��+1
			}
			else {
				pLayout = InsertLineNext( pLayout, CreateLayout(pCDocLine, nCurLine, nBgn, nPos - nBgn, nCOMMENTMODE_Prev, nIndent) );
			}
			nCOMMENTMODE_Prev = nCOMMENTMODE;

			if( ( nDelLogicalLineFrom == nCurLine && nDelLogicalColFrom < nPos ) ||
				( nDelLogicalLineFrom < nCurLine )
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
			 && ( nCOMMENTMODE_Prev != pLayout->m_pNext->m_nTypePrev )
			){
				//	COMMENTMODE���قȂ�s�������܂����̂ŁA���̍s�����̍s�ƍX�V���Ă����܂��B
				bNeedChangeCOMMENTMODE = true;
			}else{
				break;	//	while( NULL != pCDocLine ) �I��
			}
		}
//		pLine = m_pcDocLineMgr->GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->m_pNext;
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
bool CLayoutMgr::IsKinsokuHead( const char *pLine, int length )
{
	const unsigned char	*p;

	if(      length == 1 ) p = (const unsigned char *)m_pszKinsokuHead_1;
	else if( length == 2 ) p = (const unsigned char *)m_pszKinsokuHead_2;
	else return false;

	if( ! p ) return false;

	for( ; *p; p += length )
	{
		if( memcmp( pLine, p, length ) == 0 ) return true;
	}

	return false;
}

/*!
	�s���֑������ɊY�����邩�𒲂ׂ�D

	@param[in] pLine ���ׂ镶���ւ̃|�C���^
	@param[in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool CLayoutMgr::IsKinsokuTail( const char *pLine, int length )
{
	const unsigned char	*p;
	
	if(      length == 1 ) p = (const unsigned char *)m_pszKinsokuTail_1;
	else if( length == 2 ) p = (const unsigned char *)m_pszKinsokuTail_2;
	else return false;
	
	if( ! p ) return false;

	for( ; *p; p += length )
	{
		if( memcmp( pLine, p, length ) == 0 ) return true;
	}

	return false;
}

/*!
	��Ǔ_��

	@param[in] c1 ���ׂ镶��1�o�C�g��
	@param[in] c2 ���ׂ镶��2�o�C�g��
	@retval true ��Ǔ_�ł���
	@retval false ��Ǔ_�łȂ�
*/

bool CLayoutMgr::IsKutoTen( unsigned char c1, unsigned char c2 )
{
	static const char	*KUTOTEN_1 = "��,.";
	static const char	*KUTOTEN_2 = "�B�A�C�D";
	unsigned const char	*p;

	if( c2 )	//�S�p
	{
		for( p = (const unsigned char *)KUTOTEN_2; *p; p += 2 )
		{
			if( *p == c1 && *(p + 1) == c2 ) return true;
		}
	}
	else		//���p
	{
		for( p = (const unsigned char *)KUTOTEN_1; *p; p++ )
		{
			if( *p == c1 ) return true;
		}
	}

	return false;
}

/*!
	�֑��Ώۋ�Ǔ_�ɊY�����邩�𒲂ׂ�D

	@param[in] pLine ���ׂ镶���ւ̃|�C���^
	@param[in] length ���Y�ӏ��̕����T�C�Y
	@retval true �֑������ɊY��
	@retval false �֑������ɊY�����Ȃ�
*/
bool CLayoutMgr::IsKinsokuKuto( const char *pLine, int length )
{
	const unsigned char	*p;
	
	if(      length == 1 ) p = (const unsigned char *)m_pszKinsokuKuto_1;
	else if( length == 2 ) p = (const unsigned char *)m_pszKinsokuKuto_2;
	else return false;
	
	if( ! p ) return false;

	for( ; *p; p += length )
	{
		if( memcmp( pLine, p, length ) == 0 ) return true;
	}

	return false;
}

///*!
//	@date 2005-08-20 D.S.Koba DoLayout()��DoLayout_Range()���番��	
//*/
//bool CLayoutMgr::IsKinsokuPosKuto(const int nMaxLineSize, const int nPosX, const int nCharChars)
//{
//	switch( nMaxLineSize - nPosX )
//	{
//	case 1:	// 1�����O
//		if( nCharChars == 2 ){
//			return true;
//		}
//		break;
//	case 0:	// 
//		if( nCharChars == 1 || nCharChars == 2 ){
//			return true;
//		}
//		break;
//	}
//	return false;
//}

/*!
	@param[in] nRest �s�̎c�蕶����
	@param[in] nCharChars ���݈ʒu�̕����T�C�Y
	@param[in] nCharChars2 ���݈ʒu�̎��̕����T�C�Y

	@date 2005-08-20 D.S.Koba DoLayout()��DoLayout_Range()���番��
*/
bool CLayoutMgr::IsKinsokuPosHead(const int nRest,
								  const int nCharChars, const int nCharChars2)
{
	switch( nRest )
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
		if( nCharChars == 2 && nCharChars2 == 2 ){
			return true;
		}
		break;
	case 2:	// 2�����O
		if( nCharChars == 2 /*&& nCharChars2 > 0*/ ){
			return true;
		}
		else if( nCharChars == 1 && nCharChars2 == 2 ){
			return true;
		}
		break;
	case 1:	// 1�����O
		if( nCharChars == 1 /*&& nCharChars2 > 0*/ ){
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

	@date 2005-08-20 D.S.Koba DoLayout()��DoLayout_Range()���番��
*/
bool CLayoutMgr::IsKinsokuPosTail(const int nRest,
								  const int nCharChars, const int nCharChars2)
{
	switch( nRest )
	{
	case 3:	// 3�����O
		if( nCharChars == 2 && nCharChars2 == 2){
			// "�i��": "��"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	case 2:	// 2�����O
		if( nCharChars == 2 ){
			// "�i��": "��"�Ő܂�Ԃ��̂Ƃ�
			return true;
		}else if( nCharChars == 1 && nCharChars2 == 2){
			// "(��": "��"��2�o�C�g�ڂŐ܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	case 1:	// 1�����O
		if( nCharChars == 1 ){
			// "(��": "��"�Ő܂�Ԃ��̂Ƃ�
			return true;
		}
		break;
	}
	return false;
}

int CLayoutMgr::Match_Quote( char szQuote, int nPos, int nLineLen, const char* pLine )
{
	int nCharChars;
	int i;
	for( i = nPos; i < nLineLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pLine, nLineLen, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if(	m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && pLine[i] == '\\' ){
				++i;
			}else
			if( 1 == nCharChars && pLine[i] == szQuote ){
				return i + 1;
			}
		}else
		if(	m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
			if( 1 == nCharChars && pLine[i] == szQuote ){
				if( i + 1 < nLineLen && pLine[i + 1] == szQuote ){
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

bool CLayoutMgr::CheckColorMODE( int &nCOMMENTMODE, int &nCOMMENTEND, int nPos, int nLineLen, const char* pLine, BOOL bDispSSTRING, BOOL bDispWSTRING )
{
	switch( nCOMMENTMODE ){
	case COLORIDX_TEXT: // 2002/03/13 novice
		if( m_cLineComment.Match( nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_COMMENT;	/* �s�R�����g�ł��� */ // 2002/03/13 novice
		}else
		if( m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK1;	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 0, nPos + (int)lstrlen( m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		}else
		if( m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine ) ){
			nCOMMENTMODE = COLORIDX_BLOCK2;	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 1, nPos + (int)lstrlen( m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );
//#endif
		}else
		if( bDispSSTRING && /* �V���O���N�H�[�e�[�V�����������\������ */
			pLine[nPos] == '\''
		){
			nCOMMENTMODE = COLORIDX_SSTRING;	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
			/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
			nCOMMENTEND = Match_Quote( '\'', nPos + 1, nLineLen, pLine );
		}else
		if( pLine[nPos] == '"' &&
			bDispWSTRING	/* �_�u���N�H�[�e�[�V�����������\������ */
		){
			nCOMMENTMODE = COLORIDX_WSTRING;	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
			/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
			nCOMMENTEND = Match_Quote( '"', nPos + 1, nLineLen, pLine );
		}
		break;
	case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
		break;
	case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
			nCOMMENTEND = m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
//#endif
	case COLORIDX_SSTRING:	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
			nCOMMENTEND = Match_Quote( '\'', nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	case COLORIDX_WSTRING:	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
		if( 0 == nCOMMENTEND ){
			/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
			nCOMMENTEND = Match_Quote( '"', nPos, nLineLen, pLine );
		}else
		if( nPos == nCOMMENTEND ){
			nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
			return true;
		}
		break;
	}
	return false;
}

/*!
	@brief �s�̒������v�Z���� (2�s�ڈȍ~�̎���������)
	
	���������s��Ȃ��̂ŁC���0��Ԃ��D
	�����͎g��Ȃ��D
	
	@return 1�s�̕\�������� (���0)
	
	@author genta
	@date 2002.10.01
*/
int CLayoutMgr::getIndentOffset_Normal( CLayout* )
{
	return 0;
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
int CLayoutMgr::getIndentOffset_Tx2x( CLayout* pLayoutPrev )
{
	//	�O�̍s�������Ƃ��́A�C���f���g�s�v�B
	if ( pLayoutPrev == NULL ) return 0;

	int nIpos = pLayoutPrev->GetIndent();

	//	�O�̍s���܂�Ԃ��s�Ȃ�΂���ɍ��킹��
	if( pLayoutPrev->m_nOffset > 0 )
		return nIpos;
	
	CMemoryIterator<CLayout> it( pLayoutPrev, m_nTabSpace );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndexDelta() == 1 && it.getCurrentChar() == TAB ){
			nIpos = it.getColumn() + it.getColumnDelta();
		}
		it.addDelta();
	}
	if ( m_nMaxLineSize - nIpos < 6 ){
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
int CLayoutMgr::getIndentOffset_LeftSpace( CLayout* pLayoutPrev )
{
	//	�O�̍s�������Ƃ��́A�C���f���g�s�v�B
	if ( pLayoutPrev == NULL ) return 0;

	//	�C���f���g�̌v�Z
	int nIpos = pLayoutPrev->GetIndent();
	
	//	Oct. 5, 2002 genta
	//	�܂�Ԃ���3�s�ڈȍ~��1�O�̍s�̃C���f���g�ɍ��킹��D
	if( pLayoutPrev->m_nOffset > 0 )
		return nIpos;
	
	//	2002.10.07 YAZAKI �C���f���g�̌v�Z
	CMemoryIterator<CLayout> it( pLayoutPrev, m_nTabSpace );

	//	Jul. 20, 2003 genta �����C���f���g�ɏ���������ɂ���
	bool bZenSpace = m_pcEditDoc->GetDocumentAttribute().m_bAutoIndent_ZENSPACE != FALSE ? 1 : 0;
	const char* szSpecialIndentChar = m_pcEditDoc->GetDocumentAttribute().m_szIndentChars;
	while( !it.end() ){
		it.scanNext();
		if (( it.getIndexDelta() == 1 && (it.getCurrentChar() == TAB || it.getCurrentChar() == ' ') ) ||
			//	Jul. 20, 2003 genta �S�p�X�y�[�X�Ή�
			( bZenSpace && it.getIndexDelta() == 2 &&
				it.getCurrentChar() == (char)0x81 && it.getCurrentPos()[1] == (char)0x40 ))
		{
			//	�C���f���g�̃J�E���g���p������
		}
		//	Jul. 20, 2003 genta �C���f���g�Ώە���
		else if( szSpecialIndentChar[0] != '\0' ){
			unsigned char buf[3]; // �����̒�����1 or 2
			memcpy( buf, it.getCurrentPos(), it.getIndexDelta() );
			buf[ it.getIndexDelta() ] = '\0';
			if( NULL != _mbsstr( (const unsigned char*)szSpecialIndentChar, buf )){
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
	if ( m_nMaxLineSize - nIpos < 6 ){
		nIpos = pLayoutPrev->GetIndent();	//	������߂�
	}
	return nIpos;	//	�C���f���g
}

/*[EOF]*/
