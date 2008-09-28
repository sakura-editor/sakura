//2002.01.08 aroka  コントロールプロセスと起動処理のためにミューテックス名を追加
//2006.04.10 ryoji  コントロールプロセス初期化完了を示すイベントフラグ名を追加
//2007.09.05 kobake ANSI版と衝突を避けるため、名前変更
//2007.09.20 kobake ANSI版とUNICODE版で別の名前を用いる

#pragma once



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      パフォーマンス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//バッファサイズ
const int LINEREADBUFSIZE	= 10240;	//!< ファイルから1行分データを読み込むときのバッファサイズ



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       定数命名補助                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//デバッグ判別、定数サフィックス 2007.09.20 kobake
#ifdef _DEBUG
	#define _DEBUG_SUFFIX_ "_DEBUG"
#else
	#define _DEBUG_SUFFIX_ ""
#endif

//ビルドコード判別、定数サフィックス 2007.09.20 kobake
#ifdef _UNICODE
	#define _CODE_SUFFIX_ "W"
#else
	#define _CODE_SUFFIX_ "A"
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        共有メモリ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#define N_SHAREDATA_VERSION		1023
#define STR_SHAREDATA_VERSION	"1023"
#define	GSTR_SHAREDATA	(_T("SakuraShareData") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_) _T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ミューテックス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//旧サクラエディタ (1.2.104.1以前)
#define	GSTR_MUTEX_SAKURA_OLD				_T("MutexSakuraEditor")

//コントロールプロセス
#define	GSTR_MUTEX_SAKURA_CP				(_T("MutexSakuraEditorCP")				_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//ノーマルプロセス初期化同期
#define	GSTR_MUTEX_SAKURA_INIT				(_T("MutexSakuraEditorInit")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//ノード操作同期
#define	GSTR_MUTEX_SAKURA_EDITARR			(_T("MutexSakuraEditorEditArr")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#define	GSTR_EVENT_SAKURA_CP_INITIALIZED	(_T("EventSakuraEditorCPInitialized")	_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ウィンドウクラス                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//コントロールトレイ
#define	GSTR_CEDITAPP		(_T("CControlTray") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)		_T(STR_SHAREDATA_VERSION))

//メインウィンドウ
#define	GSTR_EDITWINDOWNAME	(_T("TextEditorWindow") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//ビュー
#define	GSTR_VIEWNAME		(_T("EditorClient")												_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         リソース                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	Dec. 2, 2002 genta
//	固定ファイル名
#define FN_APP_ICON  _T("my_appicon.ico")
#define FN_GREP_ICON _T("my_grepicon.ico")
#define FN_TOOL_BMP  _T("my_icons.bmp")

//	標準アプリケーションアイコンリソース名
#ifdef _DEBUG
	#define ICON_DEFAULT_APP IDI_ICON_DEBUG
	#define ICON_DEFAULT_GREP IDI_ICON_GREP
#else
	#define ICON_DEFAULT_APP IDI_ICON_STD
	#define ICON_DEFAULT_GREP IDI_ICON_GREP
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          フラグ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#define _SHIFT	0x00000001
#define _CTRL	0x00000002
#define _ALT	0x00000004



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        メッセージ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//wParam: X
//lParam: Y
#define MYWM_DOSPLIT        (WM_APP+1500)	

//wParam: なんかのインデックス
//lParam: boolっぽい何か
#define MYWM_SETACTIVEPANE  (WM_APP+1510)

//!設定が変更されたことの通知メッセージ
//wParam:未使用
//lParam:未使用
#define MYWM_CHANGESETTING  (WM_APP+1520)

//! タスクトレイからの通知メッセージ
#define MYWM_NOTIFYICON		(WM_APP+100)



/*! トレイからエディタへの終了要求 */
#define	MYWM_CLOSE			(WM_APP+200)

/*! トレイからエディタへの編集ファイル情報 要求通知 */
#define	MYWM_GETFILEINFO	(WM_APP+203)

/*! カーソル位置変更通知 */
#define	MYWM_SETCARETPOS	(WM_APP+204)
/// MYWM_SETCARETPOSメッセージのLPARAM
enum e_PM_SETCARETPOS_SELECTSTATE {
	PM_SETCARETPOS_NOSELECT		= 0, /*!<選択解除 */
	PM_SETCARETPOS_SELECT		= 1, /*!<選択開始・変更 */
	PM_SETCARETPOS_KEEPSELECT	= 2, /*!<現在の選択状態を保って移動 */
};

/*! カーソル位置取得要求 */
#define	MYWM_GETCARETPOS	(WM_APP+205)

//! テキスト追加通知(共有データ経由)
//wParam:未使用
//lParam:未使用
#define	MYWM_ADDSTRING		(WM_APP+206)

/*! タグジャンプ元通知 */
#define	MYWM_SETREFERER		(WM_APP+207)

/*! 行(改行単位)データの要求 */
#define	MYWM_GETLINEDATA	(WM_APP+208)


/*! 編集ウィンドウオブジェクトからのオブジェクト削除要求 */
#define	MYWM_DELETE_ME		(WM_APP+209)

/*! 新しい編集ウィンドウの作成依頼(コマンドラインを渡す) */
#define	MYWM_OPENNEWEDITOR	(WM_APP+210)

//ヘルプっぽい何か
#define	MYWM_HTMLHELP			(WM_APP+212)

/*! タブウインドウ用メッセージ */
#define	MYWM_TAB_WINDOW_NOTIFY	(WM_APP+213)	//@@@ 2003.05.31 MIK

/*! バーの表示・非表示変更メッセージ */
#define	MYWM_BAR_CHANGE_NOTIFY	(WM_APP+214)	//@@@ 2003.06.10 MIK

/*! エディタ－トレイ間でのUI特権分離の確認メッセージ */
#define	MYWM_UIPI_CHECK	(WM_APP+215)	//@@@ 2007.06.07 ryoji

/*! ポップアップウィンドウの表示切替指示 */
#define MYWM_SHOWOWNEDPOPUPS (WM_APP+216)	//@@@ 2007.10.22 ryoji

/*! プロセスの初回アイドリング通知 */
#define MYWM_FIRST_IDLE (WM_APP+217)	//@@@ 2008.04.19 ryoji

/*! 独自のドロップファイル通知 */
#define MYWM_DROPFILES (WM_APP+218)	//@@@ 2008.06.18 ryoji

/* 再変換対応 */ // 20020331 aroka
#ifndef WM_IME_REQUEST
#define MYWM_IME_REQUEST 0x288  // ==WM_IME_REQUEST
#else
#define MYWM_IME_REQUEST WM_IME_REQUEST
#endif
#define MSGNAME_ATOK_RECONVERT TEXT("Atok Message for ReconvertString")
#define RWM_RECONVERT TEXT("MSIMEReconvert")
