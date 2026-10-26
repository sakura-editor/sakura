/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "agent/CGrepAgent.h"
#include "basis/GrepInfo.h"
#include "grep/CGrepEnumKeys.h"
#include "grep/CGrepEnumFilterFiles.h"
#include "grep/CGrepExceptFileRegexps.h"
#include "grep/CGrepEnumFilterFolders.h"
#include "agent/CSearchAgent.h"
#include "dlg/CDlgCancel.h"
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
#include "grep/GrepMessageFormat.h"
#include "debug/CRunningTimer.h"
#include "apiwrap/StdApi.h"
#include "apiwrap/StdControl.h"
#include "CSelectLang.h"
#include "sakura_rc.h"
#include "config/system_constants.h"

#include <filesystem>

#define UICHECK_INTERVAL_MILLISEC 100	// UI確認の時間間隔
#define ADDTAIL_INTERVAL_MILLISEC 50	// 結果出力の時間間隔
#define UIFILENAME_INTERVAL_MILLISEC 15	// Cancelダイアログのファイル名表示更新間隔

namespace grep {

/*!
	@brief 「Grep実行中」のダイアログ。中断を受け付けて進み具合を表示する

	DoModeless() で出し、初期表示は OnInitDialog() で行う。
*/
class CDlgGrepRunning : public CDlgCancel, public IGrepProgress {
public:
	bool IsCheckTime() override
	{
		return IsIntervalPassed( m_nextCheckTime, UICHECK_INTERVAL_MILLISEC );
	}

	bool IsFileNameTime() override
	{
		return IsIntervalPassed( m_nextFileNameTime, UIFILENAME_INTERVAL_MILLISEC );
	}

	bool IsCancelRequested() override
	{
		/* 処理中のユーザー操作を可能にする */
		/* 中断ボタン押下チェック */
		return !::BlockingHook( GetHwnd() ) || IsCanceled();
	}

	void ApplyRealTimeView() override
	{
		CEditWnd::getInstance()->SetDrawSwitchOfAllViews(
			0 != ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_REALTIMEVIEW )
		);
	}

	void SetHitCount( int nHitCount ) override
	{
		ApiWrap::DlgItem_SetText( GetHwnd(), IDC_STATIC_HITCOUNT, std::to_wstring( nHitCount ).c_str() );
	}

	void SetFileName( std::wstring_view fileName ) override
	{
		ApiWrap::DlgItem_SetText( GetHwnd(), IDC_STATIC_CURFILE, std::wstring( fileName ).c_str() );
	}

	void SetFolder( std::wstring_view folder ) override
	{
		ApiWrap::DlgItem_SetText( GetHwnd(), IDC_STATIC_CURPATH, std::wstring( folder ).c_str() );
	}

protected:
	BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam ) override
	{
		const BOOL bRet = CDlgCancel::OnInitDialog( hwndDlg, wParam, lParam );
		SetHitCount( 0 );
		SetFileName( L" " );	// 2002/09/09 Moca add
		::CheckDlgButton( GetHwnd(), IDC_CHECK_REALTIMEVIEW, GetDllShareData().m_Common.m_sSearch.m_bGrepRealTimeView );	// 2003.06.23 Moca
		return bRet;
	}

private:
	//! 前回の時機から nInterval ミリ秒を過ぎたかを返す(過ぎていれば、次の時機を今から nInterval ミリ秒後にする)
	static bool IsIntervalPassed( ULONGLONG& nNextTime, ULONGLONG nInterval )
	{
		const ULONGLONG nNow = ::GetTickCount64();
		if( nNow <= nNextTime ){
			return false;
		}
		nNextTime = nNow + nInterval;
		return true;
	}

	ULONGLONG	m_nextCheckTime = 0;		//!< この時刻を過ぎたら、処理中にユーザーによるUI操作が行われていないか確認する
	ULONGLONG	m_nextFileNameTime = 0;		//!< この時刻を過ぎたら、Cancelダイアログのファイル名表示を更新する
};

/*!
	@brief Grep の結果を、結果の文書に足す(-GOPT=U なら、文書の文字コードで標準出力に書く)
*/
class CGrepOutputToView : public IGrepOutputTarget {
public:
	CGrepOutputToView( CEditView* pcEditView, bool bStdout )
		: m_pcEditView( pcEditView )
		, m_bStdout( bStdout )
	{
	}

	void Append( const CNativeW& cmem ) override
	{
		if( m_bStdout ){
			HANDLE out = ::GetStdHandle(STD_OUTPUT_HANDLE);
			if( out && out != INVALID_HANDLE_VALUE ){
				CMemory cmemOut;
				std::unique_ptr<CCodeBase> pcCodeBase( CCodeFactory::CreateCodeBase(
						m_pcEditView->GetDocument()->GetDocumentEncoding(), 0) );
				pcCodeBase->UnicodeToCode( cmem, &cmemOut );
				DWORD dwWrite = 0;
				::WriteFile(out, cmemOut.GetRawPtr(), cmemOut.GetRawLength(), &dwWrite, nullptr);
			}
		}else{
			m_pcEditView->GetCommander().Command_ADDTAIL( cmem.GetStringPtr(), cmem.GetStringLength() );
			m_pcEditView->GetCommander().Command_GOFILEEND( false );
			if( !CEditWnd::getInstance()->UpdateTextWrap() ){	// 折り返し方法関連の更新
				CEditWnd::getInstance()->RedrawAllViews( m_pcEditView );	//	他のペインの表示を更新
			}
		}
	}

	bool IsDrawing() const override
	{
		return m_pcEditView->GetDrawSwitch();
	}

private:
	CEditView*	m_pcEditView;	//!< 結果の文書のビュー
	bool		m_bStdout;		//!< 標準出力に書くか(-GOPT=U)
};

//! 大きいファイルの読んだ割合を、前回の表示から 5% 以上進んだら表示する
void ShowReadPercent( IGrepProgress* pProgress, const wchar_t* pszFile, int nPercent, int& nOldPercent )
{
	if( nPercent - nOldPercent < 5 ){
		return;
	}
	nOldPercent = nPercent;
	pProgress->SetFileName( std::format( L"{} ({:3d}%)", pszFile, nPercent ) );
}

