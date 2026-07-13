/*! @file
	@brief Grep検索エージェント
	@note マルチスレッド対応・除外ファイル機能拡張
*/
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "agent/CGrepAgent.h"
#include "grep/CGrepEnumKeys.h"
#include "grep/CGrepEnumFilterFiles.h"
#include "grep/CGrepEnumFilterFolders.h"
#include "grep/GrepPathFormat.h"
#include "agent/CSearchAgent.h"
#include "dlg/CDlgCancel.h"
#include "dlg/CDlgGrep.h"
#include "_main/CAppMode.h"
#include "_main/CMutex.h"
#include "env/CShareData.h"
#include "env/CSakuraEnvironment.h"
#include "cmd/COpeBlk.h"
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
#include "util/string_ex2.h"
#include "util/string_ex.h"
#include "debug/CRunningTimer.h"
#include "apiwrap/StdApi.h"
#include "apiwrap/StdControl.h"
#include "CSelectLang.h"
#include "sakura_rc.h"
#include "config/system_constants.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <set>
#include <functional>
#include <string_view>
#include <chrono>

constexpr DWORD UICHECK_INTERVAL_MILLISEC = 100;	// UI確認の時間間隔
constexpr DWORD ADDTAIL_INTERVAL_MILLISEC = 50;	// 結果出力の時間間隔
constexpr DWORD UIFILENAME_INTERVAL_MILLISEC = 100;	// Cancelダイアログのファイル名表示更新間隔（P8: SendMessage コスト削減のため UICHECK_INTERVAL_MILLISEC と統一）


/*!
 * 指定された文字列をタイプ別設定に従ってエスケープする
 */
static CNativeW EscapeStringLiteral( const STypeConfig& type, const CNativeW& cmemString )
{
	CNativeW cmemWork2( cmemString );
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
	return cmemWork2;
}

/*!
 * パスリストを文字列化する
 */
class CFileLoadOrWnd{
	CFileLoad m_cfl;
	HWND m_hWnd;
	int m_nLineCurrent;
	int m_nLineNum;
public:
	CFileLoadOrWnd(const SEncodingConfig& encode, HWND hWnd)
		: m_cfl(encode)
		, m_hWnd(hWnd)
		, m_nLineCurrent(0)
		, m_nLineNum(0)
	{
	}
	~CFileLoadOrWnd() = default;
	ECodeType FileOpen(const WCHAR* pszFile, bool bBigFile, ECodeType charCode, int nFlag)
	{
		if( m_hWnd ){
			DWORD_PTR dwMsgResult = 0;
			if( 0 == ::SendMessageTimeout(m_hWnd, MYWM_GETLINECOUNT, 0, 0, SMTO_NORMAL, 10000, &dwMsgResult) ){
				// エラーかタイムアウト
				throw CError_FileOpen();
			}
			m_nLineCurrent = 0;
			m_nLineNum = (int)dwMsgResult;
			::SendMessageAny(m_hWnd, MYWM_GETFILEINFO, 0, 0);
			const EditInfo* editInfo = &GetDllShareData().m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;
			return editInfo->m_nCharCode;
		}
		return m_cfl.FileOpen(pszFile, bBigFile, charCode, nFlag);
	}
	EConvertResult ReadLine(CNativeW* buffer, CEol* pcEol){
		if( m_hWnd ){
			const int max_size = (int)GetDllShareData().m_sWorkBuffer.GetWorkBufferCount<const WCHAR>();
			const WCHAR* pLineData = GetDllShareData().m_sWorkBuffer.GetWorkBuffer<const WCHAR>();
			buffer->SetStringHoldBuffer(L"", 0);
			if( m_nLineNum <= m_nLineCurrent ){
				return RESULT_FAILURE;
			}
			int nLineOffset = 0;
			int nLineLen = 0; //初回用仮値
			do{
				// m_sWorkBuffer#m_Workの排他制御。外部コマンド出力/TraceOut/Diffが対象
				LockGuard<CMutex> guard( CShareData::GetMutexShareWork() );
				{
					nLineLen = (int)::SendMessageAny(m_hWnd, MYWM_GETLINEDATA, m_nLineCurrent, nLineOffset);
					if( nLineLen == 0 ){ return RESULT_FAILURE; } // EOF => 正常終了
					if( nLineLen < 0 ){ return RESULT_FAILURE; } // 何かエラー
					buffer->AllocStringBuffer(max_size);
					buffer->AppendString(pLineData, t_min(nLineLen, max_size));
				}
				nLineOffset += max_size;
			}while(max_size < nLineLen);
			if( 0 < nLineLen ){
				if( 1 < nLineLen && (*buffer)[nLineLen - 2] == WCODE::CR &&
						(*buffer)[nLineLen - 1] == WCODE::LF){
					pcEol->SetType(EEolType::cr_and_lf);
				}else{
					pcEol->SetTypeByString(buffer->GetStringPtr() + nLineLen - 1, 1);
				}
			}
			m_nLineCurrent++;
			return RESULT_COMPLETE;
		}
		return m_cfl.ReadLine(buffer, pcEol);
	}
	LONGLONG GetFileSize(){
		if( m_hWnd ){
			return 0;
		}
		return m_cfl.GetFileSize();
	}
	int GetPercent(){
		if( m_hWnd ){
			if( m_nLineNum <= 0 ){
				return 0; // 行数 0 のウインドウでのゼロ除算を防ぐ
			}
			return (int)(m_nLineCurrent * 100.0 / m_nLineNum);
		}
		return m_cfl.GetPercent();
	}
	
	void FileClose(){
		if( m_hWnd ){
			return;
		}
		m_cfl.FileClose();
	}
};

CGrepAgent::CGrepAgent() = default;

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

void CGrepAgent::OnAfterSave([[maybe_unused]] const SSaveInfo& sSaveInfo)
{
	// 名前を付けて保存から再ロードが除去された分の不足処理を追加（ANSI版との差異）	// 2009.08.12 ryoji
	m_bGrepMode = false;	// grepウィンドウは通常ウィンドウ化
	CAppMode::getInstance()->m_szGrepKey[0] = L'\0';
}

/*!
	@date 2014.03.09 novice 最後の\\を取り除くのをやめる(d:\\ -> d:になる)
*/
void CGrepAgent::CreateFolders( const WCHAR* pszPath, std::vector<std::wstring>& vPaths )
{
	std::wstring strPath( pszPath );
	const int nPathLen = static_cast<int>( strPath.length() );

	WCHAR* token;
	int nPathPos = 0;
	while( nullptr != (token = my_strtok<WCHAR>( strPath.data(), nPathLen, &nPathPos, L";")) ){
		std::wstring strTemp( token );
		// パスに含まれる '"' を削除する
		strTemp.erase( std::remove( strTemp.begin(), strTemp.end(), L'"' ), strTemp.end() );
		/* ロングファイル名を取得する */
		WCHAR szTmp2[_MAX_PATH];
		if( ::GetLongFileName( strTemp.c_str(), szTmp2 ) ){
			vPaths.push_back( szTmp2 );
		}else{
			vPaths.emplace_back( strTemp );
		}
	}
}

/*! 最後の\\を取り除く
	@date 2014.03.09 novice 新規作成
*/
std::wstring CGrepAgent::ChopYen( const std::wstring& str )
{
	std::wstring dst = str;
	size_t nPathLen = dst.length();

	// 最後のフォルダー区切り記号を削除する
	// [A:\]などのルートであっても削除
	for(size_t i = 0; i < nPathLen; i++ ){
		if( L'\\' == dst[i] && i == nPathLen - 1 ){
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
			std::unique_ptr<CCodeBase> pcCodeBase( CCodeFactory::CreateCodeBase(
					pcEditView->GetDocument()->GetDocumentEncoding(), 0) );
			pcCodeBase->UnicodeToCode( cmem, &cmemOut );
			DWORD dwWrite = 0;
			::WriteFile(out, cmemOut.GetRawPtr(), cmemOut.GetRawLength(), &dwWrite, nullptr);
		}
	}else{
		pcEditView->GetCommander().Command_ADDTAIL( cmem.GetStringPtr(), cmem.GetStringLength() );
		pcEditView->GetCommander().Command_GOFILEEND( FALSE );
		if( !CEditWnd::getInstance()->UpdateTextWrap() )	// 折り返し方法関連の更新	// 2008.06.10 ryoji
			CEditWnd::getInstance()->RedrawAllViews( pcEditView );	//	他のペインの表示を更新
	}
}

static int GetHwndTitle(HWND& hWndTarget, CNativeW* pmemTitle, WCHAR* pszWindowName, WCHAR* pszWindowPath, const WCHAR* pszFile)
{
	hWndTarget = nullptr;	//out引数をクリアする

	// :HWND: トークンの接頭辞定義と判定は CDlgGrep に共通化（2-E）
	constexpr auto cchTargetPrefix = int(std::size(CDlgGrep::HWND_FILE_TOKEN_PREFIX)) - 1;
	if( !CDlgGrep::IsHwndFileToken(pszFile) ){
		return 0; // ハンドルGrepではない
	}
	// HWND* を size_t* として読み書きする型パンニングを避けるため、一旦 size_t で受けてから変換する
	size_t nHwndValue = 0;
	if( 0 >= ::swscanf_s(pszFile + cchTargetPrefix, L"%zx", &nHwndValue) || !IsSakuraMainWindow((HWND)nHwndValue) ){
		return -1; // ハンドルを読み取れなかった、または、対象ウインドウハンドルが存在しない
	}
	hWndTarget = (HWND)nHwndValue;
	if( pmemTitle ){
		const wchar_t* p = L"Window:[";
		pmemTitle->SetStringHoldBuffer(p, 8);
	}
	::SendMessageAny(hWndTarget, MYWM_GETFILEINFO, 0, 0);
	EditInfo* editInfo = &(GetDllShareData().m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO);
	if( '\0' == editInfo->m_szPath[0] ){
		// Grepかアウトプットか無題
		WCHAR szTitle[_MAX_PATH];
		WCHAR szGrep[100];
		editInfo->m_bIsModified = false;
		const EditNode* node = CAppNodeManager::getInstance()->GetEditNode(hWndTarget);
		if( !node ){
			return -1; // 管理リストに見つからない（対象ウインドウが閉じられた直後など）
		}
		WCHAR* pszTagName = szTitle;
		if( editInfo->m_bIsGrep ){
			// Grepは検索キーとタグがぶつかることがあるので単に(Grep)と表示
			pszTagName = szGrep;
			wcsncpy_s(pszTagName, std::size(szGrep), L"(Grep)", _TRUNCATE);
		}
		CFileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape(szTitle, int(std::size(szTitle)), editInfo, node->m_nId, -1, nullptr );
#ifdef _WIN64
		auto_sprintf(pszWindowName, L":HWND:[%016I64x]%s", hWndTarget, pszTagName);
#else
		auto_sprintf(pszWindowName, L":HWND:[%08x]%s", hWndTarget, pszTagName);
#endif
		if( pmemTitle ){
			pmemTitle->AppendString(szTitle);
		}
		pszWindowPath[0] = L'\0';
	}else{
		SplitPath_FolderAndFile(editInfo->m_szPath, pszWindowPath, pszWindowName);
		if( pmemTitle ){
			pmemTitle->AppendString(pszWindowName);
		}
	}
	if( pmemTitle ){
		pmemTitle->AppendString(L"]");
	}
	return 1;
}


//!	@brief CGrepAgent::DoGrep のエラー・正常終了いずれの経路でも
//!	m_bGrepRunning / pcViewDst->m_bDoing_UndoRedo を確実にリセットする RAII ガード（A-3）
//!	@note 本ガード導入前は、HWNDエラー経路（GetHwndTitle が -1 を返す場合）のみ
//!	      このリセットが漏れており、以後 Grep が永続的に実行不能になる不具合があった。
struct SGrepRunningGuard {
	CGrepAgent*	pAgent;
	CEditView*	pView;
	SGrepRunningGuard( CGrepAgent* agent, CEditView* view ) noexcept : pAgent(agent), pView(view) {}
	SGrepRunningGuard(const SGrepRunningGuard&) = delete;
	SGrepRunningGuard& operator=(const SGrepRunningGuard&) = delete;
	~SGrepRunningGuard(){
		pAgent->m_bGrepRunning = false;
		pView->m_bDoing_UndoRedo = false;
	}
};

