#include "stdafx.h"
#include "CGrepAgent.h"
#include "util/window.h"
#include "debug/CRunningTimer.h"
#include "dlg/CDlgCancel.h"
#include "COpeBlk.h"
#include "util/module.h"
#include "io/CFileLoad.h"
#include "charset/CCodeMediator.h"
#include "parse/CWordParse.h"

CGrepAgent::CGrepAgent()
: m_bGrepMode( false )			/* Grepモードか */
, m_bGrepRunning( false )		/* Grep処理中 */
{
}

ECallbackResult CGrepAgent::OnBeforeClose()
{
	//GREP処理中は終了できない
	if( m_bGrepRunning ){
		// アクティブにする
		ActivateFrameWindow( CEditWnd::Instance()->GetHwnd() );	//@@@ 2003.06.25 MIK
		TopInfoMessage(
			CEditWnd::Instance()->GetHwnd(),
			_T("Grepの処理中です。\n")
		);
		return CALLBACK_INTERRUPT;
	}
	return CALLBACK_CONTINUE;
}



// Grep実行
DWORD CGrepAgent::DoGrep(
	CEditView*				pcViewDst,
	const CNativeW*			pcmGrepKey,
	const CNativeT*			pcmGrepFile,
	const CNativeT*			pcmGrepFolder,
	BOOL					bGrepSubFolder,
	const SSearchOption&	sSearchOption,
	ECodeType				nGrepCharSet,	// 2002/09/21 Moca 文字コードセット選択
	BOOL					bGrepOutputLine,
	int						nGrepOutputStyle
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CEditView::DoGrep" );
#endif

	this->m_bGrepRunning = true;


	int			nDummy;
	int			nHitCount = 0;
	wchar_t		szKey[_MAX_PATH];
	TCHAR		szFile[_MAX_PATH];
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel;
	int			nCharChars;
	//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	CBregexp	cRegexp;
	CNativeW	cmemMessage;
	int			nWork;
	int*		pnKey_CharCharsArr;
	pnKey_CharCharsArr = NULL;

	/*
	|| バッファサイズの調整
	*/
	cmemMessage.AllocStringBuffer( 4000 );

	pcViewDst->m_bDoing_UndoRedo		= true;


	/* アンドゥバッファの処理 */
	if( NULL != pcViewDst->m_pcOpeBlk ){	/* 操作ブロック */
//@@@2002.2.2 YAZAKI NULLじゃないと進まないので、とりあえずコメント。＆NULLのときは、new COpeBlkする。
//		while( NULL != m_pcOpeBlk ){}
//		delete m_pcOpeBlk;
//		m_pcOpeBlk = NULL;
	}
	else {
		pcViewDst->m_pcOpeBlk = new COpeBlk;
	}

	pcViewDst->m_bCurSrchKeyMark = true;								/* 検索文字列のマーク */
	wcscpy( pcViewDst->m_szCurSrchKey, pcmGrepKey->GetStringPtr() );	/* 検索文字列 */
	pcViewDst->m_sCurSearchOption.bRegularExp = sSearchOption.bRegularExp;		/* 検索／置換  1==正規表現 */
	pcViewDst->m_sCurSearchOption.bLoHiCase   = sSearchOption.bLoHiCase;			/* 検索／置換  1==英大文字小文字の区別 */
	/* 正規表現 */

	//	From Here Jun. 27 genta
	/*
		Grepを行うに当たって検索・画面色分け用正規表現バッファも
		初期化する．これはGrep検索結果の色分けを行うため．

		Note: ここで強調するのは最後の検索文字列であって
		Grep対象パターンではないことに注意
	*/
	if( pcViewDst->m_sCurSearchOption.bRegularExp ){
		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
		if( !InitRegexp( pcViewDst->GetHwnd(), pcViewDst->m_CurRegexp, true ) ){
			return 0;
		}

		/* 検索パターンのコンパイル */
		int nFlag = 0x00;
		nFlag |= pcViewDst->m_sCurSearchOption.bLoHiCase ? 0x01 : 0x00;
		pcViewDst->m_CurRegexp.Compile( pcViewDst->m_szCurSrchKey, nFlag );
	}
	//	To Here Jun. 27 genta

//まだ m_bCurSrchWordOnly = GetDllShareData().m_Common.m_bWordOnly;	/* 検索／置換  1==単語のみ検索 */

//	cDlgCancel.Create( G_AppInstance(), m_hwndParent );
//	hwndCancel = cDlgCancel.Open( MAKEINTRESOURCE(IDD_GREPRUNNING) );
	hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), pcViewDst->m_hwndParent, IDD_GREPRUNNING );

	::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE );
	::DlgItem_SetText( hwndCancel, IDC_STATIC_CURFILE, _T(" ") );	// 2002/09/09 Moca add
	::CheckDlgButton( hwndCancel, IDC_CHECK_REALTIMEVIEW, GetDllShareData().m_Common.m_sSearch.m_bGrepRealTimeView );	// 2003.06.23 Moca

	wcscpy( szKey, pcmGrepKey->GetStringPtr() );

	wcscpy( CAppMode::Instance()->m_szGrepKey, szKey );
	this->m_bGrepMode = true;

	//	2007.07.22 genta
	//	バージョン番号取得のため，処理を前の方へ移動した
	if( sSearchOption.bRegularExp ){
		if( !InitRegexp( pcViewDst->GetHwnd(), cRegexp, true ) ){
			return 0;
		}
		/* 検索パターンのコンパイル */
		int nFlag = 0x00;
		nFlag |= sSearchOption.bLoHiCase ? 0x01 : 0x00;
		if( !cRegexp.Compile( szKey, nFlag ) ){
			return 0;
		}
	}else{
		/* 検索条件の情報 */
		CSearchAgent::CreateCharCharsArr(
			szKey,
			wcslen( szKey ),
			&pnKey_CharCharsArr
		);
	}

