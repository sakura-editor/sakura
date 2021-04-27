/*!	@file
	@brief アウトライン解析  データ要素

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CFuncInfo.h"

/*! CFuncInfoクラス構築 */
CFuncInfo::CFuncInfo(
	CLogicInt		nFuncLineCRLF,		//!< 関数のある行(CRLF単位)
	CLogicInt		nFuncColCRLF,		//!< 関数のある桁(CRLF単位)
	CLayoutInt		nFuncLineLAYOUT,	//!< 関数のある行(折り返し単位)
	CLayoutInt		nFuncColLAYOUT,		//!< 関数のある桁(折り返し単位)
	const WCHAR*	pszFuncName,		//!< 関数名
	const WCHAR*	pszFileName,
	int				nInfo,				//!< 付加情報
	int				nDepth				//!< 深さ
)
{
	m_nFuncLineCRLF = nFuncLineCRLF;		/* 関数のある行(CRLF単位) */
	m_nFuncColCRLF = nFuncColCRLF;			/* 関数のある桁(CRLF単位) */
	m_nFuncLineLAYOUT = nFuncLineLAYOUT;	/* 関数のある行(折り返し単位) */
	m_nFuncColLAYOUT = nFuncColLAYOUT;		/* 関数のある桁(折り返し単位) */
	m_cmemFuncName = pszFuncName;
	if( pszFileName ){
		m_cmemFileName = pszFileName;
	}
	m_nInfo = nInfo;
	m_nDepth = nDepth;
	return;
}

/* CFuncInfoクラス消滅 */
CFuncInfo::~CFuncInfo()
{
}
