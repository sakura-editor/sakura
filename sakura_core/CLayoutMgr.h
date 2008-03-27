/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
	@date 1998/03/06 新規作成
	@date 1998/04/14 データの削除を実装
	@date 1999/12/20 データの置換を実装
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, Moca, genta, D.S.Koba

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CLayoutMgr;

#ifndef _CLAYOUTMGR_H_
#define _CLAYOUTMGR_H_

#include <windows.h>// 2002/2/10 aroka
#include "global.h"// 2002/2/10 aroka
#include "CLineComment.h"	//@@@ 2002.09.22 YAZAKI
#include "CBlockComment.h"	//@@@ 2002.09.22 YAZAKI
#include "basis/SakuraBasis.h"
#include <vector>
#include "util/container.h"

class CBregexp;// 2002/2/10 aroka
class CLayout;// 2002/2/10 aroka
class CDocLineMgr;// 2002/2/10 aroka
class CDocLine;// 2002/2/10 aroka
class CMemory;// 2002/2/10 aroka
class CEditDoc;// 2003/07/20 genta
struct Types;// 2005.11.20 Moca
#include "basis/SakuraBasis.h"


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


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!	@brief テキストのレイアウト情報管理

	@date 2005.11.21 Moca 色分け情報をメンバーへ移動．不要となった引数をメンバ関数から削除．
