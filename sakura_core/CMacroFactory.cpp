//	$Id$
/*!	@file
	@brief �}�N����ʊǗ�

	@author genta
	$Revision$
*/
/*
	Copyright (C) 2002, genta

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
#include "CMacroFactory.h"
#include <algorithm>
#include "ctype.h"
#include "debug.h"

static const char NULSTR[] = "";

CMacroFactory::CMacroFactory()
{}

/*!
	�^����ꂽ�g���q��map��key�ɕϊ�����
	
	@param ext [in] �g���q
	
	@par Rule
	@li NULL��""�ɂ���B
	@li �A���t�@�x�b�g�͏������ɓ���
*/
std::string CMacroFactory::Ext2Key(const char *ext)
{
	if( ext == NULL ){
		ext = NULSTR;
	}
	
	std::string key = ext;
	std::transform( key.begin(), key.end(), key.begin(), (int (*)(int))tolower);

	return key;
}

#if 0
// Jan. 31, 2004 genta
// �o�C�i���T�C�Y�팸�̂��ߊg���q�ێ��pmap���폜

/*!
	�Ή��g���q�̓o�^
	
	�g���q�ƑΉ�����Creator��o�^����D
	
	@param ext [in] �֘A�Â���g���q
	@param f [in] �o�^����Factory�֐�
	
	@sa CMacroFactory::RegisterCreator
	
	@date 2002.08.25 genta ���O�ύX
*/
bool CMacroFactory::RegisterExt( const char* ext, Creator f )
{
	if( f == NULL ){
		return false;
	}

	std::string key = Ext2Key( ext );
	
#ifdef _DEBUG
	MYTRACE( "CMacroFactory::RegisterExt(%s)\n", key.c_str() );
#endif

	//	�㏑���`�F�b�N�͍s��Ȃ�
	m_mMacroExts[ key ] = f;
	return true;
}
#endif

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
#if 0
	// Jan. 31, 2004 genta
	// �o�C�i���T�C�Y�팸�̂��ߊg���q�ێ��pmap���폜
	//	�g���q���X�g����̍폜
	MacroTypeRep::iterator ext_it = m_mMacroExts.begin();
	while( ext_it != m_mMacroExts.end() ){
		if( ext_it->second == f ){
			MacroTypeRep::iterator tmp_it;

			//	�����Ȃ�폜�����iterator�������ɂȂ�̂ŁC
			//	iterator��1�i�߂Ă��猻�݈ʒu���폜����D
			tmp_it = ext_it++;
			m_mMacroExts.erase( tmp_it );
		}
		else {
			++ ext_it;
		}
	}
#endif

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
CMacroManagerBase* CMacroFactory::Create(const char* ext)
{
	std::string key = Ext2Key( ext );

#if 0
	// Jan. 31, 2004 genta
	// �o�C�i���T�C�Y�팸�̂��ߊg���q�ێ��pmap���폜
	MacroTypeRep::iterator ext_it = m_mMacroExts.find( key );
	if( ext_it != m_mMacroExts.end()){
		CMacroManagerBase* pobj = (*ext_it->second)(key.c_str());
		if( pobj != NULL ){
#ifdef _DEBUG
			MYTRACE( "CMacroFactory::Create/ Found in map (%s)\n", key.c_str() );
#endif
			return pobj;
		}
		
		//	NULL���Ԃ��ꂽ�ꍇ
	}
#endif
	//	Creator�����Ɏ���
	for( MacroEngineRep::iterator c_it = m_mMacroCreators.begin();
		c_it != m_mMacroCreators.end(); ++ c_it ){
		CMacroManagerBase* pobj = (*c_it)(key.c_str());
		if( pobj != NULL ){
#ifdef _DEBUG
			MYTRACE( "CMacroFactory::Create/ Answered for (%s)\n", key.c_str() );
#endif
			//	�Ή��\�ɓo�^����
#if 0
			// Jan. 31, 2004 genta
			// �o�C�i���T�C�Y�팸�̂��ߊg���q�ێ��pmap���폜
			m_mMacroExts[ key ] = *c_it;
#endif
			return pobj;
		}
	}
	
	return NULL;
}

/*!
	Singleton�̂��߂�Instance�擾�֐��B
	
	Scott Meyers��Singleton
*/
CMacroFactory* CMacroFactory::Instance(void)
{
	static CMacroFactory factory;
	return &factory;
}
