/*!	@file
	@brief 機能分類定義

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, MIK, Stonee, asa-o, Misaka, genta, hor
	Copyright (C) 2002, aroka, minfu, MIK, ai, genta
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, isearch
	Copyright (C) 2005, genta, MIK
	Copyright (C) 2006, aroka, ryoji, fon
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, nasukoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

//	Sept. 14, 2000 Jepro note: functions & commands list
//	キーワード：コマンド一覧順序
//	ここに登録されているコマンドが共通設定の機能種別に表示され、キー割り当てにも設定できるようになる
//	このファイルは「コマンド一覧」のメニューの順番や表示にも使われている
//	sakura_rc.rcファイルの下のほうにあるString Tableも参照のこと

#include "StdAfx.h"
#include <Shlwapi.h>
#include "func/Funccode.h"
#include "config/maxdata.h" //MAX_MRU
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "doc/CEditDoc.h"
#include "_main/CAppMode.h"
#include "CEditApp.h"
#include "agent/CGrepAgent.h"
#include "macro/CSMacroMgr.h"
#include "window/CEditWnd.h"
#include "docplus/CDiffManager.h"
#include "env/CMarkMgr.h"	// CAutoMarkMgr
#include "util/os.h"
#include "sakura.hh"

//using namespace nsFuncCode;

const uint16_t nsFuncCode::ppszFuncKind[] = {
//	"--未定義--",	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更
//	Oct. 16, 2000 JEPRO 表示の順番をメニューバーのそれに合わせるように少し入れ替えた(下の個別のものも全部)
	STR_ERR_DLGFUNCLKUP04,	//L"ファイル操作系",
	STR_ERR_DLGFUNCLKUP05,	//L"編集系",
	STR_ERR_DLGFUNCLKUP06,	//L"カーソル移動系",
	STR_ERR_DLGFUNCLKUP07,	//L"選択系",		//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので「選択系」として独立化(サブメニュー化は構造上できないので)
	STR_ERR_DLGFUNCLKUP08,	//L"矩形選択系",	//Oct. 17, 2000 JEPRO 「選択系」に一緒にすると多くなりすぎるので「矩形選択系」も独立させた
	STR_ERR_DLGFUNCLKUP09,	//L"クリップボード系",
	STR_ERR_DLGFUNCLKUP10,	//L"挿入系",
	STR_ERR_DLGFUNCLKUP11,	//L"変換系",
	STR_ERR_DLGFUNCLKUP12,	//L"検索系",
	STR_ERR_DLGFUNCLKUP13,	//L"モード切り替え系",
	STR_ERR_DLGFUNCLKUP14,	//L"設定系",
	STR_ERR_DLGFUNCLKUP15,	//("マクロ系"),
	//	Oct. 15, 2001 genta カスタムメニューの文字列をは動的に変更可能にするためここからは外す．
//	L"カスタムメニュー",	//Oct. 21, 2000 JEPRO 「その他」から独立分離化
	STR_ERR_DLGFUNCLKUP16,	//L"ウィンドウ系",
	STR_ERR_DLGFUNCLKUP17,	//L"支援",
	STR_ERR_DLGFUNCLKUP18	//L"その他"
};
const int nsFuncCode::nFuncKindNum = int(std::size(nsFuncCode::ppszFuncKind));

/* ファイル操作系 */
const EFunctionCode pnFuncList_File[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List5→List_File)
	F_FILENEW			,	//新規作成
	F_FILENEW_NEWWINDOW	,	//新規ウインドウを開く
	F_FILEOPEN			,	//開く
	F_FILEOPEN_DROPDOWN	,	//開く(ドロップダウン)
	F_FILESAVE			,	//上書き保存
	F_FILESAVEAS_DIALOG	,	//名前を付けて保存
	F_FILESAVEALL		,	//全て上書き保存	// Jan. 24, 2005 genta
	F_FILECLOSE			,	//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	F_FILECLOSE_OPEN	,	//閉じて開く
	F_WINCLOSE			,	//ウィンドウを閉じる	//Oct.17,2000 コマンド本家は「ウィンドウ系」	//Feb. 18, 2001	JEPRO 下から移動した
	F_FILESAVECLOSE		,	//保存して閉じる Feb. 28, 2004 genta
	F_FILE_REOPEN		,	//開き直す	//Dec. 4, 2002 genta
	F_FILE_REOPEN_SJIS		,//SJISで開き直す
	F_FILE_REOPEN_JIS		,//JISで開き直す
	F_FILE_REOPEN_EUC		,//EUCで開き直す
	F_FILE_REOPEN_LATIN1	,//Latin1で開き直す	// 2010/3/20 Uchi
	F_FILE_REOPEN_UNICODE	,//Unicodeで開き直す
	F_FILE_REOPEN_UNICODEBE	,//UnicodeBEで開き直す
	F_FILE_REOPEN_UTF8		,//UTF-8で開き直す
	F_FILE_REOPEN_CESU8		,//CESU-8で開き直す
	F_FILE_REOPEN_UTF7		,//UTF-7で開き直す
	F_PRINT				,	//印刷
	F_PRINT_PREVIEW		,	//印刷プレビュー
	F_PRINT_PAGESETUP	,	//印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	F_OPEN_HfromtoC		,	//同名のC/C++ヘッダー(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
//	F_OPEN_HHPP			,	//同名のC/C++ヘッダーファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更		del 2008/6/23 Uchi
//	F_OPEN_CCPP			,	//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更	del 2008/6/23 Uchi
	F_ACTIVATE_SQLPLUS			,	/* Oracle SQL*Plusをアクティブ表示 */	//Sept. 20, 2000 「コンパイル」JEPRO アクティブ表示を上に移動した
	F_PLSQL_COMPILE_ON_SQLPLUS	,	/* Oracle SQL*Plusで実行 */	//Sept. 20, 2000 jepro 説明の「コンパイル」を「実行」に統一
	F_BROWSE			,	//ブラウズ
	F_VIEWMODE			,	//ビューモード
	F_PROPERTY_FILE		,	/* ファイルのプロパティ */
	F_OPEN_FOLDER_IN_EXPLORER	,	//ファイルの場所を開く
	F_OPEN_COMMAND_PROMPT	,		//コマンドプロンプトを開く
	F_OPEN_COMMAND_PROMPT_AS_ADMIN	,	//管理者としてコマンドプロンプトを開く
	F_OPEN_POWERSHELL	        ,	//PowerShellを開く
	F_OPEN_POWERSHELL_AS_ADMIN	,	//管理者としてPowerShellを開く
	F_PROFILEMGR		,	//プロファイルマネージャ
	F_EXITALLEDITORS	,	//編集の全終了	// 2007.02.13 ryoji F_WIN_CLOSEALL→F_EXITALLEDITORS
	F_EXITALL				//サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加
};
constexpr auto nFincList_File_Num = int(std::size(pnFuncList_File));	//Oct. 16, 2000 JEPRO 配列名変更(FuncList5→FuncList_File)

/* 編集系 */
const EFunctionCode pnFuncList_Edit[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)
	F_UNDO				,	//元に戻す(Undo)
	F_REDO				,	//やり直し(Redo)
	F_DELETE			,	//削除
	F_DELETE_BACK		,	//カーソル前を削除
	F_WordDeleteToStart	,	//単語の左端まで削除
	F_WordDeleteToEnd	,	//単語の右端まで削除
	F_WordCut			,	//単語切り取り
	F_WordDelete		,	//単語削除
	F_LineCutToStart	,	//行頭まで切り取り(改行単位)
	F_LineCutToEnd		,	//行末まで切り取り(改行単位)
	F_LineDeleteToStart	,	//行頭まで削除(改行単位)
	F_LineDeleteToEnd	,	//行末まで削除(改行単位)
	F_CUT_LINE			,	//行切り取り(折り返し単位)
	F_DELETE_LINE		,	//行削除(折り返し単位)
	F_DUPLICATELINE		,	//行の二重化(折り返し単位)
	F_INDENT_TAB		,	//TABインデント
	F_UNINDENT_TAB		,	//逆TABインデント
	F_INDENT_SPACE		,	//SPACEインデント
	F_UNINDENT_SPACE	,	//逆SPACEインデント
	F_LTRIM				,	//左(先頭)の空白を削除	2001.12.03 hor
	F_RTRIM				,	//右(末尾)の空白を削除	2001.12.03 hor
	F_SORT_ASC			,	//選択行の昇順ソート	2001.12.06 hor
	F_SORT_DESC			,	//選択行の降順ソート	2001.12.06 hor
	F_MERGE				,	//選択行のマージ		2001.12.06 hor
	F_RECONVERT				//再変換 				2002.04.09 minfu
//		F_WORDSREFERENCE		//単語リファレンス
};
constexpr auto nFincList_Edit_Num = int(std::size(pnFuncList_Edit));	//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)

/* カーソル移動系 */
const EFunctionCode pnFuncList_Move[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)
	F_UP				,	//カーソル上移動
	F_DOWN				,	//カーソル下移動
	F_LEFT				,	//カーソル左移動
	F_RIGHT				,	//カーソル右移動
	F_UP2				,	//カーソル上移動(２行ごと)
	F_DOWN2				,	//カーソル下移動(２行ごと)
	F_WORDLEFT			,	//単語の左端に移動
	F_WORDRIGHT			,	//単語の右端に移動
	F_GOLINETOP			,	//行頭に移動(折り返し単位)
	F_GOLINEEND			,	//行末に移動(折り返し単位)
//	F_ROLLDOWN			,	//スクロールダウン
//	F_ROLLUP			,	//スクロールアップ
	F_HalfPageUp		,	//半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_HalfPageDown		,	//半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_1PageUp			,	//１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	F_1PageDown			,	//１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	F_GOFILETOP			,	//ファイルの先頭に移動
	F_GOFILEEND			,	//ファイルの最後に移動
	F_CURLINECENTER		,	//カーソル行をウィンドウ中央へ
	F_CURLINETOP		,	//カーソル行をウィンドウ上部へ
	F_CURLINEBOTTOM		,	//カーソル行をウィンドウ下部へ
	F_JUMP_DIALOG		,	//指定行ヘジャンプ	//Sept. 17, 2000 JEPRO コマンド本家は「検索系」
	F_JUMP_SRCHSTARTPOS	,	//検索開始位置へ戻る	// 02/06/26 ai コマンド本家は｢検索系｣
	F_JUMPHIST_PREV		,	//移動履歴: 前へ
	F_JUMPHIST_NEXT		,	//移動履歴: 次へ
	F_JUMPHIST_SET		,	//現在位置を移動履歴に登録
	F_WndScrollDown		,	//テキストを１行下へスクロール	// 2001/06/20 asa-o
	F_WndScrollUp		,	//テキストを１行上へスクロール	// 2001/06/20 asa-o
	F_GONEXTPARAGRAPH	,	//次の段落へ移動
	F_GOPREVPARAGRAPH	,	//前の段落へ移動
	F_AUTOSCROLL		,	//オートスクロール
	F_WHEELUP			,	//ホイールアップ
	F_WHEELDOWN			,	//ホイールダウン
	F_WHEELLEFT			,	//ホイール左
	F_WHEELRIGHT		,	//ホイール右
	F_WHEELPAGEUP		,	//ホイールページアップ
	F_WHEELPAGEDOWN		,	//ホイールページダウン
	F_WHEELPAGELEFT		,	//ホイールページ左
	F_WHEELPAGERIGHT	,	//ホイールページ右
	F_MODIFYLINE_NEXT	,	//次の変更行へ移動
	F_MODIFYLINE_PREV	,	//前の変更行へ移動
};
constexpr auto nFincList_Move_Num = int(std::size(pnFuncList_Move));	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)

