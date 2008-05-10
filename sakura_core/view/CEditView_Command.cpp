#include "stdafx.h"
#include "view/CEditView.h"
#include "doc/CEditDoc.h"
#include "util/file.h"
#include "util/module.h"
#include "mymessage.h"
#include "util/window.h"
#include "CControlTray.h"
#include <io.h>
#include "doc/CLayout.h"
#include "charset/charcode.h"

/*
	指定ファイルの指定位置にタグジャンプする。

	@author	MIK
	@date	2003.04.13	新規作成
	@date	2003.04.21 genta bClose追加
	@date	2004.05.29 Moca 0以下が指定されたときは、善処する
	@date	2007.02.17 genta 相対パスの基準ディレクトリ指示を追加
*/
bool CEditView::TagJumpSub(
	const TCHAR*	pszFileName,
	CMyPoint		ptJumpTo,
	bool			bClose,			//!< [in] true: 元ウィンドウを閉じる / false: 元ウィンドウを閉じない
	bool			bRelFromIni
)
{
	HWND	hwndOwner;
	POINT	poCaret;
	// 2004/06/21 novice タグジャンプ機能追加
	TagJump	tagJump;

	// 参照元ウィンドウ保存
	tagJump.hwndReferer = CEditWnd::Instance()->GetHwnd();

	//	Feb. 17, 2007 genta 実行ファイルからの相対指定の場合は
	//	予め絶対パスに変換する．(キーワードヘルプジャンプで用いる)
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	TCHAR	szJumpToFile[1024];
	if( bRelFromIni && _IS_REL_PATH( pszFileName ) ){
		GetInidirOrExedir( szJumpToFile, pszFileName );
	}
	else {
		_tcscpy( szJumpToFile, pszFileName );
	}

	/* ロングファイル名を取得する */
	TCHAR	szWork[1024];
	if( TRUE == ::GetLongFileName( szJumpToFile, szWork ) )
	{
		_tcscpy( szJumpToFile, szWork );
	}

// 2004/06/21 novice タグジャンプ機能追加
// 2004/07/05 みちばな
// 同一ファイルだとSendMesssageで GetCaret().GetCaretLayoutPos().GetX2(),GetCaret().GetCaretLayoutPos().GetY2()が更新されてしまい、
// ジャンプ先の場所がジャンプ元として保存されてしまっているので、
// その前で保存するように変更。

	/* カーソル位置変換 */
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&tagJump.point
	);

	// タグジャンプ情報の保存
	CShareData::getInstance()->PushTagJump(&tagJump);


	/* 指定ファイルが開かれているか調べる */
	/* 開かれている場合は開いているウィンドウのハンドルも返す */
	/* ファイルを開いているか */
	if( CShareData::getInstance()->IsPathOpened( szJumpToFile, &hwndOwner ) )
	{
		// 2004.05.13 Moca マイナス値は無効
		if( 0 < ptJumpTo.y ){
			/* カーソルを移動させる */
			poCaret.y = ptJumpTo.y - 1;
			if( 0 < ptJumpTo.x ){
				poCaret.x = ptJumpTo.x - 1;
			}else{
				poCaret.x = 0;
			}
			memcpy_raw( GetDllShareData().GetWorkBuffer<void>(), &poCaret, sizeof(poCaret) );
			::SendMessageAny( hwndOwner, MYWM_SETCARETPOS, 0, 0 );
		}
		/* アクティブにする */
		ActivateFrameWindow( hwndOwner );
	}
	else{
		/* 新しく開く */
		EditInfo	inf;
		bool		bSuccess;

		_tcscpy( inf.m_szPath, szJumpToFile );
		inf.m_ptCursor.Set(CLogicInt(ptJumpTo.x - 1), CLogicInt(ptJumpTo.y - 1));
		inf.m_nViewLeftCol = CLayoutInt(-1);
		inf.m_nViewTopLine = CLayoutInt(-1);
		inf.m_nCharCode    = CODE_AUTODETECT;

		bSuccess = CControlTray::OpenNewEditor2(
			G_AppInstance(),
			this->GetHwnd(),
			&inf,
			FALSE,	/* ビューモードか */
			true	//	同期モードで開く
		);

		if( ! bSuccess )	//	ファイルが開けなかった
			return false;

		//	Apr. 23, 2001 genta
		//	hwndOwnerに値が入らなくなってしまったために
		//	Tag Jump Backが動作しなくなっていたのを修正
		if( !CShareData::getInstance()->IsPathOpened( szJumpToFile, &hwndOwner ) )
			return false;
	}

	// 2006.12.30 ryoji 閉じる処理は最後に（処理位置移動）
	//	Apr. 2003 genta 閉じるかどうかは引数による
	//	grep結果からEnterでジャンプするところにCtrl判定移動
	if( bClose )
	{
		GetCommander().Command_WINCLOSE();	//	挑戦するだけ。
	}

	return true;
}



