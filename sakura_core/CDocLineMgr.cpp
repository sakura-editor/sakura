//	$Id$
/*!	@file

	テキストの管理

	Copyright (C) 1998-2000, Norio Nakatani

	@author Norio Nakatani
	@date 1998/3/5  新規作成
*/

/* for TRACE() of MFC */
//#ifdef _DEBUG
//	#include <afx.h>
//#endif
//#ifndef _DEBUG
//	#include <windows.h>
//#endif


#include "charcode.h"
#include "CDocLineMgr.h"
#include "debug.h"
#include "charcode.h"
// Oct 6, 2000 ao
#include <stdio.h>
#include <io.h>
//#include <string.h>
//#include <memory.h>
#include "CJre.h"
#include <commctrl.h>
#include "global.h"
#include "etc_uty.h"
#include "CRunningTimer.h"

//	May 15, 2000 genta
#include "CEol.h"



/* 文字種類識別子 */
#define	CK_NULL			0	/*!< NULL 0x0<=c<=0x0 */
#define	CK_TAB			1	/*!< タブ 0x9<=c<=0x9 */
#define	CK_CR			2	/*!< CR = 0x0d  */
#define	CK_LF			3	/*!< LF = 0x0a  */

#define	CK_SPACE		4	/*!< 半角のスペース 0x20<=c<=0x20 */
#define	CK_CSYM			5	/*!< 半角の英字、アンダースコア、数字のいずれか */
#define	CK_KATA			6	/*!< 半角のカタカナ 0xA1<=c<=0xFD */
#define	CK_ETC			7	/*!< 半角のその他 */

#define	CK_MBC_SPACE	12	/*!< 2バイトのスペース */
							/*!< 0x8140<=c<=0x8140 全角スペース */
#define	CK_MBC_NOVASU	13	/*!< 伸ばす記号 0x815B<=c<=0x815B 'ー' */
#define	CK_MBC_CSYM		14	/*!< 2バイトの英字、アンダースコア、数字のいずれか */
							/*!< 0x8151<=c<=0x8151 全角アンダースコア */
							/*!< 0x824F<=c<=0x8258 全角数字 */
							/*!< 0x8260<=c<=0x8279 全角英字大文字 */
							/*!< 0x8281<=c<=0x829a 全角英字小文字 */
#define	CK_MBC_KIGO		15	/*!< 2バイトの記号 */
							/*!< 0x8141<=c<=0x81FD */
#define	CK_MBC_HIRA		16	/*!< 2バイトのひらがな */
							/*!< 0x829F<=c<=0x82F1 全角ひらがな */
#define	CK_MBC_KATA		17	/*!< 2バイトのカタカナ */
							/*!< 0x8340<=c<=0x8396 全角カタカナ */
#define	CK_MBC_GIRI		18	/*!< 2バイトのギリシャ文字 */
							/*!< 0x839F<=c<=0x83D6 全角ギリシャ文字 */
#define	CK_MBC_ROS		19	/*!< 2バイトのロシア文字: */
							/*!< 0x8440<=c<=0x8460 全角ロシア文字大文字 */
							/*!< 0x8470<=c<=0x8491 全角ロシア文字小文字 */
#define	CK_MBC_SKIGO	20	/*!< 2バイトの特殊記号 */
							/*!< 0x849F<=c<=0x879C 全角特殊記号 */
#define	CK_MBC_ETC		21	/*!< 2バイトのその他（漢字など） */





CDocLineMgr::CDocLineMgr()
{
	Init();
}




CDocLineMgr::~CDocLineMgr()
{
	Empty();
}





void CDocLineMgr::Init()
{
	m_pDocLineTop = NULL;
	m_pDocLineBot = NULL;
	m_nLines = 0;
	m_nPrevReferLine = 0;
	m_pCodePrevRefer = NULL;
	return;
}

/*!
	データのクリア
	
	全ての行を削除する
*/
void CDocLineMgr::Empty()
{
	CDocLine* pDocLine;
	CDocLine* pDocLineNext;
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLineNext = pDocLine->m_pNext;
		delete pDocLine;
		pDocLine = pDocLineNext;
	}
	return;
}





const char* CDocLineMgr::GetLineStr( int nLine, int* pnLineLen )
{
	CDocLine* pDocLine;
	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		*pnLineLen = 0;
		return NULL;
	}
//	return pDocLine->m_pLine->GetPtr( pnLineLen );
	*pnLineLen = pDocLine->m_pLine->m_nDataLen;
	return pDocLine->m_pLine->m_pData;
}

/*!
	指定された番号の行へのポインタを返す
	
	@param nLine [in] 行番号
	@return 行オブジェクトへのポインタ。該当行がない場合はNULL。
*/
CDocLine* CDocLineMgr::GetLineInfo( int nLine )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::GetLineInfo()" );
//#endif

	int nCounter;
	CDocLine* pDocLine;
	if( 0 == m_nLines ){
		return NULL;
	}
	if( nLine >= m_nLines ){
		return NULL;
	}
	if( m_pCodePrevRefer == NULL ){
#ifdef _DEBUG
		CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::GetLineInfo() 	m_pCodePrevRefer == NULL" );
#endif




#if 0 /////////	1999.12.22
		nCounter = 0;
		pDocLine = m_pDocLineTop;
		do{
			if( nLine == nCounter ){
				m_nPrevReferLine = nLine;
				m_pCodePrevRefer = pDocLine;
				m_pDocLineCurrent = pDocLine->m_pNext;
				return pDocLine;
			}
			pDocLine = pDocLine->m_pNext;
			++nCounter;
		}while( NULL != pDocLine );
#endif ///////////////
		if( nLine < (m_nLines / 2) ){
			nCounter = 0;
			pDocLine = m_pDocLineTop;
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pNext;
				nCounter++;
			}
		}else{
			nCounter = m_nLines - 1;
			pDocLine = m_pDocLineBot;
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pPrev;
				nCounter--;
			}
		}

	}else{
		if( nLine == m_nPrevReferLine ){
			m_nPrevReferLine = nLine;
			m_pDocLineCurrent = m_pCodePrevRefer->m_pNext;
			return m_pCodePrevRefer;
		}else
		if( nLine > m_nPrevReferLine ){
			nCounter = m_nPrevReferLine + 1;
			pDocLine = m_pCodePrevRefer->m_pNext;
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pNext;
				++nCounter;
			}
		}else{
			nCounter = m_nPrevReferLine - 1;
			pDocLine = m_pCodePrevRefer->m_pPrev;
			while( NULL != pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pPrev;
				nCounter--;
			}
		}
	}
	return NULL;
}





/*! 順アクセスモード：先頭行を得る

	@param pnLineLen [out] 行の長さが返る。
	@return 1行目の先頭へのポインタ。
	データが1行もないときは、長さ0、ポインタNULLが返る。

*/
const char* CDocLineMgr::GetFirstLinrStr( int* pnLineLen )
{
	char* pszLine;
	if( 0 == m_nLines ){
		pszLine = NULL;
		*pnLineLen = 0;
	}else{
		pszLine = m_pDocLineTop->m_pLine->GetPtr( pnLineLen );

		m_pDocLineCurrent = m_pDocLineTop->m_pNext;
	}
	return (const char*)pszLine;
}





/*! 
	順アクセスモード：次の行を得る

	@param pnLineLen [out] 行の長さが返る。
	@return 次行の先頭へのポインタ。
	GetFirstLinrStr()が呼び出されていないとNULLが返る

*/
const char* CDocLineMgr::GetNextLinrStr( int* pnLineLen )
{
	char* pszLine;
	if( NULL == m_pDocLineCurrent ){
		pszLine = NULL;
		*pnLineLen = 0;
	}else{
		pszLine = m_pDocLineCurrent->m_pLine->GetPtr( pnLineLen );

		m_pDocLineCurrent = m_pDocLineCurrent->m_pNext;
	}
	return (const char*)pszLine;
}




#if 0
	/* 末尾に行を追加 Ver0 */
	void CDocLineMgr::AddLineStrSz( const char* pszStr )
	{
	#ifdef _DEBUG
		CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::AddLineStrSz" );
	#endif
		CDocLine* pDocLine;
		if( 0 == m_nLines ){
			m_pDocLineBot = m_pDocLineTop = new CDocLine;
			m_pDocLineTop->m_pPrev = NULL;
			m_pDocLineTop->m_pNext = NULL;
			m_pDocLineTop->m_pLine = new CMemory( pszStr, lstrlen( pszStr ) );
		}else{
			pDocLine = new CDocLine;
			pDocLine->m_pPrev = m_pDocLineBot;
			pDocLine->m_pNext = NULL;
			pDocLine->m_pLine = new CMemory( pszStr, lstrlen( pszStr ) );
			m_pDocLineBot->m_pNext = pDocLine;
			m_pDocLineBot = pDocLine;
		}
		++m_nLines;
	}
	/* 末尾に行を追加 Ver1 */
	void CDocLineMgr::AddLineStr( const char* pData, int nDataLen )
	{
	#ifdef _DEBUG
		CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::AddLineStr(const char*, int)" );
	#endif
		CDocLine* pDocLine;
		if( 0 == m_nLines ){
			m_pDocLineBot = m_pDocLineTop = new CDocLine;
			m_pDocLineTop->m_pPrev = NULL;
			m_pDocLineTop->m_pNext = NULL;
			m_pDocLineTop->m_pLine = new CMemory( pData, nDataLen );
		}else{
			pDocLine = new CDocLine;
			pDocLine->m_pPrev = m_pDocLineBot;
			pDocLine->m_pNext = NULL;
			pDocLine->m_pLine = new CMemory( pData, nDataLen );
			m_pDocLineBot->m_pNext = pDocLine;
			m_pDocLineBot = pDocLine;
		}
		++m_nLines;
	}
	/* 末尾に行を追加 Ver2 */
	void CDocLineMgr::AddLineStr( CMemory& cmemData )
	{
	//#ifdef _DEBUG
	//	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::AddLineStr(CMemory&)" );
	//#endif
		char*	pData;
		int		nDataLen;
		pData = cmemData.GetPtr( &nDataLen );
	//	AddLineStr( pData, nDataLen );

		CDocLine* pDocLine;
		if( 0 == m_nLines ){
			m_pDocLineBot = m_pDocLineTop = new CDocLine;
			m_pDocLineTop->m_pPrev = NULL;
			m_pDocLineTop->m_pNext = NULL;
			m_pDocLineTop->m_pLine = new CMemory( pData, nDataLen );
		}else{
			pDocLine = new CDocLine;
			pDocLine->m_pPrev = m_pDocLineBot;
			pDocLine->m_pNext = NULL;
			pDocLine->m_pLine = new CMemory( pData, nDataLen );
			m_pDocLineBot->m_pNext = pDocLine;
			m_pDocLineBot = pDocLine;
		}
		++m_nLines;
		return;
	}
