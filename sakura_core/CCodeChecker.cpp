/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#include "CCodeChecker.h"
#include "charset/CCodePage.h"
#include "io/CIoBridge.h"
#include "charset/CCodeFactory.h" ////
#include "charset/CUnicode.h"

#include "doc/CEditDoc.h"
#include "doc/logic/CDocLineMgr.h"
#include "window/CEditWnd.h"
#include "util/string_ex.h"
#include "CSelectLang.h"
#include "config/app_constants.h"
#include "String_define.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     セーブ時チェック                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! CDocLineMgrが保持するデータに異なる改行コードが混在しているかどうか判定する
static bool _CheckSavingEolcode(const CDocLineMgr& pcDocLineMgr, CEol cEolType)
{
	bool bMix = false;
	if( cEolType.IsNone() ){	//改行コード変換なし
		CEol cEolCheck;	//比較対象EOL
		const CDocLine* pcDocLine = pcDocLineMgr.GetDocLineTop();
		if( pcDocLine ){
			cEolCheck = pcDocLine->GetEol();
		}
		while( pcDocLine ){
			CEol cEol = pcDocLine->GetEol();
			if( cEol != cEolCheck && cEol.IsValid() ){
				bMix = true;
				break;
			}
			pcDocLine = pcDocLine->GetNextLine();
		}
	}
	return bMix;
}

//! CDocLineMgrが保持するデータを指定文字コードで安全に保存できるかどうか判定する
static EConvertResult _CheckSavingCharcode(const CDocLineMgr& pcDocLineMgr, ECodeType eCodeType, CLogicPoint& point, CNativeW& wc)
{
	const CDocLine*	pcDocLine = pcDocLineMgr.GetDocLineTop();
	const bool bCodePageMode = IsValidCodeOrCPType(eCodeType) && !IsValidCodeType(eCodeType);
	CCodeBase* pCodeBase=CCodeFactory::CreateCodeBase(eCodeType,0);
	CMemory cmemTmp;	// バッファを再利用
	CNativeW cmemTmp2;
	CLogicInt nLine = CLogicInt(0);
	while( pcDocLine ){
		// コード変換 pcDocLine -> cmemTmp
		EConvertResult e = CIoBridge::ImplToFile(
			pcDocLine->_GetDocLineDataWithEOL(),
			&cmemTmp,
			pCodeBase
		);
		if( bCodePageMode ){
			// コードページはRESULT_LOSESOMEを返さないので、自分で文字列比較する
			EConvertResult e2 = CIoBridge::FileToImpl(
				cmemTmp,
				&cmemTmp2,
				pCodeBase,
				0
			);
			const int nDocLineLen = (Int)pcDocLine->GetLengthWithEOL();
			const int nConvertLen = (Int)cmemTmp2.GetStringLength();
			const int nDataMinLen = t_min(nDocLineLen, nConvertLen);
			const wchar_t* p = pcDocLine->GetPtr();
			const wchar_t* r = cmemTmp2.GetStringPtr();
			int nPos = -1;
			for( int i = 0; i < nDataMinLen; i++ ){
				if( p[i] != r[i] ){
					nPos = i;
					break;
				}
			}
			if( nPos == -1 && nDocLineLen != nConvertLen ){
				nPos = nDataMinLen;
			}
			if( nPos != -1 ){
				point.y = nLine;
				point.x = CLogicInt(nPos);
				// 変換できなかった位置の1文字取得
				wc.SetString( p + nPos, (Int)CNativeW::GetSizeOfChar( p, nDocLineLen, nPos ) );
				delete pCodeBase;
				return RESULT_LOSESOME;
			}
		}
		if(e!=RESULT_COMPLETE){
			if( e == RESULT_LOSESOME ){
				// 行内の位置を特定
				point.y = nLine;
				point.x = CLogicInt(-1);
				const WCHAR* pLine = pcDocLine->GetPtr();
				const CLogicInt nLineLen = pcDocLine->GetLengthWithEOL();
				CLogicInt chars = CNativeW::GetSizeOfChar( pLine, nLineLen, 0 );
				CLogicInt nPos = CLogicInt(0);
				CNativeW mem;
				while( 0 < chars ){
					mem.SetStringHoldBuffer( pLine + nPos, chars );
					EConvertResult e2 = CIoBridge::ImplToFile(
						mem,
						&cmemTmp,
						pCodeBase
					);
					if( e2 == RESULT_LOSESOME ){
						point.x = nPos;
						wc = mem;
						break;
					}
					nPos += chars;
					chars = CNativeW::GetSizeOfChar( pLine, nLineLen, nPos );
				}
			}
			delete pCodeBase;
			return e;
		}

		//次の行へ
		pcDocLine = pcDocLine->GetNextLine();
		nLine++;
	}
	delete pCodeBase;
	return RESULT_COMPLETE;
}

