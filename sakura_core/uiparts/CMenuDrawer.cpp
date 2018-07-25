/*!	@file
	@brief メニュー管理＆表示

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, Jepro
	Copyright (C) 2001, jepro, MIK, Misaka, YAZAKI, hor, genta
	Copyright (C) 2002, MIK, genta, YAZAKI, ai, Moca, hor, aroka
	Copyright (C) 2003, MIK, genta, Moca
	Copyright (C) 2004, Kazika, genta, Moca, isearch
	Copyright (C) 2005, genta, MIK, aroka
	Copyright (C) 2006, aroka, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CMenuDrawer.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "window/CSplitBoxWnd.h"
#include "CImageListMgr.h"
#include "func/CKeyBind.h"
#include "_os/COSVersionInfo.h"
#include "util/window.h"

// メニューアイコンの背景をボタンの色にする
#define DRAW_MENU_ICON_BACKGROUND_3DFACE

// メニューの選択色を淡くする
#define DRAW_MENU_SELECTION_LIGHT

// //! メニューアイコンを3Dボタンにする(旧仕様)
// //! 未定義なら、選択色との混合色とフラットな枠で選択を表現
// #define DRAW_MENU_ICON_3DBUTTON

// //! メニューのDISABLE/セパレータに影を落とす(旧仕様)
// #define DRAW_MENU_3DSTYLE


#if 0 // 未使用
void FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr)
{
//	ASSERT_VALID(this);
//	ASSERT(m_hDC != NULL);

	RECT rect;
	::SetBkColor( hdc, clr );
	::SetRect( &rect, x, y, x + cx, y + cy );
	::ExtTextOutW_AnyBuild( hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL );
}
#endif


//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CMenuDrawer::CMenuDrawer()
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = &GetDllShareData();

	m_hInstance = NULL;
	m_hWndOwner = NULL;
	m_nMenuHeight = 0;
	m_nMenuFontHeight = 0;
	m_hFontMenu = NULL;
	m_pcIcons = NULL;
	m_hCompBitmap = NULL;
	m_hCompDC = NULL;

//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。	/* ツールバーのボタン TBBUTTON構造体 */
	/* ツールバーのボタン TBBUTTON構造体 */
	/*
	typedef struct _TBBUTTON {
		int iBitmap;	// ボタン イメージの 0 から始まるインデックス
		int idCommand;	// ボタンが押されたときに送られるコマンド
		BYTE fsState;	// ボタンの状態--以下を参照
		BYTE fsStyle;	// ボタン スタイル--以下を参照
		DWORD dwData;	// アプリケーション-定義された値
		int iString;	// ボタンのラベル文字列の 0 から始まるインデックス
	} TBBUTTON;
	*/
