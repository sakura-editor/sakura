/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_CHARCODE_4C34C669_0BAB_441A_9B1D_2B9AC1895380_H_
#define SAKURA_CHARCODE_4C34C669_0BAB_441A_9B1D_2B9AC1895380_H_
#pragma once

//2007.09.13 kobake 作成
#include <array>
#include "parse/CWordParse.h"
#include "util/std_macro.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           定数                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// SJISのコードページ(CP_ACP では無くこれを使えばおそらく英語版Winでも動くはず。)	2008/5/12 Uchi
constexpr int CP_SJIS = 932;

//UNICODE定数
namespace WCODE{
	//文字
	constexpr wchar_t TAB   = L'\t';
	constexpr wchar_t SPACE = L' ';
	constexpr wchar_t CR    = L'\r';
	constexpr wchar_t LF    = L'\n';
	constexpr wchar_t ESC   = L'\x1b';

	//文字列
	constexpr wchar_t CRLF[] = L"\r\n";

	//特殊 (BREGEXP)
	constexpr wchar_t BREGEXP_DELIMITER = (wchar_t)0xFFFF;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         判定関数                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! キーワードキャラクタ */
extern const std::array<unsigned char, 128> gm_keyword_char;

//Oct. 31, 2000 JEPRO  TeX Keyword のために'\'を追加
//Nov.  9, 2000 JEPRO  HSP Keyword のために'@'を追加
//Oct. 18, 2007 kobake UNICODE用に書き直し
//Nov. 27, 2010 syat   速度改善のためテーブルに変更
[[nodiscard]] inline bool IS_KEYWORD_CHAR(wchar_t wc)
{
	if(static_cast<std::size_t>(wc) < gm_keyword_char.size() && (gm_keyword_char[wc] == CK_CSYM||gm_keyword_char[wc] == CK_UDEF) )
		return true;
	else
		return false;
}

//UNICODE判定関数群
namespace WCODE
{
	[[nodiscard]] inline bool IsAZ(wchar_t wc)
	{
		return (wc>=L'A' && wc<=L'Z') || (wc>=L'a' && wc<=L'z');
	}
	[[nodiscard]] inline bool Is09(wchar_t wc)
	{
		return (wc>=L'0' && wc<=L'9');
	}
	[[nodiscard]] inline bool IsInRange(wchar_t c, wchar_t front, wchar_t back)
	{
		return c>=front && c<=back;
	}

	//!使用フォント番号を返す
	// (0:半角/1:全角)
	int GetFontNo(wchar_t c);
	int GetFontNo2(wchar_t wc1, wchar_t wc2);

	//!全角スペースかどうか判定
	[[nodiscard]] inline bool IsZenkakuSpace(wchar_t wc)
	{
		return wc == 0x3000; //L'　'
	}

	//!制御文字であるかどうか
	bool IsControlCode(wchar_t wc);

	//!改行文字であるかどうか
	[[nodiscard]] inline bool IsLineDelimiter(wchar_t wc, bool ext)
	{
		return wc==CR || wc==LF || (ext && (wc==0x85 || wc==0x2028 || wc==0x2029));
	}
	[[nodiscard]] inline bool IsLineDelimiterBasic(wchar_t wc)
	{
		return wc==CR || wc==LF;
	}
	[[nodiscard]] inline bool IsLineDelimiterExt(wchar_t wc)
	{
		return wc==CR || wc==LF || wc==0x85 || wc==0x2028 || wc==0x2029;
	}

	//!単語の区切り文字であるかどうか
	[[nodiscard]] inline bool IsWordDelimiter(wchar_t wc)
	{
		return wc==SPACE || wc==TAB || IsZenkakuSpace(wc);
	}

	//!インデント構成要素であるかどうか。bAcceptZenSpace: 全角スペースを含めるかどうか
	[[nodiscard]] inline bool IsIndentChar(wchar_t wc,bool bAcceptZenSpace)
	{
		if(wc==TAB || wc==SPACE)return true;
		if(bAcceptZenSpace && IsZenkakuSpace(wc))return true;
		return false;
	}

	//!空白かどうか
	[[nodiscard]] inline bool IsBlank(wchar_t wc)
	{
		return wc==TAB || wc==SPACE || IsZenkakuSpace(wc);
	}

	//!ファイル名に使える文字であるかどうか
	inline bool IsValidFilenameChar(const wchar_t wc)
	{
		constexpr const wchar_t table[] = L"<>?\"|*";

		//table内の文字が含まれていたら、ダメ。
		return !wcschr(table, wc);
	}

	//!タブ表示に使える文字かどうか
	[[nodiscard]] inline bool IsTabAvailableCode(wchar_t wc)
	{
		//$$要検証
		if(wc==L'\0')return false;
		if(wc==L'\r')return false;
		if(wc==L'\n')return false;
		if(wc==L'\t')return false;
		return true;
	}

	//! 半角カナかどうか
	[[nodiscard]] inline bool IsHankakuKatakana(wchar_t c)
	{
		//参考: http://ash.jp/code/unitbl1.htm
		return c>=0xFF61 && c<=0xFF9F;
	}

