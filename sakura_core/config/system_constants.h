/*! @file
	@brief システム定数

	@date 2002.01.08 aroka  コントロールプロセスと起動処理のためにミューテックス名を追加
	@date 2006.04.10 ryoji  コントロールプロセス初期化完了を示すイベントフラグ名を追加
	@date 2007.09.05 kobake ANSI版と衝突を避けるため、名前変更
	@date 2007.09.20 kobake ANSI版とUNICODE版で別の名前を用いる
	@date 2009.01.17 nasukoji	マウスサイドボタンのキーコード定義追加（_WIN32_WINNTの都合によりシステム側で定義されない為）
	@date 2009.02.11 ryoji		共有メモリ履歴追加, NUM_TO_STRマクロ追加
	@date 2010.08.21 Moca		Win64を定数で分離
*/
/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_SYSTEM_CONSTANTS_608BC31D_86C2_4526_B749_70DBD090752A_H_
#define SAKURA_SYSTEM_CONSTANTS_608BC31D_86C2_4526_B749_70DBD090752A_H_


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       定数命名補助                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//数値定数の文字列化 2009.02.11 ryoji
#define _NUM_TO_STR(n) #n
#define NUM_TO_STR(n) _NUM_TO_STR(n)

//! デバッグ判別、定数サフィックス 2007.09.20 kobake
#ifdef _DEBUG
	#define _DEBUG_SUFFIX_ "_DEBUG"
#else
	#define _DEBUG_SUFFIX_ ""
#endif

//! ビルドコード判別、定数サフィックス 2007.09.20 kobake
#ifdef _UNICODE
	#define _CODE_SUFFIX_ "WP"
#else
	#define _CODE_SUFFIX_ "AP"
#endif

//! ターゲットマシン判別 2010.08.21 Moca 追加
#ifdef _WIN64
	#define CON_SKR_MACHINE_SUFFIX_ "M64"
