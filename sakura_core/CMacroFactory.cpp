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
#include "CMacroFactory.h"
#include <algorithm>

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
	std::transform( key.begin(), key.end(), key.begin(), tolower);

	return key;
}

/*!
	Creator�̓o�^
	
	@param ext [in] �֘A�Â���g���q
	@param f [in] �o�^����Factory�֐�
*/
bool CMacroFactory::Register( const char* ext, Creator f )
{
	if( f == NULL ){
		return false;
	}

	std::string key = Ext2Key( ext );
	
	//	�㏑���`�F�b�N�͍s��Ȃ�
	m_mMacroEngines[ key ] = f;
	return true;
}

/*!
	Creator�̓o�^����
	
	@param ext [in] �o�^��������g���q
*/
bool CMacroFactory::Unregister( const char* ext )
{
	std::string key = Ext2Key( ext );

	MacroTypeRep::iterator find_it =
		m_mMacroEngines.find( key );
	if( find_it == m_mMacroEngines.end())
		return false;
	
	m_mMacroEngines.erase( find_it );
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

	MacroTypeRep::iterator ext_it = m_mMacroEngines.find( key );
	if( ext_it == m_mMacroEngines.end()){
		return NULL;
	}
	return (*ext_it->second)(key.c_str());
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
