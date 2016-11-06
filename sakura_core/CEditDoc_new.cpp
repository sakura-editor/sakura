/*!	@file
	@brief 文書関連情報の管理

	@author Norio Nakatani
	
	@date aroka 警告対策で変数除去
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, Stonee, Misaka, hor, YAZAKI
	Copyright (C) 2002, hor, genta, aroka, Moca, MIK, ai
	Copyright (C) 2003, MIK, zenryaku, genta, little YOSHI
	Copyright (C) 2004, genta
	Copyright (C) 2005, genta, D.S.Koba, ryoji
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, maru, Moca, genta
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CEditDoc.h"
#include "charcode.h"
#include "CFuncInfoArr.h"// 2002/2/10 aroka
#include "CDocLine.h"// 2002/2/10 aroka
#include "CEditWnd.h"
#include "Debug.h"
#include "etc_uty.h"
#include "my_icmp.h" // Nov. 29, 2002 genta/moca
#include "mymessage.h"	//	Oct. 9, 2004 genta
#include "CControlTray.h"

//	From Here Aug. 21, 2000 genta
//
//	自動保存を行うかどうかのチェック
//
void CEditDoc::CheckAutoSave(void)
{
	if( m_cAutoSave.CheckAction() ){
		//	上書き保存

		if( !IsModified() )	//	変更無しなら何もしない
			return;				//	ここでは，「無変更でも保存」は無視する

		//	2003.10.09 zenryaku 保存失敗エラーの抑制
		if( !IsValidPath() )	//	まだファイル名が設定されていなければ保存しない
			return;

		bool en = m_cAutoSave.IsEnabled();
		m_cAutoSave.Enable(false);	//	2重呼び出しを防ぐため
		SaveFile( GetFilePath() );	//	保存（m_nCharCode, m_cSaveLineCodeを変更しない）
		m_cAutoSave.Enable(en);
	}
}

//
//	設定変更を自動保存動作に反映する
//
void CEditDoc::ReloadAutoSaveParam(void)
{
	m_cAutoSave.SetInterval( m_pShareData->m_Common.m_sBackup.GetAutoBackupInterval() );
	m_cAutoSave.Enable( m_pShareData->m_Common.m_sBackup.IsAutoBackupEnabled() );
}


//	ファイルの保存機能をEditViewから移動
//
bool CEditDoc::SaveFile( const char* pszPath )
{
	// 2006.09.01 ryoji 保存前自動実行マクロを実行する
	RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnSave, pszPath );

	if( FileWrite( pszPath, m_cSaveLineCode ) ){
		SetModified(false,true);	//	Jan. 22, 2002 genta

		/* 現在位置で無変更な状態になったことを通知 */
		m_cOpeBuf.SetNoModified();
		return true;
	}
	return false;
}

//	To Here Aug. 21, 2000 genta

/*! ブックマークリスト作成（無理矢理！）

	@date 2001.12.03 hor   新規作成
	@date 2002.01.19 aroka 空行をマーク対象にするフラグ bMarkUpBlankLineEnable を導入しました。
	@date 2005.10.11 ryoji "ａ@" の右２バイトが全角空白と判定される問題の対処
	@date 2005.11.03 genta 文字列長修正．右端のゴミを除去
*/
void CEditDoc::MakeFuncList_BookMark( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int		nLineLen;
	int		nLineCount;
	int		leftspace, pos_wo_space, k;
	BOOL	bMarkUpBlankLineEnable = m_pShareData->m_Common.m_sOutline.m_bMarkUpBlankLineEnable;	//! 空行をマーク対象にするフラグ 20020119 aroka
	int		nNewLineLen	= m_cNewLineCode.GetLen();
	int		nLineLast	= m_cDocLineMgr.GetLineCount();
	int		nCharChars;

	for( nLineCount = 0; nLineCount <  nLineLast; ++nLineCount ){
		if(!m_cDocLineMgr.GetLine(nLineCount)->IsBookmarked())continue;
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		// Jan, 16, 2002 hor
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if( nLineLen<=nNewLineLen && nLineCount< nLineLast ){
			  continue;
			}
		}// LTrim
		for( leftspace = 0; leftspace < nLineLen; ++leftspace ){
			if( pLine[leftspace] == ' ' ||
				pLine[leftspace] == '\t'){
				continue;
			}else if( (unsigned char)pLine[leftspace] == (unsigned char)0x81
				&& (unsigned char)pLine[leftspace + 1] == (unsigned char)0x40 ){
				++leftspace;
				continue;
			}
			break;
		}
		
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if(( leftspace >= nLineLen-nNewLineLen && nLineCount< nLineLast )||
				( leftspace >= nLineLen )) {
				continue;
			}
		}// RTrim
		// 2005.10.11 ryoji 右から遡るのではなく左から探すように修正（"ａ@" の右２バイトが全角空白と判定される問題の対処）
		k = pos_wo_space = leftspace;
		while( k < nLineLen ){
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, k );
			if( 1 == nCharChars ){
				if( !(pLine[k] == CR ||
						pLine[k] == LF ||
						pLine[k] == SPACE ||
						pLine[k] == TAB ||
						pLine[k] == '\0') )
					pos_wo_space = k + nCharChars;
			}
			else if( 2 == nCharChars ){
				if( !((unsigned char)pLine[k] == (unsigned char)0x81 && (unsigned char)pLine[k + 1] == (unsigned char)0x40) )
					pos_wo_space = k + nCharChars;
			}
			k += nCharChars;
		}
		//	Nov. 3, 2005 genta 文字列長計算式の修正
		std::string strText( &pLine[leftspace], pos_wo_space - leftspace );

		CLayoutPoint ptXY;
		m_cLayoutMgr.LogicToLayout(	0, nLineCount, &ptXY.x, &ptXY.y );
		pcFuncInfoArr->AppendData( nLineCount+1, ptXY.y+1 , strText.c_str(), 0 );
	}
	return;
}

