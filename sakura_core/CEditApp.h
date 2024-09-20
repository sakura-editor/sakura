/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#define SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_
#pragma once

#include "_main/CNormalProcess.h"

#include "uiparts/CSoundSet.h"

#include "util/design_template.h"

//!エディタ部分アプリケーションクラス。CNormalProcess1個につき、1個存在。
//2007.10.23 kobake 作成
class CEditApp : public TSingleInstance<CEditApp> {
public:
	//モジュール情報
	HINSTANCE       GetAppInstance() const{ return getEditorProcess()->GetProcessInstance(); }

	//ウィンドウ情報
	CEditWnd*       GetEditWindow() const { return m_pcEditWnd; }

	CEditDoc*		GetDocument() const { return getEditorProcess()->GetEditDoc(); }
	CAppMode*       GetAppMode() const { return getEditorProcess()->GetAppMode(); }
	CSMacroMgr*     GetSMacroMgr() const { return m_pcSMacroMgr; }

	bool OpenPropertySheet( int nPageNum );
	bool OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType );

	//ウィンドウ
	CEditWnd*			m_pcEditWnd = nullptr;

	CVisualProgress*    m_pcVisualProgress = getEditorProcess()->GetProgress();

	CPropertyManager*	m_pcPropertyManager = nullptr;	//プロパティ管理

	CSoundSet			m_cSoundSet;			//サウンド管理

	CGrepAgent*         m_pcGrepAgent = getEditorProcess()->GetGrepAgent();
	CSMacroMgr*         m_pcSMacroMgr = getEditorProcess()->GetSMacroMgr();
};

//WM_QUIT検出例外
class CAppExitException : public std::exception{
public:
	const char* what() const throw(){ return "CAppExitException"; }
};

#endif /* SAKURA_CEDITAPP_421797BC_DD8E_4209_AAF7_6BDC4D1CAAE9_H_ */
