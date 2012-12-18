/*!	@file
	@brief エディタプロセスクラス

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CProcessより分離
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, genta, Moca, MIK
	Copyright (C) 2004, Moca, naoh
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CEditApp.h"
#include "CEditWnd.h" // 2002/2/3 aroka
#include "CShareData.h"
#include "CDocLine.h" // 2003/03/28 MIK
#include "etc_uty.h"
#include "mymessage.h" // 2002/2/3 aroka
#include "Debug.h"
#include "CRunningTimer.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CNormalProcess::CNormalProcess( HINSTANCE hInstance, LPTSTR lpCmdLine )
: m_pcEditWnd( 0 )
, CProcess( hInstance, lpCmdLine )
{
}

CNormalProcess::~CNormalProcess()
{
	if( m_pcEditWnd ){
		delete m_pcEditWnd;
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     プロセスハンドラ                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief エディタプロセスを初期化する
	
	CEditWndを作成する。
	
	@author aroka
	@date 2002/01/07

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2004.05.13 Moca CEditWnd::Create()に失敗した場合にfalseを返すように．
	@date 2007.06.26 ryoji グループIDを指定して編集ウィンドウを作成する
*/
bool CNormalProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, "NormalProcess::Init" );

	/* プロセス初期化の目印 */
	HANDLE	hMutex = _GetInitializeMutex();	// 2002/2/8 aroka 込み入っていたので分離
	if( NULL == hMutex ){
		return false;
	}

	/* 共有メモリを初期化する */
	if ( !CProcess::InitializeProcess() ){
		return false;
	}

	/* コマンドラインオプション */
	bool			bReadOnly;
	bool			bDebugMode;
	bool			bGrepMode;
	bool			bGrepDlg;
	GrepInfo		gi;
	EditInfo		fi;
	
	/* コマンドラインで受け取ったファイルが開かれている場合は */
	/* その編集ウィンドウをアクティブにする */
	CCommandLine::getInstance()->GetEditInfo(&fi); // 2002/2/8 aroka ここに移動
	if( 0 < _tcslen( fi.m_szPath ) ){
		//	Oct. 27, 2000 genta
		//	MRUからカーソル位置を復元する操作はCEditDoc::FileReadで
		//	行われるのでここでは必要なし．

		HWND hwndOwner;
		/* 指定ファイルが開かれているか調べる */
		// 2007.03.13 maru 文字コードが異なるときはワーニングを出すように
		if( m_cShareData.ActiveAlreadyOpenedWindow( fi.m_szPath, &hwndOwner, fi.m_nCharCode ) ){
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
		}
	}

	MY_TRACETIME( cRunningTimer, "CheckFile" );

	// グループIDを取得
	int nGroupId = CCommandLine::getInstance()->GetGroupId();
	if( m_pShareData->m_Common.m_sTabBar.m_bNewWindow && nGroupId == -1 ){
		nGroupId = CShareData::getInstance()->GetFreeGroupId();
	}
	// CEditWndを作成
	m_pcEditWnd = new CEditWnd;
	HWND hWnd = m_pcEditWnd->Create( m_hInstance, m_pShareData->m_hwndTray, nGroupId );
	if( NULL == hWnd ){
		::ReleaseMutex( hMutex );
		::CloseHandle( hMutex );
		return false;	// 2009.06.23 ryoji CEditWnd::Create()失敗のため終了
	}

	/* コマンドラインの解析 */	 // 2002/2/8 aroka ここに移動
	bDebugMode = CCommandLine::getInstance()->IsDebugMode();
	bGrepMode  = CCommandLine::getInstance()->IsGrepMode();
	bGrepDlg   = CCommandLine::getInstance()->IsGrepDlg();

	// -1: SetDocumentTypeWhenCreate での強制指定なし
	const int nType = (fi.m_szDocType[0] == '\0' ? -1 : m_cShareData.GetDocumentTypeOfExt(fi.m_szDocType));

	if( bDebugMode ){
		/* デバッグモニタモードに設定 */
		m_pcEditWnd->SetDebugModeON();
		// 2004.09.20 naoh アウトプット用タイプ別設定
		// 文字コードを有効とする Uchi 2008/6/8
		// 2010.06.16 Moca アウトプットは CCommnadLineで -TYPE=output 扱いとする
		m_pcEditWnd->SetDocumentTypeWhenCreate( (ECodeType)fi.m_nCharCode, FALSE, nType );
	}
	else if( bGrepMode ){
		/* GREP */
		// 2010.06.16 Moca Grepでもオプション指定を適用
		m_pcEditWnd->SetDocumentTypeWhenCreate( (ECodeType)fi.m_nCharCode, FALSE, nType );
		CCommandLine::getInstance()->GetGrepInfo(&gi); // 2002/2/8 aroka ここに移動
		if( !bGrepDlg ){
			TCHAR szWork[MAX_PATH];
			/* ロングファイル名を取得する */
			if( ::GetLongFileName( gi.cmGrepFolder.GetStringPtr(), szWork ) ){
				gi.cmGrepFolder.SetString( szWork, _tcslen( szWork ) );
			}
			// 2003.06.23 Moca GREP実行前にMutexを開放
			//	こうしないとGrepが終わるまで新しいウィンドウを開けない
			m_hWnd = hWnd;
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep(
				&gi.cmGrepKey,
				&gi.cmGrepFile,
				&gi.cmGrepFolder,
				gi.bGrepSubFolder,
				gi.sGrepSearchOption,
				gi.nGrepCharSet,	//	2002/09/21 Moca
				gi.bGrepOutputLine,
				gi.nGrepOutputStyle
			);
			return true; // 2003.06.23 Moca
		}
		else{
			//-GREPDLGでダイアログを出す。　引数も反映（2002/03/24 YAZAKI）
			CShareData::getInstance()->AddToSearchKeyArr( gi.cmGrepKey.GetStringPtr() );
			CShareData::getInstance()->AddToGrepFileArr( gi.cmGrepFile.GetStringPtr() );
			CShareData::getInstance()->AddToGrepFolderArr( gi.cmGrepFolder.GetStringPtr() );
			m_pShareData->m_Common.m_sSearch.m_bGrepSubFolder = gi.bGrepSubFolder;
			m_pShareData->m_Common.m_sSearch.m_sSearchOption = gi.sGrepSearchOption;
			m_pShareData->m_Common.m_sSearch.m_nGrepCharSet = gi.nGrepCharSet;
			m_pShareData->m_Common.m_sSearch.m_bGrepOutputLine = gi.bGrepOutputLine;
			m_pShareData->m_Common.m_sSearch.m_nGrepOutputStyle = gi.nGrepOutputStyle;
			// 2003.06.23 Moca GREPダイアログ表示前にMutexを開放
			//	こうしないとGrepが終わるまで新しいウィンドウを開けない
			m_hWnd = hWnd;
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			
			//	Oct. 9, 2003 genta コマンドラインからGERPダイアログを表示させた場合に
			//	引数の設定がBOXに反映されない
			_tcscpy( m_pcEditWnd->m_cEditDoc.m_cDlgGrep.m_szText, gi.cmGrepKey.GetStringPtr() );		/* 検索文字列 */
			_tcscpy( m_pcEditWnd->m_cEditDoc.m_cDlgGrep.m_szFile, gi.cmGrepFile.GetStringPtr() );		/* 検索ファイル */
			_tcscpy( m_pcEditWnd->m_cEditDoc.m_cDlgGrep.m_szFolder, gi.cmGrepFolder.GetStringPtr() );	/* 検索フォルダ */

			
			// Feb. 23, 2003 Moca Owner windowが正しく指定されていなかった
			int nRet = m_pcEditWnd->m_cEditDoc.m_cDlgGrep.DoModal( m_hInstance, hWnd,  NULL);
			if( FALSE != nRet ){
				m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].HandleCommand(F_GREP, TRUE, 0, 0, 0, 0);
			}
			return true; // 2003.06.23 Moca
		}
	}
	else{
		// 2004.05.13 Moca さらにif分の中から前に移動
		// ファイル名が与えられなくてもReadOnly指定を有効にするため．
		bReadOnly = CCommandLine::getInstance()->IsReadOnly(); // 2002/2/8 aroka ここに移動
		if( 0 < _tcslen( fi.m_szPath ) ){
			//	Mar. 9, 2002 genta 文書タイプ指定
			m_pcEditWnd->OpenDocumentWhenStart(
				fi.m_szPath,
				(ECodeType)fi.m_nCharCode,
				bReadOnly
			);
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
				m_pcEditWnd->m_cEditDoc.m_cLayoutMgr.LogicToLayout(
					fi.m_nX,
					fi.m_nY,
					&nPosX,
					&nPosY
				);

				// 2004.04.03 Moca EOFだけの行で終わっていると、EOFの一つ上の行に移動してしまうバグ修正
				// MoveCursorが補正するのである程度行わなくて良くなった
				// From Here Mar. 28, 2003 MIK
				// 改行の真ん中にカーソルが来ないように。
				// 2008.08.20 ryoji 改行単位の行番号を渡すように修正
				const CDocLine *pTmpDocLine = m_pcEditWnd->m_cEditDoc.m_cDocLineMgr.GetLine( fi.m_nY );
				if( pTmpDocLine ){
					if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_nX ) nPosX--;
				}
				// To Here Mar. 28, 2003 MIK

				m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( nPosX, nPosY, TRUE );
				m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev =
					m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX;
			}
			m_pcEditWnd->m_cEditDoc.m_cEditViewArr[0].RedrawAll();
		}
		else{
			// 2004.05.13 Moca ファイル名が与えられなくてもReadOnlyとタイプ指定を有効にする
			m_pcEditWnd->SetDocumentTypeWhenCreate(
				(ECodeType)fi.m_nCharCode,
				bReadOnly,	// 読み取り専用か
				nType
			);
		}
	}

	m_hWnd = hWnd;

	//ウィンドウキャプション更新
	m_pcEditWnd->m_cEditDoc.UpdateCaption();
	
	//	YAZAKI 2002/05/30 IMEウィンドウの位置がおかしいのを修正。
	m_pcEditWnd->m_cEditDoc.m_cEditViewArr[m_pcEditWnd->m_cEditDoc.m_nActivePaneIndex].SetIMECompFormPos();

	//再描画
	::InvalidateRect( m_pcEditWnd->m_hWnd, NULL, TRUE );

	::ReleaseMutex( hMutex );
	::CloseHandle( hMutex );

	// 2006.09.03 ryoji オープン後自動実行マクロを実行する
	if( hWnd && !( bDebugMode || bGrepMode ) )
		m_pcEditWnd->m_cEditDoc.RunAutoMacro( m_pShareData->m_Common.m_sMacro.m_nMacroOnOpened );


	// 起動時マクロオプション
	LPCSTR pszMacro = CCommandLine::getInstance()->GetMacro();
	if( hWnd  &&  pszMacro  &&  pszMacro[0] != '\0' ){
		LPCSTR pszMacroType = CCommandLine::getInstance()->GetMacroType();
		if( pszMacroType == NULL || pszMacroType == "" || strcmpi(pszMacroType, "file") == 0 ){
			pszMacroType = NULL;
		}
		CEditView* view = &m_pcEditWnd->m_cEditDoc.m_cEditViewArr[ m_pcEditWnd->m_cEditDoc.m_nActivePaneIndex ];
		view->HandleCommand( F_EXECEXTMACRO, TRUE, (LPARAM)pszMacro, (LPARAM)pszMacroType, 0, 0 );
	}

	return hWnd ? true : false;
}