*/
//2007.10.15 XYLogicalToLayoutを廃止。LogicToLayoutに統合。
class SAKURA_CORE_API CLayoutMgr : public CProgressSubject
{
private:
	typedef CLayoutInt (CLayoutMgr::*CalcIndentProc)( CLayout* );

public:
	/*
	||  Constructors
	*/
	CLayoutMgr();
	~CLayoutMgr();
	void Create( CEditDoc*, CDocLineMgr* );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          参照系                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//2007.10.09 kobake 関数名変更: Search → SearchLineByLayoutY
	CLayoutInt GetLineCount() const{ return m_nLines; }	/* 全物理行数を返す */
	const wchar_t* GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen ) const;	/* 指定された物理行のデータへのポインタとその長さを返す */
	const wchar_t* GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen, const CLayout** ppcLayoutDes ) const;	/* 指定された物理行のデータへのポインタとその長さを返す */
	bool IsEndOfLine( const CLayoutPoint& ptLinePos );	/* 指定位置が行末(改行文字の直前)か調べる */	//@@@ 2002.04.18 MIK
	const CLayout* SearchLineByLayoutY( CLayoutInt nLineLayout ) const;	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	CLayout* SearchLineByLayoutY( CLayoutInt nLineLayout ){ return const_cast<CLayout*>(static_cast<const CLayoutMgr*>(this)->SearchLineByLayoutY(nLineLayout)); }
	bool WhereCurrentWord( CLayoutInt , CLogicInt , CLayoutRange* pSelect, CNativeW*, CNativeW* );	/* 現在位置の単語の範囲を調べる */

	//	Sep. 23, 2002 genta
	/*! タブ幅の取得
		@return タブ幅
	 */
	CLayoutInt GetTabSpace(void) const { return m_nTabSpace; }

	/*! 次のTAB位置までの幅
		@param pos [in] 現在の位置
		@return 次のTAB位置までの文字数．1～TAB幅
	 */
	CLayoutInt GetActualTabSpace(CLayoutInt pos) const { return m_nTabSpace - pos % m_nTabSpace; }

	//	Aug. 14, 2005 genta
	// Sep. 07, 2007 kobake 関数名変更 GetMaxLineSize→GetMaxLineKetas
	CLayoutInt GetMaxLineKetas(void) const { return m_nMaxLineKetas; }

	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	bool ChangeLayoutParam( CLayoutInt nTabSize, CLayoutInt nMaxLineKetas );

	// Jul. 29, 2006 genta
	void GetEndLayoutPos(CLayoutPoint* ptLayoutEnd);


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
	void LogicToLayout( const CLogicPoint& ptLogic, CLayoutPoint* pptLayout, CLayoutInt nLineHint = CLayoutInt(0) );
	void LogicToLayout( const CLogicRange& rangeLogic, CLayoutRange* prangeLayout )
	{
		LogicToLayout(rangeLogic.GetFrom(), prangeLayout->GetFromPointer());
		LogicToLayout(rangeLogic.GetTo(), prangeLayout->GetToPointer());
	}

	//!レイアウト→ロジック変換
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
	void DUMP( void );	/* テスト用にレイアウト情報をダンプ */


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
		@date 2002/04/27 YAZAKI Typesを渡すように変更。
	*/
	void SetLayoutInfo(
		bool			bDoRayout,
		const Types&	refType
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
	CLayoutInt DoLayout_Range( CLayout* , CLogicInt, CLogicPoint, EColorIndexType, CLayoutInt* );	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	CLayout* DeleteLayoutAsLogical( CLayout*, CLayoutInt, CLogicInt , CLogicInt, CLogicPoint, CLayoutInt* );	/* 論理行の指定範囲に該当するレイアウト情報を削除 */
	void ShiftLogicalLineNum( CLayout* , CLogicInt );	/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */


public:
	void Init();
	void Empty();

private:
	bool _ExistKinsokuKuto(wchar_t wc) const{ return m_pszKinsokuKuto_1.exist(wc); }
	bool _ExistKinsokuHead(wchar_t wc) const{ return m_pszKinsokuHead_1.exist(wc); }
	bool IsKinsokuHead( const wchar_t *pLine, CLogicInt length );	/*!< 行頭禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail( const wchar_t *pLine, CLogicInt length );	/*!< 行末禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKutoTen( wchar_t wc );	/*!< 句読点文字をチェックする */	//@@@ 2002.04.17 MIK
	bool IsKinsokuKuto( const wchar_t *pLine, CLogicInt length );	/*!< 句読点文字をチェックする */	//@@@ 2002.04.17 MIK
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

	//@@@ 2002.09.22 YAZAKI
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	bool CheckColorMODE( EColorIndexType& nCOMMENTMODE, int& nCOMMENTEND, int nPos, int nLineLen, const wchar_t* pLine );
	int Match_Quote( wchar_t wcQuote, int nPos, int nLineLen, const wchar_t* pLine );

	//	Oct. 1, 2002 genta インデント幅計算関数群
	CLayoutInt getIndentOffset_Normal( CLayout* pLayoutPrev );
	CLayoutInt getIndentOffset_Tx2x( CLayout* pLayoutPrev );
	CLayoutInt getIndentOffset_LeftSpace( CLayout* pLayoutPrev );

protected:
	/*
	|| 実装ヘルパ系
	*/
	//@@@ 2002.09.23 YAZAKI
	CLayout* CreateLayout( CDocLine* pCDocLine, CLogicPoint ptLogicPos, CLogicInt nLength, EColorIndexType nTypePrev, CLayoutInt nIndent );
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

	//実データ
	CLayout*				m_pLayoutTop;
	CLayout*				m_pLayoutBot;

	//フラグ等
	EColorIndexType			m_nLineTypeBot;				//!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	CLayoutInt				m_nLines;					// 全レイアウト行数
	CLayoutInt				m_nMaxLineKetas;			// 折り返し桁数
	BOOL					m_bWordWrap;				// 英文ワードラップをする
	BOOL					m_bKinsokuHead;				// 行頭禁則をする	//@@@ 2002.04.08 MIK
	BOOL					m_bKinsokuTail;				// 行末禁則をする	//@@@ 2002.04.08 MIK
	BOOL					m_bKinsokuRet;				// 改行文字をぶら下げる	//@@@ 2002.04.13 MIK
	BOOL					m_bKinsokuKuto;				// 句読点をぶら下げる	//@@@ 2002.04.17 MIK
	vector_ex<wchar_t>		m_pszKinsokuHead_1;			// 行頭禁則文字	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuTail_1;			// 行末禁則文字	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuKuto_1;			// 句読点ぶらさげ文字	//@@@ 2002.04.17 MIK
	CLayoutInt				m_nTabSpace;				// TAB文字スペース
	BOOL					m_bDispComment;				// コメントの色分け		// 2005.11.21 Moca
	BOOL					m_bDispSString;				// シングルクォーテーションの色分け		// 2005.11.21 Moca
	BOOL					m_bDispWString;				// ダブルクォーテーションの色分け		// 2005.11.21 Moca
	CLineComment			m_cLineComment;				// 行コメントデリミタ		//@@@ 2002.09.22 YAZAKI
	CBlockComment			m_cBlockComment;			// ブロックコメントデリミタ	//@@@ 2002.09.22 YAZAKI
	int						m_nStringType;				// 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""]['']
	CalcIndentProc			m_getIndentOffset;			//	Oct. 1, 2002 genta インデント幅計算関数を保持

	mutable CLayoutInt		m_nPrevReferLine;
	mutable CLayout*		m_pLayoutPrevRefer;
	
	// 2006.10.01 Moca EOFカーソル位置を記憶する(_DoLayout/DoLayout_Rangeで無効にする)
	CLayoutInt				m_nEOFLine; //!< EOF行数
	CLayoutInt				m_nEOFColumn; //!< EOF幅位置

	//	Jul. 20, 2003 genta
	//	タイプ別の設定を取得するためにCEditDocへの参照が必要
	CEditDoc*		m_pcEditDoc;
	
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUTMGR_H_ */


/*[EOF]*/
