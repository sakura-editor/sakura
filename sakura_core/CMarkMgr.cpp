//
/*!	@file
	@brief ���ݍs�̃}�[�N���Ǘ�����

	@author genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka

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

#include "StdAfx.h"
#include "_main/global.h"
#include "CMarkMgr.h"

//-----------------------------------
// CMarkMgr
//-----------------------------------
/*!
	@brief �ۊǂ���ő匏�����w�肷��B

	���݂�菬�����l��ݒ肵���Ƃ��͗]���ȗv�f�͍폜�����B

	@param max �ݒ肷��ő匏��
*/
void CMarkMgr::SetMax(int max)
{
	m_nMaxitem = max;
	Expire();	//	�w�肵�����ɗv�f�����炷
}

/*!
	@brief ���݈ʒu�̗v�f���L�����ǂ����̔���

	@retval true	�L��
	@retval false	����
*/
bool CMarkMgr::CheckCurrent(void) const
{
	if( m_nCurpos < Count() )
		return m_cMarkChain[ m_nCurpos ].IsValid();

	return false;
}

/*!
	@brief ���݈ʒu�̑O�ɗL���ȗv�f�����邩�ǂ����𒲂ׂ�

	@retval true	�L��
	@retval false	����
*/
bool CMarkMgr::CheckPrev(void) const
{
	for( int i = m_nCurpos - 1; i >= 0; i-- ){
		if( m_cMarkChain[ i ].IsValid() )
			return true;
	}
	return false;
}

/*!
	@brief ���݈ʒu�̌�ɗL���ȗv�f�����邩�ǂ����𒲂ׂ�

	@retval true	�L��
	@retval false	����
*/
bool CMarkMgr::CheckNext(void) const
{
	for( int i = m_nCurpos + 1; i < Count(); i++ ){
		if( m_cMarkChain[ i ].IsValid() )
			return true;
	}
	return false;
}

/*!
	@brief ���݈ʒu��O�̗L���Ȉʒu�܂Ői�߂�

	@retval true	����I���B���݈ʒu��1�O�̗L���ȗv�f�Ɉړ������B
	@retval false	�L���ȗv�f��������Ȃ������B���݈ʒu�͈ړ����Ă��Ȃ��B
*/
bool CMarkMgr::PrevValid(void)
{
	for( int i = m_nCurpos - 1; i >= 0; i-- ){
		if( m_cMarkChain[ i ].IsValid() ){
			m_nCurpos = i;
			return true;
		}
	}
	return false;
}
/*!
	@brief ���݈ʒu����̗L���Ȉʒu�܂Ői�߂�

	@retval true	����I���B���݈ʒu��1��̗L���ȗv�f�Ɉړ������B
	@retval false	�L���ȗv�f��������Ȃ������B���݈ʒu�͈ړ����Ă��Ȃ��B
*/
bool CMarkMgr::NextValid(void)
{
	for( int i = m_nCurpos + 1; i < Count(); i++ ){
		if( m_cMarkChain[ i ].IsValid() ){
			m_nCurpos = i;
			return true;
		}
	}
	return false;
}

//	From Here Apr. 1, 2001 genta
/*!
	���݂̃f�[�^��S�ď������A���݈ʒu�̃|�C���^�����Z�b�g����B

	@par history
	Apr. 1, 2001 genta �V�K�ǉ�
*/
void CMarkMgr::Flush(void)
{
	m_cMarkChain.erase( m_cMarkChain.begin(), m_cMarkChain.end() );
	m_nCurpos = 0;
}
//	To Here

//-----------------------------------
// CAutoMarkMgr
//-----------------------------------

/*!
	���݈ʒu�ɗv�f��ǉ�����D���݈ʒu�����͑S�č폜����B
	�v�f�ԍ����傫�������V�����f�[�^�B

	@param m �ǉ�����v�f
*/
void CAutoMarkMgr::Add(const CMark& m)
{
	//	���݈ʒu���r���̎�
	if( m_nCurpos < (int)m_cMarkChain.size() ){
		//	���݈ʒu�܂ŗv�f���폜
		m_cMarkChain.erase( m_cMarkChain.begin() + m_nCurpos, m_cMarkChain.end() );
	}

	//	�v�f�̒ǉ�
	m_cMarkChain.push_back(m);
	++m_nCurpos;

	//	�K�萔�𒴂��Ă��܂��Ƃ��̑Ή�
	Expire();
}

/*!
	�v�f�����ő�l�𒴂��Ă���ꍇ�ɗv�f�����͈͓��Ɏ��܂�悤�A
	�Â���(�ԍ��̎Ⴂ��)����폜����B
*/
void CAutoMarkMgr::Expire(void)
{
	int range = m_cMarkChain.size() - GetMax();

	if( range <= 0 )	return;

	//	�ő�l�𒴂��Ă���ꍇ
	m_cMarkChain.erase( m_cMarkChain.begin(), m_cMarkChain.begin() + range );
	m_nCurpos -= range;
	if( m_nCurpos < 0 )
		m_nCurpos = 0;
}