#endif

/*!
	末尾に行を追加

	@version 1.5
	
	@param pData [in] 追加する文字列へのポインタ
	@param nDataLen [in] 文字列の長さ
	@param cEol [in] 行末コード

*/
void CDocLineMgr::AddLineStrX( const char* pData, int nDataLen, CEOL cEol )
{
#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::AddLineStrX(const char*, int, int)" );
#endif
	CDocLine* pDocLine;
	if( 0 == m_nLines ){
		m_pDocLineBot = m_pDocLineTop = new CDocLine;
		m_pDocLineTop->m_pPrev = NULL;
		m_pDocLineTop->m_pNext = NULL;
//		if( bCRLF ){

//		if( EOL_NONE != nEOLType ){
//			m_pDocLineTop->m_pLine->m_pData[nDataLen] = '\r';
//			m_pDocLineTop->m_pLine->m_pData[nDataLen + 1] = '\0';
//			++m_pDocLineTop->m_pLine->m_nDataLen;
//		}
		m_pDocLineTop->m_cEol = cEol;	/* 改行コードの種類 */
		m_pDocLineTop->m_pLine = new CMemory( pData, nDataLen/* - m_pDocLineTop->m_nEOLLen*/ );
	}else{
		pDocLine = new CDocLine;
		pDocLine->m_pPrev = m_pDocLineBot;
		pDocLine->m_pNext = NULL;
//		if( bCRLF ){

//		if( EOL_NONE != nEOLType ){
//			pDocLine->m_pLine->m_pData[nDataLen] = '\r';
//			pDocLine->m_pLine->m_pData[nDataLen + 1] = '\0';
//			++pDocLine->m_pLine->m_nDataLen;
//		}
		pDocLine->m_cEol = cEol;	/* 改行コードの種類 */
		pDocLine->m_pLine = new CMemory( pData, nDataLen/* - pDocLine->m_nEOLLen*/ );
		m_pDocLineBot->m_pNext = pDocLine;
		m_pDocLineBot = pDocLine;
	}
	++m_nLines;
	return;
}




/* ファイルを読み込んで格納する（テスト用） */
/* （注意）Windows用にコーディングしてある */
//	nFlags:
//		bit 0: MIME Encodeされたヘッダをdecodeするかどうか
int CDocLineMgr::ReadFile( const char* pszPath, HWND hWndParent, HWND hwndProgress, int nCharCode, FILETIME* pFileTime, int nFlags )
{
#ifdef _DEBUG
	MYTRACE( "pszPath=[%s]\n", pszPath );
	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::ReadFile" );
#endif
	int			nRetVal;
	int			nEOF;
	BOOL		bBinaryAlerted;
	int			nFileLength;
	int			nReadLength;
//	char*		pSJISBuf;
//	int			nSJISBufLen;
	CMemory		cmemBuf;
	/*
	|| バッファサイズの調整
	*/
	cmemBuf.AllocBuffer( 32000 );

	nRetVal = TRUE;
	nEOF = FALSE;
	bBinaryAlerted = FALSE;
	HGLOBAL	hgRead;

	/* 既存データのクリア */
	Empty();
	Init();
	CMemory		cMem;
//	UINT		nReadBufSize = 16/*32000*/;
	char*		pBuf;
//	char*		pBuf = new char[nReadBufSize + 1];
	int			nReadSize;
	int			nBgn;
//	int			nPos;
	HFILE		hFile;
//	const char*	pLine;
//	int			nLineLen;
	BOOL		bLFisOK;
//	char*		pszCRLF_UNICODE = "\x0d\x0\x0a\x0";
//	int			nCRLF_UNICODE_LEN;
//	char*		pszCRLF = "\x0d\x0a";
//	int			nCRLF_LEN;
//	char*		pszLF = "\x0a";
//	int			nLF_LEN;
//	char*		pszCR = "\x0d";
//	int			nCR_LEN;
//	char*		pszLFCR = "\x0a\x0d";
//	int			nLFCR_LEN;
//	int			nEolCodeLen;
//	enumEOLType	nEolType;
//	BOOL		bEOL;
	int			nLineNum;
	//	May 15, 2000 genta
	CEOL cEol;

//	nCRLF_UNICODE_LEN = 4;
//	nCRLF_LEN = 2;
//	nLFCR_LEN = 2;
//	nLF_LEN = 1;
//	nCR_LEN = 1;
//	nEolType = EOL_UNKNOWN;

//	pBuf[nReadBufSize] = '\0';
//	cMem.SetData( "", lstrlen( "" ) );
	cMem.SetDataSz( "" );

	hFile = _lopen( pszPath, OF_READ );
	if( HFILE_ERROR == hFile ){
//		MYTRACE( "file open error %s\n", pszPath );
		if( (_access( pszPath, 0 )) == -1 ){
			::MYMESSAGEBOX(
				hWndParent,
				MB_OK | MB_ICONSTOP,
				GSTR_APPNAME,
//				"\'%s\'\nファイルを開けません。\nファイルが存在しません。",
				"%s\nというファイルを開けません。\nファイルが存在しません。",	//Mar. 24, 2001 jepro 若干修正
				pszPath
			 );
		}else{
			::MYMESSAGEBOX(
				hWndParent,
				MB_OK | MB_ICONSTOP,
				GSTR_APPNAME,
				"\'%s\'\nというファイルを開けません。\n他のアプリケーションで使用されている可能性があります。",
				pszPath
			 );
		}
		nRetVal = FALSE;
		goto _RETURN_;
	}

	/* ファイル時刻の取得 */
	FILETIME	FileTime;
	SYSTEMTIME	systimeL;
	if( ::GetFileTime( (HANDLE)hFile, NULL, NULL, &FileTime ) ){
		*pFileTime = FileTime;

		::FileTimeToLocalFileTime( &FileTime, &FileTime );
		::FileTimeToSystemTime( &FileTime, &systimeL );
//		MYTRACE( "Last Update: %d/%d/%d %02d:%02d:%02d\n",
//			systimeL.wYear,
//			systimeL.wMonth,
//			systimeL.wDay,
//			systimeL.wHour,
//			systimeL.wMinute,
//			systimeL.wSecond
//		);
	}else{
//		MYTRACE( "GetFileTime() error.\n" );
	}


	/* ファイルサイズの取得 */
	nFileLength = _llseek( hFile, 0, FILE_END );
	_llseek( hFile, 0, FILE_BEGIN );
	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
	}
	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( NULL ) ){
		return -1;
	}

	hgRead = ::GlobalAlloc( GHND, nFileLength );
	if( NULL == hgRead ){
		::MYMESSAGEBOX(
			hWndParent,
			MB_OK | MB_ICONSTOP,
			GSTR_APPNAME,
			"CDocLineMgr::ReadFile()\nメモリ確保に失敗しました。\n%dバイト",
			nFileLength
		);
		nRetVal = FALSE;
		goto _RETURN_;
	}
	pBuf = (char*)::GlobalLock( hgRead );
	nEOF = TRUE;

	switch( nCharCode ){
	case CODE_UNICODE:
		nReadSize = _lread( hFile, pBuf, 2 );
		if( HFILE_ERROR == nReadSize ){
//			MYTRACE( "file read error %s\n", pszPath );
			nRetVal = FALSE;
			goto _CLOSEFILE_;
		}
		break;
	}

	nReadLength = 0;
	nReadSize = _lread( hFile, pBuf, nFileLength/*nReadBufSize*/ );
	if( HFILE_ERROR == nReadSize ){
//		MYTRACE( "file read error %s\n", pszPath );
		nRetVal = FALSE;
		goto _CLOSEFILE_;
	}
//	nReadLength += nReadSize;

	bLFisOK = FALSE;
	switch( nCharCode ){
	case CODE_EUC:
		cmemBuf.SetData( pBuf, nReadSize );
		/* EUC→SJISコード変換 */
		cmemBuf.EUCToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		bLFisOK = TRUE;
		break;
	case CODE_JIS:
		cmemBuf.SetData( pBuf, nReadSize );
		/* E-Mail(JIS→SJIS)コード変換 */
		cmemBuf.JIStoSJIS( (nFlags & 1) == 1 );	//	Nov. 12, 2000 genta フラグ追加
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_UNICODE:
		cmemBuf.SetData( pBuf, nReadSize );
		/* Unicode→SJISコード変換 */
		cmemBuf.UnicodeToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_UTF8:	/* UTF-8 */
		cmemBuf.SetData( pBuf, nReadSize );
		/* UTF-8→SJISコード変換 */
		cmemBuf.UTF8ToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_UTF7:	/* UTF-7 */
		cmemBuf.SetData( pBuf, nReadSize );
		/* UTF-7→SJISコード変換 */
		cmemBuf.UTF7ToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_SJIS:
	default:
		break;
	}
	nFileLength = nReadSize;


	if( NULL != hwndProgress && 0 < nFileLength ){
		::PostMessage( hwndProgress, PBM_SETPOS, nReadLength * 100 / nFileLength , 0 );
	}
	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( NULL ) ){
		return -1;
	}



	nLineNum = 0;
//	bEOL = FALSE;



	const char*	pLine;
	int			nLineLen;
	// enumEOLType nEOLType;
	int			nEolCodeLen;
	nBgn = 0;
