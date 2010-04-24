#pragma once

// 2007.10.19 kobake
// string.h �Œ�`����Ă���֐����g�������悤�ȃ��m�B


/*
	++ ++ �����Q�l(�K���ł͖���) ++ ++

	�W���֐�������p
	�`_s:  �o�b�t�@�I�[�o�[�t���[�l���� (��: strcpy_s)
	�`i�`: �啶����������ʖ�����       (��: stricmp)

	�Ǝ�
	auto_�`:  �����̌^�ɂ��A�����ŏ��������肳���� (��: auto_strcpy)
*/

#include "util/tchar_printf.h"
#include "charset/charcode.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ������                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// ������R�s�[�╶�����r�̍ۂɁAmem�n�֐����g���Ă���ӏ������X����܂����A
// mem�n�֐���void�|�C���^���󂯎��A�^�`�F�b�N���s���Ȃ��̂Ŋ댯�ł��B
// �����ɁA�^�`�F�b�N�t����mem�n�݊��̊֐����쐬���܂����B�c�Ə��������ǁA���ۂ̃v���g�^�C�v�͂����Ɖ��̂ق��ɁB�B(auto_mem�`)
// (���Ώۂ��������Ȃ̂ŁA�������������Ƃ����T�O�͖������A
//    �֋X��AACHAR�n�ł�1�o�C�g�P�ʂ��AWCHAR�n�ł�2�o�C�g�P�ʂ��A
//    �����Ƃ݂Ȃ��ď������s���A�Ƃ������Ƃ�)

//��������r
inline int amemcmp(const ACHAR* p1, const ACHAR* p2, size_t count){ return ::memcmp(p1,p2,count); }

//�啶������������ʂ����Ƀ�������r
inline int amemicmp(const ACHAR* p1, const ACHAR* p2, size_t count){ return ::memicmp(p1,p2,count); }
       int wmemicmp(const WCHAR* p1, const WCHAR* p2, size_t count);
       int wmemicmp(const WCHAR* p1, const WCHAR* p2 );
       int wmemicmp_ascii(const WCHAR* p1, const WCHAR* p2, size_t count);

