//	$Id$
/*!	@file
	@brief アウトライン解析  データ要素

	@author Norio Nakatani
	@date	1998/06/23 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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
//	m_cmemFuncName = pszFuncName;
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
