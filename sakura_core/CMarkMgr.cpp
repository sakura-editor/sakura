// $Id$
//
//	CMark.cpp	現在行のマークを管理する
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
	Expire();	//	指定した数に要素を減らす
}

//	現在位置の要素が有効かどうか
bool CMarkMgr::CheckCurrent(void) const
{
	if( curpos < Count() )
		return dat[ curpos ].IsValid();

	return false;
}

//	現在位置の前に有効な要素があるか
bool CMarkMgr::CheckPrev(void) const
{
	for( int i = curpos - 1; i >= 0; i-- ){
		if( dat[ i ].IsValid() )
			return true;
	}
	return false;
}

//	現在位置の後に有効な要素があるか
bool CMarkMgr::CheckNext(void) const
{
	for( int i = curpos + 1; i < Count(); i++ ){
		if( dat[ i ].IsValid() )
			return true;
	}
	return false;
}

//	現在位置を前の有効な位置まで進め，trueを返す．
//	前の有効な要素がなければ現在位置は移動せずにfalseを返す．
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
//	現在位置を次の有効な位置まで進め，trueを返す．
//	次の有効な要素がなければ現在位置は移動せずにfalseを返す．
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

//	要素の追加
//
//	現在位置に要素を追加する．現在位置より後ろは全て削除
//
void CAutoMarkMgr::Add(const CMark& m)
{
	//	現在位置が途中の時
	if( curpos < dat.size()){
		//	現在位置まで要素を削除
		dat.erase( dat.begin() + curpos, dat.end() );
	}
	
	//	要素の追加
	dat.push_back(m);
	++curpos;
	
	//	規定数を超えてしまうときの対応
	Expire();
}

//	要素数が最大値を超えている場合に要素を削除する
//
//	範囲内に収まるように古い方から削除する
void CAutoMarkMgr::Expire(void)
{
	int range = dat.size() - GetMax();
	
	if( range <= 0 )	return;
	
	//	最大値を超えている場合
	dat.erase( dat.begin(), dat.begin() + range );
	curpos -= range;
	if( curpos < 0 )
		curpos = 0;
}
