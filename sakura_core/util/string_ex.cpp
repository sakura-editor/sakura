#include "StdAfx.h"
#include "string_ex.h"
#include "charset/charcode.h"
#include "util/std_macro.h"
#include <limits.h>

int __cdecl my_internal_icmp( const char *s1, const char *s2, unsigned int n, unsigned int dcount, bool flag );



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       �g���E�Ǝ�����                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	�啶���������𓯈ꎋ���镶�����r������B
	@param s1 [in] ������P
	@param s2 [in] ������Q

	@retval 0	��v
 */
int __cdecl my_stricmp( const char *s1, const char *s2 )
{
	/* �`�F�b�N���镶������uint�ő�ɐݒ肷�� */
	//return my_internal_icmp( s1, s2, (unsigned int)(~0), 0, true );
	return my_internal_icmp( s1, s2, UINT_MAX, 0, true );
}

/*!	�啶���������𓯈ꎋ���镶���񒷂�������r������B
	@param s1 [in] ������P
	@param s2 [in] ������Q
	@param n [in] ������

	@retval 0	��v
 */
int __cdecl my_strnicmp( const char *s1, const char *s2, size_t n )
{
	return my_internal_icmp( s1, s2, (unsigned int)n, 1, true );
}


LPWSTR wcscpyn(LPWSTR lpString1,LPCWSTR lpString2,int iMaxLength)
{
	size_t len2=wcslen(lpString2);
	if((int)len2>iMaxLength-1)len2=iMaxLength-1;
	wmemcpy(lpString1,lpString2,len2);
	lpString1[len2]=L'\0';
	return lpString1;
}


/*
	TCHAR �� WCHAR �܂��� ACHAR �̕ϊ��֐�
*/

ACHAR* tcstostr( ACHAR* dest, const TCHAR* src, size_t count){
	TCHAR* pr = const_cast<TCHAR*>(src);
	ACHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<ACHAR>(*pr);
		++pw;
	}
	return pw;
}
WCHAR* tcstostr( WCHAR* dest, const TCHAR* src, size_t count){
	TCHAR* pr = const_cast<TCHAR*>(src);
	WCHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<WCHAR>(*pr);
		++pw;
	}
	return pw;
}

TCHAR* strtotcs( TCHAR* dest, const ACHAR* src, size_t count )
{
	ACHAR* pr = const_cast<ACHAR*>(src);
	TCHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<TCHAR>(*pr);
		++pw;
	}
	return pw;
}
TCHAR* strtotcs( TCHAR* dest, const WCHAR* src, size_t count )
{
	WCHAR* pr = const_cast<WCHAR*>(src);
	TCHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<TCHAR>(*pr);
		++pw;
	}
	return pw;
}


/*! �����������@�\�t��strncpy

	�R�s�[��̃o�b�t�@�T�C�Y������Ȃ��悤��strncpy����B
	�o�b�t�@���s������ꍇ�ɂ�2�o�C�g�����̐ؒf�����蓾��B
	������\0�͕t�^����Ȃ����A�R�s�[�̓R�s�[��o�b�t�@�T�C�Y-1�܂łɂ��Ă����B

	@param dst [in] �R�s�[��̈�ւ̃|�C���^
	@param dst_count [in] �R�s�[��̈�̃T�C�Y
	@param src [in] �R�s�[��
	@param src_count [in] �R�s�[���镶����̖���

	@retval ���ۂɃR�s�[���ꂽ�R�s�[��̈��1����w���|�C���^

	@author genta
	@date 2003.04.03 genta
*/
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	auto_memcpy( dst, src, src_count );
	return dst + src_count;
}


const wchar_t* wcsistr( const wchar_t* s1, const wchar_t* s2 )
{
	size_t len2=wcslen(s2);
	const wchar_t* p=s1;
	const wchar_t* q=wcschr(s1,L'\0')-len2;
	while(p<=q){
		if(auto_memicmp(p,s2,len2)==0)return p;
		p++;
	}
	return NULL;
}

