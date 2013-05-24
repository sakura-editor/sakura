#include "StdAfx.h"
#include "CWordParse.h"
#include "charset/charcode.h"


//@@@ 2001.06.23 N.Nakatani
/*!
	@brief ���݈ʒu�̒P��͈̔͂𒲂ׂ� static�����o
	@author N.Nakatani
	@retval true	���� ���݈ʒu�̃f�[�^�́u�P��v�ƔF������B
	@retval false	���s ���݈ʒu�̃f�[�^�́u�P��v�Ƃ͌�������Ȃ��C������B
*/
bool CWordParse::WhereCurrentWord_2(
	const wchar_t*	pLine,			//!< [in]  ���ׂ郁�����S�̂̐擪�A�h���X
	CLogicInt		nLineLen,		//!< [in]  ���ׂ郁�����S�̗̂L����
	CLogicInt		nIdx,			//!< [in]  �����J�n�n�_:pLine����̑��ΓI�Ȉʒu
	CLogicInt*		pnIdxFrom,		//!< [out] �P�ꂪ���������ꍇ�́A�P��̐擪�C���f�b�N�X��Ԃ��B
	CLogicInt*		pnIdxTo,		//!< [out] �P�ꂪ���������ꍇ�́A�P��̏I�[�̎��̃o�C�g�̐擪�C���f�b�N�X��Ԃ��B
	CNativeW*		pcmcmWord,		//!< [out] �P�ꂪ���������ꍇ�́A���ݒP���؂�o���Ďw�肳�ꂽCMemory�I�u�W�F�N�g�Ɋi�[����B��񂪕s�v�ȏꍇ��NULL���w�肷��B
	CNativeW*		pcmcmWordLeft	//!< [out] �P�ꂪ���������ꍇ�́A���ݒP��̍��Ɉʒu����P���؂�o���Ďw�肳�ꂽCMemory�I�u�W�F�N�g�Ɋi�[����B��񂪕s�v�ȏꍇ��NULL���w�肷��B
)
{
	using namespace WCODE;

	*pnIdxFrom = nIdx;
	*pnIdxTo = nIdx;

	if( NULL == pLine ){
		return false;
	}
	if( nIdx >= nLineLen ){
		return false;
	}

	// ���݈ʒu�̕����̎�ނɂ���Ă͑I��s�\
	if( pLine[nIdx] == CR || pLine[nIdx] == LF ){
		return false;
	}

	// ���݈ʒu�̕����̎�ނ𒲂ׂ�
	ECharKind nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );

	// ������ނ��ς��܂őO���փT�[�`
	CLogicInt	nIdxNext = nIdx;
	CLogicInt	nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
	while( nCharChars > 0 ){
		CLogicInt	nIdxNextPrev = nIdxNext;
		nIdxNext -= nCharChars;
		ECharKind	nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );

		ECharKind nCharKindMerge = WhatKindOfTwoChars( nCharKindNext, nCharKind );
		if( nCharKindMerge == CK_NULL ){
			nIdxNext = nIdxNextPrev;
			break;
		}
		nCharKind = nCharKindMerge;
		nCharChars = CLogicInt(&pLine[nIdxNext] - CNativeW::GetCharPrev( pLine, nLineLen, &pLine[nIdxNext] ));
	}
	*pnIdxFrom = nIdxNext;

	if( NULL != pcmcmWordLeft ){
		pcmcmWordLeft->SetString( &pLine[*pnIdxFrom], nIdx - *pnIdxFrom );
	}

	// ������ނ��ς��܂Ō���փT�[�`
	nIdxNext = nIdx;
	nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext ); // 2005-09-02 D.S.Koba GetSizeOfChar
	while( nCharChars > 0 ){
		nIdxNext += nCharChars;
		ECharKind	nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );

		ECharKind nCharKindMerge = WhatKindOfTwoChars( nCharKindNext, nCharKind );
		if( nCharKindMerge == CK_NULL ){
			break;
		}
		nCharKind = nCharKindMerge;
		nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext ); // 2005-09-02 D.S.Koba GetSizeOfChar
	}
	*pnIdxTo = nIdxNext;

	if( NULL != pcmcmWord ){
		pcmcmWord->SetString( &pLine[*pnIdxFrom], *pnIdxTo - *pnIdxFrom );
	}
	return true;
}



