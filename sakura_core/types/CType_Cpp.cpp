/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CType.h"
#include "doc/CDocOutline.h"
#include "doc/CEditDoc.h"
#include "outline/CFuncInfoArr.h"
#include "cmd/COpeBlk.h"
#include "cmd/CViewCommander_inline.h"
#include "view/CEditView.h"
#include "view/colors/EColorIndexType.h"
#include "CSelectLang.h"

/*!
 * @brief C/C++のファイル名による判定
 *
 * @param [in] pszFileName ファイル名(nullptr指定不可、NUL終端すること)
 * @return アウトライン種別
 *
 * @detail
 * ファイル名(拡張子)に応じたアウトライン種別を取得する。
 * 拡張子の一致判定は大文字小文字を区別しない。
 * 現状、拡張子に対応するアウトライン種別はベタ書きで判定している。
 * 一致する拡張子がない場合、OUTLINE_C_CPPになる。
 */
EOutlineType GetCLangOutlineType( const WCHAR* pszFileName )
{
	auto eOutlineType = OUTLINE_C_CPP;
	if( CheckEXT( pszFileName, LTEXT("c") ) )
	{
		eOutlineType = OUTLINE_C;
	}
	else if( CheckEXT( pszFileName, LTEXT("cpp") )
		|| CheckEXT( pszFileName, LTEXT("c++") )
		|| CheckEXT( pszFileName, LTEXT("cxx") )
		|| CheckEXT( pszFileName, LTEXT("hpp") )
		|| CheckEXT( pszFileName, LTEXT("h++") )
		|| CheckEXT( pszFileName, LTEXT("hxx") ) )
	{
		eOutlineType = OUTLINE_CPP;
	}
	return eOutlineType;
}

//!CPPキーワードで始まっていれば true
inline bool IsHeadCppKeyword(const WCHAR* pData)
{
	if( wcsncmp_literal(pData, LTEXT("case")      )==0 )return true;
	if( wcsncmp_literal(pData, LTEXT("default:")  )==0 )return true;
	if( wcsncmp_literal(pData, LTEXT("public:")   )==0 )return true;
	if( wcsncmp_literal(pData, LTEXT("private:")  )==0 )return true;
	if( wcsncmp_literal(pData, LTEXT("protected:"))==0 )return true;
	return false;
}

/* C/C++ */
// Oct. 31, 2000 JEPRO VC++の生成するテキストファイルも読めるようにする
// Jan. 24, 2004 genta 関連づけ上好ましくないのでdsw,dsp,dep,makははずす
//	2003.06.23 Moca ファイル内からの入力補完機能
void CType_Cpp::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	wcscpy( pType->m_szTypeName, LTEXT("C/C++") );
	wcscpy( pType->m_szTypeExts, LTEXT("c,cpp,cxx,cc,cp,c++,h,hpp,hxx,hh,hp,h++,rc,hm") );

	//設定
	pType->m_cLineComment.CopyTo( 0, LTEXT("//"), -1 );							/* 行コメントデリミタ */
	pType->m_cBlockComments[0].SetBlockCommentRule( LTEXT("/*"), LTEXT("*/") );			/* ブロックコメントデリミタ */
	pType->m_cBlockComments[1].SetBlockCommentRule( LTEXT("#if 0"), LTEXT("#endif") );	/* ブロックコメントデリミタ2 */	//Jul. 11, 2001 JEPRO
	pType->m_nKeyWordSetIdx[0] = 0;											/* キーワードセット */
	pType->m_eDefaultOutline = OUTLINE_C_CPP;									/* アウトライン解析方法 */
	pType->m_eSmartIndent = SMARTINDENT_CPP;								/* スマートインデント種別 */
	pType->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp = true;					//半角数値を色分け表示	//Mar. 10, 2001 JEPRO
	pType->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp = true;			//	Sep. 21, 2002 genta 対括弧の強調をデフォルトONに
	pType->m_bUseHokanByFile = true;										/*! 入力補完 開いているファイル内から候補を探す */
	pType->m_bStringLineOnly = true; // 文字列は行内のみ
}

//	Mar. 15, 2000 genta
//	From Here

