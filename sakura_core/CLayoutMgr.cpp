//	$Id$
/*!	@file
	@brief �e�L�X�g�̃��C�A�E�g���Ǘ�

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CLayoutMgr.h"

/*** for TRACE()****
#include <afx.h>
***/
#include "charcode.h"
#include "debug.h"
#include <commctrl.h>
#include "CRunningTimer.h"
#include "CLayout.h"/// 2002/2/10 aroka
#include "CDocLine.h"/// 2002/2/10 aroka
#include "CDocLineMgr.h"/// 2002/2/10 aroka
#include "CMemory.h"/// 2002/2/10 aroka
#include "etc_uty.h" // Oct. 5, 2002 genta





/*
|| �R���X�g���N�^
*/
CLayoutMgr::CLayoutMgr()
: m_cLineComment(), m_cBlockComment(),
	//	Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
	getIndentOffset( &CLayoutMgr::getIndentOffset_Normal )	//	Oct. 1, 2002 genta
{
	m_pcDocLineMgr = NULL;
	m_nMaxLineSize = 0;
	m_bWordWrap = TRUE;		/* �p�����[�h���b�v������ */
	m_nTabSpace = 8;		/* TAB�����X�y�[�X */
	m_nStringType = 0;		/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
	m_bKinsokuHead = FALSE;		/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_bKinsokuTail = FALSE;		/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_bKinsokuRet  = FALSE;		/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	m_bKinsokuKuto = FALSE;		/* ��Ǔ_���Ԃ牺���� */	//@@@ 2002.04.17 MIK
	m_pszKinsokuHead_1 = NULL;	/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_pszKinsokuHead_2 = NULL;	/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_1 = NULL;	/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_2 = NULL;	/* �s���֑� */	//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1 = NULL;	/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK
	m_pszKinsokuKuto_2 = NULL;	/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK

	Init();
	return;
}


/*
|| �f�X�g���N�^
*/
CLayoutMgr::~CLayoutMgr()
{
	Empty();

	if( NULL != m_pszKinsokuHead_1 ){	/* �s���֑� */	//@@@ 2002.04.08 MIK
		delete [] m_pszKinsokuHead_1;
		m_pszKinsokuHead_1 = NULL;
	}
	if( NULL != m_pszKinsokuHead_2 ){	/* �s���֑� */	//@@@ 2002.04.08 MIK
		delete [] m_pszKinsokuHead_2;
		m_pszKinsokuHead_2 = NULL;
	}
	if( NULL != m_pszKinsokuTail_1 ){	/* �s���֑� */	//@@@ 2002.04.08 MIK
		delete [] m_pszKinsokuTail_1;
		m_pszKinsokuTail_1 = NULL;
	}
	if( NULL != m_pszKinsokuTail_2 ){	/* �s���֑� */	//@@@ 2002.04.08 MIK
		delete [] m_pszKinsokuTail_2;
		m_pszKinsokuTail_2 = NULL;
	}
	if( NULL != m_pszKinsokuKuto_1 ){	/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK
		delete [] m_pszKinsokuKuto_1;
		m_pszKinsokuKuto_1 = NULL;
	}
	if( NULL != m_pszKinsokuKuto_2 ){	/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK
		delete [] m_pszKinsokuKuto_2;
		m_pszKinsokuKuto_2 = NULL;
	}

	return;
}


