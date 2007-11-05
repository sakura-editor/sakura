/*!	@file
	@brief 行データの管理

	@author Norio Nakatani
	@date 1998/03/05  新規作成
	@date 2001/06/23 N.Nakatani 単語単位で検索する機能を実装
	@date 2001/06/23 N.Nakatani WhereCurrentWord()変更 WhereCurrentWord_2をコールするようにした
	@date 2005/09/25 D.S.Koba GetSizeOfCharで書き換え
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, ao
	Copyright (C) 2001, genta, jepro, hor
	Copyright (C) 2002, hor, aroka, MIK, Moca, genta, frozen, Azumaiya, YAZAKI
	Copyright (C) 2003, Moca, ryoji, genta, かろと
	Copyright (C) 2004, genta, Moca
	Copyright (C) 2005, D.S.Koba, ryoji, かろと

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

/* for TRACE() of MFC */
//#ifdef _DEBUG
//	#include <afx.h>
//#endif
//#ifndef _DEBUG
//	#include <windows.h>
//#endif


#include "stdafx.h"
#include "charcode.h"
#include "CDocLineMgr.h"
#include "debug.h"
#include "charcode.h"
// Oct 6, 2000 ao
#include <stdio.h>
#include <io.h>
//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
#include "CBregexp.h"
#include <commctrl.h>
#include "global.h"
#include "CRunningTimer.h"

//	May 15, 2000 genta
#include "CEol.h"
#include "CDocLine.h"// 2002/2/10 aroka ヘッダ整理
#include "CMemory.h"// 2002/2/10 aroka

#include "CFileWrite.h" //2002/05/22 Frozen
#include "CFileLoad.h" // 2002/08/30 Moca
#include "my_icmp.h" // Nov. 29, 2002 genta/moca
#include "CIoBridge.h"
#include "basis/SakuraBasis.h"
#include "parse/CWordParse.h"
#include "util/window.h"
#include "util/file.h"

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
	m_nLines = CLogicInt(0);
	m_nPrevReferLine = CLogicInt(0);
	m_pCodePrevRefer = NULL;
	m_bIsDiffUse = false;	/* DIFF使用中 */	//@@@ 2002.05.25 MIK
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





const wchar_t* CDocLineMgr::GetLineStr( CLogicInt nLine, CLogicInt* pnLineLen )
{
	CDocLine* pDocLine;
	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		*pnLineLen = CLogicInt(0);
		return NULL;
	}
	// 2002/2/10 aroka CMemory のメンバ変数に直接アクセスしない(inline化されているので速度的な問題はない)
	return pDocLine->m_cLine.GetStringPtr( pnLineLen );
}

/*!
	指定された行番号の文字列と改行コードを除く長さを取得
	
	@author Moca
	@date 2003.06.22
*/
const wchar_t* CDocLineMgr::GetLineStrWithoutEOL( CLogicInt nLine, int* pnLineLen )
{
	const CDocLine* pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		*pnLineLen = 0;
		return NULL;
	}
	*pnLineLen = pDocLine->GetLengthWithoutEOL();
	return pDocLine->m_cLine.GetStringPtr();
}

