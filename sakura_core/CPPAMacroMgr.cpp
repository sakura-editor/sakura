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
#include "StdAfx.h"
#include "CPPAMacroMgr.h"
#include "CMemory.h"
#include "CMacroFactory.h"
#include <string.h>

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
void CPPAMacroMgr::ExecKeyMacro( CEditView* pcEditView, int flags ) const
{
	m_cPPA.SetSource( m_cBuffer.GetStringPtr() );
	m_cPPA.Execute(pcEditView, flags);
}

/*! キーボードマクロをファイルから読み込む
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CPPAMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const char* pszPath )
{
	FILE* hFile = fopen( pszPath, "r" );
	if( NULL == hFile ){
		m_nReady = false;
		return FALSE;
	}

	CMemory cmemWork;

	// バッファ（cmemWork）にファイル内容を読み込み、m_cPPAに渡す。
	char	szLine[LINEREADBUFSIZE];	//	1行がLINEREADBUFSIZE以上だったら無条件にアウト
	while( NULL != fgets( szLine, sizeof(szLine), hFile ) ){
		int nLineLen = strlen( szLine );
		cmemWork.AppendString(szLine, nLineLen);
	}
	fclose( hFile );

	m_cBuffer.SetNativeData( &cmemWork );	//	m_cBufferにコピー

	m_nReady = true;
	return TRUE;
}

/*! キーボードマクロを文字列から読み込む
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CPPAMacroMgr::LoadKeyMacroStr( HINSTANCE hInstance, const char* pszCode )
{
	m_cBuffer.SetString( pszCode, strlen(pszCode) );	//	m_cBufferにコピー

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
CMacroManagerBase* CPPAMacroMgr::Creator(const char* ext)
{
	if( strcmp( ext, "ppa" ) == 0 ){
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
	if( m_cPPA.Init() ){
		CMacroFactory::Instance()->RegisterCreator( Creator );
	}
}
//	To Here Apr. 29, 2002 genta

/*[EOF]*/
