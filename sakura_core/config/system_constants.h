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
//                      ミューテックス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#define	GSTR_MUTEX_SAKURA					(_T("MutexSakuraEditor")				_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_MUTEX_SAKURA_CP				(_T("MutexSakuraEditorCP")				_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_MUTEX_SAKURA_INIT				(_T("MutexSakuraEditorInit")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))
#define	GSTR_MUTEX_SAKURA_EDITARR			(_T("MutexSakuraEditorEditArr")			_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//イベント
#define	GSTR_EVENT_SAKURA_CP_INITIALIZED	(_T("EventSakuraEditorCPInitialized")	_T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ウィンドウクラス                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//コントロールトレイ
#define	GSTR_CEDITAPP		(_T("CControlTray") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//メインウィンドウ
#define	GSTR_EDITWINDOWNAME	(_T("TextEditorWindow") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))

//ビュー
#define	GSTR_VIEWNAME		_T("EditorClient")


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        共有メモリ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#define	GSTR_CSHAREDATA	(_T("CShareData") _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_))


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
