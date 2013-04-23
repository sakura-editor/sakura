#include "StdAfx.h"
#include "charset/charcode.h"

#include "env/DLLSHAREDATA.h"

/*! �L�[���[�h�L�����N�^ */
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
		return (wc<128 && gm_keyword_char[wc]==CK_CTRL);
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

		// HDC �̏�����
		HDC hdc=GetDC(NULL);
		m_hdc = CreateCompatibleDC(hdc);
		ReleaseDC(NULL, hdc);

		m_hFont =NULL;
		m_hFontOld =NULL;
		m_pCache = 0;
	}
	~LocalCache()
	{
		// -- -- ��n�� -- -- //
		if (m_hFont != NULL) {
			SelectObject(m_hdc, m_hFontOld);
			DeleteObject(m_hFont);
		}
		DeleteDC(m_hdc);
	}
	// �ď�����
	void Init( const LOGFONT &lf )
	{
		if (m_hFontOld != NULL) {
			m_hFontOld = (HFONT)SelectObject(m_hdc, m_hFontOld);
			DeleteObject(m_hFontOld);
		}

		m_lf = lf;

		m_hFont = ::CreateFontIndirect( &lf );
		m_hFontOld = (HFONT)SelectObject(m_hdc,m_hFont);

		// -- -- ���p� -- -- //
		GetTextExtentPoint32W_AnyBuild(m_hdc,L"x",1,&m_han_size);
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
		m_pCache->m_bCharWidthCache[c/4] &= ~( 0x3<< ((c%4)*2) ); //�Y���ӏ��N���A
		m_pCache->m_bCharWidthCache[c/4] |=  ( v  << ((c%4)*2) ); //�Y���ӏ��Z�b�g
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
		SIZE size={m_han_size.cx*2,0}; //�֐������s�����Ƃ��̂��Ƃ��l���A�S�p���ŏ��������Ă���
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
			if( m_parCache[i] != &(GetDllShareData().m_sCharWidth) ){
				delete m_parCache[i];
				m_parCache[i] = 0;
			}
		}
	}
	void Init( const LOGFONT &lf, ECharWidthFontMode fMode )
 	{
		//	Fontface���ύX����Ă�����L���b�V�����N���A����	2013.04.08 aroka
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


//�������̓��I�v�Z�B���p�Ȃ�true�B
bool CalcHankakuByFont(wchar_t c)
{
	LocalCache* pcache = selector.GetCache();
	// -- -- �L���b�V�������݂���΁A��������̂܂ܕԂ� -- -- //
	if(pcache->ExistCache(c))return pcache->GetCache(c);

	// -- -- ���Δ�r -- -- //
	bool value;
	value = pcache->CalcHankakuByFont(c);

	// -- -- �L���b�V���X�V -- -- //
	pcache->SetCache(c,value);

	return pcache->GetCache(c);
}
}

//	�������̓��I�v�Z�p�L���b�V���̏������B	2007/5/18 Uchi
void InitCharWidthCache( const LOGFONT &lf, ECharWidthFontMode fMode )
{
	WCODE::selector.Init( lf, fMode );
}

//	�������̓��I�v�Z�p�L���b�V���̑I��	2013.04.08 aroka
void SelectCharWidthCache( ECharWidthFontMode fMode, ECharWidthCacheMode cMode  )
{
	assert( fMode==CWM_FONT_EDIT || cMode==CWM_CACHE_LOCAL );

	WCODE::selector.Select( fMode, cMode );
}
