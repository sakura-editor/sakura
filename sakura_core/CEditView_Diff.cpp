//	$Id$
/*!	@file
	@brief DIFF差分表示

	@author MIK
	@date	2002/05/25 ExecCmd を参考にDIFF実行結果を取り込む処理作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, GAE, YAZAKI, hor
	Copyright (C) 2002, hor, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "sakura_rc.h"
#include "etc_uty.h"
#include "global.h"
//#include "CDlgCancel.h"
#include "CDlgDiff.h"
#include "CEditDoc.h"
#include "CEditView.h"
#include "CDocLine.h"
#include "CDocLineMgr.h"
#include "CWaitCursor.h"
#include "debug.h"

/*!	差分表示
	@note	HandleCommandからの呼び出し対応(ダイアログあり版)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::Command_Diff_Dialog( void )
{
	CDlgDiff	cDlgDiff;
//	bool	bTmpFile1, bTmpFile2;
	char	szTmpFile1[_MAX_PATH];
	char	szTmpFile2[_MAX_PATH];

	//DIFF差分表示ダイアログを表示する
	if( FALSE == cDlgDiff.DoModal( m_hInstance, m_hWnd, (LPARAM)m_pcEditDoc,
		m_pcEditDoc->m_szFilePath,
		m_pcEditDoc->IsModified() ) )
	{
		return;
	}

	//自ファイル
	if( m_pcEditDoc->IsModified() )
	{
		MessageBox( NULL,  
			"差分コマンド実行は失敗しました。\nファイルを保存してから行ってください。", 
			"DIFF差分表示",
			MB_OK | MB_ICONEXCLAMATION );
		return;
/*
#if 0
		if( tmpnam( szTmpFile1 ) == NULL )
		{
			MessageBox( NULL,  
				"差分コマンド実行は失敗しました。", 
				"DIFF差分表示",
				MB_OK | MB_ICONEXCLAMATION );
			return;
		}

		bTmpFile1 = true;

		// 編集中ファイルを一時ファイルに保存する
		if( MakeDiffTmpFile( szTmpFile1, NULL ) == FALSE )
		{
			MessageBox( NULL,  
				"差分コマンド実行は失敗しました。", 
				"DIFF差分表示",
				MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
#endif
*/
	}
	else
	{
//		bTmpFile1 = false;
		strcpy( szTmpFile1, m_pcEditDoc->m_szFilePath );
		if( strlen( szTmpFile1 ) == 0 ) return;
	}

	//相手ファイル
	if( cDlgDiff.m_bIsModifiedDst )	/* 相手先ファイルは編集中か？ */
	{
		MessageBox( NULL,  
			"差分コマンド実行は失敗しました。\nファイルを保存してから行ってください。", 
			"DIFF差分表示",
			MB_OK | MB_ICONEXCLAMATION );
		return;
/*
#if 0
		if( tmpnam( szTmpFile2 ) == NULL )
		{
			MessageBox( NULL,  
				"差分コマンド実行は失敗しました。", 
				"DIFF差分表示",
				MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}

		bTmpFile2 = true;

		// 相手先編集中ファイルを一時ファイルに保存する
		if( MakeDiffTmpFile( szTmpFile2, cDlgDiff.m_hWnd_Dst ) == FALSE )
		{
			MessageBox( NULL,  
				"差分コマンド実行は失敗しました。", 
				"DIFF差分表示",
				MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
#endif
*/
	}
	else
	{
//		bTmpFile2 = false;
		strcpy( szTmpFile2, cDlgDiff.m_szFile2 );
		if( strlen( szTmpFile2 ) == 0 ) return;
	}

	/* 差分表示 */
	Command_Diff( szTmpFile1, szTmpFile2, cDlgDiff.m_nDiffFlgOpt );

/*
finish:;
	//一時ファイルを削除する
	if( bTmpFile1 ) unlink( szTmpFile1 );
	if( bTmpFile2 ) unlink( szTmpFile2 );
*/

	return;
}

