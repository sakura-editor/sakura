//	$Id$
/*!	@file
	@brief �Ǝ���r�֐�

	@author MIK
	@date Jan. 11, 2002
	@date Feb. 02, 2002  ���������𓝈�A�S�p�A���t�@�x�b�g���ꎋ�ɑΉ�
	$Revision$
*/
/*
	Copyright (C) 2002, MIK

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
/*
 * �啶���������𓯈ꎋ���镶�����r�A��������r��Ǝ��Ɏ�������B
 *   stricmp, strnicmp, memicmp
 * ����̓R���p�C���ƌ���w��ɂ���ĕs����������Ă��܂����Ƃ�������邽�߂�
 * ���̂ł��B
 * ���{��� SJIS �ł��B
 *
 * MY_ICMP_MBS @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *  ������`����ƑS�p�̃A���t�@�x�b�g���啶���������̋�ʂ��Ȃ��Ȃ�܂��B
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *
 * stricmp, strnicmp, memicmp (����� _ �t���̂���) ���g�p���Ă���t�@�C����
 * �擪�� #include "my_icmp.h" ��ǉ����܂��B�������A���̃w�b�_�t�@�C������
 * ��ɂȂ�ꏊ�ɒǉ����Ă��������B
 *   ���֐����}�N���Œ�`���������߁B
 */
#ifndef _MY_ICMP_H_
#define _MY_ICMP_H_



/*
 * �w�b�_
 */
#include "global.h"
//#define SAKURA_CORE_API



/*
 * �}�N��
 */
#define MY_ICMP_MBS  /* �}���`�o�C�g�Ή������܂� */
#undef  MY_ICMP_MBS



/* �֐����Ē�`���܂� */
#define  memicmp(a,b,c)    my_memicmp((a),(b),(c))
#define  stricmp(a,b)      my_stricmp((a),(b))
#define  strnicmp(a,b,c)   my_strnicmp((a),(b),(c))

#ifdef   _memicmp
#undef   _memicmp
#endif
#define  _memicmp(a,b,c)   my_memicmp((a),(b),(c))

#ifdef   _stricmp
#undef   _stricmp
#endif
#define  _stricmp(a,b)     my_stricmp((a),(b))

#ifdef   _strnicmp
#undef   _strnicmp
#endif
#define  _strnicmp(a,b,c)  my_strnicmp((a),(b),(c))

#if 0
#define  _mbsicmp(a,b)     my_stricmp((a),(b))
#define  _mbsnicmp(a,b,c)    my_strnicmp((a),(b),(c))
#endif  /* if 0 */

#if 0
#define  toupper(a)        my_toupper((a))
#define  tolower(a)        my_tolower((a))

#ifdef  _toupper
#undef  _toupper
#endif
#define  _toupper(a)       my_toupper((a))

#ifdef  _tolower
#undef  _tolower
#endif
#define  _tolower(a)       my_tolower((a))
#endif  /* if 0 */

#if 0
#define  setlocale(a,b)    
#endif  /* if 0 */

#define  MY_INLINE
//#define  MY_INLINE  inline

// 2005.04.07 MIK strstr�n�֐��ǉ�
#ifdef strstri
#  undef strstri
#endif
#define strstri(a,b)        my_strstri((a),(b))
#ifdef strchri
#  undef strchri
#endif
#define strchri(a,b)        my_strchri((a),(b))


/*
 * �v���g�^�C�v
 */
SAKURA_CORE_API MY_INLINE int my_toupper( int c );
SAKURA_CORE_API MY_INLINE int my_tolower( int c );
SAKURA_CORE_API MY_INLINE int my_iskanji1( int c );
SAKURA_CORE_API MY_INLINE int my_iskanji2( int c );
SAKURA_CORE_API int __cdecl my_internal_icmp( const char *s1, const char *s2, unsigned int n, unsigned int dcount, bool flag );
SAKURA_CORE_API int __cdecl my_memicmp( const void *m1, const void *m2, unsigned int n );
SAKURA_CORE_API int __cdecl my_stricmp( const char *s1, const char *s2 );
SAKURA_CORE_API int __cdecl my_strnicmp( const char *s1, const char *s2, size_t n );
#ifdef MY_ICMP_MBS
SAKURA_CORE_API MY_INLINE int my_mbtoupper2( int c );
SAKURA_CORE_API MY_INLINE int my_mbtolower2( int c );
SAKURA_CORE_API MY_INLINE int my_mbisalpha2( int c );
#endif  /* MY_ICMP_MBS */

// 2005.04.07 MIK strstr�n�֐��ǉ�
//	Aug. 21, 2005 genta ���̂ɂ�__cdecl�������̂ŁC�錾������폜
SAKURA_CORE_API const char* my_strstri( const char* s1, const char* s2 );
SAKURA_CORE_API const char* my_strstr( const char* s1, const char* s2 );
SAKURA_CORE_API const char* my_strchri( const char* s1, int c2 );
SAKURA_CORE_API const char* my_strchr( const char* s1, int c2 );

inline SAKURA_CORE_API char* my_strstri( char* s1, const char* s2 )
{
	return const_cast<char*>(my_strstri((const char*)s1, s2));
}

inline SAKURA_CORE_API char* my_strstr( char* s1, const char* s2 )
{
	return const_cast<char*>(my_strstr((const char*)s1, s2));
}

inline SAKURA_CORE_API char* my_strchri( char* s1, int c2 )
{
	return const_cast<char*>(my_strchri((const char*)s1, c2));
}

inline SAKURA_CORE_API char* my_strchr( char* s1, int c2 )
{
	return const_cast<char*>(my_strchr((const char*)s1, c2));
}

#endif  /* _MY_ICMP_H_ */

