#pragma once

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

SAKURA_CORE_API int AddLastChar( TCHAR*, int, TCHAR );/* 2003.06.24 Moca �Ō�̕������w�肳�ꂽ�����łȂ��Ƃ��͕t������ */
SAKURA_CORE_API int LimitStringLengthA( const ACHAR*, int, int, CNativeA& );/* �f�[�^���w��u�������v�ȓ��ɐ؂�l�߂� */
SAKURA_CORE_API int LimitStringLengthW( const WCHAR*, int, int, CNativeW& );/* �f�[�^���w��u�������v�ȓ��ɐ؂�l�߂� */
SAKURA_CORE_API int LimitStringLengthT( const TCHAR*, int, int, CNativeT& );/* �f�[�^���w��u�������v�ȓ��ɐ؂�l�߂� */
SAKURA_CORE_API const char* GetNextLimitedLengthText( const char*, int, int, int*, int* );/* �w�蒷�ȉ��̃e�L�X�g�ɐ؂蕪���� */
SAKURA_CORE_API const char*    GetNextLine  ( const char*   , int, int*, int*, CEol* ); /* CR0LF0,CRLF,LF,CR�ŋ�؂���u�s�v��Ԃ��B���s�R�[�h�͍s���ɉ����Ȃ� */
SAKURA_CORE_API const wchar_t* GetNextLineW ( const wchar_t*, int, int*, int*, CEol* ); // GetNextLine��wchar_t��
SAKURA_CORE_API void GetLineColm( const wchar_t*, int*, int* );
SAKURA_CORE_API int IsNumber( const wchar_t*, int, int );/* ���l�Ȃ炻�̒�����Ԃ� */	//@@@ 2001.02.17 by MIK


SAKURA_CORE_API int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith);


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

