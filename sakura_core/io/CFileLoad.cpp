/*!	@file
	@brief ファイル読み込みクラス

	@author Moca
	@date 2002/08/30 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca, genta
	Copyright (C) 2003, Moca, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "stdafx.h"
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "global.h"
#include "mem/CMemory.h"
#include "CEol.h"
#include "io/CFileLoad.h"
#include "charset/charcode.h"
#ifdef _DEBUG
#include "debug/Debug.h"
#endif
#include "io/CIoBridge.h"
#include "charset/CCodeMediator.h"
#include "util/string_ex2.h"
#include "charset/CESI.h"

/*
	@note Win32APIで実装
		2GB以上のファイルは開けない
*/

/*! ロード用バッファサイズの初期値 */
const int CFileLoad::gm_nBufSizeDef = 32768;
//(最適値がマシンによって違うのでとりあえず32KB確保する)

// /*! ロード用バッファサイズの設定可能な最低値 */
// const int gm_nBufSizeMin = 1024;

/*! コンストラクタ */
CFileLoad::CFileLoad( void )
{
	m_hFile			= NULL;
	m_nFileSize		= 0;
	m_nFileDataLen	= 0;
	m_CharCode		= CODE_DEFAULT;
	m_bBomExist		= FALSE;	// Jun. 08, 2003 Moca
	m_nFlag 		= 0;
	m_nReadLength	= 0;
	m_eMode			= FLMODE_CLOSE;	// Jun. 08, 2003 Moca

	m_nLineIndex	= -1;

	m_pReadBuf = NULL;
	m_nReadDataLen    = 0;
	m_nReadBufSize    = 0;
	m_nReadBufOffSet  = 0;
}

/*! デストラクタ */
CFileLoad::~CFileLoad( void )
{
	if( NULL != m_hFile ){
		FileClose();
	}
	if( NULL != m_pReadBuf ){
		free( m_pReadBuf );
	}
}

/*!
	ファイルを開く
	@param pFileName [in] ファイル名
	@param CharCode  [in] ファイルの文字コード．
	@param nFlag [in] 文字コードのオプション
	@param pbBomExist [out] BOMの有無
	@date 2003.06.08 Moca CODE_AUTODETECTを指定できるように変更
	@date 2003.07.26 ryoji BOM引数追加
*/
ECodeType CFileLoad::FileOpen( LPCTSTR pFileName, ECodeType CharCode, int nFlag, bool* pbBomExist )
{
	HANDLE	hFile;
	DWORD	FileSize;
	DWORD	FileSizeHigh;
	int		nBomCode;

	// FileCloseを呼んでからにしてください
	if( NULL != m_hFile ){
#ifdef _DEBUG
		::MessageBox( NULL, _T("CFileLoad::FileOpen\nFileCloseを呼んでからにしてください") , NULL, MB_OK );
#endif
		throw CError_FileOpen();
	}
	hFile = ::CreateFile(
		pFileName,
		GENERIC_READ,
		//	Oct. 18, 2002 genta FILE_SHARE_WRITE 追加
		//	他プロセスが書き込み中のファイルを開けるように
		FILE_SHARE_READ | FILE_SHARE_WRITE,	// 共有
		NULL,						// セキュリティ記述子
		OPEN_EXISTING,				// 作成方法
		FILE_FLAG_SEQUENTIAL_SCAN,	// ファイル属性
		NULL						// テンプレートファイルのハンドル
	);
	if( hFile == INVALID_HANDLE_VALUE ){
		throw CError_FileOpen();
	}
	m_hFile = hFile;

	FileSize = ::GetFileSize( hFile, &FileSizeHigh );
	// ファイルサイズが、約2GBを超える場合はとりあえずエラー
	if( 0x80000000 <= FileSize || 0 < FileSizeHigh ){
		FileClose();
		throw CError_FileOpen();
	}
	m_nFileSize = FileSize;
//	m_eMode = FLMODE_OPEN;

	// From Here Jun. 08, 2003 Moca 文字コード判定
	// データ読み込み
	Buffering();

	if( CharCode == CODE_AUTODETECT ){
		CharCode = CCodeMediator::CheckKanjiCode( m_pReadBuf, m_nReadDataLen );
	}
	// To Here Jun. 08, 2003
	// 不正な文字コードのときはデフォルト(SJIS:無変換)を設定
	if( 0 > CharCode || CODE_CODEMAX <= CharCode ){
		CharCode = CODE_SJIS;
	}
	m_CharCode = CharCode;
	m_nFlag = nFlag;

	// From Here Jun. 08, 2003 Moca BOMの除去
//-	nBomCode = CMemory::IsUnicodeBom( (const unsigned char*)m_pReadBuf, m_nReadDataLen );
	nBomCode = CCodeMediator::DetectUnicodeBom( m_pReadBuf, m_nReadDataLen );  // 2006.09.22  by rastiv
	m_nFileDataLen = m_nFileSize;
	if( nBomCode != 0 && nBomCode == m_CharCode ){
		//	Jul. 26, 2003 ryoji BOMの有無をパラメータで返す
		m_bBomExist = TRUE;
		if( pbBomExist != NULL ){
			*pbBomExist = true;
		}
		switch( nBomCode ){
			case CODE_UNICODE:
			case CODE_UNICODEBE:
				m_nFileDataLen -= 2;
				m_nReadBufOffSet += 2;
				break;
			case CODE_UTF8:
				m_nFileDataLen -= 3;
				m_nReadBufOffSet += 3;
				break;
		}
	}else{
		//	Jul. 26, 2003 ryoji BOMの有無をパラメータで返す
		if( pbBomExist != NULL ){
			*pbBomExist = false;
		}
	}
	
	// To Here Jun. 13, 2003 Moca BOMの除去
	m_eMode = FLMODE_REDY;
//	m_cmemLine.AllocBuffer( 256 );
	return m_CharCode;
}