/*!	差分表示
	@param	pszFile1	[in]	旧ファイル名
	@param	pszFile2	[in]	新ファイル名
//	@param	nFlgFile12	[in]	編集中ファイルは...
//									0	ファイル1(旧ファイル)
//									1	ファイル2(新ファイル)
	@param	nFlgOpt		[in]	0b000000000
							      ||||+--- -i ignore-case         大文字小文字同一視
							      |||+---- -w ignore-all-space    空白無視
							      ||+----- -b ignore-space-change 空白変更無視
							      |+------ -B ignore-blank-lines  空行無視
							      +------- -t expand-tabs         TAB-SPACE変換
	@note	HandleCommandからの呼び出し対応(ダイアログなし版)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::Command_Diff( 
	const char	*pszFile1,
	const char	*pszFile2,
	/* int			nFlgFile12, */
	int			nFlgOpt )
/*
	bool	bFlgCase,		//大文字小文字同一視
	bool	bFlgBlank,		//空白無視
	bool	bFlgWhite,		//空白変更無視
	bool	bFlgBLine,		//空行無視
	bool	bFlgTabSpc,		//TAB-SPACE変換
*/
{
	char	cmdline[1024];
	HANDLE	hStdOutWrite, hStdOutRead;
//	CDlgCancel	cDlgCancel;
	CWaitCursor	cWaitCursor( m_hWnd );
	int		nFlgFile12 = 1;

	//今あるDIFF差分を消去する。
	if( m_pcEditDoc->m_cDocLineMgr.IsDiffUse() )
		Command_Diff_Reset();
		//m_pcEditDoc->m_cDocLineMgr.ResetAllDiffMark();

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );

	//子プロセスの標準出力と接続するパイプを作成
	SECURITY_ATTRIBUTES	sa;
	ZeroMemory( &sa, sizeof(SECURITY_ATTRIBUTES) );
	sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle       = TRUE;
	sa.lpSecurityDescriptor = NULL;
	hStdOutRead = hStdOutWrite = 0;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE )
	{
		//エラー。対策無し
		return;
	}

	//継承不能にする
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), NULL,
				0, FALSE, DUPLICATE_SAME_ACCESS );

	//CreateProcessに渡すSTARTUPINFOを作成
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(STARTUPINFO) );
	sui.cb          = sizeof(STARTUPINFO);
	sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sui.wShowWindow = SW_HIDE;
	sui.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
	sui.hStdOutput  = hStdOutWrite;
	sui.hStdError   = hStdOutWrite;

	//オプションを作成する
	char	szOption[16];	// "-cwbBt"
	strcpy( szOption, "-" );
	if( nFlgOpt & 0x0001 ) strcat( szOption, "i" );	//-i ignore-case         大文字小文字同一視
	if( nFlgOpt & 0x0002 ) strcat( szOption, "w" );	//-w ignore-all-space    空白無視
	if( nFlgOpt & 0x0004 ) strcat( szOption, "b" );	//-b ignore-space-change 空白変更無視
	if( nFlgOpt & 0x0008 ) strcat( szOption, "B" );	//-B ignore-blank-lines  空行無視
	if( nFlgOpt & 0x0010 ) strcat( szOption, "t" );	//-t expand-tabs         TAB-SPACE変換
	if( strcmp( szOption, "-" ) == 0 ) strcpy( szOption, "" );	//オプションなし

	/* exeのあるフォルダ */
	char	szPath[_MAX_PATH + 1];
	char	szExeFolder[_MAX_PATH + 1];
	::GetModuleFileName(
		::GetModuleHandle( NULL ),
		szPath, sizeof( szPath )
	);
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, szExeFolder, NULL );

	//コマンドライン文字列作成(MAX:1024)
	wsprintf( cmdline, "%s\\%s %s \"%s\" \"%s\"",
			szExeFolder,	//sakura.exeパス
			"diff.exe",		//diff.exe
			szOption,		//diffオプション
			( nFlgFile12 ? pszFile2 : pszFile1 ),
			( nFlgFile12 ? pszFile1 : pszFile2 )
		);

	//コマンドライン実行
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
			CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE )
	{
		MessageBox( NULL, cmdline, "差分コマンド実行は失敗しました。", MB_OK | MB_ICONEXCLAMATION );
		goto finish;
	}

	{
		DWORD	read_cnt;
		DWORD	new_cnt;
		char	work[1024];
		int		j;
		bool	bLoopFlag = true;
		bool	bLineHead = true;	//行頭か
		bool	bDiffInfo = false;	//DIFF情報か
		int		nDiffLen = 0;		//DIFF情報長
		char	szDiffData[100];	//DIFF情報

		//中断ダイアログ表示
//		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );

		//実行結果の取り込み
		do {
			//処理中のユーザー操作を可能にする
//			if( !::BlockingHook( cDlgCancel.m_hWnd ) )
//			{
//				bDiffInfo = false;
//				break;
//			}

			//中断ボタン押下チェック
//			if( cDlgCancel.IsCanceled() )
//			{
				//指定されたプロセスと、そのプロセスが持つすべてのスレッドを終了させます。
//				::TerminateProcess( pi.hProcess, 0 );
//				bDiffInfo = false;
//				break;
//			}

			//プロセスが終了していないか確認
			if( WaitForSingleObject( pi.hProcess, 0 ) == WAIT_OBJECT_0 )
			{
				//終了していればループフラグをFALSEとする
				//ただしループの終了条件は プロセス終了 && パイプが空
				bLoopFlag = FALSE;
			}

			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) )	//パイプの中の読み出し待機中の文字数を取得
			{
				if( new_cnt > 0 )												//待機中のものがある
				{
					if( new_cnt >= sizeof(work) - 2 )							//パイプから読み出す量を調整
					{
						new_cnt = sizeof(work) - 2;
					}
					ReadFile( hStdOutRead, &work[0], new_cnt, &read_cnt, NULL );	//パイプから読み出し
					if( read_cnt == 0 )
					{
						continue;
					}

					//読み出した文字列をチェックする
					for( j = 0; j < (int)read_cnt/*-1*/; j++ )
					{
						if( bLineHead )
						{
							if( work[j] != '\n' && work[j] != '\r' )
							{
								bLineHead = false;
							
								//DIFF情報の始まりか？
								if( work[j] >= '0' && work[j] <= '9' )
								{
									bDiffInfo = true;
									nDiffLen = 0;
									szDiffData[nDiffLen++] = work[j];
								}
								/*
								else if( work[j] == '<' || work[j] == '>' || work[j] == '-' )
								{
									bDiffInfo = false;
									nDiffLen = 0;
								}
								*/
							}
						}
						else
						{
							//行末に達したか？
							if( work[j] == '\n' || work[j] == '\r' )
							{
								//DIFF情報があれば解析する
								if( bDiffInfo == true && nDiffLen > 0 )
								{
									szDiffData[nDiffLen] = '\0';
									AnalyzeDiffInfo( szDiffData, nFlgFile12 );
									nDiffLen = 0;
								}
								
								bDiffInfo = false;
								bLineHead = true;
							}
							else if( bDiffInfo == true )
							{
								//DIFF情報に追加する
								szDiffData[nDiffLen++] = work[j];
								if( nDiffLen >= 99 )
								{
									nDiffLen = 0;
									bDiffInfo = false;
								}
							}
						}
					}
				}
			}
			Sleep(0);
		} while( bLoopFlag || new_cnt > 0 );

		//残ったDIFF情報があれば解析する
		if( bDiffInfo == true && nDiffLen > 0 )
		{
			szDiffData[nDiffLen] = '\0';
			AnalyzeDiffInfo( szDiffData, nFlgFile12 );
		}
	}