/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動
const EFunctionCode pnFuncList_Select[] = {
	F_SELECTWORD			,	//現在位置の単語選択
	F_SELECTALL				,	//すべて選択
	F_SELECTLINE			,	//1行選択	// 2007.10.06 nasukoji
	F_BEGIN_SEL				,	//範囲選択開始
	F_UP_SEL				,	//(範囲選択)カーソル上移動
	F_DOWN_SEL				,	//(範囲選択)カーソル下移動
	F_LEFT_SEL				,	//(範囲選択)カーソル左移動
	F_RIGHT_SEL				,	//(範囲選択)カーソル右移動
	F_UP2_SEL				,	//(範囲選択)カーソル上移動(２行ごと)
	F_DOWN2_SEL				,	//(範囲選択)カーソル下移動(２行ごと)
	F_WORDLEFT_SEL			,	//(範囲選択)単語の左端に移動
	F_WORDRIGHT_SEL			,	//(範囲選択)単語の右端に移動
	F_GOLINETOP_SEL			,	//(範囲選択)行頭に移動(折り返し単位)
	F_GOLINEEND_SEL			,	//(範囲選択)行末に移動(折り返し単位)
//	F_ROLLDOWN_SEL			,	//(範囲選択)スクロールダウン
//	F_ROLLUP_SEL			,	//(範囲選択)スクロールアップ
	F_HalfPageUp_Sel		,	//(範囲選択)半ページアップ	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_HalfPageDown_Sel		,	//(範囲選択)半ページダウン	//Oct. 6, 2000 JEPRO 名称をPC-AT互換機系に変更(ROLL→PAGE) //Oct. 10, 2000 JEPRO 名称変更
	F_1PageUp_Sel			,	//(範囲選択)１ページアップ	//Oct. 10, 2000 JEPRO 従来のページアップを半ページアップと名称変更し１ページアップを追加
	F_1PageDown_Sel			,	//(範囲選択)１ページダウン	//Oct. 10, 2000 JEPRO 従来のページダウンを半ページダウンと名称変更し１ページダウンを追加
	F_GOFILETOP_SEL			,	//(範囲選択)ファイルの先頭に移動
	F_GOFILEEND_SEL			,	//(範囲選択)ファイルの最後に移動
	F_GONEXTPARAGRAPH_SEL	,	//(範囲選択)次の段落へ移動
	F_GOPREVPARAGRAPH_SEL	,	//(範囲選択)前の段落へ移動
	F_MODIFYLINE_NEXT_SEL	,	//(範囲選択)次の変更行へ移動
	F_MODIFYLINE_PREV_SEL	,	//(範囲選択)前の変更行へ移動
};
constexpr auto nFincList_Select_Num = int(std::size(pnFuncList_Select));

/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
const EFunctionCode pnFuncList_Box[] = {
//	F_BOXSELALL			,	//矩形ですべて選択
	F_BEGIN_BOX			,	//矩形範囲選択開始
	F_UP_BOX			,	//(矩形選択)カーソル上移動
	F_DOWN_BOX			,	//(矩形選択)カーソル下移動
	F_LEFT_BOX			,	//(矩形選択)カーソル左移動
	F_RIGHT_BOX			,	//(矩形選択)カーソル右移動
	F_UP2_BOX			,	//(矩形選択)カーソル上移動(２行ごと)
	F_DOWN2_BOX			,	//(矩形選択)カーソル下移動(２行ごと)
	F_WORDLEFT_BOX		,	//(矩形選択)単語の左端に移動
	F_WORDRIGHT_BOX		,	//(矩形選択)単語の右端に移動
	F_GOLOGICALLINETOP_BOX	,	//(矩形選択)行頭に移動(改行単位)
	F_GOLINETOP_BOX		,	//(矩形選択)行頭に移動(折り返し単位)
	F_GOLINEEND_BOX		,	//(矩形選択)行末に移動(折り返し単位)
	F_HalfPageUp_BOX	,	//(矩形選択)半ページアップ
	F_HalfPageDown_BOX	,	//(矩形選択)半ページダウン
	F_1PageUp_BOX		,	//(矩形選択)１ページアップ
	F_1PageDown_BOX		,	//(矩形選択)１ページダウン
	F_GOFILETOP_BOX		,	//(矩形選択)ファイルの先頭に移動
	F_GOFILEEND_BOX			//(矩形選択)ファイルの最後に移動
};
constexpr auto nFincList_Box_Num = int(std::size(pnFuncList_Box));

/* クリップボード系 */
const EFunctionCode pnFuncList_Clip[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List2→List_Clip)
	F_CUT						,	//切り取り(選択範囲をクリップボードにコピーして削除)
	F_COPY						,	//コピー(選択範囲をクリップボードにコピー)
	F_COPY_ADDCRLF				,	//折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	F_COPY_CRLF					,	//CRLF改行でコピー
	F_PASTE						,	//貼り付け(クリップボードから貼り付け)
	F_PASTEBOX					,	//矩形貼り付け(クリップボードから矩形貼り付け)
//	F_INSTEXT_W					,	//テキストを貼り付け		//Oct. 22, 2000 JEPRO ここに追加したが非公式機能なのか不明なのでコメントアウトにしておく
//	F_ADDTAIL_W					,	//最後にテキストを追加		//Oct. 22, 2000 JEPRO ここに追加したが非公式機能なのか不明なのでコメントアウトにしておく
	F_COPYLINES					,	//選択範囲内全行コピー
	F_COPYLINESASPASSAGE		,	//選択範囲内全行引用符付きコピー
	F_COPYLINESWITHLINENUMBER	,	//選択範囲内全行行番号付きコピー
	F_COPY_COLOR_HTML			,	//選択範囲内色付きHTMLコピー
	F_COPY_COLOR_HTML_LINENUMBER,	//選択範囲内行番号色付きHTMLコピー
	F_COPYFNAME					,	//このファイル名をクリップボードにコピー //2002/2/3 aroka
	F_COPYPATH					,	//このファイルのパス名をクリップボードにコピー
	F_COPYDIRPATH				,	//このファイルのフォルダー名をクリップボードにコピー
	F_COPYTAG					,	//このファイルのパス名とカーソル位置をコピー	//Sept. 14, 2000 JEPRO メニューに合わせて下に移動
	F_CREATEKEYBINDLIST				//キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO IDM_TESTのままではうまくいかないのでFに変えて登録	//Dec. 25, 2000 復活
};
constexpr auto nFincList_Clip_Num = int(std::size(pnFuncList_Clip));	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)

/* 挿入系 */
const EFunctionCode pnFuncList_Insert[] = {
	F_INS_DATE				,	// 日付挿入
	F_INS_TIME				,	// 時刻挿入
	F_CTRL_CODE_DIALOG		,	// コントロールコードの入力
	F_INS_FILE_USED_RECENTLY,	// 最近使ったファイル挿入
	F_INS_FOLDER_USED_RECENTLY,	// 最近使ったフォルダー挿入
};
constexpr auto nFincList_Insert_Num = int(std::size(pnFuncList_Insert));

/* 変換系 */
const EFunctionCode pnFuncList_Convert[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)
	F_TOLOWER				,	//小文字
	F_TOUPPER				,	//大文字
	F_TOHANKAKU				,	/* 全角→半角 */
	// From Here 2007.01.24 maru 並び順変更
	F_TOZENKAKUKATA			,	/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	F_TOZENKAKUHIRA			,	/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	F_TOZENEI				,	/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	F_TOHANEI				,	/* 全角英数→半角英数 */
	F_TOHANKATA				,	/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	// To Here 2007.01.24 maru 並び順変更
	F_HANKATATOZENKATA		,	/* 半角カタカナ→全角カタカナ */
	F_HANKATATOZENHIRA		,	/* 半角カタカナ→全角ひらがな */
	F_TABTOSPACE			,	/* TAB→空白 */
	F_SPACETOTAB			,	/* 空白→TAB */  //---- Stonee, 2001/05/27
	F_CODECNV_AUTO2SJIS		,	/* 自動判別→SJISコード変換 */
	F_CODECNV_EMAIL			,	//E-Mail(JIS→SJIS)コード変換
	F_CODECNV_EUC2SJIS		,	//EUC→SJISコード変換
	F_CODECNV_UNICODE2SJIS	,	//Unicode→SJISコード変換
	F_CODECNV_UNICODEBE2SJIS	,	//Unicode→SJISコード変換
	F_CODECNV_UTF82SJIS		,	/* UTF-8→SJISコード変換 */
	F_CODECNV_UTF72SJIS		,	/* UTF-7→SJISコード変換 */
	F_CODECNV_SJIS2JIS		,	/* SJIS→JISコード変換 */
	F_CODECNV_SJIS2EUC		,	/* SJIS→EUCコード変換 */
	F_CODECNV_SJIS2UTF8		,	/* SJIS→UTF-8コード変換 */
	F_CODECNV_SJIS2UTF7		,	/* SJIS→UTF-7コード変換 */
	F_BASE64DECODE			,	//Base64デコードして保存
	F_UUDECODE					//uudecodeして保存
	//Sept. 30, 2000JEPRO コメントアウトされてあったのを復活させた(動作しないのかも？)
	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更
};
constexpr auto nFincList_Convert_Num = int(std::size(pnFuncList_Convert));	//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)

