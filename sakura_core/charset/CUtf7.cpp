#include "stdafx.h"
#include "CUtf7.h"
#include "charset/charcode.h"


/*!
	UTF-7 �Z�b�gD �̕�������
*/
const bool    bNA  = false;
const bool CUtf7::UTF7SetD[] = {
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


//! UTF-7��Unicode�R�[�h�ϊ�
// 2007.08.13 kobake �쐬
EConvertResult CUtf7::UTF7ToUnicode( CMemory* pMem )
{
	//�f�[�^�擾
	int nDataLen;
	const char* pData=(char*)pMem->GetRawPtr(&nDataLen);

	//�K�v�ȃo�b�t�@�T�C�Y�𒲂ׂ�
	size_t dstlen=MultiByteToWideChar(
		CP_UTF7,
		0,
		pData,
		nDataLen,
		NULL,
		0
	);

	//�o�b�t�@�m��
	std::vector<wchar_t> dst(dstlen+1);

	//�ϊ�
	int ret=MultiByteToWideChar(
		CP_UTF7,
		0,
		pData,
		nDataLen,
		&dst[0],
		dstlen
	);
	dst[dstlen]=L'\0';

	//�ݒ�
	const void* p=&dst[0];
	pMem->SetRawData(p,dstlen*sizeof(wchar_t));

	return RESULT_COMPLETE;
}


/*! �R�[�h�ϊ� Unicode��UTF-7
	@date 2002.10.25 Moca UTF-7�Œ��ڃG���R�[�h�ł��镶����RFC�ɍ��킹�Đ�������
*/
EConvertResult CUtf7::UnicodeToUTF7( CMemory* pMem )
{
	//�f�[�^�擾
	int nDataLen;
	const wchar_t* pData=(wchar_t*)pMem->GetRawPtr(&nDataLen);
	nDataLen/=sizeof(wchar_t);

	//�o�͐�
	wchar_t*		pUniBuf;
	int				nUniBufLen = nDataLen;		// / sizeof(wchar_t);	2008/7/19 Uchi
	pUniBuf = new wchar_t[nUniBufLen + 1];

	int				i;
	int				j;
	unsigned char*	pDes;
	int				k;
	BOOL			bBASE64;
	int				nBgn;
	char*			pszBase64Buf;
	int				nBase64BufLen;
	char*			pszWork;
	char			cWork;

//	setlocale( LC_ALL, "Japanese" ); // wctomb ���g��Ȃ��Ȃ������߃R�����g�A�E�g
	k = 0;
	bBASE64 = FALSE;
	nBgn = 0;
//	memset( pUniBuf, 0, (nUniBufLen + 1) * sizeof( wchar_t ) );
	memcpy( pUniBuf, pData, nUniBufLen * sizeof( wchar_t ) );
	pUniBuf[nUniBufLen] = L'\0';
	for( i = 0; i < nUniBufLen; ++i ){
		j = IsUTF7Direct( pUniBuf[i]);
		if( !bBASE64 ){
			if( 1 == j ){
				k++;
			}else
			if( L'+' == pUniBuf[i] ){
				k += 2;
			}else{
				bBASE64 = TRUE;
				nBgn = i;
			}
		}else{
			if( 1 == j ){
				/* 2�o�C�g��Unicode������Ƃ����O���LO/HI�o�C�g������ */
				pszWork = (char*)(char*)&pUniBuf[nBgn];
				for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
					cWork = pszWork[j + 1];
					pszWork[j + 1] = pszWork[j];
					pszWork[j] = cWork;
				}
				/* Base64�G���R�[�h */
				pszBase64Buf = NULL;
				nBase64BufLen = 0;
				nBase64BufLen = MemBASE64_Encode(
					(char*)&pUniBuf[nBgn],			// �G���R�[�h�Ώۃf�[�^
					(i - nBgn) * sizeof( wchar_t ), // �G���R�[�h�Ώۃf�[�^��
					&pszBase64Buf,					// ���ʃf�[�^�i�[�������|�C���^�̃A�h���X
					-1,		// �G���R�[�h��̃f�[�^�������I��CRLF�Ő܂�Ԃ��ꍇ�̂P�s�ő啶���� (-1���w�肳�ꂽ�ꍇ�͐܂�Ԃ��Ȃ�)
					FALSE	// �p�f�B���O���邩
				);
				//////////////
				k++;
				k += nBase64BufLen;
				k++;
				//////////////
				delete [] pszBase64Buf;
				pszBase64Buf = NULL;
				nBase64BufLen = 0;
				bBASE64 = FALSE;
				i--;
			}else{
			}
		}
	}
	if( bBASE64 && 0 < (i - nBgn) ){
		/* 2�o�C�g��Unicode������Ƃ����O���LO/HI�o�C�g������ */
		pszWork = (char*)(char*)&pUniBuf[nBgn];
		for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
			cWork = pszWork[j + 1];
			pszWork[j + 1] = pszWork[j];
			pszWork[j] = cWork;
		}
		/* Base64�G���R�[�h */
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		nBase64BufLen = MemBASE64_Encode(
			(char*)&pUniBuf[nBgn],			// �G���R�[�h�Ώۃf�[�^
			(i - nBgn) * sizeof( wchar_t ), // �G���R�[�h�Ώۃf�[�^��
			&pszBase64Buf,					// ���ʃf�[�^�i�[�������|�C���^�̃A�h���X
			-1,		// �G���R�[�h��̃f�[�^�������I��CRLF�Ő܂�Ԃ��ꍇ�̂P�s�ő啶���� (-1���w�肳�ꂽ�ꍇ�͐܂�Ԃ��Ȃ�)
			FALSE	// �p�f�B���O���邩
		);
		//////////////
		k++;
		k += nBase64BufLen;
		k++;
		//////////////
		delete [] pszBase64Buf;
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		bBASE64 = FALSE;
	}
	delete [] pUniBuf;

	pDes = new unsigned char[k + 1];
	memset( pDes, 0, k + 1 );
	k = 0;
	bBASE64 = FALSE;
	nBgn = 0;
	pUniBuf = (wchar_t*)pData;
	for( i = 0; i < nUniBufLen; ++i ){
		j = IsUTF7Direct( pUniBuf[i] );
		if( !bBASE64 ){
			if( 1 == j ){
				pDes[k] = (unsigned char)(pUniBuf[i] & 0x007f);
				k++;
			}else
			if( L'+' == pUniBuf[i] ){
				pDes[k    ] = '+';
				pDes[k + 1] = '-';
				k += 2;
			}else{
				bBASE64 = TRUE;
				nBgn = i;
			}
		}else{
			if( 1 == j ){
				/* 2�o�C�g��Unicode������Ƃ����O���LO/HI�o�C�g������ */
				pszWork = (char*)(char*)&pUniBuf[nBgn];
				for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
					cWork = pszWork[j + 1];
					pszWork[j + 1] = pszWork[j];
					pszWork[j] = cWork;
				}
				char*	pszBase64Buf;
				int		nBase64BufLen;
				/* Base64�G���R�[�h */
				nBase64BufLen = MemBASE64_Encode(
					(char*)&pUniBuf[nBgn],			// �G���R�[�h�Ώۃf�[�^
					(i - nBgn) * sizeof( wchar_t ), // �G���R�[�h�Ώۃf�[�^��
					&pszBase64Buf,					// ���ʃf�[�^�i�[�������|�C���^�̃A�h���X
					-1,		// �G���R�[�h��̃f�[�^�������I��CRLF�Ő܂�Ԃ��ꍇ�̂P�s�ő啶���� (-1���w�肳�ꂽ�ꍇ�͐܂�Ԃ��Ȃ�)
					FALSE	// �p�f�B���O���邩
				);
				//////////////
				pDes[k] = '+';
				k++;
				memcpy( &pDes[k], pszBase64Buf, nBase64BufLen );
				k += nBase64BufLen;
				pDes[k] = '-';
				k++;
				//////////////
				delete [] pszBase64Buf;
				bBASE64 = FALSE;
				i--;
			}else{
			}
		}
	}
	if( bBASE64 && 0 < (i - nBgn) ){
		/* 2�o�C�g��Unicode������Ƃ����O���LO/HI�o�C�g������ */
		pszWork = (char*)(char*)&pUniBuf[nBgn];
		for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
			cWork = pszWork[j + 1];
			pszWork[j + 1] = pszWork[j];
			pszWork[j] = cWork;
		}
		/* Base64�G���R�[�h */
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		nBase64BufLen = MemBASE64_Encode(
			(char*)&pUniBuf[nBgn],			// �G���R�[�h�Ώۃf�[�^
			(i - nBgn) * sizeof( wchar_t ), // �G���R�[�h�Ώۃf�[�^��
			&pszBase64Buf,					// ���ʃf�[�^�i�[�������|�C���^�̃A�h���X
			-1,		// �G���R�[�h��̃f�[�^�������I��CRLF�Ő܂�Ԃ��ꍇ�̂P�s�ő啶���� (-1���w�肳�ꂽ�ꍇ�͐܂�Ԃ��Ȃ�)
			FALSE	// �p�f�B���O���邩
		);
		//////////////
		pDes[k] = '+';
		k++;
		memcpy( &pDes[k], pszBase64Buf, nBase64BufLen );
		k += nBase64BufLen;
		pDes[k] = '-';
		k++;
		//////////////
		delete [] pszBase64Buf;
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		bBASE64 = FALSE;
	}
	pMem->SetRawData( pDes, k );
	delete [] pDes;

	return RESULT_COMPLETE;
}