//! ���ʎq�Ɏg�p�\�ȕ������ǂ���
inline bool isCSymbol(wchar_t c)
{
	//return
	//	(c==L'_') ||
	//	(c>=L'0' && c<=L'9') ||
	//	(c>=L'A' && c<=L'Z') ||
	//	(c>=L'a' && c<=L'z');
	return (c<128 && gm_keyword_char[c]==CK_CSYM);
}

//! �S�p�ŁA���ʎq�Ɏg�p�\�ȕ������ǂ���
inline bool isCSymbolZen(wchar_t c)
{
	return
		(c==L'�Q') ||
		(c>=L'�O' && c<=L'�X') ||
		(c>=L'�`' && c<=L'�y') ||
		(c>=L'��' && c<=L'��');
}



//! ���݈ʒu�̕����̎�ނ𒲂ׂ�
ECharKind CWordParse::WhatKindOfChar(
	const wchar_t*	pData,
	int				pDataLen,
	int				nIdx
)
{
	using namespace WCODE;

	int nCharChars = CNativeW::GetSizeOfChar( pData, pDataLen, nIdx );
	if( nCharChars == 0 ){
		return CK_NULL;	// NULL
	}
	else if( nCharChars == 1 ){
		wchar_t c=pData[nIdx];

		//���܂ł̔��p
		if( c<128                ) return (ECharKind)gm_keyword_char[c];
		//if( c == CR              )return CK_CR;
		//if( c == LF              )return CK_LF;
		//if( c == TAB             )return CK_TAB;	// �^�u
		//if( IsControlCode(c)     )return CK_CTRL;	// ���䕶��
		//if( c == SPACE           )return CK_SPACE;	// ���p�X�y�[�X
		//if( isCSymbol(c)         )return CK_CSYM;	// ���ʎq�Ɏg�p�\�ȕ��� (���p�p�����A���p�A���_�[�X�R�A)
		if( IsHankakuKatakana(c) )return CK_KATA;	// ���p�̃J�^�J�i
		if( 0x00C0 <= c && c < 0x0180 && c != 0x00D7 && c != 0x00F7 )return CK_LATIN;
													// ���e���P�⏕�A���e���g���̂����A���t�@�x�b�g���̂��́i�~���������j
		//if( c == L'#'|| c == L'$' || c == L'@'|| c == L'\\' )return CK_UDEF;	// ���[�U��`

		//���̑�
		if( IsZenkakuSpace(c)    )return CK_ZEN_SPACE;	// �S�p�X�y�[�X
		if( c==L'�['             )return CK_ZEN_NOBASU;	// �L�΂��L�� '�['
		if( c==L'�J' || c==L'�K' )return CK_ZEN_DAKU;	// �S�p���_ �u�J�K�v
		if( isCSymbolZen(c)      )return CK_ZEN_CSYM;	// �S�p�ŁA���ʎq�Ɏg�p�\�ȕ��� 
		if( IsZenkakuKigou(c)    )return CK_ZEN_KIGO;	// �S�p�̋L��
		if( IsHiragana(c)        )return CK_HIRA;		// �Ђ炪��
		if( IsZenkakuKatakana(c) )return CK_ZEN_KATA;	// �S�p�J�^�J�i
		if( IsGreek(c)           )return CK_GREEK;		// �M���V������
		if( IsCyrillic(c)        )return CK_ZEN_ROS;	// ���V�A����
		if( IsBoxDrawing(c)      )return CK_ZEN_SKIGO;	// �S�p�̓���L��

		//������
		if( IsHankaku(c) )return CK_ETC;	// ���p�̂��̑�
		else return CK_ZEN_ETC;				// �S�p�̂��̑�(�����Ȃ�)
	}
	else if( nCharChars == 2 ){
		// �T���Q�[�g�y�A 2008/7/8 Uchi
		if (IsUTF16High(pData[nIdx]) && IsUTF16Low(pData[nIdx+1])) {
			int		nCode = 0x10000 + ((pData[nIdx] & 0x3FF)<<10) + (pData[nIdx+1] & 0x3FF);	// �R�[�h�|�C���g
			if (nCode >= 0x20000 && nCode <= 0x2FFFF) {	// CJKV �g���\��� Ext-B/Ext-C...
				return CK_ZEN_ETC;				// �S�p�̂��̑�(�����Ȃ�)
			}
		}
		return CK_ETC;	// ���p�̂��̑�
	}
	else{
		return CK_NULL;	// NULL
	}
}



