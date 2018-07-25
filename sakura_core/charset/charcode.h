/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_CHARCODE_5A887F7C_8E08_4940_AF65_BD6850C3A7B5_H_
#define SAKURA_CHARCODE_5A887F7C_8E08_4940_AF65_BD6850C3A7B5_H_

//2007.09.13 kobake 作成
#include "parse/CWordParse.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         判定関数                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// #include "charset/codechecker.h"
// SJIS関連コードは codecheker.hに移動

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           定数                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// SJISのコードページ(CP_ACP では無くこれを使えばおそらく英語版Winでも動くはず。)	2008/5/12 Uchi
#define CP_SJIS		932


//定数の素 (直接使用は控えてください)
#define TAB_ 				'\t'
#define SPACE_				' '
#define CR_					'\015'
#define LF_					'\012'
#define ESC_				'\x1b'
#define CRLF_				"\015\012"

//ANSI定数
namespace ACODE{
	//文字
	static const char TAB   = TAB_;
	static const char SPACE = SPACE_;
	static const char CR	= CR_;
	static const char LF	= LF_;
	static const char ESC	= ESC_;

	//文字列
	static const char CRLF[] = CRLF_;

	//特殊 (BREGEXP)
	static const wchar_t BREGEXP_DELIMITER = (wchar_t)0xFF;
}

//UNICODE定数
namespace WCODE{
	//文字
	static const wchar_t TAB   = LCHAR(TAB_);
	static const wchar_t SPACE = LCHAR(SPACE_);
	static const wchar_t CR    = LCHAR(CR_);
	static const wchar_t LF    = LCHAR(LF_);
	static const wchar_t ESC   = LCHAR(ESC_);

	//文字列
	static const wchar_t CRLF[] = LTEXT(CRLF_);

	//特殊 (BREGEXP)
	//$$ UNICODE版の仮デリミタ。bregonigの仕様がよくわかんないので、とりあえずこんな値にしてます。
	static const wchar_t BREGEXP_DELIMITER = (wchar_t)0xFFFF;

}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         判定関数                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*! キーワードキャラクタ */
extern const unsigned char gm_keyword_char[128];

//Oct. 31, 2000 JEPRO  TeX Keyword のために'\'を追加
//Nov.  9, 2000 JEPRO  HSP Keyword のために'@'を追加
//Oct. 18, 2007 kobake UNICODE用に書き直し
//Nov. 27, 2010 syat   速度改善のためテーブルに変更
inline bool IS_KEYWORD_CHAR(wchar_t wc)
{
	if(0 <= wc && wc < _countof(gm_keyword_char) && (gm_keyword_char[wc] == CK_CSYM||gm_keyword_char[wc] == CK_UDEF) )
		return true;
	else
		return false;
}


//UNICODE判定関数群
namespace WCODE
{
	inline bool IsAZ(wchar_t wc)
	{
		return (wc>=L'A' && wc<=L'Z') || (wc>=L'a' && wc<=L'z');
	}
	inline bool Is09(wchar_t wc)
	{
		return (wc>=L'0' && wc<=L'9');
	}
	inline bool IsInRange(wchar_t c, wchar_t front, wchar_t back)
	{
		return c>=front && c<=back;
	}

	//!半角文字(縦長長方形)かどうか判定
	bool IsHankaku(wchar_t wc);

	//!全角文字(正方形)かどうか判定
	inline bool IsZenkaku(wchar_t wc)
	{
		return !IsHankaku(wc);
	}

	//!使用フォント番号を返す
	// (0:半角/1:全角)
	int GetFontNo(wchar_t);
	int GetFontNo2(wchar_t, wchar_t);

	//!全角スペースかどうか判定
	inline bool IsZenkakuSpace(wchar_t wc)
	{
		return wc == 0x3000; //L'　'
	}

	//!制御文字であるかどうか
	bool IsControlCode(wchar_t wc);

