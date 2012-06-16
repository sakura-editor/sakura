/*!	@file
	@brief DIFF差分表示

	@author MIK
	@date	2002/05/25 ExecCmd を参考にDIFF実行結果を取り込む処理作成
 	@date	2005/10/29	maru Diff差分表示処理を分離し、ダイアログあり版・ダイアログなし版の両方からコール
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, GAE, YAZAKI, hor
	Copyright (C) 2002, hor, MIK
	Copyright (C) 2003, MIK, ryoji, genta
	Copyright (C) 2004, genta
	Copyright (C) 2005, maru
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include "sakura_rc.h"
#include "etc_uty.h"
#include "global.h"
#include "CDlgDiff.h"
#include "CEditDoc.h"
#include "CEditView.h"
#include "CDocLine.h"
#include "CDocLineMgr.h"
#include "CWaitCursor.h"
#include "COsVersionInfo.h"
#include "mymessage.h"
#include "Debug.h"

#define	SAKURA_DIFF_TEMP_PREFIX	_T("sakura_diff_")

/*!	差分表示
	@note	HandleCommandからの呼び出し対応(ダイアログなし版)
	@author	maru
	@date	2005/10/28 これまでのCommand_DiffはViewDiffInfoに名称変更
*/
void CEditView::Command_Diff( const char *szTmpFile2, int nFlgOpt )
{
	bool	bTmpFile1 = false;
	TCHAR	szTmpFile1[_MAX_PATH * 2];

	if( -1 == ::GetFileAttributes( szTmpFile2 ) )
	{
		::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("差分コマンド実行は失敗しました。\n\n比較するファイルが見つかりません。") );
		return;
	}

	//自ファイル
	if (!m_pcEditDoc->IsModified()) _tcscpy( szTmpFile1, m_pcEditDoc->GetFilePath());
	else if (MakeDiffTmpFile ( szTmpFile1, NULL )) bTmpFile1 = true;
	else return;

	//差分表示
	ViewDiffInfo(szTmpFile1, szTmpFile2, nFlgOpt);

	//一時ファイルを削除する
	if( bTmpFile1 ) _tunlink( szTmpFile1 );

	return;

}

/*!	差分表示
	@note	HandleCommandからの呼び出し対応(ダイアログあり版)
	@author	MIK
	@date	2002/05/25
	@date	2002/11/09 編集中ファイルを許可
	@date	2005/10/29 maru 一時ファイル作成処理をMakeDiffTmpFileへ移動
*/
void CEditView::Command_Diff_Dialog( void )
{
	CDlgDiff	cDlgDiff;
	bool	bTmpFile1 = false, bTmpFile2 = false;

	//DIFF差分表示ダイアログを表示する
	int nDiffDlgResult = cDlgDiff.DoModal(
		m_hInstance,
		m_hWnd,
		(LPARAM)m_pcEditDoc,
		m_pcEditDoc->GetFilePath(),
		m_pcEditDoc->IsModified()
	);
	if( !nDiffDlgResult ){
		return;
	}
	
	//自ファイル
	TCHAR	szTmpFile1[_MAX_PATH * 2];
	if (!m_pcEditDoc->IsModified()) _tcscpy( szTmpFile1, m_pcEditDoc->GetFilePath());
	else if (MakeDiffTmpFile ( szTmpFile1, NULL )) bTmpFile1 = true;
	else return;
		
	//相手ファイル
	TCHAR	szTmpFile2[_MAX_PATH * 2];
	if (!cDlgDiff.m_bIsModifiedDst) _tcscpy( szTmpFile2, cDlgDiff.m_szFile2);
	else if (MakeDiffTmpFile ( szTmpFile2, cDlgDiff.m_hWnd_Dst )) bTmpFile2 = true;
	else 
	{
		if( bTmpFile1 ) _tunlink( szTmpFile1 );
		return;
	}
	
	//差分表示
	ViewDiffInfo(szTmpFile1, szTmpFile2, cDlgDiff.m_nDiffFlgOpt);
	
	
	//一時ファイルを削除する
	if( bTmpFile1 ) _tunlink( szTmpFile1 );
	if( bTmpFile2 ) _tunlink( szTmpFile2 );

	return;
}