/*! Grep実行

  @param[in] grepInput 検索入力（キー/置換/ファイル/フォルダー）
  @param[in] sSearchOption 検索オプション
  @param[in] sGrepOption Grepオプション（値渡し・内部で正規化）

  @date 2008.12.07 nasukoji	ファイル名パターンのバッファオーバラン対策
  @date 2008.12.13 genta 検索パターンのバッファオーバラン対策
  @date 2012.10.13 novice 検索オプションをクラスごと代入
*/
DWORD CGrepAgent::DoGrep(
	CEditView*				pcViewDst,				//!< [in]  出力先ビュー
	const SGrepInput&		grepInput,				//!< [in]  検索入力（キー/置換/ファイル/フォルダー）
	const SSearchOption&	sSearchOption,			//!< [in]  検索オプション
	SGrepOption				sGrepOption,			//!< [in]  Grep オプション（内部で正規化するため値渡し）
	bool					bGrepCurFolder,			//!< [in]  カレントフォルダーを変更しない
	bool					bGrepExcludeFileRegexp	//!< [in]  除外ファイルを正規表現として扱う
)
{
	MY_RUNNINGTIMER( cRunningTimer, L"CEditView::DoGrep" );

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
	CNativeW	cUnicodeBuffer;
	int			nWork;

	/*
	|| バッファサイズの調整
	*/
	cmemMessage.AllocStringBuffer( 4000 );
	cUnicodeBuffer.AllocStringBuffer( 4000 );

	pcViewDst->m_bDoing_UndoRedo		= true;
	// A-3: 以降のどの return 経路でも m_bGrepRunning / m_bDoing_UndoRedo を確実にリセットする
	SGrepRunningGuard grepRunningGuard( this, pcViewDst );

	/* アンドゥバッファの処理 */
	if( nullptr != pcViewDst->GetDocument()->m_cDocEditor.m_pcOpeBlk ){	/* 操作ブロック */
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
	pcViewDst->m_strCurSearchKey = grepInput.pcmGrepKey->GetStringPtr();				/* 検索文字列 */
	pcViewDst->m_sCurSearchOption = sSearchOption;						// 検索オプション
	pcViewDst->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;

	// 置換後文字列の準備
	CNativeW cmemReplace;
	if( sGrepOption.bGrepReplace ){
		if( sGrepOption.bGrepPaste ){
			// 矩形・ラインモード貼り付けは未サポート
			bool bColmnSelect;
			bool bLineSelect = false;
			if( !pcViewDst->MyGetClipboardData( cmemReplace, &bColmnSelect, GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste? &bLineSelect: nullptr ) ){
				ErrorMessage( pcViewDst->m_hwndParent, LS(STR_DLGREPLC_CLIPBOARD) );
				return 0;
			}
			// 空クリップボード時に cmemReplace[len - 1] が範囲外参照になるのを防ぐ
			if( const int len = cmemReplace.GetStringLength();
				bLineSelect && ( len == 0 || (cmemReplace[len - 1] != WCODE::CR && cmemReplace[len - 1] != WCODE::LF) ) ){
				cmemReplace.AppendString(pcViewDst->GetDocument()->m_cDocEditor.GetNewLineCode().GetValue2());
			}
			if( GetDllShareData().m_Common.m_sEdit.m_bConvertEOLPaste ){
				CLogicInt len = cmemReplace.GetStringLength();
				std::vector<wchar_t> convertedText(std::max<size_t>(1, len * 2)); // 全文字\n→\r\n変換で最大の２倍になる（len==0 でも data() が有効になるよう最低 1 確保）
				auto nConvertedTextLen = pcViewDst->m_cCommander.ConvertEol(cmemReplace.GetStringPtr(), len, convertedText.data());
				cmemReplace.SetString(convertedText.data(), nConvertedTextLen);
			}
		}else{
			cmemReplace = *grepInput.pcmGrepReplace;
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
		pcViewDst->SetUndoBuffer();
		return 0;
	}

	//2014.06.13 別ウィンドウで検索したとき用にGrepダイアログの検索キーを設定
	GetEditWnd().m_cDlgGrep.m_strText = grepInput.pcmGrepKey->GetStringPtr();
	GetEditWnd().m_cDlgGrep.m_bSetText = true;
	GetEditWnd().m_cDlgGrepReplace.m_strText = grepInput.pcmGrepKey->GetStringPtr();
	if( sGrepOption.bGrepReplace ){
		GetEditWnd().m_cDlgGrepReplace.m_strText2 = grepInput.pcmGrepReplace->GetStringPtr();
	}
	GetEditWnd().m_cDlgGrepReplace.m_bSetText = true;
	hwndCancel = cDlgCancel.DoModeless( G_AppInstance(), pcViewDst->m_hwndParent, IDD_GREPRUNNING );

	::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE );
	ApiWrap::DlgItem_SetText( hwndCancel, IDC_STATIC_CURFILE, L" " );	// 2002/09/09 Moca add
	::CheckDlgButton( hwndCancel, IDC_CHECK_REALTIMEVIEW, GetDllShareData().m_Common.m_sSearch.m_bGrepRealTimeView );	// 2003.06.23 Moca

	//	2008.12.13 genta パターンが長すぎる場合は登録しない
	//	(正規表現が途中で途切れると困るので)
	//	2011.12.10 Moca 表示の際に...に切り捨てられるので登録するように
	wcsncpy_s( CAppMode::getInstance()->m_szGrepKey, int(std::size(CAppMode::getInstance()->m_szGrepKey)), grepInput.pcmGrepKey->GetStringPtr(), _TRUNCATE );
	this->m_bGrepMode = true;

	//	2007.07.22 genta
	//	バージョン番号取得のため，処理を前の方へ移動した
	CSearchStringPattern pattern;
	{
		/* 検索パターンのコンパイル */
		bool bError;
		if( sGrepOption.bGrepReplace && !sGrepOption.bGrepPaste ){
			// Grep置換
			// 2015.03.03 Grep置換がoptGlobalじゃないバグを修正
			bError = !pattern.SetPattern(pcViewDst->GetHwnd(), grepInput.pcmGrepKey->GetStringPtr(), grepInput.pcmGrepKey->GetStringLength(),
				cmemReplace.GetStringPtr(), sSearchOption, &cRegexp, true);
		}else{
			bError = !pattern.SetPattern(pcViewDst->GetHwnd(), grepInput.pcmGrepKey->GetStringPtr(), grepInput.pcmGrepKey->GetStringLength(),
				sSearchOption, &cRegexp);
		}
		if( bError ){
			pcViewDst->SetUndoBuffer();
			return 0;
		}
	}
	
	// Grep否定行はGrep置換では無効
	if( sGrepOption.bGrepReplace ){
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
		int nErrorNo = cGrepEnumKeys.SetFileKeys( grepInput.pcmGrepFile->GetStringPtr(), bGrepExcludeFileRegexp );
		if( nErrorNo != 0 ){
			pcViewDst->SetUndoBuffer();

			const WCHAR* pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS0);
			if( nErrorNo == 1 ){
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS1);
			}
			else if( nErrorNo == 2 ){
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS2);
			}
			ErrorMessage( pcViewDst->m_hwndParent, L"%s", pszErrorMessage );
			return 0;
		}
	}

	// 除外正規表現パターンの事前検証（UI初期化前に入力バリデーション）
	// `!` 付きの除外条件は検索開始前に失敗させて、ワーカー起動後の中断を避ける。
	for( const auto& pat : cGrepEnumKeys.m_vecExceptFileRegexPatterns ){
		CBregexp testRegexp;
		if( !InitRegexp( pcViewDst->m_hwndParent, testRegexp, true ) ){
			pcViewDst->SetUndoBuffer();
			return 0;
		}
		if( !testRegexp.Compile( pat.c_str(), CBregexp::optCaseSensitive ) ){
			ErrorMessage( pcViewDst->m_hwndParent, L"無効な除外正規表現パターン: %s", pat.c_str() );
			pcViewDst->SetUndoBuffer();
			return 0;
		}
	}

	// 出力対象ビューのタイプ別設定(grepout固定)
	const STypeConfig& type = pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	std::vector<std::wstring> vPaths;
	CreateFolders( grepInput.pcmGrepFolder->GetStringPtr(), vPaths );

	nWork = grepInput.pcmGrepKey->GetStringLength(); // 2003.06.10 Moca あらかじめ長さを計算しておく

	CNativeW formattedKey;
	if( 0 < nWork ){
		// ヘッダ文字列は個別に整形してから共通関数へ渡し、既存の表示順を保つ。
		formattedKey += EscapeStringLiteral(type, *grepInput.pcmGrepKey);
	}

	CNativeW formattedTarget;
	HWND hWndTarget = nullptr;
	WCHAR szWindowName[_MAX_PATH];
	WCHAR szWindowPath[_MAX_PATH];
	{
		int nHwndRet = GetHwndTitle(hWndTarget, &formattedTarget, szWindowName, szWindowPath, grepInput.pcmGrepFile->GetStringPtr());
		if( -1 == nHwndRet ){
			cmemMessage.AppendString(L"HWND handle error.\n");
			if( sGrepOption.bGrepHeader ){
				AddTail(pcViewDst, cmemMessage, sGrepOption.bGrepStdout);
			}
			return 0;
		}else if( 0 == nHwndRet ){
			{
				const auto& vecSearchFileKeys = cGrepEnumKeys.m_vecSearchFileKeys;
				std::wstring strPatterns = FormatPathList( vecSearchFileKeys );
				formattedTarget.SetString( strPatterns.c_str(), strPatterns.length() );
			}
		}
	}

	CNativeW formattedFolder;
	{
		std::list<std::wstring> folders;
		std::transform( vPaths.cbegin(), vPaths.cend(), std::back_inserter( folders ), []( const auto& path ) { return ChopYen( path ); } );
		std::wstring strPatterns = FormatPathList( folders );
		formattedFolder.AppendString( strPatterns.c_str(), strPatterns.length() );
	}

	CNativeW formattedReplace;
	if( sGrepOption.bGrepReplace && !sGrepOption.bGrepPaste ){
		formattedReplace += EscapeStringLiteral(type, cmemReplace);
	}
	CNativeW header = BuildGrepHeader(
		formattedKey.GetStringPtr(),
		formattedTarget.GetStringPtr(),
		formattedFolder.GetStringPtr(),
		sSearchOption,
		sGrepOption,
		sGrepOption.bGrepReplace ? formattedReplace.GetStringPtr() : nullptr
	);

	CNativeW excludeString;
	excludeString.AppendString(LS(STR_GREP_EXCLUDE_FILE));
	{
		auto excludeFiles = cGrepEnumKeys.GetExcludeFiles();
		std::wstring strPatterns = FormatPathList( excludeFiles );
		excludeString.AppendString( strPatterns.c_str(), strPatterns.length() );
	}
	excludeString.AppendString(L"\r\n");

	excludeString.AppendString(LS(STR_GREP_EXCLUDE_FOLDER));
	{
		auto excludeFolders = cGrepEnumKeys.GetExcludeFolders();
		std::wstring strPatterns = FormatPathList( excludeFolders );
		excludeString.AppendString( strPatterns.c_str(), strPatterns.length() );
	}
	excludeString.AppendString(L"\r\n");
	
	if( sGrepOption.bGrepSubFolder ){
		excludeString.AppendString( LS(STR_GREP_SUBFOLDER_YES) );
		header.Replace( LS(STR_GREP_SUBFOLDER_YES), excludeString.GetStringPtr() );
	}else{
		excludeString.AppendString( LS(STR_GREP_SUBFOLDER_NO) );
		header.Replace( LS(STR_GREP_SUBFOLDER_NO), excludeString.GetStringPtr() );
	}

	cmemMessage.AppendNativeData( header );

	nWork = cmemMessage.GetStringLength();
//@@@ 2002.01.03 YAZAKI Grep直後はカーソルをGrep直前の位置に動かす
	CLayoutInt tmp_PosY_Layout = pcViewDst->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( 0 < nWork && sGrepOption.bGrepHeader ){
		AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
	}
	cmemMessage._SetStringLength(0);
	
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
	cGrepExceptAbsFiles.Enumerates(L"", cGrepEnumKeys.m_vecExceptAbsFileKeys, cGrepEnumOptions);
	CGrepEnumFolders cGrepExceptAbsFolders;
	cGrepExceptAbsFolders.Enumerates(L"", cGrepEnumKeys.m_vecExceptAbsFolderKeys, cGrepEnumOptions);

	int nGrepTreeResult = 0;

	if( hWndTarget ){
		for( HWND hwnd = hWndTarget; nullptr != hwnd; hwnd = nullptr ){
			bool bOutputBaseFolder = false;
			bool bOutputFolderName = false;
			// 複数ウィンドウループ予約
			auto nPathLen = wcsnlen_s(szWindowPath, std::size(szWindowPath));
			std::wstring currentFile = szWindowPath;
			if( currentFile.size() ){
				currentFile += L'\\';
				nPathLen += 1;
			}
			currentFile += szWindowName;
			nHitCount = nGrepTreeResult;
			int nTreeRet = DoGrepFile(
				pcViewDst,
				&cDlgCancel,
				hwnd,
				grepInput.pcmGrepKey->GetStringPtr(),
				szWindowName,
				sSearchOption,
				sGrepOption,
				pattern,
				&cRegexp,
				&nHitCount,
				currentFile.c_str(),
				szWindowPath,
				(sGrepOption.bGrepSeparateFolder && sGrepOption.bGrepOutputBaseFolder ? L"" : szWindowPath),
				(sGrepOption.bGrepSeparateFolder ? szWindowName : currentFile.c_str() + nPathLen),
				bOutputBaseFolder,
				bOutputFolderName,
				cmemMessage,
				cUnicodeBuffer
			);
			if( nTreeRet == -1 ){
				nGrepTreeResult = -1;
				break;
			}
			nGrepTreeResult += nTreeRet;
		}
		if( 0 < cmemMessage.GetStringLength() ){
			AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
			pcViewDst->GetCommander().Command_GOFILEEND( false );
			if( !CEditWnd::getInstance()->UpdateTextWrap() )
				CEditWnd::getInstance()->RedrawAllViews( pcViewDst );
			cmemMessage.Clear();
		}
		nHitCount = nGrepTreeResult;
	}else if( sGrepOption.bGrepReplace ){
		// Grep置換は副作用（ファイル書き換え）を伴うため既存の直列処理を維持する
		for( int nPath = 0; nPath < (int)vPaths.size(); nPath++ ){
			bool bOutputBaseFolder = false;
			std::wstring sPath = ChopYen( vPaths[nPath] );
			int nTreeRet = DoGrepTree(
				pcViewDst,
				&cDlgCancel,
				grepInput.pcmGrepKey->GetStringPtr(),
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
				bOutputBaseFolder,
				&nHitCount,
				cmemMessage,
				cUnicodeBuffer
			);
			if( nTreeRet == -1 ){
				nGrepTreeResult = -1;
				break;
			}
			nGrepTreeResult += nTreeRet;
		}
		if( 0 < cmemMessage.GetStringLength() ) {
			AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
			cmemMessage._SetStringLength(0);
		}
	}else{
		int parallelHitCount = 0;
		const SGrepSearchParams searchParams{ grepInput.pcmGrepKey->GetStringPtr(), sSearchOption, sGrepOption };
		SGrepEnumContext enumCtx{ cGrepEnumKeys, cGrepExceptAbsFiles, cGrepExceptAbsFolders };
		nGrepTreeResult = RunParallelGrep(
			pcViewDst, &cDlgCancel,
			searchParams, enumCtx, vPaths,
			cmemMessage, parallelHitCount );
		nHitCount = parallelHitCount;
	}
	if( -1 == nGrepTreeResult && sGrepOption.bGrepHeader ){
		const wchar_t* p = LS( STR_GREP_SUSPENDED );	//L"中断しました。\r\n"
		CNativeW cmemSuspend;
		cmemSuspend.SetString( p );
		AddTail( pcViewDst, cmemSuspend, sGrepOption.bGrepStdout );
	}
	if( sGrepOption.bGrepHeader ){
		CNativeW cmemOutput = BuildGrepFooter(nHitCount, sGrepOption.bGrepReplace);
		AddTail( pcViewDst, cmemOutput, sGrepOption.bGrepStdout );
#if defined(_DEBUG) && defined(TIME_MEASURE)
		cmemOutput.SetString( strprintf( LS(STR_GREP_TIMER), cRunningTimer.Read() ).c_str() );
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
		pCEditWnd->RedrawAllViews( nullptr );

	if( !bGrepCurFolder ){
		// 現行フォルダーを検索したフォルダーに変更
		if( 0 < vPaths.size() ){
			::SetCurrentDirectory( vPaths[0].c_str() );
		}
	}

	return nHitCount;
}

