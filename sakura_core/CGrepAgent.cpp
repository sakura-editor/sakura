#include "StdAfx.h"
#include "CGrepAgent.h"
#include "CGrepEnumKeys.h"
#include "CGrepEnumFilterFiles.h"
#include "CGrepEnumFilterFolders.h"
#include "CSearchAgent.h"
#include "dlg/CDlgCancel.h"
#include "_main/CAppMode.h"
#include "COpeBlk.h"
#include "window/CEditWnd.h"
#include "charset/CCodeMediator.h"
#include "view/colors/CColorStrategy.h"
#include "charset/CCodeFactory.h"
#include "charset/CCodeBase.h"
#include "charset/CCodePage.h"
#include "io/CFileLoad.h"
#include "io/CBinaryStream.h"
#include "util/window.h"
#include "util/module.h"
#include "util/other_util.h"
#include "debug/CRunningTimer.h"
#include <deque>
#include "sakura_rc.h"

#define UICHECK_INTERVAL_MILLISEC 50	// UI確認の時間間隔
#define ADDTAIL_INTERVAL_MILLISEC 50	// 結果出力の時間間隔

CGrepAgent::CGrepAgent()
: m_bGrepMode( false )			/* Grepモードか */
, m_bGrepRunning( false )		/* Grep処理中 */
, m_dwTickAddTail( 0 )
, m_dwTickUICheck( 0 )
{
}

ECallbackResult CGrepAgent::OnBeforeClose()
{
	//GREP処理中は終了できない
	if( m_bGrepRunning ){
		// アクティブにする
		ActivateFrameWindow( CEditWnd::getInstance()->GetHwnd() );	//@@@ 2003.06.25 MIK
		TopInfoMessage(
			CEditWnd::getInstance()->GetHwnd(),
			LS(STR_GREP_RUNNINNG)
		);
		return CALLBACK_INTERRUPT;
	}
	return CALLBACK_CONTINUE;
}

void CGrepAgent::OnAfterSave(const SSaveInfo& sSaveInfo)
{
	// 名前を付けて保存から再ロードが除去された分の不足処理を追加（ANSI版との差異）	// 2009.08.12 ryoji
	m_bGrepMode = false;	// grepウィンドウは通常ウィンドウ化
	CAppMode::getInstance()->m_szGrepKey[0] = L'\0';
}

/*!
	@date 2014.03.09 novice 最後の\\を取り除くのをやめる(d:\\ -> d:になる)
*/
void CGrepAgent::CreateFolders( const TCHAR* pszPath, std::vector<std::tstring>& vPaths )
{
	const int nPathLen = auto_strlen( pszPath );
	auto_array_ptr<TCHAR> szPath(new TCHAR[nPathLen + 1]);
	auto_array_ptr<TCHAR> szTmp(new TCHAR[nPathLen + 1]);
	auto_strcpy( &szPath[0], pszPath );
	TCHAR* token;
	int nPathPos = 0;
	while( NULL != (token = my_strtok<TCHAR>( &szPath[0], nPathLen, &nPathPos, _T(";"))) ){
		auto_strcpy( &szTmp[0], token );
		TCHAR* p;
		TCHAR* q;
		p = q = &szTmp[0];
		while( *p ){
			if( *p != _T('"') ){
				if( p != q ){
					*q = *p;
				}
				q++;
			}
			p++;
		}
		*q = _T('\0');
#if 0
		// 2011.12.25 仕様変更。最後の\\は取り除く
		int	nFolderLen = q - &szTmp[0];
		if( 0 < nFolderLen ){
			int nCharChars = &szTmp[nFolderLen] - CNativeT::GetCharPrev( &szTmp[0], nFolderLen, &szTmp[nFolderLen] );
			if( 1 == nCharChars && (_T('\\') == szTmp[nFolderLen - 1] || _T('/') == szTmp[nFolderLen - 1]) ){
				szTmp[nFolderLen - 1] = _T('\0');
			}
		}
#endif
		/* ロングファイル名を取得する */
		TCHAR szTmp2[_MAX_PATH];
		if( ::GetLongFileName( &szTmp[0], szTmp2 ) ){
			vPaths.push_back( szTmp2 );
		}else{
			vPaths.push_back( &szTmp[0] );
		}
	}
}

/*! 最後の\\を取り除く
	@date 2014.03.09 novice 新規作成
*/
std::tstring CGrepAgent::ChopYen( const std::tstring& str )
{
	std::tstring dst = str;
	size_t nPathLen = dst.length();

	// 最後のフォルダ区切り記号を削除する
	// [A:\]などのルートであっても削除
	for(size_t i = 0; i < nPathLen; i++ ){
#ifdef _MBCS
		if( _IS_SJIS_1( (unsigned char)dst[i] ) && (i + 1 < nPathLen) && _IS_SJIS_2( (unsigned char)dst[i + 1] ) ){
			// SJIS読み飛ばし
			i++;
		} else
#endif
		if( _T('\\') == dst[i] && i == nPathLen - 1 ){
			dst.resize( nPathLen - 1 );
			break;
		}
	}

	return dst;
}

void CGrepAgent::AddTail( CEditView* pcEditView, const CNativeW& cmem, bool bAddStdout )
{
	m_dwTickAddTail = ::GetTickCount();
	if( bAddStdout ){
		HANDLE out = ::GetStdHandle(STD_OUTPUT_HANDLE);
		if( out && out != INVALID_HANDLE_VALUE ){
			CMemory cmemOut;
			std::auto_ptr<CCodeBase> pcCodeBase( CCodeFactory::CreateCodeBase(
					pcEditView->GetDocument()->GetDocumentEncoding(), 0) );
			pcCodeBase->UnicodeToCode( cmem, &cmemOut );
			DWORD dwWrite = 0;
			::WriteFile(out, cmemOut.GetRawPtr(), cmemOut.GetRawLength(), &dwWrite, NULL);
		}
	}else{
		pcEditView->GetCommander().Command_ADDTAIL( cmem.GetStringPtr(), cmem.GetStringLength() );
		pcEditView->GetCommander().Command_GOFILEEND( FALSE );
		if( !CEditWnd::getInstance()->UpdateTextWrap() )	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			CEditWnd::getInstance()->RedrawAllViews( pcEditView );	//	他のペインの表示を更新
	}
}

