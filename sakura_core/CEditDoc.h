/*!	@file
	@brief 文書関連情報の管理

	@author Norio Nakatani
	@date	1998/03/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, MIK, hor
	Copyright (C) 2002, genta, Moca, YAZAKI
	Copyright (C) 2003, genta, ryoji, zenryaku, naoh
	Copyright (C) 2004, Moca, novice, genta
	Copyright (C) 2005, Moca, genta, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITDOC_H_
#define _CEDITDOC_H_


#include "global.h"
#include "CAutoSave.h"
#include "CDocLineMgr.h"
#include "CEol.h"
#include "CFuncLookup.h"
#include "CLayoutMgr.h"
#include "COpeBuf.h"
#include "CShareData.h"
#include <string>

//#include <windows.h>
class CImageListMgr; // 2002/2/10 aroka
class CSMacroMgr; // 2002/2/10 aroka
class CEditWnd; // Sep. 10, 2002 genta
struct FileInfo; // 20050705 aroka
class CFuncInfoArr;
struct oneRule; // 2004.04.11 genta パラメータ内のstructを削除するため．doxygen対策
class CEditApp;

typedef CEditWnd* P_CEditWnd;

//! 文書関連情報の管理
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class SAKURA_CORE_API CEditDoc
{
public:
	/*
	||  Constructors
	*/
	CEditDoc(CEditApp* pcApp);
	~CEditDoc();

	/*
	||  初期化系メンバ関数
	*/
	BOOL Create( HINSTANCE, CImageListMgr* );
	void Init( void );	/* 既存データのクリア */
	void InitAllView();	/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */

	/*
	|| 状態
	*/
	BOOL IsEnableUndo( void );		/* Undo(元に戻す)可能な状態か？ */
	BOOL IsEnableRedo( void );		/* Redo(やり直し)可能な状態か？ */
	BOOL IsEnablePaste( void );		/* クリップボードから貼り付け可能か？ */
	void GetFileInfo( FileInfo* ) const;	/* 編集ファイル情報を格納 */ //2007.10.24 kobake 関数名変更: SetFileInfo→GetFileInfo

	/* いろいろ */
	BOOL OnFileClose( void );	/* ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行 */
	BOOL HandleCommand( EFunctionCode );
	BOOL SelectFont( LOGFONT* );

	/* ファイルを開く */
	bool FileRead(
		TCHAR*		pszPath,			//!< [in/out]
		bool*		pbOpened,			//!< [out] すでに開かれていたか
		ECodeType	nCharCode,			/*!< [in] 文字コード種別 */
		bool		bReadOnly,			/*!< [in] 読み取り専用か */
		bool		bConfirmCodeChange	/*!< [in] 文字コード変更時の確認をするかどうか */
	);

	//	Feb. 9, 2001 genta 引数追加
	BOOL FileWrite( const TCHAR*, enumEOLType cEolType );
	bool SaveFile( const TCHAR* path );	//	ファイルの保存（に伴ういろいろ）
	void OpenFile( const TCHAR* filename = NULL, ECodeType nCharCode = CODE_AUTODETECT,
		bool bReadOnly = FALSE );	//	Oct. 9, 2004 genta CEditViewより移動
	void FileClose( void );	/* 閉じて(無題) */	// 2006.12.30 ryoji
	void FileCloseOpen( const TCHAR* filename = NULL, ECodeType nCharCode = CODE_AUTODETECT, bool bReadOnly = FALSE );	/* 閉じて開く */	// 2006.12.30 ryoji
	BOOL FileSave( bool warnbeep = true, bool askname = true );	/* 上書き保存 */	// 2006.12.30 ryoji
	BOOL FileSaveAs_Dialog( void );				/* 名前を付けて保存ダイアログ */	// 2006.12.30 ryoji
	BOOL FileSaveAs( const TCHAR* filename );	/* 名前を付けて保存 */	// 2006.12.30 ryoji

	int MakeBackUp( const TCHAR* target_file );	/* バックアップの作成 */
	void SetParentCaption( void );	/* 親ウィンドウのタイトルを更新 */	// 2007.03.08 ryoji bKillFocusパラメータを除去
	BOOL OpenPropertySheet( int/*, int*/ );	/* 共通設定 */
	BOOL OpenPropertySheetTypes( int, int );	/* タイプ別設定 */

	BOOL OpenFileDialog( HWND, const TCHAR*, TCHAR*, ECodeType*, bool* );	/* 「ファイルを開く」ダイアログ */
	void OnChangeSetting( void );	/* ビューに設定変更を反映させる */
// 2004/06/21 novice タグジャンプ機能追加
#if 0
	void SetReferer( HWND , int, int );	/* タグジャンプ元など参照元の情報を保持する */