/*
|| ���C�A�E�g���̕ύX
*/
void CLayoutMgr::SetLayoutInfo(
	int		bDoRayout,
	HWND	hwndProgress,
	Types&	refType			/* �^�C�v�ʐݒ� */
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::SetLayoutInfo" );

	m_nMaxLineSize = refType.m_nMaxLineSize;
	m_bWordWrap		= refType.m_bWordWrap;		/* �p�����[�h���b�v������ */
	m_nTabSpace		= refType.m_nTabSpace;
	m_nStringType	= refType.m_nStringType;		/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */

	m_cLineComment = refType.m_cLineComment;	/* �s�R�����g�f���~�^ */	//@@@ 2002.09.22 YAZAKI
	m_cBlockComment = refType.m_cBlockComment;	/* �u���b�N�R�����g�f���~�^ */	//@@@ 2002.09.22 YAZAKI

	//	Oct. 1, 2002 genta �^�C�v�ɂ���ď����֐���ύX����
	//	���������Ă�����e�[�u���ɂ��ׂ�
	switch ( refType.m_nIndentLayout ){	/* �܂�Ԃ���2�s�ڈȍ~���������\�� */	//@@@ 2002.09.29 YAZAKI
		case 1:
			//	Nov. 16, 2002 �����o�[�֐��|�C���^�ɂ̓N���X�����K�v
			getIndentOffset = &CLayoutMgr::getIndentOffset_Tx2x;
			break;
		case 2:
			getIndentOffset = &CLayoutMgr::getIndentOffset_LeftSpace;
			break;
		default:
			getIndentOffset = &CLayoutMgr::getIndentOffset_Normal;
			break;
	}
	
	{	//@@@ 2002.04.08 MIK start
		unsigned char	*p, *q1, *q2, *k1, *k2;
		int	length;

		//��Ǔ_�̂Ԃ炳��
		m_bKinsokuKuto = refType.m_bKinsokuKuto;	/* ��Ǔ_�Ԃ炳�� */	//@@@ 2002.04.17 MIK
		if( NULL != m_pszKinsokuKuto_1 )
		{
			delete [] m_pszKinsokuKuto_1;
			m_pszKinsokuKuto_1 = NULL;
		}
		if( NULL != m_pszKinsokuKuto_2 )
		{
			delete [] m_pszKinsokuKuto_2;
			m_pszKinsokuKuto_2 = NULL;
		}
		//length = strlen( pszKinsokuHead ) + 1;
		//	Kuto_2="�B�A�C�D", Kuto_1="��,."
		length = 16;	//���ꂾ������Ώ\��
		m_pszKinsokuKuto_1 = new char[ length ];
		m_pszKinsokuKuto_2 = new char[ length ];
		k1 = (unsigned char *)m_pszKinsokuKuto_1;
		k2 = (unsigned char *)m_pszKinsokuKuto_2;
		memset( (void *)k1, 0, length );
		memset( (void *)k2, 0, length );
		//�f�[�^���͍s���֑������Őݒ肷��B

		//�s���֑�������1,2�o�C�g�����𕪂��ĊǗ�����B
		m_bKinsokuHead = refType.m_bKinsokuHead;
		if( NULL != m_pszKinsokuHead_1 )
		{
			delete [] m_pszKinsokuHead_1;
			m_pszKinsokuHead_1 = NULL;
		}
		if( NULL != m_pszKinsokuHead_2 )
		{
			delete [] m_pszKinsokuHead_2;
			m_pszKinsokuHead_2 = NULL;
		}
		length = strlen( refType.m_szKinsokuHead ) + 1;
		m_pszKinsokuHead_1 = new char[ length ];
		m_pszKinsokuHead_2 = new char[ length ];
		q1 = (unsigned char *)m_pszKinsokuHead_1;
		q2 = (unsigned char *)m_pszKinsokuHead_2;
		memset( (void *)q1, 0, length );
		memset( (void *)q2, 0, length );
		for( p = (unsigned char *)refType.m_szKinsokuHead; *p; p++ )
		{
			if( _IS_SJIS_1( *p ) )
			{
				if( IsKutoTen( *p, *(p+1) ) )	//��Ǔ_�͕ʊǗ�
				{
					*k2 = *p; k2++; p++;
					*k2 = *p; k2++;
					*k2 = 0;
				}
				else
				{
					*q2 = *p; q2++; p++;
					*q2 = *p; q2++;
					*q2 = 0;
				}
			}
			else
			{
				if( IsKutoTen( *p, 0 ) )	//��Ǔ_�͕ʊǗ�
				{
					*k1 = *p; k1++;
					*k1 = 0;
				}
				else
				{
					*q1 = *p; q1++;
					*q1 = 0;
				}
			}
		}

		//�s���֑�������1,2�o�C�g�����𕪂��ĊǗ�����B
		m_bKinsokuTail = refType.m_bKinsokuTail;
		if( NULL != m_pszKinsokuTail_1 )
		{
			delete [] m_pszKinsokuTail_1;
			m_pszKinsokuTail_1 = NULL;
		}
		if( NULL != m_pszKinsokuTail_2 )
		{
			delete [] m_pszKinsokuTail_2;
			m_pszKinsokuTail_2 = NULL;
		}
		length = strlen( refType.m_szKinsokuTail ) + 1;
		m_pszKinsokuTail_1 = new char[ length ];
		m_pszKinsokuTail_2 = new char[ length ];
		q1 = (unsigned char *)m_pszKinsokuTail_1;
		q2 = (unsigned char *)m_pszKinsokuTail_2;
		memset( (void *)q1, 0, length );
		memset( (void *)q2, 0, length );
		for( p = (unsigned char *)refType.m_szKinsokuTail; *p; p++ )
		{
			if( _IS_SJIS_1( *p ) )
			{
				*q2 = *p; q2++; p++;
				*q2 = *p; q2++;
				*q2 = 0;
			}
			else
			{
				*q1 = *p; q1++;
				*q1 = 0;
			}
		}

		m_bKinsokuRet = refType.m_bKinsokuRet;	/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK

	}	//@@@ 2002.04.08 MIK end

	if( bDoRayout ){
		DoLayout( hwndProgress, refType.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp, refType.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp );
	}

	return;
}


/*
||
|| �s�f�[�^�Ǘ��N���X�̃|�C���^�����������܂�
||
*/
void CLayoutMgr::Create( CDocLineMgr* pcDocLineMgr )
{
	Init();
	m_pcDocLineMgr = pcDocLineMgr;
	return;
}



void CLayoutMgr::Init()
{
	m_pLayoutTop = NULL;
	m_pLayoutBot = NULL;
	m_nPrevReferLine = 0;
	m_pLayoutPrevRefer = NULL;
//	m_pLayoutCurrent = NULL;
	m_nLines = 0;			/* �S�����s�� */
//	m_pszLineComment = NULL;			/* �s�R�����g�f���~�^ */
//	m_pszBlockCommentFrom = NULL;		/* �u���b�N�R�����g�f���~�^(From) */
//	m_pszBlockCommentTo = NULL;			/* �u���b�N�R�����g�f���~�^(To) */
	return;
}



void CLayoutMgr::Empty()
{
	CLayout* pLayout;
	CLayout* pLayoutNext;
	pLayout = m_pLayoutTop;
	while( NULL != pLayout ){
		pLayoutNext = pLayout->m_pNext;
		delete pLayout;
		pLayout = pLayoutNext;
	}
	return;
}


/*
|| �w�肳�ꂽ�����s�̃��C�A�E�g�����擾
|| �����s�͂O�I���W��
*/
CLayout* CLayoutMgr::Search( int nLineNum )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::Search()" );
//#endif
	CLayout*	pLayout;
	int			nCount;
	if( 0 == m_nLines ){
		return NULL;
	}
	if( nLineNum >= m_nLines ){
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
//			pLayout = pLayout->m_pNext;
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
//			pLayout = pLayout->m_pPrev;
//			nCount--;
//		}
//	}

	/*+++++++�킸���ɍ�����+++++++*/
	if( m_pLayoutPrevRefer == NULL ){
		if( nLineNum < (m_nLines / 2) ){
			nCount = 0;
			pLayout = m_pLayoutTop;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pNext;
				nCount++;
			}
		}else{
			nCount = m_nLines - 1;
			pLayout = m_pLayoutBot;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pPrev;
				nCount--;
			}
		}
	}else{
		if( nLineNum == m_nPrevReferLine ){
			return m_pLayoutPrevRefer;
		}else
		if( nLineNum > m_nPrevReferLine ){
			nCount = m_nPrevReferLine + 1;
			pLayout = m_pLayoutPrevRefer->m_pNext;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pNext;
				nCount++;
			}
		}else{
			nCount = m_nPrevReferLine - 1;
			pLayout = m_pLayoutPrevRefer->m_pPrev;
			while( NULL != pLayout ){
				if( nLineNum == nCount ){
					m_pLayoutPrevRefer = pLayout;
					m_nPrevReferLine = nLineNum;
					return pLayout;
				}
				pLayout = pLayout->m_pPrev;
				nCount--;
			}
		}
	}
	return NULL;
}


