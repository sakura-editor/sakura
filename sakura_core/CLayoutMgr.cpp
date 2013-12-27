/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, YAZAKI, genta, aroka
	Copyright (C) 2003, genta, Moca
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, D.S.Koba, Moca
	Copyright (C) 2006, genta, Moca
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CLayoutMgr.h"
#include "CShareData.h" // 2005.11.20 Moca
#include "charcode.h"
#include "Debug.h"
#include "CRunningTimer.h"
#include "CLayout.h"/// 2002/2/10 aroka
#include "CDocLine.h"/// 2002/2/10 aroka
#include "CDocLineMgr.h"/// 2002/2/10 aroka
#include "CMemory.h"/// 2002/2/10 aroka
#include "etc_uty.h" // Oct. 5, 2002 genta
#include "CMemoryIterator.h" // 2006.07.29 genta
#include "CEditDoc.h"		// 2009.08.28 nasukoji

CLayoutMgr::CLayoutMgr()
: m_getIndentOffset( &CLayoutMgr::getIndentOffset_Normal )	//	Oct. 1, 2002 genta	//	Nov. 16, 2002 メンバー関数ポインタにはクラス名が必要
{
	m_pcDocLineMgr = NULL;
	m_sTypeConfig.m_bWordWrap = true;				// 英文ワードラップをする
	m_sTypeConfig.m_nTabSpace = 8;					// TAB文字スペース
	m_sTypeConfig.m_nStringType = 0;				// 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""]['']
	m_sTypeConfig.m_bKinsokuHead = false;			// 行頭禁則				//@@@ 2002.04.08 MIK
	m_sTypeConfig.m_bKinsokuTail = false;			// 行末禁則				//@@@ 2002.04.08 MIK
	m_sTypeConfig.m_bKinsokuRet  = false;			// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	m_sTypeConfig.m_bKinsokuKuto = false;			// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	m_pszKinsokuHead_1 = NULL;						// 行頭禁則				//@@@ 2002.04.08 MIK
	m_pszKinsokuHead_2 = NULL;						// 行頭禁則				//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_1 = NULL;						// 行末禁則				//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_2 = NULL;						// 行頭禁則				//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1 = NULL;						// 句読点ぶらさげ		//@@@ 2002.04.17 MIK
	m_pszKinsokuKuto_2 = NULL;						// 句読点ぶらさげ		//@@@ 2002.04.17 MIK

	// 2005.11.21 Moca 色分けフラグをメンバで持つ
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = false; 
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;

	m_nTextWidth = 0;			// テキスト最大幅の記憶		// 2009.08.28 nasukoji
	m_nTextWidthMaxLine = 0;	// 最大幅のレイアウト行		// 2009.08.28 nasukoji

	Init();
}

CLayoutMgr::~CLayoutMgr()
{
	_Empty();

	/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	delete [] m_pszKinsokuHead_1;
	m_pszKinsokuHead_1 = NULL;
	delete [] m_pszKinsokuHead_2;
	m_pszKinsokuHead_2 = NULL;
	/* 行末禁則 */	//@@@ 2002.04.08 MIK
	delete [] m_pszKinsokuTail_1;
	m_pszKinsokuTail_1 = NULL;
	delete [] m_pszKinsokuTail_2;
	m_pszKinsokuTail_2 = NULL;
	/* 句読点ぶらさげ */	//@@@ 2002.04.17 MIK
	delete [] m_pszKinsokuKuto_1;
	m_pszKinsokuKuto_1 = NULL;
	delete [] m_pszKinsokuKuto_2;
	m_pszKinsokuKuto_2 = NULL;
}


/*
||
|| 行データ管理クラスのポインタを初期化します
||
*/
void CLayoutMgr::Create( CEditDoc* pcEditDoc, CDocLineMgr* pcDocLineMgr )
{
	Init();
	//	Jun. 20, 2003 genta EditDocへのポインタ追加
	m_pcEditDoc = pcEditDoc;
	m_pcDocLineMgr = pcDocLineMgr;
}

void CLayoutMgr::Init()
{
	m_pLayoutTop = NULL;
	m_pLayoutBot = NULL;
	m_nPrevReferLine = 0;
	m_pLayoutPrevRefer = NULL;
	m_nLines = 0;			/* 全物理行数 */

	// EOFレイアウト位置記憶	//2006.10.07 Moca
	m_nEOFLine = -1;
	m_nEOFColumn = -1;
}



void CLayoutMgr::_Empty()
{
	CLayout* pLayout = m_pLayoutTop;
	while( pLayout ){
		CLayout* pLayoutNext = pLayout->m_pNext;
		delete pLayout;
		pLayout = pLayoutNext;
	}
}




