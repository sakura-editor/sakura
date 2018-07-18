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
//                        生成と破棄                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CLayoutMgr::CLayoutMgr()
: m_getIndentOffset( &CLayoutMgr::getIndentOffset_Normal )	//	Oct. 1, 2002 genta	//	Nov. 16, 2002 メンバー関数ポインタにはクラス名が必要
{
	m_pcDocLineMgr = NULL;
	m_pTypeConfig = NULL;
	m_nMaxLineKetas = CKetaXInt(MAXLINEKETAS);
	m_nTabSpace = CKetaXInt(4);
	m_tsvInfo.m_nTsvMode = TSV_MODE_NONE;
	m_pszKinsokuHead_1.clear();						/* 行頭禁則 */	//@@@ 2002.04.08 MIK
	m_pszKinsokuTail_1.clear();						/* 行末禁則 */	//@@@ 2002.04.08 MIK
	m_pszKinsokuKuto_1.clear();						/* 句読点ぶらさげ */	//@@@ 2002.04.17 MIK

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
	_Empty();
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




/*! レイアウト情報の変更
	@param bDoLayout [in] レイアウト情報の再作成
	@param refType [in] タイプ別設定
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

	//タイプ別設定
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
		// Viewが持ってるフォント情報は古い、しょうがないので自分で作る
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
	// 最大文字幅の計算
	m_tsvInfo.m_nMaxCharLayoutX = WCODE::CalcPxWidthByFont(L'W');
	if (m_tsvInfo.m_nMaxCharLayoutX < m_nCharLayoutXPerKeta) {
		m_tsvInfo.m_nMaxCharLayoutX = m_nCharLayoutXPerKeta;
	}

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
	if(refType.m_bKinsokuKuto){	// 2009.08.06 ryoji m_bKinsokuKutoで振り分ける(Fix)
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
	if( bDoLayout ){
		_DoLayout(bBlockingHook);
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
		CMemoryIterator it = CreateCMemoryIterator(btm);
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
	ESearchDirection		eSearchDirection,	//!< [in] 検索方向
	CLayoutRange*			pMatchRange,		//!< [out] マッチレイアウト範囲
	const CSearchStringPattern&	pattern
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
		eSearchDirection,
		&cLogicRange, //pMatchRange,
		pattern
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

	@param[in]  ptLogic   ロジック位置
	@param[out] pptLayout レイアウト位置
	@param[in]  nLineHint レイアウトY値のヒント。求める値に近い値を渡すと高速に検索できる。

	@date 2004.06.16 Moca インデント表示の際のTABを含む行の座標ずれ修正
	@date 2007.09.06 kobake 関数名をCaretPos_Phys2LogからLogicToLayoutに変更
*/
void CLayoutMgr::LogicToLayout(
	const CLogicPoint&	ptLogic,
	CLayoutPoint*		pptLayout,
	CLayoutInt			nLineHint
)
{
	pptLayout->Clear();

	if(GetLineCount()==0)return; //変換不可

	// サーチ開始地点 -> pLayout, nCaretPosX, nCaretPosY
	CLayoutInt		nCaretPosX = CLayoutInt(0);
	CLayoutInt		nCaretPosY;
	const CLayout*	pLayout;
	// 2013.05.15 ヒント、ありなしの処理を統合
	{
		nLineHint = t_min(GetLineCount() - 1, nLineHint);
		nCaretPosY = t_max(CLayoutInt(ptLogic.y), nLineHint);

		// 2013.05.12 m_pLayoutPrevReferを見る
		if( nCaretPosY <= m_nPrevReferLine && m_pLayoutPrevRefer
			&& m_pLayoutPrevRefer->GetLogicLineNo() <= ptLogic.y ){
			// ヒントより現在位置のほうが後ろか同じぐらいで近い
			nCaretPosY = CLayoutInt(ptLogic.y - m_pLayoutPrevRefer->GetLogicLineNo()) + m_nPrevReferLine;
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}else{
			pLayout = SearchLineByLayoutY(nCaretPosY);
		}
		if( !pLayout ){
			pptLayout->SetY( m_nLines );
			return;
		}

		//ロジックYがでかすぎる場合は、一致するまでデクリメント (
		while(pLayout->GetLogicLineNo() > ptLogic.GetY2()){
			pLayout = pLayout->GetPrevLayout();
			nCaretPosY--;
		}

		//ロジックYが同じでOffsetが行き過ぎている場合は戻る
		if(pLayout->GetLogicLineNo() == ptLogic.GetY2()){
			while( pLayout && pLayout->GetPrevLayout() && pLayout->GetPrevLayout()->GetLogicLineNo() == ptLogic.GetY2()
				&& ptLogic.x < pLayout->GetLogicOffset() ){
				pLayout = pLayout->GetPrevLayout();
				nCaretPosY--;
			}
		}
	}


	//	Layoutを１つずつ先に進めながらptLogic.yが物理行に一致するLayoutを探す
	while( pLayout ){
		if( ptLogic.GetY2() == pLayout->GetLogicLineNo() ){
			// 2013.05.10 Moca 高速化
			const CLayout* pLayoutNext = pLayout->GetNextLayout();
			if( pLayoutNext && ptLogic.GetY2() ==pLayoutNext->GetLogicLineNo()
					&& pLayoutNext->GetLogicOffset() <= ptLogic.x ){
				nCaretPosY++;
				pLayout = pLayout->GetNextLayout();
				continue;
			}

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
				if( pData[i] ==	WCODE::TAB || ( pData[i] == L',' && m_tsvInfo.m_nTsvMode == TSV_MODE_CSV) ){
					nCharKetas = GetActualTsvSpace( nCaretPosX, pData[i] );
				}
				else{
					nCharKetas = GetLayoutXOfChar( pData, nDataLen, i );
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
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + CLayoutInt(pLayout->GetLayoutEol().GetLen()>0?1+m_nSpacing:0);
				break;
			}

			if( ptLogic.y < pLayout->m_pNext->GetLogicLineNo() ){
				//	次のLayoutが当該物理行を過ぎてしまう場合はデータ末尾のレイアウト位置を返す．
				nCaretPosX = pLayout->CalcLayoutWidth(*this) + CLayoutInt(pLayout->GetLayoutEol().GetLen()>0?1+m_nSpacing:0);
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
	pptLogic->haba = m_nCharLayoutXPerKeta;
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
				if( WCODE::IsLineDelimiter(pData[nDataLen - 1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
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
		if( pData[i] == WCODE::TAB || (pData[i] == L',' && m_tsvInfo.m_nTsvMode == TSV_MODE_CSV ) ){
			nCharKetas = GetActualTsvSpace( nX, pData[i] );
		}
		else{
			nCharKetas = GetLayoutXOfChar( pData, nDataLen, i );
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



