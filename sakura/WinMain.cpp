//	$Id$
/*!	@file
	Entry Point
	
	@author Norio Nakatani
	@date	1998/03/13 作成
	@date	2001/06/26 genta ワード単位のGrepのためのコマンドライン処理追加
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <windows.h>
//#include <stdio.h>
//#include <io.h>
//#include <ras.h>
//#include <SHLWAPI.H>

//#include "sakura_rc.h"
//#include "CEditDoc.h"
//#include "debug.h"
//#include "CKeyBind.h"
//#include "CJre.h"
#include "CEditApp.h"
//#include "CEditWnd.h"
//#include "CShareData.h"
//#include "etc_uty.h"
//#include "global.h"
//#include "CProfile.h"
//#include "CRunningTimer.h"



BOOL CALLBACK ExitingDlgProc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam		// second message parameter
)
{
	switch( uMsg ){
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}


int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int );

/////////////////////////////////////////////////////////////////////////


//struct VS_VERSION_INFO_HEAD {
//	WORD				wLength;
//	WORD				wValueLength;
//	WORD				bText;
//	WCHAR				szKey[16];
//	VS_FIXEDFILEINFO	Value;
//};
//char* m_pszAppName = "SakuraTrayClass";



/*!
	Windows Entry point
	
	1つ目のエディタプロセスの場合は、このプロセスはコントロールプロセスと
	なり、新しいエディタプロセスを起動する。そうでないときはエディタプロセス
	となる。
	
	コントロールプロセスはCEditAppクラスのインスタンスを作り、エディタ
	プロセスはCEditWndクラスのインスタンスを作る。
*/
int WINAPI WinMain(
	HINSTANCE	hInstance,		//!< handle to current instance
	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	LPSTR		lpCmdLine,		//!< pointer to command line
	int			nCmdShow		//!< show state of window
)
{

	
//	/* テスト */
//	RASCONN			RasConn;
//	DWORD			dwStatus;
//	DWORD			dwSize;
//	DWORD			dwConnections;
//	RASCONNSTATUS	RasConnStatus;
//	RasConn.dwSize = sizeof( RASCONN );
//	dwSize = RasConn.dwSize;
//	/* RAS接続の監視 */
//	dwStatus = ::RasEnumConnections(
//	  &RasConn,			// buffer to receive connections data
//	  &dwSize,			// size in bytes of buffer
//	  &dwConnections	// number of connections written to buffer
//	);
//	if( 0 == dwStatus
//	 && NULL != RasConn.hrasconn
//	){
//		/* RAS接続されている */
//		RasConnStatus.dwSize = sizeof( RASCONNSTATUS );
//		::RasGetConnectStatus( RasConn.hrasconn, &RasConnStatus );
//		if( RASCS_Connected == RasConnStatus.rasconnstate
//		 && IDYES == ::MYMESSAGEBOX(
//			NULL,
//			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
//			GSTR_APPNAME,
//			"%s  %s\n\nrimeリモートコンピュータに接続されています。\n切断しますか?",
//			RasConn.szDeviceName, RasConn.szEntryName
//		)
//		){
//			/* RAS接続を切断 */
//			::RasHangUp( RasConn.hrasconn );
//
//		}
//	}




//	CRunningTimer	cRunningTimer;
//	CProfile		cProfile;

	HANDLE			hMutex;
//	HANDLE			hFileMap;
	CEditApp*		pcEditApp;
	CEditWnd*		pcEditWnd;
	HWND			hWnd;
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	/* コマンドラインオプション */
	BOOL			bGrepMode;
	CMemory			cmGrepKey;
	CMemory			cmGrepFile;
	CMemory			cmGrepFolder;
	CMemory			cmWork;
	BOOL			bGrepSubFolder;
	BOOL			bGrepLoHiCase;
	BOOL			bGrepRegularExp;
	BOOL			bGrepKanjiCode_AutoDetect;
	BOOL			bGrepOutputLine;
	BOOL			bGrepWordOnly;	//	Jun. 25, 2001 genta
	int				nGrepOutputStyle;
	BOOL			bDebugMode;
	BOOL			bNoWindow;
	FileInfo		fi;
	BOOL			bReadOnly;
	DWORD			dwRet;
	BOOL			bFindCTRLPROCESS;
//	BOOL			bInstanceAlreadyExist;

//	cRunningTimer.Reset();
//	cProfile.ReadProfile( "A:\\WINDOWS\\WIN.INI" );
//	MYTRACE( "A:\\WINDOWS\\WIN.INI  読み込み処理 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );

//	cRunningTimer.Reset();
//	cProfile.WriteProfile( "a:\\tmp\\test.ini" );
//	MYTRACE( "a:\\tmp\\test.ini  書き込み処理 所要時間(ミリ秒) = %d\n", cRunningTimer.Read() );


//	GetDllVersion( "Comctl32.dll" );
//	GetDllVersion( "Comdlg32.dll" );

	/* Windowsバージョンのチェック */
	if( FALSE == CheckWindowsVersion( GSTR_APPNAME ) ){
		return 0;
	}
	/* システムリソースのチェック */
	// Jul. 5, 2001 shoji masami NTではリソースチェックを行わない
	if( !CheckWindowsVersionNT() ){
		if( !CheckSystemResources( GSTR_APPNAME ) ){
			return 0;
		}
	}

	/* コモンコントロールの初期化 */
	::InitCommonControls();

//	/* 共有データ構造体のアドレスを返す */
//	m_cShareData.Init();
//	m_pShareData = m_cShareData.GetShareData( NULL, NULL );


	/* コマンドラインの解析 */
	CEditApp::ParseCommandLine(
		lpCmdLine,
		&bGrepMode,
		&cmGrepKey,
		&cmGrepFile,
		&cmGrepFolder,
		&bGrepSubFolder,
		&bGrepLoHiCase,
		&bGrepRegularExp,
		&bGrepKanjiCode_AutoDetect,
		&bGrepOutputLine,
		&bGrepWordOnly,	//	Jun. 25, 2001 genta
		&nGrepOutputStyle,
		&bDebugMode,
		&bNoWindow,
		&fi,
		&bReadOnly
	);

	hMutex = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA );
	if( NULL == hMutex ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, "CreateMutex()失敗。\n終了します。" );
		return 0;
	}
	if( ::GetLastError() == ERROR_ALREADY_EXISTS ){
		/* オブジェクトがシグナル状態になるか、または、タイムアウト時間が経過するまで待つ */
		dwRet = ::WaitForSingleObject( hMutex, 20000 );
		if( WAIT_TIMEOUT == dwRet
		 || WAIT_ABANDONED == dwRet
		 || WAIT_FAILED == dwRet
		){
			::MessageBeep( MB_ICONSTOP );
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, "コントロールプロセスが応答しません。\n終了します。" );
			return 0;
		}
		::ReleaseMutex( hMutex );


		/* 共有データ構造体のアドレスを返す */
		if( !m_cShareData.Init() ){
			//	適切なデータを得られなかった
			::MessageBox( NULL, "異なるバージョンのエディタを同時に起動することはできません。", GSTR_APPNAME, MB_OK | MB_ICONERROR );
			return 0;
		}
		m_pShareData = m_cShareData.GetShareData( NULL, NULL );

		bFindCTRLPROCESS = TRUE;
		if( NULL == m_pShareData->m_hwndTray ){
			bFindCTRLPROCESS = FALSE;
		}else{
			if( FALSE == IsWindow( m_pShareData->m_hwndTray ) ){
				bFindCTRLPROCESS = FALSE;
			}else{
				char	szClassName[64];
				szClassName[0] = '\0';
				::GetClassName( m_pShareData->m_hwndTray, szClassName, sizeof( szClassName ) - 1 );
				if( 0 != strcmp( GSTR_CEDITAPP, szClassName ) ){
					bFindCTRLPROCESS = FALSE;
				}
			}
		}
		if( FALSE == bFindCTRLPROCESS ){
//動作確認用	::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, "コントロールプロセスが存在していませんでした。\n新たにコントロールプロセスを起動します。\n" );
//			bNoWindow = TRUE;
			goto CreateControlProcess;
//			return 0;
		}

		/* コマンドラインで受け取ったファイルが開かれている場合は */
		/* その編集ウィンドウをアクティブにする */
		if( 0 < strlen( fi.m_szPath ) ){
			//	Oct. 27, 2000 genta
			//	MRUからカーソル位置を復元する操作はCEditDoc::FileReadで
			//	行われるのでここでは必要なし．
#if 0
			/* MRUリストに存在するか調べる  存在するならばファイル情報を返す */
			FileInfo fiWork;
			if( m_cShareData.IsExistInMRUList( (const char*)fi.m_szPath, &fiWork ) ){
//				MYTRACE( "MRUリストに存在する[%s]\n", fi.m_szPath );
				if( 0 == fi.m_nX && 0 == fi.m_nY ){
					fi.m_nX = fiWork.m_nX;
					fi.m_nY = fiWork.m_nY;
				}
				if( 0 == fi.m_nViewTopLine && 0 == fi.m_nViewLeftCol ){
					fi.m_nViewTopLine = fiWork.m_nViewTopLine;
					fi.m_nViewLeftCol = fiWork.m_nViewLeftCol;
				}
				if( 99 == fi.m_nCharCode ){
					fi.m_nCharCode = fiWork.m_nCharCode;
				}

			}
#endif

			HWND hwndOwner;
			/* 指定ファイルが開かれているか調べる */
			if( TRUE == m_cShareData.IsPathOpened( fi.m_szPath, &hwndOwner ) ){
				/* アクティブにする */
				ActivateFrameWindow( hwndOwner );
				return 0;
			}else{

			}
		}