#else
	#define CON_SKR_MACHINE_SUFFIX_ ""
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        共有メモリ                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//!	共有メモリのバージョン
/*!
	共有メモリのバージョン番号。共有メモリの形式を変更したときはここを1増やす。

	この値は共有メモリのバージョンフィールドに格納され、異なる構造の共有メモリを
	使うエディタが同時に起動しないようにする。

	設定可能な値は 1～unsinged intの最大値

	@sa Init()

	Version 1～24:
	いろいろ
	
	Version 25:
	m_bStopsBothEndsWhenSearchWord追加
	
	Version 26:
	MacroRecに、m_bReloadWhenExecuteを追加 2002/03/11 YAZAKI
	EditInfoに、m_szDocType追加 Mar. 7, 2002 genta
	
	Version 27:
	STypeConfigに、m_szOutlineRuleFilenameを追加 2002.04.01 YAZAKI 
	
	Version 28:
	PRINTSETTINGに、m_bPrintKinsokuHead、m_bPrintKinsokuTailを追加 2002.04.09 MIK
	STypeConfigに、m_bKinsokuHead、m_bKinsokuTail、m_szKinsokuHead、m_szKinsokuTailを追加 2002.04.09 MIK

	Version 29:
	PRINTSETTINGに、m_bPrintKinsokuRetを追加 2002.04.13 MIK
	STypeConfigに、m_bKinsokuRetを追加 2002.04.13 MIK

	Version 30:
	PRINTSETTINGに、m_bPrintKinsokuKutoを追加 2002.04.17 MIK
	STypeConfigに、m_bKinsokuKutoを追加 2002.04.17 MIK

	Version 31:
	Commonに、m_bStopsBothEndsWhenSearchParagraphを追加 2002/04/26 YAZAKI

	Version 32:
	CommonからSTypeConfigへ、m_bAutoIndent、m_bAutoIndent_ZENSPACEを移動 2002/04/30 YAZAKI

	Version 33:
	Commonに、m_lf_kh(後にCommonSetting_Helper::m_lf)を追加 2002/05/21 ai
	m_nDiffFlgOptを追加 2002.05.27 MIK
	STypeConfig-ColorにCOLORIDX_DIFF_APPEND,COLORIDX_DIFF_CHANGE,COLORIDX_DIFF_DELETEを追加

	Version 34:
	STypeConfigにm_bUseDocumentIcon 追加． 2002.09.10 genta
	
	Version 35:
	Commonにm_nLineNumRightSpace 追加．2002.09.18 genta

	Version 36:
	Commonのm_bGrepKanjiCode_AutoDetectを削除、m_nGrepCharSetを追加 2002/09/21 Moca

	Version 37:
	STypeConfigのLineComment関連をm_cLineCommentに変更．  @@@ 2002.09.23 YAZAKI
	STypeConfigのBlockComment関連をm_cBlockCommentsに変更．@@@ 2002.09.23 YAZAKI

	Version 38:
	STypeConfigにm_bIndentLayoutを追加. @@@ 2002.09.29 YAZAKI
	2002.10.01 genta m_nIndentLayoutに名前変更

	Version 39:
	Commonにm_nFUNCKEYWND_GroupNumを追加． 2002/11/04 Moca

	Version 40:
	ファイル名簡易表記関連を追加． 2002/12/08～2003/01/15 Moca

	Version 41:
	STypeConfigのm_szTabViewStringサイズ拡張
	m_nWindowSizeX/Y m_nWindowOriginX/Y追加 2003.01.26 aroka

	Version 42:
	STypeConfigに独自TABマークフラグ追加 2003.03.28 MIK

	Version 43:
	最近使ったファイル・フォルダにお気に入りを追加 2003.04.08 MIK

	Version 44:
	Window Caption文字列領域をCommonに追加 2003.04.05 genta

	Version 45:
	タグファイル作成用コマンドオプション保存領域(m_nTagsOpt,m_szTagsCmdLine)を追加 2003.05.12 MIK

	Version 46:
	編集ウインドウ数修正、タブウインドウ用情報追加

	Version 47:
	ファイルからの補完をSTypeConfigに追加 2003.06.28 Moca

	Version 48:
	Grepリアルタイム表示追加 2003.06.28 Moca

	Version 49:
	ファイル情報にIsDebug追加 (タブ表示用) 2003.10.13 MIK
	
	Version 50:
	ウィンドウ位置固定・継承を追加 2004.05.13 Moca

	Version 51:
	タグジャンプ機能追加 2004/06/21 novice

	Version 52:
	前回と異なる文字コードのときに問い合わせを行うかどうかのフラグ追加	2004.10.03 genta

	Version 53:
	存在しないファイルを開こうとした場合に警告するフラグの追加	2004.10.09 genta

	Version 54:
	マウスサイドボタン対応 2004/10/10 novice

	Version 55:
	マウス中ボタン対応 2004/10/11 novice

	Version 56:
	インクリメンタルサーチ(Migemo path用) 2004/10/13 isearch
	
	Version 57:
	強調キーワード指定拡大 2005/01/13 MIK
	
	Version 58:
	強調キーワードセット可変長割り当て 2005/01/25 Moca

	Version 59:
	マクロ数を増やした 2005/01/30 genta
	
	Version 60:
	キーワード指定タグジャンプ履歴保存 2005/04/03 MIK

	Version 61:
	改行で行末の空白を削除するオプション(タイプ別設定) 2005/10/11 ryoji

	Version 62:
	バックアップフォルダ 2005.11.07 aroka

	Version 63:
	指定桁縦線表示追加 2005.11.08 Moca

	Version 64:
	タブの機能拡張(等幅、アイコン表示) 2006/01/28 ryoji
	アウトプットウィンドウ位置 2006.02.01 aroka

	Version 65:
	タブ一覧をソートする 2006/05/10 ryoji

	Version 66:
	キーワードヘルプ機能拡張 2006.04.10 fon
		キーワードヘルプ機能設定を別タブに移動, 辞書の複数化に対応, キャレット位置キーワード検索追加

	Version 67:
	キャレット色指定を追加 2006.12.07 ryoji

	Version 68:
	ファイルダイアログのフィルタ設定 2006.11.16 ryoji

	Version 69:
	「すべて置換」は置換の繰返し 2006.11.16 ryoji

	Version 70:
	[すべて閉じる]で他に編集用のウィンドウがあれば確認する 2006.12.25 ryoji

	Version 71:
	タブを閉じる動作を制御するオプション2つを追加 2007.02.11 genta
		m_bRetainEmptyTab, m_bCloseOneWinInTabMode

	Version 72:
	タブ一覧をフルパス表示する 2007.02.28 ryoji

	Version 73:
	編集ウィンドウ切替中 2007.04.03 ryoji

	Version 74:
	カーソル位置の文字列をデフォルトの検索文字列にする 2006.08.23 ryoji

	Version 75:
	マウスホイールでウィンドウ切り替え 2006.03.26 ryoji

	Version 76:
	タブのグループ化 2007.06.20 ryoji

	Version 77:
	iniフォルダ設定 2007.05.31 ryoji

	Version 78:
	エディタ－トレイ間でのUI特権分離確認のためのバージョン合わせ 2007.06.07 ryoji

	Version 79:
	外部コマンド実行のオプション拡張 2007.04.29 maru

	Version 80:
	正規表現ライブラリの切り替え 2007.07.22 genta

	Version 81:
	マウスクリックにてアクティベートされた時はカーソル位置を移動しない 2007.10.02 nasukoji

	Version 82:
	ラインモード貼り付けを可能にする 2007.10.08 ryoji

	Version 83:
	選択なしでコピーを可能にする 2007.11.18 ryoji

	Version 84:
	画面キャッシュを使う 2007.09.09 Moca

	Version 85:
	キーワードセット最大値増加 2007.12.01 genta

	Version 86:
	タイプ別設定最大値増加 2007.12.13 ryoji

	Version 87:
	テキストの折り返し方法追加 2008.05.30 nasukoji

	Version 88:
	マウスの中ボタン押下中のホイールスクロールではページスクロールする 2008.10.07 nasukoji

	Version 89:
	ホイールスクロールを利用したページスクロール・横スクロール対応（Ver.88追加部分は削除） 2009.01.17 nasukoji

	Version 90:
	STypeConfigのm_szTabViewStringサイズ拡張（Version 41）の戻し 2009.02.11 ryoji

	Version 91
	「開こうとしたファイルが大きい場合に警告」機能追加  2009.05.24

	Version 92
	デフォルト文字コードとCESU-8判別オプション対応		2009.06.15

	Version 93
	選択文字数カウント単位オプション対応				2009.06.25

	Version 94
	句読点ぶら下げ文字 2009.08.07 ryoji

	Version 95
	ウィンドウ毎にアクセラレータテーブルを作成する(Wine用) 2009.08.15 nasukoji

	Version 96:
	自動実行マクロ 2006/08/31 ryoji

	Version 97:
	高DPI用にフォントサイズ（1/10ポイント単位）を追加 2009/10/01 ryoji

	Version 98:
	WSHプラグイン 2010/03/03 syat

	Version 99:
	Backup Removable Media 対応  2010/5/27 Uchi

	Version 100:
	ICONをすべてのコマンドに割り当てる	2010/6/12 Uchi

	Version 101:
	メインニュー設定	2010/5/18 Uchi

	Version 102:
	Pluginのiniファイルへの書き込みの文字列化	2010/7/15 Uchi

	Version 103:
	アウトライン解析画面のドッキング機能追加 2010.06.05 ryoji

	Version 104:
	デフォルトの改行コードとBOM 2011.01.24 ryoji

	Version 105:
	背景画像表示 2010.09.17 Moca

	Version 106:
	上書き禁止検出時は編集禁止にする 2011.02.19 ryoji

	Version 107:
	選択範囲色指定 2011.05.18 Moca

	Version 108:
	MRUにタイプ別設定追加 2011.06.13 Moca

	Version 109:
	単語検索で複数検索 2011.11.15 ds14050, syat

	Version 110:
	改行コードを変換して貼り付ける 2011.11.22 salarm, Moca
	
	Version 111:
	タブバーフォント指定 2011.12.04 Moca
	
	Version 112:
	入力補完プラグイン 2011.06.24 Moca

	Version 113:
	マクロ停止ダイアログ 2011.08.04 syat

	Version 114:
	CCodeMediator/CESIのCEditDoc依存の修正 2012.10.12 Moca

	Version 115:
	検索・置換キーワードの文字列長制限撤廃/他のビューの検索条件を引き継ぐ 2011.12.18 Moca

	Version 116:
	MRU除外リスト 2012.10.30 Moca

	Version 117:
	自動読込時遅延 2012.11.20 Uchi

	Version 118:
	正規表現キーワード可変長 2012.12.01 Moca

	Version 119:
	CommonSetting_KeyBind のメンバ変数追加 2012.11.25 aroka

	Version 120:
	右端で折返す時のぶら下げを隠すを追加 2012.11.30 Uchi

	Version 121:
	CommonSetting_KeyBind キーの名前 2012.12.11 aroka

	Version 122:
	強調キーワードの補完 2012.10.13 Moca

	Version 123:
	ファイルダイアログの初期位置 2012.11.23 Moca

	Version 124:
	無題番号 Moca2013.01.14

	Version 125:
	文字幅に合わせてスペースを詰める 2012.10.24 Moca

	Version 126:
	カーソル行の背景色 2012.11.22 Moca

	Version 127:
	ダイアログサイズ記憶 2013.03.27 Moca

	Version 128:
	フォント幅キャッシュ整理 2013.04.05 aroka

	Version 129:
	「終了時、改行の一致を検査する」を追加 2013.04.14 Uchi

	Version 130:
	アウトラインの降順ソート

	Version 131:
	カラー印刷

	Version 132:
	印刷のヘッダ･フッタのフォント指定

	Version 133:
	外部コマンド実行のカレントディレクトリ 2013.02.22 Moca

	Version 134:
	ヒアドキュメント 2013.04.19 Moca

	Version 135:
	タブを閉じるボタン 2012.04.14 syat

	Version 136:
	無題番号の修正 2013.02.22 Moca

	Version 137:
	タイプ別フォント 2013.03.25 aroka

	Version 138:
	強調キーワード更新ロック 2013.06.10 Moca

	Version 139:
	Grep拡張 2012.07.17 Moca

	Version 140:
	TAB表示対応(文字指定/短い矢印/長い矢印)

	Version 141:
	マウススクロールキー割り当て 2013.06.12 Moca

	Version 142:
	カスタムメニューのサブメニューとして表示 2013.06.16 Moca

	Version 143:
	タブの閉じるボタン(なし/常に表示/自動表示) 2013.08.01 ryoji

	Version 144:
	STypeConfig構造体のメンバ変数型変更 2013.09.12 novice

	Version 145:
	m_bCreateAccelTblEachWin削除 2013.10.19 novice

	Version 146:
	各国語メッセージリソース対応 2013.08.02

	Version 147:
	タイプ別設定の共有データ外だし 2012.07.17 Moca

	Version 148:
	ドッキングでのブックマーク表示の同期 2013.12.23 Moca

	Version 149:
	偶数行の背景色 2013.12.30 Moca

	Version 150:
	カスタムパレット 2014.05.01 novice

	Version 151:
	DLLSHAREDATAのサイズチェック 2014.01.08 Moca

	Version 152:
	NEL等の拡張改行コードオプション 2014.06.29 Moca

	Version 153:
	ノート線 2013.12.21 Moca

	Version 154:
	タブ多段表示/タブ位置 2014.02.09 Moca

	Version 155:
	上書きモードでの矩形入力で選択範囲を削除するオプション 2014.06.02 Moca

	Version 156:
	パスの省略表記 2014.06.11 Moca

	Version 157:
	タグジャンプオプション 2014.07.10

	Version 158:
	行番号の最小桁数 2014.08.02 katze

	Version 159:
	EditInfoにBOMを追加 2013.06.24 Moca

	Version 160:
	Grep置換

	Version 161:
	ファイルツリー 2014.01.08 Moca

	Version 162:
	ミニマップ 2014.07.14 Moca

	Version 163:
	MYWM_GETLINEDATA,MYWM_SETCARETPOS,MYWM_GETCARETPOS変更 2014.05.07 Moca

	Version 164:
	矩形選択のロック 2014.07.11 Moca

	Version 165:
	メインメニュー自動更新 2015.02.26 Moca

	Version 166:
	タブバーの最大・最小値設定 2014.07.04 Moca

	Version 167:
	インデント詳細設定 2013.12.20 Moca

	Version 168:
	プロポーショナルフォント

	Version 169:
	左右リピート文字数 2014.01.08 Moca

	Version 170:
	ini読み取り専用オプション 2014.12.08 Moca

	Version 171:
	Grepファイル・フォルダ長を512(MAX_GREP_PATH)に変更

	Version 172:
	キーワードヘルプの右クリックメニュー表示選択

	-- 統合されたので元に戻す（1000～1023が使用済み） 	2008.11.16 nasukoji
	-- Version 1000:
	-- バージョン1000以降を本家統合までの間、使わせてください。かなり頻繁に構成が変更されると思われるので。by kobake 2008.03.02

*/
#define N_SHAREDATA_VERSION		172
#define STR_SHAREDATA_VERSION	NUM_TO_STR(N_SHAREDATA_VERSION)
#define	GSTR_SHAREDATA	(_T("SakuraShareData") _T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_) _T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ミューテックス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! アプリケーション実行検出用(インストーラで使用)
#define	GSTR_MUTEX_SAKURA					_T("MutexSakuraEditor")

