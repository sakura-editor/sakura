/*	このファイルから、Funccode_define.h が生成されます。
	また Funccode_enum.h で enum 情報が生成されます．
	
	以下のマクロを用いて F_*，マクロ関数情報等を定義する．
		DEF_COMMAND( id, val )
			F_* と実際のコマンドコードを定義する
		
		DEF_MACROCMD( id, val, name, param0, param1, param2, param3, ret, ext )
			キーやメニューに割り当てることができるコマンド用に，
			ロ関数の情報を定義する
		
		DEF_MACROCMD2( id, name, param0, param1, param2, param3, ret, ext )
			DEF_MACROCMD とほぼ同じだが，ReadOnly と ViewMode 等
			同一コマンド番号をもつマクロ関数が複数ある場合，
			2個目以降のマクロ関数はこちらで定義する．
		
		DEF_MACROFUNC( id, val, name, param0, param1, param2, param3, ret, ext )
			キーやメニューに割り当てることができないマクロ専用の関数用に，
			関数の情報を定義する
	
		DEF_MACROFUNC2( id, name, param0, param1, param2, param3, ret, ext )
			DEF_MACROFUNC とほぼ同じだが，GetSelectColmFromと GetSelectColumnFrom 等
			同一コマンド番号をもつマクロ関数が複数ある場合，
			2個目以降のマクロ関数はこちらで定義する．
		
	パラメータの説明
		id:	F_FILENEW 等の，コマンド番号のシンボルを指定する
		
		val:
			エディタ内部で使われるコマンド番号を定義する．16bitの数値．
			Windows 95では32768以上のコマンドをメニューやアクセラレータに設定すると動作しない．
			20000-21999 : プラグインコマンド機能番号（20個×100）
			30000-32767 : 機能番号．メニューやキーに割り当てられる．
			32768-39999 : メニューからは直接呼ばれないが、他のIDから間接的に呼ばれる機能
			40000-49511 : マクロ関数
			49512-      : 変換コマンド．状況に応じて機能を置き換える場合に用いる
			HandleCommandの引数としては32bit幅だが，上位16bitはコマンドが送られた状況を
			通知するために使用する．
			[参考]
			10000- : ウィンドウ一覧で使用する
			11000- : 最近使ったファイルで使用する
			12000- : 最近使ったフォルダで使用する
		
		name: マクロで呼び出されるときの関数名．L"Hoge" などと指定する．
		
		param0 ～ param3, ret, ext:
			それぞれ MacroFuncInfo クラスの
			m_varArguments[0～3], m_varResult, m_pData を定義する．
*/

//aliases
DEF_COMMAND(	F_INVALID,				-1 )
DEF_COMMAND(	F_DEFAULT,				0 )		
DEF_COMMAND(	F_0,					0 )		

// 未定義用(ダミーとしても使う) 	//Oct. 17, 2000 jepro noted
DEF_COMMAND(	F_DISABLE,				0 )		 	//未使用
DEF_COMMAND(	F_NODE,					0 )		 	//ノード(Main Menu用)
DEF_COMMAND(	F_SEPARATOR,			1 )		 	//セパレータ
DEF_COMMAND(	F_TOOLBARWRAP,			2 )		 	//ツールバー折返
DEF_COMMAND(	F_MACRO_EXTRA,			2 )		 	//外部マクロ（50番以上）
DEF_COMMAND(	F_PLUGCOMMAND,			3 )		 	//プラグイン
DEF_COMMAND(	F_DUMMY_MAX_CODE,		10 )	 	//特別10である必要はないけど 10まで予約

//	2007.07.07 genta 状況通知フラグ
//	コマンドが送られた状況をコマンドと併せて通知する．
DEF_COMMAND(	FA_FROMKEYBOARD,		0x00010000 ) 	//!< キーボードアクセラレータからのコマンド
DEF_COMMAND(	FA_FROMMACRO,			0x00020000 ) 	//!< マクロからのコマンド実行
DEF_COMMAND(	FA_NONRECORD,			0x00040000 ) 	//!< マクロへの記録を抑制する

DEF_COMMAND(	F_PLUGCOMMAND_FIRST,	20000 )	
DEF_COMMAND(	F_PLUGCOMMAND_LAST,		21999 )	

// Main Menu 特殊機能
DEF_COMMAND(	F_WINDOW_LIST,			29001 )	 	// ウィンドウリスト
DEF_COMMAND(	F_FILE_USED_RECENTLY,	29002 )	 	// 最近使ったファイル
DEF_COMMAND(	F_FOLDER_USED_RECENTLY,	29003 )	 	// 最近使ったフォルダ
DEF_COMMAND(	F_CUSTMENU_LIST,		29004 )	 	// カスタムメニューリスト
DEF_COMMAND(	F_USERMACRO_LIST,		29005 )	 	// 登録済みマクロリスト
DEF_COMMAND(	F_PLUGIN_LIST,			29006 )	 	// プラグインコマンドリスト
DEF_COMMAND(	F_SPECIAL_FIRST,		F_WINDOW_LIST )
DEF_COMMAND(	F_SPECIAL_LAST,			F_PLUGIN_LIST )

DEF_COMMAND(	F_MENU_FIRST,			30000 )	

