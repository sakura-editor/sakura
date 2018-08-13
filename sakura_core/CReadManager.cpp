/*
	Copyright (C) 2008, kobake

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
#include "StdAfx.h"
#include <io.h>	// access
#include "CReadManager.h"
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
	const STypeConfigMini* type;
	CDocTypeManager().GetTypeConfigMini( sLoadInfo.nType, &type );
	ECodeType	eCharCode = sLoadInfo.eCharCode;
	if (CODE_AUTODETECT == eCharCode) {
		CCodeMediator cmediator( type->m_encoding );
		eCharCode = cmediator.CheckKanjiCodeOfFile( pszPath );
	}
	if (!IsValidCodeOrCPType( eCharCode )) {
		eCharCode = type->m_encoding.m_eDefaultCodetype;	// 2011.01.24 ryoji デフォルト文字コード
	}
	bool	bBom;
	if (eCharCode == type->m_encoding.m_eDefaultCodetype) {
		bBom = type->m_encoding.m_bDefaultBom;	// 2011.01.24 ryoji デフォルトBOM
	}
	else{
		bBom = CCodeTypeName( eCharCode ).IsBomDefOn();
	}
	pFileInfo->SetCodeSet( eCharCode, bBom );

	/* 既存データのクリア */
	pcDocLineMgr->DeleteAllLine();

	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( NULL ) ){
		return RESULT_FAILURE; //######INTERRUPT
	}

	EConvertResult eRet = RESULT_COMPLETE;

	try{
		CFileLoad cfl(type->m_encoding);

		bool bBigFile;
#ifdef _WIN64
		bBigFile = true;
#else
		bBigFile = false;
#endif
		// ファイルを開く
		// ファイルを閉じるにはFileCloseメンバ又はデストラクタのどちらかで処理できます
		//	Jul. 28, 2003 ryoji BOMパラメータ追加
		cfl.FileOpen( pszPath, bBigFile, eCharCode, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode(), &bBom );
		pFileInfo->SetBomExist( bBom );

		/* ファイル時刻の取得 */
		FILETIME	FileTime;
		if( cfl.GetFileTime( NULL, NULL, &FileTime ) ){
			pFileInfo->SetFileTime( FileTime );
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
	catch( const CError_FileOpen& ex ){
		eRet = RESULT_FAILURE;
		if (ex.Reason() == CError_FileOpen::TOO_BIG) {
			// ファイルサイズが大きすぎる (32bit 版の場合は 2GB あたりが上限)
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				LS(STR_ERR_DLGDOCLM_TOOBIG),
				pszPath
			);
		}
		else if( !fexist( pszPath )){
			// ファイルがない
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				LS(STR_ERR_DLGDOCLM1),	//Mar. 24, 2001 jepro 若干修正
				pszPath
			);
		}
		else if( -1 == _taccess( pszPath, 4 )){
			// 読み込みアクセス権がない
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				LS(STR_ERR_DLGDOCLM2),
				pszPath
			 );
		}
		else{
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				LS(STR_ERR_DLGDOCLM3),
				pszPath
			 );
		}
	}
	catch( CError_FileRead ){
		eRet = RESULT_FAILURE;
		ErrorMessage(
			CEditWnd::getInstance()->GetHwnd(),
			LS(STR_ERR_DLGDOCLM4),
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
//	CModifyVisitor().ResetAllModifyFlag(pcDocLineMgr, 0);
	return eRet;
}