//2002.02.08 Grepアイコンも大きいアイコンと小さいアイコンを別々にする。
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta アイコン読み込み方法変更
	hIconBig   = GetAppIcon( G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, true );

	//	Sep. 10, 2002 genta
	//	CEditWndに新設した関数を使うように
	CEditWnd*	pCEditWnd = CEditWnd::Instance();	//	Sep. 10, 2002 genta
	pCEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
	pCEditWnd->SetWindowIcon( hIconBig, ICON_BIG );

	TCHAR szPath[_MAX_PATH];
	_tcscpy( szPath, pcmGrepFolder->GetStringPtr() );
	nDummy = _tcslen( szPath );

	/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
	nCharChars = &szPath[nDummy] - CNativeT::GetCharPrev( szPath, nDummy, &szPath[nDummy] );
	if( 1 == nCharChars && szPath[nDummy - 1] == _T('\\') ){
	}else{
		_tcscat( szPath, _T("\\") );
	}
	_tcscpy( szFile, pcmGrepFile->GetStringPtr() );

	nWork = wcslen( szKey ); // 2003.06.10 Moca あらかじめ長さを計算しておく

	/* 最後にテキストを追加 */
	CNativeW	cmemWork;
	cmemMessage.AppendString( L"\r\n□検索条件  " );
	if( 0 < nWork ){
		CNativeW cmemWork2;
		cmemWork2.SetString( szKey );
		if( pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
			cmemWork2.Replace( L"\\", L"\\\\" );
			cmemWork2.Replace( L"\'", L"\\\'" );
			cmemWork2.Replace( L"\"", L"\\\"" );
		}else{
			cmemWork2.Replace( L"\'", L"\'\'" );
			cmemWork2.Replace( L"\"", L"\"\"" );
		}
		cmemWork.AppendString( L"\"" );
		cmemWork.AppendNativeData( cmemWork2 );
		cmemWork.AppendString( L"\"\r\n" );
	}else{
		cmemWork.AppendString( L"「ファイル検索」\r\n" );
	}
	cmemMessage += cmemWork;



	cmemMessage.AppendString( L"検索対象   " );
	cmemWork.SetStringT( szFile );
	if( pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;




	cmemMessage.AppendString( L"\r\n" );
	cmemMessage.AppendString( L"フォルダ   " );
	cmemWork.SetStringT( szPath );
	if( pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;
	cmemMessage.AppendString( L"\r\n" );

	const wchar_t*	pszWork;
	if( bGrepSubFolder ){
		pszWork = L"    (サブフォルダも検索)\r\n";
	}else{
		pszWork = L"    (サブフォルダを検索しない)\r\n";
	}
	cmemMessage.AppendString( pszWork );

	if( 0 < nWork ){ // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if( sSearchOption.bWordOnly ){
		/* 単語単位で探す */
			cmemMessage.AppendString( L"    (単語単位で探す)\r\n" );
		}

		if( sSearchOption.bLoHiCase ){
			pszWork = L"    (英大文字小文字を区別する)\r\n";
		}else{
			pszWork = L"    (英大文字小文字を区別しない)\r\n";
		}
		cmemMessage.AppendString( pszWork );

		if( sSearchOption.bRegularExp ){
			//	2007.07.22 genta : 正規表現ライブラリのバージョンも出力する
			cmemMessage.AppendString( L"    (正規表現:" );
			cmemMessage.AppendStringT( cRegexp.GetVersionT() );
			cmemMessage.AppendString( L")\r\n" );
		}
	}

	if( CODE_AUTODETECT == nGrepCharSet ){
		cmemMessage.AppendString( L"    (文字コードセットの自動判別)\r\n" );
	}else if(IsValidCodeType(nGrepCharSet)){
		cmemMessage.AppendString( L"    (文字コードセット：" );
		cmemMessage.AppendStringT( CCodeTypeName(nGrepCharSet).Normal() );
		cmemMessage.AppendString( L")\r\n" );
	}

	if( 0 < nWork ){ // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if( bGrepOutputLine ){
		/* 該当行 */
			pszWork = L"    (一致した行を出力)\r\n";
		}else{
			pszWork = L"    (一致した箇所のみ出力)\r\n";
		}
		cmemMessage.AppendString( pszWork );
	}


	cmemMessage.AppendString( L"\r\n\r\n" );
	pszWork = cmemMessage.GetStringPtr( &nWork );
//@@@ 2002.01.03 YAZAKI Grep直後はカーソルをGrep直前の位置に動かす
	CLayoutInt tmp_PosY_Layout = pcViewDst->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( 0 < nWork ){
		pcViewDst->GetCommander().Command_ADDTAIL( pszWork, nWork );
	}
	//	2007.07.22 genta バージョンを取得するために，
	//	正規表現の初期化を上へ移動


	/* 表示処理ON/OFF */
	// 2003.06.23 Moca 共通設定で変更できるように
//	SetDrawSwitch(false);
	pcViewDst->SetDrawSwitch(0 != GetDllShareData().m_Common.m_sSearch.m_bGrepRealTimeView);


	int nGrepTreeResult = DoGrepTree(
		pcViewDst,
		&cDlgCancel,
		hwndCancel,
		szKey,
		pnKey_CharCharsArr,
		szFile,
		szPath,
		bGrepSubFolder,
		sSearchOption,
		nGrepCharSet,
		bGrepOutputLine,
		nGrepOutputStyle,
		&cRegexp,
		0,
		&nHitCount
	);
	if( -1 == nGrepTreeResult ){
		const wchar_t* p = L"中断しました。\r\n";
		pcViewDst->GetCommander().Command_ADDTAIL( p, wcslen( p ) );
	}
	auto_sprintf( szPath, _T("%d 個が検索されました。\r\n"), nHitCount );
	pcViewDst->GetCommander().Command_ADDTAIL( to_wchar(szPath), -1 );
