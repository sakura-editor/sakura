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
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITDOC_H_
#define _CEDITDOC_H_


#include "global.h"
#include "CAutoSave.h"
#include "CDlgFind.h"
#include "CDlgFuncList.h"
#include "CDlgGrep.h"
#include "CDlgJump.h"
#include "CDlgOpenFile.h"
#include "CDlgReplace.h"
#include "CDocLineMgr.h"
#include "CEditView.h"
#include "CEol.h"
#include "CFuncLookup.h"
#include "CHokanMgr.h"
#include "CLayoutMgr.h"
#include "COpeBuf.h"
#include "CPropCommon.h"
#include "CPropTypes.h"
#include "CShareData.h"
#include "CSplitterWnd.h"
class CImageListMgr; // 2002/2/10 aroka
class CSMacroMgr; // 2002/2/10 aroka
class CEditWnd; // Sep. 10, 2002 genta
struct EditInfo; // 20050705 aroka
class CFuncInfoArr;
struct oneRule; // 2004.04.11 genta パラメータ内のstructを削除するため．doxygen対策

//! 文書関連情報の管理
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class SAKURA_CORE_API CEditDoc
{
public:
	/*
	||  Constructors
	*/
	CEditDoc();
	~CEditDoc();

	/*
	||  初期化系メンバ関数
	*/
	BOOL Create( HINSTANCE, HWND, CImageListMgr* /*, int, int, int, int*/ );
	void InitDoc();	/* 既存データのクリア */
	void InitAllView();	/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */
	bool CreateEditViewBySplit( int );	/* ビューの分割分のウィンドウ作成要求 */

	/*
	|| 状態
	*/
	BOOL IsTextSelected( void );	/* テキストが選択されているか */
	BOOL IsEnableUndo( void );		/* Undo(元に戻す)可能な状態か？ */
	BOOL IsEnableRedo( void );		/* Redo(やり直し)可能な状態か？ */
	BOOL IsEnablePaste( void );		/* クリップボードから貼り付け可能か？ */
	void SetFileInfo( EditInfo* );	/* 編集ファイル情報を格納 */

	/* いろいろ */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージディスパッチャ */
	void OnMove( int , int , int , int );
	BOOL OnFileClose( void );	/* ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行 */
	BOOL HandleCommand( int );
	void SetActivePane( int );	/* アクティブなペインを設定 */
	int GetActivePane( void );	/* アクティブなペインを取得 */
	void SetDrawSwitchOfAllViews( BOOL bDraw );					/* すべてのペインの描画スイッチを設定する */	// 2008.06.08 ryoji
	void RedrawAllViews( CEditView* pViewExclude );				/* すべてのペインをRedrawする */
	void Views_Redraw();				/* Redrawする */
	BOOL DetectWidthOfLineNumberAreaAllPane( BOOL bRedraw );	/* すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する） */
	BOOL WrapWindowWidth( int nPane );	/* 右端で折り返す */	// 2008.06.08 ryoji
	BOOL UpdateTextWrap( void );		/* 折り返し方法関連の更新 */	// 2008.06.10 ryoji
	BOOL SelectFont( LOGFONT* );
	BOOL FileRead( /*const*/ char* , BOOL*, ECodeType, BOOL, BOOL );	/* ファイルを開く */
	//	Feb. 9, 2001 genta 引数追加
	BOOL FileWrite( const char*, EEolType cEolType );
	bool SaveFile( const char* path );	//	ファイルの保存（に伴ういろいろ）
	void OpenFile( const char *filename = NULL, ECodeType nCharCode = CODE_AUTODETECT,
		BOOL bReadOnly = FALSE );	//	Oct. 9, 2004 genta CEditViewより移動
	void FileClose( void );	/* 閉じて(無題) */	// 2006.12.30 ryoji
	void FileCloseOpen( const char *filename = NULL, ECodeType nCharCode = CODE_AUTODETECT, BOOL bReadOnly = FALSE );	/* 閉じて開く */	// 2006.12.30 ryoji
	BOOL FileSave( bool warnbeep = true, bool askname = true );	/* 上書き保存 */	// 2006.12.30 ryoji
	BOOL FileSaveAs_Dialog( void );				/* 名前を付けて保存ダイアログ */	// 2006.12.30 ryoji
	BOOL FileSaveAs( const char *filename );	/* 名前を付けて保存 */	// 2006.12.30 ryoji

	int MakeBackUp( const char* target_file );	/* バックアップの作成 */
	void SetParentCaption( void );	/* 親ウィンドウのタイトルを更新 */	// 2007.03.08 ryoji bKillFocusパラメータを除去
	BOOL OpenPropertySheet( int/*, int*/ );	/* 共通設定 */
	BOOL OpenPropertySheetTypes( int, int );	/* タイプ別設定 */

	BOOL OpenFileDialog( HWND, const char*, char*, ECodeType*, BOOL* );	/* 「ファイルを開く」ダイアログ */
	void OnChangeSetting( void );	/* ビューに設定変更を反映させる */
	//	Jul. 26, 2003 ryoji BOMオプション追加
	BOOL SaveFileDialog( char*, ECodeType*, CEol* pcEol = NULL, BOOL* pbBomExist = NULL );	/* 「ファイル名を付けて保存」ダイアログ */

	void CheckFileTimeStamp( void );	/* ファイルのタイムスタンプのチェック処理 */
	void ReloadCurrentFile( ECodeType, BOOL );/* 同一ファイルの再オープン */

	//	May 15, 2000 genta
	CEol  GetNewLineCode() const { return m_cNewLineCode; }
	void  SetNewLineCode(const CEol& t){ m_cNewLineCode = t; }

	//	Aug. 14, 2000 genta
	bool IsModificationForbidden( int nCommand );
	bool IsEditable() const { return ( m_bReadOnly == FALSE && !( 0 != m_nFileShareModeOld && m_hLockedFile == NULL ) ); }	//!< 編集可能かどうか

	//	Aug. 21, 2000 genta
	CPassiveTimer	m_cAutoSave;	//!<	自動保存管理
	void	CheckAutoSave(void);
	void	ReloadAutoSaveParam(void);	//	設定をSharedAreaから読み出す

	//	Aug. 31, 2000 genta
	const CEditView& ActiveView(void) const { return m_cEditViewArr[m_nActivePaneIndex]; }
	CEditView& ActiveView(void) { return m_cEditViewArr[m_nActivePaneIndex]; }
	bool IsEnablePane(int n) const { return 0 <= n && n < m_nEditViewCount; }
	int	GetAllViewCount() const { return m_nEditViewCount; }

	CEditView* GetDragSourceView() const { return m_pcDragSourceView; }
	void SetDragSourceView( CEditView* pcDragSourceView ) { m_pcDragSourceView = pcDragSourceView; }

	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME状態の設定

	//	Sep. 9, 2002 genta
	const char* GetFilePath(void) const { return m_szFilePath; }
	const char* GetSaveFilePath(void) const { return (m_szSaveFilePath[0] != '\0')? m_szSaveFilePath: m_szFilePath; }
	bool IsFilePathAvailable(void) const { return m_szFilePath[0] != '\0'; }
	//	2002.10.13 Moca
	const char* GetFileName(void) const; //!< ファイル名(パスなし)を取得
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
	STypeConfig& GetDocumentAttribute(void) const	//!<	設定された文書情報への参照を返す
	{
		return m_pShareData->m_Types[m_nSettingType];
	}
	//	Nov. 23, 2000 To Here

	//	May 18, 2001 genta
	//! ReadOnly状態の読み出し
	BOOL IsReadOnly( void ){ return m_bReadOnly; }
	//! ReadOnly状態の設定
	void SetReadOnly( BOOL flag){ m_bReadOnly = flag; }
	
	//	Jan. 22, 2002 genta Modified Flagの設定
	void SetModified( bool flag, bool redraw);
	/** ファイルが修正中かどうか
		@retval true ファイルは修正されている
		@retval false ファイルは修正されていない
	*/
	bool IsModified( void ) const { return m_bIsModified; }

	//	Jan. 28, 2002 genta Modified Flagの設定 (暫定BOOL Version)
	void SetModified( bool flag, BOOL redraw){
		SetModified( flag, redraw != FALSE );
	}
	void SetModified( BOOL flag, bool redraw){
		SetModified( flag != FALSE, redraw );
	}
	
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
	const char* ExParam_SkipCond(const char* pszSource, int part);
	int ExParam_Evaluate( const char* pCond );

	//	Aug. 14, 2005 genta TAB幅と折り返し位置の更新
	void ChangeLayoutParam( bool bShowProgress, int nTabSize, int nMaxLineSize );
	
	//	Oct. 2, 2005 genta 挿入モードの設定
	bool IsInsMode() const { return m_bInsMode; }
	void SetInsMode(bool mode) { m_bInsMode = mode; }

	void RunAutoMacro( int idx, const char *pszSaveFilePath = NULL );	// 2006.09.01 ryoji マクロ自動実行

	// 2007.09.09 Moca 互換BMPによる画面バッファ 
	void DeleteCompatibleBitmap(); //!< CEditViewの画面バッファを削除


protected:
	int				m_nSettingType;
	bool			m_nSettingTypeLocked;	//	文書種別の一時設定状態
	//	Jan. 22, 2002 genta public -> protectedに移動
	/*! 変更フラグ
		@date 2002.01.22 genta public→protectedに移動．型をint→boolに．
	*/
	bool			m_bIsModified;
	//	Sep. 9, 2002 genta protectedに移動
	char			m_szFilePath[_MAX_PATH];	/* 現在編集中のファイルのパス */
	char			m_szSaveFilePath[_MAX_PATH];	/* 保存時のファイルのパス（マクロ用） */	// 2006.09.04 ryoji
	
	void SetFilePath(const char*);	// Sep. 9, 2002 genta
	
public: /* テスト用にアクセス属性を変更 */
	/* 入力補完 */
	CHokanMgr		m_cHokanMgr;
	BOOL			m_bGrepRunning;				/* Grep処理中 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
//	BOOL			m_bPrintPreviewMode;		/* 印刷プレビューモードか */
	int				m_nCommandExecNum;			/* コマンド実行回数 */
	FILETIME		m_FileTime;					/* ファイルの最終更新日付 */
	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;
	ECodeType		m_nCharCode;				/* 文字コード種別 */
	BOOL			m_bBomExist;	//!< 保存時にBOMを付けるかどうか Jul. 26, 2003 ryoji 
	bool			m_bInsMode;		//!< 挿入・上書きモード Oct. 2, 2005 genta

	//	May 15, 2000 genta
protected:
	CEol 			m_cNewLineCode;				//	Enter押下時に挿入する改行コード種別
public:
	CEol			m_cSaveLineCode;			//	保存時の改行コード種別（EOL_NONE:変換なし）


	BOOL			m_bReadOnly;				/* 読み取り専用モード */
	BOOL			m_bDebugMode;				/* デバッグモニタモード */
	BOOL			m_bGrepMode;				/* Grepモードか */
	char			m_szGrepKey[1024];			/* Grepモードの場合、その検索キー */
	HWND			m_hWnd;						/* 編集ウィンドウハンドル（CSplitterWndが管理） */
	COpeBuf			m_cOpeBuf;					/* アンドゥバッファ */
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
	void			MakeFuncList_Erlang( CFuncInfoArr* pcFuncInfoArr ); /* Erlang アウトライン解析*/ // 2009.08.11 genta
	void			MakeTopicList_wztxt(CFuncInfoArr*);		/* 階層付きテキスト アウトライン解析 */ // 2003.05.20 zenryaku
	void			MakeTopicList_html(CFuncInfoArr*);		/* HTML アウトライン解析 */ // 2003.05.20 zenryaku
	void			MakeTopicList_tex(CFuncInfoArr*);		/* TeX アウトライン解析 */ // 2003.07.20 naoh
	void			MakeFuncList_RuleFile( CFuncInfoArr* );	/* ルールファイルを使ってリスト作成 2002.04.01 YAZAKI */
	int				ReadRuleFile( const char* pszFilename, oneRule* pcOneRule, int nMaxCount );	/* ルールファイル読込 2002.04.01 YAZAKI */

// From Here 2001.12.03 hor
	void			MakeFuncList_BookMark( CFuncInfoArr* );	/* ブックマークリスト作成 */
// To Here 2001.12.03 hor

	CSplitterWnd	m_cSplitterWnd;				/* 分割フレーム */
	CEditView		m_cEditViewArr[4];			/* ビュー */
	int				m_nEditViewCount;			/* ビューのうちウィンドウが存在している数 */
	int				m_nActivePaneIndex;			/* アクティブなビュー */
	CEditView*		m_pcDragSourceView;			/* ドラッグ元のビュー */
//	HWND			m_hwndActiveDialog;			/* アクティブな子ダイアログ */
	CDlgFind		m_cDlgFind;					/* 「検索」ダイアログ */
	CDlgReplace		m_cDlgReplace;				/* 「置換」ダイアログ */
	CDlgJump		m_cDlgJump;					/* 「指定行へジャンプ」ダイアログ */
//	CDlgSendMail	m_cDlgSendMail;				/* メール送信ダイアログ */
	CDlgGrep		m_cDlgGrep;					/* Grepダイアログ */
	CDlgFuncList	m_cDlgFuncList;				/* アウトライン解析結果ダイアログ */
	CEditWnd*		m_pcEditWnd;	//	Sep. 10, 2002
	/*
	||  メンバ変数
	*/
	char*			m_pszAppName;		/* Mutex作成用・ウィンドウクラス名 */
	HINSTANCE		m_hInstance;		/* インスタンスハンドル */
	HWND			m_hwndParent;		/* 親ウィンドウ（CEditWndが管理）ハンドル */

	DLLSHAREDATA*	m_pShareData;

//@@@ 2002.01.14 YAZAKI 不使用のため
//	COpeBlk*		m_pcOpeBlk;			/* 操作ブロック */
	BOOL			m_bDoing_UndoRedo;	/* アンドゥ・リドゥの実行中か */
	CDlgOpenFile	m_cDlgOpenFile;	/* ファイルオープンダイアログ */
//	2004.03.28 Moca 未使用変数削除

//	2004.02.16 Moca メンバから外す
//	CHOOSEFONT		m_cf;				/* フォント選択ダイアログ用 */

//@@	CProp1			m_cProp1;			/* 設定プロパティシート */
	CPropCommon		m_cPropCommon;
	CPropTypes		m_cPropTypes;

	int				m_nFileShareModeOld;	/* ファイルの排他制御モード */
	HFILE			m_hLockedFile;			/* ロックしているファイルのハンドル */

// 2004/06/21 novice タグジャンプ機能追加
#if 0
	HWND			m_hwndReferer;	/* 参照元ウィンドウ */
	int				m_nRefererX;	/* 参照元 行頭からのバイト位置桁 */
	int				m_nRefererLine;	/* 参照元行 折り返し無しの物理行位置 */
#endif
	
	//	Sep. 29, 2001 genta
	CSMacroMgr*		m_pcSMacroMgr;	//!< マクロ
//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一。
//	CKeyMacroMgr	m_CKeyMacroMgr;	//	キーボードマクロ @@@ 2002.1.24 YAZAKI DLLSHAREDATAから移動
	//	Oct. 2, 2001 genta
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

//	CDlgTest*		m_pcDlgTest;

	int m_nTextWrapMethodCur;		// 折り返し方法					// 2008.05.30 nasukoji
	bool m_bTextWrapMethodCurTemp;	// 折り返し方法一時設定適用中	// 2008.05.30 nasukoji

	/*
	||  実装ヘルパ関数
	*/
	void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);	//	pszSourceを展開して、pszBufferにコピー。
protected:
	void DoFileLock( void );	/* ファイルの排他ロック */
	void DoFileUnLock( void );	/* ファイルの排他ロック解除 */
	//char			m_pszCaption[1024];	//@@@ YAZAKI
	
	//	Aug. 14, 2005 genta
	int* SavePhysPosOfAllView(void);
	void RestorePhysPosOfAllView( int* posary );

	// 2005.11.21 aroka
	// 2008.11.23 nasukoji	パスが長すぎる場合への対応
	bool FormatBackUpPath( char* szNewPath, DWORD dwSize, const char* target_file );	/* バックアップパスの作成 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */


/*[EOF]*/
