/*!	@file
	@brief �����R�[�h�F���E���ʎx���֐����C�u����

	@author Sakura-Editor collaborators
	@date 1998/03/06 �V�K�쐬
	@date 2006/03/06 ���̕ύX�i�����F�����R�[�h�萔�̒�`�j
	@date 2007/03/19 ���̉���i�����F�����R�[�h�F�����C�u�����j
*/
/*
	Copyright (C) 2006, D. S. Koba, genta
	Copyright (C) 2007

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
#include "charset/codechecker.h"
#include "mem/CMemory.h"
#include "convert/convert_util2.h"
#include "charset/codeutil.h"
#include "charset/charcode.h"
#include <algorithm>



/* =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


                         �f�[�^�\�ƕϊ��⏕


*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


*/


/*
	���ʃe�[�u��  UTF-7 �̂ǂ̃Z�b�g�̕�����

	@author D. S. Koba
	@date 2007.04.29 UTF-7 �Z�b�g O �̏���ǉ� by rastiv.
*/
// !"#$%&*;<=>@[]^_`{|}
const char TABLE_IsUtf7Direct[] = {
	0, 0, 0, 0, 0, 0, 0, 0,  //00-07:
	0, 1, 1, 0, 0, 1, 0, 0,  //08-0f:TAB, LF, CR
	0, 0, 0, 0, 0, 0, 0, 0,  //10-17:
	0, 0, 0, 0, 0, 0, 0, 0,  //18-1f:
	1, 2, 2, 2, 2, 2, 2, 1,  //20-27:SP, `'`
	1, 1, 2, 0, 1, 1, 1, 1,  //28-2f:(, ), `,`, -, ., /
	1, 1, 1, 1, 1, 1, 1, 1,  //30-37:0 - 7
	1, 1, 1, 2, 2, 2, 2, 1,  //38-3f:8, 9, :, ?
	2, 1, 1, 1, 1, 1, 1, 1,  //40-47:A - G
	1, 1, 1, 1, 1, 1, 1, 1,  //48-4f:H - O
	1, 1, 1, 1, 1, 1, 1, 1,  //50-57:P - W
	1, 1, 1, 2, 0, 2, 2, 2,  //58-5f:X, Y, Z
	2, 1, 1, 1, 1, 1, 1, 1,  //60-67:a - g
	1, 1, 1, 1, 1, 1, 1, 1,  //68-6f:h - o
	1, 1, 1, 1, 1, 1, 1, 1,  //70-77:p - w
	1, 1, 1, 2, 2, 2, 0, 0,  //78-7f:x, y, z
};


#if 0 // ���g�p�����ǁA�Q�l�̂��߂ɏ����c��

/*
	JIS �G�X�P�[�v�V�[�P���X�f�[�^
	@author D. S. Koba
*/
const char JISESCDATA_ASCII[]				= "\x1b""(B";
const char JISESCDATA_JISX0201Latin[]		= "\x1b""(J";
const char JISESCDATA_JISX0201Latin_OLD[]	= "\x1b""(H";
const char JISESCDATA_JISX0201Katakana[]	= "\x1b""(I";
const char JISESCDATA_JISX0208_1978[]		= "\x1b""$@";
const char JISESCDATA_JISX0208_1983[]		= "\x1b""$B";
const char JISESCDATA_JISX0208_1990[]		= "\x1b""&@""\x1b""$B";

// ������ enumJISEscSeqType �Ɉˑ� (charcode.h �ɂĒ�`����Ă���)
const int TABLE_JISESCLEN[] = {
	0,		// JISESC_UNKNOWN
	3,		// JISESC_ASCII
	3,		// JISESC_JISX0201Latin
	3,		// JISESC_JISX0201Latin_OLD
	3,		// JISESC_JISX0201Katakana
	3,		// JISESC_JISX0208_1978
	3,		// JISESC_JISX0208_1983
	6,		// JISESC_JISX0208_1990
};
const char* TABLE_JISESCDATA[] = {
	"",
	JISESCDATA_ASCII,
	JISESCDATA_JISX0201Latin,
	JISESCDATA_JISX0201Latin_OLD,
	JISESCDATA_JISX0201Katakana,
	JISESCDATA_JISX0208_1978,
	JISESCDATA_JISX0208_1983,
	JISESCDATA_JISX0208_1990,
};

#endif
















/* =*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


                         �����R�[�h���ʎx��


*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*


*/


/*
	SJIS �̂���.

	��P�o�C�g |  1000 0001(0x81)         |   1110 0000(0xE0)         |   1010 0001(0xA1)
	           | �` 1001 1111(0x9F)       |  �` 1110 1111(0xEF)       |  �` 1101 1111(0xDF)
	           | �� SJIS �S�p�������ȃJ�i |  �� SJIS �S�p�����J�i���� |  �� ���p�J�i
	-----------+--------------------------+---------------------------+-------------------------
	��Q�o�C�g |        0100 0000(0x40)  �`  1111 1100(0xFC)          |      ----
	           |         ������ 0111 1111(0x7F) �͏���.               |

	�Q�l�F�u��G-PROJECT�� -���{�ꕶ���R�[�h�̔��ʁvhttp://www.gprj.net/dev/tips/other/kanji.shtml
	      �u�~�P�l�R�̕����R�[�h�̕����vhttp://mikeneko.creator.club.ne.jp/~lab/kcode/index.html
*/

