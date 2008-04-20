#include "stdafx.h"
#include "charcode.h"



namespace WCODE
{
	//2007.08.30 kobake �ǉ�
	bool isHankaku(wchar_t wc)
	{
		//���قږ����؁B���W�b�N���m�肵����C�����C��������Ɨǂ��B

		//�Q�l�Fhttp://www.swanq.co.jp/blog/archives/000783.html
		if(
			   wc<=0x007E //ACODE�Ƃ�
			|| wc==0x00A5 //�o�b�N�X���b�V��
			|| wc==0x203E //�ɂ��
			|| (wc>=0xFF61 && wc<=0xFF9f)
		)return true;

		//0x7F �` 0xA0 �����p�Ƃ݂Ȃ�
		//http://ja.wikipedia.org/wiki/Unicode%E4%B8%80%E8%A6%A7_0000-0FFF �����āA�Ȃ�ƂȂ�
		if(wc>=0x007F && wc<=0x00A0)return true;

		//$$ ���B�������I�Ɍv�Z�����Ⴆ�B(����̂�)
		bool CalcHankakuByFont(wchar_t);
		return CalcHankakuByFont(wc);


		return false;
	}

	//!���䕶���ł��邩�ǂ���
	bool isControlCode(wchar_t wc)
	{
		//���s�͐��䕶���Ƃ݂Ȃ��Ȃ�
		if(isLineDelimiter(wc))return false;

		//�^�u�͐��䕶���Ƃ݂Ȃ��Ȃ�
		if(wc==TAB)return false;

		return iswcntrl(wc)!=0;
	}

}



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
			L"�l�r �S�V�b�N"
		);
		m_hfntOld = (HFONT)SelectObject(m_hdc,m_hFont);

		// -- -- ���p� -- -- //
		GetTextExtentPoint32W_AnyBuild(m_hdc,L"x",1,&m_han_size);
	}
	~LocalCache()
	{
		// -- -- ��n�� -- -- //
		SelectObject(m_hdc,m_hfntOld);
		DeleteObject(m_hFont);
		DeleteDC(m_hdc);
	}
	void SetCache(wchar_t c, bool cache_value)
	{
		int v=cache_value?0x1:0x2;
		cache[c/4] &= ~( 0x3<< ((c%4)*2) ); //�Y���ӏ��N���A
		cache[c/4] |=  ( v  << ((c%4)*2) ); //�Y���ӏ��Z�b�g
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
		SIZE size={m_han_size.cx*2,0}; //�֐������s�����Ƃ��̂��Ƃ��l���A�S�p���ŏ��������Ă���
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
	int test; //cache��ꌟ�o

	HDC m_hdc;
	HFONT m_hfntOld;
	HFONT m_hFont;
	SIZE m_han_size;
};

//�������̓��I�v�Z�B���p�Ȃ�true�B
bool CalcHankakuByFont(wchar_t c)
{
	// -- -- �L���b�V�������݂���΁A��������̂܂ܕԂ� -- -- //
	static LocalCache cache; //$$ �����Share�̈�ɓ���Ă������ق����A�����Ɨǂ�
	if(cache.ExistCache(c))return cache.GetCache(c);

	// -- -- ���Δ�r -- -- //
	bool value;;
	value = cache.CalcHankakuByFont(c);

	// -- -- �L���b�V���X�V -- -- //
	cache.SetCache(c,value);

	return cache.GetCache(c);
}