finish:
	//終了処理
	CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead  );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread  ) CloseHandle( pi.hThread  );

	//分割したビューも更新
	for( int v = 0; v < 4; ++v )
		if( m_pcEditDoc->m_nActivePaneIndex != v )
			m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();

	return;
}

/*!	DIFF差分情報を解析しマーク登録
	@param	pszFile2	[in]	新ファイル名
	@param	nFlgFile12	[in]	編集中ファイルは...
									0	ファイル1(旧ファイル)
									1	ファイル2(新ファイル)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::AnalyzeDiffInfo( 
	char	*pszDiffInfo,
	int		nFlgFile12 )
{
	/*
	 * 99a99		旧ファイル99行の次行に新ファイル99行が追加された。
	 * 99a99,99		旧ファイル99行の次行に新ファイル99～99行が追加された。
	 * 99c99		旧ファイル99行が新ファイル99行に変更された。
	 * 99,99c99,99	旧ファイル99～99行が新ファイル99～99行に変更された。
	 * 99d99		旧ファイル99行が新ファイル99行の次行から削除された。
	 * 99,99d99		旧ファイル99～99行が新ファイル99行の次行から削除された。
	 * s1,e1 mode s2,e2
	 * 先頭の場合0の次行となることもある
	 */
	char	*q;
	int		s1, e1, s2, e2;
	char	mode;

	//前半ファイルの開始行
	s1 = 0;
	for( q = pszDiffInfo; *q; q++ )
	{
		if( *q == ',' ) break;
		if( *q == 'a' || *q == 'c' || *q == 'd' ) break;
		//行番号を抽出
		if( *q >= '0' && *q <= '9' ) s1 = s1 * 10 + (*q - '0');
		else return;
	}
	if( ! *q ) return;

	//前半ファイルの終了行
	if( *q != ',' )
	{
		//開始・終了行番号は同じ
		e1 = s1;
	}
	else
	{
		e1 = 0;
		for( q++; *q; q++ )
		{
			if( *q == 'a' || *q == 'c' || *q == 'd' ) break;
			//行番号を抽出
			if( *q >= '0' && *q <= '9' ) e1 = e1 * 10 + (*q - '0');
			else return;
		}
	}
	if( ! *q ) return;

	//DIFFモードを取得
	mode = *q;

	//後半ファイルの開始行
	s2 = 0;
	for( q++; *q; q++ )
	{
		if( *q == ',' ) break;
		//行番号を抽出
		if( *q >= '0' && *q <= '9' ) s2 = s2 * 10 + (*q - '0');
		else return;
	}

	//後半ファイルの終了行
	if( *q != ',' )
	{
		//開始・終了行番号は同じ
		e2 = s2;
	}
	else
	{
		e2 = 0;
		for( q++; *q; q++ )
		{
			//行番号を抽出
			if( *q >= '0' && *q <= '9' ) e2 = e2 * 10 + (*q - '0');
			else return;
		}
	}

	//行末に達してなければエラー
	if( *q ) return;

	//抽出したDIFF情報から行番号に差分マークを付ける
	if( 0 == nFlgFile12 )	//編集中ファイルは旧ファイル
	{
		if     ( mode == 'a' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_DELETE, s1    , e1     );
		else if( mode == 'c' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_CHANGE, s1 - 1, e1 - 1 );
		else if( mode == 'd' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_APPEND, s1 - 1, e1 - 1 );
	}
	else	//編集中ファイルは新ファイル
	{
		if     ( mode == 'a' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_APPEND, s2 - 1, e2 - 1 );
		else if( mode == 'c' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_CHANGE, s2 - 1, e2 - 1 );
		else if( mode == 'd' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_DELETE, s2    , e2     );
	}

	return;
}