/*!
	@brief 処理中のユーザー操作を受け付け、中断されたかを返す

	確かめる時機(IGrepProgress::IsCheckTime())でなければ何もしない。
	時機なら、メッセージを処理し、中断ボタンとリアルタイム表示のチェックボックスを確かめる。

	@param[in]	pProgress	中断の確認と進み具合の表示
	@retval true	中断する
*/
bool IsGrepCanceled( IGrepProgress* pProgress )
{
	if( !pProgress->IsCheckTime() ){
		return false;
	}
	/* 処理中のユーザー操作を可能にする */
	if( pProgress->IsCancelRequested() ){
		return true;
	}
	/* 表示設定をチェック */
	pProgress->ApplyRealTimeView();
	return false;
}

} // namespace grep

/*!
 * 指定された文字列をタイプ別設定に従ってエスケープする
 */
inline CNativeW EscapeStringLiteral( const STypeConfig& type, const CNativeW& cmemString )
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
template<class ContainerType>
std::wstring FormatPathList( const ContainerType& containter )
{
	std::wstring strPatterns;
	bool firstItem = true;
	for( const auto& pattern : containter ){
		// パスリストは ':' で区切る(2つ目以降の前に付加する)
		if( firstItem ){
			firstItem = false;
		}else {
			strPatterns += L';';
		}

		// ';' を含むパス名は引用符で囲む
		if( std::wstring::npos != std::wstring_view( pattern ).find( L';' ) ){
			strPatterns += L'"';
			strPatterns += pattern;
			strPatterns += L'"';
		}else{
			strPatterns += pattern;
		}
	}
	return strPatterns;
}

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

CGrepAgent::CGrepAgent()
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

/*!
	@brief 結果を出力先に足す

	足す直前の時刻を覚える(途中の結果を足す間隔を測るのに使う)。
*/
void CGrepAgent::AppendResult( IGrepOutputTarget* pOutput, const CNativeW& cmem )
{
	m_lastAddTailTime = ::GetTickCount64();
	pOutput->Append( cmem );
}

/*!
	検索対象のフォルダーがすべて存在するフォルダーか

	末尾の \ やルート(C:\)を含むパスでも判定できるよう、FindFirstFile 系の IsDirectory() ではなく
	std::filesystem::is_directory() を使う。フォルダーが空のリストは true(従来どおり何もしない)。
*/
static bool AllFoldersExist( const std::vector<std::wstring>& vPaths )
{
	return std::ranges::all_of( vPaths, []( const std::wstring& path ) {
		std::error_code ec;
		return std::filesystem::is_directory( path, ec );
	} );
}

