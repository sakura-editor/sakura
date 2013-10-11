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

class CEditApp;

#ifndef _CEDITAPP_H_
#define _CEDITAPP_H_
class CEditWnd;
class CSMacroMgr;

#include <windows.h>
#include "CShareData.h"
#include "CImageListMgr.h" // 2002/2/10 aroka

//!	常駐部の管理
/*!
	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CEditApp
{
public:
	static CEditApp* getInstance(){
		static CEditApp instance;

		return &instance;
	}

	void Create(HINSTANCE hInst, HWND hwndParent, int nGroupId);

private:
	// 外から作らせない。
	CEditApp(){}
	CEditApp(CEditApp const&);
	void operator=(CEditApp const&);

public:
	~CEditApp();

	//ウィンドウ
	CEditWnd*			m_pcEditWnd;

	CSMacroMgr*			m_pcSMacroMgr;			//マクロ管理
	//GUIオブジェクト
	CImageListMgr		m_cIcons;					//!< Image List
};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITAPP_H_ */


/*[EOF]*/