/*! レイアウト情報の変更
	@param bDoRayout [in] レイアウト情報の再作成
	@param refType [in] タイプ別設定
*/
void CLayoutMgr::SetLayoutInfo(
	bool				bDoRayout,
	HWND				hwndProgress,
	const STypeConfig&	refType
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CLayoutMgr::SetLayoutInfo" );

	//タイプ別設定
	m_sTypeConfig = refType;

	//	Oct. 1, 2002 genta タイプによって処理関数を変更する
	//	数が増えてきたらテーブルにすべき
	switch ( refType.m_nIndentLayout ){	/* 折り返しは2行目以降を字下げ表示 */	//@@@ 2002.09.29 YAZAKI
	case 1:
		//	Nov. 16, 2002 メンバー関数ポインタにはクラス名が必要
		m_getIndentOffset = &CLayoutMgr::getIndentOffset_Tx2x;
		break;
	case 2:
		m_getIndentOffset = &CLayoutMgr::getIndentOffset_LeftSpace;
		break;
	default:
		m_getIndentOffset = &CLayoutMgr::getIndentOffset_Normal;
		break;
	}

	{	//@@@ 2002.04.08 MIK start
		unsigned char	*p, *q1, *q2;
		int	length;

		//句読点のぶらさげ
		m_sTypeConfig.m_bKinsokuKuto = refType.m_bKinsokuKuto;	/* 句読点ぶらさげ */	//@@@ 2002.04.17 MIK
		delete [] m_pszKinsokuKuto_1;
		m_pszKinsokuKuto_1 = NULL;
		delete [] m_pszKinsokuKuto_2;
		m_pszKinsokuKuto_2 = NULL;
		length = strlen( refType.m_szKinsokuKuto ) + 1;
		m_pszKinsokuKuto_1 = new char[ length ];
		m_pszKinsokuKuto_2 = new char[ length ];
		q1 = (unsigned char *)m_pszKinsokuKuto_1;
		q2 = (unsigned char *)m_pszKinsokuKuto_2;
		memset( (void *)q1, 0, length );
		memset( (void *)q2, 0, length );
		if( m_sTypeConfig.m_bKinsokuKuto )	// 2009.08.06 ryoji m_bKinsokuKutoで振り分ける(Fix)
		{
			for( p = (unsigned char *)refType.m_szKinsokuKuto; *p; p++ )
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
		}

		//行頭禁則文字の1,2バイト文字を分けて管理する。
		m_sTypeConfig.m_bKinsokuHead = refType.m_bKinsokuHead;
		delete [] m_pszKinsokuHead_1;
		delete [] m_pszKinsokuHead_2;
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
				if( IsKinsokuKuto( (char*)p, 2 ) )	//句読点は別管理
				{
					p++;
					continue;
				}
				else{
					*q2 = *p; q2++; p++;
					*q2 = *p; q2++;
					*q2 = 0;
				}
			}
			else
			{
				if( IsKinsokuKuto( (char*)p, 1 ) )	//句読点は別管理
				{
					continue;
				}
				else
				{
					*q1 = *p; q1++;
					*q1 = 0;
				}
			}
		}

		//行末禁則文字の1,2バイト文字を分けて管理する。
		m_sTypeConfig.m_bKinsokuTail = refType.m_bKinsokuTail;
		delete [] m_pszKinsokuTail_1;
		delete [] m_pszKinsokuTail_2;
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

		m_sTypeConfig.m_bKinsokuRet = refType.m_bKinsokuRet;	/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	}	//@@@ 2002.04.08 MIK end

	if( bDoRayout ){
		_DoLayout( hwndProgress );
	}
}




