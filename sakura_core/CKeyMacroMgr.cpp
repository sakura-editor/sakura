//	$Id$
/*!	@file
	@brief キーボードマクロ

	@author Norio Nakatani

	@date 20011229 aroka バグ修正、コメント追加
	YAZAKI 組替え
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
//	#include <stdio.h>
//	#include <stdlib.h>
//	#include <malloc.h>
#include "CKeyMacroMgr.h"
#include "CMacro.h"
//	#include "debug.h"
#include "charcode.h"
//	#include "etc_uty.h"
//	#include "global.h"
//	#include "CEditView.h"

CKeyMacroMgr::CKeyMacroMgr()
{
	m_pTop = NULL;
	m_pBot = NULL;
	m_nKeyMacroDataArrNum = 0;
	m_nReady = FALSE;
	return;
}

CKeyMacroMgr::~CKeyMacroMgr()
{
	/* キーマクロのバッファをクリアする */
	ClearAll();
	return;
}


/*! キーマクロのバッファをクリアする */
void CKeyMacroMgr::ClearAll( void )
{
	CMacro* p = m_pTop;
	CMacro* del_p;
	while (p){
		del_p = p;
		p = p->GetNext();
		delete del_p;
	}
	m_nKeyMacroDataArrNum = 0;
	m_pTop = NULL;
	m_pBot = NULL;
	return;

}

/*! キーマクロのバッファにデータ追加
	機能番号と、引数ひとつを追加版。
*/
void CKeyMacroMgr::Append( int nFuncID, LPARAM lParam1 )
{
	CMacro* macro = new CMacro( nFuncID );
	macro->AddLParam( lParam1 );
	Append(macro);
}

/*! キーマクロのバッファにデータ追加
	CMacroを指定して追加する版
*/
void CKeyMacroMgr::Append( CMacro* macro )
{
	if (m_pTop){
		m_pBot->SetNext(macro);
		m_pBot = macro;
	}
	else {
		m_pTop = macro;
		m_pBot = m_pTop;
	}
	m_nKeyMacroDataArrNum++;
	return;
}



/*! キーボードマクロの保存
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CKeyMacroMgr::SaveKeyMacro( HINSTANCE hInstance, const char* pszPath ) const
{
	HFILE		hFile;
	char		szLine[1024];
	CMemory		cmemWork;
	hFile = _lcreat( pszPath, 0 );
	if( HFILE_ERROR == hFile ){
		return FALSE;
	}
	strcpy( szLine, "//キーボードマクロのファイル\r\n" );
	_lwrite( hFile, szLine, strlen( szLine ) );
	CMacro* p = m_pTop;

	while (p){
		p->Save( hInstance, hFile );
		p = p->GetNext();
	}
	_lclose( hFile );
	return TRUE;
}



/*! キーボードマクロの実行
	CMacroに委譲。
*/
void CKeyMacroMgr::ExecKeyMacro( CEditView* pcEditView ) const
{
	CMacro* p = m_pTop;
	while (p){
		p->Exec(pcEditView);
		p = p->GetNext();
	}
}

/*! キーボードマクロの読み込み
	エラーメッセージは出しません。呼び出し側でよきにはからってください。
*/
BOOL CKeyMacroMgr::LoadKeyMacro( HINSTANCE hInstance, const char* pszPath )
{
	/* キーマクロのバッファをクリアする */
	ClearAll();

	FILE* hFile = fopen( pszPath, "r" );
	if( NULL == hFile ){
		m_nReady = FALSE;
		return FALSE;
	}

	char	szFuncName[100];
	char	szFuncNameJapanese[256];
	int		nFuncID;
	int		i;
	int		nBgn, nEnd;
	CMemory cmemWork;
	CMacro* macro = NULL;

	// 一行ずつ読みこみ、コメント行を排除した上で、macroコマンドを作成する。
	char	szLine[10240];
	while( NULL != fgets( szLine, sizeof(szLine), hFile ) ){
		int nLineLen = strlen( szLine );
		// 先行する空白をスキップ
		for( i = 0; i < nLineLen; ++i ){
			if( szLine[i] != SPACE && szLine[i] != TAB ){
				break;
			}
		}
		nBgn = i;
		// コメント行の検出
		//# パフォーマンス：'/'のときだけ２文字目をテスト
		if( szLine[nBgn] == '/' && nBgn + 1 < nLineLen && szLine[nBgn + 1] == '/' ){
			continue;
		}
		// 関数名の取得
		szFuncName[0]='\0';// 初期化
		for( ; i < nLineLen; ++i ){
			//# バッファオーバーランチェック
			if( szLine[i] == '(' && (i - nBgn)< sizeof(szFuncName) ){
				memcpy( szFuncName, &szLine[nBgn], i - nBgn );
				szFuncName[i - nBgn] = '\0';
				++i;
				nBgn = i;
				break;
			}
		}

		/* 関数名→機能ID，機能名日本語 */
		nFuncID = CMacro::GetFuncInfoByName( hInstance, szFuncName, szFuncNameJapanese );
		if( -1 != nFuncID ){
			macro = new CMacro( nFuncID );
			//	Skip Space
			while (szLine[i]) {
				while( szLine[i] == ' ' || szLine[i] == '\t' )
					i++;

				if( '\"' == szLine[i] ){	//	"で始まったら文字列だよきっと。
					++i;
					nBgn = i;	//	nBgnは引数の先頭の文字
					for( ; i < nLineLen; ++i ){		//	最後の文字までスキャン
						if( szLine[i] == '\\' ){	// エスケープのスキップ
							++i;
							continue;
						}
						if( szLine[i] == '\"' ){	//	\"で終了。
							nEnd = i;	//	nEndは終わりの次の文字（"）
							break;
						}
					}
					cmemWork.SetData( szLine + nBgn, nEnd - nBgn );
					cmemWork.Replace( "\\\"", "\"" );
					cmemWork.Replace( "\\\\", "\\" );
					macro->AddParam( cmemWork.GetPtr( NULL ) );	//	引数を文字列として追加
				}
				else if ( '0' <= szLine[i] && szLine[i] <= '9' ){	//	数字で始まったら数字列だ。
					nBgn = i;	//	nBgnは引数の先頭の文字
					for( ; i < nLineLen; ++i ){		//	最後の文字までスキャン
						if( '0' <= szLine[i] && szLine[i] <= '9' ){	// エスケープのスキップ
							++i;
							continue;
						}
						else {
							nEnd = i;	//	終わりの次の文字（数字じゃない文字）
							break;
						}
					}
					macro->AddParam( atoi(&szLine[nBgn]) );	//	引数を数値として追加
				}
				else {
					//	Parse Error:文法エラーっぽい。
					nBgn = nEnd = i;
				}

				for( ; i < nLineLen; ++i ){		//	最後の文字までスキャン
					if( szLine[i] == ')' || szLine[i] == ',' ){	//	,もしくは)を読み飛ばす
						i++;
						break;
					}
				}
				if (szLine[i-1] == ')'){
					break;
				}
			}
			/* キーマクロのバッファにデータ追加 */
			Append( macro );
		}
	}
	fclose( hFile );

	m_nReady = TRUE;
	return TRUE;
}


/*[EOF]*/