//! ��̕����������������̂̎�ނ𒲂ׂ�
ECharKind CWordParse::WhatKindOfTwoChars( ECharKind kindPre, ECharKind kindCur )
{
	if( kindPre == kindCur )return kindCur;			// ����Ȃ炻�̎�ʂ�Ԃ�

	// �S�p�����E�S�p���_�͑O��̑S�p�Ђ炪�ȁE�S�p�J�^�J�i�Ɉ���������
	if( ( kindPre == CK_ZEN_NOBASU || kindPre == CK_ZEN_DAKU ) &&
		( kindCur == CK_ZEN_KATA   || kindCur == CK_HIRA     ) )return kindCur;
	if( ( kindCur == CK_ZEN_NOBASU || kindCur == CK_ZEN_DAKU ) &&
		( kindPre == CK_ZEN_KATA   || kindPre == CK_HIRA     ) )return kindPre;
	// �S�p���_�A�S�p�����̘A���́A�Ƃ肠��������̕����Ƃ݂Ȃ�
	if( ( kindPre == CK_ZEN_NOBASU || kindPre == CK_ZEN_DAKU ) &&
		( kindCur == CK_ZEN_NOBASU || kindCur == CK_ZEN_DAKU ) )return kindCur;

	if( kindPre == CK_LATIN )kindPre = CK_CSYM;		// ���e���n�����̓A���t�@�x�b�g�Ƃ݂Ȃ�
	if( kindCur == CK_LATIN )kindCur = CK_CSYM;
	if( kindPre == CK_UDEF )kindPre = CK_ETC;		// ���[�U��`�����͂��̑��̔��p�Ƃ݂Ȃ�
	if( kindCur == CK_UDEF )kindCur = CK_ETC;
	if( kindPre == CK_CTRL )kindPre = CK_ETC;		// ���䕶���͂��̑��̔��p�Ƃ݂Ȃ�
	if( kindCur == CK_CTRL )kindCur = CK_ETC;

	if( kindPre == kindCur )return kindCur;			// ����Ȃ炻�̎�ʂ�Ԃ�

	return CK_NULL;									// ����ȊO�Ȃ��̕����͕ʎ�
}


//! ��̕����������������̂̎�ނ𒲂ׂ�
ECharKind CWordParse::WhatKindOfTwoChars4KW( ECharKind kindPre, ECharKind kindCur )
{
	if( kindPre == kindCur )return kindCur;			// ����Ȃ炻�̎�ʂ�Ԃ�

	// �S�p�����E�S�p���_�͑O��̑S�p�Ђ炪�ȁE�S�p�J�^�J�i�Ɉ���������
	if( ( kindPre == CK_ZEN_NOBASU || kindPre == CK_ZEN_DAKU ) &&
		( kindCur == CK_ZEN_KATA   || kindCur == CK_HIRA     ) )return kindCur;
	if( ( kindCur == CK_ZEN_NOBASU || kindCur == CK_ZEN_DAKU ) &&
		( kindPre == CK_ZEN_KATA   || kindPre == CK_HIRA     ) )return kindPre;
	// �S�p���_�A�S�p�����̘A���́A�Ƃ肠��������̕����Ƃ݂Ȃ�
	if( ( kindPre == CK_ZEN_NOBASU || kindPre == CK_ZEN_DAKU ) &&
		( kindCur == CK_ZEN_NOBASU || kindCur == CK_ZEN_DAKU ) )return kindCur;

	if( kindPre == CK_LATIN )kindPre = CK_CSYM;		// ���e���n�����̓A���t�@�x�b�g�Ƃ݂Ȃ�
	if( kindCur == CK_LATIN )kindCur = CK_CSYM;
	if( kindPre == CK_UDEF )kindPre = CK_CSYM;		// ���[�U��`�����̓A���t�@�x�b�g�Ƃ݂Ȃ�
	if( kindCur == CK_UDEF )kindCur = CK_CSYM;
	if( kindPre == CK_CTRL )kindPre = CK_CTRL;		// ���䕶���͂��̂܂ܐ��䕶���Ƃ݂Ȃ�
	if( kindCur == CK_CTRL )kindCur = CK_CTRL;

	if( kindPre == kindCur )return kindCur;			// ����Ȃ炻�̎�ʂ�Ԃ�

	return CK_NULL;									// ����ȊO�Ȃ��̕����͕ʎ�
}