/*! Grep実行

  @param[in] pcmGrepKey 検索パターン
  @param[in] pcmGrepFile 検索対象ファイルパターン(!で除外指定))
  @param[in] pcmGrepFolder 検索対象フォルダ

  @date 2008.12.07 nasukoji	ファイル名パターンのバッファオーバラン対策
  @date 2008.12.13 genta 検索パターンのバッファオーバラン対策
  @date 2012.10.13 novice 検索オプションをクラスごと代入
*/
DWORD CGrepAgent::DoGrep(
	CEditView*				pcViewDst,
	bool					bGrepReplace,
	const CNativeW*			pcmGrepKey,
	const CNativeW*			pcmGrepReplace,
	const CNativeT*			pcmGrepFile,
	const CNativeT*			pcmGrepFolder,
	bool					bGrepCurFolder,
	BOOL					bGrepSubFolder,
	bool					bGrepStdout,
	bool					bGrepHeader,
	const SSearchOption&	sSearchOption,
	ECodeType				nGrepCharSet,	// 2002/09/21 Moca 文字コードセット選択
	int						nGrepOutputLineType,
	int						nGrepOutputStyle,
	bool					bGrepOutputFileOnly,
	bool					bGrepOutputBaseFolder,
	bool					bGrepSeparateFolder,
	bool					bGrepPaste,
	bool					bGrepBackup
)
{
	MY_RUNNINGTIMER(cRunningTimer, "CEditView::DoGrep");

	// 再入不可
	if( this->m_bGrepRunning ){
		assert_warning( false == this->m_bGrepRunning );
		return 0xffffffff;
	}

	this->m_bGrepRunning = true;

	int			nHitCount = 0;
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel;
	//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	CBregexp	cRegexp;
	CNativeW	cmemMessage;
	int			nWork;
	SGrepOption	sGrepOption;

	/*
	|| バッファサイズの調整
	*/
	cmemMessage.AllocStringBuffer( 4000 );

	pcViewDst->m_bDoing_UndoRedo		= true;


	/* アンドゥバッファの処理 */
	if( NULL != pcViewDst->GetDocument()->m_cDocEditor.m_pcOpeBlk ){	/* 操作ブロック */
//@@@2002.2.2 YAZAKI NULLじゃないと進まないので、とりあえずコメント。＆NULLのときは、new COpeBlkする。
//		while( NULL != m_pcOpeBlk ){}
//		delete m_pcOpeBlk;
//		m_pcOpeBlk = NULL;
	}
	else {
		pcViewDst->GetDocument()->m_cDocEditor.m_pcOpeBlk = new COpeBlk;
		pcViewDst->GetDocument()->m_cDocEditor.m_nOpeBlkRedawCount = 0;
	}
	pcViewDst->GetDocument()->m_cDocEditor.m_pcOpeBlk->AddRef();

	pcViewDst->m_bCurSrchKeyMark = true;								/* 検索文字列のマーク */
	pcViewDst->m_strCurSearchKey = pcmGrepKey->GetStringPtr();				/* 検索文字列 */
	pcViewDst->m_sCurSearchOption = sSearchOption;						// 検索オプション
	pcViewDst->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;

	// 置換後文字列の準備
	CNativeW cmemReplace;
	if( bGrepReplace ){
		if( bGrepPaste ){
			// 矩形・ラインモード貼り付けは未サポート
			bool bColmnSelect;
			bool bLineSelect;
			if( !pcViewDst->MyGetClipboardData( cmemReplace, &bColmnSelect, GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste? &bLineSelect: NULL ) ){
				this->m_bGrepRunning = false;
				pcViewDst->m_bDoing_UndoRedo = false;
				ErrorMessage( pcViewDst->m_hwndParent, LS(STR_DLGREPLC_CLIPBOARD) );
				return 0;
			}
			if( bLineSelect ){
				int len = cmemReplace.GetStringLength();
				if( cmemReplace[len - 1] != WCODE::CR && cmemReplace[len - 1] != WCODE::LF ){
					cmemReplace.AppendString(pcViewDst->GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
				}
			}
			if( GetDllShareData().m_Common.m_sEdit.m_bConvertEOLPaste ){
				CLogicInt len = cmemReplace.GetStringLength();
				wchar_t	*pszConvertedText = new wchar_t[len * 2]; // 全文字\n→\r\n変換で最大の２倍になる
				CLogicInt nConvertedTextLen = pcViewDst->m_cCommander.ConvertEol(cmemReplace.GetStringPtr(), len, pszConvertedText);
				cmemReplace.SetString(pszConvertedText, nConvertedTextLen);
				delete [] pszConvertedText;
			}
		}else{
			cmemReplace = *pcmGrepReplace;
		}
	}
	/* 正規表現 */

	//	From Here Jun. 27 genta
	/*
		Grepを行うに当たって検索・画面色分け用正規表現バッファも
		初期化する．これはGrep検索結果の色分けを行うため．

		Note: ここで強調するのは最後の検索文字列であって
		Grep対象パターンではないことに注意
	*/
	if( !pcViewDst->m_sSearchPattern.SetPattern(pcViewDst->GetHwnd(), pcViewDst->m_strCurSearchKey.c_str(), pcViewDst->m_strCurSearchKey.size(),
			pcViewDst->m_sCurSearchOption, &pcViewDst->m_CurRegexp) ){
		this->m_bGrepRunning = false;
		pcViewDst->m_bDoing_UndoRedo = false;
		pcViewDst->SetUndoBuffer();
		return 0;
	}

	//2014.06.13 別ウィンドウで検索したとき用にGrepダイアログの検索キーを設定
	pcViewDst->m_pcEditWnd->m_cDlgGrep.m_strText = pcmGrepKey->GetStringPtr();
	pcViewDst->m_pcEditWnd->m_cDlgGrep.m_bSetText = true;
	pcViewDst->m_pcEditWnd->m_cDlgGrepReplace.m_strText = pcmGrepKey->GetStringPtr();
	if( bGrepReplace ){
		pcViewDst->m_pcEditWnd->m_cDlgGrepReplace.m_strText2 = pcmGrepReplace->GetStringPtr();
	}
	pcViewDst->m_pcEditWnd->m_cDlgGrepReplace.m_bSetText = true;
	hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), pcViewDst->m_hwndParent, IDD_GREPRUNNING );

	::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE );
	::DlgItem_SetText( hwndCancel, IDC_STATIC_CURFILE, _T(" ") );	// 2002/09/09 Moca add
	::CheckDlgButton( hwndCancel, IDC_CHECK_REALTIMEVIEW, GetDllShareData().m_Common.m_sSearch.m_bGrepRealTimeView );	// 2003.06.23 Moca

	//	2008.12.13 genta パターンが長すぎる場合は登録しない
	//	(正規表現が途中で途切れると困るので)
	//	2011.12.10 Moca 表示の際に...に切り捨てられるので登録するように
	wcsncpy_s( CAppMode::getInstance()->m_szGrepKey, _countof(CAppMode::getInstance()->m_szGrepKey), pcmGrepKey->GetStringPtr(), _TRUNCATE );
	this->m_bGrepMode = true;

	//	2007.07.22 genta
	//	バージョン番号取得のため，処理を前の方へ移動した
	CSearchStringPattern pattern;
	{
		/* 検索パターンのコンパイル */
		bool bError;
		if( bGrepReplace && !bGrepPaste ){
			// Grep置換
			// 2015.03.03 Grep置換がoptGlobalじゃないバグを修正
			bError = !pattern.SetPattern(pcViewDst->GetHwnd(), pcmGrepKey->GetStringPtr(), pcmGrepKey->GetStringLength(),
				cmemReplace.GetStringPtr(), sSearchOption, &cRegexp, true);
		}else{
			bError = !pattern.SetPattern(pcViewDst->GetHwnd(), pcmGrepKey->GetStringPtr(), pcmGrepKey->GetStringLength(),
				sSearchOption, &cRegexp);
		}
		if( bError ){
			this->m_bGrepRunning = false;
			pcViewDst->m_bDoing_UndoRedo = false;
			pcViewDst->SetUndoBuffer();
			return 0;
		}
	}
	
	// Grepオプションまとめ
	sGrepOption.bGrepSubFolder = FALSE != bGrepSubFolder;
	sGrepOption.bGrepStdout = bGrepStdout;
	sGrepOption.bGrepHeader = bGrepHeader;
	sGrepOption.nGrepCharSet = nGrepCharSet;
	sGrepOption.nGrepOutputLineType = nGrepOutputLineType;
	sGrepOption.nGrepOutputStyle = nGrepOutputStyle;
	sGrepOption.bGrepOutputFileOnly = bGrepOutputFileOnly;
	sGrepOption.bGrepOutputBaseFolder = bGrepOutputBaseFolder;
	sGrepOption.bGrepSeparateFolder = bGrepSeparateFolder;
	sGrepOption.bGrepReplace = bGrepReplace;
	sGrepOption.bGrepPaste = bGrepPaste;
	sGrepOption.bGrepBackup = bGrepBackup;
	if( sGrepOption.bGrepReplace ){
		// Grep否定行はGrep置換では無効
		if( sGrepOption.nGrepOutputLineType == 2 ){
			sGrepOption.nGrepOutputLineType = 1; // 行単位
		}
	}

