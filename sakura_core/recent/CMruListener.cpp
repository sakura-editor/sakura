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
#include "CMruListener.h"
#include "recent/CMRUFile.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "view/CEditView.h"
#include "charset/CCodePage.h"
#include "charset/CCodeMediator.h"
#include "util/file.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        セーブ前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

void CMruListener::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	_HoldBookmarks_And_AddToMRU();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        ロード前後                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
void CMruListener::OnBeforeLoad(SLoadInfo* pLoadInfo)
{
	// 再ロード用に現在ファイルをMRU登録しておく
	// Mar. 30, 2003 genta ブックマーク保存のためMRUへ登録
	_HoldBookmarks_And_AddToMRU();	// ← 新規オープン（ファイル名未設定）では何もしない

	// 文字コード指定は明示的であるか
	bool bSpecified = IsValidCodeOrCPType(pLoadInfo->eCharCode);

	// 前回のコード -> ePrevCode
	EditInfo	fi;
	ECodeType ePrevCode = CODE_NONE;
	int nPrevTypeId = -1;
	if(CMRUFile().GetEditInfo( pLoadInfo->cFilePath, &fi )){
		ePrevCode = fi.m_nCharCode;
		nPrevTypeId = fi.m_nTypeId;
	}

	// タイプ別設定
	if( !pLoadInfo->nType.IsValidType() ){
		if( 0 <= nPrevTypeId ){
			pLoadInfo->nType = CDocTypeManager().GetDocumentTypeOfId(nPrevTypeId);
		}
		if( !pLoadInfo->nType.IsValidType() ){
			pLoadInfo->nType = CDocTypeManager().GetDocumentTypeOfPath( pLoadInfo->cFilePath );
		}
	}


	// 指定のコード -> pLoadInfo->eCharCode
	if( CODE_AUTODETECT == pLoadInfo->eCharCode ){
		if( fexist(pLoadInfo->cFilePath) ){
			// デフォルト文字コード認識のために一時的に読み込み対象ファイルのファイルタイプを適用する
			const STypeConfigMini* type;
			CDocTypeManager().GetTypeConfigMini(pLoadInfo->nType, &type);
			CCodeMediator cmediator( type->m_encoding );
			pLoadInfo->eCharCode = cmediator.CheckKanjiCodeOfFile( pLoadInfo->cFilePath );
		}
		else{
			pLoadInfo->eCharCode = ePrevCode;
		}
	}
	else if( CODE_NONE == pLoadInfo->eCharCode ){
		pLoadInfo->eCharCode = ePrevCode;
	}
	if(CODE_NONE==pLoadInfo->eCharCode){
		const STypeConfigMini* type;
		if( CDocTypeManager().GetTypeConfigMini(pLoadInfo->nType, &type) ){
			pLoadInfo->eCharCode = type->m_encoding.m_eDefaultCodetype;	//無効値の回避	// 2011.01.24 ryoji CODE_DEFAULT -> m_eDefaultCodetype
		}else{
			pLoadInfo->eCharCode = GetDllShareData().m_TypeBasis.m_encoding.m_eDefaultCodetype;
		}
	}

	//食い違う場合
	if(IsValidCodeOrCPType(ePrevCode) && pLoadInfo->eCharCode!=ePrevCode){
		//オプション：前回と文字コードが異なるときに問い合わせを行う
		if( GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange && !pLoadInfo->bRequestReload ){
			TCHAR szCpNameNew[260];
			TCHAR szCpNameOld[260];
			CCodePage::GetNameLong(szCpNameOld, ePrevCode);
			CCodePage::GetNameLong(szCpNameNew, pLoadInfo->eCharCode);
			ConfirmBeep();
			int nRet = MYMESSAGEBOX(
				CEditWnd::getInstance()->GetHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				LS(STR_ERR_DLGEDITDOC5),
				LS(STR_ERR_DLGEDITDOC6),
				pLoadInfo->cFilePath.c_str(),
				szCpNameNew,
				szCpNameOld,
				szCpNameOld,
				szCpNameNew
			);
			if( IDYES == nRet ){
				// 前回の文字コードを採用する
				pLoadInfo->eCharCode = ePrevCode;
			}
			else{
				// 元々使おうとしていた文字コードを採用する
				pLoadInfo->eCharCode = pLoadInfo->eCharCode;
			}
		}
		//食い違っても問い合わせを行わない場合
		else{
			//デフォルトの回答
			//  自動判別の場合：前回の文字コードを採用
			//  明示指定の場合：明示指定の文字コードを採用
			if(!bSpecified){ //自動判別
				pLoadInfo->eCharCode = ePrevCode;
			}
			else{ //明示指定
				pLoadInfo->eCharCode = pLoadInfo->eCharCode;
			}
		}
	}
}


