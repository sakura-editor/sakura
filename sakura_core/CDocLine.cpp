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

#include "CDocLine.h"

CDocLine::CDocLine()
{
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pLine = NULL;
//	m_nType = 0;	/* タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列 */
	m_bModify = TRUE;	/* 変更フラグ */
//	m_enumEOLType = EOL_NONE;	/* 改行コードの種類 */
//	m_nEOLLen = gm_pnEolLenArr[EOL_NONE];
//	m_nModifyCount = 32000;	/* 変更回数 */
	return;
}


CDocLine::~CDocLine()
{
	if( NULL != m_pLine ){
		delete m_pLine;
		m_pLine = NULL;
	}
	return;
}


/*[EOF]*/
