//	$Id$
/***************

 CProfile.cpp
	Copyright (C) 1998-2000, Norio Nakatani

***************/
#include "CProfile.h"
//#include <stdio.h>
#include "debug.h"
#include "global.h"
#include "etc_uty.h"
#include "CKeyWordSetMgr.h"
//#include "CRunningTimer.h"

#ifndef CRLF
	#define CRLF			"\015\012"
#endif

CProfile::CProfile()
{
	int i;
	int j;
	m_szProfileName[0] = '\0';
	m_nSecNum = 0;
	for( i = 0; i < MAX_SECNUM; ++i ){
		m_nSecDataNumArr[i] = 0;
		m_pSecNameArr[i] = NULL;
		for( j = 0; j < MAX_SECDATA; ++j ){
			m_pDataNameArr[i][j] = NULL;
			m_pDataArr[i][j] = NULL;
		}
	}
	return;
}

CProfile::~CProfile()
{
	int i;
	int j;
	m_szProfileName[0] = '\0';
	m_nSecNum = 0;
	for( i = 0; i < MAX_SECNUM; ++i ){
		m_nSecDataNumArr[i] = 0;
		
		if( NULL != m_pSecNameArr[i] ){
			delete m_pSecNameArr[i];
			m_pSecNameArr[i] = NULL;
		}
		for( j = 0; j < MAX_SECDATA; ++j ){
			if( NULL != m_pDataNameArr[i][j] ){
				delete m_pDataNameArr[i][j];
				m_pDataNameArr[i][j] = NULL;
			}
			if( NULL != m_pDataArr[i][j] ){
				delete m_pDataArr[i][j];
				m_pDataArr[i][j] = NULL;
			}
		}
	}
	return;
}

void CProfile::Init( void )
{
	int i;
	m_szProfileName[0] = '\0';
	m_nSecNum = 0;;
	for( i = 0; i < MAX_SECNUM; ++i ){
		m_nSecDataNumArr[i] = 0;
	}
}

