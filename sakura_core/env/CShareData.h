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

class CShareData;

#ifndef _CSHAREDATA_H_
#define _CSHAREDATA_H_

#include <windows.h>
#include <commctrl.h>
#include "func/CKeyBind.h"
#include "CKeyWordSetMgr.h"
#include "CPrint.h"
#include "CDataProfile.h"
#include "basis/SakuraBasis.h"
#include "config/maxdata.h"
#include "env/CAppNodeManager.h"
#include "func/CFuncLookup.h" //MacroRec




//@@@ 2001.12.26 YAZAKI CMRU, CMRUFolder
#include "recent/CMRU.h"
#include "recent/CMRUFolder.h"

//20020129 aroka
#include "func/Funccode.h"
#include "mem/CMemory.h"

#include "CMutex.h"	// 2007.07.07 genta

#include "doc/CLineComment.h"	//@@@ 2002.09.22 YAZAKI
#include "doc/CBlockComment.h"	//@@@ 2002.09.22 YAZAKI

#include "EditInfo.h"
#include "doc/CDocTypeSetting.h"





//2007.09.28 kobake Common構造体を
#include "env/CommonSetting.h"



#include "util/StaticType.h"

#include "basis/CStrictInteger.h"
#include "CPrint.h" //PRINTSETTING


#include "CDocTypeManager.h"
#include "CFormatManager.h"
#include "CHelpManager.h"


#include "DLLSHAREDATA.h"

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
class SAKURA_CORE_API CShareData
{
public:
	/*
	||	Singleton風
	*/
	static CShareData* getInstance();

public:
	/*
	||  Constructors
	*/
	CShareData();
	~CShareData();


	/*
	||  Attributes & Operations
	*/
	bool InitShareData();	/* CShareDataクラスの初期化処理 */
	DLLSHAREDATA* GetShareData(){ return m_pShareData; }		/* 共有データ構造体のアドレスを返す */

	
	//MRU系
	BOOL IsPathOpened( const TCHAR* pszPath, HWND* phwndOwner ); /* 指定ファイルが開かれているか調べる */
	BOOL ActiveAlreadyOpenedWindow( const TCHAR* pszPath, HWND* phwndOwner, ECodeType nCharCode );/* 指定ファイルが開かれているか調べつつ、多重オープン時の文字コード衝突も確認 */	// 2007.03.16

	//デバッグ  今は主にマクロ・外部コマンド実行用
	void TraceOut( LPCTSTR lpFmt, ...);	/* アウトプットウィンドウに出力(printfフォーマット) */
	void TraceOutString( const wchar_t* pszStr, int len = -1);	/* アウトプットウィンドウに出力(未加工文字列) */
	void SetTraceOutSource( HWND hwnd ){ m_hwndTraceOutSource = hwnd; }	/* TraceOut起動元ウィンドウの設定 */
	bool OpenDebugWindow( HWND hwnd, bool bAllwaysActive );	//!<  デバッグウィンドウを開く

	BOOL IsPrivateSettings( void ){ return m_pShareData->m_sFileNameManagement.m_IniFolder.m_bWritePrivate; }			/* iniファイルの保存先がユーザ別設定フォルダかどうか */	// 2007.05.25 ryoji


	//マクロ関連
	int			GetMacroFilename( int idx, TCHAR* pszPath, int nBufLen ); // idxで指定したマクロファイル名（フルパス）を取得する	//	Jun. 14, 2003 genta 引数追加．書式変更
	bool		BeReloadWhenExecuteMacro( int idx );	//	idxで指定したマクロは、実行するたびにファイルを読み込む設定か？

	


protected:
	/*
	||  実装ヘルパ関数
	*/

	//	Jan. 30, 2005 genta 初期化関数の分割
	void InitKeyword(DLLSHAREDATA*);
	bool InitKeyAssign(DLLSHAREDATA*); // 2007.11.04 genta 起動中止のため値を返す
	void InitToolButtons(DLLSHAREDATA*);
	void InitTypeConfigs(DLLSHAREDATA*);
	void InitPopupMenu(DLLSHAREDATA*);

private:
	static CShareData* _instance;

private:
	HANDLE			m_hFileMap;
	DLLSHAREDATA*	m_pShareData;
	HWND			m_hwndTraceOutSource;	// TraceOutA()起動元ウィンドウ（いちいち起動元を指定しなくてすむように）

};



///////////////////////////////////////////////////////////////////////
#endif /* _CSHAREDATA_H_ */