// ファイル操作系
//											[意味]								[引数]
DEF_MACROCMD(	F_FILENEW,				30101,	L"FileNew",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//新規作成							なし
DEF_COMMAND(	F_FILEOPEN,				30102 )	 	//開く								const WCHAR* path
DEF_MACROCMD(	F_FILESAVE,				30103,	L"FileSave",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//上書き保存						なし
DEF_MACROCMD(	F_FILESAVEAS_DIALOG,	30104,	L"FileSaveAsDialog",	VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY,	VT_EMPTY,	NULL )		//名前を付けて保存					なし
DEF_MACROCMD(	F_FILESAVEAS,			30106,	L"FileSaveAs",			VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY,	VT_EMPTY,	NULL )		//名前を付けて保存					const WCHAR* path
DEF_MACROCMD(	F_FILECLOSE,			30105,	L"FileClose",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//閉じて(無題)						なし
DEF_MACROCMD(	F_FILECLOSE_OPEN,		30107,	L"FileCloseOpen",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//閉じて開く						なし
DEF_COMMAND(	F_FILEOPEN_DROPDOWN,	30108 )	 	//開く(ドロップダウン)				const WCHAR* path
DEF_COMMAND(	F_FILESAVECLOSE,		30109 )	 	//保存して閉じる					なし
DEF_COMMAND(	F_FILENEW_NEWWINDOW,	30110 )	 	//新規作成（新しいウインドウで開く）		
DEF_MACROCMD(	F_FILE_REOPEN_SJIS,		30111,	L"FileReopenSJIS",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//SJISで開き直す					bool bNoConfirm
DEF_MACROCMD(	F_FILE_REOPEN_JIS,		30112,	L"FileReopenJIS",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//JISで開き直す						bool bNoConfirm
DEF_MACROCMD(	F_FILE_REOPEN_EUC,		30113,	L"FileReopenEUC",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//EUCで開き直す						bool bNoConfirm
DEF_MACROCMD(	F_FILE_REOPEN_UNICODE,	30114,	L"FileReopenUNICODE",	VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//Unicodeで開き直す					bool bNoConfirm
DEF_MACROCMD(	F_FILE_REOPEN_UTF8,		30115,	L"FileReopenUTF8",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//UTF-8で開き直す					bool bNoConfirm
DEF_MACROCMD(	F_FILE_REOPEN_UTF7,		30116,	L"FileReopenUTF7",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//UTF-7で開き直す					bool bNoConfirm
DEF_MACROCMD(	F_FILE_REOPEN_UNICODEBE,30117,	L"FileReopenUNICODEBE",	VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//UnicodeBEで開き直す				bool bNoConfirm
DEF_MACROCMD(	F_FILE_REOPEN_CESU8,	30118,	L"FileReopenCESU8",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//CESU-8で開き直す					bool bNoConform
DEF_MACROCMD(	F_FILE_REOPEN,			30119,	L"FileReopen",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//現在と同じ文字コードで開き直す	bool bNoConfirm
DEF_MACROCMD(	F_FILESAVEALL,			30120,	L"FileSaveAll",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//すべて上書き保存					なし
DEF_COMMAND(	F_FILESAVE_QUIET,		30121 )	 	//上書き保存(限定動作)				なし
DEF_MACROCMD(	F_FILE_REOPEN_LATIN1,	30122,	L"FileReopenLatin1",	VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//Latin1で開き直す					bool bNoConform	// 2010/3/20 Uchi
DEF_MACROCMD(	F_PRINT,				30150,	L"Print",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//印刷								なし
DEF_MACROCMD(	F_PRINT_PREVIEW,		30151,	L"PrintPreview",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//印刷プレビュー					なし
DEF_MACROCMD(	F_PRINT_PAGESETUP,		30152,	L"PrintPageSetup",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//印刷ページ設定					なし
//F_PRINT_DIALOG			= 30151,	//印刷ダイアログ					？
//F_OPEN_HHPP					= 30160,	//同名のC/C++ヘッダファイルを開く	bool bCheckOnly	// del	2008/6/23 Uchi
//F_OPEN_CCPP					= 30161,	//同名のC/C++ソースファイルを開く	bool bCheckOnly	// del	2008/6/23 Uchi
DEF_MACROCMD(	F_OPEN_HfromtoC,		30162,	L"OpenHfromtoC",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//同名のC/C++ヘッダ(ソース)を開く	bool bCheckOnly
DEF_MACROCMD(	F_ACTIVATE_SQLPLUS,		30170,	L"ActivateSQLPLUS",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//Oracle SQL*Plusをアクティブ表示	なし
DEF_MACROCMD(	F_PLSQL_COMPILE_ON_SQLPLUS,30171,	L"ExecSQLPLUS",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//Oracle SQL*Plusで実行				なし
DEF_MACROCMD(	F_BROWSE,				30180,	L"Browse",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ブラウズ							なし
DEF_MACROCMD(	F_VIEWMODE,				30185,	L"ReadOnly",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ビューモード						なし
DEF_MACROCMD2( 	F_VIEWMODE,						L"ViewMode",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ビューモード						なし
DEF_MACROCMD(	F_PROPERTY_FILE,		30190,	L"PropertyFile",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ファイルのプロパティ				なし
DEF_COMMAND(	F_PROFILEMGR,			30191 )	 	//プロファイルマネージャ			なし
DEF_MACROCMD(	F_EXITALLEDITORS,		30194,	L"ExitAllEditors",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//編集の全終了						なし
DEF_MACROCMD(	F_EXITALL,				30195,	L"ExitAll",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//サクラエディタの全終了			なし

// 編集系
DEF_MACROCMD(	F_WCHAR,				30200,	L"Char",				VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//文字入力									WCHAR c
DEF_MACROCMD(	F_IME_CHAR,				30201,	L"CharIme",				VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//全角文字入力								WORD c
DEF_MACROCMD(	F_UNDO,					30210,	L"Undo",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//元に戻す(Undo)							なし
DEF_MACROCMD(	F_REDO,					30211,	L"Redo",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//やり直し(Redo)							なし
DEF_MACROCMD(	F_DELETE,				30221,	L"Delete",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//削除										なし
DEF_MACROCMD(	F_DELETE_BACK,			30222,	L"DeleteBack",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//カーソル前を削除							なし
DEF_MACROCMD(	F_WordDeleteToStart,	30230,	L"WordDeleteToStart",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//単語の左端まで削除						なし
DEF_MACROCMD(	F_WordDeleteToEnd,		30231,	L"WordDeleteToEnd",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//単語の右端まで削除						なし
DEF_MACROCMD(	F_WordCut,				30232,	L"WordCut",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//単語切り取り								なし
DEF_MACROCMD(	F_WordDelete,			30233,	L"WordDelete",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//単語削除									なし
DEF_MACROCMD(	F_LineCutToStart,		30240,	L"LineCutToStart",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//行頭まで切り取り(改行単位)				なし
DEF_MACROCMD(	F_LineCutToEnd,			30241,	L"LineCutToEnd",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//行末まで切り取り(改行単位)				なし
DEF_MACROCMD(	F_LineDeleteToStart,	30242,	L"LineDeleteToStart",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//行頭まで削除(改行単位)					なし
DEF_MACROCMD(	F_LineDeleteToEnd,		30243,	L"LineDeleteToEnd",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//行末まで削除(改行単位)					なし
DEF_MACROCMD(	F_CUT_LINE,				30244,	L"CutLine",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//行切り取り(折り返し単位)					なし
DEF_MACROCMD(	F_DELETE_LINE,			30245,	L"DeleteLine",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//行削除(折り返し単位)						なし
DEF_MACROCMD(	F_DUPLICATELINE,		30250,	L"DuplicateLine",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//行の二重化(折り返し単位)					なし
DEF_MACROCMD(	F_INDENT_TAB,			30260,	L"IndentTab",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//TABインデント								なし
DEF_MACROCMD(	F_UNINDENT_TAB,			30261,	L"UnindentTab",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//逆TABインデント							なし
DEF_MACROCMD(	F_INDENT_SPACE,			30262,	L"IndentSpace",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//SPACEインデント							なし
DEF_MACROCMD(	F_UNINDENT_SPACE,		30263,	L"UnindentSpace",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//逆SPACEインデント							なし
//F_WORDSREFERENCE	= 30270,	//単語リファレンス							なし？
DEF_MACROCMD(	F_LTRIM,				30280,	L"LTrim",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//左(先頭)の空白を削除						なし
DEF_MACROCMD(	F_RTRIM,				30281,	L"RTrim",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//右(末尾)の空白を削除						なし
DEF_MACROCMD(	F_SORT_ASC,				30282,	L"SortAsc",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//選択行の昇順ソート						なし
DEF_MACROCMD(	F_SORT_DESC,			30283,	L"SortDesc",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//選択行の降順ソート						なし
DEF_MACROCMD(	F_MERGE,				30284,	L"Merge",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//選択行のマージ							なし
DEF_COMMAND(	F_RECONVERT,			30285 )	 	//再変換									なし


// カーソル移動系
DEF_MACROCMD(	F_UP,					30311,	L"Up",					VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//カーソル上移動							なし
DEF_MACROCMD(	F_DOWN,					30312,	L"Down",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//カーソル下移動							なし
DEF_MACROCMD(	F_LEFT,					30313,	L"Left",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//カーソル左移動							なし
DEF_MACROCMD(	F_RIGHT,				30314,	L"Right",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//カーソル右移動							なし
DEF_MACROCMD(	F_UP2,					30315,	L"Up2",					VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//カーソル上移動(２行ごと)					なし
DEF_MACROCMD(	F_DOWN2,				30316,	L"Down2",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//カーソル下移動(２行ごと)					なし
DEF_MACROCMD(	F_WORDLEFT,				30320,	L"WordLeft",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//単語の左端に移動							なし
DEF_MACROCMD(	F_WORDRIGHT,			30321,	L"WordRight",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//単語の右端に移動							なし
//F_GOLINETOP		= 30330,	//行頭に移動(改行単位)						？
//F_GOLINEEND		= 30331,	//行末に移動(改行単位)						？
DEF_MACROCMD(	F_GOLINETOP,			30332,	L"GoLineTop",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//行頭に移動(折り返し単位)					int param
DEF_MACROCMD(	F_GOLINEEND,			30333,	L"GoLineEnd",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//行末に移動(折り返し単位)					なし
//F_ROLLDOWN		= 30340,	//スクロールダウン							なし？
//F_ROLLUP			= 30341,	//スクロールアップ							なし？
DEF_MACROCMD(	F_HalfPageUp,			30340,	L"HalfPageUp",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//半ページアップ							なし
DEF_MACROCMD(	F_HalfPageDown,			30341,	L"HalfPageDown",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//半ページダウン							なし
DEF_MACROCMD(	F_1PageUp,				30342,	L"1PageUp",				VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//１ページアップ							なし
DEF_MACROCMD2( 	F_1PageUp,						L"PageUp",				VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//１ページアップ							なし
DEF_MACROCMD(	F_1PageDown,			30343,	L"1PageDown",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//１ページダウン							なし
DEF_MACROCMD2( 	F_1PageDown,					L"PageDown",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//１ページダウン							なし
//F_DISPLAYTOP		= 30344,	//画面の先頭に移動(未実装)					？
//F_DISPLAYEND		= 30345,	//画面の最後に移動(未実装)					？
DEF_MACROCMD(	F_GOFILETOP,			30350,	L"GoFileTop",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ファイルの先頭に移動						なし
DEF_MACROCMD(	F_GOFILEEND,			30351,	L"GoFileEnd",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ファイルの最後に移動						なし
DEF_MACROCMD(	F_CURLINECENTER,		30360,	L"CurLineCenter",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//カーソル行をウィンドウ中央へ				なし
DEF_MACROCMD(	F_JUMPHIST_PREV,		30370,	L"MoveHistPrev",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//移動履歴: 前へ							なし
DEF_MACROCMD(	F_JUMPHIST_NEXT,		30371,	L"MoveHistNext",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//移動履歴: 次へ							なし
DEF_MACROCMD(	F_JUMPHIST_SET,			30372,	L"MoveHistSet",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//現在位置を移動履歴に登録					なし
DEF_MACROCMD(	F_WndScrollDown,		30380,	L"F_WndScrollDown",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//テキストを１行下へスクロール				なし
DEF_MACROCMD(	F_WndScrollUp,			30381,	L"F_WndScrollUp",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//テキストを１行上へスクロール				なし
DEF_MACROCMD(	F_GONEXTPARAGRAPH,		30382,	L"GoNextParagraph",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//次の段落へ								なし
DEF_MACROCMD(	F_GOPREVPARAGRAPH,		30383,	L"GoPrevParagraph",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//前の段落へ								なし
DEF_COMMAND(	F_AUTOSCROLL,			30384 )	 	//オートスクロール
DEF_COMMAND(	F_WHEEL_FIRST,			30385 )	
DEF_MACROCMD(	F_WHEELUP,				30385,	L"WheelUp",				VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ホイールアップ								int zDelta
DEF_MACROCMD(	F_WHEELDOWN,			30386,	L"WheelDown",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ホイールダウン								int zDelta
DEF_MACROCMD(	F_WHEELLEFT,			30387,	L"WheelLeft",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ホイール左								int zDelta
DEF_MACROCMD(	F_WHEELRIGHT,			30388,	L"WheelRight",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ホイール右								int zDelta
DEF_MACROCMD(	F_WHEELPAGEUP,			30389,	L"WheelPageUp",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ホイールページアップ						int zDelta
DEF_MACROCMD(	F_WHEELPAGEDOWN,		30390,	L"WheelPageDown",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ホイールページダウン						int zDelta
DEF_MACROCMD(	F_WHEELPAGELEFT,		30391,	L"WheelPageLeft",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ホイールページ左							int zDelta
DEF_MACROCMD(	F_WHEELPAGERIGHT,		30392,	L"WheelPageRight",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ホイールページ右							int zDelta
DEF_COMMAND(	F_WHEEL_LAST,			30392 )	
DEF_MACROCMD(	F_MODIFYLINE_NEXT,		30393,	L"GoModifyLineNext",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//次の変更行へ								なし
DEF_MACROCMD(	F_MODIFYLINE_PREV,		30394,	L"GoModifyLinePrev",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//前の変更行へ								なし

// 選択系
DEF_MACROCMD(	F_SELECTWORD,			30400,	L"SelectWord",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//現在位置の単語選択						なし
DEF_MACROCMD(	F_SELECTALL,			30401,	L"SelectAll",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//すべて選択								なし
DEF_MACROCMD(	F_SELECTLINE,			30403,	L"SelectLine",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//1行選択									int param	// 2007.10.06 nasukoji
DEF_MACROCMD(	F_BEGIN_SEL,			30410,	L"BeginSelect",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//範囲選択開始								なし
DEF_MACROCMD(	F_UP_SEL,				30411,	L"Up_Sel",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)カーソル上移動					int lines
DEF_MACROCMD(	F_DOWN_SEL,				30412,	L"Down_Sel",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)カーソル下移動					なし
DEF_MACROCMD(	F_LEFT_SEL,				30413,	L"Left_Sel",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)カーソル左移動					なし
DEF_MACROCMD(	F_RIGHT_SEL,			30414,	L"Right_Sel",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)カーソル右移動					なし
DEF_MACROCMD(	F_UP2_SEL,				30415,	L"Up2_Sel",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)カーソル上移動(２行ごと)		なし
DEF_MACROCMD(	F_DOWN2_SEL,			30416,	L"Down2_Sel",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)カーソル下移動(２行ごと)		なし
DEF_MACROCMD(	F_WORDLEFT_SEL,			30420,	L"WordLeft_Sel",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)単語の左端に移動				なし
DEF_MACROCMD(	F_WORDRIGHT_SEL,		30421,	L"WordRight_Sel",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)単語の右端に移動				なし
//F_GOLINETOP_SEL	= 30430,	//(範囲選択)行頭に移動(改行単位)			？
//F_GOLINEEND_SEL	= 30431,	//(範囲選択)行末に移動(改行単位)			？
DEF_MACROCMD(	F_GOLINETOP_SEL,		30432,	L"GoLineTop_Sel",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)行頭に移動(折り返し単位)		なし
DEF_MACROCMD(	F_GOLINEEND_SEL,		30433,	L"GoLineEnd_Sel",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)行末に移動(折り返し単位)		なし
//F_ROLLDOWN_SEL	= 30440,	//(範囲選択)スクロールダウン				なし？
//F_ROLLUP_SEL		= 30441,	//(範囲選択)スクロールアップ				なし？
DEF_MACROCMD(	F_HalfPageUp_Sel,		30440,	L"HalfPageUp_Sel",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)半ページアップ					なし
DEF_MACROCMD(	F_HalfPageDown_Sel,		30441,	L"HalfPageDown_Sel",	VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)半ページダウン					なし
DEF_MACROCMD(	F_1PageUp_Sel,			30442,	L"1PageUp_Sel",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)１ページアップ					なし
DEF_MACROCMD2( 	F_1PageUp_Sel,					L"PageUp_Sel",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)１ページアップ					なし
DEF_MACROCMD(	F_1PageDown_Sel,		30443,	L"1PageDown_Sel",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)１ページダウン					なし
DEF_MACROCMD2( 	F_1PageDown_Sel,				L"PageDown_Sel",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)１ページダウン					なし
//F_DISPLAYTOP_SEL	= 30444,	//(範囲選択)画面の先頭に移動(未実装)		？
//F_DISPLAYEND_SEL	= 30445,	//(範囲選択)画面の最後に移動(未実装)		？
DEF_MACROCMD(	F_GOFILETOP_SEL,		30450,	L"GoFileTop_Sel",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)ファイルの先頭に移動			なし
DEF_MACROCMD(	F_GOFILEEND_SEL,		30451,	L"GoFileEnd_Sel",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)ファイルの最後に移動			なし
DEF_MACROCMD(	F_GONEXTPARAGRAPH_SEL,	30482,	L"GoNextParagraph_Sel",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)次の段落へ					なし
DEF_MACROCMD(	F_GOPREVPARAGRAPH_SEL,	30483,	L"GoPrevParagraph_Sel",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)前の段落へ					なし
DEF_MACROCMD(	F_MODIFYLINE_NEXT_SEL,	30484,	L"GoModifyLineNext_Sel",VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)次の変更行へ					なし
DEF_MACROCMD(	F_MODIFYLINE_PREV_SEL,	30485,	L"GoModifyLinePrev_Sel",VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(範囲選択)前の変更行へ					なし


// 矩形選択系
//F_BOXSELALL			= 30500,	//矩形ですべて選択
DEF_MACROCMD(	F_BEGIN_BOX,			30510,	L"BeginBoxSelect",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//矩形範囲選択開始						なし
//Oct. 17, 2000 JEPRO 以下に矩形選択のコマンド名のみ準備しておいた
DEF_MACROCMD(	F_UP_BOX,				30511,	L"Up_BoxSel",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)カーソル上移動
DEF_MACROCMD(	F_DOWN_BOX,				30512,	L"Down_BoxSel",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)カーソル下移動
DEF_MACROCMD(	F_LEFT_BOX,				30513,	L"Left_BoxSel",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)カーソル左移動
DEF_MACROCMD(	F_RIGHT_BOX,			30514,	L"Right_BoxSel",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)カーソル右移動
DEF_MACROCMD(	F_UP2_BOX,				30515,	L"Up2_BoxSel",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)カーソル上移動(２行ごと)
DEF_MACROCMD(	F_DOWN2_BOX,			30516,	L"Down2_BoxSel",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)カーソル下移動(２行ごと)
DEF_MACROCMD(	F_WORDLEFT_BOX,			30520,	L"WordLeft_BoxSel",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)単語の左端に移動
DEF_MACROCMD(	F_WORDRIGHT_BOX,		30521,	L"WordRight_BoxSel",	VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)単語の右端に移動
DEF_MACROCMD(	F_GOLOGICALLINETOP_BOX,	30530,	L"GoLogicalLineTop_BoxSel",VT_I4,   VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)行頭に移動(改行単位)
//F_GOLOGICALLINEEND_BOX	= 30531,	//(矩形選択)行末に移動(改行単位)
DEF_MACROCMD(	F_GOLINETOP_BOX,		30532,	L"GoLineTop_BoxSel",	VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)行頭に移動(折り返し単位)
DEF_MACROCMD(	F_GOLINEEND_BOX,		30533,	L"GoLineEnd_BoxSel",	VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)行末に移動(折り返し単位)
DEF_MACROCMD(	F_HalfPageUp_BOX,		30540,	L"HalfPageUp_BoxSel",	VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)半ページアップ
DEF_MACROCMD(	F_HalfPageDown_BOX,		30541,	L"HalfPageDown_BoxSel",	VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)半ページダウン
DEF_MACROCMD(	F_1PageUp_BOX,			30542,	L"1PageUp_BoxSel",		VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)１ページアップ
DEF_MACROCMD2( 	F_1PageUp_BOX,					L"PageUp_BoxSel",		VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)１ページアップ
DEF_MACROCMD(	F_1PageDown_BOX,		30543,	L"1PageDown_BoxSel",	VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)１ページダウン
DEF_MACROCMD2( 	F_1PageDown_BOX,				L"PageDown_BoxSel",		VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)１ページダウン
//F_DISPLAYTOP_BOX		= 30444,	//(矩形選択)画面の先頭に移動(未実装)
//F_DISPLAYEND_BOX		= 30445,	//(矩形選択)画面の最後に移動(未実装)
DEF_MACROCMD(	F_GOFILETOP_BOX,		30550,	L"GoFileTop_BoxSel",	VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)ファイルの先頭に移動
DEF_MACROCMD(	F_GOFILEEND_BOX,		30551,	L"GoFileEnd_BoxSel",	VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//(矩形選択)ファイルの最後に移動


// クリップボード系
DEF_MACROCMD(	F_CUT,					30601,	L"Cut",					VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//切り取り(選択範囲をクリップボードにコピーして削除)	なし
DEF_MACROCMD(	F_COPY,					30602,	L"Copy",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//コピー(選択範囲をクリップボードにコピー)				なし
DEF_MACROCMD(	F_COPY_ADDCRLF,			30608,	L"CopyAddCRLF",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//折り返し位置に改行をつけてコピー						なし
DEF_MACROCMD(	F_COPY_CRLF,			30603,	L"CopyCRLF",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//CRLF改行でコピー										なし
DEF_MACROCMD(	F_PASTE,				30604,	L"Paste",				VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//貼り付け(クリップボードから貼り付け)					なし
DEF_MACROCMD(	F_PASTEBOX,				30605,	L"PasteBox",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//矩形貼り付け(クリップボードから矩形貼り付け)			なし

//2007.09.18 kobake WCHARを求めるメッセージのメッセージ名を変更: 「*」→「*_W」
DEF_MACROCMD(	F_INSTEXT_W,			30606,	L"InsText",				VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//テキストを貼り付け							const WCHAR* text, bool bNoWaitCursor
DEF_MACROCMD(	F_ADDTAIL_W,			30607,	L"AddTail",				VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//最後にテキストを追加							const WCHAR* text, int length
DEF_MACROCMD(	F_COPYLINES,			30610,	L"CopyLines",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//選択範囲内全行コピー							なし
DEF_MACROCMD(	F_COPYLINESASPASSAGE,	30611,	L"CopyLinesAsPassage",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//選択範囲内全行引用符付きコピー				なし
DEF_MACROCMD(	F_COPYLINESWITHLINENUMBER,30612,	L"CopyLinesWithLineNumber",VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//選択範囲内全行行番号付きコピー				なし
DEF_MACROCMD(	F_COPY_COLOR_HTML,		30613,	L"CopyColorHtml",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//選択範囲内色付きHTMLコピー					なし
DEF_MACROCMD(	F_COPY_COLOR_HTML_LINENUMBER,30614,	L"CopyColorHtmlWithLineNumber",VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//選択範囲内行番号色付きHTMLコピー		なし
DEF_MACROCMD(	F_COPYPATH,				30620,	L"CopyPath",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//このファイルのパス名をクリップボードにコピー	なし
DEF_MACROCMD(	F_COPYTAG,				30621,	L"CopyTag",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//このファイルのパス名とカーソル位置をコピー	なし
DEF_MACROCMD(	F_COPYFNAME,			30622,	L"CopyFilename",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//このファイル名をクリップボードにコピー		なし
DEF_MACROCMD(	F_CREATEKEYBINDLIST,	30630,	L"CopyKeyBindList",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//キー割り当て一覧をコピー						なし


// 挿入系
DEF_MACROCMD(	F_INS_DATE,				30790,	L"InsertDate",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//日付挿入										なし
DEF_MACROCMD(	F_INS_TIME,				30791,	L"InsertTime",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//時刻挿入										なし
DEF_MACROCMD(	F_CTRL_CODE_DIALOG,		30792,	L"CtrlCodeDialog",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//コントロールコードの入力(ダイアログ)			なし
DEF_MACROCMD(	F_CTRL_CODE,			30793,	L"CtrlCode",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//コントロールコードの入力						wchar_t code


// 変換系
DEF_MACROCMD(	F_TOLOWER,				30800,	L"ToLower",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//小文字							なし
DEF_MACROCMD(	F_TOUPPER,				30801,	L"ToUpper",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//大文字							なし
DEF_MACROCMD(	F_TOHANKAKU,			30810,	L"ToHankaku",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//全角→半角 						なし
DEF_MACROCMD(	F_TOHANKATA,			30817,	L"ToHankata",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//全角カタカナ→半角カタカナ		なし
DEF_MACROCMD(	F_TOZENKAKUKATA,		30811,	L"ToZenKata",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//半角＋全ひら→全角・カタカナ		なし
DEF_MACROCMD(	F_TOZENKAKUHIRA,		30812,	L"ToZenHira",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//半角＋全カタ→全角・ひらがな		なし
DEF_MACROCMD(	F_HANKATATOZENKATA,		30813,	L"HanKataToZenKata",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//半角カタカナ→全角カタカナ		なし
DEF_MACROCMD(	F_HANKATATOZENHIRA,		30814,	L"HanKataToZenHira",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//半角カタカナ→全角ひらがな		なし
DEF_MACROCMD(	F_TOZENEI,				30815,	L"ToZenEi",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//半角英数→全角英数				なし
DEF_MACROCMD(	F_TOHANEI,				30816,	L"ToHanEi",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//全角英数→半角英数				なし
DEF_MACROCMD(	F_TABTOSPACE,			30830,	L"TABToSPACE",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//TAB→空白 						なし
DEF_MACROCMD(	F_SPACETOTAB,			30831,	L"SPACEToTAB",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//空白→TAB							なし
DEF_MACROCMD(	F_CODECNV_AUTO2SJIS,	30850,	L"AutoToSJIS",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//自動判別→SJISコード変換 			なし
DEF_MACROCMD(	F_CODECNV_EMAIL,		30851,	L"JIStoSJIS",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//E-Mail(JIS→SJIS)コード変換		なし
DEF_MACROCMD(	F_CODECNV_EUC2SJIS,		30852,	L"EUCtoSJIS",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//EUC→SJISコード変換				なし
DEF_MACROCMD(	F_CODECNV_UNICODE2SJIS,	30853,	L"CodeCnvUNICODEtoSJIS",VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//Unicode→SJISコード変換			なし
DEF_MACROCMD(	F_CODECNV_UNICODEBE2SJIS,30856,	L"CodeCnvUNICODEBEtoSJIS",VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//UnicodeBE→SJISコード変換			なし
DEF_MACROCMD(	F_CODECNV_UTF82SJIS,	30854,	L"UTF8toSJIS",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//UTF-8→SJISコード変換 			なし
DEF_MACROCMD(	F_CODECNV_UTF72SJIS,	30855,	L"UTF7toSJIS",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//UTF-7→SJISコード変換 			なし
DEF_MACROCMD(	F_CODECNV_SJIS2JIS,		30860,	L"SJIStoJIS",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//SJIS→JISコード変換 				なし
DEF_MACROCMD(	F_CODECNV_SJIS2EUC,		30861,	L"SJIStoEUC",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//SJIS→EUCコード変換 				なし
DEF_MACROCMD(	F_CODECNV_SJIS2UTF8,	30862,	L"SJIStoUTF8",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//SJIS→UTF-8コード変換 			なし
DEF_MACROCMD(	F_CODECNV_SJIS2UTF7,	30863,	L"SJIStoUTF7",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//SJIS→UTF-7コード変換 			なし
DEF_MACROCMD(	F_BASE64DECODE,			30870,	L"Base64Decode",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//Base64デコードして保存			なし
DEF_MACROCMD(	F_UUDECODE,				30880,	L"Uudecode",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//uudecodeして保存					なし


// 検索系 
DEF_MACROCMD(	F_SEARCH_DIALOG,		30901,	L"SearchDialog",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//検索(単語検索ダイアログ)						なし
DEF_MACROCMD(	F_SEARCH_NEXT,			30902,	L"SearchNext",			VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//次を検索										HWND hwnd, const WCHAR* text
DEF_MACROCMD(	F_SEARCH_PREV,			30903,	L"SearchPrev",			VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//前を検索										HWND hwnd
DEF_MACROCMD(	F_REPLACE_DIALOG,		30904,	L"ReplaceDialog",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//置換(置換ダイアログ)							なし
DEF_MACROCMD(	F_REPLACE,				30906,	L"Replace",				VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY,	VT_EMPTY,	NULL )		//置換(実行)									HWND hwnd
DEF_MACROCMD(	F_REPLACE_ALL,			30907,	L"ReplaceAll",			VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY,	VT_EMPTY,	NULL )		//すべて置換(実行)								なし
DEF_MACROCMD(	F_SEARCH_CLEARMARK,		30905,	L"SearchClearMark",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//検索マークの切替え							なし
DEF_MACROCMD(	F_JUMP_SRCHSTARTPOS,	30909,	L"SearchStartPos",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//検索開始位置へ戻る							なし
DEF_COMMAND(	F_SEARCH_BOX,			30908 )	 	//検索(ボックス)								なし
DEF_COMMAND(	F_GREP_DIALOG,			30910 )	 	//Grep											なし
DEF_MACROCMD(	F_GREP,					30911,	L"Grep",				VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_I4   ,	VT_EMPTY,	&s_MacroInfoEx_i )		//Grep											なし
DEF_COMMAND(	F_GREP_REPLACE_DLG,		30912 )	 	//Grep置換										なし
DEF_MACROCMD(	F_GREP_REPLACE,			30913,	L"GrepReplace",			VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_BSTR ,	VT_EMPTY,	&s_MacroInfoEx_ii )		//Grep置換										なし
DEF_COMMAND(	F_JUMP_DIALOG,			30920 )	 	//指定行へジャンプ								なし
DEF_MACROCMD(	F_JUMP,					30921,	L"Jump",				VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//指定行へジャンプ								なし
DEF_MACROCMD(	F_OUTLINE,				30930,	L"Outline",				VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//アウトライン解析								int nAction
DEF_COMMAND(	F_OUTLINE_TOGGLE,		30931 )	 	//アウトライン解析(toggle)						なし
DEF_MACROCMD(	F_TAGJUMP,				30940,	L"TagJump",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//タグジャンプ									bool bClose
DEF_MACROCMD(	F_TAGJUMPBACK,			30941,	L"TagJumpBack",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//タグジャンプバック							なし
DEF_COMMAND(	F_TAGJUMP_LIST,			30942 )	 	//タグジャンプリスト							？
DEF_MACROCMD(	F_TAGS_MAKE,			30943,	L"TagMake",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//タグファイルの作成							なし
DEF_MACROCMD(	F_DIRECT_TAGJUMP,		30944,	L"DirectTagJump",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ダイレクトタグジャンプ						なし
DEF_COMMAND(	F_TAGJUMP_CLOSE,		30945 )	 	//閉じてタグジャンプ(元ウィンドウclose)			なし
DEF_MACROCMD(	F_TAGJUMP_KEYWORD,		30946,	L"KeywordTagJump",		VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//キーワードを指定してダイレクトタグジャンプ	const WCHAR* keyword
DEF_MACROCMD(	F_COMPARE,				30950,	L"Compare",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ファイル内容比較								なし
DEF_MACROCMD(	F_BRACKETPAIR,			30960,	L"BracketPair",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//対括弧の検索									なし
DEF_MACROCMD(	F_BOOKMARK_SET,			30970,	L"BookmarkSet",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ブックマーク設定・解除						なし
DEF_MACROCMD(	F_BOOKMARK_NEXT,		30971,	L"BookmarkNext",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//次のブックマークへ							なし
DEF_MACROCMD(	F_BOOKMARK_PREV,		30972,	L"BookmarkPrev",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//前のブックマークへ							なし
DEF_MACROCMD(	F_BOOKMARK_RESET,		30973,	L"BookmarkReset",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ブックマークの全解除							なし
DEF_MACROCMD(	F_BOOKMARK_VIEW,		30974,	L"BookmarkView",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ブックマークの一覧							int nAction
DEF_MACROCMD(	F_BOOKMARK_PATTERN,		30975,	L"BookmarkPattern",		VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//パターンに一致する行をマーク					なし
DEF_MACROCMD(	F_DIFF_DIALOG,			30976,	L"DiffDialog",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//DIFF差分表示(ダイアログ)						なし
DEF_MACROCMD(	F_DIFF,					30977,	L"Diff",				VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//DIFF差分表示									const WCHAR* szTmpFile2, int nFlgOpt
DEF_MACROCMD(	F_DIFF_NEXT,			30978,	L"DiffNext",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//次の差分へ									なし
DEF_MACROCMD(	F_DIFF_PREV,			30979,	L"DiffPrev",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//前の差分へ									なし
DEF_MACROCMD(	F_DIFF_RESET,			30980,	L"DiffReset",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//差分の全解除									なし
DEF_COMMAND(	F_ISEARCH_NEXT,			30981 )	 	//前方インクリメンタルサーチ					？
DEF_COMMAND(	F_ISEARCH_PREV,			30982 )	 	//後方インクリメンタルサーチ					？
DEF_COMMAND(	F_ISEARCH_REGEXP_NEXT,	30983 )	 	//前方正規表現インクリメンタルサーチ		？
DEF_COMMAND(	F_ISEARCH_REGEXP_PREV,	30984 )	 	//後方正規表現インクリメンタルサーチ		？
DEF_COMMAND(	F_ISEARCH_MIGEMO_NEXT,	30985 )	 	//前方MIGEMOインクリメンタルサーチ			？
DEF_COMMAND(	F_ISEARCH_MIGEMO_PREV,	30986 )	 	//後方MIGEMOインクリメンタルサーチ			？
DEF_MACROCMD(	F_FUNCLIST_NEXT,		30988,	L"FuncListNext",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//次の関数リストマークへ						なし
DEF_MACROCMD(	F_FUNCLIST_PREV,		30989,	L"FuncListPrev",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//前の関数リストマークへ						なし
DEF_COMMAND(	F_FILETREE,				30990 )	 	//ファイルツリー								なし

// モード切り替え系 
DEF_MACROCMD(	F_CHGMOD_INS,			31001,	L"ChgmodINS",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//挿入／上書きモード切り替え			なし
DEF_MACROCMD(	F_CHG_CHARSET,			31010,	L"ChgCharSet",			VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//文字コードセット指定					
DEF_COMMAND(	F_CHGMOD_EOL_CRLF,		31081 )	 	//入力改行コード指定(CRLF)				なし
DEF_COMMAND(	F_CHGMOD_EOL_LF,		31082 )	 	//入力改行コード指定(LF)				なし
DEF_COMMAND(	F_CHGMOD_EOL_CR,		31083 )	 	//入力改行コード指定(CR)				なし
DEF_MACROCMD(	F_CANCEL_MODE,			31099,	L"CancelMode",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//各種モードの取り消し					なし

// 設定系 
DEF_MACROCMD(	F_SHOWTOOLBAR,			31100,	L"ShowToolbar",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ツールバーの表示						なし
DEF_MACROCMD(	F_SHOWFUNCKEY,			31101,	L"ShowFunckey",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ファンクションキーの表示 				なし
DEF_MACROCMD(	F_SHOWSTATUSBAR,		31102,	L"ShowStatusbar",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ステータスバーの表示 					なし
DEF_MACROCMD(	F_SHOWTAB,				31103,	L"ShowTab",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//タブの表示							なし
DEF_MACROCMD(	F_SHOWMINIMAP,			31104,	L"ShowMiniMap",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ミニマップの表示						なし
DEF_MACROCMD(	F_TYPE_LIST,			31110,	L"TypeList",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//タイプ別設定一覧 						なし
DEF_MACROCMD(	F_OPTION_TYPE,			31111,	L"OptionType",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//タイプ別設定 							なし
DEF_MACROCMD(	F_OPTION,				31112,	L"OptionCommon",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//共通設定 								なし
DEF_MACROCMD(	F_FAVORITE,				31113,	L"OptionFavorite",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//履歴の管理							なし
DEF_MACROCMD(	F_FONT,					31120,	L"SelectFont",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// フォント設定							なし
DEF_MACROCMD(	F_SETFONTSIZE,			31121,	L"SetFontSize",			VT_I4,    VT_I4,    VT_I4,    VT_EMPTY,	VT_EMPTY,	NULL )		// フォントサイズ設定					int fontSize, int shift, int mode
DEF_COMMAND(	F_SETFONTSIZEUP,		31122 )	 	// フォントサイズ拡大					なし
DEF_COMMAND(	F_SETFONTSIZEDOWN,		31123 )	 	// フォントサイズ縮小					なし
DEF_MACROCMD(	F_WRAPWINDOWWIDTH,		31140,	L"WrapWindowWidth",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 現在のウィンドウ幅で折り返し			なし
DEF_COMMAND(	F_TMPWRAPNOWRAP,		31141 )	 	// 折り返さない（一時設定）				なし
DEF_COMMAND(	F_TMPWRAPSETTING,		31142 )	 	// 指定桁で折り返す（一時設定）			なし
DEF_COMMAND(	F_TMPWRAPWINDOW,		31143 )	 	// 右端で折り返す（一時設定）			なし
DEF_MACROCMD(	F_SELECT_COUNT_MODE,	31144,	L"SelectCountMode",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 文字カウントの方法を取得、設定する	int nMode

DEF_COMMAND(	F_TYPE_SCREEN,			31115 )	 	// タイプ別設定『スクリーン』			？
DEF_COMMAND(	F_TYPE_COLOR,			31116 )	 	// タイプ別設定『カラー』				？
DEF_COMMAND(	F_TYPE_WINDOW,			31114 )	 	// タイプ別設定『ウィンドウ』			？
DEF_COMMAND(	F_TYPE_HELPER,			31117 )	 	// タイプ別設定『支援』					？
DEF_COMMAND(	F_TYPE_REGEX_KEYWORD,	31118 )	 	// タイプ別設定『正規表現キーワード』	？
DEF_COMMAND(	F_TYPE_KEYHELP,			31119 )	 	// タイプ別設定『キーワードヘルプ』		？
DEF_COMMAND(	F_OPTION_GENERAL,		32000 )	 	// 共通設定『全般』 					？
DEF_COMMAND(	F_OPTION_WINDOW,		32001 )	 	// 共通設定『ウィンドウ』				？ 
DEF_COMMAND(	F_OPTION_EDIT,			32002 )	 	// 共通設定『編集』 					？
DEF_COMMAND(	F_OPTION_FILE,			32003 )	 	// 共通設定『ファイル』 				？
DEF_COMMAND(	F_OPTION_BACKUP,		32004 )	 	// 共通設定『バックアップ』				？
DEF_COMMAND(	F_OPTION_FORMAT,		32005 )	 	// 共通設定『書式』						？
//F_OPTION_URL			= 32006,	// 共通設定『クリッカブルURL』			？
DEF_COMMAND(	F_OPTION_GREP,			32007 )	 	// 共通設定『Grep』						？
DEF_COMMAND(	F_OPTION_KEYBIND,		32008 )	 	// 共通設定『キー割り当て』				？
DEF_COMMAND(	F_OPTION_CUSTMENU,		32009 )	 	// 共通設定『カスタムメニュー』			？
DEF_COMMAND(	F_OPTION_TOOLBAR,		32010 )	 	// 共通設定『ツールバー』				？
DEF_COMMAND(	F_OPTION_KEYWORD,		32011 )	 	// 共通設定『強調キーワード』			？
DEF_COMMAND(	F_OPTION_HELPER,		32012 )	 	// 共通設定『支援』						？
DEF_COMMAND(	F_OPTION_MACRO,			32013 )	 	// 共通設定『マクロ』					？
DEF_COMMAND(	F_OPTION_FNAME,			32014 )	 	// 共通設定『ファイル名表示』			？
DEF_COMMAND(	F_OPTION_TAB,			32015 )	 	// 共通設定『タブ』						？
DEF_COMMAND(	F_OPTION_STATUSBAR,		32016 )	 	// 共通設定『ステータスバー』			？
DEF_COMMAND(	F_OPTION_PLUGIN,		32017 )	 	// 共通設定『プラグイン』				？
DEF_COMMAND(	F_OPTION_MAINMENU,		32018 )	 	// 共通設定『メインメニュー』			？

// マクロ系
DEF_COMMAND(	F_RECKEYMACRO,			31250 )	 	// キーマクロの記録開始／終了		なし
DEF_COMMAND(	F_SAVEKEYMACRO,			31251 )	 	// キーマクロの保存					なし
DEF_COMMAND(	F_LOADKEYMACRO,			31252 )	 	// キーマクロの読み込み				なし
DEF_COMMAND(	F_EXECKEYMACRO,			31253 )	 	// キーマクロの実行					なし
DEF_MACROCMD(	F_EXECEXTMACRO,			31254,	L"ExecExternalMacro",	VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 名前を指定してマクロ実行			const WCHAR* command
DEF_MACROCMD(	F_EXECMD_DIALOG,		31270,	L"ExecCommandDialog",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 外部コマンド実行					const WCHAR* command
DEF_MACROCMD(	F_EXECMD,				31271,	L"ExecCommand",			VT_BSTR,  VT_I4,    VT_BSTR,  VT_EMPTY,	VT_EMPTY,	NULL )		// 外部コマンド実行					const WCHAR* command

// ウィンドウ系
DEF_MACROCMD(	F_SPLIT_V,				31310,	L"SplitWinV",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//上下に分割								なし
DEF_MACROCMD(	F_SPLIT_H,				31311,	L"SplitWinH",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//左右に分割								なし
DEF_MACROCMD(	F_SPLIT_VH,				31312,	L"SplitWinVH",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//縦横に分割								なし
DEF_MACROCMD(	F_WINCLOSE,				31320,	L"WinClose",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//閉じる									なし
DEF_MACROCMD(	F_WIN_CLOSEALL,			31321,	L"WinCloseAll",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//すべて閉じる								なし
DEF_MACROCMD(	F_CASCADE,				31330,	L"CascadeWin",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//重ねて表示								なし
DEF_MACROCMD(	F_TILE_V,				31331,	L"TileWinV",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//上下に並べて表示							なし
DEF_MACROCMD(	F_TILE_H,				31332,	L"TileWinH",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//左右に並べて表示							なし
DEF_COMMAND(	F_BIND_WINDOW,			31333 )	 	//グループ化								なし
DEF_MACROCMD(	F_TOPMOST,				31334,	L"WindowTopMost",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//常に手前に表示							int top
DEF_MACROCMD(	F_NEXTWINDOW,			31340,	L"NextWindow",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//次のウィンドウ							？
DEF_MACROCMD(	F_PREVWINDOW,			31341,	L"PrevWindow",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//前のウィンドウ							？
DEF_MACROCMD(	F_WINLIST,				31336,	L"WindowList",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//ウィンドウ一覧							なし
DEF_COMMAND(	F_DLGWINLIST,			31337 )	 	//ウィンドウ一覧ダイアログ					なし
DEF_MACROCMD(	F_MAXIMIZE_V,			31350,	L"MaximizeV",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//縦方向に最大化							なし
DEF_MACROCMD(	F_MINIMIZE_ALL,			31351,	L"MinimizeAll",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//すべて最小化								なし
DEF_MACROCMD(	F_MAXIMIZE_H,			31352,	L"MaximizeH",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//横方向に最大化							なし
DEF_MACROCMD(	F_REDRAW,				31360,	L"ReDraw",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//再描画									なし
DEF_MACROCMD(	F_WIN_OUTPUT,			31370,	L"ActivateWinOutput",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		//アウトプットウィンドウ表示				なし
DEF_MACROCMD(	F_GROUPCLOSE,			31380,	L"GroupClose",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// グループを閉じる							なし
DEF_MACROCMD(	F_NEXTGROUP,			31381,	L"NextGroup",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 次のグループ								なし
DEF_MACROCMD(	F_PREVGROUP,			31382,	L"PrevGroup",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 前のグループ								なし
DEF_MACROCMD(	F_TAB_MOVERIGHT,		31383,	L"TabMoveRight",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// タブを右に移動							なし
DEF_MACROCMD(	F_TAB_MOVELEFT,			31384,	L"TabMoveLeft",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// タブを左に移動							なし
DEF_MACROCMD(	F_TAB_SEPARATE,			31385,	L"TabSeparate",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 新規グループ								なし
DEF_MACROCMD(	F_TAB_JOINTNEXT,		31386,	L"TabJointNext",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 次のグループに移動						なし
DEF_MACROCMD(	F_TAB_JOINTPREV,		31387,	L"TabJointPrev",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 前のグループに移動						なし
DEF_MACROCMD(	F_TAB_CLOSEOTHER,		31388,	L"TabCloseOther",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// このタブ以外を閉じる						なし
DEF_MACROCMD(	F_TAB_CLOSELEFT,		31389,	L"TabCloseLeft",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 左をすべて閉じる							なし
DEF_MACROCMD(	F_TAB_CLOSERIGHT,		31390,	L"TabCloseRight",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 右をすべて閉じる							なし


// 支援
DEF_MACROCMD(	F_HOKAN,				31430,	L"Complete",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 入力補完											なし
DEF_MACROCMD(	F_TOGGLE_KEY_SEARCH,	31431,	L"ToggleKeyHelpSearch",	VT_I4, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// キーワードヘルプ自動表示							int option
DEF_MACROCMD(	F_HELP_CONTENTS,		31440,	L"HelpContents",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// ヘルプ目次										なし
DEF_MACROCMD(	F_HELP_SEARCH,			31441,	L"HelpSearch",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// ヘルプキーワード検索								なし
DEF_MACROCMD(	F_MENU_ALLFUNC,			31445,	L"CommandList",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// コマンド一覧										なし
DEF_MACROCMD(	F_EXTHELP1,				31450,	L"ExtHelp1",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 外部ヘルプ１										なし
DEF_MACROCMD(	F_EXTHTMLHELP,			31451,	L"ExtHtmlHelp",			VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 外部HTMLヘルプ									const WCHAR* helpfile, const WCHAR* keyword
DEF_MACROCMD(	F_ABOUT,				31455,	L"About",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// バージョン情報									なし


//	Jul. 4, 2000 genta
DEF_COMMAND(	F_USERMACRO_0,			31600 )	 	// 登録マクロ開始


// カスタムメニュー
DEF_MACROCMD(	F_MENU_RBUTTON,			31580,	L"RMenu",				VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 右クリックメニュー		なし
DEF_MACROCMD(	F_CUSTMENU_1,			31501,	L"CustMenu1",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー1		なし
DEF_MACROCMD(	F_CUSTMENU_2,			31502,	L"CustMenu2",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー2		なし
DEF_MACROCMD(	F_CUSTMENU_3,			31503,	L"CustMenu3",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー3		なし
DEF_MACROCMD(	F_CUSTMENU_4,			31504,	L"CustMenu4",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー4		なし
DEF_MACROCMD(	F_CUSTMENU_5,			31505,	L"CustMenu5",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー5		なし
DEF_MACROCMD(	F_CUSTMENU_6,			31506,	L"CustMenu6",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー6		なし
DEF_MACROCMD(	F_CUSTMENU_7,			31507,	L"CustMenu7",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー7		なし
DEF_MACROCMD(	F_CUSTMENU_8,			31508,	L"CustMenu8",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー8		なし
DEF_MACROCMD(	F_CUSTMENU_9,			31509,	L"CustMenu9",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー9		なし
DEF_MACROCMD(	F_CUSTMENU_10,			31510,	L"CustMenu10",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー10		なし
DEF_MACROCMD(	F_CUSTMENU_11,			31511,	L"CustMenu11",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー11		なし
DEF_MACROCMD(	F_CUSTMENU_12,			31512,	L"CustMenu12",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー12		なし
DEF_MACROCMD(	F_CUSTMENU_13,			31513,	L"CustMenu13",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー13		なし
DEF_MACROCMD(	F_CUSTMENU_14,			31514,	L"CustMenu14",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー14		なし
DEF_MACROCMD(	F_CUSTMENU_15,			31515,	L"CustMenu15",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー15		なし
DEF_MACROCMD(	F_CUSTMENU_16,			31516,	L"CustMenu16",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー16		なし
DEF_MACROCMD(	F_CUSTMENU_17,			31517,	L"CustMenu17",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー17		なし
DEF_MACROCMD(	F_CUSTMENU_18,			31518,	L"CustMenu18",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー18		なし
DEF_MACROCMD(	F_CUSTMENU_19,			31519,	L"CustMenu19",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー19		なし
DEF_MACROCMD(	F_CUSTMENU_20,			31520,	L"CustMenu20",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー20		なし
DEF_MACROCMD(	F_CUSTMENU_21,			31521,	L"CustMenu21",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー21		なし
DEF_MACROCMD(	F_CUSTMENU_22,			31522,	L"CustMenu22",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー22		なし
DEF_MACROCMD(	F_CUSTMENU_23,			31523,	L"CustMenu23",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー23		なし
DEF_MACROCMD(	F_CUSTMENU_24,			31524,	L"CustMenu24",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// カスタムメニュー24		なし
DEF_COMMAND(	F_CUSTMENU_LAST,		F_CUSTMENU_24 )

DEF_COMMAND(	F_CUSTMENU_BASE,		31500 )	 	// カスタムメニュー基準番号

// その他
//F_SENDMAIL		= 31570,		// メール送信	//Oct. 17, 2000 JEPRO メール機能は死んでいるのでコメントアウトにした

//	Windows 95の制約により，WM_COMMANDで使う機能番号として32768以上の値を用いることができません．
//	メニュー等で使われないマクロ専用のコマンドにはそれ以上の値を割り当てましょう．

//	メニューからは直接呼ばれないが、他のIDから間接的に呼ばれる機能
DEF_COMMAND(	F_MENU_NOT_USED_FIRST,	32768 )	
DEF_MACROCMD(	F_CHGMOD_EOL,			32800,	L"ChgmodEOL",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 入力改行コード指定						enumEOLType e
DEF_MACROCMD(	F_SET_QUOTESTRING,		32801,	L"SetMsgQuoteStr",		VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 共通設定: 引用符の設定					const WCHAR* quotestr
DEF_MACROCMD(	F_TRACEOUT,				32802,	L"TraceOut",			VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// マクロ用アウトプットウインドウに出力		const WCHAR* outputstr, int nFlgOpt
DEF_MACROCMD(	F_PUTFILE,				32803,	L"PutFile",				VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY,   VT_EMPTY,	NULL )		// 作業中ファイルの一時出力 2006.12.10 maru
DEF_MACROCMD(	F_INSFILE,				32804,	L"InsFile",				VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY,   VT_EMPTY,	NULL )		// キャレット位置にファイル挿入 2006.12.10 maru
DEF_MACROCMD(	F_TEXTWRAPMETHOD,		32805,	L"TextWrapMethod",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// テキストの折り返し方法					なし
DEF_MACROCMD(	F_INSBOXTEXT,			32806,	L"InsBoxText",			VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// 矩形テキスト挿入
DEF_MACROCMD(	F_MOVECURSOR,			32807,	L"MoveCursor",			VT_I4,    VT_I4,    VT_I4,    VT_EMPTY,	VT_EMPTY,	NULL )		// カーソル移動
DEF_MACROCMD(	F_MOVECURSORLAYOUT,		32808,	L"MoveCursorLayout",	VT_I4,    VT_I4,    VT_I4,    VT_EMPTY,	VT_EMPTY,	NULL )		// カーソル移動(レイアウト)
DEF_MACROCMD(	F_STATUSMSG,			32809,	L"StatusMsg",			VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// ステータスメッセージ
DEF_MACROCMD(	F_MSGBEEP,				32810,	L"MsgBeep",				VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// ビープ音
DEF_MACROCMD(	F_CHANGETYPE,			32811,	L"ChangeType",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// タイプ別設定一時適用						int nTypeIndex
DEF_MACROCMD(	F_FILEOPEN2,			32812,	L"FileOpen",			VT_BSTR,  VT_I4,    VT_I4,    VT_BSTR,	VT_EMPTY,	NULL )		// 開く2									const WCHAR* filename, ECodeType nCharCode, bool bViewMode, const WCHAR* defaultName
DEF_MACROCMD(	F_COMMITUNDOBUFFER,		32813,	L"CommitUndoBuffer",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL  )		// OpeBlKコミット
DEF_MACROCMD(	F_ADDREFUNDOBUFFER,		32814,	L"AddRefUndoBuffer",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL  )		// OpeBlK AddRef
DEF_MACROCMD(	F_SETUNDOBUFFER,		32815,	L"SetUndoBuffer",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL  )		// OpeBlK Release
DEF_MACROCMD(	F_APPENDUNDOBUFFERCURSOR,32816,	L"AppendUndoBufferCursor",VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL  )		// OpeBlK にカーソル位置を追加
DEF_MACROCMD(	F_CLIPBOARDEMPTY,		32817,	L"ClipboardEmpty",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// クリップボードクリア
DEF_MACROCMD(	F_SETVIEWTOP,			32818,	L"SetViewTop",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// ビューの上の行数を設定					CLayoutYInt nLineNumber
DEF_MACROCMD(	F_SETVIEWLEFT,			32819,	L"SetViewLeft",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_EMPTY,	NULL )		// ビューの左端の桁数を設定					CLayoutXInt nColumnIndex


DEF_COMMAND(	F_FUNCTION_FIRST,		40000 )	 	// 2003-02-21 鬼 これ以上だと関数

DEF_MACROFUNC(	F_GETFILENAME,			40001,	L"GetFilename",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// 編集中のファイル名を取得する
DEF_MACROFUNC(	F_GETSAVEFILENAME,		40018,	L"GetSaveFilename",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// 保存時のファイル名を取得する	// 2006.09.04 ryoji
DEF_MACROFUNC(	F_GETSELECTED,			40002,	L"GetSelectedString",	VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// 選択範囲の取得
DEF_MACROFUNC(	F_EXPANDPARAMETER,		40003,	L"ExpandParameter",		VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// コマンドラインパラメータ展開
DEF_MACROFUNC(	F_GETLINESTR,			40004,	L"GetLineStr",			VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// 指定行論理データを取得する
DEF_MACROFUNC(	F_GETLINECOUNT,			40005,	L"GetLineCount",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 論理行数を取得する
DEF_MACROFUNC(	F_CHGTABWIDTH,			40006,	L"ChangeTabWidth",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// タブサイズを取得、設定する
DEF_MACROFUNC(	F_ISTEXTSELECTED,		40007,	L"IsTextSelected",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// テキストが選択されているか
DEF_MACROFUNC(	F_GETSELLINEFROM,		40008,	L"GetSelectLineFrom",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 選択開始行の取得
DEF_MACROFUNC(	F_GETSELCOLUMNFROM,		40009,	L"GetSelectColumnFrom",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 選択開始桁の取得
DEF_MACROFUNC2( F_GETSELCOLUMNFROM,				L"GetSelectColmFrom",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 選択開始桁の取得
DEF_MACROFUNC(	F_GETSELLINETO,			40010,	L"GetSelectLineTo",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 選択終了行の取得
DEF_MACROFUNC(	F_GETSELCOLUMNTO,		40011,	L"GetSelectColumnTo",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 選択終了桁の取得
DEF_MACROFUNC2(	F_GETSELCOLUMNTO,				L"GetSelectColmTo",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 選択終了桁の取得
DEF_MACROFUNC(	F_ISINSMODE,			40012,	L"IsInsMode",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 挿入／上書きモードの取得
DEF_MACROFUNC(	F_GETCHARCODE,			40013,	L"GetCharCode",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 文字コード取得
DEF_MACROFUNC(	F_GETLINECODE,			40014,	L"GetLineCode",			VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 改行コード取得
DEF_MACROFUNC(	F_ISPOSSIBLEUNDO,		40015,	L"IsPossibleUndo",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// Undo可能か調べる
DEF_MACROFUNC(	F_ISPOSSIBLEREDO,		40016,	L"IsPossibleRedo",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// Redo可能か調べる
DEF_MACROFUNC(	F_CHGWRAPCOLUMN,		40017,	L"ChangeWrapColumn",	VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 折り返し桁を取得、設定する 2008.06.19 ryoji
DEF_MACROFUNC2(	F_CHGWRAPCOLUMN,				L"ChangeWrapColm",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 折り返し桁を取得、設定する 2008.06.19 ryoji
DEF_MACROFUNC(	F_ISCURTYPEEXT,			40019,	L"IsCurTypeExt",		VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 指定した拡張子が現在のタイプ別設定に含まれているかどうかを調べる 2006.09.04 ryoji
DEF_MACROFUNC(	F_ISSAMETYPEEXT,		40020,	L"IsSameTypeExt",		VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// ２つの拡張子が同じタイプ別設定に含まれているかどうかを調べる 2006.09.04 ryoji
DEF_MACROFUNC(	F_INPUTBOX,				40021,	L"InputBox",			VT_BSTR,  VT_BSTR,  VT_I4,    VT_EMPTY,	VT_BSTR,	NULL  )		// テキスト入力ダイアログの表示
DEF_MACROFUNC(	F_MESSAGEBOX,			40023,	L"MessageBox",			VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// メッセージボックスの表示
DEF_MACROFUNC(	F_ERRORMSG,				40024,	L"ErrorMsg",			VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// メッセージボックス（エラー）の表示
DEF_MACROFUNC(	F_WARNMSG,				40025,	L"WarnMsg",				VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// メッセージボックス（警告）の表示
DEF_MACROFUNC(	F_INFOMSG,				40026,	L"InfoMsg",				VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// メッセージボックス（情報）の表示
DEF_MACROFUNC(	F_OKCANCELBOX,			40027,	L"OkCancelBox",			VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// メッセージボックス（確認：OK／キャンセル）の表示
DEF_MACROFUNC(	F_YESNOBOX,				40028,	L"YesNoBox",			VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// メッセージボックス（確認：はい／いいえ）の表示
DEF_MACROFUNC(	F_COMPAREVERSION,		40029,	L"CompareVersion",		VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// バージョン番号の比較
DEF_MACROFUNC(	F_MACROSLEEP,			40030,	L"Sleep",				VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 指定した時間（ミリ秒）停止
DEF_MACROFUNC(	F_FILEOPENDIALOG,		40031,	L"FileOpenDialog",		VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// ファイルを開くダイアログの表示
DEF_MACROFUNC(	F_FILESAVEDIALOG,		40032,	L"FileSaveDialog",		VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// ファイルを保存ダイアログの表示
DEF_MACROFUNC(	F_FOLDERDIALOG,			40033,	L"FolderDialog",		VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// フォルダを開くダイアログの表示
DEF_MACROFUNC(	F_GETCLIPBOARD,			40034,	L"GetClipboard",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// クリップボードの文字列を取得
DEF_MACROFUNC(	F_SETCLIPBOARD,			40035,	L"SetClipboard",		VT_I4,    VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// クリップボードに文字列を設定
DEF_MACROFUNC(	F_LAYOUTTOLOGICLINENUM,	40036,	L"LayoutToLogicLineNum",VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// ロジック行番号取得
DEF_MACROFUNC(	F_LOGICTOLAYOUTLINENUM,	40037,	L"LogicToLayoutLineNum",VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// レイアウト行番号取得
DEF_MACROFUNC(	F_LINECOLUMNTOINDEX,	40038,	L"LineColumnToIndex",	VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// ロジック桁番号取得
DEF_MACROFUNC(	F_LINEINDEXTOCOLUMN,	40039,	L"LineIndexToColumn",	VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// レイアウト桁番号取得
DEF_MACROFUNC(	F_GETCOOKIE,			40040,	L"GetCookie",			VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// Cookie取得
DEF_MACROFUNC(	F_GETCOOKIEDEFAULT,		40041,	L"GetCookieDefault",	VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_EMPTY,	VT_BSTR,	NULL  )		// Cookie取得デフォルト値
DEF_MACROFUNC(	F_SETCOOKIE,			40042,	L"SetCookie",			VT_BSTR,  VT_BSTR,  VT_BSTR,  VT_EMPTY,	VT_I4,		NULL  )		// Cookie設定
DEF_MACROFUNC(	F_DELETECOOKIE,			40043,	L"DeleteCookie",		VT_BSTR,  VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// Cookie削除
DEF_MACROFUNC(	F_GETCOOKIENAMES,		40044,	L"GetCookieNames",		VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_BSTR,	NULL  )		// Cookie名前取得
DEF_MACROFUNC(	F_SETDRAWSWITCH,		40045,	L"SetDrawSwitch",		VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 再描画スイッチ設定
DEF_MACROFUNC(	F_GETDRAWSWITCH,		40046,	L"GetDrawSwitch",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 再描画スイッチ取得
DEF_MACROFUNC(	F_ISSHOWNSTATUS,		40047,	L"IsShownStatus",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// ステータスウィンドウが表示されているか
DEF_MACROFUNC(	F_GETSTRWIDTH,			40048,	L"GetStrWidth",			VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 文字列幅取得
DEF_MACROFUNC(	F_GETSTRLAYOUTLENGTH,	40049,	L"GetStrLayoutLength",	VT_BSTR,  VT_I4,    VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 文字列のレイアウト幅取得
DEF_MACROFUNC(	F_GETDEFAULTCHARLENGTH,	40050,	L"GetDefaultCharLength",VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// デフォルト文字幅の取得
DEF_MACROFUNC(	F_ISINCLUDECLIPBOARDFORMAT,40051,	L"IsIncludeClipboardFormat",VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// クリップボードの形式取得
DEF_MACROFUNC(	F_GETCLIPBOARDBYFORMAT,	40052,	L"GetClipboardByFormat",VT_BSTR,  VT_I4,    VT_I4,    VT_EMPTY,	VT_BSTR,	NULL  )		// クリップボードの指定形式で取得
DEF_MACROFUNC(	F_SETCLIPBOARDBYFORMAT,	40053,	L"SetClipboardByFormat",VT_BSTR,  VT_BSTR,  VT_I4,    VT_I4,    VT_I4,		NULL  )		// クリップボードの指定形式で設定
DEF_MACROFUNC(	F_GETLINEATTRIBUTE,		40054,	L"GetLineAttribute",	VT_I4,    VT_I4,    VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 行属性取得
DEF_MACROFUNC(	F_ISTEXTSELECTINGLOCK,	40055,	L"IsTextSelectingLock",	VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// 選択状態のロックを取得
DEF_MACROFUNC(	F_GETVIEWLINES,			40056,	L"GetViewLines",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// ビューの行数取得
DEF_MACROFUNC(	F_GETVIEWCOLUMNS,		40057,	L"GetViewColumns",		VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// ビューの列数取得
DEF_MACROFUNC(	F_CREATEMENU,			40058,	L"CreateMenu",			VT_I4,    VT_BSTR,  VT_EMPTY, VT_EMPTY,	VT_I4,		NULL  )		// メニュー作成


//	= 2005,.01.10 genta ISearch用補助コード
//	2007.07.07 genta 16bit以内に収めないと状況コードと衝突するのでコードを変更
DEF_COMMAND(	F_ISEARCH_ADD_CHAR,		0xC001 ) 	//	Incremental Searchへ1文字へ追加
DEF_COMMAND(	F_ISEARCH_ADD_STR,		0xC002 ) 	//	Incremental Searchへ文字列へ追加
DEF_COMMAND(	F_ISEARCH_DEL_BACK,		0xC003 ) 	//	Incremental Searchの末尾から1文字削除

#undef DEF_COMMAND
#undef DEF_MACROCMD
#undef DEF_MACROCMD2
#undef DEF_MACROFUNC
#undef DEF_MACROFUNC2