BOOL CProfile::ReadProfile( const char* pszProfileName )
{
//	FILE*		pFile;
	const char*	pLine;
	int			nLineLen;
	int			nCurrentSection = -1;
//	int			nCharChars;
	int			i;
//	int			j;
	HFILE		hFile;
	HGLOBAL		hgRead;
	int			nFileLength;
	char*		pBuf;
	int			nBgn;
//	int			nPos;
	CEOL		cEol;
	strcpy( m_szProfileName, pszProfileName );
	hFile = _lopen( pszProfileName, OF_READ );
	if( HFILE_ERROR == hFile ){
		return FALSE;
	}
	/* ファイルサイズの取得 */
	nFileLength = _llseek( hFile, 0, FILE_END );
	_llseek( hFile, 0, FILE_BEGIN );

	hgRead = ::GlobalAlloc( GHND, nFileLength );
	pBuf = (char*)::GlobalLock( hgRead );

	_lread( hFile, pBuf, nFileLength );
	_lclose( hFile );

	nBgn = 0;
//	nLineCount = -1;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
//	while( NULL != ( pLine = GetNextLine( pBuf, nFileLength, &nLineLen, &nBgn, NULL, FALSE ) ) ){
	while( NULL != ( pLine = GetNextLine( pBuf, nFileLength, &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
//			szLine = &pBuf[nBgn];
			if( 2 < nLineLen && 
				0 == memcmp( pLine, "//", 2 )	
			){
				/* コメント行 */	
			}else{
				if( '[' == pLine[0]
				 && 1 < nLineLen
				 && NULL != memchr( &pLine[1], ']', nLineLen - 1 )
				){
					nCurrentSection = AddSection( &pLine[1], nLineLen - 2 );
				}else
				if( -1 != nCurrentSection ){
					for( i = 0; i < nLineLen; ++i ){
						if( '=' == pLine[i] ){
							AddSectionData( nCurrentSection, &pLine[0], i, &pLine[i + 1], nLineLen - i - 1 );
							break;
						}
					}
				}
			}
		}
	}
	::GlobalUnlock( hgRead );
	::GlobalFree( hgRead );
	return TRUE;
}


int CProfile::SearchSection( const char* pszSectionName, int nSectionNameLen )
{
	int			i;
	int			nWork;
	const char*	pszWork;
	for( i = 0; i < m_nSecNum; ++i ){
		pszWork = m_pSecNameArr[i]->GetPtr( &nWork );
		if( nWork == nSectionNameLen &&
			0 == memcmp( pszSectionName, pszWork, nWork ) 
		){
			return i;
		}
	}
	return -1;
}

int CProfile::SearchDataName( int nCurrentSection, const char* pszDataName, int nDataNameLen )
{
	int			i;
	int			nWork;
	const char*	pszWork;
	for( i = 0; i < m_nSecDataNumArr[nCurrentSection]; ++i ){
		pszWork = m_pDataNameArr[nCurrentSection][i]->GetPtr( &nWork );
		if( nWork == nDataNameLen &&
			0 == memcmp( pszDataName, pszWork, nWork ) 
		){
			return i;
		}
	}
	return -1;

}

int CProfile::AddSection( const char* pszSectionName, int nSectionNameLen )
{
//	int			i;
	int			nCurrentSection;
	if( -1 == ( nCurrentSection = SearchSection( pszSectionName, nSectionNameLen ) ) ){
		nCurrentSection = m_nSecNum;
		if( nCurrentSection + 1 > MAX_SECNUM ){
			return -1;
		}
		m_nSecNum++;
		m_pSecNameArr[nCurrentSection] = new CMemory( pszSectionName, nSectionNameLen );
//		MYTRACE( "★[%s]\n", m_pSecNameArr[nCurrentSection]->GetPtr( NULL ) );

		
//		m_pSecNameArr[nCurrentSection]->SetData( pszSectionName, nSectionNameLen );
	}
	return nCurrentSection;
}




BOOL CProfile::AddSectionData( 
	int			nCurrentSection, 
	const char*	pszDataName, 
	int			nDataNameLen,
	const char*	pszData, 
	int			nDataLen
)
{
	int		nCurrentIdx;
	if( 0 > nCurrentSection || 
		m_nSecNum <= nCurrentSection ){
		return FALSE;
	}
	if( -1 == ( nCurrentIdx = SearchDataName( nCurrentSection, pszDataName, nDataNameLen ) ) ){
		nCurrentIdx = m_nSecDataNumArr[nCurrentSection];
		if( nCurrentIdx + 1 > MAX_SECDATA ){
//			MYTRACE( "★★★★nCurrentIdx + 1 > MAX_SECDATA\n   nCurrentIdx=%d, MAX_SECDATA=%d\n", nCurrentIdx, MAX_SECDATA );
			return FALSE;
		}
		m_nSecDataNumArr[nCurrentSection]++;
		m_pDataNameArr[nCurrentSection][nCurrentIdx] = new CMemory( pszDataName, nDataNameLen );
		
		
		m_pDataArr[nCurrentSection][nCurrentIdx] = new CMemory;
//		m_pDataNameArr[nCurrentSection][nCurrentIdx]->SetData( pszDataName, nDataNameLen );
	}
	m_pDataArr[nCurrentSection][nCurrentIdx]->SetData( pszData, nDataLen );
//	MYTRACE( "　□[%s]=[%s]\n", m_pDataNameArr[nCurrentSection][nCurrentIdx]->GetPtr( NULL ), m_pDataArr[nCurrentSection][nCurrentIdx]->GetPtr( NULL ) );
	return TRUE;
}



BOOL CProfile::WriteProfile( const char* pszProfileName, const char* pszComment )
{
	int		i;
	int		j;
//	char	szLine[102400];
	HFILE	hFile;
	char*	pData;
	int		nDataLen;
	if( NULL != pszProfileName ){
		strcpy( m_szProfileName, pszProfileName );
	}

	hFile = _lcreat( m_szProfileName, 0 );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			"ファイルを作成できませんでした。\n\n%s", m_szProfileName
		);
		return FALSE;
	}
	/* コメント書き込み */
	_lwrite( hFile, "//", 2 );
	_lwrite( hFile, pszComment, strlen( pszComment ) );
	_lwrite( hFile, "\r\n", 2 );
	_lwrite( hFile, "\r\n", 2 );


	for( i = 0; i < m_nSecNum; ++i ){
		if( 0 < i ){
			_lwrite( hFile, "\r\n", 2 );
		}
		_lwrite( hFile, "[", 1 );
		pData = m_pSecNameArr[i]->GetPtr( &nDataLen );
		_lwrite( hFile, pData, nDataLen );
		_lwrite( hFile, "]\r\n", 3 );
		for( j = 0; j < m_nSecDataNumArr[i]; ++j ){
			pData = m_pDataNameArr[i][j]->GetPtr( &nDataLen );
			_lwrite( hFile, pData, nDataLen );
			_lwrite( hFile, "=", 1 );
			pData = m_pDataArr[i][j]->GetPtr( &nDataLen );
			_lwrite( hFile, pData, nDataLen );
			_lwrite( hFile, "\r\n", 2 );
		}
	}
	_lwrite( hFile, "\r\n", 2 );
	_lclose( hFile );
	return TRUE;
}