//	nPos = 0;
	while( NULL != ( pLine = GetNextLine( pBuf, nFileLength, &nLineLen, &nBgn, &cEol ) ) ){
		nEolCodeLen = cEol.GetLen();
		++nLineNum;
//		AddLineStrX( &pBuf[nBgn], nPos - nBgn + nEolCodeLen, nEolType );
		AddLineStrX( pLine, nLineLen + nEolCodeLen, cEol );
//		nReadLength += nPos - nBgn + nEolCodeLen;
		nReadLength += nLineLen + nEolCodeLen;
		if( NULL != hwndProgress && 0 < nFileLength && 0 == ( nLineNum % 1024 ) ){
			::PostMessage( hwndProgress, PBM_SETPOS, nReadLength * 100 / nFileLength , 0 );
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( NULL ) ){
				return -1;
			}
		}
//		nBgn = nPos + nEolCodeLen;
//		nPos = nBgn;
	}


//	do{
//		while( nBgn < nReadSize && nPos	< nReadSize ){
//			/* 改行コードがあった */
//			if( pBuf[nPos] == '\n' || pBuf[nPos] == '\r' ){
//				/* 改行コードの長さを調べる */
//				if( nPos <= nReadSize - nCRLF_UNICODE_LEN &&
//					0 == memcmp( &pBuf[nPos], pszCRLF_UNICODE, nCRLF_UNICODE_LEN )
//				){
//					nEolCodeLen = nCRLF_UNICODE_LEN;
//					nEolType = EOL_CRLF_UNICODE;
//				}else
//				if( nPos <= nReadSize - nCRLF_LEN &&
//					0 == memcmp( &pBuf[nPos], pszCRLF, nCRLF_LEN )
//				){
//					nEolCodeLen = nCRLF_LEN;
//					nEolType = EOL_CRLF;
//				}else
//				if( nPos <= nReadSize - nLFCR_LEN &&
//					0 == memcmp( &pBuf[nPos], pszLFCR, nLFCR_LEN )
//				){
//					nEolCodeLen = nLFCR_LEN;
//					nEolType = EOL_LFCR;
//				}else
//				if( nPos <= nReadSize - nLF_LEN &&
//					0 == memcmp( &pBuf[nPos], pszLF, nLF_LEN )
//				){
//					nEolCodeLen = nLF_LEN;
//					nEolType = EOL_LF;
//				}else{
//					nEolCodeLen = nCR_LEN;
//					nEolType = EOL_CR;
//				}
//				++nLineNum;
//				AddLineStrX( &pBuf[nBgn], nPos - nBgn + nEolCodeLen, nEolType );
//				nReadLength += nPos - nBgn + nEolCodeLen/*cMem.GetLength()*/;
//				if( NULL != hwndProgress && 0 < nFileLength && 0 == (nLineNum % 1000 ) ){
//					::PostMessage( hwndProgress, PBM_SETPOS, nReadLength * 100 / nFileLength , 0 );
//					/* 処理中のユーザー操作を可能にする */
//					if( !::BlockingHook() ){
//						return -1;
//					}
//				}
//				nBgn = nPos + nEolCodeLen;
//				nPos = nBgn;
//			}else{
//				++nPos;
//			}
//		}
//		if( /*nReadSize < nReadBufSize*/nPos - nBgn > 0 ){
//			if( nBgn < nPos ){
//				AddLineStrX( &pBuf[nBgn], nPos - nBgn, EOL_NONE );
//				nReadLength += nPos - nBgn/*cMem.GetLength()*/;
//				if( NULL != hwndProgress && 0 < nFileLength ){
//					::PostMessage( hwndProgress, PBM_SETPOS, nReadLength * 100 / nFileLength , 0 );
//				}
//				/* 処理中のユーザー操作を可能にする */
//				if( !::BlockingHook() ){
//					return -1;
//				}
//			}
//			nEOF = TRUE;
//		}
//
//	}while( !nEOF );





_CLOSEFILE_:;
	_lclose( hFile );
_RETURN_:;
//	delete pBuf;
	if( NULL != hgRead ){
		::GlobalUnlock( hgRead );
		::GlobalFree( hgRead );
	}

	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
	}
	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( NULL ) ){
		return -1;
	}

	/* 行変更状態をすべてリセット */
	ResetAllModifyFlag();
//		TRUE	/* 変更回数を0にするかどうか */
//	);

	return nRetVal;
}












/* バッファ内容をファイルに書き出す (テスト用) */
/* (注意) Windows用にコーディングしてある */
int CDocLineMgr::WriteFile( const char* pszPath, HWND hWndParent, HWND hwndProgress, int nCharCode, FILETIME* pFileTime, CEOL cEol )
{
	const char*		pLine;
//	char*			pLineUnicode;
	int				nLineLen;
	int				nRetVal;
	int				nLineNumber;
//	int				i;
	int				nWriteLen;
//	int				bCRLF;
//	int				nLineUnicodeLen;
//	char*			pszCRLF_UNICODE = "\x0d\x0\x0a\x0";
//	char*			pszCRLF_LF = "\x0a";
	CMemory			cmemBuf;
	CDocLine*		pCDocLine;

	/*
	|| バッファサイズの調整
	*/
	cmemBuf.AllocBuffer( 32000 );

//#ifdef _DEBUG
//		if( IDYES != MYMESSAGEBOX(
//			hWndParent,
//			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
//			"_DEBUG テスト機能",
//			"改行コードを LFCR にして保存します。\nよろしいですか？"
//		) ){
//			return FALSE;
//		}
//#endif
//	int				nCharCode = CODE_SJIS;
//	int				nCharCode = CODE_EUC;
//	int				nCharCode = CODE_JIS;
//	int				nCharCode = CODE_UNICODE;


	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
	}

	nRetVal = TRUE;
	HFILE hFile;
// Oct 6, 2000 ao
/* ファイル出力にstream を使うようにする */
	// 改行コードを勝手に制御されない様、バイナリモードで開く
	FILE *sFile=fopen(pszPath,"wb"); /*add*/

	///* ファイルを書き込み用にオープンする */
//-	hFile = _lopen( pszPath, OF_WRITE );
//-	if( HFILE_ERROR == hFile ){
//-	}else{
//-		_lclose( hFile );
//-	}
//-	hFile = _lcreat(pszPath, 0);
//-	if( HFILE_ERROR == hFile ){
	if( !sFile ){ /*add*/
//		MYTRACE( "file create error %s\n", pszPath );
		::MYMESSAGEBOX(
			hWndParent,
			MB_OK | MB_ICONSTOP,
			GSTR_APPNAME,
			"\'%s\'\nファイルを保存できません。\nパスが存在しないか、他のアプリケーションで使用されている可能性があります。",
			pszPath
		 );
		nRetVal = FALSE;
		goto _RETURN_;
	}

	switch( nCharCode ){
	case CODE_UNICODE:
//-		if( HFILE_ERROR == _lwrite( hFile, "\xff\xfe", 2 ) ){
		if( fwrite( "\xff\xfe", sizeof( char ), 2, sFile ) < 2 ){ /* add */
//			MYTRACE( "file write error %s\n", pszPath );
			nRetVal = FALSE;
			goto _CLOSEFILE_;
		}
		break;
	}

	nLineNumber = 0;
//	pLine = GetFirstLinrStr( &nLineLen );
	pCDocLine = m_pDocLineTop;

//	while( NULL != pLine ){
	while( NULL != pCDocLine ){
		++nLineNumber;
		pLine = pCDocLine->m_pLine->GetPtr( &nLineLen );


		if( NULL != hwndProgress && 0 < m_nLines && 0 == ( nLineNumber % 1024 ) ){
			::PostMessage( hwndProgress, PBM_SETPOS, nLineNumber * 100 / m_nLines , 0 );
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( NULL ) ){
				return -1;
			}
		}

//		for( i = 0; i < nLineLen; ++i ){
//			if( pLine[i] == CR ||
//				pLine[i] == LF ){
//				break;
//			}
//		}
//		if( i < nLineLen ){
//			nWriteLen = i;
//			bCRLF = TRUE;
//		}else{
//			nWriteLen = nLineLen;
//			bCRLF = FALSE;
//		}


		nWriteLen = nLineLen - pCDocLine->m_cEol.GetLen();
		cmemBuf.SetDataSz( "" );
		if( 0 < nWriteLen ){
			cmemBuf.SetData( pLine, nWriteLen );

			/* 書き込み時のコード変換 */
			switch( nCharCode ){
			case CODE_UNICODE:
				/* SJIS→Unicodeコード変換 */
				cmemBuf.SJISToUnicode();
				break;
			case CODE_UTF8:	/* UTF-8 */
				/* SJIS→UTF-8コード変換 */
				cmemBuf.SJISToUTF8();
				break;
			case CODE_UTF7:	/* UTF-7 */
				/* SJIS→UTF-7コード変換 */
				cmemBuf.SJISToUTF7();
				break;
			case CODE_EUC:
				/* SJIS→EUCコード変換 */
				cmemBuf.SJISToEUC();
				break;
			case CODE_JIS:
				/* SJIS→JISコード変換 */
				cmemBuf.SJIStoJIS();
				break;
			case CODE_SJIS:
			default:
				break;
			}
		}
//		if( bCRLF ){
		if( EOL_NONE != pCDocLine->m_cEol ){
			/* 書き込み時の改行コード変換 */
			switch( nCharCode ){
			case CODE_UNICODE:
				/* ASCII+SJIS→Unicode変換 */
				//	May 15, 2000 genta
				{
					CEOL ue( EOL_CRLF_UNICODE );
					cmemBuf.Append( ue.GetValue(), ue.GetLen() );
				}
				break;
// 1999.12.20
//			case CODE_EUC:
//				cmemBuf.Append( gm_pszEolDataArr[EOL_LF], LEN_EOL_LF );
//				break;
			default:
				//	From Here Feb. 8, 2001 genta 改行コード変換処理を追加
				if( cEol == EOL_NONE ){
// 1999.12.20
//				/* 改行コードをCRLFに変換 */
//				cmemBuf.Append( gm_pszEolDataArr[EOL_CRLF], LEN_EOL_CRLF );
					/* 改行コードを変換しない */
					cmemBuf.Append( pCDocLine->m_cEol.GetValue(), pCDocLine->m_cEol.GetLen() );
					break;
				}
				else {
					/* 改行コードを指定されたものに変換 */
					cmemBuf.Append( cEol.GetValue(), cEol.GetLen() );
				}
				//	To Here Feb. 8, 2001 genta
			}
		}
		if( 0 < cmemBuf.GetLength() ){
//-			if( HFILE_ERROR == _lwrite( hFile, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() ) ){
			if( fwrite( cmemBuf.GetPtr( NULL ), sizeof( char ), cmemBuf. GetLength(), sFile ) /* add */
					< (size_t)cmemBuf.GetLength() ){ /* add */
//				MYTRACE( "file write error %s\n", pszPath );
				nRetVal = FALSE;
				goto _CLOSEFILE_;
			}
		}