const char* stristr(const char* s1, const char* s2)
{
	//$ ���{��l�����ĂȂ��̂ŁA����܂���ɗ����Ȃ��ŁBstristr_j���g���̂��]�܂����B
	size_t len2=strlen(s2);
	const char* p=s1;
	const char* q=strchr(s1,L'\0')-len2;
	while(p<=q){
		if(auto_memicmp(p,s2,len2)==0)return p;
		p++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK    �V�K�쐬
	@date 2007.10.21 kobake �֐����ύX: my_strchri��strichr_j
*/
const char* strichr_j( const char* s1, char c2 )
{
	if(c2==0)return ::strchr(s1,0); //������I�[��T�����߂�c2��0��n�����ꍇ���A���������������悤�ɁB 2007.10.16 kobake

	int C2 = my_toupper( c2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_toupper( *p1 ) == C2 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK    �V�K�쐬
	@date 2007.10.21 kobake �֐����ύX: my_strchr��strchr_j
*/
const char* strchr_j(const char* str, char c)
{
	if(c==0)return ::strchr(str,0); //������I�[��T�����߂�c��0��n�����ꍇ���A���������������悤�ɁB 2007.10.16 kobake

	for( const char* p1 = str; *p1; p1++ ){
		if( *p1 == c ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	strstr()��2byte code�Ή���

	@date 2005.04.07 MIK �V�K�쐬
	@date 2007.10.21 kobake �֐����ύX: my_strstr��strstr_j
*/
const char* strstr_j(const char* s1, const char* s2)
{
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( strncmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	strstr()�̑啶�����������ꎋ��

	@note
	Windows API�ɂ���StrStrI��IE4�������Ă��Ȃ�PC�ł͎g�p�s�̂���
	�Ǝ��ɍ쐬

	@date 2005.04.07 MIK    �V�K�쐬
	@date 2007.10.21 kobake �֐����ύX: my_strstri��stristr_j
*/
const char* stristr_j( const char* s1, const char* s2 )
{
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_strnicmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �݊�                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#if _MSC_VER<1400 //VS2005���O�Ȃ�
errno_t wcscat_s(wchar_t* szDst, size_t nDstCount, const wchar_t* szSrc)
{
	// �{���� _set_invalid_parameter_handler �Őݒ肳�ꂽ�n���h�����N�����܂�
	if(!szDst)return EINVAL;
	if(!szSrc)return EINVAL;

	size_t nDstLen=wcslen(szDst);
	if(nDstLen>=nDstCount)return EINVAL;

	size_t nSrcCount=wcslen(szSrc)+1;
	wchar_t* p=&szDst[nDstLen];           //�ǉ��ꏊ
	int nRestCount = nDstCount-(p-szDst); //szDst�ɒǉ��ł���v�f��

	//�͂ݏo���Ȃ�
	if((int)nSrcCount<=nRestCount){
		wmemmove(p,szSrc,nSrcCount);
	}
	//�͂ݏo��
	else{
		return ERANGE;
		//wmemmove(p,szSrc,nRestCount-1); p[nRestCount-1]=L'\0';
	}

	return 0;
}

errno_t strcat_s(char *dest, size_t num, const char *src)
{
	if(!dest || !src) return EINVAL;
	size_t size1 = strnlen(dest, num);
	if(size1 == num) return EINVAL; // dest�����I��
	if(num <= size1+strlen(src)) return ERANGE;
	strcat(dest, src);
	return 0;
}
errno_t strcpy_s(char *dest, size_t num, const char *src)
{
	if(!dest || !src) return EINVAL;
	if(num <= strlen(src)) return ERANGE;
	strcpy(dest, src);
	return 0;
}
errno_t wcscpy_s(wchar_t *dest, size_t num, const wchar_t *src)
{
	if(!dest || !src) return EINVAL;
	if(num <= wcslen(src)) return ERANGE;
	wcscpy(dest, src);
	return 0;
}
errno_t strncpy_s(char *dest, size_t num, const char *src, size_t count)
{
	if(!dest || !src) return EINVAL;
	if(num == 0) return EINVAL;
	if(count == _TRUNCATE) {
		memcpy(dest, src, std::min(num, strlen(src)+1));
		dest[num-1] = '\0';
	} else {
		if(num <= count) { *dest = '\0'; return EINVAL; }
		memcpy(dest, src, count+1);
	}
	return 0;
}
errno_t wcsncpy_s(wchar_t *dest, size_t num, const wchar_t *src, size_t count)
{
	if(!dest || !src) return EINVAL;
	if(num == 0) return EINVAL;
	if(count == _TRUNCATE) {
		wmemcpy(dest, src, std::min(num, wcslen(src)+1));
		dest[num-1] = L'\0';
	} else {
		if(num <= count) { *dest = L'\0'; return EINVAL; }
		wmemcpy(dest, src, count+1);
	}
	return 0;
}
size_t strnlen(const char *str, size_t num)
{
	for(size_t i = 0; i < num; ++i)
	{
		if(str[i] == '\0') return i;
	}
	return num;
}
size_t wcsnlen(const wchar_t *str, size_t num)
{
	for(size_t i = 0; i < num; ++i)
	{
		if(str[i] == L'\0') return i;
	}
	return num;
}
int vsprintf_s(char *buf, size_t num, const char *fmt, va_list vaarg)
{
	// �蔲��
	if(!buf || num == 0 || !fmt) { errno = EINVAL; return -1; }
	buf[num-1] = '\0';
	return _vsnprintf(buf, num-1, fmt, vaarg);
}
int vswprintf_s(wchar_t *buf, size_t num, const wchar_t *fmt, va_list vaarg)
{
	// �蔲��
	if(!buf || num == 0 || !fmt) { errno = EINVAL; return -1; }
	buf[num-1] = L'\0';
	return _vsnwprintf(buf, num-1, fmt, vaarg);
}
int vsnprintf_s(char *buf, size_t num, size_t count, const char *fmt, va_list vaarg)
{
	// �蔲��
	if(!buf || num == 0 || !fmt) { errno = EINVAL; return -1; }
	buf[num-1] = L'\0';
	return _vsnprintf(buf, num-1, fmt, vaarg);
}
int _vsnwprintf_s(wchar_t *buf, size_t num, size_t count, const wchar_t *fmt, va_list vaarg)
{
	// �蔲��
	if(!buf || num == 0 || !fmt) { errno = EINVAL; return -1; }
	buf[num-1] = L'\0';
	return _vsnwprintf(buf, num-1, fmt, vaarg);
}
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �����R�[�h�ϊ�                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


//SJIS��UNICODE�B�I�[��L'\0'��t���Ă����ŁB
size_t mbstowcs2(wchar_t* dst,const char* src,size_t dst_count)
{
	size_t ret=::mbstowcs(dst,src,dst_count-1);
	dst[ret]=L'\0';
	return ret;
}
size_t mbstowcs2(wchar_t* pDst, int nDstCount, const char* pSrc, int nSrcCount)
{
	int ret=MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcCount,
		pDst,
		nDstCount-1
	);
	pDst[ret]=L'\0';
	return (size_t)ret;
}

//UNICODE��SJIS�B�I�[��'\0'��t���Ă����ŁB
size_t wcstombs2(char* dst,const wchar_t* src,size_t dst_count)
{
	size_t ret=::wcstombs(dst,src,dst_count-1);
	dst[ret]='\0';
	return ret;
}

//SJIS��UNICODE�B�߂�l��new[]�Ŋm�ۂ��ĕԂ��B
wchar_t* mbstowcs_new(const char* src)
{
	size_t new_length=mbstowcs(NULL,src,0);
	wchar_t* ret=new wchar_t[new_length+1];
	mbstowcs(ret,src,new_length);
	ret[new_length]=L'\0';
	return ret;
}
wchar_t* mbstowcs_new(const char* pSrc, int nSrcLen)
{
	//�K�v�ȗ̈�T�C�Y
	int nNewLength = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0
	);
	
	//�m��
	wchar_t* pNew = new wchar_t[nNewLength+1];

	//�ϊ�
	nNewLength = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		pNew,
		nNewLength
	);
	pNew[nNewLength] = L'\0';

	return pNew;
}

//UNICODE��SJIS�B�߂�l��new[]�Ŋm�ۂ��ĕԂ��B
char* wcstombs_new(const wchar_t* src)
{
	return wcstombs_new(src,wcslen(src));
}
//�߂�l��new[]�Ŋm�ۂ��ĕԂ��B
char* wcstombs_new(const wchar_t* pSrc,int nSrcLen)
{
	//�K�v�ȗ̈�T�C�Y
	int nNewLength = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	//�m��
	char* pNew = new char[nNewLength+1];

	//�ϊ�
	nNewLength = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		pNew,
		nNewLength,
		NULL,
		NULL
	);
	pNew[nNewLength] = '\0';

	return pNew;
}

//SJIS��UNICODE�B�߂�l��vector�Ƃ��ĕԂ��B
void mbstowcs_vector(const char* src, std::vector<wchar_t>* ret)
{
	mbstowcs_vector(src,strlen(src),ret);
}

//���߂�lret�ɂ����āAret->size()�������񒷂ł͂Ȃ����Ƃɒ��ӁB�������́A(ret->size()-1)�������񒷂ƂȂ�B
void mbstowcs_vector(const char* pSrc, int nSrcLen, std::vector<wchar_t>* ret)
{
	//�K�v�ȗe��
	int nNewLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0
	);

	//�m��
	ret->resize(nNewLen+1);

	//�ϊ�
	nNewLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		&(*ret)[0],
		nNewLen
	);
	(*ret)[nNewLen]=L'\0';
}


//UNICODE��SJIS�B�߂�l��vector�Ƃ��ĕԂ��B
void wcstombs_vector(const wchar_t* src, std::vector<char>* ret)
{
	wcstombs_vector(src,wcslen(src),ret);
}
void wcstombs_vector(const wchar_t* pSrc, int nSrcLen, std::vector<char>* ret)
{
	//�K�v�ȗe��
	int nNewLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0,
		NULL,
		NULL
	);

	//�m��
	ret->resize(nNewLen + 1);

	//�ϊ�
	nNewLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		&(*ret)[0],
		nNewLen,
		NULL,
		NULL
	);
	(*ret)[nNewLen]='\0';
}

#ifdef _UNICODE
	size_t _tcstowcs(WCHAR* wszDst, const TCHAR* tszSrc, size_t nDstCount)
	{
		wcsncpy_s(wszDst, nDstCount, tszSrc, _TRUNCATE);
		return wcslen(wszDst);
	}
	size_t _tcstombs(CHAR*  szDst,  const TCHAR* tszSrc, size_t nDstCount)
	{
		return wcstombs2(szDst, tszSrc, nDstCount);
	}
	size_t _wcstotcs(TCHAR* tszDst, const WCHAR* wszSrc, size_t nDstCount)
	{
		wcsncpy_s(tszDst, nDstCount, wszSrc, _TRUNCATE);
		return wcslen(tszDst);
	}
	size_t _mbstotcs(TCHAR* tszDst, const CHAR*  szSrc,  size_t nDstCount)
	{
		return mbstowcs2(tszDst, szSrc, nDstCount);
	}
	int _tctomb(const TCHAR* p,ACHAR* mb)
	{
		return wctomb(mb,*p);
	}
	int _tctowc(const TCHAR* p,WCHAR* wc)
	{
		*wc=*p;
		return 1;
	}
#else
	size_t _tcstowcs(WCHAR* wszDst, const TCHAR* tszSrc, size_t nDstCount)
	{
		return mbstowcs2(wszDst, tszSrc, nDstCount);
	}
	size_t _tcstombs(CHAR*  szDst,  const TCHAR* tszSrc, size_t nDstCount)
	{
		strncpy_s(szDst, nDstCount, tszSrc, _TRUNCATE);
		return strlen(szDst);
	}
	size_t _wcstotcs(TCHAR* tszDst, const WCHAR* wszSrc, size_t nDstCount)
	{
		return wcstombs2(tszDst, wszSrc, nDstCount);
	}
	size_t _mbstotcs(TCHAR* tszDst, const CHAR*  szSrc,  size_t nDstCount)
	{
		strncpy_s(tszDst, nDstCount, szSrc, _TRUNCATE);
		return strlen(tszDst);
	}
	int _tctomb(const TCHAR* tc,ACHAR* mb)
	{
		mb[0]=tc[0];
		if(_IS_SJIS_1(tc[0])){ mb[1]=tc[1]; return 2; }
		return 1;
	}
	int _tctowc(const TCHAR* tc,WCHAR* wc)
	{
		return mbtowc(wc,tc,_IS_SJIS_1(tc[0])?2:1);
	}
#endif



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          ������                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int wmemicmp(const WCHAR* p1,const WCHAR* p2,size_t count)
{
	for(size_t i=0;i<count;i++){
		int n=skr_towlower(*p1++)-skr_towlower(*p2++);	//��ASCII���ϊ�
		if(n!=0)return n;
	}
	return 0;
}

int wmemicmp(const WCHAR* p1,const WCHAR* p2)
{
	return wmemicmp(p1,p2, t_max(wcslen(p1), wcslen(p2)));
}

int wmemicmp_ascii(const WCHAR* p1,const WCHAR* p2,size_t count)
{
	for(size_t i=0;i<count;i++){
		int n=my_towlower(*p1++)-my_towlower(*p2++);	//ASCII�̂ݕϊ��i�����j
		if(n!=0)return n;
	}
	return 0;
}






/*!
	�󔒂��܂ރt�@�C�������l�������g�[�N���̕���
	
	�擪�ɂ���A��������؂蕶���͖�������D
	
	@return �g�[�N��

	@date 2004.02.15 �݂�   �œK��
	@date 2007.10.21 kobake �e���v���[�g��
*/
//$ ����������Ԃ�����B�B
namespace{
	template <class T> struct Charset{};
	template <> struct Charset<ACHAR>{ static const ACHAR QUOT= '"'; };
	template <> struct Charset<WCHAR>{ static const WCHAR QUOT=L'"'; };
}
template <class CHAR_TYPE>
CHAR_TYPE* my_strtok(
	CHAR_TYPE*			pBuffer,	//[in] ������o�b�t�@(�I�[�����邱��)
	int					nLen,		//[in] ������̒���
	int*				pnOffset,	//[in/out] �I�t�Z�b�g
	const CHAR_TYPE*	pDelimiter	//[in] ��؂蕶��
)
{
	int i = *pnOffset;
	CHAR_TYPE* p;

	do {
		bool bFlag = false;	//�_�u���R�[�e�[�V�����̒����H
		if( i >= nLen ) return NULL;
		p = &pBuffer[i];
		for( ; i < nLen; i++ )
		{
			if( pBuffer[i] == Charset<CHAR_TYPE>::QUOT ) bFlag = ! bFlag;
			if( ! bFlag )
			{
				if( auto_strchr( pDelimiter, pBuffer[i] ) )
				{
					pBuffer[i++] = _T('\0');
					break;
				}
			}
		}
		*pnOffset = i;
	} while( ! *p );	//��̃g�[�N���Ȃ玟��T��
	return p;
}
//�C���X�^���X��
template ACHAR* my_strtok(ACHAR*,int,int*,const ACHAR*);
template WCHAR* my_strtok(WCHAR*,int,int*,const WCHAR*);




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#ifdef MY_ICMP_MBS
int my_mbtoupper2( int c );
int my_mbtolower2( int c );
int my_mbisalpha2( int c );
#endif  /* MY_ICMP_MBS */

#ifdef MY_ICMP_MBS
/*!	�S�p�A���t�@�x�b�g�̂Q�����ڂ�啶���ɕϊ�����B
	@param c [in] �ϊ����镶���R�[�h

	@note
		0x8260 - 0x8279 : �`...�y
		0x8281 - 0x829a : ��...��

	@return �ϊ����ꂽ�����R�[�h
*/
int my_mbtoupper2( int c )
{
	if( c >= 0x81 && c <= 0x9a ) return c - (0x81 - 0x60);
	return c;
}
#endif  /* MY_ICMP_MBS */



#ifdef MY_ICMP_MBS
/*!	�S�p�A���t�@�x�b�g�̂Q�����ڂ��������ɕϊ�����B
	@param c [in] �ϊ����镶���R�[�h

	@return �ϊ����ꂽ�����R�[�h
*/
int my_mbtolower2( int c )
{
	if( c >= 0x60 && c <= 0x79 ) return c + (0x81 - 0x60);
	return c;
}
#endif  /* MY_ICMP_MBS */



#ifdef MY_ICMP_MBS
/*!	�S�p�A���t�@�x�b�g�̂Q�����ڂ����ׂ�B
	@param c [in] �������镶���R�[�h

	@retval 1	�S�p�A���t�@�x�b�g�Q�o�C�g�ڂł���
	@retval 0	������
*/
int my_mbisalpha2( int c )
{
	if( (c >= 0x60 && c <= 0x79) || (c >= 0x81 && c <= 0x9a) ) return 1;
	return 0;
}
#endif  /* MY_ICMP_MBS */




/*!	�啶���������𓯈ꎋ���镶���񒷂�������r������B
	@param s1   [in] ������P
	@param s2   [in] ������Q
	@param n    [in] ������
	@param dcount  [in] �X�e�b�v�l (1=strnicmp,memicmp, 0=stricmp)
	@param flag [in] ������I�[�`�F�b�N (true=stricmp,strnicmp, false=memicmp)

	@retval 0	��v
	@date 2002.11.29 Moca 0�ȊO�̎��̖߂�l���C�u���̒l�̍��v����u�啶���Ƃ����Ƃ��̍��v�ɕύX
 */
int __cdecl my_internal_icmp( const char *s1, const char *s2, unsigned int n, unsigned int dcount, bool flag )
{
	unsigned int	i;
	unsigned char	*p1, *p2;
//	2002.11.29 Moca ���̒l��ێ�����K�v���Ȃ��Ȃ������� *_lo, *_up���폜
//	int	c1, c1_lo, c1_up;
//	int	c2, c2_lo, c2_up;
	int 	c1, c2;
	bool	prev1, prev2; /* �O�̕����� SJIS�̂P�o�C�g�ڂ� */
#ifdef MY_ICMP_MBS
	bool	mba1, mba2;
#endif  /* MY_ICMP_MBS */

	p1 = (unsigned char*)s1;
	p2 = (unsigned char*)s2;
	prev1 = prev2 = false;
#ifdef MY_ICMP_MBS
	mba1 = mba2 = false;
#endif  /* MY_ICMP_MBS */

	/* �w�蒷�����J��Ԃ� */
	for(i = n; i > 0; i -= dcount)
	{
		/* ��r�ΏۂƂȂ镶�����擾���� */
//		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
//		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);
		c1 = (int)((unsigned int)*p1);
		c2 = (int)((unsigned int)*p2);

		/* 2002.11.29 Moca ������̏I�[�ɒB���������ׂ镔�� �͌���ֈړ� */

		/* �����P�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g���� */
		if( prev1 ){	/* �O�̕��������{��P�o�C�g�� */
			/* ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev1 = false;
#ifdef MY_ICMP_MBS
			/* �S�p�����̃A���t�@�x�b�g */
			if( mba1 ){
				mba1 = false;
				if( my_mbisalpha2( c1 ) ){
					c1 = my_mbtoupper2( c1 );
				}
			}
#endif  /* MY_ICMP_MBS */
		}
		else if( my_iskanji1(c1) ){
			/* ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev1 = true;
#ifdef MY_ICMP_MBS
			if( c1 == 0x82 ) mba1 = true;
#endif  /* MY_ICMP_MBS */
		}
		else{
			c1 = my_toupper(c1);
		}

		/* �����Q�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g���� */
		if( prev2 ){	/* �O�̕��������{��P�o�C�g�� */
			/* ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev2 = false;
#ifdef MY_ICMP_MBS
			/* �S�p�����̃A���t�@�x�b�g */
			if( mba2 ){
				mba2 = false;
				if( my_mbisalpha2( c2 ) ){
					c2 = my_mbtoupper2( c2 );
				}
			}
#endif  /* MY_ICMP_MBS */
		}
		else if( my_iskanji1(c2) ){
			/* ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev2 = true;
#ifdef MY_ICMP_MBS
			if( c2 == 0x82 ) mba2 = true;
#endif  /* MY_ICMP_MBS */
		}
		else{
			c2 = my_toupper(c2);
		}

		/* ��r���� */
//		if( (c1_lo - c2_lo) && (c1_up - c2_up) ) return c1 - c2;	/* �߂�l�͌��̕����̍� */
		if( c1 - c2 ) return c1 - c2;	/* �߂�l�͑啶���ɕϊ����������̍� */

		/* 2002.11.29 Moca �߂�l��ύX�������Ƃɂ��C���������啶���ϊ��̌�Ɉړ�
		   �Е����� NULL���� �̏ꍇ�͏�̔�r�������_�� return ���邽�߂��̏����͕s�v */
		if( flag ){
			/* ������̏I�[�ɒB���������ׂ� */
			if( ! c1 ) return 0;
		}
		/* �|�C���^��i�߂� */
		p1++;
		p2++;
	}

	return 0;
}


// skr_towupper() / skr_tolower()
//
// 2010.09.28 ryoji
// BugReport/64: towupper(c) �ɂ���� U+00e0-U+00fc �� U+0020 �����ꎋ�������̑΍�
// VC �̃����^�C���� c < 256 �̏����ł͂Ȃ��� locale �ɑΉ����� "ANSI �n��" �ϊ��e�[�u���������s���Ă���͗l
// �iUnicode �n�ϊ��֐��Ȃ̂� locale �� "Japanese" ���� c < 256 �͈̔͂ł� SJIS �p�炵���ϊ��e�[�u�����g����j
// ����ł͓s���������̂� c < 256 �͈͂̕ϊ��� "English"(Windows-1252) locale �𗘗p����B
//   �EUnicode �̍ŏ��� 256 �̕����ʒu�� Windows-1252 �̐e�ʂ� ISO-8859-1 �R���B
//   �E����� 0x80-0x9F �̋�ԂŁAWindows-1252 �ł͐}�`�����AISO-8859-1(Unicode) �ł͐��䕶���B
// �� �����^�C���� towupper(c)/tolower(c) ���������҂��铮��ɂȂ����Ƃ��Ă����̕��@���g�������Ė�薳���͂�
int skr_towupper( int c )
{
#ifndef __MINGW32__
	static wchar_t szMap[256];	// c < 256 �p�̕ϊ��e�[�u��
	static bool bInit = false;
	if( !bInit ){
		int i;
		_locale_t locale = _create_locale( LC_CTYPE, "English" );
		for( i = 0; i < 0x80; i++ ) szMap[i] = (wchar_t)my_towupper( i );	// ���O�ŕϊ�
		for( ; i < 0xA0; i++ ) szMap[i] = (wchar_t)i;						// ���ϊ��i����R�[�h���j
		for( ; i < 255; i++ ) szMap[i] = _towupper_l( (wchar_t)i, locale );	// "English"locale�ŕϊ�
		szMap[255] = 0x0178;	// Windows-1252 ���� 0x9f(���䕶����) �Ƀ}�b�v���Ă��܂��̂�
		_free_locale( locale );
		bInit = true;
	}

	if( c < 256 ) return szMap[c];
#endif
	return towupper( (wchar_t)c );
}

int skr_towlower( int c )
{
#ifndef __MINGW32__
	static wchar_t szMap[256];	// c < 256 �p�̕ϊ��e�[�u��
	static bool bInit = false;
	if( !bInit ){
		int i;
		_locale_t locale = _create_locale( LC_CTYPE, "English" );
		for( i = 0; i < 0x80; i++ ) szMap[i] = (wchar_t)my_towlower( i );	// ���O�ŕϊ�
		for( ; i < 0xA0; i++ ) szMap[i] = (wchar_t)i;						// ���ϊ��i����R�[�h���j
		for( ; i < 256; i++ ) szMap[i] = _towlower_l( (wchar_t)i, locale );	// "English"locale�ŕϊ�
		_free_locale( locale );
		bInit = true;
	}

	if( c < 256 ) return szMap[c];
#endif
	return towlower( (wchar_t)c );
}
