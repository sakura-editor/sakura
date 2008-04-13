/*!	@file
	@brief アウトライン解析

	@author genta
	@date	2004.08.08 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta
	Copyright (C) 2002, frozen
	Copyright (C) 2003, zenryaku
	Copyright (C) 2005, genta, D.S.Koba, じゅうじ

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include "CDocOutline.h"

#include <string.h>
#include "global.h"

#include "doc/CEditDoc.h"
#include "CFuncInfoArr.h"
#include "doc/CDocLine.h"
#include "charcode.h"
#include "util/other_util.h"

//	Mar. 15, 2000 genta
//	From Here
/*!
	スペースの判定
*/
bool C_IsSpace( char c );
inline bool C_IsSpace( wchar_t c )
{
	return (
		L'\t' == c ||
		L' ' == c ||
		WCODE::CR == c ||
		WCODE::LF == c
	);
}

/*!
	関数に用いることができる文字かどうかの判定
*/
bool C_IsWordChar( char c );
inline bool C_IsWordChar( wchar_t c )
{
	return (
		L'_' == c ||
		L':' == c ||
		L'~' == c ||
		(L'a' <= c && c <= L'z' )||
		(L'A' <= c && c <= L'Z' )||
		(L'0' <= c && c <= L'9' )
	);
}
//	To Here

//	From Here Apr. 1, 2001 genta
/*!
	特殊な関数名 "operator" かどうかを判定する。

	文字列が"operator"それ自身か、あるいは::の後ろにoperatorと続いて
	終わっているときにoperatorと判定。

	演算子の評価順序を保証するため2つのif文に分けてある

	@param szStr 判定対象の文字列
	@param nLen 文字列の長さ。
	本質的には不要であるが、高速化のために既にある値を利用する。
*/
static bool C_IsOperator( wchar_t* szStr, int nLen	)
{
	if( nLen >= 8 && szStr[ nLen - 1 ] == L'r' ){
		if( nLen > 8 ?
				wcscmp( szStr + nLen - 9, L":operator" ) == 0 :	// メンバー関数による定義
				wcscmp( szStr, L"operator" ) == 0	// friend関数による定義
		 ){
		 	return true;
		}
	}
	return false;
}
//	To Here Apr. 1, 2001 genta

/*!
	改行直前を \ でエスケープしているかどうか判定

	@date 2005.12.06 じゅうじ 最後の1文字しか見ないと2バイトコードの後半がバックスラッシュの場合に誤認する
*/
static bool C_IsLineEsc(const char *s, int len);
static bool C_IsLineEsc(const wchar_t *s, int len)
{
	if ( len > 0 && s[len-1] == L'\n' ) len--;
	if ( len > 0 && s[len-1] == L'\r' ) len--;
	if ( len > 0 && s[len-1] == L'\n' ) len--;

	if ( len > 0 && s[len-1] == L'\\' ) {
		if ( len == 1 ) {
			return(true);
		} else if ( len == 2 ) {
			if ( CNativeW::GetSizeOfChar( s, 2 , 0 ) == 1 )
				return(true);
		} else { //残り３バイト以上
			if ( CNativeW::GetSizeOfChar( s, len , len-2 ) == 1 )
				return(true);
			if ( CNativeW::GetSizeOfChar( s, len , len-3 ) == 2 )
				return(true);
		}
	}
	return(false);
}


/*!
	Cプリプロセッサの #if/ifdef/ifndef - #else - #endif状態管理クラス

	ネストレベルは32レベル=(sizeof(int) * 8)まで
*/

class CCppPreprocessMng {
public:
	CCppPreprocessMng(void) :
		m_stackptr( 0 ), m_bitpattern( 1 ), m_enablebuf( 0 ), m_maxnestlevel( 32 ), m_ismultiline( false )
	{}

	CLogicInt ScanLine(const wchar_t*, CLogicInt);

private:
	bool m_ismultiline; //!< 複数行のディレクティブ
	int m_maxnestlevel;	//!< ネストレベルの最大値

	int m_stackptr;	//!< ネストレベル
	/*!
		ネストレベルに対応するビットパターン
		
		m_stackptr = n の時，下から(n-1)bit目に1が入っている
	*/
	unsigned int m_bitpattern;
	unsigned int m_enablebuf;	//!< 処理の有無を保存するバッファ
};

/*!
	Cプリプロセッサの #if/ifdef/ifndef - #else - #endifを元に
	処理の必要性を判定する．

	与えられた1行の文字列を先頭から走査し，C/C++での走査が必要な場合は
	先頭の空白を除いた開始位置を，不要な場合はlengthを返す．
	呼び出し側では戻り値から解析を始めるので，
	lengthを返すことはすべて空白と見なすことになる．

	ネストの最大を超えた場合には記憶域がないために判定は不可能となるが，
	ネストレベルだけは管理する．

	@param str		[in] 1行の文字列
	@param length	[in] 文字列長

	@return C解析開始位置．処理不要の場合はlength(行末までスキップ)．
	
	@par elifの扱い
	if (A) elif (B) elif (C) else (D) endifのような場合には(A)-(D)のどれか1つ
	だけが実行される．しかし，そうなると1ビットでは管理できないしネストを
	囲むようなケースでelifを使うことはあまり無いと勝手に決めて見なかったことにする．

	@author genta
	@date 2004.08.10 新規作成
	@date 2004.08.13 zenryaku 複数行のディレクティブに対応

*/
CLogicInt CCppPreprocessMng::ScanLine( const wchar_t* str, CLogicInt _length )
{
	int length=_length;

	const wchar_t* lastptr = str + length;	//	処理文字列末尾
	const wchar_t* p;	//	処理中の位置

	//	skip whitespace
	for( p = str; C_IsSpace( *p ) && p < lastptr ; ++p )
		;
	if( lastptr <= p )
		return CLogicInt(length);	//	空行のため処理不要

	if(m_ismultiline){ // 複数行のディレクティブは無視
		m_ismultiline = C_IsLineEsc(str, length); // 行末が \ で終わっていないか
		return CLogicInt(length);
	}

	if( *p != L'#' ){	//	プリプロセッサ以外の処理はメイン部に任せる
		if( m_enablebuf ){
			return CLogicInt(length);	//	1ビットでも1となっていたら無視
		}
		return CLogicInt(p - str);
	}

	++p; // #をスキップ
	
	//	skip whitespace
	for( ; C_IsSpace( *p ) && p < lastptr ; ++p )
		;

	//	ここからPreprocessor directive解析
	if( p + 2 + 2 < lastptr && wcsncmp_literal( p, L"if" ) == 0 ){
		// if
		p += 2;
		
		int enable = 0;	//	0: 処理しない, 1: else以降が有効, 2: 最初が有効, 
		
		//	if 0は最初が無効部分とみなす．
		//	それ以外のif/ifdef/ifndefは最初が有効部分と見なす
		//	最初の条件によってこの時点ではp < lastptrなので判定省略
		if( C_IsSpace( *p ) ){
			//	if 0 チェック
			//	skip whitespace
			for( ; C_IsSpace( *p ) && p < lastptr ; ++p )
				;
			if( *p == L'0' ){
				enable = 1;
			}
			else {
				enable = 2;
			}
		}
		else if(
			( p + 3 < lastptr && wcsncmp_literal( p, L"def" ) == 0 ) ||
			( p + 4 < lastptr && wcsncmp_literal( p, L"ndef" ) == 0 )){
			enable = 2;
		}
		
		//	保存領域の確保とビットパターンの設定
		if( enable > 0 ){
			m_bitpattern = 1 << m_stackptr;
			++m_stackptr;
			if( enable == 1 ){
				m_enablebuf |= m_bitpattern;
			}
		}
	}
	else if( p + 4 < lastptr && wcsncmp_literal( p, L"else" ) == 0 ){
		if( m_stackptr < m_maxnestlevel ){
			m_enablebuf ^= m_bitpattern;
		}
	}
	else if( p + 5 < lastptr && wcsncmp_literal( p, L"endif" ) == 0 ){
		if( m_stackptr > 0 ){
			--m_stackptr;
			m_enablebuf &= ~m_bitpattern;
			m_bitpattern = ( 1 << ( m_stackptr - 1 ));
		}
	}
	else{
		m_ismultiline = C_IsLineEsc(str, length); // 行末が \ で終わっていないか
	}

	return CLogicInt(length);	//	基本的にプリプロセッサ指令は無視
}