//	キーワード：アイコン順序(アイコンインデックス)
//	Sept. 16, 2000 Jepro note: アイコン登録メニュー
//	以下の登録はツールバーだけでなくアイコンをもつすべてのメニューで利用されている
//	数字はビットマップリソースのIDB_MYTOOLに登録されているアイコンの先頭からの順番のようである
//	アイコンをもっと登録できるように横幅を16dotsx218=2048dotsに拡大
//	縦も15dotsから16dotsにして「プリンタ」アイコンや「ヘルプ1」の、下が欠けている部分を補ったが15dotsまでしか表示されないらしく効果なし
//	→
//	Sept. 17, 2000 縦16dot目を表示できるようにした
//	修正したファイルにはJEPRO_16thdotとコメントしてあるのでもし間違っていたらそれをキーワードにして検索してください(Sept. 28, 2000現在 6箇所変更)
//	IDB_MYTOOLの16dot目に見やすいように横16dotsづつの区切りになる目印を付けた
//
//	Sept. 16, 2000 見やすいように横に20個(あるいは32個)づつに配列しようとしたが配列構造を変えなければうまく格納できないので
//	それを解決するのが先決(→げんた氏改修版ur3β13で解決)
//
//	Sept. 16, 2000 JEPRO できるだけ系ごとに集まるように順番を大幅に入れ替えた  それに伴いCShareData.cppで設定している初期設定値も変更
//	Oct. 22, 2000 JEPRO アイコンのビットマップリソースの2次元配置が可能になったため根本的に配置転換した
//	・配置の基本は「コマンド一覧」に入っている機能(コマンド)順	なお「コマンド一覧」自体は「メニューバー」の順におおよそ準拠している
//	・アイコンビットマップファイルには横32個X15段ある(2010.06.26 13段から拡張)
//	・互換性と新コマンド追加の両立の都合で飛び地あり
//	・メニューに属する系および各系の段との関係は次の通り(2012.03.10 現在)：
//		ファイル----- ファイル操作系	(1段目32個: 1-32)
//		編集--------- 編集系			(2段目32個: 33-64)
//		移動--------- カーソル移動系	(3段目32個: 65-96)
//		選択--------- 選択系			(4段目32個: 97-128)
//					+ 矩形選択系		(5段目32個: 129-160) //(注. 矩形選択系のほとんどは未実装)
//					+ クリップボード系	(6段目24個: 161-184)
//			★挿入系					(6段目残りの8個: 185-192)
//		変換--------- 変換系			(7段目32個: 193-224)
//		検索--------- 検索系			(8段目32個: 225-256)
//		ツール------- モード切り替え系	(9段目4個: 257-260)
//					+ 設定系			(9段目次の16個: 261-276)
//					+ マクロ系			(9段目最後の11個: 277-287)
//					+ 外部マクロ		(12段目32個: 353-384/13段目19個: 385-403)
//					+ カスタムメニュー	(10段目25個: 289-313)
//		ウィンドウ--- ウィンドウ系		(11段目22個: 321-342)
//					+ タブ系			(10段目残りの7個: 314-320/9段目最期の1個: 288)
//		ヘルプ------- 支援				(11段目残りの10個: 343-352)
//	注1.「挿入系」はメニューでは「編集」に入っている
//	注2.「コマンド一覧」に入ってないコマンドもわかっている範囲で位置予約にしておいた
//  注3. F_DISABLE は未定義用(ダミーとしても使う)
//	注4. ユーザー用に確保された場所は特にないので各段の空いている後ろの方を使ってください。
//	注5. アイコンビットマップの有効段数は、CImageListMgr の MAX_Y です。

	static const int tbd[] = {
/* ファイル操作系(1段目32個: 1-32) */
/*  1 */		F_FILENEW					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//新規作成
/*  2 */		F_FILEOPEN					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//開く
/*  3 */		F_FILESAVE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//上書き保存
/*  4 */		F_FILESAVEAS_DIALOG			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//名前を付けて保存	//Sept. 18, 2000 JEPRO 追加
/*  5 */		F_FILECLOSE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
/*  6 */		F_FILECLOSE_OPEN			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//閉じて開く
/*  7 */		F_FILE_REOPEN_SJIS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJISで開き直す
/*  8 */		F_FILE_REOPEN_JIS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//JISで開き直す
/*  9 */		F_FILE_REOPEN_EUC			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//EUCで開き直す
/* 10 */		F_FILE_REOPEN_UNICODE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Unicodeで開き直す
/* 11 */		F_FILE_REOPEN_UTF8			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UTF-8で開き直す
/* 12 */		F_FILE_REOPEN_UTF7			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UTF-7で開き直す
/* 13 */		F_PRINT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//印刷
/* 14 */		F_PRINT_PREVIEW				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//印刷プレビュー
/* 15 */		F_PRINT_PAGESETUP			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//印刷ページ設定	//Sept. 21, 2000 JEPRO 追加
/* 16 */		F_OPEN_HfromtoC				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
/* 17 */		F_DISABLE	/*F_OPEN_HHPP*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更		del 2008/6/23 Uchi
/* 18 */		F_DISABLE	/*F_OPEN_CCPP*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更	del 2008/6/23 Uchi
/* 19 */		F_ACTIVATE_SQLPLUS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Oracle SQL*Plusをアクティブ表示 */	//Sept. 20, 2000 JEPRO 追加
/* 20 */		F_PLSQL_COMPILE_ON_SQLPLUS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Oracle SQL*Plusで実行 */	//Sept. 17, 2000 jepro 説明の「コンパイル」を「実行」に統一
/* 21 */		F_BROWSE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ブラウズ
/* 22 */		F_PROPERTY_FILE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ファイルのプロパティ//Sept. 16, 2000 JEPRO mytool1.bmpにあった「ファイルのプロパティ」アイコンをIDB_MYTOOLにコピー
/* 23 */		F_VIEWMODE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ビューモード
/* 24 */		F_FILE_REOPEN_UNICODEBE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UnicodeBEで開き直す // Moca, 2002/05/26 追加
/* 25 */		F_FILEOPEN_DROPDOWN			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//開く(ドロップダウン)
/* 26 */		F_FILE_REOPEN				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//開きなおす
/* 27 */		F_EXITALL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加
/* 28 */		F_FILESAVECLOSE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//保存して閉じる Feb. 28, 2004 genta
/* 29 */		F_FILENEW_NEWWINDOW			/* , TBSTATE_ENABLED, TBSTATE_BUTTON, 0, 0 */,	//新規ウインドウを開く 2009.06.17
/* 30 */		F_FILESAVEALL				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//全て上書き保存 Jan. 24, 2005 genta
/* 31 */		F_EXITALLEDITORS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//編集の全終了	// 2007.02.13 ryoji 追加
/* 32 */		F_FILE_REOPEN_CESU8			/* . TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,  //CESU-8で開きなおす

/* 編集系(2段目32個: 32-64) */
/* 33 */		F_UNDO							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//元に戻す(Undo)
/* 34 */		F_REDO							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//やり直し(Redo)
/* 35 */		F_DELETE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//削除
/* 36 */		F_DELETE_BACK					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カーソル前を削除
/* 37 */		F_WordDeleteToStart				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//単語の左端まで削除
/* 38 */		F_WordDeleteToEnd				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//単語の右端まで削除
/* 39 */		F_WordDelete					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//単語削除
/* 40 */		F_WordCut						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//単語切り取り
/* 41 */		F_LineDeleteToStart				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//行頭まで削除(改行単位)
/* 42 */		F_LineDeleteToEnd				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//行末まで削除(改行単位)
/* 43 */		F_LineCutToStart				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//行頭まで切り取り(改行単位)
/* 44 */		F_LineCutToEnd					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//行末まで切り取り(改行単位)
/* 45 */		F_DELETE_LINE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//行削除(折り返し単位)
/* 46 */		F_CUT_LINE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//行切り取り(改行単位)
/* 47 */		F_DUPLICATELINE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//行の二重化(折り返し単位)
/* 48 */		F_INDENT_TAB					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//TABインデント
/* 49 */		F_UNINDENT_TAB					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//逆TABインデント
/* 50 */		F_INDENT_SPACE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SPACEインデント
/* 51 */		F_UNINDENT_SPACE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//逆SPACEインデント
/* 52 */		F_DISABLE/*F_WORDSREFERENCE*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//単語リファレンス	//アイコン未作
/* 53 */		F_LTRIM							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//LTRIM		// 2001.12.03 hor
/* 54 */		F_RTRIM							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//RTRIM		// 2001.12.03 hor
/* 55 */		F_SORT_ASC						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SORT_ASC	// 2001.12.06 hor
/* 56 */		F_SORT_DESC						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SORT_DES	// 2001.12.06 hor
/* 57 */		F_MERGE							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//MERGE		// 2001.12.06 hor
/* 58 */		F_RECONVERT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//再変換	// 2002.4.12 YAZAKI
/* 59 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 60 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 61 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 62 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 63 */		F_PROFILEMGR					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//プロファイルマネージャ
/* 64 */		F_FILE_REOPEN_LATIN1			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Latin1で開きなおす	// 2010/3/20 Uchi

/* カーソル移動系(3段目32個: 65-96) */
/* 65 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 66 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 67 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 68 */		F_UP							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カーソル上移動
/* 69 */		F_DOWN							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カーソル下移動
/* 70 */		F_LEFT							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カーソル左移動
/* 71 */		F_RIGHT							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カーソル右移動
/* 72 */		F_UP2							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カーソル上移動(２行ごと)
/* 73 */		F_DOWN2							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カーソル下移動(２行ごと)
/* 74 */		F_WORDLEFT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//単語の左端に移動
/* 75 */		F_WORDRIGHT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//単語の右端に移動
/* 76 */		F_GOLINETOP						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//行頭に移動(折り返し単位)
/* 77 */		F_GOLINEEND						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//行末に移動(折り返し単位)
/* 78 */		F_HalfPageUp					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
/* 79 */		F_HalfPageDown					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
/* 80 */		F_1PageUp						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
/* 81 */		F_1PageDown						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
/* 82 */		F_DISABLE/*F_DISPLAYTOP*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//画面の先頭に移動(未実装)
/* 83 */		F_DISABLE/*F_DISPLAYEND*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//画面の最後に移動(未実装)
/* 84 */		F_GOFILETOP						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ファイルの先頭に移動
/* 85 */		F_GOFILEEND						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ファイルの最後に移動
/* 86 */		F_CURLINECENTER					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カーソル行をウィンドウ中央へ
/* 87 */		F_JUMPHIST_PREV					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//移動履歴: 前へ	//Sept. 28, 2000 JEPRO 追加
/* 88 */		F_JUMPHIST_NEXT					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//移動履歴: 次へ	//Sept. 28, 2000 JEPRO 追加
/* 89 */		F_WndScrollDown					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//テキストを１行下へスクロール	//Jun. 28, 2001 JEPRO 追加
/* 90 */		F_WndScrollUp					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//テキストを１行上へスクロール	//Jun. 28, 2001 JEPRO 追加
/* 91 */		F_GONEXTPARAGRAPH				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次の段落へ
/* 92 */		F_GOPREVPARAGRAPH				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前の段落へ
/* 93 */		F_JUMPHIST_SET					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//現在位置を移動履歴に登録
/* 94 */		F_MODIFYLINE_PREV				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前の変更行へ
/* 95 */		F_MODIFYLINE_NEXT				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次の変更行へ
/* 96 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* 選択系(4段目32個: 97-128) */
/* 97 */		F_SELECTWORD					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//現在位置の単語選択
/* 98 */		F_SELECTALL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//すべて選択	//Sept. 21, 2000 JEPRO 追加
/* 99 */		F_BEGIN_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//範囲選択開始
/* 100 */		F_UP_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)カーソル上移動
/* 101 */		F_DOWN_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)カーソル下移動
/* 102 */		F_LEFT_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)カーソル左移動
/* 103 */		F_RIGHT_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)カーソル右移動
/* 104 */		F_UP2_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)カーソル上移動(２行ごと)
/* 105 */		F_DOWN2_SEL						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)カーソル下移動(２行ごと)
/* 106 */		F_WORDLEFT_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)単語の左端に移動
/* 107 */		F_WORDRIGHT_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)単語の右端に移動
/* 108 */		F_GOLINETOP_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)行頭に移動(折り返し単位)
/* 109 */		F_GOLINEEND_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)行末に移動(折り返し単位)
/* 110 */		F_HalfPageUp_Sel				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
/* 111 */		F_HalfPageDown_Sel				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
/* 112 */		F_1PageUp_Sel					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
/* 113 */		F_1PageDown_Sel					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
/* 114 */		F_DISABLE/*F_DISPLAYTOP_SEL*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)画面の先頭に移動(未実装)
/* 115 */		F_DISABLE/*F_DISPLAYEND_SEL*/	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)画面の最後に移動(未実装)
/* 116 */		F_GOFILETOP_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)ファイルの先頭に移動
/* 117 */		F_GOFILEEND_SEL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)ファイルの最後に移動
/* 118 */		F_GONEXTPARAGRAPH_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)次の段落へ
/* 119 */		F_GOPREVPARAGRAPH_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)前の段落へ
/* 120 */		F_SELECTLINE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//1行選択
/* 121 */		F_FUNCLIST_PREV					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前の関数リストマーク
/* 122 */		F_FUNCLIST_NEXT					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次の関数リストマーク
/* 123 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 124 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 125 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 126 */		F_MODIFYLINE_PREV_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)前の変更行へ
/* 127 */		F_MODIFYLINE_NEXT_SEL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(範囲選択)次の変更行へ
/* 128 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* 矩形選択系(5段目32個: 129-160) */ //(注. 矩形選択系のほとんどは未実装)
/* 129 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 130 */		F_DISABLE/*F_BOXSELALL*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//矩形ですべて選択
/* 131 */		F_BEGIN_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//矩形範囲選択開始	//Sept. 29, 2000 JEPRO 追加
/* 132 */		F_UP_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル上移動
/* 133 */		F_DOWN_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル下移動
/* 134 */		F_LEFT_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル左移動
/* 135 */		F_RIGHT_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル右移動
/* 136 */		F_UP2_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル上移動(２行ごと)
/* 137 */		F_DOWN2_BOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル下移動(２行ごと)
/* 138 */		F_WORDLEFT_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)単語の左端に移動
/* 139 */		F_WORDRIGHT_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)単語の右端に移動
/* 140 */		F_GOLINETOP_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)行頭に移動(折り返し単位)
/* 141 */		F_GOLINEEND_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)行末に移動(折り返し単位)
/* 142 */		F_HalfPageUp_BOX				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)半ページアップ
/* 143 */		F_HalfPageDown_BOX				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)半ページダウン
/* 144 */		F_1PageUp_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)１ページアップ
/* 145 */		F_1PageDown_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)１ページダウン
/* 146 */		F_DISABLE/*F_DISPLAYTOP_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)画面の先頭に移動(未実装)
/* 147 */		F_DISABLE/*F_DISPLAYEND_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)画面の最後に移動(未実装)
/* 148 */		F_GOFILETOP_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)ファイルの先頭に移動
/* 149 */		F_GOFILEEND_BOX					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)ファイルの最後に移動
/* 150 */		F_GOLOGICALLINETOP_BOX			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)行頭に移動(改行単位)
/* 151 */		F_DISABLE/*F_GOLOGICALLINEEND_BOX*/		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 152 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 153 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 154 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 155 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 156 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 157 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 158 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 159 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 160 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* クリップボード系(6段目24個: 161-184) */
/* 161 */		F_CUT							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//切り取り(選択範囲をクリップボードにコピーして削除)
/* 162 */		F_COPY							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//コピー(選択範囲をクリップボードにコピー)
/* 163 */		F_COPY_CRLF						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//CRLF改行でコピー
/* 164 */		F_PASTE							/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//貼り付け(クリップボードから貼り付け)
/* 165 */		F_PASTEBOX						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//矩形貼り付け(クリップボードから貼り付け)
/* 166 */		F_DISABLE/*F_INSTEXT_W*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//テキストを貼り付け	(未公開コマンド？未完成？)
/* 167 */		F_DISABLE/*F_ADDTAIL_W*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//最後にテキストを追加	(未公開コマンド？未完成？)
/* 168 */		F_COPYLINES						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//選択範囲内全行コピー	//Sept. 30, 2000 JEPRO 追加
/* 169 */		F_COPYLINESASPASSAGE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//選択範囲内全行引用符付きコピー	//Sept. 30, 2000 JEPRO 追加
/* 170 */		F_COPYLINESWITHLINENUMBER		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//選択範囲内全行行番号付きコピー	//Sept. 30, 2000 JEPRO 追加
/* 171 */		F_COPYPATH						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//このファイルのパス名をコピー //added Oct. 22, 2000 JEPRO				//Nov. 5, 2000 JEPRO 追加
/* 172 */		F_COPYTAG						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//このファイルのパス名とカーソル位置をコピー //added Oct. 22, 2000 JEPRO	//Nov. 5, 2000 JEPRO 追加
/* 173 */		F_CREATEKEYBINDLIST				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キー割り当て一覧をコピー //added Oct. 22, 2000 JEPRO	//Dec. 25, 2000 JEPRO アイコン追加
/* 174 */		F_COPYFNAME						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//このファイル名をクリップボードにコピー //2002/2/3 aroka
/* 175 */		F_COPY_ADDCRLF					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//折り返し位置に改行をつけてコピー
/* 176 */		F_COPY_COLOR_HTML				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//選択範囲内色付きHTMLコピー
/* 177 */		F_COPY_COLOR_HTML_LINENUMBER	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//選択範囲内行番号色付きHTMLコピー
/* 178 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 179 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 180 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 181 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 182 */		F_CHGMOD_EOL_CRLF,	// 2003.06.23 Moca
/* 183 */		F_CHGMOD_EOL_LF,	// 2003.06.23 Moca
/* 184 */		F_CHGMOD_EOL_CR,	// 2003.06.23 Moca