//		pLine = GetNextLinrStr( &nLineLen );
		pCDocLine = pCDocLine->m_pNext;
	}
_CLOSEFILE_:;
//-	_lclose( hFile );
	fflush( sFile );/* add */
	fclose( sFile );/* add */
// Oct 6, 2000 ao end
/* ファイル出力に関する変更はここまで。
	この後変更後のファイル情報を開くためにファイルアクセスしているが、ここまで無理に変更する必要はないでしょう。*/

	/* 更新後のファイル時刻の取得 */
	hFile = _lopen( pszPath, OF_READ );
	if( HFILE_ERROR != hFile ){

		FILETIME	FileTime;
		SYSTEMTIME	systimeL;
		if( ::GetFileTime( (HANDLE)hFile, NULL, NULL, &FileTime ) ){
			*pFileTime = FileTime;
			::FileTimeToLocalFileTime( &FileTime, &FileTime );
			::FileTimeToSystemTime( &FileTime, &systimeL );
//			MYTRACE( "Last Update: %d/%d/%d %02d:%02d:%02d\n",
//				systimeL.wYear,
//				systimeL.wMonth,
//				systimeL.wDay,
//				systimeL.wHour,
//				systimeL.wMinute,
//				systimeL.wSecond
//			);
		}else{
//			MYTRACE( "GetFileTime() error.\n" );
		}
	}
	_lclose( hFile );


_RETURN_:;
	if( NULL != hwndProgress ){
		::PostMessage( hwndProgress, PBM_SETPOS, 0, 0 );
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) ){
			return -1;
		}
	}

	/* 行変更状態をすべてリセット */
	ResetAllModifyFlag();
//		FALSE	/* 変更回数を0にするかどうか */
//	);

	return nRetVal;
}





/* データの削除 */
/*
|| 指定行内の文字しか削除できません
|| データ変更によって影響のあった、変更前と変更後の行の範囲を返します
|| この情報をもとに、レイアウト情報などを更新してください。
||
*/
void CDocLineMgr::DeleteData(
			int			nLine,
			int			nDelPos,
			int			nDelLen,
			int*		pnModLineOldFrom,	/* 影響のあった変更前の行(from) */
			int*		pnModLineOldTo,		/* 影響のあった変更前の行(to) */
			int*		pnDelLineOldFrom,	/* 削除された変更前論理行(from) */
			int*		pnDelLineOldNum,	/* 削除された行数 */
			CMemory&	cmemDeleted,		/* 削除されたデータ */
			int			bUndo				/* Undo操作かどうか */
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::DeleteData" );
#endif
	CDocLine*	pDocLine;
	CDocLine*	pDocLine2;
	char*		pData;
	int			nDeleteLength;
	char*		pLine;
	int			nLineLen;
	char*		pLine2;
	int			nLineLen2;
	*pnModLineOldFrom = nLine;	/* 影響のあった変更前の行(from) */
	*pnModLineOldTo = nLine;	/* 影響のあった変更前の行(to) */
	*pnDelLineOldFrom = 0;		/* 削除された変更前論理行(from) */
	*pnDelLineOldNum = 0;		/* 削除された行数 */
//	cmemDeleted.SetData( "", lstrlen( "" ) );
	cmemDeleted.SetDataSz( "" );

	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		return;
	}
	/* Undo操作かどうか */
//	if( bUndo ){
//		pDocLine->m_nModifyCount--;				/* 変更回数 */
//		if( 0 == pDocLine->m_nModifyCount ){	/* 変更回数 */
//			pDocLine->m_bModify = FALSE;		/* 変更フラグ */
//		}
//		if( 0 > pDocLine->m_nModifyCount ){		/* 変更回数 */
//			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, "作者に教えて欲しいエラー",
//				"CDocLineMgr::DeleteData()で変更カウンタが0以下になりました。バグじゃぁああ"
//			);
//		}
//	}else{
//		++pDocLine->m_nModifyCount;	/* 変更回数 */
//		pDocLine->m_bModify = TRUE;	/* 変更フラグ */
//	}
	pDocLine->m_bModify = TRUE;		/* 変更フラグ */

	pLine = pDocLine->m_pLine->GetPtr( &nLineLen );

	if( nDelPos >= nLineLen ){
		return;
	}
	/* 「改行」を削除する場合は、次の行と連結する */
//	if( ( nDelPos == nLineLen -1 && ( pLine[nDelPos] == CR || pLine[nDelPos] == LF ) )
//	 || nDelPos + nDelLen >= nLineLen
	if( ( EOL_NONE != pDocLine->m_cEol && nDelPos == nLineLen - pDocLine->m_cEol.GetLen() )
	 || ( EOL_NONE != pDocLine->m_cEol && nDelPos + nDelLen >  nLineLen - pDocLine->m_cEol.GetLen() )
	 || ( EOL_NONE == pDocLine->m_cEol && nDelPos + nDelLen >= nLineLen - pDocLine->m_cEol.GetLen() )
	){
		/* 実際に削除するバイト数 */
		nDeleteLength = nLineLen - nDelPos;

		/* 削除されるデータ */
		cmemDeleted.SetData( &pLine[nDelPos], nDeleteLength );

		/* 次の行の情報 */
		if( NULL == ( pDocLine2 = pDocLine->m_pNext ) ){
			pData = new char[nLineLen + 1];
			if( nDelPos > 0 ){
				memcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				memcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			pData[ nLineLen - nDeleteLength ] = '\0';
			/* 改行コードの情報を更新 */
			pDocLine->m_cEol.SetType( EOL_NONE );

			if( 0 < nLineLen - nDeleteLength ){
				pDocLine->m_pLine->SetData( pData, nLineLen - nDeleteLength );
			}else{
				/* 行の削除 */
				if( NULL != pDocLine->m_pPrev ){
					pDocLine->m_pPrev->m_pNext = NULL;
				}
				m_pDocLineBot = pDocLine->m_pPrev;
				*pnDelLineOldFrom = nLine;	/* 削除された変更前論理行(from) */
				*pnDelLineOldNum = 1;		/* 削除された行数 */
				m_nLines--;					/* 全行数 */
				if( 0 == m_nLines ){
					/* データがなくなった */
					Init();
				}
			}
			delete [] pData;
		}else{
			*pnModLineOldTo = nLine + 1;	/* 影響のあった変更前の行(to) */
			pLine2 = pDocLine2->m_pLine->GetPtr( &nLineLen2 );
			pData = new char[nLineLen + nLineLen2 + 1];
			if( nDelPos > 0 ){
				memcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				memcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			/* 次の行のデータを連結 */
			memcpy( pData + (nLineLen - nDeleteLength), pLine2, nLineLen2 );
			pData[ nLineLen - nDeleteLength + nLineLen2 ] = '\0';
			pDocLine->m_pLine->SetData( pData, nLineLen - nDeleteLength + nLineLen2 );
			/* 改行コードの情報を更新 */
			pDocLine->m_cEol = pDocLine2->m_cEol;

			/* 次の行を削除 && 次次行とのリストの連結*/
			pDocLine->m_pNext = pDocLine2->m_pNext;
			if( NULL != pDocLine->m_pNext ){
				pDocLine->m_pNext->m_pPrev = pDocLine;
			}else{
				m_pDocLineBot = pDocLine;
			}
			delete pDocLine2;
			*pnDelLineOldFrom = nLine + 1;	/* 削除された変更前論理行(from) */
			*pnDelLineOldNum = 1;			/* 削除された行数 */
			m_nLines--;						/* 全行数 */
			if( 0 == m_nLines ){
				/* データがなくなった */
				Init();
			}
			delete [] pData;
		}
	}else{
		/* 実際に削除するバイト数 */
		nDeleteLength = nDelLen;

		/* 削除されるデータ */
		cmemDeleted.SetData( &pLine[nDelPos], nDeleteLength );

		pData = new char[nLineLen + 1];
		if( nDelPos > 0 ){
			memcpy( pData, pLine, nDelPos );
		}
		if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
			memcpy(
				pData + nDelPos,
				pLine + nDelPos + nDeleteLength,
				nLineLen - ( nDelPos + nDeleteLength )
			);
		}
		pData[ nLineLen - nDeleteLength ] = '\0';
		if( 0 < nLineLen - nDeleteLength ){
			pDocLine->m_pLine->SetData( pData, nLineLen - nDeleteLength );
		}else{
		}
		delete [] pData;
	}
//	DUMP();
	return;
}