/*!
	@brief エディタプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool CNormalProcess::MainLoop()
{
	if( m_pcEditWnd && m_hWnd ){
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
void CNormalProcess::OnExitProcess()
{
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief Mutex(プロセス初期化の目印)を取得する

	多数同時に起動するとウィンドウが表に出てこないことがある。
	
	@date 2002/2/8 aroka InitializeProcessから移動
	@retval Mutex のハンドルを返す
	@retval 失敗した時はリリースしてから NULL を返す
*/
HANDLE CNormalProcess::_GetInitializeMutex() const
{
	MY_RUNNINGTIMER( cRunningTimer, "NormalProcess::_GetInitializeMutex" );
	HANDLE hMutex;
	hMutex = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA_INIT );
	if( NULL == hMutex ){
		ErrorBeep();
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, _T("CreateMutex()失敗。\n終了します。") );
		return NULL;
	}
	if( ::GetLastError() == ERROR_ALREADY_EXISTS ){
		DWORD dwRet = ::WaitForSingleObject( hMutex, 15000 );	// 2002/2/8 aroka 少し長くした
		if( WAIT_TIMEOUT == dwRet ){// 別の誰かが起動中
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, _T("エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。") );
			::CloseHandle( hMutex );
			return NULL;
		}
	}
	return hMutex;
}

/*[EOF]*/
