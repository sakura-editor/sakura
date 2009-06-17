#include "stdafx.h"
#include "CEuc.h"
#include "CShiftJis.h"
#include "charset/charcode.h"
#include "charset/codechecker.h"


/*!
	EUCJP �� Unicode �ϊ��֐�
*/
int CEuc::EucjpToUni( const char* pSrc, const int nSrcLen, wchar_t* pDst, bool* pbError )
{
	const unsigned char *pr, *pr_end;
	unsigned short *pw;
	int nclen;
	ECharSet echarset;
	bool berror_tmp, berror=false;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for( ; (nclen = CheckEucjpChar(reinterpret_cast<const char*>(pr), pr_end-pr, &echarset)) != 0; pr += nclen ){
		switch( echarset ){
		case CHARSET_ASCII7:
			// �ی�R�[�h
			if( nclen != 1 ){
				nclen = 1;
			}
			// 7-bit ASCII �̕ϊ�
			*pw = *pr;
			++pw;
			break;
		case CHARSET_JIS_HANKATA:
		case CHARSET_JIS_ZENKAKU:
			// �ی�R�[�h
			if( echarset == CHARSET_JIS_HANKATA && nclen != 2 ){
				nclen = 2;
			}
			if( echarset == CHARSET_JIS_ZENKAKU && nclen != 2 ){
				nclen = 2;
			}
			// �S�p�����E���p�J�^�J�i�����̕ϊ�
			pw += _EucjpToUni_char( pr, pw, echarset, &berror_tmp );
			if( berror_tmp == true ){
				berror = true;
			}
			break;
		default:// case CHARSET_BINARY:
			// �ی�R�[�h
			if( nclen != 1 ){
				nclen = 1;
			}
			// �ǂݍ��݃G���[�ɂȂ��������� PUA �ɑΉ��Â���
			pw += BinToText( pr, nclen, pw );
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}


/* EUC��Unicode�R�[�h�ϊ� */
//2007.08.13 kobake �ǉ�
EConvertResult CEuc::EUCToUnicode(CMemory* pMem)
{
	// �G���[���
	bool bError = false;

	// �\�[�X�擾
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( pMem->GetRawPtr(&nSrcLen) );

	// �ϊ���o�b�t�@�T�C�Y�Ƃ��̊m��
	wchar_t* pDst;
	try{
		pDst = new wchar_t[nSrcLen];
	}catch( ... ){
		pDst = NULL;
	}
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// �ϊ�
	int nDstLen = EucjpToUni( pSrc, nSrcLen, pDst, &bError );

	// pMem ���X�V
	pMem->SetRawData( pDst, nDstLen*sizeof(wchar_t) );

	// ��n��
	delete [] pDst;

	//$$ SJIS����Ă���̂Ŗ��ʂɃf�[�^�����������H
	// �G���[��Ԃ��悤�ɂ���B	2008/5/12 Uchi
	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}





int CEuc::UniToEucjp( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool* pbError )
{
	int nclen;
	const unsigned short *pr, *pr_end;
	unsigned char* pw;
	bool berror=false, berror_tmp;
	ECharSet echarset;

	pr = reinterpret_cast<const unsigned short*>(pSrc);
	pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned char*>(pDst);

	while( (nclen = CheckUtf16leChar(reinterpret_cast<const wchar_t*>(pr), pr_end-pr, &echarset, 0)) > 0 ){
		// �ی�R�[�h
		switch( echarset ){
		case CHARSET_UNI_NORMAL:
			nclen = 1;
			break;
		case CHARSET_UNI_SURROG:
			nclen = 2;
			break;
		default:
			echarset = CHARSET_BINARY;
			nclen = 1;
		}
		if( echarset != CHARSET_BINARY ){
			pw += _UniToEucjp_char( pr, pw, echarset, &berror_tmp );
			// �ی�R�[�h
			if( berror_tmp == true ){
				berror = true;
			}
			pr += nclen;
		}else{
			if( nclen == 1 && IsBinaryOnSurrogate(static_cast<wchar_t>(*pr)) ){
				*pw = static_cast<unsigned char>( TextToBin(*pr) & 0x00ff );
				++pw;
			}else{
				// �ی�R�[�h
				berror = true;
				*pw = '?';
				++pw;
			}
			++pr;
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned char*>(pDst);
}


EConvertResult CEuc::UnicodeToEUC(CMemory* pMem)
{
	// �G���[���
	bool bError = false;

	const wchar_t* pSrc = reinterpret_cast<wchar_t*>( pMem->GetRawPtr() );
	int nSrcLen = pMem->GetRawLength() / sizeof(wchar_t);

	// �K�v�ȃo�b�t�@�T�C�Y�𒲂ׂă��������m��
	char* pDst;
	try{
		pDst = new char[nSrcLen * 2];
	}catch( ... ){
		pDst = NULL;
	}
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// �ϊ�
	int nDstLen = UniToEucjp( pSrc, nSrcLen, pDst, &bError );

	// pMem ���X�V
	pMem->SetRawData( pDst, nDstLen );

	// ��n��
	delete [] pDst;

	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}




// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
EConvertResult CEuc::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	CMemory	cCharBuffer;
	EConvertResult	res;
	int				i;
	TCHAR*			pd; 
	unsigned char*	ps; 
	bool			bbinary=false;

	// 2008/6/21 Uchi
	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUniInEuc) {
		// Unicode�ŕ\��
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	// 1�����f�[�^�o�b�t�@
	cCharBuffer.SetRawData("",0);
	cCharBuffer.AppendRawData( cSrc, sizeof(wchar_t));

	if( IsBinaryOnSurrogate(cSrc[0]) ){
		bbinary = true;
	}

	// EUC-JP �ϊ�
	res = UnicodeToEUC(&cCharBuffer);
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex�ϊ�
	ps = reinterpret_cast<unsigned char*>( cCharBuffer.GetRawPtr() );
	pd = pDst;
	if( bbinary == false ){
		for (i = cCharBuffer.GetRawLength(); i >0; i--, ps ++, pd += 2) {
			auto_sprintf( pd, _T("%02x"), *ps);
		}
	}else{
		auto_sprintf( pd, _T("?%02x"), *ps );
	}

	return RESULT_COMPLETE;
}