/*!	次の差分を探し，見つかったら移動する
*/
void CEditView::Command_Diff_Next( void )
{
	int			nX = 0;
	int			nY;
	int			nYOld;
	BOOL		bFound = FALSE;
	BOOL		bRedo = TRUE;

	nY = m_nCaretPosY_PHY;
	nYOld = nY;

re_do:;	
	if( m_pcEditDoc->m_cDocLineMgr.SearchDiffMark( nY, 1 /* 後方検索 */, &nY ) )
	{
		bFound = TRUE;
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nX, nY, &nX, &nY );
		if( m_bSelectingLock )
		{
			if( !IsTextSelected() ) BeginSelectArea();
		}
		else
		{
			if( IsTextSelected() ) DisableSelectArea( TRUE );
		}
		MoveCursor( nX, nY, TRUE );
		if( m_bSelectingLock )
		{
			ChangeSelectAreaByCurrentCursor( nX, nY );
		}
	}

	if( m_pShareData->m_Common.m_bSearchAll )
	{
		if( !bFound		// 見つからなかった
		 && bRedo )		// 最初の検索
		{
			nY = 0 - 1;	/* 1個手前を指定 */
			bRedo = FALSE;
			goto re_do;		// 先頭から再検索
		}
	}
	if( bFound )
	{
		if( nYOld >= nY ) SendStatusMessage( "▼先頭から再検索しました" );
	}
	else
	{
		SendStatusMessage( "▽見つかりませんでした" );
		if( m_pShareData->m_Common.m_bNOTIFYNOTFOUND )	/* 見つからないときメッセージを表示 */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"後方(↓) に差分が見つかりません。" );
	}

	return;
}