/*!
	@brief 指定された物理行のレイアウト情報を取得

	@param nLineNum [in] 物理行番号 (0～)
*/
CLayout* CLayoutMgr::SearchLineByLayoutY(
	int nLineNum
)
{
	CLayout*	pLayout;
	int			nCount;
	if( 0 == m_nLines ){
		return NULL;
	}

	//	Mar. 19, 2003 Moca nLineNumが負の場合のチェックを追加
	if( 0 > nLineNum || nLineNum >= m_nLines ){
		if( 0 > nLineNum ){
			DEBUG_TRACE( _T("CLayoutMgr::SearchLineByLayoutY() nLineNum = %d\n"), nLineNum );
		}
		return NULL;
	}
//	/*+++++++ 低速版 +++++++++*/
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


	/*+++++++わずかに高速版+++++++*/
	// 2004.03.28 Moca m_pLayoutPrevReferより、Top,Botのほうが近い場合は、そちらを利用する
	int nPrevToLineNumDiff = abs( m_nPrevReferLine - nLineNum );
	if( m_pLayoutPrevRefer == NULL
	  || nLineNum < nPrevToLineNumDiff
	  || m_nLines - nLineNum < nPrevToLineNumDiff
	){
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
		}
		else if( nLineNum > m_nPrevReferLine ){
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
		}
		else{
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


//@@@ 2002.09.23 YAZAKI CLayout*を作成するところは分離して、InsertLineNext()と共通化
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

//@@@ 2002.09.23 YAZAKI CLayout*を作成するところは分離して、AddLineBottom()と共通化
CLayout* CLayoutMgr::InsertLineNext( CLayout* pLayoutPrev, CLayout* pLayout )
{
	CLayout* pLayoutNext;

	if(	0 == m_nLines ){
		/* 初 */
		m_pLayoutBot = m_pLayoutTop = pLayout;
		m_pLayoutTop->m_pPrev = NULL;
		m_pLayoutTop->m_pNext = NULL;
	}
	else if( NULL == pLayoutPrev ){
		/* 先頭に挿入 */
		m_pLayoutTop->m_pPrev = pLayout;
		pLayout->m_pPrev = NULL;
		pLayout->m_pNext = m_pLayoutTop;
		m_pLayoutTop = pLayout;
	}else
	if( NULL == pLayoutPrev->m_pNext ){
		/* 最後に挿入 */
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		pLayout->m_pNext = NULL;
		m_pLayoutBot = pLayout;
	}else{
		/* 途中に挿入 */
		pLayoutNext = pLayoutPrev->m_pNext;
		pLayoutPrev->m_pNext = pLayout;
		pLayoutNext->m_pPrev = pLayout;
		pLayout->m_pPrev = pLayoutPrev;
		pLayout->m_pNext = pLayoutNext;
	}
	m_nLines++;
	return pLayout;
}

/* CLayoutを作成する
	@@@ 2002.09.23 YAZAKI
	@date 2009.08.28 nasukoji	レイアウト長を引数に追加
*/
CLayout* CLayoutMgr::CreateLayout(
	CDocLine*		pCDocLine,
	CLogicPoint		ptLogicPos,
	int				nLength,
	EColorIndexType	nTypePrev,
	int				nIndent,
	int				nPosX
)
{
	CLayout* pLayout = new CLayout;
	pLayout->m_pCDocLine = pCDocLine;

	pLayout->m_ptLogicPos = ptLogicPos;
	pLayout->m_nLength = nLength;
	pLayout->m_nTypePrev = nTypePrev;
	pLayout->m_nIndent = nIndent;

	if( EOL_NONE == pCDocLine->m_cEol ){
		pLayout->m_cEol.SetType( EOL_NONE );/* 改行コードの種類 */
	}else{
		if( pLayout->m_ptLogicPos.x + pLayout->m_nLength >
			pCDocLine->m_cLine.GetStringLength() - pCDocLine->m_cEol.GetLen()
		){
			pLayout->m_cEol = pCDocLine->m_cEol;/* 改行コードの種類 */
		}else{
			pLayout->m_cEol = EOL_NONE;/* 改行コードの種類 */
		}
	}

	// 2009.08.28 nasukoji	「折り返さない」選択時のみレイアウト長を記憶する
	// 「折り返さない」以外で計算しないのはパフォーマンス低下を防ぐ目的なので、
	// パフォーマンスの低下が気にならない程なら全ての折り返し方法で計算する
	// ようにしても良いと思う。
	// （その場合CLayoutMgr::CalculateTextWidth()の呼び出し箇所をチェック）
	pLayout->m_nLayoutWidth = ( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ) ? nPosX : 0;

	return pLayout;
}


/*
|| 指定された物理行のデータへのポインタとその長さを返す Ver0

	@date 2002/2/10 aroka CMemory変更
*/
const char* CLayoutMgr::GetLineStr( int nLine, int* pnLineLen )
{
	CLayout* pLayout;
	if( NULL == ( pLayout = SearchLineByLayoutY( nLine )	) ){
		return NULL;
	}
	*pnLineLen = pLayout->m_nLength;
	return pLayout->m_pCDocLine->m_cLine.GetStringPtr() + pLayout->m_ptLogicPos.x;
}

/*!	指定された物理行のデータへのポインタとその長さを返す Ver1
	@date 2002/03/24 YAZAKI GetLineStr( int nLine, int* pnLineLen )と同じ動作に変更。
*/
const char* CLayoutMgr::GetLineStr( int nLine, int* pnLineLen, const CLayout** ppcLayoutDes )
{
	if( NULL == ( (*ppcLayoutDes) = SearchLineByLayoutY( nLine )	) ){
		return NULL;
	}
	*pnLineLen = (*ppcLayoutDes)->m_nLength;
	return (*ppcLayoutDes)->m_pCDocLine->m_cLine.GetStringPtr() + (*ppcLayoutDes)->m_ptLogicPos.x;
}

/*
|| 指定された位置がレイアウト行の途中の行末かどうか調べる

	@date 2002/4/27 MIK
*/
bool CLayoutMgr::IsEndOfLine(
	const CLayoutPoint& ptLinePos
)
{
	const CLayout* pLayout;

	if( NULL == ( pLayout = SearchLineByLayoutY( ptLinePos.y )	) )
	{
		return false;
	}

	if( EOL_NONE == pLayout->m_cEol.GetType() )
	{	/* この行に改行はない */
		/* この行の最後か？ */
		if( ptLinePos.x == pLayout->m_nLength ) return true;
	}

	return false;
}

/*!	@brief ファイル末尾のレイアウト位置を取得する

	ファイル末尾まで選択する場合に正確な位置情報を与えるため

	既存の関数では物理行からレイアウト位置を変換する必要があり，
	処理に無駄が多いため，専用関数を作成
	
	@date 2006.07.29 genta
	@date 2006.10.01 Moca メンバで保持するように。データ変更時には、_DoLayout/DoLayout_Rangeで無効にする。
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

	if( 0 == m_nLines || m_pLayoutBot == NULL ){
		// データが空
		ptLayoutEnd->x = 0;
		ptLayoutEnd->y = 0;
		m_nEOFColumn = ptLayoutEnd->x;
		m_nEOFLine = ptLayoutEnd->y;
		return;
	}

	CLayout *btm = m_pLayoutBot;
	if( btm->m_cEol != EOL_NONE ){
		//	末尾に改行がある
		ptLayoutEnd->x = 0;
		ptLayoutEnd->y = GetLineCount();
	}
	else {
		CMemoryIterator<CLayout> it( btm, GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			it.addDelta();
		}
		ptLayoutEnd->x = it.getColumn();
		ptLayoutEnd->y = GetLineCount() - 1;
		// 2006.10.01 Moca Start [EOF]のみのレイアウト行処理が抜けていたバグを修正
		if( GetMaxLineKetas() <= ptLayoutEnd->x ){
			ptLayoutEnd->x = 0;
			ptLayoutEnd->y++;
		}
		// 2006.10.01 Moca End
	}
	m_nEOFColumn = ptLayoutEnd->x;
	m_nEOFLine = ptLayoutEnd->y;
}


/*!	行内文字削除

	@date 2002/03/24 YAZAKI bUndo削除
	@date 2009/08/28 nasukoji	テキスト最大幅の算出に対応
*/
void CLayoutMgr::DeleteData_CLayoutMgr(
	int			nLineNum,
	int			nDelPos,
	int			nDelLen,
	int*		pnModifyLayoutLinesOld,
	int*		pnModifyLayoutLinesNew,
	int*		pnDeleteLayoutLines,
	CMemory*	cmemDeleted			//!< [out] 削除されたデータ
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CLayoutMgr::DeleteData_CLayoutMgr" );
#endif
	const char*	pLine;
	int			nLineLen;
	CLayout*	pLayout;
	CLayout*	pLayoutPrev;
	CLayout*	pLayoutWork;
	int			nModLineOldFrom;	/* 影響のあった変更前の行(from) */
	int			nModLineOldTo;		/* 影響のあった変更前の行(to) */
	int			nDelLineOldFrom;	/* 削除された変更前論理行(from) */
	int			nDelLineOldNum;		/* 削除された行数 */
	int			nRowNum;
	int			nDelStartLogicalLine;
	int			nDelStartLogicalPos;
	EColorIndexType	nCurrentLineType;
	int			nLineWork;

	/* 現在行のデータを取得 */
	pLine = GetLineStr( nLineNum, &nLineLen );
	if( NULL == pLine ){
		return;
	}
	pLayout = m_pLayoutPrevRefer;
	nDelStartLogicalLine = pLayout->m_ptLogicPos.y;
	nDelStartLogicalPos  = nDelPos + pLayout->m_ptLogicPos.x;

	pLayoutWork = pLayout;
	nLineWork = nLineNum;
	while( 0 != pLayoutWork->m_ptLogicPos.x ){
		pLayoutWork = pLayoutWork->m_pPrev;
		--nLineWork;
	}
	nCurrentLineType = pLayoutWork->m_nTypePrev;

	/* テキストのデータを削除 */
	m_pcDocLineMgr->DeleteData_CDocLineMgr(
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

	/*--- 変更された行のレイアウト情報を再生成 ---*/
	/* 論理行の指定範囲に該当するレイアウト情報を削除して */
	/* 削除した範囲の直前のレイアウト情報のポインタを返す */
	int		nAllLinesOld = m_nLines;
	pLayoutPrev = DeleteLayoutAsLogical(
		pLayoutWork,
		nLineWork,
		nModLineOldFrom,
		nModLineOldTo,
		nDelStartLogicalLine,
		nDelStartLogicalPos,
		pnModifyLayoutLinesOld
	);

	/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */
	/* 論理行が削除された場合は０より小さい行数 */
	/* 論理行が挿入された場合は０より大きい行数 */
	ShiftLogicalLineNum( pLayoutPrev, nDelLineOldNum * (-1) );

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_ptLogicPos.y;
		}
	}else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_ptLogicPos.y -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}
	}

	// 2009.08.28 nasukoji	テキスト最大幅算出用の引数を設定
	CalTextWidthArg ctwArg;
	ctwArg.ptLayout.y   = nLineNum;			// 編集開始行
	ctwArg.ptLayout.x   = nDelPos;			// 編集開始桁
	ctwArg.nDelLines    = 0;				// 削除行は1行
	ctwArg.nAllLinesOld = nAllLinesOld;		// 編集前のテキスト行数
	ctwArg.bInsData     = FALSE;			// 追加文字列なし

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	int	nAddInsLineNum;
	*pnModifyLayoutLinesNew = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		nDelStartLogicalLine, nDelStartLogicalPos,
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	*pnDeleteLayoutLines = nAllLinesOld - m_nLines + nAddInsLineNum;
	return;
}