//	GetCommander().Command_GOFILEEND( FALSE );
#ifdef _DEBUG
	auto_sprintf( szPath, _T("処理時間: %dミリ秒\r\n"), cRunningTimer.Read() );
	pcViewDst->GetCommander().Command_ADDTAIL( to_wchar(szPath), -1 );
#endif

	pcViewDst->GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), tmp_PosY_Layout), TRUE );	//	カーソルをGrep直前の位置に戻す。

	cDlgCancel.CloseDialog( 0 );

	/* アクティブにする */
	ActivateFrameWindow( CEditWnd::Instance()->GetHwnd() );


	/* アンドゥバッファの処理 */
	if( NULL != pcViewDst->m_pcOpeBlk ){
		if( 0 < pcViewDst->m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
			/* 操作の追加 */
			pcViewDst->m_pcEditDoc->m_cDocEditor.m_cOpeBuf.AppendOpeBlk( pcViewDst->m_pcOpeBlk );
		}
		else{
			delete pcViewDst->m_pcOpeBlk;
		}
		pcViewDst->m_pcOpeBlk = NULL;
	}

	//	Apr. 13, 2001 genta
	//	Grep実行後はファイルを変更無しの状態にする．
	pcViewDst->m_pcEditDoc->m_cDocEditor.SetModified(false,false);

	this->m_bGrepRunning = false;
	pcViewDst->m_bDoing_UndoRedo = false;

	if( NULL != pnKey_CharCharsArr ){
		delete [] pnKey_CharCharsArr;
		pnKey_CharCharsArr = NULL;
	}
//	if( NULL != pnKey_CharUsedArr ){
//		delete [] pnKey_CharUsedArr;
//		pnKey_CharUsedArr = NULL;
//	}

	/* 表示処理ON/OFF */
	pcViewDst->SetDrawSwitch(true);

	/* フォーカス移動時の再描画 */
	pcViewDst->m_pcEditWnd->SetActivePane( pcViewDst->m_nMyIndex );
	pcViewDst->m_pcEditWnd->RedrawInactivePane();

	return nHitCount;
}



/*
 * SORTED_LIST_BSEARCH
 *   リストの探索にbsearchを使います。
 *   指定しない場合は、線形探索になります。
 * SORTED_LIST
 *   リストをqsortします。
 *
 * メモ：
 *   線形探索でもqsortを使い、文字列比較の大小関係が逆転したところで探索を
 *   打ち切れば少しは速いかもしれません。
 */
//#define SORTED_LIST
//#define SORTED_LIST_BSEARCH

#ifdef SORTED_LIST_BSEARCH
#define SORTED_LIST
#endif

#ifdef SORTED_LIST
typedef int (* COMP)(const void *, const void *);

/*!
	qsort用比較関数
	引数a,bは文字列へのポインタのポインタであることに注意。
	
	@param a [in] 比較文字列へのポインタのポインタ(list)
	@param b [in] 比較文字列へのポインタのポインタ(list)
	@return 比較結果
*/
int grep_compare_pp(const void* a, const void* b)
{
	return _tcscmp( *((const TCHAR**)a), *((const TCHAR**)b) );
}

/*!
	bsearch用比較関数
	引数bは文字列へのポインタのポインタであることに注意。
	
	@param a [in] 比較文字列へのポインタ(key)
	@param b [in] 比較文字列へのポインタのポインタ(list)
	@return 比較結果
*/
int grep_compare_sp(const void* a, const void* b)
{
	return _tcscmp( (const TCHAR*)a, *((const TCHAR**)b) );
}
#endif

