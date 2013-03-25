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
#include "CLayoutMgr.h"
#include "charcode.h"
#include "etc_uty.h"
#include "Debug.h"
#include <commctrl.h>
#include <stdlib.h>
#include "CLayout.h" // 2002/2/10 aroka
#include "CDocLineMgr.h" // 2002/2/10 aroka
#include "CEditDoc.h"		// 2009.08.28 nasukoji



/* 文字列置換 */
void CLayoutMgr::ReplaceData_CLayoutMgr(
	LayoutReplaceArg*	pArg
)
{
	int	nxFrom;
	int	nyFrom;
	int	nxTo;
	int	nyTo;
	int nLineWork;

	int	nWork_nLines = m_nLines;	//変更前の全行数の保存	@@@ 2002.04.19 MIK

	/* 置換先頭位置のレイアウト情報 */
	CLayout* pLayout = (CLayout*)SearchLineByLayoutY( pArg->nDelLineFrom );
	EColorIndexType nCurrentLineType = COLORIDX_DEFAULT;
	CLayout* pLayoutWork = pLayout;
	nLineWork = pArg->nDelLineFrom;

	if( pLayoutWork ){
		while( 0 != pLayoutWork->m_nOffset ){
			pLayoutWork = pLayoutWork->m_pPrev;
			nLineWork--;
		}
		nCurrentLineType = pLayoutWork->m_nTypePrev;
	}


	/*
	||  カーソル位置変換
	||  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置) →
	||  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	LayoutToLogic( pArg->nDelColmFrom, pArg->nDelLineFrom, &nxFrom, &nyFrom );
	LayoutToLogic( pArg->nDelColmTo, pArg->nDelLineTo, &nxTo, &nyTo );

	/* 指定範囲のデータを置換(削除 & データを挿入)
	  Fromを含む位置からToの直前を含むデータを削除する
	  Fromの位置へテキストを挿入する
	*/
	DocLineReplaceArg DLRArg;
	DLRArg.nDelLineFrom = nyFrom;			// 削除範囲行  From 改行単位の行番号 0開始)
	DLRArg.nDelPosFrom = nxFrom;			// 削除範囲位置From 改行単位の行頭からのバイト位置 0開始)
	DLRArg.nDelLineTo = nyTo;				// 削除範囲行  To   改行単位の行番号 0開始)
	DLRArg.nDelPosTo = nxTo;				// 削除範囲位置To   改行単位の行頭からのバイト位置 0開始)
	DLRArg.pcmemDeleted = pArg->pcmemDeleted;	// 削除されたデータを保存
	DLRArg.pInsData = pArg->pInsData;			// 挿入するデータ
	DLRArg.nInsDataLen = pArg->nInsDataLen;		// 挿入するデータの長さ
	m_pcDocLineMgr->ReplaceData(
		&DLRArg
	);
	pArg->nNewLine = DLRArg.nNewLine;			/* 挿入された部分の次の位置の行 */
	pArg->nNewPos = DLRArg.nNewPos;				/* 挿入された部分の次の位置のデータ位置 */


	/*--- 変更された行のレイアウト情報を再生成 ---*/
	/* 論理行の指定範囲に該当するレイアウト情報を削除して */
	/* 削除した範囲の直前のレイアウト情報のポインタを返す */

	int nAllLinesOld = m_nLines;
	int	nModifyLayoutLinesOld = 0;
	CLayout* pLayoutPrev;
	int nWork;
	nWork = __max( DLRArg.nDeletedLineNum, DLRArg.nInsLineNum );


	if( pLayoutWork ){
		pLayoutPrev = DeleteLayoutAsLogical(
			pLayoutWork,
			nLineWork,
			nyFrom,
			nyFrom + nWork,
			nyFrom, nxFrom,
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
	int	nRowNum;
	if( NULL == pLayoutPrev ){
		if( NULL == m_pLayoutTop ){
			nRowNum = m_pcDocLineMgr->GetLineCount();
		}else{
			nRowNum = m_pLayoutTop->m_nLinePhysical;
		}
	}
	else{
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

	// 2009.08.28 nasukoji	テキスト最大幅算出用の引数を設定
	CalTextWidthArg ctwArg;
	ctwArg.nLineFrom    = pArg->nDelLineFrom;						// 編集開始桁
	ctwArg.nColmFrom    = pArg->nDelColmFrom;						// 編集開始列
	ctwArg.nDelLines    = pArg->nDelLineTo - pArg->nDelLineFrom;	// 削除行なし
	ctwArg.nAllLinesOld = nWork_nLines;								// 編集前のテキスト行数
	ctwArg.bInsData     = pArg->nInsDataLen ? TRUE : FALSE;			// 追加文字列の有無

	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	int nAddInsLineNum;
	pArg->nModLineTo = DoLayout_Range(
		pLayoutPrev,
		nRowNum,
		nyFrom, nxFrom,
		nCurrentLineType,
		&ctwArg,
		&nAddInsLineNum
	);

	pArg->nAddLineNum = nWork_nLines - m_nLines;	//変更後の全行数との差分	@@@ 2002.04.19 MIK
	if( 0 == pArg->nAddLineNum )
		pArg->nAddLineNum = nModifyLayoutLinesOld - pArg->nModLineTo;	/* 再描画ヒント レイアウト行の増減 */
	pArg->nModLineFrom = pArg->nDelLineFrom;	/* 再描画ヒント 変更されたレイアウト行From */
	pArg->nModLineTo += ( pArg->nModLineFrom - 1 ) ;	/* 再描画ヒント 変更されたレイアウト行To */

	/* レイアウト位置への変換 */
	LogicToLayout( pArg->nNewPos, pArg->nNewLine, &pArg->nNewPos, &pArg->nNewLine );
}


/*[EOF]*/