//@@@ 2002.09.23 YAZAKI CLayout*���쐬����Ƃ���͕������āAInsertLineNext()�Ƌ��ʉ�
void CLayoutMgr::AddLineBottom( CLayout* pLayout )
{
	if(	0 == m_nLines ){
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

	if(	0 == m_nLines ){
		/* �� */
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
		m_pLayoutTop->m_pNext = NULL;
	}else
	if( NULL == pLayoutPrev ){
		/* �擪�ɑ}�� */
		m_pLayoutTop->m_pPrev = pLayout;
		pLayout->m_pPrev = NULL;
		pLayout->m_pNext = m_pLayoutTop;
		m_pLayoutTop = pLayout;
	}else
	if( NULL == pLayoutPrev->m_pNext ){
		/* �Ō�ɑ}�� */
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		pLayout->m_pNext = NULL;
		m_pLayoutBot = pLayout;
	}else{
		/* �r���ɑ}�� */
		pLayoutNext = pLayoutPrev->m_pNext;
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
*/
CLayout* CLayoutMgr::CreateLayout( CDocLine* pCDocLine, int nLine, int nOffset, int nLength, int nTypePrev, int nIndent )
{
	CLayout* pLayout = new CLayout;
	pLayout->m_pCDocLine = pCDocLine;

	pLayout->m_nLinePhysical = nLine;
	pLayout->m_nOffset = nOffset;
	pLayout->m_nLength = nLength;
	pLayout->m_nTypePrev = nTypePrev;
	pLayout->m_nIndent = nIndent;

	if( EOL_NONE == pCDocLine->m_cEol ){
		pLayout->m_cEol.SetType( EOL_NONE );/* ���s�R�[�h�̎�� */
	}else{
		if( pLayout->m_nOffset + pLayout->m_nLength >
			pCDocLine->m_pLine->GetLength() - pCDocLine->m_cEol.GetLen()
		){
			pLayout->m_cEol = pCDocLine->m_cEol;/* ���s�R�[�h�̎�� */
		}else{
			pLayout->m_cEol = EOL_NONE;/* ���s�R�[�h�̎�� */
		}
	}
	return pLayout;
}


/*
|| �w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver0

	@date 2002/2/10 aroka CMemory�ύX
*/
const char* CLayoutMgr::GetLineStr( int nLine, int* pnLineLen )
{
	CLayout* pLayout;
	if( NULL == ( pLayout = Search( nLine )	) ){
		return NULL;
	}
	*pnLineLen = pLayout->m_nLength;
	return pLayout->m_pCDocLine->m_pLine->GetPtr() + pLayout->m_nOffset;
}

/*!	�w�肳�ꂽ�����s�̃f�[�^�ւ̃|�C���^�Ƃ��̒�����Ԃ� Ver1
	@date 2002/03/24 YAZAKI GetLineStr( int nLine, int* pnLineLen )�Ɠ�������ɕύX�B
*/
const char* CLayoutMgr::GetLineStr( int nLine, int* pnLineLen, const CLayout** ppcLayoutDes )
{
#if 0
	const CLayout* pLayout;
	const char* pData;
	int nDataLen;
	if( NULL == ( pLayout = Search( nLine )	) ){
		*ppcLayoutDes = pLayout;
		return NULL;
	}
	pData = m_pcDocLineMgr->GetLineStr( pLayout->m_nLinePhysical, &nDataLen );
	*pnLineLen = pLayout->m_nLength;
	*ppcLayoutDes = pLayout;
	return pData + pLayout->m_nOffset;
#endif
	if( NULL == ( (*ppcLayoutDes) = Search( nLine )	) ){
		return NULL;
	}
	*pnLineLen = (*ppcLayoutDes)->m_nLength;
	return (*ppcLayoutDes)->m_pCDocLine->m_pLine->GetPtr() + (*ppcLayoutDes)->m_nOffset;
}

/*
|| �w�肳�ꂽ�ʒu�����C�A�E�g�s�̓r���̍s�����ǂ������ׂ�

	@date 2002/4/27 MIK
*/
bool CLayoutMgr::IsEndOfLine( int nLine, int nPos )
{
	CLayout* pLayout;

	if( NULL == ( pLayout = Search( nLine )	) )
	{
		return false;
	}

	if( EOL_NONE == pLayout->m_cEol.GetType() )
	{	/* ���̍s�ɉ��s�͂Ȃ� */
		/* ���̍s�̍Ōォ�H */
		if( nPos == pLayout->m_nLength ) return true;
	}

	return false;
}



/*!	�s�������폜

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CLayoutMgr::DeleteData_CLayoutMgr(
		int			nLineNum,
		int			nDelPos,
		int			nDelLen,
		int			*pnModifyLayoutLinesOld,
		int			*pnModifyLayoutLinesNew,
		int			*pnDeleteLayoutLines,
		CMemory&	cmemDeleted,			/* �폜���ꂽ�f�[�^ */
		BOOL		bDispSSTRING,	/* �V���O���N�H�[�e�[�V�����������\������ */
		BOOL		bDispWSTRING	/* �_�u���N�H�[�e�[�V�����������\������ */
//		BOOL		bUndo			/* Undo���삩�ǂ��� */
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::DeleteData_CLayoutMgr" );
#endif
	const char*	pLine;
	int			nLineLen;
	CLayout*	pLayout;
	CLayout*	pLayoutPrev;
//	CLayout*	pLayoutNext;
	CLayout*	pLayoutWork;
	int			nModLineOldFrom;	/* �e���̂������ύX�O�̍s(from) */
	int			nModLineOldTo;		/* �e���̂������ύX�O�̍s(to) */
	int			nDelLineOldFrom;	/* �폜���ꂽ�ύX�O�_���s(from) */
	int			nDelLineOldNum;		/* �폜���ꂽ�s�� */
	int			nRowNum;
	int			nAllLinesOld;
	int			nDelStartLogicalLine;
	int			nDelStartLogicalPos;
	int			nCurrentLineType;
	int			nAddInsLineNum;
	int			nLineWork;

	/* ���ݍs�̃f�[�^���擾 */
	pLine = GetLineStr( nLineNum, &nLineLen );
	if( NULL == pLine ){
		return;
	}
	pLayout = m_pLayoutPrevRefer;
	nDelStartLogicalLine = pLayout->m_nLinePhysical;
	nDelStartLogicalPos  = nDelPos + pLayout->m_nOffset;

//	pLayoutWork = pLayout;
//	do{
//		pLayoutWork = pLayoutWork->m_pNext;
//	}while( NULL != pLayoutWork && 0 != pLayoutWork->m_nOffset );
//	pLayoutNext = pLayoutWork;


	pLayoutWork = pLayout;
	nLineWork = nLineNum;
	while( 0 != pLayoutWork->m_nOffset ){
		pLayoutWork = pLayoutWork->m_pPrev;
		--nLineWork;
	}
	nCurrentLineType = pLayoutWork->m_nTypePrev;

	/* �e�L�X�g�̃f�[�^���폜 */
	m_pcDocLineMgr->DeleteData_CDocLineMgr(
		nDelStartLogicalLine, nDelStartLogicalPos,
		nDelLen, &nModLineOldFrom, &nModLineOldTo,
		&nDelLineOldFrom, &nDelLineOldNum, cmemDeleted
	);

//	DUMP();

	/*--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---*/
	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
	/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */
	nAllLinesOld = m_nLines;
	pLayoutPrev = DeleteLayoutAsLogical(
		pLayoutWork,
		nLineWork,

		nModLineOldFrom, nModLineOldTo,
		nDelStartLogicalLine, nDelStartLogicalPos,
		pnModifyLayoutLinesOld
	);

	/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
	/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
	/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
	ShiftLogicalLineNum( pLayoutPrev, -1 * nDelLineOldNum );

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_nLinePhysical;
		}
	}else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_nLinePhysical - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_nLinePhysical -
				pLayoutPrev->m_nLinePhysical - 1;
		}
	}

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	*pnModifyLayoutLinesNew = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		nDelStartLogicalLine, nDelStartLogicalPos,
		nCurrentLineType,
		&nAddInsLineNum,
		bDispSSTRING,	/* �V���O���N�H�[�e�[�V�����������\������ */
		bDispWSTRING	/* �_�u���N�H�[�e�[�V�����������\������ */
	);

	*pnDeleteLayoutLines = nAllLinesOld - m_nLines + nAddInsLineNum;
	return;
}






