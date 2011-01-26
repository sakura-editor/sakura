#include "stdafx.h"
#include "CReadManager.h"
#include <io.h>	// access
#include "CEditApp.h"	// CAppExitException
#include "window/CEditWnd.h"
#include "charset/CCodeMediator.h"
#include "io/CFileLoad.h"
#include "util/window.h"

/*!
	ファイルを読み込んで格納する（分割読み込みテスト版）
	@version	2.0
	@note	Windows用にコーディングしてある
	@retval	TRUE	正常読み込み
	@retval	FALSE	エラー(またはユーザによるキャンセル?)
	@date	2002/08/30 Moca 旧ReadFileを元に作成 ファイルアクセスに関する部分をCFileLoadで行う
	@date	2003/07/26 ryoji BOMの状態の取得を追加
*/
EConvertResult CReadManager::ReadFile_To_CDocLineMgr(
	CDocLineMgr*		pcDocLineMgr,	//!< [out]
	const SLoadInfo&	sLoadInfo,		//!< [in]
	SFileInfo*			pFileInfo		//!< [out]
)
{
	LPCTSTR pszPath = sLoadInfo.cFilePath.c_str();

	// 文字コード種別
	pFileInfo->eCharCode = sLoadInfo.eCharCode;
	if( CODE_AUTODETECT==pFileInfo->eCharCode) {
		CCodeMediator cmediator( CEditWnd::Instance()->GetDocument() );
		pFileInfo->eCharCode = cmediator.CheckKanjiCodeOfFile( pszPath );
	}
	STypeConfig& types = CDocTypeManager().GetTypeSetting( sLoadInfo.nType );
	if( !IsValidCodeType(pFileInfo->eCharCode) ){
		pFileInfo->eCharCode = static_cast<ECodeType>( types.m_eDefaultCodetype );	// 2011.01.24 ryoji デフォルト文字コード
	}
	if ( pFileInfo->eCharCode == static_cast<ECodeType>( types.m_eDefaultCodetype ) ){
		pFileInfo->bBomExist = ( types.m_bDefaultBom != FALSE );	// 2011.01.24 ryoji デフォルトBOM
	}
	else{
		pFileInfo->bBomExist = ( pFileInfo->eCharCode == CODE_UNICODE || pFileInfo->eCharCode == CODE_UNICODEBE );
	}

	/* 既存データのクリア */
	pcDocLineMgr->DeleteAllLine();

	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( NULL ) ){
		return RESULT_FAILURE; //######INTERRUPT
	}

	EConvertResult eRet = RESULT_COMPLETE;

	try{
		CFileLoad cfl;

		// ファイルを開く
		// ファイルを閉じるにはFileCloseメンバ又はデストラクタのどちらかで処理できます
		//	Jul. 28, 2003 ryoji BOMパラメータ追加
		cfl.FileOpen( pszPath, pFileInfo->eCharCode, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode(), &pFileInfo->bBomExist );

		/* ファイル時刻の取得 */
		FILETIME	FileTime;
		if( cfl.GetFileTime( NULL, NULL, &FileTime ) ){
			pFileInfo->cFileTime.SetFILETIME(FileTime);
		}

		// ReadLineはファイルから 文字コード変換された1行を読み出します
		// エラー時はthrow CError_FileRead を投げます
		int				nLineNum = 0;
		CEol			cEol;
		CNativeW		cUnicodeBuffer;
		EConvertResult	eRead;
		while( RESULT_FAILURE != (eRead = cfl.ReadLine( &cUnicodeBuffer, &cEol )) ){
			if(eRead==RESULT_LOSESOME){
				eRet = RESULT_LOSESOME;
			}
			const wchar_t*	pLine = cUnicodeBuffer.GetStringPtr();
			int		nLineLen = cUnicodeBuffer.GetStringLength();
			++nLineNum;
			CDocEditAgent(pcDocLineMgr).AddLineStrX( pLine, nLineLen );
			//経過通知
			if(nLineNum%512==0){
				NotifyProgress(cfl.GetPercent());
				// 処理中のユーザー操作を可能にする
				if( !::BlockingHook( NULL ) ){
					throw CAppExitException(); //中断検出
				}
			}
		}

		// ファイルをクローズする
		cfl.FileClose();
	}
	catch(CAppExitException){
		//WM_QUITが発生した
		return RESULT_FAILURE;
	}
	catch( CError_FileOpen ){
		eRet = RESULT_FAILURE;
		if( !fexist( pszPath )){
			// ファイルがない
			ErrorMessage(
				CEditWnd::Instance()->GetHwnd(),
				_T("%ls\nというファイルを開けません。\nファイルが存在しません。"),	//Mar. 24, 2001 jepro 若干修正
				pszPath
			);
		}
		else if( -1 == _taccess( pszPath, 4 )){
			// 読み込みアクセス権がない
			ErrorMessage(
				CEditWnd::Instance()->GetHwnd(),
				_T("\'%ts\'\nというファイルを開けません。\n読み込みアクセス権がありません。"),
				pszPath
			 );
		}
		else{
			ErrorMessage(
				CEditWnd::Instance()->GetHwnd(),
				_T("\'%ts\'\nというファイルを開けません。\n他のアプリケーションで使用されている可能性があります。"),
				pszPath
			 );
		}
	}
	catch( CError_FileRead ){
		eRet = RESULT_FAILURE;
		ErrorMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("\'%ts\'というファイルの読み込み中にエラーが発生しました。\nファイルの読み込みを中止します。"),
			pszPath
		 );
		/* 既存データのクリア */
		pcDocLineMgr->DeleteAllLine();
	} // 例外処理終わり

	NotifyProgress(0);
	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( NULL ) ){
		return RESULT_FAILURE; //####INTERRUPT
	}

	/* 行変更状態をすべてリセット */
	CModifyVisitor().ResetAllModifyFlag(pcDocLineMgr);
	return eRet;
}


