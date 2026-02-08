/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
class CEditApp : public TSingleton<CEditApp>{
	friend class TSingleton<CEditApp>;
	CEditApp(){}
	virtual ~CEditApp();

public:
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
private:
	CPropertyManager*	m_pcPropertyManager;	//プロパティ管理
public:
	CGrepAgent*			m_pcGrepAgent;			//GREPモード
	CSoundSet			m_cSoundSet;			//サウンド管理

	//GUIオブジェクト
	CImageListMgr		m_cIcons;					//!< Image List
};

//WM_QUIT検出例外
class CAppExitException : public std::exception{
public:
	const char* what() const throw() override{ return "CAppExitException"; }
};
#endif /* SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_ */