/*!
	@brief C/C++関数リスト作成

	@param bVisibleMemberFunc クラス、構造体定義内のメンバ関数の宣言をアウトライン解析結果に登録する場合はtrue

	@par MODE一覧
	- 0	通常
	- 20	Single quotation文字列読み込み中
	- 21	Double quotation文字列読み込み中
	- 8	コメント読み込み中
	- 1	単語読み込み中
	- 2	記号列読み込み中
	- 999	長過ぎる単語無視中


	@par FuncIdの値の意味
	10の位で目的別に使い分けている．C/C++用は10位が0
	- 1: 宣言
	- 2: 通常の関数 (追加文字列無し)
	- 3: クラス("クラス")
	- 4: 構造体 ("構造体")
	- 5: 列挙体("列挙体")
	- 6: 共用体("共用体")
	- 7: 名前空間("名前空間")

	@param pcFuncInfoArr [out] 関数一覧を返すためのクラス。
	ここに関数のリストを登録する。
*/
void CDocOutline::MakeFuncList_C( CFuncInfoArr* pcFuncInfoArr ,bool bVisibleMemberFunc )
{
	const wchar_t*	pLine;
	CLogicInt	nLineLen;
	CLogicInt	i;

	// 2002/10/27 frozen　ここから
	// nNestLevelを nNestLevel_global を nNestLevel_func に分割した。
	int			nNestLevel_global = 0;	// nNestLevel_global 関数外の {}のレベル  
	int			nNestLevel_func   = 0;	//	nNestLevel_func 関数の定義、および関数内の	{}のレベル
//	int			nNestLevel2;			//	nNestLevel2	()に対する位置 // 2002/10/27 frozen nNastLevel_fparamとnMode2のM2_FUNC_NAME_ENDで代用
	int			nNestLevel_fparam = 0;	// ()のレベル
	int			nNestPoint_class = 0;	// 外側から何番目の{がクラスの定義を囲む{か？ (一番外側なら1、0なら無し。bVisibleMemberFuncがfalseの時のみ有効。trueでは常に0)
	// 2002/10/27 frozen　ここまで

	int			nCharChars;			//	多バイト文字を読み飛ばすためのもの
	wchar_t		szWordPrev[256];	//	1つ前のword
	wchar_t		szWord[256];		//	現在解読中のwordを入れるところ
	int			nWordIdx = 0;
	int			nMaxWordLeng = 100;	//	許容されるwordの最大長さ
	int			nMode;				//	現在のstate

	// 2002/10/27 frozen　ここから
	//! 状態2
	enum MODE2
	{
		M2_NORMAL			= 0x00,	//!< 通常
		M2_NAMESPACE_SAVE	= 0x11,	//!< ネームスペース名調査中
			// 「通常」状態で単語 "class" "struct" "union" "enum" "namespace"を読み込むと、この状態になり、';' '{' ',' '>' '='を読み込むと「通常」になる。
			//
			// ':' を読み込むと「ネームスペース名調査完了」へ移行すると同時に
			// szWordをszTokenNameに保存し、あとで ':' 又は '{' の直前の単語が調べられるようにしている。
			// これは "__declspec( dllexport )"のように"class"とクラス名の間にキーワードが書いてある場合でもクラス名を取得できるようにするため。
			//
			// '<' を読み込むと「テンプレートクラス名調査中」に移行する。
		M2_TEMPLATE_SAVE	= 0x12, //!< テンプレートクラス名調査中
			// ';' '{'を読み込むと「通常」になる。
			// また、この状態の間は単語を区切る方法を一時的に変更し、
			// 「template_name <paramA,paramB>」のような文字列を一つの単語をみなすようにする。
			// これは特殊化したクラステンプレートを実装する際の構文で有効に働く。	
		M2_NAMESPACE_END	= 0x13,	//!< ネームスペース名調査完了。(';' '{' を読み込んだ時点で「通常」になる。 )
		M2_FUNC_NAME_END	= 0x14, //!< 関数名調査完了。(';' '{' を読み込んだ時点で「通常」になる。 )
		M2_AFTER_EQUAL		= 0x05,	//!< '='の後。
			//「通常」かつ nNestLevel_fparam==0 で'='が見つかるとこの状態になる。（ただし "opreator"の直後は除く）
			// ';'が見つかると「通常」に戻る。
			// int val=abs(-1);
			// のような文が関数とみなされないようにするために使用する。
		M2_KR_FUNC			= 0x16,	//!< K&Rスタイルの関数定義を調査する。
		M2_AFTER_ITEM		= 0x10,
	} nMode2 = M2_NORMAL;

	const int	nNamespaceNestMax	= 32;			//!< ネスト可能なネームスペース、クラス等の最大数
	int			nNamespaceLen[nNamespaceNestMax+1];	//!< ネームスペース全体の長さ
	const int	nNamespaceLenMax 	= 512;			//!< 最大のネームスペース全体の長さ
	wchar_t		szNamespace[nNamespaceLenMax];		//!< 現在のネームスペース(終端が\0になっているとは限らないので注意)
	const int 	nItemNameLenMax	 	= 256;
	wchar_t		szItemName[nItemNameLenMax];		//!< すぐ前の 関数名 or クラス名 or 構造体名 or 共用体名 or 列挙体名 or ネームスペース名
	// 例えば下のコードの←の部分での
	// szNamespaceは"Namespace\ClassName\"
	// nMamespaceLenは{10,20}
	// nNestLevel_globalは2となる。
	//
	//　namespace Namespace{
	//　class ClassName{
	//　←
	//　}}

	CLogicInt	nItemLine;			//!< すぐ前の 関数 or クラス or 構造体 or 共用体 or 列挙体 or ネームスペースのある行
	int			nItemFuncId;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	szNamespace[0] = L'\0';	// 2002/10/27 frozen
	nNamespaceLen[0] = 0;	// 2002/10/27 frozen
	nMode = 0;
	
	//	Aug. 10, 2004 genta プリプロセス処理クラス
	CCppPreprocessMng cCppPMng;
	
	CLogicInt		nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);

		//	From Here Aug. 10, 2004 genta
		//	プリプロセス処理
		//	コメント中でなければプリプロセッサ指令を先に判定させる
		if( 8 != nMode && 10 != nMode ){	/* chg 2005/12/6 じゅうじ 次の行が空白でもよい	*/
			i = cCppPMng.ScanLine( pLine, nLineLen );
		}
		else {
			i = CLogicInt(0);
		}
		//	C/C++としての処理が不要なケースでは i == nLineLenとなっているので
		//	以下の解析処理はSKIPされる．
		//	To Here Aug. 10, 2004 genta
		
		for( ; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if( 1 < nCharChars ){
				i += CLogicInt(nCharChars - 1);
				continue;
			}
/* del start 2005/12/6 じゅうじ	*/
			/* エスケープシーケンスは常に取り除く */
			/* シングルクォーテーション文字列読み込み中 */
			/* ダブルクォーテーション文字列読み込み中 */
			// いずれもコメント処理の後へ移動
/* del end 2005/12/6 じゅうじ	*/
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}
			/* ラインコメント読み込み中 */
			// 2003/06/24 zenryaku
			else if( 10 == nMode)
			{
				if(!C_IsLineEsc(pLine, nLineLen)){
					nMode = 0;
				}
				i = nLineLen;
				continue;
			}
			/* add start 2005/12/6 じゅうじ	*/
			/* エスケープシーケンスは常に取り除く */
			else if( '\\' == pLine[i] ){
				++i;
			}
			/* シングルクォーテーション文字列読み込み中 */
			else if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}
			/* ダブルクォーテーション文字列読み込み中 */
			else if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}
			/* add end 2005/12/6 じゅうじ	*/
			/* 単語読み込み中 */
			else if( 1 == nMode ){
				if( C_IsWordChar( pLine[i] ) ){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						if( pLine[i] == L':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									wcscpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								wcsncat( szItemName, szWord, nItemNameLenMax - wcslen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = L'\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}else{
					//	From Here Mar. 31, 2001 genta
					//	operatorキーワード(演算子overload)の対応
					//	ただし、operatorキーワードの後ろにスペースが入っているとうまく動かない。
					if( C_IsOperator( szWord, nWordIdx + 1 ) ){
						//	operatorだ！
						/*  overloadする演算子一覧
							& && &=
							| || |=
							+ ++ +=
							- -- -= -> ->*
							* *=
							/ /=
							% %=
							^ ^=
							! !=
							= ==
							< <= << <<=
							> >= >> >>=
							()
							[]
							~
							,
						*/
						int oplen = 0;	// 演算子本体部の文字列長
						switch( pLine[i] ){
						case '&': // no break
						case '|': // no break
						case '+':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == pLine[ i ] ||
									pLine[ i + 1 ] == L'=' )
									oplen = 2;
							}
							break;
						case '-':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == L'-' ||
									pLine[ i + 1 ] == L'=' )
									oplen = 2;
								else if( pLine[ i + 1 ] == L'>' ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == L'*' )
											oplen = 3;
									}
								}
							}
							break;
						case '*': // no break
						case '/': // no break
						case '%': // no break
						case '^': // no break
						case '!': // no break
						case '=':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == L'=' )
									oplen = 2;
							}
							break;
						case '<': // no break
						case '>':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == pLine[ i ] ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == L'=' )
											oplen = 3;
									}
								}
								else if( pLine[ i + 1 ] == L'=' )
									oplen = 2;
							}
							break;
						case '(':
							if( i + 1 < nLineLen )
								if( pLine[ i + 1 ] == /* 括弧対応対策 ( */ ')' )
									oplen = 2;
							break;
						case '[':
							if( i + 1 < nLineLen )
								if( pLine[ i + 1 ] == /* 括弧対応対策 [ */ ']' )
									oplen = 2;
							break;
						case '~': // no break
						case ',':
							oplen = 2;
							break;
						}

						//	oplen の長さだけキーワードに追加
						for( ; oplen > 0 ; oplen--, i++ ){
							++nWordIdx;
							szWord[nWordIdx] = pLine[i];
						}
						szWord[nWordIdx + 1] = L'\0';
							// 記号列の処理を行う前は記号列のiは記号列の先頭を指していた。
							// この時点でiは記号列の1つ後を指している

							// operatorの後ろに不正な文字がある場合の動作
							// ( で始まる場合はoperatorという関数と認識される
							// それ以外の記号だと従来通り記号列がglobalのしたに現れる。

							// 演算子が抜けている場合の動作
							// 引数部が()の場合はそれが演算子と見なされるため、その行は関数定義と認識されない
							// それ以外の場合はoperatorという関数と認識される
					}
					// 2002/10/27 frozen　ここから
					if( nMode2 == M2_NAMESPACE_SAVE )
						wcscpy( szItemName, szWord );
					else if( nMode2 == M2_TEMPLATE_SAVE)
					{
						wcsncat( szItemName, szWord, nItemNameLenMax - wcslen(szItemName) );
						szItemName[ nItemNameLenMax - 1 ] = L'\0';
					}
					else if( nNestLevel_func == 0 && nMode2 == M2_NORMAL )
					{
						nItemFuncId = 0;
						if( wcscmp(szWord,L"class")==0 )
							nItemFuncId = 3;
						if( wcscmp(szWord,L"struct")==0 )
							nItemFuncId = 4;
						else if( wcscmp(szWord,L"namespace")==0 )
							nItemFuncId = 7;
						else if( wcscmp(szWord,L"enum")==0 )
							nItemFuncId = 5;
						else if( wcscmp(szWord,L"union")==0 )
							nItemFuncId = 6;
						if( nItemFuncId != 0 )
						{
							nMode2 = M2_NAMESPACE_SAVE;
							nItemLine = nLineCount + CLogicInt(1);
							wcscpy(szItemName,L"無名");
						}
					}
					else if( nMode2 == M2_FUNC_NAME_END )
					{
						nMode2 = M2_KR_FUNC;
					}
					// 2002/10/27 frozen　ここまで

					//	To Here Mar. 31, 2001 genta
					// 2004/03/12 zenryaku キーワードに _ と PARAMS を使わせない (GNUのコードが見にくくなるから)
					if( !( wcscmp(L"PARAMS",szWord) == 0 || wcscmp(L"_",szWord) == 0 ) )
						wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
				if( C_IsWordChar( pLine[i] ) ||
					C_IsSpace( pLine[i] ) ||
					 '{' == pLine[i] ||
					 '}' == pLine[i] ||
					 '(' == pLine[i] ||
					 ')' == pLine[i] ||
					 ';' == pLine[i] ||
					'\'' == pLine[i] ||
					 '"' == pLine[i] ||
					 '/' == pLine[i]
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}else{
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( C_IsSpace( pLine[i] ) ){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( C_IsSpace( pLine[i] ) )
					continue;

				if( i < nLineLen - 1 && '/' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 10;
					continue;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = 21;
					continue;
				}else
				
				// 2002/10/27 frozen ここから
				if( '{' == pLine[i] )
				{
					int nItemNameLen;
					if( nNestLevel_func !=0)
						++nNestLevel_func;
					else if(
							(nMode2 & M2_AFTER_ITEM) != 0  &&
							nNestLevel_global < nNamespaceNestMax &&
							(nNamespaceLen[nNestLevel_global] +  (nItemNameLen = wcslen(szItemName)) + 10 + 1) < nNamespaceLenMax)
					// ３番目の(&&の後の)条件
					// バッファが足りない場合は項目の追加を行わない。
					// +10は追加する文字列の最大長(追加する文字列は"::定義位置"が最長)
					// +1は終端NUL文字
					{
						wcscpy( &szNamespace[nNamespaceLen[nNestLevel_global]] , szItemName);
						//	Jan. 30, 2005 genta M2_KR_FUNC 追加
						//	関数の後ろにconst, throw または初期化子があると
						//	M2_KR_FUNCに遷移して，';'が見つからないとその状態のまま
						//	中括弧に遭遇する．
						if( nMode2 == M2_FUNC_NAME_END || nMode2 == M2_KR_FUNC )
							++ nNestLevel_func;
						else
						{
							++ nNestLevel_global;
							nNamespaceLen[nNestLevel_global] = nNamespaceLen[nNestLevel_global-1] + nItemNameLen;
							if( nItemFuncId == 7)
								wcscpy(&szNamespace[nNamespaceLen[nNestLevel_global]],L"::定義位置");
							else
							{
								szNamespace[nNamespaceLen[nNestLevel_global]] = L'\0';
								szNamespace[nNamespaceLen[nNestLevel_global]+1] = L':';
								if(bVisibleMemberFunc == false && nNestPoint_class == 0)
									nNestPoint_class = nNestLevel_global;
							}
						}
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						CLayoutPoint ptPosXY;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							CLogicPoint(0, nItemLine - 1),
							&ptPosXY
						);
						pcFuncInfoArr->AppendData( nItemLine, ptPosXY.GetY2() + CLayoutInt(1) , szNamespace, nItemFuncId);
						//	Jan. 30, 2005 genta M2_KR_FUNC 追加
						if( nMode2 != M2_FUNC_NAME_END && nMode2 != M2_KR_FUNC )
						{
							szNamespace[nNamespaceLen[nNestLevel_global]] = L':';
							nNamespaceLen[nNestLevel_global] += 2;
						}
					}
					else
					{
						//	Jan. 30, 2005 genta M2_KR_FUNC 追加
						if(nMode2 == M2_FUNC_NAME_END || nMode2 == M2_KR_FUNC)
							++ nNestLevel_func;
						else
						{
							++ nNestLevel_global;
							if ( nNestLevel_global <= nNamespaceNestMax )
								nNamespaceLen[nNestLevel_global]=nNamespaceLen[nNestLevel_global-1];
						}
					}
					// bCppInitSkip = false;	//	Mar. 4, 2001 genta
					nMode = 0;
					nMode2 = M2_NORMAL;
					// nNestLevel2 = 0;
					continue;
				}else
				// 2002/10/27 frozen ここまで
				
				if( '}' == pLine[i] ){
					//  2002/10/27 frozen ここから
//					nNestLevel2 = 0;
					if(nNestLevel_func == 0)
					{
						if(nNestLevel_global!=0)
						{
							if(nNestLevel_global == nNestPoint_class)
								nNestPoint_class = 0;
							--nNestLevel_global;
						}
					}
					else
						--nNestLevel_func;
					//  2002/10/27 frozen ここまで
					nMode = 0;
					nMode2 = M2_NORMAL;
					continue;
				}else
				if( '(' == pLine[i] ){
					//  2002/10/27 frozen ここから
//					if( nNestLevel == 0 && !bCppInitSkip ){
//						wcscpy( szFuncName, szWordPrev );
//						nFuncLine = nLineCount + 1;
//						nNestLevel2 = 1;
//					}
//					nMode = 0;
					if( nNestLevel_func == 0 && (nMode2 == M2_NORMAL || nMode2 == M2_NAMESPACE_SAVE || nMode2 == M2_NAMESPACE_END) )
					{
						if( wcscmp(szWordPrev, L"__declspec") == 0 ) {continue;}
						if(nNestLevel_fparam==0)
						{
							wcscpy( szItemName, szWordPrev);
							nItemLine = nLineCount + CLogicInt(1);
						}
						++ nNestLevel_fparam;
					}
					//  2002/10/27 frozen ここまで
					
					//	From Here Jan. 30, 2005 genta 
					if( nNestLevel_func == 0 && nMode2 == M2_KR_FUNC ){
						//	throwなら (例外の型,...) を読み飛ばす
						if(nNestLevel_fparam==0)
							++ nNestLevel_fparam;
					}
					//	To Here Jan. 30, 2005 genta 
					continue;
				}else
				if( ')' == pLine[i] ){
					//  2002/10/27 frozen ここから
//					if( 1 == nNestLevel2 ){
//						nNestLevel2 = 2;
//					}
//					nMode = 0;
					if( nNestLevel_fparam > 0)
					{
						--nNestLevel_fparam;
						if( nNestLevel_fparam == 0)
						{
							nMode2 = M2_FUNC_NAME_END;
							nItemFuncId = 2;
						}
					}
					//  2002/10/27 frozen ここまで
					continue;
				}else
				if( ';' == pLine[i] ){
					//  2002/10/27 frozen ここから
					if( nMode2 == M2_KR_FUNC )
					{
						//	Jan. 30, 2005 genta 関数後の const, throwの後ろの
						//	';'はK&R形式宣言の終わりでなく関数宣言の終わり
						if( wcscmp( szWordPrev, L"const" ) == 0 ||
							wcscmp( szWordPrev, L"throw" ) == 0 ){
								nMode2 = M2_FUNC_NAME_END;
								//	すぐ下のif文に引っかかりますように
						}
						else {
							// zenryaku K&Rスタイルの関数宣言の終了後 M2_FUNC_NAME_END にもどす
							nMode2 = M2_FUNC_NAME_END;
							continue;
						}
					} //	Jan. 30, 2005 genta K&R処理に引き続いて宣言処理も行う．
					if( nMode2 == M2_FUNC_NAME_END &&
						nNestLevel_global < nNamespaceNestMax &&
						(nNamespaceLen[nNestLevel_global] + wcslen(szItemName)) < nNamespaceLenMax &&
						nNestPoint_class == 0)
					// ３番目の(&&の後の)条件
					// バッファが足りない場合は項目の追加を行わない。
					{
						wcscpy( &szNamespace[nNamespaceLen[ nNestLevel_global]] , szItemName);

						nItemFuncId = 1;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						CLayoutPoint ptPosXY;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							CLogicPoint(0, nItemLine - 1),
							&ptPosXY
						);
						pcFuncInfoArr->AppendData( nItemLine, ptPosXY.GetY2() + CLayoutInt(1), szNamespace, nItemFuncId);
					}
					nMode2 = M2_NORMAL;
					//  2002/10/27 frozen ここまで
					nMode = 0;
					continue;
				}else if( nNestLevel_fparam == 0 ){
					if( C_IsWordChar( pLine[i] ) ){
						//	//	Mar. 15, 2000 genta
						//	From Here
						//	直前のwordの最後が::か，あるいは直後のwordの先頭が::なら
						//	クラス限定子と考えて両者を接続する．

						{
							int pos = wcslen( szWordPrev ) - 2;
							if( //	前の文字列の末尾チェック
								( pos > 0 &&	szWordPrev[pos] == L':' &&
								szWordPrev[pos + 1] == L':' ) ||
								//	次の文字列の先頭チェック
								( i < nLineLen - 1 && pLine[i] == L':' &&
									pLine[i+1] == L':' )
							){
								//	前の文字列に続ける
								wcscpy( szWord, szWordPrev );
								nWordIdx = wcslen( szWord );
							}
							//	From Here Apr. 1, 2001 genta
							//	operator new/delete 演算子の対応
							else if( C_IsOperator( szWordPrev, pos + 2 ) ){
								//	スペースを入れて、前の文字列に続ける
								szWordPrev[pos + 2] = L' ';
								szWordPrev[pos + 3] = L'\0';
								wcscpy( szWord, szWordPrev );
								nWordIdx = wcslen( szWord );
							}
							//	To Here Apr. 1, 2001 genta
							else{
								nWordIdx = 0;
							}
						}
						//	wcscpy( szWordPrev, szWord );	不要？
						//	To Here
						
						if( pLine[i] == L':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									wcscpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								wcsncat( szItemName, szWord, nItemNameLenMax - wcslen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = L'\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}

						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
						//	//	Mar. 15, 2000 genta
						//	From Here
						//	長さチェックは必須
						if( nWordIdx < nMaxWordLeng ){
							nMode = 1;
						}
						else{
							nMode = 999;
						}
						//	To Here
					}else{
						//	Aug. 13, 2004 genta
						//	szWordPrevが失われないうちにoperatorの判定を行う
						//	operatorの判定は前にクラス名が付いている可能性があるので
						//	専用の判定関数を使うべし．
						//	operatorで無ければ=は代入なのでここは宣言文ではない．
						if( pLine[i] == L'=' && nNestLevel_func == 0 && nNestLevel_fparam==0
							&& nMode2 == M2_NORMAL && ! C_IsOperator(szWordPrev,wcslen(szWordPrev)) ){
							nMode2 = M2_AFTER_EQUAL;
						}
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
						nMode = 2;

						// 2002/10/27 frozen ここから
						if( nMode2 == M2_NAMESPACE_SAVE )
						{
							if( pLine[i] == L'>' || pLine[i] == L',' || pLine[i] == L'=')
								// '<' の前に '>' , ',' , '=' があったので、おそらく
								// 前にあった"class"はテンプレートパラメータの型を表していたと考えられる。
								// よって、クラス名の調査は終了。
								// '>' はテンプレートパラメータの終了
								// ',' はテンプレートパラメータの区切り
								// '=' はデフォルトテンプレートパラメータの指定
								nMode2 = M2_NORMAL; 
							else if( pLine[i] == L'<' )
								nMode2 = M2_TEMPLATE_SAVE;
						}

						if( nMode2 == M2_TEMPLATE_SAVE)
						{
							int nItemNameLen = wcslen(szItemName);
							if(nItemNameLen + 1 < nItemNameLenMax )
							{
								szItemName[nItemNameLen] = pLine[i];
								szItemName[nItemNameLen + 1 ] = L'\0';
							}
						}
						// 2002/10/27 frozen ここまで
					}
				}
			}
		}
	}
	return;
}




