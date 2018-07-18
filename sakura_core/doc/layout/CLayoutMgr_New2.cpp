/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK, aroka
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include <stdlib.h>
#include "CLayoutMgr.h"
#include "CLayout.h" // 2002/2/10 aroka
#include "doc/logic/CDocLineMgr.h" // 2002/2/10 aroka
#include "charset/charcode.h"
#include "CSearchAgent.h"



/* 文字列置換 */
void CLayoutMgr::ReplaceData_CLayoutMgr(
	LayoutReplaceArg*	pArg
)
{
	CLayoutInt	nWork_nLines = m_nLines;	//変更前の全行数の保存	@@@ 2002.04.19 MIK

	/* 置換先頭位置のレイアウト情報 */
	EColorIndexType	nCurrentLineType = COLORIDX_DEFAULT;
	CLayoutColorInfo*	colorInfo = NULL;
	CLayoutInt		nLineWork = pArg->sDelRange.GetFrom().GetY2();

	CLayout*		pLayoutWork = SearchLineByLayoutY( pArg->sDelRange.GetFrom().GetY2() );
	if( pLayoutWork ){
		while( 0 != pLayoutWork->GetLogicOffset() ){
			pLayoutWork = pLayoutWork->GetPrevLayout();
			nLineWork--;
		}
		nCurrentLineType = pLayoutWork->GetColorTypePrev();
		colorInfo = pLayoutWork->GetLayoutExInfo()->DetachColorInfo();
	}else if( GetLineCount() == pArg->sDelRange.GetFrom().GetY2() ){
		// 2012.01.05 最終行のRedo/Undoでの色分けが正しくないのを修正
		nCurrentLineType = m_nLineTypeBot;
		colorInfo = m_cLayoutExInfoBot.DetachColorInfo();
	}


	/*
	||  カーソル位置変換
	||  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置) →
	||  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	CLogicPoint ptFrom;
	CLogicPoint ptTo;
	LayoutToLogic( pArg->sDelRange.GetFrom(), &ptFrom );
	LayoutToLogic( pArg->sDelRange.GetTo(), &ptTo );

	/* 指定範囲のデータを置換(削除 & データを挿入)
	  Fromを含む位置からToの直前を含むデータを削除する
	  Fromの位置へテキストを挿入する
	*/
	DocLineReplaceArg DLRArg;
	DLRArg.sDelRange.SetFrom(ptFrom);	//削除範囲from
	DLRArg.sDelRange.SetTo(ptTo);		//削除範囲to
	DLRArg.pcmemDeleted = pArg->pcmemDeleted;	// 削除されたデータを保存
	DLRArg.pInsData = pArg->pInsData;			// 挿入するデータ
	DLRArg.nDelSeq = pArg->nDelSeq;
	CSearchAgent(m_pcDocLineMgr).ReplaceData(
		&DLRArg
	);
	pArg->nInsSeq = DLRArg.nInsSeq;


	/*--- 変更された行のレイアウト情報を再生成 ---*/
	/* 論理行の指定範囲に該当するレイアウト情報を削除して */
	/* 削除した範囲の直前のレイアウト情報のポインタを返す */

	CLayoutInt	nModifyLayoutLinesOld = CLayoutInt(0);
	CLayout* pLayoutPrev;
	CLogicInt nWork;
	nWork = t_max( DLRArg.nDeletedLineNum, DLRArg.nInsLineNum );


	if( pLayoutWork ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,
			ptFrom.GetY2(),
			ptFrom.GetY2() + nWork,
			ptFrom,
			&nModifyLayoutLinesOld
		);

		/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */
		/* 論理行が削除された場合は０より小さい行数 */
		/* 論理行が挿入された場合は０より大きい行数 */
		if( 0 != DLRArg.nInsLineNum - DLRArg.nDeletedLineNum ){
			ShiftLogicalLineNum(
				pLayoutPrev,
				DLRArg.nInsLineNum - DLRArg.nDeletedLineNum
			);
		}
	}else{
		pLayoutPrev = m_pLayoutBot;
	}

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	CLogicInt	nRowNum;
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
	ctwArg.ptLayout     = pArg->sDelRange.GetFrom();		// 編集開始位置
	ctwArg.nDelLines    = pArg->sDelRange.GetTo().GetY2() - pArg->sDelRange.GetFrom().GetY2();	// 削除行数 - 1
	ctwArg.nAllLinesOld = nWork_nLines;								// 編集前のテキスト行数
	ctwArg.bInsData     = (pArg->pInsData && pArg->pInsData->size()) ? TRUE : FALSE;			// 追加文字列の有無

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	CLayoutInt nAddInsLineNum;
	pArg->nModLineTo = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		ptFrom,
		nCurrentLineType,
		colorInfo,
		&ctwArg,
		&nAddInsLineNum
	);

	pArg->nAddLineNum = m_nLines - nWork_nLines;	//変更後の全行数との差分	@@@ 2002.04.19 MIK
	if( 0 == pArg->nAddLineNum )
		pArg->nAddLineNum = nModifyLayoutLinesOld - pArg->nModLineTo;	/* 再描画ヒント レイアウト行の増減 */
	pArg->nModLineFrom = pArg->sDelRange.GetFrom().GetY2();	/* 再描画ヒント 変更されたレイアウト行From */
	pArg->nModLineTo += ( pArg->nModLineFrom - CLayoutInt(1) ) ;	/* 再描画ヒント 変更されたレイアウト行To */

	//2007.10.18 kobake LayoutReplaceArg::ptLayoutNewはここで算出するのが正しい
	LogicToLayout(DLRArg.ptNewPos, &pArg->ptLayoutNew); // 挿入された部分の次の位置
}