/*! @brief Grep実行

	@date 2001.06.27 genta	正規表現ライブラリの差し替え
	@date 2003.06.23 Moca   サブフォルダ→ファイルだったのをファイル→サブフォルダの順に変更
	@date 2003.06.23 Moca   ファイル名から""を取り除くように
	@date 2003.03.27 みく   除外ファイル指定の導入と重複検索防止の追加．
		大部分が変更されたため，個別の変更点記入は無し．
*/
int CGrepAgent::DoGrepTree(
	CEditView*				pcViewDst,
	CDlgCancel*				pcDlgCancel,		//!< [in] Cancelダイアログへのポインタ
	HWND					hwndCancel,			//!< [in] Cancelダイアログのウィンドウハンドル
	const wchar_t*			pszKey,				//!< [in] 検索パターン
	int*					pnKey_CharCharsArr,	//!< [in] 文字種配列(2byte/1byte)．単純文字列検索で使用．
	const TCHAR*			pszFile,			//!< [in] 検索対象ファイルパターン(!で除外指定)
	const TCHAR*			pszPath,			//!< [in] 検索対象パス
	BOOL					bGrepSubFolder,		//!< [in] TRUE: サブフォルダを再帰的に探索する / FALSE: しない
	const SSearchOption&	sSearchOption,		//!< [in] 検索オプション
	ECodeType				nGrepCharSet,		//!< [in] 文字コードセット (0:自動認識)～
	BOOL					bGrepOutputLine,	//!< [in] TRUE: ヒット行を出力 / FALSE: ヒット部分を出力
	int						nGrepOutputStyle,	//!< [in] 出力形式 1: Normal, 2: WZ風(ファイル単位)
	CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	int						nNest,				//!< [in] ネストレベル
	int*					pnHitCount			//!< [i/o] ヒット数の合計
)
{
	::DlgItem_SetText( hwndCancel, IDC_STATIC_CURPATH, pszPath );

	const TCHAR EXCEPT_CHAR = _T('!');	//除外識別子
	const TCHAR* WILDCARD_DELIMITER = _T(" ;,");	//リストの区切り
	const TCHAR* WILDCARD_ANY = _T("*.*");	//サブフォルダ探索用

	int		nWildCardLen;
	int		nPos;
	BOOL	result;
	int		i;
	WIN32_FIND_DATA w32fd;
	CNativeW		cmemMessage;
	int				nHitCountOld;
	const wchar_t*	pszWork;
	int				nWork = 0;
	nHitCountOld = -100;

	//解放の対象
	HANDLE handle      = INVALID_HANDLE_VALUE;


	/*
	 * リストの初期化(文字列へのポインタをリスト管理する)
	 */
	int checked_list_size = 256;	//確保済みサイズ
	int checked_list_count = 0;	//登録個数
	TCHAR** checked_list = (TCHAR**)malloc( sizeof( TCHAR* ) * checked_list_size );
	if( ! checked_list ) return FALSE;	//メモリ確保失敗


	/*
	 * 除外ファイルを登録する。
	 */
	nPos = 0;
	TCHAR* pWildCard = _tcsdup( pszFile );	//ワイルドカードリスト作業用
	if( ! pWildCard ) goto error_return;	//メモリ確保失敗
	nWildCardLen = _tcslen( pWildCard );
	TCHAR*	token;
	while( NULL != (token = my_strtok<TCHAR>( pWildCard, nWildCardLen, &nPos, WILDCARD_DELIMITER )) )	//トークン毎に繰り返す。
	{
		//除外ファイル指定でないか？
		if( EXCEPT_CHAR != token[0] ) continue;

		//ダブルコーテーションを除き、絶対パス名を作成する。
		TCHAR* p;
		TCHAR* q;
		p = q = ++token;
		while( *p )
		{
			if( *p != _T('"') ) *q++ = *p;
			p++;
		}
		*q = _T('\0');
		TCHAR* currentPath = NULL;	//現在探索中のパス
		currentPath = new TCHAR[ _tcslen( pszPath ) + _tcslen( token ) + 1 ];
		if( ! currentPath ) goto error_return;	//メモリ確保失敗
		_tcscpy( currentPath, pszPath );
		_tcscat( currentPath, token );

		//ファイルの羅列を開始する。
		handle = FindFirstFile( currentPath, &w32fd );
		result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;
		while( result )
		{
			if( ! (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )	//フォルダでない場合
			{
				//チェック済みリストに登録する。
				if( checked_list_count >= checked_list_size )
				{
					checked_list_size += 256;
					TCHAR** p = (TCHAR**)realloc( checked_list, sizeof( TCHAR* ) * checked_list_size );
					if( ! p ) goto error_return;	//メモリ確保失敗
					checked_list = p;
				}
				checked_list[ checked_list_count ] = _tcsdup( w32fd.cFileName );
				checked_list_count++;
			}

			//次のファイルを羅列する。
			result = FindNextFile( handle, &w32fd );
		}
		//ハンドルを閉じる。
		if( INVALID_HANDLE_VALUE != handle )
		{
			FindClose( handle );
			handle = INVALID_HANDLE_VALUE;
		}
		delete [] currentPath;
		currentPath = NULL;
	}
	free( pWildCard );
	pWildCard = NULL;


	/*
	 * カレントフォルダのファイルを探索する。
	 */
	nPos = 0;
	pWildCard = _tcsdup( pszFile );
	if( ! pWildCard ) goto error_return;	//メモリ確保失敗
	nWildCardLen = _tcslen( pWildCard );
	while( NULL != (token = my_strtok<TCHAR>( pWildCard, nWildCardLen, &nPos, WILDCARD_DELIMITER )) )	//トークン毎に繰り返す。
	{
		//除外ファイル指定か？
		if( EXCEPT_CHAR == token[0] ) continue;

		//ダブルコーテーションを除き、絶対パス名を作成する。
		TCHAR* p;
		TCHAR* q;
		p = q = token;
		while( *p )
		{
			if( *p != _T('"') ) *q++ = *p;
			p++;
		}
		*q = _T('\0');
		TCHAR* currentPath = NULL;	//現在探索中のパス
		currentPath = new TCHAR[ _tcslen( pszPath ) + _tcslen( token ) + 1 ];
		if( ! currentPath ) goto error_return;
		_tcscpy( currentPath, pszPath );
		_tcscat( currentPath, token );

		//ファイルの羅列を開始する。
#ifdef SORTED_LIST
		//ソート
		qsort( checked_list, checked_list_count, sizeof( WCHAR* ), (COMP)grep_compare_pp_w );
#endif
		int current_checked_list_count = checked_list_count;	//前回までのリストの数
		handle = FindFirstFile( currentPath, &w32fd );
		result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;
		while( result )
		{
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				goto cancel_return;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}

			/* 表示設定をチェック */
			pcViewDst->SetDrawSwitch(0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW ));

			if( ! (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )	//フォルダでない場合
			{
				/*
				 * リストにあるか調べる。
				 * 今回探索中のファイル同士が重複することはないので、
				 * 前回までのリスト(current_checked_list_count)から検索すればよい。
				 */
#ifdef SORTED_LIST_BSEARCH
				if( ! bsearch( w32fd.cFileName, checked_list, current_checked_list_count, sizeof( WCHAR* ), (COMP)grep_compare_sp ) )
#else
				bool found = false;
				TCHAR** ptr = checked_list;
				for( i = 0; i < current_checked_list_count; i++, ptr++ )
				{
#ifdef SORTED_LIST
					int n = wcscmp( *ptr, w32fd.cFileName );
					if( 0 == n )
					{
						found = true; 
						break;
					}
					else if( n > 0 )	//探索打ち切り
					{
						break;
					}
#else
					if( 0 == _tcscmp( *ptr, w32fd.cFileName ) )
					{
						found = true; 
						break;
					}
#endif
				}
				if( ! found )
#endif
				{
					//チェック済みリストに登録する。
					if( checked_list_count >= checked_list_size )
					{
						checked_list_size += 256;
						TCHAR** p = (TCHAR**)realloc( checked_list, sizeof( TCHAR* ) * checked_list_size );
						if( ! p ) goto error_return;	//メモリ確保失敗
						checked_list = p;
					}
					checked_list[ checked_list_count ] = _tcsdup( w32fd.cFileName );
					checked_list_count++;


					//GREP実行！
					::DlgItem_SetText( hwndCancel, IDC_STATIC_CURFILE, w32fd.cFileName );

					TCHAR* currentFile = new TCHAR[ _tcslen( pszPath ) + _tcslen( w32fd.cFileName ) + 1 ];
					if( ! currentFile ) goto error_return;	//メモリ確保失敗
					_tcscpy( currentFile, pszPath );
					_tcscat( currentFile, w32fd.cFileName );
					/* ファイル内の検索 */
					int nRet = DoGrepFile(
						pcViewDst,
						pcDlgCancel,
						hwndCancel,
						pszKey,
						pnKey_CharCharsArr,
						w32fd.cFileName,
						sSearchOption,
						nGrepCharSet,
						bGrepOutputLine,
						nGrepOutputStyle,
						pRegexp,
						pnHitCount,
						currentFile,
						cmemMessage
					);
					delete currentFile;
					currentFile = NULL;

					// 2003.06.23 Moca リアルタイム表示のときは早めに表示
					if( pcViewDst->GetDrawSwitch() ){
						if( LTEXT('\0') != pszKey[0] ){
							// データ検索のときファイルの合計が最大10MBを超えたら表示
							nWork += ( w32fd.nFileSizeLow + 1023 ) / 1024;
						}
						if( *pnHitCount - nHitCountOld && 
							( *pnHitCount < 20 || 10000 < nWork ) ){
							nHitCountOld = -100; // 即表示
						}
					}
					if( *pnHitCount - nHitCountOld  >= 10 ){
						/* 結果出力 */
						pszWork = cmemMessage.GetStringPtr( &nWork );
						if( 0 < nWork ){
							pcViewDst->GetCommander().Command_ADDTAIL( pszWork, nWork );
							pcViewDst->GetCommander().Command_GOFILEEND( FALSE );
							/* 結果格納エリアをクリア */
							cmemMessage.SetString( L"" );
							nWork = 0;
						}
						nHitCountOld = *pnHitCount;
					}
					if( -1 == nRet ){
						goto cancel_return;
					}
				}
			}

			//次のファイルを羅列する。
			result = FindNextFile( handle, &w32fd );
		}
		//ハンドルを閉じる。
		if( INVALID_HANDLE_VALUE != handle )
		{
			FindClose( handle );
			handle = INVALID_HANDLE_VALUE;
		}
		delete [] currentPath;
		currentPath = NULL;
	}
	free( pWildCard );
	pWildCard = NULL;

	for( i = 0; i < checked_list_count; i++ )
	{
		free( checked_list[ i ] );
	}
	free( checked_list );
	checked_list = NULL;
	checked_list_count = 0;
	checked_list_size = 0;


	/*
	 * サブフォルダを検索する。
	 */
	if( bGrepSubFolder ){
		TCHAR* subPath     = NULL;
		subPath = new TCHAR[ _tcslen( pszPath ) + _tcslen( WILDCARD_ANY ) + 1 ];
		if( ! subPath ) goto error_return;	//メモリ確保失敗
		_tcscpy( subPath, pszPath );
		_tcscat( subPath, WILDCARD_ANY );
		handle = FindFirstFile( subPath, &w32fd );
		result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;
		while( result )
		{
			//サブフォルダの探索を再帰呼び出し。
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				goto cancel_return;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}
			/* 表示設定をチェック */
			pcViewDst->SetDrawSwitch(0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW ));

			if( (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	//フォルダの場合
			 && 0 != _tcscmp( w32fd.cFileName, _T("."))
			 && 0 != _tcscmp( w32fd.cFileName, _T("..")) )
			{
				//フォルダ名を作成する。
				TCHAR* currentPath = NULL;	//現在探索中のパス
				currentPath = new TCHAR[ _tcslen( pszPath ) + _tcslen( w32fd.cFileName ) + 2 ];
				if( ! currentPath ) goto error_return;	//メモリ確保失敗
				_tcscpy( currentPath, pszPath );
				_tcscat( currentPath, w32fd.cFileName );
				_tcscat( currentPath, _T("\\") );

				int nGrepTreeResult = DoGrepTree(
					pcViewDst,
					pcDlgCancel,
					hwndCancel,
					pszKey,
					pnKey_CharCharsArr,
					pszFile,
					currentPath,
					bGrepSubFolder,
					sSearchOption,
					nGrepCharSet,
					bGrepOutputLine,
					nGrepOutputStyle,
					pRegexp,
					nNest + 1,
					pnHitCount
				);
				if( -1 == nGrepTreeResult ){
					goto cancel_return;
				}
				::DlgItem_SetText( hwndCancel, IDC_STATIC_CURPATH, pszPath );	//@@@ 2002.01.10 add サブフォルダから戻ってきたら...

				delete [] currentPath;
				currentPath = NULL;
			}

			//次のファイルを羅列する。
			result = FindNextFile( handle, &w32fd );
		}
		//ハンドルを閉じる。
		if( INVALID_HANDLE_VALUE != handle )
		{
			FindClose( handle );
			handle = INVALID_HANDLE_VALUE;
		}
		delete [] subPath;
		subPath = NULL;
	}

	::DlgItem_SetText( hwndCancel, IDC_STATIC_CURFILE, LTEXT(" ") );	// 2002/09/09 Moca add
	/* 結果出力 */
	pszWork = cmemMessage.GetStringPtr( &nWork );
	if( 0 < nWork ){
		pcViewDst->GetCommander().Command_ADDTAIL( pszWork, nWork );
		pcViewDst->GetCommander().Command_GOFILEEND( FALSE );
		/* 結果格納エリアをクリア */
		cmemMessage.SetString( LTEXT("") );
	}

	return 0;


