/*!	@file
	@brief キーボードマクロ

	@author YAZAKI

*/
/*
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CPPAMacroMgr.h"
#include "CPPA.h"
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

/*! キーボードマクロの実行
	PPA.DLLに、バッファ内容を渡して実行。
*/
void CPPAMacroMgr::ExecKeyMacro( CEditView* pcEditView ) const
{
	m_cPPA.SetSource( m_cBuffer.GetPtr() );
	m_cPPA.Execute(pcEditView);
}

/*! キーボードマクロの読み込み
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
	char	szLine[10240];	//	1行が10240以上だったら無条件にアウト
	while( NULL != fgets( szLine, sizeof(szLine), hFile ) ){
		int nLineLen = strlen( szLine );
		cmemWork.Append(szLine, nLineLen);
	}
	fclose( hFile );

	m_cBuffer.SetData( &cmemWork );	//	m_cBufferにコピー

	m_nReady = true;
	return TRUE;
}

//	From Here Apr. 29, 2002 genta
/*!
	@brief Factory

	拡張子は特に使わない。
*/
CMacroManagerBase* CPPAMacroMgr::Creator(const char*)
{
	return new CPPAMacroMgr;
}

/*!	CPPAMacroManagerの登録

	PPAが利用できないときは何もしない。
*/
void CPPAMacroMgr::declare (void)
{
	if( m_cPPA.Init() ){
		CMacroFactory::Instance()->Register("ppa", Creator);
	}
}
//	To Here Apr. 29, 2002 genta

/*[EOF]*/
