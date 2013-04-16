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
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CLayoutMgr.h"
#include "charset/charcode.h"
#include "debug/CRunningTimer.h"
#include "doc/CLayout.h"/// 2002/2/10 aroka
#include "doc/CDocLine.h"/// 2002/2/10 aroka
#include "doc/CDocLineMgr.h"/// 2002/2/10 aroka
#include "doc/CDocReader.h" // for _DEBUG
#include "mem/CMemory.h"/// 2002/2/10 aroka
#include "mem/CMemoryIterator.h" // 2006.07.29 genta
#include "basis/SakuraBasis.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "CSearchAgent.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CLayoutMgr::CLayoutMgr()
: m_getIndentOffset( &CLayoutMgr::getIndentOffset_Normal )	//	Oct. 1, 2002 genta	//	Nov. 16, 2002 メンバー関数ポインタにはクラス名が必要
{
	m_pcDocLineMgr = NULL;
	m_sTypeConfig.m_bWordWrap = true;				// 英文ワードラップをする
	m_sTypeConfig.m_nTabSpace = CLayoutInt(8);		/* TAB文字スペース */
	m_sTypeConfig.m_nStringType = 0;				/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
	m_sTypeConfig.m_bKinsokuHead = false;			// 行頭禁則				//@@@ 2002.04.08 MIK
	m_sTypeConfig.m_bKinsokuTail = false;			// 行末禁則				//@@@ 2002.04.08 MIK
	m_sTypeConfig.m_bKinsokuRet  = false;			// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	m_sTypeConfig.m_bKinsokuKuto = false;			// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	m_pszKinsokuHead_1.clear();						/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_1.clear();						/* 行末禁則 */	//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();						/* 句読点ぶらさげ */	//@@@ 2002.04.17 MIK

	// 2005.11.21 Moca 色分けフラグをメンバで持つ
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = false; 
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;
	m_sTypeConfig.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;

	m_nTextWidth = CLayoutInt(0);			// テキスト最大幅の記憶		// 2009.08.28 nasukoji
	m_nTextWidthMaxLine = CLayoutInt(0);	// 最大幅のレイアウト行		// 2009.08.28 nasukoji

	Init();
}