cancel_return:;
error_return:;
	/*
	 * エラー時はすべての確保済みリソースを解放する。
	 */
	if( INVALID_HANDLE_VALUE != handle ) FindClose( handle );

	if( pWildCard ) free( pWildCard );
//	if( currentPath ) delete [] currentPath;
//	if( subPath ) delete [] subPath;

	if( checked_list )
	{
		for( i = 0; i < checked_list_count; i++ )
		{
			free( checked_list[ i ] );
		}
		free( checked_list );
	}

	/* 結果出力 */
	pszWork = cmemMessage.GetStringPtr( &nWork );
	if( 0 < nWork )
	{
		pcViewDst->GetCommander().Command_ADDTAIL( pszWork, nWork );
		pcViewDst->GetCommander().Command_GOFILEEND( FALSE );
		/* 結果格納エリアをクリア */
		cmemMessage.SetString( LTEXT("") );
	}

	return -1;
}




/*!	@brief Grep結果を構築する

	@param pWork [out] Grep出力文字列．充分なメモリ領域を予め確保しておくこと．
		最長で 本文2000 byte＋ファイル名 _MAX_PATH byte＋行・桁位置表示の長さが必要．
		ファイル単位出力の場合は本文2500 byte + _MAX_PATH + 行・桁位置表示の長さが必要．
		

	pWorkは充分なメモリ領域を持っているコト
	@date 2002/08/29 Moca バイナリーデータに対応 pnWorkLen 追加
*/
void CGrepAgent::SetGrepResult(
	/* データ格納先 */
	wchar_t*	pWork,
	int*		pnWorkLen,			/*!< [out] Grep出力文字列の長さ */
	/* マッチしたファイルの情報 */
	const TCHAR*		pszFullPath,	/*!< [in] フルパス */
	const TCHAR*		pszCodeName,	/*!< [in] 文字コード情報．" [SJIS]"とか */
	/* マッチした行の情報 */
	int			nLine,				/*!< [in] マッチした行番号(1～) */
	int			nColm,				/*!< [in] マッチした桁番号(1～) */
	const wchar_t*	pCompareData,	/*!< [in] 行の文字列 */
	int			nLineLen,			/*!< [in] 行の文字列の長さ */
	int			nEolCodeLen,		/*!< [in] EOLの長さ */
	/* マッチした文字列の情報 */
	const wchar_t*	pMatchData,		/*!< [in] マッチした文字列 */
	int			nMatchLen,			/*!< [in] マッチした文字列の長さ */
	/* オプション */
	BOOL		bGrepOutputLine,	/*!< [in] 0: 該当部分のみ, !0: 該当行 */
	int			nGrepOutputStyle	/*!< [in] 1: Normal, 2: WZ風(ファイル単位) */
)
{

	int nWorkLen = 0;
	const wchar_t * pDispData;
	int k;
	bool bEOL = true;
	int nMaxOutStr;

	/* ノーマル */
	if( 1 == nGrepOutputStyle ){
		nWorkLen = ::auto_sprintf( pWork, L"%ls(%d,%d)%ls: ", pszFullPath, nLine, nColm, pszCodeName );
		nMaxOutStr = 2000; // 2003.06.10 Moca 最大長変更
	}
	/* WZ風 */
	else if( 2 == nGrepOutputStyle ){
		nWorkLen = ::auto_sprintf( pWork, L"・(%6d,%-5d): ", nLine, nColm );
		nMaxOutStr = 2500; // 2003.06.10 Moca 最大長変更
	}

	/* 該当行 */
	if( bGrepOutputLine ){
		pDispData = pCompareData;
		k = nLineLen - nEolCodeLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
	}
	/* 該当部分 */
	else{
		pDispData = pMatchData;
		k = nMatchLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
		// 該当部分に改行を含む場合はその改行コードをそのまま利用する(次の行に空行を作らない)
		// 2003.06.10 Moca k==0のときにバッファアンダーランしないように
		if( 0 < k && (pMatchData[ k - 1 ] == L'\r' || pMatchData[ k - 1 ] == L'\n') ){
			bEOL = false;
		}
	}

	auto_memcpy( &pWork[nWorkLen], pDispData, k );
	nWorkLen += k;
	if( bEOL ){
		auto_memcpy( &pWork[nWorkLen], L"\r\n", 2 );
		nWorkLen = nWorkLen + 2;
	}
	*pnWorkLen = nWorkLen;
}

