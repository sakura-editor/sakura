/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_CEDITAPP_935DB250_AEB5_40A5_BCFF_3B72F8E3D8339_H_
#define SAKURA_CEDITAPP_935DB250_AEB5_40A5_BCFF_3B72F8E3D8339_H_

//2007.10.23 kobake 作成

#include "util/design_template.h"
#include "uiparts/CSoundSet.h"
#include "uiparts/CImageListMgr.h"
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
	const char* what() const throw(){ return "CAppExitException"; }
};


#endif /* SAKURA_CEDITAPP_935DB250_AEB5_40A5_BCFF_3B72F8E3D8339_H_ */
/*[EOF]*/
