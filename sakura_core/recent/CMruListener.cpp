#include "stdafx.h"
#include "CMruListener.h"
#include "doc/CEditDoc.h"
#include "util/file.h"
#include "charset/CCodeMediator.h"

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
ECallbackResult CMruListener::OnCheckLoad(SLoadInfo* pLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	// 文字コード指定は明示的であるか
	bool bSpecified = IsValidCodeType(pLoadInfo->eCharCode);

	// 前回のコード -> ePrevCode
	EditInfo	fi;
	ECodeType ePrevCode = CODE_NONE;
	if(CMRU().GetEditInfo( pLoadInfo->cFilePath, &fi )){
		ePrevCode = fi.m_nCharCode;
	}

	// 指定のコード -> pLoadInfo->eCharCode
	if( CODE_AUTODETECT == pLoadInfo->eCharCode ){
		if( fexist(pLoadInfo->cFilePath) ){
			pLoadInfo->eCharCode = CCodeMediator::CheckKanjiCodeOfFile( pLoadInfo->cFilePath );
		}
		else{
			pLoadInfo->eCharCode = ePrevCode;
		}
	}
	else if( CODE_NONE == pLoadInfo->eCharCode ){
		pLoadInfo->eCharCode = ePrevCode;
	}
	if(CODE_NONE==pLoadInfo->eCharCode)pLoadInfo->eCharCode = CODE_DEFAULT;	//無効値の回避

	//食い違う場合
	if(IsValidCodeType(ePrevCode) && pLoadInfo->eCharCode!=ePrevCode){
		//オプション：前回と文字コードが異なるときに問い合わせを行う
		if( GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange ){
			const TCHAR* pszCodeNameOld = CCodeTypeName(ePrevCode).Normal();
			const TCHAR* pszCodeNameNew = CCodeTypeName(pLoadInfo->eCharCode).Normal();
			::MessageBeep( MB_ICONQUESTION );
			int nRet = MYMESSAGEBOX(
				CEditWnd::Instance()->GetHwnd(),
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				_T("文字コード情報"),
				_T("%ts\n")
				_T("\n")
				_T("このファイルを文字コード %ts で開こうとしていますが、前回は別の文字コード %ts で開かれています。\n")
				_T("前回と同じ文字コードを使いますか？\n")
				_T("\n")
				_T("・[はい(Y)]  ＝%ts\n")
				_T("・[いいえ(N)]＝%ts\n")
				_T("・[キャンセル]＝開きません"),
				pLoadInfo->cFilePath.c_str(),
				pszCodeNameNew,
				pszCodeNameOld,
				pszCodeNameOld,
				pszCodeNameNew
			);
			if( IDYES == nRet ){
				// 前回の文字コードを採用する
				pLoadInfo->eCharCode = ePrevCode;
			}
			else if( IDNO == nRet ){
				// 元々使おうとしていた文字コードを採用する
				pLoadInfo->eCharCode = pLoadInfo->eCharCode;
			}
			else if( IDCANCEL == nRet ){
				return CALLBACK_INTERRUPT;
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

	return CALLBACK_CONTINUE;
}

void CMruListener::OnBeforeLoad(const SLoadInfo& sLoadInfo)
{
	// Mar. 30, 2003 genta ブックマーク保存のためMRUへ登録
	_HoldBookmarks_And_AddToMRU();
}

void CMruListener::OnAfterLoad(const SLoadInfo& sLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();

	CMRU		cMRU;

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
			cView.GetCommander().HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
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
			cView.GetCaret().MoveCursor( ptCaretPos, TRUE );
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
		wcscpy(eiOld.m_szMarkLines,L"");
	}

	// MRUリストへの登録
	EditInfo	eiNew;
	pcDoc->GetEditInfo(&eiNew);
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
	CMRU	cMRU;
	cMRU.Add( &fi );
}