//マルチスレッド版
//
//		/* 新しい編集ウィンドウの作成依頼(コマンドラインを渡す) */
//		::strcpy( pShareData->m_szWork, lpCmdLine );
//		hwndNew = (HWND)::SendMessage( pShareData->m_hwndTray, MYWM_OPENNEWEDITOR, 0, 0 );
//		/* アクティブにする */
//		ActivateFrameWindow( hwndNew );


//複数プロセス版
		/* エディタウィンドウオブジェクトを作成 */
		pcEditWnd = new CEditWnd;
		if( bDebugMode ){
			hWnd = pcEditWnd->Create( hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );

//	#ifdef _DEBUG/////////////////////////////////////////////
			/* デバッグモニタモードに設定 */
			pcEditWnd->SetDebugModeON();
//	#endif////////////////////////////////////////////////////
		}else
		if( bGrepMode ){
			hWnd = pcEditWnd->Create( hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );
			/* GREP */
			/*nHitCount = */

			char szWork[MAX_PATH];
			/* ロングファイル名を取得する */
			if( TRUE == ::GetLongFileName( cmGrepFolder.GetPtr( NULL ), szWork ) ){
				cmGrepFolder.SetData( szWork, strlen( szWork ) );
			}
			pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep(
				&cmGrepKey,
				&cmGrepFile,
				&cmGrepFolder,
				bGrepSubFolder,
				bGrepLoHiCase,
				bGrepRegularExp,
				bGrepKanjiCode_AutoDetect,
				bGrepOutputLine,
				bGrepWordOnly,	//	Jun. 26, 2001 genta
				nGrepOutputStyle
			);
		}else{
			if( 0 < (int)strlen( fi.m_szPath ) ){
				hWnd = pcEditWnd->Create( hInstance, m_pShareData->m_hwndTray, fi.m_szPath, fi.m_nCharCode, bReadOnly/* 読み取り専用か */ );
				//	Nov. 6, 2000 genta
				//	キャレット位置の復元のため
				//	オプション指定がないときは画面移動を行わないようにする
				if( ( 0 <= fi.m_nViewTopLine || 0 <= fi.m_nViewLeftCol ) && fi.m_nViewTopLine < pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewTopLine = fi.m_nViewTopLine;
					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewLeftCol = fi.m_nViewLeftCol;
				}

				//	Nov. 6, 2000 genta
				//	キャレット位置の復元のため
				//	m_nCaretPosX_Prevの用途は不明だが，何も設定しないのはまずいのでとりあえず 0
				pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
				//	オプション指定がないときはカーソル位置設定を行わないようにする
				if( fi.m_nX > 0 || fi.m_nY > 0 ){
					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					*/
					int		nPosX;
					int		nPosY;
					pcEditWnd->m_cEditDoc.m_cLayoutMgr.CaretPos_Phys2Log(
						fi.m_nX,
						fi.m_nY,
						&nPosX,
						&nPosY
					);
					if( nPosY < pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
						pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( nPosX, nPosY, TRUE );
						pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = nPosX;
					}else{
						int		i;
						i = pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() - 1;
						if( i < 0 ){
							i = 0;
						}
						pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( 0, i, TRUE );
						pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
					}
				}
				pcEditWnd->m_cEditDoc.m_cEditViewArr[0].RedrawAll();
			}else{
				hWnd = pcEditWnd->Create( hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );
			}
		}
		if( NULL != hWnd ){
			pcEditWnd->MessageLoop();	/* メッセージループ */
		}
		delete pcEditWnd;
	}else{
CreateControlProcess:;


		/* 共有データ構造体のアドレスを返す */
		if( !m_cShareData.Init() ){
			//	適切なデータを得られなかった
			::MessageBox( NULL, "異なるバージョンのエディタを同時に起動することはできません。", GSTR_APPNAME, MB_OK | MB_ICONERROR );
			return 0;
		}
		m_pShareData = m_cShareData.GetShareData( NULL, NULL );

		/* リソースから製品バージョンの取得 */
		GetAppVersionInfo( hInstance, VS_VERSION_INFO, &m_pShareData->m_dwProductVersionMS, &m_pShareData->m_dwProductVersionLS );
//		MYTRACE( "製品バージョン=%d.%d.%d.%d\n",
//			HIWORD( m_pShareData->m_dwProductVersionMS ), LOWORD( m_pShareData->m_dwProductVersionMS ),
//			HIWORD( m_pShareData->m_dwProductVersionLS ), LOWORD( m_pShareData->m_dwProductVersionLS )
//		);

		/* 共有データのロード */
		if( FALSE == m_cShareData.LoadShareData() ){
//			int	i;
			/*	レジストリから設定データが存在しなかった場合は初回起動時とみなし全レジストリデータを作っておく */

//			/* 変更フラグ(共通設定の全体)のセット */
//			m_pShareData->m_nCommonModify = TRUE;
//
//			/* 変更フラグ フォント */
//			m_pShareData->m_bFontModify = TRUE;
//
//			m_pShareData->m_bKeyBindModify = TRUE;	/* 変更フラグ キーバインド */
//			for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
//				m_pShareData->m_bKeyBindModifyArr[i] = TRUE;	/* 変更フラグ キーバインド(キーごと) */
//			}
//			/* 変更フラグ(印刷の全体)のセット */
//			m_pShareData->m_bPrintSettingModify = TRUE;
//			/* 変更フラグ(印刷設定ごと)のセット */
//			for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
//				m_pShareData->m_bPrintSettingModifyArr[i] = TRUE;
//			}
//			m_pShareData->m_bKeyWordSetModify = TRUE;	/* 変更フラグ(キーワードの全体) */
//			for( i = 0; i < MAX_SETNUM; ++i ){
//				m_pShareData->m_bKeyWordSetModifyArr[i] = TRUE;	/* 変更フラグ(キーワードのセットごと) */
//			}
//			/* 変更フラグ(タイプ別設定) のセット */
//			for( i = 0; i < MAX_TYPES; ++i ){
//				m_pShareData->m_nTypesModifyArr[i] = TRUE;
//			}

			/* レジストリ項目 作成 */
			m_cShareData.SaveShareData();
		}

		/* タスクトレイにアイコン作成 */
		pcEditApp = new CEditApp;
		HWND	hwndTray;

		if( NULL == ( hwndTray = pcEditApp->Create( hInstance ) ) ){
			::MessageBeep( MB_ICONSTOP );
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, "ウィンドウの作成に失敗しました。\n起動できません。" );
			return 0;
		}
		m_pShareData->m_hwndTray = hwndTray;

		::ReleaseMutex( hMutex );

		/* 空の編集ウィンドウを作成 */
		if( !bNoWindow ){
			CEditApp::OpenNewEditor(
				hInstance,
				m_pShareData->m_hwndTray,
				lpCmdLine,
				//	May 30, 2000 genta
				fi.m_nCharCode,	/* 文字コードは引数の設定を引き継ぐ */
				FALSE				/* 読み取り専用か */
			);
		}
		pcEditApp->MessageLoop();

		dwRet = ::WaitForSingleObject( hMutex, 20000 );

		/* 終了ダイアログを表示する */
		HWND hwndExitingDlg;
		if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
			/* 終了中ダイアログの表示 */
			hwndExitingDlg = ::CreateDialog(
				hInstance,
				MAKEINTRESOURCE( IDD_EXITING ),
				/*m_hWnd*/::GetDesktopWindow(),
				(DLGPROC)ExitingDlgProc
			);
			::ShowWindow( hwndExitingDlg, SW_SHOW );
		}

		/* 共有データの保存 */
		m_cShareData.SaveShareData();

		/* 終了ダイアログを表示する */
		if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
			/* 終了中ダイアログの破棄 */
			::DestroyWindow( hwndExitingDlg );
		}
		
		m_pShareData->m_hwndTray = NULL;
		delete pcEditApp;
		/* アクセラレータテーブルの削除 */
		if( m_pShareData->m_hAccel != NULL ){
			::DestroyAcceleratorTable( m_pShareData->m_hAccel );
			m_pShareData->m_hAccel = NULL; 
		}

//		Sleep( 3000 );
		::ReleaseMutex( hMutex );
	}
	return 0;
}


/*[EOF]*/