/*!	文字列挿入

	@date 2002/03/24 YAZAKI bUndo削除
*/
void CLayoutMgr::InsertData_CLayoutMgr(
	int			nLineNum,
	int			nInsPos,
	const char*	pInsData,
	int			nInsDataLen,
	int*		pnModifyLayoutLinesOld,
	int*		pnInsLineNum,		// 挿入によって増えたレイアウト行の数
	int*		pnNewLine,			// 挿入された部分の次の位置の行
	int*		pnNewPos			// 挿入された部分の次の位置のデータ位置
)
{
	CLayout*	pLayout;
	CLayout*	pLayoutPrev;
	CLayout*	pLayoutWork;
	int			nInsStartLogicalLine;
	int			nInsStartLogicalPos;
	int			nInsLineNum;
	int			nRowNum;
	EColorIndexType	nCurrentLineType;
	int			nLineWork;


	// 現在行のデータを取得
	int			nLineLen;
	const char* pLine = GetLineStr( nLineNum, &nLineLen );
	if( !pLine ){
		/*
			2004.04.02 FILE / Moca カーソル位置不正のため、空テキストで
			nLineNumが0でないときに落ちる対策．データが空であることを
			カーソル位置ではなく総行数で判定することでより確実に．
		*/
		if( m_nLines == 0 )
		{
			/* 空のテキストの先頭に行を作る場合 */
			pLayout = NULL;
			nLineWork = 0;
			nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
			nInsStartLogicalPos  = 0;
			nCurrentLineType = COLORIDX_DEFAULT;
		}
		else{
			pLine = GetLineStr( m_nLines - 1, &nLineLen );
			if( ( nLineLen > 0 && ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF )) ||
				( nLineLen > 1 && ( pLine[nLineLen - 2] == CR || pLine[nLineLen - 2] == LF )) ){
				// 空でないテキストの最後に行を作る場合
				pLayout = NULL;
				nLineWork = 0;
				nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
				nInsStartLogicalPos  = 0;
				nCurrentLineType = m_nLineTypeBot;
			}
			else{
				/* 空でないテキストの最後の行を変更する場合 */
				nLineNum = m_nLines	- 1;
				nInsPos = nLineLen;
				pLayout = m_pLayoutPrevRefer;
				nLineWork = m_nPrevReferLine;


				nInsStartLogicalLine = pLayout->m_ptLogicPos.y;
				nInsStartLogicalPos  = nInsPos + pLayout->m_ptLogicPos.x;
				nCurrentLineType = pLayout->m_nTypePrev;
			}
		}
	}else{
		pLayout = m_pLayoutPrevRefer;
		nLineWork = m_nPrevReferLine;


		nInsStartLogicalLine = pLayout->m_ptLogicPos.y;
		nInsStartLogicalPos  = nInsPos + pLayout->m_ptLogicPos.x;
		nCurrentLineType = pLayout->m_nTypePrev;
	}

	if( pLayout ){
		pLayoutWork = pLayout;
		while( pLayoutWork != NULL && 0 != pLayoutWork->m_ptLogicPos.x ){
			pLayoutWork = pLayoutWork->m_pPrev;
			nLineWork--;
		}
		if( NULL != pLayoutWork ){
			nCurrentLineType = pLayoutWork->m_nTypePrev;
		}else{
			nCurrentLineType = COLORIDX_DEFAULT;
		}
	}


	// データの挿入
	int			nNewLine;	//挿入された部分の次の行
	int			nNewPos;	//挿入された部分の次のデータ位置
	m_pcDocLineMgr->InsertData_CDocLineMgr(
		nInsStartLogicalLine,
		nInsStartLogicalPos,
		pInsData,
		nInsDataLen,
		&nInsLineNum,
		&nNewLine,
		&nNewPos
	);


	//--- 変更された行のレイアウト情報を再生成 ---
	// 論理行の指定範囲に該当するレイアウト情報を削除して
	// 削除した範囲の直前のレイアウト情報のポインタを返す
	int	nAllLinesOld = m_nLines;
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

	// 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする
	// 論理行が削除された場合は０より小さい行数
	// 論理行が挿入された場合は０より大きい行数
	if( pLine ){
		ShiftLogicalLineNum( pLayoutPrev, nInsLineNum );
	}

	// 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_ptLogicPos.y;
		}
	}
	else{
		if( NULL == pLayoutPrev->m_pNext ){
			nRowNum =
				m_pcDocLineMgr->GetLineCount() -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}else{
			nRowNum =
				pLayoutPrev->m_pNext->m_ptLogicPos.y -
				pLayoutPrev->m_ptLogicPos.y - 1;
		}
	}

	// 2009.08.28 nasukoji	テキスト最大幅算出用の引数を設定
	CalTextWidthArg ctwArg;
	ctwArg.ptLayout.y   = nLineNum;			// 編集開始行
	ctwArg.ptLayout.x   = nInsPos;			// 編集開始桁
	ctwArg.nDelLines    = -1;				// 削除行なし
	ctwArg.nAllLinesOld = nAllLinesOld;		// 編集前のテキスト行数
	ctwArg.bInsData     = TRUE;				// 追加文字列あり

	// 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
	int			nAddInsLineNum;
	DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		nInsStartLogicalLine, nInsStartLogicalPos,
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	*pnInsLineNum = m_nLines - nAllLinesOld + nAddInsLineNum;

	// 論理位置→レイアウト位置変換
	pLayout = SearchLineByLayoutY( nNewLine );
	XYLogicalToLayout( pLayout, nNewLine, nNewLine, nNewPos, pnNewLine, pnNewPos );
	return;
}






