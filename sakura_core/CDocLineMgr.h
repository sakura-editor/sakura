/*!	@file
	@brief 行データの管理

	@author Norio Nakatani
	@date 1998/3/5  新規作成
	@date 2001/06/23 N.Nakatani WhereCurrentWord_2()追加 staticメンバ
	@date 2001/12/03 hor しおり(bookmark)機能追加に伴う関数追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, hor, genta
	Copyright (C) 2002, aroka, MIK, hor
	Copyright (C) 2003, Moca, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDocLineMgr;

#ifndef _CDOCLINEMGR_H_
#define _CDOCLINEMGR_H_

#include <windows.h>
#include "global.h" // 2002/2/10 aroka
#include "CEol.h" // 2002/2/10 aroka


/* 文字種類識別子 */
#define	CK_NULL			0	/*!< NULL 0x0<=c<=0x0 */
#define	CK_TAB			1	/*!< タブ 0x9<=c<=0x9 */
#define	CK_CR			2	/*!< CR = 0x0d  */
#define	CK_LF			3	/*!< LF = 0x0a  */

#define	CK_SPACE		4	/*!< 半角のスペース 0x20<=c<=0x20 */
#define	CK_CSYM			5	/*!< 半角の英字、アンダースコア、数字のいずれか */
#define	CK_KATA			6	/*!< 半角のカタカナ 0xA1<=c<=0xFD */
#define	CK_ETC			7	/*!< 半角のその他 */

#define	CK_MBC_SPACE	12	/*!< 2バイトのスペース */
							/*!< 0x8140<=c<=0x8140 全角スペース */
#define	CK_MBC_NOVASU	13	/*!< 伸ばす記号 0x815B<=c<=0x815B 'ー' */
#define	CK_MBC_CSYM		14	/*!< 2バイトの英字、アンダースコア、数字のいずれか */
							/*!< 0x8151<=c<=0x8151 全角アンダースコア */
							/*!< 0x824F<=c<=0x8258 全角数字 */
							/*!< 0x8260<=c<=0x8279 全角英字大文字 */
							/*!< 0x8281<=c<=0x829a 全角英字小文字 */
#define	CK_MBC_KIGO		15	/*!< 2バイトの記号 */
							/*!< 0x8141<=c<=0x81FD */
#define	CK_MBC_HIRA		16	/*!< 2バイトのひらがな */
							/*!< 0x829F<=c<=0x82F1 全角ひらがな */
#define	CK_MBC_KATA		17	/*!< 2バイトのカタカナ */
							/*!< 0x8340<=c<=0x8396 全角カタカナ */
#define	CK_MBC_GIRI		18	/*!< 2バイトのギリシャ文字 */
							/*!< 0x839F<=c<=0x83D6 全角ギリシャ文字 */
#define	CK_MBC_ROS		19	/*!< 2バイトのロシア文字: */
							/*!< 0x8440<=c<=0x8460 全角ロシア文字大文字 */
							/*!< 0x8470<=c<=0x8491 全角ロシア文字小文字 */
#define	CK_MBC_SKIGO	20	/*!< 2バイトの特殊記号 */
							/*!< 0x849F<=c<=0x879C 全角特殊記号 */
#define	CK_MBC_ETC		21	/*!< 2バイトのその他（漢字など） */
#define	CK_MBC_DAKU		22	/*!< 2バイトの濁点（゛゜） */



class CDocLine; // 2002/2/10 aroka
class CMemory; // 2002/2/10 aroka
class CBregexp; // 2002/2/10 aroka

struct DocLineReplaceArg {
	int			nDelLineFrom;		/* 削除範囲行  From 改行単位の行番号 0開始) */
	int			nDelPosFrom;		/* 削除範囲位置From 改行単位の行頭からのバイト位置 0開始) */
	int			nDelLineTo;			/* 削除範囲行  To   改行単位の行番号 0開始) */
	int			nDelPosTo;			/* 削除範囲位置To   改行単位の行頭からのバイト位置 0開始) */
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
	const char* GetLineStrWithoutEOL( int , int* ); // 2003.06.22 Moca
	const char* GetFirstLinrStr( int* );	/* 順アクセスモード：先頭行を得る */
	const char* GetNextLinrStr( int* );	/* 順アクセスモード：次の行を得る */

	int WhereCurrentWord( int , int , int* , int*, CMemory*, CMemory* );	/* 現在位置の単語の範囲を調べる */
	// 2001/06/23 N.Nakatani WhereCurrentWord_2()追加 staticメンバ
	static int WhereCurrentWord_2( const char*, int, int , int* , int*, CMemory*, CMemory* );	/* 現在位置の単語の範囲を調べる */

	// 現在位置の文字の種類を調べる
	static int WhatKindOfChar( const char*, int, int );	/* 現在位置の文字の種類を調べる */
	// 二つの文字を結合したものの種類を調べる
	static int WhatKindOfTwoChars( int kindPre, int kindCur );
	int PrevOrNextWord( int , int , int* , BOOL bLEFT, BOOL bStopsBothEnds );	/* 現在位置の左右の単語の先頭位置を調べる */