/*!
	指定された番号の行へのポインタを返す

	@param nLine [in] 行番号
	@return 行オブジェクトへのポインタ。該当行がない場合はNULL。
*/
CDocLine* CDocLineMgr::GetLineInfo( CLogicInt nLine )
{
	CLogicInt nCounter;
	CDocLine* pDocLine;
	if( CLogicInt(0) == m_nLines ){
		return NULL;
	}
	// 2004.03.28 Moca nLineが負の場合のチェックを追加
	if( CLogicInt(0) > nLine || nLine >= m_nLines ){
		return NULL;
	}
	// 2004.03.28 Moca m_pCodePrevReferより、Top,Botのほうが近い場合は、そちらを利用する
	CLogicInt nPrevToLineNumDiff = t_abs( m_nPrevReferLine - nLine );
	if( m_pCodePrevRefer == NULL
	  || nLine < nPrevToLineNumDiff
	  || m_nLines - nLine < nPrevToLineNumDiff
	){
		if( m_pCodePrevRefer == NULL ){
			MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::GetLineInfo() 	m_pCodePrevRefer == NULL" );
		}

		if( nLine < (m_nLines / 2) ){
			nCounter = CLogicInt(0);
			pDocLine = m_pDocLineTop;
			while( pDocLine ){
				if( nLine == nCounter ){
					m_nPrevReferLine = nLine;
					m_pCodePrevRefer = pDocLine;
					m_pDocLineCurrent = pDocLine->m_pNext;
					return pDocLine;
				}
				pDocLine = pDocLine->m_pNext;
				nCounter++;
			}
		}
		else{
			nCounter = m_nLines - CLogicInt(1);
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

	}
	else{
		if( nLine == m_nPrevReferLine ){
			m_nPrevReferLine = nLine;
			m_pDocLineCurrent = m_pCodePrevRefer->m_pNext;
			return m_pCodePrevRefer;
		}
		else if( nLine > m_nPrevReferLine ){
			nCounter = m_nPrevReferLine + CLogicInt(1);
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
		}
		else{
			nCounter = m_nPrevReferLine - CLogicInt(1);
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
const wchar_t* CDocLineMgr::GetFirstLinrStr( int* pnLineLen )
{
	const wchar_t* pszLine;
	if( CLogicInt(0) == m_nLines ){
		pszLine = NULL;
		*pnLineLen = 0;
	}else{
		pszLine = m_pDocLineTop->m_cLine.GetStringPtr( pnLineLen );

		m_pDocLineCurrent = m_pDocLineTop->m_pNext;
	}
	return pszLine;
}





/*!
	順アクセスモード：次の行を得る

	@param pnLineLen [out] 行の長さが返る。
	@return 次行の先頭へのポインタ。
	GetFirstLinrStr()が呼び出されていないとNULLが返る

*/
const wchar_t* CDocLineMgr::GetNextLinrStr( int* pnLineLen )
{
	const wchar_t* pszLine;
	if( NULL == m_pDocLineCurrent ){
		pszLine = NULL;
		*pnLineLen = 0;
	}else{
		pszLine = m_pDocLineCurrent->m_cLine.GetStringPtr( pnLineLen );

		m_pDocLineCurrent = m_pDocLineCurrent->m_pNext;
	}
	return pszLine;
}



/*!
	末尾に行を追加

	@version 1.5

	@param pData    [in] 追加する文字列へのポインタ
	@param nDataLen [in] 文字列の長さ。文字単位。
	@param cEol     [in] 行末コード

*/


void CDocLineMgr::AddLineStrX( const wchar_t* pData, int nDataLen, CEOL cEol )
{
#ifdef _DEBUG
//	CRunningTimer cRunningTimer( "CDocLineMgr::AddLineStrX(const char*, int, int)" );
#endif

	//チェーン適用
	CDocLine* pDocLine = new CDocLine;
	_PushBottom(pDocLine);

	//インスタンス設定
	pDocLine->m_cEol = cEol;	/* 改行コードの種類 */
	pDocLine->m_cLine.SetString(pData, nDataLen);
}

/*!
	ファイルを読み込んで格納する（分割読み込みテスト版）
	@version	2.0
	@note	Windows用にコーディングしてある
	@retval	TRUE	正常読み込み
	@retval	FALSE	エラー(またはユーザによるキャンセル?)
	@date	2002/08/30 Moca 旧ReadFileを元に作成 ファイルアクセスに関する部分をCFileLoadで行う
	@date	2003/07/26 ryoji BOMの状態の取得を追加
*/
int CDocLineMgr::ReadFile(
	const TCHAR*		pszPath,		//!<
	HWND				hWndParent,		//!< [in] 親ウィンドウのハンドル
	HWND				hwndProgress,	//!< [in] Progress barのウィンドウハンドル
	ECodeType			nCharCode,		//!<
	FILETIME*			pFileTime,		//!<
	int					nFlags,			//!< [in] bit 0: MIME Encodeされたヘッダをdecodeするかどうか
	BOOL*				pbBomExist		//!<
)
{
#ifdef _DEBUG
	MYTRACE_A( "pszPath=[%hs]\n", pszPath );
	MY_RUNNINGTIMER( cRunningTimer, "CDocLineMgr::ReadFile" );
#endif
	/* 既存データのクリア */
	Empty();
	Init();

	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( NULL ) ){
		return FALSE;
	}

	int nRetVal = TRUE;

	try{
		CFileLoad cfl;

		// ファイルを開く
		// ファイルを閉じるにはFileCloseメンバ又はデストラクタのどちらかで処理できます
		//	Jul. 28, 2003 ryoji BOMパラメータ追加
		cfl.FileOpen( pszPath, nCharCode, nFlags, pbBomExist );

		/* ファイル時刻の取得 */
		FILETIME	FileTime;
		if( cfl.GetFileTime( NULL, NULL, &FileTime ) ){
			*pFileTime = FileTime;
		}

		//プログレスバー初期化
		if( NULL != hwndProgress ){
			::PostMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
			::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
		}

		// ReadLineはファイルから 文字コード変換された1行を読み出します
		// エラー時はthrow CError_FileRead を投げます
		const wchar_t*	pLine;
		int				nLineLen;
		int				nLineNum = 0;
		CEOL			cEol;
		CNativeW2		cUnicodeBuffer;
		while( NULL != ( pLine = cfl.ReadLine( &cUnicodeBuffer, &nLineLen, &cEol ) ) ){
			++nLineNum;
			AddLineStrX( pLine, nLineLen, cEol );
			//512行毎にプログレスバーを更新
			if( NULL != hwndProgress && 0 == ( nLineNum % 512 ) ){
				::PostMessageAny( hwndProgress, PBM_SETPOS, cfl.GetPercent(), 0 );
				/* 処理中のユーザー操作を可能にする */
				if( !::BlockingHook( NULL ) ){
					return FALSE;
				}
			}
		}

		// ファイルをクローズする
		cfl.FileClose();
	}
	catch( CError_FileOpen ){
		nRetVal = FALSE;
		if( -1 == _taccess( pszPath, 0 )){
			// ファイルがない
			::MYMESSAGEBOX(
				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("%ls\nというファイルを開けません。\nファイルが存在しません。"),	//Mar. 24, 2001 jepro 若干修正
				pszPath
			 );
		}
		else if( -1 == _taccess( pszPath, 4 )){
			// 読み込みアクセス権がない
			::MYMESSAGEBOX(
				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%ts\'\nというファイルを開けません。\n読み込みアクセス権がありません。"),
				pszPath
			 );
		}
		else{
			::MYMESSAGEBOX(
				hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%ts\'\nというファイルを開けません。\n他のアプリケーションで使用されている可能性があります。"),
				pszPath
			 );
		}
	}
	catch( CError_FileRead ){
		nRetVal = FALSE;
		::MYMESSAGEBOX(
			hWndParent, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("\'%ts\'というファイルの読み込み中にエラーが発生しました。\nファイルの読み込みを中止します。"),
			pszPath
		 );
		/* 既存データのクリア */
		Empty();
		Init();
		nRetVal = FALSE;
	} // 例外処理終わり

	if( NULL != hwndProgress ){
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
	}
	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( NULL ) ){
		return FALSE;
	}

	/* 行変更状態をすべてリセット */
	ResetAllModifyFlag();
	return nRetVal;
}




/*! バッファ内容をファイルに書き出す (テスト用)

	@param cEol [in]		使用する改行コード
	@param bBomExist [in]	ファイル先頭にBOMを付けるか

	@note Windows用にコーディングしてある
	@date 2003.07.26 ryoji BOM引数追加
*/
EConvertResult CDocLineMgr::WriteFile(
	const TCHAR*	pszPath,
	HWND			hWndParent,
	HWND			hwndProgress,
	ECodeType		nCharCode,
	FILETIME*		pFileTime,
	CEOL			cEol,
	BOOL			bBomExist
)
{
	int				nLineNumber;
	int				nWriteLen;
	CDocLine*		pCDocLine;


	if( NULL != hwndProgress ){
		::PostMessageAny( hwndProgress, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
	}

	EConvertResult		nRetVal = RESULT_COMPLETE;

	try
	{
		CFileWrite file(pszPath);// 2002/05/22 Frozen

		//	Jul. 26, 2003 ryoji bBomExitによってBOMを付けるかどうかを決める
		if (bBomExist) {
			switch( nCharCode ){
			case CODE_UNICODE:
				file.WriteSz( "\xff\xfe" );
				break;
			case CODE_UNICODEBE:
				file.WriteSz( "\xfe\xff" );
				break;
			case CODE_UTF8: // 2003.05.04 Moca BOMの間違いを訂正
				file.WriteSz( "\xef\xbb\xbf" );
				break;
			default:
				//	genta ここに来るのはバグだ
				;
			}
		}

		nLineNumber = 0;
		pCDocLine = m_pDocLineTop;

		//ファイル書込用バッファ(この中で文字コード変換を行う)

		while( NULL != pCDocLine ){
			++nLineNumber;
			int				nLineLen;
			const wchar_t*	pLine = pCDocLine->m_cLine.GetStringPtr( &nLineLen );

			//1024行書き込むごとにプログレスバーを更新する
			if( NULL != hwndProgress && CLogicInt(0) < m_nLines && 0 == ( nLineNumber % 1024 ) ){
				::PostMessageAny( hwndProgress, PBM_SETPOS, nLineNumber * 100 / m_nLines , 0 );
				/* 処理中のユーザー操作を可能にする */
				if( !::BlockingHook( NULL ) ){
					return RESULT_FAILURE;
				}
			}

			//1行出力 -> cFileCodeBuffer
			nWriteLen = nLineLen - pCDocLine->m_cEol.GetLen();
			CMemory cFileCodeBuffer;
			if( 0 < nWriteLen ){
				CNativeW2 cmemBuf( pLine, nWriteLen );

				// 書き込み時のコード変換 cmemBuf -> cFileCodeBuffer
				EConvertResult e = CIoBridge::ImplToFile(&cmemBuf,&cFileCodeBuffer,nCharCode);
				if(e==RESULT_LOSESOME){
					if(nRetVal==RESULT_COMPLETE)nRetVal=RESULT_LOSESOME;
				}

				cmemBuf.SetString(L"",0);
			}

			//改行出力 -> cFileCodeBuffer
			if( EOL_NONE != pCDocLine->m_cEol ){

// 2002/05/09 Frozen ここから
				if( nCharCode == CODE_UNICODE ){
					if( cEol==EOL_NONE )
						cFileCodeBuffer.AppendRawData( pCDocLine->m_cEol.GetUnicodeValue(), (Int)pCDocLine->m_cEol.GetLen()*sizeof(wchar_t));
					else
						cFileCodeBuffer.AppendRawData( cEol.GetUnicodeValue(), (Int)cEol.GetLen()*sizeof(wchar_t));
				}else if( nCharCode == CODE_UNICODEBE ){
					/* UnicodeBE の改行コード設定 Moca, 2002/05/26 */
					if( cEol == EOL_NONE ) /*  */
						cFileCodeBuffer.AppendRawData( pCDocLine->m_cEol.GetUnicodeBEValue(), (Int)pCDocLine->m_cEol.GetLen()*sizeof(wchar_t) );
					else
						cFileCodeBuffer.AppendRawData( cEol.GetUnicodeBEValue(), (Int)cEol.GetLen()*sizeof(wchar_t) );
				}else{
					if( cEol == EOL_NONE )
						cFileCodeBuffer.AppendRawData(pCDocLine->m_cEol.GetValue(), (Int)pCDocLine->m_cEol.GetLen());
					else
						cFileCodeBuffer.AppendRawData(cEol.GetValue(), (Int)cEol.GetLen());
				}
// 2002/05/09 Frozen ここまで

			}

			//cFileCodeBufferを実際に出力 (ここで初めてファイル書込が発生する)
			if( 0 < cFileCodeBuffer.GetRawLength() )//{
				file.Write(cFileCodeBuffer.GetRawPtr(),cFileCodeBuffer.GetRawLength());//2002/05/22 Frozen gotoの次の}までをこの一行で置き換え

			//次の行へ
			pCDocLine = pCDocLine->m_pNext;

		}

		file.Close();


		/* 更新後のファイル時刻の取得
		 * CloseHandle前ではFlushFileBuffersを呼んでもタイムスタンプが更新
		 * されないことがある。
		 */

		// 2005.10.20 ryoji FindFirstFileを使うように変更（ファイルがロックされていてもタイムスタンプ取得可能）
		FILETIME ftime;
		if( GetLastWriteTimestamp( pszPath, ftime )){
			*pFileTime = ftime;
		}

	}
	catch(CError_FileOpen)
	{
		::MYMESSAGEBOX_A(
			hWndParent,
			MB_OK | MB_ICONSTOP,
			GSTR_APPNAME_A,
			"\'%ts\'\n"
			"ファイルを保存できません。\n"
			"パスが存在しないか、他のアプリケーションで使用されている可能性があります。",
			pszPath
		);
		nRetVal = RESULT_FAILURE;
	}
	catch(CError_FileWrite)
	{
		nRetVal = RESULT_FAILURE;
	}

//_RETURN_:; 2002/05/22 Frozen 削除（例外処理を使用するのでgoto用のラベルは使用しない）
	if( NULL != hwndProgress ){
		::PostMessageAny( hwndProgress, PBM_SETPOS, 0, 0 );
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( NULL ) ){
			return RESULT_FAILURE;
		}
	}

	return nRetVal;
}





/* データの削除 */
/*
|| 指定行内の文字しか削除できません
|| データ変更によって影響のあった、変更前と変更後の行の範囲を返します
|| この情報をもとに、レイアウト情報などを更新してください。
||
	@date 2002/03/24 YAZAKI bUndo削除
*/
void CDocLineMgr::DeleteData_CDocLineMgr(
	CLogicInt	nLine,
	CLogicInt	nDelPos,
	CLogicInt	nDelLen,
	CLogicInt*	pnModLineOldFrom,	//!< 影響のあった変更前の行(from)
	CLogicInt*	pnModLineOldTo,		//!< 影響のあった変更前の行(to)
	CLogicInt*	pnDelLineOldFrom,	//!< 削除された変更前論理行(from)
	CLogicInt*	pnDelLineOldNum,	//!< 削除された行数
	CNativeW2*	cmemDeleted			//!< [out] 削除されたデータ
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CDocLineMgr::DeleteData" );
#endif
	CDocLine*	pDocLine;
	CDocLine*	pDocLine2;
	wchar_t*	pData;
	CLogicInt	nDeleteLength;
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	const wchar_t*	pLine2;
	CLogicInt		nLineLen2;

	*pnModLineOldFrom = nLine;	/* 影響のあった変更前の行(from) */
	*pnModLineOldTo = nLine;	/* 影響のあった変更前の行(to) */
	*pnDelLineOldFrom = CLogicInt(0);		/* 削除された変更前論理行(from) */
	*pnDelLineOldNum = CLogicInt(0);		/* 削除された行数 */
//	cmemDeleted.SetData( "", lstrlen( "" ) );
	cmemDeleted->Clear();

	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		return;
	}

	pDocLine->SetModifyFlg(true);		/* 変更フラグ */

	pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );

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
		cmemDeleted->SetString( &pLine[nDelPos], nDeleteLength );

		/* 次の行の情報 */
		if( NULL == ( pDocLine2 = pDocLine->m_pNext ) ){
			pData = new wchar_t[nLineLen + 1];
			if( nDelPos > 0 ){
				wmemcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				wmemcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			pData[ nLineLen - nDeleteLength ] = L'\0';
			/* 改行コードの情報を更新 */
			pDocLine->m_cEol.SetType( EOL_NONE );

			if( 0 < nLineLen - nDeleteLength ){
				pDocLine->m_cLine.SetString( pData, nLineLen - nDeleteLength );
			}else{
				// 行の削除
				// 2004.03.18 Moca 関数を使う
				DeleteNode( pDocLine );
				pDocLine = NULL;
				*pnDelLineOldFrom = nLine;	/* 削除された変更前論理行(from) */
				*pnDelLineOldNum = CLogicInt(1);		/* 削除された行数 */
			}
			delete [] pData;
		}
		else{
			*pnModLineOldTo = nLine + CLogicInt(1);	/* 影響のあった変更前の行(to) */
			pLine2 = pDocLine2->m_cLine.GetStringPtr( &nLineLen2 );
			pData = new wchar_t[nLineLen + nLineLen2 + 1];
			if( nDelPos > 0 ){
				wmemcpy( pData, pLine, nDelPos );
			}
			if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
				wmemcpy(
					pData + nDelPos,
					pLine + nDelPos + nDeleteLength,
					nLineLen - ( nDelPos + nDeleteLength )
				);
			}
			/* 次の行のデータを連結 */
			wmemcpy( pData + (nLineLen - nDeleteLength), pLine2, nLineLen2 );
			pData[ nLineLen - nDeleteLength + nLineLen2 ] = L'\0';
			pDocLine->m_cLine.SetString( pData, nLineLen - nDeleteLength + nLineLen2 );
			/* 改行コードの情報を更新 */
			pDocLine->m_cEol = pDocLine2->m_cEol;

			/* 次の行を削除 && 次次行とのリストの連結*/
			// 2004.03.18 Moca DeleteNode を使う
			DeleteNode( pDocLine2 );
			pDocLine2 = NULL;
			*pnDelLineOldFrom = nLine + CLogicInt(1);	/* 削除された変更前論理行(from) */
			*pnDelLineOldNum = CLogicInt(1);			/* 削除された行数 */
			delete [] pData;
		}
	}
	else{
		/* 実際に削除するバイト数 */
		nDeleteLength = nDelLen;

		/* 削除されるデータ */
		cmemDeleted->SetString( &pLine[nDelPos], nDeleteLength );

		pData = new wchar_t[nLineLen + 1];
		if( nDelPos > 0 ){
			wmemcpy( pData, pLine, nDelPos );
		}
		if( 0 < nLineLen - ( nDelPos + nDeleteLength ) ){
			wmemcpy(
				pData + nDelPos,
				pLine + nDelPos + nDeleteLength,
				nLineLen - ( nDelPos + nDeleteLength )
			);
		}
		pData[ nLineLen - nDeleteLength ] = L'\0';
		if( 0 < nLineLen - nDeleteLength ){
			pDocLine->m_cLine.SetString( pData, nLineLen - nDeleteLength );
		}else{
		}
		delete [] pData;
	}