/* データの挿入 */
void CDocLineMgr::InsertData_CDocLineMgr(
			int			nLine,
			int			nInsPos,
			const char*	pInsData,
			int			nInsDataLen,
			int*		pnInsLineNum,	/* 挿入によって増えた行の数 */
			int*		pnNewLine,		/* 挿入された部分の次の位置の行 */
			int*		pnNewPos,		/* 挿入された部分の次の位置のデータ位置 */
			int			bUndo			/* Undo操作かどうか */
)
{
	CDocLine*	pDocLine;
	CDocLine*	pDocLineNew;
	char*		pLine;
	int			nLineLen;
	int			nBgn;
	int			nPos;
	CMemory		cmemPrevLine;
	CMemory		cmemCurLine;
	CMemory		cmemNextLine;
	int			nAllLinesOld = m_nLines;
	int			nCount;

	//	May 15, 2000 genta
	CEOL 		cEOLType;
	CEOL 		cEOLTypeNext;
	//enumEOLType nEOLType;
	//enumEOLType nEOLTypeNext;

	*pnNewLine = nLine;	/* 挿入された部分の次の位置の行 */
	*pnNewPos  = 0;		/* 挿入された部分の次の位置のデータ位置 */

	/* 挿入データを行終端で区切った行数カウンタ */
	nCount = 0;
	*pnInsLineNum = 0;
	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		/* ここでNULLが帰ってくるということは、*/
		/* 全テキストの最後の次の行を追加しようとしていることを示す */
		cmemPrevLine.SetDataSz( "" );
		cmemNextLine.SetDataSz( "" );
		cEOLTypeNext.SetType( EOL_NONE );
	}else{
//		/* Undo操作かどうか */
//		if( bUndo ){
//			pDocLine->m_nModifyCount--;				/* 変更回数 */
//			if( 0 == pDocLine->m_nModifyCount ){	/* 変更回数 */
//				pDocLine->m_bModify = FALSE;		/* 変更フラグ */
//			}
//			if( 0 > pDocLine->m_nModifyCount ){		/* 変更回数 */
//				::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, "作者に教えて欲しいエラー",
//					"CDocLineMgr::InsertData()で変更カウンタが0以下になりました。バグじゃぁああ"
//				);
//			}
//		}else{
//			++pDocLine->m_nModifyCount;	/* 変更回数 */
//			pDocLine->m_bModify = TRUE;	/* 変更フラグ */
//		}
		pDocLine->m_bModify = TRUE;		/* 変更フラグ */

		pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
		cmemPrevLine.SetData( pLine, nInsPos );
		cmemNextLine.SetData( &pLine[nInsPos], nLineLen - nInsPos );

		cEOLTypeNext = pDocLine->m_cEol;
	}
	nBgn = 0;
	nPos = 0;
	for( nPos = 0; nPos < nInsDataLen; ){
		if( pInsData[nPos] == '\n' || pInsData[nPos] == '\r' ){
			/* 行終端子の種類を調べる */
			cEOLType.GetTypeFromString( &pInsData[nPos], nInsDataLen - nPos );
			/* 行終端子も含めてテキストをバッファに格納 */
			cmemCurLine.SetData( &pInsData[nBgn], nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + cEOLType.GetLen();
			nPos = nBgn;
			if( NULL == pDocLine ){
				pDocLineNew = new CDocLine;

				pDocLineNew->m_pLine = new CMemory;
				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					if( NULL == m_pDocLineTop ){
						m_pDocLineTop = pDocLineNew;
					}
					pDocLineNew->m_pPrev = m_pDocLineBot;
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pDocLineNew;
					}
					m_pDocLineBot = pDocLineNew;
					pDocLineNew->m_pNext = NULL;
					pDocLineNew->m_pLine->SetData( &cmemPrevLine );
					*(pDocLineNew->m_pLine) += cmemCurLine;

					pDocLineNew->m_cEol = cEOLType;							/* 改行コードの種類 */
					// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];	/* 改行コードの長さ */
				}else{
					if( NULL != m_pDocLineBot ){
						m_pDocLineBot->m_pNext = pDocLineNew;
					}
					pDocLineNew->m_pPrev = m_pDocLineBot;
					m_pDocLineBot = pDocLineNew;
					pDocLineNew->m_pNext = NULL;
					pDocLineNew->m_pLine->SetData( &cmemCurLine );

					pDocLineNew->m_cEol = cEOLType;							/* 改行コードの種類 */
					// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];	/* 改行コードの長さ */
				}
				pDocLine = NULL;
				++m_nLines;
			}else{
				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					pDocLine->m_pLine->SetData( &cmemPrevLine );
					*(pDocLine->m_pLine) += cmemCurLine;

					pDocLine->m_cEol = cEOLType;						/* 改行コードの種類 */
					// pDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLType];	/* 改行コードの長さ */

					pDocLine = pDocLine->m_pNext;
				}else{
					pDocLineNew = new CDocLine;
					pDocLineNew->m_pLine = new CMemory;
					pDocLineNew->m_pPrev = pDocLine->m_pPrev;
					pDocLineNew->m_pNext = pDocLine;
					pDocLine->m_pPrev->m_pNext = pDocLineNew;
					pDocLine->m_pPrev = pDocLineNew;
					pDocLineNew->m_pLine->SetData( &cmemCurLine );

					pDocLineNew->m_cEol = cEOLType;							/* 改行コードの種類 */
					// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLType];	/* 改行コードの長さ */

					++m_nLines;
				}
			}

			/* 挿入データを行終端で区切った行数カウンタ */
			++nCount;
			++(*pnNewLine);	/* 挿入された部分の次の位置の行 */
		}else{
			++nPos;
		}
	}
//	nEOLType = EOL_NONE;
	if( 0 < nPos - nBgn || 0 < cmemNextLine.GetLength() ){
		cmemCurLine.SetData( &pInsData[nBgn], nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pDocLine ){
			pDocLineNew = new CDocLine;
			pDocLineNew->m_pLine = new CMemory;
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				if( NULL == m_pDocLineTop ){
					m_pDocLineTop = pDocLineNew;
				}
				pDocLineNew->m_pPrev = m_pDocLineBot;
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pDocLineNew;
				}
				m_pDocLineBot = pDocLineNew;
				pDocLineNew->m_pNext = NULL;
				pDocLineNew->m_pLine->SetData( &cmemPrevLine );
				*(pDocLineNew->m_pLine) += cmemCurLine;

				pDocLineNew->m_cEol = cEOLTypeNext;							/* 改行コードの種類 */
				// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* 改行コードの長さ */

			}else{
				if( NULL != m_pDocLineBot ){
					m_pDocLineBot->m_pNext = pDocLineNew;
				}
				pDocLineNew->m_pPrev = m_pDocLineBot;
				m_pDocLineBot = pDocLineNew;
				pDocLineNew->m_pNext = NULL;
				pDocLineNew->m_pLine->SetData( &cmemCurLine );

				pDocLineNew->m_cEol = cEOLTypeNext;							/* 改行コードの種類 */
				// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* 改行コードの長さ */

			}
			pDocLine = NULL;
			++m_nLines;
			*pnNewPos = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
		}else{
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				pDocLine->m_pLine->SetData( &cmemPrevLine );
				*(pDocLine->m_pLine) += cmemCurLine;

				pDocLine->m_cEol = cEOLTypeNext;						/* 改行コードの種類 */
				// pDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* 改行コードの長さ */

				pDocLine = pDocLine->m_pNext;
				*pnNewPos = cmemPrevLine.GetLength() + nPos - nBgn;		/* 挿入された部分の次の位置のデータ位置 */
			}else{
				pDocLineNew = new CDocLine;
				pDocLineNew->m_pLine = new CMemory;
				pDocLineNew->m_pPrev = pDocLine->m_pPrev;
				pDocLineNew->m_pNext = pDocLine;
				pDocLine->m_pPrev->m_pNext = pDocLineNew;
				pDocLine->m_pPrev = pDocLineNew;
				pDocLineNew->m_pLine->SetData( &cmemCurLine );

				pDocLineNew->m_cEol = cEOLTypeNext;							/* 改行コードの種類 */
				// pDocLineNew->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* 改行コードの長さ */


				++m_nLines;
				*pnNewPos = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}
		}
	}
	*pnInsLineNum = m_nLines - nAllLinesOld;
	return;
}

//nEOLType nEOLType nEOLTypeNext



/* 現在位置の単語の範囲を調べる */
int	CDocLineMgr::WhereCurrentWord(
	int			nLineNum,
	int			nIdx,
	int*		pnIdxFrom,
	int*		pnIdxTo,
	CMemory*	pcmcmWord,
	CMemory*	pcmcmWordLeft
)
{
	CDocLine*	pDocLine;
	char*		pLine;
	int			nLineLen;
	int			nCharKind;
	int			nCharKindNext;
	int			nIdxNext;
	int			nIdxNextPrev;
	int			nCharChars;
	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;
	pDocLine = GetLineInfo( nLineNum );
	if( NULL == pDocLine ){
		return FALSE;
	}
	pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
	if( nIdx >= nLineLen ){
		return FALSE;
	}
	/* 現在位置の文字の種類によっては選択不能 */
	if( pLine[nIdx] == CR || pLine[nIdx] == LF ){
		return FALSE;
	}
	/* 現在位置の文字の種類を調べる */
	nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );
	/* 文字種類が変わるまで前方へサーチ */
	nIdxNext = nIdx;
	nCharChars = &pLine[nIdxNext] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxNext] );
	while( nCharChars > 0 ){
		nIdxNextPrev = nIdxNext;
		nIdxNext -= nCharChars;
		nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
		if( nCharKind == CK_MBC_NOVASU ){
			if( nCharKindNext == CK_MBC_HIRA ||
				nCharKindNext == CK_MBC_KATA ){
				nCharKind = nCharKindNext;
			}
		}else
		if( nCharKind == CK_MBC_HIRA ||
			nCharKind == CK_MBC_KATA ){
			if( nCharKindNext == CK_MBC_NOVASU ){
				nCharKindNext = nCharKind;
			}
		}
		if( nCharKind != nCharKindNext ){
			nIdxNext = nIdxNextPrev;
			break;
		}
		nCharChars = &pLine[nIdxNext] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxNext] );
	}
	*pnIdxFrom = nIdxNext;

	if( NULL != pcmcmWordLeft ){
		pcmcmWordLeft->SetData( &pLine[*pnIdxFrom], nIdx - *pnIdxFrom );
	}

	/* 文字種類が変わるまで後方へサーチ */
	nIdxNext = nIdx;
	nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxNext] ) - &pLine[nIdxNext];
	while( nCharChars > 0 ){
		nIdxNext += nCharChars;
		nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
		if( nCharKind == CK_MBC_NOVASU ){
			if( nCharKindNext == CK_MBC_HIRA ||
				nCharKindNext == CK_MBC_KATA ){
				nCharKind = nCharKindNext;
			}
		}else
		if( nCharKind == CK_MBC_HIRA ||
			nCharKind == CK_MBC_KATA ){
			if( nCharKindNext == CK_MBC_NOVASU ){
				nCharKindNext = nCharKind;
			}
		}
		if( nCharKind != nCharKindNext ){
			break;
		}
		nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxNext] ) - &pLine[nIdxNext];
	}
	*pnIdxTo = nIdxNext;

	if( NULL != pcmcmWord ){
		pcmcmWord->SetData( &pLine[*pnIdxFrom], *pnIdxTo - *pnIdxFrom );
	}
	return TRUE;
}





