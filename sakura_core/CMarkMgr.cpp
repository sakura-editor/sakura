// $Id$
//
//	CMark.cpp	���ݍs�̃}�[�N���Ǘ�����
//
//	Author: genta
//	Copyright (C) 1998-2000, genta
//

#include "global.h"
#include "CMarkMgr.h"

//-----------------------------------
// CMarkMgr
//-----------------------------------
void CMarkMgr::SetMax(int max)
{
	maxitem = max;
	Expire();	//	�w�肵�����ɗv�f�����炷
}

//	���݈ʒu�̗v�f���L�����ǂ���
bool CMarkMgr::CheckCurrent(void) const
{
	if( curpos < Count() )
		return dat[ curpos ].IsValid();

	return false;
}

//	���݈ʒu�̑O�ɗL���ȗv�f�����邩
bool CMarkMgr::CheckPrev(void) const
{
	for( int i = curpos - 1; i >= 0; i-- ){
		if( dat[ i ].IsValid() )
			return true;
	}
	return false;
}

//	���݈ʒu�̌�ɗL���ȗv�f�����邩
bool CMarkMgr::CheckNext(void) const
{
	for( int i = curpos + 1; i < Count(); i++ ){
		if( dat[ i ].IsValid() )
			return true;
	}
	return false;
}

//	���݈ʒu��O�̗L���Ȉʒu�܂Ői�߁Ctrue��Ԃ��D
//	�O�̗L���ȗv�f���Ȃ���Ό��݈ʒu�͈ړ�������false��Ԃ��D
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
//	���݈ʒu�����̗L���Ȉʒu�܂Ői�߁Ctrue��Ԃ��D
//	���̗L���ȗv�f���Ȃ���Ό��݈ʒu�͈ړ�������false��Ԃ��D
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

//-----------------------------------
// CAutoMarkMgr
//-----------------------------------

//	�v�f�̒ǉ�
//
//	���݈ʒu�ɗv�f��ǉ�����D���݈ʒu�����͑S�č폜
//
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

//	�v�f�����ő�l�𒴂��Ă���ꍇ�ɗv�f���폜����
//
//	�͈͓��Ɏ��܂�悤�ɌÂ�������폜����
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
