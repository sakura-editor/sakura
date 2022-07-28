﻿/*!	@file
	@brief WSH Manager

	@date 2009.10.29 syat CWSH.cppから切り出し
*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2003, FILE
	Copyright (C) 2004, genta
	Copyright (C) 2005, FILE, zenryaku
	Copyright (C) 2009, syat
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

#include "StdAfx.h"
#include "macro/CWSHManager.h"
#include "macro/CWSH.h"
#include "macro/CEditorIfObj.h"
#include "view/CEditView.h"
#include "io/CTextStream.h"
#include "util/os.h"
#include "macro/CMacroFactory.h"

static void MacroError(BSTR Description, BSTR Source, void *Data)
{
	CEditView *View = reinterpret_cast<CEditView*>(Data);
	MessageBox(View->GetHwnd(), Description, Source, MB_ICONERROR);
}

CWSHMacroManager::CWSHMacroManager(std::wstring const AEngineName) : m_EngineName(AEngineName)
{
}

CWSHMacroManager::~CWSHMacroManager()
{
}

/** WSHマクロの実行

	@param EditView [in] 操作対象EditView
	
	@date 2007.07.20 genta : flags追加
*/
bool CWSHMacroManager::ExecKeyMacro(CEditView *EditView, int flags) const
{
	CWSHClient* Engine;
	Engine = new CWSHClient(m_EngineName.c_str(), MacroError, EditView);
	bool bRet = false;
	if(Engine->m_Valid)
	{
		//インターフェースオブジェクトの登録
		CWSHIfObj* objEditor = new CEditorIfObj();
		objEditor->ReadyMethods( EditView, flags );
		Engine->AddInterfaceObject( objEditor );
		for( CWSHIfObj::ListIter it = m_Params.begin(); it != m_Params.end(); it++ ) {
			(*it)->ReadyMethods( EditView, flags );
			Engine->AddInterfaceObject(*it);
		}

		bRet = Engine->Execute(m_Source.c_str());
	}
	delete Engine;
	return bRet;
}

/*!
	WSHマクロの読み込み（ファイルから）

	@param hInstance [in] インスタンスハンドル(未使用)
	@param pszPath   [in] ファイルのパス
*/
BOOL CWSHMacroManager::LoadKeyMacro(HINSTANCE hInstance, const WCHAR* pszPath)
{
	//ソース読み込み -> m_Source
	m_Source.clear();
	
	CTextInputStream in(pszPath);
	if(!in)
		return FALSE;

	while(in){
		m_Source+=in.ReadLineW()+L"\r\n";
	}
	return TRUE;
}

/*!
	WSHマクロの読み込み（文字列から）

	@param hInstance [in] インスタンスハンドル(未使用)
	@param pszCode   [in] マクロコード
*/
BOOL CWSHMacroManager::LoadKeyMacroStr(HINSTANCE hInstance, const WCHAR* pszCode)
{
	//ソース読み込み -> m_Source
	m_Source = pszCode;
	return TRUE;
}

CMacroManagerBase* CWSHMacroManager::Creator(const WCHAR* FileExt)
{
	WCHAR FileExtWithDot[1024], FileType[1024], EngineName[1024]; //1024を超えたら後は知りません
	
	wcscpy( FileExtWithDot, L"." );
	wcscat( FileExtWithDot, FileExt );

	if(ReadRegistry(HKEY_CLASSES_ROOT, FileExtWithDot, NULL, FileType, 1024))
	{
		lstrcat(FileType, L"\\ScriptEngine");
		if(ReadRegistry(HKEY_CLASSES_ROOT, FileType, NULL, EngineName, 1024))
		{
			return new CWSHMacroManager(EngineName);
		}
	}
	return NULL;
}

void CWSHMacroManager::declare()
{
	//暫定
	CMacroFactory::getInstance()->RegisterCreator(Creator);
}

//インターフェースオブジェクトを追加する
void CWSHMacroManager::AddParam( CWSHIfObj* param )
{
	m_Params.push_back( param );
}

//インターフェースオブジェクト達を追加する
void CWSHMacroManager::AddParam( CWSHIfObj::List& params )
{
	m_Params.insert( m_Params.end(), params.begin(), params.end() );
}

//インターフェースオブジェクトを削除する
void CWSHMacroManager::ClearParam()
{
	m_Params.clear();
}