/*!
	ファイルを閉じる
	読み込み用バッファとm_memLineもクリアされる
*/
void CFileLoad::FileClose( void )
{
	ReadBufEmpty();
	if( NULL != m_hFile ){
		::CloseHandle( m_hFile );
		m_hFile = NULL;
	}
	m_nFileSize		=  0;
	m_nFileDataLen	=  0;
	m_CharCode		= CODE_DEFAULT;
	m_bBomExist		= FALSE; // From Here Jun. 08, 2003
	m_nFlag 		=  0;
	m_nReadLength	=  0;
	m_eMode			= FLMODE_CLOSE;
	m_nLineIndex	= -1;
}

/*!
	次の論理行を文字コード変換してロードする
	順次アクセス専用
	GetNextLineのような動作をする
	@return	NULL以外	1行を保持しているデータの先頭アドレスを返す。永続的ではない一時的な領域。
			NULL		データがなかった
*/
EConvertResult CFileLoad::ReadLine(
	CNativeW*	pUnicodeBuffer,	//!< [out] UNICODEデータ受け取りバッファ。改行も含めて読み取る。
	CEol*		pcEol			//!< [i/o]
)
{
	EConvertResult eRet = RESULT_COMPLETE;

#ifdef _DEBUG
	if( m_eMode < FLMODE_REDY ){
		MYTRACE_A( "CFileLoad::ReadLine(): m_eMode = %d\n", m_eMode );
		return RESULT_FAILURE;
	}
#endif
	//行データバッファ (文字コード変換無しの生のデータ)
	static CMemory cLineBuffer;
	cLineBuffer.SetRawData("",0);

	// 1行取り出し ReadBuf -> m_memLine
	//	Oct. 19, 2002 genta while条件を整理
	int			nBufLineLen;
	int			nEolLen;
	while(1){
		const char* pLine = GetNextLineCharCode(
			m_pReadBuf,
			m_nReadDataLen,    //[in] 改行を含む長さ
			&nBufLineLen,      //[out]改行を含まない長さ
			&m_nReadBufOffSet, //[i/o]オフセット
			pcEol,
			&nEolLen
		);
		if(pLine==NULL)break;

		// ReadBufから1行を取得するとき、改行コードが欠ける可能性があるため
		if( m_nReadDataLen <= m_nReadBufOffSet && FLMODE_REDY == m_eMode ){// From Here Jun. 13, 2003 Moca
			cLineBuffer.AppendRawData( pLine, nBufLineLen );
			m_nReadBufOffSet -= nEolLen;
			// バッファロード   File -> ReadBuf
			Buffering();
		}else{
			cLineBuffer.AppendRawData( pLine, nBufLineLen );
			break;
		}
	}
	m_nReadLength += ( nBufLineLen = cLineBuffer.GetRawLength() );

	// 文字コード変換 cLineBuffer -> pUnicodeBuffer
	EConvertResult eConvertResult = CIoBridge::FileToImpl(cLineBuffer,pUnicodeBuffer,m_CharCode,m_nFlag);
	if(eConvertResult==RESULT_LOSESOME){
		eRet = RESULT_LOSESOME;
	}

	m_nLineIndex++;
	// データあり
	if( 0 != nBufLineLen + nEolLen ){
		// 改行コードを追加
		pUnicodeBuffer->AppendString( pcEol->GetValue2(), pcEol->GetLen() );
	}
	else{
		eRet = RESULT_FAILURE;
	}

	//staticバッファを解放しておく
	cLineBuffer.SetRawData("",0);

	return eRet;
}



