/*!	@file
	@brief �}�N����ʊǗ�

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta
	Copyright (C) 2004, genta

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
#include "CMacroFactory.h"
#include <algorithm>
#include "ctype.h"
#include "debug/Debug.h"

static const TCHAR NULSTR[] = _T("");

CMacroFactory::CMacroFactory()
{}

/*!
	�^����ꂽ�g���q��map��key�ɕϊ�����
	
	@param ext [in] �g���q
	
	@par Rule
	@li NULL��""�ɂ���B
	@li �A���t�@�x�b�g�͏������ɓ���
*/
std::tstring CMacroFactory::Ext2Key(const TCHAR *ext)
{
	if( ext == NULL ){
		ext = NULSTR;
	}
	
	std::tstring key = ext;
	std::transform( key.begin(), key.end(), key.begin(), _totlower);

	return key;
}

/*!
	Creator�̓o�^
	
	�g���q�̑Ή��������ɓo�^���Ȃ�Creator��o�^����D
	�������C��U�Ή����킩�����玟��ȍ~�͑Ή��\���g����D
	
	@param f [in] �o�^����Factory�֐�
	
	@sa CMacroFactory::RegisterExts

	@date 2002.08.25 genta �ǉ�
*/
bool CMacroFactory::RegisterCreator( Creator f )
{
	if( f == NULL ){
		return false;
	}

	m_mMacroCreators.push_back( f );
	return true;
}

/*!
	Creator�̓o�^����
	
	@param f [in] �o�^��������Creator
*/
bool CMacroFactory::Unregister( Creator f )
{
	//	Creator List����̍폜
	MacroEngineRep::iterator c_it = m_mMacroCreators.begin();
	while( c_it != m_mMacroCreators.end() ){
		if( *c_it == f ){
			MacroEngineRep::iterator tmp_it;

			//	�����Ȃ�폜�����iterator�������ɂȂ�̂ŁC
			//	iterator��1�i�߂Ă��猻�݈ʒu���폜����D
			tmp_it = c_it++;
			m_mMacroCreators.erase( tmp_it );
			//	�d���o�^����Ă���ꍇ���l�����āC
			//	1�������Ă��Ō�܂Ń`�F�b�N����
		}
		else {
			++ c_it;
		}
	}
	
	return true;
}

/*
	Object Factory
	
	�o�^���ꂽFactory Object�����ɌĂяo���āA
	Object������ꂽ�炻���Ԃ��B

	@pararm ext [in] �g���q
	@return Macro�I�u�W�F�N�g�B�K�؂Ȃ��̂�������Ȃ����NULL�B
*/
CMacroManagerBase* CMacroFactory::Create(const TCHAR* ext)
{
	std::tstring key = Ext2Key( ext );

	//	Creator�����Ɏ���
	for( MacroEngineRep::iterator c_it = m_mMacroCreators.begin();
		c_it != m_mMacroCreators.end(); ++ c_it ){
		CMacroManagerBase* pobj = (*c_it)(key.c_str());
		if( pobj != NULL ){
			DBPRINT_A( "CMacroFactory::Create/ Answered for (%ts)\n", key.c_str() );
			return pobj;
		}
	}
	
	return NULL;
}