/*! @file
	@brief Pythonアウトライン解析

	@author genta
	@date 2007.02.24 新規作成
*/
/*
	Copyright (C) 2007, genta

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

#include <string.h>
#include "global.h"
#include <assert.h>

#include "doc/CEditDoc.h"
#include "CFuncInfoArr.h"
#include "doc/CDocLine.h"
#include "charcode.h"



/*!
	関数に用いることができる文字かどうかの判定
	
	@date 2007.02.08 genta

	@note 厳密には1文字目に数字を使うことは出来ないが，
		それは実行してみれば明らかにわかることなので
		そこまで厳密にチェックしない
*/
inline bool Python_IsWordChar( wchar_t c ){
	return ( L'_' == c ||
			(L'a' <= c && c <= L'z' )||
			(L'A' <= c && c <= L'Z' )||
			(L'0' <= c && c <= L'9' )
		);
}

/*! pythonのパース状態を管理する構造体

	解析中に解析関数の間を引き渡される．
	このクラスは現在の状態と，文字列の性質を保持する．
	解析位置は解析関数間でパラメータとして渡されるので
	この中では保持しない．

	[状態遷移]
	開始 : STATE_NORMAL

	STATE_NORMAL/STATE_CONTINUE→STATE_CONTINUEの遷移
	- 継続行マーク有り

	STATE_NORMAL/STATE_CONTINUE→STATE_NORMALの遷移
	- 継続行マークがなく行末に達した
	- コメントに達した

	STATE_NORMAL→STATE_STRINGの遷移
	- 引用符あり

	STATE_STRING→STATE_NORMALの遷移
	- 規定の文字列終了記号
	- short stringで文字列の終了を示す引用符も継続行マークもなく行末に達した

	@date 2007.02.12 genta
*/
struct COutlinePython {
	enum {
		STATE_NORMAL,	//!< 通常行 : 行頭を含む
		STATE_STRING,	//!< 文字列中
		STATE_CONTINUE,	//!< 継続行 : 前の行からの続きなので行頭とはみなされない
	} m_state;
	
	int m_quote_char;	//!<	引用符記号
	bool m_raw_string;	//!<	エスケープ記号無視ならtrue
	bool m_long_string;	//!<	長い文字列中ならtrue

	COutlinePython();

	/*	各状態における文字列スキャンを行う
		Scan*が呼びだされるときは既にその状態になっていることが前提．
		ある状態から別の状態に移るところまでを扱う．
		別の状態に移る判定がややこしいばあいは，Enter*として関数にする．
	*/	
	int ScanNormal( const wchar_t* data, int linelen, int start_offset );
	int ScanString( const wchar_t* data, int linelen, int start_offset );
	int EnterString( const wchar_t* data, int linelen, int start_offset );
	void DoScanLine( const wchar_t* data, int linelen, int start_offset );
	
	bool IsLogicalLineTop(void) const { return STATE_NORMAL == m_state; }
};

/*!コンストラクタ: 初期化

	初期状態をSTATE_NORMALに設定する．
*/
COutlinePython::COutlinePython()
	: m_state( STATE_NORMAL ),
	m_raw_string( false ),
	m_long_string( false )
{
}

/*! @brief Python文字列の入り口で文字列種別を決定する

	文字列の種類を適切に判別し，内部状態を設定する．
	start_offsetは開始引用符を指していること．

	- 引用符1つ: short string
	- 引用符3つ: long string
	- 引用符の前にrかRあり : raw string

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置
	
	@return 調査後の位置

	@invariant
		m_state != STATE_STRING

	@note 引用符の位置で呼びだせば，抜けた後は必ずSTATE_STRINGになっているはず．
		引用符以外の位置で呼びだした場合は何もしないで抜ける．
*/
int COutlinePython::EnterString( const wchar_t* data, int linelen, int start_offset )
{
	assert( m_state != STATE_STRING );

	int col = start_offset;
	//	文字列開始チェック
	if( data[ col ] == '\"' || data[ col ] == '\'' ){
		int quote_char = data[ col ];
		m_state = STATE_STRING;
		m_quote_char = quote_char;
		//	文字列の開始
		if( col >= 1 &&
			( data[ col - 1 ] == 'r' || data[ col - 1 ] == 'R' )){
			//	厳密には直前がSHIFT_JISの2バイト目だと誤判定する可能性があるが
			//	そういう動かないコードは相手にしない
			m_raw_string = true;
		}
		else {
			m_raw_string = false;
		}
		if( col + 2 < linelen &&
			data[ col + 1 ] == quote_char &&
			data[ col + 2 ] == quote_char ){
				m_long_string = true;
				col += 2;
		}
		else {
			m_long_string = false;
		}
		++col;
	}
	return col;
}