/* 挿入系(6段目残り8個: 185-192) */
/* 185 */		F_INS_DATE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//日付挿入	//Nov. 5, 2000 JEPRO 追加
/* 186 */		F_INS_TIME	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//時刻挿入	//Nov. 5, 2000 JEPRO 追加
/* 187 */		F_CTRL_CODE_DIALOG	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//コントロールコードの入力(ダイアログ)	//@@@ 2002.06.02 MIK
/* 188 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 189 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 190 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 191 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 192 */		F_DISABLE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* 変換系(7段目32個: 193-224) */
/* 193 */		F_TOLOWER				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//小文字
/* 194 */		F_TOUPPER				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//大文字
/* 195 */		F_TOHANKAKU				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//全角→半角
/* 196 */		F_TOZENKAKUKATA			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//半角＋全ひら→全角・カタカナ	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
/* 197 */		F_TOZENKAKUHIRA			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//半角＋全カタ→全角・ひらがな	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
/* 198 */		F_HANKATATOZENKATA		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//半角カタカナ→全角カタカナ	//Sept. 18, 2000 JEPRO 追加
/* 199 */		F_HANKATATOZENHIRA		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//半角カタカナ→全角ひらがな	//Sept. 18, 2000 JEPRO 追加
/* 200 */		F_TABTOSPACE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//TAB→空白	//Sept. 20, 2000 JEPRO 追加
/* 201 */		F_CODECNV_AUTO2SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//自動判別→SJISコード変換
/* 202 */		F_CODECNV_EMAIL			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//E-Mail(JIS→SIJIS)コード変換
/* 203 */		F_CODECNV_EUC2SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//EUC→SJISコード変換
/* 204 */		F_CODECNV_UNICODE2SJIS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Unicode→SJISコード変換
/* 205 */		F_CODECNV_UTF82SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UTF-8→SJISコード変換
/* 206 */		F_CODECNV_UTF72SJIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UTF-7→SJISコード変換
/* 207 */		F_CODECNV_SJIS2JIS		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJIS→JISコード変換
/* 208 */		F_CODECNV_SJIS2EUC		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJIS→EUCコード変換
/* 209 */		F_CODECNV_SJIS2UTF8		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJIS→UTF-8コード変換
/* 210 */		F_CODECNV_SJIS2UTF7		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//SJIS→UTF-7コード変換
/* 211 */		F_BASE64DECODE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Base64デコードして保存	//Sept. 28, 2000 JEPRO 追加
/* 212 */		F_UUDECODE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//uudecodeしてファイルに保存//Sept. 28, 2000 JEPRO 追加	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更
/* 213 */		F_SPACETOTAB			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//空白→TAB	//Jun. 01, 2001 JEPRO 追加
/* 214 */		F_TOZENEI				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//半角英数→全角英数 //July. 30, 2001 Misaka 追加
/* 215 */		F_TOHANEI				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//全角英数→半角英数
/* 216 */		F_CODECNV_UNICODEBE2SJIS/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UnicodeBE→SJISコード変換	//Moca, 2002/05/26
/* 217 */		F_TOHANKATA				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//全角カタカナ→半角カタカナ	//Aug. 29, 2002 ai
/* 218 */		F_FILETREE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ファイルツリー表示
/* 219 */		F_SHOWMINIMAP			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ミニマップを表示
/* 220 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 221 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 222 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 223 */		F_TAGJUMP_CLOSE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//閉じてタグジャンプ(元ウィンドウclose)
/* 224 */		F_OUTLINE_TOGGLE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//アウトライン解析(toggle) // 20060201 aroka

/* 検索系(8段目32個: 225-256) */
/* 225 */		F_SEARCH_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//検索(単語検索ダイアログ)
/* 226 */		F_SEARCH_NEXT		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次を検索
/* 227 */		F_SEARCH_PREV		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前を検索
/* 228 */		F_REPLACE_DIALOG	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//置換	//Sept. 21, 2000 JEPRO 追加
/* 229 */		F_SEARCH_CLEARMARK	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//検索マークのクリア
/* 230 */		F_GREP_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Grep
/* 231 */		F_JUMP_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//指定行へジャンプ		//Sept. 21, 2000 JEPRO 追加
/* 232 */		F_OUTLINE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//アウトライン解析
/* 233 */		F_TAGJUMP			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タグジャンプ機能			//Sept. 21, 2000 JEPRO 追加
/* 234 */		F_TAGJUMPBACK		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タグジャンプバック機能	//Sept. 21, 2000 JEPRO 追加
/* 235 */		F_COMPARE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ファイル内容比較	//Sept. 21, 2000 JEPRO 追加
/* 236 */		F_BRACKETPAIR		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//対括弧の検索	//Sept. 20, 2000 JEPRO 追加
/* 237 */		F_BOOKMARK_SET		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ブックマーク設定・解除	// 2001.12.03 hor
/* 238 */		F_BOOKMARK_NEXT		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次のブックマークへ		// 2001.12.03 hor
/* 239 */		F_BOOKMARK_PREV		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前のブックマークへ		// 2001.12.03 hor
/* 240 */		F_BOOKMARK_RESET	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ブックマークの全解除		// 2001.12.03 hor
/* 241 */		F_BOOKMARK_VIEW		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ブックマークの一覧		// 2001.12.03 hor
/* 242 */		F_DIFF_DIALOG		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//DIFF差分表示	//@@@ 2002.05.25 MIK
/* 243 */		F_DIFF_NEXT			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次の差分へ	//@@@ 2002.05.25 MIK
/* 244 */		F_DIFF_PREV			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前の差分へ	//@@@ 2002.05.25 MIK
/* 245 */		F_DIFF_RESET		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//差分の全解除	//@@@ 2002.05.25 MIK
/* 246 */		F_SEARCH_BOX		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//検索(ボックス)
/* 247 */		F_JUMP_SRCHSTARTPOS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//検索開始位置へ戻る	// 02/06/26 ai
/* 248 */		F_TAGS_MAKE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タグファイルの作成	//@@@ 2003.04.13 MIK
/* 249 */		F_DIRECT_TAGJUMP	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダイレクトタグジャンプ	//@@@ 2003.04.15 MIK
/* 250 */		F_ISEARCH_NEXT		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前方インクリメンタルサーチ //2004.10.13 isearch
/* 251 */		F_ISEARCH_PREV		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//後方インクリメンタルサーチ //2004.10.13 isearch
/* 252 */		F_ISEARCH_REGEXP_NEXT	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//正規表現前方インクリメンタルサーチ //2004.10.13 isearch
/* 253 */		F_ISEARCH_REGEXP_PREV	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//正規表現前方インクリメンタルサーチ //2004.10.13 isearch
/* 254 */		F_ISEARCH_MIGEMO_NEXT	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//MIGEMO前方インクリメンタルサーチ //2004.10.13 isearch
/* 255 */		F_ISEARCH_MIGEMO_PREV	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//MIGEMO前方インクリメンタルサーチ //2004.10.13 isearch
/* 256 */		F_TAGJUMP_KEYWORD	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーワードを指定してダイレクトタグジャンプ //2005.03.31 MIK

/* モード切り替え系(9段目4個: 257-260) */
/* 257 */		F_CHGMOD_INS	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//挿入／上書きモード切り替え	//Nov. 5, 2000 JEPRO 追加
/* 258 */		F_CANCEL_MODE	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//各種モードの取り消し			//Nov. 7, 2000 JEPRO 追加
/* 259 */		F_CHG_CHARSET	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//文字コードセット指定			//2010/6/14 Uchi
/* 260 */		F_GREP_REPLACE_DLG	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Grep置換

/* 設定系(9段目次の16個: 261-276) */
/* 261 */		F_SHOWTOOLBAR		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ツールバーの表示
/* 262 */		F_SHOWFUNCKEY		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ファンクションキーの表示
/* 263 */		F_SHOWSTATUSBAR		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ステータスバーの表示
/* 264 */		F_TYPE_LIST			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タイプ別設定一覧	//Sept. 18, 2000 JEPRO 追加
/* 265 */		F_OPTION_TYPE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タイプ別設定
/* 266 */		F_OPTION			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//共通設定			//Sept. 16, 2000 jepro 説明を「設定プロパティシート」から変更
/* 267 */		F_FONT				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//フォント設定
/* 268 */		F_WRAPWINDOWWIDTH	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//現在のウィンドウ幅で折り返し	//	Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
/* 269 */		F_FAVORITE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//履歴の管理	//@@@ 2003.04.08 MIK
/* 270 */		F_SHOWTAB			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タブの表示	//@@@ 2003.06.10 MIK
/* 271 */		F_DISABLE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 272 */		F_TOGGLE_KEY_SEARCH	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーワードヘルプ自動表示 2007.03.92 genta
/* 273 */		F_TMPWRAPNOWRAP		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//折り返さない（一時設定）			// 2008.05.30 nasukoji
/* 274 */		F_TMPWRAPSETTING	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//指定桁で折り返す（一時設定）		// 2008.05.30 nasukoji
/* 275 */		F_TMPWRAPWINDOW		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//右端で折り返す（一時設定）		// 2008.05.30 nasukoji
/* 276 */		F_SELECT_COUNT_MODE	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//文字カウント方法	//2009.07.06 syat

/* マクロ系(9段目最後の12個: 277-288) */
/* 277 */		F_RECKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーマクロの記録開始／終了
/* 278 */		F_SAVEKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーマクロの保存		//Sept. 21, 2000 JEPRO 追加
/* 279 */		F_LOADKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーマクロの読み込み	//Sept. 21, 2000 JEPRO 追加
/* 280 */		F_EXECKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーマクロの実行		//Sept. 16, 2000 JEPRO 下から上に移動した
/* 281 */		F_EXECMD_DIALOG			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部コマンド実行//Sept. 20, 2000 JEPRO 名称をCMMANDからCOMMANDに変更(EXECCMMAND→EXECCMMAND)
/* 282 */		F_EXECEXTMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//名前を指定してマクロ実行	//2008.10.22 syat 追加
/* 283 */		F_PLUGCOMMAND			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//プラグインコマンド用に予約
/* 284 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 285 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 286 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 287 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 288 */		F_TAB_CLOSEOTHER		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//このタブ以外を閉じる		// 2009.07.07 syat,ウィンドウ系に空きがないので上の行を侵食

/* カスタムメニュー(10段目25個: 289-313) */
/* 289 */		F_MENU_RBUTTON				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//右クリックメニュー 	//Sept. 30, 2000 JEPRO 追加
/* 290 */		F_CUSTMENU_1				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー1
/* 291 */		F_CUSTMENU_2				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー2
/* 292 */		F_CUSTMENU_3				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー3
/* 293 */		F_CUSTMENU_4				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー4
/* 294 */		F_CUSTMENU_5				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー5
/* 295 */		F_CUSTMENU_6				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー6
/* 296 */		F_CUSTMENU_7				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー7
/* 297 */		F_CUSTMENU_8				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー8
/* 298 */		F_CUSTMENU_9				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー9
/* 299 */		F_CUSTMENU_10				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー10
/* 300 */		F_CUSTMENU_11				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー11
/* 301 */		F_CUSTMENU_12				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー12
/* 302 */		F_CUSTMENU_13				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー13
/* 303 */		F_CUSTMENU_14				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー14
/* 304 */		F_CUSTMENU_15				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー15
/* 305 */		F_CUSTMENU_16				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー16
/* 306 */		F_CUSTMENU_17				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー17
/* 307 */		F_CUSTMENU_18				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー18
/* 308 */		F_CUSTMENU_19				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー19
/* 309 */		F_CUSTMENU_20				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー20
/* 310 */		F_CUSTMENU_21				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー21
/* 311 */		F_CUSTMENU_22				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー22
/* 312 */		F_CUSTMENU_23				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー23
/* 313 */		F_CUSTMENU_24				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー24

/* ウィンドウ系(10段目7個: 314-320) */
/* 314 */		F_TAB_MOVERIGHT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タブを右に移動	// 2007.06.20 ryoji
/* 315 */		F_TAB_MOVELEFT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タブを左に移動	// 2007.06.20 ryoji
/* 316 */		F_TAB_SEPARATE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//新規グループ	// 2007.06.20 ryoji
/* 317 */		F_TAB_JOINTNEXT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次のグループに移動	// 2007.06.20 ryoji
/* 318 */		F_TAB_JOINTPREV				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前のグループに移動	// 2007.06.20 ryoji
/* 319 */		F_TAB_CLOSERIGHT			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//右をすべて閉じる		// 2009.07.07 syat
/* 320 */		F_TAB_CLOSELEFT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//左をすべて閉じる		// 2009.07.07 syat