/*!	������}��

	@date 2002/03/24 YAZAKI bUndo�폜
*/
void CLayoutMgr::InsertData_CLayoutMgr(
		int			nLineNum,
		int			nInsPos,
		const char*	pInsData,
		int			nInsDataLen,
		int*		pnModifyLayoutLinesOld,
		int*		pnInsLineNum,		/* �}���ɂ���đ��������C�A�E�g�s�̐� */
		int*		pnNewLine,			/* �}�����ꂽ�����̎��̈ʒu�̍s */
		int*		pnNewPos,			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
		BOOL		bDispSSTRING,	/* �V���O���N�H�[�e�[�V�����������\������ */
		BOOL		bDispWSTRING	/* �_�u���N�H�[�e�[�V�����������\������ */
//		BOOL		bUndo			/* Undo���삩�ǂ��� */
)
{
	const char*	pLine;
	int			nLineLen;
	CLayout*	pLayout;
	CLayout*	pLayoutPrev;
//	CLayout*	pLayoutNext;
	CLayout*	pLayoutLast;
	CLayout*	pLayoutWork;
	int			nInsStartLogicalLine;
	int			nInsStartLogicalPos;
	int			nInsLineNum;
	int			nAllLinesOld;
	int			nRowNum;
	int			nNewLine;			/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int			nNewPos;			/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	int			nCurrentLineType;
	int			nAddInsLineNum;
	int			nLineWork;

//	MYTRACE( "CLayoutMgr::InsertData()\n" );

	/* ���ݍs�̃f�[�^���擾 */
	pLine = GetLineStr( nLineNum, &nLineLen );
	if( NULL == pLine ){
		if( 0 == nLineNum ){
			/* ��̃e�L�X�g�̐擪�ɍs�����ꍇ */
			pLayout = NULL;
			nLineWork = 0;
			nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
			nInsStartLogicalPos  = 0;
			nCurrentLineType = 0;
		}else{
			pLine = GetLineStr( m_nLines - 1, &nLineLen );
			pLayoutLast = m_pLayoutPrevRefer;
			if( ( nLineLen > 0 && ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF )) ||
				( nLineLen > 1 && ( pLine[nLineLen - 2] == CR || pLine[nLineLen - 2] == LF )) ){
				/* ��łȂ��e�L�X�g�̍Ō�ɍs�����ꍇ */
				pLayout = NULL;
				nLineWork = 0;
				nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
				nInsStartLogicalPos  = 0;
				nCurrentLineType = m_nLineTypeBot;
			}else{
				/* ��łȂ��e�L�X�g�̍Ō�̍s��ύX����ꍇ */
				nLineNum = m_nLines	- 1;
				nInsPos = nLineLen;
				pLayout = m_pLayoutPrevRefer;
				nLineWork = m_nPrevReferLine;


				nInsStartLogicalLine = pLayout->m_nLinePhysical;
				nInsStartLogicalPos  = nInsPos + pLayout->m_nOffset;
				nCurrentLineType = pLayout->m_nTypePrev;
			}
		}
	}else{
		pLayout = m_pLayoutPrevRefer;
		nLineWork = m_nPrevReferLine;


		nInsStartLogicalLine = pLayout->m_nLinePhysical;
		nInsStartLogicalPos  = nInsPos + pLayout->m_nOffset;
		nCurrentLineType = pLayout->m_nTypePrev;
	}

	if( NULL != pLayout ){
		pLayoutWork = pLayout;
		while( pLayoutWork != NULL && 0 != pLayoutWork->m_nOffset ){
			pLayoutWork = pLayoutWork->m_pPrev;
			nLineWork--;
		}
		if( NULL != pLayoutWork ){
			nCurrentLineType = pLayoutWork->m_nTypePrev;
		}else{
			nCurrentLineType = 0;
		}
	}


	/* �f�[�^�̑}�� */
	m_pcDocLineMgr->InsertData_CDocLineMgr(
		nInsStartLogicalLine,
		nInsStartLogicalPos,
		pInsData,
		nInsDataLen,
		&nInsLineNum,
		&nNewLine,
		&nNewPos
	);