//! コントロールプロセス
#define	GSTR_MUTEX_SAKURA_CP				(_T("MutexSakuraEditorCP")				_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//! ノーマルプロセス初期化同期
#define	GSTR_MUTEX_SAKURA_INIT				(_T("MutexSakuraEditorInit")			_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//! ノード操作同期
#define	GSTR_MUTEX_SAKURA_EDITARR			(_T("MutexSakuraEditorEditArr")			_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//DLLSHARE Work操作同期
#define	GSTR_MUTEX_SAKURA_SHAREWORK			(_T("MutexSakuraEditorShareWork")		_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//! 強調キーワードロック
#define	GSTR_MUTEX_SAKURA_KEYWORD			(_T("MutexSakuraEditorKeyword")			_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//タイプ別設定転送用
#define	GSTR_MUTEX_SAKURA_DOCTYPE			(_T("MutexSakuraEditorDocType")			_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         イベント                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! 初期化完了イベント
#define	GSTR_EVENT_SAKURA_CP_INITIALIZED	(_T("EventSakuraEditorCPInitialized")	_T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     ウィンドウクラス                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! コントロールトレイ
#define	GSTR_CEDITAPP		(_T("CControlTray") _T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)		_T(STR_SHAREDATA_VERSION))

//! メインウィンドウ
#define	GSTR_EDITWINDOWNAME	(_T("TextEditorWindow") _T(CON_SKR_MACHINE_SUFFIX_) _T(_CODE_SUFFIX_) _T(_DEBUG_SUFFIX_)	_T(STR_SHAREDATA_VERSION))

