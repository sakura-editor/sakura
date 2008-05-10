/*!	@file
	@brief キーボードマクロ

	@author YAZAKI
	@date 2002年1月26日
*/
/*
	Copyright (C) 2002, YAZAKI, genta
	Copyright (C) 2004, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "CPPAMacroMgr.h"
#include "CPPA.h"
#include "mem/CMemory.h"
#include "CMacroFactory.h"
#include <string.h>
#include "io/CTextStream.h"
using namespace std;

CPPA CPPAMacroMgr::m_cPPA;

CPPAMacroMgr::CPPAMacroMgr()
{
}

CPPAMacroMgr::~CPPAMacroMgr()
{
}

/*! キーボードマクロの実行
	PPA.DLLに、バッファ内容を渡して実行。
*/
void CPPAMacroMgr::ExecKeyMacro( CEditView* pcEditView ) const
{
	m_cPPA.SetSource( to_achar(m_cBuffer.GetStringPtr()) );
	m_cPPA.Execute(pcEditView);
}

/*! キーボードマクロの読み込み
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CPPAMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath )
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

//	From Here Apr. 29, 2002 genta
/*!
	@brief Factory

	@param ext [in] オブジェクト生成の判定に使う拡張子(小文字)

	@date 2004.01.31 genta RegisterExtの廃止のためRegisterCreatorに置き換え
		そのため，過ったオブジェクト生成を行わないために拡張子チェックは必須．

*/
CMacroManagerBase* CPPAMacroMgr::Creator(const TCHAR* ext)
{
	if( _tcscmp( ext, _T("ppa") ) == 0 ){
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
		CMacroFactory::Instance()->RegisterCreator( Creator );
	}
}
//	To Here Apr. 29, 2002 genta


