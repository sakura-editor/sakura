/*!	@file
	@brief �����R�[�h�������ێ��N���X

	@author Sakura-Editor collaborators
	@date 2006/12/10 �V�K�쐬
*/
/*
	Copyright (C) 2006, rastiv

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
#include "charset/CESI.h"
#include "charset/charcode.h"
#include "CShiftJis.h"
#include "CEuc.h"
#include "CUnicode.h"
#include "CUtf7.h"
#include "CBase64.h"
#include "CJis.h"

/*!
	�e�L�X�g�̕����R�[�h���X�L��������D
*/
bool CESI::ScanEncoding( const char* pS, const int nLen )
{
	if( NULL == pS ){
		return false;
	}
	CESI::_GetEncdInf_SJis( pS, nLen, &m_pEI[0] );
	CESI::_GetEncdInf_EucJp( pS, nLen, &m_pEI[1] );
	CESI::_GetEncdInf_Jis( pS, nLen, &m_pEI[2] );
	CESI::_GetEncdInf_Utf8( pS, nLen, &m_pEI[3] );
	CESI::_GetEncdInf_Utf7( pS, nLen, &m_pEI[4] );
	CESI::_GetEncdInf_Uni( pS, nLen, &m_WEI );
	
	return true;
}



/*!
	Unicode �𔻒�.
	
	@retval ���̐��F Unicode �炵���Ɣ��f���ꂽ�D
	        ���̐��F Unicode BE �炵���Ɣ��f���ꂽ�D
	        �[���F   �ǂ���Ƃ������Ȃ��D
	        *pEI_result�F �߂�l���[���ȊO�̏ꍇ�Ɍ���C�ڍ׏���Ԃ��D
*/
int CESI::DetectUnicode( WCCODE_INFO* pEI_result )
{
	int nCRorLF_Uni;	// ���C�h���� Unicode �̉��s�̌�
	int nCRorLF_UniBe;	// ���C�h���� UnicodeBE �̉��s�̌�
	int nCRorLF_mb;		// �}���`�o�C�g�����̉��s�̌�
	int nCRorLF_wc;		// := MAX( nCRorLF_Uni, nCRorLF_UniBe )
	int nUniType;
	int d;
	
	/* �}���`�o�C�g�����̉��s�i0x0d,0x0a�j�̌����`�F�b�N */
	nCRorLF_mb = m_WEI.nCRorLF_ascii;
	if( nCRorLF_mb < 1 ){
		return 0;
	}
	
	/* ���j�R�[�h BOM �̎�ނ𔻒� */
	// nUniType > 0 : ���g���G���f�B�A���炵���D
	// nUniType < 0 : �r�b�O�G���f�B�A���炵��.
	nCRorLF_Uni = m_WEI.Uni.nCRorLF;
	nCRorLF_UniBe = m_WEI.UniBe.nCRorLF;
	d = nUniType = nCRorLF_Uni - nCRorLF_UniBe;
	
	/* ���C�h�����̉��s�i0x0d00,0x0a00 or 0x000d,0x000a�j�̌����擾 */
	nCRorLF_wc = nCRorLF_Uni;
	// d := nCRorLF_Uni - nCRorLF_UniBe
	if( nUniType < 0 ){
		nCRorLF_wc += -d;
	}
	
	/*
		���؃X�e�b�v
	*/
	
	d = nCRorLF_mb - nCRorLF_wc;
	// �����ŁC�K��  nCRorLF_mb > 0 && nCRorLF_mb >= nCRorLF_wc.
	// ������Cd > 0.
	
	if( d < nCRorLF_wc ){
		/* �}���`�o�C�g�����̉��s�����C�h�����̉��s��菭�Ȃ��ꍇ. */
		// BOM �̔��茋�ʂɏ]���D
		if( 0 < nUniType ){
			*pEI_result = m_WEI.Uni;
		}else if( nUniType < 0 ){
			*pEI_result = m_WEI.UniBe;
		}
		return nUniType;
	}else{ // 0 <= nCRorLF_wc <= d
		/* �}���`�o�C�g�����̉��s�����C�h�����̉��s�Ɠ�������葽���ꍇ. */
		// BOM �̔����j������.
		return 0;
	}
}