/* ウィンドウ系(11段目22個: 321-342) */
/* 321 */		F_SPLIT_V		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//上下に分割	//Sept. 16, 2000 jepro 説明を「縦」から「上下に」に変更
/* 322 */		F_SPLIT_H		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//左右に分割	//Sept. 16, 2000 jepro 説明を「横」から「左右に」に変更
/* 323 */		F_SPLIT_VH		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
/* 324 */		F_WINCLOSE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ウィンドウを閉じる
/* 325 */		F_WIN_CLOSEALL	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//すべてのウィンドウを閉じる	//Sept. 18, 2000 JEPRO 追加	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
/* 329 */		F_NEXTWINDOW	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次のウィンドウ
/* 330 */		F_PREVWINDOW	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前のウィンドウ
/* 326 */		F_CASCADE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//重ねて表示
/* 237 */		F_TILE_V		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//上下に並べて表示
/* 328 */		F_TILE_H		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//左右に並べて表示
/* 331 */		F_MAXIMIZE_V	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//縦方向に最大化
/* 332 */		F_MAXIMIZE_H	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//横方向に最大化 //2001.02.10 by MIK
/* 333 */		F_MINIMIZE_ALL	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//すべて最小化					//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
/* 334 */		F_REDRAW		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//再描画						//Sept. 30, 2000 JEPRO 追加
/* 335 */		F_WIN_OUTPUT	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//アウトプットウィンドウ表示	//Sept. 18, 2000 JEPRO 追加
/* 336 */		F_BIND_WINDOW	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//結合して表示	2004.07.14 kazika / Design Oct. 1, 2004 genta
/* 337 */		F_TOPMOST		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//常に手前に表示 2004.09.21 Moca
/* 338 */		F_DLGWINLIST	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ウィンドウ一覧表示
/* 339 */		F_WINLIST		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ウィンドウ一覧ポップアップ表示	// 2006.03.23 fon
/* 340 */		F_GROUPCLOSE	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//グループを閉じる	// 2007.06.20 ryoji
/* 341 */		F_NEXTGROUP		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次のグループ	// 2007.06.20 ryoji
/* 342 */		F_PREVGROUP		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前のグループ	// 2007.06.20 ryoji

/* 支援(11段目残りの10個: 343-352) */
/* 343 */		F_HOKAN			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//入力補完
/* 344 */		F_HELP_CONTENTS /*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ヘルプ目次			//Nov. 25, 2000 JEPRO 追加
/* 345 */		F_HELP_SEARCH	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ヘルプキーワード検索	//Nov. 25, 2000 JEPRO 追加
/* 346 */		F_MENU_ALLFUNC	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//コマンド一覧			//Sept. 30, 2000 JEPRO 追加
/* 347 */		F_EXTHELP1		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部ヘルプ１
/* 348 */		F_EXTHTMLHELP	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部HTMLヘルプ
/* 349 */		F_ABOUT			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//バージョン情報	//Dec. 24, 2000 JEPRO 追加
/* 350 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 351 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 352 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

//2002.01.17 hor ｢その他｣のエリアを外部マクロ用に割当て
/* 外部マクロ(12段目31個: 353-383) */
/* 353 */		F_USERMACRO_0+0		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ①
/* 354 */		F_USERMACRO_0+1		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ②
/* 355 */		F_USERMACRO_0+2		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ③
/* 356 */		F_USERMACRO_0+3		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ④
/* 357 */		F_USERMACRO_0+4		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑤
/* 358 */		F_USERMACRO_0+5		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑥
/* 359 */		F_USERMACRO_0+6		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑦
/* 360 */		F_USERMACRO_0+7		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑧
/* 361 */		F_USERMACRO_0+8		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑨
/* 362 */		F_USERMACRO_0+9		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑩
/* 363 */		F_USERMACRO_0+10	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑪
/* 364 */		F_USERMACRO_0+11	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑫
/* 365 */		F_USERMACRO_0+12	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑬
/* 366 */		F_USERMACRO_0+13	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑭
/* 367 */		F_USERMACRO_0+14	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑮
/* 368 */		F_USERMACRO_0+15	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑯
/* 369 */		F_USERMACRO_0+16	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑰
/* 370 */		F_USERMACRO_0+17	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑱
/* 371 */		F_USERMACRO_0+18	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑲
/* 372 */		F_USERMACRO_0+19	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ⑳
/* 373 */		F_USERMACRO_0+20	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ21
/* 374 */		F_USERMACRO_0+21	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ22
/* 375 */		F_USERMACRO_0+22	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ23
/* 376 */		F_USERMACRO_0+23	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ24
/* 377 */		F_USERMACRO_0+24	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ25
/* 378 */		F_USERMACRO_0+25	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ26
/* 379 */		F_USERMACRO_0+26	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ27
/* 380 */		F_USERMACRO_0+27	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ28
/* 381 */		F_USERMACRO_0+28	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ29
/* 382 */		F_USERMACRO_0+29	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ30
/* 383 */		F_USERMACRO_0+30	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ31
//	2007.10.17 genta 384は折り返しマークとして使用しているのでアイコンとしては使用できない
//	2010.06.23 アイコン位置のみ追加マクロ用として利用する
/* 384 */		F_TOOLBARWRAP		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//追加マクロ用icon位置兼、折返ツールバーボタンID

/* 外部マクロ(13段目19個: 385-403) */
/* 385 */		F_USERMACRO_0+31	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ32
/* 386 */		F_USERMACRO_0+32	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ33
/* 387 */		F_USERMACRO_0+33	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ34
/* 388 */		F_USERMACRO_0+34	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ35
/* 389 */		F_USERMACRO_0+35	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ36
/* 390 */		F_USERMACRO_0+36	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ37
/* 391 */		F_USERMACRO_0+37	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ38
/* 392 */		F_USERMACRO_0+38	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ39
/* 393 */		F_USERMACRO_0+39	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ40
/* 394 */		F_USERMACRO_0+40	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ41
/* 395 */		F_USERMACRO_0+41	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ42
/* 396 */		F_USERMACRO_0+42	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ43
/* 397 */		F_USERMACRO_0+43	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ44
/* 398 */		F_USERMACRO_0+44	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ45
/* 399 */		F_USERMACRO_0+45	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ46
/* 400 */		F_USERMACRO_0+46	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ47
/* 401 */		F_USERMACRO_0+47	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ48
/* 402 */		F_USERMACRO_0+48	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ49
/* 403 */		F_USERMACRO_0+49	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ50
/* 404 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 405 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 406 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 407 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 408 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 409 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 410 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 411 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 412 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 413 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 414 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 415 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 416 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* 417 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 418 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 419 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 420 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 421 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 422 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 423 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 424 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 425 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 426 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 427 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 428 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 429 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 430 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 431 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 432 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 433 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 434 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 435 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 436 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 437 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 438 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 439 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 440 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 441 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 442 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 443 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 444 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 445 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 446 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 447 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 448 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* 449 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 450 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 451 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 452 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 453 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 454 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 455 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 456 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 457 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 458 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 459 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 460 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 461 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 462 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 463 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 464 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 465 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 466 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 467 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 468 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 469 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 470 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 471 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 472 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 473 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 474 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 475 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 476 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 477 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 478 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 479 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 480 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* 481 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */	//最終行用ダミー(Jepro note: 最終行末にはカンマを付けないこと)

	};
	int tbd_num = _countof( tbd );

	// m_tbMyButton[0]にはセパレータが入っているため、アイコン番号とボタン番号は１つずれる
	const int INDEX_GAP = 1;
	const int myButtonEnd = tbd_num + INDEX_GAP;
	// 定数の整合性確認
	// アイコン番号
	assert_warning( tbd[TOOLBAR_ICON_MACRO_INTERNAL      - INDEX_GAP] == F_MACRO_EXTRA );
	assert_warning( tbd[TOOLBAR_ICON_PLUGCOMMAND_DEFAULT - INDEX_GAP] == F_PLUGCOMMAND );
	// コマンド番号
	assert_warning( tbd[TOOLBAR_BUTTON_F_TOOLBARWRAP     - INDEX_GAP] == F_TOOLBARWRAP );
	m_tbMyButton.resize( tbd_num + INDEX_GAP );
	SetTBBUTTONVal( &m_tbMyButton[0], -1, F_SEPARATOR, 0, TBSTYLE_SEP, 0, 0 );	//セパレータ	// 2007.11.02 ryoji アイコンの未定義化(-1)

	// 2010.06.23 Moca ループインデックスの基準をm_tbMyButtonに変更
	for( int i = INDEX_GAP; i < myButtonEnd; i++ ){
		const int funcCode = tbd[i-INDEX_GAP];
		const int imageIndex = i - INDEX_GAP;

		if( funcCode == F_TOOLBARWRAP ){
			// ツールバー改行用の仮想ボタン（実際は表示されない） // 20050809 aroka
			//	2007.10.12 genta 折り返しボタンが最後のデータと重なっているが，
			//	インデックスを変更するとsakura.iniが引き継げなくなるので
			//	重複を承知でそのままにする
			//	2010.06.23 アイコン位置は外部マクロのデフォルトアイコンとして利用中
			//	m_tbMyButton[384]自体は、ツールバーの折り返し用
			SetTBBUTTONVal(
				&m_tbMyButton[i],
				-1,						// 2007.11.02 ryoji アイコンの未定義化(-1)
				F_MENU_NOT_USED_FIRST,
				TBSTATE_ENABLED|TBSTATE_WRAP,
				TBSTYLE_SEP, 0, 0
			);
			continue;
		}

		BYTE	style;	//@@@ 2002.06.15 MIK
		switch( funcCode )	//@@@ 2002.06.15 MIK
		{
		case F_FILEOPEN_DROPDOWN:
			style = TBSTYLE_DROPDOWN;	//ドロップダウン
			break;

		case F_SEARCH_BOX:
			style = TBSTYLE_COMBOBOX;	//コンボボックス
			break;

		default:
			style = TBSTYLE_BUTTON;	//ボタン
			break;
		}

		SetTBBUTTONVal(
			&m_tbMyButton[i],
			(F_DUMMY_MAX_CODE < funcCode)? imageIndex : -1,	// 2007.11.02 ryoji アイコンの未定義化(-1)
			funcCode,
			(tbd[i] == F_DISABLE)? 0 : TBSTATE_ENABLED,	// F_DISABLE なら DISABLEに	2010/7/11 Uchi
			style, 0, 0
		);
	}

	m_nMyButtonFixSize = m_tbMyButton.size();
	
	// 2010.06.25 Moca 専用アイコンのない外部マクロがあれば、同じアイコンを共有して登録
	if( MAX_CUSTMACRO_ICO < MAX_CUSTMACRO ){
		const int nAddFuncs = MAX_CUSTMACRO - MAX_CUSTMACRO_ICO;
		const int nBaseIndex = m_tbMyButton.size();
		m_tbMyButton.resize( m_tbMyButton.size() + nAddFuncs );
		for( int k = 0; k < nAddFuncs; k++ ){
			const int macroFuncCode = F_USERMACRO_0 + MAX_CUSTMACRO_ICO + k;
			SetTBBUTTONVal(
				&m_tbMyButton[k + nBaseIndex],
				TOOLBAR_ICON_MACRO_INTERNAL - INDEX_GAP,
				macroFuncCode, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0
			);
		}
	}
	
	m_nMyButtonNum = m_tbMyButton.size();
	return;
}


