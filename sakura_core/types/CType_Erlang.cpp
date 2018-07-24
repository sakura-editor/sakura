/*!	@file
	@brief Erlang アウトライン解析
	
	@author genta
	@date 2009.08.10 created
	
*/
/*
	Copyright (C) 2009, genta

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
#include "doc/CEditDoc.h"
#include "doc/CDocOutline.h"
#include "doc/logic/CDocLine.h"
#include "outline/CFuncInfoArr.h"

/** Erlang アウトライン解析 管理＆解析
*/
struct COutlineErlang {
	enum {
		STATE_NORMAL,	//!< 解析中でない
		STATE_FUNC_CANDIDATE_FIN,	//!< 関数らしきもの(行頭のatom)を解析済み
		STATE_FUNC_ARGS1,	//!< 最初の引数確認中
		STATE_FUNC_ARGS,	//!< 2つめ以降の引数確認中
		STATE_FUNC_ARGS_FIN,//!< 関数の解析を完了
		STATE_FUNC_FOUND,	//!< 関数を発見．データの取得が可能
	} m_state;

	wchar_t m_func[64];	//!< 関数名(Arity含む) = 表示名
	CLogicInt m_lnum;	//!< 関数の行番号
	int m_argcount;		//!< 発見した引数の数
	wchar_t m_parenthesis[32];	//!< 括弧のネストを管理するもの
	int m_parenthesis_ptr;	//!< 括弧のネストレベル
	
	COutlineErlang();
	bool parse( const wchar_t* buf, int linelen, CLogicInt linenum );
	
	const wchar_t* ScanFuncName( const wchar_t* buf, const wchar_t* end, const wchar_t* p );
	const wchar_t* EnterArgs( const wchar_t* end, const wchar_t* p );
	const wchar_t* ScanArgs1( const wchar_t* end, const wchar_t* p );
	const wchar_t* ScanArgs( const wchar_t* end, const wchar_t* p );
	const wchar_t* EnterCond( const wchar_t* end, const wchar_t* p );
	const wchar_t* GetFuncName() const { return m_func; }
	CLogicInt GetFuncLine() const { return m_lnum; }

private:
	// helper functions
	bool IS_ATOM_HEAD( wchar_t wc )
	{
		return ( L'a' <= wc && wc <= L'z' )
			|| ( wc == L'_' ) || ( wc == L'@' );
	}

	bool IS_ALNUM( wchar_t wc )
	{
		return IS_ATOM_HEAD(wc) || ( L'A' <= wc && wc <= L'Z' ) || ( L'0' <= wc && wc <= L'9' );
	}

	bool IS_COMMENT( wchar_t wc )
	{
		return ( wc == L'%' );
	}

	bool IS_SPACE( wchar_t wc )
	{
		return ( wcschr( L" \t\r\n", wc ) != 0 );
	}
	
	void build_arity(int);
};

COutlineErlang::COutlineErlang() :
	m_state( STATE_NORMAL ), m_lnum( 0 ), m_argcount( 0 )
{
}

/** 関数名の取得

	@param[in] buf 行(先頭から)
	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	関数名はatom．atomは 小文字アルファベット，_, @ のいずれかから始まる
	英数文字列か，あるいはシングルクォーテーションで囲まれた文字列．
*/
const wchar_t* COutlineErlang::ScanFuncName( const wchar_t* buf, const wchar_t* end, const wchar_t* p )
{
	assert( m_state == STATE_NORMAL );

	if( p > buf || ! ( IS_ATOM_HEAD( *p ) || *p == L'\'' )) {
		return end;
	}
	
	if( *p == L'\'' ){
		do {
			++p;
		} while( *p != L'\'' && p < end );
		if( p >= end ){
			// invalid atom
			return p;
		}
		++p;
	}
	else {
		do {
			++p;
		} while( IS_ALNUM( *p ) && p < end );
	}
	
	int buf_len = sizeof( m_func ) / sizeof( m_func[0]);
	int len = p - buf;
	if( buf[0] == L'\'' ){
		++buf;
		len -= 2;
		--buf_len;
	}
	len = len < buf_len - 1 ? len : buf_len - 1;
	wcsncpy( m_func, buf, len );
	m_func[len] = L'\0';
	m_state = STATE_FUNC_CANDIDATE_FIN;
	return p;
}