// From Here Jan. 22, 2002 genta
/*! 変更フラグの設定

	@param flag [in] 設定する値．true: 変更有り / false: 変更無し
	@param redraw [in] true: タイトルの再描画を行う / false: 行わない
	
	@author genta
	@date 2002.01.22 新規作成
*/
void CEditDoc::SetModified( bool flag, bool redraw)
{
	if( m_bIsModified == flag )	//	変更がなければ何もしない
		return;

	m_bIsModified = flag;
	if( redraw )
		m_pcEditWnd->UpdateCaption();
}
// From Here Jan. 22, 2002 genta

/*! ファイル名(パスなし)を取得する
	@author Moca
	@date 2002.10.13
*/
const char * CEditDoc::GetFileName( void ) const
{
	const char *p, *pszName;
	pszName = p = GetFilePath();
	while( *p != '\0'  ){
		if( _IS_SJIS_1( (unsigned char)*p ) && _IS_SJIS_2( (unsigned char)p[1] ) ){
			p+=2;
		}else if( *p == '\\' ){
			pszName = p + 1;
			p++;
		}else{
			p++;
		}
	}
	return pszName;
}

/*!
	アイコンの設定
	
	タイプ別設定に応じてウィンドウアイコンをファイルに関連づけられた物，
	または標準のものに設定する．
	
	@author genta
	@date 2002.09.10
*/
void CEditDoc::SetDocumentIcon(void)
{
	HICON	hIconBig, hIconSmall;
	
	if( m_bGrepMode )	// Grepモードの時はアイコンを変更しない
		return;
	
	if( GetDocumentAttribute().m_bUseDocumentIcon )
		m_pcEditWnd->GetRelatedIcon( GetFilePath(), &hIconBig, &hIconSmall );
	else
		m_pcEditWnd->GetDefaultIcon( &hIconBig, &hIconSmall );

	m_pcEditWnd->SetWindowIcon( hIconBig, ICON_BIG );
	m_pcEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
}

/*!
	カレントファイルをMRUに登録する。
	ブックマークも一緒に登録する。

	@date 2003.03.30 genta 作成

*/
void CEditDoc::AddToMRU(void)
{
	EditInfo	fi;
	CMRUFile	cMRU;

	GetEditInfo( &fi );
	strcpy( fi.m_szMarkLines, m_cDocLineMgr.GetBookMarks() );

	//@@@ 2001.12.26 YAZAKI MRUリストは、CMRUに依頼する
	cMRU.Add( &fi );
}


