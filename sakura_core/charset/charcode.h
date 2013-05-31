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
#ifndef SAKURA_CHARCODE_5A887F7C_8E08_4940_AF65_BD6850C3A7B5_H_
#define SAKURA_CHARCODE_5A887F7C_8E08_4940_AF65_BD6850C3A7B5_H_

//2007.09.13 kobake �쐬

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ����֐�                            //
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
//                           �萔                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// SJIS�̃R�[�h�y�[�W(CP_ACP �ł͖���������g���΂����炭�p���Win�ł������͂��B)	2008/5/12 Uchi
#define CP_SJIS		932


//�萔�̑f (���ڎg�p�͍T���Ă�������)
#define TAB_ 				'\t'
#define SPACE_				' '
#define CR_					'\015'
#define LF_					'\012'
#define ESC_				'\x1b'
#define CRLF_				"\015\012"

//ANSI�萔
namespace ACODE{
	//����
	static const char TAB   = TAB_;
	static const char SPACE = SPACE_;
	static const char CR	= CR_;
	static const char LF	= LF_;
	static const char ESC	= ESC_;

	//������
	static const char CRLF[] = CRLF_;

	//���� (BREGEXP)
	static const wchar_t BREGEXP_DELIMITER = (wchar_t)0xFF;
}

//UNICODE�萔
namespace WCODE{
	//����
	static const wchar_t TAB   = LCHAR(TAB_);
	static const wchar_t SPACE = LCHAR(SPACE_);
	static const wchar_t CR    = LCHAR(CR_);
	static const wchar_t LF    = LCHAR(LF_);
	static const wchar_t ESC   = LCHAR(ESC_);

	//������
	static const wchar_t CRLF[] = LTEXT(CRLF_);

