//	$Id$
/*!	@file
	@brief 文書データ1行

	@author Norio Nakatani
	@date 2001/12/03 hor しおり(bookmark)機能追加に伴うメンバー追加
	@date 2001/12/18 hor bookmark, 修正フラグのアクセス関数化
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDocLine;

#ifndef _CDOCLINE_H_
#define _CDOCLINE_H_

#include "CEOL.h"
#include "CMemory.h"


//@@@ 2002.05.25 MIK
#define	MARK_DIFF_APPEND	1	//追加
#define	MARK_DIFF_CHANGE	2	//変更
#define	MARK_DIFF_DELETE	3	//削除
#define	MARK_DIFF_DEL_EX	4	//削除(EOF以降)

//!	文書データ1行
class CDocLine
{
public:
	/*
	||  Constructors
	*/
	CDocLine();
	~CDocLine();


	CDocLine*	m_pPrev;	/*!< 一つ前の要素 */
	CDocLine*	m_pNext;	/*!< 一つ後の要素 */
	CMemory*	m_pLine;	/*!< データ */
//	int			m_nType;	/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	CEOL		m_cEol;		/*!< 行末コード */
//	enumEOLType	m_enumEOLType;	/* 改行コードの種類 */
//	int			m_nEOLLen;		/* 改行コードの長さ */
//	int			m_nModifyCount;	/* 変更回数 */

//	bool		IsModifyed  ( void ) const { return m_bModify;  }	// 変更フラグの状態を取得する	2001.12.18 hor
//	void		SetModifyFlg( bool bFlg )  { m_bModify = bFlg;  }	// 変更フラグの状態を指定する	2001.12.18 hor
	bool		IsModifyed  ( void ) const { return m_bMark.m_bMarkArray.m_bModify ? true : false; }	// 変更フラグの状態を取得する	//@@@ 2002.05.25 MIK
	void		SetModifyFlg( bool bFlg )  { m_bMark.m_bMarkArray.m_bModify = bFlg ? 1 : 0; }	// 変更フラグの状態を指定する	//@@@ 2002.05.25 MIK

//	bool		IsBookMarked( void ) const { return m_bBookMark;}	// ブックマークの状態を取得する 2001.12.14 hor
//	void		SetBookMark ( bool bFlg )  { m_bBookMark = bFlg;}	// ブックマークの状態を指定する 2001.12.14 hor
	bool		IsBookMarked( void ) const { return m_bMark.m_bMarkArray.m_bBookMark ? true : false; }	// ブックマークの状態を取得する	//@@@ 2002.05.25 MIK
	void		SetBookMark ( bool bFlg )  { m_bMark.m_bMarkArray.m_bBookMark = bFlg ? 1 : 0; }	// ブックマークの状態を指定する	//@@@ 2002.05.25 MIK

	int			IsDiffMarked( void ) const { return (int)m_bMark.m_bMarkArray.m_bDiffMark; }	//差分状態を取得する	//@@@ 2002.05.25 MIK
	void		SetDiffMark( int type )    { m_bMark.m_bMarkArray.m_bDiffMark = type; }		//差分状態を設定する	//@@@ 2002.05.25 MIK

	bool		IsEmptyLine( void );	//	このCDocLineが空行（スペース、タブ、改行記号のみの行）かどうか。
	int			GetLengthWithoutEOL( void ){ return m_pLine->GetLength() - m_cEol.GetLen(); }
	char*		GetPtr( void ) const              { return m_pLine->GetPtr(); }
protected:
//	bool		m_bModify;					/*!< 変更フラグ */
//	bool		m_bBookMark;				/*!< ブックマーク */

	//マーク情報
	union {
		unsigned char	m_bAllMark;
		struct Mark_tag {
			unsigned m_bModify		: 1;	//変更フラグ
			unsigned m_bBookMark	: 1;	//ブックマーク
			unsigned m_bDiffMark	: 3;	//DIFF差分情報
		} m_bMarkArray;
	} m_bMark;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINE_H_ */


/*[EOF]*/
