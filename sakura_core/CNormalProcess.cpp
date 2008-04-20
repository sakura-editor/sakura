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
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CControlTray.h"
#include "CShareData.h"
#include "Debug.h"
#include "window/CEditWnd.h" // 2002/2/3 aroka
#include "mymessage.h" // 2002/2/3 aroka
#include "doc/CDocLine.h" // 2003/03/28 MIK
#include <tchar.h>
#include "CRunningTimer.h"
#include "util/window.h"
#include "util/file.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               コンストラクタ・デストラクタ                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CNormalProcess::CNormalProcess( HINSTANCE hInstance, LPTSTR lpCmdLine )
: m_pcEditApp( NULL )
, CProcess( hInstance, lpCmdLine )
{
}

CNormalProcess::~CNormalProcess()
{
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

	if ( !CProcess::InitializeProcess() ){
		return false;
	}

	/* コマンドラインオプション */
	bool			bViewMode;
	bool			bDebugMode;
	bool			bGrepMode;
	bool			bGrepDlg;
	GrepInfo		gi;
	EditInfo		fi;
	
	/* コマンドラインで受け取ったファイルが開かれている場合は */
	/* その編集ウィンドウをアクティブにする */
	CCommandLine::Instance()->GetEditInfo(&fi); // 2002/2/8 aroka ここに移動
	if( 0 < _tcslen( fi.m_szPath ) ){
		//	Oct. 27, 2000 genta
		//	MRUからカーソル位置を復元する操作はCEditDoc::FileLoadで
		//	行われるのでここでは必要なし．

		HWND hwndOwner;
		/* 指定ファイルが開かれているか調べる */
		// 2007.03.13 maru 文字コードが異なるときはワーニングを出すように
		if( GetShareData().ActiveAlreadyOpenedWindow( fi.m_szPath, &hwndOwner, fi.m_nCharCode ) ){
			//	From Here Oct. 19, 2001 genta
			//	カーソル位置が引数に指定されていたら指定位置にジャンプ
			if( fi.m_ptCursor.y >= 0 ){	//	行の指定があるか
				CLogicPoint& pt = *CProcess::GetDllShareData().GetWorkBuffer<CLogicPoint>();
				if( fi.m_ptCursor.x < 0 ){
					//	桁の指定が無い場合
					::SendMessageAny( hwndOwner, MYWM_GETCARETPOS, 0, 0 );
				}
				else {
					pt.x = fi.m_ptCursor.x;
				}
				pt.y = fi.m_ptCursor.y;
				::SendMessageAny( hwndOwner, MYWM_SETCARETPOS, 0, 0 );
			}
			//	To Here Oct. 19, 2001 genta
			/* アクティブにする */
			ActivateFrameWindow( hwndOwner );
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			return false;
		}
	}


	// エディタアプリケーションを作成。2007.10.23 kobake
	m_pcEditApp = new CEditApp(GetProcessInstance());
	CEditWnd* pEditWnd = m_pcEditApp->GetWindow();

	/* コマンドラインの解析 */	 // 2002/2/8 aroka ここに移動
	bDebugMode = CCommandLine::Instance()->IsDebugMode();
	bGrepMode  = CCommandLine::Instance()->IsGrepMode();
	bGrepDlg   = CCommandLine::Instance()->IsGrepDlg();

	MY_TRACETIME( cRunningTimer, "CheckFile" );
	if( bDebugMode ){
		/* デバッグモニタモードに設定 */
		CAppMode::Instance()->SetDebugModeON();
		// 2004.09.20 naoh アウトプット用タイプ別設定
		pEditWnd->GetDocument().m_cDocType.SetDocumentType( GetShareData().GetDocumentTypeExt(_T("output")), true );
	}
	else if( bGrepMode ){
		// 2004.05.13 Moca CEditWnd::Create()に失敗した場合の考慮を追加
		if( NULL == pEditWnd->GetHwnd() ){
			goto end_of_func;
		}
		/* GREP */
		CCommandLine::Instance()->GetGrepInfo(&gi); // 2002/2/8 aroka ここに移動
		if( !bGrepDlg ){
			TCHAR szWork[MAX_PATH];
			/* ロングファイル名を取得する */
			if( ::GetLongFileName( gi.cmGrepFolder.GetStringPtr(), szWork ) ){
				gi.cmGrepFolder.SetString( szWork, _tcslen( szWork ) );
			}
			// 2003.06.23 Moca GREP実行前にMutexを開放
			//	こうしないとGrepが終わるまで新しいウィンドウを開けない
			SetMainWindow( pEditWnd->GetHwnd() );
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			this->m_pcEditApp->m_pcGrepAgent->DoGrep(
				pEditWnd->m_pcEditViewArr[0],
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
			GetDllShareData().m_Common.m_sSearch.m_bGrepSubFolder = gi.bGrepSubFolder;
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption = gi.sGrepSearchOption;
			GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet = gi.nGrepCharSet;
			GetDllShareData().m_Common.m_sSearch.m_bGrepOutputLine = gi.bGrepOutputLine;
			GetDllShareData().m_Common.m_sSearch.m_nGrepOutputStyle = gi.nGrepOutputStyle;
			// 2003.06.23 Moca GREPダイアログ表示前にMutexを開放
			//	こうしないとGrepが終わるまで新しいウィンドウを開けない
			SetMainWindow( pEditWnd->GetHwnd() );
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			
			//	Oct. 9, 2003 genta コマンドラインからGERPダイアログを表示させた場合に
			//	引数の設定がBOXに反映されない
			wcscpy( pEditWnd->m_cDlgGrep.m_szText, gi.cmGrepKey.GetStringPtr() );		/* 検索文字列 */
			_tcscpy( pEditWnd->m_cDlgGrep.m_szFile, gi.cmGrepFile.GetStringPtr() );	/* 検索ファイル */
			_tcscpy( pEditWnd->m_cDlgGrep.m_szFolder, gi.cmGrepFolder.GetStringPtr() );	/* 検索フォルダ */

			
			// Feb. 23, 2003 Moca Owner windowが正しく指定されていなかった
			int nRet = pEditWnd->m_cDlgGrep.DoModal( GetProcessInstance(), pEditWnd->GetHwnd(),  NULL);
			if( FALSE != nRet ){
				pEditWnd->m_pcEditViewArr[0]->GetCommander().HandleCommand(F_GREP, TRUE, 0, 0, 0, 0);
			}
			return true; // 2003.06.23 Moca
		}
	}
	else{
		// 2004.05.13 Moca さらにif分の中から前に移動
		// ファイル名が与えられなくてもReadOnly指定を有効にするため．
		bViewMode = CCommandLine::Instance()->IsViewMode(); // 2002/2/8 aroka ここに移動
		if( 0 < _tcslen( fi.m_szPath ) ){
			//	Mar. 9, 2002 genta 文書タイプ指定
			pEditWnd->OpenDocumentWhenStart(
				SLoadInfo(
					fi.m_szPath,
					fi.m_nCharCode,
					bViewMode
				)
			);
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.m_nCharCode,
				bViewMode, // ビューモードか
				fi.m_szDocType[0] == '\0' ? CDocumentType(-1) : GetShareData().GetDocumentTypeExt( fi.m_szDocType )
			);
			// 2004.05.13 Moca CEditWnd::Create()に失敗した場合の考慮を追加
			if( NULL == pEditWnd->GetHwnd() ){
				goto end_of_func;
			}
			//	Nov. 6, 2000 genta
			//	キャレット位置の復元のため
			//	オプション指定がないときは画面移動を行わないようにする
			//	Oct. 19, 2001 genta
			//	未設定＝-1になるようにしたので，安全のため両者が指定されたときだけ
			//	移動するようにする． || → &&
			if( ( CLayoutInt(0) <= fi.m_nViewTopLine && CLayoutInt(0) <= fi.m_nViewLeftCol )
				&& fi.m_nViewTopLine < pEditWnd->GetDocument().m_cLayoutMgr.GetLineCount() ){
				pEditWnd->m_pcEditViewArr[0]->GetTextArea().SetViewTopLine( fi.m_nViewTopLine );
				pEditWnd->m_pcEditViewArr[0]->GetTextArea().SetViewLeftCol( fi.m_nViewLeftCol );
			}

			//	オプション指定がないときはカーソル位置設定を行わないようにする
			//	Oct. 19, 2001 genta
			//	0も位置としては有効な値なので判定に含めなくてはならない
			if( 0 <= fi.m_ptCursor.x || 0 <= fi.m_ptCursor.y ){
				/*
				  カーソル位置変換
				  物理位置(行頭からのバイト数、折り返し無し行位置)
				  →
				  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
				*/
				CLayoutPoint ptPos;
				pEditWnd->GetDocument().m_cLayoutMgr.LogicToLayout(
					fi.m_ptCursor,
					&ptPos
				);

				// From Here Mar. 28, 2003 MIK
				// 改行の真ん中にカーソルが来ないように。
				// 2008.08.20 ryoji 改行単位の行番号を渡すように修正
				const CDocLine *pTmpDocLine = pEditWnd->GetDocument().m_cDocLineMgr.GetLine( fi.m_ptCursor.GetY2() );
				if( pTmpDocLine ){
					if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_ptCursor.x ) ptPos.x--;
				}
				// To Here Mar. 28, 2003 MIK

				pEditWnd->m_pcEditViewArr[0]->GetCaret().MoveCursor( ptPos, TRUE );
				pEditWnd->m_pcEditViewArr[0]->GetCaret().m_nCaretPosX_Prev =
					pEditWnd->m_pcEditViewArr[0]->GetCaret().GetCaretLayoutPos().GetX2();
			}
			pEditWnd->m_pcEditViewArr[0]->RedrawAll();
		}
		else{
			// 2004.05.13 Moca ファイル名が与えられなくてもReadOnlyとタイプ指定を有効にする
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.m_nCharCode,
				bViewMode,	// ビューモードか
				fi.m_szDocType[0] == '\0' ? CDocumentType(-1) : GetShareData().GetDocumentTypeExt( fi.m_szDocType )
			);
		}
	}

