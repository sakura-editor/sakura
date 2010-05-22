#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         アクセサ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! どこからでもアクセスできる、共有データアクセサ。2007.10.30 kobake
struct DLLSHAREDATA;
DLLSHAREDATA& GetDllShareData();

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    共有メモリ構成要素                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 2010.04.19 Moca CShareDataからDLLSHAREDATAメンバのincludeをDLLSHAREDATA.hに移動

#include "config/maxdata.h"


#include "EditInfo.h"
#include "env/CAppNodeManager.h"	//SShare_Nodes

//2007.09.28 kobake Common構造体をCShareData.hから分離
#include "env/CommonSetting.h"

#include "types/CType.h" // STypeConfig
#include "CPrint.h" //PRINTSETTING
#include "env/CSearchKeywordManager.h"	//SShare_SearchKeywords
#include "env/CTagJumpManager.h"		//SShare_TagJump
#include "env/CFileNameManager.h"		//SShare_FileNameManagement
#include "recent/SShare_History.h"	//SShare_History

#ifndef SAKURA_EXCLUDE_OPTIONAL_INCLUDE
//@@@ 2001.12.26 YAZAKI CMRU, CMRUFolder
#include "recent/CMRU.h"
#include "recent/CMRUFolder.h"

#include "env/CFormatManager.h"
#include "env/CHelpManager.h"
#endif





//共有フラグ
struct SShare_Flags{
	BOOL				m_bEditWndChanging;				// 編集ウィンドウ切替中	// 2007.04.03 ryoji
	/*	@@@ 2002.1.24 YAZAKI
		キーボードマクロは、記録終了した時点でファイル「m_szKeyMacroFileName」に書き出すことにする。
		m_bRecordingKeyMacroがTRUEのときは、キーボードマクロの記録中なので、m_szKeyMacroFileNameにアクセスしてはならない。
	*/
	BOOL				m_bRecordingKeyMacro;		/* キーボードマクロの記録中 */
	HWND				m_hwndRecordingKeyMacro;	/* キーボードマクロを記録中のウィンドウ */
};

//共有ワークバッファ
struct SShare_WorkBuffer{
	//2007.09.16 kobake char型だと、常に文字列であるという誤解を招くので、BYTE型に変更。変数名も変更。
	//           UNICODE版では、余分に領域を使うことが予想されるため、ANSI版の2倍確保。
private:
	BYTE				m_pWork[32000*sizeof(TCHAR)];
public:
	template <class T>
	T* GetWorkBuffer(){ return reinterpret_cast<T*>(m_pWork); }

	template <class T>
	size_t GetWorkBufferCount(){ return sizeof(m_pWork)/sizeof(T); }

public:
	EditInfo			m_EditInfo_MYWM_GETFILEINFO;	//MYWM_GETFILEINFOデータ受け渡し用	####美しくない
};

//共有ハンドル
struct SShare_Handles{
	HWND				m_hwndTray;
	HWND				m_hwndDebug;
	HACCEL				m_hAccel;
};

//EXE情報
struct SShare_Version{
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   共有メモリ構造体本体                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DLLSHAREDATA{
	void OnInit();
	
	// -- -- バージョン -- -- //
	/*!
		データ構造 Version	//	Oct. 27, 2000 genta
		データ構造の異なるバージョンの同時起動を防ぐため
		必ず先頭になくてはならない．
	*/
	unsigned int				m_vStructureVersion;

	// -- -- 非保存対象 -- -- //
	SShare_Version				m_sVersion;	//※読込は行わないが、書込は行う
	SShare_WorkBuffer			m_sWorkBuffer;
	SShare_Flags				m_sFlags;
	SShare_Nodes				m_sNodes;
	SShare_Handles				m_sHandles;
	// 文字半角全角キャッシュ
	BYTE				m_bCharWidthCache[0x10000/4];		//16KB 文字半角全角キャッシュ 2008/5/16 Uchi
	int					m_nCharWidthCacheTest;				//cache溢れ検出

	// -- -- 保存対象 -- -- //
	//設定
	CommonSetting				m_Common;								// 共通設定
	STypeConfig					m_Types[MAX_TYPES];						// タイプ別設定
	PRINTSETTING				m_PrintSettingArr[MAX_PRINTSETTINGARR];	// 印刷ページ設定
	//その他
	SShare_SearchKeywords		m_sSearchKeywords;
	SShare_TagJump				m_sTagJump;
	SShare_FileNameManagement	m_sFileNameManagement;
	SShare_History				m_sHistory;

	//外部コマンド実行ダイアログのオプション
	int							m_nExecFlgOpt;				/* 外部コマンド実行オプション */	//	2006.12.03 maru オプションの拡張のため
	//DIFF差分表示ダイアログのオプション
	int							m_nDiffFlgOpt;				/* DIFF差分表示 */	//@@@ 2002.05.27 MIK
	//タグファイルの作成ダイアログのオプション
	TCHAR						m_szTagsCmdLine[_MAX_PATH];	/* TAGSコマンドラインオプション */	//@@@ 2003.05.12 MIK
	int							m_nTagsOpt;					/* TAGSオプション(チェック) */	//@@@ 2003.05.12 MIK


	// -- -- テンポラリ -- -- //
	//指定行へジャンプダイアログのオプション
	bool						m_bLineNumIsCRLF_ForJump;			/* 指定行へジャンプの「改行単位の行番号」か「折り返し単位の行番号」か */
};