int GetHwndTitle(HWND& hWndTarget, CNativeW* pmemTitle, WCHAR* pszWindowName, WCHAR* pszWindowPath, const WCHAR* pszFile)
{
	hWndTarget = nullptr;	//out引数をクリアする

	constexpr auto& szTargetPrefix = L":HWND:";
	constexpr auto cchTargetPrefix = int(std::size(szTargetPrefix)) - 1;
	if( 0 != wcsncmp(pszFile, szTargetPrefix, cchTargetPrefix) ){
		return 0; // ハンドルGrepではない
	}
	if( 0 >= ::swscanf_s(pszFile + cchTargetPrefix, L"%zx", (size_t*)&hWndTarget) || !IsSakuraMainWindow(hWndTarget) ){
		return -1; // ハンドルを読み取れなかった、または、対象ウインドウハンドルが存在しない
	}
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


/*! Grep実行

  @param[in] pcViewDst Grep結果の文書のビュー。出力先を指定しても、アンドゥ・カーソル・描画はこのビューで行う
  @param[in] gi Grep実行の入力一式。検索パターン・検索対象ファイルパターン(!で除外指定)・
                検索対象フォルダー・検索オプション・出力オプションを含む

  @date 2008.12.07 nasukoji	ファイル名パターンのバッファオーバラン対策
  @date 2008.12.13 genta 検索パターンのバッファオーバラン対策
  @date 2012.10.13 novice 検索オプションをクラスごと代入

  @param[in] optProgress 中断の確認と進み具合の表示。指定が無いときは「Grep実行中」のダイアログを出す。nullptr は渡さない
  @param[in] optOutput 結果の出力先。指定が無いときは pcViewDst の文書(-GOPT=U なら標準出力)に足す。nullptr は渡さない。
                       存在しないフォルダーのメッセージは、-GOPT=U のときだけ出力先に足す(それ以外はメッセージボックス)
*/
DWORD CGrepAgent::DoGrep(
	CEditView*				pcViewDst,	//!< [in] Grep結果の文書のビュー
	const GrepInfo&			gi,			//!< [in] Grep実行の入力一式(検索キー・対象・オプション)
	const std::optional<IGrepProgress*>&	optProgress,	//!< [in] 中断の確認と進み具合の表示
	const std::optional<IGrepOutputTarget*>&	optOutput	//!< [in] 結果の出力先
)
{
	MY_RUNNINGTIMER( cRunningTimer, L"CEditView::DoGrep" );

	// 再入不可
	if( this->m_bGrepRunning ){
		// かつてはDWORD(-1)を返していた。
		// ・エディタの正常動作中にDoGrepを呼ぶルートはないので例外を投げる動作に変更した。
		// ・例外の種類、メッセージについては今後精査する余地あり。
		// 　・std::runtime_error派生であればなんでもよいと思う。
		// 　・表示機会がないのでメッセージをローカライズする必要はないと思う。
		throw std::domain_error("The DoGrep method is not reentrant");
	}

	this->m_bGrepRunning = true;

	int			nHitCount = 0;
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
	pcViewDst->m_strCurSearchKey = gi.cmGrepKey.GetStringPtr();				/* 検索文字列 */
	pcViewDst->m_sCurSearchOption = gi.sGrepSearchOption;						// 検索オプション
	pcViewDst->m_nCurSearchKeySequence = GetDllShareData().m_Common.m_sSearch.m_nSearchKeySequence;

	// 置換後文字列の準備
	CNativeW cmemReplace;
	if( gi.bGrepReplace ){
		if( gi.bGrepPaste ){
			// 矩形・ラインモード貼り付けは未サポート
			bool bColmnSelect;
			bool bLineSelect = false;
			if( !pcViewDst->MyGetClipboardData( cmemReplace, &bColmnSelect, GetDllShareData().m_Common.m_sEdit.m_bEnableLineModePaste? &bLineSelect: nullptr ) ){
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
				auto nConvertedTextLen = pcViewDst->m_cCommander.ConvertEol(cmemReplace.GetStringPtr(), len, pszConvertedText);
				cmemReplace.SetString(pszConvertedText, nConvertedTextLen);
				delete [] pszConvertedText;
			}
		}else{
			cmemReplace = gi.cmGrepRep;
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
	if (!pcViewDst->RefreshSearchPattern())
	{
		this->m_bGrepRunning = false;
		pcViewDst->m_bDoing_UndoRedo = false;
		pcViewDst->SetUndoBuffer();
		return 0;
	}

	//2014.06.13 別ウィンドウで検索したとき用にGrepダイアログの検索キーを設定
	GetEditWnd().m_cDlgGrep.m_strText = gi.cmGrepKey.GetStringPtr();
	GetEditWnd().m_cDlgGrep.m_bSetText = true;
	GetEditWnd().m_cDlgGrepReplace.m_strText = gi.cmGrepKey.GetStringPtr();
	if( gi.bGrepReplace ){
		GetEditWnd().m_cDlgGrepReplace.m_strText2 = gi.cmGrepRep.GetStringPtr();
	}
	GetEditWnd().m_cDlgGrepReplace.m_bSetText = true;

	// 中断の確認と進み具合の表示。指定が無ければ「Grep実行中」のダイアログを出す
	using CDlgGrepRunningHolder = std::unique_ptr<grep::CDlgGrepRunning>;
	CDlgGrepRunningHolder pcDlgRunning{};
	if( !optProgress.has_value() ){
		pcDlgRunning = std::make_unique<grep::CDlgGrepRunning>();
		if( nullptr == pcDlgRunning->DoModeless( G_AppInstance(), pcViewDst->m_hwndParent, IDD_GREPRUNNING ) ){
			// ダイアログを出せなければ、検索せずに終える
			this->m_bGrepRunning = false;
			pcViewDst->m_bDoing_UndoRedo = false;
			pcViewDst->SetUndoBuffer();
			return 0;
		}
	}
	auto pcProgress = optProgress.value_or( pcDlgRunning.get() );

	//	2008.12.13 genta パターンが長すぎる場合は登録しない
	//	(正規表現が途中で途切れると困るので)
	//	2011.12.10 Moca 表示の際に...に切り捨てられるので登録するように
	wcsncpy_s( CAppMode::getInstance()->m_szGrepKey, int(std::size(CAppMode::getInstance()->m_szGrepKey)), gi.cmGrepKey.GetStringPtr(), _TRUNCATE );
	this->m_bGrepMode = true;

	//	2007.07.22 genta
	//	バージョン番号取得のため，処理を前の方へ移動した
	CSearchStringPattern pattern;
	{
		/* 検索パターンのコンパイル */
		bool bError;
		if( gi.bGrepReplace && !gi.bGrepPaste ){
			// Grep置換
			// 2015.03.03 Grep置換がoptGlobalじゃないバグを修正
			bError = !pattern.SetPattern(pcViewDst->GetHwnd(), gi.cmGrepKey.GetStringPtr(), gi.cmGrepKey.GetStringLength(),
				cmemReplace.GetStringPtr(), gi.sGrepSearchOption, &cRegexp, true);
		}else{
			bError = !pattern.SetPattern(pcViewDst->GetHwnd(), gi.cmGrepKey.GetStringPtr(), gi.cmGrepKey.GetStringLength(),
				gi.sGrepSearchOption, &cRegexp);
		}
		if( bError ){
			this->m_bGrepRunning = false;
			pcViewDst->m_bDoing_UndoRedo = false;
			pcViewDst->SetUndoBuffer();
			return 0;
		}
	}
	
	// Grepオプションまとめ
	const GrepInfo sGrepOption = gi.Normalized();

	// 結果の出力先。指定が無ければ結果の文書(-GOPT=U なら標準出力)に足す
	using CGrepOutputToViewHolder = std::unique_ptr<grep::CGrepOutputToView>;
	CGrepOutputToViewHolder pcViewOutput{};
	if( !optOutput.has_value() ){
		pcViewOutput = std::make_unique<grep::CGrepOutputToView>( pcViewDst, sGrepOption.bGrepStdout );
	}
	auto pcOutput = optOutput.value_or( pcViewOutput.get() );

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

	CGrepExceptFileRegexps cExceptFileRegexps;	// 除外ファイル(正規表現)。cGrepEnumKeys の照合関数が参照するので先に宣言する
	CGrepEnumKeys cGrepEnumKeys;
	{
		int nErrorNo = cGrepEnumKeys.SetFileKeys( gi.cmGrepFile.GetStringPtr(), gi.bGrepExceptFileRegexp );
		if( nErrorNo == 0 && !cExceptFileRegexps.Attach( cGrepEnumKeys, GetDllShareData().m_Common.m_sSearch.m_szRegexpLib ) ){
			nErrorNo = 3;
		}
		if( nErrorNo != 0 ){
			this->m_bGrepRunning = false;
			pcViewDst->m_bDoing_UndoRedo = false;
			pcViewDst->SetUndoBuffer();

			const WCHAR* pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS0);
			if( nErrorNo == 1 ){
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS1);
			}
			else if( nErrorNo == 2 ){
				pszErrorMessage = LS(STR_GREP_ERR_ENUMKEYS2);
			}
			else if( nErrorNo == 3 ){
				pszErrorMessage = cExceptFileRegexps.GetErrorMessage().c_str();
			}
			ErrorMessage( pcViewDst->m_hwndParent, L"%s", pszErrorMessage );
			return 0;
		}
	}

	// 出力対象ビューのタイプ別設定(grepout固定)
	const STypeConfig& type = pcViewDst->m_pcEditDoc->m_cDocType.GetDocumentAttribute();

	std::vector<std::wstring> vPaths;
	CreateFolders( gi.cmGrepFolder.GetStringPtr(), vPaths );

	// 存在しないフォルダーが含まれるときは、検索・置換を始める前に止める(Issue #2707)
	if( !AllFoldersExist( vPaths ) ){
		this->m_bGrepRunning = false;
		pcViewDst->m_bDoing_UndoRedo = false;
		pcViewDst->SetUndoBuffer();

		const std::wstring strMessage = LS( STR_DLGGREP5 );
		if( sGrepOption.bGrepStdout ){
			// 標準出力のときは、メッセージボックスで止めない(バッチが止まる)
			CNativeW cmemError;
			const std::wstring strLine = strMessage + L"\r\n";
			cmemError.SetString( strLine.c_str(), strLine.length() );
			AppendResult( pcOutput, cmemError );
		}else{
			ErrorMessage( pcViewDst->m_hwndParent, L"%s", strMessage.c_str() );
		}
		return 0;
	}

	nWork = gi.cmGrepKey.GetStringLength(); // 2003.06.10 Moca あらかじめ長さを計算しておく

	/* 最後にテキストを追加 */
	CNativeW	cmemWork;
	cmemMessage.AppendString( LS( STR_GREP_SEARCH_CONDITION ) );	//L"\r\n□検索条件  "
	if( 0 < nWork ){
		cmemMessage.AppendString( L"\"" );
		cmemMessage += EscapeStringLiteral(type, gi.cmGrepKey);
		cmemMessage.AppendString( L"\"\r\n" );
	}else{
		cmemMessage.AppendString( LS( STR_GREP_SEARCH_FILE ) );	//L"「ファイル検索」\r\n"
	}

	if( gi.bGrepReplace ){
		cmemMessage.AppendString( LS(STR_GREP_REPLACE_TO) );
		if( gi.bGrepPaste ){
			cmemMessage.AppendString( LS(STR_GREP_PASTE_CLIPBOAD) );
		}else{
			cmemMessage.AppendString( L"\"" );
			cmemMessage += EscapeStringLiteral(type, cmemReplace);
			cmemMessage.AppendString( L"\"\r\n" );
		}
	}

	HWND hWndTarget = nullptr;
	WCHAR szWindowName[_MAX_PATH];
	WCHAR szWindowPath[_MAX_PATH];
	{
		int nHwndRet = GetHwndTitle(hWndTarget, &cmemWork, szWindowName, szWindowPath, gi.cmGrepFile.GetStringPtr());
		if( -1 == nHwndRet ){
			cmemMessage.AppendString(L"HWND handle error.\n");
			if( sGrepOption.bGrepHeader ){
				AppendResult( pcOutput, cmemMessage );
			}
			return 0;
		}else if( 0 == nHwndRet ){
			{
				// 解析済みのファイルパターン配列を取得する
				const auto& vecSearchFileKeys = cGrepEnumKeys.m_vecSearchFileKeys;
				std::wstring strPatterns = FormatPathList( vecSearchFileKeys );
				cmemWork.SetString( strPatterns.c_str(), strPatterns.length() );
			}
		}
	}
	cmemMessage.AppendString( LS( STR_GREP_SEARCH_TARGET ) );	//L"検索対象   "
	cmemMessage += cmemWork;
	cmemMessage.AppendString( L"\r\n" );

	cmemMessage.AppendString( LS( STR_GREP_SEARCH_FOLDER ) );	//L"フォルダー   "
	{
		// フォルダーリストから末尾のバックスラッシュを削ったパスリストを作る
		std::list<std::wstring> folders;
		std::transform( vPaths.cbegin(), vPaths.cend(), std::back_inserter( folders ), []( const auto& path ) { return ChopYen( path ); } );
		std::wstring strPatterns = FormatPathList( folders );
		cmemMessage.AppendString( strPatterns.c_str(), strPatterns.length() );
	}
	cmemMessage.AppendString( L"\r\n" );

	cmemMessage.AppendString(LS(STR_GREP_EXCLUDE_FILE));	//L"除外ファイル   "
	{
		// 除外ファイルの解析済みリストを取得る
		auto excludeFiles = cGrepEnumKeys.GetExcludeFiles();
		std::wstring strPatterns = FormatPathList( excludeFiles );
		cmemMessage.AppendString( strPatterns.c_str(), strPatterns.length() );
	}
	cmemMessage.AppendString(L"\r\n");

	cmemMessage.AppendString(LS(STR_GREP_EXCLUDE_FOLDER));	//L"除外フォルダー   "
	{
		// 除外フォルダーの解析済みリストを取得する
		auto excludeFolders = cGrepEnumKeys.GetExcludeFolders();
		std::wstring strPatterns = FormatPathList( excludeFolders );
		cmemMessage.AppendString( strPatterns.c_str(), strPatterns.length() );
	}
	cmemMessage.AppendString(L"\r\n");

	const wchar_t*	pszWork;
	if( sGrepOption.bGrepSubFolder ){
		pszWork = LS( STR_GREP_SUBFOLDER_YES );	//L"    (サブフォルダーも検索)\r\n"
	}else{
		pszWork = LS( STR_GREP_SUBFOLDER_NO );	//L"    (サブフォルダーを検索しない)\r\n"
	}
	cmemMessage.AppendString( pszWork );

	if( sGrepOption.bGrepExceptFileRegexp ){
		cmemMessage.AppendString( LS( STR_GREP_EXCLUDE_FILE_REGEXP ) );	//L"    (除外ファイルは正規表現)\r\n"
	}

	if( 0 < nWork ){ // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if( gi.sGrepSearchOption.bWordOnly ){
		/* 単語単位で探す */
			cmemMessage.AppendString( LS( STR_GREP_COMPLETE_WORD ) );	//L"    (単語単位で探す)\r\n"
		}

		if( gi.sGrepSearchOption.bLoHiCase ){
			pszWork = LS( STR_GREP_CASE_SENSITIVE );	//L"    (英大文字小文字を区別する)\r\n"
		}else{
			pszWork = LS( STR_GREP_IGNORE_CASE );	//L"    (英大文字小文字を区別しない)\r\n"
		}
		cmemMessage.AppendString( pszWork );

		if( gi.sGrepSearchOption.bRegularExp ){
			//	2007.07.22 genta : 正規表現ライブラリのバージョンも出力する
			cmemMessage.AppendString( LS( STR_GREP_REGEX_DLL ) );	//L"    (正規表現:"
			cmemMessage.AppendString( cRegexp.GetVersionW() );
			cmemMessage.AppendString( L")\r\n" );
		}
	}

	if( CODE_AUTODETECT == sGrepOption.nGrepCharSet ){
		cmemMessage.AppendString( LS( STR_GREP_CHARSET_AUTODETECT ) );	//L"    (文字コードセットの自動判別)\r\n"
	}else if(IsValidCodeOrCPType(sGrepOption.nGrepCharSet)){
		cmemMessage.AppendString( LS( STR_GREP_CHARSET ) );	//L"    (文字コードセット："
		WCHAR szCpName[100];
		CCodePage::GetNameNormal(szCpName, sGrepOption.nGrepCharSet);
		cmemMessage.AppendString( szCpName );
		cmemMessage.AppendString( L")\r\n" );
	}

	if( 0 < nWork ){ // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if( sGrepOption.nGrepOutputLineType == 1 ){
			/* 該当行 */
			pszWork = LS( STR_GREP_SHOW_MATCH_LINE );	//L"    (一致した行を出力)\r\n"
		}else if( sGrepOption.nGrepOutputLineType == 2 ){
			// 否該当行
			pszWork = LS( STR_GREP_SHOW_MATCH_NOHITLINE );	//L"    (一致しなかった行を出力)\r\n"
		}else{
			if( gi.bGrepReplace && gi.sGrepSearchOption.bRegularExp && !gi.bGrepPaste ){
				pszWork = LS(STR_GREP_SHOW_FIRST_LINE);
			}else{
				pszWork = LS( STR_GREP_SHOW_MATCH_AREA );
			}
		}
		cmemMessage.AppendString( pszWork );

		if( sGrepOption.bGrepOutputFileOnly ){
			pszWork = LS( STR_GREP_SHOW_FIRST_MATCH );	//L"    (ファイル毎最初のみ検索)\r\n"
			cmemMessage.AppendString( pszWork );
		}
	}

	cmemMessage.AppendString( L"\r\n\r\n" );
	nWork = cmemMessage.GetStringLength();
	pszWork = cmemMessage.GetStringPtr();
