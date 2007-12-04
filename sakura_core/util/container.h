/*
	コンテナ類

	2007.11.27 kobake 作成
*/

#pragma once

#include <vector>
#include <algorithm> //find

//! vectorにちょっと機能を追加した版
template <class T>
class vector_ex : public std::vector<T>{
public:
	// -- -- 定数 -- -- //
	static const size_t INVALID_INDEX = (size_t)-1;

public:
	// -- -- インターフェース -- -- //
	//!要素を探す。見つけたらそのインデックスを返す。見つからなかったらINVALID_INDEXを返す。
	size_t index_of(const T& t) const
	{
		const_iterator p = find(begin(),end(),t);
		if(p!=end()){
			return p-begin();
		}
		else{
			return INVALID_INDEX;
		}
	}

	//!要素を探す。見つかればtrue。
	bool exist(const T& t) const
	{
		return index_of(t)!=INVALID_INDEX;
	}

	//!要素を追加。ただし重複した要素は弾く。
	void push_back_unique(const T& t)
	{
		if(!exist(t))
			push_back(t);
	}
};