//2002.02.08 Grepアイコンも大きいアイコンと小さいアイコンを別々にする。
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta アイコン読み込み方法変更
	hIconBig   = GetAppIcon( G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( G_AppInstance(), ICON_DEFAULT_GREP, FN_GREP_ICON, true );

	//	Sep. 10, 2002 genta
	//	CEditWndに新設した関数を使うように
	CEditWnd*	pCEditWnd = CEditWnd::getInstance();	//	Sep. 10, 2002 genta
	pCEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
	pCEditWnd->SetWindowIcon( hIconBig, ICON_BIG );

	CGrepEnumKeys cGrepEnumKeys;
	{
		int nErrorNo = cGrepEnumKeys.SetFileKeys( pcmGrepFile->GetStringPtr() );
		if( nErrorNo != 0 ){
			this->m_bGrepRunning = false;
			pcViewDst->m_bDoing_UndoRedo = false;
			pcViewDst->SetUndoBuffer();

			const TCHAR* pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS0);
			if( nErrorNo == 1 ){
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS1);
			}else if( nErrorNo == 2 ){
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS2);
			}
			ErrorMessage( pcViewDst->m_hwndParent, _T("%ts"), pszErrorMessage );
			return 0;
		}
	}


	std::vector<std::tstring> vPaths;
	CreateFolders( pcmGrepFolder->GetStringPtr(), vPaths );

	nWork = pcmGrepKey->GetStringLength(); // 2003.06.10 Moca あらかじめ長さを計算しておく

	/* 最後にテキストを追加 */
	CNativeW	cmemWork;
	cmemMessage.AppendString( LSW( STR_GREP_SEARCH_CONDITION ) );	//L"\r\n□検索条件  "
	if( 0 < nWork ){
		CNativeW cmemWork2;
		cmemWork2.SetNativeData( *pcmGrepKey );
		const STypeConfig& type = pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute();
		if( FALSE == type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp ){
			// 2011.11.28 色指定が無効ならエスケープしない
		}else
		if( type.m_nStringType == STRING_LITERAL_CPP || type.m_nStringType == STRING_LITERAL_CSHARP
			|| type.m_nStringType == STRING_LITERAL_PYTHON ){	/* 文字列区切り記号エスケープ方法 */
			cmemWork2.Replace( L"\\", L"\\\\" );
			cmemWork2.Replace( L"\'", L"\\\'" );
			cmemWork2.Replace( L"\"", L"\\\"" );
		}else if( type.m_nStringType == STRING_LITERAL_PLSQL ){
			cmemWork2.Replace( L"\'", L"\'\'" );
			cmemWork2.Replace( L"\"", L"\"\"" );
		}
		cmemWork.AppendString( L"\"" );
		cmemWork.AppendNativeData( cmemWork2 );
		cmemWork.AppendString( L"\"\r\n" );
	}else{
		cmemWork.AppendString( LSW( STR_GREP_SEARCH_FILE ) );	//L"「ファイル検索」\r\n"
	}
	cmemMessage += cmemWork;

	if( bGrepReplace ){
		cmemMessage.AppendString( LSW(STR_GREP_REPLACE_TO) );
		if( bGrepPaste ){
			cmemMessage.AppendString( LSW(STR_GREP_PASTE_CLIPBOAD) );
		}else{
			CNativeW cmemWork2;
			cmemWork2.SetNativeData( cmemReplace );
			const STypeConfig& type = pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute();
			if( FALSE == type.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp ){
				// 2011.11.28 色指定が無効ならエスケープしない
			}else
			if( type.m_nStringType == STRING_LITERAL_CPP || type.m_nStringType == STRING_LITERAL_CSHARP
				|| type.m_nStringType == STRING_LITERAL_PYTHON ){	/* 文字列区切り記号エスケープ方法 */
				cmemWork2.Replace( L"\\", L"\\\\" );
				cmemWork2.Replace( L"\'", L"\\\'" );
				cmemWork2.Replace( L"\"", L"\\\"" );
			}else if( type.m_nStringType == STRING_LITERAL_PLSQL ){
				cmemWork2.Replace( L"\'", L"\'\'" );
				cmemWork2.Replace( L"\"", L"\"\"" );
			}
			cmemMessage.AppendString( L"\"" );
			cmemMessage.AppendNativeData( cmemWork2 );
			cmemMessage.AppendString( L"\"\r\n" );
		}
	}


	cmemMessage.AppendString( LSW( STR_GREP_SEARCH_TARGET ) );	//L"検索対象   "
	if( pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemWork.SetStringT( pcmGrepFile->GetStringPtr() );
	cmemMessage += cmemWork;




	cmemMessage.AppendString( L"\r\n" );
	cmemMessage.AppendString( LSW( STR_GREP_SEARCH_FOLDER ) );	//L"フォルダ   "
	{
		std::tstring grepFolder;
		for( int i = 0; i < (int)vPaths.size(); i++ ){
			if( i ){
				grepFolder += _T(';');
			}
			std::tstring sPath = ChopYen( vPaths[i] );
			if( auto_strchr( sPath.c_str(), _T(';') ) ){
				grepFolder += _T('"');
				grepFolder += sPath;
				grepFolder += _T('"');
			}else{
				grepFolder += sPath;
			}
		}
		cmemWork.SetStringT( grepFolder.c_str() );
	}
	if( pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;
	cmemMessage.AppendString( L"\r\n" );

	const wchar_t*	pszWork;
	if( sGrepOption.bGrepSubFolder ){
		pszWork = LSW( STR_GREP_SUBFOLDER_YES );	//L"    (サブフォルダも検索)\r\n"
	}else{
		pszWork = LSW( STR_GREP_SUBFOLDER_NO );	//L"    (サブフォルダを検索しない)\r\n"
	}
	cmemMessage.AppendString( pszWork );

	if( 0 < nWork ){ // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if( sSearchOption.bWordOnly ){
		/* 単語単位で探す */
			cmemMessage.AppendString( LSW( STR_GREP_COMPLETE_WORD ) );	//L"    (単語単位で探す)\r\n"
		}

		if( sSearchOption.bLoHiCase ){
			pszWork = LSW( STR_GREP_CASE_SENSITIVE );	//L"    (英大文字小文字を区別する)\r\n"
		}else{
			pszWork = LSW( STR_GREP_IGNORE_CASE );	//L"    (英大文字小文字を区別しない)\r\n"
		}
		cmemMessage.AppendString( pszWork );

		if( sSearchOption.bRegularExp ){
			//	2007.07.22 genta : 正規表現ライブラリのバージョンも出力する
			cmemMessage.AppendString( LSW( STR_GREP_REGEX_DLL ) );	//L"    (正規表現:"
			cmemMessage.AppendStringT( cRegexp.GetVersionT() );
			cmemMessage.AppendString( L")\r\n" );
		}
	}

	if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
		cmemMessage.AppendString( LSW( STR_GREP_CHARSET_AUTODETECT ) );	//L"    (文字コードセットの自動判別)\r\n"
	}else if(IsValidCodeOrCPType(sGrepOption.nGrepCharSet)){
		cmemMessage.AppendString( LSW( STR_GREP_CHARSET ) );	//L"    (文字コードセット："
		TCHAR szCpName[100];
		CCodePage::GetNameNormal(szCpName, sGrepOption.nGrepCharSet);
		cmemMessage.AppendStringT( szCpName );
		cmemMessage.AppendString( L")\r\n" );
	}

	if( 0 < nWork ){ // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if( sGrepOption.nGrepOutputLineType == 1 ){
			/* 該当行 */
			pszWork = LSW( STR_GREP_SHOW_MATCH_LINE );	//L"    (一致した行を出力)\r\n"
		}else if( sGrepOption.nGrepOutputLineType == 2 ){
			// 否該当行
			pszWork = LSW( STR_GREP_SHOW_MATCH_NOHITLINE );	//L"    (一致しなかった行を出力)\r\n"
		}else{
			if( bGrepReplace && sSearchOption.bRegularExp && !bGrepPaste ){
				pszWork = LSW(STR_GREP_SHOW_FIRST_LINE);
			}else{
				pszWork = LSW( STR_GREP_SHOW_MATCH_AREA );
			}
		}
		cmemMessage.AppendString( pszWork );

		if( sGrepOption.bGrepOutputFileOnly ){
			pszWork = LSW( STR_GREP_SHOW_FIRST_MATCH );	//L"    (ファイル毎最初のみ検索)\r\n"
			cmemMessage.AppendString( pszWork );
		}
	}


	cmemMessage.AppendString( L"\r\n\r\n" );
	pszWork = cmemMessage.GetStringPtr( &nWork );
//@@@ 2002.01.03 YAZAKI Grep直後はカーソルをGrep直前の位置に動かす
	CLayoutInt tmp_PosY_Layout = pcViewDst->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( 0 < nWork && sGrepOption.bGrepHeader ){
		AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
	}
	cmemMessage._SetStringLength(0);
	pszWork = NULL;
	
	//	2007.07.22 genta バージョンを取得するために，
	//	正規表現の初期化を上へ移動


	/* 表示処理ON/OFF */
	// 2003.06.23 Moca 共通設定で変更できるように
	// 2008.06.08 ryoji 全ビューの表示ON/OFFを同期させる