//! ビュー
#define	GSTR_VIEWNAME		(_T("SakuraView")												_T(STR_SHAREDATA_VERSION))


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         リソース                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//	Dec. 2, 2002 genta
//	固定ファイル名
#define FN_APP_ICON  _T("my_appicon.ico")
#define FN_GREP_ICON _T("my_grepicon.ico")
#define FN_TOOL_BMP  _T("my_icons.bmp")

//	標準アプリケーションアイコンリソース名
#define ICON_DEFAULT_APP IDI_ICON_STD
#define ICON_DEFAULT_GREP IDI_ICON_GREP


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      パフォーマンス                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//バッファサイズ
const int LINEREADBUFSIZE	= 10240;	//!< ファイルから1行分データを読み込むときのバッファサイズ


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
//wParam:PM_CHANGESETTING_TYPEのとき、タイプ別設定Index。それ以外なし。
#define MYWM_CHANGESETTING  (WM_APP+1520)
//! MYWM_CHANGESETTINGメッセージのlParam
enum e_PM_CHANGESETTING_SELECT {
	PM_CHANGESETTING_ALL		= 0, //!< 全部
	PM_CHANGESETTING_FONT		= 1, //!< フォント変更
	PM_CHANGESETTING_FONTSIZE	= 2, //!< フォントサイズ変更(WPARAM タイプ別番号。-1で共通設定変更)
	PM_CHANGESETTING_TYPE		= 3, //!< タイプ別設定
	PM_CHANGESETTING_TYPE2		= 4, //!< タイプ別設定(再読み込みなし)
	PM_PRINTSETTING				= 5, //!< プリント設定
};
//!座標位置情報の保存
#define MYWM_SAVEEDITSTATE  (WM_APP+1521)