/*!	差分表示
	@param	pszFile1	[in]	自ファイル名
	@param	pszFile2	[in]	相手ファイル名
    @param  nFlgOpt     [in]    0b000000000
                                    ||||||+--- -i ignore-case         大文字小文字同一視
                                    |||||+---- -w ignore-all-space    空白無視
                                    ||||+----- -b ignore-space-change 空白変更無視
                                    |||+------ -B ignore-blank-lines  空行無視
                                    ||+------- -t expand-tabs         TAB-SPACE変換
                                    |+--------    (編集中のファイルが旧ファイル)
                                    +---------    (DIFF差分がないときにメッセージ表示)
	@note	HandleCommandからの呼び出し対応(ダイアログなし版)
	@author	MIK
	@date	2002/05/25
	@date	2005/10/28	旧Command_Diffから関数名の変更。
						Command_Diff_Dialogだけでなく新Command_Diff
						からも呼ばれる関数。maru
*/
void CEditView::ViewDiffInfo( 
	const TCHAR*	pszFile1,
	const TCHAR*	pszFile2,
	int				nFlgOpt
)
/*
	bool	bFlgCase,		//大文字小文字同一視
	bool	bFlgBlank,		//空白無視
	bool	bFlgWhite,		//空白変更無視
	bool	bFlgBLine,		//空行無視
	bool	bFlgTabSpc,		//TAB-SPACE変換
	bool	bFlgFile12,		//編集中のファイルが旧ファイル
*/
{
	HANDLE	hStdOutWrite, hStdOutRead;

	CWaitCursor	cWaitCursor( m_hWnd );
	int		nFlgFile12 = 1;

	/* exeのあるフォルダ */
	TCHAR	szExeFolder[_MAX_PATH + 1];

	TCHAR	cmdline[1024];
	GetExedir( cmdline, _T("diff.exe") );
	SplitPath_FolderAndFile( cmdline, szExeFolder, NULL );

	//	From Here Dec. 28, 2002 MIK
	//	diff.exeの存在チェック
	if( -1 == ::GetFileAttributes( cmdline ) )
	{
		::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T( "差分コマンド実行は失敗しました。\n\nDIFF.EXE が見つかりません。" ) );
		return;
	}

	//今あるDIFF差分を消去する。
	if( m_pcEditDoc->m_cDocLineMgr.IsDiffUse() )
		Command_Diff_Reset();
		//m_pcEditDoc->m_cDocLineMgr.ResetAllDiffMark();

	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(pi) );

	//子プロセスの標準出力と接続するパイプを作成
	SECURITY_ATTRIBUTES	sa;
	ZeroMemory( &sa, sizeof(sa) );
	sa.nLength              = sizeof(sa);
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
	ZeroMemory( &sui, sizeof(sui) );
	sui.cb          = sizeof(sui);
	sui.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	sui.wShowWindow = SW_HIDE;
	sui.hStdInput   = GetStdHandle( STD_INPUT_HANDLE );
	sui.hStdOutput  = hStdOutWrite;
	sui.hStdError   = hStdOutWrite;

	//オプションを作成する
	TCHAR	szOption[16];	// "-cwbBt"
	_tcscpy( szOption, _T("-") );
	if( nFlgOpt & 0x0001 ) _tcscat( szOption, _T("i") );	//-i ignore-case         大文字小文字同一視
	if( nFlgOpt & 0x0002 ) _tcscat( szOption, _T("w") );	//-w ignore-all-space    空白無視
	if( nFlgOpt & 0x0004 ) _tcscat( szOption, _T("b") );	//-b ignore-space-change 空白変更無視
	if( nFlgOpt & 0x0008 ) _tcscat( szOption, _T("B") );	//-B ignore-blank-lines  空行無視
	if( nFlgOpt & 0x0010 ) _tcscat( szOption, _T("t") );	//-t expand-tabs         TAB-SPACE変換
	if( _tcscmp( szOption, _T("-") ) == 0 ) _tcscpy( szOption, _T("") );	//オプションなし
	if( nFlgOpt & 0x0020 ) nFlgFile12 = 0;
	else                   nFlgFile12 = 1;

	//	To Here Dec. 28, 2002 MIK

	//OSバージョン取得
	{
		// 2010.08.28 Moca システムディレクトリ付加
		TCHAR szCmdDir[_MAX_PATH];

		COsVersionInfo cOsVer;
		//コマンドライン文字列作成(MAX:1024)
		if (cOsVer.IsWin32NT()){
			::GetSystemDirectory(szCmdDir, _countof(szCmdDir));
			wsprintf(
				cmdline,
				_T("\"%s\\cmd.exe\" /C \"\"%s\\%s\" %s \"%s\" \"%s\"\""),
				szCmdDir,
				szExeFolder,	//sakura.exeパス
				_T("diff.exe"),		//diff.exe
				szOption,		//diffオプション
				( nFlgFile12 ? pszFile2 : pszFile1 ),
				( nFlgFile12 ? pszFile1 : pszFile2 )
			);
		}
		else{
			::GetWindowsDirectory(szCmdDir, _countof(szCmdDir));
			wsprintf(
				cmdline,
				_T("\"%s\\command.com\" /C \"%s\\%s\" %s \"%s\" \"%s\""),
				szCmdDir,
				szExeFolder,	//sakura.exeパス
				_T("diff.exe"),		//diff.exe
				szOption,		//diffオプション
				( nFlgFile12 ? pszFile2 : pszFile1 ),
				( nFlgFile12 ? pszFile1 : pszFile2 )
			);
		}
	}

	//コマンドライン実行
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
			CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE )
	{
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("差分コマンド実行は失敗しました。\n\n%s"), cmdline );
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
		bool	bFirst = true;	//先頭か？	//@@@ 2003.05.31 MIK

		//中断ダイアログ表示
//		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );

		//実行結果の取り込み
		do {
			//プロセスが終了していないか確認
			// Jul. 04, 2003 genta CPUを100%使い果たすのを防ぐため 200msec休む
			// Jan. 23, 2004 genta
			// 子プロセスの出力をどんどん受け取らないと子プロセスが
			// 停止してしまうため，待ち時間を200msから20msに減らす
			if( WaitForSingleObject( pi.hProcess, 20 ) == WAIT_OBJECT_0 )
			{
				//終了していればループフラグをFALSEとする
				//ただしループの終了条件は プロセス終了 && パイプが空
				bLoopFlag = FALSE;
			}

			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) )
			{
				while( new_cnt > 0 )												//待機中のものがある
				{
					if( new_cnt >= _countof(work) - 2 )							//パイプから読み出す量を調整
					{
						new_cnt = _countof(work) - 2;
					}
					::ReadFile( hStdOutRead, &work[0], new_cnt, &read_cnt, NULL );	//パイプから読み出し
					if( read_cnt == 0 )
					{
						// Jan. 23, 2004 genta while追加のため制御を変更
						break;
					}

					//@@@ 2003.05.31 MIK
					//	先頭がBinary filesならバイナリファイルのため意味のある差分が取られなかった
					if( bFirst )
					{
						bFirst = false;
						if( strncmp( work, "Binary files ", strlen( "Binary files " ) ) == 0 )
						{
							::MYMESSAGEBOX( NULL, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("DIFF差分を行おうとしたファイルはバイナリファイルです。") );
							goto finish;
						}
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
					// Jan. 23, 2004 genta
					// 子プロセスの出力をどんどん受け取らないと子プロセスが
					// 停止してしまうため，バッファが空になるまでどんどん読み出す．
					new_cnt = 0;
					if( ! PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ){
						break;
					}
					Sleep(0); // Jan. 23, 2004 genta タスクスイッチを促す
				}
			}
		} while( bLoopFlag || new_cnt > 0 );

		//残ったDIFF情報があれば解析する
		if( bDiffInfo == true && nDiffLen > 0 )
		{
			szDiffData[nDiffLen] = '\0';
			AnalyzeDiffInfo( szDiffData, nFlgFile12 );
		}
	}


	//DIFF差分が見つからなかったときにメッセージ表示
	if( nFlgOpt & 0x0040 )
	{
		if( !m_pcEditDoc->m_cDocLineMgr.IsDiffUse() )
		{
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME, _T("DIFF差分は見つかりませんでした。") );
		}
	}


