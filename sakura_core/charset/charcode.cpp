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

/*! �L�[���[�h�L�����N�^ */
const unsigned char gm_keyword_char[128] = {
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

	//2007.08.30 kobake �ǉ�
	bool IsHankaku(wchar_t wc)
	{
		//���قږ����؁B���W�b�N���m�肵����C�����C��������Ɨǂ��B

		//�Q�l�Fhttp://www.swanq.co.jp/blog/archives/000783.html
		if(
			   wc<=0x007E //ACODE�Ƃ�
//			|| wc==0x00A5 //�~�}�[�N
//			|| wc==0x203E //�ɂ��
			|| (wc>=0xFF61 && wc<=0xFF9f)	// ���p�J�^�J�i
		)return true;

		//0x7F �` 0xA0 �����p�Ƃ݂Ȃ�
		//http://ja.wikipedia.org/wiki/Unicode%E4%B8%80%E8%A6%A7_0000-0FFF �����āA�Ȃ�ƂȂ�
		if(wc>=0x007F && wc<=0x00A0)return true;	// Control Code ISO/IEC 6429

		// �����͂��ׂē��ꕝ�Ƃ݂Ȃ�	// 2013.04.07 aroka
		if ( wc>=0x4E00 && wc<=0x9FBB		// Unified Ideographs, CJK
		  || wc>=0x3400 && wc<=0x4DB5		// Unified Ideographs Extension A, CJK
		){
			wc = 0x4E00; // '��'(0x4E00)�̕��ő�p
		}
		else
		// �n���O���͂��ׂē��ꕝ�Ƃ݂Ȃ�	// 2013.04.08 aroka
		if ( wc>=0xAC00 && wc<=0xD7A3 )		// Hangul Syllables
		{
			wc = 0xAC00; // (0xAC00)�̕��ő�p
		}
		else
		// �O���͂��ׂē��ꕝ�Ƃ݂Ȃ�	// 2013.04.08 aroka
		if (wc>=0xE000 && wc<=0xE8FF) // Private Use Area
		{
			wc = 0xE000; // (0xE000)�̕��ő�p
		}

		//$$ ���B�������I�Ɍv�Z�����Ⴆ�B(����̂�)
		return CalcHankakuByFont(wc);
	}

	//!���䕶���ł��邩�ǂ���
	bool IsControlCode(wchar_t wc)
	{
		////���s�͐��䕶���Ƃ݂Ȃ��Ȃ�
		//if(IsLineDelimiter(wc))return false;

		////�^�u�͐��䕶���Ƃ݂Ȃ��Ȃ�
		//if(wc==TAB)return false;

		//return iswcntrl(wc)!=0;
		return (wc<_countof(gm_keyword_char) && gm_keyword_char[wc]==CK_CTRL);
	}

#if 0
	/*!
		��Ǔ_��
		2008.04.27 kobake CLayoutMgr::IsKutoTen ���番��

		@param[in] c1 ���ׂ镶��1�o�C�g��
		@param[in] c2 ���ׂ镶��2�o�C�g��
		@retval true ��Ǔ_�ł���
		@retval false ��Ǔ_�łȂ�
	*/
	bool IsKutoten( wchar_t wc )
	{
		//��Ǔ_��`
		static const wchar_t *KUTOTEN=
			L"��,."
			L"�B�A�C�D"
		;

		const wchar_t* p;
		for(p=KUTOTEN;*p;p++){
			if(*p==wc)return true;
		}
		return false;
	}
#endif


	/*!
		UNICODE�������̃L���b�V���N���X�B
		1����������2�r�b�g�ŁA�l��ۑ����Ă����B
		00:��������
		01:���p
		10:�S�p
		11:-
	*/
	class LocalCache{
	public:
		LocalCache()
		{
			/* LOGFONT�̏����� */
			memset( &m_lf, 0, sizeof(m_lf) );
			memset( &m_lf2, 0, sizeof(m_lf2) );

			// HDC �̏�����
			m_hdc = NULL;
			m_hdcFull = NULL;

			m_hFont =NULL;
			m_hFontOld =NULL;
			m_pCache = 0;
		}
		~LocalCache()
		{
			// -- -- ��n�� -- -- //
			DeleteLocalData();
		}
		void DeleteLocalData(){
	 		if (m_hFont != NULL) {
				SelectObject(m_hdc, m_hFontOld);
				DeleteObject(m_hFont);
				m_hFont = NULL;
			}
			if (m_hFontFull != NULL) {
				SelectObject(m_hdcFull, m_hFontFullOld);
				DeleteObject(m_hFontFull);
				m_hFontFull = NULL;
			}
			if(m_hdc){ DeleteDC(m_hdc); m_hdc = NULL;}
			if(m_hdcFull){ DeleteDC(m_hdcFull);  m_hdcFull = NULL;}
		}
		static bool IsEqual(const LOGFONT &lhs, const LOGFONT &rhs){
			return &lhs == &rhs ||
				0 == memcmp(&lhs, &rhs, sizeof(lhs));
		}

		// �ď�����
		void Init(const LOGFONT &lf, const LOGFONT &lfFull, HDC hdcOrg)
		{
			DeleteLocalData();

			m_hdc = ::CreateCompatibleDC(hdcOrg);
			m_lf = lf;
			m_lf2 = lfFull;

			m_hFont = ::CreateFontIndirect( &lf );
			m_hFontOld = (HFONT)SelectObject(m_hdc,m_hFont);
			bool bFullFont = !IsEqual(lf,lfFull);
			if( bFullFont ){
				m_bMultiFont = true;
				m_hdcFull = CreateCompatibleDC(hdcOrg);
				m_hFontFull = ::CreateFontIndirect(&lfFull);
				m_hFontFullOld = (HFONT)SelectObject(m_hdcFull, m_hFontFull);
			}else{
				m_bMultiFont = false;
				m_hdcFull = NULL;
				m_hFontFull = NULL;
				m_hFontFullOld = NULL;
			}
			s_MultiFont = m_bMultiFont;

			// -- -- ���p� -- -- //
			// CTextMetrics::Update �Ɠ����łȂ���΂Ȃ�Ȃ�
			HDC hdcArr[2] = {m_hdc, m_hdcFull};
			int size = (bFullFont ? 2 : 1);
			m_han_size.cx = 1;
			m_han_size.cy = 1;
			for(int i = 0; i < size; i++){
				// KB145994
				// tmAveCharWidth �͕s���m(���p���S�p�Ȃ̂����s���Ȓl��Ԃ�)
				SIZE sz;
				GetTextExtentPoint32W_AnyBuild(hdcArr[i], L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, &sz);
				sz.cx = (sz.cx / 26 + 1) / 2;
				if( m_han_size.cx < sz.cx ){
					m_han_size.cx = sz.cx;
				}
				if( m_han_size.cy < sz.cy ){
					m_han_size.cy = sz.cy;
				}
			}
		}
		void SelectCache( SCharWidthCache* pCache )
		{
			m_pCache = pCache;
		}
		void Clear()
		{
			assert(m_pCache!=0);
			// �L���b�V���̃N���A
			memcpy(m_pCache->m_lfFaceName, m_lf.lfFaceName, sizeof(m_lf.lfFaceName));
			memcpy(m_pCache->m_lfFaceName2, m_lf2.lfFaceName, sizeof(m_lf2.lfFaceName));
			memset(m_pCache->m_nCharPxWidthCache, 0, sizeof(m_pCache->m_nCharPxWidthCache));
			m_pCache->m_nCharWidthCacheTest=0x12345678;
		}
		bool IsSameFontFace( const LOGFONT &lf1, const LOGFONT &lf2 )
		{
			assert(m_pCache!=0);
			return ( memcmp(m_pCache->m_lfFaceName, lf1.lfFaceName, sizeof(lf1.lfFaceName)) == 0 &&
				memcmp(m_pCache->m_lfFaceName2, lf2.lfFaceName, sizeof(lf2.lfFaceName)) == 0 );
		}
		void SetCachePx(wchar_t c, short cache_pxwidth)
		{
			m_pCache->m_nCharPxWidthCache[c] = cache_pxwidth;
		}
		short GetCachePx(wchar_t c) const
		{
			return _GetRawPx(c);
		}
		bool ExistCache(wchar_t c) const
		{
			assert(m_pCache->m_nCharWidthCacheTest==0x12345678);
			return _GetRawPx(c)!=0x0;
		}
		bool CalcHankakuByFont(wchar_t c)
		{
			SIZE size={m_han_size.cx*2,0}; //�֐������s�����Ƃ��̂��Ƃ��l���A�S�p���ŏ��������Ă���
			GetTextExtentPoint32W_AnyBuild(SelectHDC(c),&c,1,&size);
			return (size.cx<=m_han_size.cx);
		}
		bool IsHankakuByWidth(int width){
			return width<=m_han_size.cx;
		}
		int CalcPxWidthByFont(wchar_t c)
		{
			SIZE size={m_han_size.cx*2,0}; //�֐������s�����Ƃ��̂��Ƃ��l���A�S�p���ŏ��������Ă���
			// 2014.12.21 �R���g���[���R�[�h�̕\���ENUL��1px���ɂȂ�̂��X�y�[�X���ɂ���
			if (WCODE::IsControlCode(c)) {
				GetTextExtentPoint32W_AnyBuild(SelectHDC(c),&c,1,&size);
				const int nCx = size.cx;
				const wchar_t proxyChar = ((L'\0' == c) ? ' ' : L'�');
				GetTextExtentPoint32W_AnyBuild(SelectHDC(proxyChar),&proxyChar,1,&size);
				return t_max<int>(nCx, size.cx);
			}
			GetTextExtentPoint32W_AnyBuild(SelectHDC(c),&c,1,&size);
			return t_max<int>(1,size.cx);
		}
		int CalcPxWidthByFont2(const wchar_t* pc2)
		{
			SIZE size={m_han_size.cx*2,0};
			// �T���Q�[�g�͑S�p�t�H���g
			GetTextExtentPoint32W_AnyBuild(m_hdcFull?m_hdcFull:m_hdc,pc2,2,&size);
			return t_max<int>(1,size.cx);
		}
		bool GetMultiFont() const
		{
			return m_bMultiFont;
		}
		
	protected:
		int _GetRawPx(wchar_t c) const
		{
			return m_pCache->m_nCharPxWidthCache[c];
		}
		HDC SelectHDC(wchar_t c) const
		{
			return m_hdcFull && WCODE::GetFontNo(c) ? m_hdcFull : m_hdc;
		}
	private:
		HDC					m_hdc;
		HDC					m_hdcFull;
		HFONT 				m_hFontOld, m_hFontFullOld;
		HFONT 				m_hFont, m_hFontFull;
		bool				m_bMultiFont;
		SIZE				m_han_size;
		LOGFONT				m_lf;				// 2008/5/15 Uchi
		LOGFONT				m_lf2;
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
		void Init( const LOGFONT &lf1, const LOGFONT &lf2, ECharWidthFontMode fMode, HDC hdc )
	 	{
			//	Fontface���ύX����Ă�����L���b�V�����N���A����	2013.04.08 aroka
			m_localcache[fMode].Init(lf1, lf2, hdc);
			// �T�C�Y��HDC���ς���Ă��N���A����K�v������
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
		LocalCache* GetCache(){ return pcache; }
	private:
		LocalCache* pcache;
		LocalCache m_localcache[3];
		SCharWidthCache* m_parCache[3];
		ECharWidthCacheMode m_eLastEditCacheMode;
	private:
		DISALLOW_COPY_AND_ASSIGN(LocalCacheSelector);
	};

	static LocalCacheSelector selector;


	//�������̓��I�v�Z�B�s�N�Z����
	int CalcPxWidthByFont(wchar_t c)
	{
		LocalCache* pcache = selector.GetCache();
		// -- -- �L���b�V�������݂���΁A��������̂܂ܕԂ� -- -- //
		if(pcache->ExistCache(c))return pcache->GetCachePx(c);

		int width;
		width = pcache->CalcPxWidthByFont(c);

		// -- -- �L���b�V���X�V -- -- //
		pcache->SetCachePx(c,width);

		return pcache->GetCachePx(c);
	}

	int CalcPxWidthByFont2(const wchar_t* pc){
		LocalCache* pcache = selector.GetCache();
		return pcache->CalcPxWidthByFont2(pc);
	}

	//�������̓��I�v�Z�B���p�Ȃ�true�B
	bool CalcHankakuByFont(wchar_t c)
	{
		LocalCache* pcache = selector.GetCache();
		return pcache->IsHankakuByWidth(CalcPxWidthByFont(c));
	}

	// �����̎g�p�t�H���g��Ԃ�
	// @return 0:���p�p / 1:�S�p�p
	int GetFontNo( wchar_t c ){
		if( s_MultiFont ){
			if(0x0080 <= c && c <= 0xFFFF){
				return 1;
			}
		}
		return 0;
	}
	int GetFontNo2( wchar_t wc1, wchar_t wc2 ){
		if( s_MultiFont ){
			return 1;
		}
		return 0;
	}
}

//	�������̓��I�v�Z�p�L���b�V���̏������B	2007/5/18 Uchi
void InitCharWidthCache( const LOGFONT &lf, ECharWidthFontMode fMode )
{
	HDC hdc = GetDC(NULL);
	WCODE::selector.Init( lf, lf, fMode, hdc );
	ReleaseDC(NULL, hdc);
}

void InitCharWidthCacheFromDC( const LOGFONT* lfs, ECharWidthFontMode fMode, HDC hdcOrg )
{
	WCODE::selector.Init(lfs[0], lfs[1], fMode, hdcOrg);
}

 //	�������̓��I�v�Z�p�L���b�V���̑I��	2013.04.08 aroka
void SelectCharWidthCache( ECharWidthFontMode fMode, ECharWidthCacheMode cMode  )
{
	assert( fMode==CWM_FONT_EDIT || cMode==CWM_CACHE_LOCAL );

	WCODE::selector.Select( fMode, cMode );
}