/*!
	関数に用いることができる文字かどうかの判定
*/
inline bool C_IsWordChar( WCHAR c )
{
	return (
		LCHAR('_') == c ||
		LCHAR(':') == c ||
		LCHAR('~') == c ||
		(LCHAR('a') <= c && c <= LCHAR('z') )||
		(LCHAR('A') <= c && c <= LCHAR('Z') )||
		(LCHAR('0') <= c && c <= LCHAR('9') )||
		(0xa1 <= c && !iswcntrl(c) && !iswspace(c))
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
static bool C_IsOperator( WCHAR* szStr, int nLen	)
{
	if( nLen >= 8 && szStr[ nLen - 1 ] == LCHAR('r') ){
		if( nLen > 8 ?
				wcscmp( szStr + nLen - 9, LTEXT(":operator") ) == 0 :	// メンバー関数による定義
				wcscmp( szStr, LTEXT("operator") ) == 0	// friend関数による定義
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
static bool C_IsLineEsc(const WCHAR *s, int len)
{
	if ( len > 0 && WCODE::IsLineDelimiter(s[len-1], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ) len--;
	if ( len > 0 && s[len-1] == LCHAR('\r') ) len--;

	if ( len > 0 && s[len-1] == LCHAR('\\') ) {
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

static bool CPP_IsFunctionAfterKeyword( const WCHAR* s )
{
	return
		wcscmp( s, LTEXT("const") ) == 0 ||
		wcscmp( s, LTEXT("volatile") ) == 0 ||
		wcscmp( s, LTEXT("throw") ) == 0 ||
		wcscmp( s, LTEXT("default") ) == 0 ||
		wcscmp( s, LTEXT("delete") ) == 0 ||
		wcscmp( s, LTEXT("override") ) == 0 ||
		wcscmp( s, LTEXT("final") ) == 0 ||
		wcscmp( s, LTEXT("noexcept") ) == 0
		;
}

/*!
	Cプリプロセッサの #if/ifdef/ifndef - #else - #endif状態管理クラス

	ネストレベルは32レベル=(sizeof(int) * 8)まで
	
	@date 2007.12.15 genta : m_enablebufの初期値が悪さをすることがあるので0に
*/

class CCppPreprocessMng {
public:
	CCppPreprocessMng(void) = default;

	CLogicInt ScanLine(const WCHAR*, CLogicInt);

private:
	bool m_ismultiline = false; //!< 複数行のディレクティブ
	int m_maxnestlevel = 32;	//!< ネストレベルの最大値

	int m_stackptr = 0;	//!< ネストレベル
	/*!
		ネストレベルに対応するビットパターン
		
		m_stackptr = n の時，下から(n-1)bit目に1が入っている
	*/
	// 2007.12.15 genta : m_bitpatternを0にしないと，
	// いきなり#elseが現れたときにパターンがおかしくなる
	unsigned int m_bitpattern = 0;
	unsigned int m_enablebuf = 0;	//!< 処理の有無を保存するバッファ
	unsigned int m_activated = 0;   //!< 処理しない有効化したコードが有ったかどうかを記録するバッファ
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
CLogicInt CCppPreprocessMng::ScanLine( const WCHAR* str, CLogicInt _length )
{
	int length=_length;

	const WCHAR* lastptr = str + length;	//	処理文字列末尾
	const WCHAR* p;	//	処理中の位置
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;

	//	skip whitespace
	for( p = str; C_IsSpace( *p, bExtEol ) && p < lastptr ; ++p )
		;
	if( lastptr <= p )
		return CLogicInt(length);	//	空行のため処理不要

	if(m_ismultiline){ // 複数行のディレクティブは無視
		m_ismultiline = C_IsLineEsc(str, length); // 行末が \ で終わっていないか
		return CLogicInt(length);
	}

	if( *p != LCHAR('#') ){	//	プリプロセッサ以外の処理はメイン部に任せる
		if( m_enablebuf ){
			return CLogicInt(length);	//	1ビットでも1となっていたら無視
		}
		return CLogicInt(p - str);
	}

	++p; // #をスキップ
	
	//	skip whitespace
	for( ; C_IsSpace( *p, bExtEol ) && p < lastptr ; ++p )
		;

	enum CppDirectiveType {
		Directive_None,
		Directive_If,
		Directive_Ifdef,
		Directive_Ifndef,
		Directive_Else,
		Directive_Elif,
		Directive_Endif,
	} directive = Directive_None;
	     if (p + 5 < lastptr && wcsncmp_literal(p, LTEXT("ifdef") ) == 0) { directive = Directive_Ifdef;  p += 5; }
	else if (p + 6 < lastptr && wcsncmp_literal(p, LTEXT("ifndef")) == 0) { directive = Directive_Ifndef; p += 6; }
	else if (p + 2 < lastptr && wcsncmp_literal(p, LTEXT("if")    ) == 0) { directive = Directive_If;     p += 2; }
	else if (p + 4 < lastptr && wcsncmp_literal(p, LTEXT("else")  ) == 0) { directive = Directive_Else;   p += 4; }
	else if (p + 4 < lastptr && wcsncmp_literal(p, LTEXT("elif")  ) == 0) { directive = Directive_Elif;   p += 4; }
	else if (p + 5 < lastptr && wcsncmp_literal(p, LTEXT("endif") ) == 0) { directive = Directive_Endif;  p += 5; }


	//	ここからPreprocessor directive解析
	switch (directive) {
	case Directive_If:
	case Directive_Ifdef:
	case Directive_Ifndef:
	case Directive_Elif:
		//	if 0は最初が無効部分とみなす．
		//	それ以外のif/ifdef/ifndefは最初が有効部分と見なす
		//	最初の条件によってこの時点ではp < lastptrなので判定省略
		// 2007/12/13 じゅうじ : #if(0)とスペースを空けない場合の対応
		if( C_IsSpace( *p, bExtEol ) || *p == LCHAR('(') ){
			//	if 0 チェック
			//	skip whitespace
			//	2007.12.15 genta
			for( ; ( C_IsSpace( *p, bExtEol ) || *p == LCHAR('(') ) && p < lastptr ; ++p )
				;
			bool bZero = (*p == LCHAR('0'));
			//	保存領域の確保とビットパターンの設定
			if (directive == Directive_If || directive == Directive_Ifdef || directive == Directive_Ifndef) {
				m_bitpattern = 1 << m_stackptr;
				++m_stackptr;
				if (bZero) {
					m_enablebuf |= m_bitpattern;
				}
				else if (m_stackptr == 1 || m_activated & (1 << (m_stackptr - 2))) {
					m_activated |= m_bitpattern;
				}
			}
			else if (0 < m_stackptr && m_stackptr < m_maxnestlevel) {
				if (bZero || (m_activated & m_bitpattern)) {
					m_enablebuf |= m_bitpattern;
				}
				else if (!(m_activated & m_bitpattern)) {
					m_enablebuf &= ~m_bitpattern;
					m_activated |= m_bitpattern;
				}
			}
		}
		break;
	case Directive_Else:
		//	2007.12.14 genta : #ifが無く#elseが出たときのガード追加
		if( 0 < m_stackptr && m_stackptr < m_maxnestlevel ){
			if (m_activated & m_bitpattern) {
				m_enablebuf |= m_bitpattern;
			}
			else {
				if (!(m_activated & m_bitpattern)) {
					m_enablebuf &= ~m_bitpattern;
				}
				if (m_stackptr == 1 || m_activated & (1 << (m_stackptr - 2))) {
					m_activated |= m_bitpattern;
				}
			}
		}
		break;
	case Directive_Endif:
		if( m_stackptr > 0 ){
			--m_stackptr;
			m_enablebuf &= ~m_bitpattern;
			m_activated &= ~m_bitpattern;
			m_bitpattern = ( 1 << ( m_stackptr - 1 ));
		}
		break;
	case Directive_None:
	default:
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
void CDocOutline::MakeFuncList_C( CFuncInfoArr* pcFuncInfoArr ,EOutlineType& nOutlineType,
	const WCHAR* pszFileName, bool bVisibleMemberFunc
)
{
#ifdef _DEBUG
// #define TRACE_OUTLINE
#endif
	const WCHAR*	pLine;
	CLogicInt	nLineLen;
	CLogicInt	i;
	if( nOutlineType == OUTLINE_C_CPP ){
		// 2015.11.14 C/C++のファイル名による判定
		nOutlineType = GetCLangOutlineType( pszFileName );
	}

	// 2002/10/27 frozen　ここから
	// nNestLevelを nNestLevel_global を nNestLevel_func に分割した。
	int			nNestLevel_global = 0;	// nNestLevel_global 関数外の {}のレベル  
	int			nNestLevel_func   = 0;	//	nNestLevel_func 関数の定義、および関数内の	{}のレベル
//	int			nNestLevel2;			//	nNestLevel2	()に対する位置 // 2002/10/27 frozen nNastLevel_fparamとnMode2のM2_FUNC_NAME_ENDで代用
	int			nNestLevel_fparam = 0;	// ()のレベル
	int			nNestPoint_class = 0;	// 外側から何番目の{がクラスの定義を囲む{か？ (一番外側なら1、0なら無し。bVisibleMemberFuncがfalseの時のみ有効。trueでは常に0)
	// 2002/10/27 frozen　ここまで

	bool bInInitList = false;	// 2010.07.08 ryoji 関数名調査の際、現在位置が初期化リスト（':'以後）に到達したかどうかを示す
	int			nNestLevel_template = 0; // template<> func<vector<int>> などの<>の数

	WCHAR		szWordPrev[256];	//	1つ前のword
	WCHAR		szWord[256];		//	現在解読中のwordを入れるところ
	int			nWordIdx = 0;
	int			nMaxWordLeng = 100;	//	許容されるwordの最大長さ
	int			nMode;				//	現在のstate
	/*
		nMode
		  0 : 初期値
		  1 : 単語読み込み中
		      szWordに単語を格納
		  2 : 記号列
		  8 : ブロックコメント中
		 10 : // コメント中
		 20 : 文字定数 ''
		 21 : 文字列 ""
		999 : 長い単語無視
	*/

	// 2002/10/27 frozen　ここから
	//! 状態2
	enum MODE2
	{
		M2_NORMAL			= 0x00,	//!< 通常
		M2_ATTRIBUTE		= 0x02,	//!< C++/CLI attribute : 2007.05.26 genta
		M2_TEMPLATE			= 0x03, //!< "template<" でM2_TEMPLATEになり '>' でM2_NORMALに戻る
		M2_NAMESPACE_SAVE	= 0x11,	//!< ネームスペース名調査中
			// 「通常」状態で単語 "class" "struct" "union" "enum" "namespace", "__interface" を読み込むと、この状態になり、';' '{' ',' '>' '='を読み込むと「通常」になる。
			//	2007.05.26 genta キーワードに__interface追加
			//
			// ':' を読み込むと「ネームスペース名調査完了」へ移行すると同時に
			// szWordをszItemNameに保存し、あとで ':' 又は '{' の直前の単語が調べられるようにしている。
			// これは "__declspec( dllexport )"のように"class"とクラス名の間にキーワードが書いてある場合でもクラス名を取得できるようにするため。
			//
			// '<' を読み込むと「テンプレートクラス名調査中」に移行する。
		M2_TEMPLATE_SAVE	= 0x12, //!< テンプレートクラス名調査中
			// ';' '{'を読み込むと「通常」になる。
			// また、この状態の間は単語を区切る方法を一時的に変更し、
			// 「template_name <paramA,paramB>」のような文字列を一つの単語をみなすようにする。
			// これは特殊化したクラステンプレートを実装する際の構文で有効に働く。	
		M2_NAMESPACE_END	= 0x13,	//!< ネームスペース名調査完了。(';' '{' を読み込んだ時点で「通常」になる。 )
		M2_OPERATOR_WORD	= 0x14, //!< operator名調査中。operatorで '('で次に遷移　template<> names::operator<T>(x)
		M2_TEMPLATE_WORD	= 0x15, //!< テンプレート特殊化を調査中 func<int>()等 '単語 <'でM2_TEMPLATE_WORDになり、'>'(ネスト認識)でM2_NORMAL/M2_OPERATOR_WORDに戻る
		M2_FUNC_NAME_END	= 0x16, //!< 関数名調査完了。(';' '{' を読み込んだ時点で「通常」になる。 )
		M2_AFTER_EQUAL		= 0x05,	//!< '='の後。
			//「通常」かつ nNestLevel_fparam==0 で'='が見つかるとこの状態になる。（ただし "opreator"の直後は除く）
			// ';'が見つかると「通常」に戻る。
			// int val=abs(-1);
			// のような文が関数とみなされないようにするために使用する。
		M2_KR_FUNC	= 0x18,	//!< K&Rスタイル/C++の関数定義を調査する。func() word ←wordがあると遷移する
		M2_AFTER_ITEM		= 0x10,
	} nMode2 = M2_NORMAL;
	MODE2 nMode2Old = M2_NORMAL; // M2_TEMPLATE_WORDになる直前のnMode2
	MODE2 nMode2AttOld = M2_NORMAL;
	bool  bDefinedTypedef = false;	// typedef が書かれている。trueの間は関数名として認識しない。;で復帰する
	bool  bNoFunction = true; // fparamだけど関数定義でない可能性の場合

	const int	nNamespaceNestMax	= 32;			//!< ネスト可能なネームスペース、クラス等の最大数
	int			nNamespaceLen[nNamespaceNestMax+1];	//!< ネームスペース全体の長さ
	const int	nNamespaceLenMax 	= 512;			//!< 最大のネームスペース全体の長さ
	WCHAR		szNamespace[nNamespaceLenMax];		//!< 現在のネームスペース(終端が\0になっているとは限らないので注意)
	const int 	nItemNameLenMax	 	= 256;
	WCHAR		szItemName[nItemNameLenMax];		//!< すぐ前の 関数名 or クラス名 or 構造体名 or 共用体名 or 列挙体名 or ネームスペース名
	WCHAR		szTemplateName[nItemNameLenMax];		//!< すぐ前の 関数名 or クラス名 or 構造体名 or 共用体名 or 列挙体名 or ネームスペース名
	// 例えば下のコードの←の部分での
	// szNamespaceは"Namespace\ClassName\"
	// nMamespaceLenは{10,20}
	// nNestLevel_globalは2となる。
	//
	//　namespace Namespace{
	//　class ClassName{
	//　←
	//　}}
	WCHAR		szRawStringTag[32];	// C++11 raw string litteral
	int nRawStringTagLen = 0;
	int nRawStringTagCompLen = 0;

	CLogicInt	nItemLine(0);		//!< すぐ前の 関数 or クラス or 構造体 or 共用体 or 列挙体 or ネームスペースのある行
	int			nItemFuncId = 0;

	szWordPrev[0] = LCHAR('\0');
	szWord[nWordIdx] = LCHAR('\0');
	szNamespace[0] = LCHAR('\0');	// 2002/10/27 frozen
	nNamespaceLen[0] = 0;	// 2002/10/27 frozen
	szItemName[0] = LCHAR('\0');
	szTemplateName[0] = LCHAR('\0');
	nMode = 0;

	// finalで無名ではない
	auto is_final_context = [](const WCHAR* pszWord, const WCHAR* pszItemName) {
		return wcscmp(LTEXT("final"), pszWord) == 0 && wcscmp(LS(STR_OUTLINE_CPP_NONAME), pszItemName) != 0;
	};
	
	//	Aug. 10, 2004 genta プリプロセス処理クラス
	CCppPreprocessMng cCppPMng;
	bool bExtEol = GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol;
	
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
#ifdef TRACE_OUTLINE
		DEBUG_TRACE(LTEXT("line:%ls"), pLine);
#endif
		for( ; i < nLineLen; ++i ){
#ifdef TRACE_OUTLINE
			DEBUG_TRACE(LTEXT("%2d [%lc] %d %x %d %d %d wd[%ls] pre[%ls] tmp[%ls] til[%ls] %d\n"), int((Int)i), pLine[i], nMode, nMode2,
				nNestLevel_global, nNestLevel_func, nNestLevel_fparam, szWord, szWordPrev, szTemplateName, szItemName, nWordIdx );
#endif
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
			else if( '\\' == pLine[i] && nRawStringTagLen == 0 ){
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
				// operator "" _userliteral
				if( nMode2 == M2_OPERATOR_WORD ){
					auto nLen = int(wcslen(szWordPrev));
					if( nLen + 1 < int(std::size(szWordPrev)) ){
						szWordPrev[nLen] = pLine[i];
						szWordPrev[nLen + 1] = LCHAR('\0');
					}
				}
				if( '"' == pLine[i] ){
					if( nRawStringTagLen ){
						// R"abc(test
						// )abc"
						if( nRawStringTagLen <= i && wcsncmp( szRawStringTag, &pLine[i - nRawStringTagLen], nRawStringTagCompLen ) == 0 ){
							nRawStringTagLen = 0;
							nMode = 0;
							continue;
						}
					}else{
						nMode = 0;
						continue;
					}
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
						if( pLine[i] == LCHAR(':') ){
							if( pLine[i + 1] == LCHAR(':') || (0 < i && pLine[i-1] == LCHAR(':')) ){
								// name ::class or class :: member
							}else{
								// class Klass:base のように:の前にスペースがない場合
								if(nMode2 == M2_NAMESPACE_SAVE)
								{
									// 2021.3.27 class Klass final: public base{ の:だったら名前を上書きしない
									if (szWord[0] != '\0' && !is_final_context(szWord, szItemName))
										wcscpy( szItemName, szWord );
									nMode2 = M2_NAMESPACE_END;
								}
								else if( nMode2 == M2_TEMPLATE_SAVE)
								{
									wcsncat( szTemplateName, szWord, nItemNameLenMax - wcslen(szTemplateName) - 1 );
									nMode2 = M2_NAMESPACE_END;
								}
							}
						}
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = LCHAR('\0');
					}
				}else{
					// 2002/10/27 frozen　ここから
					if( nMode2 == M2_NAMESPACE_SAVE ){
						if(is_final_context(szWord, szItemName)){
							// strcut name final のfinalはクラス名の一部ではない
							// ただし struct finalは名前
						}else{
							wcscpy( szTemplateName, szWord );
							wcscpy( szItemName, szWord );
						}
					}else if( nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						// strcut name<X> final のfinalはクラス名の一部ではない
						// struct name<final> のfinalは一部
						if( wcscmp(LTEXT("final"), szWord) != 0 || nNestLevel_template != 0 ){
							auto nLen = int(wcslen(szTemplateName));
							if( 0 < nLen && C_IsWordChar(szTemplateName[nLen - 1]) && szTemplateName[nLen - 1] != LCHAR(':') && szWord[nWordIdx] != LCHAR(':') ){
								// template func<const x>() のような場合にconstの後ろにスペースを挿入
								if( nLen + 1 < nItemNameLenMax ){
									wcscat( szTemplateName, LTEXT(" ") );
									nLen++;
								}
							}
							wcsncat( szTemplateName, szWord, nItemNameLenMax - nLen - 1 );
						}
					}
					else if( nNestLevel_func == 0 && (nMode2 == M2_NORMAL || nMode2 == M2_FUNC_NAME_END) )	// 2010.07.08 ryoji 関数型マクロ呼出しを関数と誤認することがある問題対策として nMode2 == M2_FUNC_NAME_END 条件を追加し、補正がかかるようにした。
					{
						if( nMode2 == M2_NORMAL )
							nItemFuncId = 0;
						if( wcscmp(szWord,LTEXT("class"))==0 )
							nItemFuncId = FL_OBJ_CLASS;
						else if( wcscmp(szWord,LTEXT("struct"))==0 )
							nItemFuncId = FL_OBJ_STRUCT;
						else if( wcscmp(szWord,LTEXT("namespace"))==0 )
							nItemFuncId = FL_OBJ_NAMESPACE;
						else if( wcscmp(szWord,LTEXT("enum"))==0 )
							nItemFuncId = FL_OBJ_ENUM;
						else if( wcscmp(szWord,LTEXT("union"))==0 )
							nItemFuncId = FL_OBJ_UNION;
						else if( wcscmp(szWord,LTEXT("__interface"))==0 ) // 2007.05.26 genta "__interface" をクラスに類する扱いにする
							nItemFuncId = FL_OBJ_INTERFACE;
						else if( wcscmp(szWord,LTEXT("typedef")) == 0 )
							bDefinedTypedef = true;
						if( nItemFuncId != 0 && nItemFuncId != FL_OBJ_FUNCTION )	//  2010.07.08 ryoji nMode2 == M2_FUNC_NAME_END のときは nItemFuncId == 2 のはず
						{
							nMode2 = M2_NAMESPACE_SAVE;
							nItemLine = nLineCount + CLogicInt(1);
							wcscpy(szItemName,LS(STR_OUTLINE_CPP_NONAME));
						}
					}
					/*else*/ if( nMode2 == M2_FUNC_NAME_END )	// 2010.07.08 ryoji 上で条件変更したので行頭の else を除去
					{
						nMode2 = M2_KR_FUNC;
					}
					// 2002/10/27 frozen　ここまで
					if( nMode2 == M2_NORMAL ){
						// templateは終わった
						szTemplateName[0] = LCHAR('\0');
					}

					//	To Here Mar. 31, 2001 genta
					// 2004/03/12 zenryaku キーワードに _ と PARAMS を使わせない (GNUのコードが見にくくなるから)
					if( !( wcscmp(LTEXT("PARAMS"),szWord) == 0 || wcscmp(LTEXT("_"),szWord) == 0 ) )
						wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = LCHAR('\0');
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* 記号列読み込み中 */
			if( 2 == nMode ){
				if( C_IsWordChar( pLine[i] ) ||
					C_IsSpace( pLine[i], bExtEol ) ||
					 '{' == pLine[i] ||
					 '}' == pLine[i] ||
					 '(' == pLine[i] ||
					 ')' == pLine[i] ||
					 ';' == pLine[i] ||
					'\'' == pLine[i] ||
					 '"' == pLine[i] ||
					 (LCHAR('/') == pLine[i] && (LCHAR('*') == pLine[i+1] || LCHAR('/') == pLine[i+1]))
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = LCHAR('\0');
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
						szWord[nWordIdx + 1] = LCHAR('\0');
					}
					if( nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						auto nItemNameLen = int(wcslen(szTemplateName));
						if(nItemNameLen + 1 < nItemNameLenMax ){
							szTemplateName[nItemNameLen] = pLine[i];
							szTemplateName[nItemNameLen + 1 ] = LCHAR('\0');
						}
					}
					if( nMode2 == M2_OPERATOR_WORD && LCHAR('<') == pLine[i] ){
						const WCHAR* p = &szWord[nWordIdx-8];
						if(  (8 <= nWordIdx && wcsncmp_literal(p, LTEXT("operator<")) == 0)
						 || ((9 <= nWordIdx && wcsncmp_literal(p-1, LTEXT("operator<<")) == 0) && 0 < i && LCHAR('<') == pLine[i-1]) ){
							// 違う：operator<<const() / operator<<()
						}else{
							// operator< <T>() / operator<<<T>() / operator+<T>()
							nMode2Old = nMode2;
							nMode2 = M2_TEMPLATE_WORD;
							wcsncpy( szTemplateName, szWord, nItemNameLenMax );
							szTemplateName[ nItemNameLenMax - 1 ] = LCHAR('\0');
						}
					}
					if( nMode2 == M2_TEMPLATE || nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						if( pLine[i] == LCHAR('<') ){
							nNestLevel_template++;
						}else if( pLine[i] == LCHAR('>') ){
							wcscpy( szItemName, szTemplateName );
							nNestLevel_template--;
							if( nNestLevel_template == 0 ){
								if( nMode2 == M2_TEMPLATE ){
									nMode2 = M2_NORMAL;
								}else if( nMode2 == M2_TEMPLATE_WORD ){
									nMode2 = nMode2Old;
									if( nMode2 == M2_OPERATOR_WORD ){
										wcscpy(szWord, szTemplateName);
										nWordIdx = (int)wcslen(szWord) - 1;
										szTemplateName[0] = LCHAR('\0');
									}
								}
							}
						}
					}
				}
			}else
			/* 長過ぎる単語無視中 */
			if( 999 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( C_IsSpace( pLine[i], bExtEol ) ){
					nMode = 0;
					continue;
				}
			}else
			/* ノーマルモード */
			if( 0 == nMode ){
				/* 空白やタブ記号等を飛ばす */
				if( C_IsSpace( pLine[i], bExtEol ) )
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
					int nLen = (int)wcslen(szWordPrev);
					if( nMode2 == M2_NORMAL && C_IsOperator(szWordPrev, nLen) ){
						// 演算子のオペレータだった operator ""i
						if( nLen + 1 < int(std::size(szWordPrev)) ){
							const WCHAR szOperator[] = { pLine[i], 0 };
							::wcscat_s( szWordPrev, szOperator );
						}
						nMode2 = M2_OPERATOR_WORD;
					}else{
						if( wcscmp( szWordPrev, LTEXT("R") ) == 0 ||
							wcscmp( szWordPrev, LTEXT("LR") ) == 0 ||
							wcscmp( szWordPrev, LTEXT("uR") ) == 0 ||
							wcscmp( szWordPrev, LTEXT("UR") ) == 0 ||
							wcscmp( szWordPrev, LTEXT("u8R") ) == 0
						){
							// C++11 raw string
							// R"abc(string)abc"
							for( int k = i + 1; k < nLineLen; k++ ){
								if( pLine[k] == LCHAR('(') ){
									// i = 1, k = 5, len = 5-1-1=3
									CLogicInt tagLen = t_min(k - i - 1, CLogicInt(int(std::size(szRawStringTag)) - 1));
									nRawStringTagLen = tagLen + 1;
									szRawStringTag[0] = LCHAR(')');
									wcsncpy( szRawStringTag + 1, &pLine[i+1], tagLen );
									szRawStringTag[nRawStringTagLen] = LCHAR('\0');
									nRawStringTagCompLen = (int)wcslen(szRawStringTag);
									break;
								}
							}
						}
					}
					nMode = 21;
					continue;
				}else
				
				// 2002/10/27 frozen ここから
				if( '{' == pLine[i] )
				{
					bool bAddFunction = false;
					if( nMode2 == M2_FUNC_NAME_END || nMode2 == M2_KR_FUNC ){
						bAddFunction = true;
					}
					int nItemNameLen = 0;
					auto nLenDefPos = int(wcslen(LS(STR_OUTLINE_CPP_DEFPOS)));
					if( nNestLevel_func !=0 || (szWordPrev[0] == LCHAR('=') && szWordPrev[1] == LCHAR('\0')) || nMode2 == M2_AFTER_EQUAL )
						++nNestLevel_func;
					else if(
							(nMode2 & M2_AFTER_ITEM) != 0  &&
							nNestLevel_global < nNamespaceNestMax &&
							(nNamespaceLen[nNestLevel_global] +  (nItemNameLen = (int)wcslen(szItemName)) + nLenDefPos + 1) < nNamespaceLenMax &&
							(nItemLine > 0) )
					// ３番目の(&&の後の)条件
					// バッファが足りない場合は項目の追加を行わない。
					// +nLenDefPosは追加する文字列の最大長(追加する文字列は"::定義位置"が最長)
					// +1は終端NUL文字
					{
						wcscpy( &szNamespace[nNamespaceLen[nNestLevel_global]] , szItemName);
						szItemName[0] = LCHAR('\0');
						//	Jan. 30, 2005 genta M2_KR_FUNC 追加
						//	関数の後ろにconst, throw または初期化子があると
						//	M2_KR_FUNCに遷移して，';'が見つからないとその状態のまま
						//	中括弧に遭遇する．
						if( bAddFunction ){
							++ nNestLevel_func;
						}
						else
						{
							++ nNestLevel_global;
							nNamespaceLen[nNestLevel_global] = nNamespaceLen[nNestLevel_global-1] + nItemNameLen;
							if( nItemFuncId == FL_OBJ_NAMESPACE )
								wcscpy(&szNamespace[nNamespaceLen[nNestLevel_global]], LS(STR_OUTLINE_CPP_DEFPOS));
							else
							{
								szNamespace[nNamespaceLen[nNestLevel_global]] = LCHAR('\0');
								szNamespace[nNamespaceLen[nNestLevel_global]+1] = LCHAR(':');
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
#ifdef TRACE_OUTLINE
						DEBUG_TRACE( LTEXT("AppendData %d %ls\n"), nItemLine, szNamespace );
#endif
						pcFuncInfoArr->AppendData( nItemLine, ptPosXY.GetY2() + CLayoutInt(1) , szNamespace, nItemFuncId);
						bDefinedTypedef = false;
						nItemLine = -1;
						//	Jan. 30, 2005 genta M2_KR_FUNC 追加
						if( !bAddFunction )
						{
							szNamespace[nNamespaceLen[nNestLevel_global]] = LCHAR(':');
							nNamespaceLen[nNestLevel_global] += 2;
						}
					}
					else
					{
						//	Jan. 30, 2005 genta M2_KR_FUNC 追加
						if( bAddFunction )
							++ nNestLevel_func;
						else
						{
							++ nNestLevel_global;
							if ( nNestLevel_global <= nNamespaceNestMax )
								nNamespaceLen[nNestLevel_global]=nNamespaceLen[nNestLevel_global-1];
						}
					}
					// bCppInitSkip = false;	//	Mar. 4, 2001 genta
					nNestLevel_template = 0;
					nMode = 0;
					nMode2 = M2_NORMAL;
					nMode2Old = M2_NORMAL;
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
					nMode2Old = M2_NORMAL;
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
					int nLen = (int)wcslen(szWordPrev);
					bool bOperator = false;
					if( nMode2 == M2_NORMAL && nNestLevel_fparam == 0 && C_IsOperator(szWordPrev, nLen) ){
						int k;
						for( k = i + 1; k < nLineLen && C_IsSpace(pLine[k], bExtEol); k++){}
						if( k < nLineLen && pLine[k] == LCHAR(')') ){
							for( k++; k < nLineLen && C_IsSpace(pLine[k], bExtEol); k++){}
							if( k < nLineLen && (pLine[k] == LCHAR('<') || pLine[k] == LCHAR('(') ) ){
								// オペレータだった operator()( / operator()<;
								if( nLen + 1 < int(std::size(szWordPrev)) ){
									szWordPrev[nLen] = pLine[i];
									szWordPrev[nLen + 1] = LCHAR('\0');
								}
								nMode2 = M2_OPERATOR_WORD;
								bOperator = true;
							}else{
								// オペレータかoperatorというCの関数
							}
						}
					}
					//	2007.05.26 genta C++/CLI nMode2 == M2_NAMESPACE_ENDの場合を対象外に
					//	NAMESPACE_END(class クラス名 :の後ろ)においては()を関数とみなさない．
					//	TEMPLATE<sizeof(int)> のようなケースでsizeofを関数と誤認する．
					if( !bOperator && nNestLevel_func == 0
					  && (nMode2 == M2_NORMAL || nMode2 == M2_NAMESPACE_SAVE || nMode2 == M2_OPERATOR_WORD || nMode2 == M2_KR_FUNC) ){
						if(nNestLevel_fparam==0)
						{
							bool bAdd = true;
							if( wcscmp(szWordPrev, LTEXT("__declspec")) == 0
									|| wcscmp(szWordPrev, LTEXT("alignas")) == 0
									|| wcscmp(szWordPrev, LTEXT("decltype")) == 0
									|| wcscmp(szWordPrev, LTEXT("_Alignas")) == 0
									|| wcscmp(szWordPrev, LTEXT("__attribute__")) == 0
							){
								bAdd = false;
							}else{
								if( !(nMode2 == M2_KR_FUNC && bInInitList) && !(nMode2 == M2_KR_FUNC
									&& CPP_IsFunctionAfterKeyword(szWordPrev)) )	// 2010.07.08 ryoji 初期化リストに入る以前までは後発の名前を優先的に関数名候補とする
								{
									bNoFunction = false;
								}else{
									bAdd = false;
								}
							}
							if( bAdd ){
								if( szTemplateName[0] ){
									wcscpy(szItemName, szTemplateName);
								}else{
									wcscpy(szItemName, szWordPrev);
								}
								nItemLine = nLineCount + CLogicInt(1);
							}
						}
						++ nNestLevel_fparam;
					}
					//  2002/10/27 frozen ここまで
					if( nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						auto nItemNameLen = int(wcslen(szTemplateName));
						if( nItemNameLen + 1 < nItemNameLenMax ){
							szTemplateName[nItemNameLen] = pLine[i];
							szTemplateName[nItemNameLen + 1 ] = LCHAR('\0');
						}
					}
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
						if( nNestLevel_fparam == 0 && nMode2 != M2_ATTRIBUTE && nMode2 != M2_TEMPLATE_WORD ){
							if( nMode2 == M2_NORMAL )
								bInInitList = false;
							if( !bNoFunction ){
								nMode2 = M2_FUNC_NAME_END;
								nItemFuncId = FL_OBJ_FUNCTION;
							}
						}
					}
					//  2002/10/27 frozen ここまで
					if( nMode2 == M2_OPERATOR_WORD ){
						auto nLen = int(wcslen(szWordPrev));
						if( nLen + 1 < int(std::size(szWordPrev)) ){
							szWordPrev[nLen] = pLine[i];
							szWordPrev[nLen + 1] = LCHAR('\0');
						}
					}else if( nMode2 == M2_TEMPLATE_SAVE || nMode2 == M2_TEMPLATE_WORD ){
						auto nItemNameLen = int(wcslen(szTemplateName));
						if( nItemNameLen + 1 < nItemNameLenMax ){
							szTemplateName[nItemNameLen] = pLine[i];
							szTemplateName[nItemNameLen + 1 ] = LCHAR('\0');
						}
					}
					continue;
				}else
				// From Here 2007.05.26 genta C++/CLI Attributeの取り扱い
				if( '[' == pLine[i] ){
					int nLen = (int)wcslen(szWordPrev);
					if( nMode2 == M2_NORMAL && C_IsOperator(szWordPrev, nLen) ){
						// 演算子のオペレータだった operator []
						if( nLen + 1 < int(std::size(szWordPrev)) ){
							const WCHAR szOperator[] = { pLine[i], 0 };
							::wcscat_s( szWordPrev, szOperator );
						}
						nMode2 = M2_OPERATOR_WORD;
					}else
					if( nNestLevel_func == 0 && nNestLevel_fparam == 0 &&
						(nMode2 == M2_NORMAL || nMode2 == M2_NAMESPACE_SAVE || nMode2 ==  M2_TEMPLATE_SAVE
						 || nMode2 == M2_OPERATOR_WORD || nMode2 == M2_TEMPLATE_WORD )) {
						nMode2AttOld = nMode2;
						nMode2 = M2_ATTRIBUTE;
					}
					continue;
				} else
				if( ']' == pLine[i] ){
					//	Attribute内部でも[]を配列として使うかもしれないので，
					//	括弧のレベルは元に戻っている必要有り
					if( nNestLevel_fparam == 0 && nMode2 == M2_ATTRIBUTE ) {
						nMode2 = nMode2AttOld;
						continue;
					}
					if( nMode2 == M2_OPERATOR_WORD ){
						auto nLen = int(wcslen(szWordPrev));
						if( nLen + 1 < int(std::size(szWordPrev)) ){
							szWordPrev[nLen] = pLine[i];
							szWordPrev[nLen + 1] = LCHAR('\0');
						}
					}
				} else
				// To Here 2007.05.26 genta C++/CLI Attributeの取り扱い
				if( ';' == pLine[i] ){
					//  2002/10/27 frozen ここから
					if( nMode2 == M2_KR_FUNC )
					{
						// zenryaku K&Rスタイルの関数宣言の終了後 M2_FUNC_NAME_END にもどす
						nMode2 = M2_FUNC_NAME_END;
						if( nOutlineType == OUTLINE_C ){
							// CのときはK&R定義。
							continue;
						}
						// それ以外はC++の以下のような場合
						// int funcname() const;
						// void funcname() throw(MyException);
						// auto funcname() -> int;
					} //	Jan. 30, 2005 genta K&R処理に引き続いて宣言処理も行う．
					if( nMode2 == M2_FUNC_NAME_END &&
						nNestLevel_global < nNamespaceNestMax &&
						(nNamespaceLen[nNestLevel_global] + wcslen(szItemName)) < nNamespaceLenMax &&
						nNestPoint_class == 0 && !bDefinedTypedef && 0 < nItemLine)
					// ３番目の(&&の後の)条件
					// バッファが足りない場合は項目の追加を行わない。
					{
						wcscpy( &szNamespace[nNamespaceLen[ nNestLevel_global]] , szItemName);

						nItemFuncId = FL_OBJ_DECLARE;
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
#ifdef TRACE_OUTLINE
						DEBUG_TRACE( LTEXT("AppendData %d %ls\n"), nItemLine, szNamespace );
#endif
						pcFuncInfoArr->AppendData( nItemLine, ptPosXY.GetY2() + CLayoutInt(1), szNamespace, nItemFuncId);
					}
					nItemLine = -1;
					szWordPrev[0] = LCHAR('\0');
					szTemplateName[0] = LCHAR('\0'); // hoge < ... ; はtemplateではなかったのでクリア
					nNestLevel_template = 0;
					nMode2 = M2_NORMAL;
					//  2002/10/27 frozen ここまで
					bNoFunction = true;
					bDefinedTypedef = false;
					nMode2Old = M2_NORMAL;
					nMode = 0;
					continue;
				}else if( nNestLevel_fparam == 0 && nMode2 != M2_ATTRIBUTE ){
					// 2007.05.26 genta C++/CLI Attribute内部では関数名処理は一切行わない
					if( C_IsWordChar( pLine[i] ) ){
						//	//	Mar. 15, 2000 genta
						//	From Here
						//	直前のwordの最後が::か，あるいは直後のwordの先頭が::なら
						//	クラス限定子と考えて両者を接続する．
						{
							auto pos = int(wcslen(szWordPrev)) - 2;
							if( //	前の文字列の末尾チェック
								( pos > 0 &&	szWordPrev[pos] == LCHAR(':') &&
								szWordPrev[pos + 1] == LCHAR(':') ) ||
								//	次の文字列の先頭チェック
								( i < nLineLen - 1 && pLine[i] == LCHAR(':') &&
									pLine[i+1] == LCHAR(':') )
							){
								//	前の文字列に続ける
								if( nMode2 == M2_NORMAL || nMode2 == M2_OPERATOR_WORD ){
									if( szTemplateName[0] ){
										wcscpy( szWord, szTemplateName );
										szTemplateName[0] = '\0';
									}else{
										wcscpy( szWord, szWordPrev );
									}
								}
								nWordIdx = (int)wcslen( szWord );
							}
							//	From Here Apr. 1, 2001 genta
							//	operator new/delete 演算子の対応
							else if( nMode2 == M2_NORMAL && C_IsOperator( szWordPrev, pos + 2 ) ){
								if( -1 < pos && C_IsWordChar( szWordPrev[pos + 1] ) ){
									//	スペースを入れて、前の文字列に続ける
									szWordPrev[pos + 2] = LCHAR(' ');
									szWordPrev[pos + 3] = LCHAR('\0');
								}
								wcscpy( szWord, szWordPrev );
								nWordIdx = (int)wcslen( szWord );
								nMode2 = M2_OPERATOR_WORD;
							}else if( nMode2 == M2_OPERATOR_WORD ){
								// operator 継続中
								if( -1 < pos && C_IsWordChar( szWordPrev[pos + 1] ) ){
									//	スペースを入れて、前の文字列に続ける
									szWordPrev[pos + 2] = LCHAR(' ');
									szWordPrev[pos + 3] = LCHAR('\0');
								}
								wcscpy( szWord, szWordPrev );
								nWordIdx = (int)wcslen( szWord );
							}
							//	To Here Apr. 1, 2001 genta
							else{
								nWordIdx = 0;
							}
						}
						//	wcscpy( szWordPrev, szWord );	不要？
						//	To Here
						if( pLine[i] == LCHAR(':') && pLine[i+1] != LCHAR(':'))
						{
							if(nMode2 == M2_NAMESPACE_SAVE)
							{
								if(szWord[0]!='\0')
									wcscpy( szItemName, szWord );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_TEMPLATE_SAVE)
							{
								wcsncat( szTemplateName, szWord, nItemNameLenMax - wcslen(szTemplateName) - 1 );
								nMode2 = M2_NAMESPACE_END;
							}
							else if( nMode2 == M2_FUNC_NAME_END || nMode2 == M2_KR_FUNC )
							{
								bInInitList = true;
							}
						}

						//	//	Mar. 15, 2000 genta
						//	From Here
						//	長さチェックは必須
						if( nWordIdx < nMaxWordLeng ){
							nMode = 1;
							::wcsncpy_s(&szWord[nWordIdx], nMaxWordLeng - nWordIdx, &pLine[i], 1);
						}
						else{
							nMode = 999;
						}
						//	To Here
					}else{
						// 2011.12.02 template対応
						if( nMode2 == M2_NORMAL || nMode2 == M2_OPERATOR_WORD ){
							if( pLine[i] == LCHAR('<') ){
								int nLen = (int)wcslen(szWordPrev);
								if( wcscmp(szWordPrev, LTEXT("template")) == 0 ){
									nMode2 = M2_TEMPLATE;
									szTemplateName[0] = LCHAR('\0');
								}else if( C_IsOperator(szWordPrev, nLen) ){
									// operator<
								}else{
									// int func <int>();
									nMode2Old = nMode2;
									nMode2 = M2_TEMPLATE_WORD;
									wcsncpy( szTemplateName, szWordPrev, nItemNameLenMax );
									szTemplateName[ nItemNameLenMax - 1 ] = LCHAR('\0');
								}
							}else{
								szTemplateName[0] = LCHAR('\0');
							}
						}
						//	Aug. 13, 2004 genta
						//	szWordPrevが失われないうちにoperatorの判定を行う
						//	operatorの判定は前にクラス名が付いている可能性があるので
						//	専用の判定関数を使うべし．
						//	operatorで無ければ=は代入なのでここは宣言文ではない．
						int nLen = (int)wcslen(szWordPrev);
						if( pLine[i] == LCHAR('=') && nNestLevel_func == 0
							&& nMode2 == M2_NORMAL && ! C_IsOperator(szWordPrev,nLen) ){
							nMode2 = M2_AFTER_EQUAL;
						}else if( nMode2 == M2_NORMAL && C_IsOperator(szWordPrev, nLen) ){
							// 演算子のオペレータだった operator +
							wcscpy(szWord, szWordPrev);
							nWordIdx = (int)nLen -1;
							nMode2 = M2_OPERATOR_WORD;
						}else if( nMode2 == M2_OPERATOR_WORD ){
							// operator 継続中
							wcscpy(szWord, szWordPrev);
							nWordIdx = (int)nLen -1;
						}else{
							wcscpy(szWordPrev, szWord);
							nWordIdx = -1;
						}
						// 2002/10/27 frozen ここから
						if( nMode2 == M2_NAMESPACE_SAVE )
						{
#if 0
							if( pLine[i] == LCHAR('>') || pLine[i] == LCHAR(',') || pLine[i] == LCHAR('='))
								// '<' の前に '>' , ',' , '=' があったので、おそらく
								// 前にあった"class"はテンプレートパラメータの型を表していたと考えられる。
								// よって、クラス名の調査は終了。
								// '>' はテンプレートパラメータの終了
								// ',' はテンプレートパラメータの区切り
								// '=' はデフォルトテンプレートパラメータの指定
								nMode2 = M2_NORMAL; 
							else
#endif
							if( pLine[i] == LCHAR('<') )
								nMode2 = M2_TEMPLATE_SAVE;
						}
						nMode = 2;
						i--;
					}
				}
			}
		}
	}
}

struct SCommentBlock{
	int nFirst;
	int nLast;
};
static bool IsCommentBlock( std::vector<SCommentBlock>& arr, int pos )
{
	int size = (int)arr.size();
	for(int i = 0; i < size; i++){
		if( arr[i].nFirst <= pos && pos <= arr[i].nLast ){
			return true;
		}
	}
	return false;
}

/* C/C++スマートインデント処理 */
void CEditView::SmartIndent_CPP( WCHAR wcChar )
{
	const WCHAR*	pLine;
	CLogicInt		nLineLen;
	int			k;
	const WCHAR*	pLine2;
	CLogicInt	nLineLen2;
	int			nLevel;
	CLogicInt j;

	/* 調整によって置換される箇所 */
	CLogicRange sRangeA;
	sRangeA.Clear(-1);

	WCHAR*	pszData = nullptr;
	CLogicInt	nDataLen;

	int			nWork = 0;
	int			nCharChars;
	int			nSrcLen;
	WCHAR		pszSrc[1024];
	BOOL		bChange;

	int			nCaretPosX_PHY;

	CLogicPoint	ptCP;

	if(wcChar==WCODE::CR || wcschr(LTEXT(":{}()"),wcChar)!=nullptr){
		//次へ進む
	}else return;

	switch( wcChar ){
	case WCODE::CR:
	case LCHAR(':'):
	case LCHAR('}'):
	case LCHAR(')'):
	case LCHAR('{'):
	case LCHAR('('):
	{
		WCHAR wcCharOrg = wcChar;
		nCaretPosX_PHY = GetCaret().GetCaretLogicPos().x;

		pLine = m_pcEditDoc->m_cDocLineMgr.GetLine(GetCaret().GetCaretLogicPos().GetY2())->GetDocLineStrWithEOL(&nLineLen);
		if( nullptr == pLine ){
			if( WCODE::CR != wcChar ){
				return;
			}
			/* 調整によって置換される箇所 */
			sRangeA.Set(CLogicPoint(0,GetCaret().GetCaretLogicPos().y));
		}else{

			//	nWorkに処理の基準桁位置を設定する
			if( WCODE::CR != wcChar ){
				nWork = nCaretPosX_PHY - 1;
			}else{
				/*
				|| CRが入力された時、カーソル直後の識別子をインデントする。
				|| カーソル直後の識別子が'}'や')'ならば
				|| '}'や')'が入力された時と同じ処理をする
				*/

				int i;
				for( i = nCaretPosX_PHY; i < nLineLen; i++ ){
					if( WCODE::TAB != pLine[i] && WCODE::SPACE != pLine[i] ){
						break;
					}
				}
				if( i < nLineLen ){
					// 2005-09-02 D.S.Koba GetSizeOfChar
					nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
					if( 1 == nCharChars && ( pLine[i] == LCHAR(')') || pLine[i] == LCHAR('}') ) ){
						wcChar = pLine[i];
					}
					nCaretPosX_PHY = i;
					nWork = nCaretPosX_PHY;
				}else{
					nWork = nCaretPosX_PHY;
				}
			}
			int i;
			for( i = 0; i < nWork; i++ ){
				if( WCODE::TAB != pLine[i] && WCODE::SPACE != pLine[i] ){
					break;
				}
			}
			if( i < nWork ){
				if( ( LCHAR(':') == wcChar && IsHeadCppKeyword(&pLine[i]) )
					//	Sep. 18, 2002 かろと
					|| ( LCHAR('{') == wcChar && LCHAR('#') != pLine[i] )
					|| ( LCHAR('(') == wcChar && LCHAR('#') != pLine[i] )
				){

				}else{
					return;
				}
			}else{
				if( LCHAR(':') == wcChar ){
					return;
				}
			}
			/* 調整によって置換される箇所 */
			sRangeA.SetFrom(CLogicPoint(0, GetCaret().GetCaretLogicPos().GetY2()));
			sRangeA.SetTo(CLogicPoint(i, GetCaret().GetCaretLogicPos().GetY2()));
		}

		/* 対応する括弧をさがす */
		nLevel = 0;	/* {}の入れ子レベル */

		nDataLen = CLogicInt(0);
		for( j = GetCaret().GetCaretLogicPos().GetY2(); j >= CLogicInt(0); --j ){
			pLine2 = m_pcEditDoc->m_cDocLineMgr.GetLine(j)->GetDocLineStrWithEOL(&nLineLen2);
			if( j == GetCaret().GetCaretLogicPos().y ){
				// 2005.10.11 ryoji EOF のみの行もスマートインデントの対象にする
				if( nullptr == pLine2 ){
					if( GetCaret().GetCaretLogicPos().y == m_pcEditDoc->m_cDocLineMgr.GetLineCount() )
						continue;	// EOF のみの行
					break;
				}
				nCharChars = CLogicInt(&pLine2[nWork] - CNativeW::GetCharPrev( pLine2, nLineLen2, &pLine2[nWork] ));
				k = nWork - nCharChars;
			}else{
				if( nullptr == pLine2 )
					break;
				nCharChars = CLogicInt(&pLine2[nLineLen2] - CNativeW::GetCharPrev( pLine2, nLineLen2, &pLine2[nLineLen2] ));
				k = nLineLen2 - nCharChars;
			}

			// コメント・文字列検索
			bool bCommentStringCheck = m_pTypeData->m_bIndentCppStringIgnore || m_pTypeData->m_bIndentCppCommentIgnore;
			std::vector<SCommentBlock> arrCommentBlock;
			if( bCommentStringCheck ){
				int nMode = 0;
				int nBegin = -1;
				CLogicPoint  pointLogic(CLogicInt(0), j);
				CLayoutPoint pointLayout;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout(pointLogic, &pointLayout);
				const CLayout* layout = m_pcEditDoc->m_cLayoutMgr.SearchLineByLayoutY( pointLayout.GetY() );
				if( layout == nullptr ){
					nMode = 0;
				}else{
					EColorIndexType eColorIndex = layout->GetColorTypePrev();
					switch( eColorIndex ){
					case COLORIDX_SSTRING:
						if( m_pTypeData->m_bIndentCppStringIgnore ){
							nMode = 2;
							nBegin = 0;
						}
						break;
					case COLORIDX_WSTRING:
						if( m_pTypeData->m_bIndentCppStringIgnore ){
							nMode = 1;
							nBegin = 0;
						}
						break;
					case COLORIDX_BLOCK1:
						if( m_pTypeData->m_bIndentCppCommentIgnore ){
							nMode = 3;
							nBegin = 0;
						}
						break;
					default:
						break;
					}
				}
				for( int n = 0; n < nLineLen2; n++ ){
					switch( nMode ){
					case 0:
						if( m_pTypeData->m_bIndentCppStringIgnore && LCHAR('"') == pLine2[n] ){
							nMode = 1;
							nBegin = n;
						}else if( m_pTypeData->m_bIndentCppStringIgnore &&  LCHAR('\'') == pLine2[n] ){
							nMode = 2;
							nBegin = n;
						}else if( m_pTypeData->m_bIndentCppCommentIgnore &&  n + 1 < nLineLen2 && '/' == pLine2[n] && '*' == pLine2[n+1] ){
							nMode = 3;
							nBegin = n;
							n++;
						}else if( m_pTypeData->m_bIndentCppCommentIgnore && n + 1 < nLineLen2 && '/' == pLine2[n] && '/' == pLine2[n+1] ){
							SCommentBlock block = { n, nLineLen2 - 1 };
							arrCommentBlock.push_back(block);
							n = nLineLen2 - 1;
						}
						break;
					case 1:
						if( LCHAR('\\') == pLine2[n] ){
							n++;
						}else if( LCHAR('"') == pLine2[n] ){
							SCommentBlock block = { nBegin, n };
							arrCommentBlock.push_back(block);
							nBegin = -1;
							nMode = 0;
						}
						break;
					case 2:
						if( LCHAR('\\') == pLine2[n] ){
							n++;
						}else if( LCHAR('\'') == pLine2[n] ){
							SCommentBlock block = { nBegin, n };
							arrCommentBlock.push_back(block);
							nBegin = -1;
							nMode = 0;
						}
						break;
					case 3:
						if( n + 1 < nLineLen2 && '*' == pLine2[n] && '/' == pLine2[n+1] ){
							SCommentBlock block = { nBegin, n + 1 };
							arrCommentBlock.push_back(block);
							nBegin = -1;
							nMode = 0;
							n++;
						}
						break;
					default:
						break;
					}
				}
				if( 0 < nBegin ){
					// 終わりのない文字列orコメント
					SCommentBlock block = { nBegin, nLineLen2 };
					arrCommentBlock.push_back(block);
				}
				if( j == GetCaret().GetCaretLogicPos().GetY2() ){
					if( wcCharOrg != WCODE::CR ){
						int pos = GetCaret().GetCaretLogicPos().GetX();
						if( IsCommentBlock(arrCommentBlock, pos) ){
							// もし最後に入力した文字がコメント内だったら何もしない
							return;
						}
					}
				}
			}

			for( ; k >= 0; /*k--*/ ){
				if( 1 == nCharChars && ( LCHAR('}') == pLine2[k] || LCHAR(')') == pLine2[k] )
				){
					if( 0 < k && LCHAR('\'') == pLine2[k - 1]
					 && nLineLen2 - 1 > k && LCHAR('\'') == pLine2[k + 1]
					){
//						MYTRACE( L"▼[%ls]\n", pLine2 );
					}else if( bCommentStringCheck && IsCommentBlock(arrCommentBlock, k) ){
					}else{
						//同じ行の場合
						if( j == GetCaret().GetCaretLogicPos().y ){
							if( LCHAR('{') == wcChar && LCHAR('}') == pLine2[k] ){
								wcChar = LCHAR('}');
								nLevel--;	/* {}の入れ子レベル */
							}
							if( LCHAR('(') == wcChar && LCHAR(')') == pLine2[k] ){
								wcChar = LCHAR(')');
								nLevel--;	/* {}の入れ子レベル */
							}
						}

						nLevel++;	/* {}の入れ子レベル */
					}
				}
				if( 1 == nCharChars && ( LCHAR('{') == pLine2[k] || LCHAR('(') == pLine2[k] )
				){
					if( 0 < k && LCHAR('\'') == pLine2[k - 1]
					 && nLineLen2 - 1 > k && LCHAR('\'') == pLine2[k + 1]
					){
//						MYTRACE( L"▼[%ls]\n", pLine2 );
					}else if( bCommentStringCheck && IsCommentBlock(arrCommentBlock, k) ){
					}else{
						//同じ行の場合
						if( j == GetCaret().GetCaretLogicPos().y ){
							if( LCHAR('{') == wcChar && LCHAR('{') == pLine2[k] ){
								return;
							}
							if( LCHAR('(') == wcChar && LCHAR('(') == pLine2[k] ){
								return;
							}
						}
						if( 0 == nLevel ){
							break;
						}else{
							nLevel--;	/* {}の入れ子レベル */
						}
					}
				}
				nCharChars = CLogicInt(&pLine2[k] - CNativeW::GetCharPrev( pLine2, nLineLen2, &pLine2[k] ));
				if( 0 == nCharChars ){
					nCharChars = CLogicInt(1);
				}
				k -= nCharChars;
			}
			if( k < 0 ){
				/* この行にはない */
				continue;
			}

			{
				int m;
				for( m = 0; m < nLineLen2; m++ ){
					if( WCODE::TAB != pLine2[m] && WCODE::SPACE != pLine2[m] ){
						break;
					}
				}
				nDataLen = CLogicInt(m);
			}

			nCharChars = (m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bInsSpace)? (Int)m_pcEditDoc->m_cLayoutMgr.GetTabSpaceKetas(): Int(1);
			pszData = new WCHAR[nDataLen + nCharChars + 1];
			wmemcpy( pszData, pLine2, nDataLen );
			if( WCODE::CR  == wcChar || LCHAR('{') == wcChar || LCHAR('(') == wcChar ){
				// 2005.10.11 ryoji TABキーがSPACE挿入の設定なら追加インデントもSPACEにする
				//	既存文字列の右端の表示位置を求めた上で挿入するスペースの数を決定する
				if( m_pcEditDoc->m_cDocType.GetDocumentAttribute().m_bInsSpace ){	// SPACE挿入設定
					int i;
					CKetaXInt m = CKetaXInt(0);
					i = 0;
					while( i < nDataLen ){
						nCharChars = CNativeW::GetSizeOfChar( pszData, nDataLen, i );
						CKetaXInt nCharKetas = CNativeW::GetKetaOfChar( pszData, nDataLen, i );
						if( nCharChars == 1 && WCODE::TAB == pszData[i] )
							m += m_pcEditDoc->m_cLayoutMgr.GetActualTabSpaceKetas(m);
						else
							m += nCharKetas;
						i += nCharChars;
					}
					nCharChars = (Int)m_pcEditDoc->m_cLayoutMgr.GetActualTabSpaceKetas(m);
					for( int i2 = 0; i2 < nCharChars; i2++ )
						pszData[nDataLen + i2] = WCODE::SPACE;
					pszData[nDataLen + nCharChars] = LCHAR('\0');
					nDataLen += CLogicInt(nCharChars);
				}else{
					pszData[nDataLen] = WCODE::TAB;
					pszData[nDataLen + 1] = LCHAR('\0');
					++nDataLen;
				}
			}else{
				pszData[nDataLen] = LCHAR('\0');
			}
			break;
		}
		if( j < 0 ){
			/* 対応する括弧が見つからなかった */
			if( WCODE::CR == wcChar ){
				return;
			}else{
				nDataLen = CLogicInt(0);
				pszData = new WCHAR[nDataLen + 1];
				pszData[nDataLen] = LCHAR('\0');
			}
		}

		/* 調整後のカーソル位置を計算しておく */
		ptCP.x = nCaretPosX_PHY - sRangeA.GetTo().x + nDataLen;
		ptCP.y = GetCaret().GetCaretLogicPos().y;

		nSrcLen = sRangeA.GetTo().x - sRangeA.GetFrom().x;
		if( nSrcLen >= int(std::size(pszSrc)) - 1 ){
			//	Sep. 18, 2002 genta メモリリーク対策
			delete [] pszData;
			return;
		}
		if( nullptr == pLine ){
			pszSrc[0] = LCHAR('\0');
		}else{
			wmemcpy( pszSrc, &pLine[sRangeA.GetFrom().x], nSrcLen );
			pszSrc[nSrcLen] = LCHAR('\0');
		}

		/* 調整によって置換される箇所 */
		CLayoutRange sRangeLayout;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( sRangeA, &sRangeLayout );

		if( ( 0 == nDataLen && sRangeLayout.IsOne() )
		 || ( nDataLen == nSrcLen && 0 == wmemcmp( pszSrc, pszData, nDataLen ) )
		 ){
			bChange = FALSE;
		}else{
			bChange = TRUE;
			if( m_pTypeData->m_bIndentCppUndoSep ){
				//キー入力とインデントを別のアンドゥバッファにする
				SetUndoBuffer();
				if( m_cCommander.GetOpeBlk() == nullptr ){
					m_cCommander.SetOpeBlk(new COpeBlk);
				}
				m_cCommander.GetOpeBlk()->AddRef();
			}

			/* データ置換 削除&挿入にも使える */
			ReplaceData_CEditView(
				sRangeLayout,
				pszData,	/* 挿入するデータ */
				nDataLen,	/* 挿入するデータの長さ */
				true,
				m_bDoing_UndoRedo?nullptr:m_cCommander.GetOpeBlk()
			);
		}

		/* カーソル位置調整 */
		CLayoutPoint ptCP_Layout;
		m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptCP, &ptCP_Layout );

		/* 選択エリアの先頭へカーソルを移動 */
		GetCaret().MoveCursor( ptCP_Layout, true );
		GetCaret().m_nCaretPosX_Prev = GetCaret().GetCaretLayoutPos().GetX();

		if( bChange && !m_bDoing_UndoRedo ){	/* アンドゥ・リドゥの実行中か */
			/* 操作の追加 */
			m_cCommander.GetOpeBlk()->AppendOpe(
				new CMoveCaretOpe(
					GetCaret().GetCaretLogicPos()	// 操作前後のキャレット位置
				)
			);
		}
		break;
	}
	default:
		break;
	}
	if( nullptr != pszData ){
		delete [] pszData;
		pszData = nullptr;
	}
}

const WCHAR* g_ppszKeywordsCPP[] = {
	LTEXT("#define"),
	LTEXT("#elif"),
	LTEXT("#elifdef"),
	LTEXT("#elifndef"),
	LTEXT("#else"),
	LTEXT("#embed"),
	LTEXT("#endif"),
	LTEXT("#error"),
	LTEXT("#if"),
	LTEXT("#ifdef"),
	LTEXT("#ifndef"),
	LTEXT("#include"),
	LTEXT("#line"),
	LTEXT("#pragma"),
	LTEXT("#undef"),
	LTEXT("_Alignas"),
	LTEXT("_Alignof"),
	LTEXT("_Atomic"),
	LTEXT("_Bool"),
	LTEXT("_Complex"),
	LTEXT("_Generic"),
	LTEXT("_Imaginary"),
	LTEXT("_Noreturn"),
	LTEXT("_Pragma"),
	LTEXT("_Static_assert"),
	LTEXT("_Thread_local"),
	LTEXT("__DATE__"),
	LTEXT("__FILE__"),
	LTEXT("__FUNCTION__"),
	LTEXT("__FUNCDNAME__"),
	LTEXT("__FUNCSIG__"),
	LTEXT("__LINE__"),
	LTEXT("__PRETTY_FUNCTION__"),
	LTEXT("__TIME__"),
	LTEXT("__VA_ARGS__"),
	LTEXT("__cplusplus"),
	LTEXT("__declspec"),
	LTEXT("__func__"),
	LTEXT("__forceinline"),
	LTEXT("__inline"),
	LTEXT("__has_cpp_attribute"),
	LTEXT("__has_embed"),
	LTEXT("__has_include"),
	LTEXT("__restrict"),
	LTEXT("__restrict__"),
	LTEXT("alignas"),
	LTEXT("alignof"),
	LTEXT("and"),
	LTEXT("and_eq"),
	LTEXT("asm"),
	LTEXT("auto"),
	LTEXT("bitand"),
	LTEXT("bitor"),
	LTEXT("bool"),
	LTEXT("break"),
	LTEXT("case"),
	LTEXT("catch"),
	LTEXT("char"),
	LTEXT("char8_t"),
	LTEXT("char16_t"),
	LTEXT("char32_t"),
	LTEXT("class"),
	LTEXT("compl"),
	LTEXT("concept"),
	LTEXT("const"),
	LTEXT("consteval"),
	LTEXT("constexpr"),
	LTEXT("constinit"),
	LTEXT("const_cast"),
	LTEXT("continue"),
	LTEXT("contract_assert"),
	LTEXT("co_await"),
	LTEXT("co_return"),
	LTEXT("co_yield"),
	LTEXT("decltype"),
	LTEXT("default"),
	LTEXT("define"),
	LTEXT("defined"),
	LTEXT("delete"),
	LTEXT("do"),
	LTEXT("double"),
	LTEXT("dynamic_cast"),
	LTEXT("elif"),
	LTEXT("elifdef"),
	LTEXT("elifndef"),
	LTEXT("else"),
	LTEXT("embed"),
	LTEXT("endif"),
	LTEXT("enum"),
	LTEXT("error"),
	LTEXT("explicit"),
	LTEXT("export"),
	LTEXT("extern"),
	LTEXT("false"),
	LTEXT("final"),
	LTEXT("float"),
	LTEXT("for"),
	LTEXT("friend"),
	LTEXT("goto"),
	LTEXT("if"),
	LTEXT("ifdef"),
	LTEXT("ifndef"),
	LTEXT("import"),
	LTEXT("include"),
	LTEXT("inline"),
	LTEXT("int"),
	LTEXT("line"),
	LTEXT("long"),
	LTEXT("module"),
	LTEXT("mutable"),
	LTEXT("namespace"),
	LTEXT("new"),
	LTEXT("noexcept"),
	LTEXT("not"),
	LTEXT("not_eq"),
	LTEXT("nullptr"),
	LTEXT("operator"),
	LTEXT("or"),
	LTEXT("or_eq"),
	LTEXT("override"),
	LTEXT("pragma"),
	LTEXT("private"),
	LTEXT("protected"),
	LTEXT("public"),
	LTEXT("register"),
	LTEXT("reinterpret_cast"),
	LTEXT("requires"),
	LTEXT("restrict"),
	LTEXT("return"),
	LTEXT("short"),
	LTEXT("signed"),
	LTEXT("sizeof"),
	LTEXT("static"),
	LTEXT("static_assert"),
	LTEXT("static_cast"),
	LTEXT("struct"),
	LTEXT("switch"),
	LTEXT("template"),
	LTEXT("this"),
	LTEXT("thread_local"),
	LTEXT("throw"),
	LTEXT("true"),
	LTEXT("try"),
	LTEXT("typedef"),
	LTEXT("typeid"),
	LTEXT("typename"),
	LTEXT("undef"),
	LTEXT("union"),
	LTEXT("unsigned"),
	LTEXT("using"),
	LTEXT("virtual"),
	LTEXT("void"),
	LTEXT("volatile"),
	LTEXT("warning"),
	LTEXT("wchar_t"),
	LTEXT("while"),
	LTEXT("xor"),
	LTEXT("xor_eq"),
};
int g_nKeywordsCPP = int(std::size(g_ppszKeywordsCPP));
