#include "StdAfx.h"
#include "CEditView.h"
#include "_main/CAppMode.h"
#include "_os/COsVersionInfo.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "env/CFormatManager.h"
#include "dlg/CDlgCancel.h"
#include "util/window.h"
#include "util/tchar_template.h"
#include "sakura_rc.h" // IDD_EXECRUNNING

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       外部コマンド                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
class OutputAdapter
{
public:
	OutputAdapter(CEditView* view, BOOL bToEditWindow) : m_view(view),m_bWindow(bToEditWindow){
		m_pCShareData = CShareData::getInstance();
		m_pCommander  = &(view->GetCommander());
	}

	void OutputW(const WCHAR* pBuf, int size = -1);
	void OutputA(const ACHAR* pBuf, int size = -1);
	void Output(const WCHAR* pBuf, int size = -1){ OutputW(pBuf, size); }
	void Output(const ACHAR* pBuf, int size = -1){ OutputA(pBuf, size); }

private:
	BOOL m_bWindow;
	CEditView* m_view;
	CShareData* m_pCShareData;
	CViewCommander* m_pCommander;
};

/*!	@brief	外部コマンドの実行

	@param[in] pszCmd コマンドライン
	@param[in] nFlgOpt オプション
		@li	0x01	標準出力を得る
		@li	0x02	標準出力のリダイレクト先（無効=アウトプットウィンドウ / 有効=編集中のウィンドウ）
		@li	0x04	編集中ファイルを標準入力へ
		@li	0x08	標準出力をUnicodeで行う
		@li	0x10	標準入力をUnicodeで行う
		@li	0x20	情報出力する
		@li	0x40	情報出力しない

	@note	子プロセスの標準出力取得はパイプを使用する
	@note	子プロセスの標準入力への送信は一時ファイルを使用

	@author	N.Nakatani
	@date	2001/06/23
	@date	2001/06/30	GAE
	@date	2002/01/24	YAZAKI	1バイト取りこぼす可能性があった
	@date	2003/06/04	genta
	@date	2004/09/20	naoh	多少は見やすく・・・
	@date	2004/01/23	genta
	@date	2004/01/28	Moca	改行コードが分割されるのを防ぐ
	@date	2007/03/18	maru	オプションの拡張
	@date	2008/06/07	Uchi	Unidoeの使用
	@date	2009/02/21	ryoji	ビューモードや上書き禁止のときは編集中ウィンドウへは出力しない（指定時はアウトプットへ）
	@date	2010/04/12	Moca	nFlgOptの0x20,0x40追加。無限出力対策。WM_QUIT対策。UnicodeのCarry周りの修正

	TODO:	標準入力・標準エラーの取込選択。カレントディレクトリ。UTF-8等への対応
*/
void CEditView::ExecCmd( const TCHAR* pszCmd, int nFlgOpt )
{
	HANDLE				hStdOutWrite, hStdOutRead, hStdIn;
	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );
	CDlgCancel				cDlgCancel;

	bool bEditable = m_pcEditDoc->IsEditable();

	//	From Here 2006.12.03 maru 引数を拡張のため
	BOOL	bGetStdout		= nFlgOpt & 0x01 ? TRUE : FALSE;	//	子プロセスの標準出力を得る
	BOOL	bToEditWindow	= ((nFlgOpt & 0x02) && bEditable) ? TRUE : FALSE;	//	TRUE=編集中のウィンドウ / FALSAE=アウトプットウィンドウ
	BOOL	bSendStdin		= nFlgOpt & 0x04 ? TRUE : FALSE;	//	編集中ファイルを子プロセスSTDINに渡す
	BOOL	bIOUnicodeGet	= nFlgOpt & 0x08 ? TRUE : FALSE;	//	標準出力をUnicodeで行う	2008/6/17 Uchi
	BOOL	bIOUnicodeSend	= nFlgOpt & 0x10 ? TRUE : FALSE;	//	標準入力をUnicodeで行う	2008/6/20 Uchi
	//	To Here 2006.12.03 maru 引数を拡張のため
	// 2010.04.12 Moca 情報出力
	BOOL	bOutputExtInfo	= !bToEditWindow;
	if( nFlgOpt & 0x20 ) bOutputExtInfo = TRUE;
	if( nFlgOpt & 0x40 ) bOutputExtInfo = FALSE;

	// 編集中のウィンドウに出力する場合の選択範囲処理用	/* 2007.04.29 maru */
	CLayoutInt	nLineFrom(0), nColmFrom(0);
	bool bBeforeTextSelected = GetSelectionInfo().IsTextSelected();
	if (bBeforeTextSelected){
		nLineFrom = this->GetSelectionInfo().m_sSelect.GetFrom().y; //m_nSelectLineFrom;
		nColmFrom = this->GetSelectionInfo().m_sSelect.GetFrom().x; //m_nSelectColmFrom;
	}

	//子プロセスの標準出力と接続するパイプを作成
	SECURITY_ATTRIBUTES	sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE ) {
		//エラー。対策無し
		return;
	}
	//hStdOutReadのほうは子プロセスでは使用されないので継承不能にする（子プロセスのリソースを無駄に増やさない）
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), &hStdOutRead,					// 新しい継承不能ハンドルを受け取る	// 2007.01.31 ryoji
				0, FALSE,
				DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS );	// 元の継承可能ハンドルは DUPLICATE_CLOSE_SOURCE で閉じる	// 2007.01.31 ryoji


	// From Here 2007.03.18 maru 子プロセスの標準入力ハンドル
	// CDocLineMgr::WriteFileなど既存のファイル出力系の関数のなかには
	// ファイルハンドルを返すタイプのものがないので、一旦書き出してから
	// 一時ファイル属性でオープンすることに。
	hStdIn = NULL;
	if(bSendStdin){	/* 現在編集中のファイルを子プロセスの標準入力へ */
		TCHAR		szPathName[MAX_PATH];
		TCHAR		szTempFileName[MAX_PATH];
		int			nFlgOpt;

		GetTempPath( MAX_PATH, szPathName );
		GetTempFileName( szPathName, TEXT("skr_"), 0, szTempFileName );
		DEBUG_TRACE( _T("CEditView::ExecCmd() TempFilename=[%ts]\n"), szTempFileName );

		nFlgOpt = bBeforeTextSelected ? 0x01 : 0x00;		/* 選択範囲を出力 */

		if( !GetCommander().Command_PUTFILE( to_wchar(szTempFileName), bIOUnicodeSend? CODE_UNICODE : CODE_SJIS, nFlgOpt) ){	// 一時ファイル出力
			hStdIn = NULL;
		} else {
			// 子プロセスへの継承用にファイルを開く
			hStdIn = CreateFile(
				szTempFileName,
				GENERIC_READ,
				0,
				&sa,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
				NULL
			);
			if(hStdIn == INVALID_HANDLE_VALUE) hStdIn = NULL;
		}
	}
	
	if (hStdIn == NULL) {	/* 標準入力を制御しない場合、または一時ファイルの生成に失敗した場合 */
		bSendStdin = FALSE;
		hStdIn = GetStdHandle( STD_INPUT_HANDLE );
	}
	// To Here 2007.03.18 maru 子プロセスの標準入力ハンドル
	

	//CreateProcessに渡すSTARTUPINFOを作成
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(sui) );
	sui.cb = sizeof(sui);
	if( bGetStdout || bSendStdin ) {
		sui.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		sui.wShowWindow = bGetStdout ? SW_HIDE : SW_SHOW;
		sui.hStdInput = hStdIn;
		sui.hStdOutput = bGetStdout ? hStdOutWrite : GetStdHandle( STD_OUTPUT_HANDLE );
		sui.hStdError = bGetStdout ? hStdOutWrite : GetStdHandle( STD_ERROR_HANDLE );
	}

	//コマンドライン実行
	TCHAR	cmdline[1024];
	_tcscpy( cmdline, pszCmd );
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
				CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
		//実行に失敗した場合、コマンドラインベースのアプリケーションと判断して
		// command(9x) か cmd(NT) を呼び出す

		// 2010.08.27 Moca システムディレクトリ付加
		TCHAR szCmdDir[_MAX_PATH];
		if( IsWin32NT() ){
			::GetSystemDirectory(szCmdDir, _countof(szCmdDir));
		}else{
			::GetWindowsDirectory(szCmdDir, _countof(szCmdDir));
		}

		//コマンドライン文字列作成
		auto_sprintf(
			cmdline,
			_T("\"%ts\\%ts\" %ts%ts%ts"),
			szCmdDir,
			( IsWin32NT() ? _T("cmd.exe") : _T("command.com") ),
			( bIOUnicodeGet ? _T("/U") : _T("") ),		// Unicdeモードでコマンド実行	2008/6/17 Uchi
			( bGetStdout ? _T("/C ") : _T("/K ") ),
			pszCmd
		);
		if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
					CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
			MessageBox( NULL, cmdline, _T("コマンド実行は失敗しました。"), MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
	}

	// ファイル全体に対するフィルタ動作
	//	現在編集中のファイルからのデータ書きだしおよびデータ取り込みが
	//	指定されていて，かつ範囲選択が行われていない場合は
	//	「すべて選択」されているものとして，編集データ全体を
	//	コマンドの出力結果と置き換える．
	//	2007.05.20 maru
	if(!bBeforeTextSelected && bSendStdin && bGetStdout && bToEditWindow){
		GetSelectionInfo().SetSelectArea(
			CLayoutRange(
				CLayoutPoint(CLayoutInt(0), CLayoutInt(0)),
				CLayoutPoint(CLayoutInt(0), m_pcEditDoc->m_cLayoutMgr.GetLineCount())
			)
		);
		DeleteData( true );
	}

	// hStdOutWrite は CreateProcess() で継承したので親プロセスでは用済み
	// hStdInも親プロセスでは使用しないが、Win9x系では子プロセスが終了してから
	// クローズするようにしないと一時ファイルが自動削除されない
	CloseHandle(hStdOutWrite);
	hStdOutWrite = NULL;	// 2007.09.08 genta 二重closeを防ぐ

	if( bGetStdout ) {
		DWORD	new_cnt;
		int		bufidx = 0;
		bool	bLoopFlag = true;
		bool	bCancelEnd = false; // キャンセルでプロセス停止
		OutputAdapter oa(this, bToEditWindow );

		//中断ダイアログ表示
		cDlgCancel.DoModeless( G_AppInstance(), m_hwndParent, IDD_EXECRUNNING );
		// ダイアログにコマンドを表示
		::DlgItem_SetText( cDlgCancel.GetHwnd(), IDC_STATIC_CMD, pszCmd );
		//実行したコマンドラインを表示
		// 2004.09.20 naoh 多少は見やすく・・・
		// 2006.12.03 maru アウトプットウィンドウにのみ出力
		if (bOutputExtInfo)
		{
			TCHAR szTextDate[1024], szTextTime[1024];
			SYSTEMTIME systime;
			::GetLocalTime( &systime );
			CFormatManager().MyGetDateFormat( systime, szTextDate, _countof( szTextDate ) - 1 );
			CFormatManager().MyGetTimeFormat( systime, szTextTime, _countof( szTextTime ) - 1 );
			WCHAR szOutTemp[1024*2+100];
			oa.OutputW( L"\r\n" );
			oa.OutputW( L"#============================================================\r\n" );
			int len = auto_snprintf_s( szOutTemp, _countof(szOutTemp),
				L"#DateTime : %ts %ts\r\n", szTextDate, szTextTime );
			oa.OutputW( szOutTemp, len );
			len = auto_snprintf_s( szOutTemp, _countof(szOutTemp),
				L"#CmdLine  : %ts\r\n", pszCmd );
			oa.OutputW( szOutTemp, len );
			oa.OutputW( L"#============================================================\r\n" );
		}
		
		//charで読む
		typedef char PIPE_CHAR;
		const int WORK_NULL_TERMS = sizeof(wchar_t); // 出力用\0の分
		const int MAX_BUFIDX = 10; // bufidxの分
		const int MAX_WORK_READ = 1024*5; // 5KiB ReadFileで読み込む限界値
		// 2010.04.13 Moca バッファサイズの調整 1022 Byte 読み取りを 5KiBに変更
		// ボトルネックはアウトプットウィンドウへの転送
		// 相手のプログラムがVC9のstdoutではデフォルトで4096。VC6,VC8やWinXPのtypeコマンドでは1024
		// テキストモードだと new_cntが改行に\rがつく分だけ向こう側の設定値より多く一度に送られてくる
		// 4KBだと 4096 -> 100 -> 4096 -> 100 のように読み取ることになるので5KBにした
		PIPE_CHAR work[MAX_WORK_READ + MAX_BUFIDX + WORK_NULL_TERMS];
		//実行結果の取り込み
		do {
			//プロセスが終了していないか確認
			// Jun. 04, 2003 genta CPU消費を減らすために200msec待つ
			// その間メッセージ処理が滞らないように待ち方をWaitForSingleObjectから
			// MsgWaitForMultipleObjectに変更
			// Jan. 23, 2004 genta
			// 子プロセスの出力をどんどん受け取らないと子プロセスが
			// 停止してしまうため，待ち時間を200msから20msに減らす
			switch( MsgWaitForMultipleObjects( 1, &pi.hProcess, FALSE, 20, QS_ALLEVENTS )){
				case WAIT_OBJECT_0:
					//終了していればループフラグをfalseとする
					//ただしループの終了条件は プロセス終了 && パイプが空
					bLoopFlag = false;
					break;
				case WAIT_OBJECT_0 + 1:
					//処理中のユーザー操作を可能にする
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						// WM_QUIT受信。ただちに終了処理
						::TerminateProcess( pi.hProcess, 0 );
						goto finish;
					}
					break;
				default:
					break;
			}
			//中断ボタン押下チェック
			if( cDlgCancel.IsCanceled() ){
				//指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
				::TerminateProcess( pi.hProcess, 0 );
				bCancelEnd  = true;
				break;
			}
			new_cnt = 0;

			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ) {	//パイプの中の読み出し待機中の文字数を取得
				while( new_cnt > 0 ) {												//待機中のものがある

					if( new_cnt > MAX_WORK_READ) {							//パイプから読み出す量を調整
						new_cnt = MAX_WORK_READ;
					}
					DWORD	read_cnt;
					::ReadFile( hStdOutRead, &work[bufidx], new_cnt, &read_cnt, NULL );	//パイプから読み出し
					read_cnt += bufidx;													//work内の実際のサイズにする

					if( read_cnt == 0 ) {
						// Jan. 23, 2004 genta while追加のため制御を変更
						break;
					}
					// Unicode で データを受け取る start 2008/6/8 Uchi
					if (bIOUnicodeGet) {
						wchar_t*	workw;
						int			read_cntw;
						bool		bCarry;
						char		byteCarry = 0;
						workw = (wchar_t*)work;
						read_cntw = (int)read_cnt/sizeof(wchar_t);
						if( read_cnt % (int)sizeof(wchar_t) ){
							byteCarry = work[read_cnt-1];
						}
						if(read_cntw){
							workw[read_cntw] = L'\0';
							bCarry = false;
							//読み出した文字列をチェックする
							if (workw[read_cntw-1] == L'\r') {
								bCarry = true;
								read_cntw -= 1; // 2010.04.12 1文字余分に消されてた
								workw[read_cntw] = L'\0';
							}
							oa.OutputW( workw, read_cntw );
							bufidx = 0;
							if (bCarry) {
								workw[0] = L'\r'; // 2010.04.12 'r' -> '\r'
								bufidx = sizeof(wchar_t);
								DEBUG_TRACE( _T("ExecCmd: Carry last character [CR]\n") );
							}
						}
						if( read_cnt % (int)sizeof(wchar_t) ){
							// 高確率で0だと思うが1だと困る
							DEBUG_TRACE( _T("ExecCmd: Carry Unicode 1byte [%x]\n"), byteCarry );
							work[bufidx] = byteCarry;
							bufidx += 1;
						}
					}
					// end 2008/6/8 Uchi
					else {
						//読み出した文字列をチェックする
						// \r\n を \r だけとか漢字の第一バイトだけを出力するのを防ぐ必要がある
						//@@@ 2002.1.24 YAZAKI 1バイト取りこぼす可能性があった。
						//	Jan. 28, 2004 Moca 最後の文字はあとでチェックする
						int		j;
						for( j=0; j<(int)read_cnt - 1; j++ ) {
							//	2007.09.10 ryoji
							if( CNativeA::GetSizeOfChar(work, read_cnt, j) == 2 ) {
								j++;
							} else {
								if( work[j] == _T2(PIPE_CHAR,'\r') && work[j+1] == _T2(PIPE_CHAR,'\n') ) {
									j++;
								} else if( work[j] == _T2(PIPE_CHAR,'\n') && work[j+1] == _T2(PIPE_CHAR,'\r') ) {
									j++;
								}
							}
						}
						//	From Here Jan. 28, 2004 Moca
						//	改行コードが分割されるのを防ぐ
						if( j == read_cnt - 1 ){
							if( _IS_SJIS_1(work[j]) ) {
								j = read_cnt + 1; // ぴったり出力できないことを主張
							}else if( work[j] == _T2(PIPE_CHAR,'\r') || work[j] == _T2(PIPE_CHAR,'\n') ) {
								// CRLFの一部ではない改行が末尾にある
								// 次の読み込みで、CRLFの一部になる可能性がある
								j = read_cnt + 1;
							}else{
								j = read_cnt;
							}
						}
						//	To Here Jan. 28, 2004 Moca
						if( j == (int)read_cnt ) {	//ぴったり出力できる場合
							work[read_cnt] = '\0';
							//	2006.12.03 maru アウトプットウィンドウor編集中のウィンドウ分岐追加
							oa.OutputA( work, read_cnt );
							bufidx = 0;
						}
						else {
							char tmp = work[read_cnt-1];
							work[read_cnt-1] = '\0';
							//	2006.12.03 maru アウトプットウィンドウor編集中のウィンドウ分岐追加
							oa.OutputA( work, read_cnt-1 );
							work[0] = tmp;
							bufidx = 1;
							DEBUG_TRACE( _T("ExecCmd: Carry last character [%x]\n"), tmp );
						}
					}
					// Jan. 23, 2004 genta
					// 子プロセスの出力をどんどん受け取らないと子プロセスが
					// 停止してしまうため，バッファが空になるまでどんどん読み出す．
					new_cnt = 0;
					if( ! PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ){
						break;
					}
					Sleep(0);
					
					// 2010.04.12 Moca 相手が出力しつづけていると止められないから
					// BlockingHookとキャンセル確認を読取ループ中でも行う
					// bLoopFlag が立っていないときは、すでにプロセスは終了しているからTerminateしない
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						if( bLoopFlag ){
							::TerminateProcess( pi.hProcess, 0 );
						}
						goto finish;
					}
					if( cDlgCancel.IsCanceled() ){
						//指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
						if( bLoopFlag ){
							::TerminateProcess( pi.hProcess, 0 );
						}
						bCancelEnd = true;
						goto user_cancel;
					}
				}
			}
		} while( bLoopFlag || new_cnt > 0 );