#endif
	//	Jul. 26, 2003 ryoji BOMオプション追加
	BOOL SaveFileDialog( TCHAR*, ECodeType*, CEOL* pcEol = NULL, BOOL* pbBomExist = NULL );	/* 「ファイル名を付けて保存」ダイアログ */

	void CheckFileTimeStamp( void );	/* ファイルのタイムスタンプのチェック処理 */
	void ReloadCurrentFile( ECodeType, bool );/* 同一ファイルの再オープン */

	//	May 15, 2000 genta
	CEOL  GetNewLineCode() const { return m_cNewLineCode; }
	void  SetNewLineCode(const CEOL& t){ m_cNewLineCode = t; }

	//	Aug. 14, 2000 genta
	bool IsModificationForbidden( int nCommand );

	//	Aug. 21, 2000 genta
	CPassiveTimer	m_cAutoSave;	//!<	自動保存管理
	void	CheckAutoSave(void);
	void	ReloadAutoSaveParam(void);	//	設定をSharedAreaから読み出す

	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME状態の設定

	//	Sep. 9, 2002 genta
	const TCHAR* GetFilePath(void) const { return m_szFilePath; }
	bool IsFilePathAvailable(void) const { return m_szFilePath[0] != _T('\0'); }
	//	2002.10.13 Moca
	const TCHAR* GetFileName(void) const; //!< ファイル名(パスなし)を取得
	void SetDocumentIcon(void);	// Sep. 10, 2002 genta

	//	Nov. 29, 2000 From Here	genta
	//	設定の一時変更時に拡張子による強制的な設定変更を無効にする
	void LockDocumentType(void){ m_nSettingTypeLocked = true; }
	void UnlockDocumentType(void){ m_nSettingTypeLocked = false; }
	bool GetDocumentLockState(void){ return m_nSettingTypeLocked; }
	//	Nov. 29, 2000 To Here
	//	Nov. 23, 2000 From Here	genta
	//	文書種別情報の設定，取得Interface
	void SetDocumentType(int type, bool force)	//	文書種別の設定
	{
		if( (!m_nSettingTypeLocked) || force ){
			m_nSettingType = type;
			UnlockDocumentType();
			m_pShareData->m_Types[m_nSettingType].m_nRegexKeyMagicNumber++;	//@@@ 2001.11.17 add MIK
			SetDocumentIcon();	// Sep. 11, 2002 genta
		}
	}
	int GetDocumentType(void) const	//!<	文書種別の読み出し
	{
		return m_nSettingType;
	}
	Types& GetDocumentAttribute(void) const	//!<	設定された文書情報への参照を返す
	{
		return m_pShareData->m_Types[m_nSettingType];
	}
	//	Nov. 23, 2000 To Here

	//	May 18, 2001 genta
	//! ReadOnly状態の読み出し
	BOOL IsReadOnly( void ){ return m_bReadOnly; }
	//! ReadOnly状態の設定
	void SetReadOnly( bool flag){ m_bReadOnly = flag; }
	
	//	Jan. 22, 2002 genta Modified Flagの設定
	void SetModified( bool flag, bool redraw);
	/** ファイルが修正中かどうか
		@retval true ファイルは修正されている
		@retval false ファイルは修正されていない
	*/
	bool IsModified( void ) const { return m_bIsModified; }

	/*! @brief このウィンドウで新しいファイルを開けるか

		新しいウィンドウを開かずに現在のウィンドウを再利用できるかどうかのテストを行う．
		変更済み，ファイルを開いている，Grepウィンドウ，アウトプットウィンドウの場合には
		再利用不可．

		@author Moca
		@date 2005.06.24 Moca
	*/
	bool IsFileOpenInThisWindow( void ){
		return !( IsModified() || IsFilePathAvailable() || m_bGrepMode || m_bDebugMode );
	}

	void AddToMRU(void); // Mar. 30, 2003 genta
	// Mar. 31, 2003 genta ExpandParameter補助関数
	const wchar_t* ExParam_SkipCond(const wchar_t* pszSource, int part);
	int ExParam_Evaluate( const wchar_t* pCond );

	
	//	Oct. 2, 2005 genta 挿入モードの設定
	bool IsInsMode() const { return m_bInsMode; }
	void SetInsMode(bool mode) { m_bInsMode = mode; }

protected:
	
	void SetFilePath(const TCHAR* szFile);	// Sep. 9, 2002 genta
