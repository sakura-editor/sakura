//	$Id$
/*!	@file
	アウトライン解析  データ要素
	
	@author Norio Nakatani
	@date	1998/06/23 作成
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

class CFuncInfo;

#ifndef _CFUNCINFO_H_
#define _CFUNCINFO_H_

#include "CMemory.h"




/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/* アンドゥ・リドゥバッファ */
class CFuncInfo {
	public:
		CFuncInfo( int, int, char*, int );	/* CFuncInfoクラス構築 */
		~CFuncInfo();	/* CFuncInfoクラス消滅 */

//	private:
		int			m_nFuncLineCRLF;	/* 関数のある行(CRLF単位) */
		int			m_nFuncLineLAYOUT;	/* 関数のある行(折り返し単位) */
		CMemory		m_cmemFuncName;	/* 関数名 */
		int			m_nInfo;		/* 付加情報 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFO_H_ */

/*[EOF]*/
