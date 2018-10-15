/*! @file */
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