//	SetDrawSwitch(false);
	if( !CEditWnd::getInstance()->UpdateTextWrap() )	// 折り返し方法関連の更新
		CEditWnd::getInstance()->RedrawAllViews( pcViewDst );	//	他のペインの表示を更新
	const bool bDrawSwitchOld = pcViewDst->SetDrawSwitch(0 != GetDllShareData().m_Common.m_sSearch.m_bGrepRealTimeView);

	CGrepEnumOptions cGrepEnumOptions;
	CGrepEnumFiles cGrepExceptAbsFiles;
	cGrepExceptAbsFiles.Enumerates(_T(""), cGrepEnumKeys.m_vecExceptAbsFileKeys, cGrepEnumOptions);
	CGrepEnumFolders cGrepExceptAbsFolders;
	cGrepExceptAbsFolders.Enumerates(_T(""), cGrepEnumKeys.m_vecExceptAbsFolderKeys, cGrepEnumOptions);

	int nGrepTreeResult = 0;

	for( int nPath = 0; nPath < (int)vPaths.size(); nPath++ ){
		bool bOutputBaseFolder = false;
		std::tstring sPath = ChopYen( vPaths[nPath] );
		int nTreeRet = DoGrepTree(
			pcViewDst,
			&cDlgCancel,
			pcmGrepKey->GetStringPtr(),
			cmemReplace,
			cGrepEnumKeys,
			cGrepExceptAbsFiles,
			cGrepExceptAbsFolders,
			sPath.c_str(),
			sPath.c_str(),
			sSearchOption,
			sGrepOption,
			pattern,
			&cRegexp,
			0,
			bOutputBaseFolder,
			&nHitCount,
			cmemMessage
		);
		if( nTreeRet == -1 ){
			nGrepTreeResult = -1;
			break;
		}
		nGrepTreeResult += nTreeRet;
	}
	if (0 < cmemMessage.GetStringLength()) {
		AddTail(pcViewDst, cmemMessage, sGrepOption.bGrepStdout);
		cmemMessage._SetStringLength(0);
	}
	if( -1 == nGrepTreeResult && sGrepOption.bGrepHeader ){
		const wchar_t* p = LSW( STR_GREP_SUSPENDED );	//L"中断しました。\r\n"
		CNativeW cmemSuspend;
		cmemSuspend.SetString( p );
		AddTail( pcViewDst, cmemSuspend, sGrepOption.bGrepStdout );
	}
	if( sGrepOption.bGrepHeader ){
		WCHAR szBuffer[128];
		if( bGrepReplace ){
			auto_sprintf( szBuffer, LSW(STR_GREP_REPLACE_COUNT), nHitCount );
		}else{
			auto_sprintf( szBuffer, LSW( STR_GREP_MATCH_COUNT ), nHitCount );
		}
		CNativeW cmemOutput;
		cmemOutput.SetString( szBuffer );
		AddTail( pcViewDst, cmemOutput, sGrepOption.bGrepStdout );
#if defined(_DEBUG) && defined(TIME_MEASURE)
		auto_sprintf( szBuffer, LSW(STR_GREP_TIMER), cRunningTimer.Read() );
		cmemOutput.SetString( szBuffer );
		AddTail( pcViewDst, cmemOutput, sGrepOption.bGrepStdout );
#endif
	}
	pcViewDst->GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), tmp_PosY_Layout), true );	//	カーソルをGrep直前の位置に戻す。

	cDlgCancel.CloseDialog( 0 );

	/* アクティブにする */
	ActivateFrameWindow( CEditWnd::getInstance()->GetHwnd() );


	/* アンドゥバッファの処理 */
	pcViewDst->SetUndoBuffer();

	//	Apr. 13, 2001 genta
	//	Grep実行後はファイルを変更無しの状態にする．
	pcViewDst->m_pcEditDoc->m_cDocEditor.SetModified(false,false);

	this->m_bGrepRunning = false;
	pcViewDst->m_bDoing_UndoRedo = false;

	/* 表示処理ON/OFF */
	pCEditWnd->SetDrawSwitchOfAllViews( bDrawSwitchOld );

	/* 再描画 */
	if( !pCEditWnd->UpdateTextWrap() )	// 折り返し方法関連の更新	// 2008.06.10 ryoji
		pCEditWnd->RedrawAllViews( NULL );

	if( !bGrepCurFolder ){
		// 現行フォルダを検索したフォルダに変更
		if( 0 < vPaths.size() ){
			::SetCurrentDirectory( vPaths[0].c_str() );
		}
	}

	return nHitCount;
}




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
	const wchar_t*			pszKey,				//!< [in] 検索キー
	const CNativeW&			cmGrepReplace,
	CGrepEnumKeys&			cGrepEnumKeys,		//!< [in] 検索対象ファイルパターン
	CGrepEnumFiles&			cGrepExceptAbsFiles,	//!< [in] 除外ファイル絶対パス
	CGrepEnumFolders&		cGrepExceptAbsFolders,	//!< [in] 除外フォルダ絶対パス
	const TCHAR*			pszPath,			//!< [in] 検索対象パス
	const TCHAR*			pszBasePath,		//!< [in] 検索対象パス(ベースフォルダ)
	const SSearchOption&	sSearchOption,		//!< [in] 検索オプション
	const SGrepOption&		sGrepOption,		//!< [in] Grepオプション
	const CSearchStringPattern& pattern,		//!< [in] 検索パターン
	CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	int						nNest,				//!< [in] ネストレベル
	bool&					bOutputBaseFolder,	//!< [i/o] ベースフォルダ名出力
	int*					pnHitCount,			//!< [i/o] ヒット数の合計
	CNativeW&				cmemMessage			//!< [i/o] Grep結果文字列
)
{
	int			i;
	int			count;
	LPCTSTR		lpFileName;
	int			nWork = 0;
	int			nHitCountOld = -100;
	bool		bOutputFolderName = false;
	int			nBasePathLen = auto_strlen(pszBasePath);
	CGrepEnumOptions cGrepEnumOptions;
	CGrepEnumFilterFiles cGrepEnumFilterFiles;
	cGrepEnumFilterFiles.Enumerates( pszPath, cGrepEnumKeys, cGrepEnumOptions, cGrepExceptAbsFiles );

	/*
	 * カレントフォルダのファイルを探索する。
	 */
	count = cGrepEnumFilterFiles.GetCount();
	for( i = 0; i < count; i++ ){
		lpFileName = cGrepEnumFilterFiles.GetFileName( i );

		DWORD dwNow = ::GetTickCount();
		if (dwNow - m_dwTickUICheck > UICHECK_INTERVAL_MILLISEC) {
			m_dwTickUICheck = dwNow;
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				goto cancel_return;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}

			/* 表示設定をチェック */
			CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
				0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
			);
		}

		//GREP実行！
		::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, lpFileName );

		std::tstring currentFile = pszPath;
		currentFile += _T("\\");
		currentFile += lpFileName;
		int nBasePathLen2 = nBasePathLen + 1;
		if( (int)auto_strlen(pszPath) < nBasePathLen2 ){
			nBasePathLen2 = nBasePathLen;
		}

		/* ファイル内の検索 */
		int nRet;
		if( sGrepOption.bGrepReplace ){
			nRet = DoGrepReplaceFile(
				pcViewDst,
				pcDlgCancel,
				pszKey,
				cmGrepReplace,
				lpFileName,
				sSearchOption,
				sGrepOption,
				pattern,
				pRegexp,
				pnHitCount,
				currentFile.c_str(),
				pszBasePath,
				(sGrepOption.bGrepSeparateFolder && sGrepOption.bGrepOutputBaseFolder ? pszPath + nBasePathLen2 : pszPath),
				(sGrepOption.bGrepSeparateFolder ? lpFileName : currentFile.c_str() + nBasePathLen + 1),
				bOutputBaseFolder,
				bOutputFolderName,
				cmemMessage
			);
		}else{
			nRet = DoGrepFile(
				pcViewDst,
				pcDlgCancel,
				pszKey,
				lpFileName,
				sSearchOption,
				sGrepOption,
				pattern,
				pRegexp,
				pnHitCount,
				currentFile.c_str(),
				pszBasePath,
				(sGrepOption.bGrepSeparateFolder && sGrepOption.bGrepOutputBaseFolder ? pszPath + nBasePathLen2 : pszPath),
				(sGrepOption.bGrepSeparateFolder ? lpFileName : currentFile.c_str() + nBasePathLen + 1),
				bOutputBaseFolder,
				bOutputFolderName,
				cmemMessage
			);
		}

		// 2003.06.23 Moca リアルタイム表示のときは早めに表示
		if( pcViewDst->GetDrawSwitch() ){
			if( LTEXT('\0') != pszKey[0] ){
				// データ検索のときファイルの合計が最大10MBを超えたら表示
				nWork += ( cGrepEnumFilterFiles.GetFileSizeLow( i ) + 1023 ) / 1024;
			}
			if (10000 < nWork) {
				nHitCountOld = -100; // 即表示
			}
		}
		/* 結果出力 */
		if( 0 < cmemMessage.GetStringLength() &&
		   (*pnHitCount - nHitCountOld) >= 10 &&
		   (::GetTickCount() - m_dwTickAddTail) > ADDTAIL_INTERVAL_MILLISEC
		){
			AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
			cmemMessage._SetStringLength(0);
			nWork = 0;
			nHitCountOld = *pnHitCount;
		}
		if( -1 == nRet ){
			goto cancel_return;
		}
	}

	/*
	 * サブフォルダを検索する。
	 */
	if( sGrepOption.bGrepSubFolder ){
		CGrepEnumOptions cGrepEnumOptionsDir;
		CGrepEnumFilterFolders cGrepEnumFilterFolders;
		cGrepEnumFilterFolders.Enumerates( pszPath, cGrepEnumKeys, cGrepEnumOptionsDir, cGrepExceptAbsFolders );

		count = cGrepEnumFilterFolders.GetCount();
		for( i = 0; i < count; i++ ){
			lpFileName = cGrepEnumFilterFolders.GetFileName( i );

			DWORD dwNow = ::GetTickCount();
			if ( dwNow - m_dwTickUICheck > UICHECK_INTERVAL_MILLISEC ) {
				m_dwTickUICheck = dwNow;
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
				CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
					0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
				);
			}

			//フォルダ名を作成する。
			// 2010.08.01 キャンセルでメモリーリークしてました
			std::tstring currentPath  = pszPath;
			currentPath += _T("\\");
			currentPath += lpFileName;

			int nGrepTreeResult = DoGrepTree(
				pcViewDst,
				pcDlgCancel,
				pszKey,
				cmGrepReplace,
				cGrepEnumKeys,
				cGrepExceptAbsFiles,
				cGrepExceptAbsFolders,
				currentPath.c_str(),
				pszBasePath,
				sSearchOption,
				sGrepOption,
				pattern,
				pRegexp,
				nNest + 1,
				bOutputBaseFolder,
				pnHitCount,
				cmemMessage
			);
			if( -1 == nGrepTreeResult ){
				goto cancel_return;
			}
			::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURPATH, pszPath );	//@@@ 2002.01.10 add サブフォルダから戻ってきたら...
		}
	}

	::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, LTEXT(" ") );	// 2002/09/09 Moca add

	return 0;


cancel_return:;
	/* 結果出力 */
	if( 0 < cmemMessage.GetStringLength() ){
		AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
		cmemMessage._SetStringLength(0);
	}

	return -1;
}




