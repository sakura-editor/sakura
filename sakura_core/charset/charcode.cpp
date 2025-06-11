/*! @file */
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "charset/charcode.h"
#include <array>
#include "env/DLLSHAREDATA.h"

/*! キーワードキャラクタ */
const std::array<unsigned char, 128> gm_keyword_char = {
 /* 0         1         2         3         4         5         6         7         8         9         A         B         C         D         E         F             : 0123456789ABCDEF */
	CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_CTRL,  CK_TAB,   CK_LF,    CK_CTRL,  CK_CTRL,  CK_CR,    CK_CTRL,  CK_CTRL,  /* 0: ................ */
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
	static bool s_MultiFont;

	bool CalcHankakuByFont(wchar_t);

	//2007.08.30 kobake 追加
	bool IsHankaku(wchar_t wc, CCharWidthCache& cache)
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
		if ( (wc>=0x4E00 && wc<=0x9FBB)		// Unified Ideographs, CJK
		  || (wc>=0x3400 && wc<=0x4DB5)		// Unified Ideographs Extension A, CJK
		){
			wc = 0x4E00; // '一'(0x4E00)の幅で代用
		}
		// ハングルはすべて同一幅とみなす	// 2013.04.08 aroka
		else if ( wc>=0xAC00 && wc<=0xD7A3 )		// Hangul Syllables
		{
			wc = 0xAC00; // (0xAC00)の幅で代用
		}
		// 外字はすべて同一幅とみなす	// 2013.04.08 aroka
		else if (wc>=0xE000 && wc<=0xE8FF) // Private Use Area
		{
			wc = 0xE000; // (0xE000)の幅で代用
		}

		//$$ 仮。もう動的に計算しちゃえ。(初回のみ)
		return cache.CalcHankakuByFont(wc);
	}

	//!制御文字であるかどうか
	[[nodiscard]] bool IsControlCode(wchar_t wc)
	{
		return wc < gm_keyword_char.size() && gm_keyword_char[wc] == CK_CTRL;
	}
}

void CCharWidthCache::DeleteLocalData()
{
	if (m_hFont != nullptr) {
		SelectObject(m_hdc, m_hFontOld);
		DeleteObject(m_hFont);
		m_hFont = nullptr;
	}
	if (m_hFontFull != nullptr) {
		SelectObject(m_hdcFull, m_hFontFullOld);
		DeleteObject(m_hFontFull);
		m_hFontFull = nullptr;
	}
	if(m_hdc){ DeleteDC(m_hdc); m_hdc = nullptr;}
	if(m_hdcFull){ DeleteDC(m_hdcFull);  m_hdcFull = nullptr;}
}

void CCharWidthCache::Init(const LOGFONT &lf, const LOGFONT &lfFull, HDC hdcOrg)
{
	DeleteLocalData();

	m_hdc = ::CreateCompatibleDC(hdcOrg);
	m_lf = lf;
	m_lf2 = lfFull;

	m_hFont = ::CreateFontIndirect( &lf );
	m_hFontOld = (HFONT)SelectObject(m_hdc,m_hFont);
	const bool bFullFont = &lf != &lfFull && memcmp(&lf, &lfFull, sizeof(lf)) != 0;
	if( bFullFont ){
		m_bMultiFont = true;
		m_hdcFull = CreateCompatibleDC(hdcOrg);
		m_hFontFull = ::CreateFontIndirect(&lfFull);
		m_hFontFullOld = (HFONT)SelectObject(m_hdcFull, m_hFontFull);
	}else{
		m_bMultiFont = false;
		m_hdcFull = nullptr;
		m_hFontFull = nullptr;
		m_hFontFullOld = nullptr;
	}
	WCODE::s_MultiFont = m_bMultiFont;

	// -- -- 半角基準 -- -- //
	// CTextMetrics::Update と同じでなければならない
	std::array<HDC, 2> hdcArr = {m_hdc, m_hdcFull};
	int size = (bFullFont ? 2 : 1);
	m_han_size.cx = 1;
	m_han_size.cy = 1;
	for(int i = 0; i < size; i++){
		// KB145994
		// tmAveCharWidth は不正確(半角か全角なのかも不明な値を返す)
		SIZE sz;
		std::lock_guard lock(m_mtx);
		GetTextExtentPoint32(hdcArr[i], L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &sz);
		sz.cx = (sz.cx / 26 + 1) / 2;
		if( m_han_size.cx < sz.cx ){
			m_han_size.cx = sz.cx;
		}
		if( m_han_size.cy < sz.cy ){
			m_han_size.cy = sz.cy;
		}
	}
}

void CCharWidthCache::Clear()
{
	assert(m_pCache!=nullptr);
	// キャッシュのクリア
	memcpy(m_pCache->m_lfFaceName.data(), m_lf.lfFaceName, sizeof(m_lf.lfFaceName));
	memcpy(m_pCache->m_lfFaceName2.data(), m_lf2.lfFaceName, sizeof(m_lf2.lfFaceName));
	memset(m_pCache->m_nCharPxWidthCache.data(), 0, sizeof(m_pCache->m_nCharPxWidthCache));
	m_pCache->m_nCharWidthCacheTest=0x12345678;
}