/* 論理行の指定範囲に該当するレイアウト情報を削除して */
/* 削除した範囲の直前のレイアウト情報のポインタを返す */
CLayout* CLayoutMgr::DeleteLayoutAsLogical(
	CLayout*	pLayoutInThisArea,
	int			nLineOf_pLayoutInThisArea,
	int			nLineFrom,
	int			nLineTo,
	int			nDelLogicalLineFrom,
	int			nDelLogicalColFrom,
	int*		pnDeleteLines
)
{
	CLayout* pLayout;
	CLayout* pLayoutWork;
	CLayout* pLayoutNext;

	*pnDeleteLines = 0;
	if( 0 == m_nLines){	/* 全物理行数 */
		return NULL;
	}
	if( NULL == pLayoutInThisArea ){
		return NULL;
	}

	// 1999.11.22
	m_pLayoutPrevRefer = pLayoutInThisArea->m_pPrev;
	m_nPrevReferLine = nLineOf_pLayoutInThisArea - 1;


	/* 範囲内先頭に該当するレイアウト情報をサーチ */
	pLayoutWork = pLayoutInThisArea->m_pPrev;
	while( NULL != pLayoutWork && nLineFrom <= pLayoutWork->m_ptLogicPos.y){
		pLayoutWork = pLayoutWork->m_pPrev;
	}



	if( NULL == pLayoutWork ){
		pLayout	= m_pLayoutTop;
	}else{
		pLayout = pLayoutWork->m_pNext;
	}
	while( NULL != pLayout ){
		if( pLayout->m_ptLogicPos.y > nLineTo ){
			break;
		}
		pLayoutNext = pLayout->m_pNext;
		if( NULL == pLayoutWork ){
			/* 先頭行の処理 */
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
//			// 1999.12.22 前にずらすだけでよいのでは
//			m_pLayoutPrevRefer = pLayout->m_pPrev;
//			--m_nPrevReferLine;
//		}

		if( ( nDelLogicalLineFrom == pLayout->m_ptLogicPos.y &&
			  nDelLogicalColFrom < pLayout->m_ptLogicPos.x + pLayout->m_nLength ) ||
			( nDelLogicalLineFrom < pLayout->m_ptLogicPos.y )
		){
			(*pnDeleteLines)++;
		}

		if( m_pLayoutPrevRefer == pLayout ){
			DEBUG_TRACE( _T("バグバグ\n") );
		}

		delete pLayout;

		m_nLines--;	/* 全物理行数 */
		if( NULL == pLayoutNext ){
			m_pLayoutBot = pLayoutWork;
		}
		pLayout = pLayoutNext;
	}
//	MYTRACE( _T("(*pnDeleteLines)=%d\n"), (*pnDeleteLines) );

	return pLayoutWork;
}




/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */
/* 論理行が削除された場合は０より小さい行数 */
/* 論理行が挿入された場合は０より大きい行数 */
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
	/* レイアウト情報全体を更新する(なな、なんと!!!) */
	while( NULL != pLayout ){
		pLayout->m_ptLogicPos.y += nShiftLines;	/* 対応する論理行番号 */
		pLayout = pLayout->m_pNext;
	}
	return;
}