//	DUMP();
	return;
}





/*!	データの挿入

	@date 2002/03/24 YAZAKI bUndo削除
*/
void CDocLineMgr::InsertData_CDocLineMgr(
	CLogicInt		nLine,
	CLogicInt		nInsPos,
	const wchar_t*	pInsData,
	CLogicInt		nInsDataLen,
	CLogicInt*		pnInsLineNum,	// 挿入によって増えた行の数
	CLogicPoint*	pptNewPos		// 挿入された部分の次の位置
	/*
	CLogicInt*		pnNewLine,		// 挿入された部分の次の位置の行
	CLogicInt*		pnNewPos		// 挿入された部分の次の位置のデータ位置
	*/
)
{
	CDocLine*	pDocLine;
	CNativeW2	cmemPrevLine;
	CNativeW2	cmemCurLine;
	CNativeW2	cmemNextLine;
	CLogicInt	nAllLinesOld = m_nLines;
	CEOL 		cEOLTypeNext;

	bool		bBookMarkNext;	// 2001.12.03 hor 挿入によるマーク行の制御

	pptNewPos->y = nLine;	/* 挿入された部分の次の位置の行 */

	//	Jan. 25, 2004 genta
	//	挿入文字列長が0の場合に最後までpnNewPosが設定されないので
	//	初期値として0ではなく開始位置と同じ値を入れておく．
	pptNewPos->x  = nInsPos;		/* 挿入された部分の次の位置のデータ位置 */

	/* 挿入データを行終端で区切った行数カウンタ */
	*pnInsLineNum = CLogicInt(0);
	pDocLine = GetLineInfo( nLine );
	if( NULL == pDocLine ){
		/* ここでNULLが帰ってくるということは、*/
		/* 全テキストの最後の次の行を追加しようとしていることを示す */
		cmemPrevLine.SetString(L"");
		cmemNextLine.SetString(L"");
		cEOLTypeNext.SetType( EOL_NONE );
		bBookMarkNext=false;	// 2001.12.03 hor
	}
	else{
		pDocLine->SetModifyFlg(true);		/* 変更フラグ */

		CLogicInt		nLineLen;
		const wchar_t*	pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
		cmemPrevLine.SetString( pLine, nInsPos );
		cmemNextLine.SetString( &pLine[nInsPos], nLineLen - nInsPos );

		cEOLTypeNext = pDocLine->m_cEol;
		bBookMarkNext= pDocLine->IsBookMarked();	// 2001.12.03 hor
	}

	int			nCount = 0;
	CLogicInt	nBgn   = CLogicInt(0);
	CLogicInt	nPos   = CLogicInt(0);
	CEOL 		cEOLType;
	for( nPos = CLogicInt(0); nPos < nInsDataLen; ){
		if( pInsData[nPos] == L'\n' || pInsData[nPos] == L'\r' ){
			/* 行終端子の種類を調べる */
			cEOLType.GetTypeFromString( &pInsData[nPos], nInsDataLen - nPos );
			/* 行終端子も含めてテキストをバッファに格納 */
			cmemCurLine.SetString( &pInsData[nBgn], nPos - nBgn + cEOLType.GetLen() );
			nBgn = nPos + CLogicInt(cEOLType.GetLen());
			nPos = nBgn;
			if( NULL == pDocLine ){
				CDocLine* pDocLineNew = new CDocLine;

				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					this->_PushBottom(pDocLineNew);

					pDocLineNew->m_cLine.SetNativeData( cmemPrevLine );
					pDocLineNew->m_cLine += cmemCurLine;

					pDocLineNew->m_cEol = cEOLType;							/* 改行コードの種類 */
				}
				else{
					this->_PushBottom(pDocLineNew);

					pDocLineNew->m_cLine.SetNativeData( cmemCurLine );

					pDocLineNew->m_cEol = cEOLType;							/* 改行コードの種類 */
				}
				pDocLine = NULL;
			}else{
				/* 挿入データを行終端で区切った行数カウンタ */
				if( 0 == nCount ){
					pDocLine->m_cLine.SetNativeData( cmemPrevLine );
					pDocLine->m_cLine += cmemCurLine;

					pDocLine->m_cEol = cEOLType;						/* 改行コードの種類 */

					// 2001.12.13 hor
					// 行頭で改行したら元の行のマークを新しい行に移動する
					// それ以外なら元の行のマークを維持して新しい行にはマークを付けない
					if(nInsPos==CLogicInt(0)){
						pDocLine->SetBookMark(false);
					}
					else{
						bBookMarkNext=false;
					}

					pDocLine = pDocLine->m_pNext;
				}else{
					CDocLine* pDocLineNew = new CDocLine;

					_Insert(pDocLineNew, pDocLine); //pDocLineの直前に挿入
					
					pDocLineNew->m_cLine.SetNativeData( cmemCurLine );

					pDocLineNew->m_cEol = cEOLType;							/* 改行コードの種類 */
				}
			}

			/* 挿入データを行終端で区切った行数カウンタ */
			++nCount;
			++pptNewPos->y;	/* 挿入された部分の次の位置の行 */
		}else{
			++nPos;
		}
	}
