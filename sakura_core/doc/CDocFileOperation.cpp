#include "StdAfx.h"
#include "CDocFileOperation.h"
#include "CDocVisitor.h"
#include "CEditDoc.h"

#include "recent/CMRU.h"
#include "recent/CMRUFolder.h"
#include "_main/CAppMode.h" 
#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "window/CEditWnd.h"
#include "CWaitCursor.h"
#include "util/window.h"
#include "env/DLLSHAREDATA.h"
#include "env/CSakuraEnvironment.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ロック                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDocFileOperation::_ToDoLock() const
{
	// ファイルを開いていない
	if( !m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath() )return false;

	// ビューモード
	if( CAppMode::getInstance()->IsViewMode() )return false;

	// 排他設定
	if( GetDllShareData().m_Common.m_sFile.m_nFileShareMode == SHAREMODE_NOT_EXCLUSIVE )return false;

	return true;
}

void CDocFileOperation::DoFileLock(bool bMsg)
{
	if(this->_ToDoLock()){
		m_pcDocRef->m_cDocFile.FileLock(GetDllShareData().m_Common.m_sFile.m_nFileShareMode, bMsg);
	}
}

void CDocFileOperation::DoFileUnlock()
{
	m_pcDocRef->m_cDocFile.FileUnlock();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ロードUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 「ファイルを開く」ダイアログ */
//	Mar. 30, 2003 genta	ファイル名未定時の初期ディレクトリをカレントフォルダに
bool CDocFileOperation::OpenFileDialog(
	HWND				hwndParent,		//!< [in]
	const TCHAR*		pszOpenFolder,	//!< [in]     NULL以外を指定すると初期フォルダを指定できる
	SLoadInfo*			pLoadInfo,		//!< [in/out] ロード情報
	std::vector<std::tstring>&	files
)
{
	/* アクティブにする */
	ActivateFrameWindow( hwndParent );

	// ファイルオープンダイアログを表示
	m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.Create(
		G_AppInstance(),
		hwndParent,
		_T("*.*"),
		pszOpenFolder ? pszOpenFolder : CSakuraEnvironment::GetDlgInitialDir().c_str(),	// 初期フォルダ
		CMRUFile().GetPathList(),															// MRUリストのファイルのリスト
		CMRUFolder().GetPathList()														// OPENFOLDERリストのファイルのリスト
	);
	return m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.DoModalOpenDlg( pLoadInfo, &files );
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ロードフロー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDocFileOperation::DoLoadFlow(SLoadInfo* pLoadInfo)
{
	ELoadResult eLoadResult = LOADED_FAILURE;

	try{
		//ロード前チェック
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyCheckLoad(pLoadInfo))throw CFlowInterruption();

		//ロード処理
		m_pcDocRef->NotifyBeforeLoad(pLoadInfo);			//前処理
		eLoadResult = m_pcDocRef->NotifyLoad(*pLoadInfo);	//本処理
		m_pcDocRef->NotifyAfterLoad(*pLoadInfo);			//後処理
	}
	catch(CFlowInterruption){
		eLoadResult = LOADED_INTERRUPT;
	}
	catch(...){
		//予期せぬ例外が発生した場合も NotifyFinalLoad は必ず呼ぶ！
		m_pcDocRef->NotifyFinalLoad(LOADED_FAILURE);
		throw;
	}

	//最終処理
	m_pcDocRef->NotifyFinalLoad(eLoadResult);

	return eLoadResult==LOADED_OK;
}

//! ファイルを開く
bool CDocFileOperation::FileLoad(
	SLoadInfo*	pLoadInfo		//!< [in/out]
)
{
	bool bRet = DoLoadFlow(pLoadInfo);
	// 2006.09.01 ryoji オープン後自動実行マクロを実行する
	if( bRet ) m_pcDocRef->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnOpened );
	return bRet;
}

//! ファイルを開く（自動実行マクロを実行しない）
//	2009.08.11 ryoji FileLoadへのパラメータ追加にしてもいいがANSI版と整合がとりやすいので当面は別関数にしておく
bool CDocFileOperation::FileLoadWithoutAutoMacro(
	SLoadInfo*	pLoadInfo		//!< [in/out]
)
{
	return DoLoadFlow(pLoadInfo);
}

