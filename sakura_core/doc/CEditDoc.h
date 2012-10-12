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
#include "CLayoutMgr.h"
#include "CDocLineMgr.h"
#include "CBackupAgent.h"
#include "CAutoSaveAgent.h"
#include "CAutoReloadAgent.h"
#include "CDocEditor.h"
#include "CDocFile.h"
#include "CDocFileOperation.h"
#include "CDocType.h"
#include "CDocOutline.h"
#include "CDocLocker.h"
#include "func/CFuncLookup.h"
#include "CEol.h"
#include "util/design_template.h"
#include "CAppMode.h"

class CImageListMgr; // 2002/2/10 aroka
class CSMacroMgr; // 2002/2/10 aroka
class CEditWnd; // Sep. 10, 2002 genta
struct EditInfo; // 20050705 aroka
class CFuncInfoArr;
class CEditApp;

typedef CEditWnd* P_CEditWnd;
/*!
	文書関連情報の管理

	@date 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2007.12.13 kobake GetDocumentEncoding作成
	@date 2007.12.13 kobake SetDocumentEncoding作成
	@date 2007.12.13 kobake IsViewMode作成
*/
class SAKURA_CORE_API CEditDoc
: public CDocSubject
, public TInstanceHolder<CEditDoc>
{
public:
	//コンストラクタ・デストラクタ
	CEditDoc(CEditApp* pcApp);
	~CEditDoc();

	//初期化
	BOOL Create( CImageListMgr* );
	void InitDoc();	/* 既存データのクリア */
	void InitAllView();	/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */
	void Clear();

	//設定
	void SetFilePathAndIcon(const TCHAR* szFile);	// Sep. 9, 2002 genta

	//属性
	ECodeType	GetDocumentEncoding() const;				//!< ドキュメントの文字コードを取得
	void		SetDocumentEncoding(ECodeType eCharCode);	//!< ドキュメントの文字コードを設定
	bool IsModificationForbidden( EFunctionCode nCommand );	//!< 指定コマンドによる書き換えが禁止されているかどうか	//Aug. 14, 2000 genta
	bool IsEditable() const { return !CAppMode::getInstance()->IsViewMode() && !(!m_cDocLocker.IsDocWritable() && GetDllShareData().m_Common.m_sFile.m_bUneditableIfUnwritable); }	//!< 編集可能かどうか
	void GetSaveInfo(SSaveInfo* pSaveInfo) const;			//!< セーブ情報を取得

	//状態
	void GetEditInfo( EditInfo* ) const;	//!< 編集ファイル情報を取得 //2007.10.24 kobake 関数名変更: SetFileInfo→GetEditInfo
	bool IsAcceptLoad() const;				//!< このウィンドウで(新しいウィンドウを開かずに)新しいファイルを開けるか

	//イベント
	BOOL HandleCommand( EFunctionCode );
	void OnChangeType();
	void OnChangeSetting();		/* ビューに設定変更を反映させる */
	BOOL OnFileClose();			/* ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行 */

	void RunAutoMacro( int idx, LPCTSTR pszSaveFilePath = NULL );	// 2006.09.01 ryoji マクロ自動実行

	void SetBackgroundImage();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//参照
	P_CEditWnd&		m_pcEditWnd;	//	Sep. 10, 2002

	//データ構造
	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;

	//各種機能
public:
	CDocFile			m_cDocFile;
	CDocFileOperation	m_cDocFileOperation;
	CDocEditor			m_cDocEditor;
	CDocType			m_cDocType;

	//ヘルパ
public:
	CBackupAgent		m_cBackupAgent;
	CAutoSaveAgent		m_cAutoSaveAgent;		//!< 自動保存管理
	CAutoReloadAgent	m_cAutoReloadAgent;
	CDocOutline			m_cDocOutline;
	CDocLocker			m_cDocLocker;

	//動的状態
public:
	int				m_nCommandExecNum;			//!< コマンド実行回数

	//環境情報
public:
	CFuncLookup		m_cFuncLookup;				//!< 機能名，機能番号などのresolve

	//未整理変数
public:
	int				m_nTextWrapMethodCur;		// 折り返し方法					// 2008.05.30 nasukoji
	bool			m_bTextWrapMethodCurTemp;	// 折り返し方法一時設定適用中	// 2008.05.30 nasukoji

	HBITMAP			m_hBackImg;
	int				m_nBackImgWidth;
	int				m_nBackImgHeight;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */



