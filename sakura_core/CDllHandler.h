//	$Id$
/*!	@file
	DLL�̃��[�h�A�A�����[�h

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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
	*/
	int LoadLibrary(void);
	//! DLL�̃A�����[�h
	/*!
		@param force [in] �I�������Ɏ��s���Ă�DLL��������邩�ǂ���
	*/
	int FreeLibrary(bool force = false);

	//! ���p��Ԃ̃`�F�b�N�ioperator�Łj
	bool operator!(void) const { return IsAvailable(); }

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
	virtual char* GetDllName(void) = 0;

private:
	HINSTANCE m_hInstance;
};

#endif
