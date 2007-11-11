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
class CDocLine; // 2002/2/10 aroka
class CMemory; // 2002/2/10 aroka
class CBregexp; // 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "charset/CCodeBase.h"

struct DocLineReplaceArg {
	CLogicRange		sDelRange;			//!< 削除範囲。ロジック単位。
	CNativeW*		pcmemDeleted;		//!< 削除されたデータを保存 */
	CLogicInt		nDeletedLineNum;	//!< 削除した行の総数
	const wchar_t*	pInsData;			//!< 挿入するデータ
	int				nInsDataLen;		//!< 挿入するデータの長さ
	CLogicInt		nInsLineNum;		//!< 挿入によって増えた行の数
	CLogicPoint		ptNewPos;			//!< 挿入された部分の次の位置
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
//2007.09.30 kobake WhereCurrentWord_2 を CWordParse に移動
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
	wchar_t* GetAllData( int* );	/* 全行データを返す */
	CLogicInt GetLineCount( void ) { return m_nLines; }	/* 全行数を返す */
	const wchar_t* GetLineStr( CLogicInt , CLogicInt* );
	const wchar_t* GetLineStrWithoutEOL( CLogicInt , int* ); // 2003.06.22 Moca
	const wchar_t* GetFirstLinrStr( int* );	/* 順アクセスモード：先頭行を得る */
	const wchar_t* GetNextLinrStr( int* );	/* 順アクセスモード：次の行を得る */

	bool WhereCurrentWord( CLogicInt , CLogicInt , CLogicInt* , CLogicInt*, CNativeW*, CNativeW* );	/* 現在位置の単語の範囲を調べる */

	bool PrevOrNextWord( CLogicInt , CLogicInt , CLogicInt* , BOOL bLEFT, BOOL bStopsBothEnds );	/* 現在位置の左右の単語の先頭位置を調べる */
	//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
	int SearchWord( CLogicPoint ptSerachBegin, const wchar_t* , ESearchDirection eDirection, const SSearchOption& sSearchOption , CLogicRange* pMatchRange, CBregexp* ); /* 単語検索 */
//	static char* SearchString( const unsigned char*, int, int , const unsigned char* , int, int*, int*, int ); /* 文字列検索 */
	static const wchar_t* SearchString( const wchar_t*, int, int , const wchar_t* , int, int*, bool ); /* 文字列検索 */
	static void CreateCharCharsArr( const wchar_t*, int, int** );	/* 検索条件の情報 */
//	static void CreateCharUsedArr( const unsigned char*, int, const int*, int** ); /* 検索条件の情報(キー文字列の使用文字表)作成 */



	void DUMP( void );
	void ResetAllModifyFlag( void );	/* 行変更状態をすべてリセット */


// From Here 2001.12.03 hor
	void ResetAllBookMark( void );			/* ブックマークの全解除 */
	int SearchBookMark( CLogicInt nLineNum, ESearchDirection , CLogicInt* pnLineNum ); /* ブックマーク検索 */
// To Here 2001.12.03 hor

	//@@@ 2002.05.25 MIK
	void ResetAllDiffMark( void );			/* 差分表示の全解除 */
	int SearchDiffMark( CLogicInt , ESearchDirection , CLogicInt* ); /* 差分検索 */
	void SetDiffMarkRange( int nMode, CLogicInt nStartLine, CLogicInt nEndLine );	/* 差分範囲の登録 */
	bool IsDiffUse( void ) const { return m_bIsDiffUse; }	/* DIFF使用中 */

// From Here 2002.01.16 hor
	void MarkSearchWord( const wchar_t* , const SSearchOption& , CBregexp* ); /* 検索条件に該当する行にブックマークをセットする */
	void SetBookMarks( wchar_t* ); /* 物理行番号のリストからまとめて行マーク */
	wchar_t* GetBookMarks( void ); /* 行マークされてる物理行番号のリストを作る */
// To Here 2001.01.16 hor

	/*
	|| 更新系
	*/
	void Init();
	void Empty();

	//	May 15, 2000 genta
	void AddLineStrX( const wchar_t*, int, CEOL );	/* 末尾に行を追加 Ver1.5 */

	void DeleteData_CDocLineMgr(
		CLogicInt	nLine,
		CLogicInt	nDelPos,
		CLogicInt	nDelLen,
		CLogicInt*	pnModLineOldFrom,	/* 影響のあった変更前の行(from) */
		CLogicInt*	pnModLineOldTo,		/* 影響のあった変更前の行(to) */
		CLogicInt*	pnDelLineOldFrom,	/* 削除された変更前論理行(from) */
		CLogicInt*	pnDelLineOldNum,	/* 削除された行数 */
		CNativeW*	cmemDeleted			/* 削除されたデータ */
	);

	/* 指定範囲のデータを置換(削除 & データを挿入)
	  Fromを含む位置からToの直前を含むデータを削除する
	  Fromの位置へテキストを挿入する
	*/
	void CDocLineMgr::ReplaceData( DocLineReplaceArg* );
	void DeleteNode( CDocLine* );/* 行オブジェクトの削除、リスト変更、行数-- */
	void InsertNode( CDocLine*, CDocLine* );	/* 行オブジェクトの挿入、リスト変更、行数++ */

	/* データの挿入 */
	void InsertData_CDocLineMgr(
		CLogicInt		nLine,
		CLogicInt		nInsPos,
		const wchar_t*	pInsData,
		CLogicInt		nInsDataLen,
		CLogicInt*		pnInsLineNum,	// 挿入によって増えた行の数
		CLogicPoint*	pptNewPos		// 挿入された部分の次の位置
	);

	//	Nov. 12, 2000 genta 引数追加
	//	Jul. 26, 2003 ryoji BOM引数追加
	int ReadFile( const TCHAR* pszPath, HWND, HWND, ECodeType, FILETIME*, int extraflag, BOOL* pbBomExist = NULL );

	//	Feb. 6, 2001 genta 引数追加(改行コード設定)
	//	Jul. 26, 2003 ryoji BOM引数追加
	EConvertResult WriteFile( const TCHAR*, HWND, HWND, ECodeType, FILETIME*, CEOL, BOOL bBomExist = FALSE );

	CDocLine* GetLineInfo( CLogicInt nLine );
	// 2002/2/10 aroka メンバを private にしてアクセサ追加
	CDocLine* GetDocLineTop() const { return m_pDocLineTop; }
	CDocLine* GetDocLineBottom() const { return m_pDocLineBot; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         実装補助                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	// -- -- チェーン関数 -- -- // 2007.10.11 kobake 作成
	void _PushBottom(CDocLine* pDocLineNew);             //!< 最下部に挿入
	void _Insert(CDocLine* pDocLineNew, CDocLine* pPos); //!< pPosの直前に挿入

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	CDocLine*	m_pDocLineTop; //最初の行
	CDocLine*	m_pDocLineBot; //最後の行(※1行しかない場合はm_pDocLineTopと等しくなる)
	CDocLine*	m_pDocLineCurrent;	/* 順アクセス時の現在位置 */
	CLogicInt	m_nLines;		/* 全行数 */
	CLogicInt	m_nPrevReferLine;
	CDocLine*	m_pCodePrevRefer;
	bool		m_bIsDiffUse;	/* DIFF差分表示実施中 */	//@@@ 2002.05.25 MIK
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINEMGR_H_ */


/*[EOF]*/