/*!	@brief 指定されたファイルを開く

	現在の編集状況に応じて，現在のウィンドウに読み込むか，新しいウィンドウを開くか
	あるいは既に開かれているウィンドウにフォーカスを移すだけにするかを決定し，
	実行する．

	対象ファイル，エディタウィンドウの状況に依らず新しいファイルを開きたい場合に
	使用する．

	@date 2003.03.30 genta 「閉じて開く」から利用するために引数追加
	@date 2004.10.09 CEditViewより移動
	@date 2007.03.12 maru 重複コード(多重オープン処理部分など)をCShareData::IsPathOpenedに移動
*/
void CEditDoc::OpenFile( const char *filename, ECodeType nCharCode, bool bReadOnly )
{
	char		pszPath[_MAX_PATH];
	BOOL		bOpened;
	HWND		hWndOwner;

	/* 「ファイルを開く」ダイアログ */
	if( filename == NULL ){
		pszPath[0] = '\0';
		if( !OpenFileDialog( m_pcEditWnd->m_hWnd, NULL, pszPath, &nCharCode, &bReadOnly ) ){
			return;
		}
	}
	else {
		//	2007.10.01 genta 相対パスを絶対パスに変換
		//	変換しないとIsPathOpenedで正しい結果が得られず，
		//	同一ファイルを複数開くことがある．
		if( ! GetLongFileName( filename, pszPath )){
			//	ファイル名の変換に失敗
			OkMessage( m_pcEditWnd->m_hWnd,
				_T("ファイル名の変換に失敗しました [%s]"), filename );
			return;
		}
	}
	/* 指定ファイルが開かれているか調べる */
	if( CShareData::getInstance()->ActiveAlreadyOpenedWindow(pszPath, &hWndOwner, nCharCode) ){		// 開いていればアクティブにする
		/* 2007.03.12 maru 開いていたときの処理はCShareData::IsPathOpenedに移動 */
	}else{
		/* ファイルが開かれていない */
		/* 変更フラグがオフで、ファイルを読み込んでいない場合 */
//@@@ 2001.12.26 YAZAKI Grep結果で無い場合も含める。
		if( IsFileOpenInThisWindow()
		){
			BOOL bRet;
			/* ファイル読み込み */
			//	Oct. 03, 2004 genta コード確認は設定に依存
			bRet = FileRead( pszPath, &bOpened, nCharCode, bReadOnly,
							m_pShareData->m_Common.m_sFile.m_bQueryIfCodeChange );

			// 2006.09.01 ryoji オープン後自動実行マクロを実行する
			// 2007.06.27 maru すでに編集ウィンドウは開いているので、FileReadがキャンセルされた場合は開くマクロは実行不要
			if( FALSE!=bRet ) RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );
		}else{
			if( strchr( pszPath, ' ' ) ){
				char	szFile2[_MAX_PATH + 3];
				wsprintf( szFile2, "\"%s\"", pszPath );
				strcpy( pszPath, szFile2 );
			}
			/* 新たな編集ウィンドウを起動 */
			CControlTray::OpenNewEditor( m_hInstance, m_pcEditWnd->m_hWnd, pszPath, nCharCode, bReadOnly );
		}
	}
	return;
}

/* 閉じて(無題)

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
void CEditDoc::FileClose( void )
{
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	if( !OnFileClose() ){
		return;
	}
	/* 既存データのクリア */
	InitDoc();

	/* 全ビューの初期化 */
	InitAllView();

	/* 親ウィンドウのタイトルを更新 */
	m_pcEditWnd->UpdateCaption();

	// 2006.09.01 ryoji オープン後自動実行マクロを実行する
	RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );

	return;
}

/* 閉じて開く

	@param filename	[in] ファイル名
	@param nCharCode	[in] 文字コード
	@param bReadOnly	[in] 読み取り専用か

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
void CEditDoc::FileCloseOpen( const char *filename, ECodeType nCharCode, bool bReadOnly )
{
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	if( !OnFileClose() ){
		return;
	}

	// Mar. 30, 2003 genta
	char	pszPath[_MAX_PATH];

	if( filename == NULL ){
		pszPath[0] = '\0';
		if( !OpenFileDialog( m_pcEditWnd->m_hWnd, NULL, pszPath, &nCharCode, &bReadOnly ) ){
			return;
		}
	}

	/* 既存データのクリア */
	InitDoc();

	/* 全ビューの初期化 */
	InitAllView();

	if( !IsValidPath() ){
		CShareData::getInstance()->GetNoNameNumber( m_pcEditWnd->m_hWnd );
	}

	/* 親ウィンドウのタイトルを更新 */
	m_pcEditWnd->UpdateCaption();

	/* ファイルを開く */
	// Mar. 30, 2003 genta
	// Oct.  9, 2004 genta CEditDocへ移動したことによる変更
	OpenFile(( filename ? filename : pszPath ), nCharCode, bReadOnly );
}

