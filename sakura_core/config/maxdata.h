//2007.10.19 kobake

#pragma once

enum maxdata{
	MAX_EDITWINDOWS				= 256,
	MAX_SEARCHKEY				=  30,
	MAX_REPLACEKEY				=  30,
	MAX_GREPFILE				=  30,
	MAX_GREPFOLDER				=  30,
	MAX_TYPES					=  20,	//Jul. 12, 2001 JEPRO タイプ別設定の最大設定数を16から増やした
	MAX_PRINTSETTINGARR			=   8,

	//	From Here Sep. 14, 2001 genta
	MACRONAME_MAX				= 64,
	MAX_EXTCMDLEN				= 1024,
	MAX_EXTCMDMRUNUM			= 32,

	MAX_CMDLEN					= 1024,
	MAX_CMDARR					= 32,
	MAX_REGEX_KEYWORD			= 100,	//@@@ 2001.11.17 add MIK

	MAX_KEYHELP_FILE			= 20,	//@@@ 2006.04.10 fon

	MAX_MARKLINES_LEN			= 1023,	// 2002.01.18 hor
	MAX_DOCTYPE_LEN				= 7,
	MAX_TRANSFORM_FILENAME		= 16,	/// 2002.11.24 Moca

	/*! 登録できるマクロの数
		@date 2005.01.30 genta 50に増やした
	*/
	MAX_CUSTMACRO				= 50,

	// 2004/06/21 novice タグジャンプ機能追加
	MAX_TAGJUMPNUM				= 100,	// タブジャンプ情報最大値
	MAX_TAGJUMP_KEYWORD			= 30,	//タグジャンプ用キーワード最大登録数2005.04.04 MIK
	MAX_KEYWORDSET_PER_TYPE		= 10,	// 2004.01.23 genta (for MIK) タイプ別設定毎のキーワードセット数
	MAX_VERTLINES = 10,	// 2005.11.08 Moca 指定桁縦線

	//	MRUリストに関係するmaxdata
	MAX_MRU						=  36,	//Sept. 27, 2000 JEPRO 0-9, A-Z で36個になるのでそれに合わせて30→36に変更。2007.10.23 kobake maxdataに移動。
};