/*! @brief Pythonプログラムの処理

	プログラム本体部分の処理．文字列の開始，継続行，コメント，通常行末をチェックする．
	行頭判定が終わった後で引き渡されるので，関数・クラス定義は考慮しなくて良い．
	
	以下の場合に処理を終了する
	- 行末: STATE_NORMALとして処理終了
	- コメント: STATE_NORMALとして処理終了
	- 文字列の開始: EnterString() にて文字列種別の判定を行った後STATE_STRINGとして処理終了
	- 継続行: STATE_CONTINUEとして処理終了

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置
	
	@invaliant
		m_state == STATE_NORMAL || m_state == STATE_CONTINUE
	
	@return 調査後の位置
*/
int COutlinePython::ScanNormal( const wchar_t* data, int linelen, int start_offset )
{
	assert( m_state == STATE_NORMAL || m_state == STATE_CONTINUE );

	for( int col = start_offset; col < linelen; ++col ){
		int nCharChars = CNativeW::GetSizeOfChar( data, linelen, col );
		if( 1 < nCharChars ){
			col += (nCharChars - 1);
			continue;
		}
		//	コメント
		if( data[col] == '#' ){
			//	コメントは行末と同じ扱いなので
			//	わざわざ独立して扱う必要性が薄い
			//	ここで片を付けてしまおう
			m_state = STATE_NORMAL;
			break;
		}
		//	文字列
		else if( data[col] == '\"' || data[col] == '\'' ){
			return EnterString( data, linelen, col );
		}
		else if( data[col] == '\\' ){	//	継続行かもしれない
			//	CRかCRLFかLFで行末
			//	最終行には改行コードがないことがあるが，それ以降には何もないので影響しない
			if(
				( linelen - 2 == col && 
				( data[ col + 1 ] == WCODE::CR && data[ col + 2 ] == WCODE::LF )) ||
				( linelen - 1 == col && 
				( data[ col + 1 ] == WCODE::CR || data[ col + 1 ] == WCODE::LF ))
			){
				m_state = STATE_CONTINUE;
				break;
			}
		}
	}
	return linelen;
}


/*! @brief python文字列(1行)を調査する

	与えられた状態からPython文字列の状態変化を追い，
	最終的な状態を決定する．
	
	文字列の開始判定はEnterString()関数で処理済みであり，その結果が
	m_state, m_raw_string, m_long_string, m_quote_charに与えられている．
	
	m_raw_stringがtrueならbackslashによるエスケープ処理を行わない
	m_long_stringならm_quote_charが3つ続くまで文字列となる．

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置
	
	@return 調査後の位置
	
	@invariant
		m_state==STATE_STRING

	@author genta
	@date 2007.02.12 新規作成
	@date 2007.03.23 genta 文字列の継続行の処理を追加

*/
int COutlinePython::ScanString( const wchar_t* data, int linelen, int start_offset )
{
	assert( m_state == STATE_STRING );

	int quote_char = m_quote_char;
	for( int col = start_offset; col < linelen; ++col ){
		int nCharChars = CNativeW::GetSizeOfChar( data, linelen, col );
		if( 1 < nCharChars ){
			col += (nCharChars - 1);
			continue;
		}
		//	rawモード以外ではエスケープをチェック
		//	rawモードでも継続行はチェック
		if( data[ col ] == '\\' && col + 1 < linelen ){
			wchar_t key = data[ col + 1 ];
			if( ! m_raw_string ){
				if( key == L'\\' ||
					key == L'\"' ||
					key == L'\'' ){
					++col;
					//	ignore
					continue;
				}
			}
			if( key == WCODE::CR || key == WCODE::LF ){
				// \r\nをまとめて\nとして扱う必要がある
				if( col + 1 >= linelen ||
					data[ col + 2 ] == key ){
					// 本当に行末
					++col;
					continue;
				}
				else if( data[ col + 2 ] == WCODE::LF ){
					col += 2;	//	 CRLF
				}
			}
		}
		//	short string + 改行の場合はエラーから強制復帰
		else if( data[ col ] == WCODE::CR || data[ col ] == WCODE::LF ){
			//あとで
			if( ! m_long_string ){
				//	文字列の末尾を発見した
				m_state = STATE_NORMAL;
				return col + 1;
			}
		}
		//	引用符が見つかったら終了チェック
		else if( data[ col ] == quote_char ){
			if( ! m_long_string ){
				//	文字列の末尾を発見した
				m_state = STATE_NORMAL;
				return col + 1;
			}
			//	long stringの場合
			if( col + 2 < linelen &&
				data[ col + 1 ] == quote_char &&
				data[ col + 2 ] == quote_char ){
				m_state = STATE_NORMAL;
				return col + 3;
			}
		}
	}
	return linelen;
}

/*!	Python文字列を行末までスキャンして次の行の状態を決定する

	m_stateに設定された現在の状態から開始してdataをstart_offsetからlinelenに達するまで
	走査し，行末における状態をm_stateに格納する．

	現在の状態に応じてサブルーチンに解析処理を依頼する．
	サブルーチンScan**では文字列dataのstart_offsetから状態遷移が発生するまで処理を
	続け，別の状態に遷移した直後に処理済みの桁位置を返して終了する．

	この関数に戻った後は再度現在の状態に応じて処理依頼を行う．これを行末に達するまで繰り返す．

	@param[in] data 対象文字列
	@param[in] linelen データの長さ
	@param[in] start_offset 調査開始位置

*/
void COutlinePython::DoScanLine( const wchar_t* data, int linelen, int start_offset )
{
	int col = start_offset;
	while( col < linelen ){
		if( STATE_NORMAL == m_state || STATE_CONTINUE == m_state){
			col = ScanNormal( data, linelen, col );
		}
		else if( STATE_STRING == m_state ){
			col = ScanString( data, linelen, col );
		}
		else {
			//	ありえないエラー
			return;
		}
	}
}


/*!	@brief python関数リスト作成

	class, def で始まる行から名前を抜き出す．
	
	class CLASS_NAME( superclass ):
	def FUNCTION_NAME( parameters ):

	文字列とコメントを除外する必要がある．

	通常の行頭の場合に関数・クラス判定と登録処理を行う．
	Python特有の空白の数を数えてネストレベルを判定する．
	indent_levelを配列として用いており，インデントレベルごとのスペース数を格納する．
	なお，TABは8桁区切りへの移動と解釈することになっている．
	
	通常の行頭でない(文字列中および継続行)，あるいは行頭の処理完了後は
	状態機械 python_analyze_state に判定処理を依頼する．

	@par 文字列
	'' "" 両方OK
	引用符3連続でロング文字列
	直前にrかRがあったらエスケープ記号を無視(ただし改行のエスケープは有効)
	
	@par コメント
	#で始まり，行の継続はない．

	@date 2007.02.08 genta 新規作成
*/
void CDocOutline::MakeFuncList_python( CFuncInfoArr* pcFuncInfoArr )
{
	CLogicInt	nLineLen;
	CLogicInt	nLineCount;

	COutlinePython python_analyze_state;

	const int MAX_DEPTH = 10;

	int indent_level[ MAX_DEPTH ]; // 各レベルのインデント桁位置()
	indent_level[0] = 0;	// do as python does.
	int depth_index = 0;

	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		const wchar_t*	pLine;
		int depth;	//	indent depth
		CLogicInt col = CLogicInt(0);	//	current working column position

		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		
		if( python_analyze_state.IsLogicalLineTop() ){
			//	indent check
			//	May 15, 2007 genta 桁位置colはデータオフセットdと独立にしないと
			//	文字列比較がおかしくなる
			for( depth = 0, col = CLogicInt(0); col < nLineLen; ++col ){
				//	calculate indent level
				if( pLine[col] == L' ' ){
					++depth;
				}
				else if( pLine[col] == L'\t' ){
					depth = ( depth + 8 ) & ~7;
				}
				else {
					break;
				}
			}
			if( pLine[col] == L'\r' ||
				pLine[col] == L'\n' ||
				pLine[col] == L'\0' ||
				pLine[col] == L'#' ){
				//	blank line or comment line are ignored
				continue;
			}
			
			int nItemFuncId = 0;	// topic type
			if( nLineLen - col > CLogicInt(3 + 1) && wcsncmp_literal( pLine + col, L"def" ) == 0 ){
				//	"def"
				nItemFuncId = 1;
				col += CLogicInt(3); // strlen( def )
			}
			else if( nLineLen - col > CLogicInt(5 + 1) && wcsncmp_literal( pLine + col, L"class" ) == 0 ){
				// class
				nItemFuncId = 4;
				col += CLogicInt(5); // strlen( class )
			}
			else {
				python_analyze_state.DoScanLine( pLine, nLineLen, col );
				continue;
			}

			//	2006.02.28 genta 区切りチェック
			//	define, classic等が対象にならないように，後ろにスペースかタブが
			//	続くことを確認．
			//	本当は継続行として次の行に関数名を書くことも文法上は可能だが
			//	複雑になるので対応しない．
			int c = pLine[col];
			if(  c != L' ' && c != L'\t' ){
				python_analyze_state.DoScanLine( pLine, nLineLen, col );
				continue;
			}

			//	adjust current depth level
			//	関数内部の実行文のさらに奧に関数があるケースを考慮
			//	def/class以外のインデントは記録しない方がいいので
			//	見出し行と確定してからインデントレベルの判定を行う
			int i;
			for( i = depth_index; i >= 0; --i ){
				if( depth == indent_level[ i ] ){
					depth_index = i;
					break;
				}
				else if( depth > indent_level[ i ] && i < MAX_DEPTH - 1 ){
					depth_index = i + 1;
					indent_level[ depth_index ] = depth;
					break;
				}
			}

			//	2007.02.08 genta 手抜きコメント
			//	厳密には，ここで継続行を入れることが可能だが，
			//	そんなレアなケースは考慮しない
			
			//	skip whitespace
			while( col < nLineLen && C_IsSpace( pLine[col] ))
				++col;

			int w_end;
			for( w_end = col; w_end < nLineLen
					&& Python_IsWordChar( pLine[w_end] ); ++w_end )
				;
			
			//	2007.02.08 genta 手抜きコメント
			//	厳密には，この後に括弧に囲まれた引数あるいは継承元クラスが
			//	括弧に囲まれて入り，さらに:と続くが
			//	継続行の可能性があるので，そこまでチェックしない
			
			//	ここまでで登録要件OKとみなす
			
			//	このあたりは暫定

			wchar_t szWord[512];	// 適当に大きな数(pythonでは名前の長さの上限があるのか？)
			int len = w_end - col;
			
			if( len > 0 ){
				if( len > _countof( szWord ) - 1){
					len = _countof( szWord ) - 1;
				}
				wcsncpy( szWord, pLine + col, len );
				szWord[ len ] = L'\0';
			}
			else {
				wcscpy( szWord, L"名称未定" );
				len = 8;
			}
			if( nItemFuncId == 4  ){
				if( _countof( szWord ) - 8  < len ){
					//	後ろを削って入れる
					len = _countof( szWord ) - 8;
				}
				// class
				wcscpy( szWord + len, L" クラス" );
			}
			
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/
			CLayoutPoint ptPosXY;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(CLogicInt(0), nLineCount),
				&ptPosXY
			);
			pcFuncInfoArr->AppendData(
				nLineCount + CLogicInt(1),
				ptPosXY.GetY2() + CLayoutInt(1),
				szWord,
				nItemFuncId,
				depth_index
			);
			col = CLogicInt(w_end); // クラス・関数定義の続きはここから
		}
		python_analyze_state.DoScanLine( pLine, nLineLen, col );
	}
}


