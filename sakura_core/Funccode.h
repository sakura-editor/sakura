//	$Id$
/*!	@file
	機能に関するいろいろ
	
	機能番号定義

	@author Norio Nakatani
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

#ifndef _FUNCCODE_H_
#define _FUNCCODE_H_

/* 未定義用(ダミーとしても使う) */	//Oct. 17, 2000 jepro noted
#define F_DISABLE		0	//未使用


/* ファイル操作系 */
#define F_FILENEW					30101	//新規作成
#define F_FILEOPEN					30102	//ファイルを開く
#define F_FILESAVE					30103	//上書き保存
#define F_FILESAVEAS				30104	//名前を付けて保存
#define F_FILECLOSE					30105	//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
#define F_FILECLOSE_OPEN			30107	//閉じて開く
#define F_FILE_REOPEN_SJIS			30111	//SJISで開き直す
#define F_FILE_REOPEN_JIS			30112	//JISで開き直す
#define F_FILE_REOPEN_EUC			30113	//EUCで開き直す
#define F_FILE_REOPEN_UNICODE		30114	//Unicodeで開き直す
#define F_FILE_REOPEN_UTF8			30115	//UTF-8で開き直す
#define F_FILE_REOPEN_UTF7			30116	//UTF-7で開き直す
#define F_PRINT						30150	//印刷
#define F_PRINT_PREVIEW				30151	//印刷プレビュー
#define F_PRINT_PAGESETUP			30152	//印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から「印刷ページ設定」に変更
//#define F_PRINT_DIALOG				30151	//印刷ダイアログ
#define F_OPEN_HHPP					30160	//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
#define F_OPEN_CCPP					30161	//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
#define	F_OPEN_HfromtoC				30162	//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
#define F_ACTIVATE_SQLPLUS			30170	/* Oracle SQL*Plusをアクティブ表示 */
#define F_PLSQL_COMPILE_ON_SQLPLUS	30171	/* Oracle SQL*Plusで実行 */	//Sept. 17, 2000 jepro 説明の「コンパイル」を「実行」に統一
#define F_BROWSE					30180	//ブラウズ
#define F_PROPERTY_FILE				30190	/* ファイルのプロパティ */
#define F_EXITALL					30195	/* テキストエディタの全終了 */	//Dec. 27, 2000 JEPRO 追加


/* 編集系 */
#define F_CHAR				30200	//文字入力
#define F_IME_CHAR			30201	//全角文字入力
#define F_UNDO				30210	//元に戻す(Undo)
#define F_REDO				30211	//やり直し(Redo)
#define F_DELETE			30221	//カーソル位置を削除
#define F_DELETE_BACK		30222	//カーソルの前を削除
#define F_WordDeleteToStart	30230	//単語の左端まで削除
#define F_WordDeleteToEnd	30231	//単語の右端まで削除
#define F_WordCut			30232	//単語切り取り
#define F_WordDelete		30233	//単語削除
#define F_LineCutToStart	30240	//行頭まで切り取り(改行単位)
#define F_LineCutToEnd		30241	//行末まで切り取り(改行単位)
#define F_LineDeleteToStart	30242	//行頭まで削除(改行単位)
#define F_LineDeleteToEnd	30243	//行末まで削除(改行単位)
#define F_CUT_LINE			30244	//行切り取り(折り返し単位)
#define F_DELETE_LINE		30245	//行削除(折り返し単位)
#define F_DUPLICATELINE		30250	//行の二重化(折り返し単位)
#define F_INDENT_TAB		30260	//TABインデント
#define F_UNINDENT_TAB		30261	//逆TABインデント
#define F_INDENT_SPACE		30262	//SPACEインデント
#define F_UNINDENT_SPACE	30263	//逆SPACEインデント
#define F_WORDSREFERENCE	30270	//単語リファレンス


