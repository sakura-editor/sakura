/*!	@file
	@brief マクロ種別管理

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
#include <algorithm>
#include <ctype.h>
#include "CMacroFactory.h"

static const TCHAR NULSTR[] = _T("");

CMacroFactory::CMacroFactory()
{}

/*!
	与えられた拡張子をmapのkeyに変換する
	
	@param ext [in] 拡張子
	
	@par Rule
	@li NULLは""にする。
	@li アルファベットは小文字に統一
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
	Creatorの登録
	
	拡張子の対応を初期に登録しないCreatorを登録する．
	ただし，一旦対応がわかったら次回以降は対応表が使われる．
	
	@param f [in] 登録するFactory関数
	
	@sa CMacroFactory::RegisterExts

	@date 2002.08.25 genta 追加
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
	Creatorの登録解除
	
	@param f [in] 登録解除するCreator
*/
bool CMacroFactory::Unregister( Creator f )
{
	//	Creator Listからの削除
	MacroEngineRep::iterator c_it = m_mMacroCreators.begin();
	while( c_it != m_mMacroCreators.end() ){
		if( *c_it == f ){
			MacroEngineRep::iterator tmp_it;

			//	いきなり削除するとiteratorが無効になるので，
			//	iteratorを1つ進めてから現在位置を削除する．
			tmp_it = c_it++;
			m_mMacroCreators.erase( tmp_it );
			//	重複登録されている場合を考慮して，
			//	1つ見つかっても最後までチェックする
		}
		else {
			++ c_it;
		}
	}
	
	return true;
}

/*
	Object Factory
	
	登録されたFactory Objectを順に呼び出して、
	Objectが得られたらそれを返す。

	@pararm ext [in] 拡張子
	@return Macroオブジェクト。適切なものが見つからなければNULL。
*/
CMacroManagerBase* CMacroFactory::Create(const TCHAR* ext)
{
	std::tstring key = Ext2Key( ext );

	//	Creatorを順に試す
	for( MacroEngineRep::iterator c_it = m_mMacroCreators.begin();
		c_it != m_mMacroCreators.end(); ++ c_it ){
		CMacroManagerBase* pobj = (*c_it)(key.c_str());
		if( pobj != NULL ){
			DEBUG_TRACE( _T("CMacroFactory::Create/ Answered for (%ts)\n"), key.c_str() );
			return pobj;
		}
	}
	
	return NULL;
}
