#include "StdAfx.h"
#include "CWordParse.h"
#include "charset/charcode.h"


//@@@ 2001.06.23 N.Nakatani
/*!
	@brief 現在位置の単語の範囲を調べる staticメンバ
	@author N.Nakatani
	@retval true	成功 現在位置のデータは「単語」と認識する。
	@retval false	失敗 現在位置のデータは「単語」とは言いきれない気がする。
*/
bool CWordParse::WhereCurrentWord_2(
	const wchar_t*	pLine,			//!< [in]  調べるメモリ全体の先頭アドレス
	CLogicInt		nLineLen,		//!< [in]  調べるメモリ全体の有効長
	CLogicInt		nIdx,			//!< [in]  調査開始地点:pLineからの相対的な位置
	CLogicInt*		pnIdxFrom,		//!< [out] 単語が見つかった場合は、単語の先頭インデックスを返す。
	CLogicInt*		pnIdxTo,		//!< [out] 単語が見つかった場合は、単語の終端の次のバイトの先頭インデックスを返す。
	CNativeW*		pcmcmWord,		//!< [out] 単語が見つかった場合は、現在単語を切り出して指定されたCMemoryオブジェクトに格納する。情報が不要な場合はNULLを指定する。
	CNativeW*		pcmcmWordLeft	//!< [out] 単語が見つかった場合は、現在単語の左に位置する単語を切り出して指定されたCMemoryオブジェクトに格納する。情報が不要な場合はNULLを指定する。
)
{
	using namespace WCODE;

	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;

	if( NULL == pLine ){
		return false;
	}
	if( nIdx >= nLineLen ){
		return false;
	}

	// 現在位置の文字の種類によっては選択不可
	if( WCODE::IsLineDelimiter(pLine[nIdx], GetDllShareData().m_Common.m_sEdit.m_bEnableExtEol) ){
		return false;
	}

	// 現在位置の文字の種類を調べる
	ECharKind nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );

	// 文字種類が変わるまで前方へサーチ
	CLogicInt	nIdxNext = nIdx;
	CLogicInt	nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
	while( nCharChars > 0 ){
		CLogicInt	nIdxNextPrev = nIdxNext;
		nIdxNext -= nCharChars;
		ECharKind	nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );

		ECharKind nCharKindMerge = WhatKindOfTwoChars( nCharKindNext, nCharKind );
		if( nCharKindMerge == CK_NULL ){
			nIdxNext = nIdxNextPrev;
			break;
		}
		nCharKind = nCharKindMerge;
		nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
	}
	*pnIdxFrom = nIdxNext;

	if( NULL != pcmcmWordLeft ){
		pcmcmWordLeft->SetString( &pLine[*pnIdxFrom], nIdx - *pnIdxFrom );
	}

	// 文字種類が変わるまで後方へサーチ
	nIdxNext = nIdx;
	nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext ); // 2005-09-02 D.S.Koba GetSizeOfChar
	while( nCharChars > 0 ){
		nIdxNext += nCharChars;
		ECharKind	nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );

		ECharKind nCharKindMerge = WhatKindOfTwoChars( nCharKindNext, nCharKind );
		if( nCharKindMerge == CK_NULL ){
			break;
		}
		nCharKind = nCharKindMerge;
		nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext ); // 2005-09-02 D.S.Koba GetSizeOfChar
	}
	*pnIdxTo = nIdxNext;

	if( NULL != pcmcmWord ){
		pcmcmWord->SetString( &pLine[*pnIdxFrom], *pnIdxTo - *pnIdxFrom );
	}
	return true;
}



//! 識別子に使用可能な文字かどうか
inline bool isCSymbol(wchar_t c)
{
	//return
	//	(c==L'_') ||
	//	(c>=L'0' && c<=L'9') ||
	//	(c>=L'A' && c<=L'Z') ||
	//	(c>=L'a' && c<=L'z');
	return (c<_countof(gm_keyword_char) && gm_keyword_char[c]==CK_CSYM);
}

//! 全角版、識別子に使用可能な文字かどうか
inline bool isCSymbolZen(wchar_t c)
{
	return
		(c==L'＿') ||
		(c>=L'０' && c<=L'９') ||
		(c>=L'Ａ' && c<=L'Ｚ') ||
		(c>=L'ａ' && c<=L'ｚ');
}