CMenuDrawer::~CMenuDrawer()
{
	if( NULL != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = NULL;
	}
	DeleteCompDC();
	return;
}

void CMenuDrawer::Create( HINSTANCE hInstance, HWND hWndOwner, CImageListMgr* pcIcons )
{
	m_hInstance = hInstance;
	m_hWndOwner = hWndOwner;
	m_pcIcons = pcIcons;

	return;
}


void CMenuDrawer::ResetContents( void )
{
	LOGFONT	lf;
	m_menuItems.clear();

	NONCLIENTMETRICS	ncm;
	memset_raw(&ncm, 0, sizeof(ncm));

	// 以前のプラットフォームに WINVER >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する	// 2007.12.21 ryoji
	ncm.cbSize = CCSIZEOF_STRUCT( NONCLIENTMETRICS, lfMessageFont );
	::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0 );

	if( NULL != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = NULL;
	}
	lf = ncm.lfMenuFont;
	m_hFontMenu = ::CreateFontIndirect( &lf );
	m_nMenuFontHeight = lf.lfHeight;
	if( m_nMenuFontHeight < 0 ){
		m_nMenuFontHeight = -m_nMenuFontHeight;
	}else{
		// ポイント(1/72インチ)をピクセルへ
		m_nMenuFontHeight = DpiScaleY(m_nMenuFontHeight);
		if( -1 == m_nMenuFontHeight ){
			m_nMenuFontHeight = lf.lfHeight;
		}
	}
	m_nMenuHeight = m_nMenuFontHeight + 4; // margin
	if( m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
		// 最低アイコン分の高さを確保
		if( 20 > m_nMenuHeight ){
			m_nMenuHeight = 20;
		}
	}

//@@@ 2002.01.03 YAZAKI 不使用のため
//	m_nMaxTab = 0;
//	m_nMaxTabLen = 0;
	return;
}




/* メニュー項目を追加 */
void CMenuDrawer::MyAppendMenu(
	HMENU			hMenu,
	int				nFlag,
	UINT_PTR		nFuncId,
	const TCHAR*	pszLabel,
	const TCHAR*	pszKey,			// 2010/5/18 Uchi
	BOOL			bAddKeyStr,
	int				nForceIconId	//お気に入り	//@@@ 2003.04.08 MIK
)
{
	TCHAR		szLabel[_MAX_PATH * 2+ 30];
	TCHAR		szKey[10];
	int			nFlagAdd = 0;

	if( nForceIconId == -1 ) nForceIconId = nFuncId;	//お気に入り	//@@@ 2003.04.08 MIK

	szLabel[0] = _T('\0');
	if( NULL != pszLabel ){
		_tcsncpy( szLabel, pszLabel, _countof( szLabel ) - 1 );
		szLabel[ _countof( szLabel ) - 1 ] = _T('\0');
	}
	auto_strcpy( szKey, pszKey); 
	if( nFuncId != 0 ){
		/* メニューラベルの作成 */
		CKeyBind::GetMenuLabel(
			m_hInstance,
			m_pShareData->m_Common.m_sKeyBind.m_nKeyNameArrNum,
			m_pShareData->m_Common.m_sKeyBind.m_pKeyNameArr,
			nFuncId,
			szLabel,
			szKey,
			bAddKeyStr,
			_countof(szLabel)
		 );

		/* アイコン用ビットマップを持つものは、オーナードロウにする */
		{
			MyMenuItemInfo item;
			item.m_nBitmapIdx = -1;
			item.m_nFuncId = nFuncId;
			item.m_cmemLabel.SetString( szLabel );
			// メニュー項目をオーナー描画にして、アイコンを表示する
			// 2010.03.29 アクセスキーの分を詰めるためいつもオーナードローにする。ただしVista未満限定
			// Vista以上ではメニューもテーマが適用されるので、オーナードローにすると見た目がXP風になってしまう。
			if( m_pShareData->m_Common.m_sWindow.m_bMenuIcon || !IsWinVista_or_later() ){
				nFlagAdd = MF_OWNERDRAW;
			}
			/* 機能のビットマップの情報を覚えておく */
			item.m_nBitmapIdx = GetIconIdByFuncId( nForceIconId );
			m_menuItems.push_back( item );
		}
	}else{
#ifdef DRAW_MENU_ICON_BACKGROUND_3DFACE
		// セパレータかサブメニュー
		if( nFlag & (MF_SEPARATOR | MF_POPUP) ){
			if( m_pShareData->m_Common.m_sWindow.m_bMenuIcon || !IsWinVista_or_later() ){
					nFlagAdd = MF_OWNERDRAW;
			}
		}
#endif
	}

	MENUITEMINFO mii;
	memset_raw( &mii, 0, sizeof( mii ) );
	//	Aug. 31, 2001 genta
	mii.cbSize = SIZEOF_MENUITEMINFO; //Win95対策済みのsizeof(MENUITEMINFO)値

	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = 0;
	if( MF_OWNERDRAW	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_OWNERDRAW;
	if( MF_SEPARATOR	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_SEPARATOR;
	if( MF_STRING		& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_STRING;
	if( MF_MENUBREAK	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_MENUBREAK;
	if( MF_MENUBARBREAK	& ( nFlag | nFlagAdd ) ) mii.fType |= MFT_MENUBARBREAK;

	mii.fState = 0;
	if( MF_GRAYED		& ( nFlag | nFlagAdd ) ) mii.fState |= MFS_GRAYED;
	if( MF_CHECKED		& ( nFlag | nFlagAdd ) ) mii.fState |= MFS_CHECKED;

	mii.wID = nFuncId;
	mii.hSubMenu = (nFlag&MF_POPUP)?((HMENU)nFuncId):NULL;
	mii.hbmpChecked = NULL;
	mii.hbmpUnchecked = NULL;
	mii.dwItemData = (ULONG_PTR)this;
	mii.dwTypeData = szLabel;
	mii.cch = 0;

	// メニュー内の指定された位置に、新しいメニュー項目を挿入します。
	::InsertMenuItem( hMenu, 0xFFFFFFFF, TRUE, &mii );
	return;
}


/*
	ツールバー番号をボタン配列のindexに変換する
*/
inline int CMenuDrawer::ToolbarNoToIndex( int nToolbarNo ) const
{
	if( nToolbarNo < 0 ){
		return -1;
	}
	// 固定アクセス分のみ直接番号でアクセスさせる。m_nMyButtonNum は使わない
	if( 0 <= nToolbarNo && nToolbarNo < m_nMyButtonFixSize ){
		return nToolbarNo;
	}
	int nFuncID = nToolbarNo;
	return FindIndexFromCommandId( nFuncID, false );
}
 
/*
	ツールバー番号からアイコン番号を取得
*/
inline int CMenuDrawer::GetIconIdByFuncId( int nFuncID ) const
{
	int index = FindIndexFromCommandId( nFuncID, false );
	if( index < 0 ){
		return -1;
	}
	return m_tbMyButton[index].iBitmap;
}


/*! メニューアイテムの描画サイズを計算
	@param pnItemHeight [out] 高さ。いつも高さを返す
	@retval 0  機能がない場合
	@retval 1 <= val 機能のメニュー幅/セパレータの場合はダミーの値
*/
int CMenuDrawer::MeasureItem( int nFuncID, int* pnItemHeight )
{

	const TCHAR* pszLabel;
	CMyRect rc, rcSp;
	HDC hdc;
	HFONT hFontOld;

	if( F_0 == nFuncID ){ // F_0, なぜか F_SEPARATOR ではない
		// セパレータ。フォントの方の通常項目の半分の高さ
		*pnItemHeight = m_nMenuFontHeight / 2;
		return 30; // ダミーの幅
	}else if( NULL == ( pszLabel = GetLabel( nFuncID ) ) ){
		*pnItemHeight = m_nMenuHeight;
		return 0;
	}
	*pnItemHeight = m_nMenuHeight;

	hdc = ::GetDC( m_hWndOwner );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );
	// DT_EXPANDTABSをやめる
	::DrawText( hdc, pszLabel, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT );
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWndOwner, hdc );


//	*pnItemHeight = 20;
//	*pnItemHeight = 2 + 15 + 1;
	//@@@ 2002.2.2 YAZAKI Windowsの設定でメニューのフォントを大きくすると表示が崩れる問題に対処

	int nMenuWidth = rc.Width() + 3;
	if( m_pShareData->m_Common.m_sWindow.m_bMenuIcon ){
		nMenuWidth += 28+ DpiScaleX(8); // アイコンと枠 + アクセスキー隙間
	}else{
		// WM_MEASUREITEMで報告するメニュー幅より実際の幅は1文字分相当位広いので、その分は加えない
		nMenuWidth += ::GetSystemMetrics(SM_CXMENUCHECK) + 2 + 2;
	}
	return nMenuWidth;
}


/*! メニューアイテム描画
	@date 2001.12.21 YAZAKI デバッグモードでもメニューを選択したらハイライト。
	@date 2003.08.27 Moca システムカラーのブラシはCreateSolidBrushをやめGetSysColorBrushに
	@date 2010.07.24 Moca アイコン部分をボタン色にしてフラット表示にするなどの変更
		大きいフォント、黒背景対応
*/
void CMenuDrawer::DrawItem( DRAWITEMSTRUCT* lpdis )
{
	int			j;
	int			nItemStrLen;
	int			nIndentLeft;
	int			nIndentRight;
	HBRUSH		hBrush;
	RECT		rcText;
	int			nBkModeOld;
	int			nTxSysColor;

	const bool bMenuIconDraw = !!m_pShareData->m_Common.m_sWindow.m_bMenuIcon;
	const int nCxCheck = ::GetSystemMetrics(SM_CXMENUCHECK);
	const int nCyCheck = ::GetSystemMetrics(SM_CYMENUCHECK);

	if( bMenuIconDraw ){
		nIndentLeft  = 29; // 2+[2+16+2]+2 +5
	}else{
		nIndentLeft = 2 + 2 + nCxCheck;
	}
	// サブメニューの|＞の分は必要 最低8ぐらい
	nIndentRight = t_max(m_nMenuFontHeight, 8);

	// 2010.07.24 Moca アイコンを描くときにチラつくので、バックサーフェスを使う
	const bool bBackSurface = bMenuIconDraw;
	const int nTargetWidth  = lpdis->rcItem.right - lpdis->rcItem.left;
	const int nTargetHeight = lpdis->rcItem.bottom - lpdis->rcItem.top;
	HDC hdcOrg = NULL;
	HDC hdc = NULL;
	if( bBackSurface ){
		hdcOrg = lpdis->hDC;
		if( m_hCompDC && nTargetWidth <= m_nCompBitmapWidth && nTargetHeight <= m_nCompBitmapHeight ){
			hdc = m_hCompDC;
		}else{
			if( m_hCompDC ){
				DeleteCompDC();
			}
			hdc = m_hCompDC  = ::CreateCompatibleDC( hdcOrg );
			m_hCompBitmap    = ::CreateCompatibleBitmap( hdcOrg, nTargetWidth + 20, nTargetHeight + 4 );
			m_hCompBitmapOld = (HBITMAP)::SelectObject( hdc, m_hCompBitmap );
			m_nCompBitmapWidth  = nTargetWidth + 20;
			m_nCompBitmapHeight = nTargetHeight + 4;
		}
		::SetWindowOrgEx( hdc, lpdis->rcItem.left, lpdis->rcItem.top, NULL );
	}else{
		hdc = lpdis->hDC;
	}

	// 作画範囲を背景色で矩形塗りつぶし
#ifdef DRAW_MENU_ICON_BACKGROUND_3DFACE
	// アイコン部分の背景を灰色にする
	if( bMenuIconDraw ){
		const int nXIconMenu = lpdis->rcItem.left + nIndentLeft - 3 - 3;
		hBrush = ::GetSysColorBrush( COLOR_MENU );
		RECT rcFillMenuBack = lpdis->rcItem;
		rcFillMenuBack.left = nXIconMenu;
		::FillRect( hdc, &rcFillMenuBack, hBrush );

//		hBrush = ::GetSysColorBrush( COLOR_3DFACE );
		COLORREF colMenu   = ::GetSysColor( COLOR_MENU );
		COLORREF colFace = ::GetSysColor( COLOR_3DFACE );
		COLORREF colIconBack;
		// 明度らしきもの
		if( 64 < t_abs(t_max(t_max(GetRValue(colFace),GetGValue(colFace)),GetBValue(colFace))
			         - t_max(t_max(GetRValue(colMenu),GetGValue(colMenu)),GetBValue(colMenu))) ){
			colIconBack = colFace;
		}else{
			// 明るさが近いなら混色にして(XPテーマ等で)違和感を減らす
			BYTE valR = ((GetRValue(colFace) * 7 + GetRValue(colMenu) * 3) / 10);
			BYTE valG = ((GetGValue(colFace) * 7 + GetGValue(colMenu) * 3) / 10);
			BYTE valB = ((GetBValue(colFace) * 7 + GetBValue(colMenu) * 3) / 10);
			colIconBack = RGB(valR, valG, valB);
		}
		HBRUSH hbr = ::CreateSolidBrush( colIconBack );
		
		RECT rcIconBk = lpdis->rcItem;
		rcIconBk.right = nXIconMenu;
		::FillRect( hdc, &rcIconBk, hbr );
		::DeleteObject( hbr );

		// アイコンとテキストの間に縦線を描画する
		int nSepColor = (::GetSysColor(COLOR_3DSHADOW) != ::GetSysColor(COLOR_MENU) ? COLOR_3DSHADOW : COLOR_3DHIGHLIGHT);
		HPEN hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor(nSepColor) );
		HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, lpdis->rcItem.left + nIndentLeft - 3 - 3, lpdis->rcItem.top, NULL );
		::LineTo(   hdc, lpdis->rcItem.left + nIndentLeft - 3 - 3, lpdis->rcItem.bottom );
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );

	}else{
		// アイテム矩形塗りつぶし
		hBrush = ::GetSysColorBrush( COLOR_MENU );
		::FillRect( hdc, &lpdis->rcItem, hBrush );
	}
	
	if( lpdis->itemID == F_0 ){
		// セパレータの作画(セパレータのFuncCodeはF_SEPARETORではなくF_0)
		int y = lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top) / 2;
