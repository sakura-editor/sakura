//	$Id$
/*!	@file
	@brief 機能分類定義

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//	Sept. 14, 2000 Jepro note: functions & commands list
//	キーワード：コマンド一覧順序
//	ここに登録されているコマンドが共通設定の機能種別に表示され、キー割り当てにも設定できるようになる
//	このファイルは「コマンド一覧」のメニューの順番や表示にも使われている
//	sakura_rc.rcファイルの下のほうにあるString Tableも参照のこと

#include "funccode.h"

//using namespace nsFuncCode;

const char* nsFuncCode::ppszFuncKind[] = {
//	"--未定義--",	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更
//	Oct. 16, 2000 JEPRO 表示の順番をメニューバーのそれに合わせるように少し入れ替えた(下の個別のものも全部)
	"ファイル操作系",
	"編集系",
	"カーソル移動系",
	"選択系",		//Oct. 15, 2000 JEPRO 「カーソル移動系」が多くなったので「選択系」として独立化(サブメニュー化は構造上できないので)
	"矩形選択系",	//Oct. 17, 2000 JEPRO 「選択系」に一緒にすると多くなりすぎるので「矩形選択系」も独立させた
	"クリップボード系",
	"挿入系",
	"変換系",
	"検索系",
	"モード切り替え系",
	"設定系",
	"マクロ系",
	//	Oct. 15, 2001 genta カスタムメニューの文字列をは動的に変更可能にするためここからは外す．
//	"カスタムメニュー",	//Oct. 21, 2000 JEPRO 「その他」から独立分離化
	"ウィンドウ系",
	"支援",
	"その他"
};
const int nsFuncCode::nFuncKindNum = sizeof(nsFuncCode::ppszFuncKind) / sizeof(nsFuncCode::ppszFuncKind[0]);

//	From Here Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように以下の4行をコメントアウトに変更
//const int pnFuncList_Undef[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List0→List_Undef)
//	0
//};
//int	nFincList_Undef_Num = sizeof( pnFuncList_Undef ) / sizeof( pnFuncList_Undef[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List0→List_Undef)
//	To Here Oct. 14, 2000


/* ファイル操作系 */
const int pnFuncList_File[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List5→List_File)
	F_FILENEW			,	//新規作成
	F_FILEOPEN			,	//開く
	F_FILESAVE			,	//上書き保存
	F_FILESAVEAS_DIALOG	,	//名前を付けて保存
	F_FILECLOSE			,	//閉じて(無題)	//Oct. 17, 2000 jepro 「ファイルを閉じる」というキャプションを変更
	F_FILECLOSE_OPEN	,	//閉じて開く
	F_WINCLOSE			,	//ウィンドウを閉じる	//Oct.17,2000 コマンド本家は「ウィンドウ系」	//Feb. 18, 2001	JEPRO 下から移動した
	F_FILE_REOPEN_SJIS		,//SJISで開き直す
	F_FILE_REOPEN_JIS		,//JISで開き直す
	F_FILE_REOPEN_EUC		,//EUCで開き直す
	F_FILE_REOPEN_UNICODE	,//Unicodeで開き直す
	F_FILE_REOPEN_UTF8		,//UTF-8で開き直す
	F_FILE_REOPEN_UTF7		,//UTF-7で開き直す
	F_PRINT				,	//印刷
	F_PRINT_PREVIEW		,	//印刷プレビュー
	F_PRINT_PAGESETUP	,	//印刷ページ設定	//Sept. 14, 2000 jepro 「印刷のページレイアウトの設定」から変更
	F_OPEN_HfromtoC		,	//同名のC/C++ヘッダ(ソース)を開く	//Feb. 7, 2001 JEPRO 追加
	F_OPEN_HHPP			,	//同名のC/C++ヘッダファイルを開く	//Feb. 9, 2001 jepro「.cまたは.cppと同名の.hを開く」から変更
	F_OPEN_CCPP			,	//同名のC/C++ソースファイルを開く	//Feb. 9, 2001 jepro「.hと同名の.c(なければ.cpp)を開く」から変更
	F_ACTIVATE_SQLPLUS			,	/* Oracle SQL*Plusをアクティブ表示 */	//Sept. 20, 2000 「コンパイル」JEPRO アクティブ表示を上に移動した
	F_PLSQL_COMPILE_ON_SQLPLUS	,	/* Oracle SQL*Plusで実行 */	//Sept. 20, 2000 jepro 説明の「コンパイル」を「実行」に統一
	F_BROWSE			,	//ブラウズ
	F_READONLY			,	//読み取り専用
	F_PROPERTY_FILE		,	/* ファイルのプロパティ */
	F_WIN_CLOSEALL		,	//すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)	//Oct.17,2000 コマンド本家は「ウィンドウ系」	//Feb. 18, 2001	JEPRO 上から移動した
	F_EXITALL				//サクラエディタの全終了	//Dec. 27, 2000 JEPRO 追加
};
const int nFincList_File_Num = sizeof( pnFuncList_File ) / sizeof( pnFuncList_File[0] );	//Oct. 16, 2000 JEPRO 配列名変更(FuncList5→FuncList_File)