bool CLayoutMgr::ChangeLayoutParam(
	HWND hwndProgress,
	int nTabSize,
	int nMaxLineKetas
)
{
	if( nTabSize < 1 || nTabSize > 64 ) { return false; }
	if( nMaxLineKetas < MINLINEKETAS || nMaxLineKetas > MAXLINEKETAS ){ return false; }

	m_sTypeConfig.m_nTabSpace = nTabSize;
	m_sTypeConfig.m_nMaxLineKetas = nMaxLineKetas;

	_DoLayout( hwndProgress );

	return true;
}





/* 現在位置の単語の範囲を調べる */
bool CLayoutMgr::WhereCurrentWord(
	int				nLineNum,
	int				nIdx,
	CLayoutRange*	pSelect,		//!< [out]
	CMemory*		pcmcmWord,		//!< [out]
	CMemory*		pcmcmWordLeft	//!< [out]
)
{
	CLayout* pLayout = SearchLineByLayoutY( nLineNum );
	if( NULL == pLayout ){
		return false;
	}

	// 現在位置の単語の範囲を調べる
	int nFromX;
	int nToX;
	bool nRetCode = m_pcDocLineMgr->WhereCurrentWord(
		pLayout->m_ptLogicPos.y,
		pLayout->m_ptLogicPos.x + nIdx,
		&nFromX,
		&nToX,
		pcmcmWord,
		pcmcmWordLeft
	);

	if( nRetCode ){
		/* 論理位置→レイアウト位置変換 */
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_ptLogicPos.y, nFromX, &pSelect->m_ptFrom.y, &pSelect->m_ptFrom.x );
		XYLogicalToLayout( pLayout, nLineNum, pLayout->m_ptLogicPos.y, nToX, &pSelect->m_ptTo.y, &pSelect->m_ptTo.x );
	}
	return nRetCode;

}





/* 現在位置の左右の単語の先頭位置を調べる */
int CLayoutMgr::PrevOrNextWord(
	int				nLineNum,
	int				nIdx,
	CLayoutPoint*	pptLayoutNew,
	BOOL			bLEFT,
	BOOL			bStopsBothEnds
)
{
	CLayout*  pLayout = SearchLineByLayoutY( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}

	// 現在位置の左右の単語の先頭位置を調べる
	int nRetCode = m_pcDocLineMgr->PrevOrNextWord(
		pLayout->m_ptLogicPos.y,
		pLayout->m_ptLogicPos.x + nIdx,
		&pptLayoutNew->x,
		bLEFT,
		bStopsBothEnds
	);

	if( nRetCode ){
		/* 論理位置→レイアウト位置変換 */
		XYLogicalToLayout(
			pLayout,
			nLineNum,
			pLayout->m_ptLogicPos.y,
			pptLayoutNew->x,
			&pptLayoutNew->y,
			&pptLayoutNew->x
		);
	}
	return nRetCode;
}