/*!
	SJIS �������`�F�b�N

	@param[out] pnCharset �m�F���������R�[�h�̎�ʂ��i�[�����

	@return �m�F���������̒���
*/
int CheckSjisChar( const char* pS, const int nLen, ECharSet *peCharset )
{
	unsigned char uc;

	if( 0 < nLen ){
		uc = pS[0];
		if( (uc & 0x80) == 0 ){
			// ASCII �܂��̓��[�}��(JIS X 0201 Roman)
			if( peCharset ){
				*peCharset = CHARSET_ASCII7;
			}
			return 1;
		}
		if( IsSjisHankata(static_cast<char>(uc)) ){
			// ���p�J�i(JIS X 0201 Kana)
			if( peCharset ){
				*peCharset = CHARSET_JIS_HANKATA;
			}
			return 1;
		}
		if( 1 < nLen && IsSjisZen(pS) ){
			// SJIS �����E�S�p�J�i����  (JIS X 0208)
			if( peCharset ){
				*peCharset = CHARSET_JIS_ZENKAKU;
			}
			return 2;
		}
		if( peCharset ){
			*peCharset = CHARSET_BINARY;
		}
		return 1;
	}
	return 0;
}




/*
	EUC-JP �̂���.

	��1�o�C�g |   1000 1110(0x8E)   |  1000 1111(0x8F)    |  1010 0001(0xA1) �` 1111 1110(0xFE)
	          |   �� ���p�J�i       |  �� �⏕����        |  �� �������ȃJ�i
	----------+---------------------+---------------------+-------------------------------------
	��2�o�C�g |  1010 0001(0xA1)    |   1010 0001(0xA1)   |      1010 0001(0xA1)
	          | �` 1101 1111(0xDF)  |  �` 1111 1110(0xFE) |     �` 1111 1110(0xFE)
	----------+---------------------+---------------------+-------------------------------------
	��3�o�C�g |        ----         |   1010 0001(0xA1)   |        ----
	          |                     |  �` 1111 1110(0xFE) |

	�Q�l�F�u��G-PROJECT�� -���{�ꕶ���R�[�h�̔��ʁvhttp://www.gprj.net/dev/tips/other/kanji.shtml
	      �u�~�P�l�R�̕����R�[�h�̕����vhttp://mikeneko.creator.club.ne.jp/~lab/kcode/index.html
*/

/*!
	EUC-JP �������`�F�b�N

	@sa CheckSjisChar()

	@date 2006.09.23 EUCJP ���p�J�^�J�i���ʂ��Ԉ���Ă����̂��C���Dgenta
*/
int CheckEucjpChar( const char* pS, const int nLen, ECharSet *peCharset )
{
	unsigned char uc;

	if( 0 < nLen ){
		uc = pS[0];
		if( (uc & 0x80) == 0 ){
			// ASCII �܂��̓��[�}���ł�.  (JIS X 0201 Roman.)
			if( peCharset ){
				*peCharset = CHARSET_ASCII7;
			}
			return 1;
		}
		if( 1 < nLen ){
			if( IsEucjpZen(pS) ){
				// EUC-JP �����E���ȃJ�i �ł�.  (JIS X 0208.)
				if( peCharset ){
					*peCharset = CHARSET_JIS_ZENKAKU;
				}
				return 2;
			}
			if( IsEucjpHankata(pS) ){
				// ���p�J�i�ł�.  (JIS X 0201 Kana.)
				if( peCharset ){
					*peCharset = CHARSET_JIS_HANKATA;
				}
				return 2;
			}
			if( 2 < nLen ){
				if( IsEucjpSupplemtal(pS) ){
					// EUC-JP �⏕�����ł�.  (JIS X 0212.)
					if( peCharset ){
						*peCharset = CHARSET_JIS_SUPPLEMENTAL;
					}
					return 3;
				}
			}
		}
		if( peCharset ){
			*peCharset = CHARSET_BINARY;
		}
		return 1;
	}
	return 0;
}