#ifdef DRAW_MENU_3DSTYLE
		int nSepColor = COLOR_3DSHADOW;
#else
		int nSepColor = (::GetSysColor(COLOR_3DSHADOW) != ::GetSysColor(COLOR_MENU) ? COLOR_3DSHADOW : COLOR_3DHIGHLIGHT);
#endif
		HPEN hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor(nSepColor) );
		HPEN hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx( hdc, lpdis->rcItem.left + (bMenuIconDraw ? nIndentLeft : 3), y, NULL );
		::LineTo(   hdc, lpdis->rcItem.right - 2, y );
#ifdef DRAW_MENU_3DSTYLE
		HPEN hPen3d = ::CreatePen( PS_SOLID, 1, ::GetSysColor(COLOR_3DHIGHLIGHT) );
		(void)::SelectObject( hdc, hPen3d );
		::MoveToEx( hdc, lpdis->rcItem.left + (bMenuIconDraw ? nIndentLeft - 3: 3), y + 1, NULL );
		::LineTo(   hdc, lpdis->rcItem.right - 2, y + 1 );
#endif
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		
		if( bBackSurface ){
			::BitBlt( hdcOrg, lpdis->rcItem.left, lpdis->rcItem.top, nTargetWidth, nTargetHeight,
				hdc, lpdis->rcItem.left, lpdis->rcItem.top, SRCCOPY );
		}
		return; // セパレータ。作画終了
	}

#else // DRAW_MENU_ICON_BACKGROUND_3DFACE
	hBrush = ::GetSysColorBrush( COLOR_MENU );
	::FillRect( hdc, &lpdis->rcItem, hBrush );
#endif

	const int    nItemIndex = Find( (int)lpdis->itemID );
	const TCHAR* pszItemStr = m_menuItems[nItemIndex].m_cmemLabel.GetStringPtr( &nItemStrLen );
	HFONT hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );

	nBkModeOld = ::SetBkMode( hdc, TRANSPARENT );
	if( lpdis->itemState & ODS_SELECTED ){
		// アイテムが選択されている
		RECT rc1 = lpdis->rcItem;
		if( bMenuIconDraw
#ifdef DRAW_MENU_ICON_BACKGROUND_3DFACE
#else
			&& -1 != m_menuItems[nItemIndex].m_nBitmapIdx || lpdis->itemState & ODS_CHECKED
#endif
		){
			//rc1.left += (nIndentLeft - 3);
		}
#ifdef DRAW_MENU_SELECTION_LIGHT
		HPEN hPenBorder = ::CreatePen( PS_SOLID, 1, ::GetSysColor( COLOR_HIGHLIGHT ) );
		HPEN hOldPen = (HPEN)::SelectObject( hdc, hPenBorder );
		COLORREF colHilight = ::GetSysColor( COLOR_HIGHLIGHT );
		COLORREF colMenu = ::GetSysColor( COLOR_MENU );
		BYTE valR = ((GetRValue(colHilight) * 4 + GetRValue(colMenu) * 6) / 10) | 0x18;
		BYTE valG = ((GetGValue(colHilight) * 4 + GetGValue(colMenu) * 6) / 10) | 0x18;
		BYTE valB = ((GetBValue(colHilight) * 4 + GetBValue(colMenu) * 6) / 10) | 0x18;
		hBrush = ::CreateSolidBrush( RGB(valR, valG, valB) );
		HBRUSH hOldBrush = (HBRUSH)::SelectObject( hdc, hBrush );
		::Rectangle( hdc, rc1.left, rc1.top, rc1.right, rc1.bottom );
		::SelectObject( hdc, hOldPen );
		::SelectObject( hdc, hOldBrush );
		::DeleteObject( hPenBorder );
		::DeleteObject( hBrush );
#else
		hBrush = ::GetSysColorBrush( COLOR_HIGHLIGHT );
		/* 選択ハイライト矩形 */
		::FillRect( hdc, &rc1, hBrush );
#endif

		if( lpdis->itemState & ODS_DISABLED ){
			// アイテムが使用不可
			nTxSysColor = COLOR_MENU;
		}else{
#ifdef DRAW_MENU_SELECTION_LIGHT
			nTxSysColor = COLOR_MENUTEXT;
#else
			nTxSysColor = COLOR_HIGHLIGHTTEXT;
#endif
		}
	}else{
		if( lpdis->itemState & ODS_DISABLED ){
			// アイテムが使用不可
			// 背景を黒にすると同じ色になることがある
			// 2013.06.21 GRAYTEXTに変更
			// nTxSysColor = (::GetSysColor(COLOR_3DSHADOW) != ::GetSysColor(COLOR_MENU) ? COLOR_3DSHADOW : COLOR_3DHIGHLIGHT);
			nTxSysColor = COLOR_GRAYTEXT;
		}else{
			nTxSysColor = COLOR_MENUTEXT;
		}
	}
	::SetTextColor( hdc, ::GetSysColor(nTxSysColor) );

#ifdef _DEBUG
	// デバッグ用：メニュー項目に対して、ヘルプがない場合に背景色を青くする
	TCHAR	szText[1024];
	MENUITEMINFO mii;
	// メニュー項目に関する情報を取得します。
	memset_raw( &mii, 0, sizeof( mii ) );

	mii.cbSize = SIZEOF_MENUITEMINFO; // Win95対策済みのsizeof(MENUITEMINFO)値

	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = MFT_STRING;
	_tcscpy( szText, _T("--unknown--") );
	mii.dwTypeData = szText;
	mii.cch = _countof( szText ) - 1;
	if( 0 != ::GetMenuItemInfo( (HMENU)lpdis->hwndItem, lpdis->itemID, FALSE, &mii )
	 && NULL == mii.hSubMenu
	 && 0 == /* CEditWnd */::FuncID_To_HelpContextID( (EFunctionCode)lpdis->itemID ) 	/* 機能IDに対応するメニューコンテキスト番号を返す */
	){
		//@@@ 2001.12.21 YAZAKI
		if( lpdis->itemState & ODS_SELECTED ){
			::SetTextColor( hdc, ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );	//	ハイライトカラー
		}
		else {
			::SetTextColor( hdc, RGB( 0, 0, 255 ) );	//	青くしてる。
		}
//		::SetTextColor( hdc, RGB( 0, 0, 255 ) );
	}
#endif

	rcText = lpdis->rcItem;
	rcText.left += nIndentLeft + 1;
	rcText.right -= nIndentRight;

	/* TAB文字の前と後ろに分割してテキストを描画する */
	for( j = 0; j < nItemStrLen; ++j ){
		if( pszItemStr[j] == _T('\t') ){
			break;
		}
	}
	/* TAB文字の後ろ側のテキストを描画する */
	if( j < nItemStrLen ){
#ifdef DRAW_MENU_3DSTYLE
		/* アイテムが使用不可 */
		if( lpdis->itemState & ODS_DISABLED && !(lpdis->itemState & ODS_SELECTED)  ){
			COLORREF colOld = ::SetTextColor( hdc, ::GetSysColor( COLOR_3DHIGHLIGHT ) );
				rcText.left++;
				rcText.top++;
				rcText.right++;
				rcText.bottom++;
				::DrawText(
					hdc,
					&pszItemStr[j + 1],
					-1,
					&rcText,
					DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_RIGHT
				);
				rcText.left--;
				rcText.top--;
				rcText.right--;
				rcText.bottom--;
				::SetTextColor( hdc, colOld );
		}
#endif
		::DrawText(
			hdc,
			&pszItemStr[j + 1],
			-1,
			&rcText,
			DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_RIGHT
		);
	}
	/* TAB文字の前側のテキストを描画する */
