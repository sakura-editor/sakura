/*! @file */
/*
2007.10.23 kobake

デザインパターン的なモノを置けると良いなぁ。
ちなみに TSingleInstance はシングルトンパターンとは似て非なるモノですが。
*/
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

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
#ifndef SAKURA_DESIGN_TEMPLATE_BBC57590_CED0_40D0_B719_F5A4522B8A56_H_
#define SAKURA_DESIGN_TEMPLATE_BBC57590_CED0_40D0_B719_F5A4522B8A56_H_
#pragma once

#include "debug/Debug2.h"

// http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Copy_Constructors
// A macro to disallow the copy constructor and operator= functions
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  TypeName& operator=(const TypeName&) = delete; \
  TypeName(TypeName&&) = delete;           \
  TypeName& operator=(TypeName&&) = delete;

/*!
	Singletonパターン

	2008.03.03 kobake 作成
*/
template <class T>
class TSingleton{
public:
	//公開インターフェース
	static T* getInstance()
	{
		static T instance;
		return &instance;
	}

protected:
	TSingleton(){}
	DISALLOW_COPY_AND_ASSIGN(TSingleton);
};

/*!
	シングルインスタンス

	1プロセスあたりのインスタンス数を制限するためのテンプレート。
	シングルインスタンスは複数インスタンスを生成しないクラスに適用する。
	シングルインスタンスの生成済みのインスタンスはstaticメソッドから取得できる。
	インスタンス自動生成は行わないので、インスタンス生成は手動で行うこと。

	デザインパターンの「シングルトン」とは関係ないので、派生クラスは「状態」を持って良い。

	@date 2007.10.23 kobake 作成
 */
template <class T>
class TSingleInstance {
private:
	static T* gm_instance;				//!< シングルインスタンスを保持するポインタ

public:
	/*!
		作成済みのインスタンスを取得する

		@returns 作成済みのインスタンス
		@retval nullptr インスタンスが未生成
	 */
	[[nodiscard]] static T* getInstance() noexcept { return gm_instance; }

protected:
	/*!
		コンストラクタ

		staticメンバにインスタンスを記録する。
	 */
	TSingleInstance()
	{
		assert(gm_instance == nullptr);
		gm_instance = static_cast<T*>(this);
	}

	/*!
		デストラクタ

		staticメンバのポインタをクリアする。
	 */
	virtual ~TSingleInstance() noexcept
	{
		gm_instance = nullptr;
	}
};

/*!
	シングルインスタンスを保持するポインタ

	1プロセスあたり1つのインスタンスだけを許可する機構で、
	TSingleInstance<T>以外からはアクセスさせない。
 */
template <class T>
T* TSingleInstance<T>::gm_instance = nullptr;

//記録もする
#include <vector>
template <class T> class TInstanceHolder{
public:
	TInstanceHolder()
	{
		gm_table.push_back(static_cast<T*>(this));
	}
	virtual ~TInstanceHolder()
	{
		for(size_t i=0;i<gm_table.size();i++){
			if(gm_table[i]==static_cast<T*>(this)){
				gm_table.erase(gm_table.begin()+i);
				break;
			}
		}
	}
	static int GetInstanceCount(){ return (int)gm_table.size(); }
	static T* GetInstance(int nIndex)
	{
		if(nIndex>=0 && nIndex<(int)gm_table.size()){
			return gm_table[nIndex];
		}else{
			return 0;
		}
	}

private:
	static std::vector<T*> gm_table;
};
template <class T> std::vector<T*> TInstanceHolder<T>::gm_table;
#endif /* SAKURA_DESIGN_TEMPLATE_BBC57590_CED0_40D0_B719_F5A4522B8A56_H_ */
