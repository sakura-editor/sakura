//	$Id$
/************************************************************************

	CDocLine.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDocLine;

#ifndef _CDOCLINE_H_
#define _CDOCLINE_H_

#include "CMemory.h"
#include "CEOL.h"

#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDocLine
{
public:
	/*
	||  Constructors
	*/
	CDocLine();
	~CDocLine();


	CDocLine*	m_pPrev;
	CDocLine*	m_pNext;
	CMemory*	m_pLine;
//	int			m_nType;	/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	int			m_bModify;	/* 変更フラグ */
	CEOL		m_cEol;		/* 行末コード */
//	enumEOLType	m_enumEOLType;	/* 改行コードの種類 */
//	int			m_nEOLLen;		/* 改行コードの長さ */ 		
//	int			m_nModifyCount;	/* 変更回数 */
};


///////////////////////////////////////////////////////////////////////
#endif /* _CDOCLINE_H_ */

/*[EOF]*/
