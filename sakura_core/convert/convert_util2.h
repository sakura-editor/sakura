#pragma once

/*!	@file
	@brief �ϊ����[�e�B���e�B2 - BASE64 Ecode/Decode, UUDecode, Q-printable decode

	@author 
*/

/*
	Copyright (C)

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

#include "parse/CWordParse.h"




// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
//    Quoted-Printable �f�R�[�h
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//


inline ACHAR _GetHexChar( ACHAR c )
{
	if( (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') ){
		return c;
	}else if( c >= 'a' && c <= 'f' ){
		return  c - ('a' - 'A');
	}else{
		return '\0';
	}
}
inline WCHAR _GetHexChar( WCHAR c )
{
	if( (c >= L'0' && c <= L'9') || (c >= L'A' && c <= L'F') ){
		return c;
	}else if( c >= L'a' && c <= L'f' ){
		return  c - (L'a' - L'A');
	}else{
		return L'\0';
	}
}


/*
	c �̓��͒l�F 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, A, B, C, D, E, F
*/
inline int _HexToInt( ACHAR c )
{
	if( c <= '9' ){
		return c - '0';
	}else{
		return c - 'A' + 10;
	}
}
inline int _HexToInt( WCHAR c )
{
	if( c <= L'9' ){
		return c - L'0';
	}else{
		return c - L'A' + 10;
	}
}


template< class CHAR_TYPE >
int _DecodeQP( const CHAR_TYPE* pS, const int nLen, char* pDst )
{
	const CHAR_TYPE *pr;
	char *pw;
	int ninc_len;

	pr = pS;
	pw = pDst;

	while( pr < pS + nLen ){
		/* =XX �̌`���łȂ��������f�R�[�h */
		if( sizeof(CHAR_TYPE) == 2 ){
			if( *pr != L'=' ){
				*pw = static_cast<char>( *pr );
				pw += 1;
				pr += 1;
				continue;
			}
		}else{
			if( *pr != '=' ){
				*pw = static_cast<char>( *pr );
				pw += 1;
				pr += 1;
				continue;
			}
		}

		/* =XX �̕������f�R�[�h */
		ninc_len = 1;   // '=' �̕����̃C���N�������g�B
		if( pr + 2 < pS + nLen ){
			// �f�R�[�h���s����
			CHAR_TYPE c1, c2;
			c1 = _GetHexChar(pr[1]);
			c2 = _GetHexChar(pr[2]);
			if( c1 != 0 && c2 != 0 ){
				*pw = static_cast<char>(_HexToInt(c1) << 4) | _HexToInt(c2);
				++pw;
			}else{
				pw[0] = '=';
				pw[1] = static_cast<char>(pr[1] & 0x00ff);
				pw[2] = static_cast<char>(pr[2] & 0x00ff);
				pw += 3;
			}
			ninc_len += 2;
			// �����܂ŁB
		}
		pr += ninc_len;
	}

	return pw - pDst;
}







// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// BAASE64 �̃G���R�[�h/�f�R�[�h
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//

extern const uchar_t TABLE_BASE64CharToValue[];
extern const char TABLE_ValueToBASE64Char[];


// BASE64���� <-> ���l
template< class CHAR_TYPE >
inline uchar_t Base64ToVal( const CHAR_TYPE c ){
	int c_ = c;
	return static_cast<uchar_t>((c_ < 0x80)? TABLE_BASE64CharToValue[c_] : -1);
}
template< class CHAR_TYPE >
inline CHAR_TYPE ValToBase64( const char v ){
	int v_ = v;
	return static_cast<CHAR_TYPE>((v_ < 64)? TABLE_ValueToBASE64Char[v_] : -1);
}


#if 0
/*
	Bas64������̖������K�؂��ǂ������`�F�b�N
	
	���́FBASE64 ������B
*/
template< class CHAR_TYPE >
bool CheckBase64Padbit( const CHAR_TYPE *pSrc, const int nSrcLen )
{
	bool bret = true;

	if( nSrcLen < 1 ){
		return false;
	}

	/* BASE64�����̖����ɂ��āF
		ooxx xxxx   ooxx oooo                   -> 1 byte(s)
		ooxx xxxx   ooxx xxxx   ooxx xxoo          -> 2 byte(s)
		ooxx xxxx   ooxx xxxx   ooxx xxxx   ooxx xxxx -> 3 byte(s)
	*/
	
	switch( nSrcLen % 4 ){
	case 0:
		break;
	case 1:
		bret = false;
		break;
	case 2:
		if( (Base64ToVal(pSrc[nSrcLen-1]) & 0x0f) != 0 ){
			bret = false;
		}
		break;
	case 3:
		if( (Base64ToVal(pSrc[nSrcLen-1]) & 0x03) != 0 ){
			bret = false;
		}
		break;
	}
	return bret;
}
#endif

