//#include "stdafx.h"
//	Copyright (C) 1998-2000, Norio Nakatani
#include "CJre.h"
#include "debug.h"

///////////////////////////////////////////////////////////////////
// static data
int CJre::m_nUseCount = 0;
HINSTANCE CJre::m_hDll = NULL;
LPJRE2OPEN CJre::m_lpJre2Open = NULL;
LPJRE2COMPILE CJre::m_lpJre2Compile = NULL;
LPJRE2GETMATCHINFO CJre::m_lpJre2MatchInfo = NULL;
LPJRE2CLOSE CJre::m_lpJre2Close = NULL;
LPJREGETVERSION CJre::m_lpJreGetVersion = NULL;
LPGETJREMESSAGE CJre::m_lpGetJreMessage = NULL;

///////////////////////////////////////////////////////////////////
// menber



CJre::CJre()
{
	m_nUseCount++;
	m_bOpen = FALSE;
	m_cmemText.AllocBuffer( 10240 );
}




CJre::~CJre()
{
	if(m_bOpen){
		(*m_lpJre2Close)(&m_jreData);
		m_bOpen = FALSE;
	}

	m_nUseCount--;
	if(m_nUseCount == 0 && m_hDll != NULL){
		FreeLibrary(m_hDll);
		m_hDll = NULL;
	}
}




BOOL CJre::Init()
{
	
	BOOL bRetval;
	bRetval = TRUE;
	while( 1 ){
	
		if(m_nUseCount > 1 && m_hDll != NULL){
			bRetval = TRUE;
			break;
		}

		// DLLのload
		m_hDll = LoadLibrary( "jre32.dll" );
		if(m_hDll == NULL){
			bRetval = FALSE;
			break;
		}
		// APIのポインタ取得
		m_lpJre2Open = (LPJRE2OPEN)GetProcAddress(m_hDll, "Jre2Open" );
		if(m_lpJre2Open == NULL){
			bRetval = FALSE;
			break;
		}
		m_lpJre2Compile = (LPJRE2COMPILE)GetProcAddress(m_hDll, "Jre2Compile" );
		if(m_lpJre2Compile == NULL){
			bRetval = FALSE;
			break;
		}
		m_lpJre2MatchInfo = (LPJRE2GETMATCHINFO)GetProcAddress(m_hDll, "Jre2GetMatchInfo" );
		if(m_lpJre2MatchInfo == NULL){
			bRetval = FALSE;
			break;
		}
		m_lpJre2Close = (LPJRE2CLOSE)GetProcAddress(m_hDll, "Jre2Close" );
		if(m_lpJre2Close == NULL){
			bRetval = FALSE;
			break;
		}
		m_lpJreGetVersion = (LPJREGETVERSION)GetProcAddress(m_hDll, "JreGetVersion" );
		if(m_lpJreGetVersion == NULL){
			bRetval = FALSE;
			break;
		}
		m_lpGetJreMessage = (LPGETJREMESSAGE)GetProcAddress(m_hDll, "GetJreMessage" );
		if(m_lpJreGetVersion == NULL){
			bRetval = FALSE;
			break;
		}
		/* 正常 */
		bRetval = TRUE;
		break;
	}
	if( bRetval ){
		// JRE構造体の初期化
		memset(&m_jreData, 0, sizeof(JRE2));
		m_jreData.dwSize = sizeof(JRE2);

		if(!(*m_lpJre2Open)(&m_jreData)){
			bRetval = FALSE;
		}else{
			m_bOpen = TRUE;
		}
	}
	
	
	return TRUE;
}




BOOL CJre::GetVersion()
{
	MYASSERT( __FILE__, __LINE__, m_hDll != NULL);
	return (*m_lpJreGetVersion)();
}




void CJre::GetJreMessage(int nLanguage, char* pszMsgBuf )
{
	MYASSERT( __FILE__, __LINE__, m_hDll != NULL);

	(*m_lpGetJreMessage)(m_jreData.nError, nLanguage, pszMsgBuf, 512);
	return;
}




BOOL CJre::Compile(LPCTSTR lpszRe)
{
	MYASSERT( __FILE__, __LINE__, m_hDll != NULL);
	MYASSERT( __FILE__, __LINE__, lstrlen(lpszRe) > 0);
	BOOL	bResult;
	char*	pszRe;

	/* 再利用したいとき */
	if(m_bOpen){
		(*m_lpJre2Close)(&m_jreData);
		m_bOpen = FALSE;
		if( !Init() ){
			m_bOpen = FALSE;
			return FALSE;
		}
	}


	if( 0 == lstrlen( lpszRe ) ){
		return FALSE;
	}
	pszRe = new char[lstrlen( lpszRe ) + 1];
	strcpy( pszRe, lpszRe );

	bResult = (*m_lpJre2Compile)( &m_jreData, pszRe );
	delete [] pszRe;
	return bResult;
}




//LPCTSTR CJre::GetMatchInfo(LPCTSTR lpszSt, UINT nStart)
LPCTSTR CJre::GetMatchInfo(LPCTSTR lpszSt, int nStLen, UINT nStart)
{
	MYASSERT( __FILE__, __LINE__, m_hDll != NULL);
//	MYASSERT( __FILE__, __LINE__, lstrlen(lpszSt) > 0);
	MYASSERT( __FILE__, __LINE__, nStLen > 0);

	char*	pszSt;
	if( 0 == nStLen ){
		return NULL;
	}
	
	m_cmemText.SetData( lpszSt, nStLen );
//	pszSt = m_cmemText.GetPtr( NULL );
	pszSt = m_cmemText.m_pData;
//	pszSt = new char[nStLen + 1];
//	memcpy( pszSt, lpszSt, nStLen );
//	pszSt[nStLen] = '\0';


	
	m_jreData.nStart = nStart;
	if( !(*m_lpJre2MatchInfo)( &m_jreData, pszSt ) ){
//		delete [] pszSt;
		return NULL;
	}
//	delete [] pszSt;
	return lpszSt + m_jreData.nPosition;
}



///////////////////////////////////////////////////////////////////
// static function
BOOL CJre::IsExist()
{
	HINSTANCE hDll = LoadLibrary( "jre32.dll" );
	if(hDll == NULL){
		return FALSE;
	}
	FreeLibrary(hDll);
	return TRUE;
}

/*[EOF]*/
