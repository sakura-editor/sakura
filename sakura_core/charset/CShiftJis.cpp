#include "StdAfx.h"
#include "CShiftJis.h"
#include "charset/charcode.h"
#include "charset/codechecker.h"

// ��ˑ�����
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

void CShiftJis::S_GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const char* szData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		"",			0,	// EOL_NONE
		"\x0d\x0a",	2,	// EOL_CRLF
		"\x0a",		1,	// EOL_LF
		"\x0d",		1,	// EOL_CR
	};
	pcmemEol->SetRawData(aEolTable[eEolType].szData,aEolTable[eEolType].nLen);
}


//! �w�肵���ʒu�̕��������o�C�g��������Ԃ�
/*!
	@param[in] pData �ʒu�����߂���������̐擪
	@param[in] nDataLen ������
	@param[in] nIdx �ʒu(0�I���W��)
	@retval 1  1�o�C�g����
	@retval 2  2�o�C�g����
	@retval 0  �G���[

	@date 2005-09-02 D.S.Koba �쐬

	@note nIdx�͗\�ߕ����̐擪�ʒu�Ƃ킩���Ă��Ȃ���΂Ȃ�Ȃ��D
	2�o�C�g������2�o�C�g�ڂ�nIdx�ɗ^����Ɛ��������ʂ������Ȃ��D
*/
int CShiftJis::GetSizeOfChar( const char* pData, int nDataLen, int nIdx )
{
	if( nIdx >= nDataLen ){
		return 0;
	}else if( nIdx == (nDataLen - 1) ){
		return 1;
	}
	
	if( _IS_SJIS_1( reinterpret_cast<const unsigned char*>(pData)[nIdx] )
			&& _IS_SJIS_2( reinterpret_cast<const unsigned char*>(pData)[nIdx+1] ) ){
		return 2;
	}
	return 1;
}




/*!
	SJIS �� Unicode �ϊ�
*/
int CShiftJis::SjisToUni( const char *pSrc, const int nSrcLen, wchar_t *pDst, bool* pbError )
{
	ECharSet echarset;
	int nclen;
	const unsigned char *pr, *pr_end;
	unsigned short *pw;
	bool berror_tmp, berror=false;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>( pSrc );
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for( ; (nclen = CheckSjisChar(reinterpret_cast<const char*>(pr), pr_end-pr, &echarset)) != 0; pr += nclen ){
		switch( echarset ){
		case CHARSET_ASCII7:
			// �ی�R�[�h
			if( nclen != 1 ){
				nclen = 1;
			}
			// 7-bit ASCII ������ϊ�
			*pw = static_cast<unsigned short>( *pr );
			pw += 1;
			break;
		case CHARSET_JIS_ZENKAKU:
		case CHARSET_JIS_HANKATA:
			// �ی�R�[�h
			if( echarset == CHARSET_JIS_ZENKAKU && nclen != 2 ){
				nclen = 2;
			}
			if( echarset == CHARSET_JIS_HANKATA && nclen != 1 ){
				nclen = 1;
			}
			// �S�p�����܂��͔��p�J�^�J�i������ϊ�
			pw += _SjisToUni_char( pr, pw, echarset, &berror_tmp );
			if( berror_tmp == true ){
				berror = true;
			}
			break;
		default:/* CHARSET_BINARY:*/
			if( nclen != 1 ){	// �ی�R�[�h
				nclen = 1;
			}
			// �G���[����������
			pw += BinToText( pr, nclen, pw );
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



/* �R�[�h�ϊ� SJIS��Unicode */
EConvertResult CShiftJis::SJISToUnicode( CMemory* pMem )
{
	// �G���[���
	bool bError;

	//�\�[�X�擾
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( pMem->GetRawPtr(&nSrcLen) );

	// �ϊ���o�b�t�@�T�C�Y��ݒ肵�ă������̈�m��
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
	int nDstLen = SjisToUni( pSrc, nSrcLen, pDst, &bError );

	// pMem���X�V
	pMem->SetRawData( pDst, nDstLen*sizeof(wchar_t) );

	// ��n��
	delete [] pDst;

	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}







/*
	Unicode -> SJIS
*/
int CShiftJis::UniToSjis( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool *pbError )
{
	int nclen;
	const unsigned short *pr, *pr_end;
	unsigned char* pw;
	ECharSet echarset;
	bool berror=false, berror_tmp;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned short*>(pSrc);
	pr_end = reinterpret_cast<const unsigned short*>(pSrc+nSrcLen);
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
			pw += _UniToSjis_char( pr, pw, echarset, &berror_tmp );
			if( berror_tmp == true ){
				berror = true;
			}
			pr += nclen;
		}else{
			if( nclen == 1 && IsBinaryOnSurrogate(static_cast<wchar_t>(*pr)) ){
				*pw = static_cast<unsigned char>(TextToBin(*pr) & 0x000000ff);
				++pw;
			}else{
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




/* �R�[�h�ϊ� Unicode��SJIS */
EConvertResult CShiftJis::UnicodeToSJIS( CMemory* pMem )
{
	// ���
	bool berror;

	// �\�[�X�擾
	const wchar_t* pSrc = reinterpret_cast<const wchar_t*>( pMem->GetRawPtr() );
	int nSrcLen = pMem->GetRawLength() / sizeof(wchar_t);

	// �ϊ���o�b�t�@�T�C�Y��ݒ肵�ăo�b�t�@���m��
	char* pDst;
	try{
		pDst = new char[ nSrcLen * 2 ];
	}catch( ... ){
		pDst = NULL;
	}
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// �ϊ�
	int nDstLen = UniToSjis( pSrc, nSrcLen, pDst, &berror );

	// pMem���X�V
	pMem->SetRawData( pDst, nDstLen );

	// ��n��
	delete[] pDst;

	// ����
	if( berror == true ){
		return RESULT_LOSESOME;
	}else{
		return RESULT_COMPLETE;
	}
}


// �����R�[�h�\���p	UNICODE �� Hex �ϊ�	2008/6/9 Uchi
EConvertResult CShiftJis::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar)
{
	CMemory cCharBuffer;
	EConvertResult	res;
	int				i;
	unsigned char*	ps;
	TCHAR*			pd;
	bool			bbinary=false;

	// 2008/6/21 Uchi
	if (psStatusbar->m_bDispUniInSjis) {
		// Unicode�ŕ\��
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	cCharBuffer.SetRawData("",0);
	cCharBuffer.AppendRawData(cSrc, sizeof(wchar_t));

	if( IsBinaryOnSurrogate(cSrc[0]) ){
		bbinary = true;
	}

	// SJIS �ϊ�
	res = UnicodeToSJIS(&cCharBuffer);
	if (res != RESULT_COMPLETE) {
		return RESULT_LOSESOME;
	}

	// Hex�ϊ�
	ps = reinterpret_cast<unsigned char*>( cCharBuffer.GetRawPtr() );
	pd = pDst;
	if( bbinary == false ){
		for (i = cCharBuffer.GetRawLength(); i >0; i--, ps ++, pd += 2) {
			auto_sprintf( pd, _T("%02X"), *ps);
		}
	}else{
		auto_sprintf( pd, _T("?%02X"), *ps );
	}

	return RESULT_COMPLETE;
}
