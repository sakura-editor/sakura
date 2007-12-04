//�r���h(�R���p�C��)�ݒ�
//2007.10.18 kobake �쐬

#pragma once

/*!
	���i��int���g�����ǂ����B

	��ɃG�f�B�^�����̍��W�n�P�ʂɊւ���
	�R���p�C�����ɐÓI�Ȍ^�`�F�b�N�������悤�ɂȂ�܂��B
	���������̕��R���p�C�����Ԃ�������܂��B

	���s�������͕ω������B
	���s���I�[�o�[�w�b�h�s���B�R���p�C����������΃I�[�o�[�w�b�h�[���B

	�����[�X�r���h�ł͖����ɂ��Ă����Ɨǂ��B

	@date 2007.10.18 kobake
*/
#ifdef _DEBUG
#define USE_STRICT_INT //��������R�����g�A�E�g����ƌ��i��int�������ɂȂ�܂��B�����[�X�r���h�ł͏�ɖ����B
#endif


//! USE_UNFIXED_FONT ���`����ƁA�t�H���g�I���_�C�A���O�œ����t�H���g�ȊO���I�ׂ�悤�ɂȂ�
//#define USE_UNFIXED_FONT


//UNICODE BOOL�萔
#ifdef _UNICODE
static const bool UNICODE_BOOL=true;
#else
static const bool UNICODE_BOOL=false;
#endif


//DebugMonitorLib(��)���g�����ǂ���
//#define USE_DEBUGMON


//new���ꂽ�̈���킴�Ɖ������ǂ��� (�f�o�b�O�p)
#ifdef _DEBUG
#define FILL_STRANGE_IN_NEW_MEMORY
#endif



// -- -- -- -- ���ȏ�A�r���h�ݒ芮�� -- -- -- -- //


//�f�o�b�O���ؗp�Fnew���ꂽ�̈���킴�Ɖ����B2007.11.27 kobake
#ifdef FILL_STRANGE_IN_NEW_MEMORY
	#include <stdlib.h> //malloc,free
	inline void _fill_new_memory(void* p, size_t nSize, const char* pSrc, size_t nSrcLen)
	{
		char* s = (char*)p;
		size_t i;
		for(i = 0; i < nSize; i++)
		{
			*s++ = pSrc[i%nSrcLen];
		}
	}
	inline void* operator new(size_t nSize)
	{
		void* p = ::malloc(nSize);
		_fill_new_memory(p,nSize,"ƭ�",3); //�m�ۂ��ꂽ�΂���̃�������Ԃ́uƭ�ƭ�ƭ��c�v�ƂȂ�܂�
		return p;
	}
	inline void* operator new[](size_t nSize)
	{
		void* p = ::malloc(nSize);
		_fill_new_memory(p,nSize,"�ޭ�",4); //�m�ۂ��ꂽ�΂���̃�������Ԃ́u�ޭ��ޭ��ޭ��c�v�ƂȂ�܂�
		return p;
	}
	inline void operator delete(void* p)
	{
		::free(p);
	}
	inline void operator delete[](void* p)
	{
		::free(p);
	}
#endif