//	MYTRACE( "nNewLine=%d nNewPos=%d \n", nNewLine, nNewPos );


	/*--- �ύX���ꂽ�s�̃��C�A�E�g�����Đ��� ---*/
	/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
	/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */
	nAllLinesOld = m_nLines;
	if( NULL != pLayout ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,

			nInsStartLogicalLine, nInsStartLogicalLine,
			nInsStartLogicalLine, nInsStartLogicalPos,
			pnModifyLayoutLinesOld
		);
	}else{
		pLayoutPrev = m_pLayoutBot;
	}

	/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
	/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
	/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
	if( NULL != pLine ){
		ShiftLogicalLineNum( pLayoutPrev, nInsLineNum );
	}

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_nLinePhysical;
		}
	}else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_nLinePhysical - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_nLinePhysical -
				pLayoutPrev->m_nLinePhysical - 1;
		}
	}

	/* �w�背�C�A�E�g�s�ɑΉ�����_���s�̎��̘_���s����w��_���s�������ă��C�A�E�g���� */
	DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		nInsStartLogicalLine, nInsStartLogicalPos,
		nCurrentLineType,
		&nAddInsLineNum,
		bDispSSTRING,	/* �V���O���N�H�[�e�[�V�����������\������ */
		bDispWSTRING	/* �_�u���N�H�[�e�[�V�����������\������ */
	);

	*pnInsLineNum = m_nLines - nAllLinesOld + nAddInsLineNum;

	/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
	pLayout = Search( nNewLine );
	XYLogicalToLayout( pLayout, nNewLine, nNewLine, nNewPos, pnNewLine, pnNewPos );
	return;
}






/* �_���s�̎w��͈͂ɊY�����郌�C�A�E�g�����폜���� */
/* �폜�����͈͂̒��O�̃��C�A�E�g���̃|�C���^��Ԃ� */
CLayout* CLayoutMgr::DeleteLayoutAsLogical(
			CLayout* pLayoutInThisArea,
			int		nLineOf_pLayoutInThisArea,

			int nLineFrom,
			int nLineTo,
			int nDelLogicalLineFrom,
			int nDelLogicalColFrom,
			int* pnDeleteLines
)
{


//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::DeleteLayoutAsLogical" );
//#endif
	CLayout* pLayout;
	CLayout* pLayoutWork;
	CLayout* pLayoutNext;

	*pnDeleteLines = 0;
	if( 0 == m_nLines){	/* �S�����s�� */
		return NULL;
	}
	if( NULL == pLayoutInThisArea ){
		return NULL;
	}

	// 1999.11.22
	m_pLayoutPrevRefer = pLayoutInThisArea->m_pPrev;
	m_nPrevReferLine = nLineOf_pLayoutInThisArea - 1;
//	m_pLayoutPrevRefer = NULL;
//	m_nPrevReferLine = 0;


	/* �͈͓��擪�ɊY�����郌�C�A�E�g�����T�[�` */
	pLayoutWork = pLayoutInThisArea->m_pPrev;
	while( NULL != pLayoutWork && nLineFrom <= pLayoutWork->m_nLinePhysical){
		pLayoutWork = pLayoutWork->m_pPrev;
	}
//			m_pLayoutPrevRefer = pLayout->m_pPrev;
//			--m_nPrevReferLine;



	if( NULL == pLayoutWork ){
		pLayout	= m_pLayoutTop;
	}else{
		pLayout = pLayoutWork->m_pNext;
	}
	while( NULL != pLayout ){
		if( pLayout->m_nLinePhysical > nLineTo ){
			break;
		}
		pLayoutNext = pLayout->m_pNext;
		if( NULL == pLayoutWork ){
			/* �擪�s�̏��� */
			m_pLayoutTop = pLayout->m_pNext;
			if( NULL != pLayout->m_pNext ){
				pLayout->m_pNext->m_pPrev = NULL;
			}
		}else{
			pLayoutWork->m_pNext = pLayout->m_pNext;
			if( NULL != pLayout->m_pNext ){
				pLayout->m_pNext->m_pPrev = pLayoutWork;
			}
		}
//		if( m_pLayoutPrevRefer == pLayout ){
//			// 1999.12.22 �O�ɂ��炷�����ł悢�̂ł�
//			m_pLayoutPrevRefer = pLayout->m_pPrev;
//			--m_nPrevReferLine;
//		}

		if( ( nDelLogicalLineFrom == pLayout->m_nLinePhysical &&
			  nDelLogicalColFrom < pLayout->m_nOffset + pLayout->m_nLength ) ||
			( nDelLogicalLineFrom < pLayout->m_nLinePhysical )
		){
			(*pnDeleteLines)++;;
		}

#ifdef _DEBUG
		if( m_pLayoutPrevRefer == pLayout ){
			MYTRACE( "�o�O�o�O\n" );
		}
#endif
		delete pLayout;

		m_nLines--;	/* �S�����s�� */
		if( NULL == pLayoutNext ){
			m_pLayoutBot = pLayoutWork;
		}
		pLayout = pLayoutNext;
	}
//	MYTRACE( "(*pnDeleteLines)=%d\n", (*pnDeleteLines) );

	return pLayoutWork;
}