/* 検索系 */
const EFunctionCode pnFuncList_Search[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)
	F_SEARCH_DIALOG		,	//検索(単語検索ダイアログ)
	F_SEARCH_BOX		,	//検索(ボックス)
	F_SEARCH_NEXT		,	//次を検索	//Sept. 16, 2000 JEPRO "次"を"前"の前に移動
	F_SEARCH_PREV		,	//前を検索
	F_REPLACE_DIALOG	,	//置換
	F_SEARCH_CLEARMARK	,	//検索マークのクリア
	F_JUMP_SRCHSTARTPOS	,	//検索開始位置へ戻る	// 02/06/26 ai
	F_GREP_DIALOG		,	//Grep
	F_GREP_REPLACE_DLG	,	//Grep置換
	F_JUMP_DIALOG		,	//指定行ヘジャンプ
	F_OUTLINE			,	//アウトライン解析
	F_OUTLINE_TOGGLE	,	//アウトライン解析(toggle) // 20060201 aroka
	F_FILETREE			,	//ファイルツリー	// 2012.06.20 Moca
	F_TAGJUMP			,	//タグジャンプ機能
	F_TAGJUMP_CLOSE		,	//閉じてタグジャンプ(元ウィンドウを閉じる) // Apr. 03, 2003 genta
	F_TAGJUMPBACK		,	//タグジャンプバック機能
	F_TAGS_MAKE			,	//タグファイルの作成	//@@@ 2003.04.13 MIK
	F_DIRECT_TAGJUMP	,	//ダイレクトタグジャンプ	//@@@ 2003.04.15 MIK
	F_TAGJUMP_KEYWORD	,	//キーワードを指定してダイレクトタグジャンプ	//@@@ 2005.03.31 MIK
	F_COMPARE			,	//ファイル内容比較
	F_DIFF_DIALOG		,	//DIFF差分表示(ダイアログ)
	F_DIFF_NEXT			,	//次の差分へ
	F_DIFF_PREV			,	//前の差分へ
	F_DIFF_RESET		,	//差分の全解除
	F_BRACKETPAIR		,	//対括弧の検索
	F_BOOKMARK_SET		,	//ブックマーク設定・解除
	F_BOOKMARK_NEXT		,	//次のブックマークへ
	F_BOOKMARK_PREV		,	//前のブックマークへ
	F_BOOKMARK_RESET	,	//ブックマークの全解除
	F_BOOKMARK_VIEW		,	//ブックマークの一覧
	F_ISEARCH_NEXT	    ,   //前方インクリメンタルサーチ //2004.10.13 isearch
	F_ISEARCH_PREV		,	//後方インクリメンタルサーチ //2004.10.13 isearch
	F_ISEARCH_REGEXP_NEXT,	//前方正規表現インクリメンタルサーチ  //2004.10.13 isearch
	F_ISEARCH_REGEXP_PREV,	//後方正規表現インクリメンタルサーチ  //2004.10.13 isearch
	F_ISEARCH_MIGEMO_NEXT,	//前方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch
	F_ISEARCH_MIGEMO_PREV,	//後方MIGEMOインクリメンタルサーチ    //2004.10.13 isearch
	F_FUNCLIST_NEXT		,	//次の関数リストマーク
	F_FUNCLIST_PREV		,	//前の関数リストマーク
};
constexpr auto nFincList_Search_Num = int(std::size(pnFuncList_Search));	//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)

/* モード切り替え系 */	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)
const EFunctionCode pnFuncList_Mode[] = {
	F_CHGMOD_INS		,	//挿入／上書きモード切り替え
	F_CHG_CHARSET		,	//文字コードセット指定		2010/6/14 Uchi
	F_CHGMOD_EOL_CRLF	,	//入力改行コード指定(CRLF)	2003.06.23 Moca
	F_CHGMOD_EOL_LF		,	//入力改行コード指定(LF)	2003.06.23 Moca
	F_CHGMOD_EOL_CR		,	//入力改行コード指定(CR)	2003.06.23 Moca
	F_CANCEL_MODE			//各種モードの取り消し
};
constexpr auto nFincList_Mode_Num = int(std::size(pnFuncList_Mode));	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)

/* 設定系 */
const EFunctionCode pnFuncList_Set[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)
	F_SHOWTOOLBAR		,	/* ツールバーの表示 */
	F_SHOWFUNCKEY		,	/* ファンクションキーの表示 */	//Sept. 14, 2000 JEPRO ファンクションキーとステータスバーを入れ替え
	F_SHOWTAB			,	/* タブの表示 */	//@@@ 2003.06.10 MIK
	F_SHOWSTATUSBAR		,	/* ステータスバーの表示 */
	F_SHOWMINIMAP		,	// ミニマップの表示
	F_TYPE_LIST			,	/* タイプ別設定一覧 */			//Sept. 17, 2000 JEPRO 設定系に入ってなかったので追加
	F_OPTION_TYPE		,	/* タイプ別設定 */
	F_OPTION			,	/* 共通設定 */
	F_FONT				,	/* フォント設定 */
	F_SETFONTSIZEUP		,	// フォントサイズ拡大
	F_SETFONTSIZEDOWN	,	// フォントサイズ縮小
	F_WRAPWINDOWWIDTH	,	/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	F_PRINT_PAGESETUP	,	//印刷ページ設定				//Sept. 14, 2000 JEPRO 「印刷のページレイアウトの設定」を「印刷ページ設定」に変更	//Oct. 17, 2000 コマンド本家は「ファイル操作系」
	F_FAVORITE			,	//履歴の管理	//@@@ 2003.04.08 MIK
	F_TMPWRAPNOWRAP		,	//折り返さない（一時設定）			// 2008.05.30 nasukoji
	F_TMPWRAPSETTING	,	//指定桁で折り返す（一時設定）		// 2008.05.30 nasukoji
	F_TMPWRAPWINDOW		,	//右端で折り返す（一時設定）		// 2008.05.30 nasukoji
	F_SELECT_COUNT_MODE		//文字カウント設定	// 2009.07.06 syat
};
constexpr auto nFincList_Set_Num = int(std::size(pnFuncList_Set));	//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)

/* マクロ系 */
const EFunctionCode pnFuncList_Macro[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)
	F_RECKEYMACRO	,	/* キーマクロの記録開始／終了 */
	F_SAVEKEYMACRO	,	/* キーマクロの保存 */
	F_LOADKEYMACRO	,	/* キーマクロの読み込み */
	F_EXECKEYMACRO	,	/* キーマクロの実行 */
	F_EXECEXTMACRO	,	/* 名前を指定してマクロ実行 */
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	F_EXECCMMAND		/* 外部コマンド実行 */
	F_EXECMD_DIALOG		/* 外部コマンド実行 */
//	To Here Sept. 20, 2000
};
constexpr auto nFincList_Macro_Num = int(std::size(pnFuncList_Macro));	//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)

/* カスタムメニュー */	//Oct. 21, 2000 JEPRO 「その他」から分離独立化
#if 0
//	From Here Oct. 15, 2001 genta
//	カスタムメニューの文字列を動的に変更可能にするためこれは削除．
const EFunctionCode pnFuncList_Menu[] = {
	F_MENU_RBUTTON				,	/* 右クリックメニュー */
	F_CUSTMENU_1				,	/* カスタムメニュー1 */
	F_CUSTMENU_2				,	/* カスタムメニュー2 */
	F_CUSTMENU_3				,	/* カスタムメニュー3 */
	F_CUSTMENU_4				,	/* カスタムメニュー4 */
	F_CUSTMENU_5				,	/* カスタムメニュー5 */
	F_CUSTMENU_6				,	/* カスタムメニュー6 */
	F_CUSTMENU_7				,	/* カスタムメニュー7 */
	F_CUSTMENU_8				,	/* カスタムメニュー8 */
	F_CUSTMENU_9				,	/* カスタムメニュー9 */
	F_CUSTMENU_10				,	/* カスタムメニュー10 */
	F_CUSTMENU_11				,	/* カスタムメニュー11 */
	F_CUSTMENU_12				,	/* カスタムメニュー12 */
	F_CUSTMENU_13				,	/* カスタムメニュー13 */
	F_CUSTMENU_14				,	/* カスタムメニュー14 */
	F_CUSTMENU_15				,	/* カスタムメニュー15 */
	F_CUSTMENU_16				,	/* カスタムメニュー16 */
	F_CUSTMENU_17				,	/* カスタムメニュー17 */
	F_CUSTMENU_18				,	/* カスタムメニュー18 */
	F_CUSTMENU_19				,	/* カスタムメニュー19 */
	F_CUSTMENU_20				,	/* カスタムメニュー20 */
	F_CUSTMENU_21				,	/* カスタムメニュー21 */
	F_CUSTMENU_22				,	/* カスタムメニュー22 */
	F_CUSTMENU_23				,	/* カスタムメニュー23 */
	F_CUSTMENU_24				 	/* カスタムメニュー24 */
};
constexpr auto nFincList_Menu_Num = int(std::size(pnFuncList_Menu));	//Oct. 21, 2000 JEPRO 「その他」から分離独立化
#endif

/* ウィンドウ系 */
const EFunctionCode pnFuncList_Win[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)
	F_SPLIT_V			,	//上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	F_SPLIT_H			,	//左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	F_SPLIT_VH			,	//縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	F_WINCLOSE			,	//ウィンドウを閉じる
	F_WIN_CLOSEALL		,	//すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	F_TAB_CLOSEOTHER	,	//このタブ以外を閉じる	// 2008.11.22 syat
	F_NEXTWINDOW		,	//次のウィンドウ
	F_PREVWINDOW		,	//前のウィンドウ
 	F_WINLIST			,	//開いているウィンドウ一覧ポップアップ表示	// 2006.03.23 fon
	F_DLGWINLIST		,	//ウィンドウ一覧表示
	F_CASCADE			,	//重ねて表示
	F_TILE_V			,	//上下に並べて表示
	F_TILE_H			,	//左右に並べて表示
	F_TOPMOST			,	//常に手前に表示
	F_BIND_WINDOW		,	//結合して表示	// 2006.04.22 ryoji
	F_GROUPCLOSE		,	//グループを閉じる	// 2007.06.20 ryoji
	F_NEXTGROUP			,	//次のグループ	// 2007.06.20 ryoji
	F_PREVGROUP			,	//前のグループ	// 2007.06.20 ryoji
	F_TAB_MOVERIGHT		,	//タブを右に移動	// 2007.06.20 ryoji
	F_TAB_MOVELEFT		,	//タブを左に移動	// 2007.06.20 ryoji
	F_TAB_SEPARATE		,	//新規グループ	// 2007.06.20 ryoji
	F_TAB_JOINTNEXT		,	//次のグループに移動	// 2007.06.20 ryoji
	F_TAB_JOINTPREV		,	//前のグループに移動	// 2007.06.20 ryoji
	F_TAB_CLOSELEFT 	,	//左をすべて閉じる		// 2008.11.22 syat
	F_TAB_CLOSERIGHT	,	//右をすべて閉じる		// 2008.11.22 syat
	F_MAXIMIZE_V		,	//縦方向に最大化
	F_MAXIMIZE_H		,	//横方向に最大化 //2001.02.10 by MIK
	F_MINIMIZE_ALL		,	//すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	F_REDRAW			,	//再描画
	F_WIN_OUTPUT		,	//アウトプットウィンドウ表示
};
constexpr auto nFincList_Win_Num = int(std::size(pnFuncList_Win));	//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)

/* 支援 */
const EFunctionCode pnFuncList_Support[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)
	F_HOKAN						,	/* 入力補完 */
	F_TOGGLE_KEY_SEARCH			,	/* キャレット位置の単語を辞書検索する機能ON/OFF */	// 2006.03.24 fon
//Sept. 15, 2000→Nov. 25, 2000 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
	F_HELP_CONTENTS				,	/* ヘルプ目次 */			//Nov. 25, 2000 JEPRO 追加
	F_HELP_SEARCH				,	/* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
	F_MENU_ALLFUNC				,	/* コマンド一覧 */
	F_EXTHELP1					,	/* 外部ヘルプ１ */
	F_EXTHTMLHELP				,	/* 外部HTMLヘルプ */
	F_ABOUT							/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加
};
constexpr auto nFincList_Support_Num = int(std::size(pnFuncList_Support));	//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)

