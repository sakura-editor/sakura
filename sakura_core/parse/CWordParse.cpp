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

// 指定された文字列がメールアドレス前半部分の要件を満たすか判定する
inline static bool IsMailAddressLocalPart(
	_In_z_ const wchar_t* pszStart,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszAtmark
) noexcept;

// 指定された文字列がメールアドレス前半部分の要件を満たすか判定する
inline static bool IsMailAddressLocalPart(
	_In_z_ const wchar_t* pszStart,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszAtmark
) noexcept;

// 指定された文字列がメールアドレス後半部分の要件を満たすか判定する
inline static bool IsMailAddressDomain(
	_In_z_ const wchar_t* pszAtmark,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszEndOfMailBox
) noexcept;

/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す
	@date 2016.04.27 記号類を許可
	@date 2018.09.09 RFC準拠
*/
BOOL IsMailAddress( const wchar_t* pszBuf, int nBufLen, int* pnAddressLenfth )
{
	// RFC5321による mailbox の最大文字数
	const ptrdiff_t MAX_MAILBOX = 255; //255オクテット

	// バカ避け
	if (nBufLen < 1) return FALSE;

	// メールアドレスには必ず＠が含まれる
	const wchar_t* pszAtmark;

	// メールアドレス前半部分(＠の手前)をチェックする
	if (!IsMailAddressLocalPart(pszBuf, pszBuf + nBufLen, &pszAtmark)) {
		return FALSE;
	}
	assert(L'@' == *pszAtmark);

	// メールアドレスの終了位置を受け取るポインタを宣言する
	const wchar_t* pszEndOfMailBox;

	// メールアドレス後半部分(＠の後ろ)をチェックする
	if (!IsMailAddressDomain(pszAtmark, pszBuf + nBufLen, &pszEndOfMailBox))
	{
		return FALSE;
	}

	// 全体の長さが制限を超えていないかチェックする
	if (MAX_MAILBOX < pszEndOfMailBox - pszBuf)
	{
		return FALSE; // 文字数オーバー
	}

	if (pnAddressLenfth != nullptr)
	{
		*pnAddressLenfth = pszEndOfMailBox - pszBuf;
	}
	return TRUE;
}

/*!
 * 指定された文字列がメールアドレス前半部分の要件を満たすか判定する
 *
 * 高速化のため単純化した条件でチェックしている
 * 参照する標準は RFC5321
 * @see http://srgia.com/docs/rfc5321j.html
 */
inline static bool IsMailAddressLocalPart(
	_In_z_ const wchar_t* pszStart,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszAtmark
) noexcept
{
	// RFC5321による local-part の最大文字数
	const ptrdiff_t MAX_LOCAL_PART = 64; //64オクテット

	// 関数仕様
	assert(pszStart != pszEnd); // 長さ0の文字列をチェックしてはならない
	assert(pszStart < pszEnd); // 開始位置と終了位置は逆転してはならない

	// 出力値を初期化する
	*ppszAtmark = nullptr;

	// 文字列が二重引用符で始まっているかチェックして結果を保存
	const bool quoted = (L'"' == *pszStart);

	// ループ中にスキャンする文字位置を設定する
	auto pszScan = pszStart + (quoted ? 1 : 0);

	// スキャン位置が終端に達するまでループ
	while (pszScan < pszEnd)
	{
		switch (*pszScan)
		{
		case L'@':
			if (pszStart == pszScan)
			{
				return false; // local-partは1文字以上なのでNG
			}
			if (quoted)
			{
				return false; // 二重引用符で始まる場合、終端にも二重引用符が必要なのでNG
			}
			*ppszAtmark = pszScan;
			return true; // ここが正常終了
		case L'\\': // エスケープ記号
			if (pszScan + 1 == pszEnd || pszScan[1] < L'\x20' || L'\x7E' < pszScan[1])
			{
				return false;
			}
			pszScan++; // エスケープ記号の分1文字進める
			break;
		case L'"': // 二重引用符
			if (quoted && pszScan + 1 < pszEnd && L'@' == pszScan[1])
			{
				*ppszAtmark = &pszScan[1];
				return true; // ここは準正常終了。正常終了とはあえて区別しない。
			}
			return false; // 末尾以外に現れるエスケープされてない二重引用符は不正
		}
		pszScan++;
		if (MAX_LOCAL_PART < pszScan - pszStart)
		{
			return false; // 文字数オーバー
		}
	}
	return false;
}

