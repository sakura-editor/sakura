/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_STRING_EX2_AA243462_59E7_4F55_B206_FD9ED8836A09_H_
#define SAKURA_STRING_EX2_AA243462_59E7_4F55_B206_FD9ED8836A09_H_

class CEol;

// Aug. 16, 2007 kobake
wchar_t *wcsncpy_ex(wchar_t *dst, size_t dst_count, const wchar_t* src, size_t src_count);
wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src, size_t src_count);
wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src);
wchar_t *wcs_pushA(wchar_t *dst, size_t dst_count, const char* src, size_t src_count);
wchar_t *wcs_pushA(wchar_t *dst, size_t dst_count, const char* src);
#ifdef _UNICODE
#define wcs_pushT wcs_pushW
#else
#define wcs_pushT wcs_pushA
#endif

int AddLastChar( TCHAR*, int, TCHAR );/* 2003.06.24 Moca �Ō�̕������w�肳�ꂽ�����łȂ��Ƃ��͕t������ */
int LimitStringLengthA( const ACHAR*, int, int, CNativeA& );/* �f�[�^���w��u�������v�ȓ��ɐ؂�l�߂� */
int LimitStringLengthW( const WCHAR*, int, int, CNativeW& );/* �f�[�^���w��u�������v�ȓ��ɐ؂�l�߂� */
int LimitStringLengthT( const TCHAR*, int, int, CNativeT& );/* �f�[�^���w��u�������v�ȓ��ɐ؂�l�߂� */
const char* GetNextLimitedLengthText( const char*, int, int, int*, int* );/* �w�蒷�ȉ��̃e�L�X�g�ɐ؂蕪���� */
const char*    GetNextLine  ( const char*   , int, int*, int*, CEol* ); /* CR0LF0,CRLF,LF,CR�ŋ�؂���u�s�v��Ԃ��B���s�R�[�h�͍s���ɉ����Ȃ� */
const wchar_t* GetNextLineW ( const wchar_t*, int, int*, int*, CEol* ); // GetNextLine��wchar_t��
//wchar_t* GetNextLineWB( const wchar_t*, int, int*, int*, CEol* ); // GetNextLine��wchar_t��(�r�b�N�G���f�B�A���p)  // ���g�p
void GetLineColm( const wchar_t*, int*, int* );


int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith);


/*!	&�̓�d��
	���j���[�Ɋ܂܂��&��&&�ɒu��������
	@author genta
	@date 2002/01/30 cescape�Ɋg�����C
	@date 2004/06/19 genta Generic mapping
*/
inline void dupamp(const TCHAR* org, TCHAR* out)
{	cescape( org, out, _T('&'), _T('&') ); }


/*
	scanf�I���S�X�L����

	�g�p��:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		//����: a[0]=1, a[1]=23, a[2]=4 �ƂȂ�B
*/
int scan_ints(
	const wchar_t*	pszData,	//!< [in]  �f�[�^������
	const wchar_t*	pszFormat,	//!< [in]  �f�[�^�t�H�[�}�b�g
	int*			anBuf		//!< [out] �擾�������l (�v�f���͍ő�32�܂�)
);

#endif /* SAKURA_STRING_EX2_AA243462_59E7_4F55_B206_FD9ED8836A09_H_ */
/*[EOF]*/