//@@@ 2002.01.03 YAZAKI Grep直後はカーソルをGrep直前の位置に動かす
	CLayoutInt tmp_PosY_Layout = pcViewDst->m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( 0 < nWork && sGrepOption.bGrepHeader ){
		AppendResult( pcOutput, cmemMessage );
	}
	cmemMessage._SetStringLength(0);
	pszWork = nullptr;
	
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

	// 検索の条件と列挙の条件(全ファイル・全フォルダーで共通)
	const SGrepSearchCond cond{ gi.cmGrepKey.GetStringPtr(), cmemReplace, gi.sGrepSearchOption, sGrepOption, pattern };
	const SGrepEnumCond enumCond{ cGrepEnumKeys, cGrepExceptAbsFiles, cGrepExceptAbsFolders };

	int nGrepTreeResult = 0;

	if( hWndTarget ){
		for( HWND hwnd = hWndTarget; nullptr != hwnd; hwnd = nullptr ){
			SGrepOutput out{ pcOutput, pcProgress, &nHitCount, cmemMessage, cUnicodeBuffer };
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
			const SGrepFilePath path{
				szWindowName,
				currentFile.c_str(),
				szWindowPath,
				(sGrepOption.bGrepSeparateFolder && sGrepOption.bGrepOutputBaseFolder ? L"" : szWindowPath),
				(sGrepOption.bGrepSeparateFolder ? szWindowName : currentFile.c_str() + nPathLen)
			};
			int nTreeRet = DoGrepFile( cond, &cRegexp, out, hwnd, path, bOutputFolderName );
			if( nTreeRet == -1 ){
				nGrepTreeResult = -1;
				break;
			}
			nGrepTreeResult += nTreeRet;
		}
		if( 0 < cmemMessage.GetStringLength() ){
			AppendResult( pcOutput, cmemMessage );
			pcViewDst->GetCommander().Command_GOFILEEND( false );
			if( !CEditWnd::getInstance()->UpdateTextWrap() )
				CEditWnd::getInstance()->RedrawAllViews( pcViewDst );
			cmemMessage.Clear();
		}
		nHitCount = nGrepTreeResult;
	}else{
		for( int nPath = 0; nPath < (int)vPaths.size(); nPath++ ){
			SGrepOutput out{ pcOutput, pcProgress, &nHitCount, cmemMessage, cUnicodeBuffer };
			std::wstring sPath = ChopYen( vPaths[nPath] );
			int nTreeRet = DoGrepTree( cond, &cRegexp, out, enumCond, sPath.c_str(), sPath.c_str(), 0 );
			if( nTreeRet == -1 ){
				nGrepTreeResult = -1;
				break;
			}
			nGrepTreeResult += nTreeRet;
		}
		if( 0 < cmemMessage.GetStringLength() ) {
			AppendResult( pcOutput, cmemMessage );
			cmemMessage._SetStringLength(0);
		}
	}
	if( -1 == nGrepTreeResult && sGrepOption.bGrepHeader ){
		const wchar_t* p = LS( STR_GREP_SUSPENDED );	//L"中断しました。\r\n"
		CNativeW cmemSuspend;
		cmemSuspend.SetString( p );
		AppendResult( pcOutput, cmemSuspend );
	}
	if( sGrepOption.bGrepHeader ){
		WCHAR szBuffer[128];
		if( gi.bGrepReplace ){
			auto_sprintf( szBuffer, LS(STR_GREP_REPLACE_COUNT), nHitCount );
		}else{
			auto_sprintf( szBuffer, LS( STR_GREP_MATCH_COUNT ), nHitCount );
		}
		CNativeW cmemOutput;
		cmemOutput.SetString( szBuffer );
		AppendResult( pcOutput, cmemOutput );
#if defined(_DEBUG) && defined(TIME_MEASURE)
		auto_sprintf( szBuffer, LS(STR_GREP_TIMER), cRunningTimer.Read() );
		cmemOutput.SetString( szBuffer );
		AppendResult( pcOutput, cmemOutput );
#endif
	}
	pcViewDst->GetCaret().MoveCursor( CLayoutPoint(CLayoutInt(0), tmp_PosY_Layout), true );	//	カーソルをGrep直前の位置に戻す。

	// 「Grep実行中」のダイアログを閉じる
	pcDlgRunning.reset();

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

	if (!gi.bGrepCurFolder &&
		!vPaths.empty())
	{
		// 現行フォルダーを検索したフォルダーに変更
		cxx::SetCurrentDirectoryW(vPaths.front());
	}

	return nHitCount;
}

