//
/*!	@file
	@brief 現在行のマークを管理する

	@author genta
*/
/*
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka

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
#include "_main/global.h"
#include "CMarkMgr.h"

//-----------------------------------
// CMarkMgr
//-----------------------------------
/*!
	@brief 保管する最大件数を指定する。

	現在より小さい値を設定したときは余分な要素は削除される。

	@param max 設定する最大件数
*/
void CMarkMgr::SetMax(int max)
{
	m_nMaxitem = max;
	Expire();	//	指定した数に要素を減らす
}

/*!
	@brief 現在位置の要素が有効かどうかの判定

	@retval true	有効
	@retval false	無効
*/
bool CMarkMgr::CheckCurrent(void) const
{
	if( m_nCurpos < Count() )
		return m_cMarkChain[ m_nCurpos ].IsValid();

	return false;
}

/*!
	@brief 現在位置の前に有効な要素があるかどうかを調べる

	@retval true	有る
	@retval false	無い
*/
bool CMarkMgr::CheckPrev(void) const
{
	for( int i = m_nCurpos - 1; i >= 0; i-- ){
		if( m_cMarkChain[ i ].IsValid() )
			return true;
	}
	return false;
}

/*!
	@brief 現在位置の後に有効な要素があるかどうかを調べる

	@retval true	有る
	@retval false	無い
*/
bool CMarkMgr::CheckNext(void) const
{
	for( int i = m_nCurpos + 1; i < Count(); i++ ){
		if( m_cMarkChain[ i ].IsValid() )
			return true;
	}
	return false;
}

/*!
	@brief 現在位置を前の有効な位置まで進める

	@retval true	正常終了。現在位置は1つ前の有効な要素に移動した。
	@retval false	有効な要素が見つからなかった。現在位置は移動していない。
*/
bool CMarkMgr::PrevValid(void)
{
	for( int i = m_nCurpos - 1; i >= 0; i-- ){
		if( m_cMarkChain[ i ].IsValid() ){
			m_nCurpos = i;
			return true;
		}
	}
	return false;
}
/*!
	@brief 現在位置を後の有効な位置まで進める

	@retval true	正常終了。現在位置は1つ後の有効な要素に移動した。
	@retval false	有効な要素が見つからなかった。現在位置は移動していない。
*/
bool CMarkMgr::NextValid(void)
{
	for( int i = m_nCurpos + 1; i < Count(); i++ ){
		if( m_cMarkChain[ i ].IsValid() ){
			m_nCurpos = i;
			return true;
		}
	}
	return false;
}

//	From Here Apr. 1, 2001 genta
/*!
	現在のデータを全て消去し、現在位置のポインタをリセットする。

	@par history
	Apr. 1, 2001 genta 新規追加
*/
void CMarkMgr::Flush(void)
{
	m_cMarkChain.erase( m_cMarkChain.begin(), m_cMarkChain.end() );
	m_nCurpos = 0;
}
//	To Here

//-----------------------------------
// CAutoMarkMgr
//-----------------------------------

/*!
	現在位置に要素を追加する．現在位置より後ろは全て削除する。
	要素番号が大きい方が新しいデータ。

	@param m 追加する要素
*/
void CAutoMarkMgr::Add(const CMark& m)
{
	//	現在位置が途中の時
	if( m_nCurpos < (int)m_cMarkChain.size() ){
		//	現在位置まで要素を削除
		m_cMarkChain.erase( m_cMarkChain.begin() + m_nCurpos, m_cMarkChain.end() );
	}

	//	要素の追加
	m_cMarkChain.push_back(m);
	++m_nCurpos;

	//	規定数を超えてしまうときの対応
	Expire();
}

/*!
	要素数が最大値を超えている場合に要素数が範囲内に収まるよう、
	古い方(番号の若い方)から削除する。
*/
void CAutoMarkMgr::Expire(void)
{
	int range = m_cMarkChain.size() - GetMax();

	if( range <= 0 )	return;

	//	最大値を超えている場合
	m_cMarkChain.erase( m_cMarkChain.begin(), m_cMarkChain.begin() + range );
	m_nCurpos -= range;
	if( m_nCurpos < 0 )
		m_nCurpos = 0;
}



