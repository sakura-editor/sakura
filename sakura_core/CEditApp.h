//2007.10.23 kobake 作成

#pragma once

#include "util/design_template.h"
class CEditWnd;
enum EFunctionCode;
#include "CEditDoc.h"
#include "CImageListMgr.h"

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

	CEditDoc&		GetDocument(){ return m_cEditDoc; }
	CImageListMgr&	GetIcons(){ return m_cIcons; }


public:
	HINSTANCE		m_hInst;

	//ウィンドウ
	CEditWnd*		m_pcEditWnd;

	//ドキュメント
	CEditDoc		m_cEditDoc;

	//ヘルパ
	CImageListMgr	m_cIcons;					//!< Image List
};
