//2007.09.13 kobake 作成

#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         判定関数                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

inline bool _IS_SJIS_1(unsigned char c)
{
	return Charcode::IsSJisKan1(c);
}
inline bool _IS_SJIS_2(unsigned char c)
{
	return Charcode::IsSJisKan2(c);
}
inline bool _IS_SJIS_1(char c)
{
	return Charcode::IsSJisKan1((unsigned char)c);
}
inline bool _IS_SJIS_2(char c)
{
	return Charcode::IsSJisKan2((unsigned char)c);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           定数                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//定数の素 (直接使用は控えてください)
#define TAB_ 				'\t'
#define SPACE_				' '
#define CR_					'\015'
#define LF_					'\012'
#define ESC_				'\x1b'
#define CRLF_				"\015\012"
#define LFCR_				"\012\015"

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
	static const char LFCR[] = LFCR_;

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
	static const wchar_t LFCR[] = LTEXT(LFCR_);

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
	inline bool isAZ(wchar_t wc)
	{
		return (wc>=L'A' && wc<=L'Z') || (wc>=L'a' && wc<=L'z');
	}
	inline bool is09(wchar_t wc)
	{
		return (wc>=L'0' && wc<=L'9');
	}

	//!半角文字(縦長長方形)かどうか判定
	bool isHankaku(wchar_t wc);

	//!全角文字(正方形)かどうか判定
	inline bool isZenkaku(wchar_t wc)
	{
		return !isHankaku(wc);
	}

	//!全角スペースかどうか判定
	inline bool isZenkakuSpace(wchar_t wc)
	{
		return wc == 0x3000; //L'　'
	}

	//!制御文字であるかどうか
	bool isControlCode(wchar_t wc);

	//!改行文字であるかどうか
	inline bool isLineDelimiter(wchar_t wc)
	{
		return wc==CR || wc==LF;
	}

	//!単語の区切り文字であるかどうか
	inline bool isWordDelimiter(wchar_t wc)
	{
		return wc==SPACE || wc==TAB || isZenkakuSpace(wc);
	}

	//!インデント構成要素であるかどうか。bAcceptZenSpace: 全角スペースを含めるかどうか
	inline bool isIndentChar(wchar_t wc,bool bAcceptZenSpace)
	{
		if(wc==TAB || wc==SPACE)return true;
		if(bAcceptZenSpace && isZenkakuSpace(wc))return true;
		return false;
	}

	//!空白かどうか
	inline bool isBlank(wchar_t wc)
	{
		return wc==TAB || wc==SPACE || isZenkakuSpace(wc);
	}

	//!CPPキーワードで始まっていれば true
	inline bool isHeadCppKeyword(const wchar_t* pData)
	{
		#define HEAD_EQ(DATA,LITERAL) (wcsncmp(DATA,LITERAL,_countof(LITERAL)-1)==0)
		if( HEAD_EQ(pData, L"case"      ) )return true;
		if( HEAD_EQ(pData, L"default:"  ) )return true;
		if( HEAD_EQ(pData, L"public:"   ) )return true;
		if( HEAD_EQ(pData, L"private:"  ) )return true;
		if( HEAD_EQ(pData, L"protected:") )return true;
		return false;
	}

	//!ファイル名に使える文字であるかどうか
	inline bool isValidFilenameChar(const wchar_t* pData, size_t nIndex)
	{
		static const wchar_t* table = L"<>?\"|*";

		wchar_t wc = pData[nIndex];
		if(wcschr(table,wc)!=NULL)return false; //table内の文字が含まれていたら、ダメ。
		else return true;
	}

	//!タブ表示に使える文字かどうか
	inline bool isTabAvailableCode(wchar_t wc)
	{
		//$$要検証
		if(wc==L'\0')return false;
		if(wc==L'\r')return false;
		if(wc==L'\n')return false;
		if(wc==L'\t')return false;
		return true;
	}

	//! 半角カナかどうか
	inline bool isHankakuKatakana(wchar_t c)
	{
		//参考: http://ash.jp/code/unitbl1.htm
		return c>=0xFF61 && c<=0xFF9F;
	}

	//! 全角記号かどうか
	inline bool isZenkakuKigou(wchar_t c)
	{
		//$ 他にも全角記号はあると思うけど、とりあえずANSI版時代の判定を踏襲。パフォーマンス悪し。
		static const wchar_t* table=L"　、。，．・：；？！゛゜´｀¨＾￣＿ヽヾゝゞ〃仝々〆〇ー―‐／＼～∥｜…‥‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭♪†‡¶◯";
		return wcschr(table,c)!=NULL;
	}

	//! ひらがなかどうか
	inline bool isHiragana(wchar_t c)
	{
		return c>=0x3041 && c<=0x3093;
	}

	//! カタカナかどうか
	inline bool isZenkakuKatakana(wchar_t c)
	{
		return c>=0x30A1 && c<=0x30F6;
	}

	//! ギリシャ文字かどうか
	inline bool isGreek(wchar_t c)
	{
		return c>=0x0391 && c<=0x03C9;
	}

	//! キリル文字かどうか
	inline bool isCyrillic(wchar_t c)
	{
		return c>=0x0410 && c<=0x044F;
	}

	//! BOX DRAWING 文字 かどうか
	inline bool isBoxDrawing(wchar_t c)
	{
		return c>=0x2500 && c<=0x257F;
	}
}


//ANSI判定関数群
namespace ACODE
{
	//!制御文字であるかどうか
	inline bool isControlCode(char c)
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
	inline bool isTabAvailableCode(char c)
	{
		if(c=='\0')return false;
		if(c<=0x1f)return false;
		if(c>=0x7f)return false;
		return true;
	}

	//!ファイル名に使える文字であるかどうか
	inline bool isValidFilenameChar(const char* pData, size_t nIndex)
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







