//	$Id$
/*!	@file
	テキストのレイアウト情報

	@author Norio Nakatani
	@date 1998/3/11 新規作成
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

class CLayout;

#ifndef _CLAYOUT_H_
#define _CLAYOUT_H_


#ifndef TRUE
	#define TRUE 1
#endif
#ifndef FALSE
	#define FALSE 0
#endif

#ifndef NULL
	#define NULL 0
#endif

#include "CDocLine.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CLayout
{
public:
	/*
	||  Constructors
	*/
	CLayout();
	~CLayout();
	void DUMP( void );
public:
	CLayout*		m_pPrev;
	CLayout*		m_pNext;
	int				m_nLinePhysical;		/* 対応する改行単位の行の番号 */
	const CDocLine*	m_pCDocLine;
//	const char*		m_pLine;
	int				m_nOffset;		/* 対応する改行単位の行頭からのオフセット */
	int				m_nLength;		/* このレイアウト行の長さ(ハイト数) */
	int				m_nTypePrev;	/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	int				m_nTypeNext;	/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
//	enumEOLType		m_enumEOLType;	/* 改行コードの種類 */
//	int				m_nEOLLen;		/* 改行コードの長さ */
	CEOL			m_cEol;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUT_H_ */


/*[EOF]*/