	//!改行文字であるかどうか
	inline bool IsLineDelimiter(wchar_t wc, bool ext)
	{
		return wc==CR || wc==LF || (ext && (wc==0x85 || wc==0x2028 || wc==0x2029));
	}
	inline bool IsLineDelimiterBasic(wchar_t wc)
	{
		return wc==CR || wc==LF;
	}
	inline bool IsLineDelimiterExt(wchar_t wc)
	{
		return wc==CR || wc==LF || wc==0x85 || wc==0x2028 || wc==0x2029;
	}

	//!単語の区切り文字であるかどうか
	inline bool IsWordDelimiter(wchar_t wc)
	{
		return wc==SPACE || wc==TAB || IsZenkakuSpace(wc);
	}

	//!インデント構成要素であるかどうか。bAcceptZenSpace: 全角スペースを含めるかどうか
	inline bool IsIndentChar(wchar_t wc,bool bAcceptZenSpace)
	{
		if(wc==TAB || wc==SPACE)return true;
		if(bAcceptZenSpace && IsZenkakuSpace(wc))return true;
		return false;
	}

	//!空白かどうか
	inline bool IsBlank(wchar_t wc)
	{
		return wc==TAB || wc==SPACE || IsZenkakuSpace(wc);
	}

	//!ファイル名に使える文字であるかどうか
	inline bool IsValidFilenameChar(const wchar_t wc)
	{
		static const wchar_t* table = L"<>?\"|*";

		if(wcschr(table,wc)!=NULL)return false; //table内の文字が含まれていたら、ダメ。
		else return true;
	}

	//!タブ表示に使える文字かどうか
	inline bool IsTabAvailableCode(wchar_t wc)
	{
		//$$要検証
		if(wc==L'\0')return false;
		if(wc==L'\r')return false;
		if(wc==L'\n')return false;
		if(wc==L'\t')return false;
		return true;
	}

	//! 半角カナかどうか
	inline bool IsHankakuKatakana(wchar_t c)
	{
		//参考: http://ash.jp/code/unitbl1.htm
		return c>=0xFF61 && c<=0xFF9F;
	}

	//! 全角記号かどうか
	inline bool IsZenkakuKigou(wchar_t c)
	{
		//$ 他にも全角記号はあると思うけど、とりあえずANSI版時代の判定を踏襲。パフォーマンス悪し。
		// 2009.06.26 syat 「ゝゞ（ひらがな）」「ヽヾ（カタカナ）」「゛゜（全角濁点）」「仝々〇（漢字）」「ー（長音）」を除外
		// 2009.10.10 syat ANSI版の修正にあわせて「〆」を記号→漢字にする
		static const wchar_t* table=L"　、。，．・：；？！´｀¨＾￣＿〃―‐／＼～∥｜…‥‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭♪†‡¶◯";
		return wcschr(table,c)!=NULL;
	}

	//! ひらがなかどうか
	inline bool IsHiragana(wchar_t c)
	{
		// 2009.06.26 syat 「ゝゞ」を追加
		return (c>=0x3041 && c<=0x3096) || (c>=0x309D && c<=0x309E);
	}

	//! カタカナかどうか
	inline bool IsZenkakuKatakana(wchar_t c)
	{
		// 2009.06.26 syat 「ヽヾ」を追加
		return (c>=0x30A1 && c<=0x30FA) || (c>=0x30FD && c<=0x30FE);
	}

	//! ギリシャ文字かどうか
	inline bool IsGreek(wchar_t c)
	{
		return c>=0x0391 && c<=0x03C9;
	}

	//! キリル文字かどうか
	inline bool IsCyrillic(wchar_t c)
	{
		return (c>=0x0400 && c<=0x052F)  // Cyrillic, Cyrillic Supplement
			|| (c>=0x2DE0 && c<=0x2DFF)  // Cyrillic Extended-A
			|| (c>=0xA640 && c<=0xA69F); // Cyrillic Extended-B
	}