end_of_func:
	SetMainWindow( pEditWnd->GetHwnd() );

	//ウィンドウキャプション更新
	pEditWnd->UpdateCaption();

	//	YAZAKI 2002/05/30 IMEウィンドウの位置がおかしいのを修正。
	pEditWnd->GetActiveView().SetIMECompFormPos();

	//WM_SIZEをポスト
	{
		RECT rc;
		GetClientRect( pEditWnd->GetHwnd(), &rc);
		::PostMessageAny(
			pEditWnd->GetHwnd(),
			WM_SIZE,
			SIZE_RESTORED,
			MAKELPARAM(
				rc.right -rc.left,
				rc.bottom-rc.top
			)
		);
	}

	//再描画
	::InvalidateRect( pEditWnd->GetHwnd(), NULL, TRUE );

	::ReleaseMutex( hMutex );
	::CloseHandle( hMutex );


	return pEditWnd->GetHwnd() ? true : false;
}

/*!
	@brief エディタプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool CNormalProcess::MainLoop()
{
	if( GetMainWindow() ){
		m_pcEditApp->GetWindow()->MessageLoop();	/* メッセージループ */
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
	SAFE_DELETE(m_pcEditApp);
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
		TopErrorMessage( NULL, _T("CreateMutex()失敗。\n終了します。") );
		return NULL;
	}
	if( ::GetLastError() == ERROR_ALREADY_EXISTS ){
		DWORD dwRet = ::WaitForSingleObject( hMutex, 15000 );	// 2002/2/8 aroka 少し長くした
		if( WAIT_TIMEOUT == dwRet ){// 別の誰かが起動中
			TopErrorMessage( NULL, _T("エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。") );
			::CloseHandle( hMutex );
			return NULL;
		}
	}
	return hMutex;
}