//! 現在位置の文字の種類を調べる
ECharKind CWordParse::WhatKindOfChar(
	const wchar_t*	pData,
	int				pDataLen,
	int				nIdx
)
{
	using namespace WCODE;

	int nCharChars = CNativeW::GetSizeOfChar( pData, pDataLen, nIdx );
	if( nCharChars == 0 ){
		return CK_NULL;	// NULL
	}
	else if( nCharChars == 1 ){
		wchar_t c=pData[nIdx];

		//今までの半角
		if( c<_countof(gm_keyword_char) ) return (ECharKind)gm_keyword_char[c];
		//if( c == CR              )return CK_CR;
		//if( c == LF              )return CK_LF;
		//if( c == TAB             )return CK_TAB;	// タブ
		//if( IsControlCode(c)     )return CK_CTRL;	// 制御文字
		//if( c == SPACE           )return CK_SPACE;	// 半角スペース
		//if( isCSymbol(c)         )return CK_CSYM;	// 識別子に使用可能な文字 (半角英数字、半角アンダースコア)
		if( IsHankakuKatakana(c) )return CK_KATA;	// 半角のカタカナ
		if( 0x00C0 <= c && c < 0x0180 && c != 0x00D7 && c != 0x00F7 )return CK_LATIN;
													// ラテン１補助、ラテン拡張のうちアルファベット風のもの（×÷を除く）
		//if( c == L'#'|| c == L'$' || c == L'@'|| c == L'\\' )return CK_UDEF;	// ユーザ定義

		//その他
		if( IsZenkakuSpace(c)    )return CK_ZEN_SPACE;	// 全角スペース
		if( c==L'ー'             )return CK_ZEN_NOBASU;	// 伸ばす記号 'ー'
		if( c==L'゛' || c==L'゜' )return CK_ZEN_DAKU;	// 全角濁点 「゛゜」
		if( isCSymbolZen(c)      )return CK_ZEN_CSYM;	// 全角版、識別子に使用可能な文字 
		if( IsZenkakuKigou(c)    )return CK_ZEN_KIGO;	// 全角の記号
		if( IsHiragana(c)        )return CK_HIRA;		// ひらがな
		if( IsZenkakuKatakana(c) )return CK_ZEN_KATA;	// 全角カタカナ
		if( IsGreek(c)           )return CK_GREEK;		// ギリシャ文字
		if( IsCyrillic(c)        )return CK_ZEN_ROS;	// ロシア文字
		if( IsBoxDrawing(c)      )return CK_ZEN_SKIGO;	// 全角の特殊記号

		//未分類
		if( IsHankaku(c) )return CK_ETC;	// 半角のその他
		else return CK_ZEN_ETC;				// 全角のその他(漢字など)
	}
	else if( nCharChars == 2 ){
		// サロゲートペア 2008/7/8 Uchi
		if (IsUTF16High(pData[nIdx]) && IsUTF16Low(pData[nIdx+1])) {
			int		nCode = 0x10000 + ((pData[nIdx] & 0x3FF)<<10) + (pData[nIdx+1] & 0x3FF);	// コードポイント
			if (nCode >= 0x20000 && nCode <= 0x2FFFF) {	// CJKV 拡張予約域 Ext-B/Ext-C...
				return CK_ZEN_ETC;				// 全角のその他(漢字など)
			}
		}
		return CK_ETC;	// 半角のその他
	}
	else{
		return CK_NULL;	// NULL
	}
}



//! 二つの文字を結合したものの種類を調べる
ECharKind CWordParse::WhatKindOfTwoChars( ECharKind kindPre, ECharKind kindCur )
{
	if( kindPre == kindCur )return kindCur;			// 同種ならその種別を返す

	// 全角長音・全角濁点は前後の全角ひらがな・全角カタカナに引きずられる
	if( ( kindPre == CK_ZEN_NOBASU || kindPre == CK_ZEN_DAKU ) &&
		( kindCur == CK_ZEN_KATA   || kindCur == CK_HIRA     ) )return kindCur;
	if( ( kindCur == CK_ZEN_NOBASU || kindCur == CK_ZEN_DAKU ) &&
		( kindPre == CK_ZEN_KATA   || kindPre == CK_HIRA     ) )return kindPre;
	// 全角濁点、全角長音の連続は、とりあえず同種の文字とみなす
	if( ( kindPre == CK_ZEN_NOBASU || kindPre == CK_ZEN_DAKU ) &&
		( kindCur == CK_ZEN_NOBASU || kindCur == CK_ZEN_DAKU ) )return kindCur;

	if( kindPre == CK_LATIN )kindPre = CK_CSYM;		// ラテン系文字はアルファベットとみなす
	if( kindCur == CK_LATIN )kindCur = CK_CSYM;
	if( kindPre == CK_UDEF )kindPre = CK_ETC;		// ユーザ定義文字はその他の半角とみなす
	if( kindCur == CK_UDEF )kindCur = CK_ETC;
	if( kindPre == CK_CTRL )kindPre = CK_ETC;		// 制御文字はその他の半角とみなす
	if( kindCur == CK_CTRL )kindCur = CK_ETC;

	if( kindPre == kindCur )return kindCur;			// 同種ならその種別を返す

	return CK_NULL;									// それ以外なら二つの文字は別種
}