/* その他 */	//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)
const EFunctionCode pnFuncList_Others[] = {
	F_DISABLE				//Oct. 21, 2000 JEPRO 何もないとエラーになってしまうのでダミーで[未定義]を入れておく
};
constexpr auto nFincList_Others_Num = int(std::size(pnFuncList_Others));	//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)

// 特殊機能
const EFunctionCode nsFuncCode::pnFuncList_Special[] = {
	F_WINDOW_LIST,
	F_FILE_USED_RECENTLY,
	F_FOLDER_USED_RECENTLY,
	F_CUSTMENU_LIST,
	F_USERMACRO_LIST,
	F_PLUGIN_LIST,
};
const int nsFuncCode::nFuncList_Special_Num = int(std::size(nsFuncCode::pnFuncList_Special));

const int nsFuncCode::pnFuncListNumArr[] = {
//	nFincList_Undef_Num,	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更	//Oct. 16, 2000 JEPRO 変数名変更(List0→List_Undef)
	nFincList_File_Num,		/* ファイル操作系 */	//Oct. 16, 2000 JEPRO 変数名変更(List5→List_File)
	nFincList_Edit_Num,		/* 編集系 */			//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)
	nFincList_Move_Num,		/* カーソル移動系 */	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)
	nFincList_Select_Num,	/* 選択系 */			//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動
	nFincList_Box_Num,		/* 矩形選択系 */		//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
	nFincList_Clip_Num,		/* クリップボード系 */	//Oct. 16, 2000 JEPRO 変数名変更(List2→List_Clip)
	nFincList_Insert_Num,	/* 挿入系 */
	nFincList_Convert_Num,	/* 変換系 */			//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)
	nFincList_Search_Num,	/* 検索系 */			//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)
	nFincList_Mode_Num,		/* モード切り替え系 */	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)
	nFincList_Set_Num,		/* 設定系 */			//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)
	nFincList_Macro_Num,	/* マクロ系 */			//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)
//	カスタムメニューの文字列を動的に変更可能にするためこれは削除
//	nFincList_Menu_Num,		/* カスタムメニュー */	//Oct. 21, 2000 JEPRO 「その他」から分離独立化
	nFincList_Win_Num,		/* ウィンドウ系 */		//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)
	nFincList_Support_Num,	/* 支援 */				//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)
	nFincList_Others_Num	/* その他 */			//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)
};
const EFunctionCode* nsFuncCode::ppnFuncListArr[] = {
//	pnFuncList_Undef,	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更	//Oct. 16, 2000 JEPRO 変数名変更(List0→List_Undef)
	pnFuncList_File,	/* ファイル操作系 */	//Oct. 16, 2000 JEPRO 変数名変更(List5→List_File)
	pnFuncList_Edit,	/* 編集系 */			//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)
	pnFuncList_Move,	/* カーソル移動系 */	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)
	pnFuncList_Select,/* 選択系 */			//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動  (矩形選択)は新設され次第ここにおく
	pnFuncList_Box,	/* 矩形選択系 */		//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
	pnFuncList_Clip,	/* クリップボード系 */	//Oct. 16, 2000 JEPRO 変数名変更(List2→List_Clip)
	pnFuncList_Insert,/* 挿入系 */
	pnFuncList_Convert,/* 変換系 */			//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)
	pnFuncList_Search,/* 検索系 */			//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)
	pnFuncList_Mode,	/* モード切り替え系 */	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)
	pnFuncList_Set,	/* 設定系 */			//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)
	pnFuncList_Macro,	/* マクロ系 */			//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)
//	カスタムメニューの文字列を動的に変更可能にするためこれは削除
//	pnFuncList_Menu,	/* カスタムメニュー */	//Oct. 21, 2000 JEPRO「その他」から分離独立化
	pnFuncList_Win,	/* ウィンドウ系 */		//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)
	pnFuncList_Support,/* 支援 */				//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)
	pnFuncList_Others	/* その他 */			//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)
};
const int nsFuncCode::nFincListNumArrNum = int(std::size(nsFuncCode::pnFuncListNumArr));