/** パラメータの発見

	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	関数名の取得が完了し，パラメータ先頭の括弧を探す．
*/
const wchar_t* COutlineErlang::EnterArgs( const wchar_t* end, const wchar_t* p )
{
	assert( m_state == STATE_FUNC_CANDIDATE_FIN );

	while( IS_SPACE( *p ) && p < end )
		p++;
	
	if( p >= end )
		return end;

	if( IS_COMMENT( *p )){
		return end;
	}
	else if( *p == L'(' ){ // )
		m_state = STATE_FUNC_ARGS1;
		m_argcount = 0;
		m_parenthesis_ptr = 1;
		m_parenthesis[0] = *p;
		++p;

		return p;
	}

	// not a function
	m_state = STATE_NORMAL;
	return end;
}

/** 先頭パラメータの発見

	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	パラメータが0個と1個以上の判別のために状態を設けている．
*/
const wchar_t* COutlineErlang::ScanArgs1( const wchar_t* end, const wchar_t* p )
{
	assert( m_state == STATE_FUNC_ARGS1 );
	
	while( IS_SPACE( *p ) && p < end )
		p++;

	if( p >= end )
		return end;

	if( *p == /* ( */ L')' ){
		// no argument
		m_state = STATE_FUNC_ARGS_FIN;
		p++;
	}
	else if( IS_COMMENT( *p )){
		return end;
	}
	else {
		// argument found
		m_state = STATE_FUNC_ARGS;
		++m_argcount;
	}
	return p;
}

/** パラメータの解析とカウント

	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	パラメータを解析する．パラメータの数と末尾の閉じ括弧を正しく判別するために，
	引用符，括弧，パラメータの区切りのカンマに着目する．
	引用符は改行を含むことができない．
*/
const wchar_t* COutlineErlang::ScanArgs( const wchar_t* end, const wchar_t* p )
{
	assert( m_state == STATE_FUNC_ARGS );

	const int parptr_max = sizeof( m_parenthesis ) / sizeof( m_parenthesis[0] );
	wchar_t quote = L'\0'; // 先頭位置を保存
	for(const wchar_t* head = p ; p < end ; p++ ){
		if( quote ){
			if( *p == quote )
				quote = L'\0';
		}
		else {
			if( wcschr( L"([{", *p )){	//)
				// level up
				if( m_parenthesis_ptr < parptr_max ){
					m_parenthesis[ m_parenthesis_ptr ] = *p;
				}
				m_parenthesis_ptr++;
			}
			else if( wcschr( L")]}", *p )){	//)
				wchar_t op;
				switch( *p ){
					case L')': op = L'('; break;
					case L']': op = L'['; break;
					case L'}': op = L'{'; break;
					default:
						PleaseReportToAuthor(NULL, LS(STR_OUTLINE_ERLANG_SCANARGS));
						op = 0;
						break;
				}
				// level down
				--m_parenthesis_ptr;
				while( 1 <= m_parenthesis_ptr && m_parenthesis_ptr < parptr_max ){
					if( m_parenthesis[ m_parenthesis_ptr ] != op ){
						// if unmatch then skip
						--m_parenthesis_ptr;
					}
					else{
						break;
					}
				}
				
				// check level
				if( m_parenthesis_ptr == 0 ){
					m_state = STATE_FUNC_ARGS_FIN;
					++p;
					return p;
				}
			}
			else if( *p == L',' && m_parenthesis_ptr == 1 ){
				++m_argcount;
			}
			else if( *p == L';' ){
				//	セミコロンは複数の文の区切り．
				//	パラメータ中には現れないので，解析が失敗している
				//	括弧の閉じ忘れが考えられるので，仕切り直し
				m_state = STATE_NORMAL;
				return end;
			}
			else if( *p == L'.' ){
				//	ピリオドは式の末尾か，小数点として使われる．
				if( p > head && ( L'0' <= p[-1] && p[-1] <= L'9' )){
					//	小数点かもしれないので，そのままにする
				}
				else {
					//	引数の途中で文末が現れたのは解析が失敗している
					//	括弧の閉じ忘れが考えられるので，仕切り直し
					m_state = STATE_NORMAL;
					return end;
				}
			}
			else if( *p == L'"' ){
				quote = L'"';
			}
			else if( *p == L'\'' ){
				quote = L'\'';
			}
			else if( IS_COMMENT( *p )){
				return end;
			}
		}
	}
	return p;
}

