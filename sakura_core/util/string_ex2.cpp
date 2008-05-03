#include "stdafx.h"
#include "string_ex2.h"
#include "charset/charcode.h"
#include "CEol.h"

wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	auto_memcpy( dst, src, src_count );
	return dst + src_count;
}
wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src)
{
	return wcs_pushW(dst,dst_count,src,wcslen(src));
}
wchar_t *wcs_pushA(wchar_t *dst, size_t dst_count, const char* src, size_t src_count)
{
	return wcs_pushW(dst,dst_count,to_wchar(src));
}
wchar_t *wcs_pushA(wchar_t *dst, size_t dst_count, const char* src)
{
	return wcs_pushA(dst,dst_count,src,strlen(src));
}




/*! �����̃G�X�P�[�v

	@param org [in] �ϊ�������������
	@param buf [out] �ԊҌ�̕����������o�b�t�@
	@param cesc  [in] �G�X�P�[�v���Ȃ��Ƃ����Ȃ�����
	@param cwith [in] �G�X�P�[�v�Ɏg������
	
	@retval �o�͂����o�C�g�� (Unicode�̏ꍇ�͕�����)

	�����񒆂ɂ��̂܂܎g���Ƃ܂�������������ꍇ�ɂ��̕����̑O��
	�G�X�P�[�v�L�����N�^��}�����邽�߂Ɏg���D

	@note �ϊ���̃f�[�^�͍ő�Ō��̕������2�{�ɂȂ�
	@note ���̊֐���2�o�C�g�����̍l�����s���Ă��Ȃ�

	@author genta
	@date 2002/01/04 �V�K�쐬
	@date 2002/01/30 genta &��p(dupamp)�����ʂ̕�����������悤�Ɋg���D
		dupamp��inline�֐��ɂ����D
	@date 2002/02/01 genta bugfix �G�X�P�[�v���镶���Ƃ���镶���̏o�͏������t������
	@date 2004/06/19 genta Generic mapping�Ή�
*/
int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith)
{
	TCHAR *out = buf;
	for( ; *org != _T('\0'); ++org, ++out ){
		if( *org == cesc ){
			*out = cwith;
			++out;
		}
		*out = *org;
	}
	*out = _T('\0');
	return out - buf;
}




/*!	�����񂪎w�肳�ꂽ�����ŏI����Ă��Ȃ������ꍇ�ɂ�
	�����ɂ��̕�����t������D

	@param pszPath [i/o]���삷�镶����
	@param nMaxLen [in]�o�b�t�@��
	@param c [in]�ǉ�����������
	@retval  0 \��������t���Ă���
	@retval  1 \��t������
	@retval -1 �o�b�t�@�����肸�A\��t���ł��Ȃ�����
	@date 2003.06.24 Moca �V�K�쐬
*/
int AddLastChar( TCHAR* pszPath, int nMaxLen, TCHAR c ){
	int pos = _tcslen( pszPath );
	// �����Ȃ��Ƃ���\��t��
	if( 0 == pos ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[0] = c;
		pszPath[1] = _T('\0');
		return 1;
	}
	// �Ōオ\�łȂ��Ƃ���\��t��(���{����l��)
	else if( *::CharPrev( pszPath, &pszPath[pos] ) != c ){
		if( nMaxLen <= pos + 1 ){
			return -1;
		}
		pszPath[pos] = c;
		pszPath[pos + 1] = _T('\0');
		return 1;
	}
	return 0;
}