/*!
	@brief フォルダー 1 つの中から、Grep の対象ファイルを列挙する
	@param[in]	pszPath				検索するフォルダー
	@param[in]	cGrepEnumKeys		検索対象ファイル・除外のパターン
	@param[in]	cGrepExceptAbsFiles	除外ファイル(絶対パス)
	@return 対象ファイル(列挙した順)
*/
std::vector<CGrepAgent::SGrepTargetFile> CGrepAgent::EnumerateGrepTargetFiles(
	const WCHAR*		pszPath,
	CGrepEnumKeys&		cGrepEnumKeys,
	CGrepEnumFiles&		cGrepExceptAbsFiles
)
{
	CGrepEnumOptions cGrepEnumOptions;
	CGrepEnumFilterFiles cGrepEnumFilterFiles;
	cGrepEnumFilterFiles.Enumerates( pszPath, cGrepEnumKeys, cGrepEnumOptions, cGrepExceptAbsFiles );

	std::vector<SGrepTargetFile> vFiles;
	const int count = cGrepEnumFilterFiles.GetCount();
	vFiles.reserve( count );
	for( int i = 0; i < count; i++ ){
		vFiles.emplace_back( cGrepEnumFilterFiles.GetFileName( i ), cGrepEnumFilterFiles.GetFileSizeLow( i ) );
	}
	return vFiles;
}

