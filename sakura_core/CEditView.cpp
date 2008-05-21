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
	Copyright (C) 2006, Moca, aroka, ryoji, fon, genta
	Copyright (C) 2007, ryoji, じゅうじ, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "debug.h"
//#include "keycode.h"
//#include "funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
//#include "CShareData.h"
#include "CDlgCancel.h"
//#include "sakura_rc.h"
#include "etc_uty.h"
//#include "global.h"
//#include "CAutoSave.h"
#include "CLayout.h"/// 2002/2/3 aroka
#include "COpe.h"///
#include "COpeBlk.h"///
#include "CDropTarget.h"///
#include "CSplitBoxWnd.h"///
#include "CRegexKeyword.h"///	//@@@ 2001.11.17 add MIK
#include "CMarkMgr.h"///
#include "COsVersionInfo.h"
//#include "CDocLine.h"   // 2002.04.09 minfu
#include "CFileLoad.h" // 2002/08/30 Moca
#include "CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
//#include "my_icmp.h" // 2002/11/30 Moca 追加
//#include "CMigemo.h"

#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif

// novice 2004/10/10 マウスサイドボタン対応
#ifndef WM_XBUTTONDOWN
	#define WM_XBUTTONDOWN   0x020B
	#define WM_XBUTTONUP     0x020C
	#define WM_XBUTTONDBLCLK 0x020D
#endif
#ifndef XBUTTON1
	#define XBUTTON1 0x0001
	#define XBUTTON2 0x0002
#endif

#ifndef IMR_RECONVERTSTRING
#define IMR_RECONVERTSTRING             0x0004
#endif // IMR_RECONVERTSTRING

/* 2002.04.09 minfu 再変換調整 */
#ifndef IMR_CONFIRMRECONVERTSTRING
#define IMR_CONFIRMRECONVERTSTRING             0x0005
#endif // IMR_CONFIRMRECONVERTSTRING

const int STRNCMP_MAX = 100;	/* MAXキーワード長：strnicmp文字列比較最大値(CEditView::KeySearchCore) */	// 2006.04.10 fon

CEditView*	g_m_pcEditView;
LRESULT CALLBACK EditViewWndProc( HWND, UINT, WPARAM, LPARAM );
VOID CALLBACK EditViewTimerProc( HWND, UINT, UINT, DWORD );

#define IDT_ROLLMOUSE	1

/* リソースヘッダー */
#define	 BFT_BITMAP		0x4d42	  /* 'BM' */

/* リソースがDIBかどうかを判断するマクロ */
#define	 ISDIB(bft)		((bft) == BFT_BITMAP)

/* 指定された値を最も近いバイト境界に整列させるマクロ */
#define	 WIDTHBYTES(i)	((i+31)/32*4)


//@@@2002.01.14 YAZAKI staticにしてメモリの節約（(10240+10) * 3 バイト）
int CEditView::m_pnDx[MAXLINESIZE + 10];

/*
	@date 2006.01.16 Moca 他のTYMEDが利用可能でも、取得できるように変更。
	@note IDataObject::GetData() で tymed = TYMED_HGLOBAL を指定すること。
*/
BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC	fe;

	// 2006.01.16 Moca 他のTYMEDが利用可能でも、IDataObject::GetData()で
	//  tymed = TYMED_HGLOBALを指定すれば問題ない
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = TYMED_HGLOBAL;
	// 2006.03.16 Moca S_FALSEでも受け入れてしまうバグを修正(ファイルのドロップ等)
	return S_OK == pDataObject->QueryGetData( &fe );
}
HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC fe;
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	// 2006.01.16 Moca fe.tymed = -1からTYMED_HGLOBALに変更。
	fe.tymed = TYMED_HGLOBAL;

	HGLOBAL hDest = NULL;
	STGMEDIUM stgMedium;
	// 2006.03.16 Moca SUCCEEDEDマクロではS_FALSEのとき困るので、S_OKに変更
	if( S_OK == pDataObject->GetData( &fe, &stgMedium ) ){
		if( stgMedium.pUnkForRelease == NULL ){
			if( stgMedium.tymed == TYMED_HGLOBAL )
				hDest = stgMedium.hGlobal;
		}else{
			if( stgMedium.tymed == TYMED_HGLOBAL ){
				DWORD nSize = ::GlobalSize( stgMedium.hGlobal );
				hDest = ::GlobalAlloc( GMEM_SHARE|GMEM_MOVEABLE, nSize );
				if( hDest != NULL ){
					// copy the bits
					LPVOID lpSource = ::GlobalLock( stgMedium.hGlobal );
					LPVOID lpDest = ::GlobalLock( hDest );
					memcpy( lpDest, lpSource, nSize );
					::GlobalUnlock( hDest );
					::GlobalUnlock( stgMedium.hGlobal );
				}
			}
			::ReleaseStgMedium( &stgMedium );
		}
	}
	return hDest;
}


/*
|| ウィンドウプロシージャ
||
*/
LRESULT CALLBACK EditViewWndProc(
	HWND		hwnd,	// handle of window
	UINT		uMsg,	// message identifier
	WPARAM		wParam,	// first message parameter
	LPARAM		lParam 	// second message parameter
)
{
	CEditView*	pCEdit;
	switch( uMsg ){
	case WM_CREATE:
		pCEdit = ( CEditView* )g_m_pcEditView;
		return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		pCEdit = ( CEditView* )::GetWindowLongPtr( hwnd, 0 );
		if( NULL != pCEdit ){
			//	May 16, 2000 genta
			//	From Here
			if( uMsg == WM_COMMAND ){
				::SendMessage( ::GetParent( pCEdit->m_hwndParent ), WM_COMMAND, wParam,  lParam );
			}
			else{
				return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
			}
			//	To Here
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}


/*
||  タイマーメッセージのコールバック関数
||
||	現在は、マウスによる領域選択時のスクロール処理のためにタイマーを使用しています。
*/
VOID CALLBACK EditViewTimerProc(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
)
{
	CEditView*	pCEditView;
	pCEditView = ( CEditView* )::GetWindowLongPtr( hwnd, 0 );
	if( NULL != pCEditView ){
		pCEditView->OnTimer( hwnd, uMsg, idEvent, dwTime );
	}
	return;
}



//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CEditView::CEditView() :
	m_cHistory( new CAutoMarkMgr ),
	m_bActivateByMouse( FALSE ),	// 2007.10.02 nasukoji
	m_cRegexKeyword( NULL )				// 2007.04.08 ryoji
// 20020331 aroka 再変換対応 for 95/NT
// 2002.04.09 コンストラクタのなかに移動しました。 minfu
//	m_uMSIMEReconvertMsg( ::RegisterWindowMessage( RWM_RECONVERT ) ),
//	m_uATOKReconvertMsg( ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) )
{
	LOGFONT		lf;

	m_bDrawSWITCH = TRUE;
	m_pcDropTarget = new CDropTarget( this );
	m_bDragSource = FALSE;
	m_bDragMode = FALSE;					/* 選択テキストのドラッグ中か */
	m_bCurSrchKeyMark = FALSE;				/* 検索文字列 */
	//	Jun. 27, 2001 genta
	m_szCurSrchKey[0] = '\0';
	//strcpy( m_szCurSrchKey, "" );			/**/
	m_bCurSrchRegularExp = 0;				/* 検索／置換  1==正規表現 */
	m_bCurSrchLoHiCase = 0;					/* 検索／置換  1==英大文字小文字の区別 */
	m_bCurSrchWordOnly = 0;					/* 検索／置換  1==単語のみ検索 */

	m_bExecutingKeyMacro = FALSE;			/* キーボードマクロの実行中 */
	m_bPrevCommand = 0;
	m_nMyIndex = 0;

	//	Dec. 4, 2002 genta
	//	メニューバーへのメッセージ表示機能はCEditWndへ移管

	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();
	m_bCommandRunning = FALSE;	/* コマンドの実行中 */
	m_pcOpeBlk = NULL;			/* 操作ブロック */
	m_bDoing_UndoRedo = FALSE;	/* アンドゥ・リドゥの実行中か */
	m_pcsbwVSplitBox = NULL;	/* 垂直分割ボックス */
	m_pcsbwHSplitBox = NULL;	/* 水平分割ボックス */
	m_pszAppName = "EditorClient";
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_hwndVScrollBar = NULL;
	m_nVScrollRate = 1;			/* 垂直スクロールバーの縮尺 */
	m_hwndHScrollBar = NULL;
	m_hwndSizeBox = NULL;
	m_nCaretPosX = 0;			/* ビュー左端からのカーソル桁位置(０オリジン) */
	m_nCaretPosX_Prev = 0;		/* ビュー左端からのカーソル桁直前の位置(０オリジン) */
	m_nCaretPosY = 0;			/* ビュー上端からのカーソル行位置(０オリジン) */

	m_nCaretPosX_PHY = 0;		/* カーソル位置 改行単位行先頭からのバイト数(０開始) */
	m_nCaretPosY_PHY = 0;		/* カーソル位置 改行単位行の行番号(０開始) */

	m_nSrchStartPosX_PHY = -1;	/* 検索/置換開始時のカーソル位置  改行単位行先頭からのバイト数(0開始) */	// 02/06/26 ai
	m_nSrchStartPosY_PHY = -1;	/* 検索/置換開始時のカーソル位置  改行単位行の行番号(0開始) */				// 02/06/26 ai
	m_bSearch = FALSE;			/* 検索/置換開始位置を登録するか */											// 02/06/26 ai
	m_nBracketPairPosX_PHY = -1;/* 対括弧の位置 改行単位行先頭からのバイト数(0開始) */	// 02/12/13 ai
	m_nBracketPairPosY_PHY = -1;/* 対括弧の位置 改行単位行の行番号(0開始) */			// 02/12/13 ai
	m_nBracketCaretPosX_PHY = -1;	/* 03/02/18 ai */
	m_nBracketCaretPosY_PHY = -1;	/* 03/02/18 ai */
	m_bDrawBracketPairFlag = FALSE;	/* 03/02/18 ai */
	m_bDrawSelectArea = FALSE;	/* 選択範囲を描画したか */	// 02/12/13 ai

	m_nCaretWidth = 0;			/* キャレットの幅 */
	m_nCaretHeight = 0;			/* キャレットの高さ */
	m_crCaret = -1;				/* キャレットの色 */			// 2006.12.16 ryoji
	m_crBack = -1;				/* テキストの背景色 */			// 2006.12.16 ryoji
	m_hbmpCaret = NULL;			/* キャレット用ビットマップ */	// 2006.11.28 ryoji

	m_bSelectingLock = FALSE;	/* 選択状態のロック */
	m_bBeginSelect = FALSE;		/* 範囲選択中 */
	m_bBeginBoxSelect = FALSE;	/* 矩形範囲選択中 */
	m_bBeginLineSelect = FALSE;	/* 行単位選択中 */
	m_bBeginWordSelect = FALSE;	/* 単語単位選択中 */

	m_nSelectLineBgnFrom = -1;	/* 範囲選択開始行(原点) */
	m_nSelectColmBgnFrom = -1;	/* 範囲選択開始桁(原点) */
	m_nSelectLineBgnTo = -1;	/* 範囲選択開始行(原点) */
	m_nSelectColmBgnTo = -1;	/* 範囲選択開始桁(原点) */

	m_nSelectLineFrom = -1;		/* 範囲選択開始行 */
	m_nSelectColmFrom = -1;		/* 範囲選択開始桁 */
	m_nSelectLineTo = -1;		/* 範囲選択終了行 */
	m_nSelectColmTo = -1;		/* 範囲選択終了桁 */

	m_nSelectLineFromOld = 0;	/* 範囲選択開始行 */
	m_nSelectColmFromOld = 0;	/* 範囲選択開始桁 */
	m_nSelectLineToOld = 0;		/* 範囲選択終了行 */
	m_nSelectColmToOld = 0;		/* 範囲選択終了桁 */
	m_nViewAlignLeft = 0;		/* 表示域の左端座標 */
	m_nViewAlignLeftCols = 0;	/* 行番号域の桁数 */
	m_nTopYohaku = m_pShareData->m_Common.m_nRulerBottomSpace; 	/* ルーラーとテキストの隙間 */
	m_nViewAlignTop = m_nTopYohaku;		/* 表示域の上端座標 */

	/* ルーラー表示 */
	m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ルーラー高さ */
	m_nOldCaretPosX = 0;	// 前回描画したルーラーのキャレット位置 2002.02.25 Add By KK
	m_nOldCaretWidth = 0;	// 前回描画したルーラーのキャレット幅   2002.02.25 Add By KK
	m_bRedrawRuler = true;	// ルーラー全体を描き直す時=true   2002.02.25 Add By KK
	m_nViewCx = 0;				/* 表示域の幅 */
	m_nViewCy = 0;				/* 表示域の高さ */
	m_nViewColNum = 0;			/* 表示域の桁数 */
	m_nViewRowNum = 0;			/* 表示域の行数 */
	m_nViewTopLine = 0;			/* 表示域の一番上の行 */
	m_nViewLeftCol = 0;			/* 表示域の一番左の桁 */
	m_hdcCompatDC = NULL;		/* 再描画用コンパチブルＤＣ */
	m_hbmpCompatBMP = NULL;		/* 再描画用メモリＢＭＰ */
	m_hbmpCompatBMPOld = NULL;	/* 再描画用メモリＢＭＰ(OLD) */
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	m_nCompatBMPWidth = -1;
	m_nCompatBMPHeight = -1;
	// To Here 2007.09.09 Moca
	m_nCharWidth = 10;			/* 半角文字の幅 */
	m_nCharHeight = 18;			/* 文字の高さ */

	/* フォント作成 */
	m_hFont_HAN = CreateFontIndirect( &(m_pShareData->m_Common.m_lf) );

	/* 太字フォント作成 */
	lf = m_pShareData->m_Common.m_lf;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT = CreateFontIndirect( &lf );

	/* 下線フォント作成 */
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* 太字下線フォント作成 */
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT_UL = CreateFontIndirect( &lf );

	//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	//	2007.08.12 genta 初期化にShareDataの値が必要になった
	m_CurRegexp.Init(m_pShareData->m_Common.m_szRegexpLib );

	// 2004.02.08 m_hFont_ZENは未使用により削除
	m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	m_bInMenuLoop = FALSE;				/* メニュー モーダル ループに入っています */
//	MYTRACE( "CEditView::CEditView()おわり\n" );
	m_bHokan = FALSE;

	m_hFontOld = NULL;

	//	Aug. 31, 2000 genta
	m_cHistory->SetMax( 30 );

	// from here  2002.04.09 minfu OSによって再変換の方式を変える
	//	YAZAKI COsVersionInfoのカプセル化は守りましょ。
	COsVersionInfo	cOs;
//	POSVERSIONINFO pOsVer;
	
//	pOsVer =  cOs.GetOsVersionInfo();
	if( cOs.OsDoesNOTSupportReconvert() ){
		// 95 or NTならば
		m_uMSIMEReconvertMsg = ::RegisterWindowMessage( RWM_RECONVERT );
		m_uATOKReconvertMsg = ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) ;
		m_uWM_MSIME_RECONVERTREQUEST = ::RegisterWindowMessage("MSIMEReconvertRequest");
		
		m_hAtokModule = LoadLibrary("ATOK10WC.DLL");
		AT_ImmSetReconvertString = NULL;
		if ( NULL != m_hAtokModule ) {
			AT_ImmSetReconvertString =(BOOL (WINAPI *)( HIMC , int ,PRECONVERTSTRING , DWORD  ) ) GetProcAddress(m_hAtokModule,"AT_ImmSetReconvertString");
		}
	}else{ 
		// それ以外のOSのときはOS標準を使用する
		m_uMSIMEReconvertMsg = 0;
		m_uATOKReconvertMsg = 0 ;
		m_hAtokModule = 0;	//@@@ 2002.04.14 MIK
	}
	// to here  2002.04.10 minfu
	
// 2002/07/22 novice
//	m_bCaretShowFlag = false;

	//2004.10.23 isearch
	m_nISearchMode = 0;
	//m_pcmigemo = CMigemo::getInstance();
	//m_pcmigemo->Init();
	m_pcmigemo = NULL;

	// 2007.10.02 nasukoji
	m_dwTripleClickCheck = 0;		// トリプルクリックチェック用時刻初期化

	return;
}


CEditView::~CEditView()
{
	DeleteObject( m_hFont_HAN );
	DeleteObject( m_hFont_HAN_FAT );
	DeleteObject( m_hFont_HAN_UL );
	DeleteObject( m_hFont_HAN_FAT_UL );

	// キャレット用ビットマップ	// 2006.11.28 ryoji
	if( m_hbmpCaret != NULL )
		DeleteObject( m_hbmpCaret );

	if( m_hWnd != NULL ){
		DestroyWindow( m_hWnd );
	}

	/* 再描画用コンパチブルＤＣ */
	//	2007.09.30 genta 関数化
	//	m_hbmpCompatBMPもここで削除される．
	UseCompatibleDC(FALSE);

	delete m_pcDropTarget;
	m_pcDropTarget = NULL;

	delete m_cHistory;

	delete m_cRegexKeyword;	//@@@ 2001.11.17 add MIK
	
	//再変換 2002.04.10 minfu
	if(m_hAtokModule)
		FreeLibrary(m_hAtokModule);
	
	return;
}



BOOL CEditView::Create(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	CEditDoc*	pcEditDoc,
	int			nMyIndex,
	BOOL		bShow
)
{
	WNDCLASS	wc;
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;
	m_pcEditDoc = pcEditDoc;
	m_nMyIndex = nMyIndex;

	//	2007.08.18 genta 初期化にShareDataの値が必要になった
	m_cRegexKeyword = new CRegexKeyword( m_pShareData->m_Common.m_szRegexpLib );	//@@@ 2001.11.17 add MIK
	m_cRegexKeyword->RegexKeySetTypes(&(m_pcEditDoc->GetDocumentAttribute()));	//@@@ 2001.11.17 add MIK

	m_nTopYohaku = m_pShareData->m_Common.m_nRulerBottomSpace; 	/* ルーラーとテキストの隙間 */
	m_nViewAlignTop = m_nTopYohaku;								/* 表示域の上端座標 */
	/* ルーラー表示 */
//	if( m_pShareData->m_Common.m_bRulerDisp ){
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ルーラー高さ */
	}


	/* ウィンドウクラスの登録 */
	//	Apr. 27, 2000 genta
	//	サイズ変更時のちらつきを抑えるためCS_HREDRAW | CS_VREDRAW を外した
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= (WNDPROC)EditViewWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof( LONG_PTR );
	wc.hInstance		= m_hInstance;
	wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_IBEAM )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_WINDOW + 1)*/;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= m_pszAppName;
	if( 0 == ::RegisterClass( &wc ) ){
	}
	/* エディタウィンドウの作成 */
	g_m_pcEditView = this;
	m_hWnd = ::CreateWindowEx(
		0
		/*| WS_EX_CLIENTEDGE */
		| WS_EX_STATICEDGE
		,	// extended window style

		m_pszAppName,			// pointer to registered class name
		m_pszAppName,			// pointer to window name
		0
		| WS_VISIBLE
		| WS_CHILD
		| WS_CLIPCHILDREN
		/*| WS_BORDER*/
		, // window style

		CW_USEDEFAULT,			// horizontal position of window
		0,						// vertical position of window
		CW_USEDEFAULT,			// window width
		0,						// window height
		hwndParent,				// handle to parent or owner window
		NULL,					// handle to menu or child-window identifier
		m_hInstance,			// handle to application instance
		(LPVOID)this			// pointer to window-creation data
	);
	if( NULL == m_hWnd ){
		return FALSE;
	}

//	CDropTarget::Register_DropTarget( m_hWnd );
	m_pcDropTarget->Register_DropTarget( m_hWnd );
//	::OleInitialize(NULL);
//	if( FAILED( ::RegisterDragDrop( m_hWnd, this ) ) ){
//		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
//			"::RegisterDragDrop()\n失敗"
//		);
//
//	}

	/* 辞書Tip表示ウィンドウ作成 */
	m_cTipWnd.Create( m_hInstance, m_hWnd/*m_pShareData->m_hwndTray*/ );

	/* 再描画用コンパチブルＤＣ */
	// 2007.09.09 Moca 互換BMPによる画面バッファ
	// 2007.09.30 genta 関数化
	UseCompatibleDC( m_pShareData->m_Common.m_bUseCompotibleBMP );

	/* 垂直分割ボックス */
	m_pcsbwVSplitBox = new CSplitBoxWnd;
	m_pcsbwVSplitBox->Create( m_hInstance, m_hWnd, TRUE );
	/* 水平分割ボックス */
	m_pcsbwHSplitBox = new CSplitBoxWnd;
	m_pcsbwHSplitBox->Create( m_hInstance, m_hWnd, FALSE );

	/* スクロールバー作成 */
	CreateScrollBar();		// 2006.12.19 ryoji

	SetFont();
	/* スクロールバーの状態を更新する */
//	AdjustScrollBars();

	if( bShow ){
		ShowWindow( m_hWnd, SW_SHOW );
	}

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();

//	/* 入力補完ウィンドウ作成 */
//	m_cHokanMgr.DoModeless( m_hInstance , m_hWnd, (LPARAM)this );

	/* キーボードの現在のリピート間隔を取得 */
	int nKeyBoardSpeed;
	SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );
//	nKeyBoardSpeed *= 2;
	/* タイマー起動 */
	if( 0 == ::SetTimer( m_hWnd, IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc ) ){
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
			"CEditView::Create()\nタイマーが起動できません。\nシステムリソースが不足しているのかもしれません。"
		);
	}

	/* アンダーライン */
	m_cUnderLine.SetView( this );
	return TRUE;
}

/** 画面キャッシュ用CompatibleDCを用意する

	@param[in] TRUE: 画面キャッシュON

	@date 2007.09.30 genta 関数化
*/
void CEditView::UseCompatibleDC(BOOL fCache)
{
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if( fCache ){
		if( m_hdcCompatDC == NULL ){
			HDC			hdc;
			hdc = ::GetDC( m_hWnd );
			m_hdcCompatDC = ::CreateCompatibleDC( hdc );
			::ReleaseDC( m_hWnd, hdc );
#ifdef _DEBUG
			MYTRACE("CEditView::UseCompatibleDC: Created\n", fCache);
#endif
		}
		else {
#ifdef _DEBUG
			MYTRACE("CEditView::UseCompatibleDC: Reused\n", fCache);
#endif
		}
	}
	else {
		//	CompatibleBitmapが残っているかもしれないので最初に削除
		DeleteCompatibleBitmap();
		if( m_hdcCompatDC != NULL ){
			::DeleteDC( m_hdcCompatDC );
#ifdef _DEBUG
			MYTRACE("CEditView::UseCompatibleDC: Deleted.\n");
#endif
			m_hdcCompatDC = NULL;
		}
	}
}

/*! スクロールバー作成
	@date 2006.12.19 ryoji 新規作成（CEditView::Createから分離）
*/
BOOL CEditView::CreateScrollBar( void )
{
	SCROLLINFO	si;

	/* スクロールバーの作成 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles */
		"SCROLLBAR",						/* scroll bar control class */
		(LPSTR) NULL,						/* text for window title bar */
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles */
		0,									/* horizontal position */
		0,									/* vertical position */
		200,								/* width of the scroll bar */
		CW_USEDEFAULT,						/* default height */
		m_hWnd,								/* handle of main window */
		(HMENU) NULL,						/* no menu for a scroll bar */
		m_hInstance,						/* instance owning this window */
		(LPVOID) NULL						/* pointer not needed */
	);
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin  = 0;
	si.nMax  = 29;
	si.nPage = 10;
	si.nPos  = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndVScrollBar, SB_CTL, TRUE );

	/* スクロールバーの作成 */
	m_hwndHScrollBar = NULL;
	if( m_pShareData->m_Common.m_bScrollBarHorz ){	/* 水平スクロールバーを使う */
		m_hwndHScrollBar = ::CreateWindowEx(
			0L,									/* no extended styles */
			"SCROLLBAR",						/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd,								/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = 29;
		si.nPage = 10;
		si.nPos  = 0;
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
		::ShowScrollBar( m_hwndHScrollBar, SB_CTL, TRUE );
	}


	/* サイズボックス */
	if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* ファンクションキー表示位置／0:上 1:下 */
		m_hwndSizeBox = ::CreateWindowEx(
			WS_EX_CONTROLPARENT/*0L*/, 			/* no extended styles */
			"SCROLLBAR",						/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd, 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 								/* no extended styles */
			"STATIC",							/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD/* | SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd, 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}
	return TRUE;
}



/*! スクロールバー破棄
	@date 2006.12.19 ryoji 新規作成
*/
void CEditView::DestroyScrollBar( void )
{
	if( m_hwndVScrollBar )
	{
		::DestroyWindow( m_hwndVScrollBar );
		m_hwndVScrollBar = NULL;
	}

	if( m_hwndHScrollBar )
	{
		::DestroyWindow( m_hwndHScrollBar );
		m_hwndHScrollBar = NULL;
	}

	if( m_hwndSizeBox )
	{
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}
}





/*
|| メッセージディスパッチャ
*/
LRESULT CEditView::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{


	HDC			hdc;
	PAINTSTRUCT	ps;
//	int			nPos;
	int			nPosX;
	int			nPosY;
//	WORD		fwKeys;
//	short		zDelta;
//	short		xPos;
//	short		yPos;
//	int			i;
//	int			nScrollCode;
	switch ( uMsg ){
//-	case WM_DESTROYCLIPBOARD:
//-		MYTRACE( "CEditView WM_DESTROYCLIPBOARD\n" );
//-		return 0;
	case WM_MOUSEWHEEL:
		if( m_pcEditDoc->m_pcEditWnd->DoMouseWheel( wParam, lParam ) ){
			return 0L;
		}
		return OnMOUSEWHEEL( wParam, lParam );



	case WM_CREATE:
		::SetWindowLongPtr( hwnd, 0, (LONG_PTR) this );

//		/* キーボードの現在のリピート間隔を取得 */
//		int	nKeyBoardSpeed;
//		SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );
//		/* タイマー起動 */
//		::SetTimer( hwnd, IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc );

		return 0L;

	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	case WM_SHOWWINDOW:
		// ウィンドウ非表示の再に互換BMPを廃棄してメモリーを節約する
		if( hwnd == m_hWnd && (BOOL)wParam == FALSE ){
			DeleteCompatibleBitmap();
		}
		return 0L;
	// To Here 2007.09.09 Moca

	case WM_SIZE:
//		MYTRACE( "	WM_SIZE\n" );
		OnSize( LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;



	case WM_SETFOCUS:
//		MYTRACE( "	WM_SETFOCUS m_nMyIndex=%d\n", m_nMyIndex );
		OnSetFocus();

		/* 親ウィンドウのタイトルを更新 */
		SetParentCaption();

		return 0L;
	case WM_KILLFOCUS:
		OnKillFocus();

//		/* 親ウィンドウのタイトルを更新 */
//		SetParentCaption( TRUE );

		return 0L;
	case WM_CHAR:
//		MYTRACE( "WM_CHAR\n" );
		HandleCommand( F_CHAR, TRUE, wParam, 0, 0, 0 );
		return 0L;

	case WM_IME_NOTIFY:	// Nov. 26, 2006 genta
		if( wParam == IMN_SETCONVERSIONMODE || wParam == IMN_SETOPENSTATUS){
			ShowEditCaret();
		}
		return DefWindowProc( hwnd, uMsg, wParam, lParam );

	case WM_IME_COMPOSITION:
		if( IsInsMode() /* Oct. 2, 2005 genta */
		 &&	lParam & GCS_RESULTSTR
		){
			HIMC hIMC;
			DWORD dwSize;
			HGLOBAL hstr;
			LPSTR lpstr;
			hIMC = ImmGetContext( hwnd );

			if( !hIMC ){
				return 0;
//				MyError( ERROR_NULLCONTEXT );
			}

			// Get the size of the result string.
			dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);

			// increase buffer size for NULL terminator,
			//	maybe it is in Unicode
			dwSize += sizeof( WCHAR );

			hstr = GlobalAlloc( GHND, dwSize );
			if( hstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALALLOC );
			}

			lpstr = (LPSTR)GlobalLock( hstr );
			if( lpstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALLOCK );
			}

			// Get the result strings that is generated by IME into lpstr.
			ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);

//			MYTRACE( "lpstr=[%s]", lpstr );
			/* テキストを貼り付け */
			HandleCommand( F_INSTEXT, TRUE, (LPARAM)lpstr, TRUE, 0, 0 );

			ImmReleaseContext( hwnd, hIMC );

			// add this string into text buffer of application

			GlobalUnlock( hstr );
			GlobalFree( hstr );
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
//			return 0;
		}else{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	case WM_IME_CHAR:
		if( ! IsInsMode() /* Oct. 2, 2005 genta */ ){ /* 上書きモードか？ */
			HandleCommand( F_IME_CHAR, TRUE, wParam, 0, 0, 0 );
		}
		return 0L;

	// From Here 2008.03.24 Moca ATOK等の要求にこたえる
	case WM_PASTE:
		return HandleCommand( F_PASTE, TRUE, 0, 0, 0, 0 );

	case WM_COPY:
		return HandleCommand( F_COPY, TRUE, 0, 0, 0, 0 );
	// To Here 2008.03.24 Moca

//		if( NULL != m_pcOpeBlk ){	/* 操作ブロック */
//			return 1L;
//		}
//		/* コマンドの実行中 */
//		m_bCommandRunning = TRUE;
//
//		if( NULL != m_pcOpeBlk ){	/* 操作ブロック */
////			while( NULL != m_pcOpeBlk ){}
//			delete m_pcOpeBlk;
//		}
//		m_pcOpeBlk = new COpeBlk;
//
//		Command_IME_CHAR( (WORD)( (((WORD)wParam&0x00ff)<<8) | (((WORD)wParam&0xff00)>>8) ) ); /* ２バイト文字入力 */
//
//		/* アンドゥバッファの処理 */
//		if( 0 < m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
//			/* 操作の追加 */
//			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
//		}else{
//			delete m_pcOpeBlk;
//		}
//		m_pcOpeBlk = NULL;
//		m_bCommandRunning = FALSE;
//			return 0L;

	case WM_KEYUP:
		/* キーリピート状態 */
		m_bPrevCommand = 0;
		return 0L;

	// 2004.04.27 Moca From Here ALT+xでALTを押したままだとキーリピートがOFFにならない対策
	case WM_SYSKEYUP:
		m_bPrevCommand = 0;
		// 念のため呼ぶ
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	// 2004.04.27 To Here

//	case WM_MBUTTONDBLCLK:
	case WM_LBUTTONDBLCLK:

		// 2007.10.02 nasukoji	非アクティブウィンドウのダブルクリック時はここでカーソルを移動する
		// 2007.10.12 genta フォーカス移動のため，OnLBUTTONDBLCLKより移動
		if(m_bActivateByMouse){
			/* アクティブなペインを設定 */
			m_pcEditDoc->SetActivePane( m_nMyIndex );
			// カーソルをクリック位置へ移動する
			OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );	
			// 2007.10.02 nasukoji
			m_bActivateByMouse = FALSE;		// マウスによるアクティベートを示すフラグをOFF
		}
//		MYTRACE( " WM_LBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDBLCLK( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

// novice 2004/10/11 マウス中ボタン対応
	case WM_MBUTTONDOWN:
		OnMBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );

		return 0L;

	case WM_LBUTTONDOWN:
		// 2007.10.02 nasukoji
		m_bActivateByMouse = FALSE;		// マウスによるアクティベートを示すフラグをOFF
//		MYTRACE( " WM_LBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
//	case WM_MBUTTONUP:
	case WM_LBUTTONUP:

//		MYTRACE( " WM_LBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
	case WM_MOUSEMOVE:
		OnMOUSEMOVE( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_RBUTTONDBLCLK:
//		MYTRACE( " WM_RBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;
//	case WM_RBUTTONDOWN:
//		MYTRACE( " WM_RBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
//		OnRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
//		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
//			/* アクティブなペインを設定 */
//			m_pcEditDoc->SetActivePane( m_nMyIndex );
//		}
//		return 0L;
	case WM_RBUTTONUP:
//		MYTRACE( " WM_RBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnRBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

// novice 2004/10/10 マウスサイドボタン対応
	case WM_XBUTTONDOWN:
		switch ( HIWORD(wParam) ){
		case XBUTTON1:
			OnXLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		case XBUTTON2:
			OnXRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
			break;
		}

		return 0L;

	case WM_VSCROLL:
//		MYTRACE( "	WM_VSCROLL nPos=%d\n", GetScrollPos( m_hwndVScrollBar, SB_CTL ) );
		//	Sep. 11, 2004 genta 同期スクロールの関数化
		{
			int Scroll = OnVScroll(
				(int) LOWORD( wParam ), ((int) HIWORD( wParam )) * m_nVScrollRate );

			//	シフトキーが押されていないときだけ同期スクロール
			if(( ::GetKeyState( VK_SHIFT ) & 0x8000 ) == 0 ){
				SyncScrollV( Scroll );
			}
		}

		return 0L;

	case WM_HSCROLL:
//		MYTRACE( "	WM_HSCROLL nPos=%d\n", GetScrollPos( m_hwndHScrollBar, SB_CTL ) );
		//	Sep. 11, 2004 genta 同期スクロールの関数化
		{
			int Scroll = OnHScroll(
				(int) LOWORD( wParam ), ((int) HIWORD( wParam )) );

			//	シフトキーが押されていないときだけ同期スクロール
			if(( ::GetKeyState( VK_SHIFT ) & 0x8000 ) == 0 ){
				SyncScrollH( Scroll );
			}
		}

		return 0L;

	case WM_ENTERMENULOOP:
		m_bInMenuLoop = TRUE;	/* メニュー モーダル ループに入っています */

		/* 辞書Tipが起動されている */
		if( 0 == m_dwTipTimer ){
			/* 辞書Tipを消す */
			m_cTipWnd.Hide();
			m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
		}
		if( m_bHokan ){
			m_pcEditDoc->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		return 0L;

	case WM_EXITMENULOOP:
		m_bInMenuLoop = FALSE;	/* メニュー モーダル ループに入っています */
		return 0L;


	case WM_PAINT:
		hdc = ::BeginPaint( hwnd, &ps );
		OnPaint( hdc, &ps, FALSE );
		::EndPaint(hwnd, &ps);
		return 0L;

	case WM_CLOSE:
//		MYTRACE( "	WM_CLOSE\n" );
		DestroyWindow( hwnd );
		return 0L;
	case WM_DESTROY:
//		CDropTarget::Revoke_DropTarget();
		m_pcDropTarget->Revoke_DropTarget();
//		::RevokeDragDrop( m_hWnd );
//		::OleUninitialize();

		/* タイマー終了 */
		::KillTimer( m_hWnd, IDT_ROLLMOUSE );


//		MYTRACE( "	WM_DESTROY\n" );
		/*
		||子ウィンドウの破棄
		*/
		if( NULL != m_hwndVScrollBar ){	// Aug. 20, 2005 Aroka
			DestroyWindow( m_hwndVScrollBar );
			m_hwndVScrollBar = NULL;
		}
		if( NULL != m_hwndHScrollBar ){
			DestroyWindow( m_hwndHScrollBar );
			m_hwndHScrollBar = NULL;
		}
		if( NULL != m_hwndSizeBox ){
			DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		if( NULL != m_pcsbwVSplitBox ){	/* 垂直分割ボックス */
			delete m_pcsbwVSplitBox;
			m_pcsbwVSplitBox = NULL;
		}

		if( NULL != m_pcsbwHSplitBox ){	/* 垂直分割ボックス */
			delete m_pcsbwHSplitBox;
			m_pcsbwHSplitBox = NULL;
		}


		m_hWnd = NULL;
		return 0L;

	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE( "MYWM_DOSPLIT nPosX=%d nPosY=%d\n", nPosX, nPosY );
		::SendMessage( m_hwndParent, MYWM_DOSPLIT, wParam, lParam );
		return 0L;

	case MYWM_SETACTIVEPANE:
		m_pcEditDoc->SetActivePane( m_nMyIndex );
		::PostMessage( m_hwndParent, MYWM_SETACTIVEPANE, (WPARAM)m_nMyIndex, 0 );
		return 0L;

	case MYWM_IME_REQUEST:  /* 再変換  by minfu 2002.03.27 */ // 20020331 aroka
		
		//if( (wParam == IMR_RECONVERTSTRING) &&  IsTextSelected() && ( !m_bBeginBoxSelect) ){
		//	// lParamにIMEが用意した再変換用構造体のポインタが入っている。
		//	return RequestedReconversion((PRECONVERTSTRING)lParam);
		//}
		// 2002.04.09 switch case に変更  minfu 
		switch ( wParam ){
		case IMR_RECONVERTSTRING:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, false);
			
		case IMR_CONFIRMRECONVERTSTRING:
			return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, false);
			
		}
		
		return 0L;

	// 2007.10.02 nasukoji	マウスクリックにてアクティベートされた時はカーソル位置を移動しない
	case WM_MOUSEACTIVATE:
		LRESULT nRes;
		nRes = ::DefWindowProc( hwnd, uMsg, wParam, lParam );	// 親に先に処理させる
		if( nRes == MA_NOACTIVATE || nRes == MA_NOACTIVATEANDEAT ){
			return nRes;
		}

		// マウスクリックによりバックグラウンドウィンドウがアクティベートされた
		//	2007.10.08 genta オプション追加
		if( m_pShareData->m_Common.m_bNoCaretMoveByActivation &&
		   (! m_pcEditDoc->m_pcEditWnd->IsActiveApp()))
		{
			m_bActivateByMouse = TRUE;		// マウスによるアクティベート
			return MA_ACTIVATEANDEAT;		// アクティベート後イベントを破棄
		}

		/* アクティブなペインを設定 */
		m_pcEditDoc->SetActivePane( m_nMyIndex );

		return nRes;
	
	default:
// << 20020331 aroka 再変換対応 for 95/NT
		if( (m_uMSIMEReconvertMsg && (uMsg == m_uMSIMEReconvertMsg)) 
			|| (m_uATOKReconvertMsg && (uMsg == m_uATOKReconvertMsg))){
		// 2002.04.08 switch case に変更 minfu 
			switch ( wParam ){
			case IMR_RECONVERTSTRING:
				return SetReconvertStruct((PRECONVERTSTRING)lParam, true);
				
			case IMR_CONFIRMRECONVERTSTRING:
				return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, true);
				
			}
//		if( uMsg == m_uMSIMEReconvertMsg || uMsg == m_uATOKReconvertMsg){
//			if( (wParam == IMR_RECONVERTSTRING) &&  IsTextSelected() && ( !m_bBeginBoxSelect) ){
//				// lParamにIMEが用意した再変換用構造体のポインタが入っている。
//				return RequestedReconversionW((PRECONVERTSTRING)lParam);
//			}
			return 0L;
		}
// >> by aroka

		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




void CEditView::OnMove( int x, int y, int nWidth, int nHeight )
{
	MoveWindow( m_hWnd, x, y, nWidth, nHeight, TRUE );
	return;
}




/* ウィンドウサイズの変更処理 */
void CEditView::OnSize( int cx, int cy )
{
	if( NULL == m_hWnd 
		|| ( cx == 0 && cy == 0 ) ){
		// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
		// ウィンドウ無効時にも互換BMPを破棄する
		DeleteCompatibleBitmap();
		// To Here 2007.09.09 Moca
		return;
	}

	int	nCxHScroll;
	int	nCyHScroll;
	int	nCxVScroll;
	int	nCyVScroll;
	int	nVSplitHeight;	/* 垂直分割ボックスの高さ */
	int	nHSplitWidth;	/* 水平分割ボックスの幅 */


	nVSplitHeight = 0;	/* 垂直分割ボックスの高さ */
	nHSplitWidth = 0;	/* 水平分割ボックスの幅 */

	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );



	/* 垂直分割ボックス */
	if( NULL != m_pcsbwVSplitBox ){
		nVSplitHeight = 7;
		::MoveWindow( m_pcsbwVSplitBox->m_hWnd, cx - nCxVScroll , 0, nCxVScroll, nVSplitHeight, TRUE );
	}
	/* 水平分割ボックス */
	if( NULL != m_pcsbwHSplitBox ){
		nHSplitWidth = 7;
		::MoveWindow( m_pcsbwHSplitBox->m_hWnd,0, cy - nCyHScroll, nHSplitWidth, nCyHScroll, TRUE );
	}
	/* 垂直スクロールバー */
	if( NULL != m_hwndVScrollBar ){
		::MoveWindow( m_hwndVScrollBar, cx - nCxVScroll , 0 + nVSplitHeight, nCxVScroll, cy - nCyVScroll - nVSplitHeight, TRUE );
	}
	/* 水平スクロールバー */
	if( NULL != m_hwndHScrollBar ){
		::MoveWindow( m_hwndHScrollBar, 0 + nHSplitWidth, cy - nCyHScroll, cx - nCxVScroll - nHSplitWidth, nCyHScroll, TRUE );
	}

	/* サイズボックス */
	if( NULL != m_hwndSizeBox ){
		::MoveWindow( m_hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE );
	}else{
	}


	m_nViewCx = cx - nCxVScroll - m_nViewAlignLeft;														/* 表示域の幅 */
	m_nViewCy = cy - ((NULL != m_hwndHScrollBar)?nCyHScroll:0) - m_nViewAlignTop;						/* 表示域の高さ */
	m_nViewColNum = m_nViewCx / ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );	/* 表示域の桁数 */
	m_nViewRowNum = m_nViewCy / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );	/* 表示域の行数 */

//1999.12.1 仕様変更
//	/* ウィンドウリサイズ時にキャレット位置へスクロール */
//	MoveCursor( m_nCaretPosX, m_nCaretPosY, TRUE );

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* 再描画用メモリＢＭＰ */
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if( m_hdcCompatDC != NULL ){
		CreateOrUpdateCompatibleBitmap( cx, cy );
 	}
	// To Here 2007.09.09 Moca

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();	//	[Q] genta 本当に必要？

	return;
}

/*!
	画面の互換ビットマップを作成または更新する。
		必要の無いときは何もしない。
	
	@param cx ウィンドウの高さ
	@param cy ウィンドウの幅
	@return true: ビットマップを利用可能 / false: ビットマップの作成・更新に失敗

	@date 2007.09.09 Moca CEditView::OnSizeから分離。
		単純に生成するだけだったものを、仕様変更に従い内容コピーを追加。
		サイズが同じときは何もしないように変更

	@par 互換BMPにはキャレット・カーソル位置横縦線・対括弧以外の情報を全て書き込む。
		選択範囲変更時の反転処理は、画面と互換BMPの両方を別々に変更する。
		カーソル位置横縦線変更時には、互換BMPから画面に元の情報を復帰させている。

*/
bool CEditView::CreateOrUpdateCompatibleBitmap( int cx, int cy )
{
	if( NULL == m_hdcCompatDC ){
		return false;
	}
	// サイズを64の倍数で整列
	int nBmpWidthNew  = ((cx + 63) & (0x7fffffff - 63));
	int nBmpHeightNew = ((cy + 63) & (0x7fffffff - 63));
	if( nBmpWidthNew != m_nCompatBMPWidth || nBmpHeightNew != m_nCompatBMPHeight ){
#ifdef _DEBUG
	MYTRACE( "CEditView::CreateOrUpdateCompatibleBitmap( %d, %d ): resized\n", cx, cy );
#endif
		HDC	hdc = ::GetDC( m_hWnd );
		HBITMAP hBitmapNew = NULL;
		if( m_hbmpCompatBMP ){
			// BMPの更新
			HDC hdcTemp = ::CreateCompatibleDC( hdc );
			hBitmapNew = ::CreateCompatibleBitmap( hdc, nBmpWidthNew, nBmpHeightNew );
			if( hBitmapNew ){
				HBITMAP hBitmapOld = (HBITMAP)::SelectObject( hdcTemp, hBitmapNew );
				// 前の画面内容をコピーする
				::BitBlt( hdcTemp, 0, 0,
					__min( nBmpWidthNew,m_nCompatBMPWidth ),
					__min( nBmpHeightNew, m_nCompatBMPHeight ),
					m_hdcCompatDC, 0, 0, SRCCOPY );
				::SelectObject( hdcTemp, hBitmapOld );
				::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
				::DeleteObject( m_hbmpCompatBMP );
			}
			::DeleteDC( hdcTemp );
		}else{
			// BMPの新規作成
			hBitmapNew = ::CreateCompatibleBitmap( hdc, nBmpWidthNew, nBmpHeightNew );
		}
		if( hBitmapNew ){
			m_hbmpCompatBMP = hBitmapNew;
			m_nCompatBMPWidth = nBmpWidthNew;
			m_nCompatBMPHeight = nBmpHeightNew;
			m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_hdcCompatDC, m_hbmpCompatBMP );
		}else{
			// 互換BMPの作成に失敗
			// 今後も失敗を繰り返す可能性が高いので
			// m_hdcCompatDCをNULLにすることで画面バッファ機能をこのウィンドウのみ無効にする。
			//	2007.09.29 genta 関数化．既存のBMPも解放
			UseCompatibleDC(FALSE);
		}
		::ReleaseDC( m_hWnd, hdc );
	}
	return NULL != m_hbmpCompatBMP;
}


/*!
	互換メモリBMPを削除

	@note 分割ビューが非表示になった場合と
		親ウィンドウが非表示・最小化された場合に削除される。
	@date 2007.09.09 Moca 新規作成 
*/
void CEditView::DeleteCompatibleBitmap()
{
	if( m_hbmpCompatBMP ){
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
		::DeleteObject( m_hbmpCompatBMP );
		m_hbmpCompatBMP = NULL;
		m_hbmpCompatBMPOld = NULL;
		m_nCompatBMPWidth = -1;
		m_nCompatBMPHeight = -1;
	}
}



/*!	IME ONか

	@date  2006.12.04 ryoji 新規作成（関数化）
*/
bool CEditView::IsImeON( void )
{
	bool bRet;
	HIMC	hIme;
	DWORD	conv, sent;

	//	From here Nov. 26, 2006 genta
	hIme = ImmGetContext( m_hwndParent );
	if( ImmGetOpenStatus( hIme ) != FALSE ){
		ImmGetConversionStatus( hIme, &conv, &sent );
//#ifdef _DEBUG
//	MYTRACE( "ImeMode = %X\n", conv );
//#endif
		if(( conv & IME_CMODE_NOCONVERSION ) == 0 ){
			bRet = true;
		}
		else {
			bRet = false;
		}
	}
	else {
		bRet = false;
	}
	ImmReleaseContext( m_hwndParent, hIme );
	//	To here Nov. 26, 2006 genta

	return bRet;
}


/*!	キャレットの作成

	@param nCaretColor [in]	キャレットの色種別 (0:通常, 1:IME ON)
	@param nWidth [in]		キャレット幅
	@param nHeight [in]		キャレット高

	@date 2006.12.07 ryoji 新規作成
*/
void CEditView::CreateEditCaret( COLORREF crCaret, COLORREF crBack, int nWidth, int nHeight )
{
	//
	// キャレット用のビットマップを作成する
	//
	// Note: ウィンドウ互換のメモリ DC 上で PatBlt を用いてキャレット色と背景色を XOR 結合
	//       することで，目的のビットマップを得る．
	//       ※ 256 色環境では RGB 値を単純に直接演算してもキャレット色を出すための正しい
	//          ビットマップ色は得られない．
	//       参考: [HOWTO] キャレットの色を制御する方法
	//             http://support.microsoft.com/kb/84054/ja
	//

	HBITMAP hbmpCaret;	// キャレット用のビットマップ

	HDC hdc = ::GetDC( m_hWnd );

	hbmpCaret = ::CreateCompatibleBitmap( hdc, nWidth, nHeight );
	HDC hdcMem = ::CreateCompatibleDC( hdc );
	HBITMAP hbmpOld = (HBITMAP)::SelectObject( hdcMem, hbmpCaret );
	HBRUSH hbrCaret = ::CreateSolidBrush( crCaret );
	HBRUSH hbrBack = ::CreateSolidBrush( crBack );
	HBRUSH hbrOld = (HBRUSH)::SelectObject( hdcMem, hbrCaret );
	::PatBlt( hdcMem, 0, 0, nWidth, nHeight, PATCOPY );
	::SelectObject( hdcMem, hbrBack );
	::PatBlt( hdcMem, 0, 0, nWidth, nHeight, PATINVERT );
	::SelectObject( hdcMem, hbrOld );
	::SelectObject( hdcMem, hbmpOld );
	::DeleteObject( hbrCaret );
	::DeleteObject( hbrBack );
	::DeleteDC( hdcMem );

	::ReleaseDC( m_hWnd, hdc );

	// 以前のビットマップを破棄する
	if( m_hbmpCaret != NULL )
		::DeleteObject( m_hbmpCaret );
	m_hbmpCaret = hbmpCaret;

	// キャレットを作成する
	::CreateCaret( m_hWnd, hbmpCaret, nWidth, nHeight );
	return;
}


/* キャレットの表示・更新 */
void CEditView::ShowEditCaret( void )
{
	const char*		pLine;
	int				nLineLen;
	int				nCaretWidth = 0;
	int				nCaretHeight = 0;
	int				nIdxFrom;
	int				nCharChars;
//	HDC				hdc;
//	const CLayout*	pcLayout;
//	HPEN			hPen, hPenOld;


//if( m_nMyIndex == 0 && m_nCaretPosX == 0 && m_nCaretPosY == 0 ){
//	MYTRACE( "ShowEditCaret() m_nMyIndex=%d m_nCaretWidth=%d\n", m_nMyIndex, m_nCaretWidth );
//}

/*
		なんかフレームウィンドウがアクティブでないときに内部的にカーソル移動すると
		カーソルがないのに、カーソルがあるということになってしまう
		のでアクティブにしてもカーソルが出てこないときがある
		フレームウィンドウがアクティブでないときは、カーソルがないことにする
*/
	if( ::GetActiveWindow() != ::GetParent( m_hwndParent ) ){
		m_nCaretWidth = 0;
//		MYTRACE( "アクティブでないのにカーソル作っちゃったから消しちゃった。\n" );
		return;
	}

	/* アクティブなペインを取得 */
	if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
		m_nCaretWidth = 0;
//if( m_nMyIndex == 0 && m_nCaretPosX == 0 && m_nCaretPosY == 0 ){
//	MYTRACE( "m_nMyIndex[%s] != m_pcEditDoc->GetActivePane()\n", m_nMyIndex, m_pcEditDoc->GetActivePane() );
//}
		return;
	}
	/* キャレットの幅、高さを決定 */
	if( 0 == m_pShareData->m_Common.GetCaretType() ){	/* カーソルのタイプ 0=win 1=dos */
		nCaretHeight = m_nCharHeight;					/* キャレットの高さ */
		if( IsInsMode() /* Oct. 2, 2005 genta */ ){
			nCaretWidth = 2;
		}else{
			const CLayout* pcLayout;
			nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom = LineColmnToIndex( pcLayout, m_nCaretPosX );
				if( nIdxFrom >= nLineLen ||
					pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
					pLine[nIdxFrom] == TAB ){
					nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				}else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nIdxFrom );
					if( 0 < nCharChars ){
						nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * nCharChars;
					}
				}
			}
		}
	}else
	if( 1 == m_pShareData->m_Common.GetCaretType() ){	/* カーソルのタイプ 0=win 1=dos */
		if( IsInsMode() /* Oct. 2, 2005 genta */ ){
			nCaretHeight = m_nCharHeight / 2;			/* キャレットの高さ */
		}else{
			nCaretHeight = m_nCharHeight;				/* キャレットの高さ */
		}
		const CLayout* pcLayout;
		nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdxFrom = LineColmnToIndex( pcLayout, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ||
				pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
				pLine[nIdxFrom] == TAB ){
				nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			}else{
				// 2005-09-02 D.S.Koba GetSizeOfChar
				nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nIdxFrom );
				if( 0 < nCharChars ){
					nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * nCharChars;
				}
			}
		}

	}

#if 0
	hdc = ::GetDC( m_hWnd );
#endif
	//	キャレット色の取得
	ColorInfo* ColorInfoArr = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr;
	int nCaretColor = ( ColorInfoArr[COLORIDX_CARET_IME].m_bDisp && IsImeON() )? COLORIDX_CARET_IME: COLORIDX_CARET;
	COLORREF crCaret = ColorInfoArr[nCaretColor].m_colTEXT;
	COLORREF crBack = ColorInfoArr[COLORIDX_TEXT].m_colBACK;

	if( m_nCaretWidth == 0 ){
		/* キャレットがなかった場合 */
		/* キャレットの作成 */
		CreateEditCaret( crCaret, crBack, nCaretWidth, nCaretHeight );	// 2006.12.07 ryoji
		m_bCaretShowFlag = false; // 2002/07/22 novice
	}else{
		if( m_nCaretWidth != nCaretWidth || m_nCaretHeight != nCaretHeight ||
			m_crCaret != crCaret || m_crBack != crBack ){
			/* キャレットはあるが、大きさや色が変わった場合 */
			/* 現在のキャレットを削除 */
			::DestroyCaret();

			/* キャレットの作成 */
			CreateEditCaret( crCaret, crBack, nCaretWidth, nCaretHeight );	// 2006.12.07 ryoji
			m_bCaretShowFlag = false; // 2002/07/22 novice
		}else{
			/* キャレットはあるし、大きさも変わっていない場合 */
			/* キャレットを隠す */
			HideCaret_( m_hWnd ); // 2002/07/22 novice
		}
	}
	/* キャレットの位置を調整 */
	int nPosX = m_nViewAlignLeft + (m_nCaretPosX - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nPosY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight - nCaretHeight;
	::SetCaretPos( nPosX, nPosY );
	if ( m_nViewAlignLeft <= nPosX && m_nViewAlignTop <= nPosY ){
		/* キャレットの表示 */
		ShowCaret_( m_hWnd ); // 2002/07/22 novice
	}

	m_nCaretWidth = nCaretWidth;
	m_nCaretHeight = nCaretHeight;	/* キャレットの高さ */
	m_crCaret = crCaret;	//	2006.12.07 ryoji
	m_crBack = crBack;		//	2006.12.07 ryoji
	SetIMECompFormPos();

#if 0
	2002/05/12 YAZAKI ShowEditCaretで仕事をしすぎ。
	//2002.02.27 Add By KK アンダーラインのちらつきを低減
	if (m_nOldUnderLineY != m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight) {
		//アンダーラインの描画位置が、前回のアンダーライン描画位置と異なっていたら、アンダーラインを描き直す。
		CaretUnderLineOFF(TRUE);
		CaretUnderLineON(TRUE);
	}

	/* ルーラー描画 */
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );
#endif

	return;
}





/* 入力フォーカスを受け取ったときの処理 */
void CEditView::OnSetFocus( void )
{
//NG	/* 1999.11.15 */
//NG	::SetFocus( m_hwndParent );
//NG	::SetFocus( m_hWnd );

	// 2004.04.02 Moca EOFのみのレイアウト行は、0桁目のみ有効.EOFより下の行のある場合は、EOF位置にする
	{
		int nPosX = m_nCaretPosX;
		int nPosY = m_nCaretPosY;
		if( GetAdjustCursorPos( &nPosX, &nPosY ) ){
			MoveCursor( nPosX, nPosY, FALSE );
			m_nCaretPosX_Prev = m_nCaretPosX;
		}
	}

	ShowEditCaret();

//	SetIMECompFormPos();	YAZAKI ShowEditCaretで作業済み
	SetIMECompFormFont();

	/* ルーラのカーソルをグレーから黒に変更する */
	HDC hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );

	// 03/02/18 対括弧の強調表示(描画) ai
	m_bDrawBracketPairFlag = TRUE;
	DrawBracketPair( true );

	return;
}





/* 入力フォーカスを失ったときの処理 */
void CEditView::OnKillFocus( void )
{
	// 03/02/18 対括弧の強調表示(消去) ai
	DrawBracketPair( false );
	m_bDrawBracketPairFlag = FALSE;

	DestroyCaret();

	/* ルーラー描画 */
	/* ルーラのカーソルを黒からグレーに変更する */
	HDC	hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );

	/* 辞書Tipが起動されている */
	if( 0 == m_dwTipTimer ){
		/* 辞書Tipを消す */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	}

	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	return;
}





/*! 垂直スクロールバーメッセージ処理

	@param nScrollCode [in]	スクロール種別 (Windowsから渡されるもの)
	@param nPos [in]		スクロール位置(THUMBTRACK用)
	@retval	実際にスクロールした行数

	@date 2004.09.11 genta スクロール行数を返すように．
		未使用のhwndScrollBar引数削除．
*/
int CEditView::OnVScroll( int nScrollCode, int nPos )
{
	int nScrollVal = 0;

	switch( nScrollCode ){
	case SB_LINEDOWN:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( m_nViewTopLine + 1 );
//		}
		nScrollVal = ScrollAtV( m_nViewTopLine + m_pShareData->m_Common.m_nRepeatedScrollLineNum );
		break;
	case SB_LINEUP:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( m_nViewTopLine - 1 );
//		}
		nScrollVal = ScrollAtV( m_nViewTopLine - m_pShareData->m_Common.m_nRepeatedScrollLineNum );
		break;
	case SB_PAGEDOWN:
		nScrollVal = ScrollAtV( m_nViewTopLine + m_nViewRowNum );
		break;
	case SB_PAGEUP:
		nScrollVal = ScrollAtV( m_nViewTopLine - m_nViewRowNum );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtV( nPos );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtV( nPos );
		break;
	case SB_TOP:
		nScrollVal = ScrollAtV( 0 );
		break;
	case SB_BOTTOM:
		nScrollVal = ScrollAtV(( m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) - m_nViewRowNum );
		break;
	default:
		break;
	}
	return nScrollVal;
}

/*! 水平スクロールバーメッセージ処理

	@param nScrollCode [in]	スクロール種別 (Windowsから渡されるもの)
	@param nPos [in]		スクロール位置(THUMBTRACK用)
	@retval	実際にスクロールした桁数

	@date 2004.09.11 genta スクロール桁数を返すように．
		未使用のhwndScrollBar引数削除．
*/
int CEditView::OnHScroll( int nScrollCode, int nPos )
{
	int nScrollVal = 0;

	m_bRedrawRuler = true; // YAZAKI
	switch( nScrollCode ){
	case SB_LINELEFT:
		nScrollVal = ScrollAtH( m_nViewLeftCol - 4 );
		break;
	case SB_LINERIGHT:
		nScrollVal = ScrollAtH( m_nViewLeftCol + 4 );
		break;
	case SB_PAGELEFT:
		nScrollVal = ScrollAtH( m_nViewLeftCol - m_nViewColNum );
		break;
	case SB_PAGERIGHT:
		nScrollVal = ScrollAtH( m_nViewLeftCol + m_nViewColNum );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtH( nPos );
//		MYTRACE( "nPos=%d\n", nPos );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtH( nPos );
//		MYTRACE( "nPos=%d\n", nPos );
		break;
	case SB_LEFT:
		nScrollVal = ScrollAtH( 0 );
		break;
	case SB_RIGHT:
		//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
		nScrollVal = ScrollAtH( m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() - m_nViewColNum );
		break;
	}
	return nScrollVal;
}



//void CEditView::Draw3dRect( HDC hdc, int x, int y, int cx, int cy,
//	COLORREF clrTopLeft, COLORREF clrBottomRight )
//{
//	return;
//}
//
//void CEditView::FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr )
//{
//	RECT	rc;
//	::SetBkColor( hdc, clr );
//	::SetRect( &rc, x, y, x + cx, y + cy );
//	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
//	return;
//}

#if 0//////////////////////////////////////////////
/************************************************************************
 *
 * 	関数:	OpenDIB( LPSTR szFile )
 *
 * 	目的:	DIBファイルを開いてメモリDIBを作成します。また、BITMAPINFO、
 *	パレットデータ、ビットを含むメモリハンドルを作成します。
 *
 *	戻り値:	DIBを識別するハンドル
 *
 ************************************************************************/
HANDLE CEditView::OpenDIB ( LPCSTR szFile )
{
	unsigned			fh;
	BITMAPINFOHEADER	bi;
	LPBITMAPINFOHEADER	lpbi;
	DWORD				dwLen = 0;
	DWORD				dwBits;
	HANDLE				hdib;
	HANDLE				h;
	OFSTRUCT			of;

	/* ファイルを開いてDIB情報を読み取る */
	fh = OpenFile( szFile, &of, OF_READ );
	if( fh == -1 )
		return NULL;

	hdib = ReadDibBitmapInfo( fh );
	if( !hdib )
		return NULL;
	DibInfo( hdib, &bi );

	/* DIBの保持に必要なメモリ量を計算 */
	dwBits = bi.biSizeImage;
	dwLen  = bi.biSize + (DWORD)PaletteSize( &bi ) + dwBits;

	/* DIBを保持するビットマップ情報バッファサイズを増やす */
	h = GlobalReAlloc( hdib, dwLen, GHND );
	if( !h ){
		GlobalFree( hdib );
		hdib = NULL;
	}
	else
		hdib = h;
	/* ビットを読み取る */
	if( hdib ){
		lpbi = (LPBITMAPINFOHEADER)GlobalLock( hdib );
		lread( fh, (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi), dwBits );
		GlobalUnlock( hdib );
	}
	_lclose( fh );
	return hdib;
}

/************************************************************************
 *
 *	関数:	ReadDibBitmapInfo( int fh )
 *
 *	目的:	DIB形式のファイルを読み取り、そのBITMAPINFOの
 *	グローバルハンドルを返します。
 *	この関数は以前の(BITMAPCOREHEADER)形式と新しい(BITMAPINFOHEADER)形式の
 * 	両方を処理できますが、返すのはつねに新しいBITMAPINFOです。
 *
 *	 戻り値: ファイル内のDIBのBITMAPINFOを識別するハンドル
 *
 ************************************************************************/
HANDLE CEditView::ReadDibBitmapInfo ( int fh )
{
	DWORD				off;
	HANDLE				hbi = NULL;
	int					size;
	int					i;
	WORD				nNumColors;

	RGBQUAD FAR			*pRgb;
	BITMAPINFOHEADER	bi;
	BITMAPCOREHEADER	bc;
	LPBITMAPINFOHEADER	lpbi;
	BITMAPFILEHEADER	bf;
	DWORD				dwWidth = 0;
	DWORD				dwHeight = 0;
	WORD				wPlanes, wBitCount;

	if( fh == -1 )
		return NULL;

	/* ファイルポインタをリセットし、ファイルヘッダーを読み取る */
	off = _llseek( fh, 0L, SEEK_CUR );
	if( sizeof( bf ) != _lread( fh, (LPSTR)&bf, sizeof( bf ) ) )
		return FALSE;

	/* RCヘッダーがあるか調べる */
	if( !ISDIB( bf.bfType ) ){
		bf.bfOffBits = 0L;
		_llseek( fh, off, SEEK_SET );
	}
	if( sizeof( bi ) != _lread( fh, (LPSTR)&bi, sizeof( bi ) ) )
		return FALSE;

	nNumColors = DibNumColors( &bi );

	/*
	 * 情報ブロックの内容(BITMAPINFOまたはBITMAPCORE)をチェックし、
	 * それに従って情報を取得する。BITMAPCOREHEADERならば、情報を
	 * BITMAPINFOHEADER形式のブロックに転送する。
	 */
	switch ( size = (int)bi.biSize ){
	  case sizeof( BITMAPINFOHEADER ):
		break;
	  case  sizeof( BITMAPCOREHEADER ):

		bc = *(BITMAPCOREHEADER*)&bi;

		dwWidth		= (DWORD)bc.bcWidth;
		dwHeight	= (DWORD)bc.bcHeight;
		wPlanes		= bc.bcPlanes;
		wBitCount	= bc.bcBitCount;

		bi.biSize		= sizeof( BITMAPINFOHEADER );
		bi.biWidth		= dwWidth;
		bi.biHeight		= dwHeight;
		bi.biPlanes		= wPlanes;
		bi.biBitCount	= wBitCount;

		bi.biCompression	= BI_RGB;
		bi.biSizeImage	 	= 0;
		bi.biXPelsPerMeter	= 0;
		bi.biYPelsPerMeter	= 0;
		bi.biClrUsed		= nNumColors;
		bi.biClrImportant	= nNumColors;
		_llseek( fh, (LONG)sizeof( BITMAPCOREHEADER ) - sizeof( BITMAPINFOHEADER ), SEEK_CUR );
		break;
	  default:
		/* DIBではない */
		return NULL;
	}
	/* 0ならばデフォルト値を設定 */
	if( bi.biSizeImage == 0 ){
		bi.biSizeImage = WIDTHBYTES( (DWORD)bi.biWidth * bi.biBitCount ) * bi.biHeight;
	}
	if( bi.biClrUsed == 0 )
		bi.biClrUsed = DibNumColors(&bi);
	/* BITMAPINFO構造体とカラーテーブルを割り当てる */
	hbi = GlobalAlloc( GHND, (LONG)bi.biSize + nNumColors * sizeof( RGBQUAD ) );
	if( !hbi )
		return NULL;
	lpbi = (LPBITMAPINFOHEADER)GlobalLock( hbi );
	*lpbi = bi;
	/* カラーテーブルを指すポインタを取得 */
	pRgb = (RGBQUAD FAR *)( (LPSTR)lpbi + bi.biSize );
	if (nNumColors){
		if( size == sizeof( BITMAPCOREHEADER ) ){
			/*
			 * 古いカラーテーブル(3バイトのRGBTRIPLE)を新しいカラーテーブル(4バイトのRGBQUAD)に変換
			 */
			_lread( fh, (LPSTR)pRgb, nNumColors * sizeof( RGBTRIPLE ) );
			for( i = nNumColors - 1; i >= 0; i-- ){
				RGBQUAD rgb;
				rgb.rgbRed		= ((RGBTRIPLE FAR *)pRgb)[i].rgbtRed;
				rgb.rgbBlue		= ((RGBTRIPLE FAR *)pRgb)[i].rgbtBlue;
				rgb.rgbGreen	= ((RGBTRIPLE FAR *)pRgb)[i].rgbtGreen;
				rgb.rgbReserved = (BYTE)0;
				pRgb[i]			= rgb;
			}
		}
		else
			_lread( fh, (LPSTR)pRgb, nNumColors * sizeof( RGBQUAD ) );
	}
	if( bf.bfOffBits != 0L )
		_llseek( fh, off + bf.bfOffBits, SEEK_SET );
 	GlobalUnlock( hbi );
	return hbi;
}


/************************************************************************
 *
 *	関数:	DibInfo( HANDLE hbi, LPBITMAPINFOHEADER lpbi )
 *
 *	目的:	CF_DIB形式のメモリブロックに関連付けられているDIB情報を取得します。
 *
 *  戻り値:	TRUE	- 正常に終了した場合
 *			FALSE	- それ以外の場合
 *
 ************************************************************************/
BOOL CEditView::DibInfo( HANDLE hbi, LPBITMAPINFOHEADER lpbi )
{
	if( hbi ){
		*lpbi = *(LPBITMAPINFOHEADER)GlobalLock( hbi );

		/* デフォルトのメンバ設定 */
		if( lpbi->biSize != sizeof( BITMAPCOREHEADER ) ){
			if( lpbi->biSizeImage == 0L )
				lpbi->biSizeImage =
					WIDTHBYTES( lpbi->biWidth*lpbi->biBitCount ) * lpbi->biHeight;

			if( lpbi->biClrUsed == 0L )
				lpbi->biClrUsed = DibNumColors( lpbi );
		}
		GlobalUnlock( hbi );
		return TRUE;
	}
	return FALSE;
}

/********************************************************************************
 *
 *	関数:	PaletteSize( VOID FAR * pv )
 *
 *	目的:	パレットのバイト数を計算します。情報ブロックがBITMAPCOREHEADER型
 *			ならば、色数の3倍がパレットサイズになります。それ以外の場合は、
 *			色数の4倍がパレットサイズになります。
 *
 *	戻り値:	パレットのバイト数
 *
 *******************************************************************************/
WORD CEditView::PaletteSize ( VOID FAR * pv )
{
	LPBITMAPINFOHEADER lpbi;
	WORD		NumColors;
	lpbi		= (LPBITMAPINFOHEADER)pv;
	NumColors	= DibNumColors(lpbi);

	if( lpbi->biSize == sizeof( BITMAPCOREHEADER ) )
		return NumColors * sizeof( RGBTRIPLE );
	else
		return NumColors * sizeof( RGBQUAD );
}





/********************************************************************************
 *
 *	関数:	DibNumColors( VOID FAR * pv )
 *
 *	目的:	情報ブロックのBitCountメンバを参照して、DIBの色数を判断します。
 *
 *	戻り値:	DIBの色数
 *
 *******************************************************************************/
WORD CEditView::DibNumColors ( VOID FAR * pv)
{
	int					bits;
	LPBITMAPINFOHEADER lpbi;
	LPBITMAPCOREHEADER lpbc;
	lpbi = ( (LPBITMAPINFOHEADER)pv );
	lpbc = ( (LPBITMAPCOREHEADER)pv );
	/*
	 *	BITMAPINFO形式ヘッダーの場合、パレットのサイズはBITMAPCORE形式の
	 *	ヘッダーのbiClrUsedが示している。パレットのサイズは、ピクセル当たり
	 *	のビット数により異なる。
	 */
	if( lpbi->biSize != sizeof( BITMAPCOREHEADER ) ){
		if( lpbi->biClrUsed != 0 )
			return (WORD)lpbi->biClrUsed;
		bits = lpbi->biBitCount;
	}
	else
		bits = lpbc->bcBitCount;

	switch ( bits ){
		case 1:	return 2;
		case 4:	return 16;
		case 8:	return 256;
		/* 24ビットDIBにはカラーテーブルはない */
		default:	return 0;
	}
}

 /********** 64Kバイト以上の読み書きを行うプライベートルーチン *********/
/************************************************************************
 *
 *	関数:	lread( int fh, VOID FAR *pv, DWORD ul )
 *
 *	目的:	データをすべて読み取るまで32Kバイトずつデータを読み取ります。
 *
 *	戻り値:	0			- 正常に読み取れなかった場合
 *	読み取ったバイト数	- それ以外の場合
 *
 ************************************************************************/
DWORD CEditView::lread( int fh, void* pv, DWORD ul )
{
	DWORD	ulT = ul;
	BYTE	*hp = (BYTE *)pv;

#define	 MAXREAD_BYTES	32768	/* 読み取り処理時の読み取り可能なバイト数 */

	while( ul > (DWORD)MAXREAD_BYTES ){
		if( _lread( fh, (LPSTR)hp, (WORD)MAXREAD_BYTES ) != MAXREAD_BYTES )
			return 0;
		ul -= MAXREAD_BYTES;
		hp += MAXREAD_BYTES;
	}
	if( _lread(fh, (LPSTR)hp, (WORD)ul) != (WORD)ul )
		return 0;
	return ulT;
}


/************************************************************************
 * 関数:  PrintBitmap(int , int , int , int , LPCSTR );
 *
 *	目的:	指定したDIBファイルを読み込んで指定文字桁範囲に伸縮印刷します。
 *			印刷位置は現在のフォントサイズに影響されます。
 *
 *	戻り値:	なし
 *
 ************************************************************************/
void CEditView::PrintBitmap( HDC hdc, int x1, int y1, const char* szFile )
{
	HANDLE				hdib;
	BITMAPINFOHEADER	bi;
	LPBITMAPINFOHEADER	lpbi;
	LPSTR				pBuf;

	/* DIBファイルを開いてメモリDIBを作成 */
	hdib = OpenDIB ( szFile );
	if( hdib == NULL ){
//		MYTRACE( " OpenDIB()の実行に失敗  \n" );
		return;
	}
	DibInfo( hdib, &bi );

	/* DIBをプリンタDCに伸縮して転送 */
	lpbi = (LPBITMAPINFOHEADER)GlobalLock( hdib );
	if( !lpbi ){
		GlobalFree( (HGLOBAL)hdib );
		return ;
	}
	pBuf = (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize( lpbi );
	StretchDIBits ( hdc,
		x1,
		y1,
		bi.biWidth,
		bi.biHeight,
		0,
		0,
		bi.biWidth,
		bi.biHeight,
		pBuf,
		(LPBITMAPINFO)lpbi,
		DIB_RGB_COLORS,
		SRCCOPY
	);
	GlobalUnlock( hdib );
	GlobalFree( hdib );
	return;
}

#endif //////////////////////////////////////////////








/* 2点を対角とする矩形を求める */
void CEditView::TwoPointToRect(
		RECT*	prcRect,
	int		nLineFrom,
	int		nColmFrom,
	int		nLineTo,
	int		nColmTo
)
{
	if( nLineFrom < nLineTo ){
		prcRect->top	= nLineFrom;
		prcRect->bottom	= nLineTo;
	}else{
		prcRect->top	= nLineTo;
		prcRect->bottom	= nLineFrom;
	}
	if( nColmFrom < nColmTo ){
		prcRect->left	= nColmFrom;
		prcRect->right	= nColmTo;
	}else{
		prcRect->left	= nColmTo;
		prcRect->right	= nColmFrom;
	}
	return;

}

#if 0//////////////////////////////////////////
/* デバッグ用 リージョン矩形のダンプ */
void CEditView::TraceRgn( HRGN hrgn )
{
//	unsigned int	i;
	char*			pBuf;
	int				nRgnDataSize;
	RGNDATA*		pRgnData;
	RECT*			pRect;
	nRgnDataSize = ::GetRegionData( hrgn, 0, NULL );
	pBuf = new char[nRgnDataSize];
	pRgnData = (RGNDATA*)pBuf;
	nRgnDataSize = ::GetRegionData( hrgn, nRgnDataSize, pRgnData );
	pRect = (RECT*)&pRgnData->Buffer[0];
//	if( 0 < pRgnData->rdh.nCount ){
//		m_cShareData.TraceOut( "---------\n" );;
//		for( i = 0; i < pRgnData->rdh.nCount; ++i ){
//			m_cShareData.TraceOut( "\t(%d, %d, %d, %d \n", pRect[i].left, pRect[i].right, pRect[i].top, pRect[i].bottom );;
//		}
//	}
	delete [] pBuf;
	return;
}
#endif //#if 0




/*! 選択領域の描画

	@date 2006.10.01 Moca 重複コード削除．矩形作画改善．
	@date 2007.09.09 Moca 互換BMPによる画面バッファ
		画面バッファが有効時、画面と互換BMPの両方の反転処理を行う。
*/
void CEditView::DrawSelectArea( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}

	int			nFromLine;
	int			nFromCol;
	int			nToLine;
	int			nToCol;
	int			nLineNum;

	m_bDrawSelectArea = TRUE;	// 2002/12/13 ai

	// 2006.10.01 Moca 重複コード統合
	HDC         hdc = ::GetDC( m_hWnd );
	HBRUSH      hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
	HBRUSH      hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
	int         nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	HBRUSH		hBrushCompatOld;
	int			nROPCompatOld;
	if( m_hbmpCompatBMP ){
		hBrushCompatOld = (HBRUSH)::SelectObject( m_hdcCompatDC, hBrush );
		nROPCompatOld = ::SetROP2( m_hdcCompatDC, SELECTEDAREA_ROP2 );
	}
	// To Here 2007.09.09 Moca

//	MYTRACE( "DrawSelectArea()  m_bBeginBoxSelect=%s\n", m_bBeginBoxSelect?"TRUE":"FALSE" );
	if( m_bBeginBoxSelect ){		/* 矩形範囲選択中 */
		// 2001.12.21 hor 矩形エリアにEOFがある場合、RGN_XORで結合すると
		// EOF以降のエリアも反転してしまうので、この場合はRedrawを使う
		// 2002.02.16 hor ちらつきを抑止するためEOF以降のエリアが反転したらもう一度反転して元に戻すことにする
		//if((m_nViewTopLine+m_nViewRowNum+1>=m_pcEditDoc->m_cLayoutMgr.GetLineCount()) &&
		//   (m_nSelectLineTo+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ||
		//	m_nSelectLineToOld+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) ) {
		//	Redraw();
		//	return;
		//}

		const int nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
		const int nCharHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;

		RECT  rcOld;
		RECT  rcNew;
		HRGN  hrgnOld = NULL;
		HRGN  hrgnNew = NULL;
		HRGN  hrgnDraw = NULL;

		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcOld,
			m_nSelectLineFromOld,	/* 範囲選択開始行 */
			m_nSelectColmFromOld,	/* 範囲選択開始桁 */
			m_nSelectLineToOld,		/* 範囲選択終了行 */
			m_nSelectColmToOld		/* 範囲選択終了桁 */
		);
		if( rcOld.left	< m_nViewLeftCol ){
			rcOld.left = m_nViewLeftCol;
		}
		if( rcOld.right	< m_nViewLeftCol ){
			rcOld.right = m_nViewLeftCol;
		}
		if( rcOld.right > m_nViewLeftCol + m_nViewColNum + 1 ){
			rcOld.right = m_nViewLeftCol + m_nViewColNum + 1;
		}
		if( rcOld.top < m_nViewTopLine ){
			rcOld.top = m_nViewTopLine;
		}
		if( rcOld.bottom > m_nViewTopLine + m_nViewRowNum ){
			rcOld.bottom = m_nViewTopLine + m_nViewRowNum;
		}
		rcOld.left		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + rcOld.left  * nCharWidth;
		rcOld.right		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + rcOld.right * nCharWidth;
		rcOld.top		= ( rcOld.top - m_nViewTopLine ) * nCharHeight + m_nViewAlignTop;
		rcOld.bottom	= ( rcOld.bottom + 1 - m_nViewTopLine ) * nCharHeight + m_nViewAlignTop;
		hrgnOld = ::CreateRectRgnIndirect( &rcOld );

		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcNew,
			m_nSelectLineFrom,		/* 範囲選択開始行 */
			m_nSelectColmFrom,		/* 範囲選択開始桁 */
			m_nSelectLineTo,		/* 範囲選択終了行 */
			m_nSelectColmTo			/* 範囲選択終了桁 */
		);
		if( rcNew.left	< m_nViewLeftCol ){
			rcNew.left = m_nViewLeftCol;
		}
		if( rcNew.right	< m_nViewLeftCol ){
			rcNew.right = m_nViewLeftCol;
		}
		if( rcNew.right > m_nViewLeftCol + m_nViewColNum + 1 ){
			rcNew.right = m_nViewLeftCol + m_nViewColNum + 1;
		}
		if( rcNew.top < m_nViewTopLine ){
			rcNew.top = m_nViewTopLine;
		}
		if( rcNew.bottom > m_nViewTopLine + m_nViewRowNum ){
			rcNew.bottom = m_nViewTopLine + m_nViewRowNum;
		}
		rcNew.left		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + rcNew.left  * nCharWidth;
		rcNew.right		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + rcNew.right * nCharWidth;
		rcNew.top		= (rcNew.top - m_nViewTopLine) * nCharHeight + m_nViewAlignTop;
		rcNew.bottom	= (rcNew.bottom + 1 - m_nViewTopLine) * nCharHeight + m_nViewAlignTop;
		hrgnNew = ::CreateRectRgnIndirect( &rcNew );

		// 矩形作画。
		{
			/* ::CombineRgn()の結果を受け取るために、適当なリージョンを作る */
			hrgnDraw = ::CreateRectRgnIndirect( &rcNew );

			/* 旧選択矩形と新選択矩形のリージョンを結合し､ 重なりあう部分だけを除去します */
			if( NULLREGION != ::CombineRgn( hrgnDraw, hrgnOld, hrgnNew, RGN_XOR ) ){

				// 2002.02.16 hor
				// 結合後のエリアにEOFが含まれる場合はEOF以降の部分を除去します
				// 2006.10.01 Moca リーソースリークを修正したら、チラつくようになったため、
				// 抑えるために EOF以降をリージョンから削除して1度の作画にする

				// 2006.10.01 Moca Start EOF位置計算をGetEndLayoutPosに書き換え。
				int  nLastLen;
				int  nLastLine;
				m_pcEditDoc->m_cLayoutMgr.GetEndLayoutPos( nLastLen, nLastLine );
				// 2006.10.01 Moca End
				if(m_nSelectLineFrom>=nLastLine || m_nSelectLineTo>=nLastLine ||
					m_nSelectLineFromOld>=nLastLine || m_nSelectLineToOld>=nLastLine){
					//	Jan. 24, 2004 genta nLastLenは物理桁なので変換必要
					//	最終行にTABが入っていると反転範囲が不足する．
					//	2006.10.01 Moca GetEndLayoutPosで処理するためColumnToIndexは不要に。
					rcNew.left = m_nViewAlignLeft + (m_nViewLeftCol + nLastLen) * nCharWidth;
					rcNew.right = m_nViewAlignLeft + m_nViewCx;
					rcNew.top = (nLastLine - m_nViewTopLine) * nCharHeight + m_nViewAlignTop;
					rcNew.bottom = rcNew.top + nCharHeight;
					// 2006.10.01 Moca GDI(リージョン)リソースリーク修正
					HRGN hrgnEOFNew = ::CreateRectRgnIndirect( &rcNew );
					::CombineRgn( hrgnDraw, hrgnDraw, hrgnEOFNew, RGN_DIFF );
					::DeleteObject( hrgnEOFNew );
				}
				::PaintRgn( hdc, hrgnDraw );
				// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
				if( m_hbmpCompatBMP ){
					::PaintRgn( m_hdcCompatDC, hrgnDraw );
				}
				// To Here 2007.09.09 Moca
			}
		}

		//////////////////////////////////////////
		/* デバッグ用 リージョン矩形のダンプ */
//@@		TraceRgn( hrgnDraw );


		if( NULL != hrgnDraw ){
			::DeleteObject( hrgnDraw );
		}
		if( NULL != hrgnNew ){
			::DeleteObject( hrgnNew );
		}
		if( NULL != hrgnOld ){
			::DeleteObject( hrgnOld );
		}
	}else{

		/* 現在描画されている範囲と始点が同じ */
		if( m_nSelectLineFrom == m_nSelectLineFromOld &&
			m_nSelectColmFrom  == m_nSelectColmFromOld ){
			/* 範囲が後方に拡大された */
			if( m_nSelectLineTo > m_nSelectLineToOld ||
			   (m_nSelectLineTo == m_nSelectLineToOld &&
				m_nSelectColmTo > m_nSelectColmToOld ) ){
				nFromLine	= m_nSelectLineToOld;
				nFromCol	= m_nSelectColmToOld;
				nToLine		= m_nSelectLineTo;
				nToCol		= m_nSelectColmTo;
			}else{
				nFromLine	= m_nSelectLineTo;
				nFromCol	= m_nSelectColmTo;
				nToLine		= m_nSelectLineToOld;
				nToCol		= m_nSelectColmToOld;
			}
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}else
		if( m_nSelectLineTo == m_nSelectLineToOld &&
			m_nSelectColmTo  == m_nSelectColmToOld ){
			/* 範囲が前方に拡大された */
			if( m_nSelectLineFrom < m_nSelectLineFromOld ||
			   (m_nSelectLineFrom == m_nSelectLineFromOld &&
				m_nSelectColmFrom < m_nSelectColmFromOld ) ){
				nFromLine	= m_nSelectLineFrom;
				nFromCol	= m_nSelectColmFrom;
				nToLine		= m_nSelectLineFromOld;
				nToCol		= m_nSelectColmFromOld;
			}else{
				nFromLine	= m_nSelectLineFromOld;
				nFromCol	= m_nSelectColmFromOld;
				nToLine		= m_nSelectLineFrom;
				nToCol		= m_nSelectColmFrom;
			}
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}else{
			nFromLine		= m_nSelectLineFromOld;
			nFromCol		= m_nSelectColmFromOld;
			nToLine			= m_nSelectLineToOld;
			nToCol			= m_nSelectColmToOld;
			for( nLineNum	= nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
			nFromLine	= m_nSelectLineFrom;
			nFromCol	= m_nSelectColmFrom;
			nToLine		= m_nSelectLineTo;
			nToCol		= m_nSelectColmTo;
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}
	}
	
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if( m_hbmpCompatBMP ){
		::SetROP2( m_hdcCompatDC, nROPCompatOld );
		::SelectObject( m_hdcCompatDC, hBrushCompatOld );
	}
	// To Here 2007.09.09 Moca
	// 2006.10.01 Moca 重複コード統合
	::SetROP2( hdc, nROP_Old );
	::SelectObject( hdc, hBrushOld );
	::DeleteObject( hBrush );
	::ReleaseDC( m_hWnd, hdc );
	//	Jul. 9, 2005 genta 選択領域の情報を表示
	PrintSelectionInfoMsg();
	return;
}




/*! 選択領域の中の指定行の描画

	@param[in] hdc 描画領域のDevice Context Handle
	@param[in] nLineNum 描画対象行(レイアウト行)
	@param[in] nFromLine 選択開始行(レイアウト座標)
	@param[in] nFromCol  選択開始桁(レイアウト座標)
	@param[in] nToLine   選択終了行(レイアウト座標)
	@param[in] nToCol    選択終了桁(レイアウト座標)

	複数行に渡る選択範囲のうち，nLineNumで指定された1行分だけを描画する．
	選択範囲は固定されたままnLineNumのみが必要行分変化しながら呼びだされる．

	@date 2006.03.29 Moca 3000桁制限を撤廃．

*/
void CEditView::DrawSelectAreaLine(
		HDC hdc, int nLineNum, int nFromLine, int nFromCol, int nToLine, int nToCol
)
{
//	MYTRACE( "CEditView::DrawSelectAreaLine()\n" );
	RECT			rcClip;
	int				nSelectFrom;	// 描画行の選択開始桁位置
	int				nSelectTo;		// 描画行の選択開始終了位置

	if( nFromLine == nToLine ){
		nSelectFrom = nFromCol;
		nSelectTo	= nToCol;
	}else{
		// 2006.03.29 Moca 行末までの長さを求める位置を上からここに移動
		int nPosX = 0;
		const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nLineNum );
		CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
				nPosX ++;
				break;
			}
			// 2006.03.28 Moca 画面外まで求めたら打ち切る
			if( it.getColumn() > m_nViewLeftCol + m_nViewColNum ){
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		
		if( nLineNum == nFromLine ){
			nSelectFrom = nFromCol;
			nSelectTo	= nPosX;
		}else
		if( nLineNum == nToLine ){
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
			nSelectTo	= nToCol;
		}else{
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
			nSelectTo	= nPosX;
		}
		// 2006.05.24 Mocaフリーカーソル選択(選択開始/終了行)で
		// To < From になることがある。必ず From < To になるように入れ替える。
		if( nSelectTo < nSelectFrom ){
			int t = nSelectFrom;
			nSelectFrom = nSelectTo;
			nSelectTo = t;
		}
	}
	
	// 2006.03.28 Moca ウィンドウ幅が大きいと正しく反転しない問題を修正
	if( nSelectFrom < m_nViewLeftCol ){
		nSelectFrom = m_nViewLeftCol;
	}
	int		nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	int		nCharWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
	rcClip.left		= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + nSelectFrom * nCharWidth;
	rcClip.right	= (m_nViewAlignLeft - m_nViewLeftCol * nCharWidth) + nSelectTo   * nCharWidth;
	rcClip.top		= (nLineNum - m_nViewTopLine) * nLineHeight + m_nViewAlignTop;
	rcClip.bottom	= rcClip.top + nLineHeight;
	if( rcClip.right > m_nViewAlignLeft + m_nViewCx ){
		rcClip.right = m_nViewAlignLeft + m_nViewCx;
	}
	//	必要なときだけ。
	if ( rcClip.right != rcClip.left ){
		m_cUnderLine.CaretUnderLineOFF( TRUE );	//	YAZAKI
		
		// 2006.03.28 Moca 表示域内のみ処理する
		if( nSelectFrom <= m_nViewLeftCol + m_nViewColNum && m_nViewLeftCol < nSelectTo ){
			HRGN hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
			::PaintRgn( hdc, hrgnDraw );
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
			if( m_hbmpCompatBMP ){
				::PaintRgn( m_hdcCompatDC, hrgnDraw );
			}
			// To Here 2007.09.09 Moca
			::DeleteObject( hrgnDraw );
		}
	}

//	::Rectangle( hdc, rcClip.left, rcClip.top, rcClip.right + 1, rcClip.bottom + 1);
//	::FillRect( hdc, &rcClip, hBrushTextCol );

//	//	/* デバッグモニタに出力 */
//	m_cShareData.TraceOut( "DrawSelectAreaLine() rcClip.left=%d, rcClip.top=%d, rcClip.right=%d, rcClip.bottom=%d\n", rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );

	return;
}





/* テキストが選択されているか */
// 2002/03/29 Azumaiya
// インライン関数に変更(ヘッダに記述)。
/*BOOL CEditView::IsTextSelected( void )
{
	if(
		m_nSelectLineFrom	== -1 ||
		m_nSelectLineTo		== -1 ||
		m_nSelectColmFrom	== -1 ||
		m_nSelectColmTo		== -1
	)
	{
//	if( m_nSelectLineFrom == m_nSelectLineTo &&
//		m_nSelectColmFrom  == m_nSelectColmTo ){
		return FALSE;
	}
	return TRUE;

//	return ~(m_nSelectLineFrom|m_nSelectLineTo|m_nSelectColmFrom|m_nSelectColmTo) >> 31;
}*/


/* テキストの選択中か */
// 2002/03/29 Azumaiya
// インライン関数に変更(ヘッダに記述)。
/*BOOL CEditView::IsTextSelecting( void )
{
	if( m_bBeginSelect ||
		IsTextSelected()
	){
//		MYTRACE( "m_bBeginSelect=%d IsTextSelected()=%d TRUE==IsTextSelecting()\n", m_bBeginSelect, IsTextSelected() );
		return TRUE;
	}
//	MYTRACE( "m_bBeginSelect=%d IsTextSelected()=%d FALSE==IsTextSelecting()\n", m_bBeginSelect, IsTextSelected() );
	return FALSE;

//	return m_bSelectingLock|IsTextSelected();
}*/


/* フォントの変更 */
void CEditView::SetFont( void )
{
	HDC			hdc;
	HFONT		hFontOld;
	TEXTMETRIC	tm;
	int			i;
	SIZE		sz;

	hdc = ::GetDC( m_hWnd );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//	hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
	::GetTextMetrics( hdc, &tm );


// 1999.12.9
//	m_nCharWidth = tm.tmAveCharWidth - 1;
//	m_nCharHeight = tm.tmHeight + tm.tmExternalLeading;
	/* 文字の大きさを調べる */
// 2000.2.8
//	::GetTextExtentPoint32( hdc, "X", 1, &sz );
//	m_nCharHeight = sz.cy;
//	m_nCharWidth = sz.cx;
	::GetTextExtentPoint32( hdc, "大", 2, &sz );
	m_nCharHeight = sz.cy;
	m_nCharWidth = sz.cx / 2;


// 行の高さを2の倍数にする
//	if( ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) % 2 ){
//		++m_nCharHeight;
//	}

	m_nViewColNum = m_nViewCx / ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );	/* 表示域の桁数 */
	m_nViewRowNum = m_nViewCy / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );	/* 表示域の行数 */
	/* 行番号表示に必要な幅を設定 */
	DetectWidthOfLineNumberArea( FALSE );
	/* 文字列描画用文字幅配列 */
	for( i = 0; i < ( sizeof(m_pnDx) / sizeof(m_pnDx[0]) ); ++i ){
		m_pnDx[i] = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	}
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWnd, hdc );
	::InvalidateRect( m_hWnd, NULL, TRUE );
//	2002/05/12 YAZAKI 不要と思われたので。
//	if( m_nCaretWidth == 0 ){	/* キャレットがなかった場合 */
//	}else{
//		OnKillFocus();
//		OnSetFocus();
//	}
	//	Oct. 11, 2002 genta IMEのフォントも変更
	SetIMECompFormFont();
	return;
}



/* 行番号表示に必要な桁数を計算 */
int CEditView::DetectWidthOfLineNumberArea_calculate( void )
{
	int			i;
	int			nAllLines;
	int			nWork;
	if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		nAllLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	}else{
		nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	}
	if( 0 < nAllLines ){
		nWork = 100;
		for( i = 3; i < 12; ++i ){
			if( nWork > nAllLines ){	// Oct. 18, 2003 genta 式を整理
				break;
			}
			nWork *= 10;
		}
	}else{
		//	2003.09.11 wmlhq 行番号が1桁のときと幅を合わせる
		i = 3;
	}
	return i;

}


/*
行番号表示に必要な幅を設定幅が変更された場合はTRUEを返す
*/
BOOL CEditView::DetectWidthOfLineNumberArea( BOOL bRedraw )
{
	int				i;
	PAINTSTRUCT		ps;
	HDC				hdc;
//	int				nAllLines;
//	int				nWork;
	int				m_nViewAlignLeftNew;
	int				nCxVScroll;
	RECT			rc;

	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_GYOU].m_bDisp ){
		/* 行番号表示に必要な桁数を計算 */
		i = DetectWidthOfLineNumberArea_calculate();
		m_nViewAlignLeftNew = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * (i + 1);	/* 表示域の左端座標 */
		m_nViewAlignLeftCols = i + 1;
	}else{
		m_nViewAlignLeftNew = 8;
		m_nViewAlignLeftCols = 0;
	}
	//	Sep 18, 2002 genta
	m_nViewAlignLeftNew += m_pShareData->m_Common.m_nLineNumRightSpace;
	if( m_nViewAlignLeftNew != m_nViewAlignLeft ){
		m_nViewAlignLeft = m_nViewAlignLeftNew;
		::GetClientRect( m_hWnd, &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
		m_nViewCx = (rc.right - rc.left) - nCxVScroll - m_nViewAlignLeft;	/* 表示域の幅 */


		if( bRedraw ){
			/* 再描画 */
			hdc = ::GetDC( m_hWnd );
			ps.rcPaint.left = 0;
			ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
			ps.rcPaint.top = 0;
			ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//			OnKillFocus();
			m_cUnderLine.Lock();
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
			OnPaint( hdc, &ps, FALSE );
			// To Here 2007.09.09 Moca
			m_cUnderLine.UnLock();
//			OnSetFocus();
//			DispRuler( hdc );
			ShowEditCaret();
			::ReleaseDC( m_hWnd, hdc );
		}
		m_bRedrawRuler = true;
		return TRUE;
	}else{
		return FALSE;
	}
}





/* スクロールバーの状態を更新する */
void CEditView::AdjustScrollBars( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}


	int			nAllLines;
	int			nVScrollRate;
	SCROLLINFO	si;
//	int			nNowPos;

	if( NULL != m_hwndVScrollBar ){
		/* 垂直スクロールバー */
		/* nAllLines / nVScrollRate < 65535 となる整数nVScrollRateを求める */
		nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
		nAllLines+=2;
		nVScrollRate = 1;
		while( nAllLines / nVScrollRate > 65535 ){
			++nVScrollRate;
		}
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = nAllLines / nVScrollRate - 1;	/* 全行数 */
		si.nPage = m_nViewRowNum / nVScrollRate;	/* 表示域の行数 */
		si.nPos  = m_nViewTopLine / nVScrollRate;	/* 表示域の一番上の行(0開始) */
		si.nTrackPos = nVScrollRate;
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
		m_nVScrollRate = nVScrollRate;				/* 垂直スクロールバーの縮尺 */
		
		//	Nov. 16, 2002 genta
		//	縦スクロールバーがDisableになったときは必ず全体が画面内に収まるように
		//	スクロールさせる
		//	2005.11.01 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
		if( m_nViewRowNum >= nAllLines ){
			ScrollAtV( 0 );
		}
	}
	if( NULL != m_hwndHScrollBar ){
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

//@@		::GetScrollInfo( m_hwndHScrollBar, SB_CTL, &si );
//@@		if( si.nMax == m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() - 1
//@@		 && si.nPage == (UINT)m_nViewColNum
//@@		 && si.nPos  == m_nViewLeftCol
//@@	   /*&& si.nTrackPos == 1*/ ){
//@@		}else{
			/* 水平スクロールバー */
//			si.cbSize = sizeof( si );
//			si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
			si.nMin  = 0;
			//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
			si.nMax  = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() - 1;
			si.nPage = m_nViewColNum;		/* 表示域の桁数 */
			si.nPos  = m_nViewLeftCol;		/* 表示域の一番左の桁(0開始) */
			si.nTrackPos = 1;
			::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
		//	2006.1.28 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
		if( m_nViewColNum >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() ){
			ScrollAtH( 0 );
		}
//@@		}
	}

	return;
}

/*!	@brief 選択を考慮した行桁指定によるカーソル移動

	選択状態チェック→カーソル移動→選択領域更新という処理が
	あちこちのコマンドにあるのでまとめることにした．
	また，戻り値はほとんど使われていないのでvoidにした．

	選択状態を考慮してカーソルを移動する．
	非選択が指定された場合には既存選択範囲を解除して移動する．
	選択が指定された場合には選択範囲の開始・変更を併せて行う．
	インタラクティブ操作を前提とするため，必要に応じたスクロールを行う．
	カーソル移動後は上下移動でもカラム位置を保つよう，
	m_nCaretPosX_Prevの更新も併せて行う．

	@param nWk_CaretPosX	[in] 移動先桁位置(0～)
	@param nWk_CaretPosY	[in] 移動先行位置(0～)
	@param bSelect			[in] TRUE: 選択する/ FALSE: 選択解除
	@param nCaretMarginRate	[in] 縦スクロール開始位置を決める値


	@date 2006.07.09 genta 新規作成
*/
void CEditView::MoveCursorSelecting( int nWk_CaretPosX, int nWk_CaretPosY, BOOL bSelect, int nCaretMarginRate )
{
	if( bSelect ){
		if( !IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在のカーソル位置から選択を開始する */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( TRUE );
		}
	}
	MoveCursor( nWk_CaretPosX, nWk_CaretPosY, TRUE, nCaretMarginRate );	// 2007.08.22 ryoji nCaretMarginRateが使われていなかった
	m_nCaretPosX_Prev = m_nCaretPosX;
	if( bSelect ){
		/*	現在のカーソル位置によって選択範囲を変更．
		
			2004.04.02 Moca 
			キャレット位置が不正だった場合にMoveCursorの移動結果が
			引数で与えた座標とは異なることがあるため，
			nPosX, nPosYの代わりに実際の移動結果を使うように．
		*/
		ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
	}
	
}




/*!	@brief 行桁指定によるカーソル移動

	必要に応じて縦/横スクロールもする．
	垂直スクロールをした場合はその行数を返す（正／負）．
	
	@param nWk_CaretPosX	[in] 移動先桁位置(0～)
	@param nWk_CaretPosY	[in] 移動先行位置(0～)
	@param bScroll			[in] TRUE: 画面位置調整有り/ FALSE: 画面位置調整有り無し
	@param nCaretMarginRate	[in] 縦スクロール開始位置を決める値
	@return 縦スクロール行数(負:上スクロール/正:下スクロール)

	@note 不正な位置が指定された場合には適切な座標値に
		移動するため，引数で与えた座標と移動後の座標は
		必ずしも一致しない．
	
	@note bScrollがfalseの場合にはカーソル位置のみ移動する．
		trueの場合にはスクロール位置があわせて変更される

	@date 2001/10/20 deleted by novice AdjustScrollBar()を呼ぶ位置を変更
	@date 2004.04.02 Moca 行だけ有効な座標に修正するのを厳密に処理する
	@date 2004.09.11 genta bDrawスイッチは動作と名称が一致していないので
		再描画スイッチ→画面位置調整スイッチと名称変更
*/
int CEditView::MoveCursor( int nWk_CaretPosX, int nWk_CaretPosY, BOOL bScroll, int nCaretMarginRate )
{

	/* スクロール処理 */
	int		nScrollRowNum = 0;
	int		nScrollColNum = 0;
	RECT	rcScrol;
	RECT	rcClip;
	RECT	rcClip2;
//	int		nIndextY = 8;
	int		nCaretMarginY;
	HDC		hdc;
//	HPEN	hPen, hPenOld;
	int		nScrollMarginRight;
	int		nScrollMarginLeft;

	if( 0 >= m_nViewColNum ){
		return 0;
	}
	hdc = ::GetDC( m_hWnd );

	/* カーソル行アンダーラインのOFF */
//	if (IsTextSelected()) { //2002.02.27 Add By KK アンダーラインのちらつきを低減 - ここではテキスト選択時のみアンダーラインを消す。
		m_cUnderLine.CaretUnderLineOFF( bScroll );	//	YAZAKI
//	}	2002/04/04 YAZAKI 半ページスクロール時にアンダーラインが残ったままスクロールしてしまう問題に対処。

	if( m_bBeginSelect ){	/* 範囲選択中 */
		nCaretMarginY = 0;
	}else{
		//	2001/10/20 novice
		nCaretMarginY = m_nViewRowNum / nCaretMarginRate;
		if( 1 > nCaretMarginY ){
			nCaretMarginY = 1;
		}
	}
	// 2004.04.02 Moca 行だけ有効な座標に修正するのを厳密に処理する
	GetAdjustCursorPos( &nWk_CaretPosX, &nWk_CaretPosY );
	
	
	/* 水平スクロール量（文字数）の算出 */
	nScrollColNum = 0;
	nScrollMarginRight = 4;
	nScrollMarginLeft = 4;
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() > m_nViewColNum &&
		nWk_CaretPosX > m_nViewLeftCol + m_nViewColNum - nScrollMarginRight ){
		nScrollColNum =
			( m_nViewLeftCol + m_nViewColNum - nScrollMarginRight ) - nWk_CaretPosX;
	}else
	if( 0 < m_nViewLeftCol &&
		nWk_CaretPosX < m_nViewLeftCol + nScrollMarginLeft
	){
		nScrollColNum = m_nViewLeftCol + nScrollMarginLeft - nWk_CaretPosX;
		if( 0 > m_nViewLeftCol - nScrollColNum ){
			nScrollColNum = m_nViewLeftCol;
		}

	}

	m_nViewLeftCol -= nScrollColNum;

	//	From Here 2007.07.28 じゅうじ : 表示行数が3行以下の場合の動作改善
	/* 垂直スクロール量（行数）の算出 */
										// 画面が３行以下
	if( m_nViewRowNum <= 3 ){
							// 移動先は、画面のスクロールラインより上か？（up キー）
		if( nWk_CaretPosY - m_nViewTopLine < nCaretMarginY ){
			if( nWk_CaretPosY < nCaretMarginY ){	//１行目に移動
				nScrollRowNum = m_nViewTopLine;
			}else
			if( m_nViewRowNum <= 1 ){	// 画面が１行
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY;
			}else
#if !(0)	// COMMENTにすると、上下の空きを死守しない為、縦移動はgoodだが、横移動の場合上下にぶれる
			if( m_nViewRowNum <= 2 ){	// 画面が２行
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY;
			}else
#endif
			{						// 画面が３行
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY + 1;
			}
		}else
							// 移動先は、画面の最大行数－２より下か？（down キー）
		if( nWk_CaretPosY - m_nViewTopLine >= (m_nViewRowNum - nCaretMarginY - 2) ){
			int ii = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
			if( ii - nWk_CaretPosY < nCaretMarginY + 1 &&
				ii - m_nViewTopLine < m_nViewRowNum ) {
			} else
			if( m_nViewRowNum <= 2 ){	// 画面が２行、１行
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY;
			}else{						// 画面が３行
				nScrollRowNum = m_nViewTopLine - nWk_CaretPosY + 1;
			}
		}
	}else							// 移動先は、画面のスクロールラインより上か？（up キー）
	if( nWk_CaretPosY - m_nViewTopLine < nCaretMarginY ){
		if( nWk_CaretPosY < nCaretMarginY ){	//１行目に移動
			nScrollRowNum = m_nViewTopLine;
		}else{
			nScrollRowNum = -(nWk_CaretPosY - m_nViewTopLine) + nCaretMarginY;
		}
	} else
							// 移動先は、画面の最大行数－２より下か？（down キー）
	if( nWk_CaretPosY - m_nViewTopLine >= m_nViewRowNum - nCaretMarginY - 2 ){
		int ii = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
		if( ii - nWk_CaretPosY < nCaretMarginY + 1 &&
			ii - m_nViewTopLine < m_nViewRowNum ) {
		} else
		{
			nScrollRowNum =
				-(nWk_CaretPosY - m_nViewTopLine) + (m_nViewRowNum - nCaretMarginY - 2);
		}
	}
	//	To Here 2007.07.28 じゅうじ
	if( bScroll ){
		/* スクロール */
		if( abs( nScrollColNum ) >= m_nViewColNum ||
			abs( nScrollRowNum ) >= m_nViewRowNum ){
			m_nViewTopLine -= nScrollRowNum;
			::InvalidateRect( m_hWnd, NULL, TRUE );
		}else
		if( nScrollRowNum != 0 || nScrollColNum != 0 ){
			rcScrol.left = 0;
			rcScrol.right = m_nViewCx + m_nViewAlignLeft;
			rcScrol.top = m_nViewAlignTop;
			rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
			if( nScrollRowNum > 0 ){
				rcScrol.bottom =
					m_nViewCy + m_nViewAlignTop -
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				m_nViewTopLine -= nScrollRowNum;
				rcClip.left = 0;
				rcClip.right = m_nViewCx + m_nViewAlignLeft;
				rcClip.top = m_nViewAlignTop;
				rcClip.bottom =
					m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			}else
			if( nScrollRowNum < 0 ){
				rcScrol.top =
					m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				m_nViewTopLine -= nScrollRowNum;
				rcClip.left = 0;
				rcClip.right = m_nViewCx + m_nViewAlignLeft;
				rcClip.top =
					m_nViewCy + m_nViewAlignTop +
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				rcClip.bottom = m_nViewCy + m_nViewAlignTop;
			}
			if( nScrollColNum > 0 ){
				rcScrol.left = m_nViewAlignLeft;
				rcScrol.right =
					m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.left = m_nViewAlignLeft;
				rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.top = m_nViewAlignTop;
				rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
			}else
			if( nScrollColNum < 0 ){
				rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.left =
					m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.right = m_nViewCx + m_nViewAlignLeft;
				rcClip2.top = m_nViewAlignTop;
				rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
			}
			if( m_bDrawSWITCH ){
				::ScrollWindowEx(
					m_hWnd,
					nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),	/* 水平スクロール量 */
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),	/* 垂直スクロール量 */
					&rcScrol,	/* スクロール長方形の構造体のアドレス */
					NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
				);
				// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
				if( m_hbmpCompatBMP ){
					// 互換BMPもスクロール処理のためにBitBltで移動させる
					::BitBlt(
						m_hdcCompatDC,
						rcScrol.left + nScrollColNum * ( m_nCharWidth +  m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
						rcScrol.top  + nScrollRowNum * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ),
						rcScrol.right - rcScrol.left, rcScrol.bottom - rcScrol.top,
						m_hdcCompatDC, rcScrol.left, rcScrol.top, SRCCOPY
					);
				}
				// カーソルの縦線がテキストと行番号の隙間にあるとき、スクロール時に縦線領域を更新
				if( nScrollColNum != 0 && m_nOldCursorLineX == m_nViewAlignLeft - 1 ){
					RECT rcClip3;
					rcClip3.left = m_nOldCursorLineX;
					rcClip3.right = m_nOldCursorLineX + 1;
					rcClip3.top  = m_nViewAlignTop;
					rcClip3.bottom = m_nViewCy + m_nViewAlignTop;
					::InvalidateRect( m_hWnd, &rcClip3, TRUE );
				}
				// To Here 2007.09.09 Moca
				if( nScrollRowNum != 0 ){
					::InvalidateRect( m_hWnd, &rcClip, TRUE );
					if( nScrollColNum != 0 ){
						rcClip.left = 0;
						rcClip.right = m_nViewAlignLeft;
						rcClip.top = 0;
						rcClip.bottom = m_nViewCy + m_nViewAlignTop;
						::InvalidateRect( m_hWnd, &rcClip, TRUE );
					}
				}
				if( nScrollColNum != 0 ){
					::InvalidateRect( m_hWnd, &rcClip2, TRUE );
				}
			}
		}

		/* スクロールバーの状態を更新する */
		AdjustScrollBars(); // 2001/10/20 novice
	}

	/* キャレット移動 */
	m_nCaretPosX = nWk_CaretPosX;
	m_nCaretPosY = nWk_CaretPosY;

	/* カーソル位置変換
	||  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	||  →物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nCaretPosX,
		m_nCaretPosY,
		&m_nCaretPosX_PHY,	/* カーソル位置 改行単位行先頭からのバイト数(０開始) */
		&m_nCaretPosY_PHY	/* カーソル位置 改行単位行の行番号(０開始) */
	);
	// 横スクロールが発生したら、ルーラー全体を再描画 2002.02.25 Add By KK
	if (nScrollColNum != 0 ){
		//次回DispRuler呼び出し時に再描画。（bDraw=falseのケースを考慮した。）
		m_bRedrawRuler = true;
	}

	/* カーソル行アンダーラインのON */
	//CaretUnderLineON( bDraw ); //2002.02.27 Del By KK アンダーラインのちらつきを低減
	if( bScroll ){
		/* キャレットの表示・更新 */
		ShowEditCaret();

		/* ルーラの再描画 */
		DispRuler( hdc );

		/* アンダーラインの再描画 */
		m_cUnderLine.CaretUnderLineON(TRUE);

		/* キャレットの行桁位置を表示する */
		DrawCaretPosInfo();

		//	Sep. 11, 2004 genta 同期スクロールの関数化
		//	bScroll == FALSEの時にはスクロールしないので，実行しない
		SyncScrollV( -nScrollRowNum );	//	方向が逆なので符号反転が必要
		SyncScrollH( -nScrollColNum );	//	方向が逆なので符号反転が必要

	}
	::ReleaseDC( m_hWnd, hdc );


// 02/09/18 対括弧の強調表示 ai Start	03/02/18 ai mod S
	DrawBracketPair( false );
	SetBracketPairPos( true );
	DrawBracketPair( true );
// 02/09/18 対括弧の強調表示 ai End		03/02/18 ai mod E

	return nScrollRowNum;


}

/*! 正しいカーソル位置を算出する(EOF以降のみ)
	@param pnPosX [in/out] カーソルのレイアウト座標X
	@param pnPosY [in/out] カーソルのレイアウト座標Y
	@retval	TRUE 座標を修正した
	@retval	FALSE 座標は修正されなかった
	@note	EOFの直前が改行でない場合は、その行に限りEOF以降にも移動可能
			EOFだけの行は、先頭位置のみ正しい。
	@date 2004.04.02 Moca 関数化
*/
BOOL CEditView::GetAdjustCursorPos( int* pnPosX, int* pnPosY ){
	// 2004.03.28 Moca EOFのみのレイアウト行は、0桁目のみ有効.EOFより下の行のある場合は、EOF位置にする
	int nLayoutLineCount = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	int nPosX2 = *pnPosX;
	int nPosY2 = *pnPosY;
	BOOL ret = FALSE;
	if( nPosY2 >= nLayoutLineCount ){
		if( 0 < nLayoutLineCount ){
			nPosY2 = nLayoutLineCount - 1;
			const CLayout* pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( nPosY2 );
			if( pcLayout->m_cEol == EOL_NONE ){
				nPosX2 = LineIndexToColmn( pcLayout, pcLayout->GetLength() );
				// EOFだけ折り返されているか
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				if( nPosX2 >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() ){
					nPosY2++;
					nPosX2 = 0;
				}
			}else{
				// EOFだけの行
				nPosY2++;
				nPosX2 = 0;
			}
		}else{
			// 空のファイル
			nPosX2 = 0;
			nPosY2 = 0;
		}
		if( *pnPosX != nPosX2 || *pnPosY != nPosY2 ){
			*pnPosX = nPosX2;
			*pnPosY = nPosY2;
			ret = TRUE;
		}
	}
	return ret;
}


/* IME編集エリアの位置を変更 */
void CEditView::SetIMECompFormPos( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	COMPOSITIONFORM	CompForm;
	HIMC			hIMC = ::ImmGetContext( m_hWnd );
	POINT			point;
	HWND			hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	::GetCaretPos( &point );
	CompForm.dwStyle = CFS_POINT;
	CompForm.ptCurrentPos.x = (long) point.x;
	CompForm.ptCurrentPos.y = (long) point.y + m_nCaretHeight - m_nCharHeight;

	if ( hIMC ){
		::ImmSetCompositionWindow( hIMC, &CompForm );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
	return;
}





/* IME編集エリアの表示フォントを変更 */
void CEditView::SetIMECompFormFont( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	HIMC	hIMC = ::ImmGetContext( m_hWnd );
	if ( hIMC ){
		::ImmSetCompositionFont( hIMC, &(m_pShareData->m_Common.m_lf) );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
	return;
}





/** 行桁指定によるカーソル移動（座標調整付き）
	@param nNewX[in/out] カーソルのレイアウト座標X
	@param nNewY[in/out] カーソルのレイアウト座標Y
	@param bScroll[in] TRUE: 画面位置調整有り/ FALSE: 画面位置調整有り無し
	@param nCaretMarginRate[in] 縦スクロール開始位置を決める値
	@param dx[in] nNewXとマウスカーソル位置との誤差(カラム幅未満のドット数)
	@return 縦スクロール行数(負:上スクロール/正:下スクロール)

	@note マウス等による移動で不適切な位置に行かないよう座標調整してカーソル移動する

	@date 2007.08.23 ryoji 関数化（MoveCursorToPoint()から処理を抜き出し）
	@date 2007.09.26 ryoji 半角文字でも中央で左右にカーソルを振り分ける
*/
int CEditView::MoveCursorProperly( int nNewX, int nNewY, BOOL bScroll, int nCaretMarginRate, int dx )
{
	const char*		pLine;
	int				nLineLen;
	const CLayout*	pcLayout;

	if( 0 > nNewY ){
		nNewY = 0;
	}
	/* カーソルがテキスト最下端行にあるか */
	if( nNewY >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
		// 2004.04.03 Moca EOFより後ろの座標調整は、MoveCursor内でやってもらうので、削除
	}else
	/* カーソルがテキスト最上端行にあるか */
	if( nNewY < 0 ){
		nNewX = 0;
		nNewY = 0;
	}else{
		/* 移動先の行のデータを取得 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nNewY, &nLineLen, &pcLayout );

		int nColWidth = m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace;
		int nPosX = 0;
		int i = 0;
		CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
				i = nLineLen;
				break;
			}
			if( it.getColumn() + it.getColumnDelta() > nNewX ){
				if (nNewX >= (pcLayout ? pcLayout->GetIndent() : 0) && ((nNewX - it.getColumn()) * nColWidth + dx) * 2 >= it.getColumnDelta() * nColWidth){
					nPosX += it.getColumnDelta();
				}
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		if ( it.end() ){
			i = it.getIndex();
			nPosX -= it.getColumnDelta();
		}

		if( i >= nLineLen ){
// From 2001.12.21 hor フリーカーソルOFFでEOFのある行の直前がマウスで選択できないバグの修正
			if( nNewY +1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
				EOL_NONE == pcLayout->m_cEol.GetLen() ){
				nPosX = LineIndexToColmn( pcLayout, nLineLen );
			}else
// To 2001.12.21 hor
			/* フリーカーソルモードか */
			if( m_pShareData->m_Common.m_bIsFreeCursorMode
			  || ( m_bBeginSelect && m_bBeginBoxSelect )	/* マウス範囲選択中 && 矩形範囲選択中 */
//			  || m_bDragMode /* OLE DropTarget */
			  || ( m_bDragMode && m_bBeginBoxSelect ) /* OLE DropTarget && 矩形範囲選択中 */
			){
// From 2001.12.21 hor
//				if( nNewY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
//					pLine[ nLineLen - 1 ] != '\n' && pLine[ nLineLen - 1 ] != '\r'
//				){
//					nPosX = LineIndexToColmn( pLine, nLineLen, nLineLen );
//				}else{
// To 2001.12.21 hor
					nPosX = nNewX;
					//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
					if( nPosX < 0 ){
						nPosX = 0;
					}else
					if( nPosX > m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() ){	/* 折り返し文字数 */
						nPosX = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
					}
//				}
			}
		}
		nNewX = nPosX;
	}

	return MoveCursor( nNewX, nNewY, bScroll, nCaretMarginRate );
}



/* マウス等による座標指定によるカーソル移動
|| 必要に応じて縦/横スクロールもする
|| 垂直スクロールをした場合はその行数を返す(正／負)
*/
int CEditView::MoveCursorToPoint( int xPos, int yPos )
{
	int				nScrollRowNum;
	int				nNewX;
	int				nNewY;
	int				dx;

	nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	dx = (xPos - m_nViewAlignLeft) % ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );

	nScrollRowNum = MoveCursorProperly( nNewX, nNewY, TRUE, 1000, dx );
	m_nCaretPosX_Prev = m_nCaretPosX;
	return nScrollRowNum;
}
//_CARETMARGINRATE_CARETMARGINRATE_CARETMARGINRATE



/* マウス左ボタン押下 */
void CEditView::OnLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{

	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

	//isearch 2004.10.22 isearchをキャンセルする
	if (m_nISearchMode > 0 ){
		ISearchExit();
	}

//	DWORD	nKeyBoardSpeed;
	int			nCaretPosY_Old;
	CMemory		cmemCurText;
	const char*	pLine;
	int			nLineLen;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	int			nIdx;
	int			nWork;
	BOOL		tripleClickMode = FALSE;	// 2007.10.02 nasukoji	トリプルクリックであることを示す
	int			nFuncID = 0;				// 2007.11.30 nasukoji	マウス左クリックに対応する機能コード

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() == 0 ){
		return;
	}
	if( m_nCaretWidth == 0 ){ //フォーカスがないとき
		return;
	}
	nCaretPosY_Old = m_nCaretPosY;

	/* 辞書Tipが起動されている */
	if( 0 == m_dwTipTimer ){
		/* 辞書Tipを消す */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
	}else{
		m_dwTipTimer = ::GetTickCount();		/* 辞書Tip起動タイマー */
	}

	// 2007.11.30 nasukoji	トリプルクリックをチェック
	tripleClickMode = CheckTripleClick(xPos, yPos);

	if(tripleClickMode){
		// マウス左トリプルクリックに対応する機能コードはm_Common.m_pKeyNameArr[5]に入っている
		nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_TRIPLECLICK].m_nFuncCodeArr[getCtrlKeyState()];
		if( 0 == nFuncID ){
			tripleClickMode = 0;	// 割り当て機能無しの時はトリプルクリック OFF
		}
	}else{
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}

	/* 現在のマウスカーソル位置→レイアウト位置 */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
//	MYTRACE( "OnLBUTTONDOWN() nNewX=%d nNewY=%d\n", nNewX, nNewY );

	// OLEによるドラッグ & ドロップを使う
	// 2007.11.30 nasukoji	トリプルクリック時はドラッグを開始しない
	if( !tripleClickMode && TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){
		if( m_pShareData->m_Common.m_bUseOLE_DropSource ){		/* OLEによるドラッグ元にするか */
			/* 行選択エリアをドラッグした */
			if( xPos < m_nViewAlignLeft - ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) ){
				goto normal_action;
			}
			/* 指定カーソル位置が選択エリア内にあるか */
			if( 0 == IsCurrentPositionSelected(
				nNewX,		// カーソル位置X
				nNewY		// カーソル位置Y
				)
			){
				/* 選択範囲のデータを取得 */
				if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
					DWORD dwEffects;
					m_bDragSource = TRUE;
					CDataObject data( cmemCurText.GetPtr() );
					dwEffects = data.DragDrop( TRUE, DROPEFFECT_COPY | DROPEFFECT_MOVE );
					m_bDragSource = FALSE;
//					MYTRACE( "dwEffects=%d\n", dwEffects );
					if( 0 == dwEffects ){
						if( IsTextSelected() ){	/* テキストが選択されているか */
							/* 現在の選択範囲を非選択状態に戻す */
							DisableSelectArea( TRUE );
							
//@@@ 2002.01.08 YAZAKI フリーカーソルOFFで複数行選択し、行の後ろをクリックするとそこにキャレットが置かれてしまうバグ修正
							/* カーソル移動。 */
							if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
								if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
									MoveCursorToPoint( xPos, yPos );
								}else
								if( xPos < m_nViewAlignLeft ){
									MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
								}
							}
						}
					}
				}
				return;
			}
		}
	}

normal_action:;

	// ALTキーが押されている、かつトリプルクリックでない		// 2007.10.10 nasukoji	トリプルクリック対応
	if(( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) )&&( ! tripleClickMode)){
		if( IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( TRUE );
		}
		if( yPos >= m_nViewAlignTop  && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
				MoveCursorToPoint( xPos, yPos );
			}else
			if( xPos < m_nViewAlignLeft ){
				MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
			}else{
				return;
			}
		}
		m_nMouseRollPosXOld = xPos;		/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;		/* マウス範囲選択前回位置(Y座標) */
		/* 範囲選択開始 & マウスキャプチャー */
		m_bBeginSelect = TRUE;			/* 範囲選択中 */
		m_bBeginBoxSelect = TRUE;		/* 矩形範囲選択中 */
		m_bBeginLineSelect = FALSE;		/* 行単位選択中 */
		m_bBeginWordSelect = FALSE;		/* 単語単位選択中 */

//		if( m_pShareData->m_Common.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
//			/* ALTキーが押されていたか */
//			if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
//				m_bBeginBoxSelect = TRUE;	/* 矩形範囲選択中 */
//			}
//		}
		::SetCapture( m_hWnd );
		HideCaret_( m_hWnd ); // 2002/07/22 novice
		/* 現在のカーソル位置から選択を開始する */
		BeginSelectArea( );
		m_cUnderLine.CaretUnderLineOFF( TRUE );
		m_cUnderLine.Lock();
		if( xPos < m_nViewAlignLeft ){
			/* カーソル下移動 */
			Command_DOWN( TRUE, FALSE );
		}
	}else{
		/* カーソル移動 */
		if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
			}else
			if( xPos < m_nViewAlignLeft ){
			}else{
				return;
			}
		}
		else
		if( yPos < m_nViewAlignTop ){
			//	ルーラクリック
			return;
		}
		else {
			return;
		}

		/* マウスのキャプチャなど */
		m_nMouseRollPosXOld = xPos;		/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;		/* マウス範囲選択前回位置(Y座標) */
		/* 範囲選択開始 & マウスキャプチャー */
		m_bBeginSelect = TRUE;			/* 範囲選択中 */
//		m_bBeginBoxSelect = FALSE;		/* 矩形範囲選択中でない */
		m_bBeginLineSelect = FALSE;		/* 行単位選択中 */
		m_bBeginWordSelect = FALSE;		/* 単語単位選択中 */
		::SetCapture( m_hWnd );
		HideCaret_( m_hWnd ); // 2002/07/22 novice


		if(tripleClickMode){		// 2007.10.10 nasukoji	トリプルクリックを処理する
			// 1行選択でない場合は選択文字列を解除
			// 2007.11.05 nasukoji	トリプルクリックが1行選択でなくてもクアドラプルクリックを有効とする
			if(F_SELECTLINE != nFuncID){
				OnLBUTTONUP( fwKeys, xPos, yPos );	// ここで左ボタンアップしたことにする

				if( IsTextSelected() )		// テキストが選択されているか
					DisableSelectArea( TRUE );		// 現在の選択範囲を非選択状態に戻す
			}

			// 2007.10.10 nasukoji	単語の途中で折り返されていると下の行が選択されてしまうことへの対処
			MoveCursorToPoint( xPos, yPos );	// カーソル移動

			// コマンドコードによる処理振り分け
			// マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
			::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ), (LPARAM)NULL );

			// 1行選択でない場合はここで抜ける（他の選択コマンドの時問題となるかも）
			if(F_SELECTLINE != nFuncID)
				return;

			// 2007.10.13 nasukoji	選択するものが無い（[EOF]のみの行）時は通常クリックと同じ処理
			if(( ! IsTextSelected() )&&( m_nCaretPosY_PHY >= m_pcEditDoc->m_cDocLineMgr.GetLineCount() )){
				BeginSelectArea();				// 現在のカーソル位置から選択を開始する
				m_bBeginLineSelect = FALSE;		// 行単位選択中 OFF
			}
		}else
		/* 選択開始処理 */
		/* SHIFTキーが押されていたか */
		if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
			if( IsTextSelected() ){			/* テキストが選択されているか */
				if( m_bBeginBoxSelect ){	/* 矩形範囲選択中 */
					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( TRUE );
					/* 現在のカーソル位置から選択を開始する */
					BeginSelectArea( );
				}else{
				}
			}else{
				/* 現在のカーソル位置から選択を開始する */
				BeginSelectArea( );
			}

			/* カーソル移動 */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
		}else{
			if( IsTextSelected() ){	/* テキストが選択されているか */
				/* 現在の選択範囲を非選択状態に戻す */
				DisableSelectArea( TRUE );
			}
			/* カーソル移動 */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
			/* 現在のカーソル位置から選択を開始する */
			BeginSelectArea( );
		}


		/******* この時点で必ず true == IsTextSelected() の状態になる ****:*/
		if( !IsTextSelected() ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
				"バグってる"
			);
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			m_nCaretPosX,	// カーソル位置X
			m_nCaretPosY	// カーソル位置Y
		);
//		MYTRACE( "◆◆◆nWorkRel = %d\n", nWorkRel );


		/* 現在のカーソル位置によって選択範囲を変更 */
		ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );


		// CTRLキーが押されている、かつトリプルクリックでない		// 2007.10.10 nasukoji	トリプルクリック対応
		if(( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) )&&( ! tripleClickMode)){
			m_bBeginWordSelect = TRUE;		/* 単語単位選択中 */
			if( !IsTextSelected() ){
				/* 現在位置の単語選択 */
				if ( Command_SELECTWORD() ){
					m_nSelectLineBgnFrom = m_nSelectLineFrom;	/* 範囲選択開始行(原点) */
					m_nSelectColmBgnFrom = m_nSelectColmFrom;	/* 範囲選択開始桁(原点) */
					m_nSelectLineBgnTo = m_nSelectLineTo;		/* 範囲選択開始行(原点) */
					m_nSelectColmBgnTo = m_nSelectColmTo;		/* 範囲選択開始桁(原点) */
				}
			}else{

				/* 選択領域描画 */
				DrawSelectArea();


				/* 指定された桁に対応する行のデータ内の位置を調べる */
				const CLayout* pcLayout;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineFrom, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_nSelectColmFrom );
					/* 現在位置の単語の範囲を調べる */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_nSelectLineFrom, nIdx, &nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
						nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
						nColmTo = LineIndexToColmn( pcLayout, nColmTo );


						nWork = IsCurrentPositionSelected(
							nColmFrom,	// カーソル位置X
							nLineFrom	// カーソル位置Y
						);
						if( -1 == nWork || 0 == nWork ){
							m_nSelectLineFrom = nLineFrom;
							m_nSelectColmFrom = nColmFrom;
							if( 1 == nWorkRel ){
								m_nSelectLineBgnFrom = nLineFrom;	/* 範囲選択開始行(原点) */
								m_nSelectColmBgnFrom = nColmFrom;	/* 範囲選択開始桁(原点) */
								m_nSelectLineBgnTo = nLineTo;		/* 範囲選択開始行(原点) */
								m_nSelectColmBgnTo = nColmTo;		/* 範囲選択開始桁(原点) */
							}
						}
	//					if( 1 == IsCurrentPositionSelected(
	//						nColmTo,	// カーソル位置X
	//						nLineTo		// カーソル位置Y
	//					) ){
	//						m_nSelectLineFrom = nLineTo;
	//						m_nSelectColmFrom = nColmTo;
	//					}
					}
				}
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineTo, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_nSelectColmTo );
					/* 現在位置の単語の範囲を調べる */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_nSelectLineTo, nIdx,
						&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
						nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
						nColmTo = LineIndexToColmn( pcLayout, nColmTo );

						nWork = IsCurrentPositionSelected(
							nColmFrom,	// カーソル位置X
							nLineFrom	// カーソル位置Y
						);
						if( -1 == nWork || 0 == nWork ){
							m_nSelectLineTo = nLineFrom;
							m_nSelectColmTo = nColmFrom;
						}
						if( 1 == IsCurrentPositionSelected(
							nColmTo,	// カーソル位置X
							nLineTo		// カーソル位置Y
						) ){
							m_nSelectLineTo = nLineTo;
							m_nSelectColmTo = nColmTo;
						}
						if( -1 == nWorkRel || 0 == nWorkRel ){
							m_nSelectLineBgnFrom = nLineFrom;	/* 範囲選択開始行(原点) */
							m_nSelectColmBgnFrom = nColmFrom;	/* 範囲選択開始桁(原点) */
							m_nSelectLineBgnTo = nLineTo;		/* 範囲選択開始行(原点) */
							m_nSelectColmBgnTo = nColmTo;		/* 範囲選択開始桁(原点) */
						}
					}
				}

				if( 0 < nWorkRel ){

				}
				/* 選択領域描画 */
				DrawSelectArea();
			}
		}

		// 行番号エリアをクリックした
		// 2007.12.08 nasukoji	シフトキーを押している場合は行頭クリックとして扱う
		if(( xPos < m_nViewAlignLeft )&&( !((SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ))){
			/* 現在のカーソル位置から選択を開始する */
//			BeginSelectArea( );
			m_bBeginLineSelect = TRUE;

			// 2002.10.07 YAZAKI 折り返し行をインデントしているときに選択がおかしいバグの対策
			Command_GOLINEEND( TRUE, FALSE );
			Command_RIGHT( TRUE, FALSE, FALSE );

			//	Apr. 14, 2003 genta
			//	行番号の下をクリックしてドラッグを開始するとおかしくなるのを修正
			//	行番号をクリックした場合にはChangeSelectAreaByCurrentCursor()にて
			//	m_nSelectColmTo/m_nSelectLineToに-1が設定されるが、上の
			//	Command_GOLINEEND(), Command_RIGHT()によって行選択が行われる。
			//	しかしキャレットが末尾にある場合にはキャレットが移動しないので
			//	m_nSelectColmTo/m_nSelectLineToが-1のまま残ってしまい、それが
			//	原点に設定されるためにおかしくなっていた。
			//	なので、範囲選択が行われていない場合は起点末尾の設定を行わないようにする
			if( IsTextSelected() ){
				m_nSelectLineBgnTo = m_nSelectLineTo;	/* 範囲選択開始行(原点) */
				m_nSelectColmBgnTo = m_nSelectColmTo;	/* 範囲選択開始桁(原点) */
			}
		}else{
//			/* 現在のカーソル位置から選択を開始する */
//			BeginSelectArea( );
//			m_bBeginLineSelect = FALSE;

			/* URLがクリックされたら選択するか */
			//	Sep. 7, 2003 genta URLの強調表示OFFの時はURLは普通の文字として扱う
			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_bDisp &&
				TRUE == m_pShareData->m_Common.m_bSelectClickedURL ){

				int			nUrlLine;	// URLの行(折り返し単位)
				int			nUrlIdxBgn;	// URLの位置(行頭からのバイト位置)
				int			nUrlLen;	// URLの長さ(バイト数)
				/* カーソル位置にURLが有る場合のその範囲を調べる */
				if( IsCurrentPositionURL(
					m_nCaretPosX,	// カーソル位置X
					m_nCaretPosY,	// カーソル位置Y
					&nUrlLine,		// URLの行(改行単位)
					&nUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
					&nUrlLen,		// URLの長さ(バイト数)
					NULL			// URL受け取り先
				) ){

					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( TRUE );

					/* 選択範囲の変更 */
#if 0
					2002/04/03 YAZAKI 不要な処理でした。
					m_nSelectLineBgnFrom = nUrlLine;			/* 範囲選択開始行(原点) */
					m_nSelectColmBgnFrom = nUrlIdxBgn;			/* 範囲選択開始桁(原点) */
					m_nSelectLineBgnTo = nUrlLine;				/* 範囲選択開始行(原点) */
					m_nSelectColmBgnTo = nUrlIdxBgn + nUrlLen;	/* 範囲選択開始桁(原点) */

					m_nSelectLineFrom =	nUrlLine;
					m_nSelectColmFrom = nUrlIdxBgn;
					m_nSelectLineTo = nUrlLine;
					m_nSelectColmTo = nUrlIdxBgn + nUrlLen;
#endif
					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						2002/04/08 YAZAKI 少しでもわかりやすく。
					*/
					int nColmFrom, nLineFrom, nColmTo, nLineTo;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nUrlIdxBgn          , nUrlLine, &nColmFrom, &nLineFrom );
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nUrlIdxBgn + nUrlLen, nUrlLine, &nColmTo, &nLineTo );

					m_nSelectLineBgnFrom = nLineFrom;		/* 範囲選択開始行(原点) */
					m_nSelectColmBgnFrom = nColmFrom;		/* 範囲選択開始桁(原点) */
					m_nSelectLineBgnTo = nLineTo;		/* 範囲選択開始行(原点) */
					m_nSelectColmBgnTo = nColmTo;		/* 範囲選択開始桁(原点) */

					m_nSelectLineFrom = nLineFrom;
					m_nSelectColmFrom = nColmFrom;
					m_nSelectLineTo = nLineTo;
					m_nSelectColmTo = nColmTo;

					/* 選択領域描画 */
					DrawSelectArea();
				}
			}
		}
	}
//	/* キーボードの現在のリピート間隔を取得 */
//	SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );
//	nKeyBoardSpeed *= 3;
//	/* タイマー起動 */
//	::SetTimer( m_hWnd, IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc );
	return;
}

/* 指定カーソル位置にURLが有る場合のその範囲を調べる */
/* 戻り値がTRUEの場合、*ppszURLは呼び出し側でdeleteすること */
BOOL CEditView::IsCurrentPositionURL(
		int		nCaretPosX,		// カーソル位置X
		int		nCaretPosY,		// カーソル位置Y
		int*	pnUrlLine,		// URLの行(改行単位)
		int*	pnUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
		int*	pnUrlLen,		// URLの長さ(バイト数)
		char**	ppszURL			// URL受け取り先(関数内でnewする)
)
{
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::IsCurrentPositionURL" );

	const char*	pLine;
//	const char*	pLineWork;
	int			nLineLen;
	int			nX;
	int			nY;
//	char*		pURL;
	int			i;
//	BOOL		bFindURL;
//	int			nCharChars;
	int			nUrlLen;

	/*
	  カーソル位置変換
	  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
	  →
	  物理位置(行頭からのバイト数、折り返し無し行位置)
	*/
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		nCaretPosX,
		nCaretPosY,
		(int*)&nX,
		(int*)&nY
	);
	*pnUrlLine = nY;
	pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nY, &nLineLen );

	i = nX - 200;
	if( i < 0 ){
		i = 0;
	}
	for( ; i <= nX && i < nLineLen && i < nX + 200; ){
	/* カーソル位置から前方に250バイトまでの範囲内で行頭に向かってサーチ */
		/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
		if( FALSE == IsURL( &pLine[i], nLineLen - i, &nUrlLen ) ){
			++i;
		}else{
			if( i <= nX && nX < i + nUrlLen ){
				/* URLを返す場合 */
				if( NULL != ppszURL ){
					*ppszURL = new char[nUrlLen + 1];
					memcpy( *ppszURL, &pLine[i], nUrlLen );
					(*ppszURL)[nUrlLen] = '\0';
					/* *ppszURLは呼び出し側でdeleteすること */
				}
				*pnUrlLen = nUrlLen;
				*pnUrlLine = nY;
				*pnUrlIdxBgn = i;
				return TRUE;
			}else{
				i += nUrlLen;
			}
		}
	}
	return FALSE;
}


/*!	トリプルクリックのチェック
	@brief トリプルクリックを判定する
	
	2回目のクリックから3回目のクリックまでの時間がダブルクリック時間以内で、
	かつその時のクリック位置のずれがシステムメトリック（X:SM_CXDOUBLECLK,
	Y:SM_CYDOUBLECLK）の値（ピクセル）以下の時トリプルクリックとする。
	
	@param[in] xPos		マウスクリックX座標
	@param[in] yPos		マウスクリックY座標
	@return		トリプルクリックの時はTRUEを返す
	トリプルクリックでない時はFALSEを返す

	@note	m_dwTripleClickCheckが0でない時にチェックモードと判定するが、PCを
			連続稼動している場合49.7日毎にカウンタが0になる為、わずかな可能性
			であるがトリプルクリックが判定できない時がある。
	
	@date 2007.10.02 nasukoji	新規作成
	@date 2007.10.11 nasukoji	改行文字より左側でないとトリプルクリックが認識されない不具合を修正
	@date 2007.10.13 nasukoji	行番号表示エリアのトリプルクリックは通常クリックとして扱う
*/
BOOL CEditView::CheckTripleClick( int xPos, int yPos )
{

	// トリプルクリックチェック有効でない（時刻がセットされていない）
	if(! m_dwTripleClickCheck)
		return FALSE;

	BOOL result = FALSE;

	// 前回クリックとのクリック位置のずれを算出
	int dxpos = m_nMouseRollPosXOld - xPos;
	int dypos = m_nMouseRollPosYOld - yPos;

	if(dxpos < 0)
		dxpos = -dxpos;		// 絶対値化

	if(dypos < 0)
		dypos = -dypos;		// 絶対値化

	// 行番号表示エリアでない、かつクリックプレスからダブルクリック時間以内、
	// かつダブルクリックの許容ずれピクセル以下のずれの時トリプルクリックとする
	//	2007.10.12 genta/dskoba システムのダブルクリック速度，ずれ許容量を取得
	if( (xPos >= m_nViewAlignLeft)&&
		(::GetTickCount() - m_dwTripleClickCheck <= GetDoubleClickTime() )&&
		(dxpos <= GetSystemMetrics(SM_CXDOUBLECLK) ) &&
		(dypos <= GetSystemMetrics(SM_CYDOUBLECLK)))
	{
		result = TRUE;
	}else{
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
	}
	
	return result;
}


/* マウス右ボタン押下 */
void CEditView::OnRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	/* 現在のマウスカーソル位置→レイアウト位置 */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	/* 指定カーソル位置が選択エリア内にあるか */
	if( 0 == IsCurrentPositionSelected(
		nNewX,		// カーソル位置X
		nNewY		// カーソル位置Y
		)
	){
		return;
	}
	OnLBUTTONDOWN( fwKeys, xPos , yPos );
	return;
}

/* マウス右ボタン開放 */
void CEditView::OnRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	if( m_bBeginSelect ){	/* 範囲選択中 */
		/* マウス左ボタン開放のメッセージ処理 */
		OnLBUTTONUP( fwKeys, xPos, yPos );
	}


	int		nIdx;
	int		nFuncID;
// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス右クリックに対応する機能コードはm_Common.m_pKeyNameArr[1]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_RIGHT].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
//	/* 右クリックメニュー */
//	Command_MENU_RBUTTON();
	return;
}


// novice 2004/10/11 マウス中ボタン対応
/*!
	マウス中ボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.11 novice 新規作成
*/
void CEditView::OnMBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス中ボタンに対応する機能コードはm_Common.m_pKeyNameArr[2]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_CENTER].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}
}


// novice 2004/10/10 マウスサイドボタン対応
/*!
	マウス左サイドボタンを押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
*/
void CEditView::OnXLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス左サイドボタンに対応する機能コードはm_Common.m_pKeyNameArr[3]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_LEFTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}


/*!
	マウス右サイドボタン押したときの処理

	@param fwKeys [in] first message parameter
	@param xPos [in] マウスカーソルX座標
	@param yPos [in] マウスカーソルY座標
	@date 2004.10.10 novice 新規作成
	@date 2004.10.11 novice マウス中ボタン対応のため変更
*/
void CEditView::OnXRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	int		nIdx;
	int		nFuncID;

	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス右サイドボタンに対応する機能コードはm_Common.m_pKeyNameArr[4]に入っている */
	nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_RIGHTSIDE].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	return;
}


VOID CEditView::OnTimer(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
	)
{
	POINT		po;
	RECT		rc;

	if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
		if( m_bDragSource ){
			return;
		}
	}
	/* 範囲選択中でない場合 */
	if(!m_bBeginSelect){
		if(TRUE == KeyWordHelpSearchDict( LID_SKH_ONTIMER, &po, &rc ) ){	// 2006.04.10 fon
			/* 辞書Tipを表示 */
			m_cTipWnd.Show( po.x, po.y + m_nCharHeight, NULL );
		}
	}else{
		::GetCursorPos( &po );
		::GetWindowRect(m_hWnd, &rc );
		if( !PtInRect( &rc, po ) ){
			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
			return;
		}

		// 1999.12.18 クライアント領域内ではタイマー自動ドラッグ+ロールしない
		return;

// 2001.12.21 hor 以下、実行されないのでコメントアウトします (行頭////はもともとコメント行です)
////		rc.top += m_nViewAlignTop;
//		RECT rc2;
//		rc2 = rc;
//		rc2.bottom = rc.top + m_nViewAlignTop + ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//		if( PtInRect( &rc2, po )
//		 && 0 < m_nViewTopLine
//		){
//			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
//			return;
//		}
//		rc2 = rc;
//		rc2.top = rc.bottom - ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//		if( PtInRect( &rc2, po )
//			&& m_pcEditDoc->m_cLayoutMgr.GetLineCount() > m_nViewTopLine + m_nViewRowNum
//		){
//			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
//			return;
//		}
//
////		rc.top += 48;
////		rc.bottom -= 48;
////		if( !PtInRect( &rc, po ) ){
////			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
////		}
	}
	return;
}

/*! キーワード辞書検索の前提条件チェックと、検索

	@date 2006.04.10 fon OnTimer, CreatePopUpMenu_Rから分離
*/
BOOL CEditView::KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc )
{
	CMemory		cmemCurText;
	char*		pszWork;
	int			nWorkLength;
	int			i;

	/* キーワードヘルプを使用するか？ */
	if( !m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp )	/* キーワードヘルプ機能を使用する */	// 2006.04.10 fon
		goto end_of_search;
	/* フォーカスがあるか？ */
	if( !(m_nCaretWidth > 0) ) 
		goto end_of_search;
	/* ウィンドウ内にマウスカーソルがあるか？ */
	GetCursorPos( po );
	GetWindowRect( m_hWnd, rc );
	if( !PtInRect( rc, *po ) )
		goto end_of_search;
	switch(nID){
	case LID_SKH_ONTIMER:
		/* 右コメントの１～３でない場合 */
		if(!( m_bInMenuLoop == FALSE	&&			/* １．メニュー モーダル ループに入っていない */
			0 != m_dwTipTimer			&&			/* ２．辞書Tipを表示していない */
			300 < ::GetTickCount() - m_dwTipTimer	/* ３．一定時間以上、マウスが固定されている */
		) )	goto end_of_search;
		break;
	case LID_SKH_POPUPMENU_R:
		if(!( m_bInMenuLoop == FALSE	//&&			/* １．メニュー モーダル ループに入っていない */
		//	0 != m_dwTipTimer			&&			/* ２．辞書Tipを表示していない */
		//	1000 < ::GetTickCount() - m_dwTipTimer	/* ３．一定時間以上、マウスが固定されている */
		) )	goto end_of_search;
		break;
	default:
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, _T("作者に教えて欲しいエラー"),
		_T("CEditView::KeyWordHelpSearchDict\nnID=%d") );
	}
	/* 選択範囲のデータを取得(複数行選択の場合は先頭の行のみ) */
	if( GetSelectedData( cmemCurText, TRUE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
		pszWork = cmemCurText.GetPtr();
		nWorkLength	= lstrlen( pszWork );
		for( i = 0; i < nWorkLength; ++i ){
			if( pszWork[i] == '\0' ||
				pszWork[i] == CR ||
				pszWork[i] == LF ){
				break;
			}
		}
		char*	pszBuf = new char[i + 1];
		memcpy( pszBuf, pszWork, i );
		pszBuf[i] = '\0';
		cmemCurText.SetData( pszBuf, i );
		delete [] pszBuf;
	}/* キャレット位置の単語を取得する処理 */	// 2006.03.24 fon
	else if(m_pShareData->m_Common.m_bUseCaretKeyWord){
		if(!GetCurrentWord(&cmemCurText))
			goto end_of_search;
	}else
		goto end_of_search;

	if( CMemory::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) &&	/* 既に検索済みか */
		(!m_cTipWnd.m_KeyWasHit) )								/* 該当するキーがなかった */
		goto end_of_search;
	m_cTipWnd.m_cKey = cmemCurText;

	/* 検索実行 */
	if( FALSE == KeySearchCore(&m_cTipWnd.m_cKey) )
		goto end_of_search;
	m_dwTipTimer = 0;		/* 辞書Tipを表示している */
	m_poTipCurPos = *po;	/* 現在のマウスカーソル位置 */
	return TRUE;			/* ここまで来ていればヒット・ワード */

	/* キーワードヘルプ表示処理終了 */
	end_of_search:
	return FALSE;
}

/*! キーワード辞書検索処理メイン

	@date 2006.04.10 fon KeyWordHelpSearchDictから分離
*/
BOOL CEditView::KeySearchCore( const CMemory* pcmemCurText )
{
	CMemory*	pcmemRefKey;
	CMemory*	pcmemRefText;
	LPSTR		pszWork;
	int			nCmpLen = STRNCMP_MAX; // 2006.04.10 fon
	int			nLine; // 2006.04.10 fon


	int nTypeNo = m_pcEditDoc->GetDocumentType();
	m_cTipWnd.m_cInfo.SetDataSz( "" );	/* tooltipバッファ初期化 */
	/* 1行目にキーワード表示の場合 */
	if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpKeyDisp){	/* キーワードも表示する */	// 2006.04.10 fon
		m_cTipWnd.m_cInfo.AppendSz( "[ " );
		m_cTipWnd.m_cInfo.AppendSz( pcmemCurText->GetPtr() );
		m_cTipWnd.m_cInfo.AppendSz( " ]" );
	}
	/* 途中まで一致を使う場合 */
	if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpPrefix)
		nCmpLen = lstrlen( pcmemCurText->GetPtr() );	// 2006.04.10 fon
	m_cTipWnd.m_KeyWasHit = FALSE;
	for(int i=0;i<m_pShareData->m_Types[nTypeNo].m_nKeyHelpNum;i++){	//最大数：MAX_KEYHELP_FILE
		if( 1 == m_pShareData->m_Types[nTypeNo].m_KeyHelpArr[i].m_nUse ){
			if(m_cDicMgr.Search( pcmemCurText->GetPtr(), nCmpLen, &pcmemRefKey, &pcmemRefText, m_pShareData->m_Types[nTypeNo].m_KeyHelpArr[i].m_szPath, &nLine )){	// 2006.04.10 fon (nCmpLen,pcmemRefKey,nSearchLine)引数を追加
				/* 該当するキーがある */
				pszWork = pcmemRefText->GetPtr();
				/* 有効になっている辞書を全部なめて、ヒットの都度説明の継ぎ増し */
				if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpAllSearch){	/* ヒットした次の辞書も検索 */	// 2006.04.10 fon
					/* バッファに前のデータが詰まっていたらseparator挿入 */
					if(m_cTipWnd.m_cInfo.GetLength() != 0)
						m_cTipWnd.m_cInfo.AppendSz( "\n--------------------\n■" );
					else
						m_cTipWnd.m_cInfo.AppendSz( "■" );	/* 先頭の場合 */
					/* 辞書のパス挿入 */
					m_cTipWnd.m_cInfo.AppendSz( m_pShareData->m_Types[nTypeNo].m_KeyHelpArr[i].m_szPath );
					m_cTipWnd.m_cInfo.AppendSz( "\n" );
					/* 前方一致でヒットした単語を挿入 */
					if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* 選択範囲で前方一致検索 */
						m_cTipWnd.m_cInfo.AppendSz( pcmemRefKey->GetPtr() );
						m_cTipWnd.m_cInfo.AppendSz( " >>\n" );
					}/* 調査した「意味」を挿入 */
					m_cTipWnd.m_cInfo.AppendSz( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* タグジャンプ用の情報を残す */
					if(FALSE == m_cTipWnd.m_KeyWasHit){
						m_cTipWnd.m_nSearchDict=i;	/* 辞書を開くとき最初にヒットした辞書を開く */
						m_cTipWnd.m_nSearchLine=nLine;
						m_cTipWnd.m_KeyWasHit = TRUE;
					}
				}else{	/* 最初のヒット項目のみ返す場合 */
					/* キーワードが入っていたらseparator挿入 */
					if(m_cTipWnd.m_cInfo.GetLength() != 0)
						m_cTipWnd.m_cInfo.AppendSz( "\n--------------------\n" );
					/* 前方一致でヒットした単語を挿入 */
					if(m_pcEditDoc->GetDocumentAttribute().m_bUseKeyHelpPrefix){	/* 選択範囲で前方一致検索 */
						m_cTipWnd.m_cInfo.AppendSz( pcmemRefKey->GetPtr() );
						m_cTipWnd.m_cInfo.AppendSz( " >>\n" );
					}/* 調査した「意味」を挿入 */
					m_cTipWnd.m_cInfo.AppendSz( pszWork );
					delete pcmemRefText;
					delete pcmemRefKey;	// 2006.07.02 genta
					/* タグジャンプ用の情報を残す */
					m_cTipWnd.m_nSearchDict=i;
					m_cTipWnd.m_nSearchLine=nLine;
					m_cTipWnd.m_KeyWasHit = TRUE;
					return TRUE;
				}
			}
		}
	}
	if(m_cTipWnd.m_KeyWasHit == TRUE){
			return TRUE;
	}
	/* 該当するキーがなかった場合 */
	return FALSE;
}


/* マウス移動のメッセージ処理 */
void CEditView::OnMOUSEMOVE( WPARAM fwKeys, int xPos , int yPos )
{
//#ifdef _DEBUG
//	gm_ProfileOutput = 1;
//	CRunningTimer cRunningTimer( (const char*)"CEditView::OnMOUSEMOVE" );
//#endif
	int			nScrollRowNum;
	POINT		po;
	const char*	pLine;
	int			nLineLen;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	int			nIdx;
	int			nWorkF;
	int			nWorkT;

	int			nSelectLineBgnFrom_Old;		/* 範囲選択開始行(原点) */
	int			nSelectColmBgnFrom_Old;		/* 範囲選択開始桁(原点) */
	int			nSelectLineBgnTo_Old;		/* 範囲選択開始行(原点) */
	int			nSelectColmBgnTo_Old;		/* 範囲選択開始桁(原点) */
	int			nSelectLineFrom_Old;
	int			nSelectColmFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColmTo_Old;
	int			nSelectLineFrom;
	int			nSelectColmFrom;
	int			nSelectLineTo;
	int			nSelectColmTo;

	nSelectLineBgnFrom_Old	= m_nSelectLineBgnFrom;		/* 範囲選択開始行(原点) */
	nSelectColmBgnFrom_Old	= m_nSelectColmBgnFrom;		/* 範囲選択開始桁(原点) */
	nSelectLineBgnTo_Old	= m_nSelectLineBgnTo;		/* 範囲選択開始行(原点) */
	nSelectColmBgnTo_Old	= m_nSelectColmBgnTo;		/* 範囲選択開始桁(原点) */
	nSelectLineFrom_Old		= m_nSelectLineFrom;
	nSelectColmFrom_Old		= m_nSelectColmFrom;
	nSelectLineTo_Old		= m_nSelectLineTo;
	nSelectColmTo_Old		= m_nSelectColmTo;

	if( !m_bBeginSelect ){	/* 範囲選択中 */
		::GetCursorPos( &po );
		//	2001/06/18 asa-o: 補完ウィンドウが表示されていない
		if(!m_bHokan){
			/* 辞書Tipが起動されている */
			if( 0 == m_dwTipTimer ){
				if( (m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y ) ){
					/* 辞書Tipを消す */
					m_cTipWnd.Hide();
					m_dwTipTimer = ::GetTickCount();	/* 辞書Tip起動タイマー */
				}
			}else{
				m_dwTipTimer = ::GetTickCount();		/* 辞書Tip起動タイマー */
			}
		}
		/* 現在のマウスカーソル位置→レイアウト位置 */
		int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
		int			nUrlLine;	// URLの行(折り返し単位)
		int			nUrlIdxBgn;	// URLの位置(行頭からのバイト位置)
		int			nUrlLen;	// URLの長さ(バイト数)


		/* 選択テキストのドラッグ中か */
		if( m_bDragMode ){
			if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLEによるドラッグ & ドロップを使う */
				/* 座標指定によるカーソル移動 */
				nScrollRowNum = MoveCursorToPoint( xPos , yPos );
			}
		}else{
			/* 行選択エリア? */
			if( xPos < m_nViewAlignLeft || yPos < m_nViewAlignTop ){	//	2002/2/10 aroka
				/* 矢印カーソル */
				if( yPos >= m_nViewAlignTop )
					::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_RVARROW ) ) );
				else
					::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}else

			if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop	/* OLEによるドラッグ & ドロップを使う */
			 && TRUE == m_pShareData->m_Common.m_bUseOLE_DropSource /* OLEによるドラッグ元にするか */
			 && 0 == IsCurrentPositionSelected(						/* 指定カーソル位置が選択エリア内にあるか */
				nNewX,	// カーソル位置X
				nNewY	// カーソル位置Y
				)
			){
				/* 矢印カーソル */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}else
			/* カーソル位置にURLが有る場合 */
			//	Sep. 7, 2003 genta URLの強調表示OFFの時はURLチェックも行わない
			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_bDisp &&
				IsCurrentPositionURL(
				nNewX,			// カーソル位置X
				nNewY,			// カーソル位置Y
				&nUrlLine,		// URLの行(改行単位)
				&nUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
				&nUrlLen,		// URLの長さ(バイト数)
				NULL/*&pszURL*/	// URL受け取り先
			) ){
				/* 手カーソル */
				::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
			}else{
				//migemo isearch 2004.10.22
				if( m_nISearchMode > 0 ){
					if (m_nISearchDirection == 1){
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_F)));
					}else{
						::SetCursor( ::LoadCursor( m_hInstance,MAKEINTRESOURCE(IDC_CURSOR_ISEARCH_B)));
					}
				}else
				/* アイビーム */
				::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
			}
		}
		return;
	}
	::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
	if( m_bBeginBoxSelect ){	/* 矩形範囲選択中 */
		/* 座標指定によるカーソル移動 */
		nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		/* 現在のカーソル位置によって選択範囲を変更 */
		ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		m_nMouseRollPosXOld = xPos;	/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;	/* マウス範囲選択前回位置(Y座標) */
	}else{
		/* 座標指定によるカーソル移動 */
		if(( xPos < m_nViewAlignLeft || m_dwTripleClickCheck )&& m_bBeginLineSelect ){		// 2007.10.02 nasukoji	行単位選択中
			// 2007.10.13 nasukoji	上方向の行選択時もマウスカーソルの位置の行が選択されるようにする
			int nNewY = yPos;
			int nLineHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;		// 1行の高さ

			// 選択開始行以下へのドラッグ時は1行下にカーソルを移動する
			if( m_nViewTopLine + (yPos - m_nViewAlignTop) / nLineHeight >= m_nSelectLineBgnTo )
				nNewY += nLineHeight;

			// カーソルを移動
			nScrollRowNum = MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) , nNewY );

			// 2007.10.13 nasukoji	2.5クリックによる行単位のドラッグ
			if( m_dwTripleClickCheck ){
				int nSelectStartLine = m_nSelectLineBgnTo;

				// 選択開始行以上にドラッグした
				if( m_nCaretPosY <= nSelectStartLine ){
					Command_GOLINETOP( TRUE, 0x09 );	// 改行単位の行頭へ移動
				}else{
					int nCaretPosX;
					int nCaretPosY;

					int nCaretPrevPosX_PHY;
					int nCaretPrevPosY_PHY = m_nCaretPosY_PHY;

					// 選択開始行より下にカーソルがある時は1行前と物理行番号の違いをチェックする
					// 選択開始行にカーソルがある時はチェック不要
					if(m_nCaretPosY > nSelectStartLine){
						// 1行前の物理行を取得する
						m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
							0, m_nCaretPosY - 1, &nCaretPrevPosX_PHY, &nCaretPrevPosY_PHY
						);
					}

					// 前の行と同じ物理行
					if( nCaretPrevPosY_PHY == m_nCaretPosY_PHY ){
						// 1行先の物理行からレイアウト行を求める
						m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
							0, m_nCaretPosY_PHY + 1, &nCaretPosX, &nCaretPosY
						);

						// カーソルを次の物理行頭へ移動する
						nScrollRowNum = MoveCursor( nCaretPosX, nCaretPosY, TRUE );
					}
				}
			}
		}else{
			nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		}
		m_nMouseRollPosXOld = xPos;	/* マウス範囲選択前回位置(X座標) */
		m_nMouseRollPosYOld = yPos;	/* マウス範囲選択前回位置(Y座標) */

		/* CTRLキーが押されていたか */
//		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		if( !m_bBeginWordSelect ){
			/* 現在のカーソル位置によって選択範囲を変更 */
			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		}else{
//			/* 現在のカーソル位置によって選択範囲を変更 */
//			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
			/* 現在のカーソル位置によって選択範囲を変更(テストのみ) */
			ChangeSelectAreaByCurrentCursorTEST(
				(int)m_nCaretPosX,
				(int)m_nCaretPosY,
				(int&)nSelectLineFrom,
				(int&)nSelectColmFrom,
				(int&)nSelectLineTo,
				(int&)nSelectColmTo
			);
			/* 選択範囲に変更なし */
			if( nSelectLineFrom_Old == nSelectLineFrom
			 && nSelectColmFrom_Old == nSelectColmFrom
			 && nSelectLineTo_Old == nSelectLineTo
			 && nSelectColmTo_Old == nSelectColmTo
			){
				ChangeSelectAreaByCurrentCursor(
					(int)m_nCaretPosX,
					(int)m_nCaretPosY
				);
				return;
			}
			const CLayout* pcLayout;
			if( NULL != ( pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout ) ) ){
				nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );
				/* 現在位置の単語の範囲を調べる */
				if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					m_nCaretPosY, nIdx,
					&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
				){
					/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
					nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
					nColmTo = LineIndexToColmn( pcLayout, nColmTo );

					nWorkF = IsCurrentPositionSelectedTEST(
						nColmFrom,	// カーソル位置X
						nLineFrom,	// カーソル位置Y
						(int)nSelectLineFrom,
						(int)nSelectColmFrom,
						(int)nSelectLineTo,
						(int)nSelectColmTo
					);
					nWorkT = IsCurrentPositionSelectedTEST(
						nColmTo,	// カーソル位置X
						nLineTo,	// カーソル位置Y
						(int)nSelectLineFrom,
						(int)nSelectColmFrom,
						(int)nSelectLineTo,
						(int)nSelectColmTo
					);
//					MYTRACE( "nWorkF=%d nWorkT=%d\n", nWorkF, nWorkT );
					if( -1 == nWorkF/* || 0 == nWorkF*/ ){
						/* 始点が前方に移動。現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
					}else
					if( /*0 == nWorkT ||*/ 1 == nWorkT ){
						/* 終点が後方に移動。現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
					}else
					if( nSelectLineFrom_Old == nSelectLineFrom
					 && nSelectColmFrom_Old == nSelectColmFrom
					){
						/* 始点が無変更＝前方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
					}else
					if( nSelectLineTo_Old == nSelectLineTo
					 && nSelectColmTo_Old == nSelectColmTo
					){
						/* 終点が無変更＝後方に縮小された */
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
					}
				}else{
					/* 現在のカーソル位置によって選択範囲を変更 */
					ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
				}
			}else{
				/* 現在のカーソル位置によって選択範囲を変更 */
				ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
			}
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer




/* マウスホイールのメッセージ処理 */
LRESULT CEditView::OnMOUSEWHEEL( WPARAM wParam, LPARAM lParam )
{
	WORD	fwKeys;
	short	zDelta;
	short	xPos;
	short	yPos;
	int		i;
	int		nScrollCode;
	int		nRollLineNum;

	fwKeys = LOWORD(wParam);			// key flags
	zDelta = (short) HIWORD(wParam);	// wheel rotation
	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
	yPos = (short) HIWORD(lParam);		// vertical position of pointer
//	MYTRACE( "CEditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n", fwKeys, zDelta, xPos, yPos );
	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}




	/* マウスホイールによるスクロール行数をレジストリから取得 */
	nRollLineNum = 6;
	/* レジストリの存在チェック */
	// 2006.06.03 Moca ReadRegistry に書き換え
	unsigned int uDataLen;	// size of value data
	char szValStr[256];
	uDataLen = sizeof(szValStr) - 1;
	if( ReadRegistry( HKEY_CURRENT_USER, "Control Panel\\desktop", "WheelScrollLines", szValStr, uDataLen ) ){
		nRollLineNum = ::atoi( szValStr );
	}
	if( -1 == nRollLineNum ){/* 「1画面分スクロールする」 */
		nRollLineNum = m_nViewRowNum;	// 表示域の行数
	}else{
		if( nRollLineNum < 1 ){
			nRollLineNum = 1;
		}
		if( nRollLineNum > 30 ){	//@@@ YAZAKI 2001.12.31 10→30へ。
			nRollLineNum = 30;
		}
	}
	for( i = 0; i < nRollLineNum; ++i ){
//		::PostMessage( m_hWnd, WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (WPARAM)m_hwndVScrollBar );
//		::SendMessage( m_hWnd, WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (WPARAM)m_hwndVScrollBar );

		//	Sep. 11, 2004 genta 同期スクロール行数
		int line;

		if( nScrollCode == SB_LINEUP ){
			line = ScrollAtV( m_nViewTopLine - 1 );
		}else{
			line = ScrollAtV( m_nViewTopLine + 1 );
		}
		SyncScrollV( line );
	}
	return 0;
}





/* 現在のカーソル位置から選択を開始する */
void CEditView::BeginSelectArea( void )
{
	m_nSelectLineBgnFrom = m_nCaretPosY;/* 範囲選択開始行(原点) */
	m_nSelectColmBgnFrom = m_nCaretPosX;/* 範囲選択開始桁(原点) */
	m_nSelectLineBgnTo = m_nCaretPosY;	/* 範囲選択開始行(原点) */
	m_nSelectColmBgnTo = m_nCaretPosX;	/* 範囲選択開始桁(原点) */

	m_nSelectLineFrom = m_nCaretPosY;	/* 範囲選択開始行 */
	m_nSelectColmFrom = m_nCaretPosX;	/* 範囲選択開始桁 */
	m_nSelectLineTo = m_nCaretPosY;		/* 範囲選択終了行 */
	m_nSelectColmTo = m_nCaretPosX;		/* 範囲選択終了桁 */
	return;
}





/* 現在の選択範囲を非選択状態に戻す */
void CEditView::DisableSelectArea( BOOL bDraw )
{
	m_nSelectLineFromOld = m_nSelectLineFrom;	/* 範囲選択開始行 */
	m_nSelectColmFromOld = m_nSelectColmFrom;	/* 範囲選択開始桁 */
	m_nSelectLineToOld = m_nSelectLineTo;		/* 範囲選択終了行 */
	m_nSelectColmToOld = m_nSelectColmTo;		/* 範囲選択終了桁 */
//	m_nSelectLineFrom = 0;
//	m_nSelectColmFrom = 0;
//	m_nSelectLineTo = 0;
//	m_nSelectColmTo = 0;

	m_nSelectLineFrom	= -1;
	m_nSelectColmFrom	= -1;
	m_nSelectLineTo		= -1;
	m_nSelectColmTo		= -1;

	if( bDraw ){
		DrawSelectArea();
		m_bDrawSelectArea = FALSE;	// 02/12/13 ai
	}
	m_bSelectingLock	 = FALSE;	/* 選択状態のロック */
	m_nSelectLineFromOld = 0;		/* 範囲選択開始行 */
	m_nSelectColmFromOld = 0; 		/* 範囲選択開始桁 */
	m_nSelectLineToOld = 0;			/* 範囲選択終了行 */
	m_nSelectColmToOld = 0;			/* 範囲選択終了桁 */
	m_bBeginBoxSelect = FALSE;		/* 矩形範囲選択中 */
	m_bBeginLineSelect = FALSE;		/* 行単位選択中 */
	m_bBeginWordSelect = FALSE;		/* 単語単位選択中 */

	// 2002.02.16 hor 直前のカーソル位置をリセット
	m_nCaretPosX_Prev=m_nCaretPosX;

	//	From Here Dec. 6, 2000 genta
	//	To Here Dec. 6, 2000 genta

	/* カーソル行アンダーラインのON */
	m_cUnderLine.CaretUnderLineON( bDraw );
	return;
}





/* 現在のカーソル位置によって選択範囲を変更 */
void CEditView::ChangeSelectAreaByCurrentCursor( int nCaretPosX, int nCaretPosY )
{
//	MYTRACE( "ChangeSelectAreaByCurrentCursor( %d, %d )\n", nCaretPosX, nCaretPosY );
//	int			nLineFrom;
//	int			nColmFrom;
//	int			nLineTo;
//	int			nColmTo;
//	const char*	pLine;
//	int			nLineLen;
//	int			nIdx;

	m_nSelectLineFromOld = m_nSelectLineFrom;	/* 範囲選択開始行 */
	m_nSelectColmFromOld = m_nSelectColmFrom; 	/* 範囲選択開始桁 */
	m_nSelectLineToOld = m_nSelectLineTo;		/* 範囲選択終了行 */
	m_nSelectColmToOld = m_nSelectColmTo;		/* 範囲選択終了桁 */

	//	2002/04/08 YAZAKI コードの重複を排除
	ChangeSelectAreaByCurrentCursorTEST(
		nCaretPosX,
		nCaretPosY, 
		m_nSelectLineFrom,
		m_nSelectColmFrom,
		m_nSelectLineTo,
		m_nSelectColmTo
	);
	/* 選択領域の描画 */
	DrawSelectArea();
	return;
}

/* 現在のカーソル位置によって選択範囲を変更(テストのみ) */
void CEditView::ChangeSelectAreaByCurrentCursorTEST(
	int		nCaretPosX,
	int		nCaretPosY,
	int&	nSelectLineFrom,
	int&	nSelectColmFrom,
	int&	nSelectLineTo,
	int&	nSelectColmTo
)
{
//	MYTRACE( "ChangeSelectAreaByCurrentCursor( %d, %d )\n", nCaretPosX, nCaretPosY );
//	int			nLineFrom;
//	int			nColmFrom;
//	int			nLineTo;
//	int			nColmTo;
//	const char*	pLine;
//	int			nLineLen;
//	int			nIdx;


	if( m_nSelectLineBgnFrom == m_nSelectLineBgnTo /* 範囲選択開始行(原点) */
	 && m_nSelectColmBgnFrom == m_nSelectColmBgnTo ){
		if( nCaretPosY == m_nSelectLineBgnFrom
		 && nCaretPosX == m_nSelectColmBgnFrom ){
			/* 選択解除 */
			nSelectLineFrom = -1;
			nSelectColmFrom  = -1;
			nSelectLineTo = -1;
			nSelectColmTo = -1;
		}else
		if( nCaretPosY < m_nSelectLineBgnFrom
		 || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX < m_nSelectColmBgnFrom ) ){
			nSelectLineFrom = nCaretPosY;
			nSelectColmFrom = nCaretPosX;
			nSelectLineTo = m_nSelectLineBgnFrom;
			nSelectColmTo = m_nSelectColmBgnFrom;
		}else{
			nSelectLineFrom = m_nSelectLineBgnFrom;
			nSelectColmFrom = m_nSelectColmBgnFrom;
			nSelectLineTo = nCaretPosY;
			nSelectColmTo = nCaretPosX;
		}
	}else{
		/* 常時選択範囲の範囲内 */
		if( ( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) )
		 && ( nCaretPosY < m_nSelectLineBgnTo || ( nCaretPosY == m_nSelectLineBgnTo && nCaretPosX < m_nSelectColmBgnTo ) )
		){
			nSelectLineFrom = m_nSelectLineBgnFrom;
			nSelectColmFrom = m_nSelectColmBgnFrom;
			if ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX == m_nSelectColmBgnFrom ){
				nSelectLineTo = m_nSelectLineBgnTo;	//	m_nSelectLineBgnTo;
				nSelectColmTo = m_nSelectColmBgnTo;	//	m_nSelectColmBgnTo;
			}
			else {
				nSelectLineTo = nCaretPosY;	//	m_nSelectLineBgnTo;
				nSelectColmTo = nCaretPosX;	//	m_nSelectColmBgnTo;
			}
		}else
		if( !( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) ) ){
			/* 常時選択範囲の前方向 */
			nSelectLineFrom = nCaretPosY;
			nSelectColmFrom  = nCaretPosX;
			nSelectLineTo = m_nSelectLineBgnTo;
			nSelectColmTo = m_nSelectColmBgnTo;
		}else{
			/* 常時選択範囲の後ろ方向 */
			nSelectLineFrom = m_nSelectLineBgnFrom;
			nSelectColmFrom = m_nSelectColmBgnFrom;
			nSelectLineTo = nCaretPosY;
			nSelectColmTo = nCaretPosX;
		}
	}
	return;

}


/* マウス左ボタン開放のメッセージ処理 */
void CEditView::OnLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE( "OnLBUTTONUP()\n" );
	CMemory		cmemBuf, cmemClip;

	/* 範囲選択終了 & マウスキャプチャーおわり */
	if( m_bBeginSelect ){	/* 範囲選択中 */
		/* マウス キャプチャを解放 */
		::ReleaseCapture();
		ShowCaret_( m_hWnd ); // 2002/07/22 novice

//		/* タイマー終了 */
//		::KillTimer( m_hWnd, IDT_ROLLMOUSE );
		m_bBeginSelect = FALSE;

//		if( !IsTextSelected() ){	/* テキストが選択されているか */
			if( m_nSelectLineFrom == m_nSelectLineTo &&
				m_nSelectColmFrom == m_nSelectColmTo
			){
				/* 現在の選択範囲を非選択状態に戻す */
				DisableSelectArea( TRUE );

				// 対括弧の強調表示	// 2007.10.18 ryoji
				DrawBracketPair( false );
				SetBracketPairPos( true );
				DrawBracketPair( true );
			}
//		}
		m_cUnderLine.UnLock();
	}
	return;
}





/* マウス左ボタンダブルクリック */
void CEditView::OnLBUTTONDBLCLK( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE( "OnLBUTTONDBLCLK()\n" );

	int			nIdx;
	int			nFuncID;
	int			nUrlLine;	// URLの行(折り返し単位)
	int			nUrlIdxBgn;	// URLの位置(行頭からのバイト位置)
	int			nUrlLen;	// URLの長さ(バイト数)
	char*		pszURL;
	const char*	pszMailTo = "mailto:";

	// 2007.10.06 nasukoji	クアドラプルクリック時はチェックしない
	if(! m_dwTripleClickCheck){
		/* カーソル位置にURLが有る場合のその範囲を調べる */
		//	Sep. 7, 2003 genta URLの強調表示OFFの時はURLチェックも行わない
		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_URL].m_bDisp &&
			IsCurrentPositionURL(
			m_nCaretPosX,	// カーソル位置X
			m_nCaretPosY,	// カーソル位置Y
			&nUrlLine,		// URLの行(改行単位)
			&nUrlIdxBgn,	// URLの位置(行頭からのバイト位置)
			&nUrlLen,		// URLの長さ(バイト数)
			&pszURL			// URL受け取り先
		) ){
			char*		pszWork = NULL;
			char*		pszOPEN;

			/* URLを開く */
			/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */
			if( TRUE == IsMailAddress( pszURL, lstrlen( pszURL ), NULL ) ){
				pszWork = new char[ lstrlen( pszURL ) + lstrlen( pszMailTo ) + 1];
				strcpy( pszWork, pszMailTo );
				strcat( pszWork, pszURL );
				pszOPEN = pszWork;
			}else{
				if( _tcsnicmp( pszURL, _T("ttp://"), 6 ) == 0 ){	//抑止URL
					pszWork = new TCHAR[ _tcslen( pszURL ) + 1 + 1 ];
					_tcscpy( pszWork, _T("h") );
					_tcscat( pszWork, pszURL );
					pszOPEN = pszWork;
				}else if( _tcsnicmp( pszURL, _T("tp://"), 5 ) == 0 ){	//抑止URL
					pszWork = new TCHAR[ _tcslen( pszURL ) + 2 + 1 ];
					_tcscpy( pszWork, _T("ht") );
					_tcscat( pszWork, pszURL );
					pszOPEN = pszWork;
				}else{
					pszOPEN = pszURL;
				}
			}
			::ShellExecute( NULL, "open", pszOPEN, NULL, NULL, SW_SHOW );
			delete [] pszURL;
			if( NULL != pszWork ){
				delete [] pszWork;
			}
			return;
		}

		/* GREP出力モードまたはデバッグモード かつ マウス左ボタンダブルクリックでタグジャンプ の場合 */
		//	2004.09.20 naoh 外部コマンドの出力からTagjumpできるように
		if( (m_pcEditDoc->m_bGrepMode || m_pcEditDoc->m_bDebugMode) && m_pShareData->m_Common.m_bGTJW_LDBLCLK ){
			/* タグジャンプ機能 */
			Command_TAGJUMP();
			return;
		}
	}

// novice 2004/10/10
	/* Shift,Ctrl,Altキーが押されていたか */
	nIdx = getCtrlKeyState();
	/* マウス左クリックに対応する機能コードはm_Common.m_pKeyNameArr[?]に入っている 2007.10.06 nasukoji */
	nFuncID = m_pShareData->m_pKeyNameArr[
		m_dwTripleClickCheck ? MOUSEFUNCTION_QUADCLICK : MOUSEFUNCTION_DOUBLECLICK
		].m_nFuncCodeArr[nIdx];
	if(m_dwTripleClickCheck){
		// 非選択状態にした後左クリックしたことにする
		// すべて選択の場合は、3.5クリック時の選択状態保持とドラッグ開始時の
		// 範囲変更のため。
		// クアドラプルクリック機能が割り当てられていない場合は、ダブルクリック
		// として処理するため。
		if( IsTextSelected() )		// テキストが選択されているか
			DisableSelectArea( TRUE );		// 現在の選択範囲を非選択状態に戻す

		if(! nFuncID){
			m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF
			nFuncID = m_pShareData->m_pKeyNameArr[MOUSEFUNCTION_DOUBLECLICK].m_nFuncCodeArr[nIdx];
			OnLBUTTONDOWN( fwKeys, xPos , yPos );	// カーソルをクリック位置へ移動する
		}
	}

	if( nFuncID != 0 ){
		/* コマンドコードによる処理振り分け */
		//	May 19, 2006 genta マウスからのメッセージはCMD_FROM_MOUSEを上位ビットに入れて送る
//		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
		::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, CMD_FROM_MOUSE ),  (LPARAM)NULL );
	}

	// 2007.10.06 nasukoji	クアドラプルクリック時もここで抜ける
	if(m_dwTripleClickCheck){
		m_dwTripleClickCheck = 0;	// トリプルクリックチェック OFF（次回は通常クリック）
		return;
	}

	// 2007.11.06 nasukoji	ダブルクリックが単語選択でなくてもトリプルクリックを有効とする
	// 2007.10.02 nasukoji	トリプルクリックチェック用に時刻を取得
	m_dwTripleClickCheck = ::GetTickCount();

	// ダブルクリック位置として記憶
	m_nMouseRollPosXOld = xPos;			/* マウス範囲選択前回位置(X座標) */
	m_nMouseRollPosYOld = yPos;			/* マウス範囲選択前回位置(Y座標) */

	/*	2007.07.09 maru 機能コードの判定を追加
		ダブルクリックからのドラッグでは単語単位の範囲選択(エディタの一般的動作)になるが
		この動作は、ダブルクリック＝単語選択を前提としたもの。
		キー割り当ての変更により、ダブルクリック≠単語選択のときには m_bBeginWordSelect = TRUE
		にすると、処理の内容によっては表示がおかしくなるので、ここで抜けるようにする。
	*/
	if(F_SELECTWORD != nFuncID) return;

	/* 範囲選択開始 & マウスキャプチャー */
	m_bBeginSelect = TRUE;				/* 範囲選択中 */
	m_bBeginBoxSelect = FALSE;			/* 矩形範囲選択中でない */
	m_bBeginLineSelect = FALSE;			/* 行単位選択中 */
	m_bBeginWordSelect = TRUE;			/* 単語単位選択中 */

	if( m_pShareData->m_Common.m_bFontIs_FIXED_PITCH ){	/* 現在のフォントは固定幅フォントである */
		/* ALTキーが押されていたか */
		if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
			m_bBeginBoxSelect = TRUE;	/* 矩形範囲選択中 */
		}
	}

	::SetCapture( m_hWnd );
	HideCaret_( m_hWnd ); // 2002/07/22 novice
	if( IsTextSelected() ){
		/* 常時選択範囲の範囲 */
		m_nSelectLineBgnTo = m_nSelectLineTo;
		m_nSelectColmBgnTo = m_nSelectColmTo;
	}else{
		/* 現在のカーソル位置から選択を開始する */
		BeginSelectArea( );
	}

	return;
}





/* カーソル上下移動処理 */
int CEditView::Cursor_UPDOWN( int nMoveLines, int bSelect )
{
	const char*		pLine;
	int				nLineLen;
	int				nPosX = 0;
	int				nPosY = m_nCaretPosY;
	int				i;
	int				nLineCols;
	int				nScrollLines;
	const CLayout*	pcLayout;
	nScrollLines = 0;
	if( nMoveLines > 0 ){
		/* カーソルがテキスト最下端行にあるか */
		if( m_nCaretPosY + nMoveLines >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
			nMoveLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - m_nCaretPosY  - 1;
		}
		if( nMoveLines <= 0 ){
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				nLineCols = LineIndexToColmn( pcLayout, nLineLen );
				/* 改行で終わっているか */
				//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
				if( ( EOL_NONE != pcLayout->m_cEol.GetLen() )
//				if( ( pLine[ nLineLen - 1 ] == '\n' || pLine[ nLineLen - 1 ] == '\r' )
				 || nLineCols >= m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize()
				){
					if( bSelect ){
						if( !IsTextSelected() ){	/* テキストが選択されているか */
							/* 現在のカーソル位置から選択を開始する */
							BeginSelectArea();
						}
					}else{
						if( IsTextSelected() ){	/* テキストが選択されているか */
							/* 現在の選択範囲を非選択状態に戻す */
							DisableSelectArea( TRUE );
						}
					}
					nPosX = 0;
					++nPosY;
					if( bSelect ){
						/* 現在のカーソル位置によって選択範囲を変更 */
						ChangeSelectAreaByCurrentCursor( nPosX, nPosY );
					}
					nScrollLines = MoveCursor( nPosX, nPosY, m_bDrawSWITCH /* TRUE */ ); // YAZAKI.
				}
			}
			//	Sep. 11, 2004 genta 同期スクロールの関数化
			//	MoveCursorでスクロール位置調整済み
			//SyncScrollV( nScrollLines );
			return nScrollLines;
		}
	}else{
		/* カーソルがテキスト最上端行にあるか */
		if( m_nCaretPosY + nMoveLines < 0 ){
			nMoveLines = - m_nCaretPosY;
		}
		if( nMoveLines >= 0 ){
			//	Sep. 11, 2004 genta 同期スクロールの関数化
			SyncScrollV( nScrollLines );
			return nScrollLines;
		}
	}
	if( bSelect ){
		if( !IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在のカーソル位置から選択を開始する */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){	/* テキストが選択されているか */
			/* 現在の選択範囲を非選択状態に戻す */
			DisableSelectArea( TRUE );
		}
	}
	/* 次の行のデータを取得 */
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY + nMoveLines, &nLineLen, &pcLayout );
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
			i = nLineLen;
			break;
		}
		if( it.getColumn() + it.getColumnDelta() > m_nCaretPosX_Prev ){
			i = it.getIndex();
			break;
		}
		it.addDelta();
	}
	nPosX += it.getColumn();
	if ( it.end() ){
		i = it.getIndex();
	}

	if( i >= nLineLen ){
		/* フリーカーソルモードか */
		if( m_pShareData->m_Common.m_bIsFreeCursorMode
		 || IsTextSelected() && m_bBeginBoxSelect	/* 矩形範囲選択中 */
		){
			if( m_nCaretPosY + nMoveLines + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount()  ){
				if( NULL != pLine ){
					if( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ){
						nPosX = m_nCaretPosX_Prev;
					}
				}
			}else{
				nPosX = m_nCaretPosX_Prev;
			}
		}
	}
	nScrollLines = MoveCursor( nPosX, m_nCaretPosY + nMoveLines, m_bDrawSWITCH /* TRUE */ ); // YAZAKI.
	if( bSelect ){
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelected() && -1 != m_nOldUnderLineY ){
//			HDC		hdc;
//			HPEN	hPen, hPenOld;
//			hdc = ::GetDC( m_hWnd );
//			/* カーソル行アンダーラインの消去 */
//			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//			hPenOld = (HPEN)::SelectObject( hdc, hPen );
//			::MoveToEx(
//				hdc,
//				m_nViewAlignLeft,
//				m_nOldUnderLineY,
//				NULL
//			);
//			::LineTo(
//				hdc,
//				m_nViewCx + m_nViewAlignLeft,
//				m_nOldUnderLineY
//			);
//			::SelectObject( hdc, hPenOld );
//			::DeleteObject( hPen );
//			m_nOldUnderLineY = -1;
//			::ReleaseDC( m_hWnd, hdc );
//		}
		/* 現在のカーソル位置によって選択範囲を変更 */
//		ChangeSelectAreaByCurrentCursor( nPosX, m_nCaretPosY + nMoveLines );
		ChangeSelectAreaByCurrentCursor( nPosX, m_nCaretPosY );
	}

	return nScrollLines;
}





/*! 指定上端行位置へスクロール

	@param nPos [in] スクロール位置
	@retval 実際にスクロールした行数 (正:下方向/負:上方向)

	@date 2004.09.11 genta 行数を戻り値として返すように．(同期スクロール用)
*/
int CEditView::ScrollAtV( int nPos )
{
	int			nScrollRowNum;
	RECT		rcScrol;
	RECT		rcClip;
	if( nPos < 0 ){
		nPos = 0;
	}else
	if( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 )- m_nViewRowNum < nPos ){
		nPos = ( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 ) - m_nViewRowNum;
		if( nPos < 0 ){
			nPos = 0;
		}
	}
	if( m_nViewTopLine == nPos ){
		return 0;	//	スクロール無し。
	}
	/* 垂直スクロール量（行数）の算出 */
	nScrollRowNum = m_nViewTopLine - nPos;

	/* スクロール */
	if( abs( nScrollRowNum ) >= m_nViewRowNum ){
		m_nViewTopLine = nPos;
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = m_nViewCx + m_nViewAlignLeft;
		rcScrol.top = m_nViewAlignTop;
		rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
		if( nScrollRowNum > 0 ){
			rcScrol.bottom =
				m_nViewCy + m_nViewAlignTop -
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			m_nViewTopLine = nPos;
			rcClip.left = 0;
			rcClip.right = m_nViewCx + m_nViewAlignLeft;
			rcClip.top = m_nViewAlignTop;
			rcClip.bottom =
				m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
		}else
		if( nScrollRowNum < 0 ){
			rcScrol.top =
				m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			m_nViewTopLine = nPos;
			rcClip.left = 0;
			rcClip.right = m_nViewCx + m_nViewAlignLeft;
			rcClip.top =
				m_nViewCy + m_nViewAlignTop +
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			rcClip.bottom = m_nViewCy + m_nViewAlignTop;
		}
		if( m_bDrawSWITCH ){
			::ScrollWindowEx(
				m_hWnd,
				0,	/* 水平スクロール量 */
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),		/* 垂直スクロール量 */
				&rcScrol,	/* スクロール長方形の構造体のアドレス */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
			// 互換BMPのスクロール
			if( m_hbmpCompatBMP ){
				::BitBlt(
					m_hdcCompatDC, rcScrol.left,
					rcScrol.top + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),
					rcScrol.right - rcScrol.left, rcScrol.bottom - rcScrol.top,
					m_hdcCompatDC, rcScrol.left, rcScrol.top, SRCCOPY
				);
			}
			// To Here 2007.09.09 Moca
			::InvalidateRect( m_hWnd, &rcClip, TRUE );
			::UpdateWindow( m_hWnd );
		}
	}

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* キャレットの表示・更新 */
	ShowEditCaret();

	return -nScrollRowNum;	//方向が逆なので符号反転が必要
}




/*! 指定左端桁位置へスクロール

	@param nPos [in] スクロール位置
	@retval 実際にスクロールした桁数 (正:右方向/負:左方向)

	@date 2004.09.11 genta 桁数を戻り値として返すように．(同期スクロール用)
*/
int CEditView::ScrollAtH( int nPos )
{
	int			nScrollColNum;
	RECT		rcScrol;
	RECT		rcClip2;
	if( nPos < 0 ){
		nPos = 0;
	}else
	//	Aug. 18, 2003 ryoji 変数のミスを修正
	//	ウィンドウの幅をきわめて狭くしたときに編集領域が行番号から離れてしまうことがあった．
	//	Aug. 14, 2005 genta 折り返し幅をLayoutMgrから取得するように
	if( m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() - m_nViewColNum  < nPos ){
		nPos = m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() - m_nViewColNum ;
		//	May 29, 2004 genta 折り返し幅よりウィンドウ幅が大きいときにWM_HSCROLLが来ると
		//	nPosが負の値になることがあり，その場合にスクロールバーから編集領域が
		//	離れてしまう．
		if( nPos < 0 )
			nPos = 0;
	}
	if( m_nViewLeftCol == nPos ){
		return 0;
	}
	/* 水平スクロール量（文字数）の算出 */
	nScrollColNum = m_nViewLeftCol - nPos;

	/* スクロール */
	if( abs( nScrollColNum ) >= m_nViewColNum /*|| abs( nScrollRowNum ) >= m_nViewRowNum*/ ){
		m_nViewLeftCol = nPos;
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = m_nViewCx + m_nViewAlignLeft;
		rcScrol.top = m_nViewAlignTop;
		rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
		if( nScrollColNum > 0 ){
			rcScrol.left = m_nViewAlignLeft;
			rcScrol.right =
				m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.left = m_nViewAlignLeft;
			rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.top = m_nViewAlignTop;
			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
		}else
		if( nScrollColNum < 0 ){
			rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.left =
				m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.right = m_nViewCx + m_nViewAlignLeft;
			rcClip2.top = m_nViewAlignTop;
			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
		}
		m_nViewLeftCol = nPos;
		if( m_bDrawSWITCH ){
			::ScrollWindowEx(
				m_hWnd,
				nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),		/* 水平スクロール量 */
				0,	/* 垂直スクロール量 */
				&rcScrol,	/* スクロール長方形の構造体のアドレス */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
			// 互換BMPのスクロール
			if( m_hbmpCompatBMP ){
				::BitBlt(
					m_hdcCompatDC, rcScrol.left + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
						rcScrol.top, rcScrol.right - rcScrol.left, rcScrol.bottom - rcScrol.top,
					m_hdcCompatDC, rcScrol.left, rcScrol.top , SRCCOPY
				);
			}
			// カーソルの縦線がテキストと行番号の隙間にあるとき、スクロール時に縦線領域を更新
			if( m_nOldCursorLineX == m_nViewAlignLeft - 1 ){
				RECT rcClip3;
				rcClip3.left = m_nOldCursorLineX;
				rcClip3.right = m_nOldCursorLineX + 1;
				rcClip3.top  = m_nViewAlignTop;
				rcClip3.bottom = m_nViewCy + m_nViewAlignTop;
				::InvalidateRect( m_hWnd, &rcClip3, TRUE );
			}
			// To Here 2007.09.09 Moca
			::InvalidateRect( m_hWnd, &rcClip2, TRUE );
			::UpdateWindow( m_hWnd );
		}
	}
	//	2006.1.28 aroka 判定条件誤り修正 (バーが消えてもスクロールしない)
	// 先にAdjustScrollBarsを呼んでしまうと、二度目はここまでこないので、
	// DispRulerが呼ばれない。そのため、順序を入れ替えた。
	m_bRedrawRuler = true; // ルーラーを再描画する。
	HDC hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	/* キャレットの表示・更新 */
	ShowEditCaret();

	return -nScrollColNum;	//方向が逆なので符号反転が必要
}

/*!	垂直同期スクロール

	垂直同期スクロールがONならば，対応するウィンドウを指定行数同期スクロールする
	
	@param line [in] スクロール行数 (正:下方向/負:上方向/0:何もしない)
	
	@author asa-o
	@date 2001.06.20 asa-o 新規作成
	@date 2004.09.11 genta 関数化

	@note 動作の詳細は設定や機能拡張により変更になる可能性がある

*/
void CEditView::SyncScrollV( int line )
{
	if( m_pShareData->m_Common.m_bSplitterWndVScroll && line != 0 )
	{
		CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01];
#if 0
		//	差分を保ったままスクロールする場合
		pcEditView -> ScrollByV( line );
#else
		pcEditView -> ScrollAtV( m_nViewTopLine );
#endif
	}
}

/*!	水平同期スクロール

	水平同期スクロールがONならば，対応するウィンドウを指定行数同期スクロールする．
	
	@param col [in] スクロール桁数 (正:右方向/負:左方向/0:何もしない)
	
	@author asa-o
	@date 2001.06.20 asa-o 新規作成
	@date 2004.09.11 genta 関数化

	@note 動作の詳細は設定や機能拡張により変更になる可能性がある
*/
void CEditView::SyncScrollH( int col )
{
	if( m_pShareData->m_Common.m_bSplitterWndHScroll && col != 0 )
	{
		CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x02];
		HDC			hdc = ::GetDC( pcEditView->m_hWnd );
		
#if 0
		//	差分を保ったままスクロールする場合
		pcEditView -> ScrollByH( col );
#else
		pcEditView -> ScrollAtH( m_nViewLeftCol );
#endif
		m_bRedrawRuler = true; //2002.02.25 Add By KK スクロール時ルーラー全体を描きなおす。
		DispRuler( hdc );
		::ReleaseDC( m_hWnd, hdc );
	}
}

/* 選択範囲のデータを取得 */
/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
BOOL CEditView::GetSelectedData(
		CMemory&	cmemBuf,
		BOOL		bLineOnly,
		const char*	pszQuote,			/* 先頭に付ける引用符 */
		BOOL		bWithLineNumber,	/* 行番号を付与する */
		BOOL		bAddCRLFWhenCopy,	/* 折り返し位置で改行記号を入れる */
//	Jul. 25, 2000 genta
		enumEOLType	neweol				//	コピー後の改行コード EOL_NONEはコード保存
)
{
	const char*		pLine;
	int				nLineLen;
	int				nLineNum;
	int				nIdxFrom;
	int				nIdxTo;
	RECT			rcSel;
	int				nRowNum;
	int				nLineNumCols;
	char*			pszLineNum;
	char*			pszSpaces = "                    ";
	const CLayout*	pcLayout;
	CEOL			appendEol( neweol );
	bool			addnl = false;

	/* 範囲選択がされていない */
	if( !IsTextSelected() ){
		return FALSE;
	}
	if( bWithLineNumber ){	/* 行番号を付与する */
		/* 行番号表示に必要な桁数を計算 */
		nLineNumCols = DetectWidthOfLineNumberArea_calculate();
		nLineNumCols += 1;
		pszLineNum = new char[nLineNumCols + 1];
	}


	if( m_bBeginBoxSelect ){	/* 矩形範囲選択中 */
		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,		/* 範囲選択開始行 */
			m_nSelectColmFrom,		/* 範囲選択開始桁 */
			m_nSelectLineTo,		/* 範囲選択終了行 */
			m_nSelectColmTo			/* 範囲選択終了桁 */
		);
//		cmemBuf.SetData( "", 0 );
		cmemBuf.SetDataSz( "" );

		//<< 2002/04/18 Azumaiya
		// サイズ分だけ要領をとっておく。
		// 結構大まかに見ています。
		int i = rcSel.bottom - rcSel.top;

		// 最初に行数分の改行量を計算してしまう。
		int nBufSize = strlen(CRLF) * i;

		// 実際の文字量。
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( rcSel.top, &nLineLen, &pcLayout );
		for(; i != 0 && pcLayout != NULL; i--, pcLayout = pcLayout->m_pNext)
		{
			pLine = pcLayout->m_pCDocLine->m_pLine->GetPtr() + pcLayout->m_nOffset;
			nLineLen = pcLayout->m_nLength;
			if( NULL != pLine )
			{
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );

				nBufSize += nIdxTo - nIdxFrom;
			}
			if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
				break;
			}
		}

		// 大まかに見た容量を元にサイズをあらかじめ確保しておく。
		cmemBuf.AllocBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		nRowNum = 0;
		for( nLineNum = rcSel.top; nLineNum <= rcSel.bottom; ++nLineNum ){
//			if( nRowNum > 0 ){
//				cmemBuf.AppendSz( CRLF );
//				if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
//					break;
//				}
//			}
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );
				//2002.02.08 hor
				// pLineがNULLのとき(矩形エリアの端がEOFのみの行を含むとき)は以下を処理しない
				if( nIdxTo - nIdxFrom > 0 ){
					if( pLine[nIdxTo - 1] == '\n' || pLine[nIdxTo - 1] == '\r' ){
						cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
					}else{
						cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
					}
				}
			}
			++nRowNum;
//			if( nRowNum > 0 ){
				cmemBuf.AppendSz( CRLF );
				if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
					break;
				}
//			}
		}
	}else{
		cmemBuf.SetDataSz( "" );

		//<< 2002/04/18 Azumaiya
		//  これから貼り付けに使う領域の大まかなサイズを取得する。
		//  大まかというレベルですので、サイズ計算の誤差が（容量を多く見積もる方に）結構出ると思いますが、
		// まぁ、速さ優先ということで勘弁してください。
		//  無駄な容量確保が出ていますので、もう少し精度を上げたいところですが・・・。
		//  とはいえ、逆に小さく見積もることになってしまうと、かなり速度をとられる要因になってしまうので
		// 困ってしまうところですが・・・。
		m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineFrom, &nLineLen, &pcLayout );
		int nBufSize = 0;
		int i = m_nSelectLineTo - m_nSelectLineFrom;
		// 先頭に引用符を付けるとき。
		if ( NULL != pszQuote )
		{
			nBufSize += strlen(pszQuote);
		}

		// 行番号を付ける。
		if ( bWithLineNumber )
		{
			nBufSize += nLineNumCols;
		}

		// 改行コードについて。
		if ( neweol == EOL_UNKNOWN )
		{
			nBufSize += strlen(CRLF);
		}
		else
		{
			nBufSize += appendEol.GetLen();
		}

		// すべての行について同様の操作をするので、行数倍する。
		nBufSize *= i;

		// 実際の各行の長さ。
		for (; i != 0 && pcLayout != NULL; i--, pcLayout = pcLayout->m_pNext )
		{
			nBufSize += pcLayout->m_nLength + appendEol.GetLen();
			if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
				break;
			}
		}

		// 調べた長さ分だけバッファを取っておく。
		cmemBuf.AllocBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		for( nLineNum = m_nSelectLineFrom; nLineNum <= m_nSelectLineTo; ++nLineNum ){
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				break;
			}
			if( nLineNum == m_nSelectLineFrom ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom = LineColmnToIndex( pcLayout, m_nSelectColmFrom );
			}else{
				nIdxFrom = 0;
			}
			if( nLineNum == m_nSelectLineTo ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxTo = LineColmnToIndex( pcLayout, m_nSelectColmTo );
			}else{
				nIdxTo = nLineLen;
			}
			if( nIdxTo - nIdxFrom == 0 ){
				continue;
			}

#if 0
			//	Jul. 25, 2000	genta
			//	改行処理変更のため削除
			/* 改行も処理するんかぃのぉ・・・？ */
			if( EOL_NONE != pcLayout->m_cEol && nIdxTo >= nLineLen ){
				nIdxTo = nLineLen - 1;
			}
#endif

			if( NULL != pszQuote && 0 < lstrlen( pszQuote ) ){	/* 先頭に付ける引用符 */
//				cmemBuf.Append( pszQuote, lstrlen( pszQuote ) );
				cmemBuf.AppendSz( pszQuote );
			}
			if( bWithLineNumber ){	/* 行番号を付与する */
				wsprintf( pszLineNum, " %d:" , nLineNum + 1 );
				cmemBuf.Append( pszSpaces, nLineNumCols - lstrlen( pszLineNum ) );
//				cmemBuf.Append( pszLineNum, lstrlen( pszLineNum ) );
				cmemBuf.AppendSz( pszLineNum );
			}


			if( EOL_NONE != pcLayout->m_cEol ){
//			if( pLine[nIdxTo - 1] == '\n' || pLine[nIdxTo - 1] == '\r' ){
//				cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
//				cmemBuf.AppendSz( CRLF );

				if( nIdxTo >= nLineLen ){
					cmemBuf.Append( &pLine[nIdxFrom], nLineLen - 1 - nIdxFrom );
					//	Jul. 25, 2000 genta
					cmemBuf.AppendSz( ( neweol == EOL_UNKNOWN ) ?
						(pcLayout->m_cEol).GetValue() :	//	コード保存
						appendEol.GetValue() );			//	新規改行コード
				}
				else {
					cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				}
			}else{
				cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				if( nIdxTo - nIdxFrom >= nLineLen ){
					if( bAddCRLFWhenCopy ||  /* 折り返し行に改行を付けてコピー */
						NULL != pszQuote || /* 先頭に付ける引用符 */
						bWithLineNumber 	/* 行番号を付与する */
					){
//						cmemBuf.Append( CRLF, lstrlen( CRLF ) );
						//	Jul. 25, 2000 genta
						cmemBuf.AppendSz(( neweol == EOL_UNKNOWN ) ?
							CRLF :						//	コード保存
							appendEol.GetValue() );		//	新規改行コード
					}
				}
			}
			if( bLineOnly ){	/* 複数行選択の場合は先頭の行のみ */
				break;
			}
		}
	}
	if( bWithLineNumber ){	/* 行番号を付与する */
		delete [] pszLineNum;
	}
	return TRUE;
}




/* 選択範囲内の全行をクリップボードにコピーする */
void CEditView::CopySelectedAllLines(
	const char*	pszQuote,		/* 先頭に付ける引用符 */
	BOOL		bWithLineNumber	/* 行番号を付与する */
)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	int			nSelectLineFromOld;	/* 範囲選択開始行 */
	int			nSelectColFromOld; 	/* 範囲選択開始桁 */
	int			nSelectLineToOld;	/* 範囲選択終了行 */
	int			nSelectColToOld;	/* 範囲選択終了桁 */
	RECT		rcSel;
	CMemory		cmemBuf;
//	HGLOBAL		hgClip;
//	char*		pszClip;
//	const char*	pLine;
//	int			nLineLen;
	if( !IsTextSelected() ){	/* テキストが選択されているか */
		return;
	}
	/* 矩形範囲選択中か */
	if( m_bBeginBoxSelect ){
		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,					/* 範囲選択開始行 */
			m_nSelectColmFrom,					/* 範囲選択開始桁 */
			m_nSelectLineTo,					/* 範囲選択終了行 */
			m_nSelectColmTo						/* 範囲選択終了桁 */
		);
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( TRUE );
//		/* 挿入データの先頭位置へカーソルを移動 */
//		MoveCursor( rcSel.left, rcSel.top, FALSE );
		m_nSelectLineFrom = rcSel.top;			/* 範囲選択開始行 */
		m_nSelectColmFrom = 0;					/* 範囲選択開始桁 */
		m_nSelectLineTo = rcSel.bottom + 1;		/* 範囲選択終了行 */
		m_nSelectColmTo = 0;					/* 範囲選択終了桁 */
//		m_bBeginBoxSelect = FALSE;	2004.06.22 Moca 上のDisableSelectAreaでクリア済み
	}else{
		nSelectLineFromOld = m_nSelectLineFrom;	/* 範囲選択開始行 */
		nSelectColFromOld = 0;					/* 範囲選択開始桁 */
		nSelectLineToOld = m_nSelectLineTo;		/* 範囲選択終了行 */
		if( m_nSelectColmTo > 0 ){
			++nSelectLineToOld;					/* 範囲選択終了行 */
		}
		nSelectColToOld = 0;					/* 範囲選択終了桁 */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( TRUE );
		m_nSelectLineFrom = nSelectLineFromOld;	/* 範囲選択開始行 */
		m_nSelectColmFrom = nSelectColFromOld; 	/* 範囲選択開始桁 */
		m_nSelectLineTo = nSelectLineToOld;		/* 範囲選択終了行 */
		m_nSelectColmTo = nSelectColToOld;		/* 範囲選択終了桁 */
	}
	/* 再描画 */
	//	::UpdateWindow();
	hdc = ::GetDC( m_hWnd );
	ps.rcPaint.left = 0;
	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
	ps.rcPaint.top = m_nViewAlignTop;
	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	OnPaint( hdc, &ps, FALSE );
	// To Here 2007.09.09 Moca
	::ReleaseDC( m_hWnd, hdc );
	/* 選択範囲をクリップボードにコピー */
	/* 選択範囲のデータを取得 */
	/* 正常時はTRUE,範囲未選択の場合は終了する */
	if( FALSE == GetSelectedData(
		cmemBuf,
		FALSE,
		pszQuote, /* 引用符 */
		bWithLineNumber, /* 行番号を付与する */
		m_pShareData->m_Common.m_bAddCRLFWhenCopy /* 折り返し位置に改行記号を入れる */
	) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* クリップボードにデータを設定 */
	MySetClipboardData( cmemBuf.GetPtr(), cmemBuf.GetLength(), FALSE );


//	/* Windowsクリップボードにコピー */
//	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, cmemBuf.GetLength() + 1 );
//	pszClip = (char*)::GlobalLock( hgClip );
//	memcpy( pszClip, cmemBuf.GetPtr(), cmemBuf.GetLength() + 1 );
//	::GlobalUnlock( hgClip );
//	::OpenClipboard( m_hWnd );
//	::EmptyClipboard();
//	::SetClipboardData( CF_OEMTEXT, hgClip );
//	::CloseClipboard();
	return;
}

/* 選択エリアのテキストを指定方法で変換 */
void CEditView::ConvSelectedArea( int nFuncCode )
{
	CMemory		cmemBuf;
	int			nNewLine;		/* 挿入された部分の次の位置の行 */
	int			nNewPos;		/* 挿入された部分の次の位置のデータ位置 */
	COpe*		pcOpe = NULL;
//	BOOL		bBoxSelected;
//	HDC			hdc;
//	PAINTSTRUCT	ps;
	RECT		rcSel;

	int			nPosX;
	int			nPosY;
	int			nIdxFrom;
	int			nIdxTo;
	int			nLineNum;
	int			nDelPos;
	int			nDelLen;
	int			nDelPosNext;
	int			nDelLenNext;
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			i;
	CMemory*	pcMemDeleted;
	CWaitCursor cWaitCursor( m_hWnd );

	int			nSelectLineFromOld;				/* 範囲選択開始行 */
	int			nSelectColFromOld; 				/* 範囲選択開始桁 */
	int			nSelectLineToOld;				/* 範囲選択終了行 */
	int			nSelectColToOld;				/* 範囲選択終了桁 */
	BOOL		bBeginBoxSelectOld;

	/* テキストが選択されているか */
	if( !IsTextSelected() ){
		return;
	}

	nSelectLineFromOld	= m_nSelectLineFrom;	/* 範囲選択開始行 */
	nSelectColFromOld	= m_nSelectColmFrom;	/* 範囲選択開始桁 */
	nSelectLineToOld	= m_nSelectLineTo;		/* 範囲選択終了行 */
	nSelectColToOld		= m_nSelectColmTo;		/* 範囲選択終了桁 */
	bBeginBoxSelectOld	= m_bBeginBoxSelect;


	/* 矩形範囲選択中か */
	if( m_bBeginBoxSelect ){

		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,					/* 範囲選択開始行 */
			m_nSelectColmFrom,					/* 範囲選択開始桁 */
			m_nSelectLineTo,					/* 範囲選択終了行 */
			m_nSelectColmTo						/* 範囲選択終了桁 */
		);

		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( TRUE );

		nIdxFrom = 0;
		nIdxTo = 0;
		for( nLineNum = rcSel.bottom; nLineNum >= rcSel.top - 1; nLineNum-- ){
			const CLayout* pcLayout;
			nDelPosNext = nIdxFrom;
			nDelLenNext	= nIdxTo - nIdxFrom;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* 指定された桁に対応する行のデータ内の位置を調べる */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );

				for( i = nIdxFrom; i <= nIdxTo; ++i ){
					if( pLine[i] == CR || pLine[i] == LF ){
						nIdxTo = i;
						break;
					}
				}
			}else{
				nIdxFrom	= 0;
				nIdxTo		= 0;
			}
			nDelPos = nDelPosNext;
			nDelLen	= nDelLenNext;
			if( nLineNum < rcSel.bottom && 0 < nDelLen ){
				pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2, &pcLayout );
				nPosX = LineIndexToColmn( pcLayout, nDelPos );
				nPosY =  nLineNum + 1;
				if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						nPosX,
						nPosY,
						&pcOpe->m_nCaretPosX_PHY_Before,
						&pcOpe->m_nCaretPosY_PHY_Before
					);
				}else{
					pcOpe = NULL;
				}

				pcMemDeleted = new CMemory;
				/* 指定位置の指定長データ削除 */
				DeleteData2(
					nPosX/*rcSel.left*/,
					nPosY/*nLineNum + 1*/,
					nDelLen,
					pcMemDeleted,
					pcOpe		/* 編集操作要素 COpe */
//					FALSE,
//					FALSE
				);
				cmemBuf.SetData( pcMemDeleted );
				if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						nPosX,
						nPosY,
						&pcOpe->m_nCaretPosX_PHY_After,
						&pcOpe->m_nCaretPosY_PHY_After
					);
					/* 操作の追加 */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}else{
					delete pcMemDeleted;
					pcMemDeleted = NULL;
				}
				/* 機能種別によるバッファの変換 */
				ConvMemory( &cmemBuf, nFuncCode );
				if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						nPosX,
						nPosY,
						&pcOpe->m_nCaretPosX_PHY_Before,
						&pcOpe->m_nCaretPosY_PHY_Before
					);
				}
				/* 現在位置にデータを挿入 */
				InsertData_CEditView(
					nPosX,
					nPosY,
					cmemBuf.GetPtr(),
					cmemBuf.GetLength(),
					&nNewLine,
					&nNewPos,
					pcOpe,
					TRUE/*FALSE*/
				);
				/* カーソルを移動 */
				MoveCursor( nNewPos, nNewLine, FALSE );
				m_nCaretPosX_Prev = m_nCaretPosX;
				if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
					pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* 操作後のキャレット位置Ｘ */
					pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* 操作後のキャレット位置Ｙ */
					/* 操作の追加 */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}
			}
		}
		/* 挿入データの先頭位置へカーソルを移動 */
		MoveCursor( rcSel.left, rcSel.top, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;

		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;									/* 操作種別 */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* 操作前のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* 操作前のキャレット位置Ｙ */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}else{
		/* 選択範囲のデータを取得 */
		/* 正常時はTRUE,範囲未選択の場合はFALSEを返す */
		GetSelectedData( cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy );

		/* 機能種別によるバッファの変換 */
		ConvMemory( &cmemBuf, nFuncCode );

//		/* 選択エリアを削除 */
//		DeleteData( FALSE );

		int nCaretPosYOLD=m_nCaretPosY;

		/* データ置換 削除&挿入にも使える */
		ReplaceData_CEditView(
			m_nSelectLineFrom,		/* 範囲選択開始行 */
			m_nSelectColmFrom,		/* 範囲選択開始桁 */
			m_nSelectLineTo,		/* 範囲選択終了行 */
			m_nSelectColmTo,		/* 範囲選択終了桁 */
			NULL,					/* 削除されたデータのコピー(NULL可能) */
			cmemBuf.GetPtr(),		/* 挿入するデータ */ // 2002/2/10 aroka CMemory変更
			cmemBuf.GetLength(),		/* 挿入するデータの長さ */ // 2002/2/10 aroka CMemory変更
			FALSE/*TRUEbRedraw*/
		);

		// From Here 2001.12.03 hor
		//	選択エリアの復元
		m_nSelectLineFrom	=	nSelectLineFromOld;	/* 範囲選択開始行 */
		m_nSelectColmFrom	=	nSelectColFromOld;	/* 範囲選択開始桁 */
		m_nSelectLineTo		=	m_nCaretPosY;		/* 範囲選択終了行 */
		m_nSelectColmTo		=	m_nCaretPosX;		/* 範囲選択終了桁 */
		if(nCaretPosYOLD==m_nSelectLineFrom) {
			MoveCursor( m_nSelectColmFrom, m_nSelectLineFrom, TRUE );
		}else{
			MoveCursor( m_nSelectColmTo, m_nSelectLineTo, TRUE );
		}
		m_nCaretPosX_Prev = m_nCaretPosX;
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;									/* 操作種別 */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* 操作前のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* 操作前のキャレット位置Ｙ */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
		RedrawAll();
		// To Here 2001.12.03 hor

		return;


#if 0///////////////////////////////
		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe = new COpe;
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* 操作前のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* 操作前のキャレット位置Ｙ */
		}
		/* 現在位置にデータを挿入 */
		InsertData_CEditView(
			m_nCaretPosX,
			m_nCaretPosY,
			cmemBuf.GetPtr(),
			cmemBuf.GetLength(),
			&nNewLine,
			&nNewPos,
			pcOpe,
			TRUE/*FALSE*/
		);

		/* カーソルを移動 */
		MoveCursor( nNewPos, nNewLine, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;

		if( !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* 操作後のキャレット位置Ｘ */
			pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* 操作後のキャレット位置Ｙ */
			/* 操作の追加 */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
#endif ///////////////////////////////

	}
#if 0///////////////////////////////
	m_nSelectLineFrom	= nSelectLineFromOld;	/* 範囲選択開始行 */
	m_nSelectColmFrom	= nSelectColFromOld;	/* 範囲選択開始桁 */
	m_nSelectLineTo		= nSelectLineToOld;		/* 範囲選択終了行 */
	m_nSelectColmTo		= nSelectColToOld;		/* 範囲選択終了桁 */
	m_bBeginBoxSelect	= bBeginBoxSelectOld;

	m_pcEditDoc->SetModified(true);	/* 変更フラグ */

	/* 再描画 */
	//	::UpdateWindow();
	hdc = ::GetDC( m_hWnd );
	ps.rcPaint.left		= 0;
	ps.rcPaint.right	= m_nViewAlignLeft + m_nViewCx;
	ps.rcPaint.top		= m_nViewAlignTop;
	ps.rcPaint.bottom	= m_nViewAlignTop + m_nViewCy;
	OnKillFocus();
	OnPaint( hdc, &ps, FALSE );
	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );
#endif ///////////////////////////////


	return;
}


/* 機能種別によるバッファの変換 */
void CEditView::ConvMemory( CMemory* pCMemory, int nFuncCode )
{
	switch( nFuncCode ){
	case F_TOLOWER: pCMemory->ToLower(); break;						/* 小文字 */
	case F_TOUPPER: pCMemory->ToUpper(); break;						/* 大文字 */
	case F_TOHANKAKU: pCMemory->ToHankaku( 0x0 ); break;					/* 全角→半角 */
	case F_TOHANKATA: pCMemory->ToHankaku( 0x01 ); break;					/* 全角カタカナ→半角カタカナ */	// Aug. 29, 2002 ai
	case F_TOZENEI: pCMemory->ToZenkaku( 2, 0 );				/* 2== 英数専用				*/ break;	/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	case F_TOHANEI: pCMemory->ToHankaku( 0x4 );						/* 2== 英数専用				*/ break;	/* 全角英数→半角英数 */			//July. 30, 2001 Misaka
	case F_TOZENKAKUKATA: pCMemory->ToZenkaku( 0, 0 );			/* 1== ひらがな 0==カタカナ */ break;	/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA: pCMemory->ToZenkaku( 1, 0 );			/* 1== ひらがな 0==カタカナ */ break;	/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKAKUKATA: pCMemory->ToZenkaku( 0, 1 );	/* 1== ひらがな 0==カタカナ */ break;	/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENKAKUHIRA: pCMemory->ToZenkaku( 1, 1 );	/* 1== ひらがな 0==カタカナ */ break;	/* 半角カタカナ→全角ひらがな */
	case F_CODECNV_EMAIL:		pCMemory->JIStoSJIS(); break;		/* E-Mail(JIS→SJIS)コード変換 */
	case F_CODECNV_EUC2SJIS:	pCMemory->EUCToSJIS(); break;		/* EUC→SJISコード変換 */
	case F_CODECNV_UNICODE2SJIS:pCMemory->UnicodeToSJIS(); break;	/* Unicode→SJISコード変換 */
	case F_CODECNV_UNICODEBE2SJIS: pCMemory->UnicodeBEToSJIS(); break;	/* UnicodeBE→SJISコード変換 */
	case F_CODECNV_SJIS2JIS:	pCMemory->SJIStoJIS();break;		/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC: 	pCMemory->SJISToEUC();break;		/* SJIS→EUCコード変換 */
	case F_CODECNV_UTF82SJIS:	pCMemory->UTF8ToSJIS();break;		/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS:	pCMemory->UTF7ToSJIS();break;		/* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2UTF7:	pCMemory->SJISToUTF7();break;		/* SJIS→UTF-7コード変換 */
	case F_CODECNV_SJIS2UTF8:	pCMemory->SJISToUTF8();break;		/* SJIS→UTF-8コード変換 */
	case F_CODECNV_AUTO2SJIS:	pCMemory->AUTOToSJIS();break;		/* 自動判別→SJISコード変換 */
	case F_TABTOSPACE:
		pCMemory->TABToSPACE(
			//	Sep. 23, 2002 genta LayoutMgrの値を使う
			m_pcEditDoc->m_cLayoutMgr.GetTabSpace()
		);break;	/* TAB→空白 */
	case F_SPACETOTAB:	//#### Stonee, 2001/05/27
		pCMemory->SPACEToTAB(
			//	Sep. 23, 2002 genta LayoutMgrの値を使う
			m_pcEditDoc->m_cLayoutMgr.GetTabSpace()
		);
		break;		/* 空白→TAB */
	case F_LTRIM:	Command_TRIM2( pCMemory , TRUE  );break;	// 2001.12.03 hor
	case F_RTRIM:	Command_TRIM2( pCMemory , FALSE );break;	// 2001.12.03 hor
	}
	return;

}



/* 指定された桁に対応する行のデータ内の位置を調べる Ver1
	
	@@@ 2002.09.28 YAZAKI CDocLine版
*/
int CEditView::LineColmnToIndex( const CDocLine* pcDocLine, int nColumn )
{
	int i2 = 0;
	CMemoryIterator<CDocLine> it( pcDocLine, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}


/* 指定された桁に対応する行のデータ内の位置を調べる Ver1
	
	@@@ 2002.09.28 YAZAKI CLayoutが必要になりました。
*/
int CEditView::LineColmnToIndex( const CLayout* pcLayout, int nColumn )
{
	int i2 = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}



/* 指定された桁に対応する行のデータ内の位置を調べる Ver0 */
/* 指定された桁より、行が短い場合はpnLineAllColLenに行全体の表示桁数を返す */
/* それ以外の場合はpnLineAllColLenに０をセットする
	
	@@@ 2002.09.28 YAZAKI CLayoutが必要になりました。
*/
int CEditView::LineColmnToIndex2( const CLayout* pcLayout, int nColumn, int& pnLineAllColLen )
{
	pnLineAllColLen = 0;

	int i2 = 0;
	int nPosX2 = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	if( i2 >= pcLayout->GetLength() ){
		nPosX2 += it.getColumn();
		pnLineAllColLen = nPosX2;
	}
	return i2;
}





/*
||	指定された行のデータ内の位置に対応する桁の位置を調べる
||
||	@@@ 2002.09.28 YAZAKI CLayoutが必要になりました。
*/
int CEditView::LineIndexToColmn( const CLayout* pcLayout, int nIndex )
{
	//	以下、iterator版
	int nPosX2 = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > nIndex ){
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
}


/*
||	指定された行のデータ内の位置に対応する桁の位置を調べる
||
||	@@@ 2002.09.28 YAZAKI CDocLine版
*/
int CEditView::LineIndexToColmn( const CDocLine* pcDocLine, int nIndex )
{
	int nPosX2 = 0;
	CMemoryIterator<CDocLine> it( pcDocLine, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > nIndex ){
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
}



/* ポップアップメニュー(右クリック) */
int	CEditView::CreatePopUpMenu_R( void )
{
	int			nId;
//	HMENU		hMenuTop;
	HMENU		hMenu;
	POINT		po;
//	UINT		fuFlags;
//	int			cMenuItems;
//	int			nPos;
	RECT		rc;
	CMemory		cmemCurText;
	char*		pszWork;
	int			i;
	int			nMenuIdx;
	char		szLabel[300];
	char		szLabel2[300];
	UINT		uFlags;
//	BOOL		bBool;


	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->m_CMenuDrawer.ResetContents();

	/* 右クリックメニューの定義はカスタムメニュー配列の0番目 */
	nMenuIdx = CUSTMENU_INDEX_FOR_RBUTTONUP;	//マジックナンバー排除	//@@@ 2003.06.13 MIK
//	if( nMenuIdx < 0 || MAX_CUSTOM_MENU	<= nMenuIdx ){
//		return 0;
//	}
//	if( 0 == m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx] ){
//		return 0;
//	}

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = m_pcEditDoc->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
	for( i = 0; i < m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( 0 == m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		}else{
			//	Oct. 3, 2001 genta
			FuncLookup.Funccode2Name( m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
//			::LoadString( m_hInstance, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* キー */
			if( '\0' == m_pShareData->m_Common.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				strcpy( szLabel2, szLabel );
			}else{
				wsprintf( szLabel2, "%s (&%c)", szLabel, m_pShareData->m_Common.m_nCustMenuItemKeyArr[nMenuIdx][i] );
			}
			/* 機能が利用可能か調べる */
			if( TRUE == CEditWnd::IsFuncEnable( m_pcEditDoc, m_pShareData, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
//			bBool = ::AppendMenu( hMenu, uFlags, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel2 );
			pCEditWnd->m_CMenuDrawer.MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] , szLabel2 );

		}
	}

	if( !m_bBeginSelect ){	/* 範囲選択中 */
		if( TRUE == KeyWordHelpSearchDict( LID_SKH_POPUPMENU_R, &po, &rc ) ){	// 2006.04.10 fon
			pszWork = m_cTipWnd.m_cInfo.GetPtr();
			// 2002.05.25 Moca &の考慮を追加 
			char*	pszShortOut = new char[160 + 1];
			if( 80 < lstrlen( pszWork ) ){
				char*	pszShort = new char[80 + 1];
				memcpy( pszShort, pszWork, 80 );
				pszShort[80] = '\0';
				dupamp( (const char*)pszShort, pszShortOut );
				delete [] pszShort;
			}else{
				dupamp( (const char*)pszWork, pszShortOut );
			}
			::InsertMenu( hMenu, 0, MF_BYPOSITION, IDM_COPYDICINFO, "キーワードの説明をクリップボードにコピー(&K)" );	// 2006.04.10 fon ToolTip内容を直接表示するのをやめた
			delete [] pszShortOut;
			::InsertMenu( hMenu, 1, MF_BYPOSITION, IDM_JUMPDICT, "キーワード辞書を開く(&J)" );	// 2006.04.10 fon
			::InsertMenu( hMenu, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
		}
	}
	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		::GetParent( m_hwndParent )/*m_hWnd*/,
		NULL
	);
	::DestroyMenu( hMenu );
	return nId;
}



/*! 親ウィンドウのタイトルを更新

	@date 2007.03.08 ryoji bKillFocusパラメータを除去
*/
void CEditView::SetParentCaption( void )
{
	m_pcEditDoc->SetParentCaption();
	return;
}


/*! キャレットの行桁位置およびステータスバーの状態表示の更新

	@note ステータスバーの状態の並び方の変更はメッセージを受信する
		CEditWnd::DispatchEvent()のWM_NOTIFYにも影響があることに注意
	
	@note ステータスバーの出力内容の変更はCEditWnd::OnSize()の
		カラム幅計算に影響があることに注意
*/
void CEditView::DrawCaretPosInfo( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}

	char			szText[64];
	HWND			hwndFrame;
	unsigned char*	pLine;
	int				nLineLen;
	int				nIdxFrom;
	int				nCharChars;
	CEditWnd*		pCEditWnd;
	const CLayout*	pcLayout;
	// 2002.05.26 Moca  gm_pszCodeNameArr_2 を使う
	const char* pCodeName = gm_pszCodeNameArr_2[m_pcEditDoc->m_nCharCode];
//	2002/04/08 YAZAKI コードの重複を削除

	hwndFrame = ::GetParent( m_hwndParent );
	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	/* カーソル位置の文字コード */
//	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen );
	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );

	//	May 12, 2000 genta
	//	改行コードの表示を追加
	//	From Here
	CEOL cNlType = m_pcEditDoc->GetNewLineCode();
	const char *nNlTypeName = cNlType.GetName();
	//	To Here

	int nPosX, nPosY;
	if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
		if (pcLayout && pcLayout->m_nOffset){
			char* pLine = pcLayout->m_pCDocLine->GetPtr();
			int nLineLen = m_nCaretPosX_PHY;	//	 - pcLayout->m_nOffset;
			nPosX = 0;
			int i;
			//	Oct. 4, 2002 genta
			//	文字位置のカウント方法が誤っていたのを修正
			for( i = 0; i < nLineLen; ){
				// 2005-09-02 D.S.Koba GetSizeOfChar
				int nCharChars = CMemory::GetSizeOfChar( (const char *)pLine, nLineLen, i );
				if ( nCharChars == 1 && pLine[i] == TAB ){
					//	Sep. 23, 2002 genta LayoutMgrの値を使う
					nPosX += m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( nPosX );
					++i;
				}
				else {
					nPosX += nCharChars;
					i += nCharChars;
				}
			}
			nPosX ++;	//	補正
		}
		else {
			nPosX = m_nCaretPosX + 1;
		}
		nPosY = m_nCaretPosY_PHY + 1;
	}
	else {
		nPosX = m_nCaretPosX + 1;
		nPosY = m_nCaretPosY + 1;
	}

	/* ステータス情報を書き出す */
	if( NULL == pCEditWnd->m_hwndStatusBar ){
		/* ウィンドウ右上に書き出す */
		//	May 12, 2000 genta
		//	改行コードの表示を追加
		//	From Here
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdxFrom = LineColmnToIndex( pcLayout, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ){
				/* szText */
				wsprintf( szText, "%s(%s)       %6d：%d", pCodeName, nNlTypeName, nPosY, nPosX );	//Oct. 31, 2000 JEPRO //Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
			}else{
				if( nIdxFrom < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( (char *)pLine, nLineLen, nIdxFrom );
				}else{
					nCharChars = pcLayout->m_cEol.GetLen();
				}
				switch( nCharChars ){
				case 1:
					/* szText */
					wsprintf( szText, "%s(%s)   [%02x]%6d：%d", pCodeName, nNlTypeName, pLine[nIdxFrom], nPosY, nPosX );//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
					break;
				case 2:
					/* szText */
					wsprintf( szText, "%s(%s) [%02x%02x]%6d：%d", pCodeName, nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , nPosY, nPosX);//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
					break;
				case 4:
					/* szText */
					wsprintf( szText, "%s(%s) [%02x%02x%02x%02x]%d：%d", pCodeName, nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] , nPosY, nPosX);//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
					break;
				default:
					/* szText */
					wsprintf( szText, "%s(%s)       %6d：%d", pCodeName, nNlTypeName, nPosY, nPosX );//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
				}
			}
		}else{
			/* szText */
			wsprintf( szText, "%s(%s)       %6d：%d", pCodeName, nNlTypeName, nPosY, nPosX );//Oct. 31, 2000 JEPRO メニューバーでの表示桁を節約
		}
		//	To Here
		//	Dec. 4, 2002 genta メニューバー表示はCEditWndが行う
		m_pcEditDoc->m_pcEditWnd->PrintMenubarMessage( szText );
	}else{
		/* ステータスバーに状態を書き出す */
		char	szText_1[64];
		char	szText_3[32]; // szText_2 => szTest_3 に変更 64バイトもいらない 2002.06.05 Moca 
		char	szText_6[16]; // szText_5 => szTest_6 に変更 64バイトもいらない 2002.06.05 Moca
		wsprintf( szText_1, "%5d 行 %4d 桁", nPosY, nPosX );	//Oct. 30, 2000 JEPRO 千万行も要らん

		nCharChars = 0;
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdxFrom = LineColmnToIndex( pcLayout, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ){
			}else{
				if( nIdxFrom < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( (char *)pLine, nLineLen, nIdxFrom );
				}else{
					nCharChars = pcLayout->m_cEol.GetLen();
				}
			}
		}

		if( 1 == nCharChars ){
			wsprintf( szText_3, "%02x  ", pLine[nIdxFrom] );
		}else
		if( 2 == nCharChars ){
			wsprintf( szText_3, "%02x%02x", pLine[nIdxFrom],  pLine[nIdxFrom + 1] );
		// 2003.08.26 Moca CR0LF0廃止で 4 == nCharChars を削除
		}else{
			wsprintf( szText_3, "    " );
		}

		if( IsInsMode() /* Oct. 2, 2005 genta */ ){
			strcpy( szText_6, "挿入" );
		}else{
			strcpy( szText_6, "上書" );
		}
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)"" );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 1 | 0, (LPARAM) (LPINT)szText_1 );
		//	May 12, 2000 genta
		//	改行コードの表示を追加．後ろの番号を1つずつずらす
		//	From Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 2 | 0, (LPARAM) (LPINT)nNlTypeName );
		//	To Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 3 | 0, (LPARAM) (LPINT)szText_3 );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 4 | 0, (LPARAM) (LPINT)gm_pszCodeNameArr_1[m_pcEditDoc->m_nCharCode] );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 5 | SBT_OWNERDRAW, (LPARAM) (LPINT)"" );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 6 | 0, (LPARAM) (LPINT)szText_6 );
	}

	return;
}

/*!	選択範囲情報メッセージの表示

	@author genta
	@date 2005.07.09 genta 新規作成
	@date 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策を追加
*/
void CEditView::PrintSelectionInfoMsg(void)
{
	//	出力されないなら計算を省略
	if( ! m_pcEditDoc->m_pcEditWnd->SendStatusMessage2IsEffective() )
		return;

	if( ! IsTextSelected() ){
		m_pcEditDoc->m_pcEditWnd->SendStatusMessage2( "" );
		return;
	}

	char msg[128];
	//	From here 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策
	int nLineCount = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( m_nSelectLineFrom >= nLineCount ){	// 先頭行が実在しない
		m_pcEditDoc->m_pcEditWnd->SendStatusMessage2( "" );
		return;
	}
	int select_line;
	if( m_nSelectLineTo >= nLineCount ){	// 最終行が実在しない
		select_line = nLineCount - m_nSelectLineFrom + 1;
	}
	else {
		select_line = m_nSelectLineTo - m_nSelectLineFrom + 1;
	}
	//	To here 2006.06.06 ryoji 選択範囲の行が実在しない場合の対策
	if( m_bBeginBoxSelect ){
		//	矩形の場合は幅と高さだけでごまかす
		int select_col = m_nSelectColmFrom - m_nSelectColmTo;
		if( select_col < 0 ){
			select_col = -select_col;
		}
		wsprintf( msg, "%d Columns * %d lines selected.",
			select_col, select_line );
			
	}
	else {
		//	通常の選択では選択範囲の中身を数える
		int select_sum = 0;	//	バイト数合計
		const char *pLine;	//	データを受け取る
		int	nLineLen;		//	行の長さ
		const CLayout*	pcLayout;

		//	1行目
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineFrom, &nLineLen, &pcLayout );
		if( pLine ){
			//	1行だけ選択されている場合
			if( m_nSelectLineFrom == m_nSelectLineTo ){
				select_sum = LineColmnToIndex( pcLayout, m_nSelectColmTo )
					- LineColmnToIndex( pcLayout, m_nSelectColmFrom );
			}
			else {	//	2行以上選択されている場合
				select_sum = pcLayout->GetLengthWithoutEOL() + pcLayout->m_cEol.GetLen()
					- LineColmnToIndex( pcLayout, m_nSelectColmFrom );

				//	GetSelectedDataと似ているが，先頭行と最終行は排除している
				//	Aug. 16, 2005 aroka nLineNumはfor以降でも使われるのでforの前で宣言する
				//	VC .NET以降でもMicrosoft拡張を有効にした標準動作はVC6と同じことに注意
				int nLineNum;
				for( nLineNum = m_nSelectLineFrom + 1;
					nLineNum < m_nSelectLineTo; ++nLineNum ){
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
					//	2006.06.06 ryoji 指定行のデータが存在しない場合の対策
					if( NULL == pLine )
						break;
					select_sum += pcLayout->GetLengthWithoutEOL() + pcLayout->m_cEol.GetLen();
				}

				//	最終行の処理
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
				if( pLine ){
					int last_line_chars = LineColmnToIndex( pcLayout, m_nSelectColmTo );
					select_sum += last_line_chars;
					if( last_line_chars == 0 ){
						//	最終行の先頭にキャレットがある場合は
						//	その行を行数に含めない
						--select_line;
					}
				}
				else
				{
					//	最終行が空行なら
					//	その行を行数に含めない
					--select_line;
				}
			}
		}

#ifdef _DEBUG
		wsprintf( msg, "%d bytes (%d lines) selected. [%d:%d]-[%d:%d]",
			select_sum, select_line,
			m_nSelectColmFrom, m_nSelectLineFrom,
			m_nSelectColmTo, m_nSelectLineTo );
#else
		wsprintf( msg, "%d bytes (%d lines) selected.", select_sum, select_line );
#endif
	}
	m_pcEditDoc->m_pcEditWnd->SendStatusMessage2( msg );
}


/* 設定変更を反映させる */
void CEditView::OnChangeSetting( void )
{
	RECT		rc;
	LOGFONT		lf;

	m_nTopYohaku = m_pShareData->m_Common.m_nRulerBottomSpace; 		/* ルーラーとテキストの隙間 */
	m_nViewAlignTop = m_nTopYohaku;									/* 表示域の上端座標 */

	/* ルーラー表示 */
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ルーラー高さ */
	}

	/* フォント作成 */
	::DeleteObject( m_hFont_HAN );
	m_hFont_HAN = CreateFontIndirect( &(m_pShareData->m_Common.m_lf) );

	/* 太字フォント作成 */
	::DeleteObject( m_hFont_HAN_FAT );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT = CreateFontIndirect( &lf );

	/* 下線フォント作成 */
	::DeleteObject( m_hFont_HAN_UL );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* 太字下線フォント作成 */
	::DeleteObject( m_hFont_HAN_FAT_UL );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT_UL = CreateFontIndirect( &lf );



	/* フォントの変更 */
	SetFont();

	/* フォントが変わっているかもしれないので、カーソル移動 */
	MoveCursor( m_nCaretPosX, m_nCaretPosY, TRUE );


	/* スクロールバーの状態を更新する */
	AdjustScrollBars();
	
	//	2007.09.30 genta 画面キャッシュ用CompatibleDCを用意する
	UseCompatibleDC( m_pShareData->m_Common.m_bUseCompotibleBMP );

	/* ウィンドウサイズの変更処理 */
	::GetClientRect( m_hWnd, &rc );
	OnSize( rc.right, rc.bottom );

	/* 再描画 */
	::InvalidateRect( m_hWnd, NULL, TRUE );

	return;
}




/* フォーカス移動時の再描画

	@date 2001/06/21 asa-o 「スクロールバーの状態を更新する」「カーソル移動」削除
*/
void CEditView::RedrawAll( void )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	/* 再描画 */
	hdc = ::GetDC( m_hWnd );

//	OnKillFocus();

	::GetClientRect( m_hWnd, &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );
//	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );

	/* キャレットの表示 */
	ShowEditCaret();

	/* キャレットの行桁位置を表示する */
	DrawCaretPosInfo();

	/* 親ウィンドウのタイトルを更新 */
	SetParentCaption();

	//	Jul. 9, 2005 genta	選択範囲の情報をステータスバーへ表示
	PrintSelectionInfoMsg();

	/* スクロールバーの状態を更新する */
	AdjustScrollBars();

	return;
}

// 2001/06/21 Start by asa-o 再描画
void CEditView::Redraw( void )
{
	HDC			hdc;
	PAINTSTRUCT	ps;

	hdc = ::GetDC( m_hWnd );

	::GetClientRect( m_hWnd, &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );

	::ReleaseDC( m_hWnd, hdc );

	return;
}
// 2001/06/21 End

/* 自分の表示状態を他のビューにコピー */
void CEditView::CopyViewStatus( CEditView* pView )
{
	if( pView == NULL ){
		return;
	}
	if( pView == this ){
		return;
	}

	/* 入力状態 */
	pView->m_nCaretPosX 			= m_nCaretPosX;			/* ビュー左端からのカーソル桁位置（０開始）*/
	pView->m_nCaretPosX_Prev		= m_nCaretPosX_Prev;	/* ビュー左端からのカーソル桁位置（０オリジン）*/
	pView->m_nCaretPosY				= m_nCaretPosY;			/* ビュー上端からのカーソル行位置（０開始）*/
//	キャレットの幅・高さはコピーしない。2002/05/12 YAZAKI
//	pView->m_nCaretWidth			= m_nCaretWidth;		/* キャレットの幅 */
//	pView->m_nCaretHeight			= m_nCaretHeight;		/* キャレットの高さ */

	/* キー状態 */
	pView->m_bSelectingLock			= m_bSelectingLock;		/* 選択状態のロック */
	pView->m_bBeginSelect			= m_bBeginSelect;		/* 範囲選択中 */
	pView->m_bBeginBoxSelect		= m_bBeginBoxSelect;	/* 矩形範囲選択中 */

	pView->m_nSelectLineBgnFrom		= m_nSelectLineBgnFrom;	/* 範囲選択開始行(原点) */
	pView->m_nSelectColmBgnFrom		= m_nSelectColmBgnFrom;	/* 範囲選択開始桁(原点) */
	pView->m_nSelectLineBgnTo		= m_nSelectLineBgnTo;	/* 範囲選択開始行(原点) */
	pView->m_nSelectColmBgnTo		= m_nSelectColmBgnTo;	/* 範囲選択開始桁(原点) */

	pView->m_nSelectLineFrom		= m_nSelectLineFrom;	/* 範囲選択開始行 */
	pView->m_nSelectColmFrom		= m_nSelectColmFrom;	/* 範囲選択開始桁 */
	pView->m_nSelectLineTo			= m_nSelectLineTo;		/* 範囲選択終了行 */
	pView->m_nSelectColmTo			= m_nSelectColmTo;		/* 範囲選択終了桁 */
	pView->m_nSelectLineFromOld		= m_nSelectLineFromOld;	/* 範囲選択開始行 */
	pView->m_nSelectColmFromOld		= m_nSelectColmFromOld;	/* 範囲選択開始桁 */
	pView->m_nSelectLineToOld		= m_nSelectLineToOld;	/* 範囲選択終了行 */
	pView->m_nSelectColmToOld		= m_nSelectColmToOld;	/* 範囲選択終了桁 */
	pView->m_nMouseRollPosXOld		= m_nMouseRollPosXOld;	/* マウス範囲選択前回位置(X座標) */
	pView->m_nMouseRollPosYOld		= m_nMouseRollPosYOld;	/* マウス範囲選択前回位置(Y座標) */

	/* 画面情報 */
	pView->m_nViewAlignLeft			= m_nViewAlignLeft;		/* 表示域の左端座標 */
	pView->m_nViewAlignLeftCols		= m_nViewAlignLeftCols;	/* 行番号域の桁数 */
	pView->m_nViewAlignTop			= m_nViewAlignTop;		/* 表示域の上端座標 */
//	pView->m_nViewCx				= m_nViewCx;			/* 表示域の幅 */
//	pView->m_nViewCy				= m_nViewCy;			/* 表示域の高さ */
//	pView->m_nViewColNum			= m_nViewColNum;		/* 表示域の桁数 */
//	pView->m_nViewRowNum			= m_nViewRowNum;		/* 表示域の行数 */
	pView->m_nViewTopLine			= m_nViewTopLine;		/* 表示域の一番上の行(0開始) */
	pView->m_nViewLeftCol			= m_nViewLeftCol;		/* 表示域の一番左の桁(0開始) */

	/* 表示方法 */
	pView->m_nCharWidth				= m_nCharWidth;			/* 半角文字の幅 */
	pView->m_nCharHeight			= m_nCharHeight;		/* 文字の高さ */

	return;
}


/* 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ */
void CEditView::SplitBoxOnOff( BOOL bVert, BOOL bHorz, BOOL bSizeBox )
{
	RECT	rc;
	if( bVert ){
		if( NULL != m_pcsbwVSplitBox ){	/* 垂直分割ボックス */
		}else{
			m_pcsbwVSplitBox = new CSplitBoxWnd;
			m_pcsbwVSplitBox->Create( m_hInstance, m_hWnd, TRUE );
		}
	}else{
		if( NULL != m_pcsbwVSplitBox ){	/* 垂直分割ボックス */
			delete m_pcsbwVSplitBox;
			m_pcsbwVSplitBox = NULL;
		}else{
		}
	}
	if( bHorz ){
		if( NULL != m_pcsbwHSplitBox ){	/* 水平分割ボックス */
		}else{
			m_pcsbwHSplitBox = new CSplitBoxWnd;
			m_pcsbwHSplitBox->Create( m_hInstance, m_hWnd, FALSE );
		}
	}else{
		if( NULL != m_pcsbwHSplitBox ){	/* 水平分割ボックス */
			delete m_pcsbwHSplitBox;
			m_pcsbwHSplitBox = NULL;
		}else{
		}
	}

	if( bSizeBox ){
		if( NULL != m_hwndSizeBox ){
			::DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,													/* no extended styles */
			"SCROLLBAR",										/* scroll bar control class */
			(LPSTR) NULL,										/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,													/* horizontal position */
			0,													/* vertical position */
			200,												/* width of the scroll bar */
			CW_USEDEFAULT,										/* default height */
			m_hWnd,												/* handle of main window */
			(HMENU) NULL,										/* no menu for a scroll bar */
			m_hInstance,										/* instance owning this window */
			(LPVOID) NULL										/* pointer not needed */
		);
	}else{
		if( NULL != m_hwndSizeBox ){
			DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,														/* no extended styles */
			"STATIC",												/* scroll bar control class */
			(LPSTR) NULL,											/* text for window title bar */
			WS_VISIBLE | WS_CHILD /*| SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,														/* horizontal position */
			0,														/* vertical position */
			200,													/* width of the scroll bar */
			CW_USEDEFAULT,											/* default height */
			m_hWnd,													/* handle of main window */
			(HMENU) NULL,											/* no menu for a scroll bar */
			m_hInstance,											/* instance owning this window */
			(LPVOID) NULL											/* pointer not needed */
		);
	}
	::ShowWindow( m_hwndSizeBox, SW_SHOW );

	::GetClientRect( m_hWnd, &rc );
	OnSize( rc.right, rc.bottom );

	return;
}





/* Grep実行 */
DWORD CEditView::DoGrep(
	const CMemory*	pcmGrepKey,
	const CMemory*	pcmGrepFile,
	const CMemory*	pcmGrepFolder,
	BOOL		bGrepSubFolder,
	BOOL		bGrepLoHiCase,
	BOOL		bGrepRegularExp,
	int			nGrepCharSet,	// 2002/09/21 Moca 文字コードセット選択
	BOOL		bGrepOutputLine,
	BOOL		bWordOnly,
	int			nGrepOutputStyle
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CEditView::DoGrep" );
#endif

	m_pcEditDoc->m_bGrepRunning = TRUE;


	int			nDummy;
	int			nHitCount = 0;
	char		szKey[_MAX_PATH];
	char		szFile[_MAX_PATH];
	char		szPath[_MAX_PATH];
//	int			nNewLine;		/* 挿入された部分の次の位置の行 */
//	int			nNewPos;		/* 挿入された部分の次の位置のデータ位置 */
//	COpe*		pcOpe = NULL;
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel;
	char*		pszWork;
	HWND		hwndMainFrame;
	int			nCharChars;
	//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	CBregexp	cRegexp;
	CMemory		cmemMessage;
	CMemory		cmemWork;
	int			nWork;
	int*		pnKey_CharCharsArr;
	pnKey_CharCharsArr = NULL;

	/*
	|| バッファサイズの調整
	*/
	cmemMessage.AllocBuffer( 4000 );



//	int*				pnKey_CharUsedArr;
//	pnKey_CharUsedArr		= NULL;
//	GrepParam*			pGrepParam;

//	CEditView*			pCEditView;
//	CMemory				cmGrepKey;
//	CMemory				cmGrepFile;
//	CMemory				cmGrepFolder;
//	BOOL				bGrepSubFolder;
//	BOOL				bGrepLoHiCase;
//	BOOL				bGrepRegularExp;
//	int					nGrepCharSet;
//	BOOL				bGrepOutputLine;

//	pGrepParam				= (GrepParam*)dwGrepParam;

//	pCEditView				= (CEditView*)pGrepParam->pCEditView;
//	cmGrepKey				= *pGrepParam->pcmGrepKey;
//	cmGrepFile				= *pGrepParam->pcmGrepFile;
//	cmGrepFolder			= *pGrepParam->pcmGrepFolder;
//	bGrepSubFolder			= pGrepParam->bGrepSubFolder;
//	bGrepLoHiCase			= pGrepParam->bGrepLoHiCase;
//	bGrepRegularExp			= pGrepParam->bGrepRegularExp;
//	nGrepCharSet			= pGrepParam->nGrepCharSet;
//	bGrepOutputLine			= pGrepParam->bGrepOutputLine;

	m_bDoing_UndoRedo		= TRUE;


	/* アンドゥバッファの処理 */
	if( NULL != m_pcOpeBlk ){	/* 操作ブロック */
//@@@2002.2.2 YAZAKI NULLじゃないと進まないので、とりあえずコメント。＆NULLのときは、new COpeBlkする。
//		while( NULL != m_pcOpeBlk ){}
//		delete m_pcOpeBlk;
//		m_pcOpeBlk = NULL;
	}
	else {
		m_pcOpeBlk = new COpeBlk;
	}

	m_bCurSrchKeyMark = TRUE;								/* 検索文字列のマーク */
	strcpy( m_szCurSrchKey, pcmGrepKey->GetPtr() );	/* 検索文字列 */
	m_bCurSrchRegularExp = bGrepRegularExp;					/* 検索／置換  1==正規表現 */
	m_bCurSrchLoHiCase = bGrepLoHiCase;						/* 検索／置換  1==英大文字小文字の区別 */
	/* 正規表現 */

	//	From Here Jun. 27 genta
	/*
		Grepを行うに当たって検索・画面色分け用正規表現バッファも
		初期化する．これはGrep検索結果の色分けを行うため．

		Note: ここで強調するのは最後の検索文字列であって
		Grep対象パターンではないことに注意
	*/
	if( m_bCurSrchRegularExp ){
		//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
		if( !InitRegexp( m_hWnd, m_CurRegexp, true ) ){
			return 0;
		}

		/* 検索パターンのコンパイル */
		int nFlag = 0x00;
		nFlag |= m_bCurSrchLoHiCase ? 0x01 : 0x00;
		m_CurRegexp.Compile( m_szCurSrchKey, nFlag );
	}
	//	To Here Jun. 27 genta

//まだ m_bCurSrchWordOnly = m_pShareData->m_Common.m_bWordOnly;	/* 検索／置換  1==単語のみ検索 */

//	cDlgCancel.Create( m_hInstance, m_hwndParent );
//	hwndCancel = cDlgCancel.Open( MAKEINTRESOURCE(IDD_GREPRUNNING) );
	hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_GREPRUNNING );

	::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE );
	::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, " " );	// 2002/09/09 Moca add
	::CheckDlgButton( hwndCancel, IDC_CHECK_REALTIMEVIEW, m_pShareData->m_Common.m_bGrepRealTimeView );	// 2003.06.23 Moca

	pszWork = pcmGrepKey->GetPtr();
	strcpy( szKey, pszWork );

	strcpy( m_pcEditDoc->m_szGrepKey, szKey );
	m_pcEditDoc->m_bGrepMode = TRUE;

	//	2007.07.22 genta
	//	バージョン番号取得のため，処理を前の方へ移動した
	if( bGrepRegularExp ){
		if( !InitRegexp( m_hWnd, cRegexp, true ) ){
			return 0;
		}
		/* 検索パターンのコンパイル */
		int nFlag = 0x00;
		nFlag |= bGrepLoHiCase ? 0x01 : 0x00;
		if( !cRegexp.Compile( szKey, nFlag ) ){
			return 0;
		}
	}else{
		/* 検索条件の情報 */
		CDocLineMgr::CreateCharCharsArr(
			(const unsigned char *)szKey,
			lstrlen( szKey ),
			&pnKey_CharCharsArr
		);
	}

//	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_BIG, (LPARAM)::LoadIcon( m_hInstance, IDI_QUESTION ) );
//2002.02.08 Grepアイコンも大きいアイコンと小さいアイコンを別々にする。
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta アイコン読み込み方法変更
	hIconBig = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true );

	//	Sep. 10, 2002 genta
	//	CEditWndに新設した関数を使うように
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
	pCEditWnd->SetWindowIcon( hIconBig, ICON_BIG );

	pszWork = pcmGrepFolder->GetPtr();
	strcpy( szPath, pszWork );
	nDummy = lstrlen( szPath );
	/* フォルダの最後が「半角かつ'\\'」でない場合は、付加する */
	nCharChars = &szPath[nDummy] - CMemory::MemCharPrev( szPath, nDummy, &szPath[nDummy] );
	if( 1 == nCharChars && szPath[nDummy - 1] == '\\' ){
	}else{
		strcat( szPath, "\\" );
	}
	strcpy( szFile, pcmGrepFile->GetPtr( /* &nDummy */ ) );

	nWork = lstrlen( szKey ); // 2003.06.10 Moca あらかじめ長さを計算しておく

	/* 最後にテキストを追加 */
	cmemMessage.AppendSz( "\r\n□検索条件  " );
	if( 0 < nWork ){
		CMemory cmemWork2;
		cmemWork2.SetDataSz( szKey );
		if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
			cmemWork2.Replace_j( "\\", "\\\\" );
			cmemWork2.Replace_j( "\'", "\\\'" );
			cmemWork2.Replace_j( "\"", "\\\"" );
		}else{
			cmemWork2.Replace_j( "\'", "\'\'" );
			cmemWork2.Replace_j( "\"", "\"\"" );
		}
		cmemWork.AppendSz( "\"" );
		cmemWork.Append( &cmemWork2 );
		cmemWork.AppendSz( "\"\r\n" );
	}else{
		cmemWork.AppendSz( "「ファイル検索」\r\n" );
	}
	cmemMessage += cmemWork;



	cmemMessage.AppendSz( "検索対象   " );
	cmemWork.SetDataSz( szFile );
	if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;




	cmemMessage.AppendSz( "\r\n" );
	cmemMessage.AppendSz( "フォルダ   " );
	cmemWork.SetDataSz( szPath );
	if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;
	cmemMessage.AppendSz( "\r\n" );

	if( bGrepSubFolder ){
		pszWork = "    (サブフォルダも検索)\r\n";
	}else{
		pszWork = "    (サブフォルダを検索しない)\r\n";
	}
	cmemMessage.AppendSz( pszWork );

	if( 0 < nWork ){ // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if( bWordOnly ){
		/* 単語単位で探す */
			cmemMessage.AppendSz( "    (単語単位で探す)\r\n" );
		}

		if( bGrepLoHiCase ){
			pszWork = "    (英大文字小文字を区別する)\r\n";
		}else{
			pszWork = "    (英大文字小文字を区別しない)\r\n";
		}
		cmemMessage.AppendSz( pszWork );

		if( bGrepRegularExp ){
			//	2007.07.22 genta : 正規表現ライブラリのバージョンも出力する
			cmemMessage.AppendSz( "    (正規表現:" );
			cmemMessage.AppendSz( cRegexp.GetVersion() );
			cmemMessage.AppendSz( ")\r\n" );
		}
	}

	if( CODE_AUTODETECT == nGrepCharSet ){
		cmemMessage.AppendSz( "    (文字コードセットの自動判別)\r\n" );
	}else if( 0 <= nGrepCharSet && nGrepCharSet < CODE_CODEMAX ){
		cmemMessage.AppendSz( "    (文字コードセット：" );
		cmemMessage.AppendSz( gm_pszCodeNameArr_1[nGrepCharSet] );
		cmemMessage.AppendSz( ")\r\n" );
	}

	if( 0 < nWork ){ // 2003.06.10 Moca ファイル検索の場合は表示しない // 2004.09.26 条件誤り修正
		if( bGrepOutputLine ){
		/* 該当行 */
			pszWork = "    (一致した行を出力)\r\n";
		}else{
			pszWork = "    (一致した箇所のみ出力)\r\n";
		}
		cmemMessage.AppendSz( pszWork );
	}


	cmemMessage.AppendSz( "\r\n\r\n" );
	pszWork = cmemMessage.GetPtr( &nWork );
//@@@ 2002.01.03 YAZAKI Grep直後はカーソルをGrep直前の位置に動かす
	int tmp_PosY_PHY = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
	}
	//	2007.07.22 genta バージョンを取得するために，
	//	正規表現の初期化を上へ移動

	/* 表示処理ON/OFF */
	// 2003.06.23 Moca 共通設定で変更できるように
//	m_bDrawSWITCH = FALSE;
	m_bDrawSWITCH = m_pShareData->m_Common.m_bGrepRealTimeView;


	if( -1 == DoGrepTree(
		&cDlgCancel, hwndCancel, szKey,
		pnKey_CharCharsArr,
//		pnKey_CharUsedArr,
		szFile, szPath, bGrepSubFolder, bGrepLoHiCase,
		bGrepRegularExp, nGrepCharSet,
		bGrepOutputLine, bWordOnly, nGrepOutputStyle, &cRegexp, 0, &nHitCount
	) ){
		wsprintf( szPath, "中断しました。\r\n", nHitCount );
		Command_ADDTAIL( szPath, lstrlen( szPath ) );
	}
	wsprintf( szPath, "%d 個が検索されました。\r\n", nHitCount );
	Command_ADDTAIL( szPath, lstrlen( szPath ) );
//	Command_GOFILEEND( FALSE );
#ifdef _DEBUG
	wsprintf( szPath, "処理時間: %dミリ秒\r\n", cRunningTimer.Read() );
	Command_ADDTAIL( szPath, lstrlen( szPath ) );
//	Command_GOFILEEND( FALSE );
#endif
	MoveCursor( 0, tmp_PosY_PHY, TRUE );	//	カーソルをGrep直前の位置に戻す

	cDlgCancel.CloseDialog( 0 );

	/* アクティブにする */
	hwndMainFrame = ::GetParent( m_hwndParent );
	/* アクティブにする */
	ActivateFrameWindow( hwndMainFrame );


	/* アンドゥバッファの処理 */
	if( NULL != m_pcOpeBlk ){
		if( 0 < m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
			/* 操作の追加 */
			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
		}else{
			delete m_pcOpeBlk;
		}
		m_pcOpeBlk = NULL;
	}

	//	Apr. 13, 2001 genta
	//	Grep実行後はファイルを変更無しの状態にする．
	m_pcEditDoc->SetModified(false,false);

	m_pcEditDoc->m_bGrepRunning = FALSE;
	m_bDoing_UndoRedo = FALSE;

	if( NULL != pnKey_CharCharsArr ){
		delete [] pnKey_CharCharsArr;
		pnKey_CharCharsArr = NULL;
	}
//	if( NULL != pnKey_CharUsedArr ){
//		delete [] pnKey_CharUsedArr;
//		pnKey_CharUsedArr = NULL;
//	}

	/* 表示処理ON/OFF */
	m_bDrawSWITCH = TRUE;

	/* フォーカス移動時の再描画 */
	RedrawAll();

	return nHitCount;
}

/*
 * SORTED_LIST_BSEARCH
 *   リストの探索にbsearchを使います。
 *   指定しない場合は、線形探索になります。
 * SORTED_LIST
 *   リストをqsortします。
 *
 * メモ：
 *   線形探索でもqsortを使い、文字列比較の大小関係が逆転したところで探索を
 *   打ち切れば少しは速いかもしれません。
 */
//#define SORTED_LIST
//#define SORTED_LIST_BSEARCH

#ifdef SORTED_LIST_BSEARCH
#define SORTED_LIST
#endif

#ifdef SORTED_LIST
typedef int (* COMP)(const void *, const void *);

/*!
	qsort用比較関数
	引数a,bは文字列へのポインタのポインタであることに注意。
	
	@param a [in] 比較文字列へのポインタのポインタ(list)
	@param b [in] 比較文字列へのポインタのポインタ(list)
	@return 比較結果
*/
int grep_compare_pp(const void* a, const void* b)
{
	return _tcscmp( *((const TCHAR**)a), *((const TCHAR**)b) );
}

/*!
	bsearch用比較関数
	引数bは文字列へのポインタのポインタであることに注意。
	
	@param a [in] 比較文字列へのポインタ(key)
	@param b [in] 比較文字列へのポインタのポインタ(list)
	@return 比較結果
*/
int grep_compare_sp(const void* a, const void* b)
{
	return _tcscmp( (const TCHAR*)a, *((const TCHAR**)b) );
}
#endif

/*! @brief Grep実行 

	@param pcDlgCancel		[in] Cancelダイアログへのポインタ
	@param hwndCancel		[in] Cancelダイアログのウィンドウハンドル
	@param pszKey			[in] 検索パターン
	@param pnKey_CharCharsArr	[in] 文字種配列(2byte/1byte)．単純文字列検索で使用．
	@param pszFile			[in] 検索対象ファイルパターン(!で除外指定)
	@param pszPath			[in] 検索対象パス
	@param bGrepSubFolder	[in] TRUE: サブフォルダを再帰的に探索する / FALSE: しない
	@param bGrepLoHiCase	[in] TRUE: 大文字小文字の区別あり / FALSE: 無し
	@param bGrepRegularExp	[in] TRUE: 検索パターンは正規表現 / FALSE: 文字列
	@param nGrepCharSet		[in] 文字コードセット (0:自動認識)～
	@param bGrepOutputLine	[in] TRUE: ヒット行を出力 / FALSE: ヒット部分を出力
	@param bWordOnly		[in] TRUE: 単語単位で一致を判断 / FALSE: 部分にも一致する
	@param nGrepOutputStyle	[in] 出力形式 1: Normal, 2: WZ風(ファイル単位)
	@param pRegexp			[in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	@param nNest			[in] ネストレベル
	@param pnHitCount		[i/o] ヒット数の合計
	
	@date 2003.06.23 Moca サブフォルダ→ファイルだったのをファイル→サブフォルダの順に変更
	@date 2003.06.23 Moca ファイル名から""を取り除くように
	@date 2003.03.27 みく 除外ファイル指定の導入と重複検索防止の追加．
		大部分が変更されたため，個別の変更点記入は無し．
*/
int CEditView::DoGrepTree(
	CDlgCancel* pcDlgCancel,
	HWND		hwndCancel,
	const char*	pszKey,
	int*		pnKey_CharCharsArr,
	const char*	pszFile,
	const char*	pszPath,
	BOOL		bGrepSubFolder,
	BOOL		bGrepLoHiCase,
	BOOL		bGrepRegularExp,
	int			nGrepCharSet,
	BOOL		bGrepOutputLine,
	BOOL		bWordOnly,
	int			nGrepOutputStyle,
	//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	CBregexp*	pRegexp,
	int			nNest,
	int*		pnHitCount
)
{
	::SetDlgItemText( hwndCancel, IDC_STATIC_CURPATH, pszPath );

	const TCHAR EXCEPT_CHAR = _T('!');	//除外識別子
	const TCHAR* WILDCARD_DELIMITER = _T(" ;,");	//リストの区切り
	const TCHAR* WILDCARD_ANY = _T("*.*");	//サブフォルダ探索用

	int		nWildCardLen;
	int		nPos;
	TCHAR*	token;
	BOOL	result;
	int		i;
	WIN32_FIND_DATA w32fd;
	CMemory			cmemMessage;
	int				nHitCountOld;
	char*			pszWork;
	int				nWork = 0;
	nHitCountOld = -100;

	//解放の対象
	TCHAR* pWildCard   = NULL;	//ワイルドカードリスト作業用
	TCHAR* currentPath = NULL;	//現在探索中のパス
	TCHAR* subPath     = NULL;
	HANDLE handle      = INVALID_HANDLE_VALUE;


	/*
	 * リストの初期化(文字列へのポインタをリスト管理する)
	 */
	int checked_list_size = 256;	//確保済みサイズ
	int checked_list_count = 0;	//登録個数
	TCHAR** checked_list = (TCHAR**)malloc( sizeof( TCHAR* ) * checked_list_size );
	if( ! checked_list ) return FALSE;	//メモリ確保失敗


	/*
	 * 除外ファイルを登録する。
	 */
	nPos = 0;
	pWildCard = _tcsdup( pszFile );
	if( ! pWildCard ) goto error_return;	//メモリ確保失敗
	nWildCardLen = _tcslen( pWildCard );
	while( NULL != (token = my_strtok( pWildCard, nWildCardLen, &nPos, WILDCARD_DELIMITER )) )	//トークン毎に繰り返す。
	{
		//除外ファイル指定でないか？
		if( EXCEPT_CHAR != token[0] ) continue;

		//ダブルコーテーションを除き、絶対パス名を作成する。
		TCHAR* p;
		TCHAR* q;
		p = q = ++token;
		while( *p )
		{
			if( *p != _T('"') ) *q++ = *p;
			p++;
		}
		*q = _T('\0');
		currentPath = new TCHAR[ _tcslen( pszPath ) + _tcslen( token ) + 1 ];
		if( ! currentPath ) goto error_return;	//メモリ確保失敗
		_tcscpy( currentPath, pszPath );
		_tcscat( currentPath, token );

		//ファイルの羅列を開始する。
		handle = FindFirstFile( currentPath, &w32fd );
		result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;
		while( result )
		{
			if( ! (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )	//フォルダでない場合
			{
				//チェック済みリストに登録する。
				if( checked_list_count >= checked_list_size )
				{
					checked_list_size += 256;
					TCHAR** p = (TCHAR**)realloc( checked_list, sizeof( TCHAR* ) * checked_list_size );
					if( ! p ) goto error_return;	//メモリ確保失敗
					checked_list = p;
				}
				checked_list[ checked_list_count ] = _tcsdup( w32fd.cFileName );
				checked_list_count++;
			}

			//次のファイルを羅列する。
			result = FindNextFile( handle, &w32fd );
		}
		//ハンドルを閉じる。
		if( INVALID_HANDLE_VALUE != handle )
		{
			FindClose( handle );
			handle = INVALID_HANDLE_VALUE;
		}
		delete [] currentPath;
		currentPath = NULL;
	}
	free( pWildCard );
	pWildCard = NULL;


	/*
	 * カレントフォルダのファイルを探索する。
	 */
	nPos = 0;
	pWildCard = _tcsdup( pszFile );
	if( ! pWildCard ) goto error_return;	//メモリ確保失敗
	nWildCardLen = _tcslen( pWildCard );
	while( NULL != (token = my_strtok( pWildCard, nWildCardLen, &nPos, WILDCARD_DELIMITER )) )	//トークン毎に繰り返す。
	{
		//除外ファイル指定か？
		if( EXCEPT_CHAR == token[0] ) continue;

		//ダブルコーテーションを除き、絶対パス名を作成する。
		TCHAR* p;
		TCHAR* q;
		p = q = token;
		while( *p )
		{
			if( *p != _T('"') ) *q++ = *p;
			p++;
		}
		*q = _T('\0');
		currentPath = new TCHAR[ _tcslen( pszPath ) + _tcslen( token ) + 1 ];
		if( ! currentPath ) goto error_return;
		_tcscpy( currentPath, pszPath );
		_tcscat( currentPath, token );

		//ファイルの羅列を開始する。
#ifdef SORTED_LIST
		//ソート
		qsort( checked_list, checked_list_count, sizeof( TCHAR* ), (COMP)grep_compare_pp );
#endif
		int current_checked_list_count = checked_list_count;	//前回までのリストの数
		handle = FindFirstFile( currentPath, &w32fd );
		result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;
		while( result )
		{
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
				goto cancel_return;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}
			/* 表示設定をチェック */
			m_bDrawSWITCH = ::IsDlgButtonChecked( pcDlgCancel->m_hWnd, IDC_CHECK_REALTIMEVIEW );

			if( ! (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )	//フォルダでない場合
			{
				/*
				 * リストにあるか調べる。
				 * 今回探索中のファイル同士が重複することはないので、
				 * 前回までのリスト(current_checked_list_count)から検索すればよい。
				 */
#ifdef SORTED_LIST_BSEARCH
				if( ! bsearch( w32fd.cFileName, checked_list, current_checked_list_count, sizeof( TCHAR* ), (COMP)grep_compare_sp ) )
#else
				bool found = false;
				TCHAR** ptr = checked_list;
				for( i = 0; i < current_checked_list_count; i++, ptr++ )
				{
#ifdef SORTED_LIST
					int n = _tcscmp( *ptr, w32fd.cFileName );
					if( 0 == n )
					{
						found = true; 
						break;
					}
					else if( n > 0 )	//探索打ち切り
					{
						break;
					}
#else
					if( 0 == _tcscmp( *ptr, w32fd.cFileName ) )
					{
						found = true; 
						break;
					}
#endif
				}
				if( ! found )
#endif
				{
					//チェック済みリストに登録する。
					if( checked_list_count >= checked_list_size )
					{
						checked_list_size += 256;
						TCHAR** p = (TCHAR**)realloc( checked_list, sizeof( TCHAR* ) * checked_list_size );
						if( ! p ) goto error_return;	//メモリ確保失敗
						checked_list = p;
					}
					checked_list[ checked_list_count ] = _tcsdup( w32fd.cFileName );
					checked_list_count++;


					//GREP実行！
					::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, w32fd.cFileName );

					TCHAR* currentFile = new TCHAR[ _tcslen( pszPath ) + _tcslen( w32fd.cFileName ) + 1 ];
					if( ! currentFile ) goto error_return;	//メモリ確保失敗
					_tcscpy( currentFile, pszPath );
					_tcscat( currentFile, w32fd.cFileName );
					/* ファイル内の検索 */
					int nRet = DoGrepFile(
						pcDlgCancel, hwndCancel, pszKey,
						pnKey_CharCharsArr,
						w32fd.cFileName,
						bGrepLoHiCase,
						bGrepRegularExp, nGrepCharSet,
						bGrepOutputLine, bWordOnly, nGrepOutputStyle,
						pRegexp, pnHitCount, currentFile, cmemMessage
					);
					delete currentFile;
					currentFile = NULL;

					// 2003.06.23 Moca リアルタイム表示のときは早めに表示
					if( m_bDrawSWITCH ){
						if( _T('\0') != pszKey[0] ){
							// データ検索のときファイルの合計が最大10MBを超えたら表示
							nWork += ( w32fd.nFileSizeLow + 1023 ) / 1024;
						}
						if( *pnHitCount - nHitCountOld && 
							( *pnHitCount < 20 || 10000 < nWork ) ){
							nHitCountOld = -100; // 即表示
						}
					}
					if( *pnHitCount - nHitCountOld  >= 10 ){
						/* 結果出力 */
						pszWork = cmemMessage.GetPtr( &nWork );
						if( 0 < nWork ){
							Command_ADDTAIL( pszWork, nWork );
							Command_GOFILEEND( FALSE );
							/* 結果格納エリアをクリア */
							cmemMessage.SetDataSz( _T("") );
							nWork = 0;
						}
						nHitCountOld = *pnHitCount;
					}
					if( -1 == nRet ){
						goto cancel_return;
					}
				}
			}

			//次のファイルを羅列する。
			result = FindNextFile( handle, &w32fd );
		}
		//ハンドルを閉じる。
		if( INVALID_HANDLE_VALUE != handle )
		{
			FindClose( handle );
			handle = INVALID_HANDLE_VALUE;
		}
		delete [] currentPath;
		currentPath = NULL;
	}
	free( pWildCard );
	pWildCard = NULL;

	for( i = 0; i < checked_list_count; i++ )
	{
		free( checked_list[ i ] );
	}
	free( checked_list );
	checked_list = NULL;
	checked_list_count = 0;
	checked_list_size = 0;


	/*
	 * サブフォルダを検索する。
	 */
	if( bGrepSubFolder ){
		subPath = new TCHAR[ _tcslen( pszPath ) + _tcslen( WILDCARD_ANY ) + 1 ];
		if( ! subPath ) goto error_return;	//メモリ確保失敗
		_tcscpy( subPath, pszPath );
		_tcscat( subPath, WILDCARD_ANY );
		handle = FindFirstFile( subPath, &w32fd );
		result = (INVALID_HANDLE_VALUE != handle) ? TRUE : FALSE;
		while( result )
		{
			//サブフォルダの探索を再帰呼び出し。
			/* 処理中のユーザー操作を可能にする */
			if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
				goto cancel_return;
			}
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}
			/* 表示設定をチェック */
			m_bDrawSWITCH = ::IsDlgButtonChecked( pcDlgCancel->m_hWnd, IDC_CHECK_REALTIMEVIEW );

			if( (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)	//フォルダの場合
			 && 0 != _tcscmp( w32fd.cFileName, _T("."))
			 && 0 != _tcscmp( w32fd.cFileName, _T("..")) )
			{
				//フォルダ名を作成する。
				currentPath = new TCHAR[ _tcslen( pszPath ) + _tcslen( w32fd.cFileName ) + 2 ];
				if( ! currentPath ) goto error_return;	//メモリ確保失敗
				_tcscpy( currentPath, pszPath );
				_tcscat( currentPath, w32fd.cFileName );
				_tcscat( currentPath, _T("\\") );

				if( -1 == DoGrepTree(
					pcDlgCancel, hwndCancel,
					pszKey,
					pnKey_CharCharsArr,
					pszFile, currentPath,
					bGrepSubFolder, bGrepLoHiCase,
					bGrepRegularExp, nGrepCharSet,
					bGrepOutputLine, bWordOnly, nGrepOutputStyle, pRegexp, nNest + 1, pnHitCount
				) ){
					goto cancel_return;
				}
				::SetDlgItemText( hwndCancel, IDC_STATIC_CURPATH, pszPath );	//@@@ 2002.01.10 add サブフォルダから戻ってきたら...

				delete [] currentPath;
				currentPath = NULL;
			}

			//次のファイルを羅列する。
			result = FindNextFile( handle, &w32fd );
		}
		//ハンドルを閉じる。
		if( INVALID_HANDLE_VALUE != handle )
		{
			FindClose( handle );
			handle = INVALID_HANDLE_VALUE;
		}
		delete [] subPath;
		subPath = NULL;
	}

	::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, _T(" ") );	// 2002/09/09 Moca add
	/* 結果出力 */
	pszWork = cmemMessage.GetPtr( &nWork );
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
		Command_GOFILEEND( FALSE );
		/* 結果格納エリアをクリア */
		cmemMessage.SetDataSz( _T("") );
	}

	return 0;


cancel_return:;
error_return:;
	/*
	 * エラー時はすべての確保済みリソースを解放する。
	 */
	if( INVALID_HANDLE_VALUE != handle ) FindClose( handle );

	if( pWildCard ) free( pWildCard );
	if( currentPath ) delete [] currentPath;
	if( subPath ) delete [] subPath;

	if( checked_list )
	{
		for( i = 0; i < checked_list_count; i++ )
		{
			free( checked_list[ i ] );
		}
		free( checked_list );
	}

	/* 結果出力 */
	pszWork = cmemMessage.GetPtr( &nWork );
	if( 0 < nWork )
	{
		Command_ADDTAIL( pszWork, nWork );
		Command_GOFILEEND( FALSE );
		/* 結果格納エリアをクリア */
		cmemMessage.SetDataSz( _T("") );
	}

	return -1;
}



/*!	@brief Grep結果を構築する

	@param pWork [out] Grep出力文字列．充分なメモリ領域を予め確保しておくこと．
		最長で 本文2000 byte＋ファイル名 _MAX_PATH byte＋行・桁位置表示の長さが必要．
		ファイル単位出力の場合は本文2500 byte + _MAX_PATH + 行・桁位置表示の長さが必要．
		

	pWorkは充分なメモリ領域を持っているコト
	@date 2002/08/29 Moca バイナリーデータに対応 pnWorkLen 追加
*/
void CEditView::SetGrepResult(
	/* データ格納先 */
	char*		pWork,
	int*		pnWorkLen,			/*!< [out] Grep出力文字列の長さ */
	/* マッチしたファイルの情報 */
	const char*		pszFullPath,	/*!< [in] フルパス */
	const char*		pszCodeName,	/*!< [in] 文字コード情報．" [SJIS]"とか */
	/* マッチした行の情報 */
	int			nLine,				/*!< [in] マッチした行番号(1～) */
	int			nColm,				/*!< [in] マッチした桁番号(1～) */
	const char*		pCompareData,	/*!< [in] 行の文字列 */
	int			nLineLen,			/*!< [in] 行の文字列の長さ */
	int			nEolCodeLen,		/*!< [in] EOLの長さ */
	/* マッチした文字列の情報 */
	const char*		pMatchData,		/*!< [in] マッチした文字列 */
	int			nMatchLen,			/*!< [in] マッチした文字列の長さ */
	/* オプション */
	BOOL		bGrepOutputLine,	/*!< [in] 0: 該当部分のみ, !0: 該当行 */
	int			nGrepOutputStyle	/*!< [in] 1: Normal, 2: WZ風(ファイル単位) */
)
{

	int nWorkLen = 0;
	const char * pDispData;
	int k;
	bool bEOL = true;
	int nMaxOutStr;

	if( 1 == nGrepOutputStyle ){
	/* ノーマル */
		nWorkLen = ::wsprintf( pWork, "%s(%d,%d)%s: ", pszFullPath, nLine, nColm, pszCodeName );
		nMaxOutStr = 2000; // 2003.06.10 Moca 最大長変更
	}else
	if( 2 == nGrepOutputStyle ){
	/* WZ風 */
		nWorkLen = ::wsprintf( pWork, "・(%6d,%-5d): ", nLine, nColm );
		nMaxOutStr = 2500; // 2003.06.10 Moca 最大長変更
	}

	if( bGrepOutputLine ){
	/* 該当行 */
		pDispData = pCompareData;
		k = nLineLen - nEolCodeLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
	}else{
	/* 該当部分 */
		pDispData = pMatchData;
		k = nMatchLen;
		if( nMaxOutStr < k ){
			k = nMaxOutStr; // 2003.06.10 Moca 最大長変更
		}
		// 該当部分に改行を含む場合はその改行コードをそのまま利用する(次の行に空行を作らない)
		// 2003.06.10 Moca k==0のときにバッファアンダーランしないように
		if( 0 < k && (pMatchData[ k - 1 ] == '\r' || pMatchData[ k - 1 ] == '\n') ){
			bEOL = false;
		}
	}

	memcpy( &pWork[nWorkLen], pDispData, k );
	nWorkLen += k;
	if( bEOL ){
		memcpy( &pWork[nWorkLen], "\r\n", 2 );
		nWorkLen = nWorkLen + 2;
	}
	*pnWorkLen = nWorkLen;
}

/*!
	Grep実行 (CFileLoadを使ったテスト版)

	@param pcDlgCancel		[in] Cancelダイアログへのポインタ
	@param hwndCancel		[in] Cancelダイアログのウィンドウハンドル
	@param pszKey			[in] 検索パターン
	@param pnKey_CharCharsArr	[in] 文字種配列(2byte/1byte)．単純文字列検索で使用．
	@param pszFile			[in] 処理対象ファイル名(表示用)
	@param bGrepLoHiCase	[in] TRUE: 大文字小文字の区別あり / FALSE: 無し
	@param bGrepRegularExp	[in] TRUE: 検索パターンは正規表現 / FALSE: 文字列
	@param nGrepCharSet		[in] 文字コードセット (0:自動認識)～
	@param bGrepOutputLine	[in] TRUE: ヒット行を出力 / FALSE: ヒット部分を出力
	@param bWordOnly		[in] TRUE: 単語単位で一致を判断 / FALSE: 部分にも一致する
	@param nGrepOutputStyle	[in] 出力形式 1: Normal, 2: WZ風(ファイル単位)
	@param pRegexp			[in] 正規表現コンパイルデータ。既にコンパイルされている必要がある
	@param pnHitCount		[i/o] ヒット数の合計．元々の値に見つかった数を加算して返す．
	@param pszFullPath		[in] 処理対象ファイルパス

	@retval -1 GREPのキャンセル
	@retval それ以外 ヒット数(ファイル検索時はファイル数)

	@date 2002/08/30 Moca CFileLoadを使ったテスト版
	@date 2004/03/28 genta 不要な引数nNest, bGrepSubFolder, pszPathを削除
*/
int CEditView::DoGrepFile(
	CDlgCancel* pcDlgCancel,
	HWND		hwndCancel,
	const char*	pszKey,
	int*		pnKey_CharCharsArr,
//	int*		pnKey_CharUsedArr,
	const char*	pszFile,
	BOOL		bGrepLoHiCase,
	BOOL		bGrepRegularExp,
	int			nGrepCharSet,
	BOOL		bGrepOutputLine,
	BOOL		bWordOnly,
	int			nGrepOutputStyle,
	//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
	CBregexp*	pRegexp,
	int*		pnHitCount,
	const char*	pszFullPath,
	CMemory&	cmemMessage
)
{
	int		nHitCount;
//	char	szLine[16000];
	char	szWork[3000]; // ここは SetGrepResult() が返す文字列を格納できるサイズが必要
	char	szWork0[_MAX_PATH + 100];
	int		nLine;
	int		nWorkLen;
	const char*	pszRes; // 2002/08/29 const付加
	int		nCharCode;
	const char*	pCompareData; // 2002/08/29 const付加
	const char*	pszCodeName; // 2002/08/29 const付加
	int		nColm;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
	int		nLineLen;
	const	char*	pLine;
	CEOL	cEol;
	int		nEolCodeLen;
	CFileLoad	cfl;
	int		nOldPercent = 0;

	int	nKeyKen = lstrlen( pszKey );

	//	ここでは正規表現コンパイルデータの初期化は不要

	pszCodeName = "";
	nHitCount = 0;
	nLine = 0;

	/* 検索条件が長さゼロの場合はファイル名だけ返す */
	// 2002/08/29 行ループの前からここに移動
	if( 0 == nKeyKen ){
		if( CODE_AUTODETECT == nGrepCharSet ){
			// 2003.06.10 Moca コード判別処理をここに移動．
			// 判別エラーでもファイル数にカウントするため
			// ファイルの日本語コードセット判別
			nCharCode = CMemory::CheckKanjiCodeOfFile( pszFullPath );
			if( -1 == nCharCode ){
				pszCodeName = "  [(DetectError)]";
			}else{
				pszCodeName = gm_pszCodeNameArr_3[nCharCode];
			}
		}
		if( 1 == nGrepOutputStyle ){
		/* ノーマル */
			wsprintf( szWork0, "%s%s\r\n", pszFullPath, pszCodeName );
		}else{
		/* WZ風 */
			wsprintf( szWork0, "■\"%s\"%s\r\n", pszFullPath, pszCodeName );
		}
		cmemMessage.AppendSz( szWork0 );
		++(*pnHitCount);
		::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
		return 1;
	}


	try{
	// ファイルを開く
	// FileCloseで明示的に閉じるが、閉じていないときはデストラクタで閉じる
	// 2003.06.10 Moca 文字コード判定処理もFileOpenで行う
	nCharCode = cfl.FileOpen( pszFullPath, nGrepCharSet, 0 );
	if( CODE_AUTODETECT == nGrepCharSet ){
		pszCodeName = gm_pszCodeNameArr_3[nCharCode];
	}
	wsprintf( szWork0, "■\"%s\"%s\r\n", pszFullPath, pszCodeName );
//	/* 処理中のユーザー操作を可能にする */
	if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
		return -1;
	}
	/* 中断ボタン押下チェック */
	if( pcDlgCancel->IsCanceled() ){
		return -1;
	}

	/* 検索条件が長さゼロの場合はファイル名だけ返す */
	// 2002/08/29 ファイルオープンの手前へ移動

	// 注意 : cfl.ReadLine が throw する可能性がある
	while( NULL != ( pLine = cfl.ReadLine( &nLineLen, &cEol ) ) ){
		nEolCodeLen = cEol.GetLen();
		++nLine;
		pCompareData = pLine;

		/* 処理中のユーザー操作を可能にする */
		if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
			return -1;
		}
		if( 0 == nLine % 64 ){
			/* 中断ボタン押下チェック */
			if( pcDlgCancel->IsCanceled() ){
				return -1;
			}
			//	2003.06.23 Moca 表示設定をチェック
			m_bDrawSWITCH = ::IsDlgButtonChecked( pcDlgCancel->m_hWnd, IDC_CHECK_REALTIMEVIEW );
			// 2002/08/30 Moca 進行状態を表示する(5MB以上)
			if( 5000000 < cfl.GetFileSize() ){
				int nPercent = cfl.GetPercent();
				if( 5 <= nPercent - nOldPercent ){
					nOldPercent = nPercent;
					::wsprintf( szWork, "%s (%3d%%)", pszFile, nPercent );
					::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, szWork );
				}
			}
		}

		/* 正規表現検索 */
		if( bGrepRegularExp ){
			int nColmPrev = 0;

			//	Jun. 21, 2003 genta ループ条件見直し
			//	マッチ箇所を1行から複数検出するケースを標準に，
			//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
			//	ループ継続・打ち切り条件(bGrepOutputLine)を逆にした．
			//	Jun. 27, 2001 genta	正規表現ライブラリの差し替え
			// From Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
			while( pRegexp->Match( pCompareData, nLineLen, 0 ) ){

					//	パターン発見
					nColm = pRegexp->GetIndex() + 1;
					int matchlen = pRegexp->GetMatchLen();

					/* Grep結果を、szWorkに格納する */
					SetGrepResult(
						szWork, &nWorkLen,
						pszFullPath, pszCodeName,
						nLine, nColm + nColmPrev, pCompareData, nLineLen, nEolCodeLen,
						pCompareData + nColm - 1, matchlen,
						bGrepOutputLine, nGrepOutputStyle
					);
					// To Here 2005.03.19 かろと もはやBREGEXP構造体に直接アクセスしない
					if( 2 == nGrepOutputStyle ){
					/* WZ風 */
						if( !bOutFileName ){
							cmemMessage.AppendSz( szWork0 );
							bOutFileName = TRUE;
						}
					}
					cmemMessage.Append( szWork, nWorkLen );
					++nHitCount;
					++(*pnHitCount);
					if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
						::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					//	Jun. 21, 2003 genta 行単位で出力する場合は1つ見つかれば十分
					if ( bGrepOutputLine ) {
						break;
					}
					//	探し始める位置を補正
					//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
					//	nClom : マッチ位置
					//	matchlen : マッチした文字列の長さ
					int nPosDiff = nColm + matchlen;
					if( matchlen != 0 ){
						nPosDiff--;
					}
					pCompareData += nPosDiff;
					nLineLen -= nPosDiff;
					nColmPrev += nPosDiff;
			}
		}else
		/* 単語のみ検索 */
		if( bWordOnly ){
			/*
				2002/02/23 Norio Nakatani
				単語単位のGrepを試験的に実装。単語はWhereCurrentWord()で判別してますので、
				英単語やC/C++識別子などの検索条件ならヒットします。

				2002/03/06 YAZAKI
				Grepにも試験導入。
				WhereCurrentWordで単語を抽出して、その単語が検索語とあっているか比較する。
			*/
			int nNextWordFrom = 0;
			int nNextWordFrom2;
			int nNextWordTo2;
			// Jun. 26, 2003 genta 無駄なwhileは削除
			while( TRUE ==
					CDocLineMgr::WhereCurrentWord_2( pCompareData, nLineLen, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )
				){
					if( nKeyKen == nNextWordTo2 - nNextWordFrom2 ){
						// const char* pData = pCompareData;	// 2002/2/10 aroka CMemory変更 , 2002/08/29 Moca pCompareDataのconst化により不要?
						/* 1==大文字小文字の区別 */
						if( (FALSE == bGrepLoHiCase && 0 == _memicmp( &(pCompareData[nNextWordFrom2]) , pszKey, nKeyKen ) ) ||
							(TRUE  == bGrepLoHiCase && 0 ==	  memcmp( &(pCompareData[nNextWordFrom2]) , pszKey, nKeyKen ) )
						){
							/* Grep結果を、szWorkに格納する */
							SetGrepResult(
								szWork, &nWorkLen,
								pszFullPath, pszCodeName,
								//	Jun. 25, 2002 genta
								//	桁位置は1始まりなので1を足す必要がある
								nLine, nNextWordFrom2 + 1, pCompareData, nLineLen, nEolCodeLen,
								&(pCompareData[nNextWordFrom2]), nKeyKen,
								bGrepOutputLine, nGrepOutputStyle
							);
							if( 2 == nGrepOutputStyle ){
							/* WZ風 */
								if( !bOutFileName ){
									cmemMessage.AppendSz( szWork0 );
									bOutFileName = TRUE;
								}
							}

							cmemMessage.Append( szWork, nWorkLen );
							++nHitCount;
							++(*pnHitCount);
							//	May 22, 2000 genta
							// if( 0 == ( (*pnHitCount) % 16 ) ){
								::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
							// }
						}
					}
					/* 現在位置の左右の単語の先頭位置を調べる */
					if( !CDocLineMgr::SearchNextWordPosition( pCompareData, nLineLen, nNextWordFrom, &nNextWordFrom, FALSE ) ){
						break;	//	次の単語が無い。
					}
				}
		}
		else {
			/* 文字列検索 */
			int nColmPrev = 0;
			//	Jun. 21, 2003 genta ループ条件見直し
			//	マッチ箇所を1行から複数検出するケースを標準に，
			//	マッチ箇所を1行から1つだけ検出する場合を例外ケースととらえ，
			//	ループ継続・打ち切り条件(bGrepOutputLine)を逆にした．
			while( NULL != ( pszRes = CDocLineMgr::SearchString(
					(const unsigned char *)pCompareData, nLineLen,
					0,
					(const unsigned char *)pszKey, nKeyKen,
					pnKey_CharCharsArr,
					bGrepLoHiCase
				) ) ){
					nColm = pszRes - pCompareData + 1;

					/* Grep結果を、szWorkに格納する */
					SetGrepResult(
						szWork, &nWorkLen,
						pszFullPath, pszCodeName,
						nLine, nColm + nColmPrev, pCompareData, nLineLen, nEolCodeLen,
						pszRes, nKeyKen,
						bGrepOutputLine, nGrepOutputStyle
					);
					if( 2 == nGrepOutputStyle ){
					/* WZ風 */
						if( !bOutFileName ){
							cmemMessage.AppendSz( szWork0 );
							bOutFileName = TRUE;
						}
					}

					cmemMessage.Append( szWork, nWorkLen );
					++nHitCount;
					++(*pnHitCount);
					//	May 22, 2000 genta
					if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
						::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					
					//	Jun. 21, 2003 genta 行単位で出力する場合は1つ見つかれば十分
					if ( bGrepOutputLine ) {
						break;
					}
					//	探し始める位置を補正
					//	2003.06.10 Moca マッチした文字列の後ろから次の検索を開始する
					//	nClom : マッチ位置
					//	matchlen : マッチした文字列の長さ
					int nPosDiff = nColm += nKeyKen - 1;
					pCompareData += nPosDiff;
					nLineLen -= nPosDiff;
					nColmPrev += nPosDiff;
			}
		}
	}

	// ファイルを明示的に閉じるが、ここで閉じないときはデストラクタで閉じている
	cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		wsprintf( szWork, "file open error [%s]\r\n", pszFullPath );
		Command_ADDTAIL( szWork, lstrlen( szWork ) );
		return 0;
	}
	catch( CError_FileRead ){
		wsprintf( szWork, "CEditView::DoGrepFile() ファイルの読み込み中にエラーが発生しました。\r\n");
		Command_ADDTAIL( szWork, lstrlen( szWork ) );
	} // 例外処理終わり

	return nHitCount;
}


/*
	カーソル直前の単語を取得 単語の長さを返します
	単語区切り
*/
int CEditView::GetLeftWord( CMemory* pcmemWord, int nMaxWordLen )
{
	const char*	pLine;
	int			nLineLen;
	int			nIdx;
	int			nIdxTo;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	CMemory		cmemWord;
	int			nCurLine;
	int			nCharChars;
	const CLayout* pcLayout;

	nCurLine = m_nCaretPosY;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen, &pcLayout );
	if( NULL == pLine ){
//		return 0;
		nIdxTo = 0;
	}else{
		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */
		nIdxTo = LineColmnToIndex( pcLayout, m_nCaretPosX );
	}
	if( 0 == nIdxTo || NULL == pLine ){
		if( nCurLine <= 0 ){
			return 0;
		}
		nCurLine--;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( pLine[nLineLen - 1] == '\r' || pLine[nLineLen - 1] == '\n' ){
			return 0;
		}
		/* 指定された桁に対応する行のデータ内の位置を調べる Ver1 */
//		nIdxTo = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );

		nCharChars = &pLine[nLineLen] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nLineLen] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdxTo = nLineLen;
		nIdx = nIdxTo - nCharChars;

//		nIdx = nIdxTo = nLineLen - 1;
	}else{
		nCharChars = &pLine[nIdxTo] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxTo] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdx = nIdxTo - nCharChars;
	}
	if( 1 == nCharChars ){
		if( pLine[nIdx] == SPACE || pLine[nIdx] == TAB ){
			return 0;
		}
	}
	if( 2 == nCharChars ){
		if( (unsigned char)pLine[nIdx	 ] == (unsigned char)0x81 &&
			(unsigned char)pLine[nIdx + 1] == (unsigned char)0x40
		){
			return 0;
		}
	}


//	nIdx = nIdxTo - 1;
//	if( 0 > nIdx ){
//		return 0;
//	}
//	if( pLine[nIdx] == SPACE ||
//		pLine[nIdx] == TAB ||
//		(
//			nIdx > 0 &&
//			pLine[nIdx] == 0x40 &&
//			(unsigned char)pLine[nIdx - 1] == (unsigned char)0x81
//		)
//	){
//		return 0;
//	}
//	while( 0 <= nIdx && ( pLine[nIdx] != SPACE && pLine[nIdx] != TAB ) ){
//		nIdx--;
//	}
//	++nIdx;
//	if( nIdxTo - nIdx > 0 ){
//		if( nMaxWordLen < nIdxTo - nIdx ){
//			return 0;
//		}else{
//			pcmemWord->SetData( &pLine[nIdx], nIdxTo - nIdx );
//			return nIdxTo - nIdx;
//		}
//	}else{
//		return 0;
//	}



	/* 現在位置の単語の範囲を調べる */
	if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		nCurLine, nIdx,
		&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, &cmemWord, pcmemWord )
	){
		pcmemWord->Append( &pLine[nIdx], nCharChars );
//		MYTRACE( "==========\n" );
//		MYTRACE( "cmemWord=[%s]\n", cmemWord.GetPtr() );
//		MYTRACE( "pcmemWord=[%s]\n", pcmemWord->GetPtr() );

		return pcmemWord->GetLength();
	}else{
		return 0;
	}
}
/*!
	キャレット位置の単語を取得
	単語区切り

	@param[out] pcmemWord キャレット位置の単語
	@return true: 成功，false: 失敗
	
	@date 2006.03.24 fon (CEditView::Command_SELECTWORDを流用)
*/
BOOL CEditView::GetCurrentWord(
		CMemory* pcmemWord
)
{
	int				nLineFrom;
	int				nColmFrom;
	int				nLineTo;
	int				nColmTo;
	int				nIdx;
	const CLayout*	pcLayout = m_pcEditDoc->m_cLayoutMgr.Search( m_nCaretPosY );
	if( NULL == pcLayout ){
		return false;	/* 単語選択に失敗 */
	}
	/* 指定された桁に対応する行のデータ内の位置を調べる */
	nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );

	/* 現在位置の単語の範囲を調べる */
	if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		m_nCaretPosY, nIdx,
		&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, pcmemWord, NULL ) ){
		return true;	/* 単語選択に成功 */
	}
	else {
		return false;	/* 単語選択に失敗 */
	}
}


/* 指定カーソル位置が選択エリア内にあるか
	【戻り値】
	-1	選択エリアより前方 or 無選択
	0	選択エリア内
	1	選択エリアより後方
*/
int CEditView::IsCurrentPositionSelected(
	int		nCaretPosX,		// カーソル位置X
	int		nCaretPosY		// カーソル位置Y
)
{
	if( !IsTextSelected() ){	/* テキストが選択されているか */
		return -1;
	}
	RECT	rcSel;
	POINT	po;


	/* 矩形範囲選択中か */
	if( m_bBeginBoxSelect ){
		/* 2点を対角とする矩形を求める */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,		/* 範囲選択開始行 */
			m_nSelectColmFrom,		/* 範囲選択開始桁 */
			m_nSelectLineTo,		/* 範囲選択終了行 */
			m_nSelectColmTo			/* 範囲選択終了桁 */
		);
		++rcSel.bottom;
		po.x = nCaretPosX;
		po.y = nCaretPosY;
		if( m_bDragSource ){
			if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){ /* Ctrlキーが押されていたか */
				++rcSel.left;
			}else{
				++rcSel.right;
			}
		}
		if( PtInRect( &rcSel, po ) ){
			return 0;
		}
		if( rcSel.top > nCaretPosY ){
			return -1;
		}
		if( rcSel.bottom < nCaretPosY ){
			return 1;
		}
		if( rcSel.left > nCaretPosX ){
			return -1;
		}
		if( rcSel.right < nCaretPosX ){
			return 1;
		}
	}else{
		if( m_nSelectLineFrom > nCaretPosY ){
			return -1;
		}
		if( m_nSelectLineTo < nCaretPosY ){
			return 1;
		}
		if( m_nSelectLineFrom == nCaretPosY ){
			if( m_bDragSource ){
				if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){	/* Ctrlキーが押されていたか */
					if( m_nSelectColmFrom >= nCaretPosX ){
						return -1;
					}
				}else{
					if( m_nSelectColmFrom > nCaretPosX ){
						return -1;
					}
				}
			}else
			if( m_nSelectColmFrom > nCaretPosX ){
				return -1;
			}
		}
		if( m_nSelectLineTo == nCaretPosY ){
			if( m_bDragSource ){
				if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){	/* Ctrlキーが押されていたか */
					if( m_nSelectColmTo <= nCaretPosX ){
						return 1;
					}
				}else{
					if( m_nSelectColmTo < nCaretPosX ){
						return 1;
					}
				}
			}else
			if( m_nSelectColmTo <= nCaretPosX ){
				return 1;
			}
		}
		return 0;
	}
	return -1;
}

/* 指定カーソル位置が選択エリア内にあるか (テスト)
	【戻り値】
	-1	選択エリアより前方 or 無選択
	0	選択エリア内
	1	選択エリアより後方
*/
int CEditView::IsCurrentPositionSelectedTEST(
	int		nCaretPosX,		// カーソル位置X
	int		nCaretPosY,		// カーソル位置Y
	int		nSelectLineFrom,
	int		nSelectColmFrom,
	int		nSelectLineTo,
	int		nSelectColmTo
)
{
	if( !IsTextSelected() ){	/* テキストが選択されているか */
		return -1;
	}
//	RECT	rcSel;
//	POINT	po;


	if( nSelectLineFrom > nCaretPosY ){
		return -1;
	}
	if( nSelectLineTo < nCaretPosY ){
		return 1;
	}
	if( nSelectLineFrom == nCaretPosY ){
		if( nSelectColmFrom > nCaretPosX ){
			return -1;
		}
	}
	if( nSelectLineTo == nCaretPosY ){
		if( nSelectColmTo <= nCaretPosX ){
			return 1;
		}
	}
	return 0;
}

/*! クリップボードからデータを取得
	@date 2005/05/29 novice UNICODE TEXT 対応処理を追加
	@date 2007.10.04 ryoji MSDEVLineSelect対応処理を追加
*/
BOOL CEditView::MyGetClipboardData( CMemory& cmemBuf, BOOL* pbColmnSelect, BOOL* pbLineSelect /*= NULL*/ )
{
	HGLOBAL		hglb;
	char*		lptstr;

	if( NULL != pbColmnSelect ){
		*pbColmnSelect = FALSE;
	}
	if( NULL != pbLineSelect ){
		*pbLineSelect = FALSE;
	}


	UINT uFormatSakuraClip;
	UINT uFormat;
	uFormatSakuraClip = ::RegisterClipboardFormat( "SAKURAClip" );
	if( !::IsClipboardFormatAvailable( CF_OEMTEXT )
	 && !::IsClipboardFormatAvailable( uFormatSakuraClip )
	){
		return FALSE;
	}
	if ( !::OpenClipboard( m_hWnd ) ){
		return FALSE;
	}

	char	szFormatName[128];

	if( NULL != pbColmnSelect || NULL != pbColmnSelect ){
		/* 矩形選択や行選択のテキストデータがクリップボードにあるか */
		uFormat = 0;
		while( 0 != ( uFormat = ::EnumClipboardFormats( uFormat ) ) ){
			// Jul. 2, 2005 genta : check return value of GetClipboardFormatName
			if( ::GetClipboardFormatName( uFormat, szFormatName, sizeof(szFormatName) - 1 ) ){
				if( NULL != pbColmnSelect && 0 == lstrcmp( "MSDEVColumnSelect", szFormatName ) ){
					*pbColmnSelect = TRUE;
					break;
				}
				if( NULL != pbLineSelect && 0 == lstrcmp( "MSDEVLineSelect", szFormatName ) ){
					*pbLineSelect = TRUE;
					break;
				}
			}
		}
	}
	if( ::IsClipboardFormatAvailable( uFormatSakuraClip ) ){
		hglb = ::GetClipboardData( uFormatSakuraClip );
		if (hglb != NULL) {
			lptstr = (char*)::GlobalLock(hglb);
			cmemBuf.SetData( lptstr + sizeof(int), *((int*)lptstr) );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return TRUE;
		}
	}else{
		// From Here 2005/05/29 novice UNICODE TEXT 対応処理を追加
		hglb = ::GetClipboardData( CF_UNICODETEXT );
		if( hglb != NULL ){
			lptstr = (char*)::GlobalLock(hglb);
			//	UnicodeToSJISでは後ろに余計な空白が入るので，
			//	一時変数を介した後に\0までを取り出す．
			CMemory cmemUnicode( lptstr, GlobalSize(lptstr) );
			cmemUnicode.UnicodeToSJIS();
			cmemBuf.SetDataSz( cmemUnicode.GetPtr() );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return TRUE;
		}
		//	To Here 2005/05/29 novice

		hglb = ::GetClipboardData( CF_OEMTEXT );
		if( hglb != NULL ){
			lptstr = (char*)::GlobalLock(hglb);
			cmemBuf.SetDataSz( lptstr );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return TRUE;
		}
	}
	::CloseClipboard();
	return FALSE;
}

/* クリップボードにデータを設定
	@date 2004.02.17 Moca エラーチェックするように
	@date 2007.10.04 ryoji MSDEVLineSelect対応処理を追加
 */
BOOL CEditView::MySetClipboardData( const char* pszText, int nTextLen, BOOL bColmnSelect, BOOL bLineSelect /*= FALSE*/ )
{
	HGLOBAL		hgClipText;
	HGLOBAL		hgClipSakura;
	HGLOBAL		hgClipMSDEVColm;
	HGLOBAL		hgClipMSDEVLine;

	char*		pszClip;
	UINT		uFormat;
	/* Windowsクリップボードにコピー */
	if( FALSE == ::OpenClipboard( m_hWnd ) ){
		return FALSE;
	}
	::EmptyClipboard();
	// ヌル終端までの長さ
	int nNullTerminateLen = lstrlen( pszText );

	/* テキスト形式のデータ */
	hgClipText = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		nNullTerminateLen + 1
	);
	if( hgClipText ){
		pszClip = (char*)::GlobalLock( hgClipText );
		memcpy( pszClip, pszText, nNullTerminateLen );
		pszClip[nNullTerminateLen] = '\0';
		::GlobalUnlock( hgClipText );
		::SetClipboardData( CF_OEMTEXT, hgClipText );
	}

	/* バイナリ形式のデータ
		(int) 「データ」の長さ
		「データ」
	*/
	UINT	uFormatSakuraClip;
	uFormatSakuraClip = ::RegisterClipboardFormat( _T("SAKURAClip") );
	if( 0 != uFormatSakuraClip ){
		hgClipSakura = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			nTextLen + sizeof( int ) + 1
		);
		if( hgClipSakura ){
			pszClip = (char*)::GlobalLock( hgClipSakura );
			*((int*)pszClip) = nTextLen;
			memcpy( pszClip + sizeof( int ), pszText, nTextLen );
			::GlobalUnlock( hgClipSakura );
			::SetClipboardData( uFormatSakuraClip, hgClipSakura );
		}
	}


	/* 矩形選択を示すダミーデータ */
	if( bColmnSelect ){
		uFormat = ::RegisterClipboardFormat( _T("MSDEVColumnSelect") );
		if( 0 != uFormat ){
			hgClipMSDEVColm = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVColm ){
				pszClip = (char*)::GlobalLock( hgClipMSDEVColm );
				pszClip[0] = '\0';
				::GlobalUnlock( hgClipMSDEVColm );
				::SetClipboardData( uFormat, hgClipMSDEVColm );
			}
		}
	}

	/* 行選択を示すダミーデータ */
	if( bLineSelect ){
		uFormat = ::RegisterClipboardFormat( _T("MSDEVLineSelect") );
		if( 0 != uFormat ){
			hgClipMSDEVLine = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			if( hgClipMSDEVLine ){
				pszClip = (char*)::GlobalLock( hgClipMSDEVLine );
				pszClip[0] = (char)0x01;
				::GlobalUnlock( hgClipMSDEVLine );
				::SetClipboardData( uFormat, hgClipMSDEVLine );
			}
		}
	}
	::CloseClipboard();

	if( bColmnSelect && !hgClipMSDEVColm ){
		return FALSE;
	}
	if( bLineSelect && !hgClipMSDEVLine ){
		return FALSE;
	}
	
	if( !hgClipText && !hgClipSakura ){
		return FALSE;
	}
	return TRUE;
}






STDMETHODIMP CEditView::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::DragEnter()\n" );
#endif

	if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop	/* OLEによるドラッグ & ドロップを使う */
		//	Oct. 22, 2005 genta 上書き禁止(ファイルがロックされている)場合も不可
		 && !( 0 != m_pcEditDoc->m_nFileShareModeOld && m_pcEditDoc->m_hLockedFile == NULL )
		 && !m_pcEditDoc->IsReadOnly() ){ // Mar. 30, 2003 読みとり専用のファイルにはドロップさせない
	}else{
		return E_UNEXPECTED;	//	Moca E_INVALIDARGから変更
	}

	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;
	if( IsDataAvailable( pDataObject, CF_TEXT )
//	 && NULL != GetGlobalData(pDataObject, CF_TEXT)
	){
		/* 自分をアクティブペインにする */
		m_pcEditDoc->SetActivePane( m_nMyIndex );

		/* 選択テキストのドラッグ中か */
		m_bDragMode = TRUE;

	//	/* 編集ウィンドウオブジェクトからのアクティブ要求 */
	//	::PostMessage( m_pShareData->m_hwndTray, MYWM_ACTIVATE_ME, (WPARAM)::GetParent( m_hwndParent ),  0 );

		/* 入力フォーカスを受け取ったときの処理 */
		OnSetFocus();

	//	::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
	//	OnLBUTTONDOWN( dwKeyState, pt.x, pt.y );

		m_pcDropTarget->m_pDataObject = pDataObject;
		/* Ctrl,ALT,キーが押されていたか */
		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL )
			|| FALSE == m_bDragSource	// Aug. 6, 2004 genta DragOver/Dropでは入っているがここだけ漏れていた
		){
			*pdwEffect = DROPEFFECT_COPY;
		}else{
			*pdwEffect = DROPEFFECT_MOVE;
		}
//		/* アクティブにする */
//		ActivateFrameWindow( GetParent( m_hwndParent ) );
//		ActivateFrameWindow( m_hWnd_DropTarget/*GetParent( m_hwndParent )*/ );

//		::SetFocus( m_hWnd_DropTarget );
		::SetFocus( m_hWnd );
	}else{
		return E_INVALIDARG;
//		*pdwEffect = DROPEFFECT_NONE;

	}
//	/* アクティブにする */
//	ActivateFrameWindow( ::GetParent( m_hwndParent ) );
//	::PostMessage( ::GetParent( m_hwndParent ), WM_ACTIVATE, MAKELONG( 0, WA_ACTIVE ), 0 );
//	/*
//	||	処理中のユーザー操作を可能にする
	//	||	ブロッキングフック (メッセージ配送)
//	*/
//	BlockingHook();

	DragOver( dwKeyState, pt, pdwEffect );
	return S_OK;
}

STDMETHODIMP CEditView::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::DragOver()\n" );
#endif
//	RECT	rc;
//	POINT	po;
//	po.x = pt.x;
//	po.y = pt.y;
//	::GetWindowRect(m_hWnd, &rc );
//	if( m_bDragSource && PtInRect( &rc, po ) ){
//	}else{
		/* マウス移動のメッセージ処理 */
//		::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
		::ScreenToClient( m_hWnd, (LPPOINT)&pt );
		OnMOUSEMOVE( dwKeyState, pt.x , pt.y );
//	}

//	MYTRACE( "m_nCaretPosX=%d, m_nCaretPosY=%d\n", m_nCaretPosX, m_nCaretPosY );

	if ( pdwEffect == NULL )
		return E_INVALIDARG;
//	::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
//	DWORD dwIndex = LOWORD( ::SendMessage( m_hWnd_DropTarget, EM_CHARFROMPOS, 0, MAKELPARAM( pt.x, pt.y ) ) );
//	if ( dwIndex != (WORD) -1 ){
//		::SendMessage( m_hWnd_DropTarget, EM_SETSEL, dwIndex, dwIndex );
//		::SendMessage( m_hWnd_DropTarget, EM_SCROLLCARET, 0, 0 );
//	}
	if( NULL == m_pcDropTarget->m_pDataObject ){
		*pdwEffect = DROPEFFECT_NONE;
	}else
	if( m_bDragSource
	 && 0 == IsCurrentPositionSelected( /* 指定カーソル位置が選択エリア内にあるか */
			m_nCaretPosX,				// カーソル位置X
			m_nCaretPosY				// カーソル位置Y
		)
	){
		*pdwEffect = DROPEFFECT_NONE;
	}else
	/* Ctrl,ALT,キーが押されていたか */
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL )
	 || FALSE == m_bDragSource
	){
		*pdwEffect = DROPEFFECT_COPY;
	}else{
		*pdwEffect = DROPEFFECT_MOVE;
	}
	return S_OK;
}



STDMETHODIMP CEditView::DragLeave( void )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::DragLeave()\n" );
#endif

	if( !m_bDragSource ){
		/* 入力フォーカスを失ったときの処理 */
		OnKillFocus();
		::SetFocus(NULL);
	}else{
		// 1999.11.15
		OnSetFocus();
		::SetFocus( ::GetParent( m_hwndParent ) );
	}

	m_pcDropTarget->m_pDataObject = NULL;

	/* 選択テキストのドラッグ中か */
	m_bDragMode = FALSE;

	return S_OK;
}

STDMETHODIMP CEditView::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::Drop()\n" );
#endif
	BOOL		bBoxSelected;
	BOOL		bMove;
	BOOL		bMoveToPrev;
	RECT		rcSel;
	int			nCaretPosX_Old;
	int			nCaretPosY_Old;
	CMemory		cmemBuf;
	CMemory		cmemClip;
	int			bBeginBoxSelect_Old;
//	int			nSelectLineBgn_Old;			/* 範囲選択開始行(原点) */
//	int			nSelectColBgn_Old;			/* 範囲選択開始桁(原点) */
	int			nSelectLineBgnFrom_Old;		/* 範囲選択開始行(原点) */
	int			nSelectColBgnFrom_Old;		/* 範囲選択開始桁(原点) */
	int			nSelectLineBgnTo_Old;		/* 範囲選択開始行(原点) */
	int			nSelectColBgnTo_Old;		/* 範囲選択開始桁(原点) */
	int			nSelectLineFrom_Old;
	int			nSelectColFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColTo_Old;
//	MYTRACE( "CEditView::Drop()\n" );

	if( !m_bDragSource
	 && IsTextSelected() ){	/* テキストが選択されているか */
		/* 現在の選択範囲を非選択状態に戻す */
		DisableSelectArea( TRUE );
	}
	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;
	*pdwEffect = DROPEFFECT_NONE;
	if( IsDataAvailable(pDataObject, CF_TEXT) ){
#ifdef _DEBUG
		MYTRACE( "TRUE == IsDataAvailable()\n" );
#endif
		HGLOBAL		hData = GetGlobalData(pDataObject, CF_TEXT);
#ifdef _DEBUG
		MYTRACE( "%xh == GetGlobalData(pDataObject, CF_TEXT)\n", hData );
#endif
		if (hData == NULL){
			m_pcDropTarget->m_pDataObject = NULL;
			/* 選択テキストのドラッグ中か */
			m_bDragMode = FALSE;
			return E_INVALIDARG;
		}

		DWORD		nSize = 0;
		LPCTSTR lpszSource = (LPCTSTR) ::GlobalLock(hData);

//		MYTRACE( "lpszSource=[%s]\n", lpszSource );

		/* 移動かコピーか */
		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL )
		 || FALSE == m_bDragSource
		){
			bMove = FALSE;
			*pdwEffect = DROPEFFECT_COPY;
		}else{
			bMove = TRUE;
			*pdwEffect = DROPEFFECT_MOVE;
		}

		if( m_bDragSource ){
			if( NULL != m_pcOpeBlk ){
			}else{
				m_pcOpeBlk = new COpeBlk;
			}
			bBoxSelected = m_bBeginBoxSelect;
			/* 選択範囲のデータを取得 */
//			GetSelectedData( cmemBuf, FALSE, NULL, FALSE );
//			cmemBuf.SetData( lpszSource, lstrlen( lpszSource ) );
			cmemBuf.SetDataSz( lpszSource );

			/* 移動の場合、位置関係を算出 */
			if( bMove ){
				if( bBoxSelected ){
					/* 2点を対角とする矩形を求める */
					TwoPointToRect(
						&rcSel,
						m_nSelectLineFrom,		/* 範囲選択開始行 */
						m_nSelectColmFrom,		/* 範囲選択開始桁 */
						m_nSelectLineTo,		/* 範囲選択終了行 */
						m_nSelectColmTo			/* 範囲選択終了桁 */
					);
					++rcSel.bottom;
					if( m_nCaretPosY >= rcSel.bottom ){
						bMoveToPrev = FALSE;
					}else
					if( m_nCaretPosY + rcSel.bottom - rcSel.top < rcSel.top ){
						bMoveToPrev = TRUE;
					}else
					if( m_nCaretPosX < rcSel.left ){
						bMoveToPrev = TRUE;
					}else{
						bMoveToPrev = FALSE;
					}
				}else{
					if( m_nSelectLineFrom > m_nCaretPosY ){
						bMoveToPrev = TRUE;
					}else
					if( m_nSelectLineFrom == m_nCaretPosY ){
						if( m_nSelectColmFrom > m_nCaretPosX ){
							bMoveToPrev = TRUE;
						}else{
							bMoveToPrev = FALSE;
						}
					}else{
						bMoveToPrev = FALSE;
					}
				}
			}
			if( !bMove ){
				/* コピーモード */
				/* 現在の選択範囲を非選択状態に戻す */
				DisableSelectArea( TRUE );
			}
			nCaretPosX_Old = m_nCaretPosX;
			nCaretPosY_Old = m_nCaretPosY;
			if( bMove ){
				if( bMoveToPrev ){
					/* 移動モード & 前に移動 */
					/* 選択エリアを削除 */
					DeleteData( TRUE );
					MoveCursor( nCaretPosX_Old, nCaretPosY_Old, TRUE );
				}else{
					bBeginBoxSelect_Old = m_bBeginBoxSelect;

					nSelectLineBgnFrom_Old	= m_nSelectLineBgnFrom;	/* 範囲選択開始行(原点) */
					nSelectColBgnFrom_Old	= m_nSelectColmBgnFrom;	/* 範囲選択開始桁(原点) */
					nSelectLineBgnTo_Old	= m_nSelectLineBgnTo;	/* 範囲選択開始行(原点) */
					nSelectColBgnTo_Old		= m_nSelectColmBgnTo;	/* 範囲選択開始桁(原点) */

					nSelectLineFrom_Old	= m_nSelectLineFrom;
					nSelectColFrom_Old	= m_nSelectColmFrom;
					nSelectLineTo_Old	= m_nSelectLineTo;
					nSelectColTo_Old	= m_nSelectColmTo;
					/* 現在の選択範囲を非選択状態に戻す */
					DisableSelectArea( TRUE );
				}
			}
			if( FALSE == bBoxSelected ){	/* 矩形範囲選択中 */
				//	2004,05.14 Moca 引数に文字列長を追加
				Command_INSTEXT( TRUE, cmemBuf.GetPtr(), cmemBuf.GetLength(), FALSE );
			}else{
				// 2004.07.12 Moca クリップボードを書き換えないように
				// TRUE == bBoxSelected
				// FALSE == m_bBeginBoxSelect
				cmemClip.SetDataSz( "" );
				/* 貼り付け（クリップボードから貼り付け）*/
				Command_PASTEBOX( cmemBuf.GetPtr(), cmemBuf.GetLength() );
				AdjustScrollBars(); // 2007.07.22 ryoji
				Redraw();
			}
			if( bMove ){
				if( bMoveToPrev ){
				}else{
					/* 移動モード & 後ろに移動*/
					m_bBeginBoxSelect = bBeginBoxSelect_Old;
					m_nSelectLineBgnFrom = nSelectLineBgnFrom_Old;	/* 範囲選択開始行(原点) */
					m_nSelectColmBgnFrom = nSelectColBgnFrom_Old;	/* 範囲選択開始桁(原点) */
					m_nSelectLineBgnTo = nSelectLineBgnTo_Old;		/* 範囲選択開始行(原点) */
					m_nSelectColmBgnTo = nSelectColBgnTo_Old;		/* 範囲選択開始桁(原点) */

					m_nSelectLineFrom = nSelectLineFrom_Old;
					m_nSelectColmFrom = nSelectColFrom_Old;
					m_nSelectLineTo = nSelectLineTo_Old;
					m_nSelectColmTo = nSelectColTo_Old;

					/* 選択エリアを削除 */
					DeleteData( TRUE );
					MoveCursor( nCaretPosX_Old, nCaretPosY_Old, TRUE );
				}
			}
			/* アンドゥバッファの処理 */
			if( NULL != m_pcOpeBlk ){
				if( 0 < m_pcOpeBlk->GetNum() ){	/* 操作の数を返す */
					/* 操作の追加 */
					m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
					m_pcEditDoc->RedrawInactivePane();	// 他のペインの表示	// 2007.07.22 ryoji
				}else{
					delete m_pcOpeBlk;
				}
				m_pcOpeBlk = NULL;
			}
		}else{
			HandleCommand( F_INSTEXT, TRUE, (LPARAM)lpszSource, TRUE, 0, 0 );
		}
		::GlobalUnlock(hData);
		// 2004.07.12 fotomo/もか メモリーリークの修正
		if( 0 == (GMEM_LOCKCOUNT & ::GlobalFlags(hData)) ){
			::GlobalFree(hData);
		}
	}else{
#ifdef _DEBUG
		MYTRACE( "FALSE == IsDataAvailable()\n" );
#endif
	}
	m_pcDropTarget->m_pDataObject = NULL;
//	::SetFocus(NULL);

	/* 選択テキストのドラッグ中か */
	m_bDragMode = FALSE;

	/* 編集ウィンドウオブジェクトからのアクティブ要求 */
	::SetFocus( ::GetParent( m_hwndParent ) );
	SetActiveWindow( m_hWnd );
//	::PostMessage( m_pShareData->m_hwndTray, MYWM_ACTIVATE_ME, (WPARAM)::GetParent( m_hwndParent ),  0 );

//	::ShowCaret( m_hWnd );
	return S_OK;
}





/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
void CEditView::GetCurrentTextForSearch( CMemory& cmemCurText )
{

	int				i;
	char			szTopic[_MAX_PATH];
//	CMemory			cmemCurText;
	const char*		pLine;
	int				nLineLen;
	int				nIdx;
	int				nLineFrom;
	int				nColmFrom;
	int				nLineTo;
	int				nColmTo;
//	const CLayout*	pcLayout;

	cmemCurText.SetDataSz( "" );
	szTopic[0] = '\0';
	if( IsTextSelected() ){	/* テキストが選択されているか */
		/* 選択範囲のデータを取得 */
		if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
			/* 検索文字列を現在位置の単語で初期化 */
			strncpy( szTopic, cmemCurText.GetPtr(), _MAX_PATH - 1 );
			szTopic[_MAX_PATH - 1] = '\0';
		}
	}else{
		const CLayout*	pcLayout;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* 指定された桁に対応する行のデータ内の位置を調べる */
			nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );

			/* 現在位置の単語の範囲を調べる */
			if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
				m_nCaretPosY, nIdx,
				&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
			){
				/* 指定された行のデータ内の位置に対応する桁の位置を調べる */
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
				nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
				nColmTo = LineIndexToColmn( pcLayout, nColmTo );
				/* 選択範囲の変更 */
				m_nSelectLineBgnFrom = nLineFrom;	/* 範囲選択開始行(原点) */
				m_nSelectColmBgnFrom = nColmFrom;	/* 範囲選択開始桁(原点) */
				m_nSelectLineBgnTo = nLineTo;		/* 範囲選択開始行(原点) */
				m_nSelectColmBgnTo = nColmTo;		/* 範囲選択開始桁(原点) */

				m_nSelectLineFrom = nLineFrom;
				m_nSelectColmFrom = nColmFrom;
				m_nSelectLineTo = nLineTo;
				m_nSelectColmTo = nColmTo;
				/* 選択範囲のデータを取得 */
				if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
					/* 検索文字列を現在位置の単語で初期化 */
					strncpy( szTopic, cmemCurText.GetPtr(), MAX_PATH - 1 );
					szTopic[MAX_PATH - 1] = '\0';
				}
				/* 現在の選択範囲を非選択状態に戻す */
				DisableSelectArea( FALSE );
			}
		}
	}

	/* 検索文字列は改行まで */
	for( i = 0; i < (int)lstrlen( szTopic ); ++i ){
		if( szTopic[i] == CR || szTopic[i] == LF ){
			szTopic[i] = '\0';
			break;
		}
	}
//	cmemCurText.SetData( szTopic, lstrlen( szTopic ) );
	cmemCurText.SetDataSz( szTopic );
	return;

}


/*!	現在カーソル位置単語または選択範囲より検索等のキーを取得（ダイアログ用）
	@date 2006.08.23 ryoji 新規作成
*/
void CEditView::GetCurrentTextForSearchDlg( CMemory& cmemCurText )
{
	cmemCurText.SetDataSz( "" );

	if( IsTextSelected() ){	// テキストが選択されている
		GetCurrentTextForSearch( cmemCurText );
	}
	else{	// テキストが選択されていない
		if( m_pShareData->m_Common.m_bCaretTextForSearch ){
			GetCurrentTextForSearch( cmemCurText );	// カーソル位置単語を取得
		}
		else{
			cmemCurText.SetDataSz( m_pShareData->m_szSEARCHKEYArr[0] );	// 履歴からとってくる
		}
	}
}


/* カーソル行アンダーラインのON */
void CCaretUnderLine::CaretUnderLineON( BOOL bDraw )
{
	if( m_nLockCounter ) return;	//	ロックされていたら何もできない。
	m_pcEditView->CaretUnderLineON( bDraw );
}



/* カーソル行アンダーラインのOFF */
void CCaretUnderLine::CaretUnderLineOFF( BOOL bDraw )
{
	if( m_nLockCounter ) return;	//	ロックされていたら何もできない。
	m_pcEditView->CaretUnderLineOFF( bDraw );
}


/*! カーソル行アンダーラインのON
	@date 2007.09.09 Moca カーソル位置縦線処理追加
*/
void CEditView::CaretUnderLineON( BOOL bDraw )
{

	BOOL bUnderLine = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp;
	BOOL bCursorVLine = m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bDisp;
	if( FALSE == bUnderLine && FALSE == bCursorVLine ){
		return;
	}

	if( IsTextSelected() ){	/* テキストが選択されているか */
		return;
	}
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	if( bCursorVLine ){
		// カーソル位置縦線。-1してキャレットの左に来るように。
		m_nOldCursorLineX = m_nViewAlignLeft + (m_nCaretPosX - m_nViewLeftCol)
			* (m_pcEditDoc->GetDocumentAttribute().m_nColmSpace + m_nCharWidth ) - 1;
		if( -1 == m_nOldCursorLineX ){
			m_nOldCursorLineX = -2;
		}
	}else{
		m_nOldCursorLineX = -1;
	}

	if( bDraw
	 && m_bDrawSWITCH
	 && m_nViewAlignLeft - m_pShareData->m_Common.m_nLineNumRightSpace < m_nOldCursorLineX
	 && m_nOldCursorLineX <= m_nViewAlignLeft + m_nViewCx
	 && m_bDoing_UndoRedo == FALSE
	){
		// カーソル位置縦線の描画
		// アンダーラインと縦線の交点で、下線が上になるように先に縦線を引く。
		HDC		hdc;
		HPEN	hPen, hPenOld;
		int     nROP_Old = 0;
		hdc = ::GetDC( m_hWnd );
		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, m_nOldCursorLineX, m_nViewAlignTop, NULL );
		::LineTo(   hdc, m_nOldCursorLineX, m_nViewCy + m_nViewAlignTop );
		// 「太字」のときは2dotの線にする。その際カーソルに掛からないように左側を太くする
		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bFatFont &&
			m_nViewAlignLeft - m_pShareData->m_Common.m_nLineNumRightSpace < m_nOldCursorLineX - 1 ){
			::MoveToEx( hdc, m_nOldCursorLineX - 1, m_nViewAlignTop, NULL );
			::LineTo(   hdc, m_nOldCursorLineX - 1, m_nViewCy + m_nViewAlignTop );
		}
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		::ReleaseDC( m_hWnd, hdc );
		hdc= NULL;
	}
	if( bUnderLine ){
		m_nOldUnderLineY = m_nViewAlignTop + (m_nCaretPosY - m_nViewTopLine)
			 * (m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight) + m_nCharHeight;
		if( -1 == m_nOldUnderLineY ){
			m_nOldUnderLineY = -2;
		}
	}else{
		m_nOldUnderLineY = -1;
	}
	// To Here 2007.09.09 Moca

	if( bDraw
	 && m_bDrawSWITCH
	 && m_nOldUnderLineY >=m_nViewAlignTop
	 && m_bDoing_UndoRedo == FALSE	/* アンドゥ・リドゥの実行中か */
	){
//		MYTRACE( "★カーソル行アンダーラインの描画\n" );
		/* ★カーソル行アンダーラインの描画 */
		HDC		hdc;
		HPEN	hPen, hPenOld;
		hdc = ::GetDC( m_hWnd );
		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx(
			hdc,
			m_nViewAlignLeft,
			m_nOldUnderLineY,
			NULL
		);
		::LineTo(
			hdc,
			m_nViewCx + m_nViewAlignLeft,
			m_nOldUnderLineY
		);
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		::ReleaseDC( m_hWnd, hdc );
		hdc= NULL;
	}
	return;
}



/*! カーソル行アンダーラインのOFF
	@date 2007.09.09 Moca カーソル位置縦線処理追加
*/
void CEditView::CaretUnderLineOFF( BOOL bDraw )
{
	if( FALSE == m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp &&
			FALSE == m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bDisp ){
		return;
	}

//	if( IsTextSelected() ){	/* テキストが選択されているか */
//		m_nOldUnderLineY = -1;
//		return;
//	}

	if( -1 != m_nOldUnderLineY ){
		if( bDraw
		 && m_bDrawSWITCH
		 && m_nOldUnderLineY >=m_nViewAlignTop
		 && m_bDoing_UndoRedo == FALSE	/* アンドゥ・リドゥの実行中か */
		){
//			MYTRACE( "★カーソル行アンダーラインの消去\n" );
			/* カーソル行アンダーラインの消去（無理やり） */
#if 1
			PAINTSTRUCT ps;
			ps.rcPaint.left = m_nViewAlignLeft;
			ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
			ps.rcPaint.top = m_nOldUnderLineY;
			ps.rcPaint.bottom = m_nOldUnderLineY + 1; // 2007.09.09 Moca +1 するように
			HDC hdc = ::GetDC( m_hWnd );
			m_cUnderLine.Lock();
			//	不本意ながら選択情報をバックアップ。
			int nSelectLineFrom = m_nSelectLineFrom;
			int nSelectLineTo = m_nSelectLineTo;
			int nSelectColmFrom = m_nSelectColmFrom;
			int nSelectColmTo = m_nSelectColmTo;
			m_nSelectLineFrom = -1;
			m_nSelectLineTo = -1;
			m_nSelectColmFrom = -1;
			m_nSelectColmTo = -1;
			// 可能なら互換BMPからコピーして再作画
			OnPaint( hdc, &ps, TRUE );
			//	選択情報を復元
			m_nSelectLineFrom = nSelectLineFrom;
			m_nSelectLineTo = nSelectLineTo;
			m_nSelectColmFrom = nSelectColmFrom;
			m_nSelectColmTo = nSelectColmTo;
			m_cUnderLine.UnLock();
			ReleaseDC( m_hWnd, hdc );
#else
			HDC		hdc;
			HPEN	hPen, hPenOld;
			hdc = ::GetDC( m_hWnd );
			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx(
				hdc,
				m_nViewAlignLeft,
				m_nOldUnderLineY,
				NULL
			);
			::LineTo(
				hdc,
				m_nViewCx + m_nViewAlignLeft,
				m_nOldUnderLineY
			);
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
			::ReleaseDC( m_hWnd, hdc );
#endif
		}
		m_nOldUnderLineY = -1;
	}
	
	// From Here 2007.09.09 Moca 互換BMPによる画面バッファ
	// カーソル位置縦線
	if( -1 != m_nOldCursorLineX ){
		if( bDraw
		 && m_bDrawSWITCH
		 && m_nViewAlignLeft - m_pShareData->m_Common.m_nLineNumRightSpace < m_nOldCursorLineX
		 && m_nOldCursorLineX <= m_nViewAlignLeft + m_nViewCx
		 && m_bDoing_UndoRedo == FALSE
		){
			PAINTSTRUCT ps;
			ps.rcPaint.left = m_nOldCursorLineX;
			ps.rcPaint.right = m_nOldCursorLineX + 1;
			ps.rcPaint.top = m_nViewAlignTop;
			ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_CURSORVLINE].m_bFatFont ){
				ps.rcPaint.left += -1;
			}
			HDC hdc = ::GetDC( m_hWnd );
			m_cUnderLine.Lock();
			//	不本意ながら選択情報をバックアップ。
			int nSelectLineFrom = m_nSelectLineFrom;
			int nSelectLineTo = m_nSelectLineTo;
			int nSelectColmFrom = m_nSelectColmFrom;
			int nSelectColmTo = m_nSelectColmTo;
			m_nSelectLineFrom = -1;
			m_nSelectLineTo = -1;
			m_nSelectColmFrom = -1;
			m_nSelectColmTo = -1;
			// 可能なら互換BMPからコピーして再作画
			OnPaint( hdc, &ps, TRUE );
			//	選択情報を復元
			m_nSelectLineFrom = nSelectLineFrom;
			m_nSelectLineTo = nSelectLineTo;
			m_nSelectColmFrom = nSelectColmFrom;
			m_nSelectColmTo = nSelectColmTo;
			m_cUnderLine.UnLock();
			ReleaseDC( m_hWnd, hdc );
		}
		m_nOldCursorLineX = -1;
	};
	// To Here 2007.09.09 Moca
	return;
}


#if 0
/* 現在、Enterなどで挿入する改行コードの種類を取得 */
CEOL CEditView::GetCurrentInsertEOL( void )
{
	return m_pcEditDoc->GetNewLineCode();
}
#endif

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
void CEditView::ExecCmd( const char* pszCmd, const int nFlgOpt )
{
	char				cmdline[1024];
	HANDLE				hStdOutWrite, hStdOutRead, hStdIn;
	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
	CDlgCancel				cDlgCancel;

	//	From Here 2006.12.03 maru 引数を拡張のため
	BOOL	bGetStdout;			//	子プロセスの標準出力を得る
	BOOL	bToEditWindow;		//	TRUE=編集中のウィンドウ / FALSAE=アウトプットウィンドウ
	BOOL	bSendStdin;			//	編集中ファイルを子プロセスSTDINに渡す
	
	bGetStdout = nFlgOpt & 0x01 ? TRUE : FALSE;
	bToEditWindow = nFlgOpt & 0x02 ? TRUE : FALSE;
	bSendStdin = nFlgOpt & 0x04 ? TRUE : FALSE;
	//	To Here 2006.12.03 maru 引数を拡張のため

	// 編集中のウィンドウに出力する場合の選択範囲処理用	/* 2007.04.29 maru */
	int	nLineFrom, nColmFrom;
	BOOL	bBeforeTextSelected = IsTextSelected();
	if (bBeforeTextSelected){
		nLineFrom = m_nSelectLineFrom;
		nColmFrom = m_nSelectColmFrom;
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
#ifdef _DEBUG
		MYTRACE( "CEditView::ExecCmd() TempFilename=[%s]\n", szTempFileName );
#endif
		
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

		//OSバージョン取得
		COsVersionInfo cOsVer;
		//コマンドライン文字列作成
		wsprintf( cmdline, "%s %s%s",
				( cOsVer.IsWin32NT() ? "cmd.exe" : "command.com" ),
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
		SetSelectArea( 0, 0, m_pcEditDoc->m_cLayoutMgr.GetLineCount(), 0 );
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
		char	work[1024];
		int		bufidx = 0;
		int		j;
		BOOL	bLoopFlag = TRUE;

		//中断ダイアログ表示
		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );
		//実行したコマンドラインを表示
		// 2004.09.20 naoh 多少は見やすく・・・
		if (FALSE==bToEditWindow) {	//	2006.12.03 maru アウトプットウィンドウにのみ出力
			char szTextDate[1024], szTextTime[1024];
			SYSTEMTIME systime;
			::GetLocalTime( &systime );
			CShareData::getInstance()->MyGetDateFormat( systime, szTextDate, sizeof( szTextDate ) - 1 );
			CShareData::getInstance()->MyGetTimeFormat( systime, szTextTime, sizeof( szTextTime ) - 1 );
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
							Command_INSTEXT(FALSE, work, read_cnt, TRUE);
						}
						bufidx = 0;
#ifdef _DEBUG
	//MYTRACE( "ExecCmd: No leap character\n");
#endif
					} else {
						char tmp = work[read_cnt-1];
						//	2006.12.03 maru アウトプットウィンドウor編集中のウィンドウ分岐追加
						if (FALSE==bToEditWindow) {
							work[read_cnt-1] = '\0';
							CShareData::getInstance()->TraceOut( "%s", work );
						} else {
							Command_INSTEXT(FALSE, work, read_cnt-1, TRUE);
						}
						work[0] = tmp;
						bufidx = 1;
#ifdef _DEBUG
	MYTRACE( "ExecCmd: Carry last character [%d]\n", tmp );
#endif
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
			if(result > 0) ActivateFrameWindow( m_pShareData->m_hwndDebug );
		}
		else {						//	2006.12.03 maru 編集中のウィンドウに出力時は最後に再描画
			Command_INSTEXT(FALSE, work, bufidx, TRUE);	/* 最後の文字の処理 */
			if (bBeforeTextSelected){	// 挿入された部分を選択状態に
				SetSelectArea( nLineFrom, nColmFrom, m_nCaretPosY, m_nCaretPosX );
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

/*!
	検索／置換／ブックマーク検索時の状態をステータスバーに表示する

	@date 2002.01.26 hor 新規作成
	@date 2002.12.04 genta 実体をCEditWndへ移動
*/
void CEditView::SendStatusMessage( const char* msg )
{
	m_pcEditDoc->m_pcEditWnd->SendStatusMessage( msg );
}
// 使わなくなりました minfu 2002.04.10 
///*  IMEからの再変換要求に応える minfu 2002.03.27 */
//LRESULT CEditView::RequestedReconversion(PRECONVERTSTRING pReconv)
//{
//	CMemory cmemBuf;
//	int nlen;
//	
//	/* 選択範囲のデータを取得 */
//	if( FALSE == GetSelectedData( cmemBuf, FALSE, NULL, FALSE, FALSE ) ){
//		::MessageBeep( MB_ICONHAND );
//		return 0L;
//	}
//	
//	/* pReconv構造体に 値をセット */
//	nlen =  cmemBuf.GetLength();
//	if ( pReconv != NULL ) {    
//		pReconv->dwSize = sizeof(RECONVERTSTRING) + nlen + 1;
//		pReconv->dwVersion = 0;
//		pReconv->dwStrLen = nlen ;
//		pReconv->dwStrOffset = sizeof(RECONVERTSTRING) ;
//		pReconv->dwCompStrLen = nlen;
//		pReconv->dwCompStrOffset = 0;
//		pReconv->dwTargetStrLen = nlen;
//		pReconv->dwTargetStrOffset = 0 ;
//		
//		strncpy ( (char *)(pReconv + 1), cmemBuf.GetPtr( NULL ), nlen);
//	
//	}/* pReconv がNULLのときはサイズを返すのみ */
//	
//	/* RECONVERTSTRING構造体のサイズが戻り値 */
//	return nlen + sizeof(RECONVERTSTRING);
//
//}
//
///*  IMEからの再変換要求に応える for 95/NT */ // 20020331 aroka
//LRESULT CEditView::RequestedReconversionW(PRECONVERTSTRING pReconv)
//{
//	CMemory cmemBuf;
//	int nlen;
//	
//	/* 選択範囲のデータを取得 */
//	if( FALSE == GetSelectedData( cmemBuf, FALSE, NULL, FALSE, FALSE ) ){
//		::MessageBeep( MB_ICONHAND );
//		return 0L;
//	}
//	
//	/* cmemBuf を UNICODE に変換 */
//	cmemBuf.SJISToUnicode();
//	/* pReconv構造体に 値をセット */
//	nlen =  cmemBuf.GetLength();
//	if ( pReconv != NULL ) {
//		pReconv->dwSize = sizeof(RECONVERTSTRING) + nlen  + sizeof(wchar_t);
//		pReconv->dwVersion = 0;
//		pReconv->dwStrLen = nlen/sizeof(wchar_t) ;
//		pReconv->dwStrOffset = sizeof(RECONVERTSTRING) ;
//		pReconv->dwCompStrLen = nlen/sizeof(wchar_t);
//		pReconv->dwCompStrOffset = 0;
//		pReconv->dwTargetStrLen = nlen/sizeof(wchar_t);
//		pReconv->dwTargetStrOffset = 0 ;
//		
//		wcsncpy ( (wchar_t *)(pReconv + 1), (wchar_t *)cmemBuf.GetPtr(), nlen/sizeof(wchar_t) );
//	
//	}/* pReconv がNULLのときはサイズを返すのみ */
//	
//	/* RECONVERTSTRING構造体のサイズが戻り値 */
//	return nlen + sizeof(RECONVERTSTRING);
//
//}

//  2002.04.09 minfu from here
/*再変換用 カーソル位置から前後200byteを取り出してRECONVERTSTRINGを埋める */
/*  引数  pReconv RECONVERTSTRING構造体へのポインタ。                     */
/*        bUnicode trueならばUNICODEで構造体を埋める                      */
/*  戻り値   RECONVERTSTRINGのサイズ                                      */
LRESULT CEditView::SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode)
{
	const char	*pLine;
	int 		nLineLen;
	int			nCurrentLine /* , nCurrentColumn */ ;
//	int			nCurLogicalLine;
	
	//行の中で再変換のAPIにわたすとする文字列の開始位置と長さ（考慮文字列）
	int			nReconvIndex, nReconvLen, nReconvLenWithNull ;
	
	//行の中で再変換の注目する文節とする文字列の開始位置、終了位置、長さ（対象文字列）
	int			nSelectedIndex, nSelectedEndIndex, nSelectedLen;
	
	int			nSelectColumnFrom /* , nSelectColumnTo */ ;
	int			nSelectLineFrom, nSelectLineTo;
	
	DWORD		dwReconvTextLen;
	DWORD		dwCompStrOffset, dwCompStrLen;
	
//	int			nCurrentIndex;
	CMemory		cmemBuf1;
	const char*		pszReconv;
	CDocLine*	pcCurDocLine;
	
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;
	
	//矩形選択中は何もしない
	if( m_bBeginBoxSelect )
		return 0;
	
//	char sz[1024];
	if( IsTextSelected() ){
		//テキストが選択されているとき
		nSelectColumnFrom = m_nSelectColmFrom;
		nSelectLineFrom   = m_nSelectLineFrom;
		
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(m_nSelectColmFrom, m_nSelectLineFrom, &nSelectedIndex, &nCurrentLine);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(m_nSelectColmTo, m_nSelectLineTo, &nSelectedEndIndex, &nSelectLineTo);
		
		//選択範囲が複数行の時は
		if (nSelectLineTo != nCurrentLine){
			//行末までに制限
			pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo(nCurrentLine);
			nSelectedEndIndex = pcCurDocLine->m_pLine->GetLength();
		}
		
		nSelectedLen = nSelectedEndIndex - nSelectedIndex;
		
	}else{
		//テキストが選択されていないとき
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(m_nCaretPosX ,m_nCaretPosY , &nSelectedIndex, &nCurrentLine);
		nSelectedLen = 0;
	}
	
	pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo(nCurrentLine);
	
	if (NULL == pcCurDocLine )
		return 0;
	
	nLineLen = pcCurDocLine->m_pLine->GetLength() - pcCurDocLine->m_cEol.GetLen() ; //改行コードをのぞいた長さ
	
	if ( 0 == nLineLen )
		return 0;
	
	pLine = pcCurDocLine->m_pLine->GetPtr();

	//再変換考慮文字列開始
	nReconvIndex = 0;
	if ( nSelectedIndex > 200 ) {
		const char* pszWork = pLine;
		while( (nSelectedIndex - nReconvIndex) > 200 ){
			pszWork = ::CharNext( pszWork);
			nReconvIndex = pszWork - pLine ;
		}
	}
	
	//再変換考慮文字列終了
	nReconvLen = nLineLen - nReconvIndex;
	if ( (nReconvLen + nReconvIndex - nSelectedIndex) > 200 ){
		const char* pszWork = pLine + nSelectedIndex;
		nReconvLen = nSelectedIndex - nReconvIndex;
		while( ( nReconvLen + nReconvIndex - nSelectedIndex) <= 200 ){
			pszWork = ::CharNext( pszWork);
			nReconvLen = pszWork - (pLine + nReconvIndex) ;
		}
	}
	
	//対象文字列の調整
	if ( nSelectedIndex + nSelectedLen > nReconvIndex + nReconvLen ){
		nSelectedLen = nReconvIndex + nReconvLen - nSelectedIndex;
	}
	
	pszReconv =  pLine + nReconvIndex;
	
	if(bUnicode){
		
		//考慮文字列の開始から対象文字列の開始まで
		if( nSelectedIndex - nReconvIndex > 0 ){
			cmemBuf1.SetData(pszReconv, nSelectedIndex - nReconvIndex);
			cmemBuf1.SJISToUnicode();
			dwCompStrOffset = cmemBuf1.GetLength();  //Offset はbyte
		}else{
			dwCompStrOffset = 0;
		}
		
		//対象文字列の開始から対象文字列の終了まで
		if (nSelectedLen > 0 ){
			cmemBuf1.SetData(pszReconv + nSelectedIndex, nSelectedLen);  
			cmemBuf1.SJISToUnicode();
			dwCompStrLen = cmemBuf1.GetLength() / sizeof(wchar_t);
		}else{
			dwCompStrLen = 0;
		}
		
		//考慮文字列すべて
		cmemBuf1.SetData(pszReconv , nReconvLen );
		cmemBuf1.SJISToUnicode();
		
		dwReconvTextLen =  cmemBuf1.GetLength() / sizeof(wchar_t);
		nReconvLenWithNull =  cmemBuf1.GetLength()  + sizeof(wchar_t);
		
		pszReconv = cmemBuf1.GetPtr();
		
	}else{
		dwReconvTextLen = nReconvLen;
		nReconvLenWithNull = nReconvLen + 1;
		dwCompStrOffset = nSelectedIndex - nReconvIndex;
		dwCompStrLen    = nSelectedLen;
	}
	
	if ( NULL != pReconv) {
		//再変換構造体の設定
		pReconv->dwSize = sizeof(RECONVERTSTRING) + nReconvLenWithNull ;
		pReconv->dwVersion = 0;
		pReconv->dwStrLen = dwReconvTextLen ;
		pReconv->dwStrOffset = sizeof(RECONVERTSTRING) ;
		pReconv->dwCompStrLen = dwCompStrLen;
		pReconv->dwCompStrOffset = dwCompStrOffset;
		pReconv->dwTargetStrLen = dwCompStrLen;
		pReconv->dwTargetStrOffset = dwCompStrOffset;
		
		// 2004.01.28 Moca ヌル終端の修正
		if( bUnicode ){
			CopyMemory( (void *)(pReconv + 1), (void *)pszReconv , nReconvLenWithNull - sizeof(wchar_t) );
			*((wchar_t *)(pReconv + 1) + nReconvLenWithNull - sizeof(wchar_t) ) = L'\0';
		}else{
			CopyMemory( (void *)(pReconv + 1), (void *)pszReconv , nReconvLenWithNull - 1 );
			*((char *)(pReconv + 1) + nReconvLenWithNull - 1 ) = '\0';
		}
	}
	
	// 再変換情報の保存
	m_nLastReconvIndex = nReconvIndex;
	m_nLastReconvLine  = nCurrentLine;
	
	return sizeof(RECONVERTSTRING) + nReconvLenWithNull;

}

/*再変換用 エディタ上の選択範囲を変更する 2002.04.09 minfu */
LRESULT CEditView::SetSelectionFromReonvert(PRECONVERTSTRING pReconv, bool bUnicode){
	
	DWORD		dwOffset, dwLen;
	CMemory		cmemBuf;
//	char		sz[2000];
	
	// 再変換情報が保存されているか
	if ( (m_nLastReconvIndex < 0) || (m_nLastReconvLine < 0))
		return 0;

	if ( IsTextSelected()) 
		DisableSelectArea( TRUE );
	
	if (bUnicode){
		
		//考慮文字列の開始から対象文字列の開始まで
		if( pReconv->dwCompStrOffset > 0){
			cmemBuf.SetData((const char *)((const wchar_t *)(pReconv + 1)), 
								pReconv->dwCompStrOffset ); 
			cmemBuf.UnicodeToSJIS();
			dwOffset = cmemBuf.GetLength();
			
		}else
			dwOffset = 0;

		//対象文字列の開始から対象文字列の終了まで
		if( pReconv->dwCompStrLen > 0 ){
			cmemBuf.SetData((const char *)(const wchar_t *)(pReconv + 1) + pReconv->dwCompStrOffset , 
								pReconv->dwCompStrLen * sizeof(wchar_t)); 
			cmemBuf.UnicodeToSJIS();
			dwLen = cmemBuf.GetLength();
			
		}else
			dwLen = 0;
		
	}else{
		dwOffset = pReconv->dwCompStrOffset ;
		dwLen =  pReconv->dwCompStrLen;
	}
	
	//選択開始の位置を取得
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(m_nLastReconvIndex + dwOffset 
												, m_nLastReconvLine, &m_nSelectColmFrom, &m_nSelectLineFrom);
	//選択終了の位置を取得
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(m_nLastReconvIndex + dwOffset + dwLen
												, m_nLastReconvLine, &m_nSelectColmTo, &m_nSelectLineTo);

	/* 単語の先頭にカーソルを移動 */
	MoveCursor( m_nSelectColmFrom, m_nSelectLineFrom, TRUE );

	//選択範囲再描画 
	DrawSelectArea();

	// 再変換情報の破棄
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;

	return 1;

}


// 2002/07/22 novice
/*!
	キャレットの表示
*/
void CEditView::ShowCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == false ){
		::ShowCaret( hwnd );
		m_bCaretShowFlag = true;
	}
}


/*!
	キャレットの非表示
*/
void CEditView::HideCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == true ){
		::HideCaret( hwnd );
		m_bCaretShowFlag = false;
	}
}

/*!
	@date 2003/02/18 ai
	@param flag [in] モード(true:登録, false:解除)
*/
void CEditView::SetBracketPairPos( bool flag )
{
	int	nCol;
	int	nLine;
	int	mode;

	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if( m_bDoing_UndoRedo || ( m_bDrawSWITCH == FALSE ) ){
		return;
	}

	if( !m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// 対括弧の検索&登録
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	mode = 2;

	if( ( flag == true ) && !IsTextSelected() && ( m_bDrawSelectArea == FALSE )
		&& ( m_bBeginBoxSelect == FALSE ) && SearchBracket( m_nCaretPosX, m_nCaretPosY, &nCol, &nLine, &mode ) )
	{
		// 登録指定(flag=true)			&&
		// テキストが選択されていない	&&
		// 選択範囲を描画していない		&&
		// 矩形範囲選択中でない			&&
		// 対応する括弧が見つかった		場合
		if ( ( nCol >= m_nViewLeftCol ) && ( nCol <= m_nViewLeftCol + m_nViewColNum )
			&& ( nLine >= m_nViewTopLine ) && ( nLine <= m_nViewTopLine + m_nViewRowNum ) )
		{
			// 表示領域内の場合

			// レイアウト位置から物理位置へ変換(強調表示位置を登録)
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys( nCol, nLine, &m_nBracketPairPosX_PHY, &m_nBracketPairPosY_PHY );
			m_nBracketCaretPosY_PHY = m_nCaretPosY_PHY;
			if( 0 == ( mode & 4 ) ){
				// カーソルの後方文字位置
				m_nBracketCaretPosX_PHY = m_nCaretPosX_PHY;
			}else{
				// カーソルの前方文字位置
				m_nBracketCaretPosX_PHY = m_nCaretPosX_PHY - m_nCharSize;
			}
			return;
		}
	}

	// 括弧の強調表示位置情報初期化
	m_nBracketPairPosX_PHY  = -1;
	m_nBracketPairPosY_PHY  = -1;
	m_nBracketCaretPosX_PHY = -1;
	m_nBracketCaretPosY_PHY = -1;

	return;
}

/*!
	対括弧の強調表示
	@date 2002/09/18 ai
	@date 2003/02/18 ai 再描画対応の為大改造
*/
void CEditView::DrawBracketPair( bool bDraw )
{
	int			i;
	const char*	pLine;
	int			nLineLen;
	int			nCol;
	int			nLine;
	int			OutputX;
	int			nLeft;
	int			nTop;
	HDC			hdc;
	COLORREF	crBackOld;
	COLORREF	crTextOld;
	HFONT		hFontOld;

	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if( m_bDoing_UndoRedo || ( m_bDrawSWITCH == FALSE ) ){
		return;
	}

	if( !m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// 括弧の強調表示位置が未登録の場合は終了
	if( ( m_nBracketPairPosX_PHY  < 0 ) || ( m_nBracketPairPosY_PHY  < 0 )
	 || ( m_nBracketCaretPosX_PHY < 0 ) || ( m_nBracketCaretPosY_PHY < 0 ) ){
		return;
	}

	// 描画指定(bDraw=true)				かつ
	// ( テキストが選択されている		又は
	//   選択範囲を描画している			又は
	//   矩形範囲選択中					又は
	//   フォーカスを持っていない		又は
	//   アクティブなペインではない )	場合は終了
	if( bDraw
	 &&( IsTextSelected() || ( m_bDrawSelectArea == TRUE ) || m_bBeginBoxSelect || ( m_bDrawBracketPairFlag == FALSE )
	 || ( m_pcEditDoc->m_nActivePaneIndex != m_nMyIndex ) ) ){
		return;
	}

	hdc = ::GetDC( m_hWnd );
	Types *TypeDataPtr = &( m_pcEditDoc->GetDocumentAttribute() );

	for( i = 0; i < 2; i++ )
	{	// i=0:カーソル位置の括弧,i=1:対括弧
		if( i == 0 ){
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( m_nBracketCaretPosX_PHY, m_nBracketCaretPosY_PHY, &nCol, &nLine );
		}else{
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( m_nBracketPairPosX_PHY,  m_nBracketPairPosY_PHY,  &nCol, &nLine );
		}

		if ( ( nCol >= m_nViewLeftCol ) && ( nCol <= m_nViewLeftCol + m_nViewColNum )
			&& ( nLine >= m_nViewTopLine ) && ( nLine <= m_nViewTopLine + m_nViewRowNum ) )
		{	// 表示領域内の場合
			if( ( bDraw == false ) && ( m_bDrawSelectArea == TRUE ) && ( 0 == IsCurrentPositionSelected( nCol, nLine ) ) )
			{	// 選択範囲描画済みで消去対象の括弧が選択範囲内の場合
				continue;
			}
			const CLayout* pcLayout;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLine, &nLineLen, &pcLayout );
			if( NULL != pLine )
			{
				int nColorIndex;
				OutputX = LineColmnToIndex( pcLayout, nCol );
				if( bDraw )	{
					nColorIndex = COLORIDX_BRACKET_PAIR;
				}else{
					if( IsBracket( pLine, OutputX, m_nCharSize ) ){
						// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
						//nColorIndex = GetColorIndex( hdc, pcLayout, OutputX );
						if( i == 0 ){
							nColorIndex = GetColorIndex( hdc, pcLayout, m_nBracketCaretPosX_PHY );
						}else{
							nColorIndex = GetColorIndex( hdc, pcLayout, m_nBracketPairPosX_PHY );
						}
					}else{
						SetBracketPairPos( false );
						//::MessageBeep( MB_ICONSTOP );
						break;
					}
				}
				hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
				m_hFontOld = NULL;
				crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
				crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT );
				SetCurrentColor( hdc, nColorIndex );

				int nHeight = m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
				nLeft = (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + nCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				nTop  = ( nLine - m_nViewTopLine ) * nHeight + m_nViewAlignTop;

				// 03/03/03 ai カーソルの左に括弧があり括弧が強調表示されている状態でShift+←で選択開始すると
				//             選択範囲内に反転表示されない部分がある問題の修正
				if( /*bDraw &&*/ ( nCol == m_nCaretPosX ) && ( m_bCaretShowFlag == true ) ){
					HideCaret_( m_hWnd );	// キャレットが一瞬消えるのを防止
					DispText( hdc, nLeft, nTop, &pLine[OutputX], m_nCharSize );
					// 2006.04.30 Moca 対括弧の縦線対応
					DispVerticalLines( hdc, nTop, nTop + nHeight, nCol, nCol + m_nCharSize );
					ShowCaret_( m_hWnd );	// キャレットが一瞬消えるのを防止
				}else{
					DispText( hdc, nLeft, nTop, &pLine[OutputX], m_nCharSize );
					// 2006.04.30 Moca 対括弧の縦線対応
					DispVerticalLines( hdc, nTop, nTop + nHeight, nCol, nCol + m_nCharSize );
				}

				if( NULL != m_hFontOld ){
					::SelectObject( hdc, m_hFontOld );
					m_hFontOld = NULL;
				}
				::SetTextColor( hdc, crTextOld );
				::SetBkColor( hdc, crBackOld );
				::SelectObject( hdc, hFontOld );

				if( ( m_pcEditDoc->m_nActivePaneIndex == m_nMyIndex )
					&& ( ( nLine == m_nCaretPosY ) || ( nLine - 1 == m_nCaretPosY ) ) ){	// 03/02/27 ai 行の間隔が"0"の時にアンダーラインが欠ける事がある為修正
					m_cUnderLine.CaretUnderLineON( TRUE );
				}
			}
		}
	}

	::ReleaseDC( m_hWnd, hdc );

	return;
}

/*! 指定位置のColorIndexの取得
	CEditView::DispLineNewを元にしたためCEditView::DispLineNewに
	修正があった場合は、ここも修正が必要。

	@par nCOMMENTMODE
	関数内部で状態遷移のために使われる変数nCOMMENTMODEと状態の関係。
 - COLORIDX_TEXT     : テキスト
 - COLORIDX_COMMENT  : 行コメント
 - COLORIDX_BLOCK1   : ブロックコメント1
 - COLORIDX_SSTRING  : シングルコーテーション
 - COLORIDX_WSTRING  : ダブルコーテーション
 - COLORIDX_KEYWORD1 : 強調キーワード1
 - COLORIDX_CTRLCODE : コントロールコード
 - COLORIDX_DIGIT    : 半角数値
 - COLORIDX_BLOCK2   : ブロックコメント2
 - COLORIDX_KEYWORD2 : 強調キーワード2
 - COLORIDX_URL      : URL
 - COLORIDX_SEARCH   : 検索
 - 1000: 正規表現キーワード
 	色指定SetCurrentColorを呼ぶときにCOLORIDX_*値を加算するので、
 	1000～COLORIDX_LASTまでは正規表現で使用する。
*/
int CEditView::GetColorIndex(
		HDC						hdc,
		const CLayout*			pcLayout,
		int						nCol
)
{
	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	const char*				pLine;	//@@@ 2002.09.22 YAZAKI
	int						nLineLen;
	int						nLineBgn;
	int						nBgn;
	int						nPos;
	int						nCharChars;
	int						nCharChars_2;
	int						nCOMMENTMODE;
	int						nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	const CLayout*			pcLayout2;
	int						i, j;
	int						nIdx;
	int						nUrlLen;
	BOOL					bSearchStringMode;
	BOOL					bSearchFlg;			// 2002.02.08 hor
	int						nSearchStart;		// 2002.02.08 hor
	int						nSearchEnd;
	bool					bKeyWordTop = true;	//	Keyword Top
	int						nColorIndex;

//@@@ 2001.11.17 add start MIK
	int		nMatchLen;
	int		nMatchColor;
//@@@ 2001.11.17 add end MIK

	bSearchStringMode = FALSE;
	bSearchFlg	= TRUE;	// 2002.02.08 hor
	nSearchStart= -1;	// 2002.02.08 hor
	nSearchEnd	= -1;	// 2002.02.08 hor

	/* 論理行データの取得 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory変更
		nLineLen = pcLayout->m_pCDocLine->m_pLine->GetLength()/* - pcLayout->m_nOffset*/;	// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
		pLine = pcLayout->m_pCDocLine->m_pLine->GetPtr()/* + pcLayout->m_nOffset*/;			// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応

		// 2005.11.20 Moca 色が正しくないことがある問題に対処
		const CLayout* pcLayoutLineFirst = pcLayout;
		// 論理行の最初のレイアウト情報を取得する
		while( 0 != pcLayoutLineFirst->m_nOffset ){
			pcLayoutLineFirst = pcLayoutLineFirst->m_pPrev;
		}
		nCOMMENTMODE = pcLayoutLineFirst->m_nTypePrev;
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

	}else{
		pLine = NULL;
		nLineLen = 0;
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;
		pcLayout2 = NULL;
	}

	/* 現在の色を指定 */
	//@SetCurrentColor( hdc, nCOMMENTMODE );
	nColorIndex = nCOMMENTMODE;	// 02/12/18 ai

	nBgn = 0;
	nPos = 0;
	nLineBgn = 0;
	nCharChars = 0;

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		while( nPos <= nCol ){	// 03/10/24 ai 行頭のColorIndexが取得できない問題に対応

			nBgn = nPos;
			nLineBgn = nBgn;

			while( nPos - nLineBgn <= nCol ){	// 02/12/18 ai
				/* 検索文字列の色分け */
				if( TRUE == m_bCurSrchKeyMark	/* 検索文字列のマーク */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor 正規表現の検索文字列マークを少し高速化
					if(!bSearchStringMode && (!m_bCurSrchRegularExp || (bSearchFlg && nSearchStart < nPos))){
						bSearchFlg=IsSearchString( pLine, nLineLen, nPos, &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==nPos
					){
						nBgn = nPos;
						bSearchStringMode = TRUE;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
						nColorIndex = COLORIDX_SEARCH;	// 02/12/18 ai
					}else
					if( bSearchStringMode
					 && nSearchEnd == nPos
					){
						nBgn = nPos;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( nPos >= nLineLen - pcLayout2->m_cEol.GetLen() ){
					goto end_of_line;
				}
				SEARCH_START:;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//正規表現キーワード
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, nPos, nLineLen, &nMatchLen, &nMatchColor )
					 /*&& TypeDataPtr->m_ColorInfoArr[nMatchColor].m_bDisp*/ )
					{
						/* 現在の色を指定 */
						nBgn = nPos;
						nCOMMENTMODE = 1000 + nMatchColor;	/* 色指定 */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* キーワード文字列の終端をセットする */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}
					else
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;

						nCOMMENTMODE = COLORIDX_COMMENT;	/* 行コメントである */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* ブロックコメント1である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos + (int)lstrlen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* ブロックコメント2である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos + (int)lstrlen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
//#endif
					}else
					if( pLine[nPos] == '\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* シングルクォーテーション文字列を表示する */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_SSTRING;	/* シングルクォーテーション文字列である */ // 2002/03/13 novice

						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( pLine[nPos] == '"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* ダブルクォーテーション文字列を表示する */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_WSTRING;	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URLを表示する */
					 && ( TRUE == IsURL( &pLine[nPos], nLineLen - nPos, &nUrlLen ) )	/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_URL;	/* URLモード */ // 2002/03/13 novice
						nCOMMENTEND = nPos + nUrlLen;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 Start by MIK: 半角数値を強調表示
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* 半角数字を表示する */
					{
						/* キーワード文字列の終端をセットする */
						i = nPos + i;
						/* 現在の色を指定 */
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_DIGIT;	/* 半角数値である */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 End by MIK: 半角数値を強調表示
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx[0] != -1 && /* キーワードセット */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* 強調キーワードを表示する */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						/* キーワード文字列の終端を探す */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* キーワードが登録単語ならば、色を変える */
						j = i - nPos;
						/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx[0],
							&pLine[nPos],
							j
						);
						if( nIdx != -1 ){
							/* 現在の色を指定 */
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_KEYWORD1;	/* 強調キーワード1 */ // 2002/03/13 novice
							nCOMMENTEND = i;
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
						}else{		//MIK START ADD 2000.12.01 second keyword & binary search
							// 2005.01.13 MIK 強調キーワード数追加に伴う配列化
							for( int my_i = 1; my_i < 10; my_i++ )
							{
								if(TypeDataPtr->m_nKeyWordSetIdx[my_i] != -1 && /* キーワードセット */							//MIK 2000.12.01 second keyword
									TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1 + my_i].m_bDisp)									//MIK
								{																							//MIK
									/* ｎ番目のセットから指定キーワードをサーチ 無いときは-1を返す */						//MIK
									nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
										TypeDataPtr->m_nKeyWordSetIdx[my_i] ,													//MIK
										&pLine[nPos],																		//MIK
										j																					//MIK
									);																						//MIK
									if( nIdx != -1 ){																		//MIK
										/* 現在の色を指定 */																//MIK
										nBgn = nPos;																		//MIK
										nCOMMENTMODE = COLORIDX_KEYWORD1 + my_i;	/* 強調キーワード2 */ // 2002/03/13 novice		//MIK
										nCOMMENTEND = i;																	//MIK
										if( !bSearchStringMode ){															//MIK
											//@SetCurrentColor( hdc, nCOMMENTMODE );										//MIK
											nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
										}																					//MIK
										break;
									}																						//MIK
								}																							//MIK
								else
								{
									if(TypeDataPtr->m_nKeyWordSetIdx[my_i] == -1 )
										break;
								}
							}
						}			//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[nPos] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URLモードである */
				case COLORIDX_KEYWORD1:	/* 強調キーワード1 */
				case COLORIDX_DIGIT:	/* 半角数値である */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* 強調キーワード2 */	//MIK
				case COLORIDX_KEYWORD3:
				case COLORIDX_KEYWORD4:
				case COLORIDX_KEYWORD5:
				case COLORIDX_KEYWORD6:
				case COLORIDX_KEYWORD7:
				case COLORIDX_KEYWORD8:
				case COLORIDX_KEYWORD9:
				case COLORIDX_KEYWORD10:
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* コントロールコード */ // 2002/03/13 novice
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* 行コメントである */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* ブロックコメント1である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_BLOCK2:	/* ブロックコメント2である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* この物理行にブロックコメントの終端があるか */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_SSTRING:	/* シングルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* シングルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* ダブルクォーテーション文字列である */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ダブルクォーテーション文字列の終端があるか */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* 文字列区切り記号エスケープ方法 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* 現在の色を指定 */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 ){	//正規表現キーワード1～10
						if( nPos == nCOMMENTEND ){
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* 現在の色を指定 */
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pLine[nPos] == TAB ){
					nBgn = nPos + 1;
					nCharChars = 1;
				}else
				if( (unsigned char)pLine[nPos] == 0x81 && (unsigned char)pLine[nPos + 1] == 0x40	//@@@ 2001.11.17 upd MIK
				 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )	//@@@ 2002.01.04
				{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
					nBgn = nPos + 2;
					nCharChars = 2;
				}
				//半角空白（半角スペース）を表示 2002.04.28 Add by KK 
				else if (pLine[nPos] == ' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
					 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					nBgn = nPos + 1;
					nCharChars = 1;
				}
				else{
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, nPos );
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
					 &&	(
								//	Jan. 23, 2002 genta 警告抑制
							( (unsigned char)pLine[nPos] <= (unsigned char)0x1F ) ||
							( (unsigned char)'~' < (unsigned char)pLine[nPos] && (unsigned char)pLine[nPos] < (unsigned char)'｡' ) ||
							( (unsigned char)'ﾟ' < (unsigned char)pLine[nPos] )
						)
					 && pLine[nPos] != TAB && pLine[nPos] != CR && pLine[nPos] != LF
					){
						nBgn = nPos;
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* コントロールコード モード */ // 2002/03/13 novice
						/* コントロールコード列の終端を探す */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							// 2005-09-02 D.S.Koba GetSizeOfChar
							nCharChars_2 = CMemory::GetSizeOfChar( pLine, nLineLen, i );
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if( (
								//	Jan. 23, 2002 genta 警告抑制
								( (unsigned char)pLine[i] <= (unsigned char)0x1F ) ||
									( (unsigned char)'~' < (unsigned char)pLine[i] && (unsigned char)pLine[i] < (unsigned char)'｡' ) ||
									( (unsigned char)'ﾟ' < (unsigned char)pLine[i] )
								) &&
								pLine[i] != TAB && pLine[i] != CR && pLine[i] != LF
							){
							}else{
								break;
							}
						}
						nCOMMENTEND = i;
						/* 現在の色を指定 */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
					}
				}
				nPos+= nCharChars;
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength ){
			if( nPos > nCol ){	// 03/10/24 ai 行頭のColorIndexが取得できない問題に対応
				break;
			}
		}

end_of_line:;

	}

//@end_of_func:;
	return nColorIndex;
}

/*!	挿入モード取得

	@date 2005.10.02 genta 管理方法変更のため関数化
*/
bool CEditView::IsInsMode(void) const
{
	return m_pcEditDoc->IsInsMode();
}

void CEditView::SetInsMode(bool mode)
{
	m_pcEditDoc->SetInsMode( mode );
}
/*[EOF]*/
