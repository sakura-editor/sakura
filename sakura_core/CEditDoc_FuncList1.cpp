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
	Copyright (C) 2007, genta, じゅうじ

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include <string.h>
#include "global.h"

#include "CEditDoc.h"
#include "CFuncInfoArr.h"
#include "CDocLine.h"
#include "charcode.h"

//	Mar. 15, 2000 genta
//	From Here
/*!
	スペースの判定
*/
inline bool C_IsSpace( char c ){
	return ('\t' == c ||
			 ' ' == c ||
			  CR == c ||
			  LF == c
	);
}

/*!
	関数に用いることができる文字かどうかの判定
*/
inline bool C_IsWordChar( char c ){
	return ( '_' == c ||
			 ':' == c ||
			 '~' == c ||
			('a' <= c && c <= 'z' )||
			('A' <= c && c <= 'Z' )||
			('0' <= c && c <= '9' )
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
static bool C_IsOperator( char* szStr, int nLen	)
{
	if( nLen >= 8 && szStr[ nLen - 1 ] == 'r' ){
		if( nLen > 8 ?
				strcmp( szStr + nLen - 9, ":operator" ) == 0 :	// メンバー関数による定義
				strcmp( szStr, "operator" ) == 0	// friend関数による定義
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
static bool C_IsLineEsc(const char *s, int len)
{
	if ( len > 0 && s[len-1] == '\n' ) len--;
	if ( len > 0 && s[len-1] == '\r' ) len--;
	if ( len > 0 && s[len-1] == '\n' ) len--;

	if ( len > 0 && s[len-1] == '\\' ) {
		if ( len == 1 ) {
			return(true);
		} else if ( len == 2 ) {
			if ( CMemory::GetSizeOfChar( s, 2 , 0 ) == 1 )
				return(true);
		} else {				/* 残り３バイト以上	*/
			if ( CMemory::GetSizeOfChar( s, len , len-2 ) == 1 )
				return(true);
			if ( CMemory::GetSizeOfChar( s, len , len-3 ) == 2 )
				return(true);
		}
	}
	return(false);
}

/*!
	Cプリプロセッサの #if/ifdef/ifndef - #else - #endif状態管理クラス

	ネストレベルは32レベル=(sizeof(int) * 8)まで
	
	@date 2007.12.15 genta : m_enablebufの初期値が悪さをすることがあるので0に
*/

class CCppPreprocessMng {
public:
	CCppPreprocessMng(void) :
		// 2007.12.15 genta : m_bitpatternを0にしないと，
		// いきなり#elseが現れたときにパターンがおかしくなる
		m_stackptr( 0 ), m_bitpattern( 0 ), m_enablebuf( 0 ), m_maxnestlevel( 32 ), m_ismultiline( false )
	{}

	int ScanLine(const char*, int);

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
	@date 2007.12.13 じゅうじ : ifの直後にスペースがない場合の対応

*/
int CCppPreprocessMng::ScanLine( const char* str, int length )
{
	const char* lastptr = str + length;	//	処理文字列末尾
	const char* p;	//	処理中の位置

	//	skip whitespace
	for( p = str; C_IsSpace( *p ) && p < lastptr ; ++p )
		;
	if( lastptr <= p )
		return length;	//	空行のため処理不要

	if(m_ismultiline){ // 複数行のディレクティブは無視
		m_ismultiline = C_IsLineEsc(str, length); // 行末が \ で終わっていないか
		return length;
	}

	if( *p != '#' ){	//	プリプロセッサ以外の処理はメイン部に任せる
		if( m_enablebuf ){
			return length;	//	1ビットでも1となっていたら無視
		}
		return p - str;
	}

	++p; // #をスキップ
	
	//	skip whitespace
	for( ; C_IsSpace( *p ) && p < lastptr ; ++p )
		;

	//	ここからPreprocessor directive解析
	if( p + 2 + 2 < lastptr && strncmp( p, "if", 2 ) == 0 ){	//	sizeof( "if" ) == 2
		// if
		p += 2;
		
		int enable = 0;	//	0: 処理しない, 1: else以降が有効, 2: 最初が有効, 
		
		//	if 0は最初が無効部分とみなす．
		//	それ以外のif/ifdef/ifndefは最初が有効部分と見なす
		//	最初の条件によってこの時点ではp < lastptrなので判定省略
		// 2007/12/13 じゅうじ : #if(0)とスペースを空けない場合の対応
		if( C_IsSpace( *p ) || *p == '(' ){
			//	if 0 チェック
			//	skip whitespace
			//	2007.12.15 genta
			for( ; ( C_IsSpace( *p ) || *p == '(' ) && p < lastptr ; ++p )
				;
			if( *p == '0' ){
				enable = 1;
			}
			else {
				enable = 2;
			}
		}
		else if(
			( p + 3 < lastptr && strncmp( p, "def", 3 ) == 0 ) ||
			( p + 4 < lastptr && strncmp( p, "ndef", 4 ) == 0 )){
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
	else if( p + 4 < lastptr && strncmp( p, "else", 4 ) == 0 ){	//	sizeof( "else" ) == 4
		//	2007.12.14 genta : #ifが無く#elseが出たときのガード追加
		if( 0 < m_stackptr && m_stackptr < m_maxnestlevel ){
			m_enablebuf ^= m_bitpattern;
		}
	}
	else if( p + 5 < lastptr && strncmp( p, "endif", 5 ) == 0 ){	//	sizeof( "endif" ) == 5
		if( m_stackptr > 0 ){
			--m_stackptr;
			m_enablebuf &= ~m_bitpattern;
			m_bitpattern = ( 1 << ( m_stackptr - 1 ));
		}
	}
	else{
		m_ismultiline = C_IsLineEsc(str, length); // 行末が \ で終わっていないか
	}

	return length;	//	基本的にプリプロセッサ指令は無視
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
void CEditDoc::MakeFuncList_C( CFuncInfoArr* pcFuncInfoArr ,bool bVisibleMemberFunc )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;

	// 2002/10/27 frozen　ここから
	// nNestLevelを nNestLevel_global を nNestLevel_func に分割した。
	int			nNestLevel_global = 0;	// nNestLevel_global 関数外の {}のレベル  
	int			nNestLevel_func   = 0;	//	nNestLevel_func 関数の定義、および関数内の	{}のレベル
//	int			nNestLevel2;			//	nNestLevel2	()に対する位置 // 2002/10/27 frozen nNastLevel_fparamとnMode2のM2_FUNC_NAME_ENDで代用
	int			nNestLevel_fparam = 0;	// ()のレベル
	int			nNestPoint_class = 0;	// 外側から何番目の{がクラスの定義を囲む{か？ (一番外側なら1、0なら無し。bVisibleMemberFuncがfalseの時のみ有効。trueでは常に0)
	// 2002/10/27 frozen　ここまで

	int			nCharChars;			//	多バイト文字を読み飛ばすためのもの
	char		szWordPrev[256];	//	1つ前のword
	char		szWord[256];		//	現在解読中のwordを入れるところ
	int			nWordIdx = 0;
	int			nMaxWordLeng = 100;	//	許容されるwordの最大長さ
	int			nMode;				//	現在のstate

	// 2002/10/27 frozen　ここから
	//! 状態2
	enum MODE2
	{
		M2_NORMAL			= 0x00,	//!< 通常
		M2_ATTRIBUTE		= 0x02,	//!< C++/CLI attribute : 2007.05.26 genta

		M2_NAMESPACE_SAVE	= 0x11,	//!< ネームスペース名調査中
			// 「通常」状態で単語 "class" "struct" "union" "enum" "namespace", "__interface" を読み込むと、この状態になり、';' '{' ',' '>' '='を読み込むと「通常」になる。
			//	2007.05.26 genta キーワードに__interface追加
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
//	char		szFuncName[256];	//	関数名

	const int	nNamespaceNestMax	= 32;			//!< ネスト可能なネームスペース、クラス等の最大数
	int			nNamespaceLen[nNamespaceNestMax+1];	//!< ネームスペース全体の長さ
	const int	nNamespaceLenMax 	= 512;			//!< 最大のネームスペース全体の長さ
	char		szNamespace[nNamespaceLenMax];		//!< 現在のネームスペース(終端が\0になっているとは限らないので注意)
	const int 	nItemNameLenMax	 	= 256;
	char		szItemName[nItemNameLenMax];		//!< すぐ前の 関数名 or クラス名 or 構造体名 or 共用体名 or 列挙体名 or ネームスペース名
	// 例えば下のコードの←の部分での
	// szNamespaceは"Namespace\ClassName\"
	// nMamespaceLenは{10,20}
	// nNestLevel_globalは2となる。
	//
	//　namespace Namespace{
	//　class ClassName{
	//　←
	//　}}

	int			nItemLine;			//!< すぐ前の 関数 or クラス or 構造体 or 共用体 or 列挙体 or ネームスペースのある行
	int			nItemFuncId;

//	int			nFuncLine;
//	int			nFuncId;
	
//	int			nFuncNum;			// 使っていないようなので削除
	// 2002/10/27 frozen　ここまで

	//	Mar. 4, 2001 genta
//	bool		bCppInitSkip;		//	C++のメンバー変数、親クラスの初期化子をSKIP // 2002/10/27 frozen nMode2の機能で代用
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	szNamespace[0] = '\0';	// 2002/10/27 frozen
	nNamespaceLen[0] = 0;	// 2002/10/27 frozen
	nMode = 0;
	
	//	Aug. 10, 2004 genta プリプロセス処理クラス
	CCppPreprocessMng cCppPMng;
	
//	FuncNum = 0;
//	bCppInitSkip = false;
//	for( nLineCount = 0; nLineCount <  m_cLayoutMgr.GetLineCount(); ++nLineCount ){
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
//		pLine = m_cLayoutMgr.GetLineStr( nLineCount, &nLineLen );
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );

		//	From Here Aug. 10, 2004 genta
		//	プリプロセス処理
		//	コメント中でなければプリプロセッサ指令を先に判定させる
		if( 8 != nMode && 10 != nMode ){	/* chg 2005/12/6 じゅうじ 次の行が空白でもよい	*/
			i = cCppPMng.ScanLine( pLine, nLineLen );
		}
		else {
			i = 0;
		}
		//	C/C++としての処理が不要なケースでは i == nLineLenとなっているので
		//	以下の解析処理はSKIPされる．
		//	To Here Aug. 10, 2004 genta
		
		for( ; i < nLineLen; ++i ){
			/* 1バイト文字だけを処理する */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CMemory::GetSizeOfChar( pLine, nLineLen, i );
			if( 1 < nCharChars ){
				i += (nCharChars - 1);
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
			}else
			/* ラインコメント読み込み中 */
			// 2003/06/24 zenryaku
			if( 10 == nMode)
			{
				if(!C_IsLineEsc(pLine, nLineLen)){
					nMode = 0;
				}
				i = nLineLen;
				continue;
			}else
/* add start 2005/12/6 じゅうじ	*/
			/* エスケープシーケンスは常に取り除く */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* シングルクォーテーション文字列読み込み中 */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* ダブルクォーテーション文字列読み込み中 */
			if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
/* add end 2005/12/6 じゅうじ	*/
			/* 単語読み込み中 */
			if( 1 == nMode ){
				if( C_IsWordChar( pLine[i] ) ){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						if( pLine[i] == ':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									strcpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								strncat( szItemName, szWord, nItemNameLenMax - strlen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = '\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
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
									pLine[ i + 1 ] == '=' )
									oplen = 2;
							}
							break;
						case '-':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == '-' ||
									pLine[ i + 1 ] == '=' )
									oplen = 2;
								else if( pLine[ i + 1 ] == '>' ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == '*' )
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
								if( pLine[ i + 1 ] == '=' )
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
										if( pLine[ i + 2 ] == '=' )
											oplen = 3;
									}
								}
								else if( pLine[ i + 1 ] == '=' )
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
						szWord[nWordIdx + 1] = '\0';
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
						strcpy( szItemName, szWord );
					else if( nMode2 == M2_TEMPLATE_SAVE)
					{
						strncat( szItemName, szWord, nItemNameLenMax - strlen(szItemName) );
						szItemName[ nItemNameLenMax - 1 ] = '\0';
					}
					else if( nNestLevel_func == 0 && nMode2 == M2_NORMAL )
					{
						nItemFuncId = 0;
						if( strcmp(szWord,"class")==0 )
							nItemFuncId = 3;
						if( strcmp(szWord,"struct")==0 )
							nItemFuncId = 4;
						else if( strcmp(szWord,"namespace")==0 )
							nItemFuncId = 7;
						else if( strcmp(szWord,"enum")==0 )
							nItemFuncId = 5;
						else if( strcmp(szWord,"union")==0 )
							nItemFuncId = 6;
						else if( strcmp(szWord,"__interface")==0 ) // 2007.05.26 genta "__interface" をクラスに類する扱いにする
							nItemFuncId = 8;
						if( nItemFuncId != 0 )
						{
							nMode2 = M2_NAMESPACE_SAVE;
							nItemLine = nLineCount + 1;
							strcpy(szItemName,"無名");
						}
					}
					else if( nMode2 == M2_FUNC_NAME_END )
					{
						nMode2 = M2_KR_FUNC;
					}
					// 2002/10/27 frozen　ここまで

					//	To Here Mar. 31, 2001 genta
					// 2004/03/12 zenryaku キーワードに _ と PARAMS を使わせない (GNUのコードが見にくくなるから)
					if( !( strcmp("PARAMS",szWord) == 0 || strcmp("_",szWord) == 0 ) )
						strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
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
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
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
						szWord[nWordIdx + 1] = '\0';
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
							(nNamespaceLen[nNestLevel_global] +  (nItemNameLen = strlen(szItemName)) + 10 + 1) < nNamespaceLenMax)
					// ３番目の(&&の後の)条件
					// バッファが足りない場合は項目の追加を行わない。
					// +10は追加する文字列の最大長(追加する文字列は"::定義位置"が最長)
					// +1は終端NUL文字
					{
						strcpy( &szNamespace[nNamespaceLen[nNestLevel_global]] , szItemName);
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
								strcpy(&szNamespace[nNamespaceLen[nNestLevel_global]],"::定義位置");
							else
							{
								szNamespace[nNamespaceLen[nNestLevel_global]] = '\0';
								szNamespace[nNamespaceLen[nNestLevel_global]+1] = ':';
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
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nItemLine - 1,
							&nPosX,
							&nPosY
						);
						pcFuncInfoArr->AppendData( nItemLine, nPosY + 1 , szNamespace, nItemFuncId);
						//	Jan. 30, 2005 genta M2_KR_FUNC 追加
						if( nMode2 != M2_FUNC_NAME_END && nMode2 != M2_KR_FUNC )
						{
							szNamespace[nNamespaceLen[nNestLevel_global]] = ':';
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
//						strcpy( szFuncName, szWordPrev );
//						nFuncLine = nLineCount + 1;
//						nNestLevel2 = 1;
//					}
//					nMode = 0;
					//	2007.05.26 genta C++/CLI nMode2 == M2_NAMESPACE_ENDの場合を対象外に
					//	NAMESPACE_END(class クラス名 :の後ろ)においては()を関数とみなさない．
					//	TEMPLATE<sizeof(int)> のようなケースでsizeofを関数と誤認する．
					if( nNestLevel_func == 0 && (nMode2 == M2_NORMAL || nMode2 == M2_NAMESPACE_SAVE ) )
					{
						if( strcmp(szWordPrev, "__declspec") == 0 ) {continue;}
						if(nNestLevel_fparam==0)
						{
							strcpy( szItemName, szWordPrev);
							nItemLine = nLineCount + 1;
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
						//	2007.05.26 genta C++/CLI Attribute内部ではnMode2の変更は行わない
						if( nNestLevel_fparam == 0 && nMode2 != M2_ATTRIBUTE )
						{
							nMode2 = M2_FUNC_NAME_END;
							nItemFuncId = 2;
						}
					}
					//  2002/10/27 frozen ここまで
					continue;
				}else
				// From Here 2007.05.26 genta C++/CLI Attributeの取り扱い
				if( '[' == pLine[i] ){
					if( nNestLevel_func == 0 && nNestLevel_fparam == 0 && nMode2 == M2_NORMAL ) {
						nMode2 = M2_ATTRIBUTE;
						continue;
					}
				} else
				if( ']' == pLine[i] ){
					//	Attribute内部でも[]を配列として使うかもしれないので，
					//	括弧のレベルは元に戻っている必要有り
					if( nNestLevel_fparam == 0 && nMode2 == M2_ATTRIBUTE ) {
						nMode2 = M2_NORMAL;
						continue;
					}
				} else
				// To Here 2007.05.26 genta C++/CLI Attributeの取り扱い
				if( ';' == pLine[i] ){
					//  2002/10/27 frozen ここから
//					if( 2 == nNestLevel2 ){
//						//	閉じ括弧')'の後の';' すなわち関数宣言
//						if( 0 != strcmp( "sizeof", szFuncName ) ){
//							nFuncId = 1;
//							++nFuncNum;
//							/*
//							  カーソル位置変換
//							  物理位置(行頭からのバイト数、折り返し無し行位置)
//							  →
//							  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
//							*/
//							int		nPosX;
//							int		nPosY;
//							m_cLayoutMgr.CaretPos_Phys2Log(
//								0,
//								nFuncLine - 1,
//								&nPosX,
//								&nPosY
//							);
//							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szFuncName, nFuncId);
////						pcFuncInfoArr->AppendData( nFuncLine, szFuncName, nFuncId );
//						}
//					}
//					nNestLevel2 = 0;
					if( nMode2 == M2_KR_FUNC )
					{
						//	Jan. 30, 2005 genta 関数後の const, throwの後ろの
						//	';'はK&R形式宣言の終わりでなく関数宣言の終わり
						if( strcmp( szWordPrev, "const" ) == 0 ||
							strcmp( szWordPrev, "throw" ) == 0 ){
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
						(nNamespaceLen[nNestLevel_global] + strlen(szItemName)) < nNamespaceLenMax &&
						nNestPoint_class == 0)
					// ３番目の(&&の後の)条件
					// バッファが足りない場合は項目の追加を行わない。
					{
						strcpy( &szNamespace[nNamespaceLen[ nNestLevel_global]] , szItemName);

						nItemFuncId = 1;
						/*
						  カーソル位置変換
						  物理位置(行頭からのバイト数、折り返し無し行位置)
						  →
						  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nItemLine - 1,
							&nPosX,
							&nPosY
						);
						pcFuncInfoArr->AppendData( nItemLine, nPosY + 1, szNamespace, nItemFuncId);
					}
					nMode2 = M2_NORMAL;
					//  2002/10/27 frozen ここまで
					nMode = 0;
					continue;
				}else if( nNestLevel_fparam == 0 && nMode2 != M2_ATTRIBUTE ){
					// 2007.05.26 genta C++/CLI Attribute内部では関数名処理は一切行わない
					if( C_IsWordChar( pLine[i] ) ){
						//  2002/10/27 frozen ここから削除
//						if( 2 == nNestLevel2 ){
//							//	閉じ括弧が無いけどとりあえず登録しちゃう
//							if( 0 != strcmp( "sizeof", szFuncName ) ){
//								nFuncId = 2;
//								++nFuncNum;
//								/*
//								  カーソル位置変換
//								  物理位置(行頭からのバイト数、折り返し無し行位置)
//								  →
//								  レイアウト位置(行頭からの表示桁位置、折り返しあり行位置)
//								*/
//								int		nPosX;
//								int		nPosY;
//								m_cLayoutMgr.CaretPos_Phys2Log(
//									0,
//									nFuncLine - 1,
//									&nPosX,
//									&nPosY
//								);
//								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
//							}
//							nNestLevel2 = 0;
//							//	Mar 4, 2001 genta	初期化子だったときはそれ以降の登録を制限する
//							if( pLine[i] == ':' )
//								bCppInitSkip = true;
//						}
						//  2002/10/27 frozen ここまで削除

						//	//	Mar. 15, 2000 genta
						//	From Here
						//	直前のwordの最後が::か，あるいは直後のwordの先頭が::なら
						//	クラス限定子と考えて両者を接続する．

						{
							int pos = strlen( szWordPrev ) - 2;
							if( //	前の文字列の末尾チェック
								( pos > 0 &&	szWordPrev[pos] == ':' &&
								szWordPrev[pos + 1] == ':' ) ||
								//	次の文字列の先頭チェック
								( i < nLineLen - 1 && pLine[i] == ':' &&
									pLine[i+1] == ':' )
							){
								//	前の文字列に続ける
								strcpy( szWord, szWordPrev );
								nWordIdx = strlen( szWord );
							}
							//	From Here Apr. 1, 2001 genta
							//	operator new/delete 演算子の対応
							else if( C_IsOperator( szWordPrev, pos + 2 ) ){
								//	スペースを入れて、前の文字列に続ける
								szWordPrev[pos + 2] = ' ';
								szWordPrev[pos + 3] = '\0';
								strcpy( szWord, szWordPrev );
								nWordIdx = strlen( szWord );
							}
							//	To Here Apr. 1, 2001 genta
							else{
								nWordIdx = 0;
							}
						}
						//	strcpy( szWordPrev, szWord );	不要？
						//	To Here
						
						if( pLine[i] == ':')
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									strcpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								strncat( szItemName, szWord, nItemNameLenMax - strlen(szItemName) );
								szItemName[ nItemNameLenMax - 1 ] = '\0';
								nMode2 = M2_NAMESPACE_END;
							}
						}

						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
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
						if( pLine[i] == '=' && nNestLevel_func == 0 && nNestLevel_fparam==0
							&& nMode2 == M2_NORMAL && ! C_IsOperator(szWordPrev,strlen(szWordPrev)) ){
							nMode2 = M2_AFTER_EQUAL;
						}
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						nMode = 2;

						// 2002/10/27 frozen ここから
						if( nMode2 == M2_NAMESPACE_SAVE )
						{
							if( pLine[i] == '>' || pLine[i] == ',' || pLine[i] == '=')
								// '<' の前に '>' , ',' , '=' があったので、おそらく
								// 前にあった"class"はテンプレートパラメータの型を表していたと考えられる。
								// よって、クラス名の調査は終了。
								// '>' はテンプレートパラメータの終了
								// ',' はテンプレートパラメータの区切り
								// '=' はデフォルトテンプレートパラメータの指定
								nMode2 = M2_NORMAL; 
							else if( pLine[i] == '<' )
								nMode2 = M2_TEMPLATE_SAVE;
						}

						if( nMode2 == M2_TEMPLATE_SAVE)
						{
							int nItemNameLen = strlen(szItemName);
							if(nItemNameLen + 1 < nItemNameLenMax )
							{
								szItemName[nItemNameLen] = pLine[i];
								szItemName[nItemNameLen + 1 ] = '\0';
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