/* 現在位置の左右の単語の先頭位置を調べる */
int CDocLineMgr::PrevOrNextWord(
		int		nLineNum,
		int		nIdx,
		int*	pnColmNew,
		int		bLEFT
)
{
	CDocLine*	pDocLine;
	char*		pLine;
	int			nLineLen;
	int			nCharKind;
	int			nCharKindNext;
	int			nIdxNext;
	int			nIdxNextPrev;
	int			nCharChars;
	int			nCount;
	pDocLine = GetLineInfo( nLineNum );
	if( NULL == pDocLine ){
		return FALSE;
	}
	pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
	if( nIdx >= nLineLen ){
		nIdx = nLineLen - 1;
//		return FALSE;
	}
	/* 現在位置の文字の種類によっては選択不能 */
	if( FALSE == bLEFT && ( pLine[nIdx] == CR || pLine[nIdx] == LF ) ){
		return FALSE;
	}
	/* 現在位置の文字の種類を調べる */
	nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );
	/* 前の単語か？後ろの単語か？ */
	if( bLEFT ){
		if( nIdx == 0 ){
			return FALSE;
		}
		/* 文字種類が変わるまで前方へサーチ */
		/* 空白とタブは無視する */
		nCount = 0;
		nIdxNext = nIdx;
		nCharChars = &pLine[nIdxNext] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxNext] );
		while( nCharChars > 0 ){
			nIdxNextPrev = nIdxNext;
			nIdxNext -= nCharChars;
			nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
			/* 空白とタブは無視する */
			if( nCharKind == CK_MBC_NOVASU ){
				if( nCharKindNext == CK_MBC_HIRA ||
					nCharKindNext == CK_MBC_KATA ){
					nCharKind = nCharKindNext;
				}
			}else
			if( nCharKind == CK_MBC_HIRA ||
				nCharKind == CK_MBC_KATA ){
				if( nCharKindNext == CK_MBC_NOVASU ){
					nCharKindNext = nCharKind;
				}
			}
			if( nCharKind != nCharKindNext ){
				/* サーチ開始位置の文字が空白またはタブの場合 */
				if( nCharKind == CK_TAB	|| nCharKind == CK_SPACE ){
					nCharKind = nCharKindNext;
				}else{
					if( nCount == 0){
						nCharKind = nCharKindNext;
					}else{
						nIdxNext = nIdxNextPrev;
						break;
					}
				}
			}
			nCharChars = &pLine[nIdxNext] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxNext] );
			++nCount;
		}
		*pnColmNew = nIdxNext;
	}else{
		/* 文字種類が変わるまで後方へサーチ */
		/* 空白とタブは無視する */
		nIdxNext = nIdx;
		nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxNext] ) - &pLine[nIdxNext];
		while( nCharChars > 0 ){
			nIdxNext += nCharChars;
			nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
			/* 空白とタブは無視する */
			if( nCharKindNext != CK_TAB	&& nCharKindNext != CK_SPACE ){
				if( nCharKind == CK_MBC_NOVASU ){
					if( nCharKindNext == CK_MBC_HIRA ||
						nCharKindNext == CK_MBC_KATA ){
						nCharKind = nCharKindNext;
					}
				}else
				if( nCharKind == CK_MBC_HIRA ||
					nCharKind == CK_MBC_KATA ){
					if( nCharKindNext == CK_MBC_NOVASU ){
						nCharKindNext = nCharKind;
					}
				}
				if( nCharKind != nCharKindNext ){
					break;
				}
			}else{
				nCharKind = nCharKindNext;
			}
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxNext] ) - &pLine[nIdxNext];
		}
		*pnColmNew = nIdxNext;
	}
	return TRUE;
}





/* 単語検索 */
/* 見つからない場合は０を返す */
int CDocLineMgr::SearchWord(
	int			nLineNum,		/* 検索開始行 */
	int			nIdx, 			/* 検索開始位置 */
	const char*	pszPattern,		/* 検索条件 */
	int			bPrevOrNext,	/* 0==前方検索 1==後方検索 */
	int			bRegularExp,	/* 1==正規表現 */
	int			bLoHiCase,		/* 1==英大文字小文字の区別 */
	int			bWordOnly,		/* 1==単語のみ検索 */
	int*		pnLineNum, 		/* マッチ行 */
	int*		pnIdxFrom, 		/* マッチ位置from */
	int*		pnIdxTo,  		/* マッチ位置to */
	CJre*		pCJre			/* 正規表現コンパイルデータ */
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::SearchWord" );
//#endif
	CDocLine*	pDocLine;
	int			nLinePos;
	int			nIdxPos;
	const char*	pLine;
	int			nLineLen;
	char*		pszRes;
	int			nHitTo;
	int			nHitPos;
	int			nHitPosOld;
	int			nHitLenOld;
	int			nRetVal;
	int*		pnKey_CharCharsArr;
	pnKey_CharCharsArr = NULL;
//	int*		pnKey_CharUsedArr;
//	pnKey_CharUsedArr = NULL;
	/* 検索条件の情報 */
	CDocLineMgr::CreateCharCharsArr(
		(const unsigned char *)pszPattern,
		lstrlen( pszPattern ),
		&pnKey_CharCharsArr
	);
//	/* 検索条件の情報(キー文字列の使用文字表)作成 */
//	CDocLineMgr::CreateCharUsedArr(
//		(const unsigned char *)pszPattern,
//		lstrlen( pszPattern ),
//		pnKey_CharCharsArr,
//		&pnKey_CharUsedArr
//	);

	/* 1==正規表現 */
	if( bRegularExp ){
//		CJre	cJre;
//		/* CJreクラスの初期化 */
//		cJre.Init();
//
//		/* jre32.dllの存在チェック */
//		if( FALSE == cJre.IsExist() ){
//			::MessageBox( 0, "jre32.dllが見つかりません。\n正規表現を利用するにはjre32.dllが必要です。\n", "情報", MB_OK | MB_ICONEXCLAMATION );
//			nRetVal = 0;
//			goto end_of_func;
//		}
//		/* 検索パターンのコンパイル */
//		if( !cJre.Compile( pszPattern ) ){
//			nRetVal = 0;
//			goto end_of_func;
//		}
		/* 0==前方検索 1==後方検索 */
		if( 0 == bPrevOrNext ){
			nLinePos = nLineNum;
			nHitTo = nIdx;
			nIdxPos = 0;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				nHitPos    = -1;
				while( 1 ){
					nHitPosOld = nHitPos;
					nHitLenOld = pCJre->m_jreData.nLength;
					pszRes = (char*)(pCJre->GetMatchInfo( pLine, nLineLen, nIdxPos ));
					if( NULL != pszRes ){
						nHitPos = pszRes - pLine;
						nIdxPos = pszRes - pLine + 1;
						if( nHitPos >= nHitTo ){
							if( -1 != nHitPosOld ){
								*pnLineNum = nLinePos;				/* マッチ行 */
								*pnIdxFrom = nHitPosOld;			/* マッチ位置from */
								*pnIdxTo = *pnIdxFrom + nHitLenOld;	/* マッチ位置to */
								nRetVal = 1;
								goto end_of_func;
							}else{
								break;
							}
						}
					}else{
						if( -1 != nHitPosOld ){
							*pnLineNum = nLinePos;				/* マッチ行 */
							*pnIdxFrom = nHitPosOld;			/* マッチ位置from */
							*pnIdxTo = *pnIdxFrom + nHitLenOld;	/* マッチ位置to */
							nRetVal = 1;
							goto end_of_func;
						}else{
							break;
						}
					}
				}
				nLinePos--;
				pDocLine = pDocLine->m_pPrev;
				nIdxPos = 0;
				if( NULL != pDocLine ){
//					nHitTo = lstrlen( pDocLine->m_pLine->GetPtr( NULL ) );
					nHitTo = pDocLine->m_pLine->GetLength();
				}
			}
			nRetVal = 0;
			goto end_of_func;
		}else{
			nIdxPos = nIdx;
			nLinePos = nLineNum;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				pszRes = (char*)pCJre->GetMatchInfo( pLine, nLineLen, nIdxPos );
				if( NULL != pszRes ){
					*pnLineNum = nLinePos;								/* マッチ行 */
					*pnIdxFrom = pszRes - pLine;						/* マッチ位置from */
					*pnIdxTo = *pnIdxFrom + pCJre->m_jreData.nLength;	/* マッチ位置to */
					nRetVal = 1;
					goto end_of_func;
				}
				++nLinePos;
				pDocLine = pDocLine->m_pNext;
				nIdxPos = 0;
			}
			nRetVal = 0;
			goto end_of_func;
		}
	}else
	/* 1==単語のみ検索 */
	if( bWordOnly ){
		nRetVal = 0;
		goto end_of_func;
	}else{
		/* 0==前方検索 1==後方検索 */
		if( 0 == bPrevOrNext ){
			nLinePos = nLineNum;
			nHitTo = nIdx;

			nIdxPos = 0;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				nHitPos = -1;
				while( 1 ){
					nHitPosOld = nHitPos;
					pszRes = SearchString(
						(const unsigned char *)pLine,
						nLineLen,
						nIdxPos,
						(const unsigned char *)pszPattern,
						lstrlen( pszPattern ),
						pnKey_CharCharsArr,
//						pnKey_CharUsedArr,
						bLoHiCase
					);
					if( NULL != pszRes ){
						nHitPos = pszRes - pLine;
						nIdxPos = pszRes - pLine + 1;
						if( nHitPos >= nHitTo ){
							if( -1 != nHitPosOld ){
								*pnLineNum = nLinePos;							/* マッチ行 */
								*pnIdxFrom = nHitPosOld;						/* マッチ位置from */
								*pnIdxTo = *pnIdxFrom + lstrlen( pszPattern );	/* マッチ位置to */
								nRetVal = 1;
								goto end_of_func;
							}else{
								break;
							}
						}
					}else{
						if( -1 != nHitPosOld ){
							*pnLineNum = nLinePos;							/* マッチ行 */
							*pnIdxFrom = nHitPosOld;						/* マッチ位置from */
							*pnIdxTo = *pnIdxFrom + lstrlen( pszPattern );	/* マッチ位置to */
							nRetVal = 1;
							goto end_of_func;
						}else{
							break;
						}
					}
				}
				nLinePos--;
				pDocLine = pDocLine->m_pPrev;
				nIdxPos = 0;
				if( NULL != pDocLine ){
//					nHitTo = lstrlen( pDocLine->m_pLine->GetPtr( NULL ) );
					nHitTo = pDocLine->m_pLine->GetLength();
				}
			}
			nRetVal = 0;
			goto end_of_func;
		}else{
			nIdxPos = nIdx;
			nLinePos = nLineNum;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
				pszRes = SearchString(
					(const unsigned char *)pLine,
					nLineLen,
					nIdxPos,
					(const unsigned char *)pszPattern,
					lstrlen( pszPattern ),
					pnKey_CharCharsArr,
//					pnKey_CharUsedArr,
					bLoHiCase
				);
				if( NULL != pszRes ){
					*pnLineNum = nLinePos;							/* マッチ行 */
					*pnIdxFrom = pszRes - pLine;					/* マッチ位置from */
					*pnIdxTo = *pnIdxFrom + lstrlen( pszPattern );	/* マッチ位置to */
					nRetVal = 1;
					goto end_of_func;
				}
				++nLinePos;
				pDocLine = pDocLine->m_pNext;
				nIdxPos = 0;
			}
			nRetVal = 0;
			goto end_of_func;
		}
	}
