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

#ifndef SAKURA_CEDITDOC_CE42530D_FEC1_4B51_9CA3_470856295FEF8_H_
#define SAKURA_CEDITDOC_CE42530D_FEC1_4B51_9CA3_470856295FEF8_H_


#include "_main/global.h"
#include "_main/CAppMode.h"
#include "CDocEditor.h"
#include "CDocFile.h"
#include "CDocFileOperation.h"
#include "CDocType.h"
#include "CDocOutline.h"
#include "CDocLocker.h"
#include "layout/CLayoutMgr.h"
#include "logic/CDocLineMgr.h"
#include "CBackupAgent.h"
#include "CAutoSaveAgent.h"
#include "CAutoReloadAgent.h"
#include "func/CFuncLookup.h"
#include "CEol.h"
#include "macro/CCookieManager.h"
#include "util/design_template.h"

class CSMacroMgr; // 2002/2/10 aroka
class CEditWnd; // Sep. 10, 2002 genta
struct EditInfo; // 20050705 aroka
class CFuncInfoArr;
class CEditApp;

/*!
	文書関連情報の管理

	@date 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2007.12.13 kobake GetDocumentEncoding作成
	@date 2007.12.13 kobake SetDocumentEncoding作成
	@date 2007.12.13 kobake IsViewMode作成
*/
class CEditDoc
: public CDocSubject
, public TInstanceHolder<CEditDoc>
{
public:
	//コンストラクタ・デストラクタ
	CEditDoc(CEditApp* pcApp);
	~CEditDoc();

	//初期化
	BOOL Create( CEditWnd* pcEditWnd );
	void InitDoc();	/* 既存データのクリア */
	void InitAllView();	/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */
	void Clear();

	//設定
	void SetFilePathAndIcon(const TCHAR* szFile);	// Sep. 9, 2002 genta

	//属性
	ECodeType	GetDocumentEncoding() const;				//!< ドキュメントの文字コードを取得
	bool		GetDocumentBomExist() const;				//!< ドキュメントのBOM付加を取得
	void		SetDocumentEncoding(ECodeType eCharCode, bool bBom);	//!< ドキュメントの文字コードを設定
	bool IsModificationForbidden( EFunctionCode nCommand ) const;	//!< 指定コマンドによる書き換えが禁止されているかどうか	//Aug. 14, 2000 genta
	bool IsEditable() const { return !CAppMode::getInstance()->IsViewMode() && !(!m_cDocLocker.IsDocWritable() && GetDllShareData().m_Common.m_sFile.m_bUneditableIfUnwritable); }	//!< 編集可能かどうか
	void GetSaveInfo(SSaveInfo* pSaveInfo) const;			//!< セーブ情報を取得

	//状態
	void GetEditInfo( EditInfo* ) const;	//!< 編集ファイル情報を取得 //2007.10.24 kobake 関数名変更: SetFileInfo→GetEditInfo
	bool IsAcceptLoad() const;				//!< このウィンドウで(新しいウィンドウを開かずに)新しいファイルを開けるか

	//イベント
	BOOL HandleCommand( EFunctionCode );
	void OnChangeType();
	void OnChangeSetting(bool bDoLayout = true, bool bBlockingHook = true);		// ビューに設定変更を反映させる
	BOOL OnFileClose(bool);			/* ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行 */

	void RunAutoMacro( int idx, LPCTSTR pszSaveFilePath = NULL );	// 2006.09.01 ryoji マクロ自動実行

	void SetBackgroundImage();

	void SetCurDirNotitle();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//参照
	CEditWnd*		m_pcEditWnd;	//	Sep. 10, 2002

	//データ構造
	CDocLineMgr		m_cDocLineMgr;
	CLayoutMgr		m_cLayoutMgr;

	//各種機能
public:
	CDocFile			m_cDocFile;
	CDocFileOperation	m_cDocFileOperation;
	CDocEditor			m_cDocEditor;
	CDocType			m_cDocType;
	CCookieManager		m_cCookie;

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
	LOGFONT			m_lfCur;					// 一時設定フォント
	int				m_nPointSizeCur;			// 一時設定フォントサイズ
	bool			m_blfCurTemp;				// フォント設定適用中
	int				m_nPointSizeOrg;			// 元のフォントサイズ
	bool			m_bTabSpaceCurTemp;			// タブ幅一時設定適用中			// 2013.05.30 Moca

	HBITMAP			m_hBackImg;
	int				m_nBackImgWidth;
	int				m_nBackImgHeight;
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CEDITDOC_CE42530D_FEC1_4B51_9CA3_470856295FEF8_H_ */