user_cancel:

		// 最後の文字の出力(たいていCR)
		if( 0 < bufidx ){
			if( bIOUnicodeGet ){
				if( bufidx % (int)sizeof(wchar_t) ){
					DEBUG_TRACE( _T("ExecCmd: Carry last Unicode byte [%x]\n"), work[bufidx-1] );
					// UTF-16なのに奇数バイトだった
					work[bufidx] = 0x00; // 上位バイトを0にしてごまかす
					bufidx += 1;
				}
				wchar_t* workw = (wchar_t*)work;
				int bufidxw = bufidx / (int)sizeof(wchar_t);
				workw[bufidxw] = L'\0';
				oa.OutputW( workw, bufidxw );
			}else{
				work[bufidx] = '\0';
				oa.OutputA( work, bufidx );
			}
		}

		if( bCancelEnd && bOutputExtInfo ){
			//	2006.12.03 maru アウトプットウィンドウにのみ出力
			//最後にテキストを追加
			oa.OutputW( L"\r\n中断しました。\r\n" );
		}
		
		{
			//	2006.12.03 maru アウトプットウィンドウにのみ出力
			//	Jun. 04, 2003 genta	終了コードの取得と出力
			DWORD result;
			::GetExitCodeProcess( pi.hProcess, &result );
			if( bOutputExtInfo ){
				WCHAR endCode[128];
				auto_sprintf( endCode, L"\r\n終了コード: %d\r\n", result );
				oa.OutputW( endCode );
			}
			// 2004.09.20 naoh 終了コードが1以上の時はアウトプットをアクティブにする
			if(!bToEditWindow && result > 0) ActivateFrameWindow( GetDllShareData().m_sHandles.m_hwndDebug );
		}
		if (bToEditWindow) {
			if (bBeforeTextSelected){	// 挿入された部分を選択状態に
				GetSelectionInfo().SetSelectArea(
					CLayoutRange(
						CLayoutPoint(nColmFrom, nLineFrom),
						GetCaret().GetCaretLayoutPos()// CLayoutPoint(m_nCaretPosY, m_nCaretPosX )
					)
				);
				GetSelectionInfo().DrawSelectArea();
			}
			//	2006.12.03 maru 編集中のウィンドウに出力時は最後に再描画
			RedrawAll();
		}
	}


