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

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDocLine;

#ifndef _CDOCLINE_H_
#define _CDOCLINE_H_

#include "CEOL.h"
#include "CMemory.h"

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif

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

	bool		IsModifyed  ( void ) const { return m_bModify;  }	// 変更フラグの状態を取得する	2001.12.18 hor
	void		SetModifyFlg( bool bFlg )  { m_bModify = bFlg;  }	// 変更フラグの状態を指定する	2001.12.18 hor

	bool		IsBookMarked( void ) const { return m_bBookMark;}	// ブックマークの状態を取得する 2001.12.14 hor
	void		SetBookMark ( bool bFlg )  { m_bBookMark = bFlg;}	// ブックマークの状態を指定する 2001.12.14 hor

	bool		IsEmptyLine( void );	//	このCDocLineが空行（スペース、タブ、改行記号のみの行）かどうか。
	int			GetLengthWithoutEOL( void ){ return m_pLine->GetLength() - m_cEol.GetLen(); }
	char*		GetPtr( void )             { return m_pLine->GetPtr(); }
protected:
	bool		m_bModify;					/*!< 変更フラグ */
	bool		m_bBookMark;				/*!< ブックマーク */
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINE_H_ */


/*[EOF]*/
