//	$Id$
/*!	@file
	@brief エディタプロセスクラス

	@author aroka
	@date 2002/01/07 Create
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CProcessより分離

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CEditApp.h"
//#include "CMemory.h"
#include "CShareData.h"
#include "Debug.h"
#include "etc_uty.h"
#include "CEditWnd.h" // 2002/2/3 aroka
#include "mymessage.h" // 2002/2/3 aroka
#include <tchar.h>


/*!
	@brief エディタプロセスを初期化する
	
	CEditWndを作成する。
	
	@author aroka
	@date 2002/01/07
*/
bool CNormalProcess::Initialize()
{
	HANDLE			hMutex;
	HWND			hWnd;
//	CShareData		m_cShareData;
//	DLLSHAREDATA*	m_pShareData;

	/* プロセス初期化の目印 */
	hMutex = GetInitializeMutex();	// 2002/2/8 aroka 込み入っていたので分離
	if( NULL == hMutex ){
		return false;
	}

	if ( CProcess::Initialize() == false ){
		return false;
	}
#if 0
	/* 共有データ構造体のアドレスを返す */
	if( !m_cShareData.Init() ){
		//	適切なデータを得られなかった
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONERROR, GSTR_APPNAME,
			_T("異なるバージョンのエディタを同時に起動することはできません。") );
		::ReleaseMutex( hMutex );
		::CloseHandle( hMutex );
		return false;
	}
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );
#endif

	/* コマンドラインオプション */
	bool			bReadOnly;
	bool			bDebugMode;
	bool			bGrepMode;
	GrepInfo		gi;
	FileInfo		fi;
	
	/* コマンドラインで受け取ったファイルが開かれている場合は */
	/* その編集ウィンドウをアクティブにする */
	CCommandLine::Instance()->GetFileInfo(fi); // 2002/2/8 aroka ここに移動
	if( 0 < strlen( fi.m_szPath ) ){
		//	Oct. 27, 2000 genta
		//	MRUからカーソル位置を復元する操作はCEditDoc::FileReadで
		//	行われるのでここでは必要なし．

		HWND hwndOwner;
		/* 指定ファイルが開かれているか調べる */
		if( FALSE != m_cShareData.IsPathOpened( fi.m_szPath, &hwndOwner ) ){
			//	From Here Oct. 19, 2001 genta
			//	カーソル位置が引数に指定されていたら指定位置にジャンプ
			if( fi.m_nY >= 0 ){	//	行の指定があるか
				POINT& pt = *(POINT*)CProcess::m_pShareData->m_szWork;
				if( fi.m_nX < 0 ){
					//	桁の指定が無い場合
					::SendMessage( hwndOwner, MYWM_GETCARETPOS, 0, 0 );
				}
				else {
					pt.x = fi.m_nX;
				}
				pt.y = fi.m_nY;
				::SendMessage( hwndOwner, MYWM_SETCARETPOS, 0, 0 );
			}
			//	To Here Oct. 19, 2001 genta
			/* アクティブにする */
			ActivateFrameWindow( hwndOwner );
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			return false;
		}else{

		}
	}

//複数プロセス版
	/* エディタウィンドウオブジェクトを作成 */
	m_pcEditWnd = new CEditWnd;

	/* コマンドラインの解析 */	 // 2002/2/8 aroka ここに移動
	bDebugMode = CCommandLine::Instance()->IsDebugMode();
	bGrepMode = CCommandLine::Instance()->IsGrepMode();
	
	if( bDebugMode ){
		hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );

//	#ifdef _DEBUG/////////////////////////////////////////////
		/* デバッグモニタモードに設定 */
		m_pcEditWnd->SetDebugModeON();
