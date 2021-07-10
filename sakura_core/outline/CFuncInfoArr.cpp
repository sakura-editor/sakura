/*!	@file
	@brief アウトライン解析 データ配列

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include "outline/CFuncInfoArr.h"
#include "outline/CFuncInfo.h"

void CFuncInfoArr::Empty(void)
{
	m_cFuncInfoArr.clear();
	m_AppendTextArr.clear();
	m_nAppendTextLenMax = 0;
	return;
}

/*! 配列の最後にデータを追加する */
void CFuncInfoArr::AppendData( CFuncInfo&& cFuncInfo )
{
	m_cFuncInfoArr.emplace_back(cFuncInfo);
}

/*! 配列の最後にデータを追加する

	@date 2002.04.01 YAZAKI 深さ導入
*/
void CFuncInfoArr::AppendData(
	CLogicInt		nFuncLineCRLF,		//!< 関数のある行(CRLF単位)
	CLogicInt		nFuncColCRLF,		//!< 関数のある桁(CRLF単位)
	CLayoutInt		nFuncLineLAYOUT,	//!< 関数のある行(折り返し単位)
	CLayoutInt		nFuncColLAYOUT,		//!< 関数のある桁(折り返し単位)
	const WCHAR*	pszFuncName,		//!< 関数名
	const WCHAR*	pszFileName,		//!< ファイル名
	int				nInfo,				//!< 付加情報
	int				nDepth				//!< 深さ
)
{
	AppendData(CFuncInfo(nFuncLineCRLF, nFuncColCRLF, nFuncLineLAYOUT, nFuncColLAYOUT,
		pszFuncName, pszFileName, nInfo, nDepth));
	return;
}

void CFuncInfoArr::AppendData(
	CLogicInt		nFuncLineCRLF,		//!< 関数のある行(CRLF単位)
	CLayoutInt		nFuncLineLAYOUT,	//!< 関数のある行(折り返し単位)
	const WCHAR*	pszFuncName,		//!< 関数名
	int				nInfo,				//!< 付加情報
	int				nDepth				//!< 深さ
)
{
	AppendData(nFuncLineCRLF,CLogicInt(1),nFuncLineLAYOUT,CLayoutInt(1),pszFuncName,NULL,nInfo,nDepth);
	return;
}

void CFuncInfoArr::DUMP( void )
{
#ifdef _DEBUG
	int i;
	MYTRACE( L"=============================\n" );
	for( i = 0; i < m_cFuncInfoArr.size(); i++ ){
		MYTRACE( L"[%d]------------------\n", i );
		MYTRACE( L"m_nFuncLineCRLF	=%d\n", m_cFuncInfoArr[i].m_nFuncLineCRLF );
		MYTRACE( L"m_nFuncLineLAYOUT	=%d\n", m_cFuncInfoArr[i].m_nFuncLineLAYOUT );
		MYTRACE( L"m_strFuncName	=[%s]\n", m_cFuncInfoArr[i].m_strFuncName.c_str() );
		MYTRACE( L"m_strFileName	=[%s]\n", m_cFuncInfoArr[i].m_strFileName.c_str() );
		MYTRACE( L"m_nInfo			=%d\n", m_cFuncInfoArr[i].m_nInfo );
	}
	MYTRACE( L"=============================\n" );
#endif
}

void CFuncInfoArr::SetAppendText( int info, std::wstring s, bool overwrite )
{
	if( m_AppendTextArr.find( info ) == m_AppendTextArr.end() ){
		// キーが存在しない場合、追加する
		std::pair<int, std::wstring> pair(info, s);
		m_AppendTextArr.insert( pair );
		if( m_nAppendTextLenMax < (int)s.length() ){
			m_nAppendTextLenMax = s.length();
		}
	}else{
		// キーが存在する場合、値を書き換える
		if( overwrite ){
			m_AppendTextArr[ info ] = s;
		}
	}
}

std::wstring CFuncInfoArr::GetAppendText( int info )
{
	// キーが存在する場合、値を返す
	if( m_AppendTextArr.find( info ) != m_AppendTextArr.end() ){
		return m_AppendTextArr[info];
	}
	// キーが存在しない場合、空文字列を返す
	return {};
}
