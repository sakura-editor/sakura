//	$Id$
/*!	@file
	文書データ1行

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

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