/*!
	JIS �� �G�X�P�[�v����������o����

	@param [in]  pS         �����f�[�^
	@param [in]  nLen       �����f�[�^��
	@param [out] peEscType  ���o���ꂽ�G�X�P�[�v������̎��

	@retval n == 0 �����f�[�^���Ȃ�
	@retval n > 0 �G�X�P�[�v�V�[�P���X�̒���
	@retval n < 0 �G�X�P�[�v�V�[�P���X�����o����Ȃ�����

	@note
		�߂�l���[�����傫���ꍇ�Ɍ���C*pnEscType ���X�V�����D\n
		pnEscType �� NULL �ł��ǂ��D\n
*/
int DetectJisEscseq( const char* pS, const int nLen, EMyJisEscseq* peEscType )
{
	const char *pr, *pr_end;
	int expected_esc_len;
	EJisEscseq ejisesc;
	EMyJisEscseq emyjisesc;

	if( nLen < 1 ){
		*peEscType = MYJISESC_NONE;
		return 0;
	}

	ejisesc = JISESC_UNKNOWN;
	expected_esc_len = 0;
	pr = const_cast<char*>( pS );
	pr_end = pS + nLen;

	if( pr[0] == ACODE::ESC ){
		expected_esc_len++;
		pr++;
		if( pr + 1 < pr_end ){
			expected_esc_len += 2;
			if( pr[0] == '(' ){
				if( pr[1] == 'B' ){
					ejisesc = JISESC_ASCII;				// ESC ( B  -  ASCII
				}else if( pr[1] == 'J'){
					ejisesc = JISESC_JISX0201Latin;		// ESC ( J  -  JIS X 0201 ���e��
				}else if( pr[1] == 'H'){
					ejisesc = JISESC_JISX0201Latin_OLD;	// ESC ( H  -  JIS X 0201 ���e��
				}else if( pr[1] == 'I' ){
					ejisesc = JISESC_JISX0201Katakana;	// ESC ( I  -  JIS X 0201 �Љ���
				}
			}else if( pr[0] == '$' ){
				if( pr[1] == 'B' ){
					ejisesc = JISESC_JISX0208_1983;		// ESC $ B  -  JIS X 0208-1983
				}else if( pr[1] == '@' ){
					ejisesc = JISESC_JISX0208_1978;		// ESC $ @  -  JIS X 0208-1978  (��JIS)
				}
			}
		}else if( pr + 4 < pr_end ){
			expected_esc_len += 5;
			if( 0 == strncmp( pr, "&@\x1b$B", 5 ) ){
				ejisesc = JISESC_JISX0208_1990;			// ESC & @ ESC $ B  -  JIS X 0208-1990
			}
		}
	}

	// ���o���ꂽJIS �G�X�P�[�v�V�[�P���X���ʂh�c��
	// ������ JIS �G�X�P�[�v�V�[�P���X���ʂh�c�ɕϊ�
	switch( ejisesc ){
	case JISESC_ASCII:
	case JISESC_JISX0201Latin_OLD:
	case JISESC_JISX0201Latin:
		emyjisesc = MYJISESC_ASCII7;
		break;
	case JISESC_JISX0201Katakana:
		emyjisesc = MYJISESC_HANKATA;
		break;
	case JISESC_JISX0208_1978:
	case JISESC_JISX0208_1990:
	case JISESC_JISX0208_1983:
		emyjisesc = MYJISESC_ZENKAKU;
		break;
	default:
		if( 0 < expected_esc_len ){
			emyjisesc = MYJISESC_UNKNOWN;
		}else{
			emyjisesc = MYJISESC_NONE;
		}
	}

	*peEscType = emyjisesc;
	return expected_esc_len;
}



/*!
	JIS ��������`�F�b�N

	���̃G�X�P�[�v�V�[�P���X���玟�̃G�X�P�[�v�V�[�P���X�ɕς��Ԃ��u���b�N�ƕ֋X�I�ɌĂ�ł��܂��B
*/
int _CheckJisAnyPart(
		const char *pS,			// [in]    �`�F�b�N�ΏۂƂȂ�o�b�t�@�|�C���^
		const int nLen,			// [in]    �`�F�b�N�ΏۂƂȂ�o�b�t�@�̒���
		const char **ppNextChar,		// [out]   ���̃G�X�P�[�v�V�[�P���X������̎��̕����ւ̃|�C���^
								//       �܂�A���Ɍ������J�n���镶����i�擪�̃G�X�P�[�v�V�[�P���X���܂߂Ȃ��j�ւ̃|�C���^
		EMyJisEscseq *peNextEsc,// [out]   ���̃G�X�P�[�v�V�[�P���X�̎��
		int *pnErrorCount,		// [out]   �u���b�N���̕s��������
		const int nType			// [in]    ���̃G�X�P�[�v�V�[�P���X�̎��
)
{
	EMyJisEscseq emyesc = MYJISESC_NONE;
	int nesclen;
	int nerror_cnt;
	const char *pr, *pr_end;

	if( nLen < 1 ){
		*peNextEsc = MYJISESC_NONE;
		*ppNextChar = const_cast<char*>( pS );
		*pnErrorCount = 0;
		return 0;
	}

	nerror_cnt = 0;
	nesclen = 0;
	pr = pS;
	pr_end = pS + nLen;

	for( ; pr < pr_end; pr++ ){
		nesclen = DetectJisEscseq( pr, pr_end-pr, &emyesc );  // ���̃G�X�P�[�v�V�[�P���X������
		if( emyesc != MYJISESC_NONE || nesclen > 0 ){
			// ���� nesclen �� JIS �G�X�P�[�v�V�[�P���X�i��� emyesc�j����������
			break;
		}
		if( pnErrorCount ){
			switch( nType ){
			case JISCHECK_ASCII7:
				if( !IsAscii7(*pr) ){
					nerror_cnt++;
				}
				break;
			case JISCHECK_HANKATA:
				if( !IsJisHankata(*pr) ){
					nerror_cnt++;
				}
				break;
			case JISCHECK_ZENKAKU:
				if( (pr-pS+1) % 2 == 0 ){
					if( !IsJisZen(pr-1) ){
						nerror_cnt += 2;
					}
				}
				break;
			default:
				if( !IsJis(*pr) ){
					nerror_cnt++;
				}
			}
		}
	}
	if( pnErrorCount ){
		*pnErrorCount = nerror_cnt;
	}

	*peNextEsc = emyesc;
	if( pr < pr_end ){
		*ppNextChar = const_cast<const char*>(pr) + nesclen;
	}else{
		*ppNextChar = const_cast<const char*>(pr_end);
		pr = pr_end;
	}

	return pr - pS;
}














