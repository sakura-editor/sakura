// $Id$
/************************************************************************

	CDocLineMgr.h
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/3/5  新規作成


************************************************************************/

class CDocLineMgr;

#ifndef _CDOCLINEMGR_H_
#define _CDOCLINEMGR_H_


#include <windows.h>
#include "CDocLine.h"
#include "CMemory.h"
#include "cRunningTimer.h"
#include "CJre.h"

struct DocLineReplaceArg {
	int			nDelLineFrom;			/* 削除範囲行  From 改行単位の行番号 0開始) */
	int			nDelPosFrom;			/* 削除範囲位置From 改行単位の行頭からのバイト位置　0開始) */
	int			nDelLineTo;			/* 削除範囲行　To   改行単位の行番号 0開始) */
	int			nDelPosTo;				/* 削除範囲位置To   改行単位の行頭からのバイト位置　0開始) */
	CMemory*	pcmemDeleted;		/* 削除されたデータを保存 */
	int			nDeletedLineNum;	/* 削除した行の総数 */
	const char*	pInsData;			/* 挿入するデータ */      
	int			nInsDataLen;		/* 挿入するデータの長さ */
	int			nInsLineNum;		/* 挿入によって増えた行の数 */
	int			nNewLine;			/* 挿入された部分の次の位置の行 */
	int			nNewPos;			/* 挿入された部分の次の位置のデータ位置 */
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDocLineMgr
{
public:
	/*
	||  Constructors
	*/
	CDocLineMgr();
	~CDocLineMgr();

	/*
	||  参照系
	*/
	char* GetAllData( int* );	/* 全行データを返す */
	int GetLineCount( void ) { return m_nLines; }	/* 全行数を返す */
	const char* GetLineStr( int , int* );
	const char* GetFirstLinrStr( int* );	/* 順アクセスモード：先頭行を得る */
	const char* GetNextLinrStr( int* );	/* 順アクセスモード：次の行を得る */
	int	WhereCurrentWord( int , int , int* , int*, CMemory*, CMemory* );	/* 現在位置の単語の範囲を調べる */
	static int	WhatKindOfChar( char*, int, int );	/* 現在位置の文字の種類を調べる */
	int PrevOrNextWord( int , int , int* , int );	/* 現在位置の左右の単語の先頭位置を調べる */
	int SearchWord( int , int , const char* , int , int , int , int , int* , int* , int*, CJre* ); /* 単語検索 */
//	static char* SearchString( const unsigned char*, int, int , const unsigned char* , int, int*, int*, int ); /* 文字列検索 */
	static char* SearchString( const unsigned char*, int, int , const unsigned char* , int, int*, int ); /* 文字列検索 */
	static void CreateCharCharsArr( const unsigned char*, int, int** );	/* 検索条件の情報 */
//	static void CreateCharUsedArr( const unsigned char*, int, const int*, int** ); /* 検索条件の情報(キー文字列の使用文字表)作成 */


	
	void DUMP( void );
//	void ResetAllModifyFlag( BOOL );	/* 行変更状態をすべてリセット */
	void ResetAllModifyFlag( void );	/* 行変更状態をすべてリセット */


	/*
	|| 更新系
	*/
	void Init();
	void Empty();
//	void InsertLineStr( int );	/* 指定行の前に追加する */

#if 0
	void AddLineStrSz( const char* );	/* 末尾に行を追加 Ver0 */
	void AddLineStr( const char*, int );	/* 末尾に行を追加 Ver1 */
	void AddLineStr( CMemory& );	/* 末尾に行を追加 Ver2 */
#endif	
	//	May 15, 2000 genta
	void AddLineStrX( const char*, int, CEOL );	/* 末尾に行を追加 Ver1.5 */

	void DeleteData( int , int , int , int* , int* , int* , int*, CMemory&, int );	/* データの削除 */

	/* 指定範囲のデータを置換(削除 & データを挿入)
	  Fromを含む位置からToの直前を含むデータを削除する
	  Fromの位置へテキストを挿入する
	*/
	void CDocLineMgr::ReplaceData(
		DocLineReplaceArg*
#if 0
		int			nDelLineFrom,		/* 削除範囲行  From 改行単位の行番号 0開始) */
		int			nDelPosFrom,		/* 削除範囲位置From 改行単位の行頭からのバイト位置　0開始) */
		int			nDelLineTo,			/* 削除範囲行　To   改行単位の行番号 0開始) */
		int			nDelPosTo,			/* 削除範囲位置To   改行単位の行頭からのバイト位置　0開始) */
		CMemory*	pcmemDeleted,		/* 削除されたデータを保存 */
		int*		pnDeletedLineNum,	/* 削除した行の総数 */
		const char*	pInsData,			/* 挿入するデータ */      
		int			nInsDataLen,		/* 挿入するデータの長さ */
		int*		pnInsLineNum,		/* 挿入によって増えた行の数 */
		int*		pnNewLine,			/* 挿入された部分の次の位置の行 */
		int*		pnNewPos			/* 挿入された部分の次の位置のデータ位置 */
#endif
	);
	void DeleteNode( CDocLine* );/* 行オブジェクトの削除、リスト変更、行数-- */
	void InsertNode( CDocLine*, CDocLine* );	/* 行オブジェクトの挿入、リスト変更、行数++ */

	
	void InsertData_CDocLineMgr( int , int , const char* , int , int* , int* , int*, int );	/* データの挿入 */
	//	Nov. 12, 2000 genta 引数追加
	int ReadFile( const char*, HWND, HWND, int, FILETIME*, int extraflag );
	//	Feb. 6, 2001 genta 引数追加(改行コード設定)
	int WriteFile( const char*, HWND, HWND, int, FILETIME*, CEOL );
	CDocLine* GetLineInfo( int );

	/*
	|| メンバ変数
	*/
	CDocLine*	m_pDocLineTop;
	CDocLine*	m_pDocLineBot;
	CDocLine*	m_pDocLineCurrent;	/* 順アクセス時の現在位置 */
	int			m_nLines;		/* 全行数 */
	int			m_nPrevReferLine;
	CDocLine*	m_pCodePrevRefer;
protected:

	/*
	|| 実装ヘルパ系
	*/
protected:



};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINEMGR_H_ */

/*[EOF]*/
