//	$Id$
/*!	@file
	@brief BREGEXP Library Handler

	Perl5互換正規表現を扱うDLLであるBREGEXP.DLLを利用するためのインターフェース

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001-2002, genta
	Copyright (C) 2002, hor

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

#include <stdio.h>
#include <string.h>
#include "CBregexp.h"
#include "etc_uty.h"

const char BREGEXP_OPT_KI[]	= "ki";
const char BREGEXP_OPT_K[]	= "k";

CBregexp::CBregexp() : m_sRep( NULL )
{
}

CBregexp::~CBregexp()
{
}

//	Jul. 5, 2001 genta 引数追加。ただし、ここでは使わない。
char *
CBregexp::GetDllName( char* str )
{
	return "BREGEXP.DLL";
}
/*!
	DLLの初期化

	関数のアドレスを取得してメンバに保管する．

	@retval 0 成功
	@retval 1 アドレス取得に失敗
*/
int CBregexp::InitDll()
{
	//	アドレス取得
	if( (BMatch = (BREGEXP_BMatch)GetProcAddress( GetInstance(), "BMatch" )) == NULL )
		return 1;
	if( (BSubst = (BREGEXP_BSubst)GetProcAddress( GetInstance(), "BSubst" )) == NULL )
		return 1;
	if( (BTrans = (BREGEXP_BTrans)GetProcAddress( GetInstance(), "BTrans" )) == NULL )
		return 1;
	if( (BSplit = (BREGEXP_BSplit)GetProcAddress( GetInstance(), "BSplit" )) == NULL )
		return 1;
	if( (BRegfree = (BREGEXP_BRegfree)GetProcAddress( GetInstance(), "BRegfree" )) == NULL )
		return 1;
	if( (BRegexpVersion = (BREGEXP_BRegexpVersion)GetProcAddress( GetInstance(), "BRegexpVersion" )) == NULL )
		return 1;

	return 0;
}

/*!
	BREGEXP構造体の解放
*/
int CBregexp::DeinitDll( void )
{
	ReleaseCompileBuffer();
	return 0;
}

/*!
	JRE32のエミュレーション関数．空の文字列に対して検索を行うことにより
	BREGEXP構造体の生成のみを行う．

	@param szPattern [in] 検索パターン

	@retval true 成功
	@retval false 失敗
	
*/
bool CBregexp::Compile( const char* szPattern )
{
	static char tmp[2] = "\0";	//	検索対象となる空文字列

	//	DLLが利用可能でないときはエラー終了
	if( !IsAvailable() )
		return false;

	//	BREGEXP構造体の解放
	ReleaseCompileBuffer();

	//	Jan. 31, 2002 genta
	//	/のエスケープ
	//	メモリ確保．
	char *szNPattern = new char[ strlen(szPattern) * 2 + 5 ];
	szNPattern[0] = '/';
	char *pEnd = szNPattern + 1 + cescape( szPattern, szNPattern + 1, '/', '\\' );
	*pEnd = '/';
	*++pEnd = 'k';
	*++pEnd = '\0';

	BMatch( szNPattern, tmp, tmp+1, &m_sRep, m_szMsg );
	delete [] szNPattern;

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if( m_szMsg[0] ){
		ReleaseCompileBuffer();
		return false;
	}

	return true;
}

/*!
	JRE32のエミュレーション関数．既にあるコンパイル構造体を利用して検索（1行）を
	行う．

	@param target [in] 検索対象領域先頭アドレス
	@param len [in] 検索対象領域サイズ
	@param nStart [in] 検索開始位置．(先頭は0)
	@param rep [out] コンパイル構造体へのアドレスを返す。
	検索には内部で保持している構造体が用いられるため、ここに別の構造体を設定しても
	それは検索には用いられない。

	@retval true Match
	@retval false No Match または エラー。エラーはrep == NULLにより判定可能。

	@note 戻り値==-1

	@par repに返される情報


*/
bool CBregexp::GetMatchInfo( const char* target, int len, int nStart, BREGEXP**rep )
{
	//	DLLが利用可能でないとき、または構造体が未設定の時はエラー終了
	if( (!IsAvailable()) || m_sRep == NULL ){
		*rep = NULL;
		return false;
	}

	*rep = m_sRep;
	//	検索文字列＝NULLを指定すると前回と同一の文字列と見なされる
	// BMatch( m_cPtn.c_str(), target + nStart, target + nStart - len, &m_sRep, m_szMsg );
	if( BMatch( NULL, target + nStart, target + len, &m_sRep, m_szMsg ) ){
		return true;
	}

	return false;
}

// 2002/01/19 novice
/*!
	正規表現による文字列置換

	@param szPattern0 [in] マッチパターン
	@param szPattern1 [in] 置換文字列
	@param target [in] 置換対象データ
	@param len [in] 置換対象データ長
	@param out [out] 置換後文字列		// 2002.01.26 hor
	@retval true 成功
	@retval false 失敗

*/
bool CBregexp::Replace( const char* szPattern0, const char* szPattern1, char *target, int len , char **out)
{
	int result;

	if( !IsAvailable() ){
		return false;
	}

	ReleaseCompileBuffer();

	//	From Here Feb. 01, 2002 genta
	//	/のエスケープ
	char *szNPattern = new char[ ( strlen( szPattern0 ) + strlen( szPattern1 )) * 2 + 5 ];
	szNPattern[0] = 's';
	szNPattern[1] = '/';
	char *pEnd = szNPattern + 2;

	pEnd = pEnd + cescape( szPattern0, pEnd, '/', '\\' );
	*pEnd = '/';
	++pEnd;

	pEnd = pEnd + cescape( szPattern1, pEnd, '/', '\\' );
	*pEnd = '/';
	*++pEnd = 'k';
	*++pEnd = 'm';
	*++pEnd = '\0';
	//	To Here Feb. 01, 2002 genta

	result = BSubst( szNPattern, target, target + len, &m_sRep, m_szMsg );
	delete [] szNPattern;

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if( m_szMsg[0] ){
		return false;
	}

	if( result ){
		if( m_sRep->outp != NULL && m_sRep->outp != '\0' ){
//			strcpy( target, m_sRep->outp );
			int i=lstrlen(m_sRep->outp);
			*out = new char[i+1];
			strcpy( *out, m_sRep->outp );
			(*out)[i] = '\0';
		}else{
//			strcpy( target, "" );
			*out = new char[1];
			(*out)[0] = '\0';
		}

		return true;
	}

	return false;
}

/*[EOF]*/
