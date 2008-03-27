//2007.10.23 kobake 作成

#pragma once

#include "util/design_template.h"
class CEditWnd;
class CPropertyManager;
class CMruListener;
enum EFunctionCode;
#include "CEditDoc.h"
#include "CImageListMgr.h"
#include "CPropertyManager.h"
#include "CSMacroMgr.h"
#include "CGrepAgent.h"
#include "CSoundSet.h"
#include "CSaveAgent.h"
#include "CLoadAgent.h"
#include "CVisualProgress.h"

//!エディタ部分アプリケーションクラス。CNormalProcess1個につき、1個存在。
// 2007.10.30 kobake OnHelp_MenuItemをCEditWndから持ってきた
// 2007.10.30 kobake 関数名変更: OnHelp_MenuItem→ShowFuncHelp
class CEditApp : public TSingleInstance<CEditApp>{
public:
	//コンストラクタ・デストラクタ
	CEditApp(HINSTANCE hInst);
	virtual ~CEditApp();

	//モジュール情報
	HINSTANCE GetAppInstance() const{ return m_hInst; }	//!< インスタンスハンドル取得

	//ウィンドウ情報
	CEditWnd* GetWindow(){ return m_pcEditWnd; }		//!< ウィンドウ取得

	//パス情報
	LPCTSTR GetHelpFilePath() const;					//!< ヘルプファイルのフルパスを返す

	//ヘルプ関連
	void ShowFuncHelp( HWND, EFunctionCode ) const;					//!< メニューアイテムに対応するヘルプを表示

	CEditDoc&		GetDocument(){ return *m_pcEditDoc; }
	CImageListMgr&	GetIcons(){ return m_cIcons; }


public:
	HINSTANCE			m_hInst;

	//ドキュメント
	CEditDoc*			m_pcEditDoc;

	//ウィンドウ
	CEditWnd*			m_pcEditWnd;

	//IO管理
	CLoadAgent*			m_pcLoadAgent;
	CSaveAgent*			m_pcSaveAgent;
	CVisualProgress*	m_pcVisualProgress;

	//その他ヘルパ
	CMruListener*		m_pcMruListener;		//MRU管理
	CSMacroMgr*			m_pcSMacroMgr;			//マクロ管理
	CPropertyManager*	m_pcPropertyManager;	//プロパティ管理
	CGrepAgent*			m_pcGrepAgent;			//GREPモード
	CSoundSet			m_cSoundSet;			//サウンド管理

	//GUIオブジェクト
	CImageListMgr		m_cIcons;					//!< Image List
};


//WM_QUIT検出例外
class CAppExitException : public std::exception{
public:
	const char* what() const throw(){ return "CAppExitException"; }
};