	//���� (BREGEXP)
	//$$ UNICODE�ł̉��f���~�^�Bbregonig�̎d�l���悭�킩��Ȃ��̂ŁA�Ƃ肠��������Ȓl�ɂ��Ă܂��B
	static const wchar_t BREGEXP_DELIMITER = (wchar_t)0xFFFF;

}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ����֐�                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*! �L�[���[�h�L�����N�^ */
extern const unsigned char gm_keyword_char[128];

//Oct. 31, 2000 JEPRO  TeX Keyword �̂��߂�'\'��ǉ�
//Nov.  9, 2000 JEPRO  HSP Keyword �̂��߂�'@'��ǉ�
//Oct. 18, 2007 kobake UNICODE�p�ɏ�������
//Nov. 27, 2010 syat   ���x���P�̂��߃e�[�u���ɕύX
inline bool IS_KEYWORD_CHAR(wchar_t wc)
{
	if(0 <= wc && wc < _countof(gm_keyword_char) && (gm_keyword_char[wc] == CK_CSYM||gm_keyword_char[wc] == CK_UDEF) )
		return true;
	else
		return false;
}


//UNICODE����֐��Q
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

	//!���p����(�c�������`)���ǂ�������
	bool IsHankaku(wchar_t wc);

	//!�S�p����(�����`)���ǂ�������
	inline bool IsZenkaku(wchar_t wc)
	{
		return !IsHankaku(wc);
	}

	//!�S�p�X�y�[�X���ǂ�������
	inline bool IsZenkakuSpace(wchar_t wc)
	{
		return wc == 0x3000; //L'�@'
	}

	//!���䕶���ł��邩�ǂ���
	bool IsControlCode(wchar_t wc);

	//!���s�����ł��邩�ǂ���
	inline bool IsLineDelimiter(wchar_t wc)
	{
		return wc==CR || wc==LF || wc==0x85 || wc==0x2028 || wc==0x2029;
	}

	//!�P��̋�؂蕶���ł��邩�ǂ���
	inline bool IsWordDelimiter(wchar_t wc)
	{
		return wc==SPACE || wc==TAB || IsZenkakuSpace(wc);
	}

	//!�C���f���g�\���v�f�ł��邩�ǂ����BbAcceptZenSpace: �S�p�X�y�[�X���܂߂邩�ǂ���
	inline bool IsIndentChar(wchar_t wc,bool bAcceptZenSpace)
	{
		if(wc==TAB || wc==SPACE)return true;
		if(bAcceptZenSpace && IsZenkakuSpace(wc))return true;
		return false;
	}

	//!�󔒂��ǂ���
	inline bool IsBlank(wchar_t wc)
	{
		return wc==TAB || wc==SPACE || IsZenkakuSpace(wc);
	}

	//!�t�@�C�����Ɏg���镶���ł��邩�ǂ���
	inline bool IsValidFilenameChar(const wchar_t* pData, size_t nIndex)
	{
		static const wchar_t* table = L"<>?\"|*";

		wchar_t wc = pData[nIndex];
		if(wcschr(table,wc)!=NULL)return false; //table���̕������܂܂�Ă�����A�_���B
		else return true;
	}

	//!�^�u�\���Ɏg���镶�����ǂ���
	inline bool IsTabAvailableCode(wchar_t wc)
	{
		//$$�v����
		if(wc==L'\0')return false;
		if(wc==L'\r')return false;
		if(wc==L'\n')return false;
		if(wc==L'\t')return false;
		return true;
	}

	//! ���p�J�i���ǂ���
	inline bool IsHankakuKatakana(wchar_t c)
	{
		//�Q�l: http://ash.jp/code/unitbl1.htm
		return c>=0xFF61 && c<=0xFF9F;
	}

	//! �S�p�L�����ǂ���
	inline bool IsZenkakuKigou(wchar_t c)
	{
		//$ ���ɂ��S�p�L���͂���Ǝv�����ǁA�Ƃ肠����ANSI�Ŏ���̔���𓥏P�B�p�t�H�[�}���X�����B
		// 2009.06.26 syat �u�T�U�i�Ђ炪�ȁj�v�u�R�S�i�J�^�J�i�j�v�u�J�K�i�S�p���_�j�v�u�W�X�Z�i�����j�v�u�[�i�����j�v�����O
		// 2009.10.10 syat ANSI�ł̏C���ɂ��킹�āu�Y�v���L���������ɂ���
		static const wchar_t* table=L"�@�A�B�C�D�E�F�G�H�I�L�M�N�O�P�Q�V�\�]�^�_�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~�����������������������������������������������������������������������������������������������������������ȁɁʁˁ́́΁ځہ܁݁ށ߁�����������������������";
		return wcschr(table,c)!=NULL;
	}

	//! �Ђ炪�Ȃ��ǂ���
	inline bool IsHiragana(wchar_t c)
	{
		// 2009.06.26 syat �u�T�U�v��ǉ�
		return (c>=0x3041 && c<=0x3096) || (c>=0x309D && c<=0x309E);
	}

	//! �J�^�J�i���ǂ���
	inline bool IsZenkakuKatakana(wchar_t c)
	{
		// 2009.06.26 syat �u�R�S�v��ǉ�
		return (c>=0x30A1 && c<=0x30FA) || (c>=0x30FD && c<=0x30FE);
	}

	//! �M���V���������ǂ���
	inline bool IsGreek(wchar_t c)
	{
		return c>=0x0391 && c<=0x03C9;
	}

	//! �L�����������ǂ���
	inline bool IsCyrillic(wchar_t c)
	{
		return c>=0x0410 && c<=0x044F;
	}

	//! BOX DRAWING ���� ���ǂ���
	inline bool IsBoxDrawing(wchar_t c)
	{
		return c>=0x2500 && c<=0x257F;
	}

	//! ��Ǔ_��
	//bool IsKutoten( wchar_t wc );

/* codechecker.h �ֈړ�
	//! ���ʃT���Q�[�g�G���A���H	from ssrc_2004-06-05wchar00703b	2008/5/15 Uchi
	inline bool IsUTF16High( wchar_t c )
	{
		return ( 0xd800 == (0xfc00 & c ));
	}
	//! ���ʃT���Q�[�g�G���A���H	from ssrc_2004-06-05wchar00703b	2008/5/15 Uchi
	inline bool IsUTF16Low( wchar_t c )
	{
		return ( 0xdc00 == (0xfc00 & c ));
	}
*/
}


//ANSI����֐��Q
namespace ACODE
{
	inline bool IsAZ(char c)
	{
		return (c>='A' && c<='Z') || (c>='a' && c<='z');
	}

	//!���䕶���ł��邩�ǂ���
	inline bool IsControlCode(char c)
	{
		unsigned char n=(unsigned char)c;
		if(c==TAB)return false;
		if(c==CR )return false;
		if(c==LF )return false;
		if(n<=0x1F)return true;
		if(n>=0x7F && n<=0x9F)return true;
		if(n>=0xE0)return true;
		return false;
	}

	//!�^�u�\���Ɏg���镶�����ǂ���
	inline bool IsTabAvailableCode(char c)
	{
		if(c=='\0')return false;
		if(c<=0x1f)return false;
		if(c>=0x7f)return false;
		return true;
	}

	//!�t�@�C�����Ɏg���镶���ł��邩�ǂ���
	inline bool IsValidFilenameChar(const char* pData, size_t nIndex)
	{
		static const TCHAR* table = _T("<>?\"|*");
		char c = pData[nIndex];

		//table���̕������܂܂�Ă���
		if(_tcschr(table,c)!=NULL){
			//���ꂪ1�o�C�g������������
			if( nIndex==0 || (nIndex>0 && !_IS_SJIS_1(pData[nIndex-1])) ){
				//�g�����Ⴂ���Ȃ������I
				return false;
			}
		}

		return true;
	}
}

//TCHAR����֐��Q
namespace TCODE
{
	#ifdef _UNICODE
		using namespace WCODE;
	#else
		using namespace ACODE;
	#endif
}

// �������̓��I�v�Z�p�L���b�V���֘A
struct SCharWidthCache {
	// �������p�S�p�L���b�V��
	TCHAR		m_lfFaceName[LF_FACESIZE];
	BYTE		m_bCharWidthCache[0x10000/4];		//16KB �������p�S�p�L���b�V�� 2008/5/16 Uchi
	int			m_nCharWidthCacheTest;				//cache��ꌟ�o
};

enum ECharWidthFontMode {
	CWM_FONT_EDIT,
	CWM_FONT_PRINT,
	CWM_FONT_MAX,
};
enum ECharWidthCacheMode {
	CWM_CACHE_NEUTRAL,
	CWM_CACHE_SHARE,
	CWM_CACHE_LOCAL,
};

// �L���b�V���̏������֐��Q
void SelectCharWidthCache( ECharWidthFontMode fMode, ECharWidthCacheMode cMode );  //<! ���[�h��ύX�������Ƃ�
void InitCharWidthCache( const LOGFONT &lf, ECharWidthFontMode fMode=CWM_FONT_EDIT ); //<! �t�H���g��ύX�����Ƃ�

#endif /* SAKURA_CHARCODE_5A887F7C_8E08_4940_AF65_BD6850C3A7B5_H_ */
/*[EOF]*/
