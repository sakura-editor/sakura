/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
	@date 1998/03/06 新規作成
	@date 1998/04/14 データの削除を実装
	@date 1999/12/20 データの置換を実装
	@date 2009/08/28 nasukoji	CalTextWidthArg定義追加、DoLayout_Range()の引数変更
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, Moca, genta, D.S.Koba
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CLAYOUTMGR_H_
#define _CLAYOUTMGR_H_

#include <Windows.h>// 2002/2/10 aroka
#include <vector>
#include "doc/CDocListener.h"
#include "_main/global.h"// 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "types/CType.h"
#include "view/colors/EColorIndexType.h"
#include "util/container.h"

class CBregexp;// 2002/2/10 aroka
class CLayout;// 2002/2/10 aroka
class CDocLineMgr;// 2002/2/10 aroka
class CDocLine;// 2002/2/10 aroka
class CMemory;// 2002/2/10 aroka
class CEditDoc;// 2003/07/20 genta
class CColorStrategy;

struct LayoutReplaceArg {
	CLayoutRange	sDelRange;		//!< 削除範囲。レイアウト単位。
	CNativeW*		pcmemDeleted;	//!< 削除されたデータ
	const wchar_t*	pInsData;		//!< 挿入するデータ
	CLogicInt		nInsDataLen;	//!< 挿入するデータの長さ
	CLayoutInt		nAddLineNum;	//!< 再描画ヒント レイアウト行の増減
	CLayoutInt		nModLineFrom;	//!< 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う)
	CLayoutInt		nModLineTo;		//!< 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う)
	CLayoutPoint	ptLayoutNew;	//!< 挿入された部分の次の位置の位置(レイアウト桁位置, レイアウト行)
};

// 編集時のテキスト最大幅算出用		// 2009.08.28 nasukoji
struct CalTextWidthArg {
	CLayoutInt		nLineFrom;		//!< 編集開始行
	CLayoutInt		nColumnFrom;	//!< 編集開始桁
	CLayoutInt		nDelLines;		//!< 削除に関係する行数 - 1（負数の時削除なし）
	CLayoutInt		nAllLinesOld;	//!< 編集前のテキスト行数
	BOOL			bInsData;		//!< 追加文字列あり
};

class CLogicPointEx: public CLogicPoint{
public:
	CLayoutInt ext;
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!	@brief テキストのレイアウト情報管理

