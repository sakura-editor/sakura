//2007.09.30 kobake CDocLineMgr から分離
/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CWORDPARSE_6C07A5B6_49DE_430F_A56F_9F2B003214FC9_H_
#define SAKURA_CWORDPARSE_6C07A5B6_49DE_430F_A56F_9F2B003214FC9_H_

#include "basis/SakuraBasis.h"
class CNativeW;

//! 文字種類識別子
enum ECharKind{
	CK_NULL,			//!< NULL
	CK_TAB,				//!< タブ 0x9<=c<=0x9
	CK_CR,				//!< CR = 0x0d 
	CK_LF,				//!< LF = 0x0a 
	CK_CTRL,			//!< 上記以外の c<0x20

	CK_SPACE,			//!< 半角のスペース 0x20<=c<=0x20
	CK_CSYM,			//!< 識別子に使用可能な文字 (英数字、アンダースコア)
	CK_KATA,			//!< 半角のカタカナ 0xA1<=c<=0xFD
	CK_LATIN,			//!< ラテン１補助、ラテン拡張のうちアルファベット風のもの 0x00C0<=c<0x0180
	CK_UDEF,			//!< ユーザ定義キーワード文字（#$@\）
	CK_ETC,				//!< 半角のその他

	CK_ZEN_SPACE,		//!< 全角スペース
	CK_ZEN_NOBASU,		//!< 伸ばす記号 0x815B<=c<=0x815B 'ー'
	CK_ZEN_DAKU,		//!< 全角濁点 0x309B<=c<=0x309C 「゛゜」
	CK_ZEN_CSYM,		//!< 全角版、識別子に使用可能な文字 (英数字、アンダースコア)

	CK_ZEN_KIGO,		//!< 全角の記号
	CK_HIRA,			//!< ひらがな
	CK_ZEN_KATA,		//!< 全角カタカナ
	CK_GREEK,			//!< ギリシャ文字
	CK_ZEN_ROS,			//!< ロシア文字:
	CK_ZEN_SKIGO,		//!< 全角の特殊記号
	CK_ZEN_ETC,			//!< 全角のその他（漢字など）
};

class CWordParse{
public:
	//2001.06.23 N.Nakatani
	//2007.09.30 kobake     CDocLineMgrから移動
	/*!
		@brief 現在位置の単語の範囲を調べる staticメンバ
		@author N.Nakatani
		@retval true	成功 現在位置のデータは「単語」と認識する。
		@retval false	失敗 現在位置のデータは「単語」とは言いきれない気がする。
	*/
	static bool WhereCurrentWord_2(
		const wchar_t*	pLine,			//[in]  調べるメモリ全体の先頭アドレス
		CLogicInt		nLineLen,		//[in]  調べるメモリ全体の有効長
		CLogicInt		nIdx,			//[out] 調査開始地点:pLineからの相対的な位置
		CLogicInt*		pnIdxFrom,		//[out] 単語が見つかった場合は、単語の先頭インデックスを返す。
		CLogicInt*		pnIdxTo,		//[out] 単語が見つかった場合は、単語の終端の次のバイトの先頭インデックスを返す。
		CNativeW*		pcmcmWord,		//[out] 単語が見つかった場合は、現在単語を切り出して指定されたCMemoryオブジェクトに格納する。情報が不要な場合はNULLを指定する。
		CNativeW*		pcmcmWordLeft	//[out] 単語が見つかった場合は、現在単語の左に位置する単語を切り出して指定されたCMemoryオブジェクトに格納する。情報が不要な場合はNULLを指定する。
	);

	//! 現在位置の文字の種類を調べる
	static ECharKind WhatKindOfChar(
		const wchar_t*	pData,
		int				pDataLen,
		int				nIdx
	);

	//! 二つの文字を結合したものの種類を調べる
	static ECharKind WhatKindOfTwoChars(
		ECharKind		kindPre,
		ECharKind		kindCur
	);