/*! ルールファイルを読み込み、ルール構造体の配列を作成する

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca 引数nMaxCountを追加。バッファ長チェックをするように変更
*/
int CDocOutline::ReadRuleFile( const TCHAR* pszFilename, SOneRule* pcOneRule, int nMaxCount )
{
	long	i;
	// 2003.06.23 Moca 相対パスは実行ファイルからのパスとして開く
	// 2007.05.19 ryoji 相対パスは設定ファイルからのパスを優先
	FILE*	pFile = _tfopen_absini( pszFilename, _T("r") );
	if( NULL == pFile ){
		return 0;
	}
	wchar_t	szLine[LINEREADBUFSIZE];
	const wchar_t*	pszDelimit = L" /// ";
	const wchar_t*	pszKeySeps = L",\0";
	wchar_t*	pszWork;
	int nDelimitLen = wcslen( pszDelimit );
	int nCount = 0;
	while( NULL != fgetws( szLine, _countof(szLine), pFile ) && nCount < nMaxCount ){
		pszWork = wcsstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != L';' ){
			*pszWork = L'\0';
			pszWork += nDelimitLen;

			/* 最初のトークンを取得します。 */
			wchar_t* pszToken = wcstok( szLine, pszKeySeps );
			while( NULL != pszToken ){
//				nRes = wcsicmp( pszKey, pszToken );
				for( i = 0; i < (int)wcslen(pszWork); ++i ){
					if( pszWork[i] == L'\r' ||
						pszWork[i] == L'\n' ){
						pszWork[i] = L'\0';
						break;
					}
				}
				wcsncpy( pcOneRule[nCount].szMatch, pszToken, 255 );
				wcsncpy( pcOneRule[nCount].szGroupName, pszWork, 255 );
				pcOneRule[nCount].szMatch[255] = L'\0';
				pcOneRule[nCount].szGroupName[255] = L'\0';
				pcOneRule[nCount].nLength = wcslen(pcOneRule[nCount].szMatch);
				nCount++;
				pszToken = wcstok( NULL, pszKeySeps );
			}
		}
	}
	fclose( pFile );
	return nCount;
}

/*! ルールファイルを元に、トピックリストを作成

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca ネストの深さが最大値を超えるとバッファオーバーランするのを修正
		最大値以上は追加せずに無視する
	@date 2007.11.29 kobake SOneRule test[1024] でスタックが溢れていたのを修正
*/
void CDocOutline::MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr )
{
	wchar_t*		pszText;

	/* ルールファイルの内容をバッファに読み込む */
	auto_array_ptr<SOneRule> test = new SOneRule[1024];	// 1024個許可。 2007.11.29 kobake スタック使いすぎなので、ヒープに確保するように修正。
	int nCount = ReadRuleFile(m_pcDocRef->m_cDocType.GetDocumentAttribute().m_szOutlineRuleFilename, test.get(), 1024 );
	if ( nCount < 1 ){
		return;
	}

	/*	ネストの深さは、32レベルまで、ひとつのヘッダは、最長256文字まで区別
		（256文字まで同じだったら同じものとして扱います）
	*/
	const int	nMaxStack = 32;	//	ネストの最深
	int			nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t		pszStack[nMaxStack][256];
	wchar_t		szTitle[256];			//	一時領域
	for( CLogicInt nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//行取得
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}

		//行頭の空白飛ばし
		int		i;
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == L' ' || pLine[i] == L'\t' || pLine[i] == L'　'){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//先頭文字が見出し記号のいずれかであれば、次へ進む
		int		j;
		for( j = 0; j < nCount; j++ ){
			if ( 0 == wcsncmp( &pLine[i], test[j].szMatch, test[j].nLength ) ){
				wcscpy( szTitle, test[j].szGroupName );
				break;
			}
		}
		if( j >= nCount ){
			continue;
		}

		/*	ルールにマッチした行は、アウトライン結果に表示する。
		*/

		//行文字列から改行を取り除く pLine -> pszText
		pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		int nTextLen = wcslen( pszText );
		for( i = 0; i < nTextLen; ++i ){
			if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
				pszText[i] = L'\0';
				break;
			}
		}

		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		CLayoutPoint ptPos;
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepthを計算 */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	ループ途中でbreak;してきた。＝今までに同じ見出しが存在していた。
			//	ので、同じレベルに合わせてAppendData.
			nDepth = k;
		}
		else if( nMaxStack> k ){
			//	いままでに同じ見出しが存在しなかった。
			//	ので、pszStackにコピーしてAppendData.
			wcscpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーランするから規制する
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}



/*! COBOL アウトライン解析 */
void CDocOutline::MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				k;
	wchar_t			szDivision[1024];
	wchar_t			szLabel[1024];
	const wchar_t*	pszKeyWord;
	int				nKeyWordLen;
	BOOL			bDivision;

	szDivision[0] = L'\0';
	szLabel[0] =  L'\0';


	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}
		/* コメント行か */
		if( 7 <= nLineLen && pLine[6] == L'*' ){
			continue;
		}
		/* ラベル行か */
		if( 8 <= nLineLen && pLine[7] != L' ' ){
			k = 0;
			for( i = 7; i < nLineLen; ){
				if( pLine[i] == '.'
				 || pLine[i] == WCODE::CR
				 || pLine[i] == WCODE::LF
				){
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if( pLine[i - 1] == L' ' ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != L' ' ){
							break;
						}
					}
				}
			}
			szLabel[k] = L'\0';
//			MYTRACE_A( "szLabel=[%ls]\n", szLabel );



			pszKeyWord = L"division";
			nKeyWordLen = wcslen( pszKeyWord );
			bDivision = FALSE;
			for( i = 0; i <= (int)wcslen( szLabel ) - nKeyWordLen; ++i ){
				if( 0 == auto_memicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					szLabel[i + nKeyWordLen] = L'\0';
					wcscpy( szDivision, szLabel );
					bDivision = TRUE;
					break;
				}
			}
			if( bDivision ){
				continue;
			}
			/*
			  カーソル位置変換
			  物理位置(行頭からのバイト数、折り返し無し行位置)
			  →
			  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
			*/

			CLayoutPoint ptPos;
			wchar_t	szWork[1024];
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			auto_sprintf( szWork, L"%ls::%ls", szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , szWork, 0 );
		}
	}
	return;
}


/*! アセンブラ アウトライン解析

	@author MIK
	@date 2004.04.12 作り直し
*/
void CDocOutline::MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr )
{
	CLogicInt nTotalLine;

	nTotalLine = m_pcDocRef->m_cDocLineMgr.GetLineCount();

	for( CLogicInt nLineCount = CLogicInt(0); nLineCount < nTotalLine; nLineCount++ ){
		const WCHAR* pLine;
		CLogicInt nLineLen;
		WCHAR* pTmpLine;
		int length;
		int offset;
#define MAX_ASM_TOKEN 2
		WCHAR* token[MAX_ASM_TOKEN];
		int j;
		WCHAR* p;

		//1行取得する。
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( pLine == NULL ) break;

		//作業用にコピーを作成する。バイナリがあったらその後ろは知らない。
		pTmpLine = wcsdup( pLine );
		if( pTmpLine == NULL ) break;
		if( wcslen( pTmpLine ) >= (unsigned int)nLineLen ){	//バイナリを含んでいたら短くなるので...
			pTmpLine[ nLineLen ] = L'\0';	//指定長で切り詰め
		}

		//行コメント削除
		p = wcsstr( pTmpLine, L";" );
		if( p ) *p = L'\0';

		length = wcslen( pTmpLine );
		offset = 0;

		//トークンに分割
		for( j = 0; j < MAX_ASM_TOKEN; j++ ) token[ j ] = NULL;
		for( j = 0; j < MAX_ASM_TOKEN; j++ ){
			token[ j ] = my_strtok<WCHAR>( pTmpLine, length, &offset, L" \t\r\n" );
			if( token[ j ] == NULL ) break;
			//トークンに含まれるべき文字でないか？
			if( wcsstr( token[ j ], L"\"") != NULL
			 || wcsstr( token[ j ], L"\\") != NULL
			 || wcsstr( token[ j ], L"'" ) != NULL ){
				token[ j ] = NULL;
				break;
			}
		}

		if( token[ 0 ] != NULL ){	//トークンが1個以上ある
			int nFuncId = -1;
			WCHAR* entry_token = NULL;

			length = wcslen( token[ 0 ] );
			if( length >= 2
			 && token[ 0 ][ length - 1 ] == L':' ){	//ラベル
				token[ 0 ][ length - 1 ] = L'\0';
				nFuncId = 51;
				entry_token = token[ 0 ];
			}
			else if( token[ 1 ] != NULL ){	//トークンが2個以上ある
				if( wcsicmp( token[ 1 ], L"proc" ) == 0 ){	//関数
					nFuncId = 50;
					entry_token = token[ 0 ];
				}else
				if( wcsicmp( token[ 1 ], L"endp" ) == 0 ){	//関数終了
					nFuncId = 52;
					entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("macro") ) == 0 ){	//マクロ
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("struc") ) == 0 ){	//構造体
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				}
			}

			if( nFuncId >= 0 ){
				/*
				  カーソル位置変換
				  物理位置(行頭からのバイト数、折り返し無し行位置)
				  →
				  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
				*/
				CLayoutPoint ptPos;
				m_pcDocRef->m_cLayoutMgr.LogicToLayout(
					CLogicPoint(0, nLineCount),
					&ptPos
				);
				pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1), entry_token, nFuncId );
			}
		}

		free( pTmpLine );
	}

	return;
}



/*! 階層付きテキスト アウトライン解析

	@author zenryaku
	@date 2003.05.20 zenryaku 新規作成
	@date 2003.05.25 genta 実装方法一部修正
	@date 2003.06.21 Moca 階層が2段以上深くなる場合を考慮
*/
void CDocOutline::MakeTopicList_wztxt(CFuncInfoArr* pcFuncInfoArr)
{
	int levelPrev = 0;

	for(CLogicInt nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const wchar_t*	pLine;
		CLogicInt		nLineLen;

		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta 判定順序変更
		if( *pLine == L'.' )
		{
			const wchar_t* pPos;	//	May 25, 2003 genta
			int			nLength;
			wchar_t		szTitle[1024];

			//	ピリオドの数＝階層の深さを数える
			for( pPos = pLine + 1 ; *pPos == L'.' ; ++pPos )
				;

			CLayoutPoint ptPos;
			m_pcDocRef->m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			
			int level = pPos - pLine;

			// 2003.06.27 Moca 階層が2段位上深くなるときは、無題の要素を追加
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (無題)を挿入
				//	ただし，TAG一覧には出力されないように
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData(
						nLineCount+CLogicInt(1),
						ptPos.GetY2()+CLayoutInt(1),
						L"(無題)",
						FUNCINFO_NOCLIPTEXT,
						dummyLevel - 1
					);
				}
			}
			levelPrev = level;

			nLength = auto_sprintf(szTitle,L"%d - ", level );
			
			wchar_t *pDest = szTitle + nLength; // 書き込み先
			wchar_t *pDestEnd = szTitle + _countof(szTitle) - 2;
			
			while( pDest < pDestEnd )
			{
				if( *pPos ==L'\r' || *pPos ==L'\n' || *pPos == L'\0')
				{
					break;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = L'\0';
			pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle, 0, level - 1);
		}
	}
}

