/*!	@file
	@brief �����R�[�h�F�����C�u����

	@author Sakura-Editor collaborators
	@date 2006/03/06 �V�K�쐬
*/
/*
	Copyright (C) 2006, D. S. Koba, rastiv, genta

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

#include "stdafx.h"
#include "global.h"
#include "charcode.h"

namespace Charcode
{

	/*
		D. S. Koba ������ EncodingConverter.cpp (2005-06-28��) ����
		��`����Ă��������e�[�u�����قڂ��̂܂܈��p�D
	*/
	////////////////////////////////////////////////////////////////////////////

	const uchar_t ucNA = 0xff;
	const bool    bNA  = false;

	/*!
		BASE64 �ϊ��e�[�u�� ���̂P
	*/
	const uchar_t BASE64CHAR[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
		'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
		'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3',
		'4', '5', '6', '7', '8', '9', '+', '/'
	};

	/*!
		BASE64 �ϊ��e�[�u�� ���̂Q
	*/
	const uchar_t BASE64VAL[] = {
		ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //00-07:
		ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //08-0f:
		ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //10-17:
		ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //18-1f:
		ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //20-27:
		ucNA, ucNA, ucNA,   62, ucNA, ucNA, ucNA,   63, //28-2f:    +   /
		  52,   53,   54,   55,   56,   57,   58,   59, //30-37: 01234567
		  60,   61, ucNA, ucNA, ucNA, ucNA, ucNA, ucNA, //38-3f: 89      
		ucNA,    0,    1,    2,    3,    4,    5,    6, //40-47:  ABCDEFG
		   7,    8,    9,   10,   11,   12,   13,   14, //48-4f: HIJKLMNO
		  15,   16,   17,   18,   19,   20,   21,   22, //50-57: PQRSTUVW
		  23,   24,   25, ucNA, ucNA, ucNA, ucNA, ucNA, //58-5f: XYZ     
		   0,   26,   27,   28,   29,   30,   31,   32, //60-67: `abcdefg
		  33,   34,   35,   36,   37,   38,   39,   40, //68-6f: hijklmno
		  41,   42,   43,   44,   45,   46,   47,   48, //70-77: pqrstuvw
		  49,   50,   51, ucNA, ucNA, ucNA, ucNA, ucNA, //78-7f: xyz
	};

	/*!
		UTF-7 �Z�b�gD �̕�������
	*/
	const bool UTF7SetD[] = {
		bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  //00-07:
		bNA,  true, true, bNA,  bNA,  true, bNA,  bNA,  //08-0f:TAB, LF, CR
		bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  //10-17:
		bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  //18-1f:
		true, bNA,  bNA,  bNA,  bNA,  bNA,  bNA,  true, //20-27:SP, `'`
		true, true, bNA,  bNA,  true, true, true, true, //28-2f:(, ), `,`, -, ., /
		true, true, true, true, true, true, true, true, //30-37:0 - 7
		true, true, true, bNA,  bNA,  bNA,  bNA,  true, //38-3f:8, 9, :, ?
		bNA,  true, true, true, true, true, true, true, //40-47:A - G
		true, true, true, true, true, true, true, true, //48-4f:H - O
		true, true, true, true, true, true, true, true, //50-57:P - W
		true, true, true, bNA,  bNA,  bNA,  bNA,  bNA,  //58-5f:X, Y, Z
		bNA,  true, true, true, true, true, true, true, //60-67:a - g
		true, true, true, true, true, true, true, true, //68-6f:h - o
		true, true, true, true, true, true, true, true, //70-77:p - w
		true, true, true, bNA,  bNA,  bNA,  bNA,  bNA,  //78-7f:x, y, z
	};

	const char JISESCDATA_ASCII[]				= "\x1b" "(B";
	const char JISESCDATA_JISX0201Latin[]		= "\x1b" "(J";
	const char JISESCDATA_JISX0201Latin_OLD[]	= "\x1b" "(H";
	const char JISESCDATA_JISX0201Katakana[]	= "\x1b" "(I";
	const char JISESCDATA_JISX0208_1978[]		= "\x1b" "$@";
	const char JISESCDATA_JISX0208_1983[]		= "\x1b" "$B";
	const char JISESCDATA_JISX0208_1990[]		= "\x1b" "&@""\x1b""$B";

	/* �����܂� */////////////////////////////////////////////////////////////////////////////


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
		NULL,
		JISESCDATA_ASCII,
		JISESCDATA_JISX0201Latin,
		JISESCDATA_JISX0201Latin_OLD,
		JISESCDATA_JISX0201Katakana,
		JISESCDATA_JISX0208_1978,
		JISESCDATA_JISX0208_1983,
		JISESCDATA_JISX0208_1990,
	};

	bool __fastcall IsSJisKan1( const uchar_t c ){
	// �Q�l URL: http://www.st.rim.or.jp/~phinloda/cqa/cqa15.html#Q4
	//	return ( ((0x81 <= c) && (c <= 0x9f)) || ((0xe0 <= c) && (c <= 0xfc)) );
		return static_cast<unsigned int>(c ^ 0x20) - 0xa1 < 0x3c;
	}
	bool __fastcall IsSJisKan2( const uchar_t c ){
		return ( 0x40 <= c && c <= 0xfc && c != 0x7f );
	}
	bool __fastcall IsSJisKan( const uchar_t* pC ){
		return ( IsSJisKan1(*pC) && IsSJisKan2(*(pC+1)) );
	}
	bool __fastcall IsSJisHanKata( const uchar_t c ){
		return ( 0xa1 <= c && c <= 0xdf );
	}
	bool __fastcall IsEucKan1( const uchar_t c ){
		return ( 0xa1 <= c && c <= 0xfe );
	}
	bool __fastcall IsEucKan2( const uchar_t c ){
		return ( 0xa1 <= c && c <= 0xfe );
	}
	bool __fastcall IsEucKan( const uchar_t* pC ){
		return ( IsEucKan1(*pC) && IsEucKan2(*(pC+1)) );
	}
	bool __fastcall IsEucHanKana2( const uchar_t c ){
		return ( 0xa1 <= c && c <= 0xdf );
	}
	bool __fastcall IsUtf16SurrogHi( const uchar16_t wc ){
	//	return ( 0xd800 <= wc && wc <= 0xdbff );
		return ( (wc & 0xfc00) == 0xd800 );
	}
	bool __fastcall IsUtf16SurrogLow( const uchar16_t wc ){
	//	return ( 0xdc00 <= wc && wc <= 0xdfff );
		return ( (wc & 0xfc00) == 0xdc00 );
	}
