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
#include "CAutoSaveAgent.h"
#include "CDocLineMgr.h"
#include "CEol.h"
#include "CFuncLookup.h"
#include "CLayoutMgr.h"
#include "COpeBuf.h"
#include "CShareData.h"
#include <string>
#include "CDocListener.h"
#include "util/design_template.h"
#include "CBackupAgent.h"
#include "CAutoReloadAgent.h"
#include "util/file.h" //CFileTime
#include "CFile.h"
#include "CDocEditor.h"
#include "CDocFile.h"
#include "CDocFileOperation.h"
#include "CDocOutline.h"
#include "CDocType.h"
#include "CDocLocker.h"

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
	/*
	||  Constructors
	*/
	CEditDoc(CEditApp* pcApp);
	~CEditDoc();

	/*
	||  初期化系メンバ関数
	*/
	BOOL Create( CImageListMgr* );
	void InitDoc();	/* 既存データのクリア */
	void InitAllView();	/* 全ビューの初期化：ファイルオープン/クローズ時等に、ビューを初期化する */

	/*
	|| 状態
	*/
	void GetEditInfo( EditInfo* ) const;	/* 編集ファイル情報を格納 */ //2007.10.24 kobake 関数名変更: SetFileInfo→GetEditInfo
	void GetSaveInfo(SSaveInfo* pSaveInfo) const;

	/*
	|| 属性
	*/
	ECodeType	GetDocumentEncoding() const;				//!< ドキュメントの文字コードを取得
	void		SetDocumentEncoding(ECodeType eCharCode);	//!< ドキュメントの文字コードを設定

	/* いろいろ */
	BOOL OnFileClose();	/* ファイルを閉じるときのMRU登録 & 保存確認 ＆ 保存実行 */
	BOOL HandleCommand( EFunctionCode );
	BOOL SelectFont( LOGFONT* );



	void OnChangeSetting( void );	/* ビューに設定変更を反映させる */



	//	Aug. 14, 2000 genta
	bool IsModificationForbidden( int nCommand );

	/*! @brief このウィンドウで新しいファイルを開けるか

		新しいウィンドウを開かずに現在のウィンドウを再利用できるかどうかのテストを行う．
		変更済み，ファイルを開いている，Grepウィンドウ，アウトプットウィンドウの場合には
		再利用不可．

		@author Moca
		@date 2005.06.24 Moca
	*/
	bool IsAcceptLoad() const;

	void SetFilePathAndIcon(const TCHAR* szFile);	// Sep. 9, 2002 genta

public:
	HWND GetSplitterHwnd() const;
	HWND GetOwnerHwnd() const;

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
};



///////////////////////////////////////////////////////////////////////
#endif /* _CEDITDOC_H_ */


/*[EOF]*/