/* �w��s����̍s�̃��C�A�E�g���ɂ��āA�_���s�ԍ����w��s�������V�t�g���� */
/* �_���s���폜���ꂽ�ꍇ�͂O��菬�����s�� */
/* �_���s���}�����ꂽ�ꍇ�͂O���傫���s�� */
void CLayoutMgr::ShiftLogicalLineNum( CLayout* pLayoutPrev, int nShiftLines )
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::ShiftLogicalLineNum" );

	CLayout* pLayout;
//	CLayout* pLayoutNext;
	if( 0 == nShiftLines ){
		return;
	}
	if( NULL == pLayoutPrev ){
		pLayout = m_pLayoutTop;
	}else{
		pLayout = pLayoutPrev->m_pNext;
	}
	/* ���C�A�E�g���S�̂��X�V����(�ȂȁA�Ȃ��!!!) */
	while( NULL != pLayout ){
//		pLayoutNext = pLayout->m_pNext;
		pLayout->m_nLinePhysical += nShiftLines;	/* �Ή�����_���s�ԍ� */
//		pLayout = pLayoutNext;
		pLayout = pLayout->m_pNext;
	}
	return;
}





/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
int CLayoutMgr::WhereCurrentWord(
	int			nLineNum,
	int			nIdx,
	int*		pnLineFrom,
	int*		pnIdxFrom,
	int*		pnLineTo,
	int*		pnIdxTo,
	CMemory*	pcmcmWord,
	CMemory*	pcmcmWordLeft
)
{
	int nRetCode;
	CLayout* pLayout;
	pLayout = Search( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}
	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	nRetCode = m_pcDocLineMgr->WhereCurrentWord(
		pLayout->m_nLinePhysical,
		pLayout->m_nOffset + nIdx,
		pnIdxFrom,
		pnIdxTo,
		pcmcmWord,
		pcmcmWordLeft
	);
	if( nRetCode ){
		/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_nLinePhysical, *pnIdxFrom, pnLineFrom, pnIdxFrom );
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_nLinePhysical, *pnIdxTo,	pnLineTo, pnIdxTo );
	}
	return nRetCode;

}





/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
int CLayoutMgr::PrevOrNextWord(
		int		nLineNum,
		int		nIdx,
		int*	pnLineNew,
		int*	pnColmNew,
		BOOL	bLEFT,
		BOOL	bStopsBothEnds
)
{
	int			nRetCode;
	CLayout*	pLayout;
	pLayout = Search( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}
	/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
	nRetCode = m_pcDocLineMgr->PrevOrNextWord(
		pLayout->m_nLinePhysical,
		pLayout->m_nOffset + nIdx,
		pnColmNew,
		bLEFT,
		bStopsBothEnds
	);

	if( nRetCode ){
		/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_nLinePhysical, *pnColmNew, pnLineNew, pnColmNew );
	}
	return nRetCode;
}





//! �P�ꌟ��
/*
	@retval 0 ������Ȃ�
*/
int CLayoutMgr::SearchWord(
	int			nLineNum, 		/* �����J�n�s */
	int			nIdx, 			/* �����J�n�ʒu */
	const char*	pszPattern,		/* �������� */
	int			bPrevOrNext,	/* 0==�O������ 1==������� */
	int			bRegularExp,	/* 1==���K�\�� */
	int			bLoHiCase,		/* 1==�啶���������̋�� */
	int			bWordOnly,		/* 1==�P��̂݌��� */
	int*		pnLineFrom, 	/* �}�b�`���C�A�E�g�sfrom */
	int*		pnIdxFrom, 		/* �}�b�`���C�A�E�g�ʒufrom */
	int*		pnLineTo, 		/* �}�b�`���C�A�E�g�sto */
	int*		pnIdxTo,  		/* �}�b�`���C�A�E�g�ʒuto */
	//!	[in] ���K�\���R���p�C���f�[�^
	CBregexp*	pRegexp	//	Jun. 26, 2001 genta

)
{
	int			nRetCode;
	int			nLogLine;
	CLayout*	pLayout;
	pLayout = Search( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}
	/* �P�ꌟ�� */
	nRetCode = m_pcDocLineMgr->SearchWord(
		pLayout->m_nLinePhysical,
		pLayout->m_nOffset + nIdx,
		pszPattern,
		bPrevOrNext,
		bRegularExp,
		bLoHiCase,
		bWordOnly,
		pnLineFrom,
		pnIdxFrom,
		pnIdxTo,
		pRegexp			/* ���K�\���R���p�C���f�[�^ */
	);

	if( nRetCode ){
		/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
		nLogLine = *pnLineFrom;
		CaretPos_Phys2Log(
			*pnIdxFrom,
			nLogLine,
			pnIdxFrom,
			pnLineFrom
		);
		CaretPos_Phys2Log(
			*pnIdxTo,
			nLogLine,
			pnIdxTo,
			pnLineTo
		);
	}
	return nRetCode;
}