bool CCharWidthCache::CalcHankakuByFont(wchar_t c)
{
	return CalcPxWidthByFont(c) <= m_han_size.cx;
}

int CCharWidthCache::QueryPixelWidth(wchar_t c) const
{
	SIZE size={m_han_size.cx*2,0}; //関数が失敗したときのことを考え、全角幅で初期化しておく
	std::lock_guard lock(m_mtx);
	// 2014.12.21 コントロールコードの表示・NULが1px幅になるのをスペース幅にする
	if (WCODE::IsControlCode(c)) {
		GetTextExtentPoint32(SelectHDC(c),&c,1,&size);
		const int nCx = size.cx;
		const wchar_t proxyChar = ((L'\0' == c) ? ' ' : L'･');
		GetTextExtentPoint32(SelectHDC(proxyChar),&proxyChar,1,&size);
		return t_max<int>(nCx, size.cx);
	}
	GetTextExtentPoint32(SelectHDC(c),&c,1,&size);
	// 等幅フォントでも GetTextExtentPoint32 で取得したピクセル幅が半角と全角でぴったし2倍の違いにならない事がある
	// 対策として半角より少しでも幅が広い場合は半角幅の2倍に揃える
	if ((m_lf.lfPitchAndFamily & FIXED_PITCH) && size.cx > m_han_size.cx)
		size.cx = 2 * m_han_size.cx;
	return t_max<int>(1,size.cx);
}

int CCharWidthCache::CalcPxWidthByFont(wchar_t c) {
	// キャッシュから文字の情報を取得する。情報がなければ、計算して登録する。
	if (!m_pCache->m_nCharPxWidthCache[c]) {
		m_pCache->m_nCharPxWidthCache[c] = static_cast<short>(QueryPixelWidth(c));
	}
	return m_pCache->m_nCharPxWidthCache[c];
}

int CCharWidthCache::CalcPxWidthByFont2(const wchar_t* pc2) const
{
	SIZE size={m_han_size.cx*2,0};
	std::lock_guard lock(m_mtx);
	// サロゲートは全角フォント
	GetTextExtentPoint32(m_hdcFull?m_hdcFull:m_hdc,pc2,2,&size);
	return t_max<int>(1,size.cx);
}
		
[[nodiscard]] HDC CCharWidthCache::SelectHDC(wchar_t c) const
{
	return m_hdcFull && WCODE::GetFontNo(c) ? m_hdcFull : m_hdc;
}

namespace WCODE {
	class CacheSelector{
	public:
		CacheSelector() : pcache(m_localcache.data())
		{
			for( int i=0; i<CWM_FONT_MAX; i++ ){
				m_parCache[i] = 0;
			}
		}
		~CacheSelector()
		{
			for( int i=0; i<CWM_FONT_MAX; i++ ){
				delete m_parCache[i];
				m_parCache[i] = 0;
			}
		}
		void Init( const LOGFONT &lf1, const LOGFONT &lf2, ECharWidthFontMode fMode, HDC hdc )
	 	{
			//	Fontfaceが変更されていたらキャッシュをクリアする	2013.04.08 aroka
			m_localcache[fMode].Init(lf1, lf2, hdc);
			// サイズやHDCが変わってもクリアする必要がある
			m_localcache[fMode].Clear();
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
			WCODE::s_MultiFont = pcache->GetMultiFont();
		}
		[[nodiscard]] CCharWidthCache* GetCache(){ return pcache; }
	private:
		std::array<CCharWidthCache, 3> m_localcache;
		std::array<SCharWidthCache*, 3> m_parCache;
		ECharWidthCacheMode m_eLastEditCacheMode = CWM_CACHE_NEUTRAL;
		CCharWidthCache* pcache;
		DISALLOW_COPY_AND_ASSIGN(CacheSelector);
	};

	static CacheSelector selector;

	// 文字の使用フォントを返す
	// @return 0:半角用 / 1:全角用
	[[nodiscard]] int GetFontNo( wchar_t c ){
		if (s_MultiFont && 0x0080 <= c && c <= 0xFFFF){
			return 1;
		}
		return 0;
	}
	[[nodiscard]] int GetFontNo2( wchar_t, wchar_t ){
		if( s_MultiFont ){
			return 1;
		}
		return 0;
	}
}

//	文字幅の動的計算用キャッシュの初期化。	2007/5/18 Uchi
void InitCharWidthCache( const LOGFONT &lf, ECharWidthFontMode fMode )
{
	HDC hdc = GetDC(nullptr);
	WCODE::selector.Init( lf, lf, fMode, hdc );
	ReleaseDC(nullptr, hdc);
}

void InitCharWidthCacheFromDC( const LOGFONT* lfs, ECharWidthFontMode fMode, HDC hdcOrg )
{
	WCODE::selector.Init(lfs[0], lfs[1], fMode, hdcOrg);
}

 //	文字幅の動的計算用キャッシュの選択	2013.04.08 aroka
void SelectCharWidthCache( ECharWidthFontMode fMode, ECharWidthCacheMode cMode  )
{
	assert( fMode==CWM_FONT_EDIT || cMode==CWM_CACHE_LOCAL );

	WCODE::selector.Select( fMode, cMode );
}

[[nodiscard]] CCharWidthCache& GetCharWidthCache()
{
	return *WCODE::selector.GetCache();
}