//! 単語検索
/*
	@retval 0 見つからない
*/
int CLayoutMgr::SearchWord(
	int						nLineNum, 			//!< [in] 検索開始行
	int						nIdx, 				//!< [in] 検索開始位置
	const char*				pszPattern,			//!< [in] 検索条件
	ESearchDirection		eSearchDirection,	//!< [in] 検索方向
	const SSearchOption&	sSearchOption,		//!< [in] 検索オプション
	int*					pnLineFrom, 		//!< [out] マッチレイアウト行from
	int*					pnIdxFrom, 			//!< [out] マッチレイアウト位置from
	int*					pnLineTo, 			//!< [out] マッチレイアウト行to
	int*					pnIdxTo,  			//!< [out] マッチレイアウト位置to
	CBregexp*				pRegexp				//!< [in] 正規表現コンパイルデータ	Jun. 26, 2001 genta
)
{
	int			nRetCode;
	int			nLogLine;
	CLayout*	pLayout;
	pLayout = SearchLineByLayoutY( nLineNum );
	if( NULL == pLayout ){
		return FALSE;
	}

	// 単語検索
	nRetCode = m_pcDocLineMgr->SearchWord(
		pLayout->m_ptLogicPos.y,
		pLayout->m_ptLogicPos.x + nIdx,
		pszPattern,
		eSearchDirection,
		sSearchOption,
		pnLineFrom,
		pnIdxFrom,
		pnIdxTo,
		pRegexp			/* 正規表現コンパイルデータ */
	);

	// 論理位置→レイアウト位置変換
	if( nRetCode ){
		nLogLine = *pnLineFrom;
		LogicToLayout(
			*pnIdxFrom,
			nLogLine,
			pnIdxFrom,
			pnLineFrom
		);
		LogicToLayout(
			*pnIdxTo,
			nLogLine,
			pnIdxTo,
			pnLineTo
		);
	}
	return nRetCode;
}