/*!
	BASE64 �f�R�[�h���s�֐�

	�O�̎������Q�l�ɁB
	������ BASE64 ���͕���������肵�Ă���B
*/
template< class CHAR_TYPE >
int _DecodeBase64( const CHAR_TYPE *pSrc, const int nSrcLen, char *pDest )
{
	long lData;
	int nDesLen;
	int sMax;
	int nsrclen = nSrcLen;

	// ������̍Ō�̃p�b�h���� '=' �𕶎��񒷂Ɋ܂߂Ȃ��悤�ɂ��鏈��
	{
		int i = 0;
		bool bret;
		for( ; i < nsrclen; i++ ){
			if( sizeof(CHAR_TYPE) == 2 ){
				bret = ( pSrc[nsrclen-1-i] == L'=' );
			}else{
				bret = ( pSrc[nsrclen-1-i] == '=' );
			}
			if( bret != true ){
				break;
			}
		}
		nsrclen -= i;
	}

	nDesLen = 0;
	for( int i = 0; i < nsrclen; i++ ){
		if( i < nsrclen - (nsrclen % 4) ){
			sMax = 4;
		}else{
			sMax = (nsrclen % 4);
		}
		lData = 0;
		for( int j = 0; j < sMax; j++ ){
			long k = Base64ToVal( pSrc[i + j] );
			lData |= k << ((4 - j - 1) * 6);
		}
		for( int j = 0; j < (sMax * 6)/ 8 ; j++ ){
			pDest[nDesLen] = static_cast<char>((lData >> (8 * (2 - j))) & 0x0000ff);
			nDesLen++;
		}
		i+= 3;
	}
	return nDesLen;
}


/*!
	BASE64 �G���R�[�h���s�֐�

	�O�̎������Q�l�ɁB
	�p�b�h�����Ȃǂ͕t�����Ȃ��B�G���[�`�F�b�N�Ȃ��B
*/
template< class CHAR_TYPE >
int _EncodeBase64( const char *pSrc, const int nSrcLen, CHAR_TYPE *pDest )
{
	const unsigned char *psrc;
	unsigned long lDataSrc;
	int i, j, k, n, v;
	int nDesLen;

	psrc = reinterpret_cast<const unsigned char *>(pSrc);
	nDesLen = 0;
	for( i = 0; i < nSrcLen; i += 3 ){
		lDataSrc = 0;
		if( nSrcLen - i < 3 ){
			n = nSrcLen % 3;
			j = (n * 4 + 2) / 3;  // �[���؂�グ
		}else{
			n = 3;
			j = 4;
		}
		// n ����G���R�[�h���钷��
		// j �G���R�[�h���BASE64������
		for( k = 0; k < n; k++ ){
			lDataSrc |=
				static_cast<unsigned long>(psrc[i + k]) << ((n - k - 1) * 8);
		}
		// �p�b�h�r�b�g�t���BlDataSrc �̒����� 6*j �ɂȂ�悤�ɒ��߂���B
		lDataSrc <<= j * 6 - n * 8;
		// �G���R�[�h���ď������ށB
		for( k = 0; k < j; k++ ){
			v = static_cast<int>((lDataSrc >> (6 * (j - k - 1))) & 0x0000003f);
			pDest[nDesLen] = ValToBase64<CHAR_TYPE>( v );
			nDesLen++;
		}
	}
	return nDesLen;
}






// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//
// UU �f�R�[�h
//
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//




/*
	Unix-to-Unix �̂���

egin <permission> <file name>
begin
<encoded data>

end

<permission>�F
	�t�@�C���������Ɏg���p�[�~�b�V�����̒l
	�iWindows�ł̓p�[�~�b�V���������݂��Ȃ��H�̂ŁA600 �܂��� 666 ��p����j

<file name>�F
	�t�@�C���������Ɏg���t�@�C����

<encoded data>�F
	�E�o�C�i���f�[�^��3�o�C�g�����o���A�����3�o�C�g��MSB����LSB�ւƕ��ׂ�
	�@24�r�b�g���̃f�[�^�������4�������AMSB���珇��6�r�b�g�����o���B
	�@���o�������ꂼ��̒l�� 0x20(�󔒕���)�����Z��7�r�b�gASCII�����ɕϊ����A
	�@���o�������ɏ�������ł����B
	�E�f�[�^����3�̔{���ɂȂ��Ă��Ȃ��ꍇ�́A0�Ńp�f�B���O����3�̔{���ƂȂ�悤���߂���B
	�E�s�̍ŏ��ɂ́A���̍s�ɉ��o�C�g���̃f�[�^�����邩�̏����������ށB
	�E1�s�ɂ�45�o�C�g���̃f�[�^�i60�����j���������ނ̂�����Łi���܂�H�j�A�Ō�̍s�ȊO�́A
	�@"M"�i45+0x20�j���s�̐擪�ƂȂ�B
	�E���������ꂽ�f�[�^�́A0�o�C�g�̍s�ŏI������B
	�E�s���̋󔒂��폜����Q�[�g�E�F�C�ɑΏ����邽�߁A�󔒂́A"~"(0x7E)�܂���"`"(0x60)�������Ɏg���B
*/