// 機能コードに対するヘルプトピックIDのマップ
const std::map<EFunctionCode, int> g_FuncHelpMap = {
	/* ファイル操作系 */
	{ F_FILENEW,						HLP000025 },	//新規作成
	{ F_FILENEW_NEWWINDOW,				HLP000339 },	//新規ウインドウで開く
	{ F_FILEOPEN,						HLP000015 },	//開く
	{ F_FILEOPEN_DROPDOWN,				HLP000015 },	//開く(ドロップダウン)
	{ F_FILESAVE,						HLP000020 },	//上書き保存
	{ F_FILESAVEAS_DIALOG,				HLP000021 },	//名前を付けて保存
	{ F_FILESAVEALL,					HLP000313 },	//すべて上書き保存
	{ F_FILESAVECLOSE,					HLP000287 },	//保存して閉じる
	{ F_FILECLOSE,						HLP000017 },	//閉じて(無題)
	{ F_FILECLOSE_OPEN,					HLP000119 },	//閉じて開く
	{ F_FILE_REOPEN,					HLP000283 },	//開き直す
	{ F_FILE_REOPEN_SJIS,				HLP000156 },	//SJISで開き直す
	{ F_FILE_REOPEN_JIS,				HLP000157 },	//JISで開き直す
	{ F_FILE_REOPEN_EUC,				HLP000158 },	//EUCで開き直す
	{ F_FILE_REOPEN_LATIN1,				HLP000341 },	//Latin1で開き直す
	{ F_FILE_REOPEN_UNICODE,			HLP000159 },	//Unicodeで開き直す
	{ F_FILE_REOPEN_UNICODEBE,			HLP000256 },	//UnicodeBEで開き直す
	{ F_FILE_REOPEN_UTF8,				HLP000160 },	//UTF-8で開き直す
	{ F_FILE_REOPEN_CESU8,				HLP000337 },	//CESU-8で開き直す
	{ F_FILE_REOPEN_UTF7,				HLP000161 },	//UTF-7で開き直す
	{ F_PRINT,							HLP000162 },	//印刷
	{ F_PRINT_PREVIEW,					HLP000120 },	//印刷プレビュー
	{ F_PRINT_PAGESETUP,				HLP000122 },	//印刷ページ設定
	{ F_OPEN_HfromtoC,					HLP000192 },	//同名のC/C++ヘッダー(ソース)を開く
	{ F_ACTIVATE_SQLPLUS,				HLP000132 },	//Oracle SQL*Plusをアクティブ表示
	{ F_PLSQL_COMPILE_ON_SQLPLUS,		HLP000027 },	//Oracle SQL*Plusで実行
	{ F_BROWSE,							HLP000121 },	//ブラウズ
	{ F_VIEWMODE,						HLP000249 },	//ビューモード
	{ F_PROPERTY_FILE,					HLP000022 },	//ファイルのプロパティ
	{ F_OPEN_FOLDER_IN_EXPLORER,		HLP000373 },	//ファイルの場所を開く
	{ F_OPEN_COMMAND_PROMPT,			HLP000376 },	//コマンドプロンプトを開く
	{ F_OPEN_COMMAND_PROMPT_AS_ADMIN,	HLP000377 },	//管理者としてコマンドプロンプトを開く
	{ F_OPEN_POWERSHELL,				HLP000378 },	//PowerShellを開く
	{ F_OPEN_POWERSHELL_AS_ADMIN,		HLP000379 },	//管理者としてPowerShellを開く
	{ F_PROFILEMGR,						HLP000363 },	//プロファイルマネージャ

	{ F_EXITALLEDITORS,					HLP000030 },	//編集の全終了
	{ F_EXITALL,						HLP000028 },	//サクラエディタの全終了

	/* 編集系 */
	{ F_UNDO,							HLP000032 },	//元に戻す(Undo)
	{ F_REDO,							HLP000033 },	//やり直し(Redo)
	{ F_DELETE,							HLP000041 },	//削除
	{ F_DELETE_BACK,					HLP000042 },	//カーソル前を削除
	{ F_WordDeleteToStart,				HLP000166 },	//単語の左端まで削除
	{ F_WordDeleteToEnd,				HLP000167 },	//単語の右端まで削除
	{ F_WordCut,						HLP000169 },	//単語切り取り
	{ F_WordDelete,						HLP000168 },	//単語削除
	{ F_LineCutToStart,					HLP000172 },	//行頭まで切り取り(改行単位)
	{ F_LineCutToEnd,					HLP000173 },	//行末まで切り取り(改行単位)
	{ F_LineDeleteToStart,				HLP000170 },	//行頭まで削除(改行単位)
	{ F_LineDeleteToEnd,				HLP000171 },	//行末まで削除(改行単位)
	{ F_CUT_LINE,						HLP000174 },	//行切り取り(折り返し単位)
	{ F_DELETE_LINE,					HLP000137 },	//行削除(折り返し単位)
	{ F_DUPLICATELINE,					HLP000043 },	//行の二重化(折り返し単位)
	{ F_INDENT_TAB,						HLP000113 },	//TABインデント
	{ F_UNINDENT_TAB,					HLP000113 },	//逆TABインデント
	{ F_INDENT_SPACE,					HLP000114 },	//SPACEインデント
	{ F_UNINDENT_SPACE,					HLP000114 },	//逆SPACEインデント
	{ F_RECONVERT,						HLP000218 },	//再変換

	/* カーソル移動系 */
	{ F_UP,								HLP000289 },	//カーソル上移動
	{ F_DOWN,							HLP000289 },	//カーソル下移動
	{ F_LEFT,							HLP000289 },	//カーソル左移動
	{ F_RIGHT,							HLP000289 },	//カーソル右移動
	{ F_UP2,							HLP000220 },	//カーソル上移動(２行ごと)
	{ F_DOWN2,							HLP000221 },	//カーソル下移動(２行ごと)
	{ F_WORDLEFT,						HLP000222 },	//単語の左端に移動
	{ F_WORDRIGHT,						HLP000223 },	//単語の右端に移動
	{ F_GOLINETOP,						HLP000224 },	//行頭に移動(折り返し単位)
	{ F_GOLINEEND,						HLP000225 },	//行末に移動(折り返し単位)
	{ F_HalfPageUp,						HLP000245 },	//半ページアップ
	{ F_HalfPageDown,					HLP000246 },	//半ページダウン
	{ F_1PageUp,						HLP000226 },	//１ページアップ
	{ F_1PageDown,						HLP000227 },	//１ページダウン
	{ F_GOFILETOP,						HLP000228 },	//ファイルの先頭に移動
	{ F_GOFILEEND,						HLP000229 },	//ファイルの最後に移動
	{ F_CURLINECENTER,					HLP000230 },	//カーソル行をウィンドウ中央へ
	{ F_JUMP_SRCHSTARTPOS,				HLP000264 },	//検索開始位置へ戻る
	{ F_JUMPHIST_PREV,					HLP000231 },	//移動履歴: 前へ
	{ F_JUMPHIST_NEXT,					HLP000232 },	//移動履歴: 次へ
	{ F_JUMPHIST_SET,					HLP000265 },	//現在位置を移動履歴に登録
	{ F_WndScrollDown,					HLP000198 },	//テキストを１行下へスクロール
	{ F_WndScrollUp,					HLP000199 },	//テキストを１行上へスクロール
	{ F_GONEXTPARAGRAPH,				HLP000262 },	//前の段落へ移動
	{ F_GOPREVPARAGRAPH,				HLP000263 },	//前の段落へ移動
	{ F_AUTOSCROLL,						HLP000296 },	//オートスクロール
	{ F_SETFONTSIZEUP,					HLP000359 },	//フォントサイズ拡大
	{ F_SETFONTSIZEDOWN,				HLP000360 },	//フォントサイズ縮小
	{ F_MODIFYLINE_NEXT,				HLP000366 },	//次の変更行へ移動
	{ F_MODIFYLINE_PREV,				HLP000367 },	//前の変更行へ移動

	/* 選択系 */
	{ F_SELECTWORD,						HLP000045 },	//現在位置の単語選択
	{ F_SELECTALL,						HLP000044 },	//すべて選択
	{ F_SELECTLINE,						HLP000108 },	//1行選択
	{ F_BEGIN_SEL,						HLP000233 },	//範囲選択開始
	{ F_UP_SEL,							HLP000290 },	//(範囲選択)カーソル上移動
	{ F_DOWN_SEL,						HLP000290 },	//(範囲選択)カーソル下移動
	{ F_LEFT_SEL,						HLP000290 },	//(範囲選択)カーソル左移動
	{ F_RIGHT_SEL,						HLP000290 },	//(範囲選択)カーソル右移動
	{ F_UP2_SEL,						HLP000234 },	//(範囲選択)カーソル上移動(２行ごと)
	{ F_DOWN2_SEL,						HLP000235 },	//(範囲選択)カーソル下移動(２行ごと)
	{ F_WORDLEFT_SEL,					HLP000236 },	//(範囲選択)単語の左端に移動
	{ F_WORDRIGHT_SEL,					HLP000237 },	//(範囲選択)単語の右端に移動
	{ F_GONEXTPARAGRAPH_SEL,			HLP000273 },	//(範囲選択)前の段落へ移動
	{ F_GOPREVPARAGRAPH_SEL,			HLP000274 },	//(範囲選択)前の段落へ移動
	{ F_GOLINETOP_SEL,					HLP000238 },	//(範囲選択)行頭に移動(折り返し単位)
	{ F_GOLINEEND_SEL,					HLP000239 },	//(範囲選択)行末に移動(折り返し単位)
	{ F_HalfPageUp_Sel,					HLP000247 },	//(範囲選択)半ページアップ
	{ F_HalfPageDown_Sel,				HLP000248 },	//(範囲選択)半ページダウン
	{ F_1PageUp_Sel,					HLP000240 },	//(範囲選択)１ページアップ
	{ F_1PageDown_Sel,					HLP000241 },	//(範囲選択)１ページダウン
	{ F_GOFILETOP_SEL,					HLP000242 },	//(範囲選択)ファイルの先頭に移動
	{ F_GOFILEEND_SEL,					HLP000243 },	//(範囲選択)ファイルの最後に移動
	{ F_MODIFYLINE_NEXT_SEL,			HLP000369 },	//(範囲選択)次の変更行へ移動
	{ F_MODIFYLINE_PREV_SEL,			HLP000370 },	//(範囲選択)前の変更行へ移動

	/* 矩形選択系 */
	{ F_BEGIN_BOX,						HLP000244 },	//矩形範囲選択開始
	{ F_UP_BOX,							HLP000299 },	//(矩形選択)カーソル上移動
	{ F_DOWN_BOX,						HLP000299 },	//(矩形選択)カーソル下移動
	{ F_LEFT_BOX,						HLP000299 },	//(矩形選択)カーソル左移動
	{ F_RIGHT_BOX,						HLP000299 },	//(矩形選択)カーソル右移動
	{ F_UP2_BOX,						HLP000344 },	//(矩形選択)カーソル上移動(２行ごと)
	{ F_DOWN2_BOX,						HLP000345 },	//(矩形選択)カーソル下移動(２行ごと)
	{ F_WORDLEFT_BOX,					HLP000346 },	//(矩形選択)単語の左端に移動
	{ F_WORDRIGHT_BOX,					HLP000347 },	//(矩形選択)単語の右端に移動
	{ F_GOLINETOP_BOX,					HLP000350 },	//(矩形選択)行頭に移動(折り返し単位)
	{ F_GOLOGICALLINETOP_BOX,			HLP000361 },	//(矩形選択)行頭に移動(改行単位)
	{ F_GOLINEEND_BOX,					HLP000351 },	//(矩形選択)行末に移動(折り返し単位)
	{ F_HalfPageUp_BOX,					HLP000356 },	//(矩形選択)半ページアップ
	{ F_HalfPageDown_BOX,				HLP000357 },	//(矩形選択)半ページダウン
	{ F_1PageUp_BOX,					HLP000352 },	//(矩形選択)１ページアップ
	{ F_1PageDown_BOX,					HLP000353 },	//(矩形選択)１ページダウン
	{ F_GOFILETOP_BOX,					HLP000354 },	//(矩形選択)ファイルの先頭に移動
	{ F_GOFILEEND_BOX,					HLP000355 },	//(矩形選択)ファイルの最後に移動

	/* 整形系 */
	{ F_LTRIM,							HLP000210 },	//左(先頭)の空白を削除
	{ F_RTRIM,							HLP000211 },	//右(末尾)の空白を削除
	{ F_SORT_ASC,						HLP000212 },	//選択行の昇順ソート
	{ F_SORT_DESC,						HLP000213 },	//選択行の降順ソート
	{ F_MERGE,							HLP000214 },	//選択行のマージ

	/* クリップボード系 */
	{ F_CUT,							HLP000034 },	//切り取り(選択範囲をクリップボードにコピーして削除)
	{ F_COPY,							HLP000035 },	//コピー(選択範囲をクリップボードにコピー)
	{ F_COPY_ADDCRLF,					HLP000219 },	//折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	{ F_COPY_CRLF,						HLP000163 },	//CRLF改行でコピー(選択範囲をクリップボードにコピー)
	{ F_PASTE,							HLP000039 },	//貼り付け(クリップボードから貼り付け)
	{ F_PASTEBOX,						HLP000040 },	//矩形貼り付け(クリップボードから矩形貼り付け)
	{ F_COPYLINES,						HLP000036 },	//選択範囲内全行コピー
	{ F_COPYLINESASPASSAGE,				HLP000037 },	//選択範囲内全行引用符付きコピー
	{ F_COPYLINESWITHLINENUMBER,		HLP000038 },	//選択範囲内全行行番号付きコピー
	{ F_COPY_COLOR_HTML,				HLP000342 },	//選択範囲内色付きHTMLコピー
	{ F_COPY_COLOR_HTML_LINENUMBER,		HLP000343 },	//選択範囲内行番号色付きHTMLコピー
	{ F_COPYPATH,						HLP000056 },	//このファイルのパス名をクリップボードにコピー
	{ F_COPYDIRPATH,					HLP000380 },	//このファイルのフォルダー名をクリップボードにコピー
	{ F_COPYTAG,						HLP000175 },	//このファイルのパス名とカーソル位置をコピー
	{ F_COPYFNAME,						HLP000303 },	//このファイル名をクリップボードにコピー
	{ F_CREATEKEYBINDLIST,				HLP000057 },	//キー割り当て一覧をクリップボードへコピー

	/* 挿入系 */
	{ F_INS_DATE,						HLP000164 },	//日付挿入
	{ F_INS_TIME,						HLP000165 },	//時刻挿入
	{ F_CTRL_CODE_DIALOG,				HLP000255 },	//コントロールコード入力

	/* 変換系 */
	{ F_TOLOWER,						HLP000047 },	//小文字
	{ F_TOUPPER,						HLP000048 },	//大文字
	{ F_TOHANKAKU,						HLP000049 },	//全角→半角
	{ F_TOHANKATA,						HLP000258 },	//全角カタカナ→半角カタカナ
	{ F_TOZENKAKUKATA,					HLP000050 },	//半角＋全ひら→全角・カタカナ
	{ F_TOZENKAKUHIRA,					HLP000051 },	//半角＋全カタ→全角・ひらがな
	{ F_HANKATATOZENKATA,				HLP000123 },	//半角カタカナ→全角カタカナ
	{ F_HANKATATOZENHIRA,				HLP000124 },	//半角カタカナ→全角ひらがな
	{ F_TOZENEI,						HLP000200 },	//半角英数→全角英数
	{ F_TOHANEI,						HLP000215 },	//全角英数→半角英数
	{ F_TABTOSPACE,						HLP000182 },	//TAB→空白
	{ F_SPACETOTAB,						HLP000196 },	//空白→TAB
	{ F_CODECNV_AUTO2SJIS,				HLP000178 },	//自動判別→SJISコード変換
	{ F_CODECNV_EMAIL,					HLP000052 },	//E-Mail(JIS→SJIS)コード変換
	{ F_CODECNV_EUC2SJIS,				HLP000053 },	//EUC→SJISコード変換
	{ F_CODECNV_UNICODE2SJIS,			HLP000179 },	//Unicode→SJISコード変換
	{ F_CODECNV_UNICODEBE2SJIS,			HLP000257 },	//UnicodeBE→SJISコード変換
	{ F_CODECNV_UTF82SJIS,				HLP000142 },	//UTF-8→SJISコード変換
	{ F_CODECNV_UTF72SJIS,				HLP000143 },	//UTF-7→SJISコード変換
	{ F_CODECNV_SJIS2JIS,				HLP000117 },	//SJIS→JISコード変換
	{ F_CODECNV_SJIS2EUC,				HLP000118 },	//SJIS→EUCコード変換
	{ F_CODECNV_SJIS2UTF8,				HLP000180 },	//SJIS→UTF-8コード変換
	{ F_CODECNV_SJIS2UTF7,				HLP000181 },	//SJIS→UTF-7コード変換
	{ F_BASE64DECODE,					HLP000054 },	//Base64デコードして保存
	{ F_UUDECODE,						HLP000055 },	//uudecodeして保存

	/* 検索系 */
	{ F_SEARCH_DIALOG,					HLP000059 },	//検索(単語検索ダイアログ)
	{ F_SEARCH_BOX,						HLP000059 },	//検索(ボックス)
	{ F_SEARCH_NEXT,					HLP000061 },	//次を検索
	{ F_SEARCH_PREV,					HLP000060 },	//前を検索
	{ F_REPLACE_DIALOG,					HLP000062 },	//置換(置換ダイアログ)
	{ F_SEARCH_CLEARMARK,				HLP000136 },	//検索マークのクリア
	{ F_GREP_DIALOG,					HLP000067 },	//Grep
	{ F_GREP_REPLACE_DLG,				HLP000362 },	//Grep置換
	{ F_JUMP_DIALOG,					HLP000063 },	//指定行へジャンプ
	{ F_OUTLINE,						HLP000064 },	//アウトライン解析
	{ F_OUTLINE_TOGGLE,					HLP000317 },	//アウトライン解析(トグル)
	{ F_TAGJUMP,						HLP000065 },	//タグジャンプ機能
	{ F_TAGJUMPBACK,					HLP000066 },	//タグジャンプバック機能
	{ F_TAGS_MAKE,						HLP000280 },	//タグファイルの作成
	{ F_TAGJUMP_LIST,					HLP000281 },	//タグジャンプ一覧
	{ F_DIRECT_TAGJUMP,					HLP000281 },	//ダイレクトタグジャンプ
	{ F_TAGJUMP_CLOSE,					HLP000291 },	//閉じてタグジャンプ(元ウィンドウClose)
	{ F_TAGJUMP_KEYWORD,				HLP000310 },	//キーワードを指定してタグジャンプ
	{ F_COMPARE,						HLP000116 },	//ファイル内容比較
	{ F_DIFF_DIALOG,					HLP000251 },	//DIFF差分表示(ダイアログ)
	{ F_DIFF_NEXT,						HLP000252 },	//次の差分へ
	{ F_DIFF_PREV,						HLP000253 },	//前の差分へ
	{ F_DIFF_RESET,						HLP000254 },	//差分の全解除
	{ F_BRACKETPAIR,					HLP000183 },	//対括弧の検索
	{ F_BOOKMARK_SET,					HLP000205 },	//ブックマーク設定・解除
	{ F_BOOKMARK_NEXT,					HLP000206 },	//次のブックマークへ
	{ F_BOOKMARK_PREV,					HLP000207 },	//前のブックマークへ
	{ F_BOOKMARK_RESET,					HLP000208 },	//ブックマークの全解除
	{ F_BOOKMARK_VIEW,					HLP000209 },	//ブックマークの一覧
	{ F_ISEARCH_NEXT,					HLP000304 },	//前方インクリメンタルサーチ
	{ F_ISEARCH_PREV,					HLP000305 },	//後方インクリメンタルサーチ
	{ F_ISEARCH_REGEXP_NEXT,			HLP000306 },	//正規表現前方インクリメンタルサーチ
	{ F_ISEARCH_REGEXP_PREV,			HLP000307 },	//正規表現後方インクリメンタルサーチ
	{ F_ISEARCH_MIGEMO_NEXT,			HLP000308 },	//MIGEMO前方インクリメンタルサーチ
	{ F_ISEARCH_MIGEMO_PREV,			HLP000309 },	//MIGEMO後方インクリメンタルサーチ
	{ F_FUNCLIST_NEXT,					HLP000364 },	//次の関数リストマーク
	{ F_FUNCLIST_PREV,					HLP000365 },	//前の関数リストマーク
	{ F_FILETREE,						HLP000368 },	//ファイルツリー

	/* モード切り替え系 */
	{ F_CHGMOD_INS,						HLP000046 },	//挿入／上書きモード切り替え
	{ F_CHG_CHARSET,					HLP000297 },	//文字コードセット指定
	{ F_CHGMOD_EOL_CRLF,				HLP000285 },	//入力改行コード指定
	{ F_CHGMOD_EOL_CR,					HLP000285 },	//入力改行コード指定
	{ F_CHGMOD_EOL_LF,					HLP000285 },	//入力改行コード指定
	{ F_CANCEL_MODE,					HLP000194 },	//各種モードの取り消し

	/* 設定系 */
	{ F_SHOWTOOLBAR,					HLP000069 },	//ツールバーの表示
	{ F_SHOWFUNCKEY,					HLP000070 },	//ファンクションキーの表示
	{ F_SHOWTAB,						HLP000282 },	//タブの表示
	{ F_SHOWSTATUSBAR,					HLP000134 },	//ステータスバーの表示
	{ F_SHOWMINIMAP,					HLP000371 },	// ミニマップの表示
	{ F_TYPE_LIST,						HLP000072 },	//タイプ別設定一覧
	{ F_OPTION_TYPE,					HLP000073 },	//タイプ別設定
	{ F_OPTION,							HLP000076 },	//共通設定
	{ F_TYPE_SCREEN,					HLP000074 },	//タイプ別設定『スクリーン』
	{ F_TYPE_COLOR,						HLP000075 },	//タイプ別設定『カラー』
	{ F_TYPE_WINDOW,					HLP000319 },	//タイプ別設定『ウィンドウ』
	{ F_TYPE_HELPER,					HLP000197 },	//タイプ別設定『支援』
	{ F_TYPE_REGEX_KEYWORD,				HLP000203 },	//タイプ別設定『正規表現キーワード』
	{ F_TYPE_KEYHELP,					HLP000315 },	//タイプ別設定『キーワードヘルプ』
	{ F_OPTION_GENERAL,					HLP000081 },	//共通設定『全般』
	{ F_OPTION_WINDOW,					HLP000146 },	//共通設定『ウィンドウ』
	{ F_OPTION_TAB,						HLP000150 },	//共通設定『タブバー』
	{ F_OPTION_EDIT,					HLP000144 },	//共通設定『編集』
	{ F_OPTION_FILE,					HLP000083 },	//共通設定『ファイル』
	{ F_OPTION_BACKUP,					HLP000145 },	//共通設定『バックアップ』
	{ F_OPTION_FORMAT,					HLP000082 },	//共通設定『書式』
	{ F_OPTION_GREP,					HLP000148 },	//共通設定『Grep』
	{ F_OPTION_KEYBIND,					HLP000084 },	//共通設定『キー割り当て』
	{ F_OPTION_CUSTMENU,				HLP000087 },	//共通設定『カスタムメニュー』
	{ F_OPTION_TOOLBAR,					HLP000085 },	//共通設定『ツールバー』
	{ F_OPTION_KEYWORD,					HLP000086 },	//共通設定『強調キーワード』
	{ F_OPTION_HELPER,					HLP000088 },	//共通設定『支援』
	{ F_OPTION_MACRO,					HLP000201 },	//共通設定『マクロ』
	{ F_OPTION_STATUSBAR,				HLP000147 },	//共通設定『ステータスバー』
	{ F_OPTION_PLUGIN,					HLP000151 },	//共通設定『プラグイン』
	{ F_OPTION_FNAME,					HLP000277 },	//共通設定『ファイル名表示』プロパティ
	{ F_OPTION_MAINMENU,				HLP000152 },	//共通設定『メインメニュー』
	{ F_FONT,							HLP000071 },	//フォント設定
	{ F_WRAPWINDOWWIDTH,				HLP000184 },	//現在のウィンドウ幅で折り返し
	{ F_FAVORITE,						HLP000279 },	//履歴の管理
	{ F_TMPWRAPNOWRAP,					HLP000340 },	//折り返さない
	{ F_TMPWRAPSETTING,					HLP000340 },	//指定桁で折り返す
	{ F_TMPWRAPWINDOW,					HLP000340 },	//右端で折り返す
	{ F_SELECT_COUNT_MODE,				HLP000336 },	//文字カウント方法

	/* マクロ */
	{ F_RECKEYMACRO,					HLP000125 },	//キーマクロ記録開始／終了
	{ F_SAVEKEYMACRO,					HLP000127 },	//キーマクロ保存
	{ F_LOADKEYMACRO,					HLP000128 },	//キーマクロ読み込み
	{ F_EXECKEYMACRO,					HLP000126 },	//キーマクロ実行
	{ F_EXECEXTMACRO,					HLP000332 },	//名前を指定してマクロ実行
	{ F_EXECMD_DIALOG,					HLP000103 },	//外部コマンド実行

	/* カスタムメニュー */
	{ F_MENU_RBUTTON,					HLP000195 },	//右クリックメニュー
	{ F_CUSTMENU_1,						HLP000186 },	//カスタムメニュー1
	{ F_CUSTMENU_2,						HLP000186 },	//カスタムメニュー2
	{ F_CUSTMENU_3,						HLP000186 },	//カスタムメニュー3
	{ F_CUSTMENU_4,						HLP000186 },	//カスタムメニュー4
	{ F_CUSTMENU_5,						HLP000186 },	//カスタムメニュー5
	{ F_CUSTMENU_6,						HLP000186 },	//カスタムメニュー6
	{ F_CUSTMENU_7,						HLP000186 },	//カスタムメニュー7
	{ F_CUSTMENU_8,						HLP000186 },	//カスタムメニュー8
	{ F_CUSTMENU_9,						HLP000186 },	//カスタムメニュー9
	{ F_CUSTMENU_10,					HLP000186 },	//カスタムメニュー10
	{ F_CUSTMENU_11,					HLP000186 },	//カスタムメニュー11
	{ F_CUSTMENU_12,					HLP000186 },	//カスタムメニュー12
	{ F_CUSTMENU_13,					HLP000186 },	//カスタムメニュー13
	{ F_CUSTMENU_14,					HLP000186 },	//カスタムメニュー14
	{ F_CUSTMENU_15,					HLP000186 },	//カスタムメニュー15
	{ F_CUSTMENU_16,					HLP000186 },	//カスタムメニュー16
	{ F_CUSTMENU_17,					HLP000186 },	//カスタムメニュー17
	{ F_CUSTMENU_18,					HLP000186 },	//カスタムメニュー18
	{ F_CUSTMENU_19,					HLP000186 },	//カスタムメニュー19
	{ F_CUSTMENU_20,					HLP000186 },	//カスタムメニュー20
	{ F_CUSTMENU_21,					HLP000186 },	//カスタムメニュー21
	{ F_CUSTMENU_22,					HLP000186 },	//カスタムメニュー22
	{ F_CUSTMENU_23,					HLP000186 },	//カスタムメニュー23
	{ F_CUSTMENU_24,					HLP000186 },	//カスタムメニュー24

	/* ウィンドウ系 */
	{ F_SPLIT_V,						HLP000093 },	//上下に分割
	{ F_SPLIT_H,						HLP000094 },	//左右に分割
	{ F_SPLIT_VH,						HLP000095 },	//縦横に分割
	{ F_WINCLOSE,						HLP000018 },	//ウィンドウを閉じる
	{ F_WIN_CLOSEALL,					HLP000019 },	//すべてのウィンドウを閉じる
	{ F_NEXTWINDOW,						HLP000092 },	//次のウィンドウ
	{ F_PREVWINDOW,						HLP000091 },	//前のウィンドウ
	{ F_WINLIST,						HLP000314 },	//ウィンドウ一覧
	{ F_DLGWINLIST,						HLP000372 },	//ウィンドウ一覧表示
	{ F_BIND_WINDOW,					HLP000311 },	//結合して表示
	{ F_CASCADE,						HLP000138 },	//重ねて表示
	{ F_TILE_V,							HLP000140 },	//上下に並べて表示
	{ F_TILE_H,							HLP000139 },	//左右に並べて表示
	{ F_TOPMOST,						HLP000312 },	//常に手前に表示
	{ F_MAXIMIZE_V,						HLP000141 },	//縦方向に最大化
	{ F_MAXIMIZE_H,						HLP000098 },	//横方向に最大化
	{ F_MINIMIZE_ALL,					HLP000096 },	//すべて最小化
	{ F_REDRAW,							HLP000187 },	//再描画
	{ F_WIN_OUTPUT,						HLP000188 },	//アウトプットウィンドウ表示
	{ F_GROUPCLOSE,						HLP000320 },	//グループを閉じる
	{ F_NEXTGROUP,						HLP000321 },	//次のグループ
	{ F_PREVGROUP,						HLP000322 },	//前のグループ
	{ F_TAB_MOVERIGHT,					HLP000323 },	//タブを右に移動
	{ F_TAB_MOVELEFT,					HLP000324 },	//タブを左に移動
	{ F_TAB_SEPARATE,					HLP000325 },	//新規グループ
	{ F_TAB_JOINTNEXT,					HLP000326 },	//次のグループに移動
	{ F_TAB_JOINTPREV,					HLP000327 },	//前のグループに移動
	{ F_TAB_CLOSEOTHER,					HLP000333 },	//このタブ以外を閉じる
	{ F_TAB_CLOSELEFT,					HLP000334 },	//左をすべて閉じる
	{ F_TAB_CLOSERIGHT,					HLP000335 },	//右をすべて閉じる

	/* 支援 */
	{ F_HOKAN,							HLP000111 },	//入力補完機能
	{ F_TOGGLE_KEY_SEARCH,				HLP000318 },	//キャレット位置辞書検索機能ON/OFF
	{ F_HELP_CONTENTS,					HLP000100 },	//ヘルプ目次
	{ F_HELP_SEARCH,					HLP000101 },	//ヘルプキーワード検索
	{ F_MENU_ALLFUNC,					HLP000189 },	//コマンド一覧
	{ F_EXTHELP1,						HLP000190 },	//外部ヘルプ１
	{ F_EXTHTMLHELP,					HLP000191 },	//外部HTMLヘルプ
	{ F_ABOUT,							HLP000102 },	//バージョン情報

	/* その他 */
};

