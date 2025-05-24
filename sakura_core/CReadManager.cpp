/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include <io.h>	// _access
#include "CReadManager.h"
#include "CEditApp.h"	// CAppExitException
#include "window/CEditWnd.h"
#include "charset/CCodeMediator.h"
#include "io/CFileLoad.h"
#include "util/window.h"
#include "CSelectLang.h"
#include "String_define.h"
#include <atomic>
#include <future>

/*!
	ファイルを読み込んで格納する（分割読み込みテスト版）
	@version	2.0
	@note	Windows用にコーディングしてある
	@retval	TRUE	正常読み込み
	@retval	FALSE	エラー(またはユーザーによるキャンセル?)
	@date	2002/08/30 Moca 旧ReadFileを元に作成 ファイルアクセスに関する部分をCFileLoadで行う
	@date	2003/07/26 ryoji BOMの状態の取得を追加
*/
EConvertResult CReadManager::ReadFile_To_CDocLineMgr(
	CDocLineMgr*		pcDocLineMgr,	//!< [out]
	const SLoadInfo&	sLoadInfo,		//!< [in]
	SFileInfo*			pFileInfo		//!< [out]
)
{
	LPCWSTR pszPath = sLoadInfo.cFilePath.c_str();

	// 文字コード種別
	const STypeConfigMini* type = NULL;
	if( !CDocTypeManager().GetTypeConfigMini( sLoadInfo.nType, &type ) ){
		return RESULT_FAILURE;
	}
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
		CFileLoad cfl( type->m_encoding );

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

		// 行データ読み込みに使うスレッド数 (メインスレッドを含む)
		const int nThreadCount = (std::max)(1, (int)std::thread::hardware_concurrency());

		std::vector<CFileLoad> vecThreadFileLoads( nThreadCount );
		std::vector<CDocLineMgr> vecThreadDocLineMgrs( nThreadCount );
		std::vector<std::future<EConvertResult>> vecWorkerFutures;
		std::atomic<bool> bCanceled = false;

		size_t nOffsetBegin = cfl.GetNextLineOffset( (size_t)cfl.GetFileSize() );
		for( int i = nThreadCount - 1; 0 <= i; i-- ){
			// 分担する範囲を決める
			const size_t nOffsetEnd = nOffsetBegin;
			nOffsetBegin = cfl.GetNextLineOffset( (size_t)((double)cfl.GetFileSize() / nThreadCount * i) );

			if( nOffsetBegin == nOffsetEnd ){
				continue;
			}

			vecThreadFileLoads[i].Prepare( cfl, nOffsetBegin, nOffsetEnd );
			vecThreadDocLineMgrs[i].SetMemoryResource(pcDocLineMgr->GetMemoryResource());

			if( i == 0 ){
				// 最後はメインスレッドで処理
				eRet = ReadLines( true, vecThreadFileLoads[i], vecThreadDocLineMgrs[i], bCanceled );
			}else{
				// ワーカースレッドで処理
				vecWorkerFutures.push_back(
					std::async(
						std::launch::async,
						&CReadManager::ReadLines,
						this,
						false,
						std::ref(vecThreadFileLoads[i]),
						std::ref(vecThreadDocLineMgrs[i]),
						std::ref(bCanceled)
					)
				);
			}
		}

		// ワーカースレッド待ち合わせ
		for( auto&& future : vecWorkerFutures ){
			EConvertResult eRetSub = future.get();
			if( eRetSub != RESULT_COMPLETE ){
				eRet = eRetSub;
			}
		}

		if( bCanceled.load() ){
			// 中断
			throw CAppExitException();
		}

		// 各スレッドの処理結果をpcDocLineMgrに集約
		for( int i = 0; i < nThreadCount; i++ ){
			pcDocLineMgr->AppendAsMove( vecThreadDocLineMgrs[i] );
		}

		cfl.FileClose();
	}
	catch(const CAppExitException&){
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
		else if( -1 == _waccess( pszPath, 4 )){
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
	catch( const CError_FileRead& ){
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

/*!
	ファイルから行データを読み込む
	@param[in]		bMainThread	メインスレッドで実行しているかどうか
	@param[in]		cFileLoad	ファイル読み込みクラス
	@param[out]		cDocLineMgr	読み込んだ行データを格納
	@param[in,out]	bCanceled	処理中断フラグ
	@returns	読み込み処理結果
*/
EConvertResult CReadManager::ReadLines(
	bool				bMainThread,
	CFileLoad&			cFileLoad,
	CDocLineMgr&		cDocLineMgr,
	std::atomic<bool>&	bCanceled
)
{
	CEol			cEol;
	CNativeW		cUnicodeBuffer;
	EConvertResult	eRead;
	constexpr DWORD timeInterval = 33;
	auto			nextTime = GetTickCount64() + timeInterval;
	EConvertResult	eRet = RESULT_COMPLETE;

	while( RESULT_FAILURE != (eRead = cFileLoad.ReadLine( &cUnicodeBuffer, &cEol )) ){
		if( eRead == RESULT_LOSESOME ){
			eRet = RESULT_LOSESOME;
		}

		if( bCanceled.load() ){
			break;
		}

		const wchar_t* pLine = cUnicodeBuffer.GetStringPtr();
		const auto nLineLen = cUnicodeBuffer.GetStringLength();
		CDocEditAgent(&cDocLineMgr).AddLineStrX( pLine, nLineLen );

		if( bMainThread ){
			// 経過通知
			const auto currTime = GetTickCount64();
			if( currTime >= nextTime ){
				nextTime += timeInterval;
				NotifyProgress( cFileLoad.GetPercent() );
				// 処理中のユーザー操作を可能にする
				if( !::BlockingHook( NULL ) ){
					// 中断検知
					bCanceled.store( true );
					eRet = RESULT_FAILURE;
					break;
				}
			}
		}
	}

	return eRet;
}