//	nEOLType = EOL_NONE;
	if( CLogicInt(0) < nPos - nBgn || 0 < cmemNextLine.GetStringLength() ){
		cmemCurLine.SetString( &pInsData[nBgn], nPos - nBgn );
		cmemCurLine += cmemNextLine;
		if( NULL == pDocLine ){
			CDocLine* pDocLineNew = new CDocLine;
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				_PushBottom(pDocLineNew);

				pDocLineNew->m_cLine.SetNativeData( cmemPrevLine );
				pDocLineNew->m_cLine += cmemCurLine;

				pDocLineNew->m_cEol = cEOLTypeNext;							/* 改行コードの種類 */

			}else{
				_PushBottom(pDocLineNew);

				pDocLineNew->m_cLine.SetNativeData( cmemCurLine );

				pDocLineNew->m_cEol = cEOLTypeNext;							/* 改行コードの種類 */

			}
			pDocLine = NULL;
			pptNewPos->x = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
		}else{
			/* 挿入データを行終端で区切った行数カウンタ */
			if( 0 == nCount ){
				pDocLine->m_cLine.SetNativeData( cmemPrevLine );
				pDocLine->m_cLine += cmemCurLine;

				pDocLine->m_cEol = cEOLTypeNext;						/* 改行コードの種類 */
				// pDocLine->m_nEOLLen = gm_pnEolLenArr[nEOLTypeNext];	/* 改行コードの長さ */

				pDocLine = pDocLine->m_pNext;
				pptNewPos->x = CLogicInt(cmemPrevLine.GetStringLength()) + nPos - nBgn;		/* 挿入された部分の次の位置のデータ位置 */
			}
			else{
				CDocLine* pDocLineNew = new CDocLine;

				_Insert(pDocLineNew, pDocLine); //pDocLineの直前に挿入

				pDocLineNew->m_cLine.SetNativeData( cmemCurLine );

				pDocLineNew->m_cEol = cEOLTypeNext;							/* 改行コードの種類 */
				pDocLineNew->SetBookMark(bBookMarkNext);	// 2001.12.03 hor ブックマークを復元

				pptNewPos->x = nPos - nBgn;	/* 挿入された部分の次の位置のデータ位置 */
			}
		}
	}
	*pnInsLineNum = m_nLines - nAllLinesOld;
	return;
}

