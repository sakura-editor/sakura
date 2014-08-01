/*!	@file
	@brief 文書ウィンドウの管理

	@author Norio Nakatani
	@date	1998/03/13 作成
	@date   2005/09/02 D.S.Koba GetSizeOfCharで書き換え
*/
/*
	Copyright (C) 1998-2002, Norio Nakatani
	Copyright (C) 2000, genta, JEPRO, MIK
	Copyright (C) 2001, genta, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca, minfu, KK, novice, ai, Azumaiya, genta
	Copyright (C) 2003, MIK, ai, ryoji, Moca, wmlhq, genta
	Copyright (C) 2004, genta, Moca, novice, naoh, isearch, fotomo
	Copyright (C) 2005, genta, MIK, novice, aroka, D.S.Koba, かろと, Moca
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta, maru
	Copyright (C) 2007, ryoji, じゅうじ, maru, genta, Moca, nasukoji, D.S.Koba
	Copyright (C) 2008, ryoji, nasukoji, bosagami, Moca, genta
	Copyright (C) 2009, nasukoji, ryoji, syat
	Copyright (C) 2010, ryoji, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CEditView.h"
#include "Debug.h"
#include "charcode.h"
#include "CEditWnd.h"
#include "CDlgCancel.h"
#include "etc_uty.h"

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
	@date	2009/02/21	ryoji	編集禁止のときは編集中ウィンドウへは出力しない（指定時はアウトプットへ）
*/
void CEditView::ExecCmd( const char* pszCmd, const int nFlgOpt )
{
	char				cmdline[1024];
	HANDLE				hStdOutWrite, hStdOutRead, hStdIn;
	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
	CDlgCancel				cDlgCancel;

	bool bEditable = m_pcEditDoc->IsEditable();	// 編集禁止

	//	From Here 2006.12.03 maru 引数を拡張のため
	BOOL	bGetStdout;			//	子プロセスの標準出力を得る
	BOOL	bToEditWindow;		//	TRUE=編集中のウィンドウ / FALSAE=アウトプットウィンドウ
	BOOL	bSendStdin;			//	編集中ファイルを子プロセスSTDINに渡す
	
	bGetStdout = nFlgOpt & 0x01 ? TRUE : FALSE;
	bToEditWindow = ((nFlgOpt & 0x02) && bEditable) ? TRUE : FALSE;
	bSendStdin = nFlgOpt & 0x04 ? TRUE : FALSE;
	//	To Here 2006.12.03 maru 引数を拡張のため

	// 編集中のウィンドウに出力する場合の選択範囲処理用	/* 2007.04.29 maru */
	CLayoutPoint ptFrom = CLayoutPoint( 0, 0 );
	BOOL	bBeforeTextSelected = IsTextSelected();
	if (bBeforeTextSelected){
		ptFrom = m_sSelect.m_ptFrom;
	}
	
	//子プロセスの標準出力と接続するパイプを作成
	SECURITY_ATTRIBUTES	sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
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
		DEBUG_TRACE( _T("CEditView::ExecCmd() TempFilename=[%s]\n"), szTempFileName );
		
		nFlgOpt = bBeforeTextSelected ? 0x01 : 0x00;		/* 選択範囲を出力 */
		
		if( FALSE == Command_PUTFILE( szTempFileName, CODE_SJIS, nFlgOpt) ){	// 一時ファイル出力
			hStdIn = NULL;
		} else {
			hStdIn = CreateFile( szTempFileName, GENERIC_READ,		// 子プロセスへの継承用にファイルを開く
					0, &sa, OPEN_EXISTING,
					FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
					NULL );
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
	ZeroMemory( &sui, sizeof(STARTUPINFO) );
	sui.cb = sizeof(STARTUPINFO);
	if( bGetStdout || bSendStdin ) {
		sui.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
		sui.wShowWindow = bGetStdout ? SW_HIDE : SW_SHOW;
		sui.hStdInput = hStdIn;
		sui.hStdOutput = bGetStdout ? hStdOutWrite : GetStdHandle( STD_OUTPUT_HANDLE );
		sui.hStdError = bGetStdout ? hStdOutWrite : GetStdHandle( STD_ERROR_HANDLE );
	}

	//コマンドライン実行
	strcpy( cmdline, pszCmd );
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
				CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
		//実行に失敗した場合、コマンドラインベースのアプリケーションと判断して
		// command(9x) か cmd(NT) を呼び出す

		// 2010.08.27 Moca システムディレクトリ付加
		TCHAR szCmdDir[_MAX_PATH];
		if( IsWin32NT() ){
			::GetSystemDirectory(szCmdDir, sizeof(szCmdDir));
		}else{
			::GetWindowsDirectory(szCmdDir, sizeof(szCmdDir));
		}

		//コマンドライン文字列作成
		wsprintf( cmdline, "\"%s\\%s\" %s%s",
				szCmdDir,
				( IsWin32NT() ? "cmd.exe" : "command.com" ),
				( bGetStdout ? "/C " : "/K " ), pszCmd );
		if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
					CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
			MessageBox( NULL, cmdline, "コマンド実行は失敗しました。", MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
	}

	// ファイル全体に対するフィルタ動作
	//	現在編集中のファイルからのデータ書きだしおよびデータ取り込みが
	//	指定されていて，かつ範囲選択が行われていない場合は
	//	「すべて選択」されているものとして，編集データ全体を
	//	コマンドの出力結果と置き換える．
	//	2007.05.20 maru
	if((FALSE == bBeforeTextSelected) && bSendStdin && bGetStdout && bToEditWindow){
		CLayoutRange sRange = CLayoutRange( CLayoutPoint( 0, 0 ), CLayoutPoint( 0, m_pcEditDoc->m_cLayoutMgr.GetLineCount()) );
		SetSelectArea( sRange );
		DeleteData( true );
	}

	// hStdOutWrite は CreateProcess() で継承したので親プロセスでは用済み
	// hStdInも親プロセスでは使用しないが、Win9x系では子プロセスが終了してから
	// クローズするようにしないと一時ファイルが自動削除されない
	CloseHandle(hStdOutWrite);
	hStdOutWrite = NULL;	// 2007.09.08 genta 二重closeを防ぐ

	if( bGetStdout ) {
		DWORD	read_cnt;
		DWORD	new_cnt;
		char	work[1024];
		int		bufidx = 0;
		int		j;
		BOOL	bLoopFlag = TRUE;

		//中断ダイアログ表示
		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );
		//実行したコマンドラインを表示
		// 2004.09.20 naoh 多少は見やすく・・・
		if (FALSE==bToEditWindow) {	//	2006.12.03 maru アウトプットウィンドウにのみ出力
			TCHAR szTextDate[1024], szTextTime[1024];
			SYSTEMTIME systime;
			::GetLocalTime( &systime );
			CShareData::getInstance()->MyGetDateFormat( systime, szTextDate, _countof( szTextDate ) - 1 );
			CShareData::getInstance()->MyGetTimeFormat( systime, szTextTime, _countof( szTextTime ) - 1 );
			CShareData::getInstance()->TraceOut( "\r\n%s\r\n", "#============================================================" );
			CShareData::getInstance()->TraceOut( "#DateTime : %s %s\r\n", szTextDate, szTextTime );
			CShareData::getInstance()->TraceOut( "#CmdLine  : %s\r\n", pszCmd );
			CShareData::getInstance()->TraceOut( "#%s\r\n", "==============================" );
		}
		
		
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
					if( !::BlockingHook( cDlgCancel.m_hWnd ) ){
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
				if (FALSE==bToEditWindow) {	//	2006.12.03 maru アウトプットウィンドウにのみ出力
					//最後にテキストを追加
					const char* pszText = "\r\n中断しました。\r\n";
					CShareData::getInstance()->TraceOut( "%s", pszText );
				}
				break;
			}
			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ) {	//パイプの中の読み出し待機中の文字数を取得
				while( new_cnt > 0 ) {												//待機中のものがある
					if( new_cnt >= sizeof(work)-2 ) {							//パイプから読み出す量を調整
						new_cnt = sizeof(work)-2;
					}
					ReadFile( hStdOutRead, &work[bufidx], new_cnt, &read_cnt, NULL );	//パイプから読み出し
					read_cnt += bufidx;													//work内の実際のサイズにする
					
					if( read_cnt == 0 ) {
						// Jan. 23, 2004 genta while追加のため制御を変更
						break;
					}
					//読み出した文字列をチェックする
					// \r\n を \r だけとか漢字の第一バイトだけを出力するのを防ぐ必要がある
					//@@@ 2002.1.24 YAZAKI 1バイト取りこぼす可能性があった。
					//	Jan. 28, 2004 Moca 最後の文字はあとでチェックする
					for( j=0; j<(int)read_cnt - 1; j++ ) {
						//	2007.09.10 ryoji
						if( CMemory::GetSizeOfChar(work, read_cnt, j) == 2 ) {
							j++;
						} else {
							if( work[j] == '\r' && work[j+1] == '\n' ) {
								j++;
							} else if( work[j] == '\n' && work[j+1] == '\r' ) {
								j++;
							}
						}
					}
					//	From Here Jan. 28, 2004 Moca
					//	改行コードが分割されるのを防ぐ
					if( j == read_cnt - 1 ){
						if( _IS_SJIS_1(work[j]) ) {
							j = read_cnt + 1; // ぴったり出力できないことを主張
						}else if( work[j] == '\r' || work[j] == '\n' ) {
							// CRLF,LFCRの一部ではない改行が末尾にある
							// 次の読み込みで、CRLF,LFCRの一部になる可能性がある
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
							CShareData::getInstance()->TraceOut( "%s", work );
						} else {
							Command_INSTEXT( false, work, read_cnt, TRUE);
						}
						bufidx = 0;
						//DEBUG_TRACE( _T("ExecCmd: No leap character\n"));
					} else {
						char tmp = work[read_cnt-1];
						//	2006.12.03 maru アウトプットウィンドウor編集中のウィンドウ分岐追加
						if (FALSE==bToEditWindow) {
							work[read_cnt-1] = '\0';
							CShareData::getInstance()->TraceOut( "%s", work );
						} else {
							Command_INSTEXT( false, work, read_cnt-1, TRUE);
						}
						work[0] = tmp;
						bufidx = 1;
						DEBUG_TRACE( _T("ExecCmd: Carry last character [%d]\n"), tmp );
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
		
		if (FALSE==bToEditWindow) {	//	2006.12.03 maru アウトプットウィンドウにのみ出力
			work[bufidx] = '\0';
			CShareData::getInstance()->TraceOut( "%s", work );	/* 最後の文字の処理 */
			//	Jun. 04, 2003 genta	終了コードの取得と出力
			DWORD result;
			::GetExitCodeProcess( pi.hProcess, &result );
			CShareData::getInstance()->TraceOut( "\r\n終了コード: %d\r\n", result );

			// 2004.09.20 naoh 終了コードが1以上の時はアウトプットをアクティブにする
			if(result > 0) ActivateFrameWindow( m_pShareData->m_sHandles.m_hwndDebug );
		}
		else {						//	2006.12.03 maru 編集中のウィンドウに出力時は最後に再描画
			Command_INSTEXT( false, work, bufidx, TRUE);	/* 最後の文字の処理 */
			if (bBeforeTextSelected){	// 挿入された部分を選択状態に
				CLayoutRange sRange;
				sRange.m_ptFrom = ptFrom;
				sRange.m_ptTo = m_ptCaretPos;
				SetSelectArea( sRange );
				DrawSelectArea();
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

/*[EOF]*/
