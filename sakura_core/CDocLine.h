/*!	@file
	@brief 文書データ1行

	@author Norio Nakatani

	@date 2001/12/03 hor しおり(bookmark)機能追加に伴うメンバー追加
	@date 2001/12/18 hor bookmark, 修正フラグのアクセス関数化
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


	CDocLine*	m_pPrev;	//!< 一つ前の要素
	CDocLine*	m_pNext;	//!< 一つ後の要素
	CNativeW	m_cLine;	//!< データ  2007.10.11 kobake ポインタではなく、実体を持つように変更
	CEOL		m_cEol;		//!< 行末コード

	bool		IsModifyed  ( void ) const { return m_bMark.m_bMarkArray.m_bModify ? true : false; }	// 変更フラグの状態を取得する	//@@@ 2002.05.25 MIK
	void		SetModifyFlg( bool bFlg )  { m_bMark.m_bMarkArray.m_bModify = bFlg ? 1 : 0; }	// 変更フラグの状態を指定する	//@@@ 2002.05.25 MIK

	bool		IsBookMarked( void ) const { return m_bMark.m_bMarkArray.m_bBookMark ? true : false; }	// ブックマークの状態を取得する	//@@@ 2002.05.25 MIK
	void		SetBookMark ( bool bFlg )  { m_bMark.m_bMarkArray.m_bBookMark = bFlg ? 1 : 0; }	// ブックマークの状態を指定する	//@@@ 2002.05.25 MIK

	int			IsDiffMarked( void ) const { return (int)m_bMark.m_bMarkArray.m_bDiffMark; }	//差分状態を取得する	//@@@ 2002.05.25 MIK
	void		SetDiffMark( int type )    { m_bMark.m_bMarkArray.m_bDiffMark = type; }		//差分状態を設定する	//@@@ 2002.05.25 MIK

	bool		IsEmptyLine( void ) const;	//	このCDocLineが空行（スペース、タブ、改行記号のみの行）かどうか。
	CLogicInt	GetLengthWithoutEOL( void ) const	{ return m_cLine.GetStringLength() - m_cEol.GetLen(); } //!< 戻り値は文字単位。
	const wchar_t*	GetPtr( void ) const			{ return m_cLine.GetStringPtr(); }
	CLogicInt	GetLength() const					{ return m_cLine.GetStringLength(); }	//	CMemoryIterator用

	//! 文字列参照を取得
	CStringRef GetStringRef() const{ return CStringRef(GetPtr(), GetLength()); }
protected:
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