	//! BOX DRAWING 文字 かどうか
	inline bool IsBoxDrawing(wchar_t c)
	{
		return c>=0x2500 && c<=0x257F;
	}

	//!文字が半角かどうかを取得(DLLSHARE/フォント依存)
	bool CalcHankakuByFont(wchar_t c);
	//!文字のpx幅を取得(DLLSHARE/フォント依存)
	int  CalcPxWidthByFont(wchar_t c);
	//!文字のpx幅を取得(DLLSHARE/フォント依存)
	int  CalcPxWidthByFont2(const wchar_t* c);
	//! 句読点か
	//bool IsKutoten( wchar_t wc );

/* codechecker.h へ移動
	//! 高位サロゲートエリアか？	from ssrc_2004-06-05wchar00703b	2008/5/15 Uchi
	inline bool IsUTF16High( wchar_t c )
	{
		return ( 0xd800 == (0xfc00 & c ));
	}
	//! 下位サロゲートエリアか？	from ssrc_2004-06-05wchar00703b	2008/5/15 Uchi
	inline bool IsUTF16Low( wchar_t c )
	{
		return ( 0xdc00 == (0xfc00 & c ));
	}
*/
}


//ANSI判定関数群
namespace ACODE
{
	inline bool IsAZ(char c)
	{
		return (c>='A' && c<='Z') || (c>='a' && c<='z');
	}

	//!制御文字であるかどうか
	inline bool IsControlCode(char c)
	{
		unsigned char n=(unsigned char)c;
		if(c==TAB)return false;
		if(c==CR )return false;
		if(c==LF )return false;
		if(n<=0x1F)return true;
		if(n>=0x7F && n<=0x9F)return true;
		if(n>=0xE0)return true;
		return false;
	}

	//!タブ表示に使える文字かどうか
	inline bool IsTabAvailableCode(char c)
	{
		if(c=='\0')return false;
		if(c<=0x1f)return false;
		if(c>=0x7f)return false;
		return true;
	}

	//!ファイル名に使える文字であるかどうか
	inline bool IsValidFilenameChar(const char c)
	{
		static const char* table = "<>?\"|*";

		//table内の文字が含まれていて
		if(strchr(table,c)!=NULL){
			// 2013.06.01 判定間違いを削除
			return false;
		}

		return true;
	}
}

//TCHAR判定関数群
namespace TCODE
{
	#ifdef _UNICODE
		using namespace WCODE;
	#else
		using namespace ACODE;
	#endif
}

// 文字幅の動的計算用キャッシュ関連
struct SCharWidthCache {
	// 文字半角全角キャッシュ
	TCHAR		m_lfFaceName[LF_FACESIZE];
	TCHAR		m_lfFaceName2[LF_FACESIZE];
	short		m_nCharPxWidthCache[0x10000];
	int			m_nCharWidthCacheTest;				//cache溢れ検出
};

enum ECharWidthFontMode {
	CWM_FONT_EDIT,
	CWM_FONT_PRINT,
	CWM_FONT_MINIMAP,
	CWM_FONT_MAX,
};
enum ECharWidthCacheMode {
	CWM_CACHE_NEUTRAL,
	CWM_CACHE_SHARE,
	CWM_CACHE_LOCAL,
};

// キャッシュの初期化関数群
void SelectCharWidthCache( ECharWidthFontMode fMode, ECharWidthCacheMode cMode );  //!< モードを変更したいとき
void InitCharWidthCache( const LOGFONT &lf, ECharWidthFontMode fMode=CWM_FONT_EDIT ); //!< フォントを変更したとき
void InitCharWidthCacheFromDC(const LOGFONT* lfs, ECharWidthFontMode fMode, HDC hdcOrg );

#endif /* SAKURA_CHARCODE_5A887F7C_8E08_4940_AF65_BD6850C3A7B5_H_ */
/*[EOF]*/