//! タスクトレイからの通知メッセージ
#define MYWM_NOTIFYICON		(WM_APP+100)



/*! トレイからエディタへの終了要求 */
#define	MYWM_CLOSE			(WM_APP+200)
enum e_PM_CLOSE_WPARAM {
	PM_CLOSE_EXIT				= 1, //!< 全終了
	PM_CLOSE_GREPNOCONFIRM		= 2, //!< Grepで終了確認しない
};
#define	MYWM_ALLOWACTIVATE	(WM_APP+201)

/*! トレイからエディタへの編集ファイル情報 要求通知 */
#define	MYWM_GETFILEINFO	(WM_APP+203)

/*! カーソル位置変更通知 */
#define	MYWM_SETCARETPOS	(WM_APP+204)
//! MYWM_SETCARETPOSメッセージのlParam
enum e_PM_SETCARETPOS_SELECTSTATE {
	PM_SETCARETPOS_NOSELECT		= 0, //!< 選択解除
	PM_SETCARETPOS_SELECT		= 1, //!< 選択開始・変更
	PM_SETCARETPOS_KEEPSELECT	= 2, //!< 現在の選択状態を保って移動
};

/*! カーソル位置取得要求 */
#define	MYWM_GETCARETPOS	(WM_APP+205)

//! 未使用
//wParam:未使用
//lParam:未使用
#define	MYWM_ADDSTRING		(WM_APP+206)