	//	pLine（長さ：nLineLen）の文字列から次の単語を探す。探し始める位置はnIdxで指定。
	static int SearchNextWordPosition(
		const char* pLine,
		int			nLineLen,
		int			nIdx,		//	桁数
		int*		pnColmNew,	//	見つかった位置
		BOOL		bStopsBothEnds	//	単語の両端で止まる
	);
	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	int SearchWord( int , int , const char* , int , int , int , int , int* , int* , int*, CBregexp* ); /* 単語検索 */
//	static char* SearchString( const unsigned char*, int, int , const unsigned char* , int, int*, int*, int ); /* 文字列検索 */
	static char* SearchString( const unsigned char*, int, int , const unsigned char* , int, int*, int ); /* 文字列検索 */
	static void CreateCharCharsArr( const unsigned char*, int, int** );	/* 検索条件の情報 */
//	static void CreateCharUsedArr( const unsigned char*, int, const int*, int** ); /* 検索条件の情報(キー文字列の使用文字表)作成 */



	void DUMP( void );
//	void ResetAllModifyFlag( BOOL );	/* 行変更状態をすべてリセット */
	void ResetAllModifyFlag( void );	/* 行変更状態をすべてリセット */


// From Here 2001.12.03 hor
	void ResetAllBookMark( void );			/* ブックマークの全解除 */
	int SearchBookMark( int , int , int* ); /* ブックマーク検索 */
// To Here 2001.12.03 hor

	//@@@ 2002.05.25 MIK
	void ResetAllDiffMark( void );			/* 差分表示の全解除 */
	int SearchDiffMark( int , int , int* ); /* 差分検索 */
	void SetDiffMarkRange( int nMode, int nStartLine, int nEndLine );	/* 差分範囲の登録 */
	bool IsDiffUse( void ) const { return m_bIsDiffUse; }	/* DIFF使用中 */

// From Here 2002.01.16 hor
	void MarkSearchWord( const char* , int , int , int , CBregexp* ); /* 検索条件に該当する行にブックマークをセットする */
	void SetBookMarks( char* ); /* 物理行番号のリストからまとめて行マーク */
	char* GetBookMarks( void ); /* 行マークされてる物理行番号のリストを作る */
// To Here 2001.01.16 hor

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

	void DeleteData_CDocLineMgr(
		int			nLine,
		int			nDelPos,
		int			nDelLen,
		int*		pnModLineOldFrom,	/* 影響のあった変更前の行(from) */
		int*		pnModLineOldTo,		/* 影響のあった変更前の行(to) */
		int*		pnDelLineOldFrom,	/* 削除された変更前論理行(from) */
		int*		pnDelLineOldNum,	/* 削除された行数 */
		CMemory&	cmemDeleted			/* 削除されたデータ */
	);

	/* 指定範囲のデータを置換(削除 & データを挿入)
	  Fromを含む位置からToの直前を含むデータを削除する
	  Fromの位置へテキストを挿入する
	*/
	void CDocLineMgr::ReplaceData(
		DocLineReplaceArg*
#if 0
		int			nDelLineFrom,		/* 削除範囲行  From 改行単位の行番号 0開始) */
		int			nDelPosFrom,		/* 削除範囲位置From 改行単位の行頭からのバイト位置 0開始) */
		int			nDelLineTo,			/* 削除範囲行  To   改行単位の行番号 0開始) */
		int			nDelPosTo,			/* 削除範囲位置To   改行単位の行頭からのバイト位置 0開始) */
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

	/* データの挿入 */
	void InsertData_CDocLineMgr(
		int			nLine,
		int			nInsPos,
		const char*	pInsData,
		int			nInsDataLen,
		int*		pnInsLineNum,	/* 挿入によって増えた行の数 */
		int*		pnNewLine,		/* 挿入された部分の次の位置の行 */
		int*		pnNewPos		/* 挿入された部分の次の位置のデータ位置 */
	);

	//	Nov. 12, 2000 genta 引数追加
	//	Jul. 26, 2003 ryoji BOM引数追加
	int ReadFile( const char*, HWND, HWND, int, FILETIME*, int extraflag, BOOL* pbBomExist = NULL );
	//	Feb. 6, 2001 genta 引数追加(改行コード設定)
	//	Jul. 26, 2003 ryoji BOM引数追加
	int WriteFile( const char*, HWND, HWND, int, FILETIME*, CEOL, BOOL bBomExist = FALSE );
	CDocLine* GetLineInfo( int );
	// 2002/2/10 aroka メンバを private にしてアクセサ追加
	CDocLine* GetDocLineTop() const { return m_pDocLineTop; }
	CDocLine* GetDocLineBottom() const { return m_pDocLineBot; }

private:

	/*
	|| メンバ変数
	*/
	CDocLine*	m_pDocLineTop;
	CDocLine*	m_pDocLineBot;
	CDocLine*	m_pDocLineCurrent;	/* 順アクセス時の現在位置 */
	int			m_nLines;		/* 全行数 */
	int			m_nPrevReferLine;
	CDocLine*	m_pCodePrevRefer;
	bool		m_bIsDiffUse;	/* DIFF差分表示実施中 */	//@@@ 2002.05.25 MIK
protected:

	/*
	|| 実装ヘルパ系
	*/
protected:



};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINEMGR_H_ */


/*[EOF]*/