inline BYTE _UUDECODE_CHAR( WCHAR c )
{
	BYTE c_ = (c & 0xff);
	if( c_ == L'`' || c_ == L'~' ){
		c_ = L' ';
	}
	return static_cast<BYTE>((static_cast<BYTE>(c_) - 0x20) & 0x3f);
}
inline BYTE _UUDECODE_CHAR( ACHAR c )
{
	if( c == '`' || c == '~' ){
		c = ' ';
	}
	return static_cast<BYTE>((static_cast<BYTE>(c) - 0x20) & 0x3f);
}



/*
	UU �f�R�[�_�[�i��s�������s����o�[�W�����j

	@param[in] nSrcLen	�K���A4�̔{���ł��邱�ƁB
	@param[in] pDest	�K���A(nSrcLen / 4) * 3 �ȏ�̃o�b�t�@���m�ۂ���Ă��邱�ƁB

	@return ��s�����f�R�[�h�������ʓ���ꂽ���f�[�^�̃o�C�g��
	        �������񂾃f�[�^���߂�l�����傫���Ƃ�������̂Œ��ӁB
*/
template< class CHAR_TYPE >
int _DecodeUU_line( const CHAR_TYPE *pSrc, const int nSrcLen, char *pDest )
{
	unsigned long lDataDes;
	const CHAR_TYPE *pr;

	if( nSrcLen < 1 ){
		return 0;
	}

	pr = pSrc+1;  // �擪�̕����iM(0x20+45)�Ȃǁj���΂�
	int i = 0;
	int j = 0;
	int k = 0;
	for( ; i < nSrcLen; i += 4 ){
		lDataDes = 0;
		for( j = 0; j < 4; ++j ){
			lDataDes |= _UUDECODE_CHAR(pr[i+j]) << ((4 - j - 1) * 6);
		}
		for( j = 0; j < 3; ++j ){
			pDest[k + j] = (char)((lDataDes >> ((3 - j - 1) * 8)) & 0x000000ff);
		}
		k += 3;
	}

	return _UUDECODE_CHAR(pSrc[0]); // 1�s�����f�R�[�h�����Ƃ��ɓ����鐶�f�[�^�̃o�C�g�����擾
}

/*!
	UU�G���R�[�h�̃w�b�_�[���������
*/
template< class CHAR_TYPE >
bool CheckUUHeader( const CHAR_TYPE *pSrc, const int nLen, TCHAR *pszFilename )
{
	using namespace WCODE;

	const CHAR_TYPE *pr, *pr_end;
	CHAR_TYPE *pwstart;
	int nwlen, nstartidx;
	CHAR_TYPE pszSplitChars[16];

	if( sizeof(CHAR_TYPE) == 2 ){
		// �X�y�[�X�܂��̓^�u����؂蕶��
		pszSplitChars[0] = L' ';
		pszSplitChars[1] = L'\t';
		pszSplitChars[2] = L'\0';
	}else{
		// �X�y�[�X�܂��̓^�u����؂蕶��
		pszSplitChars[0] = ' ';
		pszSplitChars[1] = '\t';
		pszSplitChars[2] = '\0';
	}
	

	if( nLen < 1 ){
		if( pszFilename ){
			pszFilename[0] = _WINT('\0');
		}
		return false;
	}

	// �擪�̋󔒁E���s�������X�L�b�v
	for( nstartidx = 0; nstartidx < nLen; ++nstartidx ){
		CHAR_TYPE c = pSrc[nstartidx];
		if( sizeof(CHAR_TYPE) == 2 ){
			if( c != L'\r' && c != L'\n' && c != L' ' && c != L'\t' ){
				break;
			}
		}else{
			if( c != '\r' && c != '\n' && c != ' ' && c != '\t' ){
				break;
			}
		}
	}

	pr = pSrc + nstartidx;
	pr_end = pSrc + nLen;

	// �w�b�_�[�̍\��
	// begin  755  <filename>

	/* begin ���擾 */

	pr += CWordParse::GetWord( pr, pr_end-pr, pszSplitChars, &pwstart, &nwlen );
	if( nwlen != 5 ){
		// error.
		return false;
	}
	if( sizeof(CHAR_TYPE) == 2 ){
		if( wcsncmp(pwstart, L"begin", 5) != 0 ){
			// error.
			return false;
		}
	}else{
		if( strncmp(reinterpret_cast<const char*>(pwstart), "begin", 5) != 0 ){
			// error.
			return false;
		}
	}

	/* 3����8�i���iUnix �V�X�e���̃p�[�~�b�V�����j���擾 */

	pr += CWordParse::GetWord( pr, pr_end-pr, pszSplitChars, &pwstart, &nwlen );
	if( nwlen != 3 ){
		// error.
		return false;
	}
	for( int i = 0; i < nwlen; i++ ){
		if( sizeof(CHAR_TYPE) == 2 ){
			// WCHAR �̏ꍇ�̏���
			if( !iswdigit(pwstart[i]) || (pwstart[i] == L'8' || pwstart[i] == L'9') ){
				// error.
				return false;
			}
		}else{
			// ACHAR �̏ꍇ�̏���
			if( !isdigit(pwstart[i]) || (pwstart[i] == '8' || pwstart[i] == '9') ){
				// error.
				return false;
			}
		}
	}

	/* �����o���p�̃t�@�C�������擾 */

	pr += CWordParse::GetWord( pr, pr_end-pr, pszSplitChars, &pwstart, &nwlen );
	// �����̋󔒁E���s�������X�L�b�v
	for( ; nwlen > 0; --nwlen ){
		CHAR_TYPE c = pwstart[nwlen-1];
		if( sizeof(CHAR_TYPE) == 2 ){
			if( c != L'\r' && c != L'\n' && c != L' ' && c != L'\t' ){
				break;
			}
		}else{
			if( c != '\r' && c != '\n' && c != ' ' && c != '\t' ){
				break;
			}
		}
	}
	if( nwlen < 1 || nwlen + 1  > _MAX_PATH ){
		// error.
		return false;
	}
	// �t�@�C�������i�[
	if( pszFilename ){
		strtotcs( pszFilename, pwstart, nwlen );
		pszFilename[nwlen] = _WINT('\0');
	}

	return true;
}


/*!
	UU �t�b�^�[���m�F
*/
template< class CHAR_TYPE >
bool CheckUUFooter( const CHAR_TYPE *pS, const int nLen )
{
	int nstartidx;
	const CHAR_TYPE* psrc;
	int nsrclen;
	int i;

	// �t�b�^�[�̍\��
	// end
	// �� ��s�̓t�b�^�[�Ɋ܂߂Ȃ��B

	// �擪�̉��s�E�󔒕������X�L�b�v
	for( nstartidx = 0; nstartidx < nLen; ++nstartidx ){
		CHAR_TYPE c = pS[nstartidx];
		if( sizeof(CHAR_TYPE) == 2 ){
			// WCHAR �̏ꍇ�̏���
			if( c != L'\r' && c != L'\n' && c != L' ' && c != L'\t' ){
				break;
			}
		}else{
			// ACHAR �̏ꍇ�̏���
			if( c != '\r' && c != '\n' && c != ' ' && c != '\t' ){
				break;
			}
		}
	}

	psrc = pS + nstartidx;
	nsrclen = nLen - nstartidx;
	i = 0;

	if( nsrclen < 3 ){
		return false;
	}
	if( sizeof(CHAR_TYPE) == 2 ){
		if( wcsncmp(&pS[nstartidx], L"end", 3) != 0 ){
			// error.
			return false;
		}
	}else{
		if( strncmp(reinterpret_cast<const char*>(&pS[nstartidx]), "end", 3) != 0 ){
			// error.
			return false;
		}
	}
	i += 3;

	// end �̌オ�󔒕����΂���ł��邱�Ƃ��m�F
	for( ; i < nsrclen; ++i ){
		CHAR_TYPE c = psrc[i];
		if( sizeof(CHAR_TYPE) == 2 ){
			// WCHAR �̏ꍇ�̏���
			if( c != L'\r' && c != L'\n' && c != L' ' && c != L'\t' ){
				return false;
			}
		}else{
			// ACHAR �̏ꍇ�̏���
			if( c != '\r' && c != '\n' && c != ' ' && c != '\t' ){
				return false;
			}
		}
	}

	return true;
}