//���̊֐��Ɠ����V�O�j�`���ŁB
//������ȊO�̃�����������mem�`�n�֐����g����ʂł́A���̊֐����g���Ă����ƁA�Ӗ��������͂����肵�ėǂ��B
inline void* memset_raw(void* dest, int c, size_t size){ return ::memset(dest,c,size); }
inline void* memcpy_raw(void* dest, const void* src, size_t size){ return ::memcpy(dest,src,size); }


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�����ϊ�
inline int my_toupper( int c ){ return (((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c); }
inline int my_tolower( int c ){ return (((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c); }
inline int my_towupper( int c ){ return (((c) >= L'a') && ((c) <= L'z')) ? ((c) - L'a' + L'A') : (c); }
inline int my_towlower( int c ){ return (((c) >= L'A') && ((c) <= L'Z')) ? ((c) - L'A' + L'a') : (c); }

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �g���E�Ǝ�����                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//�������̏���t���R�s�[
LPWSTR wcscpyn(LPWSTR lpString1,LPCWSTR lpString2,int iMaxLength); //iMaxLength�͕����P�ʁB

//	Apr. 03, 2003 genta
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count);

//�啶������������ʂ����ɕ����������
const WCHAR* wcsistr( const WCHAR* s1, const WCHAR* s2 );
const ACHAR* stristr( const ACHAR* s1, const ACHAR* s2 );
inline WCHAR* wcsistr( WCHAR* s1, const WCHAR* s2 ){ return const_cast<WCHAR*>(wcsistr(static_cast<const WCHAR*>(s1),s2)); }
inline ACHAR* stristr( ACHAR* s1, const ACHAR* s2 ){ return const_cast<ACHAR*>(stristr(static_cast<const ACHAR*>(s1),s2)); }
#ifdef _UNICODE
#define _tcsistr wcsistr
#else
#define _tcsistr stristr
#endif

//�啶������������ʂ����ɕ�����������i���{��Ή��Łj
const char* strchr_j(const char* s1, char c);				//!< strchr �̓��{��Ή��ŁB
const char* strichr_j( const char* s1, char c );			//!< strchr �̑啶�����������ꎋ�����{��Ή��ŁB
const char* strstr_j(const char* s1, const char* s2);		//!< strstr �̓��{��Ή��ŁB
const char* stristr_j( const char* s1, const char* s2 );	//!< strstr �̑啶�����������ꎋ�����{��Ή��ŁB
inline char* strchr_j ( char* s1, char c         ){ return const_cast<char*>(strchr_j ((const char*)s1, c )); }
inline char* strichr_j( char* s1, char c         ){ return const_cast<char*>(strichr_j((const char*)s1, c )); }
inline char* strstr_j ( char* s1, const char* s2 ){ return const_cast<char*>(strstr_j ((const char*)s1, s2)); }
inline char* stristr_j( char* s1, const char* s2 ){ return const_cast<char*>(stristr_j((const char*)s1, s2)); }
#ifdef _UNICODE
#define _tcsistr_j wcsistr
#else
#define _tcsistr_j stristr_j
#endif

template <class CHAR_TYPE>
CHAR_TYPE* my_strtok(
	CHAR_TYPE*			pBuffer,	//[in] ������o�b�t�@(�I�[�����邱��)
	int					nLen,		//[in] ������̒���
	int*				pnOffset,	//[in/out] �I�t�Z�b�g
	const CHAR_TYPE*	pDelimiter	//[in] ��؂蕶��
);


// �� �V�O�j�`������ѓ���d�l�͕ς��Ȃ����ǁA
// �R���p�C���ƌ���w��ɂ���ĕs����������Ă��܂����Ƃ�������邽�߂�
// �Ǝ��Ɏ��������������́B
int my_stricmp( const char *s1, const char *s2 );
int my_strnicmp( const char *s1, const char *s2, size_t n );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �݊�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// VS2005�ȍ~�̈��S�ŕ�����֐�
#if _MSC_VER<1400 //VS2005���O�Ȃ�
	typedef int error_t;
	error_t wcscat_s(wchar_t* szDst, size_t nDstCount, const wchar_t* szSrc);
	
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//        auto�n�i_UNICODE ��`�Ɉˑ����Ȃ��֐��j              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//char�^�ɂ��邩wchar_t�^�ɂ��邩�m�肵�Ȃ��ϐ�������܂��B
//���L�֐��Q���g���ĕ����񑀍���s�����ꍇ�A
//�����A���̕ϐ��̌^���ς���Ă��A���̑���ӏ������������Ȃ��Ă�
//�ςނ��ƂɂȂ�܂��B
//
//�����L���X�g�ɂ��g�p�͐������܂���B
//���������A���̊֐��Ăяo���Ɍ��炸�A�����L���X�g�͍Œ���ɗ��߂Ă��������B
//���������́AC++�̌��i�Ȍ^�`�F�b�N�̉��b���󂯂邱�Ƃ��ł��Ȃ��Ȃ�܂��B


//�]���n
inline ACHAR* auto_memcpy(ACHAR* dest, const ACHAR* src, size_t count){        ::memcpy (dest,src,count); return dest; }
inline WCHAR* auto_memcpy(WCHAR* dest, const WCHAR* src, size_t count){ return ::wmemcpy(dest,src,count);              }
inline ACHAR* auto_strcpy(ACHAR* dst, const ACHAR* src){ return strcpy(dst,src); }
inline WCHAR* auto_strcpy(WCHAR* dst, const WCHAR* src){ return wcscpy(dst,src); }
inline errno_t auto_strcpy_s(ACHAR* dst, size_t nDstCount, const ACHAR* src){ return strcpy_s(dst,nDstCount,src); }
inline errno_t auto_strcpy_s(WCHAR* dst, size_t nDstCount, const WCHAR* src){ return wcscpy_s(dst,nDstCount,src); }
inline ACHAR* auto_strncpy(ACHAR* dst,const ACHAR* src,size_t count){ return strncpy(dst,src,count); }
inline WCHAR* auto_strncpy(WCHAR* dst,const WCHAR* src,size_t count){ return wcsncpy(dst,src,count); }
inline ACHAR* auto_memset(ACHAR* dest, ACHAR c, size_t count){        memset (dest,c,count); return dest; }
inline WCHAR* auto_memset(WCHAR* dest, WCHAR c, size_t count){ return wmemset(dest,c,count);              }

//��r�n
inline int auto_memcmp (const ACHAR* p1, const ACHAR* p2, size_t count){ return amemcmp(p1,p2,count); }
inline int auto_memcmp (const WCHAR* p1, const WCHAR* p2, size_t count){ return wmemcmp(p1,p2,count); }
inline int auto_strcmp (const ACHAR* p1, const ACHAR* p2){ return strcmp(p1,p2); }
inline int auto_strcmp (const WCHAR* p1, const WCHAR* p2){ return wcscmp(p1,p2); }
inline int auto_strncmp(const ACHAR* str1, const ACHAR* str2, size_t count){ return strncmp(str1,str2,count); }
inline int auto_strncmp(const WCHAR* str1, const WCHAR* str2, size_t count){ return wcsncmp(str1,str2,count); }

//��r�n�iASCII, UCS2 ��p�j
inline int auto_memicmp(const ACHAR* p1, const ACHAR* p2, size_t count){ return amemicmp(p1,p2,count); }
inline int auto_memicmp(const WCHAR* p1, const WCHAR* p2, size_t count){ return wmemicmp(p1,p2,count); }

//��r�n�iSJIS, UTF-16 ��p)
inline int auto_strnicmp(const ACHAR* p1, const ACHAR* p2, size_t count){ return my_strnicmp(p1,p2,count); }
inline int auto_strnicmp(const WCHAR* p1, const WCHAR* p2, size_t count){ return wmemicmp(p1,p2,count); } // Stub.
inline int auto_stricmp(const ACHAR* p1, const ACHAR* p2){ return my_stricmp(p1,p2); }
inline int auto_stricmp(const WCHAR* p1, const WCHAR* p2){ return wmemicmp(p1,p2); } // Stub.

//�����v�Z�n
inline size_t auto_strlen(const ACHAR* str){ return strlen(str); }
inline size_t auto_strlen(const WCHAR* str){ return wcslen(str); }
inline size_t auto_strnlen(const ACHAR* str, size_t count){ return strnlen(str, count); }
inline size_t auto_strnlen(const WCHAR* str, size_t count){ return wcsnlen(str, count); }

//�����n�iSJIS, UCS2 ��p�j
inline const ACHAR* auto_strstr(const ACHAR* str, const ACHAR* strSearch){ return ::strstr_j(str,strSearch); }
inline const WCHAR* auto_strstr(const WCHAR* str, const WCHAR* strSearch){ return ::wcsstr  (str,strSearch); }
inline       ACHAR* auto_strstr(      ACHAR* str, const ACHAR* strSearch){ return ::strstr_j(str,strSearch); }
inline       WCHAR* auto_strstr(      WCHAR* str, const WCHAR* strSearch){ return ::wcsstr  (str,strSearch); }
inline const ACHAR* auto_strchr(const ACHAR* str, ACHAR c){ return ::strchr_j(str,c); }
inline const WCHAR* auto_strchr(const WCHAR* str, WCHAR c){ return ::wcschr  (str,c); }
inline       ACHAR* auto_strchr(      ACHAR* str, ACHAR c){ return ::strchr_j(str,c); }
inline       WCHAR* auto_strchr(      WCHAR* str, WCHAR c){ return ::wcschr  (str,c); }

//�ϊ��n
inline long auto_atol(const ACHAR* str){ return atol(str);  }
inline long auto_atol(const WCHAR* str){ return _wtol(str); }
ACHAR* tcstostr( ACHAR* dest, const TCHAR* src, size_t count );
WCHAR* tcstostr( WCHAR* dest, const TCHAR* src, size_t count );
TCHAR* strtotcs( TCHAR* dest, const ACHAR* src, size_t count );
TCHAR* strtotcs( TCHAR* dest, const WCHAR* src, size_t count );

//�󎚌n
inline int auto_snprintf_s(ACHAR* buf, size_t count, const ACHAR* format, ...)   { va_list v; va_start(v,format); int ret=tchar_vsnprintf_s (buf,count,format,v); va_end(v); return ret; }
inline int auto_snprintf_s(WCHAR* buf, size_t count, const WCHAR* format, ...)   { va_list v; va_start(v,format); int ret=tchar_vsnwprintf_s(buf,count,format,v); va_end(v); return ret; }
inline int auto_sprintf(ACHAR* buf, const ACHAR* format, ...)                    { va_list v; va_start(v,format); int ret=tchar_vsprintf (buf,format,v); va_end(v); return ret; }
inline int auto_sprintf(WCHAR* buf, const WCHAR* format, ...)                    { va_list v; va_start(v,format); int ret=tchar_vswprintf(buf,format,v); va_end(v); return ret; }
inline int auto_sprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, ...){ va_list v; va_start(v,format); int ret=tchar_vsprintf_s (buf,nBufCount,format,v); va_end(v); return ret; }
inline int auto_sprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, ...){ va_list v; va_start(v,format); int ret=tchar_vswprintf_s(buf,nBufCount,format,v); va_end(v); return ret; }
inline int auto_vsprintf(ACHAR* buf, const ACHAR* format, va_list& v){ return tchar_vsprintf (buf,format,v); }
inline int auto_vsprintf(WCHAR* buf, const WCHAR* format, va_list& v){ return tchar_vswprintf(buf,format,v); }
inline int auto_vsprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v){ return tchar_vsprintf_s (buf, nBufCount, format, v); }
inline int auto_vsprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v){ return tchar_vswprintf_s(buf, nBufCount, format, v); }


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �����R�[�h�ϊ�                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <vector>

//SJIS��UNICODE�B�I�[��L'\0'��t���Ă����ŁB
size_t mbstowcs2(wchar_t* dst,const char* src,size_t dst_count);
size_t mbstowcs2(wchar_t* pDst, int nDstCount, const char* pSrc, int nSrcCount);

//UNICODE��SJIS�B�I�[��'\0'��t���Ă����ŁB
size_t wcstombs2(char* dst,const wchar_t* src,size_t dst_count);

//SJIS��UNICODE�B
wchar_t*	mbstowcs_new(const char* pszSrc);								//�߂�l��new[]�Ŋm�ۂ��ĕԂ��B�g���I�������delete[]���邱�ƁB
wchar_t*	mbstowcs_new(const char* pSrc, int nSrcLen);					//�߂�l��new[]�Ŋm�ۂ��ĕԂ��B�g���I�������delete[]���邱�ƁB
void		mbstowcs_vector(const char* src, std::vector<wchar_t>* ret);	//�߂�l��vector�Ƃ��ĕԂ��B
void		mbstowcs_vector(const char* pSrc, int nSrcLen, std::vector<wchar_t>* ret);	//�߂�l��vector�Ƃ��ĕԂ��B

//UNICODE��SJIS
char*	wcstombs_new(const wchar_t* src); //�߂�l��new[]�Ŋm�ۂ��ĕԂ��B
char*	wcstombs_new(const wchar_t* pSrc,int nSrcLen); //�߂�l��new[]�Ŋm�ۂ��ĕԂ��B
void	wcstombs_vector(const wchar_t* pSrc, std::vector<char>* ret); //�߂�l��vector�Ƃ��ĕԂ��B
void	wcstombs_vector(const wchar_t* pSrc, int nSrcLen, std::vector<char>* ret); //�߂�l��vector�Ƃ��ĕԂ��B

//TCHAR
size_t _tcstowcs(WCHAR* wszDst, const TCHAR* tszSrc, size_t nDstCount);
size_t _tcstombs(CHAR*  szDst,  const TCHAR* tszSrc, size_t nDstCount);
size_t _wcstotcs(TCHAR* tszDst, const WCHAR* wszSrc, size_t nDstCount);
size_t _mbstotcs(TCHAR* tszDst, const CHAR*  szSrc,  size_t nDstCount);
int _tctomb(const TCHAR* p,ACHAR* mb);
int _tctowc(const TCHAR* p,WCHAR* wc);





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       ���e������r                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// ���e�����Ƃ̕������r�̍ۂɁA��ł��ŕ���������͂���̂�
// ��Ԃ��|�����ɁA�ێ琫�����Ȃ���̂ŁA
// �J�v�Z�������ꂽ�֐���}�N���ɏ�����C����̂��]�܂����B

//wcsncmp�̕������w���szData2����wcslen�Ŏ擾���Ă�����
inline int wcsncmp_auto(const wchar_t* strData1, const wchar_t* szData2)
{
	return wcsncmp(strData1,szData2,wcslen(szData2));
}

//wcsncmp�̕������w���literalData2�̑傫���Ŏ擾���Ă�����
#define wcsncmp_literal(strData1, literalData2) \
	::wcsncmp(strData1, literalData2, _countof(literalData2) - 1 ) //���I�[�k�����܂߂Ȃ��̂ŁA_countof����}�C�i�X1����

//strncmp�̕������w���literalData2�̑傫���Ŏ擾���Ă�����
#define strncmp_literal(strData1, literalData2) \
	::strncmp(strData1, literalData2, _countof(literalData2) - 1 ) //���I�[�k�����܂߂Ȃ��̂ŁA_countof����}�C�i�X1����

//TCHAR
#ifdef _UNICODE
	#define _tcsncmp_literal wcsncmp_literal
#else
	#define _tcsncmp_literal strncmp_literal
#endif