//! 同一ファイルの再オープン
void CDocFileOperation::ReloadCurrentFile(
	ECodeType	nCharCode		//!< [in] 文字コード種別
)
{
	if( !fexist(m_pcDocRef->m_cDocFile.GetFilePath()) ){
		/* ファイルが存在しない */
		//	Jul. 26, 2003 ryoji BOMを標準設定に
		m_pcDocRef->m_cDocFile.m_sFileInfo.eCharCode = nCharCode;
		switch( m_pcDocRef->m_cDocFile.m_sFileInfo.eCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_pcDocRef->m_cDocFile.m_sFileInfo.bBomExist = true;
			break;
		case CODE_UTF8:
		default:
			m_pcDocRef->m_cDocFile.m_sFileInfo.bBomExist = false;
			break;
		}
		// カーソル位置表示を更新する	// 2008.07.22 ryoji
		m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().ShowCaretPosInfo();
		return;
	}

	//カーソル位置保存
	CLayoutInt		nViewTopLine = m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	/* 表示域の一番上の行(0開始) */
	CLayoutInt		nViewLeftCol = m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	/* 表示域の一番左の桁(0開始) */
	CLayoutPoint	ptCaretPosXY = m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos();

	//ロード
	SLoadInfo sLoadInfo;
	sLoadInfo.cFilePath=m_pcDocRef->m_cDocFile.GetFilePath();
	sLoadInfo.eCharCode=nCharCode;
	sLoadInfo.bViewMode=CAppMode::getInstance()->IsViewMode();
	sLoadInfo.bRequestReload=true;
	bool bRet = this->DoLoadFlow(&sLoadInfo);

	// カーソル位置復元 (※ここではオプションのカーソル位置復元（＝改行単位）が指定されていない場合でも復元する)
	// 2007.08.23 ryoji 表示領域復元
	if( ptCaretPosXY.GetY2() < m_pcDocRef->m_cLayoutMgr.GetLineCount() ){
		m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().SetViewTopLine(nViewTopLine);
		m_pcDocRef->m_pcEditWnd->GetActiveView().GetTextArea().SetViewLeftCol(nViewLeftCol);
	}
	m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().MoveCursorProperly( ptCaretPosXY, TRUE );	// 2007.08.23 ryoji MoveCursor()->MoveCursorProperly()
	m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().m_nCaretPosX_Prev = m_pcDocRef->m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX2();

	// 2006.09.01 ryoji オープン後自動実行マクロを実行する
	if( bRet ) m_pcDocRef->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnOpened );
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         セーブUI                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //



//pszOpenFolder pszOpenFolder


/*! 「ファイル名を付けて保存」ダイアログ
	@date 2001.02.09 genta	改行コードを示す引数追加
	@date 2003.03.30 genta	ファイル名未定時の初期ディレクトリをカレントフォルダに
	@date 2003.07.20 ryoji	BOMの有無を示す引数追加
	@date 2006.11.10 ryoji	ユーザー指定の拡張子を状況依存で変化させる
*/
bool CDocFileOperation::SaveFileDialog(
	SSaveInfo*	pSaveInfo	//!< [out]
)
{
	//ダイアログオープン時のディレクトリを決定
	std::tstring strDefFolder; // デフォルトフォルダ
	{
		// ファイルパスのディレクトリ
		strDefFolder = this->m_pcDocRef->m_cDocFile.GetFilePathClass().GetDirPath();

		// カレントディレクトリ
		if(strDefFolder.length()==0){
			TCHAR	buf[_MAX_PATH];
			int		n = ::GetCurrentDirectory( _MAX_PATH, buf );
			if( 0 != n && n < _MAX_PATH )strDefFolder = buf;
		}
	}

	//拡張子指定
	// 一時適用や拡張子なしの場合の拡張子をタイプ別設定から持ってくる
	// 2008/6/14 大きく改造 Uchi
	TCHAR	szDefaultWildCard[_MAX_PATH + 10];	// ユーザー指定拡張子
	{
		LPCTSTR	szExt;
		TCHAR	szWork[MAX_TYPES_EXTS];
		TCHAR*	pStr;
		TCHAR*	pEnd;

		CTypeConfig	nSettingType = m_pcDocRef->m_cDocType.GetDocumentType();
		//ファイルパスが無い場合は *.txt とする
		if(!this->m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath()){
			szExt = _T("");
		}
		else {
			szExt = this->m_pcDocRef->m_cDocFile.GetFilePathClass().GetExt();
		}
		if (nSettingType.GetIndex() == 0) {
			// 基本
			if (szExt[0] == _T('\0')) { 
				// ファイルパスが無いまたは拡張子なし
				_tcscpy(szDefaultWildCard, _T("*.txt"));
			}
			else {
				// 拡張子あり
				_tcscpy(szDefaultWildCard, _T("*"));
				_tcscat(szDefaultWildCard, szExt);
			}
		}
		else {
			szDefaultWildCard[0] = _T('\0'); 
			if (szExt[0] != _T('\0')) {
				// ファイルパスがあり、拡張子ありの場合、トップに指定
				_tcscpy(szDefaultWildCard, _T("*"));
				_tcscat(szDefaultWildCard, szExt);
			}
			// 拡張子を指定に合わせる
			pStr = pEnd = CDocTypeManager().GetTypeSetting(nSettingType).m_szTypeExts;
			do {
				if (*pEnd == _T('\0') || *pEnd == _T(',')) {
					auto_strncpy(szWork, pStr, pEnd - pStr);
					szWork[pEnd - pStr]= _T('\0');
					if (szExt[0] == _T('\0') || auto_stricmp(szWork, szExt + 1) != 0) {
						// 拡張子指定なし、またはマッチした拡張子でない
						if (szDefaultWildCard[0] != _T('\0')) {
							_tcscat(szDefaultWildCard, _T(";"));
						}
						_tcscat(szDefaultWildCard, _T("*."));
						_tcscat(szDefaultWildCard, szWork);
					}
					pStr = pEnd + 1;
				}
			} while	(*pEnd++ != _T('\0'));
		}

		if(!this->m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath()){
			//「新規から保存時は全ファイル表示」オプション	// 2008/6/15 バグフィックス Uchi
			if( GetDllShareData().m_Common.m_sFile.m_bNoFilterSaveNew )
				_tcscat(szDefaultWildCard, _T(";*.*"));	// 全ファイル表示
		}
		else {
			//「新規以外から保存時は全ファイル表示」オプション
			if( GetDllShareData().m_Common.m_sFile.m_bNoFilterSaveFile )
				_tcscat(szDefaultWildCard, _T(";*.*"));	// 全ファイル表示
		}
	}

	// ダイアログを表示
	m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.Create(
		G_AppInstance(),
		CEditWnd::getInstance()->GetHwnd(),
		szDefaultWildCard,
		strDefFolder.c_str(),
		CMRUFile().GetPathList(),		//	最近のファイル
		CMRUFolder().GetPathList()	//	最近のフォルダ
	);
	return m_pcDocRef->m_pcEditWnd->m_cDlgOpenFile.DoModalSaveDlg( pSaveInfo, pSaveInfo->eCharCode == CODE_CODEMAX );
}