/*!
	Grep実行 (CFileLoadを使ったテスト版)

	@param pcDlgCancel		[in] Cancelダイアログへのポインタ
	@param hwndCancel		[in] Cancelダイアログのウィンドウハンドル
	@param pszKey			[in] 検索パターン
	@param pnKey_CharCharsArr	[in] 文字種配列(2byte/1byte)．単純文字列検索で使用．
	@param pszFile			[in] 処理対象ファイル名(表示用)
	@param bGrepLoHiCase	[in] TRUE: 大文字小文字の区別あり / FALSE: 無し
	@param bGrepRegularExp	[in] TRUE: 検索パターンは正規表現 / FALSE: 文字列
	@param nGrepCharSet		[in] 文字コードセット (0:自動認識)～
	@param bGrepOutputLine	[in] TRUE: ヒット行を出力 / FALSE: ヒット部分を出力
	@param bWordOnly		[in] TRUE: 単語単位で一致を判断 / FALSE: 部分にも一致する
	@param nGrepOutputStyle	[in] 出力形式 1: Normal, 2: WZ風(ファイル単位)
	@param pRegexp			[in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	@param pnHitCount		[i/o] ヒット数の合計．元々の値に見つかった数を加算して返す．
	@param pszFullPath		[in] 処理対象ファイルパス

	@retval -1 GREPのキャンセル
	@retval それ以外 ヒット数(ファイル検索時はファイル数)

	@date 2002/08/30 Moca CFileLoadを使ったテスト版
	@date 2004/03/28 genta 不要な引数nNest, bGrepSubFolder, pszPathを削除
*/
int CGrepAgent::DoGrepFile(
	CEditView*				pcViewDst,
	CDlgCancel*				pcDlgCancel,
	HWND					hwndCancel,
	const wchar_t*			pszKey,
	int*					pnKey_CharCharsArr,
	const TCHAR*			pszFile,
	const SSearchOption&	sSearchOption,
	ECodeType				nGrepCharSet,
	BOOL					bGrepOutputLine,
	int						nGrepOutputStyle,
	CBregexp*				pRegexp,		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	int*					pnHitCount,
	const TCHAR*			pszFullPath,
	CNativeW&				cmemMessage
)
{
	int		nHitCount;
//	char	szLine[16000];
	wchar_t	szWork[3000]; // ここは SetGrepResult() が返す文字列を格納できるサイズが必要
	wchar_t	szWork0[_MAX_PATH + 100];
	int		nLine;
	int		nWorkLen;
	const wchar_t*	pszRes; // 2002/08/29 const付加
	ECodeType	nCharCode;
	const wchar_t*	pCompareData; // 2002/08/29 const付加
	int		nColm;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
	CEol	cEol;
	int		nEolCodeLen;
	CFileLoad	cfl;
	int		nOldPercent = 0;

	int	nKeyKen = wcslen( pszKey );

	//	ここでは正規表現コンパイルデータの初期化は不要

	const TCHAR*	pszCodeName; // 2002/08/29 const付加
	pszCodeName = _T("");
	nHitCount = 0;
	nLine = 0;

	/* 検索条件が長さゼロの場合はファイル名だけ返す */
	// 2002/08/29 行ループの前からここに移動
	if( 0 == nKeyKen ){
		if( CODE_AUTODETECT == nGrepCharSet ){
			// 2003.06.10 Moca コード判別処理をここに移動．
			// 判別エラーでもファイル数にカウントするため
			// ファイルの日本語コードセット判別
			nCharCode = CCodeMediator::CheckKanjiCodeOfFile( pszFullPath );
			if( -1 == nCharCode ){
				pszCodeName = _T("  [(DetectError)]");
			}else{
				pszCodeName = CCodeTypeName(nCharCode).Bracket();
			}
		}
		if( 1 == nGrepOutputStyle ){
		/* ノーマル */
			auto_sprintf( szWork0, L"%ts%ts\r\n", pszFullPath, pszCodeName );
		}else{
		/* WZ風 */
			auto_sprintf( szWork0, L"■\"%ts\"%ts\r\n", pszFullPath, pszCodeName );
		}
		cmemMessage.AppendString( szWork0 );
		++(*pnHitCount);
		::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
		return 1;
	}


	try{
	// ファイルを開く
	// FileCloseで明示的に閉じるが、閉じていないときはデストラクタで閉じる
	// 2003.06.10 Moca 文字コード判定処理もFileOpenで行う
	nCharCode = cfl.FileOpen( pszFullPath, nGrepCharSet, 0 );
	if( CODE_AUTODETECT == nGrepCharSet ){
		pszCodeName = CCodeTypeName(nCharCode).Bracket();
	}
	auto_sprintf( szWork0, L"■\"%ts\"%ts\r\n", pszFullPath, pszCodeName );
//	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
		return -1;
	}
	/* 中断ボタン押下チェック */
	if( pcDlgCancel->IsCanceled() ){
		return -1;
	}

	/* 検索条件が長さゼロの場合はファイル名だけ返す */
	// 2002/08/29 ファイルオープンの手前へ移動

	// 注意 : cfl.ReadLine が throw する可能性がある
	CNativeW cUnicodeBuffer;
	while( RESULT_FAILURE != cfl.ReadLine( &cUnicodeBuffer, &cEol ) )
	{
		const wchar_t*	pLine = cUnicodeBuffer.GetStringPtr();
		int		nLineLen = cUnicodeBuffer.GetStringLength();

		nEolCodeLen = cEol.GetLen();
		++nLine;
		pCompareData = pLine;

		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
			return -1;
		}
		if( 0 == nLine % 64 ){
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				return -1;
			}
			//	2003.06.23 Moca 表示設定をチェック
			pcViewDst->SetDrawSwitch(0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW ));
			// 2002/08/30 Moca 進行状態を表示する(5MB以上)
			if( 5000000 < cfl.GetFileSize() ){
				int nPercent = cfl.GetPercent();
				if( 5 <= nPercent - nOldPercent ){
					nOldPercent = nPercent;
					::auto_sprintf( szWork, L"%ls (%3d%%)", pszFile, nPercent );
					::DlgItem_SetText( hwndCancel, IDC_STATIC_CURFILE, szWork );
				}
			}
		}

		/* 正規表現検索 */
		if( sSearchOption.bRegularExp ){
			int nColmPrev = 0;

			//	Jun. 21, 2003 genta ループ条件見直し
			//	マッチ箇所を1行から複数検出するケースを標準に，
			//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
			//	ループ継続・打ち切り条件(bGrepOutputLine)を逆にした．
			//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
			// From Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
			while( pRegexp->Match( pCompareData, nLineLen, 0 ) ){

					//	パターン発見
					nColm = pRegexp->GetIndex() + 1;
					int matchlen = pRegexp->GetMatchLen();

					/* Grep結果を、szWorkに格納する */
					SetGrepResult(
						szWork,
						&nWorkLen,
						pszFullPath,
						pszCodeName,
						nLine,
						nColm + nColmPrev,
						pCompareData,
						nLineLen,
						nEolCodeLen,
						pCompareData + nColm - 1,
						matchlen,
						bGrepOutputLine,
						nGrepOutputStyle
					);
					// To Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
					if( 2 == nGrepOutputStyle ){
					/* WZ風 */
						if( !bOutFileName ){
							cmemMessage.AppendString( szWork0 );
							bOutFileName = TRUE;
						}
					}
					cmemMessage.AppendString( szWork, nWorkLen );
					++nHitCount;
					++(*pnHitCount);
					if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
						::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					//	Jun. 21, 2003 genta 行単位で出力する場合は1つ見つかれば十分
					if ( bGrepOutputLine ) {
						break;
					}
					//	探し始める位置を補正
					//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
					//	nClom : マッチ位置
					//	matchlen : マッチした文字列の長さ
					int nPosDiff = nColm + matchlen;
					if( matchlen != 0 ){
						nPosDiff--;
					}
					pCompareData += nPosDiff;
					nLineLen -= nPosDiff;
					nColmPrev += nPosDiff;
			}
		}
		/* 単語のみ検索 */
		else if( sSearchOption.bWordOnly ){
			/*
				2002/02/23 Norio Nakatani
				単語単位のGrepを試験的に実装。単語はWhereCurrentWord()で判別してますので、
				英単語やC/C++識別子などの検索条件ならヒットします。

				2002/03/06 YAZAKI
				Grepにも試験導入。
				WhereCurrentWordで単語を抽出して、その単語が検索語とあっているか比較する。
			*/
			CLogicInt nNextWordFrom = CLogicInt(0);
			CLogicInt nNextWordFrom2;
			CLogicInt nNextWordTo2;
			// Jun. 26, 2003 genta 無駄なwhileは削除
			while( CWordParse::WhereCurrentWord_2( pCompareData, CLogicInt(nLineLen), nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL ) ){
				if( nKeyKen == nNextWordTo2 - nNextWordFrom2 ){
					// const char* pData = pCompareData;	// 2002/2/10 aroka CMemory変更 , 2002/08/29 Moca pCompareDataのconst化により不要?
					/* 1==大文字小文字の区別 */
					if( (!sSearchOption.bLoHiCase && 0 == auto_memicmp( &(pCompareData[nNextWordFrom2]) , pszKey, nKeyKen ) ) ||
						(sSearchOption.bLoHiCase && 0 ==	 auto_memcmp( &(pCompareData[nNextWordFrom2]) , pszKey, nKeyKen ) )
					){
						/* Grep結果を、szWorkに格納する */
						SetGrepResult(
							szWork, &nWorkLen,
							pszFullPath, pszCodeName,
							//	Jun. 25, 2002 genta
							//	桁位置は1始まりなので1を足す必要がある
							nLine, nNextWordFrom2 + 1, pCompareData, nLineLen, nEolCodeLen,
							&(pCompareData[nNextWordFrom2]), nKeyKen,
							bGrepOutputLine, nGrepOutputStyle
						);
						if( 2 == nGrepOutputStyle ){
						/* WZ風 */
							if( !bOutFileName ){
								cmemMessage.AppendString( szWork0 );
								bOutFileName = TRUE;
							}
						}

						cmemMessage.AppendString( szWork, nWorkLen );
						++nHitCount;
						++(*pnHitCount);
						//	May 22, 2000 genta
						// if( 0 == ( (*pnHitCount) % 16 ) ){
							::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
						// }
					}
				}
				/* 現在位置の左右の単語の先頭位置を調べる */
				if( !CWordParse::SearchNextWordPosition( pCompareData, CLogicInt(nLineLen), nNextWordFrom, &nNextWordFrom, FALSE ) ){
					break;	//	次の単語が無い。
				}
			}
		}
		else {
			/* 文字列検索 */
			int nColmPrev = 0;
			//	Jun. 21, 2003 genta ループ条件見直し
			//	マッチ箇所を1行から複数検出するケースを標準に，
			//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
			//	ループ継続・打ち切り条件(bGrepOutputLine)を逆にした．
			while(1){
				pszRes = CSearchAgent::SearchString(
					pCompareData,
					nLineLen,
					0,
					pszKey,
					nKeyKen,
					pnKey_CharCharsArr,
					sSearchOption.bLoHiCase
				);
				if(!pszRes)break;

				nColm = pszRes - pCompareData + 1;

				/* Grep結果を、szWorkに格納する */
				SetGrepResult(
					szWork, &nWorkLen,
					pszFullPath, pszCodeName,
					nLine, nColm + nColmPrev, pCompareData, nLineLen, nEolCodeLen,
					pszRes, nKeyKen,
					bGrepOutputLine, nGrepOutputStyle
				);
				if( 2 == nGrepOutputStyle ){
				/* WZ風 */
					if( !bOutFileName ){
						cmemMessage.AppendString( szWork0 );
						bOutFileName = TRUE;
					}
				}

				cmemMessage.AppendString( szWork, nWorkLen );
				++nHitCount;
				++(*pnHitCount);
				//	May 22, 2000 genta
				if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
					::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				}
				
				//	Jun. 21, 2003 genta 行単位で出力する場合は1つ見つかれば十分
				if ( bGrepOutputLine ) {
					break;
				}
				//	探し始める位置を補正
				//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
				//	nClom : マッチ位置
				//	matchlen : マッチした文字列の長さ
				int nPosDiff = nColm += nKeyKen - 1;
				pCompareData += nPosDiff;
				nLineLen -= nPosDiff;
				nColmPrev += nPosDiff;
			}
		}
	}

	// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
	cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		auto_sprintf( szWork, L"file open error [%ts]\r\n", pszFullPath );
		pcViewDst->GetCommander().Command_ADDTAIL( szWork, wcslen( szWork ) );
		return 0;
	}
	catch( CError_FileRead ){
		auto_sprintf( szWork, L"CEditView::DoGrepFile() ファイルの読み込み中にエラーが発生しました。\r\n");
		pcViewDst->GetCommander().Command_ADDTAIL( szWork, wcslen( szWork ) );
	} // 例外処理終わり

	return nHitCount;
}


