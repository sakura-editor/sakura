//	$Id$
/*!	@file
	編集操作要素

	@author Norio Nakatani
	@date 1998/06/09 新規作成
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

class COpe;

#ifndef _COPE_H_
#define _COPE_H_

#include "CMemory.h"




/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/* 編集操作要素 COpe */
class COpe {
	public:
		COpe();		/* COpeクラス構築 */
		~COpe();	/* COpeクラス消滅 */

		void DUMP( void );	/* 編集操作要素のダンプ */

		int		m_nOpe;			/* 操作種別 */

//- 1999.12.22 メモリ食う
//-		int		m_nCaretPosX_Before;	/* 操作前のキャレット位置Ｘ */
//-		int		m_nCaretPosY_Before;	/* 操作前のキャレット位置Ｙ */
//-		int		m_nCaretPosX_To;		/* 操作前のキャレット位置Ｘ To */
//-		int		m_nCaretPosY_To;		/* 操作前のキャレット位置Ｙ To */
//-		int		m_nCaretPosX_After; 	/* 操作後のキャレット位置Ｘ */
//-		int		m_nCaretPosY_After; 	/* 操作後のキャレット位置Ｙ */

		int		m_nCaretPosX_PHY_Before;	/* カーソル位置 改行単位行先頭からのバイト数（０開始） */
		int		m_nCaretPosY_PHY_Before;	/* カーソル位置 改行単位行の行番号（０開始） */
		int		m_nCaretPosX_PHY_To;		/* 操作前のキャレット位置Ｘ To 改行単位行先頭からのバイト数（０開始）*/
		int		m_nCaretPosY_PHY_To;		/* 操作前のキャレット位置Ｙ To 改行単位行の行番号（０開始）*/
		int		m_nCaretPosX_PHY_After;		/* カーソル位置 改行単位行先頭からのバイト数（０開始） */
		int		m_nCaretPosY_PHY_After;		/* カーソル位置 改行単位行の行番号（０開始） */



		int		m_nDataLen;					/* 操作に関連するデータのサイズ */
		CMemory*	m_pcmemData;			/* 操作に関連するデータ */

	public:
	private:
};



///////////////////////////////////////////////////////////////////////
#endif /* _COPE_H_ */


/*[EOF]*/
