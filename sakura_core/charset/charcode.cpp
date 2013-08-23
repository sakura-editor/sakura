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

#include "StdAfx.h"
#include "charset/charcode.h"

#include "env/DLLSHAREDATA.h"

/*! キーワードキャラクタ */
const unsigned char gm_keyword_char[128] = {
 /* 0         1         2         3         4         5         6         7         8         9         A         B         C         D         E         F             : 0123456789ABCDEF */
	CK_NULL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_TAB,   CK_LF,    CK_CTRL,  CK_CTRL,  CK_CR,    CK_CTRL,  CK_CTRL,  /* 0: ................ */
	CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  /* 1: ................ */
	CK_SPACE, CK_ETC,   CK_ETC,   CK_UDEF,  CK_UDEF,  CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   /* 2:  !"#$%&'()*+,-./ */
	CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   /* 3: 0123456789:;<=>? */
	CK_UDEF,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  /* 4: @ABCDEFGHIJKLMNO */
	CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_ETC,   CK_UDEF,  CK_ETC,   CK_ETC,   CK_CSYM,  /* 5: PQRSTUVWXYZ[\]^_ */
	CK_ETC,   CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  /* 6: `abcdefghijklmno */
	CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_CSYM,  CK_ETC,   CK_ETC,   CK_ETC,   CK_ETC,   CK_CTRL,   /* 7: pqrstuvwxyz{|}~. */
	/* 0: not-keyword, 1:__iscsym(), 2:user-define */
};

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

		// 漢字はすべて同一幅とみなす	// 2013.04.07 aroka
		if ( wc>=0x4E00 && wc<=0x9FBB		// Unified Ideographs, CJK
		  || wc>=0x3400 && wc<=0x4DB5		// Unified Ideographs Extension A, CJK
		){
			wc = 0x4E00; // '一'(0x4E00)の幅で代用
		}
		else
		// ハングルはすべて同一幅とみなす	// 2013.04.08 aroka
		if ( wc>=0xAC00 && wc<=0xD7A3 )		// Hangul Syllables
		{
			wc = 0xAC00; // (0xAC00)の幅で代用
		}
		else
		// 外字はすべて同一幅とみなす	// 2013.04.08 aroka
		if (wc>=0xE000 && wc<=0xE8FF) // Private Use Area
		{
			wc = 0xE000; // (0xE000)の幅で代用
		}

		//$$ 仮。もう動的に計算しちゃえ。(初回のみ)
		return CalcHankakuByFont(wc);
	}

	//!制御文字であるかどうか
	bool IsControlCode(wchar_t wc)
	{
		////改行は制御文字とみなさない
		//if(IsLineDelimiter(wc))return false;

		////タブは制御文字とみなさない
		//if(wc==TAB)return false;

		//return iswcntrl(wc)!=0;
		return (wc<128 && gm_keyword_char[wc]==CK_CTRL);
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
			m_hFontOld =NULL;
			m_pCache = 0;
		}
		~LocalCache()
		{
			// -- -- 後始末 -- -- //
			if (m_hFont != NULL) {
				SelectObject(m_hdc, m_hFontOld);
				DeleteObject(m_hFont);
			}
			DeleteDC(m_hdc);
		}
		// 再初期化
		void Init( const LOGFONT &lf )
		{
			if (m_hFontOld != NULL) {
				m_hFontOld = (HFONT)SelectObject(m_hdc, m_hFontOld);
				DeleteObject(m_hFontOld);
			}

			m_lf = lf;

			m_hFont = ::CreateFontIndirect( &lf );
			m_hFontOld = (HFONT)SelectObject(m_hdc,m_hFont);

			// -- -- 半角基準 -- -- //
			GetTextExtentPoint32W_AnyBuild(m_hdc,L"x",1,&m_han_size);
		}
		void SelectCache( SCharWidthCache* pCache )
		{
			m_pCache = pCache;
		}
		void Clear()
		{
			assert(m_pCache!=0);
			// キャッシュのクリア
			memcpy(m_pCache->m_lfFaceName, m_lf.lfFaceName, sizeof(m_lf.lfFaceName));
			memset(m_pCache->m_bCharWidthCache, 0, sizeof(m_pCache->m_bCharWidthCache));
			m_pCache->m_nCharWidthCacheTest=0x12345678;
		}
		bool IsSameFontFace( const LOGFONT &lf )
		{
			assert(m_pCache!=0);
			return ( memcmp(m_pCache->m_lfFaceName, lf.lfFaceName, sizeof(lf.lfFaceName)) == 0 );
		}
		void SetCache(wchar_t c, bool cache_value)
		{
			int v=cache_value?0x1:0x2;
			m_pCache->m_bCharWidthCache[c/4] &= ~( 0x3<< ((c%4)*2) ); //該当箇所クリア
			m_pCache->m_bCharWidthCache[c/4] |=  ( v  << ((c%4)*2) ); //該当箇所セット
		}
		bool GetCache(wchar_t c) const
		{
			return _GetRaw(c)==0x1?true:false;
		}
		bool ExistCache(wchar_t c) const
		{
			assert(m_pCache->m_nCharWidthCacheTest==0x12345678);
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
			return (m_pCache->m_bCharWidthCache[c/4]>>((c%4)*2))&0x3;
		}
	private:
		HDC					m_hdc;
		HFONT				m_hFontOld;
		HFONT				m_hFont;
		SIZE				m_han_size;
		LOGFONT				m_lf;				// 2008/5/15 Uchi
		SCharWidthCache*	m_pCache;
	};

	class LocalCacheSelector{
	public:
		LocalCacheSelector()
		{
			pcache = &m_localcache[0];
			for( int i=0; i<CWM_FONT_MAX; i++ ){
				m_parCache[i] = 0;
			}
			m_eLastEditCacheMode = CWM_CACHE_NEUTRAL;
		}
		~LocalCacheSelector()
		{
			for( int i=0; i<CWM_FONT_MAX; i++ ){
				delete m_parCache[i];
				m_parCache[i] = 0;
			}
		}
		void Init( const LOGFONT &lf, ECharWidthFontMode fMode )
	 	{
			//	Fontfaceが変更されていたらキャッシュをクリアする	2013.04.08 aroka
			m_localcache[fMode].Init(lf);
			if( !m_localcache[fMode].IsSameFontFace(lf) )
			{
				m_localcache[fMode].Clear();
			}
		}
		void Select( ECharWidthFontMode fMode, ECharWidthCacheMode cMode )
		{
			ECharWidthCacheMode cmode = (cMode==CWM_CACHE_NEUTRAL)?m_eLastEditCacheMode:cMode;

			pcache = &m_localcache[fMode];
			if( cmode == CWM_CACHE_SHARE ){
				pcache->SelectCache( &(GetDllShareData().m_sCharWidth) );
			}else{
				if( m_parCache[fMode] == 0 ){
					m_parCache[fMode] = new SCharWidthCache;
				}
				pcache->SelectCache( m_parCache[fMode] );
			}
			if( fMode==CWM_FONT_EDIT ){ m_eLastEditCacheMode = cmode; }
		}
		LocalCache* GetCache(){ return pcache; }
	private:
		LocalCache* pcache;
		LocalCache m_localcache[2];
		SCharWidthCache* m_parCache[2];
		ECharWidthCacheMode m_eLastEditCacheMode;
	};

	static LocalCacheSelector selector;


	//文字幅の動的計算。半角ならtrue。
	bool CalcHankakuByFont(wchar_t c)
	{
		LocalCache* pcache = selector.GetCache();
		// -- -- キャッシュが存在すれば、それをそのまま返す -- -- //
		if(pcache->ExistCache(c))return pcache->GetCache(c);

		// -- -- 相対比較 -- -- //
		bool value;
		value = pcache->CalcHankakuByFont(c);

		// -- -- キャッシュ更新 -- -- //
		pcache->SetCache(c,value);

		return pcache->GetCache(c);
	}
}

//	文字幅の動的計算用キャッシュの初期化。	2007/5/18 Uchi
void InitCharWidthCache( const LOGFONT &lf, ECharWidthFontMode fMode )
{
	WCODE::selector.Init( lf, fMode );
}

//	文字幅の動的計算用キャッシュの選択	2013.04.08 aroka
void SelectCharWidthCache( ECharWidthFontMode fMode, ECharWidthCacheMode cMode  )
{
	assert( fMode==CWM_FONT_EDIT || cMode==CWM_CACHE_LOCAL );

	WCODE::selector.Select( fMode, cMode );
}