#if 0 //���g�p
	bool __fastcall IsUtf16SurrogHiOrLow( const uchar16_t wc ){
		uchar16_t wc_ = wc & (uchar16_t)0xfc00;
		return ( wc_ == 0xd800 || wc_ == 0xdc00 );
	}
	bool __fastcall IsUtf16Surrogates( const uchar16_t* pwC ){
		return IsUtf16SurrogHi( *pwC ) && IsUtf16SurrogLow( *(pwC+1) );
	}
#endif
	bool __fastcall IsBase64Char( const uchar_t c ){
		return ( !(c & 0x80) && BASE64VAL[c] != 0xff );
	}
	bool __fastcall IsUtf7SetDChar( const uchar_t c ){
		return ( !(c & 0x80) && UTF7SetD[c] );
	}
	
	uchar_t __fastcall Base64_CharToVal( const uchar_t c ){
		return BASE64VAL[c];
	}
	uchar_t __fastcall Base64_ValToChar( const uchar_t v ){
		return BASE64CHAR[v];
	}
	int __fastcall GetJisESCSeqLen( const enumJisESCSeqType eEscType ){
		return TABLE_JISESCLEN[(int)eEscType];
	}
	const char* __fastcall GetJisESCSeqData( const enumJisESCSeqType eEscType ){
		return TABLE_JISESCDATA[(int)eEscType];
	}

	/*
		UTF-7 �̂���.
		
		UTF-7 �Z�b�gD�F�@�p�p�����A'(),-./:?�A����сATAB SP CR LF
		UTF-7 �Z�b�gO�F�@!"#$%&*;<=>@[]^_`{|}
		UTF-7 �Z�b�gB�F�@�p�b�h���������� BASE64 ����
		
		1. �Z�b�gD �܂��� �Z�b�gO �ɂ��郆�j�R�[�h������, ������ ASCII �����ŕ\�������.
		2. �Z�b�gD �܂��� �Z�b�gO �ɂȂ����j�R�[�h������, BASE64 ����������, �Z�b�gB ������ɂ���ĕ\�������.
		3. �Z�b�gB �̊J�n��, ASCII ���� '+' �ɂ����, �Z�b�gB ������̏I�[��, �Z�b�gB �ɂȂ������̏o���ɂ���ĔF�������.
		   �Z�b�gB ������̏I�[�����ɂ�, �Z�b�gB �ɂȂ� ASCII ���� '-' �����Ă��悢���ƂɂȂ��Ă���,
		   ���̏I�[�����͌����莟��폜�����.
		   �Z�b�gB �J�n�����ł��� ASCII ���� '+' ���̂�, "+-" �Ƃ���������ŕ\�����.
		
		�Q�l�����F�uUCS��UTF�vhttp://homepage1.nifty.com/nomenclator/unicode/ucs_utf.htm
		          �uRFC 2152�vhttp://www.ietf.org/rfc/rfc2152.txt
	*/

	/*!
		UTF-7 �Z�b�g�c�̕�����
		
		@param[out] ref_pNext : �Ō�ɓǂݍ��񂾎��̕����ւ̃|�C���^��Ԃ��D
		@retval �s���o�C�g����Ԃ��D
	*/
	int CheckUtf7SetDPart( const uchar_t* pS, const int nLen, uchar_t*& ref_pNext )
	{
		uchar_t* ptr;
		uchar_t* base_ptr;
		uchar_t* end_ptr;
		int nlostbytes = 0;
		
		base_ptr = const_cast<uchar_t*>(pS);
		end_ptr = base_ptr + nLen;
		for( ptr = base_ptr; ptr < end_ptr; ++ptr ){
			if( IsUtf7SetDChar( *ptr ) ){
				continue;
			}
			if( *ptr == '+' ){
				// UTF-7 Set B �����̊J�n�L���𔭌�.
				break;
			}
			nlostbytes++;
		}
		
		ref_pNext = ptr;
		return nlostbytes;
	}

	/*!
		UTF-7 �Z�b�g�a�̕�����
		
		@param[out] ref_pNext : �Ō�ɓǂݍ��񂾎��̕����ւ̃|�C���^��Ԃ��D
		@retval �s���o�C�g����Ԃ��D
		@note ���̊֐������s����O�ɕK�� CheckUtf7SetDPart() �����s���邱��.
	*/
	int CheckUtf7SetBPart( const uchar_t* pS, const int nLen, uchar_t*& ref_pNext )
	{
		uchar_t* ptr;
		uchar_t* base_ptr;
		uchar_t* end_ptr;
		int nlostbytes = 0;
		int nrem;
		
		base_ptr = const_cast<uchar_t*>(pS);
		end_ptr = base_ptr + nLen;
		for( ptr = base_ptr; ptr < end_ptr; ++ptr ){
			if( !IsBase64Char( *ptr ) ){
				// UTF-7 Set D �����̊J�n��F��.
				break;
			}
		}
		
		/*
		�� ���؃X�e�b�v
		
		�f�R�[�h��̃f�[�^�����`�F�b�N����.
		�������Ă����f�[�^�� (ptr-base_ptr) ���W�Ŋ����Ă݂�.
		���̗]��̒l����l������r�b�g��́c
		
		             |----------------------------- Base64 �\�� --------------------------------------------|
		             ��1�o�C�g  ��2�o�C�g  ��3�o�C�g  ��4�o�C�g  ��5�o�C�g  ��6�o�C�g  ��7�o�C�g  ��8�o�C�g 
		�c��P����   00xx xxxx  00xx xxxx  00xx xx00     ---        ---        ---        ---        ---    
		�c��Q����   00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx 0000     ---        ---    
		�c��R����   00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx  00xx xxxx 
		
		��L�R�ʂ�̂��Â�ɂ����Ă͂܂�Ȃ��ꍇ�͑S�f�[�^�𗎂Ƃ��i�s���o�C�g�Ƃ���j.
		*/
		
		nrem = (ptr-base_ptr) % 8;
		switch ( nrem ){
		case 3:
			if( Base64_CharToVal( ptr[-1] ) & 0x03 ){
				nlostbytes = ptr - base_ptr;
			}
			break;
		case 6:
			if( Base64_CharToVal( ptr[-1] ) & 0x0f ){
				nlostbytes = ptr - base_ptr;
			}
			break;
		case 0:
			break;
		default:
			nlostbytes = ptr - base_ptr;
		}
		
		ref_pNext = ptr;
		return nlostbytes;
	}


	/*!
		UTF-7 �̕����R�[�h��������擾����
	*/
	void GetEncdInf_Utf7( const char* pS, const int nLen, MBCODE_INFO* pEI )
	{
		uchar_t* ptr;
		uchar_t* base_ptr;
		uchar_t* end_ptr;
		int nlostbytes;
		int num_of_base64_encoded_bytes;
		bool bSetBPart;
		int nret;
		
		nlostbytes = 0;
		num_of_base64_encoded_bytes = 0;
		bSetBPart = false;
		base_ptr = (uchar_t *)pS;
		ptr = base_ptr;
		end_ptr = base_ptr + nLen;
		
		while( 1 ){
			nret = CheckUtf7SetDPart( base_ptr, end_ptr-base_ptr, ptr );
			nlostbytes += nret;
			
			ptr++;  // '+' ���X�L�b�v�D
			base_ptr = ptr;
			
			nret = CheckUtf7SetBPart( base_ptr, end_ptr-base_ptr, ptr );
			// �����ŁC [���L�o�C�g��] := [���ۂɒ������ꂽ�f�[�^��] - [�s���o�C�g��]
			num_of_base64_encoded_bytes += (ptr-base_ptr) - nret;
			
			// ptr == end_ptr �����藧�ꍇ, CheckUtf7SetBPart() �֐��̓����ɂ��C
			// �P���ɕs���o�C�g�� := ptr-base_ptr�i�ǂݍ��񂾃o�C�g���j�ƂȂ鋰�ꂪ����̂ŁC
			// ���� ptr == end_ptr �����藧�Ƃ��̓��[�v�E�o�D
			if( end_ptr <= ptr ){
				break;
			}
			nlostbytes += nret;
			
			if( *ptr == '-' ){
				ptr++;
				if( end_ptr <= ptr ){
					break;
				}
			}
			base_ptr = ptr;
		}
		pEI->eCodeID = CODE_UTF7;
		pEI->nSpecBytes = num_of_base64_encoded_bytes;
		pEI->nDiff = num_of_base64_encoded_bytes - nlostbytes;
		
		return;
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
		UTF-8 �̕�����
		
		@retval ���̐� : ����� UTF-8 �o�C�g��.
		@retval ���̐� : �s���� UTF-8 �o�C�g��. (�����ϊ��s�Ƃ��͈͕s���Ƃ�)
		@retval 0      : �����f�[�^���Ȃ��Ȃ���.
	*/
	int CheckUtf8Char( const uchar_t* pC, const int nLen )
	{
		uchar_t c0, c1, c2, c3, ctemp;
		
#if 0
		// rastiv  ������₷���R�[�h�D
		
		if( nLen < 1 ){
			return 0;
		}
		
		c0 = *pC;
		if( /*nLen > 0 &&*/ (c0 & 0x80) == 0 ){
			return 1;
		}
		if( 1 < nLen && (c0 & 0xe0) == 0xc0 ){
			c1 = pC[1];
			if( (c1 & 0xc0) == 0x80 ){
				if( (c0 & 0x1e) == 0 ){
					// �f�R�[�h�ł��܂���.(�����ϊ��s��)
					return -2;
				}
				return 2;
			}
		}
		if( 2 < nLen && (c0 & 0xf0) == 0xe0 ){
			c1 = pC[1];
			c2 = pC[2];
			if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 ){
				if( (c0 & 0x0f) == 0 && (c1 & 0x20) == 0 ){
					// �f�R�[�h�ł��܂���.(�����ϊ��s��)
					return -3;
				}
				if( (c0 & 0x0f) == 0x0d && (c1 & 0x20) != 0 ){
					// U+D800 ���� U+DFFF �̃T���Q�[�g�̈�͕s���ł�.
					return -3;
				}
				return 3;
			}
		}
		if( 3 < nLen && (c0 & 0xf8) == 0xf0 ){
			c1 = pC[1];
			c2 = pC[2];
			c3 = pC[3];
			if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 && (c3 & 0xc0) == 0x80 ){
				if( (c0 & 0x07) == 0 && (c1 & 0x30) == 0 ){
					// �f�R�[�h�ł��܂���.(�����ϊ��s��)
					return -4;
				}
				if( (c0 & 0x04) != 0 && ((c0 & 0x03) != 0 || (c1 & 0x30) != 0) ){
					// �l���傫�����܂�.
					return -4;
				}
				return 4;
			}
		}
		return -1;
#endif
		
		if( 0 < nLen ){
			c0 = *pC;
			if( (c0 & 0x80) == 0 ){
				return 1;
			}
			if( 1 < nLen ){
				c1 = pC[1];
				if( (c0 & 0xe0) == 0xc0 ){
					if( (c1 & 0xc0) == 0x80 ){
						if( (c0 & 0x1e) == 0 ){
							// �f�R�[�h�ł��܂���.(�����ϊ��s�̈�)
							return -2;
						}
						return 2;
					}
				}
				if( 2 < nLen ){
					c2 = pC[2];
					if( (c0 & 0xf0) == 0xe0 ){
					//	if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 ){
						if( ((c1 & 0xc0) & (c2 & 0xc0)) == 0x80 ){
						//	if( (c0 & 0x0f) == 0 && (c1 & 0x20) == 0 ){
							if( ((c0 & 0x0f) | (c1 & 0x20)) == 0 ){
								// �f�R�[�h�ł��܂���.(�����ϊ��s�̈�)
								return -3;
							}
						//	if( (c0 & 0x0f) == 0x0d && (c1 & 0x20) != 0 ){
							if( (((c0 & 0x0f) ^ 0x0d) | (c1 & 0x20)) == 0x20 ){
								// U+D800 ���� U+DFFF �̃T���Q�[�g�̈�͕s���ł�.
								return -3;
							}
							return 3;
						}
					}
					if( 3 < nLen ){
						c3 = pC[3];
						if( (c0 & 0xf8) == 0xf0 ){
						//	if( (c1 & 0xc0) == 0x80 && (c2 & 0xc0) == 0x80 && (c3 & 0xc0) == 0x80 ){
							if( ((c1 & 0xc0) & (c2 & 0xc0) & (c3 & 0xc0)) == 0x80 ){
							//	if( (c0 & 0x07) == 0 && (c1 & 0x30) == 0 ){
								if( ((c0 & 0x07) | (c1 & 0x30)) == 0 ){
									// �f�R�[�h�ł��܂���.(�����ϊ��s�̈�)
									return -4;
								}
							//	if( (c0 & 0x04) != 0 && ((c0 & 0x03) != 0 || (c1 & 0x30) != 0) ){
								ctemp = static_cast<uchar_t>(c0 & 0x04);
								if( (ctemp | (c0 & 0x03)) | (ctemp | (c1 & 0x30)) ){
									// �l���傫�����܂�.
									return -4;
								}
								return 4;
							}
						}
					} // 3 < nLen
				} // 2 < nLen
			} // 1 < nLen
			return -1;
		} // 0 < nLen
		return 0;
	}
	int GuessCharLen_utf8(const uchar_t* pC, const int nlen )
	{
		uchar_t uc;
		
		if( nlen < 1 ){
			return 0;
		}
		uc = *pC;
		if( 1 < nlen && ( uc & 0xe0 ) == 0xc0 ){
			return 2;
		}
		if( 2 < nlen && ( uc & 0xf0 ) == 0xe0 ){
			return 3;
		}
		if( 3 < nlen && ( uc & 0xf8 ) == 0xf0 ){
			return 4;
		}
		return 1;
	}


	/*!
		UTF-8 �̕����R�[�h��������擾����
	*/
	void GetEncdInf_Utf8( const char* pS, const int nLen, MBCODE_INFO* pEI )
	{
		uchar_t* ptr;
		uchar_t* end_ptr;
		int nlostbytes;
		int num_of_utf8_encoded_bytes;
		int nret;
		
		nlostbytes = 0;
		num_of_utf8_encoded_bytes = 0;
		ptr = (uchar_t *)pS;
		end_ptr = ptr + nLen;
		
		while( 0 != (nret = CheckUtf8Char( ptr, end_ptr-ptr )) ){
			if( 0 < nret ){
				ptr += nret;
				if( 1 < nret ){
					num_of_utf8_encoded_bytes += nret;
				}
			}else{
				if( end_ptr - ptr < GuessCharLen_utf8( ptr ) ){
					// �f�[�^�����Ȃ��Ȃ��Ă��܂����D
					break;
				}
				ptr += -nret;
				nlostbytes += -nret;
			}
		}
		pEI->eCodeID = CODE_UTF8;
		pEI->nSpecBytes = num_of_utf8_encoded_bytes;
		pEI->nDiff = num_of_utf8_encoded_bytes - nlostbytes;
		
		return;
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
		UTF-16 �̕�����
	    ��ɃT���Q�[�g�y�A�̃`�F�b�N������.
	    
	    @note �ԋp�l�� �Q�̔{���ł��邱�ƁD
	*/
	int imp_CheckUtf16Char( const uchar_t* pC, const int nLen, bool bBigEndian )
	{
		const uchar16_t* pwC = reinterpret_cast<const uchar16_t*>(pC);
		uchar16_t wc1, wc2, tmp;
		
		if( 1 < nLen ){
			wc1 = pwC[0];
			if( bBigEndian ){
				tmp = static_cast<uchar16_t>(wc1 >> 8);
				wc1 <<= 8;
				wc1 |= tmp;
			}
			if( (wc1 & 0xfffe) == 0xfffe || IsUtf16SurrogLow(wc1) ){
				/* ����`�����C�܂��͉��ʃT���Q�[�g�D */
				// 0xffff �Ƃ� 0xfffe �͏����I�ɓ����Ŏg�p���邩������Ȃ��̂ŃJ�b�g�D
				return -2;
			}
			if( !IsUtf16SurrogHi(wc1) ){
				return 2; // �ʏ�̕����D
			}
			if( 3 < nLen ){
				wc2 = pwC[1];
				if( bBigEndian ){
					tmp = static_cast<uchar16_t>(wc2 >> 8);
					wc2 <<= 8;
					wc2 |= tmp;
				}
				if( IsUtf16SurrogLow(wc2) ){
					return 4;  // �T���Q�[�g�y�A�D
				}
			}
			return -2;  // �T���Q�[�g�� �� �s���D
		}
		return 0;
	}
	int CheckUtf16Char( const uchar_t* pC, const int nLen ){
		return imp_CheckUtf16Char( pC, nLen, false );
	}
	int CheckUtf16BeChar( const uchar_t* pC, const int nLen ){
		return imp_CheckUtf16Char( pC, nLen, true );
	}