/* 編集系 */
const int pnFuncList_Edit[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)
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
const int nFincList_Edit_Num = sizeof( pnFuncList_Edit ) / sizeof( pnFuncList_Edit[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)


/* カーソル移動系 */
const int pnFuncList_Move[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)
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
	F_JUMP_DIALOG		,	//指定行ヘジャンプ	//Sept. 17, 2000 JEPRO コマンド本家は「検索系」
	F_JUMPPREV			,	//移動履歴: 前へ
	F_JUMPNEXT			,	//移動履歴: 次へ
	F_WndScrollDown		,	//テキストを１行下へスクロール	// 2001/06/20 asa-o
	F_WndScrollUp		,	//テキストを１行上へスクロール	// 2001/06/20 asa-o
	F_GONEXTPARAGRAPH	,	//次の段落へ
	F_GOPREVPARAGRAPH		//前の段落へ
};
const int nFincList_Move_Num = sizeof( pnFuncList_Move ) / sizeof( pnFuncList_Move[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)


/* 選択系 */	//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動
const int pnFuncList_Select[] = {
	F_SELECTWORD			,	//現在位置の単語選択
	F_SELECTALL				,	//すべて選択
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
	F_GONEXTPARAGRAPH_SEL	,	//(範囲選択)次の段落へ
	F_GOPREVPARAGRAPH_SEL		//(範囲選択)前の段落へ
};
const int nFincList_Select_Num = sizeof( pnFuncList_Select ) / sizeof( pnFuncList_Select[0] );


/* 矩形選択系 */	//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
const int pnFuncList_Box[] = {
//	F_BOXSELALL			,	//矩形ですべて選択
	F_BEGIN_BOX			,	//矩形範囲選択開始
/*
	F_UP_BOX				//(矩形選択)カーソル上移動
	F_DOWN_BOX			,	//(矩形選択)カーソル下移動
	F_LEFT_BOX			,	//(矩形選択)カーソル左移動
	F_RIGHT_BOX			,	//(矩形選択)カーソル右移動
	F_UP2_BOX			,	//(矩形選択)カーソル上移動(２行ごと)
	F_DOWN2_BOX			,	//(矩形選択)カーソル下移動(２行ごと)
	F_WORDLEFT_BOX		,	//(矩形選択)単語の左端に移動
	F_WORDRIGHT_BOX		,	//(矩形選択)単語の右端に移動
	F_GOLINETOP_BOX		,	//(矩形選択)行頭に移動(折り返し単位)
	F_GOLINEEND_BOX		,	//(矩形選択)行末に移動(折り返し単位)
	F_HalfPageUp_Box	,	//(矩形選択)半ページアップ
	F_HalfPageDown_Box	,	//(矩形選択)半ページダウン
	F_1PageUp_Box		,	//(矩形選択)１ページアップ
	F_1PageDown_Box		,	//(矩形選択)１ページダウン
	F_GOFILETOP_BOX		,	//(矩形選択)ファイルの先頭に移動
	F_GOFILEEND_BOX			//(矩形選択)ファイルの最後に移動
*/
};
const int nFincList_Box_Num = sizeof( pnFuncList_Box ) / sizeof( pnFuncList_Box[0] );


/* クリップボード系 */
const int pnFuncList_Clip[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List2→List_Clip)
	F_CUT						,	//切り取り(選択範囲をクリップボードにコピーして削除)
	F_COPY						,	//コピー(選択範囲をクリップボードにコピー)
	F_COPY_ADDCRLF				,	//折り返し位置に改行をつけてコピー(選択範囲をクリップボードにコピー)
	F_COPY_CRLF					,	//CRLF改行でコピー
	F_PASTE						,	//貼り付け(クリップボードから貼り付け)
	F_PASTEBOX					,	//矩形貼り付け(クリップボードから矩形貼り付け)
//	F_INSTEXT					,	//テキストを貼り付け		//Oct. 22, 2000 JEPRO ここに追加したが非公式機能なのか不明なのでコメントアウトにしておく
//	F_ADDTAIL					,	//最後にテキストを追加		//Oct. 22, 2000 JEPRO ここに追加したが非公式機能なのか不明なのでコメントアウトにしておく
	F_COPYLINES					,	//選択範囲内全行コピー
	F_COPYLINESASPASSAGE		,	//選択範囲内全行引用符付きコピー
	F_COPYLINESWITHLINENUMBER	,	//選択範囲内全行行番号付きコピー
	F_COPYFNAME					,	//このファイル名をクリップボードにコピー //2002/2/3 aroka
	F_COPYPATH					,	//このファイルのパス名をクリップボードにコピー
	F_COPYTAG					,	//このファイルのパス名とカーソル位置をコピー	//Sept. 14, 2000 JEPRO メニューに合わせて下に移動
	F_CREATEKEYBINDLIST				//キー割り当て一覧をコピー	//Sept. 15, 2000 JEPRO IDM_TESTのままではうまくいかないのでFに変えて登録	//Dec. 25, 2000 復活
};
const int nFincList_Clip_Num = sizeof( pnFuncList_Clip ) / sizeof( pnFuncList_Clip[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)


/* 挿入系 */
const int pnFuncList_Insert[] = {
	F_INS_DATE				,	// 日付挿入
	F_INS_TIME					// 時刻挿入
};
const int nFincList_Insert_Num = sizeof( pnFuncList_Insert ) / sizeof( pnFuncList_Insert[0] );


/* 変換系 */
const int pnFuncList_Convert[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)
	F_TOLOWER				,	//英大文字→英小文字
	F_TOUPPER				,	//英小文字→英大文字
	F_TOHANKAKU				,	/* 全角→半角 */
	F_TOZENEI				,	/* 半角英数→全角英数 */			//July. 30, 2001 Misaka
	F_TOHANEI				,	/* 全角英数→半角英数 */
	F_TOZENKAKUKATA			,	/* 半角＋全ひら→全角・カタカナ */	//Sept. 17, 2000 jepro 説明を「半角→全角カタカナ」から変更
	F_TOZENKAKUHIRA			,	/* 半角＋全カタ→全角・ひらがな */	//Sept. 17, 2000 jepro 説明を「半角→全角ひらがな」から変更
	F_HANKATATOZENKAKUKATA	,	/* 半角カタカナ→全角カタカナ */
	F_HANKATATOZENKAKUHIRA	,	/* 半角カタカナ→全角ひらがな */
	F_TABTOSPACE			,	/* TAB→空白 */
	F_SPACETOTAB			,	/* 空白→TAB */  //#### Stonee, 2001/05/27
	F_CODECNV_AUTO2SJIS		,	/* 自動判別→SJISコード変換 */
	F_CODECNV_EMAIL			,	//E-Mail(JIS→SJIS)コード変換
	F_CODECNV_EUC2SJIS		,	//EUC→SJISコード変換
	F_CODECNV_UNICODE2SJIS	,	//Unicode→SJISコード変換
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
const int nFincList_Convert_Num = sizeof( pnFuncList_Convert ) / sizeof( pnFuncList_Convert[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)


/* 検索系 */
const int pnFuncList_Search[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)
	F_SEARCH_DIALOG		,	//検索(単語検索ダイアログ)
	F_SEARCH_NEXT		,	//次を検索	//Sept. 16, 2000 JEPRO "次"を"前"の前に移動
	F_SEARCH_PREV		,	//前を検索
	F_REPLACE_DIALOG	,	//置換
	F_SEARCH_CLEARMARK	,	//検索マークのクリア
	F_GREP_DIALOG		,	//Grep
	F_JUMP_DIALOG		,	//指定行ヘジャンプ
	F_OUTLINE			,	//アウトライン解析
	F_TAGJUMP			,	//タグジャンプ機能
	F_TAGJUMPBACK		,	//タグジャンプバック機能
	F_COMPARE			,	//ファイル内容比較
	F_BRACKETPAIR		,	//対括弧の検索
	F_BOOKMARK_SET		,	//ブックマーク設定・解除
	F_BOOKMARK_NEXT		,	//次のブックマークへ
	F_BOOKMARK_PREV		,	//前のブックマークへ
	F_BOOKMARK_RESET	,	//ブックマークの全解除
	F_BOOKMARK_VIEW			//ブックマークの一覧
};
const int nFincList_Search_Num = sizeof( pnFuncList_Search ) / sizeof( pnFuncList_Search[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)


/* モード切り替え系 */	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)
const int pnFuncList_Mode[] = {
	F_CHGMOD_INS		,	//挿入／上書きモード切り替え
	F_CANCEL_MODE			//各種モードの取り消し
};
const int nFincList_Mode_Num = sizeof( pnFuncList_Mode ) / sizeof( pnFuncList_Mode[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)


/* 設定系 */
const int pnFuncList_Set[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)
	F_SHOWTOOLBAR		,	/* ツールバーの表示 */
	F_SHOWFUNCKEY		,	/* ファンクションキーの表示 */	//Sept. 14, 2000 JEPRO ファンクションキーとステータスバーを入れ替え
	F_SHOWSTATUSBAR		,	/* ステータスバーの表示 */
	F_TYPE_LIST			,	/* タイプ別設定一覧 */			//Sept. 17, 2000 JEPRO 設定系に入ってなかったので追加
	F_OPTION_TYPE		,	/* タイプ別設定 */
	F_OPTION			,	/* 共通設定 */
	F_FONT				,	/* フォント設定 */
	F_WRAPWINDOWWIDTH	,	/* 現在のウィンドウ幅で折り返し */	//Oct. 7, 2000 JEPRO WRAPWINDIWWIDTH を WRAPWINDOWWIDTH に変更
	F_PRINT_PAGESETUP		//印刷ページ設定				//Sept. 14, 2000 JEPRO 「印刷のページレイアウトの設定」を「印刷ページ設定」に変更	//Oct. 17, 2000 コマンド本家は「ファイル操作系」
};
int		nFincList_Set_Num = sizeof( pnFuncList_Set ) / sizeof( pnFuncList_Set[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)


/* マクロ系 */
const int pnFuncList_Macro[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)
	F_RECKEYMACRO	,	/* キーマクロの記録開始／終了 */
	F_SAVEKEYMACRO	,	/* キーマクロの保存 */
	F_LOADKEYMACRO	,	/* キーマクロの読み込み */
	F_EXECKEYMACRO	,	/* キーマクロの実行 */
//	From Here Sept. 20, 2000 JEPRO 名称CMMANDをCOMMANDに変更
//	F_EXECCMMAND		/* 外部コマンド実行 */
	F_EXECCOMMAND_DIALOG	/* 外部コマンド実行 */
//	To Here Sept. 20, 2000

};
const int nFincList_Macro_Num = sizeof( pnFuncList_Macro) / sizeof( pnFuncList_Macro[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)


/* カスタムメニュー */	//Oct. 21, 2000 JEPRO 「その他」から分離独立化
#if 0
//	From Here Oct. 15, 2001 genta
//	カスタムメニューの文字列を動的に変更可能にするためこれは削除．
const int pnFuncList_Menu[] = {
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
const int nFincList_Menu_Num = sizeof( pnFuncList_Menu ) / sizeof( pnFuncList_Menu[0] );	//Oct. 21, 2000 JEPRO 「その他」から分離独立化
#endif

/* ウィンドウ系 */
const int pnFuncList_Win[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)
	F_SPLIT_V			,	//上下に分割	//Sept. 17, 2000 jepro 説明の「縦」を「上下に」に変更
	F_SPLIT_H			,	//左右に分割	//Sept. 17, 2000 jepro 説明の「横」を「左右に」に変更
	F_SPLIT_VH			,	//縦横に分割	//Sept. 17, 2000 jepro 説明に「に」を追加
	F_WINCLOSE			,	//ウィンドウを閉じる
	F_WIN_CLOSEALL		,	//すべてのウィンドウを閉じる	//Oct. 17, 2000 JEPRO 名前を変更(F_FILECLOSEALL→F_WIN_CLOSEALL)
	F_NEXTWINDOW		,	//次のウィンドウ
	F_PREVWINDOW		,	//前のウィンドウ
	F_CASCADE			,	//重ねて表示
	F_TILE_V			,	//上下に並べて表示
	F_TILE_H			,	//左右に並べて表示
	F_MAXIMIZE_V		,	//縦方向に最大化
	F_MAXIMIZE_H		,	//横方向に最大化 //2001.02.10 by MIK
	F_MINIMIZE_ALL		,	//すべて最小化	//Sept. 17, 2000 jepro 説明の「全て」を「すべて」に統一
	F_REDRAW			,	//再描画
	F_WIN_OUTPUT			//アウトプットウィンドウ表示
};
const int nFincList_Win_Num = sizeof( pnFuncList_Win ) / sizeof( pnFuncList_Win[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)


/* 支援 */
const int pnFuncList_Support[] = {	//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)
	F_HOKAN						,	/* 入力補完 */
//Sept. 15, 2000→Nov. 25, 2000 JEPRO //ショートカットキーがうまく働かないので殺してあった下の2行を修正・復活
	F_HELP_CONTENTS				,	/* ヘルプ目次 */			//Nov. 25, 2000 JEPRO 追加
	F_HELP_SEARCH				,	/* ヘルプキーワード検索 */	//Nov. 25, 2000 JEPRO 追加
	F_MENU_ALLFUNC				,	/* コマンド一覧 */
	F_EXTHELP1					,	/* 外部ヘルプ１ */
	F_EXTHTMLHELP				,	/* 外部HTMLヘルプ */
	F_ABOUT							/* バージョン情報 */	//Dec. 24, 2000 JEPRO 追加
};
const int nFincList_Support_Num = sizeof( pnFuncList_Support ) / sizeof( pnFuncList_Support[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)


/* その他 */	//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)
const int pnFuncList_Others[] = {
//	F_SENDMAIL					,	/* メール送信 */
	F_DISABLE				//Oct. 21, 2000 JEPRO 何もないとエラーになってしまうのでダミーで[未定義]を入れておく
};
const int nFincList_Others_Num = sizeof( pnFuncList_Others ) / sizeof( pnFuncList_Others[0] );	//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)





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
const int *	nsFuncCode::ppnFuncListArr[] = {
//	(int*)pnFuncList_Undef,	//Oct. 14, 2000 JEPRO 「--未定義--」を表示させないように変更	//Oct. 16, 2000 JEPRO 変数名変更(List0→List_Undef)
	(int*)pnFuncList_File,	/* ファイル操作系 */	//Oct. 16, 2000 JEPRO 変数名変更(List5→List_File)
	(int*)pnFuncList_Edit,	/* 編集系 */			//Oct. 16, 2000 JEPRO 変数名変更(List3→List_Edit)
	(int*)pnFuncList_Move,	/* カーソル移動系 */	//Oct. 16, 2000 JEPRO 変数名変更(List1→List_Move)
	(int*)pnFuncList_Select,/* 選択系 */			//Oct. 15, 2000 JEPRO 「カーソル移動系」から(選択)を移動  (矩形選択)は新設され次第ここにおく
	(int*)pnFuncList_Box,	/* 矩形選択系 */		//Oct. 17, 2000 JEPRO (矩形選択)が新設され次第ここにおく
	(int*)pnFuncList_Clip,	/* クリップボード系 */	//Oct. 16, 2000 JEPRO 変数名変更(List2→List_Clip)
	(int*)pnFuncList_Insert,/* 挿入系 */
	(int*)pnFuncList_Convert,/* 変換系 */			//Oct. 16, 2000 JEPRO 変数名変更(List6→List_Convert)
	(int*)pnFuncList_Search,/* 検索系 */			//Oct. 16, 2000 JEPRO 変数名変更(List4→List_Search)
	(int*)pnFuncList_Mode,	/* モード切り替え系 */	//Oct. 16, 2000 JEPRO 変数名変更(List8→List_Mode)
	(int*)pnFuncList_Set,	/* 設定系 */			//Oct. 16, 2000 JEPRO 変数名変更(List9→List_Set)
	(int*)pnFuncList_Macro,	/* マクロ系 */			//Oct. 16, 2000 JEPRO 変数名変更(List10→List_Macro)
//	カスタムメニューの文字列を動的に変更可能にするためこれは削除
//	(int*)pnFuncList_Menu,	/* カスタムメニュー */	//Oct. 21, 2000 JEPRO「その他」から分離独立化
	(int*)pnFuncList_Win,	/* ウィンドウ系 */		//Oct. 16, 2000 JEPRO 変数名変更(List7→List_Win)
	(int*)pnFuncList_Support,/* 支援 */				//Oct. 16, 2000 JEPRO 変数名変更(List11→List_Support)
	(int*)pnFuncList_Others	/* その他 */			//Oct. 16, 2000 JEPRO 変数名変更(List12→List_Others)
};
const int nsFuncCode::nFincListNumArrNum = sizeof( nsFuncCode::pnFuncListNumArr ) / sizeof( nsFuncCode::pnFuncListNumArr[0] );


/*[EOF]*/