/*!	@brief 除外正規表現パターンを "(?:p1)|(?:p2)|..." の1本に結合する（P4）
	@note ファイルあたりの Match 呼び出しを 1 回にするための前処理。
	      結合パターンのコンパイル失敗時は呼び出し側で個別パターン方式にフォールバックする。
*/
static std::wstring BuildCombinedExcludePattern( const std::vector<std::wstring>& patterns )
{
	std::wstring combined;
	for( const auto& pat : patterns ){
		if( !combined.empty() ){
			combined += L"|";
		}
		combined += L"(?:";
		combined += pat;
		combined += L")";
	}
	return combined;
}

/*!	@brief 1 パターンが P4 結合を阻害する構文を含むか判定する（S134: ネスト削減のため分離）
	@retval true 後方参照または名前付きグループを含む（結合不可）
*/
static bool HasCombineBlockingSyntax( const std::wstring& pat )
{
	size_t i = 0;
	while( i + 1 < pat.size() ){
		if( pat[i] == L'\\' ){
			if( const wchar_t c = pat[i + 1]; (L'1' <= c && c <= L'9') || c == L'k' || c == L'g' ){
				return true;	// \1～\9 / \k<name> / \g<name>
			}
			i += 2;	// エスケープ対象文字ごと読み飛ばす（"\\\\1" の誤検知防止）
			continue;
		}
		if( pat[i] == L'(' && pat[i + 1] == L'?' && i + 2 < pat.size() ){
			const wchar_t c2 = pat[i + 2];
			const bool bLookBehind = ( c2 == L'<' && i + 3 < pat.size()
				&& (pat[i + 3] == L'=' || pat[i + 3] == L'!') );
			if( (c2 == L'<' && !bLookBehind) || c2 == L'\'' ){
				return true;	// (?<name>...) / (?'name'...) 名前付きグループ
			}
		}
		i++;
	}
	return false;
}

/*!	@brief 除外正規表現パターン群が P4 結合可能か判定する
	@note "(?:p1)|(?:p2)" 結合はキャプチャグループ番号が全パターン通しで振り直される。
	      後方参照（\1～\9、\k、\g）や名前付きグループを含むパターンは結合すると
	      参照先が変わり意味が変化する（コンパイルは成功するためコンパイル失敗
	      フォールバックも働かない）。該当パターンを含む場合は結合せず個別コンパイル
	      方式を使う（安全側の誤判定は性能低下のみで動作は変わらない）。
*/
static bool CanCombineExcludePatterns( const std::vector<std::wstring>& patterns )
{
	return std::ranges::none_of( patterns, HasCombineBlockingSyntax );
}

/*!	@brief 除外正規表現パターン群をコンパイルする（S134: ネスト削減のため共通化）
	@note P4: 全除外パターンを1本に結合し、ファイルあたりのマッチ回数を1回にする。
	      後方参照・名前付きグループを含む場合、または結合パターンのコンパイルに
	      失敗した場合は従来の個別パターン方式にフォールバックする。
	      resize() は MoveInsertable を要求するが CBregexp はムーブ不可（CDllImp が= delete）。
	      vector(n) はデフォルト構築のみで済むため、ベクターのムーブ代入で代替する。
*/
static void CompileExcludeRegexps( const std::vector<std::wstring>& vecPatterns, std::vector<CBregexp>& regexps )
{
	if( vecPatterns.empty() ){
		return;
	}
	regexps = std::vector<CBregexp>( 1 );
	InitRegexp( nullptr, regexps[0], false );
	if( !CanCombineExcludePatterns( vecPatterns )
		|| !regexps[0].Compile( BuildCombinedExcludePattern( vecPatterns ).c_str(), CBregexp::optCaseSensitive ) ){
		regexps = std::vector<CBregexp>( vecPatterns.size() );
		for( size_t ri = 0; ri < regexps.size(); ri++ ){
			InitRegexp( nullptr, regexps[ri], false );
			regexps[ri].Compile( vecPatterns[ri].c_str(), CBregexp::optCaseSensitive );
		}
	}
}

/*! @brief Grep実行

	@date 2001.06.27 genta	正規表現ライブラリの差し替え
	@date 2003.06.23 Moca   サブフォルダー→ファイルだったのをファイル→サブフォルダーの順に変更
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
	CGrepEnumFolders&		cGrepExceptAbsFolders,	//!< [in] 除外フォルダー絶対パス
	const WCHAR*			pszPath,			//!< [in] 検索対象パス
	const WCHAR*			pszBasePath,		//!< [in] 検索対象パス(ベースフォルダー)
	const SSearchOption&	sSearchOption,		//!< [in] 検索オプション
	const SGrepOption&		sGrepOption,		//!< [in] Grepオプション
	const CSearchStringPattern& pattern,		//!< [in] 検索パターン
	CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	bool&					bOutputBaseFolder,	//!< [i/o] ベースフォルダー名出力
	int*					pnHitCount,			//!< [i/o] ヒット数の合計
	CNativeW&				cmemMessage,		//!< [i/o] Grep結果文字列
	CNativeW&				cUnicodeBuffer,
	std::vector<CBregexp>* pExclRegexps	//!< [in] コンパイル済み除外正規表現（nullptr: 内部でコンパイル）
)
{
	int			i;
	int			count;
	LPCWSTR		lpFileName;
	int			nWork = 0;
	int			nHitCountOld = -100;
	bool		bOutputFolderName = false;
	auto nBasePathLen = int(std::wstring_view(pszBasePath).length());

	// B-1: goto cancel_return の代替。残存メッセージをフラッシュしてキャンセル値(-1)を返す。
	auto FlushAndCancel = [&]() {
		if( 0 < cmemMessage.GetStringLength() ){
			AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
			cmemMessage._SetStringLength(0);
		}
		return -1;
	};

	CGrepEnumOptions cGrepEnumOptions;
	CGrepEnumFilterFiles cGrepEnumFilterFiles;
	cGrepEnumFilterFiles.Enumerates( pszPath, cGrepEnumKeys, cGrepEnumOptions, cGrepExceptAbsFiles );

	// P7: ループ内での文字列再構築を避けるため、パス部分を先に構築してファイル名部分だけ差し替える
	std::wstring currentFile = pszPath;
	currentFile += L"\\";
	const size_t nCurrentDirLen = currentFile.size();

	// 正規表現除外パターン: 呼び出し元からコンパイル済みが渡された場合は使い回す（再帰コスト削減）
	// 渡されなかった場合（最上位呼び出し）のみここでコンパイルする
	std::vector<CBregexp> localExclRegexps;
	std::vector<CBregexp>* pVecExclRegexps = pExclRegexps;

	if( pVecExclRegexps == nullptr ){
		// 再帰呼び出しでは同じ除外正規表現を共有し、無駄な再コンパイルを防ぐ。
		CompileExcludeRegexps( cGrepEnumKeys.m_vecExceptFileRegexPatterns, localExclRegexps );
		pVecExclRegexps = &localExclRegexps;
	}

	/*
	 * カレントフォルダーのファイルを探索する。
	 */
	count = cGrepEnumFilterFiles.GetCount();
	for( i = 0; i < count; i++ ){
		lpFileName = cGrepEnumFilterFiles.GetFileName( i );

		DWORD dwNow = ::GetTickCount();
		if( dwNow - m_dwTickUICheck > UICHECK_INTERVAL_MILLISEC ){
			m_dwTickUICheck = dwNow;
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
				return FlushAndCancel();
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				return FlushAndCancel();
			}

			/* 表示設定をチェック */
			CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
				0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
			);
		}

		// 定期的に grep 中のファイル名表示を更新
		if( dwNow - m_dwTickUIFileName > UIFILENAME_INTERVAL_MILLISEC ){
			m_dwTickUIFileName = dwNow;
			ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, lpFileName );
		}

		currentFile.resize( nCurrentDirLen );
		currentFile += lpFileName;
		int nBasePathLen2 = nBasePathLen + 1;
		if( (int)std::wstring_view(pszPath).length() < nBasePathLen2 ){
			nBasePathLen2 = nBasePathLen;
		}

		// 正規表現除外パターンによるフィルタリング（!プレフィックス指定）
		{
			bool bExcluded = false;
			for( auto& reExcl : *pVecExclRegexps ){
				if( reExcl.Match( currentFile.c_str(), (int)currentFile.size(), 0 ) ){
					bExcluded = true;
					break;
				}
			}
			if( bExcluded ) continue;
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
				cmemMessage,
				cUnicodeBuffer
			);
		}else{
			nRet = DoGrepFile(
				pcViewDst,
				pcDlgCancel,
				nullptr,
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
				cmemMessage,
				cUnicodeBuffer
			);
		}

		// 2003.06.23 Moca リアルタイム表示のときは早めに表示
		if( pcViewDst->GetDrawSwitch() ){
			if( L'\0' != pszKey[0] ){
				// データ検索のときファイルの合計が最大10MBを超えたら表示
				nWork += ( cGrepEnumFilterFiles.GetFileSizeLow( i ) + 1023 ) / 1024;
			}
			if( 10000 < nWork ){
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
			return FlushAndCancel();
		}
	}

	/*
	 * サブフォルダーを検索する。
	 */
	if( sGrepOption.bGrepSubFolder ){
		CGrepEnumOptions cGrepEnumOptionsDir;
		CGrepEnumFilterFolders cGrepEnumFilterFolders;
		cGrepEnumFilterFolders.Enumerates( pszPath, cGrepEnumKeys, cGrepEnumOptionsDir, cGrepExceptAbsFolders );

		count = cGrepEnumFilterFolders.GetCount();
		for( i = 0; i < count; i++ ){
			lpFileName = cGrepEnumFilterFolders.GetFileName( i );

			DWORD dwNow = ::GetTickCount();
			if( dwNow - m_dwTickUICheck > UICHECK_INTERVAL_MILLISEC ) {
				m_dwTickUICheck = dwNow;
				//サブフォルダーの探索を再帰呼び出し。
				/* 処理中のユーザー操作を可能にする */
				if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
					return FlushAndCancel();
				}
				/* 中断ボタン押下チェック */
				if( pcDlgCancel->IsCanceled() ){
					return FlushAndCancel();
				}
				/* 表示設定をチェック */
				CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
					0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
				);
			}

			//フォルダー名を作成する。
			// 2010.08.01 キャンセルでメモリリークしてました
			std::wstring currentPath  = pszPath;
			currentPath += L"\\";
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
				bOutputBaseFolder,
				pnHitCount,
				cmemMessage,
				cUnicodeBuffer,
				pVecExclRegexps	// コンパイル済みパターンを再帰に引き継ぐ（再コンパイル不要）
			);
			if( -1 == nGrepTreeResult ){
				return FlushAndCancel();
			}
			ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURPATH, pszPath );	//@@@ 2002.01.10 add サブフォルダーから戻ってきたら...
		}
	}

	ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, L" " );	// 2002/09/09 Moca add

	return 0;
}

/*!	@brief UICHECK 間隔ごとのキャンセル確認・表示設定更新
	@param[in] pcDlgCancel Cancelダイアログ（nullptr 時は何もしない）
	@param[in] dwNow 現在の GetTickCount() 値
	@retval 0 続行
	@retval -1 キャンセル（BlockingHook 失敗 or 中断ボタン）
*/
int CGrepAgent::CheckGrepCancelUI( CDlgCancel* pcDlgCancel, DWORD dwNow )
{
	if( pcDlgCancel == nullptr || dwNow - m_dwTickUICheck <= UICHECK_INTERVAL_MILLISEC ){
		return 0;
	}
	m_dwTickUICheck = dwNow;
	if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
		return -1;
	}
	if( pcDlgCancel->IsCanceled() ){
		return -1;
	}
	CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
		0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW )
	);
	return 0;
}