/*!
	@brief フォルダー 1 つぶんの対象ファイルを、列挙した順に検索する

	@retval 0	正常
	@retval -1	中断した。出力していない結果は cmemMessage に残っている
*/
int CGrepAgent::DoGrepFiles(
	const SGrepSearchCond&	cond,				//!< [in] 検索の条件(全ファイルで共通)
	CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	SGrepOutput&			out,				//!< [i/o] 結果の出力先と出力の状態
	const std::vector<SGrepTargetFile>& vFiles,	//!< [in] 対象ファイル(EnumerateGrepTargetFiles() の結果)
	const WCHAR*			pszPath,			//!< [in] 検索するフォルダー
	const WCHAR*			pszBasePath			//!< [in] 検索対象パス(ベースフォルダー)
)
{
	IGrepOutputTarget* const	pOutput = out.pOutput;
	IGrepProgress* const	pProgress = out.pProgress;
	const wchar_t* const	pszKey = cond.pszKey;
	const GrepInfo&			sGrepOption = cond.sGrepOption;
	const int* const		pnHitCount = out.pnHitCount;
	CNativeW&				cmemMessage = out.cmemMessage;
	int			nWork = 0;
	int			nHitCountOld = -100;
	bool		bOutputFolderName = false;
	const auto	nBasePathLen = int(std::wstring_view(pszBasePath).size());
	// pszPath はベースフォルダーかその下。ベースフォルダーそのものなら区切りの「\」は無い
	const int	nBasePathLen2 = std::min( nBasePathLen + 1, int(std::wstring_view(pszPath).size()) );
	const WCHAR* pszFolder = (sGrepOption.bGrepSeparateFolder && sGrepOption.bGrepOutputBaseFolder ? pszPath + nBasePathLen2 : pszPath);

	for( const auto& target : vFiles ){
		const LPCWSTR lpFileName = target.strFileName.c_str();

		if( grep::IsGrepCanceled( pProgress ) ){
			return -1;
		}

		// 定期的に grep 中のファイル名表示を更新
		if( pProgress->IsFileNameTime() ){
			pProgress->SetFileName( lpFileName );
		}

		std::wstring currentFile = pszPath;
		currentFile += L"\\";
		currentFile += lpFileName;
		const WCHAR* pszRelPath = (sGrepOption.bGrepSeparateFolder ? lpFileName : currentFile.c_str() + nBasePathLen + 1);

		/* ファイル内の検索 */
		const SGrepFilePath path{ lpFileName, currentFile.c_str(), pszBasePath, pszFolder, pszRelPath };
		int nRet;
		if( sGrepOption.bGrepReplace ){
			nRet = DoGrepReplaceFile( cond, pRegexp, out, path, bOutputFolderName );
		}else{
			nRet = DoGrepFile( cond, pRegexp, out, nullptr, path, bOutputFolderName );
		}

		// 2003.06.23 Moca リアルタイム表示のときは早めに表示
		// データ検索のときファイルの合計が最大10MBを超えたら表示
		if( pOutput->IsDrawing() && L'\0' != pszKey[0] ){
			nWork += ( target.dwFileSizeLow + 1023 ) / 1024;
		}
		if( pOutput->IsDrawing() && 10000 < nWork ){
			nHitCountOld = -100; // 即表示
		}
		/* 結果出力 */
		if( 0 < cmemMessage.GetStringLength() &&
		   (*pnHitCount - nHitCountOld) >= 10 &&
		   (::GetTickCount64() - m_lastAddTailTime) > ADDTAIL_INTERVAL_MILLISEC
		){
			AppendResult( pOutput, cmemMessage );
			cmemMessage._SetStringLength(0);
			nWork = 0;
			nHitCountOld = *pnHitCount;
		}
		if( -1 == nRet ){
			return -1;
		}
	}
	return 0;
}