//!「ファイル名を付けて保存」ダイアログ
bool CDocFileOperation::SaveFileDialog(LPTSTR szPath)
{
	SSaveInfo sSaveInfo;
	sSaveInfo.cFilePath = szPath;
	sSaveInfo.eCharCode = CODE_CODEMAX; //###トリッキー
	bool bRet = SaveFileDialog(&sSaveInfo);
	_tcscpy_s(szPath, _MAX_PATH, sSaveInfo.cFilePath);
	return bRet;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       セーブフロー                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CDocFileOperation::DoSaveFlow(SSaveInfo* pSaveInfo)
{
	ESaveResult eSaveResult = SAVED_FAILURE;

	try{
		//オプション：無変更でも上書きするか
		// 2009.04.12 ryoji CSaveAgent::OnCheckSave()から移動
		// ### 無変更なら上書きしないで抜ける処理はどの CDocListener の OnCheckSave() よりも前に
		// ### （保存するかどうか問い合わせたりするよりも前に）やるぺきことなので、
		// ### スマートじゃない？かもしれないけど、とりあえずここに配置しておく
		if( !GetDllShareData().m_Common.m_sFile.m_bEnableUnmodifiedOverwrite ){
			// 上書きの場合
			if(pSaveInfo->bOverwriteMode){
				// 無変更の場合は警告音を出し、終了
				if(!m_pcDocRef->m_cDocEditor.IsModified() && pSaveInfo->cEol==EOL_NONE){ //※改行コード指定保存がリクエストされた場合は、「変更があったもの」とみなす
					CEditApp::getInstance()->m_cSoundSet.NeedlessToSaveBeep();
					throw CFlowInterruption();
				}
			}
		}

		//セーブ前チェック
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyCheckSave(pSaveInfo))throw CFlowInterruption();

		//セーブ前おまけ処理
		if(CALLBACK_INTERRUPT==m_pcDocRef->NotifyPreBeforeSave(pSaveInfo))throw CFlowInterruption();

		// 2006.09.01 ryoji 保存前自動実行マクロを実行する
		m_pcDocRef->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnSave, pSaveInfo->cFilePath );

		//セーブ処理
		m_pcDocRef->NotifyBeforeSave(*pSaveInfo);	//前処理
		m_pcDocRef->NotifySave(*pSaveInfo);			//本処理
		m_pcDocRef->NotifyAfterSave(*pSaveInfo);	//後処理

		//結果
		eSaveResult = SAVED_OK; //###仮
	}
	catch(CFlowInterruption){
		eSaveResult = SAVED_INTERRUPT;
	}
	catch(...){
		//予期せぬ例外が発生した場合も NotifyFinalSave は必ず呼ぶ！
		m_pcDocRef->NotifyFinalSave(SAVED_FAILURE);
		throw;
	}

	//最終処理
	m_pcDocRef->NotifyFinalSave(eSaveResult);

	return eSaveResult==SAVED_OK;
}


