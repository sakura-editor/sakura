//	$Id$
/*!	@file
	@brief BREGEXP Library Handler

	Perl5互換正規表現を扱うDLLであるBREGEXP.DLLを利用するためのインターフェース

	@author genta
	@date Jun. 10, 2001
	@date 2002/2/1 hor		ReleaseCompileBufferを適宜追加
	@date Jul. 25, 2002 genta 行頭条件を考慮した検索を行うように．(置換はまだ)
*/
/*
	Copyright (C) 2001-2002, genta
	Copyright (C) 2002, novice, hor

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

CBregexp::CBregexp() : m_sRep( NULL ),
	m_bTop( false )	//	Jul, 25, 2002 genta
{
}

CBregexp::~CBregexp()
{
	//<< 2002/03/27 Azumaiya
	// 一応、クラスの終了時にコンパイルバッファを解放。
	DeinitDll();
	//>> 2002/03/27 Azumaiya
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
	//	Apr. 15, 2002 genta
	//	CPPA.cpp を参考に設定を配列化した
	
	const ImportTable table[] = {
		{ &BMatch,		"BMatch" },
		{ &BSubst,		"BSubst" },
		{ &BTrans,		"BTrans" },
		{ &BSplit,		"BSplit" },
		{ &BRegfree, 	"BRegfree" },
		{ &BRegexpVersion,	"BRegexpVersion" },
		{ NULL, 0 }
	};
	
	if( ! RegisterEntries( table )){
		return 1;
	}
	
#if 0
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
#endif

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

	検索文字列はperl形式 i.e. /pattern/option または m/pattern/option

	@param szPattern [in] 検索パターン
	@param bOption [in]
		0x01：大文字小文字の区別をする。

	@retval true 成功
	@retval false 失敗
*/
bool CBregexp::Compile( const char* szPattern, int bOption )
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
	char *szNPattern = new char[ strlen(szPattern) * 2 + 15 ];	//	15：「s///option」が余裕ではいるように。
	szNPattern[0] = '/';
	char *pEnd = szNPattern + 1 + cescape( szPattern, szNPattern + 1, '/', '\\' );
	*pEnd = '/';
	*++pEnd = 'k';
	if( !(bOption & 0x01) )		// 2002/2/1 hor IgnoreCase オプション追加 マージ：aroka
		*++pEnd = 'i';		// 同上
	*++pEnd = '\0';

	BMatch( szNPattern, tmp, tmp+1, &m_sRep, m_szMsg );
	delete [] szNPattern;

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if( m_szMsg[0] ){
		ReleaseCompileBuffer();
		return false;
	}
	
	//	From Here Jul. 25, 2002 genta
	//	行頭条件のチェックを追加
	if( szPattern[0] == '^' ){
		m_bTop = true;
	}
	//	To Here Jul. 25, 2002 genta

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

	//	From Here Jul. 25, 2002 genta
	//	行頭チェックの追加
	if( m_bTop && nStart != 0 ){
		return false;
	}
	//	To Here Jul. 25, 2002 genta


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
	@param bOption [in]
		0x01：大文字小文字の区別をする。

	@retval true 成功
	@retval false 失敗
*/
bool CBregexp::Replace( const char* szPattern0, const char* szPattern1, char *target, int len, char **out, int bOption)
{
	int result;

	if( !IsAvailable() ){
		return false;
	}

	ReleaseCompileBuffer();

	//	From Here Feb. 01, 2002 genta
	//	/のエスケープ
	char *szNPattern = new char[ ( strlen( szPattern0 ) + strlen( szPattern1 )) * 2 + 15 ];	//	15：「s///option」が余裕ではいるように。
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
	if( !(bOption & 0x01) )		// 2002/2/1 hor IgnoreCase オプション追加 マージ：aroka
		*++pEnd = 'i';		// 同上
	*++pEnd = '\0';
	//	To Here Feb. 01, 2002 genta

	result = BSubst( szNPattern, target, target + len, &m_sRep, m_szMsg );
	delete [] szNPattern;

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if( m_szMsg[0] ){
		ReleaseCompileBuffer();
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

		ReleaseCompileBuffer();
		return true;
	}

	ReleaseCompileBuffer();
	return false;
}

