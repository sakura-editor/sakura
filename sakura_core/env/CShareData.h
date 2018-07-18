/*!	@file
	@brief プロセス間共有データへのアクセス

	@author Norio Nakatani
	@date 1998/05/26  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro, genta
	Copyright (C) 2001, jepro, genta, asa-o, MIK, YAZAKI, hor
	Copyright (C) 2002, genta, aroka, Moca, MIK, YAZAKI, hor
	Copyright (C) 2003, Moca, aroka, MIK, genta
	Copyright (C) 2004, Moca, novice, genta
	Copyright (C) 2005, MIK, genta, ryoji, aroka, Moca
	Copyright (C) 2006, aroka, ryoji, D.S.Koba, fon
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, Uchi
	Copyright (C) 2011, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
//2007.09.23 kobake m_nSEARCHKEYArrNum,      m_szSEARCHKEYArr      を m_aSearchKeys      にまとめました
//2007.09.23 kobake m_nREPLACEKEYArrNum,     m_szREPLACEKEYArr     を m_aReplaceKeys     にまとめました
//2007.09.23 kobake m_nGREPFILEArrNum,       m_szGREPFILEArr       を m_aGrepFiles       にまとめました
//2007.09.23 kobake m_nGREPFOLDERArrNum,     m_szGREPFOLDERArr     を m_aGrepFolders     にまとめました
//2007.09.23 kobake m_szCmdArr,              m_nCmdArrNum          を m_aCommands        にまとめました
//2007.09.23 kobake m_nTagJumpKeywordArrNum, m_szTagJumpKeywordArr を m_aTagJumpKeywords にまとめました
//2007.12.13 kobake DLLSHAREDATAへの簡易アクセサを用意


#ifndef SAKURA_ENV_CSHAREDATA_H_
#define SAKURA_ENV_CSHAREDATA_H_

#include "CSelectLang.h"		// 2011.04.10 nasukoji

class CShareData;

// 2010.04.19 Moca DLLSHAREDATA関連はDLLSHAREDATA.h等最低限必要な場所へ移動
// CShareData.hは、自分のInterfaceしか提供しません。別にDLLSHAREDATA.hをincludeすること。
struct DLLSHAREDATA;
struct STypeConfig;
class CMutex;

/*!	@brief 共有データの管理

	CShareDataはCProcessのメンバであるため，両者の寿命は同一です．
	本来はCProcessオブジェクトを通じてアクセスするべきですが，
	CProcess内のデータ領域へのポインタをstatic変数に保存することで
	Singletonのようにどこからでもアクセスできる構造になっています．

	共有メモリへのポインタをm_pShareDataに保持します．このメンバは
	公開されていますが，CShareDataによってMap/Unmapされるために
	ChareDataの消滅によってポインタm_pShareDataも無効になることに
	注意してください．

	@date 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
*/
class CShareData : public TSingleton<CShareData>
{
	friend class TSingleton<CShareData>;
	CShareData();
	~CShareData();

public:
	/*
	||  Attributes & Operations
	*/
	bool InitShareData();	/* CShareDataクラスの初期化処理 */
	void RefreshString();	/* 言語選択後に共有メモリ内の文字列を更新する */
	
	//MRU系
	BOOL IsPathOpened( const TCHAR* pszPath, HWND* phwndOwner ); /* 指定ファイルが開かれているか調べる */
	BOOL ActiveAlreadyOpenedWindow( const TCHAR* pszPath, HWND* phwndOwner, ECodeType nCharCode );/* 指定ファイルが開かれているか調べつつ、多重オープン時の文字コード衝突も確認 */	// 2007.03.16

	//デバッグ  今は主にマクロ・外部コマンド実行用
	void TraceOut( LPCTSTR lpFmt, ...);	/* アウトプットウィンドウに出力(printfフォーマット) */
	void TraceOutString( const wchar_t* pszStr, int len = -1);	/* アウトプットウィンドウに出力(未加工文字列) */
	void SetTraceOutSource( HWND hwnd ){ m_hwndTraceOutSource = hwnd; }	/* TraceOut起動元ウィンドウの設定 */
	bool OpenDebugWindow( HWND hwnd, bool bAllwaysActive );	//!<  デバッグウィンドウを開く

	BOOL IsPrivateSettings( void );


	//マクロ関連
	int			GetMacroFilename( int idx, TCHAR* pszPath, int nBufLen ); // idxで指定したマクロファイル名（フルパス）を取得する	//	Jun. 14, 2003 genta 引数追加．書式変更
	bool		BeReloadWhenExecuteMacro( int idx );	//	idxで指定したマクロは、実行するたびにファイルを読み込む設定か？

	//タイプ別設定(コントロールプロセス専用)
	void CreateTypeSettings();
	std::vector<STypeConfig*>& GetTypeSettings();

	// 国際化対応のための文字列を変更する(コントロールプロセス専用)
	void ConvertLangValues(std::vector<std::wstring>& values, bool bSetValues);

	static CMutex& GetMutexShareWork();

protected:
	/*
	||  実装ヘルパ関数
	*/

	//	Jan. 30, 2005 genta 初期化関数の分割
	void InitKeyword(DLLSHAREDATA*);
	bool InitKeyAssign(DLLSHAREDATA*); // 2007.11.04 genta 起動中止のため値を返す
	void RefreshKeyAssignString(DLLSHAREDATA*);
	void InitToolButtons(DLLSHAREDATA*);
	void InitTypeConfigs(DLLSHAREDATA*, std::vector<STypeConfig*>&);
	void InitPopupMenu(DLLSHAREDATA*);

public:
	static void InitFileTree(SFileTree*);

private:
	CSelectLang m_cSelectLang;			// メッセージリソースDLL読み込み用（プロセスに1個）		// 2011.04.10 nasukoji
	HANDLE			m_hFileMap;
	DLLSHAREDATA*	m_pShareData;
	std::vector<STypeConfig*>* 	m_pvTypeSettings;	//	(コントロールプロセスのみ)
	HWND			m_hwndTraceOutSource;	// TraceOutA()起動元ウィンドウ（いちいち起動元を指定しなくてすむように）

};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_ENV_CSHAREDATA_H_ */



