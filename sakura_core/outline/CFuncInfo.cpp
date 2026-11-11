/*!	@file
	@brief アウトライン解析  データ要素

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "outline/CFuncInfo.h"

/*! CFuncInfoクラス構築 */
CFuncInfo::CFuncInfo(
	CLogicInt		nFuncLineCRLF,		//!< 関数のある行(CRLF単位)
	CLogicInt		nFuncColCRLF,		//!< 関数のある桁(CRLF単位)
	CLayoutInt		nFuncLineLAYOUT,	//!< 関数のある行(折り返し単位)
	CLayoutInt		nFuncColLAYOUT,		//!< 関数のある桁(折り返し単位)
	const WCHAR*	pszFuncName,		//!< 関数名
	const WCHAR*	pszFileName,
	int				nInfo				//!< 付加情報
)
: m_nDepth(0) // 深さ
{
	m_nFuncLineCRLF = nFuncLineCRLF;		/* 関数のある行(CRLF単位) */
	m_nFuncColCRLF = nFuncColCRLF;			/* 関数のある桁(CRLF単位) */
	m_nFuncLineLAYOUT = nFuncLineLAYOUT;	/* 関数のある行(折り返し単位) */
	m_nFuncColLAYOUT = nFuncColLAYOUT;		/* 関数のある桁(折り返し単位) */
	m_cmemFuncName.SetString( pszFuncName );
	if( pszFileName ){
		m_cmemFileName.SetString( pszFileName );
	}

	m_nInfo = nInfo;
	return;
}

/* CFuncInfoクラス消滅 */
CFuncInfo::~CFuncInfo() = default;

/*!
 * @brief 行番号を取得
 *
 * @param[in] bLineNumIsCRLF true:CRLF単位の行番号を返す false:折り返し単位の行番号を返す
 */
int CFuncInfo::GetLineNumber(
	bool bLineNumIsCRLF
) const
{
	int lineNumber;

	// 改行単位の行番号を表示する場合
	if (bLineNumIsCRLF) {
		lineNumber = static_cast<int>(m_nFuncLineCRLF);
	}
	// 折り返し単位の行番号を表示する場合
	else {
		lineNumber = static_cast<int>(static_cast<Int>(m_nFuncLineLAYOUT));
	}

	return lineNumber;
}

/*!
 * @brief 桁位置を取得
 *
 * 桁位置とは、行頭からの文字数を指す。
 * 折り返し単位の桁位置は、折り返し後の行頭からの文字数を指す。
 *
 * @param[in] bLineNumIsCRLF
 */
int CFuncInfo::GetColumnPosition(
	bool bLineNumIsCRLF
) const
{
	int columnPosition;

	// 改行単位の行番号を表示する場合
	if (bLineNumIsCRLF) {
		columnPosition = static_cast<int>(m_nFuncColCRLF);
	}
	// 折り返し単位の行番号を表示する場合
	else {
		columnPosition = static_cast<int>(static_cast<Int>(m_nFuncColLAYOUT));
	}

	return columnPosition;
}