CLayoutMgr::~CLayoutMgr()
{
	_Empty();

	m_pszKinsokuHead_1.clear();	/* 行頭禁則 */
	m_pszKinsokuTail_1.clear();	/* 行末禁則 */	//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();	/* 句読点ぶらさげ */	//@@@ 2002.04.17 MIK
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
	m_nPrevReferLine = CLayoutInt(0);
	m_pLayoutPrevRefer = NULL;
	m_nLines = CLayoutInt(0);			/* 全物理行数 */
	m_nLineTypeBot = COLORIDX_DEFAULT;

	// EOFレイアウト位置記憶	//2006.10.07 Moca
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




/*
|| レイアウト情報の変更
*/
void CLayoutMgr::SetLayoutInfo(
	bool				bDoRayout,
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

	//句読点ぶら下げ文字	// 2009.08.07 ryoji
	//refType.m_szKinsokuKuto → m_pszKinsokuKuto_1
	m_pszKinsokuKuto_1.clear();
	if(m_sTypeConfig.m_bKinsokuKuto){	// 2009.08.06 ryoji m_bKinsokuKutoで振り分ける(Fix)
		for( const wchar_t* p = refType.m_szKinsokuKuto; *p; p++ ){
			m_pszKinsokuKuto_1.push_back_unique(*p);
		}
	}

	//行頭禁則文字
	//refType.m_szKinsokuHead → (句読点以外) m_pszKinsokuHead_1
	m_pszKinsokuHead_1.clear();
	for( const wchar_t* p = refType.m_szKinsokuHead; *p; p++ ){
		if(m_pszKinsokuKuto_1.exist(*p)){
			continue;
		}
		else{
			m_pszKinsokuHead_1.push_back_unique(*p);
		}
	}

	//行末禁則文字
	//refType.m_szKinsokuTail → m_pszKinsokuTail_1
	m_pszKinsokuTail_1.clear();
	for( const wchar_t* p = refType.m_szKinsokuTail; *p; p++ ){
		m_pszKinsokuTail_1.push_back_unique(*p);
	}

	//レイアウト
	if( bDoRayout ){
		_DoLayout();
	}
}




/*!
	@brief 指定された物理行のレイアウト情報を取得

	@param nLineNum [in] 物理行番号 (0～)
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

	//	Mar. 19, 2003 Moca nLineNumが負の場合のチェックを追加
	if( CLayoutInt(0) > nLineNum || nLineNum >= m_nLines ){
		if( CLayoutInt(0) > nLineNum ){
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


	/*+++++++わずかに高速版+++++++*/
	// 2004.03.28 Moca m_pLayoutPrevReferより、Top,Botのほうが近い場合は、そちらを利用する
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


//@@@ 2002.09.23 YAZAKI CLayout*を作成するところは分離して、InsertLineNext()と共通化
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

//@@@ 2002.09.23 YAZAKI CLayout*を作成するところは分離して、AddLineBottom()と共通化
CLayout* CLayoutMgr::InsertLineNext( CLayout* pLayoutPrev, CLayout* pLayout )
{
	CLayout* pLayoutNext;

	if(	CLayoutInt(0) == m_nLines ){
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
	if( NULL == pLayoutPrev->GetNextLayout() ){
		/* 最後に挿入 */
		m_pLayoutBot->m_pNext = pLayout;
		pLayout->m_pPrev = m_pLayoutBot;
		pLayout->m_pNext = NULL;
		m_pLayoutBot = pLayout;
	}else{
		/* 途中に挿入 */
		pLayoutNext = pLayoutPrev->GetNextLayout();
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
		pLayout->m_cEol.SetType( EOL_NONE );/* 改行コードの種類 */
	}else{
		if( pLayout->GetLogicOffset() + pLayout->GetLengthWithEOL() >
			pCDocLine->GetLengthWithEOL() - pCDocLine->GetEol().GetLen()
		){
			pLayout->m_cEol = pCDocLine->GetEol();/* 改行コードの種類 */
		}else{
			pLayout->m_cEol = EOL_NONE;/* 改行コードの種類 */
		}
	}

	// 2009.08.28 nasukoji	「折り返さない」選択時のみレイアウト長を記憶する
	// 「折り返さない」以外で計算しないのはパフォーマンス低下を防ぐ目的なので、
	// パフォーマンスの低下が気にならない程なら全ての折り返し方法で計算する
	// ようにしても良いと思う。
	// （その場合CLayoutMgr::CalculateTextWidth()の呼び出し箇所をチェック）
	pLayout->SetLayoutWidth( ( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ) ? nPosX : CLayoutInt(0) );

	return pLayout;
}


/*
|| 指定された物理行のデータへのポインタとその長さを返す Ver0

	@date 2002/2/10 aroka CMemory変更
*/
const wchar_t* CLayoutMgr::GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen ) const //#####いらんやろ
{
	const CLayout* pLayout;
	if( NULL == ( pLayout = SearchLineByLayoutY( nLine )	) ){
		return NULL;
	}
	*pnLineLen = CLogicInt(pLayout->GetLengthWithEOL());
	return pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset();
}

/*!	指定された物理行のデータへのポインタとその長さを返す Ver1
	@date 2002/03/24 YAZAKI GetLineStr( int nLine, int* pnLineLen )と同じ動作に変更。
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
|| 指定された位置がレイアウト行の途中の行末かどうか調べる

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
	{	/* この行に改行はない */
		/* この行の最後か？ */
		if( ptLinePos.x == (Int)pLayout->GetLengthWithEOL() ) return true; //$$ 単位混在
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

	if( CLayoutInt(0) == m_nLines || m_pLayoutBot == NULL ){
		// データが空
		ptLayoutEnd->x = CLayoutInt(0);
		ptLayoutEnd->y = CLayoutInt(0);
		m_nEOFColumn = ptLayoutEnd->x;
		m_nEOFLine = ptLayoutEnd->y;
		return;
	}

	CLayout *btm = m_pLayoutBot;
	if( btm->m_cEol != EOL_NONE ){
		//	末尾に改行がある
		ptLayoutEnd->Set(CLayoutInt(0), GetLineCount());
	}
	else {
		CMemoryIterator it( btm, GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			it.addDelta();
		}
		ptLayoutEnd->Set(it.getColumn(), GetLineCount() - CLayoutInt(1));
		// [EOF]のみ折り返すのはやめる	// 2009.02.17 ryoji
		//// 2006.10.01 Moca Start [EOF]のみのレイアウト行処理が抜けていたバグを修正
		//if( GetMaxLineKetas() <= ptLayoutEnd->GetX2() ){
		//	ptLayoutEnd->SetX(CLayoutInt(0));
		//	ptLayoutEnd->y++;
		//}
		//// 2006.10.01 Moca End
	}
	m_nEOFColumn = ptLayoutEnd->x;
	m_nEOFLine = ptLayoutEnd->y;
}


