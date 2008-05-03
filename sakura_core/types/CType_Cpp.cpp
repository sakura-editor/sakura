#include "stdafx.h"
#include "CType.h"
#include "doc/CDocOutline.h"
#include "CFuncInfoArr.h"

/* C/C++ */
// Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読めるようにする
// Jan. 24, 2004 genta 関連づけ上好ましくないのでdsw,dsp,dep,makははずす
//	2003.06.23 Moca ファイル内からの入力補完機能
void CType_Cpp::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	_tcscpy( pType->m_szTypeName, _T("C/C++") );
	_tcscpy( pType->m_szTypeExts, _T("c,cpp,cxx,cc,cp,c++,h,hpp,hxx,hh,hp,h++,rc,hm") );

	//設定
	pType->m_cLineComment.CopyTo( 0, L"//", -1 );							/* 行コメントデリミタ */
	pType->m_cBlockComment.SetBlockCommentRule( 0, L"/*", L"*/" );			/* ブロックコメントデリミタ */
	pType->m_cBlockComment.SetBlockCommentRule( 1, L"#if 0", L"#endif" );	/* ブロックコメントデリミタ2 */	//Jul. 11, 2001 JEPRO
	pType->m_nKeyWordSetIdx[0] = 0;											/* キーワードセット */
	pType->m_nDefaultOutline = OUTLINE_CPP;									/* アウトライン解析方法 */
	pType->m_nSmartIndent = SMARTINDENT_CPP;								/* スマートインデント種別 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;					//半角数値を色分け表示	//Mar. 10, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;			//	Sep. 21, 2002 genta 対括弧の強調をデフォルトONに
	pType->m_bUseHokanByFile = TRUE;										/*! 入力補完 開いているファイル内から候補を探す */
}


//	Mar. 15, 2000 genta
//	From Here


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
}




const wchar_t* g_ppszKeywordsCPP[] = {
	L"#define",
	L"#elif",
	L"#else",
	L"#endif",
	L"#error",
	L"#if",
	L"#ifdef",
	L"#ifndef",
	L"#include",
	L"#line",
	L"#pragma",
	L"#undef",
	L"__FILE__",
	L"__declspec",
	L"asm",
	L"auto",
	L"bool",
	L"break",
	L"case",
	L"catch",
	L"char",
	L"class",
	L"const",
	L"const_cast",
	L"continue",
	L"default",
	L"define",
	L"defined",
	L"delete",
	L"do",
	L"double",
	L"dynamic_cast",
	L"elif",
	L"else",
	L"endif",
	L"enum",
	L"error",
	L"explicit",
	L"export",
	L"extern",
	L"false",
	L"float",
	L"for",
	L"friend",
	L"goto",
	L"if",
	L"ifdef",
	L"ifndef",
	L"include",
	L"inline",
	L"int",
	L"line",
	L"long",
	L"mutable",
	L"namespace",
	L"new",
	L"operator",
	L"pragma",
	L"private",
	L"protected",
	L"public",
	L"register",
	L"reinterpret_cast",
	L"return",
	L"short",
	L"signed",
	L"sizeof",
	L"static",
	L"static_cast",
	L"struct",
	L"switch",
	L"template",
	L"this",
	L"throw",
	L"true",
	L"try",
	L"typedef",
	L"typeid",
	L"typename",
	L"undef",
	L"union",
	L"unsigned",
	L"using",
	L"virtual",
	L"void",
	L"volatile",
	L"wchar_t",
	L"while"
};
int g_nKeywordsCPP = _countof(g_ppszKeywordsCPP);