public:
	void			MakeFuncList_C( CFuncInfoArr*,bool bVisibleMemberFunc = true );		/* C/C++関数リスト作成 */
	void 			MakeFuncList_PLSQL( CFuncInfoArr* );	/* PL/SQL関数リスト作成 */
	void 			MakeTopicList_txt( CFuncInfoArr* );		/* テキスト・トピックリスト作成 */
	void			MakeFuncList_Java( CFuncInfoArr* );		/* Java関数リスト作成 */
	void			MakeTopicList_cobol( CFuncInfoArr* );	/* COBOL アウトライン解析 */
	void			MakeTopicList_asm( CFuncInfoArr* );		/* アセンブラ アウトライン解析 */
	void			MakeFuncList_Perl( CFuncInfoArr* );		/* Perl関数リスト作成 */	//	Sep. 8, 2000 genta
	void			MakeFuncList_VisualBasic( CFuncInfoArr* );/* Visual Basic関数リスト作成 */ //June 23, 2001 N.Nakatani
	void			MakeFuncList_python( CFuncInfoArr* pcFuncInfoArr ); /* Python アウトライン解析*/ // 2007.02.08 genta
	void			MakeTopicList_wztxt(CFuncInfoArr*);		/* 階層付きテキスト アウトライン解析 */ // 2003.05.20 zenryaku
	void			MakeTopicList_html(CFuncInfoArr*);		/* HTML アウトライン解析 */ // 2003.05.20 zenryaku
	void			MakeTopicList_tex(CFuncInfoArr*);		/* TeX アウトライン解析 */ // 2003.07.20 naoh
	void			MakeFuncList_RuleFile( CFuncInfoArr* );	/* ルールファイルを使ってリスト作成 2002.04.01 YAZAKI */
	int				ReadRuleFile( const TCHAR* pszFilename, oneRule* pcOneRule, int nMaxCount );	/* ルールファイル読込 2002.04.01 YAZAKI */

// From Here 2001.12.03 hor
	void			MakeFuncList_BookMark( CFuncInfoArr* );	/* ブックマークリスト作成 */
// To Here 2001.12.03 hor

	/*
	||  実装ヘルパ関数
	*/
	void ExpandParameter(const WChar* pszSource, WChar* pszBuffer, int nBufferLen);	//	pszSourceを展開して、pszBufferにコピー。
protected:
	void DoFileLock( void );	/* ファイルの排他ロック */
	void DoFileUnLock( void );	/* ファイルの排他ロック解除 */
	//char			m_pszCaption[1024];	//@@@ YAZAKI
	
	// 2005.11.21 aroka
	bool FormatBackUpPath( TCHAR* szNewPath, const TCHAR* target_file );	/* バックアップパスの作成 */

public:
	HWND GetSplitterHwnd() const;
	HWND GetOwnerHwnd() const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	P_CEditWnd&		m_pcEditWnd;	//	Sep. 10, 2002

	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;

private:
//	HWND			m_hwndParent;		/* 親ウィンドウ（CEditWndが管理）ハンドル */

protected:
	int				m_nSettingType;
	bool			m_nSettingTypeLocked;	//	文書種別の一時設定状態
	bool			m_bIsModified;

private:
	SFilePath		m_szFilePath;	/* 現在編集中のファイルのパス */

public: /* テスト用にアクセス属性を変更 */
	BOOL			m_bGrepRunning;				/* Grep処理中 */
	int				m_nCommandExecNum;			/* コマンド実行回数 */
	FILETIME		m_FileTime;					/* ファイルの最終更新日付 */
	ECodeType		m_nCharCode;				/* 文字コード種別 */
	BOOL			m_bBomExist;	//!< 保存時にBOMを付けるかどうか Jul. 26, 2003 ryoji 
	bool			m_bInsMode;		//!< 挿入・上書きモード Oct. 2, 2005 genta

	//	May 15, 2000 genta
protected:
	CEOL 			m_cNewLineCode;				//	Enter押下時に挿入する改行コード種別

public:
	CEOL			m_cSaveLineCode;			//	保存時の改行コード種別（EOL_NONE:変換なし）

	bool			m_bReadOnly;				/* 読み取り専用モード */
	BOOL			m_bDebugMode;				/* デバッグモニタモード */
	BOOL			m_bGrepMode;				/* Grepモードか */
	wchar_t			m_szGrepKey[1024];			/* Grepモードの場合、その検索キー */
public:
	COpeBuf			m_cOpeBuf;					/* アンドゥバッファ */

	TCHAR*			m_pszAppName;		/* Mutex作成用・ウィンドウクラス名 */
	HINSTANCE		m_hInstance;		/* インスタンスハンドル */
	DLLSHAREDATA*	m_pShareData;
	BOOL			m_bDoing_UndoRedo;	/* アンドゥ・リドゥの実行中か */
	EShareMode	m_nFileShareModeOld;	/* ファイルの排他制御モード */
	HANDLE			m_hLockedFile;			/* ロックしているファイルのハンドル */
	CSMacroMgr*		m_pcSMacroMgr;	//!< マクロ
	CFuncLookup		m_cFuncLookup;	//!< 機能名，機能番号などのresolve

	//	From Here Dec. 4, 2002 genta
	//	更新監視と更新通知
	/*!
		ファイルが更新された場合に再読込を行うかどうかのフラグ
	*/
	enum WatchUpdate {
		WU_QUERY,	//!< 再読込を行うかどうかダイアログボックスで問い合わせる
		WU_NOTIFY,	//!< 更新されたことをステータスバーで通知
		WU_NONE		//!< 更新監視を行わない
	};
	WatchUpdate		m_eWatchUpdate;	//!< 更新監視方法
	//	To Here Dec. 4, 2002 genta

};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */


/*[EOF]*/