//	#endif////////////////////////////////////////////////////
	}else
	if( bGrepMode ){
		hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );
		/* GREP */
		/*nHitCount = */

		TCHAR szWork[MAX_PATH];
		CCommandLine::Instance()->GetGrepInfo(gi); // 2002/2/8 aroka ここに移動
		/* ロングファイル名を取得する */
		if( FALSE != ::GetLongFileName( gi.cmGrepFolder.GetPtr( NULL ), szWork ) ){
			gi.cmGrepFolder.SetData( szWork, strlen( szWork ) );
		}
		m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep(
			&gi.cmGrepKey,
			&gi.cmGrepFile,
			&gi.cmGrepFolder,
			gi.bGrepSubFolder,
			gi.bGrepNoIgnoreCase,
			gi.bGrepRegularExp,
			gi.bGrepKanjiCode_AutoDetect,
			gi.bGrepOutputLine,
			gi.bGrepWordOnly,	//	Jun. 26, 2001 genta
			gi.nGrepOutputStyle
		);
	}else{
		if( 0 < (int)strlen( fi.m_szPath ) ){
			bReadOnly = CCommandLine::Instance()->IsReadOnly(); // 2002/2/8 aroka ここに移動
			hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, 
										fi.m_szPath, fi.m_nCharCode, bReadOnly/* 読み取り専用か */ );
			//	Nov. 6, 2000 genta
			//	キャレット位置の復元のため
			//	オプション指定がないときは画面移動を行わないようにする
			//	Oct. 19, 2001 genta
			//	未設定＝-1になるようにしたので，安全のため両者が指定されたときだけ
			//	移動するようにする． || → &&
			if( ( 0 <= fi.m_nViewTopLine && 0 <= fi.m_nViewLeftCol )
				&& fi.m_nViewTopLine < m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
				m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewTopLine = fi.m_nViewTopLine;
				m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewLeftCol = fi.m_nViewLeftCol;
			}

			//	Nov. 6, 2000 genta
			//	キャレット位置の復元のため
			//	m_nCaretPosX_Prevの用途は不明だが，何も設定しないのはまずいのでとりあえず 0
			m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
			//	オプション指定がないときはカーソル位置設定を行わないようにする
			//	Oct. 19, 2001 genta
			//	0も位置としては有効な値なので判定に含めなくてはならない
			if( 0 <= fi.m_nX || 0 <= fi.m_nY ){
				/*
				  カーソル位置変換
				  物理位置(行頭からのバイト数、折り返し無し行位置)
				  →
				  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
				*/
				int		nPosX;
				int		nPosY;
				m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.CaretPos_Phys2Log(
					fi.m_nX,
					fi.m_nY,
					&nPosX,
					&nPosY
				);
				if( nPosY < m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( nPosX, nPosY, TRUE );
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = nPosX;
				}else{
					int		i;
					i = m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() - 1;
					if( i < 0 ){
						i = 0;
					}
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( 0, i, TRUE );
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
				}
			}
			m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].RedrawAll();
		}else{
			hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );
		}
	}
	m_hWnd = hWnd;
	::ReleaseMutex( hMutex );
	::CloseHandle( hMutex );
	return true;
}

/*!
	@brief エディタプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool CNormalProcess::MainLoop()
{
	if( NULL != m_pcEditWnd && NULL != m_hWnd ){
		m_pcEditWnd->MessageLoop();	/* メッセージループ */
		return true;
	}
	return false;
}

/*!
	@brief エディタプロセスを終了する
	
	@author aroka
	@date 2002/01/07
	こいつはなにもしない。後始末はdtorで。
*/
void CNormalProcess::Terminate()
{
}



/*!
	デストラクタ
	
	@date 2002/2/3 aroka ヘッダから移動
*/
CNormalProcess::~CNormalProcess()
{
	if( m_pcEditWnd ){
		delete m_pcEditWnd;
	}
};


/*!
	@brief Mutex(プロセス初期化の目印)を取得する

	多数同時に起動するとウィンドウが表に出てこないことがある。
	
	@date 2002/2/8 aroka Initializeから移動
	@retval Mutex のハンドルを返す
	@retval 失敗した時はリリースしてから NULL を返す
*/
HANDLE CNormalProcess::GetInitializeMutex() const
{
	HANDLE hMutex;
	hMutex = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA_INIT );
	if( NULL == hMutex ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST,
			GSTR_APPNAME, _T("CreateMutex()失敗。\n終了します。") );
		return NULL;
	}
	if( ::GetLastError() == ERROR_ALREADY_EXISTS ){
		DWORD dwRet = ::WaitForSingleObject( hMutex, 15000 );	// 2002/2/8 aroka 少し長くした
		if( WAIT_TIMEOUT == dwRet ){// 別の誰かが起動中
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				_T("エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。") );
			::CloseHandle( hMutex );
			return NULL;
		}
	}
	return hMutex;
}

/*[EOF]*/
