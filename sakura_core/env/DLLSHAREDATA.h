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
#ifndef SAKURA_DLLSHAREDATA_3A6DD7E0_90DC_4219_8570_F5C1B8B6A306_H_
#define SAKURA_DLLSHAREDATA_3A6DD7E0_90DC_4219_8570_F5C1B8B6A306_H_

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         アクセサ                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//! どこからでもアクセスできる、共有データアクセサ。2007.10.30 kobake
struct DLLSHAREDATA;

//DLLSHAREDATAへの簡易アクセサ
inline DLLSHAREDATA& GetDllShareData()
{
	extern DLLSHAREDATA* g_theDLLSHAREDATA;

	assert(g_theDLLSHAREDATA);
	return *g_theDLLSHAREDATA;
}

inline DLLSHAREDATA& GetDllShareData(bool bNullCheck)
{
	extern DLLSHAREDATA* g_theDLLSHAREDATA;

	if( bNullCheck ){
		assert(g_theDLLSHAREDATA);
	}
	return *g_theDLLSHAREDATA;
}

//DLLSHAREDATAを確保したら、まずこれを呼ぶ。破棄する前にも呼ぶ。
inline void SetDllShareData(DLLSHAREDATA* pShareData)
{
	extern DLLSHAREDATA* g_theDLLSHAREDATA;

	g_theDLLSHAREDATA = pShareData;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    共有メモリ構成要素                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 2010.04.19 Moca CShareDataからDLLSHAREDATAメンバのincludeをDLLSHAREDATA.hに移動

#include "config/maxdata.h"

#include "env/CAppNodeManager.h"	//SShare_Nodes
//2007.09.28 kobake Common構造体をCShareData.hから分離
#include "env/CommonSetting.h"
#include "env/CSearchKeywordManager.h"	//SShare_SearchKeywords
#include "env/CTagJumpManager.h"		//SShare_TagJump
#include "env/CFileNameManager.h"		//SShare_FileNameManagement

#include "EditInfo.h"
#include "types/CType.h" // STypeConfig
#include "print/CPrint.h" //PRINTSETTING
#include "recent/SShare_History.h"	//SShare_History





//! 共有フラグ
struct SShare_Flags{
	BOOL				m_bEditWndChanging;				// 編集ウィンドウ切替中	// 2007.04.03 ryoji
	/*	@@@ 2002.1.24 YAZAKI
		キーボードマクロは、記録終了した時点でファイル「m_szKeyMacroFileName」に書き出すことにする。
		m_bRecordingKeyMacroがTRUEのときは、キーボードマクロの記録中なので、m_szKeyMacroFileNameにアクセスしてはならない。
	*/
	BOOL				m_bRecordingKeyMacro;		/* キーボードマクロの記録中 */
	HWND				m_hwndRecordingKeyMacro;	/* キーボードマクロを記録中のウィンドウ */
};

//! 共有ワークバッファ
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
	CLogicPoint			m_LogicPoint;					//!< カーソル位置
	STypeConfig			m_TypeConfig;
};

//! 共有ハンドル
struct SShare_Handles{
	HWND				m_hwndTray;
	HWND				m_hwndDebug;
	HACCEL				m_hAccel;
};

//! EXE情報
struct SShare_Version{
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
};


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   共有メモリ構造体本体                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

struct DLLSHAREDATA{
	// -- -- バージョン -- -- //
	/*!
		データ構造 Version	//	Oct. 27, 2000 genta
		データ構造の異なるバージョンの同時起動を防ぐため
		必ず先頭になくてはならない．
	*/
	unsigned int				m_vStructureVersion;
	unsigned int				m_nSize;

	// -- -- 非保存対象 -- -- //
	SShare_Version				m_sVersion;	//※読込は行わないが、書込は行う
	SShare_WorkBuffer			m_sWorkBuffer;
	SShare_Flags				m_sFlags;
	SShare_Nodes				m_sNodes;
	SShare_Handles				m_sHandles;

	SCharWidthCache				m_sCharWidth;							//!< 文字半角全角キャッシュ
	DWORD						m_dwCustColors[16];						//!< フォントDialogカスタムパレット

	// プラグイン
	short						m_PlugCmdIcon[MAX_PLUGIN*MAX_PLUG_CMD];	//!< プラグイン コマンド ICON 番号	// 2010/7/3 Uchi
	int							m_maxTBNum;								//!< ツールバーボタン 最大値		// 2010/7/5 Uchi

	// -- -- 保存対象 -- -- //
	//設定
	CommonSetting				m_Common;								// 共通設定
	int							m_nTypesCount;	// タイプ別設定数
	STypeConfig					m_TypeBasis;							// タイプ別設定: 共通
	STypeConfigMini				m_TypeMini[MAX_TYPES];					// タイプ別設定(mini)
	PRINTSETTING				m_PrintSettingArr[MAX_PRINTSETTINGARR];	// 印刷ページ設定
	int							m_nLockCount;	//!< ロックカウント
	
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

class CShareDataLockCounter{
public:
	CShareDataLockCounter();
	~CShareDataLockCounter();

	static int GetLockCounter();
	static void WaitLock( HWND, CShareDataLockCounter** = NULL );
private:
};
#endif /* SAKURA_DLLSHAREDATA_3A6DD7E0_90DC_4219_8570_F5C1B8B6A306_H_ */
/*[EOF]*/