/*!
 * 指定された文字列がメールアドレス後半部分の要件を満たすか判定する
 */
inline static bool IsMailAddressDomain(
	_In_z_ const wchar_t* pszAtmark,
	_In_ const wchar_t* pszEnd,
	_Out_ const wchar_t** ppszEndOfMailBox
) noexcept
{
	// ccTLDの最小文字数
	const ptrdiff_t MIN_TLD = 2;

	// ドメインの最小文字数
	const ptrdiff_t MIN_DOMAIN = 3;

	// ドメインの最大文字数
	const ptrdiff_t MAX_DOMAIN = 63;

	// 関数仕様
	assert(pszAtmark + 1 < pszEnd); // @位置と終了位置は逆転してはならない
	assert(L'@' == *pszAtmark); // @位置にある文字は@でなければならない

	// 出力値を初期化する
	*ppszEndOfMailBox = nullptr;

	// ループ中にスキャンする文字位置を設定する
	auto pszScan = pszAtmark + 1;

	auto dotCount = 0;
	auto domainLength = 0;
	auto prevHyphen = false;

	// スキャン位置が終端に達するまでループ
	while (pszScan < pszEnd)
	{
		switch (*pszScan)
		{
		case L'.': // ドット記号
			if (dotCount == 0 && domainLength < MIN_DOMAIN)
			{
				return false; // ドメイン名の最小文字数は3なのでNG
			}
			if (0 < dotCount && domainLength < MIN_TLD)
			{
				// これはco.jpなどを正しく認識させるために必要。
				return false; // ドットで区切られる部分の最小文字数は2なのでNG
			}
			if (prevHyphen)
			{
				return false; // ハイフンに続くドットはNG
			}
			dotCount++;
			domainLength = 0;
			prevHyphen = false;
			break;
		case L'-': // ハイフン記号
			if (domainLength == 0)
			{
				return false; // ドットに続くハイフンはNG
			}
			if (prevHyphen)
			{
				return false; // 連続するハイフンはNG
			}
			domainLength++;
			prevHyphen = true;
			break;
		default:
			if (dotCount == 0)
			{
				return false; // ドメイン部には一つ以上のドット記号が必要なのでNG
			}
			if (domainLength == 0)
			{
				return false; // ドットで終わるドメインはNG
			}
			if (prevHyphen)
			{
				return false; // ハイフンで終わるドメインはNG
			}
			*ppszEndOfMailBox = pszScan;
			return true; // ここも正常終了
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
		case L'G':
		case L'H':
		case L'I':
		case L'J':
		case L'K':
		case L'L':
		case L'M':
		case L'N':
		case L'O':
		case L'P':
		case L'Q':
		case L'R':
		case L'S':
		case L'T':
		case L'U':
		case L'V':
		case L'W':
		case L'X':
		case L'Y':
		case L'Z':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'g':
		case L'h':
		case L'i':
		case L'j':
		case L'k':
		case L'l':
		case L'm':
		case L'n':
		case L'o':
		case L'p':
		case L'q':
		case L'r':
		case L's':
		case L't':
		case L'u':
		case L'v':
		case L'w':
		case L'x':
		case L'y':
		case L'z':
			domainLength++;
			prevHyphen = false;
			break;
		}
		pszScan++;
		if (pszScan == pszEnd)
		{
			*ppszEndOfMailBox = pszScan;
			return true; // ここが正常終了
		}
		if (MAX_DOMAIN < domainLength)
		{
			return false; // 文字数オーバー
		}
	}
	return false;
}