/*!	���̒P��̐擪��T��
	pLine�i�����FnLineLen�j�̕����񂩂�P���T���B
	�T���n�߂�ʒu��nIdx�Ŏw��B�����͌���Ɍ���B�P��̗��[�Ŏ~�܂�Ȃ��i�֌W�Ȃ�����j
*/
bool CWordParse::SearchNextWordPosition(
	const wchar_t*	pLine,
	CLogicInt		nLineLen,
	CLogicInt		nIdx,		//	����
	CLogicInt*		pnColumnNew,	//	���������ʒu
	BOOL			bStopsBothEnds	//	�P��̗��[�Ŏ~�܂�
)
{
	// ������ނ��ς��܂Ō���փT�[�`
	// �󔒂ƃ^�u�͖�������

	// ���݈ʒu�̕����̎�ނ𒲂ׂ�
	ECharKind nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );

	CLogicInt nIdxNext = nIdx;
	// 2005-09-02 D.S.Koba GetSizeOfChar
	CLogicInt nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext );
	while( nCharChars > 0 ){
		nIdxNext += nCharChars;
		ECharKind nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
		// �󔒂ƃ^�u�͖�������
		if( nCharKindNext == CK_TAB || nCharKindNext == CK_SPACE ){
			if ( bStopsBothEnds && nCharKind != nCharKindNext ){
				*pnColumnNew = nIdxNext;
				return true;
			}
			nCharKind = nCharKindNext;
		}
		else {
			ECharKind nCharKindMerge = WhatKindOfTwoChars( nCharKind, nCharKindNext );
			if( nCharKindMerge == CK_NULL ){
				*pnColumnNew = nIdxNext;
				return true;
			}
			nCharKind = nCharKindMerge;
		}
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext );
	}
	return false;
}


/*!	���̒P��̐擪��T��
	pLine�i�����FnLineLen�j�̕����񂩂�P���T���B
	�T���n�߂�ʒu��nIdx�Ŏw��B�����͌���Ɍ���B�P��̗��[�Ŏ~�܂�Ȃ��i�֌W�Ȃ�����j
*/
bool CWordParse::SearchNextWordPosition4KW(
	const wchar_t*	pLine,
	CLogicInt		nLineLen,
	CLogicInt		nIdx,		//	����
	CLogicInt*		pnColumnNew,	//	���������ʒu
	BOOL			bStopsBothEnds	//	�P��̗��[�Ŏ~�܂�
)
{
	// ������ނ��ς��܂Ō���փT�[�`
	// �󔒂ƃ^�u�͖�������

	// ���݈ʒu�̕����̎�ނ𒲂ׂ�
	ECharKind nCharKind = WhatKindOfChar( pLine, nLineLen, nIdx );

	CLogicInt nIdxNext = nIdx;
	// 2005-09-02 D.S.Koba GetSizeOfChar
	CLogicInt nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext );
	while( nCharChars > 0 ){
		nIdxNext += nCharChars;
		ECharKind nCharKindNext = WhatKindOfChar( pLine, nLineLen, nIdxNext );
		// �󔒂ƃ^�u�͖�������
		if( nCharKindNext == CK_TAB || nCharKindNext == CK_SPACE ){
			if ( bStopsBothEnds && nCharKind != nCharKindNext ){
				*pnColumnNew = nIdxNext;
				return true;
			}
			nCharKind = nCharKindNext;
		}
		else {
			ECharKind nCharKindMerge = WhatKindOfTwoChars4KW( nCharKind, nCharKindNext );
			if( nCharKindMerge == CK_NULL ){
				*pnColumnNew = nIdxNext;
				return true;
			}
			nCharKind = nCharKindMerge;
		}
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, nIdxNext );
	}
	return false;
}