/*!
	バッファにデータを読み込む
	@note エラー時は throw する
*/
void CFileLoad::Buffering( void )
{
	DWORD	ReadSize;

	// メモリー確保
	if( NULL == m_pReadBuf ){
		int nBufSize;
		nBufSize = ( m_nFileSize < gm_nBufSizeDef )?( m_nFileSize ):( gm_nBufSizeDef );
		//	Borland C++では0バイトのmallocを獲得失敗と見なすため
		//	最低1バイトは取得することで0バイトのファイルを開けるようにする
		if( 0 >= nBufSize ){
			nBufSize = 1; // Jun. 08, 2003  BCCのmalloc(0)がNULLを返す仕様に対処
		}

		m_pReadBuf = (char *)malloc( nBufSize );
		if( NULL == m_pReadBuf ){
			throw CError_FileRead(); // メモリー確保に失敗
		}
		m_nReadDataLen = 0;
		m_nReadBufSize = nBufSize;
		m_nReadBufOffSet = 0;
	}
	// ReadBuf内にデータが残っている
	else if( m_nReadBufOffSet < m_nReadDataLen ){
		m_nReadDataLen -= m_nReadBufOffSet;
		memmove( m_pReadBuf, &m_pReadBuf[m_nReadBufOffSet], m_nReadDataLen );
		m_nReadBufOffSet = 0;
	}
	else{
		m_nReadBufOffSet = 0;
		m_nReadDataLen = 0;
	}
	// ファイルの読み込み
	ReadSize = Read( &m_pReadBuf[m_nReadDataLen], m_nReadBufSize - m_nReadDataLen );
	if( 0 == ReadSize ){
		m_eMode = FLMODE_READBUFEND;	// ファイルなどの終わりに達したらしい
	}
	m_nReadDataLen += ReadSize;
}

/*!
	バッファクリア
*/
void CFileLoad::ReadBufEmpty( void )
{
	if ( NULL != m_pReadBuf ){
		free( m_pReadBuf );
		m_pReadBuf = NULL;
	}
	m_nReadDataLen    = 0;
	m_nReadBufSize    = 0;
	m_nReadBufOffSet  = 0;
}


/*!
	 現在の進行率を取得する
	 @return 0% - 100%  若干誤差が出る
*/
int CFileLoad::GetPercent( void ){
	int nRet;
	if( 0 == m_nFileDataLen || m_nReadLength > m_nFileDataLen ){
		nRet = 100;
	}else if(  0x10000 > m_nFileDataLen ){
		nRet = m_nReadLength * 100 / m_nFileDataLen ;
	}else{
		nRet = m_nReadLength / 128 * 100 / ( m_nFileDataLen / 128 );
	}
	return nRet;
}

/*!
	GetNextLineの汎用文字コード版
*/
const char* CFileLoad::GetNextLineCharCode(
	const char*	pData,		//!< [in]	検索文字列
	int			nDataLen,	//!< [in]	検索文字列のバイト数
	int*		pnLineLen,	//!< [out]	1行のバイト数を返すただしEOLは含まない
	int*		pnBgn,		//!< [i/o]	検索文字列のバイト単位のオフセット位置
	CEol*		pcEol,		//!< [i/o]	EOL
	int*		pnEolLen	//!< [out]	EOLのバイト数 (Unicodeで困らないように)
){
	const char *pRetStr;
	switch( m_CharCode ){
	case CODE_UNICODE:
		*pnBgn /= sizeof( wchar_t );
		pRetStr = (const char*)GetNextLineW(
			(const wchar_t*)pData,
			nDataLen / sizeof( wchar_t ),
			pnLineLen,
			pnBgn,
			pcEol );
		*pnLineLen *= sizeof( wchar_t );
		*pnBgn     *= sizeof( wchar_t );
		*pnEolLen   = (Int)pcEol->GetLen() * sizeof( wchar_t );
		break;

	case CODE_UNICODEBE:
		*pnBgn /= sizeof( wchar_t );
		pRetStr = (const char*)GetNextLineWB(
			(const wchar_t*)pData,
			nDataLen / sizeof( wchar_t ),
			pnLineLen,
			pnBgn,
			pcEol );
		*pnLineLen *= sizeof( wchar_t );
		*pnBgn     *= sizeof( wchar_t );
		*pnEolLen   = (Int)pcEol->GetLen() * sizeof( wchar_t );
		break;

	default:
		pRetStr = GetNextLine( pData, nDataLen, pnLineLen, pnBgn, pcEol );
		*pnEolLen = (Int)pcEol->GetLen();
		break;
	}
	return pRetStr;
}



