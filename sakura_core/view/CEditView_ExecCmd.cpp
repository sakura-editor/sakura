#include "stdafx.h"
#include "CEditView.h"
#include "dlg/CDlgCancel.h"
#include "util/window.h"
#include "util/tchar_template.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       外部コマンド                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	@brief	外部コマンドの実行

	@param[in] pszCmd コマンドライン
	@param[in] nFlgOpt オプション
		@li	0x01	標準出力を得る
		@li	0x02	標準出力のりダイレクト先（無効=アウトプットウィンドウ / 有効=編集中のウィンドウ）
		@li	0x04	編集中ファイルを標準入力へ

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
*/
void CEditView::ExecCmd( const TCHAR* pszCmd, int nFlgOpt )
{
	HANDLE				hStdOutWrite, hStdOutRead, hStdIn;
	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );
	CDlgCancel				cDlgCancel;

	//	From Here 2006.12.03 maru 引数を拡張のため
	BOOL	bGetStdout		= nFlgOpt & 0x01 ? TRUE : FALSE;	//	子プロセスの標準出力を得る
	BOOL	bToEditWindow	= nFlgOpt & 0x02 ? TRUE : FALSE;	//	TRUE=編集中のウィンドウ / FALSAE=アウトプットウィンドウ
	BOOL	bSendStdin		= nFlgOpt & 0x04 ? TRUE : FALSE;	//	編集中ファイルを子プロセスSTDINに渡す
	//	To Here 2006.12.03 maru 引数を拡張のため

	// 編集中のウィンドウに出力する場合の選択範囲処理用	/* 2007.04.29 maru */
	CLayoutInt	nLineFrom, nColmFrom;
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
		DBPRINT( _T("CEditView::ExecCmd() TempFilename=[%ts]\n"), szTempFileName );
		
		nFlgOpt = bBeforeTextSelected ? 0x01 : 0x00;		/* 選択範囲を出力 */
		
		if( !GetCommander().Command_PUTFILE( to_wchar(szTempFileName), CODE_SJIS, nFlgOpt) ){	// 一時ファイル出力
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
	//OSバージョン取得
	COsVersionInfo cOsVer;		// move to	2008/6/7 Uchi
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
				CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
		//実行に失敗した場合、コマンドラインベースのアプリケーションと判断して
		// command(9x) か cmd(NT) を呼び出す

		//OSバージョン取得
		//COsVersionInfo cOsVer;		// move from	2008/6/7 Uchi
		//コマンドライン文字列作成
		auto_sprintf(
			cmdline,
			_T("%ls %ls%ts"),
			( cOsVer.IsWin32NT() ? L"cmd.exe /U" : L"command.com" ),	// Unicdeモードでコマンド実行	2008/6/7 Uchi
			( bGetStdout ? L"/C " : L"/K " ),
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
		DeleteData( TRUE );
	}

	// hStdOutWrite は CreateProcess() で継承したので親プロセスでは用済み
	// hStdInも親プロセスでは使用しないが、Win9x系では子プロセスが終了してから
	// クローズするようにしないと一時ファイルが自動削除されない
	CloseHandle(hStdOutWrite);
	hStdOutWrite = NULL;	// 2007.09.08 genta 二重closeを防ぐ

	if( bGetStdout ) {
		DWORD	read_cnt;
		DWORD	new_cnt;
		int		bufidx = 0;
		int		j;
		BOOL	bLoopFlag = TRUE;

		//中断ダイアログ表示
		cDlgCancel.DoModeless( G_AppInstance(), m_hwndParent, IDD_EXECRUNNING );
		//実行したコマンドラインを表示
		// 2004.09.20 naoh 多少は見やすく・・・
		if (FALSE==bToEditWindow)	//	2006.12.03 maru アウトプットウィンドウにのみ出力
		{
			TCHAR szTextDate[1024], szTextTime[1024];
			SYSTEMTIME systime;
			::GetLocalTime( &systime );
			CShareData::getInstance()->MyGetDateFormat( systime, szTextDate, _countof( szTextDate ) - 1 );
			CShareData::getInstance()->MyGetTimeFormat( systime, szTextTime, _countof( szTextTime ) - 1 );
			CShareData::getInstance()->TraceOut( _T("\r\n%ls\r\n"), _T("#============================================================") );
			CShareData::getInstance()->TraceOut( _T("#DateTime : %ls %ls\r\n"), szTextDate, szTextTime );
			CShareData::getInstance()->TraceOut( _T("#CmdLine  : %ls\r\n"), pszCmd );
			CShareData::getInstance()->TraceOut( _T("#%ls\r\n"), _T("============================================================") );
		}
		
		//charで読む
		typedef char PIPE_CHAR;
		PIPE_CHAR work[1024];
		
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
					//終了していればループフラグをFALSEとする
					//ただしループの終了条件は プロセス終了 && パイプが空
					bLoopFlag = FALSE;
					break;
				case WAIT_OBJECT_0 + 1:
					//処理中のユーザー操作を可能にする
					if( !::BlockingHook( cDlgCancel.GetHwnd() ) ){
						break;
					}
					break;
				default:
					break;
			}
			//中断ボタン押下チェック
			if( cDlgCancel.IsCanceled() ){
				//指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
				::TerminateProcess( pi.hProcess, 0 );
				if (!bToEditWindow) {	//	2006.12.03 maru アウトプットウィンドウにのみ出力
					//最後にテキストを追加
					CShareData::getInstance()->TraceOut( _T("%ts"), _T("\r\n中断しました。\r\n") );
				}
				break;
			}
			new_cnt = 0;

			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ) {	//パイプの中の読み出し待機中の文字数を取得
				while( new_cnt > 0 ) {												//待機中のものがある

					if( new_cnt >= _countof(work)-2 - bufidx) {							//パイプから読み出す量を調整
						new_cnt = _countof(work)-2 - bufidx;
					}
					::ReadFile( hStdOutRead, &work[bufidx], new_cnt, &read_cnt, NULL );	//パイプから読み出し
					read_cnt += bufidx;													//work内の実際のサイズにする

					if( read_cnt == 0 ) {
						// Jan. 23, 2004 genta while追加のため制御を変更
						break;
					}
					// Unicode で データを受け取る start 2008/6/8 Uchi
					if (cOsVer.IsWin32NT()) {
						wchar_t*	workw;
						int			read_cntw;
						bool		bCarry;
						workw = (wchar_t*)work;
						read_cntw = (int)read_cnt/sizeof(wchar_t);
						workw[read_cntw] = '\0';
						bCarry = false;
						//読み出した文字列をチェックする
						if (workw[read_cntw-1] == L'\r') {
							bCarry = true;
							read_cntw -= sizeof(wchar_t);
							workw[read_cntw] = '\0';
						}
						if (FALSE==bToEditWindow) {
							CShareData::getInstance()->TraceOut( L"%s", workw );
						} else {
							GetCommander().Command_INSTEXT(FALSE, workw, CLogicInt(-1), TRUE);
						}
						bufidx = 0;
						if (bCarry) {
							workw[0] = L'r';
							bufidx = sizeof(wchar_t);
							DBPRINT_A( "ExecCmd: Carry last character [CR]\n");
						}
					}
					// end 2008/6/8 Uchi
					else {
						//読み出した文字列をチェックする
						// \r\n を \r だけとか漢字の第一バイトだけを出力するのを防ぐ必要がある
						//@@@ 2002.1.24 YAZAKI 1バイト取りこぼす可能性があった。
						//	Jan. 28, 2004 Moca 最後の文字はあとでチェックする
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
							//	2006.12.03 maru アウトプットウィンドウor編集中のウィンドウ分岐追加
							if (FALSE==bToEditWindow) {
								work[read_cnt] = '\0';
								CShareData::getInstance()->TraceOut( _T("%hs"), work );
							} else {
								GetCommander().Command_INSTEXT(FALSE, to_wchar(work,read_cnt), CLogicInt(-1), TRUE);
							}
							bufidx = 0;
						}
						else {
							char tmp = work[read_cnt-1];
							//	2006.12.03 maru アウトプットウィンドウor編集中のウィンドウ分岐追加
							if (FALSE==bToEditWindow) {
								work[read_cnt-1] = '\0';
								CShareData::getInstance()->TraceOut( _T("%hs"), work );
							} else {
								GetCommander().Command_INSTEXT(FALSE, to_wchar(work,read_cnt-1), CLogicInt(-1), TRUE);
							}
							work[0] = tmp;
							bufidx = 1;
							DBPRINT_A( "ExecCmd: Carry last character [%d]\n", tmp );
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
				}
			}
		} while( bLoopFlag || new_cnt > 0 );
		
		if (!bToEditWindow) {	//	2006.12.03 maru アウトプットウィンドウにのみ出力
			work[bufidx] = '\0';
			CShareData::getInstance()->TraceOut( _T("%hs"), work );	/* 最後の文字の処理 */
			//	Jun. 04, 2003 genta	終了コードの取得と出力
			DWORD result;
			::GetExitCodeProcess( pi.hProcess, &result );
			CShareData::getInstance()->TraceOut( _T("\r\n終了コード: %d\r\n"), result );

			// 2004.09.20 naoh 終了コードが1以上の時はアウトプットをアクティブにする
			if(result > 0) ActivateFrameWindow( GetDllShareData().m_hwndDebug );
		}
		else {						//	2006.12.03 maru 編集中のウィンドウに出力時は最後に再描画
			GetCommander().Command_INSTEXT(FALSE, to_wchar(work,bufidx), CLogicInt(-1), TRUE);	/* 最後の文字の処理 */
			if (bBeforeTextSelected){	// 挿入された部分を選択状態に
				GetSelectionInfo().SetSelectArea(
					CLayoutRange(
						CLayoutPoint(nColmFrom, nLineFrom),
						GetCaret().GetCaretLayoutPos()// CLayoutPoint(m_nCaretPosY, m_nCaretPosX )
					)
				);
				GetSelectionInfo().DrawSelectArea();
			}
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