/* �_���ʒu�����C�A�E�g�ʒu�ϊ� */
void CLayoutMgr::XYLogicalToLayout(
		CLayout*	pLayoutInThisArea,
		int			nLayoutLineOfThisArea,
		int			nLogLine,
		int			nLogIdx,
		int*		pnLayLine,
		int*		pnLayIdx
)
{
	CLayout*	pLayout;
	int			nCurLayLine;
	*pnLayLine = 0;
	*pnLayIdx = 0;

	if( pLayoutInThisArea == NULL ){
		pLayoutInThisArea = m_pLayoutBot;
		nLayoutLineOfThisArea = m_nLines - 1;
	}
	if( pLayoutInThisArea == NULL ){
		*pnLayLine = m_nLines;
		*pnLayIdx = 0;
		return;
	}
	/* �͈͓��擪�ɊY�����郌�C�A�E�g�����T�[�` */
	if( (pLayoutInThisArea->m_nLinePhysical > nLogLine) ||
		(pLayoutInThisArea->m_nLinePhysical == nLogLine &&
		 pLayoutInThisArea->m_nOffset >	nLogIdx)
	){
		/* ���݈ʒu���O���Ɍ������ăT�[�` */
		pLayout =  pLayoutInThisArea->m_pPrev;
		nCurLayLine = nLayoutLineOfThisArea - 1;
		while( pLayout != NULL ){
			if( pLayout->m_nLinePhysical == nLogLine &&
				pLayout->m_nOffset <= nLogIdx &&
				nLogIdx <= pLayout->m_nOffset + pLayout->m_nLength
			){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_nOffset;
				return;
			}
			if( NULL == pLayout->m_pPrev ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = 0;
				return;
			}
			pLayout = pLayout->m_pPrev;
			nCurLayLine--;
		}
	}else{
		/* ���݈ʒu���܂ތ���Ɍ������ăT�[�` */
		pLayout =  pLayoutInThisArea;
		nCurLayLine = nLayoutLineOfThisArea;
		while( pLayout != NULL ){
			if( pLayout->m_nLinePhysical == nLogLine &&
				pLayout->m_nOffset <= nLogIdx  &&
				nLogIdx <= pLayout->m_nOffset + pLayout->m_nLength
			 ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_nOffset;
				return;
			}
			if( NULL == pLayout->m_pNext ){
//				if( nCurLayLine == nLogLine ){
//					*pnLayLine = nCurLayLine;
//					*pnLayIdx = pLayout->m_nLength;
//				}else{
					*pnLayLine = nCurLayLine + 1;
					*pnLayIdx = 0;
//				}
				return;
			}else
			if( pLayout->m_pNext->m_nLinePhysical > nLogLine ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_nOffset;
				return;
			}
			pLayout = pLayout->m_pNext;
			nCurLayLine++;
		}
	}
	return;
}


/*
  �J�[�\���ʒu�ϊ�
  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
  ��
  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
*/
void CLayoutMgr::CaretPos_Phys2Log(
		int		nX,
		int		nY,
		int*	pnCaretPosX,
		int*	pnCaretPosY
//		BOOL	bFreeCaret
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::CaretPos_Phys2Log" );
//#endif
	const CLayout*	pLayout;
//	CLayout*		pLayoutNext;
	int				nCaretPosX;
	int				nCaretPosY;
	const char*		pData;
	int				nDataLen;
	int				i;
	int				nCharChars;

	*pnCaretPosX = 0;
	*pnCaretPosY = 0;
//1999.12.11
//	�T�[�`�J�n�n�_�͐擪����
//	pLayout = m_pLayoutTop;
//	nCaretPosY = 0;

	/* ���s�P�ʍs <= �܂�Ԃ��P�ʍs �����藧����A
	�T�[�`�J�n�n�_���ł��邾���ړI�n�֋߂Â��� */
//	pLayout = GetLineData( nY );
	pLayout = Search( nY );
	if( NULL == pLayout ){
		if( 0 < m_nLines ){
			*pnCaretPosY = m_nLines;
		}
		return;
	}
	nCaretPosY = nY;


	nCaretPosX = 0;
	do{
		if( nY == pLayout->m_nLinePhysical ){
			nCaretPosX = 0;
//			pData = GetLineStr( nCaretPosY, &nDataLen );
//			pData = pLayout->m_pLine + pLayout->m_nOffset;
			pData = pLayout->m_pCDocLine->m_pLine->GetPtr() + pLayout->m_nOffset; // 2002/2/10 aroka CMemory�ύX
			nDataLen = pLayout->m_nLength;

			for( i = 0; i < nDataLen; ++i ){
				if( pLayout->m_nOffset + i >= nX ){
					break;
				}
				if( pData[i] ==	TAB ){
					// Sep. 23, 2002 genta �����o�[�֐����g���悤��
					nCharChars = GetActualTabSpace( nCaretPosX );
				}else{
					nCharChars = CMemory::MemCharNext( pData, nDataLen, &pData[i] ) - &pData[i];
				}
				if( nCharChars == 0 ){
					nCharChars = 1;
				}
				nCaretPosX += nCharChars;
				if( pData[i] ==	TAB ){
					nCharChars = 1;
				}
				i += nCharChars - 1;
			}
			if( i < nDataLen ){
				break;
			}
			if( NULL == pLayout->m_pNext ){
				nCaretPosX += ( nDataLen - i );
//				nCaretPosX = 0;
				break;
			}
			if( nY < pLayout->m_pNext->m_nLinePhysical ){
				nCaretPosX += ( nDataLen - i );
				break;
			}
//		}else{
//			nCaretPosX = 0;
		}
		if( nY < pLayout->m_nLinePhysical ){
			break;
		}else{
//			nCaretPosX = 0;
			nCaretPosY++;
		}
		pLayout = pLayout->m_pNext;
	}while( NULL != pLayout );
	*pnCaretPosX = nCaretPosX + (pLayout ? pLayout->GetIndent() : 0);
	*pnCaretPosY = nCaretPosY;
//#ifdef _DEBUG
//	MYTRACE( "\t\tnCaretPosY - nY = %d\n", nCaretPosY - nY );
//#endif
	return;
}