//! 機能番号に応じてヘルプトピック番号を返す
/*!
	@author Stonee
	@date	2001/02/23
	@param nFuncID 機能番号
	@return ヘルプトピック番号。該当IDが無い場合には0を返す。

	内容はcase文の羅列。

	@par history
	2001.12.22 YAZAKI sakura.hhを参照するように変更
*/
int FuncID_To_HelpContextID(EFunctionCode nFuncID) noexcept
{
	if (g_FuncHelpMap.contains(nFuncID)) {
		return g_FuncHelpMap.at(nFuncID);
	}
	if (IDM_SELMRU <= nFuncID && nFuncID < IDM_SELMRU + int(MAX_MRU)) {
		return HLP000029;	//最近使ったファイル
	}
	if (IDM_SELOPENFOLDER <= nFuncID && nFuncID < IDM_SELOPENFOLDER + int(MAX_OPENFOLDER)) {
		return HLP000023;	//最近使ったフォルダー
	}
	if (IDM_SELWINDOW <= nFuncID && nFuncID < IDM_SELWINDOW + int(MAX_EDITWINDOWS)) {
		return HLP000097;	//ウィンドウリスト
	}
	if (F_USERMACRO_0 <= nFuncID && nFuncID < F_USERMACRO_0 + int(MAX_CUSTMACRO)) {
		return HLP000202;	//登録済みマクロ
	}
	return 0;
}