/*! @brief Grep実行

	@date 2001.06.27 genta	正規表現ライブラリの差し替え
	@date 2003.06.23 Moca   サブフォルダー→ファイルだったのをファイル→サブフォルダーの順に変更
	@date 2003.06.23 Moca   ファイル名から""を取り除くように
	@date 2003.03.27 みく   除外ファイル指定の導入と重複検索防止の追加．
		大部分が変更されたため，個別の変更点記入は無し．
*/
int CGrepAgent::DoGrepTree(
	const SGrepSearchCond&	cond,				//!< [in] 検索の条件(全ファイルで共通)
	CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	SGrepOutput&			out,				//!< [i/o] 結果の出力先と出力の状態
	const SGrepEnumCond&	enumCond,			//!< [in] フォルダーの列挙の条件
	const WCHAR*			pszPath,			//!< [in] 検索対象パス
	const WCHAR*			pszBasePath,		//!< [in] 検索対象パス(ベースフォルダー)
	int						nNest				//!< [in] ネストレベル
)
{
	IGrepOutputTarget* const	pOutput = out.pOutput;
	IGrepProgress* const	pProgress = out.pProgress;
	const GrepInfo&			sGrepOption = cond.sGrepOption;
	CNativeW&				cmemMessage = out.cmemMessage;
	int			count;
	LPCWSTR		lpFileName;

	/*
	 * カレントフォルダーのファイルを探索する。
	 */
	if( const std::vector<SGrepTargetFile> vFiles = EnumerateGrepTargetFiles( pszPath, enumCond.cGrepEnumKeys, enumCond.cGrepExceptAbsFiles );
		-1 == DoGrepFiles( cond, pRegexp, out, vFiles, pszPath, pszBasePath ) ){
		goto cancel_return;
	}

	/*
	 * サブフォルダーを検索する。
	 */
	if( sGrepOption.bGrepSubFolder ){
		CGrepEnumOptions cGrepEnumOptionsDir;
		CGrepEnumFilterFolders cGrepEnumFilterFolders;
		cGrepEnumFilterFolders.Enumerates( pszPath, enumCond.cGrepEnumKeys, cGrepEnumOptionsDir, enumCond.cGrepExceptAbsFolders );

		count = cGrepEnumFilterFolders.GetCount();
		for( int i = 0; i < count; i++ ){
			lpFileName = cGrepEnumFilterFolders.GetFileName( i );

			//フォルダー名を作成する。
			// 2010.08.01 キャンセルでメモリリークしてました
			std::wstring currentPath  = pszPath;
			currentPath += L"\\";
			currentPath += lpFileName;

			//サブフォルダーの探索を再帰呼び出し。中断されたら呼ばない
			if( grep::IsGrepCanceled( pProgress )
			 || -1 == DoGrepTree( cond, pRegexp, out, enumCond, currentPath.c_str(), pszBasePath, nNest + 1 ) ){
				goto cancel_return;
			}
			pProgress->SetFolder( pszPath );	//@@@ 2002.01.10 add サブフォルダーから戻ってきたら...
		}
	}

	pProgress->SetFileName( L" " );	// 2002/09/09 Moca add

	return 0;

cancel_return:;
	/* 結果出力 */
	if( 0 < cmemMessage.GetStringLength() ){
		AppendResult( pOutput, cmemMessage );
		cmemMessage._SetStringLength(0);
	}

	return -1;
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
	const SGrepSearchCond&	cond,				//!< [in] 検索の条件(全ファイルで共通)
	CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	SGrepOutput&			out,				//!< [i/o] 結果の出力先と出力の状態
	HWND					hWndTarget,			//!< [in] 対象Windows(NULLでファイル)
	const SGrepFilePath&	path,				//!< [in] 検索するファイルの名前とパス
	bool&					bOutputFolderName	//!< [i/o] フォルダー名を出力したか
)
{
	// 構造体にまとめた引数を、本体で使ってきた名前で受ける(置換後文字列は使わない)
	[[maybe_unused]] const auto& [pszKey, cmGrepReplace, sSearchOption, sGrepOption, pattern] = cond;
	auto& [pOutput, pProgress, pnHitCount, cmemMessage, cUnicodeBuffer, bOutputBaseFolder] = out;
	const auto& [pszFile, pszFullPath, pszBaseFolder, pszFolder, pszRelPath] = path;
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

	auto nKeyLen = int(wcslen(pszKey));
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
			auto pszWork = std::make_unique<wchar_t[]>(wcslen(pszFullPath) + wcslen(pszCodeName) + 10);
			wchar_t* szWork0 = &pszWork[0];
			if( sGrepOption.bGrepOutputBaseFolder || sGrepOption.bGrepSeparateFolder ){
				if( !bOutputBaseFolder && sGrepOption.bGrepOutputBaseFolder ){
					const wchar_t* pszFormatBasePath = L"";
					if( sGrepOption.bGrepSeparateFolder ){
						pszFormatBasePath = L"◎\"%s\"\r\n";	// (A)
					}else{
						pszFormatBasePath = pszFormatBasePath2;	// (B)
					}
					auto_sprintf( szWork0, pszFormatBasePath, pszBaseFolder );
					cmemMessage.AppendString( szWork0 );
					bOutputBaseFolder = true;
				}
				if( !bOutputFolderName && sGrepOption.bGrepSeparateFolder ){
					if( pszFolder[0] ){
						auto_sprintf( szWork0, L"■\"%s\"\r\n", pszFolder );	// (C), (D)
					}else{
						wcscpy( szWork0, L"■\r\n" );
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
		pProgress->SetHitCount( *pnHitCount );
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

		/* 処理中のユーザー操作を可能にし、中断ボタン押下をチェックする */
		if( pProgress->IsCheckTime() && pProgress->IsCancelRequested() ){
			return -1;
		}
		int nOutputHitCount = 0;

		/* 検索条件が長さゼロの場合はファイル名だけ返す */
		// 2002/08/29 ファイルオープンの手前へ移動
	
		const CLineMatchFinder finder( pattern, pRegexp );

		// 注意 : cfl.ReadLine が throw する可能性がある
		while( RESULT_FAILURE != cfl.ReadLine( &cUnicodeBuffer, &cEol ) )
		{
			const wchar_t*	pLine = cUnicodeBuffer.GetStringPtr();
			const int	nLineLen = cUnicodeBuffer.GetStringLength();

			nEolCodeLen = cEol.GetLen();
			++nLine;

			/* 処理中のユーザー操作を可能にする */
			// 2010.08.31 間隔を1/32にする
			if( 0 == nLine % 32 && pProgress->IsCheckTime() ){
				/* 中断ボタン押下チェック */
				if( pProgress->IsCancelRequested() ){
					return -1;
				}
				//	2003.06.23 Moca 表示設定をチェック
				pProgress->ApplyRealTimeView();
				// 2002/08/30 Moca 進行状態を表示する(5MB以上)
				if( 5000000 < cfl.GetFileSize() ){
					grep::ShowReadPercent( pProgress, pszFile, cfl.GetPercent(), nOldPercent );
				}else{
					pProgress->SetFileName( pszFile );
				}
				pProgress->SetHitCount( *pnHitCount );
				pProgress->SetFolder( pszFolder );
			}
			int nHitOldLine = nHitCount;
			int nHitCountOldLine = *pnHitCount;

			/* 一致を探す(正規表現・単語単位・文字列) */
			//	Jun. 21, 2003 genta ループ条件見直し
			//	マッチ箇所を1行から複数検出するケースを標準に，
			//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
			//	ループ継続・打ち切り条件(nGrepOutputLineType)を逆にした．
			int nStart = 0;
			while( const auto match = finder.Find( pLine, nLineLen, nStart ) ){
				++nHitCount;
				++(*pnHitCount);
				if( sGrepOption.nGrepOutputLineType != 2 ){
					OutputPathInfo(
						cmemMessage, sGrepOption,
						pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
						bOutputBaseFolder, bOutputFolderName, bOutFileName
					);
					//	Jun. 25, 2002 genta 桁位置は1始まりなので1を足す
					SetGrepResult(
						cmemMessage, pszDispFilePath, pszCodeName,
						nLine, match->nIndex + 1, pLine, nLineLen, nEolCodeLen,
						pLine + match->nIndex, match->nLength, sGrepOption
					);
				}
				//	Jun. 21, 2003 genta 行単位で出力する場合は1つ見つかれば十分
				if ( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ) {
					break;
				}
				//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
				nStart = CLineMatchFinder::GetNextStart( pLine, nLineLen, *match );
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
				(::GetTickCount64() - m_lastAddTailTime) >= ADDTAIL_INTERVAL_MILLISEC
			){
				nOutputHitCount = nHitCount;
				AppendResult( pOutput, cmemMessage );
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
	virtual ~CError_WriteFileOpen(){}
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
		,out(nullptr)
		,pcCodeBase(CCodeFactory::CreateCodeBase(code_,0))
		,memMessage(message)
	{
		name += L".skrnew";
	}
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
				out = new CBinaryOutputStream(name.c_str(), true);
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
			delete out;
			out = nullptr;
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
			delete out;
			out = nullptr;
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
	CBinaryOutputStream* out;
	std::unique_ptr<CCodeBase> pcCodeBase;
	CNativeW&	memMessage;
};

/*!
	Grep置換実行
	@date 2013.06.12 Moca 新規作成
*/
int CGrepAgent::DoGrepReplaceFile(
	const SGrepSearchCond&	cond,				//!< [in] 検索の条件(全ファイルで共通)
	CBregexp*				pRegexp,			//!< [in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	SGrepOutput&			out,				//!< [i/o] 結果の出力先と出力の状態
	const SGrepFilePath&	path,				//!< [in] 検索するファイルの名前とパス
	bool&					bOutputFolderName	//!< [i/o] フォルダー名を出力したか
)
{
	// 構造体にまとめた引数を、本体で使ってきた名前で受ける(検索語は使わない)
	[[maybe_unused]] const auto& [pszKey, cmGrepReplace, sSearchOption, sGrepOption, pattern] = cond;
	auto& [pOutput, pProgress, pnHitCount, cmemMessage, cUnicodeBuffer, bOutputBaseFolder] = out;
	const auto& [pszFile, pszFullPath, pszBaseFolder, pszFolder, pszRelPath] = path;
	LONGLONG	nLine = 0;
	int		nHitCount = 0;
	ECodeType	nCharCode;
	BOOL	bOutFileName = FALSE;
	CEol	cEol;
	int		nEolCodeLen;
	int		nOldPercent = 0;
	const WCHAR*	pszCodeName = L"";

	const STypeConfigMini* type = nullptr;
	if( !CDocTypeManager().GetTypeConfigMini( CDocTypeManager().GetDocumentTypeOfPath( pszFile ), &type ) ){
		return -1;
	}
	CFileLoad	cfl( type->m_encoding );	// 2012/12/18 Uchi 検査するファイルのデフォルトの文字コードを取得する様に
	bool bBom;
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
		/* 処理中のユーザー操作を可能にし、中断ボタン押下をチェックする */
		if( pProgress->IsCancelRequested() ){
			return -1;
		}
		int nOutputHitCount = 0;

		const CLineMatchFinder finder( pattern, pRegexp );

		CNativeW cOutBuffer;
		// 注意 : cfl.ReadLine が throw する可能性がある
		while( RESULT_FAILURE != cfl.ReadLine( &cUnicodeBuffer, &cEol ) )
		{
			const wchar_t*	pLine = cUnicodeBuffer.GetStringPtr();
			int		nLineLen = cUnicodeBuffer.GetStringLength();

			nEolCodeLen = cEol.GetLen();
			++nLine;

			if( pProgress->IsCheckTime() ){
				/* 処理中のユーザー操作を可能にし、中断ボタン押下をチェックする */
				if( pProgress->IsCancelRequested() ){
					return -1;
				}
				//	2003.06.23 Moca 表示設定をチェック
				pProgress->ApplyRealTimeView();
				// 2002/08/30 Moca 進行状態を表示する(5MB以上)
				if( 5000000 < cfl.GetFileSize() ){
					grep::ShowReadPercent( pProgress, pszFile, cfl.GetPercent(), nOldPercent );
				}
			}
			cOutBuffer.SetString( L"", 0 );
			bool bOutput = true;
			if( sGrepOption.bGrepOutputFileOnly && 1 <= nHitCount ){
				bOutput = false;
			}

			/* 正規表現(貼り付けなし) */
			if( sSearchOption.bRegularExp && !sGrepOption.bGrepPaste ){
				// gオプションで行末まで一度に置換する
				if( const int nMatchNum = pRegexp->Replace( pLine, nLineLen, 0 ) ){
					//	パターン発見
					const int nIndex = pRegexp->GetIndex();
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
							pLine + nIndex, pRegexp->GetMatchLen(),
							sGrepOption
						);
					}
					output.OutputHead();
					nHitCount += nMatchNum;
					*pnHitCount += nMatchNum;
					cOutBuffer.AppendString( pRegexp->GetString(), pRegexp->GetStringLen() );
				}else{
					cOutBuffer.AppendString( pLine, nLineLen );
				}
			}
			/* 一致を探して置き換える(正規表現で貼り付け・単語単位・文字列) */
			else{
				int nStart = 0;
				int nCopyFrom = 0;	// 置換後の行に、まだ写していない部分の始まり
				while( const auto match = finder.Find( pLine, nLineLen, nStart ) ){
					if( bOutput ){
						OutputPathInfo(
							cmemMessage, sGrepOption,
							pszFullPath, pszBaseFolder, pszFolder, pszRelPath, pszCodeName,
							bOutputBaseFolder, bOutputFolderName, bOutFileName
						);
						/* Grep結果を、cmemMessageに格納する */
						SetGrepResult(
							cmemMessage, pszDispFilePath, pszCodeName,
							nLine, match->nIndex + 1,
							pLine, nLineLen, nEolCodeLen,
							pLine + match->nIndex, match->nLength,
							sGrepOption
						);
						if( sGrepOption.nGrepOutputLineType != 0 || sGrepOption.bGrepOutputFileOnly ){
							bOutput = false;
						}
					}
					output.OutputHead();
					++nHitCount;
					++(*pnHitCount);
					// 一致の前の部分を写す(一致が写し始める位置より前なら写さない)
					cOutBuffer.AppendString( &pLine[nCopyFrom], std::max( 0, match->nIndex - nCopyFrom ) );
					cOutBuffer.AppendNativeData( cmGrepReplace );
					//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
					//	長さ 0 の一致では、写し始める位置も探し始める位置と一緒に 1 文字進む(#2743 の 4。今の動きのまま)
					nStart = CLineMatchFinder::GetNextStart( pLine, nLineLen, *match );
					nCopyFrom = nStart;
				}
				if( nCopyFrom < nLineLen ){
					cOutBuffer.AppendString( &pLine[nCopyFrom], nLineLen - nCopyFrom );
				}
			}
			output.AppendBuffer(cOutBuffer);

			if( 0 < cmemMessage.GetStringLength() &&
				(::GetTickCount64() - m_lastAddTailTime > ADDTAIL_INTERVAL_MILLISEC)
			){
				nOutputHitCount = nHitCount;
				AppendResult( pOutput, cmemMessage );
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