	@date 2005.11.21 Moca 色分け情報をメンバーへ移動．不要となった引数をメンバ関数から削除．
*/
//2007.10.15 XYLogicalToLayoutを廃止。LogicToLayoutに統合。
class CLayoutMgr : public CProgressSubject
{
private:
	typedef CLayoutInt (CLayoutMgr::*CalcIndentProc)( CLayout* );

public:
	//生成と破棄
	CLayoutMgr();
	~CLayoutMgr();
	void Create( CEditDoc*, CDocLineMgr* );
	void Init();
	void _Empty();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        コンフィグ                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//! タブ幅の取得
	CLayoutInt GetTabSpace() const { return m_sTypeConfig.m_nTabSpace; }


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          参照系                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//2007.10.09 kobake 関数名変更: Search → SearchLineByLayoutY
	CLayoutInt		GetLineCount() const{ return m_nLines; }	/* 全物理行数を返す */
	const wchar_t*	GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen ) const;	/* 指定された物理行のデータへのポインタとその長さを返す */
	const wchar_t*	GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen, const CLayout** ppcLayoutDes ) const;	/* 指定された物理行のデータへのポインタとその長さを返す */

	//先頭と末尾
	CLayout*		GetTopLayout()		{ return m_pLayoutTop; }
	CLayout*		GetBottomLayout()	{ return m_pLayoutBot; }
	const CLayout*	GetTopLayout() const { return m_pLayoutTop; }
	const CLayout*	GetBottomLayout() const { return m_pLayoutBot; }

	//レイアウトを探す
	const CLayout*	SearchLineByLayoutY( CLayoutInt nLineLayout ) const;	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	CLayout*		SearchLineByLayoutY( CLayoutInt nLineLayout ){ return const_cast<CLayout*>(static_cast<const CLayoutMgr*>(this)->SearchLineByLayoutY(nLineLayout)); }

	//ワードを探す
	bool			WhereCurrentWord( CLayoutInt , CLogicInt , CLayoutRange* pSelect, CNativeW*, CNativeW* );	/* 現在位置の単語の範囲を調べる */

	//判定
	bool			IsEndOfLine( const CLayoutPoint& ptLinePos );	/* 指定位置が行末(改行文字の直前)か調べる */	//@@@ 2002.04.18 MIK

	/*! 次のTAB位置までの幅
		@param pos [in] 現在の位置
		@return 次のTAB位置までの文字数．1～TAB幅
	 */
	CLayoutInt GetActualTabSpace(CLayoutInt pos) const { return m_sTypeConfig.m_nTabSpace - pos % m_sTypeConfig.m_nTabSpace; }

	//	Aug. 14, 2005 genta
	// Sep. 07, 2007 kobake 関数名変更 GetMaxLineSize→GetMaxLineKetas
	CLayoutInt GetMaxLineKetas(void) const { return m_sTypeConfig.m_nMaxLineKetas; }

	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	bool ChangeLayoutParam( CLayoutInt nTabSize, CLayoutInt nMaxLineKetas );

	// Jul. 29, 2006 genta
	void GetEndLayoutPos(CLayoutPoint* ptLayoutEnd);

	CLayoutInt GetMaxTextWidth(void) const { return m_nTextWidth; }		// 2009.08.28 nasukoji	テキスト最大幅を返す


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           検索                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	int PrevOrNextWord( CLayoutInt, CLogicInt, CLayoutPoint* pptLayoutNew, BOOL, BOOL bStopsBothEnds );	/* 現在位置の左右の単語の先頭位置を調べる */
public:
	int PrevWord( CLayoutInt nLineNum, CLogicInt nIdx, CLayoutPoint* pptLayoutNew, BOOL bStopsBothEnds ){ return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, TRUE, bStopsBothEnds); }	/* 現在位置の左右の単語の先頭位置を調べる */
	int NextWord( CLayoutInt nLineNum, CLogicInt nIdx, CLayoutPoint* pptLayoutNew, BOOL bStopsBothEnds ){ return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, FALSE, bStopsBothEnds); }	/* 現在位置の左右の単語の先頭位置を調べる */

	int SearchWord( CLayoutInt nLine, CLogicInt nIdx, const wchar_t* , ESearchDirection eSearchDirection, const SSearchOption& sSearchOption, CLayoutRange* pMatchRange, CBregexp* );	/* 単語検索 */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        単位の変換                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//!ロジック→レイアウト
	void LogicToLayoutEx( const CLogicPointEx& ptLogicEx, CLayoutPoint* pptLayout, CLayoutInt nLineHint = CLayoutInt(0) )
	{
		LogicToLayout( ptLogicEx, pptLayout, nLineHint );
		pptLayout->x += ptLogicEx.ext;
	}
	void LogicToLayout( const CLogicPoint& ptLogic, CLayoutPoint* pptLayout, CLayoutInt nLineHint = CLayoutInt(0) );
	void LogicToLayout( const CLogicRange& rangeLogic, CLayoutRange* prangeLayout )
	{
		LogicToLayout(rangeLogic.GetFrom(), prangeLayout->GetFromPointer());
		LogicToLayout(rangeLogic.GetTo(), prangeLayout->GetToPointer());
	}

	//!レイアウト→ロジック変換
	void LayoutToLogicEx( const CLayoutPoint& ptLayout, CLogicPointEx* pptLogicEx ) const;
	void LayoutToLogic( const CLayoutPoint& ptLayout, CLogicPoint* pptLogic ) const;
	void LayoutToLogic( const CLayoutRange& rangeLayout, CLogicRange* prangeLogic ) const
	{
		LayoutToLogic(rangeLayout.GetFrom(), prangeLogic->GetFromPointer());
		LayoutToLogic(rangeLayout.GetTo(), prangeLogic->GetToPointer());
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         デバッグ                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void DUMP();	/* テスト用にレイアウト情報をダンプ */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         編集とか                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*
	|| 更新系
	*/
	/* レイアウト情報の変更
		@date Jun. 01, 2001 JEPRO char* (行コメントデリミタ3用)を1つ追加
		@date 2002.04.13 MIK 禁則,改行文字をぶら下げる,句読点ぶらさげを追加
		@date 2002/04/27 YAZAKI STypeConfigを渡すように変更。
	*/
	void SetLayoutInfo(
		bool			bDoRayout,
		const STypeConfig&	refType
	);
	
	/* 行内文字削除 */
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void DeleteData_CLayoutMgr(
		CLayoutInt	nLineNum,
		CLogicInt	nDelPos,
		CLogicInt	nDelLen,
		CLayoutInt*	pnModifyLayoutLinesOld,
		CLayoutInt*	pnModifyLayoutLinesNew,
		CLayoutInt*	pnDeleteLayoutLines,
		CNativeW*	cmemDeleted			/* 削除されたデータ */
	);

	/* 文字列挿入 */
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void InsertData_CLayoutMgr(
		CLayoutInt		nLineNum,
		CLogicInt		nInsPos,
		const wchar_t*	pInsData,
		CLogicInt		nInsDataLen,
		CLayoutInt*		pnModifyLayoutLinesOld,
		CLayoutInt*		pnInsLineNum,		/* 挿入によって増えたレイアウト行の数 */
		CLayoutPoint*	pptNewLayout		// 挿入された部分の次の位置のデータ位置
	);

	/* 文字列置換 */
	void ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
	);

	BOOL CalculateTextWidth( BOOL bCalLineLen = TRUE, CLayoutInt nStart = CLayoutInt(-1), CLayoutInt nEnd = CLayoutInt(-1) );	/* テキスト最大幅を算出する */		// 2009.08.28 nasukoji
	void ClearLayoutLineWidth( void );				/* 各行のレイアウト行長の記憶をクリアする */		// 2009.08.28 nasukoji