/*! HTML アウトライン解析

	@author zenryaku
	@date 2003.05.20 zenryaku 新規作成
	@date 2004.04.19 zenryaku 空要素を判定
	@date 2004.04.20 Moca コメント処理と、不明な終了タグを無視する処理を追加
*/
void CDocOutline::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;
	BOOL			bEndTag;
	BOOL			bCommentTag = FALSE;

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	一時領域
	CLogicInt			nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine)
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			// 2004.04.20 Moca コメントを処理する
			if( bCommentTag )
			{
				if( i < nLineLen - 3 && 0 == wmemcmp( L"-->", pLine + i , 3 ) )
				{
					bCommentTag = FALSE;
					i += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if(pLine[i]!=L'<' || nDepth>=nMaxStack)
			{
				continue;
			}
			bEndTag	=	FALSE;
			if(pLine[++i]==L'/')
			{
				i++;
				bEndTag	=	TRUE;
			}
			for(j=0;i+j<nLineLen && j<_countof(szTitle)-1;j++)
			{
				if((pLine[i+j]<L'a' || pLine[i+j]>L'z') &&
					(pLine[i+j]<L'A' || pLine[i+j]>L'Z') &&
					!(j!=0 && pLine[i+j]>=L'0' && pLine[i+j]<=L'9'))
				{
					break;
				}
				szTitle[j]	=	pLine[i+j];
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here コメントを処理する
				if( i < nLineLen - 3 && 0 == wmemcmp( L"!--", pLine + i, 3 ) )
				{
					bCommentTag = TRUE;
					i += 3;
				}
				// 2004.04.20 Moca To Here
				continue;
			}
			szTitle[j]	=	'\0';
			if(bEndTag)
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca 追加
				// 終了タグ
				while(nDepth>0)
				{
					nDepth--;
					if(!wcsicmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca ツリー中と一致しないときは、この終了タグは無視
				if( nDepth == 0 )
				{
					if(wcsicmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}
			}
			else
			{
				if(wcsicmp(szTitle,L"br") && wcsicmp(szTitle,L"area") &&
					wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
				{
					CLayoutPoint ptPos;

					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(i, nLineCount),
						&ptPos
					);

					if(wcsicmp(szTitle,L"hr") && wcsicmp(szTitle,L"meta") && wcsicmp(szTitle,L"link") &&
						wcsicmp(szTitle,L"input") && wcsicmp(szTitle,L"img") && wcsicmp(szTitle,L"area") &&
						wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
					{
						// 終了タグなしを除く全てのタグらしきものを判定
						wcscpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<_countof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if(pLine[i+j]==L'/' && pLine[i+j+1]==L'>')
								{
									bEndTag	=	TRUE;
									break;
								}
								else if(pLine[i+j]==L'>')
								{
									break;
								}
							}
							if(!bEndTag)
							{
								szTitle[k++]	=	L' ';
								for(j-=k-1;i+j+k<nLineLen && k<_countof(szTitle)-1;k++)
								{
									if(pLine[i+j+k]==L'<' || pLine[i+j+k]==L'\r' || pLine[i+j+k]==L'\n')
									{
										break;
									}
									szTitle[k]	=	pLine[i+j+k];
								}
							j += k-1;
							}
						}
						szTitle[k]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,(bEndTag ? nDepth : nDepth++));
					}
					else
					{
						for(;i+j<nLineLen && j<_countof(szTitle)-1;j++)
						{
							if(pLine[i+j]=='>')
							{
								break;
							}
							szTitle[j]	=	pLine[i+j];
						}
						szTitle[j]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,nDepth);
					}
				}
			}
			i	+=	j;
		}
	}
}

/*! TeX アウトライン解析

	@author naoh
	@date 2003.07.21 naoh 新規作成
	@date 2005.01.03 naoh 「マ」などの"}"を含む文字に対する修正、prosperのslideに対応
*/
void CDocOutline::MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;

	const int nMaxStack = 8;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t szTag[32], szTitle[256];			//	一時領域
	int thisSection=0, lastSection = 0;	// 現在のセクション種類と一つ前のセクション種類
	int stackSection[nMaxStack];		// 各深さでのセクションの番号
	int nStartTitlePos;					// \section{dddd} の dddd の部分の始まる番号
	int bNoNumber;						// * 付の場合はセクション番号を付けない

	// 一行ずつ
	CLogicInt	nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_pcDocRef->m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if(!pLine) break;
		// 一文字ずつ
		for(i=0;i<nLineLen-1;i++)
		{
			if(pLine[i] == L'%') break;	// コメントなら以降はいらない
			if(nDepth>=nMaxStack)continue;
			if(pLine[i] != L'\\')continue;	// 「\」がないなら次の文字へ
			++i;
			// 見つかった「\」以降の文字列チェック
			for(j=0;i+j<nLineLen && j<_countof(szTag)-1;j++)
			{
				if(pLine[i+j] == L'{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1チェック
					bNoNumber = (pLine[i+j-1] == '*');
					nStartTitlePos = j+i+1;
					break;
				}
				szTag[j] = pLine[i+j];
			}
			if(j==0) continue;
			if(bNoNumber){
				szTag[j-1] = L'\0';
			}else{
				szTag[j]   = L'\0';
			}
//			MessageBoxA(NULL, szTitle, L"", MB_OK);

			thisSection = 0;
			if(!wcscmp(szTag,L"subsubsection")) thisSection = 4;
			else if(!wcscmp(szTag,L"subsection")) thisSection = 3;
			else if(!wcscmp(szTag,L"section")) thisSection = 2;
			else if(!wcscmp(szTag,L"chapter")) thisSection = 1;
			else if(!wcscmp(szTag,L"begin")) {		// beginなら prosperのslideの可能性も考慮
				// さらに{slide}{}まで読みとっておく
				if(wcsstr(pLine, L"{slide}")){
					k=0;
					for(j=nStartTitlePos+1;i+j<nLineLen && j<_countof(szTag)-1;j++)
					{
						if(pLine[i+j] == '{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1チェック
							nStartTitlePos = j+i+1;
							break;
						}
						szTag[k++]	=	pLine[i+j];
					}
					szTag[k] = '\0';
					thisSection = 1;
				}
			}

			if( thisSection > 0)
			{
				// sectionの中身取得
				for(k=0;nStartTitlePos+k<nLineLen && k<_countof(szTitle)-1;k++)
				{
					if(_IS_SJIS_1((unsigned char)pLine[k+nStartTitlePos])) {
						szTitle[k] = pLine[k+nStartTitlePos];
						k++;	// 次はチェック不要
					} else if(pLine[k+nStartTitlePos] == '}') {
						break;
					}
					szTitle[k] = pLine[k+nStartTitlePos];
				}
				szTitle[k] = '\0';

				CLayoutPoint ptPos;

				WCHAR tmpstr[256];
				WCHAR secstr[4];

				m_pcDocRef->m_cLayoutMgr.LogicToLayout(
					CLogicPoint(i, nLineCount),
					&ptPos
				);

				int sabunSection = thisSection - lastSection;
				if(lastSection == 0){
					nDepth = 0;
					stackSection[0] = 1;
				}else{
					nDepth += sabunSection;
					if(sabunSection > 0){
						if(nDepth >= nMaxStack) nDepth=nMaxStack-1;
						stackSection[nDepth] = 1;
					}else{
						if(nDepth < 0) nDepth=0;
						++stackSection[nDepth];
					}
				}
				tmpstr[0] = L'\0';
				if(!bNoNumber){
					for(k=0; k<=nDepth; k++){
						auto_sprintf(secstr, L"%d.", stackSection[k]);
						wcscat(tmpstr, secstr);
					}
					wcscat(tmpstr, L" ");
				}
				wcscat(tmpstr, szTitle);
				pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1), tmpstr, 0, nDepth);
				if(!bNoNumber) lastSection = thisSection;
			}
			i	+=	j;
		}
	}
}