BOOL CProfile::IOProfileData( 
	BOOL		bRead, 
	const char* pszSectionName,
//	int			nSentio 
	const char*	lpValueName,
	int			nRegCnvID,
	char*		lpDataSrc,
	int			cbDataSrc
)
{
//	LONG			lRet;
	DWORD			dwType;
	const char*		pData;		// address of value data 
	DWORD			nDataLen;	// size of value data 
//	char			szValueStr[1024];
	static char		szValueStr[MAX_SETNUM * MAX_KEYWORDNUM * ( MAX_KEYWORDLEN ) + 1];
	int				nWork;
	int*			pnWork;
//	int*			pwWork;
	int				nCurrentSection;
	int				nCurrentIdx;
#ifdef _DEBUG
	if( NULL == lpDataSrc ){
		MYTRACE( "MY_RegVal_IO() NULL == lpDataSrc\n" );	
	}
#endif
//	szValueStr[0] = '\0';
	/* 「読み込み」 か 「書き込み」か */
	if( bRead ){
		/* 読み込み */
//		switch( nRegCnvID ){
//		case REGCNV_INT2SZ:
//			dwType = REG_SZ;
//			break;
//		case REGCNV_SZ2SZ:
//			dwType = REG_SZ;
//			break;
//		case REG_MULTI_SZ:
//			dwType = REG_MULTI_SZ;
//			break;
//		default:
//			dwType = REG_BINARY;
//			break;
//		}
		nDataLen = sizeof( szValueStr ) - 1;
		szValueStr[0] = '\0';

		if( -1 != (nCurrentSection = SearchSection( pszSectionName, strlen( pszSectionName ) ) ) ){
			if( -1 == ( nCurrentIdx = SearchDataName( nCurrentSection, lpValueName, strlen( lpValueName ) ) ) ){
//				::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "作者に教えて欲しいエラー",
//					"データが見つかりません。\n[%s]%s\n", pszSectionName, lpValueName
//				);
				return FALSE;
			}
		}else{
//			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "作者に教えて欲しいエラー",
//				"セクションが見つかりません。\n%s\n", pszSectionName
//			);
			return FALSE;
		}
//		return TRUE;

//		lRet = ::RegQueryValueEx( hKey, lpValueName, NULL, &dwType, (unsigned char *)szValueStr, &nDataLen );
//		if( ERROR_SUCCESS != lRet ){
//			char*	pszMsgBuf;
//			::FormatMessage(
//				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
//				FORMAT_MESSAGE_FROM_SYSTEM | 
//				FORMAT_MESSAGE_IGNORE_INSERTS,
//				NULL,
//				::GetLastError(),
//				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // デフォルト言語
//				(LPTSTR) &pszMsgBuf,
//				0,
//				NULL 
//			);
//			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "作者に教えて欲しいエラー",
//				"レジストリ項目　値の読み込み失敗 lpValueName=[%s]%s\n", lpValueName, pszMsgBuf
//			);
//			MYTRACE( "レジストリ項目　値の読み込み失敗 lpValueName=[%s]%s\n", lpValueName, pszMsgBuf );
//			::LocalFree( pszMsgBuf );
//		}else{
			switch( nRegCnvID ){
			case REGCNV_INT2SZ:
				*((int*)lpDataSrc) = atoi( m_pDataArr[nCurrentSection][nCurrentIdx]->GetPtr( NULL ) );
//				*((int*)lpDataSrc) = atoi( szValueStr );
				break;
			case REGCNV_SZ2SZ:
				strcpy( (char *)lpDataSrc, m_pDataArr[nCurrentSection][nCurrentIdx]->GetPtr( NULL ) );
//				strcpy( (char *)lpDataSrc, szValueStr );
				break;
			case REG_MULTI_SZ:
				memcpy( lpDataSrc, m_pDataArr[nCurrentSection][nCurrentIdx]->GetPtr( NULL ), cbDataSrc );
//				memcpy( lpDataSrc, szValueStr, cbDataSrc );
				break;
			case REGCNV_CHAR2SZ:
				if( 0 == m_pDataArr[nCurrentSection][nCurrentIdx]->GetLength() ){
					lpDataSrc[0] = '\0';
				}else{
					memcpy( lpDataSrc, m_pDataArr[nCurrentSection][nCurrentIdx]->GetPtr( NULL ), 1 );
				}
//				lpDataSrc[1] = '\0';
				break;
			case REGCNV_WORD2SZ:
				*((WORD*)lpDataSrc) = (WORD)atoi( m_pDataArr[nCurrentSection][nCurrentIdx]->GetPtr( NULL ) );
				break;
			default:
				memcpy( lpDataSrc, m_pDataArr[nCurrentSection][nCurrentIdx]->GetPtr( NULL ), cbDataSrc );
//				memcpy( lpDataSrc, szValueStr, cbDataSrc );
				break;
			}
			return TRUE;
//		}
//		return lRet;
	}else{
		/* 書き込み */
		switch( nRegCnvID ){
		case REGCNV_INT2SZ:
			pnWork = (int*)lpDataSrc;
//			sprintf( szValueStr, "%d", *pnWork );
			itoa( *pnWork, szValueStr, 10 );
			dwType = REG_SZ;
			pData = szValueStr;
			nDataLen = strlen( szValueStr );
			break;
		case REGCNV_SZ2SZ:
			dwType = REG_SZ;
			pData = lpDataSrc;
			nDataLen = strlen( (const char*)lpDataSrc );
			break;
		case REG_MULTI_SZ:
			dwType = REG_MULTI_SZ;
			pData = lpDataSrc;
			nDataLen = cbDataSrc;
			break;
		case REGCNV_CHAR2SZ:
			if( '\0' == lpDataSrc[0] ){
				pData = "";
				nDataLen = 0;
			}else{
				pData = lpDataSrc;
				nDataLen = 1;
			}
			break;
//			memcpy( lpDataSrc, m_pDataArr[nCurrentSection][nCurrentIdx]->GetPtr( NULL ), 1 );
//			break;
		case REGCNV_WORD2SZ:
			nWork = (int)(*((WORD*)lpDataSrc));
//			sprintf( szValueStr, "%d", *pnWork );
			itoa( nWork, szValueStr, 10 );
			dwType = REG_SZ;
			pData = szValueStr;
			nDataLen = strlen( szValueStr );
			break;
			break;

		default:
			dwType = REG_BINARY;
			pData = lpDataSrc;
			nDataLen = cbDataSrc;
			break;
		}

		if( -1 != (nCurrentSection = AddSection( pszSectionName, strlen( pszSectionName ) ) ) ){
			if( -1 == AddSectionData( nCurrentSection, lpValueName, strlen( lpValueName ), pData, nDataLen ) ){
				::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "作者に教えて欲しいエラー",
					"データが追加出来ません。\n[%s]%s\n", pszSectionName, lpValueName
				);
				return FALSE;
			}
		}else{
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "作者に教えて欲しいエラー",
				"セクションが追加出来ません。\n%s\n", pszSectionName
			);
			return FALSE;
		}
		return TRUE;