protected:
	/*
	||  参照系
	*/
	const char* GetFirstLinrStr( int* );	/* 順アクセスモード：先頭行を得る */
	const char* GetNextLinrStr( int* );	/* 順アクセスモード：次の行を得る */


	/*
	|| 更新系
	*/
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void _DoLayout();	/* 現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します */
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	// 2009.08.28 nasukoji	テキスト最大幅算出用引数追加
	CLayoutInt DoLayout_Range( CLayout* , CLogicInt, CLogicPoint, EColorIndexType, const CalTextWidthArg*, CLayoutInt* );	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	void CalculateTextWidth_Range( const CalTextWidthArg* pctwArg );	/* テキストが編集されたら最大幅を算出する */	// 2009.08.28 nasukoji
	CLayout* DeleteLayoutAsLogical( CLayout*, CLayoutInt, CLogicInt , CLogicInt, CLogicPoint, CLayoutInt* );	/* 論理行の指定範囲に該当するレイアウト情報を削除 */
	void ShiftLogicalLineNum( CLayout* , CLogicInt );	/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */

	//部品
	struct SLayoutWork{
		//毎ループ初期化
		int				nKinsokuType;
		CLogicInt		nPos;
		CLogicInt		nBgn;
		CStringRef		cLineStr;
		CLogicInt		nWordBgn;
		CLogicInt		nWordLen;
		CLayoutInt		nPosX;
		CLayoutInt		nIndent;
		CLayout*		pLayoutCalculated;

		//ループ外
		CDocLine*		pcDocLine;
		CLayout*		pLayout;
		CColorStrategy*	pcColorStrategy;
		CColorStrategy*	pcColorStrategy_Prev;
		CLogicInt		nCurLine;

		//ループ外 (DoLayoutのみ)
//		CLogicInt		nLineNum;

		//ループ外 (DoLayout_Rangeのみ)
		bool			bNeedChangeCOMMENTMODE;
		CLayoutInt		nModifyLayoutLinesNew;
		
		//ループ外 (DoLayout_Range引数)
		CLayoutInt*		pnExtInsLineNum;
		CLogicPoint		ptDelLogicalFrom;

		//関数
		CLayout* _CreateLayout(CLayoutMgr* mgr);
	};
	//関数ポインタ
	typedef void (CLayoutMgr::*PF_OnLine)(SLayoutWork*);
	//DoLayout用
	bool _DoKinsokuSkip(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoWordWrap(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoKutoBurasage(SLayoutWork* pWork);
	void _DoGyotoKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoGyomatsuKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine);
	bool _DoTab(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _MakeOneLine(SLayoutWork* pWork, PF_OnLine pfOnLine);
	//DoLayout用コア
	void _OnLine1(SLayoutWork* pWork);
	//DoLayout_Range用コア
	void _OnLine2(SLayoutWork* pWork);


private:
	bool _ExistKinsokuKuto(wchar_t wc) const{ return m_pszKinsokuKuto_1.exist(wc); }
	bool _ExistKinsokuHead(wchar_t wc) const{ return m_pszKinsokuHead_1.exist(wc); }
	bool IsKinsokuHead( wchar_t wc );	/*!< 行頭禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail( wchar_t wc );	/*!< 行末禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKinsokuKuto( wchar_t wc );	/*!< 句読点文字をチェックする */	//@@@ 2002.04.17 MIK
	//	2005-08-20 D.S.Koba 禁則関連処理の関数化
	/*! 句読点ぶら下げの処理位置か
		@date 2005-08-20 D.S.Koba
		@date Sep. 3, 2005 genta 最適化
	*/
	bool IsKinsokuPosKuto(CLayoutInt nRest, CLayoutInt nCharChars ) const {
		return nRest < nCharChars;
	}
	bool IsKinsokuPosHead(CLayoutInt, CLayoutInt, CLayoutInt);	//!< 行頭禁則の処理位置か
	bool IsKinsokuPosTail(CLayoutInt, CLayoutInt, CLayoutInt);	//!< 行末禁則の処理位置か
public:
	int Match_Quote( wchar_t wcQuote, int nPos, const CStringRef& cLineStr ) const;
private:
	//	Oct. 1, 2002 genta インデント幅計算関数群
	CLayoutInt getIndentOffset_Normal( CLayout* pLayoutPrev );
	CLayoutInt getIndentOffset_Tx2x( CLayout* pLayoutPrev );
	CLayoutInt getIndentOffset_LeftSpace( CLayout* pLayoutPrev );

protected:
	/*
	|| 実装ヘルパ系
	*/
	//@@@ 2002.09.23 YAZAKI
	// 2009.08.28 nasukoji	nPosX引数追加
	CLayout* CreateLayout( CDocLine* pCDocLine, CLogicPoint ptLogicPos, CLogicInt nLength, EColorIndexType nTypePrev, CLayoutInt nIndent, CLayoutInt nPosX );
	CLayout* InsertLineNext( CLayout*, CLayout* );
	void AddLineBottom( CLayout* );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	CDocLineMgr*			m_pcDocLineMgr;	/* 行バッファ管理マネージャ */

protected:
	// 2002.10.07 YAZAKI add m_nLineTypeBot
	// 2007.09.07 kobake 変数名変更: m_nMaxLineSize→m_nMaxLineKetas
	// 2007.10.08 kobake 変数名変更: getIndentOffset→m_getIndentOffset

	//参照
	CEditDoc*		m_pcEditDoc;

	//実データ
	CLayout*				m_pLayoutTop;
	CLayout*				m_pLayoutBot;

	//タイプ別設定
	STypeConfig				m_sTypeConfig;
	vector_ex<wchar_t>		m_pszKinsokuHead_1;			// 行頭禁則文字	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuTail_1;			// 行末禁則文字	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuKuto_1;			// 句読点ぶらさげ文字	//@@@ 2002.04.17 MIK
	CalcIndentProc			m_getIndentOffset;			//	Oct. 1, 2002 genta インデント幅計算関数を保持

	//フラグ等
	EColorIndexType			m_nLineTypeBot;				//!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	CLayoutInt				m_nLines;					// 全レイアウト行数

	mutable CLayoutInt		m_nPrevReferLine;
	mutable CLayout*		m_pLayoutPrevRefer;
	
	// EOFカーソル位置を記憶する(_DoLayout/DoLayout_Rangeで無効にする)	//2006.10.01 Moca
	CLayoutInt				m_nEOFLine; //!< EOF行数
	CLayoutInt				m_nEOFColumn; //!< EOF幅位置

	// テキスト最大幅を記憶（折り返し位置算出に使用）	// 2009.08.28 nasukoji
	CLayoutInt				m_nTextWidth;				// テキスト最大幅の記憶
	CLayoutInt				m_nTextWidthMaxLine;		// 最大幅のレイアウト行
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUTMGR_H_ */