/*!	@brief フォルダー走査のみ行い、SGrepFileTask をベクターに積む（メインスレッド用）
	@note  ファイルの検索は行わない。DoGrep() からの並列化用エントリポイント。
*/
void CGrepAgent::DoGrepTreeEnumerate(
	CDlgCancel*					pcDlgCancel,	//!< [in]  Cancelダイアログ（nullptr 可）
	const SGrepOption&			sGrepOption,	//!< [in]  Grep オプション（パス整形フラグ用）
	const SGrepEnumContext&		enumCtx,		//!< [in]  列挙・除外オブジェクト群
	const WCHAR*				pszPath,		//!< [in]  列挙対象パス
	const WCHAR*				pszBasePath,	//!< [in]  ベースパス
	std::vector<SGrepFileTask>&	vecTasks,		//!< [out] 生成されたファイルタスク
	bool&						bCancelled		//!< [out] キャンセルされたら true
)
{
	int i;
	int count;
	LPCWSTR lpFileName;
	auto nBasePathLen = int(std::wstring_view(pszBasePath).length());
	CGrepEnumOptions cGrepEnumOptions;
	CGrepEnumFilterFiles cGrepEnumFilterFiles;
	cGrepEnumFilterFiles.Enumerates( pszPath, enumCtx.keys, cGrepEnumOptions, enumCtx.exceptFiles );

	// P7: ループ内での文字列再構築を避けるため、パス部分を先に構築してファイル名部分だけ差し替える
	std::wstring currentFile = pszPath;
	currentFile += L"\\";
	const size_t nCurrentDirLen = currentFile.size();

	count = cGrepEnumFilterFiles.GetCount();
	for( i = 0; i < count; i++ ){
		lpFileName = cGrepEnumFilterFiles.GetFileName( i );

		DWORD dwNow = ::GetTickCount();
		if( CheckGrepCancelUI( pcDlgCancel, dwNow ) != 0 ){
			bCancelled = true;
			return;
		}
		if( pcDlgCancel != nullptr && dwNow - m_dwTickUIFileName > UIFILENAME_INTERVAL_MILLISEC ){
			m_dwTickUIFileName = dwNow;
			ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, lpFileName );
		}

		currentFile.resize( nCurrentDirLen );
		currentFile += lpFileName;
		// pszPath 長がベースパス以下なら区切り '\\' 分を加算しない（従来の if と同値・分岐レス化）
		const int nBasePathLen2 = nBasePathLen
			+ static_cast<int>( nBasePathLen < (int)std::wstring_view(pszPath).length() );

		SGrepFileTask task;
		task.fullPath  = currentFile;
		task.fileName  = lpFileName;
		task.baseFolder = pszBasePath;
		task.folder    = ( sGrepOption.bGrepSeparateFolder && sGrepOption.bGrepOutputBaseFolder )
		                 ? std::wstring( pszPath + nBasePathLen2 ) : pszPath;
		task.relPath   = sGrepOption.bGrepSeparateFolder
		                 ? lpFileName : currentFile.c_str() + nBasePathLen + 1;
		vecTasks.push_back( std::move(task) );
	}

	if( sGrepOption.bGrepSubFolder ){
		CGrepEnumOptions cGrepEnumOptionsDir;
		CGrepEnumFilterFolders cGrepEnumFilterFolders;
		cGrepEnumFilterFolders.Enumerates( pszPath, enumCtx.keys, cGrepEnumOptionsDir, enumCtx.exceptFolders );

		count = cGrepEnumFilterFolders.GetCount();
		for( i = 0; i < count; i++ ){
			lpFileName = cGrepEnumFilterFolders.GetFileName( i );

			DWORD dwNow = ::GetTickCount();
			if( CheckGrepCancelUI( pcDlgCancel, dwNow ) != 0 ){
				bCancelled = true;
				return;
			}

			std::wstring currentPath = pszPath;
			currentPath += L"\\";
			currentPath += lpFileName;

			DoGrepTreeEnumerate(
				pcDlgCancel,
				sGrepOption,
				enumCtx,
				currentPath.c_str(),
				pszBasePath,
				vecTasks,
				bCancelled
			);
			if( bCancelled ) return;

			if( pcDlgCancel != nullptr ) ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURPATH, pszPath );
		}
	}

	if( pcDlgCancel != nullptr ) ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, L" " );
}

/*!	@brief マッチした行番号と桁番号をGrep結果に出力する為に文字列化
	auto_sprintf 関数を 書式文字列 "(%I64d,%d)" で実行するのと同等の処理結果を生成
	高速化の為に自前実装に置き換え
	@return 出力先文字列
*/
template <size_t nCapacity>
static inline
wchar_t* lineColumnToString(
	wchar_t (&strWork)[nCapacity],	/*!< [out] 出力先 */
	LONGLONG	nLine,				/*!< [in] マッチした行番号(1～) */
	int			nColumn				/*!< [in] マッチした桁番号(1～) */
)
{
	// int2dec_destBufferSufficientLength 関数の
	// 戻り値から -1 しているのは終端0文字の分を削っている為
	constexpr size_t requiredMinimumCapacity =
		1		// (
		+ int2dec_destBufferSufficientLength<LONGLONG>() - 1	// I64d
		+ 1		// ,
		+ int2dec_destBufferSufficientLength<int32_t>() - 1	// %d
		+ 1		// )
		+ 1		// \0 終端0文字の分
	;
	static_assert(nCapacity >= requiredMinimumCapacity, "nCapacity not enough.");
	wchar_t* p = strWork;
	*p++ = L'(';
	p += int2dec(nLine, p);
	*p++ = L',';
	p += int2dec(nColumn, p);
	*p++ = L')';
	*p = L'\0';
#ifdef _DEBUG
	// Debug 版に限って両方実行して、両者が一致することを確認
	wchar_t strWork2[requiredMinimumCapacity];
	::auto_sprintf( strWork2, L"(%I64d,%d)", nLine, nColumn );
	assert(wcscmp(strWork, strWork2) == 0);
#endif
	return strWork;
}

/*!	@brief Grep結果を構築する

	pWorkは充分なメモリ領域を持っているコト
	@date 2002/08/29 Moca バイナリーデータに対応 pnWorkLen 追加
	@date 2013.11.05 Moca cmemMessageに直接追加するように
*/
void CGrepAgent::SetGrepResult(
	CNativeW&				cmemMessage,	//!< [out] 整形結果の追記先
	const WCHAR*			pszFilePath,	//!< [in]  フルパス or 相対パス
	const WCHAR*			pszCodeName,	//!< [in]  文字コード情報"[SJIS]"とか
	const SGrepMatchInfo&	mi,				//!< [in]  ヒット位置情報
	const SGrepOption&		sGrepOption		//!< [in]  Grep オプション
)
{
	FormatGrepResultLine(cmemMessage, pszFilePath, pszCodeName, mi, sGrepOption);
}

