/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#define SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#pragma once

//2007.10.23 kobake 作成

#include "util/design_template.h"
#include "uiparts/CSoundSet.h"
#include "uiparts/CImageListMgr.h"
#include "types/CType.h"

class CEditDoc;
class CEditWnd;
class CLoadAgent;
class CSaveAgent;
class CVisualProgress;
class CMruListener;
class CSMacroMgr;
class CPropertyManager;
class CGrepAgent;
enum EFunctionCode;

//!エディタ部分アプリケーションクラス。CNormalProcess1個につき、1個存在。
class CEditApp final : public TSakuraSingleton<CEditApp> {
public:
	CEditApp() = default;
	~CEditApp() noexcept;

	void Create(HINSTANCE hInst, int);

	//モジュール情報
	HINSTANCE GetAppInstance() const{ return m_hInst; }	//!< インスタンスハンドル取得

	//ウィンドウ情報
	CEditWnd* GetEditWindow(){ return m_pcEditWnd; }		//!< ウィンドウ取得

	CEditDoc*		GetDocument(){ return m_pcEditDoc; }
	CImageListMgr&	GetIcons(){ return m_cIcons; }

	bool OpenPropertySheet( int nPageNum );
	bool OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType );

public:
	HINSTANCE			m_hInst = nullptr;

	//ドキュメント
	CEditDoc*			m_pcEditDoc = nullptr;

	//ウィンドウ
	CEditWnd*			m_pcEditWnd = nullptr;

	//IO管理
	CLoadAgent*			m_pcLoadAgent = nullptr;
	CSaveAgent*			m_pcSaveAgent = nullptr;
	CVisualProgress*	m_pcVisualProgress = nullptr;

	//その他ヘルパ
	CMruListener*		m_pcMruListener = nullptr;		//!< MRU管理
	CSMacroMgr*			m_pcSMacroMgr = nullptr;		//!< マクロ管理

	CPropertyManager*	m_pcPropertyManager = nullptr;	//!< プロパティ管理

	CGrepAgent*			m_pcGrepAgent = nullptr;		//!< GREPモード
	CSoundSet			m_cSoundSet;					//!< サウンド管理

	//GUIオブジェクト
	CImageListMgr		m_cIcons;					//!< Image List
};

//WM_QUIT検出例外
class CAppExitException : public std::exception{
public:
	const char* what() const throw() override{ return "CAppExitException"; }
};

#endif /* SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_ */
