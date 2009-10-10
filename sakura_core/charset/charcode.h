//2007.09.13 kobake 作成

#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         判定関数                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "charset/codechecker.h"
inline bool _IS_SJIS_1(unsigned char c)
{
	return IsSjisZen1(static_cast<char>(c));
}
inline bool _IS_SJIS_2(unsigned char c)
{
	return IsSjisZen2(static_cast<char>(c));
}
inline bool _IS_SJIS_1(char c)
{
	return IsSjisZen1(c);
}
inline bool _IS_SJIS_2(char c)
{
	return IsSjisZen2(c);
}
inline int my_iskanji1( int c )
{
	return IsSjisZen1(static_cast<char>(c & 0x00ff));
}
inline int my_iskanji2( int c )
{
	return IsSjisZen2(static_cast<char>(c & 0x00ff));
}

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




//Oct. 31, 2000 JEPRO  TeX Keyword のために'\'を追加
//Nov.  9, 2000 JEPRO  HSP Keyword のために'@'を追加
//Oct. 18, 2007 kobake UNICODE用に書き直し
inline bool IS_KEYWORD_CHAR(wchar_t wc)
{
	if(wc==L'#')return true;				//user-define
	if(wc==L'$')return true;				//user-define
	if(wc>=L'0' && wc<=L'9')return true;	//iscsym
	if(wc==L'@')return true;				//user-define
	if(wc>=L'A' && wc<=L'Z')return true;	//iscsym
	if(wc>=L'a' && wc<=L'z')return true;	//iscsym
	if(wc==L'_')return true;				//iscsym
	if(wc==L'\\')return true;				//user-define
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

	//!全角スペースかどうか判定
	inline bool IsZenkakuSpace(wchar_t wc)
	{
		return wc == 0x3000; //L'　'
	}

	//!制御文字であるかどうか
	bool IsControlCode(wchar_t wc);

	//!改行文字であるかどうか
	inline bool IsLineDelimiter(wchar_t wc)
	{
		return wc==CR || wc==LF;
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
	inline bool IsValidFilenameChar(const wchar_t* pData, size_t nIndex)
	{
		static const wchar_t* table = L"<>?\"|*";

		wchar_t wc = pData[nIndex];
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
		return c>=0x0410 && c<=0x044F;
	}

	//! BOX DRAWING 文字 かどうか
	inline bool IsBoxDrawing(wchar_t c)
	{
		return c>=0x2500 && c<=0x257F;
	}

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
		if(n>=0x7F && n<=0xA0)return true;
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
	inline bool IsValidFilenameChar(const char* pData, size_t nIndex)
	{
		static const TCHAR* table = _T("<>?\"|*");
		char c = pData[nIndex];

		//table内の文字が含まれていて
		if(_tcschr(table,c)!=NULL){
			//それが1バイト文字だったら
			if( nIndex==0 || (nIndex>0 && !_IS_SJIS_1(pData[nIndex-1])) ){
				//使っちゃいけない文字！
				return false;
			}
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

// 文字幅キャッシュ関連
void InitCharWidthCache( const LOGFONT &lf );
void InitCharWidthCacheCommon();
