// $Id$
//
/*!	@file
	���ݍs�̃}�[�N���Ǘ�����

	@author genta
	@version $Revision$

	Copyright (C) 1998-2000, genta
*/

#include "global.h"
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
	maxitem = max;
	Expire();	//	�w�肵�����ɗv�f�����炷
}

/*!
	@brief ���݈ʒu�̗v�f���L�����ǂ����̔���
	
	@retval true	�L��
	@retval false	����
*/
bool CMarkMgr::CheckCurrent(void) const
{
	if( curpos < Count() )
		return dat[ curpos ].IsValid();

	return false;
}

/*!
	@brief ���݈ʒu�̑O�ɗL���ȗv�f�����邩�ǂ����𒲂ׂ�

	@retval true	�L��
	@retval false	����
*/
bool CMarkMgr::CheckPrev(void) const
{
	for( int i = curpos - 1; i >= 0; i-- ){
		if( dat[ i ].IsValid() )
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
	for( int i = curpos + 1; i < Count(); i++ ){
		if( dat[ i ].IsValid() )
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
	for( int i = curpos - 1; i >= 0; i-- ){
		if( dat[ i ].IsValid() ){
			curpos = i;
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
	for( int i = curpos + 1; i < Count(); i++ ){
		if( dat[ i ].IsValid() ){
			curpos = i;
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
	dat.erase( dat.begin(), dat.end());
	curpos = 0;
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
	if( curpos < dat.size()){
		//	���݈ʒu�܂ŗv�f���폜
		dat.erase( dat.begin() + curpos, dat.end() );
	}
	
	//	�v�f�̒ǉ�
	dat.push_back(m);
	++curpos;
	
	//	�K�萔�𒴂��Ă��܂��Ƃ��̑Ή�
	Expire();
}

/*!
	�v�f�����ő�l�𒴂��Ă���ꍇ�ɗv�f�����͈͓��Ɏ��܂�悤�A
	�Â���(�ԍ��̎Ⴂ��)����폜����B
*/
void CAutoMarkMgr::Expire(void)
{
	int range = dat.size() - GetMax();
	
	if( range <= 0 )	return;
	
	//	�ő�l�𒴂��Ă���ꍇ
	dat.erase( dat.begin(), dat.begin() + range );
	curpos -= range;
	if( curpos < 0 )
		curpos = 0;
}