	//! 二つの文字を結合したものの種類を調べる for 強調キーワード
	static ECharKind WhatKindOfTwoChars4KW(
		ECharKind		kindPre,
		ECharKind		kindCur
	);

	//	pLine（長さ：nLineLen）の文字列から次の単語を探す。探し始める位置はnIdxで指定。
	static bool SearchNextWordPosition(
		const wchar_t*	pLine,
		CLogicInt		nLineLen,
		CLogicInt		nIdx,		//	桁数
		CLogicInt*		pnColumnNew,	//	見つかった位置
		BOOL			bStopsBothEnds	//	単語の両端で止まる
	);

	//	pLine（長さ：nLineLen）の文字列から次の単語を探す。探し始める位置はnIdxで指定。 for 強調キーワード
	static bool SearchNextWordPosition4KW(
		const wchar_t*	pLine,
		CLogicInt		nLineLen,
		CLogicInt		nIdx,		//	桁数
		CLogicInt*		pnColumnNew,	//	見つかった位置
		BOOL			bStopsBothEnds	//	単語の両端で止まる
	);


	template< class CHAR_TYPE >
	static int GetWord( const CHAR_TYPE*, const int, const CHAR_TYPE *pszSplitCharList,
		CHAR_TYPE **ppWordStart, int *pnWordLen );

protected:

	static bool _match_charlist( const ACHAR c, const ACHAR *pszList );
	static bool _match_charlist( const WCHAR c, const WCHAR *pszList );
};

BOOL IsURL( const wchar_t*, int, int* );/* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
BOOL IsMailAddress( const wchar_t*, int, int* );	/* 現在位置がメールアドレスならば、NULL以外と、その長さを返す */



// ACHAR 版
inline bool CWordParse::_match_charlist( const ACHAR c, const ACHAR *pszList )
{
	for( int i = 0; pszList[i] != '\0'; i++ ){
		if( pszList[i] == c ){ return true; }
	}
	return false;
}
// WCHAR 版
inline bool CWordParse::_match_charlist( const WCHAR c, const WCHAR *pszList )
{
	for( int i = 0; pszList[i] != L'\0'; i++ ){
		if( pszList[i] == c ){ return true; }
	}
	return false;
}

/*!
	@param [in] pS					文字列バッファ
	@param [in] nLen				文字列バッファの長さ
	@param [in] pszSplitCharList	区切り文字たち
	@param [out] ppWordStart		単語の開始位置
	@param [out] pnWordLen			単語の長さ

	@return 読んだデータの長さ。
*/
template< class CHAR_TYPE >
int CWordParse::GetWord( const CHAR_TYPE *pS, const int nLen, const CHAR_TYPE *pszSplitCharList,
	CHAR_TYPE **ppWordStart, int *pnWordLen )
{
	const CHAR_TYPE *pr = pS;
	CHAR_TYPE *pwordstart;
	int nwordlen;

	if( nLen < 1 ){
		pwordstart = const_cast<CHAR_TYPE *>(pS);
		nwordlen = 0;
		goto end_func;
	}

	// 区切り文字をスキップ
	for( ; pr < pS + nLen; pr++ ){
		// 区切り文字でない文字の間ループ
		if( !_match_charlist(*pr, pszSplitCharList) ){
			break;
		}
	}
	pwordstart = const_cast<CHAR_TYPE*>(pr);   // 単語の先頭位置を記録

	// 単語をスキップ
	for( ; pr < pS + nLen; pr++ ){
		// 区切り文字がくるまでループ
		if( _match_charlist(*pr, pszSplitCharList) ){
			break;
		}
	}
	nwordlen = pr - pwordstart;  // 単語の長さを記録

end_func:
	if( ppWordStart ){
		*ppWordStart = pwordstart;
	}
	if( pnWordLen ){
		*pnWordLen = nwordlen;
	}
	return pr - pS;
}

#endif /* SAKURA_CWORDPARSE_6C07A5B6_49DE_430F_A56F_9F2B003214FC9_H_ */
/*[EOF]*/