/*
	UTF-16 �T���Q�[�g�����̂���.

	U+10000 ���� U+10FFFF �̕����l a0 �ɑ΂��Ă�,

		a0 = HHHHHHHHHHLLLLLLLLLL  U+10000 �` U+10FFFF
		w1 = 110110HH HHHHHHHH     ��ʃT���Q�[�g�FU+D800 �` U+DBFF
		w2 = 110111LL LLLLLLLL     ���ʃT���Q�[�g�FU+DC00 �` U+DFFF

	1. 0x10000 ������, 20�r�b�g�̕����l a1 (0x00000 �` 0xFFFFF) �ŕ\��������,
	     a1 �� a0 - 0x10000
	2. ��� 10�r�b�g�� w1, ���� 10�r�b�g�� w2 �ɕ���,
	     w1 �� (a1 & 0xFFC0) >> 6
	     w2 ��  a1 & 0x03FF
	3. w1, w2 �̏�� 6�r�b�g���̋󂫗̈��, ���ꂼ�� 110110 �� 110111 �Ŗ��߂�.
	     w1 �� w1 | 0xD800
	     w2 �� w2 | 0xDC00


	U+FFFE, U+FFFF ��, ����`�l.

	�Q�l�����F�uUCS��UTF�vhttp://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm
*/

/*!
	UTF-16 LE/BE �������`�F�b�N�@(�g�ݍ��킹������l���Ȃ�)
*/
int _CheckUtf16Char( const wchar_t* pS, const int nLen, ECharSet *peCharset, const int nOption, const bool bBigEndian )
{
	wchar_t wc1, wc2 = 0;
	int ncwidth;
	ECharSet echarset;

	if( nLen < 1 ){
		return 0;
	}

	echarset = CHARSET_UNI_NORMAL;

	// ������ǂݍ���

	wc1 = pS[0];
	if( bBigEndian == true ){
		wc1 = _SwapHLByte( wc1 );
	}
	if( 1 < nLen ){
		wc2 = pS[1];
		if( bBigEndian == true ){
			wc2 = _SwapHLByte( wc2 );
		}
	}

	if( 2 <= nLen ){

		// �T���Q�[�g�y�A�̊m�F

		if( IsUtf16SurrogHi(wc1) && IsUtf16SurrogLow(wc2) ){
			echarset = CHARSET_UNI_SURROG;
			ncwidth = 2;
			goto EndFunc;
		}
	}

	// �T���Q�[�g�f�Ђ̊m�F

	if( IsUtf16SurrogHi(wc1) || IsUtf16SurrogLow(wc1) ){
		echarset = CHARSET_BINARY;
		ncwidth = 1;
		goto EndFunc;
	}

	// �T���Q�[�g�y�A�łȂ�����
	ncwidth = 1;

	// �񕶎��Ɨ\��R�[�h�|�C���g�̊m�F
	if( nOption != 0 && echarset != CHARSET_BINARY ){
		if( ncwidth == 1 ){
			if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc1) ){
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
		}else if( ncwidth == 2 ){
			wchar32_t wc32_checking = DecodeUtf16Surrog( wc1, wc2 );
			if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc32_checking) ){
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
		}else{
			// �ی�R�[�h
			echarset = CHARSET_BINARY;
			ncwidth = 1;
		}
	}


EndFunc:;
	if( peCharset ){
		*peCharset = echarset;
	}

	return ncwidth;
}







/* -------------------------------------------------------------------------------------------------------------- *
UTF-8�̃R�[�h
�r�b�g��		���e
0xxx xxxx	1�o�C�g�R�[�h�̐擪
110x xxxx	2�o�C�g�R�[�h�̐擪
1110 xxxx	3�o�C�g�R�[�h�̐擪
1111 0xxx	4�o�C�g�R�[�h�̐擪
10xx xxxx	UTF-8 �o�C�g�R�[�h�� 2 �o�C�g�ڈȍ~

UTF-8�̃G���R�[�f�B���O

�r�b�g��                  MSB -         UCS �r�b�g��         - LSB     ��1�o�C�g  ��2�o�C�g  ��3�o�C�g  ��4�o�C�g
\u0�`\u7F         (UCS2)  0000 0000 0000 0000  0000 0000 0aaa bbbb  -> 0aaa bbbb     ---        ---        ---
\u80�`\u7FF       (UCS2)  0000 0000 0000 0000  0000 0aaa bbbb cccc  -> 110a aabb  10bb cccc     ---        ---
\u800�`\uFFFF     (UCS2)  0000 0000 0000 0000  aaaa bbbb cccc dddd  -> 1110 aaaa  10bb bbcc  10cc dddd     ---
\u10000�`\u1FFFFF (UCS4)  0000 0000 000a bbbb  cccc dddd eeee ffff  -> 1111 0abb  10bb cccc  10dd ddee  10ee ffff

�Q�l�����F�uUCS��UTF�vhttp://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm
* --------------------------------------------------------------------------------------------------------------- */

