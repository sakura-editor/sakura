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

#ifndef __CMACROTYPEMGR_H_
#define __CMACROTYPEMGR_H_

#pragma warning(disable: 4786)

#include <map>
#include <string>

class CMacroManagerBase;

/*!
	@brief マクロHandler生成クラス

	Singleton
*/
class CMacroFactory {
public:
	typedef CMacroManagerBase* (*Creator)(const char*);

	bool Register( const char*, Creator );
	bool Unregister( const char* );

	CMacroManagerBase* Create(const char*);
	
	static CMacroFactory* Instance();

private:
	CMacroFactory();
	std::string Ext2Key(const char *ext);

	typedef std::map< std::string, Creator> MacroTypeRep;

	MacroTypeRep m_mMacroEngines;
};
#endif