finish:
	//終了処理
	CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead  );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread  ) CloseHandle( pi.hThread  );

	//分割したビューも更新
	m_pcEditDoc->Views_Redraw();

	return;
}

/*!	DIFF差分情報を解析しマーク登録
	@param	pszDiffInfo	[in]	新ファイル名
	@param	nFlgFile12	[in]	編集中ファイルは...
									0	ファイル1(旧ファイル)
									1	ファイル2(新ファイル)
	@author	MIK
	@date	2002/05/25
*/
void CEditView::AnalyzeDiffInfo( 
	const char*	pszDiffInfo,
	int			nFlgFile12
)
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
	const char	*q;
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
		if     ( mode == 'a' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_DELETE, CLogicInt(s1    ), CLogicInt(e1    ) );
		else if( mode == 'c' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_CHANGE, CLogicInt(s1 - 1), CLogicInt(e1 - 1) );
		else if( mode == 'd' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_APPEND, CLogicInt(s1 - 1), CLogicInt(e1 - 1) );
	}
	else	//編集中ファイルは新ファイル
	{
		if     ( mode == 'a' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_APPEND, CLogicInt(s2 - 1), CLogicInt(e2 - 1) );
		else if( mode == 'c' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_CHANGE, CLogicInt(s2 - 1), CLogicInt(e2 - 1) );
		else if( mode == 'd' ) m_pcEditDoc->m_cDocLineMgr.SetDiffMarkRange( MARK_DIFF_DELETE, CLogicInt(s2    ), CLogicInt(e2    ) );
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
		if( m_bSelectingLock ){
			if( !IsTextSelected() ) BeginSelectArea();
		}
		else{
			if( IsTextSelected() ) DisableSelectArea( TRUE );
		}
		MoveCursor( nX, nY, TRUE );
		if( m_bSelectingLock ){
			ChangeSelectAreaByCurrentCursor( nX, nY );
		}
	}


	if( m_pShareData->m_Common.m_bSearchAll ){
		// 見つからなかった。かつ、最初の検索
		if( !bFound	&& bRedo ){
			nY = 0 - 1;	// 1個手前を指定
			bRedo = FALSE;
			goto re_do;		// 先頭から再検索
		}
	}

	if( bFound ){
		if( nYOld >= nY ) SendStatusMessage( _T("▼先頭から再検索しました") );
	}
	else{
		SendStatusMessage( _T("▽見つかりませんでした") );
		if( m_pShareData->m_Common.m_bNOTIFYNOTFOUND )	/* 見つからないときメッセージを表示 */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME, _T("後方(↓) に差分が見つかりません。") );
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
	if( m_pcEditDoc->m_cDocLineMgr.SearchDiffMark( nY, 0 /* 前方検索 */, &nY ) ){
		bFound = TRUE;
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nX, nY, &nX, &nY );
		if( m_bSelectingLock ){
			if( !IsTextSelected() ) BeginSelectArea();
		}
		else{
			if( IsTextSelected() ) DisableSelectArea( TRUE );
		}

		MoveCursor( nX, nY, TRUE );
		if( m_bSelectingLock ){
			ChangeSelectAreaByCurrentCursor( nX, nY );
		}
	}

	if( m_pShareData->m_Common.m_bSearchAll ){
		// 見つからなかった、かつ、最初の検索
		if( !bFound	&& bRedo ){
			nY = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1 + 1;	// 1個手前を指定
			bRedo = FALSE;
			goto re_do;	// 末尾から再検索
		}
	}

	if( bFound ){
		if( nYOld <= nY ) SendStatusMessage( "▲末尾から再検索しました" );
	}
	else{
		SendStatusMessage( _T("△見つかりませんでした") );
		if( m_pShareData->m_Common.m_bNOTIFYNOTFOUND )	/* 見つからないときメッセージを表示 */
			::MYMESSAGEBOX( m_hWnd,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME, _T("前方(↑) に差分が見つかりません。") );
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
	m_pcEditDoc->Views_Redraw();
	return;
}