/*!
	UTF-8 �������`�F�b�N�@(�g�ݍ��킹������l���Ȃ�)

	@sa CheckSjisChar()

	@date 2008/11/01 syat UTF8�t�@�C���ŉ��Ă̓��ꕶ�����ǂݍ��߂Ȃ��s����C��
*/
int CheckUtf8Char( const char *pS, const int nLen, ECharSet *peCharset, const bool bAllow4byteCode, const int nOption )
{
	unsigned char c0, c1, c2, c3;
	int ncwidth;
	ECharSet echarset;

	if( nLen < 1 ){
		return 0;
	}

	echarset = CHARSET_UNI_NORMAL;
	c0 = pS[0];

	if( c0 < 0x80 ){	// ��P�o�C�g�� 0aaabbbb �̏ꍇ
		ncwidth = 1;	// �P�o�C�g�R�[�h�ł���
		goto EndFunc;
	}else
	if( 1 < nLen && (c0 & 0xe0) == 0xc0 ){	// ��P�o�C�g��110aaabb�̏ꍇ
		c1 = pS[1];
		// ��Q�o�C�g��10bbcccc�̏ꍇ
		if( (c1 & 0xc0) == 0x80 ){
			ncwidth = 2;	// �Q�o�C�g�R�[�h�ł���
			// ��P�o�C�g��aaabb=0000x�̏ꍇ�i\u80�����ɕϊ������j
			if( (c0 & 0x1e) == 0 ){
				// �f�R�[�h�ł��Ȃ�.(�����ϊ��s�̈�)
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			goto EndFunc;
		}
	}else
	if( 2 < nLen && (c0 & 0xf0) == 0xe0 ){	// ��P�o�C�g��1110aaaa�̏ꍇ
		c1 = pS[1];
		c2 = pS[2];
		// ��Q�o�C�g��10bbbbcc�A��R�o�C�g��10ccdddd�̏ꍇ
		if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 ){
			ncwidth = 3;	// �R�o�C�g�R�[�h�ł���
			// ��P�o�C�g��aaaa=0000�A��Q�o�C�g��bbbb=0xxx�̏ꍇ(\u800�����ɕϊ������)
			if( (c0 & 0x0f) == 0 && (c1 & 0x20) == 0 ){
				// �f�R�[�h�ł��Ȃ�.(�����ϊ��s�̈�)
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			//if( (c0 & 0x0f) == 0x0f && (c1 & 0x3f) == 0x3f && (c2 & 0x3e) == 0x3e ){
			//	// Unicode �łȂ�����(U+FFFE, U+FFFF)
			//	charset = CHARSET_BINARY;
			//	ncwidth = 1;
			//}
			if( bAllow4byteCode == true && (c0 & 0x0f) == 0x0d && (c1 & 0x20) != 0 ){
				// �T���Q�[�g�̈� (U+D800 ���� U+DFFF)
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			goto EndFunc;
		}
	}else
	if( 3 < nLen && (c0 & 0xf8) == 0xf0 ){
		c1 = pS[1];
		c2 = pS[2];
		c3 = pS[3];
		// ��2�o�C�g��10bbcccc�A��3�o�C�g��10ddddee�A��4�o�C�g��10ddddee�̏ꍇ
		if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 && (c3 & 0xc0) == 0x80 ){
			ncwidth = 4;  // �S�o�C�g�R�[�h�ł���
			echarset = CHARSET_UNI_SURROG;  // �T���Q�[�g�y�A�̕����i�������j
			// ��1�o�C�g��abb=000�A��2�o�C�g��bb=00�̏ꍇ�i\u10000�����ɕϊ������j
			if( (c0 & 0x07) == 0 && (c1 & 0x30) == 0 ){
				// �f�R�[�h�ł��Ȃ�.(�����ϊ��s�̈�)
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			// �P�o�C�g�ڂ� 11110xxx=11110100�̂Ƃ��A
			// ���A1111 01xx : 10xx oooo �� x �̂Ƃ���ɒl������Ƃ�
			if( (c0 & 0x04) != 0 && ((c0 & 0x03) != 0 || (c1 & 0x30) != 0) ){
				// �l���傫�����i0x10ffff���傫���j
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			if( bAllow4byteCode == false ){
				echarset = CHARSET_BINARY;
				ncwidth = 1;
			}
			goto EndFunc;
		}
	}

	// �K��O�̃t�H�[�}�b�g
	echarset = CHARSET_BINARY;
	ncwidth = 1;

EndFunc:

	// �񕶎��Ɨ\��R�[�h�|�C���g���`�F�b�N
	if( nOption != 0 && echarset != CHARSET_BINARY ){
		wchar32_t wc32;
		wc32 = DecodeUtf8( reinterpret_cast<const unsigned char*>(pS), ncwidth );
		if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc32) ){
			echarset = CHARSET_BINARY;
			ncwidth = 1;
		}else{
			// �ی�R�[�h
			echarset = CHARSET_BINARY;
			ncwidth = 1;
		}
	}

	if( peCharset ){
		*peCharset = echarset;
	}
	return ncwidth;
}

int CheckUtf8Char2( const char *pS, const int nLen, ECharSet *peCharset, const bool bAllow4byteCode, const int nOption )
{
	unsigned char c0, c1, c2;
	int ncwidth;
	ECharSet echarset;

	ECharSet echarset1;
	int nclen1;

	if( nLen < 1 ){
		return 0;
	}

	nclen1 = CheckUtf8Char( pS, nLen, &echarset1, true, 0 );
	echarset = echarset1;
	c0 = pS[0];
	if( echarset1 == CHARSET_BINARY ){
		if( 1 == nLen && (c0 & 0xe0) == 0xc0 ){	// ��P�o�C�g��110aaabb�̏ꍇ
			echarset = CHARSET_BINARY2;
			ncwidth = 1;
			goto EndFunc;
		}else
		if( 2 == nLen && (c0 & 0xf0) == 0xe0 ){	// ��P�o�C�g��1110aaaa�̏ꍇ
			c1 = pS[1];
			// ��Q�o�C�g��10bbbbcc�A��R�o�C�g��10ccdddd�̏ꍇ
			if( (c1 & 0xc0) == 0x80 ){
				ncwidth = 2;	// �R�o�C�g�R�[�h�̐擪2�o�C�g�ł���
				if( (c0 & 0x0f) == 0 && (c1 & 0x20) == 0 ){
					// �f�R�[�h�ł��Ȃ�.(�����ϊ��s�̈�)
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				//if( (c0 & 0x0f) == 0x0f && (c1 & 0x3f) == 0x3f && (c2 & 0x3e) == 0x3e ){
				//	// Unicode �łȂ�����(U+FFFE, U+FFFF)
				//	charset = CHARSET_BINARY;
				//	ncwidth = 1;
				//}
				if( bAllow4byteCode == true && (c0 & 0x0f) == 0x0d && (c1 & 0x20) != 0 ){
					// �T���Q�[�g�̈� (U+D800 ���� U+DFFF)
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				goto EndFunc;
			}
		}else
		if( 1 == nLen && (c0 & 0xf0) == 0xe0 ){
			echarset = CHARSET_BINARY2;
			ncwidth = 1;
			goto EndFunc;
		}else
		if( 0 < nLen && nLen <= 3 && (c0 & 0xf8) == 0xf0 ){
			if( 1 < nLen ){
				c1 = pS[1];
			}else{
				c1 = 0xbf;
			}
			if( 2 < nLen ){
				c2 = pS[2];
			}else{
				c2 = 0xbf;
			}
			// ��2�o�C�g��10bbcccc�A��3�o�C�g��10ddddee
			if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 ){
				ncwidth = std::max(nLen,3);  // �S�o�C�g�R�[�h�ł���
				echarset = CHARSET_UNI_SURROG;  // �T���Q�[�g�y�A�̕����i�������j
				// ��1�o�C�g��abb=000�A��2�o�C�g��bb=00�̏ꍇ�i\u10000�����ɕϊ������j
				if( (c0 & 0x07) == 0 && (c1 & 0x30) == 0 ){
					// �f�R�[�h�ł��Ȃ�.(�����ϊ��s�̈�)
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				// �P�o�C�g�ڂ� 11110xxx=11110100�̂Ƃ��A
				// ���A1111 01xx : 10xx oooo �� x �̂Ƃ���ɒl������Ƃ�
				if( (c0 & 0x04) != 0 && (c0 & 0x03) != 0 ){
					// �l���傫�����i0x10ffff���傫���j
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				if( bAllow4byteCode == false ){
					echarset = CHARSET_BINARY;
					ncwidth = 1;
				}
				goto EndFunc;
			}
		}
	}else{
		ncwidth = nclen1;
		goto EndFunc;
	}

	// �K��O�̃t�H�[�}�b�g
	echarset = CHARSET_BINARY;
	ncwidth = 1;

EndFunc:

	if( peCharset ){
		*peCharset = echarset;
	}
	return ncwidth;
}

/*
	CESU-8 �����̃`�F�b�N�@(�g�ݍ��킹������l���Ȃ�)
*/
int CheckCesu8Char( const char* pS, const int nLen, ECharSet* peCharset, const int nOption )
{
	ECharSet echarset1, echarset2, eret_charset;
	int nclen1, nclen2, nret_clen;

	if( nLen < 1 ){
		return 0;
	}

	// �P�����ڂ̃X�L����
	nclen1 = CheckUtf8Char( &pS[0], nLen, &echarset1, false, 0 );

	// ���������R�����̏ꍇ
	if( nclen1 < 3 ){
		// echarset == BAINARY �̏ꍇ�́A����Ȃ� nclen1 < 3
		eret_charset = echarset1;
		nret_clen = nclen1;
	}else
	// ���������R�̏ꍇ
	if( nclen1 == 3 ){
		// ����ȂR�o�C�g�������������B

		// �Q�����ڂ̃X�L����
		nclen2 = CheckUtf8Char( &pS[3], nLen-3, &echarset2, false, 0 );

		// &pS[3]����̕��������R�łȂ��� echarset2 �� CHARSET_BINARY �������ꍇ�B
		if( nclen2 != 3 || echarset2 == CHARSET_BINARY ){
			// nclen1 �� echarset1 �����ʂƂ���B
			eret_charset = echarset1;
			nret_clen = nclen1;
			// &pS[0] ����R�o�C�g���T���Q�[�g�Ђ������ꍇ�B
			if( IsUtf8SurrogHi(&pS[0]) || IsUtf8SurrogLow(&pS[0]) ){
				eret_charset = CHARSET_BINARY;
				nret_clen = 1;
			}
			goto EndFunc;
		}

		//    nclen1 == 3 && echarset1 != CHARSET_BINARY
		// && nclen2 == 3 && echarset2 != CHARSET_BINARY �̏ꍇ�B

		// UTF-8�ŃT���Q�[�g�y�A���m�F�B
		if( IsUtf8SurrogHi(&pS[0]) && IsUtf8SurrogLow(&pS[3]) ){
			// CESU-8 �ł��邩�ǂ������`�F�b�N
			eret_charset = CHARSET_UNI_SURROG;
			nret_clen = 6;  // CESU-8 �̃T���Q�[�g�ł���
		}else
		// &pS[0] ����R�o�C�g���T���Q�[�g�Ђ������ꍇ�B
		if( IsUtf8SurrogHi(&pS[0]) || IsUtf8SurrogLow(&pS[0]) ){
			eret_charset = CHARSET_BINARY;
			nret_clen = 1;
		}else
		// �ʏ�̂R�o�C�g����
		{
			eret_charset = echarset1;
			nret_clen = 3;
		}
	}else
	// ���������R���傫���ꍇ
	{  // nclen1 == 4
		// UTF-16 �T���Q�[�g�ɕϊ������̈�
		// 4�o�C�g�R�[�h�͋֎~
		eret_charset = CHARSET_BINARY;
		nret_clen = 1;
	}

EndFunc:;


	// �񕶎��Ɨ\��R�[�h�|�C���g���m�F
	if( nOption != 0 && eret_charset != CHARSET_BINARY ){
		wchar32_t wc32;
		if( nret_clen < 4 ){
			wc32 = DecodeUtf8( reinterpret_cast<const unsigned char*>(pS), nret_clen );
			if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc32) ){
				eret_charset = CHARSET_BINARY;
				nret_clen = 1;
			}
		}else if( nret_clen == 6 ){
			wc32 = DecodeUtf16Surrog(
				static_cast<unsigned short>(DecodeUtf8(reinterpret_cast<const unsigned char*>(&pS[0]), 3) & 0x0000ffff),
				static_cast<unsigned short>(DecodeUtf8(reinterpret_cast<const unsigned char*>(&pS[3]), 3) & 0x0000ffff) );
			if( (nOption & UC_NONCHARACTER) && IsUnicodeNoncharacter(wc32) ){
				eret_charset = CHARSET_BINARY;
				nret_clen = 1;
			}
		}else{
			// �ی�R�[�h
			eret_charset = CHARSET_BINARY;
			nret_clen = 1;
		}
	}

	if( peCharset ){
		*peCharset = eret_charset;
	}
	return nret_clen;
}




/*
	UTF-7 �̂���.

	UTF-7 �Z�b�gD�F�@���p�p�����A'(),-./:?�A����сATAB SP CR LF
	UTF-7 �Z�b�gO�F�@!"#$%&*;<=>@[]^_`{|}
	UTF-7 �Z�b�gB�F�@�p�b�h���������� BASE64 ���� (Modified Base 64)

	1. �Z�b�gD �܂��� �Z�b�gO �ɂ��郆�j�R�[�h������, ������ ASCII �����ŕ\�������.
	2. �Z�b�gD �܂��� �Z�b�gO �ɂȂ����j�R�[�h������, Modified Base 64 ����������, �Z�b�gB ������ɂ���ĕ\�������.
	3. �Z�b�gB �̊J�n��, ASCII ���� '+' �ɂ����, �Z�b�gB ������̏I�[��, �Z�b�gB �ɂȂ������̏o���ɂ���ĔF�������.
	   �Z�b�gB ������̏I�[�����ɂ�, �Z�b�gB �ɂȂ� ASCII ���� '-' �����Ă��悢���ƂɂȂ��Ă���,
	   ���̏I�[������, �f�R�[�_�[�ɂ��, �����莟��폜�����.
	4. �Z�b�gB �J�n�����ł��� ASCII ���� '+' ���̂�, "+-" �Ƃ���������ŕ\�����.

	�Q�l�����F�uUCS��UTF�vhttp://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm
	          �uRFC 2152�vhttp://www.ietf.org/rfc/rfc2152.txt
*/

/*!
	UTF-7 �Z�b�g�c�̕�����

	@return �Z�b�g�c�̕�����̒���

	@param[out] ppNextChar ���̃u���b�N�iUTF-7�Z�b�gB�����j�̐擪�����̃|�C���^���i�[�����B�i'+'���΂��j

	pbError �� NULL �ȊO�ɐݒ肵�Ă��āApbError �� true ���i�[���ꂽ�ꍇ�A
	�߂�l�� ppNextChar �Ɋi�[�����|�C���^�͎g���Ȃ��B
	1�ȏ�̃G���[��������Ό�₩��O���̂ł��������K���Ȏd�l�ɁB
*/
int CheckUtf7DPart( const char *pS, const int nLen, char **ppNextChar, bool *pbError )
{
	const char *pr, *pr_end;
	bool berror = false;

	if( nLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		*ppNextChar = const_cast<char*>( pS );
		return 0;
	}

	pr = pS;
	pr_end = pS + nLen;
	for( ; pr < pr_end; ++pr ){
		if( *pr == '+' ){
			break;
		}
		if( !IsUtf7Direct(*pr) ){
			// UTF-7�Z�b�gD�̕����W���łȂ����̂��P�����ł������Ă���ꍇ�A
			// �G���[��Ԃ��B*pbError == true �̏ꍇ�́A
			// *ppNextChar �͕s��ƂȂ�B
			berror = true;
//			break;    // �������[�v�ɂȂ�̂ł����� break ���Ȃ��B
		}
	}
	if( pbError ){
		*pbError = berror;
	}

	if( pr < pr_end ){
		// '+' ���X�L�b�v
		*ppNextChar = const_cast<char*>(pr) + 1;
	}else{
		*ppNextChar = const_cast<char*>(pr);
	}
	return pr - pS;
}




/*!
	UTF-7 �Z�b�g�a�̕�����

	@return �Z�b�g�a������̒���

	@param[out] ppNextChar ���̃u���b�N�iUTF-7�Z�b�gD�����j�̐擪�����̃|�C���^���i�[�����i����'-'���΂��j

	@note ���̊֐��̑O�� CheckUtf7DPart() �����s�����K�v������B
*/
int CheckUtf7BPart( const char *pS, const int nLen, char **ppNextChar, bool *pbError, const int nOption )
{
	const char *pr, *pr_end;
	bool berror_found, bminus_found;
	int nchecklen;

	wchar_t* pdata;
	int ndatalen, nret;
	ECharSet echarset;
	CMemory cmbuffer;


	if( nLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		*ppNextChar = const_cast<char*>( pS );
		return 0;
	}

	berror_found = false;
	bminus_found = false;
	pr = pS;
	pr_end = pS + nLen;

	for( ; pr < pr_end; ++pr ){
		// �Z�b�g�a�̕����łȂ��Ȃ�܂Ń��[�v
		if( !IsBase64(*pr) ){
			if( *pr == '-' ){
				bminus_found= true;
			}else{
				bminus_found = false;
			}
			break;
		}
	}

	nchecklen = pr - pS;

	// �ی�R�[�h
	if( nchecklen < 1 ){
		nchecklen = 0;
	}


	/*
	�� �f�R�[�h��̃f�[�^���̊m�F

	�������Ă����f�[�^�� nchecklen(= pr - pS) ���W�Ŋ����Ă݂�.
	���̗]��̒l����l������r�b�g��́c

	             |----------------------------- Base64 �\�� --------------------------------------------|
	             ��1�o�C�g  ��2�o�C�g  ��3�o�C�g  ��4�o�C�g  ��5�o�C�g  ��6�o�C�g  ��7�o�C�g  ��8�o�C�g
	�c��P����   00xx xxxx  00xx xxxx  00xx xx00     ---        ---        ---        ---        ---
	�c��Q����   00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx 0000     ---        ---
	�c��R����   00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx

	��L�R�ʂ�̂��Â�ɂ����Ă͂܂�Ȃ��ꍇ�͑S�f�[�^�𗎂Ƃ��i�s���o�C�g�Ƃ���j.
	*/
	const char *pr_ = pr - 1;
	switch( nchecklen % 8 ){
	case 0:
		break;
	case 3:
		if( Base64ToVal(pr_[0]) & 0x03 ){
			berror_found = true;
		}
		break;
	case 6:
		if( Base64ToVal(pr_[0]) & 0x0f ){
			berror_found = true;
		}
		break;
	case 8:
		// nchecklen == 0 �̏ꍇ
		break;
	default:
		berror_found = true;
	}

	if( UC_LOOSE == (nOption & UC_LOOSE) ){
		goto EndFunc;
	}

	// UTF-7������ "+-" �̃`�F�b�N

	if( pr < pr_end && (nchecklen < 1 && bminus_found != true) ){
		// �ǂݎ��|�C���^���f�[�^�̏I�[���w���Ă��Ȃ���
		// �m�F�ł��� Set B ������̒������[���̏ꍇ�́A
		// �K���I�[���� '-' �����݂��Ă��邱�Ƃ��m�F����B
		berror_found = true;
	}

	// ���ۂɃf�R�[�h���ē��e���m�F����B

	if( berror_found == true || nchecklen < 1 ){
		goto EndFunc;
	}

	cmbuffer.AllocBuffer( nchecklen );
	pdata = reinterpret_cast<wchar_t*>( cmbuffer.GetRawPtr() );
	if( pdata == NULL ){
		goto EndFunc;
	}
	ndatalen = _DecodeBase64(pS, nchecklen, reinterpret_cast<char*>(pdata)) / sizeof(wchar_t);
	CMemory::SwapHLByte( reinterpret_cast<char*>(pdata), ndatalen*sizeof(wchar_t) );
	for( int i = 0; i < ndatalen; i += nret ){
		nret = CheckUtf16leChar( &pdata[i], ndatalen - i, &echarset, nOption & UC_NONCHARACTER );
		if( echarset == CHARSET_BINARY ){
			berror_found = true;
			goto EndFunc;
		}
		if( nret == 1 && IsUtf7SetD(pdata[i]) ){
			berror_found = true;
			goto EndFunc;
		}
	}

EndFunc:;

	if( pbError ){
		*pbError = berror_found;
	}

	if( (berror_found == false || UC_LOOSE == (nOption & UC_LOOSE)) && (pr < pr_end && bminus_found == true) ){
		// '-' ���X�L�b�v�B
		*ppNextChar = const_cast<char*>(pr) + 1;
	}else{
		*ppNextChar = const_cast<char*>(pr);
	}

	return nchecklen;
}