#ifdef DRAW_MENU_3DSTYLE
	/* アイテムが使用不可 */
	if( lpdis->itemState & ODS_DISABLED && !(lpdis->itemState & ODS_SELECTED)  ){
		COLORREF colOld = ::SetTextColor( hdc, ::GetSysColor( COLOR_3DHIGHLIGHT ) );

		rcText.left++;
		rcText.top++;
		rcText.right++;
		rcText.bottom++;
		::DrawText( hdc, pszItemStr, j, &rcText, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_LEFT );

		rcText.left--;
		rcText.top--;
		rcText.right--;
		rcText.bottom--;
		::SetTextColor( hdc, colOld );
	}
#endif
	::DrawText(
		hdc,
		pszItemStr,
		j,
		&rcText,
		DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_LEFT
	);
	::SelectObject( hdc, hFontOld  );
	::SetBkMode( hdc, nBkModeOld );


	// 16*16のアイコンを上下中央へ置いたときの上の座標
	int nIconTop = lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top) / 2 - (16/2);

	// 枠は アイコン横幅xメニュー縦幅で表示し真ん中にアイコンを置く

	if( bMenuIconDraw && (lpdis->itemState & ODS_CHECKED) ){
		/* アイコンを囲む枠 */
// 2010.07.12 Moca グレーの3D表示をやめる
#ifdef DRAW_MENU_ICON_3DBUTTON
		// チェック状態なら凹んだ3D枠を描画する
		CSplitBoxWnd::Draw3dRect(
			hdc, lpdis->rcItem.left + 1, lpdis->rcItem.top,
			2 + 16 + 2, lpdis->rcItem.bottom - lpdis->rcItem.top,
			::GetSysColor( COLOR_3DSHADOW ),
			::GetSysColor( COLOR_3DHIGHLIGHT )
		);
#else
		{
			// フラットな枠 + 半透明の背景色
			HBRUSH hBrush = ::GetSysColorBrush( COLOR_HIGHLIGHT );
			const int MENUICO_PADDING = 0;
			const int MENUICO_BORDER  = 1;
			const int MENUICO_PB  = MENUICO_PADDING + MENUICO_BORDER;
			const int MENUICO_SIZE = MENUICO_PB + 16 + MENUICO_PB;
			const int left = lpdis->rcItem.left + 2 - MENUICO_PB;
			const int top = nIconTop - MENUICO_PB;
			RECT rc;
			::SetRect( &rc, left-1, top-1, left + MENUICO_SIZE+2, top + MENUICO_SIZE+1 );
			::FillRect( hdc, &rc, hBrush );

			COLORREF colHilight = ::GetSysColor( COLOR_HIGHLIGHT );
			COLORREF colMenu = ::GetSysColor( COLOR_MENU );
			// 16bitカラーの黒色でも少し明るくするように or 0x18 する
			BYTE valR;
			BYTE valG;
			BYTE valB;
			if( lpdis->itemState & ODS_SELECTED ){	// 選択状態
				valR = ((GetRValue(colHilight) * 6 + GetRValue(colMenu) * 4) / 10) | 0x18;
				valG = ((GetGValue(colHilight) * 6 + GetGValue(colMenu) * 4) / 10) | 0x18;
				valB = ((GetBValue(colHilight) * 6 + GetBValue(colMenu) * 4) / 10) | 0x18;
			} else {								// 非選択状態
				valR = ((GetRValue(colHilight) * 2 + GetRValue(colMenu) * 8) / 10) | 0x18;
				valG = ((GetGValue(colHilight) * 2 + GetGValue(colMenu) * 8) / 10) | 0x18;
				valB = ((GetBValue(colHilight) * 2 + GetBValue(colMenu) * 8) / 10) | 0x18;
			}
			HBRUSH hbr = ::CreateSolidBrush( RGB(valR, valG, valB) );
			::SetRect( &rc, left + MENUICO_BORDER-1, top + MENUICO_BORDER-1,
				left + MENUICO_SIZE - MENUICO_BORDER+2, top + MENUICO_SIZE - MENUICO_BORDER+1 );
			::FillRect( hdc, &rc, hbr );
			::DeleteObject( hbr );
		}
#endif

// 2010.07.12 Moca グレーの3D表示をやめる
#ifdef DRAW_MENU_ICON_3DBUTTON
		/* アイテムが選択されていない場合は3D枠の中を明るく塗りつぶす */
		if( lpdis->itemState & ODS_SELECTED ){
		}else{
			HBRUSH hbr = ::GetSysColorBrush( COLOR_3DLIGHT );
			RECT rc;
			::SetRect( &rc, lpdis->rcItem.left + 2 + 1, lpdis->rcItem.top + 1,
				lpdis->rcItem.left + 1 + 2 + 16 + 2 - 1, lpdis->rcItem.bottom - 1 );
			::FillRect( hdc, &rc, hbr );
		}
#endif
	}


	/* 機能の画像が存在するならメニューアイコン?を描画する */
	if( bMenuIconDraw && -1 != m_menuItems[nItemIndex].m_nBitmapIdx ){
		/* 3D枠を描画する */
		/* アイテムが選択されている */
		if( lpdis->itemState & ODS_SELECTED ){
			/* アイテムが使用不可 */
			if( lpdis->itemState & ODS_DISABLED /*&& !(lpdis->itemState & ODS_SELECTED)*/  ){
			}else{
				if( lpdis->itemState & ODS_CHECKED ){
				}else{
// 2010.07.12 Moca グレーの3D表示をやめる
#ifdef DRAW_MENU_ICON_3DBUTTON
					// アイコンを囲む枠(メニューの高さいっぱい)
					CSplitBoxWnd::Draw3dRect(
						hdc, lpdis->rcItem.left + 1, lpdis->rcItem.top,
						2 + 16 + 2, lpdis->rcItem.bottom - lpdis->rcItem.top,
						::GetSysColor( COLOR_3DHIGHLIGHT ),
						::GetSysColor( COLOR_3DSHADOW )
					 );
#endif
				}
			}
		}

		/* アイテムが使用不可 */
		if( lpdis->itemState & ODS_DISABLED ){
			/* 淡色アイコン */
			m_pcIcons->Draw( m_menuItems[nItemIndex].m_nBitmapIdx,
				hdc,	//	Target DC
				lpdis->rcItem.left + 2,	//	X
				//@@@ 2002.1.29 YAZAKI Windowsの設定でメニューのフォントを大きくすると表示が崩れる問題に対処
				nIconTop,	//	Y
				ILD_MASK
			);

		}else{
			/* 通常のアイコン */
			m_pcIcons->Draw( m_menuItems[nItemIndex].m_nBitmapIdx,
				hdc,	//	Target DC
				lpdis->rcItem.left + 2,	//	X
				//@@@ 2002.1.29 YAZAKI Windowsの設定でメニューのフォントを大きくすると表示が崩れる問題に対処
				nIconTop,	//	Y
				ILD_NORMAL
			);
		}

	}else{
		// チェックボックスを表示
		if( lpdis->itemState & ODS_CHECKED ){
			/* チェックマークの表示 */
			if( bMenuIconDraw ){
				// だいたい中心座標
				int nX = lpdis->rcItem.left + 16/2;
				int nY = nIconTop + 16/2;
				HPEN hPen   = NULL;
				HPEN hPenOld = NULL;
				// 2010.05.31 チェックの色を黒(未指定)からテキスト色に変更
				hPen = ::CreatePen( PS_SOLID, 1, ::GetSysColor(COLOR_MENUTEXT) );
				hPenOld = (HPEN)::SelectObject( hdc, hPen );
#if 0
// チェックマークも自分で書く場合
				if( !bMenuIconDraw ){
					nX -= 4; // iconがない場合、左マージン=2アイコン枠=2分がない
				}
#endif
				const int nBASE = 100*100; // 座標,nScale共に0.01単位
				// 16dot幅しかないので 1.0倍から2.1倍までスケールする(10-23)
				const int nScale = t_max(100, t_min(210, int((lpdis->rcItem.bottom - lpdis->rcItem.top - 2) * 100) / (16-2) ));
				for( int nBold = 1; nBold <= (281*nScale)/nBASE; nBold++ ){
					::MoveToEx( hdc, nX - (187*nScale)/nBASE, nY - (187*nScale)/nBASE, NULL );
					::LineTo(   hdc, nX -   (0*nScale)/nBASE, nY -   (0*nScale)/nBASE );
					::LineTo(   hdc, nX + (468*nScale)/nBASE, nY - (468*nScale)/nBASE );
					nY++;
				}
				if( hPen ){
					::SelectObject( hdc, hPenOld );
					::DeleteObject( hPen );
				}
			}else{
				// OSにアイコン作画をしてもらう(黒背景等対応)
				HDC hdcMem = ::CreateCompatibleDC( hdc );
				HBITMAP hBmpMono = ::CreateBitmap( nCxCheck, nCyCheck, 1, 1, NULL );
				HBITMAP hOld = (HBITMAP)::SelectObject( hdcMem, hBmpMono );
				RECT rcCheck = {0,0, nCxCheck, nCyCheck};
				::DrawFrameControl( hdcMem, &rcCheck, DFC_MENU, DFCS_MENUCHECK );
				COLORREF colTextOld = ::SetTextColor(hdc, RGB(0,0,0) );
				COLORREF colBackOld = ::SetBkColor(hdc,   RGB(255,255,255) );
				::BitBlt( hdc, lpdis->rcItem.left+2, lpdis->rcItem.top+2, nCxCheck, nCyCheck, hdcMem, 0, 0, SRCAND );
				::SetTextColor( hdc, ::GetSysColor(nTxSysColor) );
				::SetBkColor( hdc, RGB(0,0,0) );
				::BitBlt( hdc, lpdis->rcItem.left+2, lpdis->rcItem.top+2, nCxCheck, nCyCheck, hdcMem, 0, 0, SRCPAINT );
				::SetTextColor( hdc, colTextOld );
				::SetBkColor( hdc, colBackOld );
				::SelectObject( hdcMem, hOld );
				::DeleteObject( hBmpMono );
				::DeleteDC( hdcMem );
			}
		}
	}
	if( bBackSurface ){
		::BitBlt( hdcOrg, lpdis->rcItem.left, lpdis->rcItem.top, nTargetWidth, nTargetHeight,
			hdc, lpdis->rcItem.left, lpdis->rcItem.top, SRCCOPY );
	}
	return;
}

/*!
	作画終了
	メニューループ終了時に呼び出すとリソース節約になる
	
	@date 20100724 Moca バックサーフェス用に新設
*/
void CMenuDrawer::EndDrawMenu()
{
	DeleteCompDC();
}


void CMenuDrawer::DeleteCompDC()
{
	if( m_hCompDC ){
		::SelectObject( m_hCompDC, m_hCompBitmapOld );
		::DeleteObject( m_hCompBitmap );
		::DeleteObject( m_hCompDC );
//		DEBUG_TRACE( _T("CMenuDrawer::DeleteCompDC %x\n"), m_hCompDC );
		m_hCompDC = NULL;
		m_hCompBitmap = NULL;
		m_hCompBitmapOld = NULL;
	}
}