//! wc��ascii�Ȃ�0-127�̂܂ܕԂ��B����ȊO��0��Ԃ��B
uchar_t wc_to_c(wchar_t wc)
{
#if 0
//! wc��SJIS1�o�C�g�����Ȃ�char�ɕϊ�����0�`255��Ԃ��BSJIS2�o�C�g�����Ȃ�0��Ԃ��B
	char buf[3]={0,0,0};
	int ret=wctomb(buf,wc);
	if(ret==-1)return 0;   //�G���[
	if(buf[1]!=0)return 0; //�G���[����
	return buf[0] <= 0x7F ? buf[0]: 0; //1�o�C�g�ŕ\�����̂ŁA�����Ԃ�  2011.12.17 �o�b�t�@�I�[�o�[�����̏C��
#endif
	// 2011.12.15 wctomb���g��Ȃ���
	if(wc <= 0x7F){
		return (uchar_t)wc;
	}
	return 0;
}

//@@@ 2002.01.24 Start by MIK
/*!
	������URL���ǂ�������������B
	
	@retval TRUE URL�ł���
	@retval FALSE URL�łȂ�
	
	@note �֐����ɒ�`�����e�[�u���͕K�� static const �錾�ɂ��邱��(���\�ɉe�����܂�)�B
		url_char �̒l�� url_table �̔z��ԍ�+1 �ɂȂ��Ă��܂��B
		�V���� URL ��ǉ�����ꍇ�� #define �l���C�����Ă��������B
		url_table �͓��������A���t�@�x�b�g���ɂȂ�悤�ɕ��ׂĂ��������B

	2007.10.23 kobake UNICODE�Ή��B//$ wchar_t��p�̃e�[�u��(�܂��͔��胋�[�`��)��p�ӂ����ق��������͏オ��͂��ł��B
*/
BOOL IsURL(
	const wchar_t*	pszLine,	//!< [in]  ������
	int				nLineLen,	//!< [in]  ������̒���
	int*			pnMatchLen	//!< [out] URL�̒���
)
{
	struct _url_table_t {
		wchar_t	name[12];
		int		length;
		bool	is_mail;
	};
	static const struct _url_table_t	url_table[] = {
		/* �A���t�@�x�b�g�� */
		L"file://",		7,	false, /* 1 */
		L"ftp://",		6,	false, /* 2 */
		L"gopher://",	9,	false, /* 3 */
		L"http://",		7,	false, /* 4 */
		L"https://",	8,	false, /* 5 */
		L"mailto:",		7,	true,  /* 6 */
		L"news:",		5,	false, /* 7 */
		L"nntp://",		7,	false, /* 8 */
		L"prospero://",	11,	false, /* 9 */
		L"telnet://",	9,	false, /* 10 */
		L"tp://",		5,	false, /* 11 */	//2004.02.02
		L"ttp://",		6,	false, /* 12 */	//2004.02.02
		L"wais://",		7,	false, /* 13 */
		L"{",			0,	false  /* 14 */  /* '{' is 'z'+1 : terminate */
	};

/* �e�[�u���̕ێ琫�����߂邽�߂̒�` */
	const char urF = 1;
	const char urG = 3;
	const char urH = 4;
	const char urM = 6;
	const char urN = 7;
	const char urP = 9;
	const char urT = 10;
	const char urW = 13;	//2004.02.02

	static const char	url_char[] = {
	  /* +0  +1  +2  +3  +4  +5  +6  +7  +8  +9  +A  +B  +C  +D  +E  +F */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +00: */
		  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,	/* +10: */
		  0, -1,  0, -1, -1, -1, -1,  0,  0,  0,  0, -1, -1, -1, -1, -1,	/* +20: " !"#$%&'()*+,-./" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0, -1,	/* +30: "0123456789:;<=>?" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	/* +40: "@ABCDEFGHIJKLMNO" */
		 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0, -1,  0,  0, -1,	/* +50: "PQRSTUVWXYZ[\]^_" */
		  0, -1, -1, -1, -1, -1,urF,urG,urH, -1, -1, -1, -1,urM,urN, -1,	/* +60: "`abcdefghijklmno" */
		urP, -1, -1, -1,urT, -1, -1,urW, -1, -1, -1,  0,  0,  0, -1,  0,	/* +70: "pqrstuvwxyz{|}~ " */
		/* ����128�o�C�g�]���ɂ����if����2�ӏ��폜�ł��� */
		/* 0    : not url char
		 * -1   : url char
		 * other: url head char --> url_table array number + 1
		 */
	};

	const wchar_t *p = pszLine;
	const struct _url_table_t	*urlp;
	int	i;

	if( wc_to_c(*p)==0 ) return FALSE;	/* 2�o�C�g���� */
	if( 0 < url_char[wc_to_c(*p)] ){	/* URL�J�n���� */
		for(urlp = &url_table[url_char[wc_to_c(*p)]-1]; urlp->name[0] == wc_to_c(*p); urlp++){	/* URL�e�[�u����T�� */
			if( (urlp->length <= nLineLen) && (auto_memcmp(urlp->name, pszLine, urlp->length) == 0) ){	/* URL�w�b�_�͈�v���� */
				p += urlp->length;	/* URL�w�b�_�����X�L�b�v���� */
				if( urlp->is_mail ){	/* ���[����p�̉�͂� */
					if( IsMailAddress(p, nLineLen - urlp->length, pnMatchLen) ){
						*pnMatchLen = *pnMatchLen + urlp->length;
						return TRUE;
					}
					return FALSE;
				}
				for(i = urlp->length; i < nLineLen; i++, p++){	/* �ʏ�̉�͂� */
					if( wc_to_c(*p)==0 || (!(url_char[wc_to_c(*p)])) ) break;	/* �I�[�ɒB���� */
				}
				if( i == urlp->length ) return FALSE;	/* URL�w�b�_���� */
				*pnMatchLen = i;
				return TRUE;
			}
		}
	}
	return IsMailAddress(pszLine, nLineLen, pnMatchLen);
}

