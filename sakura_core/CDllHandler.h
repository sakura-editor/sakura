//	$Id$
/*!	@file
	DLL�̃��[�h�A�A�����[�h

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta

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

#ifndef _LOAD_LIBRARY_H_
#define _LOAD_LIBRARY_H_

#include <windows.h>
#include "global.h"

//! DLL�̓��I��Load/Unload���s�����߂̃N���X
class SAKURA_CORE_API CDllHandler {
public:
	CDllHandler();
	virtual ~CDllHandler();

	//! ���p��Ԃ̃`�F�b�N
	/*!
		DLL�̊֐����Ăяo���邩��Ԃǂ���

		@retval true ���p�\
		@retval false ���p�s�\
	*/
	virtual bool IsAvailable(void) const { return m_hInstance != NULL; }
	//! DLL�̃��[�h
	/*!
		@retval 0 ����I���BDLL�����[�h���ꂽ�B
		@retval other �ُ�I���BDLL�̓��[�h����Ȃ������B

		@date Jul. 5, 2001 genta �����ǉ��B�p�X�̎w��ȂǂɎg����
	*/
	int LoadLibrary(char* str = NULL);
	//! DLL�̃A�����[�h
	/*!
		@param force [in] �I�������Ɏ��s���Ă�DLL��������邩�ǂ���
	*/
	int FreeLibrary(bool force = false);

	//! ���p��Ԃ̃`�F�b�N�ioperator�Łj
	bool operator!(void) const { return IsAvailable(); }

	//!	DLL�̃��[�h
	/*!
		�ڍׂȖ߂�l��Ԃ��Ȃ����ƈȊO��LoadLibrary()�Ɠ���
	*/
	bool Init(char* str = NULL){ return LoadLibrary(str) == 0; }

	//! �C���X�^���X�n���h���̎擾
	HINSTANCE GetInstance() const { return m_hInstance; }
protected:
	//!	DLL�̏�����
	/*!
		DLL�̃��[�h�ɐ�����������ɌĂяo�����D�G���g���|�C���g��
		�m�F�Ȃǂ��s���D

		@retval 0 ����I��
		@retval other �ُ�I���D�l�̈Ӗ��͎��R�ɐݒ肵�ėǂ��D

		@note 0�ȊO�̒l��Ԃ����ꍇ�́A�ǂݍ���DLL���������D
	*/
	virtual int InitDll(void) = 0;
	//!	�֐��̏�����
	/*!
		DLL�̃A�����[�h���s�����O�ɌĂяo�����D�������̉���Ȃǂ�
		�s���D

		@retval 0 ����I��
		@retval other �ُ�I���D�l�̈Ӗ��͎��R�ɐݒ肵�ėǂ��D

		@note 0�ȊO��Ԃ����Ƃ���DLL��Unload�͍s���Ȃ��D
	*/
	virtual int DeinitDll(void);
	//! DLL�t�@�C�����̎擾
	/*!
		@date Jul. 5, 2001 genta �����ǉ��B�p�X�̎w��ȂǂɎg����
	*/
	virtual char* GetDllName(char*) = 0;

private:
	HINSTANCE m_hInstance;
};

#endif


/*[EOF]*/
