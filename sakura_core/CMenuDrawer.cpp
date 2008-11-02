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
#include "stdafx.h"
#include "CMenuDrawer.h"
#include "sakura_rc.h"
#include "global.h"
#include "debug.h"
#include "CSplitBoxWnd.h"
#include "CEditWnd.h"
#include "etc_uty.h"

//	Jan. 29, 2002 genta
//	Win95/NTが納得するsizeof( MENUITEMINFO )
//	これ以外の値を与えると古いOSでちゃんと動いてくれない．
const int SIZEOF_MENUITEMINFO = 44;

void FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr)
{
//	ASSERT_VALID(this);
//	ASSERT(m_hDC != NULL);

	RECT rect;
	::SetBkColor( hdc, clr );
	::SetRect( &rect, x, y, x + cx, y + cy );
	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL );
}


//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
CMenuDrawer::CMenuDrawer()
{
	/* 共有データ構造体のアドレスを返す */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_nMenuItemNum = 0;
	m_nMenuHeight = 0;
	m_hFontMenu = NULL;
	m_hFontMenuUndelLine = NULL;

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
	SetTBBUTTONVal( &m_tbMyButton[0], -1, 0, 0, TBSTYLE_SEP, 0, 0 );	//セパレータ	// 2007.11.02 ryoji アイコンの未定義化(-1)
#if 0
	2002/04/26 無用な汎用性は排除。
	struct TBUTTONDATA {
		int			idCommand;
		BYTE		fsState;
		BYTE		fsStyle;
		DWORD		dwData;
		int			iString;
	};
#endif
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
//	・アイコンビットマップファイルには横32個X13段あるが有効にしてあるのは11段まで(12段目は基本的に作業用, 13段目は試作品など保管用)
//	・メニューに属する系および各系の段との関係は次の通り(Oct. 22, 2000 現在)：
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
//					+ マクロ系			(9段目最後の12個: 277-288)
//					+ 外部マクロ		(12段目12個: 353-372)
//					+ カスタムメニュー	(10段目32個: 289-320)
//		ウィンドウ--- ウィンドウ系		(11段目22個: 321-342)
//		ヘルプ------- 支援				(11段目残りの10個: 343-352)
//			★その他					(12段目残りの10個: 373-384)
//	注1.「挿入系」はメニューでは「編集」に入っている
//	注2.「その他」はメニューには入っていないものを入れる (現在何もないので12段目を設定してない)
//	注3.「コマンド一覧」で敢えて重複していれてあるコマンドはその「本家」の方に配置した
//	注4.「コマンド一覧」に入ってないコマンドもわかっている範囲で位置予約にしておいた
//  注5. F_DISABLE は未定義用(ダミーとしても使う)
//	注6. ユーザー用に確保された場所は特にないので各段の空いている後ろの方を使ってください。

	int /* TBUTTONDATA */ tbd[] = {
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
/* 17 */		F_OPEN_HHPP					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
/* 18 */		F_OPEN_CCPP					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
/* 19 */		F_ACTIVATE_SQLPLUS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Oracle SQL*Plusをアクティブ表示 */	//Sept. 20, 2000 JEPRO 追加
/* 20 */		F_PLSQL_COMPILE_ON_SQLPLUS	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//Oracle SQL*Plusで実行 */	//Sept. 17, 2000 jepro 説明の「コンパイル」を「実行」に統一
/* 21 */		F_BROWSE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ブラウズ
/* 22 */		F_PROPERTY_FILE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ファイルのプロパティ//Sept. 16, 2000 JEPRO mytool1.bmpにあった「ファイルのプロパティ」アイコンをIDB_MYTOOLにコピー
/* 23 */		F_READONLY					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//読み取り専用
/* 24 */		F_FILE_REOPEN_UNICODEBE		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//UnicodeBEで開き直す // Moca, 2002/05/26 追加
/* 25 */		F_FILEOPEN_DROPDOWN			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//開く(ドロップダウン)
/* 26 */		F_FILE_REOPEN				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//開きなおす
/* 27 */		F_EXITALL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加
/* 28 */		F_FILESAVECLOSE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//保存して閉じる Feb. 28, 2004 genta
/* 29 */		F_DISABLE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 30 */		F_FILESAVEALL					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//全て上書き保存 Jan. 24, 2005 genta
/* 31 */		F_EXITALLEDITORS			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//編集の全終了	// 2007.02.13 ryoji 追加
/* 32 */		F_DISABLE					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

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
/* 63 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 64 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

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
/* 87 */		F_JUMPHIST_PREV						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//移動履歴: 前へ	//Sept. 28, 2000 JEPRO 追加
/* 88 */		F_JUMPHIST_NEXT						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//移動履歴: 次へ	//Sept. 28, 2000 JEPRO 追加
/* 89 */		F_WndScrollDown					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//テキストを１行下へスクロール	//Jun. 28, 2001 JEPRO 追加
/* 90 */		F_WndScrollUp					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//テキストを１行上へスクロール	//Jun. 28, 2001 JEPRO 追加
/* 91 */		F_GONEXTPARAGRAPH				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次の段落へ
/* 92 */		F_GOPREVPARAGRAPH				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前の段落へ
/* 93 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 94 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 95 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
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
/* 120 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 121 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 122 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 123 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 124 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 125 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 126 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 127 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 128 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* 矩形選択系(5段目32個: 129-160) */ //(注. 矩形選択系のほとんどは未実装)
/* 129 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 130 */		F_DISABLE/*F_BOXSELALL*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//矩形ですべて選択
/* 131 */		F_BEGIN_BOX								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//矩形範囲選択開始	//Sept. 29, 2000 JEPRO 追加
/* 132 */		F_DISABLE/*F_UP_BOX*/					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル上移動
/* 133 */		F_DISABLE/*F_DOWN_BOX*/					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル下移動
/* 134 */		F_DISABLE/*F_LEFT_BOX*/					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル左移動
/* 135 */		F_DISABLE/*F_RIGHT_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル右移動
/* 136 */		F_DISABLE/*F_UP2_BOX*/					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル上移動(２行ごと)
/* 137 */		F_DISABLE/*F_DOWN2_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)カーソル下移動(２行ごと)
/* 138 */		F_DISABLE/*F_WORDLEFT_BOX*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)単語の左端に移動
/* 139 */		F_DISABLE/*F_WORDRIGHT_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)単語の右端に移動
/* 140 */		F_DISABLE/*F_GOLINETOP_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)行頭に移動(折り返し単位)
/* 141 */		F_DISABLE/*F_GOLINEEND_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)行末に移動(折り返し単位)
/* 142 */		F_DISABLE/*F_HalfPageUp_Box*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)半ページアップ
/* 143 */		F_DISABLE/*F_HalfPageDown_Box*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)半ページダウン
/* 144 */		F_DISABLE/*F_1PageUp_Box*/				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)１ページアップ
/* 145 */		F_DISABLE/*F_1PageDown_Box*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)１ページダウン
/* 146 */		F_DISABLE/*F_DISABLE/*F_DISPLAYTOP_BOX*//* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)画面の先頭に移動(未実装)
/* 147 */		F_DISABLE/*F_DISPLAYEND_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)画面の最後に移動(未実装)
/* 148 */		F_DISABLE/*F_GOFILETOP_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)ファイルの先頭に移動
/* 149 */		F_DISABLE/*F_GOFILEEND_BOX*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//(矩形選択)ファイルの最後に移動
/* 150 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 151 */		F_DISABLE								/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
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
/* 166 */		F_DISABLE/*F_INSTEXT*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//テキストを貼り付け	(未公開コマンド？未完成？)
/* 167 */		F_DISABLE/*F_ADDTAIL*/			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//最後にテキストを追加	(未公開コマンド？未完成？)
/* 168 */		F_COPYLINES						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//選択範囲内全行コピー	//Sept. 30, 2000 JEPRO 追加
/* 169 */		F_COPYLINESASPASSAGE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//選択範囲内全行引用符付きコピー	//Sept. 30, 2000 JEPRO 追加
/* 170 */		F_COPYLINESWITHLINENUMBER		/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//選択範囲内全行行番号付きコピー	//Sept. 30, 2000 JEPRO 追加
/* 171 */		F_COPYPATH						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//このファイルのパス名をコピー //added Oct. 22, 2000 JEPRO				//Nov. 5, 2000 JEPRO 追加
/* 172 */		F_COPYTAG						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//このファイルのパス名とカーソル位置をコピー //added Oct. 22, 2000 JEPRO	//Nov. 5, 2000 JEPRO 追加
/* 173 */		F_CREATEKEYBINDLIST				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キー割り当て一覧をコピー //added Oct. 22, 2000 JEPRO	//Dec. 25, 2000 JEPRO アイコン追加
/* 174 */		F_COPYFNAME						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//このファイル名をクリップボードにコピー //2002/2/3 aroka
/* 175 */		F_COPY_ADDCRLF					/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//折り返し位置に改行をつけてコピー
/* 176 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 177 */		F_DISABLE						/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
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
/* 198 */		F_HANKATATOZENKAKUKATA	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//半角カタカナ→全角カタカナ	//Sept. 18, 2000 JEPRO 追加
/* 199 */		F_HANKATATOZENKAKUHIRA	/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//半角カタカナ→全角ひらがな	//Sept. 18, 2000 JEPRO 追加
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
/* 218 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 219 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 220 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 221 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 222 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 223 */		F_DISABLE				/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
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
/* 259 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 260 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

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
/* 276 */		F_DISABLE			/* , TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* マクロ系(9段目最後の12個: 277-288) */
/* 277 */		F_RECKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーマクロの記録開始／終了
/* 278 */		F_SAVEKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーマクロの保存		//Sept. 21, 2000 JEPRO 追加
/* 279 */		F_LOADKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーマクロの読み込み	//Sept. 21, 2000 JEPRO 追加
/* 280 */		F_EXECKEYMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//キーマクロの実行		//Sept. 16, 2000 JEPRO 下から上に移動した
/* 281 */		F_EXECCOMMAND_DIALOG	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部コマンド実行//Sept. 20, 2000 JEPRO 名称をCMMANDからCOMMANDに変更(EXECCMMAND→EXECCMMAND)
/* 282 */		F_EXECEXTMACRO			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//名前を指定してマクロ実行	//2008.10.22 syat 追加
/* 283 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 284 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 285 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 286 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 287 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 288 */		F_DISABLE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

/* カスタムメニュー(10段目32個: 289-320) */
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
/* 300 */		F_DISABLE/*F_CUSTMENU_11*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー11	//アイコン未作
/* 301 */		F_DISABLE/*F_CUSTMENU_12*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー12	//アイコン未作
/* 302 */		F_DISABLE/*F_CUSTMENU_13*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー13	//アイコン未作
/* 303 */		F_DISABLE/*F_CUSTMENU_14*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー14	//アイコン未作
/* 304 */		F_DISABLE/*F_CUSTMENU_15*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー15	//アイコン未作
/* 305 */		F_DISABLE/*F_CUSTMENU_16*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー16	//アイコン未作
/* 306 */		F_DISABLE/*F_CUSTMENU_17*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー17	//アイコン未作
/* 307 */		F_DISABLE/*F_CUSTMENU_18*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー18	//アイコン未作
/* 308 */		F_DISABLE/*F_CUSTMENU_19*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー19	//アイコン未作
/* 309 */		F_DISABLE/*F_CUSTMENU_20*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー20	//アイコン未作
/* 310 */		F_DISABLE/*F_CUSTMENU_21*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー21	//アイコン未作
/* 311 */		F_DISABLE/*F_CUSTMENU_22*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー22	//アイコン未作
/* 312 */		F_DISABLE/*F_CUSTMENU_23*/	/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー23	//アイコン未作
/* 313 */		F_CUSTMENU_24				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//カスタムメニュー24	//アイコン未作
/* 314 */		F_TAB_MOVERIGHT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タブを右に移動	// 2007.06.20 ryoji
/* 315 */		F_TAB_MOVELEFT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//タブを左に移動	// 2007.06.20 ryoji
/* 316 */		F_TAB_SEPARATE				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//新規グループ	// 2007.06.20 ryoji
/* 317 */		F_TAB_JOINTNEXT				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//次のグループに移動	// 2007.06.20 ryoji
/* 318 */		F_TAB_JOINTPREV				/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//前のグループに移動	// 2007.06.20 ryoji
/* 319 */		F_DISABLE					/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 320 */		F_DISABLE					/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー

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
/* 338 */		F_DISABLE		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
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
/* 外部マクロ(12段目32個: 353-384) */
/* 353 */		F_USERMACRO_0		/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//外部マクロ①
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
/* 373 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 374 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 375 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 376 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 377 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 378 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 379 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 380 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 381 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 382 */		F_DISABLE 			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
/* 383 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */,	//ダミー
//	2007.10.17 genta 384は折り返しマークとして使用しているのでアイコンとしては使用できない
/* 384 */		F_DISABLE			/*, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 */	//最終行用ダミー(Jepro note: 最終行末にはカンマを付けないこと)

};
	int tbd_num = sizeof( tbd ) / sizeof( tbd[0] );
	BYTE	style;	//@@@ 2002.06.15 MIK
	
	for( int i = 0; i < tbd_num; i++ ){
		switch( tbd[i] )	//@@@ 2002.06.15 MIK
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

		//	m_tbMyButton[0]にはセパレータが入っているため。
		SetTBBUTTONVal(
			&m_tbMyButton[i+1],
			(tbd[i] == F_DISABLE)? -1: i,	// 2007.11.02 ryoji アイコンの未定義化(-1)
			tbd[i],				//	tbd[i].idCommand,
			TBSTATE_ENABLED,	//	tbd[i].fsState,
			style /*TBSTYLE_BUTTON*/,		//	tbd[i].fsStyle,
			0,					//	tbd[i].dwData,
			0					//	tbd[i].iString
		);
	}

	// ツールバー改行用の仮想ボタン（実際は表示されない） // 20050809 aroka
	//	2007.10.12 genta 折り返しボタンが最後のデータと重なっているが，
	//	インデックスを変更するとsakura.iniが引き継げなくなるので
	//	重複を承知でそのままにする
	SetTBBUTTONVal(
		&m_tbMyButton[tbd_num],
		-1,						// 2007.11.02 ryoji アイコンの未定義化(-1)
		F_MENU_NOT_USED_FIRST,			//	tbd[i].idCommand,
		TBSTATE_ENABLED|TBSTATE_WRAP,	//	tbd[i].fsState,
		TBSTYLE_SEP,			//	tbd[i].fsStyle,
		0,						//	tbd[i].dwData,
		0						//	tbd[i].iString
	);
	m_nMyButtonNum = tbd_num + 1;	//	+ 1は、セパレータの分
	return;
}


