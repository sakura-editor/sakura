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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CLayoutMgr;

#ifndef _CLAYOUTMGR_H_
#define _CLAYOUTMGR_H_

//#include <windows.h>
#include "CLayout.h"
#include "CDocLineMgr.h"
#include "CMemory.h"
 //	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
#include "CBregexp.h"

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif


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
	BOOL		bUndo;					/*!< Undo操作かどうか */
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
	const char* GetLineStr2( int , int*, const CLayout** );	/* 指定された物理行のデータへのポインタとその長さを返す */
//	const CLayout* GetLineData( int );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	CLayout* Search( int );	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	int WhereCurrentWord( int , int , int* , int* , int* , int*, CMemory*, CMemory* );	/* 現在位置の単語の範囲を調べる */
	int PrevOrNextWord( int, int, int*, int*, int );	/* 現在位置の左右の単語の先頭位置を調べる */
	int SearchWord( int	, int , const char* , int , int , int , int , int* , int* , int* , int*, CBregexp* );	/* 単語検索 */
	void CaretPos_Phys2Log( int, int, int*, int* );
	void CaretPos_Log2Phys( int, int, int*, int* );
	void DUMP( void );	/* テスト用にレイアウト情報をダンプ */

	/*
	|| 更新系
	*/
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
	void SetLayoutInfo( int , BOOL, int , char*, char*, char*, char*, char*, char*, char*, int, int, HWND, BOOL, BOOL ); /* レイアウト情報の変更 */	//Jun. 01, 2001 JEPRO char* (行コメントデリミタ3用)を1つ追加
//#else
//	void SetLayoutInfo( int , BOOL, int , char*, char*, char*, char*, int, int, HWND, BOOL, BOOL ); /* レイアウト情報の変更 */
//#endif
	void DeleteData_CLayoutMgr( int , int , int, int *, int *, int *, CMemory&, BOOL, BOOL, BOOL );	/* 行内文字削除 */
	void InsertData_CLayoutMgr( int, int, const char*, int, int*, int*, int*, int*, BOOL, BOOL, BOOL );	/* 文字列挿入 */

/* 文字列置換 */
void CLayoutMgr::ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
#if 0
		int			nDelLineFrom,			/* 削除範囲行  From レイアウト行番号 */
		int			nDelColmFrom,			/* 削除範囲位置From レイアウト行桁位置 */
		int			nDelLineTo,				/* 削除範囲行  To   レイアウト行番号 */
		int			nDelColmTo,				/* 削除範囲位置To   レイアウト行桁位置 */
		CMemory*	pcmemDeleted,			/* 削除されたデータ */
		const char*	pInsData,				/* 挿入するデータ */
		int			nInsDataLen,			/* 挿入するデータの長さ */

		int*		pnAddLineNum,			/* 再描画ヒント レイアウト行の増減 */
		int*		pnModLineFrom,			/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */
		int*		pnModLineTo,			/* 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う) */

		int*		pnNewLine,				/* 挿入された部分の次の位置の行(レイアウト行) */
		int*		pnNewPos,				/* 挿入された部分の次の位置のデータ位置(レイアウト桁位置) */

		BOOL		bDispSSTRING,			/* シングルクォーテーション文字列を表示する */
		BOOL		bDispWSTRING,			/* ダブルクォーテーション文字列を表示する */
		BOOL		bUndo					/* Undo操作かどうか */
#endif
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

};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUTMGR_H_ */


/*[EOF]*/
