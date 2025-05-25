/*! @file */
/*
	コンテナ類

	2007.11.27 kobake 作成
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CONTAINER_30F2CB18_623C_48C4_BC32_ED3563FB2763_H_
#define SAKURA_CONTAINER_30F2CB18_623C_48C4_BC32_ED3563FB2763_H_
#pragma once

#include <vector>
#include <algorithm> //find

//! vectorにちょっと機能を追加した版
template <class T>
class vector_ex : public std::vector<T>{
public:
	using std::vector<T>::begin;
	using std::vector<T>::end;
	using std::vector<T>::push_back;

public:
	// -- -- インターフェース -- -- //
	//!要素を探す。見つかればtrue。
	bool exist(const T& t) const
	{
		return std::find(begin(), end(), t) != end();
	}

	//!要素を追加。ただし重複した要素は弾く。
	bool push_back_unique(const T& t)
	{
		if(!exist(t)){
			push_back(t);
			return true;
		}
		return false;
	}
};
#endif /* SAKURA_CONTAINER_30F2CB18_623C_48C4_BC32_ED3563FB2763_H_ */