/*! 上書き保存

	@param warnbeep [in] true: 保存不要 or 保存禁止のときに警告を出す
	@param askname	[in] true: ファイル名未設定の時に入力を促す

	@date 2006.12.30 ryoji CEditView::Command_FILESAVE()から処理本体を切り出し
*/
BOOL CEditDoc::FileSave( bool warnbeep, bool askname )
{

	/* 無変更でも上書きするか */
	if( !m_pShareData->m_Common.m_sFile.m_bEnableUnmodifiedOverwrite
	 && !IsModified()	// 変更フラグ
	 ){
	 	//	Feb. 28, 2004 genta
	 	//	保存不要でも警告音を出して欲しくない場合がある
	 	if( warnbeep ){
			ErrorBeep();
		}
		return TRUE;
	}

	if( !IsValidPath() ){
		if( ! askname ){
			return FALSE;
		}
		//	Feb. 28, 2004 genta SAVEASの結果が正しく返されていなかった
		//	次の処理と組み合わせるときに問題が生じる
		//return Command_FILESAVEAS_DIALOG();
		FileSaveAs_Dialog();
	}
	else {
		//	Jun.  5, 2004 genta
		//	読み取り専用のチェックをCEditDocから上書き保存処理に移動
		if( m_bReadOnly ){	/* 読み取り専用モード */
			if( warnbeep ){
				ErrorBeep();
				TopErrorMessage(
					m_pcEditWnd->m_hWnd,
					_T("%s\n\nは読み取り専用モードで開いています。 上書き保存はできません。\n\n")
					_T("名前を付けて保存をすればいいと思います。"),
					IsValidPath() ? GetFilePath() : _T("(無題)")
				);
			}
			return FALSE;
		}

		if( SaveFile( GetFilePath() ) ){	//	m_nCharCode, m_cSaveLineCodeを変更せずに保存
			/* キャレットの行桁位置を表示する */
			m_pcEditWnd->GetActiveView().ShowCaretPosInfo();
			return TRUE;
		}
	}
	return FALSE;
}

/*! 名前を付けて保存ダイアログ

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS_DIALOG()から処理本体を切り出し
*/
BOOL CEditDoc::FileSaveAs_Dialog( void )
{
	//	Aug. 16, 2000 genta
	//	現在のファイル名を初期値で与えない
	//	May 18, 2001 genta
	//	現在のファイル名を与えないのは上書き禁止の時のみ
	//	そうでない場合には現在のファイル名を初期値として設定する。
	char szPath[_MAX_PATH + 1];
	if( IsReadOnly() )
		szPath[0] = '\0';
	else
		strcpy( szPath, GetFilePath() );

	//	Feb. 9, 2001 genta
	//	Jul. 26, 2003 ryoji BOMの有無を与えるパラメータ
	if( SaveFileDialog( szPath, &m_nCharCode, &m_cSaveLineCode, &m_bBomExist ) ){
		//	Jun.  5, 2004 genta
		//	読み取り専用のチェックをCEditDocから上書き保存処理に移動
		//	同名で上書きされるのを防ぐ
		if( m_bReadOnly && strcmp( szPath, GetFilePath()) == 0 ){
			ErrorBeep();
			TopErrorMessage(
				m_pcEditWnd->m_hWnd,
				_T("読み取り専用モードでは同一ファイルへの上書き保存はできません。")
			);
		}
		else {
			//Command_FILESAVEAS( szPath );
			FileSaveAs( szPath );
			m_cSaveLineCode = EOL_NONE;	// 2008.03.20 ryoji 改行コードは引き継がない（上書き保存では常に「変換なし」）
			return TRUE;
		}
	}
	m_cSaveLineCode = EOL_NONE;	// 2008.03.20 ryoji 改行コードは引き継がない（上書き保存では常に「変換なし」）
	return FALSE;
}

/* 名前を付けて保存

	@param filename	[in] ファイル名

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
BOOL CEditDoc::FileSaveAs( const char *filename )
{
	if( SaveFile( filename ) ){
		/* キャレットの行桁位置を表示する */
		m_pcEditWnd->GetActiveView().ShowCaretPosInfo();
		OnChangeSetting();	//	タイプ別設定の変更を指示。
		//	再オープン
		//	Jul. 26, 2003 ryoji 現在開いているのと同じコードで開き直す
		ReloadCurrentFile( m_nCharCode, false );
		return TRUE;
	}
	return FALSE;
}

/*[EOF]*/
