//	$Id$
/*!	@file
	アウトライン解析 データ配列

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

class CFuncInfoArr;

#ifndef _CFUNCINFOARR_H_
#define _CFUNCINFOARR_H_

#include "CFuncInfo.h"




/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/* アンドゥ・リドゥバッファ */
class CFuncInfoArr {
	public:
		CFuncInfoArr();	/* CFuncInfoArrクラス構築 */
		~CFuncInfoArr();	/* CFuncInfoArrクラス消滅 */
		CFuncInfo* GetAt( int );	/* 0<=の指定番号のデータを返す */
		void AppendData( CFuncInfo* );	/* 配列の最後にデータを追加する */
		void AppendData( int, int, char*, int );	/* 配列の最後にデータを追加する */
		int	GetNum( void ){	return m_nFuncInfoArrNum; }	/* 配列要素数を返す */
		void Empty( void );
		void DUMP( void );



		char		m_szFilePath[_MAX_PATH + 1];	/* 解析対象ファイル名 */
	private:
		int			m_nFuncInfoArrNum;	/* 配列要素数 */
		CFuncInfo**	m_ppcFuncInfoArr;	/* 配列 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFOARR_H_ */


/*[EOF]*/
