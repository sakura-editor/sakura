// 2008.11.10  �ϊ����W�b�N����������

#include "StdAfx.h"
#include "CJis.h"
#include "CShiftJis.h"
#include <mbstring.h>
#include "convert/CDecode_Base64Decode.h"
#include "charset/charcode.h"
#include "charset/codeutil.h"
#include "convert/convert_util2.h"
#include "charset/codechecker.h"

// ��ˑ�����
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �e�픻��萔                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	@author D. S. Koba
//	���}�[�N��Y���Ă�����̂́A�������݂Ŏg����B
const char CJis::JISESCDATA_ASCII7[]				= "\x1b" "(B";  // ��
const char CJis::JISESCDATA_JISX0201Latin[]		= "\x1b" "(J";
const char CJis::JISESCDATA_JISX0201Latin_OLD[]	= "\x1b" "(H";
const char CJis::JISESCDATA_JISX0201Katakana[]	= "\x1b" "(I";  // ��
const char CJis::JISESCDATA_JISX0208_1978[]		= "\x1b" "$@";
const char CJis::JISESCDATA_JISX0208_1983[]		= "\x1b" "$B";  // ��
const char CJis::JISESCDATA_JISX0208_1990[]		= "\x1b" "&@""\x1b""$B";

#if 0 // ���g�p
const int CJis::TABLE_JISESCLEN[] = {
	0,		// JISESC_UNKNOWN
	3,		// JISESC_ASCII
	3,		// JISESC_JISX0201Latin
	3,		// JISESC_JISX0201Latin_OLD
	3,		// JISESC_JISX0201Katakana
	3,		// JISESC_JISX0208_1978
	3,		// JISESC_JISX0208_1983
	6,		// JISESC_JISX0208_1990
};
const char* CJis::TABLE_JISESCDATA[] = {
	NULL,
	JISESCDATA_ASCII,
	JISESCDATA_JISX0201Latin,
	JISESCDATA_JISX0201Latin_OLD,
	JISESCDATA_JISX0201Katakana,
	JISESCDATA_JISX0208_1978,
	JISESCDATA_JISX0208_1983,
	JISESCDATA_JISX0208_1990,
};
#endif