/*! 指定拡張子のファイルに対応するファイルを開く補助関数

	@param bCheckOnly 
	@param bBeepWhenMiss 
	@param file_ext 
	@param file_extno 
	@param open_ext 
	@param open_extno
	@param errmes 

	@date 2003.06.28 Moca ヘッダ・ソースファイルオープン機能のコードを統合
	@date 2008.04.09 ryoji 処理対象(file_ext)と開く対象(open_ext)の扱いが逆になっていたのを修正
*/
BOOL CEditView::OPEN_ExtFromtoExt(
	BOOL			bCheckOnly,		//!< [in] true: チェックのみ行ってファイルは開かない
	BOOL			bBeepWhenMiss,	//!< [in] true: ファイルを開けなかった場合に警告音を出す
	const TCHAR*	file_ext[],		//!< [in] 処理対象とする拡張子
	const TCHAR*	open_ext[],		//!< [in] 開く対象とする拡張子
	int				file_extno,		//!< [in] 処理対象拡張子リストの要素数
	int				open_extno,		//!< [in] 開く対象拡張子リストの要素数
	const TCHAR*	errmes			//!< [in] ファイルを開けなかった場合に表示するエラーメッセージ
)
{
//From Here Feb. 7, 2001 JEPRO 追加
	int		i;
	BOOL	bwantopen_c;
//To Here Feb. 7, 2001

	/* 編集中ファイルの拡張子を調べる */
	for( i = 0; i < file_extno; i++ ){
		if( CheckEXT( GetDocument()->m_cDocFile.GetFilePath(), file_ext[i] ) ){
			bwantopen_c = TRUE;
			goto open_c;
		}
	}
	if( bBeepWhenMiss ){
		ErrorBeep();
	}
	return FALSE;

open_c:;
//To Here Feb. 7, 2001

	TCHAR	szPath[_MAX_PATH];
	TCHAR	szDrive[_MAX_DRIVE];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	HWND	hwndOwner;

	_tsplitpath( GetDocument()->m_cDocFile.GetFilePath(), szDrive, szDir, szFname, szExt );

	for( i = 0; i < open_extno; i++ ){
		_tmakepath( szPath, szDrive, szDir, szFname, open_ext[i] );
		if( !fexist(szPath) ){
			if( i < open_extno - 1 )
				continue;
			if( bBeepWhenMiss ){
				ErrorBeep();
			}
			return FALSE;
		}
		break;
	}
	if( bCheckOnly ){
		return TRUE;
	}
//To Here Feb. 7, 2001

	/* 指定ファイルが開かれているか調べる */
	/* 開かれている場合は開いているウィンドウのハンドルも返す */
	/* ファイルを開いているか */
	if( CShareData::getInstance()->IsPathOpened( szPath, &hwndOwner ) ){
	}else{
		/* 文字コードはこのファイルに合わせる */
		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = szPath;
		sLoadInfo.eCharCode = GetDocument()->GetDocumentEncoding();
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor(
			G_AppInstance(),
			this->GetHwnd(),
			sLoadInfo,
			NULL,
			true
		);
		/* ファイルを開いているか */
		if( CShareData::getInstance()->IsPathOpened( szPath, &hwndOwner ) ){
		}else{
			ErrorMessage( this->GetHwnd(), _T("%ts\n\n%ts\n\n"), errmes, szPath );
			return FALSE;
		}
	}
	/* アクティブにする */
	ActivateFrameWindow( hwndOwner );

// 2004/06/21 novice タグジャンプ機能追加
// 2004/07/09 genta/Moca タグジャンプバックの登録が取り除かれていたが、
//            こちらでも従来どおり登録する
	TagJump	tagJump;
	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	GetDocument()->m_cLayoutMgr.LayoutToLogic(
		GetCaret().GetCaretLayoutPos(),
		&tagJump.point
	);
	tagJump.hwndReferer = CEditWnd::Instance()->GetHwnd();
	// タグジャンプ情報の保存
	CShareData::getInstance()->PushTagJump(&tagJump);
	return TRUE;
}