#if 0  // ���g�p�D
	int GuessCharLenAsUtf16_imp( const uchar_t* pC, const int nLen, bool bBigEndian )
	{
		uchar16_t wc1, wc2;
		
		if( nLen < 2 ){
			return 0;
		}
		wc1 = *reinterpret_cast<const uchar16_t *>(pC);
		if( bBigEndian ){
			wc2 = wc1;
			wc2 >>= 8;
			wc1 <<= 8;
			wc1 |= wc2;
		}
		if( 3 < nLen && IsUtf16SurrogHi(wc1) ){
			return 4;
		}else{
			return 2;
		}
	}
	int GuessCharLenAsUtf16( const uchar_t* pC, const int nLen )
	{
		return GuessCharLenAsUtf16_imp( pC, nLen, false );
	}
	int GuessCharLenAsUtf16Be( const uchar_t* pC, const int nLen )
	{
		return GuessCharLenAsUtf16_imp( pC, nLen, true );
	}
#endif

	/*!
		UNICODE �̕����R�[�h��������擾����
	*/
	void GetEncdInf_Uni( const char* pS, const int nLen, UNICODE_INFO* pWEI )
	{
		uchar_t* ptr;
		uchar_t* end_ptr;
		int nlostbytes;
		int nCRorLF_uni;	// Unicode �� CR, LF �̃o�C�g��
		int nCRorLF_ascii;	// ASCII �� CR, LF �̃o�C�g��
		int nret;
		uchar16_t wc, wc_tmp1, wc_tmp2;
		
		nlostbytes = 0;
		nCRorLF_ascii = 0;
		nCRorLF_uni = 0;
		ptr = (uchar_t *)pS;
		end_ptr = ptr + nLen;
		
		while( 0 != (nret = CheckUtf16Char(ptr, end_ptr-ptr)) ){
			if( 0 < nret ){
				if( nret == 2 ){
					//
					// ���C�h����(Unicode)�̉��s�ƃ}���`�o�C�g�����̉��s���J�E���g
					//
					wc = *reinterpret_cast<uchar16_t *>(ptr);
					wc_tmp1 = static_cast<uchar16_t>(0x00ff & wc);
					wc_tmp2 = static_cast<uchar16_t>(0xff00 & wc);
					if( 0x000a == wc_tmp1 ){
						nCRorLF_ascii++;
						if( 0x0000 == wc_tmp2 ){
							nCRorLF_uni++;
						}else if( 0x0a00 == wc_tmp2 || 0x0d00 == wc_tmp2 ){
							nCRorLF_ascii++;
						}
					}else if( 0x000d == wc_tmp1 ){
						nCRorLF_ascii++;
						if( 0x0000 == wc_tmp2 ){
							nCRorLF_uni++;
						}else if( 0x0a00 == wc_tmp2 || 0x0d00 == wc_tmp2 ){
							nCRorLF_ascii++;
						}
					}else{
						if( 0x0a00 == wc_tmp2 || 0x0d00 == wc_tmp2 ){
							nCRorLF_ascii++;
						}
					}
				}/*else{ // nret == 4
				}*/
				ptr += nret;
			}else if( nret == -2 ){
				// �T���Q�[�g�y�A���Ј�������Ȃ�, �܂��͒l������`.
				ptr += 2;
				nlostbytes += 2;
			}else{
				break;
			}
		}
		pWEI->Uni.eCodeID = CODE_UNICODE;
		pWEI->Uni.nCRorLF = nCRorLF_uni;
		pWEI->Uni.nLostBytes = nlostbytes;
		pWEI->nCRorLF_ascii = nCRorLF_ascii;
		
		nlostbytes = 0;
		nCRorLF_uni = 0;
		ptr = (uchar_t *)pS;
		//end_ptr = ptr + nLen;
		
		while( 0 != (nret = CheckUtf16BeChar(ptr, end_ptr-ptr)) ){
			if( 0 < nret ){
				if( nret == 2 ){
					//
					// ���C�h����(Unicode BE)�̉��s���J�E���g
					//
					wc = *reinterpret_cast<uchar16_t *>(ptr);
					if( 0x0a00 == wc || 0x0d00 == wc ){
						nCRorLF_uni++;
					}
				}/*else{ // nret == 4
				}*/
				ptr += nret;
			}else if( nret == -2 ){
				// �T���Q�[�g�y�A���Ј�������Ȃ�, �܂��͒l������`.
				ptr += 2;
				nlostbytes += 2;
			}else{
				break;
			}
		}
		pWEI->UniBe.eCodeID = CODE_UNICODEBE;
		pWEI->UniBe.nCRorLF = nCRorLF_uni;
		pWEI->UniBe.nLostBytes = nlostbytes;
		
		return;
	}


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
		SJIS �̕�����
	*/
	int CheckSJisChar( const uchar_t* pS, const int nLen )
	{
		uchar_t uc;
		
		if( 0 < nLen ){
			uc = *pS;
			if( (uc & 0x80) == 0 || IsSJisHanKata( uc ) ){
				// ASCII �܂��̓��[�}��(JIS X 0201 Roman)
				// ���p�J�i(JIS X 0201 Kana)
				return 1;
			}
			if( 1 < nLen && IsSJisKan(pS) ){
				// SJIS �����E�S�p�J�i����  (JIS X 0208)
				return 2;
			}
			return -1;
		}
		return 0;
	}
	int CheckSJisCharR( const uchar_t* pS, const int nLen )
	{
		uchar_t uc;
		
		if( 0 < nLen ){
			uc = pS[-1];
			if( uc < 0x40 || 0x7f == uc ){
				// ASCII �܂��̓��[�}��(JIS X 0201 Roman.)
				return 1;
			}
			if( 1 < nLen && IsSJisKan1(pS[-2]) ){
				// SJIS �����E�S�p�J�i����  (JIS X 0208)
				return 2;
			}
			if( IsSJisHanKata(uc) ){
				// ���p�J�i(JIS X 0201 Kana)
				return 1;
			}
			return -1;
		}
		return 0;
	}
	int GuessCharLen_sjis( const uchar_t* pC, const int nLen )
	{
		if( nLen < 1 ){
			return 0;
		}
		if( 1 < nLen && IsSJisKan1(*pC) ){
			return 2;
		}
		return 1;
	}

	/*!
		SJIS �̕����R�[�h��������擾����
	*/
	void GetEncdInf_SJis( const char* pS, const int nLen, MBCODE_INFO* pEI )
	{
		uchar_t* ptr;
		uchar_t* end_ptr;
		int nlostbytes;
		int num_of_sjis_encoded_bytes;
		int nret;
		
		nlostbytes = 0;
		num_of_sjis_encoded_bytes = 0;
		ptr = (uchar_t *)pS;
		end_ptr = ptr + nLen;
		
		while( 0 != (nret = CheckSJisChar( ptr, end_ptr-ptr )) ){
			if( 0 < nret ){
				ptr += nret;
				if( 1 < nret ){
					num_of_sjis_encoded_bytes += nret;
				}
			}else /* ret < 0 */{
				if( end_ptr-ptr < GuessCharLen_sjis( ptr ) ){
					// �f�[�^���c�菭�Ȃ��Ȃ�܂���.
					break;
				}
				ptr++;
				nlostbytes++;
			}
		}
		pEI->eCodeID = CODE_SJIS;
		pEI->nSpecBytes = num_of_sjis_encoded_bytes;
		pEI->nDiff = num_of_sjis_encoded_bytes - nlostbytes;
		
		return;
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
		EUC-JP �̕�����
		
		fix: 2006.09.23 genta  EUCJP ���p�J�^�J�i���ʂ��Ԉ���Ă����̂��C���D
	*/
	int CheckEucJpChar( const uchar_t* pS, const int nLen )
	{
		uchar_t uc;
		
		if( 0 < nLen ){
			uc = *pS;
			if( (uc & 0x80) == 0 ){
				// ASCII �܂��̓��[�}���ł�.  (JIS X 0201 Roman.)
				return 1;
			}
			if( 1 < nLen ){
				if( IsEucKan( pS ) ){
					// EUC-JP �����E���ȃJ�i �ł�.  (JIS X 0208.)
					return 2;
				}
			//-	if( uc == 0x8e && IsEucKan( pS ) ){
			//-		// ���p�J�i�ł�.  (JIS X 0201 Kana.)
			//-		return 2;
			//-	}
				if( uc == 0x8e && IsEucHanKana2( pS[1] ) ){
					// ���p�J�i�ł�.  (JIS X 0201 Kana.)
					return 2;
				}
				if( 2 < nLen ){
					if( uc == 0x8f && IsEucKan( pS+1 ) ){
						// EUC-JP �⏕�����ł�.  (JIS X 0212.)
						return 3;
					}
				}
			}
			return -1;
		}
		return 0;
	}
	int GuessCharLen_eucjp( const uchar_t*pC, const int nlen )
	{
		uchar_t uc;
	
		if( nlen < 1 ){
			return 0;
		}
		uc = *pC;
		if( 2 < nlen && uc == 0x8f ){
			return 3;
		}
		if( 1 < nlen && (uc == 0x8e || IsEucKan1(uc)) ){
			return 2;
		}
		return 1;
	}


	/*!
		EUC-JP �̕����R�[�h��������擾����
	*/
	void GetEncdInf_EucJp( const char* pS, const int nLen, MBCODE_INFO* pEI )
	{
		uchar_t* ptr;
		uchar_t* end_ptr;
		int nlostbytes;
		int num_of_eucjp_encoded_bytes;
		int nret;
		
		nlostbytes = 0;
		num_of_eucjp_encoded_bytes = 0;
		ptr = (uchar_t *)pS;
		end_ptr = ptr + nLen;
		while( 0 != (nret = CheckEucJpChar( ptr, end_ptr-ptr )) ){
			if( 0 < nret ){
				ptr += nret;
				if( 1 < nret ){
					num_of_eucjp_encoded_bytes += nret;
				}
			}else /* ret < 0 */{
				if( end_ptr-ptr < GuessCharLen_eucjp( ptr ) ){
					// �c��f�[�^�����Ȃ��Ȃ�܂���...
					break;
				}
				ptr++;
				nlostbytes++;
			}
		}
		pEI->eCodeID = CODE_EUC;
		pEI->nSpecBytes = num_of_eucjp_encoded_bytes;
		pEI->nDiff = num_of_eucjp_encoded_bytes - nlostbytes;
		
		return;
	}




	/*!
		�}���`�o�C�g�����̒������擾
		
		@param [in] pC   �f�[�^
		@param [in] nLen �f�[�^��
		
		@retval ���o���ꂽ�����̃o�C�g��
		
		@note �f�[�^ pC �̍ŏ��̈ꕶ������������D
	*/
	int GetCharLen_sjis( const uchar_t* pC, const int nLen ){
		int nret = CheckSJisChar( pC, nLen );
		return 0 <= nret ? nret : -nret;
	}
	int GetCharLenR_sjis( const uchar_t* pC, const int nLen ){
		int nret = CheckSJisCharR( pC, nLen );
		return 0 <= nret ? nret : -nret;
	}
	int GetCharLen_eucjp( const uchar_t* pC, const int nLen ){
		int nret = CheckEucJpChar( pC, nLen );
		return 0 <= nret ? nret : -nret;
	}
	int GetCharLen_utf8( const uchar_t* pC, const int nLen ){
		int nret = CheckUtf8Char( pC, nLen );
		return 0 <= nret ? nret : -nret;
	}


	/*!
		JIS �� �G�X�P�[�v����������o����
		
		@param [in]  pS			���o�Ώۃf�[�^
		@param [in]  nLen		���o�Ώۃf�[�^��
		@param [out] pnEscType	���o���ꂽ�G�X�P�[�v������̎��
		
		@retval
			���o���ꂽ�ꍇ�́C���o���ꂽ�G�X�P�[�v������
			���o����Ȃ������ꍇ�́C -1
			���o�f�[�^���Ȃ��ꍇ�́C 0
		
		@note
			�߂�l���[�����傫���ꍇ�Ɍ���C*pnEscType ���X�V�����D
			pnEscType �� NULL �ł��ǂ��D
		
		
		�����������W��       16�i�\��            ������\��
		------------------------------------------------------------
		JIS C 6226-1978      1b 24 40            ESC $ @
		JIS X 0208-1983      1b 24 42            ESC $ B
		JIS X 0208-1990      1b 26 40 1b 24 42   ESC & @ ESC $ B
		JIS X 0212-1990      1b 24 28 44         ESC $ ( D
		JIS X 0213:2000 1��  1b 24 28 4f         ESC $ ( O
		JIS X 0213:2004 1��  1b 24 28 51         ESC $ ( Q
		JIS X 0213:2000 2��  1b 24 28 50         ESC $ ( P
		JIS X 0201 ���e��    1b 28 4a            ESC ( J
		JIS X 0201 ���e��    1b 28 48            ESC ( H         ���j�I [*]
		JIS X 0201 �Љ���    1b 28 49            ESC ( I
		ISO/IEC 646 IRV      1b 28 42            ESC ( B
		
		  [*] ���j�I�ȗ��R�ɂ��o�������G�X�P�[�v�V�[�P���X�D
		      JIS X 0201�̎w���Ƃ��Ă͎g�p���ׂ��łȂ��D
		
		�o�W�Fhttp://www.asahi-net.or.jp/~wq6k-yn/code/
		�Q�l�Fhttp://homepage2.nifty.com/zaco/code/
	
	*/
	int DetectJisESCSeq( const uchar_t* pS, const int nLen, int* pnEscType )
	{
		const uchar_t* end_ptr = pS + nLen;
		uchar_t* p;
		int expected_esc_len;
		int nEscType;
		
		if( nLen < 1 ){
			return 0;
		}
		
		nEscType = JISESC_UNKNOWN;
		expected_esc_len = 0;
		
		if( *pS == ACODE::ESC ){
			expected_esc_len++;
			p = const_cast<uchar_t *>(pS)+1;
			if( p+2 <= end_ptr ){
				expected_esc_len += 2;
				if( *p == '(' ){
					if( p[1] == 'B' ){
						nEscType = JISESC_ASCII;			// ESC ( B  -  ASCII
					}else if( p[1] == 'J'){
						nEscType = JISESC_JISX0201Latin;	// ESC ( J  -  JIS X 0201 ���e��
					}else if( p[1] == 'H'){
						nEscType = JISESC_JISX0201Latin_OLD;// ESC ( H  -  JIS X 0201 ���e��
					}else if( p[1] == 'I' ){
						nEscType = JISESC_JISX0201Katakana;	// ESC ( I  -  JIS X 0201 �Љ���
					}
				}else if( *p == '$' ){
					if( p[1] == 'B' ){
						nEscType = JISESC_JISX0208_1983;	// ESC $ B  -  JIS X 0208-1983
					}else if( p[1] == '@' ){
						nEscType = JISESC_JISX0208_1978;	// ESC $ @  -  JIS X 0208-1978  (��JIS)
					}
				}
			}else if( p+5 <= end_ptr ){
				expected_esc_len += 5;
				if( 0 == memcmp( p, &JISESCDATA_JISX0208_1990[1], 5 ) ){
					nEscType = JISESC_JISX0208_1990;		// ESC & @ ESC $ B  -  JIS X 0208-1990
				}
			}
		}
		
		if( 0 < expected_esc_len ){
			if( pnEscType ){
				*pnEscType = nEscType;
			}
			if( JISESC_UNKNOWN != nEscType ){
				return expected_esc_len;
			}else{
				return 1;
			}
		}else{
			return -1;
		}
	}

#if 0
	int CheckJisChar_JISX0208( const uchar_t* pS, const int nLen )
	{
		uchar_t uc0, uc1;
		
		if( nLen < 1 ){
			return 0;
		}
		if( 1 < nLen ){
			uc0 = *(pS  );
			uc1 = *(pS+1);
			if( (0x20 < uc0 && uc0 < 0x80)
			 && (0x20 < uc1 && uc1 < 0x80)
			){
				return 2;
			}
		}
		return -1;
	}
#endif

	/*!
		JIS �̕����R�[�h��������擾����
	*/
	void GetEncdInf_Jis( const char* pS, const int nLen, MBCODE_INFO* pEI )
	{
		uchar_t* ptr;
		uchar_t* end_ptr;
		int nescbytes;
		int nlostbytes;
		int nret;
		int nEscType;
		
		nescbytes = 0;
		nlostbytes = 0;
		nEscType = JISESC_ASCII;
		ptr = (uchar_t *)pS;
		end_ptr = ptr + nLen;
		
		for( ; 0 != (nret = DetectJisESCSeq(ptr, end_ptr - ptr, &nEscType)); ptr += nret ){
			if( nret < 0 ){
				nret = 1;
			}else{
				nescbytes += nret;
			}
			if( *ptr & 0x80 ){
				nlostbytes++;
			}
		}
		pEI->eCodeID = CODE_JIS;
		pEI->nSpecBytes = nescbytes;
		pEI->nDiff = nescbytes - nlostbytes;
		
		return;
	}
	
	
	/*!
		2007.08.14 kobake �߂�l��ECodeType�ɕύX

		������̐擪��Unicode�nBOM���t���Ă��邩�H
		
		@retval	CODE_NONE		�Ȃ�,�����o
		@retval	CODE_UNICODE	Unicode
		@retval	CODE_UTF8		UTF-8
		@retval	CODE_UNICODEBE	UnicodeBE
	*/
	ECodeType DetectUnicodeBom( const char* pS, int nLen )
	{
		const uchar_t* pBuf = reinterpret_cast<const uchar_t *>(pS);

		if( NULL == pS ){
			return CODE_NONE;
		}
		if( 2 <= nLen ){
			if( pBuf[0] == 0xff && pBuf[1] == 0xfe ){
				return CODE_UNICODE;
			}
			if( pBuf[0] == 0xfe && pBuf[1] == 0xff ){
				return CODE_UNICODEBE;
			}
			if( 3 <= nLen ){
				if( pBuf[0] == 0xef && pBuf[1] == 0xbb && pBuf[2] == 0xbf ){
					return CODE_UTF8;
				}
			}
		}
		return CODE_NONE;
	}







} // ends namespace Charcode.



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
protected:
	int _GetRaw(wchar_t c) const
	{
		return (cache[c/4]>>((c%4)*2))&0x3;
	}
