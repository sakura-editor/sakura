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
#ifndef SAKURA_PRIMITIVE_0AE619F1_2A04_42A0_92F6_72C9B845799E_H_
#define SAKURA_PRIMITIVE_0AE619F1_2A04_42A0_92F6_72C9B845799E_H_

// -- -- -- -- �_���^ -- -- -- -- //

// novice 2002/09/14
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
#define BOOL	int
#endif


// -- -- -- -- �萔 -- -- -- -- //

#ifndef NULL
#define NULL 0
#endif


// -- -- -- -- ���� -- -- -- -- //

//char,wchar_t �̕ς��ɁA�ʖ��� ACHAR,WCHAR ���g���ƁA�\�[�X���`�����₷���P�[�X������B
typedef char ACHAR;


//TCHAR�ǉ��@�\
//TCHAR�Ƌt�̕����^��NOT_TCHAR�Ƃ��Ē�`����
#ifdef _UNICODE
typedef char NOT_TCHAR;
#else
typedef wchar_t NOT_TCHAR;
#endif


//WIN_CHAR (WinAPI�ɓn���̂ŁA�K��TCHAR�łȂ���΂Ȃ�Ȃ�����)
typedef TCHAR WIN_CHAR;
#define _WINT(A) _T(A)


//EDIT_CHAR
typedef wchar_t WChar;      //�G�f�B�^�ŗp����e�L�X�g�Ǘ��f�[�^�^
typedef wchar_t EDIT_CHAR;
#define _EDITL(A) LTEXT(A)


//�����R�[�h�ʁA�����^
typedef unsigned char	uchar_t;		//  unsigned char �̕ʖ��D
typedef unsigned short	uchar16_t;		//  UTF-16 �p�D
typedef unsigned long	uchar32_t;		//  UTF-32 �p�D
typedef long			wchar32_t;



// -- -- -- -- ���̑� -- -- -- -- //

typedef char KEYCODE;

//int�݊�
#ifdef USE_STRICT_INT
	#include "CLaxInteger.h"
	typedef CLaxInteger Int;
#else
	typedef int Int;
#endif

#endif /* SAKURA_PRIMITIVE_0AE619F1_2A04_42A0_92F6_72C9B845799E_H_ */
/*[EOF]*/
