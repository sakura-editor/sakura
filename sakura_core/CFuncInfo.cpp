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

#include "CFuncInfo.h"
#include <windows.h>
//#include <string.h>

/*! CFuncInfoクラス構築 */
CFuncInfo::CFuncInfo(
	int		nFuncLineCRLF,	/*!< 関数のある行(CRLF単位) */
	int		nFuncLineLAYOUT,/*!< 関数のある行(折り返し単位) */
//	int		nFuncLine,		/*!< 関数のある行 */
	char*	pszFuncName,	/*!< 関数名 */
	int		nInfo			/*!< 付加情報 */
)
{
	m_nFuncLineCRLF = nFuncLineCRLF;		/* 関数のある行(CRLF単位) */
	m_nFuncLineLAYOUT = nFuncLineLAYOUT;	/* 関数のある行(折り返し単位) */
//	m_nFuncLine = nFuncLine;;
//    m_cmemFuncName = pszFuncName;
//	m_cmemFuncName.SetData( pszFuncName, lstrlen( pszFuncName ) );
	m_cmemFuncName.SetDataSz( pszFuncName );

	m_nInfo = nInfo;
	return;
}




/* CFuncInfoクラス消滅 */
CFuncInfo::~CFuncInfo()
{

}


/*[EOF]*/
