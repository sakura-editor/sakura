//	$Id$
/*!	@file
	@brief マクロ種別管理

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
	与えられた拡張子をmapのkeyに変換する
	
	@param ext [in] 拡張子
	
	@par Rule
	@li NULLは""にする。
	@li アルファベットは小文字に統一
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
	Creatorの登録
	
	@param ext [in] 関連づける拡張子
	@param f [in] 登録するFactory関数
*/
bool CMacroFactory::Register( const char* ext, Creator f )
{
	if( f == NULL ){
		return false;
	}

	std::string key = Ext2Key( ext );
	
	//	上書きチェックは行わない
	m_mMacroEngines[ key ] = f;
	return true;
}

/*!
	Creatorの登録解除
	
	@param ext [in] 登録解除する拡張子
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
	
	登録されたFactory Objectを順に呼び出して、
	Objectが得られたらそれを返す。

	@pararm ext [in] 拡張子
	@return Macroオブジェクト。適切なものが見つからなければNULL。
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
	SingletonのためのInstance取得関数。
	
	Scott MeyersのSingleton
*/
CMacroFactory* CMacroFactory::Instance(void)
{
	static CMacroFactory factory;
	return &factory;
}
