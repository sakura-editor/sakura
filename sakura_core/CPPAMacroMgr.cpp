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

CPPA CPPAMacroMgr::m_cPPA;

CPPAMacroMgr::CPPAMacroMgr()
: CKeyMacroMgr()
{
}

CPPAMacroMgr::~CPPAMacroMgr()
{
}

/*! キーボードマクロの実行
	CMacroに委譲。
*/
void CPPAMacroMgr::ExecKeyMacro( CEditView* pcEditView ) const
{
	m_cPPA.Execute(pcEditView);
}

/*! キーボードマクロの読み込み
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CPPAMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const char* pszPath )
{
	FILE* hFile = fopen( pszPath, "r" );
	if( NULL == hFile ){
		m_nReady = FALSE;
		return FALSE;
	}

	CMemory cmemWork;

	// 一行ずつ読みこみ、コメント行を排除した上で、macroコマンドを作成する。
	char	szLine[10240];	//	1行が10240以上だったら無条件にアウト
	while( NULL != fgets( szLine, sizeof(szLine), hFile ) ){
		int nLineLen = strlen( szLine );
		cmemWork.Append(szLine, nLineLen);
	}
	fclose( hFile );

	m_cPPA.SetSource( cmemWork.GetPtr2() );

	m_nReady = TRUE;
	return TRUE;
}


/*[EOF]*/
