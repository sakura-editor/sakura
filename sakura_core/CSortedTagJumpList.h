//	$Id$
/*!	@file
	@brief タグジャンプリスト

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2005, MIK

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

#ifndef _CSORTED_TAGJUMP_LIST_H_
#define _CSORTED_TAGJUMP_LIST_H_

#include "global.h"

#define MAX_TAG_STRING_LENGTH _MAX_PATH	//管理する文字列の最大長

/*!	@brief ダイレクトタグジャンプ用検索結果をソートして保持する．
	@author MIK
*/
class CSortedTagJumpList {
public:
	CSortedTagJumpList(int max);
	~CSortedTagJumpList();

	BOOL AddParam( TCHAR* keyword, TCHAR* filename, int no, TCHAR type, TCHAR* note, int depth );
	BOOL GetParam( int index, TCHAR* keyword, TCHAR* filename, int* no, TCHAR* type, TCHAR* note, int* depth );
	int GetCount( void ){ return m_nCount; }
	void Empty( void );
	bool IsOverflow( void ){ return m_bOverflow; }

	typedef struct tagjump_info_t {
		struct tagjump_info_t*	next;	//!< 次のリスト
		TCHAR*	keyword;	//!< キーワード
		TCHAR*	filename;	//!< ファイル名
		int		no;			//!< 行番号
		TCHAR	type;		//!< 種類
		TCHAR*	note;		//!< 備考
		int		depth;		//!< (さかのぼる)階層
	} TagJumpInfo;

	TagJumpInfo* GetPtr( int index );

	/*!	@brief 管理数の最大値を取得する

		@date 2005.04.22 genta 最大値を可変に
	*/
	int GetCapacity(void) const { return m_MAX_TAGJUMPLIST; }

private:
	TagJumpInfo*	m_pTagjump;	//!< タグジャンプ情報
	int				m_nCount;	//!< 個数
	bool			m_bOverflow;	//!< オーバーフロー
	
	//	2005.04.22 genta 最大値を可変に
	const int		m_MAX_TAGJUMPLIST;	//!< 管理する情報の最大数

	void Free( TagJumpInfo* item );
};

#endif	//_CSORTED_TAGJUMP_LIST_H_
