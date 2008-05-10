/*!	@file
	@brief DLL�̃��[�h�A�A�����[�h

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

#include "stdafx.h"
#include "CDllHandler.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        �����Ɣj��                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CDllImp::CDllImp()
	: m_hInstance( NULL )
{
}

/*!
	�I�u�W�F�N�g���őO��DLL���ǂݍ��܂ꂽ��Ԃł����DLL�̉�����s���D
*/
CDllImp::~CDllImp()
{
	if( IsAvailable() ){
		DeinitDll(true);
	}
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         DLL���[�h                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

EDllResult CDllImp::InitDll(LPCTSTR pszSpecifiedDllName)
{
	if( IsAvailable() ){
		//	���ɗ��p�\�ŗL��Ή������Ȃ��D
		return DLL_SUCCESS;
	}

	//���O�����������؂��A�L���Ȃ��̂��̗p����
	LPCTSTR pszLastName  = NULL;
	bool bInitImpFailure = false;
	for(int i = -1; ;i++)
	{
		//���O���
		LPCTSTR pszName = NULL;
		if(i==-1){ //�܂��͈����Ŏw�肳�ꂽ���O����B
			pszName = pszSpecifiedDllName;
		}
		else{ //�N���X��`��DLL��
			pszName = GetDllNameImp(i);
			//GetDllNameImp����擾�������O�������Ȃ烋�[�v�𔲂���
			if(!pszName || !pszName[0]){
				break;
			}
			//GetDllNameImp����擾�������O���O����Ɠ����Ȃ烋�[�v�𔲂���
			if(pszLastName && _tcsicmp(pszLastName,pszName)==0){
				break;
			}
		}
		pszLastName = pszName;

		//���O�������̏ꍇ�́A���̖��O���������B
		if(!pszName || !pszName[0])continue;

		//DLL���[�h�B���[�h�ł��Ȃ������玟�̖��O���������B
		m_hInstance = ::LoadLibrary(pszName);
		if(!m_hInstance)continue;

		//��������
		bool ret = InitDllImp();

		//���������Ɏ��s�����ꍇ��DLL��������A���̖��O���������B
		if(!ret){
			bInitImpFailure = true;
			::FreeLibrary( m_hInstance );
			m_hInstance = NULL;
			continue;
		}

		//���������ɐ��������ꍇ�́ADLL����ۑ����A���[�v�𔲂���
		if(ret){
			m_strLoadedDllName = pszName;
			break;
		}
	}

	//���[�h�Ə��������ɐ����Ȃ�
	if(IsAvailable()){
		return DLL_SUCCESS;
	}
	//���������Ɏ��s�������Ƃ���������
	else if(bInitImpFailure){
		return DLL_INITFAILURE; //DLL���[�h�͂ł������ǁA���̏��������Ɏ��s
	}
	//����ȊO
	else{
		return DLL_LOADFAILURE; //DLL���[�h���̂Ɏ��s
	}
}

bool CDllImp::DeinitDll(bool force)
{
	if( m_hInstance == NULL || (!IsAvailable()) ){
		//	DLL���ǂݍ��܂�Ă��Ȃ���Ή������Ȃ�
		return true;
	}

	//�I������
	bool ret = DeinitDllImp();
	
	//DLL���
	if( ret || force ){
		//DLL�������
		m_strLoadedDllName = _T("");

		//DLL���
		::FreeLibrary( m_hInstance );
		m_hInstance = NULL;

		return true;
	}
	else{
		return false;
	}
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

LPCTSTR CDllImp::GetLoadedDllName() const
{
	return m_strLoadedDllName.c_str();
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                  �I�[�o�[���[�h�\����                     //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�������ȗ��ł���悤�ɂ��邽�߁A��̊֐���p�ӂ��Ă���
*/
bool CDllImp::DeinitDllImp()
{
	return true;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	�e�[�u���ŗ^����ꂽ�G���g���|�C���^�A�h���X������ꏊ��
	�Ή����镶���񂩂璲�ׂ��G���g���|�C���^��ݒ肷��B
	
	@param table [in] ���O�ƃA�h���X�̑Ή��\�B�Ō��{NULL,0}�ŏI��邱�ƁB
	@retval true �S�ẴA�h���X���ݒ肳�ꂽ�B
	@retval false �A�h���X�̎擾�Ɏ��s�����֐����������B
*/
bool CDllImp::RegisterEntries(const ImportTable table[])
{
	if(!IsAvailable())return false;

	for(int i = 0; table[i].proc!=NULL; i++)
	{
		FARPROC proc;
		if ((proc = ::GetProcAddress(GetInstance(), table[i].name)) == NULL) 
		{
			return false;
		}
		*((FARPROC*)table[i].proc) = proc;
	}
	return true;
}