ECallbackResult CCodeChecker::OnCheckSave(SSaveInfo* pSaveInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	//改行コードが混在しているかどうか判定
	bool bTmpResult = false;
	if( pcDoc->m_cDocType.GetDocumentAttribute().m_bChkEnterAtEnd ){
		bTmpResult = _CheckSavingEolcode(
			pcDoc->m_cDocLineMgr, pSaveInfo->cEol
		);
	}

	//ユーザー問い合わせ
	if (bTmpResult) {
		int nDlgResult = MYMESSAGEBOX(
			CEditWnd::getInstance()->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONWARNING,
			GSTR_APPNAME,
			LS(STR_CODECHECKER_EOL_UNIFY),
			pcDoc->m_cDocEditor.GetNewLineCode().GetName()
		);
		switch(nDlgResult){
		case IDYES:		pSaveInfo->cEol = pcDoc->m_cDocEditor.GetNewLineCode(); break; //統一
		case IDNO:		break; //続行
		case IDCANCEL:	return CALLBACK_INTERRUPT; //中断
		}
	}

	//指定文字コードで安全に保存できるかどうか判定
	CLogicPoint point;
	CNativeW cmemChar(L"", 0);
	EConvertResult nTmpResult = _CheckSavingCharcode(
		pcDoc->m_cDocLineMgr, pSaveInfo->eCharCode,
		point, cmemChar
	);

	//ユーザー問い合わせ
	if(nTmpResult==RESULT_LOSESOME){
		WCHAR szCpName[100];
		WCHAR  szLineNum[60];  // 123桁
		WCHAR  szCharCode[12]; // U+12ab or 1234abcd
		CCodePage::GetNameNormal(szCpName, pSaveInfo->eCharCode);
		szCharCode[0] = L'\0';
		szLineNum[0] = L'\0';
		if( point.x == -1 ){
			cmemChar.SetString(LS(STR_ERR_CSHAREDATA22));
		}else{
			auto_sprintf( szLineNum, L"%d", (int)((Int)point.x) + 1 );
			wcscat( szLineNum, LS(STR_DLGFNCLST_LIST_COL) );
			CUnicode().UnicodeToHex( cmemChar.GetStringPtr(), cmemChar.GetStringLength(),
				szCharCode, &GetDllShareData().m_Common.m_sStatusbar );
		}
		int nDlgResult = MYMESSAGEBOX(
			CEditWnd::getInstance()->GetHwnd(),
			MB_YESNOCANCEL | MB_ICONWARNING,
			GSTR_APPNAME,
			LS(STR_CODECHECKER_CONFORM_LOSESOME),
			szCpName,
			(int)((Int)point.y + 1),
			szLineNum,
			cmemChar.GetStringPtr(),
			szCharCode
		);
		switch(nDlgResult){
		case IDYES:		break; //続行
		case IDNO:		return CALLBACK_INTERRUPT; //中断
		case IDCANCEL:
			{
				CLogicPoint pt(point.x < 0 ? CLogicInt(0) : point.x, point.y);
				GetEditWnd().GetActiveView().GetCommander().Command_MOVECURSOR(pt, 0);
			}
			return CALLBACK_INTERRUPT; //中断
		}
	}
	return CALLBACK_CONTINUE;
}

void CCodeChecker::OnFinalSave(ESaveResult eSaveResult)
{
	//カキコ結果
	if(eSaveResult==SAVED_LOSESOME){
		ErrorMessage(CEditWnd::getInstance()->GetHwnd(), LS(STR_CODECHECKER_LOSESOME_SAVE));
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ロード時チェック                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CCodeChecker::OnFinalLoad(ELoadResult eLoadResult)
{
	if(eLoadResult==LOADED_LOSESOME){
		ErrorMessage(
			CEditWnd::getInstance()->GetHwnd(),
			LS(STR_CODECHECKER_LOSESOME_ROAD)
		);
	}
}
