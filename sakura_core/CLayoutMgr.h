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
	Copyright (C) 2006, genta, Moca
	Copyright (C) 2007, kobake
	Copyright (C) 2009, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CLAYOUTMGR_H_
#define _CLAYOUTMGR_H_

#include <windows.h>// 2002/2/10 aroka
#include "global.h"// 2002/2/10 aroka
#include "CShareData.h"
#include "CLineComment.h"	//@@@ 2002.09.22 YAZAKI
#include "CBlockComment.h"	//@@@ 2002.09.22 YAZAKI

class CBregexp;// 2002/2/10 aroka
class CLayout;// 2002/2/10 aroka
class CDocLineMgr;// 2002/2/10 aroka
class CDocLine;// 2002/2/10 aroka
class CMemory;// 2002/2/10 aroka
class CEditDoc;// 2003/07/20 genta
struct STypeConfig;// 2005.11.20 Moca


struct LayoutReplaceArg {
	int			nDelLineFrom;			//!< 削除範囲行  From レイアウト行番号
	int			nDelColmFrom;			//!< 削除範囲位置From レイアウト行桁位置
	int			nDelLineTo;				//!< 削除範囲行  To   レイアウト行番号
	int			nDelColmTo;				//!< 削除範囲位置To   レイアウト行桁位置
	CMemory*	pcmemDeleted;			//!< 削除されたデータ
	const char*	pInsData;				//!< 挿入するデータ
	int			nInsDataLen;			//!< 挿入するデータの長さ

	int			nAddLineNum;			//!< 再描画ヒント レイアウト行の増減
	int			nModLineFrom;			//!< 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う)
	int			nModLineTo;				//!< 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う)

	int			nNewLine;				//!< 挿入された部分の次の位置の行(レイアウト行)
	int			nNewPos;				//!< 挿入された部分の次の位置のデータ位置(レイアウト桁位置)
};

// 編集時のテキスト最大幅算出用		// 2009.08.28 nasukoji
struct CalTextWidthArg {
	int  nLineFrom;				//!< 編集開始行
	int  nColmFrom;				//!< 編集開始桁
	int  nDelLines;				//!< 削除に関係する行数 - 1（負数の時削除なし）
	int  nAllLinesOld;			//!< 編集前のテキスト行数
	BOOL bInsData;				//!< 追加文字列あり
};


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!	@brief テキストのレイアウト情報管理

	@date 2005.11.21 Moca 色分け情報をメンバーへ移動．不要となった引数をメンバ関数から削除．
*/
class CLayoutMgr
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
	void Init();
	void _Empty();

	/*
	||  参照系
	*/
	int GetLineCount( void ) { return m_nLines; }	/* 全物理行数を返す */
	const char* GetLineStr( int , int* );	/* 指定された物理行のデータへのポインタとその長さを返す */
	const char* GetLineStr( int , int*, const CLayout** );	/* 指定された物理行のデータへのポインタとその長さを返す */
	bool IsEndOfLine( int nLine, int nPos );	/* 指定位置が行末(改行文字の直前)か調べる */	//@@@ 2002.04.18 MIK
	CLayout* SearchLineByLayoutY( int );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	int WhereCurrentWord( int , int , int* , int* , int* , int*, CMemory*, CMemory* );	/* 現在位置の単語の範囲を調べる */
	//	Sep. 23, 2002 genta
	//! タブ幅の取得
	int GetTabSpace() const { return m_sTypeConfig.m_nTabSpace; }
	/*! 次のTAB位置までの幅
		@param pos [in] 現在の位置
		@return 次のTAB位置までの文字数．1～TAB幅
	 */
	int GetActualTabSpace(int pos) const { return m_sTypeConfig.m_nTabSpace - pos % m_sTypeConfig.m_nTabSpace; }

	//	Aug. 14, 2005 genta
	// Sep. 07, 2007 kobake 関数名変更 GetMaxLineSize→GetMaxLineKetas
	int GetMaxLineKetas(void) const { return m_sTypeConfig.m_nMaxLineKetas; }

	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	bool ChangeLayoutParam( HWND hwndProgress, int nTabSize, int nMaxLineKetas );

	// Jul. 29, 2006 genta
	void GetEndLayoutPos(int& lX, int& lY);
	CLayout* GetTopLayout()		{ return m_pLayoutTop; }	// 2009.02.17 ryoji
	CLayout* GetBottomLayout()	{ return m_pLayoutBot; }	// 2009.02.17 ryoji
	
	int GetMaxTextWidth(void) const { return m_nTextWidth; }	// 2009.08.28 nasukoji	テキスト最大幅を返す