/* 論理位置→レイアウト位置変換 */
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
	/* 範囲内先頭に該当するレイアウト情報をサーチ */
	if( (pLayoutInThisArea->m_ptLogicPos.y > nLogLine) ||
		(pLayoutInThisArea->m_ptLogicPos.y == nLogLine &&
		 pLayoutInThisArea->m_ptLogicPos.x >	nLogIdx)
	){
		/* 現在位置より前方に向かってサーチ */
		pLayout =  pLayoutInThisArea->m_pPrev;
		nCurLayLine = nLayoutLineOfThisArea - 1;
		while( pLayout != NULL ){
			if( pLayout->m_ptLogicPos.y == nLogLine &&
				pLayout->m_ptLogicPos.x <= nLogIdx &&
				nLogIdx <= pLayout->m_ptLogicPos.x + pLayout->m_nLength
			){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_ptLogicPos.x;
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
		/* 現在位置を含む後方に向かってサーチ */
		pLayout =  pLayoutInThisArea;
		nCurLayLine = nLayoutLineOfThisArea;
		while( pLayout != NULL ){
			if( pLayout->m_ptLogicPos.y == nLogLine &&
				pLayout->m_ptLogicPos.x <= nLogIdx  &&
				nLogIdx <= pLayout->m_ptLogicPos.x + pLayout->m_nLength
			 ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_ptLogicPos.x;
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
			if( pLayout->m_pNext->m_ptLogicPos.y > nLogLine ){
				*pnLayLine = nCurLayLine;
				*pnLayIdx = nLogIdx - pLayout->m_ptLogicPos.x;
				return;
			}
			pLayout = pLayout->m_pNext;
			nCurLayLine++;
		}
	}
	return;
}


/*!
	@brief カーソル位置変換 物理→レイアウト

	物理位置(行頭からのバイト数、折り返し無し行位置)
	→レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)

	@date 2004.06.16 Moca インデント表示の際のTABを含む行の座標ずれ修正
	@date 2007.09.06 kobake 関数名をCaretPos_Phys2LogからLogicToLayoutに変更
*/
void CLayoutMgr::LogicToLayout(
	int		nX,				//!< [in] 物理位置X
	int		nY,				//!< [in] 物理位置Y
	int*	pnCaretPosX,	//!< [out] 論理位置X
	int*	pnCaretPosY		//!< [out] 論理位置Y
)
{
	int				nCaretPosX = 0;
	int				nCaretPosY;
	const CLayout*	pLayout;
	int				nCharChars;

	*pnCaretPosX = 0;
	*pnCaretPosY = 0;

	// 改行単位行 <= 折り返し単位行 が成り立つから、
	// サーチ開始地点をできるだけ目的地へ近づける
	pLayout = SearchLineByLayoutY( nY );
	if( !pLayout ){
		if( 0 < m_nLines ){
			*pnCaretPosY = m_nLines;
		}
		return;
	}
	nCaretPosY = nY;

	//	Layoutを１つずつ先に進めながらnYが物理行に一致するLayoutを探す
	do{
		if( nY == pLayout->m_ptLogicPos.y ){
			//	2004.06.16 Moca インデント表示の際に位置がずれる(TAB位置ずれによる)
			//	TAB幅を正確に計算するには当初からインデント分を加えておく必要がある．
			nCaretPosX = pLayout->GetIndent();
			const char*		pData;
			pData = pLayout->m_pCDocLine->m_cLine.GetStringPtr() + pLayout->m_ptLogicPos.x; // 2002/2/10 aroka CMemory変更
			int	nDataLen = pLayout->m_nLength;

			int i;
			for( i = 0; i < nDataLen; ++i ){
				if( pLayout->m_ptLogicPos.x + i >= nX ){
					break;
				}
				if( pData[i] ==	TAB ){
					// Sep. 23, 2002 genta メンバー関数を使うように
					nCharChars = GetActualTabSpace( nCaretPosX );
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pData, nDataLen, i );
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
				//	nX, nYがこの行の中に見つかったらループ打ち切り
				break;
			}

			if( !pLayout->m_pNext ){
				//	当該位置に達していなくても，レイアウト末尾ならデータ末尾を返す．
				nCaretPosX += ( nDataLen - i );
				break;
			}

			if( nY < pLayout->m_pNext->m_ptLogicPos.y ){
				//	次のLayoutが当該物理行を過ぎてしまう場合はデータ末尾を返す．
				nCaretPosX += ( nDataLen - i );
				break;
			}
		}
		if( nY < pLayout->m_ptLogicPos.y ){
			//	ふつうはここには来ないと思うが... (genta)
			//	Layoutの指す物理行が探している行より先を指していたら打ち切り
			break;
		}

		//	次の行へ進む
		nCaretPosY++;
		pLayout = pLayout->m_pNext;
	}
	while( pLayout );

	//	2004.06.16 Moca インデント表示の際の位置ずれ修正
	*pnCaretPosX = pLayout ? nCaretPosX : 0;
	*pnCaretPosY = nCaretPosY;
}

/*!
	@brief カーソル位置変換  レイアウト→物理

	レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	→物理位置(行頭からのバイト数、折り返し無し行位置)

	@date 2007.09.06 kobake 関数名をCaretPos_Log2Phys→LayoutToLogicに変更
*/
void CLayoutMgr::LayoutToLogic(
	int		nCaretPosX,		//!< [in] 論理位置X
	int		nCaretPosY,		//!< [in] 論理位置Y
	int*	pnX,			//!< [out] 物理位置X
	int*	pnY				//!< [out] 物理位置Y
)
{
	int				nCharKetas;

	*pnX = 0;
	*pnY = 0;
	if( nCaretPosY > m_nLines ){
		*pnX = 0;
		*pnY = m_nLines;
		return;
	}

	int				nDataLen;
	const char*		pData;

	BOOL			bEOF = FALSE;
	int				nX;
	const CLayout*	pcLayout = SearchLineByLayoutY( nCaretPosY );
	if( !pcLayout ){
		if( 0 < nCaretPosY ){
			pcLayout = SearchLineByLayoutY( nCaretPosY - 1 );
			if( NULL == pcLayout ){
				*pnX = 0;
				*pnY = m_pcDocLineMgr->GetLineCount(); // 2002/2/10 aroka CDocLineMgr変更
				return;
			}
			else{
				pData = GetLineStr( nCaretPosY - 1, &nDataLen );
				if( pData[nDataLen - 1] == '\r' || pData[nDataLen - 1] == '\n' ){
					*pnX = 0;
					*pnY = m_pcDocLineMgr->GetLineCount(); // 2002/2/10 aroka CDocLineMgr変更
					return;
				}
				else{
					*pnY = m_pcDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka CDocLineMgr変更
					bEOF = TRUE;
					nX = 999999;
					goto checkloop;

				}
			}
		}
		*pnX = 0;
		*pnY = m_nLines;
		return;
	}
	else{
		*pnY = pcLayout->m_ptLogicPos.y;
	}

	pData = GetLineStr( nCaretPosY, &nDataLen );
	nX = pcLayout ? pcLayout->GetIndent() : 0;
checkloop:;
	int			i;
	for( i = 0; i < nDataLen; ++i )
	{
		if( pData[i] ==	TAB ){
			// Sep. 23, 2002 genta メンバー関数を使うように
			nCharKetas = GetActualTabSpace( nX );
		}
		else{
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharKetas = CMemory::GetSizeOfChar( pData, nDataLen, i );
		}

		if( nCharKetas == 0 ){
			nCharKetas = 1;
		}

		nX += nCharKetas;
		if( nX > nCaretPosX && !bEOF ){
			break;
		}

		if( pData[i] ==	TAB ){
			nCharKetas = 1;
		}
		i += nCharKetas - 1;
	}
	i += pcLayout->m_ptLogicPos.x;
	*pnX = i;
	return;
}


/* テスト用にレイアウト情報をダンプ */
void CLayoutMgr::DUMP()
{
#ifdef _DEBUG
	const char* pData;
	int nDataLen;
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
		pLayoutNext = pLayout->m_pNext;
		MYTRACE( _T("\t-------\n") );
		MYTRACE( _T("\tthis=%08lxh\n"), pLayout );
		MYTRACE( _T("\tm_pPrev =%08lxh\n"),		pLayout->m_pPrev );
		MYTRACE( _T("\tm_pNext =%08lxh\n"),		pLayout->m_pNext );
		MYTRACE( _T("\tm_ptLogicPos.y=%d\n"),	pLayout->m_ptLogicPos.y );
		MYTRACE( _T("\tm_ptLogicPos.x=%d\n"),	pLayout->m_ptLogicPos.x );
		MYTRACE( _T("\tm_nLength=%d\n"),		pLayout->m_nLength );
		MYTRACE( _T("\tm_enumEOLType =%s\n"),	pLayout->m_cEol.GetName() );
		MYTRACE( _T("\tm_nEOLLen =%d\n"),		pLayout->m_cEol.GetLen() );
		MYTRACE( _T("\tm_nTypePrev=%d\n"),		pLayout->m_nTypePrev );
		pData = m_pcDocLineMgr->GetLineStr( pLayout->m_ptLogicPos.y, &nDataLen );
		MYTRACE( _T("\t[%s]\n"), pData );
		pLayout = pLayoutNext;
	}
	MYTRACE( _T("------------------------\n") );
#endif
	return;
}


/*[EOF]*/