/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
BOOL IsMailAddress( const wchar_t* pszBuf, int nBufLen, int* pnAddressLenfth )
{
	int		j;
	int		nDotCount;
	int		nBgn;


	j = 0;
	if( (pszBuf[j] >= L'a' && pszBuf[j] <= L'z')
	 || (pszBuf[j] >= L'A' && pszBuf[j] <= L'Z')
	 || (pszBuf[j] >= L'0' && pszBuf[j] <= L'9')
	){
		j++;
	}else{
		return FALSE;
	}
	while( j < nBufLen - 2 &&
		(
		(pszBuf[j] >= L'a' && pszBuf[j] <= L'z')
	 || (pszBuf[j] >= L'A' && pszBuf[j] <= L'Z')
	 || (pszBuf[j] >= L'0' && pszBuf[j] <= L'9')
	 || (pszBuf[j] == L'.')
	 || (pszBuf[j] == L'-')
	 || (pszBuf[j] == L'_')
		)
	){
		j++;
	}
	if( j == 0 || j >= nBufLen - 2  ){
		return FALSE;
	}
	if( L'@' != pszBuf[j] ){
		return FALSE;
	}
//	nAtPos = j;
	j++;
	nDotCount = 0;
//	nAlphaCount = 0;


	for (;;) {
		nBgn = j;
		while( j < nBufLen &&
			(
			(pszBuf[j] >= L'a' && pszBuf[j] <= L'z')
		 || (pszBuf[j] >= L'A' && pszBuf[j] <= L'Z')
		 || (pszBuf[j] >= L'0' && pszBuf[j] <= L'9')
		 || (pszBuf[j] == L'-')
		 || (pszBuf[j] == L'_')
			)
		){
			j++;
		}
		if( 0 == j - nBgn ){
			return FALSE;
		}
		if( L'.' != pszBuf[j] ){
			if( 0 == nDotCount ){
				return FALSE;
			}else{
				break;
			}
		}else{
			nDotCount++;
			j++;
		}
	}
	if( NULL != pnAddressLenfth ){
		*pnAddressLenfth = j;
	}
	return TRUE;
}