protected:
	int PrevOrNextWord( int, int, int*, int*, BOOL, BOOL bStopsBothEnds );	/* 現在位置の左右の単語の先頭位置を調べる */
public:
	int PrevWord( int nLineNum, int nIdx, int* pnLineNew, int* pnColmNew, BOOL bStopsBothEnds ){ return PrevOrNextWord(nLineNum, nIdx, pnLineNew, pnColmNew, TRUE, bStopsBothEnds); }	/* 現在位置の左右の単語の先頭位置を調べる */
	int NextWord( int nLineNum, int nIdx, int* pnLineNew, int* pnColmNew, BOOL bStopsBothEnds ){ return PrevOrNextWord(nLineNum, nIdx, pnLineNew, pnColmNew, FALSE, bStopsBothEnds); }	/* 現在位置の左右の単語の先頭位置を調べる */

	int SearchWord( int	, int , const char* , ESearchDirection , const SSearchOption& , int* , int* , int* , int*, CBregexp* );	/* 単語検索 */
	void LogicToLayout( int, int, int*, int* );
	void LayoutToLogic( int, int, int*, int* );
public:
	void DUMP();	/* テスト用にレイアウト情報をダンプ */

	/*
	|| 更新系
	*/
	/* レイアウト情報の変更
		@date Jun. 01, 2001 JEPRO char* (行コメントデリミタ3用)を1つ追加
		@date 2002.04.13 MIK 禁則,改行文字をぶら下げる,句読点ぶらさげを追加
		@date 2002/04/27 YAZAKI STypeConfigを渡すように変更。
	*/
	void SetLayoutInfo(
		bool	bDoRayout,
		HWND	hwndProgress,
		const STypeConfig& refType
	);
	
	/* 行内文字削除 */
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void DeleteData_CLayoutMgr(
		int			nLineNum,
		int			nDelPos,
		int			nDelLen,
		int*		pnModifyLayoutLinesOld,
		int*		pnModifyLayoutLinesNew,
		int*		pnDeleteLayoutLines,
		CMemory*	cmemDeleted			/* 削除されたデータ */
	);

	/* 文字列挿入 */
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void InsertData_CLayoutMgr(
		int			nLineNum,
		int			nInsPos,
		const char*	pInsData,
		int			nInsDataLen,
		int*		pnModifyLayoutLinesOld,
		int*		pnInsLineNum,		/* 挿入によって増えたレイアウト行の数 */
		int*		pnNewLine,			// 挿入された部分の次の位置の行
		int*		pnNewPos			// 挿入された部分の次の位置のデータ位置
	);

	/* 文字列置換 */
	void ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
	);

	BOOL CalculateTextWidth( BOOL bCalLineLen = TRUE, int nStart = -1, int nEnd = -1 );	/* テキスト最大幅を算出する */		// 2009.08.28 nasukoji
	void ClearLayoutLineWidth( void );				/* 各行のレイアウト行長の記憶をクリアする */		// 2009.08.28 nasukoji



protected:
	/*
	||  参照系
	*/
	const char* GetFirstLinrStr( int* );	/* 順アクセスモード：先頭行を得る */
	const char* GetNextLinrStr( int* );	/* 順アクセスモード：次の行を得る */
	void XYLogicalToLayout( CLayout*, int, int, int, int*, int* );	/* 論理位置→レイアウト位置変換 */


	/*
	|| 更新系
	*/
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void _DoLayout( HWND );	/* 現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します */
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	// 2009.08.28 nasukoji	テキスト最大幅算出用引数追加
	int DoLayout_Range( CLayout* , int, int, int, EColorIndexType, const CalTextWidthArg*, int* );	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	void CalculateTextWidth_Range( const CalTextWidthArg* pctwArg );	/* テキストが編集されたら最大幅を算出する */	// 2009.08.28 nasukoji
	CLayout* DeleteLayoutAsLogical( CLayout*, int, int , int, int, int, int* );	/* 論理行の指定範囲に該当するレイアウト情報を削除 */
	void ShiftLogicalLineNum( CLayout* , int );	/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */

private:
	bool IsKinsokuHead( const char *pLine, int length );	/*!< 行頭禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail( const char *pLine, int length );	/*!< 行末禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKinsokuKuto( const char *pLine, int length );	/*!< 句読点文字をチェックする */	//@@@ 2002.04.17 MIK
	//	2005-08-20 D.S.Koba 禁則関連処理の関数化
	/*! 句読点ぶら下げの処理位置か
		@date 2005-08-20 D.S.Koba
		@date Sep. 3, 2005 genta 最適化
	*/
	bool IsKinsokuPosKuto(const int nRest, const int nCharChars ) const {
		return nRest < nCharChars;
	}
	bool IsKinsokuPosHead(const int, const int, const int);	//!< 行頭禁則の処理位置か
	bool IsKinsokuPosTail(const int, const int, const int);	//!< 行末禁則の処理位置か

	//@@@ 2002.09.22 YAZAKI
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	bool CheckColorMODE( EColorIndexType& nCOMMENTMODE, int& nCOMMENTEND, int nPos, int nLineLen, const char* pLine );
	int Match_Quote( char szQuote, int nPos, int nLineLen, const char* pLine );

	//	Oct. 1, 2002 genta インデント幅計算関数群
	int getIndentOffset_Normal( CLayout* pLayoutPrev );
	int getIndentOffset_Tx2x( CLayout* pLayoutPrev );
	int getIndentOffset_LeftSpace( CLayout* pLayoutPrev );

	/*
	|| 実装ヘルパ系
	*/
	//@@@ 2002.09.23 YAZAKI
	// 2009.08.15 nasukoji	nPosX引数追加
	CLayout* CreateLayout( CDocLine* pCDocLine, int nLine, int nOffset, int nLength, EColorIndexType nTypePrev, int nIndent, int nPosX );
	CLayout* InsertLineNext( CLayout*, CLayout* );
	void AddLineBottom( CLayout* );

	/*
	|| メンバ変数
	*/
public:
	CDocLineMgr*	m_pcDocLineMgr;	/* 行バッファ管理マネージャ */

protected:
	// 2002.10.07 YAZAKI add m_nLineTypeBot
	// 2007.09.07 kobake 変数名変更: m_nMaxLineSize→m_nMaxLineKetas
	// 2007.10.08 kobake 変数名変更: getIndentOffset→m_getIndentOffset

	CEditDoc*		m_pcEditDoc;

	CLayout*		m_pLayoutTop;
	CLayout*		m_pLayoutBot;

	STypeConfig		m_sTypeConfig;
	char*			m_pszKinsokuHead_1;			// 行頭禁則文字	//@@@ 2002.04.08 MIK
	char*			m_pszKinsokuHead_2;			// 行頭禁則文字	//@@@ 2002.04.08 MIK
	char*			m_pszKinsokuTail_1;			// 行末禁則文字	//@@@ 2002.04.08 MIK
	char*			m_pszKinsokuTail_2;			// 行末禁則文字	//@@@ 2002.04.08 MIK
	char*			m_pszKinsokuKuto_1;			// 句読点ぶらさげ文字	//@@@ 2002.04.17 MIK
	char*			m_pszKinsokuKuto_2;			// 句読点ぶらさげ文字	//@@@ 2002.04.17 MIK
	CalcIndentProc	m_getIndentOffset;			//	Oct. 1, 2002 genta インデント幅計算関数を保持

	EColorIndexType	m_nLineTypeBot;				//!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	int				m_nLines;					// 全物理行数

	int				m_nPrevReferLine;
	CLayout*		m_pLayoutPrevRefer;
	
	// EOFカーソル位置を記憶する(_DoLayout/DoLayout_Rangeで無効にする)	//2006.10.01 Moca 
	int				m_nEOFLine; //!< EOF行数
	int				m_nEOFColumn; //!< EOF幅位置

	// テキスト最大幅を記憶（折り返し位置算出に使用）	// 2009.08.28 nasukoji
	int m_nTextWidth;				// テキスト最大幅の記憶
	int m_nTextWidthMaxLine;		// 最大幅のレイアウト行
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUTMGR_H_ */


/*[EOF]*/