/* カーソル移動系 */
#define F_UP				30311	//カーソル上移動
#define F_DOWN				30312	//カーソル下移動
#define F_LEFT				30313	//カーソル左移動
#define F_RIGHT				30314	//カーソル右移動
#define F_UP2				30315	//カーソル上移動(２行ごと)
#define F_DOWN2				30316	//カーソル下移動(２行ごと)
#define F_WORDLEFT			30320	//単語の左端に移動
#define F_WORDRIGHT			30321	//単語の右端に移動
//#define F_GOLINETOP		30330	//行頭に移動(改行単位)
//#define F_GOLINEEND		30331	//行末に移動(改行単位)
#define F_GOLINETOP			30332	//行頭に移動(折り返し単位)
#define F_GOLINEEND			30333	//行末に移動(折り返し単位)
//#define F_ROLLDOWN			30340	//スクロールダウン//Oct. 10, 2000 JEPRO 下の半ページアップに名称変更
//#define F_ROLLUP			30341	//スクロールアップ//Oct. 10, 2000 JEPRO 下の半ページダウンに名称変更
#define F_HalfPageUp		30340	//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
#define F_HalfPageDown		30341	//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
#define F_1PageUp			30342	//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
#define F_1PageDown			30343	//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
//#define F_DISPLAYTOP		30344	//画面の先頭に移動(未実装)
//#define F_DISPLAYEND		30345	//画面の最後に移動(未実装)
#define F_GOFILETOP			30350	//ファイルの先頭に移動
#define F_GOFILEEND			30351	//ファイルの最後に移動
#define F_CURLINECENTER		30360	//カーソル行をウィンドウ中央へ
#define F_JUMPPREV			30370	//移動履歴: 前へ
#define F_JUMPNEXT			30371	//移動履歴: 次へ
#define F_WndScrollUp		30380	//画面を上へ１行スクロール	// 2001/06/20 asa-o
#define F_WndScrollDown		30381	//画面を下へ１行スクロール	// 2001/06/20 asa-o


/* 選択系 */
#define F_SELECTWORD		30400	//現在位置の単語選択
#define F_SELECTALL			30401	//すべて選択
#define F_BEGIN_SEL			30410	//範囲選択開始
#define F_UP_SEL			30411	//(範囲選択)カーソル上移動
#define F_DOWN_SEL			30412	//(範囲選択)カーソル下移動
#define F_LEFT_SEL			30413	//(範囲選択)カーソル左移動
#define F_RIGHT_SEL			30414	//(範囲選択)カーソル右移動
#define F_UP2_SEL			30415	//(範囲選択)カーソル上移動(２行ごと)
#define F_DOWN2_SEL			30416	//(範囲選択)カーソル下移動(２行ごと)
#define F_WORDLEFT_SEL		30420	//(範囲選択)単語の左端に移動
#define F_WORDRIGHT_SEL		30421	//(範囲選択)単語の右端に移動
//#define F_GOLINETOP_SEL		30430	//(範囲選択)行頭に移動(改行単位)
//#define F_GOLINEEND_SEL		30431	//(範囲選択)行末に移動(改行単位)
#define F_GOLINETOP_SEL		30432	//(範囲選択)行頭に移動(折り返し単位)
#define F_GOLINEEND_SEL		30433	//(範囲選択)行末に移動(折り返し単位)
//#define F_ROLLDOWN_SEL		30440	//(範囲選択)スクロールダウン//Oct. 10, 2000 JEPRO 下の半ページアップに名称変更
//#define F_ROLLUP_SEL		30441	//(範囲選択)スクロールアップ//Oct. 10, 2000 JEPRO 下の半ページダウンに名称変更
#define F_HalfPageUp_Sel	30440	//(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
#define F_HalfPageDown_Sel	30441	//(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
#define F_1PageUp_Sel		30442	//(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
#define F_1PageDown_Sel		30443	//(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
//#define F_DISPLAYTOP_SEL	30444	//(範囲選択)画面の先頭に移動(未実装)
//#define F_DISPLAYEND_SEL	30445	//(範囲選択)画面の最後に移動(未実装)
#define F_GOFILETOP_SEL		30450	//(範囲選択)ファイルの先頭に移動
#define F_GOFILEEND_SEL		30451	//(範囲選択)ファイルの最後に移動