//! 二つの文字を結合したものの種類を調べる
ECharKind CWordParse::WhatKindOfTwoChars4KW( ECharKind kindPre, ECharKind kindCur )
{
	if( kindPre == kindCur )return kindCur;			// 同種ならその種別を返す

	// 全角長音・全角濁点は前後の全角ひらがな・全角カタカナに引きずられる
	if( ( kindPre == CK_ZEN_NOBASU || kindPre == CK_ZEN_DAKU ) &&
		( kindCur == CK_ZEN_KATA   || kindCur == CK_HIRA     ) )return kindCur;
	if( ( kindCur == CK_ZEN_NOBASU || kindCur == CK_ZEN_DAKU ) &&
		( kindPre == CK_ZEN_KATA   || kindPre == CK_HIRA     ) )return kindPre;
	// 全角濁点、全角長音の連続は、とりあえず同種の文字とみなす
	if( ( kindPre == CK_ZEN_NOBASU || kindPre == CK_ZEN_DAKU ) &&
		( kindCur == CK_ZEN_NOBASU || kindCur == CK_ZEN_DAKU ) )return kindCur;

	if( kindPre == CK_LATIN )kindPre = CK_CSYM;		// ラテン系文字はアルファベットとみなす
	if( kindCur == CK_LATIN )kindCur = CK_CSYM;
	if( kindPre == CK_UDEF )kindPre = CK_CSYM;		// ユーザ定義文字はアルファベットとみなす
	if( kindCur == CK_UDEF )kindCur = CK_CSYM;
	if( kindPre == CK_CTRL )kindPre = CK_CTRL;		// 制御文字はそのまま制御文字とみなす
	if( kindCur == CK_CTRL )kindCur = CK_CTRL;

	if( kindPre == kindCur )return kindCur;			// 同種ならその種別を返す

	return CK_NULL;									// それ以外なら二つの文字は別種
}


/*!	次の単語の先頭を探す
	pLine（長さ：nLineLen）の文字列から単語を探す。
	探し始める位置はnIdxで指定。方向は後方に限定。単語の両端で止まらない（関係ないから）
*/
bool CWordParse::SearchNextWordPosition(
	const wchar_t*	pLine,
	CLogicInt		nLineLen,
	CLogicInt		nIdx,		//	桁数
	CLogicInt*		pnColumnNew,	//	見つかった位置
	BOOL			bStopsBothEnds	//	単語の両端で止まる
)
{
	// 文字種類が変わるまで後方へサーチ
	// 空白とタブは無視する

	// 現在位置の文字の種類を調べる
	ECharKind nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );

	CLogicInt nIdxNext = nIdx;
	// 2005-09-02 D.S.Koba GetSizeOfChar
	CLogicInt nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext );
	while( nCharChars > 0 ){
		nIdxNext += nCharChars;
		ECharKind nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
		// 空白とタブは無視する
		if( nCharKindNext == CK_TAB || nCharKindNext == CK_SPACE ){
			if ( bStopsBothEnds && nCharKind != nCharKindNext ){
				*pnColumnNew = nIdxNext;
				return true;
			}
			nCharKind = nCharKindNext;
		}
		else {
			ECharKind nCharKindMerge = WhatKindOfTwoChars( nCharKind, nCharKindNext );
			if( nCharKindMerge == CK_NULL ){
				*pnColumnNew = nIdxNext;
				return true;
			}
			nCharKind = nCharKindMerge;
		}
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext );
	}
	return false;
}