/*
	ツールバー登録のための番号を返す。
	プラグインのみボタンのindexのかわりにidCommandをそのまま返す
	@date 2010.06.24 Moca 新規作成
	@note この値がiniのツールバーアイテムの記録に使われる
*/
int CMenuDrawer::FindToolbarNoFromCommandId( int idCommand, bool bOnlyFunc ) const
{
	// 先に存在確認をする
	int index = FindIndexFromCommandId( idCommand, bOnlyFunc );
	if( -1 < index ){
		// 固定部分以外(プラグインなど)はindexではなくidCommandのままにする
		if( m_nMyButtonFixSize <= index ){
			// もし コマンド番号が明らかに小さいと区別がつかない
			assert_warning( idCommand < m_nMyButtonFixSize );
			return idCommand;
		}
	}
	return index;
}

/** コマンドコードからツールバーボタン情報のINDEXを得る

	@param idCommand [in] コマンドコード
	@param bOnlyFunc [in] 有効な機能の範囲で検索する

	@retval みつからなければ-1を返す。

	@date 2005.08.09 aroka m_nMyButtonNum隠蔽のため追加
	@date 2005.11.02 ryoji bOnlyFuncパラメータを追加
 */
int CMenuDrawer::FindIndexFromCommandId( int idCommand, bool bOnlyFunc ) const
{
	if( bOnlyFunc ){
		// 機能の範囲外（セパレータや折り返しなど特別なもの）は除外する
		if ( !( F_MENU_FIRST <= idCommand && idCommand < F_MENU_NOT_USED_FIRST )
			&& !( F_PLUGCOMMAND_FIRST <= idCommand && idCommand < F_PLUGCOMMAND_LAST )){
			return -1;
		}
	}

	int nIndex = -1;
	for( int i = 0; i < m_nMyButtonNum; i++ ){
		if( m_tbMyButton[i].idCommand == idCommand ){
			nIndex = i;
			break;
		}
	}

	return nIndex;
}

/** インデックスからボタン情報を得る

	@param nToolbarNo [in] ボタン情報のツールバー番号
	@retval ボタン情報

	@date 2007.11.02 ryoji 範囲外の場合は未定義のボタン情報を返すように
	@date 2010.06.24 Moca 引数をツールバー番号に変更
 */
TBBUTTON CMenuDrawer::getButton( int nToolbarNo ) const
{
	int index = ToolbarNoToIndex( nToolbarNo );
	if( 0 <= index && index < m_nMyButtonNum ){
		return m_tbMyButton[index];
	}

	// 範囲外なら未定義のボタン情報を作成して返す
	// （sakura.iniに範囲外インデックスが指定があった場合など、堅牢性のため）
	static TBBUTTON tbb;
	SetTBBUTTONVal( &tbb, -1, F_DISABLE, 0, TBSTYLE_BUTTON, 0, 0 );
	return tbb;
}


int CMenuDrawer::Find( int nFuncID )
{
	int i;
	int nItemNum = (int)m_menuItems.size();
	for( i = 0; i < nItemNum; ++i ){
		if( nFuncID == m_menuItems[i].m_nFuncId ){
			break;
		}
	}
	if( i >= nItemNum ){
		return -1;
	}else{
		return i;
	}
}


const TCHAR* CMenuDrawer::GetLabel( int nFuncID )
{
	int i;
	if( -1 == ( i = Find( nFuncID ) ) ){
		return NULL;
	}
	return m_menuItems[i].m_cmemLabel.GetStringPtr();
}

TCHAR CMenuDrawer::GetAccelCharFromLabel( const TCHAR* pszLabel )
{
	int i;
	int nLen = (int)_tcslen( pszLabel );
	for( i = 0; i + 1 < nLen; ++i ){
		if( _T('&') == pszLabel[i] ){
			if( _T('&') == pszLabel[i + 1]  ){
				i++;
			}else{
				return (TCHAR)_totupper( pszLabel[i + 1] );
			}
		}
	}
	return _T('\0');
}



struct WorkData{
	int				idx;
	MENUITEMINFO	mii;
};

/*! メニューアクセスキー押下時の処理(WM_MENUCHAR処理) */
LRESULT CMenuDrawer::OnMenuChar( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	TCHAR				chUser;
	HMENU				hmenu;
	int i;
	chUser = (TCHAR) LOWORD(wParam);	// character code
	hmenu = (HMENU) lParam;				// handle to menu
//	MYTRACE( _T("::GetMenuItemCount( %xh )==%d\n"), hmenu, ::GetMenuItemCount( hmenu ) );

	//	Oct. 27, 2000 genta
	if( 0 <= chUser && chUser < ' '){
		chUser += '@';
	}
	else {
		chUser = (TCHAR)_totupper( chUser );
	}

	// 2011.11.18 vector化
	std::vector<WorkData> vecAccel;
	size_t nAccelSel = 99999;
	for( i = 0; i < ::GetMenuItemCount( hmenu ); i++ ){
		TCHAR	szText[1024];
		// メニュー項目に関する情報を取得します。
		MENUITEMINFO		mii;
		memset_raw( &mii, 0, sizeof( mii ) );

		mii.cbSize = SIZEOF_MENUITEMINFO; //Win95対策済みのsizeof(MENUITEMINFO)値

		mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
		mii.fType = MFT_STRING;
		_tcscpy( szText, _T("--unknown--") );
		mii.dwTypeData = szText;
		mii.cch = _countof( szText ) - 1;
		if( 0 == ::GetMenuItemInfo( hmenu, i, TRUE, &mii ) ){
			continue;
		}
		const TCHAR* pszLabel;
		if( NULL == ( pszLabel = GetLabel( mii.wID ) ) ){
			continue;
		}
		if( chUser == GetAccelCharFromLabel( pszLabel ) ){
			WorkData work;
			work.idx = i;
			work.mii = mii;
			if( /*-1 == nAccelSel ||*/ MFS_HILITE & mii.fState ){
				nAccelSel = vecAccel.size();
			}
			vecAccel.push_back( work );
		}
	}
//	MYTRACE( _T("%d\n"), (int)mapAccel.size() );
	if( 0 == vecAccel.size() ){
		return  MAKELONG( 0, MNC_IGNORE );
	}
	if( 1 == vecAccel.size() ){
		return  MAKELONG( vecAccel[0].idx, MNC_EXECUTE );
	}
//	MYTRACE( _T("nAccelSel=%d vecAccel.size()=%d\n"), nAccelSel, vecAccel.size() );
	if( nAccelSel + 1 >= vecAccel.size() ){
//		MYTRACE( _T("vecAccel[0].idx=%d\n"), vecAccel[0].idx );
		return  MAKELONG( vecAccel[0].idx, MNC_SELECT );
	}else{
//		MYTRACE( _T("vecAccel[nAccelSel + 1].idx=%d\n"), vecAccel[nAccelSel + 1].idx );
		return  MAKELONG( vecAccel[nAccelSel + 1].idx, MNC_SELECT );
	}
}

//	Jul. 21, 2003 genta
//	コメントアウトされていた部分を削除 (CImageListで再利用)


/* TBBUTTON構造体にデータをセット */
void CMenuDrawer::SetTBBUTTONVal(
	TBBUTTON*	ptb,
	int			iBitmap,
	int			idCommand,
	BYTE		fsState,
	BYTE		fsStyle,
	DWORD_PTR	dwData,
	INT_PTR		iString
) const
{
	/*
typedef struct _TBBUTTON {
	int iBitmap;	// ボタン イメージの 0 から始まるインデックス
	int idCommand;	// ボタンが押されたときに送られるコマンド
	BYTE fsState;	// ボタンの状態--以下を参照
	BYTE fsStyle;	// ボタン スタイル--以下を参照
	DWORD dwData;	// アプリケーション-定義された値
	int iString;	// ボタンのラベル文字列の 0 から始まるインデックス
} TBBUTTON;
*/

	ptb->iBitmap	= iBitmap;
	ptb->idCommand	= idCommand;
	ptb->fsState	= fsState;
	ptb->fsStyle	= fsStyle;
	ptb->dwData		= dwData;
	ptb->iString	= iString;
	return;
}

//ツールバーボタンを追加する
//	マネージメント機能追加	2010/7/3 Uchi 
//		全ウィンドウで同じ機能番号の場合、同じICON番号を持つように調整
void CMenuDrawer::AddToolButton( int iBitmap, int iCommand )
{
	TBBUTTON tbb;
	int 	iCmdNo;
	int 	i;
	
	if (m_pShareData->m_maxTBNum < m_nMyButtonNum) {
		m_pShareData->m_maxTBNum = m_nMyButtonNum;
	}

	if (iCommand >= F_PLUGCOMMAND_FIRST && iCommand <= F_PLUGCOMMAND_LAST) {
		iCmdNo = iCommand - F_PLUGCOMMAND_FIRST;
		if (m_pShareData->m_PlugCmdIcon[iCmdNo] != 0) {
			if (m_tbMyButton.size() <= (size_t)(int)m_pShareData->m_PlugCmdIcon[iCmdNo]) {
				// このウィンドウで未登録
				// 空きを詰め込む
				SetTBBUTTONVal( &tbb,TOOLBAR_ICON_PLUGCOMMAND_DEFAULT-1, 0, 0, TBSTYLE_BUTTON, 0, 0 );
				for (i = m_tbMyButton.size(); i < m_pShareData->m_PlugCmdIcon[iCmdNo]; i++) {
					m_tbMyButton.push_back( tbb );
					m_nMyButtonNum++;
				}

				// 未登録
				SetTBBUTTONVal( &tbb, iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 );
				//最後に追加に変更
				m_tbMyButton.push_back( tbb );
				m_nMyButtonNum++;
			}
			else {
				// 再設定
				SetTBBUTTONVal( &m_tbMyButton[m_pShareData->m_PlugCmdIcon[iCmdNo]],
					iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 );
			}
		}
		else {
			// 全体で未登録
			if (m_tbMyButton.size() < (size_t)m_pShareData->m_maxTBNum) {
				// 空きを詰め込む
				SetTBBUTTONVal( &tbb, TOOLBAR_ICON_PLUGCOMMAND_DEFAULT-1, 0, 0, TBSTYLE_BUTTON, 0, 0 );
				for (i = m_tbMyButton.size(); i < m_pShareData->m_maxTBNum; i++) {
					m_tbMyButton.push_back( tbb );
					m_nMyButtonNum++;
				}
			}
			// 新規登録
			SetTBBUTTONVal( &tbb, iBitmap, iCommand, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 );

			m_pShareData->m_PlugCmdIcon[iCmdNo] = (short)m_tbMyButton.size();
			//最後から２番目に挿入する。一番最後は番兵で固定。
			//2010.06.23 Moca 最後に追加に変更
			m_tbMyButton.push_back( tbb );
			m_nMyButtonNum++;
		}
	}
	if (m_pShareData->m_maxTBNum < m_nMyButtonNum) {
		m_pShareData->m_maxTBNum = m_nMyButtonNum;
	}
}
