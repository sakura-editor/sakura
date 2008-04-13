#include "stdafx.h"
#include "CMruListener.h"
#include "doc/CEditDoc.h"
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
ECallbackResult CMruListener::OnCheckLoad(SLoadInfo* pLoadInfo)
{
	CEditDoc* pcDoc = GetListeningDoc();
	bool bConfirmCodeChange = GetDllShareData().m_Common.m_sFile.m_bQueryIfCodeChange;

	/* MRUリストに存在するか調べる  存在するならばファイル情報を返す */
	CMRU		cMRU;
	BOOL		bIsExistInMRU;
	EditInfo	fi;
	if ( cMRU.GetEditInfo( pLoadInfo->cFilePath, &fi ) ){
		bIsExistInMRU = TRUE;

		// 前回に指定された文字コード種別に変更する
		if( CODE_NONE == pcDoc->m_cDocFile.m_sFileInfo.eCharCode ){
			pcDoc->m_cDocFile.m_sFileInfo.eCharCode = fi.m_nCharCode;
		}
		
		// 文字コード指定の再オープンなら前回を無視
		if( !bConfirmCodeChange && CODE_AUTODETECT==pcDoc->m_cDocFile.m_sFileInfo.eCharCode ){
			pcDoc->m_cDocFile.m_sFileInfo.eCharCode = fi.m_nCharCode;
		}

		// 存在しないファイルの文字コード指定なしなら前回を継承
		if( !fexist(pLoadInfo->cFilePath) && CODE_AUTODETECT==pcDoc->m_cDocFile.m_sFileInfo.eCharCode ){
			pcDoc->m_cDocFile.m_sFileInfo.eCharCode = fi.m_nCharCode;
		}
	}
	else {
		bIsExistInMRU = FALSE;
	}

	/* 文字コードが異なるときに確認する */
	if( bConfirmCodeChange && bIsExistInMRU ){
		if (pcDoc->m_cDocFile.m_sFileInfo.eCharCode != fi.m_nCharCode ) {	// MRU の文字コードと判別が異なる
			const TCHAR* pszCodeName = NULL;
			const TCHAR* pszCodeNameNew = NULL;

			// gm_pszCodeNameArr_Normal を使うように変更 Moca. 2002/05/26
			if(IsValidCodeType(fi.m_nCharCode)){
				pszCodeName = CCodeTypeName(fi.m_nCharCode).Normal();
			}
			if(IsValidCodeType(pcDoc->m_cDocFile.m_sFileInfo.eCharCode)){
				pszCodeNameNew = CCodeTypeName(pcDoc->m_cDocFile.m_sFileInfo.eCharCode).Normal();
			}
			if( pszCodeName != NULL ){
				::MessageBeep( MB_ICONQUESTION );
				int nRet = MYMESSAGEBOX(
					CEditWnd::Instance()->GetHwnd(),
					MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
					_T("文字コード情報"),
					_T("%ts\n\nこのファイルは、前回は別の文字コード %ts で開かれています。\n")
					_T("前回と同じ文字コードを使いますか？\n")
					_T("\n")
					_T("・[はい(Y)]  ＝%ts\n")
					_T("・[いいえ(N)]＝%ts\n")
					_T("・[キャンセル]＝開きません"),
					pcDoc->m_cDocFile.GetFilePath(),
					pszCodeName,
					pszCodeName,
					pszCodeNameNew
				);
				if( IDYES == nRet ){
					/* 前回に指定された文字コード種別に変更する */
					pcDoc->m_cDocFile.m_sFileInfo.eCharCode = fi.m_nCharCode;
				}
				else if( IDCANCEL == nRet ){
					pcDoc->m_cDocFile.m_sFileInfo.eCharCode = CODE_DEFAULT;
					//	Sep. 10, 2002 genta
					pcDoc->SetFilePathAndIcon( _T("") );
					return CALLBACK_INTERRUPT;
				}
			}else{
				PleaseReportToAuthor(
					CEditWnd::Instance()->GetHwnd(),
					_T("バグじゃぁあああ！！！")
				);
				//	Sep. 10, 2002 genta
				pcDoc->SetFilePathAndIcon( _T("") );
				return CALLBACK_INTERRUPT;
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