/*!	次の単語の先頭を探す
	pLine（長さ：nLineLen）の文字列から単語を探す。
	探し始める位置はnIdxで指定。方向は後方に限定。単語の両端で止まらない（関係ないから）
*/
bool CWordParse::SearchNextWordPosition4KW(
	const wchar_t*	pLine,
	CLogicInt		nLineLen,
	CLogicInt		nIdx,		//	桁数
	CLogicInt*		pnColumnNew,	//	見つかった位置
	BOOL			bStopsBothEnds	//	単語の両端で止まる
)
{
	// 文字種類が変わるまで後方へサーチ
	// 空白とタブは無視する

	// 現在位置の文字の種類を調べる
	ECharKind nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );

	CLogicInt nIdxNext = nIdx;
	// 2005-09-02 D.S.Koba GetSizeOfChar
	CLogicInt nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext );
	while( nCharChars > 0 ){
		nIdxNext += nCharChars;
		ECharKind nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
		// 空白とタブは無視する
		if( nCharKindNext == CK_TAB || nCharKindNext == CK_SPACE ){
			if ( bStopsBothEnds && nCharKind != nCharKindNext ){
				*pnColumnNew = nIdxNext;
				return true;
			}
			nCharKind = nCharKindNext;
		}
		else {
			ECharKind nCharKindMerge = WhatKindOfTwoChars4KW( nCharKind, nCharKindNext );
			if( nCharKindMerge == CK_NULL ){
				*pnColumnNew = nIdxNext;
				return true;
			}
			nCharKind = nCharKindMerge;
		}
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext );
	}
	return false;
}


//! wcがasciiなら0-127のまま返す。それ以外は0を返す。
uchar_t wc_to_c(wchar_t wc)
{
#if 0
//! wcがSJIS1バイト文字ならcharに変換して0～255を返す。SJIS2バイト文字なら0を返す。
	char buf[3]={0,0,0};
	int ret=wctomb(buf,wc);
	if(ret==-1)return 0;   //エラー
	if(buf[1]!=0)return 0; //エラー扱い
	return buf[0] <= 0x7F ? buf[0]: 0; //1バイトで表せたので、これを返す  2011.12.17 バッファオーバーランの修正
#endif
	// 2011.12.15 wctombを使わない版
	if(wc <= 0x7F){
		return (uchar_t)wc;
	}
	return 0;
}

//@@@ 2002.01.24 Start by MIK
/*!
	文字列がURLかどうかを検査する。
	
	@retval TRUE URLである
	@retval FALSE URLでない
	
	@note 関数内に定義したテーブルは必ず static const 宣言にすること(性能に影響します)。
		url_char の値は url_table の配列番号+1 になっています。
		新しい URL を追加する場合は #define 値を修正してください。
		url_table は頭文字がアルファベット順になるように並べてください。

	2007.10.23 kobake UNICODE対応。//$ wchar_t専用のテーブル(または判定ルーチン)を用意したほうが効率は上がるはずです。
*/
BOOL IsURL(
	const wchar_t*	pszLine,	//!< [in]  文字列
	int				nLineLen,	//!< [in]  文字列の長さ
	int*			pnMatchLen	//!< [out] URLの長さ
)
{
	struct _url_table_t {
		wchar_t	name[12];
		int		length;
		bool	is_mail;
	};
	static const struct _url_table_t	url_table[] = {
		/* アルファベット順 */
		{ L"file://",		7,	false }, /* 1 */
		{ L"ftp://",		6,	false }, /* 2 */
		{ L"gopher://",		9,	false }, /* 3 */
		{ L"http://",		7,	false }, /* 4 */
		{ L"https://",		8,	false }, /* 5 */
		{ L"mailto:",		7,	true  }, /* 6 */
		{ L"news:",			5,	false }, /* 7 */
		{ L"nntp://",		7,	false }, /* 8 */
		{ L"prospero://",	11,	false }, /* 9 */
		{ L"telnet://",		9,	false }, /* 10 */
		{ L"tp://",			5,	false }, /* 11 */	//2004.02.02
		{ L"ttp://",		6,	false }, /* 12 */	//2004.02.02
		{ L"wais://",		7,	false }, /* 13 */
		{ L"{",				0,	false }  /* 14 */  /* '{' is 'z'+1 : terminate */
	};

/* テーブルの保守性を高めるための定義 */
	const char urF = 1;
	const char urG = 3;
	const char urH = 4;
	const char urM = 6;
	const char urN = 7;
	const char urP = 9;
	const char urT = 10;
	const char urW = 13;	//2004.02.02

	static const char	url_char[] = {
	  /* +0  +1  +2  +3  +4  +5  +6  +7  +8  +9  +A  +B  +C  +D  +E  +F */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +00: */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +10: */
		  0, -1,  0, -1, -1, -1, -1,  0,  0,  0,  0, -1, -1, -1, -1, -1,	/* +20: " !"#$%&'()*+,-./" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0, -1,	/* +30: "0123456789:;<=>?" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* +40: "@ABCDEFGHIJKLMNO" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0,  0, -1,	/* +50: "PQRSTUVWXYZ[\]^_" */
		  0, -1, -1, -1, -1, -1,urF,urG,urH, -1, -1, -1, -1,urM,urN, -1,	/* +60: "`abcdefghijklmno" */
		urP, -1, -1, -1,urT, -1, -1,urW, -1, -1, -1,  0,  0,  0, -1,  0,	/* +70: "pqrstuvwxyz{|}~ " */
		/* あと128バイト犠牲にすればif文を2箇所削除できる */
		/* 0    : not url char
		 * -1   : url char
		 * other: url head char --> url_table array number + 1
		 */
	};

	const wchar_t *p = pszLine;
	const struct _url_table_t	*urlp;
	int	i;

	if( wc_to_c(*p)==0 ) return FALSE;	/* 2バイト文字 */
	if( 0 < url_char[wc_to_c(*p)] ){	/* URL開始文字 */
		for(urlp = &url_table[url_char[wc_to_c(*p)]-1]; urlp->name[0] == wc_to_c(*p); urlp++){	/* URLテーブルを探索 */
			if( (urlp->length <= nLineLen) && (auto_memcmp(urlp->name, pszLine, urlp->length) == 0) ){	/* URLヘッダは一致した */
				p += urlp->length;	/* URLヘッダ分をスキップする */
				if( urlp->is_mail ){	/* メール専用の解析へ */
					if( IsMailAddress(p, nLineLen - urlp->length, pnMatchLen) ){
						*pnMatchLen = *pnMatchLen + urlp->length;
						return TRUE;
					}
					return FALSE;
				}
				for(i = urlp->length; i < nLineLen; i++, p++){	/* 通常の解析へ */
					if( wc_to_c(*p)==0 || (!(url_char[wc_to_c(*p)])) ) break;	/* 終端に達した */
				}
				if( i == urlp->length ) return FALSE;	/* URLヘッダだけ */
				*pnMatchLen = i;
				return TRUE;
			}
		}
	}
	return IsMailAddress(pszLine, nLineLen, pnMatchLen);
}