/*!
	Unicode�̕�����UTF-7�Œ��ڃG���R�[�h�ł��邩���ׂ�
	@author Moca
	@date 2002.10.25 �V�K�쐬

	TAB SP CR LF �� ���ڃG���R�[�h�\
	��{�Z�b�g
	         '(),-./012...789:?ABC...XYZabc...xyz
	�ȉ��̓I�v�V�����Ń��[���ł͎x����������ꍇ������
	         !"#$%&*;<=>@[\]^_`{|}
	�Ƃ肠�����������ŃI�v�V�����͒��ڕϊ��ł��Ȃ��Ɣ��f����
*/
int CUtf7::IsUTF7Direct( wchar_t wc )
{
	int nret = 0;
	if( (wc & 0xff00) == 0 ){
		nret = CUtf7::IsUtf7SetDChar( (unsigned char)wc );
	}
	return nret;
}



/*
* Base64�G���R�[�h
*
*	�����������f�[�^�́A�V���Ɋm�ۂ����������Ɋi�[����܂�
*	�I�����ɁA���̃������n���h�����w�肳�ꂽ�A�h���X�Ɋi�[���܂�
*	���������ꂽ�f�[�^��́A�ꉞNULL�I�[������ɂȂ��Ă��܂�
*
*/
int CUtf7::MemBASE64_Encode(
	const char*	pszSrc		,	// �G���R�[�h�Ώۃf�[�^
	int			nSrcLen		,	// �G���R�[�h�Ώۃf�[�^��
	char**		ppszDes		,	// ���ʃf�[�^�i�[�������|�C���^�̃A�h���X
	int			nWrap		,	// �G���R�[�h��̃f�[�^�������I��CRLF�Ő܂�Ԃ��ꍇ�̂P�s�ő啶���� (-1���w�肳�ꂽ�ꍇ�͐܂�Ԃ��Ȃ�)
	int			bPadding		// �p�f�B���O���邩
)
{
	int			i, j, k, m, n;
	long		nDesIdx;
//	char*		pszDes;
	long		nDataDes;
	long		nDataSrc;
	long		nLineLen;
	char		cw;
	const char	szBASE64CODE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int			nBASE64CODE_Num = _countof( szBASE64CODE );
	int			nDesLen;

	// ��������̒������Z�o�i�w�b�_�[�E�t�b�^�[�������j
	//=?ISO-2022-JP?B? GyRC QXc/ Lkgi JE4/ NiRq Siwk MRso Qg== ?=
	nDesLen = ((nSrcLen / 3) + ((nSrcLen % 3)? 1:0)) * 4;
	if( -1 != nWrap ){
		nDesLen += 2 * ( nDesLen / nWrap + ((nDesLen % nWrap)? 1:0 ));
	}

	(*ppszDes) = new char[nDesLen + 1];
	memset( (*ppszDes), 0, nDesLen + 1 );

	nDesIdx = 0;
	nLineLen = 0;
	for( i = 0; i < nSrcLen; i += 3 ){
		memcpy( &nDataDes, "====", 4 );
		nDataSrc = 0;
		if( nSrcLen - i < 3 ){
			k = (nSrcLen % 3) * 8 / 6 + 1;
			for( m = 0; m < nSrcLen % 3; m++ ){
				((char*)&nDataSrc)[3 - m] = pszSrc[i + m];
			}
		}else{
			k = 4;
			for( m = 0; m < 3; m++ ){
				((char*)&nDataSrc)[3 - m] = pszSrc[i + m];
			}
		}
		for( j = 0; j < k; j++ ){
			cw = (char)((nDataSrc >> (6 * (3 - j) + 8)) & 0x0000003f);
			((char*)&nDataDes)[j] = szBASE64CODE[(int)cw];
		}
		if( bPadding ){		// �p�f�B���O���邩
			k = 4;
		}else{
			nDesLen -= (4 - k);
		}
		for( n = 0; n < k; n++ ){
			// �����܂�Ԃ��̏���
			if( nWrap != -1 && nLineLen == nWrap ){
				(*ppszDes)[nDesIdx + 0] = ACODE::CR;
				(*ppszDes)[nDesIdx + 1] = ACODE::LF;
				nDesIdx += 2;
				nLineLen = 0;
			}
			(*ppszDes)[nDesIdx] = ((char*)&nDataDes)[n];
			nDesIdx++;
			nLineLen++;
		}
	}
//	GlobalUnlock( *phgDes );
	return nDesLen;
}