/* 矩形選択系 */
//#define F_BOXSELALL		30500	//矩形ですべて選択
#define	F_BEGIN_BOX			30510	//矩形範囲選択開始
//Oct. 17, 2000 JEPRO 以下に矩形選択のコマンド名のみ準備しておいた
//#define F_UP_BOX			30511	//(矩形選択)カーソル上移動
//#define F_DOWN_BOX			30512	//(矩形選択)カーソル下移動
//#define F_LEFT_BOX			30513	//(矩形選択)カーソル左移動
//#define F_RIGHT_BOX			30514	//(矩形選択)カーソル右移動
//#define F_UP2_BOX			30515	//(矩形選択)カーソル上移動(２行ごと)
//#define F_DOWN2_BOX			30516	//(矩形選択)カーソル下移動(２行ごと)
//#define F_WORDLEFT_BOX		30520	//(矩形選択)単語の左端に移動
//#define F_WORDRIGHT_BOX		30521	//(矩形選択)単語の右端に移動
////#define F_GOLINETOP_BOX		30530	//(矩形選択)行頭に移動(改行単位)
////#define F_GOLINEEND_BOX		30531	//(矩形選択)行末に移動(改行単位)
//#define F_GOLINETOP_BOX		30532	//(矩形選択)行頭に移動(折り返し単位)
//#define F_GOLINEEND_BOX		30533	//(矩形選択)行末に移動(折り返し単位)
//#define F_HalfPageUp_Box	30540	//(矩形選択)半ページアップ
//#define F_HalfPageDown_Box	30541	//(矩形選択)半ページダウン
//#define F_1PageUp_Box		30542	//(矩形選択)１ページアップ
//#define F_1PageDown_Box		30543	//(矩形選択)１ページダウン
////#define F_DISPLAYTOP_BOX	30444	//(矩形選択)画面の先頭に移動(未実装)
////#define F_DISPLAYEND_BOX	30445	//(矩形選択)画面の最後に移動(未実装)
//#define F_GOFILETOP_BOX		30550	//(矩形選択)ファイルの先頭に移動
//#define F_GOFILEEND_BOX		30551	//(矩形選択)ファイルの最後に移動


/* クリップボード系 */
#define F_CUT						30601	//切り取り(選択範囲をクリップボードにコピーして削除)
#define F_COPY						30602	//コピー(選択範囲をクリップボードにコピー)
#define F_COPY_CRLF					30603	//CRLF改行でコピー
#define F_PASTE						30604	//貼り付け(クリップボードから貼り付け)
#define F_PASTEBOX					30605	//矩形貼り付け(クリップボードから矩形貼り付け)
#define F_INSTEXT					30606	//テキストを貼り付け
#define F_ADDTAIL					30607	//最後にテキストを追加
#define F_COPYLINES					30610	//選択範囲内全行コピー
#define F_COPYLINESASPASSAGE		30611	//選択範囲内全行引用符付きコピー
#define F_COPYLINESWITHLINENUMBER	30612	//選択範囲内全行行番号付きコピー
#define F_COPYPATH					30620	//このファイルのパス名をクリップボードにコピー
#define F_COPYTAG					30621	//このファイルのパス名とカーソル位置をコピー
//Sept. 16, 2000 JEPRO ショートカットキーがうまく働かないので次行は殺してある	//Dec. 25, 2000 復活
#define F_CREATEKEYBINDLIST			30630	//キー割り当て一覧をコピー //Sept. 15, 2000 JEPRO 上の行はIDM_TESTのままではうまくいかないのでFに変えて登録


/* 挿入系 */
#define F_INS_DATE				30790	//日付挿入
#define F_INS_TIME				30791	//時刻挿入


/* 変換系 */
#define F_TOLOWER				30800	//英大文字→英小文字
#define F_TOUPPER				30801	//英小文字→英大文字
#define F_TOHANKAKU				30810	/* 全角→半角 */
#define F_TOZENKAKUKATA			30811	/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
#define F_TOZENKAKUHIRA			30812	/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
#define F_HANKATATOZENKAKUKATA	30813	/* 半角カタカナ→全角カタカナ */
#define F_HANKATATOZENKAKUHIRA	30814	/* 半角カタカナ→全角ひらがな */
#define F_TABTOSPACE			30830	/* TAB→空白 */
#define F_SPACETOTAB			30831	/* 空白→TAB *///#### Stonee, 2001/05/27
#define F_CODECNV_AUTO2SJIS		30850	/* 自動判別→SJISコード変換 */
#define F_CODECNV_EMAIL			30851	//E-Mail(JIS→SJIS)コード変換
#define F_CODECNV_EUC2SJIS		30852	//EUC→SJISコード変換
#define F_CODECNV_UNICODE2SJIS	30853	//Unicode→SJISコード変換
#define F_CODECNV_UTF82SJIS		30854	/* UTF-8→SJISコード変換 */
#define F_CODECNV_UTF72SJIS		30855	/* UTF-7→SJISコード変換 */
#define F_CODECNV_SJIS2JIS		30860	/* SJIS→JISコード変換 */
#define F_CODECNV_SJIS2EUC		30861	/* SJIS→EUCコード変換 */
#define F_CODECNV_SJIS2UTF8		30862	/* SJIS→UTF-8コード変換 */
#define F_CODECNV_SJIS2UTF7		30863	/* SJIS→UTF-7コード変換 */
#define F_BASE64DECODE			30870	//Base64デコードして保存
#define F_UUDECODE				30880	//uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更