/*!	@brief Grep結果を構築する


	pWorkは充分なメモリ領域を持っているコト
	@date 2002/08/29 Moca バイナリーデータに対応 pnWorkLen 追加
	@date 2013.11.05 Moca cmemMessageに直接追加するように
*/
void CGrepAgent::SetGrepResult(
	/* データ格納先 */
	CNativeW& cmemMessage,
	/* マッチしたファイルの情報 */
	const TCHAR*		pszFilePath,	/*!< [in] フルパス or 相対パス*/
	const TCHAR*		pszCodeName,	/*!< [in] 文字コード情報．" [SJIS]"とか */
	/* マッチした行の情報 */
	LONGLONG	nLine,				/*!< [in] マッチした行番号(1～) */
	int			nColumn,			/*!< [in] マッチした桁番号(1～) */
	const wchar_t*	pCompareData,	/*!< [in] 行の文字列 */
	int			nLineLen,			/*!< [in] 行の文字列の長さ */
	int			nEolCodeLen,		/*!< [in] EOLの長さ */
	/* マッチした文字列の情報 */
	const wchar_t*	pMatchData,		/*!< [in] マッチした文字列 */
	int			nMatchLen,			/*!< [in] マッチした文字列の長さ */
	/* オプション */
	const SGrepOption&	sGrepOption
)
{

	CNativeW cmemBuf(L"");
	wchar_t strWork[64];
	const wchar_t * pDispData;
	int k;
	bool bEOL = true;
	int nMaxOutStr = 0;

	/* ノーマル */
	if( 1 == sGrepOption.nGrepOutputStyle ){
		if( sGrepOption.bGrepOutputBaseFolder || sGrepOption.bGrepSeparateFolder ){
			cmemBuf.AppendString( L"・" );
		}
		cmemBuf.AppendStringT( pszFilePath );
		::auto_sprintf( strWork, L"(%I64d,%d)", nLine, nColumn );
		cmemBuf.AppendString( strWork );
		cmemBuf.AppendStringT( pszCodeName );
		cmemBuf.AppendString( L": " );
		nMaxOutStr = 2000; // 2003.06.10 Moca 最大長変更
	}
	/* WZ風 */
	else if( 2 == sGrepOption.nGrepOutputStyle ){
		::auto_sprintf( strWork, L"・(%6I64d,%-5d): ", nLine, nColumn );
		cmemBuf.AppendString( strWork );
		nMaxOutStr = 2500; // 2003.06.10 Moca 最大長変更
	}
	// 結果のみ
	else if( 3 == sGrepOption.nGrepOutputStyle ){
		nMaxOutStr = 2500;
	}

	/* 該当行 */
	if( sGrepOption.nGrepOutputLineType != 0 ){
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
		if( 0 < k && WCODE::IsLineDelimiter(pMatchData[ k - 1 ], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
			bEOL = false;
		}
	}

	cmemMessage.AllocStringBuffer( cmemMessage.GetStringLength() + cmemBuf.GetStringLength() + 2 );
	cmemMessage.AppendNativeData( cmemBuf );
	cmemMessage.AppendString( pDispData, k );
	if( bEOL ){
		cmemMessage.AppendString( L"\r\n", 2 );
	}
}

static void OutputPathInfo(
	CNativeW&		cmemMessage,
	SGrepOption		sGrepOption,
	const TCHAR*	pszFullPath,
	const TCHAR*	pszBaseFolder,
	const TCHAR*	pszFolder,
	const TCHAR*	pszRelPath,
	const TCHAR*	pszCodeName,
	bool&			bOutputBaseFolder,
	bool&			bOutputFolderName,
	BOOL&			bOutFileName
)
{
	{
		// バッファを2^n 分確保する
		int n = 1024;
		int size = cmemMessage.GetStringLength() + 300;
		while( n < size ){
			n *= 2;
		}
		cmemMessage.AllocStringBuffer( n );
	}
	if( 3 == sGrepOption.nGrepOutputStyle ){
		return;
	}

	if( !bOutputBaseFolder && sGrepOption.bGrepOutputBaseFolder ){
		if( !sGrepOption.bGrepSeparateFolder && 1 == sGrepOption.nGrepOutputStyle ){
			cmemMessage.AppendString( L"■\"" );
		}else{
			cmemMessage.AppendString( L"◎\"" );
		}
		cmemMessage.AppendStringT( pszBaseFolder );
		cmemMessage.AppendString( L"\"\r\n" );
		bOutputBaseFolder = true;
	}
	if( !bOutputFolderName && sGrepOption.bGrepSeparateFolder ){
		if( pszFolder[0] ){
			cmemMessage.AppendString( L"■\"" );
			cmemMessage.AppendStringT( pszFolder );
			cmemMessage.AppendString( L"\"\r\n" );
		}else{
			cmemMessage.AppendString( L"■\r\n" );
		}
		bOutputFolderName = true;
	}
	if( 2 == sGrepOption.nGrepOutputStyle ){
		if( !bOutFileName ){
			const TCHAR* pszDispFilePath = ( sGrepOption.bGrepSeparateFolder || sGrepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;
			if( sGrepOption.bGrepSeparateFolder ){
				cmemMessage.AppendString( L"◆\"" );
			}else{
				cmemMessage.AppendString( L"■\"" );
			}
			cmemMessage.AppendStringT( pszDispFilePath );
			cmemMessage.AppendString( L"\"" );
			cmemMessage.AppendStringT( pszCodeName );
			cmemMessage.AppendString( L"\r\n" );
			bOutFileName = TRUE;
		}
	}
}

/*!
	Grep実行 (CFileLoadを使ったテスト版)

	@retval -1 GREPのキャンセル
	@retval それ以外 ヒット数(ファイル検索時はファイル数)

	@date 2001/06/27 genta	正規表現ライブラリの差し替え
	@date 2002/08/30 Moca CFileLoadを使ったテスト版
	@date 2004/03/28 genta 不要な引数nNest, bGrepSubFolder, pszPathを削除
*/
int CGrepAgent::DoGrepFile(
	CEditView*				pcViewDst,			//!< 
	CDlgCancel*				pcDlgCancel,		//!< [in] Cancelダイアログへのポインタ
	const wchar_t*			pszKey,				//!< [in] 検索パターン
	const TCHAR*			pszFile,			//!< [in] 処理対象ファイル名(表示用)
	const SSearchOption&	sSearchOption,		//!< [in] 検索オプション
	const SGrepOption&		sGrepOption,		//!< [in] Grepオプション
	const CSearchStringPattern& pattern,		//!< [in] 検索パターン
	CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	int*					pnHitCount,			//!< [i/o] ヒット数の合計．元々の値に見つかった数を加算して返す．
	const TCHAR*			pszFullPath,		//!< [in] 処理対象ファイルパス C:\Folder\SubFolder\File.ext
	const TCHAR*			pszBaseFolder,		//!< [in] 検索フォルダ C:\Folder
	const TCHAR*			pszFolder,			//!< [in] サブフォルダ SubFolder (!bGrepSeparateFolder) または C:\Folder\SubFolder (!bGrepSeparateFolder)
	const TCHAR*			pszRelPath,			//!< [in] 相対パス File.ext(bGrepSeparateFolder) または  SubFolder\File.ext(!bGrepSeparateFolder)
	bool&					bOutputBaseFolder,	//!< 
	bool&					bOutputFolderName,	//!< 
	CNativeW&				cmemMessage			//!< [i/o] Grep結果文字列
)
{
	int		nHitCount;
	LONGLONG	nLine;
	const wchar_t*	pszRes; // 2002/08/29 const付加
	ECodeType	nCharCode;
	const wchar_t*	pCompareData; // 2002/08/29 const付加
	int		nColumn;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
	CEol	cEol;
	int		nEolCodeLen;
	const STypeConfigMini* type;
	CDocTypeManager().GetTypeConfigMini( CDocTypeManager().GetDocumentTypeOfPath( pszFile ), &type );
	CFileLoad	cfl( type->m_encoding );	// 2012/12/18 Uchi 検査するファイルのデフォルトの文字コードを取得する様に
	int		nOldPercent = 0;

	int	nKeyLen = wcslen( pszKey );
	// ファイル名表示
	const TCHAR* pszDispFilePath = ( sGrepOption.bGrepSeparateFolder || sGrepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;

	//	ここでは正規表現コンパイルデータの初期化は不要

	const TCHAR*	pszCodeName; // 2002/08/29 const付加
	pszCodeName = _T("");
	nHitCount = 0;
	nLine = 0;

	/* 検索条件が長さゼロの場合はファイル名だけ返す */
	// 2002/08/29 行ループの前からここに移動
	if( 0 == nKeyLen ){
		TCHAR szCpName[100];
		if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
			// 2003.06.10 Moca コード判別処理をここに移動．
			// 判別エラーでもファイル数にカウントするため
			// ファイルの日本語コードセット判別
			// 2014.06.19 Moca ファイル名のタイプ別のm_encodingに変更
			CCodeMediator cmediator( type->m_encoding );
			nCharCode = cmediator.CheckKanjiCodeOfFile( pszFullPath );
			if( !IsValidCodeOrCPType(nCharCode) ){
				pszCodeName = _T("  [(DetectError)]");
			}else if( IsValidCodeType(nCharCode) ){
				pszCodeName = CCodeTypeName(nCharCode).Bracket();
			}else{
				CCodePage::GetNameBracket(szCpName, nCharCode);
				pszCodeName = szCpName;
			}
		}
		{
			const wchar_t* pszFormatFullPath = L"";
			const wchar_t* pszFormatBasePath2 = L"";
			const wchar_t* pszFormatFilePath = L"";
			const wchar_t* pszFormatFilePath2 = L"";
			if( 1 == sGrepOption.nGrepOutputStyle ){
				// ノーマル
				pszFormatFullPath   = L"%ts%ts\r\n";
				pszFormatBasePath2  = L"■\"%ts\"\r\n";
				pszFormatFilePath   = L"・\"%ts\"%ts\r\n";
				pszFormatFilePath2  = L"・\"%ts\"%ts\r\n";
			}else if( 2 == sGrepOption.nGrepOutputStyle ){
				/* WZ風 */
				pszFormatFullPath   = L"■\"%ts\"%ts\r\n";
				pszFormatBasePath2  = L"◎\"%ts\"\r\n";
				pszFormatFilePath   = L"◆\"%ts\"%ts\r\n";
				pszFormatFilePath2  = L"■\"%ts\"%ts\r\n";
			}else if( 3 == sGrepOption.nGrepOutputStyle ){
				// 結果のみ
				pszFormatFullPath   = L"%ts%ts\r\n";
				pszFormatBasePath2  = L"■\"%ts\"\r\n";
				pszFormatFilePath   = L"%ts\r\n";
				pszFormatFilePath2  = L"%ts\r\n";
			}
/*
			Base/Sep
			O / O  : (A)BaseFolder -> (C)Folder(Rel) -> (E)RelPath(File)
			O / X  : (B)BaseFolder ->                   (F)RelPath(RelFolder/File)
			X / O  :                  (D)Folder(Abs) -> (G)RelPath(File)
			X / X  : (H)FullPath
*/
			auto_array_ptr<wchar_t> pszWork(new wchar_t[auto_strlen(pszFullPath) + auto_strlen(pszCodeName) + 10]);
			wchar_t* szWork0 = &pszWork[0];
			if( sGrepOption.bGrepOutputBaseFolder || sGrepOption.bGrepSeparateFolder ){
				if( !bOutputBaseFolder && sGrepOption.bGrepOutputBaseFolder ){
					const wchar_t* pszFormatBasePath = L"";
					if( sGrepOption.bGrepSeparateFolder ){
						pszFormatBasePath = L"◎\"%ts\"\r\n";	// (A)
					}else{
						pszFormatBasePath = pszFormatBasePath2;	// (B)
					}
					auto_sprintf( szWork0, pszFormatBasePath, pszBaseFolder );
					cmemMessage.AppendString( szWork0 );
					bOutputBaseFolder = true;
				}
				if( !bOutputFolderName && sGrepOption.bGrepSeparateFolder ){
					if( pszFolder[0] ){
						auto_sprintf( szWork0, L"■\"%ts\"\r\n", pszFolder );	// (C), (D)
					}else{
						auto_strcpy( szWork0, L"■\r\n" );
					}
					cmemMessage.AppendString( szWork0 );
					bOutputFolderName = true;
				}
				auto_sprintf( szWork0,
					(sGrepOption.bGrepSeparateFolder ? pszFormatFilePath // (E)
						: pszFormatFilePath2),	// (F), (G)
					pszDispFilePath, pszCodeName );
				cmemMessage.AppendString( szWork0 );
			}else{
				auto_sprintf( szWork0, pszFormatFullPath, pszFullPath, pszCodeName );	// (H)
				cmemMessage.AppendString( szWork0 );
			}
		}
		++(*pnHitCount);
		::SetDlgItemInt( pcDlgCancel->GetHwnd(), IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
		return 1;
	}


	try{
	// ファイルを開く
	// FileCloseで明示的に閉じるが、閉じていないときはデストラクタで閉じる
	// 2003.06.10 Moca 文字コード判定処理もFileOpenで行う
	nCharCode = cfl.FileOpen( pszFullPath, true, sGrepOption.nGrepCharSet, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode() );
	TCHAR szCpName[100];
	{
		if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
			if( IsValidCodeType(nCharCode) ){
				auto_strcpy( szCpName, CCodeTypeName(nCharCode).Bracket() );
				pszCodeName = szCpName;
			}else{
				CCodePage::GetNameBracket(szCpName, nCharCode);
				pszCodeName = szCpName;
			}
		}
	}

	DWORD dwNow = ::GetTickCount();
	if ( dwNow - m_dwTickUICheck > UICHECK_INTERVAL_MILLISEC ) {
		m_dwTickUICheck = dwNow;
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
			return -1;
		}
		/* 中断ボタン押下チェック */
		if( pcDlgCancel->IsCanceled() ){
			return -1;
		}
	}
	int nOutputHitCount = 0;

	/* 検索条件が長さゼロの場合はファイル名だけ返す */
	// 2002/08/29 ファイルオープンの手前へ移動
	
	std::vector<std::pair<const wchar_t*, CLogicInt> > searchWords;
	if( sSearchOption.bWordOnly ){
		CSearchAgent::CreateWordList( searchWords, pszKey, nKeyLen );
	}

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
		// 2010.08.31 間隔を1/32にする
		if( 0 == nLine % 32 ) {
			DWORD dwNow = ::GetTickCount();
			if ( dwNow - m_dwTickUICheck > UICHECK_INTERVAL_MILLISEC ) {
				m_dwTickUICheck = dwNow;
				if (!::BlockingHook( pcDlgCancel->GetHwnd() )) {
					return -1;
				}
				/* 中断ボタン押下チェック */
				if( pcDlgCancel->IsCanceled() ){
					return -1;
				}
				//	2003.06.23 Moca 表示設定をチェック
				CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
					0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
				);
				// 2002/08/30 Moca 進行状態を表示する(5MB以上)
				if( 5000000 < cfl.GetFileSize() ){
					int nPercent = cfl.GetPercent();
					if( 5 <= nPercent - nOldPercent ){
						nOldPercent = nPercent;
						TCHAR szWork[10];
						::auto_sprintf( szWork, _T(" (%3d%%)"), nPercent );
						std::tstring str;
						str = str + pszFile + szWork;
						::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, str.c_str() );
					}
				}else{
					::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, pszFile );
				}
				::SetDlgItemInt( pcDlgCancel->GetHwnd(), IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURPATH, pszFolder );
			}
		}
		int nHitOldLine = nHitCount;
		int nHitCountOldLine = *pnHitCount;

		/* 正規表現検索 */
		if( sSearchOption.bRegularExp ){
			int nIndex = 0;
#ifdef _DEBUG
			int nIndexPrev = -1;
#endif

			//	Jun. 21, 2003 genta ループ条件見直し
			//	マッチ箇所を1行から複数検出するケースを標準に，
			//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
			//	ループ継続・打ち切り条件(nGrepOutputLineType)を逆にした．
			//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
			// From Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
			// 2010.08.25 行頭以外で^にマッチする不具合の修正
			while( nIndex <= nLineLen && pRegexp->Match( pLine, nLineLen, nIndex ) ){

					//	パターン発見
					nIndex = pRegexp->GetIndex();
					int matchlen = pRegexp->GetMatchLen();
#ifdef _DEBUG
					if( nIndex <= nIndexPrev ){
						MYTRACE( _T("ERROR: CEditView::DoGrepFile() nIndex <= nIndexPrev break \n") );
						break;
					}
					nIndexPrev = nIndex;
#endif
					++nHitCount;
					++(*pnHitCount);
					if( sGrepOption.nGrepOutputLineType != 2 ){
						OutputPathInfo(
							cmemMessage, sGrepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						SetGrepResult(
							cmemMessage, pszDispFilePath, pszCodeName,
							nLine, nIndex + 1, pLine, nLineLen, nEolCodeLen,
							pLine + nIndex, matchlen, sGrepOption
						);
					}
					// To Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
					//	Jun. 21, 2003 genta 行単位で出力する場合は1つ見つかれば十分
					if ( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ) {
						break;
					}
					//	探し始める位置を補正
					//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
					if( matchlen <= 0 ){
						matchlen = CNativeW::GetSizeOfChar( pLine, nLineLen, nIndex );
						if( matchlen <= 0 ){
							matchlen = 1;
						}
					}
					nIndex += matchlen;
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
			int nMatchLen;
			int nIdx = 0;
			// Jun. 26, 2003 genta 無駄なwhileは削除
			while( ( pszRes = CSearchAgent::SearchStringWord(pLine, nLineLen, nIdx, searchWords, sSearchOption.bLoHiCase, &nMatchLen) ) != NULL ){
				nIdx = pszRes - pLine + nMatchLen;
				++nHitCount;
				++(*pnHitCount);
				if( sGrepOption.nGrepOutputLineType != 2 ){
					OutputPathInfo(
						cmemMessage, sGrepOption,
						pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
						bOutputBaseFolder, bOutputFolderName, bOutFileName
					);
					SetGrepResult(
						cmemMessage, pszDispFilePath, pszCodeName,
						//	Jun. 25, 2002 genta
						//	桁位置は1始まりなので1を足す必要がある
						nLine, pszRes - pLine + 1, pLine, nLineLen, nEolCodeLen,
						pszRes, nMatchLen, sGrepOption
					);
				}

				// 2010.10.31 ryoji 行単位で出力する場合は1つ見つかれば十分
				if ( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ) {
					break;
				}
			}
		}
		else {
			/* 文字列検索 */
			int nColumnPrev = 0;
			//	Jun. 21, 2003 genta ループ条件見直し
			//	マッチ箇所を1行から複数検出するケースを標準に，
			//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
			//	ループ継続・打ち切り条件(nGrepOutputLineType)を逆にした．
			for (;;) {
				pszRes = CSearchAgent::SearchString(
					pCompareData,
					nLineLen,
					0,
					pattern
				);
				if(!pszRes)break;

				nColumn = pszRes - pCompareData + 1;

				++nHitCount;
				++(*pnHitCount);
				if( sGrepOption.nGrepOutputLineType != 2 ){
					OutputPathInfo(
						cmemMessage, sGrepOption,
						pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
						bOutputBaseFolder, bOutputFolderName, bOutFileName
					);
					SetGrepResult(
						cmemMessage, pszDispFilePath, pszCodeName,
						nLine, nColumn + nColumnPrev, pCompareData, nLineLen, nEolCodeLen,
						pszRes, nKeyLen, sGrepOption
					);
				}
				
				//	Jun. 21, 2003 genta 行単位で出力する場合は1つ見つかれば十分
				if ( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ) {
					break;
				}
				//	探し始める位置を補正
				//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
				//	nClom : マッチ位置
				//	matchlen : マッチした文字列の長さ
				int nPosDiff = nColumn += nKeyLen - 1;
				pCompareData += nPosDiff;
				nLineLen -= nPosDiff;
				nColumnPrev += nPosDiff;
			}
		}
		// 2014.09.23 否ヒット行を出力
		if( sGrepOption.nGrepOutputLineType == 2 ){
			bool bNoHit = nHitOldLine == nHitCount;
			// ヒット数を戻す
			nHitCount = nHitOldLine;
			*pnHitCount = nHitCountOldLine;
			// 否ヒット行だった
			if( bNoHit ){
				nHitCount++;
				(*pnHitCount)++;
				OutputPathInfo(
					cmemMessage, sGrepOption,
					pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
					bOutputBaseFolder, bOutputFolderName, bOutFileName
				);
				SetGrepResult(
					cmemMessage, pszDispFilePath, pszCodeName,
					nLine, 1, pLine, nLineLen, nEolCodeLen,
					pLine, nLineLen, sGrepOption
				);
			}
		}
		if( 0 < cmemMessage.GetStringLength() &&
		   (nHitCount - nOutputHitCount >= 10) &&
		   (::GetTickCount() - m_dwTickAddTail) >= ADDTAIL_INTERVAL_MILLISEC
		){
			nOutputHitCount = nHitCount;
			AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
			cmemMessage._SetStringLength(0);
		}

		// ファイル検索の場合は、1つ見つかったら終了
		if( sGrepOption.bGrepOutputFileOnly && 1 <= nHitCount ){
			break;
		}
	}

	// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
	cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		CNativeW str(LSW(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		cmemMessage.AppendNativeData( str );
		return 0;
	}
	catch( CError_FileRead ){
		CNativeW str(LSW(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		cmemMessage.AppendNativeData( str );
	} // 例外処理終わり

	return nHitCount;
}

class CError_WriteFileOpen
{
public:
	virtual ~CError_WriteFileOpen(){}
};

class CWriteData{
public:
	CWriteData(int& hit, LPCTSTR name, ECodeType code_, bool bBom_, bool bOldSave_, CNativeW& message)
		:nHitCount(hit)
		,fileName(name)
		,code(code_)
		,bBom(bBom_)
		,bOldSave(bOldSave_)
		,bufferSize(0)
		,out(NULL)
		,pcCodeBase(CCodeFactory::CreateCodeBase(code_,0))
		,memMessage(message)
		{}
	void AppendBuffer(const CNativeW& strLine)
	{
		if( !out ){
			bufferSize += strLine.GetStringLength();
			buffer.push_back(strLine);
			// 10MB 以上だったら出力してしまう
			if( 0xa00000 <= bufferSize ){
				OutputHead();
			}
		}else{
			Output(strLine);
		}
	}
	void OutputHead()
	{
		if( !out ){
			std::tstring name = fileName;
			name += _T(".skrnew");
			try{
				out = new CBinaryOutputStream(name.c_str(), true);
			}catch( CError_FileOpen ){
				throw CError_WriteFileOpen();
			}
			if( bBom ){
				CMemory cBom;
				pcCodeBase->GetBom(&cBom);
				out->Write(cBom.GetRawPtr(), cBom.GetRawLength());
			}
			for(size_t i = 0; i < buffer.size(); i++){
				Output(buffer[i]);
			}
			buffer.clear();
			std::deque<CNativeW>().swap(buffer);
		}
	}
	void Output(const CNativeW& strLine)
	{
		CMemory dest;
		pcCodeBase->UnicodeToCode(strLine, &dest);
		// 場合によっては改行ごとではないので、JIS/UTF-7での出力が一定でない可能性あり
		out->Write(dest.GetRawPtr(), dest.GetRawLength());
	}
	void Close()
	{
		if( nHitCount && out ){
			out->Close();
			delete out;
			out = NULL;
			if( bOldSave ){
				std::tstring oldFile = fileName;
				oldFile += _T(".skrold");
				if( fexist(oldFile.c_str()) ){
					if( FALSE == ::DeleteFile( oldFile.c_str() ) ){
						std::wstring msg = LSW(STR_GREP_REP_ERR_DELETE);
						msg += L"[";
						msg += to_wchar(oldFile.c_str());
						msg += L"]\r\n";
						memMessage.AppendString( msg.c_str() );
						return;
					}
				}
				if( FALSE == ::MoveFile( fileName, oldFile.c_str() ) ){
					std::wstring msg = LSW(STR_GREP_REP_ERR_REPLACE);
					msg += L"[";
					msg += to_wchar(oldFile.c_str());
					msg += L"]\r\n";
					memMessage.AppendString( msg.c_str() );
					return;
				}
			}else{
				if( FALSE == ::DeleteFile( fileName ) ){
					std::wstring msg = LSW(STR_GREP_REP_ERR_DELETE);
					msg += L"[";
					msg += to_wchar(fileName);
					msg += L"]\r\n";
					memMessage.AppendString( msg.c_str() );
					return;
				}
			}
			std::tstring name = std::tstring(fileName);
			name += _T(".skrnew");
			if( FALSE == ::MoveFile( name.c_str(), fileName ) ){
				std::wstring msg = LSW(STR_GREP_REP_ERR_REPLACE);
				msg += L"[";
				msg += to_wchar(fileName);
				msg += L"]\r\n";
				memMessage.AppendString( msg.c_str()  );
				return;
			}
		}
		return;
	}
	~CWriteData()
	{
		if( out ){
			out->Close();
			delete out;
			out = NULL;
			std::tstring name = std::tstring(fileName);
			name += _T(".skrnew");
			::DeleteFile( name.c_str() );
		}
	}
private:
	int& nHitCount;
	LPCTSTR fileName;
	ECodeType code;
	bool bBom;
	bool bOldSave;
	size_t bufferSize;
	std::deque<CNativeW> buffer;
	CBinaryOutputStream* out;
	std::auto_ptr<CCodeBase> pcCodeBase;
	CNativeW&	memMessage;
};

/*!
	Grep置換実行
	@date 2013.06.12 Moca 新規作成
*/
int CGrepAgent::DoGrepReplaceFile(
	CEditView*				pcViewDst,
	CDlgCancel*				pcDlgCancel,
	const wchar_t*			pszKey,
	const CNativeW&			cmGrepReplace,
	const TCHAR*			pszFile,
	const SSearchOption&	sSearchOption,
	const SGrepOption&		sGrepOption,
	const CSearchStringPattern& pattern,
	CBregexp*				pRegexp,		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	int*					pnHitCount,
	const TCHAR*			pszFullPath,
	const TCHAR*			pszBaseFolder,
	const TCHAR*			pszFolder,
	const TCHAR*			pszRelPath,
	bool&					bOutputBaseFolder,
	bool&					bOutputFolderName,
	CNativeW&				cmemMessage
)
{
	LONGLONG	nLine = 0;
	int		nHitCount = 0;
	ECodeType	nCharCode;
	BOOL	bOutFileName = FALSE;
	CEol	cEol;
	int		nEolCodeLen;
	int		nOldPercent = 0;
	int	nKeyLen = wcslen( pszKey );
	const TCHAR*	pszCodeName = _T("");

	const STypeConfigMini* type;
	CDocTypeManager().GetTypeConfigMini( CDocTypeManager().GetDocumentTypeOfPath( pszFile ), &type );
	CFileLoad	cfl( type->m_encoding );	// 2012/12/18 Uchi 検査するファイルのデフォルトの文字コードを取得する様に
	bool bBom;
	// ファイル名表示
	const TCHAR* pszDispFilePath = ( sGrepOption.bGrepSeparateFolder || sGrepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;


	try{
	// ファイルを開く
	// FileCloseで明示的に閉じるが、閉じていないときはデストラクタで閉じる
	// 2003.06.10 Moca 文字コード判定処理もFileOpenで行う
	nCharCode = cfl.FileOpen( pszFullPath, true, sGrepOption.nGrepCharSet, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode(), &bBom );
	CWriteData output(nHitCount, pszFullPath, nCharCode, bBom, sGrepOption.bGrepBackup, cmemMessage );
	TCHAR szCpName[100];
	{
		if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
			if( IsValidCodeType(nCharCode) ){
				auto_strcpy( szCpName, CCodeTypeName(nCharCode).Bracket() );
				pszCodeName = szCpName;
			}else{
				CCodePage::GetNameBracket(szCpName, nCharCode);
				pszCodeName = szCpName;
			}
		}
	}
	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
		return -1;
	}
	/* 中断ボタン押下チェック */
	if( pcDlgCancel->IsCanceled() ){
		return -1;
	}
	int nOutputHitCount = 0;

	std::vector<std::pair<const wchar_t*, CLogicInt> > searchWords;
	if( sSearchOption.bWordOnly ){
		CSearchAgent::CreateWordList( searchWords, pszKey, nKeyLen );
	}

	CNativeW cOutBuffer;
	// 注意 : cfl.ReadLine が throw する可能性がある
	CNativeW cUnicodeBuffer;
	while( RESULT_FAILURE != cfl.ReadLine( &cUnicodeBuffer, &cEol ) )
	{
		const wchar_t*	pLine = cUnicodeBuffer.GetStringPtr();
		int		nLineLen = cUnicodeBuffer.GetStringLength();

		nEolCodeLen = cEol.GetLen();
		++nLine;

		DWORD dwNow = ::GetTickCount();
		if ( dwNow - m_dwTickUICheck > UICHECK_INTERVAL_MILLISEC ) {
			m_dwTickUICheck = dwNow;
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook(pcDlgCancel->GetHwnd()) ){
				return -1;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				return -1;
			}
			//	2003.06.23 Moca 表示設定をチェック
			CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
				0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
			);
			// 2002/08/30 Moca 進行状態を表示する(5MB以上)
			if( 5000000 < cfl.GetFileSize() ){
				int nPercent = cfl.GetPercent();
				if( 5 <= nPercent - nOldPercent ){
					nOldPercent = nPercent;
					TCHAR szWork[10];
					::auto_sprintf( szWork, _T(" (%3d%%)"), nPercent );
					std::tstring str;
					str = str + pszFile + szWork;
					::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, str.c_str() );
				}
			}
		}
		cOutBuffer.SetString( L"", 0 );
		bool bOutput = true;
		if( sGrepOption.bGrepOutputFileOnly && 1 <= nHitCount ){
			bOutput = false;
		}

		/* 正規表現検索 */
		if( sSearchOption.bRegularExp ){
			int nIndex = 0;
			int nIndexOld = nIndex;
			int nMatchNum = 0;
			//	Jun. 21, 2003 genta ループ条件見直し
			//	マッチ箇所を1行から複数検出するケースを標準に，
			//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
			//	ループ継続・打ち切り条件(bGrepOutputLine)を逆にした．
			//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
			// From Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
			// 2010.08.25 行頭以外で^にマッチする不具合の修正
			while( nIndex <= nLineLen &&
				(( !sGrepOption.bGrepPaste && (nMatchNum = pRegexp->Replace( pLine, nLineLen, nIndex ))) || 
				 ( sGrepOption.bGrepPaste && pRegexp->Match( pLine, nLineLen, nIndex ))) ){
				//	パターン発見
				nIndex = pRegexp->GetIndex();
				int matchlen = pRegexp->GetMatchLen();
				if( bOutput ){
					OutputPathInfo(
						cmemMessage, sGrepOption,
						pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
						bOutputBaseFolder, bOutputFolderName, bOutFileName
					);
					/* Grep結果を、cmemMessageに格納する */
					SetGrepResult(
						cmemMessage, pszDispFilePath, pszCodeName,
						nLine, nIndex + 1,
						pLine, nLineLen, nEolCodeLen,
						pLine + nIndex, matchlen,
						sGrepOption
					);
					// To Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
					if( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ){
						bOutput = false;
					}
				}
				output.OutputHead();
				++nHitCount;
				++(*pnHitCount);
				if( !sGrepOption.bGrepPaste ){
					// gオプションでは行末まで一度に置換済み
					nHitCount += nMatchNum - 1;
					*pnHitCount += nMatchNum - 1;
					cOutBuffer.AppendString( pRegexp->GetString(), pRegexp->GetStringLen() );
					nIndexOld = nLineLen;
					break;
				}
				if( 0 < nIndex - nIndexOld ){
					cOutBuffer.AppendString( &pLine[nIndexOld], nIndex - nIndexOld );
				}
				cOutBuffer.AppendNativeData( cmGrepReplace );
				//	探し始める位置を補正
				//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
				if( matchlen <= 0 ){
					matchlen = CNativeW::GetSizeOfChar( pLine, nLineLen, nIndex );
					if( matchlen <= 0 ){
						matchlen = 1;
					}
				}
				nIndex += matchlen;
				nIndexOld = nIndex;
			}
			if( 0 < nLineLen - nIndexOld ){
				cOutBuffer.AppendString( &pLine[nIndexOld], nLineLen - nIndexOld );
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
			const wchar_t* pszRes;
			int nMatchLen;
			int nIdx = 0;
			int nOutputPos = 0;
			// Jun. 26, 2003 genta 無駄なwhileは削除
			while( pszRes = CSearchAgent::SearchStringWord(pLine, nLineLen, nIdx, searchWords, sSearchOption.bLoHiCase, &nMatchLen) ){
				nIdx = pszRes - pLine + nMatchLen;
				if( bOutput ){
					OutputPathInfo(
						cmemMessage, sGrepOption,
						pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
						bOutputBaseFolder, bOutputFolderName, bOutFileName
					);
					/* Grep結果を、cmemMessageに格納する */
					SetGrepResult(
						cmemMessage, pszDispFilePath, pszCodeName,
						//	Jun. 25, 2002 genta
						//	桁位置は1始まりなので1を足す必要がある
						nLine, pszRes - pLine + 1, pLine, nLineLen, nEolCodeLen,
						pszRes, nMatchLen,
						sGrepOption
					);
					if( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ){
						bOutput = false;
					}
				}
				output.OutputHead();
				++nHitCount;
				++(*pnHitCount);
				if( 0 < pszRes - pLine - nOutputPos ){
					cOutBuffer.AppendString( &pLine[nOutputPos], pszRes - pLine - nOutputPos );
				}
				cOutBuffer.AppendNativeData( cmGrepReplace );
				nOutputPos = pszRes - pLine + nMatchLen;
			}
			cOutBuffer.AppendString( &pLine[nOutputPos], nLineLen - nOutputPos );
		}
		else {
			/* 文字列検索 */
			int nColumnPrev = 0;
			const wchar_t*	pCompareData = pLine;
			int nCompareLen = nLineLen;
			//	Jun. 21, 2003 genta ループ条件見直し
			//	マッチ箇所を1行から複数検出するケースを標準に，
			//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
			//	ループ継続・打ち切り条件(bGrepOutputLine)を逆にした．
			for(;;){
				const wchar_t* pszRes = CSearchAgent::SearchString( pCompareData, nCompareLen, 0, pattern );
				if(!pszRes)break;

				int	nColumn = pszRes - pCompareData;
				if( bOutput ){
					OutputPathInfo(
						cmemMessage, sGrepOption,
						pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
						bOutputBaseFolder, bOutputFolderName, bOutFileName
					);
					/* Grep結果を、cmemMessageに格納する */
					SetGrepResult(
						cmemMessage, pszDispFilePath, pszCodeName,
						nLine, nColumn + nColumnPrev + 1, pLine, nLineLen, nEolCodeLen,
						pszRes, nKeyLen,
						sGrepOption
					);
					if( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ){
						bOutput = false;
					}
				}
				output.OutputHead();
				++nHitCount;
				++(*pnHitCount);
				if( nColumn ){
					cOutBuffer.AppendString( pCompareData, nColumn );
				}
				cOutBuffer.AppendNativeData( cmGrepReplace );
				//	探し始める位置を補正
				//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
				//	nClom : マッチ位置
				//	matchlen : マッチした文字列の長さ
				int nPosDiff = nColumn + nKeyLen;
				pCompareData += nPosDiff;
				nCompareLen -= nPosDiff;
				nColumnPrev += nPosDiff;
			}
			cOutBuffer.AppendString( &pLine[nColumnPrev], nLineLen - nColumnPrev );
		}
		output.AppendBuffer(cOutBuffer);

		if( 0 < cmemMessage.GetStringLength() &&
		   (::GetTickCount() - m_dwTickAddTail > ADDTAIL_INTERVAL_MILLISEC)
		){
			nOutputHitCount = nHitCount;
			AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
			cmemMessage._SetStringLength(0);
		}
	}

	// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
	cfl.FileClose();
	output.Close();
	} // try
	catch( CError_FileOpen ){
		CNativeW str(LSW(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		cmemMessage.AppendNativeData( str );
		return 0;
	}
	catch( CError_FileRead ){
		CNativeW str(LSW(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%ts", to_wchar(pszFullPath));
		cmemMessage.AppendNativeData( str );
	}
	catch( CError_WriteFileOpen ){
		std::tstring file = pszFullPath;
		file += _T(".skrnew");
		CNativeW str(LSW(STR_GREP_ERR_FILEWRITE));
		str.Replace(L"%ts", to_wchar(file.c_str()));
		cmemMessage.AppendNativeData( str );
	} // 例外処理終わり

	return nHitCount;
}