/*
  �J�[�\���ʒu�ϊ�
  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
  ��
  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
*/
void CLayoutMgr::CaretPos_Log2Phys(
		int		nCaretPosX,
		int		nCaretPosY,
		int*	pnX,
		int*	pnY
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CLayoutMgr::CaretPos_Log2Phys" );
//#endif
	int				nX;
//	int				nY;
	const CLayout*	pcLayout;
	int				i;
	const char*		pData;
	int				nDataLen;
	int				nCharChars;
	BOOL			bEOF;
	bEOF = FALSE;

	*pnX = 0;
	*pnY = 0;
	if( nCaretPosY > m_nLines ){
		*pnX = 0;
		*pnY = m_nLines;
		return;
	}
	pcLayout = Search( nCaretPosY );
	if( NULL == pcLayout ){
		if( 0 < nCaretPosY ){
			pcLayout = Search( nCaretPosY - 1 );
			if( NULL == pcLayout ){
				*pnX = 0;
				*pnY = m_pcDocLineMgr->GetLines(); // 2002/2/10 aroka CDocLineMgr�ύX
				return;
			}else{
				pData = GetLineStr( nCaretPosY - 1, &nDataLen );
				if( pData[nDataLen - 1] == '\r' || pData[nDataLen - 1] == '\n' ){
					*pnX = 0;
					*pnY = m_pcDocLineMgr->GetLines(); // 2002/2/10 aroka CDocLineMgr�ύX
					return;
				}else{
					*pnY = m_pcDocLineMgr->GetLines() - 1; // 2002/2/10 aroka CDocLineMgr�ύX
					bEOF = TRUE;
					nX = 999999;
					goto checkloop;

				}
			}
		}
		*pnX = 0;
		*pnY = m_nLines;
		return;
	}else{
		*pnY = pcLayout->m_nLinePhysical;
	}

	pData = GetLineStr( nCaretPosY, &nDataLen );
	nX = pcLayout ? pcLayout->GetIndent() : 0;
checkloop:;
//	enumEOLType nEOLType;
	for( i = 0; i < nDataLen; ++i ){
		if( pData[i] ==	TAB ){
			// Sep. 23, 2002 genta �����o�[�֐����g���悤��
			nCharChars = GetActualTabSpace( nX );
//		}else
//		if( pData[i] == '\r' || pData[i] == '\n' ){
//			/* �s�I�[�q�̎�ނ𒲂ׂ� */
//			nEOLType = GetEOLType( &pData[i], nDataLen - i );
//			nCharChars = 1;
		}else{
			nCharChars = CMemory::MemCharNext( pData, nDataLen, &pData[i] ) - &pData[i];
		}

		if( nCharChars == 0 ){
			nCharChars = 1;
		}
		nX += nCharChars;
		if( nX > nCaretPosX && !bEOF ){
			break;
		}
		if( pData[i] ==	TAB ){
			nCharChars = 1;
		}
//		if( pData[i] == '\r' || pData[i] == '\n' ){
//			nCharChars = gm_pnEolLenArr[nEOLType];
//		}
		i += nCharChars - 1;
	}
	i += pcLayout->m_nOffset;
	*pnX = i;
	return;
}



/* �e�X�g�p�Ƀ��C�A�E�g�����_���v */
void CLayoutMgr::DUMP( void )
{
#ifdef _DEBUG
	const char* pData;
	int nDataLen;
	MYTRACE( "------------------------\n" );
	MYTRACE( "m_nLines=%d\n", m_nLines );
	MYTRACE( "m_pLayoutTop=%08lxh\n", m_pLayoutTop );
	MYTRACE( "m_pLayoutBot=%08lxh\n", m_pLayoutBot );
	MYTRACE( "m_nMaxLineSize=%d\n", m_nMaxLineSize );

	MYTRACE( "m_nTabSpace=%d\n", m_nTabSpace );
	CLayout* pLayout;
	CLayout* pLayoutNext;
	pLayout = m_pLayoutTop;
	while( NULL != pLayout ){
		pLayoutNext = pLayout->m_pNext;
		MYTRACE( "\t-------\n" );
		MYTRACE( "\tthis=%08lxh\n", pLayout );
		MYTRACE( "\tm_pPrev =%08lxh\n",		pLayout->m_pPrev );
		MYTRACE( "\tm_pNext =%08lxh\n",		pLayout->m_pNext );
		MYTRACE( "\tm_nLinePhysical=%d\n",	pLayout->m_nLinePhysical );
		MYTRACE( "\tm_nOffset=%d\n",		pLayout->m_nOffset );
		MYTRACE( "\tm_nLength=%d\n",		pLayout->m_nLength );
		MYTRACE( "\tm_enumEOLType =%s\n",	pLayout->m_cEol.GetName() );
		MYTRACE( "\tm_nEOLLen =%d\n",		pLayout->m_cEol.GetLen() );
		MYTRACE( "\tm_nTypePrev=%d\n",		pLayout->m_nTypePrev );
		pData = m_pcDocLineMgr->GetLineStr( pLayout->m_nLinePhysical, &nDataLen );
		MYTRACE( "\t[%s]\n", pData );
		pLayout = pLayoutNext;
	}
	MYTRACE( "------------------------\n" );
#endif
	return;
}


/*[EOF]*/