void CMruListener::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	CMRUFile		cMRU;

	EditInfo	eiOld;
	bool bIsExistInMRU = cMRU.GetEditInfo(pcDoc->m_cDocFile.GetFilePath(),&eiOld);

	//キャレット位置の復元
	if( bIsExistInMRU && GetDllShareData().m_Common.m_sFile.GetRestoreCurPosition() ){
		//キャレット位置取得
		CLayoutPoint ptCaretPos;
		pcDoc->m_cLayoutMgr.LogicToLayout(eiOld.m_ptCursor, &ptCaretPos);

		//ビュー取得
		CEditView& cView = pcDoc->m_pcEditWnd->GetActiveView();

		if( ptCaretPos.GetY2() >= pcDoc->m_cLayoutMgr.GetLineCount() ){
			//ファイルの最後に移動
			cView.GetCommander().HandleCommand( F_GOFILEEND, false, 0, 0, 0, 0 );
		}
		else{
			cView.GetTextArea().SetViewTopLine( eiOld.m_nViewTopLine ); // 2001/10/20 novice
			cView.GetTextArea().SetViewLeftCol( eiOld.m_nViewLeftCol ); // 2001/10/20 novice
			// From Here Mar. 28, 2003 MIK
			// 改行の真ん中にカーソルが来ないように。
			const CDocLine *pTmpDocLine = pcDoc->m_cDocLineMgr.GetLine( eiOld.m_ptCursor.GetY2() );	// 2008.08.22 ryoji 改行単位の行番号を渡すように修正
			if( pTmpDocLine ){
				if( pTmpDocLine->GetLengthWithoutEOL() < eiOld.m_ptCursor.x ) ptCaretPos.x--;
			}
			// To Here Mar. 28, 2003 MIK
			cView.GetCaret().MoveCursor( ptCaretPos, true );
			cView.GetCaret().m_nCaretPosX_Prev = cView.GetCaret().GetCaretLayoutPos().GetX2();
		}
	}

	// ブックマーク復元  // 2002.01.16 hor
	if( bIsExistInMRU ){
		if( GetDllShareData().m_Common.m_sFile.GetRestoreBookmarks() ){
			CBookmarkManager(&pcDoc->m_cDocLineMgr).SetBookMarks(eiOld.m_szMarkLines);
		}
	}
	else{
		eiOld.m_szMarkLines[0] = L'\0';
	}

	// MRUリストへの登録
	EditInfo	eiNew;
	pcDoc->GetEditInfo(&eiNew);
	// 2014.07.04 ブックマークの保持(エディタが落ちたときブックマークが消えるため)
	if( bIsExistInMRU ){
		if( GetDllShareData().m_Common.m_sFile.GetRestoreBookmarks() ){
			// SetBookMarksでデータがNUL区切りに書き換わっているので再取得
			cMRU.GetEditInfo(pcDoc->m_cDocFile.GetFilePath(),&eiOld);
			auto_strcpy(eiNew.m_szMarkLines, eiOld.m_szMarkLines);
		}
	}
	cMRU.Add( &eiNew );
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       クローズ前後                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

ECallbackResult CMruListener::OnBeforeClose()
{
	//	Mar. 30, 2003 genta サブルーチンにまとめた
	_HoldBookmarks_And_AddToMRU();

	return CALLBACK_CONTINUE;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ヘルパ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	カレントファイルをMRUに登録する。
	ブックマークも一緒に登録する。

	@date 2003.03.30 genta 作成

*/
void CMruListener::_HoldBookmarks_And_AddToMRU()
{
	//EditInfo取得
	CEditDoc* pcDoc = GetListeningDoc();
	EditInfo	fi;
	pcDoc->GetEditInfo( &fi );

	//ブックマーク情報の保存
	wcscpy( fi.m_szMarkLines, CBookmarkManager(&pcDoc->m_cDocLineMgr).GetBookMarks() );

	//MRUリストに登録
	CMRUFile	cMRU;
	cMRU.Add( &fi );
}