CMenuDrawer::~CMenuDrawer()
{
	if( NULL != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = NULL;
	}
	if( NULL != m_hFontMenuUndelLine ){
		::DeleteObject( m_hFontMenuUndelLine );
		m_hFontMenuUndelLine = NULL;
	}
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
	int		i;
	LOGFONT	lf;
	for( i = 0; i < m_nMenuItemNum; ++i ){
		m_cmemMenuItemStrArr[i].SetData( "", 0 );
		m_nMenuItemFuncArr[i] = 0;
	}
	m_nMenuItemNum = 0;

	NONCLIENTMETRICS	ncm;
	// 以前のプラットフォームに WINVER >= 0x0600 で定義される構造体のフルサイズを渡すと失敗する	// 2007.12.21 ryoji
	ncm.cbSize = CCSIZEOF_STRUCT( NONCLIENTMETRICS, lfMessageFont );
	::SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm.cbSize, (PVOID)&ncm, 0 );

	m_nMenuHeight = ncm.iMenuHeight;
	if( 21 > m_nMenuHeight ){
		m_nMenuHeight = 21;
	}

	if( NULL != m_hFontMenu ){
		::DeleteObject( m_hFontMenu );
		m_hFontMenu = NULL;
	}
	if( NULL != m_hFontMenuUndelLine ){
		::DeleteObject( m_hFontMenuUndelLine );
		m_hFontMenuUndelLine = NULL;
	}
	lf = ncm.lfMenuFont;
	m_hFontMenu = ::CreateFontIndirect( &lf );
	lf.lfUnderline = TRUE;
	m_hFontMenuUndelLine = ::CreateFontIndirect( &lf );
