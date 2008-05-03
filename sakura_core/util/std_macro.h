//2007.10.18 kobake �쐬

#pragma once


#define SAFE_DELETE(p) do{ if(p){ delete p; p=0; } }while(0)


/*
	2007.10.18 kobake
	�e���v���[�g�� min �Ƃ� max �Ƃ��B

	�ǂ����̕W���w�b�_�ɁA�����悤�Ȃ��̂��������C�����邯�ǁA
	NOMINMAX ���`����ɂ��Ă��A�Ȃ񂾂� min �Ƃ� max �Ƃ��������O���ƁA
	�e���v���[�g���Ă�ł�񂾂��}�N�����Ă�ł�񂾂��󕪂���Ȃ��̂ŁA
	�����I�Ɂut_�`�v�Ƃ������O�����֐���p�ӁB
*/

template <class T>
T t_min(T t1,T t2)
{
	return t1<t2?t1:t2;
}

template <class T>
T t_max(T t1,T t2)
{
	return t1>t2?t1:t2;
}

template <class T>
T t_abs(T t)
{
	return t>=T(0)?t:T(-t);
}

template <class T>
void t_swap(T& t1, T& t2)
{
	T tmp = t1;
	t1 = t2;
	t2 = tmp;
}



/*
	2007.10.19 kobake
	_countof�}�N���B_countof���g���Ȃ��Â��R���p�C���p�B

	�������A���̏ꏊ�Ńe���v���[�g���育��g���Ă���̂ŁA
	�ǂ����ɂ���Â����Ńr���h�͒ʂ�Ȃ��\���B
*/

#ifndef _countof
#define _countof(A) (sizeof(A)/sizeof(A[0]))
#endif

//sizeof
#define sizeof_raw(V)  sizeof(V)
#define sizeof_type(V) sizeof(V)


/*
	2007.10.19 kobake

	���e�����������A�����w��}�N��
*/

//�r���h��Ɋ֌W�Ȃ��AUNICODE�B
#define __LTEXT(A) L##A
#define LTEXT(A) __LTEXT(A)
#define LCHAR(A) __LTEXT(A)

//�r���h��Ɋ֌W�Ȃ��AANSI�B
#define ATEXT(A) A