namespace func {

/* 機能が利用可能か調べる */
bool isEnabled(EFunctionCode nId) noexcept
{
	const auto pShareData = GetDllShareDataPtr();
	if (!pShareData) return false;

	const auto pcEditDoc = GetDocument();
	const auto pcEditWnd = GetEditWndPtr();

	/* 書き換え禁止のときを一括チェック */
	if (pcEditDoc && pcEditDoc->IsModificationForbidden(nId))
		return false;

	switch( nId ){
	case F_RECKEYMACRO:	/* キーマクロの記録開始／終了 */
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			return pShareData->m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow();	/* キーボードマクロを記録中のウィンドウ */
		}else{
			return true;
		}
	case F_SAVEKEYMACRO:	/* キーマクロの保存 */
		//	Jun. 16, 2002 genta
		//	キーマクロエンジン以外のマクロを読み込んでいるときは
		//	実行はできるが保存はできない．
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			return pShareData->m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow();	/* キーボードマクロを記録中のウィンドウ */
		}else{
			return CEditApp::getInstance()->m_pcSMacroMgr->IsSaveOk();
		}
	case F_EXECKEYMACRO:	/* キーマクロの実行 */
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			return pShareData->m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow();	/* キーボードマクロを記録中のウィンドウ */
		}else{
			//@@@ 2002.1.24 YAZAKI m_szKeyMacroFileNameにファイル名がコピーされているかどうか。
			return pShareData->m_Common.m_sMacro.m_szKeyMacroFileName[0];
		}
	case F_LOADKEYMACRO:	/* キーマクロの読み込み */
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			return pShareData->m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow();	/* キーボードマクロを記録中のウィンドウ */
		}else{
			return true;
		}
	case F_EXECEXTMACRO:	/* 名前を指定してマクロ実行 */
		return true;

	case F_SEARCH_CLEARMARK:	//検索マークのクリア
		return true;

	case F_JUMP_SRCHSTARTPOS:	// 検索開始位置へ戻る
		return pcEditWnd && pcEditWnd->GetActiveView().m_ptSrchStartPos_PHY.BothNatural();

	case F_COMPARE:	/* ファイル内容比較 */
		return 2 <= pShareData->m_sNodes.m_nEditArrNum;

	case F_DIFF_NEXT:	/* 次の差分へ */	//@@@ 2002.05.25 MIK
	case F_DIFF_PREV:	/* 前の差分へ */	//@@@ 2002.05.25 MIK
	case F_DIFF_RESET:	/* 差分の全解除 */	//@@@ 2002.05.25 MIK
		return CDiffManager::getInstance()->IsDiffUse();

	case F_DIFF_DIALOG:	/* DIFF差分表示 */	//@@@ 2002.05.25 MIK
		//if( pcEditDoc->IsModified() ) return false;
		//if( ! pcEditDoc->m_cDocFile.GetFilePathClass().IsValidPath() ) return false;
		return true;

	case F_BEGIN_BOX:	//矩形範囲選択開始
	case F_UP_BOX:
	case F_DOWN_BOX:
	case F_LEFT_BOX:
	case F_RIGHT_BOX:
	case F_UP2_BOX:
	case F_DOWN2_BOX:
	case F_WORDLEFT_BOX:
	case F_WORDRIGHT_BOX:
	case F_GOLOGICALLINETOP_BOX:
	case F_GOLINETOP_BOX:
	case F_GOLINEEND_BOX:
	case F_HalfPageUp_BOX:
	case F_HalfPageDown_BOX:
	case F_1PageUp_BOX:
	case F_1PageDown_BOX:
	case F_GOFILETOP_BOX:
	case F_GOFILEEND_BOX:
		return pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH;	/* 現在のフォントは固定幅フォントである */

	case F_PASTEBOX:
		/* クリップボードから貼り付け可能か？ */
		return pcEditDoc && pcEditDoc->m_cDocEditor.IsEnablePaste() && pShareData->m_Common.m_sView.m_bFontIs_FIXED_PITCH;

	case F_PASTE:
		/* クリップボードから貼り付け可能か？ */
		return pcEditDoc && pcEditDoc->m_cDocEditor.IsEnablePaste();

	case F_FILENEW:	/* 新規作成 */
	case F_GREP_DIALOG:	/* Grep */
	case F_GREP_REPLACE_DLG:
		/* 編集ウィンドウの上限チェック */
		return pShareData->m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ;

	case F_FILESAVE:	/* 上書き保存 */
		if (pcEditDoc && !CAppMode::getInstance()->IsViewMode() ){	/* ビューモード */
			return pcEditDoc->m_cDocEditor.IsModified() ||	// 変更フラグ
				pcEditDoc->m_cDocFile.IsChgCodeSet() ||		// 文字コードの変更
				/* 無変更でも上書きするか */
				pShareData->m_Common.m_sFile.m_bEnableUnmodifiedOverwrite;
		}else{
			return false;
		}
	case F_COPYLINES:				//選択範囲内全行コピー
	case F_COPYLINESASPASSAGE:		//選択範囲内全行引用符付きコピー
	case F_COPYLINESWITHLINENUMBER:	//選択範囲内全行行番号付きコピー
	case F_COPY_COLOR_HTML:				//選択範囲内色付きHTMLコピー
	case F_COPY_COLOR_HTML_LINENUMBER:	//選択範囲内行番号色付きHTMLコピー
		//テキストが選択されていればtrue
		return pcEditWnd && pcEditWnd->GetActiveView().GetSelectionInfo().IsTextSelected();

	case F_TOLOWER:					/* 小文字 */
	case F_TOUPPER:					/* 大文字 */
	case F_TOHANKAKU:				/* 全角→半角 */
	case F_TOHANKATA:				/* 全角カタカナ→半角カタカナ */	//Aug. 29, 2002 ai
	case F_TOZENEI:					/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	case F_TOHANEI:					/* 全角英数→半角英数 */
	case F_TOZENKAKUKATA:			/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	case F_TOZENKAKUHIRA:			/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	case F_HANKATATOZENKATA:		/* 半角カタカナ→全角カタカナ */
	case F_HANKATATOZENHIRA:		/* 半角カタカナ→全角ひらがな */
	case F_TABTOSPACE:				/* TAB→空白 */
	case F_SPACETOTAB:				/* 空白→TAB */  //---- Stonee, 2001/05/27
	case F_CODECNV_AUTO2SJIS:		/* 自動判別→SJISコード変換 */
	case F_CODECNV_EMAIL:			/* E-Mail(JIS→SJIS)コード変換 */
	case F_CODECNV_EUC2SJIS:		/* EUC→SJISコード変換 */
	case F_CODECNV_UNICODE2SJIS:	/* Unicode→SJISコード変換 */
	case F_CODECNV_UNICODEBE2SJIS:	/* UnicodeBE→SJISコード変換 */
	case F_CODECNV_UTF82SJIS:		/* UTF-8→SJISコード変換 */
	case F_CODECNV_UTF72SJIS:		/* UTF-7→SJISコード変換 */
	case F_CODECNV_SJIS2JIS:		/* SJIS→JISコード変換 */
	case F_CODECNV_SJIS2EUC:		/* SJIS→EUCコード変換 */
	case F_CODECNV_SJIS2UTF8:		/* SJIS→UTF-8コード変換 */
	case F_CODECNV_SJIS2UTF7:		/* SJIS→UTF-7コード変換 */
	case F_BASE64DECODE:			/* Base64デコードして保存 */
	case F_UUDECODE:				//uudecodeして保存	//Oct. 17, 2000 jepro 説明を「選択部分をUUENCODEデコード」から変更
		// テキストが選択されていればtrue
		return pcEditWnd && pcEditWnd->GetActiveView().GetSelectionInfo().IsTextSelected();

	case F_CUT_LINE:	//行切り取り(折り返し単位)
	case F_DELETE_LINE:	//行削除(折り返し単位)
		// テキストが選択されていなければtrue
		return pcEditWnd && !pcEditWnd->GetActiveView().GetSelectionInfo().IsTextSelected();

	case F_UNDO:		return pcEditDoc && pcEditDoc->m_cDocEditor.IsEnableUndo();	/* Undo(元に戻す)可能な状態か？ */
	case F_REDO:		return pcEditDoc && pcEditDoc->m_cDocEditor.IsEnableRedo();	/* Redo(やり直し)可能な状態か？ */

	case F_OPEN_HfromtoC:				//同名のC/C++ヘッダー(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
		return pcEditDoc && pcEditDoc->m_cDocFile.GetFilePathClass().IsValidPath() && pcEditWnd && pcEditWnd->GetActiveView().GetCommander().Command_OPEN_HfromtoC(TRUE);

	case F_COPYPATH:
	case F_COPYDIRPATH:
	case F_COPYTAG:
	case F_COPYFNAME:					// 2002/2/3 aroka
	case F_PLSQL_COMPILE_ON_SQLPLUS:	/* Oracle SQL*Plusで実行 */
	case F_BROWSE:						//ブラウズ
	//case F_VIEWMODE:					//ビューモード	//	Sep. 10, 2002 genta 常に使えるように
	//case F_PROPERTY_FILE:				//ファイルのプロパティ	// 2009.04.11 ryoji コメントアウト
	case F_OPEN_FOLDER_IN_EXPLORER:		//ファイルの場所を開く
		return pcEditDoc && pcEditDoc->m_cDocFile.GetFilePathClass().IsValidPath();	// 現在編集中のファイルのパス名をクリップボードにコピーできるか

	case F_OPEN_COMMAND_PROMPT:			//コマンドプロンプトを開く
	case F_OPEN_COMMAND_PROMPT_AS_ADMIN:	//管理者としてコマンドプロンプトを開く
		/* UNC パスの場合は、メニューを無効にする */
		return !(pcEditDoc && (!pcEditDoc->m_cDocFile.GetFilePathClass().IsValidPath() || ::PathIsUNCW(pcEditDoc->m_cDocFile.GetFilePath())));

	case F_OPEN_POWERSHELL:				//PowerShellを開く
	case F_OPEN_POWERSHELL_AS_ADMIN:	//管理者としてPowerShellを開く
		// UNC パスでも動作するので、UNC パスかどうかはチェックしない
		if (pcEditDoc && !pcEditDoc->m_cDocFile.GetFilePathClass().IsValidPath())
		{
			return false;
		}
		return IsPowerShellAvailable();

	case F_JUMPHIST_PREV:	//	移動履歴: 前へ
		return pcEditWnd && pcEditWnd->GetActiveView().m_cHistory->CheckPrev();

	case F_JUMPHIST_NEXT:	//	移動履歴: 次へ
		return pcEditWnd && pcEditWnd->GetActiveView().m_cHistory->CheckNext();

	case F_JUMPHIST_SET:	//	現在位置を移動履歴に登録
		return true;

	case F_DIRECT_TAGJUMP:	//ダイレクトタグジャンプ	//@@@ 2003.04.15 MIK
	case F_TAGJUMP_KEYWORD:	//キーワードを指定してダイレクトタグジャンプ	//@@@ 2005.03.31 MIK
		return !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode
			&& pcEditDoc && pcEditDoc->m_cDocFile.GetFilePathClass().IsValidPath();
	
	//タブモード時はウインドウ並べ替え禁止です。	@@@ 2003.06.12 MIK
	case F_TILE_H:
	case F_TILE_V:
	case F_CASCADE:
		//Start 2004.07.15 Kazika タブウィンド時も実行可能
		return true;
		//End 2004.07.15 Kazika
	case F_BIND_WINDOW:	//2004.07.14 Kazika 新規追加
	case F_TAB_MOVERIGHT:	// 2007.06.20 ryoji 追加
	case F_TAB_MOVELEFT:	// 2007.06.20 ryoji 追加
	case F_TAB_CLOSELEFT:	// 2009.12.26 syat 追加
	case F_TAB_CLOSERIGHT:	// 2009.12.26 syat 追加
		//非タブモード時はウィンドウを結合して表示できない
		return pShareData->m_Common.m_sTabBar.m_bDispTabWnd != FALSE;
	case F_GROUPCLOSE:		// 2007.06.20 ryoji 追加
	case F_NEXTGROUP:		// 2007.06.20 ryoji 追加
	case F_PREVGROUP:		// 2007.06.20 ryoji 追加
		return ( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin );
	case F_TAB_SEPARATE:	// 2007.06.20 ryoji 追加
	case F_TAB_JOINTNEXT:	// 2007.06.20 ryoji 追加
	case F_TAB_JOINTPREV:	// 2007.06.20 ryoji 追加
	case F_FILENEW_NEWWINDOW:	// 2011.11.15 syat 追加
		return ( pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin );
	default:
		break;
	}
	return true;
}