/* CR0LF0,CRLF,LF,CR�ŋ�؂���u�s�v��Ԃ��B���s�R�[�h�͍s���ɉ����Ȃ� */
const char* GetNextLine(
	const char*		pData,
	int				nDataLen,
	int*			pnLineLen,
	int*			pnBgn,
	CEol*			pcEol
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	//	May 15, 2000 genta
	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		/* ���s�R�[�h�������� */
		if( pData[i] == '\n' || pData[i] == '\r' ){
			/* �s�I�[�q�̎�ނ𒲂ׂ� */
			pcEol->SetTypeByString( &pData[i], nDataLen - i );
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}

/*!
	GetNextLine��wchar_t��
	GetNextLine���쐬
	static �����o�֐�
*/
const wchar_t* GetNextLineW(
	const wchar_t*	pData,		//!< [in]	����������
	int				nDataLen,	//!< [in]	����������̕�����
	int*			pnLineLen,	//!< [out]	1�s�̕�������Ԃ�������EOL�͊܂܂Ȃ�
	int*			pnBgn,		//!< [i/o]	����������̃I�t�Z�b�g�ʒu
	CEol*			pcEol		//!< [out]	EOL
)
{
	int		i;
	int		nBgn;
	nBgn = *pnBgn;

	pcEol->SetType( EOL_NONE );
	if( *pnBgn >= nDataLen ){
		return NULL;
	}
	for( i = *pnBgn; i < nDataLen; ++i ){
		// ���s�R�[�h��������
		if( pData[i] == L'\n' || pData[i] == L'\r' ){
			// �s�I�[�q�̎�ނ𒲂ׂ�
			pcEol->SetTypeByString(&pData[i], nDataLen - i);
			break;
		}
	}
	*pnBgn = i + pcEol->GetLen();
	*pnLineLen = i - nBgn;
	return &pData[nBgn];
}



/*! �w�蒷�ȉ��̃e�L�X�g�ɐ؂蕪����

	@param pText     [in] �؂蕪���ΏۂƂȂ镶����ւ̃|�C���^
	@param nTextLen  [in] �؂蕪���ΏۂƂȂ镶����S�̂̒���
	@param nLimitLen [in] �؂蕪���钷��
	@param pnLineLen [out] ���ۂɎ��o���ꂽ������̒���
	@param pnBgn     [i/o] ����: �؂蕪���J�n�ʒu, �o��: ���o���ꂽ������̎��̈ʒu

	@note 2003.05.25 ���g�p�̂悤��
*/
const char* GetNextLimitedLengthText( const char* pText, int nTextLen, int nLimitLen, int* pnLineLen, int* pnBgn )
{
	int		i;
	int		nBgn;
	int		nCharChars;
	nBgn = *pnBgn;
	if( nBgn >= nTextLen ){
		return NULL;
	}
	for( i = nBgn; i + 1 < nTextLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeA::GetSizeOfChar( pText, nTextLen, i );
		if( 0 == nCharChars ){
			nCharChars = 1;
		}
		if( i + nCharChars - nBgn >= nLimitLen ){
			break;
		}
		i += ( nCharChars - 1 );
	}
	*pnBgn = i;
	*pnLineLen = i - nBgn;
	return &pText[nBgn];
}





//! �f�[�^���w��u�������v�ȓ��ɐ؂�l�߂�B�߂�l�͌��ʂ̕������B
SAKURA_CORE_API int LimitStringLengthW(
	const wchar_t*	pszData,		//!< [in]
	int				nDataLength,	//!< [in]
	int				nLimitLength,	//!< [in]
	CNativeW&		cmemDes			//!< [out]
)
{
	int n=nDataLength;
	if(n>nLimitLength)n=nLimitLength;
	cmemDes.SetString(pszData,n);
	return n;
}



void GetLineColm( const wchar_t* pLine, int* pnJumpToLine, int* pnJumpToColm )
{
	int		i;
	int		j;
	int		nLineLen;
	wchar_t	szNumber[32];
	nLineLen = wcslen( pLine );
	i = 0;
	for( ; i < nLineLen; ++i ){
		if( pLine[i] >= L'0' &&
			pLine[i] <= L'9' ){
			break;
		}
	}
	wmemset( szNumber, 0, _countof( szNumber ) );
	if( i >= nLineLen ){
	}else{
		/* �s�ʒu ���s�P�ʍs�ԍ�(1�N�_)�̒��o */
		j = 0;
		for( ; i < nLineLen && j + 1 < _countof( szNumber ); ){
			szNumber[j] = pLine[i];
			j++;
			++i;
			if( pLine[i] >= L'0' &&
				pLine[i] <= L'9' ){
				continue;
			}
			break;
		}
		*pnJumpToLine = _wtoi( szNumber );

		/* ���ʒu ���s�P�ʍs�擪����̃o�C�g��(1�N�_)�̒��o */
		if( i < nLineLen && pLine[i] == ',' ){
			wmemset( szNumber, 0, _countof( szNumber ) );
			j = 0;
			++i;
			for( ; i < nLineLen && j + 1 < _countof( szNumber ); ){
				szNumber[j] = pLine[i];
				j++;
				++i;
				if( pLine[i] >= L'0' &&
					pLine[i] <= L'9' ){
					continue;
				}
				break;
			}
			*pnJumpToColm = _wtoi( szNumber );
		}
	}
	return;
}






//@@@ 2001.11.07 Start by MIK
//#ifdef COMPILE_COLOR_DIGIT
/*
 * ���l�Ȃ璷����Ԃ��B
 * 10�i���̐����܂��͏����B16�i��(����)�B
 * ������   ���l(�F����)
 * ---------------------
 * 123      123
 * 0123     0123
 * 0xfedc   0xfedc
 * -123     -123
 * &H9a     &H9a     (�������\�[�X����#if��L���ɂ����Ƃ�)
 * -0x89a   0x89a
 * 0.5      0.5
 * 0.56.1   0.56 , 1 (�������\�[�X����#if��L���ɂ�����"0.56.1"�ɂȂ�)
 * .5       5        (�������\�[�X����#if��L���ɂ�����".5"�ɂȂ�)
 * -.5      5        (�������\�[�X����#if��L���ɂ�����"-.5"�ɂȂ�)
 * 123.     123
 * 0x567.8  0x567 , 8
 */
/*
 * ���p���l
 *   1, 1.2, 1.2.3, .1, 0xabc, 1L, 1F, 1.2f, 0x1L, 0x2F, -.1, -1, 1e2, 1.2e+3, 1.2e-3, -1e0
 *   10�i��, 16�i��, LF�ڔ���, ���������_��, ������
 *   IP�A�h���X�̃h�b�g�A��(�{���͐��l����Ȃ��񂾂��)
 */
int IsNumber(const wchar_t *buf, int offset, int length)
{
	register const wchar_t* p;
	register const wchar_t* q;
	register int i = 0;
	register int d = 0;
	register int f = 0;

	p = &buf[offset];
	q = &buf[length];

	if( *p == L'0' )  /* 10�i��,C��16�i�� */
	{
		p++; i++;
		if( ( p < q ) && ( *p == L'x' ) )  /* C��16�i�� */
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= L'0' && *p <= L'9' )
				 || ( *p >= L'A' && *p <= L'F' )
				 || ( *p >= L'a' && *p <= L'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "0x" �Ȃ� "0" ���������l */
			if( i == 2 ) return 1;
			
			/* �ڔ��� */
			if( p < q )
			{
				if( *p == L'L' || *p == L'l' || *p == L'F' || *p == L'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p >= L'0' && *p <= L'9' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < L'0' || *p > L'9' )
				{
					if( *p == L'.' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == L'.' ) break;  /* "." ���A���Ȃ璆�f */
						}
					}
					else if( *p == L'E' || *p == L'e' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == L'+' || *(p + 1) == L'-' )
							 && ( *(p + 2) >= L'0' && *(p + 2) <= L'9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == L'.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
			/* �ڔ��� */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == L'L' || *p == L'l' ))
				 || *p == L'F' || *p == L'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == L'.' )
		{
			while( p < q )
			{
				if( *p < L'0' || *p > L'9' )
				{
					if( *p == L'.' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						d++;
						if( d > 1 )
						{
							if( *(p - 1) == L'.' ) break;  /* "." ���A���Ȃ璆�f */
						}
					}
					else if( *p == L'E' || *p == L'e' )
					{
						if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
						if( p + 2 < q )
						{
							if( ( *(p + 1) == L'+' || *(p + 1) == L'-' )
							 && ( *(p + 2) >= L'0' && *(p + 2) <= L'9' ) )
							{
								p++; i++;
								p++; i++;
								f = 1;
							}
							else if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else if( p + 1 < q )
						{
							if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
							{
								p++; i++;
								f = 1;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( *(p - 1)  == L'.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
			/* �ڔ��� */
			if( p < q )
			{
				if( *p == L'F' || *p == L'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else if( *p == L'E' || *p == L'e' )
		{
			p++; i++;
			while( p < q )
			{
				if( *p < L'0' || *p > L'9' )
				{
					if( ( *p == L'+' || *p == L'-' ) && ( *(p - 1) == L'E' || *(p - 1) == L'e' ) )
					{
						if( p + 1 < q )
						{
							if( *(p + 1) < L'0' || *(p + 1) > L'9' )
							{
								/* "0E+", "0E-" */
								break;
							}
						}
						else
						{
							/* "0E-", "0E+" */
							break;
						}
					}
					else
					{
						break;
					}
				}
				p++; i++;
			}
			if( i == 2 ) return 1;  /* "0E", 0e" �Ȃ� "0" �����l */
			/* �ڔ��� */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == L'L' || *p == L'l' ))
				 || *p == L'F' || *p == L'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
		else
		{
			/* "0" ���������l */
			/*if( *p == L'.' ) return i - 1;*/  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
			if( p < q )
			{
				if( (( d == 0 ) && ( *p == L'L' || *p == L'l' ))
				 || *p == L'F' || *p == L'f' )
				{
					p++; i++;
				}
			}
			return i;
		}
	}

	else if( *p >= L'1' && *p <= L'9' )  /* 10�i�� */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < L'0' || *p > L'9' )
			{
				if( *p == L'.' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == L'.' ) break;  /* "." ���A���Ȃ璆�f */
					}
				}
				else if( *p == L'E' || *p == L'e' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == L'+' || *(p + 1) == L'-' )
						 && ( *(p + 2) >= L'0' && *(p + 2) <= L'9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		if( *(p - 1) == L'.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
		/* �ڔ��� */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == L'L' || *p == L'l' ))
			 || *p == L'F' || *p == L'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == L'-' )  /* �}�C�i�X */
	{
		p++; i++;
		while( p < q )
		{
			if( *p < L'0' || *p > L'9' )
			{
				if( *p == L'.' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == L'.' ) break;  /* "." ���A���Ȃ璆�f */
					}
				}
				else if( *p == L'E' || *p == L'e' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == L'+' || *(p + 1) == L'-' )
						 && ( *(p + 2) >= L'0' && *(p + 2) <= L'9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "-", "-." �����Ȃ琔�l�łȂ� */
		//@@@ 2001.11.09 start MIK
		//if( i <= 2 ) return 0;
		//if( *(p - 1)  == L'.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
		if( i == 1 ) return 0;
		if( *(p - 1) == L'.' )
		{
			i--;
			if( i == 1 ) return 0;
			return i;
		}  //@@@ 2001.11.09 end MIK
		/* �ڔ��� */
		if( p < q )
		{
			if( (( d == 0 ) && ( *p == L'L' || *p == L'l' ))
			 || *p == L'F' || *p == L'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

	else if( *p == L'.' )  /* �����_ */
	{
		d++;
		p++; i++;
		while( p < q )
		{
			if( *p < L'0' || *p > L'9' )
			{
				if( *p == L'.' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					d++;
					if( d > 1 )
					{
						if( *(p - 1) == L'.' ) break;  /* "." ���A���Ȃ璆�f */
					}
				}
				else if( *p == L'E' || *p == L'e' )
				{
					if( f == 1 ) break;  /* �w�����ɓ����Ă��� */
					if( p + 2 < q )
					{
						if( ( *(p + 1) == L'+' || *(p + 1) == L'-' )
						 && ( *(p + 2) >= L'0' && *(p + 2) <= L'9' ) )
						{
							p++; i++;
							p++; i++;
							f = 1;
						}
						else if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else if( p + 1 < q )
					{
						if( *(p + 1) >= L'0' && *(p + 1) <= L'9' )
						{
							p++; i++;
							f = 1;
						}
						else
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			p++; i++;
		}
		/* "." �����Ȃ琔�l�łȂ� */
		if( i == 1 ) return 0;
		if( *(p - 1)  == L'.' ) return i - 1;  /* �Ōオ "." �Ȃ�܂߂Ȃ� */
		/* �ڔ��� */
		if( p < q )
		{
			if( *p == L'F' || *p == L'f' )
			{
				p++; i++;
			}
		}
		return i;
	}

#if 0
	else if( *p == L'&' )  /* VB��16�i�� */
	{
		p++; i++;
		if( ( p < q ) && ( *p == L'H' ) )
		{
			p++; i++;
			while( p < q )
			{
				if( ( *p >= L'0' && *p <= L'9' )
				 || ( *p >= L'A' && *p <= L'F' )
				 || ( *p >= L'a' && *p <= L'f' ) )
				{
					p++; i++;
				}
				else
				{
					break;
				}
			}
			/* "&H" �����Ȃ琔�l�łȂ� */
			if( i == 2 ) i = 0;
			return i;
		}

		/* "&" �����Ȃ琔�l�łȂ� */
		return 0;
	}
#endif

	/* ���l�ł͂Ȃ� */
	return 0;
}
//@@@ 2001.11.07 End by MIK




/*
	scanf�I���S�X�L����

	�g�p��:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		//����: a[0]=1, a[1]=23, a[2]=4 �ƂȂ�B
*/
int scan_ints(
	const wchar_t*	pszData,	//!< [in]
	const wchar_t*	pszFormat,	//!< [in]
	int*			anBuf		//!< [out]
)
{
	//�v�f��
	int num = 0;
	const wchar_t* p = pszFormat;
	while(*p){
		if(*p==L'%')num++;
		p++;
	}

	//�X�L����
	int dummy[32];
	memset(dummy,0,sizeof(dummy));
	int nRet = swscanf(
		pszData,pszFormat,
		&dummy[ 0],&dummy[ 1],&dummy[ 2],&dummy[ 3],&dummy[ 4],&dummy[ 5],&dummy[ 6],&dummy[ 7],&dummy[ 8],&dummy[ 9],
		&dummy[10],&dummy[11],&dummy[12],&dummy[13],&dummy[14],&dummy[15],&dummy[16],&dummy[17],&dummy[18],&dummy[19],
		&dummy[20],&dummy[21],&dummy[22],&dummy[23],&dummy[24],&dummy[25],&dummy[26],&dummy[27],&dummy[28],&dummy[29]
	);

	//���ʃR�s�[
	for(int i=0;i<num;i++){
		anBuf[i]=dummy[i];
	}

	return nRet;
}

