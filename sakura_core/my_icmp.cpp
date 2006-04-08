//	$Id$
/*!	@file
	@brief �Ǝ��������r�֐�

	@author MIK
	@date Jan. 11, 2002
	@date Feb. 02, 2002  ���������𓝈�A�S�p�A���t�@�x�b�g���ꎋ�ɑΉ�
	@date Apr. 07, 2005  MIK strstr�n�֐���ǉ�
	$Revision$
*/
/*
	Copyright (C) 2002-2005, MIK
	Copyright (C) 2002, Moca

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
/*!
 * �啶���������𓯈ꎋ���镶�����r�A��������r��Ǝ��Ɏ�������B
 *
 *   stricmp, strnicmp, memicmp
 *
 * ����̓R���p�C���ƌ���w��ɂ���ĕs����������Ă��܂����Ƃ�������邽�߂�
 * ���̂ł��B
 * ���{��� SJIS �ł��B
 *
 * stricmp, strnicmp, memicmp (����� _ �t���̂���) ���g�p���Ă���t�@�C����
 * �擪�� #include "my_icmp.h" ��ǉ����܂��B�������A���̃w�b�_�t�@�C������
 * ��ɂȂ�ꏊ�ɒǉ����Ă��������B
 *   ���֐����}�N���Œ�`���������߁B
 */



/*
 * �w�b�_
 */
#include "stdafx.h"
#include <stdio.h>
#include <limits.h>
#include "my_icmp.h"



/*!	�啶���ɕϊ�����B
	@param c [in] �ϊ����镶���R�[�h

	@return �ϊ����ꂽ�����R�[�h
*/
SAKURA_CORE_API MY_INLINE int my_toupper( int c )
{
	if( c >= 'a' && c <= 'z' ) return c - ('a' - 'A');
	return c;
}



/*!	�������ɕϊ�����B
	@param c [in] �ϊ����镶���R�[�h

	@return �ϊ����ꂽ�����R�[�h
*/
SAKURA_CORE_API MY_INLINE int my_tolower( int c )
{
	if( c >= 'A' && c <= 'Z' ) return c + ('a' - 'A');
	return c;
}



/*!	���{��P�o�C�g�ڂ����ׂ�B
	@param c [in] �������镶���R�[�h

	@retval 1	�����P�o�C�g�ڂł���
	@retval 0	�����P�o�C�g�ڂł͂Ȃ�
*/
SAKURA_CORE_API MY_INLINE int my_iskanji1( int c )
{
	if( (c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc) ) return 1;
	return 0;
}



/*!	���{��Q�o�C�g�ڂ����ׂ�B
	@param c [in] �������镶���R�[�h

	@retval 1	�����Q�o�C�g�ڂł���
	@retval 0	�����Q�o�C�g�ڂł͂Ȃ�
*/
SAKURA_CORE_API MY_INLINE int my_iskanji2( int c )
{
	if( (c >= 0x40 && c <= 0x7e) || (c >= 0x80 && c <= 0xfc) ) return 1;
	return 0;
}



#ifdef MY_ICMP_MBS
/*!	�S�p�A���t�@�x�b�g�̂Q�����ڂ�啶���ɕϊ�����B
	@param c [in] �ϊ����镶���R�[�h

	@note
		0x8260 - 0x8279 : �`...�y
		0x8281 - 0x829a : ��...��

	@return �ϊ����ꂽ�����R�[�h
*/
SAKURA_CORE_API MY_INLINE int my_mbtoupper2( int c )
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
SAKURA_CORE_API MY_INLINE int my_mbtolower2( int c )
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
SAKURA_CORE_API MY_INLINE int my_mbisalpha2( int c )
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
SAKURA_CORE_API int __cdecl my_internal_icmp( const char *s1, const char *s2, unsigned int n, unsigned int dcount, bool flag )
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
//					c1_lo = my_mbtolower2( c1 );
//					c1_up = my_mbtoupper2( c1 );
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
//			c1_lo = my_tolower(c1);
//			c1_up = my_toupper(c1);
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
//					c2_lo = my_mbtolower2( c2 );
//					c2_up = my_mbtoupper2( c2 );
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
//			c2_lo = my_tolower(c2);
//			c2_up = my_toupper(c2);
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



/*!	�啶���������𓯈ꎋ���郁������r������B
	@param m1 [in] �f�[�^�P
	@param m2 [in] �f�[�^�Q
	@param n [in] �f�[�^��

	@retval 0	��v

	@note
	�{����������r�̓o�C�i���f�[�^���r������̂ł���A��r�Ώۃf�[�^��
	���{��ł��낤���Ȃ��낤���֌W�Ȃ��B
	�������A�e�L�X�g�f�[�^�Ƀo�C�i�����܂܂�邱�Ƃ����邽�� memicmp ��
	��r���s���Ă���B
	�����ł͓��{��ɑΉ����� memicmp ����������B
*/
SAKURA_CORE_API int __cdecl my_memicmp( const void *m1, const void *m2, unsigned int n )
{
	return my_internal_icmp( (const char*)m1, (const char*)m2, n, 1, false );
}



/*!	�啶���������𓯈ꎋ���镶�����r������B
	@param s1 [in] ������P
	@param s2 [in] ������Q

	@retval 0	��v
 */
SAKURA_CORE_API int __cdecl my_stricmp( const char *s1, const char *s2 )
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
SAKURA_CORE_API int __cdecl my_strnicmp( const char *s1, const char *s2, size_t n )
{
	return my_internal_icmp( s1, s2, (unsigned int)n, 1, true );
}

/*!
	strstr()�̑啶�����������ꎋ��

	@note
	Windows API�ɂ���StrStrI��IE4�������Ă��Ȃ�PC�ł͎g�p�s�̂���
	�Ǝ��ɍ쐬

	@date 2005.04.07 MIK �V�K�쐬
*/
const char* my_strstri( const char* s1, const char* s2 ){
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_strnicmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	strstr()��2byte code�Ή���

	@date 2005.04.07 MIK �V�K�쐬
*/
const char* my_strstr( const char* s1, const char* s2 ){
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( strncmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK �V�K�쐬
*/
const char* my_strchri( const char* s1, int c2 ){
	int C2 = my_toupper( c2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_toupper( *p1 ) == C2 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK �V�K�쐬
*/
const char* my_strchr( const char* s1, int c2 ){
	for( const char* p1 = s1; *p1; p1++ ){
		if( *p1 == c2 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}
/*[EOF]*/