/* 検索系 */
#define F_SEARCH_DIALOG		30901	//検索(単語検索ダイアログ)
#define F_SEARCH_NEXT		30902	//次を検索
#define F_SEARCH_PREV		30903	//前を検索
#define F_REPLACE			30904	//置換(置換ダイアログ)
#define F_SEARCH_CLEARMARK	30905	//検索マークのクリア
#define F_GREP				30910	//Grep
#define F_JUMP				30920	//指定行へジャンプ
#define F_OUTLINE			30930	//アウトライン解析
#define F_TAGJUMP			30940	//タグジャンプ機能
#define F_TAGJUMPBACK		30941	//タグジャンプバック機能
#define F_COMPARE			30950	//ファイル内容比較
#define F_BRACKETPAIR		30960	//対括弧の検索


/* モード切り替え系 */
#define F_CHGMOD_INS		31001	//挿入／上書きモード切り替え
#define F_CANCEL_MODE		31099	//各種モードの取り消し


/* 設定系 */
#define F_SHOWTOOLBAR		31100	/* ツールバーの表示 */
#define F_SHOWFUNCKEY		31101	/* ファンクションキーの表示 */
#define F_SHOWSTATUSBAR		31102	/* ステータスバーの表示 */
#define F_TYPE_LIST			31110	/* タイプ別設定一覧 */
#define F_OPTION_TYPE		31111	/* タイプ別設定 */
#define F_OPTION			31112	/* 共通設定 */

//From here 設定ダイアログ用の機能番号を用意  Stonee, 2001/05/18
#define F_TYPE_SCREEN		31115	/* タイプ別設定『スクリーン』 */
#define F_TYPE_COLOR		31116	/* タイプ別設定『カラー』 */
#define F_OPTION_GENERAL	32000	/* 共通設定『全般』 */
#define F_OPTION_WINDOW		32001	/* 共通設定『ウィンドウ』 */
#define F_OPTION_EDIT		32002	/* 共通設定『編集』 */
#define F_OPTION_FILE		32003	/* 共通設定『ファイル』 */
#define F_OPTION_BACKUP		32004	/* 共通設定『バックアップ』 */
#define F_OPTION_FORMAT		32005	/* 共通設定『書式』 */
#define F_OPTION_URL		32006	/* 共通設定『クリッカブルURL』 */
#define F_OPTION_GREP		32007	/* 共通設定『Grep』 */
#define F_OPTION_KEYBIND	32008	/* 共通設定『キー割り当て』 */
#define F_OPTION_CUSTMENU	32009	/* 共通設定『カスタムメニュー』 */
#define F_OPTION_TOOLBAR	32010	/* 共通設定『ツールバー』 */
#define F_OPTION_KEYWORD	32011	/* 共通設定『強調キーワード』 */
#define F_OPTION_HELPER		32012	/* 共通設定『支援』 */
//To here  Stonee, 2001/05/18

#define F_FONT				31120	/* フォント設定 */
#define F_WRAPWINDOWWIDTH	31140	//現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更


/* マクロ系 */
#define F_RECKEYMACRO		31250	/* キーマクロの記録開始／終了 */
#define F_SAVEKEYMACRO		31251	/* キーマクロの保存 */
#define F_LOADKEYMACRO		31252	/* キーマクロの読み込み */
#define F_EXECKEYMACRO		31253	/* キーマクロの実行 */
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	#define F_EXECCMMAND		31270	/* 外部コマンド実行 */
#define F_EXECCOMMAND		31270	/* 外部コマンド実行 */
//	To Here Sept. 20, 2000