/*!	@brief 折り返しの動作を決定

	トグルコマンド「現在のウィンドウ幅で折り返し」を行った場合の動作を決定する
	
	@retval TGWRAP_NONE No action
	@retval TGWRAP_FULL 最大値
	@retval TGWRAP_WINDOW ウィンドウ幅
	@retval TGWRAP_PROP 設定値

	@date 2006.01.08 genta メニュー表示で同一の判定を使うため，Command_WRAPWINDOWWIDTH()より分離．
	@date 2006.01.08 genta 判定条件を見直し
*/
CEditView::TOGGLE_WRAP_ACTION CEditView::GetWrapMode( CLayoutInt* _newKetas )
{
	CLayoutInt& newKetas=*_newKetas;
	//@@@ 2002.01.14 YAZAKI 現在のウィンドウ幅で折り返されているときは、最大値にするコマンド。
	//2002/04/08 YAZAKI ときどきウィンドウ幅で折り返されないことがあるバグ修正。
	// 20051022 aroka 現在のウィンドウ幅→最大値→文書タイプの初期値 をトグルにするコマンド
	// ウィンドウ幅==文書タイプ||最大値==文書タイプ の場合があるため判定順序に注意する。
	/*	Jan.  8, 2006 genta
		じゅうじさんの要望により判定方法を再考．現在の幅に合わせるのを最優先に．
	
		基本動作： 設定値→ウィンドウ幅
			→(ウィンドウ幅と合っていなければ)→ウィンドウ幅→上へ戻る
			→(ウィンドウ幅と合っていたら)→最大値→設定値
			ただし，最大値==設定値の場合には最大値→設定値の遷移が省略されて上に戻る
			
			ウィンドウ幅が極端に狭い場合にはウィンドウ幅に合わせることは出来ないが，
			設定値と最大値のトグルは可能．

		1)現在の折り返し幅==ウィンドウ幅 : 最大値
		2)現在の折り返し幅!=ウィンドウ幅
		3)→ウィンドウ幅が極端に狭い場合
		4)　└→折り返し幅!=最大値 : 最大値
		5)　└→折り返し幅==最大値
		6)　　　└→最大値==設定値 : 変更不能
		7)　　　└→最大値!=設定値 : 設定値
		8)→ウィンドウ幅が十分にある
		9)　└→折り返し幅==最大値
		a)　　　└→最大値!=設定値 : 設定値
	 	b)　　　└→最大値==設定値 : ウィンドウ幅
		c)　└→ウィンドウ幅
	*/
	
	if (GetDocument()->m_cLayoutMgr.GetMaxLineKetas() == GetTextArea().m_nViewColNum ){
		// a)
		newKetas = CLayoutInt(MAXLINEKETAS);
		return TGWRAP_FULL;
	}
	else if( 10 > GetTextArea().m_nViewColNum - 1 ){ // 2)
		// 3)
		if( GetDocument()->m_cLayoutMgr.GetMaxLineKetas() != MAXLINEKETAS ){
			// 4)
			newKetas = CLayoutInt(MAXLINEKETAS);
			return TGWRAP_FULL;
		}
		else if( GetDocument()->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas == MAXLINEKETAS ){ // 5)
			// 6)
			return TGWRAP_NONE;
		}
		else { // 7)
			newKetas = CLayoutInt(GetDocument()->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas);
			return TGWRAP_PROP;
		}
	}
	else { // 8)
		if( GetDocument()->m_cLayoutMgr.GetMaxLineKetas() == MAXLINEKETAS && // 9)
			GetDocument()->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas != MAXLINEKETAS ){
			// a)
			newKetas = CLayoutInt(GetDocument()->m_cDocType.GetDocumentAttribute().m_nMaxLineKetas);
			return TGWRAP_PROP;
			
		}
		else {	// b) c)
			//	現在のウィンドウ幅
			newKetas = GetTextArea().m_nViewColNum;
			return TGWRAP_WINDOW;
		}
	}
}


