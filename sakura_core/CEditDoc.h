//	$Id$
/*!	@file
	@brief 文書関連情報の管理

	@author Norio Nakatani
	@date	1998/03/13 作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITDOC_H_
#define _CEDITDOC_H_


#include <windows.h>
//#include "CMemory.h"  // 2002/2/10 aroka
#include "CDocLineMgr.h"
#include "CLayoutMgr.h"
//#include "COpe.h" // 2002/2/10 aroka
//#include "COpeBlk.h" // 2002/2/10 aroka
#include "COpeBuf.h"
#include "CDlgFind.h"
#include "CDlgReplace.h"
//@@#include "CProp1.h"
#include "CShareData.h"
//#include "CFuncInfoArr.h" // 2002/2/10 aroka
//@@@ 2002.01.14 YAZAKI 不要
//#include "CSplitBoxWnd.h"
#include "CEditView.h"
#include "CSplitterWnd.h"
#include "CDlgOpenFile.h"
//#include "CDlgSendMail.h"
#include "CDlgGrep.h"
#include "CDlgJump.h"
#include "CPropCommon.h"
#include "CPropTypes.h"
#include "CDlgFuncList.h"
//#include "CDlgTest.h"
#include "CHokanMgr.h"
#include "CAutoSave.h"
//#include "CImageListMgr.h" // 2002/2/10 aroka
//#include "CSMacroMgr.h" // 2002/2/10 aroka
#include "CFuncLookup.h"
class CImageListMgr; // 2002/2/10 aroka
class CSMacroMgr; // 2002/2/10 aroka

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
	void Init( void );	/* 既存データのクリア */
	void InitAllView();	/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */

	/*
	|| 状態
	*/
	BOOL IsTextSelected( void );	/* テキストが選択されているか */
	BOOL IsEnableUndo( void );		/* Undo(元に戻す)可能な状態か？ */
	BOOL IsEnableRedo( void );		/* Redo(やり直し)可能な状態か？ */
	BOOL IsEnablePaste( void );		/* クリップボードから貼り付け可能か？ */
	void SetFileInfo( FileInfo* );	/* 編集ファイル情報を格納 */

	/* いろいろ */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージディスパッチャ */
	void OnMove( int , int , int , int );
	BOOL OnFileClose( void );	/* ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行 */
	BOOL HandleCommand( int );
	void SetActivePane( int );	/* アクティブなペインを設定 */
	int GetActivePane( void );	/* アクティブなペインを取得 */
	void RedrawInactivePane( void );	/* 非アクティブなペインをRedrawする */
	BOOL DetectWidthOfLineNumberAreaAllPane( BOOL bRedraw );	/* すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する） */
	BOOL SelectFont( LOGFONT* );
	BOOL FileRead( /*const*/ char* , BOOL*, int, BOOL, BOOL );	/* ファイルを開く */
	//	Feb. 9, 2001 genta 引数追加
	BOOL FileWrite( const char*, enumEOLType cEolType );
	bool SaveFile( const char* path );	//	ファイルの保存（に伴ういろいろ）
	BOOL MakeBackUp( void );	/* バックアップの作成 */
	void SetParentCaption( BOOL = FALSE );	/* 親ウィンドウのタイトルを更新 */
	BOOL OpenPropertySheet( int/*, int*/ );	/* 共通設定 */
	BOOL OpenPropertySheetTypes( int, int );	/* タイプ別設定 */


	BOOL OpenFileDialog( HWND, const char*, char*, int*, BOOL* );	/* 「ファイルを開く」ダイアログ */
	void OnChangeSetting( void );	/* ビューに設定変更を反映させる */
	void SetReferer( HWND , int, int );	/* タグジャンプ元など参照元の情報を保持する */
	BOOL SaveFileDialog( char*, int*, CEOL* pcEol = NULL );	/* 「ファイル名を付けて保存」ダイアログ */

	void CheckFileTimeStamp( void );	/* ファイルのタイムスタンプのチェック処理 */
	void ReloadCurrentFile( BOOL, BOOL );/* 同一ファイルの再オープン */

	//	May 15, 2000 genta
	CEOL  GetNewLineCode() const { return m_cNewLineCode; }
	void  SetNewLineCode(const CEOL& t){ m_cNewLineCode = t; }

	//	Aug. 14, 2000 genta
	bool IsModificationForbidden( int nCommand );

	//	Aug. 21, 2000 genta
	CPassiveTimer	m_cAutoSave;	//!<	自動保存管理
	void	CheckAutoSave(void);
	void	ReloadAutoSaveParam(void);	//	設定をSharedAreaから読み出す

	//	Aug. 31, 2000 genta
	const CEditView& ActiveView(void) const { return m_cEditViewArr[m_nActivePaneIndex]; }
	//	Nov. 20, 2000 genta
	void SetImeMode(int mode);	//	IME状態の設定

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


protected:
	int				m_nSettingType;
	bool			m_nSettingTypeLocked;	//	文書種別の一時設定状態
	//	Jan. 22, 2002 genta public -> protectedに移動
	/*! 変更フラグ
		@date 2002.01.22 genta public→protectedに移動．型をint→boolに．
	*/
	bool			m_bIsModified;

