#include "stdafx.h"
#include "charcode.h"



namespace WCODE
{
	//2007.08.30 kobake 追加
	bool isHankaku(wchar_t wc)
	{
		//※ほぼ未検証。ロジックが確定したらインライン化すると良い。

		//参考：http://www.swanq.co.jp/blog/archives/000783.html
		if(
			   wc<=0x007E //ACODEとか
			|| wc==0x00A5 //バックスラッシュ
			|| wc==0x203E //にょろ
			|| (wc>=0xFF61 && wc<=0xFF9f)
		)return true;

		//0x7F ～ 0xA0 も半角とみなす
		//http://ja.wikipedia.org/wiki/Unicode%E4%B8%80%E8%A6%A7_0000-0FFF を見て、なんとなく
		if(wc>=0x007F && wc<=0x00A0)return true;

		//$$ 仮。もう動的に計算しちゃえ。(初回のみ)
		bool CalcHankakuByFont(wchar_t);
		return CalcHankakuByFont(wc);


		return false;
	}

	//!制御文字であるかどうか
	bool isControlCode(wchar_t wc)
	{
		//改行は制御文字とみなさない
		if(isLineDelimiter(wc))return false;

		//タブは制御文字とみなさない
		if(wc==TAB)return false;

		return iswcntrl(wc)!=0;
	}

}



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
		memset(cache,0,sizeof(cache));
		test=0x12345678;

		HDC hdc=GetDC(NULL);
		m_hdc = CreateCompatibleDC(hdc);
		ReleaseDC(NULL, hdc);
		m_hFont = CreateFontW(
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS,
			CLIP_CHARACTER_PRECIS,
			DEFAULT_QUALITY,
			FIXED_PITCH,
			L"ＭＳ ゴシック"
		);
		m_hfntOld = (HFONT)SelectObject(m_hdc,m_hFont);

		// -- -- 半角基準 -- -- //
		GetTextExtentPoint32W_AnyBuild(m_hdc,L"x",1,&m_han_size);
	}
	~LocalCache()
	{
		// -- -- 後始末 -- -- //
		SelectObject(m_hdc,m_hfntOld);
		DeleteObject(m_hFont);
		DeleteDC(m_hdc);
	}
	void SetCache(wchar_t c, bool cache_value)
	{
		int v=cache_value?0x1:0x2;
		cache[c/4] &= ~( 0x3<< ((c%4)*2) ); //該当箇所クリア
		cache[c/4] |=  ( v  << ((c%4)*2) ); //該当箇所セット
	}
	bool GetCache(wchar_t c) const
	{
		return _GetRaw(c)==0x1?true:false;
	}
	bool ExistCache(wchar_t c) const
	{
		assert(test==0x12345678);
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
		return (cache[c/4]>>((c%4)*2))&0x3;
	}
private:
	BYTE cache[0x10000/4]; //16KB
	int test; //cache溢れ検出

	HDC m_hdc;
	HFONT m_hfntOld;
	HFONT m_hFont;
	SIZE m_han_size;
};

//文字幅の動的計算。半角ならtrue。
bool CalcHankakuByFont(wchar_t c)
{
	// -- -- キャッシュが存在すれば、それをそのまま返す -- -- //
	static LocalCache cache; //$$ これはShare領域に入れておいたほうが、もっと良い
	if(cache.ExistCache(c))return cache.GetCache(c);

	// -- -- 相対比較 -- -- //
	bool value;;
	value = cache.CalcHankakuByFont(c);

	// -- -- キャッシュ更新 -- -- //
	cache.SetCache(c,value);

	return cache.GetCache(c);
}