/** 関数本体の区切り，または条件文の検出

	@param[in] end buf末尾
	@param[in] p 解析の現在位置
	
	パラメータ本体を表す記号(->)か条件文の開始キーワード(when)を
	見つけたら，関数発見とする．
	それ以外の場合は関数ではなかったと考える．
*/
const wchar_t* COutlineErlang::EnterCond( const wchar_t* end, const wchar_t* p )
{
	while( IS_SPACE( *p ) && p < end )
		p++;

	if( p >= end )
		return end;

	if( p + 1 < end && wcsncmp( p, L"->", 2 ) == 0){
		p += 2;
		m_state = STATE_FUNC_FOUND;
	}
	else if( p + 3 < end && wcsncmp( p, L"when", 4 ) == 0 ){
		m_state = STATE_FUNC_FOUND;
		p += 4;
	}
	else if( IS_COMMENT( *p )){
		return end;
	}
	else {
		m_state = STATE_NORMAL;
	}
	return end;
}

/** 行の解析

	@param[in] buf 行(先頭から)
	@param[in] linelen 行の長さ
	@param[in] linenum 行番号
*/
bool COutlineErlang::parse( const wchar_t* buf, int linelen, CLogicInt linenum )
{
	const wchar_t* pos = buf;
	const wchar_t* const end = buf + linelen;
	if( m_state == STATE_FUNC_FOUND ){
		m_state = STATE_NORMAL;
	}
	if( m_state == STATE_NORMAL ){
		pos = ScanFuncName( buf, end, pos );
		if( m_state != STATE_NORMAL ){
			m_lnum = linenum;
		}
	}
	while( pos < end ){
		switch( m_state ){
			case STATE_FUNC_CANDIDATE_FIN:
				pos = EnterArgs( end, pos ); break;
			case STATE_FUNC_ARGS1:
				pos = ScanArgs1( end, pos ); break;
			case STATE_FUNC_ARGS:
				pos = ScanArgs( end, pos ); break;
			case STATE_FUNC_ARGS_FIN:
				pos = EnterCond( end, pos ); break;
			default:
				PleaseReportToAuthor(NULL, _T("COutlineErlang::parse Unknown State: %d"), m_state );
				break;
		}
		if( m_state == STATE_FUNC_FOUND ){
			build_arity( m_argcount );
			break;
		}
	}
	return m_state == STATE_FUNC_FOUND;
}

/** 関数名の後ろに Arity (引数の数)を付加する

	@param[in] arity 引数の数
	
	関数名の後ろに /パラメータ数 の形で文字列を追加する．
	バッファが不足する場合はできるところまで書き込む．
	そのため，10個以上の引数がある場合に，引数の数の下位桁が欠けることがある．
*/ 
void COutlineErlang::build_arity( int arity )
{
	int len = wcslen( m_func );
	const int buf_size = sizeof( m_func ) / sizeof( m_func[0]);
	wchar_t *p = &m_func[len];
	wchar_t numstr[12];
	
	if( len + 1 >= buf_size )
		return; // no room

	numstr[0] = L'/';
	_itow( arity, numstr + 1, 10 );
	wcsncpy( p, numstr, buf_size - len - 1 );
	m_func[ buf_size - 1 ] = L'\0';
}

/** Erlang アウトライン解析

	@par 主な仮定と方針
	関数宣言は1カラム目から記載されている．
	
	
	@par 解析アルゴリズム
	1カラム目がアルファベットの場合: 関数らしいとして解析開始 / 関数名を保存
	スペースは読み飛ばす
	( を発見したら ) まで引数を数える．その場合入れ子の括弧と文字列を考慮
	-> または when があれば関数定義と見なす(次の行にまたがっても良い)
	途中 % (コメント) が現れたら行末まで読み飛ばす
*/
void CDocOutline::MakeFuncList_Erlang( CFuncInfoArr* pcFuncInfoArr )
{

	COutlineErlang erl_state_machine;
	CLogicInt	nLineCount;

	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		CLogicInt nLineLen;

		const wchar_t* pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( erl_state_machine.parse( pLine, nLineLen, nLineCount )){
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			CLayoutPoint ptPosXY;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(CLogicInt(0), erl_state_machine.GetFuncLine()),
				&ptPosXY
			);
			pcFuncInfoArr->AppendData(
				erl_state_machine.GetFuncLine() + CLogicInt(1),
				ptPosXY.GetY2() + CLayoutInt(1),
				erl_state_machine.GetFuncName(),
				0,
				0
			);
		}
	}
}
