/*! @file
	@brief �t�@�C���̎����ۑ�

	@author genta
	@date 2000
*/
/*
	Copyright (C) 2000-2001, genta

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
#include "CAutoSaveAgent.h"





//	From Here Aug. 21, 2000 genta
//
//	�����ۑ����s�����ǂ����̃`�F�b�N
//
void CAutoSaveAgent::CheckAutoSave()
{
	if( m_cPassiveTimer.CheckAction() ){
		CEditDoc* pcDoc = GetListeningDoc();

		//	�㏑���ۑ�

		if( !pcDoc->m_cDocEditor.IsModified() )	//	�ύX�����Ȃ牽�����Ȃ�
			return;				//	�����ł́C�u���ύX�ł��ۑ��v�͖�������

		//	2003.10.09 zenryaku �ۑ����s�G���[�̗}��
		if( !pcDoc->m_cDocFile.GetFilePathClass().IsValidPath() )	//	�܂��t�@�C�������ݒ肳��Ă��Ȃ���Εۑ����Ȃ�
			return;

		bool en = m_cPassiveTimer.IsEnabled();
		m_cPassiveTimer.Enable(false);	//	2�d�Ăяo����h������
		pcDoc->m_cDocFileOperation.FileSave();	//	�ۑ�
		m_cPassiveTimer.Enable(en);
	}
}

//
//	�ݒ�ύX�������ۑ�����ɔ��f����
//
void CAutoSaveAgent::ReloadAutoSaveParam()
{
	m_cPassiveTimer.SetInterval( GetDllShareData().m_Common.m_sBackup.GetAutoBackupInterval() );
	m_cPassiveTimer.Enable( GetDllShareData().m_Common.m_sBackup.IsAutoBackupEnabled() );
}


//----------------------------------------------------------
//	class CPassiveTimer
//
//----------------------------------------------------------
/*!
	���ԊԊu�̐ݒ�
	@param m �Ԋu(min)
	�Ԋu��0�ȉ��ɐݒ肵���Ƃ���1�b�Ƃ݂Ȃ��B�ݒ�\�ȍő�Ԋu��35792���B
*/
void CPassiveTimer::SetInterval(int m)
{
	if( m <= 0 )
		m = 1;
	else if( m >= 35792 )	//	35792���ȏゾ�� int �ŕ\���ł��Ȃ��Ȃ�
		m = 35792;

	nInterval = m * MSec2Min;
}
/*!
	�^�C�}�[�̗L���E�����̐؂�ւ�
	@param flag true:�L�� / false: ����
	�������L���ɐ؂�ւ����Ƃ��̓��Z�b�g�����B
*/
void CPassiveTimer::Enable(bool flag)
{
	if( bEnabled != flag ){	//	�ύX������Ƃ�
		bEnabled = flag;
		if( flag ){	//	enabled
			Reset();
		}
	}
}
/*!
	�O���Œ���Ɏ��s�����Ƃ��납��Ăяo�����֐��B
	�Ăяo�����ƌo�ߎ��Ԃ��`�F�b�N����B

	@retval true ���莞�Ԃ��o�߂����B���̂Ƃ��͑����������I�Ƀ��Z�b�g�����B
	@retval false ����̎��ԂɒB���Ă��Ȃ��B
*/
bool CPassiveTimer::CheckAction(void)
{
	if( !IsEnabled() )	//	�L���łȂ���Ή������Ȃ�
		return false;

	//	������r
	DWORD now = ::GetTickCount();
	int diff;

	diff = now - nLastTick;	//	TickCount�����肵�Ă�����ł��܂������͂�...

	if( diff < nInterval )	//	�K�莞�ԂɒB���Ă��Ȃ�
		return false;

	Reset();
	return true;
}