/*!	@brief 検索結果 1 件分のフォーマット生成（HWND 非依存）
*/
void CGrepAgent::FormatGrepResultLine(
	CNativeW&				cmemMessage,	//!< [out] 整形結果の追記先
	const WCHAR*			pszFilePath,	//!< [in]  表示用ファイルパス
	const WCHAR*			pszCodeName,	//!< [in]  文字コード表記（"[SJIS]" 等）
	const SGrepMatchInfo&	mi,				//!< [in]  ヒット位置情報
	const SGrepOption&		sGrepOption		//!< [in]  Grep オプション
)
{
	// P7: 呼び出しごとの CNativeW 生成を避け、スレッドローカルのバッファを再利用する
	static thread_local CNativeW cmemBuf;
	cmemBuf.SetStringHoldBuffer( L"", 0 );
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
		cmemBuf.AppendString( pszFilePath );
		cmemBuf.AppendString( lineColumnToString(strWork, mi.nLine, mi.nColumn) );
		cmemBuf.AppendString( pszCodeName );
		cmemBuf.AppendString( L": " );
		nMaxOutStr = 2000; // 2003.06.10 Moca 最大長変更
	}
	/* WZ風 */
	else if( 2 == sGrepOption.nGrepOutputStyle ){
		::auto_sprintf( strWork, L"・(%6I64d,%-5d): ", mi.nLine, mi.nColumn );
		cmemBuf.AppendString( strWork );
		nMaxOutStr = 2500; // 2003.06.10 Moca 最大長変更
	}
	// 結果のみ
	else if( 3 == sGrepOption.nGrepOutputStyle ){
		nMaxOutStr = 2500;
	}

	/* 該当行 */
	if( sGrepOption.nGrepOutputLineType != 0 ){
		pDispData = mi.pCompareData;
		k = mi.nLineLen - mi.nEolCodeLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
	}
	/* 該当部分 */
	else{
		pDispData = mi.pMatchData;
		k = mi.nMatchLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
		// 該当部分に改行を含む場合はその改行コードをそのまま利用する(次の行に空行を作らない)
		// 2003.06.10 Moca k==0のときにバッファアンダーランしないように
		if( 0 < k && WCODE::IsLineDelimiter(mi.pMatchData[ k - 1 ], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
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
	const SGrepOption&	sGrepOption,
	const WCHAR*	pszFullPath,
	const WCHAR*	pszBaseFolder,
	const WCHAR*	pszFolder,
	const WCHAR*	pszRelPath,
	const WCHAR*	pszCodeName,
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
		cmemMessage.AppendString( pszBaseFolder );
		cmemMessage.AppendString( L"\"\r\n" );
		bOutputBaseFolder = true;
	}
	if( !bOutputFolderName && sGrepOption.bGrepSeparateFolder ){
		if( pszFolder[0] ){
			cmemMessage.AppendString( L"■\"" );
			cmemMessage.AppendString( pszFolder );
			cmemMessage.AppendString( L"\"\r\n" );
		}else{
			cmemMessage.AppendString( L"■\r\n" );
		}
		bOutputFolderName = true;
	}
	if( 2 == sGrepOption.nGrepOutputStyle ){
		if( !bOutFileName ){
			const WCHAR* pszDispFilePath = ( sGrepOption.bGrepSeparateFolder || sGrepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;
			if( sGrepOption.bGrepSeparateFolder ){
				cmemMessage.AppendString( L"◆\"" );
			}else{
				cmemMessage.AppendString( L"■\"" );
			}
			cmemMessage.AppendString( pszDispFilePath );
			cmemMessage.AppendString( L"\"" );
			cmemMessage.AppendString( pszCodeName );
			cmemMessage.AppendString( L"\r\n" );
			bOutFileName = TRUE;
		}
	}
}

/*!	@brief 1 行分の検索一致を判定して結果を出力する（DoGrepFile / DoGrepFileWorker 共通化・C-1）
	@note 正規表現/単語のみ/文字列の 3 分割と否ヒット行(nGrepOutputLineType==2)処理をまとめる。
	      pnGlobalHitCount が非 null のときのみ *pnGlobalHitCount にも加算する（直列版用）。
	@return この行で追加されたヒット数
*/
int CGrepAgent::MatchAndEmitGrepLine(
	const wchar_t*			pLine,
	int						nLineLen,
	LONGLONG				nLine,
	int						nEolCodeLen,
	int						nKeyLen,
	const std::vector<std::pair<const wchar_t*, CLogicInt>>& searchWords,
	const SSearchOption&	sSearchOption,
	const SGrepOption&		sGrepOption,
	const CSearchStringPattern&	pattern,
	CBregexp*				pRegexp,
	const SGrepOutputPaths&	paths,
	bool&					bOutputBaseFolder,
	bool&					bOutputFolderName,
	BOOL&					bOutFileName,
	int*					pnGlobalHitCount,
	CNativeW&				cmemMessage
)
{
	int nHitCount = 0;
	// 否ヒット行(type==2)処理で *pnGlobalHitCount を行開始値に巻き戻すための退避（直列版のみ使用）
	const int nGlobalOld = ( pnGlobalHitCount != nullptr ) ? *pnGlobalHitCount : 0;

	/* 正規表現検索 */
	if( sSearchOption.bRegularExp ){
		int nIndex = 0;
#ifdef _DEBUG
		int nIndexPrev = -1;
#endif
		while( nIndex <= nLineLen && pRegexp->Match( pLine, nLineLen, nIndex ) ){
			nIndex = pRegexp->GetIndex();
			int matchlen = pRegexp->GetMatchLen();
#ifdef _DEBUG
			if( nIndex <= nIndexPrev ){
				MYTRACE( L"ERROR: CGrepAgent::MatchAndEmitGrepLine() nIndex <= nIndexPrev break \n" );
				break;
			}
			nIndexPrev = nIndex;
#endif
			++nHitCount;
			// 直列版（DoGrepFile）のみ *pnHitCount にも加算する（並列版は nullptr）
			if( pnGlobalHitCount != nullptr ){ ++(*pnGlobalHitCount); }
			if( sGrepOption.nGrepOutputLineType != 2 ){
				OutputPathInfo(
					cmemMessage, sGrepOption,
					paths.pszFullPath, paths.pszBaseFolder, paths.pszFolder, paths.pszRelPath, paths.pszCodeName,
					bOutputBaseFolder, bOutputFolderName, bOutFileName
				);
				SetGrepResult(
					cmemMessage, paths.pszDispFilePath, paths.pszCodeName,
					SGrepMatchInfo{ nLine, nIndex + 1, pLine, nLineLen, nEolCodeLen, pLine + nIndex, matchlen },
					sGrepOption
				);
			}
			if( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ){
				break;
			}
			if( matchlen <= 0 ){
				const int nSizeOfChar = CNativeW::GetSizeOfChar( pLine, nLineLen, nIndex );
				matchlen = ( 0 < nSizeOfChar ) ? nSizeOfChar : 1;
			}
			nIndex += matchlen;
		}
	}
	/* 単語のみ検索 */
	else if( sSearchOption.bWordOnly ){
		const wchar_t* pszRes;
		int nMatchLen;
		int nIdx = 0;
		while( ( pszRes = CSearchAgent::SearchStringWord( pLine, nLineLen, nIdx, searchWords, sSearchOption.bLoHiCase, &nMatchLen ) ) != nullptr ){
			nIdx = int(pszRes - pLine + nMatchLen);
			++nHitCount;
			// 直列版（DoGrepFile）のみ *pnHitCount にも加算する（並列版は nullptr）
			if( pnGlobalHitCount != nullptr ){ ++(*pnGlobalHitCount); }
			if( sGrepOption.nGrepOutputLineType != 2 ){
				OutputPathInfo(
					cmemMessage, sGrepOption,
					paths.pszFullPath, paths.pszBaseFolder, paths.pszFolder, paths.pszRelPath, paths.pszCodeName,
					bOutputBaseFolder, bOutputFolderName, bOutFileName
				);
				SetGrepResult(
					cmemMessage, paths.pszDispFilePath, paths.pszCodeName,
					SGrepMatchInfo{ nLine, int(pszRes - pLine + 1), pLine, nLineLen, nEolCodeLen, pszRes, nMatchLen },
					sGrepOption
				);
			}
			if( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ){
				break;
			}
		}
	}
	else{
		/* 文字列検索 */
		int nColumnPrev = 0;
		const wchar_t* pCompareData = pLine;
		for(;;){
			const wchar_t* pszRes = CSearchAgent::SearchString( pCompareData, nLineLen, 0, pattern );
			if( !pszRes ){ break; }
			auto nColumn = int(pszRes - pCompareData + 1);
			++nHitCount;
			// 直列版（DoGrepFile）のみ *pnHitCount にも加算する（並列版は nullptr）
			if( pnGlobalHitCount != nullptr ){ ++(*pnGlobalHitCount); }
			if( sGrepOption.nGrepOutputLineType != 2 ){
				OutputPathInfo(
					cmemMessage, sGrepOption,
					paths.pszFullPath, paths.pszBaseFolder, paths.pszFolder, paths.pszRelPath, paths.pszCodeName,
					bOutputBaseFolder, bOutputFolderName, bOutFileName
				);
				SetGrepResult(
					cmemMessage, paths.pszDispFilePath, paths.pszCodeName,
					SGrepMatchInfo{ nLine, nColumn + nColumnPrev, pCompareData, nLineLen, nEolCodeLen, pszRes, nKeyLen },
					sGrepOption
				);
			}
			if( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ){
				break;
			}
			int nPosDiff = nColumn + nKeyLen - 1;
			pCompareData += nPosDiff;
			nLineLen     -= nPosDiff;
			nColumnPrev  += nPosDiff;
		}
	}

	/* 否ヒット行を出力 */
	if( sGrepOption.nGrepOutputLineType == 2 ){
		bool bNoHit = ( 0 == nHitCount );
		nHitCount = 0;
		// 直列版（DoGrepFile）のみ *pnHitCount を行開始値へ巻き戻す（並列版は nullptr）
		if( pnGlobalHitCount != nullptr ){
			*pnGlobalHitCount = nGlobalOld;
		}
		if( bNoHit ){
			nHitCount++;
			// 直列版（DoGrepFile）のみ *pnHitCount にも加算する（並列版は nullptr）
			if( pnGlobalHitCount != nullptr ){
				(*pnGlobalHitCount)++;
			}
			OutputPathInfo(
				cmemMessage, sGrepOption,
				paths.pszFullPath, paths.pszBaseFolder, paths.pszFolder, paths.pszRelPath, paths.pszCodeName,
				bOutputBaseFolder, bOutputFolderName, bOutFileName
			);
			SetGrepResult(
				cmemMessage, paths.pszDispFilePath, paths.pszCodeName,
				SGrepMatchInfo{ nLine, 1, pLine, nLineLen, nEolCodeLen, pLine, nLineLen },
				sGrepOption
			);
		}
	}

	return nHitCount;
}

/*!	@brief 検索キーが空（ファイル検索）の場合のファイル名出力（DoGrepFileWorker の CC 削減用）
	@retval 1 ヒット数（ファイル名 1 件）
*/
static int EmitFileNameOnlyForWorker(
	const SGrepFileTask&	task,
	const SGrepOption&		sGrepOption,
	const STypeConfigMini*	type,
	const WCHAR*			pszDispFilePath,
	CNativeW&				cmemMessage
)
{
	const WCHAR* pszCodeName = L"";
	WCHAR szCpName[100];

	if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
		CCodeMediator cmediator( type->m_encoding );
		const ECodeType nCharCode = cmediator.CheckKanjiCodeOfFile( task.fullPath.c_str() );

		if( !IsValidCodeOrCPType(nCharCode) ){
			pszCodeName = L"  [(DetectError)]";
		}else if( IsValidCodeType(nCharCode) ){
			pszCodeName = CCodeTypeName(nCharCode).Bracket();
		}else{
			CCodePage::GetNameBracket(szCpName, nCharCode);
			pszCodeName = szCpName;
		}
	}
	const wchar_t* pszFormatFullPath  = L"";
	const wchar_t* pszFormatFilePath  = L"";
	const wchar_t* pszFormatFilePath2 = L"";
	if( 1 == sGrepOption.nGrepOutputStyle ){
		pszFormatFullPath  = L"%s%s\r\n";
		pszFormatFilePath  = L"・\"%s\"%s\r\n";
		pszFormatFilePath2 = L"・\"%s\"%s\r\n";
	}else if( 2 == sGrepOption.nGrepOutputStyle ){
		pszFormatFullPath  = L"■\"%s\"%s\r\n";
		pszFormatFilePath  = L"◆\"%s\"%s\r\n";
		pszFormatFilePath2 = L"■\"%s\"%s\r\n";
	}else if( 3 == sGrepOption.nGrepOutputStyle ){
		pszFormatFullPath  = L"%s%s\r\n";
		pszFormatFilePath  = L"%s\r\n";
		pszFormatFilePath2 = L"%s\r\n";
	}
	auto pszWork = std::make_unique<wchar_t[]>( task.fullPath.size() + std::wstring_view(pszCodeName).length() + 10 );
	wchar_t* szWork0 = &pszWork[0];
	if( sGrepOption.bGrepOutputBaseFolder || sGrepOption.bGrepSeparateFolder ){
		auto_sprintf( szWork0,
			(sGrepOption.bGrepSeparateFolder ? pszFormatFilePath : pszFormatFilePath2),
			pszDispFilePath, pszCodeName );
		cmemMessage.AppendString( szWork0 );
	}else{
		auto_sprintf( szWork0, pszFormatFullPath, task.fullPath.c_str(), pszCodeName );
		cmemMessage.AppendString( szWork0 );
	}
	return 1;
}

/*!	@brief ワーカースレッド用ファイル内Grep処理（UI更新なし・atomic cancelフラグ使用）

	フォルダーヘッダー（bOutputBaseFolder/bOutputFolderName）は呼び出し元の
	並列オーケストレーターが管理するため、本関数内では出力しない（true固定）。
	WZ風スタイルのファイルヘッダー（bOutFileName）はファイル単位なので本関数内で制御する。

	@retval -1 キャンセル
	@retval それ以外 ヒット数
*/
int CGrepAgent::DoGrepFileWorker(
	const SGrepSearchParams&	searchParams,	//!< [in] 検索条件（キー/検索/Grepオプション）
	const SGrepFileTask&		task,			//!< [in] ファイルタスク
	CBregexp*					pLocalRegexp,	//!< [in] スレッドローカルCBregexpインスタンス
	const CSearchStringPattern&	localPattern,	//!< [in] スレッドローカル検索パターン
	const std::atomic<bool>&	bCancelled,		//!< [in] キャンセルフラグ
	CNativeW&					cmemMessage,	//!< [out] 結果バッファ（スレッドローカル）
	CNativeW&					cUnicodeBuffer	//!< [out] 行読み込みバッファ（スレッドローカル）
)
{
	const wchar_t* pszKey = searchParams.pszKey;
	const SSearchOption& sSearchOption = searchParams.sSearchOption;
	const SGrepOption& sGrepOption = searchParams.sGrepOption;

	int			nHitCount = 0;
	LONGLONG	nLine = 0;
	ECodeType	nCharCode;
	BOOL		bOutFileName = FALSE;
	CEol		cEol;
	int			nEolCodeLen;

	const STypeConfigMini* type = nullptr;
	if( !CDocTypeManager().GetTypeConfigMini( CDocTypeManager().GetDocumentTypeOfPath( task.fileName.c_str() ), &type ) ){
		return -1;
	}
	// 拡張子ベースのタイプ別設定を使うため、判定対象はフルパスではなくファイル名にする。
	CFileLoad cfl( type->m_encoding );

	auto nKeyLen = int(std::wstring_view(pszKey).length());
	const WCHAR* pszDispFilePath = ( sGrepOption.bGrepSeparateFolder || sGrepOption.bGrepOutputBaseFolder )
	                               ? task.relPath.c_str() : task.fullPath.c_str();
	const WCHAR* pszCodeName = L"";

	// フォルダーヘッダーはオーケストレーター側で管理するためワーカー内では出力しない。
	bool bOutputBaseFolder = true;
	bool bOutputFolderName = true;

	/* 検索条件が長さゼロの場合はファイル名だけ返す */
	if( 0 == nKeyLen ){
		return EmitFileNameOnlyForWorker( task, sGrepOption, type, pszDispFilePath, cmemMessage );
	}

	try{
		nCharCode = cfl.FileOpen( task.fullPath.c_str(), true, sGrepOption.nGrepCharSet,
		                          GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode() );
		WCHAR szCpName[100];
		if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
			if( IsValidCodeType(nCharCode) ){
				wcscpy_s( szCpName, _countof(szCpName), CCodeTypeName(nCharCode).Bracket() );
				pszCodeName = szCpName;
			}else{
				CCodePage::GetNameBracket(szCpName, nCharCode);
				pszCodeName = szCpName;
			}
		}

		if( bCancelled.load() ){
			return -1;
		}

		std::vector<std::pair<const wchar_t*, CLogicInt>> searchWords;
		if( sSearchOption.bWordOnly ){
			CSearchAgent::CreateWordList( searchWords, pszKey, nKeyLen );
		}

		while( RESULT_FAILURE != cfl.ReadLine( &cUnicodeBuffer, &cEol ) )
		{
			const wchar_t* pLine    = cUnicodeBuffer.GetStringPtr();
			int            nLineLen = cUnicodeBuffer.GetStringLength();
			nEolCodeLen = cEol.GetLen();
			++nLine;

			// キャンセルチェック（32行ごと、メインスレッドのUIは不要）
			if( 0 == nLine % 32 && bCancelled.load() ){
				return -1;
			}

			const SGrepOutputPaths grepPaths{ task.fullPath.c_str(), task.baseFolder.c_str(), task.folder.c_str(), task.relPath.c_str(), pszDispFilePath, pszCodeName };
			nHitCount += MatchAndEmitGrepLine(
				pLine, nLineLen, nLine, nEolCodeLen, nKeyLen,
				searchWords, sSearchOption, sGrepOption, localPattern, pLocalRegexp,
				grepPaths, bOutputBaseFolder, bOutputFolderName, bOutFileName,
				nullptr, cmemMessage
			);

			if( sGrepOption.bGrepOutputFileOnly && 1 <= nHitCount ){
				break;
			}
		}
		cfl.FileClose();
	}
	catch( const CError_FileOpen& ){
		CNativeW str(LS(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%s", task.fullPath);
		cmemMessage.AppendNativeData( str );
		return 0;
	}
	catch( const CError_FileRead& ){
		CNativeW str(LS(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%s", task.fullPath);
		cmemMessage.AppendNativeData( str );
	}

	return nHitCount;
}

/*!	@brief DoGrepFile 読み込みループ用: キャンセル確認・進捗表示更新
	@retval 0 続行
	@retval -1 キャンセル
*/
static int CheckCancelAndUpdateProgress(
	CDlgCancel*		pcDlgCancel,
	CFileLoadOrWnd&	cfl,
	const WCHAR*	pszFile,
	const WCHAR*	pszFolder,
	int				nHitCount,
	int&			nOldPercent,
	DWORD&			dwTickUICheck
)
{
	DWORD dwNowLoop = ::GetTickCount();
	if ( dwNowLoop - dwTickUICheck <= UICHECK_INTERVAL_MILLISEC ) {
		return 0;
	}
	dwTickUICheck = dwNowLoop;
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
			WCHAR szWork[10];
			::auto_sprintf( szWork, L" (%3d%%)", nPercent );
			std::wstring str;
			str = str + pszFile + szWork;
			ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, str.c_str() );
		}
	}else{
		ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, pszFile );
	}
	::SetDlgItemInt( pcDlgCancel->GetHwnd(), IDC_STATIC_HITCOUNT, nHitCount, FALSE );
	ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURPATH, pszFolder );
	return 0;
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
	HWND					hWndTarget,			//!< [in] 対象Windows(NULLでファイル)
	const wchar_t*			pszKey,				//!< [in] 検索パターン
	const WCHAR*			pszFile,			//!< [in] 処理対象ファイル名(表示用)
	const SSearchOption&	sSearchOption,		//!< [in] 検索オプション
	const SGrepOption&		sGrepOption,		//!< [in] Grepオプション
	const CSearchStringPattern& pattern,		//!< [in] 検索パターン
	CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	int*					pnHitCount,			//!< [i/o] ヒット数の合計．元々の値に見つかった数を加算して返す．
	const WCHAR*			pszFullPath,		//!< [in] 処理対象ファイルパス C:\Folder\SubFolder\File.ext
	const WCHAR*			pszBaseFolder,		//!< [in] 検索フォルダー C:\Folder
	const WCHAR*			pszFolder,			//!< [in] サブフォルダー SubFolder (!bGrepSeparateFolder) または C:\Folder\SubFolder (!bGrepSeparateFolder)
	const WCHAR*			pszRelPath,			//!< [in] 相対パス File.ext(bGrepSeparateFolder) または  SubFolder\File.ext(!bGrepSeparateFolder)
	bool&					bOutputBaseFolder,	//!< 
	bool&					bOutputFolderName,	//!< 
	CNativeW&				cmemMessage,		//!< [i/o] Grep結果文字列
	CNativeW&				cUnicodeBuffer
)
{
	int		nHitCount;
	LONGLONG	nLine;
	ECodeType	nCharCode;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
	CEol	cEol;
	int		nEolCodeLen;
	const STypeConfigMini* type = nullptr;
	if( !CDocTypeManager().GetTypeConfigMini( CDocTypeManager().GetDocumentTypeOfPath( pszFile ), &type ) ){
		return -1;
	}
	CFileLoadOrWnd	cfl( type->m_encoding, hWndTarget );	// 2012/12/18 Uchi 検査するファイルのデフォルトの文字コードを取得する様に
	int		nOldPercent = 0;

	auto nKeyLen = int(std::wstring_view(pszKey).length());
	// ファイル名表示
	const WCHAR* pszDispFilePath = ( sGrepOption.bGrepSeparateFolder || sGrepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;

	//	ここでは正規表現コンパイルデータの初期化は不要

	const WCHAR*	pszCodeName; // 2002/08/29 const付加
	pszCodeName = L"";
	nHitCount = 0;
	nLine = 0;

	/* 検索条件が長さゼロの場合はファイル名だけ返す */
	// 2002/08/29 行ループの前からここに移動
	if( 0 == nKeyLen ){
		WCHAR szCpName[100];
		if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
			// 2003.06.10 Moca コード判別処理をここに移動．
			// 判別エラーでもファイル数にカウントするため
			// ファイルの日本語コードセット判別
			// 2014.06.19 Moca ファイル名のタイプ別のm_encodingに変更
			CCodeMediator cmediator( type->m_encoding );
			nCharCode = cmediator.CheckKanjiCodeOfFile( pszFullPath );
			if( !IsValidCodeOrCPType(nCharCode) ){
				pszCodeName = L"  [(DetectError)]";
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
				pszFormatFullPath   = L"%s%s\r\n";
				pszFormatBasePath2  = L"■\"%s\"\r\n";
				pszFormatFilePath   = L"・\"%s\"%s\r\n";
				pszFormatFilePath2  = L"・\"%s\"%s\r\n";
			}else if( 2 == sGrepOption.nGrepOutputStyle ){
				/* WZ風 */
				pszFormatFullPath   = L"■\"%s\"%s\r\n";
				pszFormatBasePath2  = L"◎\"%s\"\r\n";
				pszFormatFilePath   = L"◆\"%s\"%s\r\n";
				pszFormatFilePath2  = L"■\"%s\"%s\r\n";
			}else if( 3 == sGrepOption.nGrepOutputStyle ){
				// 結果のみ
				pszFormatFullPath   = L"%s%s\r\n";
				pszFormatBasePath2  = L"■\"%s\"\r\n";
				pszFormatFilePath   = L"%s\r\n";
				pszFormatFilePath2  = L"%s\r\n";
			}
/*
			Base/Sep
			O / O  : (A)BaseFolder -> (C)Folder(Rel) -> (E)RelPath(File)
			O / X  : (B)BaseFolder ->                   (F)RelPath(RelFolder/File)
			X / O  :                  (D)Folder(Abs) -> (G)RelPath(File)
			X / X  : (H)FullPath
*/
			auto pszWork = std::make_unique<wchar_t[]>(std::wstring_view(pszFullPath).length() + std::wstring_view(pszCodeName).length() + 10);
			wchar_t* szWork0 = &pszWork[0];
			if( sGrepOption.bGrepOutputBaseFolder || sGrepOption.bGrepSeparateFolder ){
				if( !bOutputBaseFolder && sGrepOption.bGrepOutputBaseFolder ){
					const wchar_t* pszFormatBasePath =
						sGrepOption.bGrepSeparateFolder ? L"◎\"%s\"\r\n"	// (A)
						                                : pszFormatBasePath2;	// (B)
					auto_sprintf( szWork0, pszFormatBasePath, pszBaseFolder );
					cmemMessage.AppendString( szWork0 );
					bOutputBaseFolder = true;
				}
				if( !bOutputFolderName && sGrepOption.bGrepSeparateFolder ){
					// L"■\r\n" は %s を含まないため pszFolder は評価されるが使用されない（可変長引数の余剰は無視される）
					auto_sprintf( szWork0, (pszFolder[0] ? L"■\"%s\"\r\n" : L"■\r\n"), pszFolder );	// (C), (D)
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
		WCHAR szCpName[100];
		{
			if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
				if( IsValidCodeType(nCharCode) ){
					wcscpy_s( szCpName, CCodeTypeName(nCharCode).Bracket() );
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
		while( RESULT_FAILURE != cfl.ReadLine( &cUnicodeBuffer, &cEol ) )
		{
			const wchar_t*	pLine = cUnicodeBuffer.GetStringPtr();
			int		nLineLen = cUnicodeBuffer.GetStringLength();

			nEolCodeLen = cEol.GetLen();
			++nLine;

			/* 処理中のユーザー操作を可能にする */
			// 2010.08.31 間隔を1/32にする
			if( 0 == nLine % 32 && -1 == CheckCancelAndUpdateProgress(
					pcDlgCancel, cfl, pszFile, pszFolder, *pnHitCount, nOldPercent, m_dwTickUICheck ) ) {
				return -1;
			}
			const SGrepOutputPaths grepPaths{ pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszDispFilePath, pszCodeName };
			nHitCount += MatchAndEmitGrepLine(
				pLine, nLineLen, nLine, nEolCodeLen, nKeyLen,
				searchWords, sSearchOption, sGrepOption, pattern, pRegexp,
				grepPaths, bOutputBaseFolder, bOutputFolderName, bOutFileName,
				pnHitCount, cmemMessage
			);
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
	catch( const CError_FileOpen& ){
		CNativeW str(LS(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%s", pszFullPath);
		cmemMessage.AppendNativeData( str );
		return 0;
	}
	catch( const CError_FileRead& ){
		CNativeW str(LS(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%s", pszFullPath);
		cmemMessage.AppendNativeData( str );
	} // 例外処理終わり

	return nHitCount;
}

class CError_WriteFileOpen
{
public:
	virtual ~CError_WriteFileOpen() = default;
};

class CWriteData{
public:
	CWriteData(int& hit, LPCWSTR name_, ECodeType code_, bool bBom_, bool bOldSave_, CNativeW& message)
		:nHitCount(hit)
		,fileName(name_)
		,name(name_)
		,bBom(bBom_)
		,bOldSave(bOldSave_)
		,bufferSize(0)
		,pcCodeBase(CCodeFactory::CreateCodeBase(code_,0))
		,memMessage(message)
	{
		name += L".skrnew";
	}
	CWriteData(const CWriteData&) = delete;
	CWriteData& operator=(const CWriteData&) = delete;
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
			try{
				out = std::make_unique<CBinaryOutputStream>(name.c_str(), true);
			}catch( const CError_FileOpen& ){
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
			out.reset();
			if( bOldSave ){
				std::wstring oldFile = fileName;
				oldFile += L".skrold";
				if( fexist(oldFile.c_str()) ){
					if( FALSE == ::DeleteFile( oldFile.c_str() ) ){
						memMessage.AppendString( LS(STR_GREP_REP_ERR_DELETE) );
						memMessage.AppendStringF( L"[%s]\r\n", oldFile.c_str());
						return;
					}
				}
				if( FALSE == ::MoveFile( fileName, oldFile.c_str() ) ){
					memMessage.AppendString( LS(STR_GREP_REP_ERR_REPLACE) );
					memMessage.AppendStringF( L"[%s]\r\n", oldFile.c_str());
					return;
				}
			}else{
				if( FALSE == ::DeleteFile( fileName ) ){
					memMessage.AppendString( LS(STR_GREP_REP_ERR_DELETE) );
					memMessage.AppendStringF( L"[%s]\r\n", fileName );
					return;
				}
			}
			if( FALSE == ::MoveFile( name.c_str(), fileName ) ){
				memMessage.AppendString( LS(STR_GREP_REP_ERR_REPLACE) );
				memMessage.AppendStringF( L"[%s]\r\n", fileName );
				return;
			}
		}
		return;
	}
	~CWriteData()
	{
		if( out ){
			out->Close();
			out.reset();
			::DeleteFile( name.c_str() );
		}
	}
private:
	int& nHitCount;
	LPCWSTR fileName;
	std::wstring name;
	bool bBom;
	bool bOldSave;
	size_t bufferSize;
	std::deque<CNativeW> buffer;
	std::unique_ptr<CBinaryOutputStream> out;
	std::unique_ptr<CCodeBase> pcCodeBase;
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
	const WCHAR*			pszFile,
	const SSearchOption&	sSearchOption,
	const SGrepOption&		sGrepOption,
	const CSearchStringPattern& pattern,
	CBregexp*				pRegexp,		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	int*					pnHitCount,
	const WCHAR*			pszFullPath,
	const WCHAR*			pszBaseFolder,
	const WCHAR*			pszFolder,
	const WCHAR*			pszRelPath,
	bool&					bOutputBaseFolder,
	bool&					bOutputFolderName,
	CNativeW&				cmemMessage,
	CNativeW&				cUnicodeBuffer
)
{
	LONGLONG	nLine = 0;
	int		nHitCount = 0;
	ECodeType	nCharCode;
	BOOL	bOutFileName = FALSE;
	CEol	cEol;
	int		nEolCodeLen;
	int		nOldPercent = 0;
	auto nKeyLen = int(std::wstring_view(pszKey).length());
	const WCHAR*	pszCodeName = L"";

	const STypeConfigMini* type = nullptr;
	if( !CDocTypeManager().GetTypeConfigMini( CDocTypeManager().GetDocumentTypeOfPath( pszFile ), &type ) ){
		return -1;
	}
	CFileLoad	cfl( type->m_encoding );	// 2012/12/18 Uchi 検査するファイルのデフォルトの文字コードを取得する様に
	bool bBom = false;
	// ファイル名表示
	const WCHAR* pszDispFilePath = ( sGrepOption.bGrepSeparateFolder || sGrepOption.bGrepOutputBaseFolder ) ? pszRelPath : pszFullPath;

	try{
		// ファイルを開く
		// FileCloseで明示的に閉じるが、閉じていないときはデストラクタで閉じる
		// 2003.06.10 Moca 文字コード判定処理もFileOpenで行う
		nCharCode = cfl.FileOpen( pszFullPath, true, sGrepOption.nGrepCharSet, GetDllShareData().m_Common.m_sFile.GetAutoMIMEdecode(), &bBom );
		CWriteData output(nHitCount, pszFullPath, nCharCode, bBom, sGrepOption.bGrepBackup, cmemMessage );
		WCHAR szCpName[100];
		{
			if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
				if( IsValidCodeType(nCharCode) ){
					wcscpy_s( szCpName, CCodeTypeName(nCharCode).Bracket() );
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

		std::vector<std::pair<const wchar_t*, CLogicInt> > searchWords;
		if( sSearchOption.bWordOnly ){
			CSearchAgent::CreateWordList( searchWords, pszKey, nKeyLen );
		}

		CNativeW cOutBuffer;
		// 注意 : cfl.ReadLine が throw する可能性がある
		while( RESULT_FAILURE != cfl.ReadLine( &cUnicodeBuffer, &cEol ) )
		{
			const wchar_t*	pLine = cUnicodeBuffer.GetStringPtr();
			int		nLineLen = cUnicodeBuffer.GetStringLength();

			nEolCodeLen = cEol.GetLen();
			++nLine;

			DWORD dwNow = ::GetTickCount();
			if( dwNow - m_dwTickUICheck > UICHECK_INTERVAL_MILLISEC ){
				m_dwTickUICheck = dwNow;
				/* 処理中のユーザー操作を可能にする */
				if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
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
				if( const int nPercent = cfl.GetPercent();
					5000000 < cfl.GetFileSize() && 5 <= nPercent - nOldPercent ){
					nOldPercent = nPercent;
					WCHAR szWork[10];
					::auto_sprintf( szWork, L" (%3d%%)", nPercent );
					std::wstring str;
					str = str + pszFile + szWork;
					ApiWrap::DlgItem_SetText( pcDlgCancel->GetHwnd(), IDC_STATIC_CURFILE, str.c_str() );
				}
			}
			cOutBuffer.SetStringHoldBuffer( L"", 0 );	// P7: 確保済みバッファを保持したまま長さ 0 に戻す
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
							SGrepMatchInfo{ nLine, nIndex + 1, pLine, nLineLen, nEolCodeLen, pLine + nIndex, matchlen },
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
				while ((pszRes = CSearchAgent::SearchStringWord(pLine, nLineLen, nIdx, searchWords, sSearchOption.bLoHiCase, &nMatchLen))) {
					nIdx = int(pszRes - pLine + nMatchLen);
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
							SGrepMatchInfo{ nLine, int(pszRes - pLine + 1), pLine, nLineLen, nEolCodeLen, pszRes, nMatchLen },
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
					nOutputPos = int(pszRes - pLine + nMatchLen);
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

					auto nColumn = int(pszRes - pCompareData);
					if( bOutput ){
						OutputPathInfo(
							cmemMessage, sGrepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						/* Grep結果を、cmemMessageに格納する */
						SetGrepResult(
							cmemMessage, pszDispFilePath, pszCodeName,
							SGrepMatchInfo{ nLine, nColumn + nColumnPrev + 1, pLine, nLineLen, nEolCodeLen, pszRes, nKeyLen },
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
				AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
				cmemMessage._SetStringLength(0);
			}
		}

		// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
		cfl.FileClose();
		output.Close();
	} // try
	catch( const CError_FileOpen& ){
		CNativeW str(LS(STR_GREP_ERR_FILEOPEN));
		str.Replace(L"%s", pszFullPath);
		cmemMessage.AppendNativeData( str );
		return 0;
	}
	catch( const CError_FileRead& ){
		CNativeW str(LS(STR_GREP_ERR_FILEREAD));
		str.Replace(L"%s", pszFullPath);
		cmemMessage.AppendNativeData( str );
	}
	catch( const CError_WriteFileOpen& ){
		std::wstring file = pszFullPath;
		file += L".skrnew";
		CNativeW str(LS(STR_GREP_ERR_FILEWRITE));
		str.Replace(L"%s", file.c_str());
		cmemMessage.AppendNativeData( str );
	} // 例外処理終わり

	return nHitCount;
}

/*!	@brief Grep結果ヘッダの出力形式説明部を追記する（BuildGrepHeader の CC 削減用）
*/
static void AppendGrepHeaderOutputStyle( CNativeW& cmemMessage, const SSearchOption& sSearchOption, const SGrepOption& sGrepOption )
{
	const wchar_t* pszWork;
	if( sGrepOption.nGrepOutputLineType == 1 ){
		pszWork = LS( STR_GREP_SHOW_MATCH_LINE );
	}else if( sGrepOption.nGrepOutputLineType == 2 ){
		pszWork = LS( STR_GREP_SHOW_MATCH_NOHITLINE );
	}else{
		if( sGrepOption.bGrepReplace && sSearchOption.bRegularExp && !sGrepOption.bGrepPaste ){
			pszWork = LS(STR_GREP_SHOW_FIRST_LINE);
		}else{
			pszWork = LS( STR_GREP_SHOW_MATCH_AREA );
		}
	}
	cmemMessage.AppendString( pszWork );

	if( sGrepOption.bGrepOutputFileOnly ){
		pszWork = LS( STR_GREP_SHOW_FIRST_MATCH );
		cmemMessage.AppendString( pszWork );
	}
}

// 結果ヘッダ生成（"検索条件 ..."）
CNativeW CGrepAgent::BuildGrepHeader(
	const wchar_t* pszKey,
	const wchar_t* pszFile,
	const wchar_t* pszFolder,
	const SSearchOption& sSearchOption,
	const SGrepOption& sGrepOption,
	const wchar_t* pszReplace
)
{
	if( pszKey    == nullptr ) pszKey    = L"";
	if( pszFile   == nullptr ) pszFile   = L"";
	if( pszFolder == nullptr ) pszFolder = L"";

	CNativeW cmemMessage;
	auto nWork = (int)std::wstring_view(pszKey).length();

	cmemMessage.AppendString( LS( STR_GREP_SEARCH_CONDITION ) );
	if( 0 < nWork ){
		cmemMessage.AppendString( L"\"" );
		cmemMessage.AppendString( pszKey );
		cmemMessage.AppendString( L"\"\r\n" );
	}else{
		cmemMessage.AppendString( LS( STR_GREP_SEARCH_FILE ) );
	}

	if( sGrepOption.bGrepReplace ){
		cmemMessage.AppendString( LS( STR_GREP_REPLACE_TO ) );
		if( sGrepOption.bGrepPaste ){
			cmemMessage.AppendString( LS( STR_GREP_PASTE_CLIPBOAD ) );
		}else if( pszReplace != nullptr ){
			cmemMessage.AppendString( L"\"" );
			cmemMessage.AppendString( pszReplace );
			cmemMessage.AppendString( L"\"\r\n" );
		}
	}

	cmemMessage.AppendString( LS( STR_GREP_SEARCH_TARGET ) );
	cmemMessage.AppendString( pszFile );
	cmemMessage.AppendString( L"\r\n" );

	cmemMessage.AppendString( LS( STR_GREP_SEARCH_FOLDER ) );
	cmemMessage.AppendString( pszFolder );
	cmemMessage.AppendString( L"\r\n" );

	const wchar_t*	pszWork;
	if( sGrepOption.bGrepSubFolder ){
		pszWork = LS( STR_GREP_SUBFOLDER_YES );
	}else{
		pszWork = LS( STR_GREP_SUBFOLDER_NO );
	}
	cmemMessage.AppendString( pszWork );

	if( 0 < nWork ){
		if( sSearchOption.bWordOnly ){
			cmemMessage.AppendString( LS( STR_GREP_COMPLETE_WORD ) );
		}
		if( sSearchOption.bLoHiCase ){
			pszWork = LS( STR_GREP_CASE_SENSITIVE );
		}else{
			pszWork = LS( STR_GREP_IGNORE_CASE );
		}
		cmemMessage.AppendString( pszWork );

		if( sSearchOption.bRegularExp ){
			CBregexp cRegexp;
			cmemMessage.AppendString( LS( STR_GREP_REGEX_DLL ) );
			cmemMessage.AppendString( cRegexp.GetVersionW() );
			cmemMessage.AppendString( L")\r\n" );
		}
	}

	if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
		cmemMessage.AppendString( LS( STR_GREP_CHARSET_AUTODETECT ) );
	}else if(IsValidCodeOrCPType(sGrepOption.nGrepCharSet)){
		cmemMessage.AppendString( LS( STR_GREP_CHARSET ) );
		WCHAR szCpName[100];
		CCodePage::GetNameNormal(szCpName, sGrepOption.nGrepCharSet);
		cmemMessage.AppendString( szCpName );
		cmemMessage.AppendString( L")\r\n" );
	}

	if( 0 < nWork ){
		AppendGrepHeaderOutputStyle( cmemMessage, sSearchOption, sGrepOption );
	}

	cmemMessage.AppendString( L"\r\n\r\n" );
	return cmemMessage;
}

// 結果フッタ生成（"該当 N 件" / "N 件を置換"）
CNativeW CGrepAgent::BuildGrepFooter(int nHitCount, bool bGrepReplace)
{
	CNativeW cmemMessage;
	const WCHAR* pszFormat = bGrepReplace ? LS( STR_GREP_REPLACE_COUNT ) : LS( STR_GREP_MATCH_COUNT );

	// strprintf は必要長を動的確保するため固定バッファ長の検証は不要
	cmemMessage.SetString( strprintf( pszFormat, nHitCount ).c_str() );
	return cmemMessage;
}

#ifdef _DEBUG
// 0-5: 性能計測基盤（Debug 限定）
// 並列 Grep の 列挙時間 / 検索時間 / resultMutex ロック待ち / バッチ終端待ち を集計し、
// Grep 終了時に MYTRACE で出力する。Release ビルドでは一切のコードを生成しない。
struct SGrepPerfStats {
	std::atomic<long long> enumUs{ 0 };			// ファイル列挙時間（メインスレッド、μ秒）
	std::atomic<long long> searchUs{ 0 };		// ファイル内検索時間（全ワーカー合算、μ秒）
	std::atomic<long long> lockWaitUs{ 0 };		// resultMutex ロック待ち（全ワーカー合算、μ秒）
	std::atomic<long long> batchWaitUs{ 0 };	// バッチ終端待ち（メインスレッド、μ秒）
	void Trace() const {
		MYTRACE( L"[GrepPerf] enum=%lldus search=%lldus lockWait=%lldus batchWait=%lldus\n",
			enumUs.load(), searchUs.load(), lockWaitUs.load(), batchWaitUs.load() );
	}
};
static long long GrepPerfNowUs()
{
	using namespace std::chrono;
	return duration_cast<microseconds>( steady_clock::now().time_since_epoch() ).count();
}
#define GREP_PERF_BEGIN( var )			const long long var = GrepPerfNowUs()
#define GREP_PERF_ADD( field, var )		perfStats.field.fetch_add( GrepPerfNowUs() - (var) )
#define GREP_PERF_TRACE()				perfStats.Trace()
#else
#define GREP_PERF_BEGIN( var )			((void)0)
#define GREP_PERF_ADD( field, var )		((void)0)
#define GREP_PERF_TRACE()				((void)0)
#endif

/*!	@brief 並列Grep結果マージ時のフォルダーヘッダー出力（重複排除つき・最初のマッチ時のみ出力）
	@note resultMutex を保持した状態で呼び出すこと。
*/
static void AppendGrepFolderHeader(
	CNativeW&								sharedMessage,		//!< [i/o] 共有結果バッファ
	const SGrepOption&						sGrepOption,		//!< [in]  Grep オプション
	const std::wstring&						baseFolder,			//!< [in]  ベースフォルダー
	const std::wstring&						folder,				//!< [in]  サブフォルダー
	std::set<std::wstring, std::less<>>&	writtenBaseFolders,	//!< [i/o] 出力済みベースフォルダー
	std::set<std::wstring, std::less<>>&	writtenFolders		//!< [i/o] 出力済みフォルダー
)
{
	if( sGrepOption.bGrepOutputBaseFolder && !writtenBaseFolders.contains(baseFolder) ){
		writtenBaseFolders.insert( baseFolder );
		sharedMessage.AppendString( sGrepOption.bGrepSeparateFolder ? L"◎\"" : L"■\"" );
		sharedMessage.AppendString( baseFolder );
		sharedMessage.AppendString( L"\"\r\n" );
	}
	if( sGrepOption.bGrepSeparateFolder && !writtenFolders.contains(folder) ){
		writtenFolders.insert( folder );
		if( !folder.empty() ){
			sharedMessage.AppendString( L"■\"" );
			sharedMessage.AppendString( folder );
			sharedMessage.AppendString( L"\"\r\n" );
		}else{
			sharedMessage.AppendString( L"■\r\n" );
		}
	}
}

/*!	@brief 並列Grep: スレッドプールでファイル検索を並列化する
	@retval -1 キャンセル
	@retval 0 完了
*/
int CGrepAgent::RunParallelGrep(
	CEditView*							pcViewDst,		//!< [in]  出力先ビュー（nullptr 可）
	CDlgCancel*							pcDlgCancel,	//!< [in]  Cancelダイアログ（nullptr 可）
	const SGrepSearchParams&			searchParams,	//!< [in]  検索条件（キー/検索/Grep オプション）
	const SGrepEnumContext&				enumCtx,		//!< [in]  列挙・除外オブジェクト群
	const std::vector<std::wstring>&	vPaths,			//!< [in]  検索対象パス群
	CNativeW&							cmemMessage,	//!< [out] 結果メッセージ追記先
	int&								nHitCountOut	//!< [out] 合計ヒット数（キャンセル時も部分値）
)
{
	int nGrepTreeResult = 0;

	const std::wstring searchKey( searchParams.pszKey );
	const auto& sSearchOption = searchParams.sSearchOption;
	const auto& sGrepOption = searchParams.sGrepOption;
	auto& cGrepEnumKeys = enumCtx.keys;
	auto& cGrepExceptAbsFiles = enumCtx.exceptFiles;
	auto& cGrepExceptAbsFolders = enumCtx.exceptFolders;

	const std::vector<std::wstring>& regexPatterns = cGrepEnumKeys.m_vecExceptFileRegexPatterns;

	// スレッド数 = max(設定値, 論理コア数 / 4)  ※設定値はiniファイルの nGrepThreadCount（1〜8）
	const int nIniThreads = GetDllShareData().m_Common.m_sSearch.m_nGrepThreadCount;
	const auto nClampedIni = (unsigned int)std::max( 1, std::min( nIniThreads, 8 ) );
	const unsigned int nThreads = std::max<unsigned int>(
		nClampedIni,
		std::thread::hardware_concurrency() / 4 );

	// ヒット数（全バッチ共通・バッチ間で引き継ぐ）
	std::atomic atomicHitCount{ 0 };

#ifdef _DEBUG
	// 0-5: 性能計測（Debug 限定）
	SGrepPerfStats perfStats;
#endif

	// ===== スレッドプール: バッチ間でスレッドを再利用し生成・破棄コストを排除 =====
	// バッチ番号インクリメントで新バッチを通知し、condition_variable で待機中ワーカーを起床させる。
	std::mutex poolMutex;
	std::condition_variable cvPoolStart;
	std::condition_variable cvBatchDone;                // P2-1: バッチ終端通知（poolMutex で保護）
	size_t poolBatchId = 0;                             // バッチ番号（インクリメントで新バッチ通知）
	bool bPoolShutdown = false;                         // true: ワーカーに終了を指示
	const std::vector<SGrepFileTask>* pPoolBatch = nullptr; // 現在バッチのタスクリスト
	std::atomic<size_t> poolNextTask{ 0 };              // 次に処理するタスクのインデックス
	std::atomic poolBatchActive{ 0 };           // 現在バッチを処理中のワーカー数
	std::atomic bWorkCancelled{ false };        // true: キャンセル済み（バッチ間で維持）
	std::mutex resultMutex;
	CNativeW sharedMessage;
	std::set<std::wstring, std::less<>> writtenBaseFolders;
	std::set<std::wstring, std::less<>> writtenFolders;

	// スレッドプールのワーカーを生成（1回のみ）。
	// 各ワーカーはスレッドローカルの正規表現とバッファを持ち、毎回の再初期化を避ける。
	std::vector<std::jthread> poolWorkers;
	poolWorkers.reserve( nThreads );
	std::atomic nActiveWorkers{ nThreads };  // 初期化成功したワーカー数（デッドロック防止用）
	std::atomic<unsigned int> nInitDone{ 0 };             // 初期化完了ワーカー数（成功・失敗を含む、バリア用）
	std::mutex initMutex;
	std::condition_variable cvInitDone;

	struct PoolJoinGuard {
		std::vector<std::jthread>&	workers;
		std::mutex&					mtx;
		std::condition_variable&	cv;
		bool&						bShutdown;
		PoolJoinGuard( std::vector<std::jthread>& w, std::mutex& m, std::condition_variable& c, bool& s )
			: workers(w), mtx(m), cv(c), bShutdown(s) {}
		PoolJoinGuard(const PoolJoinGuard&) = delete;
		PoolJoinGuard& operator=(const PoolJoinGuard&) = delete;
		~PoolJoinGuard(){
			{
				std::scoped_lock lk( mtx );
				bShutdown = true;
			}
			cv.notify_all();
			for( auto& th : workers ){
				if( th.joinable() ) th.join();
			}
		}
	};
	PoolJoinGuard poolJoinGuard{ poolWorkers, poolMutex, cvPoolStart, bPoolShutdown };

	for( unsigned int t = 0; t < nThreads; t++ ){
		poolWorkers.emplace_back( [
#ifdef _DEBUG
			&perfStats,
#endif
			this, &searchKey, &sSearchOption, &sGrepOption, &regexPatterns,
			&nActiveWorkers, &nInitDone, &initMutex, &cvInitDone,
			&poolMutex, &cvPoolStart, &cvBatchDone, &bPoolShutdown,
			&poolBatchId, &pPoolBatch, &poolNextTask, &poolBatchActive,
			&bWorkCancelled, &resultMutex, &sharedMessage,
			&writtenBaseFolders, &writtenFolders, &atomicHitCount ](){
			size_t localBatchId = 0;

			// --- スレッドローカル初期化（スレッド生存中に1回のみ実行）---
			CBregexp localRegexp;
			CSearchStringPattern localPattern;
			try{
				localPattern.SetPattern( nullptr,
					searchKey.c_str(), searchKey.size(),
					sSearchOption, &localRegexp );
			}catch(...){
				// 初期化失敗: 実働ワーカー数を減らし、バリア通知後シャットダウンまで待機
				nActiveWorkers.fetch_sub( 1 );
				{
					std::scoped_lock lkInit( initMutex );
					nInitDone.fetch_add( 1 );
				}
				cvInitDone.notify_one();
				std::unique_lock lk( poolMutex );
				cvPoolStart.wait( lk, [&bPoolShutdown]{ return bPoolShutdown; } );
				return;
			}

			// 初期化成功: バリアへ通知
			{
				std::scoped_lock lkInit( initMutex );
				nInitDone.fetch_add( 1 );
			}
			cvInitDone.notify_one();

			std::vector<CBregexp> excludeRegexps;
			CompileExcludeRegexps( regexPatterns, excludeRegexps );

			CNativeW localMessage;
			CNativeW localUnicodeBuffer;
			localMessage.AllocStringBuffer( 4000 );
			localUnicodeBuffer.AllocStringBuffer( 4000 );

			// P1: 結果マージのロック競合削減
			// ヒットのたびに resultMutex を取得せず、ワーカーローカルに結果をため込み、
			// しきい値（32 ファイル分 または 約 64KB）に達した時点でまとめてマージする。
			// フォルダーヘッダー管理（writtenBaseFolders/writtenFolders）はマージ時にまとめて処理する。
			// 出力順序はもともとワーカー完了順で非決定的なため、本変更による順序仕様の変化はない。
			struct SPendingResult {
				const std::wstring*	pBaseFolder;	//!< バッチ内の SGrepFileTask を参照（バッチ生存中のみ有効）
				const std::wstring*	pFolder;
				CNativeW			message;
			};
			constexpr size_t kFlushFileCount = 32;
			constexpr size_t kFlushCharCount = 64 * 1024 / sizeof(wchar_t);
			std::vector<SPendingResult> pendingResults;
			pendingResults.reserve( kFlushFileCount );
			size_t nPendingChars = 0;
			DWORD dwLastFlushTick = ::GetTickCount();	// リアルタイム表示維持用の時間ベースフラッシュ判定

			auto FlushPendingResults = [&](){
				if( pendingResults.empty() ) return;
				GREP_PERF_BEGIN( perfLockStart );
				std::scoped_lock lk( resultMutex );
				GREP_PERF_ADD( lockWaitUs, perfLockStart );
				for( const auto& pending : pendingResults ){
					AppendGrepFolderHeader( sharedMessage, sGrepOption,
						*pending.pBaseFolder, *pending.pFolder,
						writtenBaseFolders, writtenFolders );
					sharedMessage.AppendNativeData( pending.message );
				}
				pendingResults.clear();
				nPendingChars = 0;
				dwLastFlushTick = ::GetTickCount();
			};

			// S134: ネスト削減のため、正規表現除外判定（フルパス全体に対してマッチング）をラムダ化
			auto IsExcludedPath = [&excludeRegexps]( const std::wstring& fullPath ){
				for( auto& reExcl : excludeRegexps ){
					if( reExcl.Match( fullPath.c_str(), (int)fullPath.size(), 0 ) ){
						return true;
					}
				}
				return false;
			};

			// しきい値到達、またはリアルタイム表示の遅延を押さえるため ADDTAIL_INTERVAL 経過でマージする
			// （ロック取得は最大でも 1000/ADDTAIL_INTERVAL 回/秒/ワーカーに抑制される）
			auto MaybeFlushPendingResults = [&](){
				if( pendingResults.size() >= kFlushFileCount || nPendingChars >= kFlushCharCount
					|| (::GetTickCount() - dwLastFlushTick) > ADDTAIL_INTERVAL_MILLISEC ){
					FlushPendingResults();
				}
			};

			// S134: ネスト削減のため、バッチ内タスク処理ループをラムダ化（同一スレッド内で同期実行）
			auto ProcessBatchTasks = [&]( const std::vector<SGrepFileTask>& batch ){
				while( true ){
					const size_t idx = poolNextTask.fetch_add( 1 );
					if( idx >= batch.size() || bWorkCancelled.load() ) break;

					const SGrepFileTask& task = batch[idx];

					if( IsExcludedPath( task.fullPath ) ) continue;

					// ファイル内検索
					localMessage._SetStringLength(0);
					const SGrepSearchParams workerParams{ searchKey.c_str(), sSearchOption, sGrepOption };
					GREP_PERF_BEGIN( perfSearchStart );
					const int fileHits = DoGrepFileWorker(
						workerParams, task,
						&localRegexp, localPattern,
						bWorkCancelled,
						localMessage, localUnicodeBuffer
					);
					GREP_PERF_ADD( searchUs, perfSearchStart );

					if( fileHits == -1 ){
						bWorkCancelled.store( true );
						continue;	// ループ先頭の判定で終了する
					}

					if( fileHits > 0 || localMessage.GetStringLength() > 0 ){
						// P1: ここではロックを取らずローカルに蓄積し、しきい値到達時にまとめてマージする
						auto& pending = pendingResults.emplace_back();
						pending.pBaseFolder = &task.baseFolder;
						pending.pFolder = &task.folder;
						pending.message.AppendNativeData( localMessage );
						nPendingChars += (size_t)localMessage.GetStringLength();
						atomicHitCount.fetch_add( fileHits );
						MaybeFlushPendingResults();
					}
				}
			};

			while( true ){
				// 新しいバッチまたはシャットダウンを待機
				const std::vector<SGrepFileTask>* pBatch = nullptr;
				{
					std::unique_lock lk( poolMutex );
					cvPoolStart.wait( lk, [&poolBatchId, &localBatchId, &bPoolShutdown]{
					return poolBatchId > localBatchId || bPoolShutdown;
				} );
					if( bPoolShutdown && poolBatchId <= localBatchId ) break;
					localBatchId = poolBatchId;
					pBatch = pPoolBatch;
				}

				// バッチ処理（try/catch で例外をキャンセル扱い）
				try{
					ProcessBatchTasks( *pBatch );
				}catch(...){
					// 予期せぬ例外（std::bad_alloc 等）をキャンセル扱いにする
					bWorkCancelled.store( true );
				}
				// P1: ローカルにため込んだ結果を残さずマージする（キャンセル・例外経路でも実行）
				try{
					FlushPendingResults();
				}catch(...){
					bWorkCancelled.store( true );
				}
				// try/catch どちらの経路でも必ず実行し、デッドロックを防止する
				// P2-1: ロストウェイクアップ防止のためデクリメントは poolMutex 内で行い、メインスレッドへ終端を通知する
				{
					std::scoped_lock lk( poolMutex );
					poolBatchActive.fetch_sub( 1 );
				}
				cvBatchDone.notify_all();
			}
		} );
	}

	// 全ワーカーの初期化完了を待ってからバッチ処理を開始（4-1: デッドロック防止）
	{
		std::unique_lock lk( initMutex );
		cvInitDone.wait( lk, [&nInitDone, &nThreads]{ return nInitDone.load() >= nThreads; } );
	}

	// 全ワーカーが初期化に失敗した場合、結果 0 件のまま無警告で終了せずエラーとして中断する
	if( nActiveWorkers.load() == 0 ){
		ErrorMessage( pcViewDst != nullptr ? pcViewDst->m_hwndParent : nullptr,
			L"Grep: 検索用ワーカースレッドの初期化にすべて失敗しました。検索を中断します。" );	// TODO: Phase 5 E-1 で LS() 化
		nHitCountOut = 0;
		return -1;
	}

	// バッチ実行ラムダ: vecTasks をプールのワーカーに割り当て結果を出力する
	// 戻り値: true=継続, false=キャンセル発生
	auto RunBatch = [&]( const std::vector<SGrepFileTask>& vecTasks ) {
		if( vecTasks.empty() ) return true;
		if( bWorkCancelled.load() ) return false;

		// バッチ設定（ロック下でバッチ番号をインクリメントしてワーカーを起床）
		{
			std::scoped_lock lk( poolMutex );
			pPoolBatch = &vecTasks;
			poolNextTask.store( 0 );
			poolBatchActive.store( (int)nActiveWorkers.load() );
			++poolBatchId;
		}
		cvPoolStart.notify_all();

		// メインスレッド: 全ワーカーが完全終了するまでUI更新・キャンセル監視・結果フラッシュを継続
		// P2-1: Sleep(5) ポーリングを廃止し、バッチ終端は cvBatchDone で即時通知を受ける。
		// UI ポンプ（BlockingHook/進捗更新）とリアルタイム表示のフラッシュが必要なため完全ブロックにはせず、
		// ADDTAIL_INTERVAL_MILLISEC のタイムアウト付き待機で定期的にループを回す（従来の表示更新間隔を維持）。
		GREP_PERF_BEGIN( perfBatchStart );
		while( true ){
			{
				std::unique_lock lk( poolMutex );
				if( cvBatchDone.wait_for( lk, std::chrono::milliseconds( ADDTAIL_INTERVAL_MILLISEC ),
						[&poolBatchActive]{ return poolBatchActive.load() <= 0; } ) ){
					break;	// 全ワーカーがバッチを抜けた
				}
			}

			if( DWORD dwNow = ::GetTickCount(); pcDlgCancel != nullptr && dwNow - m_dwTickUICheck > UICHECK_INTERVAL_MILLISEC ){
				m_dwTickUICheck = dwNow;
				if( !::BlockingHook( pcDlgCancel->GetHwnd() ) ){
					bWorkCancelled.store( true );
				}
				if( pcDlgCancel->IsCanceled() ){
					bWorkCancelled.store( true );
				}
				CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
					0 != ::IsDlgButtonChecked( pcDlgCancel->GetHwnd(), IDC_CHECK_REALTIMEVIEW ) );
				::SetDlgItemInt( pcDlgCancel->GetHwnd(), IDC_STATIC_HITCOUNT,
				                 atomicHitCount.load(), FALSE );
			}

			// 共有バッファをリアルタイム出力にフラッシュ
			{
				std::scoped_lock lk( resultMutex );
				if( sharedMessage.GetStringLength() > 0 ){
					cmemMessage.AppendNativeData( sharedMessage );
					sharedMessage._SetStringLength(0);
				}
			}
			if( pcViewDst != nullptr &&
			    0 < cmemMessage.GetStringLength() &&
			    (::GetTickCount() - m_dwTickAddTail) > ADDTAIL_INTERVAL_MILLISEC ){
				AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
				cmemMessage._SetStringLength(0);
			}
		}
		GREP_PERF_ADD( batchWaitUs, perfBatchStart );

		// 最終フラッシュ
		{
			std::scoped_lock lk( resultMutex );
			if( sharedMessage.GetStringLength() > 0 ){
				cmemMessage.AppendNativeData( sharedMessage );
				sharedMessage._SetStringLength(0);
			}
		}
		if( pcViewDst != nullptr && 0 < cmemMessage.GetStringLength() ){
			AddTail( pcViewDst, cmemMessage, sGrepOption.bGrepStdout );
			cmemMessage._SetStringLength(0);
		}

		return !bWorkCancelled.load();
	};

	// 各検索パスをサブフォルダー単位でバッチ処理
	for( int nPath = 0; nPath < (int)vPaths.size() && nGrepTreeResult != -1; nPath++ ){
		std::wstring sPath = ChopYen( vPaths[nPath] );

		// 検索パスが変わるたびにフォルダーヘッダー出力履歴をリセット
		{
			std::scoped_lock lk( resultMutex );
			writtenBaseFolders.clear();
			writtenFolders.clear();
		}

		// バッチ0: 検索パス直下のファイル（サブフォルダーは含まない）
		{
			SGrepOption sGrepOptionNoSub = sGrepOption;
			sGrepOptionNoSub.bGrepSubFolder = false;
			bool bEnumCancelled = false;
			std::vector<SGrepFileTask> vecTasks;
			GREP_PERF_BEGIN( perfEnumStart );
			DoGrepTreeEnumerate(
				pcDlgCancel, sGrepOptionNoSub,
				SGrepEnumContext{ cGrepEnumKeys, cGrepExceptAbsFiles, cGrepExceptAbsFolders },
				sPath.c_str(), sPath.c_str(),
				vecTasks, bEnumCancelled
			);
			GREP_PERF_ADD( enumUs, perfEnumStart );
			if( bEnumCancelled || !RunBatch(vecTasks) ){
				nGrepTreeResult = -1;	// 後続の nGrepTreeResult == -1 判定とループ条件で終了する
			}
		}

		// バッチ1..N: 直下サブフォルダーをそれぞれ独立バッチで列挙・検索・解放
		if( !sGrepOption.bGrepSubFolder || nGrepTreeResult == -1 ){
			continue;
		}
		CGrepEnumOptions cGrepEnumOptionsDir;
		CGrepEnumFilterFolders cTopFolders;
		GREP_PERF_BEGIN( perfEnumDirStart );
		cTopFolders.Enumerates(
			sPath.c_str(), cGrepEnumKeys, cGrepEnumOptionsDir, cGrepExceptAbsFolders );
		GREP_PERF_ADD( enumUs, perfEnumDirStart );

		const int nFolderCount = cTopFolders.GetCount();
		for( int fi = 0; fi < nFolderCount && nGrepTreeResult != -1; fi++ ){
			std::wstring subFolder = sPath + L"\\" + cTopFolders.GetFileName(fi);
			bool bEnumCancelled = false;
			std::vector<SGrepFileTask> vecTasks;
			GREP_PERF_BEGIN( perfEnumSubStart );
			DoGrepTreeEnumerate(
				pcDlgCancel, sGrepOption,
				SGrepEnumContext{ cGrepEnumKeys, cGrepExceptAbsFiles, cGrepExceptAbsFolders },
				subFolder.c_str(), sPath.c_str(),
				vecTasks, bEnumCancelled
			);
			GREP_PERF_ADD( enumUs, perfEnumSubStart );
			if( bEnumCancelled || !RunBatch(vecTasks) ){
				nGrepTreeResult = -1;	// ループ条件 nGrepTreeResult != -1 で終了する
			}
		}
	}

	// スレッドプールのシャットダウンと join は PoolJoinGuard のデストラクタが
	// 担う（正常終了・例外伝播のどちらの経路でも確実に実行される）。
	nHitCountOut = atomicHitCount.load();
	GREP_PERF_TRACE();
	return nGrepTreeResult;
}
