//	$Id$
/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
	@date 1998/03/06 新規作成
	@date 1998/04/14 データの削除を実装
	@date 1999/12/20 データの置換を実装
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CLayoutMgr;

#ifndef _CLAYOUTMGR_H_
#define _CLAYOUTMGR_H_

#include <windows.h>// 2002/2/10 aroka
#include "global.h"// 2002/2/10 aroka
//#include "CLayout.h"// 2002/2/10 aroka
//#include "CDocLineMgr.h"// 2002/2/10 aroka
//#include "CMemory.h"// 2002/2/10 aroka
// //	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
//#include "CBregexp.h"// 2002/2/10 aroka
#include "CShareData.h"
class CBregexp;// 2002/2/10 aroka
class CLayout;// 2002/2/10 aroka
class CDocLineMgr;// 2002/2/10 aroka
class CDocLine;// 2002/2/10 aroka
class CMemory;// 2002/2/10 aroka




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

	BOOL		bDispSSTRING;			/*!< シングルクォーテーション文字列を表示する */
	BOOL		bDispWSTRING;			/*!< ダブルクォーテーション文字列を表示する */
//	BOOL		bUndo;					/*!< Undo操作かどうか */	@date 2002/03/24 YAZAKI bUndo削除
};


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//! テキストのレイアウト情報管理
class SAKURA_CORE_API CLayoutMgr
{
public:
	/*
	||  Constructors
	*/
	CLayoutMgr();
	~CLayoutMgr();
	void Create( CDocLineMgr* );
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
	void DeleteData_CLayoutMgr(
		int			nLineNum,
		int			nDelPos,
		int			nDelLen,
		int			*pnModifyLayoutLinesOld,
		int			*pnModifyLayoutLinesNew,
		int			*pnDeleteLayoutLines,
		CMemory&	cmemDeleted,			/* 削除されたデータ */
		BOOL		bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
		BOOL		bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
	);
	/* 文字列挿入 */
	void InsertData_CLayoutMgr(
		int			nLineNum,
		int			nInsPos,
		const char*	pInsData,
		int			nInsDataLen,
		int*		pnModifyLayoutLinesOld,
		int*		pnInsLineNum,		/* 挿入によって増えたレイアウト行の数 */
		int*		pnNewLine,			/* 挿入された部分の次の位置の行 */
		int*		pnNewPos,			/* 挿入された部分の次の位置のデータ位置 */
		BOOL		bDispSSTRING,	/* シングルクォーテーション文字列を表示する */
		BOOL		bDispWSTRING	/* ダブルクォーテーション文字列を表示する */
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
	void XYLogicalToLayout( CLayout*, int, int, int, int*, int* );	/* 論理位置→レイアウト位置変換 */


	/*
	|| 更新系
	*/
//	void SetMaxLineSize( int, int, int );	/* 折り返し文字数の変更／レイアウト情報再構築 */
	void DoLayout( HWND, BOOL,BOOL );	/* 現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します */
//	void DoLayout( int, BOOL, HWND, BOOL, BOOL );	/* 新しい折り返し文字数に合わせて全データのレイアウト情報を再生成します */
//	int DoLayout3( CLayout* , int, int, int );	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	int DoLayout3_New( CLayout* , /*CLayout* ,*/ int, int, int, int, int*, BOOL, BOOL );	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
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
//	CLayout*		m_pLayoutCurrent;			/* 順アクセス時の現在位置 */
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
	char*			m_pszLineComment;			/* 行コメントデリミタ */
	char*			m_pszLineComment2;			/* 行コメントデリミタ2 */
	char*			m_pszLineComment3;			/* 行コメントデリミタ3 */	//Jun. 01, 2001 JEPRO 追加
	char*			m_pszBlockCommentFrom;		/* ブロックコメントデリミタ(From) */
	char*			m_pszBlockCommentTo;		/* ブロックコメントデリミタ(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	char*			m_pszBlockCommentFrom2;		/* ブロックコメントデリミタ2(From) */
	char*			m_pszBlockCommentTo2;		/* ブロックコメントデリミタ2(To) */
//#endif
	int				m_nStringType;				/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */

	int				m_nPrevReferLine;
	CLayout*		m_pLayoutPrevRefer;
	/*
	|| 実装ヘルパ系
	*/
	CLayout* InsertLineNext( CLayout*, CDocLine*, /*const char*,*/ int, int, int, int, int );
	void AddLineBottom( CDocLine*, /*const char*,*/ int, int, int, int, int );
public:
	void Init();
	void Empty();

private:
	bool IsKinsokuHead( const char *pLine, int length );	/*!< 行頭禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail( const char *pLine, int length );	/*!< 行末禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKutoTen( unsigned char c1, unsigned char c2 );	/*!< 句読点文字をチェックする */	//@@@ 2002.04.17 MIK
	bool IsKinsokuKuto( const char *pLine, int length );	/*!< 句読点文字をチェックする */	//@@@ 2002.04.17 MIK
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUTMGR_H_ */


/*[EOF]*/
