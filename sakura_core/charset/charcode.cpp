#include "stdafx.h"
#include "charset/charcode.h"

#include "env/DLLSHAREDATA.h"


namespace WCODE
{
	bool CalcHankakuByFont(wchar_t);

	//2007.08.30 kobake 追加
	bool IsHankaku(wchar_t wc)
	{
		//※ほぼ未検証。ロジックが確定したらインライン化すると良い。

		//参考：http://www.swanq.co.jp/blog/archives/000783.html
		if(
			   wc<=0x007E //ACODEとか
//			|| wc==0x00A5 //円マーク
//			|| wc==0x203E //にょろ
			|| (wc>=0xFF61 && wc<=0xFF9f)	// 半角カタカナ
		)return true;

		//0x7F ～ 0xA0 も半角とみなす
		//http://ja.wikipedia.org/wiki/Unicode%E4%B8%80%E8%A6%A7_0000-0FFF を見て、なんとなく
		if(wc>=0x007F && wc<=0x00A0)return true;	// Control Code ISO/IEC 6429

		// 漢字は全角とみなす
		if ( wc>=0x4E00 && wc<=0x9FBB		// Unified Ideographs, CJK
		  || wc>=0x3400 && wc<=0x4DB5		// Unified Ideographs Extension A, CJK
		  || wc>=0xAC00 && wc<=0xD7A3		// 	Hangul Syllables
		) return false;	// Private Use Area

		// 外字は全角とみなす
		if (wc>=0xE000 && wc<=0xE8FF)	return false;	// Private Use Area

		//$$ 仮。もう動的に計算しちゃえ。(初回のみ)
		return CalcHankakuByFont(wc);
	}

	//!制御文字であるかどうか
	bool IsControlCode(wchar_t wc)
	{
		//改行は制御文字とみなさない
		if(IsLineDelimiter(wc))return false;

		//タブは制御文字とみなさない
		if(wc==TAB)return false;

		return iswcntrl(wc)!=0;
	}

#if 0
	/*!
		句読点か
		2008.04.27 kobake CLayoutMgr::IsKutoTen から分離

		@param[in] c1 調べる文字1バイト目
		@param[in] c2 調べる文字2バイト目
		@retval true 句読点である
		@retval false 句読点でない
	*/
	bool IsKutoten( wchar_t wc )
	{
		//句読点定義
		static const wchar_t *KUTOTEN=
			L"｡､,."
			L"。、，．"
		;

		const wchar_t* p;
		for(p=KUTOTEN;*p;p++){
			if(*p==wc)return true;
		}
		return false;
	}
#endif


/*!
	UNICODE文字情報のキャッシュクラス。
	1文字当たり2ビットで、値を保存しておく。
	00:未初期化
	01:半角
	10:全角
	11:-
*/
class LocalCache{
public:
	LocalCache()
	{
		/* LOGFONTの初期化 */
		memset( &m_lf, 0, sizeof(m_lf) );

		// HDC の初期化
		HDC hdc=GetDC(NULL);
		m_hdc = CreateCompatibleDC(hdc);
		ReleaseDC(NULL, hdc);

		m_hFont =NULL;
		m_hfntOld =NULL;
	}
	~LocalCache()
	{
		// -- -- 後始末 -- -- //
		if (m_hFont != NULL) {
			SelectObject(m_hdc, m_hfntOld);
			DeleteObject(m_hFont);
		}
		DeleteDC(m_hdc);
	}
	// 再初期化
	void Init( const LOGFONT &lf )
	{
		if (m_hfntOld != NULL) {
			m_hfntOld = (HFONT)SelectObject(m_hdc, m_hfntOld);
			DeleteObject(m_hfntOld);
		}

		m_lf = lf;

		m_hFont = ::CreateFontIndirect( &lf );
		m_hfntOld = (HFONT)SelectObject(m_hdc,m_hFont);

		// -- -- 半角基準 -- -- //
		GetTextExtentPoint32W_AnyBuild(m_hdc,L"x",1,&m_han_size);
	}
	void SetCache(wchar_t c, bool cache_value)
	{
		int v=cache_value?0x1:0x2;
		GetDllShareData().m_bCharWidthCache[c/4] &= ~( 0x3<< ((c%4)*2) ); //該当箇所クリア
		GetDllShareData().m_bCharWidthCache[c/4] |=  ( v  << ((c%4)*2) ); //該当箇所セット
	}
	bool GetCache(wchar_t c) const
	{
		return _GetRaw(c)==0x1?true:false;
	}
	bool ExistCache(wchar_t c) const
	{
		assert(GetDllShareData().m_nCharWidthCacheTest==0x12345678);
		return _GetRaw(c)!=0x0;
	}
	bool CalcHankakuByFont(wchar_t c)
	{
		SIZE size={m_han_size.cx*2,0}; //関数が失敗したときのことを考え、全角幅で初期化しておく
		GetTextExtentPoint32W_AnyBuild(m_hdc,&c,1,&size);
		return (size.cx<=m_han_size.cx);
	}
protected:
	int _GetRaw(wchar_t c) const
	{
		return (GetDllShareData().m_bCharWidthCache[c/4]>>((c%4)*2))&0x3;
	}
private:
	HDC m_hdc;
	HFONT m_hfntOld;
	HFONT m_hFont;
	SIZE m_han_size;
	LOGFONT	m_lf;							// 2008/5/15 Uchi
};

static LocalCache cache;

//文字幅の動的計算。半角ならtrue。
bool CalcHankakuByFont(wchar_t c)
{
	// -- -- キャッシュが存在すれば、それをそのまま返す -- -- //
	if(cache.ExistCache(c))return cache.GetCache(c);

	// -- -- 相対比較 -- -- //
	bool value;
	value = cache.CalcHankakuByFont(c);

	// -- -- キャッシュ更新 -- -- //
	cache.SetCache(c,value);

	return cache.GetCache(c);
}
}

//	文字幅の動的計算用キャッシュの初期化。	2007/5/18 Uchi
void InitCharWidthCache( const LOGFONT &lf )
{
	WCODE::cache.Init(lf);
}

//	文字幅の動的計算用キャッシュの初期化。	2007/5/18 Uchi
void InitCharWidthCacheCommon()
{
	// キャッシュのクリア
	memset(GetDllShareData().m_bCharWidthCache, 0, sizeof(GetDllShareData().m_bCharWidthCache));
	GetDllShareData().m_nCharWidthCacheTest=0x12345678;
}