/*!	行内文字削除

	@date 2002/03/24 YAZAKI bUndo削除
	@date 2009/08/28 nasukoji	テキスト最大幅の算出に対応
*/
void CLayoutMgr::DeleteData_CLayoutMgr(
	CLayoutInt	nLineNum,
	CLogicInt	nDelPos,
	CLogicInt	nDelLen,
	CLayoutInt*	pnModifyLayoutLinesOld,
	CLayoutInt*	pnModifyLayoutLinesNew,
	CLayoutInt*	pnDeleteLayoutLines,
	CNativeW*	cmemDeleted				//!< [out] 削除されたデータ
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
	CLogicInt		nModLineOldFrom;	/* 影響のあった変更前の行(from) */
	CLogicInt		nModLineOldTo;		/* 影響のあった変更前の行(to) */
	CLogicInt		nDelLineOldFrom;	/* 削除された変更前論理行(from) */
	CLogicInt		nDelLineOldNum;		/* 削除された行数 */
	CLogicInt		nRowNum;
	CLogicInt		nDelStartLogicalLine;
	CLogicInt		nDelStartLogicalPos;
	EColorIndexType	nCurrentLineType;
	CLayoutInt		nLineWork;

	/* 現在行のデータを取得 */
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

	/* テキストのデータを削除 */
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

	/*--- 変更された行のレイアウト情報を再生成 ---*/
	/* 論理行の指定範囲に該当するレイアウト情報を削除して */
	/* 削除した範囲の直前のレイアウト情報のポインタを返す */
	CLayoutInt		nAllLinesOld = m_nLines;
	pLayoutPrev = DeleteLayoutAsLogical(
		pLayoutWork,
		nLineWork,
		nModLineOldFrom,
		nModLineOldTo,
		CLogicPoint(nDelStartLogicalPos, nDelStartLogicalLine),
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

	// 2009.08.28 nasukoji	テキスト最大幅算出用の引数を設定
	CalTextWidthArg ctwArg;
	ctwArg.nLineFrom    = nLineNum;				// 編集開始行
	ctwArg.nColmFrom    = CLayoutInt(nDelPos);	// 編集開始桁
	ctwArg.nDelLines    = 0;					// 削除行は1行
	ctwArg.nAllLinesOld = nAllLinesOld;			// 編集前のテキスト行数
	ctwArg.bInsData     = FALSE;				// 追加文字列なし

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
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






/*!	文字列挿入

	@date 2002/03/24 YAZAKI bUndo削除
	@date 2009/08/28 nasukoji	テキスト最大幅の算出に対応
*/
void CLayoutMgr::InsertData_CLayoutMgr(
	CLayoutInt		nLineNum,
	CLogicInt		nInsPos,
	const wchar_t*	pInsData,
	CLogicInt		nInsDataLen,
	CLayoutInt*		pnModifyLayoutLinesOld,
	CLayoutInt*		pnInsLineNum,			// 挿入によって増えたレイアウト行の数
	CLayoutPoint*	pptNewLayout			// 挿入された部分の次の位置
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


	// 現在行のデータを取得 -> pLine, nLineLen
	CLogicInt		nLineLen;
	const wchar_t*	pLine = GetLineStr( nLineNum, &nLineLen );
	if( !pLine ){
		/*
			2004.04.02 FILE / Moca カーソル位置不正のため、空テキストで
			nLineNumが0でないときに落ちる対策．データが空であることを
			カーソル位置ではなく総行数で判定することでより確実に．
		*/
		if( m_nLines == CLayoutInt(0) )
		{
			/* 空のテキストの先頭に行を作る場合 */
			pLayout = NULL;
			nLineWork = CLayoutInt(0);
			nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
			nInsStartLogicalPos  = CLogicInt(0);
			nCurrentLineType = COLORIDX_DEFAULT;
		}
		else{
			using namespace WCODE;

			pLine = GetLineStr( m_nLines - CLayoutInt(1), &nLineLen );
			//終端2文字のどちらかにcr,lfのいずれかが含まれている場合
			if( ( nLineLen > 0 && ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF )) ||
				( nLineLen > 1 && ( pLine[nLineLen - 2] == CR || pLine[nLineLen - 2] == LF )) ){
				// 空でないテキストの最後に行を作る場合
				pLayout = NULL;
				nLineWork = CLayoutInt(0);
				nInsStartLogicalLine = m_pcDocLineMgr->GetLineCount();
				nInsStartLogicalPos  = CLogicInt(0);
				nCurrentLineType = m_nLineTypeBot;
			}
			else{
				/* 空でないテキストの最後の行を変更する場合 */
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


	// データの挿入
	CLogicPoint ptNewPos;	//挿入された部分の次のデータ位置
	CDocEditAgent(m_pcDocLineMgr).InsertData_CDocLineMgr(
		nInsStartLogicalLine,
		nInsStartLogicalPos,
		pInsData,
		nInsDataLen,
		&nInsLineNum,
		&ptNewPos
	);


	//--- 変更された行のレイアウト情報を再生成 ---
	// 論理行の指定範囲に該当するレイアウト情報を削除して
	// 削除した範囲の直前のレイアウト情報のポインタを返す
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

	// 2009.08.28 nasukoji	テキスト最大幅算出用の引数を設定
	CalTextWidthArg ctwArg;
	ctwArg.nLineFrom    = nLineNum;				// 編集開始行
	ctwArg.nColmFrom    = CLayoutInt(nInsPos);	// 編集開始桁
	ctwArg.nDelLines    = -1;					// 削除行なし
	ctwArg.nAllLinesOld = nAllLinesOld;			// 編集前のテキスト行数
	ctwArg.bInsData     = TRUE;					// 追加文字列あり

	// 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする
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

	// 論理位置→レイアウト位置変換
	LogicToLayout( ptNewPos, pptNewLayout );
	return;
}






/* 論理行の指定範囲に該当するレイアウト情報を削除して */
/* 削除した範囲の直前のレイアウト情報のポインタを返す */
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
	if( CLayoutInt(0) == m_nLines){	/* 全物理行数 */
		return NULL;
	}
	if( NULL == pLayoutInThisArea ){
		return NULL;
	}

	// 1999.11.22
	m_pLayoutPrevRefer = pLayoutInThisArea->GetPrevLayout();
	m_nPrevReferLine = nLineOf_pLayoutInThisArea - CLayoutInt(1);


	/* 範囲内先頭に該当するレイアウト情報をサーチ */
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
			/* 先頭行の処理 */
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
//			// 1999.12.22 前にずらすだけでよいのでは
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
	/* レイアウト情報全体を更新する(なな、なんと!!!) */
	while( NULL != pLayout ){
		pLayout->OffsetLogicLineNo(nShiftLines);	/* 対応する論理行番号 */
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





/* 現在位置の単語の範囲を調べる */
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

	// 現在位置の単語の範囲を調べる -> ロジック単位pSelect, pcmemWord, pcmemWordLeft
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
		/* 論理位置→レイアウト位置変換 */
		CLayoutPoint ptFrom;
		LogicToLayout( CLogicPoint(nFromX, pLayout->GetLogicLineNo()), &ptFrom, nLineNum );
		pSelect->SetFrom(ptFrom);

		CLayoutPoint ptTo;
		LogicToLayout( CLogicPoint(nToX, pLayout->GetLogicLineNo()), &ptTo, nLineNum );
		pSelect->SetTo(ptTo);
	}
	return nRetCode;

}





/* 現在位置の左右の単語の先頭位置を調べる */
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

	// 現在位置の左右の単語の先頭位置を調べる
	CLogicInt	nPosNew;
	int			nRetCode = CSearchAgent(m_pcDocLineMgr).PrevOrNextWord(
		pLayout->GetLogicLineNo(),
		pLayout->GetLogicOffset() + nIdx,
		&nPosNew,
		bLEFT,
		bStopsBothEnds
	);

	if( nRetCode ){
		/* 論理位置→レイアウト位置変換 */
		LogicToLayout(
			CLogicPoint(nPosNew,pLayout->GetLogicLineNo()),
			pptLayoutNew,
			nLineNum
		);
	}
	return nRetCode;
}





//! 単語検索
/*
	@retval 0 見つからない
*/
int CLayoutMgr::SearchWord(
	CLayoutInt				nLine,				//!< [in] 検索開始レイアウト行
	CLogicInt				nIdx,				//!< [in] 検索開始データ位置
	const wchar_t*			pszPattern,			//!< [in] 検索条件
	ESearchDirection		eSearchDirection,	//!< [in] 検索方向
	const SSearchOption&	sSearchOption,		//!< [in] 検索オプション
	CLayoutRange*			pMatchRange,		//!< [out] マッチレイアウト範囲
	CBregexp*				pRegexp				//!< [in]  正規表現コンパイルデータ  Jun. 26, 2001 genta
)
{
	int			nRetCode;
	const CLayout*	pLayout;
	pLayout = this->SearchLineByLayoutY( nLine );
	if( NULL == pLayout ){
		return FALSE;
	}

	// 単語検索 -> cLogicRange (データ位置)
	CLogicRange cLogicRange;
	nRetCode = CSearchAgent(m_pcDocLineMgr).SearchWord(
		CLogicPoint(pLayout->GetLogicOffset() + nIdx, pLayout->GetLogicLineNo()),
		pszPattern,
		eSearchDirection,
		sSearchOption,
		&cLogicRange, //pMatchRange,
		pRegexp			/* 正規表現コンパイルデータ */
	);

	// 論理位置→レイアウト位置変換
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
//                        単位の変換                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief カーソル位置変換 物理→レイアウト

	物理位置(行頭からのバイト数、折り返し無し行位置)
	→レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)

	@date 2004.06.16 Moca インデント表示の際のTABを含む行の座標ずれ修正
	@date 2007.09.06 kobake 関数名をCaretPos_Phys2LogからLogicToLayoutに変更
*/
void CLayoutMgr::LogicToLayout(
	const CLogicPoint&	ptLogic,	//!< [in]  ロジック位置
	CLayoutPoint*		pptLayout,	//!< [out] レイアウト位置
	CLayoutInt			nLineHint	//!< [in]  レイアウトY値のヒント。求める値に近い値を渡すと高速に検索できる。
)
{
	pptLayout->Clear();

	if(GetLineCount()==0)return; //変換不可

	// サーチ開始地点 -> pLayout, nCaretPosX, nCaretPosY
	CLayoutInt		nCaretPosX = CLayoutInt(0);
	CLayoutInt		nCaretPosY;
	const CLayout*	pLayout;
	if(nLineHint==0){
		nCaretPosY = CLayoutInt(ptLogic.y);

		// [ヒント無しの場合]
		// ロジック行 <= レイアウト行 が成り立つから、
		// サーチ開始地点をできるだけ目的地へ近づける
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

		// [ヒント有りの場合]
		pLayout = SearchLineByLayoutY(nCaretPosY);
		if(!pLayout) pLayout = SearchLineByLayoutY( nCaretPosY = CLayoutInt(0) );
		
		//ロジックYがでかすぎる場合は、一致するまでデクリメント (
		while(pLayout->GetLogicLineNo() > ptLogic.GetY2()){
			pLayout = pLayout->GetPrevLayout();
			nCaretPosY--;
		}

		//ロジックYが同じ場合は、ロジックY内の最小レイアウトYを開始地点とする
		if(pLayout->GetLogicLineNo() == ptLogic.GetY2()){
			while(pLayout->m_pPrev && pLayout->GetPrevLayout()->GetLogicLineNo() == ptLogic.GetY2()){
				pLayout = pLayout->GetPrevLayout();
				nCaretPosY--;
			}
		}

	}


	//	Layoutを１つずつ先に進めながらptLogic.yが物理行に一致するLayoutを探す
	do{
		if( ptLogic.GetY2() == pLayout->GetLogicLineNo() ){
			//	2004.06.16 Moca インデント表示の際に位置がずれる(TAB位置ずれによる)
			//	TAB幅を正確に計算するには当初からインデント分を加えておく必要がある．
			nCaretPosX = pLayout->GetIndent();
			const wchar_t*	pData;
			pData = pLayout->GetDocLineRef()->GetPtr() + pLayout->GetLogicOffset(); // 2002/2/10 aroka CMemory変更
			CLogicInt	nDataLen = (CLogicInt)pLayout->GetLengthWithEOL();

			CLogicInt i;
			for( i = CLogicInt(0); i < nDataLen; ++i ){
				if( pLayout->GetLogicOffset() + i >= ptLogic.x ){
					break;
				}

				//文字ロジック幅 -> nCharChars
				CLogicInt nCharChars = CNativeW::GetSizeOfChar( pData, nDataLen, i );
				if( nCharChars == 0 )
					nCharChars = CLogicInt(1);

				//文字レイアウト幅 -> nCharKetas
				CLayoutInt nCharKetas;
				if( pData[i] ==	WCODE::TAB ){
					// Sep. 23, 2002 genta メンバー関数を使うように
					nCharKetas = GetActualTabSpace( nCaretPosX );
				}
				else{
					nCharKetas = CNativeW::GetKetaOfChar( pData, nDataLen, i );
				}
//				if( nCharKetas == 0 )				// 削除 サロゲートペア対策	2008/7/5 Uchi
//					nCharKetas = CLayoutInt(1);

				//レイアウト加算
				nCaretPosX += nCharKetas;

				//ロジック加算
				if( pData[i] ==	WCODE::TAB ){
					nCharChars = CLogicInt(1);
				}
				i += nCharChars - CLogicInt(1);
			}
			if( i < nDataLen ){
				//	ptLogic.x, ptLogic.yがこの行の中に見つかったらループ打ち切り
				break;
			}

			if( !pLayout->GetNextLayout() ){
				//	当該位置に達していなくても，レイアウト末尾ならデータ末尾のレイアウト位置を返す．
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + CLayoutInt(pLayout->GetLayoutEol().GetLen()>0?1:0);
				break;
			}

			if( ptLogic.y < pLayout->m_pNext->GetLogicLineNo() ){
				//	次のLayoutが当該物理行を過ぎてしまう場合はデータ末尾のレイアウト位置を返す．
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + CLayoutInt(pLayout->GetLayoutEol().GetLen()>0?1:0);
				break;
			}
		}
		if( ptLogic.GetY2() < pLayout->GetLogicLineNo() ){
			//	ふつうはここには来ないと思うが... (genta)
			//	Layoutの指す物理行が探している行より先を指していたら打ち切り
			break;
		}

		//	次の行へ進む
		nCaretPosY++;
		pLayout = pLayout->GetNextLayout();
	}
	while( pLayout );

	//	2004.06.16 Moca インデント表示の際の位置ずれ修正
	pptLayout->Set(
		pLayout ? nCaretPosX : CLayoutInt(0),
		nCaretPosY
	);
}

/*!
	@brief カーソル位置変換  レイアウト→物理

	レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	→物理位置(行頭からのバイト数、折り返し無し行位置)

	@date 2007.09.06 kobake 関数名をCaretPos_Log2Phys→LayoutToLogicに変更
*/
void CLayoutMgr::LayoutToLogicEx(
	const CLayoutPoint&	ptLayout,	//!< [in]  レイアウト位置
	CLogicPointEx*		pptLogic	//!< [out] ロジック位置
) const
{
	pptLogic->Set(CLogicInt(0), CLogicInt(0));
	pptLogic->ext = 0;
	if( ptLayout.GetY2() > m_nLines ){
		//2007.10.11 kobake Y値が間違っていたので修正
		//pptLogic->Set(0, m_nLines);
		pptLogic->Set(CLogicInt(0), m_pcDocLineMgr->GetLineCount());
		return;
	}

	CLogicInt		nDataLen;
	const wchar_t*	pData;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        Ｙ値の決定                           //
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
					pptLogic->y = m_pcDocLineMgr->GetLineCount() - 1; // 2002/2/10 aroka CDocLineMgr変更
					bEOF = TRUE;
					// nX = CLayoutInt(MAXLINEKETAS);
					nX = pcLayout->GetIndent();
					goto checkloop;

				}
			}
		}
		//2007.10.11 kobake Y値が間違っていたので修正
		//pptLogic->Set(0, m_nLines);
		pptLogic->Set(CLogicInt(0), m_pcDocLineMgr->GetLineCount());
		return;
	}
	else{
		pptLogic->y = pcLayout->GetLogicLineNo();
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        Ｘ値の決定                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	pData = GetLineStr( ptLayout.GetY2(), &nDataLen );
	nX = pcLayout ? pcLayout->GetIndent() : CLayoutInt(0);

checkloop:;
	CLogicInt	i;
	for( i = CLogicInt(0); i < nDataLen; ++i )
	{
		//文字ロジック幅 -> nCharChars
		CLogicInt	nCharChars;
		nCharChars = CNativeW::GetSizeOfChar( pData, nDataLen, i );
		if( nCharChars == 0 )
			nCharChars = CLogicInt(1);
		
		//文字レイアウト幅 -> nCharKetas
		CLayoutInt	nCharKetas;
		if( pData[i] == WCODE::TAB ){
			nCharKetas = GetActualTabSpace( nX );
		}
		else{
			nCharKetas = CNativeW::GetKetaOfChar( pData, nDataLen, i );
		}
//		if( nCharKetas == 0 )				// 削除 サロゲートペア対策	2008/7/5 Uchi
//			nCharKetas = CLayoutInt(1);

		//レイアウト加算
		if( nX + nCharKetas > ptLayout.GetX2() && !bEOF ){
			break;
		}
		nX += nCharKetas;

		//ロジック加算
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
//                         デバッグ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* テスト用にレイアウト情報をダンプ */
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
#endif _DEBUG
	return;
}