public: /* テスト用にアクセス属性を変更 */
	/* 入力補完 */
	CHokanMgr		m_cHokanMgr;
	BOOL			m_bGrepRunning;				/* Grep処理中 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
//	BOOL			m_bPrintPreviewMode;		/* 印刷プレビューモードか */
	int				m_nCommandExecNum;			/* コマンド実行回数 */
	char			m_szFilePath[_MAX_PATH];	/* 現在編集中のファイルのパス */
	FILETIME		m_FileTime;					/* ファイルの最終更新日付 */
	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;
	int				m_nCharCode;				/* 文字コード種別 */

	//	May 15, 2000 genta
	CEOL 			m_cNewLineCode;				//	Enter押下時に挿入する改行コード種別
	CEOL			m_cSaveLineCode;			//	保存時の改行コード種別（EOL_NONE:変換なし）


	BOOL			m_bReadOnly;				/* 読み取り専用モード */
	BOOL			m_bDebugMode;				/* デバッグモニタモード */
	BOOL			m_bGrepMode;				/* Grepモードか */
	char			m_szGrepKey[1024];			/* Grepモードの場合、その検索キー */
	HWND			m_hWnd;						/* 編集ウィンドウハンドル（CSplitterWndが管理） */
	COpeBuf			m_cOpeBuf;					/* アンドゥバッファ */
public:
	void			MakeFuncList_C( CFuncInfoArr* );		/* C/C++関数リスト作成 */
	void 			MakeFuncList_PLSQL( CFuncInfoArr* );	/* PL/SQL関数リスト作成 */
	void 			MakeTopicList_txt( CFuncInfoArr* );		/* テキスト・トピックリスト作成 */
	void			MakeFuncList_Java( CFuncInfoArr* );		/* Java関数リスト作成 */
	void			MakeTopicList_cobol( CFuncInfoArr* );	/* COBOL アウトライン解析 */
	void			MakeTopicList_asm( CFuncInfoArr* );		/* アセンブラ アウトライン解析 */
	void			MakeFuncList_Perl( CFuncInfoArr* );		/* Perl関数リスト作成 */	//	Sep. 8, 2000 genta
	void			MakeFuncList_VisualBasic( CFuncInfoArr* );/* Visual Basic関数リスト作成 */ //June 23, 2001 N.Nakatani
	void			MakeFuncList_RuleFile( CFuncInfoArr* );	/* ルールファイルを使ってリスト作成 2002.04.01 YAZAKI */
	int				ReadRuleFile( char* pszFilename, struct oneRule* pcOneRule );	/* ルールファイル読込 2002.04.01 YAZAKI */

// From Here 2001.12.03 hor
	void			MakeFuncList_BookMark( CFuncInfoArr* );	/* ブックマークリスト作成 */
// To Here 2001.12.03 hor

	CSplitterWnd	m_cSplitterWnd;				/* 分割フレーム */
	CEditView		m_cEditViewArr[4];			/* ビュー */
	int				m_nActivePaneIndex;			/* アクティブなビュー */
//	HWND			m_hwndActiveDialog;			/* アクティブな子ダイアログ */
	CDlgFind		m_cDlgFind;					/* 「検索」ダイアログ */
	CDlgReplace		m_cDlgReplace;				/* 「置換」ダイアログ */
	CDlgJump		m_cDlgJump;					/* 「指定行へジャンプ」ダイアログ */
//	CDlgSendMail	m_cDlgSendMail;				/* メール送信ダイアログ */
	CDlgGrep		m_cDlgGrep;					/* Grepダイアログ */
	CDlgFuncList	m_cDlgFuncList;				/* アウトライン解析結果ダイアログ */
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
	char			m_szDefaultWildCard[_MAX_PATH + 1];	/* 「開く」での最初のワイルドカード */
	char			m_szInitialDir[_MAX_PATH + 1];		/* 「開く」での初期ディレクトリ */
	OPENFILENAME	m_ofn;							/* 「ファイルを開く」ダイアログ用構造体 */
	CHOOSEFONT		m_cf;				/* フォント選択ダイアログ用 */

//@@	CProp1			m_cProp1;			/* 設定プロパティシート */
	CPropCommon		m_cPropCommon;
	CPropTypes		m_cPropTypes;

	int				m_nFileShareModeOld;	/* ファイルの排他制御モード */
	HFILE			m_hLockedFile;			/* ロックしているファイルのハンドル */

	HWND			m_hwndReferer;	/* 参照元ウィンドウ */
	int				m_nRefererX;	/* 参照元 行頭からのバイト位置桁 */
	int				m_nRefererLine;	/* 参照元行 折り返し無しの物理行位置 */
	
	//	Sep. 29, 2001 genta
	CSMacroMgr*		m_pcSMacroMgr;	//!< マクロ
//@@@ 2002.2.2 YAZAKI マクロをCSMacroMgrに統一。
//	CKeyMacroMgr	m_CKeyMacroMgr;	//	キーボードマクロ @@@ 2002.1.24 YAZAKI DLLSHAREDATAから移動
	//	Oct. 2, 2001 genta
	CFuncLookup		m_cFuncLookup;	//!< 機能名，機能番号などのresolve

//	CDlgTest*		m_pcDlgTest;

	/*
	||  実装ヘルパ関数
	*/
	void ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen);	//	pszSourceを展開して、pszBufferにコピー。
protected:
	void DoFileLock( void );	/* ファイルの排他ロック */
	void DoFileUnLock( void );	/* ファイルの排他ロック解除 */
	//char			m_pszCaption[1024];	//@@@ YAZAKI
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */


/*[EOF]*/
