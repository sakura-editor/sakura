/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_MAXDATA_CEFC5993_30DF_4A7C_981A_512B8CE6FD979_H_
#define SAKURA_MAXDATA_CEFC5993_30DF_4A7C_981A_512B8CE6FD979_H_

/*! 最大値定義
	@date 2007.10.19 kobake 新規作成
*/
enum maxdata{
	MAX_EDITWINDOWS				= 256,	//!< 編集ウィンドウ数
	MAX_SEARCHKEY				=  30,	//!< 検索キー
	MAX_REPLACEKEY				=  30,	//!< 置換キー
	MAX_GREPFILE				=  30,	//!< Grepファイル
	MAX_GREPFOLDER				=  30,	//!< Grepフォルダ
	MAX_GREP_PATH				= 512,	//!< Grepファイルパス長
	MAX_TYPES					=  60,	//!< タイプ別設定
	MAX_TYPES_NAME				=  64,	//!< タイプ属性：名称 バッファサイズ
	MAX_TYPES_EXTS				=  64,	//!< タイプ属性：拡張子リスト バッファサイズ
	MAX_PRINTSETTINGARR			=   8,	//!< 印刷ページ設定

	//	From Here Sep. 14, 2001 genta
	MACRONAME_MAX				= 64,
	MAX_EXTCMDLEN				= 1024,
	MAX_EXTCMDMRUNUM			= 32,

	MAX_CMDLEN					= 1024,
	MAX_CMDARR					= 32,
	MAX_REGEX_KEYWORD			= 100,	//@@@ 2001.11.17 add MIK
	MAX_REGEX_KEYWORDLEN		= 1000,
	MAX_REGEX_KEYWORDLISTLEN	= MAX_REGEX_KEYWORD * 100 + 1,

	MAX_KEYHELP_FILE			= 20,	//@@@ 2006.04.10 fon

	MAX_MARKLINES_LEN			= 1023,	// 2002.01.18 hor
	MAX_DOCTYPE_LEN				= 7,
	MAX_TRANSFORM_FILENAME		= 16,	/// 2002.11.24 Moca

	/*! 登録できるマクロの数
		@date 2005.01.30 genta 50に増やした
	*/
	MAX_CUSTMACRO				= 50,
	MAX_CUSTMACRO_ICO			= 50,	// アイコンに専用位置を割り当てている数	// 2010.06.25 Moca 追加

	// 2004/06/21 novice タグジャンプ機能追加
	MAX_TAGJUMPNUM				= 100,	// タブジャンプ情報最大値
	MAX_TAGJUMP_KEYWORD			= 30,	//タグジャンプ用キーワード最大登録数2005.04.04 MIK
	MAX_KEYWORDSET_PER_TYPE		= 10,	// 2004.01.23 genta (for MIK) タイプ別設定毎のキーワードセット数
	MAX_VERTLINES = 10,	// 2005.11.08 Moca 指定桁縦線

	//	MRUリストに関係するmaxdata
	MAX_MRU						=  36,	//Sept. 27, 2000 JEPRO 0-9, A-Z で36個になるのでそれに合わせて30→36に変更。2007.10.23 kobake maxdataに移動。
	MAX_OPENFOLDER				=  36,	//Sept. 27, 2000 JEPRO 0-9, A-Z で36個になるのでそれに合わせて30→36に変更

	MAX_PLUGIN					= 40,	// 登録できるプラグインの数
	MAX_PLUG_CMD				= 50,	// 登録できるプラグイン コマンドの数+1(1 origin分)
	MAX_PLUG_OPTION				= 100,	// 登録できるプラグインオプションの数
	MAX_PLUGIN_ID				= 63+1,	// プラグインIDの最大長さ
	MAX_PLUGIN_NAME				= 63+1,	// プラグイン名の最大長さ
	MAX_PLUG_STRING				= 100,	// 登録できるプラグイン文字列の数

	// MainMenu
	MAX_MAINMENU				= 500,	// 登録できるメインメニューの数
	MAX_MAINMENU_TOP			= 20,	// 登録できるメインメニューの数
	MAX_MAIN_MENU_NAME_LEN		= 40,	// メインメニュー名文字列長
};

#endif /* SAKURA_MAXDATA_CEFC5993_30DF_4A7C_981A_512B8CE6FD979_H_ */
/*[EOF]*/