//@@@ 2002.01.03 YAZAKI 不使用のため
//	m_nMaxTab = 0;
//	m_nMaxTabLen = 0;
	return;
}




/* メニューアイテムの描画サイズを計算 */
int CMenuDrawer::MeasureItem( int nFuncID, int* pnItemHeight )
{


	*pnItemHeight = m_nMenuHeight;
	const char* pszLabel;
	RECT rc;
	HDC hdc;
	HFONT hFontOld;

	if( NULL == ( pszLabel = GetLabel( nFuncID ) ) ){
		return 0;
	}
	hdc = ::GetDC( m_hWndOwner );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );
	::DrawText( hdc, pszLabel, strlen( pszLabel ), &rc, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_CALCRECT );
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWndOwner, hdc );


//	*pnItemHeight = 20;
//	*pnItemHeight = 2 + 15 + 1;
	//@@@ 2002.2.2 YAZAKI Windowsの設定でメニューのフォントを大きくすると表示が崩れる問題に対処
	*pnItemHeight = GetSystemMetrics(SM_CYMENU);

	return rc.right - rc.left + 20 + 8;
//	return m_nMaxTab + 16 + m_nMaxTabLen;

}

/* メニュー項目を追加 */
//void CMenuDrawer::MyAppendMenu( HMENU hMenu, int nFlag, int nFuncId, const char* pszLabel, BOOL bAddKeyStr )
void CMenuDrawer::MyAppendMenu( HMENU hMenu, int nFlag, int nFuncId, const char* pszLabel, BOOL bAddKeyStr, int nForceIconId )	//お気に入り	//@@@ 2003.04.08 MIK
{
	char		szLabel[256];
	int			nFlagAdd = 0;
	int			i;

	if( nForceIconId == -1 ) nForceIconId = nFuncId;	//お気に入り	//@@@ 2003.04.08 MIK

	szLabel[0] = '\0';
	if( NULL != pszLabel ){
		strncpy( szLabel, pszLabel, sizeof( szLabel ) / sizeof( szLabel[0] ) - 1 );
		szLabel[ sizeof( szLabel ) / sizeof( szLabel[0] ) - 1 ] = '\0';
	}
	if( nFuncId != 0 ){
		/* メニューラベルの作成 */
		CKeyBind::GetMenuLabel(
			m_hInstance,
			m_pShareData->m_nKeyNameArrNum,
			m_pShareData->m_pKeyNameArr,
			nFuncId,
			szLabel,
			bAddKeyStr
		 );

		/* アイコン用ビットマップを持つものは、オーナードロウにする */

		if( m_nMenuItemNum + 1 > MAX_MENUITEMS ){
			::MYMESSAGEBOX(	NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
				"CMenuDrawer::MyAppendMenu()エラー\n\nCMenuDrawerが管理できるメニューアイテムの上限はCMenuDrawer::MAX_MENUITEMS==%dです。\n ", MAX_MENUITEMS
			);
		}else{

			m_nMenuItemBitmapIdxArr[m_nMenuItemNum] = -1;
			m_nMenuItemFuncArr[m_nMenuItemNum] = nFuncId;
			m_cmemMenuItemStrArr[m_nMenuItemNum].SetData( szLabel, strlen( szLabel ) );
//#ifdef _DEBUG
			/* メニュー項目をオーナー描画にする */
			/* メニューにアイコンを表示する */
			if( m_pShareData->m_Common.m_bMenuIcon ){
				nFlagAdd = MF_OWNERDRAW;
			}
//#endif
			/* 機能のビットマップがあるかどうか調べておく */
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
//			for( i = 0; i < m_cShareData.m_nMyButtonNum; ++i ){
//				if( nFuncId == m_cShareData.m_tbMyButton[i].idCommand ){
//					/* 機能のビットマップの情報を覚えておく */
//					m_nMenuItemBitmapIdxArr[m_nMenuItemNum] = m_cShareData.m_tbMyButton[i].iBitmap;
//					break;
//				}
//			}
			for( i = 0; i < m_nMyButtonNum; ++i ){
				//if( nFuncId == m_tbMyButton[i].idCommand ){
				if( nForceIconId == m_tbMyButton[i].idCommand ){	//お気に入り	//@@@ 2003.04.08 MIK
					/* 機能のビットマップの情報を覚えておく */
					m_nMenuItemBitmapIdxArr[m_nMenuItemNum] = m_tbMyButton[i].iBitmap;
					break;
				}
			}
			m_nMenuItemNum++;
		}
	}

	MENUITEMINFO mii;
	memset( &mii, 0, sizeof( MENUITEMINFO ) );
	//	Aug. 31, 2001 genta
#ifdef _WIN64
	mii.cbSize = sizeof( MENUITEMINFO ); // 64bit版ではサイズ違う
#else
	//mii.cbSize = sizeof( MENUITEMINFO ); // 本当はこちらの書き方が正しいが，
	mii.cbSize = SIZEOF_MENUITEMINFO; // サイズが大きいとWin95で動かないので，Win95が納得する値を決め打ち
#endif
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




/*! メニューアイテム描画
	@date 2001.12.21 YAZAKI デバッグモードでもメニューを選択したらハイライト。
	@date 2003.08.27 Moca システムカラーのブラシはCreateSolidBrushをやめGetSysColorBrushに
*/
void CMenuDrawer::DrawItem( DRAWITEMSTRUCT* lpdis )
{

//	int			i;
	int			j;
	int			nItemIndex;
	HDC			hdc;
	const char*	pszItemStr;
	int			nItemStrLen;
//	TEXTMETRIC	tm;
//	SIZE		sz;
	HFONT		hFontOld;
	int			nIndentLeft;
	int			nIndentRight;
	int			nTextTopMargin;
	RECT		rc1;
	HBRUSH		hBrush;
	RECT		rcText;
	int			nBkModeOld;

	nIndentLeft = 26;
	nIndentRight = 8;
	nTextTopMargin = 3;

/*	for( i = 0; i < m_nMenuItemNum; ++i ){
		if( (int)lpdis->itemID == m_nMenuItemFuncArr[i] ){
			break;
		}
	}
	if( i >= m_nMenuItemNum ){
		return;
	}
	nItemIndex = i;
*/
//@@@ 2002.01.03 YAZAKI 極力メンバ関数を使用するように。
	nItemIndex = Find( (int)lpdis->itemID );
	pszItemStr = m_cmemMenuItemStrArr[nItemIndex].GetPtr( &nItemStrLen );

//	hdc = ::GetDC( m_hWndOwner );
	hdc = lpdis->hDC;
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontMenu );


	/* アイテム矩形塗りつぶし */
//	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_MENU ) );
	hBrush = ::GetSysColorBrush( COLOR_MENU );
	::FillRect( hdc, &lpdis->rcItem, hBrush );
//	::DeleteObject( hBrush );


	/* アイテムが選択されている */
	nBkModeOld = ::SetBkMode( hdc, TRANSPARENT );
	if( lpdis->itemState & ODS_SELECTED ){
//		hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT/*COLOR_3DHIGHLIGHT*/ ) );
		hBrush = ::GetSysColorBrush( COLOR_HIGHLIGHT );
		rc1 = lpdis->rcItem;
		if( -1 != m_nMenuItemBitmapIdxArr[nItemIndex] || lpdis->itemState & ODS_CHECKED ){
			rc1.left += (nIndentLeft - 2);
		}
		/* 選択ハイライト矩形 */
		::FillRect( hdc, &rc1, hBrush );
//		::DeleteObject( hBrush );

		/* アイテムが使用不可 */
		if( lpdis->itemState & ODS_DISABLED ){
			::SetTextColor( hdc, ::GetSysColor( COLOR_MENU/*COLOR_3DSHADOW*/ ) );
		}else{
			::SetTextColor( hdc, ::GetSysColor( COLOR_HIGHLIGHTTEXT/*COLOR_MENUTEXT*//*COLOR_3DHIGHLIGHT*/ ) );
		}
	}else{
		/* アイテムが使用不可 */
		if( lpdis->itemState & ODS_DISABLED ){
			::SetTextColor( hdc, ::GetSysColor( COLOR_3DSHADOW ) );
		}else{
			::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
		}
	}

#ifdef _DEBUG
	char	szText[1024];
	MENUITEMINFO mii;
	// メニュー項目に関する情報を取得します。
	memset( &mii, 0, sizeof( MENUITEMINFO ) );
#ifdef _WIN64
	mii.cbSize = sizeof( MENUITEMINFO ); // 64bit版ではサイズ違う
#else
	mii.cbSize = SIZEOF_MENUITEMINFO; // Jan. 29, 2002 genta
#endif
	mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
	mii.fType = MFT_STRING;
	strcpy( szText, "--unknown--" );
	mii.dwTypeData = (LPTSTR)szText;
	mii.cch = sizeof( szText ) - 1;
	if( 0 != ::GetMenuItemInfo( (HMENU)lpdis->hwndItem, lpdis->itemID, FALSE, (MENUITEMINFO*)&mii )
	 && NULL == mii.hSubMenu
	 && 0 == /* CEditWnd */::FuncID_To_HelpContextID( lpdis->itemID ) 	/* 機能IDに対応するメニューコンテキスト番号を返す */
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
	rcText.left += nIndentLeft;
	rcText.right -= nIndentRight;

	/* TAB文字の前と後ろに分割してテキストを描画する */
	for( j = 0; j < nItemStrLen; ++j ){
		if( pszItemStr[j] == '\t' ){
			break;
		}
	}
	/* TAB文字の後ろ側のテキストを描画する */
	if( j < nItemStrLen ){
		/* アイテムが使用不可 */
		if( lpdis->itemState & ODS_DISABLED && !(lpdis->itemState & ODS_SELECTED)  ){
			COLORREF colOld = ::SetTextColor( hdc, ::GetSysColor( COLOR_3DHIGHLIGHT ) );
				rcText.left++;
				rcText.top++;
				rcText.right++;
				rcText.bottom++;
				::DrawText( hdc, &pszItemStr[j + 1], strlen( &pszItemStr[j + 1] ), &rcText, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_RIGHT );
				rcText.left--;
				rcText.top--;
				rcText.right--;
				rcText.bottom--;
				::SetTextColor( hdc, colOld );
		}
		::DrawText( hdc, &pszItemStr[j + 1], strlen( &pszItemStr[j + 1] ), &rcText, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_RIGHT );
	}
	/* TAB文字の前側のテキストを描画する */
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
	::DrawText( hdc, pszItemStr, j, &rcText, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS | DT_LEFT );
	::SelectObject( hdc, hFontOld  );
	::SetBkMode( hdc, nBkModeOld );

	/* チェック状態なら凹んだ3D枠を描画する */
	if( lpdis->itemState & ODS_CHECKED ){
		/* アイコンを囲む枠 */
		CSplitBoxWnd::Draw3dRect(
			hdc, lpdis->rcItem.left + 1, lpdis->rcItem.top,
			2 + 16 + 2, lpdis->rcItem.bottom - lpdis->rcItem.top,
			::GetSysColor( COLOR_3DSHADOW ),
			::GetSysColor( COLOR_3DHILIGHT )
		);
		/* アイテムが選択されていない場合は3D枠の中を明るく塗りつぶす */
		if( lpdis->itemState & ODS_SELECTED ){
		}else{
//			HBRUSH hbr = ::CreateSolidBrush( ::GetSysColor( COLOR_3DHILIGHT ) );
//			HBRUSH hbr = ::CreateSolidBrush( ::GetSysColor( COLOR_3DLIGHT ) );
			HBRUSH hbr = ::GetSysColorBrush( COLOR_3DLIGHT );
			HBRUSH hbrOld = (HBRUSH)::SelectObject( hdc, hbr );
			RECT rc;
			::SetRect( &rc, lpdis->rcItem.left + 1 + 1, lpdis->rcItem.top + 1, lpdis->rcItem.left + 1 + 1 + 16 + 2, lpdis->rcItem.top + 1+ 15 + 2 );
			::FillRect( hdc, &rc, hbr );
			::SelectObject( hdc, hbrOld );
//			::DeleteObject( hbr );
		}
	}


	/* 機能の画像が存在するならメニューアイコン?を描画する */
	if( -1 != m_nMenuItemBitmapIdxArr[nItemIndex] ){
		/* 3D枠を描画する */
		/* アイテムが選択されている */
		if( lpdis->itemState & ODS_SELECTED ){
			/* アイテムが使用不可 */
			if( lpdis->itemState & ODS_DISABLED /*&& !(lpdis->itemState & ODS_SELECTED)*/  ){
			}else{
				if( lpdis->itemState & ODS_CHECKED ){
				}else{
					/* アイコンを囲む枠 */
					CSplitBoxWnd::Draw3dRect(
						hdc, lpdis->rcItem.left + 1, lpdis->rcItem.top,
						2 + 16 + 2, lpdis->rcItem.bottom - lpdis->rcItem.top,
						::GetSysColor( COLOR_3DHILIGHT ),
						::GetSysColor( COLOR_3DSHADOW )
					 );
				}
			}
		}

		/* アイテムが使用不可 */
		if( lpdis->itemState & ODS_DISABLED ){
			/* 淡色アイコン */
			// 2003.09.04 Moca SetTextColorする必要は無い
//			COLORREF cOld;
//			cOld = SetTextColor( hdc, GetSysColor(COLOR_3DSHADOW) );	//Oct. 24, 2000 これは標準ではRGB(128,128,128)と同じ
//			cOld = SetTextColor( hdc, RGB(132,132,132) );	//Oct. 24, 2000 JEPRO もう少し薄くした
			m_pcIcons->Draw( m_nMenuItemBitmapIdxArr[nItemIndex],
				hdc,	//	Target DC
				lpdis->rcItem.left + 1,	//	X
				//@@@ 2002.1.29 YAZAKI Windowsの設定でメニューのフォントを大きくすると表示が崩れる問題に対処
				//lpdis->rcItem.top + 1,		//	Y
				lpdis->rcItem.top + GetSystemMetrics(SM_CYMENU)/2 - 8,	//	Y
				ILD_MASK
			);
//			SetTextColor( hdc, cOld );

		}else{
/*
			COLORREF colBk;
			if( lpdis->itemState & ODS_CHECKED && !( lpdis->itemState & ODS_SELECTED ) ){
				colBk = ::GetSysColor( COLOR_3DLIGHT );
//				colBk = ::GetSysColor( COLOR_3DHILIGHT );
			}else{
				colBk = ::GetSysColor( COLOR_MENU );
			}
*/
			/* 通常のアイコン */
			m_pcIcons->Draw( m_nMenuItemBitmapIdxArr[nItemIndex],
				hdc,	//	Target DC
				lpdis->rcItem.left + 1 + 1,	//	X
				//@@@ 2002.1.29 YAZAKI Windowsの設定でメニューのフォントを大きくすると表示が崩れる問題に対処
				lpdis->rcItem.top + GetSystemMetrics(SM_CYMENU)/2 - 8,	//	Y
				//lpdis->rcItem.top + 1,		//	Y
				ILD_NORMAL
			);
		}

	}else{
		if( lpdis->itemState & ODS_CHECKED ){
			/* アイコンがないチェックマークの表示 */
			int nX, nY;
			nX = lpdis->rcItem.left + 1 + 8;
			nY = lpdis->rcItem.top + 8;
			::MoveToEx( hdc, nX, nY, NULL );
			::LineTo( hdc, nX + 1, nY + 1 );
			::LineTo( hdc, nX + 4, nY - 2 );

			nY++;
			::MoveToEx( hdc, nX, nY, NULL );
			::LineTo( hdc, nX + 1, nY + 1 );
			::LineTo( hdc, nX + 4, nY - 2 );

		}
	}

	//	::ReleaseDC( m_hWndOwner, hdc );
	return;
}


/** コマンドコードからツールバーボタン情報のINDEXを得る

	@param idCommand [in] コマンドコード
	@param bOnlyFunc [in] 有効な機能の範囲で検索する

	@retval みつからなければ-1を返す。

	@date 2005.08.09 aroka m_nMyButtonNum隠蔽のため追加
	@date 2005.11.02 ryoji bOnlyFuncパラメータを追加
 */
int CMenuDrawer::FindIndexFromCommandId( int idCommand, bool bOnlyFunc )
{
	if( bOnlyFunc ){
		// 機能の範囲外（セパレータや折り返しなど特別なもの）は除外する
		if ( !( F_MENU_FIRST <= idCommand && idCommand < F_MENU_NOT_USED_FIRST ) ){
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

	@param index [in] ボタン情報のインデックス
	@retval ボタン情報

	@date 2007.11.02 ryoji 範囲外の場合は未定義のボタン情報を返すように
 */
TBBUTTON CMenuDrawer::getButton( int index ) const
{
	if( 0 <= index && index < m_nMyButtonNum )
		return m_tbMyButton[index];

	// 範囲外なら未定義のボタン情報を作成して返す
	// （sakura.iniに範囲外インデックスが指定があった場合など、堅牢性のため）
	TBBUTTON tbb;
	SetTBBUTTONVal( &tbb, -1, F_DISABLE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 );
	return tbb;
}

int CMenuDrawer::Find( int nFuncID )
{
	int i;
	for( i = 0; i < m_nMenuItemNum; ++i ){
		if( (int)nFuncID == m_nMenuItemFuncArr[i] ){
			break;
		}
	}
	if( i >= m_nMenuItemNum ){
		return -1;
	}else{
		return i;
	}
}


const char* CMenuDrawer::GetLabel( int nFuncID )
{
	int i;
	if( -1 == ( i = Find( nFuncID ) ) ){
		return NULL;
	}
	return m_cmemMenuItemStrArr[i].GetPtr();
}

char CMenuDrawer::GetAccelCharFromLabel( const char* pszLabel )
{
	int i;
	for( i = 0; i + 1 < (int)strlen( pszLabel ); ++i ){
		if( '&' == pszLabel[i] ){
			if( '&' == pszLabel[i + 1]  ){
				i++;
			}else{
				return toupper( pszLabel[i + 1] );
			}
		}
	}
	return (char)0;
}





/*! メニューアクセスキー押下時の処理(WM_MENUCHAR処理) */
LRESULT CMenuDrawer::OnMenuChar( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	TCHAR				chUser;
	UINT				fuFlag;
	HMENU				hmenu;
//	HMENU				hMenu;
	MENUITEMINFO		mii;
	int i;
	chUser = (TCHAR) LOWORD(wParam);	// character code
	fuFlag = (UINT) HIWORD(wParam);		// menu flag
	hmenu = (HMENU) lParam;				// handle to menu
//	MYTRACE( "::GetMenuItemCount( %xh )==%d\n", hmenu, ::GetMenuItemCount( hmenu ) );

	//	Oct. 27, 2000 genta
	if( 0 <= chUser && chUser < ' '){
		chUser += '@';
	}
	else {
		chUser = toupper( chUser );
	}

	struct WorkData{
		int				idx;
		MENUITEMINFO	mii;
	};

	WorkData vecAccel[100];
	int nAccelNum;
	int nAccelSel;
	nAccelNum = 0;
	nAccelSel = 99999;
	for( i = 0; i < ::GetMenuItemCount( hmenu ); i++ ){
		char	szText[1024];
		// メニュー項目に関する情報を取得します。
		memset( &mii, 0, sizeof( MENUITEMINFO ) );
#ifdef _WIN64
	mii.cbSize = sizeof( MENUITEMINFO ); // 64bit版ではサイズ違う
#else
		// Jan. 29, 2002 gentaWinNT4でアクセラレータが効かないのが直るはず
		mii.cbSize = SIZEOF_MENUITEMINFO;
#endif
		mii.fMask = MIIM_CHECKMARKS | MIIM_DATA | MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
		mii.fType = MFT_STRING;
		strcpy( szText, "--unknown--" );
		mii.dwTypeData = (LPTSTR)szText;
		mii.cch = sizeof( szText ) - 1;
		if( 0 == ::GetMenuItemInfo( hmenu, i, TRUE, (MENUITEMINFO*)&mii ) ){
			continue;
		}
		const char* pszLabel;
		if( NULL == ( pszLabel = GetLabel( mii.wID ) ) ){
			continue;
		}
		if( chUser == GetAccelCharFromLabel( pszLabel ) ){
			vecAccel[nAccelNum].idx = i;
			vecAccel[nAccelNum].mii = mii;
			if( /*-1 == nAccelSel ||*/ MFS_HILITE & mii.fState ){
				nAccelSel = nAccelNum;
			}
			nAccelNum++;
		}
	}
//	MYTRACE( "%d\n", (int)mapAccel.size() );
	if( 0 == nAccelNum ){
		return  MAKELONG( 0, MNC_IGNORE );
	}
	if( 1 == nAccelNum ){
		return  MAKELONG( vecAccel[0].idx, MNC_EXECUTE );
	}
//	MYTRACE( "nAccelSel=%d nAccelNum=%d\n", nAccelSel, nAccelNum );
	if( nAccelSel + 1 >= nAccelNum ){
//		MYTRACE( "vecAccel[0].idx=%d\n", vecAccel[0].idx );
		return  MAKELONG( vecAccel[0].idx, MNC_SELECT );
	}else{
//		MYTRACE( "vecAccel[nAccelSel + 1].idx=%d\n", vecAccel[nAccelSel + 1].idx );
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

/*[EOF]*/