/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す
	@date 2016.04.27 記号類を許可
*/
BOOL IsMailAddress( const wchar_t* pszBuf, int nBufLen, int* pnAddressLenfth )
{
	int		j;
	int		nDotCount;
	int		nBgn;


	j = 0;
	if( (pszBuf[j] >= L'a' && pszBuf[j] <= L'z')
	 || (pszBuf[j] >= L'A' && pszBuf[j] <= L'Z')
	 || (pszBuf[j] >= L'0' && pszBuf[j] <= L'9')
	 || NULL != wcschr(L"!#$%&'*+-/=?^_`{|}~", pszBuf[j])
	){
		j++;
	}else{
		return FALSE;
	}
	while( j < nBufLen - 2 &&
		(
		(pszBuf[j] >= L'a' && pszBuf[j] <= L'z')
	 || (pszBuf[j] >= L'A' && pszBuf[j] <= L'Z')
	 || (pszBuf[j] >= L'0' && pszBuf[j] <= L'9')
	 || (pszBuf[j] == L'.')
	 || NULL != wcschr(L"!#$%&'*+-/=?^_`{|}~", pszBuf[j])
		)
	){
		j++;
	}
	if( j == 0 || j >= nBufLen - 2  ){
		return FALSE;
	}
	if( L'@' != pszBuf[j] ){
		return FALSE;
	}
//	nAtPos = j;
	j++;
	nDotCount = 0;
//	nAlphaCount = 0;


	for (;;) {
		nBgn = j;
		while( j < nBufLen &&
			(
			(pszBuf[j] >= L'a' && pszBuf[j] <= L'z')
		 || (pszBuf[j] >= L'A' && pszBuf[j] <= L'Z')
		 || (pszBuf[j] >= L'0' && pszBuf[j] <= L'9')
		 || (pszBuf[j] == L'-')
		 || (pszBuf[j] == L'_')
			)
		){
			j++;
		}
		if( 0 == j - nBgn ){
			return FALSE;
		}
		if( L'.' != pszBuf[j] ){
			if( 0 == nDotCount ){
				return FALSE;
			}else{
				break;
			}
		}else{
			nDotCount++;
			j++;
		}
	}
	if( NULL != pnAddressLenfth ){
		*pnAddressLenfth = j;
	}
	return TRUE;
}
