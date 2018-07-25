#include "StdAfx.h"
#include "CWriteManager.h"
#include <list>
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"
#include "CEditApp.h" // CAppExitException
#include "window/CEditWnd.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "charset/CUnicode.h"
#include "io/CIoBridge.h"
#include "io/CBinaryStream.h"
#include "util/window.h"


/*! バッファ内容をファイルに書き出す (テスト用)

	@note Windows用にコーディングしてある
	@date 2003.07.26 ryoji BOM引数追加
*/
EConvertResult CWriteManager::WriteFile_From_CDocLineMgr(
	const CDocLineMgr&	pcDocLineMgr,	//!< [in]
	const SSaveInfo&	sSaveInfo		//!< [in]
)
{
	EConvertResult		nRetVal = RESULT_COMPLETE;
	std::auto_ptr<CCodeBase> pcCodeBase( CCodeFactory::CreateCodeBase(sSaveInfo.eCharCode,0) );

	{
		// 変換テスト
		CNativeW buffer = L"abcde";
		CMemory tmp;
		EConvertResult e = pcCodeBase->UnicodeToCode( buffer, &tmp );
		if(e==RESULT_FAILURE){
			nRetVal=RESULT_FAILURE;
			ErrorMessage(
				CEditWnd::getInstance()->GetHwnd(),
				LS(STR_FILESAVE_CONVERT_ERROR),
				sSaveInfo.cFilePath.c_str()
			);
			return nRetVal;
		}
	}


	try
	{
		//ファイルオープン
		CBinaryOutputStream out(sSaveInfo.cFilePath,true);

		//各行出力
		int			nLineNumber = 0;
		const CDocLine*	pcDocLine = pcDocLineMgr.GetDocLineTop();
		// 1行目
		{
			++nLineNumber;
			CMemory cmemOutputBuffer;
			{
				CNativeW cstrSrc;
				CMemory cstrBomCheck;
				pcCodeBase->GetBom( &cstrBomCheck );
				if( sSaveInfo.bBomExist && 0 < cstrBomCheck.GetRawLength() ){
					// 1行目にはBOMを付加する。エンコーダでbomがある場合のみ付加する。
					CUnicode().GetBom( cstrSrc._GetMemory() );
				}
				if( pcDocLine ){
					cstrSrc.AppendNativeData( pcDocLine->_GetDocLineDataWithEOL() );
				}
				EConvertResult e = pcCodeBase->UnicodeToCode( cstrSrc, &cmemOutputBuffer );
				if(e==RESULT_LOSESOME){
					nRetVal=RESULT_LOSESOME;
				}
				if(e==RESULT_FAILURE){
					nRetVal=RESULT_FAILURE;
					ErrorMessage(
						CEditWnd::getInstance()->GetHwnd(),
						LS(STR_FILESAVE_CONVERT_ERROR),
						sSaveInfo.cFilePath.c_str()
					);
					throw CError_FileWrite();
				}
			}
			out.Write(cmemOutputBuffer.GetRawPtr(), cmemOutputBuffer.GetRawLength());
			if( pcDocLine ){
				pcDocLine = pcDocLine->GetNextLine();
			}
		}
		CMemory cmemOutputBuffer;
		while( pcDocLine ){
			++nLineNumber;

			//経過通知
			if(pcDocLineMgr.GetLineCount()>0 && nLineNumber%1024==0){
				NotifyProgress(nLineNumber * 100 / pcDocLineMgr.GetLineCount());
				// 処理中のユーザー操作を可能にする
				if( !::BlockingHook( NULL ) ){
					throw CAppExitException(); //中断検出
				}
			}

			//1行出力 -> cmemOutputBuffer
			{
				// 書き込み時のコード変換 cstrSrc -> cmemOutputBuffer
				EConvertResult e = pcCodeBase->UnicodeToCode(
					pcDocLine->_GetDocLineDataWithEOL(),
					&cmemOutputBuffer
				);
				if(e==RESULT_LOSESOME){
					if(nRetVal==RESULT_COMPLETE)nRetVal=RESULT_LOSESOME;
				}
				if(e==RESULT_FAILURE){
					nRetVal=RESULT_FAILURE;
					ErrorMessage(
						CEditWnd::getInstance()->GetHwnd(),
						LS(STR_FILESAVE_CONVERT_ERROR),
						sSaveInfo.cFilePath.c_str()
					);
					break;
				}
			}

			//ファイルに出力 cmemOutputBuffer -> fp
			out.Write(cmemOutputBuffer.GetRawPtr(), cmemOutputBuffer.GetRawLength());

			//次の行へ
			pcDocLine = pcDocLine->GetNextLine();
		}

		//ファイルクローズ
		out.Close();
	}
	catch(CError_FileOpen){ //########### 現時点では、この例外が発生した場合は正常に動作できない
		ErrorMessage(
			CEditWnd::getInstance()->GetHwnd(),
			LS(STR_SAVEAGENT_OTHER_APP),
			sSaveInfo.cFilePath.c_str()
		);
		nRetVal = RESULT_FAILURE;
	}
	catch(CError_FileWrite){
		nRetVal = RESULT_FAILURE;
	}
	catch(CAppExitException){
		//中断検出
		return RESULT_FAILURE;
	}

	return nRetVal;
}