finish:
	//終了処理
	if(bSendStdin) CloseHandle( hStdIn );	/* 2007.03.18 maru 標準入力の制御のため */
	if(hStdOutWrite) CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread ) CloseHandle( pi.hThread );
}

/*!
	@param pBuf size未指定なら要NUL終端
	@param size WCHAR単位 
*/
void OutputAdapter::OutputW(const WCHAR* pBuf, int size)
{
	if( m_bWindow ){
		m_pCommander->Command_INSTEXT( false, pBuf, CLogicInt(size), TRUE);
	}else{
		m_pCShareData->TraceOutString( pBuf , size );
	}
}

/*
	@param pBuf size未指定なら要NUL終端
	@param size ACHAR単位 
*/
void OutputAdapter::OutputA(const ACHAR* pBuf, int size)
{
	if( m_bWindow ){
		CNativeW buf;
		if( -1 == size ){
			buf.SetStringOld(pBuf);
		}else{
			buf.SetStringOld(pBuf,size);
		}
		m_pCommander->Command_INSTEXT( false, buf.GetStringPtr(), buf.GetStringLength(), TRUE);
	}else{
		// TraceOutString側にANSI版を作ったほうが高速
		CNativeW buf;
		if( -1 == size ){
			buf.SetStringOld(pBuf);
		}else{
			buf.SetStringOld(pBuf,size);
		}
		m_pCShareData->TraceOutString( buf.GetStringPtr(), (int)buf.GetStringLength() );
	}
}