/*! PL/SQL関数リスト作成 */
void CDocOutline::MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	wchar_t		szFuncName[80];
	CLogicInt	nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	nFuncNum = 0;
	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL( &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
//			if( 1 < nCharChars ){
//				i += (nCharChars - 1);
//				continue;
//			}
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( L'\'' == pLine[i] ){
					if( i + 1 < nLineLen && L'\'' == pLine[i + 1] ){
						++i;
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i + 1 < nLineLen && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
					) )
				 || 2 == nCharChars
				){
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
				else{
					if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"FUNCTION" ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PROCEDURE" ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PACKAGE" ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == wcsicmp( szWord, L"BODY" ) ){
						nFuncOrProc = 4;
						nParseCnt = 1;
					}
					else if( 1 == nParseCnt ){
						if( 1 == nFuncOrProc ||
							2 == nFuncOrProc ||
							3 == nFuncOrProc ||
							4 == nFuncOrProc ){
							++nParseCnt;
							wcscpy( szFuncName, szWord );
						}else
						if( 3 == nFuncOrProc ){

						}
					}else
					if( 2 == nParseCnt ){
						if( 0 == wcsicmp( szWord, L"IS" ) ){
							if( 1 == nFuncOrProc ){
								nFuncId = 11;	/* ファンクション本体 */
							}else
							if( 2 == nFuncOrProc ){
								nFuncId = 21;	/* プロシージャ本体 */
							}else
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* パッケージ仕様部 */
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* パッケージ本体 */
							}
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							CLayoutPoint ptPos;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(0, nFuncLine - 1),
								&ptPos
							);
							pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1), szFuncName, nFuncId );
							nParseCnt = 0;
						}
						if( 0 == wcsicmp( szWord, L"AS" ) ){
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* パッケージ仕様部 */
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
								*/
								CLayoutPoint ptPos;
								m_pcDocRef->m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
							else if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* パッケージ本体 */
								++nFuncNum;
								/*
								  カーソル位置変換
								  物理位置(行頭からのバイト数、折り返し無し行位置)
								  →
								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
								*/
								CLayoutPoint ptPos;
								m_pcDocRef->m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
						}
					}
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
				if( L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'\t' == pLine[i] ||
					 L' ' == pLine[i] ||
					 WCODE::CR == pLine[i] ||
					 WCODE::LF == pLine[i] ||
					 L'{' == pLine[i] ||
					 L'}' == pLine[i] ||
					 L'(' == pLine[i] ||
					 L')' == pLine[i] ||
					 L';' == pLine[i] ||
					L'\'' == pLine[i] ||
					 L'/' == pLine[i] ||
					 L'-' == pLine[i]
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}else{
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'-' == pLine[i] &&  L'-' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L';' == pLine[i] ){
					if( 2 == nParseCnt ){
						if( 1 == nFuncOrProc ){
							nFuncId = 10;	/* ファンクション宣言 */
						}else{
							nFuncId = 20;	/* プロシージャ宣言 */
						}
						++nFuncNum;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						CLayoutPoint ptPos;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							CLogicPoint(0, nFuncLine - 1),
							&ptPos
						);
						pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
						nParseCnt = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( (1 == nCharChars && (
						L'_' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
						) )
					 || 2 == nCharChars
					){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}





/*!	テキスト・トピックリスト作成
	
	@date 2002.04.01 YAZAKI CDlgFuncList::SetText()を使用するように改訂。
	@date 2002.11.03 Moca	階層が最大値を超えるとバッファオーバーランするのを修正
							最大値以上は追加せずに無視する
	@date 2007.8頃   kobake 機械的にUNICODE化
	@date 2007.11.29 kobake UNICODE対応できてなかったので修正
*/
void CDocOutline::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	using namespace WCODE;

	//見出し記号
	const wchar_t*	pszStarts = GetDllShareData().m_Common.m_sFormat.m_szMidashiKigou;
	int				nStartsLen = wcslen( pszStarts );

	/*	ネストの深さは、nMaxStackレベルまで、ひとつのヘッダは、最長32文字まで区別
		（32文字まで同じだったら同じものとして扱います）
	*/
	const int nMaxStack = 32;	//	ネストの最深
	int nDepth = 0;				//	いまのアイテムの深さを表す数値。
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	一時領域
	CLogicInt				nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//行取得
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine )break;

		//行頭の空白飛ばし
		int i;
		for( i = 0; i < nLineLen; ++i ){
			if( WCODE::isBlank(pLine[i]) ){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//先頭文字が見出し記号のいずれかであれば、次へ進む
		if(NULL==wcschr(pszStarts,pLine[0]))continue;

		//見出し種類の判別 -> szTitle
		if( pLine[i] == L'(' ){
			if(0){}
			else if ( IsInRange(pLine[i + 1], L'0', L'9') ) wcscpy( szTitle, L"(0)" ); //数字
			else if ( IsInRange(pLine[i + 1], L'A', L'Z') ) wcscpy( szTitle, L"(A)" ); //英大文字
			else if ( IsInRange(pLine[i + 1], L'a', L'z') ) wcscpy( szTitle, L"(a)" ); //英小文字
			else continue; //※「(」の次が英数字で無い場合、見出しとみなさない
		}
		else if( IsInRange(pLine[i], L'０', L'９') ) wcscpy( szTitle, L"０" ); // 全角数字
		else if( IsInRange(pLine[i], L'①', L'⑳') ) wcscpy( szTitle, L"①" ); // ①～⑳
		else if( IsInRange(pLine[i], L'Ⅰ', L'Ⅹ') ) wcscpy( szTitle, L"Ⅰ" ); // Ⅰ～Ⅹ
		else if( wcschr(L"〇一二三四五六七八九十百零壱弐参伍", pLine[i]) ) wcscpy( szTitle, L"一" ); //漢数字
		else{
			szTitle[0]=pLine[i];
			szTitle[1]=L'\0';
		}

		/*	「見出し記号」に含まれる文字で始まるか、
			(0、(1、...(9、(A、(B、...(Z、(a、(b、...(z
			で始まる行は、アウトライン結果に表示する。
		*/

		//行文字列から改行を取り除く pLine -> pszText
		wchar_t*	pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		for( i = 0; i < (int)wcslen(pszText); ++i ){
			if( pszText[i] == CR || pszText[i] == LF )pszText[i] = L'\0';
		}

		/*
		  カーソル位置変換
		  物理位置(行頭からのバイト数、折り返し無し行位置)
		  →
		  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
		*/
		CLayoutPoint ptPos;
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepthを計算 */
		int k;
		bool bAppend = true;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	ループ途中でbreak;してきた。＝今までに同じ見出しが存在していた。
			//	ので、同じレベルに合わせてAppendData.
			nDepth = k;
		}
		else if( nMaxStack > k ){
			//	いままでに同じ見出しが存在しなかった。
			//	ので、pszStackにコピーしてAppendData.
			wcscpy(pszStack[nDepth], szTitle);
		}
		else{
			// 2002.11.03 Moca 最大値を超えるとバッファオーバーラン
			// nDepth = nMaxStack;
			bAppend = false;
		}
		
		if( bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}



/* Java関数リスト作成 */
void CDocOutline::MakeFuncList_Java( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt	nLineLen;
	int			i;
	int			nNestLevel;
	int			nCharChars;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	wchar_t		szFuncName[80];
	CLogicInt	nFuncLine = CLogicInt(0);
	int			nFuncId;
	int			nFuncNum;
	wchar_t		szClass[1024];

	int			nClassNestArr[16];
	int			nClassNestArrNum;
	int			nNestLevel2Arr[16];

	nNestLevel = 0;
	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	nNestLevel2Arr[0] = 0;
	nFuncNum = 0;
	szClass[0] = L'\0';
	nClassNestArrNum = 0;
	CLogicInt		nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			//nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				i += (nCharChars - 1);
				continue;
			}

			/* エスケープシーケンスは常に取り除く */
			if( L'\\' == pLine[i] ){
				++i;
			}else
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( L'\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* ダブルクォーテーション文字列読み込み中 */
			if( 21 == nMode ){
				if( L'"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* コメント読み込み中 */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}
			/* 単語読み込み中 */
			else if( 1 == nMode ){
				if( L'_' == pLine[i] ||
					L':' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'.' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}
else{
					/* クラス宣言部分を見つけた */
					//	Oct. 10, 2002 genta interfaceも対象に
					if( 0 == wcscmp( L"class", szWordPrev ) ||
						0 == wcscmp( L"interface", szWordPrev )
					 ){
						nClassNestArr[nClassNestArrNum] = nNestLevel;
						++nClassNestArrNum;
						if( 0 < nNestLevel	){
							wcscat( szClass, L"\\" );
						}
						wcscat( szClass, szWord );

						nFuncId = 0;
						++nFuncNum;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						CLogicPoint  ptPosXY_Logic = CLogicPoint(CLogicInt(0), nLineCount);
						CLayoutPoint ptPosXY_Layout;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(
							ptPosXY_Logic,
							&ptPosXY_Layout
						);
						wchar_t szWork[256];
						auto_sprintf( szWork, L"%ls::%ls", szClass, L"定義位置" );
						pcFuncInfoArr->AppendData( ptPosXY_Logic.GetY2() + CLogicInt(1), ptPosXY_Layout.GetY2() + CLayoutInt(1), szWork, nFuncId ); //2007.10.09 kobake レイアウト・ロジックの混在バグ修正
					}

					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
				if( L'_' == pLine[i] ||
					L':' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i] ||
					L'{' == pLine[i] ||
					L'}' == pLine[i] ||
					L'(' == pLine[i] ||
					L')' == pLine[i] ||
					L';' == pLine[i]	||
					L'\'' == pLine[i] ||
					L'"' == pLine[i] ||
					L'/' == pLine[i] ||
					L'.' == pLine[i]
				){
					nMode = 0;
					i--;
					continue;
				}else{
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'/' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L'"' == pLine[i] ){
					nMode = 21;
					continue;
				}else
				if( L'{' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						//	メソッド中でさらにメソッドを定義することはないので
						//	ネストレベル判定追加 class/interfaceの直下の場合のみ判定する
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != wcscmp( L"sizeof", szFuncName )
						 && 0 != wcscmp( L"if", szFuncName )
						 && 0 != wcscmp( L"for", szFuncName )
						 && 0 != wcscmp( L"do", szFuncName )
						 && 0 != wcscmp( L"while", szFuncName )
						 && 0 != wcscmp( L"catch", szFuncName )
						 && 0 != wcscmp( L"switch", szFuncName )
						 && 0 != wcscmp( L"return", szFuncName )
						){
							nFuncId = 2;
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							CLayoutPoint ptPosXY;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)),
								&ptPosXY
							);
							wchar_t szWork[256];
							auto_sprintf( szWork, L"%ls::%ls", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1), szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					++nNestLevel;
					nMode = 0;
					continue;
				}else
				if( L'}' == pLine[i] ){
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}

					nNestLevel--;
					if( 0 < nClassNestArrNum &&
						nClassNestArr[nClassNestArrNum - 1] == nNestLevel
					){
						nClassNestArrNum--;
						int k;
						for( k = wcslen( szClass ) - 1; k >= 0; k-- ){
							if( L'\\' == szClass[k] ){
								break;
							}
						}
						if( 0 > k ){
							k = 0;
						}
						szClass[k] = L'\0';
					}
					nMode = 0;
					continue;
				}else
				if( L'(' == pLine[i] ){
					if( 0 < nClassNestArrNum /*nNestLevel == 1*/ &&
						0 != wcscmp( L"new", szWordPrev )
					){
						wcscpy( szFuncName, szWord );
						nFuncLine = nLineCount + CLogicInt(1);
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 1;
						}
					}
					nMode = 0;
					continue;
				}else
				if( L')' == pLine[i] ){
					int			k;
					const wchar_t*	pLine2;
					CLogicInt		nLineLen2;
					CLogicInt	nLineCount2;
					nLineCount2 = nLineCount;
					pLine2 = pLine;
					nLineLen2 = nLineLen;
					k = i + 1;
					BOOL		bCommentLoop;
					bCommentLoop = FALSE;
				loop_is_func:;
					for( ; k < nLineLen2; ++k ){
						if( !bCommentLoop ){
							if( pLine2[k] != L' ' && pLine2[k] != WCODE::TAB && pLine2[k] != WCODE::CR && pLine2[k] != WCODE::LF ){
								if( k + 1 < nLineLen2 && pLine2[k] == L'/' && pLine2[k + 1] == L'*' ){
									bCommentLoop = TRUE;
									++k;
								}else
								if( k + 1 < nLineLen2 && pLine2[k] == L'/' && pLine2[k + 1] == L'/' ){
									k = nLineLen2 + 1;
									break;
								}else{
									break;
								}
							}
						}else{
							if( k + 1 < nLineLen2 && pLine2[k] == L'*' && pLine2[k + 1] == L'/' ){
								bCommentLoop = FALSE;
								++k;
							}
						}
					}
					if( k >= nLineLen2 ){
						k = 0;
						++nLineCount2;
						pLine2 = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount2)->GetDocLineStrWithEOL(&nLineLen2);
						if( NULL != pLine2 ){
							goto loop_is_func;
						}
						if( 0 < nClassNestArrNum ){
							nNestLevel2Arr[nClassNestArrNum - 1] = 0;
						}
					}else{
						//	Oct. 10, 2002 genta
						//	abscract にも対応
						if( pLine2[k] == L'{' || pLine2[k] == L';' ||
							__iscsym( pLine2[k] ) ){
							if( 0 < nClassNestArrNum ){
								if( 1 == nNestLevel2Arr[nClassNestArrNum - 1] ){
									nNestLevel2Arr[nClassNestArrNum - 1] = 2;
								}
							}
						}else{
							if( 0 < nClassNestArrNum ){
								nNestLevel2Arr[nClassNestArrNum - 1] = 0;
							}
						}
					}
					nMode = 0;
					continue;
				}else
				if( L';' == pLine[i] ){
					if( 0 < nClassNestArrNum && 2 == nNestLevel2Arr[nClassNestArrNum - 1] ){
						//	Oct. 10, 2002 genta
						// 関数の中で別の関数の宣言部を使うことって，Javaであるの？
						if( nClassNestArr[nClassNestArrNum - 1] == nNestLevel - 1
						 && 0 != wcscmp( L"sizeof", szFuncName )
						 && 0 != wcscmp( L"if", szFuncName )
						 && 0 != wcscmp( L"for", szFuncName )
						 && 0 != wcscmp( L"do", szFuncName )
						 && 0 != wcscmp( L"while", szFuncName )
						 && 0 != wcscmp( L"catch", szFuncName )
						 && 0 != wcscmp( L"switch", szFuncName )
						 && 0 != wcscmp( L"return", szFuncName )
						){
							nFuncId = 1;
							++nFuncNum;
							/*
							  カーソル位置変換
							  物理位置(行頭からのバイト数、折り返し無し行位置)
							  →
							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
							*/
							CLayoutPoint ptPosXY;
							m_pcDocRef->m_cLayoutMgr.LogicToLayout(
								CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)),
								&ptPosXY
							);
							wchar_t szWork[256];
							auto_sprintf( szWork, L"%ls::%ls", szClass, szFuncName );
							pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1), szWork, nFuncId );
						}
					}
					if( 0 < nClassNestArrNum ){
						nNestLevel2Arr[nClassNestArrNum - 1] = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( L'_' == pLine[i] ||
						L':' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
						L'.' == pLine[i]
					){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
						nMode = 1;
					}else{
						nMode = 0;
					}
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}




//	From Here Sep 8, 2000 genta
//
//!	Perl用アウトライン解析機能（簡易版）
/*!
	単純に /^\\s*sub\\s+(\\w+)/ に一致したら $1を取り出す動作を行う。
	ネストとかは面倒くさいので考えない。
	package{ }を使わなければこれで十分．無いよりはまし。

	@par nModeの意味
	@li 0: はじめ
	@li 2: subを見つけた後
	@li 1: 単語読み出し中

	@date 2005.06.18 genta パッケージ区切りを表す ::と'を考慮するように
*/
void CDocOutline::MakeFuncList_Perl( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt			nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;

	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		nMode = 0;
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	1 < nCharChars ){
				break;
			}

			/* 単語読み込み中 */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}
				if( 's' != pLine[i] )
					break;
				//	sub の一文字目かもしれない
				if( nLineLen - i < 4 )
					break;
				if( wcsncmp_literal( pLine + i, L"sub" ) )
					break;
				int c = pLine[ i + 3 ];
				if( c == L' ' || c == L'\t' ){
					nMode = 2;	//	発見
					i += 3;
				}
				else
					break;
			}
			else if( 2 == nMode ){
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
				){
					//	関数名の始まり
					nWordIdx = 0;
					szWord[nWordIdx] = pLine[i];
					szWord[nWordIdx + 1] = L'\0';
					nMode = 1;
					continue;
				}
				else
					break;

			}
			else if( 1 == nMode ){
				if( L'_' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					//	Jun. 18, 2005 genta パッケージ修飾子を考慮
					//	コロンは2つ連続しないといけないのだが，そこは手抜き
					L':' == pLine[i] || L'\'' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						break;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = L'\0';
					}
				}else{
					//	関数名取得
					/*
					  カーソル位置変換
					  物理位置(行頭からのバイト数、折り返し無し行位置)
					  →
					  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
					*/
					CLayoutPoint ptPosXY;
					m_pcDocRef->m_cLayoutMgr.LogicToLayout(
						CLogicPoint(CLogicInt(0), nLineCount),
						&ptPosXY
					);
					//	Mar. 9, 2001
					pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPosXY.GetY2() + CLayoutInt(1), szWord, 0 );

					break;
				}
			}
		}
	}