private:
	BYTE cache[0x10000/4]; //16KB
	int test; //cache��ꌟ�o
};

//�������̓��I�v�Z�B���p�Ȃ�true�B
bool CalcHankakuByFont(wchar_t c)
{
	// -- -- �L���b�V�������݂���΁A��������̂܂ܕԂ� -- -- //
	static LocalCache cache; //$$ �����Share�̈�ɓ���Ă������ق����A�����Ɨǂ�
	if(cache.ExistCache(c))return cache.GetCache(c);

	// -- -- ��ƗpHDC -- -- //
	HDC hdc=GetDC(NULL);
	HFONT hFont = CreateFontW(
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
	HFONT hfntOld = (HFONT)SelectObject(hdc,hFont);

	// -- -- ���p� -- -- //
	SIZE han_size;
	GetTextExtentPoint32W2(hdc,L"x",1,&han_size);

	// -- -- ����256��������C�ɔ��� -- -- //
	int begin=c/256*256;
	int end=begin+256;
	for(int i=begin;i<end;i++){
		// -- -- ���Δ�r -- -- //
		SIZE size={han_size.cx*2,0}; //�֐������s�����Ƃ��̂��Ƃ��l���A�S�p���ŏ��������Ă���
		wchar_t tmp = (wchar_t)i;
		GetTextExtentPoint32W2(hdc,&tmp,1,&size);
		int char_width;
		if(size.cx>han_size.cx){
			char_width=2;
		}
		else{
			char_width=1;
		}

		// -- -- �L���b�V���X�V -- -- //
		cache.SetCache(i,char_width==1);
	}

	// -- -- ��n�� -- -- //
	SelectObject(hdc,hfntOld);
	ReleaseDC(NULL,hdc);

	return cache.GetCache(c);
}