//	Jul. 4, 2000 genta
#define F_USERMACRO_0		31200	/* 登録マクロ開始 */
#define SIZE_CUSTMACRO		100		/* 登録できるマクロの数 */


/* カスタムメニュー */
#define F_MENU_RBUTTON		31580	/* 右クリックメニュー */
#define F_CUSTMENU_1		31501	/* カスタムメニュー1 */
#define F_CUSTMENU_2		31502	/* カスタムメニュー2 */
#define F_CUSTMENU_3		31503	/* カスタムメニュー3 */
#define F_CUSTMENU_4		31504	/* カスタムメニュー4 */
#define F_CUSTMENU_5		31505	/* カスタムメニュー5 */
#define F_CUSTMENU_6		31506	/* カスタムメニュー6 */
#define F_CUSTMENU_7		31507	/* カスタムメニュー7 */
#define F_CUSTMENU_8		31508	/* カスタムメニュー8 */
#define F_CUSTMENU_9		31509	/* カスタムメニュー9 */
#define F_CUSTMENU_10		31510	/* カスタムメニュー10 */
#define F_CUSTMENU_11		31511	/* カスタムメニュー11 */
#define F_CUSTMENU_12		31512	/* カスタムメニュー12 */
#define F_CUSTMENU_13		31513	/* カスタムメニュー13 */
#define F_CUSTMENU_14		31514	/* カスタムメニュー14 */
#define F_CUSTMENU_15		31515	/* カスタムメニュー15 */
#define F_CUSTMENU_16		31516	/* カスタムメニュー16 */
#define F_CUSTMENU_17		31517	/* カスタムメニュー17 */
#define F_CUSTMENU_18		31518	/* カスタムメニュー18 */
#define F_CUSTMENU_19		31519	/* カスタムメニュー19 */
#define F_CUSTMENU_20		31520	/* カスタムメニュー20 */
#define F_CUSTMENU_21		31521	/* カスタムメニュー21 */
#define F_CUSTMENU_22		31522	/* カスタムメニュー22 */
#define F_CUSTMENU_23		31523	/* カスタムメニュー23 */
#define F_CUSTMENU_24		31524	/* カスタムメニュー24 */


/* ウィンドウ系 */
#define F_SPLIT_V			31310	//上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
#define F_SPLIT_H			31311	//左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
#define F_SPLIT_VH			31312	//縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
#define F_WINCLOSE			31320	//ウィンドウを閉じる
#define F_WIN_CLOSEALL		31321	//すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
#define F_CASCADE			31330	//重ねて表示
#define F_TILE_V			31331	//上下に並べて表示
#define F_TILE_H			31332	//左右に並べて表示
#define F_NEXTWINDOW		31340	//次のウィンドウ
#define F_PREVWINDOW		31341	//前のウィンドウ
#define F_MAXIMIZE_V		31350	//縦方向に最大化
#define F_MINIMIZE_ALL		31351	//すべて最小化		//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
#define F_MAXIMIZE_H		31352	//横方向に最大化	//2001.02.10 by MIK
#define F_REDRAW			31360	//再描画
#define F_WIN_OUTPUT		31370	//アウトプットウィンドウ表示


/* 支援 */
#define F_HOKAN				31430		/* 入力補完 */
//Sept. 16, 2000→Nov. 25, 2000 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
#define F_HELP_CONTENTS		31440		/* ヘルプ目次 */			//Nov. 25, 2000 JEPRO 追加
#define F_HELP_SEARCH		31441		/* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
#define F_MENU_ALLFUNC		31445		/* コマンド一覧 */
#define F_EXTHELP1			31450		/* 外部ヘルプ１ */
#define F_EXTHTMLHELP		31451		/* 外部HTMLヘルプ */
#define F_ABOUT				31455		/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加


/* その他 */
//#define F_SENDMAIL		31570		/* メール送信 */	//Oct. 17, 2000 JEPRO メール機能は死んでいるのでコメントアウトにした




/* 機能一覧に関するデータ宣言 */
namespace nsFuncCode{
	extern const char*	ppszFuncKind[];
	extern const int	nFuncKindNum;
	extern const int	pnFuncListNumArr[];
	extern const int*	ppnFuncListArr[];
	extern const int	nFincListNumArrNum;
};
///////////////////////////////////////////////////////////////////////
#endif /* _FUNCCODE_H_ */


/*[EOF]*/
