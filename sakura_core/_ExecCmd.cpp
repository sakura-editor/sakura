//	$Id$
//	Copyright (C) 1998-2000, Norio Nakatani
/*********

  ExecCmd("ping 127.0.0.1 -t" );
ExecCmd("cl /?" );

************/


/* 外部コマンド実行 & 標準出力トラップ */
#ifdef _DEBUG
void CEditWnd::ExecCmd(LPCSTR lpszCmd/*, HANDLE hFile*/) 
{
	MYTRACE( "%s\n", lpszCmd );
	
	HANDLE hReadPipe; // address of variable for read handle
	HANDLE hWritePipe; // address of variable for write handle
	BOOL bRes;
	DWORD dwNumberOfBytesRead;
	DWORD dwNumberOfBytesToRead;
	DWORD dwNumberOfBytesWritten;
	char szBuffer[32];
	CMemory cmemOutBuf;
//	OVERLAPPED Overlapped;
	bRes = ::CreatePipe(
		&hReadPipe, // address of variable for read handle
		&hWritePipe, // address of variable for write handle
		NULL, // pointer to security attributes
		0 // number of bytes reserved for pipe
	);
	// set up overlapped structure fields 
	// to simplify this sample, we'll eschew an event handle 
//	Overlapped.Offset     = 0; 
//	Overlapped.OffsetHigh = 0; 
//	Overlapped.hEvent     = NULL; 

	
	STARTUPINFO		StartupInfo;
	PROCESS_INFORMATION	ProcessInfo;
	char			szCmd[512];

	GetStartupInfo(&StartupInfo);
	StartupInfo.dwFlags	|= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_HIDE/*SW_SHOW*/;
	StartupInfo.hStdOutput	= hWritePipe;
	StartupInfo.hStdError	= hWritePipe;

//	lstrcpy( szCmd, "CMD.EXE /C ");	// Win95/98 なら command.com
	lstrcpy( szCmd, "command.com /C " );	// Win95/98 なら command.com
	lstrcat( szCmd, lpszCmd );
//	lstrcat( szCmd, " > con.txt" );	// 標準出力のみならこれでＯＫ

	if ( !CreateProcess(NULL, szCmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE,
		NULL, NULL, &StartupInfo, &ProcessInfo) ) {
	    return;
	}
	MYTRACE( "ProcessInfo.hProcess   =%xh\n", ProcessInfo.hProcess    ) ;
	MYTRACE( "ProcessInfo.hThread    =%xh\n", ProcessInfo.hThread     ) ;
	MYTRACE( "ProcessInfo.dwProcessId=%xh\n", ProcessInfo.dwProcessId ) ;
	MYTRACE( "ProcessInfo.dwThreadId =%xh\n", ProcessInfo.dwThreadId  ) ;


	DWORD dwRes;
	CDlgCancel cDlgCancel;
	cDlgCancel.DoModeless( m_hInstance, m_hWnd, IDD_GREPRUNNING );	/* モードレスダイアログの表示 */

	while( 1 ){
		dwRes = WaitForSingleObject(ProcessInfo.hProcess, 0/*INFINITE*/ );
//		if( WAIT_OBJECT_0 == dwRes ){
			bRes = ::WriteFile( hWritePipe, "\r\n", 2, &dwNumberOfBytesWritten, NULL );
//		}

		cmemOutBuf.SetDataSz( "" );
		while( 1 ){
			dwNumberOfBytesRead = 0;
			bRes = ::ReadFile(
			  hReadPipe, // handle of file to read
			  szBuffer, // pointer to buffer that receives data
			  sizeof( szBuffer ) - 1, // number of bytes to read
			  &dwNumberOfBytesRead, // pointer to number of bytes read
			  NULL/*&Overlapped*/ // pointer to structure for data
			);
//			MYTRACE( "bRes=%d dwNumberOfBytesRead=%d\n", bRes, dwNumberOfBytesRead );
			szBuffer[dwNumberOfBytesRead] = '\0';
//			MYTRACE( "szBuffer=[%s]\n", szBuffer );
//			MYTRACE( "%s", szBuffer );
			cmemOutBuf.AppendSz( szBuffer );
			if( 0 == dwNumberOfBytesRead 
			 || dwNumberOfBytesRead < sizeof( szBuffer ) - 1	
			){
				break;
			}
		}
		cmemOutBuf.m_pData[cmemOutBuf.m_nDataLen - 2] = '\0';
		MYTRACE( cmemOutBuf.m_pData );



//		if( WAIT_TIMEOUT == dwRes ) MYTRACE( "WAIT_TIMEOUT\n" );
//		if( WAIT_ABANDONED == dwRes ) MYTRACE( "WAIT_ABANDONED\n" );
		if( WAIT_OBJECT_0 == dwRes ){
			MYTRACE( "\n==============\n" );
			MYTRACE( "WAIT_OBJECT_0\n" );
			break;
		}
		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( cDlgCancel.m_hWnd ) ){
			break;
		}
		/* 中断ボタン押下チェック */
		if( cDlgCancel.IsCanceled() ){
			break;
		}
	}
//	ExitProcess(ProcessInfo.hProcess);
	bRes = ::CloseHandle(ProcessInfo.hThread);
	bRes = ::CloseHandle(ProcessInfo.hProcess);


	bRes = ::CloseHandle( hReadPipe );
	bRes = ::CloseHandle( hWritePipe );

	cDlgCancel.CloseDialog( 0 );	 

}
#endif