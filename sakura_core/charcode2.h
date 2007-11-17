//2007.09.13 kobake �쐬

#pragma once

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ����֐�                            //
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
//                           �萔                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�萔�̑f (���ڎg�p�͍T���Ă�������)
#define TAB_ 				'\t'
#define SPACE_				' '
#define CR_					'\015'
#define LF_					'\012'
#define ESC_				'\x1b'
#define CRLF_				"\015\012"
#define LFCR_				"\012\015"

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
	static const char LFCR[] = LFCR_;

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
	static const wchar_t LFCR[] = LTEXT(LFCR_);

	//���� (BREGEXP)
	//$$ UNICODE�ł̉��f���~�^�Bbregonig�̎d�l���悭�킩��Ȃ��̂ŁA�Ƃ肠��������Ȓl�ɂ��Ă܂��B
	static const wchar_t BREGEXP_DELIMITER = (wchar_t)0xFFFF;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         ����֐�                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //




//Oct. 31, 2000 JEPRO  TeX Keyword �̂��߂�'\'��ǉ�
//Nov.  9, 2000 JEPRO  HSP Keyword �̂��߂�'@'��ǉ�
//Oct. 18, 2007 kobake UNICODE�p�ɏ�������
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


//UNICODE����֐��Q
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

	//!���p����(�c�������`)���ǂ�������
	bool isHankaku(wchar_t wc);

	//!�S�p����(�����`)���ǂ�������
	inline bool isZenkaku(wchar_t wc)
	{
		return !isHankaku(wc);
	}

	//!�S�p�X�y�[�X���ǂ�������
	inline bool isZenkakuSpace(wchar_t wc)
	{
		return wc == 0x3000; //L'�@'
	}

	//!���䕶���ł��邩�ǂ���
	bool isControlCode(wchar_t wc);

	//!���s�����ł��邩�ǂ���
	inline bool isLineDelimiter(wchar_t wc)
	{
		return wc==CR || wc==LF;
	}

	//!�P��̋�؂蕶���ł��邩�ǂ���
	inline bool isWordDelimiter(wchar_t wc)
	{
		return wc==SPACE || wc==TAB || isZenkakuSpace(wc);
	}

	//!�C���f���g�\���v�f�ł��邩�ǂ����BbAcceptZenSpace: �S�p�X�y�[�X���܂߂邩�ǂ���
	inline bool isIndentChar(wchar_t wc,bool bAcceptZenSpace)
	{
		if(wc==TAB || wc==SPACE)return true;
		if(bAcceptZenSpace && isZenkakuSpace(wc))return true;
		return false;
	}

	//!�󔒂��ǂ���
	inline bool isBlank(wchar_t wc)
	{
		return wc==TAB || wc==SPACE || isZenkakuSpace(wc);
	}

	//!CPP�L�[���[�h�Ŏn�܂��Ă���� true
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

	//!�t�@�C�����Ɏg���镶���ł��邩�ǂ���
	inline bool isValidFilenameChar(const wchar_t* pData, size_t nIndex)
	{
		static const wchar_t* table = L"<>?\"|*";

		wchar_t wc = pData[nIndex];
		if(wcschr(table,wc)!=NULL)return false; //table���̕������܂܂�Ă�����A�_���B
		else return true;
	}

	//!�^�u�\���Ɏg���镶�����ǂ���
	inline bool isTabAvailableCode(wchar_t wc)
	{
		//$$�v����
		if(wc==L'\0')return false;
		if(wc==L'\r')return false;
		if(wc==L'\n')return false;
		if(wc==L'\t')return false;
		return true;
	}

	//! ���p�J�i���ǂ���
	inline bool isHankakuKatakana(wchar_t c)
	{
		//�Q�l: http://ash.jp/code/unitbl1.htm
		return c>=0xFF61 && c<=0xFF9F;
	}

	//! �S�p�L�����ǂ���
	inline bool isZenkakuKigou(wchar_t c)
	{
		//$ ���ɂ��S�p�L���͂���Ǝv�����ǁA�Ƃ肠����ANSI�Ŏ���̔���𓥏P�B�p�t�H�[�}���X�����B
		static const wchar_t* table=L"�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~�����������������������������������������������������������������������������������������������������������ȁɁʁˁ́́΁ځہ܁݁ށ߁�����������������������";
		return wcschr(table,c)!=NULL;
	}

	//! �Ђ炪�Ȃ��ǂ���
	inline bool isHiragana(wchar_t c)
	{
		return c>=0x3041 && c<=0x3093;
	}

	//! �J�^�J�i���ǂ���
	inline bool isZenkakuKatakana(wchar_t c)
	{
		return c>=0x30A1 && c<=0x30F6;
	}

	//! �M���V���������ǂ���
	inline bool isGreek(wchar_t c)
	{
		return c>=0x0391 && c<=0x03C9;
	}

	//! �L�����������ǂ���
	inline bool isCyrillic(wchar_t c)
	{
		return c>=0x0410 && c<=0x044F;
	}

	//! BOX DRAWING ���� ���ǂ���
	inline bool isBoxDrawing(wchar_t c)
	{
		return c>=0x2500 && c<=0x257F;
	}
}


//ANSI����֐��Q
namespace ACODE
{
	//!���䕶���ł��邩�ǂ���
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

	//!�^�u�\���Ɏg���镶�����ǂ���
	inline bool isTabAvailableCode(char c)
	{
		if(c=='\0')return false;
		if(c<=0x1f)return false;
		if(c>=0x7f)return false;
		return true;
	}

	//!�t�@�C�����Ɏg���镶���ł��邩�ǂ���
	inline bool isValidFilenameChar(const char* pData, size_t nIndex)
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