//		lRet = ::RegSetValueEx( hKey, lpValueName, 0, dwType, pData, nDataLen );
//		if( ERROR_SUCCESS != lRet ){
//			char*	pszMsgBuf;
//			::FormatMessage(
//				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
//				FORMAT_MESSAGE_FROM_SYSTEM | 
//				FORMAT_MESSAGE_IGNORE_INSERTS,
//				NULL,
//				::GetLastError(),
//				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // デフォルト言語
//				(LPTSTR) &pszMsgBuf,
//				0,
//				NULL 
//			);
//			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONINFORMATION | MB_TOPMOST, "作者に教えて欲しいエラー",
//				"レジストリ項目　値の書き込み()失敗 lpValueName=[%s]%s\n", lpValueName, pszMsgBuf
//			);
//			MYTRACE( "レジストリ項目　値の書き込み()失敗 lpValueName=[%s]%s\n", lpValueName, pszMsgBuf );
//			::LocalFree( pszMsgBuf );
//		}
//		return lRet;
	}

} 






void CProfile::DUMP( void )
{
#ifdef _DEBUG
	int	i;
	int	j;
	MYTRACE( "\n\n\n========================================-\n" );
	MYTRACE( "m_nSecNum=%d\n", m_nSecNum );
	MYTRACE( "m_szProfileName=%s\n", m_szProfileName );
	MYTRACE( "========================================-\n" );
	for( i = 0; i < m_nSecNum; ++i ){
		MYTRACE( "\n■m_pSecNameArr[%d]=%s\n", i, m_pSecNameArr[i]->GetPtr( NULL ) );
		MYTRACE( "m_nSecDataNumArr[%d]=%d\n", i, m_nSecDataNumArr[i] );
		for( j = 0; j < m_nSecDataNumArr[i]; ++j ){
			MYTRACE( "・\tm_pDataNameArr[%d][%d]=%s\n", i, j, m_pDataNameArr[i][j]->GetPtr( NULL ) );
		}
	}
#endif
	return;
}


/* [EOF] */