#ifdef _DEBUG
	pcFuncInfoArr->DUMP();
#endif
	return;
}
//	To HERE Sep. 8, 2000 genta








//	From Here June 23, 2001 N.Nakatani
//!	Visual Basic関数リスト作成（簡易版）
/*!
	Visual Basicのコードから単純にユーザー定義の関数やステートメントを取り出す動作を行う。

    Jul 10, 2003 little YOSHI  細かく解析するように変更
                               すべてのキーワードは自動的に成形されるので、大文字小文字は完全に一致する。
                               フォームやモジュールだけではなく、クラスにも対応。
							   ただし、Constの「,」で連続宣言には未対応
	Jul. 21, 2003 genta キーワードの大文字・小文字を同一視するようにした
	Aug  7, 2003 little YOSHI  ダブルクォーテーションで囲まれたテキストを無視するようにした
	                           関数名などをVBの名前付け規則より255文字に拡張
*/
void CDocOutline::MakeFuncList_VisualBasic( CFuncInfoArr* pcFuncInfoArr )
{
	const int	nMaxWordLeng = 255;	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	const wchar_t*	pLine;
	CLogicInt		nLineLen = CLogicInt(0);//: 2002/2/3 aroka 警告対策：初期化
	int			i;
	int			nCharChars;
	wchar_t		szWordPrev[256];	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	wchar_t		szWord[256];		// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	int			nWordIdx = 0;
	int			nMode;
	wchar_t		szFuncName[256];	// Aug 7, 2003 little YOSHI  VBの名前付け規則より255文字に拡張
	CLogicInt	nFuncLine;
	int			nFuncId;
	int			nParseCnt = 0;
	bool		bClass;			// クラスモジュールフラグ
	bool		bProcedure;		// プロシージャフラグ（プロシージャ内ではTrue）
	bool		bDQuote;		// ダブルクォーテーションフラグ（ダブルクォーテーションがきたらTrue）

	// 調べるファイルがクラスモジュールのときはType、Constの挙動が異なるのでフラグを立てる
	bClass	= false;
	int filelen = _tcslen(m_pcDocRef->m_cDocFile.GetFilePath());
	if ( 4 < filelen ) {
		if ( 0 == _tcsicmp((m_pcDocRef->m_cDocFile.GetFilePath() + filelen - 4), _FT(".cls")) ) {
			bClass	= true;
		}
	}

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	pLine = NULL;
	bProcedure	= false;
	CLogicInt		nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_pcDocRef->m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		if( NULL != pLine ){
			if( L'_' != pLine[nLineLen-1]){
				nParseCnt = 0;
			}
		}
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		nFuncId = 0;
		bDQuote	= false;
		for( i = 0; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if(	0 == nCharChars ){
				nCharChars = 1;
			}
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
					) )
				 || 2 == nCharChars
				){
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						auto_memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				} else if (1 == nCharChars && '"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Public") ) {
						// パブリック宣言を見つけた！
						nFuncId |= 0x10;
					}else
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Private") ) {
						// プライベート宣言を見つけた！
						nFuncId |= 0x20;
					}else
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Friend") ) {
						// フレンド宣言を見つけた！
						nFuncId |= 0x30;
					}else
					if ( 0 == nParseCnt && 0 == wcsicmp(szWord, L"Static") ) {
						// スタティック宣言を見つけた！
						nFuncId |= 0x100;
					}else
					if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Function" ) ){
						if ( 0 == wcsicmp( szWordPrev, L"End" ) ){
							// プロシージャフラグをクリア
							bProcedure	= false;
						}else
						if( 0 != wcsicmp( szWordPrev, L"Exit" ) ){
							if( 0 == wcsicmp( szWordPrev, L"Declare" ) ){
								nFuncId |= 0x200;	// DLL参照宣言
							}else{
								bProcedure	= true;	// プロシージャフラグをセット
							}
							nFuncId |= 0x01;		// 関数
							nParseCnt = 1;
							nFuncLine = nLineCount + CLogicInt(1);
						}
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Sub" ) ){
						if ( 0 == wcsicmp( szWordPrev, L"End" ) ){
							// プロシージャフラグをクリア
							bProcedure	= false;
						}else
						if( 0 != wcsicmp( szWordPrev, L"Exit" ) ){
							if( 0 == wcsicmp( szWordPrev, L"Declare" ) ){
								nFuncId |= 0x200;	// DLL参照宣言
							}else{
								bProcedure	= true;	// プロシージャフラグをセット
							}
							nFuncId |= 0x02;		// 関数
							nParseCnt = 1;
							nFuncLine = nLineCount + CLogicInt(1);
						}
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Get" )
					 && 0 == wcsicmp( szWordPrev, L"Property" )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId	|= 0x03;		// プロパティ取得
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Let" )
					 && 0 == wcsicmp( szWordPrev, L"Property" )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId |= 0x04;		// プロパティ設定
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Set" )
					 && 0 == wcsicmp( szWordPrev, L"Property" )
					){
						bProcedure	= true;	// プロシージャフラグをセット
						nFuncId |= 0x05;		// プロパティ参照
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Const" )
					 && 0 != wcsicmp( szWordPrev, L"#" )
					){
						if ( bClass || bProcedure || 0 == ((nFuncId >> 4) & 0x0f) ) {
							// クラスモジュールでは強制的にPrivate
							// プロシージャ内では強制的にPrivate
							// Publicの指定がないとき、デフォルトでPrivateになる
							nFuncId &= 0x0f2f;
							nFuncId	|= 0x20;
						}
						nFuncId	|= 0x06;		// 定数
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Enum" )
					){
						nFuncId	|= 0x207;		// 列挙型宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Type" )
					){
						if ( bClass ) {
							// クラスモジュールでは強制的にPrivate
							nFuncId &= 0x0f2f;
							nFuncId	|= 0x20;
						}
						nFuncId	|= 0x208;		// ユーザ定義型宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Event" )
					){
						nFuncId	|= 0x209;		// イベント宣言
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"Property" )
					 && 0 == wcsicmp( szWordPrev, L"End")
					){
						bProcedure	= false;	// プロシージャフラグをクリア
					}
					else if( 1 == nParseCnt ){
						wcscpy( szFuncName, szWord );
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  → レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						CLayoutPoint ptPosXY;
						m_pcDocRef->m_cLayoutMgr.LogicToLayout(	CLogicPoint(CLogicInt(0), nFuncLine - CLogicInt(1)), &ptPosXY );
						pcFuncInfoArr->AppendData( nFuncLine, ptPosXY.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
						nParseCnt = 0;
						nFuncId	= 0;	// Jul 10, 2003  little YOSHI  論理和を使用するため、必ず初期化
					}

					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}
			/* 記号列読み込み中 */
			else if( 2 == nMode ){
				// Jul 10, 2003  little YOSHI
				// 「#Const」と「Const」を区別するために、「#」も識別するように変更
				if( L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i] ||
					L'{' == pLine[i] ||
					L'}' == pLine[i] ||
					L'(' == pLine[i] ||
					L')' == pLine[i] ||
					L';' == pLine[i]	||
					L'\'' == pLine[i] ||
					L'/' == pLine[i]	||
					L'-' == pLine[i] ||
					L'#' == pLine[i]
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				} else if (1 == nCharChars && L'"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen && L'\'' == pLine[i] ){
					break;
				} else if (1 == nCharChars && L'"' == pLine[i]) {
					// Aug 7, 2003 little YOSHI  追加
					// テキストの中は無視します。
					nMode	= 3;
				}else{
					if( (1 == nCharChars && (
						L'_' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
						) )
					 || 2 == nCharChars
					){
						nWordIdx = 0;

						auto_memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						nWordIdx = 0;
						auto_memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			} else
			/* テキストが閉じるまで読み飛ばす */	// Aug 7, 2003 little YOSHI  追加
			if (nMode == 3) {
				// 連続するダブルクォーテーションは無視する
				if (1 == nCharChars && L'"' == pLine[i]) {
					// ダブルクォーテーションが現れたらフラグを反転する
					bDQuote	= !bDQuote;
				} else if (bDQuote) {
					// ダブルクォーテーションの次に
					// ダブルクォーテーション以外の文字が現れたらノーマルモードに移行
					--i;
					nMode	= 0;
					bDQuote	= false;
					continue;
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}
//	To Here June 23, 2001 N.Nakatani


// From Here 2001.12.03 hor
/*! ブックマークリスト作成（無理矢理！）

	@date 2002.01.19 aroka 空行をマーク対象にするフラグ bMarkUpBlankLineEnable を導入しました。
	@date 2005.10.11 ryoji "ａ@" の右２バイトが全角空白と判定される問題の対処
	@date 2005.11.03 genta 文字列長修正．右端のゴミを除去
*/
void CDocOutline::MakeFuncList_BookMark( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	CLogicInt		nLineCount;
	int		leftspace, pos_wo_space, k;
	wchar_t*	pszText;
	BOOL	bMarkUpBlankLineEnable = GetDllShareData().m_Common.m_sOutline.m_bMarkUpBlankLineEnable;	//! 空行をマーク対象にするフラグ 20020119 aroka
	int		nNewLineLen	= m_pcDocRef->m_cDocEditor.m_cNewLineCode.GetLen();
	CLogicInt	nLineLast	= m_pcDocRef->m_cDocLineMgr.GetLineCount();
	int		nCharChars;

	for( nLineCount = CLogicInt(0); nLineCount <  nLineLast; ++nLineCount ){
		if(!CBookmarkGetter(m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)).IsBookmarked())continue;
		pLine = m_pcDocRef->m_cDocLineMgr.GetLine(nLineCount)->GetDocLineStrWithEOL(&nLineLen);
		if( NULL == pLine ){
			break;
		}
		// Jan, 16, 2002 hor
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if( nLineLen<=nNewLineLen && nLineCount< nLineLast ){
			  continue;
			}
		}// LTrim
		for( leftspace = 0; leftspace < nLineLen; ++leftspace ){
			if( WCODE::isBlank(pLine[leftspace]) ){
				continue;
			}
			break;
		}
		
		if( bMarkUpBlankLineEnable ){// 20020119 aroka
			if(( leftspace >= nLineLen-nNewLineLen && nLineCount< nLineLast )||
				( leftspace >= nLineLen )) {
				continue;
			}
		}// RTrim
		// 2005.10.11 ryoji 右から遡るのではなく左から探すように修正（"ａ@" の右２バイトが全角空白と判定される問題の対処）
		k = pos_wo_space = leftspace;
		while( k < nLineLen ){
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, k );
			if( 1 == nCharChars ){
				if( !(pLine[k] == WCODE::CR ||
						pLine[k] == WCODE::LF ||
						pLine[k] == WCODE::SPACE ||
						pLine[k] == WCODE::TAB ||
						WCODE::isZenkakuSpace(pLine[k]) ||
						pLine[k] == L'\0') )
					pos_wo_space = k + nCharChars;
			}
			k += nCharChars;
		}
		//	Nov. 3, 2005 genta 文字列長計算式の修正
		{
			int nLen = pos_wo_space - leftspace;
			pszText = new wchar_t[nLen + 1];
			wmemcpy( pszText, &pLine[leftspace], nLen );
			pszText[nLen] = L'\0';
		}
		CLayoutPoint ptXY;
		//int nX,nY
		m_pcDocRef->m_cLayoutMgr.LogicToLayout(	CLogicPoint(CLogicInt(0), nLineCount), &ptXY );
		pcFuncInfoArr->AppendData( nLineCount+CLogicInt(1), ptXY.GetY2()+CLayoutInt(1) , pszText, 0 );
		delete [] pszText;
	}
	return;
}
// To Here 2001.12.03 hor