void CEditView::AddToCmdArr( const TCHAR* szCmd )
{
	CRecentCmd	cRecentCmd;
	cRecentCmd.AppendItem( szCmd );
	cRecentCmd.Terminate();
}


/* 正規表現の検索パターンを必要に応じて更新する(ライブラリが使用できないときはFALSEを返す) */
/* 2002.01.16 hor 共通ロジックを関数にしただけ・・・ */
BOOL CEditView::ChangeCurRegexp(void)
{
	BOOL	bChangeState;
	if( !m_bCurSrchKeyMark
	 || 0 != wcscmp( m_szCurSrchKey, GetDllShareData().m_aSearchKeys[0] )
	 || m_sCurSearchOption != GetDllShareData().m_Common.m_sSearch.m_sSearchOption
	){
		bChangeState = TRUE;
	}else{
		bChangeState = FALSE;
	}

	m_bCurSrchKeyMark = true;									// 検索文字列のマーク
	wcscpy( m_szCurSrchKey, GetDllShareData().m_aSearchKeys[0] );// 検索文字列
	m_sCurSearchOption = GetDllShareData().m_Common.m_sSearch.m_sSearchOption;// 検索／置換  オプション
	/* 正規表現 */
	if( m_sCurSearchOption.bRegularExp
	 && bChangeState
	){
		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
		if( !InitRegexp( this->GetHwnd(), m_CurRegexp, true ) ){
			return FALSE;
		}
		int nFlag = 0x00;
		nFlag |= m_sCurSearchOption.bLoHiCase ? 0x01 : 0x00;
		/* 検索パターンのコンパイル */
		m_CurRegexp.Compile( m_szCurSrchKey, nFlag );
	}

	if( bChangeState ){
		/* フォーカス移動時の再描画 */
		RedrawAll();
	}

	return TRUE;
}




/*!
	カーソル行をクリップボードにコピーする

	@date 2007.10.08 ryoji 新規（Command_COPY()から処理抜き出し）
*/
void CEditView::CopyCurLine(
	bool			bAddCRLFWhenCopy,		//!< [in] 折り返し位置に改行コードを挿入するか？
	enumEOLType		neweol,					//!< [in] コピーするときのEOL。
	bool			bEnableLineModePaste	//!< [in] ラインモード貼り付けを可能にする
)
{
	if( GetSelectionInfo().IsTextSelected() ){
		return;
	}

	const CLayout*	pcLayout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( GetCaret().GetCaretLayoutPos().y );
	if( NULL == pcLayout ){
		return;
	}

	/* クリップボードに入れるべきテキストデータを、cmemBufに格納する */
	CNativeW cmemBuf;
	cmemBuf.SetString( pcLayout->GetPtr(), pcLayout->GetLengthWithoutEOL() );
	if( pcLayout->GetLayoutEol().GetLen() != 0 ){
		cmemBuf.AppendString(
			( neweol == EOL_UNKNOWN ) ?
				pcLayout->GetLayoutEol().GetValue2() : CEol(neweol).GetValue2()
		);
	}else if( bAddCRLFWhenCopy ){	// 2007.10.08 ryoji bAddCRLFWhenCopy対応処理追加
		cmemBuf.AppendString(
			( neweol == EOL_UNKNOWN ) ?
				WCODE::CRLF : CEol(neweol).GetValue2()
		);
	}

	/* クリップボードにデータcmemBufの内容を設定 */
	BOOL bSetResult = MySetClipboardData(
		cmemBuf.GetStringPtr(),
		cmemBuf.GetStringLength(),
		false,
		bEnableLineModePaste
	);
	if( !bSetResult ){
		ErrorBeep();
	}
}