//nEOLType nEOLType nEOLTypeNext



/* 現在位置の単語の範囲を調べる */
// 2001/06/23 N.Nakatani WhereCurrentWord()変更 WhereCurrentWord_2をコールするようにした
bool CDocLineMgr::WhereCurrentWord(
	CLogicInt	nLineNum,
	CLogicInt	nIdx,
	CLogicInt*	pnIdxFrom,
	CLogicInt*	pnIdxTo,
	CNativeW2*	pcmcmWord,
	CNativeW2*	pcmcmWordLeft
)
{
	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;

	CDocLine*	pDocLine = GetLineInfo( nLineNum );
	if( NULL == pDocLine ){
		return false;
	}

	CLogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );

	/* 現在位置の単語の範囲を調べる */
	return CWordParse::WhereCurrentWord_2( pLine, nLineLen, nIdx, pnIdxFrom, pnIdxTo, pcmcmWord, pcmcmWordLeft );
}




// 現在位置の左右の単語の先頭位置を調べる
bool CDocLineMgr::PrevOrNextWord(
	CLogicInt	nLineNum,		//	行数
	CLogicInt	nIdx,			//	桁数
	CLogicInt*	pnColmNew,		//	見つかった位置
	BOOL		bLEFT,			//	TRUE:前方（左）へ向かう。FALSE:後方（右）へ向かう。
	BOOL		bStopsBothEnds	//	単語の両端で止まる
)
{
	using namespace WCODE;

	CDocLine*	pDocLine = GetLineInfo( nLineNum );
	if( NULL == pDocLine ){
		return false;
	}

	CLogicInt		nLineLen;
	const wchar_t*	pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );

	// ABC D[EOF]となっていたときに、Dの後ろにカーソルを合わせ、単語の左端に移動すると、Aにカーソルがあうバグ修正。YAZAKI
	if( nIdx >= nLineLen ){
		if (bLEFT && nIdx == nLineLen){
		}
		else {
			nIdx = nLineLen - CLogicInt(1);
		}
	}
	/* 現在位置の文字の種類によっては選択不能 */
	if( !bLEFT && ( pLine[nIdx] == CR || pLine[nIdx] == LF ) ){
		return false;
	}
	/* 前の単語か？後ろの単語か？ */
	if( bLEFT ){
		/* 現在位置の文字の種類を調べる */
		ECharKind	nCharKind = CWordParse::WhatKindOfChar( pLine, nLineLen, nIdx );
		if( nIdx == 0 ){
			return false;
		}

		/* 文字種類が変わるまで前方へサーチ */
		/* 空白とタブは無視する */
		int		nCount = 0;
		CLogicInt	nIdxNext = nIdx;
		CLogicInt	nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW2::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
		while( nCharChars > 0 ){
			CLogicInt		nIdxNextPrev = nIdxNext;
			nIdxNext -= nCharChars;
			ECharKind nCharKindNext = CWordParse::WhatKindOfChar( pLine, nLineLen, nIdxNext );
			/* 空白とタブは無視する */
			if( nCharKind == CK_ZEN_NOBASU ){
				if( nCharKindNext == CK_HIRA || nCharKindNext == CK_ZEN_KATA ){
					nCharKind = nCharKindNext;
				}
			}
			else if( nCharKind == CK_HIRA || nCharKind == CK_ZEN_KATA ){
				if( nCharKindNext == CK_ZEN_NOBASU ){
					nCharKindNext = nCharKind;
				}
			}
			if( nCharKind != nCharKindNext ){
				/* サーチ開始位置の文字が空白またはタブの場合 */
				if( nCharKind == CK_TAB	|| nCharKind == CK_SPACE ){
					if ( bStopsBothEnds && nCount ){
						nIdxNext = nIdxNextPrev;
						break;
					}
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
			nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW2::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
			++nCount;
		}
		*pnColmNew = nIdxNext;
	}else{
		CWordParse::SearchNextWordPosition(pLine, nLineLen, nIdx, pnColmNew, bStopsBothEnds);
	}
	return true;
}





/*! 単語検索

	@date 2003.05.22 かろと 行頭処理など見直し
	@date 2005.11.26 かろと \rや.が\r\nにヒットしないように
*/
/* 見つからない場合は０を返す */
int CDocLineMgr::SearchWord(
	CLogicPoint				ptSerachBegin,	//!< 検索開始位置
	const wchar_t*			pszPattern,		//!< 検索条件
	ESearchDirection		eDirection,		//!< 検索方向
	const SSearchOption&	sSearchOption,	//!< 検索オプション
	CLogicRange*			pMatchRange,	//!< [out] マッチ範囲。ロジック単位。
	CBregexp*				pRegexp			//!< [in]  正規表現コンパイルデータ。既にコンパイルされている必要がある */
)
{
	CDocLine*	pDocLine;
	CLogicInt	nLinePos;
	CLogicInt	nIdxPos;
	CLogicInt	nIdxPosOld;
	const wchar_t*	pLine;
	int			nLineLen;
	const wchar_t*	pszRes;
	int			nHitTo;
	int			nHitPos;
	int			nHitPosOld;
	int			nRetVal = 0;
	//	Jun. 10, 2003 Moca
	//	lstrlenを毎回呼ばずにnPatternLenを使うようにする
	const int	nPatternLen = wcslen( pszPattern );	//2001/06/23 N.Nakatani

	// 検索条件の情報 -> pnKey_CharCharsArr
	int* pnKey_CharCharsArr = NULL;
	CDocLineMgr::CreateCharCharsArr(
		pszPattern,
		nPatternLen,
		&pnKey_CharCharsArr
	);

	//正規表現
	if( sSearchOption.bRegularExp ){
		nLinePos = ptSerachBegin.GetY2();		// 検索行＝検索開始行
		pDocLine = GetLineInfo( nLinePos );
		//前方検索
		if( eDirection == SEARCH_BACKWARD ){
			//
			// 前方(↑)検索(正規表現)
			//
			nHitTo = ptSerachBegin.x;				// 検索開始位置
			nIdxPos = 0;
			while( NULL != pDocLine ){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				nHitPos		= -1;	// -1:この行でマッチ位置なし
				while( 1 ){
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					if (	nIdxPos <= pDocLine->GetLengthWithoutEOL() 
						&&	pRegexp->Match( pLine, nLineLen, nIdxPos ) ){
						// 検索にマッチした！
						nHitPos = pRegexp->GetIndex();
						nIdxPos = pRegexp->GetLastIndex();
						// 長さ０でマッチしたので、この位置で再度マッチしないように、１文字進める
						if (nIdxPos == nHitPos) {
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nIdxPos += (CNativeW2::GetSizeOfChar( pLine, nLineLen, nIdxPos ) == 2 ? 2 : 1);
						}
						if( nHitPos >= nHitTo ){
							// マッチしたのは、カーソル位置以降だった
							// すでにマッチした位置があれば、それを返し、なければ前の行へ
							break;
						}
					} else {
						// マッチしなかった
						// すでにマッチした位置があれば、それを返し、なければ前の行へ
						break;
					}
				}

				if ( -1 != nHitPosOld ) {
					// この行でマッチした位置が存在するので、この行で検索終了
					pMatchRange->SetFromX( nHitPosOld );	// マッチ位置from
					pMatchRange->SetToX  ( nIdxPosOld );	// マッチ位置to
					break;
				} else {
					// この行でマッチした位置が存在しないので、前の行を検索へ
					nLinePos--;
					pDocLine = pDocLine->m_pPrev;
					nIdxPos = 0;
					if( NULL != pDocLine ){
						nHitTo = pDocLine->m_cLine.GetStringLength() + 1;		// 前の行のNULL文字(\0)にもマッチさせるために+1 2003.05.16 かろと 
					}
				}
			}
		}
		//後方検索
		else {
			//
			// 後方検索(正規表現)
			//
			nIdxPos = ptSerachBegin.x;
			while( NULL != pDocLine ){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				if(		nIdxPos <= pDocLine->GetLengthWithoutEOL() 
					&&	pRegexp->Match( pLine, nLineLen, nIdxPos ) ){
					// マッチした
					pMatchRange->SetFromX( pRegexp->GetIndex()     );	// マッチ位置from
					pMatchRange->SetToX  ( pRegexp->GetLastIndex() );	// マッチ位置to
					break;
				}
				++nLinePos;
				pDocLine = pDocLine->m_pNext;
				nIdxPos = 0;
			}
		}
		//
		// 正規表現検索の後処理
		if ( pDocLine != NULL ) {
			// マッチした行がある
			pMatchRange->SetFromY(nLinePos); // マッチ行
			pMatchRange->SetToY  (nLinePos); // マッチ行
			nRetVal = 1;
			// レイアウト行では改行文字内の位置を表現できないため、マッチ開始位置を補正
			if (pMatchRange->GetFrom().x > pDocLine->GetLengthWithoutEOL()) {
				// \r\n改行時に\nにマッチすると置換できない不具合となるため
				// 改行文字内でマッチした場合、改行文字の始めからマッチしたことにする
				pMatchRange->SetFromX( pDocLine->GetLengthWithoutEOL() );
			}
		}
	}
	//単語のみ検索
	else if( sSearchOption.bWordOnly ){
		/*
			2001/06/23 Norio Nakatani
			単語単位の検索を試験的に実装。単語はWhereCurrentWord()で判別してますので、
			英単語やC/C++識別子などの検索条件ならヒットします。
		*/

		// 前方検索
		if( eDirection == SEARCH_BACKWARD ){
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = GetLineInfo( nLinePos );
			CLogicInt nNextWordFrom;
			CLogicInt nNextWordFrom2;
			CLogicInt nNextWordTo2;
			CLogicInt nWork;
			nNextWordFrom = ptSerachBegin.GetX2();
			while( NULL != pDocLine ){
				if( PrevOrNextWord( nLinePos, nNextWordFrom, &nWork, TRUE, FALSE ) ){
					nNextWordFrom = nWork;
					if( WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL ) ){
						if( nPatternLen == nNextWordTo2 - nNextWordFrom2 ){
							const wchar_t* pData = pDocLine->m_cLine.GetStringPtr();	// 2002/2/10 aroka CMemory変更
							/* 1==大文字小文字の区別 */
							if( (!sSearchOption.bLoHiCase && 0 == _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) ) ||
								(sSearchOption.bLoHiCase && 0 ==	 auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) )
							){
								pMatchRange->SetFromY(nLinePos);	// マッチ行
								pMatchRange->SetToY  (nLinePos);	// マッチ行
								pMatchRange->SetFromX(nNextWordFrom2);						// マッチ位置from
								pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);// マッチ位置to
								nRetVal = 1;
								goto end_of_func;
							}
						}
						continue;
					}
				}
				/* 前の行を見に行く */
				nLinePos--;
				pDocLine = pDocLine->m_pPrev;
				if( NULL != pDocLine ){
					nNextWordFrom = pDocLine->m_cLine.GetStringLength() - pDocLine->m_cEol.GetLen();
					if( 0 > nNextWordFrom ){
						nNextWordFrom = CLogicInt(0);
					}
				}
			}
		}
		// 後方検索
		else{
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = GetLineInfo( nLinePos );
			CLogicInt nNextWordFrom;

			CLogicInt nNextWordFrom2;
			CLogicInt nNextWordTo2;
			nNextWordFrom = ptSerachBegin.GetX2();
			while( NULL != pDocLine ){
				if( WhereCurrentWord( nLinePos, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL ) ){
					if( nPatternLen == nNextWordTo2 - nNextWordFrom2 ){
						const wchar_t* pData = pDocLine->m_cLine.GetStringPtr();	// 2002/2/10 aroka CMemory変更
						/* 1==大文字小文字の区別 */
						if( (!sSearchOption.bLoHiCase && 0 ==  _memicmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) ) ||
							(sSearchOption.bLoHiCase && 0 == auto_memcmp( &(pData[nNextWordFrom2]) , pszPattern, nPatternLen ) )
						){
							pMatchRange->SetFromY(nLinePos);	// マッチ行
							pMatchRange->SetToY  (nLinePos);	// マッチ行
							pMatchRange->SetFromX(nNextWordFrom2);						// マッチ位置from
							pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);// マッチ位置to
							nRetVal = 1;
							goto end_of_func;
						}
					}
					/* 現在位置の左右の単語の先頭位置を調べる */
					if( PrevOrNextWord( nLinePos, nNextWordFrom, &nNextWordFrom, FALSE, FALSE ) ){
						continue;
					}
				}
				/* 次の行を見に行く */
				nLinePos++;
				pDocLine = pDocLine->m_pNext;
				nNextWordFrom = CLogicInt(0);
			}
		}

		nRetVal = 0;
		goto end_of_func;
	}
	//普通の検索 (正規表現でも単語単位でもない)
	else{
		//前方検索
		if( eDirection == SEARCH_BACKWARD ){
			nLinePos = ptSerachBegin.GetY2();
			nHitTo = ptSerachBegin.x;

			nIdxPos = 0;
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				nHitPos = -1;
				while( 1 ){
					nHitPosOld = nHitPos;
					nIdxPosOld = nIdxPos;
					pszRes = SearchString(
						pLine,
						nLineLen,
						nIdxPos,
						pszPattern,
						nPatternLen,
						pnKey_CharCharsArr,
						sSearchOption.bLoHiCase
					);
					if( NULL != pszRes ){
						nHitPos = pszRes - pLine;
						nIdxPos = nHitPos + nPatternLen;	// マッチ文字列長進めるように変更 2005.10.28 Karoto
						if( nHitPos >= nHitTo ){
							if( -1 != nHitPosOld ){
								pMatchRange->SetFromY(nLinePos);	// マッチ行
								pMatchRange->SetToY  (nLinePos);	// マッチ行
								pMatchRange->SetFromX(nHitPosOld);	// マッチ位置from
 								pMatchRange->SetToX  (nIdxPosOld);	// マッチ位置to
								nRetVal = 1;
								goto end_of_func;
							}else{
								break;
							}
						}
					}else{
						if( -1 != nHitPosOld ){
							pMatchRange->SetFromY(nLinePos);	// マッチ行
							pMatchRange->SetToY  (nLinePos);	// マッチ行
							pMatchRange->SetFromX(nHitPosOld);	// マッチ位置from
							pMatchRange->SetToX  (nIdxPosOld);	// マッチ位置to
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
					nHitTo = pDocLine->m_cLine.GetStringLength();
				}
			}
			nRetVal = 0;
			goto end_of_func;
		}
		//後方検索
		else{
			nIdxPos = ptSerachBegin.x;
			nLinePos = ptSerachBegin.GetY2();
			pDocLine = GetLineInfo( nLinePos );
			while( NULL != pDocLine ){
				pLine = pDocLine->m_cLine.GetStringPtr( &nLineLen );
				pszRes = SearchString(
					pLine,
					nLineLen,
					nIdxPos,
					pszPattern,
					nPatternLen,
					pnKey_CharCharsArr,
					sSearchOption.bLoHiCase
				);
				if( NULL != pszRes ){
					pMatchRange->SetFromY(nLinePos);	// マッチ行
					pMatchRange->SetToY  (nLinePos);	// マッチ行
					pMatchRange->SetFromX(pszRes - pLine);							// マッチ位置from (文字単位)
					pMatchRange->SetToX  (pMatchRange->GetFrom().x + nPatternLen);	// マッチ位置to   (文字単位)
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

	return nRetVal;
}

/* 検索条件の情報(キー文字列の全角か半角かの配列)作成 */
void CDocLineMgr::CreateCharCharsArr(
	const wchar_t*	pszPattern,
	int				nSrcLen,
	int**			ppnCharCharsArr
)
{
	int		i;
	int*	pnCharCharsArr;
	pnCharCharsArr = new int[nSrcLen];
	for( i = 0; i < nSrcLen; /*i++*/ ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		pnCharCharsArr[i] = CNativeW2::GetSizeOfChar( pszPattern, nSrcLen, i );
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


/*!
	文字列検索
	@return 見つかった場所のポインタ。見つからなかったらNULL。
*/
const wchar_t* CDocLineMgr::SearchString(
	const wchar_t*	pLine,
	int				nLineLen,
	int				nIdxPos,
	const wchar_t*	pszPattern,
	int				nPatternLen,
	int*			pnCharCharsArr,
	bool			bLoHiCase
)
{
	if( nLineLen < nPatternLen ){
		return NULL;
	}
	if( 0 >= nPatternLen || 0 >= nLineLen){
		return NULL;
	}

	int	nPos;
	int	nCompareTo;

	// 線形探索 (kobake注: 後でCharPointerWに置き換えると、サロゲートペアに対応できます)
	nCompareTo = nLineLen - nPatternLen;	//	Mar. 4, 2001 genta

	for( nPos = nIdxPos; nPos <= nCompareTo; nPos++ ){
		int	i;
		for( i = 0; i < nPatternLen; i++ ){
			if( !bLoHiCase ){	//	英大小文字の同一視
				if( towupper( pLine[nPos + i] ) != towupper( pszPattern[i] ) )
					break;
			}
			else {
				if( pLine[nPos + i] != pszPattern[i] ){
					break;
				}
			}
		}
		if( i >= nPatternLen ){
			return &pLine[nPos];
		}
	}
	return NULL;
}




/*!	@brief CDocLineMgrDEBUG用

	@date 2004.03.18 Moca
		m_pDocLineCurrentとm_pCodePrevReferがデータチェーンの
		要素を指しているかの検証機能を追加．

*/
void CDocLineMgr::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE_A( "------------------------\n" );

	CDocLine* pDocLine;
	CDocLine* pDocLineNext;
	CDocLine* pDocLineEnd = NULL;
	pDocLine = m_pDocLineTop;

	// 正当性を調べる
	bool bIncludeCurrent = false;
	bool bIncludePrevRefer = false;
	CLogicInt nNum = CLogicInt(0);
	if( m_pDocLineTop->m_pPrev != NULL ){
		MYTRACE_A( "error: m_pDocLineTop->m_pPrev != NULL\n");
	}
	if( m_pDocLineBot->m_pNext != NULL ){
		MYTRACE_A( "error: m_pDocLineBot->m_pNext != NULL\n" );
	}
	while( NULL != pDocLine ){
		if( m_pDocLineCurrent == pDocLine ){
			bIncludeCurrent = true;
		}
		if( m_pCodePrevRefer == pDocLine ){
			bIncludePrevRefer = true;
		}
		if( NULL != pDocLine->m_pNext ){
			if( pDocLine->m_pNext == pDocLine ){
				MYTRACE_A( "error: pDocLine->m_pPrev Invalid value.\n" );
				break;
			}
			if( pDocLine->m_pNext->m_pPrev != pDocLine ){
				MYTRACE_A( "error: pDocLine->m_pNext->m_pPrev != pDocLine.\n" );
				break;
			}
		}else{
			pDocLineEnd = pDocLine;
		}
		pDocLine = pDocLine->m_pNext;
		nNum++;
	}
	
	if( pDocLineEnd != m_pDocLineBot ){
		MYTRACE_A( "error: pDocLineEnd != m_pDocLineBot" );
	}
	
	if( nNum != m_nLines ){
		MYTRACE_A( "error: nNum(%d) != m_nLines(%d)\n", nNum, m_nLines );
	}
	if( false == bIncludeCurrent && m_pDocLineCurrent != NULL ){
		MYTRACE_A( "error: m_pDocLineCurrent=%08lxh Invalid value.\n", m_pDocLineCurrent );
	}
	if( false == bIncludePrevRefer && m_pCodePrevRefer != NULL ){
		MYTRACE_A( "error: m_pCodePrevRefer =%08lxh Invalid value.\n", m_pCodePrevRefer );
	}

	// DUMP
	MYTRACE_A( "m_nLines=%d\n", m_nLines );
	MYTRACE_A( "m_pDocLineTop=%08lxh\n", m_pDocLineTop );
	MYTRACE_A( "m_pDocLineBot=%08lxh\n", m_pDocLineBot );
	pDocLine = m_pDocLineTop;
	while( NULL != pDocLine ){
		pDocLineNext = pDocLine->m_pNext;
		MYTRACE_A( "\t-------\n" );
		MYTRACE_A( "\tthis=%08lxh\n", pDocLine );
		MYTRACE_A( "\tpPrev; =%08lxh\n", pDocLine->m_pPrev );
		MYTRACE_A( "\tpNext; =%08lxh\n", pDocLine->m_pNext );

		MYTRACE_A( "\tm_enumEOLType =%ls\n", pDocLine->m_cEol.GetName() );
		MYTRACE_A( "\tm_nEOLLen =%d\n", pDocLine->m_cEol.GetLen() );


//		MYTRACE_A( "\t[%ls]\n", *(pDocLine->m_pLine) );
		MYTRACE_A( "\tpDocLine->m_cLine.GetLength()=[%d]\n", pDocLine->m_cLine.GetStringLength() );
		MYTRACE_A( "\t[%ls]\n", pDocLine->m_cLine.GetStringPtr() );


		pDocLine = pDocLineNext;
	}
	MYTRACE_A( "------------------------\n" );
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
		pDocLine->SetModifyFlg(false);		/* 変更フラグ */
//		if( bResetModifyCount ){			/* 変更回数を0にするかどうか */
//			pDocLine->m_nModifyCount = 0;	/* 変更回数 */
//		}

		pDocLine = pDocLineNext;
	}
	return;
}


/* 全行データを返す
	改行コードは、CFLF統一される。
	@retval 全行データ。freeで開放しなければならない。
	@note   Debug版のテストにのみ使用している。
*/
wchar_t* CDocLineMgr::GetAllData( int*	pnDataLen )
{
	int			nDataLen;
	wchar_t*	pLine;
	int			nLineLen;
	CDocLine* 	pDocLine;

	pDocLine = m_pDocLineTop;
	nDataLen = 0;
	while( NULL != pDocLine ){
		//	Oct. 7, 2002 YAZAKI
		nDataLen += pDocLine->GetLengthWithoutEOL() + 2;	//	\r\nを追加して返すため+2する。
		pDocLine = pDocLine->m_pNext;
	}

	wchar_t* pData;
	pData = (wchar_t*)malloc( (nDataLen + 1) * sizeof(wchar_t) );
	if( NULL == pData ){
		::MYMESSAGEBOX_A(
			NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME_A,
			"CDocLineMgr::GetAllData()\nメモリ確保に失敗しました。\n%dバイト",
			nDataLen + 1
		);
		return NULL;
	}
	pDocLine = m_pDocLineTop;

	nDataLen = 0;
	while( NULL != pDocLine ){
		//	Oct. 7, 2002 YAZAKI
		nLineLen = pDocLine->GetLengthWithoutEOL();
		if( 0 < nLineLen ){
			pLine = pDocLine->m_cLine.GetStringPtr();
			wmemcpy( &pData[nDataLen], pLine, nLineLen );
			nDataLen += nLineLen;
		}
		pData[nDataLen++] = L'\r';
		pData[nDataLen++] = L'\n';
		pDocLine = pDocLine->m_pNext;
	}
	pData[nDataLen] = L'\0';
	*pnDataLen = nDataLen;
	return pData;
}


/* 行オブジェクトの削除、リスト変更、行数-- */
void CDocLineMgr::DeleteNode( CDocLine* pCDocLine )
{
	m_nLines--;	/* 全行数 */
	if( CLogicInt(0) == m_nLines ){
		/* データがなくなった */
		Init();
		return;
	}
	
	if( NULL == pCDocLine->m_pPrev ){
		m_pDocLineTop = pCDocLine->m_pNext;
	}
	else{
		pCDocLine->m_pPrev->m_pNext = pCDocLine->m_pNext;
	}

	if( NULL == pCDocLine->m_pNext ){
		m_pDocLineBot = pCDocLine->m_pPrev;
	}
	else{
		pCDocLine->m_pNext->m_pPrev = pCDocLine->m_pPrev;
	}
	
	if( m_pCodePrevRefer == pCDocLine ){
		m_pCodePrevRefer = pCDocLine->m_pNext;
	}
	delete pCDocLine;

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


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- チェーン関数 -- -- // 2007.10.11 kobake 作成
//!最下部に挿入
void CDocLineMgr::_PushBottom(CDocLine* pDocLineNew)
{
	if( !m_pDocLineTop ){
		m_pDocLineTop = pDocLineNew;
	}
	pDocLineNew->m_pPrev = m_pDocLineBot;

	if( m_pDocLineBot ){
		m_pDocLineBot->m_pNext = pDocLineNew;
	}
	m_pDocLineBot = pDocLineNew;
	pDocLineNew->m_pNext = NULL;

	++m_nLines;
}

//!pPosの直前に挿入
void CDocLineMgr::_Insert(CDocLine* pDocLineNew, CDocLine* pPos)
{
	pDocLineNew->m_pPrev = pPos->m_pPrev;
	pDocLineNew->m_pNext = pPos;
	pPos->m_pPrev->m_pNext = pDocLineNew;
	pPos->m_pPrev = pDocLineNew;

	++m_nLines;
}