/*! 上書き保存

	@return 保存が行われたor保存不要のため何も行わなかったときにtrueを返す

	@date 2004.06.05 genta  ビューモードのチェックをCEditDocから上書き保存処理に移動
	@date 2006.12.30 ryoji  CEditView::Command_FILESAVE()から処理本体を切り出し
	@date 2008.03.20 kobake 戻り値の仕様を定義
*/
bool CDocFileOperation::FileSave()
{
	//ファイル名が指定されていない場合は「名前を付けて保存」のフローへ遷移
	if( !m_pcDocRef->m_cDocFile.GetFilePathClass().IsValidPath() ){
		return FileSaveAs();
	}

	//セーブ情報
	SSaveInfo sSaveInfo;
	m_pcDocRef->GetSaveInfo(&sSaveInfo);
	sSaveInfo.cEol = EOL_NONE; //改行コード無変換
	sSaveInfo.bOverwriteMode = true; //上書き要求

	//上書き処理
	return m_pcDocRef->m_cDocFileOperation.DoSaveFlow(&sSaveInfo);
}



/*! 名前を付けて保存フロー

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS_DIALOG()から処理本体を切り出し
*/
bool CDocFileOperation::FileSaveAs( const WCHAR* filename, EEolType eEolType )
{
	//セーブ情報
	SSaveInfo sSaveInfo;
	m_pcDocRef->GetSaveInfo(&sSaveInfo);
	if( filename ){
		sSaveInfo.cFilePath = to_tchar(filename);
		sSaveInfo.cEol = eEolType;
	}else{
		if(CAppMode::getInstance()->IsViewMode())sSaveInfo.cFilePath = _T(""); //※読み込み専用モードのときはファイル名を指定しない

		//ダイアログ表示
		if(!SaveFileDialog(&sSaveInfo))return false;
	}

	//セーブ処理
	if( DoSaveFlow(&sSaveInfo) ){
		// オープン後自動実行マクロを実行する（ANSI版ではここで再ロード実行→自動実行マクロが実行される）
		// 提案時の Patches#1550557 に、「名前を付けて保存」でオープン後自動実行マクロが実行されることの是非について議論の経緯あり
		//   →”ファイル名に応じて表示を変化させるマクロとかを想定すると、これはこれでいいように思います。”
		m_pcDocRef->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnOpened );
		return true;
	}

	return false;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         クローズ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*
	閉じて(無題)。
	ユーザキャンセル操作等によりクローズされなかった場合は false を返す。

	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
bool CDocFileOperation::FileClose()
{
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	if( !m_pcDocRef->OnFileClose() ){
		return false;
	}

	/* 既存データのクリア */
	m_pcDocRef->InitDoc();

	/* 全ビューの初期化 */
	m_pcDocRef->InitAllView();

	/* 親ウィンドウのタイトルを更新 */
	m_pcDocRef->m_pcEditWnd->UpdateCaption();

	// 2006.09.01 ryoji オープン後自動実行マクロを実行する
	m_pcDocRef->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnOpened );

	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          その他                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* 閉じて開く
	@date 2006.12.30 ryoji CEditView::Command_FILESAVEAS()から処理本体を切り出し
*/
void CDocFileOperation::FileCloseOpen( const SLoadInfo& _sLoadInfo )
{
	/* ファイルを閉じるときのMRU登録 & 保存確認 & 保存実行 */
	if( !m_pcDocRef->OnFileClose() ){
		return;
	}

	//ファイル名指定が無い場合はダイアログで入力させる
	SLoadInfo sLoadInfo = _sLoadInfo;
	if( sLoadInfo.cFilePath.Length()==0 ){
		std::vector<std::tstring> files;
		if( !OpenFileDialog( CEditWnd::getInstance()->GetHwnd(), NULL, &sLoadInfo, files ) ){
			return;
		}
		sLoadInfo.cFilePath = files[0].c_str();
		// 他のファイルは新規ウィンドウ
		for( size_t i = 1; i < files.size(); i++ ){
			SLoadInfo sFilesLoadInfo = sLoadInfo;
			sFilesLoadInfo.cFilePath = files[i].c_str();
			CControlTray::OpenNewEditor(
				G_AppInstance(),
				CEditWnd::getInstance()->GetHwnd(),
				sFilesLoadInfo,
				NULL,
				true
			);
		}
	}

	/* 既存データのクリア */
	m_pcDocRef->InitDoc();

	/* 全ビューの初期化 */
	m_pcDocRef->InitAllView();

	/* 親ウィンドウのタイトルを更新 */
	m_pcDocRef->m_pcEditWnd->UpdateCaption();

	//開く
	FileLoadWithoutAutoMacro(&sLoadInfo);

	// オープン後自動実行マクロを実行する
	// ※ロードしてなくても(無題)には変更済み
	m_pcDocRef->RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnOpened );
}
