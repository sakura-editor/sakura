//	$Id$
/************************************************************************

	CFuncInfoArr.cpp

	アウトライン解析　データ配列
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/23  新規作成

************************************************************************/
#include <stdlib.h>
#include <malloc.h>
#include "CFuncInfoArr.h"


/* CFuncInfoArrクラス構築 */
CFuncInfoArr::CFuncInfoArr()
{
	m_nFuncInfoArrNum = 0;	/* 配列要素数 */
	m_ppcFuncInfoArr = NULL;	/* 配列 */
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



/* 配列の最後にデータを追加する */
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



/* 配列の最後にデータを追加する */
void CFuncInfoArr::AppendData(
	int		nFuncLineCRLF,	/* 関数のある行(CRLF単位) */
	int		nFuncLineLAYOUT,/* 関数のある行(折り返し単位) */
	char*	pszFuncName,	/* 関数名 */
	int		nInfo			/* 付加情報 */
)
{
	CFuncInfo* pcFuncInfo = new CFuncInfo( nFuncLineCRLF, nFuncLineLAYOUT, pszFuncName, nInfo );
	AppendData( pcFuncInfo );
	return;
}


void CFuncInfoArr::DUMP( void )
{
#ifdef _DEBUG
	int i;
	MYTRACE( "=============================\n" );
	for( i = 0; i < m_nFuncInfoArrNum; i++ ){
		MYTRACE( "[%d]------------------\n", i );
		MYTRACE( "m_nFuncLineCRLF	=%d\n", m_ppcFuncInfoArr[i]->m_nFuncLineCRLF );
		MYTRACE( "m_nFuncLineLAYOUT	=%d\n", m_ppcFuncInfoArr[i]->m_nFuncLineLAYOUT );
		MYTRACE( "m_cmemFuncName	=[%s]\n", m_ppcFuncInfoArr[i]->m_cmemFuncName.m_pData );
		MYTRACE( "m_nInfo			=%d\n", m_ppcFuncInfoArr[i]->m_nInfo );
	}
	MYTRACE( "=============================\n" );
#endif
}
