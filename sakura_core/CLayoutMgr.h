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
	Copyright (C) 2009, ryoji, nasukoji

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

class CBregexp;// 2002/2/10 aroka
class CLayout;// 2002/2/10 aroka
class CDocLineMgr;// 2002/2/10 aroka
class CDocLine;// 2002/2/10 aroka
class CMemory;// 2002/2/10 aroka
class CEditDoc;// 2003/07/20 genta
struct Types;// 2005.11.20 Moca



struct LayoutReplaceArg {
	int			nDelLineFrom;			/*!< 削除範囲行  From レイアウト行番号 */
	int			nDelColmFrom;			/*!< 削除範囲位置From レイアウト行桁位置 */
	int			nDelLineTo;				/*!< 削除範囲行  To   レイアウト行番号 */
	int			nDelColmTo;				/*!< 削除範囲位置To   レイアウト行桁位置 */
	CMemory*	pcmemDeleted;			/*!< 削除されたデータ */
	const char*	pInsData;				/*!< 挿入するデータ */
	int			nInsDataLen;			/*!< 挿入するデータの長さ */

	int			nAddLineNum;			/*!< 再描画ヒント レイアウト行の増減 */
	int			nModLineFrom;			/*!< 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
	int			nModLineTo;			/*!< 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */

	int			nNewLine;				/*!< 挿入された部分の次の位置の行(レイアウト行) */
	int			nNewPos;				/*!< 挿入された部分の次の位置のデータ位置(レイアウト桁位置) */

//	BOOL		bUndo;					/*!< Undo操作かどうか */	@date 2002/03/24 YAZAKI bUndo削除
};

// 編集時のテキスト最大幅算出用		// 2009.08.28 nasukoji
struct CalTextWidthArg {
	int  nLineFrom;				/*!< 編集開始行 */
	int  nColmFrom;				/*!< 編集開始桁 */
	int  nDelLines;				/*!< 削除に関係する行数 - 1（負数の時削除なし） */
	int  nAllLinesOld;			/*!< 編集前のテキスト行数 */
	BOOL bInsData;				/*!< 追加文字列あり */
};


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!	@brief テキストのレイアウト情報管理

	@date 2005.11.21 Moca 色分け情報をメンバーへ移動．不要となった引数をメンバ関数から削除．
*/
class SAKURA_CORE_API CLayoutMgr
{
public:
	/*
	||  Constructors
	*/
	CLayoutMgr();
	~CLayoutMgr();
	void Create( CEditDoc*, CDocLineMgr* );
	/*
	||  参照系
	*/
	int GetLineCount( void ) { return m_nLines; }	/* 全物理行数を返す */
	const char* GetLineStr( int , int* );	/* 指定された物理行のデータへのポインタとその長さを返す */
	const char* GetLineStr( int , int*, const CLayout** );	/* 指定された物理行のデータへのポインタとその長さを返す */
	bool IsEndOfLine( int nLine, int nPos );	/* 指定位置が行末(改行文字の直前)か調べる */	//@@@ 2002.04.18 MIK
//	const CLayout* GetLineData( int );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	CLayout* Search( int );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	int WhereCurrentWord( int , int , int* , int* , int* , int*, CMemory*, CMemory* );	/* 現在位置の単語の範囲を調べる */
	//	Sep. 23, 2002 genta
	/*! タブ幅の取得
		@return タブ幅
	 */
	int GetTabSpace(void) const { return m_nTabSpace; }
	/*! 次のTAB位置までの幅
		@param pos [in] 現在の位置
		@return 次のTAB位置までの文字数．1～TAB幅
	 */
	int GetActualTabSpace(int pos) const { return m_nTabSpace - pos % m_nTabSpace; }
	//	Aug. 14, 2005 genta
	int GetMaxLineSize(void) const { return m_nMaxLineSize; }
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	bool ChangeLayoutParam( HWND hwndProgress, int nTabSize, int nMaxLineSize );
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

	int SearchWord( int	, int , const char* , int , int , int , int , int* , int* , int* , int*, CBregexp* );	/* 単語検索 */
	void CaretPos_Phys2Log( int, int, int*, int* );
	void CaretPos_Log2Phys( int, int, int*, int* );
	void DUMP( void );	/* テスト用にレイアウト情報をダンプ */

	/*
	|| 更新系
	*/
	/* レイアウト情報の変更
		@date Jun. 01, 2001 JEPRO char* (行コメントデリミタ3用)を1つ追加
		@date 2002.04.13 MIK 禁則,改行文字をぶら下げる,句読点ぶらさげを追加
		@date 2002/04/27 YAZAKI Typesを渡すように変更。
	*/
	void SetLayoutInfo( int, HWND, Types& refType );
	
