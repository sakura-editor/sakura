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
#include "stdafx.h"
#include "CMacroFactory.h"
#include <algorithm>
#include "ctype.h"
#include "debug.h"

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
	std::transform( key.begin(), key.end(), key.begin(), (int (*)(int))tolower);

	return key;
}

#if 0
// Jan. 31, 2004 genta
// バイナリサイズ削減のため拡張子保持用mapを削除

/*!
	対応拡張子の登録
	
	拡張子と対応するCreatorを登録する．
	
	@param ext [in] 関連づける拡張子
	@param f [in] 登録するFactory関数
	
	@sa CMacroFactory::RegisterCreator
	
	@date 2002.08.25 genta 名前変更
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

	//	上書きチェックは行わない
	m_mMacroExts[ key ] = f;
	return true;
}
#endif

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
#if 0
	// Jan. 31, 2004 genta
	// バイナリサイズ削減のため拡張子保持用mapを削除
	//	拡張子リストからの削除
	MacroTypeRep::iterator ext_it = m_mMacroExts.begin();
	while( ext_it != m_mMacroExts.end() ){
		if( ext_it->second == f ){
			MacroTypeRep::iterator tmp_it;

			//	いきなり削除するとiteratorが無効になるので，
			//	iteratorを1つ進めてから現在位置を削除する．
			tmp_it = ext_it++;
			m_mMacroExts.erase( tmp_it );
		}
		else {
			++ ext_it;
		}
	}
#endif

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
CMacroManagerBase* CMacroFactory::Create(const char* ext)
{
	std::string key = Ext2Key( ext );

#if 0
	// Jan. 31, 2004 genta
	// バイナリサイズ削減のため拡張子保持用mapを削除
	MacroTypeRep::iterator ext_it = m_mMacroExts.find( key );
	if( ext_it != m_mMacroExts.end()){
		CMacroManagerBase* pobj = (*ext_it->second)(key.c_str());
		if( pobj != NULL ){
#ifdef _DEBUG
			MYTRACE( "CMacroFactory::Create/ Found in map (%s)\n", key.c_str() );
#endif
			return pobj;
		}
		
		//	NULLが返された場合
	}
#endif
	//	Creatorを順に試す
	for( MacroEngineRep::iterator c_it = m_mMacroCreators.begin();
		c_it != m_mMacroCreators.end(); ++ c_it ){
		CMacroManagerBase* pobj = (*c_it)(key.c_str());
		if( pobj != NULL ){
#ifdef _DEBUG
			MYTRACE( "CMacroFactory::Create/ Answered for (%s)\n", key.c_str() );
#endif
			//	対応表に登録する
#if 0
			// Jan. 31, 2004 genta
			// バイナリサイズ削減のため拡張子保持用mapを削除
			m_mMacroExts[ key ] = *c_it;
#endif
			return pobj;
		}
	}
	
	return NULL;
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
