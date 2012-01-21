/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_BUILD_CONFIG_26C6FCD0_99D7_4AF6_89C1_F34581417333_H_
#define SAKURA_BUILD_CONFIG_26C6FCD0_99D7_4AF6_89C1_F34581417333_H_

//�r���h(�R���p�C��)�ݒ�
//2007.10.18 kobake �쐬
//2009.09.10 syat ���������[�N�`�F�b�N��ǉ�

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


//crtdbg.h�ɂ�郁�����[���[�N�`�F�b�N���g�����ǂ����i�f�o�b�O�p�j
#ifdef _DEBUG
//#define USE_LEAK_CHECK_WITH_CRTDBG
#endif

// -- -- �d�l�ύX -- -- //

//�S�p�X�y�[�X�`��
//#define NEW_ZENSPACE //�V�����`�惋�[�`�� (�S�p�X�y�[�X��j����`�ŕ`��) ���̗p



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


//crtdbg.h�ɂ�郁�����[���[�N�`�F�b�N���g�����ǂ����i�f�o�b�O�p�j
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	//new���Z�q���I�[�o�[���C�h����w�b�_��crtdbg.h�̑O��include���Ȃ��ƃR���p�C���G���[�ƂȂ�	
	//�Q�l�Fhttp://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=99818
	#include <xiosbase>
	#include <xlocale>
	#include <xmemory>
	#include <xtree>

	#include <crtdbg.h>
	#define new DEBUG_NEW
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
	//����ƁAWinMain�̐擪�� _CrtSetDbgFlag() ���Ă�
#endif

#endif /* SAKURA_BUILD_CONFIG_26C6FCD0_99D7_4AF6_89C1_F34581417333_H_ */
/*[EOF]*/