//<< 2002/03/27 Azumaiya
/*!
	JRE32のエミュレーション関数．空の文字列に対して置換を行うことにより
	BREGEXP構造体の生成のみを行う．

	@param szPattern0 [in] 置換パターン
	@param szPattern1 [in] 置換後文字列パターン
	@param bOption [in]
		0x01：大文字小文字の区別をする。

	@retval true 成功
	@retval false 失敗
*/
bool CBregexp::CompileReplace( const char* szPattern0, const char* szPattern1, int bOption )
{
	static char tmp[2] = "\0";	//	検索対象となる空文字列

	if( !IsAvailable() ){
		return false;
	}

	ReleaseCompileBuffer();

	// \xFF をセパレータに採用。
	int nPattern0 = strlen( szPattern0 );
	int nPattern1 = strlen( szPattern1 );
	char *szNPattern = new char[ nPattern0 + nPattern1 + 15 ];	//	15：「s///option」が余裕ではいるように。
	char *pEnd = szNPattern;
	pEnd++[0] = 's';
	pEnd++[0] = '\xFF';

	// strcpy を使ってもよいと思いますが、速度的にこちらの方が勝っていると思いますので・・・。
	pEnd = (char *)memcpy(pEnd, szPattern0, nPattern0) + nPattern0;

	pEnd++[0] = '\xFF';

	pEnd = (char *)memcpy(pEnd, szPattern1, nPattern1) + nPattern1;

	pEnd++[0] = '\xFF';
	pEnd++[0] = 'k';
	pEnd++[0] = 'm';
	if( !(bOption & 0x01) )		// 2002/2/1 hor IgnoreCase オプション追加 マージ：aroka
	{
		pEnd++[0] = 'i';		// 同上
	}
	pEnd[0] = '\0';
	// 上記のようにも書けるけど、見難いと思うので、わかりやすい書き方をします。
	// どちらかを使ってください。
	// ただ、上記の方が速いと思いますが・・・。
/*	char *szNPattern = new char[ lstrlen( szPattern0 ) + lstrlen( szPattern1 ) + 15 ];	//	15：「s///option」が余裕ではいるように。
	int nPattern = sprintf(szNPattern, "s\xFF%s\xFF%s\xFFkm", szPattern0, szPattern1);
	if( !(bOption & 0x01) )		// 2002/2/1 hor IgnoreCase オプション追加 マージ：aroka
	{
		szNPattern[nPattern++] = 'i';		// 同上
		szNPattern[nPattern] = '\0';
	}*/

	BSubst( szNPattern, tmp, tmp + 1, &m_sRep, m_szMsg );
	delete [] szNPattern;

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if( m_szMsg[0] ){
		ReleaseCompileBuffer();
		return false;
	}

	//	From Here Jul. 25, 2002 genta
	//	行頭条件のチェックを追加
	if( szPattern0[0] == '^' ){
		m_bTop = true;
	}
	//	To Here Jul. 25, 2002 genta
	return true;
}

/*!
	正規表現による文字列置換
	既にあるコンパイル構造体を利用して置換（1行）を
	行う．

	@param szTarget [in] 置換対象データ
	@param nLen [in] 置換対象データ長
	@param pszOut [out] 置換後文字列
	@param pnOutLen [out] 置換後文字列の長さ

	@retval true 成功
	@retval false 失敗
*/
bool CBregexp::GetReplaceInfo(char *szTarget, int nLen, char **pszOut, int *pnOutLen)
{
	if( !IsAvailable() || m_sRep == NULL )
	{
		return false;
	}

	BSubst( NULL, szTarget, szTarget + nLen, &m_sRep, m_szMsg );

	//	メッセージが空文字列でなければ何らかのエラー発生。
	//	サンプルソース参照
	if( m_szMsg[0] )
	{
		ReleaseCompileBuffer();
		return false;
	}

	if( m_sRep->outp != NULL && m_sRep->outp[0] != '\0' )
	{
		int i = strlen(m_sRep->outp);
		*pszOut = new char[i+1];
		memcpy( *pszOut, m_sRep->outp, i );
		(*pszOut)[i] = '\0';
		*pnOutLen = i;
	}
	else
	{
		*pszOut = new char[1];
		(*pszOut)[0] = '\0';
		*pnOutLen = 0;
	}

	return true;
}
//>> 2002/03/27 Azumaiya

/*[EOF]*/