/*!
	JIS �̈�u���b�N�i�G�X�P�[�v�V�[�P���X�ƃG�X�P�[�v�V�[�P���X�̊Ԃ̋�ԁj��ϊ� 

	eMyJisesc �́AMYJISESC_HANKATA �� MYJISESC_ZENKAKU�B
*/
int CJis::_JisToUni_block( const unsigned char* pSrc, const int nSrcLen, unsigned short* pDst, const EMyJisEscseq eMyJisesc, bool* pbError )
{
	const unsigned char* pr;
	unsigned short* pw;
	unsigned char chankata;
	unsigned char czenkaku[2];
	unsigned int ctemp;
	bool berror=false;
	int nret;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = pSrc;
	pw = pDst;

	switch( eMyJisesc ){
	case MYJISESC_ASCII7:
		for( ; pr < pSrc+nSrcLen; ++pr ){
			if( IsAscii7(static_cast<const char>(*pr)) ){
				pw[0] = *pr;
			}else{
				berror = true;
				pw[0] = L'?';
			}
			++pw;
		}
		break;
	case MYJISESC_HANKATA:
		for( ; pr < pSrc+nSrcLen; ++pr ){
			if( IsJisHankata(static_cast<const char>(*pr)) ){
				// JIS �� SJIS
				chankata = (*pr | 0x80);
				// SJIS �� Unicode
				nret = MyMultiByteToWideChar_JP( &chankata, 1, pw, false );
				if( nret < 1 ){
					nret = 1;
				}
				pw += nret;
			}else{
				berror = true;
				pw[0] = L'?';
				++pw;
			}
		}
		break;
	case MYJISESC_ZENKAKU:
		for( ; pr < pSrc+nSrcLen-1; pr += 2 ){
			if( IsJisZen(reinterpret_cast<const char*>(pr)) ){
				// JIS -> SJIS
				ctemp = _mbcjistojms( (static_cast<unsigned int>(pr[0]) << 8) | pr[1] );
				if( ctemp != 0 ){
				// �ϊ��ɐ����B
					// SJIS �� Unicode
					czenkaku[0] = static_cast<unsigned char>( (ctemp & 0x0000ff00) >> 8 );
					czenkaku[1] = static_cast<unsigned char>( ctemp & 0x000000ff );
					nret = MyMultiByteToWideChar_JP( &czenkaku[0], 2, pw, false );
					if( nret < 1 ){
						// SJIS �� Unicode �ϊ��Ɏ��s
	  					berror = true;
						pw[0] = L'?';
						nret = 1;
					}
					pw += nret;
				}else{
				// �ϊ��Ɏ��s�B
					berror = true;
					pw[0] = L'?';
					++pw;
				}
			}else{
				berror = true;
				pw[0] = L'?';
				++pw;
			}
		}
		break;
	case MYJISESC_UNKNOWN:
		berror = true;
		for( ; pr < pSrc+nSrcLen; ++pr ){
			if( IsJis(static_cast<const char>(*pr)) ){
				pw[0] = *pr;
			}else{
				pw[0] = L'?';
			}
			++pw;
		}
		break;
	default:
		// �v���I�G���[����R�[�h
		berror = true;
		for( ; pr < pSrc+nSrcLen; ++pr ){
			pw[0] = L'?';
			++pw;
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - pDst;
}




/*
	JIS �� Unicode �ϊ�
*/
int CJis::JisToUni( const char* pSrc, const int nSrcLen, wchar_t* pDst, bool* pbError )
{
	const unsigned char *pr, *pr_end;
	const unsigned char *pr_next;
	unsigned short *pw;
	bool berror=false, berror_tmp;
	int nblocklen;
	EMyJisEscseq esctype, next_esctype;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);
	esctype = MYJISESC_ASCII7;

//	enum EMyJisEscseq {
//		MYJISESC_NONE,
//		MYJISESC_ASCII7,
//		MYJISESC_HANKATA,
//		MYJISESC_ZENKAKU,
//		MYJISESC_UNKNOWN,
//	};

	do{
		// �V�[�P���X�̃`�F�b�N
		switch( esctype ){
		case MYJISESC_ASCII7:
			// ASCII7 �u���b�N���`�F�b�N
			nblocklen = CheckJisAscii7Part(
				reinterpret_cast<const char*>(pr), pr_end-pr, reinterpret_cast<const char**>(&pr_next), &next_esctype, NULL );
			break;
		case MYJISESC_HANKATA:
			// ���p�J�^�J�i�u���b�N���`�F�b�N
			nblocklen = CheckJisHankataPart(
				reinterpret_cast<const char*>(pr), pr_end-pr,  reinterpret_cast<const char**>(&pr_next), &next_esctype, NULL );
			break;
		case MYJISESC_ZENKAKU:
			// �S�p�u���b�N���`�F�b�N
			nblocklen = CheckJisZenkakuPart(
				reinterpret_cast<const char*>(pr), pr_end-pr,  reinterpret_cast<const char**>(&pr_next), &next_esctype, NULL );
			break;
		default: // MYJISESC_UNKNOWN:
			// �s���ȃG�X�P�[�v�V�[�P���X����n�܂�u���b�N���`�F�b�N
			nblocklen = CheckJisUnknownPart(
				reinterpret_cast<const char*>(pr), pr_end-pr,  reinterpret_cast<const char**>(&pr_next), &next_esctype, NULL );
		}
		// �ϊ����s
		pw += _JisToUni_block( pr, nblocklen, pw, esctype, &berror_tmp );
		if( berror_tmp == true ){
			berror = true;
		}
		esctype = next_esctype;
		pr = pr_next;
	}while( pr_next < pr_end );

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �C���^�[�t�F�[�X                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/* E-Mail(JIS��Unicode)�R�[�h�ϊ� */
//2007.08.13 kobake �ǉ�
EConvertResult CJis::JISToUnicode(CMemory* pMem, bool base64decode)
{
	// �G���[���
	bool berror;

	// �\�[�X���擾
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( pMem->GetRawPtr(&nSrcLen) );

	// �\�[�X�o�b�t�@�|�C���^�ƃ\�[�X�̒���
	const char* psrc = pSrc;
	int nsrclen = nSrcLen;
	CMemory cmem;

	if( base64decode == true ){
		// ISO-2202-J �p�� MIME �w�b�_�[���f�R�[�h
		bool bret = MIMEHeaderDecode( pSrc, nSrcLen, &cmem, CODE_JIS );
		if( bret == true ){
			psrc = reinterpret_cast<const char*>( cmem.GetRawPtr() );
			nsrclen = cmem.GetRawLength();
		}
	}


	// �ϊ���o�b�t�@���擾
	wchar_t* pDst;
	try{
		pDst = new wchar_t[nsrclen * 3 + 1];
		if( pDst == NULL ){
			return RESULT_FAILURE;
		}
	}catch( ... ){
		return RESULT_FAILURE;
	}

	// �ϊ�
	int nDstLen = JisToUni( psrc, nsrclen, pDst, &berror );

	// pMem �ɃZ�b�g
	pMem->SetRawData( pDst, nDstLen * sizeof(wchar_t) );

	
	delete [] pDst;

	if( berror == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}


/*!
	SJIS -> JIS �ϊ�
*/
int CJis::_SjisToJis_char( const unsigned char* pSrc, unsigned char* pDst, ECharSet eCharset, bool* pbError )
{
	int nret;
	bool berror=false;
	unsigned int ctemp, ctemp_;

	switch( eCharset ){
	case CHARSET_ASCII7:
		*pDst = static_cast<char>( *pSrc );
		nret = 1;
		break;
	case CHARSET_JIS_HANKATA:
		*pDst = static_cast<char>(*pSrc & 0x7f);
		nret = 1;
		break;
	case CHARSET_JIS_ZENKAKU:
		// JIS -> SJIS
		ctemp_ = SjisFilter_basis( static_cast<unsigned int>(pSrc[0] << 8) | pSrc[1] );
		ctemp_ = SjisFilter_ibm2nec( ctemp_ );
		ctemp = _mbcjmstojis( ctemp_ );
		if( ctemp != 0 ){
			// �Ԋ҂ɐ����B
			pDst[0] = static_cast<char>( (ctemp & 0x0000ff00) >> 8 );
			pDst[1] = static_cast<char>( ctemp & 0x000000ff );
			nret = 2;
		}else{
			// �ϊ��Ɏ��s
			berror = true;
			// '�E'  0x2126(JIS) ���o��
			pDst[0] = 0x21;
			pDst[1] = 0x26;
			nret = 2;
		}
		break;
	default:
		// �G���[����R�[�h
		berror = true;
		*pDst = '?';
		nret = 1;
	}

	if( pbError ){
		*pbError = berror;
	}

	return nret;
}



int CJis::UniToJis( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool* pbError )
{
	const unsigned short *pr, *pr_end;
	unsigned char* pw;
	ECharSet echarset, echarset_cur, echarset_tmp;
	unsigned char cbuf[4];
	int nlen, nclen;
	bool berror=false, berror_tmp;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned short*>(pSrc);
	pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned char*>(pDst);
	echarset_cur = CHARSET_ASCII7;

	while( (nclen = CheckUtf16leChar(reinterpret_cast<const wchar_t*>(pr), pr_end-pr, &echarset_tmp, 0)) > 0 ){
		// Unicode -> SJIS
		nlen = MyWideCharToMultiByte_JP( pr, nclen, &cbuf[0] );
		if( nlen < 1 ){
			// Unicode -> SJIS �Ɏ��s
			berror = true;
			if( echarset_cur == CHARSET_ASCII7 ){
				*pw = '?';
				++pw;
			}else if( echarset_cur == CHARSET_JIS_HANKATA ){
				// '�' 0x25(JIS) ���o��
				*pw = 0x25;
				++pw;
			}else if( echarset_cur == CHARSET_JIS_ZENKAKU ){
				// '�E' 0x2126(JIS) ���o��
				pw[0] = 0x21;
				pw[1] = 0x26;
				pw += 2;
			}else{
				// �ی�R�[�h
				*pw = '?';
				++pw;
			}
			pr += nclen;
		}else{
			// �����Z�b�g���m�F
			if( nlen == 1 ){
				if( IsAscii7(cbuf[0]) ){
					echarset = CHARSET_ASCII7;
				}else{
					echarset = CHARSET_JIS_HANKATA;
				}
			}else if( nlen == 2 ){
				echarset = CHARSET_JIS_ZENKAKU;
			}else{
				// �G���[����R�[�h
				echarset = CHARSET_ASCII7;
				nlen = 1;
			}

			// const char CJis::JISESCDATA_ASCII[]				= "\x1b" "(B";  // ��
			// const char CJis::JISESCDATA_JISX0201Katakana[]	= "\x1b" "(I";  // ��
			// const char CJis::JISESCDATA_JISX0208_1983[]		= "\x1b" "$B";  // ��
			if( echarset != echarset_cur ){
				// �����Z�b�g���ς��΁A
				// �G�X�P�[�v�V�[�P���X��������o��
				switch( echarset ){
				case CHARSET_JIS_HANKATA:
					strncpy( reinterpret_cast<char*>(pw), JISESCDATA_JISX0201Katakana, 3 );
					pw += 3;
					break;
				case CHARSET_JIS_ZENKAKU:
					strncpy( reinterpret_cast<char*>(pw), JISESCDATA_JISX0208_1983, 3 );
					pw += 3;
					break;
				default: // case CHARSET_ASCII7:
					strncpy( reinterpret_cast<char*>(pw), JISESCDATA_ASCII7, 3 );
					pw += 3;
					break;
				}
				echarset_cur = echarset; // ���݂̕����Z�b�g��ݒ�
			}

			// SJIS -> JIS
			pw += _SjisToJis_char( &cbuf[0], pw, echarset_cur, &berror_tmp );
			if( berror_tmp == true ){
				berror = true;
			}
			pr += nclen;
		}
	}
	// CHARSET_ASCII7 �Ńf�[�^���I�����Ȃ��ꍇ�́A�ϊ��f�[�^�̍Ō��
	// CHARSET_ASCII7 �̃G�X�P�[�v�V�[�P���X���o��
	if( echarset_cur != CHARSET_ASCII7 ){
		strncpy( reinterpret_cast<char*>(pw), JISESCDATA_ASCII7, 3 );
		pw += 3;
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned char*>(pDst);
}


EConvertResult CJis::UnicodeToJIS(CMemory* pMem)
{
	bool berror=false;

	// �\�[�X���擾
	const wchar_t* pSrc = reinterpret_cast<const wchar_t*>( pMem->GetRawPtr() );
	int nSrcLen = pMem->GetRawLength() / sizeof(wchar_t);

	// �K�v�ȃo�b�t�@�e�ʂ��m�F���ăo�b�t�@���m��
	char* pDst;
	try{
		pDst = new char[nSrcLen * 8];
	}catch( ... ){
		pDst = NULL;
	}
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// �ϊ�
	int nDstLen = UniToJis( pSrc, nSrcLen, pDst, &berror );

	// pMem ���Z�b�g
	pMem->SetRawData( pDst, nDstLen );

	delete [] pDst;

	if( berror ){
		return RESULT_LOSESOME;
	}else{
		return RESULT_COMPLETE;
	}
}





// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
EConvertResult CJis::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	static CMemory	cCharBuffer;
	EConvertResult	res;
	int				i;
	TCHAR*			pd; 
	unsigned char*	ps; 

	// 2008/6/21 Uchi
	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUniInJis) {
		// Unicode�ŕ\��
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	// 1�����f�[�^�o�b�t�@
	cCharBuffer.SetRawData("",0);
	cCharBuffer.AppendRawData( cSrc, sizeof(wchar_t));

	// EUC-JP �ϊ�
	res = UnicodeToJIS(&cCharBuffer);
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex�ϊ�
	bool	bInEsc;
	bInEsc = false;
	pd = pDst;
	for (i = cCharBuffer.GetRawLength(), ps = (unsigned char*)cCharBuffer.GetRawPtr(); i >0; i--, ps ++) {
		if (*ps == 0x1B) {
			bInEsc = true;
		}
		else if (bInEsc) {
			if (*ps >= 'A' && *ps <='Z') {
				bInEsc = false;
			}
		}
		else {
			auto_sprintf( pd, _T("%02x"), *ps);
			pd += 2;
		}
	}

	return RESULT_COMPLETE;
}