end_of_func:;
	if( NULL != pnKey_CharCharsArr ){
		delete [] pnKey_CharCharsArr;
		pnKey_CharCharsArr = NULL;
	}
//	if( NULL != pnKey_CharUsedArr ){
//		delete [] pnKey_CharUsedArr;
//		pnKey_CharUsedArr = NULL;
//	}
	return nRetVal;

}

/* 検索条件の情報(キー文字列の全角か半角かの配列)作成 */
void CDocLineMgr::CreateCharCharsArr(
	const unsigned char*	pszPattern,
	int						nSrcLen,
	int**					ppnCharCharsArr
)
{
	int		i;
	int*	pnCharCharsArr;
	pnCharCharsArr = new int[nSrcLen];
	for( i = 0; i < nSrcLen; /*i++*/ ){
		pnCharCharsArr[i] = CMemory::MemCharNext( (const char *)pszPattern, nSrcLen, (const char *)&pszPattern[i] ) - (const char *)&pszPattern[i];
		if( 0 == pnCharCharsArr[i] ){
			pnCharCharsArr[i] = 1;
		}
		if( 2 == pnCharCharsArr[i] ){
			pnCharCharsArr[i + 1] = pnCharCharsArr[i];
		}
		i+= pnCharCharsArr[i];
	}
	*ppnCharCharsArr = pnCharCharsArr;
	return;
}


//	/* 検索条件の情報(キー文字列の使用文字表)作成 */
//	void CDocLineMgr::CreateCharUsedArr(
//		const unsigned char*	pszPattern,
//		int						nSrcLen,
//		const int*				pnCharCharsArr,
//		int**					ppnCharUsedArr
//	)
//	{
//		int		i;
//		int*	pnCharUsedArr;
//		int		nCharCode;
//		pnCharUsedArr = new int[0x0000ffff];
//
//		i = 0xffff;//TEST
//
//		for( i = 0; i < 0x0000ffff; ++i ){
//			pnCharUsedArr[i] = nSrcLen;
//		}
//		for( i = 0; i < nSrcLen; /*i++*/ ){
//			if( 2 == pnCharCharsArr[i] ){
//	//			nCharCode = *((WORD*)(&pszPattern[i]));
//				nCharCode = MAKEWORD( pszPattern[i + 1], pszPattern[i] );
//			}else{
//				nCharCode = (int)pszPattern[i];
//			}
//			if( nSrcLen == pnCharUsedArr[nCharCode] ){
//				pnCharUsedArr[nCharCode] = i;
//			}
//			i+= pnCharCharsArr[i];
//		}
//		*ppnCharUsedArr = pnCharUsedArr;
//		return;
//	}


/* 文字列検索 */
char* CDocLineMgr::SearchString(
		const unsigned char*	pLine,
		int						nDesLen,
		int						nIdxPos,
		const unsigned char*	pszPattern,
		int						nSrcLen,
		int*					pnCharCharsArr,
//		int*					pnCharUsedArr,
		int						bLoHiCase
)
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CDocLineMgr::SearchString" );
//#endif
	if( nDesLen < nSrcLen ){
		return NULL;
	}
	if( 0 >= nSrcLen || 0 >= nDesLen){
		return NULL;
	}

	int	nPos;
	int	i;
	int	j;
	int	nWork;
	int	nCharChars;
	int	nCharChars1;
	int	nCharChars2;
	int	nCompareTo;

//	Mar. 4, 2001 genta
#if 0
	if( !bLoHiCase ){
		nCompareTo = nDesLen - nSrcLen;
		for( nPos = nIdxPos; nPos <= nCompareTo; /*nPos++*/ ){
			nCharChars = CMemory::MemCharNext( (const char *)pLine, nDesLen, (const char *)&pLine[nPos] ) - (const char *)&pLine[nPos];
			nCharChars1 = nCharChars;
			for( i = 0; i < nSrcLen; /*i++*/ ){
				if( NULL != pnCharCharsArr ){
					nCharChars2 = pnCharCharsArr[i];
				}else{
					nCharChars2 = CMemory::MemCharNext( (const char *)pszPattern, nSrcLen, (const char *)&pszPattern[i] ) - (const char *)&pszPattern[i];
				}
				if( nCharChars1 != nCharChars2 ){
					break;
				}
				if( 0 == nCharChars1 ){
					nWork =1;
				}else{
					nWork = nCharChars1;
				}

				for( j = 0; j < nWork; ++j ){
					if( islower( pLine[nPos + i + j] ) ){
						if( islower( pszPattern[i + j] ) ){
							if( toupper( pLine[nPos + i + j] ) != toupper( pszPattern[i + j] ) ){
								break;
							}
						}else{
							if( toupper( pLine[nPos + i + j] ) != pszPattern[i + j] ){
								break;
							}
						}
					}else{
						if( islower( pszPattern[i + j] ) ){
							if( pLine[nPos + i + j] != toupper( pszPattern[i + j] ) ){
								break;
							}
						}else{
							if( pLine[nPos + i + j] != pszPattern[i + j] ){
								break;
							}
						}
					}
				}
				if( j < nWork ){
					break;
				}
				if( 0 == nCharChars2 ){
					++i;
				}else{
					i+= nCharChars2;
				}
				nCharChars1 = CMemory::MemCharNext( (const char *)pLine, nDesLen, (const char *)&pLine[nPos + i] ) - (const char *)&pLine[nPos + i];
			}
			if( i >= nSrcLen ){
				return (char *)&pLine[nPos];
			}
			if( 0 == nCharChars ){
				++nPos;
			}else{
				nPos+= nCharChars;
			}
		}
		return NULL;
	}else{
//	Mar. 4, 2001 genta
#endif
		/* 線形探索 */
		nCompareTo = nDesLen - nSrcLen;	//	Mar. 4, 2001 genta
		for( nPos = nIdxPos; nPos <= nCompareTo; /*nPos++*/ ){
			nCharChars = CMemory::MemCharNext( (const char *)pLine, nDesLen, (const char *)&pLine[nPos] ) - (const char *)&pLine[nPos];
			nCharChars1 = nCharChars;
			for( i = 0; i < nSrcLen; /*i++*/ ){
				if( NULL != pnCharCharsArr ){
					nCharChars2 = pnCharCharsArr[i];
				}else{
					nCharChars2 = CMemory::MemCharNext( (const char *)pszPattern, nSrcLen, (const char *)&pszPattern[i] ) - (const char *)&pszPattern[i];
				}
				if( nCharChars1 != nCharChars2 ){
					break;
				}
				if( 0 == nCharChars1 ){
					nWork =1;
				}else{
					nWork = nCharChars1;
				}
				//	From Here Mar. 4, 2001 genta
				if( !bLoHiCase && nWork == 1 ){	//	英大小文字の同一視
					if( toupper( pLine[nPos + i] ) != toupper( pszPattern[i] ) )
						break;
				}
				else {
					for( j = 0; j < nWork; ++j ){
						if( pLine[nPos + i + j] != pszPattern[i + j] ){
							break;
						}
					}
					if( j < nWork ){
						break;
					}
				}
				//	To Here
				if( 0 == nCharChars2 ){
					++i;
				}else{
					i+= nCharChars2;
				}
				nCharChars1 = CMemory::MemCharNext( (const char *)pLine, nDesLen, (const char *)&pLine[nPos + i] ) - (const char *)&pLine[nPos + i];
			}
			if( i >= nSrcLen ){
				return (char *)&pLine[nPos];
			}
			if( 0 == nCharChars ){
				++nPos;
			}else{
				nPos+= nCharChars;
			}
		}
		return NULL;
	//	Mar. 4, 2001 genta	: comment out
	//}
}



