/*!	@file
	@brief アウトライン解析 データ配列

	@author Norio Nakatani
	@date	1998/06/23 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdlib.h>
#include "outline/CFuncInfoArr.h"
#include "outline/CFuncInfo.h"


/* CFuncInfoArrクラス構築 */
CFuncInfoArr::CFuncInfoArr()
{
	m_nFuncInfoArrNum = 0;	/* 配列要素数 */
	m_ppcFuncInfoArr = NULL;	/* 配列 */
	m_nAppendTextLenMax = 0;
	return;
}




/* CFuncInfoArrクラス消滅 */
CFuncInfoArr::~CFuncInfoArr()
{
	Empty();
	return;
}

void CFuncInfoArr::Empty( void )
{
	int i;
	if( m_nFuncInfoArrNum > 0 && NULL != m_ppcFuncInfoArr ){
		for( i = 0; i < m_nFuncInfoArrNum; ++i ){
			delete m_ppcFuncInfoArr[i];
			m_ppcFuncInfoArr[i] = NULL;
		}
		m_nFuncInfoArrNum = 0;
		free( m_ppcFuncInfoArr );
		m_ppcFuncInfoArr = NULL;
	}
	m_AppendTextArr.clear();
	m_nAppendTextLenMax = 0;
	return;
}


/* 0<=の指定番号のデータを返す */
/* データがない場合はNULLを返す */
CFuncInfo* CFuncInfoArr::GetAt( int nIdx )
{
	if( nIdx >= m_nFuncInfoArrNum ){
		return NULL;
	}
	return m_ppcFuncInfoArr[nIdx];
}



/*! 配列の最後にデータを追加する */
void CFuncInfoArr::AppendData( CFuncInfo* pcFuncInfo )
{
	if( 0 == m_nFuncInfoArrNum){
		m_ppcFuncInfoArr = (CFuncInfo**)malloc( sizeof(CFuncInfo*) * (m_nFuncInfoArrNum + 1) );
	}else{
		m_ppcFuncInfoArr = (CFuncInfo**)realloc( m_ppcFuncInfoArr, sizeof(CFuncInfo*) * (m_nFuncInfoArrNum + 1) );
	}
	m_ppcFuncInfoArr[m_nFuncInfoArrNum] = pcFuncInfo;
	m_nFuncInfoArrNum++;
	return;
}



/*! 配列の最後にデータを追加する

	@date 2002.04.01 YAZAKI 深さ導入
*/
void CFuncInfoArr::AppendData(
	CLogicInt		nFuncLineCRLF,		//!< 関数のある行(CRLF単位)
	CLogicInt		nFuncColCRLF,		//!< 関数のある桁(CRLF単位)
	CLayoutInt		nFuncLineLAYOUT,	//!< 関数のある行(折り返し単位)
	CLayoutInt		nFuncColLAYOUT,		//!< 関数のある桁(折り返し単位)
	const TCHAR*	pszFuncName,		//!< 関数名
	const TCHAR*	pszFileName,		//!< ファイル名
	int				nInfo,				//!< 付加情報
	int				nDepth				//!< 深さ
)
{
	CFuncInfo* pcFuncInfo = new CFuncInfo( nFuncLineCRLF, nFuncColCRLF, nFuncLineLAYOUT, nFuncColLAYOUT,
		pszFuncName, pszFileName, nInfo );
	pcFuncInfo->m_nDepth = nDepth;
	AppendData( pcFuncInfo );
	return;
}

void CFuncInfoArr::AppendData(
	CLogicInt			nFuncLineCRLF,		//!< 関数のある行(CRLF単位)
	CLogicInt			nFuncColCRLF,		//!< 関数のある桁(CRLF単位)
	CLayoutInt			nFuncLineLAYOUT,	//!< 関数のある行(折り返し単位)
	CLayoutInt			nFuncColLAYOUT,		//!< 関数のある桁(折り返し単位)
	const NOT_TCHAR*	pszFuncName,		//!< 関数名
	const NOT_TCHAR*	pszFileName,		//!< ファイル名
	int					nInfo,				//!< 付加情報
	int					nDepth				//!< 深さ
)
{
	AppendData(nFuncLineCRLF,nFuncColCRLF,nFuncLineLAYOUT,nFuncColLAYOUT,to_tchar(pszFuncName),
		(pszFileName?to_tchar(pszFileName):NULL),nInfo,nDepth);
}

void CFuncInfoArr::AppendData(
	CLogicInt		nFuncLineCRLF,		//!< 関数のある行(CRLF単位)
	CLayoutInt		nFuncLineLAYOUT,	//!< 関数のある行(折り返し単位)
	const TCHAR*	pszFuncName,		//!< 関数名
	int				nInfo,				//!< 付加情報
	int				nDepth				//!< 深さ
)
{
	AppendData(nFuncLineCRLF,CLogicInt(1),nFuncLineLAYOUT,CLayoutInt(1),pszFuncName,NULL,nInfo,nDepth);
	return;
}

void CFuncInfoArr::AppendData(
	CLogicInt			nFuncLineCRLF,		//!< 関数のある行(CRLF単位)
	CLayoutInt			nFuncLineLAYOUT,	//!< 関数のある行(折り返し単位)
	const NOT_TCHAR*	pszFuncName,		//!< 関数名
	int					nInfo,				//!< 付加情報
	int					nDepth				//!< 深さ
)
{
	AppendData(nFuncLineCRLF,nFuncLineLAYOUT,to_tchar(pszFuncName),nInfo,nDepth);
}


void CFuncInfoArr::DUMP( void )
{
#ifdef _DEBUG
	int i;
	MYTRACE( _T("=============================\n") );
	for( i = 0; i < m_nFuncInfoArrNum; i++ ){
		MYTRACE( _T("[%d]------------------\n"), i );
		MYTRACE( _T("m_nFuncLineCRLF	=%d\n"), m_ppcFuncInfoArr[i]->m_nFuncLineCRLF );
		MYTRACE( _T("m_nFuncLineLAYOUT	=%d\n"), m_ppcFuncInfoArr[i]->m_nFuncLineLAYOUT );
		MYTRACE( _T("m_cmemFuncName	=[%ts]\n"), m_ppcFuncInfoArr[i]->m_cmemFuncName.GetStringPtr() );
		MYTRACE( _T("m_cmemFileName	=[%ts]\n"),
			(m_ppcFuncInfoArr[i]->m_cmemFileName.GetStringPtr() ? m_ppcFuncInfoArr[i]->m_cmemFileName.GetStringPtr() : _T("NULL")) );
		MYTRACE( _T("m_nInfo			=%d\n"), m_ppcFuncInfoArr[i]->m_nInfo );
	}
	MYTRACE( _T("=============================\n") );
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
#ifndef	_UNICODE
		std::tstring t = to_tchar(s.c_str());
		if( m_nAppendTextLenMax < (int)t.length() ){
			m_nAppendTextLenMax = t.length();
		}
#endif
	}else{
		// キーが存在する場合、値を書き換える
		if( overwrite ){
			m_AppendTextArr[ info ] = s;
		}
	}
}

std::wstring CFuncInfoArr::GetAppendText( int info )
{
	if( m_AppendTextArr.find( info ) == m_AppendTextArr.end() ){
		// キーが存在しない場合、空文字列を返す
		return std::wstring();
	}else{
		// キーが存在する場合、値を返す
		return m_AppendTextArr[ info ];
	}
}