	//! 全角記号かどうか
	[[nodiscard]] inline bool IsZenkakuKigou(wchar_t c)
	{
		//$ 他にも全角記号はあると思うけど、とりあえずANSI版時代の判定を踏襲。パフォーマンス悪し。
		// 2009.06.26 syat 「ゝゞ（ひらがな）」「ヽヾ（カタカナ）」「゛゜（全角濁点）」「仝々〇（漢字）」「ー（長音）」を除外
		// 2009.10.10 syat ANSI版の修正にあわせて「〆」を記号→漢字にする
		constexpr const wchar_t table[] = L"　、。，．・：；？！´｀¨＾￣＿〃―‐／＼～∥｜…‥‘’“”（）〔〕［］｛｝〈〉《》「」『』【】＋－±×÷＝≠＜＞≦≧∞∴♂♀°′″℃￥＄￠￡％＃＆＊＠§☆★○●◎◇◆□■△▲▽▼※〒→←↑↓〓∈∋⊆⊇⊂⊃∪∩∧∨￢⇒⇔∀∃∠⊥⌒∂∇≡≒≪≫√∽∝∵∫∬Å‰♯♭♪†‡¶◯";
		return wcschr(table,c);
	}

	//! ひらがなかどうか
	[[nodiscard]] inline bool IsHiragana(wchar_t c)
	{
		// 2009.06.26 syat 「ゝゞ」を追加
		return (c>=0x3041 && c<=0x3096) || (c>=0x309D && c<=0x309E);
	}

	//! カタカナかどうか
	[[nodiscard]] inline bool IsZenkakuKatakana(wchar_t c)
	{
		// 2009.06.26 syat 「ヽヾ」を追加
		return (c>=0x30A1 && c<=0x30FA) || (c>=0x30FD && c<=0x30FE);
	}

	//! ギリシャ文字かどうか
	[[nodiscard]] inline bool IsGreek(wchar_t c)
	{
		return c>=0x0391 && c<=0x03C9;
	}

	//! キリル文字かどうか
	[[nodiscard]] inline bool IsCyrillic(wchar_t c)
	{
		return (c>=0x0400 && c<=0x052F)  // Cyrillic, Cyrillic Supplement
			|| (c>=0x2DE0 && c<=0x2DFF)  // Cyrillic Extended-A
			|| (c>=0xA640 && c<=0xA69F); // Cyrillic Extended-B
	}

	//! BOX DRAWING 文字 かどうか
	[[nodiscard]] inline bool IsBoxDrawing(wchar_t c)
	{
		return c>=0x2500 && c<=0x257F;
	}

	//!文字が半角かどうかを取得(DLLSHARE/フォント依存)
	bool CalcHankakuByFont(wchar_t c);
	//!文字のpx幅を取得(DLLSHARE/フォント依存)
	int  CalcPxWidthByFont(wchar_t c);
	//!文字のpx幅を取得(DLLSHARE/フォント依存)
	int  CalcPxWidthByFont2(const wchar_t* c);
}

// 文字幅の動的計算用キャッシュ関連
struct SCharWidthCache {
	// 文字半角全角キャッシュ
	std::array<WCHAR, LF_FACESIZE> m_lfFaceName;
	std::array<WCHAR, LF_FACESIZE> m_lfFaceName2;
	std::array<short, 0x10000> m_nCharPxWidthCache;
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

/*!
	文字幅情報のキャッシュクラス。
	1文字当たり2バイトで文字のピクセル幅を保存しておく。
*/
class CCharWidthCache {
public:
	CCharWidthCache() = default;
	CCharWidthCache(const CCharWidthCache&) = delete;
	CCharWidthCache& operator=(const CCharWidthCache&) = delete;
	~CCharWidthCache() { DeleteLocalData(); }

	// 再初期化
	void Init(const LOGFONT& lf, const LOGFONT& lfFull, HDC hdcOrg);
	void SelectCache(SCharWidthCache* pCache) { m_pCache = pCache; }
	void Clear();
	[[nodiscard]] bool GetMultiFont() const { return m_bMultiFont; }

	virtual bool CalcHankakuByFont(wchar_t c) const;
	virtual int CalcPxWidthByFont(wchar_t c);
	virtual int CalcPxWidthByFont2(const wchar_t* pc2) const;

private:
	void DeleteLocalData();
	int QueryPixelWidth(wchar_t c) const;
	[[nodiscard]] HDC SelectHDC(wchar_t c) const;

	HDC m_hdc = nullptr;
	HDC m_hdcFull = nullptr;
	HFONT m_hFontOld = nullptr;
	HFONT m_hFontFullOld = nullptr;
	HFONT m_hFont = nullptr;
	HFONT m_hFontFull = nullptr;
	bool m_bMultiFont;
	SIZE m_han_size;
	LOGFONT m_lf{};				// 2008/5/15 Uchi
	LOGFONT m_lf2{};
	SCharWidthCache* m_pCache = nullptr;
};

// キャッシュの初期化関数群
void SelectCharWidthCache( ECharWidthFontMode fMode, ECharWidthCacheMode cMode );  //!< モードを変更したいとき
void InitCharWidthCache( const LOGFONT &lf, ECharWidthFontMode fMode=CWM_FONT_EDIT ); //!< フォントを変更したとき
void InitCharWidthCacheFromDC(const LOGFONT* lfs, ECharWidthFontMode fMode, HDC hdcOrg );
[[nodiscard]] CCharWidthCache& GetCharWidthCache();

namespace WCODE {
	//!半角文字(縦長長方形)かどうか判定
	bool IsHankaku(wchar_t wc, const CCharWidthCache& cache = GetCharWidthCache());
	//!全角文字(正方形)かどうか判定
	inline bool IsZenkaku(wchar_t wc) { return !IsHankaku(wc); }
}

#endif /* SAKURA_CHARCODE_4C34C669_0BAB_441A_9B1D_2B9AC1895380_H_ */
