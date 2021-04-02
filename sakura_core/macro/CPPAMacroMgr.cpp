﻿/*!	@file
	@brief キーボードマクロ

	@author YAZAKI
	@date 2002年1月26日
*/
/*
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2004, genta
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CPPAMacroMgr.h"
#include "mem/CNativeW.h"
#include "CMacroFactory.h"
#include <string.h>
#include "io/CTextStream.h"
#include "util/tchar_convert.h"

using namespace std;

CPPA CPPAMacroMgr::m_cPPA;

CPPAMacroMgr::CPPAMacroMgr()
{
}

CPPAMacroMgr::~CPPAMacroMgr()
{
}

/** PPAマクロの実行

	PPA.DLLに、バッファ内容を渡して実行。

	@date 2007.07.20 genta flags追加
*/
bool CPPAMacroMgr::ExecKeyMacro( CEditView* pcEditView, int flags ) const
{
	m_cPPA.SetSource( to_achar(m_cBuffer.GetStringPtr()) );
	return m_cPPA.Execute(pcEditView, flags);
}

/*! キーボードマクロの読み込み（ファイルから）
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CPPAMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const WCHAR* pszPath )
{
	CTextInputStream in( pszPath );
	if(!in){
		m_nReady = false;
		return FALSE;
	}

	CNativeW cmemWork;

	// バッファ（cmemWork）にファイル内容を読み込み、m_cPPAに渡す。
	while( in ){
		wstring szLine = in.ReadLineW();
		szLine += L"\n";
		cmemWork.AppendString(szLine.c_str());
	}
	in.Close();

	m_cBuffer.SetNativeData( cmemWork );	//	m_cBufferにコピー

	m_nReady = true;
	return TRUE;
}

/*! キーボードマクロの読み込み（文字列から）
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CPPAMacroMgr::LoadKeyMacroStr( HINSTANCE hInstance, const WCHAR* pszCode )
{
	m_cBuffer.SetString( pszCode );	//	m_cBufferにコピー

	m_nReady = true;
	return TRUE;
}

//	From Here Apr. 29, 2002 genta
/*!
	@brief Factory

	@param ext [in] オブジェクト生成の判定に使う拡張子(小文字)

	@date 2004.01.31 genta RegisterExtの廃止のためRegisterCreatorに置き換え
		そのため，過ったオブジェクト生成を行わないために拡張子チェックは必須．

*/
CMacroManagerBase* CPPAMacroMgr::Creator(const WCHAR* ext)
{
	if( wcscmp( ext, L"ppa" ) == 0 ){
		return new CPPAMacroMgr;
	}
	return NULL;
}

/*!	CPPAMacroManagerの登録

	PPAが利用できないときは何もしない。

	@date 2004.01.31 genta RegisterExtの廃止のためRegisterCreatorに置き換え
*/
void CPPAMacroMgr::declare (void)
{
	if( DLL_SUCCESS == m_cPPA.InitDll() ){
		CMacroFactory::getInstance()->RegisterCreator( Creator );
	}
}
//	To Here Apr. 29, 2002 genta
