//	$Id$
/*!	@file
	CDicMgrクラス定義

	@author Norio Nakatani
	@date	1998/11/05 作成
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

class CDicMgr;

#ifndef _CDICMGR_H_
#define _CDICMGR_H_

#include <windows.h>
#include "CMemory.h"
#include "debug.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDicMgr
{
public:
	/*
	||  Constructors
	*/
	CDicMgr();
	~CDicMgr();

	/*
	||  Attributes & Operations
	*/
//	BOOL Open( char* );
	static BOOL Search( const char*, CMemory**, const char* );
	static int HokanSearch( const char* , BOOL, CMemory** , int, const char* );
//	BOOL Close( char* );


protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDICMGR_H_ */


/*[EOF]*/