/*!
	�}���`�o�C�g�n�����R�[�h�𔻒�.
	
	@retval   ����|�C���g�i���L�o�C�g�� �| �s���o�C�g���j�̍ł��傫�����̂�Ԃ��D
	        *pEI_result: ����|�C���g���ł��������������R�[�h�̏ڍׁD
*/
int CESI::DetectMultibyte( MBCODE_INFO* pEI_result )
{
	MBCODE_INFO* ppEI_MostDiff[NUM_OF_MBCODE];
	MBCODE_INFO* pei_tmp;
	int i, j;
	
	/*
		�u���L�o�C�g�� �| �s���o�C�g���v�̐��̑傫�����Ƀ\�[�g�i�o�u���\�[�g�j
	*/
	for( i = 0; i < NUM_OF_MBCODE; i++ ){
		ppEI_MostDiff[i] = &m_pEI[i];
	}
	for( i = 1; i < NUM_OF_MBCODE; i++ ){
		for( j = 0; j < NUM_OF_MBCODE - i; j++ ){
			if( ppEI_MostDiff[j]->nDiff < ppEI_MostDiff[j+1]->nDiff ){
				pei_tmp = ppEI_MostDiff[j+1];
				ppEI_MostDiff[j+1] = ppEI_MostDiff[j];
				ppEI_MostDiff[j] = pei_tmp;
			}
		}
	}
	*pEI_result = *ppEI_MostDiff[0];
	return ppEI_MostDiff[0]->nDiff;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// 2006.12.17 rastiv CDlgProperty::SetData���R�[�h
// 2008.04.20 kobake CESI�Ɉړ�
CNativeT CESI::GetDebugInfo(const char* pBuf, int nBufLen)
{
	CNativeT		cmemProp;
	MBCODE_INFO		mbci_tmp;
	UNICODE_INFO	uci_tmp;
	TCHAR			szWork[500];

	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: Unicode���H
	*/
	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: UnicodeBE���H
	*/
	CESI::_GetEncdInf_Uni( pBuf, nBufLen, &uci_tmp );
	auto_sprintf( szWork, _T("Unicode�R�[�h�����F���s�o�C�g��=%d  BE���s�o�C�g��=%d ASCII���s�o�C�g��=%d\r\n")
		, uci_tmp.Uni.nCRorLF, uci_tmp.UniBe.nCRorLF, uci_tmp.nCRorLF_ascii );
	cmemProp.AppendString( szWork );
	
	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: EUC���H
	*/
	CESI::_GetEncdInf_EucJp( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("EUCJP�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
	
	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: SJIS���H
	*/
	CESI::_GetEncdInf_SJis( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("SJIS�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );
	
	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: JIS���H
	*/
	CESI::_GetEncdInf_Jis( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("JIS�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: UTF-8S���H
	*/
	CESI::_GetEncdInf_Utf8( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("UTF-8�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	/*
	||�t�@�C���̓��{��R�[�h�Z�b�g����: UTF-7S���H
	*/
	CESI::_GetEncdInf_Utf7( pBuf, nBufLen, &mbci_tmp );
	auto_sprintf( szWork, _T("UTF-7�R�[�h�����F���L�o�C�g��=%d  �|�C���g��=%d\r\n")
		, mbci_tmp.nSpecBytes, mbci_tmp.nDiff );
	cmemProp.AppendString( szWork );

	return cmemProp;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                    ���L�����w���p�֐�                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
int CESI::_GuessCharLen_utf8(const uchar_t* pC, const int nlen )
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
int CESI::_GuessCharLen_sjis( const uchar_t* pC, const int nLen )
{
	if( nLen < 1 ){
		return 0;
	}
	if( 1 < nLen && CShiftJis::IsSJisKan1(*pC) ){
		return 2;
	}
	return 1;
}
int CESI::_GuessCharLen_eucjp( const uchar_t*pC, const int nlen )
{
	uchar_t uc;

	if( nlen < 1 ){
		return 0;
	}
	uc = *pC;
	if( 2 < nlen && uc == 0x8f ){
		return 3;
	}
	if( 1 < nlen && (uc == 0x8e || CEuc::IsEucKan1(uc)) ){
		return 2;
	}
	return 1;
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
int CESI::_CheckSJisChar( const uchar_t* pS, const int nLen )
{
	uchar_t uc;
	
	if( 0 < nLen ){
		uc = *pS;
		if( (uc & 0x80) == 0 || CShiftJis::IsSJisHanKata( uc ) ){
			// ASCII �܂��̓��[�}��(JIS X 0201 Roman)
			// ���p�J�i(JIS X 0201 Kana)
			return 1;
		}
		if( 1 < nLen && CShiftJis::IsSJisKan(pS) ){
			// SJIS �����E�S�p�J�i����  (JIS X 0208)
			return 2;
		}
		return -1;
	}
	return 0;
}
int CESI::_CheckSJisCharR( const uchar_t* pS, const int nLen )
{
	uchar_t uc;
	
	if( 0 < nLen ){
		uc = pS[-1];
		if( uc < 0x40 || 0x7f == uc ){
			// ASCII �܂��̓��[�}��(JIS X 0201 Roman.)
			return 1;
		}
		if( 1 < nLen && CShiftJis::IsSJisKan1(pS[-2]) ){
			// SJIS �����E�S�p�J�i����  (JIS X 0208)
			return 2;
		}
		if( CShiftJis::IsSJisHanKata(uc) ){
			// ���p�J�i(JIS X 0201 Kana)
			return 1;
		}
		return -1;
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
	EUC-JP �̕�����
	
	fix: 2006.09.23 genta  EUCJP ���p�J�^�J�i���ʂ��Ԉ���Ă����̂��C���D
*/
int CESI::_CheckEucJpChar( const uchar_t* pS, const int nLen )
{
	uchar_t uc;
	
	if( 0 < nLen ){
		uc = *pS;
		if( (uc & 0x80) == 0 ){
			// ASCII �܂��̓��[�}���ł�.  (JIS X 0201 Roman.)
			return 1;
		}
		if( 1 < nLen ){
			if( CEuc::IsEucKan( pS ) ){
				// EUC-JP �����E���ȃJ�i �ł�.  (JIS X 0208.)
				return 2;
			}
		//-	if( uc == 0x8e && CEuc::IsEucKan( pS ) ){
		//-		// ���p�J�i�ł�.  (JIS X 0201 Kana.)
		//-		return 2;
		//-	}
			if( uc == 0x8e && CEuc::IsEucHanKata2( pS[1] ) ){
				// ���p�J�i�ł�.  (JIS X 0201 Kana.)
				return 2;
			}
			if( 2 < nLen ){
				if( uc == 0x8f && CEuc::IsEucKan( pS+1 ) ){
					// EUC-JP �⏕�����ł�.  (JIS X 0212.)
					return 3;
				}
			}
		}
		return -1;
	}
	return 0;
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
int CESI::_CheckUtf8Char( const uchar_t* pC, const int nLen )
{
	uchar_t c0, c1, c2, c3, ctemp;
	
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
int CESI::_imp_CheckUtf16Char( const uchar_t* pC, const int nLen, bool bBigEndian )
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
		if( (wc1 & 0xfffe) == 0xfffe || CUnicode::IsUtf16SurrogLow(wc1) ){
			/* ����`�����C�܂��͉��ʃT���Q�[�g�D */
			// 0xffff �Ƃ� 0xfffe �͏����I�ɓ����Ŏg�p���邩������Ȃ��̂ŃJ�b�g�D
			return -2;
		}
		if( !CUnicode::IsUtf16SurrogHi(wc1) ){
			return 2; // �ʏ�̕����D
		}
		if( 3 < nLen ){
			wc2 = pwC[1];
			if( bBigEndian ){
				tmp = static_cast<uchar16_t>(wc2 >> 8);
				wc2 <<= 8;
				wc2 |= tmp;
			}
			if( CUnicode::IsUtf16SurrogLow(wc2) ){
				return 4;  // �T���Q�[�g�y�A�D
			}
		}
		return -2;  // �T���Q�[�g�� �� �s���D
	}
	return 0;
}
int CESI::_CheckUtf16Char( const uchar_t* pC, const int nLen )
{
	return _imp_CheckUtf16Char( pC, nLen, false );
}
int CESI::_CheckUtf16BeChar( const uchar_t* pC, const int nLen )
{
	return _imp_CheckUtf16Char( pC, nLen, true );
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
int CESI::_CheckUtf7SetDPart( const uchar_t* pS, const int nLen, uchar_t*& ref_pNext )
{
	uchar_t* ptr;
	uchar_t* base_ptr;
	uchar_t* end_ptr;
	int nlostbytes = 0;
	
	base_ptr = const_cast<uchar_t*>(pS);
	end_ptr = base_ptr + nLen;
	for( ptr = base_ptr; ptr < end_ptr; ++ptr ){
		if( CUtf7::IsUtf7SetDChar( *ptr ) ){
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
	@note ���̊֐������s����O�ɕK�� _CheckUtf7SetDPart() �����s���邱��.
*/
int CESI::_CheckUtf7SetBPart( const uchar_t* pS, const int nLen, uchar_t*& ref_pNext )
{
	uchar_t* ptr;
	uchar_t* base_ptr;
	uchar_t* end_ptr;
	int nlostbytes = 0;
	int nrem;
	
	base_ptr = const_cast<uchar_t*>(pS);
	end_ptr = base_ptr + nLen;
	for( ptr = base_ptr; ptr < end_ptr; ++ptr ){
		if( !CBase64::IsBase64Char( *ptr ) ){
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
		if( CBase64::Base64_CharToVal( ptr[-1] ) & 0x03 ){
			nlostbytes = ptr - base_ptr;
		}
		break;
	case 6:
		if( CBase64::Base64_CharToVal( ptr[-1] ) & 0x0f ){
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
int CESI::_DetectJisESCSeq( const uchar_t* pS, const int nLen, EJisESCSeqType* pnEscType )
{
	const uchar_t* end_ptr = pS + nLen;
	uchar_t* p;
	int expected_esc_len;
	EJisESCSeqType nEscType;
	
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
			if( 0 == memcmp( p, &CJis::JISESCDATA_JISX0208_1990[1], 5 ) ){
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



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      ������`�F�b�N                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*!
	2007.08.14 kobake �߂�l��ECodeType�ɕύX

	������̐擪��Unicode�nBOM���t���Ă��邩�H
	
	@retval	CODE_NONE		�Ȃ�,�����o
	@retval	CODE_UNICODE	Unicode
	@retval	CODE_UTF8		UTF-8
	@retval	CODE_UNICODEBE	UnicodeBE
*/
ECodeType CESI::DetectUnicodeBom( const char* pS, int nLen )
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



/*!
	SJIS �̕����R�[�h��������擾����
*/
void CESI::_GetEncdInf_SJis( const char* pS, const int nLen, MBCODE_INFO* pEI )
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
	
	while( 0 != (nret = _CheckSJisChar( ptr, end_ptr-ptr )) ){
		if( 0 < nret ){
			ptr += nret;
			if( 1 < nret ){
				num_of_sjis_encoded_bytes += nret;
			}
		}else /* ret < 0 */{
			if( end_ptr-ptr < _GuessCharLen_sjis( ptr ) ){
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

/*!
	JIS �̕����R�[�h��������擾����
*/
void CESI::_GetEncdInf_Jis( const char* pS, const int nLen, MBCODE_INFO* pEI )
{
	uchar_t* ptr = (uchar_t *)pS;
	uchar_t* end_ptr = ptr + nLen;
	int nescbytes = 0;
	int nlostbytes = 0;
	EJisESCSeqType nEscType = JISESC_ASCII;
	
	int nret;
	for( ; 0 != (nret = _DetectJisESCSeq(ptr, end_ptr - ptr, &nEscType)); ptr += nret ){
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
}


/*!
	EUC-JP �̕����R�[�h��������擾����
*/
void CESI::_GetEncdInf_EucJp( const char* pS, const int nLen, MBCODE_INFO* pEI )
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
	while( 0 != (nret = _CheckEucJpChar( ptr, end_ptr-ptr )) ){
		if( 0 < nret ){
			ptr += nret;
			if( 1 < nret ){
				num_of_eucjp_encoded_bytes += nret;
			}
		}else /* ret < 0 */{
			if( end_ptr-ptr < _GuessCharLen_eucjp( ptr ) ){
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
	UTF-8 �̕����R�[�h��������擾����
*/
void CESI::_GetEncdInf_Utf8( const char* pS, const int nLen, MBCODE_INFO* pEI )
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
	
	while( 0 != (nret = _CheckUtf8Char( ptr, end_ptr-ptr )) ){
		if( 0 < nret ){
			ptr += nret;
			if( 1 < nret ){
				num_of_utf8_encoded_bytes += nret;
			}
		}else{
			if( end_ptr - ptr < _GuessCharLen_utf8( ptr ) ){
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

/*!
	UTF-7 �̕����R�[�h��������擾����
*/
void CESI::_GetEncdInf_Utf7( const char* pS, const int nLen, MBCODE_INFO* pEI )
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
		nret = _CheckUtf7SetDPart( base_ptr, end_ptr-base_ptr, ptr );
		nlostbytes += nret;
		
		ptr++;  // '+' ���X�L�b�v�D
		base_ptr = ptr;
		
		nret = _CheckUtf7SetBPart( base_ptr, end_ptr-base_ptr, ptr );
		// �����ŁC [���L�o�C�g��] := [���ۂɒ������ꂽ�f�[�^��] - [�s���o�C�g��]
		num_of_base64_encoded_bytes += (ptr-base_ptr) - nret;
		
		// ptr == end_ptr �����藧�ꍇ, _CheckUtf7SetBPart() �֐��̓����ɂ��C
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


/*!
	UNICODE �̕����R�[�h��������擾����
*/
void CESI::_GetEncdInf_Uni( const char* pS, const int nLen, UNICODE_INFO* pWEI )
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
	
	while( 0 != (nret = _CheckUtf16Char(ptr, end_ptr-ptr)) ){
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
	
	while( 0 != (nret = _CheckUtf16BeChar(ptr, end_ptr-ptr)) ){
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
