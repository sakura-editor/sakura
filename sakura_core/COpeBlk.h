//	$Id$
/*!	@file
	編集操作要素ブロック
	
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

class COpeBlk;

#ifndef _COPEBLK_H_
#define _COPEBLK_H_

#include <windows.h>
#include "COpe.h"
#include "CLayoutMgr.h"




/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/* 編集操作要素ブロック　COpeBlk */
class COpeBlk {
	public:
		COpeBlk();	/* COpeBlkクラス構築 */
		~COpeBlk();	/* COpeBlkクラス消滅 */

		int GetNum( void ){ return m_nCOpeArrNum; };	/* 操作の数を返す */
//		int AppendOpe( COpe*, CLayoutMgr* );	/* 操作の追加 */
		int AppendOpe( COpe* );	/* 操作の追加 */
		COpe* GetOpe( int );	/* 操作を返す */

		void DUMP( void );	/* 編集操作要素ブロックのダンプ */
	private:
		void Init( void );
		void Empty( void );

		/* データ */
		int		m_nCOpeArrNum;	/* 操作の数 */
		COpe**	m_ppCOpeArr;	/* 操作の配列 */
};



//////////////////////////////////////////////////////////////////////12
#endif /* _COPEBLK_H_ */

/*[EOF]*/