/*! タグジャンプ元通知 */
#define	MYWM_SETREFERER		(WM_APP+207)

/*! 行(改行単位)データの要求(Send)
	共有データ： EditWnd:Write→呼び出し元Read
	wParam:CLogicInt ロジック行
	lParam:ClogicInt ロジック列オフセット
	@retval 0：行なし正常終了。EOF最終行
	@retval 1以上：行データあり。行データ長
	@retval -1以下：エラー
	@date 2014.05.07 仕様変更。戻り値のマイナス lParamに意味を追加
*/
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

//! テキスト追加通知(共有データ経由)
// wParam:追加する文字数(WCHAR単位)
// lParam:未使用
// @2010.05.11 Moca メッセージ追加
#define	MYWM_ADDSTRINGLEN_W (WM_APP+219)

/*! アウトライン解析画面の通知 */
#define MYWM_OUTLINE_NOTIFY (WM_APP+220)	//@@@ 2010.06.06 ryoji

//! タイプ別を設定
// wParam:タイプ別番号
#define MYWM_SET_TYPESETTING (WM_APP+221)
//! タイプ別を取得
#define MYWM_GET_TYPESETTING (WM_APP+222)

//! タイプ別を追加
#define MYWM_ADD_TYPESETTING (WM_APP+223)
//! タイプ別を削除
#define MYWM_DEL_TYPESETTING (WM_APP+224)

//! ウィンドウ一覧表示
#define MYWM_DLGWINLIST (WM_APP+225)

/* 再変換対応 */ // 20020331 aroka
#ifndef WM_IME_REQUEST
#define MYWM_IME_REQUEST 0x288  // ==WM_IME_REQUEST
#else
#define MYWM_IME_REQUEST WM_IME_REQUEST
#endif
#define MSGNAME_ATOK_RECONVERT TEXT("Atok Message for ReconvertString")
#define RWM_RECONVERT TEXT("MSIMEReconvert")

#ifndef VK_XBUTTON1
#define	VK_XBUTTON1		0x05	// マウスサイドボタン1（Windows2000以降で使用可能）	// 2009.01.17 nasukoji
#define	VK_XBUTTON2		0x06    // マウスサイドボタン2（Windows2000以降で使用可能）	// 2009.01.17 nasukoji
#endif

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif





#ifndef PBM_SETMARQUEE
#define PBM_SETMARQUEE	(WM_USER+10)
#endif

#ifndef PBS_MARQUEE
#define PBS_MARQUEE 0x08
#endif

#endif /* SAKURA_SYSTEM_CONSTANTS_608BC31D_86C2_4526_B749_70DBD090752A_H_ */
/*[EOF]*/