/*!	前の差分を探し，見つかったら移動する
*/
void CEditView::Command_Diff_Prev( void )
{
	int			nX = 0;
	int			nY;
	int			nYOld;
	BOOL		bFound = FALSE;
	BOOL		bRedo = TRUE;

	nY = m_nCaretPosY_PHY;
	nYOld = nY;

re_do:;
	if( m_pcEditDoc->m_cDocLineMgr.SearchDiffMark( nY, 0 /* 前方検索 */, &nY ) )
	{
		bFound = TRUE;
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nX, nY, &nX, &nY );
		if( m_bSelectingLock )
		{
			if( !IsTextSelected() ) BeginSelectArea();
		}
		else
		{
			if( IsTextSelected() ) DisableSelectArea( TRUE );
		}
		MoveCursor( nX, nY, TRUE );
		if( m_bSelectingLock )
		{
			ChangeSelectAreaByCurrentCursor( nX, nY );
		}
	}

	if( m_pShareData->m_Common.m_bSearchAll )
	{
		if( !bFound	// 見つからなかった
		 && bRedo )	// 最初の検索
		{
			nY = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 + 1;	/* 1個手前を指定 */
			bRedo = FALSE;
			goto re_do;	// 末尾から再検索
		}
	}
	if( bFound )
	{
		if( nYOld <= nY ) SendStatusMessage( "▲末尾から再検索しました" );
	}
	else
	{
		SendStatusMessage( "△見つかりませんでした" );
		if( m_pShareData->m_Common.m_bNOTIFYNOTFOUND )	/* 見つからないときメッセージを表示 */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"前方(↑) に差分が見つかりません。" );
	}

	return;
}

/*!	差分表示の全解除
	@author	MIK
	@date	2002/05/26
*/
void CEditView::Command_Diff_Reset( void )
{
	m_pcEditDoc->m_cDocLineMgr.ResetAllDiffMark();

	//分割したビューも更新
	for( int v = 0; v < 4; ++v )
		if( m_pcEditDoc->m_nActivePaneIndex != v )
			m_pcEditDoc->m_cEditViewArr[v].Redraw();
	Redraw();
	return;
}

/*!	一時ファイルを作成する
	@author	MIK
	@date	2002/05/26
*/
/*
BOOL CEditView::MakeDiffTmpFile( char* filename, HWND hWnd )
{
#if 0
	char*	pLineData;
	int		nLineLen;
	int		y;

	// 行(改行単位)データの要求
	if( hWnd )
	{
		pLineData = m_pShareData->m_szWork;
		nLineLen = ::SendMessage( hWnd, MYWM_GETLINEDATA, y, 0 );
	}
	else
	{
		pLineData = m_pcEditDoc->m_cDocLineMgr.GetLineStr( y, &nLineLenSrc );
	}

	while( 1 )
	{
		if( hWnd && nLineLen > sizeof( m_pShareData->m_szWork ) )
		{
			// 一時バッファを超えている
		}

		// 行(改行単位)データの要求 
		if( hWnd ) nLineLen = ::SendMessage( hWnd, MYWM_GETLINEDATA, y, 0 );
		else       pLineData = m_pcEditDoc->m_cDocLineMgr.GetLineStr( y, &nLineLen );
		
	}
#endif

	return FALSE;
}
*/