	/* 行内文字削除 */
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void DeleteData_CLayoutMgr(
		int			nLineNum,
		int			nDelPos,
		int			nDelLen,
		int			*pnModifyLayoutLinesOld,
		int			*pnModifyLayoutLinesNew,
		int			*pnDeleteLayoutLines,
		CMemory&	cmemDeleted			/* 削除されたデータ */
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
		int*		pnNewLine,			/* 挿入された部分の次の位置の行 */
		int*		pnNewPos			/* 挿入された部分の次の位置のデータ位置 */
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
//	void SetMaxLineSize( int, int, int );	/* 折り返し文字数の変更／レイアウト情報再構築 */
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	void DoLayout( HWND );	/* 現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します */
//	void DoLayout( int, BOOL, HWND, BOOL, BOOL );	/* 新しい折り返し文字数に合わせて全データのレイアウト情報を再生成します */
//	int DoLayout3( CLayout* , int, int, int );	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	// 2009.08.28 nasukoji	テキスト最大幅算出用引数追加
	int DoLayout_Range( CLayout* , int, int, int, int, const CalTextWidthArg*, int* );	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	void CalculateTextWidth_Range( const CalTextWidthArg* pctwArg );	/* テキストが編集されたら最大幅を算出する */	// 2009.08.28 nasukoji
	CLayout* DeleteLayoutAsLogical( CLayout*, int, int , int, int, int, int* );	/* 論理行の指定範囲に該当するレイアウト情報を削除 */
	void ShiftLogicalLineNum( CLayout* , int );	/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */

	/*
	|| メンバ変数
	*/
public:
	CDocLineMgr*	m_pcDocLineMgr;	/* 行バッファ管理マネージャ */
protected:
	CLayout*		m_pLayoutTop;
	CLayout*		m_pLayoutBot;
	//	2002.10.07 YAZAKI add
	int				m_nLineTypeBot;	/*!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	int				m_nLines;					/* 全物理行数 */
	int				m_nMaxLineSize;				/* 折り返し文字数 */
	BOOL			m_bWordWrap;				/* 英文ワードラップをする */
	BOOL			m_bKinsokuHead;				/* 行頭禁則をする */	//@@@ 2002.04.08 MIK
	BOOL			m_bKinsokuTail;				/* 行末禁則をする */	//@@@ 2002.04.08 MIK
	BOOL			m_bKinsokuRet;				/* 改行文字をぶら下げる */	//@@@ 2002.04.13 MIK
	BOOL			m_bKinsokuKuto;				/* 句読点をぶら下げる */	//@@@ 2002.04.17 MIK
	char*			m_pszKinsokuHead_1;			/* 行頭禁則文字 */	//@@@ 2002.04.08 MIK
	char*			m_pszKinsokuHead_2;			/* 行頭禁則文字 */	//@@@ 2002.04.08 MIK
	char*			m_pszKinsokuTail_1;			/* 行末禁則文字 */	//@@@ 2002.04.08 MIK
	char*			m_pszKinsokuTail_2;			/* 行末禁則文字 */	//@@@ 2002.04.08 MIK
	char*			m_pszKinsokuKuto_1;			/* 句読点ぶらさげ文字 */	//@@@ 2002.04.17 MIK
	char*			m_pszKinsokuKuto_2;			/* 句読点ぶらさげ文字 */	//@@@ 2002.04.17 MIK
	int				m_nTabSpace;				/* TAB文字スペース */
	BOOL			m_bDispComment;				/* コメントの色分け */		// 2005.11.21 Moca
	BOOL			m_bDispSString;				/* シングルクォーテーションの色分け */		// 2005.11.21 Moca
	BOOL			m_bDispWString;				/* ダブルクォーテーションの色分け */		// 2005.11.21 Moca
	CLineComment	m_cLineComment;				/* 行コメントデリミタ */		//@@@ 2002.09.22 YAZAKI
	CBlockComment	m_cBlockComment;			/* ブロックコメントデリミタ */	//@@@ 2002.09.22 YAZAKI
	int				m_nStringType;				/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
//	int				m_nIndentLayout;			/* 折り返しは2行目以降を字下げ表示 */	//@@@ 2002.09.29 YAZAKI
	//	Oct. 1, 2002 genta インデント幅計算関数を保持
	int (CLayoutMgr::*getIndentOffset)( CLayout* );


	int				m_nPrevReferLine;
	CLayout*		m_pLayoutPrevRefer;
	
	// 2006.10.01 Moca EOFカーソル位置を記憶する(DoLayout/DoLayout_Rangeで無効にする)
	int				m_nEOFLine; //!< EOF行数
	int				m_nEOFColumn; //!< EOF幅位置

	//	Jul. 20, 2003 genta
	//	タイプ別の設定を取得するためにCEditDocへの参照が必要
	CEditDoc*		m_pcEditDoc;

	int m_nTextWidth;				// テキスト最大幅の記憶			// 2009.08.28 nasukoji
	int m_nTextWidthMaxLine;		// 最大幅のレイアウト行			// 2009.08.28 nasukoji
	
	/*
	|| 実装ヘルパ系
	*/
	//@@@ 2002.09.23 YAZAKI
	// 2009.08.15 nasukoji	nPosX引数追加
	CLayout* CreateLayout( CDocLine* pCDocLine, int nLine, int nOffset, int nLength, int nTypePrev, int nIndent, int nPosX );
	CLayout* InsertLineNext( CLayout*, CLayout* );
	void AddLineBottom( CLayout* );
public:
	void Init();
	void Empty();

private:
	bool IsKinsokuHead( const char *pLine, int length );	/*!< 行頭禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail( const char *pLine, int length );	/*!< 行末禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	//bool IsKutoTen( unsigned char c1, unsigned char c2 );	/*!< 句読点文字をチェックする */	//@@@ 2002.04.17 MIK
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
	bool CheckColorMODE( int& nCOMMENTMODE, int& nCOMMENTEND, int nPos, int nLineLen, const char* pLine );
	int Match_Quote( char szQuote, int nPos, int nLineLen, const char* pLine );
//	int getMaxLineSize( CLayout* pLayoutPrev );
	
	//	Oct. 1, 2002 genta インデント幅計算関数群
	int getIndentOffset_Normal( CLayout* pLayoutPrev );
	int getIndentOffset_Tx2x( CLayout* pLayoutPrev );
	int getIndentOffset_LeftSpace( CLayout* pLayoutPrev );
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUTMGR_H_ */


/*[EOF]*/
