//	$Id$
/*!	@file
	BREGEXP Library Handler
	
	Perl5互換正規表現を扱うDLLであるBREGEXP.DLLを利用するためのインターフェース

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta
    Project Sakura Editor: http://sakura-editor.sourceforge.net/

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "CBregexp.h"
//#include "CMemory.h"

CBregexp::CBregexp() : m_sRep( NULL )
{
}

CBregexp::~CBregexp()
{
}

char *
CBregexp::GetDllName()
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
	if((BMatch = (BREGEXP_BMatch)GetProcAddress(GetInstance(), "BMatch" )) == NULL )
		return 1;
	if((BSubst = (BREGEXP_BSubst)GetProcAddress(GetInstance(), "BSubst" )) == NULL )
		return 1;
	if((BTrans = (BREGEXP_BTrans)GetProcAddress(GetInstance(), "BTrans" )) == NULL )
		return 1;
	if((BSplit = (BREGEXP_BSplit)GetProcAddress(GetInstance(), "BSplit" )) == NULL )
		return 1;
	if((BRegfree = (BREGEXP_BRegfree)GetProcAddress(GetInstance(), "BRegfree" )) == NULL )
		return 1;
	if((BRegexpVersion = (BREGEXP_BRegexpVersion)GetProcAddress(GetInstance(), "BRegexpVersion" )) == NULL )
		return 1;

	return 0;
}

/*!
	BREGEXP構造体の解放
*/
int CBregexp::DeinitDll(void)
{
	ReleaseCompileBuffer();
	return 0;
}

/*!
	JRE32のエミュレーション関数．空の文字列に対して検索を行うことにより
	BREGEXP構造体の生成のみを行う．
	
	検索文字列はperl形式 i.e. /pattern/option または m/pattern/option
	
	@param str[in] 検索パターン
	
	@retval true 成功
	@retval false 失敗
*/
bool CBregexp::Compile(const char* szPattern)
{
	static char tmp[2] = "\0";	//	検索対象となる空文字列

	//	DLLが利用可能でないときはエラー終了
	if(!IsAvailable())
		return false;
	
	//	BREGEXP構造体の解放
	ReleaseCompileBuffer();

	BMatch( szPattern, tmp, tmp+1, &m_sRep, m_szMsg );
	
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
bool CBregexp::GetMatchInfo(const char* target, int len, int nStart, BREGEXP**rep)
{
	//	DLLが利用可能でないとき、または構造体が未設定の時はエラー終了
	if((!IsAvailable()) || m_sRep == NULL ){
		*rep = NULL;
		return false;
	}

	*rep = m_sRep;
	//	検索文字列＝NULLを指定すると前回と同一の文字列と見なされる
	// BMatch( m_cPtn.c_str(), target + nStart, target + nStart - len, &m_sRep, m_szMsg );
	if( BMatch( NULL, target + nStart, target + len, &m_sRep, m_szMsg )){
		return true;
	}

	return false;
}

#if 0
/*!
	@note この関数は未使用のため実装無し。

	@param str1[in] 文字列1
	@param str2[in] 文字列2．無い場合はNULL．
	@retval 利用可能な境界文字(01-FF)．エラーの場合は0を返す．
	
	通常の正規表現からパターン文字列を生成するときの境界の選択を行う．
	境界には任意の文字を使えるが、str1/str2に現れない文字を選ばなくてはならない．
	2つ設定できるのは置換するときに置換前と置換後の両方に現れない文字列を選ぶため．
	
	@par 選択方法
	文字列を順に見て0x21から0x7Eの間の文字が出現したかどうかを記録しておく．
	その後、使われていない文字の中でもっとも先頭にあるものを使う．
	
*/
int CBregexp::ChooseBoundary(const char* str1, const char* str2)
{
	return '/';
	//	とりあえず何も無し
}
#endif