/*!	一時ファイルを作成する
	@author	MIK
	@date	2002/05/26
	@date	2005/10/29	引数変更const char* → char*
						一時ファイル名の取得処理もここでおこなう。maru
*/
BOOL CEditView::MakeDiffTmpFile( TCHAR* filename, HWND hWnd )
{
	const char*	pLineData;
	int		nLineLen;
	FILE	*fp;

	TCHAR* pszTmpName = _ttempnam( NULL, SAKURA_DIFF_TEMP_PREFIX );
	if( NULL == pszTmpName ){
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("差分コマンド実行は失敗しました。") );
		return FALSE;
	}

	_tcscpy( filename, pszTmpName );
	free( pszTmpName );

	//自分か？
	if( NULL == hWnd )
	{
		CEol	cEol( m_pcEditDoc->m_cSaveLineCode );
		FILETIME	filetime;
		return (BOOL)m_pcEditDoc->m_cDocLineMgr.WriteFile( 
			filename, 
			m_pcEditDoc->m_hWnd,
			NULL,
			m_pcEditDoc->m_nCharCode,
			&filetime,
			cEol,
			m_pcEditDoc->m_bBomExist);	//	Jul. 26, 2003 ryoji BOM
	}

	fp = fopen( filename, "wb" );
	if( NULL == fp ){
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("差分コマンド実行は失敗しました。\n\n一時ファイルを作成できません。") );
		return FALSE;
	}

	CLogicInt y = CLogicInt(0);

	// 行(改行単位)データの要求
	if( hWnd ){
		pLineData = m_pShareData->m_szWork;
		nLineLen = ::SendMessage( hWnd, MYWM_GETLINEDATA, y, 0 );
	}
	else{
		pLineData = m_pcEditDoc->m_cDocLineMgr.GetLineStr( y, &nLineLen );
	}

	while(1){
		if( 0 == nLineLen || NULL == pLineData ) break;

		if( hWnd && nLineLen > sizeof( m_pShareData->m_szWork ) ){
			// 一時バッファを超えている
			fclose( fp );
			_tunlink( filename );		//関数の実行に失敗したとき、一時ファイルの削除は関数内で行う。2005.10.29
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("差分コマンド実行は失敗しました。\n\n行が長すぎます。") );
			return FALSE;
		}

		if( 1 != fwrite( pLineData, nLineLen, 1, fp ) ){
			fclose( fp );
			_tunlink( filename );	//関数の実行に失敗したとき、一時ファイルの削除は関数内で行う。2005.10.29
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME, _T("差分コマンド実行は失敗しました。\n\n一時ファイルを作成できません。") );
			return FALSE;
		}

		y++;

		// 行(改行単位)データの要求 
		if( hWnd ) nLineLen = ::SendMessage( hWnd, MYWM_GETLINEDATA, y, 0 );
		else       pLineData = m_pcEditDoc->m_cDocLineMgr.GetLineStr( y, &nLineLen );
	}

	fclose( fp );

	return TRUE;
}


/*[EOF]*/