/* 機能がチェック状態か調べる */
bool isChecked(EFunctionCode nId) noexcept
{
	const auto pShareData = GetDllShareDataPtr();
	if (!pShareData) return false;

	const auto pcEditDoc = GetDocument();
	const auto pcEditWnd = GetEditWndPtr();

	//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことにより、プレビュー判定削除
	ECodeType eDocCode = pcEditDoc ? pcEditDoc->GetDocumentEncoding() : CODE_NONE;
	switch( nId ){
	case F_FILE_REOPEN_SJIS:		return CODE_SJIS == eDocCode;
	case F_FILE_REOPEN_JIS:			return CODE_JIS == eDocCode;
	case F_FILE_REOPEN_EUC:			return CODE_EUC == eDocCode;
	case F_FILE_REOPEN_LATIN1:		return CODE_LATIN1 == eDocCode;		// 2010/3/20 Uchi
	case F_FILE_REOPEN_UNICODE:		return CODE_UNICODE == eDocCode;
	case F_FILE_REOPEN_UNICODEBE:	return CODE_UNICODEBE == eDocCode;
	case F_FILE_REOPEN_UTF8:		return CODE_UTF8 == eDocCode;
	case F_FILE_REOPEN_CESU8:		return CODE_CESU8 == eDocCode;
	case F_FILE_REOPEN_UTF7:		return CODE_UTF7 == eDocCode;
	case F_RECKEYMACRO:	/* キーマクロの記録開始／終了 */
		if( pShareData->m_sFlags.m_bRecordingKeyMacro ){	/* キーボードマクロの記録中 */
			return pShareData->m_sFlags.m_hwndRecordingKeyMacro == GetMainWindow();	/* キーボードマクロを記録中のウィンドウ */
		}else{
			return false;
		}
	case F_SHOWTOOLBAR:			return pcEditWnd && pcEditWnd->m_cToolbar.GetToolbarHwnd();
	case F_SHOWFUNCKEY:			return pcEditWnd && pcEditWnd->m_cFuncKeyWnd.GetHwnd();
	case F_SHOWTAB:				return pcEditWnd && pcEditWnd->m_cTabWnd.GetHwnd();
	case F_SHOWSTATUSBAR:		return pcEditWnd && pcEditWnd->m_cStatusBar.GetStatusHwnd();
	case F_SHOWMINIMAP:			return pcEditWnd && pcEditWnd->GetMiniMap().GetHwnd();
	// 2008.05.30 nasukoji	テキストの折り返し方法
	case F_TMPWRAPNOWRAP:		return pcEditDoc && WRAP_NO_TEXT_WRAP  == pcEditDoc->m_nTextWrapMethodCur;	// 折り返さない
	case F_TMPWRAPSETTING:		return pcEditDoc && WRAP_SETTING_WIDTH == pcEditDoc->m_nTextWrapMethodCur;	// 指定桁で折り返す
	case F_TMPWRAPWINDOW:		return pcEditDoc && WRAP_WINDOW_WIDTH  == pcEditDoc->m_nTextWrapMethodCur;	// 右端で折り返す
	// 2009.07.06 syat  文字カウント方法
	case F_SELECT_COUNT_MODE:
		return !pcEditWnd || SELECT_COUNT_TOGGLE != pcEditWnd->m_nSelectCountMode
			? pShareData->m_Common.m_sStatusbar.m_bDispSelCountByByte
			: SELECT_COUNT_BY_BYTE == pcEditWnd->m_nSelectCountMode;
	// Mar. 6, 2002 genta
	case F_VIEWMODE:			return CAppMode::getInstance()->IsViewMode(); //ビューモード
	//	From Here 2003.06.23 Moca
	case F_CHGMOD_EOL_CRLF:		return pcEditDoc && EEolType::cr_and_lf == pcEditDoc->m_cDocEditor.GetNewLineCode();
	case F_CHGMOD_EOL_LF:		return pcEditDoc && EEolType::line_feed == pcEditDoc->m_cDocEditor.GetNewLineCode();
	case F_CHGMOD_EOL_CR:		return pcEditDoc && EEolType::carriage_return == pcEditDoc->m_cDocEditor.GetNewLineCode();
	//	To Here 2003.06.23 Moca
	//	2003.07.21 genta
	case F_CHGMOD_INS:			return pcEditDoc && pcEditDoc->m_cDocEditor.IsInsMode();	//	Oct. 2, 2005 genta 挿入モードはドキュメント毎に補完するように変更した
	case F_TOGGLE_KEY_SEARCH:	return pShareData->m_Common.m_sSearch.m_bUseCaretKeyWord != FALSE;	//	2007.02.03 genta キーワードポップアップのON/OFF状態を反映する
	case F_BIND_WINDOW:			return ((pShareData->m_Common.m_sTabBar.m_bDispTabWnd) && !(pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin));	//2004.07.14 Kazika 追加
	case F_TOPMOST:				return ((DWORD)::GetWindowLongPtrW( GetMainWindow(), GWL_EXSTYLE ) & WS_EX_TOPMOST) != 0;	// 2004.09.21 Moca
	// Jan. 10, 2004 genta インクリメンタルサーチ
	case F_ISEARCH_NEXT:
	case F_ISEARCH_PREV:
	case F_ISEARCH_REGEXP_NEXT:
	case F_ISEARCH_REGEXP_PREV:
	case F_ISEARCH_MIGEMO_NEXT:
	case F_ISEARCH_MIGEMO_PREV:
		return pcEditWnd && pcEditWnd->GetActiveView().IsISearchEnabled(nId);

	case F_OUTLINE_TOGGLE: // 20060201 aroka アウトラインウィンドウ
		// ToDo:ブックマークリストが出ているときもへこんでしまう。
		return pcEditWnd && pcEditWnd->m_cDlgFuncList.GetHwnd();

	default:
		break;
	}
	//End 2004.07.14 Kazika

	return false;
}

} // namespace func