/* 現在位置の文字の種類を調べる */
int	CDocLineMgr::WhatKindOfChar(
		char*	pData,
		int		pDataLen,
		int		nIdx
)
{
	int		nCharChars;
	WORD	wChar;
	nCharChars = CMemory::MemCharNext( pData, pDataLen, &pData[nIdx] ) - &pData[nIdx];
	if( nCharChars == 0 ){
		return CK_NULL;	/* NULL 0x0<=c<=0x0 */
	}else
	if( nCharChars == 1 ){	/* 半角文字 */
		if( pData[nIdx] == CR ){
			return CK_CR;	/* CR = 0x0d */
		}
		if( pData[nIdx] == LF ){
			return CK_LF;	/* LF = 0x0a */
		}
		if( pData[nIdx] == TAB ){
			return CK_TAB;	/* タブ 0x9<=c<=0x9 */
		}
		if( pData[nIdx] == SPACE ){
			return CK_SPACE;	/* 半角のスペース 0x20<=c<=0x20 */
		}
		if( __iscsym( pData[nIdx] ) ){
			return CK_CSYM;	/* 半角の英字、アンダースコア、数字のいずれか */
		}
		if( pData[nIdx] >= (char)0xa1 && pData[nIdx] <= (char)0xfd ){
			return CK_KATA;	/* 半角のカタカナ 0xA1<=c<=0xFD */
		}
		return CK_ETC;	/* 半角のその他 */

	}else
	if( nCharChars == 2 ){	/* 全角文字 */
		wChar =  (WORD)(unsigned char)(pData[nIdx + 1]) | (((WORD)(unsigned char)(pData[nIdx])) << 8);
//		MYTRACE( "wChar=%0xh\n", wChar );
		if( wChar == (WORD)0x8140 ){
			return CK_MBC_SPACE;	/* 2バイトのスペース */
		}
		if( wChar == (WORD)0x815B ){
			return CK_MBC_NOVASU;	/* 伸ばす記号 0x815B<=c<=0x815B 'ー' */
		}
		if( wChar == (WORD)0x8151 ||								/* 0x8151<=c<=0x8151 全角アンダースコア */
			(wChar >= (WORD)0x824F && wChar <= (WORD)0x8258 ) ||	/* 0x824F<=c<=0x8258 全角数字 */
			(wChar >= (WORD)0x8260 && wChar <= (WORD)0x8279 ) ||	/* 0x8260<=c<=0x8279 全角英大文字 */
			(wChar >= (WORD)0x8281 && wChar <= (WORD)0x829a )		/* 0x8281<=c<=0x829a 全角英小文字 */
		){
			return CK_MBC_CSYM;	/* 2バイトの英字、アンダースコア、数字のいずれか */
		}
		if( wChar >= (WORD)0x8140 && wChar <= (WORD)0x81FD ){
			return CK_MBC_KIGO;	/* 2バイトの記号 */
		}
		if( wChar >= (WORD)0x829F && wChar <= (WORD)0x82F1 ){
			return CK_MBC_HIRA;	/* 2バイトのひらがな */
		}
		if( wChar >= (WORD)0x8340 && wChar <= (WORD)0x8396 ){
			return CK_MBC_KATA;	/* 2バイトのカタカナ */
		}
		if( wChar >= (WORD)0x839F && wChar <= (WORD)0x83D6 ){
			return CK_MBC_GIRI;	/* 2バイトのギリシャ文字 */
		}
		if( ( wChar >= (WORD)0x8440 && wChar <= (WORD)0x8460 ) ||	/* 0x8440<=c<=0x8460 全角ロシア文字大文字 */
			( wChar >= (WORD)0x8470 && wChar <= (WORD)0x8491 ) ){	/* 0x8470<=c<=0x8491 全角ロシア文字小文字 */
			return CK_MBC_ROS;	/* 2バイトのロシア文字: */
		}
		if( wChar >= (WORD)0x849F && wChar <= (WORD)0x879C ){
			return CK_MBC_SKIGO;	/* 2バイトの特殊記号 */
		}
		return CK_MBC_ETC;	/* 2バイトのその他(漢字など) */
	}else{
		return CK_NULL;	/* NULL 0x0<=c<=0x0 */
	}
}





void CDocLineMgr::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE( "------------------------\n" );
	MYTRACE( "m_nLines=%d\n", m_nLines );
	MYTRACE( "m_pDocLineTop=%08lxh\n", m_pDocLineTop );
	MYTRACE( "m_pDocLineBot=%08lxh\n", m_pDocLineBot );

	CDocLine* pDocLine;
	CDocLine* pDocLineNext;
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLineNext = pDocLine->m_pNext;
		MYTRACE( "\t-------\n" );
		MYTRACE( "\tthis=%08lxh\n", pDocLine );
		MYTRACE( "\tpPrev; =%08lxh\n", pDocLine->m_pPrev );
		MYTRACE( "\tpNext; =%08lxh\n", pDocLine->m_pNext );

		MYTRACE( "\tm_enumEOLType =%s\n", pDocLine->m_cEol.GetName() );
		MYTRACE( "\tm_nEOLLen =%d\n", pDocLine->m_cEol.GetLen() );


//		MYTRACE( "\t[%s]\n",   (char*)*(pDocLine->m_pLine) );
		MYTRACE( "\tpDocLine->m_pLine->m_nDataLen=[%d]\n",   pDocLine->m_pLine->m_nDataLen );
		MYTRACE( "\t[%s]\n",   pDocLine->m_pLine->GetPtr( NULL ) );


		pDocLine = pDocLineNext;
	}
	MYTRACE( "------------------------\n" );
#endif
	return;
}

/* 行変更状態をすべてリセット */
/*

  ・変更フラグCDocLineオブジェクト作成時にはTRUEである
  ・変更回数はCDocLineオブジェクト作成時には1である

  ファイルを読み込んだときは変更フラグを FALSEにする
  ファイルを読み込んだときは変更回数を 0にする

  ファイルを上書きした時は変更フラグを FALSEにする
  ファイルを上書きした時は変更回数は変えない

  変更回数はUndoしたときに-1される
  変更回数が0になった場合は変更フラグをFALSEにする



*/
void CDocLineMgr::ResetAllModifyFlag( void )
//		BOOL bResetModifyCount /* 変更回数を0にするかどうか */
//)
{
	CDocLine* pDocLine;
	CDocLine* pDocLineNext;
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLineNext = pDocLine->m_pNext;
		pDocLine->m_bModify = FALSE;		/* 変更フラグ */
//		if( bResetModifyCount ){			/* 変更回数を0にするかどうか */
//			pDocLine->m_nModifyCount = 0;	/* 変更回数 */
//		}

		pDocLine = pDocLineNext;
	}
	return;
}


/* 全行データを返す */
char* CDocLineMgr::GetAllData( int*	pnDataLen )
{
	int			nDataLen;
	char*		pLine;
	int			nLineLen;
//	int			nAllocLen;
	CDocLine* 	pDocLine;
//	CDocLine* 	pDocLineNext;

	pDocLine = m_pDocLineTop;
//	pData = NULL;
	nDataLen = 0;
	while( NULL != pDocLine ){
		pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
		if( 0 < nLineLen &&
			( '\r' == pLine[nLineLen - 1] || '\n' == pLine[nLineLen - 1] )
		){
			nLineLen--;
		}
		if( 0 < nLineLen &&
			( '\r' == pLine[nLineLen - 1] || '\n' == pLine[nLineLen - 1] )
		){
			nLineLen--;
		}
		nDataLen+= nLineLen;
		nDataLen+= 2;
		pDocLine = pDocLine->m_pNext;
	}

	char*	pData;
	pData = (char*)malloc( nDataLen + 1 );
	if( NULL == pData ){
		::MYMESSAGEBOX(
			NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			"CDocLineMgr::GetAllData()\nメモリ確保に失敗しました。\n%dバイト",
			nDataLen + 1
		);
		return NULL;
	}
	pDocLine = m_pDocLineTop;
//	pData = NULL;
	nDataLen = 0;
	while( NULL != pDocLine ){
		pLine = pDocLine->m_pLine->GetPtr( &nLineLen );
		if( 0 < nLineLen &&
			( '\r' == pLine[nLineLen - 1] || '\n' == pLine[nLineLen - 1] )
		){
			nLineLen--;
		}
		if( 0 < nLineLen &&
			( '\r' == pLine[nLineLen - 1] || '\n' == pLine[nLineLen - 1] )
		){
			nLineLen--;
		}
		if( 0 <= nLineLen ){
			memcpy( &pData[nDataLen], pLine, nLineLen );
			nDataLen+= nLineLen;
		}
		memcpy( &pData[nDataLen], "\r\n", 2 );
		nDataLen+= 2;
		pDocLine = pDocLine->m_pNext;
	}
	pData[nDataLen] = '\0';
	*pnDataLen = nDataLen;
	return (char*)pData;
}


/* 行オブジェクトの削除、リスト変更、行数-- */
void CDocLineMgr::DeleteNode( CDocLine* pCDocLine )
{
	m_nLines--;	/* 全行数 */
	if( 0 == m_nLines ){
		/* データがなくなった */
		Init();
		return;
	}
	if( NULL == pCDocLine->m_pPrev ){
		m_pDocLineTop = pCDocLine->m_pNext;
	}else{
		pCDocLine->m_pPrev->m_pNext = pCDocLine->m_pNext;
	}
	if( NULL == pCDocLine->m_pNext ){
		m_pDocLineBot = pCDocLine->m_pPrev;
	}else{
		pCDocLine->m_pNext->m_pPrev = pCDocLine->m_pPrev;
	}
	if( m_pCodePrevRefer == pCDocLine ){
		m_pCodePrevRefer = pCDocLine->m_pNext;
	}
	delete pCDocLine;
	pCDocLine = NULL;


	return;
}



/* 行オブジェクトの挿入、リスト変更、行数++ */
/* pCDocLinePrevの次にpCDocLineを挿入する */
/* NULL==pCDocLinePrevのときリストの先頭に挿入 */
void CDocLineMgr::InsertNode( CDocLine* pCDocLinePrev, CDocLine* pCDocLine )
{
	pCDocLine->m_pPrev = pCDocLinePrev;
	if( NULL != pCDocLinePrev ){
		pCDocLine->m_pNext = pCDocLinePrev->m_pNext;
		pCDocLinePrev->m_pNext = pCDocLine;
	}else{
		pCDocLine->m_pNext = m_pDocLineTop;
		m_pDocLineTop = pCDocLine;
	}
	if( NULL != pCDocLine->m_pNext ){
		pCDocLine->m_pNext->m_pPrev = pCDocLine;
	}else{
		m_pDocLineBot = pCDocLine;
	}
	m_pDocLineTop = pCDocLine;
	m_nLines++;	/* 全行数 */
	return;

}


/*[EOF]*/
