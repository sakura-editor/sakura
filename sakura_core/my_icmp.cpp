//	$Id$
/*!	@file
	@brief �Ǝ���r�֐�

	@author MIK
	@date Jan. 11, 2002
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
#include <stdio.h>
#include "my_icmp.h"



/*!	�啶���ɕϊ�����B
	@param c[in] �ϊ����镶���R�[�h

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
	�Ȃ��A���{��1�o�C�g�ڂ��������玟�̕����͕K�����{��2�o�C�g�ڂƔF��
	����̂ŁA�o�C�i���f�[�^�ɂ���Ă͔�r��F���̉\�������邱�Ƃɒ���
	���邱�ƁB
*/
SAKURA_CORE_API int my_memicmp( const void *m1, const void *m2, unsigned int n )
{
	unsigned int	i;
	unsigned char	*p1, *p2;
	int	c1, c1_lo, c1_up;
	int	c2, c2_lo, c2_up;
	int	prev1, prev2;

	p1 = (unsigned char*)m1;
	p2 = (unsigned char*)m2;
	prev1 = prev2 = 0;

	/* �w�蒷�����J��Ԃ� */
	for(i = n; i > 0; i--)
	{
		/* ��r�ΏۂƂȂ镶�����擾���� */
		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);

		/* �����P�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g���� */
		if( prev1 ){	/* �O�̕��������{��P�o�C�g�� */
			/* ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev1 = 0;
			/* ���͓��{��łȂ��ꍇ�A�ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else if( my_iskanji1(c1) ){
			/* ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev1 = 1;
			/* ���{��̂Q�o�C�g�ڂ��s���ł��P�o�C�g�ڂ͕ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else{
			c1_lo = my_tolower(c1);
			c1_up = my_toupper(c1);
		}

		/* �����Q�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g���� */
		if( prev2 ){	/* �O�̕��������{��P�o�C�g�� */
			/* ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev2 = 0;
			/* ���͓��{��łȂ��ꍇ�A�ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else if( my_iskanji1(c2) ){
			/* ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev2 = 1;
			/* ���{��̂Q�o�C�g�ڂ��s���ł��P�o�C�g�ڂ͕ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else{
			c2_lo = my_tolower(c2);
			c2_up = my_toupper(c2);
		}

		/* ��r���� */
		if( (c1_lo - c2_lo) && (c1_up - c2_up) ) return c1 - c2;	/* �߂�l�͌��̕����̍� */

		/* �|�C���^��i�߂� */
		p1++;
		p2++;
	}

	return 0;
}



/*!	�啶���������𓯈ꎋ���镶�����r������B
	@param s1 [in] ������P
	@param s2 [in] ������Q

	@retval 0	��v
 */
SAKURA_CORE_API int my_stricmp( const char *s1, const char *s2 )
{
	unsigned char	*p1, *p2;
	int	c1, c1_lo, c1_up;
	int	c2, c2_lo, c2_up;
	int	prev1, prev2;

	p1 = (unsigned char*)s1;
	p2 = (unsigned char*)s2;
	prev1 = prev2 = 0;

	/* ������̏I�[�܂ŌJ��Ԃ� */
	while(1)
	{
		/* ��r�ΏۂƂȂ镶�����擾���� */
		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);

		/* ������̏I�[�ɒB���������ׂ� */
		if( ! c1 ){
			if( ! c2 ) return 0;
			return 0 - c2;
		}
		else if( ! c2 ){
			return c1;
		}

		/* �����P�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g���� */
		if( prev1 ){	/* �O�̕��������{��P�o�C�g�� */
			/* ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev1 = 0;
			/* ���͓��{��łȂ��ꍇ�A�ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else if( my_iskanji1(c1) ){
			/* ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev1 = 1;
			/* ���{��̂Q�o�C�g�ڂ��s���ł��P�o�C�g�ڂ͕ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else{
			c1_lo = my_tolower(c1);
			c1_up = my_toupper(c1);
		}

		/* �����Q�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g���� */
		if( prev2 ){	/* �O�̕��������{��P�o�C�g�� */
			/* ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev2 = 0;
			/* ���͓��{��łȂ��ꍇ�A�ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else if( my_iskanji1(c2) ){
			/* ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev2 = 1;
			/* ���{��̂Q�o�C�g�ڂ��s���ł��P�o�C�g�ڂ͕ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else{
			c2_lo = my_tolower(c2);
			c2_up = my_toupper(c2);
		}

		/* ��r���� */
		if( (c1_lo - c2_lo) && (c1_up - c2_up) ) return c1 - c2;	/* �߂�l�͌��̕����̍� */

		/* �|�C���^��i�߂� */
		p1++;
		p2++;
	}
	/*NOTREACHED*/
}



/*!	�啶���������𓯈ꎋ���镶���񒷂�������r������B
	@param s1 [in] ������P
	@param s2 [in] ������Q
	@param n [in] ������

	@retval 0	��v
 */
SAKURA_CORE_API int my_strnicmp( const char *s1, const char *s2, size_t n )
{
	unsigned int	i;
	unsigned char	*p1, *p2;
	int	c1, c1_lo, c1_up;
	int	c2, c2_lo, c2_up;
	int	prev1, prev2;

	p1 = (unsigned char*)s1;
	p2 = (unsigned char*)s2;
	prev1 = prev2 = 0;

	/* �w�蒷�����J��Ԃ� */
	for(i = n; i > 0; i--)
	{
		/* ��r�ΏۂƂȂ镶�����擾���� */
		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);

		/* ������̏I�[�ɒB���������ׂ� */
		if( ! c1 ){
			if( ! c2 ) return 0;
			return 0 - c2;
		}
		else if( ! c2 ){
			return c1;
		}

		/* �����P�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g���� */
		if( prev1 ){	/* �O�̕��������{��P�o�C�g�� */
			/* ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev1 = 0;
			/* ���͓��{��łȂ��ꍇ�A�ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else if( my_iskanji1(c1) ){
			/* ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev1 = 1;
			/* ���{��̂Q�o�C�g�ڂ��s���ł��P�o�C�g�ڂ͕ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else{
			c1_lo = my_tolower(c1);
			c1_up = my_toupper(c1);
		}

		/* �����Q�̓��{��`�F�b�N���s����r�p�̑啶�����������Z�b�g���� */
		if( prev2 ){	/* �O�̕��������{��P�o�C�g�� */
			/* ����͓��{��Q�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev2 = 0;
			/* ���͓��{��łȂ��ꍇ�A�ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else if( my_iskanji1(c2) ){
			/* ����͓��{��P�o�C�g�ڂȂ̂ŕϊ����Ȃ� */
			prev2 = 1;
			/* ���{��̂Q�o�C�g�ڂ��s���ł��P�o�C�g�ڂ͕ϊ��͈͂ɂȂ��̂Ŗ��Ȃ� */
		}
		else{
			c2_lo = my_tolower(c2);
			c2_up = my_toupper(c2);
		}

		/* ��r���� */
		if( (c1_lo - c2_lo) && (c1_up - c2_up) ) return c1 - c2;	/* �߂�l�͌��̕����̍� */

		/* �|�C���^��i�߂� */
		p1++;
		p2++;
	}

	return 0;
}



/*[EOF]*/
