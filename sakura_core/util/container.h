/*
	コンテナ類

	2007.11.27 kobake 作成
*/
/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CONTAINER_82819006_7BEE_4D84_82A0_B61B0E0BAFD79_H_
#define SAKURA_CONTAINER_82819006_7BEE_4D84_82A0_B61B0E0BAFD79_H_

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

#endif /* SAKURA_CONTAINER_82819006_7BEE_4D84_82A0_B61B0E0BAFD79_H_ */
/*[EOF]*/
